/*	SCCS Id: @(#)edog.h	3.2	95/05/31	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EDOG_H
#define EDOG_H

/*	various types of food, the lower, the better liked.	*/

#define DOGFOOD	0
#define CADAVER	1
#define ACCFOOD	2
#define MANFOOD	3
#define APPORT	4
#define POISON	5
#define UNDEF	6
#define TABU	7

struct edog {
	long droptime;			/* moment dog dropped object */
	unsigned dropdist;		/* dist of drpped obj from @ */
	int apport;			/* amount of training */
	long whistletime;		/* last time he whistled */
	long hungrytime;		/* will get hungry at this time */
	coord ogoal;			/* previous goal location */
};
#define EDOG(mon)	((struct edog *)&(mon)->mextra[0])

#endif /* EDOG_H */
