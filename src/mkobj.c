/*	SCCS Id: @(#)mkobj.c	3.1	93/02/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "prop.h"

STATIC_DCL void FDECL(mkbox_cnts,(struct obj *));

struct icp {
    int  iprob; /* probability of an item type */
    char ilet;	/* item class */
};

#ifdef OVL1

const struct icp mkobjprobs[] = {
{10, WEAPON_CLASS},
{10, ARMOR_CLASS},
{20, FOOD_CLASS},
{ 8, TOOL_CLASS},
{ 8, GEM_CLASS},
{16, POTION_CLASS},
{16, SCROLL_CLASS},
{ 4, SPBOOK_CLASS},
{ 4, WAND_CLASS},
{ 3, RING_CLASS},
{ 1, AMULET_CLASS}
};

const struct icp boxiprobs[] = {
{18, GEM_CLASS},
{15, FOOD_CLASS},
{18, POTION_CLASS},
{18, SCROLL_CLASS},
{12, SPBOOK_CLASS},
{ 7, GOLD_CLASS},
{ 6, WAND_CLASS},
{ 5, RING_CLASS},
{ 1, AMULET_CLASS}
};

#ifdef REINCARNATION
const struct icp rogueprobs[] = {
{12, WEAPON_CLASS},
{12, ARMOR_CLASS},
{22, FOOD_CLASS},
{22, POTION_CLASS},
{22, SCROLL_CLASS},
{ 5, WAND_CLASS},
{ 5, RING_CLASS}
};
#endif

const struct icp hellprobs[] = {
{20, WEAPON_CLASS},
{20, ARMOR_CLASS},
{16, FOOD_CLASS},
{12, TOOL_CLASS},
{10, GEM_CLASS},
{ 1, POTION_CLASS},
{ 1, SCROLL_CLASS},
{ 8, WAND_CLASS},
{ 8, RING_CLASS},
{ 4, AMULET_CLASS}
};

static NEARDATA int mksx=0, mksy=0;

struct obj *
mkobj_at(let,x,y, artif)
char let;
int x,y;
boolean artif;
{
	register struct obj *otmp;

	mksx = x; mksy = y;
	/* We need to know the X, Y coordinates while creating the object,
	 * to insure shop boxes are empty.
	 * Yes, this is a horrible kludge...
	 */
	otmp = mkobj(let,artif);
	otmp->nobj = fobj;
	fobj = otmp;
	place_object(otmp, x, y);
	mksx = mksy = 0;
	return(otmp);
}

struct obj *
mksobj_at(otyp,x,y,init)
int otyp,x,y;
boolean init;
{
	register struct obj *otmp;

	mksx = x; mksy = y;
	otmp = mksobj(otyp,init,TRUE);
	otmp->nobj = fobj;
	place_object(otmp, x, y);
	mksx = mksy = 0;
	return((fobj = otmp));
}

struct obj *
mkobj(let, artif)
char let;
boolean artif;
{
	register int tprob, i, prob = rnd(1000);

	if(let == RANDOM_CLASS) {
		const struct icp *iprobs =
#ifdef REINCARNATION
				    (Is_rogue_level(&u.uz)) ?
				    (const struct icp *)rogueprobs :
#endif
				    Inhell ? (const struct icp *)hellprobs :
				    (const struct icp *)mkobjprobs;

		for(tprob = rnd(100);
		    (tprob -= iprobs->iprob) > 0;
		    iprobs++);
		let = iprobs->ilet;
	}

	i = bases[letindex(let)];
	while((prob -= objects[i].oc_prob) > 0) i++;

	if(objects[i].oc_class != let || !OBJ_NAME(objects[i]))
		panic("probtype error, let=%c i=%d", let, i);

	return(mksobj(i, TRUE, artif));
}

STATIC_OVL void
mkbox_cnts(box)
/* Note: does not check to see if it overloaded the box capacity; usually
 * possible only with corpses in ice boxes.
 */
struct obj *box;
{
	register int n;
	register struct obj *otmp, *gold = 0;

	box->cobj = (struct obj *) 0;

	switch(box->otyp) {
		case ICE_BOX:		n = 20; break;
		case CHEST:		n = 5; break;
		case LARGE_BOX:		n = 3; break;
		case SACK:
		case OILSKIN_SACK:
		case BAG_OF_HOLDING:	n = 1; break;
		default:		n = 0; break;
	}

	for (n = rn2(n+1); n > 0; n--) {
	    if (box->otyp == ICE_BOX) {
		if (!(otmp = mksobj(CORPSE, TRUE, TRUE))) continue;
		/* Note: setting age to 0 is correct.  Age has a different
		 * from usual meaning for objects stored in ice boxes. -KAA
		 */
		otmp->age = 0L;
	    } else {
		register int tprob;
		const struct icp *iprobs = boxiprobs;

		for (tprob = rnd(100); (tprob -= iprobs->iprob) > 0; iprobs++)
		    ;
		if (!(otmp = mkobj(iprobs->ilet, TRUE))) continue;

		/* handle a couple of special cases */
		if (otmp->otyp == GOLD_PIECE) {
		    /* 2.5 x level's usual amount; weight adjusted below */
		    otmp->quan = (long)(rnd(level_difficulty()+2) * rnd(75));
		    if (gold) {			/* gold already in this box */
			gold->quan += otmp->quan;	/* merge */
			dealloc_obj(otmp);	/* note: not yet in any chain */
			continue;
		    } else {
			gold = otmp;		/* remember this object */
		    }
		} else while (otmp->otyp == ROCK) {
		    otmp->otyp = rnd_class(DILITHIUM_CRYSTAL, LOADSTONE);
		    if (otmp->quan > 2L) otmp->quan = 1L;
		    otmp->owt = weight(otmp);
		}
		if (box->otyp == BAG_OF_HOLDING) {
		    if (Is_mbag(otmp)) {
			otmp->otyp = SACK;
			otmp->spe = 0;
			otmp->owt = weight(otmp);
		    } else while (otmp->otyp == WAN_CANCELLATION)
			    otmp->otyp = rnd_class(WAN_LIGHT, WAN_LIGHTNING);
		}
	    }
	    otmp->nobj = box->cobj;
	    box->cobj = otmp;
	}
	if (gold) gold->owt = weight(gold);	/* quantity was diddled */
	return;
}

int
rndmonnum()	/* select a random, common monster type */
{
	register struct permonst *ptr;
	register int	i;

	/* Plan A: get a level-appropriate common monster */
	ptr = rndmonst();
	if (ptr) return(monsndx(ptr));

	/* Plan B: get any common monster */
	do {
	    ptr = &mons[(i = rn2(NUMMONS))];
	} while((ptr->geno & G_NOGEN) || (!Inhell && (ptr->geno & G_HELL)));

	return(i);
}

#endif /* OVL1 */
#ifdef OVLB
const char dknowns[] = { WAND_CLASS, RING_CLASS, POTION_CLASS, SCROLL_CLASS,
			 GEM_CLASS, SPBOOK_CLASS, WEAPON_CLASS, 0};

/*ARGSUSED*/
struct obj *
mksobj(otyp, init, artif)
int otyp;
boolean init;
boolean artif;
{
	int tryct;
	struct obj *otmp;
	char let = objects[otyp].oc_class;

	otmp = newobj(0);
	*otmp = zeroobj;
	otmp->age = monstermoves;
	otmp->o_id = flags.ident++;
	otmp->quan = 1L;
	otmp->oclass = let;
	otmp->otyp = otyp;
	otmp->dknown = index(dknowns, let) ? 0 : 1;
	if (!objects[otmp->otyp].oc_uses_known)
		otmp->known = 1;
	if (init) switch (let) {
	case WEAPON_CLASS:
		otmp->quan = (otmp->otyp <= SHURIKEN) ? (long) rn1(6,6) : 1L;
		if(!rn2(11)) {
			otmp->spe = rne(3);
			otmp->blessed = rn2(2);
		} else if(!rn2(10)) {
			curse(otmp);
			otmp->spe = -rne(3);
		} else	blessorcurse(otmp, 10);

		if (artif && !rn2(20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
		break;
	case FOOD_CLASS:
	    otmp->oeaten = 0;
	    switch(otmp->otyp) {
		case CORPSE:
		    /* overridden by mkcorpstat() */
		    do otmp->corpsenm = rndmonnum();
		    while (mons[otmp->corpsenm].geno & G_NOCORPSE);
		    break;
		case EGG:
		    if(!rn2(3)) {		/* "live" eggs */
			register struct permonst *ptr;
			for(tryct = 0;
			    (!(ptr = rndmonst()) ||
			    (!lays_eggs(ptr) && ptr != &mons[PM_KILLER_BEE])) &&
				tryct < 100;
			    tryct++);
			if(tryct < 100)	otmp->corpsenm = monsndx(ptr);
			else		otmp->corpsenm = -1; /* empty */
		    } else		otmp->corpsenm = -1; /* empty */
		    break;
		case TIN:
		    if(!rn2(6)) {
			otmp->spe = 1;		/* spinach */
			otmp->corpsenm = -1;
		    } else do {
			otmp->corpsenm = rndmonnum();
		    } while (mons[otmp->corpsenm].geno & G_NOCORPSE);
		    blessorcurse(otmp, 10);
		    break;
#ifdef TUTTI_FRUTTI
		case SLIME_MOLD:
		    otmp->spe = current_fruit;
		    break;
#endif
	    }
	    if (otmp->otyp == CORPSE) break;
	    /* fall into next case */

	case GEM_CLASS:
		if (otmp->otyp == LOADSTONE) curse(otmp);
		else if (otmp->otyp == ROCK) otmp->quan = (long) rn1(6,6);
		else if (otmp->otyp != LUCKSTONE && !rn2(6)) otmp->quan = 2L;
		else otmp->quan = 1L;
		break;
	case TOOL_CLASS:
	    switch(otmp->otyp) {
		case TALLOW_CANDLE:
		case WAX_CANDLE:	otmp->spe = 1;
					otmp->age = 20L * /* 400 or 200 */
					      (long)objects[otmp->otyp].oc_cost;
					otmp->lamplit = 0;
					otmp->quan = 1L +
					      (long)(rn2(2) ? rn2(7) : 0);
					blessorcurse(otmp, 5);
					break;
		case BRASS_LANTERN:
		case OIL_LAMP:		otmp->spe = 1;
					otmp->age = (long) rn1(500,1000);
					otmp->lamplit = 0;
					blessorcurse(otmp, 5);
					break;
		case MAGIC_LAMP:	otmp->spe = 1;
					otmp->lamplit = 0;
					blessorcurse(otmp, 2);
					break;
		case CHEST:
		case LARGE_BOX:		otmp->olocked = !!(rn2(5));
					otmp->otrapped = !(rn2(10));
		case ICE_BOX:
		case SACK:
		case OILSKIN_SACK:
		case BAG_OF_HOLDING:	mkbox_cnts(otmp);
					break;
		case MAGIC_MARKER:	otmp->spe = rn1(70,30);
					break;
		case CAN_OF_GREASE:	otmp->spe = rnd(25);
					blessorcurse(otmp, 10);
					break;
		case CRYSTAL_BALL:	otmp->spe = rnd(5);
					blessorcurse(otmp, 2);
					break;
		case HORN_OF_PLENTY:
		case BAG_OF_TRICKS:	otmp->spe = rnd(20);
					break;
		case FIGURINE:	{	int tryct2 = 0;
					do
					    otmp->corpsenm = rndmonnum();
					while(is_human(&mons[otmp->corpsenm])
						&& tryct2++ < 30);
					blessorcurse(otmp, 4);
					break;
				}
		case BELL_OF_OPENING:   otmp->spe = 3;
					break;
		case MAGIC_FLUTE:
		case MAGIC_HARP:
		case FROST_HORN:
		case FIRE_HORN:
		case DRUM_OF_EARTHQUAKE:
					otmp->spe = rn1(5,4);
					break;
	    }
	    break;
	case AMULET_CLASS:
		if(rn2(10) && (otmp->otyp == AMULET_OF_STRANGULATION ||
		   otmp->otyp == AMULET_OF_CHANGE ||
		   otmp->otyp == AMULET_OF_RESTFUL_SLEEP)) {
			curse(otmp);
		} else	blessorcurse(otmp, 10);
	case VENOM_CLASS:
	case CHAIN_CLASS:
	case BALL_CLASS:
		break;
	case POTION_CLASS:
	case SCROLL_CLASS:
#ifdef MAIL
		if (otmp->otyp != SCR_MAIL)
#endif
			blessorcurse(otmp, 4);
		break;
	case SPBOOK_CLASS:
		blessorcurse(otmp, 17);
		break;
	case ARMOR_CLASS:
		if(rn2(10) && (otmp->otyp == FUMBLE_BOOTS ||
		   otmp->otyp == LEVITATION_BOOTS ||
		   otmp->otyp == HELM_OF_OPPOSITE_ALIGNMENT ||
		   otmp->otyp == GAUNTLETS_OF_FUMBLING ||
		   !rn2(11))) {
			curse(otmp);
			otmp->spe = -rne(3);
		} else if(!rn2(10)) {
			otmp->blessed = rn2(2);
			otmp->spe = rne(3);
		} else	blessorcurse(otmp, 10);
		break;
	case WAND_CLASS:
		if(otmp->otyp == WAN_WISHING) otmp->spe = rnd(3); else
		otmp->spe = rn1(5,
			(objects[otmp->otyp].oc_dir == NODIR) ? 11 : 4);
		blessorcurse(otmp, 17);
		otmp->recharged = 0; /* used to control recharging */
		break;
	case RING_CLASS:
		if(objects[otmp->otyp].oc_charged) {
		    blessorcurse(otmp, 3);
		    if(rn2(10)) {
			if(rn2(10) && bcsign(otmp))
			    otmp->spe = bcsign(otmp) * rne(3);
			else otmp->spe = rn2(2) ? rne(3) : -rne(3);
		    }
		    /* make useless +0 rings much less common */
		    if (otmp->spe == 0) otmp->spe = rn2(4) - rn2(3);
		    /* negative rings are usually cursed */
		    if (otmp->spe < 0 && rn2(5)) curse(otmp);
		} else if(rn2(10) && (otmp->otyp == RIN_TELEPORTATION ||
#ifdef POLYSELF
			  otmp->otyp == RIN_POLYMORPH ||
#endif
			  otmp->otyp == RIN_AGGRAVATE_MONSTER ||
			  otmp->otyp == RIN_HUNGER || !rn2(9))) {
			curse(otmp);
		}
		break;
	case ROCK_CLASS:
		switch (otmp->otyp) {
		    case STATUE:
			if (rn2(level_difficulty()/2 + 10) > 10) {
				struct obj *book;
				book = mkobj(SPBOOK_CLASS,FALSE);
				otmp->cobj = book;
			}
			/* overridden by mkcorpstat() */
			otmp->corpsenm = rndmonnum();
		}
		break;
	case GOLD_CLASS:
		break;	/* do nothing */
	default:
		impossible("impossible mkobj %d, sym '%c'.", otmp->otyp, objects[otmp->otyp].oc_class);
		return (struct obj *)0;
	}
	/* unique objects may have an associated artifact entry */
	if (objects[otyp].oc_unique && !otmp->oartifact)
	    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	otmp->owt = weight(otmp);
	return(otmp);
}

void
bless(otmp)
register struct obj *otmp;
{
	otmp->cursed = 0;
	otmp->blessed = 1;
	if (otmp->otyp == LUCKSTONE
	    || (otmp->oartifact && spec_ability(otmp, SPFX_LUCK))) {
	    int luckbon = stone_luck(TRUE);
	    if(!luckbon && !carrying(LUCKSTONE)) u.moreluck = 0;
	    else if (luckbon >= 0) u.moreluck = LUCKADD;
	    else u.moreluck = -LUCKADD;
	} else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
	return;
}

void
unbless(otmp)
register struct obj *otmp;
{
	otmp->blessed = 0;
	if (otmp->otyp == LUCKSTONE
	    || (otmp->oartifact && spec_ability(otmp, SPFX_LUCK))) {
	    int luckbon = stone_luck(TRUE);
	    if (!luckbon && !carrying(LUCKSTONE)) u.moreluck = 0;
	    else if (luckbon >= 0) u.moreluck = LUCKADD;
	    else u.moreluck = -LUCKADD;
	} else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
}

void
curse(otmp)
register struct obj *otmp;
{
	otmp->blessed = 0;
	otmp->cursed = 1;
	if (otmp->otyp == LUCKSTONE
	    || (otmp->oartifact && spec_ability(otmp, SPFX_LUCK))) {
	    int luckbon = stone_luck(TRUE);
	    if (!luckbon && !carrying(LUCKSTONE)) u.moreluck = 0;
	    else if (luckbon >= 0) u.moreluck = LUCKADD;
	    else u.moreluck = -LUCKADD;
	} else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
	return;
}

void
uncurse(otmp)
register struct obj *otmp;
{
	otmp->cursed = 0;
	if (otmp->otyp == LUCKSTONE
	    || (otmp->oartifact && spec_ability(otmp, SPFX_LUCK))) {
	    int luckbon = stone_luck(TRUE);
	    if (!luckbon && !carrying(LUCKSTONE)) u.moreluck = 0;
	    else if (luckbon >= 0) u.moreluck = LUCKADD;
	    else u.moreluck = -LUCKADD;
	} else if (otmp->otyp == BAG_OF_HOLDING)
		otmp->owt = weight(otmp);
}

#endif /* OVLB */
#ifdef OVL1
void
blessorcurse(otmp, chance)
register struct obj *otmp;
register int chance;
{
	if(otmp->blessed || otmp->cursed) return;

	if(!rn2(chance))
	    if(!rn2(2)) {
		curse(otmp);
	    } else {
		bless(otmp);
	    }
	return;
}

#endif /* OVL1 */
#ifdef OVLB

int
bcsign(otmp)
register struct obj *otmp;
{
	return(!!otmp->blessed - !!otmp->cursed);
}

#endif /* OVLB */
#ifdef OVL0

/*
 *  Calculate the weight of the given object.  This will recursively follow
 *  and calculate the weight of any containers.
 *
 *  Note:  It is possible to end up with an incorrect weight if some part
 *	   of the code messes with a contained object and doesn't update the
 *	   container's weight.
 */
int
weight(obj)
register struct obj *obj;
{
	int wt = objects[obj->otyp].oc_weight;

	if (Is_container(obj) || obj->otyp == STATUE) {
		struct obj *contents;
		register int cwt = 0;

		if (obj->otyp == STATUE && obj->corpsenm > -1)
		    wt = (int)obj->quan *
			 ((int)mons[obj->corpsenm].cwt * 3 / 2);

		for(contents=obj->cobj; contents; contents=contents->nobj)
			cwt += weight(contents);
		/*
		 *  The weight of bags of holding is calculated as the weight
		 *  of the bag plus the weight of the bag's contents modified
		 *  as follows:
		 *
		 *	Bag status	Weight of contents
		 *	----------	------------------
		 *	cursed			2x
		 *	blessed			x/4 + 1
		 *	otherwise		x/2 + 1
		 *
		 *  The macro DELTA_CWT in pickup.c also implements these
		 *  weight equations.
		 *
		 *  Note:  The above checks are performed in the given order.
		 *	   this means that if an object is both blessed and
		 *	   cursed (not supposed to happen), it will be treated
		 *	   as cursed.
		 */
		if (obj->otyp == BAG_OF_HOLDING)
		    cwt = obj->cursed ? (cwt * 2) :
					(1 + (cwt / (obj->blessed ? 4 : 2)));

		return wt + cwt;
	}
	if (obj->otyp == CORPSE && obj->corpsenm > -1)
		return (int)obj->quan * mons[obj->corpsenm].cwt;
	else if (obj->otyp == GOLD_PIECE)
		return (int)((obj->quan + 50L) / 100L);
	return(wt ? wt*(int)obj->quan : ((int)obj->quan + 1)>>1);
}

#endif /* OVL0 */
#ifdef OVLB

void
mkgold(amount, x, y)
long amount;
int x, y;
{
    register struct obj *gold = g_at(x,y);

    if (amount <= 0L) amount = (long)(1 + rnd(level_difficulty()+2) * rnd(30));
    if (gold) {
	gold->quan += amount;
    } else {
	gold = mksobj_at(GOLD_PIECE,x,y,TRUE);
	gold->quan = amount;
    }
    gold->owt = weight(gold);
}

#endif /* OVLB */
#ifdef OVL1
struct obj *
mkcorpstat(objtype, ptr, x, y, init)
int objtype;	/* CORPSE or STATUE */
register struct permonst *ptr;
int x, y;
boolean init;
{
	register struct obj *otmp;

	if(objtype != CORPSE && objtype != STATUE)
		impossible("making corpstat type %d", objtype);
	otmp = mksobj_at(objtype, x, y, init);
	if(otmp)  {
		if(ptr)	otmp->corpsenm = monsndx(ptr);
		else	otmp->corpsenm = rndmonnum();
		otmp->owt = weight(otmp);
	}
	return(otmp);
}

#endif /* OVL1 */
#ifdef OVLB
struct obj *
mk_tt_object(objtype, x, y)
int objtype; /* CORPSE or STATUE */
register int x, y;
{
	register struct obj *otmp;

	/* player statues never contain books */
	if ((otmp = mksobj_at(objtype,x,y,FALSE)) != 0)
		otmp = tt_oname(otmp);
	return(otmp);
}

/* make a new corpse or statue, uninitialized if a statue (i.e. no books) */
struct obj *
mk_named_object(objtype, ptr, x, y, nm, lth)
int objtype; /* CORPSE or STATUE */
register struct permonst *ptr;
int x, y;
char * nm;
register int lth;
{
	struct obj *otmp;

	otmp = mkcorpstat(objtype,ptr,x,y,(objtype != STATUE));
	if (lth > 0) {
		/* Note: oname() is safe since otmp is first in both chains */
		otmp = oname(otmp, nm, FALSE);
		fobj = otmp;
		level.objects[x][y] = otmp;
	}
	return(otmp);
}

boolean
is_flammable(otmp)
register struct obj *otmp;
{
	int otyp = otmp->otyp;

	if (objects[otyp].oc_oprop == FIRE_RES) return FALSE;

	return((objects[otyp].oc_material <= WOOD &&
			objects[otyp].oc_material != LIQUID));
}

#endif /* OVLB */
#ifdef OVL1

/*
 * These routines maintain the single-linked lists headed in level.objects[][]
 * and threaded through the nexthere fields in the object-instance structure.
 */

void
place_object(otmp, x, y)
/* put an object on top of the pile at the given location */
register struct obj *otmp;
int x, y;
{
    register struct obj *otmp2 = level.objects[x][y];

    if (otmp->otyp == BOULDER) block_point(x,y);	/* vision */

    if (otmp2 && (otmp2->otyp == BOULDER)) {
	otmp->nexthere = otmp2->nexthere;
	otmp2->nexthere = otmp;
    } else {
	otmp->nexthere = otmp2;
	level.objects[x][y] = otmp;
    }

    /* set the new object's location */
    otmp->ox = x;
    otmp->oy = y;
}

#endif /* OVL1 */
#ifdef OVLB
void
remove_object(otmp)
register struct obj *otmp;
{
    register struct obj *odel;

    if (otmp->otyp == BOULDER) unblock_point(otmp->ox,otmp->oy); /* vision */

    if (otmp == level.objects[otmp->ox][otmp->oy])
	level.objects[otmp->ox][otmp->oy] = otmp->nexthere;
    else
	for (odel = level.objects[otmp->ox][otmp->oy];
						    odel; odel = odel->nexthere)
	    if (odel->nexthere == otmp) {
		odel->nexthere = otmp->nexthere;
		break;
	    }
}

void
move_object(otmp, x, y)
register struct obj *otmp;
int x, y;
{
    remove_object(otmp);
    place_object(otmp, x, y);
}

#endif /* OVLB */

/*mkobj.c*/
