/*    SCCS Id: @(#)amitty.c     3.2    96/02/04
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland 1993,1996  */
/* NetHack may be freely redistributed.  See license for details. */

/* TTY-specific code for the Amiga
 * This is still experimental.
 * Still to do:
 * add real termcap handling - currently requires ANSI_DEFAULT
 */

#include "hack.h"
#include "termcap.h"
#include <stdio.h>

#ifdef _DCC
# define getch() getchar()
#endif
#ifdef __SASC_60
# include <clib/dos_protos.h>
#endif

void NDECL( tty_change_color );
char *NDECL( tty_get_color_string );

#ifdef TTY_GRAPHICS

extern long afh_in;

void settty(const char *s){
	end_screen();
	if(s)raw_print(s);
	iflags.cbreak=ON;	/* this is too easy: probably wrong */
#if 1 /* should be version>=36 */
/*	if(IsInteractive(afh_in)){ */
		SetMode(afh_in,0);	/* con mode */
/*	} */
#endif
}
void gettty(){
#if 1 /* should be VERSION >=36 */
/*	if(IsInteractive(afh_in)){ */
		SetMode(afh_in,1);	/* raw mode */
/*	} */
#endif
}
void setftty(){
	iflags.cbreak=ON;	/* ditto */
}
char kill_char='X'-'@';
char erase_char='\b';
tgetch(){
	char x;
	Read(afh_in,&x,1);
	return (x=='\r')?'\n':x;
}
void get_scr_size(){
	CO=80;
	LI=24;
}

#endif

void tty_change_color() {}
char *tty_get_color_string() { return( "" ); }
