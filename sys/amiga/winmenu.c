/*	SCCS Id: @(#)winmenu.c	3.2	96/02/17	*/
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993,1996. */
/* NetHack may be freely redistributed.  See license for details. */

#include "NH:sys/amiga/windefs.h"
#include "NH:sys/amiga/winext.h"
#include "NH:sys/amiga/winproto.h"

/* Start building the text for a menu */
void
amii_start_menu(window)
    register winid window;
{
    register int i;
    register struct amii_WinDesc *cw;
    register amii_menu_item *mip;

    if(window == WIN_ERR || (cw = amii_wins[window]) == NULL || cw->type != NHW_MENU)
	panic(winpanicstr,window, "start_menu");

    amii_clear_nhwindow(window);

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

    for( mip = cw->menu.items, i = 0; (mip = cw->menu.items) && i < cw->menu.count; ++i )
    {
	cw->menu.items = mip->next;
    	free( mip );
    }

    cw->menu.items = 0;
    cw->menu.count = 0;
    cw->menu.chr = 'a';

    if( cw->morestr ) free( cw->morestr );
    cw->morestr = NULL;

    if( window == WIN_INVEN && cw->win != NULL )
    {
	if( alwaysinvent )
	    cw->wasup = 1;
    }
    cw->cury = cw->rows = cw->maxrow = cw->maxcol = 0;
    return;
}

/* Add a string to a menu */
void
amii_add_menu(window,glyph, id, ch, gch, attr, str, preselected)
    register winid window;
    register int glyph;
    register const anything *id;
    register char ch;
    register char gch;
    register int attr;
    register const char *str;
    register BOOLEAN_P preselected;
{
    register struct amii_WinDesc *cw;
    amii_menu_item *mip;
    char buf[ QBUFSZ ];

    if(str == NULL)return;

    if(window == WIN_ERR || (cw = amii_wins[window]) == NULL || cw->type != NHW_MENU)
	panic(winpanicstr,window, "add_menu");

    mip = (amii_menu_item *)alloc( sizeof( *mip ) );
    mip->identifier = *id;
    mip->selected = preselected;
    mip->attr = attr;
    mip->glyph = Is_rogue_level(&u.uz) ? NO_GLYPH : glyph;
    mip->selector = 0;
    mip->gselector = gch;

    if (id->a_void && !ch && cw->menu.chr != 0)
    {
	ch = cw->menu.chr++;
	if( ch == 'z' )
	    cw->menu.chr = 'A';
	if( ch == 'Z' )
	    cw->menu.chr = 0;
    }

    mip->canselect = ( id->a_void != 0 );

    if( id->a_void && ch != '\0')
    {
	Sprintf( buf, "%c - %s", ch, str );
	str = buf;
	mip->canselect = 1;
    }

    mip->selector = ch;

    amii_putstr( window, attr, str );

    mip->str = cw->data[ cw->cury - 1 ];
    cw->menu.count++;

    mip->next = NULL;

    if( cw->menu.items == 0 )
    	cw->menu.last = cw->menu.items = mip;
    else
    {
	cw->menu.last->next = mip;
	cw->menu.last = mip;
    }
}

/* Done building a menu. */

void
amii_end_menu(window,morestr)
    register winid window;
    register const char *morestr;
{
    register struct amii_WinDesc *cw;

    if(window == WIN_ERR || (cw=amii_wins[window]) == NULL
       || cw->type != NHW_MENU )
	panic(winpanicstr,window, "end_menu");

    if( morestr && *morestr )
    {
	anything any;
	any.a_void = 0;
	amii_add_menu( window, NO_GLYPH, &any, 0, 0, ATR_NONE, morestr,
	   MENU_UNSELECTED);
    }

    if( morestr )
	cw->morestr = strdup( morestr );
}

/* Select something from the menu. */

int
amii_select_menu(window, how, mip )
    register winid window;
    register int how;
    register menu_item **mip;
{
    int cnt;
    register struct amii_WinDesc *cw;

    if( window == WIN_ERR || ( cw=amii_wins[window] ) == NULL ||
	  cw->type != NHW_MENU )
	panic(winpanicstr,window, "select_menu");

    cnt = DoMenuScroll( window, 1, how, mip );

    /* This would allow the inventory window to stay open. */
    if( !alwaysinvent || window != WIN_INVEN )
	dismiss_nhwindow(window);       /* Now tear it down */
    return cnt;
}

amii_menu_item *
find_menu_item( register struct amii_WinDesc *cw, int idx )
{
    amii_menu_item *mip;
    for( mip = cw->menu.items; idx > 0 && mip; mip = mip->next )
	--idx;

    return( mip );
}

int
make_menu_items( register struct amii_WinDesc *cw, register menu_item **rmip )
{
    register int idx = 0;
    register amii_menu_item *mip;
    register menu_item *mmip;

    for( mip = cw->menu.items; mip; mip = mip->next )
    {
	if( mip->selected )
	    ++idx;
    }

    if( idx )
    {
	mmip = *rmip = (menu_item *)alloc( idx * sizeof( *mip ) );
	for( mip = cw->menu.items; mip; mip = mip->next )
	{
	    if( mip->selected )
	    {
		mmip->item = mip->identifier;
		mmip->count = -1;
		mmip++;
	    }
	}

	cw->mi = *rmip;
    }
    return( idx );
}

int
DoMenuScroll( win, blocking, how, retmip )
    int win, blocking, how;
    menu_item **retmip;
{
    amii_menu_item *amip;
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
    int totalvis;
    SHORT mx, my;
    static char title[ 100 ];
    int dosize = 1;
    struct Screen *scrn = HackScreen;

    if( win == WIN_ERR || ( cw = amii_wins[ win ] ) == NULL )
	panic(winpanicstr,win,"DoMenuScroll");

    /*  Initial guess at window sizing values */
    txwd = txwidth;
    if( WINVERS_AMIV )
    {
	if( win == WIN_INVEN )
	    txh = max( txheight, pictdata.ysize + 3 ); /* interline space */
	else
	    txh = txheight; /* interline space */
    }
    else
	txh = txheight; /* interline space */

    /* Check to see if we should open the window, should need to for
     * TEXT and MENU but not MESSAGE.
     */

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
	    if( WINVERS_AMIV )
	    {
		if( win == WIN_INVEN )
		    xsize += pictdata.xsize + 4;
	    }
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
	    if( cw->type == NHW_MENU )
	    {
		if( nw->LeftEdge > 10 )
		    nw->LeftEdge -= 10;
		else
		    nw->LeftEdge = 0;
		if( amiIDisplay->ypix - nw->Height > 10 )
		    nw->TopEdge += 10;
		else
		    nw->TopEdge = amiIDisplay->ypix - nw->Height - 1;
	    }
	    if( cw->type == NHW_TEXT && xsize < amiIDisplay->xpix )
		nw->LeftEdge -= ( amiIDisplay->xpix - xsize ) / 2;
	}
	else if( win == WIN_INVEN )
	{
	    struct Window *mw = amii_wins[ WIN_MAP ]->win;
	    struct Window *sw = amii_wins[ WIN_STATUS ]->win;

	    xsize = scrn->WBorLeft + scrn->WBorRight + MenuScroll.Width + 1 +
				(txwd * cw->maxcol);

	    /* Make space for the glyph to appear at the left of the description */
	    if( WINVERS_AMIV )
		xsize += pictdata.xsize + 4;

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

	if( WINVERS_AMIV )
	{
	    /* Make sure we are using the correct hook structure */
	    nw->Extension = cw->wintags;
	}

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
	if( WINVERS_AMIV )
	{
	    if( win == WIN_INVEN )
		txh = max( w->RPort->TxHeight, pictdata.ysize + 3 ); /* interline space */
	    else
		txh = w->RPort->TxHeight; /* interline space */
	}
	else
	    txh = w->RPort->TxHeight; /* interline space */

	/* subtract 2 to account for spacing away from border (1 on each side) */
	wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) / txh;
	if( WINVERS_AMIV )
	{
	    if( win == WIN_INVEN )
	    {
		cw->cols = ( w->Width - w->BorderLeft -
			    w->BorderRight - 4 - pictdata.xsize - 3 ) / txwd;
	    }
	    else
	    {
		cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
	    }
   	}
	else
	{
	    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
	}
	totalvis = CountLines( win );
    }
    else
    {
	txwd = w->RPort->TxWidth;
	if( WINVERS_AMIV )
	{
	    if( win == WIN_INVEN )
		txh = max( w->RPort->TxHeight, pictdata.ysize + 3 ); /* interline space */
	    else
		txh = w->RPort->TxHeight; /* interline space */
	}
	else
	{
	    txh = w->RPort->TxHeight; /* interline space */
	}

	/* subtract 2 to account for spacing away from border (1 on each side) */
	wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) / txh;
	if( WINVERS_AMIV )
	{
	    if( win == WIN_INVEN )
	    {
		cw->cols = ( w->Width - w->BorderLeft -
			    w->BorderRight - 4 - pictdata.xsize - 3) / txwd;
	    }
	    else
		cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
	}
	else
	{
	    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
	}

	totalvis = CountLines( win );

	for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
	    gd = gd->NextGadget;

	if( gd )
	{
	    pip = (struct PropInfo *)gd->SpecialInfo;
	    hidden = max( totalvis - wheight, 0 );
	    topidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
	}
    }

    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
	gd = gd->NextGadget;

    if( !gd ) panic("Can't find scroll gadget" );

    morc = 0;
    oidx = -1;

#if 0
    /* Make sure there are no selections left over from last time. */
/* XXX potential problem for preselection if this is really needed */
  for( amip = cw->menu.items; amip; amip = amip->next )
	amip->selected = 0;
#endif

    DisplayData( win, topidx );

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

		    totalvis = CountLines( win );
		    wheight = ( w->Height - w->BorderTop -
						w->BorderBottom - 2) / txh;
		    if( WINVERS_AMIV )
		    {
			if( win == WIN_INVEN )
			{
			    cw->cols = ( w->Width - w->BorderLeft -
					w->BorderRight - 4 - pictdata.xsize - 3) / txwd;
			}
			else
			    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
		    }
		    else
		    {
			cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
		    }

		    if( wheight < 2 )
			wheight = 2;

		    /* Make the prop gadget the right size and place */

		    DisplayData( win, topidx );
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

			    DisplayData( win, topidx );
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
			DisplayData( win, topidx );
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
				DisplayData( win, topidx );
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
				DisplayData( win, topidx );
				SetPropInfo( w, gd, wheight,
						    totalvis, topidx );
			    }
			    oldsecs = oldmics = 0;
			}
		    }
		    else if( code == '\33' )
		    {
			morc = '\33';
			aredone = 1;
		    }
		    else
		    {
			for( amip = cw->menu.items; amip; amip = amip->next )
			{
			    if( amip->selector == code )
			    {
				if( how == PICK_ONE )
				    aredone = 1;
				amip->selected = !amip->selected;
			    } else if (amip->gselector == code )
			    {
				amip->selected = !amip->selected;
			    }
			}
			DisplayData( win, topidx );
			morc = code;
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
		    totalvis = CountLines( win );
		    hidden = max( totalvis - wheight, 0 );
		    aidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
		    if( aidx != topidx )
			DisplayData( win, topidx = aidx );
		    break;

		case MOUSEMOVE:
		    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			gd = gd->NextGadget;

		    pip = (struct PropInfo *)gd->SpecialInfo;
		    totalvis = CountLines( win );
		    hidden = max( totalvis - wheight, 0 );
		    aidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
		    if( aidx != topidx )
			DisplayData( win, topidx = aidx );
		    break;

		case INACTIVEWINDOW:
		    if( win == WIN_MESSAGE || ( win == WIN_INVEN && alwaysinvent ) )
			aredone = 1;
		    break;

		case MOUSEBUTTONS:
		    if( ( code == SELECTUP || code == SELECTDOWN ) &&
					cw->type == NHW_MENU && how != PICK_NONE )
		    {
			/* Which one is the mouse pointing at? */

			aidx = ( ( my - w->BorderTop - 1 ) / txh ) + topidx;

			/* If different lines, don't select double click */

			if( aidx != oidx )
			{
			    oldsecs = 0;
			    oldmics = 0;
			}

			/* If releasing, check for double click */

			if( code == SELECTUP )
			{
			    amip = find_menu_item( cw, aidx );
			    if( aidx == oidx )
			    {
				if( DoubleClick( oldsecs,
						    oldmics, secs, mics ) )
				{
				    aredone = 1;
				}
				oldsecs = secs;
				oldmics = mics;
			    }
			    else
			    {
				amip = find_menu_item( cw, oidx );
				amip->selected = 0;
			    }
			    DisplayData( win, topidx );
			}
			else if( aidx - topidx < wheight &&
				aidx < cw->maxrow && code == SELECTDOWN )
			{
			    /* Remove old highlighting if visible */

			    amip = find_menu_item( cw, oidx );
			    if( amip && oidx != aidx &&
				    ( oidx > topidx && oidx - topidx < wheight ) )
			    {
				if( how != PICK_ANY )
				    amip->selected = 0;
				oidx = -1;
			    }
			    amip = find_menu_item( cw, aidx );

			    if( amip && amip->canselect && how != PICK_NONE )
			    {
				oidx = aidx;
				if( !DoubleClick( oldsecs,
						    oldmics, secs, mics ) )
				{
				    amip->selected = !amip->selected;
				}
			    }
			    else
			    {
				DisplayBeep( NULL );
				oldsecs = 0;
				oldmics = 0;
			    }
			    DisplayData( win, topidx );
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
    return( make_menu_items( cw, retmip ) );
}

void
ReDisplayData( win )
    winid win;
{
    int totalvis;
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

    wheight = (w->Height - w->BorderTop - w->BorderBottom-2)/w->RPort->TxHeight;

    pip = (struct PropInfo *)gd->SpecialInfo;
    totalvis = CountLines( win );
    hidden = max( totalvis - wheight, 0 );
    aidx = (((ULONG)hidden * pip->VertPot) + (MAXPOT/2)) >> 16;
    clear_nhwindow( win );
    DisplayData( win, aidx );
}

long
FindLine( win, line )
    winid win;
    int line;
{
    int txwd;
    register char *t;
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register int i, disprow, len;
    int col = -1;

    if( win == WIN_ERR || !(cw = amii_wins[win]) || !( w = cw->win ) )
    {
	panic( winpanicstr, win, "No Window in FindLine" );
    }
    txwd = w->RPort->TxWidth;
    if( WINVERS_AMIV )
    {
	if( win == WIN_INVEN )
	{
	    cw->cols = ( w->Width - w->BorderLeft -
			w->BorderRight - 4 - pictdata.xsize - 3) / txwd;
	}
	else
	    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
    }
    else
    {
	cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
    }

    disprow = 0;
    for( col = i = 0; line > disprow && i < cw->maxrow; ++i )
    {
	t = cw->data[ i ] + SOFF;
	if( cw->data[i][1] >= 0 )
	{
	    ++disprow;
	    col = 0;
	}

	while( *t )
	{
	    len = strlen( t );
	    if( col + len > cw->cols )
		len = cw->cols - col;
	    while( len > 0 )
	    {
	    	if( !t[len] || t[len] == ' ' )
		    break;
	    	--len;
	    }
	    if( len == 0 )
		len = strlen(t);
	    t += len;
	    col += len;
	    if( *t )
	    {
		while( *t == ' ' )
		    ++t;
		col = 0;
		++disprow;
	    }
	}
    }
    return( i );
}

long
CountLines( win )
    winid win;
{
    int txwd;
    amii_menu_item *mip;
    register char *t;
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register int i, disprow, len;
    int col = -1;

    if( win == WIN_ERR || !(cw = amii_wins[win]) || !( w = cw->win ) )
    {
	panic( winpanicstr, win, "No Window in CountLines" );
    }

    txwd = w->RPort->TxWidth;
    if( WINVERS_AMIV )
    {
	if( win == WIN_INVEN )
	{
	    cw->cols = ( w->Width - w->BorderLeft -
			w->BorderRight - 4 - pictdata.xsize - 3) / txwd;
	}
	else
	    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
    }
    else
    {
	cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
    }

    disprow = cw->maxrow;
    mip = cw->menu.items;
    for( col = i = 0; i < cw->maxrow; ++i )
    {
	t = cw->data[ i ] + SOFF;
	if( cw->type == NHW_MESSAGE && cw->data[ i ][ SEL_ITEM ] < 0 )
	    --disprow;
	else
	    col = 0;
	while( *t )
	{
	    len = strlen( t );
	    if( col + len > cw->cols )
		len = cw->cols - col;
	    while( len > 0 && t[len] )
	    {
	    	if( t[len] == ' ' )
		    break;
	    	--len;
	    }
	    if( len == 0 )
		len = strlen(t);
	    t += len;
	    col += len;
	    if( *t )
	    {
		while( *t == ' ' )
		    ++t;
		col = 0;
		++disprow;
	    }
	}
    }
    return( disprow );
}

void
DisplayData( win, start )
    winid win;
    int start;
{
    int txwd;
    amii_menu_item *mip;
    register char *t;
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register struct RastPort *rp;
    register int i, disprow, len, wheight;
    int whichcolor = -1;
    int col;

    if( win == WIN_ERR || !(cw = amii_wins[win]) || !( w = cw->win ) )
    {
	panic( winpanicstr, win, "No Window in DisplayData" );
    }

    rp = w->RPort;
    SetDrMd( rp, JAM2 );
    if( WINVERS_AMIV && win == WIN_INVEN )
    {
	wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) /
		    max( rp->TxHeight, pictdata.ysize + 3 );
    }
    else
    {
	wheight = ( w->Height - w->BorderTop - w->BorderBottom - 2 ) /
		    rp->TxHeight;
    }

    cw->rows = wheight;
    txwd = rp->TxWidth;
    if( WINVERS_AMIV )
    {
	if( win == WIN_INVEN )
	{
	    cw->cols = ( w->Width - w->BorderLeft -
			w->BorderRight - 4 - pictdata.xsize - 3) / txwd;
	}
	else
	    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
    }
    else
    {
	cw->cols = ( w->Width - w->BorderLeft - w->BorderRight - 4 ) / txwd;
    }

    mip = cw->menu.items;
    for( i = 0; mip && i < start; ++i )
    {
	mip = mip->next;
    }
    /* Get the real line to display at */
    start = FindLine( win, start );

    /* Skip any initial response to a previous line */
    if( cw->type == NHW_MESSAGE && mip && mip->selected < 0 )
	++start;
    if( WINVERS_AMIV && cw->type == NHW_MESSAGE )
	SetAPen( rp, amii_msgAPen );
    
    for( disprow = i = start; disprow < wheight + start; i++ )
    {
	/* Just erase unused lines in the window */
    	if( i >= cw->maxrow )
    	{
	    if (WINVERS_AMIV){
		if(win == WIN_INVEN){
		    amii_curs( win, 0, disprow - start );
		    amiga_print_glyph( win, 0, NO_GLYPH);
		}
	    }
	    amii_curs( win, 1, disprow - start );
	    amii_cl_end( cw, 0 );
	    ++disprow;
	    continue;
    	}

	/* Any string with a highlighted attribute goes
	 * onto the end of the current line in the message window.
	 */
	if( cw->type == NHW_MESSAGE )
	    SetAPen( rp, cw->data[ i ][ SEL_ITEM ] < 0 ? C_RED : amii_msgAPen );

	/* Selected text in the message window goes onto the end of the current line */
	if( cw->type != NHW_MESSAGE || cw->data[ i ][ SEL_ITEM ] >= 0 )
	{
	    amii_curs( win, 1, disprow - start );
	    if( WINVERS_AMIV )
	    {
		if( mip )
		    amiga_print_glyph( win, 0, mip->glyph );
		amii_curs( win, 1, disprow - start );
	    }
	    col = 0;
	}

	/* If this entry is to be highlighted, do so */
	if( mip && mip->selected != 0 )
	{
	    if( whichcolor != 1 )
	    {
		SetDrMd( rp, JAM2 );
		if( WINVERS_AMIV )
		{
		    SetAPen( rp, amii_menuBPen );
		    SetBPen( rp, C_BLUE );
		}
		else
		{
		    SetAPen( rp, C_BLUE );
		    SetBPen( rp, amii_menuAPen );
		}
		whichcolor = 1;
	    }
	}
	else if( whichcolor != 2 )
	{
	    SetDrMd( rp, JAM2 );
	    if( cw->type == NHW_MESSAGE )
	    {
		SetAPen( rp, amii_msgAPen );
		SetBPen( rp, amii_msgBPen );
	    }
	    else if( cw->type == NHW_MENU )
	    {
		SetAPen( rp, amii_menuAPen );
		SetBPen( rp, amii_menuBPen );
	    }
	    else if( cw->type == NHW_TEXT )
	    {
		SetAPen( rp, amii_textAPen );
		SetBPen( rp, amii_textBPen );
	    }
	    whichcolor = 2;
	}

	/* Next line out, wrap if too long */

	t = cw->data[ i ] + SOFF;
	++disprow;
	col = 0;
	while( *t )
	{
	    len = strlen( t );
	    if( len > cw->cols )
		len = cw->cols;
	    while( len > 0 )
	    {
	    	if( !t[len] || t[len] == ' ' )
		    break;
	    	--len;
	    }
	    if( len == 0 )
		len = strlen(t);
	    Text( rp, t, len );
	    t += len;
	    col += len;
	    amii_cl_end( cw, col );
	    if( *t )
	    {
		++disprow;
		/* Stop at the bottom of the window */
		if( disprow >= wheight + start )
		    break;
		while( *t == ' ' )
		    ++t;
		amii_curs( win, 1, disprow - start - 1 );
		if( mip && win == WIN_INVEN )
		{
		    /* Erase any previous glyph drawn here. */
		    amiga_print_glyph( win, 0, NO_GLYPH );
		    amii_curs( win, 1, disprow - start - 1 );
		}
		Text( rp, "+", 1 );
		col = 1;
	    }
	}

	if( cw->type == NHW_MESSAGE )
	{
	    SetAPen( rp, amii_msgBPen );
	    SetBPen( rp, amii_msgBPen );
	}
	else if( cw->type == NHW_MENU )
	{
	    SetAPen( rp, amii_menuBPen );
	    SetBPen( rp, amii_menuBPen );
	}
	else if( cw->type == NHW_TEXT )
	{
	    SetAPen( rp, amii_textBPen );
	    SetBPen( rp, amii_textBPen );
	}
	amii_cl_end( cw, col );
	whichcolor = -1;
	if( mip ) mip = mip->next;
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
