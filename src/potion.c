/*	SCCS Id: @(#)potion.c	3.1	93/02/06		  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVLB
static void NDECL(ghost_from_bottle);
static boolean FDECL(neutralizes, (struct obj *,struct obj *));

static NEARDATA int nothing, unkn;
#endif /* OVLB */

extern boolean notonhead;	/* for long worms */
#ifdef OVLB
boolean notonhead = FALSE;
#endif /* OVLB */

#ifdef OVLB

static NEARDATA const char beverages[] = { POTION_CLASS, 0 };

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
			You("stagger....");
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
make_vomiting(xtime, talk)
long xtime;
boolean talk;
{
	long old = Vomiting;

	if(!xtime && old)
	    if(talk) You("feel much less nauseous now.");

	Vomiting = xtime;
}


void
make_blinded(xtime, talk)
long xtime;
boolean talk;
{
	long old = Blinded;
	boolean changed = 0;

	if (u.usleep) talk = FALSE;

	if (!xtime && old && !Blindfolded
#ifdef POLYSELF
	    && haseyes(uasmon)
#endif
	    ) {
	    if (talk) {
		if (Hallucination)
		    pline("Far out!  Everything is all cosmic again!");
		else		   You("can see again.");
	    }
	    changed = TRUE;
	}
	if (xtime && !old && !Blindfolded
#ifdef POLYSELF
	    && haseyes(uasmon)
#endif
	    ) {
	    if (talk) {
		if (Hallucination) pline("Oh, bummer!  Everything is dark!  Help!");
		else		   pline("A cloud of darkness falls upon you.");
	    }
	    changed = TRUE;

	    /* Before the hero goes blind, set the ball&chain variables. */
	    if (Punished) set_bc(0);
	}
	Blinded = xtime;
	if (changed) {
	    flags.botl = 1;
	    vision_full_recalc = 1;
	    if (Telepat) see_monsters();
	}
}

void
make_hallucinated(xtime, talk, mask)
long xtime;	/* nonzero if this is an attempt to turn on hallucination */
boolean talk;
long mask;	/* nonzero if resistance status should change by mask */
{
	boolean changed = 0;
#ifdef LINT
	const char *message = 0;
#else
	const char *message;
#endif

	if (!xtime)
	    message = "Everything looks SO boring now.";
	else
	    message = "Oh wow!  Everything seems so cosmic!";

	if (mask) {
	    if (HHallucination) changed = TRUE;

	    if (!xtime) HHalluc_resistance |= mask;
	    else HHalluc_resistance &= ~mask;
	} else {
	    if (!HHalluc_resistance && (!!HHallucination != !!xtime))
		changed = TRUE;
	    HHallucination = xtime;
	}

	if (changed) {
	    if (u.uswallow) {
		swallowed(0);	/* redraw swallow display */
	    } else {
		/* The see_* routines should be called *before* the pline. */
		see_monsters();
		see_objects();
	    }
	    flags.botl = 1;
	    if (!Blind && talk) pline(message);
	}
}

static void
ghost_from_bottle()
{
	struct monst *mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy);

	if (!mtmp) {
		pline("This bottle turns out to be empty.");
		return;
	}
	if (Blind) {
		pline("As you open the bottle, something emerges.");
		return;
	}
	pline("As you open the bottle, an enormous %s emerges!",
		Hallucination ? rndmonnam() : (const char *)"ghost");
	if(flags.verbose)
	    You("are frightened to death, and unable to move.");
	nomul(-3);
	nomovemsg = "You regain your composure.";
}

int
dodrink() {
	register struct obj *otmp;
	const char *potion_descr;

	if (Strangled) {
		pline("If you can't breathe air, how can you drink liquid?");
		return 0;
	}
	/* Is there a fountain to drink from here? */
	if (IS_FOUNTAIN(levl[u.ux][u.uy].typ) && !Levitation) {
		if(yn("Drink from the fountain?") == 'y') {
			drinkfountain();
			return 1;
		}
	}
#ifdef SINKS
	/* Or a kitchen sink? */
	if (IS_SINK(levl[u.ux][u.uy].typ)) {
		if (yn("Drink from the sink?") == 'y') {
			drinksink();
			return 1;
		}
	}
#endif

	otmp = getobj(beverages, "drink");
	if(!otmp) return(0);
#ifndef NO_SIGNAL
	otmp->in_use = TRUE;		/* you've opened the stopper */
#endif
	potion_descr = OBJ_DESCR(objects[otmp->otyp]);
	if (potion_descr && !strcmp(potion_descr, "milky") && !rn2(13)) {
		ghost_from_bottle();
		useup(otmp);
		return(1);
	} else if (potion_descr && !strcmp(potion_descr, "smoky") && !rn2(13)) {
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
	case SPE_RESTORE_ABILITY:
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
		    if((ABASE(A_STR) == AMAX(A_STR)) && (u.uhs >= 3))
			losestr(1);		/* kludge - mrs */
		}
		break;
	case POT_HALLUCINATION:
		if (Hallucination || HHalluc_resistance) nothing++;
		make_hallucinated(HHallucination +
			  rn1(200, 600 - 300*bcsign(otmp)), TRUE, 0L);
		break;
	case POT_WATER:
		if(!otmp->blessed && !otmp->cursed) {
			pline("This tastes like %swater.",
			      otmp->odiluted ? "impure " : "");
			lesshungry(rnd(otmp->odiluted ? 3 : 10));
			break;
		}
		unkn++;
		if(
#ifdef POLYSELF
		   is_undead(uasmon) || is_demon(uasmon) ||
#endif
				u.ualign.type == A_CHAOTIC) {
		    if(otmp->blessed) {
			pline("This burns like acid!");
			exercise(A_CON, FALSE);
#ifdef POLYSELF
			if (u.ulycn != -1) {
				Your("affinity to %s disappears!",
				     makeplural(mons[u.ulycn].mname));
				if(uasmon == &mons[u.ulycn] && !Polymorph_control)
					rehumanize();
				u.ulycn = -1;
			}
#endif
			losehp(d(2,6), "potion of holy water", KILLED_BY_AN);
		    } else if(otmp->cursed) {
			You("feel quite proud of yourself.");
			healup(d(2,6),0,0,0);
			exercise(A_CON, TRUE);
		    }
		} else
		    if(otmp->blessed) {
			You("feel full of awe.");
			make_sick(0L,TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_CON, TRUE);
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
			if(u.ualign.type == A_LAWFUL) {
			    pline("This burns like acid!");
			    losehp(d(2,6), "potion of unholy water",
				KILLED_BY_AN);
			} else
			    You("feel full of dread.");
			exercise(A_CON, FALSE);
		    }
		break;
	case POT_BOOZE:
		unkn++;
		pline("Ooph!  This tastes like %s%s!",
		      otmp->odiluted ? "watered down " : "",
		      Hallucination ? "furniture polish" : "liquid fire");
		if (!otmp->blessed) make_confused(HConfusion + d(3,8),FALSE);
		/* the whiskey makes us feel better */
		if (u.uhp < u.uhpmax && !otmp->odiluted)
			losehp(-1, "", 0); /* can't kill you */
		lesshungry(10 * (2 + bcsign(otmp)));
		exercise(A_WIS, FALSE);
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
			if (otmp->blessed) {
				(void) adjattrib(A_INT, 1, FALSE);
				(void) adjattrib(A_WIS, 1, FALSE);
			}
			You("feel self-knowledgeable...");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(FALSE);
			pline("The feeling subsides.");
		}
		exercise(A_WIS, !otmp->cursed);
		break;
	case POT_INVISIBILITY:
	case SPE_INVISIBILITY:
		if(Invisible || See_invisible) nothing++;
		else {
		     if(!Blind)
		       pline(Hallucination ?
			 "Far out, man!  You can see right through yourself!" :
			 "Gee!  All of a sudden, you can't see yourself.");
		     else
		       You("feel rather airy."), unkn++;
		}
		if (otmp->blessed && !(HInvis & FROMOUTSIDE)) {
			nothing = 0;
			if(yn("Do you want the invisibility to be permanent?")
			    == 'n')
				HInvis += rn1(15,31);
			else HInvis |= FROMOUTSIDE;
		} else HInvis += rn1(15,31);
		newsym(u.ux,u.uy);	/* update position */
		if(otmp->cursed) {
		    pline("For some reason, you feel your presence is known.");
		    aggravate();
		}
		break;
	case POT_SEE_INVISIBLE:
		/* tastes like fruit juice in Rogue */
	case POT_FRUIT_JUICE:
		unkn++;
		if(otmp->cursed)
			pline("Yecch!  This tastes %s.",
			  Hallucination ? "overripe" : "rotten"
			 );
		else pline (Hallucination ?
#ifdef TUTTI_FRUTTI
		   "This tastes like 10%% real %s juice all-natural beverage." :
		   "This tastes like %s%s juice.",
		   otmp->odiluted ? "reconstituted " : "", pl_fruit
#else
		   "This tastes like 10%% real fruit juice all-natural beverage." :
		   "This tastes like %sfruit juice.",
		   otmp->odiluted ? "reconstituted " : ""
#endif
			    );
		if (otmp->otyp == POT_FRUIT_JUICE) {
			lesshungry((otmp->odiluted ? 5 : 10) * (2 + bcsign(otmp)));
			break;
		}
		if (!otmp->cursed) {
			/* Tell them they can see again immediately, which
			 * will help them identify the potion...
			 */
			make_blinded(0L,TRUE);
		}
		if (otmp->blessed)
			HSee_invisible |= FROMOUTSIDE;
		else
			HSee_invisible += rn1(100,750);
		set_mimic_blocking(); /* do special mimic handling */
		see_monsters();	/* see invisible monsters */
		newsym(u.ux,u.uy); /* see yourself! */
		break;
	case POT_PARALYSIS:
		if(Levitation || Is_waterlevel(&u.uz))
			You("are motionlessly suspended.");
		else
			Your("%s are frozen to the floor!",
				makeplural(body_part(FOOT)));
		nomul(-(rn1(10, 25 - 12*bcsign(otmp))));
		exercise(A_DEX, FALSE);
		break;
	case POT_MONSTER_DETECTION:
	case SPE_DETECT_MONSTERS:
		if (monster_detect(otmp, 0))
			return(1);		/* nothing detected */
		exercise(A_WIS, TRUE);
		break;
	case POT_OBJECT_DETECTION:
	case SPE_DETECT_TREASURE:
		if (object_detect(otmp, 0))
			return(1);		/* nothing detected */
		exercise(A_WIS, TRUE);
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
				losehp(1, "mildly contaminated potion",
					KILLED_BY_AN);
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
			(void) adjattrib(typ,
					Poison_resistance ? -1 : -rn1(4,3),
					TRUE);
			if(!Poison_resistance)
				losehp(rnd(10)+5*!!(otmp->cursed),
				       "contaminated potion", KILLED_BY_AN);
			exercise(A_CON, FALSE);
		    }
		}
		if(Hallucination) {
			You("are shocked back to your senses!");
			make_hallucinated(0L,FALSE,0L);
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
		} else {      /* If blessed, increase all; if not, try up to */
		    int itmp; /* 6 times to find one which can be increased. */
		    i = -1;		/* increment to 0 */
		    for (ii = A_MAX; ii > 0; ii--) {
			i = (otmp->blessed ? i + 1 : rn2(A_MAX));
			/* only give "your X is already as high as it can get"
			   message on last attempt (except blessed potions) */
			itmp = (otmp->blessed || ii == 1) ? 0 : -1;
			if (adjattrib(i, 1, itmp) && !otmp->blessed)
			    break;
		    }
		}
		break;
	case POT_SPEED:
		if(Wounded_legs && !otmp->cursed) {
			heal_legs();
			unkn++;
			break;
		}		/* and fall through */
	case SPE_HASTE_SELF:
		if(!(Fast & ~INTRINSIC)) /* wwf@doe.carleton.ca */
			You("are suddenly moving %sfaster.",
				Fast ? "" : "much ");
		else {
			Your("%s get new energy.",
				makeplural(body_part(LEG)));
			unkn++;
		}
		exercise(A_DEX, TRUE);
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
			if((ledger_no(&u.uz) == 1 && u.uhave.amulet) ||
						      Can_rise_up(&u.uz)) {
			    const char *riseup = "rise up, through the ceiling!";
			    if(ledger_no(&u.uz) == 1) {
			        You(riseup);
				goto_level(&earth_level, FALSE, FALSE, FALSE);
			    } else {
			        register int newlev = depth(&u.uz)-1;
				d_level newlevel;

				get_level(&newlevel, newlev);
				if(on_level(&newlevel, &u.uz)) {
				    pline("It tasted bad.");
				    break;
				} else You(riseup);
				goto_level(&newlevel, FALSE, FALSE, FALSE);
			    }
			}
			else You("have an uneasy feeling.");
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
		exercise(A_STR, TRUE);
		break;
	case POT_EXTRA_HEALING:
		You("feel much better.");
		healup(d(5,4) + 5 * bcsign(otmp),
		       2+3*!!(otmp->blessed), !(otmp->cursed), 1);
		make_hallucinated(0L,TRUE,0L);
		exercise(A_STR, TRUE);
		exercise(A_CON, TRUE);
		break;
	case POT_LEVITATION:
	case SPE_LEVITATION:
		if(!Levitation) {
			/* kludge to ensure proper operation of float_up() */
			HLevitation = 1;
			float_up();
			/* reverse kludge */
			HLevitation = 0;
			if (otmp->cursed && !Is_waterlevel(&u.uz)) {
	if((u.ux != xupstair || u.uy != yupstair)
	   && (u.ux != sstairs.sx || u.uy != sstairs.sy || !sstairs.up)
	   && (!xupladder || u.ux != xupladder || u.uy != yupladder)
	) {
					You("hit your %s on the ceiling.",
						body_part(HEAD));
					losehp(uarmh ? 1 : rnd(10),
						"colliding with the ceiling",
						KILLED_BY);
				} else (void) doup();
			}
		} else
			nothing++;
		if (otmp->blessed) {
			char buf[BUFSZ];
			int lmoves;

			makeknown(POT_LEVITATION);
			do {
	getlin("How many moves do you wish to levitate for? [1-300]", buf);
			    lmoves = (!*buf || *buf=='\033') ? 0 : atoi(buf);
			} while (lmoves < 1 || lmoves > 300);
			HLevitation += lmoves;
		} else HLevitation += rnd(150);
		u.uprops[LEVITATION].p_tofn = float_down;
		break;
	case POT_GAIN_ENERGY:			/* M. Stephenson */
		{	register int	 num;
			if(otmp->cursed)
			    You("feel lackluster.");
			else
			    pline("Magical energies course through your body.");
			num = rnd(5) + 5 * otmp->blessed + 1;
			u.uenmax += (otmp->cursed) ? -num : num;
			u.uen += (otmp->cursed) ? -num : num;
			if(u.uenmax <= 0) u.uenmax = 0;
			if(u.uen <= 0) u.uen = 0;
			flags.botl = 1;
			exercise(A_WIS, TRUE);
		}
		break;
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
register const char *txt;
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
	register const char *botlnam = bottlenames[rn2(SIZE(bottlenames))];
	boolean distance, isyou = (mon == &youmonst);

	if(isyou) {
		distance = 0;
		pline("The %s crashes on your %s and breaks into shards.",
			botlnam, body_part(HEAD));
		losehp(rnd(2), "thrown potion", KILLED_BY_AN);
	} else {
		distance = distu(mon->mx,mon->my);
		if (!cansee(mon->mx,mon->my)) pline("Crash!");
		else {
		    char *mnam = mon_nam(mon);
		    char buf[BUFSZ];

		    if(has_head(mon->data)) {
			Sprintf(buf, "%s %s",
				s_suffix(mnam),
				(notonhead ? "body" : "head"));
		    } else {
			Strcpy(buf, mnam);
		    }
		    pline("The %s crashes on %s and breaks into shards.",
			   botlnam, buf);
		}
		if(rn2(5) && mon->mhp > 1)
			mon->mhp--;
	}

	if (cansee(mon->mx,mon->my))
		pline("%s evaporates.", The(xname(obj)));

	if (!isyou) switch (obj->otyp) {

	case POT_RESTORE_ABILITY:
	case POT_GAIN_ABILITY:
	case POT_HEALING:
	case POT_EXTRA_HEALING:
		if(mon->mhp < mon->mhpmax) {
		    mon->mhp = mon->mhpmax;
		    if (canseemon(mon))
			pline("%s looks sound and hale again.", Monnam(mon));
		}
		break;
	case POT_SICKNESS:
		if((mon->mhpmax > 3) && !resist(mon, POTION_CLASS, 0, NOTELL))
			mon->mhpmax /= 2;
		if((mon->mhp > 2) && !resist(mon, POTION_CLASS, 0, NOTELL))
			mon->mhp /= 2;
		if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
		if (canseemon(mon))
		    pline("%s looks rather ill.", Monnam(mon));
		break;
	case POT_CONFUSION:
	case POT_BOOZE:
		if(!resist(mon, POTION_CLASS, 0, NOTELL))  mon->mconf = TRUE;
		break;
	case POT_INVISIBILITY:
		mon->minvis = TRUE;
		newsym(mon->mx,mon->my);
		break;
	case POT_PARALYSIS:
		if (mon->mcanmove) {
			mon->mcanmove = 0;
			/* really should be rnd(5) for consistency with players
			 * breathing potions, but...
			 */
			mon->mfrozen = rnd(25);
		}
		break;
	case POT_SPEED:
		if (mon->mspeed == MSLOW) mon->mspeed = 0;
		else mon->mspeed = MFAST;
		break;
	case POT_BLINDNESS:
		if(haseyes(mon->data)) {
		    register int btmp = 64 + rn2(32) +
			rn2(32) * !resist(mon, POTION_CLASS, 0, NOTELL);
		    mon->mblinded |= btmp;
		    mon->mcansee = 0;
		}
		break;
	case POT_WATER:
		if (is_undead(mon->data) || is_demon(mon->data)) {
			if (obj->blessed) {
				pline("%s shrieks in pain!", Monnam(mon));
				mon->mhp -= d(2,6);
				if (mon->mhp <1) killed(mon);
			} else if (obj->cursed) {
				if (canseemon(mon))
				    pline("%s looks healthier.", Monnam(mon));
				mon->mhp += d(2,6);
				if (mon->mhp > mon->mhpmax)
					mon->mhp = mon->mhpmax;
			}
		} else if(mon->data == &mons[PM_GREMLIN]) {
			struct monst *mtmp2 = clone_mon(mon);

			if (mtmp2) {
				mtmp2->mhpmax = (mon->mhpmax /= 2);
				if (canseemon(mon))
					pline("%s multiplies.", Monnam(mon));
			}
		}
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
	/* Note: potionbreathe() does its own docall() */
	if (distance==0 || ((distance < 3) && rn2(5)))
		potionbreathe(obj);
	else if (obj->dknown && !objects[obj->otyp].oc_name_known &&
		   !objects[obj->otyp].oc_uname && cansee(mon->mx,mon->my))
		docall(obj);
	if(*u.ushops && obj->unpaid) {
	        register struct monst *shkp =
			shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));

		if(!shkp)
		    obj->unpaid = 0;
		else {
		    (void)stolen_value(obj, u.ux, u.uy,
				 (boolean)shkp->mpeaceful, FALSE);
		    subfrombill(obj, shkp);
		}
	}
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
		exercise(A_STR, TRUE);
		break;
	case POT_SICKNESS:
		if (pl_character[0] != 'H') {
			if(u.uhp <= 5) u.uhp = 1; else u.uhp -= 5;
			flags.botl = 1;
			exercise(A_CON, FALSE);
		}
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
		exercise(A_DEX, FALSE);
		break;
	case POT_SPEED:
		Fast += rnd(5);
		Your("knees seem more flexible now.");
		exercise(A_DEX, TRUE);
		break;
	case POT_BLINDNESS:
		if (!Blind && !u.usleep) pline("It suddenly gets dark.");
		make_blinded(Blinded + rnd(5),FALSE);
		break;
	case POT_WATER:
#ifdef POLYSELF
		if(u.umonnum == PM_GREMLIN) {
		    struct monst *mtmp;
		    if(mtmp = cloneu()) {
			mtmp->mhpmax = (u.mhmax /= 2);
			You("multiply.");
		    }
		}
#endif
/*
	case POT_GAIN_LEVEL:
	case POT_LEVITATION:
	case POT_FRUIT_JUICE:
	case POT_MONSTER_DETECTION:
	case POT_OBJECT_DETECTION:
*/
		break;
	}
	/* note: no obfree() */
	if (obj->dknown && !objects[obj->otyp].oc_name_known &&
						!objects[obj->otyp].oc_uname)
		docall(obj);
}

static boolean
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
				return TRUE;
		case POT_HEALING:
		case POT_EXTRA_HEALING:
		case UNICORN_HORN:
			if (o2->otyp == POT_SICKNESS ||
			    o2->otyp == POT_HALLUCINATION ||
			    o2->otyp == POT_BLINDNESS ||
			    o2->otyp == POT_CONFUSION)
				return TRUE;
	}

	return FALSE;
}

boolean
get_wet(obj)
register struct obj *obj;
/* returns TRUE if something happened (potion should be used up) */
{
        if(snuff_lit(obj)) return(TRUE);

	if (obj->greased) {
		grease_protect(obj,NULL,FALSE);
		return(FALSE);
	}
	switch (obj->oclass) {
	    case WEAPON_CLASS:
		if (!obj->oerodeproof && is_rustprone(obj) &&
		    (obj->oeroded < MAX_ERODE) && !rn2(10)) {
			Your("%s some%s.", aobjnam(obj, "rust"),
			     obj->oeroded ? " more" : "what");
			obj->oeroded++;
			return TRUE;
		} else break;
	    case POTION_CLASS:
		if (obj->otyp == POT_WATER) return FALSE;
		Your("%s.", aobjnam(obj,"dilute"));
		if (obj->odiluted) {
			obj->odiluted = 0;
			obj->blessed = obj->cursed = FALSE;
			obj->otyp = POT_WATER;
		} else obj->odiluted++;
		return TRUE;
	    case SCROLL_CLASS:
		if (obj->otyp != SCR_BLANK_PAPER
#ifdef MAIL
		    && obj->otyp != SCR_MAIL
#endif
		    ) {
			if (!Blind) {
				boolean oq1 = obj->quan == 1L;
				pline("The scroll%s fade%s.",
					oq1 ? "" : "s",
					oq1 ? "s" : "");
			}
			if(obj->unpaid) {
			    subfrombill(obj, shop_keeper(*u.ushops));
			    You("erase it, you pay for it.");
			    bill_dummy_object(obj);
			}
			obj->otyp = SCR_BLANK_PAPER;
			return TRUE;
		} else break;
	    case SPBOOK_CLASS:
		if (obj->otyp != SPE_BLANK_PAPER) {

			if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
	pline("%s suddenly heats up; steam rises and it remains dry.",
				The(xname(obj)));
			} else {
			    if (!Blind) {
				    boolean oq1 = obj->quan == 1L;
				    pline("The spellbook%s fade%s.",
					oq1 ? "" : "s", oq1 ? "s" : "");
			    }
			    if(obj->unpaid) {
			        subfrombill(obj, shop_keeper(*u.ushops));
			        You("erase it, you pay for it.");
			        bill_dummy_object(obj);
			    }
			    obj->otyp = SPE_BLANK_PAPER;
			}
			return TRUE;
		}
	}
	Your("%s wet.", aobjnam(obj,"get"));
	return FALSE;
}

int
dodip()
{
	register struct obj *potion, *obj;
	const char *tmp;
	uchar here;
	char allow_all[2];
	char c;

	allow_all[0] = ALL_CLASSES; allow_all[1] = '\0';
	if(!(obj = getobj(allow_all, "dip")))
		return(0);

	here = levl[u.ux][u.uy].typ;
	/* Is there a fountain to dip into here? */
	if (IS_FOUNTAIN(here)) {
		if(yn("Dip it into the fountain?") == 'y') {
			dipfountain(obj);
			return(1);
		}
	}
        if (is_pool(u.ux,u.uy)) {
		c = (here == POOL) ? yn("Dip it into the pool?")
				   : yn("Dip it into the moat?");
		if(c == 'y') {
			(void) get_wet(obj);
			return(1);
		}
	}

	if(!(potion = getobj(beverages, "dip into")))
		return(0);
	if (potion == obj && potion->quan == 1L) {
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
				uncurse(obj);
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
				bless(obj);
				obj->bknown=1;
				goto poof;
			}
		} else if (potion->cursed) {
			if (obj->blessed) {
				if (!Blind)
				    Your("%s %s.", aobjnam(obj, "glow"),
				     Hallucination ? hcolor() : (const char *)"brown");
				unbless(obj);
				obj->bknown=1;
				goto poof;
			} else if(!obj->cursed) {
				if (!Blind) {
				    tmp = Hallucination ? hcolor() : Black;
				    Your("%s with a%s %s aura.",
					  aobjnam(obj, "glow"),
					  index(vowels, *tmp) ? "n" : "", tmp);
				}
				curse(obj);
				obj->bknown=1;
				goto poof;
			}
		} else
			if (get_wet(obj))
			    goto poof;
	}
	else if(obj->oclass == POTION_CLASS && obj->otyp != potion->otyp) {
		/* Mixing potions is dangerous... */
		pline("The potions mix...");
		if (obj->cursed || !rn2(10)) {
			pline("BOOM!  They explode!");
			exercise(A_STR, FALSE);
			potionbreathe(obj);
			useup(obj);
			useup(potion);
			losehp(rnd(10), "alchemic blast", KILLED_BY_AN);
			return(1);
		}

		obj->blessed = obj->cursed = obj->bknown = 0;
		if (Blind) obj->dknown = 0;

		switch (neutralizes(obj, potion) ||
			obj->odiluted ? 1 : rnd(8)) {
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
				  otmp = mkobj(POTION_CLASS,FALSE);
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

		if (obj->otyp == POT_WATER) {
			obj->odiluted = 0; /* in case it was diluted before */
			pline("The mixture bubbles violently%s.",
				Blind ? "" : ", then clears");
		} else {
			obj->odiluted++;
			if (!Blind) {
				pline("The mixture looks %s.",
					OBJ_DESCR(objects[obj->otyp]));
				obj->dknown = TRUE;
			}
		}

		useup(potion);
		return(1);
	}

	if(obj->oclass == WEAPON_CLASS && obj->otyp <= SHURIKEN) {
	    if(potion->otyp == POT_SICKNESS && !obj->opoisoned) {
		char buf[BUFSZ];
		Strcpy(buf, The(xname(potion)));
		pline("%s form%s a coating on %s.",
			buf, potion->quan == 1L ? "s" : "", the(xname(obj)));
		obj->opoisoned = TRUE;
		goto poof;
	    } else if(obj->opoisoned &&
		      (potion->otyp == POT_HEALING ||
		       potion->otyp == POT_EXTRA_HEALING)) {
		pline("A coating wears off %s.", the(xname(obj)));
		obj->opoisoned = 0;
		goto poof;
	    }
	}

	if(obj->otyp == UNICORN_HORN && neutralizes(obj, potion)) {
		/* with multiple merged potions, we should split off one and
		   just clear it, but clearing them all together is easier */
		boolean more_than_one = potion->quan > 1L;
		pline("The potion%s clear%s.",
			more_than_one ? "s" : "",
			more_than_one ? "" : "s");
		potion->otyp = POT_WATER;
		potion->blessed = 0;
		potion->cursed = 0;
		potion->odiluted = 0;
		return(1);
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
		pline("In a cloud of smoke, %s emerges!", a_monnam(mtmp));
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
		mtmp->mpeaceful = TRUE;
		set_malign(mtmp);
		break;
	case 3 : verbalize("It is about time!");
		pline("%s vanishes.", Monnam(mtmp));
		mongone(mtmp);
		break;
	default: verbalize("You disturbed me, fool!");
		break;
	}
}

#endif /* OVLB */

/*potion.c*/
