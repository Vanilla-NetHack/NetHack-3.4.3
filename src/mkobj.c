/*	SCCS Id: @(#)mkobj.c	3.0	88/10/30
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct icp {
    int  iprob; /* probability of an item type */
    char ilet;	/* item class */
};

const struct icp mkobjprobs[] = {
{10, WEAPON_SYM},
{10, ARMOR_SYM},
{20, FOOD_SYM},
{ 8, TOOL_SYM},
{ 8, GEM_SYM},
#ifdef SPELLS
{16, POTION_SYM},
{16, SCROLL_SYM},
{ 4, SPBOOK_SYM},
#else
{18, POTION_SYM},
{18, SCROLL_SYM},
#endif
{ 4, WAND_SYM},
{ 3, RING_SYM},
{ 1, AMULET_SYM}};

const struct icp boxiprobs[] = {
{18, GEM_SYM},
#ifdef SPELLS
{15, FOOD_SYM},
{20, POTION_SYM},
{20, SCROLL_SYM},
{15, SPBOOK_SYM},
#else
{20, FOOD_SYM},
{25, POTION_SYM},
{25, SCROLL_SYM},
#endif
{ 6, WAND_SYM},
{ 5, RING_SYM},
{ 1, AMULET_SYM}};

#ifdef REINCARNATION
const struct icp rogueprobs[] = {
{12, WEAPON_SYM},
{12, ARMOR_SYM},
{22, FOOD_SYM},
{22, POTION_SYM},
{22, SCROLL_SYM},
{ 5, WAND_SYM},
{ 5, RING_SYM}};
#endif

const struct icp hellprobs[] = {
{20, WEAPON_SYM},
{20, ARMOR_SYM},
{16, FOOD_SYM},
{12, TOOL_SYM},
{10, GEM_SYM},
{ 1, POTION_SYM},
{ 1, SCROLL_SYM},
{ 8, WAND_SYM},
{ 8, RING_SYM},
{ 4, AMULET_SYM}};

static int mksx=0, mksy=0;

struct obj *
mkobj_at(let,x,y)
char let;
int x,y;
{
	register struct obj *otmp;

	mksx = x; mksy = y;
	/* We might need to know the X, Y coordinates while creating the
	 * object, i.e. to insure shop boxes are empty.
	 * Yes, this is a horrible kludge...
	 */
	otmp = mkobj(let,TRUE);
	otmp->ox = x;
	otmp->oy = y;
	otmp->nobj = fobj;
	fobj = otmp;
	levl[x][y].omask = 1;
	mksx = mksy = 0;
	return(otmp);
}

struct obj *
mksobj_at(otyp,x,y)
int otyp,x,y;
{
	register struct obj *otmp;

	mksx = x; mksy = y;
	otmp = mksobj(otyp,TRUE);
	otmp->ox = x;
	otmp->oy = y;
	otmp->nobj = fobj;
	levl[x][y].omask = 1;
	mksx = mksy = 0;
	return((fobj = otmp));
}

struct obj *
mkobj(let, artif)
char let;
boolean artif;
{
	register int tprob, i, prob = rnd(1000);

	if(let == RANDOM_SYM) {
		struct icp *iprobs =
#ifdef REINCARNATION
				    (dlevel == rogue_level) ? rogueprobs :
#endif
				    Inhell ? hellprobs :
				    mkobjprobs;

		for(tprob = rnd(100);
		    (tprob -= iprobs->iprob) > 0;
		    iprobs++);
		let = iprobs->ilet;
	}

	i = bases[letindex(let)];
	while((prob -= objects[i].oc_prob) > 0) i++;

	if(objects[i].oc_olet != let || !objects[i].oc_name)
		panic("probtype error, let=%c i=%d", let, i);

	return(mksobj(i, artif));
}

static void
mkbox_cnts(box)
/* Note: does not check to see if it overloaded the box capacity; usually
 * possible only with corpses in ice boxes.
 */
struct obj *box;
{
	register int n;
	register struct obj *otmp;

	if(in_shop(mksx, mksy)) return; /* boxes are empty in shops */

	switch(box->otyp) {
		case ICE_BOX: 		n = 20; break;
		case CHEST:		n = 5; break;
		case LARGE_BOX:		n = 3; break;
		case SACK:
		case BAG_OF_HOLDING:	n = 1; break;
		default:		n = 0; break;
	}

	for(n = rn2(n+1); n > 0; n--) {
	    if (box->otyp == ICE_BOX) {
		otmp = mksobj(CORPSE, TRUE);
		/* Note: setting age to 0 is correct.  Age has a different
		 * from usual meaning for objects stored in ice boxes. -KAA
		 */
		otmp->age = 0;
	    } else {
		register int tprob;
		struct icp *iprobs = boxiprobs;

		for(tprob = rnd(100);
		    (tprob -= iprobs->iprob) > 0;
		    iprobs++);
		otmp = mkobj(iprobs->ilet, TRUE);
	    }
	    if (otmp) {
		otmp->cobj = box;
		otmp->nobj = fcobj;
		fcobj = otmp;
		/* inc_cwt(box, otmp); --done by weight() */
	    }
	}
	return;
}

int
rndmonnum() {	/* select a random, common monster type */

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

const char dknowns[] = { WAND_SYM, RING_SYM, POTION_SYM, SCROLL_SYM, GEM_SYM,
#ifdef SPELLS
SPBOOK_SYM,
#endif
WEAPON_SYM, 0};

/*ARGSUSED*/
struct obj *
mksobj(otyp, artif)
int otyp;
boolean artif;
{
	int tryct;
	struct obj *otmp;
	char let = objects[otyp].oc_olet;

	otmp = newobj(0);
	*otmp = zeroobj;
	otmp->age = moves;
	otmp->o_id = flags.ident++;
	otmp->quan = 1;
	otmp->olet = let;
	otmp->otyp = otyp;
	otmp->dknown = index(dknowns, let) ? 0 : 1;
	if (!uses_known(otmp))
		otmp->known = 1;
	switch(let) {
	case WEAPON_SYM:
		otmp->quan = (otmp->otyp <= SHURIKEN) ? rn1(6,6) : 1;
		if(!rn2(11)) {
			otmp->spe = rne(2);
			otmp->blessed = rn2(2);
		} else if(!rn2(10)) {
			curse(otmp);
			otmp->spe = -rne(2);
		} else	blessorcurse(otmp, 10);

#ifdef NAMED_ITEMS
		if(artif && !rn2(20)) mkartifact(&otmp);
#endif
		break;
	case FOOD_SYM:
		if(otmp->otyp == CORPSE) {
		    /* overridden by mkcorpse_at() */
		    do otmp->corpsenm = rndmonnum();
		    while (mons[otmp->corpsenm].geno & G_NOCORPSE);
		    break;
		} else if(otmp->otyp == EGG) {
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
		} else if(otmp->otyp == TIN) {
		    if(!rn2(10)) {
			otmp->spe = 1;		/* spinach */
			otmp->corpsenm = -1;
		    } else do {
			otmp->corpsenm = rndmonnum();
		    } while (mons[otmp->corpsenm].geno & G_NOCORPSE);
		    blessorcurse(otmp, 10);
		}
#ifdef TUTTI_FRUTTI
		else if (otmp->otyp == SLIME_MOLD)
		    otmp->spe = current_fruit;
#endif
		/* fall into next case */
	case GEM_SYM:
		if (otmp->otyp == LOADSTONE) curse(otmp);
		else if (otmp->otyp == ROCK) otmp->quan = rn1(6,6);
		else if (otmp->otyp != LUCKSTONE && !rn2(6)) otmp->quan = 2;
		else otmp->quan = 1;
		break;
	case TOOL_SYM:
	    switch(otmp->otyp) {
		case LAMP:		otmp->spe = rnd(10);
					blessorcurse(otmp, 5);
					break;
		case MAGIC_LAMP:	otmp->spe = 1;
					otmp->recharged = 0;
					blessorcurse(otmp, 2);
					break;
		case KEY:		/* key # index */
		case SKELETON_KEY:	otmp->spe = rn2(N_LOX);
					break;
		case CHEST:		/* lock # index */
		case LARGE_BOX:		otmp->spe = rn2(N_LOX);
					otmp->olocked = !!(rn2(5));
					otmp->otrapped = !(rn2(10));
		case ICE_BOX:
		case SACK:
		case BAG_OF_HOLDING:	mkbox_cnts(otmp);
					break;
		case MAGIC_MARKER:	otmp->spe = rn1(70,30);
					break;
		case CRYSTAL_BALL:	otmp->spe = rnd(5);
					blessorcurse(otmp, 2);
					break;
		case BAG_OF_TRICKS:	otmp->spe = rnd(20);
					break;
		case FIGURINE:		otmp->corpsenm = rndmonnum();
					blessorcurse(otmp, 4);
					break;
#ifdef MUSIC
		case MAGIC_FLUTE:
		case MAGIC_HARP:
		case FROST_HORN:
		case FIRE_HORN:
		case DRUM_OF_EARTHQUAKE:
					otmp->spe = rn1(5,4);
					break;
#endif /* MUSIC /**/
	    }
	    break;
	case AMULET_SYM:
		if(rn2(10) && (otmp->otyp == AMULET_OF_STRANGULATION ||
		   otmp->otyp == AMULET_OF_CHANGE ||
		   otmp->otyp == AMULET_OF_RESTFUL_SLEEP)) {
			curse(otmp);
		} else	blessorcurse(otmp, 10);
	case VENOM_SYM:
	case CHAIN_SYM:
	case BALL_SYM:
		break;
	case POTION_SYM:
	case SCROLL_SYM:
#ifdef MAIL
		if (otmp->otyp != SCR_MAIL)
#endif
			blessorcurse(otmp, 4);
		break;
#ifdef SPELLS
	case SPBOOK_SYM:
		blessorcurse(otmp, 17);
		break;
#endif
	case ARMOR_SYM:
		if(rn2(10) && (otmp->otyp == FUMBLE_BOOTS ||
		   otmp->otyp == LEVITATION_BOOTS ||
		   otmp->otyp == HELM_OF_OPPOSITE_ALIGNMENT ||
		   otmp->otyp == GAUNTLETS_OF_FUMBLING ||
		   !rn2(11))) {
			curse(otmp);
			otmp->spe = -rne(2);
		} else if(!rn2(10)) {
			otmp->spe = rne(2);
			otmp->blessed = rn2(2);
		} else	blessorcurse(otmp, 10);
		if(otmp->otyp == DRAGON_SCALE_MAIL)
			otmp->corpsenm = PM_GREY_DRAGON +
			    rn2(PM_YELLOW_DRAGON-PM_GREY_DRAGON+1);
		break;
	case WAND_SYM:
#ifdef HARD
		if(otmp->otyp == WAN_WISHING) otmp->spe = rnd(3); else
#else		
		if(otmp->otyp == WAN_WISHING) otmp->spe = 3; else
#endif		
		otmp->spe = rn1(5,
			(objects[otmp->otyp].bits & NODIR) ? 11 : 4);
		blessorcurse(otmp, 17);
		otmp->recharged = 0; /* used to control recharging */
		break;
	case RING_SYM:
		if(objects[otmp->otyp].oc_charged) {
		    blessorcurse(otmp, 3);
		    if(rn2(10)) {
			if(rn2(10) && bcsign(otmp))
			    otmp->spe = bcsign(otmp) * rne(3);
			else otmp->spe = rn2(2) ? rne(3) : -rne(3);
		    }
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
	case ROCK_SYM:
		switch (otmp->otyp) {
		    case STATUE:
			/* contains book? */
			otmp->spe = (rn2(dlevel/2 + 10) > 10);
			/* overridden by mkstatue() */
			otmp->corpsenm = rndmonnum();
			otmp->owt = mons[otmp->corpsenm].cwt;
		}
		break;
	default:
		impossible("impossible mkobj %d, sym '%c'.", otmp->otyp, let);
		return (struct obj *)0;
	}
	otmp->owt = weight(otmp);
	return(otmp);
}

void
bless(otmp)
register struct obj *otmp;
{
	otmp->cursed = 0;
	otmp->blessed = 1;
	return;
}

void
curse(otmp)
register struct obj *otmp;
{
	otmp->blessed = 0;
	otmp->cursed = 1;
	return;
}

void
blessorcurse(otmp, chance)
register struct obj *otmp;
register int chance;
{
	if(otmp->blessed || otmp->cursed) return;

	if(!rn2(chance))
	    if(!rn2(2) || Inhell) { /* in hell, don't usually bless items */
		curse(otmp);
	    } else {
		bless(otmp);
	    }
	return;
}

int
bcsign(otmp)
register struct obj *otmp;
{
	return(!!otmp->blessed - !!otmp->cursed);
}

int
letter(c) {
	return(('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

int
weight(obj)
register struct obj *obj;
{
	register int wt = objects[obj->otyp].oc_weight;

	if (Is_container(obj)) {
		struct obj *contents;
		obj->owt = wt;
		for(contents=fcobj; contents; contents=contents->nobj) {
			if (contents->cobj == obj)
				inc_cwt(obj, contents);
		}
		return obj->owt;
	}
	if (obj->otyp == CORPSE && obj->corpsenm > -1)
		return obj->quan * mons[obj->corpsenm].cwt;
	else if (obj->otyp == STATUE && obj->corpsenm > -1)
		return obj->quan * (mons[obj->corpsenm].cwt * 3 / 2);
	return(wt ? wt*obj->quan : (obj->quan + 1)>>1);
}

void
mkgold(num,x,y)
long num;
int x, y;
{
	register struct gold *gold;
	register long amount = (num ? num : 1 + (rnd(dlevel+2) * rnd(30)));

	if(levl[x][y].gmask) {
		gold = g_at(x,y);
		gold->amount += amount;
	} else {
		gold = newgold();
		gold->ngold = fgold;
		gold->gx = x;
		gold->gy = y;
		gold->amount = amount;
		fgold = gold;
		levl[x][y].gmask = 1;
		/* do sth with display? */
	}
	return;
}

struct obj *
mkcorpse_at(ptr, x, y)
register struct permonst *ptr;
int	x, y;
{
	register struct obj *otmp;

	otmp = mksobj_at(CORPSE, x, y);
	if(otmp)  {
		otmp->corpsenm = monsndx(ptr);
		otmp->owt = ptr->cwt;
	}
	return(otmp);
}

struct obj *
mk_tt_corpse(x, y)
register int x, y;
{
	register struct obj *otmp;

	if((otmp = mksobj(CORPSE,FALSE))) {
		otmp->ox = x;
		otmp->oy = y;
		if(otmp = tt_oname(otmp)) {
			otmp->nobj = fobj;
			fobj = otmp;
			levl[x][y].omask = 1;
		}
	}
	return(otmp);
}

struct obj *
mkstatue(ptr, x, y)
register struct permonst *ptr;
int x, y;
{
	register struct obj *otmp;

	if((otmp = mksobj_at(STATUE, x, y))) {

	    if(ptr)	otmp->corpsenm = monsndx(ptr);
	    else	otmp->corpsenm = rndmonnum();
	}
	return(otmp);
}

struct obj *
mk_named_object(objtype, ptr, x, y, nm, lth)
int objtype; /* CORPSE or STATUE */
register struct permonst *ptr;
int x, y;
char * nm;
register int lth;
{
	struct obj *otmp;
	register struct obj *obj2;

	if (lth == 0) {
		switch(objtype) {
			case STATUE: return (mkstatue(ptr, x, y));
			case CORPSE: return (mkcorpse_at(ptr, x, y));
			default: impossible("making named type %d", objtype);
				return mksobj_at(objtype, x, y);
		}
	}

	if((otmp = mksobj(objtype,FALSE))) {
		obj2 = newobj(lth);
		*obj2 = *otmp;
		obj2->corpsenm = monsndx(ptr);
		obj2->owt = ptr->cwt;
		obj2->onamelth = lth;
		Strcpy (ONAME(obj2), nm);
		free( (genericptr_t)otmp);
		obj2->ox = x;
		obj2->oy = y;
		obj2->nobj = fobj;
		fobj = obj2;
		levl[x][y].omask = 1;
		return(obj2);
	} else  return((struct obj *)0);
}

#ifdef MEDUSA
struct obj *
mk_tt_statue(x, y)
register int x, y;
{
	register struct obj *otmp;

	if((otmp = mksobj(STATUE,FALSE))) {
		otmp->ox = x;
		otmp->oy = y;
		if(otmp = tt_oname(otmp)) {
			otmp->nobj = fobj;
			fobj = otmp;
			levl[x][y].omask = 1;
			otmp->spe = 0;
			/* player statues never contain books */
		}
	}
	return(otmp);
}
#endif

boolean
is_flammable(otmp)
register struct obj *otmp;
{
	return((objects[otmp->otyp].oc_material == WOOD ||
			objects[otmp->otyp].oc_material == 0));

}

boolean
is_rustprone(otmp)
register struct obj *otmp;
{
	return(objects[otmp->otyp].oc_material == METAL);
}

void
set_omask(x, y)
register xchar x, y;
{
	levl[x][y].gmask = (g_at(x, y) != (struct gold *)0);
	levl[x][y].omask = (o_at(x, y) != (struct obj *)0);
}
