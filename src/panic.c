/*	SCCS Id: @(#)panic.c	3.0	88/05/03
 * Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985.
 *
 *	This code was adapted from the code in end.c to run in a standalone
 *	mode for the makedefs / drg code.
 */
/* NetHack may be freely redistributed.  See license for details. */

#include	"config.h"

#ifdef MSDOS
#undef exit
extern void exit P((int));
#endif
 
/*VARARGS1*/
boolean panicking;

void
panic(str,a1,a2,a3,a4,a5,a6)
char *str;
{
	if(panicking++)
#ifdef SYSV
	    (void)
#endif
		abort();    /* avoid loops - this should never happen*/

	(void) fputs(" ERROR:  ", stderr);
	Printf(str,a1,a2,a3,a4,a5,a6);
	(void) fflush(stderr);
#ifdef UNIX
# ifdef SYSV
		(void)
# endif
		    abort();	/* generate core dump */
#endif
	exit(1);		/* redundant */
	return;
}

