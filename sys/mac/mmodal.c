/*	SCCS Id: @(#)mmodal.c	3.1	93/01/24		  */
/* Copyright (c) Jon W{tte, Hao-Yang Wang, Jonathan Handler 1992. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <OSUtils.h>

void FlashButton ( DialogPtr , short ) ;
void trans_num_keys ( EventRecord * ) ;

#define MAX_MV_DIALOGS 20
static int old_dialog_count = 0;
static struct {
	short	  id;
	Boolean   init_visible;
	DialogPtr dialog;
} old_dialog[MAX_MV_DIALOGS];

/* Instead of calling GetNewDialog everytime, just call
   SelectWindow/ShowWindow for the old dialog to remember its location.
*/
/*
 *	Unfortunately, this does not work, as it doesn't handle old text
 *	in edit text boxes, and not ParamText parameters either.
 *
 */
DialogPtr
mv_get_new_dialog(short dialogID)
{
	DialogPtr dialog;
	int d_idx = old_dialog_count;
	Rect oldRect ;
	Boolean hadOld = 0 ;

	old_dialog[0].id = dialogID;
	while (old_dialog[d_idx].id != dialogID)
		--d_idx;

/*
 *	This routine modified so that the old dialog is
 *	disposed, and the new one read in after we remember
 *	the old dialog's position.
 *
 *	This takes care of strange default strings and ParamTexts
 *
 */

	if ( d_idx ) {

		dialog = old_dialog [ d_idx ] . dialog ;
		oldRect = dialog -> portBits . bounds ;
		DisposeDialog ( dialog ) ;
		old_dialog [ d_idx ] . dialog = ( DialogPtr ) NULL ;
		hadOld = 1 ;

	} else {

		d_idx = ++ old_dialog_count ;
	}

	dialog = GetNewDialog(dialogID, nil, (WindowPtr)-1);
	if (dialog) {

		if ( hadOld ) {

			MoveWindow ( dialog , - oldRect . left , - oldRect . top , FALSE ) ;
		}
		old_dialog[d_idx].id = dialogID;
		old_dialog[d_idx].init_visible
			= ((WindowPeek)dialog)->visible;
		old_dialog[d_idx].dialog = dialog;
	}
	return dialog;
}

/* Instead of actually closing the dialog, just hide it so its location
   is remembered. */
void mv_close_dialog(DialogPtr dialog) {
	HideWindow(dialog);
}

/* This routine is stolen/borrowed from HandleClick (macwin.c).  See the
   comments in mv_modal_dialog for more information. */
void
mv_handle_click ( EventRecord * theEvent )
{
	int code ;
	WindowPtr theWindow ;
	Rect r = ( * GetGrayRgn ( ) ) -> rgnBBox ;

	InsetRect ( & r , 4 , 4 ) ;
	InitCursor ( ) ;

	code = FindWindow ( theEvent -> where , & theWindow ) ;

	switch ( code ) {

	case inContent :
		if ( theWindow != FrontWindow ( ) ) {
			nhbell ( ) ;
		}
		break ;

	case inDrag :
		InitCursor ( ) ;
		DragWindow ( theWindow , theEvent -> where , & r ) ;
		SaveWindowPos ( theWindow ) ;
		break ;

	default :
		HandleEvent ( theEvent ) ;
	}
}

void
mv_modal_dialog(ModalFilterProcPtr filterProc, short *itemHit)
{
	GrafPtr org_port;
	GetPort(&org_port);

	for (;;) {
		DialogPtr dialog = FrontWindow();
		EventRecord evt;

		WaitNextEvent(everyEvent, &evt, GetCaretTime(), nil);

		if (evt.what == keyDown)
			if (evt.modifiers & cmdKey) {
				if ((evt.message & charCodeMask) == '.') {
					/* 0x351b is the key code and character code of the esc key. */
					evt.message = 0x351b;
					evt.modifiers &= ~cmdKey;
				}
			} else
				trans_num_keys(&evt);

		if (filterProc) {
			if ((*filterProc)(dialog, &evt, itemHit))
				break;
		} else if (evt.what == keyDown) {
			char ch = evt.message & charCodeMask;
			if (ch == CHAR_CR || ch == CHAR_ENTER) {
				*itemHit = ok;
				FlashButton(dialog, ok);
				break;
			}
		}

		if (IsDialogEvent(&evt)) {
			DialogPtr dont_care;
			if (DialogSelect(&evt, &dont_care, itemHit))
				break;

		/* The following part is problemmatic: (1) Calling HandleEvent
		   here may cause some re-entrance problem (seems ok, but I am
		   not sure). (2) It is ugly to treat mouseDown events as a
		   special case.  If we can just say "else HandleEvent(&evt);"
		   here it will be better. */
		} else if (evt.what == mouseDown)
				mv_handle_click(&evt);
			else
				HandleEvent(&evt);

		SetPort(org_port);
	}
}
