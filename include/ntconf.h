/*	SCCS Id: @(#)ntconf.h	3.3	96/10/14	*/
/* Copyright (c) NetHack PC Development Team 1993, 1994.  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef NTCONF_H
#define NTCONF_H

/* #define SHELL	/* nt use of pcsys routines caused a hang */

#define RANDOM		/* have Berkeley random(3) */

#define TEXTCOLOR	/* Color text */

#define PATHLEN		64	/* maximum pathlength */
#define FILENAME	80	/* maximum filename length (conservative) */
#define EXEPATH			/* Allow .exe location to be used as HACKDIR */
#define TRADITIONAL_GLYPHMAP	/* Store glyph mappings at level change time */
#ifdef WIN32CON
#define LAN_FEATURES		/* Include code for lan-aware features. */
#endif

#define PC_LOCKING		/* Prevent overwrites of aborted or in-progress games */
				/* without first receiving confirmation. */

/*
 * -----------------------------------------------------------------
 *  The remaining code shouldn't need modification.
 * -----------------------------------------------------------------
 */
/* #define SHORT_FILENAMES	/* All NT filesystems support long names now */

#define MICRO		/* always define this! */
#define NO_TERMS
#define ASCIIGRAPH

/* The following is needed for prototypes of certain functions */
#if defined(_MSC_VER)
#include <process.h>	/* Provides prototypes of exit(), spawn()      */
#endif

#include <string.h>	/* Provides prototypes of strncmpi(), etc.     */
#ifdef STRNCMPI
#define strncmpi(a,b,c) strnicmp(a,b,c)
#endif

#include <sys/types.h>
#include <stdlib.h>

#define NO_SIGNAL
#define index	strchr
#define rindex	strrchr
#include <time.h>
#define USE_STDARG
#ifdef RANDOM
/* Use the high quality random number routines. */
#define Rand()	random()
#else
#define Rand()	rand()
#endif

#define FCMASK	0660	/* file creation mask */
#define regularize	nt_regularize
#define HLOCK "NHPERM"

#ifndef M
#define M(c)		((char) (0x80 | (c)))
/* #define M(c)		((c) - 128) */
#endif

#ifndef C
#define C(c)		(0x1f & (c))
#endif

#if defined(DLB)
#define FILENAME_CMP  stricmp		      /* case insensitive */
#endif

#ifdef MICRO
# ifndef MICRO_H
#include "micro.h"	/* contains necessary externs for [os_name].c */
# endif
#endif

#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <conio.h>
#undef kbhit		/* Use our special NT kbhit */
#define kbhit (*nt_kbhit)

#ifdef LAN_FEATURES
#define MAX_LAN_USERNAME 20
#define LAN_MAIL
#define LAN_RO_PLAYGROUND	/* not implemented in 3.3.0 */
#define LAN_SHARED_BONES	/* not implemented in 3.3.0 */
#include "nhlan.h"
#endif

#ifndef alloca
#define ALLOCA_HACK	/* used in util/panic.c */
#endif

#ifndef REDO
#undef	Getchar
#define Getchar nhgetch
#endif

#ifdef _MSC_VER
#if 0
#pragma warning(disable:4018)	/* signed/unsigned mismatch */
#pragma warning(disable:4305)	/* init, conv from 'const int' to 'char' */
#endif
#pragma warning(disable:4761)	/* integral size mismatch in arg; conv supp*/
#ifdef YYPREFIX
#pragma warning(disable:4102)	/* unreferenced label */
#endif
#endif

#endif /* NTCONF_H */
