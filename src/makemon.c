/*	SCCS Id: @(#)makemon.c	3.1	93/05/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "epri.h"
#include "emin.h"
#ifdef REINCARNATION
# include <ctype.h>
#endif

STATIC_VAR NEARDATA struct monst zeromonst;

#define uncommon(ptr) \
	(((ptr)->geno & (G_GENOD | G_EXTINCT | G_NOGEN | G_UNIQ)) || \
	 (!Inhell ? ((ptr)->geno & G_HELL) : ((ptr)->maligntyp > A_NEUTRAL)))

#ifdef OVL0
static boolean NDECL(cmavail);
static int FDECL(align_shift, (struct permonst *));
#endif /* OVL0 */
STATIC_DCL boolean FDECL(is_home_elemental,(struct permonst *));
STATIC_DCL boolean FDECL(wrong_elem_type, (struct permonst *));
STATIC_DCL void FDECL(m_initgrp,(struct monst *,int,int,int));
STATIC_DCL void FDECL(m_initthrow,(struct monst *,int,int));
STATIC_DCL void FDECL(m_initweap,(struct monst *));
#ifdef OVL1
static void FDECL(m_initinv,(struct monst *));
#endif /* OVL1 */

extern int monstr[];

#define m_initsgrp(mtmp, x, y)	m_initgrp(mtmp, x, y, 3)
#define m_initlgrp(mtmp, x, y)	m_initgrp(mtmp, x, y, 10)
#define toostrong(monindx, lev) (monstr[monindx] > lev)
#define tooweak(monindx, lev)	(monstr[monindx] < lev)

#ifdef OVLB
STATIC_OVL boolean
is_home_elemental(ptr)
register struct permonst *ptr;
{
	if (ptr->mlet != S_ELEMENTAL) return FALSE;
	if (!In_endgame(&u.uz)) return FALSE;
	switch(monsndx(ptr)) {
		case PM_AIR_ELEMENTAL: return Is_airlevel(&u.uz);
		case PM_FIRE_ELEMENTAL: return Is_firelevel(&u.uz);
		case PM_EARTH_ELEMENTAL: return Is_earthlevel(&u.uz);
		case PM_WATER_ELEMENTAL: return Is_waterlevel(&u.uz);
	}
	return FALSE;	/* shouldn't be reached */
}

/*
 * Return true if the given monster cannot exist on this elemental level.
 */
STATIC_OVL boolean
wrong_elem_type(ptr)
    register struct permonst *ptr;
{
    if (Is_earthlevel(&u.uz)) {
	/* no restrictions? */
    } else if (Is_waterlevel(&u.uz)) {
	/* just monsters that can swim */
	if(!is_swimmer(ptr)) return TRUE;
    } else if (Is_firelevel(&u.uz)) {
	if(!resists_fire(ptr)) return TRUE;
    } else if (Is_airlevel(&u.uz)) {
	if(!(is_flyer(ptr) && ptr->mlet != S_TRAPPER) && !is_floater(ptr)
	   && !amorphous(ptr) && !noncorporeal(ptr) && !is_whirly(ptr))
	    return TRUE;
    }
    return FALSE;
}

STATIC_OVL void
m_initgrp(mtmp, x, y, n)	/* make a group just like mtmp */
register struct monst *mtmp;
register int x, y, n;
{
	coord mm;
	register int cnt = rnd(n);
	struct monst *mon;

/*
 *	Temporary kludge to cut down on swarming at lower character levels
 *	till we can get this game a little more balanced. [mrs]
 */
	cnt /= (u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1;
	if(!cnt) cnt++;

	mm.x = x;
	mm.y = y;
	while(cnt--) {
		if (peace_minded(mtmp->data)) continue;
		/* Don't create groups of peaceful monsters since they'll get
		 * in our way.  If the monster has a percentage chance so some
		 * are peaceful and some are not, the result will just be a
		 * smaller group.
		 */
		if (enexto(&mm, mm.x, mm.y, mtmp->data)) {
		    mon = makemon(mtmp->data, mm.x, mm.y);
		    mon->mpeaceful = FALSE;
		    set_malign(mon);
		    /* Undo the second peace_minded() check in makemon(); if the
		     * monster turned out to be peaceful the first time we
		     * didn't create it at all; we don't want a second check.
		     */
		}
	}
}

STATIC_OVL
void
m_initthrow(mtmp,otyp,oquan)
struct monst *mtmp;
int otyp,oquan;
{
	register struct obj *otmp;

	otmp = mksobj(otyp, TRUE, FALSE);
	otmp->quan = (long) rn1(oquan, 3);
	otmp->owt = weight(otmp);
	if (otyp == ORCISH_ARROW) otmp->opoisoned = TRUE;
	mpickobj(mtmp, otmp);
}

#endif /* OVLB */
#ifdef OVL2

STATIC_OVL void
m_initweap(mtmp)
register struct monst *mtmp;
{
	register struct permonst *ptr = mtmp->data;
	register int mm = monsndx(ptr);
#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) return;
#endif
/*
 *	first a few special cases:
 *
 *		giants get a boulder to throw sometimes.
 *		ettins get clubs
 *		kobolds get darts to throw
 *		centaurs get some sort of bow & arrows or bolts
 *		soldiers get all sorts of things.
 *		kops get clubs & cream pies.
 */
	switch (mtmp->data->mlet) {
	    case S_GIANT:
		if (rn2(2)) (void)mongets(mtmp, (ptr != &mons[PM_ETTIN]) ?
				    BOULDER : CLUB);
		break;
	    case S_HUMAN:
		if(is_mercenary(ptr)) {
		    int w1 = 0, w2 = 0;
		    switch (mm) {

			case PM_WATCHMAN:
#ifdef ARMY
			case PM_SOLDIER:
#endif
			  if (!rn2(3)) {
			      w1 = rn1(BEC_DE_CORBIN - PARTISAN + 1, PARTISAN);
			      w2 = rn2(2) ? DAGGER : KNIFE;
			  } else w1 = rn2(2) ? SPEAR : SHORT_SWORD;
			  break;
#ifdef ARMY
			case PM_SERGEANT:
			  w1 = rn2(2) ? FLAIL : MACE;
			  break;
			case PM_LIEUTENANT:
			  w1 = rn2(2) ? BROADSWORD : LONG_SWORD;
			  break;
			case PM_CAPTAIN:
#endif
			case PM_WATCH_CAPTAIN:
			  w1 = rn2(2) ? LONG_SWORD : SILVER_SABER;
			  break;
			default:
			  if (!rn2(4)) w1 = DAGGER;
			  if (!rn2(7)) w2 = SPEAR;
			  break;
		    }
		    if (w1) (void)mongets(mtmp, w1);
		    if (!w2 && w1 != DAGGER && !rn2(4)) w2 = KNIFE;
		    if (w2) (void)mongets(mtmp, w2);
		} else if (is_elf(ptr)) {
		    if (rn2(2))
			(void) mongets(mtmp,
				   rn2(2) ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
		    if (rn2(2)) (void)mongets(mtmp, ELVEN_LEATHER_HELM);
		    else if (!rn2(4)) (void)mongets(mtmp, ELVEN_BOOTS);
		    if (rn2(2)) (void)mongets(mtmp, ELVEN_DAGGER);
		    switch (rn2(3)) {
			case 0:
			    if (!rn2(4)) (void)mongets(mtmp, ELVEN_SHIELD);
			    if (rn2(3)) (void)mongets(mtmp, ELVEN_SHORT_SWORD);
			    (void)mongets(mtmp, ELVEN_BOW);
			    m_initthrow(mtmp, ELVEN_ARROW, 12);
			    break;
			case 1:
			    (void)mongets(mtmp, ELVEN_BROADSWORD);
			    if (rn2(2)) (void)mongets(mtmp, ELVEN_SHIELD);
			    break;
			case 2:
			    if (rn2(2)) {
				(void)mongets(mtmp, ELVEN_SPEAR);
				(void)mongets(mtmp, ELVEN_SHIELD);
			    }
			    break;
		    }
		    if(mtmp->data == &mons[PM_ELVENKING])
			(void)mongets(mtmp, PICK_AXE);
		}
		break;

	    case S_ANGEL:
		{
		    int spe2;
		    /* create minion stuff; can't use mongets */
		    struct obj *otmp = mksobj(LONG_SWORD, FALSE, FALSE);

		    /* maybe make it special */
		    if(!rn2(20) || is_lord(mtmp->data))
			otmp = oname(otmp, artiname(
				rn2(2) ? ART_DEMONBANE : ART_SUNSWORD), 0);
		    bless(otmp);
		    otmp->oerodeproof = TRUE;
		    spe2 = rn2(4);
		    otmp->spe = max(otmp->spe, spe2);
		    mpickobj(mtmp, otmp);

		    otmp = mksobj(!rn2(4) || is_lord(mtmp->data) ?
				  SHIELD_OF_REFLECTION : LARGE_SHIELD,
				  FALSE, FALSE);
		    otmp->cursed = FALSE;
		    otmp->oerodeproof = TRUE;
		    otmp->spe = 0;
		    mpickobj(mtmp, otmp);
		}
		break;

	    case S_HUMANOID:
		if (mm == PM_HOBBIT) {
		    switch (rn2(3)) {
			case 0:
			    (void)mongets(mtmp, DAGGER);
			    break;
			case 1:
			    (void)mongets(mtmp, ELVEN_DAGGER);
			    break;
			case 2:
			    (void)mongets(mtmp, SLING);
			    break;
		      }
		    if (!rn2(10)) (void)mongets(mtmp, ELVEN_MITHRIL_COAT);
		    if (!rn2(10)) (void)mongets(mtmp, DWARVISH_CLOAK);
		} else if (is_dwarf(ptr)) {
		    if (rn2(7)) (void)mongets(mtmp, DWARVISH_CLOAK);
		    if (rn2(7)) (void)mongets(mtmp, IRON_SHOES);
		    if (!rn2(4)) {
			(void)mongets(mtmp, DWARVISH_SHORT_SWORD);
			/* note: you can't use a mattock with a shield */
			if (rn2(2)) (void)mongets(mtmp, DWARVISH_MATTOCK);
			else {
				(void)mongets(mtmp, AXE);
				(void)mongets(mtmp, DWARVISH_ROUNDSHIELD);
			}
			(void)mongets(mtmp, DWARVISH_IRON_HELM);
			if (!rn2(3))
			    (void)mongets(mtmp, DWARVISH_MITHRIL_COAT);
		    } else {
			(void)mongets(mtmp, !rn2(3) ? PICK_AXE : DAGGER);
		    }
		}
		break;
# ifdef KOPS
	    case S_KOP:		/* create Keystone Kops with cream pies to
				 * throw. As suggested by KAA.	   [MRS]
				 */
		if (!rn2(4)) m_initthrow(mtmp, CREAM_PIE, 2);
		if (!rn2(3)) (void)mongets(mtmp,(rn2(2)) ? CLUB : RUBBER_HOSE);
		break;
# endif
	    case S_ORC:
		if(rn2(2)) (void)mongets(mtmp, ORCISH_HELM);
		switch (mm != PM_ORC_CAPTAIN ? mm :
			rn2(2) ? PM_MORDOR_ORC : PM_URUK_HAI) {
		    case PM_MORDOR_ORC:
			if(!rn2(3)) (void)mongets(mtmp, SCIMITAR);
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_SHIELD);
			if(!rn2(3)) (void)mongets(mtmp, KNIFE);
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_CHAIN_MAIL);
			break;
		    case PM_URUK_HAI:
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_CLOAK);
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_SHORT_SWORD);
			if(!rn2(3)) (void)mongets(mtmp, IRON_SHOES);
			if(!rn2(3)) {
			    (void)mongets(mtmp, ORCISH_BOW);
			    m_initthrow(mtmp, ORCISH_ARROW, 12);
			}
			if(!rn2(3)) (void)mongets(mtmp, URUK_HAI_SHIELD);
			break;
		    default:
			if (mm != PM_ORC_SHAMAN && rn2(2))
			  (void)mongets(mtmp, (mm == PM_GOBLIN || rn2(2) == 0)
						   ? ORCISH_DAGGER : SCIMITAR);
		}
		break;
	    case S_OGRE:
		if (!rn2(mm == PM_OGRE_KING ? 3 : mm == PM_OGRE_LORD ? 6 : 12))
		    (void) mongets(mtmp, BATTLE_AXE);
		break;
	    case S_KOBOLD:
		if (!rn2(4)) m_initthrow(mtmp, DART, 12);
		break;

	    case S_CENTAUR:
		if (rn2(2)) {
		    if(ptr == &mons[PM_FOREST_CENTAUR]) {
			(void)mongets(mtmp, BOW);
			m_initthrow(mtmp, ARROW, 12);
		    } else {
			(void)mongets(mtmp, CROSSBOW);
			m_initthrow(mtmp, CROSSBOW_BOLT, 12);
		    }
		}
		break;
	    case S_WRAITH:
		(void)mongets(mtmp, KNIFE);
		(void)mongets(mtmp, LONG_SWORD);
		break;
	    case S_ZOMBIE:
		if (!rn2(4)) (void)mongets(mtmp, LEATHER_ARMOR);
		if (!rn2(4))
			(void)mongets(mtmp, (rn2(3) ? KNIFE : SHORT_SWORD));
		break;
	    case S_DEMON:
		switch (mm) {
		    case PM_BALROG:
			(void)mongets(mtmp, BULLWHIP);
			(void)mongets(mtmp, BROADSWORD);
			break;
		    case PM_ORCUS:
			(void)mongets(mtmp, WAN_DEATH); /* the Wand of Orcus */
			break;
		    case PM_HORNED_DEVIL:
			(void)mongets(mtmp, rn2(4) ? TRIDENT : BULLWHIP);
			break;
		    case PM_ICE_DEVIL:
			if (!rn2(4)) (void)mongets(mtmp, SPEAR);
			break;
		    case PM_ASMODEUS:
			(void)mongets(mtmp, WAN_COLD);
			break;
		    case PM_DISPATER:
			(void)mongets(mtmp, WAN_STRIKING);
			break;
		    case PM_YEENOGHU:
			(void)mongets(mtmp, FLAIL);
			break;
		}
		/* prevent djinnis and mail daemons from leaving objects when
		 * they vanish
		 */
		if (!is_demon(ptr)) break;
		/* fall thru */
/*
 *	Now the general case, Some chance of getting some type
 *	of weapon for "normal" monsters.  Certain special types
 *	of monsters will get a bonus chance or different selections.
 */
	    default:
	      {
		int bias;
		
		bias = is_lord(ptr) + is_prince(ptr) * 2 + extra_nasty(ptr);
		switch(rnd(14 - (2 * bias))) {
		    case 1:
			if(strongmonst(ptr)) (void) mongets(mtmp, BATTLE_AXE);
			else m_initthrow(mtmp, DART, 12);
			break;
		    case 2:
			if(strongmonst(ptr))
			    (void) mongets(mtmp, TWO_HANDED_SWORD);
			else {
			    (void) mongets(mtmp, CROSSBOW);
			    m_initthrow(mtmp, CROSSBOW_BOLT, 12);
			}
			break;
		    case 3:
			(void) mongets(mtmp, BOW);
			m_initthrow(mtmp, ARROW, 12);
			break;
		    case 4:
			if(strongmonst(ptr)) (void) mongets(mtmp, LONG_SWORD);
			else m_initthrow(mtmp, DAGGER, 3);
			break;
		    case 5:
			if(strongmonst(ptr))
			    (void) mongets(mtmp, LUCERN_HAMMER);
			else (void) mongets(mtmp, AKLYS);
			break;
		    default:
			break;
		}
	      }
	      break;
	}
#ifdef MUSE
	if ((int) mtmp->m_lev > rn2(75))
		(void) mongets(mtmp, rnd_offensive_item(mtmp));
#endif
}

#endif /* OVL2 */
#ifdef OVL1

static void
m_initinv(mtmp)
register struct	monst	*mtmp;
{
	register int cnt;
	register struct obj *otmp;
	register struct permonst *ptr = mtmp->data;
#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) return;
#endif
/*
 *	Soldiers get armour & rations - armour approximates their ac.
 *	Nymphs may get mirror or potion of object detection.
 */
	switch(ptr->mlet) {

	    case S_HUMAN:
		if(is_mercenary(ptr)) {
		    register int mac;
#ifdef MUSE
		    switch(monsndx(ptr)) {
			case PM_GUARD: mac = -1; break;
# ifdef ARMY
			case PM_SOLDIER: mac = 3; break;
			case PM_SERGEANT: mac = 0; break;
			case PM_LIEUTENANT: mac = -2; break;
			case PM_CAPTAIN: mac = -3; break;
# endif
			case PM_WATCHMAN: mac = 3; break;
			case PM_WATCH_CAPTAIN: mac = -2; break;
			default: impossible("odd mercenary %d?", monsndx(ptr));
				mac = 0;
				break;
		    }
#else
		    mac = ptr->ac;
#endif

		    if (mac < -1 && rn2(5))
			mac += 7 + mongets(mtmp, (rn2(5)) ?
					   PLATE_MAIL : CRYSTAL_PLATE_MAIL);
		    else if (mac < 3 && rn2(5))
			mac += 6 + mongets(mtmp, (rn2(3)) ?
					   SPLINT_MAIL : BANDED_MAIL);
		    else if (rn2(5))
			mac += 3 + mongets(mtmp, (rn2(3)) ?
					   RING_MAIL : STUDDED_LEATHER_ARMOR);
		    else
			mac += 2 + mongets(mtmp, LEATHER_ARMOR);

		    if (mac < 10 && rn2(3))
			mac += 1 + mongets(mtmp, HELMET);
		    else if (mac < 10 && rn2(2))
			mac += 1 + mongets(mtmp, DENTED_POT);
		    if (mac < 10 && rn2(3))
			mac += 1 + mongets(mtmp, SMALL_SHIELD);
		    else if (mac < 10 && rn2(2))
			mac += 2 + mongets(mtmp, LARGE_SHIELD);
		    if (mac < 10 && rn2(3))
			mac += 1 + mongets(mtmp, LOW_BOOTS);
		    else if (mac < 10 && rn2(2))
			mac += 2 + mongets(mtmp, HIGH_BOOTS);
		    if (mac < 10 && rn2(3))
			mac += 1 + mongets(mtmp, LEATHER_GLOVES);
		    else if (mac < 10 && rn2(2))
			mac += 1 + mongets(mtmp, ELVEN_CLOAK);

#ifndef MUSE
		    if (mac != 10 && rn2(5)) {	/* make up the difference */
			otmp = mksobj(RIN_PROTECTION, FALSE, FALSE);
			otmp->spe = (10 - mac + rn2(3) - rn2(3));
			if(otmp->spe < 0) curse(otmp);
			mpickobj(mtmp, otmp);
		    }
#endif
#ifdef ARMY
		    if(ptr != &mons[PM_GUARD] &&
			ptr != &mons[PM_WATCHMAN] &&
			ptr != &mons[PM_WATCH_CAPTAIN]) {
			if (!rn2(3)) (void) mongets(mtmp, K_RATION);
			if (!rn2(2)) (void) mongets(mtmp, C_RATION);
# ifdef MUSE
			if (ptr != &mons[PM_SOLDIER] && !rn2(3))
				(void) mongets(mtmp, BUGLE);
# endif
		    } else
#endif
			   if (ptr == &mons[PM_WATCHMAN] && rn2(3))
				(void) mongets(mtmp, TIN_WHISTLE);
		} else if (ptr == &mons[PM_SHOPKEEPER]) {
		    (void) mongets(mtmp,SKELETON_KEY);
		}
		break;

	    case S_NYMPH:
		if(!rn2(2)) (void) mongets(mtmp, MIRROR);
		if(!rn2(2)) (void) mongets(mtmp, POT_OBJECT_DETECTION);
		break;

	    case S_GIANT:
		if (ptr == &mons[PM_MINOTAUR])
		    (void) mongets(mtmp, WAN_DIGGING);
		else if (is_giant(ptr)) {
		    for(cnt = rn2((int)(mtmp->m_lev / 2)); cnt; cnt--) {
			    otmp = mksobj(rnd_class(DILITHIUM_CRYSTAL,LUCKSTONE-1),FALSE,FALSE);
			    otmp->quan = (long) rn1(2, 3);
			    otmp->owt = weight(otmp);
			    mpickobj(mtmp, otmp);
		    }
		}
		break;
	    case S_WRAITH:
		if (ptr == &mons[PM_NAZGUL]) {
			otmp = mksobj(RIN_INVISIBILITY, FALSE, FALSE);
			curse(otmp);
			mpickobj(mtmp, otmp);
		}
		break;
	    case S_QUANTMECH:
		if (!rn2(20)) {
			struct obj *cat;

			otmp = mksobj(LARGE_BOX, FALSE, FALSE);
	/* actually, whether this is a corpse or a live cat shouldn't
	   really be decided until the box is opened... */
			cat = mksobj(CORPSE, FALSE, FALSE);
			cat->corpsenm = PM_HOUSECAT;
			cat->owt = weight(cat);
			cat = oname(cat, "Schroedinger's Cat", FALSE);
			cat->nobj = otmp->cobj;
			otmp->cobj = cat;
			otmp->owt = weight(otmp);
			mpickobj(mtmp, otmp);
		}
		break;
	    case S_LEPRECHAUN:
		mtmp->mgold = (long) d(level_difficulty(), 30);
		break;
	    default:
		break;
	}

#ifdef ARMY	/* ordinary soldiers rarely have access to magic (or gold :-) */
	if (ptr == &mons[PM_SOLDIER] && rn2(13)) return;
#endif
#ifdef MUSE
	if ((int) mtmp->m_lev > rn2(50))
		(void) mongets(mtmp, rnd_defensive_item(mtmp));
	if ((int) mtmp->m_lev > rn2(100))
		(void) mongets(mtmp, rnd_misc_item(mtmp));
#endif
	if (likes_gold(ptr) && !mtmp->mgold && !rn2(5))
		mtmp->mgold =
		      (long) d(level_difficulty(), mtmp->minvent ? 5 : 10);
}

/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: near player (if !in_mklev)
 *
 *	In case we make a monster group, only return the one at [x,y].
 */
struct monst *
makemon(ptr, x, y)
register struct permonst *ptr;
register int	x, y;
{
	register struct monst *mtmp;
	register int	ct;
	boolean anything = (!ptr);
	boolean byyou = (x == u.ux && y == u.uy);

	/* if caller wants random location, do it here */
	if(x == 0 && y == 0) {
		int tryct = 0;	/* careful with bigrooms */
		do {
			x = rn1(COLNO-3,2);
			y = rn2(ROWNO);
		} while(!goodpos(x, y, (struct monst *)0, ptr) ||
			(!in_mklev && tryct++ < 50 && cansee(x, y)));
	} else if (byyou && !in_mklev) {
		coord bypos;

		if(enexto(&bypos, u.ux, u.uy, ptr)) {
			x = bypos.x;
			y = bypos.y;
		} else
			return((struct monst *)0);
	}

	/* if a monster already exists at the position, return */
	if(MON_AT(x, y))
		return((struct monst *) 0);

	if(ptr){
		/* if you are to make a specific monster and it has
		   already been genocided, return */
		if(ptr->geno & G_GENOD) return((struct monst *) 0);
	} else {
		/* make a random (common) monster that can survive here.
		 * (the special levels ask for random monsters at specific
		 * positions, causing mass drowning on the medusa level,
		 * for instance.)
		 */
		int tryct = 0;	/* maybe there are no good choices */
		do {
			if(!(ptr = rndmonst())) {
#ifdef DEBUG
			    pline("Warning: no monster.");
#endif
			    return((struct monst *) 0);	/* no more monsters! */
			}
		} while(!goodpos(x, y, (struct monst *)0, ptr) && tryct++ < 50);
	}
	/* if it's unique, don't ever make it again */
	if (ptr->geno & G_UNIQ) ptr->geno |= G_EXTINCT;

	mtmp = newmonst(ptr->pxlth);
	*mtmp = zeromonst;		/* clear all entries in structure */
	for(ct = 0; ct < ptr->pxlth; ct++)
		((char *) &(mtmp->mextra[0]))[ct] = 0;
	mtmp->nmon = fmon;
	fmon = mtmp;
	mtmp->m_id = flags.ident++;
	mtmp->data = ptr;
	mtmp->mxlth = ptr->pxlth;

	mtmp->m_lev = adj_lev(ptr);
	if (is_golem(ptr))
	    mtmp->mhpmax = mtmp->mhp = golemhp(monsndx(ptr));
	else if (is_rider(ptr)) {
		/* We want low HP, but a high mlevel so they can attack well */
		mtmp->mhpmax = mtmp->mhp = d(10,8);
	} else if(ptr->mlevel > 49) {
	    /* "special" fixed hp monster
	     * the hit points are encoded in the mlevel in a somewhat strange
	     * way to fit in the 50..127 positive range of a signed character
	     * above the 1..49 that indicate "normal" monster levels */
	    mtmp->mhpmax = mtmp->mhp = 2*(ptr->mlevel - 6);
	    mtmp->m_lev = mtmp->mhp / 4;	/* approximation */
	} else if((ptr->mlet == S_DRAGON) && (ptr >= &mons[PM_GRAY_DRAGON]))
	    mtmp->mhpmax = mtmp->mhp = mtmp->m_lev*8;
	else if(!mtmp->m_lev) mtmp->mhpmax = mtmp->mhp = rnd(4);
	else if(is_home_elemental(ptr))
	    mtmp->mhpmax = mtmp->mhp = 3 * d((int)mtmp->m_lev, 8);
	else mtmp->mhpmax = mtmp->mhp = d((int)mtmp->m_lev, 8);

	if (is_female(ptr)) mtmp->female = TRUE;
	else if (is_male(ptr)) mtmp->female = FALSE;
	else mtmp->female = rn2(2);	/* ignored for neuters */

	place_monster(mtmp, x, y);
	mtmp->mcansee = mtmp->mcanmove = TRUE;
	mtmp->mpeaceful = peace_minded(ptr);

	switch(ptr->mlet) {
		case S_MIMIC:
			set_mimic_sym(mtmp);
			break;
		case S_SPIDER:
		case S_SNAKE:
			if(in_mklev)
			    if(x && y)
				(void) mkobj_at(0, x, y, TRUE);
			if(hides_under(ptr) && OBJ_AT(x, y))
			    mtmp->mundetected = TRUE;
			break;
		case S_STALKER:
		case S_EEL:
			mtmp->minvis = TRUE;
			break;
		case S_LEPRECHAUN:
			mtmp->msleep = TRUE;
			break;
		case S_JABBERWOCK:
		case S_NYMPH:
			if(rn2(5) && !u.uhave.amulet) mtmp->msleep = TRUE;
			break;
		case S_ORC:
			if(pl_character[0] == 'E') mtmp->mpeaceful = FALSE;
			break;
		case S_UNICORN:
			if (sgn(u.ualign.type) == sgn(ptr->maligntyp))
				mtmp->mpeaceful = TRUE;
			break;
	}
	if (ptr == &mons[PM_CHAMELEON]) {
		/* If you're protected with a ring, don't create
		 * any shape-changing chameleons -dgk
		 */
		if (Protection_from_shape_changers)
			mtmp->cham = FALSE;
		else {
			mtmp->cham = TRUE;
			(void) newcham(mtmp, rndmonst());
		}
	} else if (ptr == &mons[PM_WIZARD_OF_YENDOR]) {
		mtmp->iswiz = TRUE;
		flags.no_of_wizards++;
	} else if (ptr == &mons[PM_VLAD_THE_IMPALER])
		(void) mongets(mtmp, CANDELABRUM_OF_INVOCATION);
#ifdef MULDGN
	else if (ptr->msound == MS_NEMESIS)
		(void) mongets(mtmp, BELL_OF_OPENING);
#else
	else if (ptr == &mons[PM_MEDUSA])
		(void) mongets(mtmp, BELL_OF_OPENING);
#endif

	if(in_mklev) {
		if(((is_ndemon(ptr)) ||
		    (ptr == &mons[PM_WUMPUS]) ||
		    (ptr == &mons[PM_LONG_WORM]) ||
		    (ptr == &mons[PM_GIANT_EEL])) && !u.uhave.amulet && rn2(5))
			mtmp->msleep = TRUE;
	} else {
		if(byyou) {
			newsym(mtmp->mx,mtmp->my);
			set_apparxy(mtmp);
		}
	}
	if(is_dprince(ptr)) {
	    mtmp->mpeaceful = mtmp->minvis = TRUE;
	    if (uwep && uwep->oartifact == ART_EXCALIBUR)
		mtmp->mpeaceful = mtmp->mtame = FALSE;
	}
	if ( (ptr == &mons[PM_LONG_WORM]) && (mtmp->wormno = get_wormno()) ) {
	    /* we can now create worms with tails - 11/91 */
	    initworm(mtmp, rn2(5));
	    if (count_wsegs(mtmp)) place_worm_tail_randomly(mtmp, x, y);
	}
	set_malign(mtmp);		/* having finished peaceful changes */
	if(anything) {
	    if((ptr->geno & G_SGROUP) && rn2(2))
		m_initsgrp(mtmp, mtmp->mx, mtmp->my);
	    else if(ptr->geno & G_LGROUP) {
			if(rn2(3))  m_initlgrp(mtmp, mtmp->mx, mtmp->my);
			else	    m_initsgrp(mtmp, mtmp->mx, mtmp->my);
	    }
	}

	if(is_armed(ptr))
		m_initweap(mtmp);	/* equip with weapons / armor */
	m_initinv(mtmp);    /* add on a few special items incl. more armor */
#ifdef MUSE
	m_dowear(mtmp, TRUE);
#endif

	if (!in_mklev)
	    newsym(mtmp->mx,mtmp->my);	/* make sure the mon shows up */

	return(mtmp);
}

boolean
enexto(cc, xx, yy, mdat)
coord *cc;
register xchar xx, yy;
struct permonst *mdat;
{
	register xchar x,y;
	coord foo[15], *tfoo;
	int range, i;
	int xmin, xmax, ymin, ymax;

	tfoo = foo;
	range = 1;
	do {	/* full kludge action. */
		xmin = max(1, xx-range);
		xmax = min(COLNO-1, xx+range);
		ymin = max(0, yy-range);
		ymax = min(ROWNO-1, yy+range);

		for(x = xmin; x <= xmax; x++)
			if(goodpos(x, ymin, (struct monst *)0, mdat)) {
				tfoo->x = x;
#ifdef MAC_MPW32
				( tfoo ) -> y = ymin ;
				tfoo ++ ;
#else
				(tfoo++)->y = ymin;
#endif
				if(tfoo == &foo[15]) goto foofull;
			}
		for(x = xmin; x <= xmax; x++)
			if(goodpos(x, ymax, (struct monst *)0, mdat)) {
				tfoo->x = x;
#ifdef MAC_MPW32
				( tfoo ) -> y = ymax ;
				tfoo ++ ;
#else
				(tfoo++)->y = ymax;
#endif
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = ymin+1; y < ymax; y++)
			if(goodpos(xmin, y, (struct monst *)0, mdat)) {
				tfoo->x = xmin;
#ifdef MAC_MPW32
				( tfoo ) -> y = y ;
				tfoo ++ ;
#else
				(tfoo++)->y = y;
#endif
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = ymin+1; y < ymax; y++)
			if(goodpos(xmax, y, (struct monst *)0, mdat)) {
				tfoo->x = xmax;
#ifdef MAC_MPW32
				( tfoo ) -> y = y ;
				tfoo ++ ;
#else
				(tfoo++)->y = y;
#endif
				if(tfoo == &foo[15]) goto foofull;
			}
		range++;
		if(range > ROWNO && range > COLNO) return FALSE;
	} while(tfoo == foo);
foofull:
	i = rn2((int)(tfoo - foo));
	cc->x = foo[i].x;
	cc->y = foo[i].y;
	return TRUE;
}

int
goodpos(x, y, mtmp, mdat)
int x,y;
struct monst *mtmp;	/* existing monster being moved, if any */
struct permonst *mdat;
{
	struct monst *mtmp2;

	if (x < 1 || x > COLNO-2 || y < 1 || y > ROWNO-2 || MON_AT(x, y))
		return 0;

	/* in many cases, we're trying to create a new monster, which
	 * can't go on top of the player or any existing monster.
	 * however, occasionally we are relocating engravings or objects,
	 * which could be colocated and thus get restricted a bit too much.
	 * oh well.
	 */
	if (x == u.ux && y == u.uy) return 0;
	if ((mtmp2 = m_at(x, y)) && mtmp != mtmp2) return 0;

	if (mdat) {
	    if (IS_POOL(levl[x][y].typ))
		if (mdat == &playermon &&
		    (HLevitation || Wwalking || Amphibious))
			return 1;
		else	return (is_flyer(mdat) || is_swimmer(mdat));
	    if (levl[x][y].typ == LAVAPOOL)
		if (mdat == &playermon && (HLevitation))
			return 1;
		else return
			(is_flyer(mdat) || (mdat == &mons[PM_FIRE_ELEMENTAL]));
	    if (passes_walls(mdat) && may_passwall(x,y)) return 1;
	}
	if (!ACCESSIBLE(levl[x][y].typ)) return 0;
	if (closed_door(x, y) && (!mdat || !amorphous(mdat)))
		return 0;
	if (sobj_at(BOULDER, x, y) && (!mdat || !throws_rocks(mdat)))
		return 0;
	return 1;
}

#endif /* OVL1 */
#ifdef OVLB

/*
 * rloc_to()
 *
 * Pulls a monster from its current position and places a monster at
 * a new x and y.  If oldx is 0, then the monster was not in the levels.monsters
 * array.  However, if oldx is 0, oldy may still have a value because mtmp is a
 * migrating_mon.  Worm tails are always placed randomly around the head of
 * the worm.
 */

void
rloc_to(mtmp, x, y)
	struct monst *mtmp;
	register int x, y;
{
	register int oldx = mtmp->mx, oldy = mtmp->my;

	if(x == mtmp->mx && y == mtmp->my) /* that was easy */
		return;

	if (oldx) {				/* "pick up" monster */
	    if(mtmp->wormno)
		remove_worm(mtmp);
	    else {
		remove_monster(oldx, oldy);
		newsym(oldx, oldy);		/* update old location */
	    }
	}

	place_monster(mtmp, x, y);		/* put monster down */

	if(mtmp->wormno)			/* now put down tail */
		place_worm_tail_randomly(mtmp, x, y);

	if(u.ustuck == mtmp){
		if(u.uswallow) {
			u.ux = x;
			u.uy = y;
			docrt();
		} else	u.ustuck = 0;
	}

	newsym(x, y);				/* update new location */
	set_apparxy(mtmp);			/* orient monster */
}

#endif /* OVLB */
#ifdef OVL2

void
rloc(mtmp)
	struct monst *mtmp;
{
	register int x = xupstair, y = yupstair, trycount;

	/* if the wiz teleports away to heal, try the up staircase,
	   to block the player's escaping before he's healed */
	if (!mtmp->iswiz || !goodpos(x, y, mtmp, mtmp->data)) {
	    trycount = 0;
	    do {
		x = rn1(COLNO-3,2);
		y = rn2(ROWNO);
	    } while(!goodpos(x,y,mtmp,mtmp->data) && ++trycount < 1000);
	    /* last ditch attempt to find a good place */
	    if (trycount >= 1000) {
		for (x = 2; x < COLNO - 1; x++)
		    for (y = 0; y < ROWNO; y++)
			if (goodpos(x,y,mtmp,mtmp->data))
			    goto found_atlast;
		/* level either full of monsters or somehow faulty */
		impossible("rloc(): couldn't relocate monster");
		return;
	    }
	}
found_atlast:;
	rloc_to(mtmp, x, y);
}

void
rloc_shk(mtmp)   /* to be used when teleporting a shopkeeper */
struct monst *mtmp;
{
	register int x, y, ox, oy, trycount;

	if(!mtmp->isshk) return;
	trycount = 0;
	do {
		x = rn1(COLNO-3,2);
		y = rn2(ROWNO);
	} while(!goodpos(x,y,mtmp,mtmp->data) && ++trycount < 1000);
	/* last ditch attempt to find a good place */
	if (trycount >= 1000) {
		for (x = 2; x < COLNO - 1; x++)
		    for (y = 0; y < ROWNO; y++)
			if (goodpos(x,y,mtmp,mtmp->data))
			    goto found_ok;
		/* this really shouldn't happen - after all, shopkeeper's
		   original position should always be available */
		impossible("rloc_shk(): couldn't relocate shopkeeper");
		return;
	}
found_ok:;
	ox = mtmp->mx;
	oy = mtmp->my;
	rloc_to(mtmp, x, y);
	make_angry_shk(mtmp, ox, oy);
}

#endif /* OVL2 */
#ifdef OVLB

void
vloc(mtmp)
struct monst *mtmp;
{
	register struct mkroom *croom = search_special(VAULT);
	coord c;

	if(croom && somexy(croom, &c) && goodpos(c.x, c.y, mtmp, mtmp->data)) {
		rloc_to(mtmp, c.x, c.y);
		return;
	}
	rloc(mtmp);
}

#endif /* OVLB */
#ifdef OVL0

static boolean
cmavail()	/* return TRUE if "common" monsters can be generated */
{
	struct permonst *ptr;

	for(ptr = &mons[0]; ptr != &mons[NUMMONS]; ptr++)
	   if(!uncommon(ptr))  return TRUE;

	return FALSE;
}

/*
 *	shift the probability of a monster's generation by
 *	comparing the dungeon alignment and monster alignment.
 *	return an integer in the range of 0-5.
 */
static int
align_shift(ptr)
register struct permonst *ptr;
{
    static NEARDATA long oldmoves = 0L;	/* != 1, starting value of moves */
    static NEARDATA s_level *lev;
    register int alshift;

    if(oldmoves != moves) {
	lev = Is_special(&u.uz);
	oldmoves = moves;
    }
    switch((lev) ? lev->flags.align : dungeons[u.uz.dnum].flags.align) {
    default:	/* just in case */
    case AM_NONE:	alshift = 0;
			break;
    case AM_LAWFUL:	alshift = (ptr->maligntyp+20)/(2*ALIGNWEIGHT);
			break;
    case AM_NEUTRAL:	alshift = (20 - abs(ptr->maligntyp))/ALIGNWEIGHT;
			break;
    case AM_CHAOTIC:	alshift = (-(ptr->maligntyp-20))/(2*ALIGNWEIGHT);
			break;
    }
    return alshift;
}

struct	permonst *
rndmonst()		/* select a random monster */
{
	register struct permonst *ptr;
	register int i, ct;
	register int zlevel;
	static NEARDATA int minmlev, maxmlev, accept;
	static NEARDATA long oldmoves = 0L;	/* != 1, starting value of moves */
#ifdef REINCARNATION
	static NEARDATA boolean upper;
#endif
	static NEARDATA boolean elemlevel;

#ifdef MULDGN
	if(u.uz.dnum == quest_dnum && (ptr = qt_montype())) return(ptr);
#endif
	if(oldmoves != moves) {		/* must recalculate accept */
	    oldmoves = moves;
	    zlevel = level_difficulty();
	    if(!cmavail()) {
#ifdef DEBUG
		pline("cmavail() fails!");
#endif
		return((struct permonst *) 0);
	    }

	    /* determine the level of the weakest monster to make. */
	    minmlev = zlevel/6;
	    /* determine the level of the strongest monster to make. */
	    maxmlev = (zlevel + u.ulevel)>>1;
#ifdef REINCARNATION
	    upper = Is_rogue_level(&u.uz);
#endif
	    elemlevel = In_endgame(&u.uz) && !Is_astralevel(&u.uz);
/*
 *	Find out how many monsters exist in the range we have selected.
 */
	    accept = 0;
	    for(ct = 0, ptr = &mons[0] ; ptr != &mons[NUMMONS]; ct++, ptr++) {
		if(tooweak(ct, minmlev) || toostrong(ct, maxmlev))
		    continue;
#ifdef REINCARNATION
		if(upper && !isupper(def_monsyms[(int)ptr->mlet])) continue;
#endif
		if(elemlevel && wrong_elem_type(ptr)) continue;
		if(uncommon(ptr)) continue;
		accept += (ptr->geno & G_FREQ);
		accept += align_shift(ptr);
	    }
	}

	if(!accept) {
#ifdef DEBUG
		pline("no accept!");
#endif
		return((struct permonst *) 0);
	}
/*
 *	Now, select a monster at random.
 */
	ct = rnd(accept);
	for(i = 0,ptr = &mons[0]; ptr != &mons[NUMMONS] && ct > 0; i++,ptr++) {
		if(tooweak(i, minmlev) || toostrong(i, maxmlev))
		    continue;
#ifdef REINCARNATION
		if(upper & !isupper(def_monsyms[(int)ptr->mlet])) continue;
#endif
		if(elemlevel && wrong_elem_type(ptr)) continue;
		if(uncommon(ptr)) continue;
		ct -= (ptr->geno & G_FREQ);
		ct -= align_shift(ptr);
	}
	if(ct > 0) {
#ifdef DEBUG
		pline("no count!");
#endif
		return((struct permonst *) 0);
	}
	return(--ptr);	/* subtract extra increment */
}

#endif /* OVL0 */
#ifdef OVL1

/*	The routine below is used to make one of the multiple types
 *	of a given monster class.  The second parameter specifies a
 *	special casing bit mask to allow any of the normal genesis
 *	masks to be deactivated.  Returns 0 if no monsters
 *	in that class can be made.
 */

struct permonst *
mkclass(class,spc)
char	class;
int	spc;
{
	register int	first, last, num = 0;
	int maxmlev, mask = (G_GENOD | G_EXTINCT | G_NOGEN | G_UNIQ) & ~spc;

	maxmlev = level_difficulty() >> 1;
	if(class < 1 || class >= MAXMCLASSES) {
	    impossible("mkclass called with bad class!");
	    return((struct permonst *) 0);
	}
/*	Assumption #1:	monsters of a given class are contiguous in the
 *			mons[] array.
 */
	for(first = 0; first < NUMMONS; first++)
	    if (mons[first].mlet == class) break;
	if (first == NUMMONS) return((struct permonst *) 0);

	for(last = first; last < NUMMONS && mons[last].mlet == class; last++)
	    if(!(mons[last].geno & mask)) {
		/* consider it */
		if(num && toostrong(last, maxmlev) &&
		   monstr[last] != monstr[last-1] && rn2(2)) break;
		num += mons[last].geno & G_FREQ;
	    }

	if(!num) return((struct permonst *) 0);

/*	Assumption #2:	monsters of a given class are presented in ascending
 *			order of strength.
 */
	for(num = rnd(num); num > 0; first++)
	    if(!(mons[first].geno & mask)) {
		/* skew towards lower value monsters at lower exp. levels */
		num -= mons[first].geno & G_FREQ;
		if (num && adj_lev(&mons[first]) > (u.ulevel*2)) {
		    /* but not when multiple monsters are same level */
		    if (mons[first].mlevel != mons[first+1].mlevel)
			num--;
		}
	    }
	first--; /* correct an off-by-one error */

	return(&mons[first]);
}

int
adj_lev(ptr)	/* adjust strength of monsters based on u.uz and u.ulevel */
register struct permonst *ptr;
{
	int	tmp, tmp2;

	if((tmp = ptr->mlevel) > 49) return(50); /* "special" demons/devils */
	tmp2 = (level_difficulty() - tmp);
	if(tmp2 < 0) tmp--;		/* if mlevel > u.uz decrement tmp */
	else tmp += (tmp2 / 5);		/* else increment 1 per five diff */

	tmp2 = (u.ulevel - ptr->mlevel);	/* adjust vs. the player */
	if(tmp2 > 0) tmp += (tmp2 / 4);		/* level as well */

	tmp2 = (3 * ((int) ptr->mlevel))/ 2;	/* crude upper limit */
	return((tmp > tmp2) ? tmp2 : (tmp > 0 ? tmp : 0)); /* 0 lower limit */
}

#endif /* OVL1 */
#ifdef OVLB

struct permonst *
grow_up(mtmp,victim)		/* mon mtmp "grows up" to a bigger version. */
register struct monst *mtmp;
register struct monst *victim;
{
	register int newtype;
	register struct permonst *ptr = mtmp->data;

	if (victim) {
	    if (ptr->mlevel >= 50 || is_golem(ptr) || is_home_elemental(ptr)
		    || is_mplayer(ptr))
		/* doesn't grow up, has strange hp calculation so might be
		 * weakened by tests below */
		return ptr;

	    mtmp->mhpmax = mtmp->mhpmax + (1 + rn2((int)victim->m_lev+1));
	    if (mtmp->mhpmax <= (8 * (int)mtmp->m_lev)
			|| (mtmp->m_lev == 0 && mtmp->mhpmax <= 4))
		/* not ready to grow up */
		return ptr;
	}
#ifdef MUSE
	/* else it's a gain level potion; always go up a level */
	else {
	    int foo=rnd(8);

	    mtmp->mhp += foo;
	    mtmp->mhpmax += foo;
	}
#endif

	newtype = little_to_big(monsndx(ptr));
	if ((int) (++mtmp->m_lev) >= mons[newtype].mlevel
					&& newtype != monsndx(ptr)) {
		if (mons[newtype].geno & G_GENOD) { /* allow G_EXTINCT */
			pline("As %s grows up into %s, %s dies!",
				mon_nam(mtmp),
				an(mons[newtype].mname),
				mon_nam(mtmp));
			mondied(mtmp);
			return (struct permonst *)0;
		}
		mtmp->data = &mons[newtype];
		newsym(mtmp->mx, mtmp->my);	/* color may change */
		mtmp->m_lev = mons[newtype].mlevel;
	}
	if (newtype == monsndx(ptr) && victim &&
	    (int) mtmp->m_lev > (3*(int)mtmp->data->mlevel) / 2)
		mtmp->m_lev = (3*(int)mtmp->data->mlevel) / 2;
	if (mtmp->m_lev > 0) {
	    if (mtmp->mhp > (int) mtmp->m_lev * 8)
		mtmp->mhp = mtmp->m_lev * 8;
	    if (mtmp->mhpmax > (int) mtmp->m_lev * 8)
		mtmp->mhpmax = mtmp->m_lev * 8;
	}
	return(mtmp->data);
}

#endif /* OVLB */
#ifdef OVL1

int
mongets(mtmp, otyp)
register struct monst *mtmp;
register int otyp;
{
	register struct obj *otmp;

#ifdef MUSE
	if (!otyp) return 0;
#endif
	if((otmp = (otyp) ? mksobj(otyp,TRUE,FALSE) : mkobj((char)otyp,FALSE))) {
	    if (mtmp->data->mlet == S_DEMON) {
		/* demons always get cursed objects */
		curse(otmp);
	    } else if(is_lminion(mtmp->data)) {
		/* lawful minions don't get cursed, bad, or rusting objects */
		otmp->cursed = FALSE;
		if(otmp->spe < 0) otmp->spe = 0;
		otmp->oerodeproof = TRUE;
	    } else if(is_mplayer(mtmp->data) && is_sword(otmp))
			otmp->spe = (3 + rn2(4));
	    if(otmp->otyp == CANDELABRUM_OF_INVOCATION) {
		otmp->spe = 0;
		otmp->age = 0L;
		otmp->lamplit = FALSE;
		otmp->blessed = otmp->cursed = FALSE;
	    }
	    mpickobj(mtmp, otmp);
	    return(otmp->spe);
	} else return(0);
}

#endif /* OVL1 */
#ifdef OVLB

int
golemhp(type)
int type;
{
	switch(type) {
		case PM_STRAW_GOLEM: return 20;
		case PM_ROPE_GOLEM: return 30;
		case PM_LEATHER_GOLEM: return 40;
		case PM_WOOD_GOLEM: return 50;
		case PM_FLESH_GOLEM: return 40;
		case PM_CLAY_GOLEM: return 50;
		case PM_STONE_GOLEM: return 60;
		case PM_IRON_GOLEM: return 80;
		default: return 0;
	}
}

#endif /* OVLB */
#ifdef OVL1

/*
 *	Alignment vs. yours determines monster's attitude to you.
 *	( some "animal" types are co-aligned, but also hungry )
 */
boolean
peace_minded(ptr)
register struct permonst *ptr;
{
	aligntyp mal = ptr->maligntyp, ual = u.ualign.type;

	if (always_peaceful(ptr)) return TRUE;
	if (always_hostile(ptr)) return FALSE;
#ifdef MULDGN
	if (ptr->msound == MS_LEADER || ptr->msound == MS_GUARDIAN)
		return TRUE;
	if (ptr->msound == MS_NEMESIS)	return FALSE;
#endif

	/* the monster is hostile if its alignment is different from the
	 * player's */
	if (sgn(mal) != sgn(ual)) return FALSE;

	/* Negative monster hostile to player with Amulet. */
	if (mal < A_NEUTRAL && u.uhave.amulet) return FALSE;

	/* minions are hostile to players that have strayed at all */
	if (is_minion(ptr)) return(u.ualign.record >= 0);

	/* Last case:  a chance of a co-aligned monster being
	 * hostile.  This chance is greater if the player has strayed
	 * (u.ualign.record negative) or the monster is not strongly aligned.
	 */
	return !!rn2(16 + (u.ualign.record < -15 ? -15 : u.ualign.record)) &&
		!!rn2(2 + abs(mal));
}

/* Set malign to have the proper effect on player alignment if monster is
 * killed.  Negative numbers mean it's bad to kill this monster; positive
 * numbers mean it's good.  Since there are more hostile monsters than
 * peaceful monsters, the penalty for killing a peaceful monster should be
 * greater than the bonus for killing a hostile monster to maintain balance.
 * Rules:
 *   it's bad to kill peaceful monsters, potentially worse to kill always-
 *	peaceful monsters
 *   it's never bad to kill a hostile monster, although it may not be good
 */
void
set_malign(mtmp)
struct monst *mtmp;
{
	schar mal = mtmp->data->maligntyp;
	boolean coaligned;

	if (mtmp->ispriest || mtmp->isminion) {
		/* some monsters have individual alignments; check them */
		if (mtmp->ispriest)
			mal = EPRI(mtmp)->shralign;
		else if (mtmp->isminion)
			mal = EMIN(mtmp)->min_align;
		/* unless alignment is none, set mal to -5,0,5 */
		/* (see align.h for valid aligntyp values)     */
		if(mal != A_NONE)
			mal *= 5;
	}

	coaligned = (sgn(mal) == sgn(u.ualign.type));
#ifdef MULDGN
	if (mtmp->data->msound == MS_LEADER) {
		mtmp->malign = -20;
	} else
#endif
	      if (mal == A_NONE) {
		if (mtmp->mpeaceful)
			mtmp->malign = 0;
		else
			mtmp->malign = 20;	/* really hostile */
	} else if (always_peaceful(mtmp->data)) {
		if (mtmp->mpeaceful)
			mtmp->malign = -3*max(5,abs(mal));
		else
			mtmp->malign = 3*max(5,abs(mal)); /* renegade */
	} else if (always_hostile(mtmp->data)) {
		if (coaligned)
			mtmp->malign = 0;
		else
			mtmp->malign = max(5,abs(mal));
	} else if (coaligned) {
		if (mtmp->mpeaceful)
			mtmp->malign = -3*max(3,abs(mal));
		else	/* renegade */
			mtmp->malign = max(3,abs(mal));
	} else	/* not coaligned and therefore hostile */
		mtmp->malign = abs(mal);
}

#endif /* OVL1 */
#ifdef OVLB

static NEARDATA char syms[] = {
	MAXOCLASSES, MAXOCLASSES+1, RING_CLASS, WAND_CLASS, WEAPON_CLASS,
	FOOD_CLASS, GOLD_CLASS, SCROLL_CLASS, POTION_CLASS, ARMOR_CLASS,
	AMULET_CLASS, TOOL_CLASS, ROCK_CLASS, GEM_CLASS, SPBOOK_CLASS,
	S_MIMIC_DEF, S_MIMIC_DEF, S_MIMIC_DEF,
};

void
set_mimic_sym(mtmp)		/* KAA, modified by ERS */
register struct monst *mtmp;
{
	int typ, roomno, rt;
	unsigned appear, ap_type;
	int s_sym;
	struct obj *otmp;
	int mx, my;

	if (!mtmp) return;
	mx = mtmp->mx; my = mtmp->my;
	typ = levl[mx][my].typ;
					/* only valid for INSIDE of room */
	roomno = levl[mx][my].roomno - ROOMOFFSET;
	if (roomno >= 0)
		rt = rooms[roomno].rtype;
#ifdef SPECIALIZATION
	else if (IS_ROOM(typ))
		rt = OROOM,  roomno = 0;
#endif
	else	rt = 0;	/* roomno < 0 case for GCC_WARN */

	if (OBJ_AT(mx, my)) {
		ap_type = M_AP_OBJECT;
		appear = level.objects[mx][my]->otyp;
	} else if (IS_DOOR(typ) || IS_WALL(typ) ||
		   typ == SDOOR || typ == SCORR) {
		ap_type = M_AP_FURNITURE;
		/*
		 *  If there is a wall to the left that connects to this
		 *  location, then the mimic mimics a horizontal closed door.
		 *  This does not allow doors to be in corners of rooms.
		 */
		if (mx != 0 &&
			(levl[mx-1][my].typ == HWALL    ||
			 levl[mx-1][my].typ == TLCORNER ||
			 levl[mx-1][my].typ == TRWALL   ||
			 levl[mx-1][my].typ == BLCORNER ||
			 levl[mx-1][my].typ == TDWALL   ||
			 levl[mx-1][my].typ == CROSSWALL||
			 levl[mx-1][my].typ == TUWALL    ))
		    appear = S_hcdoor;
		else
		    appear = S_vcdoor;

		if(!mtmp->minvis || See_invisible)
		    block_point(mx,my);	/* vision */
	} else if (level.flags.is_maze_lev && rn2(2)) {
		ap_type = M_AP_OBJECT;
		appear = STATUE;
	} else if (roomno < 0) {
		ap_type = M_AP_OBJECT;
		appear = BOULDER;
		if(!mtmp->minvis || See_invisible)
		    block_point(mx,my);	/* vision */
	} else if (rt == ZOO || rt == VAULT) {
		ap_type = M_AP_OBJECT;
		appear = GOLD_PIECE;
	} else if (rt == DELPHI) {
		if (rn2(2)) {
			ap_type = M_AP_OBJECT;
			appear = STATUE;
		} else {
			ap_type = M_AP_FURNITURE;
			appear = S_fountain;
		}
	} else if (rt == TEMPLE) {
		ap_type = M_AP_FURNITURE;
		appear = S_altar;
	/*
	 * We won't bother with beehives, morgues, barracks, throne rooms
	 * since they shouldn't contain too many mimics anyway...
	 */
	} else if (rt >= SHOPBASE) {
		s_sym = get_shop_item(rt - SHOPBASE);
		if (s_sym < 0) {
			ap_type = M_AP_OBJECT;
			appear = -s_sym;
		} else {
			if (s_sym == RANDOM_CLASS)
				s_sym = syms[rn2((int)sizeof(syms)-2) + 2];
			goto assign_sym;
		}
	} else {
		s_sym = syms[rn2((int)sizeof(syms))];
assign_sym:
		if (s_sym >= MAXOCLASSES) {
			ap_type = M_AP_FURNITURE;
			appear = s_sym == MAXOCLASSES ? S_upstair : S_dnstair;
		} else if (s_sym == GOLD_CLASS) {
			ap_type = M_AP_OBJECT;
			appear = GOLD_PIECE;
		} else {
			ap_type = M_AP_OBJECT;
			if (s_sym == S_MIMIC_DEF) {
				appear = STRANGE_OBJECT;
			} else {
				otmp = mkobj( (char) s_sym, FALSE );
				appear = otmp->otyp;
				/* make sure container contents are free'ed */
				obfree(otmp, (struct obj *) 0);
			}
		}
	}
	mtmp->m_ap_type = ap_type;
	mtmp->mappearance = appear;
}

#endif /* OVLB */

/*makemon.c*/
