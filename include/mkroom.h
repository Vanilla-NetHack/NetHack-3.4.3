/*	SCCS Id: @(#)mkroom.h	3.0	89/01/07
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MKROOM_H
#define MKROOM_H

/* mkroom.h - types and structures for room and shop initialization */

struct mkroom {
	schar lx,hx,ly,hy;	/* usually xchar, but hx may be -1 */
	schar rtype,rlit,doorct,fdoor;
};

struct shclass {
	const char	*name;	/* name of the shop type */
	char	symb;	/* this identifies the shop type */
	int	prob;	/* the shop type probability in % */
	schar	dist;	/* artifact placement type */
#define D_SCATTER	0	/* normal placement */
#define D_SHOP		1	/* shop-like placement */
#define D_TEMPLE	2	/* temple-like placement */
	struct itp {
	    int	iprob;	/* probability of an item type */
	    int itype;	/* item type: if >=0 a class, if < 0 a specific item */
	} iprobs[5];
	char **shknms;	/* string list of shopkeeper names for this type */
};
extern const struct shclass shtypes[];	/* defined in shknam.c */

extern struct mkroom rooms[MAXNROFROOMS+1];
/* the normal rooms on the current level are described in rooms[0..n] for
 * some n<MAXNROFROOMS
 * the vault, if any, is described by rooms[n+1]
 * the next rooms entry has hx -1 as a flag
 * there is at most one non-vault special room on a level
 */
extern coord doors[DOORMAX];

/* values for rtype in the room definition structure */
#define OROOM		 0	/* ordinary room */
#define COURT		 2	/* contains a throne */
#define	SWAMP		 3	/* contains pools */
#define	VAULT		 4	/* contains piles of gold */
#define	BEEHIVE		 5	/* contains killer bees and royal jelly */
#define	MORGUE		 6	/* contains corpses, undead and ghosts */
#define BARRACKS	 7	/* contains soldiers and their gear */
#define	ZOO		 8	/* floor covered with treasure and monsters */
#define DELPHI		 9	/* contains Oracle and peripherals */
#define	TEMPLE		10	/* contains a shrine */
#define	SHOPBASE	11	/* everything above this is a shop */
#define ARMORSHOP	12	/* specific shop defines for level compiler */
#define	SCROLLSHOP	13
#define	POTIONSHOP	14
#define	WEAPONSHOP	15
#define	FOODSHOP	16
#define	RINGSHOP	17
#define	WANDSHOP	18
#define	TOOLSHOP	19
#ifdef SPELLS
#define	BOOKSHOP	20
#endif

#define IS_SHOP(x)	((x).rtype >= SHOPBASE)

#endif /* MKROOM_H /**/
