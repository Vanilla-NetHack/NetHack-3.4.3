/*	SCCS Id: @(#)vmsconf.h	3.0	88/07/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef VMS
#ifndef VMSCONF_H
#define VMSCONF_H

/*
 * Edit these to choose values appropriate for your site.
 * WIZARD is the username allowed to use the debug option of nethack; no harm
 *   is done by leaving it as a username that doesn't exist at your site.
 * HACKDIR can be overridden at run-time with the logical name HACKDIR, as in
 *   $ define hackdir disk$users:[games.nethack]
 */
#define Local_WIZARD    "GENTZEL"               /*(must be uppercase)*/
#define Local_HACKDIR   "USR$ROOT0:[GENTZEL.NHDIR]"

/*
 * This section cleans up the stuff done in config.h so that it
 * shouldn't need to be modified.  It's conservative so that if
 * config.h is actually edited, the changes won't impact us.
 */
#ifdef UNIX
# undef UNIX
#endif
#ifdef HACKDIR
# undef HACKDIR
#endif
#ifdef WIZARD
# undef WIZARD
#endif
#ifdef WIZARD_NAME
# undef WIZARD_NAME
#endif
#define HACKDIR Local_HACKDIR
#ifndef KR1ED
# define WIZARD Local_WIZARD
# define WIZARD_NAME WIZARD
#else
# define WIZARD 1
# define WIZARD_NAME Local_WIZARD
#endif

/* filenames require punctuation to avoid redirection via logical names */
#undef RECORD
#define RECORD	"record;1"	/* scoreboard file (retains high scores) */
#undef LOGFILE
#define LOGFILE	"logfile;0"	/* optional file (records all games) */

#define HLOCK	"perm;1"	/* an empty file used for locking purposes */
#define LLOCK	"perm.lock;1"	/* temporary link to previous */

/* want compression--for level & save files--performed within NetHack itself */
#ifdef COMPRESS
# undef COMPRESS
#endif
#ifndef ZEROCOMP
# define ZEROCOMP
#endif

/*
 * If nethack.exe will be installed with privilege so that the playground
 * won't need to be left unprotected, define SECURE to suppress a couple
 * of file protection fixups (protection of bones files and ownership of
 * save files).
 */
/* #define SECURE /**/

/*
 * If you define MAIL, then the player will be notified of new broadcasts
 * when they arrive.
 */
#define MAIL

#define RANDOM		/* use others/random.c instead of vaxcrtl rand/srand */

#define FCMASK	0660	/* file creation mask */

/* vaxcrtl object library is not available on MicroVMS (4.4 thru 4.6(.7?))
   unless it's retreived from a full VMS system or leftover from a really
   ancient version of VAXC.  #define no_c$$translate and also create a
   linker options file called vaxcrtl.opt containing one line
sys$share:vaxcrtl/shareable
   to link against the vaxcrtl shareable image.  Then include ',vaxcrtl/opt'
   on the link command instead of either ',sys$library:vaxcrtl/lib' or
   '$ define lnk$library sys$library:vaxcrtl'
   Linking against the vaxcrtl sharable image is encouraged and will result
   in significantly smaller .EXE files.  The routine C$$TRANSLATE (used in
   vmsunix.c) is not available from the sharable image version of vaxcrtl.
 */
/* #define no_c$$translate /**/

/*
 * The remainder of the file should not need to be changed.
 */

/* GCC 1.36 (or maybe GAS) for VMS has a bug with extern const declarations.
   Until it is fixed, eliminate const. */
#ifdef __GNUC__
# define const
#endif
#if defined(VAXC) && !defined(ANCIENT_VAXC)
# ifdef volatile
#  undef volatile
# endif
# ifdef const
#  undef const
# endif
#endif

#include <time.h>
#ifndef __GNUC__
#include <file.h>
#else	/* values needed from missing include file */
# define O_RDONLY 0
# define O_WRONLY 1
# define O_RDWR   2
# define O_CREAT 0x200
#endif /* __GNUC__ */

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

#define bcopy(s1,s2,sz) memcpy(s2,s1,sz)
#define unlink(x) delete(x)
#define exit(x) vms_exit(x)
#define getuid() vms_getuid()
#define abort() vms_abort()
#define creat(f,m) vms_creat(f,m)

/* VMS global names are case insensitive... */
#define An vms_an

#endif
#endif /* VMS /* */
