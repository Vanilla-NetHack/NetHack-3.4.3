/*	SCCS Id: @(#)global.h	3.0	89/07/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include <stdio.h>


/* #define BETA		/* if a beta-test copy  [MRS] */
#define VERSION	"3.0f"	/* version number. */

/*
 * Files expected to exist in the playground directory.
 */

#define RECORD	"record"  /* the file containing the list of topscorers */
#define	HELP	"help"	  /* the file containing command descriptions */
#define	SHELP	"hh"	  /* abbreviated form of the same */
#define	RUMORFILE	"rumors"	/* a file with fortune cookies */
#define ORACLEFILE	"oracles"	/* a file with oracular information */
#define	DATAFILE	"data"	/* a file giving the meaning of symbols used */
#define CMDHELPFILE	"cmdhelp"	/* file telling what commands do */
#define HISTORY		"history"	/* a file giving nethack's history */
#define LICENSE		"license"	/* file with license information */
#define OPTIONFILE	"opthelp"	/* a file explaining runtime options */



/* Assorted definitions that may depend on selections in config.h. */

/*
 * for DUMB preprocessor and compiler, e.g., cpp and pcc supplied
 * with Microport SysV/AT, which have small symbol tables;
 * DUMB if needed is defined in CFLAGS
 */
#ifdef DUMB
#ifdef BITFIELDS
#undef BITFIELDS
#endif
#ifndef STUPID
#define STUPID
#endif
#ifndef STUPID_CPP
#define STUPID_CPP
#endif
#endif	/* DUMB */

/*
 * type xchar: small integers in the range 0 - 127, usually coordinates
 * although they are nonnegative they must not be declared unsigned
 * since otherwise comparisons with signed quantities are done incorrectly
 */
typedef schar	xchar;
typedef	xchar	boolean;		/* 0 or 1 */

#define	TRUE	((boolean)1)
#define	FALSE	((boolean)0)

#ifdef BITFIELDS
#define	Bitfield(x,n)	unsigned x:n
#else
#define	Bitfield(x,n)	uchar x
#endif

/*
 * According to ANSI, prototypes for old-style declarations must widen the
 * arguments to int.  However, the MSDOS compilers accept shorter arguments
 * (char, short, etc.) in prototypes and do typechecking with them.  Therefore
 * this mess to allow the better typechecking while also allowing some
 * prototypes for the ANSI compilers so people quit trying to fix the prototypes
 * to match the standard and thus lose the typechecking.
 */
#if defined(MSDOS) && !(defined(AMIGA) || defined(TOS))
# define CHAR_P char
# define SCHAR_P schar
# define UCHAR_P uchar
# define XCHAR_P xchar
# define BOOLEAN_P boolean
#else
# ifdef __STDC__
#  define CHAR_P int
#  define SCHAR_P int
#  define UCHAR_P int
#  define XCHAR_P int
#  define BOOLEAN_P int
# endif
#endif


#define	SIZE(x)	(int)(sizeof(x) / sizeof(x[0]))

/* (No, LARGEST_INT doesn't have to correspond to the largest integer on
 * a particular machine.)
 */
#define LARGEST_INT	((1 << 15) - 1)


#ifdef STRONGHOLD
# ifdef ALTARS
#  ifdef THEOLOGY
#define ENDGAME
#  endif
# endif
#endif

#ifdef REDO
#define Getchar pgetchar
#endif

/*
 * Automatic inclusions for the subsidiary files.
 * Please don't change the order.  It does matter.
 */

#ifndef COORD_H
#include "coord.h"
#endif
#if defined(UNIX) && !defined(UNIXCONF_H)
# include "unixconf.h"
#endif

#if defined(MSDOS) && !defined(PCCONF_H)
# include "pcconf.h"
#endif

#if defined(TOS) && !defined(TOSCONF_H)
# include "tosconf.h"
#endif

#if defined(AMIGA) && !defined(AMICONF_H)
# include "amiconf.h"
#endif



/*
 * Configurable internal parameters.
 *
 * Please be very careful if you are going to change one of these.  Any
 * changes in these parameters, unless properly done, can render the
 * executable inoperative.
 */

/* size of terminal screen is (at least) (ROWNO+3) by COLNO */
#define	COLNO	80
#define	ROWNO	21

#define	MAXNROFROOMS	20	/* max number of rooms per level */
#define	DOORMAX		120	/* max number of doors per level */

#define	BUFSZ		256	/* for getlin buffers */

#define	PL_NSIZ		32	/* name of player, ghost, shopkeeper */
#define	PL_CSIZ		20	/* sizeof pl_character */
#define PL_FSIZ		32	/* fruit name */

#define	MAXLEVEL	50	/* max number of levels in the dungeon */
#ifdef ENDGAME
#define ENDLEVEL (MAXLEVEL+1)	/* endgame level */
#endif
#define HELLLEVEL	30	/* first hell level (varies ifdef STRONGHOLD) */
#define	MAXULEV		30	/* max character experience level */

#define	MAXMONNO	120	/* geno monst after this number killed */
#define MHPMAX		500	/* maximum monster hp */

#endif /* GLOBAL_H /**/
