/*	SCCS Id: @(#)macconf.h	3.1	91/07/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef MAC
# ifndef MACCONF_H
#  define MACCONF_H

/*
 * Select your compiler...
 * This could probably be automagic later on - I believe there's
 * a unique symbol for MPW at least (it's "macintosh")
 */
#  define MAC_MPW32		/* Headers, and for avoiding a bug */
/* #  define MAC_THINKC5	/* Maybe supported now ... */

#  define RANDOM
#  define NO_SIGNAL	/* You wouldn't believe our signals ... */
#  define FILENAME 256

#  include "system.h"

typedef long off_t ;

/*
 * Try and keep the number of files here to an ABSOLUTE minimum !
 * include the relevant files in the relevant .c files instead !
 */
#  include <segload.h>
#  include <stdlib.h>
#  include <windows.h>
#  include <memory.h>
#  include <quickdraw.h>
#  include <events.h>
#  include <controls.h>
#  include <dialogs.h>
#  include <fonts.h>

/*
 * We could use the PSN under sys 7 here ...
 */
#  define getpid() 1
#  define getuid() 1
#  define index strchr
#  define rindex strrchr

#  define Rand random

#  define error progerror

# if defined(VISION_TABLES) && defined(BRACES)
#  undef BRACES
# endif

/*
 * macfile.c
 * MAC file I/O routines
 */

extern int maccreat ( const char * name , long fileType ) ;
extern int macopen ( const char * name , int flags , long fileType ) ;
extern int macclose ( int fd ) ;
extern int macread ( int fd , void * ptr , unsigned ) ;
extern int macwrite ( int fd , void * ptr , unsigned ) ;
extern long macseek ( int fd , long pos , short whence ) ;


# if !defined(O_WRONLY)
#  include <fcntl.h>
# endif

#if !defined(SPEC_LEV) && !defined(DGN_COMP)
# define creat maccreat
# define open macopen
# define close macclose
# define read macread
# define write macwrite
# define lseek macseek
#endif

# define TEXT_TYPE 'TEXT'
# define LEVL_TYPE 'LEVL'
# define BONE_TYPE 'BONE'
# define SAVE_TYPE 'SAVE'
# define PREF_TYPE 'PREF'
# define DATA_TYPE 'DATA'
# define MAC_CREATOR 'nh31' /* Registered with DTS ! */

typedef struct macdirs {
	Str32		dataName ;
	short		dataRefNum ;
	long		dataDirID ;

	Str32		saveName ;
	short		saveRefNum ;
	long		saveDirID ;

	Str32		levelName ;
	short		levelRefNum ;
	long		levelDirID ;
} MacDirs ;

typedef struct macflags {
	Bitfield ( processes , 1 ) ;
	Bitfield ( color , 1 ) ;
	Bitfield ( folders , 1 ) ;
	Bitfield ( tempMem , 1 ) ;
	Bitfield ( help , 1 ) ;
	Bitfield ( fsSpec , 1 ) ;
	Bitfield ( trueType , 1 ) ;
	Bitfield ( aux , 1 ) ;
	Bitfield ( alias , 1 ) ;
	Bitfield ( standardFile , 1 ) ;
} MacFlags ;

extern MacDirs theDirs ;
extern MacFlags macFlags ;

/*
 * Mac windows
 */
#define NUM_MACWINDOWS 15
#define TEXT_BLOCK 512L
#define WIN_BASE_RES 128
#define WIN_BASE_KIND 128
#define NUM_MENU_ITEMS 60 /* We've run out of letters by then ... */
#define CHAR_ENTER ((char)3)
#define CHAR_BS ((char)8)
#define CHAR_LF ((char)10)
#define CHAR_CR ((char)13)
#define CHAR_ESC ((char)27)
#define CHAR_BLANK ((char)32)
#define CHAR_DELETE ((char)127)

#define MAC_GRAPHICS_ENV

/* Window constants */
#define kMapWindow 0
#define kStatusWindow 1
#define kMessageWindow 2
#define kTextWindow 3
#define kMenuWindow 4
#define kLastWindowKind kMenuWindow

extern Boolean RetrievePosition ( short , short * , short * ) ;
extern Boolean RetrieveSize ( short , short , short , short * , short * ) ;
extern void SavePosition ( short , short , short ) ;
extern void SaveSize ( short , short , short ) ;
extern void SaveWindowPos ( WindowPtr ) ;
extern void SaveWindowSize ( WindowPtr ) ;

/*
 * This determines the minimum logical line length in text windows
 * That is; even if physical width is less, this is where line breaks
 * go at the minimum. 350 is about right for score lines with a
 * geneva 10 pt font.
 */
#define MIN_RIGHT 350

#define NUM_CANCEL_ITEMS 10

typedef struct NhWindow {
	WindowPtr		theWindow ;
	short			kind ;
	void			( * keyFunc ) ( EventRecord * , WindowPtr ) ;
	void			( * clickFunc ) ( EventRecord * , WindowPtr ) ;
	void			( * updateFunc ) ( EventRecord * , WindowPtr ) ;
	void			( * cursorFunc ) ( EventRecord * , WindowPtr , RgnHandle ) ;
	Handle			windowText ;
	long			windowTextLen ;
	long			textBase ;
	Point			cursor ;		/* In CHARS / LINES */
	short			leading ;
	short			charHeight ;
	short			charWidth ;
	short			fontNum ;
	short			fontSize ;
	short			lin ;			/* Used by menus */
	short			wid ;			/* Used by menus */
	char			itemChars [ NUM_MENU_ITEMS ] ;
	char			cancelStr [ NUM_CANCEL_ITEMS ] ;
	char			cancelChar ;
	char			clear ;
	char			cursorDrawn ;
	short			scrollPos ;
	ControlHandle	scrollBar ;
} NhWindow ;

#define NUM_STAT_ROWS 2
#define NUM_ROWS 22
#define NUM_COLS 81 /* We shouldn't use column 0 */

typedef struct MapData {
	char		map [ NUM_ROWS ] [ NUM_COLS ] ;
} MapData ;

typedef struct StatusData {
	char		map [ NUM_STAT_ROWS ] [ NUM_COLS ] ;
} StatusData ;

extern NhWindow * theWindows ;

extern struct window_procs mac_procs ;

extern short text_wind_font;
#define set_text_wind_font(fnt) (text_wind_font = fnt)
#define mono_font()	set_text_wind_font(monaco)
#define normal_font()	set_text_wind_font(geneva)

#define E extern

/*
 * Define PORT_HELP to be the name of the port-specfic help file.
 * This file is included into the resource fork of the application. 
 */
#define PORT_HELP "MacHelp"

E void NDECL(port_help);

E void NDECL(mac_init_nhwindows);
E void NDECL(mac_player_selection);
E void NDECL(mac_askname);
E void NDECL(mac_get_nh_event) ;
E void FDECL(mac_exit_nhwindows, (const char *));
E void FDECL(mac_suspend_nhwindows, (const char *));
E void NDECL(mac_resume_nhwindows);
E winid FDECL(mac_create_nhwindow, (int));
E void FDECL(mac_clear_nhwindow, (winid));
E void FDECL(mac_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(mac_destroy_nhwindow, (winid));
E void FDECL(mac_curs, (winid,int,int));
E void FDECL(mac_putstr, (winid, int, const char *));
E void FDECL(mac_display_file, (const char *, BOOLEAN_P));
E void FDECL(mac_start_menu, (winid));
E void FDECL(mac_add_menu, (winid, CHAR_P, int, const char *));
E void FDECL(mac_end_menu, (winid, CHAR_P, const char *, const char *));
E char FDECL(mac_select_menu, (winid));
E void NDECL(mac_update_inventory);
E void NDECL(mac_mark_synch);
E void NDECL(mac_wait_synch);
#ifdef CLIPPING
E void FDECL(mac_cliparound, (int, int));
#endif
E void FDECL(mac_print_glyph, (winid,XCHAR_P,XCHAR_P,int));
E void FDECL(mac_raw_print, (const char *));
E void FDECL(mac_raw_print_bold, (const char *));
E int NDECL(mac_nhgetch);
E int FDECL(mac_nh_poskey, (int *, int *, int *));
E void NDECL(mac_nhbell);
E int NDECL(mac_doprev_message);
E char FDECL(mac_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(mac_getlin, (const char *,char *));
#ifdef COM_COMPL
E void FDECL(mac_get_ext_cmd, (char *));
#endif /* COM_COMPL */
E void FDECL(mac_number_pad, (int));
E void NDECL(mac_delay_output);

/* defined in macwin.c and exported for used in mmodal.c */
E void		FDECL(HandleEvent, (EventRecord *));

/* defined in mmodal.c */
E DialogPtr FDECL(mv_get_new_dialog, (short));
E void		FDECL(mv_close_dialog, (DialogPtr));
E void		FDECL(mv_modal_dialog, (ModalFilterProcPtr, short *));

#undef E

extern void DimMenuBar ( void ) ;
extern void UndimMenuBar ( void ) ;
extern int SanePositions ( void ) ;

#define NHW_BASE 0
extern winid BASE_WINDOW , WIN_MAP , WIN_MESSAGE , WIN_INVEN , WIN_STATUS ;

extern Boolean itworked( short );
extern void mustwork( short );
extern void VDECL(progerror, (const char *,...));
extern void attemptingto( char *  );
extern void pushattemptingto( char *  );
extern void popattempt( void );
extern void UnloadAllSegments( void );
extern void InitSegMgmt( void * );
extern void IsResident ( void * );
extern void NotResident ( void * );

# endif /* ! MACCONF_H */
#endif /* MAC */
