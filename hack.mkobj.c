/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#ifdef MKLEV
#include "mklev.h"
#else
#include "hack.h"
#endif MKLEV

#include "hack.onames.h"

char mkobjstr[] = "))[[!!!!????%%%%/=**))[[!!!!????%%%%/=**(";
struct obj *mkobj(), *mksobj();

mkobj_at(let,x,y)
register let,x,y;
{
	register struct obj *otmp = mkobj(let);
	otmp->ox = x;
	otmp->oy = y;
	otmp->nobj = fobj;
	fobj = otmp;
}

#ifndef MKLEV
mksobj_at(let,otyp,x,y)
register let,otyp,x,y;
{
	register struct obj *otmp = mksobj(let, otyp);
	otmp->ox = x;
	otmp->oy = y;
	otmp->nobj = fobj;
	fobj = otmp;
}
#endif MKLEV

struct obj *
mkobj(let) {
	if(!let) let = mkobjstr[rn2(sizeof(mkobjstr) - 1)];
	return(mksobj(let, letter(let) ? CORPSE : probtype(let)));
}
	

struct obj zeroobj;

struct obj *
mksobj(let, otyp) {
	register struct obj *otmp;

	otmp = newobj(0);
	*otmp = zeroobj;
#ifdef MKLEV
	otmp->age = 0;
	otmp->o_id = 0;
#else
	otmp->age = moves;
	otmp->o_id = flags.ident++;
#endif MKLEV
	otmp->quan = 1;
	if(letter(let)){
		otmp->olet = FOOD_SYM;
		otmp->otyp = CORPSE + ((let > 'Z') ? (let-'a'+'Z'-'@'+1) :
				(let-'@'));
		otmp->spe = let;
		otmp->known = 1;
		otmp->owt = weight(otmp);
		return(otmp);
	}
	otmp->olet = let;
	otmp->otyp = otyp;
	otmp->dknown = index("/=!?*", let) ? 0 : 1;
	switch(let) {
	case WEAPON_SYM:
		otmp->quan = (otmp->otyp <= ROCK) ? rn1(6,6) : 1;
		if(!rn2(11)) otmp->spe = rnd(3);
		else if(!rn2(10)) {
			otmp->cursed = 1;
			otmp->spe = -rnd(3);
		}
		break;
	case FOOD_SYM:
	case GEM_SYM:
		otmp->quan = rn2(6) ? 1 : 2;
	case TOOL_SYM:
	case CHAIN_SYM:
	case BALL_SYM:
	case ROCK_SYM:
	case POTION_SYM:
	case SCROLL_SYM:
	case AMULET_SYM:
		break;
	case ARMOR_SYM:
		if(!rn2(8)) otmp->cursed = 1;
		if(!rn2(10)) otmp->spe = rnd(3);
		else if(!rn2(9)) {
			otmp->spe = -rnd(3);
			otmp->cursed = 1;
		}
		otmp->spe += 10 - objects[otmp->otyp].a_ac;
		break;
	case WAND_SYM:
		if(otmp->otyp == WAN_WISHING) otmp->spe = 3; else
		otmp->spe = rn1(5,
			(objects[otmp->otyp].bits & NODIR) ? 11 : 4);
		break;
	case RING_SYM:
		if(objects[otmp->otyp].bits & SPEC) {
			if(!rn2(3)) {
				otmp->cursed = 1;
				otmp->spe = -rnd(2);
			} else otmp->spe = rnd(2);
		} else if(otmp->otyp == RIN_TELEPORTATION ||
			  otmp->otyp == RIN_AGGRAVATE_MONSTER ||
			  otmp->otyp == RIN_HUNGER || !rn2(9))
			otmp->cursed = 1;
		break;
	default:
		panic("impossible mkobj");
	}
	otmp->owt = weight(otmp);
	return(otmp);
}

letter(c) {
	return(('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
}

weight(obj)
register struct obj *obj;
{
register int wt = objects[obj->otyp].oc_weight;
	return(wt ? wt*obj->quan : (obj->quan + 1)/2);
}

mkgold(num,x,y)
register num;
{
	register struct gen *gtmp;
	register int amount = num ? num : 1 + (rnd(dlevel+2) * rnd(30));

	if(gtmp = g_at(x,y,fgold))
		gtmp->gflag += amount;
	else {
		gtmp = newgen();
		gtmp->ngen = fgold;
		gtmp->gx = x;
		gtmp->gy = y;
		gtmp->gflag = amount;
		fgold = gtmp;
#ifdef MKLEV
		levl[x][y].scrsym = '$';
#endif MKLEV
	}
}
