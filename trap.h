/*	SCCS Id: @(#)trap.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* trap.h - version 1.0.2 */

struct trap {
	struct trap *ntrap;
	xchar tx,ty;
	Bitfield(ttyp,5);
	Bitfield(tseen,1);
	Bitfield(once,1);
};

extern struct trap *ftrap;
struct trap *t_at();
#define newtrap()	(struct trap *) alloc(sizeof(struct trap))

/* Standard Hack traps. */
#define NO_TRAP         0
#define BEAR_TRAP       1
#define ARROW_TRAP      2
#define DART_TRAP       3
#define TRAPDOOR        4
#define TELEP_TRAP      5
#define PIT             6
#define SLP_GAS_TRAP    7
#define PIERC           8
#define MIMIC           9

/* Defines below this line are automatically added by makedefs (-t option) */
/* if you add any additional code below the next line, it will disappear.  */
/* DO NOT REMOVE THIS LINE */

#define	MGTRP		10
#define	SQBRD		11
#define	WEB		12
#define	SPIKED_PIT	13
#define	LEVEL_TELEP	14
#define	ANTI_MAGIC	15
#define	RUST_TRAP	16

#define	TRAPNUM	17
