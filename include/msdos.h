/*	SCCS Id: @(#)msdos.h	3.0	88/07/21
/* msdos.h - function declarations for msdos.c */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MSDOS_H
#define MSDOS_H

#ifdef TOS
#define msmsg	cprintf
#endif
extern const char *alllevels, *allbones;
extern char levels[], bones[], permbones[], SAVEF[], hackdir[];
extern int ramdisk;
#if defined(DGK) && !defined(TOS)
extern int count_only;
#endif

#define CTRL(ch) (ch & 0x37)
#define ABORT CTRL('A')
#define COUNT 0x1
#define WRITE 0x2

#endif /* MSDOS_H /* */
