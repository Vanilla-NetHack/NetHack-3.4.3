/*	SCCS Id: @(#)display.h	3.1	92/07/11		  */
/* Copyright (c) Dean Luick, with acknowledgements to Kevin Darcy */
/* and Dave Cohrs, 1990.					  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DISPLAY_H
#define DISPLAY_H

#ifndef VISION_H
#include "vision.h"
#endif

#ifndef MONDATA_H
#include "mondata.h"	/* for mindless() */
#endif

#ifndef INVISIBLE_OBJECTS
#define vobj_at(x,y) (level.objects[x][y])
#endif

/*
 * sensemon()
 *  
 * Returns true if the hero can sense the given monster.  This includes
 * monsters that are hiding or mimicing other monsters.
 */
#define sensemon(mon) (		/* The hero can always sense a monster IF:  */\
    (!mindless(mon->data)) &&	/* 1. the monster has a brain to sense AND  */\
      ((Blind && Telepat) ||	/* 2a. hero is blind and telepathic OR      */\
    				/* 2b. hero is wearing a telepathy inducing */\
				/*	 object and in range		    */\
      ((HTelepat & (WORN_HELMET|WORN_AMUL|W_ART)) &&			      \
	(distu(mon->mx, mon->my) <= (BOLT_LIM * BOLT_LIM))))		      \
)


/*
 * mon_visible()
 *  
 * Returns true if the hero can see the monster.  It is assumed that the
 * hero can physically see the location of the monster.  The function
 * vobj_at() returns a pointer to an object that the hero can see there.
 */
#define mon_visible(mon) (		/* The hero can see the monster	    */\
					/* IF the monster		    */\
    (!mon->minvis || See_invisible) &&	/* 1. is not invisible AND	    */\
    (!mon->mundetected)			/* 2. not an undetected hider	    */\
)


/*
 * canseemon()
 *  
 * This is the globally used canseemon().  It is not called within the display
 * routines.
 */
#define canseemon(mon) (cansee(mon->mx, mon->my) && mon_visible(mon))


/*
 * canspotmon(mon)
 *
 * This is sensemon() or mon_visible() except that hiding under objects
 * is considered irrelevant for this special case.
 */
#define canspotmon(mon)	\
	(mon && (Blind ? sensemon(mon) : (!mon->minvis || See_invisible)))

/*
 * is_safepet(mon)
 *
 * A special case check used in attack() and domove().  Placing the
 * definition here is convenient.
 */
#define is_safepet(mon)	\
	(mon && mon->mtame && canspotmon(mon) && flags.safe_dog \
		&& !Confusion && !Hallucination && !Stunned)


/*
 * canseeself()
 *  
 * This returns true if the hero can see her/himself.
 *
 * The u.uswallow check assumes that you can see yourself even if you are
 * invisible.  If not, then we don't need the check.
 */
#ifdef POLYSELF
#define canseeself()	(Blind || u.uswallow || (!Invisible && !u.uundetected))
#else
#define canseeself()	(Blind || u.uswallow || !Invisible)
#endif


/*
 * random_monster()
 * random_object()
 *  
 * Respectively return a random monster or object number.
 */
#define random_monster() rn2(NUMMONS)
#define random_object()  (rn2(NROFOBJECTS) + 1)


/*
 * what_obj()
 * what_mon()
 *  
 * If hallucinating, choose a random object/monster, otherwise, use the one
 * given.
 */
#define what_obj(obj)	(Hallucination ? random_object()  : obj)
#define what_mon(mon)	(Hallucination ? random_monster() : mon)


/*
 * covers_objects()
 * covers_traps()
 *  
 * These routines are true if what is really at the given location will
 * "cover" any objects or traps that might be there.
 */
#define covers_objects(xx,yy)						      \
    ((is_pool(xx,yy) && !Underwater) || (levl[xx][yy].typ == LAVAPOOL))

#define covers_traps(xx,yy)	covers_objects(xx,yy)


/*
 * tmp_at() control calls.
 */
#define DISP_BEAM   (-1)  /* Keep all glyphs showing & clean up at end. */
#define DISP_FLASH  (-2)  /* Clean up each glyph before displaying new one. */
#define DISP_CHANGE (-3)  /* Change glyph. */
#define DISP_END    (-4)  /* Clean up. */


/* Total number of cmap indices in the sheild_static[] array. */
#define SHIELD_COUNT 21


/*
 *  display_self()
 *  
 *  Display the hero.  This has degenerated down to this.  Perhaps there is
 *  more needed here, but I can't think of any cases.
 */
#ifdef POLYSELF
#define display_self()						\
    show_glyph(u.ux, u.uy,					\
	u.usym == 0 ? objnum_to_glyph(GOLD_PIECE) :		\
	monnum_to_glyph((u.umonnum < 0 ? u.umonster : u.umonnum)))
#else
#define display_self()						\
    show_glyph(u.ux, u.uy,					\
	u.usym == 0 ? objnum_to_glyph(GOLD_PIECE) :		\
	monnum_to_glyph(u.umonster))
#endif


/*
 * A glyph is an abstraction that represents a _unique_ monster, object,
 * dungeon part, or effect.  The uniqueness is important.  For example,
 * It is not enough to have four (one for each "direction") zap beam glyphs,
 * we need a set of four for each beam type.  Why go to so much trouble?
 * Because it is possible that any given window dependent display driver
 * [print_glyph()] can produce something different for each type of glyph.
 * That is, a beam of cold and a beam of fire would not only be different
 * colors, but would also be represented by different symbols.
 *
 * Glyphs are grouped for easy accessibility:
 *
 * monster	Represents all the wild (not tame) monsters.  Count: NUMMONS.
 *
 * pet		Represents all of the tame monsters.  Count: NUMMONS
 *
 * corpse	One for each monster.  Count: NUMMONS
 *
 * object	One for each object.  Count: NROFOBJECTS+1 (we need the +1
 *		because NROFOBJECTS does not include the illegal object)
 *
 * trap		One for each trap type.  Count: TRAPNUM
 *
 * cmap		One for each entry in the character map.  The character map
 *		is the dungeon features and other miscellaneous things.
 *		Count: MAXPCHARS
 *
 * zap beam	A set of four (there are four directions) for each beam type.
 *		The beam type is shifted over 2 positions and the direction
 *		is stored in the lower 2 bits.  Count: NUM_ZAP << 2
 *
 * swallow	A set of eight for each monster.  The eight positions rep-
 *		resent those surrounding the hero.  The monster number is
 *		shifted over 3 positions and the swallow position is stored
 *		in the lower three bits.  Count: NUMMONS << 3
 *
 * The following are offsets used to convert to and from a glyph.
 */
#define NUM_ZAP	8	/* number of zap beam types */

#define GLYPH_MON_OFF  	  0
#define GLYPH_PET_OFF 	  (NUMMONS        + GLYPH_MON_OFF)
#define GLYPH_BODY_OFF 	  (NUMMONS        + GLYPH_PET_OFF)
#define GLYPH_OBJ_OFF 	  (NUMMONS        + GLYPH_BODY_OFF)
#define GLYPH_TRAP_OFF	  (NROFOBJECTS+1  + GLYPH_OBJ_OFF)
#define GLYPH_CMAP_OFF	  (TRAPNUM        + GLYPH_TRAP_OFF)
#define GLYPH_ZAP_OFF	  (MAXPCHARS      + GLYPH_CMAP_OFF)
#define GLYPH_SWALLOW_OFF ((NUM_ZAP << 2) + GLYPH_ZAP_OFF)

#define MAX_GLYPH 	  ((NUMMONS << 3) + GLYPH_SWALLOW_OFF)


#define mon_to_glyph(mon) ((int) what_mon(monsndx((mon)->data))+GLYPH_MON_OFF)
#define pet_to_glyph(mon) ((int) what_mon(monsndx((mon)->data))+GLYPH_PET_OFF)

/* This has the unfortunate side effect of needing a global variable	*/
/* to store a result. 'otg_temp' is defined and declared in decl.{ch}.	*/
#define obj_to_glyph(obj)						      \
    (Hallucination ?							      \
	((otg_temp = random_object()) == CORPSE ?			      \
	    random_monster() + GLYPH_BODY_OFF :				      \
	    otg_temp + GLYPH_OBJ_OFF)	:				      \
	((obj)->otyp == CORPSE ?					      \
	    (int) (obj)->corpsenm + GLYPH_BODY_OFF :			      \
	    (int) (obj)->otyp + GLYPH_OBJ_OFF))

#define trap_to_glyph(trap)	((int) (trap)->ttyp + GLYPH_TRAP_OFF)
#define cmap_to_glyph(cmap_idx)	((int) (cmap_idx)   + GLYPH_CMAP_OFF)

/* Not affected by hallucination.  Gives a generic body for CORPSE */
#define objnum_to_glyph(onum)	((int) (onum) + GLYPH_OBJ_OFF)
#define monnum_to_glyph(mnum)	((int) (mnum) + GLYPH_MON_OFF)
#define petnum_to_glyph(mnum)	((int) (mnum) + GLYPH_PET_OFF)


/*
 * Change the given glyph into it's given type.  Note:
 *	1) Pets are animals and are converted to the proper monster number.
 *	2) Bodies are all mapped into the generic CORPSE object
 *	3) glyph_to_swallow() does not return a showsyms[] index, but an
 *	   offset from the first swallow symbol.
 *	4) These functions assume that the glyph type has already been
 *	   determined.  That is, you have checked it with a glyph_is_XXXX()
 *	   call.
 */
#define glyph_to_mon(glyph)	((int) ((glyph) < GLYPH_PET_OFF ?	      \
				glyph - GLYPH_MON_OFF : glyph - GLYPH_PET_OFF))
#define glyph_to_obj(glyph)	((int) ((glyph) < GLYPH_OBJ_OFF ?	      \
				CORPSE : (glyph) - GLYPH_OBJ_OFF))
#define glyph_to_trap(glyph)	((int) (glyph) - GLYPH_TRAP_OFF)
#define glyph_to_cmap(glyph)	((int) (glyph) - GLYPH_CMAP_OFF)
#define glyph_to_swallow(glyph) (((glyph) - GLYPH_SWALLOW_OFF) & 0x7)

/*
 * Return true if the given glyph is what we want.  Note that bodies are
 * considered objects.
 */
#define glyph_is_monster(glyph)						      \
    ((glyph) >= GLYPH_MON_OFF && (glyph) < GLYPH_BODY_OFF)
#define glyph_is_object(glyph)						      \
    ((glyph) >= GLYPH_BODY_OFF && (glyph) < GLYPH_TRAP_OFF)
#define glyph_is_trap(glyph)						      \
    ((glyph) >= GLYPH_TRAP_OFF && (glyph) < GLYPH_CMAP_OFF)
#define glyph_is_cmap(glyph)						      \
    ((glyph) >= GLYPH_CMAP_OFF && (glyph) < GLYPH_ZAP_OFF)
#define glyph_is_swallow(glyph) \
    ((glyph) >= GLYPH_SWALLOW_OFF && (glyph) < MAX_GLYPH)

#endif /* DISPLAY_H */
