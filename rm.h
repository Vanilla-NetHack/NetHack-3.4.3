/*	SCCS Id: @(#)rm.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* rm.h - version 1.0.2 */

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
 * A few of the associated symbols are not hardwired.
 */
#ifdef QUEST
#define	CORR_SYM	':'
#else
#define	CORR_SYM	'#'
#endif
#define	POOL_SYM	'}'
#define FOUNTAIN_SYM    '{'
#define THRONE_SYM      '\\'
#define WEB_SYM         '"'
#define DOOR_SYM	'+'

#define	ERRCHAR	']'

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
	uchar typ:5;
	uchar new:1;
	uchar seen:1;
	uchar lit:1;
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
