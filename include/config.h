/*	SCCS Id: @(#)config.h	3.0	89/06/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define	CONFIG_H


/*
 * Section 1:	OS selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select either SYSV or BSD in unixconf.h.
 *		A "VMS" option is not needed since the VMS C-compiler
 *		provides it (comment out the default OS option here).
 */

#define UNIX		/* delete if no fork(), exec() available */

#ifdef __MSDOS__	/* Turbo C auto-defines __MSDOS__, MSC defines MSDOS */
#define MSDOS		/* define for MS-DOS (in case compiler doesn't) */
#else
/* #define MSDOS	/* define for MS-DOS and most other micros */
			/* DO NOT define for AMIGA - MSDOS will be
			 * automatically defined at the right place. */
# ifdef AZTEC_C 	/* Manx 3.6 auto-defines this */
#  ifdef MCH_AMIGA	/* Manx 3.6 auto-defines this for AMIGA */
#   ifndef AMIGA
#define AMIGA		/* define for Commodore-Amiga */
#   endif		/* (Lattice auto-defines AMIGA) */
#  endif
# endif
/* #define TOS		/* define for Atari 1040ST */

/* #define STUPID	/* avoid some complicated expressions if
			   your C compiler chokes on them */
/* #define STUPID_CPP	/* use many small functions instead of macros to
			   avoid overloading limited preprocessors */
/* #define TERMINFO	/* uses terminfo rather than termcap */
			/* should be defined for most, but not all, SYSV */
			/* in particular, it should NOT be defined for the
			 * UNIXPC unless you remove the use of the shared
			 * library in the makefile */
/* #define MINIMAL_TERM	/* if a terminal handles highlighting or tabs poorly,
			   try this define, used in pager.c and termcap.c */
/* #define MACOS 	/* define for Apple Macintosh */
#endif

#ifdef AMIGA
#define NEED_VARARGS
# ifdef AZTEC_C
#  define KR1ED		/* Aztec 3.6 needs extra help for defined() */
# endif
#endif

#ifdef MACOS
/* #define KR1ED	/* for compilers which can't handle defined() */
			/* Lightspeed & Aztec can't handle defined() yet */
/* #define LSC		/* for the Lightspeed 3.01p4 C compiler on the Mac */
/* #define AZTEC	/* for the Manx Aztec C 3.6c compiler */
#define THINKC4	/* for the Think C 4 compiler */
/* #define MAKEDEFS_C	/* uncomment this ONLY while compiling makedefs */
/* #define CUSTOM_IO	/* uncomment only while compiling Nethack */
# ifndef MAKEDEFS_C
#  ifndef NEED_VARARGS
#define NEED_VARARGS	/* if you're using precompiled headers */
#  endif
# endif
#define SMALLDATA	/* for Mac compilers with 32K global data limit */
#endif


#ifdef KR1ED		/* For compilers which cannot handle defined() */
#define defined(x) (-x-1 != -1)
/* Because:
 * #define FOO => FOO={} => defined( ) => (-1 != - - 1) => 1
 * #define FOO 1 or on command-line -DFOO
 *      => defined(1) => (-1 != - 1 - 1) => 1
 * if FOO isn't defined, FOO=0. But some compilers default to 0 instead of 1
 * for -DFOO, oh well.
 *      => defined(0) => (-1 != - 0 - 1) => 0
 *
 * But:
 * defined("") => (-1 != - "" - 1)
 *   [which is an unavoidable catastrophe.]
 */
#endif



/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, or NEWS removes that feature
 *		from the game; otherwise set the appropriate wizard name.
 *		LOGFILE and NEWS refer to files in the playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
# ifndef KR1ED
#define WIZARD  "izchak" /* the person allowed to use the -D option */
# else
#define WIZARD
#define WIZARD_NAME "johnny"
# endif
#endif

#define LOGFILE "logfile"	/* larger file for debugging purposes */
#define NEWS "news"		/* the file containing the latest hack news */

/*
 *	If COMPRESS is defined, it should contain the full path name of your
 *	'compress' program.  Defining ZEROCOMP causes NetHack to do simpler
 *	zero-run compression internally.  Both COMPRESS and ZEROCOMP create
 *	smaller bones/level/save files, but require additional code and time.
 */

#ifndef MACOS
#define COMPRESS "/usr/local/compress"  /* path name for 'compress' */
# ifndef COMPRESS
#define ZEROCOMP	/* Use only if COMPRESS is not used -- Olaf Seibert */
# endif
#endif

#ifndef MACOS
#define CHDIR		/* delete if no chdir() available */
#endif

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
#define HACKDIR "/usr/games/lib/nethackdir" 	/* nethack directory */

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files.  In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
/* #define SECURE	/* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS.
 * #define MAX_NR_OF_PLAYERS 6
 */
#endif /* CHDIR /**/



/*
 * Section 3:	Definitions that may vary with system type.
 *		For example, both schar and uchar should be short ints on
 *		the AT&T 3B2/3B5/etc. family.
 */

/*
 * Uncomment the following line if your compiler doesn't understand the
 * 'void' type (and thus would give all sorts of compile errors without
 * this definition).
 */
/* #define void int			/* define if no "void" data type. */

#include "tradstdc.h"

/*
 * type schar: small signed integers (8 bits suffice) (eg. TOS)
 *
 *	typedef char	schar;
 *
 *      will do when you have signed characters; otherwise use
 *
 *	typedef short int schar;
 */
#ifdef AZTEC
#define schar	char
#else
typedef signed char	schar;
#endif

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 *
 *	typedef unsigned char	uchar;
 *
 *	will be satisfactory if you have an "unsigned char" type;
 *	otherwise use
 *
 *	typedef unsigned short int uchar;
 */
typedef unsigned char	uchar;

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS	/* Good bitfield handling */



/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */ 

/* game features */
#define POLYSELF      /* Polymorph self code by Ken Arromdee */
#define THEOLOGY      /* Smarter gods - The Unknown Hacker */
#define SOUNDS        /* Add more life to the dungeon */
#define KICK          /* Allow kicking things besides doors -Izchak Miller */
/* dungeon features */
#define THRONES       /* Thrones and Courts by M. Stephenson */
#define FOUNTAINS     /* Fountain code by SRT (+ GAN + EB) */
#define SINKS         /* Kitchen sinks - Janet Walz */
#define ALTARS        /* Sacrifice sites - Jean-Christophe Collet */
/* dungeon levels */
#define WALLIFIED_MAZE /* Fancy mazes - Jean-Christophe Collet */
#define REINCARNATION /* Rogue-like levels */
#define STRONGHOLD    /* Challenging special levels - Jean-Christophe Collet*/
/* monsters & objects */
#define ORACLE        /* Include another source of information */
#define MEDUSA        /* Mirrors and the Medusa by Richard P. Hughey */
#define KOPS          /* Keystone Kops by Scott R. Turner */
#define ARMY          /* Soldiers, barracks by Steve Creps */
#define WORM          /* Long worms */
#define GOLEMS        /* Golems, by KAA */
#define INFERNO       /* Demons & Demonlords */
#ifdef INFERNO
#define SEDUCE        /* Succubi/incubi additions, by KAA, suggested by IM */
#endif
#define TOLKIEN       /* More varieties of objects and monsters */
#define PROBING       /* Wand of probing code by Gil Neiger */
#define WALKIES       /* Leash code by M. Stephenson */
#define SHIRT         /* Hawaiian shirt code by Steve Linhart */
#define MUSIC         /* Musical instruments - Jean-Christophe Collet */
#define TUTTI_FRUTTI  /* Fruits as in Rogue, but which work... -KAA */
#define SPELLS        /* Spell casting by M. Stephenson */
#define NAMED_ITEMS   /* Special named items handling */
/* difficulty */
#define ELBERETH      /* Allow for disabling the E word - Mike 3point */
#define EXPLORE_MODE  /* Allow non-scoring play with additional powers */
#define HARD          /* Enhanced wizard code by M. Stephenson */
/* I/O */
#define REDO          /* support for redoing last command - DGK */
#define COM_COMPL     /* Command line completion by John S. Bien */
#ifndef AMIGA
#define CLIPPING      /* allow smaller screens -- ERS */
#endif

#ifdef REDO
#define DOAGAIN '\001'		/* The "redo" key used in tty.c and cmd.c */
#endif

#define EXP_ON_BOTL	/* Show experience on bottom line */
/* #define SCORE_ON_BOTL	/* added by Gary Erickson (erickson@ucivax) */



#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H /**/
