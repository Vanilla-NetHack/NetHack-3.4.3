/*	SCCS Id: @(#)mactty.h	3.1	93/03/01			*/
/* Copyright (c) Jon W{tte 1993.					*/
/* NetHack may be freely redistributed.  See license for details.	*/

/*
 * This header is the supported external interface for the "tty" window
 * package. This package sports care-free handling of "dumb" tty windows 
 * (preferrably using monospaced fonts) - it does NOT remember the strings
 * sent to it; rather, it uses an offscreen bitmap.
 * 
 * For best performance, make sure it is aligned on a 32-pixel boundary
 * (or at least a 16-pixel one) in black & white. For 24bit color,
 * alignment doesn't matter, and for 8-bit color, alignment to every
 * fourth pixel is most efficient.
 *
 * (c) Copyright 1993 Jon W{tte
 */

/*
 * You should really not poke in the structures used by the tty window.
 * However, since it uses the wRefCon of windows (by calling GetWRefCon
 * and SetWRefCon) you lose that possibility. If you still want to store
 * information about a window, the FIRST location _pointed to_ by the
 * wRefCon will be a void * that you can use for whatever reasons. Don't
 * take the address of this variable and expect it to stay the same
 * across calls to the tty window.
 *
 * void * my_config_ptr = * ( void * * ) GetWRefCon ( tty_window ) ;
 */

/*
 * The library uses the window's port temporarily through SetPortBits;
 * that means you shouldn't do any funky things to the clipping region
 * etc. Actually, you shouldn't even resize the window, as that will clip
 * new drawing.
 *
 * Also, if you use this library under Pascal, remember that the string
 * passed to add_tty_string() is a "C" style string with NO length byte,
 * and a terminating zero byte at the end instead.
 */

#ifndef _H_tty_public
# define _H_tty_public

#include <Windows.h>
#include <QDOffscreen.h>
#include <Fonts.h>
#include <Memory.h>
#include <OSUtils.h>
#include <Errors.h>
#include <GestaltEqu.h>
#include <Palettes.h>
#include <Desk.h>
#include <DiskInit.h>
#include <OSEvents.h>


/*
 * If you export this library to Pascal, which doesn't have vsprintf
 * to rely on, you will need to change this define to 0
 */
#define PRINTF_TTY 1
/*
 * If you want the functions getchar_tty and gets_tty, you have to define
 * this. Currently not supported.
 */
#define TTY_INPUT 0
/*
 * If you want some fancy operations that not a normal TTY device normally
 * supports, use EXTENDED_SUPPORT. For frames, area erases and area scrolls,
 * plus bitmap graphics - RESOLUTION DEPENDENT, be sure to call
 * get_tty_metrics and use those limits.
 */
#define EXTENDED_SUPPORT 1
/*
 * if you print a lot of single characters, accumulating each one in a
 * clipping region will take too much time. Instead, define this, which
 * will clip in rects.
 */
#define CLIP_RECT_ONLY 1

typedef enum tty_attrib {

/*
 * Flags relating to the general functioning of the window.
 * These flags are passed at create_tty time, and changing them
 * later will clear the screen.
 */
	TTY_ATTRIB_FLAGS ,
/*
 * When using proportional fonts, this will place each character
 * separately, ensuring aligned columns (but looking ugly and taking
 * time)
 */
# define TA_MOVE_EACH_CHAR 1L
/*
 * This means draw each change as it occurs instead of collecting the area
 * and draw it all at once at update_tty() - slower, but more reliable.
 */
# define TA_ALWAYS_REFRESH 2L
/*
 * When reaching the right end, we either just stop drawing, or wrap to the
 * next line.
 */
# define TA_WRAP_AROUND 4L
/*
 * Overstrike means that characters are added on top of each other; i e don't
 * clear the letter beneath. This is faster, using srcOr under QuickDraw
 */
# define TA_OVERSTRIKE 8L
/*
 * We may want the window not to scroll when we reach the end line,
 * but stop drawing instead.
 */
# define TA_INHIBIT_VERT_SCROLL 16L

/*
 * Foreground and background colors. These only affect characters
 * drawn by subsequent calls; not what's already there (but it does
 * affect clears)
 * On b/w screens these do nothing.
 */
	TTY_ATTRIB_FOREGROUND ,
	TTY_ATTRIB_BACKGROUND ,
# define TA_RGB_TO_TTY(r) ((((long)((r).red>>8)&0xff)<<16)+\
	(((long)((r).green>>8)&0xff)<<8)+((long)((r).blue>>8)&0xff))

/*
 * Attributes relating to the cursor, and character set mappings
 */
	TTY_ATTRIB_CURSOR ,
/*
 * Blinking cursor is more noticeable when it's idle
 */
# define TA_BLINKING_CURSOR 1L
/*
 * When handling input, do we echo characters as they are typed?
 */
# define TA_ECHO_INPUT 2L
/*
 * Do we return each character code separately, or map delete etc? Note
 * that non-raw input means getchar won't return anything until the user
 * has typed a return.
 */
# define TA_RAW_INPUT 4L
/*
 * Do we print every character as it is (including BS, NL and CR!) or do
 * do we interpret characters such as NL, BS and CR?
 */
# define TA_RAW_OUTPUT 8L
/*
 * When getting a NL, do we also move to the left?
 */
# define TA_NL_ADD_CR 16L
/*
 * When getting a CR, do we also move down?
 */
# define TA_CR_ADD_NL 32L
/*
 * Wait for input or return what we've got?
 */
# define TA_NONBLOCKING_IO 64L

/*
 * A callback function for command keys entered when locked in an input loop.
 * Calling convention:
 * pascal void callback_function ( EventRecord * event , WindowPtr window ) ;
 */
	TTY_COMMAND_KEY_CALLBACK ,
/*
 * This function is called to allocate memory for the window. Note that
 * create_tty doesn't create any memory (except that created by GetNewWindow)
 * but init_tty_name and init_tty_number do allocate memory.
 * Calling convention:
 * pascal short allocate_memory ( WindowPtr window , void * * to_alloc ,
 *     long size ) ;
 * should return 0 for success, and error code for fail.
 */
	TTY_ALLOCATE_MEMORY_FUNCTION ,
/*
 * This is the counterpart to the allocate function, called to free memory.
 * Calling convention:
 * pascal short free_memory ( WindowPtr window , void * to_free ) ;
 * should return 0 for success, and error code for fail.
 */
	TTY_FREE_MEMORY_FUNCTION ,
/*
 * Use this function to beep, for instance for too large buffer or for
 * printing a bell character in non-RAW mode.
 * pascal void ( * tty_beep ) ( WindowPtr window ) ;
 */
 	TTY_BEEP_FUNCTION ,
/*
 * Use this macro to cast a function pointer to a tty attribute; this will help
 * portability to systems where a function pointer doesn't fit in a long
 */
# define TA_ATTRIB_FUNC(x) ((long)(x))

/*
 * This symbolic constant is used to check the number of attributes
 */
	TTY_NUMBER_ATTRIBUTES

} tty_attrib ;

/*
 * Character returned by end-of-file condition
 */
# define TTY_EOF -1


/*
 * Create the window according to a resource WIND template.
 * The window pointer pointed to by window should be NULL to
 * allocate the window record dynamically, or point to a
 * WindowRecord structure already allocated.
 *
 * Passing in_color means you have to be sure there's color support;
 * on the Mac, this means 32bit QuickDraw present, else it will
 * crash. Not passing in_color means everything's rendered in
 * black & white.
 */
extern pascal short create_tty ( WindowPtr * window , short resource_id ,
	Boolean in_color ) ;

/*
 * Use init_tty_name or init_tty_number to initialize a window
 * once allocated by create_tty. Size parameters are in characters.
 */
extern pascal short init_tty_name ( WindowPtr window ,
	unsigned char * font_name , short font_size , short x_size ,
	short y_size ) ;
extern pascal short init_tty_number ( WindowPtr window , short font_number ,
	short font_size , short x_size , short y_size ) ;

/*
 * Close and deallocate a window and its data
 */
extern pascal short destroy_tty ( WindowPtr window ) ;

/*
 * Change the font and font size used in the window for drawing after
 * the calls are made. To change the coordinate system, be sure to call
 * force_tty_coordinate_system_recalc() - else it may look strange if
 * the new font doesn't match the old one.
 */
extern pascal short set_tty_font_name ( WindowPtr window ,
	unsigned char * name ) ;
extern pascal short set_tty_font_number ( WindowPtr window , short number ) ;
extern pascal short set_tty_font_size ( WindowPtr window , short size ) ;
extern pascal short force_tty_coordinate_system_recalc ( WindowPtr window ) ;

/*
 * Getting some metrics about the tty and its drawing.
 */
extern pascal short get_tty_metrics ( WindowPtr window , short * x_size ,
	short * y_size , short * x_size_pixels , short * y_size_pixels ,
	short * font_number , short * font_size ,
	short * char_width , short * row_height ) ;

/*
 * The basic move cursor function. 0,0 is topleft.
 */
extern pascal short move_tty_cursor ( WindowPtr window , short x_pos ,
	short y_pos ) ;

/*
 * Return the location of the tty cursor
 */
extern pascal short get_tty_cursor ( WindowPtr window , short * x_pos ,
	short * y_pos ) ;

/*
 * Flush all changes done to a tty to the screen (see TA_ALWAYS_UPDATE above)
 */
extern pascal short update_tty ( WindowPtr window ) ;

/*
 * Add a character to the tty and update the cursor position
 */
extern pascal short add_tty_char ( WindowPtr window , short character ) ;

/*
 * Add a string of characters to the tty and update the cursor
 * position. The string is 0-terminated!
 */
extern pascal short add_tty_string ( WindowPtr window , const char * string ) ;

#if PRINTF_TTY
/*
 * The cool, standard C printf function, here for convenience. Requires
 * vsprintf in the libraries. Change the PRINTF_TTY define to remove it.
 */
extern short printf_tty ( WindowPtr window , const char * format , ... ) ;
#endif

/*
 * Change or read an attribute of the tty. Note that some attribute changes
 * may clear the screen. See the above enum and defines for values.
 * Attributes can be both function pointers and special flag values.
 */
extern pascal short get_tty_attrib ( WindowPtr window , tty_attrib attrib ,
	long * value ) ;
extern pascal short set_tty_attrib ( WindowPtr window , tty_attrib attrib ,
	long value ) ;

/*
 * Scroll the actual TTY image, in characters, positive means up/left
 * scroll_tty ( my_tty , 0 , 1 ) means a linefeed. Is always carried out
 * directly, regardless of the wait-update setting. Does updates before
 * scrolling.
 */
extern pascal short scroll_tty ( WindowPtr window , short delta_x ,
	short delta_y ) ;

/*
 * Erase the offscreen bitmap and move the cursor to 0,0. Re-init some window
 * values (font etc) Is always carried out directly on-screen, regardless of
 * the wait-for-update setting. Clears update area.
 */
extern pascal short clear_tty ( WindowPtr window ) ;

/*
 * We changed our mind about the size we want to draw in - in characters.
 * You need to change the window size separately with SizeWindow.
 */
extern pascal short resize_tty_area ( WindowPtr window , short x_size ,
	short y_size ) ;

/*
 * Call this function after calling WaitNextEvent in your program. It will
 * return 0 if this was an event handled by the tty window, and you can go
 * right back to calling WaitNextEvent. Non-0 means you handle the event.
 */
extern pascal short handle_tty_event ( WindowPtr window ,
	EventRecord * event ) ;

/*
 * For screen dumps, open the printer port and call this function. Can be used
 * for clipboard as well (only for a PICT, though; this library doesn't concern
 * itself with characters, just bitmaps)
 */
extern pascal short image_tty ( WindowPtr window ) ;

#if TTY_INPUT
/*
 * Enter an internal event loop to read a character, that is returned in the
 * character parameter. Note that several callback attributes may be called
 * here to handle events the tty cannot handle itself. To handle command keys,
 * you can install a command key handler. Note that if input isn't raw, you
 * will not get any input until return is pressed.
 */
extern pascal short getchar_tty ( WindowPtr window , short * character ) ;

/*
 * Read an entire string, bounded by the length specified.
 */
extern pascal short gets_tty ( WindowPtr window , char * buffer ,
	short buffer_length ) ;

#endif /* TTY_INPUT */

#if EXTENDED_SUPPORT

/*
 * Various versions of delete character/s, insert line/s etc can be handled by
 * this general-purpose function. Negative num_ means delete, positive means
 * insert, and you can never be sure which of row and col operations come first
 * if you specify both...
 */
extern pascal short mangle_tty_rows_columns ( WindowPtr window ,
	short from_row , short num_rows , short from_col , short num_cols ) ;

/*
 * For erasing just an area of characters
 */
extern pascal short clear_tty_window ( WindowPtr window , short from_row ,
	short from_col , short to_row , short to_col ) ;

/*
 * For framing an area without using grahpics characters.
 * Note that the given limits are those used for framing, you should not
 * draw in them. frame_fatness should typically be 1-5, and may be clipped
 * if it is too large.
 */
extern pascal short frame_tty_window ( WindowPtr window , short from_row ,
	short from_col , short to_row , short to_col , short frame_fatness ) ;

/*
 * For inverting specific characters after the fact. May look funny in color.
 */
extern pascal short invert_tty_window ( WindowPtr window , short from_row ,
	short from_col , short to_row , short to_col ) ;

/*
 * For drawing lines on the tty - VERY DEVICE DEPENDENT. Use get_tty_metrics.
 */
extern pascal short draw_tty_line ( WindowPtr window , short from_x ,
	short from_y , short to_x , short to_y ) ;

#endif /* EXTENDED_SUPPORT */

#endif /* _H_tty_public */
