/*    SCCS Id: @(#)amitty.c     3.1    93/01/08
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland 1993	  */
/* NetHack may be freely redistributed.  See license for details. */

/* TTY-specific code for the Amiga */

/* Still to do:
 * add command line switches for enough control to use as BBS door
 * add realy termcap handling - currently requires ANSI_DEFAULT
 * fix tids everywhere
 * fix commented out code that tries (and fails) to avoid problems with
 *  typeahead - we may need to resort to basic packet I/O. Sigh.
 * prototype and related cleanup
 */

#include "hack.h"
#include "termcap.h"
#include <stdio.h>

void NDECL( get_scr_size );
void NDECL( tty_change_color );
char *NDECL( tty_get_color_string );

#ifdef TTY_GRAPHICS
void settty(const char *s){
	end_screen();
	if(s)raw_print(s);
	flags.cbreak=ON;	/* this is too easy: probably wrong */
#if 0 /* should be version>=36 */
	if(IsInteractive(Input())){
		SetMode(Input(),0);	/* con mode */
	}
#endif
}
void gettty(){
#if 0 /* should be VERSION >=36 */
	if(IsInteractive(Input())){
		SetMode(Input(),1);	/* raw mode */
	}
#endif
}
void setftty(){
	flags.cbreak=ON;	/* ditto */
}
char kill_char='X'-'@';
char erase_char='\b';
tgetch(){
#if 1
	int x=getch();		/* can't use getch() - typeahead ends up stalling the
				 * game (since it's a con:) */
#else
	int x;
	Read(Input(),&x,1);
#endif
	return (x=='\r')?'\n':x;
}
void get_scr_size(){
	CO=80;
	LI=24;
}

#endif

void tty_change_color() {}
char *tty_get_color_string() { return( "" ); }
