/*	SCCS Id: @(#)termcap.h	3.0	89/10/27
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

/* common #defines for pri.c and termcap.c */

#ifndef MSDOS
# ifndef MACOS
#  define TERMLIB	/* include termcap code */
# endif
#endif

/* might display need graphics code? */
#if !defined(AMIGA) && !defined(TOS) && !defined(MACOS)
# if defined(TERMLIB) || defined(OS2)
#  define ASCIIGRAPH
# endif
#endif
