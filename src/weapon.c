/*	SCCS Id: @(#)weapon.c	3.0	89/11/19
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	This module contains code for calculation of "to hit" and damage
 *	bonuses for any given weapon used, as well as weapons selection
 *	code for monsters.
 */
#include	"hack.h"

#ifdef OVLB

static const char kebabable[] = { S_XORN, S_DRAGON, S_NAGA, S_GIANT, 0 };

/*
 * 	hitval returns an integer representing the "to hit" bonuses
 *	of "otmp" against the monster type "ptr".
 */
int
hitval(otmp, ptr)
struct	obj *otmp;
struct	permonst *ptr;
{
	int	tmp = 0;

	if(otmp->olet == WEAPON_SYM || otmp->otyp == PICK_AXE
						|| otmp->otyp == UNICORN_HORN)
		tmp += otmp->spe;

/*	Put weapon specific "to hit" bonuses in below:		*/
	switch(otmp->otyp) {

#ifdef TOLKIEN
	    case DWARVISH_MATTOCK:
#endif
	    case TWO_HANDED_SWORD:	tmp -= 1; break;
	    case KATANA:		tmp += 1; break;
#ifdef TOLKIEN
	    case ELVEN_DAGGER:
	    case ORCISH_DAGGER:
#endif
	    case DAGGER:
	    case SCALPEL:
	    case ATHAME:
	    case SHURIKEN:		tmp += 2; break;
#ifdef WORM
	    case CRYSKNIFE:		tmp += 3; break;
#endif
	}

/*	Put weapon vs. monster type "to hit" bonuses in below:	*/

	/* Blessed weapons used against undead or demons */
	if(otmp->olet == WEAPON_SYM && otmp->blessed &&
	   (is_demon(ptr) || is_undead(ptr))) tmp += 2;

	if(otmp->otyp >= SPEAR && otmp->otyp <= JAVELIN &&
	   index(kebabable, ptr->mlet)) tmp += 2;

/*	Put specially named weapon "to hit" bonuses in below:	*/
#ifdef NAMED_ITEMS
	tmp += spec_abon(otmp, ptr);
#endif
	return(tmp);
}

/*
 * 	dmgval returns an integer representing the damage bonuses
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
	    if(objects[otmp->otyp].wldam)
		tmp = rnd(objects[otmp->otyp].wldam);
	    switch (otmp->otyp) {
		case CROSSBOW_BOLT:
		case MORNING_STAR:
		case PARTISAN:
#ifdef TOLKIEN
		case ELVEN_BROADSWORD:
#endif
		case BROADSWORD:	tmp++; break;

		case FLAIL:
		case RANSEUR:
		case VOULGE:		tmp += rnd(4); break;

		case ACID_VENOM:
		case HALBERD:
		case SPETUM:		tmp += rnd(6); break;

		case BARDICHE:
		case TRIDENT:		tmp += d(2,4); break;

#ifdef TOLKIEN
		case DWARVISH_MATTOCK:
#endif
		case TWO_HANDED_SWORD:	tmp += d(2,6); break;
	    }
	} else {
	    if(objects[otmp->otyp].wsdam)
		tmp = rnd(objects[otmp->otyp].wsdam);
	    switch (otmp->otyp) {
		case CROSSBOW_BOLT:
		case MACE:
		case WAR_HAMMER:
		case FLAIL:
		case SPETUM:
		case TRIDENT:		tmp++; break;

		case BARDICHE:
		case BILL_GUISARME:
		case GUISARME:
		case LUCERN_HAMMER:
		case MORNING_STAR:
		case RANSEUR:
		case BROADSWORD:
#ifdef TOLKIEN
		case ELVEN_BROADSWORD:
#endif
		case VOULGE:		tmp += rnd(4); break;

		case ACID_VENOM:	tmp += rnd(6); break;
	    }
	}
	if (otmp->otyp == BULLWHIP && thick_skinned(ptr))
		/* thick skinned/scaled creatures don't feel it */
		tmp = 0;
	if (otmp->olet == WEAPON_SYM || otmp->otyp == PICK_AXE
						|| otmp->otyp == UNICORN_HORN)
		tmp += otmp->spe;

/*	Put weapon vs. monster type damage bonuses in below:	*/
	if(otmp->olet == WEAPON_SYM) {
	    if (otmp->blessed && (is_undead(ptr) || is_demon(ptr)))
		tmp += rnd(4);
	}

/*	Put specially named weapon damage bonuses in below:	*/
#ifdef NAMED_ITEMS
	tmp += spec_dbon(otmp, ptr, tmp);
#endif
	return(tmp);
}

void
set_uasmon() {		/* update the "uasmon" structure */

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

#define	Oselect(x)	if((otmp = m_carrying(mtmp, x))) return(otmp);

#ifdef TOLKIEN
static const int rwep[] =
	{ DWARVISH_SPEAR, ELVEN_SPEAR, SPEAR, ORCISH_SPEAR, JAVELIN,
	  SHURIKEN, SILVER_ARROW, ELVEN_ARROW, ARROW, ORCISH_ARROW,
	  CROSSBOW_BOLT, ELVEN_DAGGER, DAGGER, ORCISH_DAGGER, KNIFE, ROCK,
	  LOADSTONE, LUCKSTONE, DART, BOOMERANG, CREAM_PIE
	  /* note: CREAM_PIE should NOT be #ifdef KOPS */
	  };
#else
static const int rwep[] =
	{ SPEAR, JAVELIN, SHURIKEN, SILVER_ARROW, ARROW, CROSSBOW_BOLT,
	  DAGGER, KNIFE, ROCK, LOADSTONE, LUCKSTONE, DART, BOOMERANG, CREAM_PIE
	  /* note: CREAM_PIE should NOT be #ifdef KOPS */
	  };
#endif

struct obj *
select_rwep(mtmp)	/* select a ranged weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	int i;
#ifdef KOPS
	char mlet = mtmp->data->mlet;

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
	    boolean no_propellor = FALSE;
	    int prop;

	    /* shooting gems from slings; this goes just before the darts */
	    if (rwep[i]==DART && !likes_gems(mtmp->data)
			&& m_carrying(mtmp, SLING)) {
		for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
		    if(otmp->olet==GEM_SYM &&
				(otmp->otyp != LOADSTONE || !otmp->cursed))
			return(otmp);
		}
	    }
	    prop = (objects[rwep[i]]).w_propellor;
	    if (prop > 0) {
		switch (prop) {
		case WP_BOW:
#ifdef TOLKIEN
		  no_propellor = !(m_carrying(mtmp, BOW) ||
				   m_carrying(mtmp, ELVEN_BOW) ||
				   m_carrying(mtmp, ORCISH_BOW));
#else
		  no_propellor = !(m_carrying(mtmp, BOW));
#endif
		  break;
		case WP_SLING:
		  no_propellor = !(m_carrying(mtmp, SLING));
		  break;
		case WP_CROSSBOW:
		  no_propellor = !(m_carrying(mtmp, CROSSBOW));
		}
	      }
	    if (!no_propellor) {
		/* Note: cannot use m_carrying for loadstones, since it will
		 * always select the first object of a type, and maybe the
		 * monster is carrying two but only the first is unthrowable.
		 */
		if (rwep[i] != LOADSTONE) {
			Oselect(rwep[i]);
		} else for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
		    if (otmp->otyp == LOADSTONE && !otmp->cursed)
			return otmp;
		}
	    }
	  }

	/* failure */
	return (struct obj *)0;
}

#ifdef TOLKIEN
/* 0 = used by any monster; 1 = only used by strong monsters */
static const int hwep[][2] =
	{ {DWARVISH_MATTOCK,1}, {TWO_HANDED_SWORD,1}, {KATANA,0},
	  {UNICORN_HORN,1},
#ifdef WORM
	  {CRYSKNIFE,0},
#endif
	  {TRIDENT,0}, {LONG_SWORD,0}, {ELVEN_BROADSWORD,0}, {BROADSWORD,0},
	  {LUCERN_HAMMER,1}, {SCIMITAR,1}, {HALBERD,1}, {PARTISAN,1},
	  {LANCE,1}, {FAUCHARD,1}, {BILL_GUISARME,1}, {BEC_DE_CORBIN,1},
	  {GUISARME,1}, {RANSEUR,1}, {SPETUM,1}, {VOULGE,1}, {BARDICHE,0},
	  {MORNING_STAR,0}, {GLAIVE,0}, {ELVEN_SHORT_SWORD,0},
	  {DWARVISH_SHORT_SWORD,0}, {SHORT_SWORD,0}, {ORCISH_SHORT_SWORD,0},
	  {MACE,0}, {AXE,0}, {DWARVISH_SPEAR,0}, {ELVEN_SPEAR,0}, {SPEAR,0},
	  {ORCISH_SPEAR,0}, {FLAIL,0}, {QUARTERSTAFF,1}, {JAVELIN,0},
	  {AKLYS,0}, {CLUB,0}, {PICK_AXE,0},
#ifdef KOPS
	  {RUBBER_HOSE,0},
#endif /* KOPS */
	  {WAR_HAMMER,0}, {ELVEN_DAGGER,0}, {DAGGER,0}, {ORCISH_DAGGER,0},
	  {ATHAME,0}, {SCALPEL,0}, {KNIFE,0},
#ifdef WORM
	  {WORM_TOOTH,0},
#endif
	  {BULLWHIP,0}
	};
#else /* TOLKIEN */
/* 0 = used by any monster; 1 = only used by strong monsters */
static const int hwep[][2] =
	{ {TWO_HANDED_SWORD,1}, {KATANA,0}, {UNICORN_HORN,1},
#ifdef WORM
	  {CRYSKNIFE,0},
#endif
	  {TRIDENT,0}, {LONG_SWORD,0}, {BROADSWORD,0}, {LUCERN_HAMMER,1},
	  {SCIMITAR,1}, {HALBERD,1}, {PARTISAN,1}, {LANCE,1}, {FAUCHARD,1},
	  {BILL_GUISARME,1}, {BEC_DE_CORBIN,1}, {GUISARME,1}, {RANSEUR,1},
	  {SPETUM,1}, {VOULGE,1}, {BARDICHE,0}, {MORNING_STAR,0}, {GLAIVE,0},
	  {SHORT_SWORD,0}, {MACE,0}, {AXE,0}, {SPEAR,0}, {FLAIL,0},
	  {QUARTERSTAFF,1}, {JAVELIN,0}, {AKLYS,0}, {CLUB,0}, {PICK_AXE,0},
#ifdef KOPS
	  {RUBBER_HOSE,0},
#endif /* KOPS */
	  {WAR_HAMMER,0}, {DAGGER,0}, {ATHAME,0}, {SCALPEL,0}, {KNIFE,0},
#ifdef WORM
	  {WORM_TOOTH,0},
#endif
	  {BULLWHIP,0}
	};
#endif /* TOLKIEN */

struct obj *
select_hwep(mtmp)	/* select a hand to hand weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	int i;
	boolean strong = strongmonst(mtmp->data);

	if(is_giant(mtmp->data))	/* giants just love to use clubs */
	    Oselect(CLUB);

	/* only strong monsters can wield big (esp. long) weapons */
	/* all monsters can wield the remaining weapons */
	for (i = 0; i < SIZE(hwep); i++)
	    if (strong || hwep[i][1]==0)
		Oselect(hwep[i][0]);

	/* failure */
	return (struct obj *)0;
}

int
abon() {	/* attack bonus for strength & dexterity */
	int	sbon;

#ifdef POLYSELF
	if (u.umonnum >= 0) return(adj_lev(&mons[u.umonnum])-3);
#endif
	if(ACURR(A_STR) < 6) sbon = -2;
	else if(ACURR(A_STR) < 8) sbon = -1;
	else if(ACURR(A_STR) < 17) sbon = 0;
	else if(ACURR(A_STR) < 69) sbon = 1;	/* up to 18/50 */
	else if(ACURR(A_STR) < 118) sbon = 2;
	else sbon = 3;
/*
 *	Temporary kludge - make it a bit easier for a low level character
 *			   to hit until we tune the game a little better.
 */
	sbon += (u.ulevel < 3) ? 2 : (u.ulevel < 5) ? 1 : 0;

	if(ACURR(A_DEX) < 4) return(sbon-3);
	else if(ACURR(A_DEX) < 6) return(sbon-2);
	else if(ACURR(A_DEX) < 8) return(sbon-1);
	else if(ACURR(A_DEX) < 14) return(sbon);
	else return(sbon+ACURR(A_DEX)-14);
}

#endif /* OVL0 */
#ifdef OVL1

int
dbon() {	/* damage bonus for strength */
#ifdef POLYSELF
	if (u.umonnum >= 0) return(0);
#endif

	if(ACURR(A_STR) < 6) return(-1);
	else if(ACURR(A_STR) < 16) return(0);
	else if(ACURR(A_STR) < 18) return(1);
	else if(ACURR(A_STR) == 18) return(2);		/* up to 18 */
	else if(ACURR(A_STR) < 94) return(3);		/* up to 18/75 */
	else if(ACURR(A_STR) < 109) return(4);		/* up to 18/90 */
	else if(ACURR(A_STR) < 118) return(5);	/* up to 18/99 */
	else return(6);
}

#endif /* OVL1 */
