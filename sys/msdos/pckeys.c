/*	SCCS Id: @(#)pckeys.c	 3.2	 96/05/11		  */
/* Copyright (c) NetHack PC Development Team 1996                 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  MSDOS tile-specific key handling.
 */

#include "hack.h"

#ifdef MSDOS
# ifdef USE_TILES
#include "wintty.h" 
#include "pcvideo.h"

boolean FDECL(pckeys, (unsigned char, unsigned char));

extern struct WinDesc *wins[MAXWIN];	/* from wintty.c */
extern boolean tiles_on;		/* from video.c */
extern boolean traditional;		/* from video.c */
extern boolean inmap;			/* from video.c */
extern boolean overview;

#define SHIFT		(0x1 | 0x2)
#define CTRL		0x4
#define ALT		0x8

/*
 * Check for special interface manipulation keys.
 * Returns TRUE if the scan code triggered something.
 *
 */
boolean
pckeys(scancode, shift)
unsigned char scancode;
unsigned char shift;
{
   boolean opening_dialog;

   opening_dialog = pl_character[0] ? FALSE : TRUE;
#  ifdef SIMULATE_CURSOR
    switch(scancode) {
	case 0x3d:	/* F3 = toggle cursor type */
		HideCursor();
		cursor_type += 1;
		if (cursor_type >= NUM_CURSOR_TYPES) cursor_type = 0;
		DrawCursor();
		break;
#  endif
	case 0x74:	/* Control-right_arrow = scroll horizontal to right */
		if ((shift & CTRL) && tiles_on && !opening_dialog)
			vga_userpan(1);
		break;

	case 0x73:	/* Control-left_arrow = scroll horizontal to left */
		if ((shift & CTRL) && tiles_on && !opening_dialog)
			vga_userpan(0);
		break;
	case 0x3E:	/* F4 = toggle overview mode */
		if (tiles_on && !opening_dialog) {
			traditional = FALSE;
			vga_overview(overview ? 0 : 1);
		}
		break;
	case 0x3F:	/* F5 = toggle traditional mode */
		if (tiles_on && !opening_dialog) {
			overview = FALSE;
			vga_traditional(traditional ? 0 : 1);
		}
		break;
	default:
		return FALSE;
    }
    return TRUE;
}
# endif /* USE_TILES */
#endif /* MSDOS */

/*pckeys.c*/
