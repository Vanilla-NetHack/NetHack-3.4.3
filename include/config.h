/*	SCCS Id: @(#)config.h	3.2	96/01/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define CONFIG_H


/*
 * Section 1:	OS selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select BSD, ULTRIX, SYSV, or HPUX in unixconf.h.
 *		A "VMS" option is not needed since the VMS C-compilers
 *		provide it (no need to change sec#1, vmsconf.h handles it).
 */

#define UNIX		/* delete if no fork(), exec() available */

/*
 * MS DOS - compilers
 *
 * Microsoft C auto-defines MSDOS,
 * Borland C   auto-defines __MSDOS__,
 * DJGPP       auto-defines MSDOS.
 */

/* #define MSDOS	/* use if not defined by compiler or cases below */

#ifdef __MSDOS__	/* for Borland C */
# ifndef MSDOS
# define MSDOS
# endif
#endif

#ifdef __TURBOC__
# define __MSC		/* increase Borland C compatibility in libraries */
#endif

#ifdef MSDOS
# undef UNIX
#endif

/*
 * Mac Stuff.
 */
#ifdef applec		/*	MPW auto-defined symbol */
# define MAC
# undef UNIX
#endif

#ifdef THINK_C		/* Think C auto-defined symbol */
# define MAC
# define NEED_VARARGS
# undef UNIX
#endif

#ifdef __MWERKS__	/* defined by Metrowerks compiler */
# define MAC
# define NEED_VARARGS
# define USE_STDARGS
# undef UNIX
#endif


/*
 * Amiga setup.
 */
#ifdef AZTEC_C	/* Manx auto-defines this */
# ifdef MCH_AMIGA	/* Manx auto-defines this for AMIGA */
#  ifndef AMIGA
#define AMIGA		/* define for Commodore-Amiga */
#  endif		/* (SAS/C auto-defines AMIGA) */
#define AZTEC_50	/* define for version 5.0 of manx */
# endif
#endif
#ifdef __SASC_60
# define NEARDATA __near /* put some data close */
#else
# ifdef _DCC
# define NEARDATA __near /* put some data close */
# else
# define NEARDATA
# endif
#endif
#ifdef AMIGA
# define NEED_VARARGS
# undef	UNIX
# define DLB
# define HACKDIR "NetHack:"
# define NO_MACRO_CPATH
#endif

/*
 * Atari auto-detection
 */

#ifdef atarist
# undef UNIX
# define TOS
#else
# ifdef __MINT__
#  undef UNIX
#  define TOS
# endif
#endif

/*
 * Windows NT Autodetection
 */

#ifdef WIN32
# undef UNIX
# undef MSDOS
#endif

/*
 * and other systems...
 */

/* #define OS2		/* define for OS/2 */

/* #define TOS		/* define for Atari ST/TT */

/* #define STUPID	/* avoid some complicated expressions if
			   your C compiler chokes on them */
/* #define TERMINFO	/* uses terminfo rather than termcap */
			/* should be defined for HPUX and most, but not all,
			   SYSV */
			/* in particular, it should NOT be defined for the
			 * UNIXPC unless you remove the use of the shared
			 * library in the makefile */
/* #define MINIMAL_TERM	/* if a terminal handles highlighting or tabs poorly,
			   try this define, used in pager.c and termcap.c */
/* #define ULTRIX_CC20	/* define only if using cc v2.0 on a DECstation */
/* #define ULTRIX_PROTO	/* define for Ultrix 4.0 (or higher) on a DECstation;
			 * if you get compiler errors, don't define this. */
			/* Hint: if you're not developing code, don't define
			   ULTRIX_PROTO. */

#ifdef VMS	/* really old compilers need special handling, detected here */
# undef UNIX
# ifdef __DECC
#  ifndef __DECC_VER	/* buggy early versions want widened prototypes	*/
#   define NOTSTDC	/* except when typedefs are involved		*/
#   define USE_VARARGS
#  else
#   define NHSTDC
#   define USE_STDARG
#   define POSIX_TYPES
#   define _DECC_V4_SOURCE	/* avoid some incompatible V5.x changes */
#  endif
#  undef __HIDE_FORBIDDEN_NAMES	/* need non-ANSI library support functions */
# else
#  ifdef VAXC	/* must use CC/DEFINE=ANCIENT_VAXC for vaxc v2.2 or older */
#   ifdef ANCIENT_VAXC	/* vaxc v2.2 and earlier [lots of warnings to come] */
#    define KR1ED	/* simulate defined() */
#    define USE_VARARGS
#   else		/* vaxc v2.3,2.4,or 3.x, or decc in vaxc mode */
#     if defined(USE_PROTOTYPES) /* this breaks 2.2 (*forces* use of ANCIENT)*/
#      define __STDC__ 0 /* vaxc is not yet ANSI compliant, but close enough */
#      define signed	/* well, almost close enough */
#include <stddef.h>
#      define UNWIDENED_PROTOTYPES
#     endif
#     define USE_STDARG
#   endif
#  endif /*VAXC*/
# endif /*__DECC*/
# ifdef VERYOLD_VMS	/* v4.5 or earlier; no longer available for testing */
#  define USE_OLDARGS	/* <varargs.h> is there, vprintf & vsprintf aren't */
#  ifdef USE_VARARGS
#   undef USE_VARARGS
#  endif
#  ifdef USE_STDARG
#   undef USE_STDARG
#  endif
# endif
#endif /*VMS*/

#ifdef vax
/* just in case someone thinks a DECstation is a vax. It's not, it's a mips */
# ifdef ULTRIX_PROTO
#  undef ULTRIX_PROTO
# endif
# ifdef ULTRIX_CC20
#  undef ULTRIX_CC20
# endif
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

/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */
#define TTY_GRAPHICS	/* good old tty based graphics */
/* #define X11_GRAPHICS	/* X11 interface */

/*
 * Define the default window system.  This should be one that is compiled
 * into your system (see defines above).  Known window systems are:
 *
 *	tty, X11, mac, amii, win32
 */

/* MAC also means MAC windows */
#ifdef MAC
# ifndef	AUX
#  define DEFAULT_WINDOW_SYS "mac"
# endif
#endif

/* Amiga supports AMII_GRAPHICS and/or TTY_GRAPHICS */
#ifdef AMIGA
# define AMII_GRAPHICS			/* (optional) */
# define DEFAULT_WINDOW_SYS "amii"	/* "amii", "amitile" or "tty" */
#endif

/* Windows NT supports TTY_GRAPHICS */
#ifdef WIN32
#  define DEFAULT_WINDOW_SYS "tty"
#endif

#ifndef DEFAULT_WINDOW_SYS
# define DEFAULT_WINDOW_SYS "tty"
#endif

#ifdef X11_GRAPHICS
/*
 * There are two ways that X11 tiles may be defined.  (1) using a custom
 * format loaded by NetHack code, or (2) using the XPM format loaded by
 * the free XPM library.  The second option allows you to then use other
 * programs to generate tiles files.  For example, the PBMPlus tools
 * would allow:
 *  xpmtoppm <x11tiles.xpm | pnmscale 1.25 | ppmquant 90 >x11tiles_big.xpm
 */
/* # define USE_XPM		/* Disable if you do not have the XPM library */
# ifdef USE_XPM
#  define GRAPHIC_TOMBSTONE	/* Use graphical tombstone (rip.xpm) */
# endif
#endif

/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, or NEWS removes that feature
 *		from the game; otherwise set the appropriate wizard name.
 *		LOGFILE and NEWS refer to files in the playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
# ifndef KR1ED
#  define WIZARD  "wizard" /* the person allowed to use the -D option */
# else
#  define WIZARD
#  define WIZARD_NAME "wizard"
# endif
#endif

#define LOGFILE "logfile"	/* larger file for debugging purposes */
#define NEWS "news"		/* the file containing the latest hack news */

/*
 *	If COMPRESS is defined, it should contain the full path name of your
 *	'compress' program.  Defining INTERNAL_COMP causes NetHack to do
 *	simpler byte-stream compression internally.  Both COMPRESS and
 *	INTERNAL_COMP create smaller bones/level/save files, but require
 *	additional code and time.  Currently, only UNIX fully implements
 *	COMPRESS; other ports should be able to uncompress save files a
 *	la unixmain.c if so inclined.
 *	If you define COMPRESS, you must also define COMPRESS_EXTENSION
 *	as the extension your compressor appends to filenames after
 *	compression.
 */

#ifdef UNIX
/* path and file name extension for compression program */
# define COMPRESS "/usr/ucb/compress"	     /* Lempel-Ziv compression */
# define COMPRESS_EXTENSION ".Z"	     /* compress's extension */

/* An example of one alternative you might want to use: */
/* # define COMPRESS "/usr/local/bin/gzip"   /* FSF gzip compression */
/* # define COMPRESS_EXTENSION ".gz"	     /* normal gzip extension */
#endif
#ifndef COMPRESS
# define INTERNAL_COMP	/* control use of NetHack's compression routines */
#endif

/*
 *	Data librarian.  Defining DLB places most of the support files into
 *	a tar-like file, thus making a neater installation.  See *conf.h
 *	for detailed configuration.
 */
/* #define DLB		/* not supported on all platforms */

/*
 *	Defining INSURANCE slows down level changes, but allows games that
 *	died due to program or system crashes to be resumed from the point
 *	of the last level change, after running a utility program.
 */
#define INSURANCE	/* allow crashed game recovery */

#ifndef MAC
# define CHDIR		/* delete if no chdir() available */
#endif

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
# ifndef HACKDIR
#  define HACKDIR "/usr/games/lib/nethackdir"	/* nethack directory */
# endif

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
#endif /* CHDIR */



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
/* #define NOVOID			/* define if no "void" data type. */

/*
 * Uncomment the following line if your compiler falsely claims to be
 * a standard C compiler (i.e., defines __STDC__ without cause).
 * Examples are Apollo's cc (in some versions) and possibly SCO UNIX's rcc.
 */
/* #define NOTSTDC			/* define for lying compilers */

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
# define schar	char
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
#ifndef _AIX32		/* identical typedef in system file causes trouble */
typedef unsigned char	uchar;
#endif

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS	/* Good bitfield handling */

/* #define STRNCMPI /* compiler/library has the strncmpi function */

/*
 * There are various choices for the NetHack vision system.  There is a
 * choice of two algorithms with the same behavior.  Defining VISION_TABLES
 * creates huge (60K) tables at compile time, drastically increasing data
 * size, but runs slightly faster than the alternate algorithm.  (MSDOS in
 * particular cannot tolerate the increase in data size; other systems can
 * flip a coin weighted to local conditions.)
 *
 * If VISION_TABLES is not defined, things will be faster if you can use
 * MACRO_CPATH.  Some cpps, however, cannot deal with the size of the
 * functions that have been macroized.
 */

/*#define VISION_TABLES	/* use vision tables generated at compile time */
#ifndef VISION_TABLES
# ifndef NO_MACRO_CPATH
#  define MACRO_CPATH	/* use clear_path macros instead of functions */
# endif
#endif



/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

/* dungeon features */
#define WEAPON_SKILLS	/* Weapon skills - Stephen White */
#define SINKS		/* Kitchen sinks - Janet Walz */
/* dungeon levels */
#define WALLIFIED_MAZE	/* Fancy mazes - Jean-Christophe Collet */
#define REINCARNATION	/* Special Rogue-like levels */
/* monsters & objects */
#define KOPS		/* Keystone Kops by Scott R. Turner */
#define SEDUCE		/* Succubi/incubi seduction, by KAA, suggested by IM */
#define TOURIST		/* Tourist players with cameras and Hawaiian shirts */
/* difficulty */
#define ELBERETH	/* Engraving the E-word repels monsters */
/* I/O */
#define REDO		/* support for redoing last command - DGK */
#if !defined(MAC)
# define CLIPPING	/* allow smaller screens -- ERS */
#endif

#ifdef REDO
# define DOAGAIN '\001'	/* ^A, the "redo" key used in cmd.c and getline.c */
#endif

#define EXP_ON_BOTL	/* Show experience on bottom line */
/* #define SCORE_ON_BOTL	/* added by Gary Erickson (erickson@ucivax) */

#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H */
