/*	SCCS Id: @(#)makemon.c	3.0	88/04/11
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

struct monst zeromonst;
static int uncommon P((struct permonst *));

int monstr[NUMMONS];

#define m_initsgrp(mtmp, x, y) m_initgrp(mtmp, x, y, 3)
#define m_initlgrp(mtmp, x, y) m_initgrp(mtmp, x, y, 10)
#define toostrong(monindx, lev) (monstr[monindx] > lev)
#define tooweak(monindx, lev)	(monstr[monindx] < lev)

static void
m_initgrp(mtmp, x, y, n)	/* make a group just like mtmp */
register struct monst *mtmp;
register int x, y, n;
{
	coord mm;
	register int cnt = rnd(n);
	struct monst *mon;

	mm.x = x;
	mm.y = y;
	while(cnt--) {
		if (peace_minded(mtmp->data)) continue;
		/* Don't create groups of peaceful monsters since they'll get
		 * in our way.  If the monster has a percentage chance so some
		 * are peaceful and some are not, the result will just be a
		 * smaller group.
		 */
		enexto(&mm, mm.x, mm.y, mtmp->data);
		mon = makemon(mtmp->data, mm.x, mm.y);
		mon->mpeaceful = 0;
		set_malign(mon);
		/* Undo the second peace_minded() check in makemon(); if the
		 * monster turned out to be peaceful the first time we didn't
		 * create it at all; we don't want a second check.
		 */
	}
}

static void
m_initthrow(mtmp,otyp,oquan)
struct monst *mtmp;
int otyp,oquan;
{
	register struct obj *otmp;

	otmp = mksobj(otyp,FALSE);
	otmp->quan = 2 + rnd(oquan);
	otmp->owt = weight(otmp);
#ifdef TOLKIEN
	if (otyp == ORCISH_ARROW) otmp->opoisoned = 1;
#endif
	mpickobj(mtmp, otmp);
}

static void
m_initweap(mtmp)
register struct monst *mtmp;
{
	register struct permonst *ptr = mtmp->data;
#ifdef REINCARNATION
	if (dlevel==rogue_level) return;
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
		if(is_mercenary(ptr))
		    switch(monsndx(ptr)) {

#ifdef ARMY
			case PM_SOLDIER:
			  (void) mongets(mtmp, rn2(2) ? SPEAR : SHORT_SWORD);
			  break;
			case PM_SERGEANT:
			  (void) mongets(mtmp, rn2(2) ? FLAIL : MACE);
			  break;
			case PM_LIEUTENANT:
			  (void) mongets(mtmp, rn2(2) ? GLAIVE : LONG_SWORD);
			  break;
			case PM_CAPTAIN:
			  (void) mongets(mtmp, rn2(2) ? LONG_SWORD : SCIMITAR);
			  break;
#endif
			default:    if (!rn2(4)) (void) mongets(mtmp, DAGGER);
				    if (!rn2(7)) (void) mongets(mtmp, SPEAR);
				    break;
		    }
		    break;

	    case S_HUMANOID:
#ifdef TOLKIEN
		if (monsndx(ptr) == PM_HOBBIT) {
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
		} else if (is_dwarf(ptr)) {
		    (void)mongets(mtmp, DWARVISH_CLOAK);
		    (void)mongets(mtmp, IRON_SHOES);
		    if (!rn2(4)) {
			(void)mongets(mtmp, DWARVISH_SHORT_SWORD);
			(void)mongets(mtmp,
			    rn2(3) ? DWARVISH_MATTOCK : AXE);
			(void)mongets(mtmp, DWARVISH_IRON_HELM);
			(void)mongets(mtmp, DWARVISH_ROUNDSHIELD);
			if (!rn2(3))
			    (void)mongets(mtmp, DWARVISH_MITHRIL_COAT);
		    } else {
			(void)mongets(mtmp, PICK_AXE);
		    }
		} else if (is_elf(ptr)) {
		    (void)mongets(mtmp,
			rn2(2) ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
		    if (rn2(2)) (void)mongets(mtmp, ELVEN_LEATHER_HELM);
		    if (rn2(3)) (void)mongets(mtmp, ELVEN_DAGGER);
		    switch (rn2(3)) {
			case 0:
			    if (!rn2(4)) (void)mongets(mtmp, ELVEN_SHIELD);
			    (void)mongets(mtmp, ELVEN_SHORT_SWORD);
			    (void)mongets(mtmp, ELVEN_BOW);
			    m_initthrow(mtmp, ELVEN_ARROW, 12);
			    break;
			case 1:
			    (void)mongets(mtmp, ELVEN_BROADSWORD);
			    if (rn2(2)) (void)mongets(mtmp, ELVEN_SHIELD);
			    break;
			case 2:
			    (void)mongets(mtmp, ELVEN_SPEAR);
			    (void)mongets(mtmp, ELVEN_SHIELD);
			    break;
		    }
		}
#else /* TOLKIEN */
		if (is_dwarf(ptr)) {
		    (void)mongets(mtmp, IRON_SHOES);
		    if (rn2(4) == 0) {
			(void)mongets(mtmp, SHORT_SWORD);
			(void)mongets(mtmp,
			    (rn2(3) == 0) ? AXE : TWO_HANDED_SWORD);
			(void)mongets(mtmp, LARGE_SHIELD);
			if (rn2(3) == 0)
			    (void)mongets(mtmp, DWARVISH_MITHRIL_COAT);
		    } else {
			(void)mongets(mtmp, PICK_AXE);
		    }
		} else if (is_elf(ptr)) {
		    (void)mongets(mtmp, ELVEN_CLOAK);
		    if (rn2(3)) (void)mongets(mtmp, DAGGER);
		    switch (rn2(3)) {
			case 0:
			    if (!rn2(4)) (void)mongets(mtmp, SMALL_SHIELD);
			    (void)mongets(mtmp, SHORT_SWORD);
			    (void)mongets(mtmp, BOW);
			    m_initthrow(mtmp, ARROW, 12);
			    break;
			case 1:
			    (void)mongets(mtmp, BROADSWORD);
			    if (rn2(2)) (void)mongets(mtmp, SMALL_SHIELD);
			    break;
			case 2:
			    (void)mongets(mtmp, SPEAR);
			    (void)mongets(mtmp, SMALL_SHIELD);
			    break;
		    }
		}
#endif /* TOLKIEN */
		break;
# ifdef KOPS
	    case S_KOP:		/* create Keystone Kops with cream pies to
				 * throw. As suggested by KAA.	   [MRS]
				 */
		if (!rn2(4)) m_initthrow(mtmp, CREAM_PIE, 2);
		if (!rn2(3)) (void)mongets(mtmp, (rn2(2)) ? CLUB : RUBBER_HOSE);
		break;
#endif
	    case S_ORC:
#ifdef TOLKIEN
		{ int mm = monsndx(ptr);
		  if(rn2(2)) (void)mongets(mtmp, ORCISH_HELM);
		  if (mm == PM_MORDOR_ORC ||
		     (mm == PM_ORC_CAPTAIN && rn2(2))) {
		      if(rn2(2)) (void)mongets(mtmp, SCIMITAR);
		      if(rn2(2)) (void)mongets(mtmp, ORCISH_SHIELD);
		      if(rn2(2)) (void)mongets(mtmp, KNIFE);
		      if(rn2(2)) (void)mongets(mtmp, ORCISH_CHAIN_MAIL);
		  } else if (mm == PM_URUK_HAI || mm == PM_ORC_CAPTAIN) {
		      if(rn2(2)) (void)mongets(mtmp, ORCISH_CLOAK);
		      if(rn2(2)) (void)mongets(mtmp, ORCISH_SHORT_SWORD);
		      if(rn2(2)) (void)mongets(mtmp, IRON_SHOES);
		      if(rn2(2)) {
			  (void)mongets(mtmp, ORCISH_BOW);
			  m_initthrow(mtmp, ORCISH_ARROW, 12);
		      }
		      if(rn2(2)) (void)mongets(mtmp, URUK_HAI_SHIELD);
		  } else if (mm != PM_ORC_SHAMAN) {
		      (void)mongets(mtmp, (mm == PM_GOBLIN || rn2(2) == 0) ?
				    ORCISH_DAGGER : SCIMITAR);
		  }
		}
#else /* TOLKIEN */
		{ int mm = monsndx(ptr);
		  if(rn2(2)) (void)mongets(mtmp, ORCISH_HELM);
		  if (mm == PM_ORC_CAPTAIN) {
		      if(rn2(2)) {
			  if(rn2(2)) (void)mongets(mtmp, SCIMITAR);
			  if(rn2(2)) (void)mongets(mtmp, SMALL_SHIELD);
			  if(rn2(2)) (void)mongets(mtmp, KNIFE);
			  if(rn2(2)) (void)mongets(mtmp, CHAIN_MAIL);
		      } else {
			  if(rn2(2)) (void)mongets(mtmp, SHORT_SWORD);
			  if(rn2(2)) (void)mongets(mtmp, IRON_SHOES);
			  if(rn2(2)) {
			      (void)mongets(mtmp, BOW);
			      m_initthrow(mtmp, ARROW, 12);
			  }
			  if(rn2(2)) (void)mongets(mtmp, SMALL_SHIELD);
		      }
		  } else if (mm != PM_ORC_SHAMAN) {
		      (void)mongets(mtmp, (mm == PM_GOBLIN || rn2(2) == 0) ?
				    DAGGER : SCIMITAR);
		  }
		}
#endif /* TOLKIEN */
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
	    case S_DEMON:
#ifdef HARD
		if (monsndx(ptr) == PM_BALROG) {
		    (void)mongets(mtmp, BULLWHIP);
		    (void)mongets(mtmp, BROADSWORD);
		    break;
		}
#endif
		/* prevent djinnis and mail daemons from leaving objects when
		 * they vanish
		 */
		if (!is_demon(ptr)) break;
		/* fall thru */
/*
 *	Now the general case, ~40% chance of getting some type
 *	of weapon. TODO: Add more weapons types (use bigmonst());
 */
	    default:
		switch(rnd(12)) {
		    case 1:
			m_initthrow(mtmp, DART, 12);
			break;
		    case 2:
			(void) mongets(mtmp, CROSSBOW);
			m_initthrow(mtmp, CROSSBOW_BOLT, 12);
			break;
		    case 3:
			(void) mongets(mtmp, BOW);
			m_initthrow(mtmp, ARROW, 12);
			break;
		    case 4:
			m_initthrow(mtmp, DAGGER, 3);
			break;
		    case 5:
			(void) mongets(mtmp, AKLYS);
			break;
		    default:
			break;
		}
		break;
	}
}

static void
m_initinv(mtmp)
register struct	monst	*mtmp;
{
	register int cnt;
	register struct obj *otmp;
	register struct permonst *ptr = mtmp->data;
#ifdef REINCARNATION
	if (dlevel==rogue_level) return;
#endif
/*
 *	Soldiers get armour & rations - armour approximates their ac.
 *	Nymphs may get mirror or potion of object detection.
 */
	switch(mtmp->data->mlet) {

	    case S_HUMAN:
		if(is_mercenary(ptr)) {
		    register int mac;

		    if((mac = ptr->ac) < -1)
			mac += 7 + mongets(mtmp, (rn2(5)) ?
					   PLATE_MAIL : CRYSTAL_PLATE_MAIL);
		    else if(mac < 3)
			mac += 6 + mongets(mtmp, (rn2(3)) ?
					   SPLINT_MAIL : BANDED_MAIL);
		    else
			mac += 3 + mongets(mtmp, (rn2(3)) ?
					   RING_MAIL : STUDDED_LEATHER_ARMOR);

		    if(mac < 10) {
			mac += 1 + mongets(mtmp, HELMET);
			if(mac < 10) {
			    mac += 1 + mongets(mtmp, SMALL_SHIELD);
			    if(mac < 10) {
				mac += 1 + mongets(mtmp, ELVEN_CLOAK);
				if(mac < 10)
				    mac += 1 +mongets(mtmp, LEATHER_GLOVES);
			    }
			}
		    }

		    if(mac != 10) {	/* make up the difference */
			otmp = mksobj(RIN_PROTECTION,FALSE);
			otmp->spe = (10 - mac);
			if(otmp->spe < 0) curse(otmp);
			mpickobj(mtmp, otmp);
		    }
#ifdef ARMY
		    if(ptr != &mons[PM_GUARD]) {
			if (!rn2(3)) (void) mongets(mtmp, K_RATION);
			if (!rn2(2)) (void) mongets(mtmp, C_RATION);
		    }
#endif
		}
		break;

	    case S_NYMPH:
#ifdef MEDUSA
		if(!rn2(2)) (void) mongets(mtmp, MIRROR);
#endif
		if(!rn2(2)) (void) mongets(mtmp, POT_OBJECT_DETECTION);
		break;

	    case S_GIANT:
		if(mtmp->data == &mons[PM_MINOTAUR])
		    (void) mongets(mtmp, WAN_DIGGING);
		else if (is_giant(mtmp->data)) {
		    for(cnt = rn2((int)(mtmp->m_lev / 2)); cnt; cnt--) {
			    do
				otmp = mkobj(GEM_SYM,FALSE);
			    while (otmp->otyp >= LAST_GEM+6);
			    otmp->quan = 2 + rnd(2);
			    otmp->owt = weight(otmp);
			    mpickobj(mtmp, otmp);
		    }
		}
		break;
#ifdef TOLKIEN
	    case S_WRAITH:
		if(mtmp->data == &mons[PM_NAZGUL]) {
			otmp = mksobj(RIN_INVISIBILITY, FALSE);
			curse(otmp);
			mpickobj(mtmp, otmp);
		}
		break;
#endif
	    default:
		break;
	}
}

/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: call mnexto (if !in_mklev)
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

	/* if caller wants random location, do it here */
	if(x == 0 && y == 0) {
		int uroom;
		int tryct = 0;	/* careful with bigrooms */

		if(!in_mklev) uroom = inroom(u.ux, u.uy);

		do {
			x = rn1(COLNO-3,2);
			y = rn2(ROWNO);
		} while(!goodpos(x, y, ptr) ||
			(!in_mklev && tryct++ < 50 && inroom(x, y) == uroom));
	}

	/* if a monster already exists at the position, return */
	if(MON_AT(x, y))
		return((struct monst *) 0);

	if(ptr){
		/* if you are to make a specific monster and it has 
		   already been genocided, return */
		if(ptr->geno & G_GENOD) return((struct monst *) 0);
	} else {
		/* make a random (common) monster. */
#ifdef REINCARNATION
		if (!(ptr = (dlevel==rogue_level) ? roguemon() : rndmonst()))
#else
		if(!(ptr = rndmonst()))
#endif
		{
#ifdef DEBUG
		    pline("Warning: no monster.");
#endif
		    return((struct monst *) 0);	/* no more monsters! */
		}
	}
	/* if it's unique, don't ever make it again */
	if (ptr->geno & G_UNIQ) ptr->geno &= G_GENOD;
/* gotmon:	/* label not referenced */
	mtmp = newmonst(ptr->pxlth);
	*mtmp = zeromonst;		/* clear all entries in structure */
	for(ct = 0; ct < ptr->pxlth; ct++)
		((char *) &(mtmp->mextra[0]))[ct] = 0;
 	if(type_is_pname(ptr))
 		Strcpy(NAME(mtmp), ptr->mname);
	mtmp->nmon = fmon;
	fmon = mtmp;
	mtmp->m_id = flags.ident++;
	mtmp->data = ptr;
	mtmp->mxlth = ptr->pxlth;

	mtmp->m_lev = adj_lev(ptr);
#ifdef GOLEMS
	if (is_golem(ptr))
	    mtmp->mhpmax = mtmp->mhp = golemhp(monsndx(ptr));
	else
#endif /* GOLEMS */
 	if(ptr->mlevel > 49) {
	    /* "special" fixed hp monster
	     * the hit points are encoded in the mlevel in a somewhat strange
	     * way to fit in the 50..127 positive range of a signed character
	     * above the 1..49 that indicate "normal" monster levels */
 	    mtmp->mhpmax = mtmp->mhp = 2*(ptr->mlevel - 6);
 	    mtmp->m_lev = mtmp->mhp / 4;	/* approximation */
 	} else if((ptr->mlet == S_DRAGON) && (ptr >= &mons[PM_GRAY_DRAGON]))
	    mtmp->mhpmax = mtmp->mhp = 80;
	else if(!mtmp->m_lev) mtmp->mhpmax = mtmp->mhp = rnd(4);
	else mtmp->mhpmax = mtmp->mhp = d((int)mtmp->m_lev, 8);
	place_monster(mtmp, x, y);
	mtmp->mcansee = 1;
	mtmp->mpeaceful = peace_minded(ptr);

	switch(ptr->mlet) {
		case S_MIMIC:
			set_mimic_sym(mtmp);
			break;
		case S_SPIDER:
		case S_SNAKE:
			mtmp->mhide = 1;
			if(in_mklev)
			    if(x && y)
				(void) mkobj_at(0, x, y);
			if(OBJ_AT(x, y) || levl[x][y].gmask)
			    mtmp->mundetected = 1;
			break;
		case S_CHAMELEON:
			/* If you're protected with a ring, don't create
			 * any shape-changing chameleons -dgk
			 */
			if (Protection_from_shape_changers)
				mtmp->cham = 0;
			else {
				mtmp->cham = 1;
				(void) newcham(mtmp, rndmonst());
			}
			break;
		case S_STALKER:
		case S_EEL:
			mtmp->minvis = 1;
			break;
		case S_LEPRECHAUN:
			mtmp->msleep = 1;
			break;
		case S_NYMPH:
			if(rn2(5) && !u.uhave_amulet) mtmp->msleep = 1;
			break;
		case S_UNICORN:
			if ((ptr==&mons[PM_WHITE_UNICORN] && 
				u.ualigntyp == U_LAWFUL) ||
			(ptr==&mons[PM_GRAY_UNICORN] && 
				u.ualigntyp == U_NEUTRAL) ||
			(ptr==&mons[PM_BLACK_UNICORN] && 
				u.ualigntyp == U_CHAOTIC))
				mtmp->mpeaceful = 1;
			break;
	}
	if (ptr == &mons[PM_WIZARD_OF_YENDOR]) {
		mtmp->iswiz = 1;
		flags.no_of_wizards++;
	}

	if(in_mklev) {
		if(((is_ndemon(ptr)) ||
		    (ptr == &mons[PM_WUMPUS]) ||
#ifdef WORM
		    (ptr == &mons[PM_LONG_WORM]) ||
#endif
		    (ptr == &mons[PM_GIANT_EEL])) && rn2(5))
			mtmp->msleep = 1;
	} else {
		if(x == u.ux && y == u.uy) {
			mnexto(mtmp);
			if (ptr->mlet == S_MIMIC) {
				set_mimic_sym(mtmp);
				unpmon(mtmp);
				pmon(mtmp);
			}
		}
	}
#ifdef HARD
	if(is_dprince(ptr)) {
	    mtmp->mpeaceful = mtmp->minvis = 1;
# ifdef NAMED_ITEMS
	    if(uwep)
		if(!strcmp(ONAME(uwep), "Excalibur"))
		    mtmp->mpeaceful = mtmp->mtame = 0;
# endif
	}
#endif
#ifdef WORM
	if(ptr == &mons[PM_LONG_WORM] && getwn(mtmp))  initworm(mtmp);
#endif
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
		m_initweap(mtmp);	/* equip with weapons / armour */
	m_initinv(mtmp);		/* add on a few special items */

	return(mtmp);
}

void
enexto(cc, xx, yy, mdat)
coord *cc;
register xchar xx, yy;
struct permonst *mdat;
{
	register xchar x,y;
	coord foo[15], *tfoo;
	int range, i;

	tfoo = foo;
	range = 1;
	do {	/* full kludge action. */
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x, yy-range, mdat)) {
				tfoo->x = x;
				(tfoo++)->y = yy-range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x, yy+range, mdat)) {
				tfoo->x = x;
				(tfoo++)->y = yy+range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx-range, y, mdat)) {
				tfoo->x = xx-range;
				(tfoo++)->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx+range, y, mdat)) {
				tfoo->x = xx+range;
				(tfoo++)->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		range++;
	} while(tfoo == foo);
foofull:
	i = rn2(tfoo - foo);
	cc->x = foo[i].x;
	cc->y = foo[i].y;
	return;
}

int
goodpos(x, y, mdat)
int x,y;
struct permonst *mdat;
{
	if (x < 1 || x > COLNO-2 || y < 1 || y > ROWNO-2 || MON_AT(x, y))
		return 0;
	if (x == u.ux && y == u.uy) return 0;
	if (mdat) {
	    if (IS_POOL(levl[x][y].typ))
		if (mdat == &playermon && HLevitation)	return 1;
		else	return (is_flyer(mdat) || is_swimmer(mdat));
	    if (passes_walls(mdat)) return 1;
	}
	if (!ACCESSIBLE(levl[x][y].typ)) return 0;
	if (IS_DOOR(levl[x][y].typ) &&
		    (levl[x][y].doormask & (D_LOCKED | D_CLOSED)) &&
		    (!mdat || !amorphous(mdat)))
		return 0;
	if (sobj_at(BOULDER, x, y) && (!mdat || !throws_rocks(mdat)))
		return 0;
	return 1;
}

void
rloc(mtmp)
struct monst *mtmp;
{
	register int tx, ty;

#ifdef WORM		/* do not relocate worms */
	if(mtmp->wormno && mtmp->mx) return;
#endif
	/* if the wiz teleports away to heal, try the up staircase,
	   to block the player's escaping before he's healed */
	if(!mtmp->iswiz || !goodpos(tx = xupstair, ty = yupstair, mtmp->data))
	   do {
		tx = rn1(COLNO-3,2);
		ty = rn2(ROWNO);
	   } while(!goodpos(tx,ty,mtmp->data));
	if(mtmp->mx != 0 && mtmp->my != 0)
		remove_monster(mtmp->mx, mtmp->my);
	place_monster(mtmp, tx, ty);
	if(u.ustuck == mtmp){
		if(u.uswallow) {
			u.ux = tx;
			u.uy = ty;
			docrt();
		} else	u.ustuck = 0;
	}
	pmon(mtmp);
	set_apparxy(mtmp);
}

static int
cmnum()	{	/* return the number of "common" monsters */

	int	i, count;

	for(i = count = 0; mons[i].mlet; i++)
	   if(!uncommon(&mons[i]))  count++;

	return(count);
}

static int
uncommon(ptr)
struct	permonst *ptr;
{
	return (ptr->geno & (G_GENOD | G_NOGEN | G_UNIQ)) ||
		(!Inhell ? ptr->geno & G_HELL : ptr->maligntyp > 0);
}

/* This routine is designed to return an integer value which represents
 * an approximation of monster strength.  It uses a similar method of
 * determination as "experience()" to arrive at the strength.
 */
static int
mstrength(ptr)
struct permonst *ptr;
{
	int	i, tmp2, n, tmp = ptr->mlevel;

 	if(tmp > 49)		/* special fixed hp monster */
	    tmp = 2*(tmp - 6) / 4;

/*	For creation in groups */
	n = (!!(ptr->geno & G_SGROUP));
	n += (!!(ptr->geno & G_LGROUP)) << 1;

/*	For ranged attacks */
	if (ranged_attk(ptr)) n++;

/*	For higher ac values */
	n += (ptr->ac < 0);

/*	For very fast monsters */
	n += (ptr->mmove >= 18);

/*	For each attack and "special" attack */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].aatyp;
	    n += (tmp2 > 0);
	    n += (tmp2 == AT_MAGC);
	    n += (tmp2 == AT_WEAP && strongmonst(ptr));
	}

/*	For each "special" damage type */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].adtyp;
	    if((tmp2 == AD_DRLI) || (tmp2 == AD_STON)
#ifdef POLYSELF
					|| (tmp2 == AD_WERE)
#endif
								) n += 2;
	    else n += (tmp2 != AD_PHYS);
	    n += ((ptr->mattk[i].damd * ptr->mattk[i].damn) > 23);
	}

/*	Leprechauns are special cases.  They have many hit dice so they
	can hit and are hard to kill, but they don't really do much damage. */
	if (ptr == &mons[PM_LEPRECHAUN]) n -= 2;

/*	Finally, adjust the monster level  0 <= n <= 24 (approx.) */
	if(n == 0) tmp--;
	else if(n >= 6) tmp += ( n / 2 );
	else tmp += ( n / 3 + 1);

	return((tmp >= 0) ? tmp : 0);
}

void
init_monstr()
{
	register int ct;

	for(ct = 0; mons[ct].mlet; ct++)
		monstr[ct] = mstrength(&(mons[ct]));
}

struct	permonst *
rndmonst() {		/* select a random monster */
	register struct permonst *ptr;
	register int i, ct;
	register int zlevel;
	static int minmlev, maxmlev, accept;
	static long oldmoves = 0L;	/* != 1, starting value of moves */

	if(oldmoves != moves) {		/* must recalculate accept */
	    oldmoves = moves;
	    zlevel = u.uhave_amulet ? MAXLEVEL : dlevel;
	    if(cmnum() <= 0) {
#ifdef DEBUG
		pline("cmnum() fails!");
#endif
		return((struct permonst *) 0);
	    }

	    /* determine the level of the weakest monster to make. */
	    minmlev = zlevel/6;
	    /* determine the level of the strongest monster to make. */
	    maxmlev = (zlevel + u.ulevel)>>1;
/*
 *	Find out how many monsters exist in the range we have selected.
 */
	    for(accept = ct = 0 ; mons[ct].mlet; ct++) {
		ptr = &(mons[ct]);
		if(uncommon(ptr)) continue;
		if(tooweak(ct, minmlev) || toostrong(ct, maxmlev))
		    continue;
		accept += (ptr->geno & G_FREQ);
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
	for(i = 0; mons[i].mlet && ct > 0; i++) {
		ptr = &(mons[i]);
		if(uncommon(ptr)) continue;
		if(tooweak(i, minmlev) || toostrong(i, maxmlev))
		    continue;
		ct -= (ptr->geno & G_FREQ);
	}
	if(ct > 0) {
#ifdef DEBUG
		pline("no count!");
#endif
		return((struct permonst *) 0);
	}
	return(ptr);
}

/*	The routine below is used to make one of the multiple types
 *	of a given monster class.  It will return 0 if no monsters
 *	in that class can be made.
 */

struct permonst *
mkclass(mlet)
char	mlet;
{
	register int	first, last, num = 0;

	if(!mlet) {
	    impossible("mkclass called with null arg!");
	    return((struct permonst *) 0);
	}
/*	Assumption #1:	monsters of a given class are contiguous in the
 *			mons[] array.
 */
	for(first = 0; mons[first].mlet != mlet; first++)
		if(!mons[first].mlet)	return((struct permonst *) 0);

	for(last = first; mons[last].mlet && mons[last].mlet == mlet; last++)
	    if(!(mons[last].geno & (G_GENOD | G_NOGEN | G_UNIQ)))
		num += mons[last].geno & G_FREQ;

	if(!num) return((struct permonst *) 0);

/*	Assumption #2:	monsters of a given class are presented in ascending
 *			order of strength.
 */
	for(num = rnd(num); num > 0; first++)
	    if(!(mons[first].geno & (G_GENOD | G_NOGEN | G_UNIQ))) { /* consider it */
		/* skew towards lower value monsters at lower exp. levels */
		if(adj_lev(&mons[first]) > (u.ulevel*2)) num--;
		num -= mons[first].geno & G_FREQ;
	    }
	first--; /* correct an off-by-one error */

	return(&mons[first]);
}

int
adj_lev(ptr)	/* adjust strength of monsters based on dlevel and u.ulevel */
register struct permonst *ptr;
{
	int	tmp, tmp2;

	if((tmp = ptr->mlevel) > 49) return(50); /* "special" demons/devils */
	tmp2 = (dlevel - tmp);
	if(tmp2 < 0) tmp--;		/* if mlevel > dlevel decrement tmp */
	else tmp += (tmp2 / 5);		/* else increment 1 per five diff */

	tmp2 = (u.ulevel - ptr->mlevel);	/* adjust vs. the player */
	if(tmp2 > 0) tmp += (tmp2 / 4);		/* level as well */

	tmp2 = 3 * ptr->mlevel/ 2;		/* crude upper limit */
	return((tmp > tmp2) ? tmp2 : (tmp > 0 ? tmp : 0)); /* 0 lower limit */
}

struct permonst *
grow_up(mtmp)		/* mon mtmp "grows up" to a bigger version. */
register struct monst *mtmp;
{
	register int newtype;
	register struct permonst *ptr = mtmp->data;

	if (ptr->mlevel >= 50 || mtmp->mhpmax <= 8*mtmp->m_lev)
	    return ptr;
	newtype = little_to_big(monsndx(ptr));
	if (++mtmp->m_lev >= mons[newtype].mlevel) {
		if (mons[newtype].geno & G_GENOD) {
			pline("As %s grows up into a%s %s, %s dies!",
				mon_nam(mtmp),
				index(vowels,*mons[newtype].mname) ? "n" : "",
				mons[newtype].mname,
				mon_nam(mtmp));
			mondied(mtmp);
			return (struct permonst *)0;
		}
		mtmp->data = &mons[newtype];
		mtmp->m_lev = mons[newtype].mlevel;
	}
	if (mtmp->m_lev > 3*mtmp->data->mlevel / 2)
		mtmp->m_lev = 3*mtmp->data->mlevel / 2;
	return(mtmp->data);
}

int
mongets(mtmp, otyp)
register struct monst *mtmp;
register int otyp;
{
	register struct obj *otmp;

	if((otmp = (otyp) ? mksobj(otyp,FALSE) : mkobj(otyp,FALSE))) {
	    if (mtmp->data->mlet == S_DEMON) {
		/* demons always get cursed objects */
		curse(otmp);
	    }
	    mpickobj(mtmp, otmp);
	    return(otmp->spe);	    
	} else return(0);
}

#ifdef REINCARNATION
struct permonst *
roguemon()
{
/* Make a monster for a Rogue-like level; only capital letters.  There are
 * no checks for "too hard" or "too easy", though dragons are specifically
 * ruled out because playtesting showed they made the level too hard.
 * Modified from rndmonst().
 */
#define isupper(x) ('A'<=(x) && (x)<='Z')
	register struct permonst *ptr;
	register int accept,ct,i;

	/* See how many there are. */
	accept = 0;
	for(ct = PM_APE ; isupper(mons[ct].mlet); ct++) {
		if (mons[ct].mlet == S_DRAGON) continue;
		ptr = &(mons[ct]);
		if(uncommon(ptr)) continue;
		accept += (ptr->geno & G_FREQ);
	}
	if(!accept) return((struct permonst *) 0);

	/* Now, select one at random. */
	ct = rnd(accept);
	for(i = PM_APE; isupper(mons[i].mlet) && ct > 0; i++) {
		if (mons[i].mlet == S_DRAGON) continue;
		ptr = &(mons[i]);
		if(uncommon(ptr)) continue;
		ct -= (ptr->geno & G_FREQ);
	}
	if(ct > 0) return((struct permonst *) 0);
	return(ptr);
}
#endif

#ifdef GOLEMS
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
#endif /* GOLEMS */

/*
 *	Alignment vs. yours determines monster's attitude to you.
 *	( some "animal" types are co-aligned, but also hungry )
 */
boolean
peace_minded(ptr)
register struct permonst *ptr;
{
	schar mal = ptr->maligntyp, ual = u.ualigntyp;

	if (always_peaceful(ptr)) return TRUE;
	if (always_hostile(ptr)) return FALSE;

	/* the monster is hostile if its alignment is different from the
	 * player's */
	if (sgn(mal) != sgn(ual)) return FALSE;

	/* Negative monster hostile to player with Amulet. */
	if (mal < 0 && u.uhave_amulet) return FALSE;

	/* Last case:  a chance of a co-aligned monster being
	 * hostile.  This chance is greater if the player has strayed
	 * (u.ualign negative) or the monster is not strongly aligned.
	 */
	return !!rn2(16 + (u.ualign < -15 ? -15 : u.ualign)) &&
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
	boolean coaligned = (sgn(mal) == sgn(u.ualigntyp));

	if (always_peaceful(mtmp->data))
		mtmp->malign = -3*max(5,abs(mal));
	else if (always_hostile(mtmp->data)) {
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

static char syms[] = { 0, 0, RING_SYM, WAND_SYM, WEAPON_SYM, FOOD_SYM, GOLD_SYM,
	SCROLL_SYM, POTION_SYM, ARMOR_SYM, AMULET_SYM, TOOL_SYM, ROCK_SYM,
	GEM_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	S_MIMIC_DEF, S_MIMIC_DEF, S_MIMIC_DEF,
};

void
set_mimic_sym(mtmp) /* KAA */
register struct monst *mtmp;
{
	int roomno, rt;
	char sym;
	if (!mtmp) return;

	syms[0] = UP_SYM;
	syms[1] = DN_SYM;

	mtmp->mimic = 1;
	roomno = inroom(mtmp->mx, mtmp->my);
	if (levl[mtmp->mx][mtmp->my].gmask)
		sym = GOLD_SYM;
	else if (OBJ_AT(mtmp->mx, mtmp->my))
		sym = level.objects[mtmp->mx][mtmp->my]->olet;
	else if (IS_DOOR(levl[mtmp->mx][mtmp->my].typ) ||
		 IS_WALL(levl[mtmp->mx][mtmp->my].typ))
		sym = DOOR_SYM;
	else if (is_maze_lev)
		sym = rn2(2) ? ROCK_SYM : syms[rn2(sizeof syms)];
	else if (roomno < 0)
		sym = ROCK_SYM;
	else if ((rt = rooms[roomno].rtype) == ZOO || rt == VAULT)
		sym = GOLD_SYM;
#ifdef ORACLE
	else if (rt == DELPHI)
		sym = rn2(2) ? ROCK_SYM : FOUNTAIN_SYM;
#endif
#ifdef ALTARS
	else if (rt == TEMPLE)
		sym = ALTAR_SYM;
#endif
	/* We won't bother with beehives, morgues, barracks, throne rooms
	 * since they shouldn't contain too many mimics anyway...
	 */
	else if (rt >= SHOPBASE) {
		int s_sym = get_shop_item(rt - SHOPBASE);

		if (s_sym < 0) sym = objects[-s_sym].oc_olet;
		else if (s_sym == RANDOM_SYM)
			sym = syms[rn2(sizeof(syms)-2) + 2];
		else sym = s_sym;
	} else sym = syms[rn2(sizeof syms)];
	mtmp->mappearance = sym;
}
