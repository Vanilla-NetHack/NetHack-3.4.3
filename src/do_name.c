/*	SCCS Id: @(#)do_name.c	3.1	92/12/29	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVLB

static void FDECL(do_oname, (struct obj *));

void
getpos(cc,force,goal)
coord	*cc;
boolean force;
const char *goal;
{
    register int cx, cy, i, c;
    int sidx, tx, ty;
    int lastc, lastx, lasty;
    const char *sdp = flags.num_pad ? ndir : sdir;

    if(flags.verbose) pline("(For instructions type a ?)");
    cx = cc->x;
    cy = cc->y;
    lastc = -1;
    lastx = lasty = 0;
#ifdef CLIPPING
    cliparound(cx, cy);
#endif
    curs(WIN_MAP, cx,cy);
    flush_screen(0);
    while((c = nh_poskey(&tx, &ty, &sidx)) != '.') {
        if(c == '\033') {
            cc->x = -10;
	    clear_nhwindow(WIN_MESSAGE);
            return;
        }
	if(c == 0) {
	    /* a mouse click event, just assign and return */
	    cx = tx;
	    cy = ty;
	    break;
	}
	for(i=0; i<8; i++)
	    if (sdp[i] == c) {
		if (1 <= cx + xdir[i] && cx + xdir[i] < COLNO)
		    cx += xdir[i];
		if (0 <= cy + ydir[i] && cy + ydir[i] < ROWNO)
		    cy += ydir[i];
		goto nxtc;
	    } else if (sdp[i] == lowc((char)c)) {
		cx += xdir[i]*8;
		cy += ydir[i]*8;
		if(cx < 1) cx = 1;
		if(cx > COLNO-1) cx = COLNO-1;
		if(cy < 0) cy = 0;
		if(cy > ROWNO-1) cy = ROWNO-1;
		goto nxtc;
	    }

	if(c == '?'){
	    char sbuf[80];
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    Sprintf(sbuf, "Use [%s] to move the cursor to %s.",
		  flags.num_pad ? "2468" : "hjkl", goal);
	    putstr(tmpwin, 0, sbuf);
	    putstr(tmpwin, 0,
		   "Use [HJKL] to move the cursor 8 units at a time.");
	    putstr(tmpwin, 0, "Or enter a background symbol (ex. <).");
	    putstr(tmpwin, 0, "Type a . when you are at the right place.");
	    if(!force)
		putstr(tmpwin, 0, "Type Space or Escape when you're done.");
	    putstr(tmpwin, 0, "");
	    display_nhwindow(tmpwin, TRUE);
	    destroy_nhwindow(tmpwin);
	} else {
	    if (!index(quitchars, c)) {
		for(sidx = 1; sidx < sizeof(showsyms); sidx++)
		    if(defsyms[sidx].sym == c) {
			/* sidx = cmap_to_glyph(sidx); */
			if(sidx != lastc) {
			    lasty = 0;
			    lastx = 1;
			}
			lastc = sidx;
		    loopback:
			for (ty = lasty; ty < ROWNO; ty++) {
			    for (tx = lastx; tx < COLNO; tx++) {
				if ((IS_POOL(levl[tx][ty].typ) ||
				     IS_FURNITURE(levl[tx][ty].typ)) &&
	 defsyms[sidx].sym == defsyms[glyph_to_cmap(levl[tx][ty].glyph)].sym) {
				    cx = tx;
				    lastx = tx+1;
				    cy = ty;
				    lasty = ty;
				    goto nxtc;
				}
			    }
			    lastx = 1;
			}
			if(lasty != 0) {
			    lasty = 0;
			    lastx = 1;
			    goto loopback;
			}
			pline("Can't find dungeon feature '%c'", c);
			goto nxtc;
		    }

		pline("Unknown direction: '%s' (%s).",
		      visctrl((char)c),
		      force ?
		      flags.num_pad ? "use 2468 or ." :
		      "use hjkl or ." :
		      "aborted");
	    }
	    if(force) goto nxtc;
	    pline("Done.");
	    cc->x = -1;
	    cc->y = 0;
	    return;
	}
    nxtc:	;
#ifdef CLIPPING
	cliparound(cx, cy);
#endif
	curs(WIN_MAP,cx,cy);
	flush_screen(0);
    }
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
do_mname()
{
	char buf[BUFSZ];
	coord cc;
	register int cx,cy;
	register struct monst *mtmp;
	register char *curr;
	boolean blank;
	char qbuf[QBUFSZ];

	if (Hallucination) {
		You("would never recognize it anyway.");
		return 0;
	}
	cc.x = u.ux;
	cc.y = u.uy;
	getpos(&cc, FALSE, "the monster you want to name");
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
	mtmp = m_at(cx, cy);
	if (!mtmp || (!sensemon(mtmp) &&
			(!cansee(cx,cy) || mtmp->mundetected
			|| mtmp->m_ap_type == M_AP_FURNITURE
			|| mtmp->m_ap_type == M_AP_OBJECT
			|| (mtmp->minvis && !See_invisible)))) {
		pline("I see no monster there.");
		return(0);
	}
	Sprintf(qbuf, "What do you want to call %s?", x_monnam(mtmp, 0,
		(char *)0, 1));
	getlin(qbuf,buf);
	clear_nhwindow(WIN_MESSAGE);
	if(!*buf || *buf == '\033') return(0);

	/* unnames monster if all spaces */
	for (curr = buf, blank = 1; *curr; blank = (*curr++ == ' '));
	if(blank) *buf = '\0';

 	if(type_is_pname(mtmp->data))
 	    pline("%s doesn't like being called names!", Monnam(mtmp));
	else (void) christen_monst(mtmp, buf);
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
	char buf[BUFSZ], qbuf[QBUFSZ];
	register char *curr;

	Sprintf(qbuf, "What do you want to name %s?", doname(obj));
	getlin(qbuf, buf);
	clear_nhwindow(WIN_MESSAGE);
	if(!*buf || *buf == '\033')	return;

	/* strip trailing spaces; unnames item if all spaces */
	for (curr = eos(buf); curr > buf; )
		if (*--curr == ' ') *curr = '\0'; else break;

	if(obj->oartifact)
		pline("The artifact seems to resist the attempt.");
	else if (restrict_name(obj, buf) || exist_artifact(obj->otyp, buf)) {
		int n = rn2((int)strlen(buf));
		register char c1, c2;

		c1 = lowc(buf[n]);
		do c2 = 'a' + rn2('z'-'a'); while (c1 == c2);
		buf[n] = (buf[n] == c1) ? c2 : highc(c2);  /* keep same case */
		pline("While engraving your %s slips.", body_part(HAND));
		display_nhwindow(WIN_MESSAGE, FALSE);
		You("engrave: \"%s\".",buf);
		(void)oname(obj, buf, 1);
	}
	else
		(void)oname(obj, buf, 1);
}

struct obj *
oname(obj, buf, ininv)
register struct obj *obj;
const char	*buf;
register int ininv;
{
	register struct obj *otmp, *otmp2;
	register int	lth;

	lth = *buf ? strlen(buf)+1 : 0;
	if(lth > 63){
		lth = 63;
	}
	/* if already properly named */
	if(lth == obj->onamelth && (!lth || !strcmp(ONAME(obj),buf)))
		return obj;

	/* If named artifact exists in the game, do not create another.
	 * Also trying to create an artifact shouldn't de-artifact
	 * it (e.g. Excalibur from prayer). In this case the object
	 * will retain its current name. */
	if (obj->oartifact || exist_artifact(obj->otyp, buf))
		return obj;

	otmp2 = newobj(lth);
	*otmp2 = *obj;	/* the cobj pointer is copied to otmp2 */
	otmp2->onamelth = lth;
	artifact_exists(otmp2, buf, TRUE);

#ifdef __GNUC__
	/* Avoid an old compiler bug (always gave empty name otherwise). */
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
	/* obfree(obj, otmp2);	/* now unnecessary: no pointers on bill */
	dealloc_obj(obj);	/* let us hope nobody else saved a pointer */
	return otmp2;
}

static const char NEARDATA callable[] = {
	SCROLL_CLASS, POTION_CLASS, WAND_CLASS, RING_CLASS, AMULET_CLASS,
	GEM_CLASS, SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0 };

int
ddocall()
{
	register struct obj *obj;
#ifdef REDO
	char	ch;
#endif
	char allow_all[2];

	switch(
#ifdef REDO
		ch = 
#endif
		ynq("Name an individual object?")) {
	case 'q':
		break;
	case 'y':
#ifdef REDO
		savech(ch);
#endif
		allow_all[0] = ALL_CLASSES; allow_all[1] = '\0';
		obj = getobj(allow_all, "name");
		if(obj) do_oname(obj);
		break;
	default :
#ifdef REDO
		savech(ch);
#endif
		obj = getobj(callable, "call");
		if (obj) {
			if (!obj->dknown) {
				You("would never recognize another one.");
				return 0;
			}
			docall(obj);
		}
		break;
	}
	return 0;
}

void
docall(obj)
register struct obj *obj;
{
	char buf[BUFSZ], qbuf[QBUFSZ];
	struct obj otemp;
	register char **str1;
	register char *str;
	boolean blank;

	if (!obj->dknown) return; /* probably blind */
	otemp = *obj;
	otemp.quan = 1L;
	otemp.onamelth = 0;
	if (objects[otemp.otyp].oc_class == POTION_CLASS && otemp.corpsenm) {
		/* kludge, meaning it's sink water */
		Sprintf(qbuf,"Call a stream of %s fluid:",
				OBJ_DESCR(objects[otemp.otyp]));
	} else
		Sprintf(qbuf, "Call %s:", an(xname(&otemp)));
	getlin(qbuf, buf);
	clear_nhwindow(WIN_MESSAGE);
	if(!*buf || *buf == '\033')
		return;

	/* clear old name */
	str1 = &(objects[obj->otyp].oc_uname);
	if(*str1) free((genericptr_t)*str1);

	/* uncalls item if all spaces */
	for (str = buf, blank = 1; *str; blank = (*str++ == ' '));
	if(blank) *buf = '\0';
	if (!*buf) {
		if (*str1)	/* had name, so possibly remove from disco[] */
			undiscover_object(obj->otyp),  *str1 = NULL;
	} else {
		*str1 = strcpy((char *) alloc((unsigned)strlen(buf)+1), buf);
		discover_object(obj->otyp, FALSE); /* possibly add to disco[] */
	}
}

#endif /*OVLB*/
#ifdef OVL0

static const char *ghostnames[] = {
	/* these names should have length < PL_NSIZ */
	/* Capitalize the names for aesthetics -dgk */
	"Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
	"Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
	"Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Robert",
	"Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Jiro", "Mizue",
	"Stephan", "Lance Braccus", "Shadowhawk"
};

/* Monster naming functions:
 * x_monnam is the generic monster-naming function.
 * mon_nam: the rust monster  it  the invisible orc  Fido
 * l_monnam:  rust monster    it  invisible orc      dog called fido
 * Monnam:    The rust monster    It  The invisible orc  Fido
 * Adjmonnam: The poor rust monster  It   The poor invisible orc  The poor Fido
 * Amonnam:   A rust monster      It  An invisible orc   Fido
 * a_monnam:  a rust monster      it  an invisible orc   Fido
 */

char *
x_monnam(mtmp, article, adjective, called)
register struct monst *mtmp;
/* Articles:
 * 0: "the" in front of everything except names and "it"
 * 1: "the" in front of everything except "it"; looks bad for names unless you
 *    are also using an adjective.
 * 2: "a" in front of everything except "it".
 * 3: no article at all.
 */
int article, called;
const char *adjective;
{
#ifdef LINT	/* static char buf[BUFSZ]; */
	char buf[BUFSZ];
#else
	static char buf[BUFSZ];
#endif
	char *name = (mtmp->mnamelth && !Hallucination && !mtmp->isshk) ?
	                          NAME(mtmp) : 0;
	int force_the = (!Hallucination && mtmp->data ==
		&mons[PM_WIZARD_OF_YENDOR]);

	buf[0] = '\0';
	if(mtmp->ispriest || mtmp->isminion) {
		name = priestname(mtmp);
		if (article == 3 && !strncmp(name, "the ", 4)) name += 4;
		return name;
	}
	if(!canseemon(mtmp) && !sensemon(mtmp) &&
					!(u.uswallow && mtmp == u.ustuck)) {
	    if(!mtmp->wormno || (mtmp != m_at(bhitpos.x, bhitpos.y)) ||
	       !(cansee(bhitpos.x, bhitpos.y) && mon_visible(mtmp))) {
		Strcpy(buf, "it");
		return (buf);
	    }
	}
	if (mtmp->isshk) {
		Strcpy(buf, shkname(mtmp));
		if (mtmp->data == &mons[PM_SHOPKEEPER] && !mtmp->minvis)
		    return(buf);
		/* For normal shopkeepers, just 'Asidonhopo'.
		 * For unusual ones, 'Asidonhopo the invisible shopkeeper'
		 * or 'Asidonhopo the blue dragon'.
		 */
		Strcat(buf, " ");
	}
	if (force_the ||
	       ((article == 1 || ((!name || called) && article == 0)) &&
		   (Hallucination || !type_is_pname(mtmp->data))))
		Strcat(buf, "the ");
	if (adjective) {
		Strcat(buf, adjective);
		Strcat(buf, " ");
	}
	if (mtmp->minvis)
		Strcat(buf, "invisible ");
	if (name && !called) {
		Strcat(buf, name);
		goto bot_nam;
	}
	if (mtmp->data == &mons[PM_GHOST] && !Hallucination) {
		register const char *gn = (const char *) mtmp->mextra;
		if(!*gn) {		/* might also look in scorefile */
		    gn = ghostnames[rn2(SIZE(ghostnames))];
		    Strcpy((char *) mtmp->mextra, !rn2(5) ? 
			                   (const char *)plname : gn);
		}
		Sprintf(buf, "%s ghost", s_suffix((char *) mtmp->mextra));
	} else {
	        if(Hallucination)
		    Strcat(buf, rndmonnam());
		else {
		    if(is_mplayer(mtmp->data) && !In_endgame(&u.uz)) { 
		        char pbuf[BUFSZ];
	                Strcpy(pbuf, rank_of((unsigned)mtmp->m_lev,
		                              highc(mtmp->data->mname[0]), 
			                      (boolean)mtmp->female));
			Strcat(buf, lcase(pbuf));
		    } else
		        Strcat(buf, mtmp->data->mname);
		}
	}
	if(name) {
		Strcat(buf, " called ");
		Strcat(buf, NAME(mtmp));
	}
bot_nam:
	if (article == 2 && !force_the && (!name || called) &&
	    (Hallucination || !type_is_pname(mtmp->data)))
		return an(buf);
	else
		return(buf);
}

#endif /* OVL0 */
#ifdef OVLB

char *
l_monnam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, 3, (char *)0, 1));
}

#endif /* OVLB */
#ifdef OVL0

char *
mon_nam(mtmp)
register struct monst *mtmp;
{
	return(x_monnam(mtmp, 0, (char *)0, 0));
}

char *
Monnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = mon_nam(mtmp);

	*bp = highc(*bp);
	return(bp);
}

#endif /* OVL0 */
#ifdef OVLB

char *
Adjmonnam(mtmp, adj)
register struct monst *mtmp;
register const char *adj;
{
	register char *bp = x_monnam(mtmp,1,adj,0);

	*bp = highc(*bp);
	return(bp);
}

char *
a_monnam(mtmp)
register struct monst *mtmp;
{
	return x_monnam(mtmp, 2, (char *)0, 0);
}

char *
Amonnam(mtmp)
register struct monst *mtmp;
{
	register char *bp = a_monnam(mtmp);

	*bp = highc(*bp);
	return(bp);
}

static const char NEARDATA *bogusmons[] = {
	"jumbo shrimp", "giant pigmy", "gnu", "killer penguin", 
	"giant cockroach", "giant slug", "maggot", "pterodactyl",
	"tyrannosaurus rex", "basilisk", "beholder", "nightmare",
	"efreeti", "marid", "rot grub", "bookworm", "doppelganger",
	"shadow", "hologram", "jester", "attorney", "sleazoid",
	"killer tomato", "amazon", "robot", "battlemech",
	"rhinovirus", "harpy", "lion-dog", "rat-ant",
						/* misc. */
	"grue", "Christmas-tree monster", "luck sucker", "paskald",
	"brogmoid", "dornbeast",		/* Quendor (Zork, &c.) */
	"Ancient Multi-Hued Dragon", "Evil Iggy",
						/* Moria */
	"emu", "kestrel", "xeroc", "venus flytrap",
						/* Rogue */
	"creeping coins",			/* Wizardry */
	"hydra", "siren",			/* Greek legend */
	"killer bunny",				/* Monty Python */
	"rodent of unusual size",		/* The Princess Bride */
	"Smokey the bear",	/* "Only you can prevent forest fires!" */
	"Luggage",				/* Discworld */
	"Ent", 					/* Lord of the Rings */
	"tangle tree", "nickelpede", "wiggle",	/* Xanth */
	"white rabbit", "snark",		/* Lewis Carroll */
	"pushmi-pullyu",			/* Dr. Doolittle */
	"smurf",				/* The Smurfs */
	"tribble", "Klingon", "Borg",		/* Star Trek */
	"Ewok", 				/* Star Wars */
	"Totoro",				/* Tonari no Totoro */
	"ohmu", 				/* Nausicaa */
	"Godzilla", "King Kong",		/* monster movies */
	"earthquake beast",			/* old L of SH */
	"Invid",				/* Robotech */
	"Terminator",				/* The Terminator */
	"boomer",				/* Bubblegum Crisis */
	"Dalek",				/* Dr. Who ("Exterminate!") */
	"microscopic space fleet", "Ravenous Bugblatter Beast of Traal",
						/* HGttG */
	"teenage mutant ninja turtle",		/* TMNT */
	"samurai rabbit",			/* Usagi Yojimbo */
	"aardvark",				/* Cerebus */
	"Audrey II",				/* Little Shop of Horrors */
	"witch doctor", "one-eyed one-horned flying purple people eater"
						/* 50's rock 'n' roll */
};

const char *
rndmonnam() {  /* Random name of monster type, if hallucinating */
	int name;

	do {
		name = rn2(PM_ARCHEOLOGIST + SIZE(bogusmons));
		/* archeologist: 1 past last valid monster */
	} while(name < PM_ARCHEOLOGIST &&
	    (type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN)));
	if (name >= PM_ARCHEOLOGIST) return bogusmons[name-PM_ARCHEOLOGIST];
	return(mons[name].mname);
}

#ifdef OVL2

static const char NEARDATA *hcolors[] = {
	"ultraviolet", "infrared", "bluish-orange",
	"reddish-green", "dark white", "light black", "sky blue-pink",
	"salty", "sweet", "sour", "bitter",
	"striped", "spiral", "swirly", "plaid", "checkered", "argyle",
	"paisley", "blotchy", "guernsey-spotted", "polka-dotted",
	"square", "round", "triangular",
	"cabernet", "sangria", "fuchsia", "wisteria",
	"lemon-lime", "strawberry-banana", "peppermint",
	"romantic", "incandescent"
};

const char *
hcolor()
{
	return hcolors[rn2(SIZE(hcolors))];
}
#endif /* OVL2 */

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

/*do_name.c*/
