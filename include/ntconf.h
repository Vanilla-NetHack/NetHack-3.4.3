/*	SCCS Id: @(#)ntconf.h	3.1	93/04/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef NTCONF_H
#define NTCONF_H

#define MICRO		/* always define this! */

#define SHELL

#define RANDOM		/* have Berkeley random(3) */

#define TEXTCOLOR	/* Color text */

#define PATHLEN		64	/* maximum pathlength */
#define FILENAME	80	/* maximum filename length (conservative) */
#ifndef MICRO_H
#include "micro.h"      /* contains necessary externs for [os_name].c */
#endif


/*
 *  The remaining code shouldn't need modification.
 */

#define NO_TERMS       /* April 8/93 mja */
#define ASCIIGRAPH
 
#ifndef SYSTEM_H
#include "system.h"
#endif
#define index	strchr
#define rindex	strrchr

#include <time.h>

#ifdef RANDOM
/* Use the high quality random number routines. */
#define Rand()	random()
#else
#define Rand()	rand()
#endif

#define FCMASK	0660	/* file creation mask */

#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <conio.h>
#undef kbhit	        /* Use our special NT kbhit */

#define exit	msexit		/* do chdir first */

#ifndef REDO
#undef	Getchar
#define Getchar nhgetch
#endif


#endif /* NTCONF_H */
