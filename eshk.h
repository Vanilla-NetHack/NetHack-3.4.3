/*	SCCS Id: @(#)eshk.h	2.1	87/09/28
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#define	BILLSZ	200

struct bill_x {
	unsigned bo_id;
	unsigned price;		/* price per unit */
	Bitfield(useup,1);
	Bitfield(bquan,7);
};

struct eshk {
	long int robbed;	/* amount stolen by most recent customer */
	boolean following;	/* following customer since he owes us sth */
	schar shoproom;		/* index in rooms; set by inshop() */
	coord shk;		/* usual position shopkeeper */
	coord shd;		/* position shop door */
	int shoplevel;		/* level of his shop */
	int billct;
	struct bill_x bill[BILLSZ];
	int visitct;		/* nr of visits by most recent customer */
	char customer[PL_NSIZ];	/* most recent customer */
	char shknam[PL_NSIZ];
};
