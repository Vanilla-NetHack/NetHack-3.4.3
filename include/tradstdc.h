/*	SCCS Id: @(#)tradstdc.h	3.1	92/04/01	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRADSTDC_H
#define TRADSTDC_H

#if defined(DUMB) && !defined(NOVOID)
#define NOVOID
#endif

#ifdef NOVOID
#define void int
#endif

#if defined(__STDC__) && !defined(NOTSTDC)
#define NHSTDC
#endif

#if defined(ultrix) && defined(__STDC__) && !defined(__LANGUAGE_C)
/* Ultrix seems to be in a constant state of flux.  This check attempts to
 * set up ansi compatibility if it wasn't set up correctly by the compiler.
 */
#ifdef mips
#define __mips mips
#endif

#ifdef LANGUAGE_C
#define __LANGUAGE_C LANGUAGE_C
#endif

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

#if defined(apollo)             /* Apollos have stdarg(3) but not stdarg.h */
# define USE_VARARGS
#endif

#if defined(NHSTDC) || defined(ULTRIX_PROTO) || defined(MAC)
# if !defined(USE_VARARGS) && !defined(USE_OLDARGS) && !defined(USE_STDARG)
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
# if defined(ULTRIX_PROTO) && !defined(_VA_LIST_)
#  define _VA_LIST_	/* prevents multiple def in stdio.h */
# endif
#else
# ifdef USE_VARARGS
#  include <varargs.h>
#  define VA_DECL(typ1,var1)	(va_alist) va_dcl {\
		va_list the_args; typ1 var1;
#  define VA_DECL2(typ1,var1,typ2,var2)	(va_alist) va_dcl {\
		va_list the_args; typ1 var1; typ2 var2;
#  define VA_ARGS		the_args
#  define VA_START(x)		va_start(the_args)
#  define VA_INIT(var1,typ1)	var1 = va_arg(the_args, typ1)
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

#if defined(NHSTDC) || defined(MSDOS) || defined(MAC) || defined(ULTRIX_PROTO)

/*
 * Used for robust ANSI parameter forward declarations:
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

/* generic pointer, always a macro; genericptr_t is usually a typedef */
# define genericptr	void *

# if defined(__TURBOC__) || (defined(ULTRIX_PROTO) && !defined(__GNUC__)) || defined(OS2_CSET2)
/* Cover for stupid Turbo C */
/* And Ultrix on a DECstation with 2.0 compiler, which coredumps on
 *   typedef void * genericptr_t;
 *   extern void a(void(*)(int, genericptr_t));
 * Using the #define is OK for other compiler versions too.
 */
/* And IBM CSet/2.  The redeclaration of free hoses the compile. */
#  define genericptr_t	genericptr
# else
#  if !defined(NHSTDC) && !defined(MAC)
#   define const
#   define signed
#   define volatile
#  endif
# endif

# if !defined(LATTICE) && !defined(MAC)
		/* Lattice can't even PARSE the const below! */
		/* MPW can parse but expects an identifier, not a keyword... */
#  if defined(ULTRIX_PROTO) && !defined(NHSTDC) && !defined(const)
#  define const		/* the system header files are *not* __STDC__ */
#  endif
#  if defined(apollo) && !defined(const)
#  define const         /* too much trouble with printf(char *format, ...) */
#  endif                 /* instead of printf(const char *format, ...) etc. */
# endif

#else /* NHSTDC */	/* a "traditional" C  compiler */

# define NDECL(f)	f()
# define FDECL(f,p)	f()
# define VDECL(f,p)	f()

# if defined(AMIGA) || defined(HPUX) || defined(POSIX_TYPES) || defined(__DECC)
#  define genericptr	void *
# endif
# ifndef genericptr
#  define genericptr	char *
# endif

/*
 * Traditional C compilers don't have "signed", "const", or "volatile".
 */
# define signed
# define const
# define volatile

#endif /* NHSTDC */


#ifndef genericptr_t
typedef genericptr genericptr_t;	/* (void *) or (char *) */
#endif


/*
 * According to ANSI, prototypes for old-style declarations must widen the
 * arguments to int.  However, the MSDOS compilers accept shorter arguments
 * (char, short, etc.) in prototypes and do typechecking with them.  Therefore
 * this mess to allow the better typechecking while also allowing some
 * prototypes for the ANSI compilers so people quit trying to fix the
 * prototypes to match the standard and thus lose the typechecking.
 */
#if defined(MSDOS) && !defined(__TURBOC__) && !defined(__GO32__)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(AMIGA) && !defined(AZTEC_50)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(MAC) && !defined(THINK_C)
#define UNWIDENED_PROTOTYPES
#endif

#if defined(ULTRIX_PROTO) && defined(ULTRIX_CC20)
#define UNWIDENED_PROTOTYPES
#endif
#if defined(apollo)
#define UNWIDENED_PROTOTYPES
#endif

#ifndef UNWIDENED_PROTOTYPES
# if defined(NHSTDC) || defined(__TURBOC__) || defined(ULTRIX_PROTO) || defined(THINK_C)
# define WIDENED_PROTOTYPES
# endif
#endif

#if defined(sgi) && !defined(__GNUC__)
/*
 * As of IRIX 4.0.1, /bin/cc claims to be an ANSI compiler, but it thinks
 * it's impossible for a prototype to match an old-style definition with
 * unwidened argument types.  Thus, we have to turn off all NetHack
 * prototypes, and avoid declaring several system functions, since the system
 * include files have prototypes and the compiler also complains that
 * prototyped and unprototyped declarations don't match.
 */
# undef NDECL
# undef FDECL
# undef VDECL
# define NDECL(f)	f()
# define FDECL(f,p)	f()
# define VDECL(f,p)	f()
#endif


#ifdef __HC__	/* MetaWare High-C defaults to unsigned chars */
# undef signed
#endif


/*
 * Allow gcc2 to check parameters of printf-like calls with -Wformat;
 * append this to a prototype declaration (see pline() in extern.h).
 */
#ifdef __GNUC__
# if __GNUC__ >= 2
#define PRINTF_F(f,v) __attribute__ ((format (printf, f, v)))
# endif
#endif
#ifndef PRINTF_F
#define PRINTF_F(f,v)
#endif

#endif /* TRADSTDC_H */
