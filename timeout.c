/*	SCCS Id: @(#)timeout.c	2.0	87/09/18
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	"hack.h"

timeout(){
register struct prop *upp;
#ifdef KAA
register struct monst *mtmp;
#endif
	if(Stoned) stoned_dialogue();
#ifdef KAA
	if(u.mtimedone) if (!--u.mtimedone) rehumanize();
# ifdef KOPS
	if(u.ucreamed > 0) u.ucreamed--;
# endif
#endif
	for(upp = u.uprops; upp < u.uprops+SIZE(u.uprops); upp++)
	    if((upp->p_flgs & TIMEOUT) && !(--upp->p_flgs & TIMEOUT)) {
		if(upp->p_tofn) (*upp->p_tofn)();
		else switch(upp - u.uprops){
		case STONED:
			killer = "cockatrice";
			done("died");
			break;
		case SICK:
			pline("You die because of food poisoning.");
			killer = u.usick_cause;
			done("died");
			break;
		case FAST:
			pline("You feel yourself slowing down.");
			break;
		case CONFUSION:
			if (Hallucination) pline("You feel less trippy now.");
			else
				pline("You feel less confused now.");
			break;
		case BLINDED:
			if (Hallucination) pline("Oh like wow! What a rush.");
			else		   pline("You can see again.");
			setsee();
			break;
		case INVIS:
			on_scr(u.ux,u.uy);
			if (!See_invisible)
				pline("You are no longer invisible.");
			break;
		case WOUNDED_LEGS:
			heal_legs();
			break;
#ifdef KAA
		case HALLUCINATION:
			pline("Everything looks SO boring now.");
			setsee();
			for (mtmp=fmon; mtmp; mtmp=mtmp->nmon)
				if ((Blind && Telepat) || canseemon(mtmp))
					atl(mtmp->mx, mtmp->my, (!mtmp->mappearance || 
					Protection_from_shape_changers) 
					? mtmp->data->mlet : mtmp->mappearance);
			break;
#endif
		}
	}
}

/* He is being petrified - dialogue by inmet!tower */
char *stoned_texts[] = {
	"You are slowing down.",		/* 5 */
	"Your limbs are stiffening.",		/* 4 */
	"Your limbs have turned to stone.",	/* 3 */
	"You have turned to stone.",		/* 2 */
	"You are a statue."			/* 1 */
};

stoned_dialogue()
{
	register long i = (Stoned & TIMEOUT);

	if(i > 0 && i <= SIZE(stoned_texts))
		pline(stoned_texts[SIZE(stoned_texts) - i]);
	if(i == 5)
		Fast = 0;
	if(i == 3)
		nomul(-3);
}
