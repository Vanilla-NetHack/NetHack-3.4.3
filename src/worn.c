/*	SCCS Id: @(#)worn.c	3.2	96/03/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static void FDECL(m_lose_armor, (struct monst *,struct obj *));

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
#ifdef TOURIST
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

/* this only allows for one blocking item per property;
   to be general, we'd need a separate uprops[].i_blocked
   field rather than just a single bit in uprops[].p_flgs */
#define w_blocks(otmp)	\
		((otmp->otyp == MUMMY_WRAPPING) ? INVIS :	\
		 (otmp->otyp == CORNUTHAUM && !Role_is('W')) ? CLAIRVOYANT : 0)
		/* note: monsters don't have clairvoyance, so your role
		   has no significant effect on their use of w_blocks() */

void
setworn(obj, mask)
register struct obj *obj;
long mask;
{
	register const struct worn *wp;
	register struct obj *oobj;
	register int p;

	if ((mask & (W_ARM|I_SPECIAL)) == (W_ARM|I_SPECIAL)) {
	    /* restoring saved game; no properties are conferred via skin */
	    uskin = obj;
	 /* assert( !uarm ); */
	} else {
	    for(wp = worn; wp->w_mask; wp++) if(wp->w_mask & mask) {
		oobj = *(wp->w_obj);
		if(oobj && !(oobj->owornmask & wp->w_mask))
			impossible("Setworn: mask = %ld.", wp->w_mask);
		if(oobj) {
		    oobj->owornmask &= ~wp->w_mask;
		    /* leave as "x = x <op> y", here and below, for broken
		     * compilers */
		    p = objects[oobj->otyp].oc_oprop;
		    u.uprops[p].p_flgs = u.uprops[p].p_flgs & ~wp->w_mask;
		    if (oobj->oartifact) set_artifact_intrinsic(oobj, 0, mask);
		    if ((p = w_blocks(oobj)) != 0)
			u.uprops[p].p_flgs &= ~I_BLOCKED;
		}
		*(wp->w_obj) = obj;
		if(obj) {
		    obj->owornmask |= wp->w_mask;
		/* prevent getting intrinsics from wielding potions, etc... */
		/* wp_mask should be same as mask at this point */
		    if(obj->oclass == WEAPON_CLASS || mask != W_WEP) {
			p = objects[obj->otyp].oc_oprop;
			u.uprops[p].p_flgs = u.uprops[p].p_flgs | wp->w_mask;
		    }
		    if (obj->oartifact) set_artifact_intrinsic(obj, 1, mask);
		    if ((p = w_blocks(obj)) != 0)
			u.uprops[p].p_flgs |= I_BLOCKED;
		}
	    }
	}
	update_inventory();
}

/* called e.g. when obj is destroyed */
void
setnotworn(obj)
register struct obj *obj;
{
	register const struct worn *wp;
	register int p;

	if (!obj) return;
	for(wp = worn; wp->w_mask; wp++)
		if(obj == *(wp->w_obj)) {
			*(wp->w_obj) = 0;
			p = objects[obj->otyp].oc_oprop;
			u.uprops[p].p_flgs = u.uprops[p].p_flgs & ~wp->w_mask;
			obj->owornmask &= ~wp->w_mask;
			if (obj->oartifact)
			    set_artifact_intrinsic(obj, 0, wp->w_mask);
			if ((p = w_blocks(obj)) != 0)
			    u.uprops[p].p_flgs &= ~I_BLOCKED;
		}
	update_inventory();
}

void
mon_set_minvis(mon)
struct monst *mon;
{
	mon->perminvis = 1;
	if (!mon->invis_blkd) {
	    mon->minvis = 1;
	    newsym(mon->mx, mon->my);		/* make it disappear */
	    if (mon->wormno) see_wsegs(mon);	/* and any tail too */
	}
}

/* armor put on or taken off; might be magical variety */
void
update_mon_intrinsics(mon, obj, on)
struct monst *mon;
struct obj *obj;
boolean on;
{
	int unseen = !canseemon(mon);

	switch (objects[obj->otyp].oc_oprop) {
	 case INVIS:
	    mon->minvis = on ? !mon->invis_blkd : mon->perminvis;
	    break;
	 default:
	    break;
	}
	switch (w_blocks(obj)) {
	 case INVIS:
	    mon->invis_blkd = on ? 1 : 0;
	    mon->minvis = on ? 0 : mon->perminvis;
	    break;
	 default:
	    break;
	}

	/* if couldn't see it but now can, or vice versa, update display */
	if (unseen ^ !canseemon(mon))
	    newsym(mon->mx, mon->my);
}

int
find_mac(mon)
register struct monst *mon;
{
	register struct obj *obj;
	int base = mon->data->ac;
	long mwflags = mon->misc_worn_check;

	for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->owornmask & mwflags)
		base -= ARM_BONUS(obj);
		/* since ARM_BONUS is positive, subtracting it increases AC */
	}
	return base;
}

/* weapons are handled separately; rings and eyewear aren't used by monsters */
#define m_might_wear(O) ((O)->oclass == ARMOR_CLASS ||	\
			 (O)->oclass == AMULET_CLASS)

/* Wear first object of that type it finds, and never switch unless it
 * has none at all.  This means that monsters with leather armor never
 * switch to plate mail, but it also avoids the overhead of either having 8
 * struct obj *s for every monster in the game, or of doing multiple inventory
 * searches each round using which_armor().
 *
 * We'll let monsters put on shirts and/or suits under worn cloaks, but
 * not shirts under worn suits.  This is somewhat arbitrary, but it's
 * too tedious to have them remove and later replace outer garments,
 * and preventing suits under cloaks makes it a little bit too easy for
 * players to influence what gets worn.  Putting on a shirt underneath
 * already worn body armor is too obviously buggy...
 */
void
m_dowear(mon, creation)
register struct monst *mon;
boolean creation;
{
	register struct obj *obj;

	/* Note the restrictions here are the same as in dowear in do_wear.c
	 * except for the additional restriction on intelligence.  (Players
	 * are always intelligent, even if polymorphed).
	 */
	if (verysmall(mon->data) || nohands(mon->data) || is_animal(mon->data))
		return;
	/* give mummies a chance to wear their wrappings */
	if (mindless(mon->data) && (mon->data->mlet != S_MUMMY || !creation))
		return;

	for(obj = mon->minvent; obj; obj = obj->nobj) {
		long flag;
		int m_delay = 0;

		/* if already worn or never wearable, skip it right away */
		if (obj->owornmask || !m_might_wear(obj)) continue;

		if (obj->oclass == AMULET_CLASS) {
			if (obj->otyp != AMULET_OF_LIFE_SAVING &&
			    obj->otyp != AMULET_OF_REFLECTION)
				continue;
			flag = W_AMUL;
#ifdef TOURIST
		} else if (is_shirt(obj)) {
			if (cantweararm(mon->data) ||
			    /* can't put on shirt if already wearing suit */
			    (mon->misc_worn_check & W_ARM))
				continue;
			flag = W_ARMU;
			m_delay = (mon->misc_worn_check & W_ARMC) ? 2 : 0;
#endif
		} else if (is_cloak(obj)) {
			/* treating small as a special case allows
			   hobbits, gnomes, and kobolds to wear cloaks */
			if (cantweararm(mon->data) &&
			    mon->data->msize != MZ_SMALL)
				continue;
			flag = W_ARMC;
		} else if (is_helmet(obj)) {
			flag = W_ARMH;
		} else if (is_shield(obj)) {
			if (MON_WEP(mon) && bimanual(MON_WEP(mon)))
				continue;
			flag = W_ARMS;
		} else if (is_gloves(obj)) {
			if (MON_WEP(mon) && MON_WEP(mon)->cursed)
				continue;
			flag = W_ARMG;
		} else if (is_boots(obj)) {
			if (slithy(mon->data) || mon->data->mlet == S_CENTAUR)
				continue;
			flag = W_ARMF;
		} else if (obj->oclass == ARMOR_CLASS) {
			if (cantweararm(mon->data))
				continue;
			flag = W_ARM;
			m_delay = (mon->misc_worn_check & W_ARMC) ? 2 : 0;
		} else
			continue;		/* shouldn't be possible */

		if (mon->misc_worn_check & flag) continue;
			/* already wearing one */
		if (!creation && canseemon(mon)) {
			pline("%s puts on %s.", Monnam(mon),
						distant_name(obj, doname));
			m_delay += objects[obj->otyp].oc_delay;
			mon->mfrozen = m_delay;
			if (mon->mfrozen) mon->mcanmove = 0;
		}
		mon->misc_worn_check |= flag;
		obj->owornmask |= flag;
		update_mon_intrinsics(mon, obj, TRUE);
		if (mon->mfrozen) break;		/* now busy */
	}
}

struct obj *
which_armor(mon, flag)
struct monst *mon;
long flag;
{
	register struct obj *obj;

	for(obj = mon->minvent; obj; obj = obj->nobj)
		if (obj->owornmask & flag) return obj;
	return((struct obj *)0);
}

/* remove an item of armor and then drop it */
static void
m_lose_armor(mon, obj)
struct monst *mon;
struct obj *obj;
{
	mon->misc_worn_check &= ~obj->owornmask;
	obj->owornmask = 0L;
	update_mon_intrinsics(mon, obj, FALSE);

	obj_extract_self(obj);
	place_object(obj, mon->mx, mon->my);
	/* call stackobj() if we ever drop anything that can merge */
	newsym(mon->mx, mon->my);
}

void
mon_break_armor(mon)
struct monst *mon;
{
	register struct obj *otmp;
	struct permonst *mdat = mon->data;
	boolean vis = cansee(mon->mx, mon->my);
	const char *pronoun = him[pronoun_gender(mon)],
			*ppronoun = his[pronoun_gender(mon)];

	if (breakarm(mdat)) {
	    if ((otmp = which_armor(mon, W_ARM)) != 0) {
		if (vis)
		    pline("%s breaks out of %s armor!", Monnam(mon), ppronoun);
		else
		    You_hear("a cracking sound.");
		m_useup(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMC)) != 0) {
		if (otmp->oartifact) {
		    if (vis)
			pline("%s cloak falls off!", s_suffix(Monnam(mon)));
		    m_lose_armor(mon, otmp);
		} else {
		    if (vis)
			pline("%s cloak tears apart!", s_suffix(Monnam(mon)));
		    else
			You_hear("a ripping sound.");
		    m_useup(mon, otmp);
		}
	    }
#ifdef TOURIST
	    if ((otmp = which_armor(mon, W_ARMU)) != 0) {
		if (vis)
		    pline("%s shirt rips to shreds!", s_suffix(Monnam(mon)));
		else
		    You_hear("a ripping sound.");
		m_useup(mon, otmp);
	    }
#endif
	} else if (sliparm(mdat)) {
	    if ((otmp = which_armor(mon, W_ARM)) != 0) {
		if (vis)
		    pline("%s armor falls around %s!",
			         s_suffix(Monnam(mon)), pronoun);
		else
		    You_hear("a thud.");
		m_lose_armor(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMC)) != 0) {
		if (vis)
		    if (is_whirly(mon->data))
			pline("%s cloak falls, unsupported!",
			             s_suffix(Monnam(mon)));
		    else
			pline("%s shrinks out of %s cloak!", Monnam(mon),
								ppronoun);
		m_lose_armor(mon, otmp);
	    }
#ifdef TOURIST
	    if ((otmp = which_armor(mon, W_ARMU)) != 0) {
		if (vis)
		    if (sliparm(mon->data))
			pline("%s seeps right through %s shirt!",
					Monnam(mon), ppronoun);
		    else
			pline("%s becomes much too small for %s shirt!",
					Monnam(mon), ppronoun);
		m_lose_armor(mon, otmp);
	    }
#endif
	}
	if (nohands(mdat) || verysmall(mdat)) {
	    if ((otmp = which_armor(mon, W_ARMG)) != 0) {
		if (vis)
		    pline("%s drops %s gloves%s!", Monnam(mon), ppronoun,
					MON_WEP(mon) ? " and weapon" : "");
		possibly_unwield(mon);
		m_lose_armor(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMS)) != 0) {
		if (vis)
		    pline("%s can no longer hold %s shield!", Monnam(mon),
								ppronoun);
		else
		    You_hear("a clank.");
		m_lose_armor(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMH)) != 0) {
		if (vis)
		    pline("%s helmet falls to the %s!",
			  s_suffix(Monnam(mon)), surface(mon->mx, mon->my));
		else
		    You_hear("a clank.");
		m_lose_armor(mon, otmp);
	    }
	}
	if (nohands(mdat) || verysmall(mdat) || slithy(mdat) ||
	    mdat->mlet == S_CENTAUR) {
	    if ((otmp = which_armor(mon, W_ARMF)) != 0) {
		if (vis) {
		    if (is_whirly(mon->data))
			pline("%s boots fall away!",
			               s_suffix(Monnam(mon)));
		    else pline("%s boots %s off %s feet!",
			s_suffix(Monnam(mon)),
			verysmall(mdat) ? "slide" : "are pushed", ppronoun);
		}
		m_lose_armor(mon, otmp);
	    }
	}
}

/*worn.c*/
