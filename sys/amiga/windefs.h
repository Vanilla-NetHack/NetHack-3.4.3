/*    SCCS Id: @(#)windefs.h    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

/* These should probably not even be options, but, I will leave them
 * for now... GGW
 */

#include "hack.h"
#include "wintype.h"
#include "winami.h"
#include "func_tab.h"

/*#define   TOPL_GETLINE	/* Don't use a window for getlin() */
/*#define   WINDOW_YN		/* Use a window for y/n questions */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <dos.h>
#include <exec/alerts.h>
#include <exec/devices.h>
#include <exec/execbase.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dosextens.h>
#include <ctype.h>
#undef  strcmpi
#include <string.h>
#include <errno.h>

#ifdef  IDCMP_CLOSEWINDOW
# ifndef	INTUI_NEW_LOOK
#  define	INTUI_NEW_LOOK
# endif
#endif

#ifdef AZTEC_C
#include <functions.h>
#else
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/console.h>
#ifdef	VIEWWINDOW
#include <proto/layers.h>
#endif
#include <proto/diskfont.h>

/* kludge - see amirip for why */
# undef red
# undef green
# undef blue
# include <proto/graphics.h>

#ifndef __SASC_60
#undef index
# define index strchr
#endif

#include <proto/intuition.h>
#endif

#ifdef	SHAREDLIB
#include "amiga:lib/libmacs.h"
#endif

#ifdef	INTUI_NEW_LOOK
#include <utility/tagitem.h>
#endif

/* cw->data[x] contains 2 characters worth of special information.  These
 * characters are stored at the offsets as described here.
 */
#define VATTR	  0	/* Video attribute is in this slot */
#define SEL_ITEM  1	/* If this is a select item, slot is 1 else 0 */
#define SOFF	  2	/* The string starts here.  */

/* Nethack defines NULL as ((char *)0) which is very inconvienent... */
#undef NULL
#define NULL 0L

/*
 * Versions we need of various libraries.  We can't use LIBRARY_VERSION
 * as defined in <exec/types.h> because some of the libraries we need
 * don't have that version number in the 1.2 ROM.
 */

#define INTUITION_VERSION	33L
#define GRAPHICS_VERSION	33L
#define DISKFONT_VERSION	34L
#define ICON_VERSION		34L

/* These values are just sorta suggestions in use, but are minimum requirements
 * in reality...
 */
#define WINDOWHEIGHT	192
#define SCREENHEIGHT	200
#define WIDTH		640

/* This character is a solid block (cursor) in Hack.font */
#define CURSOR_CHAR	0x90

#define FONTHEIGHT	8
#define FONTWIDTH	8
#define FONTBASELINE	8

#ifdef	VIEWWINDOW
#define MAPFTWIDTH	4
#define MAPFTHEIGHT	4
#define MAPFTBASELN	3
#define VIEWCHARWIDTH	16	/* Each square is 16 pixels wide */
#define VIEWCHARHEIGHT	16	/* Each square is 16 pixels tall */
#else
#define MAPFTWIDTH	8
#define MAPFTHEIGHT	8
#define MAPFTBASELN	6
#endif

/* If Compiling with the "New Look", redefine these now */
#ifdef  INTUI_NEW_LOOK
#define NewWindow ExtNewWindow
#define NewScreen ExtNewScreen
#endif

#define         SIZEOF_DISKNAME 8

#define CSI     '\x9b'
#define NO_CHAR     -1
#define RAWHELP     0x5F    /* Rawkey code of the HELP key */

#define C_BLACK		0
#define C_WHITE		1
#define C_BROWN		2
#define C_CYAN		3
#define C_GREEN		4
#define C_MAGENTA	5
#define C_BLUE		6
#define C_RED		7
#ifdef	VIEWWINDOW
#define C_VBLACK0	8
#define C_VBLACK1	9
#define C_VBLACK2	10
#define C_VBLACK3	11
#define C_VBLACK4	12
#define C_VBLACK5	13
#define C_VBLACK6	14
#define C_VBLACK7	15
#endif

/* topl kludges */
#define TOPL_NOSPACE	topl_addspace=0
#define TOPL_SPACE	topl_addspace=1
