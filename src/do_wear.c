/*	SCCS Id: @(#)do_wear.c	3.0	88/05/10
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static int todelay;

static long takeoff_mask = 0L, taking_off = 0L;
static const long takeoff_order[] = { WORN_BLINDF, 1L, /* weapon */
	WORN_SHIELD, WORN_GLOVES, LEFT_RING, RIGHT_RING, WORN_CLOAK,
	WORN_HELMET, WORN_AMUL, WORN_ARMOR,
#ifdef SHIRT
	WORN_SHIRT,
#endif
	WORN_BOOTS, 0L };

void
off_msg(otmp) register struct obj *otmp; {
	if(flags.verbose)
	    You("were wearing %s.", doname(otmp));
}

/* for items that involve no delay */
static void
on_msg(otmp) register struct obj *otmp; {
	register char *bp = xname(otmp);
	char buf[BUFSZ];

	setan(bp, buf);
	if(flags.verbose)
	    You("are now wearing %s.", buf);
}

boolean
is_boots(otmp) register struct obj *otmp; {
	return(otmp->otyp >= LOW_BOOTS &&
	   	otmp->otyp <= LEVITATION_BOOTS);
}

boolean
is_helmet(otmp) register struct obj *otmp; {
#ifdef TOLKIEN
	return(otmp->otyp >= ELVEN_LEATHER_HELM &&
		otmp->otyp <= HELM_OF_TELEPATHY);
#else
	return(otmp->otyp >= ORCISH_HELM &&
		otmp->otyp <= HELM_OF_TELEPATHY);
#endif
}

boolean
is_gloves(otmp) register struct obj *otmp; {
	return(otmp->otyp >= LEATHER_GLOVES &&
	   	otmp->otyp <= GAUNTLETS_OF_DEXTERITY);
}

boolean
is_cloak(otmp) register struct obj *otmp; {
	return(otmp->otyp >= MUMMY_WRAPPING &&
		otmp->otyp <= CLOAK_OF_DISPLACEMENT);
}

boolean
is_shield(otmp) register struct obj *otmp; {
	return(otmp->otyp >= SMALL_SHIELD &&
	   	otmp->otyp <= SHIELD_OF_REFLECTION);
}

/*
 * The Type_on() functions should be called *after* setworn().
 * The Type_off() functions call setworn() themselves.
 */

static int
Boots_on() {
    long oldprop =
		u.uprops[objects[uarmf->otyp].oc_oprop].p_flgs & ~(WORN_BOOTS | TIMEOUT);

    switch(uarmf->otyp) {
	case LOW_BOOTS:
	case IRON_SHOES:
	case HIGH_BOOTS:
	case WATER_WALKING_BOOTS:
	case JUMPING_BOOTS:
		break;
	case SPEED_BOOTS:
		if (!oldprop) {
			makeknown(uarmf->otyp);
			You("feel yourself speed up.");
		}
		break;
	case ELVEN_BOOTS:
		if (!oldprop) {
			makeknown(uarmf->otyp);
			You("walk very quietly.");
		}
		break;
	case FUMBLE_BOOTS:
		if (!oldprop)
			Fumbling += rnd(20);
		break;
	case LEVITATION_BOOTS:
		if (!oldprop) {
			makeknown(uarmf->otyp);
			float_up();
		}
		break;
	default: impossible("Unknown type of boots (%d)", uarmf->otyp);
    }
    return 0;
}

int
Boots_off() {
    register struct obj *obj = uarmf;
	/* For levitation, float_down() returns if Levitation, so we
	 * must do a setworn() _before_ the levitation case.
	 */
    long oldprop =
		u.uprops[objects[uarmf->otyp].oc_oprop].p_flgs & ~(WORN_BOOTS | TIMEOUT);

    setworn((struct obj *)0, W_ARMF);
    switch(obj->otyp) {
	case SPEED_BOOTS:
		if (!oldprop) {
			makeknown(obj->otyp);
			You("feel yourself slow down.");
		}
		break;
	case WATER_WALKING_BOOTS:
		if(is_pool(u.ux,u.uy) && !Levitation
#ifdef POLYSELF
		    && !is_flyer(uasmon)
#endif
		    ) {
			makeknown(obj->otyp);
			/* make boots known in case you survive the drowning */
			drown();
		}
		break;
	case ELVEN_BOOTS:
		if (!oldprop) {
			makeknown(obj->otyp);
			You("sure are noisy.");
		}
		break;
	case FUMBLE_BOOTS:
		if (!oldprop)
			Fumbling = 0;
		break;
	case LEVITATION_BOOTS:
		if (!oldprop) {
			(void) float_down();
			makeknown(obj->otyp);
		}
		break;
	case LOW_BOOTS:
	case IRON_SHOES:
	case HIGH_BOOTS:
	case JUMPING_BOOTS:
		break;
	default: impossible("Unknown type of boots (%d)", obj->otyp);
    }
    return 0;
}

static int
Cloak_on() {
    long oldprop = u.uprops[objects[uarmc->otyp].oc_oprop].p_flgs & ~WORN_CLOAK;

    switch(uarmc->otyp) {
	case ELVEN_CLOAK:
	case CLOAK_OF_PROTECTION:
	case CLOAK_OF_DISPLACEMENT:
		makeknown(uarmc->otyp);
		break;
	case MUMMY_WRAPPING:
#ifdef TOLKIEN
	case ORCISH_CLOAK:
	case DWARVISH_CLOAK:
#endif
	case CLOAK_OF_MAGIC_RESISTANCE:
		break;
	case CLOAK_OF_INVISIBILITY:
		if (!oldprop && !See_invisible && !Blind) {
			makeknown(uarmc->otyp);
			pline("Suddenly you cannot see yourself.");
		}
		break;
	default: impossible("Unknown type of cloak (%d)", uarmc->otyp);
    }
    return 0;
}

int
Cloak_off() {
    long oldprop = u.uprops[objects[uarmc->otyp].oc_oprop].p_flgs & ~WORN_CLOAK;

    switch(uarmc->otyp) {
	case MUMMY_WRAPPING:
	case ELVEN_CLOAK:
#ifdef TOLKIEN
	case ORCISH_CLOAK:
	case DWARVISH_CLOAK:
#endif
	case CLOAK_OF_PROTECTION:
	case CLOAK_OF_MAGIC_RESISTANCE:
	case CLOAK_OF_DISPLACEMENT:
		break;
	case CLOAK_OF_INVISIBILITY:
		if (!oldprop && !See_invisible && !Blind) {
			makeknown(uarmc->otyp);
			pline("Suddenly you can see yourself.");
		}
		break;
	default: impossible("Unknown type of cloak (%d)", uarmc->otyp);
    }
    setworn((struct obj *)0, W_ARMC);
    return 0;
}

static int
Helmet_on() {
    switch(uarmh->otyp) {
	case FEDORA:
	case HELMET:
#ifdef TOLKIEN
	case ELVEN_LEATHER_HELM:
	case DWARVISH_IRON_HELM:
#endif
	case ORCISH_HELM:
	case HELM_OF_TELEPATHY:
		break;
	case HELM_OF_BRILLIANCE:
		if (uarmh->spe) {
			ABON(A_INT) += uarmh->spe;
			ABON(A_WIS) += uarmh->spe;
			flags.botl = 1;
			makeknown(uarmh->otyp);
		}
		break;
	case HELM_OF_OPPOSITE_ALIGNMENT:
		if (u.ualigntyp == U_NEUTRAL) u.ualigntyp = rnd(2) ? -1 : 1;
		else u.ualigntyp = -(u.ualigntyp);
		makeknown(uarmh->otyp);
		flags.botl = 1;
		break;
	default: impossible("Unknown type of helm (%d)", uarmh->otyp);
    }
    return 0;
}

int
Helmet_off() {
    switch(uarmh->otyp) {
	case FEDORA:
	case HELMET:
#ifdef TOLKIEN
	case ELVEN_LEATHER_HELM:
	case DWARVISH_IRON_HELM:
#endif
	case ORCISH_HELM:
	case HELM_OF_TELEPATHY:
		break;
	case HELM_OF_BRILLIANCE:
		if (uarmh->spe) {
			ABON(A_INT) -= uarmh->spe;
			ABON(A_WIS) -= uarmh->spe;
			flags.botl = 1;
		}
		break;
	case HELM_OF_OPPOSITE_ALIGNMENT:
#ifdef THEOLOGY
		u.ualigntyp = u.ualignbase[0];
#else
		if (pl_character[0] == 'P' ||
		    pl_character[0] == 'T' ||
		    pl_character[0] == 'W')
			u.ualigntyp = U_NEUTRAL;
		else u.ualigntyp = -(u.ualigntyp);
#endif
		flags.botl = 1;
		break;
	default: impossible("Unknown type of helm (%d)", uarmh->otyp);
    }
    setworn((struct obj *)0, W_ARMH);
    return 0;
}

static int
Gloves_on() {
    long oldprop =
	u.uprops[objects[uarmg->otyp].oc_oprop].p_flgs & ~(WORN_GLOVES | TIMEOUT);

    switch(uarmg->otyp) {
	case LEATHER_GLOVES:
		break;
	case GAUNTLETS_OF_FUMBLING:
		if (!oldprop)
			Fumbling += rnd(20);
		break;
	case GAUNTLETS_OF_POWER:
		makeknown(uarmg->otyp);
		flags.botl = 1; /* taken care of in attrib.c */
		break;
	case GAUNTLETS_OF_DEXTERITY:
		if (uarmg->spe) makeknown(uarmg->otyp);
		ABON(A_DEX) += uarmg->spe;
		flags.botl = 1;
		break;
	default: impossible("Unknown type of gloves (%d)", uarmg->otyp);
    }
    return 0;
}

int
Gloves_off() {
    long oldprop =
	u.uprops[objects[uarmg->otyp].oc_oprop].p_flgs & ~(WORN_GLOVES | TIMEOUT);

    switch(uarmg->otyp) {
	case LEATHER_GLOVES:
		break;
	case GAUNTLETS_OF_FUMBLING:
		if (!oldprop)
			Fumbling = 0;
		break;
	case GAUNTLETS_OF_POWER:
		makeknown(uarmg->otyp);
		flags.botl = 1; /* taken care of in attrib.c */
		break;
	case GAUNTLETS_OF_DEXTERITY:
		if (uarmg->spe) makeknown(uarmg->otyp);
		ABON(A_DEX) -= uarmg->spe;
		flags.botl = 1;
		break;
	default: impossible("Unknown type of gloves (%d)", uarmg->otyp);
    }
    setworn((struct obj *)0, W_ARMG);
    return 0;
}

/*
static int
Shield_on() {
    switch(uarms->otyp) {
	case SMALL_SHIELD:
#ifdef TOLKIEN
	case ELVEN_SHIELD:
	case URUK_HAI_SHIELD:
	case ORCISH_SHIELD:
	case DWARVISH_ROUNDSHIELD:
#endif
	case LARGE_SHIELD:
	case SHIELD_OF_REFLECTION:
		break;
	default: impossible("Unknown type of shield (%d)", uarms->otyp);
    }
    return 0;
}
*/

int
Shield_off() {
/*
    switch(uarms->otyp) {
	case SMALL_SHIELD:
#ifdef TOLKIEN
	case ELVEN_SHIELD:
	case URUK_HAI_SHIELD:
	case ORCISH_SHIELD:
	case DWARVISH_ROUNDSHIELD:
#endif
	case LARGE_SHIELD:
	case SHIELD_OF_REFLECTION:
		break;
	default: impossible("Unknown type of shield (%d)", uarms->otyp);
    }
*/
    setworn((struct obj *)0, W_ARMS);
    return 0;
}

/* This must be done in worn.c, because one of the possible intrinsics conferred
 * is fire resistance, and we have to immediately set HFire_resistance in worn.c
 * since worn.c will check it before returning.
static int
Armor_on()
{
    return 0;
}
 */

int
Armor_off()
{
    setworn((struct obj *)0, W_ARM);
    return 0;
}

/* The gone functions differ from the off functions in that if you die from
 * taking it off and have life saving, you still die.
 */
int
Armor_gone()
{
    setnotworn(uarm);
    return 0;
}

static void
Amulet_on()
{
    char buf[BUFSZ];

    switch(uamul->otyp) {
	case AMULET_OF_ESP:
	case AMULET_OF_LIFE_SAVING:
	case AMULET_VERSUS_POISON:
	case AMULET_OF_REFLECTION:
		break;
	case AMULET_OF_CHANGE:
		makeknown(AMULET_OF_CHANGE);
		flags.female = !flags.female;
		max_rank_sz();
		/* Don't use same message as polymorph */
		You("are suddenly very %s!", flags.female ? "feminine"
			: "masculine");
		if (pl_character[0]=='P')
			Strcpy(pl_character+6, flags.female? "ess":"");
		if (pl_character[0]=='C')
			Strcpy(pl_character+5, flags.female ? "woman" : "man");
#ifdef WIZARD
		if (!wizard) {
#endif
newname:	more();
		do {
		    pline("What shall you be called, %s? ",
			flags.female ? "madam" : "sir");
		    getlin(buf);
		} while (buf[0]=='\033' || buf[0]==0);
		if (!strcmp(plname,buf)) {
		    pline("Sorry, that name no longer seems appropriate!");
		    goto newname;
		}
		flags.botl = 1;
		(void)strncpy(plname, buf, sizeof(plname)-1);
		Sprintf(SAVEF, "save/%d%s", getuid(), plname);
		regularize(SAVEF+5);		/* avoid . or / in name */
#ifdef WIZARD
		}
#endif
		pline("The amulet disintegrates!");
		useup(uamul);
		break;
	case AMULET_OF_STRANGULATION:
		makeknown(AMULET_OF_STRANGULATION);
		pline("It constricts your throat!");
		Strangled = 6;
		break;
	case AMULET_OF_RESTFUL_SLEEP:
		Sleeping = rnd(100);
		break;
	case AMULET_OF_YENDOR:
		break;
    }
}

void
Amulet_off()
{
    switch(uamul->otyp) {
	case AMULET_OF_ESP:
	case AMULET_OF_LIFE_SAVING:
	case AMULET_VERSUS_POISON:
	case AMULET_OF_REFLECTION:
		break;
	case AMULET_OF_CHANGE:
		impossible("Wearing an amulet of change?");
		break;
	case AMULET_OF_STRANGULATION:
		if (Strangled) {
			You("can breathe more easily!");
			Strangled = 0;
		}
		break;
	case AMULET_OF_RESTFUL_SLEEP:
		Sleeping = 0;
		break;
	case AMULET_OF_YENDOR:
		break;
    }
    setworn((struct obj *)0, W_AMUL);
}

void
Ring_on(obj)
register struct obj *obj;
{
    long oldprop = u.uprops[objects[obj->otyp].oc_oprop].p_flgs & ~W_RING;

    switch(obj->otyp){
	case RIN_TELEPORTATION:
	case RIN_REGENERATION:
	case RIN_SEARCHING:
	case RIN_STEALTH:
	case RIN_HUNGER:
	case RIN_AGGRAVATE_MONSTER:
	case RIN_POISON_RESISTANCE:
	case RIN_FIRE_RESISTANCE:
	case RIN_COLD_RESISTANCE:
	case RIN_SHOCK_RESISTANCE:
	case RIN_CONFLICT:
	case RIN_WARNING:
	case RIN_TELEPORT_CONTROL:
#ifdef POLYSELF
	case RIN_POLYMORPH:
	case RIN_POLYMORPH_CONTROL:
#endif
		break;
	case RIN_SEE_INVISIBLE:
		if (Invisible && !Blind) {
			newsym(u.ux,u.uy);
			pline("Suddenly you can see yourself.");
			makeknown(RIN_SEE_INVISIBLE);
		}
		break;
	case RIN_INVISIBILITY:
		if (!oldprop && !See_invisible && !Blind) {
			makeknown(RIN_INVISIBILITY);
			Your("body takes on a %s transparency...",
				Hallucination ? "normal" : "strange");
		}
	case RIN_ADORNMENT:
		ABON(A_CHA) += obj->spe;
		flags.botl = 1;
		if (obj->spe || objects[RIN_ADORNMENT].oc_name_known) {
			makeknown(RIN_ADORNMENT);
			obj->known = 1;
		}
		break;
	case RIN_LEVITATION:
		if(!oldprop) {
			float_up();
			makeknown(RIN_LEVITATION);
			obj->known = 1;
		}
		break;
	case RIN_GAIN_STRENGTH:
		ABON(A_STR) += obj->spe;
		flags.botl = 1;
		if (obj->spe || objects[RIN_GAIN_STRENGTH].oc_name_known) {
			makeknown(RIN_GAIN_STRENGTH);
			obj->known = 1;
		}
		break;
	case RIN_INCREASE_DAMAGE:
		u.udaminc += obj->spe;
		break;
	case RIN_PROTECTION_FROM_SHAPE_CHAN:
		rescham();
		break;
	case RIN_PROTECTION:
		flags.botl = 1;
		if (obj->spe || objects[RIN_PROTECTION].oc_name_known) {
			makeknown(RIN_PROTECTION);
			obj->known = 1;
		}
		break;
    }
}

static void
Ring_off_or_gone(obj,gone)
register struct obj *obj;
boolean gone;
{
    register long mask = obj->owornmask & W_RING;

    if(!(u.uprops[objects[obj->otyp].oc_oprop].p_flgs & mask))
	impossible("Strange... I didn't know you had that ring.");
    if(gone) setnotworn(obj);
    else setworn((struct obj *)0, obj->owornmask);
    switch(obj->otyp) {
	case RIN_TELEPORTATION:
	case RIN_REGENERATION:
	case RIN_SEARCHING:
	case RIN_STEALTH:
	case RIN_HUNGER:
	case RIN_AGGRAVATE_MONSTER:
	case RIN_POISON_RESISTANCE:
	case RIN_FIRE_RESISTANCE:
	case RIN_COLD_RESISTANCE:
	case RIN_SHOCK_RESISTANCE:
	case RIN_CONFLICT:
	case RIN_WARNING:
	case RIN_TELEPORT_CONTROL:
#ifdef POLYSELF
	case RIN_POLYMORPH:
	case RIN_POLYMORPH_CONTROL:
#endif
		break;
	case RIN_SEE_INVISIBLE:
		if (Invisible && !Blind) {
			pline("Suddenly you cannot see yourself.");
			makeknown(RIN_SEE_INVISIBLE);
		}
		break;
	case RIN_INVISIBILITY:
		if (!(Invisible & ~W_RING) && !See_invisible && !Blind) {
			Your("body seems to unfade...");
			makeknown(RIN_INVISIBILITY);
		}
		break;
	case RIN_ADORNMENT:
		ABON(A_CHA) -= obj->spe;
		flags.botl = 1;
		break;
	case RIN_LEVITATION:
		(void) float_down();
		if (!Levitation) makeknown(RIN_LEVITATION);
		break;
	case RIN_GAIN_STRENGTH:
		ABON(A_STR) -= obj->spe;
		flags.botl = 1;
		break;
	case RIN_INCREASE_DAMAGE:
		u.udaminc -= obj->spe;
		break;
	case RIN_PROTECTION_FROM_SHAPE_CHAN:
		/* If you're no longer protected, let the chameleons
		 * change shape again -dgk
		 */
		restartcham();
		break;
    }
}

void
Ring_off(obj)
struct obj *obj;
{
	Ring_off_or_gone(obj,FALSE);
}

void
Ring_gone(obj)
struct obj *obj;
{
	Ring_off_or_gone(obj,TRUE);
}

void
Blindf_on(otmp) 
register struct obj *otmp;
{
	setworn(otmp, W_TOOL);
	on_msg(otmp);
	seeoff(0);
}

void
Blindf_off(otmp) 
register struct obj *otmp;
{
	setworn((struct obj *)0, otmp->owornmask);
	off_msg(otmp);
	if (!Blinded)	make_blinded(1L,FALSE); /* See on next move */
	else		You("still cannot see.");
}

/* called in main to set intrinsics of worn start-up items */
void
set_wear() {
/*	if (uarm)  (void) Armor_on(); */
	if (uarmc) (void) Cloak_on();
	if (uarmf) (void) Boots_on();
	if (uarmg) (void) Gloves_on();
	if (uarmh) (void) Helmet_on();
/*	if (uarms) (void) Shield_on(); */
}

static const char clothes[] = {ARMOR_SYM, 0};
static const char accessories[] = {RING_SYM, AMULET_SYM, TOOL_SYM, 0};

int
dotakeoff() {
	register struct obj *otmp;
	int armorpieces = 0;

#define MOREARM(x) if (x) { armorpieces++; otmp = x; }
	MOREARM(uarmh);
	MOREARM(uarms);
	MOREARM(uarmg);
	MOREARM(uarmf);
	if (uarmc) {
		armorpieces++;
		otmp = uarmc;
	} else if (uarm) {
		armorpieces++;
		otmp = uarm;
#ifdef SHIRT
	} else if (uarmu) {
		armorpieces++;
		otmp = uarmu;
#endif
	}
	if (!armorpieces) {
		pline("Not wearing any armor.");
		return 0;
	}
	if (armorpieces > 1)
		otmp = getobj(clothes, "take off");
	if (otmp == 0) return(0);
	if (!(otmp->owornmask & W_ARMOR)) {
		You("are not wearing that.");
		return(0);
	}
	if (((otmp == uarm) && (uarmc))
#ifdef SHIRT
				|| ((otmp == uarmu) && (uarmc || uarm))
#endif
								) {
		You("can't take that off.");
		return(0);
	}
	if(otmp == uarmg && uwep && uwep->cursed) {	/* myers@uwmacc */
    You("seem unable to take off the gloves while holding your %s.",
	  is_sword(uwep) ? "sword" : "weapon");
		uwep->bknown = 1;
		return(0);
	}
	if(otmp == uarmg && Glib) {
    You("can't remove the slippery gloves with your slippery fingers.");
		return(0);
	}
	if(otmp == uarmf && u.utrap && u.utraptype == TT_BEARTRAP) {  /* -3. */
		pline("The bear trap prevents you from pulling your foot out.");
		return(0);
	}
	(void) armoroff(otmp);
	return(1);
}

int
doremring() {
	register struct obj *otmp;
	int Accessories = 0;

#define MOREACC(x) if (x) { Accessories++; otmp = x; }
	MOREACC(uleft);
	MOREACC(uright);
	MOREACC(uamul);
	MOREACC(ublindf);

	if(!Accessories) {
		pline("Not wearing any accessories.");
		return(0);
	}
	if (Accessories != 1) otmp = getobj(accessories, "take off");
	if(!otmp) return(0);
	if(!(otmp->owornmask & (W_RING | W_AMUL | W_TOOL))) {
		You("are not wearing that.");
		return(0);
	}
	if(cursed(otmp)) return(0);
	if(otmp->olet == RING_SYM) {
#ifdef POLYSELF
		if (nolimbs(uasmon)) {
			pline("It seems to be stuck.");
			return(0);
		}
#endif
		if (uarmg && uarmg->cursed) {
			uarmg->bknown = 1;
You("seem unable to remove your ring without taking off your gloves.");
			return(0);
		}
		if (welded(uwep) && bimanual(uwep)) {
			uwep->bknown = 1;
You("seem unable to remove the ring while your hands hold your %s.",
				is_sword(uwep) ? "sword" : "weapon");
			return(0);
		}
		if (welded(uwep) && otmp==uright) {
			uwep->bknown = 1;
You("seem unable to remove the ring while your right hand holds your %s.",
				is_sword(uwep) ? "sword" : "weapon");
			return(0);
		}
		/* Sometimes we want to give the off_msg before removing and
		 * sometimes after; for instance, "you were wearing a moonstone
		 * ring (on right hand)" is desired but "you were wearing a
		 * square amulet (being worn)" is not because of the redundant
		 * "being worn".
		 */
		off_msg(otmp);
		Ring_off(otmp);
	} else if(otmp->olet == AMULET_SYM) {
		Amulet_off();
		off_msg(otmp);
	} else Blindf_off(otmp); /* does its own off_msg */
	return(1);
}

int
cursed(otmp) register struct obj *otmp; {
	/* Curses, like chickens, come home to roost. */
	if(otmp->cursed){
		You("can't.  %s to be cursed.",
			(is_boots(otmp) || is_gloves(otmp) || otmp->quan > 1)
			? "They seem" : "It seems");
		otmp->bknown = 1;
		return(1);
	}
	return(0);
}

int
armoroff(otmp) register struct obj *otmp; {
	register int delay = -objects[otmp->otyp].oc_delay;

	if(cursed(otmp)) return(0);
	if(delay) {
		nomul(delay);
		if (is_helmet(otmp)) {
			nomovemsg = "You finish taking off your helmet.";
			afternmv = Helmet_off;
		     }
		else if (is_gloves(otmp)) {
			nomovemsg = "You finish taking off your gloves.";
			afternmv = Gloves_off;
		     }
		else if (is_boots(otmp)) {
			nomovemsg = "You finish taking off your boots.";
			afternmv = Boots_off;
		     }
		else {
			nomovemsg = "You finish taking off your suit.";
			afternmv = Armor_off;
		}
	} else {
		/* Be warned!  We want off_msg after removing the item to
		 * avoid "You were wearing ____ (being worn)."  However, an
		 * item which grants fire resistance might cause some trouble
		 * if removed in Hell and lifesaving puts it back on; in this
		 * case the message will be printed at the wrong time (after
		 * the messages saying you died and were lifesaved).  Luckily,
		 * no cloak, shield, or fast-removable armor grants fire
		 * resistance, so we can safely do the off_msg afterwards.
		 * Rings do grant fire resistance, but for rings we want the
		 * off_msg before removal anyway so there's no problem.  Take
		 * care in adding armors granting fire resistance; this code
		 * might need modification.
		 */
		if(is_cloak(otmp))
			(void) Cloak_off();
		else if(is_shield(otmp))
			(void) Shield_off();
		else setworn((struct obj *)0, otmp->owornmask & W_ARMOR);
		off_msg(otmp);
	}
	takeoff_mask = taking_off = 0L;
	return(1);
}

int
dowear() {
	register struct obj *otmp;
	register int delay;
	register int err = 0;
	long mask = 0;

#ifdef POLYSELF
	/* cantweararm checks for suits of armor */
	/* verysmall or nohands checks for shields, gloves, etc... */
	if ((verysmall(uasmon) || nohands(uasmon))) {
		pline("Don't even bother.");
		return(0);
	}
#endif
	otmp = getobj(clothes, "wear");
	if(!otmp) return(0);
#ifdef POLYSELF
	if (cantweararm(uasmon) && !is_shield(otmp) &&
			!is_helmet(otmp) && !is_gloves(otmp) &&
			!is_boots(otmp)) {
		pline("The %s will not fit on your body.",
			is_cloak(otmp) ? "cloak" :
# ifdef SHIRT
			otmp->otyp == HAWAIIAN_SHIRT ? "shirt" :
# endif
			"suit");
		return(0);
	}
#endif
	if(otmp->owornmask & W_ARMOR) {
		You("are already wearing that!");
		return(0);
	}
	if(is_helmet(otmp)) {
		if(uarmh) {
			You("are already wearing a helmet.");
			err++;
		} else
			mask = W_ARMH;
	} else if(is_shield(otmp)){
		if(uarms) {
			You("are already wearing a shield.");
			err++;
		}
		if(uwep && bimanual(uwep)) {
		You("cannot hold a shield and wield a two-handed %s.",
		      is_sword(uwep) ? "sword" : "weapon");
			err++;
		}
		if(!err) mask = W_ARMS;
	} else if(is_boots(otmp)) {
		   if(uarmf) {
			You("are already wearing boots.");
			err++;
		   } else
			mask = W_ARMF;
	} else if(is_gloves(otmp)) {
		if(uarmg) {
			You("are already wearing gloves.");
			err++;
		} else
		if(uwep && uwep->cursed) {
			You("cannot wear gloves over your %s.",
			      is_sword(uwep) ? "sword" : "weapon");
			err++;
		} else
			mask = W_ARMG;
#ifdef SHIRT
	} else if( otmp->otyp == HAWAIIAN_SHIRT ) {
		if (uarm || uarmc || uarmu) {
			if(!uarm && !uarmc) /* then uarmu */
			   You("are already wearing a shirt.");
			else
			   You("can't wear that over your %s.",
				 (uarm && !uarmc) ? "armor" : "cloak");
			err++;
		} else
			mask = W_ARMU;
#endif
	} else if(is_cloak(otmp)) {
		if(uarmc) {
			You("are already wearing a cloak.");
			err++;
		} else
			mask = W_ARMC;
	} else {
		if(uarmc) {
			You("cannot wear armor over a cloak.");
			err++;
		} else if(uarm) {
			You("are already wearing some armor.");
			err++;
		}
		if(!err) mask = W_ARM;
	}
/* Unnecessary since now only weapons and special items like pick-axes get
 * welded to your hand, not armor
	if(welded(otmp)) {
		if(!err++)
			weldmsg(otmp, FALSE);
	}
 */
	if(err) return(0);

	otmp->known = 1;
	if(otmp == uwep)
		setuwep((struct obj *)0);
	setworn(otmp, mask);
	delay = -objects[otmp->otyp].oc_delay;
	if(delay){
		nomul(delay);
		if(is_boots(otmp)) afternmv = Boots_on;
		if(is_helmet(otmp)) afternmv = Helmet_on;
		if(is_gloves(otmp)) afternmv = Gloves_on;
/*		if(otmp == uarm) afternmv = Armor_on; */
		nomovemsg = "You finish your dressing maneuver.";
	} else {
		if(is_cloak(otmp)) (void) Cloak_on();
/*		if(is_shield(otmp)) (void) Shield_on(); */
		on_msg(otmp);
	}
	takeoff_mask = taking_off = 0L;
	return(1);
}

int
doputon() {
	register struct obj *otmp;
	long mask = 0;

	if(uleft && uright && uamul && ublindf) {
#ifdef POLYSELF
		Your("%s%s are full, and you're already wearing an amulet and a blindfold.",
			(humanoid(uasmon) || u.usym==S_CENTAUR) ? "ring-" : "",
			makeplural(body_part(FINGER)));
#else
		Your("ring-fingers are full, and you're already wearing an amulet and a blindfold.");
#endif
		return(0);
	}
	otmp = getobj(accessories, "wear");
	if(!otmp) return(0);
	if(otmp->owornmask & (W_RING | W_AMUL | W_TOOL)) {
		You("are already wearing that!");
		return(0);
	}
	if(welded(otmp)) {
		weldmsg(otmp, TRUE);
		return(0);
	}
	if(otmp == uwep)
		setuwep((struct obj *)0);
	if(otmp->olet == RING_SYM) {
#ifdef POLYSELF
		if(nolimbs(uasmon)) {
			You("cannot make the ring stick to your body.");
			return(0);
		}
#endif
		if(uleft && uright){
#ifdef POLYSELF
			pline("There are no more %s%s to fill.",
				(humanoid(uasmon) || u.usym==S_CENTAUR)
					? "ring-" : "",
				makeplural(body_part(FINGER)));
#else
			pline("There are no more ring-fingers to fill.");
#endif
			return(0);
		}
		if(uleft) mask = RIGHT_RING;
		else if(uright) mask = LEFT_RING;
		else do {
			char answer;

#ifdef POLYSELF
			pline("What %s%s, Right or Left? ",
				(humanoid(uasmon) || u.usym==S_CENTAUR)
					? "ring-" : "",
				body_part(FINGER));
#else
			pline("What ring-finger, Right or Left? ");
#endif
			if(index(quitchars, (answer = readchar())))
				return(0);
			switch(answer){
			case 'l':
			case 'L':
				mask = LEFT_RING;
				break;
			case 'r':
			case 'R':
				mask = RIGHT_RING;
				break;
			}
		} while(!mask);
		if (uarmg && uarmg->cursed) {
			uarmg->bknown = 1;
		    You("cannot remove your gloves to put on the ring.");
			return(0);
		}
		if (welded(uwep) && bimanual(uwep)) {
			/* welded will set bknown */
	    You("cannot free your weapon hands to put on the ring.");
			return(0);
		}
		if (welded(uwep) && mask==RIGHT_RING) {
			/* welded will set bknown */
	    You("cannot free your weapon hand to put on the ring.");
			return(0);
		}
		setworn(otmp, mask);
		Ring_on(otmp);
	} else if (otmp->olet == AMULET_SYM) {
		if(uamul) {
			You("are already wearing an amulet.");
			return(0);
		}
		setworn(otmp, W_AMUL);
		if (otmp->otyp == AMULET_OF_CHANGE) {
			Amulet_on();
			/* Don't do a prinv() since the amulet is now gone */
			return(1);
		}
		Amulet_on();
	} else {	/* it's a blindfold */
		if (ublindf) {
			You("are already wearing a blindfold.");
			return(0);
		}
		if (otmp->otyp != BLINDFOLD) {
			You("can't wear that!");
			return(0);
		}
		Blindf_on(otmp);
		return(1);
	}
	prinv(otmp);
	return(1);
}

void
find_ac() {
	register int uac = 10;
#ifdef POLYSELF
	if (u.mtimedone) uac = mons[u.umonnum].ac;
#endif
	if(uarm) uac -= ARM_BONUS(uarm);
	if(uarmc) uac -= ARM_BONUS(uarmc);
	if(uarmh) uac -= ARM_BONUS(uarmh);
	if(uarmf) uac -= ARM_BONUS(uarmf);
	if(uarms) uac -= ARM_BONUS(uarms);
	if(uarmg) uac -= ARM_BONUS(uarmg);
#ifdef SHIRT
	if(uarmu) uac -= ARM_BONUS(uarmu);
#endif
	if(uleft && uleft->otyp == RIN_PROTECTION) uac -= uleft->spe;
	if(uright && uright->otyp == RIN_PROTECTION) uac -= uright->spe;
#ifdef THEOLOGY
	if (Protection & INTRINSIC) uac -= u.ublessed;
#endif
	if(uac != u.uac){
		u.uac = uac;
		flags.botl = 1;
	}
}

void
glibr()
{
	register struct obj *otmp;
	int xfl = 0;
#ifdef HARD
	boolean leftfall, rightfall;

	leftfall = (uleft && !uleft->cursed && (!uwep || !uwep->cursed));
	rightfall = (uright && !uright->cursed && (!uwep || !uwep->cursed
		|| !bimanual(uwep)));
#else
#define leftfall uleft
#define rightfall uright
#endif
	if(!uarmg) if(leftfall || rightfall)
#ifdef POLYSELF
				if(!nolimbs(uasmon))
#endif
						{
		/* Note: at present also cursed rings fall off */
		/* changed 10/30/86 by GAN */
		Your("%s off your %s.",
			(leftfall && rightfall) ? "rings slip" : "ring slips",
			makeplural(body_part(FINGER)));
		xfl++;
		if(leftfall) {
			otmp = uleft;
			Ring_off(uleft);
			dropx(otmp);
		}
		if(rightfall) {
			otmp = uright;
			Ring_off(uright);
			dropx(otmp);
		}
	}
	if(((otmp = uwep) != (struct obj *)0)
#ifdef HARD
	   && !otmp->cursed
#endif
	) {
		/* Note: at present also cursed weapons fall */
		/* changed 10/30/86 by GAN */
		Your("%s %sslips from your %s.",
			is_sword(uwep) ? "sword" : "weapon",
			xfl ? "also " : "",
			makeplural(body_part(HAND)));
		setuwep((struct obj *)0);
		dropx(otmp);
	}
}

struct obj *
some_armor(){
register struct obj *otmph = (uarmc ? uarmc : uarm);
	if(uarmh && (!otmph || !rn2(4))) otmph = uarmh;
	if(uarmg && (!otmph || !rn2(4))) otmph = uarmg;
	if(uarmf && (!otmph || !rn2(4))) otmph = uarmf;
	if(uarms && (!otmph || !rn2(4))) otmph = uarms;
#ifdef SHIRT
	if(!uarm && !uarmc && uarmu && (!otmph || !rn2(4))) otmph = uarmu;
#endif
	return(otmph);
}

void
corrode_armor(){
register struct obj *otmph = some_armor();

	if (otmph && otmph != uarmf) {
	    if (otmph->rustfree || objects[otmph->otyp].oc_material != METAL ||
		otmph->otyp >= LEATHER_ARMOR) {
			Your("%s not affected!",
				aobjnam(otmph, "are"));
			return;
	    }
	    Your("%s!", aobjnam(otmph, "corrode"));
	    otmph->spe--;
	    adj_abon(otmph, -1);
	}
}

static int
select_off(otmp)
register struct obj *otmp;
{
	if(!otmp) return(0);
	if(cursed(otmp)) return(0);
#ifdef POLYSELF
	if(otmp->olet==RING_SYM && nolimbs(uasmon)) return(0);
#endif
	if(welded(uwep) && (otmp==uarmg || otmp==uright || (otmp==uleft
			&& bimanual(uwep))))
		return(0);
	if(uarmg && uarmg->cursed && (otmp==uright || otmp==uleft)) {
		uarmg->bknown = 1;
		return(0);
	}
	if((otmp==uarm 
#ifdef SHIRT
			|| otmp==uarmu
#endif
					) && uarmc && uarmc->cursed) {
		uarmc->bknown = 1;
		return(0);
	}
#ifdef SHIRT
	if(otmp==uarmu && uarm && uarm->cursed) {
		uarm->bknown = 1;
		return(0);
	}
#endif

	if(otmp == uarm) takeoff_mask |= WORN_ARMOR;
	else if(otmp == uarmc) takeoff_mask |= WORN_CLOAK;
	else if(otmp == uarmf) takeoff_mask |= WORN_BOOTS;
	else if(otmp == uarmg) takeoff_mask |= WORN_GLOVES;
	else if(otmp == uarmh) takeoff_mask |= WORN_HELMET;
	else if(otmp == uarms) takeoff_mask |= WORN_SHIELD;
#ifdef SHIRT
	else if(otmp == uarmu) takeoff_mask |= WORN_SHIRT;
#endif
	else if(otmp == uleft) takeoff_mask |= LEFT_RING;
	else if(otmp == uright) takeoff_mask |= RIGHT_RING;
	else if(otmp == uamul) takeoff_mask |= WORN_AMUL;
	else if(otmp == ublindf) takeoff_mask |= WORN_BLINDF;
	else if(otmp == uwep) takeoff_mask |= 1L;	/* WIELDED_WEAPON */

	else impossible("select_off: %s???", doname(otmp));

	return(0);
}

static struct obj *
do_takeoff() {

	register struct obj *otmp = 0;

	if (taking_off == 1L) { /* weapon */
	  if(!cursed(uwep)) {
	    setuwep((struct obj *) 0);
	    You("are empty %s.", body_part(HANDED));
	  }
	} else if (taking_off ==  WORN_ARMOR) {
	  otmp = uarm;
	  if(!cursed(otmp)) (void) Armor_off();
	} else if (taking_off == WORN_CLOAK) {
	  otmp = uarmc;
	  if(!cursed(otmp)) (void) Cloak_off();
	} else if (taking_off == WORN_BOOTS) {
	  otmp = uarmf;
	  if(!cursed(otmp)) (void) Boots_off();
	} else if (taking_off == WORN_GLOVES) {
	  otmp = uarmg;
	  if(!cursed(otmp)) (void) Gloves_off();
	} else if (taking_off == WORN_HELMET) {
	  otmp = uarmh;
	  if(!cursed(otmp)) (void) Helmet_off();
	} else if (taking_off == WORN_SHIELD) {
	  otmp = uarms;
	  if(!cursed(otmp)) (void) Shield_off();
#ifdef SHIRT
	} else if (taking_off == WORN_SHIRT) {
	  otmp = uarmu;
	  if(!cursed(otmp))
	    setworn((struct obj *)0, uarmu->owornmask & W_ARMOR);
#endif
	} else if (taking_off == WORN_AMUL) {
	  otmp = uamul;
	  if(!cursed(otmp)) Amulet_off();
	} else if (taking_off == LEFT_RING) {
	  otmp = uleft;
	  if(!cursed(otmp)) Ring_off(uleft);
	} else if (taking_off == RIGHT_RING) {
	  otmp = uright;
	  if(!cursed(otmp)) Ring_off(uright);
	} else if (taking_off == WORN_BLINDF) {
	  if(!cursed(ublindf)) {
	    setworn((struct obj *)0, ublindf->owornmask);
	    if(!Blinded) make_blinded(1L,FALSE); /* See on next move */
	    else	 You("still cannot see.");
	  }
	} else impossible("do_takeoff: taking off %lx", taking_off);

	return(otmp);
}

static int
take_off() {

	register int i;
	register struct obj *otmp;

	if(taking_off) {
	    if(todelay > 0) {

		todelay--;
		return(1);	/* still busy */
	    } else if((otmp = do_takeoff())) off_msg(otmp);

	    takeoff_mask &= ~taking_off;
	    taking_off = 0L;
	}

	for(i = 0; takeoff_order[i]; i++)
	    if(takeoff_mask & takeoff_order[i]) {

		taking_off = takeoff_order[i];
		break;
	    }

	otmp = (struct obj *) 0;

	if (taking_off == 0L) {
	  You("finish disrobing.");
	  return 0;
	} else if (taking_off == 1L) {
	  todelay = 1;
	} else if (taking_off == WORN_ARMOR) {
	  otmp = uarm;
	} else if (taking_off == WORN_CLOAK) {
	  otmp = uarmc;
	} else if (taking_off == WORN_BOOTS) {
	  otmp = uarmf;
	} else if (taking_off == WORN_GLOVES) {
	  otmp = uarmg;
	} else if (taking_off == WORN_HELMET) {
	  otmp = uarmh;
	} else if (taking_off == WORN_SHIELD) {
	  otmp = uarms;
#ifdef SHIRT
	} else if (taking_off == WORN_SHIRT) {
	  otmp = uarmu;
#endif
	} else if (taking_off == WORN_AMUL) {
	  todelay = 1;
	} else if (taking_off == LEFT_RING) {
	  todelay = 1;
	} else if (taking_off == RIGHT_RING) {
	  todelay = 1;
	} else if (taking_off == WORN_BLINDF) {
	  todelay = 2;
	} else {
	  impossible("take_off: taking off %lx", taking_off);
	  return 0;	/* force done */
	}

	if(otmp) todelay = objects[otmp->otyp].oc_delay;
	set_occupation(take_off, "disrobing", 0);
	return(1);		/* get busy */
}

int
doddoremarm() {

	if(taking_off || takeoff_mask) {

	    You("continue disrobing.");
	    set_occupation(take_off, "disrobing", 0);
	    return(take_off());
	}

	(void) ggetobj("take off", select_off, 0);
	if(takeoff_mask) return(take_off());
	else		 return(0);
}

void
adj_abon(otmp, delta)
register struct obj *otmp;
register schar delta;
{
	if (uarmg && otmp->otyp == GAUNTLETS_OF_DEXTERITY) {
		ABON(A_DEX) += (delta);
		flags.botl = 1;
	}
	if (uarmh && otmp->otyp == HELM_OF_BRILLIANCE) {
		ABON(A_INT) += (delta);
		ABON(A_WIS) += (delta);
		flags.botl = 1;
	}
}
