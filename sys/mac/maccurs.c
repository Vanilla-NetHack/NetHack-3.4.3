/*	SCCS Id: @(#)maccurs.c	3.1	93/01/24		  */
/* Copyright (c) Jon W{tte, 1992.				  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include <Folders.h>
#include <Windows.h>
#include <ToolUtils.h>
#include <Resources.h>
#include <Memory.h>
#include <Files.h>

extern void dprintf ( char * , ... ) ;

#define DIV_FACTOR 3

static Boolean winFileInit = 0 ;
static unsigned char winFileName [ 32 ] ;
static long winFileDir ;
static short winFileVol ;
static Handle winFileContents = NULL ;

typedef struct WinPosSave {
	short			validPos ;
	short			validSize ;
	short			top ;
	short			left ;
	short			height ;
	short			width ;
} WinPosSave ;

static WinPosSave savePos [ kLastWindowKind + 1 ] ;
static WinPosSave usePos [ kLastWindowKind + 1 ] ;



static void
InitWinFile ( void )
{
	StringHandle sh ;
	long len ;
	short ref = 0 ;

	if ( winFileInit ) {
		return ;
	}
/* We trust the glue. If there's an error, store in game dir. */
	if ( FindFolder ( kOnSystemDisk , kPreferencesFolderType , kCreateFolder ,
		& winFileVol , & winFileDir ) ) {
		winFileVol = 0 ;
		winFileDir = 0 ;
	}
	sh = GetString ( 128 ) ;
	if ( sh && * sh ) {
		BlockMove ( * sh , winFileName , * * sh + 1 ) ;
		ReleaseResource ( sh ) ;
	} else {
		BlockMove ( "\PNetHack Windows" , winFileName , 16 ) ;
	}
	if ( HOpen ( winFileVol , winFileDir , winFileName , fsRdPerm , & ref ) ) {
		return ;
	}
	len = sizeof ( savePos ) ;
	if ( ! FSRead ( ref , & len , savePos ) ) {
		winFileInit = 1 ;
	}
	BlockMove ( savePos , usePos , sizeof ( savePos ) ) ;
	FSClose ( ref ) ;
}


static void
FlushWinFile ( void )
{
	short ref ;
	long len ;

	if ( ! winFileInit ) {
		if ( ! winFileName [ 0 ] ) {
			return ;
		}
		HCreate ( winFileVol , winFileDir , winFileName , MAC_CREATOR , PREF_TYPE ) ;
		HCreateResFile ( winFileVol , winFileDir , winFileName ) ;
	}
	if ( HOpen ( winFileVol , winFileDir , winFileName , fsWrPerm , & ref ) ) {
		return ;
	}
	winFileInit = 1 ;
	len = sizeof ( savePos ) ;
	( void ) FSWrite ( ref , & len , savePos ) ; /* Don't care about error */
	FSClose ( ref ) ;
}


#if 0
/* Now we use click_to_cmd() instead. */

int
clickSector ( int x , int y , int toX , int toY )
{
	Boolean isDown = 0 ;
	Boolean isLeft = 0 ;
	Boolean inHoriz = 0 ;
	Boolean inVert = 0 ;

	if ( toX == x && toY == y ) {

		return 0 ; /* On the same spot */
	}

	if ( abs ( toY - y ) <= abs ( toX - x ) / DIV_FACTOR ) {

		inHoriz = 1 ;
	}
	if ( abs ( toX - x ) <= abs ( toY - y ) / DIV_FACTOR ) {

		inVert = 1 ;
	}
	if ( toY > y ) {

		isDown = 1 ;
	}
	if ( toX < x ) {

		isLeft = 1 ;
	}

	if ( inHoriz ) {

		if ( isLeft ) {

			return 7 ;
		}
		return 3 ;
	}
	if ( inVert ) {

		if ( isDown ) {

			return 5 ;
		}
		return 1 ;
	}
	if ( isDown ) {

		if ( isLeft ) {

			return 6 ;
		}
		return 4 ;

	} else {

		if ( isLeft ) {

			return 8 ;
		}
		return 2 ;
	}
}

#endif


Boolean
RetrievePosition ( short kind , short * top , short * left ) {
Point p ;

	InitWinFile ( ) ;
	if ( kind < 0 || kind > kLastWindowKind ) {
		dprintf ( "Retrieve Bad kind %d" , kind ) ;
		return 0 ;
	}
	if ( ! usePos [ kind ] . validPos ) {
		dprintf ( "Retrieve Not stored kind %d" , kind ) ;
		return 0 ;
	}
	* top = usePos [ kind ] . top ;
	* left = usePos [ kind ] . left ;
	p . h = * left ;
	p . v = * top ;
	dprintf ( "Retrieve Kind %d Point (%d,%d)" , kind , * left , * top ) ;
	return PtInRgn ( p , GetGrayRgn ( ) ) ;
}


Boolean
RetrieveSize ( short kind , short top , short left , short * height , short * width )
{
	Point p ;

	InitWinFile ( ) ;
	if ( kind < 0 || kind > kLastWindowKind ) {
		return 0 ;
	}
	if ( ! usePos [ kind ] . validSize ) {
		return 0 ;
	}
	* width = usePos [ kind ] . width ;
	* height = usePos [ kind ] . height ;
	p . h = left + * width ;
	p . v = top + * height ;
	return PtInRgn ( p , GetGrayRgn ( ) ) ;
}


void
SavePosition ( short kind , short top , short left )
{
	InitWinFile ( ) ;
	if ( kind < 0 || kind > kLastWindowKind ) {
		dprintf ( "Save bad kind %d" , kind ) ;
		return ;
	}
	savePos [ kind ] . validPos = 1 ;
	savePos [ kind ] . top = top ;
	savePos [ kind ] . left = left ;
	dprintf ( "Save kind %d point (%d,%d)" , kind , left , top ) ;
	FlushWinFile ( ) ;
}


void
SaveSize ( short kind , short height , short width )
{
	InitWinFile ( ) ;
	if ( kind < 0 || kind > kLastWindowKind ) {
		return ;
	}
	savePos [ kind ] . validSize = 1 ;
	savePos [ kind ] . width = width ;
	savePos [ kind ] . height = height ;
	FlushWinFile ( ) ;
}


static short
GetWinKind ( WindowPtr win )
{
short kind ;
NhWindow * nhw = GetNhWin ( win ) ;
char * typeStr [ ] = {
	"map" , "status" , "message" , "text" , "menu" ,
} ;

	if ( ! nhw || ( ( ( long ) nhw ) & 1 ) || nhw -> theWindow != win ) {
		return -1 ;
	}
	kind = nhw -> kind ;
	if ( kind < 0 || kind > NHW_TEXT ) {
		return -1 ;
	}
	dprintf ( "Got window kind %d (%lx)->%lx" , kind , win , nhw ) ;
	switch ( kind ) {
	case NHW_MAP :
	case NHW_STATUS :
	case NHW_BASE :
		kind = kMapWindow ;
		break ;
	case NHW_MESSAGE :
		kind = kMessageWindow ;
		break ;
	case NHW_MENU :
		kind = kMenuWindow ;
		break ;
	default :
		kind = kTextWindow ;
		break ;
	}
	dprintf ( "Returning kind %s" , typeStr [ kind ] ) ;
	return kind ;
}


Boolean
RetrieveWinPos ( WindowPtr win , short * top , short * left )
{
	short kind ;

	kind = GetWinKind ( win ) ;
	if ( kind < 0 || kind > kLastWindowKind ) {
		return 0 ;
	}
	return RetrievePosition ( kind , top , left ) ;
}


void
SaveWindowPos ( WindowPtr win )
{
	short kind ;
	GrafPtr gp ;
	Point p = { 0 , 0 } ;

	kind = GetWinKind ( win ) ;
	if ( kind < 0 || kind > kLastWindowKind ) {
		return ;
	}
	GetPort ( & gp ) ;
	SetPort ( win ) ;
	LocalToGlobal ( & p ) ;
	AddPt ( * ( Point * ) & ( win -> portRect ) , & p ) ; /* Adjust for origin */
	SetPort ( gp ) ;
	SavePosition ( kind , p . v , p . h ) ;
}


void
SaveWindowSize ( WindowPtr win )
{
	short kind , width , height ;

	kind = GetWinKind ( win ) ;
	width = win -> portRect . right - win -> portRect . left ;
	height = win -> portRect . bottom - win -> portRect . top ;
	SaveSize ( kind , height , width ) ;
}
