/*	SCCS Id: @(#)ntconf.h	3.2	96/10/14	*/
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

#include <string.h>     /* Provides prototypes of strncmpi(), etc.     */
#ifdef STRNCMPI
#define strncmpi(a,b,c) strnicmp(a,b,c)
#endif

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
#define regularize	nt_regularize

#ifndef M
#define M(c)		(0x80 | (c))
/* #define M(c)		((c) - 128) */
#endif

#ifndef C
#define C(c)		(0x1f & (c))
#endif

#if defined(DLB)
#define FILENAME_CMP  stricmp                 /* case insensitive */
#endif

#ifdef MICRO
# ifndef MICRO_H
#include "micro.h"      /* contains necessary externs for [os_name].c */
# endif
#endif

#include <fcntl.h>
#include <io.h>
#include <direct.h>
#include <conio.h>
#undef kbhit	        /* Use our special NT kbhit */
#define kbhit (*nt_kbhit)

#ifndef alloca
#define ALLOCA_HACK	/* used in util/panic.c */
#endif

#ifndef REDO
#undef	Getchar
#define Getchar nhgetch
#endif

#ifdef _MSC_VER
#pragma warning(disable:4018)	/* signed/unsigned mismatch */
#pragma warning(disable:4305)	/* init, conv from 'const int' to 'char' */
#endif

#endif /* NTCONF_H */
