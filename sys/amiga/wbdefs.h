/*    SCCS Id: @(#)wbdefs.h     3.1    93/01/08
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993.  */
/* NetHack may be freely redistributed.  See license for details. */

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#ifdef AZTEC_C
#include <libraries/dosextens.h>
#include <functions.h>
#else
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef  IDCMP_CLOSEWINDOW
#define INTUI_NEW_LOOK  1
#endif

#define R_DISK		1   /* Refresh reasons */
#define R_WINDOW	2
#define R_SCROLL	3

#define PLAYERMENU	0

#define MENU_PROJECT	0   /* so we can shuffle them around easily */
#define ITEM_HELP	0
#define ITEM_ABOUT	1
#define ITEM_SCORES	2
#define ITEM_RECOVER	3
#define ITEM_CONFIG	4
#define ITEM_QUIT	5

#define MENU_GAME	1
#define ITEM_INFO	0
#define ITEM_COPYOPT	1
#define ITEM_DISCARD	2
#define ITEM_RENAME	3

#define GADSCROLL	1   /* The scroll bar */
#define GADNEWGAME	2   /* New Game requested */

#define GADSCRLUP	3   /* Scroll Up Gadget */
#define GADSCRLDOWN	4   /* Scroll Down Gadget */
#define GADINFOSCRL	5   /* The scroll bar */

#define GADQUESTYES	6
#define GADQUESTNO	7

#define GADCOMSTR	8

#define GADCHARNAME	9
#define GADOUTFILE	10
#define GADCATNAME	11
#define GADDOGNAME	12

#define GADSTRSAVE	15
#define GADSTRLEVELS	16
#define GADSTRPATH	17
#define GADSTRPENS	18
#define GADSTRHACKDIR	19
#define GADCONFSAVE	21
#define GADCONFLOAD	22
#define GADCONFNAME	23
#define GADTOOLUP	24
#define GADTOOLDOWN	25
#define GADADDTOOL	26
#define GADDELTOOL	27
#define GADTOOLTYPES	28
#define GADSTRCANCEL	29
#define GADHELPOKAY	30
#define GADHELPFRWD	31
#define GADHELPBKWD	32
#define GADEDITOPTS	33

#define GADRESTDIR	34
#define GADRESTOLD	35
#define GADRESTNEW	36
#define GADRESTCAN	37
#define GADRESTOKAY	38
#define GADSAVEINFO	39
#define GADUSEINFO	40
#define GADQUITINFO	41
#define GADPLNAME	42

/*
 *  Option gadgets GadgetID's
 */
#define GADOCOLOR	101
#define GADOCONFIRM	102
#define GADODISCLOSE	103
#define GADOFIXINV	104
#define GADONULL	105
#define GADOTIME	106
#define GADONEWS	107
#define GADOHELP	108
#define GADOFLUSH	109
#define GADORESTONSPACE	110
#define GADOPICKUP	111
#define GADOSOUND	112
#define GADONUMBERPAD	113
#define GADOSAFEPET	114
#define GADOSILENT	115
#define GADOTOMBSTONE	116
#define GADOVERBOSE	117
#define GADOSTANDOUT	118
#define GADOSORTPACK	119
#define GADOFEMALE	120
#define GADOIGNINTR	121
#define GADOPACKORDER	122
#define GADODOGNAME	123
#define GADOCATNAME	124
#define GADOFRUIT	125
#define GADOOBJECTS	126
#define GADOASKSAVE	127
#define GADOCANCEL	128
#define GADOOKAY	129
#define GADOCHKPOINT	130
#define GADOHILITEPET	131
#define GADOLEGACY	132
#define GADOLITCORRIDOR	133
#define GADOSHOWEXP	134
#define GADOSHOWSCORE	135
#define	GADONAME	136
#define	GADOSCORE	137
#define	GADOPALETTE	138
#define	GADOWINDOWTYPE	139
#define	GADOMSGHISTORY	140
#define	GADOPICKUPTYPES	141
#define	GADOPETTYPE	142

/* Definition of workbench size layout */

#define ORIGINX ( win->BorderLeft + 5 )
#define ORIGINY ( win->BorderTop + Message.Height + 5 )
#define CORNERX ( win->Width - win->BorderRight )
#define CORNERY ( win->Height - win->BorderBottom )

/* String buffer in String Gadget */

#define Sbuff(gd)	(((struct StringInfo*)((gd)->SpecialInfo))->Buffer)

/* The string gadgets' buffers */

#define StrPath		(Sbuff(&Conf_StrPath))
#define StrOptions	(Sbuff(&Conf_StrOptions))
#define StrHackdir	(Sbuff(&Conf_StrHackdir))
#define StrPens		(Sbuff(&Conf_StrPens))
#define StrLevels	(Sbuff(&Conf_StrLevels))
#define StrSave		(Sbuff(&Conf_StrSave))
#define StrConf		(Sbuff(&Conf_ConfigName))

#define RstDir		(Sbuff(&Rst_RestDir))
#define RstOld		(Sbuff(&Rst_RestOld))
#define RstNew		(Sbuff(&Rst_RestNew))

#define StrString	(Sbuff(&Str_String))

#define StrTools	(Sbuff(&Info_ToolTypes))
#define StrPlayer	(Sbuff(&Info_Player))

#define GAMEIMAGE	"HackExe:NetHack"
#define GAMESTACK	50000

#define PATH_IDX	0
#define OPTIONS_IDX	1
#define HACKDIR_IDX	2
#define LEVELS_IDX	3
#define SAVE_IDX	4
#define PENS_IDX	5

#define NUMIDX		6

#define PL_RANDOM	0
#define PL_ARCHEOLOGIST	1
#define PL_BARBARIAN	2
#define PL_CAVEMAN	3
#define PL_ELF		4
#define PL_HEALER	5
#define PL_KNIGHT	6
#define PL_PRIEST	7
#define PL_ROGUE	8
#define PL_SAMURAI	9
#define PL_TOURIST	10
#define PL_VALKYRIE	11
#define PL_WIZARD	12

#define MENUITEMNO( menu, itm, sitm ) FULLMENUNUM( menu, itm, sitm )
#define GADWIDTH( gad )     max((gad)->Width, \
		    strlen((gad)->GadgetText->IText) * win->RPort->TxWidth)

/* Horizontal space between gadgets */
#define GADINCX		10

/* Vertical space between gadgets */
#define GADINCY		(win->RPort->TxHeight + 1)

#define NO_FLASH	0
#define FLASH		1
