/*	SCCS Id: @(#)potion.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* potion.c - version 1.0.3 */

#include "hack.h"
extern int float_down();
extern char *nomovemsg;
extern struct monst youmonst;
extern struct monst *makemon();
char *hcolor();
#ifdef KAA
char *xname();
extern char pl_character[];
#endif
#ifdef FOUNTAINS
extern int drinkfountain();
extern int dipfountain();
#endif

int	nothing, unkn;

dodrink() {
	register struct obj *otmp;
	register int	retval;

#ifdef FOUNTAINS

      /* Is there something to drink here, i.e., a fountain? */
       if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
	  pline("Drink from the fountain? [ny] ");
	  if(readchar() == 'y') {
	    (void) drinkfountain();
	    return(0);
	  }
       }   

#endif /* FOUNTAINS /**/

	nothing = unkn = 0;
	otmp = getobj("!", "drink");
	if(!otmp) return(0);
	if(!strcmp(objects[otmp->otyp].oc_descr, "smoky") && !rn2(13)) {
		ghost_from_bottle();
		goto use_it;
	}
	if((retval = peffects(otmp)) >= 0) return(retval);

	if(nothing) {
	    unkn++;
	    pline("You have a %s feeling for a moment, then it passes.",
		  Hallucination ? "normal" : "peculiar");
	}
	if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
		if(!unkn) {
			objects[otmp->otyp].oc_name_known = 1;
			more_experienced(0,10);
		} else if(!objects[otmp->otyp].oc_uname)
			docall(otmp);
	}
use_it:
	useup(otmp);
	return(1);
}

peffects(otmp)
	register struct obj	*otmp;
{
	register struct obj	*objs;
	register struct monst	*mtmp;

	switch(otmp->otyp){
	case POT_RESTORE_STRENGTH:
#ifdef SPELLS
	case SPE_RESTORE_STRENGTH:
#endif
		unkn++;
		pline("Wow!  This makes you feel great!");
		if(u.ustr < u.ustrmax) {
			u.ustr = u.ustrmax;
			flags.botl = 1;
		}
		break;
#ifdef KAA
	case POT_HALLUCINATION:
		if (Hallucination) nothing++;
		else pline("Oh wow!  Everything looks so cosmic!");
		Hallucination += rn1(100,750);
		setsee();
		break;
	case POT_HOLY_WATER:
		unkn++;
		if(index("VWZ&",u.usym)) {
			pline("This burns like acid!");
			losehp(d(2,6)); /* will never kill you */
		} else {
			pline("You feel full of awe.");
			if (Sick) Sick=0;
			if (HConfusion) HConfusion=0;
		}
#else
	case POT_HOLY_WATER:
	case POT_HALLUCINATION:
#endif
		break;
	case POT_BOOZE:
		unkn++;
		pline("Ooph!  This tastes like liquid fire!");
		HConfusion += d(3,8);
		/* the whiskey makes us feel better */
		if(u.uhp < u.uhpmax) losehp(-1, "bottle of whiskey");
		if(!rn2(4)) {
			pline("You pass out.");
			multi = -rnd(15);
			nomovemsg = "You awake with a headache.";
		}
		break;
	case POT_INVISIBILITY:
#ifdef SPELLS
	case SPE_INVISIBILITY:
#endif
		if(Invis || See_invisible)
		  nothing++;
		else {
		  if(!Blind)
		    pline("Gee!  All of a sudden, you can't see yourself.");
		  else
		    pline("You feel rather airy."), unkn++;
		  newsym(u.ux,u.uy);
		}
		HInvis += rn1(15,31);
		break;
	case POT_FRUIT_JUICE:
		pline("This tastes like fruit juice.");
		lesshungry(20);
		break;
	case POT_HEALING:
		pline("You begin to feel better.");
		healup(rnd(10), 1, 1, 1);
		break;
	case POT_PARALYSIS:
		if(Levitation)
			pline("You are motionlessly suspended.");
		else
			pline("Your feet are frozen to the floor!");
		nomul(-(rn1(10,25)));
		break;
	case POT_MONSTER_DETECTION:
#ifdef SPELLS
	case SPE_DETECT_MONSTERS:
#endif
		if(!fmon) {
			strange_feeling(otmp, "You feel threatened.");
			return(1);
		} else {
			cls();
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				if(mtmp->mx > 0)
				at(mtmp->mx,mtmp->my,Hallucination ? rndmonsym() :
				mtmp->data->mlet);
			prme();
			pline("You sense the presence of monsters.");
			more();
			docrt();
		}
		break;
	case POT_OBJECT_DETECTION:
#ifdef SPELLS
	case SPE_DETECT_TREASURE:
#endif
		if(!fobj) {
			strange_feeling(otmp, "You feel a pull downward.");
			return(1);
		} else {
		    for(objs = fobj; objs; objs = objs->nobj)
			if(objs->ox != u.ux || objs->oy != u.uy)
				goto outobjmap;
		    pline("You sense the presence of objects close nearby.");
		    break;
		outobjmap:
			cls();
			for(objs = fobj; objs; objs = objs->nobj)
				at(objs->ox,objs->oy,Hallucination ? rndobjsym()
				 : objs->olet);

			/* monster possessions added by GAN 12/16/86 */
			for(mtmp = fmon ; mtmp ; mtmp = mtmp->nmon)
				if(mtmp->minvent)
					for(objs = mtmp->minvent; objs ;
					    objs = objs->nobj)
						at(mtmp->mx,mtmp->my,objs->olet);
			prme();
			pline("You sense the presence of objects.");
			more();
			docrt();
		}
		break;
	case POT_SICKNESS:
		pline("Yech! This stuff tastes like poison.");
		if(Poison_resistance)
    pline("(But in fact it was biologically contaminated orange juice.)");
#ifdef KAA
		if (pl_character[0] == 'H')
			pline("Fortunately you have been immunized!");
		else {
#endif
			losestr(rn1(4,3));
			losehp(rnd(10), "contaminated potion");
#ifdef KAA
		}
#endif
		if(Hallucination) {
			pline("You are shocked back to your senses!");
			Hallucination=1;
		}
		break;
	case POT_CONFUSION:
		if(!Confusion)
		    if (Hallucination) {
			pline("What a trippy feeling!");
			unkn++;
		    } else
			pline("Huh, What?  Where am I?");
		else	nothing++;
		HConfusion += rn1(7,16);
		break;
	case POT_GAIN_STRENGTH:
		pline("Wow do you feel strong!");
		gainstr(0);
		break;
	case POT_SPEED:
		if(Wounded_legs) {
			heal_legs();
			unkn++;
			break;
		}		/* and fall through */
#ifdef SPELLS
	case SPE_HASTE_SELF:
#endif
		if(!(Fast & ~INTRINSIC))
			pline("You are suddenly moving much faster.");
		else
			pline("Your legs get new energy."), unkn++;
		Fast += rn1(10,100);
		break;
	case POT_BLINDNESS:
		if(!Blind)
			if (Hallucination)
				pline("Bummer!  Everything is dark!  Help!");
			else 
				pline("A cloud of darkness falls upon you.");
		else	nothing++;
		Blind += rn1(100,250);
		seeoff(0);
		break;
	case POT_GAIN_LEVEL: 
		pluslvl();
		break;
	case POT_EXTRA_HEALING:
		pline("You feel much better.");
		healup(d(2,20)+1, 2, 1, 1);
		if(Hallucination) Hallucination = 1;
		break;
	case POT_LEVITATION:
#ifdef SPELLS
	case SPE_LEVITATION:
#endif
		if(!Levitation)
			float_up();
		else
			nothing++;
		Levitation += rnd(100);
		u.uprops[PROP(RIN_LEVITATION)].p_tofn = float_down;
		break;
	case POT_GAIN_ENERGY:			/* M. Stephenson */
#ifdef SPELLS
		{	register int	 num;
			if(Confusion) {
			    pline("You feel feverish.");
			    unkn++;
			} else
			    pline("Magical energies course through your body.");
			num = rnd(5) + 1;
			u.uenmax += num;
			u.uen += num;
			flags.botl = 1;
			break;
		}
#else
		pline("This potion tastes wierd!");
		break;
#endif
	default:
		impossible("What a funny potion! (%u)", otmp->otyp);
		return(0);
	}
	return(-1);
}

healup(nhp, nxtra, curesick, cureblind)
	int	nhp, nxtra;
	register boolean curesick, cureblind;
{
#ifdef KAA
	if (u.mtimedone & nhp) {
		u.mh += rnd(nhp);
		if (u.mh > u.mhmax) u.mh = ++u.mhmax;
	}
#endif
	if(nhp)	{
		u.uhp += nhp;
		if(u.uhp > u.uhpmax)	u.uhp = (u.uhpmax += nxtra);
	}
	if(Blind && cureblind)	Blind = 1;	/* see on next move */
	if(Sick && curesick)	Sick = 0;
	flags.botl = 1;
	return;
}

pluslvl()
{
	register num;

	pline("You feel more experienced.");
	num = rnd(10);
	u.uhpmax += num;
	u.uhp += num;
#ifdef SPELLS
	num = rnd(u.ulevel/2+1) + 1;		/* M. Stephenson */
	u.uenmax += num;
	u.uen += num;
#endif
	if(u.ulevel < 14) {
		extern long newuexp();

		u.uexp = newuexp()+1;
		pline("Welcome to experience level %u.", ++u.ulevel);
	}
	flags.botl = 1;
}

strange_feeling(obj,txt)
register struct obj *obj;
register char *txt;
{
	if(flags.beginner)
		pline("You have a %s feeling for a moment, then it passes.",
		Hallucination ? "normal" : "strange");
	else
		pline(txt);
	if(!objects[obj->otyp].oc_name_known && !objects[obj->otyp].oc_uname)
		docall(obj);
	useup(obj);
}

char *bottlenames[] = {
	"bottle", "phial", "flagon", "carafe", "flask", "jar", "vial"
};

potionhit(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	extern char *xname();
	register char *botlnam = bottlenames[rn2(SIZE(bottlenames))];
	boolean uclose, isyou = (mon == &youmonst);

	if(isyou) {
		uclose = TRUE;
		pline("The %s crashes on your head and breaks into shivers.",
			botlnam);
		losehp(rnd(2), "thrown potion");
	} else {
		uclose = (dist(mon->mx,mon->my) < 3);
		/* perhaps 'E' and 'a' have no head? */
		pline("The %s crashes on %s's head and breaks into shivers.",
			botlnam, monnam(mon));
		if(rn2(5) && mon->mhp > 1)
			mon->mhp--;
	}
	pline("The %s evaporates.", xname(obj));

#ifdef KAA
	if(!isyou) switch (obj->otyp) {
#else
	if(!isyou && !rn2(3)) switch(obj->otyp) {
#endif

	case POT_RESTORE_STRENGTH:
	case POT_GAIN_STRENGTH:
	case POT_HEALING:
	case POT_EXTRA_HEALING:
		if(mon->mhp < mon->mhpmax) {
			mon->mhp = mon->mhpmax;
			pline("%s looks sound and hale again!", Monnam(mon));
		}
		break;
	case POT_SICKNESS:
		if((mon->mhpmax > 3) && !resist(mon, '!', 0, NOTELL))
			mon->mhpmax /= 2;
		if((mon->mhp > 2) && !resist(mon, '!', 0, NOTELL))
			mon->mhp /= 2;
#ifdef KAA
		pline("%s looks rather ill.", Monnam(mon));
#endif
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!resist(mon, '!', 0, NOTELL))  mon->mconf = 1;
		break;
	case POT_INVISIBILITY:
		unpmon(mon);
		mon->minvis = 1;
		pmon(mon);
		break;
	case POT_PARALYSIS:
		mon->mfroz = 1;
		break;
	case POT_SPEED:
		mon->mspeed = MFAST;
		break;
	case POT_BLINDNESS:
		mon->mblinded |= 64 + rn2(32) +
				      rn2(32) * !resist(mon, '!', 0, NOTELL);
		break;
#ifdef KAA
	case POT_HOLY_WATER:
		if (index("ZVW &", mon->data->mlet)) {
			pline("%s shrieks in pain!", Monnam(mon));
			mon->mhp -= d(2,6);
			if (mon->mhp <1) killed(mon);
		}
		break;
#endif
/*	
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
		break;
*/
	}
	if(uclose && rn2(5))
		potionbreathe(obj);
	obfree(obj, Null(obj));
}

potionbreathe(obj)
register struct obj *obj;
{
	switch(obj->otyp) {
	case POT_RESTORE_STRENGTH:
	case POT_GAIN_STRENGTH:
		if(u.ustr < u.ustrmax) u.ustr++, flags.botl = 1;
		break;
	case POT_HEALING:
	case POT_EXTRA_HEALING:
		if(u.uhp < u.uhpmax) u.uhp++, flags.botl = 1;
		break;
	case POT_SICKNESS:
		if(u.uhp <= 5) u.uhp = 1; else u.uhp -= 5;
		flags.botl = 1;
		break;
	case POT_HALLUCINATION:
#ifdef KAA
		pline("You have a vision for a moment.");
		break;
#endif
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!Confusion)
			pline("You feel somewhat dizzy.");
		HConfusion += rnd(5);
		break;
	case POT_INVISIBILITY:
		pline("For an instant you couldn't see your right hand.");
		break;
	case POT_PARALYSIS:
		pline("Something seems to be holding you.");
		nomul(-rnd(5));
		break;
	case POT_SPEED:
		Fast += rnd(5);
		pline("Your knees seem more flexible now.");
		break;
	case POT_BLINDNESS:
		if(!Blind) pline("It suddenly gets dark.");
		Blind += rnd(5);
		seeoff(0);
		break;
/*	
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
		break;
*/
	}
	/* note: no obfree() */
}

/*
 * -- rudimentary -- to do this correctly requires much more work
 * -- all sharp weapons get one or more qualities derived from the potions
 * -- texts on scrolls may be (partially) wiped out; do they become blank?
 * --   or does their effect change, like under Confusion?
 * -- all objects may be made invisible by POT_INVISIBILITY
 * -- If the flask is small, can one dip a large object? Does it magically
 * --   become a jug? Etc.
 */
dodip(){
	register struct obj *potion, *obj;
#ifdef KAA
	char *tmp;
#endif

	if(!(obj = getobj("#", "dip")))
		return(0);
#ifdef FOUNTAINS
	/* Is there something to dip into here, i.e., a fountain? */
	if (levl[u.ux][u.uy].typ == FOUNTAIN) {
		pline("Dip it in the fountain? [ny] ");
		if(readchar() == 'y') {
			dipfountain(obj);
			return(1);
		}
	}
#endif
	if(!(potion = getobj("!", "dip into")))
		return(0);
#ifndef KAA
	pline("Interesting...");
#else
	if(potion->otyp == POT_HOLY_WATER) {
		if (obj->cursed) {
			obj->cursed=0;
			pline("Your %s %s.", aobjnam(obj,"softly glow"), 
			Hallucination ? hcolor() : "amber");
	poof:	useup(potion);
			return(1);
		} else if(obj->otyp >= ARROW && obj->otyp <= SPEAR) {
			obj->dknown=1;
			tmp = Hallucination ? hcolor() : "light blue";
	/* dknown for weapons is meaningless, so it's free to be reused. */
			pline("Your %s with a%s %s aura.", aobjnam(obj,"softly glow"),
			index("aeiou",*tmp) ? "n" : "", tmp);
			goto poof;
		}
	}
#endif
	if(obj->otyp == ARROW || obj->otyp == DART ||
	   obj->otyp == CROSSBOW_BOLT || obj->otyp == SHURIKEN) {
		if(potion->otyp == POT_SICKNESS) {
			char buf[BUFSZ];
			useup(potion);
			if(obj->spe < 7) obj->spe++;	/* %% */
			sprintf(buf, xname(potion));
			pline("The %s forms a coating on the %s.",
				buf, xname(obj));
		}
	}
#ifdef KAA
	pline("Interesting...");
#endif
	return(1);
}

ghost_from_bottle(){
	extern struct permonst pm_ghost;
	register struct monst *mtmp;

	if(!(mtmp = makemon(PM_GHOST,u.ux,u.uy))){
		pline("This bottle turns out to be empty.");
		return;
	}
	mnexto(mtmp);
	pline("As you open the bottle, an enormous ghost emerges!");
	pline("You are frightened to death, and unable to move.");
	nomul(-3);
}

gainstr(inc)
register int	inc;
{
	if(u.ustr >= 118) return;	/* > 118 is impossible */

	if((u.ustr > 17) && !inc)	u.ustr += rnd(118 - u.ustr);
#ifdef HARD
	else				u.ustr++;
#else 
	else				u.ustr += (inc) ? 1 : rnd(3);
#endif

	if(u.ustr > u.ustrmax)		u.ustrmax = u.ustr;
	flags.botl = 1;
}
