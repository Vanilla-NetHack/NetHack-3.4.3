/*	SCCS Id: @(#)do_name.c	3.0	89/11/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef NAMED_ITEMS
# include <ctype.h>
#endif

#ifdef OVLB

static char *FDECL(visctrl, (CHAR_P));
static void FDECL(do_oname, (struct obj *));

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
int force;
const char *goal;
{
	register int cx, cy, i, c;
	const char *sdp = flags.num_pad ? ndir : sdir;
#ifdef MACOS
	extern short	macflags;
	Boolean	fUpdateFlagOn;
	long	ticks;
#endif

	if(flags.verbose) pline("(For instructions type a ?)");
#ifdef MACOS	
	if ((macflags & fDoUpdate) && (macflags & fDoNonKeyEvt)) {
		fUpdateFlagOn = true;
	} else {
		fUpdateFlagOn = false;
		macflags |= (fDoUpdate | fDoNonKeyEvt);
	}
	macflags |= fMoveWRTMouse;
#endif
	cx = cc->x;
	cy = cc->y;
#ifdef CLIPPING
	cliparound(cx, cy);
	(void) win_curs(cx, cy);
#else
	curs(cx,cy+2);
#endif
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
#ifdef MACOS
			macflags &= ~fMoveWRTMouse;
			if (!fUpdateFlagOn)
				macflags &= ~(fDoUpdate | fDoNonKeyEvt);
#endif
			cc->x = -1;
			cc->y = 0;
			return;
		}
	nxtc:	;
#ifdef CLIPPING
		cliparound(cx, cy);
		(void) win_curs(cx, cy);
#else
		curs(cx,cy+2);
#endif
	}
#ifdef MACOS
	macflags &= ~fMoveWRTMouse;
	if (!fUpdateFlagOn)
		macflags &= ~(fDoUpdate | fDoNonKeyEvt);
#endif
	cc->x = cx;
	cc->y = cy;
	return;
}

struct monst *
christen_monst(mtmp, name)
struct monst *mtmp;
const char *name;
{
	register int lth,i;
	register struct monst *mtmp2;

	/* dogname and catname are 63-character arrays; the generic naming
	 * function do_mname() below also cut names off at 63 characters */
	lth = strlen(name)+1;
	if(lth > 63){
		lth = 63;
	}
	mtmp2 = newmonst(mtmp->mxlth + lth);
	*mtmp2 = *mtmp;
	for(i=0; i<mtmp->mxlth; i++)
		((char *) mtmp2->mextra)[i] = ((char *) mtmp->mextra)[i];
	mtmp2->mnamelth = lth;
	(void)strncpy(NAME(mtmp2), name, lth);
	NAME(mtmp2)[lth-1] = 0;
	replmon(mtmp,mtmp2);
	return(mtmp2);
}

int
do_mname(){
	char buf[BUFSZ];
	coord cc;
	register int cx,cy;
	register struct monst *mtmp;
	register char *curr;
	boolean blank;

	if (Hallucination) {
		You("would never recognize it anyway.");
		return 0;
	}
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
 		mtmp->mpeaceful = 0;
		mtmp->msleep = 0;
 	    }
 	    return(0);
 	}
	(void) christen_monst(mtmp, buf);
	return(0);
}

/*
 * This routine changes the address of obj. Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when obj is in the inventory.
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
	if(is_artifact(obj))
		pline("The artifact seems to resist the attempt.");
	else if (restr_name(obj, buf) || exist_artifact(obj, buf)) {
		int n = rn2(strlen(buf));
		char c1,c2;

		c1 = isupper(buf[n]) ? tolower(buf[n]) : buf[n];
		while (c1 == (c2 = 'a' + rn2('z'-'a')));
		if (isupper(buf[n]))
			/* islower(c2) guaranteed by generation */
			buf[n] = toupper(c2);
		else buf[n] = c2;
		pline("While engraving your hand slips.");
		more();
		You("engrave: \"%s\".",buf);
		(void)oname(obj, buf, 1);
	}
	else
#endif
		(void)oname(obj, buf, 1);
}

struct obj *
oname(obj, buf, ininv)
register struct obj *obj;
const char	*buf;
register int ininv;
{
	register struct obj *otmp, *otmp2, *contents;
	register int	lth;

	lth = *buf ? strlen(buf)+1 : 0;
	if(lth > 63){
		lth = 63;
	}
	/* if already properly named */
	if(lth == obj->onamelth && (!lth || !strcmp(ONAME(obj),buf)))
		return obj;
#ifdef NAMED_ITEMS
	/* If named artifact exists in the game, do not create another.
	 * Also trying to create an artifact shouldn't de-artifact
	 * it (e.g. Excalibur from prayer). In this case the object
	 * will retain its current name. */
	if (is_artifact(obj) || exist_artifact(obj, buf))
		return obj;
	else
		artifact_exists(obj, buf, TRUE);
#endif
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
	if(lth) {
		(void)strncpy(ONAME(otmp2), buf, lth);
		ONAME(otmp2)[lth-1] = 0;
	}
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
	if (Is_container(obj)) {
		for(contents=fcobj; contents; contents=contents->nobj)
			if(contents->cobj==obj) contents->cobj = otmp2;
	}
	/* obfree(obj, otmp2);	/* now unnecessary: no pointers on bill */
	free((genericptr_t) obj);	/* let us hope nobody else saved a pointer */
	return otmp2;
}

static const char NEARDATA callable[] = {
	SCROLL_SYM, POTION_SYM, WAND_SYM, RING_SYM, AMULET_SYM, GEM_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	ARMOR_SYM, TOOL_SYM, 0 };

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
	} else
		pline("Call %s: ", an(xname(&otemp)));
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033')
		return;

	/* clear old name */
	str1 = &(objects[obj->otyp].oc_uname);
	if(*str1) free((genericptr_t)*str1);

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

#endif /*OVLB*/
#ifdef OVL0

static const char *ghostnames[] = {
	/* these names should have length < PL_NSIZ */
	/* Capitalize the names for aesthetics -dgk */
	"Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
	"Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
	"Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Robert",
	"Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Havok",
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
	} else if(mtmp->mnamelth && !vb && !Hallucination) {
		if(isinvis) {
		    Strcpy(buf, "the invisible ");
		    Strcat(buf, NAME(mtmp));
		} else 
		    Strcpy(buf, NAME(mtmp));
		return(buf);
	}

	switch(mtmp->data->mlet) {
	    case S_GHOST:
		{ register const char *gn = (const char *) mtmp->mextra;
		  if(!*gn) {		/* might also look in scorefile */
		    gn = ghostnames[rn2(SIZE(ghostnames))];
		    Strcpy((char *) mtmp->mextra, !rn2(5) ? (const char *)plname : gn);
		  }
		  if (Hallucination) {
		    Strcat(buf, "the ");
		    Strcat(buf, rndmonnam());
		  }
		  else
		    Sprintf(buf, "%s's ghost", (char *) mtmp->mextra);
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
	if(vb && mtmp->mnamelth && !Hallucination) {
		Strcat(buf, " called ");
		Strcat(buf, NAME(mtmp));
	}
	return(buf);
}

#endif /* OVL0 */
#ifdef OVLB

char *
lmonnam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, 1));
}

#endif /* OVLB */
#ifdef OVL0

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

#endif /* OVL0 */
#ifdef OVLB

char *
a_monnam(mtmp,adj)
register struct monst *mtmp;
register const char *adj;
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
register const char *adj;
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
register const char *adj;
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

const char *
rndmonnam() {  /* Random name of monster type, if hallucinating */
	int name;

	do {
		name = rn2(PM_ARCHEOLOGIST);
		/* archeologist: first player class */
	} while(type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN));
	return(mons[name].mname);
}

const char *pronoun_pairs[][2] = {
	{"him", "her"}, {"Him", "Her"}, {"his", "her"}, {"His", "Her"},
	{"he", "she"}, {"He", "She"},
	{0, 0}
};

char *
self_pronoun(str, pronoun)
const char *str;
const char *pronoun;
{
	static char NEARDATA buf[BUFSZ];
	register int i;

	for(i=0; pronoun_pairs[i][0]; i++) {
		if(!strncmp(pronoun, pronoun_pairs[i][0], 3)) {
			Sprintf(buf, str, pronoun_pairs[i][flags.female]);
			return buf;
		}
	}
	impossible("never heard of pronoun %s?", pronoun);
	Sprintf(buf, str, pronoun_pairs[i][0]);
	return buf;
}

#ifdef REINCARNATION
const char *
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

#endif /* OVLB */
