/*	SCCS Id: @(#)worn.c	3.0	88/12/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static const char crispy[] = "The flames of Hell burn you to a crisp.";
static void set_armor_intrinsic P((struct obj *,long,BOOLEAN_P));

const struct worn {
	long w_mask;
	struct obj **w_obj;
} worn[] = {
	{ W_ARM, &uarm },
	{ W_ARMC, &uarmc },
	{ W_ARMH, &uarmh },
	{ W_ARMS, &uarms },
	{ W_ARMG, &uarmg },
	{ W_ARMF, &uarmf },
#ifdef SHIRT
	{ W_ARMU, &uarmu },
#endif
	{ W_RINGL, &uleft },
	{ W_RINGR, &uright },
	{ W_WEP, &uwep },
	{ W_AMUL, &uamul },
	{ W_TOOL, &ublindf },
	{ W_BALL, &uball },
	{ W_CHAIN, &uchain },
	{ 0, 0 }
};

void
setworn(obj, mask)
register struct obj *obj;
long mask;
{
	register struct worn *wp;
	register struct obj *oobj;

	for(wp = worn; wp->w_mask; wp++) if(wp->w_mask & mask) {
		oobj = *(wp->w_obj);
		if(oobj && !(oobj->owornmask & wp->w_mask))
			impossible("Setworn: mask = %ld.", wp->w_mask);
		if(oobj) {
		    oobj->owornmask &= ~wp->w_mask;
		    /* leave as "x = x <op> y", here and below, for broken
		     * compilers */
		    u.uprops[objects[oobj->otyp].oc_oprop].p_flgs = 
			    u.uprops[objects[oobj->otyp].oc_oprop].p_flgs & 
				~wp->w_mask;
		    set_armor_intrinsic(oobj, wp->w_mask, 0);
		}
		*(wp->w_obj) = obj;
		if(obj) {
		    obj->owornmask |= wp->w_mask;
		    u.uprops[objects[obj->otyp].oc_oprop].p_flgs = 
			    u.uprops[objects[obj->otyp].oc_oprop].p_flgs | 
				wp->w_mask;
		    set_armor_intrinsic(obj, wp->w_mask, 1);
		}
	}
	/* A kludge to solve the problem of someone gaining fire resistance
	 * only from an item, then entering Hell and removing/unwielding it.
	 * Checking this every time setworn gets called is a bit of an
	 * overkill. --KAA
	 */
	if (Inhell && !Fire_resistance) {
		pline(crispy);
		killer = "loss of fire protection";
		done(BURNING);
		/* If we're here they survived with life saving, so put the
		 * weapon they just unwielded back in their hands...
		 */
		if (oobj->otyp != DRAGON_SCALE_MAIL
				&& oobj->otyp != RIN_FIRE_RESISTANCE
#ifdef NAMED_ITEMS
				&& !defends(AD_FIRE, oobj)
#endif
				&& oobj->corpsenm != PM_RED_DRAGON)
			impossible("lost FR from a non-FR item?");
		setworn(oobj, mask);
	}
}

/* called e.g. when obj is destroyed */
void
setnotworn(obj)
register struct obj *obj;
{
	register struct worn *wp;

	if (!obj) return;
	for(wp = worn; wp->w_mask; wp++)
		if(obj == *(wp->w_obj)) {
			*(wp->w_obj) = 0;
			u.uprops[objects[obj->otyp].oc_oprop].p_flgs = 
				u.uprops[objects[obj->otyp].oc_oprop].p_flgs & 
					~wp->w_mask;
			obj->owornmask &= ~wp->w_mask;
			set_armor_intrinsic(obj, wp->w_mask, 0);
		}
	/* See comments above in setworn().  The major difference is the
	 * need to check AMULET_SYM so if someone goes to Hell without
	 * being fire resistant, then dies, when their amulet saves them
	 * and disintegrates this code will not be triggered. --KAA
	 */
	if (Inhell && !Fire_resistance && obj->olet != AMULET_SYM) {
		pline(crispy);
		killer = "loss of fire protection";
		done(BURNING);
		/* Survived with lifesaving, etc...; there's no general way
		 * to undo the setnotworn()--we can't re-wear/wield the
		 * item since it might have been stolen, disintegrated, etc....
		 */
#if defined(WIZARD) || defined(EXPLORE_MODE)
		while(1) {
			/* keep doing it until they finally decide they really
			 * _do_ want to die, since we can't possibly continue
			 * the game from this point...
			 */
#endif
			You("are still burning and die again...");
			done(BURNING);
#if defined(WIZARD) || defined(EXPLORE_MODE)
		}
#endif
	}
}

static void
set_armor_intrinsic(obj,maskbit,on)
register struct obj *obj;
long maskbit;	/* people can do funny things like wield armor */
boolean on;
{
	long *mask;

	if (obj->otyp != DRAGON_SCALE_MAIL) return;
	switch(obj->corpsenm) {
		case PM_GRAY_DRAGON:
			mask = &Antimagic;
			break;
		case PM_RED_DRAGON:
			mask = &HFire_resistance;
			break;
		case PM_WHITE_DRAGON:
			mask = &HCold_resistance;
			break;
		case PM_BLUE_DRAGON:
			mask = &HShock_resistance;
			break;
		case PM_GREEN_DRAGON:
			mask = &HPoison_resistance;
			break;
		case PM_ORANGE_DRAGON:
			mask = &HSleep_resistance;
			break;
		case PM_BLACK_DRAGON:
			mask = &HDisint_resistance;
			break;
		case PM_YELLOW_DRAGON:
		default:
			return;
	}
	if (on) *mask |= maskbit;
	else *mask &= ~maskbit;
}
