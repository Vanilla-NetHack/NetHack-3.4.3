/*	SCCS Id: @(#)windows.c	3.1	93/01/08	*/
/* Copyright (c) D. Cohrs, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif
#ifdef X11_GRAPHICS
/* cannot just blindly include winX.h without including all of X11 stuff */
/* and must get the order of include files right.  Don't bother */
extern struct window_procs X11_procs;
extern void NDECL(win_X11_init);
#endif
#ifdef MAC
extern struct window_procs mac_procs ;
#endif
#ifdef AMIGA_INTUITION
extern struct window_procs amii_procs ;
#endif

struct window_procs NEARDATA windowprocs;

static
struct win_choices {
    struct window_procs *procs;
    void NDECL((*ini_routine));		/* optional (can be 0) */
} winchoices[] = {
#ifdef TTY_GRAPHICS
    { &tty_procs, win_tty_init },
#endif
#ifdef X11_GRAPHICS
    { &X11_procs, win_X11_init },
#endif
#ifdef MAC
	{ & mac_procs , NULL } ,
#endif
#ifdef AMIGA_INTUITION
	{ & amii_procs , NULL } ,
#endif
    { 0, 0 }		/* must be last */
};

void
choose_windows(s)
const char *s;
{
    register int i;

    for(i=0; winchoices[i].procs; i++)
	if (!strcmpi(s, winchoices[i].procs->name)) {
	    windowprocs = *winchoices[i].procs;
	    if (winchoices[i].ini_routine) (*winchoices[i].ini_routine)();
	    return;
	}

    raw_printf("Window type %s not recognized.  Choices are:", s);
    for(i=0; winchoices[i].procs; i++)
	raw_printf("        %s", winchoices[i].procs->name);
}

/*windows.c*/
