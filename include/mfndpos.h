/*	SCCS Id: @(#)mfndpos.h	3.0	88/10/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* mfndpos.h - version 1.0.2 */
 
#ifndef MFNDPOS_H
#define MFNDPOS_H

/* changed by GAN 02/06/87 to add nine extra bits for traps -
 * this because new traps make nine for traps insufficient
 */
 
#define ALLOW_TRAPS     0777777L
#define ALLOW_U         01000000L	/* can attack you */
#define ALLOW_M         02000000L	/* can attack other monsters */
#define ALLOW_TM        04000000L
#define ALLOW_ALL       (ALLOW_U | ALLOW_M | ALLOW_TM | ALLOW_TRAPS)
#define ALLOW_SSM       010000000L	/* ignores scare monster */
#define ALLOW_ROCK      020000000L	/* pushes rocks */
#define NOTONL          040000000L	/* stays off direct line to player */
#define NOGARLIC        0100000000L	/* hates garlic */
#define ALLOW_WALL      0200000000L	/* walks through walls */
#define ALLOW_DIG       0400000000L	/* digs */
#define ALLOW_SANCT	01000000000L	/* enters a temple */

#endif /* MFNDPOS_H /**/
