/*	SCCS Id: @(#)mttymain.c	3.1	93/02/26			*/
/* Copyright (c) Jon W{tte, 1993					*/
/* NetHack may be freely redistributed.  See license for details.	*/

#include "hack.h"
#include "wintty.h"
#include "mactty.h"

#include <stdarg.h>
#include <Menus.h>

extern void InitRes ( void ) ;
extern Boolean small_screen ;
extern void DoMenu ( long ) ;
extern void UpdateMenus ( void ) ;
extern int GetFromKeyQueue ( void ) ;
extern void dprintf ( char * , ... ) ;


#define POWER_LIMIT 22
#define SECONDARY_POWER_LIMIT 16
#define CHANNEL_LIMIT 14
#define SECONDARY_CHANNEL_LIMIT 12

#define MT_WINDOW 135
#define MT_WIDTH 80
static short MT_HEIGHT = 24;


/*
 * Names:
 *
 * Statics are prefixed _
 * Mac-tty becomes mt_
 */


static void _mt_set_colors ( long * colors ) ;


static long _mt_attrs [ 5 ] [ 2 ] = {
	{ 0x202020 , 0xffffff } , /* Normal */
	{ 0xff8080 , 0xffffff } , /* Underline */
	{ 0x40c020 , 0xe0e0e0 } , /* Bold */
	{ 0x003030 , 0xff0060 } , /* Blink */
	{ 0xff8888 , 0x000000 } , /* Inverse */
} ;


static char _attrs_inverse [ 5 ] = {
	0 , 0 , 0 , 0 , 0 ,
} ;


#if 0
#define BLACK		0
#define RED		1
#define GREEN		2
#define BROWN		3	/* on IBM, low-intensity yellow is brown */
#define BLUE		4
#define MAGENTA 	5
#define CYAN		6
#define GRAY		7	/* low-intensity white */
#define NO_COLOR	8
#define ORANGE_COLORED	9	/* "orange" conflicts with the object */
#define BRIGHT_GREEN	10
#define YELLOW		11
#define BRIGHT_BLUE	12
#define BRIGHT_MAGENTA  13
#define BRIGHT_CYAN	14
#define WHITE		15
#define MAXCOLORS	16
#endif

static long _mt_colors [ 16 ] [ 2 ] = {
	{ 0x000000 , 0xaaaaaa } , /* Black */
	{ 0x880000 , 0xffffff } , /* Red */
	{ 0x008800 , 0xffffff } , /* Green */
	{ 0x553300 , 0xffffff } , /* Brown */
	{ 0x000088 , 0xffffff } , /* Blue */
	{ 0x770077 , 0xffffff } , /* Magenta */
	{ 0x007777 , 0xffffff } , /* Cyan */
	{ 0x888888 , 0xffffff } , /* Gray */
	{ 0x222222 , 0xffffff } , /* No Color */
	{ 0xeeee00 , 0x606060 } , /* Orange */
	{ 0x00ff00 , 0x606060 } , /* Bright Green */
	{ 0xeeee00 , 0x606060 } , /* Yellow */
	{ 0x0000ff , 0x606060 } , /* Bright Blue */
	{ 0xee00ee , 0x606060 } , /* Bright Magenta */
	{ 0x00eeee , 0x606060 } , /* Bright Cyan */
	{ 0x000000 , 0xffffff } , /* White */
} ;

static char _colors_inverse [ MAXCOLORS ] = {
	1 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 ,
} ;


#ifdef CHANGE_COLOR

void
tty_change_color ( int color , long rgb , int reverse ) {
long inverse = 0 , the_rgb = rgb ;
int total_power = 0 , max_channel = 0 ;
int cnt = 3 ;

	the_rgb >>= 4 ;
	while ( cnt -- > 0 ) {
		total_power += the_rgb & 0xf ;
		max_channel = max ( max_channel , the_rgb & 0xf ) ;
		the_rgb >>= 8 ;
	}

	if ( total_power >= POWER_LIMIT ||
		( total_power >= SECONDARY_POWER_LIMIT &&
			max_channel >= SECONDARY_CHANNEL_LIMIT ) ||
		max_channel >= CHANNEL_LIMIT ) {
		inverse = 0x000000 ;
	} else {
		inverse = 0xffffff ;
	}

	if ( reverse ) {
	long rev = rgb ;

		rgb = inverse ;
		inverse = rev ;
	}

	if ( color >= MAXCOLORS ) {
		if ( color - MAXCOLORS >= 5 ) {
			impossible ( "Changing too many colors" ) ;
			return ;
		}
		_mt_attrs [ color - MAXCOLORS ] [ 0 ] = rgb ;
		_mt_attrs [ color - MAXCOLORS ] [ 1 ] = inverse ;
		_attrs_inverse [ color - MAXCOLORS ] = reverse ;
	} else if ( color >= 0 ) {
		_mt_colors [ color ] [ 0 ] = rgb ;
		_mt_colors [ color ] [ 1 ] = inverse ;
		_colors_inverse [ color ] = reverse ;
	} else {
		impossible ( "Changing negative color" ) ;
	}
}


static char color_buf [ 5 * ( MAXCOLORS + 5 ) + 1 ] ;

char *
tty_get_color_string ( void ) {
char tmp [ 10 ] ;
int count ;

	color_buf [ 0 ] = 0 ;

	for ( count = 0 ; count < MAXCOLORS ; count ++ ) {
	int flag = _colors_inverse [ count ] ? 1 : 0 ;

		sprintf ( tmp , "%s%s%x%x%x" , count ? "/" : "" ,
			flag ? "-" : "" ,
			( _mt_colors [ count ] [ flag ] >> 20 ) & 0xf ,
			( _mt_colors [ count ] [ flag ] >> 12 ) & 0xf ,
			( _mt_colors [ count ] [ flag ] >> 4 ) & 0xf ) ;
		strcat ( color_buf , tmp ) ;
	}
	for ( count = 0 ; count < 5 ; count ++ ) {
	int flag = _colors_inverse [ count ] ? 1 : 0 ;

		sprintf ( tmp , "/%s%x%x%x" ,
			flag ? "-" : "" ,
			( _mt_attrs [ count ] [ flag ] >> 20 ) & 0xf ,
			( _mt_attrs [ count ] [ flag ] >> 12 ) & 0xf ,
			( _mt_attrs [ count ] [ flag ] >> 4 ) & 0xf ) ;
		strcat ( color_buf , tmp ) ;
	}

	return color_buf ;
}
#endif


extern struct DisplayDesc *ttyDisplay;	/* the tty display descriptor */

char kill_char = 27 ;
char erase_char = 8 ;

WindowPtr _mt_window = NULL ;
static Boolean _mt_in_color = 0 ;


static void
_mt_init_stuff ( void )
{
long resp , flag ;
short num_cols , num_rows , win_width , win_height , font_num , font_size ;
short char_width , row_height ;
short hor , vert ;

#if 1
	if ( windowprocs == mac_procs ) {
		dprintf ( "Mac Windows" ) ;
		MT_HEIGHT -= 1 ;
	} else {
		dprintf ( "TTY Windows" ) ;
	}
#else
	if (!strcmp(windowprocs.name, "mac")) {
		MT_HEIGHT -= 1;	/* the message box is in a separate window */
	}
#endif

	LI = MT_HEIGHT ;
	CO = MT_WIDTH ;

	/*
	 * If there is at least one screen CAPABLE of color, and if
	 * 32-bit QD is there, we use color. 32-bit QD is needed for the
	 * offscreen GWorld
	 */
	if ( ! Gestalt ( gestaltQuickdrawVersion , & resp ) && resp > 0x1ff ) {
	GDHandle gdh ;

		gdh = GetDeviceList ( ) ;
		while ( gdh ) {
			if ( TestDeviceAttribute ( gdh , screenDevice ) ) {
				if ( HasDepth ( gdh , 8 , 1 , 1 ) ||
					HasDepth ( gdh , 12 , 1 , 1 ) || /* Intuition tells me this may happen
														on color LCDs */
					HasDepth ( gdh , 16 , 1 , 1 ) ||
					HasDepth ( gdh , 4 , 1 , 1 ) || /* why here!? */
					HasDepth ( gdh , 32 , 1 , 1 ) ) {

					_mt_in_color = 1 ;
					break ;
				}
			}
			gdh = GetNextDevice ( gdh ) ;
		}
	}

	mustwork ( create_tty ( & _mt_window , MT_WINDOW , _mt_in_color ) ) ;
	( ( WindowPeek ) _mt_window ) -> windowKind = ( WIN_BASE_KIND + NHW_MAP ) ;
	SelectWindow ( _mt_window ) ;
	SetPort ( _mt_window ) ;
	SetOrigin ( -3 , -3 ) ;
	font_num = 0 ;
	font_size = ( flags.large_font && ! small_screen ) ? 12 : 9 ;
	GetFNum ( "\PHackFont" , & font_num ) ;
	if ( font_num != 0 ) {
		mustwork ( init_tty_number ( _mt_window , font_num , font_size ,
			MT_WIDTH , MT_HEIGHT + 1 ) ) ;
	} else {
		mustwork ( init_tty_name ( _mt_window , "\PMonaco" , font_size ,
			MT_WIDTH , MT_HEIGHT + 1 ) ) ;
	}

	mustwork ( get_tty_metrics ( _mt_window , & num_cols , & num_rows , & win_width ,
		& win_height , & font_num , & font_size , & char_width , & row_height ) ) ;

	SizeWindow ( _mt_window , win_width + 6 , win_height + 6 , 1 ) ;
	dprintf ( "Checking for TTY window position" ) ;
	if ( RetrievePosition ( kMapWindow , & vert , & hor ) ) {
		dprintf ( "\PMoving window to (%d,%d)" , hor , vert ) ;
		MoveWindow ( _mt_window , hor , vert , 1 ) ;
	}
	ShowWindow ( _mt_window ) ;
	SetPort ( _mt_window ) ;

	mustwork ( get_tty_attrib ( _mt_window , TTY_ATTRIB_FLAGS , & flag ) ) ;
/* Start in raw, always flushing mode */
	flag |= TA_ALWAYS_REFRESH ;
	mustwork ( set_tty_attrib ( _mt_window , TTY_ATTRIB_FLAGS , flag ) ) ;

	mustwork ( get_tty_attrib ( _mt_window , TTY_ATTRIB_CURSOR , & flag ) ) ;
	flag |= TA_BLINKING_CURSOR | TA_WRAP_AROUND ;
#ifdef applec
	flag &= ~ TA_CR_ADD_NL ;
#else
	flag |= ~ TA_NL_ADD_CR ;
#endif
	mustwork ( set_tty_attrib ( _mt_window , TTY_ATTRIB_CURSOR , flag ) ) ;

	InitRes ( ) ;
}


static void
_mt_handle_event ( EventRecord * event ) {
Rect r ;
int code ;
WindowPtr window ;

	if ( event -> what == mouseDown ) {
		r = ( * GetGrayRgn ( ) ) -> rgnBBox ;
		InsetRect ( & r , 3 , 3 ) ;

		code = FindWindow ( event -> where , & window ) ;
		switch ( code ) {
			case inDrag :
				DragWindow ( window , event -> where , & r ) ;
				if ( window == _mt_window ) {
					SaveWindowPos ( window ) ;
				}
				break ;
			case inSysWindow :
				SystemClick ( event , window ) ;
				break ;
			case inMenuBar :
				UpdateMenus ( ) ;
				DoMenu ( MenuSelect ( event -> where ) ) ;
				break ;
			default :
				/* Do nothing */
				;
		}
	} else if ( event -> what == diskEvt ) {
		if ( event -> message & 0xffff0000 != 0L ) {
		Point p = { 100 , 100 } ;

			( void ) itworked ( DIBadMount ( p , event -> message ) ) ;
		}
	}
}


int
tgetch ( void ) {
EventRecord event ;
long sleepTime = 0 ;
int ret ;

	while ( 1 ) {
		update_tty ( _mt_window ) ;
		ret = GetFromKeyQueue ( ) ;
		if ( ret ) {
			return ret ;
		}
		WaitNextEvent ( -1 , & event , sleepTime , 0 ) ;
		SetPort ( _mt_window ) ;
		if ( handle_tty_event ( _mt_window , & event ) ) {
			_mt_handle_event ( & event ) ;
		}
		if ( event . what == keyDown || event . what == autoKey ) {
			if ( ! ( event . modifiers & cmdKey ) ) {
				return ( event . message & 0xff ) ;
			} else {
				DoMenu ( MenuKey ( event . message & 0xff ) ) ;
			}
		} else if ( ! sleepTime ) {
			Point p = event . where ;
			GlobalToLocal ( & p ) ;
			if ( PtInRect ( p , & ( _mt_window -> portRect ) ) ) {
				ObscureCursor ( ) ;
			} else {
				InitCursor ( ) ;
			}
		}
		if ( event . what == nullEvent ) {
			sleepTime = GetCaretTime ( ) ;
		} else {
			sleepTime = 0 ;
		}
	}
}


void
getreturn ( char * str ) {
	FlushEvents ( -1 , 0 ) ;
	printf_tty ( _mt_window , "Press space %s" , str ) ;
	( void ) tgetch ( ) ;
}


int
has_color ( int color ) {
#if defined(applec)
# pragma unused(color)
#endif
Rect r;
Point p = { 0 , 0 } ;
GDHandle gh ;

	if ( ! _mt_in_color ) {
		return 0 ;
	}

	r = _mt_window -> portRect ;
	SetPort ( _mt_window ) ;
	GlobalToLocal ( & p ) ;
	OffsetRect ( & r , p . h , p . v ) ;

	gh = GetMaxDevice ( & r ) ;
	if ( ! gh ) {
		return 0 ;
	}

	return ( * ( ( * gh ) -> gdPMap ) ) -> pixelSize > 4 ; /* > 4 bpp */
}


void
tty_delay_output ( void ) {
EventRecord event ;
long toWhen = TickCount ( ) + 3 ;

	while ( TickCount ( ) < toWhen ) {
		WaitNextEvent ( updateMask , & event , 3L , 0 ) ;
		if ( event . what == updateEvt ) {
			if ( ! handle_tty_event ( _mt_window , & event ) ) {
				_mt_handle_event ( & event ) ;
			}
		}
	}
}


void
tty_nhbell ( void ) {
	SysBeep ( 20 ) ;
}


void
cmov ( int x , int y ) {
	move_tty_cursor ( _mt_window , x , y ) ;
	ttyDisplay -> cury = y ;
	ttyDisplay -> curx = x ;
}


void
nocmov ( int x , int y ) {
	cmov ( x , y ) ;
}


static void
_mt_set_colors ( long * colors ) {
short err ;

	if ( ! _mt_in_color ) {
		return ;
	}
	err = set_tty_attrib ( _mt_window , TTY_ATTRIB_FOREGROUND , colors [ 0 ] ) ;
	err = set_tty_attrib ( _mt_window , TTY_ATTRIB_BACKGROUND , colors [ 1 ] ) ;
}


void
term_end_attr ( int attr ) {
#if defined(applec)
# pragma unused ( attr )
#endif
	_mt_set_colors ( _mt_attrs [ 0 ] ) ;
}


void
term_start_attr ( int attr ) {
	switch ( attr ) {
		case ATR_ULINE:
			_mt_set_colors ( _mt_attrs [ 1 ] ) ;
			break ;
		case ATR_BOLD:
			_mt_set_colors ( _mt_attrs [ 2 ] ) ;
			break ;
		case ATR_BLINK:
			_mt_set_colors ( _mt_attrs [ 3 ] ) ;
			break ;
		case ATR_INVERSE:
			_mt_set_colors ( _mt_attrs [ 4 ] ) ;
			break ;
		default:
			_mt_set_colors ( _mt_attrs [ 0 ] ) ;
			break ;
	}
}


void
standoutend ( void ) {
	term_end_attr ( ATR_INVERSE ) ;
}


void
standoutbeg ( void ) {
	term_start_attr ( ATR_INVERSE ) ;
}


void
xputs ( char * str ) {
short err ;

	err = add_tty_string ( _mt_window , str ) ;
}


void
term_end_color ( void ) {
	_mt_set_colors ( _mt_colors [ NO_COLOR ] ) ;
}


void
cl_end ( void ) {
short err ;

	_mt_set_colors ( _mt_attrs [ 0 ] ) ;
	err = clear_tty_window ( _mt_window , ttyDisplay -> curx , ttyDisplay -> cury ,
		MT_WIDTH - 1 , ttyDisplay -> cury ) ;
}


void
clear_screen ( void ) {
short err ;

	_mt_set_colors ( _mt_attrs [ 0 ] ) ;
	err = clear_tty ( _mt_window ) ;
}


void
cl_eos ( void ) {
short err ;

	cl_end ( ) ;
	_mt_set_colors ( _mt_attrs [ 0 ] ) ;
	err = clear_tty_window ( _mt_window , 0 , ttyDisplay -> cury + 1 , MT_WIDTH - 1 ,
		MT_HEIGHT - 1 ) ;
}


void
home ( void ) {
short err ;

	err = move_tty_cursor ( _mt_window , 0 , 0 ) ;
	ttyDisplay -> curx = 0 ;
	ttyDisplay -> cury = 0 ;
}


void
backsp ( void ) {
short err ;

	err = add_tty_char ( _mt_window , 8 ) ;
	err = add_tty_char ( _mt_window , 32 ) ;
	err = add_tty_char ( _mt_window , 8 ) ;
	err = update_tty ( _mt_window ) ;
}


void
msmsg ( const char * str , ... ) {
va_list args ;
char * buf = alloc ( 1000 ) ;

	va_start ( args , str ) ;
	vsprintf ( buf , str , args ) ;
	va_end ( args ) ;

	xputs ( buf ) ;
	free ( buf ) ;
}


void
term_end_raw_bold ( void ) {
	standoutend ( ) ;
}


void
term_start_raw_bold ( void ) {
	standoutbeg ( ) ;
}


void
term_start_color ( int color ) {
	if ( color >= 0 && color < MAXCOLORS ) {
		_mt_set_colors ( _mt_colors [ color ] ) ;
	}
}


void
setftty ( void ) {
long flag ;

	mustwork ( get_tty_attrib ( _mt_window , TTY_ATTRIB_FLAGS , & flag ) ) ;
/* Buffered output in the game */
	flag &= ~ TA_ALWAYS_REFRESH ;
	flag |= TA_INHIBIT_VERT_SCROLL ; /* don't scroll */
	mustwork ( set_tty_attrib ( _mt_window , TTY_ATTRIB_FLAGS , flag ) ) ;

	mustwork ( get_tty_attrib ( _mt_window , TTY_ATTRIB_CURSOR , & flag ) ) ;
#ifdef applec
	flag &= ~ TA_CR_ADD_NL ;
#else
	flag |= ~ TA_NL_ADD_CR ;
#endif
	mustwork ( set_tty_attrib ( _mt_window , TTY_ATTRIB_CURSOR , flag ) ) ;

	flags . cbreak = 1 ;
}


void
tty_startup ( int * width , int * height  ) {
	_mt_init_stuff ( ) ;
	* width = MT_WIDTH ;
	* height = MT_HEIGHT ;
}


void
gettty ( void ) {
}


void
settty ( char * str ) {
long flag ;

	update_tty ( _mt_window ) ;

	mustwork ( get_tty_attrib ( _mt_window , TTY_ATTRIB_FLAGS , & flag ) ) ;
/* Buffered output in the game, raw in "raw" mode */
	flag &= ~ TA_INHIBIT_VERT_SCROLL ; /* scroll */
	flag |= TA_ALWAYS_REFRESH ;
	mustwork ( set_tty_attrib ( _mt_window , TTY_ATTRIB_FLAGS , flag ) ) ;

	mustwork ( get_tty_attrib ( _mt_window , TTY_ATTRIB_CURSOR , & flag ) ) ;
#ifdef applec
	flag |= TA_CR_ADD_NL ;
#else
	flag |= TA_NL_ADD_CR ;
#endif
	mustwork ( set_tty_attrib ( _mt_window , TTY_ATTRIB_CURSOR , flag ) ) ;

	tty_raw_print ( "\n" ) ;
	if ( str ) {
		tty_raw_print ( str ) ;
	}
}


void
tty_number_pad ( int arg ) {
#if defined(applec)
# pragma unused(arg)
#endif
}


void
tty_start_screen ( void ) {
	flags . cbreak = 1 ;
}


void
tty_end_screen ( void ) {
}


int
term_puts ( char * str ) {
	xputs ( str ) ;
	return strlen ( str ) ;
}


int
term_putc ( int c ) {
short err ;

	err = add_tty_char ( _mt_window , c ) ;
	return err ? EOF : c ;
}


int
term_flush ( void * desc ) {
	if ( desc == stdout || desc == stderr ) {
		update_tty ( _mt_window ) ;
	} else {
		impossible ( "Substituted flush for file" ) ;
		return fflush ( desc ) ;
	}
	return 0 ;
}
