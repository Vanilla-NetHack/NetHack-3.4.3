/*	SCCS Id: @(#)hack.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.h - version 1.0.3 */

#include "config.h"

#define	Null(type)	((struct type *) 0)

#include	"objclass.h"

typedef struct {
	xchar x,y;
} coord;

extern coord bhitpos;	/* place where thrown weapon falls to the ground */

#include	"monst.h"	/* uses coord */
#include	"gold.h"
#include	"trap.h"
#include	"obj.h"
#include	"flag.h"

#define	plur(x)	(((x) == 1) ? "" : "s")
#define min(x,y) ((x) < (y) ? (x) : (y))

#define	BUFSZ	256	/* for getlin buffers */
#define	PL_NSIZ	32	/* name of player, ghost, shopkeeper */

#include	"rm.h"

#define Inhell		(dlevel >= 30)
#define	newstring(x)	(char *) alloc((unsigned)(x))

#ifdef SPELLS
#include "spell.h"
#define	NO_SPELL	0
#endif

#define	TELL	1
#define NOTELL	0

#define ON 1
#define OFF 0

#include "extern.h"

#define DIST(x1,y1,x2,y2)       (((x1)-(x2))*((x1)-(x2)) + ((y1)-(y2))*((y1)-(y2)))

#define	PL_CSIZ		20	/* sizeof pl_character */
#define	MAX_CARR_CAP	120	/* so that boulders can be heavier */
#define	MAXLEVEL	40
#define	FAR	(COLNO+2)	/* position outside screen */
