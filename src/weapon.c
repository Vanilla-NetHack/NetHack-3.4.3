/*	SCCS Id: @(#)weapon.c	3.1	93/02/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	This module contains code for calculation of "to hit" and damage
 *	bonuses for any given weapon used, as well as weapons selection
 *	code for monsters.
 */
#include	"hack.h"

#ifdef OVLB

static NEARDATA const char kebabable[] = { S_XORN, S_DRAGON, S_NAGA, S_GIANT, 0 };

/*
 *	hitval returns an integer representing the "to hit" bonuses
 *	of "otmp" against the monster type "ptr".
 */
int
hitval(otmp, ptr)
struct	obj *otmp;
struct	permonst *ptr;
{
	int	tmp = 0;

	if (otmp->oclass == WEAPON_CLASS ||
	    otmp->otyp == PICK_AXE || otmp->otyp == UNICORN_HORN)
		tmp += otmp->spe;

/*	Put weapon specific "to hit" bonuses in below:		*/
	tmp += objects[otmp->otyp].oc_hitbon;

/*	Put weapon vs. monster type "to hit" bonuses in below:	*/

	/* Blessed weapons used against undead or demons */
	if(otmp->oclass == WEAPON_CLASS && otmp->blessed &&
	   (is_demon(ptr) || is_undead(ptr))) tmp += 2;

	if(otmp->otyp >= SPEAR && otmp->otyp <= JAVELIN &&
	   index(kebabable, ptr->mlet)) tmp += 2;

	/* Check specially named weapon "to hit" bonuses */
	if (otmp->oartifact) tmp += spec_abon(otmp, ptr);
	return tmp;
}

/*
 *	dmgval returns an integer representing the damage bonuses
 *	of "otmp" against the monster type "ptr".
 */
int
dmgval(otmp, ptr)
struct	obj *otmp;
struct	permonst *ptr;
{
	int	tmp = 0;

	if(otmp->otyp == CREAM_PIE)	return(0);

	if(ptr->msize >= MZ_HUMAN) {
	    if(objects[otmp->otyp].oc_wldam)
		tmp = rnd(objects[otmp->otyp].oc_wldam);
	    switch (otmp->otyp) {
		case CROSSBOW_BOLT:
		case MORNING_STAR:
		case PARTISAN:
		case ELVEN_BROADSWORD:
		case BROADSWORD:	tmp++; break;

		case FLAIL:
		case RANSEUR:
		case VOULGE:		tmp += rnd(4); break;

		case ACID_VENOM:
		case HALBERD:
		case SPETUM:		tmp += rnd(6); break;

		case BATTLE_AXE:
		case BARDICHE:
		case TRIDENT:		tmp += d(2,4); break;

		case TSURUGI:
		case DWARVISH_MATTOCK:
		case TWO_HANDED_SWORD:	tmp += d(2,6); break;
	    }
	} else {
	    if(objects[otmp->otyp].oc_wsdam)
		tmp = rnd(objects[otmp->otyp].oc_wsdam);
	    switch (otmp->otyp) {
		case CROSSBOW_BOLT:
		case MACE:
		case WAR_HAMMER:
		case FLAIL:
		case SPETUM:
		case TRIDENT:		tmp++; break;

		case BATTLE_AXE:
		case BARDICHE:
		case BILL_GUISARME:
		case GUISARME:
		case LUCERN_HAMMER:
		case MORNING_STAR:
		case RANSEUR:
		case BROADSWORD:
		case ELVEN_BROADSWORD:
		case VOULGE:		tmp += rnd(4); break;

		case ACID_VENOM:	tmp += rnd(6); break;
	    }
	}
	if (otmp->oclass == WEAPON_CLASS || otmp->otyp == PICK_AXE
						|| otmp->otyp == UNICORN_HORN)
		tmp += otmp->spe;

	tmp -= otmp->oeroded;

	if (objects[otmp->otyp].oc_material <= LEATHER && thick_skinned(ptr))
		/* thick skinned/scaled creatures don't feel it */
		tmp = 0;
	if (ptr == &mons[PM_SHADE] && objects[otmp->otyp].oc_material != SILVER)
		tmp = 0;

/*	Put weapon vs. monster type damage bonuses in below:	*/
	if(otmp->oclass == WEAPON_CLASS) {
	    if (otmp->blessed && (is_undead(ptr) || is_demon(ptr)))
		tmp += rnd(4);
	    if ((otmp->otyp == AXE || otmp->otyp == BATTLE_AXE)
		&& is_wooden(ptr))
		tmp += rnd(4);
	    if (objects[otmp->otyp].oc_material == SILVER && hates_silver(ptr))
		tmp += rnd(20);
	}

	return(tmp);
}

void
set_uasmon()		/* update the "uasmon" structure */
{
#ifdef POLYSELF
	if(u.umonnum >= 0) uasmon = &mons[u.umonnum];
	else {
#endif

		uasmon = &playermon;
		playermon.mlevel = u.ulevel;
		playermon.ac = u.uac;
		playermon.mr = (u.ulevel > 8) ? 5 * (u.ulevel-7) : u.ulevel;
#ifdef POLYSELF
	}
#endif
	return;
}

#endif /* OVLB */
#ifdef OVL0

#define	Oselect(x)	if((otmp = oselect(mtmp, x))) return(otmp);

static struct obj * FDECL(oselect, (struct monst *,int));

static struct obj *
oselect(mtmp, x)
struct monst *mtmp;
int x;
{
	struct obj *otmp;

	for(otmp=mtmp->minvent; otmp; otmp = otmp->nobj) {
		if (otmp->otyp == x && touch_artifact(otmp,mtmp)
#ifdef MUSE
			 && !(x == CORPSE && otmp->corpsenm != PM_COCKATRICE)
#endif
									)
			return otmp;
	}
	return (struct obj *)0;
}

static NEARDATA const int rwep[] =
	{ DWARVISH_SPEAR, ELVEN_SPEAR, SPEAR, ORCISH_SPEAR, JAVELIN,
	  SHURIKEN, SILVER_ARROW, ELVEN_ARROW, ARROW, ORCISH_ARROW,
	  CROSSBOW_BOLT, ELVEN_DAGGER, DAGGER, ORCISH_DAGGER, KNIFE, ROCK,
	  LOADSTONE, LUCKSTONE, DART, /* BOOMERANG, */ CREAM_PIE
	  /* note: CREAM_PIE should NOT be #ifdef KOPS */
	  };

static struct obj *propellor;

struct obj *
select_rwep(mtmp)	/* select a ranged weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	int i;

#ifdef KOPS
	char mlet = mtmp->data->mlet;
#endif

	propellor = &zeroobj;
#ifdef KOPS
	if(mlet == S_KOP)	/* pies are first choice for Kops */
	    Oselect(CREAM_PIE);
#endif
	if(throws_rocks(mtmp->data))	/* ...boulders for giants */
	    Oselect(BOULDER);
	/*
	 * other than these two specific cases, always select the
	 * most potent ranged weapon to hand.
	 */
	for (i = 0; i < SIZE(rwep); i++) {
	    int prop;

	    propellor = &zeroobj;
	    /* shooting gems from slings; this goes just before the darts */
	    if (rwep[i]==DART && !likes_gems(mtmp->data)
			&& (propellor = m_carrying(mtmp, SLING))) {
		for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
		    if(otmp->oclass==GEM_CLASS &&
				(otmp->otyp != LOADSTONE || !otmp->cursed))
			return(otmp);
		}
	    }
	    prop = (objects[rwep[i]]).w_propellor;
	    if (prop > 0) {
		switch (prop) {
		case WP_BOW:
		  propellor = (oselect(mtmp, ELVEN_BOW));
		  if (!propellor) propellor = (oselect(mtmp, BOW));
		  if (!propellor) propellor = (oselect(mtmp, ORCISH_BOW));
		  break;
		case WP_SLING:
		  propellor = (oselect(mtmp, SLING));
		  break;
		case WP_CROSSBOW:
		  propellor = (oselect(mtmp, CROSSBOW));
		}
#ifdef MUSE
		if ((otmp = MON_WEP(mtmp)) && otmp->cursed && otmp != propellor
				&& mtmp->weapon_check == NO_WEAPON_WANTED)
			propellor = 0;
#endif
	    }
	    /* propellor = obj, propellor to use
	     * propellor = &zeroobj, doesn't need a propellor
	     * propellor = 0, needed one and didn't have one
	     */
	    if (propellor != 0) {
		/* Note: cannot use m_carrying for loadstones, since it will
		 * always select the first object of a type, and maybe the
		 * monster is carrying two but only the first is unthrowable.
		 */
		if (rwep[i] != LOADSTONE) {
#ifdef MUSE
			/* Don't throw a cursed weapon-in-hand */
			if ((otmp = oselect(mtmp, rwep[i]))
			    && (!otmp->cursed || otmp != MON_WEP(mtmp)))
				return(otmp);
#else
			Oselect(rwep[i]);
#endif
		} else for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
		    if (otmp->otyp == LOADSTONE && !otmp->cursed)
			return otmp;
		}
	    }
	  }

	/* failure */
	return (struct obj *)0;
}

/* 0 = used by any monster; 1 = only used by strong monsters */
static NEARDATA const struct hwep { short otyp, big; } hwep[] = {
#ifdef MUSE
	  {CORPSE,0},  /* cockatrice corpse */
#endif
	  {TSURUGI,1}, {RUNESWORD,0},
	  {DWARVISH_MATTOCK,1}, {TWO_HANDED_SWORD,1}, {BATTLE_AXE,1},
	  {KATANA,0}, {UNICORN_HORN,1}, {CRYSKNIFE,0},
	  {TRIDENT,0}, {LONG_SWORD,0}, {ELVEN_BROADSWORD,0}, {BROADSWORD,0},
	  {LUCERN_HAMMER,1}, {SCIMITAR,1}, {SILVER_SABER,0}, {HALBERD,1},
	  {PARTISAN,1}, {LANCE,1}, {FAUCHARD,1}, {BILL_GUISARME,1},
	  {BEC_DE_CORBIN,1}, {GUISARME,1}, {RANSEUR,1}, {SPETUM,1},
	  {VOULGE,1}, {BARDICHE,0}, {MORNING_STAR,0}, {GLAIVE,0},
	  {ELVEN_SHORT_SWORD,0}, {DWARVISH_SHORT_SWORD,0}, {SHORT_SWORD,0},
	  {ORCISH_SHORT_SWORD,0}, {MACE,0}, {AXE,0}, {DWARVISH_SPEAR,0},
	  {ELVEN_SPEAR,0}, {SPEAR,0}, {ORCISH_SPEAR,0}, {FLAIL,0},
	  {QUARTERSTAFF,1}, {JAVELIN,0}, {AKLYS,0}, {CLUB,0}, {PICK_AXE,0},
#ifdef KOPS
	  {RUBBER_HOSE,0},
#endif /* KOPS */
	  {WAR_HAMMER,0}, {ELVEN_DAGGER,0}, {DAGGER,0}, {ORCISH_DAGGER,0},
	  {ATHAME,0}, {SCALPEL,0}, {KNIFE,0}, {WORM_TOOTH,0}, {BULLWHIP,0}
	};

struct obj *
select_hwep(mtmp)	/* select a hand to hand weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	register int i;
	register const struct hwep *hw;
	boolean strong = strongmonst(mtmp->data);

	if(is_giant(mtmp->data))	/* giants just love to use clubs */
	    Oselect(CLUB);

	/* only strong monsters can wield big (esp. long) weapons */
	/* all monsters can wield the remaining weapons */
	for (i = 0, hw = hwep; i < SIZE(hwep); i++, hw++)
	    if ((strong || !hw->big) &&
#ifdef MUSE
	      (!objects[hw->otyp].oc_bimanual ||
					(mtmp->misc_worn_check & W_ARMS)) &&
#endif
	(objects[hw->otyp].oc_material != SILVER || !hates_silver(mtmp->data)))
		Oselect(hw->otyp);

	/* failure */
	return (struct obj *)0;
}

#ifdef MUSE
/* Called after polymorphing a monster, robbing it, etc....  Monsters
 * otherwise never unwield stuff on their own.  Shouldn't print messages.
 */
void
possibly_unwield(mon)
register struct monst *mon;
{
	register struct obj *obj;
	struct obj *otmp, *backobj, *mw_tmp;

	if (!(mw_tmp = MON_WEP(mon)))
		return;
	for(obj=mon->minvent; obj; obj=obj->nobj)
		if (obj == mw_tmp) break;
	if (!obj) { /* The weapon was stolen or destroyed */
		MON_NOWEP(mon);
		mon->weapon_check = NEED_WEAPON;
		return;
	}
	if (!attacktype(mon->data, AT_WEAP)) {
		MON_NOWEP(mon);
		mon->weapon_check = NO_WEAPON_WANTED;
		if (cansee(mon->mx, mon->my)) {
			pline("%s drops %s.", Monnam(mon),
				distant_name(obj, doname));
		}
		backobj = 0;
		for(otmp = mon->minvent; otmp; otmp = otmp->nobj) {
			/* flooreffects unnecessary, can't wield boulders */
			if (otmp == obj) {
				if (!backobj) mon->minvent = otmp->nobj;
				else backobj->nobj = otmp->nobj;
				place_object(otmp, mon->mx, mon->my);
				otmp->nobj = fobj;
				fobj = otmp;
				stackobj(fobj);
				if(cansee(mon->mx,mon->my))
					newsym(mon->mx, mon->my);
				break;
			}
			backobj = otmp;
		}
		return;
	}
	/* The remaining case where there is a change is where a monster
	 * is polymorphed into a stronger/weaker monster with a different
	 * choice of weapons.  This has no parallel for players.  It can
	 * be handled by waiting until mon_wield_item is actually called.
	 * Though the monster still wields the wrong weapon until then,
	 * this is OK since the player can't see it.
	 * Note that if there is no change, setting the check to NEED_WEAPON
	 * is harmless.
	 * Possible problem: big monster with big cursed weapon gets
	 * polymorphed into little monster.  But it's not quite clear how to
	 * handle this anyway....
	 */
	mon->weapon_check = NEED_WEAPON;
	return;
}

/* Let a monster try to wield a weapon, based on mon->weapon_check.
 * Returns 1 if the monster took time to do it, 0 if it did not.
 */
int
mon_wield_item(mon)
register struct monst *mon;
{
	struct obj *obj;

	/* This case actually should never happen */
	if (mon->weapon_check == NO_WEAPON_WANTED) return 0;

	switch(mon->weapon_check) {
		case NEED_HTH_WEAPON:
			obj = select_hwep(mon);
			break;
		case NEED_RANGED_WEAPON:
			(void)select_rwep(mon);
			obj = propellor;
			break;
		case NEED_PICK_AXE:
			obj = m_carrying(mon, PICK_AXE);
			break;
		default: impossible("weapon_check %d for %s?",
				mon->weapon_check, mon_nam(mon));
			return 0;
	}
	if (obj && obj != &zeroobj) {
		struct obj *mw_tmp = MON_WEP(mon);
		if (mw_tmp == obj) { /* already wielding it */
			mon->weapon_check = NEED_WEAPON;
			return 0;
		}
		/* Actually, this isn't necessary--as soon as the monster
		 * wields the weapon, the weapon welds itself, so the monster
		 * can know it's cursed and needn't even bother trying.
		 * Still....
		 */
		if (mw_tmp && mw_tmp->cursed && mw_tmp->otyp != CORPSE) {
		    if (canseemon(mon)) {
			if (obj->otyp == PICK_AXE) {
			    pline("Since %s weapon %s welded to %s hand,",
				  s_suffix(mon_nam(mon)),
				  (mw_tmp->quan == 1L) ? "is" : "are",
				  his[pronoun_gender(mon)]);
			    pline("%s cannot wield that %s.",
				mon_nam(mon), xname(obj));
			} else {
			    pline("%s tries to wield %s.", Monnam(mon),
				doname(obj));
			    pline("%s %s %s welded to %s hand!",
				  s_suffix(Monnam(mon)), xname(mw_tmp),
				  (mw_tmp->quan == 1L) ? "is" : "are",
				  his[pronoun_gender(mon)]);
			}
			mw_tmp->bknown = 1;
		    }
		    mon->weapon_check = NO_WEAPON_WANTED;
		    return 1;
		}
		mon->mw = obj;		/* wield obj */
		mon->weapon_check = NEED_WEAPON;
		if (canseemon(mon)) {
			pline("%s wields %s!", Monnam(mon), doname(obj));
			if (obj->cursed && obj->otyp != CORPSE) {
				pline("%s %s to %s hand!",
					The(xname(obj)),
					(obj->quan == 1L) ? "welds itself"
					    : "weld themselves",
					s_suffix(mon_nam(mon)));
				obj->bknown = 1;
			}
		}
		return 1;
	}
	mon->weapon_check = NEED_WEAPON;
	return 0;
}

/* rearrange a monster's inventory so that wielded weapon is first */
void
sort_mwep(mon)
struct monst *mon;
{
	struct obj *otmp, *prev, *mw_tmp = MON_WEP(mon);

	if (!mw_tmp) return;
	for (otmp = mon->minvent, prev = 0; otmp; otmp = otmp->nobj) {
		if (otmp == mw_tmp)  break;
		prev = otmp;
	}
	if (!otmp) {
		MON_NOWEP(mon);
	} else if (prev) {
		prev->nobj = otmp->nobj;
		otmp->nobj = mon->minvent;
		mon->minvent = otmp;
	}
}
#endif

int
abon() {	/* attack bonus for strength & dexterity */
	int	sbon;
	register int	str = ACURR(A_STR), dex = ACURR(A_DEX);

#ifdef POLYSELF
	if (u.umonnum >= 0) return(adj_lev(&mons[u.umonnum])-3);
#endif
	if (str < 6) sbon = -2;
	else if (str < 8) sbon = -1;
	else if (str < 17) sbon = 0;
	else if (str < 69) sbon = 1;	/* up to 18/50 */
	else if (str < 118) sbon = 2;
	else sbon = 3;
/*
 *	Temporary kludge - make it a bit easier for a low level character
 *			   to hit until we tune the game a little better.
 */
	sbon += (u.ulevel < 3) ? 1 : 0;

	if (dex < 4) return(sbon-3);
	else if (dex < 6) return(sbon-2);
	else if (dex < 8) return(sbon-1);
	else if (dex < 14) return(sbon);
	else return(sbon + dex-14);
}

#endif /* OVL0 */
#ifdef OVL1

int
dbon() {	/* damage bonus for strength */
	register int	str = ACURR(A_STR);

#ifdef POLYSELF
	if (u.umonnum >= 0) return(0);
#endif

	if (str < 6) return(-1);
	else if (str < 16) return(0);
	else if (str < 18) return(1);
	else if (str == 18) return(2);		/* up to 18 */
	else if (str < 94) return(3);		/* up to 18/75 */
	else if (str < 109) return(4);		/* up to 18/90 */
	else if (str < 118) return(5);		/* up to 18/99 */
	else return(6);
}

#endif /* OVL1 */

/*weapon.c*/
