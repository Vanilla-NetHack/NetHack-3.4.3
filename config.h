/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */
/* config.h version 1.0.1: added definition of HELP */

#ifndef CONFIG	/* make sure the compiler doesnt see the typedefs twice */

#define	CONFIG
#define	VAX		/* to get proper struct initialization */
#define BSD		/* delete this line on System V */
/* #define STUPID */	/* avoid some complicated expressions if
			   your C compiler chokes on them */

#define WIZARD  "play"	/* the person allowed to use the -w option */
#define	NEWS	"news"	/* the file containing the latest hack news */
#define	HELP	"help"	/* the file containing a description of the commands */
#define	FMASK	0660	/* file creation mask */

#define OPTIONS		/* do not delete the 'o' command */
#define SHELL		/* do not delete the '!' command */
#define	TRACK		/* do not delete the tracking properties of monsters */

/* size of terminal screen is (ROWNO+2) by COLNO */
#define	COLNO	80
#define	ROWNO	22

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
 * (thus, in fact, you could make xchar equal to schar)
 */
typedef char	xchar;
typedef	xchar	boolean;		/* 0 or 1 */
#define	TRUE	1
#define	FALSE	0

/*
 * Declaration of bitfields in various structs; if your C compiler
 * doesnt handle bitfields well, e.g., if it is unable to initialize
 * structs containing bitfields, then you might use
 *	#define Bitfield(x,n)	xchar x
 * since the bitfields used never have more than 7 bits. (Most have 1 bit.)
 */
#define	Bitfield(x,n)	unsigned x:n

#endif CONFIG
