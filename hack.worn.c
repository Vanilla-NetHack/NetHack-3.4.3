/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"

struct worn {
	long w_mask;
	struct obj **w_obj;
} worn[] = {
	{ W_ARM, &uarm },
	{ W_ARM2, &uarm2 },
	{ W_ARMH, &uarmh },
	{ W_ARMS, &uarms },
	{ W_ARMG, &uarmg },
	{ W_RINGL, &uleft },
	{ W_RINGR, &uright },
	{ W_WEP, &uwep },
	{ W_BALL, &uball },
	{ W_CHAIN, &uchain },
	{ 0, 0 }
};

setworn(obj, mask)
register struct obj *obj;
long mask;
{
	register struct worn *wp;
	register struct obj *oobj;

	for(wp = worn; wp->w_mask; wp++) if(wp->w_mask & mask) {
		oobj = *(wp->w_obj);
		if(oobj && !(oobj->owornmask & wp->w_mask)){
			pline("Setworn: mask = %d.", wp->w_mask);
			impossible();
		}
		if(oobj) oobj->owornmask &= ~wp->w_mask;
		if(obj && oobj && wp->w_mask == W_ARM){
			if(uarm2) {
				pline("Setworn: uarm2 set?");
				impossible();
			} else
 setworn(uarm, W_ARM2);
		}
		*(wp->w_obj) = obj;
		if(obj) obj->owornmask |= wp->w_mask;
	}
	if(uarm2 && !uarm) {
		uarm = uarm2;
		uarm2 = 0;
		uarm->owornmask ^= (W_ARM | W_ARM2);
	}
}

/* called e.g. when obj is destroyed */
setnotworn(obj) register struct obj *obj; {
	register struct worn *wp;

	for(wp = worn; wp->w_mask; wp++)
		if(obj == *(wp->w_obj)) {
			*(wp->w_obj) = 0;
			obj->owornmask &= ~wp->w_mask;
		}
	if(uarm2 && !uarm) {
		uarm = uarm2;
		uarm2 = 0;
		uarm->owornmask ^= (W_ARM | W_ARM2);
	}
}
