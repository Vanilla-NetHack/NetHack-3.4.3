/*	SCCS Id: @(#)amiconf.h	3.1	93/01/17	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1990, 1991, 1992, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef AMICONF_H
#define AMICONF_H

#undef abs		/* avoid using macro form of abs */
#undef min		/* this gets redefined */
#undef max		/* this gets redefined */

#include <time.h>	/* get time_t defined before use! */

#ifdef LATTICE		/* since Lattice can prevent re-inclusion */
# include <stdlib.h>	/* and other things, including builtins */
#endif

#ifdef AZTEC_50
# include <stdlib.h>
# define AZTEC_C_WORKAROUND /* Bug which turns up in sounds.c. Bummer... */
# define NO_SIGNAL	/* 5.0 signal handling doesn't like SIGINT...   */
#endif

#ifdef LATTICE		/* Lattice defines DEBUG when you use -d1 which */
# ifdef DEBUG		/* we need for useful SnapShots, but DEBUG is   */
#  undef DEBUG		/* used in several files to turn on things we   */
# endif			/* don't want (e.g. eat.c), so we get rid of    */
#endif			/* DEBUG unless asked for in a particular file  */

#ifdef _DCC
# include <stdlib.h>
# define _SIZE_T
#endif

typedef long off_t;

#define MICRO		/* must be defined to allow some inclusions */

#ifndef	__SASC_60
# define O_BINARY	0
#endif

/* Compile in New Intuition look for 2.0 */
#ifdef	IDCMP_CLOSEWINDOW
# define	INTUI_NEW_LOOK	1
#endif

#define MFLOPPY         /* You'll probably want this; provides assistance
			 * for typical personal computer configurations
			 */
#define RANDOM

extern void FDECL(exit, (int));
extern void NDECL(CleanUp);
extern void FDECL(Abort, (long));
extern int NDECL(getpid);
extern char *FDECL(CopyFile, (const char *, const char *));
extern int NDECL(WindowGetchar);

extern boolean FromWBench;	/* how were we run? */
extern int ami_argc;
extern char **ami_argv;

#ifndef MICRO_H
# include "micro.h"
#endif

#ifndef PCCONF_H
# include "pcconf.h"     /* remainder of stuff is almost same as the PC */
# undef	OVERLAY
#endif

#define remove(x)       unlink(x)

#ifdef LATTICE
#define FFLUSH(fp) _flsbf(-1, fp)    /* Was fflush */
#endif

#ifdef AZTEC_C
#define FFLUSH(fp) flsh_(fp, -1)     /* Was fflush */
extern FILE *FDECL(freopen, (const char *, const char *, FILE *));
extern char *FDECL(gets, (char *));
#endif

#ifdef _DCC
#define FFLUSH(fp) fflush(fp)
#endif

#define msmsg		raw_printf

/*
 * If AZTEC_C  we can't use the long cpath in vision.c....
 */
#ifdef AZTEC_C
# undef MACRO_CPATH
#endif

/*
 *  (Possibly) configurable Amiga options:
 */

#define TEXTCOLOR		/* Use colored monsters and objects */
#define HACKFONT		/* Use special hack.font */
#define SHELL			/* Have a shell escape command (!) */
#define MAIL			/* Get mail at unexpected occasions */
#define DEFAULT_ICON "NetHack:default.icon"	/* private icon */
#define AMIFLUSH		/* toss typeahead (select flush in .cnf) */
/*#define AMIGA_WINDOWED_CORNLINE /* Use windows for pager, inventory, etc */

/* new window system options */
#define AMIGA_INTUITION		/* high power graphics interface */

#ifdef	TEXTCOLOR
# define	DEPTH	3
#else
# define	DEPTH	2
#endif

#define PORT_HELP	"nethack:amii.hlp"

#undef	TERMLIB

#endif /* AMICONF_H */
