/*	SCCS Id: @(#)do.c	3.1	93/03/30	*/
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
extern int errno;
#endif

#ifdef MFLOPPY
extern struct finfo fileinfo[];
#else
extern boolean level_exists[];
#endif

#ifdef SINKS
# ifdef OVLB
static void FDECL(trycall, (struct obj *));
# endif /* OVLB */
STATIC_DCL void FDECL(dosinkring, (struct obj *));
#endif /* SINKS */

STATIC_PTR int FDECL(drop, (struct obj *));
STATIC_PTR int NDECL(wipeoff);

#ifdef OVL2
static int NDECL(currentlevel_rewrite);
/* static boolean FDECL(badspot, (XCHAR_P,XCHAR_P)); */
#endif

#ifdef OVLB

static NEARDATA const char drop_types[] =
	{ ALLOW_COUNT, GOLD_CLASS, ALL_CLASSES, 0 };

int
dodrop()
{
	int result;

	if (*u.ushops) sellobj_state(TRUE);
	result = drop(getobj(drop_types, "drop"));
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
			You("hear a%s splash.", lava ? " sizzling" : "");
		    wake_nearby();
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
 * called with the object not in any chain.  Returns 1 if the object is
 * gone.
 */
boolean
flooreffects(obj,x,y,verb)
struct obj *obj;
int x,y;
const char *verb;
{
	struct trap *t;

	/* make sure things like water_damage() have no pointers to follow */
	obj->nobj = obj->nexthere = (struct obj *)0;

	if (obj->otyp == BOULDER && boulder_hits_pool(obj, x, y, FALSE))
		return TRUE;
	else if (obj->otyp == BOULDER && (t = t_at(x,y)) != 0 &&
		 (t->ttyp==PIT || t->ttyp==SPIKED_PIT || t->ttyp==TRAPDOOR)) {
		struct monst *mtmp;

		bury_objs(x, y);
		if(!Can_fall_thru(&u.uz) && t->ttyp == TRAPDOOR)
			return FALSE;
		if (((mtmp = m_at(x, y)) && mtmp->mtrapped) ||
		    (u.utrap && x==u.ux && y==u.uy)) {
		    /* u.utrap = 0;     /* player remains trapped. See trap.c */
		    if (*verb)
			pline("The boulder %ss into the pit%s.", verb,
				(mtmp) ? "" : " with you");
		    if (mtmp) {
			if (!passes_walls(mtmp->data) && !throws_rocks(mtmp->data))
				if (hmon(mtmp, obj, TRUE))
				    return FALSE;	/* still alive */
				else
				    bury_objs(x, y);	/* treasure, corpse */
		    } else
#ifdef POLYSELF
			if (!passes_walls(uasmon) && !throws_rocks(uasmon))
#endif
			{
				losehp(rnd(15), "squished under a boulder",
					NO_KILLER_PREFIX);
				return FALSE;
			}
		}
		if (*verb) {
			if (Blind) {
				if ((x == u.ux) && (y == u.uy))
					You("hear a CRASH! beneath you.");
				else
					You("hear the boulder %s.", verb);
			} else if (cansee(x, y)) {
				pline("The boulder %sfills a %s.",
					 t->tseen ? "" : "triggers and ",
					 t->ttyp == TRAPDOOR ?
					 "trap door" : "pit");
			}
		}
		deltrap(t);
		obfree(obj, (struct obj *)0);
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
			an(Hallucination ? hcolor() :
				obj->blessed ? amber : Black),
			doname(obj),
			(obj->quan == 1L) ? "s" : "");
		if (!Hallucination) obj->bknown = 1;
	} else {
		pline("%s land%s on the altar.", Doname2(obj),
			(obj->quan == 1L) ? "s" : "");
		if (obj->otyp != GOLD_PIECE)
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
		pline("The sink quivers upward for a moment.");
		break;
	    case RIN_POISON_RESISTANCE:
#ifdef TUTTI_FRUTTI
		You("smell rotten %s.", makeplural(pl_fruit));
#else
		You("smell rotten fruit.");
#endif
		break;
	    case RIN_AGGRAVATE_MONSTER:
		pline("Several flies buzz angrily around the sink.");
		break;
	    case RIN_SHOCK_RESISTANCE:
		pline("Static electricity surrounds the sink.");
		break;
	    case RIN_CONFLICT:
		You("hear loud noises coming from the drain.");
		break;
	    case RIN_GAIN_STRENGTH:
		pline("The water flow seems %ser now.",
			(obj->spe<0) ? "weak" : "strong");
		break;
	    case RIN_INCREASE_DAMAGE:
		pline("The water's force seems %ser now.",
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
		    pline("The faucets flash brightly for a moment.");
		    break;
		case RIN_REGENERATION:
		    pline("The sink looks as good as new.");
		    break;
		case RIN_INVISIBILITY:
		    You("don't see anything happen to the sink.");
		    break;
		case RIN_SEE_INVISIBLE:
		    You("see some air in the sink.");
		    break;
		case RIN_STEALTH:
		pline("The sink seems to blend into the floor for a moment.");
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
		pline("The hot water faucet flashes brightly for a moment.");
		    break;
		case RIN_COLD_RESISTANCE:
		pline("The cold water faucet flashes brightly for a moment.");
		    break;
		case RIN_PROTECTION_FROM_SHAPE_CHAN:
		    pline("The sink looks nothing like a fountain.");
		    break;
		case RIN_PROTECTION:
		    pline("The sink glows %s for a moment.",
			    Hallucination ? hcolor() :
			    (obj->spe<0) ? Black : silver);
		    break;
		case RIN_WARNING:
		    pline("The sink glows %s for a moment.",
			    Hallucination ? hcolor() : White);
		    break;
		case RIN_TELEPORTATION:
		    pline("The sink momentarily vanishes.");
		    break;
		case RIN_TELEPORT_CONTROL:
	    pline("The sink looks like it is being beamed aboard somewhere.");
		    break;
#ifdef POLYSELF
		case RIN_POLYMORPH:
		    pline("The sink momentarily looks like a fountain.");
		    break;
		case RIN_POLYMORPH_CONTROL:
	pline("The sink momentarily looks like a regularly erupting geyser.");
		    break;
#endif
	    }
	}
	if(ideed)
	    trycall(obj);
	else
	    You("hear the ring bouncing down the drainpipe.");
	if (!rn2(20)) {
		pline("The sink backs up, leaving %s.", doname(obj));
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
			Norep("You cannot %s something you are wearing.",word);
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
#ifdef WALKIES
	if (obj->otyp == LEASH && obj->leashmon != 0) {
		if (*word)
			pline ("The leash is tied around your %s.",
					body_part(HAND));
		return(FALSE);
	}
#endif
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
#ifdef SINKS
	if((obj->oclass == RING_CLASS) && IS_SINK(levl[u.ux][u.uy].typ)
							&& !u.uswallow) {
		dosinkring(obj);
		return(1);
	}
#endif
	if (Levitation && !(Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ||
			    is_pool(u.ux, u.uy) || u.uswallow)) {
	    if(flags.verbose) You("drop %s.", doname(obj));
	    if (obj->otyp != GOLD_PIECE) freeinv(obj);
	    (void) snuff_candle(obj);
	    hitfloor(obj);
	    return(1);
	}
	if (IS_ALTAR(levl[u.ux][u.uy].typ) && !u.uswallow) {
		doaltarobj(obj);	/* set bknown */
	} else
		if(flags.verbose) You("drop %s.", doname(obj));
	dropx(obj);
	return(1);
}

/* Called in several places - should not produce texts */
void
dropx(obj)
register struct obj *obj;
{
	/* Money is *not* in our inventory */
	if (obj->otyp != GOLD_PIECE) freeinv(obj);
	(void) snuff_candle(obj);
	if(!u.uswallow && obj != uball &&
			  ship_object(obj, u.ux, u.uy, FALSE)) return;
	dropy(obj);
}

void
dropy(obj)
register struct obj *obj;
{
	if (!u.uswallow && flooreffects(obj,u.ux,u.uy,"drop")) return;
	if(obj->otyp == CRYSKNIFE)
		obj->otyp = WORM_TOOTH;
	(void) snuff_candle(obj);
	/* uswallow check done by GAN 01/29/87 */
	if(u.uswallow) {
		if (obj != uball) {		/* mon doesn't pick up ball */
		    mpickobj(u.ustuck,obj);
		}
	} else  {
		obj->nobj = fobj;
		fobj = obj;
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

/* drop several things */
int
doddrop()
{
	int result;

	if (*u.ushops) sellobj_state(TRUE);
	result = ggetobj("drop", drop, 0);
	if (*u.ushops) sellobj_state(FALSE);
	reset_occupations();

	return result;
}

#endif /* OVL0 */
#ifdef OVL2

/* on a ladder, used in goto_level */
static NEARDATA boolean at_ladder = FALSE;

int
dodown()
{
	struct trap *trap = 0;

	if( (u.ux != xdnstair || u.uy != ydnstair)
	     && (!xdnladder || u.ux != xdnladder || u.uy != ydnladder)
	     && (!sstairs.sx || u.ux != sstairs.sx || u.uy != sstairs.sy
			|| sstairs.up)
	  ) {
		if (!(trap = t_at(u.ux,u.uy)) || trap->ttyp != TRAPDOOR
			|| !Can_fall_thru(&u.uz) || !trap->tseen) {
			You("can't go down here.");
			return(0);
		}
	}
	if(u.ustuck) {
		You("are being held, and cannot go down.");
		return(1);
	}
	if(Levitation) {
		pline("You're floating high above the %s.",
			levl[u.ux][u.uy].typ == STAIRS ? "stairs" :
			levl[u.ux][u.uy].typ == LADDER ? "ladder" :
			"trap door");
		return(0);
	}
	if (on_level(&valley_level, &u.uz) && !u.uevent.gehennom_entered) {
		You("are standing at the gate to Gehennom.");
		pline("Unspeakable cruelty and harm lurk down there.");
		if (yn("Are you sure you want to enter?") != 'y')
			return(0);
		else pline("So be it.");
		u.uevent.gehennom_entered = 1;	/* don't ask again */
	}

#ifdef WALKIES
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	}
#endif
	if (trap)
#ifdef POLYSELF
		You("%s into the trap door.", locomotion(uasmon, "jump"));
#else
		You("jump into the trap door.");
#endif
	if (levl[u.ux][u.uy].typ == LADDER) at_ladder = TRUE;
	next_level(!trap);
	at_ladder = FALSE;
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
		You("can't go up here.");
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
#ifdef WALKIES
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	}
#endif
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
register boolean at_stairs, falling, portal;
{
	register int fd;
	register boolean up = (depth(newlevel) < depth(&u.uz));
	register boolean newdungeon = (u.uz.dnum != newlevel->dnum);
#ifdef REINCARNATION
	int new = 0;	/* made a new level? */
#endif

	if (dunlev(newlevel) > dunlevs_in_dungeon(newlevel))
		newlevel->dlevel = dunlevs_in_dungeon(newlevel);
	if (newdungeon && In_endgame(newlevel)) { /* 1st Endgame Level !!! */
		if (u.uhave.amulet)
		    assign_level(newlevel, &earth_level);
		else return;
	}
	if (ledger_no(newlevel) <= 0)
		done(ESCAPED);	/* in fact < 0 is impossible */
	/* If you have the amulet and are trying to get out of Hell, going
	 * up a set of stairs sometimes does some very strange things!
	 */
	if (Inhell && up && !newdungeon && u.uhave.amulet &&
				(dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz)-3)) {
		if (!rn2(4)) {
		    if (u.ualign.type == A_CHAOTIC ||
			    (u.ualign.type == A_NEUTRAL && rn2(2)))
			assign_level(newlevel, &u.uz);
		    else
			assign_rnd_level(newlevel, &u.uz, rnd(3));

		    pline("A mysterious force momentarily surrounds you...");
		    if (on_level(newlevel, &u.uz)) {
			(void) safe_teleds();
#ifdef WALKIES
			(void) next_to_u();
#endif
			return;
		    }
		}
	}
#ifdef MULDGN
	/* Prevent the player from going past the first quest level unless
	 * (s)he has been given the go-ahead by the leader.
	 */
	if (on_level(&u.uz, &qstart_level) && !newdungeon && !ok_to_quest()) {

		pline("A mysterious force prevents you from descending.");
		return;
	}
#endif
	if (on_level(newlevel, &u.uz)) return;		/* this can happen */

	fd = currentlevel_rewrite();
	if (fd < 0) return;

	if (falling) /* assuming this is only trapdoor */
	    impact_drop((struct obj *)0, u.ux, u.uy, newlevel->dlevel);

	check_special_room(TRUE);		/* probably was a trap door */
	if (Punished) unplacebc();
	u.utrap = 0;				/* needed in level_tele */
	fill_pit(u.ux, u.uy);
	u.ustuck = 0;				/* idem */
	u.uinwater = 0;
	keepdogs();
	if (u.uswallow)				/* idem */
		u.uswldtim = u.uswallow = 0;
	/*
	 *  We no longer see anything on the level.  Make sure that this
	 *  follows u.uswallow set to null since uswallow overrides all
	 *  normal vision.
	 */
	vision_recalc(2);
	bufon(fd);
	savelev(fd,ledger_no(&u.uz), WRITE_SAVE | FREE_SAVE);
	bclose(fd);

#ifdef REINCARNATION
	if (Is_rogue_level(newlevel) || Is_rogue_level(&u.uz))
		assign_rogue_graphics(Is_rogue_level(newlevel));
#endif
	assign_level(&u.uz0, &u.uz);
	assign_level(&u.uz, newlevel);
	assign_level(&u.utolev, newlevel);
	u.utotype = 0;
	if (dunlev_reached(&u.uz) < dunlev(&u.uz))
		dunlev_reached(&u.uz) = dunlev(&u.uz);

	/* set default level change destination areas */
	/* the special level code may override these */
	(void) memset((genericptr_t) &updest, 0, sizeof updest);
	(void) memset((genericptr_t) &dndest, 0, sizeof dndest);

	if (In_endgame(&u.uz) ||
#ifdef MFLOPPY
	    /* If the level has no .where yet, it hasn't been made */
	    !fileinfo[ledger_no(&u.uz)].where) {
#else
	    !level_exists[ledger_no(&u.uz)]) {
#endif
		mklev();
#ifdef REINCARNATION
		new = 1;	/* made the level */
#endif
	} else {
		fd = open_levelfile(ledger_no(&u.uz));
		if (fd < 0) {
			pline("Cannot open file (#%d) for level %d (errno %d).",
					ledger_no(&u.uz), depth(&u.uz), errno);
			pline("Probably someone removed it.");
			done(TRICKED);
		}
		minit();	/* ZEROCOMP */
		getlev(fd, hackpid, ledger_no(&u.uz), FALSE);
		(void) close(fd);
	}

	if (portal && !In_endgame(&u.uz)) {
	    /* find the portal on the new level */
	    register struct trap *ttrap;

	    for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap)
		if (ttrap->ttyp == MAGIC_PORTAL) break;

	    if (!ttrap) panic("goto_level: no corresponding portal!");
	    u.ux = ttrap->tx;
	    u.uy = ttrap->ty;
	} else if (at_stairs && !In_endgame(&u.uz)) {
	    if (up) {
		if (at_ladder) {
		    u.ux = xdnladder;
		    u.uy = ydnladder;
		} else {
		    if (newdungeon) {
			if (Is_stronghold(&u.uz)) {
			    register xchar x, y;

			    do {
				x = (COLNO - 2 - rnd(5));
				y = rn1(ROWNO - 4, 3);
			    } while(occupied(x, y) ||
				    IS_WALL(levl[x][y].typ));
			    u.ux = x;
			    u.uy = y;
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
		    u.ux = xupladder;
		    u.uy = yupladder;
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
	    if (up)
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
	losedogs();
	obj_delivery();

	initrack();

	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
	if(MON_AT(u.ux, u.uy)) {
		impossible("mnexto failed (do.c)?");
		rloc(m_at(u.ux, u.uy));
	}
	remove_cadavers(&fobj);	/* remove rotted meat (before seen) */

	/* initial movement of bubbles just before vision_recalc */
	if (Is_waterlevel(&u.uz))
		movebubbles();

	/* Reset the screen. */
	vision_reset();		/* reset the blockages */
	docrt();		/* does a full vision recalc */

	/* give room entrance message, if any */
	check_special_room(FALSE);

	/* In Nethack 3.1, Gehennom starts after the stronghold.  Moreover,
	 * there are traps in the stronghold, that can send the player
	 * to Gehennom (gnark, gnark)!  So we have to test here:
	 */
	if(!In_hell(&u.uz0) && Inhell) {
	    if(Is_valley(newlevel)) {
		You("arrive at the Valley of the Dead...");
		pline("The odor of burnt flesh and decay pervades the air.");
#ifdef MICRO
		display_nhwindow(WIN_MESSAGE, FALSE);
#endif
		You("hear groans and moans everywhere.");
	    } else pline("It is hot here.  You smell smoke...");
	}

#ifdef REINCARNATION
	/*
	 *  Move all plines beyond the screen reset.
	 */
	if (new && Is_rogue_level(&u.uz))
	    You("enter what seems to be an older, more primitive world.");
#endif
	/* Final confrontation */
	if (In_endgame(&u.uz) && newdungeon && u.uhave.amulet &&
						   flags.no_of_wizards == 0)
		resurrect();
	if (newdungeon && In_tower(&u.uz))
		pline("The heat and smoke are gone.");
#ifdef MULDGN
	/* the message from your quest leader */
	if (!In_quest(&u.uz0) && at_dgn_entrance("The Quest") &&
		!(u.uevent.qexpelled || u.uevent.qcompleted || leaderless())) {

		if (u.uevent.qcalled) {
			com_pager(3);
		} else {
			com_pager(2);
			u.uevent.qcalled = TRUE;
		}
	}

	if(Is_knox(&u.uz)) {
		register struct monst *mtmp;

		You("penetrated a high security area!");
		pline("An alarm sounds!");
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		    if(mtmp->msleep) mtmp->msleep = 0;
	}
#endif /* MULDGN */
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
			    register struct obj *otmp =
					   mksobj(SILVER_SABER, FALSE, FALSE);

			    if (!Blind)
				pline("An angel appears near you.");
			    else
			You("feel the presence of a friendly angel near you.");
			    /* guardian angel -- the one case mtame doesn't
			     * imply an edog structure, so we don't want to
			     * call tamedog().
			     */
			    mtmp->mtame = 10;
			    /* make him strong enough vs. endgame foes */
			    mtmp->m_lev = rn1(8,15);
			    mtmp->mhp = mtmp->mhpmax =
					d((int)mtmp->m_lev,10) + 30 + rnd(30);
			    bless(otmp);
			    otmp->spe = 7;
			    mpickobj(mtmp, otmp);
			}
		    }
		}
	}

#ifdef MULDGN
	onquest();
#endif
	assign_level(&u.uz0, &u.uz); /* reset u.uz0 */

#ifdef INSURANCE
	save_currentstate();
#endif

	pickup(1);
}

/* handle something like portal ejection */
void
deferred_goto()
{
	if (!on_level(&u.uz, &u.utolev)) {
	    d_level dest;
	    int typmask = u.utotype; /* save it; goto_level zeroes u.utotype */

	    assign_level(&dest, &u.utolev);
	    goto_level(&dest, !!(typmask&4), !!(typmask&2), !!(typmask&1));
	    if (typmask & 0200) {	/* remove portal */
		deltrap(t_at(u.ux, u.uy));
		newsym(u.ux, u.uy);
	    }
	}
}

#endif /* OVL2 */
#ifdef OVL3

void
revive_corpse(corpse, odds, mark_as_old)    /* see remove_cadavers() */
register struct obj *corpse;
int odds;
boolean mark_as_old;
{
    register struct monst *mtmp;

    corpse->oldcorpse = mark_as_old;

    /* odds == 0 is a special case meaning 'always revive' */
    if (!odds || !rn2(odds)) {
	if (carried(corpse)) {
	    if (corpse == uwep) {
		if ((mtmp = revive(corpse,TRUE)) != 0)
		    pline("The %s%s %s writhes out of your grasp!",
			(mtmp->mhp < mtmp->mhpmax) ? "bite-covered ":"",
				mtmp->data->mname, xname(corpse));
	    } else if ((mtmp = revive(corpse,TRUE)) != 0)
		You("feel squirming in your backpack!");
	} else {
	    if ((mtmp = revive(corpse,FALSE)) && cansee(mtmp->mx,mtmp->my))
		pline("%s rises from the dead!",
		    (mtmp->mhp==mtmp->mhpmax) ? Monnam(mtmp)
			: Adjmonnam(mtmp, "bite-covered"));
	}
    }
}

/*
 *  Remove old cadavers from any object chain.  Regenerate trolls
 *  thanks to KAA.  Follows containers (except ice boxes).
 */
#define TAINT_AGE	50	/* min. limit for tainting.  see eat.c */
#define ODDS_RV1	37	/* 1/37 odds for 50 moves = 75% revive */
#define ODDS_RV2	2	/* special case 1/2 odds of reviving */

void
remove_cadavers(chain)
struct obj **chain;
{
    register struct obj *obj, *nobj, *pobj = (struct obj *)0;
    register long corpse_age;
    boolean ininv = (*chain == invent);
    boolean onfloor = (*chain == fobj);
    boolean buried = (*chain == level.buriedobjlist);

    for (obj = *chain; obj; obj = nobj) {
	nobj = obj->nobj;

	if (obj->otyp == CORPSE) {
	    /* corpses laying on ice deteriorate more slowly */
	    if (onfloor && obj->age < monstermoves &&
		rn2(3) && is_ice(obj->ox, obj->oy)) obj->age++;
	    corpse_age = monstermoves - obj->age;

	    if (is_rider(&mons[obj->corpsenm]) && corpse_age >= 12 && onfloor) {
		/* these always come back eventually */
		/* riders can't be picked up, but can be buried, but
		 * the astral level seems to rule out that possibility
		 */
		revive_corpse(obj, 3, FALSE);
	    } else if (mons[obj->corpsenm].mlet == S_TROLL && !obj->oldcorpse
		       && !(mons[obj->corpsenm].geno & (G_GENOD | G_EXTINCT))
		       && (onfloor || ininv)) {

		/* the corpse has 50 moves, the lower limit for tainting,
		 * to attempt re-animation.  if it is unsuccessful it is
		 * marked to prevent further attempts.  if we leave the
		 * level and return to old corpses that haven't been marked
		 * they're given a one-shot chance to re-animate.
		 */
		if (corpse_age < TAINT_AGE)
		    revive_corpse(obj, ODDS_RV1, FALSE);
		else if (corpse_age == TAINT_AGE)
		    revive_corpse(obj, ODDS_RV1, TRUE);
		else revive_corpse(obj, ODDS_RV2, TRUE);

	    } else if (obj->corpsenm != PM_LIZARD && (250 < corpse_age)) {
		if(ininv)
		    useup(obj);
		else if(onfloor)
		    delobj(obj);
		else if(buried)
		    delburiedobj(obj);
		else { /* in a container */
		    if(pobj) pobj->nobj = nobj;
		    else *chain = nobj;
		    obfree(obj, (struct obj *) 0);
		    obj = 0;
		}
	    }
	} else if (Has_contents(obj) && obj->otyp != ICE_BOX) {
	    remove_cadavers(&obj->cobj);
	}
	/* pobj is only used for containers, which don't allow revive() -dlc */
	/* and for monster inventory (special cases only) under MUSE */
	if (obj) pobj = obj;
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

#endif /* OVLB */
#ifdef OVL1

/* split obj so that it gets size num */
/* remainder is put in the object structure delivered by this call */
struct obj *
splitobj(obj, num)
register struct obj *obj;
register long num;
{
register struct obj *otmp;
	/* assert(0 < num && num < obj->quan); */
	otmp = newobj(obj->onamelth);
	*otmp = *obj;		/* copies whole structure */
	otmp->o_id = flags.ident++;
	obj->quan = num;
	obj->owt = weight(obj);
	otmp->quan -= num;
	otmp->owt = weight(otmp);	/* -= obj->owt ? */
	obj->nobj = obj->nexthere = otmp;
	if (obj->onamelth)
		(void)strncpy(ONAME(otmp), ONAME(obj), (int)obj->onamelth);
	if(obj->unpaid) splitbill(obj,otmp);
	return(otmp);
}

#endif /* OVL1 */
#ifdef OVLB

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
}

#endif /* OVLB */

/*do.c*/
