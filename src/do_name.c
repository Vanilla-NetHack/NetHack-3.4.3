/*	SCCS Id: @(#)do_name.c	3.0	88/11/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static
char *
visctrl(c)
char c;
{
#ifdef LINT	/* static char ccc[3]; */
	char ccc[3];
#else
	static char ccc[3];
#endif

	if(c < 040) {
		ccc[0] = '^';
		ccc[1] = c + 0100;
		ccc[2] = 0;
	} else {
		ccc[0] = c;
		ccc[1] = 0;
	}
	return(ccc);
}

void
getpos(cc,force,goal)
coord	*cc;
int force; char *goal;
{
	register int cx, cy, i, c;
	char *sdp = flags.num_pad ? ndir : sdir;
	if(flags.verbose) pline("(For instructions type a ?)");
	cx = cc->x;
	cy = cc->y;
	curs(cx,cy+2);
	while((c = readchar()) != '.'){
		for(i=0; i<8; i++) if(sdp[i] == c){
			if(1 <= cx + xdir[i] && cx + xdir[i] <= COLNO)
				cx += xdir[i];
			if(0 <= cy + ydir[i] && cy + ydir[i] <= ROWNO-1)
				cy += ydir[i];
			goto nxtc;
		}
		if(c == '?'){
			pline("Use [%s] to move the cursor to %s.",
			      flags.num_pad ? "2468" : "hjkl", goal);
			pline("Type a . when you are at the right place.");
		} else {
			if (!index(quitchars, c))
			    pline("Unknown direction: '%s' (%s).",
				visctrl(c),
				force ?
				    flags.num_pad ? "use 2468 or ." :
						    "use hjkl or ." :
				    "aborted");
			if(force) goto nxtc;
			cc->x = -1;
			cc->y = 0;
			return;
		}
	nxtc:	;
		curs(cx,cy+2);
	}
	cc->x = cx;
	cc->y = cy;
	return;
}

int
do_mname(){
	char buf[BUFSZ];
	coord cc;
	register int cx,cy,lth,i;
	register struct monst *mtmp, *mtmp2;
	register char *curr;
	boolean blank;

	cc.x = u.ux;
	cc.y = u.uy;
	getpos(&cc, 0, "the monster you want to name");
	cx = cc.x;
	cy = cc.y;
	if(cx < 0) return(0);
	if (cx == u.ux && cy == u.uy) {
		pline("This %s creature is called %s and cannot be renamed.",
		ACURR(A_CHA) > 14 ?
		(flags.female ? "beautiful" : "handsome") :
		"ugly",
		plname);
		return(0);
	}
	if (!cansee(cx,cy) || !MON_AT(cx,cy) || (mtmp = m_at(cx, cy))->mimic
		    || (mtmp->minvis && !See_invisible) || mtmp->mundetected) {
		pline("I see no monster there.");
		return(0);
	}
	pline("What do you want to call %s? ", lmonnam(mtmp));
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033') return(0);

	/* unnames monster if all spaces */
	for (curr = buf, blank = 1; *curr; blank = (*curr++ == ' '));
	if(blank) *buf = '\0';

 	if(type_is_pname(mtmp->data)) {
 	    pline("%s doesn't like being called names!", Monnam(mtmp));
 	    if(!mtmp->mtame) {
 		pline("%s gets %sangry!", Monnam(mtmp),
 		      mtmp->mpeaceful ? "" : "very ");
 		mtmp->mpeaceful = mtmp->msleep = 0;
 	    }
 	    return(0);
 	}
	lth = strlen(buf)+1;
	if(lth > 63){
		buf[62] = 0;
		lth = 63;
	}
	mtmp2 = newmonst(mtmp->mxlth + lth);
	*mtmp2 = *mtmp;
	for(i=0; i<mtmp->mxlth; i++)
		((char *) mtmp2->mextra)[i] = ((char *) mtmp->mextra)[i];
	mtmp2->mnamelth = lth;
	Strcpy(NAME(mtmp2), buf);
	replmon(mtmp,mtmp2);
	return(0);
}

/*
 * This routine changes the address of  obj . Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when  obj  is in the inventory.
 */
static
void
do_oname(obj)
register struct obj *obj;
{
	char buf[BUFSZ];
	register char *curr;
	boolean blank;

	pline("What do you want to name %s? ", doname(obj));
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033')	return;

	/* unnames item if all spaces */
	for (curr = buf, blank = 1; *curr; blank = (*curr++ == ' '));
	if(blank) *buf = '\0';

#ifdef NAMED_ITEMS
	if(is_artifact(obj) || restr_name(obj, buf))
		pline("Somehow you can't seem to engrave that word.");
	else
#endif
		(void)oname(obj, buf, 1);
}

struct obj *
oname(obj, buf, ininv)
register struct obj *obj;
char	*buf;
register int ininv;
{
	register struct obj *otmp, *otmp2;
	register int	lth;

	lth = *buf ? strlen(buf)+1 : 0;
	if(lth > 63){
		buf[62] = 0;
		lth = 63;
	}
	otmp2 = newobj(lth);
	*otmp2 = *obj;
	otmp2->onamelth = lth;
#ifdef __GNUC__
	/* Without the following line, the program gives anything an empty
	 * name when I try to #name it.  Probably a compiler bug, but at the
	 * point where I discovered this, there's no time to check to make
	 * sure.
	 */
	if (buf) (void)donull();
#endif
	if(lth) Strcpy(ONAME(otmp2), buf);

	if (obj->owornmask) {
		/* Note: dying by burning in Hell causes problems if you
		 * try doing this when owornmask isn't set.
		 */
		setworn((struct obj *)0, obj->owornmask);
		setworn(otmp2, otmp2->owornmask);
	}

	if (ininv) {
		/* do freeinv(obj); etc. by hand in order to preserve
		   the position of this object in the inventory */
		if(obj == invent) invent = otmp2;
		else for(otmp = invent; ; otmp = otmp->nobj){
			if(!otmp)
				panic("oname: cannot find obj.");
			if(otmp->nobj == obj){
				otmp->nobj = otmp2;
				break;
			}
		}
	}
	/* obfree(obj, otmp2);	/* now unnecessary: no pointers on bill */
	free((genericptr_t) obj);	/* let us hope nobody else saved a pointer */
	return otmp2;
}

static const char callable[] = {
	SCROLL_SYM, POTION_SYM, WAND_SYM, RING_SYM, AMULET_SYM, GEM_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	ARMOR_SYM, 0 };

int
ddocall()
{
	register struct obj *obj;
#ifdef REDO
	char	ch;

	if (!in_doagain)
#endif
		pline("Name an individual object? ");
	switch(
#ifdef REDO
		ch = 
#endif
		ynq()) {
	case 'q':
		break;
	case 'y':
#ifdef REDO
		savech(ch);
#endif
		obj = getobj("#", "name");
		if(obj) do_oname(obj);
		break;
	default:
#ifdef REDO
		savech(ch);
#endif
		obj = getobj(callable, "call");
		if(obj) docall(obj);
	}
	return 0;
}

void
docall(obj)
register struct obj *obj;
{
	char buf[BUFSZ];
	struct obj otemp;
	register char **str1;
	register char *str;
	boolean blank;

	otemp = *obj;
	otemp.quan = 1;
	otemp.onamelth = 0;
	if (otemp.corpsenm) { /* kludge, meaning it's sink water */
		pline("Call a stream of %s fluid: ",
				objects[otemp.otyp].oc_descr);
	} else {
		str = xname(&otemp);
		pline("Call %s %s: ", index(vowels,*str) ? "an" : "a", str);
	}
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033')
		return;

	/* clear old name */
	str1 = &(objects[obj->otyp].oc_uname);
	if(*str1) free(*str1);

	/* uncalls item if all spaces */
	for (str = buf, blank = 1; *str; blank = (*str++ == ' '));
	if(blank) *buf = '\0';
	if (!*buf) {
		*str1 = NULL;
		return;
	}

	str = (char *) alloc((unsigned)strlen(buf)+1);
	Strcpy(str,buf);
	*str1 = str;
}

const char *ghostnames[] = {
	/* these names should have length < PL_NSIZ */
	/* Capitalize the names for aesthetics -dgk */
	"Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
	"Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
	"Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Robert",
	"Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Miracleman",
	"Stephan", "Lance Braccus", "Shadowhawk"
};

char *
x_monnam(mtmp, vb)
register struct monst *mtmp;
int vb;
{
#ifdef LINT	/* static char buf[BUFSZ]; */
	char buf[BUFSZ];
#else
	static char buf[BUFSZ];
#endif
	boolean isinvis = (mtmp->minvis && mtmp->data != &mons[PM_STALKER]
				&& mtmp->data != &mons[PM_GHOST]);

	buf[0] = '\0';
#if defined(ALTARS) && defined(THEOLOGY)
	if(mtmp->ispriest) return(priestname(mtmp));
#endif
	if(mtmp->isshk) {
		Strcpy(buf, shkname(mtmp));
		if (mtmp->data == &mons[PM_SHOPKEEPER] && !mtmp->minvis)
		    return(buf);
		/* For normal shopkeepers, just 'Asidonhopo'.
		 * For unusual ones, 'Asidonhopo the invisible shopkeeper'
		 * or 'Asidonhopo the blue dragon'.
		 */
		Strcat(buf, " ");
	} else if(mtmp->mnamelth && !vb) {
		if(isinvis) {
		    Strcpy(buf, "the invisible ");
		    Strcat(buf, NAME(mtmp));
		} else 
		    Strcpy(buf, NAME(mtmp));
		return(buf);
	}

	switch(mtmp->data->mlet) {
	    case S_GHOST:
		{ register char *gn = (char *) mtmp->mextra;
		  if(!*gn) {		/* might also look in scorefile */
		    gn = ghostnames[rn2(SIZE(ghostnames))];
			Strcpy((char *) mtmp->mextra, !rn2(5) ? plname : gn);
		  }
		  Sprintf(buf, "%s's ghost", gn);
		}
		break;
	    default:
		if (mtmp->minvis)
			Strcat(buf, "the invisible ");
		else if (!type_is_pname(mtmp->data) || Hallucination
				|| mtmp->data == &mons[PM_WIZARD_OF_YENDOR])
			Strcat(buf, "the ");
		Strcat(buf, Hallucination ? rndmonnam() : mtmp->data->mname);
	}
	if(vb && mtmp->mnamelth) {
		Strcat(buf, " called ");
		Strcat(buf, NAME(mtmp));
	}
	return(buf);
}

char *
lmonnam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, 1));
}

char *
mon_nam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, 0));
}

char *
Monnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = mon_nam(mtmp);

	if('a' <= *bp && *bp <= 'z') *bp += ('A' - 'a');
	return(bp);
}

char *
a_monnam(mtmp,adj)
register struct monst *mtmp;
register char *adj;
{
	register char *bp = mon_nam(mtmp);
#ifdef LINT	/* static char buf[BUFSZ]; */
	char buf[BUFSZ];
#else
	static char buf[BUFSZ];
#endif

	if(!strncmp(bp, "the ", 4)) bp += 4;
	Sprintf(buf, "the %s %s", adj, bp);
	return(buf);
}

/* sometimes we don't want an article in front of definite names */

char *
a2_monnam(mtmp,adj)
register struct monst *mtmp;
register char *adj;
{
	register char *bp = mon_nam(mtmp);
#ifdef LINT	/* static char buf[BUFSZ]; */
	char buf[BUFSZ];
#else
	static char buf[BUFSZ];
#endif

	if(!strncmp(bp, "the ", 4))
		Sprintf(buf, "the %s %s", adj, bp+4);
	else
		Sprintf(buf, "%s %s", adj, bp);
	return(buf);
}

char *
Amonnam(mtmp, adj)
register struct monst *mtmp;
register char *adj;
{
	register char *bp = a_monnam(mtmp,adj);

	*bp = 'T';
	return(bp);
}

char *
Xmonnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = Monnam(mtmp);

	if(!strncmp(bp, "The ", 4) && !type_is_pname(mtmp->data)) {
		if(index(vowels,*(bp+4))) {
			*((++bp)+1) = 'n';
		} else
			bp += 2;
		*bp = 'A';
	}
	return(bp);
}

char *
defmonnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = Xmonnam(mtmp);

	if (!strncmp(bp,"A ",2) || !strncmp(bp,"An ",3)) *bp = 'a';
	return(bp);
}

char *
rndmonnam() {  /* Random name of monster type, if hallucinating */
	int name;

	do {
		name = rn2(PM_CHAMELEON);
		/* chameleon: last monster before player classes */
	} while(type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN));
	return(mons[name].mname);
}

#ifdef REINCARNATION
char *
roguename() /* Name of a Rogue player */
{
	char *i, *opts;

	if(opts = getenv("ROGUEOPTS")) {
		for(i=opts; *i; i++)
			if (!strncmp("name=",i,5)) {
				char *j;
				if (j=index(i+5,','))
					*j = (char)0;
				return i+5;
			}
	}
	return rn2(3) ? (rn2(2) ? "Michael Toy" : "Kenneth Arnold")
		: "Glenn Wichman";
}
#endif

