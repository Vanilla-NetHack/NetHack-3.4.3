/*	SCCS Id: @(#)wield.c	2.1	87/11/09
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	"hack.h"
extern struct obj zeroobj;
extern char *hcolor();
#ifdef KAA
extern boolean unweapon;
#endif

setuwep(obj) register struct obj *obj; {
	setworn(obj, W_WEP);
}

dowield()
{
	register struct obj *wep;
	register int res = 0;

	multi = 0;
#ifdef KAA
	if (cantwield(u.usym)) {
		pline("Don't be ridiculous!");
		return(0);
	}
#endif
	if(!(wep = getobj("#-)", "wield"))) /* nothing */;
	else if(wep == &zeroobj) {
	  if(uwep == 0) {
	    pline("You are already empty handed.");
	  } else if (welded(uwep))
		pline("The %s welded to your hand!",aobjnam(uwep,"are"));
	  	else  {
	  	  setuwep((struct obj *) 0);
	  	  res++;
	  	  pline("You are empty handed.");
	  	}
	} else if(uwep == wep)
		pline("You are already wielding that!");
	else if(welded(uwep))
		pline("The %s welded to your hand!",
			aobjnam(uwep, "are"));
	/* Prevent wielding a cockatrice in pack when not wearing gloves KAA*/
	else if (!uarmg && wep->otyp == DEAD_COCKATRICE) {
		pline("You wield the dead cockatrice in your bare hands.");
		pline("You turn to stone ...");
		killer="dead cockatrice";
		done("died");
	} else if(uarms && wep->otyp == TWO_HANDED_SWORD)
	pline("You cannot wield a two-handed sword and wear a shield.");
	else if(wep->owornmask & (W_ARMOR | W_RING))
		pline("You cannot wield that!");
	else {
		setuwep(wep);
		res++;
		if(welded(uwep))
		    pline("The %s %s to your hand!",
			aobjnam(uwep, "weld"),
			(uwep->quan == 1) ? "itself" : "themselves"); /* a3 */
		else prinv(uwep);
	}
#ifdef KAA
	if(res && uwep)
		unweapon = (uwep->otyp >= BOW || uwep->otyp <= BOOMERANG) ? 
		TRUE : FALSE;
#endif
	return(res);
}

corrode_weapon(){
	if(!uwep || uwep->olet != WEAPON_SYM) return;	/* %% */
	if(uwep->rustfree)
		pline("Your %s not affected.", aobjnam(uwep, "are"));
	else if (uwep->spe > -6) {
		pline("Your %s!", aobjnam(uwep, "corrode"));
		uwep->spe--;
	} else	pline("Your %s quite rusted now.", aobjnam(uwep, "look"));
}

chwepon(otmp,amount)
register struct obj *otmp;
register amount;
{
register char *color = (amount < 0) ? "black" : "green";
register char *time;

	if(Hallucination) color=hcolor();
	if(!uwep || uwep->olet != WEAPON_SYM) {
		strange_feeling(otmp,
			(amount > 0) ? "Your hands twitch."
				     : "Your hands itch.");
		return(0);
	}

	if(uwep->otyp == WORM_TOOTH && amount > 0) {
		uwep->otyp = CRYSKNIFE;
		pline("Your weapon seems sharper now.");
		uwep->cursed = 0;
		return(1);
	}

	if(uwep->otyp == CRYSKNIFE && amount < 0) {
		uwep->otyp = WORM_TOOTH;
		pline("Your weapon looks duller now.");
		return(1);
	}

	/* there is a (soft) upper limit to uwep->spe */
	if(amount > 0 && uwep->spe > 5 && rn2(3)) {
	    pline("Your %s violently %s for a while and then evaporate%s.",
		aobjnam(uwep,"glow"),Hallucination ? hcolor() : "green",
		plur(uwep->quan));

	    while(uwep)		/* let all of them disappear */
				/* note: uwep->quan = 1 is nogood if unpaid */
		useup(uwep);
	    return(1);
	}
	if(!rn2(6)) amount *= 2;
	time = (amount*amount == 1) ? "moment" : "while";
	pline("Your %s %s for a %s.",
		aobjnam(uwep, "glow"), color, time);
	uwep->spe += amount;
	if(amount > 0) uwep->cursed = 0;
	return(1);
}

int
welded(obj) register struct obj *obj;  {
	return(obj && obj == uwep && obj->cursed &&
	       (obj->olet == WEAPON_SYM || obj->otyp == HEAVY_IRON_BALL ||
		obj->otyp == CAN_OPENER || obj->otyp == PICK_AXE));
}
