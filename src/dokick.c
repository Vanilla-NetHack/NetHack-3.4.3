/*	SCCS Id: @(#)dokick.c	3.0	89/6/9
/* Copyright (c) Izchak Miller, Mike Stephenson, Steve Linhart, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#include	"eshk.h"

#define martial()	((pl_character[0] == 'S' || pl_character[0] == 'P'))

#ifdef KICK

# ifdef WORM
extern boolean notonhead;
# endif

static void
kickdmg(mon, clumsy)
register struct monst *mon;
register boolean clumsy;
{
	register int mdx, mdy;
	register int dmg = (((uarmg && 
				uarmg->otyp == GAUNTLETS_OF_POWER) ? 
				25 : ACURR(A_STR) > 18 ? 18 : ACURR(A_STR))+
				ACURR(A_DEX)+ACURR(A_CON))/15;

	/* excessive wt affects dex, so it affects dmg */
	if(clumsy) dmg = dmg/2;

	/* kicking a dragon or an elephant will not harm it */
	if(thick_skinned(mon->data)) dmg = 0;
	

	/* squeeze some guilt feelings... */
	if(mon->mtame) {
# ifdef SOUNDS
	    if (rn2(10)) yelp(mon);
	    else growl(mon); /* give them a moment's worry */
# endif
	    mon->mtame--;
	    mon->mflee = mon->mtame ? 1 : 0;
# ifdef HISX
	    mon->mfleetim = mon->mfleetim + (dmg ? rnd(dmg) : 1);
# else
	    mon->mfleetim += (dmg ? rnd(dmg) : 1);
# endif
	}
	
	if (dmg)
		mon->mhp -= (!martial() ? rnd(dmg) :
			rnd(dmg)+rnd(ACURR(A_DEX)/2));  
	if(mon->mhp < 1) {
		(void) passive(mon, TRUE, 0, TRUE);
		killed(mon);
		return;
	}
	if(martial() && !bigmonst(mon->data) && !rn2(3) && !mon->mfroz) {
	    	/* see if the monster has a place to move into */
	    	mdx = mon->mx + u.dx;
	    	mdy = mon->my + u.dy;
	    	if(goodpos(mdx, mdy, mon->data)) {
			kludge("%s reels from the blow.", Monnam(mon));
			levl[mon->mx][mon->my].mmask = 0;
			levl[mdx][mdy].mmask = 1;
			mon->mx = mdx;
			mon->my = mdy;
			pmon(mon);
			set_apparxy(mon);
	    	}
	}
	(void) passive(mon, FALSE, 1, TRUE);

/*	it is unchivalrous to attack the defenseless or from behind */
	if (pl_character[0] == 'K' && u.ualigntyp == U_LAWFUL && 
		u.ualign > -10 && (mon->mfroz || mon->msleep || mon->mflee))
	    	adjalign(-1);

}

static void
kick_monster(x, y)
register int x, y;
{
	register boolean clumsy = FALSE;
	register struct monst *mon = m_at(x, y);
	register int i, j;

	if(special_case(mon)) return;
	setmangry(mon);
#ifdef POLYSELF
	/* Kick attacks by kicking monsters are normal attacks, not special.
	 * If you have >1 kick attack, you get all of them.
	 */
	if (attacktype(uasmon, AT_KICK)) {
	    schar tmp = find_roll_to_hit(mon);
	    for(i=0; i<NATTK; i++) {
		int sum = 0;
		if (uasmon->mattk[i].aatyp == AT_KICK && multi >= 0) {
		    /* check multi; maybe they had 2 kicks and the first */
		    /* was a kick against a floating eye */
		    j = 1;
		    if (tmp > rnd(20)) {
			kludge("You kick %s.", mon_nam(mon));
			sum = damageum(mon, &(uasmon->mattk[i]));
			if (sum == 2)
				(void)passive(mon, 1, 0, TRUE);
			else (void)passive(mon, sum, 1, TRUE);
		    } else {
			missum(mon, &(uasmon->mattk[i]));
			(void)passive(mon, 0, 1, TRUE);
		    }
		}
	    }
	    return;
	}
#endif

	/* no need to check POLYSELF since only ghosts, which you can't turn */
	/* into, are noncorporeal */
	if(noncorporeal(mon->data)) {
		Your("kick passes through!");
		return;
	}

	if(Levitation && !rn2(3) && verysmall(mon->data) &&
	   !is_flyer(mon->data)) {
		pline("Floating in the air, you miss wildly!");
		(void) passive(mon, FALSE, 1, TRUE);
		return;
	}

	i = abs(inv_weight());
	j = weight_cap();

	if(i < (j*3)/10) {
		if(!rn2((i < j/10) ? 2 : (i < j/5) ? 3 : 4)) {
			if(martial() && !rn2(2)) goto doit;
			Your("clumsy kick does no damage.");
			(void) passive(mon, FALSE, 1, TRUE);
			return;
		}
		if(i < j/10) clumsy = TRUE;
		else if(!rn2((i < j/5) ? 2 : 3)) clumsy = TRUE;  
	} 

	if(Fumbling) clumsy = TRUE;

	else if(uarm && objects[uarm->otyp].oc_bulky && ACURR(A_DEX) < rnd(25))
		clumsy = TRUE;
doit:
	kludge("You kick %s.", mon_nam(mon));
	if(!rn2(clumsy ? 3 : 4) && (clumsy || !bigmonst(mon->data)) && 
	   mon->mcansee && !mon->mtrapped && !thick_skinned(mon->data) && 
	   mon->data->mlet != S_EEL && haseyes(mon->data) && !mon->mfroz && 
	   !mon->mstun && !mon->mconf && mon->data->mmove >= 12) {
		if(!nohands(mon->data) && !rn2(martial() ? 5 : 3)) {
		    kludge("%s blocks your %skick.", Monnam(mon), 
				clumsy ? "clumsy " : "");
		    (void) passive(mon, FALSE, 1, TRUE);
		    return;
		} else {
		    mnexto(mon);
		    if(mon->mx != x || mon->my != y) {
		        pline("%s %s, %s evading your %skick.", 
				Blind ? "It" : Monnam(mon),
				(can_teleport(mon->data) ? "teleports" :
				 is_floater(mon->data) ? "floats" :
				 is_flyer(mon->data) ? "flutters" :
				 nolimbs(mon->data) ? "slides" :
				 "jumps"),
				clumsy ? "easily" : "nimbly",
				clumsy ? "clumsy " : "");
			(void) passive(mon, FALSE, 1, TRUE);
		        return;
		    } 
		}
	}
	kickdmg(mon, clumsy);
}
#endif /* KICK */

/* return TRUE if caught, FALSE otherwise */
boolean
ghitm(mtmp, amount)
register struct monst *mtmp;
register long amount;
{
	if(!likes_gold(mtmp->data) && !mtmp->isshk 
#if defined(ALTARS) && defined(THEOLOGY)
		&& !mtmp->ispriest
#endif
		) 
		wakeup(mtmp);
	else {
		mtmp->msleep = 0;
		mtmp->meating = 0;
		if(!rn2(4)) setmangry(mtmp); /* not always pleasing */
		
		/* greedy monsters catch gold */
		kludge("%s catches the gold.", Monnam(mtmp));
		mtmp->mgold += amount;
		if (mtmp->isshk) {
			long robbed = ESHK(mtmp)->robbed;

			if (robbed) {
				robbed -= amount;
				if (robbed < 0) robbed = 0;
				pline("The amount %scovers %s recent losses.",
					!robbed ? "" : "partially ", 
					ESHK(mtmp)->ismale ? "his" : "her");
				ESHK(mtmp)->robbed = robbed;
				if(!robbed)
					make_happy_shk(mtmp);
			} else {
				if(mtmp->mpeaceful) {
				    ESHK(mtmp)->credit += amount;
				    You("have %ld zorkmids in credit.",
						ESHK(mtmp)->credit);
				} else verbalize("Thanks, scum!");
			}
		}
#if defined(ALTARS) && defined(THEOLOGY)
		else if(mtmp->ispriest) {
			if(mtmp->mpeaceful)
			    verbalize("Thank you for your contribution.");
			else verbalize("Thanks, scum!");
		}
#endif
		return(1);
	}
	return(0);

}

boolean
bad_kick_throw_pos(x,y)
xchar x,y;
{
	register struct rm *lvl = &(levl[x][y]);

	return(!ACCESSIBLE(lvl->typ) || lvl->typ == SDOOR ||
	    (IS_DOOR(lvl->typ) && (lvl->doormask & (D_CLOSED | D_LOCKED))) );
}

struct monst *
ghit(ddx, ddy, range)
register int ddx, ddy, range; 
{
	register struct monst *mtmp = (struct monst *) 0;

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	tmp_at(-1, GOLD_SYM);	/* open call */
	tmp_at(-3, (int)AT_GLD);
	while(range-- > 0) {
		bhitpos.x += ddx;
		bhitpos.y += ddy;
		if(levl[bhitpos.x][bhitpos.y].mmask) {
			mtmp = m_at(bhitpos.x,bhitpos.y);
			tmp_at(-1, -1); /* close call */
			return(mtmp);
		}
		/* stop on a zorkmid */
		if(levl[bhitpos.x][bhitpos.y].gmask ||
		     	    OBJ_AT(bhitpos.x, bhitpos.y)) {
			tmp_at(-1, -1); /* close call */
			return (struct monst *)0;
		}
		if(bad_kick_throw_pos(bhitpos.x,bhitpos.y)) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		}
		tmp_at(bhitpos.x, bhitpos.y);
	}
	tmp_at(-1, -1);
	return(struct monst *)0;
}

#ifdef KICK
static int
kick_object(x, y)
int x, y;
{
	int range, odx, ody, cnt = 0;
	register struct monst *mon;
	struct gold *gold;
	register struct obj *otmp, *obj;
	boolean costly = FALSE;

	/* if a pile, the "top" object gets kicked */
	for (otmp = fobj; otmp; otmp = otmp->nobj)
		if(otmp->ox == x && otmp->oy == y)
		    if(!otmp->cobj) {
			    cnt++;
			    if(cnt == 1) obj = otmp;
		    }

	/* range < 2 means the object will not move.	*/
	/* maybe dexterity should also figure here.     */
	if(cnt) range = (int)((ACURR(A_STR) > 18 ? 20 : 
				ACURR(A_STR))/2 - obj->owt/4);
	else range = rnd((int)ACURR(A_STR));

	if(range < 1) range = 1; /* safety... */
	if(martial()) range = range + rnd(3);

	/* see if the object has a place to move into */
	odx = x + u.dx;
	ody = y + u.dy;
	if(bad_kick_throw_pos(odx,ody))
	    range = 1;

	if(Fumbling && !rn2(3)) {
		Your("clumsy kick missed.");
		return(1);
	}

	if(!cnt && levl[x][y].gmask) {
		long zm;
		gold = g_at(x, y);
		zm = gold->amount;
		if(IS_ROCK(levl[x][y].typ)) {
			if ((!martial() && rn2(20) > ACURR(A_DEX))
#ifdef POLYSELF
				|| IS_ROCK(levl[u.ux][u.uy].typ)
#endif
								) {
				pline("%s doesn't come loose.",
					Blind ? "It" : "The gold");
				return(!rn2(3) || martial());
			}
			pline("%s comes loose.", Blind ? "It" : "The gold");
			freegold(gold);
			newsym(x, y);
			mkgold(zm, u.ux, u.uy);
			if (Invisible
#ifdef POLYSELF
					&& !u.uundetected
#endif
						) newsym(u.ux, u.uy);
			return(1);
		}
		if(range < 2 || zm > 300L) /* arbitrary */
		    return(0);
		freegold(gold);
		newsym(x, y);
		if(mon = ghit(u.dx, u.dy, range)) {
			setmangry(mon); /* not a means for payment to shk */
			if(ghitm(mon, zm)) /* was it caught? */
			    return(1);
		}
		mkgold(zm, bhitpos.x, bhitpos.y);
		if(cansee(bhitpos.x, bhitpos.y)) prl(bhitpos.x,bhitpos.y);
		return(1);
	}

	if(obj->otyp == BOULDER || obj == uball || obj == uchain)
		return(0);

	/* a box gets a chance of breaking open here */
	if(Is_box(obj)) {
		boolean otrp = obj->otrapped;

		if (!obj->olocked && (!rn2(3) ||
					(martial() && !rn2(2)))) {
		    pline("The lid slams open, then falls shut.");
		    if(otrp) goto gotcha;
		    return(1);
		} else if (obj->olocked && 
				(!rn2(5) || (martial() && !rn2(2)))) {
		    You("break open the lock!");
		    obj->olocked = 0;
	            if(otrp) {
gotcha:
		        chest_trap(obj, LEG);
		    }
		    return(1);
		}
		/* let it fall through to the next cases... */
	}

	if(Levitation && !rn2(3)) {
		You("miss."); /* do not identify the object */
		return(1);
	}

	/* fragile objects should not be kicked */
	if (breaks(obj, FALSE)) return(1);

	costly = costly_spot(x, y);

	/* potions get a chance of breaking here */
	if(obj->olet == POTION_SYM) {
		if(rn2(2)) {
		    You("smash the %s!", xname(obj));
		    if(costly) addtobill(obj, FALSE);
		    potionbreathe(obj);
		    delobj(obj);
		    return(1);
		}
	}

	if(IS_ROCK(levl[x][y].typ)) {
		if ((!martial() && rn2(20) > ACURR(A_DEX))
#ifdef POLYSELF
				|| IS_ROCK(levl[u.ux][u.uy].typ)
#endif
								) {
			if (Blind) pline("It doesn't come loose.");
			else pline("The %s do%sn't come loose.",
				distant_name(obj, xname),
				(obj->quan==1) ? "es" : "");
			return(!rn2(3) || martial());
		}
		if (Blind) pline("It comes loose.");
		else pline("The %s come%s loose.", distant_name(obj, xname),
			(obj->quan==1) ? "s" : "");
		move_object(obj, u.ux, u.uy);
		newsym(x, y);
		stackobj(obj);
		if (Invisible
#ifdef POLYSELF
				&& !u.uundetected
#endif
						) newsym(u.ux, u.uy);
		if (costly && !costly_spot(u.ux, u.uy)) addtobill(obj, FALSE);
		return(1);
	}

	/* too heavy to move. make sure not to call bhit  */
	/* in this function when range < 2 (a display bug */
	/* results otherwise).  			  */
	if(range <= 2) {
	    if(Is_box(obj)) pline("THUD!");
	    else pline("Thump!");
	    if(!rn2(3) || martial()) return(1);
	    return(0);
	}

	/* Needed to fool bhit's display-cleanup to show immediately	*/
	/* the next object in the pile.  We know here that the object	*/
	/* will move, so there is no need to worry about the location,	*/
	/* which merely needs to be something other than ox, oy.	*/
	move_object(obj, u.ux, u.uy);
	if(cnt == 1 && !levl[x][y].mmask) newsym(x, y);

	mon = bhit(u.dx, u.dy, range, obj->olet,
			(int (*)()) 0, (int (*)()) 0, obj);
	if(mon) {
# ifdef WORM
		if (mon->mx != bhitpos.x || mon->my != bhitpos.y)
			notonhead = TRUE;
# endif
		/* awake monster if sleeping */
		wakeup(mon);
		if(thitmonst(mon, obj)) return(1);
	}
	if(costly && !costly_spot(bhitpos.x,bhitpos.y)) addtobill(obj, FALSE);
	move_object(obj, bhitpos.x, bhitpos.y);
	stackobj(obj);
	if(!levl[obj->ox][obj->oy].mmask) newsym(obj->ox, obj->oy);
	return(1);
}
#endif /* KICK */


int
dokick() {		/* try to kick the door down - noisy! */
        register int x, y;
	register struct rm *maploc;
	register int avrg_attrib = (ACURR(A_STR)+ACURR(A_DEX)+ACURR(A_CON))/3;

#ifdef POLYSELF
	if(nolimbs(uasmon)) {
		You("have no legs to kick with.");
		return(0);
	}
	if(verysmall(uasmon)) {
		You("are too small to do any kicking.");
		return(0);
	}
#endif
	if(Wounded_legs) {
		Your("%s %s in no shape for kicking.",
		      ((Wounded_legs & BOTH_SIDES)==BOTH_SIDES)
			? makeplural(body_part(LEG)) : body_part(LEG),
		      ((Wounded_legs & BOTH_SIDES)==BOTH_SIDES) ? "are" : "is");
		return(0);
	}

	if(inv_weight() > 0) {
		Your("load is too heavy to balance yourself for a kick.");
		return(0);
	}

        if(u.utrap) {
		switch (u.utraptype) {
		    case TT_PIT:
			pline("There's nothing to kick down here.");
		    case TT_WEB:
		    case TT_BEARTRAP:
			You("can't move your %s!", body_part(LEG));
		}
		return(0);
	}

	if(!getdir(1)) return(0);
	if(!u.dx && !u.dy) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;

	if(u.uswallow) {
		switch(rn2(3)) {
		case 0:  You("can't move your %s!", body_part(LEG));
			 break;
		case 1:  pline("%s burps loudly.", Monnam(u.ustuck)); break;
		default: Your("feeble kick has no effect."); break;
		}
		return(1);
	}

	wake_nearby();
	u_wipe_engr(2);

	maploc = &levl[x][y];

#ifdef KICK
	/* The next four main loops should stay in */
	/* their present order: monsters, objects, */
	/* non-doors, doors.			   */ 

	if(maploc->mmask) {
		kick_monster(x, y);
		return(1);
	}

	if((OBJ_AT(x, y) || maploc->gmask) && !Levitation) {
		if(kick_object(x, y)) return(1);
		else goto ouch;
	}

	if(!IS_DOOR(maploc->typ)) {
		if(maploc->typ == SDOOR) {
		    if(rn2(30) < avrg_attrib) { 
			pline("Crash!  You kick open a secret door!");
			maploc->typ = DOOR;
			atl(x, y, (char) DOOR_SYM);
			if(maploc->doormask & D_TRAPPED) {
			    b_trapped("door");
			    maploc->doormask = D_NODOOR;
			} else
			    maploc->doormask = D_ISOPEN;
			return(1);
		    } else goto ouch;
		}
		if(maploc->typ == SCORR) {
		    if(rn2(30) < avrg_attrib) { 
			pline("Crash!  You kick open a secret passage!");
			maploc->typ = CORR;
			atl(x, y, (char) CORR_SYM);
			return(1);
		    } else goto ouch;
		}
# ifdef THRONES
		if(IS_THRONE(maploc->typ)) {
		    register int i;
		    if((u.uluck < 0 || maploc->doormask) && !rn2(3)) {
			pline("CRASH!  You destroy the throne.");
			maploc->typ = ROOM;
			maploc->doormask = 0; /* don't leave loose ends.. */
			mkgold((long)rnd(200), x, y);
			prl(x, y);
			return(1);
		    } else if(u.uluck && !rn2(3) && !maploc->doormask) {
			You("kick loose some ornamental coins and gems!");
			mkgold((300L+(long)rn2(201)), x, y);
			i = u.uluck + 1;
			if(i > 6) i = 6;
			while(i--) (void) mkobj_at(GEM_SYM, x, y);
			prl(x, y);
			/* prevent endless milking */
			maploc->doormask = T_LOOTED; 
			return(1);
		    } else if (!rn2(4)) {
			register struct trap *ttmp = 
					maketrap(u.ux,u.uy,TRAPDOOR);
			dotrap(ttmp);
			return(1);
		    }
		    goto ouch;
		}
# endif
# ifdef ALTARS
		if(IS_ALTAR(maploc->typ)) {
		    You("kick the altar.");
		    if(!rn2(3)) goto ouch; 
#  ifdef THEOLOGY
		    altar_wrath(x, y);
#  endif
		    return(1);
		}
# endif
# ifdef SINKS
		if(IS_SINK(maploc->typ)) {
		    if(rn2(5)) {
			if(flags.soundok)
			    pline("Klunk!  The pipes vibrate noisily.");
			else pline("Klunk!");
		        return(1);
		    } else if(!rn2(3) &&
			      !(mons[PM_BLACK_PUDDING].geno & G_GENOD)) {
			pline("A %s ooze gushes up from the drain!",
			      Hallucination ? hcolor() : black);
			pmon(makemon(&mons[PM_BLACK_PUDDING], x, y));
			return(1);
#  ifdef HARD
		    } else if(!rn2(3) &&
#   ifndef POLYSELF
			      poly_gender() != 2 &&
#   endif
			      !(mons[poly_gender() == 1 ? PM_INCUBUS : PM_SUCCUBUS].geno & G_GENOD)) {
			/* can't resist... */
			pline("The dish washer returns!");
			pmon(makemon(&mons[poly_gender() == 1 ? PM_INCUBUS : PM_SUCCUBUS], x, y));
			return(1);
#  endif
		    } else if(!rn2(3)) {
			pline("Flupp!  Muddy waste pops up from the drain.");
			if(!maploc->doormask) { /* only once per sink */
			    if(!Blind) 
				You("see a ring shining in its midst.");
			    (void) mkobj_at(RING_SYM, x, y);
			    prl(x, y);
			    maploc->doormask = T_LOOTED;
			}
			return(1);
		    }
		    goto ouch;
		}
# endif
		if(maploc->typ == STAIRS 
# ifdef STRONGHOLD
					|| maploc->typ == LADDER
# endif
		  ) goto ouch;
		if(IS_STWALL(maploc->typ)) {
ouch:
		    pline("Ouch!  That hurts!");
		    if(!rn2(3)) set_wounded_legs(RIGHT_SIDE, 5 + rnd(5));
		    losehp(rnd(ACURR(A_CON) > 15 ? 3 : 5), "dumb move");
		    return(1);
		}
# ifdef STRONGHOLD
		if (is_drawbridge_wall(x,y) >= 0) {
		    pline("The drawbridge is unaffected.");
		    return(1);
		}
# endif
		goto dumb;
	}
#endif /* KICK */

	if(maploc->doormask == D_ISOPEN ||
	   maploc->doormask == D_BROKEN ||
	   maploc->doormask == D_NODOOR) {
#ifdef KICK
dumb:
#endif
		if (martial() || ACURR(A_DEX) >= 16 || rn2(3)) {
			You("kick at empty space.");
		} else {
			pline("Dumb move!  You strain a muscle.");
			set_wounded_legs(RIGHT_SIDE, 5 + rnd(5));
		}
		return(0);
	}

	/* door is known to be CLOSED or LOCKED */
	if(rnl(35) < avrg_attrib + (!martial() ? 0 : ACURR(A_DEX))) {
		/* break the door */
		if(maploc->doormask & D_TRAPPED) {
		    pline("As you kick the door, it explodes!");
		    b_trapped("door");
		    maploc->doormask = D_NODOOR;
		} else if(ACURR(A_STR) > 18 && !rn2(5) && !in_shop(x, y)) {
		    pline("As you kick the door, it shatters to pieces!");
		    maploc->doormask = D_NODOOR;
		} else {
		    pline("As you kick the door, it crashes open!");
		    maploc->doormask = D_BROKEN;
		    if(in_shop(x, y) && !in_shop(u.ux, u.uy))
			pay_for_door(x, y, "break");
		}
	} else	pline("WHAMMM!!!");

	return(1);
}
