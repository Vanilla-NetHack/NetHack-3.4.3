/*	SCCS Id: @(#)macconf.h	3.0	88/07/21 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Copyright (c) Johnny Lee, 1989 		*/
/* NetHack may be freely redistributed.  See license for details. */
#ifdef MACOS
#ifndef MACCONF_H
#define MACCONF_H

/*
 *  The following options are configurable:
 */

#define RANDOM		1 /* have Berkeley random(3) */

#define PATHLEN	220	/* maximum pathlength */
#define FILENAME	31	/* maximum filename length (conservative) */

#define glo(x)	name_file(lock, x)	/* name_file used for bones */
#include "msdos.h"	/* contains necessary externs for [os_name].c */
extern char *configfile;
#define NO_SIGNAL	1
#define	perror(x)

/*
 *  The remaining code shouldn't need modification.
 */

#ifndef SYSTEM_H
#include "system.h"
#endif


#ifdef RANDOM
/* Use the high quality random number routines. */
#define Rand()	random()
#define Srand(seed)	srandom(seed)
#else
#define Rand()	rand()
#define Srand(seed)	srand(seed)
#endif /* RANDOM */

#ifndef REDO
#undef	Getchar
#define Getchar tgetch
#endif

#ifdef THINK_C

#define index	strchr
#define rindex	strrchr
#include <time.h>
#define	FCMASK	O_WRONLY | O_BINARY | O_CREAT	/* file creation mask */

#ifdef LSC
#include	<types.h>
#include	<io.h>
#define	memcpy(x,y,j)	movmem(y,x,j)
extern char	*calloc();
#else
#include	<Fcntl.h>
#include	<Stddef.h>
#include	<Stdlib.h>
#include	<String.h>
#undef getuid
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
#ifdef stdout
#undef stdout
#define stdout (FILE *)NULL
#endif
#endif
#endif

#include	<Quickdraw.h>
#include	<FontMgr.h>
#include	<EventMgr.h>
#include	<WindowMgr.h>
#include	<MenuMgr.h>
#include	<StdFilePkg.h>
#include	<SegmentLdr.h>
#include	<ToolboxUtil.h>
#include	<OSUtil.h>
#include	<DialogMgr.h>
#include	<FileMgr.h>
#include	<HFS.h>
#include	<Color.h>
#include	<ResourceMgr.h>

#ifdef fflush
#undef	fflush
#define	fflush(x)
#endif

/* these two defines for variables in decl.c; they conflict with */
/* variables in Quickdraw.h - the Quickdraw variables are never used in NH */
#define	black	Black
#define	white	White


#else	/* Aztec and MPW */

#ifdef AZTEC
#include	<utime.h>	/* AZTEC 3.6c */
#define	curs(x,y)	tcurs(x,y)
#else
#include	<Time.h>	/* MPW 3.0 */
#endif


#include	<Quickdraw.h>
#include	<Fonts.h>
#include	<Events.h>
#include	<Windows.h>
#include	<Menus.h>
#include	<Packages.h>
#include	<SegLoad.h>
#include	<ToolUtils.h>
#include	<OSUtils.h>
#include	<Dialogs.h>
#include	<Files.h>
#include	<Resources.h>
#ifdef MPW
#include	<Script.h>
#include	<SysEqu.h>
#endif
#include	<Signal.h>
#include	<String.h>
#include	<FCntl.h>
#define	FCMASK	O_WRONLY | O_CREAT	/* file creation mask */
#endif

/* typdef and defines for special custom termcap routines */
typedef struct term_info {
	short	tcur_x,tcur_y;
	short	fontNum,fontSize;
	short	ascent,descent,height,charWidth;
	short	maxRow,maxCol;
	char	**screen;
	short	inColor;
	short	auxFileVRefNum;
	short	recordVRefNum;
	SysEnvRec	system;
	char	*keyMap;
	short	color[8];
	Handle	shortMBarHandle,
			fullMBarHandle;
} term_info;

#define TEXTCOLOR	1

#define	appleMenu	101
#define	fileMenu	102
#define	editMenu	103
#define	inventMenu	104
#define actionMenu	105
#define prepMenu	106
#define	moveMenu	107
#define extendMenu	108

#ifdef THINK_C
#define MAINGRAFPORT	thePort
#define	ARROW_CURSOR	arrow
#define	SCREEN_BITS	screenBits
#else
#define MAINGRAFPORT	qd.thePort
#define	ARROW_CURSOR	qd.arrow
#define	SCREEN_BITS	qd.screenBits
#endif

/* used in mac.c */
#define Screen_Border	4
#define	TOP_OFFSET		30
#define	LEFT_OFFSET	10

/* for macflags variable */
#define	fZoomOnContextSwitch		0x200
#define	fUseCustomFont		0x100
#define	fToggleNumPad		0x80
#define	fInvertedScreen		0x40
#define	fExtCmdSeq1			0x20
#define	fExtCmdSeq2			0x10
#define	fExtCmdSeq3			0x08
#define	fDoNonKeyEvt		0x02
#define	fDoUpdate			0x01


#define	CREATOR	'nh30'
#define	EXPLORE_TYPE	'XPLR'
#define	SAVE_TYPE	'SAVE'
#define	BONES_TYPE	'BONE'
#define	LEVEL_TYPE	'LEVL'
#define	HACK_DATA	'HDTA'
#define MONST_DATA	101
#define OBJECT_DATA	104
#define	DEFAULT_DATA	100

#include "extern.h"

#endif /* MACCONF_H /* */
#endif /* MACOS / */
