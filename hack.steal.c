/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"

stealgold(mtmp)  register struct monst *mtmp; {
register struct gen *gold = g_at(u.ux, u.uy, fgold);
register tmp;
	if(gold && ( !u.ugold || gold->gflag > u.ugold || !rn2(5))) {
		mtmp->mgold += gold->gflag;
		freegold(gold);
		if(Invis) newsym(u.ux, u.uy);
		pline("%s quickly snatches some gold from between your feet!",
			Monnam(mtmp));
		if(!u.ugold || !rn2(5)) {
			rloc(mtmp);
			mtmp->mflee = 1;
		}
	} else if(u.ugold) {
		u.ugold -= (tmp = somegold());
		pline("Your purse feels lighter.");
		mtmp->mgold += tmp;
		rloc(mtmp);
		mtmp->mflee = 1;
		flags.botl = 1;
	}
}

somegold(){
	return( (u.ugold < 100) ? u.ugold :
		(u.ugold > 10000) ? rnd(10000) : rnd((int) u.ugold) );
}

/* steal armor after he finishes taking it off */
unsigned stealoid;		/* object to be stolen */
unsigned stealmid;		/* monster doing the stealing */
stealarm(){
	register struct monst *mtmp;
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
	  if(otmp->o_id == stealoid) {
	    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	      if(mtmp->m_id == stealmid) {
		if(dist(mtmp->mx,mtmp->my) < 3) {
		  freeinv(otmp);
		  pline("%s steals %s!", Monnam(mtmp), doname(otmp));
		  mpickobj(mtmp,otmp);
		  mtmp->mflee = 1;
		  rloc(mtmp);
		}
		break;
	      }
	    break;
	  }
 stealoid = 0;
}

/* returns 1 when something was stolen */
/* (or at least, when N should flee now) */
/* avoid stealing the object stealoid */
steal(mtmp)
struct monst *mtmp;
{
	register struct obj *otmp;
	register tmp;
	register named = 0;

	if(!invent){
	    if(Blind)
	      pline("Somebody tries to rob you, but finds nothing to steal.");
	    else
	      pline("%s tries to rob you, but she finds nothing to steal!",
		Monnam(mtmp));
	    return(1);	/* let her flee */
	}
	tmp = 0;
	for(otmp = invent; otmp; otmp = otmp->nobj)
		tmp += ((otmp->owornmask & (W_ARMOR | W_RING)) ? 5 : 1);
	tmp = rn2(tmp);
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if((tmp -= ((otmp->owornmask & (W_ARMOR | W_RING)) ? 5 : 1))
			< 0) break;
	if(!otmp) panic("Steal fails!");
	if(otmp->o_id == stealoid) return(0);
	if((otmp->owornmask & (W_ARMOR | W_RING))){
		switch(otmp->olet) {
		case RING_SYM:
			ringoff(otmp);
			break;
		case ARMOR_SYM:
			if(multi < 0 || otmp == uarms){
			  setworn((struct obj *) 0, otmp->owornmask & W_ARMOR);
			  break;
			}
		{ int curssv = otmp->cursed;
			otmp->cursed = 0;
			pline("%s seduces you and %s off your %s.",
				Amonnam(mtmp, Blind ? "gentle" : "beautiful"),
				otmp->cursed ? "helps you to take"
					    : "you start taking",
				(otmp == uarmg) ? "gloves" :
				(otmp == uarmh) ? "helmet" : "armor");
			named++;
			(void) armoroff(otmp);
			otmp->cursed = curssv;
			if(multi < 0){
				extern char *nomovemsg;
				extern int (*afternmv)();
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
			impossible();
		}
	}
	else if(otmp == uwep)
		setuwep((struct obj *) 0);
	if(otmp->olet == CHAIN_SYM) {
		pline("How come you are carrying that chain?");
		impossible();
	}
	if(Punished && otmp == uball){
		Punished = 0;
		freeobj(uchain);
		free((char *) uchain);
		uchain = (struct obj *) 0;
		uball->spe = 0;
		uball = (struct obj *) 0;	/* superfluous */
	}
	freeinv(otmp);
	pline("%s stole %s.", named ? "She" : Monnam(mtmp), doname(otmp));
	mpickobj(mtmp,otmp);
	return((multi < 0) ? 0 : 1);
}

mpickobj(mtmp,otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	otmp->nobj = mtmp->minvent;
	mtmp->minvent = otmp;
}

/* release the objects the killed animal has stolen */
relobj(mtmp,show)
register struct monst *mtmp;
register show;
{
	register struct obj *otmp, *otmp2;

	for(otmp = mtmp->minvent; otmp; otmp = otmp2){
		otmp->ox = mtmp->mx;
		otmp->oy = mtmp->my;
		otmp2 = otmp->nobj;
		otmp->nobj = fobj;
		fobj = otmp;
		stackobj(fobj);
		if(show & cansee(mtmp->mx,mtmp->my))
			atl(otmp->ox,otmp->oy,otmp->olet);
	}
	mtmp->minvent = (struct obj *) 0;
	if(mtmp->mgold || mtmp->data->mlet == 'L') {
		register int tmp;

		tmp = (mtmp->mgold > 10000) ? 10000 : mtmp->mgold;
		mkgold( tmp + d(dlevel,30), mtmp->mx, mtmp->my);
		if(show & cansee(mtmp->mx,mtmp->my))
			atl(mtmp->mx,mtmp->my,'$');
	}
}
