/*	SCCS Id: @(#)dothrow.c	2.1	87/11/01
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* dothrow.c - version 1.0 */

/* Contains code for 't' (throw) */

#include "hack.h"

extern struct obj *splitobj(), *addinv();
extern boolean hmon();
extern struct monst youmonst;
extern char *Doname();
#ifdef KAA
extern char *xname();
#endif

struct monst *bhit(), *boomhit();
dothrow()
{
	register struct obj *obj;

	obj = getobj("#)", "throw");   /* it is also possible to throw food */
				       /* (or jewels, or iron balls ... ) */
	if(!obj || !getdir(1))	       /* ask "in what direction?" */
		return(0);
	if(obj->owornmask & (W_ARMOR | W_RING)){
		pline("You can't throw something you are wearing.");
		return(0);
	}
#ifdef KAA
	if(obj->otyp == ENORMOUS_ROCK && u.usym != '9') {
		pline("It's too heavy.");
		return(1);
	}
	if(!u.dx && !u.dy && !u.dz) {
		pline("You cannot throw an object at yourself.");
		return(0);
	}
#endif
	u_wipe_engr(2);

	if(obj == uwep){
		if(obj->cursed){
			pline("Your weapon is welded to your hand.");
			return(1);
		}
		if(obj->quan > 1)
			setuwep(splitobj(obj, 1));
		else
			setuwep((struct obj *) 0);
	}
	else if(obj->quan > 1)
		(void) splitobj(obj, 1);
	freeinv(obj);
	return(throwit(obj));
}

throwit(obj)
	register struct obj *obj;
{
	register struct monst *mon;

	if(u.uswallow) {
		mon = u.ustuck;
		bhitpos.x = mon->mx;
		bhitpos.y = mon->my;
	} else if(u.dz) {
	  if(u.dz < 0) {
	    pline("%s hits the ceiling, then falls back on top of your head.",
		Doname(obj));		/* note: obj->quan == 1 */
	    if(obj->olet == POTION_SYM)
		potionhit(&youmonst, obj);
	    else {
		if(uarmh) pline("Fortunately, you are wearing a helmet!");
		losehp(uarmh ? 1 : rnd((int)(obj->owt)), "falling object");
		dropy(obj);
	    }
	  } else hitfloor(obj);
	  return(1);

	} else if(obj->otyp == BOOMERANG) {
		mon = boomhit(u.dx, u.dy);
		if(mon == &youmonst) {		/* the thing was caught */
			(void) addinv(obj);
			return(1);
		}
	} else {
		if(obj->otyp == PICK_AXE && shkcatch(obj))
		    return(1);

		mon = bhit(u.dx, u.dy, (obj->otyp == ICE_BOX) ? 1 :
			(!Punished || obj != uball) ? 8 : !u.ustuck ? 5 : 1,
			obj->olet,
			(int (*)()) 0, (int (*)()) 0, obj);
	}
	if(mon) {
		/* awake monster if sleeping */
		wakeup(mon);
		if(thitmonst(mon, obj)) return(1);
	}
	if(!u.uswallow)  {
		/* the code following might become part of dropy() */
		if(obj->otyp == CRYSKNIFE)
			obj->otyp = WORM_TOOTH;
		obj->ox = bhitpos.x;
		obj->oy = bhitpos.y;
		obj->nobj = fobj;
		fobj = obj;
		/* prevent him from throwing articles to the exit and escaping */
		/* subfrombill(obj); */
		stackobj(obj);
		if(Punished && obj == uball &&
			(bhitpos.x != u.ux || bhitpos.y != u.uy)){
			freeobj(uchain);
			unpobj(uchain);
			if(u.utrap){
				if(u.utraptype == TT_PIT)
					pline("The ball pulls you out of the pit!");
#ifdef SPIDERS
				else if(u.utraptype == TT_WEB)  {
					pline("The ball pulls you out of the web!");
					pline("The web is destroyed!");
					deltrap(t_at(u.ux,u.uy));
				}
#endif
				else  {
				register long side =
					rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
				pline("The ball pulls you out of the bear trap.");
				pline("Your %s leg is severely damaged.",
					(side == LEFT_SIDE) ? "left" : "right");                                set_wounded_legs(side, 500+rn2(1000));
				losehp(2, "thrown ball");
				}
				u.utrap = 0;
			}        
			unsee();
			uchain->nobj = fobj;
			fobj = uchain;
			u.ux = uchain->ox = bhitpos.x - u.dx;
			u.uy = uchain->oy = bhitpos.y - u.dy;
			setsee();
			(void) inshop();
		}
		if(cansee(bhitpos.x, bhitpos.y)) prl(bhitpos.x,bhitpos.y);
	}  else  
		mpickobj(u.ustuck,obj);
	return(1);
}

hitfloor(obj)
	register struct obj *obj;
{
	pline("%s hits the floor.", Doname(obj));
	if(obj->otyp == EXPENSIVE_CAMERA) {
		pline("It is shattered in a thousand pieces!");
		obfree(obj, Null(obj));
#ifdef RPH
	} else	if(obj->otyp == MIRROR) {
	    	pline ("The mirror shatters.  That's seven years bad luck!");
		obfree(obj, Null(obj));
		u.uluck -= 2;
		if ((int)u.uluck < LUCKMIN) u.uluck = LUCKMIN;
#endif	
	} else	if(obj->otyp == EGG) {
		pline("\"Splash!\"");
		obfree(obj, Null(obj));
#ifdef KAA
	} else	if(obj->otyp == CREAM_PIE) {
		pline("What a mess!");
		obfree(obj, Null(obj));
#endif
	} else	if(obj->olet == POTION_SYM) {
		pline("The flask breaks, and you smell a peculiar odor ...");
		potionbreathe(obj);
		obfree(obj, Null(obj));
	} else
		dropy(obj);
}

thitmonst(mon, obj)
	register struct monst *mon;
	register struct obj   *obj;
{
	register int	tmp;

	if(obj->olet == WEAPON_SYM) {
		tmp = -1+u.ulevel+mon->data->ac+abon();
		if(obj->otyp < DART) {
			if(!uwep ||
			    uwep->otyp != obj->otyp+(BOW-ARROW))
				tmp -= 4;
			else {
				tmp += uwep->spe;
			}
		} else
		if(obj->otyp == BOOMERANG) tmp += 4;
		tmp += obj->spe;
		if(u.uswallow || tmp >= rnd(20)) {
			if(hmon(mon,obj,1) == TRUE){
			  /* mon still alive */
#ifndef NOWORM
			  cutworm(mon,bhitpos.x,bhitpos.y,obj->otyp);
#endif
			} else mon = 0;
			/* weapons thrown disappear sometimes */
			if(obj->otyp < BOOMERANG && rn2(3)) {
				/* check bill; free */
				obfree(obj, (struct obj *) 0);
				return(1);
			}
		} else miss(objects[obj->otyp].oc_name, mon);
	} else if(obj->otyp == HEAVY_IRON_BALL) {
		tmp = -1+u.ulevel+mon->data->ac+abon();
		if(!Punished || obj != uball) tmp += 2;
		if(u.utrap) tmp -= 2;
		if(u.uswallow || tmp >= rnd(20)) {
			if(hmon(mon,obj,1) == FALSE)
				mon = 0;	/* he died */
		} else miss("iron ball", mon);
#ifdef KAA
	} else if (obj->otyp == ENORMOUS_ROCK) {
		tmp = 15+mon->data->ac;  /* Very likely to hit! */
		if (hmon(mon, obj, 1) == FALSE)	mon=0;
		else miss("enormous rock",mon);
	} else if(obj->otyp == CREAM_PIE &&
		(u.ulevel > rn2(10)) || u.ustuck == mon) {
		pline("The cream pie splashes over %s%s!",monnam(mon),
			index("aEfgy",mon->data->mlet) ? "" : "'s face");
		obfree(obj, (struct obj *) 0);
		if(mon->msleep) mon->msleep = 0;
		setmangry(mon);
		mon->mcansee = 0;
		mon->mblinded += rnd(25);
		if (mon->mblinded <= 0) mon->mblinded = 127;
		return(1);
#endif
	} else if(obj->olet == POTION_SYM && u.ulevel > rn2(15)) {
		potionhit(mon, obj);
		return(1);
	} else {
		pline("The %s misses %s.",xname(obj), 
		cansee(bhitpos.x,bhitpos.y) ? monnam(mon) : "it");

		if(obj->olet == FOOD_SYM && mon->data->mlet == 'd')
			if(tamedog(mon,obj)) return(1);
		if(obj->olet == GEM_SYM && mon->data->mlet == 'u' &&
			!mon->mtame){
			char buf[BUFSZ];
			char *nogood = " is not interested in your junk.";
			char *addluck = " graciously accepts your gift.";
	
			strcpy(buf,Monnam(mon));
 
			if(obj->dknown &&
			   objects[obj->otyp].oc_name_known)  {
				if(objects[obj->otyp].g_val > 0)  {
					u.uluck += 5;
					strcat(buf,addluck);
				}  else
					strcat(buf,nogood);
			}  else  {  /* value unknown to @ */
				u.uluck++;
				strcat(buf,addluck);
			}
			if(u.uluck > LUCKMAX)   /* dan@ut-ngp */
				u.uluck = LUCKMAX;
			pline(buf);
			mpickobj(mon, obj);
			rloc(mon);
			return(1);
		}
	}
	return(0);
}
