/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#if defined(UNIX) && !defined(LINT)
static	const char	SCCS_Id[] = "@(#)hack.c	3.0\t89/11/20";
#endif

STATIC_DCL int NDECL(moverock);
#ifdef SINKS
STATIC_DCL void NDECL(dosinkfall);
#endif

#ifdef OVL1
static boolean FDECL(is_edge,(XCHAR_P,XCHAR_P));
static boolean FDECL(bad_rock,(XCHAR_P,XCHAR_P));
#endif /* OVL1 */

#ifdef OVLB

/* called on movement:
	1. when throwing ball+chain far away
	2. when teleporting
	3. when walking out of a lit room
 */
void
unsee() {
	register xchar x,y;
	register struct rm *lev;

	if(seehx){
		seehx = 0;
	} 
	/*
	 *  Erase surrounding positions if needed.  We don't need to do this
	 *  if we are blind, since we can't see them anyway.  This removes the
	 *  pl6 bug that makes monsters disappear if they are next to you if
	 *  you teleport while blind and telepathic.
	 */
	else if(!Blind)
	    for(x = u.ux-1; x < u.ux+2; x++)
	        for(y = u.uy-1; y < u.uy+2; y++) {
		    if(!isok(x, y)) continue;
		    lev = &levl[x][y];
		    if(!lev->lit && lev->scrsym == ROOM_SYM) {
			lev->scrsym = STONE_SYM;
			lev->new = 1;
			on_scr(x,y);
		    }
	        }
}

/* called:
	in apply.c:  seeoff(0)	- when taking a picture of yourself
				- when donning a blindfold
	in do.c:     seeoff(0)	- blind after drinking potion
	in do.c:     seeoff(1)	- go up or down the stairs
	in eat.c:    seeoff(0)	- blind after eating rotten food
	in mhitu.c:  seeoff(0)	- blinded by a glowing eye
	in mhitu.c:  seeoff(1)	- swallowed
	in mthrow.c: seeoff(0)	- hit by a cream pie.
	in potion.c: seeoff(0)	- quaffing or sniffing a potion of blindness
	in spell.c:  seeoff(0)	- due to a cursed spell book
	in trap.c:   seeoff(1)	- fall through trap door
 */
void
seeoff(mode)
int mode;
{		/* 1 to redo @, 0 to leave them */
		/* 1 means misc movement, 0 means blindness */
	register xchar x,y;
	register struct rm *lev;

	if(u.udispl && mode){
		u.udispl = 0;
		levl[u.udisx][u.udisy].scrsym = news0(u.udisx,u.udisy);
	}
	if(seehx) {
		seehx = 0;
	} else
	    if(!mode) {
		for(x = u.ux-1; x < u.ux+2; x++)
		    for(y = u.uy-1; y < u.uy+2; y++) {
			if(!isok(x, y)) continue;
			lev = &levl[x][y];
			if(MON_AT(x, y))
			    unpmon(m_at(x,y));
			if(!lev->lit && lev->scrsym == ROOM_SYM) {
			    lev->seen = 0;
			    atl(x, y, (char)STONE_SYM);
			}
		    }
	    }
}

#endif /* OVLB */
#ifdef OVL2

STATIC_OVL int
moverock() {
	register xchar rx, ry;
	register struct obj *otmp, *otmp2;
	register struct trap *ttmp;
	register struct	monst *mtmp;

#ifdef POLYSELF
	if (passes_walls(uasmon)) return 0;
#endif
	while(otmp = sobj_at(BOULDER, u.ux+u.dx, u.uy+u.dy)) {
		rx = u.ux+2*u.dx;
		ry = u.uy+2*u.dy;
		nomul(0);
#ifdef POLYSELF
		if (verysmall(uasmon)) {
		    pline("You're too small to push that boulder.");
		    goto cannot_push;
		}
#endif
		if(isok(rx,ry) && !IS_ROCK(levl[rx][ry].typ) &&
		    (!IS_DOOR(levl[rx][ry].typ) || !(u.dx && u.dy) || (
#ifdef REINCARNATION
			dlevel != rogue_level &&
#endif
		     (levl[rx][ry].doormask & ~D_BROKEN) == D_NODOOR)) &&
		    !sobj_at(BOULDER, rx, ry)) {
			if(MON_AT(rx, ry)) {
			    mtmp = m_at(rx,ry);
			    if(canseemon(mtmp))
				pline("There's %s on the other side.",
				      mon_nam(mtmp));
			    else
				You("hear a monster behind the boulder.");
			    if(flags.verbose)
				pline("Perhaps that's why you cannot move it.");
			    goto cannot_push;
			}
			if(ttmp = t_at(rx,ry))
			    switch(ttmp->ttyp) {
			    case SPIKED_PIT:
			    case PIT:
				You("push the boulder into a pit!");
				deltrap(ttmp);
				delobj(otmp);
				if(cansee(rx,ry)) newsym(rx,ry);
				else levl[rx][ry].seen = 0;
				if(flags.verbose)
				    pline("It completely fills the pit!");
				continue;
			    case TRAPDOOR:
				if(is_maze_lev
#ifdef STRONGHOLD
					&& (dlevel > stronghold_level)
#endif
					) break;
				pline("The boulder falls into and plugs a hole in the ground!");
				deltrap(ttmp);
				delobj(otmp);
				if(cansee(rx,ry)) newsym(rx,ry);
				else levl[rx][ry].seen = 0;
				continue;
			    case LEVEL_TELEP:
			    case TELEP_TRAP:
				You("push the boulder and suddenly it disappears!");
				rloco(otmp);
				continue;
			    }
			if(closed_door(rx, ry))
				goto nopushmsg;
			if(is_pool(rx,ry)) {
#ifdef STRONGHOLD
				if(levl[rx][ry].typ == DRAWBRIDGE_UP)
				    levl[rx][ry].drawbridgemask |= DB_FLOOR;
				else
#endif
				    levl[rx][ry].typ = ROOM;
				mnewsym(rx,ry);
				if(!Blind) prl(rx,ry);
				You("push the boulder into the water.");
				if(flags.verbose && !Blind)
				    pline("Now you can cross the water!");
				delobj(otmp);
				continue;
			}
				/*
				 * Re-link at top of fobj chain so that 
				 * pile order is preserved when level is 
				 * restored.
				 */
			if (otmp != fobj) {
				otmp2 = fobj;
				while (otmp2->nobj && otmp2->nobj != otmp) 
					otmp2 = otmp2->nobj;
				if (!otmp2->nobj)
				    impossible("moverock: error in fobj chain");
				else {
					otmp2->nobj = otmp->nobj;	
					otmp->nobj = fobj;
					fobj = otmp;
				}
			}
			move_object(otmp, rx, ry);
			/* pobj(otmp); */
			if(cansee(rx,ry)) atl(rx,ry,otmp->olet);
			newsym(u.ux+u.dx, u.uy+u.dy);

			{
#ifdef LINT	/* static long lastmovetime; */
			long lastmovetime;
			lastmovetime = 0;
#else
			static long NEARDATA lastmovetime;
#endif
			/* note: this var contains garbage initially and
			   after a restore */
			if(moves > lastmovetime+2 || moves < lastmovetime)
			pline("With great effort you move the boulder.");
			lastmovetime = moves;
			}
		} else {
nopushmsg:
		    You("try to move the boulder, but in vain.");
	    cannot_push:
#ifdef POLYSELF
		    if (throws_rocks(uasmon)) {
			if(!flags.pickup)
				pline("However, you easily can push it aside.");
			else
				pline("However, you easily can pick it up.");
			break;
		    }
#endif
		    if((!invent || inv_weight()+90 <= 0) &&
			(!u.dx || !u.dy || (IS_ROCK(levl[u.ux][u.uy+u.dy].typ)
					&& IS_ROCK(levl[u.ux+u.dx][u.uy].typ)))
#ifdef POLYSELF
			|| verysmall(uasmon)
#endif
									)
		    {
			pline("However, you can squeeze yourself into a small opening.");
			break;
		    } else
			return (-1);
		}
	}
	return (0);
}

#endif /* OVL2 */
#ifdef OVLB

void
movobj(obj, ox, oy)
register struct obj *obj;
register xchar ox, oy;
{
	remove_object(obj);
	if (cansee(obj->ox, obj->oy)) {
		levl[obj->ox][obj->oy].seen = 0;
		prl(obj->ox, obj->oy);
	} else
		newsym(obj->ox, obj->oy);
	place_object(obj, ox, oy);
	if (cansee(ox, oy)) {
		levl[ox][oy].seen = 0;
		prl(ox, oy);
	} else
		newsym(ox, oy);
}

#ifdef SINKS
STATIC_OVL
void
dosinkfall() {
	register struct obj *obj;

# ifdef POLYSELF
	if (is_floater(uasmon)) {
		You("wobble unsteadily for a moment.");
	} else {
# endif
		You("crash to the floor!");
		losehp((rn1(10, 20 - (int)ACURR(A_CON))),
			"fell onto a sink", NO_KILLER_PREFIX);
		for(obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
		    if(obj->olet == WEAPON_SYM) {
			You("fell on %s.",doname(obj));
			losehp(rn2(3),"fell onto a sink", NO_KILLER_PREFIX);
		    }
# ifdef POLYSELF
	}
# endif

	HLevitation = (HLevitation & ~TIMEOUT) + 1;
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

#endif /* OVLB */
#ifdef OVL1

static boolean
is_edge(x,y)
register xchar x,y;
/* return true if (x,y) is on the edge of a room
 * we cannot rely on IS_DOOR(levl[x][y].typ) because some of the stronghold
 * "rooms" are actually outside areas without doors
 */
{
	register int roomno = inroom(x,y);

	if(roomno < 0) return FALSE;
	return((x == rooms[roomno].lx - 1) || (x == rooms[roomno].hx + 1) ||
	       (y == rooms[roomno].ly - 1) || (y == rooms[roomno].hy + 1));
}

#endif /* OVL1 */
#ifdef OVLB

boolean
may_dig(x,y)
register xchar x,y;
/* intended to be called only on ROCKs */
{
return (!(IS_STWALL(levl[x][y].typ) && (levl[x][y].diggable & W_NONDIGGABLE)));
}

#endif /* OVLB */
#ifdef OVL1

static boolean
bad_rock(x,y)
register xchar x,y;
{
	return(IS_ROCK(levl[x][y].typ)
#ifdef POLYSELF
		    && !passes_walls(uasmon)
		    && (!tunnels(uasmon) || needspick(uasmon) || !may_dig(x,y))
#endif
	);
}

void
domove() {
	register struct monst *mtmp;
	register struct rm *tmpr,*ust;
	register xchar x,y;
	struct trap *trap;

	u_wipe_engr(rnd(5));

	if(inv_weight() > 0){
		You("collapse under your load.");
		nomul(0);
		return;
	}
	if(u.uswallow) {
		register xchar xx,yy;

		u.dx = u.dy = 0;
		xx = u.ux;
		yy = u.uy;
		x = u.ux = u.ustuck->mx;
		y = u.uy = u.ustuck->my;
		if(xx != x || yy != y) newsym(xx,yy);
		mtmp = u.ustuck;
	} else {
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
			} while(!isok(x, y) || bad_rock(x, y));
		}
		if(!isok(x, y)) {
			nomul(0);
			return;
		}
		if((trap = t_at(x, y)) && trap->tseen)
			nomul(0);
		if(u.ustuck && (x != u.ustuck->mx ||
				y != u.ustuck->my)) {
			if(dist(u.ustuck->mx, u.ustuck->my) > 2) {
			/* perhaps it fled (or was teleported or ... ) */
				u.ustuck = 0;
			} else {
#ifdef POLYSELF
				/* If polymorphed into a sticking monster,
				 * u.ustuck means it's stuck to you, not you
				 * to it.
				 */
				if (sticks(uasmon)) {
					kludge("You release %s.",
						mon_nam(u.ustuck));
					u.ustuck = 0;
				} else {
#endif
					kludge("You cannot escape from %s!",
						mon_nam(u.ustuck));
					nomul(0);
					return;
#ifdef POLYSELF
				}
#endif
			}
		}
		mtmp = m_at(x,y);
		if (mtmp) {
			/* Don't attack if you're running */
			if (flags.run && !mtmp->mimic &&
				    (Blind ? Telepat :
					 (!mtmp->minvis || See_invisible))) {
				nomul(0);
				flags.move = 0;
				return;
			}
		}
	}

	u.ux0 = u.ux;
	u.uy0 = u.uy;
	tmpr = &levl[x][y];

	/* attack monster */
	if(mtmp) {
		nomul(0);
		gethungry();
		if(multi < 0) return;	/* we just fainted */

		/* try to attack; note that it might evade */
		if(attack(mtmp)) return;
	}

	/* not attacking an animal, so we try to move */
#ifdef POLYSELF
	if(!uasmon->mmove) {
		You("are rooted %s.", Levitation ? "in place"
			: "to the ground");
		nomul(0);
		return;
	}
#endif
	if(u.utrap) {
		if(u.utraptype == TT_PIT) {
		    if(flags.verbose)
			Norep("You are still in a pit.");
		    u.utrap--;
		} else if (u.utraptype == TT_WEB) {
		    if(flags.verbose)
		    	Norep("You are stuck to the web.");
		    u.utrap--;
		} else {
		    if(flags.verbose)
			Norep("You are caught in a bear trap.");
		    if((u.dx && u.dy) || !rn2(5)) u.utrap--;
		}
		return;
	}
	/* check for physical obstacles */
#ifdef POLYSELF
	if (!passes_walls(uasmon)) {
#endif
#ifdef STRONGHOLD
	    if(dlevel == stronghold_level && is_db_wall(x,y)) {
		    pline("The drawbridge is up!");
		    nomul(0);
		    return;
	    }
#endif
	    if(closed_door(x, y)) {
#ifdef POLYSELF
		if(amorphous(uasmon))
		    You("ooze under the door.");
		else {
#endif
		    flags.move = 0;
		    if(x == u.ux || y == u.uy) {
		        if (Blind || Stunned || ACURR(A_DEX) < 10 || Fumbling)
			    pline("Ouch!  You bump into a door.");
		         else pline("That door is closed.");
		    }
		    nomul(0);
		    return;
#ifdef POLYSELF
		}
#endif
	    }
#ifdef POLYSELF
	}
#endif
	ust = &levl[u.ux][u.uy];

	if(bad_rock(x,y) ||
	   (u.dx && u.dy
#ifdef POLYSELF
		 && !passes_walls(uasmon)
#endif
		 && ( (IS_DOOR(ust->typ) && block_entry(x, y)) ||
#ifdef REINCARNATION
			(IS_DOOR(tmpr->typ) &&
			 ((tmpr->doormask & ~D_BROKEN) 
			   || dlevel == rogue_level || block_door(x, y))
                        )  
		     || (IS_DOOR(ust->typ) &&
			 ((ust->doormask & ~D_BROKEN) || dlevel == rogue_level)
                        )
#else
			(IS_DOOR(tmpr->typ) &&
			 ((tmpr->doormask & ~D_BROKEN) || block_door(x, y))) ||
			(IS_DOOR(ust->typ) && (ust->doormask & ~D_BROKEN))
#endif
		    )
	   )) {
		flags.move = 0;
		nomul(0);
		return;
	}
	if(moverock() < 0) return;
	if(u.dx && u.dy && bad_rock(u.ux,y) &&
			   bad_rock(x,u.uy)) {
#ifdef POLYSELF
	    if (bigmonst(uasmon)) {
		Your("body is too large to fit through.");
		nomul(0);
		return;
	    }
#endif
	    if (invent && inv_weight()+40 > 0) {
		You("are carrying too much to get through.");
		nomul(0);
		return;
	    }
	}
	if(Punished &&
	   dist2(x, y, uchain->ox, uchain->oy) > 2) {
		if(carried(uball)) {
			movobj(uchain, u.ux, u.uy);
			goto nodrag;
		}

		if(dist2(x, y, uball->ox, uball->oy) < 3) {
			/* leave ball, move chain under/over ball */
			movobj(uchain, uball->ox, uball->oy);
			goto nodrag;
		}

		if(inv_weight() + (int)(uball->owt >> 1) > 0) {
			You("cannot %sdrag the heavy iron ball.",
			invent ? "carry all that and also " : "");
			nomul(0);
			return;
		}

		movobj(uball, uchain->ox, uchain->oy);
		movobj(uchain, u.ux, u.uy);
		nomul(-2);
		nomovemsg = "";
	nodrag:	;
	}
#ifdef POLYSELF
	if (tunnels(uasmon) && !needspick(uasmon) && IS_ROCK(tmpr->typ)) {
		static const char NEARDATA *digtxt;

		if(dig_pos.x != x || dig_pos.y != y
		    || dig_level != dlevel || dig_down) {
			dig_down = FALSE;
			dig_pos.x = x;
			dig_pos.y = y;
			dig_level = dlevel;
			dig_effort = 30;
			You("start chewing a hole in the rock.");
			return;
		} else if ((dig_effort += 30) < 100)  {
		    if(flags.verbose)
			You("continue chewing the rock up.");
		    return;
		} else {
			if (IS_WALL(tmpr->typ)) {
				digtxt = "You chew a hole in the wall.";
				if(!is_maze_lev)
				  tmpr->typ = DOOR;
				else
				  tmpr->typ = ROOM;
			} else if (tmpr->typ==SDOOR) {
				digtxt = "You chew through a secret door.";
				tmpr->typ = DOOR;
				if(!(tmpr->doormask & D_TRAPPED))
					tmpr->doormask = D_BROKEN;
			} else {
				digtxt = "You chew a passage through the rock.";
				tmpr->typ = CORR;
			}
			mnewsym(x, y);
			prl(x, y);
			pline(digtxt);
			if(IS_DOOR(tmpr->typ) && (tmpr->doormask & D_TRAPPED)) {
				b_trapped("door");
				tmpr->doormask = D_NODOOR;
				mnewsym(x, y);
				prl(x, y);
			}
			dig_level = -1;
		}
	}
#endif
	u.ux += u.dx;
	u.uy += u.dy;
	reset_occupations();
	if(flags.run) {
		if(IS_DOOR(tmpr->typ) ||
#ifdef POLYSELF
		(IS_ROCK(tmpr->typ)) ||
#endif
		(xupstair == u.ux && yupstair == u.uy) ||
		(xdnstair == u.ux && ydnstair == u.uy)
#ifdef STRONGHOLD
		|| (xupladder == u.ux && yupladder == u.uy)
		|| (xdnladder == u.ux && ydnladder == u.uy)
#endif
#ifdef FOUNTAINS
		|| IS_FOUNTAIN(tmpr->typ)
#endif
#ifdef THRONES
		|| IS_THRONE(tmpr->typ)
#endif
#ifdef SINKS
		|| IS_SINK(tmpr->typ)
#endif
#ifdef ALTARS
		|| IS_ALTAR(tmpr->typ)
#endif
		)
			nomul(0);
	}
#ifdef POLYSELF
	if (hides_under(uasmon))
	    u.uundetected = (OBJ_AT(u.ux, u.uy) || levl[u.ux][u.uy].gmask);
	else if (u.dx || u.dy) { /* piercer */
	    if (u.usym == S_MIMIC_DEF)
		u.usym = S_MIMIC;
	    u.uundetected = 0;
	}
#endif

/*
	if(u.udispl) {
		u.udispl = 0;
		newsym(u.ux0,u.uy0);
	}
*/
	if(!Blind) {
	    register xchar backx = u.ux0 - u.dx;   /* one step beyond old pos */
	    register xchar backy = u.uy0 - u.dy;
	    register xchar frontx = u.ux + u.dx;   /* one step beyond new pos */
	    register xchar fronty = u.uy + u.dy;
	    register boolean newedge = is_edge(u.ux,u.uy);
	    register boolean oldedge = is_edge(u.ux0,u.uy0);

	    /* ust is old position, tmpr is new position */
	    if(oldedge && newedge && inroom(u.ux0,u.uy0) == inroom(u.ux,u.uy)) {
		/* moving along wall */
		nose1(backx,backy);
		prl1(frontx,fronty);

	    } else if(oldedge || newedge) {
		if(isok(backx,backy) && levl[backx][backy].lit)
		    setsee();
		else
		    nose1(backx,backy);

		if(isok(frontx,fronty) && levl[frontx][fronty].lit)
		    setsee();
		else {
		    prl1(frontx,fronty);
		    prl1(u.ux,u.uy);    /* make sure side walls are seen */
		}

	    } else if(!tmpr->lit) {
		/* we haven't crossed an edge, so old & new are both light or
		 * both dark.  if both light, we need do nothing.
		 */
		nose1(backx,backy);
		prl1(frontx,fronty);
	    }

	} else {
		pru();
	}
#ifdef WALKIES
	check_leash(u.ux0,u.uy0);
#endif
	if(u.ux0 != u.ux || u.uy0 != u.uy) u.umoved = TRUE;
	spoteffects();
}

#endif /* OVL1 */
#ifdef OVL2

void
spoteffects()
{
	register struct trap *trap;

	if(is_pool(u.ux,u.uy) && !(Levitation || Wwalking
#ifdef POLYSELF
	    || is_flyer(uasmon)
#endif
	    ))
		drown();	/* not necessarily fatal */
	else {
		(void) inshop();
#ifdef SINKS
		if(IS_SINK(levl[u.ux][u.uy].typ) && Levitation)
			dosinkfall();
#endif
		if(!flags.nopick &&
		   (OBJ_AT(u.ux, u.uy) || levl[u.ux][u.uy].gmask))
			pickup(1);
		else read_engr_at(u.ux,u.uy);
		if(trap = t_at(u.ux,u.uy))
			dotrap(trap);	/* fall into pit, arrow trap, etc. */
	}

}

#endif /* OVL2 */
#ifdef OVLB

int
dopickup() {
	/* uswallow case added by GAN 01/29/87 */
	if(u.uswallow)  {
		if (is_animal(u.ustuck->data)) {
		    You("pick up %s's tongue.", mon_nam(u.ustuck));
		    pline("But it's kind of slimy, so you drop it.");
		} else
		    You("don't %s anything in here to pick up.",
			  Blind ? "feel" : "see");
		return(1);
	}
	if(!OBJ_AT(u.ux, u.uy) && levl[u.ux][u.uy].gmask == 0) {
		pline("There is nothing here to pick up.");
		return(0);
	}
	if(Levitation) {
		You("cannot reach the floor.");
		return(1);
	}
	pickup(0);
	return(1);
}

#endif /* OVLB */
#ifdef OVL2

/* stop running if we see something interesting */
/* turn around a corner if that is the only way we can proceed */
/* do not turn left or right twice */
void
lookaround() {
	register int x, y, i, x0, y0, m0, i0 = 9, corrct = 0, noturn = 0;
	register struct monst *mtmp;
#if defined(LINT) || defined(__GNULINT__)
	/* suppress "used before set" message */
	x0 = y0 = m0 = 0;
#endif
#ifdef POLYSELF
	/* Grid bugs stop if trying to move diagonal, even if blind.  Maybe */
	/* they polymorphed while in the middle of a long move. */
	if (u.umonnum == PM_GRID_BUG && u.dx && u.dy) {
		nomul(0);
		return;
	}
#endif
	if(Blind || flags.run == 0) return;
	for(x = u.ux-1; x <= u.ux+1; x++) for(y = u.uy-1; y <= u.uy+1; y++) {
		if(!isok(x,y)) continue;
#ifdef POLYSELF
		if(u.umonnum == PM_GRID_BUG && x != u.ux && y != u.uy) continue;
#endif
		if(x == u.ux && y == u.uy) continue;
		if((mtmp = m_at(x,y)) && !mtmp->mimic &&
		    (!mtmp->minvis || See_invisible) && !mtmp->mundetected) {
			if((flags.run != 1 && !mtmp->mtame)
					|| (x == u.ux+u.dx && y == u.uy+u.dy))
				goto stop;
		}
		if(levl[x][y].typ == STONE) continue;
		if(x == u.ux-u.dx && y == u.uy-u.dy) continue;
		{
		register uchar sym = levl[x][y].scrsym;

		if (IS_ROCK(levl[x][y].typ) ||
		   (sym == ROOM_SYM && !IS_DOOR(levl[x][y].typ)))
		     continue;
		else if (sym == CLOSED_DOOR_SYM) {
			if(x != u.ux && y != u.uy) continue;
			if(flags.run != 1) goto stop;
			goto bcorr;
		} else if (sym == CORR_SYM) {
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
		} else if (sym == TRAP_SYM) {
			if(flags.run == 1) goto bcorr;	/* if you must */
			if(x == u.ux+u.dx && y == u.uy+u.dy) goto stop;
			continue;
		} else if (sym == POOL_SYM) {
			/* pools only stop you if directly in front, and stop
			 * you even if you are running
			 */
			if(!Levitation &&
#ifdef POLYSELF
				!is_flyer(uasmon) &&
#endif
				/* No Wwalking check; otherwise they'd be able
				 * to test boots by trying to SHIFT-direction
				 * into a pool and seeing if the game allowed it
				 */
				x == u.ux+u.dx && y == u.uy+u.dy) goto stop;
			continue;
		} else {		/* e.g. objects or trap or stairs */
			if(flags.run == 1) goto bcorr;
			if(mtmp) continue;		/* d */
		}
		stop:
			nomul(0);
			return;
		}
	}
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
		    if((u.dx == u.dy && y0 == u.uy) ||
			(u.dx != u.dy && y0 != u.uy))
			i = -1;		/* half turn left */
		    else
			i = 1;		/* half turn right */
		} else {
		    if((x0-u.ux == y0-u.uy && !u.dy) ||
			(x0-u.ux != y0-u.uy && u.dy))
			i = 1;		/* half turn right */
		    else
			i = -1;		/* half turn left */
		}
		i += u.last_str_turn;
		if(i <= 2 && i >= -2) {
			u.last_str_turn = i;
			u.dx = x0-u.ux, u.dy = y0-u.uy;
		}
	}
}

/* something like lookaround, but we are not running */
/* react only to monsters that might hit us */
int
monster_nearby() {
	register int x,y;
	register struct monst *mtmp;

	if(!Blind)
	for(x = u.ux-1; x <= u.ux+1; x++)
	    for(y = u.uy-1; y <= u.uy+1; y++) {
		if(!isok(x,y)) continue;
		if(x == u.ux && y == u.uy) continue;
		if((mtmp = m_at(x,y)) && !mtmp->mimic &&
		   !mtmp->mtame && !mtmp->mpeaceful &&
		   !noattacks(mtmp->data) &&
		   mtmp->mcanmove && !mtmp->msleep &&  /* aplvax!jcn */
		   (!mtmp->minvis || See_invisible) &&
		   !onscary(u.ux, u.uy, mtmp))
			return(1);
	}
	return(0);
}

#endif /* OVL2 */
#ifdef OVL0

int
cansee(x,y)
xchar x,y;
{
	if(Blind || (u.uswallow && (x != u.ux || y != u.uy))) return(0);
	if(IS_ROCK(levl[x][y].typ) && levl[u.ux][u.uy].typ == CORR &&
				!MON_AT(x, y) && !levl[u.ux][u.uy].lit)
		return(0);
	if(dist(x,y) < 3) return(1);
	if(levl[x][y].lit &&
		((seelx <= x && x <= seehx && seely <= y && y <= seehy) ||
		(seelx2 <= x && x <= seehx2 && seely2 <= y && y <= seehy2)))
		return(1);
	return(0);
}

#endif /* OVL0 */
#ifdef OVL1

int
sgn(a)
	register int a;
{
	return((a > 0) ? 1 : (a == 0) ? 0 : -1);
}

#endif /* OVL1 */
#ifdef OVL2

void
getcorners(lx1,hx1,ly1,hy1,lx2,hx2,ly2,hy2)
xchar *lx1,*hx1,*ly1,*hy1,*lx2,*hx2,*ly2,*hy2;
/* return corners of one or two rooms player is in, so we can tell what areas
 * can be seen, or otherwise affected by room-specific things.  (two rooms are
 * possible when in a doorway of the stronghold)
 * the player is already known to be in at least one room
 */
{
	register int uroom1,uroom2;
	register xchar ux,uy;

	uroom1 = inroom(u.ux,u.uy);
	*lx1 = rooms[uroom1].lx - 1;
	*hx1 = rooms[uroom1].hx + 1;
	*ly1 = rooms[uroom1].ly - 1;
	*hy1 = rooms[uroom1].hy + 1;

	if(!IS_DOOR(levl[u.ux][u.uy].typ)) {
		*lx2 = 1;
		*hx2 = 0;
		*ly2 = 1;
		*hy2 = 0;
	} else {
		for(ux = u.ux-1; ux <= u.ux+1; ux++)
			for(uy = u.uy-1; uy <= u.uy+1; uy++) {
				if(!isok(ux,uy)) continue;
				if(IS_ROCK(levl[ux][uy].typ) ||
					IS_DOOR(levl[ux][uy].typ)) continue;
				/* might have side-by-side walls, in which case
				 * should only be able to see one room */
				uroom2 = inroom(ux,uy);
				if(uroom2 >= 0 && uroom2 != uroom1 && 
				   rooms[uroom2].rlit) {
					*lx2 = rooms[uroom2].lx - 1;
					*ly2 = rooms[uroom2].ly - 1;
					*hx2 = rooms[uroom2].hx + 1;
					*hy2 = rooms[uroom2].hy + 1;
					return;
				}
			}
		*lx2 = 1;
		*hx2 = 0;
		*ly2 = 1;
		*hy2 = 0;
	}
}

#endif /* OVL2 */
#ifdef OVL1

void
setsee() {
	register int x, y;

	if(Blind) {
		pru();
		return;
	}
	if(!levl[u.ux][u.uy].lit) {
		seelx = u.ux-1;
		seehx = u.ux+1;
		seely = u.uy-1;
		seehy = u.uy+1;
		seelx2 = seely2 = 1;
		seehx2 = seehy2 = 0;
	} else {
		getcorners(&seelx,&seehx,&seely,&seehy,
			   &seelx2,&seehx2,&seely2,&seehy2);
	}
	for(y = seely; y <= seehy; y++)
		for(x = seelx; x <= seehx; x++) {
			prl(x,y);
	}
	for(y = seely2; y <= seehy2; y++)
		for(x = seelx2; x <= seehx2; x++) {
			prl(x,y);
	}
	if(!levl[u.ux][u.uy].lit) seehx = 0; /* seems necessary elsewhere */
	else {
	    if(seely == u.uy) for(x = u.ux-1; x <= u.ux+1; x++) prl(x,seely-1);
	    if(seehy == u.uy) for(x = u.ux-1; x <= u.ux+1; x++) prl(x,seehy+1);
	    if(seelx == u.ux) for(y = u.uy-1; y <= u.uy+1; y++) prl(seelx-1,y);
	    if(seehx == u.ux) for(y = u.uy-1; y <= u.uy+1; y++) prl(seehx+1,y);
	}
}

#endif /* OVL1 */
#ifdef OVL2

void
nomul(nval)
	register int nval;
{
	if(multi < nval) return;	/* This is a bug fix by ab@unido */
	multi = nval;
	flags.mv = flags.run = 0;
}

#endif /* OVL2 */
#ifdef OVL1

void
losehp(n, knam, k_format)
register int n;
register const char *knam;
boolean k_format;
{
#ifdef POLYSELF
	if (u.mtimedone) {
		u.mh -= n;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		flags.botl = 1;
		if (u.mh < 1) rehumanize();
		return;
	}
#endif
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
		if(index("WEV", pl_character[0])) {
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
				You("hear the wailing of the Banshee...");
			else
				You("hear the howling of the CwnAnnwn...");
		}
	}
}

int
weight_cap() {
	register int carrcap;

#ifdef HARD
	carrcap = 5 * (((ACURR(A_STR) > 18) ? 20 : ACURR(A_STR)) + u.ulevel);
#else
	carrcap = 5 * u.ulevel;      /* New strength stewr 870807 */
	if (ACURR(A_STR) < 19) carrcap += 5 * ACURR(A_STR);
	if (ACURR(A_STR) > 18) carrcap += ACURR(A_STR) - 18 + 90;
	if (ACURR(A_STR) > 68) carrcap += ACURR(A_STR) - 68;
	if (ACURR(A_STR) > 93) carrcap += ACURR(A_STR) - 93;
	if (ACURR(A_STR) > 108) carrcap += 2 * (ACURR(A_STR) - 108);
	if (ACURR(A_STR) > 113) carrcap += 5 * (ACURR(A_STR) - 113);
	if (ACURR(A_STR) == 118) carrcap += 100;
#endif
#ifdef POLYSELF
	if (u.mtimedone) {
		/* consistent with can_carry() in mon.c */
		if (u.usym==S_NYMPH) carrcap = MAX_CARR_CAP;
		else if (!uasmon->cwt)
			carrcap = (carrcap * uasmon->mlevel * 6)/45;
		else if (!strongmonst(uasmon)
			|| (strongmonst(uasmon) && (uasmon->cwt > 45)))
			carrcap = (carrcap * uasmon->cwt / 45);
	}
#endif
	if(Levitation) 			/* pugh@cornell */
		carrcap = MAX_CARR_CAP;
	else {
		if(carrcap > MAX_CARR_CAP) carrcap = MAX_CARR_CAP;
		if(Wounded_legs & LEFT_SIDE) carrcap -= 10;
		if(Wounded_legs & RIGHT_SIDE) carrcap -= 10;
	}
	return(carrcap);
}

int
inv_weight() {
	register struct obj *otmp = invent;
#ifdef LINT	/* long to int conversion */
	register int wt = 0;
#else
	register int wt = (int)((u.ugold + 500L)/1000L);
#endif /* LINT */
	while(otmp){
#ifdef POLYSELF
		if (otmp->otyp != BOULDER || !throws_rocks(uasmon))
#endif
			wt += otmp->owt;
		otmp = otmp->nobj;
	}
	return(wt - weight_cap());
}

#endif /* OVL1 */
#ifdef OVLB

int
inv_cnt() {
	register struct obj *otmp = invent;
	register int ct = 0;

	while(otmp){
		ct++;
		otmp = otmp->nobj;
	}
	return(ct);
}

int
identify(otmp)		/* also called by newmail() */
	register struct obj *otmp;
{
	makeknown(otmp->otyp);
	otmp->known = otmp->dknown = otmp->bknown = 1;
	prinv(otmp);
	return(1);
}

#ifdef STUPID_CPP	/* otherwise these functions are macros in hack.h */
char
yn() {
	return(yn_function(ynchars, 'n'));
}

char
ynq() {
	return(yn_function(ynqchars, 'q'));
}

char
ynaq() {
	return(yn_function(ynaqchars, 'y'));
}

char
nyaq() {
	return(yn_function(nyaqchars, 'n'));
}

int
max(a,b) int a,b; {
	return((a > b) ? a : b);
}

int
min(a,b) int a,b; {
	return((a < b) ? a : b);
}

char *
plur(x) long x; {
	return((x == 1L) ? "" : "s");
}

void
makeknown(x) unsigned x; {
	objects[x].oc_name_known = 1;
}
#endif /* STUPID_CPP */

#endif /* OVLB */
