/*	SCCS Id: @(#)tosconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef TOS
#ifndef TOSCONF_H
#define TOSCONF_H

#define MSDOS		/* must be defined to allow some inclusions */

/*
   Adjust these options to suit your compiler. The default here is for
   GNU C with the latest version of the library.
*/

/*#define NO_SIGNAL 		/* library doesn't support signals	*/
/*#define NO_FSTAT		/* library doesn't have fstat() call	*/

#ifdef O_BINARY
#define FCMASK 	O_BINARY
#else
#define FCMASK	0666
#define O_BINARY 0
#endif

#ifdef UNIXDEBUG
#define remove(x)	unlink(x)
#endif

/* configurable options */
#define DGK			/* lots of enhancements 	*/
#define RANDOM			/* improved random numbers 	*/
#define SHELL			/* allow spawning of shell 	*/
#define TEXTCOLOR		/* allow color 			*/
#define TERMLIB			/* use termcap			*/

#ifndef TERMLIB
#define ANSI_DEFAULT		/* use vt52 by default		*/
#endif

#ifndef MSDOS_H
#include "msdos.h"
#endif
#ifndef PCCONF_H
#include "pcconf.h"	 	 /* remainder of stuff is same as the PC */
#endif

#endif /* TOSCONF_H /* */
#endif /* TOS /* */
