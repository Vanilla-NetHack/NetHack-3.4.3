/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
#define	SIZE(x)	(sizeof(x) / sizeof(x[0]))

timeout(){
register struct prop *upp;
	for(upp = u.uprops; upp < u.uprops+SIZE(u.uprops); upp++)
	    if((upp->p_flgs & TIMEOUT) && !--upp->p_flgs) {
		if(upp->p_tofn) (*upp->p_tofn)();
		else switch(upp - u.uprops){
		case SICK:
			pline("You die because of food poisoning");
			killer = u.usick_cause;
			done("died");
			/* NOTREACHED */
		case FAST:
			pline("You feel yourself slowing down");
			break;
		case CONFUSION:
			pline("You feel less confused now");
			break;
		case BLIND:
			pline("You can see again");
			setsee();
			break;
		case INVIS:
			on_scr(u.ux,u.uy);
			pline("You are no longer invisible.");
		}
	}
}
