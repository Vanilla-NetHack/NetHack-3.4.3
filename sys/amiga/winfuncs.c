/*    SCCS Id: @(#)winfuncs.c    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "amiga:windefs.h"
#include "amiga:winext.h"
#include "amiga:winproto.h"
#include "incl:patchlevel.h"

int topl_addspace=1;
extern struct TagItem scrntags[];

void
amii_destroy_nhwindow(win)      /* just hide */
    register winid win;
{
    register struct amii_WinDesc *cw;

    if( win == WIN_ERR || ( cw = amii_wins[win] ) == NULL )
    {
	panic(winpanicstr,win,"destroy_nhwindow");
    }

#ifdef	VIEWWINDOW
    if( cw->type == NHW_MAP )
    {
	amii_destroy_nhwindow( WIN_VIEW );
	amii_destroy_nhwindow( WIN_VIEWBOX );

	/* If inventory is up, close it now, it will be freed later */
	if( alwaysinvent && WIN_INVEN != WIN_ERR &&
			    amii_wins[ WIN_INVEN ] &&
			    amii_wins[ WIN_INVEN ]->win )
	{
	    dismiss_nhwindow( WIN_INVEN );
	}
    }
#endif

    /* Tear down the Intuition stuff */
    dismiss_nhwindow(win);

    if( cw->resp )
	free( cw->resp );
    cw->resp = NULL;

    if( cw->canresp )
	free( cw->canresp );
    cw->canresp = NULL;

    if( cw->morestr )
	free( cw->morestr );
    cw->morestr = NULL;

    free( cw );
    amii_wins[win] = NULL;
}

amii_create_nhwindow(type)
    register int type;
{
    register struct Window *w = NULL;
    register struct NewWindow *nw = NULL;
    register struct amii_WinDesc *wd = NULL;
    struct Window *mapwin = NULL, *stwin = NULL, *msgwin = NULL;
    register int newid;
    int maph;

    maph = ( 22 * MAPFTHEIGHT ) + HackScreen->WBorTop +
			HackScreen->WBorBottom + MAPFTHEIGHT + 1 + 1;
    if( WIN_STATUS != WIN_ERR && amii_wins[ WIN_STATUS ] )
	stwin = amii_wins[ WIN_STATUS ]->win;

    if( WIN_MESSAGE != WIN_ERR && amii_wins[ WIN_MESSAGE ] )
	msgwin = amii_wins[ WIN_MESSAGE ]->win;

    if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
	mapwin = amii_wins[ WIN_MAP ]->win;

    /* Create Port anytime that we need it */

    if( HackPort == NULL )
    {
	HackPort = CreatePort( NULL, 0 );
	if( !HackPort )
	    panic( "no memory for msg port" );
    }

    nw = &new_wins[ type ].newwin;
    nw->Width = amiIDisplay->xpix;
    nw->Screen = HackScreen;

    nw->DetailPen = C_BLACK;
    nw->BlockPen = C_WHITE;

    if( type == NHW_MAP || type == NHW_BASE )
    {
	nw->LeftEdge = 0;

	if(
#ifndef	VIEWWINDOW
		bigscreen &&
#endif
		type == NHW_MAP )
	{
	    nw->Height = maph;
	    if( msgwin && stwin )
	    {
		nw->TopEdge = msgwin->TopEdge + msgwin->Height +
			    ((stwin->TopEdge-
			    (msgwin->TopEdge + msgwin->Height)-maph))/2;
	    }
	    else
	    {
		panic( "msgwin and stwin must open before map" );
	    }
#ifdef	VIEWWINDOW
	    if( !bigscreen )
		nw->Flags &= ~(BORDERLESS);
	    nw->Width = (MAPFTWIDTH * 80) + HackScreen->WBorLeft +
						HackScreen->WBorRight;
#else
	    nw->Width = HackScreen->Width;
#endif
	}
	else
	{
	    nw->TopEdge = 1;
	    nw->Height = amiIDisplay->ypix - nw->TopEdge;
	}
    }
#ifdef	VIEWWINDOW
    else if( type == NHW_VIEWBOX )
    {
	struct Window *w = amii_wins[ NHW_MAP ]->win;

	nw->LeftEdge = w->Width;
	nw->TopEdge = w->TopEdge;
	nw->Width = amiIDisplay->xpix - nw->LeftEdge;
	if( msgwin && stwin )
	{
	    nw->Height = stwin->TopEdge - nw->TopEdge;
	}
	else
	{
	    nw->Height = 10 * VIEWCHARHEIGHT +
		    w->BorderTop + w->BorderBottom;
	}
	nw->MaxHeight = VIEWCHARHEIGHT*24;
	nw->MaxWidth = VIEWCHARWIDTH*80;
	if( nw->TopEdge + nw->Height > amiIDisplay->ypix - 1 )
	    nw->Height = amiIDisplay->ypix - nw->TopEdge - 1;
	if( !bigscreen )
	    nw->Flags &= ~(WINDOWSIZING|WINDOWDRAG|WINDOWDEPTH|WINDOWCLOSE);
    }
    else if( type == NHW_VIEW )
    {
	struct Window *vw = amii_wins[ WIN_VIEWBOX ]->win;
	int i;

	nw->LeftEdge = vw->LeftEdge + vw->BorderLeft;
	nw->TopEdge = vw->TopEdge + vw->BorderTop;
	nw->Width = amiIDisplay->xpix - nw->LeftEdge - 1 - vw->BorderRight;
	nw->Height = vw->Height - vw->BorderTop - vw->BorderBottom;
	nw->MaxHeight = VIEWCHARHEIGHT*24;
	nw->MaxWidth = VIEWCHARWIDTH*80;
	if( nw->TopEdge + nw->Height > amiIDisplay->ypix - 1 )
	    nw->Height = amiIDisplay->ypix - nw->TopEdge - 1;
	InitBitMap( &amii_vbm, DEPTH, VIEWCHARWIDTH * 88, VIEWCHARHEIGHT * 30 );
	for( i = 0; i < DEPTH; ++i )
	{
	    if( ( amii_vbm.Planes[i] = AllocRaster( VIEWCHARWIDTH * 88,
							VIEWCHARHEIGHT * 30 ) ) == 0 )
	    {
		panic( "can't allocate bitmap for view window" );
	    }
	    memset( amii_vbm.Planes[i], 0,
	    			RASSIZE( VIEWCHARWIDTH * 88, VIEWCHARHEIGHT * 30 ) );
	}
	nw->Flags |= SUPER_BITMAP;
	nw->BitMap = &amii_vbm;
    }
#endif
    else if( type == NHW_STATUS )
    {
#ifndef	VIEWWINDOW
	if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
	    w = amii_wins[ WIN_MAP ]->win;
	else
#endif
	if( WIN_BASE != WIN_ERR && amii_wins[ WIN_BASE ] )
	    w = amii_wins[ WIN_BASE ]->win;
	else
	    panic( "No window to base STATUS location from" );

	/* Status window is relative to bottom of WIN_BASE/WIN_MAP */

	/* Expand the height of window by borders */
	nw->Height = (txheight * 2) + 4;
	if( bigscreen )
	{
	    nw->Height += txheight + w->WScreen->WBorTop + 1 + w->WScreen->WBorBottom;
	}

	nw->TopEdge = amiIDisplay->ypix - nw->Height - 1;
	nw->LeftEdge = w->LeftEdge;
	if( nw->LeftEdge + nw->Width >= amiIDisplay->xpix )
	    nw->LeftEdge = 0;
	if( nw->Width >= amiIDisplay->xpix - nw->LeftEdge )
	    nw->Width = amiIDisplay->xpix - nw->LeftEdge;
    }
    else if( type == NHW_MESSAGE )
    {
#ifndef	VIEWWINDOW
	if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
	    w = amii_wins[ WIN_MAP ]->win;
	else
#endif
	if( WIN_BASE != WIN_ERR && amii_wins[ WIN_BASE ] )
	    w = amii_wins[ WIN_BASE ]->win;
	else
	    panic( "No window to base STATUS location from" );

	nw->TopEdge = 1;
	nw->Height = HackScreen->WBorTop + 1 +
		((txheight+1)*(1+(scrollmsg != 0))) + 1 + HackScreen->WBorBottom;
	if( ( HackScreen->Height - 1 -
		( ( TextsFont->tf_YSize +
		    HackScreen->WBorTop + 1 + HackScreen->WBorBottom ) * 2 ) -
		( ( TextsFont->tf_YSize + 1 ) * 2 ) - 2 ) - maph <
		    TextsFont->tf_YSize + 3 )
	{
	    scrollmsg = 0;
	}
	if( scrollmsg )
	{
	    nw->FirstGadget = &MsgScroll;
	    nw->Height = HackScreen->Height - 1 -
		/* Size of all borders for bigscreen */
		( ( TextsFont->tf_YSize + HackScreen->WBorTop + 1 +
					HackScreen->WBorBottom ) * 2 ) -
		/* Text space in status window */
		( ( TextsFont->tf_YSize + 1 ) * 2 ) - 2 -
		maph;
	    nw->Flags |= WINDOWSIZING|WINDOWDRAG;
	}
#ifdef  INTUI_NEW_LOOK
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	{
	    MsgPropScroll.Flags |= PROPNEWLOOK;
	}
#endif
	/* Just allow height adjustments */
	nw->MinWidth = w->Width;
	nw->MinHeight = HackScreen->WBorTop +
		HackScreen->WBorBottom + ((TextsFont->tf_YSize+1)*2) + 3;
    }

    nw->IDCMPFlags |= MENUPICK;

    /* Check if there is "Room" for all this stuff... */
    if( bigscreen && type != NHW_BASE && type != NHW_VIEW )
    {
	nw->Flags &= ~( BORDERLESS | BACKDROP );
#ifdef	VIEWWINDOW
	nw->Flags |= ( WINDOWDRAG | WINDOWDEPTH | SIZEBRIGHT );
#else
	if( HackScreen->Width < 657 )
	{
	    nw->Flags |= ( WINDOWDRAG | WINDOWDEPTH );
	}
	else
	{
	    nw->Flags |= ( WINDOWDRAG | WINDOWDEPTH | SIZEBRIGHT );
	    if( type == NHW_MAP )
		nw->Flags |= WINDOWSIZING;
	}
#endif

/*#endif*/
#ifdef	VIEWWINDOW
	if( type == NHW_VIEWBOX )
	    nw->Flags |= WINDOWSIZING;
#endif
    }

    /* No titles on a hires only screen */
    if( !bigscreen )
	nw->Title = 0;

    /* Don't open MENU or TEXT windows yet */

    if( type == NHW_MENU || type == NHW_TEXT )
	w = NULL;
    else
	w=OpenShWindow( (void *)nw );

    if( w == NULL && type != NHW_MENU && type != NHW_TEXT )
    {
	char buf[ 100 ];

	sprintf( buf, "nw(%d) is l: %d, t: %d, w: %d, h: %d",
		type,
		nw->LeftEdge, nw->TopEdge,
		nw->Width, nw->Height );
	raw_print( buf );
	panic("bad openwin %d",type);
    }

    /* Check for an empty slot */

    for(newid = 0; newid<MAXWIN + 1; newid++)
    {
	if(amii_wins[newid] == 0)
	    break;
    }

    if(newid==MAXWIN+1)
	panic("time to write re-alloc code\n");

    /* Set wincnt accordingly */

    if( newid > wincnt )
	wincnt = newid;

    /* Do common initialization */

    wd = (struct amii_WinDesc *)alloc(sizeof(struct amii_WinDesc));
    memset( wd, 0, sizeof( struct amii_WinDesc ) );
    amii_wins[newid] = wd;

    wd->newwin = NULL;
    wd->win = w;
    wd->type = type;
    wd->wflags = 0;
    wd->active = FALSE;
    wd->curx=wd->cury = 0;
    wd->resp = wd->canresp = wd->morestr = 0;   /* CHECK THESE */
    wd->maxrow = new_wins[type].maxrow;
    wd->maxcol = new_wins[type].maxcol;

    if( type != NHW_TEXT && type != NHW_MENU )
    {
	if( TextsFont && ( type == NHW_MESSAGE || type == NHW_STATUS ) )
	{
	    SetFont(w->RPort, TextsFont);
	    txheight = w->RPort->TxHeight;
	    txwidth = w->RPort->TxWidth;
	    txbaseline = w->RPort->TxBaseline;
	    if( type == NHW_MESSAGE )
	    {
		if( scrollmsg )
		{
		    WindowLimits( w, w->Width, w->BorderTop +
				w->BorderBottom +
				((txheight+1)*3) + 1, 0, 0 );
		}
		else
		{
		    WindowLimits( w, w->Width, w->BorderTop +
				w->BorderBottom +
				txheight + 2, 0, 0 );
		}
	    }
	}
#ifdef HACKFONT
	else if( HackFont )
	    SetFont(w->RPort, HackFont);
#endif
	wd->rows = ( w->Height - w->BorderTop -
		w->BorderBottom - 2 ) / w->RPort->TxHeight;
	wd->cols = ( w->Width - w->BorderLeft -
		w->BorderRight - 2 ) / w->RPort->TxWidth;
    }

    /* Okay, now do the individual type initialization */

    switch(type)
    {
	/* History lines for MESSAGE windows are stored in cw->data[?].
	 * maxcol and maxrow are used as cursors.  maxrow is the count
	 * of the number of history lines stored.  maxcol is the cursor
	 * to the last line that was displayed by ^P.
	 */
	case NHW_MESSAGE:
	    SetMenuStrip(w, HackMenu);
#ifdef	VIEWWINDOW
	    if(flags.msg_history<20)flags.msg_history=20;
#else
	    if(flags.msg_history<40)flags.msg_history=40;
#endif
	    if(flags.msg_history>400)flags.msg_history=400;
	    flags.window_inited=TRUE;
	    wd->data = (char **)alloc( flags.msg_history*sizeof( char * ) );
	    memset( wd->data, 0, flags.msg_history * sizeof( char * ) );
	    wd->maxrow = wd->maxcol = 0;
	    /* Indicate that we have not positioned the cursor yet */
	    wd->curx = -1;
	    break;

	    /* A MENU contains a list of lines in wd->data[?].  These
	     * lines are created in amii_putstr() by reallocating the size
	     * of wd->data to hold enough (char *)'s.  wd->rows is the
	     * number of (char *)'s allocated.  wd->maxrow is the number
	     * used.  wd->maxcol is used to track how wide the menu needs
	     * to be.  wd->resp[x] contains the characters that correspond
	     * to selecting wd->data[x].  wd->resp[x] corresponds to
	     * wd->data[x] for any x. Elements of wd->data[?] that are not
	     * valid selections have the corresponding element of
	     * wd->resp[] set to a value of '\01';  i.e. a ^A which is
	     * not currently a valid keystroke for responding to any
	     * MENU or TEXT window.
	     */
	case NHW_MENU:
	    wd->resp=(char*)alloc(256);
	    wd->resp[0]=0;
	    wd->rows = wd->maxrow = 0;
	    wd->cols = wd->maxcol = 0;
	    wd->data = NULL;
	    break;

	    /* See the explanation of MENU above.  Except, wd->resp[] is not
	     * used for TEXT windows since there is no selection of a
	     * a line performed/allowed.  The window is always full
	     * screen width.
	     */
	case NHW_TEXT:
	    wd->rows = wd->maxrow = 0;
	    wd->cols = wd->maxcol = amiIDisplay->cols;
	    wd->data = NULL;
	    wd->morestr = NULL;
	    break;

	    /* The status window has only two lines.  These are stored in
	     * wd->data[], and here we allocate the space for them.
	     */
	case NHW_STATUS:
	    SetMenuStrip(w, HackMenu);
	    /* wd->cols is the number of characters which fit across the
	     * screen.
	     */
	    wd->data=(char **)alloc(3*sizeof(char *));
	    wd->data[0] = (char *)alloc(wd->cols + 10);
	    wd->data[1] = (char *)alloc(wd->cols + 10);
	    wd->data[2] = NULL;
	    break;

	    /* NHW_MAP does not use wd->data[] or the other text
	     * manipulating members of the amii_WinDesc structure.
	     */
	case NHW_MAP:
	    SetMenuStrip(w, HackMenu);
#ifdef	VIEWWINDOW
	    WIN_VIEWBOX = amii_create_nhwindow( NHW_VIEWBOX );
	    WIN_VIEW = amii_create_nhwindow( NHW_VIEW );
	    if( HackFont4 )
		SetFont( w->RPort, HackFont4 );
#else
	    if( HackFont )
		SetFont( w->RPort, HackFont );
#endif
	    break;

	    /* The base window must exist until CleanUp() deletes it. */
	case NHW_BASE:
	    SetMenuStrip(w, HackMenu);
	    /* Make our requesters come to our screen */
	    {
		register struct Process *myProcess =
					(struct Process *) FindTask(NULL);
		pr_WindowPtr = (struct Window *)(myProcess->pr_WindowPtr);
		myProcess->pr_WindowPtr = (APTR) w;
	    }

	    /* Need this for RawKeyConvert() */

	    ConsoleIO.io_Data = (APTR) w;
	    ConsoleIO.io_Length = sizeof( struct Window );
	    ConsoleIO.io_Message.mn_ReplyPort = CreatePort(NULL, 0L);
	    if( OpenDevice("console.device", -1L,
				(struct IORequest *) &ConsoleIO, 0L) != 0)
	    {
		Abort(AG_OpenDev | AO_ConsoleDev);
	    }

	    ConsoleDevice = (struct Library *) ConsoleIO.io_Device;

	    KbdBuffered = 0;

#ifdef HACKFONT
	    if( TextsFont )
		SetFont( w->RPort, TextsFont );
	    else if( HackFont )
		SetFont( w->RPort, HackFont );
#endif
	    txwidth = w->RPort->TxWidth;
	    txheight = w->RPort->TxHeight;
	    txbaseline = w->RPort->TxBaseline;
	    break;

#ifdef	VIEWWINDOW
	case NHW_VIEWBOX:
	    /* Position BitMap at zero, zero */
	    ScrollLayer( 0, w->RPort->Layer,
				-w->RPort->Layer->Scroll_X,
				  -w->RPort->Layer->Scroll_Y );
	case NHW_VIEW:
	    if( HackFont16 )
		SetFont( w->RPort, HackFont16 );
	    wd->curx = -1;
	    wd->cury = -1;
	    SetMenuStrip(w, HackMenu);
	    break;
#endif

	default:
	    panic("bad create_nhwindow( %d )\n",type);
	    return WIN_ERR;
    }

    return( newid );
}

/* Initialize the windowing environment */

void
amii_init_nhwindows()
{
    int forcenobig = 0;

    if (HackScreen)
	panic( "init_nhwindow() called twice", 0 );

    WIN_MESSAGE = WIN_ERR;
    WIN_MAP = WIN_ERR;
    WIN_STATUS = WIN_ERR;
    WIN_INVEN = WIN_ERR;
    WIN_BASE = WIN_ERR;

#ifndef	SHAREDLIB
    if ( (IntuitionBase = (struct IntuitionBase *)
	  OpenLibrary("intuition.library", INTUITION_VERSION)) == NULL)
    {
	Abort(AG_OpenLib | AO_Intuition);
    }

    if ( (GfxBase = (struct GfxBase *)
	      OpenLibrary("graphics.library", GRAPHICS_VERSION)) == NULL)
    {
	Abort(AG_OpenLib | AO_GraphicsLib);
    }

#ifdef	VIEWWINDOW
    if ( (LayersBase = (struct Library *)
		OpenLibrary("layers.library", 0)) == NULL)
    {
	Abort(AG_OpenLib | AO_LayersLib);
    }
#endif
#endif
    amiIDisplay=(struct amii_DisplayDesc *)alloc(sizeof(struct amii_DisplayDesc));
    memset( amiIDisplay, 0, sizeof( struct amii_DisplayDesc ) );

    /* Use Intuition sizes for overscan screens... */

    amiIDisplay->ypix = GfxBase->NormalDisplayRows;
    amiIDisplay->xpix = GfxBase->NormalDisplayColumns;

    amiIDisplay->cols = amiIDisplay->xpix / FONTWIDTH;

    amiIDisplay->toplin=0;
    amiIDisplay->rawprint=0;
    amiIDisplay->lastwin=0;

    if( bigscreen == 0 )
    {
	if( ( GfxBase->ActiView->ViewPort->Modes & LACE ) == LACE )
	{
	    amiIDisplay->ypix *= 2;
	    NewHackScreen.ViewModes |= LACE;
	    bigscreen = 1;
	}
	else if( GfxBase->NormalDisplayRows >= 270 )
	{
	    bigscreen = 1;
	}
    }
    else if( bigscreen == -1 )
    {
	bigscreen = 0;
	forcenobig = 1;
    }
    else if( bigscreen )
    {
	/* If bigscreen requested and we don't have enough rows in
	 * noninterlaced mode, switch to interlaced...
	 */
	if( GfxBase->NormalDisplayRows < 270 )
	{
	    amiIDisplay->ypix *= 2;
	    NewHackScreen.ViewModes |= LACE;
	}
    }

    if( !bigscreen )
    {
    	scrollmsg = 0;
    	alwaysinvent = 0;
    }
    amiIDisplay->rows = amiIDisplay->ypix / FONTHEIGHT;

#ifdef HACKFONT
    /*
     *  Load the fonts that we need.
     */

    if( DiskfontBase =
		OpenLibrary( "diskfont.library", DISKFONT_VERSION ) )
    {
	Hack80.ta_Name -= SIZEOF_DISKNAME;
	HackFont = OpenDiskFont( &Hack80 );
	Hack80.ta_Name += SIZEOF_DISKNAME;

#ifdef	VIEWWINDOW
	Hack40.ta_Name -= SIZEOF_DISKNAME;
	HackFont4 = OpenDiskFont( &Hack40 );
	Hack40.ta_Name += SIZEOF_DISKNAME;

	Hack160.ta_Name -= SIZEOF_DISKNAME;
	HackFont16 = OpenDiskFont( &Hack160 );
	Hack160.ta_Name += SIZEOF_DISKNAME;
#endif
	/* Textsfont13 is filled in with "FONT=" settings. The default is
	 * courier/13.
	 */
	TextsFont = NULL;
	if( bigscreen )
	    TextsFont = OpenDiskFont( &TextsFont13 );

	/* Try hack/8 for texts if no user specified font */
	if( TextsFont == NULL )
	{
	    Hack80.ta_Name -= SIZEOF_DISKNAME;
	    TextsFont = OpenDiskFont( &Hack80 );
	    Hack80.ta_Name += SIZEOF_DISKNAME;
	}

	/* If no fonts, make everything topaz 8 for non-view windows.
	 */
	if( !HackFont || !TextsFont )
	{
	    Hack80.ta_Name = "topaz.font";
	    if( !HackFont )
	    {
		HackFont = OpenFont( &Hack80 );
		if( !HackFont )
		    panic( "Can't get a map font, topaz:8" );
	    }

	    if( !TextsFont )
	    {
		TextsFont = OpenFont( &Hack80 );
		if( !TextsFont )
		    panic( "Can't open text font" );
	    }
	}
#ifdef	VIEWWINDOW
	/*
	 * These other fonts are required for the view windows, so
	 * we have to "panic".
	 */
	if( !HackFont4 || !HackFont16 )
	{
	    panic( "Can't open all hack/4 or hack/16 font" );
	}
#endif
	CloseLibrary(DiskfontBase);
	DiskfontBase = NULL;
    }
#endif

    /* This is the size screen we want to open, within reason... */

    NewHackScreen.Width = max( WIDTH, amiIDisplay->xpix );
    NewHackScreen.Height = max( SCREENHEIGHT, amiIDisplay->ypix );
    {
    static char fname[18];
    sprintf(fname,"NetHack %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
    NewHackScreen.DefaultTitle=fname;
    }
    NewHackScreen.BlockPen = C_CYAN;
#ifdef	INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
    	int i;
    	long modeid;
	struct Screen *wbscr;

	for( i = 0; scrntags[i].ti_Tag != SA_DisplayID &&
		    scrntags[i].ti_Tag != TAG_DONE; ++i )
	{
	    continue;
	}

	NewHackScreen.Width = STDSCREENWIDTH;
	NewHackScreen.Height = STDSCREENHEIGHT;

	if( forcenobig == 0 )
	{
	    wbscr = LockPubScreen( "Workbench" );
	    if( wbscr )
		modeid = GetVPModeID( &wbscr->ViewPort ); 

	    if( ( wbscr != NULL ) && ( modeid != INVALID_ID ) )
	    {
		if( wbscr->ViewPort.Modes & LACE )
		    NewHackScreen.ViewModes |= LACE;
		UnlockPubScreen( NULL, wbscr );
		if( scrntags[i].ti_Tag == SA_DisplayID )
		{
		    scrntags[i].ti_Data = (ULONG)modeid;
		}
	    }
	}
	else
	{
	    if( scrntags[i].ti_Tag == SA_DisplayID )
	    {
		scrntags[i].ti_Tag = TAG_IGNORE;
	    }
	}
    }
#endif

    if( ( HackScreen = OpenScreen( (void *)&NewHackScreen ) ) == NULL )
	Abort( AN_OpenScreen & ~AT_DeadEnd );
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
	PubScreenStatus( HackScreen, 0 );
#endif

    amiIDisplay->ypix = HackScreen->Height;
    amiIDisplay->xpix = HackScreen->Width;

#ifdef TEXTCOLOR
    LoadRGB4(&HackScreen->ViewPort, flags.amii_curmap, 1L << DEPTH );
#endif

    /* Display the copyright etc... */

    if( WIN_BASE == WIN_ERR )
	WIN_BASE = amii_create_nhwindow( NHW_BASE );
    amii_clear_nhwindow( WIN_BASE );
    amii_putstr( WIN_BASE, 0, "" );
    amii_putstr( WIN_BASE, 0, "" );
    amii_putstr( WIN_BASE, 0, "" );
    amii_putstr( WIN_BASE, 0,
      "NetHack, Copyright 1985, 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993.");
    amii_putstr( WIN_BASE, 0,
	"         By Stichting Mathematisch Centrum and M. Stephenson.");
    amii_putstr( WIN_BASE, 0, "         See license for details.");
    amii_putstr( WIN_BASE, 0, "");

    Initialized = 1;
}

/* Clear the indicated window */

void
amii_clear_nhwindow(win)
    register winid win;
{
    register struct amii_WinDesc *cw;
    register struct Window *w;

    if( win == WIN_ERR || ( cw = amii_wins[win] ) == NULL )
	panic( winpanicstr, win, "clear_nhwindow" );

    if( w = cw->win )
	SetDrMd( w->RPort, JAM2);

    cursor_off( win );

    /* should be: clear the rastport, reset x,y etc */

    if( cw->type == NHW_MENU || cw->type == NHW_TEXT )
    {
	/* Window might not be opened yet */

	if( w )
	{
	    SetAPen( w->RPort, 0 );
	    SetBPen( w->RPort, 0 );
	    RectFill( w->RPort, w->BorderLeft, w->BorderTop,
	      w->Width - w->BorderRight-1,
	      w->Height - w->BorderBottom-1 );
	    SetAPen( w->RPort, 1 );
	}
    }
    else if( w )
    {
	if( cw->type == NHW_MESSAGE )
	{
	    amii_curs( win, 1, 0 );
	    if( !scrollmsg )
		TextSpaces( w->RPort, cw->cols );
	}
	else
	{
	    SetAPen( w->RPort, 0 );
	    SetBPen( w->RPort, 0 );
	    RectFill( w->RPort, w->BorderLeft, w->BorderTop,
	      w->Width - w->BorderRight-1,
	      w->Height - w->BorderBottom-1 );
	    SetAPen( w->RPort, 1 );
#ifdef	VIEWWINDOW
	    if( win == WIN_MAP )
		amii_clear_nhwindow( WIN_VIEW );
#endif
	}
    }

    cw->cury = 0;
    cw->curx = 0;
    amii_curs( win, 1, 0 );
}

/* Dismiss the window from the screen */

void
dismiss_nhwindow(win)
    register winid win;
{
    int i;
    register struct Window *w;
    register struct amii_WinDesc *cw;

    if( win == WIN_ERR || ( cw = amii_wins[win] ) == NULL )
    {
	panic(winpanicstr,win, "dismiss_nhwindow");
    }

    w = cw->win;

    if( w )
    {
	/* All windows have this stuff attached to them. */
	if( win == WIN_MAP || win == WIN_BASE ||
		win == WIN_MESSAGE || win == WIN_STATUS
#ifdef	VIEWWINDOW
		|| win == WIN_VIEW || win == WIN_VIEWBOX
#endif
		)
	{
	    ClearMenuStrip( w );
	}

	/* Save where user like inventory to appear */
	if( win == WIN_INVEN )
	{
	    lastinvent.MinX = w->LeftEdge;
	    lastinvent.MinY = w->TopEdge;
	    lastinvent.MaxX = w->Width;
	    lastinvent.MaxY = w->Height;
	}

	/* Close the window */
	CloseShWindow( w );
	cw->win = NULL;

#ifdef	VIEWWINDOW
	if( cw->type == NHW_VIEW )
	{
	    for( i = 0; i < DEPTH; ++i )
	    {
		FreeRaster( amii_vbm.Planes[i],
			VIEWCHARWIDTH*88, VIEWCHARHEIGHT*30 );
	    }
	}
#endif

	/* Free copy of NewWindow structure for TEXT/MENU windows. */
	if( cw->newwin )
	    FreeNewWindow( (void *)cw->newwin );
	cw->newwin = NULL;
    }

    if( cw->canresp )
	free( cw->canresp );
    cw->canresp = NULL;

    if( cw->morestr )
	free( cw->morestr );
    cw->morestr = NULL;

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
    cw->maxrow = cw->maxcol = 0;
}

void
amii_exit_nhwindows(str)
    const char *str;
{
    /* Seems strange to have to do this... but we need the BASE window
     * left behind...
     */
    kill_nhwindows( 0 );
    if( str )
    {
	raw_print( "\n");	/* be sure we're not under the top margin */
	raw_print( str );
    }
}

void
amii_display_nhwindow(win,blocking)
    winid win;
    boolean blocking;
{
    int i;
    static int lastwin = -1;
    struct Window *w;
    struct amii_WinDesc *cw;

    if( !Initialized )
	return;
    lastwin = win;

    if( win == WIN_ERR || ( cw = amii_wins[win] ) == NULL )
	panic(winpanicstr,win,"display_nhwindow");

    if( cw->type == NHW_MESSAGE )
	cw->wflags &= ~FLMAP_SKIP;

    if( w = cw->win )
	WindowToFront( w );

    if( cw->type == NHW_MESSAGE || cw->type == NHW_STATUS )
	return;

    if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
    {
	flush_glyph_buffer( amii_wins[ WIN_MAP ]->win );
    }

    if( cw->type == NHW_MENU || cw->type == NHW_TEXT )
    {
	DoMenuScroll( win, blocking );
    }
    else if( cw->type==NHW_MAP )
    {
#ifdef	VIEWWINDOW
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	{
	    MoveWindowInFrontOf( amii_wins[ WIN_VIEW ]->win,
						amii_wins[ WIN_VIEWBOX ]->win );
	}
	else
	{
	    WindowToFront( amii_wins[ WIN_VIEW ]->win );
	}
	amii_end_glyphout( WIN_VIEW );
#endif
	amii_end_glyphout( win );
	for( i = 0; i < MAXWIN; ++i )
	{
	    if( ( cw = amii_wins[i] ) != NULL &&
		( cw->type == NHW_STATUS || cw->type == NHW_MESSAGE ) &&
		( cw->win ) )
	    {
		WindowToFront(cw->win);
	    }
	}

	/* Do more if it is time... */
	if( blocking == TRUE && amii_wins[ WIN_MESSAGE ]->curx )
	{
	    outmore( amii_wins[ WIN_MESSAGE ] );
	}
    }

#ifdef	VIEWWINDOW
    /* Pop the inventory window to the top if it is always displayed. */
    if( alwaysinvent && WIN_INVEN != WIN_ERR &&
		    (cw = amii_wins[ WIN_INVEN ] ) && cw->win )
    {
	WindowToFront( cw->win );
    }
#endif
}

void
amii_curs(window, x, y)
winid window;
register int x, y;  /* not xchar: perhaps xchar is unsigned and
	       curx-x would be unsigned as well */
{
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register struct RastPort *rp;

    if( window == WIN_ERR || ( cw = amii_wins[window] ) == NULL )
	panic(winpanicstr,  window, "curs");
    if( (w = cw->win) == NULL )
    {
	if( cw->type == NHW_MENU || cw->type == NHW_TEXT )
	    return;
	else
	    panic( "No window open yet in curs() for winid %d\n", window );
    }
    amiIDisplay->lastwin = window;

    /* Make sure x is within bounds */
    if( x > 0 )
	--x;    /* column 0 is never used */
    else
	x = 0;

    cw->curx = x;
    cw->cury = y;
#ifdef DEBUG
    if( x<0 || y<0 || y >= cw->rows || x >= cw->cols )
    {
	char *s = "[unknown type]";
	switch(cw->type)
	{
	    case NHW_MESSAGE: s = "[topl window]"; break;
	    case NHW_STATUS: s = "[status window]"; break;
	    case NHW_MAP: s = "[map window]"; break;
	    case NHW_MENU: s = "[menu window]"; break;
	    case NHW_TEXT: s = "[text window]"; break;
	}
	impossible("bad curs positioning win %d %s (%d,%d)", window, s, x, y);
	return;
    }
#endif

#ifdef CLIPPING
    if(clipping && window == WIN_MAP)
    {
	x -= clipx;
	y -= clipy;
    }
#endif

    /* Output all saved output before doing cursor movements for MAP */

    if( cw->type == NHW_MAP )
    {
	flush_glyph_buffer( w );
#ifdef	VIEWWINDOW
	flush_glyph_buffer( amii_wins[ WIN_VIEW ]->win );
#endif
    }

    /* Actually do it */

    rp = w->RPort;
    if( cw->type == NHW_MENU || cw->type == NHW_TEXT )
    {
	Move( rp, (x * rp->TxWidth) + w->BorderLeft + 1,
	    (y*rp->TxHeight ) + rp->TxBaseline + w->BorderTop + 1 );
    }
    else if( cw->type == NHW_MAP || cw->type == NHW_BASE
#ifdef	VIEWWINDOW
	|| cw->type == NHW_VIEW
#endif
	)
    {
	/* These coordinate calculations must be synced with those
	 * in flush_glyph_buffer() in amiwind.c.  curs_on_u() will
	 * use this code, all other drawing occurs through the glyph
	 * code.  In order for the cursor to appear on top of the hero,
	 * the code must compute X,Y in the same manner relative to
	 * the RastPort coordinates.
	 *
	 * y = w->BorderTop + (g_nodes[i].y-1) * rp->TxHeight +
	 *   rp->TxBaseline + 1;
	 * x = g_nodes[i].x * rp->TxWidth + w->BorderLeft;
	 */

	Move( rp, (x * w->RPort->TxWidth) + w->BorderLeft,
			w->BorderTop + ( (y + 1) * w->RPort->TxHeight ) +
			w->RPort->TxBaseline + 1 );
    }
    else if( cw->type == NHW_MESSAGE && !scrollmsg )
    {
	Move( rp, (x * w->RPort->TxWidth) + w->BorderLeft + 2,
			w->BorderTop + w->RPort->TxBaseline + 3 );
    }
    else if( cw->type == NHW_STATUS )
    {
	Move( rp, (x * w->RPort->TxWidth) + w->BorderLeft + 2,
			(y*(w->RPort->TxHeight+1)) + w->BorderTop +
			w->RPort->TxBaseline + 1 );
    }
    else
    {
	Move( rp, (x * w->RPort->TxWidth) + w->BorderLeft + 2,
			(y*w->RPort->TxHeight) + w->BorderTop +
			w->RPort->TxBaseline + 1 );
    }
}

void
amii_set_text_font( name, size )
    char *name;
    int size;
{
    register int i;
    register struct amii_WinDesc *cw;
    int osize = TextsFont13.ta_YSize;
    static char nname[ 100 ];

    strncpy( nname, name, sizeof( nname ) - 1 );
    nname[ sizeof( nname ) - 1 ] = 0;

    TextsFont13.ta_Name = nname;
    TextsFont13.ta_YSize = size;

    /* No alternate text font allowed for 640x269 or smaller */
    if( !HackScreen || !bigscreen )
	return;

    /* Look for windows to set, and change them */

    if( DiskfontBase =
		OpenLibrary( "diskfont.library", DISKFONT_VERSION ) )
    {
	TextsFont = OpenDiskFont( &TextsFont13 );
	for( i = 0; TextsFont && i < MAXWIN; ++i )
	{
	    if( (cw = amii_wins[ i ]) && cw->win != NULL )
	    {
	    	switch( cw->type )
	    	{
	    	case NHW_STATUS:
		    MoveWindow( cw->win, 0, -( size - osize ) * 2 );
		    SizeWindow( cw->win, 0, ( size - osize ) * 2 );
		    SetFont( cw->win->RPort, TextsFont );
		    break;
	    	case NHW_MESSAGE:
	    	case NHW_MAP:
	    	case NHW_BASE:
		    SetFont( cw->win->RPort, TextsFont );
		    break;
	    	}
	    }
	}
    }
    CloseLibrary(DiskfontBase);
    DiskfontBase = NULL;
}

void
kill_nhwindows( all )
    register int all;
{
    register int i;
    register struct amii_WinDesc *cw;

    /* Foreach open window in all of amii_wins[], CloseShWindow, free memory */

    for( i = 0; i < MAXWIN; ++i )
    {
	if( (cw = amii_wins[ i ]) && (cw->type != NHW_BASE || all) )
	{
	    amii_destroy_nhwindow( i );
	}
    }
}

void
amii_cl_end( cw, i )
    register struct amii_WinDesc *cw;
    register int i;
{
    register struct Window *w = cw->win;
    register int oy, ox;

    if( !w )
	panic("NULL window pointer in amii_cl_end()");

    oy = w->RPort->cp_y;
    ox = w->RPort->cp_x;

    TextSpaces( w->RPort, cw->cols - i );

    Move( w->RPort, ox, oy );
}

void
cursor_off( window )
    winid window;
{
    register struct amii_WinDesc *cw;
    register struct Window *w;
    register struct RastPort *rp;
    int curx, cury;
    long dmode;
    short apen, bpen;
    unsigned char ch;

    if( window == WIN_ERR || ( cw = amii_wins[window] ) == NULL )
    {
	flags.window_inited=0;
	panic(winpanicstr,window, "cursor_off");
    }

    if( !(cw->wflags & FLMAP_CURSUP ) )
	return;
    w = cw->win;

    if( !w )
	return;

    cw->wflags &= ~FLMAP_CURSUP;
    rp = w->RPort;

    /* Save the current information */
    curx = rp->cp_x;
    cury = rp->cp_y;
    dmode = rp->DrawMode;
    apen = rp->FgPen;
    bpen = rp->BgPen;
    SetAPen( rp, cw->curs_apen );
    SetBPen( rp, cw->curs_bpen );
    SetDrMd( rp, COMPLEMENT );

    ch = CURSOR_CHAR;
    Move( rp, cw->cursx, cw->cursy );
    Text( rp, &ch, 1 );

#ifdef	VIEWWINDOW
    /* Remove view window outline from map */
    if( window == WIN_MAP )
    {
	SetAPen( rp, C_RED );
	SetBPen( rp, C_BLACK );
	SetDrMd( rp, COMPLEMENT );
    	Move( rp, cw->vwx, cw->vwy );
    	Draw( rp, cw->vcx, cw->vwy );
    	Draw( rp, cw->vcx, cw->vcy );
    	Draw( rp, cw->vwx, cw->vcy );
    	Draw( rp, cw->vwx, cw->vwy );
    }
#endif

    /* Put back the other stuff */

    Move( rp, curx, cury );
    SetDrMd( rp, dmode );
    SetAPen( rp, apen );
    SetBPen( rp, bpen );

#ifdef	VIEWWINDOW
    if( window == WIN_MAP )
	cursor_off( WIN_VIEW );
#endif
}

void
cursor_on( window )
    winid window;
{
    register struct amii_WinDesc *cw;
    register struct Window *w;
#ifdef	VIEWWINDOW
    int deltax, deltay, modx, mody;
    register struct amii_WinDesc *vcw;
    register struct Window *vw;
#endif
    register struct RastPort *rp;
    unsigned char ch;
    long dmode;
    short apen, bpen;

    if( window == WIN_ERR || ( cw = amii_wins[window] ) == NULL )
    {
	/* tty does this differently - is this OK? */
	flags.window_inited=0;
	panic(winpanicstr,window, "cursor_on");
    }

    if( (cw->wflags & FLMAP_CURSUP ) )
	cursor_off( window );

    w = cw->win;

    if( !w )
	return;

    cw->wflags |= FLMAP_CURSUP;
    rp = w->RPort;

    /* Save the current information */

    cw->cursx = rp->cp_x;
    cw->cursy = rp->cp_y;
    apen = rp->FgPen;
    bpen = rp->BgPen;
    dmode = rp->DrawMode;
    ch = CURSOR_CHAR;

    /* Draw in complement mode. The cursor body will be C_WHITE */

    cw->curs_apen = C_WHITE;
    cw->curs_bpen = C_WHITE;
    SetAPen( rp, cw->curs_apen );
    SetBPen( rp, cw->curs_bpen );
    SetDrMd( rp, COMPLEMENT );
    Move( rp, cw->cursx, cw->cursy );
    Text( rp, &ch, 1 );
    Move( rp, cw->cursx, cw->cursy );

#ifdef	VIEWWINDOW
    if( window == WIN_MAP )
    {
    	int x, y, cx, cy;

	if( WIN_VIEW == WIN_ERR || ( vcw = amii_wins[ WIN_VIEW ] ) == NULL )
	{
	    flags.window_inited=0;
	    panic(winpanicstr,WIN_VIEW, "cursor_on");
	}

	if( (vcw->wflags & FLMAP_CURSUP ) )
	    cursor_off( WIN_VIEW );

	vw = vcw->win;

    	x = cw->cursx - (vw->Width/8);
	if( x <= w->BorderLeft )
	    x = w->BorderLeft + 1;

    	y = cw->cursy - (vw->Height/8);
	if( y <= w->BorderTop )
	    y = w->BorderTop + 1;

	cx = x + (vw->Width/4);
	if( cx >= w->Width - w->BorderRight )
	{
	    cx = w->Width - w->BorderRight-1;
	    x = cx - (vw->Width/4);
	}

	cy = y + (vw->Height/4);
	if( cy >= w->Height - w->BorderBottom )
	{
	    cy = w->Height - w->BorderBottom-1;
	    y = cy - (vw->Height/4);
	}
	cw->vwx = x;
	cw->vwy = y;
	cw->vcx = cx;
	cw->vcy = cy;
	SetAPen( rp, C_RED );
	SetBPen( rp, C_BLACK );
	SetDrMd( rp, COMPLEMENT );
    	Move( rp, cw->vwx, cw->vwy );
    	Draw( rp, cw->vcx, cw->vwy );
    	Draw( rp, cw->vcx, cw->vcy );
    	Draw( rp, cw->vwx, cw->vcy );
    	Draw( rp, cw->vwx, cw->vwy );

	/* Position VIEW at same location as cursor in MAP */
	vcw->curx  = ( cw->cursx * 4 ) - (vw->Width/2) - vw->RPort->Layer->Scroll_X;
	vcw->cury  = ( cw->cursy * 4 ) - ((2*vw->Height)/3) - vw->RPort->Layer->Scroll_Y;

	if( vcw->curx + vw->RPort->Layer->Scroll_X < 0 )
	    vcw->curx = -vw->RPort->Layer->Scroll_X;
	else if( vcw->curx + vw->RPort->Layer->Scroll_X > vw->RPort->Layer->bounds.MaxX )
	    vcw->curx = vw->RPort->Layer->bounds.MaxX - vw->RPort->Layer->Scroll_X;

	if( vcw->cury + vw->RPort->Layer->Scroll_Y < 0 )
	    vcw->cury = -vw->RPort->Layer->Scroll_Y;
	else if( vcw->cury + vw->RPort->Layer->Scroll_Y > vw->RPort->Layer->bounds.MaxY )
	    vcw->cury = vw->RPort->Layer->bounds.MaxY - vw->RPort->Layer->Scroll_Y;

	{
		char buf[ 100 ];
		sprintf( buf, "bounds: %d,%d,%d,%d",
			vw->RPort->Layer->bounds.MinX,
			vw->RPort->Layer->bounds.MinY,
			vw->RPort->Layer->bounds.MaxX,
			vw->RPort->Layer->bounds.MaxY );
		putstr( WIN_MESSAGE, 1, buf );
		sprintf( buf, "loc - old: %d,%d,  new: %d,%d",
			vw->RPort->Layer->Scroll_X,
			vw->RPort->Layer->Scroll_Y,
			vcw->curx + vw->RPort->Layer->Scroll_X,
			vw->RPort->Layer->Scroll_Y + vcw->cury );
		putstr( WIN_MESSAGE, 1, buf );
	}

	/* Figure out the scroll values to move in no more than 3 scrolls */
	deltax     = vcw->curx / 3;
	deltay     = vcw->cury / 3;
	modx       = vcw->curx % 3;
	mody       = vcw->cury % 3;
	vcw->curx -= modx;
	vcw->cury -= mody;

	while( vcw->curx != 0 || vcw->cury != 0 )
	{
	    ScrollLayer( 0, vw->RPort->Layer, deltax, deltay );
	    vcw->curx -= deltax;
	    vcw->cury -= deltay;
	}
	if( modx || mody )
	    ScrollLayer( 0, vw->RPort->Layer, modx, mody );
    }
#endif

    SetDrMd( rp, dmode );
    SetAPen( rp, apen );
    SetBPen( rp, bpen );
}

void amii_suspend_nhwindows( str )
    char *str;
{
    if( HackScreen )
	ScreenToBack( HackScreen );
}

void amii_resume_nhwindows()
{
    if( HackScreen )
	ScreenToFront( HackScreen );
}

void amii_bell()
{
    DisplayBeep( NULL );
}

void
removetopl(cnt)
	int cnt;
{
    struct amii_WinDesc *cw=amii_wins[WIN_MESSAGE];
					/* NB - this is sufficient for
					 * yn_function, but that's it
					 */
    if(cw->curx < cnt)cw->curx=0;
    else cw->curx -= cnt;

    amii_curs(WIN_MESSAGE, cw->curx+1, 0);
    amii_cl_end(cw, cw->curx);
}
/*#endif /* AMIGA_INTUITION */

#ifdef  PORT_HELP
void
port_help()
{
    display_file( PORT_HELP, 1 );
}
#endif

/*
 *  print_glyph
 *
 *  Print the glyph to the output device.  Don't flush the output device.
 *
 *  Since this is only called from show_glyph(), it is assumed that the
 *  position and glyph are always correct (checked there)!
 */

void
amii_print_glyph(win,x,y,glyph)
    winid win;
    xchar x,y;
    int glyph;
{
    struct amii_WinDesc *cw;
    uchar   ch;
    register int offset;
#ifdef TEXTCOLOR
    int     color;
#ifndef	SHAREDLIB
    extern int zapcolors[];
#endif

    if( win == WIN_ERR || (cw=amii_wins[win]) == NULL || cw->type != NHW_MAP)
	panic(winpanicstr,win,"print_glyph");

#define zap_color(n)  color = flags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) color = flags.use_color ? defsyms[n].color : NO_COLOR
#define trap_color(n) color = flags.use_color ? \
		(((n) == WEB) ? defsyms[S_web ].color  : \
			    defsyms[S_trap].color) : \
			NO_COLOR
#define obj_color(n)  color = flags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = flags.use_color ? mons[n].mcolor : NO_COLOR
#define pet_color(n)  color = flags.use_color ? mons[n].mcolor :          \
		/* If no color, try to hilite pets; black  */ \
		/* should be HI                */ \
		    ((flags.hilite_pet) ? BLACK : NO_COLOR)

# else /* no text color */

#define zap_color(n)
#define cmap_color(n)
#define trap_color(n)
#define obj_color(n)
#define mon_color(n)
#define pet_color(n)

#endif

    /*
     *  Map the glyph back to a character.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *        offsets.  The order is set in display.h.
     */
    if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) {  /* swallow */
	/* see swallow_to_glyph()in display.c */
	ch = (uchar) showsyms[S_sw_tl + (offset & 0x7)];
	mon_color(offset >> 3);
    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) {       /* zap beam */
	ch = showsyms[S_vbeam + (offset & 0x3)];
	zap_color((offset >> 2));
    } else if( ( offset = (glyph - GLYPH_CMAP_OFF) ) >= 0 ) {   /* cmap */
	ch = showsyms[offset];
	cmap_color(offset);
    } else if ( ( offset = (glyph - GLYPH_TRAP_OFF) ) >= 0 ) {  /* trap */
	ch = (offset == WEB) ? showsyms[S_web] : showsyms[S_trap];
	trap_color(offset);
    } else if( ( offset = (glyph - GLYPH_OBJ_OFF) ) >= 0 ) {    /* object */
	ch = oc_syms[objects[offset].oc_class];
	obj_color(offset);
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) {  /* a corpse */
	ch = oc_syms[objects[CORPSE].oc_class];
	mon_color(offset);
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {   /* a pet */
	ch = (uchar) monsyms[mons[offset].mlet];
	pet_color(offset);
    } else /*if( glyph_is_monster(glyph) )*/ {      /* a monster */
	ch = (uchar) monsyms[mons[glyph].mlet];
	mon_color(glyph);
    }

    /* Move the cursor. */
#ifdef CLIPPING
    if (!win_curs(x, y)) return;
#else
    amii_curs(win,x,y+2);
#endif

#ifdef TEXTCOLOR
    /* Turn off color if rogue level. */
# ifdef REINCARNATION
    if (Is_rogue_level(&u.uz))
	color = NO_COLOR;
#  endif

    amiga_print_glyph(win,color,ch);
#else
    g_putch(ch);    /* print the character */
#endif
    cw->curx++;     /* one character over */
}

/* Make sure the user sees a text string when no windowing is available */

void
amii_raw_print(s)
    register const char *s;
{
    if( !s )
	return;
    if(amiIDisplay)
	amiIDisplay->rawprint++;

    if( Initialized == 0 && WIN_BASE == WIN_ERR )
	    init_nhwindows();

    if( amii_rawprwin != WIN_ERR )
	amii_putstr( amii_rawprwin, 0, s );
    else if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
	amii_putstr( WIN_MAP, 0, s );
    else if( WIN_BASE != WIN_ERR && amii_wins[ WIN_BASE ] )
	amii_putstr( WIN_BASE, 0, s );
    else
    {
	puts( s);
	fflush(stdout);
    }
}

/* Make sure the user sees a bold text string when no windowing
 * is available
 */

void
amii_raw_print_bold(s)
    register const char *s;
{
    if( !s )
	return;

    if(amiIDisplay)
	amiIDisplay->rawprint++;

    if( Initialized == 0 && WIN_BASE == WIN_ERR )
	    init_nhwindows();

    if( amii_rawprwin != WIN_ERR )
	amii_putstr( amii_rawprwin, 1, s );
    else if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
	amii_putstr( WIN_MAP, 1, s );
    else if( WIN_BASE != WIN_ERR && amii_wins[ WIN_BASE ] )
	amii_putstr( WIN_BASE, 1, s );
    else
    {
	printf("\33[1m%s\33[0m\n",s);
	fflush(stdout);
    }
}

/* Rebuild/update the inventory if the window is up.
 */
void
amii_update_inventory()
{
    register struct amii_WinDesc *cw;

    if( WIN_INVEN != WIN_ERR && ( cw = amii_wins[ WIN_INVEN ] ) &&
				cw->type == NHW_MENU && cw->win )
    {
	display_inventory( NULL, FALSE );
    }
}

/* Humm, doesn't really do anything useful */

void
amii_mark_synch()
{
    if(!amiIDisplay)
	fflush(stderr);
/* anything else?  do we need this much? */
}

/* Wait for everything to sync.  Nothing is asynchronous, so we just
 * ask for a key to be pressed.
 */
void
amii_wait_synch()
{
    if(!amiIDisplay || amiIDisplay->rawprint)
    {
	if(amiIDisplay) amiIDisplay->rawprint=0;
    }
    else
    {
	display_nhwindow(WIN_MAP,TRUE);
	flush_glyph_buffer( amii_wins[ WIN_MAP ]->win );
    }
}

void
amii_setclipped()
{
    clipping = TRUE;
    clipx=clipy=0;
    clipxmax=CO;        /* WRONG */
    clipymax=LI-5;      /* WRONG */
}

void
amii_cliparound(x,y)
    register int x,y;
{
/* pull this from wintty.c - LATER */
}

void
flushIDCMP( port )
	struct MsgPort *port;
{
	struct Message *msg;
	while( msg = GetMsg( port ) )
		ReplyMsg( msg );
}
