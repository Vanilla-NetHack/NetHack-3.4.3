/*	SCCS Id: @(#)config.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* config.h - version 1.0.3 */

#ifndef CONFIG	/* make sure the compiler does not see the typedefs twice */
#define	CONFIG

#define	CHDIR		/* delete if no chdir() available */

/*
 * Some include files are in a different place under SYSV
 * 	BSD		   SYSV
 * <strings.h>		<string.h>
 * <sys/wait.h>		<wait.h>
 * <sys/time.h>		<time.h>
 * <sgtty.h>		<termio.h>
 * Some routines are called differently
 * index		strchr
 * rindex		strrchr
 * Also, the code for suspend and various ioctls is only given for BSD4.2
 * (I do not have access to a SYSV system.)
 */
/* #define MSDOS 	/* define for MS-DOS (actually defined by compiler) */
#define	UNIX		/* delete if no fork(), exec() available */
#define BSD		/* defind for 4.n BSD  */
/* #define SYSV		/* define for System V */

/* #define BETA		/* if a beta-test copy  [MRS] */
#define VERSION	"1.3d"	/* version number. */

#define PYRAMID_BUG 	/* avoid a bug on the Pyramid */
/* #define APOLLO		/* same for the Apollo */
/* #define STUPID		/* avoid some complicated expressions if
			   your C compiler chokes on them */
/* #define NOWAITINCLUDE	/* neither <wait.h> nor <sys/wait.h> exists */
#ifdef MSDOS
#define	NOWAITINCLUDE
#endif

#define WIZARD  "mike"	/* the person allowed to use the -D option */
#define RECORD	"record"/* the file containing the list of topscorers */
#define	NEWS	"news"	/* the file containing the latest hack news */
#define	HELP	"help"	/* the file containing a description of the commands */
#define	SHELP	"hh"	/* abbreviated form of the same */
#define	RUMORFILE	"rumors"	/* a file with fortune cookies */
#define	DATAFILE	"data"	/* a file giving the meaning of symbols used */
#define	FMASK	0660	/* file creation mask */

#ifdef UNIX
#define	HLOCK	"perm"	/* an empty file used for locking purposes */
#define LLOCK	"safelock"	/* link to previous */

/*
 * Define DEF_PAGER as your default pager, e.g. "/bin/cat" or "/usr/ucb/more"
 * If defined, it can be overridden by the environment variable PAGER.
 * Hack will use its internal pager if DEF_PAGER is not defined.
 * (This might be preferable for security reasons.)
 * #define DEF_PAGER	".../mydir/mypager"
 */

/*
 * If you define MAIL, then the player will be notified of new mail
 * when it arrives. If you also define DEF_MAILREADER then this will
 * be the default mail reader, and can be overridden by the environment
 * variable MAILREADER; otherwise an internal pager will be used.
 * A stat system call is done on the mailbox every MAILCKFREQ moves.
 */
#define	MAIL
#define	DEF_MAILREADER	"/usr/bin/mail"		/* or e.g. /bin/mail */
#define	MAILCKFREQ	1


#define SHELL		/* do not delete the '!' command */

#ifdef BSD
#define	SUSPEND		/* let ^Z suspend the game */
#endif
#endif /* UNIX /**/

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
#define HACKDIR	"/usr/games/lib/nethackdir"

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files. In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
#define SECURE			/* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS.
 * #define MAX_NR_OF_PLAYERS	6
 */
#endif /* CHDIR /**/

/* size of terminal screen is (at least) (ROWNO+2) by COLNO */
#define	COLNO	80
#define	ROWNO	22

#ifdef BSD
#include <strings.h>		/* declarations for strcat etc. */
#else
#include <string.h>		/* idem on System V */
#define	index	strchr
#define	rindex	strrchr
#endif

/*
 * small signed integers (8 bits suffice)
 *	typedef	char	schar;
 * will do when you have signed characters; otherwise use
 *	typedef	short int schar;
 */
typedef	char	schar;

/*
 * small unsigned integers (8 bits suffice - but 7 bits do not)
 * - these are usually object types; be careful with inequalities! -
 *	typedef	unsigned char	uchar;
 * will be satisfactory if you have an "unsigned char" type; otherwise use
 *	typedef unsigned short int uchar;
 */
typedef	unsigned char	uchar;

/*
 * small integers in the range 0 - 127, usually coordinates
 * although they are nonnegative they must not be declared unsigned
 * since otherwise comparisons with signed quantities are done incorrectly
 */
typedef schar	xchar;
typedef	xchar	boolean;		/* 0 or 1 */
#define	TRUE	1
#define	FALSE	0

/*
 * Declaration of bitfields in various structs; if your C compiler
 * doesnt handle bitfields well, e.g., if it is unable to initialize
 * structs containing bitfields, then you might use
 *	#define Bitfield(x,n)	uchar x
 * since the bitfields used never have more than 7 bits. (Most have 1 bit.)
 */
#define	Bitfield(x,n)	uchar x

#define	SIZE(x)	(int)(sizeof(x) / sizeof(x[0]))

#ifdef MSDOS
#include <fcntl.h>
#define	exit	msexit		/* do chdir first */
#ifdef getchar
#	undef getchar
#endif /* getchar /**/
#define getchar tgetch
#define DGK			/* MS DOS specific enhancements by dgk */

#ifdef DGK
#  include "msdos.h"	/* contains necessary externs for msdos.c */
#  define SHELL		/* via exec of COMMAND.COM */
#  define PATHLEN	64	/* maximum pathlength */
#  define FILENAME 80	/* maximum filename length (conservative) */
#  define FROMPERM	1	/* for ramdisk use */
#  define TOPERM	2	/* for ramdisk use */
#  define glo(x)	name_file(lock, x)	/* name_file used for bones */
#  define IS_CORNER(x)	((x) == symbol.tlcorn || (x) == symbol.trcorn\
				|| (x) == symbol.blcorn || (x)==symbol.brcorn)
	/* screen symbols for using character graphics. */
	struct symbols {
		unsigned char vwall, hwall, tlcorn, trcorn, blcorn, brcorn;
		unsigned char door, room, corr;
	};
	extern struct symbols symbol;
	extern char *configfile;
#endif /* DGK /**/
#endif /* MSDOS /**/

/*
 *	Conditional compilation of special options are controlled here.
 *	If you define the following flags, you will add not only to the
 *	complexity of the game but also to the size of the load module.
 */ 
 
#define	SPELLS		/* Spell casting by M. Stephenson */
#define	PRAYERS		/* Prayer code by M. Stephenson */
#define KAA		/* Various changes made by Ken Arromdee */
#define MARKER		/* Magic marker modification from Gil Neiger */
#define	NEWCLASS	/* Samurai/Ninja etc. by M. Stephenson */ 
#define	SAFE_ATTACK 	/* Safe attack code by Don Kneller */
#define	PROBING		/* Wand of probing code by Gil Neiger */
#define	DIAGS		/* Diagnostics after death/quit by Gil Neiger */
#define	SORTING		/* Sorted inventory by Don Kneller */
#define	DGKMOD		/* Additional features by Don Kneller */
#define REDO 		/* support for redoing last command - DGK */
#define	HARD		/* Enhanced wizard code by M. Stephenson */
#define	WALKIES		/* Leash code by M. Stephenson */
#define NEWTRAPS	/* Magic and Squeeky board traps by Scott R. Turner*/
#define FREEHAND	/* Cannot use Pick-axe without wielding it. */
#define SPIDERS		/* Spiders and webs by Scott R. Turner */
#define FOUNTAINS	/* Fountain code by SRT (+ GAN + EB) */
#define KOPS		/* Keystone Kops by Scott R. Turner */
#define ROCKMOLE	/* Rockmoles by Scott R. Turner */

/*
 *	Status Line options.
 */

#define	GOLD_ON_BOTL
#define	EXP_ON_BOTL
	 
#ifdef REDO
#define DOAGAIN	'\001'		/* Used in tty.c and cmd.c */
#endif

#ifdef DGKMOD
#define LARGEST_INT	((1 << 15) - 1)
#endif

#endif /* CONFIG /**/
