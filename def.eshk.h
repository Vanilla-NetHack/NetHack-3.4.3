/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#define	BILLSZ	200
struct bill_x {
	unsigned bo_id;
	unsigned useup:1;
	unsigned bquan:7;
	unsigned price;		/* price per unit */
};

struct eshk {
	long int robbed;	/* amount stolen by most recent customer */
	schar shoproom;		/* index in rooms; set by inshop() */
	coord shk;		/* usual position shopkeeper */
	coord shd;		/* position shop door */
	int billct;
	struct bill_x bill[BILLSZ];
	int visitct;		/* nr of visits by most recent customer */
	char customer[PL_NSIZ];	/* most recent customer */
	char shknam[PL_NSIZ];
};
