/*	SCCS Id: @(#)lock.c	3.0	88/10/22
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

STATIC_PTR int NDECL(picklock);
STATIC_PTR int NDECL(forcelock);

STATIC_VAR struct xlock_s {
	int	door_or_box, picktyp;
	struct rm  *door;
	struct obj *box;
	int chance, usedtime;
} NEARDATA xlock;

#ifdef OVLB

static boolean FDECL(obstructed,(int,int));

STATIC_PTR
int
picklock() {	/* try to open/close a lock */

	if(!xlock.door_or_box) {	/* box */

	    if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy)) {
		return((xlock.usedtime = 0));		/* you or it moved */
	    }
	} else {		/* door */
	    if(xlock.door != &(levl[u.ux+u.dx][u.uy+u.dy])) {
		return((xlock.usedtime = 0));		/* you moved */
	    }
	    switch (xlock.door->doormask) {
		case D_NODOOR:
		    pline("This doorway has no door.");
		    return((xlock.usedtime = 0));
		case D_ISOPEN:
		    pline("Picking the lock of an open door is pointless.");
		    return((xlock.usedtime = 0));
		case D_BROKEN:
		    pline("This door is broken.");
		    return((xlock.usedtime = 0));
	    }
	}

	if(xlock.usedtime++ >= 50
#ifdef POLYSELF
	   || nohands(uasmon)
#endif
	   ) {
	    You("give up your attempt to %s the lock.",
		  (xlock.door_or_box ? !(xlock.door->doormask & D_LOCKED) :
		   !xlock.box->olocked) ? "lock" :
		  ((xlock.picktyp == LOCK_PICK) ? "pick" : "open" ));

	    return((xlock.usedtime = 0));
	}

	if(rn2(100) > xlock.chance) return(1);		/* still busy */

	if(xlock.door_or_box) {
	    You("succeed in %sing the lock.",
		  !(xlock.door->doormask & D_LOCKED) ? "lock" :
		  ((xlock.picktyp == LOCK_PICK) ? "pick" : "open" ));
	    if(xlock.door->doormask & D_TRAPPED) {
		    b_trapped("door");
		    xlock.door->doormask = D_NODOOR;
		    mnewsym(u.ux+u.dx, u.uy+u.dy);
		    prl(u.ux+u.dx, u.uy+u.dy);
	    } else if(xlock.door->doormask == D_LOCKED)
		xlock.door->doormask = D_CLOSED;
	    else xlock.door->doormask = D_LOCKED;
	} else {
	    You("succeed in %sing the lock.",
		  (!xlock.box->olocked) ? "lock" :
		  (xlock.picktyp == LOCK_PICK) ? "pick" : "open" );
	    xlock.box->olocked = !xlock.box->olocked;
	    if(xlock.box->otrapped)	chest_trap(xlock.box, FINGER);
	}
	return((xlock.usedtime = 0));
}

STATIC_PTR
int
forcelock() {	/* try to force a locked chest */

	register struct obj *otmp, *otmp2;
	register struct obj *probj = fcobj;  /* initialize to make lint happy */

	if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy))
		return((xlock.usedtime = 0));		/* you or it moved */

	if(xlock.usedtime++ >= 50 || !uwep
#ifdef POLYSELF
	   || nohands(uasmon)
#endif
	   ) {
	    You("give up your attempt to force the lock.");

	    return((xlock.usedtime = 0));
	}

	if(xlock.picktyp) {	/* blade */

	    if(rn2(1000-uwep->spe) > 992 && !uwep->cursed) {
		/* for a +0 weapon, probability that it survives an unsuccessful
		 * attempt to force the lock is (.992)^50 = .67
		 */
		pline("%sour %s broke!",
		      (uwep->quan > 1) ? "One of y" : "Y", xname(uwep));
		useup(uwep);
		You("give up your attempt to force the lock.");
		return((xlock.usedtime = 0));
	    }
	} else			/* blunt */
	    wake_nearby();	/* due to hammering on the container */

	if(rn2(100) > xlock.chance) return(1);		/* still busy */

	You("succeed in forcing the lock.");
	xlock.box->olocked = !xlock.box->olocked;
	if(!xlock.picktyp && !rn2(3)) {

	    pline("In fact, you've totally destroyed the %s.",
		  xname(xlock.box));
	    for(otmp = fcobj; otmp; otmp = otmp2) {

		otmp2 = otmp->nobj;
		if(otmp->cobj == xlock.box) {

		    /* unlink it from the "contained" list */
		    if(otmp == fcobj) fcobj = otmp2;
		    else	      probj->nobj = otmp2;

		    if(!rn2(3) || otmp->olet == POTION_SYM)
			free((genericptr_t) otmp);
		    else { /* spill it onto the floor */
			otmp->nobj = xlock.box->nobj;
			xlock.box->nobj = otmp;
			otmp->cobj = (struct obj *)0;
			place_object(otmp, u.ux, u.uy);
			stackobj(otmp);
		    }
		} else probj = otmp;
	    }
	    delobj(xlock.box);
	}
	return((xlock.usedtime = 0));
}

#endif /* OVLB */
#ifdef OVL0

void
reset_pick() { xlock.usedtime = 0; }

#endif /* OVL0 */
#ifdef OVLB

int
pick_lock(pick) /* pick a lock with a given object */
	register struct	obj	*pick;
{
	register int x, y, picktyp, c, ch;
	register struct rm	*door;
	register struct obj	*otmp;

#ifdef __GNULINT__
	ch = 0;		/* GCC myopia */
#endif
	picktyp = pick->otyp;
	if(xlock.usedtime && picktyp == xlock.picktyp) {

	    You("resume your attempt to %s the lock.",
		  (xlock.door_or_box ? !(xlock.door->doormask & D_LOCKED) :
		   !xlock.box->olocked) ? "lock" :
		  ((xlock.picktyp == LOCK_PICK) ? "pick" : "open" ));

	    set_occupation(picklock,
			   (picktyp == LOCK_PICK) ? "picking the lock" :
						    "opening the lock",  0);
	    return(1);
	}

#ifdef POLYSELF
	if(nohands(uasmon)) {
		You("can't hold a %s - you have no hands!", xname(pick));
		return(0);
	}
#endif
	if((picktyp != LOCK_PICK && picktyp != CREDIT_CARD &&
	    picktyp != SKELETON_KEY && picktyp != KEY)) {
		impossible("picking lock with object %d?", picktyp);
		return(0);
	}
	if(!getdir(1)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if((x == u.ux) && (y == u.uy)) { /* pick the lock on a container */
	    c = 'n';			/* in case there are no boxes here */
	    for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if(Is_box(otmp) &&
			       /* credit cards are only good for unlocking */
			       (picktyp != CREDIT_CARD || otmp->olocked)) {
		    pline("There is %s here, %s the lock? ",
		    doname(otmp), (!otmp->olocked) ? "close" :
		    ((picktyp == LOCK_PICK) ? "pick" : "open" ));

		    c = ynq();
		    if(c == 'q') return(0);
		    if(c == 'n') continue;

		    if(picktyp == KEY && otmp->spe != pick->spe) {
			    pline("The %s won't fit the lock.",xname(pick));
			    return(1);
		    }
		    switch(picktyp) {
			case CREDIT_CARD:
			    ch = ACURR(A_DEX)+(20*(pl_character[0] == 'R'));
			    break;
			case LOCK_PICK:
			    ch = 4*ACURR(A_DEX)+(25*(pl_character[0] == 'R'));
			    break;
			case SKELETON_KEY:
			    ch = 75 + ACURR(A_DEX);
			    break;
			case KEY:
			    ch = 1000;
			    break;
			default:	ch = 0;
		    }
		    if(otmp->cursed) ch /= 2;

		    xlock.door_or_box = 0;
		    xlock.picktyp = picktyp;
		    xlock.box = otmp;
		    break;
		}
	    if(c != 'y')
		return(0);		/* decided against all boxes */
	} else {			/* pick the lock in a door */
	    struct monst *mtmp;

	    door = &levl[x][y];
	    if ((mtmp = m_at(x,y)) && canseemon(mtmp) && !mtmp->mimic) {
		if (picktyp == CREDIT_CARD &&
#ifdef ORACLE
		    (mtmp->isshk || mtmp->data == &mons[PM_ORACLE]))
#else
		    mtmp->isshk)
#endif
		    verbalize("No checks, no credit, no problem.");
		else
		    kludge("I don't think %s would appreciate that.", mon_nam(mtmp));
		return(0);
	    }
	    if(!IS_DOOR(door->typ)) {
#ifdef STRONGHOLD
		if (is_drawbridge_wall(x,y) >= 0)
		    You("%s no lock on the drawbridge.",
				Blind ? "feel" : "see");
		else
#endif
		You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
	    }
	    switch (door->doormask) {
		case D_NODOOR:
		    pline("This doorway has no door.");
		    return(0);
		case D_ISOPEN:
		    pline("Picking the lock of an open door is pointless.");
		    return(0);
		case D_BROKEN:
		    pline("This door is broken.");
		    return(0);
		default:
		    /* credit cards are only good for unlocking */
		    if(picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
			You("can't lock a door with a credit card.");
			return(0);
		    }

		    pline("%sock it? ", (door->doormask & D_LOCKED) ? "Unl" : "L" );

		    c = yn();
		    if(c == 'n') return(0);

		    switch(picktyp) {
			case CREDIT_CARD:
			    ch = 2*ACURR(A_DEX)+(20*(pl_character[0] == 'R'));
			    break;
			case LOCK_PICK:
			    ch = 3*ACURR(A_DEX)+(30*(pl_character[0] == 'R'));
			    break;
			case SKELETON_KEY:
			    ch = 70 + ACURR(A_DEX);
			    break;
			case KEY:
			    pline("The %s won't fit the door.", xname(pick));
			    return(1);
			default:    ch = 0;
		    }
		    xlock.door_or_box = 1;
		    xlock.door = door;
	    }
	}
	flags.move = 0;
	xlock.chance = ch;
	xlock.picktyp = picktyp;
	xlock.usedtime = 0;
	set_occupation(picklock,
		       (picktyp == LOCK_PICK) ? "picking the lock" :
						"opening the lock",  0);
	return(1);
}

int
doforce() {		/* try to force a chest with your weapon */

	register struct obj *otmp;
	register int c, picktyp;

	if(!uwep ||	/* proper type test */
	   (uwep->olet != WEAPON_SYM && uwep->olet != ROCK_SYM &&
						uwep->otyp != PICK_AXE) ||
	   (uwep->otyp < DAGGER) ||
	   (uwep->otyp > VOULGE && uwep->olet != ROCK_SYM &&
						uwep->otyp != PICK_AXE)
	  ) {
	    You("can't force anything without a %sweapon.",
		  (uwep) ? "proper " : "");
	    return(0);
	}

	picktyp = (uwep->otyp >= DAGGER && uwep->otyp <= KATANA);
	if(xlock.usedtime && xlock.box && picktyp == xlock.picktyp) {
	    You("resume your attempt to force the lock.");
	    set_occupation(forcelock, "forcing the lock", 0);
	    return(1);
	}

	/* A lock is made only for the honest man, the thief will break it. */
	xlock.box = (struct obj *)0;
	for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
	    if(Is_box(otmp)) {
		if(otmp->olocked)
		    pline("There is %s here, force the lock? ", doname(otmp));
		else {
		    pline("There is a %s here, but it's already unlocked.",
			  xname(otmp));
		    continue;
		}

		c = ynq();
		if(c == 'q') return(0);
		if(c == 'n') continue;

		if(picktyp)
		    You("force your %s into a crack and pry.", xname(uwep));
		else
		    You("start bashing it with your %s.", xname(uwep));
		xlock.box = otmp;
		xlock.chance = objects[otmp->otyp].wldam * 2;
		xlock.picktyp = picktyp;
		xlock.usedtime = 0;
		break;
	    }

	if(xlock.box)	set_occupation(forcelock, "forcing the lock", 0);
	else		You("decide not to force the issue.");
	return(1);
}

int
doopen() {		/* try to open a door */
	register int x, y;
	register struct rm *door;
	struct monst *mtmp;

	if(!getdir(1)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if((x == u.ux) && (y == u.uy)) return(0);

	if((mtmp = m_at(x,y)) && mtmp->mimic &&
				mtmp->m_ap_type == M_AP_FURNITURE &&
				mtmp->mappearance == S_cdoor &&
				!Protection_from_shape_changers) {
		stumble_onto_mimic(mtmp);
		return(1);
	}

	door = &levl[x][y];

	if(!IS_DOOR(door->typ)) {
#ifdef STRONGHOLD
		if (is_db_wall(x,y)) {
		    pline("There is no obvious way to open the drawbridge.");
		    return(0);
		}
#endif
		You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
	}

	if(!(door->doormask & D_CLOSED)) {
	  switch(door->doormask) {
	     case D_BROKEN: pline("This door is broken."); break;
	     case D_NODOOR: pline("This doorway has no door."); break;
	     case D_ISOPEN: pline("This door is already open."); break;
	     default:	    pline("This door is locked."); break;
	  }
	  return(0);
	}

#ifdef POLYSELF
	if(verysmall(uasmon)) {
	    pline("You're too small to pull the door open.");
	    return(0);
	}
#endif
	/* door is known to be CLOSED */
	if (rnl(20) < (ACURR(A_STR)+ACURR(A_DEX)+ACURR(A_CON))/3) {
	    pline("The door opens.");
	    if(door->doormask & D_TRAPPED) {
		b_trapped("door");
		door->doormask = D_NODOOR;
	    } else
		door->doormask = D_ISOPEN;
	    mnewsym(x,y);
	    prl(x,y);
	} else {
	    pline("The door resists!");
	}

	return(1);
}

static
boolean
obstructed(x,y)
register int x, y;
{
	if(MON_AT(x, y)) {
		if (m_at(x,y)->mimic) goto obj;	  
		pline("%s stands in the way!", Blind ?
			"Some creature" : Monnam(m_at(x,y)));
		return(TRUE);
	}
	if (OBJ_AT(x, y) || levl[x][y].gmask) {
obj:
		pline("Something's in the way.");
		return(TRUE);
	}
	return(FALSE);
}

int
doclose() {		/* try to close a door */
	register int x, y;
	register struct rm *door;
	struct monst *mtmp;

	if(!getdir(1)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if((x == u.ux) && (y == u.uy)) {
		You("are in the way!");
		return(1);
	}

	if((mtmp = m_at(x,y)) && mtmp->mimic &&
				mtmp->m_ap_type == M_AP_FURNITURE && 
				mtmp->mappearance == S_cdoor &&
				!Protection_from_shape_changers) {
		stumble_onto_mimic(mtmp);
		return(1);
	}

	door = &levl[x][y];

	if(!IS_DOOR(door->typ)) {
#ifdef STRONGHOLD
		if (door->typ == DRAWBRIDGE_DOWN)
		    pline("There is no obvious way to close the drawbridge.");
		else
#endif
		You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
	}

	if(door->doormask == D_NODOOR) {
	    pline("This doorway has no door.");
	    return(0);
	}

	if(obstructed(x, y)) return(0);

	if(door->doormask == D_BROKEN) {
	    pline("This door is broken.");
	    return(0);
	}

	if(door->doormask & (D_CLOSED | D_LOCKED)) {
	    pline("This door is already closed.");
	    return(0);
	}

	if(door->doormask == D_ISOPEN) {
#ifdef POLYSELF
	    if(verysmall(uasmon)) {
		 pline("You're too small to push the door closed.");
		 return(0);
 	    }
#endif
	    if (rn2(25) < (ACURR(A_STR)+ACURR(A_DEX)+ACURR(A_CON))/3) {
		pline("The door closes.");
		door->doormask = D_CLOSED;
		mnewsym(x,y);
		prl(x,y);
	    }
	    else pline("The door resists!");
	}

	return(1);
}

int
boxlock(obj, otmp)	/* box obj was hit with spell effect otmp */
			/* returns 1 if something happened */
	register struct obj *obj, *otmp;	/* obj *is* a box */
{
	register boolean res = 0;

	switch(otmp->otyp) {
	    case WAN_LOCKING:
#ifdef SPELLS
	    case SPE_WIZARD_LOCK:
#endif
			if(!obj->olocked) {
				pline("Klunk!");
				obj->olocked = !(obj->olocked);
				res = 1;
			} else	res = 0;
			break;
	    case WAN_OPENING:
#ifdef SPELLS
	    case SPE_KNOCK:
#endif
			if(obj->olocked) {
				pline("Klick!");
				obj->olocked = !(obj->olocked);
				res = 1;
			} else	res = 0;
			break;
	}
	return(res);
}

int
doorlock(otmp,x,y)	/* door was hit with spell effect otmp */
	register struct obj *otmp;
	int x, y;
{
	register struct rm *door = &levl[x][y];
	boolean res = 1;

	if(door->typ == SDOOR) {
	    if(otmp->otyp == WAN_OPENING
#ifdef SPELLS
	       || otmp->otyp == SPE_KNOCK
#endif /* SPELLS /**/
	      ) {
		door->typ = DOOR;
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
		if(cansee(x,y)) pline("A section of the wall opens up!");
		mnewsym(x,y);
		return(1);
	    } else
		return(0);
	}

#ifdef STRONGHOLD
	/* make sure it isn't an open drawbridge */
	if (is_maze_lev && find_drawbridge(&x,&y)) {
	    if(otmp->otyp == WAN_OPENING
#ifdef SPELLS
	       || otmp->otyp == SPE_KNOCK
#endif /* SPELLS /**/
	      )
		    (void) open_drawbridge(x,y);
	    else
		    (void) close_drawbridge(x,y);
	    return 1;
	}
#endif

	switch(otmp->otyp) {
	    case WAN_LOCKING:
#ifdef SPELLS
	    case SPE_WIZARD_LOCK:
#endif
		if(obstructed(x,y)) return 0;
		if (cansee(x,y))
		switch (door->doormask & ~D_TRAPPED) {
			case D_CLOSED:
				pline("The door locks!");
				break;
			case D_ISOPEN:
				pline("The door swings shut, and locks!");
				break;
			case D_BROKEN:
				pline("The broken door reassembles and locks!");
				break;
			case D_NODOOR:
	pline("A cloud of dust springs up and assembles itself into a door!");
				break;
			default: res = 0;
		}
		door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
		mnewsym(x,y);
		if(cansee(x,y)) prl(x,y);
		break;
	    case WAN_OPENING:
#ifdef SPELLS
	    case SPE_KNOCK:
#endif
		if(door->doormask & D_LOCKED) {
		    door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
		    if(cansee(x,y)) pline("The door unlocks!");
		} else res = 0;
		break;
	    case WAN_STRIKING:
#ifdef SPELLS
	    case SPE_FORCE_BOLT:
#endif
		if(door->doormask & (D_LOCKED | D_CLOSED)) {
		    if(door->doormask & D_TRAPPED) {
			if (MON_AT(x, y))
			    (void) mb_trapped(m_at(x,y));
			else if (flags.verbose)
			    if (cansee(x,y))
			       pline("KABOOM!!	You see a door explode.");
			    else if (flags.soundok)
			       You("hear a distant explosion.");
			door->doormask = D_NODOOR;
			mnewsym(x,y);
			if (cansee(x,y)) prl(x,y);
			break;
		    }
		    door->doormask = D_BROKEN;
		    if (flags.verbose)
			if (cansee(x,y))
			    pline("The door crashes open!");
			else if (flags.soundok)
			    You("hear a crashing sound.");
		    mnewsym(x,y);
		    if (cansee(x,y)) prl(x,y);
		} else res = 0;
		break;
	    default:	impossible("magic (%d) attempted on door.", otmp->otyp);
	}
	return res;
}

#ifdef STUPID_CPP	/* otherwise these functions are macros in obj.h */
int
Is_container(otmp) struct obj * otmp; {
	return(otmp->otyp >= ICE_BOX && otmp->otyp <= BAG_OF_TRICKS);
}

int
Is_box(otmp) struct obj * otmp; {
	return(otmp->otyp == LARGE_BOX || otmp->otyp == CHEST);
}

int
Is_mbag(otmp) struct obj * otmp; {
	return(otmp->otyp == BAG_OF_HOLDING || otmp->otyp == BAG_OF_TRICKS);
}

int
is_sword(otmp) struct obj * otmp; {
	return(otmp->otyp >= SHORT_SWORD && otmp->otyp <= KATANA);
}

int
bimanual(otmp) struct obj * otmp; {
	return((otmp->olet == WEAPON_SYM || otmp->otyp == UNICORN_HORN)
		&& objects[otmp->otyp].oc_bimanual);
}
#endif /* STUPID_CPP */

#endif /* OVLB */
