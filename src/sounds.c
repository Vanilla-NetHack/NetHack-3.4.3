/*	SCCS Id: @(#)sounds.c	3.1	93/02/09
/* 	Copyright (c) 1989 Janet Walz, Mike Threepoint */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

#ifdef OVLB

static int FDECL(domonnoise,(struct monst *));
static int NDECL(dochat);

#endif /* OVLB */

#ifdef SOUNDS

#ifdef OVL0

void
dosounds()
{
    register xchar hallu;
    register struct mkroom *sroom;
    register int vx, vy;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
    int xx;
#endif

    hallu = Hallucination ? 1 : 0;

    if(!flags.soundok || u.uswallow || Underwater) return;

    if (level.flags.nfountains && !rn2(400))
	switch (rn2(3)+hallu) {
	    case 0:
		You("hear bubbling water.");
		break;
	    case 1:
		You("hear water falling on coins.");
		break;
	    case 2:
		You("hear the splashing of a naiad.");
		break;
	    case 3:
		You("hear a soda fountain!");
		break;
	}
    if (level.flags.nsinks && !rn2(300))
	switch (rn2(2)+hallu) {
	    case 0:
		You("hear a slow drip.");
		break;
	    case 1:
		You("hear a gurgling noise.");
		break;
	    case 2:
		You("hear dishes being washed!");
		break;
	}

    if (level.flags.has_court && !rn2(200)) {
	switch (rn2(3)+hallu) {
	    case 0:
		You("hear the tones of courtly conversation.");
		break;
	    case 1:
		You("hear a sceptre pounded in judgment.");
		break;
	    case 2:
		pline("Someone shouts \"Off with %s head!\"",
		      his[flags.female]);
		break;
	    case 3:
		You("hear Queen Beruthiel's cats!");
		break;
	}
	return;
    }
    if (level.flags.has_swamp && !rn2(200)) {
	switch (rn2(2)+hallu) {
	    case 0:
		You("hear mosquitoes!");
		break;
	    case 1:
		You("smell marsh gas!");	/* so it's a smell...*/
		break;
	    case 2:
		You("hear Donald Duck!");
		break;
	}
	return;
    }
    if (level.flags.has_vault && !rn2(200)) {
	if (!(sroom = search_special(VAULT))) {
	    /* strange ... */
	    level.flags.has_vault = 0;
	    return;
	}
	if(gd_sound())
	    switch (rn2(2)+hallu) {
		case 1: {
		    boolean gold_in_vault = FALSE;

		    for (vx = sroom->lx;vx <= sroom->hx; vx++)
			for (vy = sroom->ly; vy <= sroom->hy; vy++)
			    if (g_at(vx, vy))
				gold_in_vault = TRUE;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
		    /* Bug in aztec assembler here. Workaround below */
		    xx = ROOM_INDEX(sroom) + ROOMOFFSET;
		    xx = (xx != vault_occupied(u.urooms));
		    if(xx)
#else
		    if (vault_occupied(u.urooms) != 
			 (ROOM_INDEX(sroom) + ROOMOFFSET))
#endif /* AZTEC_C_WORKAROUND */
		    {
			if (gold_in_vault)
			    You(!hallu ? "hear someone counting money." :
				"hear the quarterback calling the play.");
			else
			    You("hear someone searching.");
			break;
		    }
		    /* fall into... (yes, even for hallucination) */
		}
		case 0:
		    You("hear the footsteps of a guard on patrol.");
		    break;
		case 2:
		    You("hear Ebenezer Scrooge!");
		    break;
	    }
	return;
    }
    if (level.flags.has_beehive && !rn2(200)) {
	switch (rn2(2)+hallu) {
	    case 0:
		You("hear a low buzzing.");
		break;
	    case 1:
		You("hear an angry drone.");
		break;
	    case 2:
		You("hear bees in your %sbonnet!",
		    uarmh ? "" : "(nonexistent) ");
		break;
	}
	return;
    }
    if (level.flags.has_morgue && !rn2(200)) {
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
	return;
    }
#ifdef ARMY
    if (level.flags.has_barracks && !rn2(200)) {
	switch (rn2(3)+hallu) {
	    case 0:
		You("hear blades being honed.");
		break;
	    case 1:
		You("hear loud snoring.");
		break;
	    case 2:
		You("hear dice being thrown.");
		break;
	    case 3:
		You("hear General MacArthur!");
		break;
	}
	return;
    }
#endif /* ARMY */
    if (level.flags.has_zoo && !rn2(200)) {
	switch (rn2(2)+hallu) {
	    case 0:
		You("hear a sound reminiscent of an elephant stepping on a peanut.");
		break;
	    case 1:
		You("hear a sound reminiscent of a seal barking.");
		break;
	    case 2:
		You("hear Doctor Doolittle!");
		break;
	}
	return;
    }
    if (level.flags.has_shop && !rn2(200)) {
	if (!(sroom = search_special(ANY_SHOP))) {
	    /* strange... */
	    level.flags.has_shop = 0;
	    return;
	}
        if(tended_shop(sroom) && 
	   !index(u.ushops, ROOM_INDEX(sroom) + ROOMOFFSET))
	    switch (rn2(2)+hallu) {
		case 0:
		    You("hear someone cursing shoplifters.");
		    break;
		case 1:
		    You("hear the chime of a cash register.");
		    break;
		case 2:
		    You("hear Neiman and Marcus arguing!");
		    break;
	    }
        return;
    }
}

#endif /* OVL0 */
#ifdef OVLB

#include "eshk.h"

void
growl(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleep || !mtmp->mcanmove) return;
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
	    pline("%s buzzes!", Monnam(mtmp));
	    break;
	case MS_SQEEK:
	    pline("%s squeals!", Monnam(mtmp));
	    break;
	case MS_SQAWK:
	    pline("%s screeches!", Monnam(mtmp));
	    break;
	case MS_NEIGH:
	    pline("%s neighs!", Monnam(mtmp));
	    break;
	case MS_WAIL:
	    pline("%s wails!", Monnam(mtmp));
	    break;
    }
}

void
yelp(mtmp)
register struct monst *mtmp;
/* the sounds of mistreated pets */
{
    if (mtmp->msleep || !mtmp->mcanmove) return;
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
	    pline("%s snarls!", Monnam(mtmp));
	    break;
	case MS_SQEEK:
	    pline("%s squeals!", Monnam(mtmp));
	    break;
	case MS_SQAWK:
	    pline("%s screaks!", Monnam(mtmp));
	    break;
	case MS_WAIL:
	    pline("%s wails!", Monnam(mtmp));
	    break;
    }
}

void
whimper(mtmp)
register struct monst *mtmp;
/* the sounds of distressed pets */
{
    if (mtmp->msleep || !mtmp->mcanmove) return;
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
	    pline("%s squeals.", Monnam(mtmp));
	    break;
    }
}

#endif /* OVLB */

#endif /* SOUNDS */

#ifdef OVLB

static int
domonnoise(mtmp)
register struct monst *mtmp;
{
    /* presumably nearness and sleep checks have already been made */
    if (!flags.soundok) return(0);
    switch (mtmp->data->msound) {
	case MS_ORACLE:
	    return doconsult(mtmp);
	case MS_PRIEST:
	    priest_talk(mtmp);
	    break;
#ifdef SOUNDS
	case MS_SILENT:
	    break;
	case MS_SQEEK:
	    pline("%s squeaks.", Monnam(mtmp));
	    break;
	case MS_SQAWK:
	    pline("%s squawks.", Monnam(mtmp));
	    break;
	case MS_MEW:
	    if (mtmp->mtame) {
		if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped || 
		    moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5)
		    pline("%s yowls.", Monnam(mtmp));
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    pline("%s purrs.", Monnam(mtmp));
		else
		    pline("%s mews.", Monnam(mtmp));
	    }
	case MS_HISS:
	    if (!mtmp->mpeaceful)
		pline("%s hisses!", Monnam(mtmp));
	    break;
	case MS_BUZZ:
	    if (!mtmp->mpeaceful)
		pline("%s buzzes angrily.", Monnam(mtmp));
	    break;
	case MS_GRUNT:
	    pline("%s grunts.", Monnam(mtmp));
	    break;
	case MS_BARK:
	    if (flags.moonphase == FULL_MOON && night()) {
		pline("%s howls.", Monnam(mtmp));
		break;
	    } else if (mtmp->mpeaceful) {
		if (mtmp->mtame &&
		    (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
		     moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
		    pline("%s whines.", Monnam(mtmp));
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    pline("%s yips.", Monnam(mtmp));
		else
		    pline("%s barks.", Monnam(mtmp));
		break;
	    }
	case MS_GROWL:
	    if (!mtmp->mpeaceful)
		pline("%s growls!", Monnam(mtmp));
	    break;
	case MS_ROAR:
	    if (!mtmp->mpeaceful)
		pline("%s roars!", Monnam(mtmp));
	    break;
	case MS_NEIGH:
	    pline("%s neighs.", Monnam(mtmp));
	    break;
	case MS_WAIL:
	    pline("%s wails mournfully.", Monnam(mtmp));
	    break;
	case MS_GURGLE:
	    pline("%s gurgles.", Monnam(mtmp));
	    break;
	case MS_BURBLE:
	    pline("%s burbles.", Monnam(mtmp));
	    break;
	case MS_SHRIEK:
	    pline("%s shrieks.", Monnam(mtmp));
	    aggravate();
	    break;
	case MS_IMITATE:
	    pline("%s imitates you.", Monnam(mtmp));
	    break;
	case MS_DJINNI:
	    if (mtmp->mtame) verbalize("Thank you for freeing me!");
	    else if (mtmp->mpeaceful) verbalize("I'm free!");
	    else verbalize("This will teach you not to disturb me!");
	    break;
	case MS_MUMBLE:
	    pline("%s mumbles incomprehensibly.", Monnam(mtmp));
	    break;
#ifdef MULDGN
	case MS_LEADER:
	case MS_NEMESIS:
	case MS_GUARDIAN:
	    quest_chat(mtmp);
	    break;
#endif
	case MS_BONES:
	    pline("%s rattles noisily.", Monnam(mtmp));
	    You("freeze for a moment.");
	    nomul(-2);
	    break;
	case MS_HUMANOID:
	    if(In_endgame(&u.uz) && is_mplayer(mtmp->data))
	        mplayer_talk(mtmp);
	    /* Generic humanoid behaviour. */
	    if (!mtmp->mpeaceful) break;
	    if (mtmp->mflee)
		pline("%s wants nothing to do with you.", Monnam(mtmp));
	    else if (mtmp->mhp < mtmp->mhpmax/4)
		pline("%s moans.", Monnam(mtmp));
	    else if (mtmp->mconf || mtmp->mstun)
		verbalize(!rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?");
	    else if (!mtmp->mcansee)
		verbalize("I can't see!");
	    else if (mtmp->mtrapped)
		verbalize("I'm trapped!");
	    else if (mtmp->mhp < mtmp->mhpmax/2)
		pline("%s asks for a potion of healing.", Monnam(mtmp));
	    /* Specific monster's interests */
	    else if (is_elf(mtmp->data))
		pline("%s curses orcs.", Monnam(mtmp));
	    else if (is_dwarf(mtmp->data))
		pline("%s talks about mining.", Monnam(mtmp));
	    else if (likes_magic(mtmp->data))
		pline("%s talks about spellcraft.", Monnam(mtmp));
	    else if (carnivorous(mtmp->data))
		pline("%s discusses hunting.", Monnam(mtmp));
	    else switch (monsndx(mtmp->data)){
		case PM_HOBBIT:
		    if (mtmp->mhpmax - mtmp->mhp >= 10)
pline("%s complains about unpleasant dungeon conditions.", Monnam(mtmp));
		    else
		    	pline("%s asks you about the One Ring.", Monnam(mtmp));
		    break;
		case PM_ARCHEOLOGIST:
pline("%s describes a recent article in \"Spelunker Today\" magazine.", Monnam(mtmp));
		    break;
# ifdef TOURIST
		case PM_TOURIST:
		    verbalize("Aloha.");
		    break;
# endif
		default:
		    pline("%s discusses dungeon exploration.", Monnam(mtmp));
	    }
	    break;
	case MS_SEDUCE:
# ifdef SEDUCE
	    if (mtmp->data->mlet != S_NYMPH &&
		could_seduce(mtmp, &youmonst, (struct attack *)0) == 1) {
			(void) doseduce(mtmp);
			break;
	    }
	    switch ((poly_gender() != mtmp->female) ? rn2(3) : 0) {
# else
	    switch ((poly_gender() == 0) ? rn2(3) : 0) {
# endif
		case 2:
			verbalize("Hello, sailor.");
			break;
		case 1:
			pline("%s comes on to you.", Monnam(mtmp));
			break;
		default:
			pline("%s cajoles you.", Monnam(mtmp));
	    }
	    break;
# ifdef KOPS
	case MS_ARREST:
	    if (mtmp->mpeaceful)
		verbalize("Just the facts, %s.",
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
		    pline("%s giggles.", Monnam(mtmp));
		    break;
		case 2:
		    pline("%s chuckles.", Monnam(mtmp));
		    break;
		case 3:
		    pline("%s snickers.", Monnam(mtmp));
		    break;
		default:
		    pline("%s laughs.", Monnam(mtmp));
	    }
	    break;
	case MS_BRIBE:
	    if (mtmp->mpeaceful && !mtmp->mtame) {
		(void) demon_talk(mtmp);
		break;
	    }
	    /* fall through */
	case MS_CUSS:
	    if (!mtmp->mpeaceful)
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
# ifdef TOURIST
	    else if (uarmu)
		verbalize("Take off your shirt, please.");
# endif
	    else verbalize("Relax, this won't hurt a bit.");
	    break;
	case MS_SELL: /* pitch, pay, total */
	    if (ANGRY(mtmp))
		pline("%s mentions how much %s dislikes %s customers.",
			ESHK(mtmp)->shknam,
			mtmp->female ? "she" : "he",
			ESHK(mtmp)->robbed ? "non-paying" : "rude");
	    else if (ESHK(mtmp)->following)
		if (strncmp(ESHK(mtmp)->customer, plname, PL_NSIZ)) {
		    verbalize("Hello %s!  I was looking for %s.",
			    plname, ESHK(mtmp)->customer);
		    ESHK(mtmp)->following = 0;
		} else {
		    verbalize("Hello %s!  Didn't you forget to pay?",
			    plname);
		}
	    else if (ESHK(mtmp)->robbed)
		pline("%s complains about a recent robbery.", ESHK(mtmp)->shknam);
	    else if (ESHK(mtmp)->billct)
		pline("%s reminds you that you haven't paid yet.", ESHK(mtmp)->shknam);
	    else if (mtmp->mgold < 50)
		pline("%s complains that business is bad.", ESHK(mtmp)->shknam);
	    else if (mtmp->mgold > 4000)
		pline("%s says that business is good.", ESHK(mtmp)->shknam);
	    else
		pline("%s talks about the problem of shoplifters.", ESHK(mtmp)->shknam);
	    break;
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
	    } else
	    switch (rn2(3)) {
		case 2:
		    verbalize("What lousy pay we're getting here!");
		    break;
		case 1:
		    verbalize("The food's not fit for Orcs!");
		    break;
		default:
		    verbalize("My feet hurt, I've been on them all day!");
	    }
	    break;
	case MS_DEATH:
	    pline("%s is busy reading a copy of Sandman #9.", Monnam(mtmp));
	    break;
	case MS_PESTILENCE:
	case MS_FAMINE:
	    verbalize("Who do you think you are, War?");
	    break;
#endif /* SOUNDS */
    }
    return(1);
}


int
dotalk()
{
    int result;
    boolean save_soundok = flags.soundok;
    flags.soundok = 1;	/* always allow sounds while chatting */
    result = dochat();
    flags.soundok = save_soundok;
    return result;
}

static int
dochat()
{
    register struct monst *mtmp;
    register int tx,ty;
    struct obj *otmp;

#ifdef POLYSELF
    if (uasmon->msound == MS_SILENT) {
	pline("As %s, you cannot speak.", an(uasmon->mname));
	return(0);
    }
#endif
    if (Strangled) {
	You("can't speak.  You're choking!");
	return(0);
    }
    if (u.uswallow) {
	pline("They won't hear you out there.");
	return(0);
    }
    if (Underwater) {
	pline("Your speech is unintelligible underwater.");
	return(0);
    }

    if (!Blind && (otmp = shop_object(u.ux, u.uy)) != (struct obj *)0) {
	/* standing on something in a shop and chatting causes the shopkeeper
	   to describe the price(s).  This can inhibit other chatting inside
	   a shop, but that shouldn't matter much.  shop_object() returns an
	   object iff inside a shop and the shopkeeper is present and willing
	   (not angry) and able (not asleep) to speak and the position contains
	   any objects other than just gold.
	*/
	price_quote(otmp);
	return(1);
    }

    (void) getdir("Talk to whom? [in what direction]");

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
    mtmp = m_at(tx, ty);
    if ((Blind && !Telepat) || !mtmp || mtmp->mundetected ||
		mtmp->m_ap_type == M_AP_FURNITURE ||
		mtmp->m_ap_type == M_AP_OBJECT) {
	pline("I see nobody there.");
	return(0);
    }
    if (!mtmp->mcanmove || mtmp->msleep) {
	pline("%s seems not to notice you.", Monnam(mtmp));
	return(0);
    }

    if (mtmp->mtame && mtmp->meating) {
	pline("%s is eating noisily.", Monnam(mtmp));
	return (0);
    }

    return domonnoise(mtmp);
}

#endif /* OVLB */

/*sounds.c*/
