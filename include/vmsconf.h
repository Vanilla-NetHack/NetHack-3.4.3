/*	SCCS Id: @(#)vmsconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef VMS
#ifndef VMSCONF_H
#define VMSCONF_H

#undef UNIX
#undef HACKDIR
#define HACKDIR	"USR$ROOT0:[GENTZEL.NHDIR]"
#undef RECORD
#define RECORD "record;1"

#undef COMPRESS
#undef ZEROCOMP

/*
 * If you define MAIL, then the player will be notified of new broadcasts
 * when they arrive.
 */
#define	MAIL

#define RANDOM		/* if neither random/srandom nor lrand48/srand48
			   is available from your system */

#define	FCMASK	0660	/* file creation mask */


/*
 * The remainder of the file should not need to be changed.
 */

#include <time.h>
#include <file.h>

#define	HLOCK	"perm;1"	/* an empty file used for locking purposes */
#define LLOCK	"safelock;1"	/* link to previous */

#ifndef REDO
# define Getchar vms_getchar
#else
# define tgetch vms_getchar
#endif

#define SHELL		/* do not delete the '!' command */

#include "system.h"

#define index	strchr
#define rindex	strrchr

/* Use the high quality random number routines. */
#if defined(RANDOM)
#define Rand()	random()
#define Srand(seed) srandom(seed)
#else
#define Rand()	rand()
#define Srand(seed) srand(seed)
#endif

#define unlink(x) delete(x)
#define exit(x) vms_exit(x)
#define getuid() vms_getuid()
#define abort() vms_abort()
#define creat(f,m) vms_creat(f,m)

#endif
#endif /* VMS /* */
