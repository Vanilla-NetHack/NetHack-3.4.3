/*	SCCS Id: @(#)amiconf.h	3.1	93/01/17	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1990, 1991, 1992, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef AMICONF_H
#define AMICONF_H

#undef abs		/* avoid using macro form of abs */
#ifndef __SASC_60
# undef min		/* this gets redefined */
# undef max		/* this gets redefined */
#endif

#include <time.h>	/* get time_t defined before use! */

#ifdef __SASC_60	/* since SAS can prevent re-inclusion */
# include <stdlib.h>	/* general things, including builtins */
# include <string.h>
#endif

#ifdef AZTEC_50
# include <stdlib.h>
# define AZTEC_C_WORKAROUND /* Bug which turns up in sounds.c. Bummer... */
# define NO_SIGNAL	/* 5.0 signal handling doesn't like SIGINT...   */
#endif

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

#ifdef AZTEC_C
extern FILE *FDECL(freopen, (const char *, const char *, FILE *));
extern char *FDECL(gets, (char *));
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
#define AMIGA_INTUITION		/* high power graphics interface (amii) */

#ifdef	TEXTCOLOR
# define	DEPTH	3
#else
# define	DEPTH	2
#endif

#define PORT_HELP	"nethack:amii.hlp"

#undef	TERMLIB

#define	AMII_MUFFLED_VOLUME	40
#define	AMII_SOFT_VOLUME	50
#define	AMII_OKAY_VOLUME	60
#define	AMII_LOUDER_VOLUME	80

#endif /* AMICONF_H */
