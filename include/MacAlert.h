/*	SCCS Id: @(#)MacAlert.h		3.0	90/01/06
/*      Copyright (c) Jon Watte  1989		*/ 
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MACALERT_H
#define MACALERT_H

#define MAtype 'MAlt'


typedef struct Malrt {
	int width;
	int height;
	int PICTno;
	char text[256];
	char but1[32];
	char but2[32];
	char but3[32];
	char but4[32];
	int def;
	int esc;
} MAlrt, * MAlrtPtr, ** MAlrtHandle;


extern int UseMacAlert(int MAno);
extern int UseMacAlertText(int MAno, char * txt);
extern int MacAlert(int width, int height, int PICTno, char * text,
	char * but1, char * but2, char * but3, char * but4, int def, int esc);
extern int TrackThem(Rect * b_rect, char * hi_lite, int no_butts);
extern int mac_more(FILE * fp, int strip);

/* Minimum values to which given values will be justified if lower */

#define MIN_WIDTH 170
#define MIN_HEIGHT 100

/* Good values for a "normal" alert */

#define WIDTH 250
#define HEIGHT 200

/* Constants to tweak how to draw the buttons */

#define BUT_HEIGHT 20
#define BUT_WIDTH 50
#define BUT_SPACING 10
#define BUT_MARGIN 5
#define BUT_CORNER 10

/* Frame around the default button */

#define FRAME_WIDTH 3
#define FRAME_OFFSET 1
#define FRAME_CORNER 16

/* How to draw the border */

#define BORDER_PAT 5
#define BORDER_WIDTH 3

/* How long to mark the pressed ? */

#define FLASH_TIME 10 /* In ticks */


#endif
