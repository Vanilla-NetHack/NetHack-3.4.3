/*	SCCS Id: @(#)rm.h	3.0	88/10/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef RM_H
#define RM_H

/*
 * The dungeon presentation graphics code and data structures were rewritten
 * and generalized for NetHack's release 2 by Eric S. Raymond (eric@snark)
 * building on Don G. Kneller's MS-DOS implementation. See options.c for
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
#define HWALL		1
#define VWALL		2
#define TLCORNER	3
#define TRCORNER	4
#define BLCORNER	5
#define BRCORNER	6
#define CROSSWALL	7	/* For pretty mazes and special levels */
#define TUWALL		8
#define TDWALL		9
#define TLWALL		10
#define TRWALL		11
#define SDOOR		12
#define SCORR		13
#define POOL		14
#define MOAT		15	/* pool that doesn't boil, adjust messages */
#define DRAWBRIDGE_UP	16
#define DOOR		17
#define CORR		18
#define ROOM		19
#define STAIRS		20
#define LADDER		21
#define FOUNTAIN	22
#define THRONE		23
#define SINK		24
#define ALTAR		25
#define DRAWBRIDGE_DOWN	26

/*
 * Avoid using the level types in inequalities:
 * these types are subject to change.
 * Instead, use one of the macros below.
 */
#ifndef STUPID_CPP	/* otherwise these macros are functions in prisym.c */
#define IS_WALL(typ)	((typ) && (typ) <= TRWALL)
#define IS_STWALL(typ)	((typ) <= TRWALL)	/* STONE <= (typ) <= TRWALL */
#define IS_ROCK(typ)	((typ) < POOL)		/* absolutely nonaccessible */
#define IS_DOOR(typ)	((typ) == DOOR)
#define IS_FLOOR(typ)	((typ) == ROOM)
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
#endif

/*
 * The level-map symbols may be compiled in or defined at initialization time
 */

/* screen symbols for using character graphics. */
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
#define S_vbeam		12
#define S_hbeam		13
#define S_lslant	14
#define S_rslant	15
#define S_ndoor		16
#define S_vodoor	17
#define S_hodoor	18
#define S_cdoor		19
#define S_room		20
#define S_corr		21
#define S_upstair	22
#define S_dnstair	23
#define S_trap		24
#define S_web		25
#define S_pool		26
#define S_fountain	27
#define S_sink		28
#define S_throne	29
#define S_altar		30
#define S_upladder	31
#define S_dnladder	32
#define S_dbvwall	33
#define S_dbhwall	34

#define MAXPCHARS	35	/* maximum number of mapped characters */

typedef uchar symbol_array[MAXPCHARS];

extern symbol_array showsyms;
extern char *explainsyms[MAXPCHARS];  /* tells what the characters are */
#ifdef REINCARNATION
extern symbol_array savesyms;
#endif
extern symbol_array defsyms;

#define STONE_SYM	showsyms[S_stone]
#define VWALL_SYM	showsyms[S_vwall]
#define HWALL_SYM	showsyms[S_hwall]
#define TLCORN_SYM	showsyms[S_tlcorn]
#define TRCORN_SYM	showsyms[S_trcorn]
#define BLCORN_SYM	showsyms[S_blcorn]
#define BRCORN_SYM	showsyms[S_brcorn]
#define CRWALL_SYM	showsyms[S_crwall]
#define TUWALL_SYM	showsyms[S_tuwall]
#define TDWALL_SYM	showsyms[S_tdwall]
#define TLWALL_SYM	showsyms[S_tlwall]
#define TRWALL_SYM	showsyms[S_trwall]
#define VBEAM_SYM	showsyms[S_vbeam]
#define HBEAM_SYM	showsyms[S_hbeam]
#define LSLANT_SYM	showsyms[S_lslant]
#define RSLANT_SYM	showsyms[S_rslant]
#define NO_DOOR_SYM	showsyms[S_ndoor]
#define H_OPEN_DOOR_SYM	showsyms[S_hodoor]
#define V_OPEN_DOOR_SYM	showsyms[S_vodoor]
#define CLOSED_DOOR_SYM	showsyms[S_cdoor]
#define ROOM_SYM	showsyms[S_room]
#define	CORR_SYM	showsyms[S_corr]
#define UP_SYM		showsyms[S_upstair]
#define DN_SYM		showsyms[S_dnstair]
#define TRAP_SYM	showsyms[S_trap]
#define WEB_SYM		showsyms[S_web]
#define	POOL_SYM	showsyms[S_pool]
#define FOUNTAIN_SYM	showsyms[S_fountain]
#define SINK_SYM	showsyms[S_sink]
#define THRONE_SYM	showsyms[S_throne]
#define ALTAR_SYM	showsyms[S_altar]
#define UPLADDER_SYM	showsyms[S_upladder]
#define DNLADDER_SYM	showsyms[S_dnladder]
#define DB_VWALL_SYM	showsyms[S_dbvwall]
#define DB_HWALL_SYM	showsyms[S_dbhwall]

#define	ERRCHAR	']'

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
#define A_CHAOS		0
#define A_NEUTRAL	1
#define A_LAW		2

/*
 * Some altars are considered as shrines, so we need a flag.
 */
#define A_SHRINE	4

/*
 * Thrones should only be looted once.
 */
#define T_LOOTED	1

/*
 * The four directions for a DrawBridge.
 */
#define DB_NORTH	0
#define DB_SOUTH	1
#define DB_EAST 	2
#define DB_WEST 	4
#define DB_DIR		7	/* mask for direction */

/*
 * What's under a drawbridge.
 */
#define DB_MOAT		0
#define DB_FLOOR	8
#define DB_ICE		16
#define DB_UNDER	24	/* mask for underneath */

/* 
 * Some walls may be non diggable.
 */
#define W_DIGGABLE	0
#define W_NONDIGGABLE	1
#define W_GATEWAY	16	/* is a drawbridge wall */

/*
 * Ladders (in Vlad's tower) may be up or down.
 */
#define LA_UP		1
#define LA_DOWN 	2

/*
 * Room areas may be iced pools,
 */
#define ICED_POOL	8
#define ICED_MOAT	16


/*
 * at() display character types, in order of precedence.
 */
#ifndef MAXCOLORS
#define MAXCOLORS	1
#endif
 
#define AT_APP		(uchar)0
/* 1-MAXCOLORS are specific overrides, see color.h */
/* non-specific */
#define AT_ZAP		(uchar)(MAXCOLORS+1)
#define AT_MON		(uchar)(MAXCOLORS+2)
#define AT_U		AT_MON
#define AT_OBJ		(uchar)(MAXCOLORS+3)
#define AT_GLD		AT_OBJ
#define AT_MAP		(uchar)(MAXCOLORS+4)

/*
 * The structure describing a coordinate position.
 * Before adding fields, remember that this will significantly affect
 * the size of temporary files and save files.
 */
struct rm {
	uchar scrsym;
	Bitfield(typ,5);
	Bitfield(new,1);
	Bitfield(seen,1);
	Bitfield(lit,1);
	Bitfield(doormask,5);
	Bitfield(gmask,1);
};

#define altarmask	doormask
#define diggable	doormask
#define ladder		doormask
#define drawbridgemask	doormask
#define looted		doormask
#define icedpool	doormask

#ifdef MACOS
typedef struct
{
    struct rm		**locations;
    struct obj		***objects;
    struct monst	***monsters;
    struct obj		*objlist;
    struct monst	*monlist;
}
dlevel_t;
#else
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
    struct monst	*monlist;
}
dlevel_t;
#endif

extern dlevel_t	level;	/* structure describing the current level */

/*
 * Macros for compatibility with old code. Someday these will go away.
 */
#define levl		level.locations
#define fobj		level.objlist
#define fmon		level.monlist

#ifndef STUPID_CPP	/* otherwise these macros are functions */
#define OBJ_AT(x, y)	(level.objects[x][y] != (struct obj *)0)
/*
 * Macros for encapsulation of level.monsters references.
 */
#define MON_AT(x, y)	(level.monsters[x][y] != (struct monst *)0)
#define place_monster(m, x, y)	m->mx=x,m->my=y,level.monsters[m->mx][m->my]=m
#define place_worm_seg(m, x, y) level.monsters[x][y] = m
#define remove_monster(x, y)	level.monsters[x][y] = (struct monst *)0
#define m_at(x, y)		level.monsters[x][y]
#endif	/* STUPID_CPP */

#if defined(DGK) && !defined(OLD_TOS)
#define ACTIVE	1
#define SWAPPED	2

struct finfo {
	int	where;
	long	time;
	long	size;
};
extern struct finfo fileinfo[];
#define ZFINFO	{ 0, 0L, 0L }
#endif

#endif /* RM_H /**/
