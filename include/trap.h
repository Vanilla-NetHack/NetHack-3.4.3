/*	SCCS Id: @(#)trap.h	3.0	88/06/19
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRAP_H
#define TRAP_H

struct trap {
	struct trap *ntrap;
	xchar tx,ty;
	Bitfield(ttyp,5);
	Bitfield(tseen,1);
	Bitfield(once,1);
	unsigned pm;		/* monster type for (MONST | STATUE)_TRAP */
};

extern struct trap *ftrap;
#define newtrap()	(struct trap *) alloc(sizeof(struct trap))

/* Standard Hack traps. */
#define NO_TRAP         0
#define MONST_TRAP      1
#define STATUE_TRAP     2
#define BEAR_TRAP       3
#define ARROW_TRAP      4
#define DART_TRAP       5
#define TRAPDOOR        6
#define TELEP_TRAP      7
#define PIT             8
#define SLP_GAS_TRAP    9

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
#define	POLY_TRAP	17
#define	LANDMINE	18

#define	TRAPNUM	19

#endif /* TRAP_H /**/
