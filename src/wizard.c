/*	SCCS Id: @(#)wizard.c	3.0	88/04/11
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* wizard code - inspired by rogue code from Merlyn Leroy (digi-g!brian) */
/*	       - heavily modified to give the wiz balls.  (genat!mike)   */
/*	       - dewimped and given some maledictions. -3. */

#include "hack.h"

#ifdef HARD
/*	TODO:	Expand this list.	*/
static const int nasties[] = {
	PM_COCKATRICE, PM_ETTIN, PM_STALKER, PM_MINOTAUR, PM_RED_DRAGON,
	PM_GREEN_DRAGON, PM_OWLBEAR, PM_PURPLE_WORM, PM_ROCK_TROLL, PM_XAN,
	PM_GREMLIN, PM_UMBER_HULK, PM_VAMPIRE_LORD, PM_XORN, PM_ZRUTY,
#ifdef ARMY
	PM_CAPTAIN,
#endif
	};
#endif /* HARD */

/*	TODO:	investigate this. */
static const char wizapp[] = {
	S_HUMAN, S_DEMON, S_VAMPIRE, S_DRAGON, S_TROLL, S_UMBER,
	S_XORN, S_XAN, S_COCKATRICE, S_EYE, S_NAGA, S_TRAPPER,
	/* '1' /* Historical reference */ };

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
			    pline(
    "You get the creepy feeling that somebody noticed your taking the Amulet."
			    );
			return;
		    }
}

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
			if(!levl[otmp->ox][otmp->oy].mmask ||
			    (mtmp->mx == otmp->ox && mtmp->my == otmp->oy)) {

			    /* teleport to it and pick it up */
			    levl[mtmp->mx][mtmp->my].mmask = 0;
			    levl[otmp->ox][otmp->oy].mmask = 1;
			    mtmp->mx = otmp->ox;
			    mtmp->my = otmp->oy;
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
	if(mtmp->mhp < 20 + rnd(10))
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
		if(mtmp->mfroz && !rn2(5))
			mtmp->mfroz = 0;
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
	"I chortle at thee, pathetic",
	"Prepare to die,",
	"Resistance is useless,",
	"Surrender or die, thou",
	"There shall be no mercy, yon",
	"Thou shalt repent of thy cunning,",
	"Thou art as a flea to me,",
	"Thou art doomed,",
	"Thy fate is sealed,",
	"Verily, thou shalt be one dead"
/*	"Go play leapfrog with a unicorn,", */
};

/* Insult the player */
void
cuss(mtmp)
register struct monst	*mtmp;
{
	switch (rn2(5)) {
	    case 0: pline("%s casts aspersions on your ancestry.",
			  Monnam(mtmp));
		    break;
	    case 1: pline("%s laughs fiendishly.", /* typical bad guy action */
			  Monnam(mtmp));
		    break;
	    default:
		    if (u.uhave_amulet && !rn2(SIZE(random_insult)))
			pline("\"Relinquish the amulet, %s!\"",
			      random_insult[rn2(SIZE(random_insult))]);
		    else if (u.uhp < 5 && !rn2(2))	/* Panic */
			pline(rn2(2) ?
				"\"Even now thy life force ebbs, %s!\"" :
				"\"Savor thy breath, %s, it be thine last!\"",
			      random_insult[rn2(SIZE(random_insult))]);
		    else if (mtmp->mhp < 5 && !rn2(2))	/* Parthian shot */
			pline(rn2(2) ?
				"\"I shall return.\"" :
				"\"I'll be back.\"");
		    else
			pline("\"%s %s!\"",
			      random_malediction[rn2(SIZE(random_malediction))],
			      random_insult[rn2(SIZE(random_insult))]);
	}
}
