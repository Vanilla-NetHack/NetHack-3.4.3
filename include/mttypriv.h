/*	SCCS Id: @(#)mttypriv.h	3.1	93/03/01			*/
/* Copyright (c) Jon W{tte 1993.					*/
/* NetHack may be freely redistributed.  See license for details.	*/

/*
 * This file contains private structures used to implement the
 * tty windows - note that these structures may change between
 * minor releases!
 */

#ifndef _H_tty_private
# define _H_tty_private

# ifndef _H_tty_public
#  include "mactty.h"
# endif

#define TA_TO_RGB(ta,rgb) (((rgb).red=(((ta)>>16)&0xff)*257),((rgb).green=(((ta)>>8)&0xff)*257),\
	((rgb).blue=((ta)&0xff)*257)),rgb

typedef struct tty_record {
	void				* not_used ;
	WindowPtr			its_window ;
	GWorldPtr			its_window_world ;
	BitMap				its_bits ;
	GrafPtr				offscreen_port ;
	GWorldPtr			offscreen_world ;
#if CLIP_RECT_ONLY
	Rect				invalid_rect ;
#else
	RgnHandle			invalid_part ;
#endif

	short				font_number ;
	short				font_size ;
	short				char_width ;
	short				row_height ;
	short				ascent_height ;

	short				x_size ;
	short				y_size ;
	short				x_curs ;
	short				y_curs ;

	long				attribute [ TTY_NUMBER_ATTRIBUTES ] ;
	long				last_cursor ;

#if TTY_INPUT
	short				input_buffer_len ;
	short				input_buffer_limit ;
	char				* input_buffer ;
#endif

	Boolean				was_allocated ;
	Boolean				curs_state ;
	Boolean				uses_gworld ;
} tty_record ;


#endif
