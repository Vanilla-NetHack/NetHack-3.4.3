/*	SCCS Id: @(#)decl.h	3.0	88/10/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

E int bases[];
E int warnlevel;	/* defined in mon.c */
E int occtime;
E int nroom;
E int multi;
E int hackpid;
#ifdef UNIX
E int locknum;
#ifdef DEF_PAGER
E char *catmore;
#endif	/* DEF_PAGER */
#endif	/* UNIX */
E char SAVEF[];
E char *hname;
E const char *hu_stat[];	/* defined in eat.c */
E int medusa_level;
E int bigroom_level;
#ifdef REINCARNATION
E int rogue_level;
#endif
#ifdef ORACLE
E int oracle_level;
#endif
#ifdef STRONGHOLD
E int stronghold_level, tower_level;
#endif
E int wiz_level;
E boolean is_maze_lev;

E xchar xdnstair, ydnstair, xupstair, yupstair; /* stairs up and down. */
#ifdef STRONGHOLD
E xchar xdnladder, ydnladder, xupladder, yupladder; /* ladders up and down. */
#endif
E xchar scrlx, scrhx, scrly, scrhy; /* corners of new area on screen. pri.c */
E xchar dlevel;
E xchar maxdlevel; /* dungeon level */
E int done_stopprint;
E int done_hup;
E xchar curx,cury;	/* cursor location on screen */
E xchar seehx, seelx, seehy, seely; /* where to see */
E xchar seehx2, seelx2, seehy2, seely2; /* where to see */
E xchar fountsound, sinksound;	/* numbers of noisy things */

E char pl_character[PL_CSIZ];
#ifdef TUTTI_FRUTTI
E char pl_fruit[PL_FSIZ];
E int current_fruit;
E struct fruit *ffruit;
#endif
#ifdef STRONGHOLD
E char tune[6];
#  ifdef MUSIC
E schar music_heard;
#  endif
#endif

E const char quitchars[];
E const char vowels[];
E const char ynchars[];
E const char ynqchars[];
E const char ynaqchars[];
E const char nyaqchars[];
E int smeq[];
E int doorindex;
E char *save_cm;
E char *killer;
E char inv_order[];
E char plname[PL_NSIZ];
E char dogname[];
E char catname[];
E const char sdir[], ndir[];	/* defined in hack.c */
E char *occtxt;		/* defined when occupation != NULL */
E char *nomovemsg;
E const char nul[];
E char *HI, *HE, *AS, *AE;	/* set up in termcap.c */
E char *CD;			/* set up in termcap.c */
E int CO, LI;			/* set up in termcap.c: COLNO and ROWNO+3 */
E char *traps[];
#ifndef MAKEDEFS_C  /* avoid conflict with lock() */
E char lock[];
#endif
E char morc;

E const schar xdir[], ydir[];	/* idem */
E schar tbx, tby;		/* set in mthrowu.c */
E int dig_effort;	/* apply.c, hack.c */
E uchar dig_level;
E coord dig_pos;
E boolean dig_down;

E long moves;
E long wailmsg;

E boolean in_mklev;
E boolean stoned;
E boolean unweapon;
E boolean mrg_to_wielded;

#ifdef KOPS
E boolean allow_kops;
#endif

#ifdef SPELLS
#ifndef SPELLS_H
#include "spell.h"
#endif
E struct spell spl_book[];	/* sized in decl.c */
#endif

#ifdef REDO
E int in_doagain;
#endif

#ifdef MSDOSCOLOR
E char *HI_RED, *HI_YELLOW, *HI_GREEN, *HI_BLUE, *HI_WHITE; /* termcap.c */
#endif

#ifndef OBJ_H
#include "obj.h"
#endif

E struct obj *invent, *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
#ifdef SHIRT
	*uarmu, /* under-wear, so to speak */
#endif
#ifdef POLYSELF
	*uskin,
#endif
	*uamul, *uleft, *uright, *ublindf, *fcobj, *uwep;

E struct obj *uchain;	/* defined iff PUNISHED */
E struct obj *uball;	/* defined if PUNISHED */

#ifndef YOU_H
#include "you.h"
#endif

E struct you u;

#ifndef MAKEDEFS_C
#ifndef ONAMES_H
#include "onames.h"
#endif
#ifndef PM_H
#include "pm.h"
#endif
#endif /* MAKEDEFS_C */

E struct permonst playermon, *uasmon;	/* also decl'd extern in permonst.h */
					/* init'd in monst.c */
E struct obj zeroobj;		/* init'd and defined in decl.c */

E struct monst youmonst;	/* init'd and defined in decl.c */

E const char obj_symbols[];		/* init'd in objects.h */

E struct obj *billobjs;

E const char black[];
E const char amber[];
#ifdef THEOLOGY
E const char golden[];
#endif
E const char light_blue[];
E const char red[];
E const char green[];
E const char silver[];
E const char blue[];
E const char purple[];
E const char white[];

E const char nothing_happens[];
E const char thats_enough_tries[];

#undef E

#endif /* DECL_H /**/
