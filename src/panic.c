/*	SCCS Id: @(#)panic.c	3.0	89/11/15
 * Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985.
 *
 *	This code was adapted from the code in end.c to run in a standalone
 *	mode for the makedefs / drg code.
 */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS
#include	"config.h"

#ifdef MSDOS
#undef exit
extern void FDECL(exit, (int));
#endif
#ifdef AZTEC
#define abort() exit()
#endif
 
/*VARARGS1*/
boolean panicking;

void
panic VA_DECL(char *,str)
	VA_START(str);
	VA_INIT(str, char *);
	if(panicking++)
#ifdef SYSV
	    (void)
#endif
		abort();    /* avoid loops - this should never happen*/

	(void) fputs(" ERROR:  ", stderr);
	Vprintf(str,VA_ARGS);
	(void) fflush(stderr);
#if defined(UNIX) || defined(VMS)
# ifdef SYSV
		(void)
# endif
		    abort();	/* generate core dump */
#endif
	VA_END();
	exit(1);		/* redundant */
	return;
}
