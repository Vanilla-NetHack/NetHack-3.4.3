/*	SCCS Id: @(#)amiconf.h  3.0     89/09/04
/* NetHack may be freely redistributed.  See license for details. */

#ifndef AMICONF_H
#define AMICONF_H

#undef abs		/* avoid using macro form of abs */
#undef min		/* this gets redefined */
#undef max		/* this gets redefined */

#include <time.h>	/* get time_t defined before use! */

#ifdef LATTICE		/* since Lattice can prevent re-inclusion */
#include <stdlib.h>	/* and other things, including builtins */
#if (__VERSION__==5) && (__REVISION__<=4)
    /* This enables a fix in src/mondata.c needed to bypass a compiler
    bug.  If you need it and don't have it you will get monsndx panics and
    monsters that change type when they die and become corpses.
    If you don't need it and do have it, even wierder things will happen. */
#   define LATTICE_504_BUG
#endif
#endif

#ifdef LATTICE		/* Lattice defines DEBUG when you use -d1 which */
# ifdef DEBUG		/* we need for useful SnapShots, but DEBUG is   */
#  undef DEBUG		/* used in several files to turn on things we   */
# endif			/* don't want (e.g. eat.c), so we get rid of    */
#endif			/* DEBUG unless asked for in a particular file  */

typedef long off_t;

#define MSDOS		/* must be defined to allow some inclusions */

#define O_BINARY	0

#define DGK		/* You'll probably want this; provides assistance
			 * for typical personal computer configurations
			 */
#define RANDOM

extern void FDECL(exit, (int));
extern void NDECL(CleanUp);
extern void FDECL(Abort, (long));
extern int NDECL(getpid);
extern char *FDECL(CopyFile, (const char *, const char *));
extern int NDECL(WindowGetchar);
extern void FDECL(WindowPutchar, (CHAR_P));
extern void FDECL(WindowPuts, (const char *));
extern void FDECL(WindowFPuts, (const char *));
extern void VDECL(WindowPrintf, (const char *, ...));
extern void FDECL(WindowFlush, (void));

#ifndef MSDOS_H
#include "msdos.h"
#endif
#ifndef PCCONF_H
#include "pcconf.h"     /* remainder of stuff is almost same as the PC */
#endif

#ifndef LATTICE
#define memcpy(dest, source, size)  movmem(source, dest, size)
#endif
#define remove(x)       unlink(x)

#ifdef LATTICE
#define FFLUSH(fp) _flsbf(-1, fp)    /* Was fflush */
#endif

#ifdef AZTEC_C
#define FFLUSH(fp) flsh_(fp, -1)     /* Was fflush */
extern FILE *FDECL(freopen, (const char *, const char *, FILE *));
extern char *FDECL(gets, (char *));
#endif

/* Use Window functions if not in makedefs.c or lev_lex.c */

#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

#define fopen	    fopenp	/* Open most text files according to PATH */

#undef getchar
#undef putchar
#undef fflush
# ifdef LATTICE
#undef printf
# endif

#define getchar()   WindowGetchar()
#define putchar(c)  WindowPutchar(c)
#define puts(s)     WindowPuts(s)
#define fputs(s,f)  WindowFPuts(s)
#define printf	    WindowPrintf
#define fflush(fp)  WindowFlush()

#define xputs	    WindowFPuts
#define xputc	    WindowPutchar

#endif

/*
 *  Configurable Amiga options:
 */

#define TEXTCOLOR		/* Use colored monsters and objects */
#define HACKFONT		/* Use special hack.font */
#define SHELL			/* Have a shell escape command (!) */
#define MAIL			/* Get mail at unexpected occasions */
#define AMIGA_WBENCH		/* Icon support */
#define DEFAULT_ICON "NetHack:default.icon"	/* private icon for above */
#undef	TERMLIB

#endif /* AMICONF_H */
