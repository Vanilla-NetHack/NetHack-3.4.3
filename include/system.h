/*	SCCS Id: @(#)system.h 3.0	88/10/10 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#define E extern

/* some old <sys/types.h> may not define off_t and size_t; if your system is
 * one of these, define them by hand below
 */
#if !defined(THINKC4) && !defined(AMIGA) && !defined(MACOS)
# include <sys/types.h>
#endif

#if defined(TOS) && defined(__GNUC__) && !defined(_SIZE_T)
# define _SIZE_T
#endif

#if defined(MSDOS) || ((defined(AMIGA) || defined(MACOS)) && !defined(THINKC4))
# ifndef _SIZE_T
#  define _SIZE_T
typedef unsigned int	size_t;
# endif
#endif

#ifdef ULTRIX
/* The Ultrix v3.0 <sys/types.h> seems to be very wrong. */
# define time_t long
#endif
#if defined(ULTRIX) || defined(VMS)
# define off_t long
#endif
#if defined(AZTEC) || defined(THINKC4) || (defined(MSDOS) && defined(__TURBOC__))
typedef long	off_t;
#endif


/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#if defined(__STDC__) || defined(ULTRIX)
	/* also SVR3 and later, Sun4.0 and later */
# define SIG_RET_TYPE void (*)()
#else
	/* BSD, SIII, SVR2 and earlier, Sun3.5 and earlier */
# define SIG_RET_TYPE int (*)()
#endif

#if defined(BSD) || defined(ULTRIX) || defined(RANDOM)
E long random();
E void FDECL(srandom, (unsigned int));
#else
E long lrand48();
E void srand48();
#endif /* BSD || ULTRIX || RANDOM */

#if !defined(BSD) || defined(ultrix)
			/* real BSD wants all these to return int */
# ifndef MSDOS
E void FDECL(exit, (int));
# endif /* MSDOS */
E void FDECL(free, (genericptr_t));
# ifdef AMIGA
E int FDECL(perror, (const char *));
# else
#  ifndef MACOS
E void FDECL(perror, (const char *));
#  endif
# endif
#endif
#if defined(BSD) || defined(ULTRIX) || (defined(MACOS) && !defined(THINKC4))
E int qsort();
#else
# ifndef LATTICE
E void FDECL(qsort, (genericptr_t,size_t,size_t,int(*)(genericptr_t,genericptr_t)));
# endif
#endif

#ifdef ULTRIX
E long FDECL(lseek, (int,off_t,int));
  /* Ultrix 3.0 man page mistakenly says it returns an int. */
E int FDECL(write, (int,char *,int));
#else
E long FDECL(lseek, (int,long,int));
E int FDECL(write, (int,genericptr_t,unsigned));
#endif /* ULTRIX */
E int FDECL(unlink, (const char *));

#ifdef MSDOS
E int FDECL(close, (int));
E int FDECL(read, (int,genericptr_t,unsigned int));
E int FDECL(open, (const char *,int,...));
E int FDECL(dup2, (int, int));
E int FDECL(setmode, (int,int));
E int FDECL(kbhit, (void));
E int FDECL(chdir, (char *));
E char *FDECL(getcwd, (char *,int));
#endif

#ifdef TOS
E int FDECL(creat, (const char *, int));
#endif

/* both old & new versions of Ultrix want these, but real BSD does not */
#ifdef ultrix
E void abort();
E void bcopy();
#endif
#ifdef MSDOS
E void FDECL(abort, (void));
E void FDECL(_exit, (int));
E int FDECL(system, (const char *));
#endif
#ifdef HPUX
E long FDECL(fork, (void));
#endif

#ifdef SYSV
E char *memcpy();
#endif
#ifdef HPUX
E void *FDECL(memcpy, (char *,char *,int));
E int FDECL(memcmp, (char *,char *,int));
E void *FDECL(memset, (char*,int,int));
#endif
#ifdef MSDOS
# if defined(TOS) && defined(__GNUC__)
E int FDECL(memcmp, (const char *,const char *,size_t));
E char *FDECL(memcpy, (char *,const char *,size_t));
E char *FDECL(memset, (char*,int,size_t));
# else
#  ifndef LATTICE
#    ifdef MSC
void * _CDECL memcpy(void *, const void *, size_t);
void * _CDECL memset(void *, int, size_t);
#    else
E int FDECL(memcmp, (char *,char *,unsigned int));
E char *FDECL(memcpy, (char *,char *,unsigned int));
E char *FDECL(memset, (char*,int,int));
#    endif
#  endif
# endif /* TOS */
#endif

#if defined(BSD) && defined(ultrix)	/* i.e., old versions of Ultrix */
E void sleep();
#endif
#if defined(ULTRIX) || defined(SYSV)
E unsigned sleep();
#endif
#if defined(HPUX)
E unsigned int FDECL(sleep, (unsigned int));
#endif

E char *FDECL(getenv, (const char *));
E char *getlogin();
#ifdef HPUX
E long FDECL(getuid, (void));
E long FDECL(getgid, (void));
E long FDECL(getpid, (void));
#else
E int FDECL(getpid, (void));
#endif

/*# string(s).h #*/

E char	*FDECL(strcpy, (char *,const char *));
E char	*FDECL(strncpy, (char *,const char *,size_t));
E char	*FDECL(strcat, (char *,const char *));
E char	*FDECL(strncat, (char *,const char *,size_t));

#if defined(SYSV) || defined(MSDOS) || defined(AMIGA) || defined(THINK_C) || defined(VMS) || defined(HPUX)
E char	*FDECL(strchr, (const char *,int));
E char	*FDECL(strrchr, (const char *,int));
#else /* BSD */
E char	*FDECL(index, (const char *,int));
E char	*FDECL(rindex, (const char *,int));
#endif


E int	FDECL(strcmp, (const char *,const char *));
E int	FDECL(strncmp, (const char *,const char *,size_t));
#ifdef MSDOS
E size_t FDECL(strlen, (const char *));
#else
# ifdef HPUX
E unsigned int	FDECL(strlen, (char *));
# else
#  ifdef THINKC4
E size_t	FDECL(strlen, (char *));
#  else
E int	FDECL(strlen, (char *));
#  endif /* THINKC4 */
# endif /* HPUX */
#endif /* MSDOS */

/* Old varieties of BSD have char *sprintf().
 * Newer varieties of BSD have int sprintf() but allow for the old char *.
 * Several varieties of SYSV and PC systems also have int sprintf().
 * If your system doesn't agree with this breakdown, you may want to change
 * this declaration, especially if your machine treats the types differently.
 */
#if (defined(BSD) || defined(ULTRIX)) && !defined(DGUX) && !defined(NeXT)
# define OLD_SPRINTF
E char *sprintf();
#else
# ifndef TOS	/* problem with prototype mismatches with <stdio.h> */
E int FDECL(sprintf, (char *,const char *,...));
# endif
#endif

#ifdef NEED_VARARGS
# if defined(USE_STDARG) || defined(USE_VARARGS)
E int FDECL(vsprintf, (char *, const char *, va_list));
E int FDECL(vprintf, (const char *, va_list));
# else
#  define vprintf	printf
#  define vsprintf	sprintf
#  define vpline	pline
# endif
#endif /* NEED_VARARGS */

#define Sprintf	(void) sprintf
#define Strcat	(void) strcat
#define Strcpy	(void) strcpy

#if defined(MACOS) && defined(CUSTOM_IO)
# undef printf
# undef puts
# undef putchar
# undef putc
# define printf  (void) mprintf
# define puts	 mputs
# define putchar mputc
# define putc	 mputc
# define Printf  (void) mprintf
#else
# define Printf  (void) printf
#endif

#ifdef NEED_VARARGS
# define Vprintf (void) vprintf
# define Vsprintf (void) vsprintf
#endif

#ifdef TOS
E int FDECL(tgetent, (const char *,const char *));
E int FDECL(tgetnum, (const char *));
E int FDECL(tgetflag, (const char *));
E char *FDECL(tgetstr, (const char *,char **));
E char *FDECL(tgoto, (const char *,int,int));
E void FDECL(tputs, (const char *,int,int (*)()));
#else
E int FDECL(tgetent, (char *,char *));
E int FDECL(tgetnum, (char *));
E int FDECL(tgetflag, (char *));
E char *FDECL(tgetstr, (char *,char **));
E char *FDECL(tgoto, (char *,int,int));
E void FDECL(tputs, (char *,int,int (*)()));
#endif

#ifndef MACOS
E genericptr_t FDECL(malloc, (size_t));
#endif

/* time functions */

#ifndef MACOS
# ifndef LATTICE
E struct tm *FDECL(localtime, (const time_t *));
# endif

# if defined(ULTRIX) || defined(SYSV) || defined(MSDOS)
E time_t FDECL(time, (time_t *));
# else
E long FDECL(time, (time_t *));
# endif /* ULTRIX */
#endif

#ifdef MSDOS
# ifdef abs
# undef abs
# endif
E int FDECL(abs, (int));
E int FDECL(atoi, (const char *));
#endif

#undef E

#endif /* SYSTEM_H */
