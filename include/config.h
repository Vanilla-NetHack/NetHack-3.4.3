/*	SCCS Id: @(#)config.h	3.0	89/06/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define	CONFIG_H


/*
 * Section 1:	OS selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select either SYSV or BSD in unixconf.h
 */

#define	UNIX		/* delete if no fork(), exec() available */

#ifdef __MSDOS__	/* Turbo C auto-defines __MSDOS__, others MSDOS */
#define MSDOS	 	/* define for MS-DOS (actually defined by compiler) */
#else
/* # define MSDOS	/* define for MS-DOS and most other micros */
/* # define AMIGA	/* define for Commodore-Amiga */
/* #define TOS		/* for Atari 520/1040 */

/* #define STUPID	/* avoid some complicated expressions if
			   your C compiler chokes on them */
/* #define STUPID_CPP	/* use many small functions instead of macros to
			   avoid overloading limited preprocessors */
/* #define TERMINFO	/* uses terminfo rather than termcap */
			/* should be defined for most, but not all, SYSV */
/* #define MINIMAL_TERM	/* if a terminal handles highlighting or tabs poorly,
			   try this define, used in pager.c and termcap.c */
#endif



/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, or NEWS removes that feature
 *		from the game; otherwise set the appropriate wizard name.
 *		LOGFILE and NEWS refer to files in the playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
#define WIZARD  "izchak" /* the person allowed to use the -D option */
#endif

#define	LOGFILE	"logfile" /* larger file for debugging purposes */
#define	NEWS	"news"	  /* the file containing the latest hack news */


#define	CHDIR		/* delete if no chdir() available */

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
#define HACKDIR	"/usr/games/lib/nethackdir"

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files.  In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
/* #define SECURE			/* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS.
 * #define MAX_NR_OF_PLAYERS	6
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
 *	typedef	char	schar;
 *
 *      will do when you have signed characters; otherwise use
 *
 *	typedef	short int schar;
 */
typedef	signed char	schar;

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 *
 *	typedef	unsigned char	uchar;
 *
 *	will be satisfactory if you have an "unsigned char" type;
 *	otherwise use
 *
 *	typedef unsigned short int uchar;
 */
typedef	unsigned char	uchar;

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

#define	SPELLS		/* Spell casting by M. Stephenson */
#define POLYSELF	/* Polymorph self code by Ken Arromdee */
#define THRONES		/* Thrones and Courts by M. Stephenson */
#define	PROBING		/* Wand of probing code by Gil Neiger */
#define REDO 		/* support for redoing last command - DGK */
#define	HARD		/* Enhanced wizard code by M. Stephenson */
#define	WALKIES		/* Leash code by M. Stephenson */
#define FOUNTAINS	/* Fountain code by SRT (+ GAN + EB) */
#define KOPS		/* Keystone Kops by Scott R. Turner */
#define COM_COMPL	/* Command line completion by John S. Bien */
#define MEDUSA		/* Mirrors and the Medusa by Richard P. Hughey */
#define NAMED_ITEMS	/* Special named items handling */
#define ARMY		/* Soldiers, barracks by Steve Creps */
#define	SHIRT		/* Hawaiian shirt code by Steve Linhart */
#define THEOLOGY	/* Smarter gods - The Unknown Hacker */
#define SINKS		/* Kitchen sinks - Janet Walz */
#define COMPRESS "/usr/local/compress"	/* the location of 'compress' */
			/* Compressed bones / save files - Izchak Miller */
/* #define ZEROCOMP	/* Zero-run compression of files - Olaf Seibert */
			/* Use only if COMPRESS is not used */
#define SOUNDS		/* Add more life to the dungeon */
#define REINCARNATION	/* Rogue-like levels */
#define ELBERETH	/* Allow for disabling the E word - Mike 3point */
#define WORM		/* Long worms */
#define ORACLE		/* Include another source of information */
#define EXPLORE_MODE	/* Allow non-scoring play with additional powers */
#define ALTARS		/* Sacrifice sites - Jean-Christophe Collet */
#define WALLIFIED_MAZE	/* Fancy mazes - Jean-Christophe Collet */
#ifdef HARD
#define SEDUCE		/* Succubi/incubi additions, by KAA, suggested by IM */
#endif
#define STRONGHOLD	/* Challenging special levels - Jean-Christophe Collet*/
#define MUSIC		/* Musical instruments - Jean-Christophe Collet */
#define GOLEMS		/* Golems, by KAA */
#define TOLKIEN		/* More varieties of objects and monsters */
#define KICK		/* Allow kicking things besides doors -Izchak Miller */

#ifdef REDO
#define DOAGAIN	'\001'		/* The "redo" key Used in tty.c and cmd.c */
#endif

#define	EXP_ON_BOTL	/* Show experience on bottom line */
/* #define SCORE_ON_BOTL	/* added by Gary Erickson (erickson@ucivax) */



#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H /**/
