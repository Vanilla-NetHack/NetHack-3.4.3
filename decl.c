/*	SCCS Id: @(#)decl.c	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Decl.c - version 1.0.3 */

#include	"hack.h"
char nul[40];			/* contains zeros */
char plname[PL_NSIZ];		/* player name */

#ifdef GRAPHICS
struct symbols defsyms = {
    ' ', '|', '-', '-', '-', '-', '-', '+', '.', '#', '<', '>', '^',
#ifdef FOUNTAINS
    '}', '{',
#endif
#ifdef NEWCLASS
    '\\',
#endif
#ifdef SPIDERS
    '"',
#endif
};
#endif
struct symbols showsyms;	/* will contain the symbols actually used */

#ifdef DGK
char hackdir[PATHLEN];		/* where rumors, help, record are */
char levels[PATHLEN];		/* where levels are */
char lock[FILENAME];		/* pathname of level files */
char permbones[PATHLEN];	/* where permanent copy of bones go */
int ramdisk = FALSE;		/* whether to copy bones to levels or not */
int saveprompt = TRUE;
char *alllevels = "levels.*";
char *allbones = "bones.*";
char *configfile = "HACK.CNF";	/* read by read_config_file() */
#else
char lock[PL_NSIZ+4] = "1lock";	/* long enough for login name .99 */
#endif

boolean in_mklev, restoring;
struct rm levl[COLNO][ROWNO]; /* level map */

#ifndef QUEST
#include "mkroom.h"
struct mkroom rooms[MAXNROFROOMS+1];
coord doors[DOORMAX];
#endif
struct monst *fmon = 0;
struct trap *ftrap = 0;
struct gold *fgold = 0;
struct obj *fobj = 0, *fcobj = 0, *invent = 0, *uwep = 0, *uarm = 0,
	*uarm2 = 0, *uarmh = 0, *uarms = 0, *uarmg = 0, *uright = 0,
	*uleft = 0, *uchain = 0, *uball = 0;
struct flag flags;
struct you u;
#ifdef SPELLS
struct spell spl_book[MAXSPELL + 1];
#endif
struct rm levl[COLNO][ROWNO];	/* level map */
struct monst youmonst;	/* dummy; used as return value for boomhit */

xchar dlevel = 1;
xchar xupstair, yupstair, xdnstair, ydnstair;
char *save_cm = 0, *killer, *nomovemsg;

long moves = 1;
long wailmsg = 0;
int multi = 0;
char	*occtxt;
#ifdef DGKMOD
int	occtime;
#endif
#ifdef REDO
int	in_doagain;
#endif

char *HI, *HE;		/* set up in termcap.c */

char genocided[60];
char fut_geno[60];
#ifdef KAA
boolean	stoned;				/* done to monsters hit by 'c' */
boolean	unweapon;
#endif
 
xchar curx,cury;
xchar seelx, seehx, seely, seehy;	/* corners of lit room */

coord bhitpos;

char quitchars[] = " \r\n\033";
