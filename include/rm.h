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
#define IS_WALL(typ)	((typ) && (typ) <= TRWALL)
#define IS_STWALL(typ)	((typ) <= TRWALL)	/* STONE <= (typ) <= TRWALL */
#define IS_ROCK(typ)	((typ) < POOL)		/* absolutely nonaccessible */
#define IS_DOOR(typ)	((typ) == DOOR)
#define ACCESSIBLE(typ)	((typ) >= DOOR)		/* good position */
#define IS_ROOM(typ)	((typ) >= ROOM)		/* ROOM, STAIRS, furniture.. */
#define ZAP_POS(typ)	((typ) >= POOL)
#define SPACE_POS(typ)	((typ) > DOOR)
#define IS_CORNER(typ)	((typ) >= TLCORNER && (typ) <= BRCORNER)
#define IS_T(typ)	((typ) >= CRWALL && (typ) <= TRWALL)
#define IS_POOL(typ)	((typ) >= POOL && (typ) <= DRAWBRIDGE_UP)
#define IS_THRONE(typ)	((typ) == THRONE)
#define IS_FOUNTAIN(typ) ((typ) == FOUNTAIN)
#define IS_SINK(typ)	((typ) == SINK)
#define IS_ALTAR(typ)	((typ) == ALTAR)
#define IS_DRAWBRIDGE(typ) ((typ) == DRAWBRIDGE_UP || (typ) == DRAWBRIDGE_DOWN)
#define IS_FURNITURE(typ) ((typ) >= STAIRS && (typ) <= ALTAR)

/*
 * The level-map symbols may be compiled in or defined at initialization time
 */

/* screen symbols for using character graphics. */
struct symbols {
    unsigned char stone, vwall, hwall, tlcorn, trcorn, blcorn, brcorn;
    unsigned char crwall, tuwall, tdwall, tlwall, trwall;
    unsigned char vbeam, hbeam, lslant, rslant;
    unsigned char door, room, corr, upstair, dnstair, trap, web;
    unsigned char pool;
    unsigned char fountain;
    unsigned char sink;
    unsigned char throne;
    unsigned char altar;
    unsigned char upladder, dnladder, dbvwall, dbhwall;
};
extern struct symbols showsyms;
#ifdef REINCARNATION
extern struct symbols savesyms;
#endif
extern const struct symbols defsyms;

#define STONE_SYM	showsyms.stone
#define VWALL_SYM	showsyms.vwall
#define HWALL_SYM	showsyms.hwall
#define TLCORN_SYM	showsyms.tlcorn
#define TRCORN_SYM	showsyms.trcorn
#define BLCORN_SYM	showsyms.blcorn
#define BRCORN_SYM	showsyms.brcorn
#define CRWALL_SYM	showsyms.crwall
#define TUWALL_SYM	showsyms.tuwall
#define TDWALL_SYM	showsyms.tdwall
#define TLWALL_SYM	showsyms.tlwall
#define TRWALL_SYM	showsyms.trwall
#define VBEAM_SYM	showsyms.vbeam
#define HBEAM_SYM	showsyms.hbeam
#define LSLANT_SYM	showsyms.lslant
#define RSLANT_SYM	showsyms.rslant
#define DOOR_SYM	showsyms.door
#define ROOM_SYM	showsyms.room
#define	CORR_SYM	showsyms.corr
#define UP_SYM		showsyms.upstair
#define DN_SYM		showsyms.dnstair
#define TRAP_SYM	showsyms.trap
#define WEB_SYM		showsyms.web
#define	POOL_SYM	showsyms.pool
#define FOUNTAIN_SYM	showsyms.fountain
#define SINK_SYM	showsyms.sink
#define THRONE_SYM	showsyms.throne
#define ALTAR_SYM	showsyms.altar
#define UPLADDER_SYM	showsyms.upladder
#define DNLADDER_SYM	showsyms.dnladder
#define DB_VWALL_SYM	showsyms.dbvwall
#define DB_HWALL_SYM	showsyms.dbhwall

#define	ERRCHAR	']'

#define MAXPCHARS	32	/* maximum number of mapped characters */

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
#define DB_UNDER	8	/* mask for underneath */

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
 * at() display character types, in order of precedence.
 */
#define AT_APP		(uchar)0
/* 1-7 are specific overrides, see decl.h */
/* non-specific */
#define AT_ZAP		(uchar)8
#define AT_MON		(uchar)9
#define AT_U		AT_MON
#define AT_OBJ		(uchar)10
#define AT_GLD		AT_OBJ
#define AT_MAP		(uchar)11

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
	Bitfield(mmask,1);
	Bitfield(omask,1);
	Bitfield(gmask,1);
};

#define altarmask	doormask
#define diggable	doormask
#define ladder		doormask
#define drawbridgemask	doormask

extern struct rm levl[COLNO][ROWNO];

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
