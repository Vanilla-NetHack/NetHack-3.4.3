/*	SCCS Id: @(#)gold.h	3.0	88/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef GOLD_H
#define GOLD_H

struct gold {
	struct gold *ngold;
	xchar gx,gy;
	long amount;
};

extern struct gold *fgold;
#define newgold()	(struct gold *) alloc(sizeof(struct gold))

#endif /* GOLD_H /**/
