/*	SCCS Id: @(#)mactopl.c	3.1	91/07/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "macwin.h"
#include <Dialogs.h>
#include <OSUtils.h>
#include <Packages.h>

static char queued_resp(char *resp);
static pascal Boolean YNAQFilter(DialogPtr dp, EventRecord *ev , short *itemHit);
static pascal Boolean OneCharDLOGFilter(DialogPtr dp, EventRecord *ev, short *item);
static char topl_yn_function(const char *query, const char *resp, char def);
static char popup_yn_function(const char *query, const char *resp, char def);

static char
queued_resp(char *resp)
{
	char buf[30];
	if (try_key_queue(buf)) {
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
static const char * gRespStr = (const char *)0 ;
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


static pascal Boolean
YNAQFilter ( DialogPtr dp , EventRecord * ev , short * itemHit )
{
	unsigned char code ;
	char ch ;
	char * re = (char *) gRespStr ;

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

	char c = queued_resp ( (char *) resp ) ;
	if ( c )
		return c ;

	dlogID = dlog ;
	strcpy ( (char *) p , query ) ;
	ParamText ( CtoPstr ( (char *) p ) , (uchar *) 0 , (uchar *) 0 , (uchar *) 0 ) ;
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


static pascal Boolean
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

	char c = queued_resp ( (char *) resp ) ;
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
	ParamText ( CtoPstr ( (char *) pQuery ) , (uchar *) 0 , (uchar *) 0 , (uchar *) 0 ) ;
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

	return do_question_dialog ( (char *) query , dia ,
		( strchr ( resp , def ) - resp ) + 1 , (char *) resp ) ;
}


static char
topl_yn_function(const char *query, const char *resp, char def)
{
	char buf[30];
	char c = queued_resp((char *) resp);
	if (!c) {
		enter_topl_mode((char *) query);
		topl_set_resp((char *) resp, def);

		do {
			c = readchar();
			if (c && resp && !strchr(resp, c)) {
				nhbell();
				c = '\0';
			}
		} while (!c);

		topl_set_resp("", '\0');
		leave_topl_mode(buf);
		if (c == '#')
			yn_number = atoi(buf);
	}
	return c;
}


static char
popup_yn_function(const char *query, const char *resp, char def)
{
	char ch ;

	if ( ch = ynaq_dialog ( query , resp , def ) )
		return ch ;

	return topl_yn_function(query, resp, def);
}


char
mac_yn_function(query, resp, def)
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
