/*	SCCS Id: @(#)mfndpos.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* mfndpos.h - version 1.0.2 */
 
/* changed by GAN 02/06/87 to add nine extra bits for traps -
 * this because new traps make nine for traps insufficient
 */
 
#define ALLOW_TRAPS     0777777
#define ALLOW_U         01000000
#define ALLOW_M         02000000
#define ALLOW_TM        04000000
#define ALLOW_ALL       (ALLOW_U | ALLOW_M | ALLOW_TM | ALLOW_TRAPS)
#define ALLOW_SSM       010000000
#define ALLOW_ROCK      020000000
#define NOTONL          040000000
#define NOGARLIC        0100000000
#define ALLOW_WALL      0200000000
