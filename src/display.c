/*	SCCS Id: @(#)display.c	3.1	92/10/25	*/
/* Copyright (c) Dean Luick, with acknowledgements to Kevin Darcy */
/* and Dave Cohrs, 1990.					  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *			THE NEW DISPLAY CODE
 *
 * The old display code has been broken up into three parts: vision, display,
 * and drawing.  Vision decides what locations can and cannot be physically
 * seen by the hero.  Display decides _what_ is displayed at a given location.
 * Drawing decides _how_ to draw a monster, fountain, sword, etc.
 *
 * The display system uses information from the vision system to decide
 * what to draw at a given location.  The routines for the vision system
 * can be found in vision.c and vision.h.  The routines for display can
 * be found in this file (display.c) and display.h.  The drawing routines
 * are part of the window port.  See doc/window.doc for the drawing
 * interface.
 *
 * The display system deals with an abstraction called a glyph.  Anything
 * that could possibly be displayed has a unique glyph identifier.
 *
 * What is seen on the screen is a combination of what the hero remembers
 * and what the hero currently sees.  Objects and dungeon features (walls
 * doors, etc) are remembered when out of sight.  Monsters and temporary
 * effects are not remembered.  Each location on the level has an
 * associated glyph.  This is the hero's _memory_ of what he or she has
 * seen there before.
 *
 * Display rules:
 *
 *	If the location is in sight, display in order:
 *		visible monsters
 *		visible objects
 *		known traps
 *		background
 *
 *	If the location is out of sight, display in order:
 *		sensed monsters (telepathy)
 *		memory
 *
 *
 *
 * Here is a list of the major routines in this file to be used externally:
 *
 * newsym
 *
 * Possibly update the screen location (x,y).  This is the workhorse routine.
 * It is always correct --- where correct means following the in-sight/out-
 * of-sight rules.  **Most of the code should use this routine.**  This
 * routine updates the map and displays monsters.
 *
 *
 * map_background
 * map_object
 * map_trap
 * unmap_object
 *
 * If you absolutely must override the in-sight/out-of-sight rules, there
 * are two possibilities.  First, you can mess with vision to force the
 * location in sight then use newsym(), or you can  use the map_* routines.
 * The first has not been tried [no need] and the second is used in the
 * detect routines --- detect object, magic mapping, etc.  The map_*
 * routines *change* what the hero remembers.  All changes made by these
 * routines will be sticky --- they will survive screen redraws.  Do *not*
 * use these for things that only temporarily change the screen.  These
 * routines are also used directly by newsym().  unmap_object is used to
 * clear a remembered object when/if detection reveals it isn't there.
 *
 *
 * show_glyph
 *
 * This is direct (no processing in between) buffered access to the screen.
 * Temporary screen effects are run through this and its companion,
 * flush_screen().  There is yet a lower level routine, print_glyph(),
 * but this is unbuffered and graphic dependent (i.e. it must be surrounded
 * by graphic set-up and tear-down routines).  Do not use print_glyph().
 *
 *
 * see_monsters
 * see_objects
 *
 * These are only used when something affects all of the monsters or
 * objects.  For objects, the only thing is hallucination.  For monsters,
 * there are hallucination and changing from/to blindness, etc.
 *
 *
 * tmp_at
 *
 * This is a useful interface for displaying temporary items on the screen.
 * Its interface is different than previously, so look at it carefully.
 *
 *
 *
 * Parts of the rm structure that are used:
 *
 *	typ	- What is really there.
 *	glyph	- What the hero remembers.  This will never be a monster.
 *		  Monsters "float" above this.
 *	lit	- True if the position is lit.  An optimization for
 *		  lit/unlit rooms.
 *	waslit	- True if the position was *remembered* as lit.
 *	seen	- Set to true when the location is seen or felt as it really
 *		  is.  This is used primarily for walls, which look like stone
 *		  if seen from the outside of a room.  However, this is
 *		  also used as a guide for blind heros.  If the hero has
 *		  seen or felt a room feature underneath a boulder, when the
 *		  boulder is moved, the hero should see it again.  This is
 *		  also used as an indicator for unmapping detected objects.
 *
 *	doormask   - Additional information for the typ field.
 *	horizontal - Indicates whether the wall or door is horizontal or
 *		     vertical.
 */
#include "hack.h"

static void FDECL(display_monster,(XCHAR_P,XCHAR_P,struct monst *,int,XCHAR_P));
static int FDECL(swallow_to_glyph, (int, int));

#ifdef INVISIBLE_OBJECTS
/*
 * vobj_at()
 *
 * Returns a pointer to an object if the hero can see an object at the
 * given location.  This takes care of invisible objects.  NOTE, this
 * assumes that the hero is not blind and on top of the object pile.
 * It does NOT take into account that the location is out of sight, or,
 * say, one can see blessed, etc.
 */
struct obj *
vobj_at(x,y)
    xchar x,y;
{
    register struct obj *obj = level.objects[x][y];

    while (obj) {
	if (!obj->oinvis || See_invisible) return obj;
	obj = obj->nexthere;
    }
    return ((struct obj *) 0);
}
#endif	/* else vobj_at() is defined in display.h */

/*
 * The routines map_background(), map_object(), and map_trap() could just
 * as easily be:
 *
 *	map_glyph(x,y,glyph,show)
 *
 * Which is called with the xx_to_glyph() in the call.  Then I can get
 * rid of 3 routines that don't do very much anyway.  And then stop
 * having to create fake objects and traps.  However, I am reluctant to
 * make this change.
 */

/*
 * map_background()
 *
 * Make the real background part of our map.  This routine assumes that
 * the hero can physically see the location.  Update the screen if directed.
 */
void
map_background(x, y, show)
    register xchar x,y;
    register int  show;
{
    register int glyph = back_to_glyph(x,y);

    if (level.flags.hero_memory)
	levl[x][y].glyph = glyph;
    if (show) show_glyph(x,y, glyph);
}

/*
 * map_trap()
 *
 * Map the trap and print it out if directed.  This routine assumes that the
 * hero can physically see the location.
 */
void
map_trap(trap, show)
    register struct trap *trap;
    register int	 show;
{
    register int x = trap->tx, y = trap->ty;
    register int glyph = trap_to_glyph(trap);

    if (level.flags.hero_memory)
	levl[x][y].glyph = glyph;
    if (show) show_glyph(x, y, glyph);
}

/*
 * map_object()
 *
 * Map the given object.  This routine assumes that the hero can physically
 * see the location of the object.  Update the screen if directed.
 */
void
map_object(obj, show)
    register struct obj *obj;
    register int	show;
{
    register int x = obj->ox, y = obj->oy;
    register int glyph = obj_to_glyph(obj);

    if (level.flags.hero_memory)
	levl[x][y].glyph = glyph;
    if (show) show_glyph(x, y, glyph);
}

/*
 * unmap_object()
 *
 * Remove something from the map when detection reveals that it isn't
 * there any more.  Replace it with background or known trap, but not
 * with any other remembered object.  No need to update the display;
 * a full update is imminent.
 *
 * This isn't quite correct due to overloading of the seen bit.  But
 * it works well enough for now.
 */
void
unmap_object(x, y)
    register int x, y;
{
    register struct trap *trap;

    if (!level.flags.hero_memory) return;

    if ((trap = t_at(x,y)) != 0 && trap->tseen && !covers_traps(x,y))
	map_trap(trap, 0);
    else if (levl[x][y].seen) {
	struct rm *lev = &levl[x][y];

	map_background(x, y, 0);

	/* turn remembered dark room squares dark */
	if (!lev->waslit && lev->glyph == cmap_to_glyph(S_room) &&
							    lev->typ == ROOM)
	    lev->glyph = cmap_to_glyph(S_stone);
    } else 
	levl[x][y].glyph = cmap_to_glyph(S_stone);	/* default val */
}


/*
 * map_location()
 *
 * Make whatever at this location show up.  This is only for non-living
 * things.  This will not handle feeling invisible objects correctly.
 */
#define map_location(x,y,show)						\
{									\
    register struct obj   *obj;						\
    register struct trap  *trap;					\
									\
    if ((obj = vobj_at(x,y)) && !covers_objects(x,y))			\
	map_object(obj,show);						\
    else if ((trap = t_at(x,y)) && trap->tseen && !covers_traps(x,y))	\
	map_trap(trap,show);						\
    else								\
	map_background(x,y,show);					\
}


/*
 * display_monster()
 *
 * Note that this is *not* a map_XXXX() function!  Monsters sort of float
 * above everything.
 *
 * Yuck.  Display body parts by recognizing that the display position is
 * not the same as the monster position.  Currently the only body part is
 * a worm tail.
 *  
 */
static void
display_monster(x, y, mon, in_sight, worm_tail)
    register xchar x, y;	/* display position */
    register struct monst *mon;	/* monster to display */
    int in_sight;		/* TRUE if the monster is physically seen */
    register xchar worm_tail;	/* mon is actually a worm tail */
{
    register boolean mon_mimic = (mon->m_ap_type != M_AP_NOTHING);
    register int sensed = mon_mimic &&
	(Protection_from_shape_changers || sensemon(mon));

    /*
     * We must do the mimic check first.  If the mimic is mimicing something,
     * and the location is in sight, we have to change the hero's memory
     * so that when the position is out of sight, the hero remembers what
     * the mimic was mimicing.
     */

    if (mon_mimic && in_sight) {
	switch (mon->m_ap_type) {
	    default:
		impossible("display_monster:  bad m_ap_type value [ = %d ]",
							(int) mon->m_ap_type);
	    case M_AP_NOTHING:
		show_glyph(x, y, mon_to_glyph(mon));
		break;

	    case M_AP_FURNITURE: {
		/*
		 * This is a poor man's version of map_background().  I can't
		 * use map_background() because we are overriding what is in
		 * the 'typ' field.  Maybe have map_background()'s parameters
		 * be (x,y,glyph) instead of just (x,y).
		 *
		 * mappearance is currently set to an S_ index value in
		 * makemon.c.
		 */
		register int glyph = cmap_to_glyph(mon->mappearance);
		levl[x][y].glyph = glyph;
		if (!sensed) show_glyph(x,y, glyph);
		break;
	    }

	    case M_AP_OBJECT: {
		struct obj obj;	/* Make a fake object to send	*/
				/* to map_object().		*/
		obj.ox = x;
		obj.oy = y;
		obj.otyp = mon->mappearance;
		obj.corpsenm = PM_TENGU;	/* if mimicing a corpse */
		map_object(&obj,!sensed);
		break;
	    }

	    case M_AP_MONSTER:
		show_glyph(x,y, monnum_to_glyph(what_mon(mon->mappearance)));
		break;
	}
	
    }

    /* If the mimic is unsucessfully mimicing something, display the monster */
    if (!mon_mimic || sensed) {
	if (mon->mtame) {
	    if (worm_tail)
		show_glyph(x,y, petnum_to_glyph(what_mon(PM_LONG_WORM_TAIL)));
	    else    
		show_glyph(x,y, pet_to_glyph(mon));
	} else {
	    if (worm_tail)
		show_glyph(x,y, monnum_to_glyph(what_mon(PM_LONG_WORM_TAIL)));
	    else    
		show_glyph(x,y, mon_to_glyph(mon));
	}
    }
}

/*
 * feel_location()
 *
 * Feel the given location.  This assumes that the hero is blind and that
 * the given position is either the hero's or one of the eight squares
 * adjacent to the hero (except for a boulder push).
 */
void
feel_location(x, y)
    xchar x, y;
{
    struct rm *lev = &(levl[x][y]);
    struct obj *boulder;
    register struct monst *mon;

    /* The hero can't feel non pool locations while under water. */
    if (Underwater && !Is_waterlevel(&u.uz) && ! is_pool(x,y))
	return;

    /* If the hero is not in a corridor, then she will feel the wall as a */
    /* wall.  It doesn't matter if the hero is levitating or not.	  */
    if ((IS_WALL(lev->typ) || lev->typ == SDOOR) &&
						levl[u.ux][u.uy].typ != CORR)
	lev->seen = 1;

    if (Levitation && !Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz)) {
	/*
	 * Levitation Rules.  It is assumed that the hero can feel the state
	 * of the walls around herself and can tell if she is in a corridor,
	 * room, or doorway.  Boulders are felt because they are large enough.
	 * Anything else is unknown because the hero can't reach the ground.
	 * This makes things difficult.
	 *
	 * Check (and display) in order:
	 *
	 *	+ Stone, walls, and closed doors.
	 *	+ Boulders.  [see a boulder before a doorway]
	 *	+ Doors.
	 *	+ Room/water positions
	 *	+ Everything else (hallways!)
	 */
	if (IS_ROCK(lev->typ) || (IS_DOOR(lev->typ) &&
				(lev->doormask & (D_LOCKED | D_CLOSED)))) {
	    map_background(x, y, 1);
	} else if (boulder = sobj_at(BOULDER,x,y)) {
	    map_object(boulder, 1);
	} else if (IS_DOOR(lev->typ)) {
	    map_background(x, y, 1);
	} else if (IS_ROOM(lev->typ) || IS_POOL(lev->typ)) {
	    /*
	     * An open room or water location.  Normally we wouldn't touch
	     * this, but we have to get rid of remembered boulder symbols.
	     * This will only occur in rare occations when the hero goes
	     * blind and doesn't find a boulder where expected (something
	     * came along and picked it up).  We know that there is not a
	     * boulder at this location.  Show fountains, pools, etc.
	     * underneath if already seen.  Otherwise, show the appropriate
	     * floor symbol.
	     *
	     * This isn't quite correct.  If the boulder was on top of some
	     * other objects they should be seen once the boulder is removed.
	     * However, we have no way of knowing that what is there now
	     * was there then.  So we let the hero have a lapse of memory.
	     * We could also just display what is currently on the top of the
	     * object stack (if anything).
	     */
	    if (lev->glyph == objnum_to_glyph(BOULDER)) {
		if (lev->typ != ROOM && lev->seen) {
		    map_background(x, y, 1);
		} else {
		    lev->glyph = lev->waslit ? cmap_to_glyph(S_room) :
					       cmap_to_glyph(S_stone);
		    show_glyph(x,y,lev->glyph);
		}
	    }
	} else {
	    /* We feel it (I think hallways are the only things left). */
	    map_background(x, y, 1);
	    /* Corridors are never felt as lit (unless remembered that way) */
	    /* (lit_corridor only).					    */
	    if (lev->typ == CORR &&
		    lev->glyph == cmap_to_glyph(S_litcorr) && !lev->waslit)
		show_glyph(x, y, lev->glyph = cmap_to_glyph(S_corr));
	}
    } else {
	map_location(x, y, 1);

	if (Punished) {
	    /*
	     * A ball or chain is only felt if it is first on the object
	     * location list.  Otherwise, we need to clear the felt bit ---
	     * something has been dropped on the ball/chain.  If the bit is
	     * not cleared, then when the ball/chain is moved it will drop
	     * the wrong glyph.
	     */
	    if (uchain->ox == x && uchain->oy == y) {
		if (level.objects[x][y] == uchain)
		    u.bc_felt |= BC_CHAIN;
		else
		    u.bc_felt &= ~BC_CHAIN;	/* do not feel the chain */
	    }
	    if (!carried(uball) && uball->ox == x && uball->oy == y) {
		if (level.objects[x][y] == uball)
		    u.bc_felt |= BC_BALL;
		else
		    u.bc_felt &= ~BC_BALL;	/* do not feel the ball */
	    }
	}

	/* Floor spaces are dark if unlit.  Corridors are dark if unlit. */
	if (lev->typ == ROOM &&
		    lev->glyph == cmap_to_glyph(S_room) && !lev->waslit)
	    show_glyph(x,y, lev->glyph = cmap_to_glyph(S_stone));
	else if (lev->typ == CORR &&
		    lev->glyph == cmap_to_glyph(S_litcorr) && !lev->waslit)
	    show_glyph(x,y, lev->glyph = cmap_to_glyph(S_corr));
    }
    /* draw monster on top if we can sense it */
    if ((x != u.ux || y != u.uy) && (mon = m_at(x,y)) && sensemon(mon))
	display_monster(x,y,mon,1,((x != mon->mx)  || (y != mon->my)));
}

/*
 * newsym()
 *
 * Possibly put a new glyph at the given location.
 */
void
newsym(x,y)
    register xchar x,y;
{
    register struct monst *mon;
    register struct rm *lev = &(levl[x][y]);
    register int see_it;
    register xchar worm_tail;

    /* only permit updating the hero when swallowed */
    if (u.uswallow) {
	if (x == u.ux && y == u.uy) display_self();
	return;
    }
    if (Underwater && !Is_waterlevel(&u.uz)) {
	/* don't do anything unless (x,y) is an adjacent underwater position */
	int dx, dy;
	if (!is_pool(x,y)) return;
	dx = x - u.ux;	if (dx < 0) dx = -dx;
	dy = y - u.uy;	if (dy < 0) dy = -dy;
	if (dx > 1 || dy > 1) return;
    }

    /* Can physically see the location. */
    if (cansee(x,y)) {
	lev->waslit = (lev->lit!=0);	/* remember lit condition */

	if (x == u.ux && y == u.uy) {
	    if (canseeself()) {
		map_location(x,y,0);	/* map *under* self */
		display_self();
	    } else
		/* we can see what is there */
		map_location(x,y,1);
	}
	else if ((mon = m_at(x,y)) &&
		 ((see_it = mon_visible(mon)) || sensemon(mon))) {
	    map_location(x,y,0); 	/* map under the monster */
    	    worm_tail = ((x != mon->mx)  || (y != mon->my));
	    display_monster(x,y,mon,see_it,worm_tail);
	}
	else
	    map_location(x,y,1);	/* map the location */
    }

    /* Can't see the location. */
    else {
	if (x == u.ux && y == u.uy) {
	    feel_location(u.ux, u.uy);		/* forces an update */

	    if (canseeself()) display_self();
	}
	else if ((mon = m_at(x,y)) && sensemon(mon) &&
    		 		!((x != mon->mx)  || (y != mon->my))) {
	    /* Monsters are printed every time. */
	    display_monster(x,y,mon,0,0);
	}
	/*
	 * If the location is remembered as being both dark (waslit is false)
	 * and lit (glyph is a lit room or lit corridor) then it was either:
	 *
	 *	(1) A dark location that the hero could see through night
	 *	    vision.
	 *
	 *	(2) Darkened while out of the hero's sight.  This can happen
	 *	    when cursed scroll of light is read.
	 *
	 * In either case, we have to manually correct the hero's memory to
	 * match waslit.  Deciding when to change waslit is non-trivial.
	 *
	 *  Note:  If flags.lit_corridor is set, then corridors act like room
	 *	   squares.  That is, they light up if in night vision range.
	 *	   If flags.lit_corridor is not set, then corridors will
	 *	   remain dark unless lit by a light spell.
	 *
	 * These checks and changes must be here and not in back_to_glyph().
	 * They are dependent on the position being out of sight.
	 */
	else if (!lev->waslit) {
	    if (flags.lit_corridor && lev->glyph == cmap_to_glyph(S_litcorr) &&
							    lev->typ == CORR)
		show_glyph(x, y, lev->glyph = cmap_to_glyph(S_corr));
	    else if (lev->glyph == cmap_to_glyph(S_room) && lev->typ == ROOM)
		show_glyph(x, y, lev->glyph = cmap_to_glyph(S_stone));
	    else
		goto show_mem;
	} else {
show_mem:
	    show_glyph(x, y, lev->glyph);
	}
    }
}


/*
 * shieldeff()
 *
 * Put magic shield pyrotechnics at the given location.  This *could* be
 * pulled into a platform dependent routine for fancier graphics if desired.
 */
void
shieldeff(x,y)
    xchar x,y;
{
    register int i;

    if (cansee(x,y)) {	/* Don't see anything if can't see the location */
	for (i = 0; i < SHIELD_COUNT; i++) {
	    show_glyph(x, y, cmap_to_glyph(shield_static[i]));
	    flush_screen(1);	/* make sure the glyph shows up */
	    delay_output();
	}
	newsym(x,y);		/* restore the old information */
    }
}


/*
 * tmp_at()
 *
 * Temporarily place glyphs on the screen.  Do not call delay_output().  It
 * is up to the caller to decide if it wants to wait [presently, everyone
 * but explode() wants to delay].
 *
 * Call:
 *	(DISP_BEAM,   glyph)	open, initialize glyph
 *	(DISP_FLASH,  glyph)	open, initialize glyph
 *	(DISP_CHANGE, glyph)	change glyph
 *	(DISP_END,    0)	close & clean up (second argument doesn't
 *				matter)
 *	(x, y)			display the glyph at the location
 *
 * DISP_BEAM  - Display the given glyph at each location, but do not erase
 *		any until the close call.
 * DISP_FLASH - Display the given glyph at each location, but erase the
 *		previous location's glyph.
 */
void
tmp_at(x, y)
    int x, y;
{
    static coord saved[COLNO];	/* prev positions, only for DISP_BEAM */
    static int sidx = 0;	/* index of saved previous positions */
    static int sx = -1, sy;	/* previous position, only for DISP_FLASH */
    static int status;		/* either DISP_BEAM or DISP_FLASH */
    static int glyph;		/* glyph to use when printing */

    switch (x) {
	case DISP_BEAM:
	case DISP_FLASH:
	    status = x;
	    glyph  = y;
	    flush_screen(0);	/* flush buffered glyphs */
	    break;

	case DISP_CHANGE:
	    glyph = y;
	    break;

	case DISP_END:
	    if (status == DISP_BEAM) {
		register int i;

		/* Erase (reset) from source to end */
		for (i = 0; i < sidx; i++)
		    newsym(saved[i].x,saved[i].y);
		sidx = 0;
		
	    } else if (sx >= 0) {	/* DISP_FLASH (called at least once) */
		newsym(sx,sy);	/* reset the location */
		sx = -1;	/* reset sx to an illegal pos for next time */
	    }
	    break;

	default:	/* do it */
	    if (!cansee(x,y)) break;

	    if (status == DISP_BEAM) {
		saved[sidx  ].x = x;	/* save pos for later erasing */
		saved[sidx++].y = y;
	    }

	    else {	/* DISP_FLASH */
		if (sx >= 0)		/* not first call */
		    newsym(sx,sy);	/* update the old position */

		sx = x;		/* save previous pos for next call */
		sy = y;
	    }

	    show_glyph(x,y,glyph);	/* show it */
	    flush_screen(0);		/* make sure it shows up */
	    break;
    } /* end case */
}


/*
 * swallowed()
 *
 * The hero is swallowed.  Show a special graphics sequence for this.  This
 * bypasses all of the display routines and messes with buffered screen
 * directly.  This method works because both vision and display check for
 * being swallowed.
 */
void
swallowed(first)
    int first;
{
    static xchar lastx, lasty;	/* last swallowed position */
    int swallower;

    if (first)
	cls();
    else {
	register int x, y;

	/* Clear old location */
	for (y = lasty-1; y <= lasty+1; y++)
	    if(isok(lastx,y)) {
		for (x = lastx-1; x <= lastx+1; x++)
		    show_glyph(x,y,cmap_to_glyph(S_stone));
	    }
    }

    swallower = monsndx(u.ustuck->data);
    /*
     *  Display the hero surrounded by the monster's stomach.
     */
    if(isok(u.ux, u.uy-1)) {
	show_glyph(u.ux-1, u.uy-1, swallow_to_glyph(swallower, S_sw_tl));
	show_glyph(u.ux  , u.uy-1, swallow_to_glyph(swallower, S_sw_tc));
	show_glyph(u.ux+1, u.uy-1, swallow_to_glyph(swallower, S_sw_tr));
    }

    show_glyph(u.ux-1, u.uy  , swallow_to_glyph(swallower, S_sw_ml));
    display_self();
    show_glyph(u.ux+1, u.uy  , swallow_to_glyph(swallower, S_sw_mr));

    if(isok(u.ux, u.uy+1)) {
	show_glyph(u.ux-1, u.uy+1, swallow_to_glyph(swallower, S_sw_bl));
	show_glyph(u.ux  , u.uy+1, swallow_to_glyph(swallower, S_sw_bc));
	show_glyph(u.ux+1, u.uy+1, swallow_to_glyph(swallower, S_sw_br));
    }

    /* Update the swallowed position. */
    lastx = u.ux;
    lasty = u.uy;
}

/*
 * under_water()
 *
 * Similar to swallowed() in operation.  Shows hero when underwater
 * except when in water level.  Special routines exist for that.
 */
void
under_water(mode)
    int mode;
{
    static xchar lastx, lasty;
    static boolean dela;
    register int x, y;

    /* swallowing has a higher precedence than under water */
    if (Is_waterlevel(&u.uz) || u.uswallow) return;

    /* full update */
    if (mode == 1 || dela) {
	cls();
	dela = FALSE;
    }   
    /* delayed full update */
    else if (mode == 2) {
	dela = TRUE;
	return;
    }
    /* limited update */
    else {
	for (y = lasty-1; y <= lasty+1; y++)
	    for (x = lastx-1; x <= lastx+1; x++)
		if (isok(x,y)) 
		    show_glyph(x,y,cmap_to_glyph(S_stone));
    }
    for (x = u.ux-1; x <= u.ux+1; x++)
	for (y = u.uy-1; y <= u.uy+1; y++)
	    if (isok(x,y) && is_pool(x,y)) {
		if (Blind && !(x == u.ux && y == u.uy))
		    show_glyph(x,y,cmap_to_glyph(S_stone));
		else	
		    newsym(x,y);
	    }
    lastx = u.ux;
    lasty = u.uy;
}


/* ========================================================================= */

/*
 * Loop through all of the monsters and update them.  Called when:
 *	+ going blind & telepathic
 *	+ regaining sight & telepathic
 *	+ hallucinating
 *	+ doing a full screen redraw
 *	+ see invisible times out or a ring of see invisible is taken off
 *	+ when a potion of see invisible is quaffed or a ring of see
 *	  invisible is put on
 *	+ gaining telepathy when blind [givit() in eat.c, pleased() in pray.c]
 *	+ losing telepathy while blind [xkilled() in mon.c, attrcurse() in
 *	  sit.c]
 */
void
see_monsters()
{
    register struct monst *mon;
    for (mon = fmon; mon; mon = mon->nmon) {
	newsym(mon->mx,mon->my);
	if (mon->wormno) see_wsegs(mon);
    }
}

/*
 * Block/unblock light depending on what a mimic is mimicing and if it's
 * invisible or not.  Should be called only when the state of See_invisible
 * changes.
 */
void
set_mimic_blocking()
{
    register struct monst *mon;
    for (mon = fmon; mon; mon = mon->nmon)
	if(mon->minvis &&
	   ((mon->m_ap_type == M_AP_FURNITURE &&
	      (mon->mappearance == S_vcdoor || mon->mappearance == S_hcdoor))||
	    (mon->m_ap_type == M_AP_OBJECT && mon->mappearance == BOULDER))) {
	    if(See_invisible)
		block_point(mon->mx, mon->my);
	    else
		unblock_point(mon->mx, mon->my);
	}
}

/*
 * Loop through all of the object *locations* and update them.  Called when
 *	+ hallucinating.
 */
void
see_objects()
{
    register struct obj *obj;
    for(obj = fobj; obj; obj = obj->nobj)
	if (vobj_at(obj->ox,obj->oy) == obj) newsym(obj->ox, obj->oy);
}

/*
 * Put the cursor on the hero.  Flush all accumulated glyphs before doing it.
 */
void
curs_on_u()
{
    flush_screen(1);	/* Flush waiting glyphs & put cursor on hero */
}

int
doredraw()
{
    docrt();
    return 0;
}

void
docrt()
{
    register int x,y;
    register struct rm *lev;

    if (!u.ux) return; /* display isn't ready yet */

    if (u.uswallow) {
	swallowed(1);
	return;
    }
    if (Underwater && !Is_waterlevel(&u.uz)) {
	under_water(1);
	return;
    }

    /* shut down vision */
    vision_recalc(2);

    /*
     * This routine assumes that cls() does the following:
     *      + fills the physical screen with the symbol for rock
     *      + clears the glyph buffer
     */
    cls();

    /* display memory */
    for (x = 1; x < COLNO; x++) {
	lev = &levl[x][0];
	for (y = 0; y < ROWNO; y++, lev++)
	    if (lev->glyph != cmap_to_glyph(S_stone))
		show_glyph(x,y,lev->glyph);
    }

    /* see what is to be seen */
    vision_recalc(0);

    /* overlay with monsters */
    see_monsters();

    flags.botlx = 1;	/* force a redraw of the bottom line */
}


/* ========================================================================= */
/* Glyph Buffering (3rd screen) ============================================ */

typedef struct {
    xchar new;		/* perhaps move this bit into the rm strucure. */
    int   glyph;
} gbuf_entry;

static gbuf_entry gbuf[ROWNO][COLNO];
static char gbuf_start[ROWNO];
static char gbuf_stop[ROWNO];

/*
 * Store the glyph in the 3rd screen for later flushing.
 */
void
show_glyph(x,y,glyph)
    xchar x,y;
    int   glyph;
{
    /*
     * Check for bad positions and glyphs.
     */
    if (x <= 0 || x >= COLNO || y < 0 || y >= ROWNO) {
	const char *text;
	int  offset;

	/* column 0 is invalid, but it's often used as a flag, so ignore it */
	if (x == 0) return;

	/*
	 *  This assumes an ordering of the offsets.  See display.h for
	 *  the definition.
	 */
	if (glyph >= GLYPH_SWALLOW_OFF) {		/* swallow border */
	    text = "swallow border";	offset = glyph - GLYPH_SWALLOW_OFF;
	}else if (glyph >= GLYPH_ZAP_OFF) {		/* zap beam */
	    text = "zap beam";		offset = glyph - GLYPH_ZAP_OFF;
	} else if (glyph >= GLYPH_CMAP_OFF) {		/* cmap */
	    text = "cmap_index";	offset = glyph - GLYPH_CMAP_OFF;
	} else if (glyph >= GLYPH_TRAP_OFF) {		/* trap */
	    text = "trap";		offset = glyph - GLYPH_TRAP_OFF;
	} else if (glyph >= GLYPH_OBJ_OFF) {		/* object */
	    text = "object";		offset = glyph - GLYPH_OBJ_OFF;
	} else if (glyph >= GLYPH_BODY_OFF) {		/* a corpse */
	    text = "corpse";		offset = glyph - GLYPH_BODY_OFF;
	} else {					/* a monster */
	    text = "monster";		offset = glyph;
	}

	impossible("show_glyph:  bad pos %d %d with glyph %d [%s %d].",
						x, y, glyph, text, offset);
	return;
    }

    if (glyph >= MAX_GLYPH) {
	impossible("show_glyph:  bad glyph %d [max %d] at (%d,%d).",
					glyph, MAX_GLYPH, x, y);
	return;
    }

    if (gbuf[y][x].glyph != glyph) {
	gbuf[y][x].glyph = glyph;
	gbuf[y][x].new   = 1;
	if (gbuf_start[y] > x) gbuf_start[y] = x;
	if (gbuf_stop[y]  < x) gbuf_stop[y]  = x;
    }
}


/*
 * Reset the changed glyph borders so that none of the 3rd screen has
 * changed.
 */
#define reset_glyph_bbox()			\
    {						\
	int i;					\
						\
	for (i = 0; i < ROWNO; i++) {		\
	    gbuf_start[i] = COLNO-1;		\
	    gbuf_stop[i]  = 0;			\
	}					\
    }


static gbuf_entry nul_gbuf = { 0, cmap_to_glyph(S_stone) };
/*
 * Turn the 3rd screen into stone.
 */
void
clear_glyph_buffer()
{
    register int x, y;
    register gbuf_entry *gptr;

    for (y = 0; y < ROWNO; y++) {
	gptr = &gbuf[y][0];
	for (x = COLNO; x; x--) {
	    *gptr++ = nul_gbuf;
	}
    }
    reset_glyph_bbox();
}

/*
 * Assumes that the indicated positions are filled with S_stone glyphs.
 */
void
row_refresh(start,stop,y)
    int start,stop,y;
{
    register int x;

    for (x = start; x <= stop; x++)
	if (gbuf[y][x].glyph != cmap_to_glyph(S_stone))
	    print_glyph(WIN_MAP,x,y,gbuf[y][x].glyph);
}

void
cls()
{
    display_nhwindow(WIN_MESSAGE, FALSE); /* flush messages */
    flags.botlx = 1;		/* force update of botl window */
    clear_nhwindow(WIN_MAP);	/* clear physical screen */

    clear_glyph_buffer();	/* this is sort of an extra effort, but OK */
}

/*
 * Synch the third screen with the display.
 */
void
flush_screen(cursor_on_u)
    int cursor_on_u;
{
    /* Prevent infinite loops on errors:
     *	    flush_screen->print_glyph->impossible->pline->flush_screen
     */
    static   boolean flushing = 0;
    register int x,y;

    if (flushing) return;	/* if already flushing then return */
    flushing = 1;

    for (y = 0; y < ROWNO; y++) {
	register gbuf_entry *gptr = &gbuf[y][x = gbuf_start[y]];
	for (; x <= gbuf_stop[y]; gptr++, x++)
	    if (gptr->new) {
		print_glyph(WIN_MAP,x,y,gptr->glyph);
		gptr->new = 0;
	    }
    }

    if (cursor_on_u) curs(WIN_MAP, u.ux,u.uy); /* move cursor to the hero */
    display_nhwindow(WIN_MAP, FALSE);
    reset_glyph_bbox();
    flushing = 0;
    if(flags.botl || flags.botlx) bot();
}

/* ========================================================================= */

/*
 * back_to_glyph()
 *
 * Use the information in the rm structure at the given position to create
 * a glyph of a background.
 *
 * I had to add a field in the rm structure (horizontal) so that we knew
 * if open doors and secret doors were horizontal or vertical.  Previously,
 * the screen symbol had the horizontal/vertical information set at
 * level generation time.
 *
 * I used the 'ladder' field (really doormask) for deciding if stairwells
 * were up or down.  I didn't want to check the upstairs and dnstairs
 * variables.
 */
int
back_to_glyph(x,y)
    xchar x,y;
{
    int idx;
    struct rm *ptr = &(levl[x][y]);

    switch (ptr->typ) {
	case SCORR:
	case STONE:		idx = S_stone;	  break;
	case ROOM:		idx = S_room;	  break;
	case CORR:
	    idx = (ptr->waslit || flags.lit_corridor) ? S_litcorr : S_corr;
	    break;
	case HWALL:	idx = ptr->seen ? S_hwall  : S_stone;   break;
	case VWALL:	idx = ptr->seen ? S_vwall  : S_stone;   break;
	case TLCORNER:	idx = ptr->seen ? S_tlcorn : S_stone;	break;
	case TRCORNER:	idx = ptr->seen ? S_trcorn : S_stone;	break;
	case BLCORNER:	idx = ptr->seen ? S_blcorn : S_stone;	break;
	case BRCORNER:	idx = ptr->seen ? S_brcorn : S_stone;	break;
	case CROSSWALL:	idx = ptr->seen ? S_crwall : S_stone;	break;
	case TUWALL:	idx = ptr->seen ? S_tuwall : S_stone;	break;
	case TDWALL:	idx = ptr->seen ? S_tdwall : S_stone;	break;
	case TLWALL:	idx = ptr->seen ? S_tlwall : S_stone;	break;
	case TRWALL:	idx = ptr->seen ? S_trwall : S_stone;	break;
	case SDOOR:
	    if (ptr->seen)
		idx = (ptr->horizontal) ? S_hwall : S_vwall;
	    else
		idx = S_stone;
	    break;
	case DOOR:
	    if (ptr->doormask) {
		if (ptr->doormask & D_BROKEN)
		    idx = S_ndoor;
		else if (ptr->doormask & D_ISOPEN)
		    idx = (ptr->horizontal) ? S_hodoor : S_vodoor;
		else			/* else is closed */
		    idx = (ptr->horizontal) ? S_hcdoor : S_vcdoor;
	    } else
		idx = S_ndoor;
	    break;
	case POOL:
	case MOAT:		idx = S_pool;	  break;
	case STAIRS:
	    idx = (ptr->ladder & LA_DOWN) ? S_dnstair : S_upstair;
	    break;
	case LADDER:
	    idx = (ptr->ladder & LA_DOWN) ? S_dnladder : S_upladder;
	    break;
	case FOUNTAIN:		idx = S_fountain; break;
	case SINK:		idx = S_sink;     break;
	case ALTAR:		idx = S_altar;    break;
	case THRONE:		idx = S_throne;   break;
	case LAVAPOOL:		idx = S_lava;	  break;
	case ICE:		idx = S_ice;      break;
	case AIR:		idx = S_air;	  break;
	case CLOUD:		idx = S_cloud;	  break;
	case WATER:		idx = S_water;	  break;
	case DBWALL:
	    idx = (ptr->horizontal) ? S_hcdbridge : S_vcdbridge;
	    break;
	case DRAWBRIDGE_UP:
	    switch(ptr->drawbridgemask & DB_UNDER) {
	    case DB_MOAT:  idx = S_pool; break;
	    case DB_LAVA:  idx = S_lava; break;
	    case DB_ICE:   idx = S_ice;  break;
	    case DB_FLOOR: idx = S_room; break;
	    default:
		impossible("Strange db-under: %d",
			   ptr->drawbridgemask & DB_UNDER);
		idx = S_room; /* something is better than nothing */
		break;
	    }
	    break;
	case DRAWBRIDGE_DOWN:
	    idx = (ptr->horizontal) ? S_hodbridge : S_vodbridge;
	    break;
	default:
	    impossible("back_to_glyph:  unknown level type [ = %d ]",ptr->typ);
	    idx = S_room;
	    break;
    }

    return cmap_to_glyph(idx);
}


/*
 * swallow_to_glyph()
 *
 * Convert a monster number and a swallow location into the correct glyph.
 * If you don't want a patchwork monster while hallucinating, decide on
 * a random monster in swallowed() and don't use what_mon() here.
 */
static int
swallow_to_glyph(mnum, loc)
    int mnum;
    int loc;
{
    if (loc < S_sw_tl || S_sw_br < loc) {
	impossible("swallow_to_glyph: bad swallow location");
	loc = S_sw_br;
    }
    return ((int) (what_mon(mnum)<<3) | (loc - S_sw_tl)) + GLYPH_SWALLOW_OFF;
}



/*
 * zapdir_to_glyph()
 *
 * Change the given zap direction and beam type into a glyph.  Each beam
 * type has four glyphs, one for each of the symbols below.  The order of
 * the zap symbols [0-3] as defined in rm.h are:
 *
 *	|  S_vbeam	( 0, 1) or ( 0,-1)
 *	-  S_hbeam	( 1, 0) or (-1,	0)
 *	\  S_lslant	( 1, 1) or (-1,-1)
 *	/  S_rslant	(-1, 1) or ( 1,-1)
 */
int
zapdir_to_glyph(dx, dy, beam_type)
    register int dx, dy;
    int beam_type;
{
    if (beam_type >= NUM_ZAP) {
	impossible("zapdir_to_glyph:  illegal beam type");
	beam_type = 0;
    }
    dx = (dx == dy) ? 2 : (dx && dy) ? 3 : dx ? 1 : 0;

    return ((int) ((beam_type << 2) | dx)) + GLYPH_ZAP_OFF;
}


/*
 * Utility routine for dowhatis() used to find out the glyph displayed at
 * the location.  This isn't necessarily the same as the glyph in the levl
 * structure, so we must check the "third screen".
 */
int
glyph_at(x, y)
    xchar x,y;
{
    if(x < 0 || y < 0 || x >= COLNO || y >= ROWNO)
	return cmap_to_glyph(S_room);			/* XXX */
    return gbuf[y][x].glyph;
}

/*display.c*/
