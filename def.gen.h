/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

struct gen {
	struct gen *ngen;
	xchar gx,gy;
	unsigned gflag;		/* 037: trap type; 040: SEEN flag */
#define	SEEN	040
};
extern struct gen *fgold, *ftrap;
struct gen *g_at();
#define newgen()	(struct gen *) alloc(sizeof(struct gen))
