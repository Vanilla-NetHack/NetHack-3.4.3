/*	SCCS Id: @(#)do_name.c	3.2	96/05/05	*/
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
    int cx, cy, i, c;
    int sidx, tx, ty;
    boolean msg_given = TRUE;	/* clear message window by default */
    const char *sdp;
    if(flags.num_pad) sdp = ndir; else sdp = sdir;	/* DICE workaround */

    if (flags.verbose) {
	pline("(For instructions type a ?)");
	msg_given = TRUE;
    }
    cx = cc->x;
    cy = cc->y;
#ifdef CLIPPING
    cliparound(cx, cy);
#endif
    curs(WIN_MAP, cx,cy);
    flush_screen(0);
#ifdef MAC
    lock_mouse_cursor(TRUE);
#endif
    while ((c = nh_poskey(&tx, &ty, &sidx)) != '.') {
	if (c == '\033') {
	    cx = cy = -10;
	    msg_given = TRUE;	/* force clear */
	    break;
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
		char matching[MAXPCHARS];
		int pass, lo_x, lo_y, hi_x, hi_y, k = 0;
		(void)memset((genericptr_t)matching, 0, sizeof matching);
		for (sidx = 1; sidx < MAXPCHARS; sidx++)
		    if (c == defsyms[sidx].sym || c == (int)showsyms[sidx])
			matching[sidx] = (char) ++k;
		if (k) {
		    for (pass = 0; pass <= 1; pass++) {
			/* pass 0: just past current pos to lower right;
			   pass 1: upper left corner to current pos */
			lo_y = (pass == 0) ? cy : 0;
			hi_y = (pass == 0) ? ROWNO - 1 : cy;
			for (ty = lo_y; ty <= hi_y; ty++) {
			    lo_x = (pass == 0 && ty == lo_y) ? cx + 1 : 1;
			    hi_x = (pass == 1 && ty == hi_y) ? cx : COLNO - 1;
			    for (tx = lo_x; tx <= hi_x; tx++) {
				k = levl[tx][ty].glyph;
				if (glyph_is_cmap(k) &&
					matching[glyph_to_cmap(k)]) {
				    cx = tx,  cy = ty;
				    if (msg_given) {
					clear_nhwindow(WIN_MESSAGE);
					msg_given = FALSE;
				    }
				    goto nxtc;
				}
			    }	/* column */
			}	/* row */
		    }		/* pass */
		    pline("Can't find dungeon feature '%c'", c);
		    msg_given = TRUE;
		    goto nxtc;
		} else {
		    pline("Unknown direction: '%s' (%s).",
			  visctrl((char)c),
			  !force ? "aborted" :
			  flags.num_pad ? "use 2468 or ." : "use hjkl or .");
		    msg_given = TRUE;
		} /* k => matching */
	    } /* !quitchars */
	    if (force) goto nxtc;
	    pline("Done.");
	    msg_given = FALSE;	/* suppress clear */
	    cx = -1;
	    cy = 0;
	    break;
	}
    nxtc:	;
#ifdef CLIPPING
	cliparound(cx, cy);
#endif
	curs(WIN_MAP,cx,cy);
	flush_screen(0);
    }
#ifdef MAC
    lock_mouse_cursor(FALSE);
#endif
    if (msg_given) clear_nhwindow(WIN_MESSAGE);
    cc->x = cx;
    cc->y = cy;
    return;
}

struct monst *
christen_monst(mtmp, name)
struct monst *mtmp;
const char *name;
{
	int lth, i;
	struct monst *mtmp2;
	char buf[PL_PSIZ];

	/* dogname & catname are PL_PSIZ arrays; object names have same limit */
	lth = *name ? (int)(strlen(name) + 1) : 0;
	if(lth > PL_PSIZ){
		lth = PL_PSIZ;
		name = strncpy(buf, name, PL_PSIZ - 1);
		buf[PL_PSIZ - 1] = '\0';
	}
	if (lth == mtmp->mnamelth) {
		/* don't need to allocate a new monst struct */
		if (lth) Strcpy(NAME(mtmp), name);
		return mtmp;
	}
	mtmp2 = newmonst(mtmp->mxlth + lth);
	*mtmp2 = *mtmp;
	for(i=0; i<mtmp->mxlth; i++)
		((char *) mtmp2->mextra)[i] = ((char *) mtmp->mextra)[i];
	mtmp2->mnamelth = lth;
	if (lth) Strcpy(NAME(mtmp2), name);
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
	if(!*buf || *buf == '\033') return(0);
	/* strip leading and trailing spaces; unnames monster if all spaces */
	(void)mungspaces(buf);

	if (type_is_pname(mtmp->data))
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
	const char *aname;
	short objtyp;

	Sprintf(qbuf, "What do you want to name %s?", doname(obj));
	getlin(qbuf, buf);
	if(!*buf || *buf == '\033')	return;
	/* strip leading and trailing spaces; unnames item if all spaces */
	(void)mungspaces(buf);

	/* relax restrictions over proper capitalization for artifacts */
	if ((aname = artifact_name(buf, &objtyp)) != 0 && objtyp == obj->otyp)
		Strcpy(buf, aname);

	if (obj->oartifact) {
		pline_The("artifact seems to resist the attempt.");
		return;
	} else if (restrict_name(obj, buf) || exist_artifact(obj->otyp, buf)) {
		int n = rn2((int)strlen(buf));
		register char c1, c2;

		c1 = lowc(buf[n]);
		do c2 = 'a' + rn2('z'-'a'); while (c1 == c2);
		buf[n] = (buf[n] == c1) ? c2 : highc(c2);  /* keep same case */
		pline("While engraving your %s slips.", body_part(HAND));
		display_nhwindow(WIN_MESSAGE, FALSE);
		You("engrave: \"%s\".",buf);
	}
	obj = oname(obj, buf);
	if (obj->where == OBJ_INVENT)
		update_inventory();
}

/*
 * Allocate a new and possibly larger storage space for an obj.
 */
struct obj *
replobj(obj, oextra_size, oextra_src, oname_size, name)
struct obj *obj;
int oextra_size;		/* storage to allocate for oextra            */
genericptr_t oextra_src;
int oname_size;			/* size of name string + 1 (null terminator) */
const char *name;
{
	struct obj *otmp;

	otmp = newobj(oextra_size + oname_size);
	*otmp = *obj;	/* the cobj pointer is copied to otmp */
	if (oextra_size) {
	    if (oextra_src)
		(void) memcpy((genericptr_t)otmp->oextra, oextra_src,
								oextra_size);
	} else {
	    otmp->mtraits = 0;
	}
	otmp->oxlth = oextra_size;
	
	otmp->onamelth = oname_size;
	otmp->timed = 0;	/* not timed, yet */
	otmp->lamplit = 0;	/* ditto */
	/* __GNUC__ note:  if the assignment of otmp->onamelth immediately
	   precedes this `if' statement, a gcc bug will miscompile the
	   test on vax (`insv' instruction used to store bitfield does
	   not set condition codes, but optimizer behaves as if it did).
	   gcc-2.8.0 should finally fix this.... */
	if (oname_size) {
	    if (name)
		Strcpy(ONAME(otmp), name);
	}

	if (obj->owornmask) {
		setworn((struct obj *)0, obj->owornmask);
		setworn(otmp, otmp->owornmask);
	}

	/*
	 * Finish inserting otmp right after obj in whatever chain
	 * it is on.  Then extract obj from the chain(s).  Don't use
	 * obj_extract_self(), we are doing an in-place swap, not
	 * actually moving something.
	 */
	switch (obj->where) {
	    case OBJ_FREE:
		/* do nothing */
		break;
	    case OBJ_INVENT:
		obj->nobj = otmp;
		extract_nobj(obj, &invent);
		break;
	    case OBJ_CONTAINED:
		obj->nobj = otmp;
		extract_nobj(obj, &obj->ocontainer);
		break;
	    case OBJ_MINVENT:
		obj->nobj = otmp;
		extract_nobj(obj, &obj->ocarry->minvent);
		break;
	    case OBJ_FLOOR:
		obj->nobj = otmp;
		obj->nexthere = otmp;
		extract_nobj(obj, &fobj);
		extract_nexthere(obj, &level.objects[obj->ox][obj->oy]);
		break;
	    default:
		panic("oname: obj position");
		break;
	}

	/* fix ocontainer pointers */
	if (Has_contents(obj)) {
		struct obj *inside;

		for(inside = obj->cobj; inside; inside = inside->nobj)
			inside->ocontainer = otmp;
	}

	/* move timers and light sources from obj to otmp */
	if (obj->timed) obj_move_timers(obj, otmp);
	if (obj->lamplit) obj_move_light_source(obj, otmp);

	/* obfree(obj, otmp);	/* now unnecessary: no pointers on bill */
	dealloc_obj(obj);	/* let us hope nobody else saved a pointer */
	return otmp;
}

struct obj *
oname(obj, name)
struct obj *obj;
const char *name;
{
	int lth;
	char buf[PL_PSIZ];

	lth = *name ? (int)(strlen(name) + 1) : 0;
	if (lth > PL_PSIZ) {
		lth = PL_PSIZ;
		name = strncpy(buf, name, PL_PSIZ - 1);
		buf[PL_PSIZ - 1] = '\0';
	}
	/* If named artifact exists in the game, do not create another.
	 * Also trying to create an artifact shouldn't de-artifact
	 * it (e.g. Excalibur from prayer). In this case the object
	 * will retain its current name. */
	if (obj->oartifact || (lth && exist_artifact(obj->otyp, name)))
		return obj;

	if (lth == obj->onamelth) {
		/* no need to replace entire object */
		if (lth) Strcpy(ONAME(obj), name);
	} else {
		obj = replobj(obj, obj->oxlth,
			      (genericptr_t)obj->oextra, lth, name);
	}
	if (lth) artifact_exists(obj, name, TRUE);
	return obj;
}

static NEARDATA const char callable[] = {
	SCROLL_CLASS, POTION_CLASS, WAND_CLASS, RING_CLASS, AMULET_CLASS,
	GEM_CLASS, SPBOOK_CLASS, ARMOR_CLASS, TOOL_CLASS, 0 };

int
ddocall()
{
	register struct obj *obj;
#ifdef REDO
	char	ch;
#endif
	char allowall[2];

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
		allowall[0] = ALL_CLASSES; allowall[1] = '\0';
		obj = getobj(allowall, "name");
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

	if (!obj->dknown) return; /* probably blind */
	otemp = *obj;
	otemp.quan = 1L;
	otemp.onamelth = 0;
#ifdef OEXTRA
	otemp.oxlth = 0;
#endif
	if (objects[otemp.otyp].oc_class == POTION_CLASS && otemp.corpsenm) {
		/* kludge, meaning it's sink water */
		Sprintf(qbuf,"Call a stream of %s fluid:",
				OBJ_DESCR(objects[otemp.otyp]));
	} else
		Sprintf(qbuf, "Call %s:", an(xname(&otemp)));
	getlin(qbuf, buf);
	if(!*buf || *buf == '\033')
		return;

	/* clear old name */
	str1 = &(objects[obj->otyp].oc_uname);
	if(*str1) free((genericptr_t)*str1);

	/* strip leading and trailing spaces; uncalls item if all spaces */
	(void)mungspaces(buf);
	if (!*buf) {
		if (*str1)	/* had name, so possibly remove from disco[] */
			undiscover_object(obj->otyp),  *str1 = (char *)0;
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
 *		  seen	      unseen	   detected		  named
 * mon_nam:	the newt	it	the invisible orc	Fido
 * l_monnam:	newt		it	invisible orc		dog called fido
 * Monnam:	The newt	It	The invisible orc	Fido
 * Adjmonnam:	The poor newt	It	The poor invisible orc	The poor Fido
 * Amonnam:	A newt		It	An invisible orc	Fido
 * a_monnam:	a newt		it	an invisible orc	Fido
 * m_monnam:	newt		xan	orc			Fido
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
 * 4: no article; "it" and invisible suppressed.
 */
int article, called;
const char *adjective;
{
#ifdef LINT	/* static char buf[BUFSZ]; */
	char buf[BUFSZ];
#else
	static char buf[BUFSZ];
#endif
	struct permonst *mdat = mtmp->data;
	char *name = 0;
	int force_the = 0, trunam = (article == 4);

	buf[0] = '\0';
	if (mtmp->ispriest || mtmp->isminion) {
	    name = priestname(mtmp);
	    if ((trunam || article == 3) && !strncmp(name, "the ", 4))
		name += 4;
	    return strcpy(buf, name);
	}
	if (!trunam && !canspotmon(mtmp) &&
		!(u.uswallow && mtmp == u.ustuck) && !killer) {
	    if (!(cansee(bhitpos.x, bhitpos.y) && mon_visible(mtmp))) {
		Strcpy(buf, "it");
		return  buf;
	    }
	}
	if (trunam || !Hallucination) {
	    if (mtmp->isshk) {
		Strcpy(buf, shkname(mtmp));
		if ((mdat == &mons[PM_SHOPKEEPER] && !mtmp->minvis) || trunam)
		    return buf;
		/* For normal shopkeepers, just 'Asidonhopo'.
		 * For unusual ones, 'Asidonhopo the invisible shopkeeper'
		 * or 'Asidonhopo the blue dragon'.
		 */
		Strcat(buf, " ");
	    } else if (mtmp->mnamelth) {
		name = NAME(mtmp);
	    }
	}

	if (!trunam) {
	    force_the = (!Hallucination &&
			 (mdat == &mons[PM_WIZARD_OF_YENDOR] || mtmp->isshk));
	    if (force_the ||
		    ((article == 1 || ((!name || called) && article == 0)) &&
			(Hallucination || !type_is_pname(mdat))))
		Strcat(buf, "the ");
	    if (adjective)
		Strcat(strcat(buf, adjective), " ");
	    if (mtmp->minvis && !Blind)
		Strcat(buf, "invisible ");
	}

	if (name && !called) {
	    Strcat(buf, name);
	    goto bot_nam;
	}

	if (Hallucination && !trunam) {
	    Strcat(buf, rndmonnam());
	} else if (mdat == &mons[PM_GHOST]) {
	    register const char *gn = (const char *) mtmp->mextra;
	    if (!*gn) {
		/* ghost doesn't have a name yet; give it one now
		   (might also want to look in scorefile) */
		gn = rn2(5) ? ghostnames[rn2(SIZE(ghostnames))] :
			      (const char *)plname;
		Strcpy((char *) mtmp->mextra, gn);
	    }
	    Sprintf(buf, "%s ghost", s_suffix((char *) mtmp->mextra));
	} else if (is_mplayer(mdat) && !In_endgame(&u.uz)) {
	    char pbuf[BUFSZ];
	    Strcpy(pbuf, rank_of((int)mtmp->m_lev,
				 highc(mdat->mname[0]),
				 (boolean)mtmp->female));
	    Strcat(buf, lcase(pbuf));
	} else {
	    Strcat(buf, mdat->mname);
	}

	if (name) {	/* if we reach here, `name' implies `called' */
	    Strcat(buf, " called ");
	    Strcat(buf, NAME(mtmp));
	}
 bot_nam:
	if (article == 2 && !force_the && (!name || called) &&
		(Hallucination || !type_is_pname(mdat)))
	    return an(buf);
	else
	    return buf;
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

/* monster's own name */
char *
m_monnam(mtmp)
struct monst *mtmp;
{
	char *result;
	unsigned save_invis = mtmp->minvis;

	mtmp->minvis = 0;  /* affects priestname() as well as x_monnam() */
	result = x_monnam(mtmp, 4, (char *)0, 0);
	mtmp->minvis = save_invis;
	return result;
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

static const char *bogusmons[] = {
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
	"Ent",					/* Lord of the Rings */
	"tangle tree", "nickelpede", "wiggle",	/* Xanth */
	"white rabbit", "snark",		/* Lewis Carroll */
	"pushmi-pullyu",			/* Dr. Doolittle */
	"smurf",				/* The Smurfs */
	"tribble", "Klingon", "Borg",		/* Star Trek */
	"Ewok",					/* Star Wars */
	"Totoro",				/* Tonari no Totoro */
	"ohmu",					/* Nausicaa */
	"youma",				/* Sailor Moon */
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
	"witch doctor", "one-eyed one-horned flying purple people eater",
						/* 50's rock 'n' roll */
	"Barney the dinosaur"			/* saccharine kiddy TV */
};

const char *
rndmonnam()	/* Random name of monster type, if hallucinating */
{
	int name;

	do {
	    name = rn1(SPECIAL_PM + SIZE(bogusmons) - LOW_PM, LOW_PM);
	} while (name < SPECIAL_PM &&
	    (type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN)));

	if (name >= SPECIAL_PM) return bogusmons[name - SPECIAL_PM];
	return mons[name].mname;
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
	static NEARDATA char buf[BUFSZ];
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

	if ((opts = getenv("ROGUEOPTS")) != 0) {
		for (i = opts; *i; i++)
			if (!strncmp("name=",i,5)) {
				char *j;
				if ((j = index(i+5,',')) != 0)
					*j = (char)0;
				return i+5;
			}
	}
	return rn2(3) ? (rn2(2) ? "Michael Toy" : "Kenneth Arnold")
		: "Glenn Wichman";
}
#endif /* REINCARNATION */
#endif /* OVLB */

#ifdef OVL2

static NEARDATA const char *hcolors[] = {
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
hcolor(colorpref)
const char *colorpref;
{
	return (Hallucination || !colorpref) ?
		hcolors[rn2(SIZE(hcolors))] : colorpref;
}
#endif /* OVL2 */

/*do_name.c*/
