/*	SCCS Id: @(#)trap.h	3.1	92/09/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* note: no longer manipulated by 'makedefs' */

#ifndef TRAP_H
#define TRAP_H

struct trap {
	struct trap *ntrap;
	xchar tx,ty;
	Bitfield(ttyp,5);
	Bitfield(tseen,1);
	Bitfield(once,1);
	d_level dst;	/* destination for portals */
};

extern struct trap *ftrap;
#define newtrap()	(struct trap *) alloc(sizeof(struct trap))
#define dealloc_trap(trap) free((genericptr_t) (trap))

/* unconditional traps */
#define NO_TRAP		0
#define ARROW_TRAP	1
#define DART_TRAP	2
#define ROCKTRAP	3
#define SQKY_BOARD	4
#define BEAR_TRAP	5
#define LANDMINE	6
#define SLP_GAS_TRAP	7
#define RUST_TRAP	8
#define FIRE_TRAP	9
#define PIT		10
#define SPIKED_PIT	11
#define TRAPDOOR	12
#define TELEP_TRAP	13
#define LEVEL_TELEP	14
#define MAGIC_PORTAL    15
#define WEB		16
#define STATUE_TRAP	17
#define MAGIC_TRAP	18
#define ANTI_MAGIC	19

/* conditional feature traps */
#ifdef POLYSELF
#define POLY_TRAP	20
#define TRAPNUM	21
#else
#define TRAPNUM	20
#endif

#endif /* TRAP_H */
