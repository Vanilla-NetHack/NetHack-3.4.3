/*	SCCS Id: @(#)hack.c	3.2	96/03/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL int NDECL(moverock);
STATIC_DCL int FDECL(still_chewing,(XCHAR_P,XCHAR_P));
#ifdef SINKS
STATIC_DCL void NDECL(dosinkfall);
#endif
STATIC_DCL boolean FDECL(monstinroom, (struct permonst *,int));

static void FDECL(move_update, (BOOLEAN_P));

#define IS_SHOP(x)	(rooms[x].rtype >= SHOPBASE)

#ifdef OVL2

boolean
revive_nasty(x, y, msg)
int x,y;
const char *msg;
{
    register struct obj *otmp, *otmp2;
    struct monst *mtmp;
    coord cc;
    boolean revived = FALSE;

    for(otmp = level.objects[x][y]; otmp; otmp = otmp2) {
	otmp2 = otmp->nexthere;
	if (otmp->otyp == CORPSE &&
	    (is_rider(&mons[otmp->corpsenm]) ||
	     otmp->corpsenm == PM_WIZARD_OF_YENDOR)) {
	    /* move any living monster already at that location */
	    if((mtmp = m_at(x,y)) && enexto(&cc, x, y, mtmp->data))
		rloc_to(mtmp, cc.x, cc.y);
	    if(msg) Norep("%s", msg);
	    revived = revive_corpse(otmp);
	}
    }

    /* this location might not be safe, if not, move revived monster */
    if (revived) {
	mtmp = m_at(x,y);
	if (mtmp && !goodpos(x, y, mtmp, mtmp->data) &&
	    enexto(&cc, x, y, mtmp->data)) {
	    rloc_to(mtmp, cc.x, cc.y);
	}
	/* else impossible? */
    }

    return (revived);
}

STATIC_OVL int
moverock()
{
    register xchar rx, ry;
    register struct obj *otmp;
    register struct trap *ttmp;
    register struct monst *mtmp;

    while ((otmp = sobj_at(BOULDER, u.ux+u.dx, u.uy+u.dy)) != 0) {
	rx = u.ux+2*u.dx;
	ry = u.uy+2*u.dy;
	nomul(0);
	if (Levitation || Is_airlevel(&u.uz)) {
	    if (Blind) feel_location(u.ux+u.dx,u.uy+u.dy);
	    You("don't have enough leverage to push %s.", the(xname(otmp)));
	    /* Give them a chance to climb over it? */
	    return -1;
	}
	if (verysmall(uasmon)) {
	    if (Blind) feel_location(u.ux+u.dx,u.uy+u.dy);
	    pline("You're too small to push that %s.", xname(otmp));
	    goto cannot_push;
	}
	if (isok(rx,ry) && !IS_ROCK(levl[rx][ry].typ) &&
	    (!IS_DOOR(levl[rx][ry].typ) || !(u.dx && u.dy) || (
#ifdef REINCARNATION
		!Is_rogue_level(&u.uz) &&
#endif
		(levl[rx][ry].doormask & ~D_BROKEN) == D_NODOOR)) &&
	    !sobj_at(BOULDER, rx, ry)) {
	    ttmp = t_at(rx, ry);
	    mtmp = m_at(rx, ry);

	    if (revive_nasty(rx, ry, "You sense movement on the other side."))
		return (-1);

	    if (mtmp && !noncorporeal(mtmp->data) &&
		    (!mtmp->mtrapped ||
			 !(ttmp && ((ttmp->ttyp == PIT) ||
				    (ttmp->ttyp == SPIKED_PIT))))) {
		if (canspotmon(mtmp))
		    pline("There's %s on the other side.", mon_nam(mtmp));
		else {
		    if (Blind) feel_location(u.ux+u.dx,u.uy+u.dy);
		    You_hear("a monster behind %s.", the(xname(otmp)));
		}
		if (flags.verbose)
		    pline("Perhaps that's why you cannot move it.");
		goto cannot_push;
	    }

	    if (ttmp)
		switch(ttmp->ttyp) {
		case LANDMINE:
		    if (rn2(10)) {
			pline("KAABLAMM!!!  %s triggers %s land mine.",
				The(xname(otmp)),
				ttmp->madeby_u ? "your" : "a");
			obj_extract_self(otmp);
			place_object(otmp, rx, ry);
			deltrap(ttmp);
			del_engr_at(rx,ry);
			scatter(rx,ry, 4, MAY_DESTROY|MAY_HIT|MAY_FRACTURE|VIS_EFFECTS);
			if (cansee(rx,ry)) newsym(rx,ry);
			continue;
		    }
		    break;
		case SPIKED_PIT:
		case PIT:
		    obj_extract_self(otmp);
		    /* vision kludge to get messages right;
		       the pit will temporarily be seen even
		       if this is one among multiple boulders */
		    if (!Blind) viz_array[ry][rx] |= IN_SIGHT;
		    if (!flooreffects(otmp, rx, ry, "fall")) {
			place_object(otmp, rx, ry);
		    }
		    continue;
		case HOLE:
		case TRAPDOOR:
		    pline("%s %s and plugs a %s in the %s!",
			  The(xname(otmp)),
			  (ttmp->ttyp == TRAPDOOR) ? "triggers" : "falls into",
			  (ttmp->ttyp == TRAPDOOR) ? "trap door" : "hole",
			  surface(rx, ry));
		    deltrap(ttmp);
		    delobj(otmp);
		    bury_objs(rx, ry);
		    if (cansee(rx,ry)) newsym(rx,ry);
		    continue;
		case LEVEL_TELEP:
		case TELEP_TRAP:
		    You("push %s and suddenly it disappears!",
			the(xname(otmp)));
		    if (ttmp->ttyp == TELEP_TRAP)
			rloco(otmp);
		    else {
			int newlev = random_teleport_level();
			d_level dest;

			if (newlev == depth(&u.uz) || In_endgame(&u.uz))
			    continue;
			obj_extract_self(otmp);
			add_to_migration(otmp);
			get_level(&dest, newlev);
			otmp->ox = dest.dnum;
			otmp->oy = dest.dlevel;
			otmp->owornmask = (long)MIGR_RANDOM;
		    }
		    seetrap(ttmp);
		    continue;
		}
	    if (closed_door(rx, ry))
		goto nopushmsg;
	    if (boulder_hits_pool(otmp, rx, ry, TRUE))
		continue;
	    /*
	     * Re-link at top of fobj chain so that pile order is preserved
	     * when level is restored.
	     */
	    if (otmp != fobj) {
		remove_object(otmp);
		place_object(otmp, otmp->ox, otmp->oy);
	    }

	    {
#ifdef LINT /* static long lastmovetime; */
		long lastmovetime;
		lastmovetime = 0;
#else
		static NEARDATA long lastmovetime;
#endif
		/* note: this var contains garbage initially and
		   after a restore */
		if (moves > lastmovetime+2 || moves < lastmovetime)
		    pline("With great effort you move %s.", the(xname(otmp)));
		exercise(A_STR, TRUE);
		lastmovetime = moves;
	    }

	    /* Move the boulder *after* the message. */
	    movobj(otmp, rx, ry);	/* does newsym(rx,ry) */
	    if (Blind) {
		feel_location(rx,ry);
		feel_location(u.ux+u.dx, u.uy+u.dy);
	    } else {
		newsym(u.ux+u.dx, u.uy+u.dy);
	    }
	} else {
	nopushmsg:
	    You("try to move %s, but in vain.", the(xname(otmp)));
	    if (Blind) feel_location(u.ux+u.dx, u.uy+u.dy);
	cannot_push:
	    if (throws_rocks(uasmon)) {
		pline("However, you can easily %s.",
			flags.pickup ? "pick it up" : "push it aside");
		break;
	    }
	    if (((!invent || inv_weight() <= -850) &&
		 (!u.dx || !u.dy || (IS_ROCK(levl[u.ux][u.uy+u.dy].typ)
				     && IS_ROCK(levl[u.ux+u.dx][u.uy].typ))))
		|| verysmall(uasmon)) {
		pline("However, you can squeeze yourself into a small opening.");
		break;
	    } else
		return (-1);
	}
    }
    return (0);
}

/*
 *  still_chewing()
 *
 *  Chew on a wall, door, or boulder.  Returns TRUE if still eating, FALSE
 *  when done.
 */
STATIC_OVL int
still_chewing(x,y)
    xchar x, y;
{
    struct rm *lev = &levl[x][y];
    struct obj *boulder = sobj_at(BOULDER,x,y);
    const char *digtxt = (char *)0, *dmgtxt = (char *)0;

    if (digging.down)		/* not continuing previous dig (w/ pick-axe) */
	(void) memset((genericptr_t)&digging, 0, sizeof digging);

    if (!boulder && IS_ROCK(lev->typ) && !may_dig(x,y)) {
	You("hurt your teeth on the hard stone.");
	nomul(0);
	return 1;
    } else if (digging.pos.x != x || digging.pos.y != y ||
		!on_level(&digging.level, &u.uz)) {
	digging.down = FALSE;
	digging.chew = TRUE;
	digging.pos.x = x;
	digging.pos.y = y;
	assign_level(&digging.level, &u.uz);
	/* solid rock takes more work & time to dig through */
	digging.effort = (IS_ROCK(lev->typ) ? 30 : 60) + u.udaminc;
	You("start chewing %s %s.",
	    boulder ? "on a" : "a hole in the",
	    boulder ? "boulder" : IS_ROCK(lev->typ) ? "rock" : "door");
	return 1;
    } else if ((digging.effort += (30 + u.udaminc)) <= 100)  {
	if (flags.verbose)
	    You("%s chewing on the %s.",
		digging.chew ? "continue" : "begin",
		boulder ? "boulder" : IS_ROCK(lev->typ) ? "rock" : "door");
	digging.chew = TRUE;
	return 1;
    }

    if (boulder) {
	delobj(boulder);		/* boulder goes bye-bye */
	You("eat the boulder.");	/* yum */

	/*
	 *  The location could still block because of
	 *	1. More than one boulder
	 *	2. Boulder stuck in a wall/stone/door.
	 *
	 *  [perhaps use does_block() below (from vision.c)]
	 */
	if (IS_ROCK(lev->typ) || closed_door(x,y) || sobj_at(BOULDER,x,y)) {
	    block_point(x,y);	/* delobj will unblock the point */
	    /* reset dig state */
	    (void) memset((genericptr_t)&digging, 0, sizeof digging);
	    return 1;
	}

    } else if (IS_WALL(lev->typ)) {
	if (*in_rooms(x, y, SHOPBASE)) {
	    add_damage(x, y, 10L * ACURRSTR);
	    dmgtxt = "damage";
	}
	digtxt = "chew a hole in the wall.";
	if (level.flags.is_maze_lev) {
	    lev->typ = ROOM;
	} else if (level.flags.is_cavernous_lev) {
	    lev->typ = CORR;
	} else {
	    lev->typ = DOOR;
	    lev->doormask = D_NODOOR;
	}
    } else if (lev->typ == SDOOR) {
	if (lev->doormask & D_TRAPPED) {
	    lev->doormask = D_NODOOR;
	    b_trapped("secret door", 0);
	} else {
	    digtxt = "chew through the secret door.";
	    lev->doormask = D_BROKEN;
	}
	lev->typ = DOOR;

    } else if (IS_DOOR(lev->typ)) {
	if (*in_rooms(x, y, SHOPBASE)) {
	    add_damage(x, y, 400L);
	    dmgtxt = "break";
	}
	if (lev->doormask & D_TRAPPED) {
	    lev->doormask = D_NODOOR;
	    b_trapped("door", 0);
	} else {
	    digtxt = "chew through the door.";
	    lev->doormask = D_BROKEN;
	}

    } else { /* STONE or SCORR */
	digtxt = "chew a passage through the rock.";
	lev->typ = CORR;
    }

    unblock_point(x, y);	/* vision */
    newsym(x, y);
    if (digtxt) You(digtxt);	/* after newsym */
    if (dmgtxt) pay_for_damage(dmgtxt);
    (void) memset((genericptr_t)&digging, 0, sizeof digging);
    return 0;
}

#endif /* OVL2 */
#ifdef OVLB

void
movobj(obj, ox, oy)
register struct obj *obj;
register xchar ox, oy;
{
	/* optimize by leaving on the fobj chain? */
	remove_object(obj);
	newsym(obj->ox, obj->oy);
	place_object(obj, ox, oy);
	newsym(ox, oy);
}

#ifdef SINKS
static NEARDATA const char fell_on_sink[] = "fell onto a sink";

STATIC_OVL void
dosinkfall()
{
	register struct obj *obj;

	if (is_floater(uasmon) || (HLevitation & FROMOUTSIDE)) {
		You("wobble unsteadily for a moment.");
	} else {
		You("crash to the floor!");
		losehp((rn1(10, 20 - (int)ACURR(A_CON))),
			fell_on_sink, NO_KILLER_PREFIX);
		exercise(A_DEX, FALSE);
		for(obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
		    if(obj->oclass == WEAPON_CLASS) {
			You("fell on %s.",doname(obj));
			losehp(rnd(3), fell_on_sink, NO_KILLER_PREFIX);
			exercise(A_CON, FALSE);
		    }
	}

	HLevitation &= ~(I_SPECIAL|W_ARTI|TIMEOUT);
	HLevitation++;
	if(uleft && uleft->otyp == RIN_LEVITATION) {
	    obj = uleft;
	    Ring_off(obj);
	    off_msg(obj);
	}
	if(uright && uright->otyp == RIN_LEVITATION) {
	    obj = uright;
	    Ring_off(obj);
	    off_msg(obj);
	}
	if(uarmf && uarmf->otyp == LEVITATION_BOOTS) {
	    obj = uarmf;
	    (void)Boots_off();
	    off_msg(obj);
	}
	HLevitation--;
}
#endif

boolean
may_dig(x,y)
register xchar x,y;
/* intended to be called only on ROCKs */
{
    return (boolean)(!(IS_STWALL(levl[x][y].typ) &&
			(levl[x][y].wall_info & W_NONDIGGABLE)));
}

boolean
may_passwall(x,y)
register xchar x,y;
{
   return (boolean)(!(IS_STWALL(levl[x][y].typ) &&
			(levl[x][y].wall_info & W_NONPASSWALL)));
}

#endif /* OVLB */
#ifdef OVL1

boolean
bad_rock(mdat,x,y)
struct permonst *mdat;
register xchar x,y;
{
	return((boolean)(IS_ROCK(levl[x][y].typ)
		    && (!tunnels(mdat) || needspick(mdat) || !may_dig(x,y))
		    && !(passes_walls(mdat) && may_passwall(x,y))));
}

boolean
invocation_pos(x, y)
xchar x, y;
{
	return((boolean)(Invocation_lev(&u.uz) && x == inv_pos.x && y == inv_pos.y));
}

#endif /* OVL1 */
#ifdef OVL3

void
domove()
{
	register struct monst *mtmp;
	register struct rm *tmpr,*ust;
	register xchar x,y;
	struct trap *trap;
	int wtcap;
	boolean on_ice;
	xchar chainx, chainy, ballx, bally;	/* ball&chain new positions */
	int bc_control;				/* control for ball&chain */
	boolean cause_delay = FALSE;	/* dragging ball will skip a move */

	u_wipe_engr(rnd(5));

	if(((wtcap = near_capacity()) >= OVERLOADED
	    || (wtcap > SLT_ENCUMBER &&
		(u.mtimedone ? (u.mh < 5 && u.mh != u.mhmax)
			: (u.uhp < 10 && u.uhp != u.uhpmax))))
	   && !Is_airlevel(&u.uz)) {
	    if(wtcap < OVERLOADED) {
		You("don't have enough stamina to move.");
		exercise(A_CON, FALSE);
	    } else
		You("collapse under your load.");
	    nomul(0);
	    return;
	}
	if(u.uswallow) {
		u.dx = u.dy = 0;
		u.ux = x = u.ustuck->mx;
		u.uy = y = u.ustuck->my;
		mtmp = u.ustuck;
	} else {
		if (Is_airlevel(&u.uz) && rn2(4) &&
			!Levitation && !is_flyer(uasmon)) {
		    switch(rn2(3)) {
		    case 0:
			You("tumble in place.");
			exercise(A_DEX, FALSE);
			break;
		    case 1:
			You_cant("control your movements very well."); break;
		    case 2:
			pline("It's hard to walk in thin air.");
			exercise(A_DEX, TRUE);
			break;
		    }
		    return;
		}

		/* check slippery ice */
		on_ice = !Levitation && is_ice(u.ux, u.uy);
		if (on_ice) {
		    static int skates = 0;
		    if (!skates) skates = find_skates();
		    if ((uarmf && uarmf->otyp == skates)
			    || resists_cold(&youmonst) || is_flyer(uasmon)
			    || is_floater(uasmon) || is_clinger(uasmon)
			    || is_whirly(uasmon))
			on_ice = FALSE;
		    else if (!rn2(Cold_resistance ? 3 : 2)) {
			Fumbling |= FROMOUTSIDE;
			Fumbling &= ~TIMEOUT;
			Fumbling += 1;	/* slip on next move */
		    }
		}
		if (!on_ice && (Fumbling & FROMOUTSIDE))
		    Fumbling &= ~FROMOUTSIDE;

		x = u.ux + u.dx;
		y = u.uy + u.dy;
		if(Stunned || (Confusion && !rn2(5))) {
			register int tries = 0;

			do {
				if(tries++ > 50) {
					nomul(0);
					return;
				}
				confdir();
				x = u.ux + u.dx;
				y = u.uy + u.dy;
			} while(!isok(x, y) || bad_rock(uasmon, x, y));
		}
		/* turbulence might alter your actual destination */
		if (u.uinwater) {
			water_friction();
			if (!u.dx && !u.dy) {
				nomul(0);
				return;
			}
			x = u.ux + u.dx;
			y = u.uy + u.dy;
		}
		if(!isok(x, y)) {
			nomul(0);
			return;
		}
		if((trap = t_at(x, y)) && trap->tseen) {
			if(flags.run >= 2) {
				nomul(0);
				flags.move = 0;
				return;
			} else
				nomul(0);
		}

		if (u.ustuck && (x != u.ustuck->mx || y != u.ustuck->my)) {
		    if (distu(u.ustuck->mx, u.ustuck->my) > 2) {
			/* perhaps it fled (or was teleported or ... ) */
			u.ustuck = 0;
		    } else if (sticks(uasmon)) {
			/* When polymorphed into a sticking monster,
			 * u.ustuck means it's stuck to you, not you to it.
			 */
			You("release %s.", mon_nam(u.ustuck));
			u.ustuck = 0;
		    } else {
			/* If holder is asleep or paralyzed:
			 *	37.5% chance of getting away,
			 *	12.5% chance of waking/releasing it;
			 * otherwise:
			 *	 7.5% chance of getting away.
			 * [strength ought to be a factor]
			 */
			switch (rn2(!u.ustuck->mcanmove ? 8 : 40)) {
			case 0: case 1: case 2:
			    You("pull free from %s.", mon_nam(u.ustuck));
			    u.ustuck = 0;
			    break;
			case 3:
			    if (!u.ustuck->mcanmove) {
				/* it's free to move on next turn */
				u.ustuck->mfrozen = 1;
				u.ustuck->msleep = 0;
			    }
			    /*FALLTHRU*/
			default:
			    You("cannot escape from %s!", mon_nam(u.ustuck));
			    nomul(0);
			    return;
			}
		    }
		}

		mtmp = m_at(x,y);
		if (mtmp) {
			/* Don't attack if you're running, and can see it */
			if (flags.run &&
			    ((!Blind && mon_visible(mtmp) &&
			      ((mtmp->m_ap_type != M_AP_FURNITURE &&
				mtmp->m_ap_type != M_AP_OBJECT) ||
			       Protection_from_shape_changers)) ||
			     sensemon(mtmp))) {
				nomul(0);
				flags.move = 0;
				return;
			}
		}
	}

	u.ux0 = u.ux;
	u.uy0 = u.uy;
	bhitpos.x = x;
	bhitpos.y = y;
	tmpr = &levl[x][y];

	/* attack monster */
	if(mtmp) {
	    nomul(0);
	    /* only attack if we know it's there */
	    /* or if it hides_under, in which case we call attack() to print
	     * the Wait! message.
	     * This is different from ceiling hiders, who aren't handled in
	     * attack().
	     */
	    if(!mtmp->mundetected || sensemon(mtmp) ||
		    ((hides_under(mtmp->data) || mtmp->data->mlet == S_EEL) &&
			!is_safepet(mtmp))){
		gethungry();
		if(wtcap >= HVY_ENCUMBER && moves%3) {
		    if(u.uhp > 1)
			u.uhp--;
		    else {
			You("pass out from exertion!");
			exercise(A_CON, FALSE);
			fall_asleep(-10, FALSE);
		    }
		}
		if(multi < 0) return;	/* we just fainted */

		/* try to attack; note that it might evade */
		/* also, we don't attack tame when _safepet_ */
		if(attack(mtmp)) return;
	    }
	}

	/* not attacking an animal, so we try to move */
	if(!uasmon->mmove) {
		You("are rooted %s.",
		    Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ?
		    "in place" : "to the ground");
		nomul(0);
		return;
	}
	if(u.utrap) {
		if(u.utraptype == TT_PIT) {
		    if (!rn2(2) && sobj_at(BOULDER, u.ux, u.uy)) {
			Your("%s gets stuck in a crevice.", body_part(LEG));
			display_nhwindow(WIN_MESSAGE, FALSE);
			clear_nhwindow(WIN_MESSAGE);
			You("free your %s.", body_part(LEG));
		    } else if (!(--u.utrap)) {
			You("crawl to the edge of the pit.");
			fill_pit(u.ux, u.uy);
			vision_full_recalc = 1;	/* vision limits change */
		    } else if (flags.verbose)
			Norep( (Hallucination && !rn2(5)) ?
				"You've fallen, and you can't get up." :
				"You are still in a pit." );
		} else if (u.utraptype == TT_LAVA) {
		    if(flags.verbose)
			Norep("You are stuck in the lava.");
		    if(!is_lava(x,y)) {
			u.utrap--;
			if((u.utrap & 0xff) == 0) {
			    You("pull yourself to the edge of the lava.");
			    u.utrap = 0;
			}
		    }
		    u.umoved = TRUE;
		} else if (u.utraptype == TT_WEB) {
		    if(uwep && uwep->oartifact == ART_STING) {
			u.utrap = 0;
			pline("Sting cuts through the web!");
			return;
		    }
		    if(--u.utrap) {
			if(flags.verbose)
			    Norep("You are stuck to the web.");
		    } else You("disentangle yourself.");
		} else if (u.utraptype == TT_INFLOOR) {
		    if(--u.utrap) {
			if(flags.verbose)
			    Norep("You are stuck in the floor.");
		    } else You("finally wiggle free.");
		} else {
		    if(flags.verbose)
			Norep("You are caught in a bear trap.");
		    if((u.dx && u.dy) || !rn2(5)) u.utrap--;
		}
		return;
	}


	/*
	 *  Check for physical obstacles.  First, the place we are going.
	 */
	if (IS_ROCK(tmpr->typ)) {
	    if (Blind) feel_location(x,y);
	    if (passes_walls(uasmon) && may_passwall(x,y)) {
		;	/* do nothing */
	    } else if (tunnels(uasmon) && !needspick(uasmon)) {
		/* Eat the rock. */
		if (still_chewing(x,y)) return;
	    } else {
		if (Is_stronghold(&u.uz) && is_db_wall(x,y))
		    pline_The("drawbridge is up!");
		flags.move = 0;
		nomul(0);
		return;
	    }
	} else if (IS_DOOR(tmpr->typ)) {
	    if (closed_door(x,y)) {
		if (Blind) feel_location(x,y);
		if (passes_walls(uasmon))
		    ;	/* do nothing */
		else if (amorphous(uasmon))
		    You("ooze under the door.");
		else if (tunnels(uasmon) && !needspick(uasmon)) {
		    /* Eat the door. */
		    if (still_chewing(x,y)) return;
		} else {
		    flags.move = 0;
		    if (x == u.ux || y == u.uy) {
			if (Blind || Stunned || ACURR(A_DEX) < 10 || Fumbling) {
			    pline("Ouch!  You bump into a door.");
			    exercise(A_DEX, FALSE);
			} else pline("That door is closed.");
		    }
		    nomul(0);
		    return;
		}
	    } else if (u.dx && u.dy && !passes_walls(uasmon)
			&& ((tmpr->doormask & ~D_BROKEN)
#ifdef REINCARNATION
					|| Is_rogue_level(&u.uz)
#endif
					|| block_door(x,y))) {
		/* Diagonal moves into a door are not allowed. */
		if (Blind) feel_location(x,y);	/* ?? */
		flags.move = 0;
		nomul(0);
		return;
	    }
	}
	if (u.dx && u.dy
		&& bad_rock(uasmon,u.ux,y) && bad_rock(uasmon,x,u.uy)) {
	    /* Move at a diagonal. */
	    if (bigmonst(uasmon)) {
		Your("body is too large to fit through.");
		nomul(0);
		return;
	    }
	    if (invent && (inv_weight() + weight_cap() > 600)) {
		You("are carrying too much to get through.");
		nomul(0);
		return;
	    }
	}

	ust = &levl[u.ux][u.uy];

	/* Now see if other things block our way . . */
	if (u.dx && u.dy && !passes_walls(uasmon)
			 && (IS_DOOR(ust->typ) && ((ust->doormask & ~D_BROKEN)
#ifdef REINCARNATION
				 || Is_rogue_level(&u.uz)
#endif
				 || block_entry(x, y))
			     )) {
	    /* Can't move at a diagonal out of a doorway with door. */
	    flags.move = 0;
	    nomul(0);
	    return;
	}

	if (sobj_at(BOULDER,x,y) && !passes_walls(uasmon)) {
	    if (!(Blind || Hallucination) && (flags.run >= 2)) {
		nomul(0);
		flags.move = 0;
		return;
	    }
	    /* tunneling monsters will chew before pushing */
	    if (tunnels(uasmon) && !needspick(uasmon)) {
		if (still_chewing(x,y)) return;
	    } else
		if (moverock() < 0) return;
	}

	/* OK, it is a legal place to move. */

	/* Move ball and chain.  */
	if (Punished)
	    if (!drag_ball(x,y, &bc_control, &ballx, &bally, &chainx, &chainy,
			&cause_delay))
		return;

	/* now move the hero */
	mtmp = m_at(x, y);
	u.ux += u.dx;
	u.uy += u.dy;
	/* if safepet at destination then move the pet to the hero's
	 * previous location using the same conditions as in attack().
	 * there are special extenuating circumstances:
	 * (1) if the pet dies then your god angers,
	 * (2) if the pet gets trapped then your god may disapprove,
	 * (3) if the pet was already trapped and you attempt to free it
	 * not only do you encounter the trap but you may frighten your
	 * pet causing it to go wild!  moral: don't abuse this privilege.
	 */
	/* Ceiling-hiding pets are skipped by this section of code, to
	 * be caught by the normal falling-monster code.
	 */
	if (is_safepet(mtmp) && !(is_hider(mtmp->data) && mtmp->mundetected)) {
		int swap_result;

		/* if trapped, there's a chance the pet goes wild */
		if (mtmp->mtrapped) {
		    if (!rn2(mtmp->mtame)) {
			mtmp->mtame = mtmp->mpeaceful = mtmp->msleep = 0;
		        growl(mtmp);
		    } else {
		        yelp(mtmp);
		    }
	        }

		if (mtmp->m_ap_type) seemimic(mtmp);
		else if (!mtmp->mtame) newsym(mtmp->mx, mtmp->my);

		mtmp->mtrapped = 0;
		mtmp->mundetected = 0;
		remove_monster(x, y);
		place_monster(mtmp, u.ux0, u.uy0);

		/* check first to see if monster drowned.
		 * then check for traps.
		 */
		if (minwater(mtmp)) {
		    swap_result = 2;
		} else swap_result = mintrap(mtmp);

		switch (swap_result) {
		case 0:
		    You("%s %s.", mtmp->mtame ? "displaced" : "frightened",
			    mtmp->mnamelth ? NAME(mtmp) : mon_nam(mtmp));
		    break;
		case 1:	/* trapped */
		case 3: /* changed levels */
		    /* there's already been a trap message, reinforce it */
		    abuse_dog(mtmp);
		    adjalign(-3);
		    break;
		case 2:
		    /* it may have drowned or died.  that's no way to
		     * treat a pet!  your god gets angry.
		     */
		    if (rn2(4)) {
			You_feel("guilty about losing your pet like this.");
			u.ugangr++;
			adjalign(-15);
		    }
		    break;
		default:
		    pline("that's strange, unknown mintrap result!");
		    break;
		}
	}

	reset_occupations();
	if (flags.run) {
		if (IS_DOOR(tmpr->typ) || IS_ROCK(tmpr->typ) ||
			IS_FURNITURE(tmpr->typ))
		    nomul(0);
	}

	if (hides_under(uasmon))
	    u.uundetected = OBJ_AT(u.ux, u.uy);
	else if (uasmon->mlet == S_EEL)
	    u.uundetected = is_pool(u.ux, u.uy) && !Is_waterlevel(&u.uz);
	else if (u.dx || u.dy) {
	    if (u.usym == S_MIMIC_DEF)
		u.usym = S_MIMIC;
	    u.uundetected = 0;
	}

	check_leash(u.ux0,u.uy0);

	if(u.ux0 != u.ux || u.uy0 != u.uy) {
	    u.umoved = TRUE;
	    /* Clean old position -- vision_recalc() will print our new one. */
	    newsym(u.ux0,u.uy0);
	    /* Since the hero has moved, adjust what can be seen/unseen. */
	    vision_recalc(1);	/* Do the work now in the recover time. */

	    /* a special clue-msg when on the Invocation position */
	    if(invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
		struct obj *otmp = carrying(CANDELABRUM_OF_INVOCATION);

		You_feel("a strange vibration under your %s.",
			makeplural(body_part(FOOT)));
		if (otmp && otmp->spe == 7 && otmp->lamplit)
		    pline("%s %s!", The(xname(otmp)),
			Blind ? "throbs palpably" :
				"glows with a strange light");
	    }
	}

	if (Punished)				/* put back ball and chain */
	    move_bc(0,bc_control,ballx,bally,chainx,chainy);

	spoteffects();
	
	/* delay next move because of ball dragging */
	/* must come after we finished picking up, in spoteffects() */
	if (cause_delay) {
	    nomul(-2);
	    nomovemsg = "";
	}
}

#endif /* OVL3 */
#ifdef OVL2

void
spoteffects()
{
	register struct trap *trap;
	register struct monst *mtmp;

	if(u.uinwater) {
		int was_underwater;

		if (!is_pool(u.ux,u.uy)) {
			if (Is_waterlevel(&u.uz))
				You("pop into an air bubble.");
			else if (is_lava(u.ux, u.uy))
				You("leave the water...");	/* oops! */
			else
				You("are on solid %s again.",
				    is_ice(u.ux, u.uy) ? "ice" : "land");
		}
		else if (Is_waterlevel(&u.uz))
			goto stillinwater;
		else if (Levitation)
			You("pop out of the water like a cork!");
		else if (is_flyer(uasmon))
			You("fly out of the water.");
		else if (Wwalking)
			You("slowly rise above the surface.");
		else
			goto stillinwater;
		was_underwater = Underwater && !Is_waterlevel(&u.uz);
		u.uinwater = 0;		/* leave the water */
		if (was_underwater) {	/* restore vision */
			docrt();
			vision_full_recalc = 1;
		}
	}
stillinwater:;
	if (!Levitation && !u.ustuck && !is_flyer(uasmon)) {
	    /* limit recursive calls through teleds() */
	    if(is_lava(u.ux,u.uy) && lava_effects())
		    return;
	    if(is_pool(u.ux,u.uy) && !Wwalking && drown())
		    return;
	}
	check_special_room(FALSE);
#ifdef SINKS
	if(IS_SINK(levl[u.ux][u.uy].typ) && Levitation)
		dosinkfall();
#endif
	pickup(1);
	if ((trap = t_at(u.ux,u.uy)) != 0)
		dotrap(trap);	/* fall into pit, arrow trap, etc. */
	if((mtmp = m_at(u.ux, u.uy)) && !u.uswallow) {
		mtmp->mundetected = mtmp->msleep = 0;
		switch(mtmp->data->mlet) {
		    case S_PIERCER:
			pline("%s suddenly drops from the %s!",
			      Amonnam(mtmp), ceiling(u.ux,u.uy));
			if(mtmp->mtame) /* jumps to greet you, not attack */
			    ;
			else if(uarmh)
			    pline("Its blow glances off your helmet.");
			else if (u.uac + 3 <= rnd(20))
			    You("are almost hit by %s!",
				x_monnam(mtmp, 2, "falling", 1));
			else {
			    int dmg;
			    You("are hit by %s!",
				x_monnam(mtmp, 2, "falling", 1));
			    dmg = d(4,6);
			    if(Half_physical_damage) dmg = (dmg+1) / 2;
			    mdamageu(mtmp, dmg);
			}
			break;
		    default:	/* monster surprises you. */
			if(mtmp->mtame)
			    pline("%s jumps near you from the %s.",
					Amonnam(mtmp), ceiling(u.ux,u.uy));
			else if(mtmp->mpeaceful) {
				You("surprise %s!",
				    Blind && !sensemon(mtmp) ?
				    something : a_monnam(mtmp));
				mtmp->mpeaceful = 0;
			} else
			    pline("%s attacks you by surprise!",
					Amonnam(mtmp));
			break;
		}
		mnexto(mtmp); /* have to move the monster */
	}
}

STATIC_OVL boolean
monstinroom(mdat,roomno)
struct permonst *mdat;
int roomno;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->data == mdat &&
		   index(in_rooms(mtmp->mx, mtmp->my, 0), roomno + ROOMOFFSET))
			return(TRUE);
	return(FALSE);
}

char *
in_rooms(x, y, typewanted)
register xchar x, y;
register int typewanted;
{
	static char buf[5];
	char rno, *ptr = &buf[4];
	int typefound, min_x, min_y, max_x, max_y_offset, step;
	register struct rm *lev;

#define goodtype(rno) (!typewanted || \
	     ((typefound = rooms[rno - ROOMOFFSET].rtype) == typewanted) || \
	     ((typewanted == SHOPBASE) && (typefound > SHOPBASE))) \

	switch (rno = levl[x][y].roomno) {
		case NO_ROOM:
			return(ptr);
		case SHARED:
			step = 2;
			break;
		case SHARED_PLUS:
			step = 1;
			break;
		default:			/* i.e. a regular room # */
			if (goodtype(rno))
				*(--ptr) = rno;
			return(ptr);
	}

	min_x = x - 1;
	max_x = x + 1;
	if (x < 1)
		min_x += step;
	else
	if (x >= COLNO)
		max_x -= step;

	min_y = y - 1;
	max_y_offset = 2;
	if (min_y < 0) {
		min_y += step;
		max_y_offset -= step;
	} else
	if ((min_y + max_y_offset) >= ROWNO)
		max_y_offset -= step;

	for (x = min_x; x <= max_x; x += step) {
		lev = &levl[x][min_y];
		y = 0;
		if (((rno = lev[y].roomno) >= ROOMOFFSET) &&
		    !index(ptr, rno) && goodtype(rno))
			*(--ptr) = rno;
		y += step;
		if (y > max_y_offset)
			continue;
		if (((rno = lev[y].roomno) >= ROOMOFFSET) &&
		    !index(ptr, rno) && goodtype(rno))
			*(--ptr) = rno;
		y += step;
		if (y > max_y_offset)
			continue;
		if (((rno = lev[y].roomno) >= ROOMOFFSET) &&
		    !index(ptr, rno) && goodtype(rno))
			*(--ptr) = rno;
	}
	return(ptr);
}

static void
move_update(newlev)
register boolean newlev;
{
	char *ptr1, *ptr2, *ptr3, *ptr4;

	Strcpy(u.urooms0, u.urooms);
	Strcpy(u.ushops0, u.ushops);
	if (newlev) {
		u.urooms[0] = '\0';
		u.uentered[0] = '\0';
		u.ushops[0] = '\0';
		u.ushops_entered[0] = '\0';
		Strcpy(u.ushops_left, u.ushops0);
		return;
	}
	Strcpy(u.urooms, in_rooms(u.ux, u.uy, 0));

	for (ptr1 = &u.urooms[0],
	     ptr2 = &u.uentered[0],
	     ptr3 = &u.ushops[0],
	     ptr4 = &u.ushops_entered[0];
	     *ptr1; ptr1++) {
		if (!index(u.urooms0, *ptr1))
			*(ptr2++) = *ptr1;	
		if (IS_SHOP(*ptr1 - ROOMOFFSET)) {
			*(ptr3++) = *ptr1;
			if (!index(u.ushops0, *ptr1))
				*(ptr4++) = *ptr1;
		}
	}
	*ptr2 = '\0';
	*ptr3 = '\0';
	*ptr4 = '\0';

	/* filter u.ushops0 -> u.ushops_left */
	for (ptr1 = &u.ushops0[0], ptr2 = &u.ushops_left[0]; *ptr1; ptr1++)
		if (!index(u.ushops, *ptr1))
			*(ptr2++) = *ptr1;
	*ptr2 = '\0';
}

void
check_special_room(newlev)
register boolean newlev;
{
	register struct monst *mtmp;
	char *ptr;

	move_update(newlev);

	if (*u.ushops0)
	    u_left_shop(u.ushops_left, newlev);

	if (!*u.uentered && !*u.ushops_entered)		/* implied by newlev */
	    return;		/* no entrance messages necessary */

	/* Did we just enter a shop? */
	if (*u.ushops_entered)
	    u_entered_shop(u.ushops_entered);

	for (ptr = &u.uentered[0]; *ptr; ptr++) {
	    register int roomno = *ptr - ROOMOFFSET, rt = rooms[roomno].rtype;

	    /* Did we just enter some other special room? */
	    /* vault.c insists that a vault remain a VAULT,
	     * and temples should remain TEMPLEs,
	     * but everything else gives a message only the first time */
	    switch (rt) {
		case ZOO:
		    pline("Welcome to David's treasure zoo!");
		    break;
		case SWAMP:
		    pline("It %s rather %s down here.",
			  Blind ? "feels" : "looks",
			  Blind ? "humid" : "muddy");
		    break;
		case COURT:
		    You("enter an opulent throne room!");
		    break;
		case MORGUE:
		    if(midnight()) {
			const char *run = locomotion(uasmon, "Run");
			pline("%s away!  %s away!", run, run);
		    } else
			You("have an uncanny feeling...");
		    break;
		case BEEHIVE:
		    You("enter a giant beehive!");
		    break;
		case BARRACKS:
		    if(monstinroom(&mons[PM_SOLDIER], roomno) ||
			monstinroom(&mons[PM_SERGEANT], roomno) ||
			monstinroom(&mons[PM_LIEUTENANT], roomno) ||
			monstinroom(&mons[PM_CAPTAIN], roomno))
			You("enter a military barracks!");
		    else
			You("enter an abandoned barracks.");
		    break;
		case DELPHI:
		    if(monstinroom(&mons[PM_ORACLE], roomno))
			verbalize("Hello, %s, welcome to Delphi!", plname);
		    break;
		case TEMPLE:
		    intemple(roomno + ROOMOFFSET);
		    /* fall through */
		default:
		    rt = 0;
	    }

	    if (rt != 0) {
		rooms[roomno].rtype = OROOM;
		if (!search_special(rt)) {
			/* No more room of that type */
			switch(rt) {
			    case COURT:
				level.flags.has_court = 0;
				break;
			    case SWAMP:
				level.flags.has_swamp = 0;
				break;
			    case MORGUE:
				level.flags.has_morgue = 0;
				break;
			    case ZOO:
				level.flags.has_zoo = 0;
				break;
			    case BARRACKS:
				level.flags.has_barracks = 0;
				break;
			    case TEMPLE:
				level.flags.has_temple = 0;
				break;
			    case BEEHIVE:
				level.flags.has_beehive = 0;
				break;
			}
		}
		if(rt==COURT || rt==SWAMP || rt==MORGUE || rt==ZOO)
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				if(!Stealth && !rn2(3))
					mtmp->msleep = 0;
	    }
	}

	return;
}

#endif /* OVL2 */
#ifdef OVLB

int
dopickup()
{
	int count;
	/* awful kludge to work around parse()'s pre-decrement */
	count = (multi || (save_cm && *save_cm == ',')) ? multi + 1 : 0;
	multi = 0;	/* always reset */
	/* uswallow case added by GAN 01/29/87 */
	if(u.uswallow) {
		if (is_animal(u.ustuck->data)) {
		    You("pick up %s tongue.",
			            s_suffix(mon_nam(u.ustuck)));
		    pline("But it's kind of slimy, so you drop it.");
		} else
		    You("don't %s anything in here to pick up.",
			  Blind ? "feel" : "see");
		return(1);
	}
	if(is_pool(u.ux, u.uy)) {
	    if (Wwalking || is_flyer(uasmon) || is_clinger(uasmon)) {
		You("cannot dive into the water to pick things up.");
		return(1);
	    } else if (!Underwater) {
		You_cant("even see the bottom, let alone pick up %s.",
				something);
		return(1);
	    }
	}
	if(!OBJ_AT(u.ux, u.uy)) {
		pline("There is nothing here to pick up.");
		return(0);
	}
	if (!can_reach_floor()) {
		You("cannot reach the %s.", surface(u.ux,u.uy));
		return(1);
	}
	pickup(-count);
	return(1);
}

#endif /* OVLB */
#ifdef OVL2

/* stop running if we see something interesting */
/* turn around a corner if that is the only way we can proceed */
/* do not turn left or right twice */
void
lookaround()
{
    register int x, y, i, x0 = 0, y0 = 0, m0 = 1, i0 = 9;
    register int corrct = 0, noturn = 0;
    register struct monst *mtmp;
    register struct trap *trap;

	/* Grid bugs stop if trying to move diagonal, even if blind.  Maybe */
	/* they polymorphed while in the middle of a long move. */
	if (u.umonnum == PM_GRID_BUG && u.dx && u.dy) {
		nomul(0);
		return;
	}

	if(Blind || flags.run == 0) return;
	for(x = u.ux-1; x <= u.ux+1; x++) for(y = u.uy-1; y <= u.uy+1; y++) {
		if(!isok(x,y)) continue;

	if(u.umonnum == PM_GRID_BUG && x != u.ux && y != u.uy) continue;

	if(x == u.ux && y == u.uy) continue;

	if((mtmp = m_at(x,y)) &&
		    mtmp->m_ap_type != M_AP_FURNITURE &&
		    mtmp->m_ap_type != M_AP_OBJECT &&
		    (!mtmp->minvis || See_invisible) && !mtmp->mundetected) {
	    if((flags.run != 1 && !mtmp->mtame)
					|| (x == u.ux+u.dx && y == u.uy+u.dy))
		goto stop;
	}

	if (levl[x][y].typ == STONE) continue;
	if (x == u.ux-u.dx && y == u.uy-u.dy) continue;

	if (IS_ROCK(levl[x][y].typ) || (levl[x][y].typ == ROOM) ||
	    IS_AIR(levl[x][y].typ))
	    continue;
	else if (closed_door(x,y)) {
	    if(x != u.ux && y != u.uy) continue;
	    if(flags.run != 1) goto stop;
	    goto bcorr;
	} else if (levl[x][y].typ == CORR) {
bcorr:
	    if(levl[u.ux][u.uy].typ != ROOM) {
		if(flags.run == 1 || flags.run == 3) {
		    i = dist2(x,y,u.ux+u.dx,u.uy+u.dy);
		    if(i > 2) continue;
		    if(corrct == 1 && dist2(x,y,x0,y0) != 1)
			noturn = 1;
		    if(i < i0) {
			i0 = i;
			x0 = x;
			y0 = y;
			m0 = mtmp ? 1 : 0;
		    }
		}
		corrct++;
	    }
	    continue;
	} else if ((trap = t_at(x,y)) && trap->tseen) {
	    if(flags.run == 1) goto bcorr;	/* if you must */
	    if(x == u.ux+u.dx && y == u.uy+u.dy) goto stop;
	    continue;
	} else if (is_pool(x,y) || is_lava(x,y)) {
	    /* water and lava only stop you if directly in front, and stop
	     * you even if you are running
	     */
	    if(!Levitation && !is_flyer(uasmon) && !is_clinger(uasmon) &&
				x == u.ux+u.dx && y == u.uy+u.dy)
			/* No Wwalking check; otherwise they'd be able
			 * to test boots by trying to SHIFT-direction
			 * into a pool and seeing if the game allowed it
			 */
			goto stop;
	    continue;
	} else {		/* e.g. objects or trap or stairs */
	    if(flags.run == 1) goto bcorr;
	    if(mtmp) continue;		/* d */
	    if(((x == u.ux - u.dx) && (y != u.uy + u.dy)) ||
	       ((y == u.uy - u.dy) && (x != u.ux + u.dx)))
	       continue;
	}
stop:
	nomul(0);
	return;
    } /* end for loops */

    if(corrct > 1 && flags.run == 2) goto stop;
    if((flags.run == 1 || flags.run == 3) && !noturn && !m0 && i0 &&
				(corrct == 1 || (corrct == 2 && i0 == 1))) {
	/* make sure that we do not turn too far */
	if(i0 == 2) {
	    if(u.dx == y0-u.uy && u.dy == u.ux-x0)
		i = 2;		/* straight turn right */
	    else
		i = -2;		/* straight turn left */
	} else if(u.dx && u.dy) {
	    if((u.dx == u.dy && y0 == u.uy) || (u.dx != u.dy && y0 != u.uy))
		i = -1;		/* half turn left */
	    else
		i = 1;		/* half turn right */
	} else {
	    if((x0-u.ux == y0-u.uy && !u.dy) || (x0-u.ux != y0-u.uy && u.dy))
		i = 1;		/* half turn right */
	    else
		i = -1;		/* half turn left */
	}

	i += u.last_str_turn;
	if(i <= 2 && i >= -2) {
	    u.last_str_turn = i;
	    u.dx = x0-u.ux;
	    u.dy = y0-u.uy;
	}
    }
}

/* something like lookaround, but we are not running */
/* react only to monsters that might hit us */
int
monster_nearby()
{
	register int x,y;
	register struct monst *mtmp;

	/* Also see the similar check in dochugw() in monmove.c */
	for(x = u.ux-1; x <= u.ux+1; x++)
	    for(y = u.uy-1; y <= u.uy+1; y++) {
		if(!isok(x,y)) continue;
		if(x == u.ux && y == u.uy) continue;
		if((mtmp = m_at(x,y)) &&
		   mtmp->m_ap_type != M_AP_FURNITURE &&
		   mtmp->m_ap_type != M_AP_OBJECT &&
		   (!mtmp->mpeaceful || Hallucination) &&
		   (!is_hider(mtmp->data) || !mtmp->mundetected) &&
		   !noattacks(mtmp->data) &&
		   mtmp->mcanmove && !mtmp->msleep &&  /* aplvax!jcn */
		   !onscary(u.ux, u.uy, mtmp) &&
		   canspotmon(mtmp))
			return(1);
	}
	return(0);
}

void
nomul(nval)
	register int nval;
{
	if(multi < nval) return;	/* This is a bug fix by ab@unido */
	u.uinvulnerable = FALSE;	/* Kludge to avoid ctrl-C bug -dlc */
	u.usleep = 0;
	multi = nval;
	flags.mv = flags.run = 0;
}

/* called when a non-movement, multi-turn action has completed */
void unmul(msg_override)
const char *msg_override;
{
	multi = 0;	/* caller will usually have done this already */
	if (msg_override) nomovemsg = msg_override;
	else if (!nomovemsg) nomovemsg = "You can move again.";
	if (*nomovemsg) pline(nomovemsg);
	nomovemsg = 0;
	u.usleep = 0;
	if (afternmv) (*afternmv)();
	afternmv = 0;
}

#endif /* OVL2 */
#ifdef OVL1

void
losehp(n, knam, k_format)
register int n;
register const char *knam;
boolean k_format;
{
	if (u.mtimedone) {
		u.mh -= n;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		flags.botl = 1;
		if (u.mh < 1) rehumanize();
		return;
	}

	u.uhp -= n;
	if(u.uhp > u.uhpmax)
		u.uhpmax = u.uhp;	/* perhaps n was negative */
	flags.botl = 1;
	if(u.uhp < 1) {
		killer_format = k_format;
		killer = knam;		/* the thing that killed you */
		You("die...");
		done(DIED);
	} else if(u.uhp*10 < u.uhpmax && moves-wailmsg > 50 && n > 0){
		wailmsg = moves;
		if (Role_is('W') || Role_is('E') || Role_is('V')) {
			if (u.uhp == 1)
				pline("%s is about to die.", pl_character);
			else if (4 <= (!!(HTeleportation & INTRINSIC)) +
				    (!!(HSee_invisible & INTRINSIC)) +
				    (!!(HPoison_resistance & INTRINSIC)) +
				    (!!(HCold_resistance & INTRINSIC)) +
				    (!!(HShock_resistance & INTRINSIC)) +
				    (!!(HFire_resistance & INTRINSIC)) +
				    (!!(HSleep_resistance & INTRINSIC)) +
				    (!!(HDisint_resistance & INTRINSIC)) +
				    (!!(HTeleport_control & INTRINSIC)) +
				    (!!(Stealth & INTRINSIC)) +
				    (!!(Fast & INTRINSIC)) +
				    (!!(HInvis & INTRINSIC)))
				pline("%s, all your powers will be lost...",
					pl_character);
			else
				pline("%s, your life force is running out.",
					pl_character);
		} else {
			if(u.uhp == 1)
				You_hear("the wailing of the Banshee...");
			else
				You_hear("the howling of the CwnAnnwn...");
		}
	}
}

int
weight_cap()
{
	register long carrcap;

	carrcap = (((ACURRSTR + ACURR(A_CON))/2)+1)*50;
	if (u.mtimedone) {
		/* consistent with can_carry() in mon.c */
		if (u.usym == S_NYMPH)
		        carrcap = MAX_CARR_CAP;
		else if (!uasmon->cwt)
			carrcap = (carrcap * (long)uasmon->msize) / MZ_HUMAN;
		else if (!strongmonst(uasmon)
			|| (strongmonst(uasmon) && (uasmon->cwt > WT_HUMAN)))
			carrcap = (carrcap * (long)uasmon->cwt / WT_HUMAN);
	}

	if(Levitation || Is_airlevel(&u.uz))	/* pugh@cornell */
		carrcap = MAX_CARR_CAP;
	else {
		if(carrcap > MAX_CARR_CAP) carrcap = MAX_CARR_CAP;
		if (!is_flyer(uasmon)) {
			if(Wounded_legs & LEFT_SIDE) carrcap -= 100;
			if(Wounded_legs & RIGHT_SIDE) carrcap -= 100;
		}
		if (carrcap < 0) carrcap = 0;
	}
	return((int) carrcap);
}

static int wc;	/* current weight_cap(); valid after call to inv_weight() */

/* returns how far beyond the normal capacity the player is currently. */
/* inv_weight() is negative if the player is below normal capacity. */
int
inv_weight()
{
	register struct obj *otmp = invent;
	register int wt;

	/* when putting stuff into containers, gold is inserted at the head
	   of invent for easier manipulation by askchain & co, but it's also
	   retained in u.ugold in order to keep the status line accurate; we
	   mustn't add its weight in twice under that circumstance */
	wt = (otmp && otmp->oclass == GOLD_CLASS) ? 0 :
		(int)((u.ugold + 50L) / 100L);

	while (otmp) {
		if (otmp->otyp != BOULDER || !throws_rocks(uasmon))
			wt += otmp->owt;
		otmp = otmp->nobj;
	}
	wc = weight_cap();
	return (wt - wc);
}

/*
 * Returns 0 if below normal capacity, or the number of "capacity units"
 * over the normal capacity the player is loaded.  Max is 5.
 */
int
calc_capacity(xtra_wt)
int xtra_wt;
{
    int cap, wt = inv_weight() + xtra_wt;

    if (wt <= 0) return UNENCUMBERED;
    if (wc <= 1) return OVERLOADED;
    cap = (wt*2 / wc) + 1;
    return min(cap, OVERLOADED);
}

int
near_capacity()
{
    return calc_capacity(0);
}

int
max_capacity()
{
    int wt = inv_weight();

    return (wt - (2 * wc));
}

boolean
check_capacity(str)
const char *str;
{
    if(near_capacity() >= EXT_ENCUMBER) {
	if(str)
	    pline(str);
	else
	    You_cant("do that while carrying so much stuff.");
	return 1;
    }
    return 0;
}

#endif /* OVL1 */
#ifdef OVLB

int
inv_cnt()
{
	register struct obj *otmp = invent;
	register int ct = 0;

	while(otmp){
		ct++;
		otmp = otmp->nobj;
	}
	return(ct);
}

#endif /* OVLB */

/*hack.c*/
