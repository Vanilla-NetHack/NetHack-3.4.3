/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "config.h"

#ifdef BSD
#include <strings.h>		/* declarations for strcat etc. */
#else
#include <string.h>		/* idem on System V */
#define	index	strchr
#define	rindex	strrchr
#endif BSD

#include	"def.objclass.h"

typedef struct {
	xchar x,y;
} coord;

#include	"def.monst.h"	/* uses coord */
#include	"def.gen.h"
#include	"def.obj.h"

extern char *sprintf();

#define	BUFSZ	256	/* for getlin buffers */
#define	PL_NSIZ	32	/* name of player, ghost, shopkeeper */

#define	HWALL 1	/* Level location types */
#define	VWALL 2
#define	SDOOR 3
#define	SCORR 4
#define	LDOOR 5
#define	DOOR 6	/* smallest accessible type */
#define	CORR 7
#define	ROOM 8
#define	STAIRS 9
#ifdef QUEST
#define	CORR_SYM	':'
#else
#define	CORR_SYM	'#'
#endif QUEST

#define	ERRCHAR	'{'

#define TRAPNUM 9

struct rm {
	char scrsym;
	unsigned typ:5;
	unsigned new:1;
	unsigned seen:1;
	unsigned lit:1;
};
extern struct rm levl[COLNO][ROWNO];

#ifndef QUEST
struct mkroom {
	xchar lx,hx,ly,hy;
	schar rtype,rlit,doorct,fdoor;
};
#define	MAXNROFROOMS	15
extern struct mkroom rooms[MAXNROFROOMS+1];
#define	DOORMAX	100
extern coord doors[DOORMAX];
#endif QUEST


#include	"def.permonst.h"
extern struct permonst mons[];
#define PM_ACIDBLOB	&mons[7]
#define	PM_PIERC	&mons[17]
#define	PM_MIMIC	&mons[37]
#define	PM_CHAM		&mons[47]
#define	PM_DEMON	&mons[54]
#define	PM_MINOTAUR	&mons[55]	/* last in mons array */
#define	PM_SHK		&mons[56]	/* very last */
#define	CMNUM		55		/* number of common monsters */

extern long *alloc();

extern xchar xdnstair, ydnstair, xupstair, yupstair; /* stairs up and down. */

extern xchar dlevel;
#ifdef WIZARD
extern boolean wizard;
#endif WIZARD
#define	newstring(x)	(char *) alloc((unsigned)(x))
