/*	SCCS Id: @(#)pctty.c	3.0	87/05/03
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* tty.c - (PC) version */

#include "hack.h"

char erase_char, kill_char;

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty(){
	erase_char = '\b';
	kill_char = 21;		/* cntl-U */
	flags.cbreak = TRUE;
#ifndef TOS
	disable_ctrlP();	/* turn off ^P processing */
#endif
}

/* reset terminal to original state */
void
settty(s) char *s; {
	end_screen();
	if(s) Printf(s);
	(void) fflush(stdout);
#ifndef TOS
	enable_ctrlP();		/* turn on ^P processing */
#endif
}

/* fatal error */
/*VARARGS1*/
void
error(s,x,y)
char *s, *x, *y;
{
	end_screen();
	putchar('\n');
	Printf(s,x,y);
	putchar('\n');
	exit(1);
}
