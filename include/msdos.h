/*	SCCS Id: @(#)msdos.h	3.0	88/07/21
/* msdos.h - function declarations for msdos.c */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MSDOS_H
#define MSDOS_H

#ifdef OLD_TOS
#define msmsg	cprintf
#endif
extern const char *alllevels, *allbones;
extern char levels[], bones[], permbones[], SAVEF[], hackdir[];
#ifdef MSDOS
extern char SAVEP[];
#endif
extern int ramdisk;
#if defined(DGK) && !defined(OLD_TOS)
extern int count_only;
#endif

#define C(c)  (0x1f & (c))
#define M(c)  (0x80 | (c))
#define ABORT C('a')
#define COUNT 0x1
#define WRITE 0x2

#endif /* MSDOS_H /* */
