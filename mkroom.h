/*	SCCS Id: @(#)mkroom.h	2.1	87/09/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

/* mkroom.h - types and structures for room and shop initialization */

struct mkroom {
	schar lx,hx,ly,hy;	/* usually xchar, but hx may be -1 */
	schar rtype,rlit,doorct,fdoor;
};

struct shclass {
	char	*name;	/* name of the shop type */
	char	symb;	/* this identifies the shop type */
    	int	prob;	/* the shop type probability in % */
	schar	dist;	/* artifact placement type */
#define D_SCATTER	0	/* normal placement */
#define D_SHOP		1	/* shop-like placement */
#define D_TEMPLE	2	/* temple-like placement */
	struct itp {
	    int	iprob;	/* probability of an item type */
	    int itype;	/* item type: if >=0 a class, if < 0 a specific item */
	} iprobs[3];
	char **shknms;	/* string list of shopkeeper names for this type */
};
extern struct shclass shtypes[];	/* defined in shknam.c */

#define	MAXNROFROOMS	15
extern struct mkroom rooms[MAXNROFROOMS+1];

#define	DOORMAX	100
extern coord doors[DOORMAX];

/* values for rtype in the room definition structure */
#define OROOM		 0	/* ordinary room */
#define COURT		 2	/* contains a throne */
#define	SWAMP		 3	/* contains pools */
#define	VAULT		 4	/* contains piles of gold */
#define	BEEHIVE		 5	/* contains killer bees and royal jelly */
#define	MORGUE		 6	/* contains corpses, undead and ghosts */
#define	ZOO		 7	/* floor covered with treasure and monsters */
#define	SHOPBASE	 8	/* everything above this is a shop */

#define IS_SHOP(x)	((x).rtype >= SHOPBASE)
