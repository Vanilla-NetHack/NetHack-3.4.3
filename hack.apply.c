/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.apply.c version 1.0.1 - "The flash awakens %s" (riv05!a3) */

#include	"hack.h"
extern struct monst *bchit();
extern struct obj *addinv();
extern char pl_character[];

doapply() {
register struct obj *obj;
	obj = getobj("(", "use or apply");
	if(!obj) return(0);
	switch(obj->otyp){
	case EXPENSIVE_CAMERA:
		use_camera(obj); break;
	case ICE_BOX:
		use_ice_box(obj); break;
	case MAGIC_WHISTLE:
		if(pl_character[0] == 'W' || u.ulevel > 9) {
			use_magic_whistle(obj);
			break;
		}
		/* fall into next case */
	case WHISTLE:
		use_whistle(obj); break;
	default:
		pline("Sorry, I don't know how to use that.");
		return(0);
	}
 return(1);
}

/* ARGSUSED */
use_camera(obj) /* register */ struct obj *obj; {
register struct monst *mtmp;
	if(!getdir()){
		flags.move = multi = 0;
		return;
	}
	if(mtmp = bchit(u.dx, u.dy, COLNO, '!')) {
		if(mtmp->msleep){
			mtmp->msleep = 0;
			pline("The flash awakens %s.", monnam(mtmp));
		} else
		if(mtmp->data->mlet != 'y')
		if(mtmp->mcansee || mtmp->mblinded){
			register int tmp = dist(mtmp->mx,mtmp->my);
			register int tmp2;
			/* if(cansee(mtmp->mx,mtmp->my)) */
			  pline("%s is blinded by the flash!",Monnam(mtmp));
			setmangry(mtmp);
			if(tmp < 9 && !mtmp->isshk && !rn2(4))
				mtmp->mflee = 1;
			if(tmp < 3) mtmp->mcansee  = mtmp->mblinded = 0;
			else {
				tmp2 = mtmp->mblinded;
				tmp2 += rnd(1 + 50/tmp);
				if(tmp2 > 127) tmp2 = 127;
				mtmp->mblinded = tmp2;
				mtmp->mcansee = 0;
			}
		}
	}
}

struct obj *current_ice_box;	/* a local variable of use_ice_box, to be
				used by its local procedures in/ck_ice_box */
in_ice_box(obj) register struct obj *obj; {
	if(obj == current_ice_box ||
		(Punished && (obj == uball || obj == uchain))){
		pline("You must be kidding.");
		return(0);
	}
	if(obj->owornmask & (W_ARMOR | W_RING)) {
		pline("You cannot refrigerate something you are wearing.");
		return(0);
	}
	if(obj->owt + current_ice_box->owt > 70) {
		pline("It won't fit.");
		return(1);	/* be careful! */
	}
	if(obj == uwep) {
		if(uwep->cursed) {
			pline("Your weapon is welded to your hand!");
			return(0);
		}
 setuwep((struct obj *) 0);
	}
	current_ice_box->owt += obj->owt;
	freeinv(obj);
	obj->o_cnt_id = current_ice_box->o_id;
	obj->nobj = fcobj;
	fcobj = obj;
	obj->age = moves - obj->age;	/* actual age */
	return(1);
}

ck_ice_box(obj) register struct obj *obj; {
	return(obj->o_cnt_id == current_ice_box->o_id);
}

out_ice_box(obj) register struct obj *obj; {
register struct obj *otmp;
	if(obj == fcobj) fcobj = fcobj->nobj;
	else {
		for(otmp = fcobj; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp->nobj) panic("out_ice_box");
		otmp->nobj = obj->nobj;
	}
	current_ice_box->owt -= obj->owt;
	obj->age = moves - obj->age;	/* simulated point of time */
	(void) addinv(obj);
}

use_ice_box(obj) register struct obj *obj; {
register int cnt = 0;
register struct obj *otmp;
	current_ice_box = obj;	/* for use by in/out_ice_box */
	for(otmp = fcobj; otmp; otmp = otmp->nobj)
		if(otmp->o_cnt_id == obj->o_id)
			cnt++;
	if(!cnt) pline("Your ice-box is empty.");
	else {
	    pline("Do you want to take something out of the ice-box? [yn] ");
	    if(readchar() == 'y')
		if(askchain(fcobj, (char *) 0, 0, out_ice_box, ck_ice_box, 0))
		    return;
		pline("That was all. Do you wish to put something in? [yn] ");
		if(readchar() != 'y') return;
	}
	/* call getobj: 0: allow cnt; #: allow all types; %: expect food */
	otmp = getobj("0#%", "put in");
	if(!otmp || !in_ice_box(otmp))
		flags.move = multi = 0;
}

struct monst *
bchit(ddx,ddy,range,sym) register int ddx,ddy,range; char sym; {
	register struct monst *mtmp = (struct monst *) 0;
	register int bchx = u.ux, bchy = u.uy;

	if(sym) Tmp_at(-1, sym);	/* open call */
	while(range--) {
		bchx += ddx;
		bchy += ddy;
		if(mtmp = m_at(bchx,bchy))
			break;
		if(levl[bchx][bchy].typ < CORR) {
			bchx -= ddx;
			bchy -= ddy;
			break;
		}
 if(sym) Tmp_at(bchx, bchy);
	}
	if(sym) Tmp_at(-1, -1);
	return(mtmp);
}

#include	"def.edog.h"
/* ARGSUSED */
use_whistle(obj) struct obj *obj; {
register struct monst *mtmp = fmon;
	pline("You produce a high whistling sound.");
	while(mtmp) {
		if(dist(mtmp->mx,mtmp->my) < u.ulevel*10) {
			if(mtmp->msleep)
				mtmp->msleep = 0;
			if(mtmp->mtame)
				EDOG(mtmp)->whistletime = moves;
		}
 mtmp = mtmp->nmon;
	}
}

/* ARGSUSED */
use_magic_whistle(obj) struct obj *obj; {
register struct monst *mtmp = fmon;
	pline("You produce a strange whistling sound.");
	while(mtmp) {
		if(mtmp->mtame) mnexto(mtmp);
		mtmp = mtmp->nmon;
	}
}
