/*	SCCS Id: @(#)eshk.h	3.0	88/04/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ESHK_H
#define ESHK_H

#define	BILLSZ	200

struct bill_x {
	unsigned bo_id;
	unsigned price;		/* price per unit */
	Bitfield(useup,1);
	Bitfield(bquan,7);
};

struct eshk {
	long int robbed;	/* amount stolen by most recent customer */
	long int credit;	/* amount credited to customer */
	long int debit;		/* amount of debt for using unpaid items */
	boolean following;	/* following customer since he owes us sth */
	schar shoproom;		/* index in rooms; set by inshop() */
	coord shk;		/* usual position shopkeeper */
	coord shd;		/* position shop door */
	int shoplevel;		/* level of his shop */
	int billct;
	struct bill_x bill[BILLSZ];
	int visitct;		/* nr of visits by most recent customer */
	char customer[PL_NSIZ];	/* most recent customer */
	boolean ismale;
	char shknam[PL_NSIZ];
};

#define	ESHK(mon)	((struct eshk *)(&(mon->mextra[0])))
#endif /* ESHK_H /**/
