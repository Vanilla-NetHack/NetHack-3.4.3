/*	SCCS Id: @(#)macwin.c	3.2	96/01/15	*/
/* Copyright (c) Jon W{tte, Hao-Yang Wang, Jonathan Handler 1992. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * macwin.c
 */

/*
 * I have to quote X11R5:
 *
 *  "If you think you know what this code is doing, you're probably wrong.
 *   Here be serious and nasty dragons."
 *
 * h+ 92-09-26
 */

#include "hack.h"
#include "func_tab.h"
#include "macwin.h"
#include "mactty.h"
#include "wintty.h"

#include <osutils.h>
#include <dialogs.h>
#include <textedit.h>
#include <menus.h>
#if defined(applec)
#include <sysequ.h>
#elif defined(__MWERKS__)
#include <LowMem.h>
#else
#include <LoMem.h>
#endif
#include <resources.h>
#include <desk.h>
#include <gestaltequ.h>
#include <balloons.h>
#include <toolutils.h>
#include <script.h>
#include <OSEvents.h>
#include <Packages.h>
#include <Sound.h>
#include <DiskInit.h>

static short kApplicInFront = 1;

NhWindow * theWindows = (NhWindow *) 0 ;

static Boolean ClosingWindowChar(const int c);
static Boolean in_topl_mode(void);
static void topl_flash_resp(int resp_idx);
static void topl_set_def(int new_def_idx);
static void adjust_window_pos(NhWindow *aWin, WindowPtr theWindow, short w);
static void mac_cliparound (int x, int y);


#ifndef USESROUTINEDESCRIPTORS /* not using universal headers */
  /* Cast everything in terms of the new Low Memory function calls. */
# if defined(applec)
#  define LMGetCurStackBase()	(*(long *) CurStackBase)
#  define LMGetDefltStack()		(*(long *) DefltStack)
#  define LMGetKeyThresh()		(*(short *) KeyThresh)
#  define LMGetKeyRepThresh()	(*(short *) KeyRepThresh)
# elif defined(THINK_C)
#  define LMGetCurStackBase()	CurStackBase
#  define LMGetDefltStack()		(*(long *) DefltStack)
#  define LMGetKeyThresh()		(*(short *) KeyThresh)
#  define LMGetKeyRepThresh()	(*(short *) KeyRepThresh)
# else
#  error /* need to define LM functions for this compiler */
# endif
#endif /* !USEROUTINEDESCRIPTORS (universal headers) */


/*
 * Borrowed from the Mac tty port
 */
extern WindowPtr _mt_window;

/*
 * Some useful #defines for the scroll bar width and height
 */
#define		SBARWIDTH	15
#define		SBARHEIGHT	13

/*
 * We put a TE on the message window for the "top line" queries.
 * top_line is the TE that holds both the query and the user's
 * response.  The first topl_query_len characters in top_line are
 * the query, the rests are the response.  topl_resp is the valid
 * response to a yn query, while topl_resp[topl_def_idx] is the
 * default response to a yn query.
 */
TEHandle top_line = (TEHandle) nil;
int		 topl_query_len;
int		 topl_def_idx = -1;
char	 topl_resp[10] = "";

#define CHAR_ANY '\n'

/*
 * inSelect means we have a menu window up for selection or
 * something similar. It makes the window with win number ==
 * inSelect a movable modal (unfortunately without the border)
 * and clicking the close box forces an RET into the key
 * buffer. Don't forget to set inSelect to WIN_ERR when you're
 * done...
 */
winid inSelect = WIN_ERR ;

/*
 * The key queue - contains 0 for unused or an ascii value.
 * When getting keys, take the first value and shift the
 * queue, shifting in a 0.
 */
#define QUEUE_LEN 24
const int keyQueueLen = QUEUE_LEN ;
static int keyQueue [ QUEUE_LEN ] ;

Boolean gClickedToMove = 0 ; /* For ObscureCursor */

static unsigned long gNextClickRepeat = 0xffffffff ;

static Point clicked_pos ;	/* For nh_poskey */
static int clicked_mod ;

static Boolean cursor_locked = false ;

static ControlActionUPP UpUPP, DownUPP;		/* scrolling callbacks, initialized in InitMac */

void
lock_mouse_cursor(Boolean new_cursor_locked)
{
	cursor_locked = new_cursor_locked;
	if (new_cursor_locked)
		PostEvent(osEvt, mouseMovedMessage<<24);
}


/*
 * Add key to input queue, force means flush left and replace if full
 */
void
AddToKeyQueue ( int ch , Boolean force )
{
	int i ;

	for ( i = 0 ; i < QUEUE_LEN ; i ++ ) {

		if ( ! keyQueue [ i ] ) {

			keyQueue [ i ] = ch ;
			return ;
		}
	}
	if ( force ) {
		for ( i = 0 ; i < QUEUE_LEN - 1 ; i ++ ) {
			keyQueue [ i ] = keyQueue [ i + 1 ] ;
		}
		keyQueue [ QUEUE_LEN - 1 ] = ch ;
	}
}


/*
 * Cursor movement
 */
RgnHandle gMouseRgn = (RgnHandle) 0 ;

/*
 * _Gestalt madness - we rely heavily on the _Gestalt glue, since we
 * don't check for the trap...
 */
MacFlags macFlags ;

/*
 * The screen layouts on the small 512x342 screen need special cares.
 */
Boolean small_screen;

/*
 * Async flag for keeping track of scrollbars...
 * Used by InvalScrollBar ( )
 */
static NhWindow * asyDSC = (NhWindow *) 0 ;

/*
 * The font to be used in the text window
 */
short text_wind_font = geneva;

/*
 * Whether to adjust the height of a text window according to its contents
 */
boolean full_screen = FALSE;

char macPad = 1 ;

Handle mBar ;
MenuHandle appleMenu ;
MenuHandle fileMenu ;
MenuHandle editMenu ;
MenuHandle actionMenu ;
MenuHandle inventoryMenu ;
MenuHandle thingsMenu ;
MenuHandle extendedMenu ;
MenuHandle infoMenu ;
MenuHandle helpMenu ;

#ifdef NHW_BASE
# undef NHW_BASE
#endif
#define NHW_BASE 0

static void FDECL(GeneralKey, ( EventRecord * theEvent , WindowPtr theWindow ));
static void FDECL(HandleKey, ( EventRecord * theEvent ));
static void FDECL(HandleClick, ( EventRecord * theEvent ));
static void FDECL(HandleUpdate, ( EventRecord * theEvent ));
static int FDECL(filter_scroll_key,(const int, NhWindow *));

#define NUM_FUNCS 6
static void FDECL(macKeyNull, ( EventRecord * , WindowPtr )) ;
static void FDECL(macKeyMessage, ( EventRecord * , WindowPtr )) ;
static void FDECL(macKeyTerm, ( EventRecord * , WindowPtr )) ;
static void FDECL(macKeyMenu, ( EventRecord * , WindowPtr )) ;
static void FDECL(macKeyText, ( EventRecord * , WindowPtr )) ;

static void FDECL(macClickNull, ( EventRecord * , WindowPtr )) ;
static void FDECL(macClickMessage, ( EventRecord * , WindowPtr )) ;
static void FDECL(macClickTerm, ( EventRecord * , WindowPtr )) ;
static void FDECL(macClickMenu, ( EventRecord * , WindowPtr )) ;
static void FDECL(macClickText, ( EventRecord * , WindowPtr )) ;

static void FDECL(macUpdateNull, ( EventRecord * , WindowPtr )) ;
static void FDECL(macUpdateMessage, ( EventRecord * , WindowPtr )) ;
static void FDECL(macUpdateTerm, ( EventRecord * , WindowPtr )) ;
static void FDECL(macUpdateMenu, ( EventRecord * , WindowPtr )) ;
static void FDECL(macUpdateText, ( EventRecord * , WindowPtr )) ;

static void FDECL(macCursorNull, ( EventRecord * , WindowPtr , RgnHandle )) ;
static void FDECL(macCursorMessage, ( EventRecord * , WindowPtr , RgnHandle )) ;
static void FDECL(macCursorTerm, ( EventRecord * , WindowPtr , RgnHandle )) ;
static void FDECL(macCursorMenu, ( EventRecord * , WindowPtr , RgnHandle )) ;
static void FDECL(macCursorText, ( EventRecord * , WindowPtr , RgnHandle )) ;


static void FDECL (DrawScrollbar, ( NhWindow * , WindowPtr ));
static void FDECL (InvalScrollBar, ( NhWindow * ));
static void FDECL(DoScrollBar,(Point, short, ControlHandle, NhWindow *, WindowPtr));
static pascal void FDECL(Up, (ControlHandle, short));
static pascal void FDECL(Down,(ControlHandle, short));


typedef void ( * CbFunc ) ( EventRecord * , WindowPtr ) ;
typedef void ( * CbCursFunc ) ( EventRecord * , WindowPtr , RgnHandle ) ;

CbFunc winKeyFuncs [ NUM_FUNCS ] = {
	macKeyNull , macKeyMessage , macKeyTerm , macKeyTerm , macKeyMenu , macKeyText
} ;

CbFunc winClickFuncs [ NUM_FUNCS ] = {
	macClickNull , macClickMessage , macClickTerm , macClickTerm , macClickMenu ,
	macClickText
} ;

CbFunc winUpdateFuncs [ NUM_FUNCS ] = {
	macUpdateNull , macUpdateMessage , macUpdateTerm , macUpdateTerm ,
	macUpdateMenu , macUpdateText
} ;

CbCursFunc winCursorFuncs [ NUM_FUNCS ] = {
	macCursorNull , macCursorMessage , macCursorTerm , macCursorTerm ,
	macCursorMenu , macCursorText
} ;

NhWindow *
GetNhWin(WindowPtr mac_win)
{
	int ix;
	for (ix = 0; ix < NUM_MACWINDOWS; ++ix)
		if (mac_win == theWindows[ix].theWindow)
			return theWindows + ix;
	return ((NhWindow *) nil) ;
}


#ifdef applec
  extern void _DataInit();
  /* This routine is part of the MPW runtime library. This external
	 reference to it is done so that we can unload its segment, %A5Init,
	 and recover c 250K of memory */
#endif


void
InitMac( void )
{
	int i ;
	long l ;
	long applLimit;

#ifdef MAC68K
 #ifdef applec
	UnloadSeg((Ptr) _DataInit);
 #endif
#endif

	if ( LMGetDefltStack() < 50 * 1024L ) {
		applLimit = (long) LMGetCurStackBase() - (50 * 1024L);
		SetApplLimit ( ( void * ) applLimit ) ;
	}
	MaxApplZone ( ) ;
	for ( i = 0 ; i < 5 ; i ++ )
		MoreMasters ( ) ;

	InitGraf ( & qd . thePort ) ;
	InitFonts ( ) ;
	InitWindows ( ) ;
	InitMenus ( ) ;
	InitDialogs ( ( ResumeProcPtr ) 0L ) ;
	TEInit ( ) ;

#if defined(MAC68K) && !defined(__MWERKS__)
	InitSegMgmt( itworked );	/* itworked is always in the main segment */
#endif

	attemptingto("start up");

	if ( Gestalt ( gestaltOSAttr , & l ) ) {
		macFlags . processes = 0 ;
		macFlags . tempMem = 0 ;
		macFlags . hasDebugger = 0 ;
	} else {
		macFlags . processes = ( l & ( 1 << gestaltLaunchControl ) ) ? 1 : 0 ;
		macFlags . tempMem = ( l & ( 1 << gestaltRealTempMemory ) ) ? 1 : 0 ;
		macFlags . hasDebugger = ( l & ( 1 << gestaltSysDebuggerSupport ) ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltQuickdrawVersion , & l ) ) {
		macFlags . color = 0 ;
	} else {
		macFlags . color = ( l >= gestalt8BitQD ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltFindFolderAttr , & l ) ) {
		macFlags . folders = 0 ;
	} else {
		macFlags . folders = ( l & ( 1 << gestaltFindFolderPresent ) ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltHelpMgrAttr , & l ) ) {
		macFlags . help = 0 ;
	} else {
		macFlags . help = ( l & ( 1 << gestaltHelpMgrPresent ) ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltFSAttr , & l ) ) {
		macFlags . fsSpec = 0 ;
	} else {
		macFlags . fsSpec = ( l & ( 1 << gestaltHasFSSpecCalls ) ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltFontMgrAttr , & l ) ) {
		macFlags . trueType = 0 ;
	} else {
		macFlags . trueType = ( l & ( 1 << gestaltOutlineFonts ) ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltAUXVersion , & l ) ) {
		macFlags . aux = 0 ;
	} else {
		macFlags . aux = ( l >= 0x200 ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltAliasMgrAttr , & l ) ) {
		macFlags . alias = 0 ;
	} else {
		macFlags . alias = ( l & ( 1 << gestaltAliasMgrPresent ) ) ? 1 : 0 ;
	}
	if ( Gestalt ( gestaltStandardFileAttr , & l ) ) {
		macFlags . standardFile = 0 ;
	} else {
		macFlags . standardFile = ( l & ( 1 << gestaltStandardFile58 ) ) ? 1 : 0 ;
	}

	gMouseRgn = NewRgn ( ) ;
	InitCursor ( ) ;
	ObscureCursor ( ) ;
	
	UpUPP = NewControlActionProc(Up);
	DownUPP = NewControlActionProc(Down);
}


#define MAX_HEIGHT 100
#define MIN_HEIGHT 50
#define MIN_WIDTH 300

/*
 * This function could be overloaded with any amount of
 * intelligence...
 */
int
SanePositions ( void )
{
	short mainTop , mainLeft ;
	short mesgTop , mesgLeft , mesgWidth , mesgHeight ;
	short left , top , width , height ;
	short ix , numText = 0 , numMenu = 0 ;
	Rect screenArea ;
	WindowPtr theWindow ;
	NhWindow * nhWin ;

	screenArea = qd . thePort -> portBits . bounds ;
	OffsetRect ( & screenArea , - screenArea . left , - screenArea . top ) ;

/* Map Window */
	nhWin = theWindows + WIN_MAP ;
	theWindow = nhWin -> theWindow ;

	height = nhWin -> charHeight * NUM_ROWS ;
	width = nhWin -> charWidth * NUM_COLS ;

	if ( ! RetrievePosition ( kMapWindow , & top , & left ) ) {
		top = GetMBarHeight ( ) + ( small_screen ? 2 : 20 ) ;
		left = ( screenArea . right - width ) / 2 ;
	}

	mainTop = top ;
	mainLeft = left ;

/* Message Window */
	nhWin = theWindows + WIN_MESSAGE ;
	theWindow = nhWin -> theWindow ;

	if ( ! RetrievePosition ( kMessageWindow , & top , & left ) ) {
		top = mainTop + _mt_window->portRect.bottom - _mt_window->portRect.top;
		if ( ! small_screen )
			top += 20 ;
		left = mainLeft ;
	}

	if ( ! RetrieveSize ( kMessageWindow , top , left , & height , & width ) ) {
		height = screenArea . bottom - top - ( small_screen ? 2-SBARHEIGHT : 2 ) ;
		if ( height > MAX_HEIGHT ) {
			height = MAX_HEIGHT ;
		} else if ( height < MIN_HEIGHT ) {
			height = MIN_HEIGHT ;
			width = MIN_WIDTH ;
			left = screenArea . right - width ;
			top = screenArea . bottom - MIN_HEIGHT ;
		}
	}
	mesgTop = top ;
	mesgLeft = left ;
	mesgHeight = height ;
	mesgWidth = width ;

/* Move these windows */
	MoveWindow ( theWindows [ WIN_MESSAGE ] . theWindow , mesgLeft , mesgTop , 1 ) ;
	SizeWindow ( theWindows [ WIN_MESSAGE ] . theWindow , mesgWidth , mesgHeight , 1 ) ;
	MoveWindow ( _mt_window , mainLeft , mainTop , 1 ) ;

/* Handle other windows */
	for ( ix = 0 ; ix < NUM_MACWINDOWS ; ix ++ ) {
		if ( ix != WIN_STATUS && ix != WIN_MESSAGE && ix != WIN_MAP && ix != BASE_WINDOW ) {
			if ( theWindow = theWindows [ ix ] . theWindow ) {
				if ( ( ( WindowPeek ) theWindow ) -> visible ) {
					if ( theWindows [ ix ] . kind == NHW_MENU ) {
						if ( ! RetrievePosition ( kMenuWindow , & top , & left ) ) {
							top = GetMBarHeight ( ) * 2 ;
							left = 2 ;
						}
						top += ( numMenu * GetMBarHeight ( ) ) ;
						while ( top > screenArea . bottom - MIN_HEIGHT ) {
							top -= ( screenArea . bottom - GetMBarHeight ( ) * 2 ) ;
							left += 20 ;
						}
						numMenu ++ ;
						MoveWindow ( theWindow , left , top , 1 ) ;
					} else {
						if ( ! RetrievePosition ( kTextWindow , & top , & left ) ) {
							top = GetMBarHeight ( ) * 2 ;
							left = screenArea . right - 3 - ( theWindow -> portRect . right -
															  theWindow -> portRect . left ) ;
						}
						top += ( numText * GetMBarHeight ( ) ) ;
						while ( top > screenArea . bottom - MIN_HEIGHT ) {
							top -= ( screenArea . bottom - GetMBarHeight ( ) * 2 ) ;
							left -= 20 ;
						}
						numText ++ ;
						MoveWindow ( theWindow , left , top , 1 ) ;
					}
				}
			}
		}
	}

	InitCursor ( ) ;

	return 0 ;
}


winid
mac_create_nhwindow ( int type )
{
	int i ;
	Rect siz ;
	NhWindow * aWin ;

	if ( type < NHW_BASE || type > NHW_TEXT ) {
		error ( "Invalid window type %d in create_nhwindow." , type ) ;
		return WIN_ERR ;
	}

	for ( i = 0 ; i < NUM_MACWINDOWS ; i ++ ) {
		if ( ! theWindows [ i ] . theWindow )
			break ;
	}
	if ( i >= NUM_MACWINDOWS ) {
		for ( i = 0 ; i < NUM_MACWINDOWS ; i ++ ) {
			WindowPeek w = ( WindowPeek ) theWindows [ i ] . theWindow ;
			if ( w -> visible || i == WIN_INVEN ||
				 w -> windowKind != WIN_BASE_KIND + NHW_MENU &&
				 w -> windowKind != WIN_BASE_KIND + NHW_TEXT )
				continue ;
			error ( "The window list is getting full, freeing unnecessary window (%d)..." ,
				i ) ;
			destroy_nhwindow ( i ) ;
			goto got1 ;
		}
		error ( "Out of window ids in create_nhwindow ! (Max = %d)" ,
			NUM_MACWINDOWS ) ;
		return WIN_ERR ;
	}

got1 :
	aWin = & theWindows [ i ] ;

	aWin -> windowTextLen = 0L ;
	aWin -> clear = 0 ; /* Yes, we need to inval the area on a clear */
	aWin -> scrollBar = (ControlHandle) 0 ;

	dprintf ( "Created window type %d" , type ) ;
	aWin -> kind = type ;
	aWin -> keyFunc = winKeyFuncs [ type ] ;
	aWin -> clickFunc = winClickFuncs [ type ] ;
	aWin -> updateFunc = winUpdateFuncs [ type ] ;
	aWin -> cursorFunc = winCursorFuncs [ type ] ;

	if (type == NHW_BASE || type == NHW_MAP || type == NHW_STATUS) {
		short x_sz, x_sz_p, y_sz_p;
		dprintf ( "This is in the TTY window" ) ;
		if (type == NHW_BASE) {
			aWin -> keyFunc = winKeyFuncs [ NHW_MAP ] ;
			aWin -> clickFunc = winClickFuncs [ NHW_MAP ] ;
			aWin -> updateFunc = winUpdateFuncs [ NHW_MAP ] ;
			aWin -> cursorFunc = winCursorFuncs [ NHW_MAP ] ;
		} else {
			if (i != tty_create_nhwindow(type)) {
				dprintf ( "Cannot create window type %d" , type ) ;
			}
			if (type == NHW_MAP) {
				wins[i]->offy = 0;	/* the message box is in a separate window */
			}
		}
		aWin->theWindow = _mt_window;
		((WindowPeek)aWin->theWindow)->windowKind = WIN_BASE_KIND + NHW_MAP;
		get_tty_metrics(aWin->theWindow, &x_sz, &aWin->lin, &x_sz_p, &y_sz_p,
										 &aWin->fontNum, &aWin->fontSize,
										 &aWin->charWidth, &aWin->charHeight);
		return i;
	}

	aWin -> theWindow = GetNewWindow ( WIN_BASE_RES + type ,
		( WindowPtr ) 0L , ( WindowPtr ) -1L ) ;
	( ( WindowPeek ) aWin -> theWindow ) -> windowKind = WIN_BASE_KIND
		+ type ;
	if ( ! ( aWin -> windowText = NewHandle ( TEXT_BLOCK ) ) ) {
		error ( "NewHandle failed in create_nhwindow (%ld bytes)" ,
			( long ) TEXT_BLOCK ) ;
		DisposeWindow ( aWin -> theWindow ) ;
		aWin -> theWindow = (WindowPtr) 0 ;
		return WIN_ERR ;
	}
	aWin -> lin = 0 ;
	aWin -> menuChar = 'a' ;
	clear_nhwindow ( i ) ;

/*HARDCODED*/

	SetPort ( aWin -> theWindow ) ;
	PenPat ( (void *) &qd . black ) ;

	switch ( type ) {
		case NHW_MESSAGE :
			GetFNum ( "\pPSHackFont", & aWin -> fontNum ) ;
			aWin -> fontSize = flags.large_font ? 12 : 9 ;
			break ;
		default:
			aWin -> fontNum = text_wind_font ;
			aWin -> fontSize = 9 ;
	}

	TextFont ( aWin -> fontNum ) ; 
	TextSize ( aWin -> fontSize ) ;

	if (type == NHW_MESSAGE && !top_line) {
		const Rect out_of_scr = { 10000, 10000, 10100, 10100 };
		TextFace(bold);
		top_line = TENew(&out_of_scr, &out_of_scr);
		TEActivate(top_line);
		TextFace(normal);
	}

	{
		FontInfo fi ;
		GetFontInfo ( & fi ) ;
		aWin -> leading = fi . leading / 2 + fi . descent ;
		aWin -> charHeight = fi . ascent + fi . descent + fi . leading ;
		aWin -> charWidth = fi . widMax ;
	}
	SetPt ( & ( theWindows [ i ] . cursor ) , 0 , 0 ) ;

	siz = aWin -> theWindow -> portRect ;
	OffsetRect ( & siz , 30 + rn2 ( 20 ) , 250 + rn2 ( 50 ) ) ;
	SizeWindow ( aWin -> theWindow , siz . right - siz . left ,
		siz . bottom - siz . top , FALSE ) ;
	MoveWindow ( aWin -> theWindow , siz . left , siz . top , FALSE ) ;

	if ( type == NHW_MENU || type == NHW_TEXT || type == NHW_MESSAGE ) {
		Rect r = siz ;
		Boolean bool ;
		OffsetRect ( & r , - r . left , - r . top ) ;
		r . left = r . right - SBARWIDTH ;
		r . bottom -= SBARHEIGHT ;
		r . top -= 1 ;
		r . right += 1 ;
		bool = ( r . bottom > r . top + 50 ) ;
		aWin -> scrollBar = NewControl ( aWin -> theWindow , & r , "\p" ,
			bool , 0 , 0 , 0 , 16 , 0L ) ;
	}
	aWin -> scrollPos = 0 ;

	return i ;
}


static MenuHandle
mustGetMHandle(int menu_id)
{
	MenuHandle menu = GetMHandle(menu_id);
	if (menu == (MenuHandle)  nil) {
		comment("Cannot find the menu.", menu_id);
		ExitToShell();
	}
	return menu;
}


#define A_LOT 0x7fffffff
#define HEAP_SPACE 100000
#define RES_FACTOR 1.1

static long
SpaceToLoad ( char * name ) {
FCBPBRec fcb ;
long ret ;

	memset ( & fcb , 0 , sizeof ( fcb ) ) ;

	fcb . ioRefNum = CurResFile ( ) ;
	fcb . ioNamePtr = (uchar *) name ;
	if ( PBGetFCBInfoSync ( & fcb ) ) {
		return A_LOT ;
	}
	ret = fcb . ioFCBEOF * RES_FACTOR + HEAP_SPACE ;
	dprintf ( "Space needed %ld" , ret ) ;
	return ret ;
}


void
InitRes ( void )
{
	InitMenuRes();
}


void
mac_init_nhwindows ( int *argcp, char **argv )
{
	int i ;

	Rect scr = (*GetGrayRgn())->rgnBBox;
	small_screen = scr.bottom - scr.top <=	9*40 ||
				   scr.bottom - scr.top <= 12*40 && flags.large_font;

	InitRes ( ) ;

	theWindows = ( NhWindow * ) NewPtr ( NUM_MACWINDOWS *
		sizeof ( NhWindow ) ) ;
	mustwork(MemError());

	for ( i = 0 ; i < NUM_MACWINDOWS ; i ++ ) {
		theWindows [ i ] . theWindow = (WindowPtr) 0 ;
	}
	for ( i = 0 ; i < QUEUE_LEN ; i ++ ) {
		keyQueue [ i ] = 0 ;
	}

	DimMenuBar ( ) ;

	tty_init_nhwindows(argcp, argv);
	flags.window_inited = TRUE;

	/* Some ugly hacks to make both interfaces happy */
	mac_create_nhwindow(NHW_BASE);
	tty_create_nhwindow(NHW_MESSAGE);
}


void
mac_nhbell ( void )
{
	Handle h ;

	if ( h = GetNamedResource ( 'snd ' , "\pNetHack Bell" ) ) {

		HLock ( h ) ;
		SndPlay ( ( SndChannelPtr ) 0 , (SndListHandle) h , 0 ) ;
		ReleaseResource ( h ) ;

	} else {

		SysBeep ( 30 ) ;
	}
}


static void
DrawScrollbar ( NhWindow * aWin , WindowPtr theWindow )
{
Rect r2 = theWindow -> portRect ;
Rect r = r2 ;
Boolean vis ;
short val , lin ;

	if ( ! aWin -> scrollBar ) {
		return ;
	}

	r2 . left = r2 . right - SBARWIDTH ;
	r2 . bottom -= SBARHEIGHT ;
	r2 . right += 1 ;
	r2 . top -= 1 ;
	if ( ( * aWin -> scrollBar ) -> contrlRect . top  != r2 . top ||
		 ( * aWin -> scrollBar ) -> contrlRect . left != r2 . left ) {
		MoveControl ( aWin -> scrollBar , r2 . left , r2 . top ) ;
	}
	if ( ( * aWin -> scrollBar ) -> contrlRect . bottom != r2 . bottom ||
		 ( * aWin -> scrollBar ) -> contrlRect . right  != r2 . right ) {
		SizeControl ( aWin -> scrollBar , r2 . right  - r2 . left ,
										  r2 . bottom - r2 . top ) ;
	}
	vis = ( r2 . bottom > r2 . top + 50 ) ;
	if ( vis && ! ( * aWin -> scrollBar ) -> contrlVis ) {
		ShowControl ( aWin -> scrollBar ) ;
	} else if ( ! vis && ( * aWin -> scrollBar ) -> contrlVis ) {
		HideControl ( aWin -> scrollBar ) ;
	}
	lin = aWin -> lin;
	if ( aWin == theWindows + WIN_MESSAGE ) {
		short min ;
		r . bottom -= SBARHEIGHT + 1 ;
		min = aWin -> save_lin + ( r . bottom - r . top ) / aWin ->
			charHeight ;
		if ( lin < min ) {
			lin = min ;
		}
	}
	if ( lin ) {
		short max = lin - ( r . bottom - r . top ) / aWin -> charHeight ;
		if ( max < 0 ) {
			max = 0 ;
		}
		SetCtlMax ( aWin -> scrollBar , max ) ;
		if ( max ) {
			HiliteControl ( aWin -> scrollBar , 0 ) ;
		} else {
			HiliteControl ( aWin -> scrollBar , 255 ) ;
		}
	} else {
		HiliteControl ( aWin -> scrollBar , 255 ) ;
	}
	val = GetCtlValue ( aWin -> scrollBar ) ;
	if ( val != aWin -> scrollPos ) {
		InvalRect ( & ( theWindow -> portRect ) ) ;
		aWin -> scrollPos = val ;
	}
	if ( aWin == asyDSC ) {
		asyDSC = (NhWindow *) 0 ;
	}
}


static void
InvalScrollBar ( NhWindow * aWin )
{
	if ( asyDSC && asyDSC != aWin ) {
		SetPort ( asyDSC -> theWindow ) ;
		DrawScrollbar ( asyDSC , asyDSC -> theWindow ) ;
	}
	asyDSC = aWin ;
}


void
mac_clear_nhwindow ( winid win )
{
	long l ;
	Rect r ;
	WindowPtr theWindow ;
	NhWindow * aWin = & theWindows [ win ] ;

	if ( win < 0 || win >= NUM_MACWINDOWS ) {
		error ( "Invalid window %d in clear_nhwindow." , win ) ;
		return ;
	}

	theWindow = aWin -> theWindow ;
	if (theWindow == _mt_window) {
		tty_clear_nhwindow(win);
		return;
	}

	if ( aWin -> clear )
		return ;
	if ( ! theWindow ) {
		error ( "Unallocated window %d in clear_nhwindow." , win ) ;
		return ;
	}
	SetPort ( theWindow ) ;
	r = theWindow -> portRect ;
	if ( aWin -> scrollBar )
		r . right -= SBARWIDTH + 1 ;
	if ( aWin == theWindows + WIN_MESSAGE )
		r . bottom -= SBARHEIGHT + 1 ;
	InvalRect ( & r ) ;
	switch ( ( ( WindowPeek ) theWindow ) -> windowKind - WIN_BASE_KIND ) {
	case NHW_MESSAGE :
		l = 0;
		while (aWin->lin > flags.msg_history) {
			const char cr = CHAR_CR;
			l = Munger(aWin->windowText, l, &cr, 1, nil, 0) + 1;
			--aWin->lin;
		}
		if (l) {
			aWin->windowTextLen -= l;
			BlockMove(*aWin->windowText + l, *aWin->windowText, aWin->windowTextLen);
		}
		aWin->last_more_lin = aWin->lin;
		aWin->save_lin		= aWin->lin;
		aWin->scrollPos = aWin->lin ? aWin->lin - 1 : 0;
		if (aWin->scrollBar) {
			SetCtlMax  (aWin->scrollBar, aWin->lin);
			SetCtlValue(aWin->scrollBar, aWin->scrollPos);
			InvalScrollBar(aWin);
		}
		break ;
	default :
		SetHandleSize ( aWin -> windowText , TEXT_BLOCK ) ;
		aWin -> windowTextLen = 0L ;
		aWin -> lin = 0 ;
		aWin -> wid = 0 ;
		aWin -> scrollPos = 0 ;
		if ( aWin -> scrollBar ) {
			SetCtlMax ( aWin -> scrollBar , 0 ) ;
			InvalScrollBar ( aWin ) ;
		}
		break ;
	}
	SetPt ( & ( aWin -> cursor ) , 0 , 0 ) ;
	aWin -> clear = 1 ;
}


static Boolean
ClosingWindowChar(const int c) {
	return  c == CHAR_ESC || c == CHAR_BLANK || c == CHAR_LF || c == CHAR_CR ||
			c == 'q' ;
}


#define BTN_IND 2
#define BTN_W	40
#define BTN_H	(SBARHEIGHT-2)

static void
topl_resp_rect(int resp_idx, Rect *r)
{
	r->left	  = (BTN_IND + BTN_W) * resp_idx + BTN_IND;
	r->right  = r->left + BTN_W;
	r->bottom = theWindows[WIN_MESSAGE].theWindow->portRect.bottom - 1;
	r->top	  = r->bottom - BTN_H;
}


static Boolean
in_topl_mode(void)
{
	return top_line &&
		   (*top_line)->viewRect.left < theWindows[WIN_MESSAGE].theWindow->portRect.right;
}


void
enter_topl_mode(char *query)
{
	if (!top_line || in_topl_mode())
		Debugger();

	putstr(WIN_MESSAGE, ATR_BOLD, query);

	(*top_line)->viewRect.left = 0;

	topl_query_len = strlen(query);
	(*top_line)->selStart = topl_query_len;
	(*top_line)->selEnd	  = topl_query_len;
	PtrToXHand(query, (*top_line)->hText, topl_query_len);
	TECalText(top_line);

	display_nhwindow(WIN_MESSAGE, FALSE);
}


void
leave_topl_mode(char *answer)
{
	char *ap, *bp;

	int ans_len = (*top_line)->teLength - topl_query_len;
	NhWindow *aWin = theWindows + WIN_MESSAGE;

	if (!in_topl_mode())
		Debugger();

	BlockMove(*(*top_line)->hText + topl_query_len, answer, ans_len);
	answer[ans_len] = '\0';

	/* remove unprintables from the answer */
	for (ap = bp = answer; *ap; ap++) {
		if (*ap >= ' ' && *ap < 128) {
			if (bp != ap)
				*bp = *ap;
			bp++;
		}
	}
	*bp = 0;
	
	if ( aWin -> windowTextLen > 0 &&
		 ( * aWin -> windowText ) [ aWin -> windowTextLen - 1 ] == CHAR_CR ) {
		-- aWin -> windowTextLen ;
		-- aWin -> lin ;
	}
	putstr(WIN_MESSAGE, ATR_BOLD, answer);

	(*top_line)->viewRect.left += 10000;
}

/*
 * TESetSelect flushes out all the pending key strokes.  I hate it.
 */
static void
topl_set_select(short selStart, short selEnd)
{
	TEDeactivate(top_line);
	(*top_line)->selStart = selStart;
	(*top_line)->selEnd	  = selEnd;
	TEActivate(top_line);
}


static void
topl_replace(char *new_ans)
{
	topl_set_select(topl_query_len, (*top_line)->teLength);
	TEDelete(top_line);
	TEInsert(new_ans, strlen(new_ans), top_line);
}


Boolean
topl_key(unsigned char ch)
{
	switch (ch) {
		case CHAR_ESC:
			topl_replace("\x1b");
			return false;
		case CHAR_ENTER: case CHAR_CR: case CHAR_LF:
			return false;

		case 0x1f & 'P':
			nh_doprev_message();
			return true;
		case '\x1e'/* up arrow */:
			topl_set_select(topl_query_len, topl_query_len);
			return true;
		case CHAR_BS: case '\x1c'/* left arrow */:
			if ((*top_line)->selEnd > topl_query_len)
				TEKey(ch, top_line);
			return true;
		default:
			TEKey(ch, top_line);
			return true;
	}
}


Boolean
topl_ext_key(unsigned char ch)
{
	switch (ch) {
		case CHAR_ESC:
			topl_replace("\x1b");
			return false;
		case CHAR_ENTER: case CHAR_CR: case CHAR_LF:
			return false;

		case 0x1f & 'P':
			nh_doprev_message();
			return true;
		case CHAR_BS:
			topl_replace("");
			return true;
		default: {
			int com_index = -1, oindex = 0;
			TEInsert(&ch, 1, top_line);
			while(extcmdlist[oindex].ef_txt != (char *)0){
				if(!strncmpi(*(*top_line)->hText + topl_query_len,
							 extcmdlist[oindex].ef_txt,
							 (*top_line)->teLength - topl_query_len))
					if(com_index == -1) /* No matches yet*/
					    com_index = oindex;
					else /* More than 1 match */
					    com_index = -2;
				oindex++;
			}
			if(com_index >= 0)
				topl_replace((char *) extcmdlist[com_index].ef_txt);
			return true;
		}
	}
}


static void
topl_flash_resp(int resp_idx)
{
	long dont_care;
	Rect frame;
	SetPort(theWindows[WIN_MESSAGE].theWindow);
	topl_resp_rect(resp_idx, &frame);
	InsetRect(&frame, 1, 1);
	InvertRect(&frame);
	Delay(GetDblTime() / 2, &dont_care);
	InvertRect(&frame);
}


static void
topl_set_def(int new_def_idx)
{
	Rect frame;
	SetPort(theWindows[WIN_MESSAGE].theWindow);
	topl_resp_rect(topl_def_idx, &frame);
	InvalRect(&frame);
	topl_def_idx = new_def_idx;
	topl_resp_rect(new_def_idx, &frame);
	InvalRect(&frame);
}


void
topl_set_resp(char *resp, char def)
{
	char *loc;
	Rect frame;
	int r_len, r_len1;

	if (!resp) {
		const char any_str[2] = { CHAR_ANY, '\0' };
		resp = (char *) any_str;
		def = CHAR_ANY;
	}

	SetPort(theWindows[WIN_MESSAGE].theWindow);
	r_len1 = strlen(resp);
	r_len  = strlen(topl_resp);
	if (r_len < r_len1)
		r_len = r_len1;
	topl_resp_rect(0, &frame);
	frame.right = (BTN_IND + BTN_W) * r_len;
	InvalRect(&frame);

	strcpy(topl_resp, resp);
	loc = strchr (resp, def);
	topl_def_idx = loc ? loc - resp : -1;
}


static char
topl_resp_key(char ch)
{
	if (strlen(topl_resp) > 0) {
		char *loc = strchr(topl_resp, ch);

		if (!loc)
			if (ch == '\x9'/* tab */) {
				topl_set_def(topl_def_idx <= 0 ? strlen(topl_resp)-1 : topl_def_idx-1);
				ch = '\0';

			} else if (ch == CHAR_ESC) {
				loc = strchr(topl_resp, 'q');
				if (!loc) {
					loc = strchr(topl_resp, 'n');
					if (!loc && topl_def_idx >= 0)
						loc = topl_resp + topl_def_idx;
				}

			} else if (ch == (0x1f & 'P')) {
				nh_doprev_message();
				ch = '\0';

			} else if (topl_def_idx >= 0) {
				if ( ch == CHAR_ENTER || ch == CHAR_CR || ch == CHAR_LF ||
					 ch == CHAR_BLANK || topl_resp[topl_def_idx] == CHAR_ANY )
					loc = topl_resp + topl_def_idx;

				else if (strchr(topl_resp, '#')) {
					if (digit(ch)) {
						topl_set_def(strchr(topl_resp, '#') - topl_resp);
						TEKey(ch, top_line);
						ch = '\0';

					} else if (topl_resp[topl_def_idx] == '#')
						if (ch == '\x1e'/* up arrow */) {
							topl_set_select(topl_query_len, topl_query_len);
							ch = '\0';
						} else if (ch == '\x1d'/* right arrow */ ||
								   ch == '\x1f'/* down arrow */ ||
								   (ch == CHAR_BS || ch == '\x1c'/* left arrow */) &&
										(*top_line)->selEnd > topl_query_len) {
							TEKey(ch, top_line);
							ch = '\0';
						}
				}
			}

		if (loc) {
			topl_flash_resp(loc - topl_resp);
			if (*loc != CHAR_ANY)
				ch = *loc;
			TEKey(ch, top_line);
		}
	}

	return ch;
}


static void
adjust_window_pos(NhWindow *aWin, WindowPtr theWindow, short w)
{
#ifdef THINK_C
	Rect scr_r   = (*GetGrayRgn())->rgnBBox, win_ind = { 20, 2, 3, 3 };
#else
	const Rect scr_r   = (*GetGrayRgn())->rgnBBox, win_ind = { 20, 2, 3, 3 };
#endif
	const short	min_w = theWindow->portRect.right - theWindow->portRect.left,
				max_w = scr_r.right - scr_r.left - win_ind.left - win_ind.right;
	Point pos;
	short h = aWin->lin * aWin->charHeight, max_h;

	SetPort(theWindow);
	if (!RetrieveWinPos(theWindow, &pos.v, &pos.h)) {
		pos.v = theWindow->portRect.top;
		pos.h = theWindow->portRect.left;
		LocalToGlobal(&pos);
	}

	max_h = full_screen ? scr_r.bottom - win_ind.bottom - scr_r.top - win_ind.top
						: scr_r.bottom - win_ind.bottom - pos.v;
	if (h > max_h	  ) h = max_h;
	if (h < MIN_HEIGHT) h = MIN_HEIGHT;
	if (w < min_w	  ) w = min_w;
	if (w > max_w	  ) w = max_w;
	SizeWindow(theWindow, w, h, true);

	if (pos.v + h + win_ind.bottom > scr_r.bottom)
		pos.v = scr_r.bottom - h - win_ind.bottom;
	if (pos.h + w + win_ind.right  > scr_r.right)
		pos.h = scr_r.right	 - w - win_ind.right;
	MoveWindow(theWindow, pos.h, pos.v, false);
}


/*
 * display/select/update the window.
 * If f is true, this window should be "modal" - don't return
 * until presumed seen.
 */
void
mac_display_nhwindow ( winid win , BOOLEAN_P f )
{
	WindowPtr theWindow ;
	NhWindow * aWin = & theWindows [ win ] ;

	if ( win < 0 || win >= NUM_MACWINDOWS ) {
		error ( "Invalid window %d in display_nhwindow." , win ) ;
		return ;
	}
	theWindow = aWin -> theWindow ;
	if ( ! theWindow ) {
		error ( "Unallocated window %d in display_nhwindow." , win ) ;
		return ;
	}

	if (theWindow == _mt_window) {
		tty_display_nhwindow(win, f);
		return;
	}

	if ( f && inSelect == WIN_ERR && win == WIN_MESSAGE ) {
		topl_set_resp ( (char *)0 , 0 ) ;
		if ( aWin -> windowTextLen > 0 &&
			 ( * aWin -> windowText ) [ aWin -> windowTextLen - 1 ] == CHAR_CR ) {
			-- aWin -> windowTextLen ;
			-- aWin -> lin ;
		}
		putstr ( win , flags . standout ? ATR_INVERSE : ATR_NONE , " --More--" ) ;
	}

	if ( ! ( ( WindowPeek ) theWindow ) -> visible || full_screen ) {

		if ( win != WIN_MESSAGE ) {
			const char cr = CHAR_CR;
			short w = 0;
			long line_start = 0;
			HLock(aWin->windowText);

			while (line_start < aWin->windowTextLen) {
				long line_end;
				short line_w;
				line_end = Munger(aWin->windowText, line_start, &cr, 1, nil, 0);
				if (line_end < 0)
					line_end = aWin->windowTextLen;
				line_w = TextWidth(*aWin->windowText, line_start, line_end - line_start);
				if (w < line_w)
					w = line_w;
				line_start = line_end + 1;
			}
			HUnlock(aWin->windowText);

			adjust_window_pos(aWin, theWindow, w + (SBARWIDTH+2));
		}

		if ( ! small_screen || win != WIN_MESSAGE || f )
			SelectWindow ( theWindow ) ;
		ShowWindow ( theWindow ) ;
	}

	SetPort ( theWindow ) ;
	if ( aWin -> scrollBar ) {
		InvalScrollBar ( aWin ) ;
	}

	if ( f && inSelect == WIN_ERR ) {
		int ch ;

		inSelect = win ;
		do {
			ch = nhgetch ( ) ;
		} while ( ! ClosingWindowChar ( ch ) ) ;
		inSelect = WIN_ERR ;

		if ( win == WIN_MESSAGE )
			topl_set_resp ( "" , '\0' ) ;
		else
			HideWindow ( theWindow ) ;

	} else {
		wait_synch ( ) ;
	}
}


void
mac_destroy_nhwindow ( winid win )
{
	WindowPtr theWindow ;
	NhWindow * aWin = & theWindows [ win ] ;
	int kind , visible ;

	if ( win < 0 || win >= NUM_MACWINDOWS ) {
		if ( flags . window_inited )
			error ( "Invalid window number %d (Max %d) in destroy_nhwindow." ,
				win , NUM_MACWINDOWS ) ;
		return ;
	}
	theWindow = aWin -> theWindow ;
	if ( ! theWindow ) {
		error ( "Not allocated window number %d in destroy_nhwindow." ,
			win ) ;
		return ;
	}
/*
 *	if (theWindow == _mt_window) {
 *		tty_destroy_nhwindow(win);
 *		return;
 *	}
 */

	/*
	 * Check special windows.
	 * The base window should never go away.
	 * The other "standard" windows should not go away
	 * unless we've exitted nhwindows.
	 */
	if ( win == BASE_WINDOW ) {
		return ;
	}
	if ( win == WIN_INVEN || win == WIN_MESSAGE ) {
		if ( flags . window_inited ) {
			if ( flags . tombstone && killer ) {
				/* Prepare for the coming of the tombstone window. */
				mono_font ( ) ;
				full_screen = TRUE ;
			}
			return ;
		}
	}

	kind = ( ( WindowPeek ) theWindow ) -> windowKind - WIN_BASE_KIND ;
	visible = ( ( WindowPeek ) theWindow ) -> visible ;

	if ( ( ! visible || ( kind != NHW_MENU && kind != NHW_TEXT ) ) &&
		theWindow != _mt_window ) {
		DisposeWindow ( theWindow ) ;
		if ( aWin -> windowText ) {
			DisposHandle ( aWin -> windowText ) ;
		}
		aWin -> theWindow = (WindowPtr) 0 ;
		aWin -> windowText = (Handle) 0 ;
	}
}


void
mac_number_pad ( int pad )
{
	macPad = pad ;
}


void
trans_num_keys(EventRecord *theEvent)
{
	if (macPad) {
		Handle h = GetResource('Nump', theEvent->modifiers & shiftKey ? 129 : 128);
		if (h) {
			short *ab = (short *)*h;
			int i = ab[0];
			while (i) {
				if ((theEvent->message & keyCodeMask) == (ab[i] & keyCodeMask)) {
					theEvent->message = ab[i];
					break;
				}
				--i;
			}
		}
	}
}


/*
 * Note; theWindow may very well be null here, since keyDown may call
 * it when theres no window !!!
 */
void
GeneralKey ( EventRecord * theEvent , WindowPtr theWindow )
{
#if defined(applec)
# pragma unused(theWindow)
#endif
	trans_num_keys ( theEvent ) ;
	AddToKeyQueue ( topl_resp_key ( theEvent -> message & 0xff ) , 1 ) ;
}


static void
macKeyNull ( EventRecord * theEvent , WindowPtr theWindow )
{
	GeneralKey ( theEvent , theWindow ) ;
}


static void
macKeyMessage ( EventRecord * theEvent , WindowPtr theWindow )
{
	GeneralKey ( theEvent , theWindow ) ;
}


static void
macKeyTerm ( EventRecord * theEvent , WindowPtr theWindow )
{
	GeneralKey ( theEvent , theWindow ) ;
}


static void
macKeyMenu ( EventRecord * theEvent , WindowPtr theWindow )
{
	NhWindow *aWin = GetNhWin(theWindow);
	int ch = theEvent->message & 0xff;
	int l;
	Rect r;

	if (aWin) {
		for (l = 0; l < aWin->lin; l++)
			if (aWin->itemChars[l] == ch) {
				aWin->itemSelected[l] = !aWin->itemSelected[l];
				if (aWin->how != PICK_ANY)	/* pick one or pick none */
					AddToKeyQueue(CHAR_CR, 1);
				break;
			}

		if (l < aWin->lin && l >= aWin->scrollPos /*&& l < ??? */) {
			SetPort(theWindow);
			r = theWindow->portRect;
			if (aWin->scrollBar)
				r.right -= SBARWIDTH;			
			r.top = l * aWin->charHeight;
			r.bottom = (l + 1) * aWin->charHeight;
			InvertRect(&r);
		}

		/* add key if didn't find it in menu and not filtered */
		if (l == aWin->lin && filter_scroll_key (ch, aWin))
			GeneralKey (theEvent, theWindow);
	}
}


static void
macKeyText ( EventRecord * theEvent , WindowPtr theWindow )
{
	char c = filter_scroll_key ( theEvent -> message & 0xff ,
								 GetNhWin ( theWindow ) ) ;
	if ( c )
		if ( inSelect == WIN_ERR && ClosingWindowChar ( c ) ) {
			HideWindow ( theWindow ) ;
			destroy_nhwindow ( GetNhWin ( theWindow ) - theWindows ) ;
		} else {
			GeneralKey ( theEvent , theWindow ) ;
		}
}


static void
macClickNull ( EventRecord * theEvent , WindowPtr theWindow )
{
	if ( ! theEvent || ! theWindow ) {
		Debugger ( ) ;
	}
}


static void
macClickMessage ( EventRecord * theEvent , WindowPtr theWindow )
{
	int r_idx = 0;
	Point mouse = theEvent->where;
	GlobalToLocal(&mouse);
	while (topl_resp[r_idx]) {
		Rect frame;
		topl_resp_rect(r_idx, &frame);
		InsetRect(&frame, 1, 1);
		if (PtInRect(mouse, &frame)) {

			Boolean in_btn = true;
			InvertRect(&frame);
			while (WaitMouseUp()) {
				SystemTask();
				GetMouse(&mouse);
				if (PtInRect(mouse, &frame) != in_btn) {
					in_btn = !in_btn;
					InvertRect(&frame);
				}
			}
			if (in_btn) {
				InvertRect(&frame);
				AddToKeyQueue ( topl_resp [ r_idx ] , 1 ) ;
			}
			return;

		}
		++r_idx;
	}

	macClickText(theEvent, theWindow);
}


static void
macClickTerm ( EventRecord * theEvent , WindowPtr theWindow )
{
	int shift_down = theEvent->modifiers & shiftKey;
	NhWindow *nhw = GetNhWin(theWindow);
	Point where = theEvent->where;
		GlobalToLocal(&where);
		where.h = where.h / nhw->charWidth + 1;
		where.v = where.v / nhw->charHeight;
	clicked_mod = shift_down ? CLICK_2 : CLICK_1;

	if (strchr(topl_resp, click_to_cmd(where.h, where.v, clicked_mod)))
		nhbell();
	else {
		if (cursor_locked)
			while (WaitMouseUp())
				SystemTask();
		else if (!shift_down)
			gNextClickRepeat = TickCount() + LMGetKeyThresh();

		gClickedToMove = TRUE;
		clicked_pos = where;
	}
}

static amtToScroll = 0 ;
static NhWindow * winToScroll = (NhWindow *) 0 ;

static pascal void
Up ( ControlHandle theBar , short part )
{
	EventRecord fake ;
	short now = GetCtlValue ( theBar ) ;
	short min = GetCtlMin ( theBar ) ;
	Rect r ;
	RgnHandle rgn = NewRgn ( ) ;

	if ( ! part ) {
		return ;
	}

	if ( now - min < amtToScroll ) {
		amtToScroll = now - min ;
	}
	if ( ! amtToScroll ) {
		return ;
	}
	SetCtlValue ( theBar , now - amtToScroll ) ;
	winToScroll -> scrollPos = now - amtToScroll ;
	r = winToScroll -> theWindow -> portRect ;
	r . right -= SBARWIDTH ;
	if ( winToScroll == theWindows + WIN_MESSAGE )
		r . bottom -= SBARHEIGHT + 1 ;
	ScrollRect ( & r , 0 , amtToScroll * winToScroll -> charHeight , rgn ) ;
	if ( rgn ) {
		InvalRgn ( rgn ) ;
		BeginUpdate ( winToScroll -> theWindow ) ;
	}
	winToScroll -> updateFunc  ( & fake , winToScroll -> theWindow ) ;
	if ( rgn ) {
		EndUpdate ( winToScroll -> theWindow ) ;
		DisposeRgn ( rgn ) ;
	}
}


static pascal void
Down ( ControlHandle theBar , short part )
{
	EventRecord fake ;
	short now = GetCtlValue ( theBar ) ;
	short max = GetCtlMax ( theBar ) ;
	Rect r ;
	RgnHandle rgn = NewRgn ( ) ;

	if ( ! part ) {
		return ;
	}

	if ( max - now < amtToScroll ) {
		amtToScroll = max - now ;
	}
	if ( ! amtToScroll ) {
		return ;
	}
	SetCtlValue ( theBar , now + amtToScroll ) ;
	winToScroll -> scrollPos = now + amtToScroll ;
	r = winToScroll -> theWindow -> portRect ;
	r . right -= SBARWIDTH ;
	if ( winToScroll == theWindows + WIN_MESSAGE )
		r . bottom -= SBARHEIGHT + 1 ;
	ScrollRect ( & r , 0 , - amtToScroll * winToScroll -> charHeight , rgn ) ; 
	if ( rgn ) {
		InvalRgn ( rgn ) ;
		BeginUpdate ( winToScroll -> theWindow ) ;
	}
	winToScroll -> updateFunc  ( & fake , winToScroll -> theWindow ) ;
	if ( rgn ) {
		EndUpdate ( winToScroll -> theWindow ) ;
		DisposeRgn ( rgn ) ;
	}
}


static void
DoScrollBar ( Point p , short code , ControlHandle theBar , NhWindow * aWin ,
	WindowPtr theWindow )
{
	ControlActionUPP func;

	winToScroll = aWin ;
	switch ( code ) {
	case inUpButton :
		func = UpUPP ;
		amtToScroll = 1 ;
		break ;
	case inDownButton :
		func = DownUPP ;
		amtToScroll = 1 ;
		break ;
	case inPageUp :
		func = UpUPP ;
		amtToScroll = ( theWindow -> portRect . bottom - theWindow ->
			portRect . top ) / aWin -> charHeight ;
		break ;
	case inPageDown :
		func = DownUPP ;
		amtToScroll = ( theWindow -> portRect . bottom - theWindow ->
			portRect . top ) / aWin -> charHeight ;
		break ;
	default :
		break ;
	}

	( void ) TrackControl ( theBar , p , func );
	if ( ! func ) {
		if ( aWin -> scrollPos != GetCtlValue ( theBar ) ) {
			aWin -> scrollPos = GetCtlValue ( theBar ) ;
			InvalRect ( & ( theWindow -> portRect ) ) ;
		}
	}
}


int
filter_scroll_key(const int ch, NhWindow *aWin)
{
	if (aWin->scrollBar && GetCtlValue(aWin->scrollBar) < GetCtlMax(aWin->scrollBar)) {
		winToScroll = aWin;
		SetPort(aWin->theWindow);
		if (ch == CHAR_BLANK) {
			amtToScroll = ( aWin->theWindow->portRect.bottom
						  - aWin->theWindow->portRect.top ) / aWin->charHeight;
			Down(aWin->scrollBar, inPageDown);
			return 0;
		}
		if (ch == CHAR_CR || ch == CHAR_LF) {
			amtToScroll = 1;
			Down(aWin->scrollBar, inDownButton);
			return 0;
		}
	}
	return ch;
}


int
mac_doprev_message(void)
{
	if (WIN_MESSAGE) {
		display_nhwindow(WIN_MESSAGE, FALSE);
		amtToScroll = 1;
		winToScroll = &theWindows[WIN_MESSAGE];
		SetPort(winToScroll->theWindow);
		Up(winToScroll->scrollBar, inUpButton);
	}	
	return 0 ;
}


static void
macClickMenu ( EventRecord * theEvent , WindowPtr theWindow )
{
	Point p ;
	short hiRow = -1 , loRow = -1 ;
	Rect r ;
	NhWindow * aWin = GetNhWin ( theWindow ) ;

	r = theWindow -> portRect ;
	if ( aWin -> scrollBar && ( * aWin -> scrollBar ) -> contrlVis ) {
		short code ;
		Point p = theEvent -> where ;
		ControlHandle theBar ;

		r . right -= SBARWIDTH ;
		GlobalToLocal ( & p ) ;
		code = FindControl ( p , theWindow , & theBar ) ;
		if ( code ) {
			DoScrollBar ( p , code , theBar , aWin , theWindow ) ;
			return ;
		}
		if ( p . h >= r . right )
			return ;
	}
	r . top = r . bottom = 0 ;
	if ( inSelect != WIN_ERR ) {
		/* As an expansion, we could select all lines between the start */
		/* (button down) and end (button up) events. */
		do {
			SystemTask ( ) ;
			GetMouse ( & p ) ;
			if ( p . h < theWindow -> portRect . left || p . h > theWindow ->
				portRect . right || p . v < 0 || p . v > theWindow -> portRect .
				bottom ) {
				hiRow = -1 ;
			} else {
				hiRow = p . v / aWin -> charHeight ;
				if ( hiRow >= aWin -> lin )
					hiRow = -1 ;
			}
			if ( hiRow != loRow ) {
				/* deselect old */
				if ( loRow > -1 && aWin -> itemChars [ loRow + aWin ->
					scrollPos ] ) {
					r . top = loRow * aWin -> charHeight ;
					r . bottom = ( loRow + 1 ) * aWin -> charHeight ;
					InvertRect ( & r ) ;
				}
				loRow = hiRow ;
				/* select new */
				if ( loRow > -1 && aWin -> itemChars [ loRow + aWin ->
					scrollPos ] ) {
					r . top = loRow * aWin -> charHeight ;
					r . bottom = ( loRow + 1 ) * aWin -> charHeight ;
					InvertRect ( & r ) ;
				}
			}
		} while ( StillDown ( ) ) ;
		if ( loRow > -1 && aWin -> itemChars [ loRow + aWin -> scrollPos ] ) {
			aWin -> itemSelected [ loRow + aWin -> scrollPos ] = 
				!aWin -> itemSelected [ loRow + aWin -> scrollPos ] ;
			if ( aWin -> how != PICK_ANY )	/* pick one or pick none */
				AddToKeyQueue( CHAR_CR , 1 ) ;
		}
	}
}


static void
macClickText ( EventRecord * theEvent , WindowPtr theWindow )
{
	short hiRow = -1 , loRow = -1 ;
	Rect r ;
	NhWindow * aWin = GetNhWin ( theWindow ) ;

	r = theWindow -> portRect ;
	if ( aWin -> scrollBar && ( * aWin -> scrollBar ) -> contrlVis ) {
		short code ;
		Point p = theEvent -> where ;
		ControlHandle theBar ;

		r . right -= SBARWIDTH ;
		GlobalToLocal ( & p ) ;
		code = FindControl ( p , theWindow , & theBar ) ;
		if ( code ) {
			DoScrollBar ( p , code , theBar , aWin , theWindow ) ;
			return ;
		}
	}
}


static void
macUpdateNull ( EventRecord * theEvent , WindowPtr theWindow )
{
	if ( ! theEvent || ! theWindow ) {
		Debugger ( ) ;
	}
}


static void
draw_growicon_vert_only(WindowPtr wind) {
	GrafPtr org_port;
	RgnHandle org_clip = NewRgn();
	Rect r = wind->portRect;
	r.left = r.right - SBARWIDTH;

	GetPort(&org_port);
	SetPort(wind);
	GetClip(org_clip);
	ClipRect(&r);
	DrawGrowIcon(wind);
	SetClip(org_clip);
	DisposeRgn(org_clip);
	SetPort(org_port);
}


static void
macUpdateMessage ( EventRecord * theEvent , WindowPtr theWindow )
{
	RgnHandle org_clip = NewRgn(), clip = NewRgn();
	Rect r = theWindow -> portRect ;
	NhWindow * aWin = GetNhWin ( theWindow ) ;
	int l ;

	if ( ! theEvent ) {
		Debugger ( ) ;
	}

	GetClip(org_clip);

	DrawControls(theWindow);
	DrawGrowIcon(theWindow);

	l = 0;
	while (topl_resp[l]) {
		StringPtr name;
		unsigned char tmp[2];
		FontInfo font;
		Rect frame;
		topl_resp_rect(l, &frame);
		switch (topl_resp[l]) {
			case 'y':
				name = "\pyes";
				break;
			case 'n':
				name = "\pno";
				break;
			case 'N':
				name = "\pNone";
				break;
			case 'a':
				name = "\pall";
				break;
			case 'q':
				name = "\pquit";
				break;
			case CHAR_ANY:
				name = "\pany key";
				break;
			default:
				tmp[0] = 1;
				tmp[1] = topl_resp[l];
				name = tmp;
				break;
		}
		TextFont(geneva);
		TextSize(9);
		GetFontInfo(&font);
		MoveTo((frame.left + frame.right  - StringWidth(name)) / 2,
			   (frame.top  + frame.bottom + font.ascent-font.descent-font.leading-1) / 2);
		DrawString(name);
		PenNormal();
		if (l == topl_def_idx)
			PenSize(2, 2);
		FrameRoundRect(&frame, 4, 4);
		++l;
	}

	r . right -= SBARWIDTH + 2;
	r . bottom -= SBARHEIGHT + 1;
	/* Clip to the portrect - scrollbar/growicon *before* adjusting the rect
		to be larger than the size of the window (!) */
	RectRgn(clip, &r);
	SectRgn(clip, org_clip, clip);
	if ( r . right < MIN_RIGHT )
		r . right = MIN_RIGHT ;
	r . top -= aWin -> scrollPos * aWin -> charHeight ;

#if 0
	/* If you enable this band of code (and disable the next band), you will get
	   fewer flickers but a slower performance while drawing the dot line. */
	{	RgnHandle dotl_rgn = NewRgn();
		Rect dotl;
		dotl.left	= r.left;
		dotl.right	= r.right;
		dotl.bottom = r.top + aWin->save_lin * aWin->charHeight;
		dotl.top	= dotl.bottom - 1;
		FillRect(&dotl, &qd.gray);
		RectRgn(dotl_rgn, &dotl);
		DiffRgn(clip, dotl_rgn, clip);
		DisposeRgn(dotl_rgn);
		SetClip(clip);
	}
#endif

	if (in_topl_mode()) {
		RgnHandle topl_rgn = NewRgn();
		Rect topl_r = r;
		topl_r.top += (aWin->lin - 1) * aWin->charHeight;
		l = (*top_line)->destRect.right - (*top_line)->destRect.left;
		(*top_line)->viewRect = topl_r;
		(*top_line)->destRect = topl_r;
		if (l != topl_r.right - topl_r.left)
			TECalText(top_line);
		TEUpdate(&topl_r, top_line);
		RectRgn(topl_rgn, &topl_r);
		DiffRgn(clip, topl_rgn, clip);
		DisposeRgn(topl_rgn);
		SetClip(clip);
	}

	DisposeRgn(clip);

	TextFont ( aWin -> fontNum ) ;
	TextSize ( aWin -> fontSize ) ;
	HLock ( aWin -> windowText ) ;
	TextBox ( * aWin -> windowText , aWin -> windowTextLen , & r , teJustLeft ) ;
	HUnlock ( aWin -> windowText ) ;

#if 1
	r.bottom = r.top + aWin->save_lin * aWin->charHeight;
	r.top	 = r.bottom - 1;
	FillRect(&r, (void *) &qd.gray);
#endif

	SetClip(org_clip);
	DisposeRgn(org_clip);
}


static void
macUpdateTerm ( EventRecord * theEvent , WindowPtr theWindow )
{
#if defined(applec)
# pragma unused(theEvent)
#endif
	tty_environment_changed ( theWindow ) ;
	image_tty(theWindow);
}


static void
macUpdateMenu ( EventRecord * theEvent , WindowPtr theWindow )
{
	Rect r = theWindow -> portRect ;
	Rect r2 = r ;
	NhWindow * aWin = GetNhWin ( theWindow ) ;
	RgnHandle h ;
	Boolean vis ;

	if ( ! theEvent ) {
		Debugger ( ) ;
	}

	draw_growicon_vert_only(theWindow);

	r2 . left = r2 . right - SBARWIDTH ;
	r2 . right += 1 ;
	r2 . top -= 1 ;
	vis = ( r2 . bottom > r2 . top + 50 ) ;
	DrawControls ( theWindow ) ;

	h = (RgnHandle) 0 ;
	if ( vis && ( h = NewRgn ( ) ) ) {
		RgnHandle tmp = NewRgn ( ) ;
		if ( ! tmp ) {
			DisposeRgn ( h ) ;
			h = (RgnHandle) 0 ;
		} else {
			GetClip ( h ) ;
			RectRgn ( tmp , & r2 ) ;
			DiffRgn ( h , tmp , tmp ) ;
			SetClip ( tmp ) ;
			DisposeRgn ( tmp ) ;
		}
	}
	if ( r . right < MIN_RIGHT )
		r . right = MIN_RIGHT ;
	r . top -= aWin -> scrollPos * aWin -> charHeight ;
	HLock ( aWin -> windowText ) ;
	TextBox ( * aWin -> windowText , aWin -> windowTextLen , & r , teJustLeft ) ;
	HUnlock ( aWin -> windowText ) ;
	if ( h ) {
		SetClip ( h ) ;
		DisposeRgn ( h ) ;
	}
}


static void
macUpdateText ( EventRecord * theEvent , WindowPtr theWindow )
{
	Rect r = theWindow -> portRect ;
	Rect r2 = r ;
	NhWindow * aWin = GetNhWin ( theWindow ) ;
	RgnHandle h ;
	Boolean vis ;

	if ( ! theEvent ) {
		Debugger ( ) ;
	}

	r2 . left = r2 . right - SBARWIDTH ;
	r2 . right += 1 ;
	r2 . top -= 1 ;
	vis = ( r2 . bottom > r2 . top + 50 ) ;
	DrawControls ( theWindow ) ;

	h = (RgnHandle) 0 ;
	if ( vis && ( h = NewRgn ( ) ) ) {
		RgnHandle tmp = NewRgn ( ) ;
		if ( ! tmp ) {
			DisposeRgn ( h ) ;
			h = (RgnHandle) 0 ;
		} else {
			GetClip ( h ) ;
			RectRgn ( tmp , & r2 ) ;
			DiffRgn ( h , tmp , tmp ) ;
			SetClip ( tmp ) ;
			DisposeRgn ( tmp ) ;
		}
	}
	if ( r . right < MIN_RIGHT )
		r . right = MIN_RIGHT ;
	r . top -= aWin -> scrollPos * aWin -> charHeight ;
	r . right -= SBARWIDTH;
	HLock ( aWin -> windowText ) ;
	TextBox ( * aWin -> windowText , aWin -> windowTextLen , & r , teJustLeft ) ;
	HUnlock ( aWin -> windowText ) ;
	draw_growicon_vert_only(theWindow);
	if ( h ) {
		SetClip ( h ) ;
		DisposeRgn ( h ) ;
	}
}


static void
macCursorNull ( EventRecord * theEvent , WindowPtr theWindow , RgnHandle mouseRgn )
{
#if defined(applec)
# pragma unused(theWindow)
#endif
	Rect r = { -1 , -1 , 2 , 2 } ;

	InitCursor ( ) ;
	OffsetRect ( & r , theEvent -> where . h , theEvent -> where . v ) ;
	RectRgn ( mouseRgn , & r ) ;
}


static void
macCursorMessage ( EventRecord * theEvent , WindowPtr theWindow , RgnHandle mouseRgn )
{
#if defined(applec)
# pragma unused(theWindow)
#endif
	Rect r = { -1 , -1 , 2 , 2 } ;

	InitCursor ( ) ;
	OffsetRect ( & r , theEvent -> where . h , theEvent -> where . v ) ;
	RectRgn ( mouseRgn , & r ) ;
}


static void
macCursorTerm ( EventRecord * theEvent , WindowPtr theWindow , RgnHandle mouseRgn )
{
	Point where ;
	char * dir_bas , * dir ;
	CursHandle ch ;
	GrafPtr gp ;
	NhWindow * nhw = GetNhWin ( theWindow ) ;
	Rect r = { 0 , 0 , 1 , 1 } ;

	GetPort ( & gp ) ;
	SetPort ( theWindow ) ;

	where = theEvent -> where ;
	GlobalToLocal ( & where ) ;

	if ( cursor_locked )
		dir = (char *)0 ;
	else {
		dir_bas = flags . num_pad ? (char *) ndir : (char *) sdir ;
		dir = strchr ( dir_bas , click_to_cmd ( where . h / nhw -> charWidth + 1 ,
												where . v / nhw -> charHeight ,
												CLICK_1 ) ) ;
	}
	ch = GetCursor ( dir ? dir - dir_bas + 513 : 512 ) ;
	if ( ch ) {

		HLock ( ( Handle ) ch ) ;
		SetCursor ( * ch ) ;
		HUnlock ( ( Handle ) ch ) ;

	} else {

		InitCursor ( ) ;
	}
	OffsetRect ( & r , theEvent -> where . h , theEvent -> where . v ) ;
	RectRgn ( mouseRgn , & r ) ;

	SetPort ( gp ) ;
}


static void
macCursorMenu ( EventRecord * theEvent , WindowPtr theWindow , RgnHandle mouseRgn )
{
#if defined(applec)
# pragma unused(theWindow)
#endif
	Rect r = { -1 , -1 , 2 , 2 } ;

	InitCursor ( ) ;
	OffsetRect ( & r , theEvent -> where . h , theEvent -> where . v ) ;
	RectRgn ( mouseRgn , & r ) ;
}


static void
macCursorText ( EventRecord * theEvent , WindowPtr theWindow , RgnHandle mouseRgn )
{
#if defined(applec)
# pragma unused(theWindow)
#endif
	Rect r = { -1 , -1 , 2 , 2 } ;

	InitCursor ( ) ;
	OffsetRect ( & r , theEvent -> where . h , theEvent -> where . v ) ;
	RectRgn ( mouseRgn , & r ) ;
}


void
UpdateMenus ( void )
{
	AdjustMenus(0);
}


void
DoMenu ( long choice )
{
	DoMenuEvt(choice);
}


void
HandleKey ( EventRecord * theEvent )
{
	WindowPtr theWindow = FrontWindow ( ) ;

	if ( theEvent -> modifiers & cmdKey ) {
		if ( theEvent -> message & 0xff == '.' ) {
			int i ;
/* Flush key queue */
			for ( i = 0 ; i < QUEUE_LEN ; i ++ ) {
				keyQueue [ i ] = 0 ;
			}
			theEvent -> message = '\033' ;
			goto dispatchKey ;
		} else {
			UpdateMenus ( ) ;
			DoMenu ( MenuKey ( theEvent -> message & 0xff ) ) ;
		}
	} else {

dispatchKey :
		if ( theWindow ) {
			GetNhWin ( theWindow ) -> keyFunc ( theEvent , theWindow ) ;
		} else {
			GeneralKey ( theEvent , (WindowPtr) 0 ) ;
		}
	}
}


void
HandleClick ( EventRecord * theEvent )
{
	int code ;
	unsigned long l ;
	WindowPtr theWindow ;
	Rect r = ( * GetGrayRgn ( ) ) -> rgnBBox ;
	NhWindow * aWin ;

	InsetRect ( & r , 4 , 4 ) ;

	code = FindWindow ( theEvent -> where , & theWindow ) ;
	aWin = GetNhWin ( theWindow ) ;

	if ( code != inContent ) {
		InitCursor ( ) ;
	} else {
		aWin -> cursorFunc ( theEvent , theWindow , gMouseRgn ) ; /* Correct direction */
	}
	switch ( code ) {

	case inContent :
		if ( inSelect == WIN_ERR || aWin - theWindows == inSelect ) {
			SelectWindow ( theWindow ) ;
			SetPort ( theWindow ) ;
			GetNhWin ( theWindow ) -> clickFunc ( theEvent , theWindow ) ;
		} else {
			nhbell ( ) ;
		}
		break ;

	case inDrag :
		if ( inSelect == WIN_ERR || aWin - theWindows == inSelect ) {
			InitCursor ( ) ;
			DragWindow ( theWindow , theEvent -> where , & r ) ;
			SaveWindowPos ( theWindow ) ;
		} else {
			nhbell ( ) ;
		}
		break ;

	case inGrow :
		if ( inSelect == WIN_ERR || aWin - theWindows == inSelect ) {
			InitCursor ( ) ;
			SetRect ( & r , 80 , 2 * aWin -> charHeight + 1 , r . right ,
				r . bottom ) ;
			if ( aWin == theWindows + WIN_MESSAGE )
				r . top += SBARHEIGHT + 1 ;
			l = GrowWindow ( theWindow , theEvent -> where , & r ) ;
			SizeWindow ( theWindow , l & 0xffff , l >> 16 , FALSE ) ;
			SaveWindowSize ( theWindow ) ;
			SetPort ( theWindow ) ;
			InvalRect ( & ( theWindow -> portRect ) ) ;
			if ( aWin -> scrollBar ) {
				InvalScrollBar ( aWin ) ;
			}
		} else {
			nhbell ( ) ;
		}
		break ;

	case inGoAway :
		WindowGoAway(theEvent, theWindow);
		break ;

	case inMenuBar :
		UpdateMenus ( ) ;
		DoMenu ( MenuSelect ( theEvent -> where ) ) ;
		break ;

	case inSysWindow :
		SystemClick( theEvent, theWindow ) ;
		break ;

	default :
		break ;
	}
}


void
HandleUpdate ( EventRecord * theEvent )
{
	WindowPtr theWindow = ( WindowPtr ) theEvent -> message ;

	BeginUpdate ( theWindow ) ;
	SetPort ( theWindow ) ;
	EraseRect ( & ( theWindow -> portRect ) ) ;
	GetNhWin ( theWindow ) -> updateFunc ( theEvent , theWindow ) ;
	EndUpdate ( theWindow ) ;
}


static void
DoOsEvt ( EventRecord * theEvent )
{
	WindowPtr wp ;
	short code ;

	if ( ( theEvent -> message & 0xff000000 ) == 0xfa000000 ) { /* Mouse Moved */

		code = FindWindow ( theEvent -> where , & wp ) ;
		if ( code != inContent ) {

			Rect r = { -1 , -1 , 2 , 2 } ;

			InitCursor ( ) ;
			OffsetRect ( & r , theEvent -> where . h , theEvent -> where . v ) ;
			RectRgn ( gMouseRgn , & r ) ;

		} else {

			NhWindow * nhw = GetNhWin ( wp ) ;
			if ( nhw ) {
				nhw -> cursorFunc ( theEvent , wp , gMouseRgn ) ;
			}
		}
	} else {

		/* Suspend/resume */
		if ( ( ( theEvent -> message & osEvtMessageMask) >> 24 ) == suspendResumeMessage )
			kApplicInFront = ( theEvent -> message & resumeFlag );
	}
}


void
HandleEvent ( EventRecord * theEvent )
{
	switch ( theEvent -> what ) {
	case autoKey :
	case keyDown :
		HandleKey ( theEvent ) ;
		break ;
	case updateEvt :
		HandleUpdate ( theEvent ) ;
		break ;
	case mouseDown :
		HandleClick ( theEvent ) ;
		break ;
	case diskEvt :
		if ( ( theEvent -> message & 0xffff0000 ) != 0 ) {

			Point p = { 150 , 150 } ;
			( void ) DIBadMount ( p , theEvent -> message ) ;
		}
		break ;
	case osEvt :
		DoOsEvt ( theEvent ) ;
		break ;
	default :
		if ( multi < 0 ) { // Get frozen, so stop repeating
			int ix;
			for ( ix = 0 ; ix < QUEUE_LEN ; ix ++ ) {
				keyQueue [ ix ] = 0 ;
			}
			FlushEvents ( keyDownMask , 0 ) ;
		}
		break ;
	}
}


long doDawdle = 0L ;

void
WindowGoAway ( EventRecord * theEvent, WindowPtr theWindow )
{
	NhWindow * aWin = GetNhWin(theWindow);

	if ( ! theEvent || TrackGoAway ( theWindow , theEvent -> where ) ) {
		if ( aWin - theWindows == BASE_WINDOW && ! flags . window_inited ) {
			AddToKeyQueue ( '\033' , 1 ) ;
			return ;
		} else {
			HideWindow ( theWindow ) ;
		}
		if ( inSelect == WIN_ERR || aWin - theWindows != inSelect ) {
			destroy_nhwindow ( aWin - theWindows ) ;
		} else {
			AddToKeyQueue ( CHAR_CR , 1 ) ;
		}
	}
}

void
DimMenuBar ( void )
{
	AdjustMenus(1);
}


void
UndimMenuBar ( void )
{
	AdjustMenus(0);
}

static int mBarDimmed = 0 ;

void
mac_get_nh_event( void )
{
	EventRecord anEvent ;

	if ( ( inSelect == WIN_ERR && flags . window_inited && ! in_topl_mode ( ) )
		 == mBarDimmed )
		if ( mBarDimmed ) {
			UndimMenuBar ( ) ;
			mBarDimmed = 0 ;
		} else {
			DimMenuBar ( ) ;
			mBarDimmed = 1 ;
		}
	/*
	 * We want to take care of keys in the buffer as fast as
	 * possible
	 */
	if ( keyQueue [ 0 ] ) {
		doDawdle = 0L ;
	}
	if ( asyDSC ) {
		SetPort ( asyDSC -> theWindow ) ;
		DrawScrollbar ( asyDSC , asyDSC -> theWindow ) ;
	}
	if ( kApplicInFront ) {
		AdjustMenus ( 0 ) ;
	}
	(void) WaitNextEvent ( -1 , & anEvent , doDawdle , gMouseRgn ) ;
	doDawdle = 0L ;
#if 0
	/* I don't want to make the tty cursor blinking. */
	if ( handle_tty_event ( _mt_window , & anEvent ) )	
#endif
		HandleEvent ( & anEvent ) ;

	if (top_line && theWindows) {
		WindowPeek win = (WindowPeek)theWindows[WIN_MESSAGE].theWindow;
		if (win && win->visible)
			TEIdle(top_line);
	}
}


int
GetFromKeyQueue ( void ) {
int ix , ret ;

	ret = keyQueue [ 0 ] ;
	for ( ix = 0 ; ix < QUEUE_LEN - 1 ; ix ++ ) {
		keyQueue [ ix ] = keyQueue [ ix + 1 ] ;
	}
	keyQueue [ ix ] = 0 ;

	return ret ;
}


int
mac_nhgetch( void ) {
int ch ;
NhWindow * nhw = flags . window_inited ? theWindows + WIN_MAP : (NhWindow *) nil ;

	if ( theWindows ) {
	NhWindow * aWin = theWindows + WIN_MESSAGE ;

		if ( WIN_MESSAGE > 0 ) {
			aWin -> last_more_lin = aWin -> lin ;
		}
	}

	wait_synch ( ) ;

	if ( ! keyQueue [ 0 ] ) {
		long total , contig ;
		static char warn = 0 ;

		PurgeSpace ( & total , & contig ) ;
		if ( contig < 25000L || total < 50000L ) {
			if ( ! warn ) {
				pline ( "Low Memory!" ) ;
				warn = 1 ;
			}
		} else {
			warn = 0 ;
		}
	}

	do {
		doDawdle = ( in_topl_mode() ? GetCaretTime ( ) : 120L ) ;

		if ( nhw ) {
			SetPort ( nhw -> theWindow ) ;
			if ( WaitMouseUp ( ) ) {
				unsigned long tick = TickCount ( ) ;

				if ( tick >= gNextClickRepeat ) {
					Point where ;

					GetMouse ( & where ) ;
					SetPt ( & clicked_pos , where . h / nhw -> charWidth ,
											where . v / nhw -> charHeight ) ;
					gClickedToMove = TRUE ;
					gNextClickRepeat = tick + LMGetKeyRepThresh();
				}
				if ( doDawdle > LMGetKeyRepThresh() ) {
					doDawdle = LMGetKeyRepThresh() ;
				}
			} else {
				gNextClickRepeat = 0xffffffff ;
			}
		}

		get_nh_event ( ) ;
		ch = GetFromKeyQueue ( ) ;
	} while ( ! ch && ! gClickedToMove ) ;

	if ( ! gClickedToMove ) {
		ObscureCursor ( ) ;
	} else {
		gClickedToMove = 0 ;
	}

#ifdef THINK_C
	if (ch == '\r') ch = '\n';
#endif

	return ch ;
}


void
mac_delay_output( void )
{
	long destTicks = TickCount ( ) + 1 ;

	while ( TickCount ( ) < destTicks ) {
		wait_synch ( ) ;
	}
}


void
mac_wait_synch( void )
{
	get_nh_event ( ) ;
}


void
mac_mark_synch( void )
{
	get_nh_event ( ) ;
}


static void
mac_cliparound ( int x , int y )
{
	/* TODO */
	if ( ! ( x * ( y + 1 ) ) ) {
		Debugger ( ) ;
	}
}


void
mac_raw_print ( const char * str )
{
	tty_raw_print(str);
}


void
mac_raw_print_bold ( const char * str )
{
	tty_raw_print_bold(str);
}


void
mac_exit_nhwindows ( const char * s )
{
	clear_screen ( ) ;
	tty_exit_nhwindows ( s ) ;
	destroy_nhwindow ( WIN_MESSAGE ) ;
	destroy_nhwindow ( WIN_INVEN ) ;
}


/*
 * Don't forget to decrease in_putstr before returning...
 */
void
mac_putstr ( winid win , int attr , const char * str )
{
	long len , slen ;
	NhWindow * aWin = & theWindows [ win ] ;
	int kind ;
	static char in_putstr = 0 ;
	Rect r ;

	if ( in_putstr > 3 ) {
		/* DebugStr ( ( ConstStr255Param ) "\pRecursion!" ) ; */
		return ;
	}
	if ( win < 0 || win >= NUM_MACWINDOWS ) {
		error ( "Invalid window %d (Max %d) in putstr." , win ,
			NUM_MACWINDOWS , attr ) ;
		return ;
	}
	if ( ! aWin -> theWindow ) {
		error ( "Unallocated window %d in putstr." , win ) ;
		return ;
	}

	if (aWin->theWindow == _mt_window) {
		tty_putstr(win, attr, str);
		return;
	}

	in_putstr ++ ;
	kind = ( ( WindowPeek ) ( aWin -> theWindow ) ) -> windowKind -
		WIN_BASE_KIND ;
	slen = strlen ( str ) ;

	{
		char * sr , * ds ;

		r = aWin->theWindow->portRect;
		if (win && win == WIN_MESSAGE) {
			r.right  -= SBARWIDTH  + 1;
			r.bottom -= SBARHEIGHT + 1;
			if ( aWin->last_more_lin < aWin->scrollPos )
				aWin->last_more_lin = aWin->scrollPos;
			if ( flags.page_wait && aWin->last_more_lin <=
				 aWin->lin - (r.bottom - r.top) / aWin->charHeight )
				display_nhwindow(win, TRUE);
		}

		/*
		 * A "default" text window - uses TextBox
		 * We just add the text, without attributes for now
		 */
		len = GetHandleSize ( aWin -> windowText ) ;
		while ( aWin -> windowTextLen + slen + 1 > len ) {
			len = ( len > 2048 ) ? ( len + 2048 ) : ( len * 2 ) ;
			SetHandleSize ( aWin -> windowText , len ) ;
			if ( MemError ( ) ) {
				error ( "SetHandleSize (putstr)" ) ;
				aWin -> windowTextLen = 0L ;
				aWin -> save_lin = 0 ;
				aWin -> lin = 0 ;
			}
		}
	
		len = aWin -> windowTextLen ;
		sr = (char *)str ;
		ds = * ( aWin -> windowText ) + len ;
		while ( * sr ) {
			if ( * sr == CHAR_LF )
				* ds = CHAR_CR ;
			else
				* ds = * sr ;
			if ( * ds == CHAR_CR && kind != NHW_MENU ) {
				aWin -> lin ++ ;
			}
			sr ++ ;
			ds ++ ;
		}

		( * ( aWin -> windowText ) ) [ len + slen ] = CHAR_CR ;
		aWin -> windowTextLen += slen + 1 ;
		aWin -> lin ++ ;
	
		SetPort ( aWin -> theWindow ) ;
		InvalRect ( & r ) ;
		aWin -> clear = 0 ;
		if ( kind == NHW_MESSAGE ) {
			short min = aWin->lin - (r.bottom - r.top) / aWin->charHeight;
			if (aWin->scrollPos < min) {
				aWin->scrollPos = min;
				SetCtlMax  (aWin->scrollBar, aWin->lin);
				SetCtlValue(aWin->scrollBar, min);
			}
		}
		if ( aWin -> scrollBar ) {
			InvalScrollBar ( aWin ) ;
		}
	}
	in_putstr -- ;
}


void
mac_curs ( winid win , int x , int y )
{
	NhWindow * aWin = & theWindows [ win ] ;
	int kind = ( ( WindowPeek ) ( aWin -> theWindow ) ) -> windowKind -
		WIN_BASE_KIND ;

	if (aWin->theWindow == _mt_window) {
		tty_curs(win, x, y);
		return;
	}

	SetPort  ( aWin -> theWindow ) ;
	MoveTo ( x * aWin -> charWidth , ( y + 1 ) * aWin -> charHeight -
		aWin -> leading ) ;
	SetPt ( & ( aWin -> cursor ) , x , y ) ;
}


void
mac_print_glyph ( winid window , XCHAR_P x , XCHAR_P y , int glyph )
{
	if (theWindows[window].theWindow != _mt_window)
		Debugger();
	tty_print_glyph(window, x, y, glyph);
}


int
mac_nh_poskey ( int * a , int * b , int * c )
{
	int ch = nhgetch();
	*a = clicked_pos.h;
	*b = clicked_pos.v;
	*c = clicked_mod;
	return ch;
}


void
mac_start_menu ( winid win )
{
	HideWindow ( theWindows [ win ] . theWindow ) ;
	clear_nhwindow ( win ) ;
}


void
mac_add_menu ( winid win , int glyph, const anything *any , CHAR_P menuChar , int attr , const char * inStr , int preselected)
{
	long addSize ;
	int newWid ;
	NhWindow * aWin = & theWindows [ win ] ;
	const char *str;
	char locStr[BUFSZ];

	if ( ! inStr || aWin -> lin >= NUM_MENU_ITEMS )
		return ;

	SetPort ( aWin -> theWindow ) ;
	if ( any->a_void != 0 && menuChar == 0 ) {
		menuChar = aWin->menuChar++;
		if (menuChar == 'z')
			aWin->menuChar = 'A';
	}
	
	if ( any->a_void != 0 ) {
		Sprintf(locStr, "%c - %s", menuChar, inStr);
		str = locStr;
	} else
		str = inStr;

	aWin -> itemChars [ aWin -> lin ] = menuChar ;
/*	aWin -> itemSelected [ aWin -> lin ] = preselected; */
	aWin -> itemSelected [ aWin -> lin ] = FALSE ;
	aWin -> itemIdentifiers [ aWin -> lin ] = *any ;

	addSize = strlen ( str ) ;
	newWid = TextWidth ( str , 0 , addSize ) ;
	if ( newWid > aWin -> wid )
		aWin -> wid = newWid ;
	putstr ( win , attr , str ) ;
}


/*
 * End a menu in this window, window must a type NHW_MENU.
 * str is a list of cancel characters (values that may be input)
 * morestr is a prompt to display, rather than the default.
 * str and morestr might be ignored by some ports.
 */
void
mac_end_menu ( winid win , const char * morestr )
{
	unsigned char buf [ 256 ] ;
	int len ;
	NhWindow * aWin = & theWindows [ win ] ;

	buf [ 0 ] = 0 ;
	if ( morestr ) {
		strncpy ( (char *)& buf [ 1 ] , morestr , 255 ) ;
		len = strlen ( morestr ) ;
		if ( len > 255 )
			buf [ 0 ] = 255 ;
		else
			buf [ 0 ] = len ;
	}
	SetWTitle ( aWin -> theWindow , buf ) ;
}


int
mac_select_menu ( winid win , int how , menu_item **selected_list )
{
	int c , l , n;
	NhWindow * aWin = & theWindows [ win ] ;
	WindowPtr theWin = aWin -> theWindow ;
	Boolean done;

	inSelect = win ;

	adjust_window_pos ( aWin , theWin , aWin -> wid + ( SBARWIDTH + 2 ) ) ;
	SetPort ( theWin ) ;
	if ( aWin -> scrollBar ) {
		InvalScrollBar ( aWin ) ;
	}
	SelectWindow ( theWin ) ;
	ShowWindow ( theWin ) ;
	InvalRect ( & ( theWin -> portRect ) ) ;

	aWin->how = (short) how;
	done = FALSE;
	do {
		c = nhgetch();
		if (c == CHAR_ESC) {
			/* deselect everything */
			for ( n = l = 0 ; l < aWin->lin ; l++ )
				aWin->itemSelected[l] = FALSE;
			done = TRUE;
		} else if (ClosingWindowChar(c)) {
			done = TRUE;
		} else {
			for (l = 0; l < aWin->lin; l++) {
				if ( aWin -> itemChars [ l ] == c ) {
					aWin->itemSelected[l] = !aWin->itemSelected[l];
					if (how == PICK_ONE) done = TRUE;
					break;
				}
			}
		}
	} while ( ! done ) ;

	HideWindow ( theWin ) ;

	/* count # selected */
	for ( n = l = 0 ; l < aWin->lin ; l++ )
		if (aWin->itemSelected[l]) n++;

	if (n) {
		menu_item *mp;
		*selected_list = mp = (menu_item *) alloc(n * sizeof(menu_item));
		for (l = 0; l < aWin->lin; l++ )
			if ( aWin->itemSelected[l]) {
				mp->item = aWin->itemIdentifiers[l];
				mp->count = -1L;
				mp++;
			}
	} else
		*selected_list = 0;

	inSelect = WIN_ERR ;

	return n;
}


void
mac_display_file ( name, complain )
const char * name;
boolean	complain;
{
	long l ;
	short refNum ;
	Ptr buf ;
	int win ;

	if ( 0 > ( refNum = macopen ( name , O_RDONLY , TEXT_TYPE ) ) ) {
		if (complain) error ( "Cannot open file %s." , name ) ;
	} else {
		l = macseek ( refNum , 0 , SEEK_END ) ;
		( void ) macseek ( refNum , 0 , 0L ) ;
		win = create_nhwindow ( NHW_TEXT ) ;
		if ( win == WIN_ERR ) {
			if (complain) error ( "Cannot make window." ) ;
			goto out ;
		}
		buf = NewPtr(l+1);
		if (buf)
			l = macread(refNum, buf, l);
		if (buf && l > 0) {
			buf[l] = '\0';
			putstr(win, 0, buf);
			display_nhwindow(win, FALSE);
		} else {
			HideWindow(theWindows[win].theWindow);
			destroy_nhwindow(win);
		}
		if (buf)
			DisposPtr(buf);
out :
		macclose ( refNum ) ;
	}
}


void
port_help ( )
{
	display_file ( PORT_HELP , TRUE ) ;
}


short frame_corner ;


static pascal void
FrameItem ( DialogPtr dlog , short item )
{
	short k ;
	Handle h ;
	Rect r ;

	GetDItem ( dlog , item , & k , & h , & r ) ;
	PenSize ( 3 , 3 ) ;
	FrameRoundRect ( & r , frame_corner , frame_corner ) ;
	PenNormal ( ) ;
}


void
SetFrameItem ( DialogPtr dlog , short frame , short item )
{
	Rect r , r2 ;
	short kind ;
	Handle h ;

	GetDItem ( dlog , item , & kind , & h , & r ) ;
	InsetRect ( & r , -4 , -4 ) ;
	r2 = r ;
	GetDItem ( dlog , frame , & kind , & h , & r ) ;
	SetDItem ( dlog , frame , kind , ( Handle ) FrameItem , & r2 ) ;
	frame_corner = 16 ;
}


//	Flash a button (for instance OK if you press enter)
//
void
FlashButton ( DialogPtr dlog , short item )
{
	short k ;
	Handle h ;
	Rect r ;
	long l ;

	GetDItem ( dlog , item , & k , & h , &  r ) ;
	if ( k == ctrlItem + btnCtrl ) {
		HiliteControl ( ( ControlHandle ) h , 1 ) ;
		Delay ( GetDblTime ( ) / 2 , & l ) ;
		HiliteControl ( ( ControlHandle ) h , 0 ) ;
	}
}

#if 0		/* I can't find anything that uses this function */
pascal Boolean
CharacterDialogFilter ( DialogPtr dp , EventRecord * ev , short * item )
{
	int ix ;
	Handle h ;
	Rect r ;
	short k ;
	Str255 s ;
	unsigned char com [ 2 ] ;

	if ( ev -> what == mouseDown ) {

		int code ;
		WindowPtr wp ;
		Rect r ;

		code = FindWindow ( ev -> where , & wp ) ;
		if ( wp != dp || code != inDrag ) {
	
			return 0 ;
		}
		r = ( * GetGrayRgn ( ) ) -> rgnBBox ;
		InsetRect ( & r , 3 , 3 ) ;
	
		DragWindow ( wp , ev -> where , & r ) ;
		SaveWindowPos ( wp ) ;

		ev -> what = nullEvent ;
		return 1 ;
	}
	if ( ev -> what != keyDown ) {

		return 0 ;
	}
	com [ 0 ] = 1 ;
	com [ 1 ] = ev -> message & 0xff ;

	if ( com [ 1 ] == 10 || com [ 1 ] == 13 || com [ 1 ] == 32 ||
		com [ 1 ] == 3 ) { // various "OK"

		* item = 1 ;
		FlashButton ( dp , 1 ) ;
		return 1 ;
	}
	if ( com [ 1 ] == 'Q' || com [ 1 ] == 'q' ||
		 com [ 1 ] == 27 || ( ev -> message & 0xff00 == 0x3500 ) ) { // escape

		* item = 2 ;
		FlashButton ( dp , 2 ) ;
		return 1 ;
	}
	for ( ix = 3 ; ix ; ix ++ ) {

		h = ( Handle ) 0 ;
		k = 0 ;
		GetDItem ( dp , ix , & k , & h , & r ) ;
		if ( ! k || ! h ) {

			return 0 ;
		}
		if ( k == 6 ) { // Radio Button Item

			GetCTitle ( ( ControlHandle ) h , s ) ;
			s [ 0 ] = 1 ;
			if ( ! IUEqualString ( com , s ) ) {

				* item = ix ;
				return 1 ;
			}
		}
	}
/*NOTREACHED*/
	return 0 ;
}
#endif /* 0 */


void
mac_player_selection ( void )
{
/* see macmain.c:mac_askname() and macmenu.c:DialogAskName() */
}

void
mac_update_inventory ( void )
{
}


void
mac_suspend_nhwindows ( const char * foo)
{
	/*	Can't relly do that :-)		*/
}


void
mac_resume_nhwindows ( void )
{
	/*	Can't relly do that :-)		*/
}


int
try_key_queue ( char * bufp )
{
	if ( keyQueue [ 0 ] ) {

		int ix , flag = 0 ;
		for ( ix = 0 ; ix < QUEUE_LEN ; ix ++ ) {

			if ( ! flag ) {

				if ( ! ( bufp [ ix ] = keyQueue [ ix ] ) ) {

					flag = 1 ;
				}
			}
			keyQueue [ ix ] = 0 ;
		}
		if ( ! flag ) {

			bufp [ ix ] = 0 ;
		}
		return 1 ;
	}

	return 0 ;
}

/* Interface definition, for windows.c */
struct window_procs mac_procs = {
    "mac",
    mac_init_nhwindows,
    mac_player_selection,
    mac_askname,
    mac_get_nh_event,
    mac_exit_nhwindows,
    mac_suspend_nhwindows,
    mac_resume_nhwindows,
    mac_create_nhwindow,
    mac_clear_nhwindow,
    mac_display_nhwindow,
    mac_destroy_nhwindow,
    mac_curs,
    mac_putstr,
    mac_display_file,
    mac_start_menu,
    mac_add_menu,
    mac_end_menu,
    mac_select_menu,
    genl_message_menu,
    mac_update_inventory,
    mac_mark_synch,
    mac_wait_synch,
#ifdef CLIPPING
    mac_cliparound,
#endif
#ifdef POSITIONBAR
    donull,
#endif
    mac_print_glyph,
    mac_raw_print,
    mac_raw_print_bold,
    mac_nhgetch,
    mac_nh_poskey,
    mac_nhbell,
    mac_doprev_message,
    mac_yn_function,
    mac_getlin,
    mac_get_ext_cmd,
    mac_number_pad,
    mac_delay_output,
#ifdef CHANGE_COLOR
	(void *) donull,
	(void *) donull,
#endif
    /* other defs that really should go away (they're tty specific) */
	0,	//    mac_start_screen,
	0, //    mac_end_screen,
    genl_outrip,
} ;

/*macwin.c*/
