/*	SCCS Id: @(#)unixconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef UNIX
#ifndef UNIXCONF_H
#define UNIXCONF_H

/*
 * Some include files are in a different place under SYSV
 * 	BSD		   SYSV
 * <sys/time.h>		<time.h>
 * <sgtty.h>		<termio.h>
 *
 * Some routines are called differently
 * index		strchr
 * rindex		strrchr
 *
 * Also, the code for suspend is only given for BSD
 */

/* #define BSD		/* define for 4.n BSD  */
			/* also for relatives like SunOS */
#define ULTRIX		/* define for Ultrix v3.0 or higher (but not lower) */
			/* Use BSD for < v3.0 */
			/* "ULTRIX" not to be confused with "ultrix" */
/* #define SYSV		/* define for System V */
/* #define NETWORK	/* if running on a networked system */
			/* e.g. Suns sharing a playground through NFS */

/* #define GENIX	/* Yet Another Unix Clone */
/* #define HISX		/* Bull Unix for XPS Machines */
/* #define UNIXPC	/* use in addition to SYSV for AT&T 7300/3B1 */
			/* also note that the stock cpp qualifies as a
			   STUPID_CPP for config.h */

/* #define PYRAMID_BUG 	/* avoid a bug on the Pyramid */
/* #define APOLLO	/* same for the Apollo */
/* #define MICROPORT_BUG /* problems with large arrays in structs */
/* #define MICROPORT_286_BUG /* Changes needed in termcap.c to get it to
			   run with Microport Sys V/AT version 2.4.
			   By Jay Maynard */

/* #define RANDOM	/* if neither random/srandom nor lrand48/srand48
			   is available from your system */
/* #define TEXTCOLOR	/* Use System V r3.2 terminfo color support */


/*
 * The next two defines are intended mainly for the Andrew File System,
 * which does not allow hard links.  If NO_FILE_LINKS is defined, lock files
 * will be created in LOCKDIR using open() instead of in the playground using
 * link().
 *		Ralf Brown, 7/26/89 (from v2.3 hack of 10/10/88)
 */

/* #define NO_FILE_LINKS	/* if no hard links */
/* #define LOCKDIR "/usr/games/lib/nethackdir"	/* where to put locks */


/*
 * Define DEF_PAGER as your default pager, e.g. "/bin/cat" or "/usr/ucb/more"
 * If defined, it can be overridden by the environment variable PAGER.
 * Hack will use its internal pager if DEF_PAGER is not defined.
 * (This might be preferable for security reasons.)
 * #define DEF_PAGER	".../mydir/mypager"
 */

/*
 * If you define MAIL, then the player will be notified of new mail
 * when it arrives. If you also define DEF_MAILREADER then this will
 * be the default mail reader, and can be overridden by the environment
 * variable MAILREADER; otherwise an internal pager will be used.
 * A stat system call is done on the mailbox every MAILCKFREQ moves.
 */

#define	MAIL		/* Deliver mail during the game */
#ifdef MAIL

# if defined(BSD) || defined(ULTRIX)
#define	DEF_MAILREADER	"/usr/ucb/Mail"
# else
#  if defined(SYSV) || defined(DGUX)
#   ifdef M_XENIX
#define	DEF_MAILREADER	"/usr/bin/mail"
#   else
#define	DEF_MAILREADER	"/usr/bin/mailx"
#   endif
#  else
#define	DEF_MAILREADER	"/bin/mail"
#  endif
# endif

#define	MAILCKFREQ	50
#endif	/* MAIL */

#ifdef COMPRESS
/* Some implementations of compress need a 'quiet' option.
 * If you've got one of these versions, put -q here.
 * You can also include any other strange options your compress needs.
 * If you have a normal compress, just leave it commented out.
 */
/* #define COMPRESS_OPTIONS	"-q"	/* */
#endif

#define	FCMASK	0660	/* file creation mask */


/*
 * The remainder of the file should not need to be changed.
 */

#if (defined(BSD) || defined(ULTRIX)) && !defined(MSDOS)
#include	<sys/time.h>
#else
#include	<time.h>
#endif

#define	HLOCK	"perm"	/* an empty file used for locking purposes */
#define LLOCK	"safelock"	/* link to previous */

#ifndef REDO
#define Getchar getchar
#else
#define tgetch getchar
#endif

#define SHELL		/* do not delete the '!' command */

#include "system.h"

#if defined(BSD) || defined(ULTRIX)
#define	SUSPEND		/* let ^Z suspend the game */
#define memcpy(d, s, n)		bcopy(s, d, n)
#define memcmp(s1, s2, n)	bcmp(s2, s1, n)
#else	/* therefore SYSV */
#define index	strchr
#define rindex	strrchr
#endif

/* Use the high quality random number routines. */
#if defined(BSD) || defined(ULTRIX) || defined(RANDOM)
#define Rand()	random()
#define Srand(seed) srandom(seed)
#else
#define Rand()	lrand48()
#define Srand(seed) srand48(seed)
#endif

#ifdef hc	/* older versions of the MetaWare High-C compiler define this */
# ifdef __HC__
#  undef __HC__
# endif
# define __HC__ hc
# undef hc
#endif

#endif /* UNIXCONF_H /* */
#endif /* UNIX /* */
