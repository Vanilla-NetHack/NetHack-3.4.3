/*	SCCS Id: @(#)sounds.c	3.1	93/03/14	*/
/*	Copyright (c) 1989 Janet Walz, Mike Threepoint */
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

    if (level.flags.nfountains && !rn2(400)) {
	static const char *fountain_msg[4] = {
		"hear bubbling water.",
		"hear water falling on coins.",
		"hear the splashing of a naiad.",
		"hear a soda fountain!",
	};
	You(fountain_msg[rn2(3)+hallu]);
    }
#ifdef SINK
    if (level.flags.nsinks && !rn2(300)) {
	static const char *sink_msg[3] = {
		"hear a slow drip.",
		"hear a gurgling noise.",
		"hear dishes being washed!",
	};
	You(sink_msg[rn2(2)+hallu]);
    }
#endif
    if (level.flags.has_court && !rn2(200)) {
	static const char *throne_msg[4] = {
		"hear the tones of courtly conversation.",
		"hear a sceptre pounded in judgment.",
		"Someone shouts \"Off with %s head!\"",
		"hear Queen Beruthiel's cats!",
	};
	int which = rn2(3)+hallu;
	if (which != 2) You(throne_msg[which]);
	else		pline(throne_msg[2], his[flags.female]);
	return;
    }
    if (level.flags.has_swamp && !rn2(200)) {
	static const char *swamp_msg[3] = {
		"hear mosquitoes!",
		"smell marsh gas!",	/* so it's a smell...*/
		"hear Donald Duck!",
	};
	You(swamp_msg[rn2(2)+hallu]);
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
	static const char *barracks_msg[4] = {
		"hear blades being honed.",
		"hear loud snoring.",
		"hear dice being thrown.",
		"hear General MacArthur!",
	};
	You(barracks_msg[rn2(3)+hallu]);
	return;
    }
#endif /* ARMY */
    if (level.flags.has_zoo && !rn2(200)) {
	static const char *zoo_msg[3] = {
		"hear a sound reminiscent of an elephant stepping on a peanut.",
		"hear a sound reminiscent of a seal barking.",
		"hear Doctor Doolittle!",
	};
	You(zoo_msg[rn2(2)+hallu]);
	return;
    }
    if (level.flags.has_shop && !rn2(200)) {
	if (!(sroom = search_special(ANY_SHOP))) {
	    /* strange... */
	    level.flags.has_shop = 0;
	    return;
	}
	if (tended_shop(sroom) &&
		!index(u.ushops, ROOM_INDEX(sroom) + ROOMOFFSET)) {
	    static const char *shop_msg[3] = {
		    "hear someone cursing shoplifters.",
		    "hear the chime of a cash register.",
		    "hear Neiman and Marcus arguing!",
	    };
	    You(shop_msg[rn2(2)+hallu]);
	}
	return;
    }
}

#endif /* OVL0 */
#ifdef OVLB

static const char *h_sounds[] = {
    "beep", "boing", "sing", "belche", "creak", "cough", "rattle",
    "ululate", "pop", "jingle", "sniffle", "tinkle", "eep"
};

void
growl(mtmp)
register struct monst *mtmp;
{
    register const char *growl_verb = 0;

    if (mtmp->msleep || !mtmp->mcanmove || !mtmp->data->msound) return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        growl_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_HISS:
	    growl_verb = "hisse";	/* hisseS */
	    break;
	case MS_BARK:
	case MS_GROWL:
	    growl_verb = "growl";
	    break;
	case MS_ROAR:
	    growl_verb = "roar";
	    break;
	case MS_BUZZ:
	    growl_verb = "buzze";
	    break;
	case MS_SQEEK:
	    growl_verb = "squeal";
	    break;
	case MS_SQAWK:
	    growl_verb = "screeche";
	    break;
	case MS_NEIGH:
	    growl_verb = "neigh";
	    break;
	case MS_WAIL:
	    growl_verb = "wail";
	    break;
    }
    if (growl_verb) pline("%s %ss!", Monnam(mtmp), growl_verb);
}

/* the sounds of mistreated pets */
void
yelp(mtmp)
register struct monst *mtmp;
{
    register const char *yelp_verb = 0;

    if (mtmp->msleep || !mtmp->mcanmove || !mtmp->data->msound) return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        yelp_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	    yelp_verb = "yowl";
	    break;
	case MS_BARK:
	case MS_GROWL:
	    yelp_verb = "yelp";
	    break;
	case MS_ROAR:
	    yelp_verb = "snarl";
	    break;
	case MS_SQEEK:
	    yelp_verb = "squeal";
	    break;
	case MS_SQAWK:
	    yelp_verb = "screak";
	    break;
	case MS_WAIL:
	    yelp_verb = "wail";
	    break;
    }
    if (yelp_verb) pline("%s %ss!", Monnam(mtmp), yelp_verb);
}

/* the sounds of distressed pets */
void
whimper(mtmp)
register struct monst *mtmp;
{
    register const char *whimper_verb = 0;

    if (mtmp->msleep || !mtmp->mcanmove || !mtmp->data->msound) return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        whimper_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_GROWL:
	    whimper_verb = "whimper";
	    break;
	case MS_BARK:
	    whimper_verb = "whine";
	    break;
	case MS_SQEEK:
	    whimper_verb = "squeal";
	    break;
    }
    if (whimper_verb) pline("%s %ss.", Monnam(mtmp), whimper_verb);
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleep || !mtmp->mcanmove ||
	!(carnivorous(mtmp->data) || herbivorous(mtmp->data))) return;
    /* presumably nearness and soundok checks have already been made */
    if (mtmp->data->msound != MS_SILENT && mtmp->data->msound <= MS_ANIMAL)
	(void) domonnoise(mtmp);
    else if (mtmp->data->msound >= MS_HUMANOID)
	verbalize("I'm hungry.");
}

#endif /* OVLB */

#endif /* SOUNDS */

#ifdef OVLB

static int
domonnoise(mtmp)
register struct monst *mtmp;
{
#ifdef SOUNDS
    register const char *pline_msg = 0;	/* Monnam(mtmp) will be prepended */
#endif

    /* presumably nearness and sleep checks have already been made */
    if (!flags.soundok) return(0);

    switch (mtmp->data->msound) {
	case MS_ORACLE:
	    return doconsult(mtmp);
	case MS_PRIEST:
	    priest_talk(mtmp);
	    break;
#ifdef MULDGN
	case MS_LEADER:
	case MS_NEMESIS:
	case MS_GUARDIAN:
	    quest_chat(mtmp);
	    break;
#endif
#ifdef SOUNDS
	case MS_SELL: /* pitch, pay, total */
	    shk_chat(mtmp);
	    break;
	case MS_SILENT:
	    break;
	case MS_BARK:
	    if (flags.moonphase == FULL_MOON && night()) {
		pline_msg = "howls.";
	    } else if (mtmp->mpeaceful) {
		if (mtmp->mtame &&
		    (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
		     moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
		    pline_msg = "whines.";
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    pline_msg = "yips.";
		else
		    pline_msg = "barks.";
	    } else {
		pline_msg = "growls.";
	    }
	    break;
	case MS_MEW:
	    if (mtmp->mtame) {
		if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
		    mtmp->mtame < 5)
		    pline_msg = "yowls.";
		else if (moves > EDOG(mtmp)->hungrytime)
		    pline_msg = "miaos.";
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    pline_msg = "purrs.";
		else
		    pline_msg = "mews.";
		break;
	    } /* else FALLTHRU */
	case MS_GROWL:
	    pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
	    break;
	case MS_ROAR:
	    pline_msg = mtmp->mpeaceful ? "snarls." : "roars!";
	    break;
	case MS_SQEEK:
	    pline_msg = "squeaks.";
	    break;
	case MS_SQAWK:
	    pline_msg = "squawks.";
	    break;
	case MS_HISS:
	    if (!mtmp->mpeaceful)
		pline_msg = "hisses!";
	    else return 0;	/* no sound */
	    break;
	case MS_BUZZ:
	    pline_msg = mtmp->mpeaceful ? "drones." : "buzzes angrily.";
	    break;
	case MS_GRUNT:
	    pline_msg = "grunts.";
	    break;
	case MS_NEIGH:
	    if (mtmp->mtame < 5)
		pline_msg = "neighs.";
	    else if (moves > EDOG(mtmp)->hungrytime)
		pline_msg = "whinnies.";
	    else
		pline_msg = "whickers.";
	    break;
	case MS_WAIL:
	    pline_msg = "wails mournfully.";
	    break;
	case MS_GURGLE:
	    pline_msg = "gurgles.";
	    break;
	case MS_BURBLE:
	    pline_msg = "burbles.";
	    break;
	case MS_SHRIEK:
	    pline_msg = "shrieks.";
	    aggravate();
	    break;
	case MS_IMITATE:
	    pline_msg = "imitates you.";
	    break;
	case MS_BONES:
	    pline("%s rattles noisily.", Monnam(mtmp));
	    You("freeze for a moment.");
	    nomul(-2);
	    break;
	case MS_LAUGH:
	    {
		static const char *laugh_msg[4] = {
		    "giggles.", "chuckles.", "snickers.", "laughs.",
		};
		pline_msg = laugh_msg[rn2(4)];
	    }
	    break;
	case MS_MUMBLE:
	    pline_msg = "mumbles incomprehensibly.";
	    break;
	case MS_DJINNI:
	    if (mtmp->mtame) verbalize("Thank you for freeing me!");
	    else if (mtmp->mpeaceful) verbalize("I'm free!");
	    else verbalize("This will teach you not to disturb me!");
	    break;
	case MS_HUMANOID:
	    if (!mtmp->mpeaceful) {
		if (In_endgame(&u.uz) && is_mplayer(mtmp->data)) {
		    mplayer_talk(mtmp);
		    break;
		} else {
		    return 0;	/* no sound */
		}
	    }
	    /* Generic peaceful humanoid behaviour. */
	    if (mtmp->mflee)
		pline_msg = "wants nothing to do with you.";
	    else if (mtmp->mhp < mtmp->mhpmax/4)
		pline_msg = "moans.";
	    else if (mtmp->mconf || mtmp->mstun)
		verbalize(!rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?");
	    else if (!mtmp->mcansee)
		verbalize("I can't see!");
	    else if (mtmp->mtrapped)
		verbalize("I'm trapped!");
	    else if (mtmp->mhp < mtmp->mhpmax/2)
		pline_msg = "asks for a potion of healing.";
	    else if (mtmp->mtame && moves > EDOG(mtmp)->hungrytime)
		verbalize("I'm hungry.");
	    /* Specific monster's interests */
	    else if (is_elf(mtmp->data))
		pline_msg = "curses orcs.";
	    else if (is_dwarf(mtmp->data))
		pline_msg = "talks about mining.";
	    else if (likes_magic(mtmp->data))
		pline_msg = "talks about spellcraft.";
	    else if (carnivorous(mtmp->data))
		pline_msg = "discusses hunting.";
	    else switch (monsndx(mtmp->data)) {
		case PM_HOBBIT:
		    pline_msg = (mtmp->mhpmax - mtmp->mhp >= 10) ?
				"complains about unpleasant dungeon conditions."
				: "asks you about the One Ring.";
		    break;
		case PM_ARCHEOLOGIST:
    pline_msg = "describes a recent article in \"Spelunker Today\" magazine.";
		    break;
# ifdef TOURIST
		case PM_TOURIST:
		    verbalize("Aloha.");
		    break;
# endif
		default:
		    pline_msg = "discusses dungeon exploration.";
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
			pline_msg = "comes on to you.";
			break;
		default:
			pline_msg = "cajoles you.";
	    }
	    break;
# ifdef KOPS
	case MS_ARREST:
	    if (mtmp->mpeaceful)
		verbalize("Just the facts, %s.",
		      flags.female ? "Ma'am" : "Sir");
	    else {
		static const char *arrest_msg[3] = {
		    "Anything you say can be used against you.",
		    "You're under arrest!",
		    "Stop in the name of the Law!",
		};
		verbalize(arrest_msg[rn2(3)]);
	    }
	    break;
# endif
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
	case MS_GUARD:
	    if (u.ugold)
		verbalize("Please drop that gold and follow me.");
	    else
		verbalize("Please follow me.");
	    break;
	case MS_SOLDIER:
	    {
		static const char *soldier_foe_msg[3] = {
		    "Resistance is useless!",
		    "You're dog meat!",
		    "Surrender!",
		},		  *soldier_pax_msg[3] = {
		    "What lousy pay we're getting here!",
		    "The food's not fit for Orcs!",
		    "My feet hurt, I've been on them all day!",
		};
		verbalize(mtmp->mpeaceful ? soldier_pax_msg[rn2(3)]
					  : soldier_foe_msg[rn2(3)]);
	    }
	    break;
	case MS_RIDER:
	    if (mtmp->data == &mons[PM_DEATH] && mtmp->mpeaceful)
		pline_msg = "is busy reading a copy of Sandman #9.";
	    else verbalize("Who do you think you are, War?");
	    break;
#endif /* SOUNDS */
    }

#ifdef SOUNDS
    if (pline_msg) pline("%s %s", Monnam(mtmp), pline_msg);
#endif
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
