/*	SCCS Id: @(#)getline.c	3.1	90/22/02
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "Dialogs.h"
#ifdef THINK_C
#include "pascal.h"
#endif
#include <Packages.h>

// void FDECL(getlin,(const char *, char *));
// void FDECL(get_ext_cmd,(char *));
int FDECL ( try_key_queue , ( char * ) ) ;

char * FDECL ( PtoCstr , ( unsigned char * ) ) ;
unsigned char * FDECL ( CtoPstr , ( char * ) ) ;
void SetFrameItem ( DialogPtr , short , short ) ;
void FlashButton ( DialogPtr , short ) ;

void FDECL ( enter_topl_mode , ( char * ) ) ;
void FDECL ( leave_topl_mode , ( char * ) ) ;
Boolean FDECL ( topl_key , ( unsigned char ) ) ;
Boolean FDECL ( topl_ext_key , ( unsigned char ) ) ;

typedef Boolean FDECL ( ( * key_func ) , ( unsigned char ) ) ;


int
get_line_from_key_queue ( char * bufp )
{
	* bufp = 0 ;
	if ( try_key_queue ( bufp ) ) {

		while ( * bufp ) {

			if ( * bufp == 10 || * bufp == 13 ) {

				* bufp = 0 ;
			}
			bufp ++ ;
		}
		return true ;
	}
	return false ;
}


pascal Boolean
getlinFilter ( DialogPtr dp , EventRecord * ev , short * itemHit )
{
	if (ev->what == keyDown) {
		int key = ev->message & keyCodeMask,
			ch	= ev->message & charCodeMask;

		if (ch == 0x1b || key == 0x3500 || key == 0x4700) {
			*itemHit = 2;
			FlashButton(dp, 2);
			return true;

		} else if (ch == CHAR_CR || ch == CHAR_ENTER) {
			*itemHit = 1;
			FlashButton(dp, 1);
			return true;
		}
	}

	return false;
}


void
popup_getlin(const char *query, char *bufp)
{
	ControlHandle	ctrl;
	DialogPtr		promptDialog;
	short			itemHit, type;
	Rect			box;
	Str255			pasStr;

	if ( get_line_from_key_queue ( bufp ) )
		return ;

	/*
	** Make a copy of the prompt string and convert the copy to a Pascal string.
	*/
	
	strcpy((char *) pasStr, query);
	CtoPstr((char *) pasStr);
	
	/*
	** Set the query line as parameter text.
	*/
	
	ParamText(pasStr, "\p", "\p", "\p");
	
	promptDialog = mv_get_new_dialog(130);
	ShowWindow(promptDialog);

	InitCursor ( ) ;
	SetFrameItem ( promptDialog , 6 , 1 ) ;
	do {
		mv_modal_dialog(&getlinFilter, &itemHit);
	} while ((itemHit != 1) && (itemHit != 2));
	
	if (itemHit != 2) {
		/*
		** Get the text from the text edit item.
		*/
		
		GetDItem(promptDialog, 4, &type, (Handle *) &ctrl, &box);
		GetIText((Handle) ctrl, pasStr);
		
		/*
		** Convert it to a 'C' string and copy it into the return value.
		*/
		
		PtoCstr(pasStr);
		strcpy(bufp, (char *) pasStr);
	} else {
		/*
		** Return a null-terminated string consisting of a single <ESC>.
		*/
		
		bufp[0] = '\033';
		bufp[1] = '\0';
	}
	
	mv_close_dialog(promptDialog);
}


void
topl_getlin(const char *query, char *bufp, key_func key)
{
	int q_len = strlen(query);

	if ( get_line_from_key_queue ( bufp ) )
		return ;

	enter_topl_mode(query);
	while ((*key)(nhgetch())) ;
	leave_topl_mode(bufp);
}


/*
 * Read a line closed with '\n' into the array char bufp[BUFSZ].
 * (The '\n' is not stored. The string is closed with a '\0'.)
 * Reading can be interrupted by an escape ('\033') - now the
 * resulting string is "\033".
 */
void
mac_getlin(const char *query, char *bufp)
{
	if (flags.popup_dialog)
		popup_getlin(query, bufp);
	else
		topl_getlin(query, bufp, &topl_key);
}


#ifdef COM_COMPL

pascal Boolean
ExtendedCommandDialogFilter ( DialogPtr dp , EventRecord * ev , short * item )
{
	int ix ;
	Handle h ;
	Rect r ;
	short k ;
	Str255 s ;
	unsigned char com [ 2 ] ;

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
	if ( com [ 1 ] == 27 || ( ev -> message & 0xff00 == 0x3500 ) ) { // escape

		* item = 2 ;
		FlashButton ( dp , 2 ) ;
		return 1 ;
	}
	for ( ix = 3 ; ix ; ix ++ ) {

		h = ( Handle ) NULL ;
		k = 0 ;
		GetDItem ( dp , ix , & k , & h , & r ) ;
		if ( ! k || ! h ) {

			return 0 ;
		}
		if ( k == 6 ) {	//	Radio Button Item

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


void
popup_get_ext_cmd(char *bufp)
{
	ControlHandle	ctrl;
	DialogPtr		extendedDialog;
	short			itemHit, type;
	Rect			box;
	char			*extendedCommand;

	/*
	** Default selection is the first item after the "Cancel" button.
	*/
	static lastItemSelected = 3;

	if ( get_line_from_key_queue ( bufp ) )
		return ;

	extendedDialog = mv_get_new_dialog(131);
	ShowWindow(extendedDialog);

	/*
	** Mark the default selection.
	*/
	
	GetDItem(extendedDialog, lastItemSelected, &type, (Handle *) &ctrl, &box);
	SetCtlValue(ctrl, 1);

	InitCursor ( ) ;
	SetFrameItem ( extendedDialog , 20 , 1 ) ;
	do {
		mv_modal_dialog((ModalFilterProcPtr) ExtendedCommandDialogFilter , &itemHit);
		if ((itemHit != 1) && (itemHit != 2)) {
			/*
			** If OK and Cancel (items 1 and 2) weren't selected then a radio button 
			** was pushed.  Unmark the previous selection.
			*/
			
			GetDItem(extendedDialog, lastItemSelected, &type, (Handle *) &ctrl, &box);
			SetCtlValue(ctrl, 0);
			
			/*
			** Mark the current selection.
			*/
			
			GetDItem(extendedDialog, itemHit, &type, (Handle *) &ctrl, &box);
			SetCtlValue(ctrl, 1);

			/*
			** Save the item number for use later.
			*/
			
			lastItemSelected = itemHit;
		}
	} while ((itemHit != 1) && (itemHit != 2));
	
	if (itemHit == 2) {
		/*
		** Return a null-terminated string consisting of a single <ESC>.
		*/
		
		bufp[0] = '\033';
		bufp[1] = '\0';
	} else {
		switch (lastItemSelected) {
		case 3:
			extendedCommand = "adjust";
			break;
		case 4:
			extendedCommand = "chat";
			break;
		case 5:
			extendedCommand = "dip";
			break;
		case 6:
			extendedCommand = "force";
			break;
		case 7:
			extendedCommand = "jump";
			break;
		case 8:
			extendedCommand = "loot";
			break;
		case 9:
			extendedCommand = "monster";
			break;
		case 10:
			extendedCommand = "name";
			break;
		case 11:
			extendedCommand = "offer";
			break;
		case 12:
			extendedCommand = "pray";
			break;
		case 13:
			extendedCommand = "rub";
			break;
		case 14:
			extendedCommand = "sit";
			break;
		case 15:
			extendedCommand = "turn";
			break;
		case 16:
			extendedCommand = "untrap";
			break;
		case 17:
			extendedCommand = "version";
			break;
		case 18:
			extendedCommand = "window";
			break;
		case 19:
			extendedCommand = "wipe";
			break;
		}
		
		/*
		** Copy the text representing the last radio button selected into the buffer.
		*/
		
		strcpy(bufp, extendedCommand);
	}
	
	mv_close_dialog(extendedDialog);
}


/* Read in an extended command - doing command line completion for
 * when enough characters have been entered to make a unique command.
 * This is just a modified getlin().   -jsb
 */
void
mac_get_ext_cmd(char *bufp)
{
#if 0	/* already in error and then broken by macmenu.c */
	if (flags.popup_dialog)
		popup_get_ext_cmd(bufp);
	else
#endif
		topl_getlin("# ", bufp, &topl_ext_key);
}

#endif /* COM_COMPL /* */

/* macgetline.c */
