/*	SCCS Id: @(#)pray.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* pray.c - version 1.0 */

#include "hack.h"

extern char *nomovemsg;
/*
#ifdef KAA
extern char *xname();
#endif
*/
dopray() {		/* M. Stephenson (1.0.3b) */
#ifdef PRAYERS
	if (u.ublesscnt > 0)  {		/* disturbing the gods too much */

		u.ublesscnt += 200;
		u.uluck -= 3;
		if (u.uluck < LUCKMIN)  u.uluck = LUCKMIN;
		if (u.ugangr++)	angrygods();
		else {			/* exactly one warning */
			pline("A voice booms out: You have angered us,");
			pline("Disturb us again at your own risk!");
		}
	} else  if (u.uluck < 0) angrygods();	/* a bad boy/girl */
	else	pleased();	    		/* or a good boy/girl */
#endif
	nomovemsg = "You finished your prayer.";
	nomul(-3);
	return(1);
}

#ifdef PRAYERS
angrygods() {
	register int	tmp;

	pline ("You get the felling the gods are angry...");
	tmp = u.ugangr + (u.uluck > 0) ? u.uluck : -u.uluck;
	switch (tmp ? rn2(tmp): 0) {

	    case 0:
	    case 1:	pline("but nothing appears to happen.");
			break;
	    case 2:
	    case 3:	pline("A voice booms out: You are arrogant, mortal.");
			pline("You must relearn your lessons!");
			if (u.ulevel > 1)	losexp();
			else  {
			    u.uexp = 0;
			    flags.botl = 1;
			}
			break;
	    case 4:
	    case 5:
	    case 6:	pline("A black glow surrounds you.");
			rndcurse();
			break;
	    case 7:
	    case 8:	pline("A voice booms out: You dare to call upon us?");
			pline("Then, die mortal!");
			mkmon_at('&', u.ux, u.uy);
			break;
				
	    default:	pline("Suddenly, a bolt of lightning strikes you!");
			pline("You are fried to a crisp.");
			killer = "pissed off deity";
			done("died");
			break;
	}
	u.ublesscnt = 250;
	return(0);
}

pleased() {

	char	*tmp, *hcolor();

	u.ugangr--;
	if (u.ugangr < 0) u.ugangr = 0;
	pline("You feel the gods are pleased.");

	switch(rn2((u.uluck + 6)/2))  {

	    case 0:	pline("but nothing seems to happen.");
			break;
	    case 1:
#ifdef KAA
			if(!uwep) {
			    pline("but nothing seems to happen.");
			    break;
			}
			if(uwep->olet == WEAPON_SYM) {
			    if (uwep->cursed) {
				uwep->cursed=0;
				pline("Your %s %s.", aobjnam(uwep,"softly glow"), 
				Hallucination ? hcolor() : "amber");
			    } else if(uwep->otyp >= ARROW && uwep->otyp <= SPEAR) {
				uwep->dknown=1;
				tmp = Hallucination ? hcolor() : "light blue";
				pline("Your %s with a%s %s aura.", aobjnam(uwep,"softly glow"),
				index("aeiou",*tmp) ? "n" : "", tmp);
			    }
			} else
#endif
				pline("but nothing seems to happen.");
			break;
	    case 2:
	    case 3:
			pline("A %s glow surrounds you",
			      Hallucination ? hcolor() : "golden");
			u.uhp = u.uhpmax += 5;
			u.ustr = u.ustrmax;
			if (u.uhunger < 900)	init_uhunger();
			if (u.uluck < 0)	u.uluck = 0;
			if (Blind)		Blind = 1;
			break;
	    case 4:
	    case 5:	pline("A voice booms out: We are pleased with your progress,");
			pline("and grant you the gift of");
			if (!(HTelepat & INTRINSIC))  {
				HTelepat = HTelepat || INTRINSIC;
				pline ("Telepathy,");
			} else if (!(Fast & INTRINSIC))  {
				Fast = Fast || INTRINSIC;
				pline ("Speed,");
			} else if (!(Stealth & INTRINSIC))  {
				Stealth = Stealth || INTRINSIC;
				pline ("Stealth,");
			} else {
			    if (!(Protection & INTRINSIC))  {
				Protection = Protection || INTRINSIC;
				if (!u.ublessed)  u.ublessed = rnd(3) + 1;
			    } else u.ublessed++;
			    pline ("our protection,");
			}
			pline ("Use it wisely in our names!");
			break;

	    case 6:	pline ("An object appears at your feet!");
			mkobj_at("+", u.ux, u.uy);
			break;

	    case 7:	pline("A voice booms out:  We crown thee...");
			pline("The Hand of Elbereth!");
			HInvis |= INTRINSIC;
			HSee_invisible |= INTRINSIC;
			HFire_resistance |= INTRINSIC;
			HCold_resistance |= INTRINSIC;
			HPoison_resistance |= INTRINSIC;
			break;

	    default:	impossible("Confused deity!");
			break;
	}
	u.ublesscnt = 300;
#ifdef HARD
	u.ublesscnt += (u.udemigod * 1000);
#endif
	return(0);
}
#endif /* PRAYERS /**/

