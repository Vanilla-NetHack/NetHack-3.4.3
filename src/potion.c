/*	SCCS Id: @(#)potion.c	3.0	88/11/11
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static int nothing, unkn;
#ifdef WORM
boolean notonhead = FALSE;
#endif

static const char beverages[] = { POTION_SYM, 0 };

void
make_confused(xtime,talk)
long xtime;
boolean talk;
{
	long old = HConfusion;

	if (!xtime && old) {
		if (talk) {
			if (Hallucination) You("feel less trippy now.");
			else		   You("feel less confused now.");
		}
		flags.botl = 1;
	}
	if (xtime && !old)
		flags.botl = 1;
	HConfusion = xtime;
}

void
make_stunned(xtime,talk)
long xtime;
boolean talk;
{
	long old = HStun;

	if (!xtime && old) {
		if (talk) {
			if (Hallucination) You("feel less wobbly now.");
			else		   You("feel a bit steadier now.");
		}
		flags.botl = 1;
	}
	if (xtime && !old) {
		if (talk)
			You("stagger...");
		flags.botl = 1;
	}
	HStun = xtime;
}

void
make_sick(xtime, talk)
long xtime;
boolean talk;
{
	long old = Sick;

#ifdef POLYSELF
	if (xtime && u.usym == S_FUNGUS) return;
#endif
	if (!xtime && old) {
		if (talk) pline("What a relief!");
		flags.botl = 1;
	}
	if (!old && xtime) {
		You("feel deathly sick.");
		flags.botl = 1;
	}
	Sick = xtime;
}

void
make_blinded(xtime, talk)
long xtime;
boolean talk;
{
	long old = Blinded;

	if (!xtime && old && !Blindfolded) {
		if (talk) {
			if (Hallucination) pline("Oh, like, wow!  What a rush.");
			else		   You("can see again.");
		}
		flags.botl = 1;
	}
	if (xtime && !old && !Blindfolded) {
		if (talk) {
			if (Hallucination)
				pline("Bummer!  Everything is dark!  Help!");
			else
				pline("A cloud of darkness falls upon you.");
		}
		seeoff(0);
		flags.botl = 1;
	}
	Blinded = xtime;
	if (!Blind)
		setsee();
}

void
make_hallucinated(xtime, talk)
long xtime;
boolean talk;
{
	long old = Hallucination;
	register struct monst *mtmp;

	if (!xtime && old ) {
		if (!Blind && talk) pline("Everything looks SO boring now.");
		for (mtmp=fmon; mtmp; mtmp=mtmp->nmon)
		  if ((Blind && Telepat) || canseemon(mtmp))
		    atl(mtmp->mx, mtmp->my, (!mtmp->mappearance ||
					     Protection_from_shape_changers)
			? mtmp->data->mlet : mtmp->mappearance);
		flags.botl = 1;
	}
	if (xtime && !old ) {
		if (!Blind && talk) pline("Oh wow!  Everything looks so cosmic!");
		flags.botl = 1;
	}
	Hallucination = xtime;
	setsee();
}

static void
ghost_from_bottle()
{
	if(!makemon(&mons[PM_GHOST], u.ux, u.uy)){
		pline("This bottle turns out to be empty.");
		return;
	}
	pline("As you open the bottle, an enormous ghost emerges!");
	if(flags.verbose)
	    You("are frightened to death, and unable to move.");
	nomul(-3);
	nomovemsg = "You regain your composure.";
}

int
dodrink() {
	register struct obj *otmp;

#ifdef FOUNTAINS
	/* Is there a fountain to drink from here? */
        if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		pline("Drink from the fountain? ");
		if(yn() == 'y') {
			(void) drinkfountain();
			return 1;
		}
	}
#endif
#ifdef SINKS
	/* Or a kitchen sink? */
	if (IS_SINK(levl[u.ux][u.uy].typ)) {
		pline("Drink from the sink? ");
		if (yn() == 'y') {
			(void) drinksink();
			return 1;
		}
	}
#endif

	otmp = getobj(beverages, "drink");
	if(!otmp) return(0);
	if(objects[otmp->otyp].oc_descr && !strcmp(objects[otmp->otyp].oc_descr, "smoky") && !rn2(13)) {
		ghost_from_bottle();
		useup(otmp);
		return(1);
	}
	if(objects[otmp->otyp].oc_descr && !strcmp(objects[otmp->otyp].oc_descr, "glowing") && !rn2(13)) {
		djinni_from_bottle(otmp);
		useup(otmp);
		return(1);
	}
	return dopotion(otmp);
}

int
dopotion(otmp)
register struct obj *otmp;
{
	int retval;

	nothing = unkn = 0;
	if((retval = peffects(otmp)) >= 0) return(retval);

	if(nothing) {
	    unkn++;
	    You("have a %s feeling for a moment, then it passes.",
		  Hallucination ? "normal" : "peculiar");
	}
	if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
		if(!unkn) {
			makeknown(otmp->otyp);
			more_experienced(0,10);
		} else if(!objects[otmp->otyp].oc_uname)
			docall(otmp);
	}
	useup(otmp);
	return(1);
}

int
peffects(otmp)
	register struct obj	*otmp;
{
	register int i, ii, isdone;

	switch(otmp->otyp){
	case POT_RESTORE_ABILITY:
#ifdef SPELLS
	case SPE_RESTORE_ABILITY:
#endif
		unkn++;
		if(otmp->cursed) {
		    pline("Ulch!  This makes you feel mediocre!");
		    break;
		} else {
		    pline("Wow!  This makes you feel %s!",
			  (otmp->blessed) ? "great" : "good");
		    i = rn2(A_MAX);		/* start at a random point */
		    for(isdone = ii = 0; !isdone && ii < A_MAX; ii++) {
			if(ABASE(i) < AMAX(i)) {
			    ABASE(i) = AMAX(i);
			    /* only first found if not blessed */
			    isdone = !(otmp->blessed);
			    flags.botl = 1;
			}
			if(++i >= A_MAX) i = 0;
		    }
		}
		break;
	case POT_HALLUCINATION:
		if (Hallucination) nothing++;
		make_hallucinated(Hallucination +
				  rn1(200, 600 - 300*bcsign(otmp)), TRUE);
		break;
	case POT_WATER:
		if(!otmp->blessed && !otmp->cursed) {
			pline("This tastes like %swater.",
			      otmp->spe == -1 ? "impure " :
			      !otmp->rustfree ? "" : "mineral "
			     );
			lesshungry(rnd(otmp->spe == -1 ? 3 : 10));
			break;
		}
		unkn++;
#ifdef POLYSELF
		if(is_undead(uasmon) || is_demon(uasmon) || 
				u.ualigntyp == U_CHAOTIC) {
		    if(otmp->blessed) {
			pline("This burns like acid!");
			losehp(d(2,6), "potion of holy water");
		    } else if(otmp->cursed) {
			You("feel quite proud of yourself.");
			healup(d(2,6),0,0,0);
		    }
		} else
#endif
		    if(otmp->blessed) {
			You("feel full of awe.");
			make_sick(0L,TRUE);
#ifdef POLYSELF
			if (u.ulycn != -1) {
				You("feel purified.");
				if(uasmon == &mons[u.ulycn] && !Polymorph_control)
					rehumanize();
				u.ulycn = -1;
			}
#endif
			/* make_confused(0L,TRUE); */
		    } else {
			if(u.ualigntyp == U_LAWFUL) {
			    pline("This burns like acid!");
			    losehp(d(2,6), "potion of unholy water");
			} else
			    You("feel full of dread.");
		    }
		break;
	case POT_BOOZE:
		unkn++;
		pline("Ooph!  This tastes like %s!",
		      Hallucination ? "furniture polish" : "liquid fire");
		if (!otmp->blessed) make_confused(HConfusion + d(3,8),FALSE);
		/* the whiskey makes us feel better */
		if(u.uhp < u.uhpmax) losehp(-1, "bottle of whiskey");
		lesshungry(10 * (2 + bcsign(otmp)));
		if(otmp->cursed) {
			You("pass out.");
			multi = -rnd(15);
			nomovemsg = "You awake with a headache.";
		}
		break;
	case POT_ENLIGHTENMENT:
		if(otmp->cursed) {
			unkn++;
			You("have an uneasy feeling...");
		} else {
			You("feel self-knowledgable...");
			if (otmp->blessed) {
				adjattrib(A_INT, 1, FALSE);
				adjattrib(A_WIS, 1, FALSE);
			}
			more();
			enlightenment();
			pline("The feeling subsides.");
		}
		break;
	case POT_INVISIBILITY:
#ifdef SPELLS
	case SPE_INVISIBILITY:
#endif
		if(Invisible || See_invisible) nothing++;
		else {
		     newsym(u.ux,u.uy);
		     if(!Blind)
		       pline("Gee!  All of a sudden, you can't see yourself.");
		     else
		       You("feel rather airy."), unkn++;
		}
		if (otmp->blessed && !(HInvis & INTRINSIC)) {
			pline("Do you want the invisibility to be permanent? ");
			nothing = 0;
			if (yn()=='n') HInvis += rn1(15,31);
			else HInvis |= INTRINSIC;
		} else HInvis += rn1(15,31);
		if(otmp->cursed) {
		    pline("For some reason, you feel your presence is known.");
		    aggravate();
		}
		break;
	case POT_SEE_INVISIBLE:
	case POT_FRUIT_JUICE:
		unkn++;
		if(otmp->cursed)
			pline("Yecch!  This tastes %s.",
			  Hallucination ? "overripe" : "rotten"
			 );
		else pline (Hallucination ?
#ifdef TUTTI_FRUTTI
		   "This tastes like 10%% real %s juice all-natural beverage." :
		   "This tastes like %s juice.", pl_fruit);
#else
		   "This tastes like 10%% real fruit juice all-natural beverage." :
		   "This tastes like fruit juice.");
#endif
		if (otmp->otyp == POT_FRUIT_JUICE) {
			lesshungry(10 * (2 + bcsign(otmp)));
			break;
		}
		if (!otmp->cursed) {
			/* Tell them they can see again immediately, which
			 * will help them identify the potion...
			 */
			make_blinded(0L,TRUE);
		}
		if (otmp->blessed)
			HSee_invisible |= INTRINSIC;
		else
			HSee_invisible += rn1(100,750);
		break;
	case POT_PARALYSIS:
		if(Levitation)
			You("are motionlessly suspended.");
		else
			Your("%s are frozen to the floor!",
				makeplural(body_part(FOOT)));
		nomul(-(rn1(10, 25 - 12*bcsign(otmp))));
		break;
	case POT_MONSTER_DETECTION:
#ifdef SPELLS
	case SPE_DETECT_MONSTERS:
#endif
		if (monster_detect(otmp))
			return(1);		/* nothing detected */
		break;
	case POT_OBJECT_DETECTION:
#ifdef SPELLS
	case SPE_DETECT_TREASURE:
#endif
		if (object_detect(otmp))
			return(1);		/* nothing detected */
		break;
	case POT_SICKNESS:
		pline("Yecch!  This stuff tastes like poison.");
		if (otmp->blessed) {
#ifdef TUTTI_FRUTTI
		pline("(But in fact it was mildly stale %s juice.)", pl_fruit);
#else
		pline("(But in fact it was mildly stale orange juice.)");
#endif
			if (pl_character[0] != 'H')
				losehp(1, "mildly contaminated potion");
		} else {
		    if(Poison_resistance)
#ifdef TUTTI_FRUTTI
    pline("(But in fact it was biologically contaminated %s juice.)",pl_fruit);
#else
    pline("(But in fact it was biologically contaminated orange juice.)");
#endif
		    if (pl_character[0] == 'H')
			pline("Fortunately, you have been immunized.");
		    else {
			int typ = rn2(A_MAX);
			poisontell(typ);
			adjattrib(typ,Poison_resistance ? -1 : -rn1(4,3), TRUE);
			if(!Poison_resistance)
				losehp(rnd(10)+5*!!(otmp->cursed),
				       "contaminated potion");
		    }
		}
		if(Hallucination) {
			You("are shocked back to your senses!");
			make_hallucinated(0L,FALSE);
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
		make_confused(HConfusion + rn1(7,16-8*bcsign(otmp)),FALSE);
		break;
	case POT_GAIN_ABILITY:
		if(otmp->cursed) {
		    pline("Ulch!  That potion tasted foul!");
		    unkn++;
		} else {
		    i = rn2(A_MAX);		/* start at a random point */
		    for(isdone = ii = 0; !isdone && ii < A_MAX; ii++) {
			adjattrib(i, 1, FALSE);
			/* only first found if not blessed */
			isdone = !(otmp->blessed);
			flags.botl = 1;
			if(++i >= A_MAX) i = 0;
		    }
		}
		break;
	case POT_SPEED:
		if(Wounded_legs && !otmp->cursed) {
			heal_legs();
			unkn++;
			break;
		}		/* and fall through */
#ifdef SPELLS
	case SPE_HASTE_SELF:
#endif
		if(!(Fast & TIMEOUT))
			You("are suddenly moving much faster.");
		else {
			Your("%s get new energy.",
				makeplural(body_part(LEG)));
			unkn++;
		}
		Fast += rn1(10,100+60*bcsign(otmp));
		break;
	case POT_BLINDNESS:
		if(Blind) nothing++;
		make_blinded(Blinded + rn1(200, 250-125*bcsign(otmp)), TRUE);
		break;
	case POT_GAIN_LEVEL:
		if (otmp->cursed) {
			unkn++;
			/* they went up a level */
			if(dlevel > 1 && dlevel <= MAXLEVEL) { 
				You("rise up, through the ceiling!");
				goto_level(dlevel-1, FALSE);
			} else You("have an uneasy feeling.");
			break;
		}
		pluslvl();
		if (otmp->blessed)
			/* blessed potions place you at a random spot in the
			 * middle of the new level instead of the low point
			 */
			u.uexp = rndexp();
		break;
	case POT_HEALING:
		You("begin to feel better.");
		healup(d(5,2) + 5 * bcsign(otmp),
		       1, !!(otmp->blessed), !(otmp->cursed));
		break;
	case POT_EXTRA_HEALING:
		You("feel much better.");
		healup(d(5,4) + 5 * bcsign(otmp),
		       2+3*!!(otmp->blessed), !(otmp->cursed), 1);
		make_hallucinated(0L,TRUE);
		break;
	case POT_LEVITATION:
#ifdef SPELLS
	case SPE_LEVITATION:
#endif
		if(!Levitation) {
			float_up();
			if (otmp->cursed) {
#ifdef STRONGHOLD
	if((u.ux != xupstair || u.uy != yupstair) &&
	   (!xupladder || u.ux != xupladder || u.uy != yupladder)) {
#else
	if(u.ux != xupstair || u.uy != yupstair) {
#endif /* STRONGHOLD /**/
					You("hit your %s on the ceiling.",
						body_part(HEAD));
					losehp(uarmh ? 1 : rnd(10),
						"collision with the ceiling");
				} else (void) doup();
			}
		} else
			nothing++;
		if (otmp->blessed) {
			char buf[BUFSZ];
			int lmoves = 0;

			makeknown(POT_LEVITATION);
			pline("How many moves do you wish to levitate for? [1-300] ");
			do {
				getlin(buf);
			} while (buf[0]=='\033' || !buf[0] ||
				(lmoves = atoi(buf)) < 1 || lmoves > 300);
			HLevitation += lmoves;
		} else HLevitation += rnd(150);
		u.uprops[LEVITATION].p_tofn = float_down;
		break;
#ifdef SPELLS
	case POT_GAIN_ENERGY:			/* M. Stephenson */
		{	register int	 num;
			if(otmp->cursed)
			    You("feel lackluster.");
			else
			    pline("Magical energies course through your body.");
			num = rnd(5) + 5 * otmp->blessed + 1;
			u.uenmax += (otmp->cursed) ? -num : num;
			u.uen += (otmp->cursed) ? -num : num;
			if(u.uenmax <= 0) u.uen = u.uenmax = 0;
			flags.botl = 1;
		}
		break;
#endif
	default:
		impossible("What a funny potion! (%u)", otmp->otyp);
		return(0);
	}
	return(-1);
}

void
healup(nhp, nxtra, curesick, cureblind)
	int	nhp, nxtra;
	register boolean curesick, cureblind;
{
#ifdef POLYSELF
	if (u.mtimedone && nhp) {
		u.mh += nhp;
		if (u.mh > u.mhmax) u.mh = (u.mhmax += nxtra);
	}
#endif
	if(nhp)	{
		u.uhp += nhp;
		if(u.uhp > u.uhpmax)	u.uhp = (u.uhpmax += nxtra);
	}
	if(cureblind)	make_blinded(0L,TRUE);
	if(curesick)	make_sick(0L,TRUE);
	flags.botl = 1;
	return;
}

void
strange_feeling(obj,txt)
register struct obj *obj;
register char *txt;
{
	if(flags.beginner)
		You("have a %s feeling for a moment, then it passes.",
		Hallucination ? "normal" : "strange");
	else
		pline(txt);

	if(!obj)	/* e.g., crystal ball finds no traps */
		return;

	if(obj->dknown && !objects[obj->otyp].oc_name_known &&
						!objects[obj->otyp].oc_uname)
		docall(obj);
	useup(obj);
}

const char *bottlenames[] = {
	"bottle", "phial", "flagon", "carafe", "flask", "jar", "vial"
};

void
potionhit(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	register char *botlnam = bottlenames[rn2(SIZE(bottlenames))];
	boolean uclose, isyou = (mon == &youmonst);

	if(isyou) {
		uclose = TRUE;
		pline("The %s crashes on your %s and breaks into shivers.",
			botlnam, body_part(HEAD));
		losehp(rnd(2), "thrown potion");
	} else {
		uclose = (dist(mon->mx,mon->my) < 3);
		if(Blind) pline("Crash!");
		else pline("The %s crashes on %s%s and breaks into shivers.",
			botlnam, mon_nam(mon), (haseyes(mon->data) &&
			mon->data != &mons[PM_FLOATING_EYE]) ?
#ifdef WORM
			(notonhead ? "'s body" : "'s head")
#else
			"'s head"
#endif
			: "");
		if(rn2(5) && mon->mhp > 1)
			mon->mhp--;
	}
#ifdef WORM
	notonhead = FALSE;
#endif
	pline("The %s evaporates.", xname(obj));

	if(!isyou) switch (obj->otyp) {

	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
	case POT_HEALING:
	case POT_EXTRA_HEALING:
		if(mon->mhp < mon->mhpmax) {
			mon->mhp = mon->mhpmax;
			if(!Blind)
			pline("%s looks sound and hale again.", Monnam(mon));
		}
		break;
	case POT_SICKNESS:
		if((mon->mhpmax > 3) && !resist(mon, POTION_SYM, 0, NOTELL))
			mon->mhpmax /= 2;
		if((mon->mhp > 2) && !resist(mon, POTION_SYM, 0, NOTELL))
			mon->mhp /= 2;
		if(!Blind)
		pline("%s looks rather ill.", Monnam(mon));
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!resist(mon, POTION_SYM, 0, NOTELL))  mon->mconf = 1;
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
		{
		    register int btmp = 64 + rn2(32) +
					rn2(32) * !resist(mon, POTION_SYM, 0, NOTELL);
		    mon->mblinded |= btmp;
		}
		break;
	case POT_WATER:
		if (is_undead(mon->data) || is_demon(mon->data)) {
			if (obj->blessed) {
				kludge("%s shrieks in pain!", Monnam(mon));
				mon->mhp -= d(2,6);
				if (mon->mhp <1) killed(mon);
			} else if (obj->cursed) {
				if(!Blind)
				pline("%s looks healthier.", Monnam(mon));
				mon->mhp += d(2,6);
			}
		}
		/* TO DO: Gremlins multiply when doused with water */
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
	if(uclose && rn2(5))
		potionbreathe(obj);
	obfree(obj, (struct obj *)0);
}

void
potionbreathe(obj)
register struct obj *obj;
{
	register int i, ii, isdone;

	switch(obj->otyp) {
	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
		if(obj->cursed) {
		    pline("Ulch!  That potion smells terrible!");
		    break;
		} else {
		    i = rn2(A_MAX);		/* start at a random point */
		    for(isdone = ii = 0; !isdone && ii < A_MAX; ii++) {
			if(ABASE(i) < AMAX(i)) {
			    ABASE(i)++;
			    /* only first found if not blessed */
			    isdone = !(obj->blessed);
			    flags.botl = 1;
			}
			if(++i >= A_MAX) i = 0;
		    }
		}
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
		You("have a vision for a moment.");
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!Confusion)
			You("feel somewhat dizzy.");
		make_confused(HConfusion + rnd(5),FALSE);
		break;
	case POT_INVISIBILITY:
		if (!See_invisible && !Invis)
			pline("For an instant you could see through yourself!");
		break;
	case POT_PARALYSIS:
		pline("Something seems to be holding you.");
		nomul(-rnd(5));
		break;
	case POT_SPEED:
		Fast += rnd(5);
		Your("knees seem more flexible now.");
		break;
	case POT_BLINDNESS:
		if(!Blind) pline("It suddenly gets dark.");
		make_blinded(Blinded + rnd(5),FALSE);
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

static int
neutralizes(o1, o2)
register struct obj *o1, *o2;
{
	switch (o1->otyp) {
		case POT_SICKNESS:
		case POT_HALLUCINATION:
		case POT_BLINDNESS:
		case POT_CONFUSION:
			if (o2->otyp == POT_HEALING ||
			    o2->otyp == POT_EXTRA_HEALING)
				return 1;
		case POT_HEALING:
		case POT_EXTRA_HEALING:
			if (o2->otyp == POT_SICKNESS ||
			    o2->otyp == POT_HALLUCINATION ||
			    o2->otyp == POT_BLINDNESS ||
			    o2->otyp == POT_CONFUSION)
				return 1;
	}

	return 0;
}

int
dodip()
{
	register struct obj *potion, *obj;
	char *tmp;

	if(!(obj = getobj("#", "dip")))
		return(0);
#ifdef FOUNTAINS
	/* Is there a fountain to dip into here? */
	if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		pline("Dip it into the fountain? ");
		if(yn() == 'y') {
			dipfountain(obj);
			return(1);
		}
	}
#endif
	if(!(potion = getobj(beverages, "dip into")))
		return(0);
	if (potion==obj && potion->quan==1) {
		pline("That is a potion bottle, not a Klein bottle!");
		return 0;
	}
	if(potion->otyp == POT_WATER) {
		if (potion->blessed) {
			if (obj->cursed) {
				if (!Blind)
				    Your("%s %s.",
					  aobjnam(obj, "softly glow"),
					  Hallucination ? hcolor() : amber);
				obj->cursed=0;
				obj->bknown=1;
	poof:
				if(!(objects[potion->otyp].oc_name_known) &&
				   !(objects[potion->otyp].oc_uname))
					docall(potion);
				useup(potion);
				return(1);
			} else if(!obj->blessed) {
				if (!Blind) {
				    tmp = Hallucination ? hcolor() : light_blue;
				    Your("%s with a%s %s aura.",
					  aobjnam(obj, "softly glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
				}
				obj->blessed=1;
				obj->bknown=1;
				goto poof;
			}
		} else if (potion->cursed) {
			if (obj->blessed) {
				if (!Blind)
				    Your("%s %s.", aobjnam(obj, "glow"),
					  Hallucination ? hcolor() : "brown");
				obj->blessed=0;
				obj->bknown=1;
				goto poof;
			} else if(!obj->cursed) {
				if (!Blind) {
				    tmp = Hallucination ? hcolor() : black;
				    Your("%s with a%s %s aura.",
					  aobjnam(obj, "glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
				}
				obj->cursed=1;
				obj->bknown=1;
				goto poof;
			}
		} else if (obj->otyp != POT_WATER) {
			if (obj->olet == WEAPON_SYM && !obj->rustfree &&
			    objects[obj->otyp].oc_material == METAL &&
			    obj->spe > -6 && !rn2(10)) {
				Your("%s somewhat.", aobjnam(obj,"rust"));
				obj->spe--;
				goto poof;
			} else if (obj->olet == POTION_SYM) {
				Your("%s.", aobjnam(obj,"dilute"));
				if (obj->spe == -1) {
					obj->spe = 0;
					obj->blessed = obj->cursed = 0;
					obj->otyp = POT_WATER;
				} else obj->spe--;
				goto poof;
			} else if (obj->olet == SCROLL_SYM &&
#ifdef MAIL
				   obj->otyp != SCR_MAIL &&
#endif
				   obj->otyp != SCR_BLANK_PAPER) {
				if (!Blind) {
					if (obj->quan == 1)
						pline("The scroll fades.");
					else pline("The scrolls fade.");
				}
				obj->otyp = SCR_BLANK_PAPER;
				goto poof;
			} else
				Your("%s wet.", aobjnam(obj,"get"));
		}
	}
	else if(obj->olet == POTION_SYM && obj->otyp != potion->otyp) {
		/* Mixing potions is dangerous... */
		pline("The potions mix...");
		if (obj->cursed || !rn2(10)) {
			pline("BOOM!  They explode!");
			u.uhp -= rnd(10);
			flags.botl = 1;
			potionbreathe(obj);
			useup(obj);
			useup(potion);
			return(1);
		}

		obj->blessed = obj->cursed = obj->bknown = 0;
		if (Blind) obj->dknown = 0;

		switch (neutralizes(obj, potion) || obj->spe == -1 ?
			1 : rnd(8)) {
			case 1:
				obj->otyp = POT_WATER;
				obj->blessed = obj->cursed = 0;
				break;
			case 2:
			case 3:
				obj->otyp = POT_SICKNESS;
				break;
			case 4:
				{
				  struct obj *otmp;
				  otmp = mkobj(POTION_SYM,FALSE);
				  obj->otyp = otmp->otyp;
				  obfree(otmp, (struct obj *)0);
				}
				break;
			default:
				if (!Blind)
			    pline("The mixture glows brightly and evaporates.");
				useup(obj);
				useup(potion);
				return(1);
		}

		obj->spe--; /* diluted */
		if (obj->otyp == POT_WATER)
			pline("The mixture bubbles violently, then clears.");
		else {
			if (!Blind) {
				pline("The mixture looks %s.", objects[obj->otyp].oc_descr);
				obj->dknown = 1;
			}
		}

		useup(potion);
		return(1);
	}

	if(obj->olet == WEAPON_SYM && obj->otyp <= SHURIKEN) {
	    if(potion->otyp == POT_SICKNESS && !obj->opoisoned) {
		char buf[BUFSZ];
		Strcpy(buf, xname(potion));
		pline("The %s form%s a coating on the %s.",
			buf, potion->quan==1 ? "s" : "", xname(obj));
		obj->opoisoned = 1;
		goto poof;
	    } else if(obj->opoisoned && 
		      (potion->otyp == POT_HEALING ||
		       potion->otyp == POT_EXTRA_HEALING)) {
		pline("A coating wears off the %s.", xname(obj));
		obj->opoisoned = 0;
		goto poof;
	    }
	}
	pline("Interesting...");
	return(1);
}


void
djinni_from_bottle(obj)
register struct obj *obj;
{
	register struct monst *mtmp;

	if(!(mtmp = makemon(&mons[PM_DJINNI], u.ux, u.uy))){
		pline("It turns out to be empty.");
		return;
	}

	if (!Blind) {
		pline("In a cloud of smoke, %s emerges!", defmonnam(mtmp));
		pline("%s speaks.", Monnam(mtmp));
	} else {
		You("smell acrid fumes.");
		pline("Something speaks.");
	}

	switch (obj->blessed ? 0 : obj->cursed ? 4 : rn2(5)) {
	case 0 : verbalize("I am in your debt.  I will grant one wish!");
		makewish();
		mongone(mtmp);
		break;
	case 1 : verbalize("Thank you for freeing me!");
		(void) tamedog(mtmp, (struct obj *)0);
		break;
	case 2 : verbalize("You freed me!");
		mtmp->mpeaceful = 1;
		break;
	case 3 : verbalize("It is about time!");
		pline("The %s vanishes.",
			Hallucination ? rndmonnam() : "djinni");
		mongone(mtmp);
		break;
	default: verbalize("You disturbed me, fool!");
		break;
	}
}

/* monster_detect is also used in the crystal ball routine */
/* returns 1 if nothing was detected		*/
/* returns 0 if something was detected		*/
int
monster_detect(otmp)
register struct obj	*otmp;
{
	register struct monst	*mtmp;

	if(!fmon) {
		if (otmp)
			strange_feeling(otmp, Hallucination ?
					      "You get the heebie jeebies." :
					      "You feel threatened.");
		return(1);
	} else {
		int woken = FALSE;

		cls();
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			if(mtmp->mx > 0)
		    at(mtmp->mx, mtmp->my,
		       (uchar)(Hallucination ? rndmonsym() : mtmp->data->mlet),
		       AT_MON);
			if (otmp && otmp->cursed && (mtmp->msleep || mtmp->mfroz)) {
				mtmp->msleep = mtmp->mfroz = 0;
				woken = TRUE;
			}
		}
		prme();
		You("sense the presence of monsters.");
		if (woken)
			pline("Monsters sense the presence of you.");
		more();
		docrt();
	}
	return(0);
}

/* object_detect is also used in the crystal ball routine */
/* returns 1 if nothing was detected		*/
/* returns 0 if something was detected		*/
int
object_detect(otmp)
register struct obj	*otmp;
{
	register struct obj	*objs;
	register struct monst	*mtmp;

	if(!fobj) {
		if (otmp)
			strange_feeling(otmp, "You feel a pull downward.");
		return(1);
	} else {
		int mfound=FALSE;

		for(objs = fobj; objs; objs = objs->nobj)
			if(objs->ox != u.ux || objs->oy != u.uy)
				goto outobjmap;
		You("sense the presence of objects nearby.");
		return(0);
	outobjmap:
		cls();
		for(objs = fobj; objs; objs = objs->nobj)
at(objs->ox, objs->oy, (uchar)(Hallucination ? rndobjsym() : objs->olet), AT_OBJ);
		/* monster possessions added by GAN 12/16/86 */
		for(mtmp = fmon ; mtmp ; mtmp = mtmp->nmon)
			if(mtmp->minvent)
				for(objs = mtmp->minvent;objs;objs = objs->nobj)
				    at(mtmp->mx, mtmp->my, (uchar)objs->olet, AT_OBJ);
		if (otmp && otmp->cursed) {
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				if (mtmp->mimic) {
					mnexto(mtmp);
					mfound = TRUE;
				}
			}
		}
		prme();
		You("sense the presence of objects.");
		if (mfound) pline("Objects sense the presence of you.");
		more();
		docrt();
	}
	return(0);
}

/* the detections are pulled out so they can	*/
/* also be used in the crystal ball routine	*/
/* returns 1 if nothing was detected		*/
/* returns 0 if something was detected		*/
int
trap_detect(sobj)
register struct obj	*sobj;
/* sobj is null if crystal ball, *scroll if gold detection scroll */
{
	register struct trap *ttmp;
	register struct obj *obj;
	register int door;
	boolean found = FALSE;
	coord cc;

	for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
		if(ttmp->tx != u.ux || ttmp->ty != u.uy)
			goto outtrapmap;
		else found = TRUE;
	}
	for(obj = fobj; obj; obj = obj->nobj) {
		if ((obj->otyp==LARGE_BOX || obj->otyp==CHEST) && obj->otrapped)
			if (obj->ox != u.ux || obj->oy != u.uy)
				goto outtrapmap;
			else found = TRUE;
	}
	for(door=0; door<=doorindex; door++) {
		cc = doors[door];
		if (levl[cc.x][cc.y].doormask & D_TRAPPED)
			if (cc.x != u.ux || cc.x != u.uy)
				goto outtrapmap;
			else found = TRUE;
	}
	if(!found) {
		char buf[42];
		Sprintf(buf, "Your %s stop itching.",
			makeplural(body_part(TOE)));
		strange_feeling(sobj,buf);
		return(1);
	}
	/* traps exist, but only under me - no separate display required */
	Your("%s itch.", makeplural(body_part(TOE)));
	return(0);
outtrapmap:
	cls();
#define SYMBOL (uchar)(Hallucination ? rndobjsym() : \
		(sobj && sobj->cursed) ? GOLD_SYM : TRAP_SYM)
#define AT Hallucination || (sobj && sobj->cursed) ? AT_OBJ : AT_MAP
	for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
		at(ttmp->tx, ttmp->ty, SYMBOL, AT);
	for(obj = fobj; obj; obj = obj->nobj) {
		if ((obj->otyp==LARGE_BOX || obj->otyp==CHEST) && obj->otrapped)
			at(obj->ox, obj->oy, SYMBOL, AT);
	}
	for(door=0; door<=doorindex; door++) {
		cc = doors[door];
		if (levl[cc.x][cc.y].doormask & D_TRAPPED)
			at(cc.x, cc.y, SYMBOL, AT);
	}
#undef SYMBOL
#undef AT
	prme();
	if (sobj && sobj->cursed)
		You("feel very greedy.");
	else
		You("feel entrapped.");
	more();
	docrt();
	return(0);
}
