/*	SCCS Id: @(#)tosconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef TOS
#ifndef TOSCONF_H
#define TOSCONF_H

#define MSDOS		/* must be defined to allow some inclusions */

#define FCMASK	0x8000

#ifdef UNIXDEBUG
#define O_BINARY	0
#define remove(x)	unlink(x)
#endif

#define Rand() rand()
#define Srand() srand()

#include "msdos.h"
#include "pcconf.h"	    /* remainder of stuff is same as the PC */

#endif /* TOSCONF_H /* */
#endif /* TOS /* */
