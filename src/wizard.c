/*	SCCS Id: @(#)wizard.c	3.1	92/11/13		  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* wizard code - inspired by rogue code from Merlyn Leroy (digi-g!brian) */
/*	       - heavily modified to give the wiz balls.  (genat!mike)   */
/*	       - dewimped and given some maledictions. -3. */
/*	       - generalized for 3.1 (mike@bullns.on01.bull.ca) */

#include "hack.h"
#ifdef MULDGN
#include "qtext.h"
#endif

#ifdef OVLB

static short FDECL(which_arti, (UCHAR_P));
static boolean FDECL(mon_has_arti, (struct monst *,SHORT_P));
static struct monst *FDECL(other_mon_has_arti, (struct monst *,SHORT_P));
static struct obj *FDECL(on_ground, (SHORT_P));
static boolean FDECL(you_have, (UCHAR_P));
static long FDECL(target_on, (UCHAR_P,struct monst *));
static long FDECL(strategy, (struct monst *));

/*	TODO:	Expand this list.	*/
static const int NEARDATA nasties[] = {
	PM_COCKATRICE, PM_ETTIN, PM_STALKER, PM_MINOTAUR, PM_RED_DRAGON,
	PM_GREEN_DRAGON, PM_OWLBEAR, PM_PURPLE_WORM, PM_ROCK_TROLL, PM_XAN,
	PM_GREMLIN, PM_UMBER_HULK, PM_VAMPIRE_LORD, PM_XORN, PM_ZRUTY,
	PM_ELF_LORD, PM_ELVENKING, PM_YELLOW_DRAGON, PM_LEOCROTTA,
	PM_CARNIVOROUS_APE, PM_FIRE_GIANT, PM_COUATL,
#ifdef ARMY
	PM_CAPTAIN,
#endif
	};

static const unsigned NEARDATA wizapp[] = {
	PM_HUMAN, PM_WATER_DEMON, PM_VAMPIRE,
	PM_RED_DRAGON, PM_TROLL, PM_UMBER_HULK,
	PM_XORN, PM_XAN, PM_COCKATRICE,
	PM_FLOATING_EYE,
	PM_GUARDIAN_NAGA,
	PM_TRAPPER
};

#endif /* OVLB */
#ifdef OVL0

/* If you've found the Amulet, make the Wizard appear after some time */
/* Also, give hints about portal locations, if amulet is worn/wielded -dlc */
void
amulet()
{
	register struct monst *mtmp;
	struct obj *amu;

	if ((((amu = uamul) && uamul->otyp == AMULET_OF_YENDOR) ||
	     ((amu = uwep) && uwep->otyp == AMULET_OF_YENDOR)) && !rn2(15)) {
	    register struct trap *ttmp;
	    for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
		if(ttmp->ttyp == MAGIC_PORTAL) {
		    int du = distu(ttmp->tx, ttmp->ty);
		    if (du <= 9)
			pline("%s feels hot!", The(xname(amu)));
		    else if (du <= 64)
			pline("%s feels very warm.", The(xname(amu)));
		    else if (du <= 144)
			pline("%s feels warm.", The(xname(amu)));
		    /* else, the amulet feels normal */
		    break;
		}
	    }
	}

	if (!flags.no_of_wizards || !u.uhave.amulet)
		return;
	/* find Wizard, and wake him if necessary */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->iswiz && mtmp->msleep && !rn2(40)) {
		mtmp->msleep = 0;
		if (distu(mtmp->mx,mtmp->my) > 2)
		    You(
    "get the creepy feeling that somebody noticed your taking the Amulet."
		    );
		return;
	    }
}

#endif /* OVL0 */
#ifdef OVLB

int
mon_has_amulet(mtmp)
register struct monst *mtmp;
{
	register struct obj *otmp;

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == AMULET_OF_YENDOR) return(1);
	return(0);
}

int
mon_has_special(mtmp)
register struct monst *mtmp;
{
	register struct obj *otmp;

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == AMULET_OF_YENDOR ||
#ifdef MULDGN
			is_quest_artifact(otmp) ||
#endif
			otmp->otyp == BELL_OF_OPENING ||
			otmp->otyp == CANDELABRUM_OF_INVOCATION ||
			otmp->otyp == SPE_BOOK_OF_THE_DEAD) return(1);
	return(0);
}

/*
 *	New for 3.1  Strategy / Tactics for the wiz, as well as other
 *	monsters that are "after" something (defined via mflag3).
 *
 *	The strategy section decides *what* the monster is going
 *	to attempt, the tactics section implements the decision.
 */
#define STRAT(w, x, y, typ) (w | ((long)(x)<<16) | ((long)(y)<<8) | (long)typ)
#define ST_NONE	0L
#define	ST_HEAL	-1L
#define ST_GROUND 0x04000000
#define ST_MONSTR 0x02000000
#define ST_PLAYER 0x01000000

#define	M_Wants(mask)	(mtmp->data->mflags3 & (mask))

static short
which_arti(mask)
	register uchar mask;
{
	switch(mask) {
	    case M3_WANTSAMUL:	return(AMULET_OF_YENDOR);
	    case M3_WANTSBELL:	return(BELL_OF_OPENING);
	    case M3_WANTSCAND:	return(CANDELABRUM_OF_INVOCATION);
	    case M3_WANTSBOOK:	return(SPE_BOOK_OF_THE_DEAD);
	    default:		break;	/* 0 signifies quest artifact */
	}
	return(0);
}

/*
 *	If "otyp" is zero, it triggers a check for the quest_artifact,
 *	since bell, book, candle, and amulet are all objects, not really
 *	artifacts right now.	[MRS]
 */
static boolean
mon_has_arti(mtmp, otyp)
	register struct monst *mtmp;
	register short	otyp;
{
	register struct obj *otmp;

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
	    if(otyp) {
		if(otmp->otyp == otyp)
			return(1);
	    }
#ifdef MULDGN
	     else if(is_quest_artifact(otmp)) return(1);
#endif
	}
	return(0);

}

static struct monst *
other_mon_has_arti(mtmp, otyp)
	register struct monst *mtmp;
	register short	otyp;
{
	register struct monst *mtmp2;

	for(mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon)
	    if(mtmp2 != mtmp)
		if(mon_has_arti(mtmp2, otyp)) return(mtmp2);

	return((struct monst *)0);
}

static struct obj *
on_ground(otyp)
	register short	otyp;
{
	register struct obj *otmp;

	for(otmp = fobj; otmp; otmp = otmp->nobj)
	    if(otyp) {
		if(otmp->otyp == otyp)
		    return(otmp);
	    }
#ifdef MULDGN
	     else if(is_quest_artifact(otmp)) return(otmp);
#endif
	return((struct obj *)0);
}

static boolean
you_have(mask)
	register uchar	mask;
{
	switch(mask) {
	    case M3_WANTSAMUL:	return(u.uhave.amulet);
	    case M3_WANTSBELL:	return(u.uhave.bell);
	    case M3_WANTSCAND:	return(u.uhave.menorah);
	    case M3_WANTSBOOK:	return(u.uhave.book);
#ifdef MULDGN
	    case M3_WANTSARTI:	return(u.uhave.questart);
#endif
	    default:		break;
	}
	return(0);
}

static long
target_on(mask, mtmp)
	register uchar  mask;
	register struct monst *mtmp;
{
	register short	otyp;
	register struct obj *otmp;
	register struct monst *mtmp2;

	if(!M_Wants(mask))	return(ST_NONE);

	otyp = which_arti(mask);
	if(!mon_has_arti(mtmp, otyp)) {
	    if(you_have(mask))
		return(STRAT(ST_PLAYER, u.ux, u.uy, mask));
	    else if((otmp = on_ground(otyp)))
		return(STRAT(ST_GROUND, otmp->ox, otmp->oy, mask));
	    else if((mtmp2 = other_mon_has_arti(mtmp, otyp)))
		return(STRAT(ST_MONSTR, mtmp2->mx, mtmp2->my, mask));
	}
	return(ST_NONE);
}

static long
strategy(mtmp)
	register struct monst *mtmp;
{
	long strat, dstrat;

	if(!is_covetous(mtmp->data)) return(ST_NONE);

	switch((mtmp->mhp*3)/mtmp->mhpmax) {	/* 0-3 */

	   default:
	    case 0:	/* panic time - mtmp is almost snuffed */
			return(ST_HEAL);

	    case 1:	/* the wiz is less cautious */
			if(mtmp->data != &mons[PM_WIZARD_OF_YENDOR])
			    return(ST_HEAL);
			/* else fall through */

	    case 2:	dstrat = ST_HEAL;
			break;

	    case 3:	dstrat = ST_NONE;
			break;
	}

	if(flags.made_amulet)
	    if((strat = target_on(M3_WANTSAMUL, mtmp)) != ST_NONE)
		return(strat);

	if(u.uevent.invoked) {		/* priorities change once gate opened */

#ifdef MULDGN
	    if((strat = target_on(M3_WANTSARTI, mtmp)) != ST_NONE)
		return(strat);
#endif
	    if((strat = target_on(M3_WANTSBOOK, mtmp)) != ST_NONE)
		return(strat);
	    if((strat = target_on(M3_WANTSBELL, mtmp)) != ST_NONE)
		return(strat);
	    if((strat = target_on(M3_WANTSCAND, mtmp)) != ST_NONE)
		return(strat);
	} else {

	    if((strat = target_on(M3_WANTSBOOK, mtmp)) != ST_NONE)
		return(strat);
	    if((strat = target_on(M3_WANTSBELL, mtmp)) != ST_NONE)
		return(strat);
	    if((strat = target_on(M3_WANTSCAND, mtmp)) != ST_NONE)
		return(strat);
#ifdef MULDGN
	    if((strat = target_on(M3_WANTSARTI, mtmp)) != ST_NONE)
		return(strat);
#endif
	}
	return(dstrat);
}

int
tactics(mtmp)
	register struct monst *mtmp;
{
	mtmp->mstrategy = strategy(mtmp);

	switch (mtmp->mstrategy) {
	    case ST_HEAL:	/* hide and recover */
		/* if wounded, hole up on or near the stairs (to block them) */
		/* unless, of course, there are no stairs (e.g. endlevel */
		if((xupstair || yupstair))
		    if (mtmp->mx != xupstair || mtmp->my != yupstair)
			(void) mnearto(mtmp, xupstair, yupstair, TRUE);

		/* if you're not around, cast healing spells */
		if (distu(mtmp->mx,mtmp->my) > (BOLT_LIM * BOLT_LIM))
		    if(mtmp->mhp <= mtmp->mhpmax - 8) {
			mtmp->mhp += rnd(8);
			return(1);
		    }
		/* fall through :-) */

	    case ST_NONE:	/* harrass */
	        if(!rn2(5)) mnexto(mtmp);
		return(0);

	    default:		/* kill, maim, pillage! */
	    {
		long  where = (mtmp->mstrategy & 0xff000000);
		xchar tx = (xchar)((mtmp->mstrategy >> 16) & 0xff),
		      ty = (xchar)((mtmp->mstrategy >> 8) & 0xff);
		uchar targ = (xchar)(mtmp->mstrategy & 0xff);
		struct obj *otmp;

		if(!targ) { /* simply wants you to close */
		    return(0);
		}
		if((u.ux == tx && u.uy == ty) || where == ST_PLAYER) {
		    /* player is standing on it (or has it) */
		    mnexto(mtmp);
		    return(0);
		}
		if(where == ST_GROUND) {
		  if(!MON_AT(tx, ty) || (mtmp->mx == tx && mtmp->my == ty)) {

		    /* teleport to it and pick it up */
		    rloc_to(mtmp, tx, ty); 	/* clean old pos */

		    if((otmp = on_ground(which_arti(targ)))) {

			if (cansee(mtmp->mx, mtmp->my))
			    pline("%s picks up %s.",
				  Monnam(mtmp), the(xname(otmp)));
			freeobj(otmp);
			mpickobj(mtmp, otmp);
			return(1);
		    } else return(0);
		  }
	        } else { /* a monster has it - 'port beside it. */
		    (void) mnearto(mtmp, tx, ty, TRUE);
		    return(0);
		}
	    }
	}
	/* NOTREACHED */
	return(0);
}

void
aggravate()
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		mtmp->msleep = 0;
		if(!mtmp->mcanmove && !rn2(5)) {
			mtmp->mfrozen = 0;
			mtmp->mcanmove = 1;
		}
	}
}

void
clonewiz()
{
	register struct monst *mtmp2;

	if(mtmp2 = makemon(&mons[PM_WIZARD_OF_YENDOR], u.ux, u.uy)) {
		mtmp2->msleep = mtmp2->mtame = mtmp2->mpeaceful = 0;
		if (!u.uhave.amulet && rn2(2)) {  /* give clone a fake */
			mtmp2->minvent = mksobj(FAKE_AMULET_OF_YENDOR, TRUE, FALSE);
		}
		mtmp2->m_ap_type = M_AP_MONSTER;
		mtmp2->mappearance = wizapp[rn2(SIZE(wizapp))];
		newsym(mtmp2->mx,mtmp2->my);
	}
}

/* create some nasty monsters, aligned or neutral with the caster */
/* a null caster defaults to a chaotic caster (e.g. the wizard) */
void
nasty(mcast)
	struct monst *mcast;
{
    register struct monst	*mtmp;
    register int	i, j, tmp;
    int castalign = (mcast ? mcast->data->maligntyp : -1);

    if(!rn2(10) && Inhell) msummon(&mons[PM_WIZARD_OF_YENDOR]);
    else {
	tmp = (u.ulevel > 3) ? u.ulevel/3 : 1; /* just in case -- rph */

	for(i = rnd(tmp); i > 0; --i)
	    for(j=0; j<20; j++) {
		if((mtmp = makemon(&mons[nasties[rn2(SIZE(nasties))]],
				   u.ux, u.uy))) {
		    mtmp->msleep = mtmp->mpeaceful = mtmp->mtame = 0;
		    set_malign(mtmp);
		} else /* GENOD? */
		    mtmp = makemon((struct permonst *)0, u.ux, u.uy);
		if(mtmp->data->maligntyp == 0 ||
		   sgn(mtmp->data->maligntyp) == sgn(castalign))
		    break;
	    }
    }
    return;
}

/*	Let's resurrect the wizard, for some unexpected fun.	*/
void
resurrect()
{
	register struct monst	*mtmp;

	if(mtmp = makemon(&mons[PM_WIZARD_OF_YENDOR], u.ux, u.uy)) {
		mtmp->msleep = mtmp->mtame = mtmp->mpeaceful = 0;
		set_malign(mtmp);
		pline("A voice booms out...");
		verbalize("So thou thought thou couldst kill me, fool.");
	}

}

/*	Here, we make trouble for the poor shmuck who actually	*/
/*	managed to do in the Wizard.				*/
void
intervene() {

	switch(rn2(6)) {

	    case 0:
	    case 1:	You("feel vaguely nervous.");
			break;
	    case 2:	if (!Blind)
			    You("notice a %s glow surrounding you.",
				  Hallucination ? hcolor() : Black);
			rndcurse();
			break;
	    case 3:	aggravate();
			break;
	    case 4:	nasty((struct monst *)0);
			break;
	    case 5:	if (!flags.no_of_wizards) resurrect();
			break;
	}
}

void
wizdead(mtmp)
register struct monst	*mtmp;
{
	flags.no_of_wizards--;
	if (!u.uevent.udemigod) {
		u.uevent.udemigod = TRUE;
		u.udg_cnt = rn1(250, 50);

		/* Make the wizard meaner the next time he appears */
		mtmp->data->mlevel++;
		mtmp->data->ac--;
	} else  
		mtmp->data->mlevel++;
}

const char *random_insult[] = {
	"antic",
	"blackguard",
	"caitiff",
	"chucklehead",
	"coistrel",
	"craven",
	"cretin",
	"cur",
	"dastard",
	"demon fodder",
	"dimwit",
	"dolt",
	"fool",
	"footpad",
	"imbecile",
	"knave",
	"maledict",
	"miscreant",
	"niddering",
	"poltroon",
	"rattlepate",
	"reprobate",
	"scapegrace",
	"varlet",
	"villein",	/* (sic.) */
	"wittol",
	"worm",
	"wretch",
};

const char *random_malediction[] = {
	"Hell shall soon claim thy remains,",
	"I chortle at thee, thou pathetic",
	"Prepare to die, thou",
	"Resistance is useless,",
	"Surrender or die, thou",
	"There shall be no mercy, thou",
	"Thou shalt repent of thy cunning,",
	"Thou art as a flea to me,",
	"Thou art doomed,",
	"Thy fate is sealed,",
	"Verily, thou shalt be one dead"
};

#ifdef SOUNDS
# ifndef MULDGN
/* Any %s will be filled in by the appropriate diety's name */
const char *angelic_malediction[] = {
	"Repent, and thou shalt be saved!",
	"Thou shalt pay for thine insolence!",
	"Very soon, my child, thou shalt meet thy maker.",
	"%s has sent me to make you pay for your sins!",
	"The wrath of %s is now upon you!",
	"Thy life belongs to %s now!",
	"Dost thou wish to receive thy final blessing?",
	"Thou art but a godless void.",
	"Thou art not worthy to seek the Amulet.",
	"No one expects the Spanish Inquisition!",
};

const char *demonic_malediction[] = {
	"I first mistook thee for a statue, when I regarded thy head of stone.",
	"Come here often?",
	"Dost pain excite thee?  Wouldst thou prefer the whip?",
	"Thinkest thou it shall tickle as I rip out thy lungs?",
	"Eat slime and die!",
	"Go ahead, fetch thy mama!  I shall wait.",
	"Go play leapfrog with a herd of unicorns!",
	"Hast thou been drinking, or art thou always so clumsy?",
	"This time I shall let thee off with a spanking, but let it not happen again.",
	"I've met smarter (and prettier) acid blobs.",
	"Look!  Thy bootlace is undone!",
	"Mercy!  Dost thou wish me to die of laughter?",
	"Run away!  Live to flee another day!",	
	"Thou hadst best fight better than thou canst dress!",
	"Twixt thy cousin and thee, Medusa is the prettier.",
	"Methinks thou wert unnaturally interested in yon corpse back there, eh, varlet?",
	"Up thy nose with a rubber hose!",
	"Verily, thy corpse could not smell worse!",
	"Wait!  I shall polymorph into a grid bug to give thee a fighting chance!",
	"Why search for the Amulet?  Thou wouldst but lose it, cretin.",
};
# endif /* MULDGN */
#endif

/* Insult or intimidate the player */
void
cuss(mtmp)
register struct monst	*mtmp;
{
#ifdef SOUNDS
	if (mtmp->iswiz) {
#endif
	    if (!rn2(5))  /* typical bad guy action */
		pline("%s laughs fiendishly.", Monnam(mtmp));
	    else 
		if (u.uhave.amulet && !rn2(SIZE(random_insult)))
		    verbalize("Relinquish the amulet, %s!",
			  random_insult[rn2(SIZE(random_insult))]);
		else if (u.uhp < 5 && !rn2(2))	/* Panic */
		    verbalize(rn2(2) ?
			  "Even now thy life force ebbs, %s!" :
			  "Savor thy breath, %s, it be thine last!",
			  random_insult[rn2(SIZE(random_insult))]);
		else if (mtmp->mhp < 5 && !rn2(2))	/* Parthian shot */
		    verbalize(rn2(2) ?
			      "I shall return." :
			      "I'll be back.");
		else
		    verbalize("%s %s!",
			  random_malediction[rn2(SIZE(random_malediction))],
			  random_insult[rn2(SIZE(random_insult))]);
#ifdef SOUNDS
	} else if(is_lminion(mtmp->data)) {
#ifndef MULDGN
		verbalize(angelic_malediction[rn2(SIZE(angelic_malediction) - 1
						  + (Hallucination ? 1 : 0))],
			  align_gname(A_LAWFUL));
#else
		com_pager(rn2(QTN_ANGELIC - 1 + (Hallucination ? 1 : 0)) +
			      QT_ANGELIC);
#endif
	} else {
	    if (!rn2(5))
		pline("%s casts aspersions on your ancestry.", Monnam(mtmp));
	    else
#ifndef MULDGN
		verbalize(demonic_malediction[rn2(SIZE(demonic_malediction))]);
#else
	        com_pager(rn2(QTN_DEMONIC) + QT_DEMONIC);
#endif
	}
#endif
}

#endif /* OVLB */

/*wizard.c*/
