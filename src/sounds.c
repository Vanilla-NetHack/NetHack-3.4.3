/*	SCCS Id: @(#)sounds.c	3.0	88/06/19 */
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright (c) 1989 Janet Walz, Mike Threepoint */

/* block some unused #defines to avoid overloading some cpp's */

#define ONAMES_H
#include "hack.h"
#include "edog.h"

void
verbalize(str)
register char *str;
{
	if(flags.soundok) pline("\"%s\"", str);
}

#ifdef SOUNDS

void
dosounds()
{
    register xchar hallu;
    register struct mkroom *sroom;
    register xchar roomtype;
    register int croomno;

    hallu = Hallucination ? 1 : 0;

    if(!flags.soundok || u.uswallow) return;

    if (fountsound && !rn2(400))
	switch (rn2(3)+hallu) {
	    case 0:
		You("hear bubbling water.");
		break;
	    case 1:
		You("hear water falling on coins...");
		break;
	    case 2:
		You("hear the splashing of a naiad.");
		break;
	    case 3:
		You("seem to hear a soda fountain!");
		break;
	}
    if (sinksound && !rn2(300))
	switch (rn2(2)+hallu) {
	    case 0:
		You("hear a slow drip.");
		break;
	    case 1:
		You("hear a gurgling noise.");
		break;
	    case 2:
		You("seem to hear dishes being washed!");
		break;
	}
    if (!rn2(300)) {
	roomtype = OROOM;
	for (sroom = &rooms[0]; ; sroom++) {	/* find any special room */
	    if (sroom->hx < 0) break;		/* no more rooms */
	    if (sroom->rtype != OROOM) {
		if (sroom->rtype < SHOPBASE)
		    roomtype = sroom->rtype;
		else {
		    croomno = inroom(u.ux,u.uy);
		    if (croomno == -1 || sroom != &rooms[croomno])
			/* player not presently in shop */
			/* other special room types disappear when player
			   enters */
			roomtype = SHOPBASE;
		}
		break;
	    }
	}
	switch (roomtype) {
#ifdef THRONES
	    case COURT:
		switch (rn2(3)+hallu) {
		    case 0:
			You("hear the tones of courtly conversation.");
			break;
		    case 1:
			You("hear a sceptre being pounded in judgement.");
			break;
		    case 2:
			pline("Someone just shouted \"Off with %s head!\"",
			    flags.female ? "her" : "his");
			break;
		    case 3:
			You("seem to hear Queen Beruthiel's cats!");
			break;
		}
		break;
#endif
	    case SWAMP:
		switch (rn2(2)+hallu) {
		    case 0:
			You("hear mosquitoes!");
			break;
		    case 1:
			You("smell marsh gas!");	/* so it's a smell...*/
			break;
		    case 2:
			You("seem to hear Donald Duck.");
			break;
		}
		break;
	    case VAULT:
		switch (rn2(2)+hallu) {
		    case 0:
			You("hear someone counting money.");
			break;
		    case 1:
			You("hear the footsteps of a guard on patrol.");
			break;
		    case 2:
			You("seem to hear Ebenezer Scrooge!");
			break;
		}
		break;
	    case BEEHIVE:
		switch (rn2(2)+hallu) {
		    case 0:
			You("hear a low buzzing.");
			break;
		    case 1:
			You("hear an angry drone.");
			break;
		    case 2:
			You("seem to hear bees in your %shelmet!",
			    uarmh ? "" : "(nonexistent) ");
			break;
		}
		break;
	    case MORGUE:
		switch (rn2(2)+hallu) {
		    case 0:
		    You("suddenly realize it is unnaturally quiet.");
			break;
		    case 1:
			pline("The hair on the back of your %s stands up.",
				body_part(NECK));
			break;
		    case 2:
			pline("The hair on your %s seems to stand up.",
				body_part(HEAD));
			break;
		}
		break;
	    case BARRACKS:
		switch (rn2(3)+hallu) {
		    case 0:
			You("hear dice being thrown.");
			break;
		    case 1:
			You("hear blades being honed.");
			break;
		    case 2:
			You("hear loud snoring.");
			break;
		    case 3:
			You("seem to hear General MacArthur!");
			break;
		}
		break;
	    case ZOO:
		switch (rn2(2)+hallu) {
		    case 0:
You("hear a sound reminding you of an elephant stepping on a peanut.");
			break;
		    case 1:
		    You("hear a sound reminding you of a trained seal.");
			break;
		    case 2:
			You("seem to hear Doctor Doolittle!");
			break;
		}
		break;
	    case SHOPBASE:
		switch (rn2(2)+hallu) {
		    case 0:
			You("hear the chime of a cash register.");
			break;
		    case 1:
			You("hear someone cursing shoplifters.");
			break;
		    case 2:
			You("seem to hear Neiman and Marcus arguing!");
			break;
		}
		break;
	    default:
		break;
	}
    }
}


#include "eshk.h"

#define NOTANGRY(mon)	mon->mpeaceful
#define ANGRY(mon)	!NOTANGRY(mon)

void
growl(mtmp)
register struct monst *mtmp;
{
    /* presumably nearness and soundok checks have already been made */
    switch (mtmp->data->msound) {
	case MS_SILENT:
	    break;
	case MS_MEW:
	case MS_HISS:
	    pline("%s hisses!", Monnam(mtmp));
	    break;
	case MS_BARK:
	case MS_GROWL:
	    pline("%s growls!", Monnam(mtmp));
	    break;
	case MS_ROAR:
	    pline("%s roars!", Monnam(mtmp));
	    break;
	case MS_BUZZ:
	    kludge("%s buzzes!", Monnam(mtmp));
	    break;
	case MS_SQEEK:
	    kludge("%s squeals!", Monnam(mtmp));
	    break;
	case MS_SQAWK:
	    kludge("%s screeches!", Monnam(mtmp));
	    break;
	case MS_NEIGH:
	    kludge("%s neighs!", Monnam(mtmp));
	    break;
    }
}

void
yelp(mtmp)
register struct monst *mtmp;
/* the sounds of mistreated pets */
{
    /* presumably nearness and soundok checks have already been made */
    switch (mtmp->data->msound) {
	case MS_MEW:
	    pline("%s yowls!", Monnam(mtmp));
	    break;
	case MS_BARK:
	case MS_GROWL:
	    pline("%s yelps!", Monnam(mtmp));
	    break;
	case MS_ROAR:
	    kludge("%s snarls!", Monnam(mtmp));
	    break;
	case MS_SQEEK:
	    kludge("%s squeals!", Monnam(mtmp));
	    break;
	case MS_SQAWK:
	    kludge("%s screaks!", Monnam(mtmp));
	    break;
    }
}

void
whimper(mtmp)
register struct monst *mtmp;
/* the sounds of distressed pets */
{
    /* presumably nearness and soundok checks have already been made */
    switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_GROWL:
	    pline("%s whimpers.", Monnam(mtmp));
	    break;
	case MS_BARK:
	    pline("%s whines.", Monnam(mtmp));
	    break;
	case MS_SQEEK:
	    kludge("%s squeals.", Monnam(mtmp));
	    break;
    }
}
#endif /* SOUNDS */


static int
domonnoise(mtmp)
register struct monst *mtmp;
{
    /* presumably nearness checks have already been made */
    if (!flags.soundok) return(0);
    switch (mtmp->data->msound) {
#ifdef ORACLE
	case MS_ORACLE:
	    return doconsult(mtmp);
#endif
#if defined(ALTARS) && defined(THEOLOGY)
	case MS_PRIEST:
	    priest_talk(mtmp);
	    break;
#endif
#ifdef SOUNDS
	case MS_SILENT:
	    break;
	case MS_SQEEK:
	    kludge("%s squeaks.", Monnam(mtmp));
	    break;
	case MS_SQAWK:
	    kludge("%s squawks.", Monnam(mtmp));
	    break;
	case MS_MEW:
	    if (mtmp->mtame) {
		if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped || 
		    moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5)
		    kludge("%s yowls.", Monnam(mtmp));
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    kludge("%s purrs.", Monnam(mtmp));
		else
		    kludge("%s mews.", Monnam(mtmp));
	    }
	case MS_HISS:
	    if (!mtmp->mpeaceful && !mtmp->mtame)
		kludge("%s hisses!", Monnam(mtmp));
	    break;
	case MS_BUZZ:
	    if (!mtmp->mpeaceful && !mtmp->mtame)
		kludge("%s buzzes angrily.", Monnam(mtmp));
	    break;
	case MS_GRUNT:
	    kludge("%s grunts.", Monnam(mtmp));
	    break;
	case MS_BARK:
	    if (flags.moonphase == FULL_MOON && night()) {
		kludge("%s howls.", Monnam(mtmp));
		break;
	    } else if (mtmp->mtame || mtmp->mpeaceful) {
		if (mtmp->mtame &&
		    (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
		     moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
		    kludge("%s whines.", Monnam(mtmp));
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    kludge("%s yips.", Monnam(mtmp));
		else
		    kludge("%s barks.", Monnam(mtmp));
		break;
	    }
	case MS_GROWL:
	    if (!mtmp->mpeaceful && !mtmp->mtame)
		kludge("%s growls!", Monnam(mtmp));
	    break;
	case MS_ROAR:
	    if (!mtmp->mpeaceful && !mtmp->mtame)
		kludge("%s roars!", Monnam(mtmp));
	    break;
	case MS_NEIGH:
	    kludge("%s neighs.", Monnam(mtmp));
	    break;
	case MS_WAIL:
	    kludge("%s wails mournfully.", Monnam(mtmp));
	    break;
	case MS_GURGLE:
	    kludge("%s gurgles.", Monnam(mtmp));
	    break;
	case MS_SHRIEK:
	    kludge("%s shrieks.", Monnam(mtmp));
	    aggravate();
	    break;
	case MS_IMITATE:
	    kludge("%s imitates you.", Monnam(mtmp));
	    break;
	case MS_DJINNI:
	    if (mtmp->mtame) verbalize("Thank you for freeing me!");
	    else if (mtmp->mpeaceful) verbalize("I'm free!");
	    else verbalize("This will teach you not to disturb me!");
	    break;
	case MS_MUMBLE:
	    kludge("%s mumbles incomprehensibly.", Monnam(mtmp));
	    break;
	case MS_HUMANOID:
	    /* Generic humanoid behaviour. */
	    if (!mtmp->mpeaceful && !mtmp->mtame) break;
	    if (mtmp->mhp < 10)
		kludge("%s moans.", Monnam(mtmp));
	    else if (mtmp->mflee)
		kludge("%s wants nothing to do with you.", Monnam(mtmp));
	    else if (mtmp->mconf || mtmp->mstun)
		verbalize(!rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?");
	    else if (mtmp->mblinded)
		verbalize("I can't see!");
	    else if (mtmp->mtrapped)
		verbalize("I'm trapped!");
	    else if (mtmp->mhp < mtmp->mhpmax/2)
		kludge("%s asks for a potion of healing.", Monnam(mtmp));
	    /* Specific monster's interests */
	    else if (is_elf(mtmp->data))
		kludge("%s complains about orcs.", Monnam(mtmp));
	    else if (is_dwarf(mtmp->data))
		kludge("%s talks about mining.", Monnam(mtmp));
	    else if (likes_magic(mtmp->data))
		kludge("%s talks about spellcraft.", Monnam(mtmp));
	    else if (carnivorous(mtmp->data))
		kludge("%s discusses what kinds of meat are safe to eat.", Monnam(mtmp));
	    else switch (monsndx(mtmp->data)){
# ifdef TOLKIEN
		case PM_HOBBIT:
		    if (mtmp->mhpmax - mtmp->mhp >= 10)
kludge("%s complains about unpleasant dungeon conditions.", Monnam(mtmp));
		    else
		    	kludge("%s asks you about the One Ring.", Monnam(mtmp));
		    break;
# endif
		case PM_ARCHEOLOGIST:
kludge("%s describes a recent article in \"Spelunker Today\" magazine.", Monnam(mtmp));
		    break;
		default:
		    kludge("%s discusses dungeon exploration.", Monnam(mtmp));
	    }
	    break;
	case MS_SEDUCE:
# ifdef SEDUCE
	    if (mtmp->data->mlet != S_NYMPH &&
		could_seduce(mtmp, &youmonst, (struct attack *)0) == 1) {
			(void) doseduce(mtmp);
			break;
	    }
	    switch ((poly_gender() != is_female(mtmp)) ? rn2(3) : 0) {
# else
	    switch ((poly_gender() == 0) ? rn2(3) : 0) {
# endif
		case 2:
			verbalize("Hello, sailor.");
			break;
		case 1:
			kludge("%s comes on to you.", Monnam(mtmp));
			break;
		default:
			kludge("%s cajoles you.", Monnam(mtmp));
	    }
	    break;
# ifdef KOPS
	case MS_ARREST:
	    if (mtmp->mpeaceful)
		pline("\"Just the facts, %s.\"",
		      flags.female ? "Ma'am" : "Sir");
	    else switch (rn2(3)) {
		case 1:
		    verbalize("Anything you say can be used against you.");
		    break;
		case 2:
		    verbalize("You're under arrest!");
		    break;
		default:
		    verbalize("Stop in the name of the Law!");
	    }
	    break;
# endif
	case MS_LAUGH:
	    switch (rn2(4)) {
		case 1:
		    kludge("%s giggles.", Monnam(mtmp));
		    break;
		case 2:
		    kludge("%s chuckles.", Monnam(mtmp));
		    break;
		case 3:
		    kludge("%s snickers.", Monnam(mtmp));
		    break;
		default:
		    kludge("%s laughs.", Monnam(mtmp));
	    }
	    break;
# ifdef HARD
	case MS_BRIBE:
	    if (mtmp->mpeaceful && !mtmp->mtame) {
		(void) demon_talk(mtmp);
		break;
	    }
# endif
	case MS_JEER:
	    kludge("%s jeers at you.", Monnam(mtmp));
	    break;
	case MS_CUSS:
	    cuss(mtmp);
	    break;
	case MS_GUARD:
	    if (u.ugold)
		verbalize("Please drop that gold and follow me.");
	    else
		verbalize("Please follow me.");
	    break;
	case MS_NURSE:
	    if (uwep)
		verbalize("Put that weapon away before you hurt someone!");
	    else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
		if (pl_character[0] == 'H')
		    verbalize("Doc, I can't help you unless you cooperate.");
		else
		    verbalize("Please undress so I can examine you.");
# ifdef SHIRT
	    else if (uarmu)
		verbalize("Take off your shirt, please.");
# endif
	    else verbalize("Relax, this won't hurt a bit.");
	    break;
	case MS_SELL: /* pitch, pay, total */
	    if (ANGRY(mtmp))
		kludge("%s mentions how much %s dislikes %s customers.",
			ESHK(mtmp)->shknam,
			ESHK(mtmp)->ismale ? "he" : "she",
			ESHK(mtmp)->robbed ? "non-paying" : "rude");
	    else if (ESHK(mtmp)->following)
		if (strncmp(ESHK(mtmp)->customer, plname, PL_NSIZ)) {
		    pline("\"Hello %s!  I was looking for %s.\"",
			    plname, ESHK(mtmp)->customer);
		    ESHK(mtmp)->following = 0;
		} else {
		    pline("\"Hello %s!  Didn't you forget to pay?\"",
			    plname);
		}
	    else if (ESHK(mtmp)->robbed)
		kludge("%s complains about a recent robbery.", ESHK(mtmp)->shknam);
	    else if (ESHK(mtmp)->billct)
		kludge("%s reminds you that you haven't paid yet.", ESHK(mtmp)->shknam);
	    else if (mtmp->mgold < 50)
		kludge("%s complains that business is bad.", ESHK(mtmp)->shknam);
	    else if (mtmp->mgold > 4000)
		kludge("%s says that business is good.", ESHK(mtmp)->shknam);
	    else
		kludge("%s talks about the problem of shoplifters.", ESHK(mtmp)->shknam);
	    break;
# ifdef ARMY
	case MS_SOLDIER:
	    if (!mtmp->mpeaceful)
	    switch (rn2(3)) {
		case 2:
		    verbalize("Resistance is useless!");
		    break;
		case 1:
		    verbalize("You're dog meat!");
		    break;
		default:
		    verbalize("Surrender!");
	    }
	    break;
# endif
#endif /* SOUNDS */
    }
    return(1);
}


int
dotalk()
{
    register struct monst *mtmp;
    register int tx,ty;

    if (u.uswallow) {
	pline("They won't hear you out there.");
	return(0);
    }

    pline("Talk to whom? [in what direction] ");
    (void) getdir(0);

    if (u.dz) {
	pline("They won't hear you %s there.", u.dz < 0 ? "up" : "down");
	return(0);
    }

    if (u.dx == 0 && u.dy == 0) {
/*
 * Let's not include this.  It raises all sorts of questions: can you wear
 * 2 helmets, 2 amulets, 3 pairs of gloves or 6 rings as a marilith,
 * etc...  --KAA
#ifdef POLYSELF
	if (u.umonnum == PM_ETTIN) {
	    You("discover that your other head makes boring conversation.");
	    return(1);
	}
#endif
*/
	pline("Talking to yourself is a bad habit for a dungeoneer.");
	return(0);
    }

    tx = u.ux+u.dx; ty = u.uy+u.dy;
    if ((Blind && !Telepat) || !levl[tx][ty].mmask ||
	    (mtmp = m_at(tx, ty))->mimic || mtmp->mundetected) {
	pline("I see nobody there.");
	return(0);
    }
    if (mtmp->mfroz || mtmp->msleep) {
	kludge("%s seems not to notice you.", Monnam(mtmp));
	return 0;
    }

    return domonnoise(mtmp);
}
