/*    SCCS Id: @(#)winmenu.c    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "amiga:windefs.h"
#include "amiga:winext.h"
#include "amiga:winproto.h"

/* Start building the text for a menu */
void
amii_start_menu(window)
    register winid window;
{
    register int i;
    register struct amii_WinDesc *cw;

    if(window == WIN_ERR || (cw = amii_wins[window]) == NULL || cw->type != NHW_MENU)
	panic(winpanicstr,window, "start_menu");

    amii_clear_nhwindow(window);
    if( window == WIN_INVEN && cw->win != NULL )
    {
	if( cw->data && ( cw->type == NHW_MESSAGE ||
		cw->type == NHW_MENU || cw->type == NHW_TEXT ) )
	{
	    for( i = 0; i < cw->maxrow; ++i )
	    {
		if( cw->data[ i ] )
		    free( cw->data[ i ] );
	    }
	    free( cw->data );
	    cw->data = NULL;
	}

	if( cw->canresp ) free( cw->canresp );
	cw->canresp = NULL;

	if( cw->morestr ) free( cw->morestr );
	cw->morestr = NULL;
	if( alwaysinvent )
	    cw->wasup = 1;
    }
    if( cw->resp )
	*cw->resp = 0;
    cw->maxrow = cw->maxcol = 0;
    return;
}

/* Add a string to a menu */
void
amii_add_menu(window,ch,attr,str)
    register winid window;
    register char ch;
    register int attr;
    register const char *str;
{
    register struct amii_WinDesc *cw;
    char tmpbuf[2];

    if(str == NULL)return;

    if(window == WIN_ERR || (cw = amii_wins[window]) == NULL || cw->type != NHW_MENU)
	panic(winpanicstr,window, "add_menu");

    /* this should translate fonts if a title line */
    amii_putstr(window, attr, str);

    if( !cw->resp )
	panic("No response buffer in add_menu()");

    if( !cw->data )
	panic("No data buffer in add_menu()");

    if(ch != '\0')
    {
	tmpbuf[0]=ch;
	tmpbuf[1]=0;
	Strcat(cw->resp, tmpbuf);
	cw->data[ cw->cury - 1 ][ SEL_ITEM ] = 1;
    }
    else
    {
	/* Use something as a place holder.  ^A is probably okay */

	tmpbuf[0]=1;
	tmpbuf[1]=0;
	Strcat(cw->resp, tmpbuf);
	cw->data[ cw->cury - 1 ][ SEL_ITEM ] = 0;
    }
}

/* Done building a menu. */

void
amii_end_menu(window,cancel,str,morestr)
    register winid window;
    register char cancel;
    register const char *str;
    register const char *morestr;
{
    register struct amii_WinDesc *cw;

    if(window == WIN_ERR || (cw=amii_wins[window]) == NULL || cw->type != NHW_MENU
	  || cw->canresp)
	panic(winpanicstr,window, "end_menu");

    if(str)
    {
	cw->canresp = (char*) alloc(strlen(str)+2);
	cw->canresp[0]=cancel;
	Strcpy(&cw->canresp[1],str);

	if( !cw->resp )
	    panic("No response buffer in end_menu()");

	strncat(cw->resp, str, 1);
    }

    if(morestr)
    {
	cw->morestr =(char *) alloc(strlen(morestr)+1);
	Strcpy(cw->morestr, morestr);
    }
}

/* Select something from the menu. */

char
amii_select_menu(window)
    register winid window;
{
    register struct amii_WinDesc *cw;

    if( window == WIN_ERR || ( cw=amii_wins[window] ) == NULL ||
	  cw->type != NHW_MENU )
	panic(winpanicstr,window, "select_menu");

    morc = 0;                       /* very ugly global variable */
    amii_display_nhwindow(window,TRUE); /* this waits for input */

    /* This would allow the inventory window to stay open. */
    if( !alwaysinvent || window != WIN_INVEN )
	dismiss_nhwindow(window);       /* Now tear it down */
    return morc;
}

void
DoMenuScroll( win, blocking )
    int win, blocking;
{
    register struct Window *w;
    register struct NewWindow *nw;
    struct PropInfo *pip;
    register struct amii_WinDesc *cw;
    struct IntuiMessage *imsg;
    struct Gadget *gd;
    register int wheight, xsize, ysize, aredone = 0;
    register int txwd, txh;
    long mics, secs, class, code;
    long oldmics = 0, oldsecs = 0;
    int aidx, oidx, topidx, hidden;
    int totalvis, i;
    char *t;
    SHORT mx, my;
    static char title[ 100 ];
    int dosize = 1;
    struct Screen *scrn = HackScreen;

    if( win == WIN_ERR || ( cw = amii_wins[ win ] ) == NULL )
	panic(winpanicstr,win,"DoMenuScroll");

    /*  Initial guess at window sizing values */
    txwd = txwidth;
    txh = txheight; /* interline space */

    /* Check to see if we should open the window, should need to for
     * TEXT and MENU but not MESSAGE.
     */

    totalvis = cw->maxrow;
    w = cw->win;
    topidx = 0;

    if( w == NULL )
    {

#ifdef  INTUI_NEW_LOOK
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	{
	    PropScroll.Flags |= PROPNEWLOOK;
	}
#endif
	nw = (void *)DupNewWindow( (void *)(&new_wins[ cw->type ].newwin) );
	if( !alwaysinvent || win != WIN_INVEN )
	{
	    xsize = scrn->WBorLeft + scrn->WBorRight + MenuScroll.Width + 1 +
				(txwd * cw->maxcol);

	    if( xsize > amiIDisplay->xpix )
		xsize = amiIDisplay->xpix;

	    /* If next row not off window, use it, else use the bottom */

	    ysize = ( txh * cw->maxrow ) +          	  /* The text space */
		    HackScreen->WBorTop + txheight + 1 +  /* Top border */
		    HackScreen->WBorBottom + 3;     	  /* The bottom border */
	    if( ysize > amiIDisplay->ypix )
		ysize = amiIDisplay->ypix;

	    /* Adjust the size of the menu scroll gadget */

	    nw->TopEdge = 0;
	    if( cw->type == NHW_TEXT && ysize < amiIDisplay->ypix )
		nw->TopEdge += ( amiIDisplay->ypix - ysize ) / 2;
	    nw->LeftEdge = amiIDisplay->xpix - xsize;
	    if( cw->type == NHW_TEXT && xsize < amiIDisplay->xpix )
		nw->LeftEdge -= ( amiIDisplay->xpix - xsize ) / 2;
	}
	else
	{
	    struct Window *mw = amii_wins[ WIN_MAP ]->win;
	    struct Window *sw = amii_wins[ WIN_STATUS ]->win;

#ifndef	VIEWWINDOW
	    xsize = scrn->WBorLeft + scrn->WBorRight + MenuScroll.Width + 1 +
				(txwd * cw->maxcol);
#else
	    xsize = mw->Width;
#endif
	    if( xsize > amiIDisplay->xpix )
		xsize = amiIDisplay->xpix;

	    /* If next row not off window, use it, else use the bottom */

	    ysize = sw->TopEdge - (mw->TopEdge + mw->Height) - 1;
	    if( ysize > amiIDisplay->ypix )
		ysize = amiIDisplay->ypix;

	    /* Adjust the size of the menu scroll gadget */

	    nw->TopEdge = mw->TopEdge + mw->Height;
	    nw->LeftEdge = 0;
	}
	cw->newwin = (void *)nw;
	if( nw == NULL )
	    panic("No NewWindow Allocated" );

	nw->Screen = HackScreen;

	if( win == WIN_INVEN )
	{
	    sprintf( title, "%s the %s's Inventory", plname, pl_character );
	    nw->Title = title;
	    if( lastinvent.MaxX != 0 )
	    {
		nw->LeftEdge = lastinvent.MinX;
		nw->TopEdge = lastinvent.MinY;
		nw->Width = lastinvent.MaxX;
		nw->Height = lastinvent.MaxY;
	    }
	}
	else if( cw->morestr )
	    nw->Title = cw->morestr;

	/* Adjust the window coordinates and size now that we know
	 * how many items are to be displayed.
	 */

	if( ( xsize > amiIDisplay->xpix - nw->LeftEdge ) &&
	    ( xsize < amiIDisplay->xpix ) )
	{
	    nw->LeftEdge = amiIDisplay->xpix - xsize;
	    nw->Width = xsize;
	}
	else
	{
	    nw->Width = min( xsize, amiIDisplay->xpix - nw->LeftEdge );
	}
	nw->Height = min( ysize, amiIDisplay->ypix - nw->TopEdge );

	/* Now, open the window */
	w = cw->win = OpenShWindow( (void *)nw );

	if( w == NULL )
	{
	    char buf[ 130 ];

	    sprintf( buf, "No Window Opened For Menu (%d,%d,%d-%d,%d-%d)",
		nw->LeftEdge, nw->TopEdge, nw->Width, amiIDisplay->xpix,
		nw->Height, amiIDisplay->ypix );
	    panic( buf );
	}

#ifdef HACKFONT
	if( TextsFont )
	    SetFont(w->RPort, TextsFont );
	else if( HackFont )
	    SetFont(w->RPort, HackFont );
#endif
	txwd = w->RPort->TxWidth;
	txh = w->RPort->TxHeight; /* interline space */

	/* subtract 2 to account for spacing away from border (1 on each side) */
	wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) / txh;
	cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 2 ) / txwd;
    }
    else
    {
	txwd = w->RPort->TxWidth;
	txh = w->RPort->TxHeight; /* interline space */

	/* subtract 2 to account for spacing away from border (1 on each side) */
	wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) / txh;
	cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 2 ) / txwd;

	if( win == WIN_MESSAGE )
	{
	    for( totalvis = i = 0; i < cw->maxrow; ++i )
	    {
		if( cw->data[i] && cw->data[i][1] != 0xff )
		    ++totalvis;
	    }
	}
	for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
	    gd = gd->NextGadget;

	if( gd )
	{
	    pip = (struct PropInfo *)gd->SpecialInfo;
	    hidden = max( cw->maxrow - wheight, 0 );
	    topidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
	}
    }

    /* Find the scroll gadget */
    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; gd = gd->NextGadget )
	continue;

    if( !gd ) panic("Can't find scroll gadget" );

    morc = 0;
    oidx = -1;

    DisplayData( win, topidx, -1 );
    WindowToFront( w );

    /* Make the prop gadget the right size and place */

    SetPropInfo( w, gd, wheight, totalvis, topidx );
    oldsecs = oldmics = 0;

    /* If window already up, don't stop to process events */
    if( cw->wasup )
    {
    	aredone = 1;
    	cw->wasup = 0;
    }

    while( !aredone )
    {
	/* Process window messages */

	WaitPort( w->UserPort );
	while( imsg = (struct IntuiMessage * ) GetMsg( w->UserPort ) )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    mics = imsg->Micros;
	    secs = imsg->Seconds;
	    gd = (struct Gadget *) imsg->IAddress;
	    mx = imsg->MouseX;
	    my = imsg->MouseY;

	    /* Only do our window or VANILLAKEY from other windows */

	    if( imsg->IDCMPWindow != w && class != VANILLAKEY &&
							class != RAWKEY )
	    {
		ReplyMsg( (struct Message *) imsg );
		continue;
	    }

	    /* Do DeadKeyConvert() stuff if RAWKEY... */
	    if( class == RAWKEY )
	    {
		class = VANILLAKEY;
		code = ConvertKey( imsg );
	    }
	    ReplyMsg( (struct Message *) imsg );

	    switch( class )
	    {
		case NEWSIZE:
		    /* Ignore every other newsize, no action needed */

		    if( !dosize )
		    {
			dosize = 1;
			break;
		    }

		    if( win == WIN_INVEN )
		    {
			lastinvent.MinX = w->LeftEdge;
			lastinvent.MinY = w->TopEdge;
			lastinvent.MaxX = w->Width;
			lastinvent.MaxY = w->Height;
		    }
		    else if( win == WIN_MESSAGE )
		    {
			lastmsg.MinX = w->LeftEdge;
			lastmsg.MinY = w->TopEdge;
			lastmsg.MaxX = w->Width;
			lastmsg.MaxY = w->Height;
		    }

		    /* Find the gadget */

		    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			gd = gd->NextGadget;

		    if( !gd )
			panic("Can't find scroll gadget" );

		    wheight = ( w->Height - w->BorderTop -
						w->BorderBottom - 2) / txh;
		    cw->cols = ( w->Width -
				w->BorderLeft - w->BorderRight ) / txwd;
		    if( wheight < 2 )
			wheight = 2;

		    /* Make the prop gadget the right size and place */

		    DisplayData( win, topidx, oidx );
		    SetPropInfo( w, gd, wheight, totalvis, topidx );

		    /* Force the window to a text line boundry <= to
		     * what the user dragged it to.  This eliminates
		     * having to clean things up on the bottom edge.
		     */

		    SizeWindow( w, 0, ( wheight * txh) +
			    w->BorderTop + w->BorderBottom + 2 - w->Height );

		    /* Don't do next NEWSIZE, we caused it */
		    dosize = 0;
		    oldsecs = oldmics = 0;
		    break;

		case VANILLAKEY:
#define CTRL(x)     ((x)-'@')
		    if( code == CTRL('D') || code == CTRL('U') )
		    {
			int endcnt, i;

			for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			    gd = gd->NextGadget;

			if( !gd )
			    panic("Can't find scroll gadget" );

			endcnt = wheight / 2;
			if( endcnt == 0 )
			    endcnt = 1;

			for( i = 0; i < endcnt; ++i )
			{
			    if( code == CTRL('D') )
			    {
				if( topidx + wheight < cw->maxrow )
				    ++topidx;
				else
				    break;
			    }
			    else
			    {
				if( topidx > 0 )
				    --topidx;
				else
				    break;
			    }

			    /* Make prop gadget the right size and place */

			    DisplayData( win, topidx, oidx );
			    SetPropInfo( w,gd, wheight, totalvis, topidx );
			}
			oldsecs = oldmics = 0;
		    }
		    else if( code == '\b' )
		    {
			for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			    gd = gd->NextGadget;

			if( !gd )
			    panic("Can't find scroll gadget" );

			if( topidx - wheight - 2 < 0 )
			{
			    topidx = 0;
			}
			else
			{
			    topidx -= wheight - 2;
			}
			DisplayData( win, topidx, oidx );
			SetPropInfo( w, gd, wheight, totalvis, topidx );
			oldsecs = oldmics = 0;
		    }
		    else if( code == ' ' )
		    {
			for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			    gd = gd->NextGadget;

			if( !gd )
			    panic("Can't find scroll gadget" );

			if( topidx + wheight >= cw->maxrow )
			{
			    morc = 0;
			    aredone = 1;
			}
			else
			{
			    /*  If there are still lines to be seen */

			    if( cw->maxrow > topidx + wheight )
			    {
				if( wheight > 2 )
				    topidx += wheight - 2;
				else
				    ++topidx;
				DisplayData( win, topidx, oidx );
				SetPropInfo( w, gd, wheight,
						    totalvis, topidx );
			    }
			    oldsecs = oldmics = 0;
			}
		    }
		    else if( code == '\n' || code == '\r' )
		    {
			for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			    gd = gd->NextGadget;

			if( !gd )
			    panic("Can't find scroll gadget" );

			/* If all line displayed, we are done */

			if( topidx + wheight >= cw->maxrow )
			{
			    morc = 0;
			    aredone = 1;
			}
			else
			{
			    /*  If there are still lines to be seen */

			    if( cw->maxrow > topidx + 1 )
			    {
				++topidx;
				DisplayData( win, topidx, oidx );
				SetPropInfo( w, gd, wheight,
						    totalvis, topidx );
			    }
			    oldsecs = oldmics = 0;
			}
		    }
		    else if( cw->resp && index( cw->resp, code ) )
		    {
			morc = code;
			aredone = 1;
		    }
		    else if( code == '\33' || code == 'q' || code == 'Q' )
		    {
			morc = '\33';
			aredone = 1;
		    }
		    break;

		case CLOSEWINDOW:
		    if( win == WIN_INVEN )
		    {
			lastinvent.MinX = w->LeftEdge;
			lastinvent.MinY = w->TopEdge;
			lastinvent.MaxX = w->Width;
			lastinvent.MaxY = w->Height;
		    }
		    else if( win == WIN_MESSAGE )
		    {
			lastmsg.MinX = w->LeftEdge;
			lastmsg.MinY = w->TopEdge;
			lastmsg.MaxX = w->Width;
			lastmsg.MaxY = w->Height;
		    }
		    aredone = 1;
		    morc = '\33';
		    break;

		case GADGETUP:
		    if( win == WIN_MESSAGE )
			aredone = 1;
		    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			gd = gd->NextGadget;

		    pip = (struct PropInfo *)gd->SpecialInfo;
		    hidden = max( cw->maxrow - wheight, 0 );
		    aidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
		    if( aidx != topidx )
			DisplayData( win, topidx = aidx, oidx );
		    break;

		case MOUSEMOVE:
		    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			gd = gd->NextGadget;

		    pip = (struct PropInfo *)gd->SpecialInfo;
		    hidden = max( cw->maxrow - wheight, 0 );
		    aidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
		    if( aidx != topidx )
			DisplayData( win, topidx = aidx, oidx );
		    break;

		case INACTIVEWINDOW:
		    if( win == WIN_MESSAGE || ( win == WIN_INVEN && alwaysinvent ) )
			aredone = 1;
		    break;

		case MOUSEBUTTONS:
		    if( ( code == SELECTUP || code == SELECTDOWN ) &&
						    cw->type == NHW_MENU )
		    {
			/* Which one is the mouse pointing at? */

			aidx = ( ( my - w->BorderTop - 1 ) /
						w->RPort->TxHeight ) + topidx;

			/* If different lines, don't select double click */

			if( aidx != oidx )
			{
			    oldsecs = 0;
			    oldmics = 0;
			}

			/* If releasing, check for double click */

			if( code == SELECTUP )
			{
			    if( aidx == oidx )
			    {
				if( DoubleClick( oldsecs,
						    oldmics, secs, mics ) )
				{
				    morc = cw->resp[ aidx ];
				    aredone = 1;
				}
				oldsecs = secs;
				oldmics = mics;
			    }
			}
			else if( aidx < cw->maxrow && code == SELECTDOWN )
			{
			    /* Remove old highlighting if visible */

			    if( oidx > topidx && oidx - topidx < wheight )
			    {
				t = cw->data[ oidx ] + SOFF;
				amii_curs( win, 1, oidx - topidx  );
				SetDrMd( w->RPort, JAM2 );
				SetAPen( w->RPort, C_WHITE );
				SetBPen( w->RPort, C_BLACK );
				Text( w->RPort, t, strlen( t ) );
				oidx = -1;
			    }

			    t = cw->data[ aidx ];
			    if( t && t[ SEL_ITEM ] == 1 )
			    {
				oidx = aidx;

				amii_curs( win, 1, aidx - topidx );
				SetDrMd( w->RPort, JAM2 );
				SetAPen( w->RPort, C_BLUE );
				SetBPen( w->RPort, C_WHITE );
				t += SOFF;
				Text( w->RPort, t, strlen( t ) );
			    }
			    else
			    {
				DisplayBeep( NULL );
				oldsecs = 0;
				oldmics = 0;
			    }
			}
		    }
		    else
		    {
			DisplayBeep( NULL );
		    }
		    break;
	    }
	}
    }
    /* Force a cursor reposition before next message output */
    if( win == WIN_MESSAGE )
	cw->curx = -1;

#if 0
    if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
	ActivateWindow( amii_wins[ WIN_MAP ]->win );
#endif
}

ReDisplayData( win )
    winid win;
{
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register struct Gadget *gd;
    unsigned long hidden, aidx, wheight;
    struct PropInfo *pip;
    
    if( win == WIN_ERR || !(cw = amii_wins[win]) || !( w = cw->win ) )
	return;

    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
	gd = gd->NextGadget;

    if( !gd )
	return;

    wheight = ( w->Height - w->BorderTop -
			w->BorderBottom - 2 ) / w->RPort->TxHeight;
    pip = (struct PropInfo *)gd->SpecialInfo;
    hidden = max( cw->maxrow - wheight, 0 );
    aidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
    DisplayData( win, aidx, -1 );
}

void
DisplayData( win, start, where )
    winid win;
    int start;
    int where;
{
    register char *t;
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register int i, idx, len, wheight;
    int col = -1;

    if( win == WIN_ERR || !(cw = amii_wins[win]) || !( w = cw->win ) )
    {
	panic( winpanicstr, win, "No Window in DisplayData" );
    }

    SetDrMd( w->RPort, JAM2 );
    wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) /
		w->RPort->TxHeight;

    /* Skip any initial response to a previous line */
    if( cw->type == NHW_MESSAGE )
    {
	if( cw->data[start] && cw->data[ start ][1] == 1 )
	    ++start;
    }

    for( idx = i = start; idx < start + wheight; i++ )
    {
    	if( i >= cw->maxrow )
    	{
	    amii_curs( win, 1, idx - start );
	    amii_cl_end( cw, 0 );
	    ++idx;
	    continue;
    	}

	/* Any character with an highlighted attribute go
	 * onto the end of the current line in the message window.
	 */
	if( cw->type == NHW_MESSAGE )
	    SetAPen( w->RPort, cw->data[i][1] ? C_RED : C_WHITE );

	if( cw->type != NHW_MESSAGE || cw->data[i][1] == 0 )
	    amii_curs( win, 1, idx - start );

	if( where == i )
	{
	    if( col != 1 )
	    {
		SetAPen( w->RPort, C_BLUE );
		SetBPen( w->RPort, C_WHITE );
		col = 1;
	    }
	}
	else if( col != 2 )
	{
	    SetAPen( w->RPort, C_WHITE );
	    SetBPen( w->RPort, C_BLACK );
	    col = 2;
	}

	/* Next line out, truncate if too long */

	len = 0;
	t = cw->data[ i ] + SOFF;
	len = strlen( t );
	if( len > cw->cols )
	    len = cw->cols;
	Text( w->RPort, t, len );

	if( cw->type != NHW_MESSAGE || cw->data[i][1] == 0 )
	{
	    amii_cl_end( cw, len );
	    ++idx;
	}
    }
    return;
}

void SetPropInfo( win, gad, vis, total, top )
    register struct Window *win;
    register struct Gadget *gad;
    register long vis, total, top;
{
    long mflags;
    register long hidden;
    register int body, pot;

    hidden = max( total-vis, 0 );

    /* Force the last section to be just to the bottom */

    if( top > hidden )
	top = hidden;

    /* Scale the body position. */
    /* 2 lines overlap */

    if( hidden > 0 && total != 0 )
	body = (ULONG) ((vis - 2) * MAXBODY) / (total - 2);
    else
	body = MAXBODY;

    if( hidden > 0 )
	pot = (ULONG) (top * MAXPOT) / hidden;
    else
	pot = 0;

    mflags = AUTOKNOB|FREEVERT;
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	mflags |= PROPNEWLOOK;
    }
#endif

    NewModifyProp( gad, win, NULL,
			    mflags, 0, pot, MAXBODY, body, 1 );
}
