/*	SCCS Id: @(#)decl.h	3.0	88/10/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

E int NEARDATA bases[];
E int NEARDATA warnlevel;	/* defined in mon.c */
E int NEARDATA occtime;
E int NEARDATA nroom;
E int NEARDATA multi;
E int hackpid;
#if defined(UNIX) || defined(VMS)
E int locknum;
#endif
#ifdef DEF_PAGER
E char *catmore;
#endif	/* DEF_PAGER */
E char SAVEF[];
E const char *hname;
E const char *hu_stat[];	/* defined in eat.c */
E int NEARDATA medusa_level;
E int NEARDATA bigroom_level;
#ifdef REINCARNATION
E int NEARDATA rogue_level;
#endif
#ifdef ORACLE
E int NEARDATA oracle_level;
#endif
#ifdef STRONGHOLD
E int NEARDATA stronghold_level, NEARDATA tower_level;
#endif
E int NEARDATA wiz_level;
E boolean NEARDATA is_maze_lev;

E xchar NEARDATA xdnstair, NEARDATA ydnstair, NEARDATA xupstair,
	NEARDATA  yupstair; /* stairs up and down. */
#ifdef STRONGHOLD
E xchar NEARDATA xdnladder, NEARDATA ydnladder, NEARDATA xupladder,
	NEARDATA yupladder; /* ladders up and down. */
#endif
E xchar NEARDATA scrlx, NEARDATA scrhx, NEARDATA scrly, NEARDATA scrhy;
	 /* corners of new area on screen. pri.c */
E xchar NEARDATA dlevel;
E xchar NEARDATA maxdlevel; /* dungeon level */
E int NEARDATA done_stopprint;
E int NEARDATA done_hup;
E xchar NEARDATA curx,NEARDATA cury;	/* cursor location on screen */
E xchar NEARDATA seehx, NEARDATA seelx, NEARDATA seehy, NEARDATA seely;
	 /* where to see */
E xchar NEARDATA seehx2, NEARDATA seelx2, NEARDATA seehy2, NEARDATA seely2;
	 /* where to see */
E xchar NEARDATA fountsound, NEARDATA sinksound; /* numbers of noisy things */

E char NEARDATA pl_character[PL_CSIZ];
E const char *pl_classes;
#ifdef TUTTI_FRUTTI
E char NEARDATA pl_fruit[PL_FSIZ];
E int NEARDATA current_fruit;
E struct fruit NEARDATA *ffruit;
#endif
#ifdef STRONGHOLD
E char NEARDATA tune[6];
#  ifdef MUSIC
E schar NEARDATA music_heard;
#  endif
#endif

E const char NEARDATA quitchars[];
E const char NEARDATA vowels[];
E const char NEARDATA ynchars[];
E const char NEARDATA ynqchars[];
E const char NEARDATA ynaqchars[];
E const char NEARDATA nyaqchars[];
E int NEARDATA smeq[];
E int NEARDATA doorindex;
E char NEARDATA *save_cm;
#define KILLED_BY_AN 0
#define KILLED_BY 1
#define NO_KILLER_PREFIX 2
E int NEARDATA killer_format;
E const char NEARDATA *killer;
E char inv_order[];
E char NEARDATA plname[PL_NSIZ];
E char NEARDATA dogname[];
E char NEARDATA catname[];
E const char NEARDATA sdir[], NEARDATA ndir[];	/* defined in cmd.c */
E const char NEARDATA *occtxt;		/* defined when occupation != NULL */
E const char NEARDATA *nomovemsg;
E const char NEARDATA nul[];
E char *HI, *HE, *AS, *AE;	/* set up in termcap.c */
E char *CD;			/* set up in termcap.c */
E int CO, LI;			/* set up in termcap.c: COLNO and ROWNO+3 */
E const char *traps[];
#ifndef MAKEDEFS_C  /* avoid conflict with lock() */
E char lock[];
#endif
E char morc;

E const schar NEARDATA xdir[], NEARDATA ydir[];	/* idem */
E schar NEARDATA tbx, NEARDATA tby;		/* set in mthrowu.c */
E int NEARDATA dig_effort;	/* apply.c, hack.c */
E uchar NEARDATA dig_level;
E coord NEARDATA dig_pos;
E boolean NEARDATA dig_down;

E long NEARDATA moves, NEARDATA monstermoves;
E long NEARDATA wailmsg;

E boolean NEARDATA in_mklev;
E boolean NEARDATA stoned;
E boolean NEARDATA unweapon;
E boolean NEARDATA mrg_to_wielded;

#ifdef KOPS
E boolean NEARDATA allow_kops;
#endif

#ifdef SPELLS
#ifndef SPELLS_H
#include "spell.h"
#endif
E struct spell NEARDATA spl_book[];	/* sized in decl.c */
#endif

#ifdef REDO
E int NEARDATA in_doagain;
#endif

#ifdef CLIPPING
E boolean clipping;
E int clipx, clipy, clipxmax, clipymax;
#endif

#ifdef TEXTCOLOR
#ifndef COLOR_H
#include "color.h"
#endif
# ifdef TOS
E const char *hilites[MAXCOLORS];
# else
E char NEARDATA *hilites[MAXCOLORS];
# endif
#endif

#ifndef OBJ_H
#include "obj.h"
#endif

E struct obj NEARDATA *invent, NEARDATA *uarm, NEARDATA *uarmc,
	NEARDATA *uarmh, NEARDATA *uarms, NEARDATA *uarmg, NEARDATA *uarmf,
#ifdef SHIRT
	NEARDATA *uarmu, /* under-wear, so to speak */
#endif
#ifdef POLYSELF
	NEARDATA *uskin,
#endif
	NEARDATA *uamul, NEARDATA *uleft, NEARDATA *uright, NEARDATA *ublindf,
	NEARDATA *fcobj, NEARDATA *uwep;

E struct obj NEARDATA *uchain;	/* defined iff PUNISHED */
E struct obj NEARDATA *uball;	/* defined if PUNISHED */

#ifndef YOU_H
#include "you.h"
#endif

E struct you NEARDATA u;

#ifndef MAKEDEFS_C
#ifndef ONAMES_H
#include "onames.h"
#endif
#ifndef PM_H
#include "pm.h"
#endif
#endif /* MAKEDEFS_C */

E struct permonst NEARDATA playermon, NEARDATA *uasmon;
					/* also decl'd extern in permonst.h */
					/* init'd in monst.c */
E struct obj NEARDATA zeroobj;		/* init'd and defined in decl.c */

E struct monst NEARDATA youmonst;	/* init'd and defined in decl.c */

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

E const char monsyms[], objsyms[];
E const char *monexplain[], *objexplain[];

#ifdef NAMED_ITEMS
E const int artifact_num;
E boolean artiexist[];
#endif

#undef E

#endif /* DECL_H /**/
