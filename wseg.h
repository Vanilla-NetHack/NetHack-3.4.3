/*	SCCS Id: @(#)wseg.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* wseg.h - version 1.0.2 */

#ifndef NOWORM
/* worm structure */
struct wseg {
	struct wseg *nseg;
	xchar wx,wy;
	Bitfield(wdispl,1);
};

#define newseg()	(struct wseg *) alloc(sizeof(struct wseg))
#endif
