/*	SCCS Id: @(#)steal.c	3.0	88/07/06
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static char *
equipname(otmp)

	register struct obj *otmp;
{

	return (
#ifdef SHIRT
		(otmp == uarmu) ? "shirt" :
#endif
		(otmp == uarmf) ? "boots" :
		(otmp == uarms) ? "shield" :
		(otmp == uarmg) ? "gloves" :
		(otmp == uarmc) ? "cloak" :
		(otmp == uarmh) ? "helmet" : "armor");
}

long		/* actually returns something that fits in an int */
somegold(){
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
	register struct gold *gold = g_at(u.ux, u.uy);
	register long tmp;
	if(gold && ( !u.ugold || gold->amount > u.ugold || !rn2(5))) {
		mtmp->mgold += gold->amount;
		freegold(gold);
		if(Invisible) newsym(u.ux, u.uy);
		pline("%s quickly snatches some gold from between your %s!",
			Monnam(mtmp), makeplural(body_part(FOOT)));
		if(!u.ugold || !rn2(5)) {
			rloc(mtmp);
			mtmp->mflee = 1;
		}
	} else if(u.ugold) {
		u.ugold -= (tmp = somegold());
		Your("purse feels lighter.");
		mtmp->mgold += tmp;
		rloc(mtmp);
		mtmp->mflee = 1;
		flags.botl = 1;
	}
}

/* steal armor after he finishes taking it off */
unsigned int stealoid;		/* object to be stolen */
unsigned int stealmid;		/* monster doing the stealing */

#ifndef OVERLAY
static 
#endif
int
stealarm(){
	register struct monst *mtmp;
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
	  if(otmp->o_id == stealoid) {
	    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	      if(mtmp->m_id == stealmid) {
		  freeinv(otmp);
		  pline("%s steals %s!", Monnam(mtmp), doname(otmp));
		  mpickobj(mtmp,otmp);
		  mtmp->mflee = 1;
		  rloc(mtmp);
		break;
	      }
	    break;
	  }
	return stealoid = 0;
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

	if(!invent){
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
	for(otmp = invent; otmp; otmp = otmp->nobj) if(!uarm || otmp != uarmc)
	    tmp += ((otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) ? 5 : 1);
	tmp = rn2(tmp);
	for(otmp = invent; otmp; otmp = otmp->nobj) if(!uarm || otmp != uarmc)
  	    if((tmp -= ((otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) ? 5 : 1))
			< 0) break;
	if(!otmp) {
		impossible("Steal fails!");
		return(0);
	}
	/* can't steal gloves while wielding - so steal the wielded item. */
	if (otmp == uarmg && uwep)
	    otmp = uwep;
	/* can't steal armor while wearing cloak - so steal the cloak. */
	else if(otmp == uarm && uarmc) otmp = uarmc;
#ifdef SHIRT
	else if(otmp == uarmu && uarmc) otmp = uarmc;
	else if(otmp == uarmu && uarm) otmp = uarm;
#endif
gotobj:
	if(otmp->o_id == stealoid) return(0);

#ifdef WALKIES
	if(otmp->otyp == LEASH && otmp->leashmon) o_unleash(otmp);
#endif

	if((otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))){
		switch(otmp->olet) {
		case TOOL_SYM:
			Blindf_off(otmp);
			break;
		case AMULET_SYM:
			Amulet_off();
			break;
		case RING_SYM:
			Ring_gone(otmp);
			break;
		case ARMOR_SYM:
			/* Stop putting on armor which has been stolen. */
			if (donning(otmp))
			    afternmv = 0;
			if(multi < 0 || otmp == uarms){
			  if (otmp == uarm)  (void) Armor_off();
			  else if (otmp == uarmc) (void) Cloak_off();
			  else if (otmp == uarmf) (void) Boots_off();
			  else if (otmp == uarmg) (void) Gloves_off();
			  else if (otmp == uarmh) (void) Helmet_off();
			  else if (otmp == uarms) (void) Shield_off();
			  else setworn((struct obj *)0, otmp->owornmask & W_ARMOR);
			  break;
			}
		{ int curssv = otmp->cursed;
			otmp->cursed = 0;
			stop_occupation();
			if(flags.female)
			    pline("%s charms you.  You gladly %s your %s.",
				  Monnam(mtmp),
				  curssv ? "let her take" : "hand over",
				  equipname(otmp));
			else
			    pline("%s seduces you and %s off your %s.",
				  Amonnam(mtmp, Blind ? "gentle" : "beautiful"),
				  curssv ? "helps you to take" : "you start taking",
				  equipname(otmp));
			named++;
			/* the following is to set multi for later on */
			(void) nomul(-objects[otmp->otyp].oc_delay);

			if (otmp == uarm)  (void) Armor_off();
			else if (otmp == uarmc) (void) Cloak_off();
			else if (otmp == uarmf) (void) Boots_off();
			else if (otmp == uarmg) (void) Gloves_off();
			else if (otmp == uarmh) (void) Helmet_off();
			else if (otmp == uarms) (void) Shield_off();
			else setworn((struct obj *)0, otmp->owornmask & W_ARMOR);
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
	if (otmp->otyp == CORPSE && otmp->corpsenm == PM_COCKATRICE
	    && !resists_ston(mtmp->data)) {
	    pline("%s turns to stone.", Monnam(mtmp));
	    stoned = TRUE;
	    xkilled(mtmp, 0);
	    return -1;
	}
	return((multi < 0) ? 0 : 1);
}

void
mpickobj(mtmp,otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	otmp->nobj = mtmp->minvent;
	mtmp->minvent = otmp;
}

void
stealamulet(mtmp)
register struct monst *mtmp;
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->otyp == AMULET_OF_YENDOR) {
		/* might be an imitation one */
		setnotworn(otmp);
		freeinv(otmp);
		mpickobj(mtmp,otmp);
		pline("%s stole %s!", Monnam(mtmp), doname(otmp));
		rloc(mtmp);
		return;
	    }
	}
}

/* release the objects the killed animal has stolen */
void
relobj(mtmp,show)
register struct monst *mtmp;
register int show;
{
	register struct obj *otmp, *otmp2;

	for(otmp = mtmp->minvent; otmp; otmp = otmp2){
		place_object(otmp, mtmp->mx, mtmp->my);
		otmp2 = otmp->nobj;
		otmp->nobj = fobj;
		if (flooreffects(otmp,mtmp->mx,mtmp->my)) continue;
		fobj = otmp;
		stackobj(fobj);
		if(show & cansee(mtmp->mx,mtmp->my))
			atl(otmp->ox,otmp->oy,Hallucination?rndobjsym() : otmp->olet);
	}
	mtmp->minvent = (struct obj *) 0;
	if(mtmp->mgold || mtmp->data->mlet == S_LEPRECHAUN) {
		register long tmp;

		tmp = (mtmp->mgold > 10000) ? 10000 : mtmp->mgold;
		mkgold((long)(tmp + d(dlevel,30)), mtmp->mx, mtmp->my);
		if(show & cansee(mtmp->mx,mtmp->my))
			atl(mtmp->mx,mtmp->my, Hallucination ? rndobjsym() : GOLD_SYM);
	}
}
