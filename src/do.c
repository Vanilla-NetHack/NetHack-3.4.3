/*	SCCS Id: @(#)do.c	3.2	96/03/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) */

#include "hack.h"
#include "lev.h"

#include <errno.h>
#ifdef _MSC_VER	/* MSC 6.0 defines errno quite differently */
# if (_MSC_VER >= 600)
#  define SKIP_ERRNO
# endif
#endif
#ifndef SKIP_ERRNO
#ifdef _DCC
const
#endif
extern int errno;
#endif

#ifdef USE_TILES
# ifdef MSDOS
extern boolean tiles_on;				/* video.c */
# endif
#endif


#ifdef SINKS
# ifdef OVLB
static void FDECL(trycall, (struct obj *));
# endif /* OVLB */
STATIC_DCL void FDECL(dosinkring, (struct obj *));
#endif /* SINKS */

STATIC_PTR int FDECL(drop, (struct obj *));
STATIC_PTR int NDECL(wipeoff);

#ifdef OVL0
static int FDECL(menu_drop, (int));
#endif
#ifdef OVL2
static int NDECL(currentlevel_rewrite);
/* static boolean FDECL(badspot, (XCHAR_P,XCHAR_P)); */
#endif

#ifdef OVLB

static NEARDATA const char drop_types[] =
	{ ALLOW_COUNT, GOLD_CLASS, ALL_CLASSES, 0 };

/* 'd' command: drop one inventory item */
int
dodrop()
{
	int result, i = (invent || u.ugold) ? 0 : (SIZE(drop_types) - 1);

	if (*u.ushops) sellobj_state(TRUE);
	result = drop(getobj(&drop_types[i], "drop"));
	if (*u.ushops) sellobj_state(FALSE);
	reset_occupations();

	return result;
}

#endif /* OVLB */
#ifdef OVL0

/* Called when a boulder is dropped, thrown, or pushed.  If it ends up
 * in a pool, it either fills the pool up or sinks away.  In either case,
 * it's gone for good...  If the destination is not a pool, returns FALSE.
 */
boolean
boulder_hits_pool(otmp, rx, ry, pushing)
struct obj *otmp;
register int rx, ry;
boolean pushing;
{
	if (!otmp || otmp->otyp != BOULDER)
	    impossible("Not a boulder?");
	else if (!Is_waterlevel(&u.uz) && (is_pool(rx,ry) || is_lava(rx,ry))) {
	    boolean lava = is_lava(rx,ry), fills_up;
	    const char *what = lava ? "lava" : "water";
	    schar ltyp = levl[rx][ry].typ;
	    int chance = rn2(10);		/* water: 90%; lava: 10% */
	    fills_up = lava ? chance == 0 : chance != 0;

	    if (fills_up) {
		if (ltyp == DRAWBRIDGE_UP) {
		    levl[rx][ry].drawbridgemask &= ~DB_UNDER; /* clear lava */
		    levl[rx][ry].drawbridgemask |= DB_FLOOR;
		} else
		    levl[rx][ry].typ = ROOM;

		bury_objs(rx, ry);
		newsym(rx,ry);
		if (pushing) {
		    You("push %s into the %s.", the(xname(otmp)), what);
		    if (flags.verbose && !Blind)
			pline("Now you can cross it!");
		    /* no splashing in this case */
		}
	    }
	    if (!fills_up || !pushing) {	/* splashing occurs */
		if (!u.uinwater) {
		    if (pushing ? !Blind : cansee(rx,ry)) {
			boolean moat = (ltyp != WATER) &&
			    !Is_medusa_level(&u.uz) && !Is_waterlevel(&u.uz);

			pline("There is a large splash as %s %s the %s.",
			      the(xname(otmp)), fills_up? "fills":"falls into",
			      lava ? "lava" : ltyp==POOL ? "pool" :
			      moat ? "moat" : "water");
		    } else if (flags.soundok)
			You_hear("a%s splash.", lava ? " sizzling" : "");
		    wake_nearto(rx, ry, 40);
		}

		if (fills_up && u.uinwater && distu(rx,ry) == 0) {
		    u.uinwater = 0;
		    docrt();
		    vision_full_recalc = 1;
		    You("find yourself on dry land again!");
		} else if (lava && distu(rx,ry) <= 2) {
		    You("are hit by molten lava%c",
			Fire_resistance ? '.' : '!');
		    losehp(d((Fire_resistance ? 1 : 3), 6),
			   "molten lava", KILLED_BY);
		} else if (!fills_up && flags.verbose &&
			   (pushing ? !Blind : cansee(rx,ry)))
		    pline("It sinks without a trace!");
	    }

	    /* boulder is now gone */
	    if (pushing) delobj(otmp);
	    else obfree(otmp, (struct obj *)0);
	    return TRUE;
	}
	return FALSE;
}

/* Used for objects which sometimes do special things when dropped; must be
 * called with the object not in any chain.  Returns TRUE if the object goes
 * away.
 */
boolean
flooreffects(obj,x,y,verb)
struct obj *obj;
int x,y;
const char *verb;
{
	struct trap *t;
	struct monst *mtmp;

	if (obj->where != OBJ_FREE)
	    panic("flooreffects: obj not free");

	/* make sure things like water_damage() have no pointers to follow */
	obj->nobj = obj->nexthere = (struct obj *)0;

	if (obj->otyp == BOULDER && boulder_hits_pool(obj, x, y, FALSE))
		return TRUE;
	else if (obj->otyp == BOULDER && (t = t_at(x,y)) != 0 &&
		 (t->ttyp==PIT || t->ttyp==SPIKED_PIT
			|| t->ttyp==TRAPDOOR || t->ttyp==HOLE)) {
		if (((mtmp = m_at(x, y)) && mtmp->mtrapped) ||
			(u.utrap && u.ux == x && u.uy == y)) {
		    if (*verb)
			pline_The("boulder %ss into the pit%s.", verb,
				(mtmp) ? "" : " with you");
		    if (mtmp) {
			if (!passes_walls(mtmp->data) &&
				!throws_rocks(mtmp->data)) {
			    if (hmon(mtmp, obj, TRUE))
				return FALSE;	/* still alive */
			} else mtmp->mtrapped = 0;
		    } else {
			if (!passes_walls(uasmon) && !throws_rocks(uasmon)) {
			    losehp(rnd(15), "squished under a boulder",
				   NO_KILLER_PREFIX);
			    return FALSE;	/* player remains trapped */
			} else u.utrap = 0;
		    }
		}
		if (*verb) {
			if (Blind) {
				if ((x == u.ux) && (y == u.uy))
					You_hear("a CRASH! beneath you.");
				else
					You_hear("the boulder %s.", verb);
			} else if (cansee(x, y)) {
				pline_The("boulder %s%s.",
				    t->tseen ? "" : "triggers and ",
				    t->ttyp == TRAPDOOR ? "plugs a trap door" :
				    t->ttyp == HOLE ? "plugs a hole" :
				    "fills a pit");
			}
		}
		deltrap(t);
		obfree(obj, (struct obj *)0);
		bury_objs(x, y);
		newsym(x,y);
		return TRUE;
	} else if (is_pool(x, y)) {
		water_damage(obj, FALSE, FALSE);
	}
	return FALSE;
}

#endif /* OVL0 */
#ifdef OVLB

void
doaltarobj(obj)  /* obj is an object dropped on an altar */
	register struct obj *obj;
{
	if (Blind) return;
	if (obj->blessed || obj->cursed) {
		pline("There is %s flash as %s hit%s the altar.",
			an(hcolor(obj->blessed ? amber : Black)),
			doname(obj),
			(obj->quan == 1L) ? "s" : "");
		if (!Hallucination) obj->bknown = 1;
	} else {
		pline("%s land%s on the altar.", Doname2(obj),
			(obj->quan == 1L) ? "s" : "");
		obj->bknown = 1;
	}
}

#ifdef SINKS
static
void
trycall(obj)
register struct obj *obj;
{
	if(!objects[obj->otyp].oc_name_known &&
	   !objects[obj->otyp].oc_uname)
	   docall(obj);
}

STATIC_OVL
void
dosinkring(obj)  /* obj is a ring being dropped over a kitchen sink */
register struct obj *obj;
{
	register struct obj *otmp,*otmp2;
	register boolean ideed = TRUE;

	You("drop %s down the drain.", doname(obj));
#ifndef NO_SIGNAL
	obj->in_use = TRUE;	/* block free identification via interrupt */
#endif
	switch(obj->otyp) {	/* effects that can be noticed without eyes */
	    case RIN_SEARCHING:
		You("thought your %s got lost in the sink, but there it is!",
			xname(obj));
#ifndef NO_SIGNAL
		obj->in_use = FALSE;
#endif
		dropx(obj);
		trycall(obj);
		return;
	    case RIN_LEVITATION:
		pline_The("sink quivers upward for a moment.");
		break;
	    case RIN_POISON_RESISTANCE:
		You("smell rotten %s.", makeplural(pl_fruit));
		break;
	    case RIN_AGGRAVATE_MONSTER:
		pline("Several flies buzz angrily around the sink.");
		break;
	    case RIN_SHOCK_RESISTANCE:
		pline("Static electricity surrounds the sink.");
		break;
	    case RIN_CONFLICT:
		You_hear("loud noises coming from the drain.");
		break;
	    case RIN_GAIN_STRENGTH:
		pline_The("water flow seems %ser now.",
			(obj->spe<0) ? "weak" : "strong");
		break;
	    case RIN_INCREASE_DAMAGE:
		pline_The("water's force seems %ser now.",
			(obj->spe<0) ? "small" : "great");
		break;
	    default:
		ideed = FALSE;
		break;
	}
	if(!Blind && !ideed) {
	    ideed = TRUE;
	    switch(obj->otyp) {		/* effects that need eyes */
		case RIN_ADORNMENT:
		    pline_The("faucets flash brightly for a moment.");
		    break;
		case RIN_REGENERATION:
		    pline_The("sink looks as good as new.");
		    break;
		case RIN_INVISIBILITY:
		    You("don't see anything happen to the sink.");
		    break;
		case RIN_SEE_INVISIBLE:
		    You("see some air in the sink.");
		    break;
		case RIN_STEALTH:
		pline_The("sink seems to blend into the floor for a moment.");
		    break;
		case RIN_HUNGER:
		    ideed = FALSE;
		    for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
			otmp2 = otmp->nexthere;
			if(otmp != uball && otmp != uchain) {
			    pline("Suddenly, %s vanishes from the sink!",
							doname(otmp));
			    delobj(otmp);
			    ideed = TRUE;
			}
		    }
		    break;
		case RIN_FIRE_RESISTANCE:
		pline_The("hot water faucet flashes brightly for a moment.");
		    break;
		case RIN_COLD_RESISTANCE:
		pline_The("cold water faucet flashes brightly for a moment.");
		    break;
		case RIN_PROTECTION_FROM_SHAPE_CHAN:
		    pline_The("sink looks nothing like a fountain.");
		    break;
		case RIN_PROTECTION:
		    pline_The("sink glows %s for a moment.",
			    hcolor((obj->spe<0) ? Black : silver));
		    break;
		case RIN_WARNING:
		    pline_The("sink glows %s for a moment.", hcolor(White));
		    break;
		case RIN_TELEPORTATION:
		    pline_The("sink momentarily vanishes.");
		    break;
		case RIN_TELEPORT_CONTROL:
	    pline_The("sink looks like it is being beamed aboard somewhere.");
		    break;
		case RIN_POLYMORPH:
		    pline_The("sink momentarily looks like a fountain.");
		    break;
		case RIN_POLYMORPH_CONTROL:
	pline_The("sink momentarily looks like a regularly erupting geyser.");
		    break;
	    }
	}
	if(ideed)
	    trycall(obj);
	else
	    You_hear("the ring bouncing down the drainpipe.");
	if (!rn2(20)) {
		pline_The("sink backs up, leaving %s.", doname(obj));
#ifndef NO_SIGNAL
		obj->in_use = FALSE;
#endif
		dropx(obj);
	}
	else
		useup(obj);
}
#endif

#endif /* OVLB */
#ifdef OVL0

/* some common tests when trying to drop or throw items */
boolean
canletgo(obj,word)
register struct obj *obj;
register const char *word;
{
	if(obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)){
		if (*word)
			Norep("You cannot %s %s you are wearing.",word,
				something);
		return(FALSE);
	}
	if (obj->otyp == LOADSTONE && obj->cursed) {
		if (*word)
			pline("For some reason, you cannot %s the stone%s!",
				word, plur(obj->quan));
		/* Kludge -- see invent.c */
		if (obj->corpsenm) {
			struct obj *otmp;

			otmp = obj;
			obj = obj->nobj;
			obj->quan += otmp->quan;
			obj->owt = weight(obj);
			freeinv(otmp);
			obfree(otmp, obj);
		}
		obj->bknown = 1;
		return(FALSE);
	}
	if (obj->otyp == LEASH && obj->leashmon != 0) {
		if (*word)
			pline ("The leash is tied around your %s.",
					body_part(HAND));
		return(FALSE);
	}
	return(TRUE);
}

STATIC_PTR
int
drop(obj)
register struct obj *obj;
{
	if(!obj) return(0);
	if(!canletgo(obj,"drop"))
		return(0);
	if(obj == uwep) {
		if(welded(uwep)) {
			weldmsg(obj, FALSE);
			return(0);
		}
		setuwep((struct obj *)0);
		if(uwep) return 0; /* lifesaved and rewielded */
	}

	if (u.uswallow) {
		/* barrier between you and the floor */
		if(flags.verbose)
			You("drop %s into %s %s.", doname(obj),
				s_suffix(mon_nam(u.ustuck)),
				is_animal(u.ustuck->data) ?
				"stomach" : "interior");
	} else {
#ifdef SINKS
	    if((obj->oclass == RING_CLASS) && IS_SINK(levl[u.ux][u.uy].typ)) {
		dosinkring(obj);
		return(1);
	    }
#endif
	    if (!can_reach_floor()) {
		if(flags.verbose) You("drop %s.", doname(obj));
		if (obj->oclass != GOLD_CLASS || obj == invent) freeinv(obj);
		hitfloor(obj);
		return(1);
	    }
	    if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
		doaltarobj(obj);	/* set bknown */
	    } else
		if(flags.verbose) You("drop %s.", doname(obj));
	}
	dropx(obj);
	return(1);
}

/* Called in several places - should not produce texts */
void
dropx(obj)
register struct obj *obj;
{
	/* Money is usually not in our inventory */
	if (obj->oclass != GOLD_CLASS || obj == invent) freeinv(obj);
	if (!u.uswallow && ship_object(obj, u.ux, u.uy, FALSE)) return;
	dropy(obj);
}

void
dropy(obj)
register struct obj *obj;
{
	if (!u.uswallow && flooreffects(obj,u.ux,u.uy,"drop")) return;
	if(obj->otyp == CRYSKNIFE)
		obj->otyp = WORM_TOOTH;
	/* uswallow check done by GAN 01/29/87 */
	if(u.uswallow) {
		if (obj != uball) {		/* mon doesn't pick up ball */
		    mpickobj(u.ustuck,obj);
		}
	} else  {
		place_object(obj, u.ux, u.uy);
		if (obj == uball)
		    drop_ball(u.ux,u.uy);
		else
		    sellobj(obj, u.ux, u.uy);
		stackobj(obj);
		if(Blind && Levitation)
		    map_object(obj, 0);
		newsym(u.ux,u.uy);	/* remap location under self */
	}
}

/* 'D' command: drop several things */
int
doddrop()
{
	int result = 0;

	add_valid_menu_class(0); /* clear any classes already there */
	if (*u.ushops) sellobj_state(TRUE);
	if (flags.menu_style != MENU_TRADITIONAL ||
		(result = ggetobj("drop", drop, 0, FALSE)) < -1)
	    result = menu_drop(result);
	if (*u.ushops) sellobj_state(FALSE);
	reset_occupations();

	return result;
}

/* Drop things from the hero's inventory, using a menu. */
static int
menu_drop(retry)
int retry;
{
    int n, i, n_dropped = 0;
    long cnt;
    struct obj *otmp, *otmp2, *u_gold = 0;
    menu_item *pick_list;
    boolean all_categories = TRUE;
    boolean drop_everything = FALSE;

    if (u.ugold) {
	/* Hack: gold is not in the inventory, so make a gold object
	   and put it at the head of the inventory list. */
	u_gold = mkgoldobj(u.ugold);	/* removes from u.ugold */
	u.ugold = u_gold->quan;		/* put the gold back */
	assigninvlet(u_gold);		/* might end up as NOINVSYM */
	u_gold->nobj = invent;
	invent = u_gold;
    }
    
    if (retry) {
	all_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
	all_categories = FALSE;
	n = query_category("Drop what type of items?",
			invent,
			UNPAID_TYPES | ALL_TYPES | CHOOSE_ALL,
			&pick_list, PICK_ANY);
	if (!n) goto drop_done;
	for (i = 0; i < n; i++) {
	    if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
		all_categories = TRUE;
	    else if (pick_list[i].item.a_int == 'A')
		drop_everything = TRUE;
	    else
		add_valid_menu_class(pick_list[i].item.a_int);
	}
	free((genericptr_t) pick_list);
    } else if (flags.menu_style == MENU_COMBINATION) {
	all_categories = FALSE;
	/* Gather valid classes via traditional NetHack method */
	i = ggetobj("drop", drop, 0, TRUE);
	if (i == -2) all_categories = TRUE;
    }

    if (drop_everything) {
	for(otmp = invent; otmp; otmp = otmp2) {
	    otmp2 = otmp->nobj;
	    n_dropped += drop(otmp);
	}
    } else {
	/* should coordinate with perm invent, maybe not show worn items */
	n = query_objlist("What would you like to drop?", invent,
			USE_INVLET|INVORDER_SORT, &pick_list,
			PICK_ANY, all_categories ? allow_all : allow_category);
	if (n > 0) {
	    for (i = 0; i < n; i++) {
		otmp = pick_list[i].item.a_obj;
		cnt = pick_list[i].count;
		if (cnt < otmp->quan && !welded(otmp) &&
			(!otmp->cursed || otmp->otyp != LOADSTONE)) {
		    otmp2 = splitobj(otmp, cnt);
		    /* assume other worn items aren't mergable */
		    if (otmp == uwep) setuwep(otmp2);
		}
		n_dropped += drop(otmp);
	    }
	    free((genericptr_t) pick_list);
	}
    }

 drop_done:
    if (u_gold && invent && invent->oclass == GOLD_CLASS) {
	/* didn't drop [all of] it */
	u_gold = invent;
	invent = u_gold->nobj;
	dealloc_obj(u_gold);
    }
    return n_dropped;
}

#endif /* OVL0 */
#ifdef OVL2

/* on a ladder, used in goto_level */
static NEARDATA boolean at_ladder = FALSE;

int
dodown()
{
	struct trap *trap = 0;
	boolean stairs_down = ((u.ux == xdnstair && u.uy == ydnstair) ||
		    (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)),
		ladder_down = (u.ux == xdnladder && u.uy == ydnladder);

	if (Levitation) {
	    if ((HLevitation & (I_SPECIAL|W_ARTI)) != 0) {
		/* end controlled levitation */
		if (float_down(I_SPECIAL|W_ARTI|TIMEOUT))
		    return 1;	/* came down, so moved */
	    }
	    floating_above(stairs_down ? "stairs" : ladder_down ?
			   "ladder" : surface(u.ux, u.uy));
	    return 0;	/* didn't move */
	}
	if (!stairs_down && !ladder_down) {
		if (!(trap = t_at(u.ux,u.uy)) ||
			(trap->ttyp != TRAPDOOR && trap->ttyp != HOLE)
			|| !Can_fall_thru(&u.uz) || !trap->tseen) {
			You_cant("go down here.");
			return(0);
		}
	}
	if(u.ustuck) {
		You("are being held, and cannot go down.");
		return(1);
	}
	if (on_level(&valley_level, &u.uz) && !u.uevent.gehennom_entered) {
		You("are standing at the gate to Gehennom.");
		pline("Unspeakable cruelty and harm lurk down there.");
		if (yn("Are you sure you want to enter?") != 'y')
			return(0);
		else pline("So be it.");
		u.uevent.gehennom_entered = 1;	/* don't ask again */
	}

	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	}

	if (trap)
	    You("%s %s.", locomotion(uasmon, "jump"),
		trap->ttyp == HOLE ? "down the hole" : "through the trap door");

	if (trap && Is_stronghold(&u.uz)) {
		goto_hell(TRUE, TRUE);
	} else {
		at_ladder = (boolean) (levl[u.ux][u.uy].typ == LADDER);
		next_level(!trap);
		at_ladder = FALSE;
	}
	return(1);
}

int
doup()
{
	if( (u.ux != xupstair || u.uy != yupstair)
	     && (!xupladder || u.ux != xupladder || u.uy != yupladder)
	     && (!sstairs.sx || u.ux != sstairs.sx || u.uy != sstairs.sy
			|| !sstairs.up)
	  ) {
		You_cant("go up here.");
		return(0);
	}
	if(u.ustuck) {
		You("are being held, and cannot go up.");
		return(1);
	}
	if(near_capacity() > SLT_ENCUMBER) {
		/* No levitation check; inv_weight() already allows for it */
		Your("load is too heavy to climb the %s.",
			levl[u.ux][u.uy].typ == STAIRS ? "stairs" : "ladder");
		return(1);
	}
	if(ledger_no(&u.uz) == 1) {
		if (yn("Beware, there will be no return! Still climb?") != 'y')
			return(0);
	}
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	}
	if (levl[u.ux][u.uy].typ == LADDER) at_ladder = TRUE;
	prev_level(TRUE);
	at_ladder = FALSE;
	return(1);
}

d_level save_dlevel = {0, 0};

/* check that we can write out the current level */
static int
currentlevel_rewrite()
{
	register int fd;

	/* since level change might be a bit slow, flush any buffered screen
	 *  output (like "you fall through a trapdoor") */
	mark_synch();

	fd = create_levelfile(ledger_no(&u.uz));

	if(fd < 0) {
		/*
		 * This is not quite impossible: e.g., we may have
		 * exceeded our quota. If that is the case then we
		 * cannot leave this level, and cannot save either.
		 * Another possibility is that the directory was not
		 * writable.
		 */
		pline("Cannot create level file for level %d.",
						ledger_no(&u.uz));
		return -1;
	}

#ifdef MFLOPPY
	if (!savelev(fd, ledger_no(&u.uz), COUNT_SAVE)) {
		(void) close(fd);
		delete_levelfile(ledger_no(&u.uz));
		pline("NetHack is out of disk space for making levels!");
		You("can save, quit, or continue playing.");
		return -1;
	}
#endif
	return fd;
}

#ifdef INSURANCE
void
save_currentstate()
{
	int fd;

	if (flags.ins_chkpt) {
		/* write out just-attained level, with pets and everything */
		fd = currentlevel_rewrite();
		if(fd < 0) return;
		bufon(fd);
		savelev(fd,ledger_no(&u.uz), WRITE_SAVE);
		bclose(fd);
	}

	/* write out non-level state */
	savestateinlock();
}
#endif

/*
static boolean
badspot(x, y)
register xchar x, y;
{
	return((levl[x][y].typ != ROOM && levl[x][y].typ != AIR &&
			 levl[x][y].typ != CORR) || MON_AT(x, y));
}
*/

void
goto_level(newlevel, at_stairs, falling, portal)
d_level *newlevel;
boolean at_stairs, falling, portal;
{
	int fd, l_idx;
	xchar new_ledger;
	boolean cant_go_back,
		up = (depth(newlevel) < depth(&u.uz)),
		newdungeon = (u.uz.dnum != newlevel->dnum),
		was_in_W_tower = In_W_tower(u.ux, u.uy, &u.uz),
		familiar = FALSE;
	boolean new = FALSE;	/* made a new level? */

	if (dunlev(newlevel) > dunlevs_in_dungeon(newlevel))
		newlevel->dlevel = dunlevs_in_dungeon(newlevel);
	if (newdungeon && In_endgame(newlevel)) { /* 1st Endgame Level !!! */
		if (u.uhave.amulet)
		    assign_level(newlevel, &earth_level);
		else return;
	}
	new_ledger = ledger_no(newlevel);
	if (new_ledger <= 0)
		done(ESCAPED);	/* in fact < 0 is impossible */

	/* If you have the amulet and are trying to get out of Gehennom, going
	 * up a set of stairs sometimes does some very strange things!
	 * Biased against law and towards chaos, but not nearly as strongly
	 * as it used to be (prior to 3.2.0).
	 * Odds:	    old				    new
	 *	"up"    L      N      C		"up"    L      N      C
	 *	 +1   75.0   75.0   75.0	 +1   75.0   75.0   75.0
	 *	  0    0.0   12.5   25.0	  0    6.25   8.33  12.5
	 *	 -1    8.33   4.17   0.0	 -1    6.25   8.33  12.5
	 *	 -2    8.33   4.17   0.0	 -2    6.25   8.33   0.0
	 *	 -3    8.33   4.17   0.0	 -3    6.25   0.0    0.0
	 */
	if (Inhell && up && u.uhave.amulet && !newdungeon && !portal &&
				(dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz)-3)) {
		if (!rn2(4)) {
		    int odds = 3 + (int)u.ualign.type,		/* 2..4 */
			diff = odds <= 1 ? 0 : rn2(odds);	/* paranoia */

		    if (diff != 0) {
			assign_rnd_level(newlevel, &u.uz, diff);
			/* if inside the tower, stay inside */
			if (was_in_W_tower &&
			    !On_W_tower_level(newlevel)) diff = 0;
		    }
		    if (diff == 0)
			assign_level(newlevel, &u.uz);

		    new_ledger = ledger_no(newlevel);

		    pline("A mysterious force momentarily surrounds you...");
		    if (on_level(newlevel, &u.uz)) {
			(void) safe_teleds();
			(void) next_to_u();
			return;
		    } else
			at_stairs = at_ladder = FALSE;
		}
	}

	/* Prevent the player from going past the first quest level unless
	 * (s)he has been given the go-ahead by the leader.
	 */
	if (on_level(&u.uz, &qstart_level) && !newdungeon && !ok_to_quest()) {
		pline("A mysterious force prevents you from descending.");
		return;
	}

	if (on_level(newlevel, &u.uz)) return;		/* this can happen */

	fd = currentlevel_rewrite();
	if (fd < 0) return;

	if (falling) /* assuming this is only trapdoor or hole */
	    impact_drop((struct obj *)0, u.ux, u.uy, newlevel->dlevel);

	check_special_room(TRUE);		/* probably was a trap door */
	if (Punished) unplacebc();
	u.utrap = 0;				/* needed in level_tele */
	fill_pit(u.ux, u.uy);
	u.ustuck = 0;				/* idem */
	u.uinwater = 0;
	keepdogs(FALSE);
	if (u.uswallow)				/* idem */
		u.uswldtim = u.uswallow = 0;
	/*
	 *  We no longer see anything on the level.  Make sure that this
	 *  follows u.uswallow set to null since uswallow overrides all
	 *  normal vision.
	 */
	vision_recalc(2);

	/*
	 * Save the level we're leaving.  If we're entering the endgame,
	 * we can get rid of all existing levels because they cannot be
	 * reached any more.  We still need to use savelev()'s cleanup
	 * for the level being left, to recover dynamic memory in use and
	 * to avoid dangling timers and light sources.
	 */
	cant_go_back = (newdungeon && In_endgame(newlevel));
	if (!cant_go_back) bufon(fd);		/* use buffered output */
	savelev(fd, ledger_no(&u.uz),
		cant_go_back ? FREE_SAVE : (WRITE_SAVE | FREE_SAVE));
	bclose(fd);
	if (cant_go_back) {
	    /* discard unreachable levels; keep #0 */
	    for (l_idx = maxledgerno(); l_idx > 0; --l_idx)
		delete_levelfile(l_idx);
	}

#ifdef REINCARNATION
	if (Is_rogue_level(newlevel) || Is_rogue_level(&u.uz))
		assign_rogue_graphics(Is_rogue_level(newlevel));
#endif
#ifdef USE_TILES
# if defined(MSDOS)
	if (flags.grmode) {
		if (Is_rogue_level(newlevel) && tiles_on)
			tiles_on = FALSE;
		else tiles_on = TRUE;
	}
# endif
	substitute_tiles(newlevel);
#endif
	assign_level(&u.uz0, &u.uz);
	assign_level(&u.uz, newlevel);
	assign_level(&u.utolev, newlevel);
	u.utotype = 0;
	if (dunlev_reached(&u.uz) < dunlev(&u.uz))
		dunlev_reached(&u.uz) = dunlev(&u.uz);
	reset_rndmonst(NON_PM);   /* u.uz change affects monster generation */

	/* set default level change destination areas */
	/* the special level code may override these */
	(void) memset((genericptr_t) &updest, 0, sizeof updest);
	(void) memset((genericptr_t) &dndest, 0, sizeof dndest);

	if (!(level_info[new_ledger].flags & LFILE_EXISTS)) {
		/* entering this level for first time; make it now */
		if (level_info[new_ledger].flags & (FORGOTTEN|VISITED)) {
		    impossible("goto_level: returning to discarded level?");
		    level_info[new_ledger].flags &= ~(FORGOTTEN|VISITED);
		}
		mklev();
		new = TRUE;	/* made the level */
	} else {
		/* returning to previously visited level; reload it */
		fd = open_levelfile(new_ledger);
		if (fd < 0) {
			pline("Cannot open file (#%d) for level %d (errno %d).",
					(int) new_ledger, depth(&u.uz), errno);
			pline("Probably someone removed it.");
			done(TRICKED);
		}
		minit();	/* ZEROCOMP */
		getlev(fd, hackpid, new_ledger, FALSE);
		(void) close(fd);
	}

	if (portal && !In_endgame(&u.uz)) {
	    /* find the portal on the new level */
	    register struct trap *ttrap;

	    for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap)
		if (ttrap->ttyp == MAGIC_PORTAL) break;

	    if (!ttrap) panic("goto_level: no corresponding portal!");
	    u_on_newpos(ttrap->tx, ttrap->ty);
	} else if (at_stairs && !In_endgame(&u.uz)) {
	    if (up) {
		if (at_ladder) {
		    u_on_newpos(xdnladder, ydnladder);
		} else {
		    if (newdungeon) {
			if (Is_stronghold(&u.uz)) {
			    register xchar x, y;

			    do {
				x = (COLNO - 2 - rnd(5));
				y = rn1(ROWNO - 4, 3);
			    } while(occupied(x, y) ||
				    IS_WALL(levl[x][y].typ));
			    u_on_newpos(x, y);
			} else u_on_sstairs();
		    } else u_on_dnstairs();
		}
		/* Remove bug which crashes with levitation/punishment  KAA */
		if (Punished && !Levitation) {
			pline("With great effort you climb the %s.",
				at_ladder ? "ladder" : "stairs");
		} else if (at_ladder)
		    You("climb up the ladder.");
	    } else {	/* down */
		if (at_ladder) {
		    u_on_newpos(xupladder, yupladder);
		} else {
		    if (newdungeon) u_on_sstairs();
		    else u_on_upstairs();
		}
		if (u.dz &&
		    (near_capacity() > UNENCUMBERED || Punished || Fumbling)) {
		    You("fall down the %s.", at_ladder ? "ladder" : "stairs");
		    if (Punished) {
			drag_down();
			if (carried(uball)) {
			    if (uwep == uball)
				setuwep((struct obj *)0);
			    freeinv(uball);
			}
		    }
		    losehp(rnd(3), "falling downstairs", KILLED_BY);
		    selftouch("Falling, you");
		} else if (u.dz && at_ladder)
		    You("climb down the ladder.");
	    }
	} else {	/* trap door or level_tele or In_endgame */
	    if (was_in_W_tower && On_W_tower_level(&u.uz))
		/* Stay inside the Wizard's tower when feasible.	*/
		/* Note: up vs down doesn't really matter in this case. */
		place_lregion(dndest.nlx, dndest.nly,
				dndest.nhx, dndest.nhy,
				0,0, 0,0, LR_DOWNTELE, (d_level *) 0);
	    else if (up)
		place_lregion(updest.lx, updest.ly,
				updest.hx, updest.hy,
				updest.nlx, updest.nly,
				updest.nhx, updest.nhy,
				LR_UPTELE, (d_level *) 0);
	    else
		place_lregion(dndest.lx, dndest.ly,
				dndest.hx, dndest.hy,
				dndest.nlx, dndest.nly,
				dndest.nhx, dndest.nhy,
				LR_DOWNTELE, (d_level *) 0);
	    if (falling) {
		if (Punished) ballfall();
		selftouch("Falling, you");
	    }
	}

	if (Punished) placebc();
	obj_delivery();		/* before killing geno'd monsters' eggs */
	losedogs();
	kill_genocided_monsters();  /* for those wiped out while in limbo */
	/*
	 * Expire all timers that have gone off while away.  Must be
	 * after migrating monsters and objects are delivered
	 * (losedogs and obj_delivery).
	 */
	run_timers();

	initrack();

	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
	if(MON_AT(u.ux, u.uy)) {
		impossible("mnexto failed (do.c)?");
		rloc(m_at(u.ux, u.uy));
	}

	/* initial movement of bubbles just before vision_recalc */
	if (Is_waterlevel(&u.uz))
		movebubbles();

	if (level_info[new_ledger].flags & FORGOTTEN) {
	    forget_map(ALL_MAP);	/* forget the map */
	    forget_traps();		/* forget all traps too */
	    familiar = TRUE;
	    level_info[new_ledger].flags &= ~FORGOTTEN;
	}

	/* Reset the screen. */
	vision_reset();		/* reset the blockages */
	docrt();		/* does a full vision recalc */

	/*
	 *  Move all plines beyond the screen reset.
	 */

	/* give room entrance message, if any */
	check_special_room(FALSE);

	/* Check whether we just entered Gehennom. */
	if (!In_hell(&u.uz0) && Inhell) {
	    if (Is_valley(&u.uz)) {
		You("arrive at the Valley of the Dead...");
		pline_The("odor of burnt flesh and decay pervades the air.");
#ifdef MICRO
		display_nhwindow(WIN_MESSAGE, FALSE);
#endif
		You_hear("groans and moans everywhere.");
	    } else pline("It is hot here.  You smell smoke...");
	}

	if (familiar) {
	    static const char *fam_msgs[4] = {
		"You have a sense of deja vu.",
		"You feel like you've been here before.",
		"This place looks familiar...",
		0	/* no message */
	    };
	    static const char *halu_fam_msgs[4] = {
		"Whoa!  Everything looks different.",
		"You are surrounded by twisty little passages, all alike.",
		"Gee, this looks like uncle Conan's place...",
		0	/* no message */
	    };
	    const char *mesg;
	    int which = rn2(4);

	    if (Hallucination)
		mesg = halu_fam_msgs[which];
	    else
		mesg = fam_msgs[which];
	    if (mesg) pline(mesg);
	}

#ifdef REINCARNATION
	if (new && Is_rogue_level(&u.uz))
	    You("enter what seems to be an older, more primitive world.");
#endif
	/* Final confrontation */
	if (In_endgame(&u.uz) && newdungeon && u.uhave.amulet)
		resurrect();
	if (newdungeon && In_V_tower(&u.uz) && In_hell(&u.uz0))
		pline_The("heat and smoke are gone.");

	/* the message from your quest leader */
	if (!In_quest(&u.uz0) && at_dgn_entrance("The Quest") &&
		!(u.uevent.qexpelled || u.uevent.qcompleted || leaderless())) {

		if (u.uevent.qcalled) {
			com_pager(Role_is('R') ? 4 : 3);
		} else {
			com_pager(2);
			u.uevent.qcalled = TRUE;
		}
	}

	/* once Croesus is dead, his alarm doesn't work any more */
	if (Is_knox(&u.uz) && (new || !mvitals[PM_CROESUS].died)) {
		register struct monst *mtmp;

		You("penetrated a high security area!");
		pline("An alarm sounds!");
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		    if(mtmp->msleep) mtmp->msleep = 0;
	}

	if(on_level(&u.uz, &astral_level)) {
		register struct monst *mtmp;

		/* reset monster hostility relative to player */
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		    reset_hostility(mtmp);

		/* create some player-monsters */
		create_mplayers(rn1(4, 3), TRUE);

		/* create a guardian angel next to player, if worthy */
		if (Conflict) {
		    coord mm;
		    int i = rnd(4);
	pline("A voice booms: \"Thy desire for conflict shall be fulfilled!\"");
		    while(i--) {
			mm.x = u.ux;
			mm.y = u.uy;
			if(enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
			    (void) mk_roamer(&mons[PM_ANGEL], u.ualign.type,
						mm.x, mm.y, FALSE);
		    }

		} else if(u.ualign.record > 8 /* fervent */) {
		    coord mm;

		pline("A voice whispers: \"Thou hast been worthy of me!\"");
		    mm.x = u.ux;
		    mm.y = u.uy;
		    if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL])) {
			if ((mtmp = mk_roamer(&mons[PM_ANGEL], u.ualign.type,
					   mm.x, mm.y, TRUE)) != 0) {
			    register struct obj *otmp;

			    if (!Blind)
				pline("An angel appears near you.");
			    else
			You_feel("the presence of a friendly angel near you.");
			    /* guardian angel -- the one case mtame doesn't
			     * imply an edog structure, so we don't want to
			     * call tamedog().
			     */
			    mtmp->mtame = 10;
			    /* make him strong enough vs. endgame foes */
			    mtmp->m_lev = rn1(8,15);
			    mtmp->mhp = mtmp->mhpmax =
					d((int)mtmp->m_lev,10) + 30 + rnd(30);
			    if ((otmp = select_hwep(mtmp)) == 0) {
				otmp = mksobj(SILVER_SABER, FALSE, FALSE);
				mpickobj(mtmp, otmp);
			    }
			    bless(otmp);
			    if (otmp->spe < 4) otmp->spe += rnd(4);
			    if ((otmp = which_armor(mtmp, W_ARMS)) == 0
			      || otmp->otyp != SHIELD_OF_REFLECTION) {
				(void) mongets(mtmp, AMULET_OF_REFLECTION);
				m_dowear(mtmp, TRUE);
			    }
			}
		    }
		}
	}

	onquest();
	assign_level(&u.uz0, &u.uz); /* reset u.uz0 */

#ifdef INSURANCE
	save_currentstate();
#endif

	pickup(1);
}

static char *dfr_pre_msg = 0,	/* pline() before level change */
	    *dfr_post_msg = 0;	/* pline() after level change */

/* change levels at the end of this turn, after monsters finish moving */
void
schedule_goto(tolev, at_stairs, falling, portal_flag, pre_msg, post_msg)
d_level *tolev;
boolean at_stairs, falling;
int portal_flag;
const char *pre_msg, *post_msg;
{
	int typmask = 0100;		/* non-zero triggers `deferred_goto' */

	/* destination flags (`goto_level' args) */
	if (at_stairs)	 typmask |= 1;
	if (falling)	 typmask |= 2;
	if (portal_flag) typmask |= 4;
	if (portal_flag < 0) typmask |= 0200;	/* flag for portal removal */
	u.utotype = typmask;
	/* destination level */
	assign_level(&u.utolev, tolev);

	if (pre_msg)
	    dfr_pre_msg = strcpy((char *)alloc(strlen(pre_msg) + 1), pre_msg);
	if (post_msg)
	    dfr_post_msg = strcpy((char *)alloc(strlen(post_msg)+1), post_msg);

	/* we don't actually have to wait unless this is a monster's move */
	if (!flags.mon_moving) deferred_goto();
}

/* handle something like portal ejection */
void
deferred_goto()
{
	if (!on_level(&u.uz, &u.utolev)) {
	    d_level dest;
	    int typmask = u.utotype; /* save it; goto_level zeroes u.utotype */

	    assign_level(&dest, &u.utolev);
	    if (dfr_pre_msg) pline(dfr_pre_msg);
	    goto_level(&dest, !!(typmask&1), !!(typmask&2), !!(typmask&4));
	    if (typmask & 0200) {	/* remove portal */
		struct trap *t = t_at(u.ux, u.uy);

		if (t) {
		    deltrap(t);
		    newsym(u.ux, u.uy);
		}
	    }
	    if (dfr_post_msg) pline(dfr_post_msg);
	}
	u.utotype = 0;		/* our caller keys off of this */
	if (dfr_pre_msg)
	    free((genericptr_t)dfr_pre_msg),  dfr_pre_msg = 0;
	if (dfr_post_msg)
	    free((genericptr_t)dfr_post_msg),  dfr_post_msg = 0;
}

#endif /* OVL2 */
#ifdef OVL3

/*
 * Return TRUE if we created a monster for the corpse.  If successful, the
 * corpse is gone.
 */
boolean
revive_corpse(corpse)
struct obj *corpse;
{
    struct monst *mtmp, *mcarry;
    boolean is_uwep, chewed;
    xchar where;
    char *cname, cname_buf[BUFSZ];

    where = corpse->where;
    is_uwep = corpse == uwep;
    cname = eos(strcpy(cname_buf, "bite-covered "));
    Strcpy(cname, corpse_xname(corpse, TRUE));
    mcarry = (where == OBJ_MINVENT) ? corpse->ocarry : 0;
    mtmp = revive(corpse);	/* corpse is gone if successful */

    if (mtmp) {
	chewed = (mtmp->mhp < mtmp->mhpmax);
	if (chewed) cname = cname_buf;	/* include "bite-covered" prefix */
	switch (where) {
	    case OBJ_INVENT:
		if (is_uwep)
		    pline_The("%s writhes out of your grasp!", cname);
		else
		    You_feel("squirming in your backpack!");
		break;

	    case OBJ_FLOOR:
		if (cansee(mtmp->mx, mtmp->my))
		    pline("%s rises from the dead!", chewed ?
			  Adjmonnam(mtmp, "bite-covered") : Monnam(mtmp));
		break;

	    case OBJ_MINVENT:		/* probably a nymph's */
		if (cansee(mtmp->mx, mtmp->my)) {
		    if (canseemon(mcarry))
			pline("Startled, %s drops %s as it revives!",
			      mon_nam(mcarry), an(cname));
		    else
			pline("%s suddenly appears!", chewed ?
			      Adjmonnam(mtmp, "bite-covered") : Monnam(mtmp));
		}
		break;

	    default:
		/* we should be able to handle the other cases... */
		impossible("revive_corpse: lost corpse @ %d", where);
		break;
	}
	return TRUE;
    }
    return FALSE;
}

/* Revive the corpse via a timeout. */
/*ARGSUSED*/
void
revive_mon(arg, timeout)
genericptr_t arg;
long timeout;
{
    struct obj *body = (struct obj *) arg;

    /* if we succeed, the corpse is gone, otherwise, rot it away */
    if (!revive_corpse(body)) {
	if (is_rider(&mons[body->corpsenm]))
	    You_feel("less hassled.");
	(void) start_timer(250L - (monstermoves-body->age),
					TIMER_OBJECT, ROT_CORPSE, arg);
    }
}

int
donull()
{
	return(1);	/* Do nothing, but let other things happen */
}

#endif /* OVL3 */
#ifdef OVLB

STATIC_PTR int
wipeoff()
{
	if(u.ucreamed < 4)	u.ucreamed = 0;
	else			u.ucreamed -= 4;
	if (Blinded < 4)	Blinded = 0;
	else			Blinded -= 4;
	if (!Blinded) {
		pline("You've got the glop off.");
		u.ucreamed = 0;
		Blinded = 1;
		make_blinded(0L,TRUE);
		return(0);
	} else if (!u.ucreamed) {
		Your("%s feels clean now.", body_part(FACE));
		return(0);
	}
	return(1);		/* still busy */
}

int
dowipe()
{
	if(u.ucreamed)  {
		static NEARDATA char buf[39];

		Sprintf(buf, "wiping off your %s", body_part(FACE));
		set_occupation(wipeoff, buf, 0);
		/* Not totally correct; what if they change back after now
		 * but before they're finished wiping?
		 */
		return(1);
	}
	Your("%s is already clean.", body_part(FACE));
	return(1);
}

void
set_wounded_legs(side, timex)
register long side;
register int timex;
{
	if(!Wounded_legs) {
		ATEMP(A_DEX)--;
		flags.botl = 1;
	}

	if(!Wounded_legs || (Wounded_legs & TIMEOUT))
		Wounded_legs |= side + timex;
	else
		Wounded_legs |= side;
	(void)encumber_msg();
}

void
heal_legs()
{
	if(Wounded_legs) {
		if (ATEMP(A_DEX) < 0) {
			ATEMP(A_DEX)++;
			flags.botl = 1;
		}

		if((Wounded_legs & BOTH_SIDES) == BOTH_SIDES) {
			Your("%s feel somewhat better.",
				makeplural(body_part(LEG)));
		} else {
			Your("%s feels somewhat better.",
				body_part(LEG));
		}
		Wounded_legs = 0;
	}
	(void)encumber_msg();
}

#endif /* OVLB */

/*do.c*/
