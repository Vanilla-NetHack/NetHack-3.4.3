/*	SCCS Id: @(#)tosconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef TOS
#ifndef TOSCONF_H
#define TOSCONF_H

#define MSDOS		/* must be defined to allow some inclusions */

/* NOTE: if your TOS compiler is dumb, #define OLD_TOS and compile with
   oldtos.c to get a minimal configuration (you may need to do some
   tweaking); otherwise (e.g. GCC) don't do it. OLD_TOS corresponds
   most closely to LATTICE C, I think */

/* #define OLD_TOS */
/* #define NO_SIGNAL */

#ifdef __GNUC__
#define FCMASK	0666
#define O_BINARY 0
#else
#define FCMASK	0x8000
#endif

#ifdef UNIXDEBUG
#define O_BINARY	0
#define remove(x)	unlink(x)
#endif

#ifdef OLD_TOS
#define Rand() rand()
#define Srand() srand()
#endif

/* configurable options */
#define DGK
#define RANDOM
#define SHELL
#define TEXTCOLOR
#define TERMLIB

#ifndef MSDOS_H
#include "msdos.h"
#endif
#ifndef PCCONF_H
#include "pcconf.h"	 	 /* remainder of stuff is same as the PC */
#endif
#ifdef TERMLIB
#undef ANSI_DEFAULT
#endif
#endif /* TOSCONF_H /* */
#endif /* TOS /* */
