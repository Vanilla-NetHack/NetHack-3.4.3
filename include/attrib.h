/*
 *	attrib.h - Header file for character class processing.
 *
 *	Copyright 1988, M. Stephenson
 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ATTRIB_H
#define ATTRIB_H

#define	A_STR	0
#define	A_INT	1
#define	A_WIS	2
#define	A_DEX	3
#define	A_CON	4
#define	A_CHA	5

#define A_MAX	6	/* used in rn2() selection of attrib */

#define	ABASE(x)	(u.acurr.a[x])
#define ABON(x)		(u.abon.a[x])
#define ACURR(x)	(schar) acurr(x)
/* should be: */
/* #define	ACURR(x)	(ABON(x) + ATEMP(x) + (u.umonnum == -1) ? ABASE(x) : MBASE(x)) */
#define	MCURR(x)	(u.macurr.a[x])
#define	AMAX(x)		(u.amax.a[x])
#define	MMAX(x)		(u.mamax.a[x])

#define	ATEMP(x)	(u.atemp.a[x])
#define	ATIME(x)	(u.atime.a[x])

struct	attribs {

	schar	a[A_MAX];
};

#endif /* ATTRIB_H /* */
