/*	SCCS Id: @(#)pray.c	1.4	87/08/08
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
			if(!uwep) {
			    if(uleft->cursed) {
				pline("your left hand glows amber.");
				uleft->cursed = 0;
			    } else if(uright->cursed) {
				pline("your right hand glows amber.");
				uleft->cursed = 0;
			    } else    pline("but nothing seems to happen.");
			    break;
			}
#ifdef KAA
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
			flags.botl = 1;
			break;
	    case 4:
	    case 5:	pline("A voice booms out: We are pleased with your progress,");
			pline("and grant you the gift of");
			if (!(HTelepat & INTRINSIC))  {
				HTelepat |= INTRINSIC;
				pline ("Telepathy,");
			} else if (!(Fast & INTRINSIC))  {
				Fast |= INTRINSIC;
				pline ("Speed,");
			} else if (!(Stealth & INTRINSIC))  {
				Stealth |= INTRINSIC;
				pline ("Stealth,");
			} else {
			    if (!(Protection & INTRINSIC))  {
				Protection |= INTRINSIC;
				if (!u.ublessed)  u.ublessed = rnd(3) + 1;
			    } else u.ublessed++;
			    pline ("our protection,");
			}
			pline ("Use it wisely in our names!");
			break;

	    case 6:	pline ("An object appears at your feet!");
			mkobj_at('+', u.ux, u.uy);
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
#ifdef NEWCLASS
doturn() {	/* Knights & Priest(esse)s only please */

	register struct monst *mtmp;
	register int	xlev = 6;
	extern char	pl_character[];

	if((pl_character[0] != 'P') &&
	   (pl_character[0] != 'K')) {

		pline("You don't know how to turn undead!");
		return(0);
	}
	if (Inhell) {

		pline("Being in hell, your gods won't help you.");
		aggravate();
		return(0);
	}
	pline("Calling upon your gods, you chant an arcane formula.");
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(cansee(mtmp->mx,mtmp->my)) {
		if(index(UNDEAD,mtmp->data->mlet) ||
		   ((mtmp->data->mlet == '&') && (u.ulevel > 10))) {

		    if(Confusion) {
			pline("Unfortunately, your voice falters.");
			mtmp->mflee = mtmp->mfroz = mtmp->msleep = 0;
		    } else if (! resist(mtmp, '+', 0, TELL))
			switch (mtmp->data->mlet) {
			    case 'V':   xlev += 2;
			    case 'W':   xlev += 4;
			    case 'Z':   if(u.ulevel >= xlev)  {
					    if(!resist(mtmp, '+', 0, NOTELL)) {
						pline("You destroy the %s", monnam(mtmp));
						mondied(mtmp);
					    } else	mtmp->mflee = 1;
					} else	mtmp->mflee = 1;
					break;
			    default:    mtmp->mflee = 1;
					break;
			}
		   }
	    }
	    nomul(-5);
	    return(1);
}
#endif /* NEWCLASS /**/

