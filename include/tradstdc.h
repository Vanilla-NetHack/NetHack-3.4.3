/*	SCCS Id: @(#)tradstdc.h	3.0	89/07/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRADSTDC_H
#define	TRADSTDC_H

#ifdef DUMB
#undef void
#define void	int
#endif

#ifdef APOLLO	/* the Apollo C compiler claims to be __STDC__, but isn't */
#undef __STDC__
#endif

/*
 * ANSI X3J11 detection.
 * Makes substitutes for compatibility with the old C standard.
 */

/* Decide how to handle variable parameter lists:
 * USE_STDARG means use the ANSI <stdarg.h> facilities (only ANSI compilers
 * should do this, and only if the library supports it).
 * USE_VARARGS means use the <varargs.h> facilities.  Again, this should only
 * be done if the library supports it.  ANSI is *not* required for this.
 * Otherwise, the kludgy old methods are used.
 * The defaults are USE_STDARG for ANSI compilers, and USE_OLDARGS for
 * others.
 */

/* #define USE_VARARGS		/* use <varargs.h> instead of <stdarg.h> */
/* #define USE_OLDARGS		/* don't use any variable argument facilites */

#if defined(__STDC__) || defined(VMS)
# if !(defined(AMIGA) && defined(AZTEC_C) || defined(USE_VARARGS) || defined(USE_OLDARGS))
#   define USE_STDARG
# endif
#endif

#ifdef NEED_VARARGS		/* only define these if necessary */
#ifdef USE_STDARG
# include <stdarg.h>
# define VA_DECL(typ1,var1)	(typ1 var1, ...) { va_list the_args;
# define VA_DECL2(typ1,var1,typ2,var2)	\
	(typ1 var1, typ2 var2, ...) { va_list the_args;
# define VA_INIT(var1,typ1)
# define VA_NEXT(var1,typ1)	var1 = va_arg(the_args, typ1)
# define VA_ARGS		the_args
# define VA_START(x)		va_start(the_args, x)
# define VA_END()		va_end(the_args)
#else
# ifdef USE_VARARGS
#  include <varargs.h>
#  define VA_DECL(typ1,var1)	(va_alist) va_dcl {\
		va_list the_args; typ1 var1;
#  define VA_DECL2(typ1,var1,typ2,var2)	(va_alist) va_dcl {\
		va_list the_args; typ1 var1; typ2 var2;
#  define VA_ARGS		the_args
#  define VA_START(x)		va_start(the_args)
#  define VA_INIT(var1,typ1) 	var1 = va_arg(the_args, typ1)
#  define VA_NEXT(var1,typ1)	var1 = va_arg(the_args,typ1)
#  define VA_END()		va_end(the_args)
# else
#   define VA_ARGS	arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
#   define VA_DECL(typ1,var1)  (var1,VA_ARGS) typ1 var1; \
	char *arg1,*arg2,*arg3,*arg4,*arg5,*arg6,*arg7,*arg8,*arg9; {
#   define VA_DECL2(typ1,var1,typ2,var2)  (var1,var2,VA_ARGS) \
	typ1 var1; typ2 var2;\
	char *arg1,*arg2,*arg3,*arg4,*arg5,*arg6,*arg7,*arg8,*arg9; {
#   define VA_START(x)
#   define VA_INIT(var1,typ1)
#   define VA_END()
# endif
#endif
#endif /* NEED_VARARGS */


#if defined(__STDC__) || defined(MSDOS) || defined(THINKC4)

/* Used for robust ANSI parameter forward declarations:
 * int VDECL(sprintf, (char *, const char *, ...));
 *
 * NDECL() is used for functions with zero arguments;
 * FDECL() is used for functions with a fixed number of arguments;
 * VDECL() is used for functions with a variable number of arguments.
 * Separate macros are needed because ANSI will mix old-style declarations
 * with prototypes, except in the case of varargs, and the OVERLAY-specific
 * trampoli.* mechanism conflicts with the ANSI <<f(void)>> syntax.
 */

# define NDECL(f)	f(void)	/* Must be overridden if OVERLAY set later */

# define FDECL(f,p)	f p

# if defined(MSDOS) || defined(USE_STDARG)
#  define VDECL(f,p)	f p
# else
#  define VDECL(f,p)	f()
# endif

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

# define NDECL(f)	f()
# define FDECL(f,p)	f()
# define VDECL(f,p)	f()

# ifndef genericptr_t
#  if defined(AMIGA) || defined(HPUX)
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
