/*	SCCS Id: @(#)mactty.c	3.1	93/03/01			*/
/* Copyright (c) Jon W{tte 1993.					*/
/* NetHack may be freely redistributed.  See license for details.	*/

/*
 * mactty.c
 *
 * This file contains the actual code for the tty library. For a 
 * description, see the file mactty.h, which contains all you
 * need to know to use the library.
 */

#include "mttypriv.h"

#if PRINTF_TTY
# include <stdio.h>
# include <stdarg.h>
#endif

extern void dprintf ( char * , ... ) ;
static void select_onscreen_window ( tty_record * record ) ;
static void select_offscreen_port ( tty_record * record ) ;

#define MEMORY_MARGIN 30000


/*
 * Error code returned when it's probably our fault, or
 * bad parameters.
 */
#define general_failure 1

/*
 * How long lines do we support for input?
 */
#define IB_LIMIT 80

/*
 * Convenience macro for most functions - put last in declaration
 */
#define RECORD(record) \
tty_record * record ; \
	if ( ! window ) { \
		dprintf ( "*** NULL Window ***" ) ; \
		return general_failure ; \
	} \
	record = ( tty_record * ) GetWRefCon ( window ) ; \
	if ( ! record ) { \
		return general_failure ; \
	}

/*
 * Simple macro for deciding wether we draw at once or delay
 */
#define DRAW_DIRECT ( 0L != ( TA_ALWAYS_REFRESH & record -> \
	attribute [ TTY_ATTRIB_FLAGS ] ) )


/*
 * Module variable used as return value for various calls.
 */
static short s_err = 0 ;

/*
 * Table of special characters. Zero is ALWAYS special; it means
 * end of string and would be MISSED if it was not included here.
 */
static const unsigned char s_cooked_controls [ ] = {
	1,	0,	0,	0,	0,	0,	0,	1,	1,	0,	1,	0,	0,	1,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
} ;

static const unsigned char s_raw_controls [ ] = {
	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,
} ;

static const unsigned char * s_control = s_cooked_controls ;


/*
 * Memory-related error
 */
static short
mem_err ( void ) {
	if ( MemError ( ) ) {
		return MemError ( ) ;
	}
	return general_failure ;
}


/*
 * Make a rectangle empty
 */
static void
empty_rect ( Rect * r ) {

	r -> right = -20000 ;
	r -> left = 20000 ;
	r -> top = 20000 ;
	r -> bottom = -20000 ;
}


/*
 * Union twp rect together
 */
static void
union_rect ( Rect * r1 , Rect * r2 , Rect * dest ) {

	if ( r1 -> left < r2 -> left ) {
		dest -> left = r1 -> left ;
	} else {
		dest -> left = r2 -> left ;
	}

	if ( r1 -> top < r2 -> top ) {
		dest -> top = r1 -> top ;
	} else {
		dest -> top = r2 -> top ;
	}

	if ( r1 -> bottom > r2 -> bottom ) {
		dest -> bottom = r1 -> bottom ;
	} else {
		dest -> bottom = r2 -> bottom ;
	}

	if ( r1 -> right > r2 -> right ) {
		dest -> right = r1 -> right ;
	} else {
		dest -> right = r2 -> right ;
	}
}


/*
 * Dispose a pointer using the set memory-allocator
 */
static short
dispose_ptr ( tty_record * record , void * ptr ) {
pascal short ( * func ) ( WindowPtr window , void * ptr ) =
	( pascal short ( * ) ( WindowPtr , void * ) )
	record -> attribute [ TTY_FREE_MEMORY_FUNCTION ] ;

	if ( ! ptr ) {
		return noErr ; /* Silently accept disposing NULLs */
	}
	if ( func ) {
		s_err = ( * func ) ( record -> its_window , ptr ) ;
	} else {
		DisposePtr ( ptr ) ;
		s_err = MemError ( ) ;
	}
	return s_err ;
}


/*
 * Allocate a pointer using the set memory-allocator
 */
static short
alloc_ptr ( tty_record * record , void * * ptr , long size ) {
pascal short ( * func ) ( WindowPtr window , void * * ptr , long ) =
	( pascal short ( * ) ( WindowPtr , void * * , long ) )
	record -> attribute [ TTY_ALLOCATE_MEMORY_FUNCTION ] ;

	if ( func ) {
		s_err = ( * func ) ( record -> its_window , ptr , size ) ;
	} else {
		* ptr = NewPtr ( size ) ;
		s_err = * ptr ? noErr : mem_err ( ) ;
	}
	return s_err ;
}


/*
 * Set up a GWorld in the record
 */
static short
allocate_offscreen_world ( tty_record * record ) {
GWorldPtr gw = NULL ;
GWorldFlags world_flags = clipPix ;
long mem_here , mem_there , other , required_mem ;
Point p = { 0 , 0 } ;
Rect r_screen ;
GDHandle gdh ;

	select_onscreen_window ( record ) ;
	LocalToGlobal ( & p ) ;
	r_screen = record -> its_bits . bounds ;
	OffsetRect ( & r_screen , p . h , p . v ) ;

	gdh = GetMaxDevice ( & r_screen ) ;
	required_mem = ( long ) ( * ( ( * gdh ) -> gdPMap ) ) -> pixelSize *
		( ( long ) record -> its_bits . bounds . right *
		record -> its_bits . bounds . bottom ) >> 3 ;

	PurgeSpace ( & other , & mem_here ) ;
	if ( other < mem_here + MEMORY_MARGIN ) {
		mem_here = other - MEMORY_MARGIN ;
	}
	dprintf ( "Heap %ld Required %ld" , mem_here , required_mem ) ;
	if ( required_mem > mem_here ) {
		mem_there = required_mem ;
		if ( required_mem > MFMaxMem ( & mem_there ) ) {
			dprintf ( "No memory" ) ;
			return memFullErr ;
		}
		world_flags |= useTempMem ;
	}
	s_err = NewGWorld ( & gw , 16 , & r_screen , NULL , NULL , world_flags ) ;
	if ( ! s_err ) {
		record -> offscreen_world = gw ;
		select_offscreen_port ( record ) ;
		SetOrigin ( 0 , 0 ) ;
		dprintf ( "New GWorld @ %lx;dm %lx CGrafPtr" , gw , gw ) ;
	}
	return s_err ;
}


/*
 * Done with GWorld, release data
 */
static short
deallocate_gworld ( tty_record * record ) {
	if ( record -> offscreen_world ) {
		DisposeGWorld ( record -> offscreen_world ) ;
		record -> offscreen_world = NULL ;
	}
	return noErr ;
}


/*
 * Save the current port/world in a safe place for later retrieval
 */
static void
save_port ( tty_record * record , void * save ) {
GWorldPtr gw ;
GDHandle gh ;
GrafPtr gp ;

	if ( record -> uses_gworld ) {
		GetGWorld ( & gw , & gh ) ;
		* ( GWorldPtr * ) save = gw ;
	} else {
		GetPort ( & gp ) ;
		* ( GrafPtr * ) save = gp ;
	}
}


/*
 * Restore current port/world after a save
 */
static void
use_port ( tty_record * record , void * port ) {
	if ( record -> uses_gworld ) {
	PixMapHandle pix_map ;

		SetGWorld ( ( GWorldPtr ) port , NULL ) ;
		if ( port == record -> offscreen_world ) {
			pix_map = GetGWorldPixMap ( record -> offscreen_world ) ;
			if ( pix_map ) {
				LockPixels ( pix_map ) ;
			}
		} else {
			pix_map = GetGWorldPixMap ( record -> offscreen_world ) ;
			if ( pix_map ) {
				UnlockPixels ( pix_map ) ;
			}
		}
	} else {
		SetPort ( ( GrafPtr ) port ) ;
	}
}


/*
 * Use offscreen drawing - lock the pixels through use_port
 */
static void
select_offscreen_port ( tty_record * record ) {
	if ( record -> uses_gworld ) {
		use_port ( record , record -> offscreen_world ) ;
	} else {
		use_port ( record , record -> offscreen_port ) ;
	}
}


/*
 * Use the window - unlock pixels
 */
static void
select_onscreen_window ( tty_record * record ) {
	if ( record -> uses_gworld ) {
		use_port ( record , record -> its_window_world ) ;
		SetPort ( record -> its_window ) ;
	} else {
		use_port ( record , record -> its_window ) ;
	}
}


/*
 * Do bits copy depending on if we're using color or not
 */
static void
copy_bits ( tty_record * record , Rect * bounds , short xfer_mode , RgnHandle mask_rgn ) {
RGBColor old_fore , old_back ;
RGBColor rgb_black = { 0 , 0 , 0 } ;
RGBColor rgb_white = { 0xffff , 0xffff , 0xffff } ;

	if ( record -> uses_gworld ) {
	GWorldFlags pix_state = GetPixelsState ( GetGWorldPixMap ( record -> offscreen_world ) ) ;

		LockPixels ( GetGWorldPixMap ( record -> offscreen_world ) ) ;
		GetForeColor ( & old_fore ) ;
		GetBackColor ( & old_back ) ;
		RGBForeColor ( & rgb_black ) ;
		RGBBackColor ( & rgb_white ) ;
		CopyBits ( ( BitMap * ) & ( record -> offscreen_world -> portPixMap ) ,
			& ( record -> its_window -> portBits ) , bounds , bounds , xfer_mode ,
			mask_rgn ) ;
		RGBForeColor ( & old_fore ) ;
		RGBBackColor ( & old_back ) ;
		SetPixelsState ( GetGWorldPixMap ( record -> offscreen_world ) , pix_state ) ;
	} else {
		CopyBits ( & ( record -> its_bits ) , & ( record -> its_window -> portBits ) ,
			bounds , bounds , xfer_mode , mask_rgn ) ;
	}
}


/*
 * Fill an area with the background color
 */
static void
erase_rect ( tty_record * record , Rect * area ) {
#if defined(applec)
# pragma unused(record)
#endif

	EraseRect ( area ) ;
}


/*
 * Get rid of offscreen bitmap
 */
static short
free_bits ( tty_record * record ) {
	if ( record -> uses_gworld ) {
		s_err = deallocate_gworld ( record ) ;
	} else {
		if ( record -> offscreen_port ) {
			ClosePort ( record -> offscreen_port ) ;
			s_err = dispose_ptr ( record , record -> offscreen_port ) ;
			if ( ! s_err ) {
				record -> offscreen_port = NULL ;
			} else {
				return s_err ;
			}
		}
		s_err = dispose_ptr ( record , record -> its_bits . baseAddr ) ;
		if ( ! s_err ) {
			record -> its_bits . baseAddr = NULL ;
		}
	}
	return s_err ;
}


/*
 * Snatch a window from the resource fork. Create the record.
 * Otherwise, do nothing.
 */
pascal short
create_tty ( WindowPtr * window , short resource_id , Boolean in_color ) {
tty_record * record ;
Boolean was_allocated = !! * window ;

	if ( in_color ) {
		* window = GetNewCWindow ( resource_id , ( Ptr ) * window , ( WindowPtr ) -1L ) ;
	} else {
		* window = GetNewWindow ( resource_id , ( Ptr ) * window , ( WindowPtr ) -1L ) ;
	}
	if ( ! * window ) {
		return mem_err ( ) ;
	}

	record = ( tty_record * ) NewPtrClear ( sizeof ( tty_record ) ) ;
	if ( ! record ) {
		if ( was_allocated ) {
			CloseWindow ( * window ) ;
		} else {
			DisposeWindow ( * window ) ;
		}
		return mem_err ( ) ;
	}
	record -> its_window = * window ;
	SetWRefCon ( * window , ( long ) record ) ;
	record -> was_allocated = was_allocated ;
	record -> its_bits . baseAddr = NULL ;
#if TTY_INPUT
	record -> input_buffer = NULL ;
#endif

/*
 * Wee need to keep the window world around if we switch worlds
 */
	record -> offscreen_world = NULL ;
	record -> uses_gworld = in_color ;
	if ( in_color ) {
	GDHandle gh ;

		SetPort ( * window ) ;
		GetGWorld ( & ( record -> its_window_world ) , & gh ) ;
	} else {
		record -> its_window_world = NULL ;
	}

#if CLIP_RECT_ONLY
	empty_rect ( & ( record -> invalid_rect ) ) ;
#else
	record -> invalid_part = NewRgn ( ) ;
	if ( ! record -> invalid_part ) {
	short err = mem_err ( ) ;

		err = destroy_tty ( * window ) ;
		return err ;
	}
#endif

	return noErr ;
}


/*
 * Initialize the struct so it actually works as a tty.
 */
pascal short
init_tty_name ( WindowPtr window , unsigned char * font_name , short font_size ,
	short x_size , short y_size ) {
short font_num = 0 ;

	GetFNum ( font_name , & font_num ) ;
	if ( ! font_num ) {
		return general_failure ;
	}
	return init_tty_number ( window , font_num , font_size , x_size , y_size ) ;
}


pascal short
init_tty_number ( WindowPtr window , short font_number , short font_size ,
	short x_size , short y_size ) {
RECORD ( record ) ;

	record -> font_number = font_number ;
	record -> font_size = font_size ;
	record -> x_size = x_size ;
	record -> y_size = y_size ;

	record -> offscreen_port = NULL ;
	record -> attribute [ TTY_ATTRIB_BACKGROUND ] = 0xffffff ; /* White */

#if TTY_INPUT
	record -> input_buffer_len = 0 ;
	record -> input_buffer_limit = IB_LIMIT ;
	s_err = alloc_ptr ( record , & ( record -> input_buffer ) , IB_LIMIT ) ;
	if ( s_err ) {
		return s_err ;
	}
#endif

	return force_tty_coordinate_system_recalc ( window ) ;
}


/*
 * Done with a window - destroy it. Release the memory only if
 * it wasn't allocated when we got it!
 */
pascal short
destroy_tty ( WindowPtr window ) {
Boolean close_flag ;
RECORD ( record ) ;

	s_err = free_bits ( record ) ;
#if TTY_INPUT
	if ( ! s_err ) {
		s_err = dispose_ptr ( record -> input_buffer ) ;
		if ( ! s_err ) {
			record -> input_buffer = NULL ;
		}
	}
#endif
	if ( ! s_err ) {
		close_flag = record -> was_allocated ;
		DisposePtr ( ( Ptr ) record ) ;
		s_err = MemError ( ) ;
		if ( close_flag ) {
			CloseWindow ( window ) ;
		} else {
			DisposeWindow ( window ) ;
		}
	}
	
	return s_err ;
}


/*
 * Use a new font for drawing.
 */
pascal short
set_tty_font_name ( WindowPtr window , unsigned char * font_name ) {
RECORD ( record ) ;

	record -> font_number = 0 ;
	GetFNum ( font_name , & ( record -> font_number ) ) ;

	return ! record -> font_number ;
}


pascal short
set_tty_font_number ( WindowPtr window , short font_number ) {
RECORD ( record ) ;

	record -> font_number = font_number ;

	return noErr ;
}


pascal short
set_tty_font_size ( WindowPtr window , short font_size ) {
RECORD ( record ) ;

	record -> font_size = font_size ;

	return noErr ;
}


static void
do_set_port_font ( tty_record * record ) {

	PenNormal ( ) ;
	TextFont ( record -> font_number ) ;
	TextFace ( 0 ) ;
	TextSize ( record -> font_size ) ;
	if ( 0L != ( record -> attribute [ TTY_ATTRIB_FLAGS ] & TA_OVERSTRIKE ) ) {
		TextMode ( srcOr ) ;
	} else {
		TextMode ( srcCopy ) ;
	}
}


/*
 * Fill in some fields from some other fields that may have changed
 */
static void
calc_font_sizes ( tty_record * record ) {
FontInfo font_info ;

	do_set_port_font ( record ) ;

	GetFontInfo ( & font_info ) ;
	record -> char_width = font_info . widMax ;
	record -> ascent_height = font_info . ascent + font_info . leading ;
	record -> row_height = record -> ascent_height + font_info . descent ;
}


/*
 * Allocate memory for the bitmap holding the tty window
 */
static short
alloc_bits ( tty_record * record ) {
void * old_port ;

	save_port ( record , & old_port ) ;
	SetRect ( & record -> its_bits . bounds , 0 , 0 ,
		record -> char_width * record -> x_size ,
		record -> row_height * record -> y_size ) ;

/*
 * Clear two highest and lowest bit - not a color pixMap, and even in size
 */
	record -> its_bits . rowBytes = ( ( record -> its_bits . bounds . right + 15 )
		>> 3 ) & 0x1ffe ;

	if ( record -> uses_gworld ) {
		s_err = allocate_offscreen_world ( record ) ;
	} else {
		s_err = alloc_ptr ( record , ( void * * ) & ( record -> its_bits . baseAddr ) ,
			record -> its_bits . rowBytes * record -> its_bits . bounds . bottom ) ;
		if ( ! s_err ) {
			s_err = alloc_ptr ( record , ( void * * ) & ( record -> offscreen_port ) ,
				sizeof ( GrafPort ) ) ;
		}
		if ( ! s_err ) {
			OpenPort ( record -> offscreen_port ) ;
			SetPort ( record -> offscreen_port ) ;
			ClipRect ( & ( record -> its_bits . bounds ) ) ;
			SetPortBits ( & ( record -> its_bits ) ) ;
		}
	}
	use_port ( record , old_port ) ;

	return s_err ;
}


static void
update_offscreen_info ( tty_record * record ) {

	select_offscreen_port ( record ) ;
	do_set_port_font ( record ) ;
}


/*
 * Recalculate the window based on new size, font, extent values,
 * and re-allocate the bitmap.
 */
pascal short
force_tty_coordinate_system_recalc ( WindowPtr window ) {
RECORD ( record ) ;

	if ( s_err = free_bits ( record ) ) {
		return s_err ;
	}
	select_onscreen_window ( record ) ;
	calc_font_sizes ( record ) ;

	if ( s_err = alloc_bits ( record ) ) {
/*
 * Catastrophe! We could not allocate memory for the bitmap! Things may go very
 * much downhill from here!
 */
 		dprintf ( "alloc_bits returned NULL in force_tty_coordinate_system_recalc!" ) ;
		return s_err ;
	}

	update_offscreen_info ( record ) ;
	return clear_tty ( window ) ;
}


/*
 * Update TTY according to new color environment for the window
 */
pascal short
tty_environment_changed ( WindowPtr window ) {
Point p = { 0 , 0 } ;
Rect r_screen ;
RECORD ( record ) ;

	if ( record -> uses_gworld ) {
		select_onscreen_window ( record ) ;
		r_screen = record -> its_bits . bounds ;
		LocalToGlobal ( & p )  ;
		OffsetRect ( & r_screen , p . h , p . v ) ;
		UpdateGWorld ( & ( record -> offscreen_world ) , 0 , & r_screen ,
			NULL , NULL , stretchPix ) ;
		select_offscreen_port ( record ) ;
		SetOrigin ( 0 , 0 ) ;
	}
	return s_err ;
}


/*
 * Read a lot of interesting and useful information from the current tty
 */
pascal short
get_tty_metrics ( WindowPtr window , short * x_size , short * y_size ,
	short * x_size_pixels , short * y_size_pixels , short * font_number ,
	short * font_size , short * char_width , short * row_height ) {
RECORD ( record ) ;

/*
 * First, test that we actually have something to draw to...
 */
	if ( ( ( NULL == record -> its_bits . baseAddr ) && ! record -> uses_gworld ) ||
		( ( NULL == record -> offscreen_world ) && record -> uses_gworld ) ) {
		return general_failure ;
	}

	* x_size = record -> x_size ;
	* y_size = record -> y_size ;
	* x_size_pixels = record -> its_bits . bounds . right ;
	* y_size_pixels = record -> its_bits . bounds . bottom ;
	* font_number = record -> font_number ;
	* font_size = record -> font_size ;
	* char_width = record -> char_width ;
	* row_height = record -> row_height ;

	return noErr ;
}


/*
 * Map a position on the map to screen coordinates
 */
static void
pos_rect ( tty_record * record , Rect * r , short x_pos , short y_pos ,
	short x_end , short y_end ) {
 
	SetRect ( r , x_pos * ( record -> char_width ) , y_pos * ( record -> row_height ) ,
		( 1 + x_end ) * ( record -> char_width ) , ( 1 + y_end ) *
		( record -> row_height ) ) ;
}


static void
accumulate_rect ( tty_record * record , Rect * rect ) {
#if CLIP_RECT_ONLY
	union_rect ( rect , & ( record -> invalid_rect ) , & ( record -> invalid_rect ) ) ;
#else
RgnHandle rh = NewRgn ( ) ;

	RectRgn ( rh , rect ) ;
	UnionRgn ( record -> invalid_part , rh , record -> invalid_part ) ;
	DisposeRgn ( rh ) ;
#endif
}


/*
 * Invert the specified position
 */
static void
curs_pos ( tty_record * record , short x_pos , short y_pos , short to_state ) {
Rect r ;

	if ( record -> curs_state == to_state ) {
		return ;
	}
	record -> curs_state = to_state ;
	pos_rect ( record , & r , x_pos , y_pos , x_pos , y_pos ) ;

	if ( DRAW_DIRECT ) {
	void * old_port ;

		save_port ( record , & old_port ) ;
		select_onscreen_window ( record ) ;
		InvertRect ( & r ) ;
		use_port ( record , old_port ) ;
	} else {
		accumulate_rect ( record , & r ) ;
	}
}


/*
 * Move the cursor (both as displayed and where drawing goes)
 * HOWEVER: The cursor is NOT stored in the bitmap!
 */
pascal short
move_tty_cursor ( WindowPtr window , short x_pos , short y_pos ) {
RECORD ( record ) ;

	select_onscreen_window ( record ) ;
	if ( record -> x_curs == x_pos && record -> y_curs == y_pos ) {
		return noErr ;
	}
	if ( record -> x_size <= x_pos || x_pos < 0 ||
		record -> y_size <= y_pos || y_pos < 0 ) {
		return general_failure ;
	}
	curs_pos ( record , record -> x_curs , record -> y_curs , 0 ) ;
	record -> x_curs = x_pos ;
	record -> y_curs = y_pos ;
	curs_pos ( record , x_pos , y_pos , 1 ) ;

	return noErr ;
}


/*
 * Get the current cursor position. Note that the cursor may not be
 * displayed there yet; it depends on wether you've called update_tty()
 * or have the window in TA_ALWAYS_REFRESH mode.
 */
pascal short
get_tty_cursor ( WindowPtr window , short * x_pos , short * y_pos ) {
RECORD ( record ) ;

	* x_pos = record -> x_curs ;
	* y_pos = record -> y_curs ;

	return noErr ;
}


/*
 * Update the screen to match the current bitmap, after adding stuff
 * with add_tty_char etc.
 */
pascal short
update_tty ( WindowPtr window ) {
Rect r ;
RECORD ( record ) ;

#if CLIP_RECT_ONLY
	if ( record -> invalid_rect . right <= record -> invalid_rect . left ||
		record -> invalid_rect . bottom <= record -> invalid_rect . top ) {
		return noErr ;
	}
	r = record -> invalid_rect ;
#else
	if ( EmptyRgn ( record -> invalid_part ) ) {
		return noErr ;
	}
	r = ( * ( record -> invalid_part ) ) -> rgnBBox ;
#endif
	select_onscreen_window ( record ) ;
#if CLIP_RECT_ONLY
	copy_bits ( record , & r , srcCopy , NULL ) ;
	empty_rect ( & ( record -> invalid_rect ) ) ;
#else
	copy_bits ( record , & r , srcCopy , NULL ) ;
	SetEmptyRgn ( record -> invalid_part ) ;
#endif
	if ( record -> curs_state ) {

		pos_rect ( record , & r , record -> x_curs , record -> y_curs ,
			record -> x_curs , record -> y_curs ) ;
		InvertRect ( & r ) ;
	}

	return noErr ;
}


/*
 * Add a single character. It is drawn directly if the correct flag is set,
 * else deferred to the next update event or call of update_tty()
 */
pascal short
add_tty_char ( WindowPtr window , short character ) {
char s [ 2 ] ;

	s [ 0 ] = character ;
	s [ 1 ] = 0 ;
	return add_tty_string ( window , s ) ;
}


/*
 * Low level add to screen
 */
static void
do_add_string ( tty_record * record , char * str , short len ) {
Rect r ;
register int x_pos , count = len ;

	if ( len < 1 ) {
		return ;
	}
	select_offscreen_port ( record ) ;

	if ( 0L != ( record -> attribute [ TTY_ATTRIB_FLAGS ] & TA_MOVE_EACH_CHAR ) ) {
		x_pos = record -> x_curs ;
		while ( count -- ) {
			MoveTo ( x_pos * record -> char_width , record -> y_curs *
				record -> row_height + record -> ascent_height ) ;
			DrawChar ( * ( str ++ ) ) ;
		}
	} else {
		MoveTo ( record -> x_curs * record -> char_width , record -> y_curs *
			record -> row_height + record -> ascent_height ) ;
		DrawText ( str , 0 , len ) ;
	}

	pos_rect ( record , & r , record -> x_curs , record -> y_curs ,
		record -> x_curs + len - 1 , record -> y_curs ) ;
	if ( DRAW_DIRECT ) {
		select_onscreen_window ( record ) ;
		copy_bits ( record , & r , srcCopy , NULL ) ;
	} else {
		accumulate_rect ( record , & r ) ;
	}
}


/*
 * Low-level cursor handling routine
 */
static void
do_add_cursor ( tty_record * record , short x_pos ) {

	record -> x_curs = x_pos ;
	if ( record -> x_curs >= record -> x_size ) {
		if ( 0L != ( record -> attribute [ TTY_ATTRIB_FLAGS ] & TA_WRAP_AROUND ) ) {
			record -> y_curs ++ ;
			record -> x_curs = 0 ;
			if ( record -> y_curs >= record -> y_size ) {
				if ( 0L != ( record -> attribute [ TTY_ATTRIB_FLAGS ] &
					TA_INHIBIT_VERT_SCROLL ) ) {
					record -> y_curs = record -> y_size ;
				} else {
					scroll_tty ( record -> its_window , 0 , 1 + record -> y_curs -
						record -> y_size ) ;
				}
			}
		} else {
			record -> x_curs = record -> x_size ;
		}
	}
}


/*
 * Beep
 */
static void
do_tty_beep ( tty_record * record ) {
	if ( record -> attribute [ TTY_BEEP_FUNCTION ] ) {
	pascal void ( * tty_beep ) ( WindowPtr ) = ( pascal void ( * ) ( WindowPtr ) )
		record -> attribute [ TTY_BEEP_FUNCTION ] ;
		( * tty_beep ) ( record -> its_window ) ;
	} else {
		SysBeep ( 20 ) ;
	}
}


/*
 * Do control character
 */
static void
do_control ( tty_record * record , short character ) {
static int recurse = 0 ;

/*
 * Check recursion because nl_add_cr and cr_add_nl may both be set and call each other
 */
	recurse ++ ;
	if ( recurse > 2 ) {
		return ;
	}
	switch ( character ) {
	case 10 :
		record -> y_curs ++ ;
		if ( record -> y_curs >= record -> y_size ) {
			scroll_tty ( record -> its_window , 0 , 1 + record -> y_curs -
				record -> y_size ) ;
		}
		if ( 0L != ( record -> attribute [ TTY_ATTRIB_CURSOR ] & TA_NL_ADD_CR ) ) {
			do_control ( record , 13 ) ;
		}
		break ;
	case 13 :
		record -> x_curs = 0 ;
		if ( 0L != ( record -> attribute [ TTY_ATTRIB_CURSOR ] & TA_CR_ADD_NL ) ) {
			do_control ( record , 10 ) ;
		}
		break ;
	case 7 :
		do_tty_beep ( record ) ;
		break ;
	case 8 :
		if ( record -> x_curs > 0 ) {
			record -> x_curs -- ;
		}
		break ;
	default :
		break ;
	}
	recurse -- ;
}


/*
 * Add a null-terminated string of characters
 */
pascal short
add_tty_string ( WindowPtr window , const char * string ) {
register const unsigned char * start_c ;
register const unsigned char * the_c ;
register short max_x , pos_x ;
RECORD ( record ) ;

	select_onscreen_window ( record ) ;
	curs_pos ( record , record -> x_curs , record -> y_curs , 0 ) ;

	the_c = ( const unsigned char * ) string ;
	max_x = record -> x_size ;
	while ( 1 ) {
		start_c = the_c ;
		pos_x = record -> x_curs ;
		if ( ( 0L == ( record -> attribute [ TTY_ATTRIB_FLAGS ] & TA_WRAP_AROUND ) ) &&
			pos_x >= max_x ) { /* Optimize away drawing across border without wrap */
			break ;
		}
		while ( pos_x < max_x && ! s_control [ * the_c ] ) {
			the_c ++ ;
			pos_x ++ ;
		}
		do_add_string ( record , ( char * ) start_c , the_c - start_c ) ;
		do_add_cursor ( record , pos_x ) ;
		if ( ! * the_c ) {
			break ;
		}
		if ( s_control [ * the_c ] ) {
			do_control ( record , * the_c ) ;
			the_c ++ ;
		}
	}
	select_onscreen_window ( record ) ;
	curs_pos ( record , record -> x_curs , record -> y_curs , 1 ) ;

	ShowWindow ( window ) ;

	return noErr ;
}


/*
 * Do a c-style printf - the result shouldn't be too long...
 */
short
printf_tty ( WindowPtr window , const char * fmt , ... ) {
static char buf [ 256 ] ;
va_list list ;

	va_start ( list , fmt ) ;
	vsprintf ( buf , fmt , list ) ;
	va_end ( list ) ;

	return add_tty_string ( window , buf ) ;
}


/*
 * Read or change attributes for the tty. Note that some attribs may
 * very well clear and reallocate the bitmap when changed, whereas
 * others (color, highlight, ...) are guaranteed not to.
 */
pascal short
get_tty_attrib ( WindowPtr window , tty_attrib attrib , long * value ) {
RECORD ( record ) ;

	if ( attrib < 0 || attrib >= TTY_NUMBER_ATTRIBUTES ) {
		return general_failure ;
	}
	* value = record -> attribute [ attrib ] ;

	return noErr ;
}


pascal short
set_tty_attrib ( WindowPtr window , tty_attrib attrib , long value ) {
long old_value ;
RGBColor rgb_color ;
RECORD ( record ) ;

	if ( attrib < 0 || attrib >= TTY_NUMBER_ATTRIBUTES ) {
		return general_failure ;
	}
	old_value = record -> attribute [ attrib ] ;
	if ( old_value == value ) {
		return noErr ;
	}
	record -> attribute [ attrib ] = value ;
	/*
	 * Presently, no attributes generate a new bitmap.
	 */
	switch ( attrib ) {
	case TTY_ATTRIB_CURSOR :
/*
 * Check if we should change tables
 */
		if ( 0L != ( value & TA_RAW_OUTPUT ) ) {
			s_control = s_raw_controls ;
		} else {
			s_control = s_cooked_controls ;
		}
		break ;
	case TTY_ATTRIB_FLAGS :
/*
 * Check if we should flush the output going from cached to draw-direct
 */
		if ( 0L != ( value & TA_ALWAYS_REFRESH ) ) {
			update_tty ( window ) ;
		}
		break ;
	case TTY_ATTRIB_FOREGROUND :
/*
 * Set foreground color
 */
 		TA_TO_RGB ( value , rgb_color ) ;
		select_offscreen_port ( record ) ;
		RGBForeColor ( & rgb_color ) ;
		select_onscreen_window ( record ) ;
		RGBForeColor ( & rgb_color ) ;
		break ;
	case TTY_ATTRIB_BACKGROUND :
/*
 * Set background color
 */
 		TA_TO_RGB ( value , rgb_color ) ;
		select_offscreen_port ( record ) ;
		RGBBackColor ( & rgb_color ) ;
		select_onscreen_window ( record ) ;
		RGBBackColor ( & rgb_color ) ;
		break ;
	default :
		break ;
	}
	return noErr ;
}


/*
 * Scroll the window. Positive is up/left. scroll_tty ( window , 0 , 1 ) is a line feed.
 * Scroll flushes the accumulated update area by calling update_tty().
 */
pascal short
scroll_tty ( WindowPtr window , short delta_x , short delta_y ) {
RgnHandle rgn ;
RECORD ( record ) ;

	select_onscreen_window ( record ) ;
	s_err = update_tty ( window ) ;

	rgn = NewRgn ( ) ;

	select_offscreen_port ( record ) ;
	ScrollRect ( & ( record -> its_bits . bounds ) , - delta_x * record -> char_width ,
		- delta_y * record -> row_height , rgn ) ;
	EraseRgn ( rgn ) ;
	SetEmptyRgn ( rgn ) ;

	select_onscreen_window ( record ) ;
	ScrollRect ( & ( record -> its_bits . bounds ) , - delta_x * record -> char_width ,
		- delta_y * record -> row_height , rgn ) ;
	EraseRgn ( rgn ) ;
	DisposeRgn ( rgn ) ;

	record -> y_curs -= delta_y ;
	record -> x_curs -= delta_x ;

	return noErr ;
}


/*
 * Clear the screen. Immediate.
 */
pascal short
clear_tty ( WindowPtr window ) {
RECORD ( record ) ;

	select_offscreen_port ( record ) ;
	erase_rect ( record , & ( record -> its_bits . bounds ) ) ;
	select_onscreen_window ( record ) ;
	curs_pos ( record , record -> x_curs , record -> y_curs , 0 ) ;
	erase_rect ( record , & ( record -> its_bits . bounds ) ) ;
#if CLIP_RECT_ONLY
	empty_rect ( & ( record -> invalid_rect ) ) ;
#else
	SetEmptyRgn ( record -> invalid_part ) ;
#endif
	curs_pos ( record , record -> x_curs , record -> y_curs , 1 ) ;

	return noErr ;
}


/*
 * Resize the area - clears and reallocates the bitmap.
 */
pascal short
resize_tty_area ( WindowPtr window , short x_size , short y_size ) {
RECORD ( record ) ;

	record -> x_size = x_size ;
	record -> y_size = y_size ;

	return force_tty_coordinate_system_recalc ( window ) ;
}


/*
 * Echo to the user if echo mode on
 */
static void
do_add_input_character ( tty_record * record , unsigned char character ) {
	if ( 0L != ( record -> attribute [ TTY_ATTRIB_CURSOR ] & TA_ECHO_INPUT ) ) {
		add_tty_char ( record -> its_window , character ) ;
	}
}


#if TTY_INPUT
/*
 * Add a key in the queue.
 */
static short
do_add_key ( tty_record * record , long message ) {
	if ( record -> input_buffer_len >= record -> input_buffer_limit ) {
		do_tty_beep ( record ) ;
		return general_failure ;
	} else {
/*
 * If input is cooked, we should fix up this here line to allow line editting...
 */
		record -> input_buffer [ record -> input_buffer_len ++ ] = message & 0xff ;
		do_add_input_character ( record , message & 0xff ) ;
		return noErr ;
	}
}
#endif


/*
 * Add a key in the queue.
 */
static short
do_cmd_key ( tty_record * record , EventRecord * event ) {
	pascal void ( * callback ) ( EventRecord * event , WindowPtr window ) =
		( pascal void ( * ) ( EventRecord * , WindowPtr ) )
		record -> attribute [ TTY_COMMAND_KEY_CALLBACK ] ;

	if ( callback ) {
		( * callback ) ( event , record -> its_window ) ;
		return noErr ;
	}
	return general_failure ;
}


/*
 * Handle a tty event:
 * Updates, pertaining to our window.
 * Key downs, entered into the queue (if we are frontmost)
 */
pascal short
handle_tty_event ( WindowPtr window , EventRecord * event ) {
RECORD ( record ) ;

	update_tty ( window ) ;

	if ( 0L != ( record -> attribute [ TTY_ATTRIB_CURSOR ] & TA_BLINKING_CURSOR ) ) {
		if ( event -> when > record -> last_cursor + GetCaretTime ( ) ) {
			curs_pos ( record , record -> x_curs , record -> y_curs ,
				! record -> curs_state ) ;
			record -> last_cursor = event -> when ;
		}
	}

	switch ( event -> what ) {
	case updateEvt :
		if ( event -> message == ( long ) window ) {
			BeginUpdate ( window ) ;
			erase_rect ( record , & ( record -> its_bits . bounds ) ) ;
			tty_environment_changed ( window ) ;
			s_err = image_tty ( window ) ;
			EndUpdate ( window ) ;
			return s_err ;
		}
		break ;
#if TTY_INPUT
	case keyDown :
	case autoKey :
		if ( FrontWindow ( ) == window ) {
			if ( event -> modifiers & cmdKey ) {
				return do_cmd_key ( record , event ) ;
			} else {
				return do_add_key ( record , event -> message ) ;
			}
		}
		break ;
#endif
	deafult :
		break ;
	}

	return general_failure ;
}


/*
 * Draw an image of the tty - used for update events and can be called
 * for screen dumps.
 */
pascal short
image_tty ( WindowPtr window ) {
Rect r ;
RECORD ( record ) ;

	select_onscreen_window ( record ) ;
	copy_bits ( record , & ( record -> its_bits . bounds ) , srcCopy , NULL ) ;
	if ( record -> curs_state ) {

		pos_rect ( record , & r , record -> x_curs , record -> y_curs ,
			record -> x_curs , record -> y_curs ) ;
		InvertRect ( & r ) ;
	}

	return noErr ;
}


#if TTY_INPUT

/*
 * Read a character depending on the input mode
 */
pascal short
getchar_tty ( WindowPtr window , short * character ) {
RECORD ( record ) ;

	if ( 0L != ( record -> attribute [ TTY_ATTRIB_CURSOR ] & TA_RAW_INPUT ) ) {
		while ( ! record -> input_buffer_len ) {
		EventRecord er ;

			WaitNextEvent ( -1 , & er , GetCaretTime ( ) , NULL ) ;
			if ( handle_tty_event ( window , & er ) ) {
				switch ( er . what ) {
				default :
					break ;
				}
			}
		}
		* character = 
	} else {

	}
}

#endif /* TTY_INPUT */

#if EXTENDED_SUPPORT
/*
 * Delete or insert operations used by many terminals can bottleneck through
 * here. Note that the order of executin for row/colum insertions is NOT
 * specified. Negative values for num_ mean delete, zero means no effect.
 */
pascal short
mangle_tty_rows_columns ( WindowPtr window , short from_row , short num_rows ,
	short from_column , short num_columns ) {
Rect r ;
RgnHandle rh = NewRgn ( ) ;
RECORD ( record ) ;

	update_tty ( window ) ; /* Always make sure screen is OK */
	curs_pos ( record , record -> x_curs , record -> y_curs , 0 ) ;

	if ( num_rows ) {
		pos_rect ( record , & r , 0 , from_row , record -> x_size - 1 ,
			record -> y_size - 1 ) ;
		select_offscreen_port ( record ) ;
		ScrollRect ( & r , 0 , num_rows * record -> row_height , rh ) ;
		EraseRgn ( rh ) ;
		SetEmptyRgn ( rh ) ;
		select_onscreen_window ( record ) ;
		ScrollRect ( & r , 0 , num_rows * record -> row_height , rh ) ;
		EraseRgn ( rh ) ;
		SetEmptyRgn ( rh ) ;
	}
	if ( num_columns ) {
		pos_rect ( record , & r , from_column , 0 , record -> x_size - 1 ,
			record -> y_size - 1 ) ;
		select_offscreen_port ( record ) ;
		ScrollRect ( & r , num_columns * record -> char_width , 0 , rh ) ;
		EraseRgn ( rh ) ;
		SetEmptyRgn ( rh ) ;
		select_onscreen_window ( record ) ;
		ScrollRect ( & r , num_columns * record -> char_width , 0 , rh ) ;
		EraseRgn ( rh ) ;
		SetEmptyRgn ( rh ) ;
	}
	DisposeRgn ( rh ) ;
	if ( record -> x_curs >= from_column ) {
		record -> x_curs += num_columns ;
	}
	if ( record -> y_curs >= from_row ) {
		record -> y_curs += num_rows ;
	}
	curs_pos ( record , record -> x_curs , record -> y_curs , 1 ) ;

	return noErr ;
}


/*
 * Clear an area
 */
pascal short
clear_tty_window ( WindowPtr window , short from_x , short from_y ,
	short to_x , short to_y ) {
Rect r ;
RECORD ( record ) ;

	if ( from_x > to_x || from_y > to_y ) {
		return general_failure ;
	}
	pos_rect ( record , & r , from_x , from_y , to_x , to_y ) ;
	select_offscreen_port ( record ) ;
	erase_rect ( record , & r ) ;
	accumulate_rect ( record , & r ) ;
	if ( DRAW_DIRECT ) {
		update_tty ( window ) ;
	}
}


/*
 * Frame an area in an aesthetically pleasing way.
 */
pascal short
frame_tty_window ( WindowPtr window , short from_x , short from_y ,
	short to_x , short to_y , short frame_fatness ) {
Rect r ;
RECORD ( record ) ;

	if ( from_x > to_x || from_y > to_y ) {
		return general_failure ;
	}
	pos_rect ( record , & r , from_x , from_y , to_x , to_y ) ;
	select_offscreen_port ( record ) ;
	PenSize ( frame_fatness , frame_fatness ) ;
	FrameRect ( & r ) ;
	PenNormal ( ) ;
	accumulate_rect ( record , & r ) ;
	if ( DRAW_DIRECT ) {
		update_tty ( window ) ;
	}
}


/*
 * Highlighting a specific part of the tty window
 */
pascal short
invert_tty_window ( WindowPtr window , short from_x , short from_y ,
	short to_x , short to_y ) {
Rect r ;
RECORD ( record ) ;

	if ( from_x > to_x || from_y > to_y ) {
		return general_failure ;
	}
	pos_rect ( record , & r , from_x , from_y , to_x , to_y ) ;
	select_offscreen_port ( record ) ;
	InvertRect ( & r ) ;
	accumulate_rect ( record , & r ) ;
	if ( DRAW_DIRECT ) {
		update_tty ( window ) ;
	}
}


static void
canonical_rect ( Rect * r , short x1 , short y1 , short x2 , short y2 ) {
	if ( x1 < x2 ) {
		if ( y1 < y2 ) {
			SetRect ( r , x1 , x2 , y1 , y2 ) ;
		} else {
			SetRect ( r , x1 , x2 , y2 , y1 ) ;
		}
	} else {
		if ( y1 < y2 ) {
			SetRect ( r , x2 , x1 , y1 , y2 ) ;
		} else {
			SetRect ( r , x2 , x1 , y2 , y1 ) ;
		}
	}
}


/*
 * Line drawing - very device dependent
 */
pascal short
draw_tty_line ( WindowPtr window , short from_x , short from_y ,
	short to_x , short to_y ) {
Rect r ;
RECORD ( record ) ;

	select_offscreen_port ( record ) ;
	MoveTo ( from_x , from_y ) ;
	LineTo ( to_x , to_y ) ;
	canonical_rect ( & r , from_x , from_y , to_x , to_y ) ;
	accumulate_rect ( record , & r ) ;
	if ( DRAW_DIRECT ) {
		update_tty ( window ) ;
	}
}


#endif /* EXTENDED_SUPPORT */
