/*	SCCS Id: @(#)lev.h	3.2	94/03/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*	Common include file for save and restore routines */

#ifndef LEV_H
#define LEV_H

#define COUNT_SAVE	0x1
#define WRITE_SAVE	0x2
#define FREE_SAVE	0x4

/* operations of the various saveXXXchn & co. routines */
#define perform_bwrite(mode)	((mode) & (COUNT_SAVE|WRITE_SAVE))
#define release_data(mode)	((mode) & FREE_SAVE)

#endif /* LEV_H */
