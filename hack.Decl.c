/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
char nul[40];			/* contains zeros */
char plname[PL_NSIZ] = "player";/* player name */
char lock[32] = "1lock";	/* long enough for login name */

#ifdef WIZARD
boolean wizard;			/* TRUE when called as  hack -w */
#endif WIZARD

struct rm levl[COLNO][ROWNO];	/* level map */
#ifndef QUEST
struct mkroom rooms[MAXNROFROOMS+1];
coord doors[DOORMAX];
#endif QUEST
struct monst *fmon = 0;
struct gen *fgold = 0, *ftrap = 0;
struct obj *fobj = 0, *fcobj = 0, *invent = 0, *uwep = 0, *uarm = 0,
	*uarm2 = 0, *uarmh = 0, *uarms = 0, *uarmg = 0, *uright = 0,
	*uleft = 0, *uchain = 0, *uball = 0;
struct flag flags;
struct you u;

xchar dlevel = 1;
xchar xupstair, yupstair, xdnstair, ydnstair;
char *save_cm = 0, *killer, *nomovemsg;

long moves = 1;
long wailmsg = 0;

int multi = 0;
char genocided[60];
char fut_geno[60];

xchar curx,cury;
xchar seelx, seehx, seely, seehy;	/* corners of lit room */

coord bhitpos;
