/*    SCCS Id: @(#)amitty.c     3.0    89/04/26
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* tty.c - (Amiga) version */


#include "hack.h"

extern int Enable_Abort;

char erase_char, kill_char;

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void gettty()
{
    erase_char = 127;	    /* DEL */
    kill_char = 21;	    /* cntl-U */
    flags.cbreak = TRUE;
    Enable_Abort = 0;
    curx = 1;
    cury = 1;
}

/* reset terminal to original state */
void settty(s)
char *s;
{
    end_screen();
    if (s) {
	printf(s);
    }
    (void) fflush(stdout);
    /* Do not close the screen, that is done in msexit() */
}


/* fatal error */
/*VARARGS1*/
void error(s,x,y)
char *s;
{
	end_screen();
	putchar('\n');
	printf(s,x,y);
	putchar('\n');
	abort(1);
}
