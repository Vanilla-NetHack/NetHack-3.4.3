/*	SCCS Id: @(#)mactopl.c	3.1	91/07/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <Dialogs.h>
#include <OSUtils.h>
#include <Packages.h>

// int NDECL(mac_doprev_message);
// char FDECL(yn_function,(const char *, const char *, CHAR_P));

int FDECL ( try_key_queue , ( char * ) ) ;

extern void SetFrameItem ( DialogPtr , short , short ) ;
extern void FlashButton ( DialogPtr , short ) ;

extern char * PtoCstr ( unsigned char * ) ;
extern unsigned char * CtoPstr ( char * ) ;

void FDECL ( enter_topl_mode , ( char * ) ) ;
void FDECL ( leave_topl_mode , ( char * ) ) ;
void FDECL ( topl_set_resp , ( char * , char ) ) ;

extern winid inSelect ;
extern short frame_corner ;

int
mac_doprev_message(void)
{
	NhWindow * aWin = & theWindows [ WIN_MESSAGE ] ;
	char * start , * stop ;

	if ( ! WIN_MESSAGE )
		return 0 ;

	stop = * aWin -> windowText ;
	start = * aWin -> windowText + aWin -> textBase - 2 ;

	while ( start > stop && * start != 10 && * start != 13 )
		start -- ;

	if ( start <= stop )
		aWin -> textBase = 0L ;
	else
		aWin -> textBase = start - stop + 1 ;
	if ( aWin -> textBase > aWin -> windowTextLen )
		aWin -> textBase = aWin -> windowTextLen ;

	display_nhwindow ( WIN_MESSAGE , FALSE ) ;
	InvalRect ( & ( aWin -> theWindow -> portRect ) ) ;

	return 0 ;
}


char
queued_resp(char *resp)
{
	char buf[30];
	if (try_key_queue(&buf)) {
		if (!resp || strchr(resp, buf[0]))
			return buf[0];
		if (digit(buf[0]) && strchr(resp, '#')) {
			yn_number = atoi(buf);
			return '#';
		}
	}
	return '\0';
}


#define YN_DLOG 133
#define YNQ_DLOG 134
#define YNAQ_DLOG 135
#define YNNAQ_DLOG 136

static int yn_user_item [ ] = { 5 , 6 , 7 , 8 } ;
static short gEnterItem , gEscItem ;
static const char * gRespStr = NULL ;
static char gDef = 0 ;
static short dlogID ;


static void
SetEnterItem ( DialogPtr dp , const short newEnterItem )
{
	short kind ;
	Handle item ;
	Rect r , r2 ;

	if ( gEnterItem != newEnterItem ) {

		GetDItem ( dp , gEnterItem , & kind , & item , & r2 ) ;
		InsetRect ( & r2 , - 4 , - 4 ) ;
		EraseRect ( & r2 ) ;
		InvalRect ( & r2 ) ;

		gEnterItem = newEnterItem ;

		GetDItem ( dp , newEnterItem , & kind , & item , & r2 ) ;
		frame_corner = kind == ctrlItem + btnCtrl ? 16 : 0 ;
		InsetRect ( & r2 , - 4 , - 4 ) ;
		InvalRect ( & r2 ) ;
		r = r2 ;
		GetDItem ( dp , yn_user_item [ dlogID - YN_DLOG ] , & kind , & item , & r2 ) ;
		SetDItem ( dp , yn_user_item [ dlogID - YN_DLOG ] , kind , item , & r ) ;
	}
}


static void
do_tabbing ( DialogPtr dp )
{
	SetEnterItem(dp, gEnterItem == 1 ? strlen(gRespStr) : gEnterItem - 1);
}


static void
set_yn_number(DialogPtr dp)
{
	if (gRespStr && gRespStr[gEnterItem-1] == '#') {
		short k;
		Handle h;
		Rect r;
		Str255 s;
		GetDItem(dp, gEnterItem, &k, &h, &r);
		GetIText(h, s);
		if (s[0])
			StringToNum(s, &yn_number);
	}
}


pascal Boolean
YNAQFilter ( DialogPtr dp , EventRecord * ev , short * itemHit )
{
	unsigned char code ;
	char ch ;
	char * re = gRespStr ;

	if ( ev -> what != keyDown ) {

		return 0 ;
	}
	code = ( ev -> message & 0xff00 ) >> 8 ;
	ch = ev -> message & 0xff ;

	switch ( code ) {

	case 0x24 :
	case 0x4c :
		set_yn_number ( dp ) ;
		* itemHit = gEnterItem ;
		FlashButton ( dp , * itemHit ) ;
		return 1 ;

	case 0x35 :
	case 0x47 :
		* itemHit = gEscItem ;
		FlashButton ( dp , * itemHit ) ;
		return 1 ;

	case 0x30 :
		do_tabbing ( dp ) ;
		return 0 ;
	}
	switch ( ch ) {

	case '\r' :
	case '\n' :
	case ' ' :
	case 3 :
		set_yn_number ( dp ) ;
		* itemHit = gEnterItem ;
		FlashButton ( dp , * itemHit ) ;
		return 1 ;

	case 9 :
		do_tabbing ( dp ) ;
		return 0 ;

	case 27 :
		* itemHit = gEscItem ;
		FlashButton ( dp , * itemHit ) ;
		return 1 ;

	case CHAR_BS :
	case 28 : case 29 : case 30 : case 31 : /* the four arrow keys */
	case '0' : case '1' : case '2' : case '3' : case '4' :
	case '5' : case '6' : case '7' : case '8' : case '9' :
	{	char * loc = strchr ( gRespStr , '#' ) ;
		if ( loc ) {
			SetEnterItem( dp , loc - gRespStr + 1 ) ;
			return 0; /* Dialog Manager will then put this key into the text field. */
		}
	}
	}

	while ( * re ) {

		if ( * re == ch ) {

			* itemHit = ( re - gRespStr ) + 1 ;
			FlashButton ( dp , * itemHit ) ;
			return 1 ;
		}
		re ++ ;
	}

	nhbell ( ) ;
	ev -> what = nullEvent ;
	return 0 ;
}


static char
do_question_dialog ( char * query , int dlog , int defbut , char * resp )
{
	Str255 p ;
	DialogPtr dp ;
	short item ;

	char c = queued_resp ( resp ) ;
	if ( c )
		return c ;

	dlogID = dlog ;
	strcpy ( p , query ) ;
	ParamText ( CtoPstr ( p ) , NULL , NULL , NULL ) ;
	dp = mv_get_new_dialog ( dlog ) ;
	if ( ! dp ) {

		return 0 ;
	}
	SetPort ( dp ) ;
	ShowWindow ( dp ) ;

	gEscItem = strlen ( resp ) ;
	gEnterItem = defbut ;
	gRespStr = resp ;

	SetFrameItem ( dp , yn_user_item [ dlogID - YN_DLOG ] , gEnterItem ) ;

	InitCursor ( ) ;
	mv_modal_dialog ( YNAQFilter , & item ) ;
	mv_close_dialog ( dp ) ;
	return resp [ item - 1 ] ;
}


pascal Boolean
OneCharDLOGFilter ( DialogPtr dp , EventRecord * ev , short * item )
{
	char ch ;
	short k ;
	Handle h ;
	Rect r ;
	unsigned char com [ 2 ] ;

	if ( ev -> what != keyDown ) {

		return 0 ;
	}
	ch = ev -> message & 0xff ;

	com [ 0 ] = 1 ;
	com [ 1 ] = ch ;

	if ( ch == 27 ) {

		GetDItem ( dp , 4 , & k , & h , & r ) ;
		SetIText ( h , com ) ;
		* item = 2 ;
		FlashButton ( dp , 2 ) ;
		return 1 ;
	}
	if ( ! gRespStr || strchr ( gRespStr , ch ) ) {

		GetDItem ( dp , 4 , & k , & h , & r ) ;
		SetIText ( h , com ) ;
		* item = 1 ;
		FlashButton ( dp , 1 ) ;
		return 1 ;
	}
	if ( ch == 10 || ch == 13 || ch == 3 || ch == 32 ) {

		com [ 1 ] = gDef ;
		GetDItem ( dp , 4 , & k , & h , & r ) ;
		SetIText ( h , com ) ;
		* item = 1 ;
		FlashButton ( dp , 1 ) ;
		return 1 ;
	}
	if ( ch > 32 && ch < 127 ) {

		GetDItem ( dp , 4 , & k , & h , & r ) ;
		SetIText ( h , com ) ;
		* item = 1 ;
		FlashButton ( dp , 1 ) ;
		return 1 ;
	}
	nhbell ( ) ;
	ev -> what = nullEvent ;
	return 1 ;
}


static char
generic_yn_function ( query , resp , def )
const char * query , * resp ;
char def ;
{
	DialogPtr dp ;
	short k , item ;
	Handle h ;
	Rect r ;
	unsigned char com [ 32 ] = { 1 , 27 } ; // margin for getitext
	Str255 pQuery ;

	char c = queued_resp ( resp ) ;
	if ( c )
		return c ;

	dp = mv_get_new_dialog ( 137 ) ;
	if ( ! dp ) {

		return 0 ;
	}
	SetPort ( dp ) ;
	ShowWindow ( dp ) ;
	InitCursor ( ) ;
	SetFrameItem ( dp , 6 , 1 ) ;
	if ( def ) {

		com [ 1 ] = def ;
	}
	strcpy ( ( char * ) pQuery , query ) ;
	if ( resp && * resp ) {

		strcat ( ( char * ) pQuery , " (" ) ;
		strcat ( ( char * ) pQuery , resp ) ;
		strcat ( ( char * ) pQuery , ")" ) ;
	}
	ParamText ( CtoPstr ( pQuery ) , NULL , NULL , NULL ) ;
	GetDItem ( dp , 4 , & k , & h , & r ) ;
	SetIText ( h , com ) ;
	SelIText ( dp , 4 , 0 , 0x7fff ) ;
	InitCursor ( ) ;
	SetFrameItem ( dp , 6 , 1 ) ;
	gRespStr = resp ;
	gDef = def ;
	do {

		mv_modal_dialog ( OneCharDLOGFilter , & item ) ;

	} while ( item != 1 && item != 2 ) ;
	GetIText ( h , com ) ;

	mv_close_dialog ( dp ) ;
	if ( item == 2 || ! com [ 0 ] ) {

		return 27 ; // escape
	}
	return com [ 1 ] ;
}


static char
ynaq_dialog ( query , resp , def )
const char * query , * resp ;
char def ;
{
	int dia = 0 ;

	if ( resp ) {

		if ( ! strcmp ( resp , ynchars ) ) {

			dia = YN_DLOG ;
		}
		if ( ! strcmp ( resp , ynqchars ) ) {

			dia = YNQ_DLOG ;
		}
		if ( ! strcmp ( resp , ynaqchars ) ) {

			dia = YNAQ_DLOG ;
		}
		if ( ! strcmp ( resp , ynNaqchars ) ) {

			dia = YNNAQ_DLOG ;
		}
	}
	if ( ! dia ) {

		return generic_yn_function ( query , resp , def ) ;
	}

	return do_question_dialog ( query , dia ,
		( strchr ( resp , def ) - resp ) + 1 , resp ) ;
}


char
topl_yn_function(query,resp, def)
const char *query,*resp;
char def;
{
	char buf[30];
	char c = queued_resp(resp);
	if (!c) {
		enter_topl_mode(query);
		topl_set_resp(resp, def);

		do {
			c = nhgetch();
			if (c && resp && !strchr(resp, c)) {
				nhbell();
				c = '\0';
			}
		} while (!c);

		topl_set_resp("", '\0');
		leave_topl_mode(&buf);
		if (c == '#')
			yn_number = atoi(buf);
	}
	return c;
}


char
popup_yn_function(query,resp, def)
const char *query,*resp;
char def;
{
	char ch [ 2 ] ;

	if ( ch [ 0 ] = ynaq_dialog ( query , resp , def ) ) {

		return ch [ 0 ] ;
	}

	return topl_yn_function(query, resp, def);
}


char
mac_yn_function(query,resp, def)
const char *query,*resp;
char def;
/*
 *   Generic yes/no function. 'def' is the default (returned by space or
 *   return; 'esc' returns 'q', or 'n', or the default, depending on
 *   what's in the string. The 'query' string is printed before the user
 *   is asked about the string.
 *   If resp is NULL, any single character is accepted and returned.
 */
{
	if (flags.popup_dialog)
		return popup_yn_function(query, resp, def);
	else
		return topl_yn_function(query, resp, def);
}

/*topl.c*/
