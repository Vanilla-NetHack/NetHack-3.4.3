/*	SCCS Id: @(#)micro.h	3.1	90/22/02	*/
/* micro.h - function declarations for various microcomputers */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MICRO_H
#define MICRO_H

extern const char *alllevels, *allbones;
extern char levels[], bones[], permbones[], hackdir[];

extern int ramdisk;

#define C(c)	(0x1f & (c))
#define M(c)	(0x80 | (c))
#define ABORT C('a')

#endif /* MICRO_H */
