/*	SCCS Id: @(#)steal.c	3.2	96/04/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_PTR int NDECL(stealarm);

#ifdef OVLB
static const char * FDECL(equipname, (struct obj *));
static void FDECL(lose_worn_item, (struct obj *));

static const char *
equipname(otmp)
register struct obj *otmp;
{
	return (
#ifdef TOURIST
		(otmp == uarmu) ? "shirt" :
#endif
		(otmp == uarmf) ? "boots" :
		(otmp == uarms) ? "shield" :
		(otmp == uarmg) ? "gloves" :
		(otmp == uarmc) ? "cloak" :
		(otmp == uarmh) ? "helmet" : "armor");
}

long		/* actually returns something that fits in an int */
somegold()
{
#ifdef LINT	/* long conv. ok */
	return(0L);
#else
	return (long)( (u.ugold < 100) ? u.ugold :
		(u.ugold > 10000) ? rnd(10000) : rnd((int) u.ugold) );
#endif
}

void
stealgold(mtmp)
register struct monst *mtmp;
{
	register struct obj *gold = g_at(u.ux, u.uy);
	register long tmp;

	if (gold && ( !u.ugold || gold->quan > u.ugold || !rn2(5))) {
	    mtmp->mgold += gold->quan;
	    delobj(gold);
	    newsym(u.ux, u.uy);
	    pline("%s quickly snatches some gold from between your %s!",
		    Monnam(mtmp), makeplural(body_part(FOOT)));
	    if(!u.ugold || !rn2(5)) {
		if (!tele_restrict(mtmp)) rloc(mtmp);
		mtmp->mflee = 1;
	    }
	} else if(u.ugold) {
	    u.ugold -= (tmp = somegold());
	    Your("purse feels lighter.");
	    mtmp->mgold += tmp;
	    if (!tele_restrict(mtmp)) rloc(mtmp);
	    mtmp->mflee = 1;
	    flags.botl = 1;
	}
}

/* steal armor after you finish taking it off */
unsigned int stealoid;		/* object to be stolen */
unsigned int stealmid;		/* monster doing the stealing */

STATIC_PTR int
stealarm()
{
	register struct monst *mtmp;
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->o_id == stealoid) {
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if(mtmp->m_id == stealmid) {
			if(!dmgtype(mtmp->data, AD_SITM)) /* polymorphed */
			    goto botm;
			if(otmp->unpaid)
			    subfrombill(otmp, shop_keeper(*u.ushops));
			freeinv(otmp);
			pline("%s steals %s!", Monnam(mtmp), doname(otmp));
			mpickobj(mtmp,otmp);
			mtmp->mflee = 1;
			if (!tele_restrict(mtmp)) rloc(mtmp);
		        break;
		    }
		}
		break;
	    }
	}
botm:   stealoid = 0;
	return 0;
}

/* an object you're wearing has been stolen */
static void
lose_worn_item(obj)
struct obj *obj;
{
	if (donning(obj))
	    cancel_don();
	if (!obj->owornmask)
	    return;

	switch (obj->oclass) {
	 case TOOL_CLASS:
	    Blindf_off(obj);
	    break;
	 case AMULET_CLASS:
	    Amulet_off();
	    break;
	 case RING_CLASS:
	    Ring_gone(obj);
	    break;
	 case ARMOR_CLASS:
	    if (obj == uarm) (void) Armor_off();
	    else if (obj == uarmc) (void) Cloak_off();
	    else if (obj == uarmf) (void) Boots_off();
	    else if (obj == uarmg) (void) Gloves_off();
	    else if (obj == uarmh) (void) Helmet_off();
	    else if (obj == uarms) (void) Shield_off();
	    else setworn((struct obj *)0, obj->owornmask & W_ARMOR);
	    break;
	 default:
	    /* shouldn't reach here, but just in case... */
	    setnotworn(obj);
	    break;
	}
}

/* Returns 1 when something was stolen (or at least, when N should flee now)
 * Returns -1 if the monster died in the attempt
 * Avoid stealing the object stealoid
 */
int
steal(mtmp)
struct monst *mtmp;
{
	register struct obj *otmp;
	register int tmp;
	register int named = 0;

	/* the following is true if successful on first of two attacks. */
	if(!monnear(mtmp, u.ux, u.uy)) return(0);

	if (!invent || (inv_cnt() == 1 && uskin)) {
	    /* Not even a thousand men in armor can strip a naked man. */
	    if(Blind)
	      pline("Somebody tries to rob you, but finds nothing to steal.");
	    else
	      pline("%s tries to rob you, but she finds nothing to steal!",
		Monnam(mtmp));
	    return(1);	/* let her flee */
	}

	if(Adornment & LEFT_RING) {
	    otmp = uleft;
	    goto gotobj;
	} else if(Adornment & RIGHT_RING) {
	    otmp = uright;
	    goto gotobj;
	}

	tmp = 0;
	for(otmp = invent; otmp; otmp = otmp->nobj)
	    if ((!uarm || otmp != uarmc) && otmp != uskin)
		tmp += ((otmp->owornmask &
			(W_ARMOR | W_RING | W_AMUL | W_TOOL)) ? 5 : 1);
	tmp = rn2(tmp);
	for(otmp = invent; otmp; otmp = otmp->nobj)
	    if ((!uarm || otmp != uarmc) && otmp != uskin)
		if((tmp -= ((otmp->owornmask &
			(W_ARMOR | W_RING | W_AMUL | W_TOOL)) ? 5 : 1)) < 0)
			break;
	if(!otmp) {
		impossible("Steal fails!");
		return(0);
	}
	/* can't steal gloves while wielding - so steal the wielded item. */
	if (otmp == uarmg && uwep)
	    otmp = uwep;
	/* can't steal armor while wearing cloak - so steal the cloak. */
	else if(otmp == uarm && uarmc) otmp = uarmc;
#ifdef TOURIST
	else if(otmp == uarmu && uarmc) otmp = uarmc;
	else if(otmp == uarmu && uarm) otmp = uarm;
#endif
gotobj:
	if(otmp->o_id == stealoid) return(0);

	if(otmp->otyp == LEASH && otmp->leashmon) o_unleash(otmp);

	if((otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))){
		switch(otmp->oclass) {
		case TOOL_CLASS:
		case AMULET_CLASS:
		case RING_CLASS:
			lose_worn_item(otmp);
			break;
		case ARMOR_CLASS:
			/* Stop putting on armor which has been stolen. */
			if (donning(otmp)) {
			    lose_worn_item(otmp);
			    break;
			}
		    {	int curssv = otmp->cursed;
			otmp->cursed = 0;
			stop_occupation();
			if(flags.female)
			    pline("%s charms you.  You gladly %s your %s.",
				  Blind ? "She" : Monnam(mtmp),
				  curssv ? "let her take" : "hand over",
				  equipname(otmp));
			else
			    pline("%s seduces you and %s off your %s.",
				  Blind ? "It" : Adjmonnam(mtmp, "beautiful"),
				  curssv ? "helps you to take" :
	(objects[otmp->otyp].oc_delay > 1) ? "you start taking" : "you take",
				  equipname(otmp));
			named++;
			/* the following is to set multi for later on */
			nomul(-objects[otmp->otyp].oc_delay);
			lose_worn_item(otmp);
			otmp->cursed = curssv;
			if(multi < 0){
				/*
				multi = 0;
				nomovemsg = 0;
				afternmv = 0;
				*/
				stealoid = otmp->o_id;
				stealmid = mtmp->m_id;
				afternmv = stealarm;
				return(0);
			}
			break;
		    }
		default:
			impossible("Tried to steal a strange worn thing.");
		}
	}
	else if(otmp == uwep) uwepgone();

	if(otmp == uball) unpunish();

	freeinv(otmp);
	pline("%s stole %s.", named ? "She" : Monnam(mtmp), doname(otmp));
	mpickobj(mtmp,otmp);
	if (otmp->otyp == CORPSE && otmp->corpsenm == PM_COCKATRICE &&
		!(mtmp->misc_worn_check & W_ARMG)) {
	    minstapetrify(mtmp, TRUE);
	    return -1;
	}
	return((multi < 0) ? 0 : 1);
}

#endif /* OVLB */
#ifdef OVL1

void
mpickobj(mtmp,otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
    if (otmp->oclass == GOLD_CLASS) {
	mtmp->mgold += otmp->quan;
	obfree(otmp, (struct obj *)0);
    } else {
	boolean snuff_otmp = FALSE;
	/* don't want hidden light source inside the monster; assumes that
	   engulfers won't have external inventories; whirly monsters cause
	   the light to be extinguished rather than letting it shine thru */
	if (otmp->lamplit &&  /* hack to avoid function calls for most objs */
		obj_sheds_light(otmp) &&
		attacktype(mtmp->data, AT_ENGL)) {
	    /* this is probably a burning object that you dropped or threw */
	    if (u.uswallow && mtmp == u.ustuck && !Blind)
		pline("%s go%s out.", The(xname(otmp)),
		      otmp->quan == 1L ? "es" : "");
	    snuff_otmp = TRUE;
	}
	/* add_to_minv() might free otmp [if merged with something else],
	   so we have to call it after doing the object checks */
	add_to_minv(mtmp, otmp);
	/* and we had to defer this until object is in mtmp's inventory */
	if (snuff_otmp) snuff_light_source(mtmp->mx, mtmp->my);
    }
}

#endif /* OVL1 */
#ifdef OVLB

void
stealamulet(mtmp)
register struct monst *mtmp;
{
	register struct obj *otmp;
	register int	real, fake;

	/* select the artifact to steal */
	if(u.uhave.amulet) {
		real = AMULET_OF_YENDOR ;
		fake = FAKE_AMULET_OF_YENDOR ;
	} else if(u.uhave.questart) {
	    real = fake = 0;		/* gcc -Wall lint */
	    for(otmp = invent; otmp; otmp = otmp->nobj)
	        if(is_quest_artifact(otmp)) goto snatch_it;
	} else if(u.uhave.bell) {
		real = BELL_OF_OPENING;
		fake = BELL;
	} else if(u.uhave.book) {
		real = SPE_BOOK_OF_THE_DEAD;
		fake = 0;
	} else if(u.uhave.menorah) {
		real = CANDELABRUM_OF_INVOCATION;
		fake = 0;
	} else return;	/* you have nothing of special interest */

/*	If we get here, real and fake have been set up. */
	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->otyp == real || (otmp->otyp == fake && !mtmp->iswiz)) {
		/* might be an imitation one */
snatch_it:
		if (otmp->owornmask)
		    lose_worn_item(otmp);
		freeinv(otmp);
		mpickobj(mtmp,otmp);
		pline("%s stole %s!", Monnam(mtmp), doname(otmp));
		if (can_teleport(mtmp->data) && !tele_restrict(mtmp))
			rloc(mtmp);
		return;
	    }
	}
}

#endif /* OVLB */
#ifdef OVL0

/* release the objects the creature is carrying */
void
relobj(mtmp,show,is_pet)
register struct monst *mtmp;
register int show;
boolean is_pet;		/* If true, pet should keep wielded/worn items */
{
	register struct obj *otmp;
	register int omx = mtmp->mx, omy = mtmp->my;
	struct obj *keepobj = 0;
	struct obj *wep = MON_WEP(mtmp);
	boolean item1 = FALSE, item2 = FALSE;

	if (!is_pet || mindless(mtmp->data) || is_animal(mtmp->data))
		item1 = item2 = TRUE;
	if (!tunnels(mtmp->data) || !needspick(mtmp->data))
		item1 = TRUE;
	while ((otmp = mtmp->minvent) != 0) {
		obj_extract_self(otmp);
		/* special case: pick-axe and unicorn horn are non-worn */
		/* items that we also want pets to keep 1 of */
		/* (It is a coincidence that these can also be wielded. */
		if (otmp->owornmask || otmp == wep ||
		    ((!item1 && otmp->otyp == PICK_AXE) ||
		     (!item2 && otmp->otyp == UNICORN_HORN && !otmp->cursed))) {
			if (is_pet) { /* dont drop worn/wielded item */
				if (otmp->otyp == PICK_AXE)
					item1 = TRUE;
				if (otmp->otyp == UNICORN_HORN && !otmp->cursed)
					item2 = TRUE;
				otmp->nobj = keepobj;
				keepobj = otmp;
				continue;
			}
			mtmp->misc_worn_check &= ~(otmp->owornmask);
			otmp->owornmask = 0L;
		}
		if (is_pet && cansee(omx, omy) && flags.verbose)
			pline("%s drops %s.", Monnam(mtmp),
					distant_name(otmp, doname));
		if (flooreffects(otmp, omx, omy, "fall")) continue;
		place_object(otmp, omx, omy);
		stackobj(otmp);
	}
	/* put kept objects back */
	while ((otmp = keepobj) != (struct obj *)0) {
	    keepobj = otmp->nobj;
	    add_to_minv(mtmp, otmp);
	}

	if (mtmp->mgold) {
		register long g = mtmp->mgold;
		mkgold(g, omx, omy);
		if (is_pet && cansee(omx, omy) && flags.verbose)
			pline("%s drops %ld gold piece%s.", Monnam(mtmp),
				g, plur(g));
		mtmp->mgold = 0L;
	}
	if (show & cansee(omx, omy))
		newsym(omx, omy);
}

#endif /* OVL0 */

/*steal.c*/
