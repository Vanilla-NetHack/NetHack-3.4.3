/*	SCCS Id: @(#)ioctl.c	2.0	87/09/18
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This cannot be part of hack.tty.c (as it was earlier) since on some
   systems (e.g. MUNIX) the include files <termio.h> and <sgtty.h>
   define the same constants, and the C preprocessor complains. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#define MONFLAG_H
#include "hack.h"

#if defined(BSD) || defined(ULTRIX) || defined(HPUX)
# ifdef HPUX
#include	<bsdtty.h>
# else
#include	<sgtty.h>
# endif
struct ltchars ltchars;
struct ltchars ltchars0 = { -1, -1, -1, -1, -1, -1 }; /* turn all off */
#else
#include	<termio.h>	/* also includes part of <sgtty.h> */
struct termio termio;
# ifdef AMIX
#include <sys/ioctl.h>
# endif /* AMIX */
#endif

void
getioctls() {
#if defined(BSD) || defined(ULTRIX) || defined(HPUX)
	(void) ioctl(fileno(stdin), (int) TIOCGLTC, (char *) &ltchars);
	(void) ioctl(fileno(stdin), (int) TIOCSLTC, (char *) &ltchars0);
#else
	(void) ioctl(fileno(stdin), (int) TCGETA, &termio);
#endif
#if defined(TIOCGWINSZ) && (defined(BSD) || defined(ULTRIX))
	{
		/*
		 * ttysize is found on Suns and BSD
		 * winsize is found on Suns, BSD, and Ultrix
		 */
		struct winsize ttsz;

		if (ioctl(fileno(stdin), (int)TIOCGWINSZ, (char *)&ttsz) != -1)
		  {
		    /*
		     * Use the kernel's values for lines and columns if it has
		     * any idea.
		     */
		    if (ttsz.ws_row)
		      LI = ttsz.ws_row;
		    if (ttsz.ws_col)
		      CO = ttsz.ws_col;
		  }
	}
#endif
}

void
setioctls() {
#if defined(BSD) || defined(ULTRIX) || defined(HPUX)
	(void) ioctl(fileno(stdin), (int) TIOCSLTC, (char *) &ltchars);
#else
	/* Now modified to run under Sys V R3.	- may have to be #ifdef'ed */
	(void) ioctl(fileno(stdin), (int) TCSETAW, &termio);
#endif
}

#ifdef SUSPEND		/* implies BSD */
int
dosuspend() {
#include	<signal.h>
#ifdef SIGTSTP
	if(signal(SIGTSTP, SIG_IGN) == SIG_DFL) {
		settty(NULL);
		(void) signal(SIGTSTP, SIG_DFL);
		(void) kill(0, SIGTSTP);
		gettty();
		setftty();
		docrt();
	} else {
		pline("I don't think your shell has job control.");
	}
#else
	pline("Sorry, it seems we have no SIGTSTP here.  Try ! or S.");
#endif
	return(0);
}
#endif /* SUSPEND /**/
