/*	SCCS Id: @(#)rm.h	3.1	93/02/21		  */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef RM_H
#define RM_H

/*
 * The dungeon presentation graphics code and data structures were rewritten
 * and generalized for NetHack's release 2 by Eric S. Raymond (eric@snark)
 * building on Don G. Kneller's MS-DOS implementation.  See drawing.c for
 * the code that permits the user to set the contents of the symbol structure.
 *
 * The door representation was changed by Ari Huttunen(ahuttune@niksula.hut.fi)
 */

/*
 * TLCORNER	TDWALL		TRCORNER
 * +- 		-+- 		-+
 * |  		 |  	 	 |
 *
 * TRWALL	CROSSWALL	TLWALL		HWALL
 * |  		 |  		 |
 * +- 		-+- 		-+		---
 * |  		 |  		 |
 *
 * BLCORNER	TUWALL		BRCORNER	VWALL
 * |  		 |  		 |		|
 * +- 		-+- 		-+		|
 */

/* Level location types */
#define STONE		0
#define VWALL		1
#define HWALL		2
#define TLCORNER	3
#define TRCORNER	4
#define BLCORNER	5
#define BRCORNER	6
#define CROSSWALL	7	/* For pretty mazes and special levels */
#define TUWALL		8
#define TDWALL		9
#define TLWALL		10
#define TRWALL		11
#define DBWALL		12
#define SDOOR		13
#define SCORR		14
#define POOL		15
#define MOAT		16	/* pool that doesn't boil, adjust messages */
#define WATER		17
#define DRAWBRIDGE_UP	18
#define LAVAPOOL	19
#define DOOR		20
#define CORR		21
#define ROOM		22
#define STAIRS		23
#define LADDER		24
#define FOUNTAIN	25
#define THRONE		26
#define SINK		27
#define ALTAR		28
#define ICE		29
#define DRAWBRIDGE_DOWN	30
#define AIR		31
#define CLOUD		32

#define INVALID_TYPE	127

/*
 * Avoid using the level types in inequalities:
 * these types are subject to change.
 * Instead, use one of the macros below.
 */
#define IS_WALL(typ)	((typ) && (typ) <= DBWALL)
#define IS_STWALL(typ)	((typ) <= DBWALL)	/* STONE <= (typ) <= DBWALL */
#define IS_ROCK(typ)	((typ) < POOL)		/* absolutely nonaccessible */
#define IS_DOOR(typ)	((typ) == DOOR)
#define ACCESSIBLE(typ)	((typ) >= DOOR)		/* good position */
#define IS_ROOM(typ)	((typ) >= ROOM)		/* ROOM, STAIRS, furniture.. */
#define ZAP_POS(typ)	((typ) >= POOL)
#define SPACE_POS(typ)	((typ) > DOOR)
#define IS_POOL(typ)	((typ) >= POOL && (typ) <= DRAWBRIDGE_UP)
#define IS_THRONE(typ)	((typ) == THRONE)
#define IS_FOUNTAIN(typ) ((typ) == FOUNTAIN)
#define IS_SINK(typ)	((typ) == SINK)
#define IS_ALTAR(typ)	((typ) == ALTAR)
#define IS_DRAWBRIDGE(typ) ((typ) == DRAWBRIDGE_UP || (typ) == DRAWBRIDGE_DOWN)
#define IS_FURNITURE(typ) ((typ) >= STAIRS && (typ) <= ALTAR)
#define IS_AIR(typ)	((typ) == AIR || (typ) == CLOUD)
#define IS_SOFT(typ)	((typ) == AIR || (typ) == CLOUD || IS_POOL(typ))

/*
 * The screen symbols may be the default or defined at game startup time.
 * See drawing.c for defaults.
 * Note: {ibm|dec}_graphics[] arrays (also in drawing.c) must be kept in synch.
 */
#define S_stone		0
#define S_vwall		1
#define S_hwall		2
#define S_tlcorn	3
#define S_trcorn	4
#define S_blcorn	5
#define S_brcorn	6
#define S_crwall	7
#define S_tuwall	8
#define S_tdwall	9
#define S_tlwall	10
#define S_trwall	11
#define S_ndoor		12
#define S_vodoor	13
#define S_hodoor	14
#define S_vcdoor	15	/* closed door, vertical wall */
#define S_hcdoor	16	/* closed door, horizontal wall */
#define S_room		17
#define S_corr		18
#define S_litcorr	19
#define S_upstair	20
#define S_dnstair	21
#define S_upladder	22
#define S_dnladder	23
#define S_trap		24
#define S_web		25
#define S_altar		26
#define S_throne	27
#define S_sink		28
#define S_fountain	29
#define S_pool		30
#define S_ice		31
#define S_lava		32
#define S_vodbridge	33
#define S_hodbridge	34
#define S_vcdbridge	35	/* closed drawbridge, vertical wall */
#define S_hcdbridge	36	/* closed drawbridge, horizontal wall */
#define S_air		37
#define S_cloud		38
#define S_water		39
#define S_vbeam		40	/* The 4 zap beam symbols.  Do NOT separate. */
#define S_hbeam		41	/* To change order or add, see function     */
#define S_lslant	42	/* zapdir_to_glyph() in display.c.	    */
#define S_rslant	43
#define S_digbeam	44	/* dig beam symbol */
#define S_flashbeam	45	/* camera flash symbol */
#define S_boomleft	46	/* thrown boomerang, open left, e.g ')'    */
#define S_boomright	47	/* thrown boomerand, open right, e.g. '('  */
#define S_ss1		48	/* 4 magic shield glyphs */
#define S_ss2		49
#define S_ss3		50
#define S_ss4		51

/* The 8 swallow symbols.  Do NOT separate.  To change order or add, see */
/* the function swallow_to_glyph() in display.c.			 */
#define S_sw_tl		52	/* swallow top left [1]			*/
#define S_sw_tc		53	/* swallow top center [2]	Order:	*/
#define S_sw_tr		54	/* swallow top right [3]		*/
#define S_sw_ml		55	/* swallow middle left [4]	1 2 3	*/
#define S_sw_mr		56	/* swallow middle right [6]	4 5 6	*/
#define S_sw_bl		57	/* swallow bottom left [7]	7 8 9	*/
#define S_sw_bc		58	/* swallow bottom center [8]		*/
#define S_sw_br		59	/* swallow bottom right [9]		*/

#define S_explode1	60	/* explosion top left			*/
#define S_explode2	61	/* explosion top center			*/
#define S_explode3	62	/* explosion top right		 Ex.	*/
#define S_explode4	63	/* explosion middle left		*/
#define S_explode5	64	/* explosion middle center	 /-\	*/
#define S_explode6	65	/* explosion middle right	 |@|	*/
#define S_explode7	66	/* explosion bottom left	 \-/	*/
#define S_explode8	67	/* explosion bottom center		*/
#define S_explode9	68	/* explosion bottom right		*/

#define MAXPCHARS	69	/* maximum number of mapped characters */

struct symdef {
    uchar sym;
    const char  *explanation;
#ifdef TEXTCOLOR
    uchar color;
#endif
};

extern const struct symdef defsyms[MAXPCHARS];	/* defaults */
extern uchar showsyms[MAXPCHARS];

/*
 * Graphics sets for display symbols
 */
#define ASCII_GRAPHICS	0	/* regular characters: '-', '+', &c */
#define IBM_GRAPHICS	1	/* PC graphic characters */
#define DEC_GRAPHICS	2	/* VT100 line drawing characters */
#define MAC_GRAPHICS	3	/* Macintosh drawing characters */

/*
 * The 5 possible states of doors
 */

#define D_NODOOR	0
#define D_BROKEN	1
#define D_ISOPEN	2
#define D_CLOSED	4
#define D_LOCKED	8
#define D_TRAPPED	16

/*
 * The 3 possible alignments for altars
 */
#ifndef ALIGN_H
#include "align.h"		/* defines the "AM_" values */
#endif

/*
 * Some altars are considered as shrines, so we need a flag.
 */
#define AM_SHRINE	8

/*
 * Thrones should only be looted once.
 */
#define T_LOOTED	1

/*
 * Fountains have limits, and special warnings.
 */
#define F_LOOTED	1
#define F_WARNED	2

/*
 * Doors are even worse :-) The special warning has a side effect
 * of instantly trapping the door, and if it was defined as trapped,
 * the guards consider that you have already been warned!
 */
#define D_WARNED	16

/*
 * Sinks have 3 different types of loot that shouldn't be abused
 */
#define S_LPUDDING	1
#define S_LDWASHER	2
#define S_LRING		4

/*
 * The four directions for a DrawBridge.
 */
#define DB_NORTH	0
#define DB_SOUTH	1
#define DB_EAST		2
#define DB_WEST		3
#define DB_DIR		3	/* mask for direction */

/*
 * What's under a drawbridge.
 */
#define DB_MOAT		0
#define DB_LAVA		4
#define DB_ICE		8
#define DB_FLOOR	16
#define DB_UNDER	28	/* mask for underneath */

/*
 * Some walls may be non diggable.
 */
#define W_DIGGABLE	0
#define W_NONDIGGABLE	1
#define W_REPAIRED	2
#define W_NONPASSWALL	4

/*
 * Ladders (in Vlad's tower) may be up or down.
 */
#define LA_UP		1
#define LA_DOWN		2

/*
 * Room areas may be iced pools
 */
#define ICED_POOL	8
#define ICED_MOAT	16

/*
 * The structure describing a coordinate position.
 * Before adding fields, remember that this will significantly affect
 * the size of temporary files and save files.
 */
struct rm {
	int glyph;		/* what the hero thinks is there */
	schar typ;		/* what is really there */
	Bitfield(seen,1);	/* speed hack for room walls on corridors */
	Bitfield(lit,1);	/* speed hack for lit rooms */
	Bitfield(flags,5);	/* extra information for typ */
	Bitfield(horizontal,1);	/* wall/door/etc is horiz. (more typ info) */
	Bitfield(waslit,1);	/* remember if a location was lit */
	Bitfield(roomno,6);	/* room # for special rooms */
	Bitfield(edge,1);	/* marks boundaries for special rooms*/
};

#define doormask	flags
#define altarmask	flags
#define diggable	flags
#define ladder		flags
#define drawbridgemask	flags
#define looted		flags
#define icedpool	flags

#define blessedftn      horizontal  /* a fountain that grants attribs */

struct damage {
	struct damage *next;
	long when, cost;
	coord place;
	schar typ;
};

struct levelflags {
	uchar	nfountains;	/* Number of fountains on level */
	uchar	nsinks;		/* Number of sinks on the level */
	/* Several flags that give hints about what's on the level */
	Bitfield(has_shop, 1);
	Bitfield(has_vault, 1);
	Bitfield(has_zoo, 1);
	Bitfield(has_court, 1);
	Bitfield(has_morgue, 1);
	Bitfield(has_beehive, 1);
#ifdef ARMY
	Bitfield(has_barracks, 1);
#endif
	Bitfield(has_temple, 1);
	Bitfield(has_swamp, 1);
	Bitfield(noteleport,1);
	Bitfield(hardfloor,1);
	Bitfield(nommap,1);
	Bitfield(hero_memory,1);	/* hero has memory */
	Bitfield(shortsighted,1);	/* monsters are shortsighted */
	Bitfield(graveyard,1);		/* has_morgue, but remains set */
	Bitfield(is_maze_lev,1);
	Bitfield(is_cavernous_lev,1);
};

typedef struct
{
    struct rm		locations[COLNO][ROWNO];
#ifndef MICROPORT_BUG
    struct obj		*objects[COLNO][ROWNO];
    struct monst	*monsters[COLNO][ROWNO];
#else
    struct obj		*objects[1][ROWNO];
    char		*yuk1[COLNO-1][ROWNO];
    struct monst	*monsters[1][ROWNO];
    char		*yuk2[COLNO-1][ROWNO];
#endif
    struct obj		*objlist;
    struct obj		*buriedobjlist;
    struct monst	*monlist;
    struct damage	*damagelist;
    struct levelflags	flags;
}
dlevel_t;

extern dlevel_t	level;	/* structure describing the current level */

/*
 * Macros for compatibility with old code. Someday these will go away.
 */
#define levl		level.locations
#define fobj		level.objlist
#define fmon		level.monlist

#define OBJ_AT(x,y)	(level.objects[x][y] != (struct obj *)0)
/*
 * Macros for encapsulation of level.monsters references.
 */
#define MON_AT(x,y)	(level.monsters[x][y] != (struct monst *)0)
#define place_monster(m,x,y)	((m)->mx=(x),(m)->my=(y),\
				 level.monsters[(m)->mx][(m)->my]=(m))
#define place_worm_seg(m,x,y)	level.monsters[x][y] = m
#define remove_monster(x,y)	level.monsters[x][y] = (struct monst *)0
#define m_at(x,y)		level.monsters[x][y]

#endif /* RM_H */
