/*	SCCS Id: @(#)emin.h	3.1	90/15/12	*/
/* Copyright (c) David Cohrs, 1990.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EMIN_H
#define EMIN_H

#include "dungeon.h"

struct emin {
	aligntyp	min_align;	/* alignment of minion */
};

#define EMIN(mon)	((struct emin *)&(mon)->mextra[0])

#endif /* EMIN_H */
