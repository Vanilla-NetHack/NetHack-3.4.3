/*	SCCS Id: @(#)tradstdc.h	3.0	89/07/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRADSTDC_H
#define	TRADSTDC_H

#ifdef DUMB
#undef void
#define void	int
#endif

/*
 * ANSI X3J11 detection.
 * Makes substitutes for compatibility with the old C standard.
 */

#if (defined(__STDC__) || defined(MSDOS)) && !defined(AMIGA)

/* Used for robust ANSI parameter forward declarations:
 * int sprintf P((char *, const char *, ...));
 */

/* However, if you do use a prototype in declaring a function but define
 * the function in an old-style manner, the compiler might (justifiably) barf.
 * Since we have to define functions in the old-style manner to accommodate
 * traditional compilers, we don't want to use prototypes if the ANSI-standard
 * compiler barfs on these.
 */
#ifdef __GNUC__ /* add other compilers as the need arises */
# define P(s)		()
#else
# define P(s)		s
#endif

# ifdef __TURBOC__	/* Cover for stupid Turbo C */
#  define genericptr_t	void *
# else
typedef void *		genericptr_t;
#  ifndef __STDC__
#   define const
#   define signed
#   define volatile
#  endif
# endif

#else /* __STDC__ */	/* a "traditional" C  compiler */

# define P(s)		()

# ifndef genericptr_t
#  ifdef AMIGA
typedef void *		genericptr_t;
#  else
typedef char *		genericptr_t;
#  endif
# endif

# define const
# define signed
# define volatile

#endif /* __STDC__ */

#ifdef __HC__	/* MetaWare High-C defaults to unsigned chars */
# undef signed
#endif

#endif /* TRADSTDC_H /**/
