/*	SCCS Id: @(#)pray.c	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include "hack.h"

extern char *nomovemsg;
extern struct monst *mkmon_at();
extern struct obj *mkobj_at();

#ifdef PRAYERS
#ifdef THEOLOGY

#define TROUBLE_STONED 1
#define TROUBLE_STARVING 2
#define TROUBLE_SICK 3
#define TROUBLE_HIT 4

#define TROUBLE_PUNISHED (-1)
#define TROUBLE_CURSED_ITEMS (-2)
#define TROUBLE_HUNGRY (-3)
#define TROUBLE_HALLUCINATION (-4)
#define TROUBLE_BLIND (-5)
#define TROUBLE_POISONED (-6)
#define TROUBLE_WOUNDED_LEGS (-7)
#define TROUBLE_CONFUSED (-8)

/* We could force rehumanize of polyselfed people, but we can't tell
   unintentional shape changes from the other kind. Oh well. */

/* Return 0 if nothing particular seems wrong, positive numbers for
   serious trouble, and negative numbers for comparative annoyances. This
   returns the worst problem. There may be others, and the gods may fix 
   more than one.

This could get as bizarre as noting surrounding opponents, (or hostile dogs),
but that's really hard.
 */


in_trouble () {

/* Borrowed from eat.c */

#define	SATIATED	0
#define NOT_HUNGRY	1
#define	HUNGRY		2
#define	WEAK		3
#define	FAINTING	4
#define FAINTED		5
#define STARVED		6

  if (Stoned) return (TROUBLE_STONED);
  if (u.uhs >= WEAK) return (TROUBLE_STARVING);
  if (Sick) return (TROUBLE_SICK);
  if (u.uhp < 5) return (TROUBLE_HIT);
  if (Punished) return (TROUBLE_PUNISHED);
  if (
      (uarmh && uarmh->cursed)	/* helmet */
      ||
      (uarms && uarms->cursed)	/* shield */
      ||
      (uarmg && uarmg->cursed)	/* gloves */
      ||
      (uarm && uarm->cursed)	/* armor */
      ||
      (uarm2 && uarm2->cursed)	/* cloak */
      ||
      (uwep && (uwep->olet == WEAPON_SYM) && (uwep->cursed))
      ||
      (uleft && uleft->cursed)
      ||
      (uright && uright->cursed))
    return (TROUBLE_CURSED_ITEMS);
  
  if (u.uhs >= HUNGRY) return (TROUBLE_HUNGRY);
  if (Hallucination) return (TROUBLE_HALLUCINATION);
  if (Blinded > 1) return (TROUBLE_BLIND); /* don't loop when fixing sets to 1 */
  if (u.ustr < u.ustrmax) return (TROUBLE_POISONED);
  if (Wounded_legs) return (TROUBLE_WOUNDED_LEGS);
  if (Confusion) return (TROUBLE_CONFUSED);
  return (0);
}

fix_worst_trouble ()
{
  char *tmp, *hcolor();

  switch (in_trouble ()) {
  case TROUBLE_STONED: pline ("You feel more limber."); Stoned = 0; break;
  case TROUBLE_HUNGRY:
  case TROUBLE_STARVING: pline ("Your stomach feels content."); init_uhunger (); break;
  case TROUBLE_SICK: pline ("You feel better."); Sick = 0; break;
  case TROUBLE_HIT: 
    pline("A %s glow surrounds you",
	  Hallucination ? hcolor () : "golden");
    u.uhp = u.uhpmax += 5;
    break;

 case TROUBLE_PUNISHED:
   Punished = 0;
   freeobj(uchain);
   unpobj(uchain);
   free((char *) uchain);
   uball->spe = 0;
   uball->owornmask &= ~W_BALL;
   uchain = uball = (struct obj *) 0;
   break;

 case TROUBLE_CURSED_ITEMS:
    { struct obj *otmp;
      char * what;
      otmp = (struct obj *) 0;
      what = (char *) 0;
      if (uarmh && uarmh->cursed) 	/* helmet */
	otmp = uarmh;
      else if (uarms && uarms->cursed) /* shield */
	otmp = uarms;
      else if (uarmg && uarmg->cursed) /* gloves */
	otmp = uarmg;
      else if (uarm && uarm->cursed) /* armor */
	otmp = uarm;
      else if (uarm2 && uarm2->cursed) /* cloak, probably */
	otmp = uarm2;
      else if (uleft && uleft->cursed) {
	otmp = uleft;
	what = "left hand";
      }
      else if (uright && uright->cursed) {
	otmp = uright;
	what = "right hand";
      }
      else if(uwep && (uwep->olet == WEAPON_SYM) && (uwep->cursed))
	otmp = uwep;
      
      otmp->cursed = 0;
      pline ("Your %s %s.", what ? what : aobjnam (otmp, "softly glow"),
	     Hallucination ? hcolor() : "amber");
      break;
    }

 case TROUBLE_HALLUCINATION:
   pline ("Looks like you are back in kansas."); Hallucination = 0; break;
 case TROUBLE_BLIND:
    pline ("Your eyes feel better."); Blinded = 1; break;
 case TROUBLE_POISONED:
   pline("There's a tiger in your tank.");
   if(u.ustr < u.ustrmax) {
     u.ustr = u.ustrmax;
     flags.botl = 1;
   }
   break;
 case TROUBLE_WOUNDED_LEGS:
   heal_legs ();
   break;
 case TROUBLE_CONFUSED:
   HConfusion = 0;
   break;
 }
}

#define CORPSE_I_TO_C(otyp)	(char) ((otyp >= DEAD_ACID_BLOB)\
					?  'a' + (otyp - DEAD_ACID_BLOB)\
					:	'@' + (otyp - DEAD_HUMAN))

extern char POISONOUS[];

dosacrifice () {
  register struct obj *otmp;
  register struct objclass *ftmp;
  int value;
  char let;
  char *hcolor ();

  otmp = getobj("%", "sacrifice");
  if(!otmp) return(0);

  ftmp = &objects[otmp->otyp];
#ifdef KAA
  if(otmp->otyp == DEAD_DEMON) let='&';
  else if (otmp->otyp == DEAD_GIANT) let='9';
  else let = CORPSE_I_TO_C(otmp->otyp);
#else
  let = CORPSE_I_TO_C(otmp->otyp);
#endif

  /* judge the food value by the nutrition, as well as some aging behavior */

  value = ftmp->nutrition;

  if (otmp->otyp >= CORPSE)
    {
      if (let != 'a' && (moves > otmp->age + 50))
	value = 0;		/* old stuff */
    }
  else if(index(POISONOUS, let)) {
    value = -10;		/* gods get annoyed */
  }


  if (value == 0)
    {
      pline ("Nothing happens.");
      return (1);
    }
  if (value < 0) /* Gods don't like poision as an offering. */
    {
#ifdef HARD
      u.ugangr++;
      angrygods();
#else
      if (u.ugangr++)	angrygods();
      else {			/* exactly one warning */
	pline("A voice booms out: You have angered us,");
	pline("Disturb us again at your own risk!");
      }
#endif
    }
  if (value > 0)
    {
      int saved_anger = u.ugangr;
      int saved_cnt = u.ublesscnt;
      int saved_luck = u.uluck;
      /* OK, you get brownie points. */
      if (Hallucination)
	pline("Your sacrifice sprouts wings and a propeller and roars away!");
      else pline("Your sacrifice is consumed in a burst of flame!");
      if(u.ugangr)
	{
	  u.ugangr -= ((value / 800) * 4);
	  if(u.ugangr < 0) u.ugangr = 0;
	  if(u.ugangr != saved_anger)
	    if (u.ugangr)
	      {
		if (Hallucination) pline ("The gods seem %s.", hcolor ());
		else pline("The gods seem slightly molified.");	  
	      }
	    else {
	      if (Hallucination) pline ("The gods seem cosmic (not a new fact).");
	      else pline ("The gods seem mollified.");
	    }
	  if ((int)u.uluck < 0) u.uluck = 0;
	}
      else if (u.ublesscnt > 0)
	{
	  u.ublesscnt -= ((value / 800 /* a food ration */) * 300);
	  if(u.ublesscnt < 0) u.ublesscnt = 0;
	  if(u.ublesscnt != saved_cnt)
	    {
	      if ((int)u.uluck < 0) u.uluck = 0;
	      if (u.ublesscnt) {
		if (Hallucination) pline ("You realize that the gods are not like you and I.");
		else pline ("You have a hopeful feeling.");
	      }
	      else {
		if (Hallucination) pline ("Overall, there is a smell of fried onions.");
		else pline ("You have a feeling of reconciliation.");
	      }
	    }
	}
      else /* you were already in pretty good standing */
	{
	  change_luck((value / 800) * LUCKMAX);
	  if (u.uluck != saved_luck)
	    {
	      if (Hallucination) pline ("You see crabgrass at your feet. A funny thing in a dungeon.");
	      else pline ("You see a four-leaf clover at your feet.");
	    }
	}
    }
  useup(otmp);
  return(1);
}
#endif 
#endif

dopray() {		/* M. Stephenson (1.0.3b) */
#ifdef PRAYERS
#ifdef THEOLOGY
  int trouble = in_trouble ();
  if ((!trouble && (u.ublesscnt > 0)) ||
      ((trouble < 0) && (u.ublesscnt > 100)) /* minor difficulties */ ||
      ((trouble > 0) && (u.ublesscnt > 200)) /* big trouble */
      ) {
#else      
    if (u.ublesscnt > 0)  {		/* disturbing the gods too much */
#endif
		u.ublesscnt += rnz(250);
		change_luck(-3);
#ifdef HARD
		u.ugangr++;
		angrygods();
#else
		if (u.ugangr++)	angrygods();
		else {			/* exactly one warning */
			pline("A voice booms out: You have angered us,");
			pline("Disturb us again at your own risk!");
		}
#endif
	} else  if ((int)u.uluck < 0) angrygods();	/* a bad player */
	else	pleased();		    		/* or a good player */
#endif
	nomovemsg = "You finished your prayer.";
	nomul(-3);
	return(1);
}

#ifdef PRAYERS
angrygods() {
	register int	tmp;

	pline ("You get the feeling the gods are angry...");
	/* changed from tmp = u.ugangr + abs (u.uluck) -- rph */
	tmp =  3*u.ugangr + (u.uluck > 0 ? -u.uluck/3 : -u.uluck);
	tmp =  (tmp > 15 ? 15 : tmp);  /* lets be a little reasonable */
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
	u.ublesscnt = rnz(300);
	return(0);
}



pleased() {

	char	*tmp, *hcolor();
#ifdef THEOLOGY	
	int trouble = in_trouble ();		/* what's your worst difficulty? */
	int pat_on_head = 0;
#endif
	u.ugangr--;
	if (u.ugangr < 0) u.ugangr = 0;
	pline("You feel the gods are pleased.");

#ifdef THEOLOGY
	/* depending on your luck, the gods will :
	   - fix your worst problem if its major.
	   - fix all your major problems.
	   - fix your worst problem if its minor.
	   - fix all of your problems.
	   - do you a gratuitous favor.

	   if you make it to the the last category, you roll randomly again
	   to see what they do for you. 

	   If your luck is at least 0, then you are guaranteed rescued
	   from your worst major problem. */

	if (!trouble) pat_on_head = 1;
	else 
	  { int action = rn1(5,u.uluck+1);
	    if (action>=5) pat_on_head = 1;
	    if (action>=4) while (in_trouble ()) fix_worst_trouble ();
	    if (action<4 && action>=3) fix_worst_trouble ();
	    if (action<4 && action>=2) while (in_trouble () > 0) fix_worst_trouble ();
	    if (action==1 && trouble > 0) fix_worst_trouble ();
	    if (action==0) pline("but nothing seems to happen.");
	  };
	if (pat_on_head)pline("The gods seem especially pleased ...");

     if (pat_on_head)
#endif
	switch(rn2((u.uluck + 6)/2))  {

	    case 0:	pline("but nothing seems to happen.");
			break;
	    case 1:
			if(!uwep) {
			    if(uleft && uleft->cursed) {
				pline("your left hand glows amber.");
				uleft->cursed = 0;
			    } else if(uright && uright->cursed) {
				pline("your right hand glows amber.");
				uright->cursed = 0;
			    } else    pline("but nothing seems to happen.");
			    break;
			}
#ifdef KAA
			if(uwep && (uwep->olet == WEAPON_SYM)) {
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
			if (Blinded)		Blinded = 1;
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
#ifdef SPELLS
			mkobj_at('+', u.ux, u.uy);
#else
			mkobj_at('?', u.ux, u.uy);
#endif
			break;

	    case 7:	pline("A voice booms out:  We crown thee...");
			pline("The Hand of Elbereth!");
			HInvis |= INTRINSIC;
			HSee_invisible |= INTRINSIC;
			HFire_resistance |= INTRINSIC;
			HCold_resistance |= INTRINSIC;
			HPoison_resistance |= INTRINSIC;
#ifdef RPH
			if(uwep && (uwep->otyp == LONG_SWORD))
				oname(uwep, "Excalibur");
#endif
			break;

	    default:	impossible("Confused deity!");
			break;
	}
	u.ublesscnt = rnz(350);
#ifdef HARD
	u.ublesscnt += (u.udemigod * rnz(1000));
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
