/*	SCCS Id: @(#)wizard.c	3.0	90/01/09
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* wizard code - inspired by rogue code from Merlyn Leroy (digi-g!brian) */
/*	       - heavily modified to give the wiz balls.  (genat!mike)   */
/*	       - dewimped and given some maledictions. -3. */

#include "hack.h"

#ifdef OVLB

#ifdef HARD
/*	TODO:	Expand this list.	*/
static const int NEARDATA nasties[] = {
	PM_COCKATRICE, PM_ETTIN, PM_STALKER, PM_MINOTAUR, PM_RED_DRAGON,
	PM_GREEN_DRAGON, PM_OWLBEAR, PM_PURPLE_WORM, PM_ROCK_TROLL, PM_XAN,
	PM_GREMLIN, PM_UMBER_HULK, PM_VAMPIRE_LORD, PM_XORN, PM_ZRUTY,
#ifdef ARMY
	PM_CAPTAIN,
#endif
	};
#endif /* HARD */

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

/* If he has found the Amulet, make the wizard appear after some time */
void
amulet(){
	register struct monst *mtmp;

	if(!flags.made_amulet || !flags.no_of_wizards)
		return;
	/* find wizard, and wake him if necessary */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->iswiz && mtmp->msleep && !rn2(40))
		    if(u.uhave_amulet) {
			mtmp->msleep = 0;
			if(dist(mtmp->mx,mtmp->my) > 2)
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

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj){
		if(otmp->otyp == AMULET_OF_YENDOR && otmp->spe >= 0) return(1);
	}
	return(0);
}

/* the wiz's prime directive */
int
wiz_get_amulet(mtmp)
register struct monst *mtmp;
{
	/* if he doesn't have the amulet */
	if(!mon_has_amulet(mtmp))
	    if(u.uhave_amulet) {

		/* player has it, dog them til he gets it or dies */
		mnexto(mtmp);
	    } else {
		register struct obj *otmp;

		/* if it is lying around someplace, he teleports to it */
		for(otmp = fobj; otmp; otmp = otmp->nobj)
		    if(otmp->otyp == AMULET_OF_YENDOR && !otmp->spe) {
			if(u.ux == otmp->ox && u.uy == otmp->oy) {
			    /* player is standing on it */
			    mnexto(mtmp);
			    return(0);
			}
			if(!MON_AT(otmp->ox, otmp->oy) ||
			    (mtmp->mx == otmp->ox && mtmp->my == otmp->oy)) {

			    /* teleport to it and pick it up */
			    remove_monster(mtmp->mx, mtmp->my);
			    place_monster(mtmp, otmp->ox, otmp->oy);
			    if (cansee(mtmp->mx, mtmp->my))
				pline("%s picks up %s.", Monnam(mtmp),
								xname(otmp));
			    freeobj(otmp);
			    mpickobj(mtmp, otmp);
			    pmon(mtmp);
			    return(1);
			}
			break;
		    }
		/* we don't know where it is */
	    }

	/* he has it or can't find it */
	/* secondary goal - stayin' alive */

	/* if wounded, hole up on or near the stairs (to block them) */
	/* unless, of course, there are no stairs (e.g. endlevel */
	if(mtmp->mhp < 20 + rnd(10) && (xupstair || yupstair))
	    if (mtmp->mx != xupstair && mtmp->my != yupstair)
		mnearto(mtmp,xupstair,yupstair,TRUE);

	/* if you're not around, cast healing spells */
	if(dist(mtmp->mx,mtmp->my) > (BOLT_LIM * BOLT_LIM))
	    if(mtmp->mhp <= mtmp->mhpmax - 8) {
		mtmp->mhp += rnd(8);
		return(1);
	    }
	    /* healthy wiz with nothing to do */
	    else if(!rn2(5))
		mnexto(mtmp);

	/* the effect is that below 30 hp, wily wiz teleports
	   again and again, unless/until he blocks the stairs.

	   if you keep away from the wounded wiz, he sits
	   there healing himself, until he gets healthy
	   and decides to punish you some more.  -3. */

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
		if (!u.uhave_amulet && rn2(2)) {  /* give clone a fake */
			mtmp2->minvent = mksobj(AMULET_OF_YENDOR,FALSE);
			mtmp2->minvent->spe = -1;
		}
		unpmon(mtmp2);
		mtmp2->m_ap_type = M_AP_MONSTER;
		mtmp2->mappearance = wizapp[rn2(SIZE(wizapp))];
		pmon(mtmp2);
	}
}

#ifdef HARD
void
nasty() {
	register struct monst	*mtmp;
	register int	i, tmp;

	if(!rn2(10) && Inhell) dsummon(&mons[PM_WIZARD_OF_YENDOR]);
	else {
	    tmp = (u.ulevel > 3) ? u.ulevel/3 : 1; /* just in case -- rph */

	    for(i = rnd(tmp); i > 0; --i)
		if((mtmp = makemon(&mons[nasties[rn2(SIZE(nasties))]], u.ux, u.uy))) {

		    mtmp->msleep = mtmp->mpeaceful = mtmp->mtame = 0;
		} else
		  (void)makemon((struct permonst *)0, u.ux, u.uy); /* GENOD? */
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
		pline("A voice booms out...");
		pline("\"So thou thought thou couldst kill me, fool.\"");
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
				  Hallucination ? hcolor() : black);
			rndcurse();
			break;
	    case 3:	aggravate();
			break;
	    case 4:	nasty();
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
	if(! u.udemigod)  {

		u.udemigod = TRUE;
		u.udg_cnt = rn1(250, 50);

	/*  Make the wizard meaner the next time he appears  */
		mtmp->data->mlevel++;
		mtmp->data->ac--;
	} else  
		mtmp->data->mlevel++;
}
#endif /* HARD /**/

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
	"Prepare to die, thou ",
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
		if (u.uhave_amulet && !rn2(SIZE(random_insult)))
		    pline("\"Relinquish the amulet, %s!\"",
			  random_insult[rn2(SIZE(random_insult))]);
		else if (u.uhp < 5 && !rn2(2))	/* Panic */
		    pline(rn2(2) ?
			  "\"Even now thy life force ebbs, %s!\"" :
			  "\"Savor thy breath, %s, it be thine last!\"",
			  random_insult[rn2(SIZE(random_insult))]);
		else if (mtmp->mhp < 5 && !rn2(2))	/* Parthian shot */
		    verbalize(rn2(2) ?
			      "I shall return." :
			      "I'll be back.");
		else
		    pline("\"%s %s!\"",
			  random_malediction[rn2(SIZE(random_malediction))],
			  random_insult[rn2(SIZE(random_insult))]);
#ifdef SOUNDS
	} else {
	    if (!rn2(5))
		kludge("%s casts aspersions on your ancestry.", Monnam(mtmp));
	    else
		verbalize(demonic_malediction[rn2(SIZE(demonic_malediction))]);
	}
#endif
}

#endif /* OVLB */
