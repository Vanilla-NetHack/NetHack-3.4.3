/*	SCCS Id: @(#)edog.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* edog.h - version 1.0.2 */

/*	various types of food, the lower, the better liked.	*/

#define	DOGFOOD	0
#define	CADAVER	1
#define	ACCFOOD	2
#define	MANFOOD	3
#define	APPORT	4
#define	POISON	5
#define	UNDEF	6

struct edog {
	long hungrytime;	/* at this time dog gets hungry */
	long eattime;		/* dog is eating */
	long droptime;		/* moment dog dropped object */
	unsigned dropdist;		/* dist of drpped obj from @ */
	unsigned apport;		/* amount of training */
	long whistletime;		/* last time he whistled */
};
#define	EDOG(mp)	((struct edog *)(&(mp->mextra[0])))
