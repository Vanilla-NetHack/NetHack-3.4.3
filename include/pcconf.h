/*	SCCS Id: @(#)pcconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef MSDOS
#ifndef PCCONF_H
#define PCCONF_H

/* #define OS2		/* define for OS/2 (Timo Hakulinen) */
/* #define OS2_CODEVIEW /* define for OS/2 CodeView debugger,
			   otherwise path searches may fail (TH) */

/*
 *  The following options are configurable:
 */

#define DGK			/* MS DOS specific enhancements by dgk */

#define TERMLIB		/* enable use of termcap file /etc/termcap */
			/* or ./termcap for MSDOS (SAC) */
			/* compile and link in Fred Fish's termcap library, */
			/* enclosed in TERMCAP.ARC, to use this */
#define ANSI_DEFAULT	/* allows NetHack to run without a ./termcap */

#define RANDOM		/* have Berkeley random(3) */

#define PATHLEN		64	/* maximum pathlength */
#define FILENAME	80	/* maximum filename length (conservative) */
#ifndef MSDOS_H
#include "msdos.h"	/* contains necessary externs for [os_name].c */
#endif
#define glo(x)	name_file(lock, (int)x)	/* name_file used for bones */
extern const char *configfile;
/*#define SHELL			/* via exec of COMMAND.COM */

#ifdef DGK
/*	Selectable DGK options:
 */
/*#define DECRAINBOW	/* enable use of DEC Rainbow graphics */

/*	Non-Selectable DGK options:
 */
# define FROMPERM	 1	/* for ramdisk use */
# define TOPERM		 2	/* for ramdisk use */

#endif /* DGK /**/

/*
 *  The remaining code shouldn't need modification.
 */

#ifndef SYSTEM_H
#include "system.h"
#endif
#define index	strchr
#define rindex	strrchr

#include <time.h>

#ifdef RANDOM
/* Use the high quality random number routines. */
#define Rand()	random()
#define Srand(seed)	srandom(seed)
#else
#define Rand()	rand()
#define Srand(seed)	srand(seed)
#endif /* RANDOM */

#ifdef __TURBOC__
#define	alloc	malloc
# if __TURBOC__ < 0x0200 /* version 2.0 has signal() */
#define	signal	ssignal
# endif
/* rename the next two functions - they clash with the Turbo C library */
#define getdate getdate_
#define itoa itoa_
#endif

#ifndef TOS
#define	FCMASK	0660	/* file creation mask */
#endif

#include <fcntl.h>

#define	exit	msexit		/* do chdir first */

#ifndef REDO
#undef	Getchar
#define Getchar tgetch
#endif

#ifndef TOS
#  define TEXTCOLOR /* */
#endif

#ifdef TEXTCOLOR
/* configurable colors */
#  define HI_MON	RED	/* red slaps! */
#  define HI_OBJ	MAGENTA
#  define HI_METAL	CYAN
#  define HI_ORGANIC	GREEN
#  define HI_GOLD	YELLOW
#  define HI_ZAP	BLUE	/* blue zaps! */
#endif

#ifndef EXTERN_H
#include "extern.h"
#endif

#endif /* PCCONF_H /* */
#endif /* MSDOS /* */
