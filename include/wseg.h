/*	SCCS Id: @(#)wseg.h	3.0	88/10/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* wseg.h - version 1.0.2 */

#ifndef WSEG_H
#define WSEG_H

# ifdef WORM
/* worm structure */
struct wseg {
	struct wseg *nseg;
	xchar wx,wy;
	Bitfield(wdispl,1);
};

#define newseg()	(struct wseg *) alloc(sizeof(struct wseg))

extern struct wseg *wsegs[32], *wheads[32], *m_atseg;
extern long wgrowtime[32];

# endif

#endif /* WSEG_H /**/
