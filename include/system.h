/*	SCCS Id: @(#)system.h 3.0	88/10/10 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#define E extern

#ifdef AMIGA
#define _SIZE_T
typedef unsigned int	size_t;
#else
# include <sys/types.h>
#endif

#ifdef ULTRIX
/* The Ultrix v3.0 <sys/types.h> seems to be very wrong. */
#define off_t long
#define time_t long
#endif

/* some old <sys/types.h> may not define off_t and size_t; if your system is
 * one of these, define them here
 */
#ifdef MSDOS
# ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int	size_t;
# endif
# ifdef __TURBOC__
typedef long  off_t;
# endif
#endif

/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#if defined(__STDC__) || defined(ULTRIX)
	/* also SVR3 and later, Sun4.0 and later */
#define SIG_RET_TYPE void (*)()
#else
	/* BSD, SIII, SVR2 and earlier, Sun3.5 and earlier */
#define SIG_RET_TYPE int (*)()
#endif

#if defined(BSD) || defined(ULTRIX) || defined(RANDOM)
E long random();
E void srandom P((unsigned int));
#else
E long lrand48();
E void srand48();
#endif /* BSD || ULTRIX || RANDOM */

#if !defined(BSD) || defined(ultrix)
			/* real BSD wants all these to return int */
# ifndef MSDOS
E void exit P((int));
# endif /* MSDOS */
E void free P((genericptr_t));
E void perror P((const char *));
#endif

#if defined(BSD) || defined(ULTRIX)
E int qsort();
#else
E void qsort P((genericptr_t,size_t,size_t,int(*)(genericptr_t,genericptr_t)));
#endif

#ifdef ULTRIX
E long lseek P((int,off_t,int));
  /* Ultrix 3.0 man page mistakenly says it returns an int. */
E int write P((int,char *,int));
#else
E long lseek P((int,long,int));
E int write P((int,genericptr_t,unsigned));
#ifdef MSDOS
E int close P((int));
E int read P((int,genericptr_t,unsigned int));
E int open P((const char *,int,...));
E int dup2 P((int, int));
E int setmode P((int,int));
E int kbhit P((void));
#endif
#endif /* ULTRIX */

#ifdef MSDOS
E int chdir P((char *));
E char *getcwd P((char *,int));
#endif

/* both old & new versions of Ultrix want these, but real BSD does not */
#ifdef ultrix
E void abort();
E void bcopy();
#endif
#ifdef MSDOS
E void abort P((void));
E void _exit P((int));
E int system P((const char *));
#endif

#ifdef SYSV
E char *memcpy();
#endif
#ifdef MSDOS
E int memcmp P((char *,char *,unsigned int));
E char *memcpy P((char *,char *,unsigned int));
E char *memset P((char*,int,int));
#endif

#if defined(BSD) && defined(ultrix)	/* i.e., old versions of Ultrix */
E void sleep();
#endif
#if defined(ULTRIX) || defined(SYSV)
E unsigned sleep();
#endif

E char *getenv P((const char *));
E char *getlogin();
E int getpid();

/*# string(s).h #*/

E char	*strcpy P((char *,const char *));
E char	*strncpy P((char *,const char *,size_t));
E char	*strcat P((char *,const char *));
E char	*strncat P((char *,const char *,size_t));

#if defined(SYSV) || defined(MSDOS)
E char	*strchr P((const char *,int));
E char	*strrchr P((const char *,int));
#else /* BSD */
E char	*index P((const char *,int));
E char	*rindex P((const char *,int));
#endif


E int	strcmp P((const char *,const char *));
E int	strncmp P((const char *,const char *,size_t));
#ifdef MSDOS
E size_t strlen P((const char *));
#else
E int	strlen();
#endif

/* Old varieties of BSD have char *sprintf().
 * Newer varieties of BSD have int sprintf() but allow for the old char *.
 * Several varieties of SYSV and PC systems also have int sprintf().
 * If your system doesn't agree with this breakdown, you may want to change
 * this declaration, especially if your machine treats the types differently.
 */
#if defined(BSD) || defined(ULTRIX)
#define OLD_SPRINTF
E char *sprintf();
#else
E int sprintf P((char *,const char *,...));
#endif

#define Sprintf	(void) sprintf
#define Strcat	(void) strcat
#define Strcpy	(void) strcpy
#define Printf  (void) printf

E int tgetent P((char *,char *));
E int tgetnum P((char *));
E int tgetflag P((char *));
E char *tgetstr P((char *,char **));
E char *tgoto P((char *,int,int));
E void tputs P((char *,int,int (*)()));

E genericptr_t malloc P((size_t));

/* time functions */

E struct tm *localtime P((const time_t *));

#if (defined(ULTRIX) || defined(SYSV) || defined(MSDOS)) && !defined(AMIGA)
E time_t time P((time_t *));
#else
E long time P((time_t *));
#endif /* ULTRIX */

#ifdef MSDOS
E int abs P((int));
E int atoi P((char *));
#endif

#undef E

#endif /* SYSTEM_H */
