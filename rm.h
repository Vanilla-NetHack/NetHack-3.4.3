/*	SCCS Id: @(#)rm.h	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* rm.h - version 1.0.2 */

/*
 * The dungeon presentation graphics code and data structures were rewritten
 * and generalized for NetHack's release 2 by Eric S. Raymond (eric@snark)
 * building on Don G. Kneller's MS-DOS implementation. See options.c for
 * the code that permits the user to set the contents of the symbol structure.
 */

/* Level location types */
#define	HWALL 1
#define	VWALL 2
#define	SDOOR 3
#define	SCORR 4
#define	LDOOR 5
#define	POOL	6	/* not yet fully implemented */
			/* this should in fact be a bit like lit */
#define	DOOR 7
#define	CORR 8
#define	ROOM 9
#define	STAIRS 10
#define FOUNTAIN 11
#define THRONE 12

/*
 * Avoid using the level types in inequalities:
 *  these types are subject to change.
 * Instead, use one of the macros below.
 */
#define	IS_WALL(typ)	((typ) <= VWALL)
#define IS_ROCK(typ)	((typ) < POOL)		/* absolutely nonaccessible */
#define	ACCESSIBLE(typ)	((typ) >= DOOR)			/* good position */
#define	IS_ROOM(typ)		((typ) >= ROOM)		/* ROOM or STAIRS */
#define	ZAP_POS(typ)		((typ) > DOOR)
#define IS_POOL(typ)    ((typ) == POOL)
#define IS_THRONE(typ)    ((typ) == THRONE)
#define IS_FOUNTAIN(typ)        ((typ) == FOUNTAIN)

/*
 * The level-map symbols may be compiled in or defined at initialization time
 */
#ifndef GRAPHICS

#define STONE_SYM	' '
#define VWALL_SYM	'|'
#define HWALL_SYM	'-'
#define TLCORN_SYM	'+'
#define TRCORN_SYM	'+'
#define BLCORN_SYM	'+'
#define BRCORN_SYM	'+'
#define DOOR_SYM	'+'
#define ROOM_SYM	'.'
#ifdef QUEST
# define	CORR_SYM	':'
#else
# define	CORR_SYM	'#'
#endif
#define UP_SYM		'<'
#define DN_SYM		'>'
#define TRAP_SYM	'^'
#define	POOL_SYM	'}'
#define FOUNTAIN_SYM    '{'
#define THRONE_SYM      '\\'
#define WEB_SYM         '"'
#else /* GRAPHICS */

/* screen symbols for using character graphics. */
struct symbols {
    unsigned char stone, vwall, hwall, tlcorn, trcorn, blcorn, brcorn;
    unsigned char door, room, corr, upstair, dnstair, trap;
#ifdef FOUNTAINS
    unsigned char pool, fountain;
#endif
#ifdef NEWCLASS
    unsigned char throne;
#endif
#ifdef SPIDERS
    unsigned char web;
#endif
};
extern struct symbols showsyms, defsyms;

#define STONE_SYM	showsyms.stone
#define VWALL_SYM	showsyms.vwall
#define HWALL_SYM	showsyms.hwall
#define TLCORN_SYM	showsyms.tlcorn
#define TRCORN_SYM	showsyms.trcorn
#define BLCORN_SYM	showsyms.blcorn
#define BRCORN_SYM	showsyms.brcorn
#define DOOR_SYM	showsyms.door
#define ROOM_SYM	showsyms.room
#define	CORR_SYM	showsyms.corr
#define UP_SYM		showsyms.upstair
#define DN_SYM		showsyms.dnstair
#define TRAP_SYM	showsyms.trap
#define	POOL_SYM	showsyms.pool
#define FOUNTAIN_SYM    showsyms.fountain
#define THRONE_SYM      showsyms.throne
#define WEB_SYM         showsyms.web
#endif

#define	ERRCHAR	']'

#define MAXPCHARS	17	/* maximum number of mapped characters */

#define IS_CORNER(x)	((x) == TLCORN_SYM || (x) == TRCORN_SYM \
			 || (x) == BLCORN_SYM || (x) == BRCORN_SYM)

/*
 * The structure describing a coordinate position.
 * Before adding fields, remember that this will significantly affect
 * the size of temporary files and save files.
 */
#ifdef MSDOS
/* Save disk space by using unsigned char's instead of unsigned ints
 */
struct rm {
	uchar scrsym;
	unsigned typ:5;
	unsigned new:1;
	unsigned seen:1;
	unsigned lit:1;
};
#else
struct rm {
	char scrsym;
	Bitfield(typ,5);
	Bitfield(new,1);
	Bitfield(seen,1);
	Bitfield(lit,1);
};
#endif /* MSDOS /**/
extern struct rm levl[COLNO][ROWNO];

#ifdef DGK
#define ACTIVE	1
#define SWAPPED	2

struct finfo {
	int	where;
	long	time;
	long	size;
};
extern struct finfo fileinfo[];
#endif
