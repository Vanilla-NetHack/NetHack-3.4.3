/*	SCCS Id: @(#)timeout.h	3.3	1999/02/13	*/
/* Copyright 1994, Dean Luick					  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TIMEOUT_H
#define TIMEOUT_H

/* generic timeout function */
typedef void FDECL((*timeout_proc), (genericptr_t, long));

/* kind of timer */
#define TIMER_LEVEL	0	/* event specific to level */
#define TIMER_GLOBAL	1	/* event follows current play */
#define TIMER_OBJECT	2	/* event follows a object */
#define TIMER_MONSTER	3	/* event follows a monster */

/* save/restore timer ranges */
#define RANGE_LEVEL  0		/* save/restore timers staying on level */
#define RANGE_GLOBAL 1		/* save/restore timers following global play */

/*
 * Timeout functions.  Add a define here, then put it in the table
 * in timeout.c.  "One more level of indirection will fix everything."
 */
#define ROT_ORGANIC	0	/* for buried organics */
#define ROT_CORPSE	1
#define REVIVE_MON	2
#define BURN_OBJECT	3
#define HATCH_EGG	4
#define FIG_TRANSFORM	5
#define NUM_TIME_FUNCS	6

#endif /* TIMEOUT_H */
