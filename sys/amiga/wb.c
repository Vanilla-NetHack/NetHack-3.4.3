/*    SCCS Id: @(#)wb.c     2.1   93/01/08			  */
/*    Copyright (c) Kenneth Lorber, Bethesda Maryland, 1991	  */
/*    Copyright (c) Gregg Wonderly, Naperville IL, 1992, 1993	  */
/* NetHack may be freely redistributed.  See license for details. */

/* Friendly Intuition interface for NetHack 3.1 on the Amiga */

#ifdef AZTEC_C
/* Aztec doesn't recognize __chip syntax */
# define __chip
#endif

#include "incl:patchlevel.h"

#include "Amiga:wbdefs.h"		/* Miscellany information */
#ifdef  INTUI_NEW_LOOK
#define NewWindow   ExtNewWindow
#define NewScreen   ExtNewScreen
#endif
#include "Amiga:wbstruct.h"
#include "Amiga:wbprotos.h"

#include "Amiga:wbdata.c"		/* All structures and global data */
#include "Amiga:wbwin.c"		/* Has static definitions */

#define C_GREY  0
#define C_BLACK 1
#define C_WHITE 2
#define C_BLUE  3

#ifndef __SASC_60
extern char *sys_errlist[];
#endif
extern int errno;

char pubscreen[ 80 ] = { "HackWB" };
char mytitle[ 80 ];

#ifdef  INTUI_NEW_LOOK
int scrlocked = 0;
UWORD scrnpens[] = { 0xffff };

struct TagItem scrntags[] =
{
    (Tag)SA_Pens, (ULONG)scrnpens,
    TAG_DONE, 0,
    TAG_DONE, 0,
    TAG_DONE, 0,
    TAG_DONE, 0,
    TAG_DONE, 0,
    TAG_DONE, 0,
    TAG_DONE, 0,
};
#endif

#define SPLIT			/* use splitter, if available */
#ifdef SPLIT
int running_split=0;		/* if 0, using normal LoadSeg/UnLoadSeg */
#endif

#ifdef AZTEC_C
extern char *strdup(char *);
#endif

#ifndef max
# define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define min(x,y) ((x) < (y) ? (x) : (y))
#endif

void diskobj_filter(struct DiskObject *);
static void UpdateInfoWin( struct Window *cwin );
BPTR s_LoadSeg(char *);
void s_UnLoadSeg(void);

main( argc, argv )
    int argc;
    struct WBStartup *argv;
{
    long mask, rmask;
    struct WBStartup *wbs;
    struct WBArg *wba;
    GPTR gptr;
    struct IntuiMessage *imsg;
    struct IntuiMessage mimsg;
    int i;

    /* Initialize and load libraries. */
    InitWB( argc, argv );

    /* open window, build menus */
    SetupWB( );

    errmsg( NO_FLASH, "Welcome to NetHack Version %d.%d.%d!",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    CopyRight( );

    ReadConfig( );

    /* Initially, no game selected so disable menu items */

    ChgGameItems( &MenuList1, 0 );

    MapGadgets( R_DISK, 1 ); /* Display the icons */

    /* Wait till user quits */

    while( !quit )
    {
	/* Wait for a message */

	mask = ( 1L << dosport->mp_SigBit ) ;
	if( wbopen )
	    mask |= ( 1L << win->UserPort->mp_SigBit );

	rmask = Wait( mask );

	/* Process the messages on the port unless the workbench is
	 * shutdown by a request to play a game.
	 */

	while( wbopen && ( imsg = ( struct IntuiMessage * )
	    GetMsg( win->UserPort ) ) )
	{
	    /* Copy the message.  This does not guarantee that all
	     * fields will still be valid, but appears to work
	     * here.  Note that we have to reply to the message
	     * before the workbench window is closed.
	     */

	    mimsg = *imsg;
	    ReplyMsg( (struct Message *)imsg );

	    switch( mimsg.Class )
	    {
	    case NEWSIZE:
		((struct Border *) Message.GadgetRender)->XY[2] =
		    win->Width - win->BorderLeft -
		    win->BorderRight - 1;
		RefreshGList( &Message, win, NULL, 1 );
		MapGadgets( R_SCROLL, 1 ); /* redisplay the icons */
#ifdef  INTUI_NEW_LOOK
		if( IntuitionBase->LibNode.lib_Version >= 37 )
		    RefreshWindowFrame( win );
#endif
		break;

	    case MENUPICK:
		if( errup > 0 )
		{
		    errmsg( NO_FLASH, "" );
		    errup = -1;
		}
		do_menu( &MenuList1, mimsg.Code );
		flushIDCMP( win->UserPort );
		break;

	    case RAWKEY:
		if( mimsg.Code == 0x5f )
		{
		    if( errup > 0 )
		    {
			errmsg( NO_FLASH, "" );
			errup = -1;
		    }

		    /* Pick the correct help message */

		    if( lastgaddown == NULL )
		    {
			text_requester( &Help1_NewWindowStructure7,
			    &Help1_IntuiTextList7 );
		    }
		    else
		    {
			text_requester( &Help2_NewWindowStructure8,
			    &Help2_IntuiTextList8 );
		    }
		}
		flushIDCMP( win->UserPort );
		break;

	    case CLOSEWINDOW:
		if( Ask( "Ready to Quit?" ) )
		    do_closewindow( );
		break;

	    case GADGETDOWN:
		if( errup > 0 )
		{
		    errmsg( NO_FLASH, "" );
		    errup = -1;
		}
		do_gadgetdown( &mimsg );
		break;

	    case GADGETUP:
		do_gadgetup( &mimsg );
		break;

	    case DISKINSERTED:
		if( errup > 0 )
		{
		    errmsg( NO_FLASH, "" );
		    errup = -1;
		}
		MapGadgets( R_DISK, 1 );
		break;

	    case MOUSEBUTTONS:
		if( errup > 0 )
		{
		    errmsg( NO_FLASH, "" );
		    errup = -1;
		}
		do_buttons( &mimsg );
		flushIDCMP( win->UserPort );
		break;
	    }
	}
	if( errup == -1 )
	    errup = 0;

	if( rmask & ( 1L << dosport->mp_SigBit ) )
	{
	    /* Get process termination messages */

	    while( wbs = (struct WBStartup *) GetMsg( dosport ) )
	    {
		/* Find the game that has terminated */

		for( gptr = gamehead; gptr && gptr->seglist != wbs->sm_Segment;)
		    gptr = gptr->next;

		/* Make sure it is there */

		if( gptr )
		{
#ifdef SPLIT
		    if(!running_split)
#endif
		    {
			/* Unload the code */
			UnLoadSeg( wbs->sm_Segment );
		    }

		    /* Free the startup message resources */
		    wba = (struct WBArg *)
			((long)wbs + sizeof( struct WBStartup ));
		    for( i = 0; i < wbs->sm_NumArgs; ++i )
		    {
			FreeMem( wba[i].wa_Name,
			    strlen( wba[i].wa_Name ) + 1 );
			UnLock( wba[i].wa_Lock );
		    }
		    FreeMem( wbs, wbs->sm_Message.mn_Length );
		    wbs = NULL;

		    /* Say the game has completed */

		    gptr->prc = NULL;
		    gptr->active = 0;
		    active_count--;
		}
	    }

	    /* If the workbench was closed, open it back up */

	    if( !wbopen )
		SetupWB( );

	    /* Reload to clear any deleted games */

	    MapGadgets( R_DISK, 1 );
	}
    }
    cleanup( 0 );
}

void
flushIDCMP( port )
	struct MsgPort *port;
{
	struct Message *msg;

	while( msg = GetMsg( port ) )
		ReplyMsg( msg );

	SetSignal( 0L, ( 1L << port->mp_SigBit ) );
}

void CopyRight( )
{
    extern char *copyright_text[];
    int line;

    SetDrMd( win->RPort, JAM2 );
    SetAPen( win->RPort, C_WHITE );
    SetBPen( win->RPort, C_GREY );

    for(line=0;copyright_text[line];line++){
	Move( win->RPort, ORIGINX+3, ORIGINY + win->RPort->TxBaseline +
		(line*win->RPort->TxHeight));
	if(copyright_text[line][0])
	     RPText( win->RPort, copyright_text[line]);
    }

    Delay( 150 );
    ClearWindow( win );
}

/*
 * Do the one time initialization things.
 */

void
InitWB( argc, wbs )
    int argc;
    register struct WBStartup *wbs;
{
    register int c, i, j;
    BPTR odir;
    char *s, **tools, **argv;
    register struct DiskObject *dobj;
    register struct WBArg *wba;

    /* Open Libraries */
    GfxBase= (struct GfxBase *) OldOpenLibrary("graphics.library");
    IconBase= OldOpenLibrary("icon.library");
    DiskfontBase= (struct DiskfontBase *)OldOpenLibrary("diskfont.library");
    IntuitionBase= (struct IntuitionBase *)OldOpenLibrary("intuition.library");

    if(!GfxBase || !IconBase || !DiskfontBase || !IntuitionBase)
    {
	error("library open failed");
	cleanup( 1 );
    }

    /* Get Port for replied WBStartup messages */

    if( ( dosport = CreatePort( NULL, 0 ) ) == NULL )
    {
	error("failed to create dosport" );
	cleanup( 1 );
    }

    /* If started from CLI */
    if( argc != 0 )
    {
	argv = (char **)wbs;
	for( i = 1; i < argc; ++i )
	{
	    if( argv[i][0] == '?' )goto usage;
	    if( argv[i][0] != '-' )
		break;
	    for( j = 1; c = argv[i][j]; ++j )
	    {
		switch( c )
		{
		case 'm':   /* Close screen and window during game to
			     * save memory  */
		    shutdown++;
		    break;

		case 'c':       /* Configuration to load */
		    if( i + 1 < argc && argv[i][j+1] == 0 )
		    {
			strcpy( StrConf, argv[ ++i ] );
			goto nextargv;
		    }
		    else
		    {
			fprintf( stderr,
			    "%s: missing config name after -c\n",
			    argv[ 0 ] );
			cleanup( 1 );
		    }
		    break;

		case 'N':       /* Public screen name */
		    if( i + 1 < argc && argv[i][j+1] == 0 )
		    {
			strcpy( pubscreen, argv[ ++i ] );
			goto nextargv;
		    }
		    else
		    {
			fprintf( stderr,
			    "%s: missing screen name after -N\n",
			    argv[ 0 ] );
			cleanup( 1 );
		    }
		    break;

		case 'f':       /* Default game "player" */
		    if( i + 1 < argc && argv[i][j+1] == 0 )
		    {
			strcpy( defgname, argv[ ++i ] );
			goto nextargv;
		    }
		    else
		    {
			fprintf( stderr,
			    "%s: missing name after -f\n",
			    argv[ 0 ] );
			    cleanup( 1 );
		    }
		    break;
		default:
		    fprintf( stderr, "%s: invalid option %c\n",
			argv[0], c );
usage:
		    fprintf( stderr,
"usage: %s [-m] [-f .def filename] [-c config filename] [ -N screen]\n",
			argv[ 0 ] );
		    cleanup( 1 );
		}
	    }
nextargv:;
	}
    }
    else
    {
	/* Process icon's ToolTypes */

	wba = wbs->sm_ArgList;
	odir = CurrentDir( wba->wa_Lock );
	if( dobj = GetDiskObject( wba->wa_Name ) )
	{
	    tools = (char **) dobj->do_ToolTypes;

	    if( s = FindToolType( tools, "OPTIONS" ) )
	    {
		/* OPTIONS=SHUTDOWN will cause the screen to be closed
		 * when a game is started
		 */
		if( MatchToolValue( s, "SHUTDOWN" ) )
		    ++shutdown;
	    }

	    /* A different configuration file name */

	    if( s = FindToolType( tools, "CONFIG" ) )
	    {
		strcpy( StrConf, s );
	    }

	    /* A different set of defaults then 'wbdefaults.def' */

	    if( s = FindToolType( tools, "DEFAULT" ) )
	    {
		strcpy( defgname, s );
	    }

	    /* A Public screen to open onto */

	    if( s = FindToolType( tools, "SCREEN" ) )
	    {
		strcpy( pubscreen, s );
	    }

	    FreeDiskObject( dobj );
	}
	if( odir )
	    CurrentDir( odir );
    }
}

/*
 * Read a nethack.cnf like file and collect the configuration
 * information from it.
 */
void ReadConfig()
{
    register FILE *fp;
    register char *buf, *t;

    /* Use a dynamic buffer to limit stack use */

    if( ( buf = xmalloc( 1024 ) ) == NULL )
    {
	error( "Can't alloc space to read config file" );
	cleanup( 1 );
    }

    /* If the file is not there, can't load it */

    if( ( fp = fopen( StrConf, "r" ) ) == NULL )
    {
	errmsg( FLASH, "Can't load config file %s", StrConf );
	free( buf );
	return;
    }

    /* Read the lines... */

    while( fgets( buf, 1024, fp ) != NULL )
    {
	if( *buf == '#' )
	    continue;

	if( ( t = strchr( buf, '\n' ) ) != NULL )
	    *t = 0;

	if( strnicmp( buf, "PATH=", 5 ) == 0 )
	{
	    setoneopt( PATH_IDX, buf + 5 );
	}
	else if( strnicmp( buf, "PENS=", 4 ) == 0 )
	{
	    setoneopt( PENS_IDX, buf + 5 );
	}
	else if( strnicmp( buf, "OPTIONS=", 8 ) == 0 )
	{
	    setoneopt( OPTIONS_IDX, buf + 8 );
	}
	else if( strnicmp( buf, "HACKDIR=", 8 ) == 0 )
	{
	    setoneopt( HACKDIR_IDX, buf + 8 );
	}
	else if( strnicmp( buf, "LEVELS=", 7 ) == 0 )
	{
	    setoneopt( LEVELS_IDX, buf + 7 );
	}
	else if( strnicmp( buf, "SAVE=", 5 ) == 0 )
	{
	    setoneopt( SAVE_IDX, buf + 5 );
	}
	else
	{
	    /* We don't allow manipulation of the other information */
	}
    }
    fclose( fp );
    free( buf );
}

/* Close the workbench screen and window */

void CloseDownWB( )
{
    ((struct Process *)FindTask( NULL ))->pr_WindowPtr = (APTR)oldwin;

    if( win && win->RPort->TmpRas )
    {
	FreeRaster( tmprasp, width, height );
    }

    if( win )
	SafeCloseWindow( win );

#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	while( !scrlocked && CloseScreen( scrn ) == FALSE )
	{
	    Ask("Close all vistor Windows to exit" );
	}
    }
    else
#endif
    {
	CloseScreen( scrn );
    }
    wbopen = 0;
}

/* Open the workbench screen and window. */

void SetupWB( )
{
    int cpyrwid, i;
    int txtdiff;
#ifdef  INTUI_NEW_LOOK
    int pubopen = 0;
#endif
    static int once = 0;

    scrlocked = 0;
#ifdef  INTUI_NEW_LOOK
    NewScreenStructure.Extension = scrntags;
    NewScreenStructure.Type |= NS_EXTENDED;
#endif

    NewScreenStructure.Width = GfxBase->NormalDisplayColumns;
    NewScreenStructure.Height = GfxBase->NormalDisplayRows;

    {
    static char dt[40];
    sprintf(dt,"WorkBench for V%d.%d.%d of NetHack",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
    NewScreenStructure.DefaultTitle = dt;
    }

#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version < 37 )
    {
#endif
	if( ( scrn = OpenScreen( (void *)
		&NewScreenStructure ) ) == NULL )
	{
	    error( "Can't create screen" );
	    cleanup( 1 );
	}

	/* Only set the pens on the screen we open */
	LoadRGB4( &scrn->ViewPort, Palette, PaletteColorCount );
#ifdef  INTUI_NEW_LOOK
    }
    else
    {
	/* No tags beyond here yet */
	scrntags[1].ti_Tag = TAG_DONE;

	if( *pubscreen != 0 )
	{
	    if( ( scrn = LockPubScreen( pubscreen ) ) == 0 )
	    {
		/* Now add our pub screen name */
		scrntags[1].ti_Tag = SA_PubName;
		scrntags[1].ti_Data = (ULONG) pubscreen;
		scrntags[2].ti_Tag = TAG_DONE;

		/* Get the default pub screen's size */
		scrn = LockPubScreen( NULL );
		NewScreenStructure.Height = scrn->Height;
		NewScreenStructure.Width = scrn->Width;
		UnlockPubScreen( NULL, scrn );

		/* Request LACE if it looks laced.  For 2.1/3.0, we will get
		 * promoted to the users choice of modes (if promotion is alloed)
		 * which is best to avoid extra coding involving copying of the
		 * viewport modes etc.
		 */
		if( NewScreenStructure.Height > 300 )
			NewScreenStructure.ViewModes |= LACE;

		if( ( scrn = OpenScreen( (void *)
		    &NewScreenStructure ) ) == NULL )
		{
		    NewScreenStructure.Height = GfxBase->NormalDisplayRows;
		    NewScreenStructure.Width = GfxBase->NormalDisplayColumns;
		    if( ( scrn = OpenScreen( (void *)
			&NewScreenStructure ) ) == NULL )
		    {
			error( "Can't create screen" );
			cleanup( 1 );
		    }
		}
		pubopen = 1;
		scrlocked = 0;
	    }
	    else
	    {
		pubopen = 0;
		scrlocked = 1;
	    }
	}
	else
	{
	    if( ( scrn = LockPubScreen( NULL ) ) == 0 )
	    {
		error( "Can't lock Workbench screen" );
		cleanup( 1 );
	    }
	    scrlocked = 1;
	}
    }
#endif

    cpyrwid = 0;
    for( i = 0; copyright_text[i]; ++i )
	cpyrwid = max(cpyrwid, strlen( copyright_text[i] ) );

    /* 28 is magic for the width of the sizing gadget under 2.04 and
     * later.
     */
    NewWindowStructure1.Width = (cpyrwid * scrn->RastPort.TxWidth) +
	    scrn->WBorLeft + scrn->WBorRight + 28;

    width = NewWindowStructure1.Width;

    if( NewWindowStructure1.LeftEdge + width > scrn->Width )
    {
	if( width > scrn->Width )
	{
	    NewWindowStructure1.LeftEdge = 0;
	    NewWindowStructure1.Width = scrn->Width;
	}
	else
	{
	    NewWindowStructure1.LeftEdge = (scrn->Width - width)/2;
	}
    }
    height = NewWindowStructure1.Height;
    NewWindowStructure1.Screen = scrn;

    txtdiff = scrn->RastPort.TxHeight - 8;

#ifdef  INTUI_NEW_LOOK
    if( scrlocked )
	sprintf( mytitle, "NetHack WB %d.%d.%d - Select a GAME or press HELP",
	  VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    else
	strcpy( mytitle, "Select a GAME or press HELP" );

    NewWindowStructure1.Title = mytitle;
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	((struct PropInfo *)Scroll.SpecialInfo)->Flags |= PROPNEWLOOK;
    }
#endif

    if( !once )
    {
	((struct Border *) Message.GadgetRender)->XY[2] =
		NewWindowStructure1.Width +
		Message.Width - Message.LeftEdge;
	Message.TopEdge = scrn->RastPort.TxHeight + scrn->WBorTop + 1;
	Message.Height = scrn->RastPort.TxHeight + 3;
	    ((struct Border *) Message.GadgetRender)->XY[1] =
	    (((struct Border *) Message.GadgetRender)->XY[3] +=
		scrn->RastPort.TxHeight - 6 );
    }

    if( ( win = MyOpenWindow( &NewWindowStructure1 ) ) == NULL )
    {
#ifdef  INTUI_NEW_LOOK
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	{
	    if( scrlocked )
		UnlockPubScreen( NULL, scrn );
	}
#endif
	error( "Can't create window" );
	cleanup( 1 );
    }
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	/* If we did not create this screen, unlock it.
	 * otherwise, advertise it for other applications
	 * to use.
	 */
	if( scrlocked )
	    UnlockPubScreen( NULL, scrn );
	else if( pubopen )
	    PubScreenStatus( scrn, 0 );
    }
#endif

    ((struct Border *) Message.GadgetRender)->XY[2] =
	    win->Width - win->BorderLeft -
		win->BorderRight - 1;

    RefreshGList( &Message, win, NULL, 1 );
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
	RefreshWindowFrame( win );
#endif

    oldwin = (struct Window *)((struct Process *)FindTask( NULL ))->pr_WindowPtr;
    ((struct Process *)FindTask( NULL ))->pr_WindowPtr = (APTR)win;

    if( ( tmprasp = (void *) AllocRaster( width, height ) ) == NULL )
    {
	win->RPort->TmpRas = NULL;
	fprintf( stderr, "No Space for raster %d x %d\n", height, width );
	cleanup( 1 );
    }

    InitTmpRas( &tmpras, tmprasp, RASSIZE( width, height ) );

    win->RPort->TmpRas = &tmpras;

    SetUpMenus( &MenuList1, scrn );
    SetMenuStrip( win, &MenuList1 );
    wbopen = 1;
    once = 1;
}

/* Map the gadgets onto the screen at the correct location */

void MapGadgets( reason, update )
    int reason;
    int update;
{
    GPTR gptr;

    /* Make sure that any down gadget is popped back up */

    if( lastgaddown )
	SetGadgetUP( &lastgaddown->dobj->do_Gadget );
    lastgaddown = NULL;

    /* Grey Menu Items, no Game icon will be selected */

    ChgGameItems( &MenuList1, 0 );

    /* Remove them first */

    for( gptr = windowgads; gptr; gptr = gptr->nextwgad )
    {
	RemoveGadget( win, &gptr->dobj->do_Gadget );
    }
    windowgads = NULL;

    /* Remove any non-existant games */

    ClearDelGames( );

    /* If disk changed, reload existing icons */

    if( reason == R_DISK )
    {
	LoadIcons( );
    }

    /* Always move back to home unless we were scrolling */

    if( reason != R_SCROLL )
    {
	curcol = 0;
    }

    /* Calculate locations and display gadgets */

    CalcLocs( update );
}

void ClearWindow( win )
    struct Window *win;
{
    /* Clear the old gadgets from the window */

    SetAPen( win->RPort, C_GREY );
    SetOPen( win->RPort, C_GREY );
    SetDrPt( win->RPort, 0xffff );
    SetDrMd( win->RPort, JAM2 );

    RectFill( win->RPort, ORIGINX, ORIGINY, CORNERX-1, CORNERY-1 );
}

/* Calculate the place for and attach the gadgets to the window */

void
CalcLocs( update )
    int update;
{
    register GPTR gptr;
    register USHORT ox, oy, cx, cy;
    int gadid = GADNEWGAME;
    int addx = 0, sizex, sizey;

    cols = vcols = 0;
    scol = -1;

    /* Upper left corner of window */

    ox = ORIGINX;
    oy = ORIGINY;

    /* Lower right corner of window */

    cx = CORNERX;

    /* Account for text labels at the bottom by pulling the bottom up. */
    cy = CORNERY - win->RPort->TxHeight - 3;

    ClearWindow( win );

    /* Map the current list */

    for( gptr = gamehead; gptr; gptr = gptr->next )
    {
	/* If not to the horizontal offset yet, don't display */

	sizex = GADWIDTH( &gptr->dobj->do_Gadget );
	sizey = gptr->dobj->do_Gadget.Height;
	addx = max( sizex, addx );

	/* If the current column comes before the visible column... */
	if( cols < curcol )
	{
	    oy += sizey + GADINCY + 3;
	    if( gptr->next )
	    {
		if( oy + gptr->next->dobj->do_Gadget.Height + 3 >= cy )
		{
		    cols++;
		    ox += addx + GADINCX;
		    if( GADWIDTH( &gptr->next->dobj->do_Gadget ) >
			gptr->next->dobj->do_Gadget.Width )
		    {
			ox += ( GADWIDTH( &gptr->next->dobj->do_Gadget ) -
			gptr->next->dobj->do_Gadget.Width ) / 2 + 1;
		    }
		    oy = ORIGINY;
		    addx = 0;
		}
	    }
	    continue;
	}

	if( scol == -1 )
	{
	    ox = ORIGINX;
	    scol = cols;
	}

	/* If visible, draw it */

	if( ox + sizex + GADINCX < cx )
	{
	    /* Link to mapped gadgets list */

	    gptr->nextwgad = windowgads;
	    windowgads = gptr;

	    /* Set screen locations, if text is longer, scoot the
	     * gadget over to make room for it.
	     */

	    if( GADWIDTH( &gptr->dobj->do_Gadget ) >
		    gptr->dobj->do_Gadget.Width )
	    {
		gptr->dobj->do_Gadget.LeftEdge = ox +
		    ( GADWIDTH( &gptr->dobj->do_Gadget ) -
			gptr->dobj->do_Gadget.Width ) / 2 + 1;
	    }
	    else
		gptr->dobj->do_Gadget.LeftEdge = ox;
	    addx = max( addx, GADWIDTH( &gptr->dobj->do_Gadget) );
	    gptr->dobj->do_Gadget.TopEdge = oy;
	    gptr->dobj->do_Gadget.GadgetID = gadid++;

	    AddGadget( win, &gptr->dobj->do_Gadget, 0 );
	}

	/* Stack vertically first, then horizontally */

	if( gptr->next )
	{
	    oy += sizey + GADINCY + 3;
	    if( oy + gptr->next->dobj->do_Gadget.Height + 3 >= cy )
	    {
		ox += addx + GADINCX;
		cols++;
		if( ox + GADWIDTH( &gptr->next->dobj->do_Gadget) < cx )
		    vcols++;
		addx = 0;
		oy = ORIGINY;
	    }
	}
    }

    /* Display all of the gadgets */

    RefreshGList( win->FirstGadget, win, NULL, -1 );

    /* Set up the slider if forcing a new position, otherwise
     * the slider was probably moved and its position should be
     * left where the user put it instead of jerking it around
     */
    if( update )
	UpdatePropGad( win, &Scroll, vcols+1, cols+1, scol );
}

/* Open the indicated window and place the IntuiText list passed in that
 * window.  Then wait for the OKAY gadget to be clicked on.
 */
void text_requester( newwin, tlist )
    register struct NewWindow *newwin;
    struct IntuiText *tlist;
{
    register struct Window *win;
    register struct IntuiMessage *imsg;
    register struct Gadget *gd;
    int done = 0;
    int i;
    long class;
    struct NewWindow **aonce;
    static struct NewWindow *once[ 6+1 ];
    int lines[ 10 ], lcnt = 0, avone = -1;
    register int txtdiff = scrn->RastPort.TxHeight - 8;

    newwin->Screen = scrn;

    for( i = 0; i < 6; ++i )
    {
	if( newwin == once[i] )
	    break;
	if( once[i] == 0 && avone == -1 )
	    avone = i;
    }
    aonce = &once[avone];

    /* If spacing not correct, fix it up now */
    if( *aonce == NULL )
    {
	register struct IntuiText *ip = tlist;
	for( ; ip; ip = ip->NextText )
	{
	    if( lcnt == 0 )
		lines[ lcnt++ ] = ip->TopEdge;
	    else
	    {
		register found = 0;
		for( i = 0; i < lcnt; ++i )
		{
		    if( lines[ i ] > ip->TopEdge )
			break;
		    if( lines[ i ] == ip->TopEdge )
		    {
			found = 1;
			break;
		    }
		}

		if( !found )
		{
		    if( i < lcnt )
		    {
			int j;
			for( j = lcnt; j > i; --j )
			    lines[ j ] = lines[ j - 1 ];
		    }
		    lcnt++;
		    lines[ i ] = ip->TopEdge;
		}
	    }
	}

	for( ip = tlist; ip; ip = ip->NextText )
	{
	    for( i = 0; i < lcnt; ++i )
	    {
		if( ip->TopEdge == lines[ i ] )
		{
		    ip->TopEdge += txtdiff*i;
		    break;
		}
	    }
	}

	gd = FindGadget( NULL, newwin, GADHELPOKAY );
	gd->TopEdge += (lcnt+1)*txtdiff;
	gd->Height += txtdiff;
	SetBorder( gd, -1 );
	newwin->Height += txtdiff * (lcnt+2);
	*aonce = newwin;
    }

    if( ( win = MyOpenWindow( newwin ) ) == NULL )
    {
	errmsg( FLASH, "Can't create window" );
	return;
    }

    PrintIText( win->RPort, tlist, 0, txtdiff );

    while( !done )
    {
	WaitPort( win->UserPort );
	while( ( imsg = (struct IntuiMessage * )
	    GetMsg( win->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    ReplyMsg( (struct Message *) imsg );
	    switch( class )
	    {
		case CLOSEWINDOW: done = 1; break;
		case VANILLAKEY: done = 1; break;
		/* Should be GADHELPOKAY */
		case GADGETUP: done = 1; break;
	    }
	}
    }
    SafeCloseWindow( win );
}

/* Scroll through a file which is passed by name */

void help_requester( file )
    char *file;
{
    register struct Window *win;
    register struct IntuiMessage *imsg;
    register struct Gadget *gd;
    FILE *fp;
    int done = 0, line = 0, lines, topline, tlines, i;
    static int once = 0, lastdown;
    char buf[ 100 ];
    long loff[ 100 ];
    long class, code;
    int txtdiff = scrn->RastPort.TxHeight - 8;

    if( ( fp = fopen( file, "r" ) ) == NULL )
    {
#ifdef  __SASC_60
	errmsg( FLASH, "Can't open %s: %s", file, strerror(errno) );
#else
	errmsg( FLASH, "Can't open %s: %s", file, sys_errlist[errno] );
#endif
	return;
    }
    for( tlines = 0; tlines < 100 ; ++tlines )
    {
	loff[ tlines ] = ftell( fp );
	if( fgets( buf, sizeof( buf ), fp ) == NULL )
	    break;
    }

    if( !once )
    {
	for( gd = Help3_NewWindowStructure10.FirstGadget;
		    gd; gd = gd->NextGadget )
	{
	    if( gd->GadgetID != 0 )
	    {
		if( gd->GadgetID == GADHELPFRWD ||
		    gd->GadgetID == GADHELPBKWD )
		{
		    gd->Height += txtdiff;
		}
		SetBorder( gd, -1 );
	    }
	}
	once = 1;
	Help3_NewWindowStructure10.Height += txtdiff;
    }

    {
    static char title[40];
    sprintf(title,"Help for NetHack WorkBench V%d.%d.%d",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
    Help3_NewWindowStructure10.Title = title;
    }

    Help3_NewWindowStructure10.Screen = scrn;
    if( ( win = MyOpenWindow( &Help3_NewWindowStructure10 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create requester window" );
	fclose( fp );
	return;
    }
    lines = ( (win->Height - win->BorderTop - 25 ) / win->RPort->TxHeight );
    topline = win->BorderTop + win->RPort->TxBaseline + 2;
    Move( win->RPort, win->BorderLeft, topline );

    SetAPen( win->RPort, C_BLACK );
    SetBPen( win->RPort, C_GREY );
    SetDrMd( win->RPort, JAM2 );

    for( i = 0; i < min( lines, tlines ); ++i )
    {
	getline( fp, loff, i, buf, sizeof( buf ) );
	Move( win->RPort, win->BorderLeft + 2,
	    topline + (i * win->RPort->TxHeight) );
	Text( win->RPort, buf, strlen( buf )-1 );
    }

    while( !done )
    {
	WaitPort( win->UserPort );
	while( ( imsg = (void *) GetMsg( win->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    gd = (struct Gadget *)imsg->IAddress;

	    ReplyMsg( (struct Message *) imsg );

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == 'u' || code == ('U'-64))
		    {
			goto bkwd;
		    }
		    else if( code == 'd' || code == ('D'-64))
		    {
			goto frwd;
		    }
		    else if( code == '\33' || code == 'q' )
		    {
			done = 1;
		    }
		    break;

		case CLOSEWINDOW:
		    done = 1;
		    break;

		case MOUSEBUTTONS:
		case INACTIVEWINDOW:
		case ACTIVEWINDOW:
		case GADGETUP:
		    lastdown = 0;
		    break;

		case GADGETDOWN:
		    lastdown = gd->GadgetID;
		    break;

		case INTUITICKS:
		    if( lastdown == GADHELPFRWD )
		    {
			frwd:
			if( line + lines < tlines )
			{
			    line++;
			    WaitTOF();
			    ScrollRaster( win->RPort, 0,
				win->RPort->TxHeight,
				win->BorderLeft,
				win->BorderTop + 2,
				win->Width - win->BorderRight - 1,
				win->BorderTop + 1 +
				(lines*win->RPort->TxHeight) );
			    getline( fp, loff, line + lines - 1,
				buf, sizeof( buf ) );
			    Move( win->RPort, win->BorderLeft + 2,
				topline + ( ( lines - 1 ) *
				win->RPort->TxHeight ) );
			    WaitTOF();
			    Text( win->RPort, buf, strlen( buf )-1 );
			}
			else
			{
			    /* EOF */
			    DisplayBeep( scrn );
			    lastdown = 0;
			}
		    }
		    else if( lastdown == GADHELPBKWD )
		    {
			bkwd:
			if( line > 0 )
			{
			    line--;
			    WaitTOF();
			    ScrollRaster( win->RPort, 0,
				-win->RPort->TxHeight,
				win->BorderLeft,
				win->BorderTop + 2,
				win->Width - win->BorderRight - 1,
				win->BorderTop + 1 +
				(lines*win->RPort->TxHeight) );
			    getline( fp, loff, line, buf, sizeof( buf ) );
			    Move( win->RPort, win->BorderLeft + 2, topline );
			    WaitTOF();
			    Text( win->RPort, buf, strlen( buf )-1 );
			}
			else
			{
			    DisplayBeep( scrn );
			    lastdown = 0;
			}
		    }
		    break;

	    }
	}
    }
    SafeCloseWindow( win );
    fclose( fp );
}

/* Act on the menu item number passed */

void
do_menu( mptr, mcode)
    struct Menu *mptr;
    register int mcode;
{
    while( mcode != MENUNULL )
    {
	switch(MENUNUM(mcode))
	{
	    case MENU_PROJECT:
		switch(ITEMNUM(mcode))
		{
		    case ITEM_HELP:
			help_requester( "NetHack:HackWB.hlp" );
			break;

		    case ITEM_ABOUT:
			text_requester( &About_NewWindowStructure9,
				&About_IntuiTextList9 );
			break;

		    case ITEM_SCORES:
			menu_scores( );
			break;

		    case ITEM_RECOVER:
			menu_recover( );
			break;

		    case ITEM_CONFIG:
			menu_config( );
			break;

		    case ITEM_QUIT:
			quit = Ask( "Ready to Quit?" );
			break;

		}
		break;

	    case MENU_GAME:
		switch( ITEMNUM( mcode ) )
		{

		    case ITEM_INFO:
			menu_info( );
			break;

		    case ITEM_COPYOPT:
			menu_copyopt( );
			break;

		    case ITEM_DISCARD:
			menu_discard( );
			break;

		    case ITEM_RENAME:
			menu_rename( );
			break;
		}
	}
        mcode = ((struct MenuItem *)ItemAddress( mptr, (long)mcode ))->NextSelect;
    }
}

void
menu_discard()
{
    register GPTR gptr;

    if( ( gptr = NeedGame() ) == NULL )
	return;

    if( Ask("Discard Selected Game?") )
    {
	lastgaddown = NULL;
	if( DeleteGame( gptr ) == 0 )
	{
	    errmsg( FLASH, "Discard may have failed for %s",
		    GameName( gptr, NULL ) );
	}

	MapGadgets( R_DISK, 1 );
    }
}

void
run_game( gptr )
    register GPTR gptr;
{
    extern UWORD __chip waitPointer[];
    struct Task *ctask;
    register struct MsgPort *proc = NULL;
    char buf[ 100 ];
    char namebuf[ 100 ];
    int once, tidx;

    if( gptr->active )
    {
	errmsg( FLASH, "%s is already in progress", gptr->name );
	return;
    }

    tidx = 0;

    /* If newgame gadget, then check game name */

    if( gptr->dobj->do_Gadget.GadgetID == GADNEWGAME )
    {
	once = 0;
	sprintf( buf, "%s/%s.sav", options[ SAVE_IDX ], gptr->name );
	while( access( buf, 0 ) == 0 )
	{
	    if( StrRequest( "Game Already Exists, Enter a New Name",
		namebuf, once ? namebuf : gptr->gname ) == 0 )
	    {
		return;
	    }
	    once = 1;
	    sprintf( buf, "%s/%s.sav", options[ SAVE_IDX ], namebuf );
	}
    }
    gptr->gname = xmalloc( 20 + strlen( gptr->name ) );

    /*
     * options[] are no longer put into the tooltypes because they are in the options
     * string now.
     */

    gptr->wbs = AllocMem( sizeof( struct WBStartup ) +
	    ( sizeof( struct WBArg ) * 2 ), MEMF_PUBLIC | MEMF_CLEAR );

    /* Check if we got everything */

    if( !gptr->gname || !gptr->wbs )
    {
	fprintf( stderr, "Can't allocate memory\n" );
	goto freemem;
    }

    /* Get the arguments structure space */

    gptr->wba = ( struct WBArg * ) ((long)gptr->wbs +
		sizeof( struct WBStartup ) );

    /* Close down window and screen if requested */

    if( shutdown )
	CloseDownWB( );

    SetPointer( win, waitPointer, 16, 16, -6, 0 );

    /* Load the game into memory */

#ifdef SPLIT
    /* Which version do we run? */
    {
	char gi[80];

	sprintf( gi, "%s.dir", GAMEIMAGE );
	if( access( gi, 0 ) == 0 ){
	    gptr->seglist = (BPTR)s_LoadSeg( gi );
	    if( gptr->seglist ) running_split=1;
	}else{
	    gptr->seglist = (BPTR)LoadSeg( GAMEIMAGE );
	}
    }
#else
    gptr->seglist = (BPTR)LoadSeg( GAMEIMAGE );
#endif
    ClearPointer( win );

    if( gptr->seglist == NULL)
    {
	if( !wbopen )
	    SetupWB( );
	errmsg( FLASH, "Can't load %s", GAMEIMAGE );
	goto freemem;
    }
    /* Build WBStartup from current game info */

    /* Set the game name for the status command */

    sprintf( gptr->gname, "NetHack %d.%d.%d %s",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, gptr->name );

    /* Create a process for the game to execute in */

    ctask = FindTask( NULL );
    proc = CreateProc( gptr->gname, ctask->tc_Node.ln_Pri,
	gptr->seglist, GAMESTACK );

    /* Check if the create failed */

    if( proc == NULL )
    {
    fprintf(stderr, "Error creating process %d\n", IoErr() );
#ifdef SPLIT
	if(!running_split)
#endif
	    UnLoadSeg( gptr->seglist );
freemem:
    if( gptr->gname ) free( gptr->gname );
    gptr->gname = NULL;

    if( gptr->wbs ) FreeMem( gptr->wbs,
	sizeof( struct WBStartup ) + sizeof( struct WBArg ) * 2 );
    gptr->wbs = NULL;
    if( !wbopen )
	SetupWB( );
    return;
    }

    /* Get the Process structure pointer */

    gptr->prc = (struct Process *) (((long)proc) - sizeof( struct Task ));

    /* Set the current directory */

    gptr->prc->pr_CurrentDir=((struct Process *)FindTask(NULL))->pr_CurrentDir;

    /* Fill in the startup message */

    gptr->wbs->sm_Process = proc;
    gptr->wbs->sm_Segment = gptr->seglist;
    gptr->wbs->sm_NumArgs = 2;
    {
    static char tw[40];
    sprintf(tw,"con:0/0/350/50/Amiga NetHack %d.%d.%d",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
    gptr->wbs->sm_ToolWindow = tw;
    }
    gptr->wbs->sm_ArgList = gptr->wba;

    /* Fill in the args */

    gptr->wba[0].wa_Name = Strdup( GAMEIMAGE );
    gptr->wba[0].wa_Lock = Lock( dirname( GAMEIMAGE ), ACCESS_READ );

    gptr->wba[1].wa_Name = Strdup( gptr->name );
    gptr->wba[1].wa_Lock = Lock( gptr->dname, ACCESS_READ );

    /* Write the updated tools types entries */

    WriteDObj( gptr, gptr->wba[1].wa_Lock );

    /* Set the message fields correctly */

    gptr->wbs->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
    gptr->wbs->sm_Message.mn_Node.ln_Pri = 0;
    gptr->wbs->sm_Message.mn_ReplyPort = dosport;
    gptr->wbs->sm_Message.mn_Length =
	sizeof( struct WBStartup ) + ( sizeof( struct WBArg ) * 2 );

    /* mark game as in use */

    active_count++;
    gptr->active = 1;

    /* Send the WB Startup message to let the game go... */

    PutMsg( proc, &gptr->wbs->sm_Message );
}

void CloseLibraries( )
{
    if( IntuitionBase )     CloseLibrary( (void *) IntuitionBase );
    IntuitionBase = 0;
    if( DiskfontBase )      CloseLibrary( (void *) DiskfontBase );
    DiskfontBase = 0;
    if( IconBase )          CloseLibrary(  IconBase );
    IconBase = 0;
    if( GfxBase )           CloseLibrary( (void *) GfxBase );
    GfxBase = 0;
}

void cleanup( code )
    int code;
{
    if( active_count )
    {
	errmsg( FLASH, "There %s still %d game%s active...",
		active_count > 1 ? "are" : "is",
		active_count,
		active_count > 1 ? "s" : "" );
	return;
    }

    if( dosport ) DeletePort( dosport );
    dosport = NULL;

    CloseDownWB( );
    CleanUpLists( );
    CloseLibraries( );

#ifdef SPLIT
    if(running_split)s_UnLoadSeg();
#endif
    exit( code );
}

GPTR AllocGITEM( )
{
    register GPTR gptr;

    if( gameavail )
    {
	gptr = gameavail;
	gameavail = gameavail->next;
    }
    else
    {
	gptr = xmalloc( sizeof( GAMEITEM ) );
    }

    if( gptr )
	memset( gptr, 0, sizeof( GAMEITEM ) );

    return( gptr );
}

void FreeGITEM( gptr )
    register GPTR gptr;
{
    /* Free all of the pieces first */

    if( gptr->talloc )
	FreeTools( gptr );

    if( gptr->dobj )
	FreeDObj( gptr->dobj );

    gptr->dobj = NULL;

    if( gptr->name )
	free( gptr->name );
    gptr->name = NULL;

    if( gptr->dname )
	free( gptr->dname );
    gptr->dname = NULL;

    if( gptr->fname )
	free( gptr->fname );
    gptr->fname = NULL;

    /* Connect it to free list */

    gptr->next = gameavail;
    gameavail = gptr;
}

struct DiskObject *AllocDObj( str )
    register char *str;
{
    register struct DiskObject *doptr;
    register char *t, *t1;

    if( ( t = strrchr( str, '.' ) ) && stricmp( t, ".info" ) == 0 )
    {
	*t = 0;
    }
    else
    {
	t = NULL;
    }

    if( doptr = GetDiskObject( str ) )
    {
	struct IntuiText *ip;

	diskobj_filter(doptr);  /* delete INTERNALCLI */

	if( ip = xmalloc( sizeof( struct IntuiText ) ) )
	{
	    memset( ip, 0, sizeof( struct IntuiText ) );
	    ip->FrontPen = C_BLACK;
	    ip->DrawMode = JAM1;
	    ip->LeftEdge = (doptr->do_Gadget.Width -
		( strlen( str ) * win->RPort->TxWidth ))/2;
	    ip->TopEdge = doptr->do_Gadget.Height;
	    ip->IText = strdup( str );
	    doptr->do_Gadget.GadgetText = ip;

	    /* Trim any .sav off of the end. */

	    if( ( t1 = strrchr( ip->IText, '.' ) ) &&
		    stricmp( t1, ".sav" ) == 0 )
	    {
		*t1 = 0;
		ip->LeftEdge += (2 * win->RPort->TxWidth);
	    }
	}
    }
    if( t ) *t = '.';

    return( doptr );
}

void LoadIcons( )
{
    register GPTR gptr, newgame;
    register BPTR savedir;
    register char *t;
    register struct FileInfoBlock *finfo;
    char buf[ 200 ];

    /* Check if we can access the new save directory */

    if( t = strchr( options[ SAVE_IDX ], ';' ) )
    {
	strncpy( buf, options[ SAVE_IDX ], sizeof( buf ) - 1 );
	buf[ sizeof( buf ) - 1 ] = 0;
	if( ( t = strchr( buf, ';' ) ) && strcmp( t, ";n" ) == 0 )
	    *t = 0;
	if( ( savedir = Lock( buf, ACCESS_READ ) ) == NULL )
	{
	    errmsg( FLASH,
		    "Can't access save directory: %s", buf );
	    return;
	}
    }
    else if( ( savedir = Lock( options[ SAVE_IDX ], ACCESS_READ ) ) == NULL )
    {
	errmsg( FLASH,
		"Can't access save directory: %s", options[ SAVE_IDX ] );
	return;
    }

    if( ( finfo = (struct FileInfoBlock *)
		    xmalloc( sizeof( struct FileInfoBlock ) ) ) == NULL )
    {
	UnLock( savedir );
	errmsg( FLASH, "Can't alloc memory" );
	return;
    }

    if( ( newgame = gamehead ) && newgame->dobj->do_Gadget.GadgetID == GADNEWGAME )
	gamehead = gamehead->next;
    else
	newgame = NULL;

    if( !Examine( savedir, finfo ) )
    {
	UnLock( savedir );
	free( finfo );
	errmsg( FLASH, "Can't Examine save directory" );
	return;
    }

    /* Collect all of the entries */

    while( ExNext( savedir, finfo ) )
    {
	/* If already got this game, continue */

	if( gptr = FindGame( finfo->fib_FileName ) )
	    continue;

	/* Get just the ones we are interested in */

	if( ( t = strrchr( finfo->fib_FileName, '.' ) ) == NULL ||
		    stricmp( t, ".info" ) != 0 )
	    continue;

	if( t == finfo->fib_FileName )
	    continue;

	/* Get a gadget item */

	if( gptr = GetWBIcon( savedir, options[ SAVE_IDX ], finfo ) )
	{
	    gptr->next = gamehead;
	    gamehead = gptr;
	}
    }

    /* Get the NewGame gadget */

    UnLock( savedir );
    if( newgame == NULL )
    {
	/* Pick up the new game if not there yet. */

	sprintf( buf, "%sNewGame.info", options[ HACKDIR_IDX ] );
	if( savedir = Lock( buf, ACCESS_READ ) )
	{
	    if( Examine( savedir, finfo ) )
	    {
		UnLock( savedir );
		savedir = Lock( options[ HACKDIR_IDX ], ACCESS_READ );
		if( gptr = GetWBIcon( savedir,
			options[ HACKDIR_IDX ], finfo ) )
		{
		    gptr->next = gamehead;
		    gamehead = gptr;
		}
	    }
	    UnLock( savedir );
	    free( finfo );
	}
	else
	{
	    errmsg( FLASH, "No access to %s", buf );
	}
    }
    else
    {
	newgame->next = gamehead;
	gamehead = newgame;
    }
}

void menu_recover()
{
    int execit = 1;
    long class, code;
    struct Gadget *gd, *lastact = 0;
    int done = 0;
    struct IntuiMessage *imsg;
    struct Window *w;
    static int once = 0;
    int txtdiff = scrn->RastPort.TxHeight - 8;
    struct IntuiText *ip;

    if( !once )
    {
	for( gd = Rst_NewWindowStructure11.FirstGadget;
		    gd; gd = gd->NextGadget )
	{
	    switch( gd->GadgetID )
	    {
		case GADRESTDIR:
		    gd->TopEdge += txtdiff;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    strcpy(RstDir,options[LEVELS_IDX]);
		    break;
		case GADRESTOLD:
		    gd->TopEdge += txtdiff*2;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    strcpy(RstOld,"levels");
		    break;
		case GADRESTNEW:
		    gd->TopEdge += txtdiff*3;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADRESTOKAY:
		    gd->TopEdge += txtdiff*4;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADRESTCAN:
		    gd->TopEdge += txtdiff*4;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
	    }
	}
	Rst_NewWindowStructure11.Height += txtdiff*5;
	for( ip = &Rst_IntuiTextList11; ip; ip = ip->NextText )
	{
	    if( *ip->IText == 'O' )
		ip->TopEdge += txtdiff;
	    else if( *ip->IText == 'N' )
		ip->TopEdge += txtdiff*2;
	}
	once = 1;
    }

    Rst_NewWindowStructure11.Screen = scrn;
    if( ( w = MyOpenWindow( &Rst_NewWindowStructure11 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create requester window" );
	return;
    }
    PrintIText( w->RPort, &Rst_IntuiTextList11, 0, txtdiff );
    lastact = FindGadget( w, NULL, GADRESTDIR );

    while( !done )
    {
	WaitPort( w->UserPort );
	while( imsg = (struct IntuiMessage *) GetMsg( w->UserPort ) )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    gd = (struct Gadget *)imsg->IAddress;
	    ReplyMsg( (struct Message *) imsg );
	    switch( class )
	    {
		case CLOSEWINDOW:
		    done = 1;
		    execit = 0;
		    break;

		case ACTIVEWINDOW:
		    ActivateGadget( lastact, w, NULL );
		    break;

		case GADGETUP:
		    if( gd->GadgetID == GADRESTOKAY )
			done = 1;
		    else if( gd->GadgetID == GADRESTCAN )
		    {
			execit = 0;
			done = 1;
		    }
		    else if( gd->GadgetID == GADRESTDIR )
		    {
			if( gd = FindGadget( w, NULL, GADRESTOLD ) )
			    ActivateGadget( lastact = gd, w, NULL );
		    }
		    else if( gd->GadgetID == GADRESTOLD )
		    {
			if( gd = FindGadget( w, NULL, GADRESTNEW ) )
			    ActivateGadget( lastact = gd, w, NULL );
		    }
		    break;

		case VANILLAKEY:
		    if( code == '\33' )
		    {
			done = 1;
			execit = 0;
		    }
		    break;
	    }
	}
    }

    SafeCloseWindow( w );

    if( execit )
    {
	char buf[255];
	sprintf( buf, "stack 65000\nNetHack:Recover -d %s %s", RstDir, RstOld );
	Execute( buf, NULL, NULL );
	MapGadgets( R_DISK, 1);
    }
}

void menu_config()
{
    register struct Window *cwin;
    int done = 0, quit;
    long class, code, qual;
    register struct IntuiMessage *imsg;
    struct IntuiText *ip;
    register struct Gadget *gd;
    static int once = 0;
    int txtdiff = scrn->RastPort.TxHeight - 8;
    char *env;

    strcpy( StrPath, options[ PATH_IDX ] );
    strcpy( StrHackdir, options[ HACKDIR_IDX ] );
    strcpy( StrPens, options[ PENS_IDX ] );
    strcpy( StrLevels, options[ LEVELS_IDX ] );
    strcpy( StrSave, options[ SAVE_IDX ] );

    if( !once )
    {
	for( gd = Conf_NewWindowStructure4.FirstGadget;
		    gd; gd = gd->NextGadget )
	{
	    switch( gd->GadgetID )
	    {
		case GADSTRPATH:
		    /* Look for "Path:" string */
		    for( ip = &Conf_IntuiTextList4;
			    ip && *ip->IText != 'P'; )
			ip = ip->NextText;
		    gd->TopEdge += txtdiff;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADSTRHACKDIR:
		    /* Look for "Hackdir:" string */
		    for( ip = &Conf_IntuiTextList4;
			    ip && *ip->IText != 'H'; )
			ip = ip->NextText;
		    if( ip )
			ip->TopEdge += txtdiff;
		    gd->TopEdge += txtdiff*2;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADSTRPENS:
		    /* Look for "Pens:" string */
		    for( ip = &Conf_IntuiTextList4; ip &&
			!(*ip->IText == 'P' && ip->IText[1] == 'e'); )
		    {
			ip = ip->NextText;
		    }
		    if( ip )
			ip->TopEdge += txtdiff*2;
		    gd->TopEdge += txtdiff*3;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADSTRLEVELS:
		    /* Look for "Levels:" string */
		    for( ip = &Conf_IntuiTextList4; ip && *ip->IText != 'L'; )
			ip = ip->NextText;
		    if( ip )
			ip->TopEdge += txtdiff*3;
		    gd->TopEdge += txtdiff*4;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADSTRSAVE:
		    /* Look for "Save Dir:" string */
		    for( ip = &Conf_IntuiTextList4; ip && *ip->IText != 'S'; )
			ip = ip->NextText;
		    if( ip )
			ip->TopEdge += txtdiff*4;
		    gd->TopEdge += txtdiff*5;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADCONFLOAD:
		case GADCONFSAVE:
		    gd->TopEdge += txtdiff*6;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;
		case GADCONFNAME:
		    for( ip = &Conf_IntuiTextList4; ip && *ip->IText != 'C'; )
			ip = ip->NextText;
		    if( ip )
			ip->TopEdge += txtdiff*6;
		    gd->TopEdge += txtdiff*7;
		    gd->Height += txtdiff;
		    SetBorder( gd, -1 );
		    break;

		default:
		    break;
	    }
	}
	Conf_NewWindowStructure4.Height += txtdiff*8;
	if( Conf_NewWindowStructure4.TopEdge +
	    Conf_NewWindowStructure4.Height > scrn->Height )
	{
	    Conf_NewWindowStructure4.TopEdge -=
		( Conf_NewWindowStructure4.TopEdge +
		Conf_NewWindowStructure4.Height ) - scrn->Height + 1;
	    if( Conf_NewWindowStructure4.TopEdge < 0 )
	    {
		Conf_NewWindowStructure4.TopEdge = 0;
		Conf_NewWindowStructure4.Height = scrn->Height - 1;
	    }
	}
	once = 1;
    }

    Conf_NewWindowStructure4.Screen = scrn;
    if( ( cwin = MyOpenWindow( &Conf_NewWindowStructure4 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create requester window" );
	return;
    }

    PrintIText( cwin->RPort, &Conf_IntuiTextList4, 0, txtdiff );
    while( !done )
    {
	WaitPort( cwin->UserPort );
	while( ( imsg = (void *) GetMsg( cwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *)imsg->IAddress;

	    ReplyMsg( (struct Message *)imsg );

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == '\33' || (code == 'b' && (qual&AMIGALEFT)))
		    {
			done = 0;
			quit = 1;
		    }
		    break;

		case ACTIVEWINDOW:
		    if( gd = FindGadget( cwin, NULL, GADCONFNAME ) )
			ActivateGadget( gd, cwin, NULL );
		    break;

		case CLOSEWINDOW:
		    done = 1;
		    quit = 0;
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
			case GADSTRPATH:
			    if( gd = FindGadget( cwin, NULL,GADSTRHACKDIR) )
				ActivateGadget( gd, cwin, NULL );
			    break;
			case GADSTRHACKDIR:
			    if( gd = FindGadget( cwin, NULL,GADSTRPENS) )
				ActivateGadget( gd, cwin, NULL );
			    break;
			case GADSTRPENS:
			    if( gd = FindGadget( cwin, NULL,GADSTRLEVELS) )
				ActivateGadget( gd, cwin, NULL );
			    break;
			case GADSTRLEVELS:
			    if( gd = FindGadget( cwin, NULL, GADSTRSAVE ) )
				ActivateGadget( gd, cwin, NULL );
			    break;
			case GADSTRSAVE:
			    if( gd = FindGadget( cwin, NULL, GADCONFNAME ) )
				ActivateGadget( gd, cwin, NULL );
			    break;

			case GADCONFNAME:	/* Do nothing... */
			    break;

			case GADCONFLOAD:
			    ReadConfig( );
                            env = malloc( strlen( StrConf ) + 3 +
						strlen( "NETHACKOPTIONS" ) );
			    sprintf( env, "NETHACKOPTIONS=@%s", StrConf );
			    putenv( env );
			    free( env );
			    strcpy( StrPath, options[ PATH_IDX ] );
			    strcpy( StrHackdir, options[ HACKDIR_IDX ] );
			    strcpy( StrPens, options[ PENS_IDX ] );
			    strcpy( StrLevels, options[ LEVELS_IDX ] );
			    strcpy( StrSave, options[ SAVE_IDX ] );
			    RefreshGList( cwin->FirstGadget, cwin, NULL, -1 );
			    break;

			case GADCONFSAVE:
        		    {
        		    	FILE *fp, *nfp;
        		    	char buf[ 300 ], *t, nname[ 100 ], oname[100], *b;

                                setoneopt( PATH_IDX, StrPath );
                                setoneopt( HACKDIR_IDX, StrHackdir );
                                setoneopt( PENS_IDX, StrPens );
                                setoneopt( LEVELS_IDX, StrLevels );
                                setoneopt( SAVE_IDX, StrSave );

        			fp = fopen( StrConf, "r" );
        			if( !fp )
        			{
				    fp = fopen( "NetHack:NetHack.cnf", "r" );
				    strcpy( StrConf, "NetHack:NetHack.cnf" );
				}
        			if( !fp )
        			{
                        	    errmsg( FLASH, "Can't open config file" );
                        	    break;
                        	}

                          	t = dirname( StrConf );
                        	b = basename( StrConf );
                          	if( t[ strlen(t)-1 ] == ':' )
                          	{
                          	    sprintf( nname, "%snew_%s", t, b);
                          	    sprintf( oname, "%sold_%s", t, b);
                        	}
                     		else
				{
                            	    sprintf( oname, "%s/old_%s", t, b);
                            	    sprintf( nname, "%s/new_%s", t, b);
                        	}

        			nfp = fopen( nname, "w" );
        			if( !nfp )
        			{
                        	    errmsg( FLASH, "Can't open new config file for write" );
                        	    fclose( fp );
                        	    break;
                        	}

        			while( fgets( buf, sizeof( buf ), fp ) )
        			{
        			    if( strncmp( buf, "PATH=", 5 ) == 0 )
        			    	fprintf( nfp, "PATH=%s\n",
						options[ PATH_IDX ] );
        			    else if( strncmp( buf, "LEVELS=", 7 ) == 0 )
        			    	fprintf( nfp, "LEVELS=%s\n",
						options[ LEVELS_IDX ] );
        			    else if( strncmp( buf, "PENS=", 5 ) == 0 )
        			    	fprintf( nfp, "PENS=%s\n",
						options[ PENS_IDX ] );
        			    else if( strncmp( buf, "OPTIONS=", 8 ) == 0 )
        			    	fprintf( nfp, "OPTIONS=%s\n",
						options[ OPTIONS_IDX ] );
        			    else if( strncmp( buf, "SAVE=", 5 ) == 0 )
        			    	fprintf( nfp, "SAVE=%s\n",
						options[ SAVE_IDX ] );
        			    else if( strncmp( buf, "HACKDIR=", 8 ) == 0 )
        			    	fprintf( nfp, "HACKDIR=%s\n",
						options[ HACKDIR_IDX ] );
        			    else
        			    {
        			    	fputs( buf, nfp );
        			    }
        			}
        			fclose( fp );
        			fclose( nfp );
        			unlink( oname );
        			rename( StrConf, oname );
        			rename( nname, StrConf );
        		    }
			    break;

			default:
			    break;
		    }
		    break;
	    }
	}
    }

    setoneopt( PATH_IDX, StrPath );
    setoneopt( HACKDIR_IDX, StrHackdir );
    setoneopt( PENS_IDX, StrPens );
    setoneopt( LEVELS_IDX, StrLevels );
    setoneopt( SAVE_IDX, StrSave );

    SafeCloseWindow( cwin );

    /* Display icons in possibly new save directory */

    MapGadgets( R_DISK, 1 );
}

void
UpdateCnfFile()
{
    FILE *fp, *nfp;
    char buf[ 300 ];
    char path=0,option=0,dir=0,pens=0,levels=0,save=0;
    char oname[ 300 ], nname[ 300 ];

    setoneopt( PATH_IDX, StrPath );
    PutOptions( curopts );
    setoneopt( HACKDIR_IDX, StrHackdir );
    setoneopt( PENS_IDX, StrPens );
    setoneopt( LEVELS_IDX, StrLevels );
    setoneopt( SAVE_IDX, StrSave );

    strcpy( oname, dirname( StrConf ) );
    if( oname[ strlen(oname)-1 ] != ':' )
    {
	sprintf( nname, "%s/new_nethack.cnf", oname );
	strcat( oname, "/" );
	strcat( oname, "old_nethack.cnf" );
    }
    else
    {
	sprintf( nname, "%snew_nethack.cnf", oname );
	strcat( oname, "old_nethack.cnf" );
    }

    fp = fopen( StrConf, "r" );
    if( !fp )
    {
        errmsg( FLASH, "Can't open nethack.cnf" );
		return;
    }
    nfp = fopen( nname, "w" );
    if( !nfp )
    {
        sprintf( buf, "Can't open %s for write", nname );
        errmsg( FLASH, buf );
        fclose( fp );
        return;
    }
    while( fgets( buf, sizeof( buf ), fp ) )
    {
        if( strncmp( buf, "PATH=", 5 ) == 0 )
	{
            fprintf( nfp, "PATH=%s\n", options[ PATH_IDX ] );
	    path=1;
	}
        else if( strncmp( buf, "LEVELS=", 7 ) == 0 )
	{
            fprintf( nfp, "LEVELS=%s\n", options[ LEVELS_IDX ] );
	    levels=1;
	}
        else if( strncmp( buf, "PENS=", 5 ) == 0 )
	{
            fprintf( nfp, "PENS=%s\n", options[ PENS_IDX ] );
	    pens=1;
	}
        else if( strncmp( buf, "OPTIONS=", 8 ) == 0 )
	{
            fprintf( nfp, "OPTIONS=%s\n", options[ OPTIONS_IDX ] );
	    option=1;
	}
        else if( strncmp( buf, "SAVE=", 5 ) == 0 )
	{
            fprintf( nfp, "SAVE=%s\n", options[ SAVE_IDX ] );
	    save=1;
	}
        else if( strncmp( buf, "HACKDIR=", 8 ) == 0 )
	{
            fprintf( nfp, "HACKDIR=%s\n", options[ HACKDIR_IDX ] );
	    dir=1;
	}
        else
        {
       	    fputs( buf, nfp );
        }
    }

    /* Write any that weren't already in the file */
    if( !path )
        fprintf( nfp, "PATH=%s\n", options[ PATH_IDX ] );
    if( !levels )
        fprintf( nfp, "LEVELS=%s\n", options[ LEVELS_IDX ] );
    if( !pens )
        fprintf( nfp, "PENS=%s\n", options[ PENS_IDX ] );
    if( !option )
        fprintf( nfp, "OPTIONS=%s\n", options[ OPTIONS_IDX ] );
    if( !save )
        fprintf( nfp, "SAVE=%s\n", options[ SAVE_IDX ] );
    if( !dir )
        fprintf( nfp, "HACKDIR=%s\n", options[ HACKDIR_IDX ] );

    /* Close up and rename files */
    fclose( fp );
    fclose( nfp );
    unlink( oname );
    if( filecopy( StrConf, oname ) == 0 )
	filecopy( nname, StrConf );
}

filecopy( from, to )
    char *from, *to;
{
    char *buf;
    int i = 0;

    buf = malloc( strlen(to) + strlen(from) + 20 );
    if( buf )
    {
    	sprintf( buf, "c:copy \"%s\" \"%s\" clone", from, to );

    	/* Check SysBase instead?  Shouldn't matter  */
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	    i = System( buf, NULL );
	else
	    Execute( buf, NULL, NULL );
    	free( buf );
    }
    else
    {
    	errmsg( FLASH, "Failed to allocate memory for copy command" );
    	return( -1 );
    }
    return( i );
}

void do_gadgetup( imsg )
    register struct IntuiMessage *imsg;
{
    register struct Gadget *gd;
    register unsigned long hid;
    int ncol, pot;

    gd = (struct Gadget *) imsg->IAddress;

    switch( gd->GadgetID )
    {
	case GADSCROLL:
	    hid = max( cols - vcols, 0 );
	    pot = ( ( struct PropInfo * ) gd->SpecialInfo )->HorizPot;
	    ncol = (hid * pot) / MAXPOT;
	    if( ncol != curcol )
	    {
		curcol = ncol;
		MapGadgets( R_SCROLL, 0 ); /* redisplay the icons */
	    }
	    break;
    }
}

void do_buttons( imsg )
    register struct IntuiMessage *imsg;
{
    if( imsg->Code == SELECTDOWN || imsg->Code == SELECTUP )
    {
	if( lastgaddown )
	{
	    SetGadgetUP( &lastgaddown->dobj->do_Gadget );
	    lastgaddown->secs = 0;
	    lastgaddown->mics = 0;
	}
	lastgaddown = NULL;
	ChgGameItems( &MenuList1, 0 );
    }
}

void do_gadgetdown( imsg )
    register struct IntuiMessage *imsg;
{
    register GPTR gptr;
    register struct Gadget *gd;

    gd = (struct Gadget *) imsg->IAddress;

    /* Don't do anything for these gadgets */

    if( gd->GadgetID < GADNEWGAME )
    {
	return;
    }

    /* Check just to make sure we got it */

    for( gptr = windowgads; gptr &&
	gptr->dobj->do_Gadget.GadgetID != gd->GadgetID; )
    {
	gptr = gptr->nextwgad;
    }

    if( !gptr )
    {
	errmsg( FLASH, "Bad GadgetID for GadgetDOWN" );
	return;
    }

    /* Fix the gadget images */

    if( lastgaddown && &lastgaddown->dobj->do_Gadget != gd )
    {
	SetGadgetUP( &lastgaddown->dobj->do_Gadget );
	gptr->secs = 0;
	gptr->mics = 0;
    }
    SetGadgetDOWN( &((lastgaddown = gptr)->dobj->do_Gadget) );

    /* Only allow game gadgets to be manipulated */

    if( lastgaddown->dobj->do_Gadget.GadgetID == GADNEWGAME )
	ChgNewGameItems( &MenuList1, 1 );
    else
	ChgGameItems( &MenuList1, 1 );

    /* Check if this gadget has been double clicked */

    if( DoubleClick( gptr->secs, gptr->mics, imsg->Seconds, imsg->Micros ) )
    {
	run_game( gptr );
	gptr->secs = 0;
	gptr->mics = 0;
	return;
    }

    gptr->secs = imsg->Seconds;
    gptr->mics = imsg->Micros;
}

void setopt( gptr )
    register GPTR gptr;
{
    CopyOptions( curopts, gptr );
    if( EditOptions( curopts, gptr ) )
	SetOptions( curopts, gptr );
}

void menu_info()
{
    int itemno;
    register struct IntuiMessage *imsg;
    char *t;
    register GPTR gptr;
    register struct Gadget *gd;
    register struct FileInfoBlock *finfo;
    register struct Window *cwin;
    register int i;
    int done = 0;
    long lock, olock;
    char **sp;
    static int once = 0;
    long code, class, qual;
    static struct IntuiText itext[ 2 ];
    char commentstr[ 100 ], *s;
    int txtdiff = scrn->RastPort.TxHeight - 8;

    if( ( gptr = NeedGame() ) == NULL )
	return;

    if( ( lock = Lock( GameName( gptr, NULL ), ACCESS_READ ) ) == NULL )
    {
	/* Can't get lock, reload and return */

	errmsg( FLASH, "Can't Lock game save file: %s",
		GameName( gptr, NULL ) );
	MapGadgets( R_DISK, 1 );
	return;
    }

    finfo = (struct FileInfoBlock *)xmalloc(sizeof(struct FileInfoBlock));
    Examine( lock, finfo );
    UnLock( lock );
    strncpy( commentstr, finfo->fib_Comment, sizeof( finfo->fib_Comment ) );
    commentstr[ sizeof( finfo->fib_Comment ) ] = 0;
    free( finfo );

    for( i = 0; i < 2; ++i )
    {
	itext[ i ].FrontPen = C_BLACK;
	itext[ i ].BackPen = C_GREY;
	itext[ i ].DrawMode = JAM2;
	itext[ i ].TopEdge = 2;
	itext[ i ].LeftEdge = 4;
    }
    ReallocTools( gptr, 0 );

    if( !once )
    {
    	Info_Comment.TopEdge += txtdiff*2;
	Info_NewWindowStructure6.Height += txtdiff * 7;
	for( gd = Info_NewWindowStructure6.FirstGadget;
		gd; gd = gd->NextGadget )
	{
	    gd->TopEdge += txtdiff;
	    if( gd->GadgetID != GADTOOLUP && gd->GadgetID != GADTOOLDOWN )
		gd->Height += txtdiff;
	    switch( gd->GadgetID )
	    {
		case 0:
		    break;

		case GADEDITOPTS:
		    gd->TopEdge += txtdiff*3;
		    SetBorder( gd, -1 );
		    break;

		case GADTOOLTYPES:
		    gd->TopEdge += txtdiff*4;
		    if( scrn->Height > 300 )
			gd->TopEdge += 2;
		    SetBorder( gd, -1 );
		    break;

		case GADDELTOOL:
		    gd->TopEdge += txtdiff*3;
		    SetBorder( gd, -1 );
		    break;

		case GADADDTOOL:
		    gd->TopEdge += txtdiff*3;
		    SetBorder( gd, -1 );
		    break;

		case GADSAVEINFO:
		    gd->TopEdge += txtdiff*5;
		    SetBorder( gd, -1 );
		    break;

		case GADQUITINFO:
		    gd->TopEdge += txtdiff*5;
		    SetBorder( gd, -1 );
		    break;

		case GADUSEINFO:
		    gd->TopEdge += txtdiff*5;
		    SetBorder( gd, -1 );
		    break;

		case GADTOOLUP:
		    gd->TopEdge += txtdiff*4;
		    gd->Flags &= ~GADGHIGHBITS;
		    gd->Flags |= GADGIMAGE|GADGHIMAGE;
		    if( scrn->Height > 300 )
		    {
			gd->GadgetRender = (APTR)&tall_up_selectimage;
			gd->SelectRender = (APTR)&tall_up_renderimage;
			gd->Height *= 2;
			if( txtdiff == 0 )
			    gd->TopEdge -= 2;
		    }
		    else
		    {
			gd->GadgetRender = (APTR)&up_selectimage;
			gd->SelectRender = (APTR)&up_renderimage;
		    }
		    break;

		case GADTOOLDOWN:
		    gd->TopEdge += txtdiff*5;
		    gd->Flags &= ~GADGHIGHBITS;
		    gd->Flags |= GADGIMAGE|GADGHIMAGE;
		    if( scrn->Height > 300 )
		    {
			gd->GadgetRender = (APTR)&tall_down_selectimage;
			gd->SelectRender = (APTR)&tall_down_renderimage;
			gd->Height *= 2;
			if( txtdiff == 0 )
			    gd->TopEdge += 4;
		    }
		    else
		    {
			gd->GadgetRender = (APTR)&down_selectimage;
			gd->SelectRender = (APTR)&down_renderimage;
		    }
		    break;

		default:
		    SetBorder( gd, -1 );
	    }
	}

	++once;
    }

    strncpy( Sbuff( &Info_Comment ), commentstr, 100 );

    /* The players name */

    strncpy( StrPlayer, ToolsEntry( gptr, "NAME" ), 100 );
    if( *StrPlayer == 0 )
    {
	strncpy( StrPlayer, gptr->name, 99 );
    }

    if( ( t = strrchr( StrPlayer, '.' ) ) && stricmp( t, ".sav" ) == 0 )
    {
	*t = 0;
    }

    /* The character class of the player */

    Info_Class.GadgetText = &itext[ 0 ];
    itext[ 0 ].IText = ToolsEntry( gptr, "CHARACTER" );
    if( *itext[ 0 ].IText == 0 )
    {
	itext[ 0 ].IText = players[ 0 ];
	SetToolLine( gptr, "CHARACTER", players[ 0 ] );
    }

    /* If there are ToolTypes entries, put the first one into the gadget */

    sp = gptr->dobj->do_ToolTypes;
    if( sp && *sp )
	strcpy( StrTools, *sp );

    if( IsEditEntry( StrTools, gptr ) )
	Info_ToolTypes.Flags &= ~GADGDISABLED;
    else
	Info_ToolTypes.Flags |= GADGDISABLED;

    Info_NewWindowStructure6.Screen = scrn;
    if( ( cwin = MyOpenWindow( &Info_NewWindowStructure6 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create info window" );
	return;
    }

    itemno = 0;
    if( s = FindToolType( (char **) gptr->dobj->do_ToolTypes, "CHARACTER" ) )
    {
	s += 10;
	for( itemno = 0; players[ itemno ]; ++itemno )
	{
	    if( strnicmp( s, players[ itemno ], strlen( s ) ) == 0 )
		break;
	}
    }
    if( !players[ itemno ] )
	itemno = 0;

    CheckOnly( &Info_MenuList6, 0, itemno );
    SetUpMenus( &Info_MenuList6, scrn );
    SetMenuStrip( cwin, &Info_MenuList6 );

    while( !done )
    {
	WaitPort( cwin->UserPort );
	while( ( imsg = (void *) GetMsg( cwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *)imsg->IAddress;

	    ReplyMsg( (struct Message *)imsg );

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == '\33' || (code == 'b' && (qual&AMIGALEFT)) )
		    {
			done = 1;
		    }
		    break;

		case CLOSEWINDOW:
		    if( sp )
		    {
			if( *sp )
			    free( *sp );
			*sp = strdup( StrTools );
		    }
		    done = 1;
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
		    case GADSAVEINFO:
			/* Write icon and quit. */
			SetToolLine( gptr, "NAME", StrPlayer );
			UpdateGameIcon( gptr );
			lock = Lock( gptr->dname, ACCESS_READ );
			if( lock )
			{
			    olock = CurrentDir( lock );
			    SetComment( gptr->fname, Sbuff( &Info_Comment ) );
			    CurrentDir( olock );
			    done = 1;
			}
			else
			{
			    errmsg( FLASH, "Can't access icon's directory" );
			    sp = gptr->dobj->do_ToolTypes;
			    strcpy( StrTools, *sp ? *sp : "" );
			    UpdateInfoWin( cwin );
			}
			break;

		    case GADUSEINFO:
			/* Quit this loop. */
			done = 1;
			break;

		    case GADQUITINFO:
			/* Reload icon and quit this loop. */
			RemoveGadget( win, &gptr->dobj->do_Gadget );
			RemoveGITEM( gptr );
			lastgaddown = NULL; /* very important... */
			MapGadgets( R_DISK, 1 );
			done = 1;
			break;

		    case GADEDITOPTS:
			setopt( gptr );
			sp = gptr->dobj->do_ToolTypes;
			strcpy( StrTools, *sp ? *sp : "" );
			UpdateInfoWin( cwin );
			break;

		    case GADADDTOOL:
			FreeTools( gptr );
			ReallocTools( gptr, 2 );
			sp = gptr->dobj->do_ToolTypes;
			for( i = 0; sp[ i ]; ++i )
			    ;
			sp[i] = strdup( "" );
			sp[i+1] = NULL;
			itext[ 0 ].IText =
			    ToolsEntry( gptr, "CHARACTER" );
			*StrTools = 0;
			Info_ToolTypes.Flags &= ~GADGDISABLED;
			UpdateInfoWin( cwin );
			sp += i;
			break;

		    case GADDELTOOL:
			while( *sp = sp[1] )
			    ++sp;
			sp = gptr->dobj->do_ToolTypes;
			strcpy( StrTools, *sp ? *sp : "" );
			Info_ToolTypes.Flags &= ~GADGDISABLED;
			UpdateInfoWin( cwin );
			break;

		    case GADTOOLTYPES:
			if( sp && *sp && CheckAndCopy( StrTools, *sp ) )
			{
			    if( *sp )
				free( *sp );
			    *sp = strdup( StrTools );
			}
			break;

		    case GADTOOLDOWN:
			if( sp && *sp && CheckAndCopy( StrTools, *sp ) )
			{
			    if( *sp )
				free( *sp );
			    *sp = strdup( StrTools );
			}

			if( sp && sp[0] && sp[1] )
			{
			    ++sp;
			    strcpy( StrTools, *sp );
			    if( IsEditEntry( StrTools, gptr ) )
				Info_ToolTypes.Flags &= ~GADGDISABLED;
			    else
				Info_ToolTypes.Flags |= GADGDISABLED;
			}
			else
			{
			    if( sp && *sp )
				strcpy( StrTools, *sp );
			    DisplayBeep( NULL );
			}
			break;

		    case GADTOOLUP:
			if( sp && *sp && CheckAndCopy( StrTools, *sp ) )
			{
			    if( *sp )
				free( *sp );
			    *sp = strdup( StrTools );
			}
			if( sp && sp > gptr->dobj->do_ToolTypes )
			{
			    --sp;
			    if( *sp )
			    {
				strcpy( StrTools, *sp );
				if( IsEditEntry( StrTools, gptr ) )
				    Info_ToolTypes.Flags &= ~GADGDISABLED;
				else
				    Info_ToolTypes.Flags |= GADGDISABLED;
			    }
			}
			else
			{
			    DisplayBeep( NULL );
			}
			break;

		    case GADPLNAME:
			SetToolLine( gptr, "NAME", StrPlayer );
			sp = gptr->dobj->do_ToolTypes;
			strcpy( StrTools, *sp ? *sp : "" );
			UpdateInfoWin( cwin );
			break;
		    }
		    RefreshGList( &Info_ToolTypes, cwin, NULL, 1 );
		    break;

	    	case MENUPICK:
	    	    while( code != MENUNULL )
	    	    {
			SetToolLine( gptr, "CHARACTER",
				    players[ ITEMNUM( code ) ] );
			itext[ 0 ].IText =
				    ToolsEntry( gptr, "CHARACTER" );
			sp = gptr->dobj->do_ToolTypes;
			strcpy( StrTools, *sp ? *sp : "" );
			UpdateInfoWin( cwin );
			code = ((struct MenuItem *)ItemAddress(
				&Info_MenuList6, (long)code ))->NextSelect;
		    }
		    break;
	    }
	}
    }

    SafeCloseWindow( cwin );
}

static void
UpdateInfoWin( cwin )
    struct Window *cwin;
{
    SetAPen( cwin->RPort, 0 );
    SetBPen( cwin->RPort, 0 );
    SetDrMd( cwin->RPort, JAM2 );
    RectFill( cwin->RPort,
	Info_Class.LeftEdge,
	Info_Class.TopEdge,
	Info_Class.LeftEdge + Info_Class.Width-1,
	Info_Class.TopEdge + Info_Class.Height-1 );
    RefreshGList( cwin->FirstGadget, cwin, NULL, -1 );
}

void
errmsg(int flash, char *str, ...)
{
    static char buf[ 200 ];
    int wid;
    va_list vp;

    va_start( vp, str );

    if( !win || !wbopen )
    {
	vprintf( str, vp );
	va_end( vp );
	return;
    }
    errup = 1;
    wid = ( win->Width + Message.LeftEdge - win->BorderRight - 3 ) /
		    win->RPort->TxWidth;
    vsprintf( buf, str, vp );
    va_end( vp );

    SetAPen( win->RPort, 0 );
    SetBPen( win->RPort, 0 );
    SetDrMd( win->RPort, JAM2 );
    RectFill( win->RPort, Message.LeftEdge, Message.TopEdge,
	win->Width + Message.Width,
	Message.TopEdge + Message.Height - 1 );

    Message.GadgetText->IText = buf;
    RefreshGList( &Message, win, 0, 1 );

    if( flash == FLASH )
	DisplayBeep( scrn );
}

/*
 * Issue an error message to the users window because it can not be done
 * any other way.
 */

void error( str )
    register char *str;
{
    char s[ 50 ];
    if( scrn ) ScreenToBack( scrn );
    Delay( 10 );
    fprintf( stderr, "%s\n", str );
    fprintf( stderr, "Hit Return: " );
    fflush( stderr );
    gets( s );
    if( scrn ) ScreenToFront( scrn );
}

/*
 * Make the gadget deselected
 */

void SetGadgetUP( gad )
    register struct Gadget *gad;
{
    if( gad->Flags & GADGIMAGE )
    {
	DrawImage( win->RPort, (struct Image *)gad->GadgetRender,
		gad->LeftEdge, gad->TopEdge );
    }
#if 0
    RemoveGadget( win, gad );
    gad->Flags &= ~(SELECTED|GADGHIGHBITS);
    gad->Flags |= GADGHIMAGE|GADGIMAGE;
    gad->Activation |= TOGGLESELECT;
    AddGadget( win, gad, 0 );
    RefreshGList( gad, win, NULL, 1 );
    RemoveGadget( win, gad );
    gad->Flags &= ~(GADGHIGHBITS);
    gad->Flags |= GADGHNONE;
    gad->Activation &= ~TOGGLESELECT;
    AddGadget( win, gad, 0 );
#endif
}

/*
 * Make the gadget selected
 */

void SetGadgetDOWN( gad )
    register struct Gadget *gad;
{
    if( gad->Flags & GADGHIMAGE )
    {
	DrawImage( win->RPort, (struct Image *)gad->SelectRender,
		gad->LeftEdge, gad->TopEdge );
    }
#if 0
    RemoveGadget( win, gad );
    gad->Flags &= ~GADGHIGHBITS;
    gad->Flags |= GADGHIMAGE|GADGIMAGE|SELECTED;
    gad->Activation |= TOGGLESELECT;
    AddGadget( win, gad, 0 );
    RefreshGList( gad, win, NULL, 1 );
    RemoveGadget( win, gad );
    gad->Flags &= ~(GADGHIGHBITS);
    gad->Flags |= GADGHNONE;
    gad->Activation &= ~TOGGLESELECT;
    AddGadget( win, gad, 0 );
#endif
}

/*
 * Generate a requester for a string value.
 */

int StrRequest( prompt, buff, val )
    char *prompt, *buff, *val;
{
    struct Window *cwin;
    struct IntuiMessage *imsg;
    int done = 0, notcan = 1;
    long class, code, qual;
    struct Gadget *gd;
    static int once = 0;
    int txtdiff = scrn->RastPort.TxHeight - 8;

    *StrString = 0;
    if( val )
	strcpy( StrString, val );
    Str_NewWindowStructure5.Title = prompt;

    if( !once )
    {
	for( gd = Str_NewWindowStructure5.FirstGadget;
		    gd; gd = gd->NextGadget )
	{
	    if( gd->GadgetID != 0 )
	    {
		gd->TopEdge += txtdiff;
		gd->Height += txtdiff;
		SetBorder( gd, -1 );
	    }
	}
	++once;
	Str_NewWindowStructure5.Height += txtdiff * 2;
    }

    Str_NewWindowStructure5.Screen = scrn;
    if( ( cwin = MyOpenWindow( &Str_NewWindowStructure5 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create requester window" );
	return( 0 );
    }

    while( !done )
    {
	WaitPort( cwin->UserPort );
	while( ( imsg = (void *) GetMsg( cwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *) imsg->IAddress;
	    ReplyMsg( (struct Message *) imsg );
	    switch( class )
	    {
		case ACTIVEWINDOW:
		    ActivateGadget( &Str_String, cwin, NULL );
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
			case GADSTRCANCEL:
			    notcan = 0;
			    done = 1;
			    break;

			default:
			    strcpy( buff, StrString );
			    done = 1;
			    break;
		    }
		    break;

		case CLOSEWINDOW:
		    strcpy( buff, StrString );
		    done = 1;
		    break;

		case VANILLAKEY:
		    if( code == '\33' || code == 'b' && (qual&AMIGALEFT) )
		    {
			done = 1;
			notcan = 0;
		    }
		    break;
	    }
	}
    }

    SafeCloseWindow( cwin );
    return( notcan );
}

/*
 * Ask the user if they really want to do something.
 */

Ask( quest )
    char *quest;
{
    register struct Window *qwin;
    register struct Gadget *gd;
    register struct IntuiMessage *imsg;
    register int done = 0, quit = 1;
    int txtdiff;
    long class, code, qual;
    static int once = 0;
    static WORD areabuffer[ 80 ];
    static USHORT apat[] = { 0x5555, 0xaaaa };
    static struct AreaInfo areaInfo = { 0 };
    PLANEPTR pp;
    struct TmpRas tmpras;

    Quest_NewWindowStructure2.Screen = scrn;
    txtdiff = scrn->RastPort.TxHeight - 8;
    if( !once )
    {
	Quest_IntuiTextList2.TopEdge += txtdiff;
	Quest_Borders2.TopEdge += txtdiff;
	Quest_NewWindowStructure2.Height += txtdiff * 2;
	SetBorder( &Quest_Borders2, 3 );
	Quest_Yes.TopEdge += txtdiff;
	Quest_Yes.Height += txtdiff;
	SetBorder( &Quest_Yes, -1 );
	Quest_No.TopEdge += txtdiff;
	Quest_No.Height += txtdiff;
	SetBorder( &Quest_No, -1 );
    }

    memset( areabuffer, 0, sizeof( areabuffer ) );
    if( ( qwin = MyOpenWindow( &Quest_NewWindowStructure2 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create requester window" );
	return( 1 );
    }

    pp = AllocRaster( qwin->Width, qwin->Height );
    if( pp )
    {
	InitArea( &areaInfo, areabuffer, 160/5 );
	qwin->RPort->AreaInfo = &areaInfo;

	InitTmpRas( &tmpras, pp, RASSIZE( qwin->Width, qwin->Height ) );
	qwin->RPort->TmpRas = &tmpras;

	SetAPen( qwin->RPort, C_WHITE );
	SetBPen( qwin->RPort, C_GREY );
	SetDrMd( qwin->RPort, JAM2 );
	SetAfPt( qwin->RPort, apat, 1 );

	AreaMove( qwin->RPort, qwin->BorderLeft, qwin->BorderTop );
	AreaDraw( qwin->RPort, qwin->Width-qwin->BorderRight, qwin->BorderTop );
	AreaDraw( qwin->RPort, qwin->Width - qwin->BorderRight,
		qwin->Height - qwin->BorderBottom );
	AreaDraw( qwin->RPort, qwin->BorderLeft,
		qwin->Height - qwin->BorderBottom );
	AreaDraw( qwin->RPort, qwin->BorderLeft, qwin->BorderTop );
	AreaEnd( qwin->RPort );

	SetAPen( qwin->RPort, C_GREY );
	SetBPen( qwin->RPort, C_GREY );
	SetDrMd( qwin->RPort, JAM2 );
	SetAfPt( qwin->RPort, NULL, 0 );

	RectFill( qwin->RPort,
		Quest_Borders2.LeftEdge,
		Quest_Borders2.TopEdge,
		Quest_Borders2.LeftEdge + Quest_Borders2.Width - 1,
		Quest_Borders2.TopEdge + Quest_Borders2.Height - 1 );
	RectFill( qwin->RPort,
		Quest_No.LeftEdge,
		Quest_No.TopEdge,
		Quest_No.LeftEdge + Quest_No.Width - 1,
		Quest_No.TopEdge + Quest_No.Height - 1 );
	RectFill( qwin->RPort,
		Quest_Yes.LeftEdge,
		Quest_Yes.TopEdge,
		Quest_Yes.LeftEdge + Quest_Yes.Width - 1,
		Quest_Yes.TopEdge + Quest_Yes.Height - 1 );
	RefreshGList( qwin->FirstGadget, qwin, NULL, -1 );
    }

    Quest_IntuiTextList2.LeftEdge = ( qwin->Width -
	( qwin->RPort->TxWidth * strlen( quest ) ) ) / 2;
    Quest_IntuiTextList2.IText = quest;
    PrintIText( qwin->RPort, &Quest_IntuiTextList2, 0, 0 );
    while( !done )
    {
	WaitPort( qwin->UserPort );
	while( ( imsg = (void *) GetMsg( qwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *)imsg->IAddress;

	    ReplyMsg( (struct Message *)imsg );

	    switch( class )
	    {
		case VANILLAKEY:
		    if( imsg->Qualifier & AMIGALEFT )
		    {
			switch( imsg->Code )
			{
			    case 'v': done = 1; quit = 0; break;
			    case '\33':
			    case 'b': done = 1; quit = 1; break;
			}
		    }
		    break;

		case CLOSEWINDOW:
		    done = 1; quit = 1; break;
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
			case GADQUESTYES: done = 1; quit = 0; break;
			case GADQUESTNO: done = 1; quit = 1; break;
		    }
		    break;
	    }
	}
    }

    if( pp )
	FreeRaster( pp, qwin->Width, qwin->Height );
    once = 1;
    SafeCloseWindow( qwin );
    return( quit == 0 );
}

/* Make sure that a game icon is selected and return the pointer to
 * the GPTR structure associated with it.
 */

GPTR NeedGame()
{
    register GPTR gptr;

    if( lastgaddown == NULL )
    {
	errmsg( FLASH, "Must select a game" );
	return( NULL );
    }

    for( gptr = windowgads; gptr; gptr = gptr->nextwgad )
    {
	if( &gptr->dobj->do_Gadget == &lastgaddown->dobj->do_Gadget )
	    break;
    }

    if( !gptr )
    {
	errmsg( FLASH, "BUG: invalid gadget selected for processing" );
	return( NULL );
    }
    return( gptr );
}

/* Set menu items SELECT flag based on 'enable' */

void ChgGameItems( menup, enable )
    struct Menu *menup;
    int enable;
{
    struct MenuItem *ip;
    int i;
    int ino;

    /* Make sure the 'Game' menu is there. */

    if( !menup || !(menup = menup->NextMenu) || !( ip = menup->FirstItem ) )
    {
	errmsg( FLASH, "BUG: invalid menu to disable with" );
	return;
    }

    /* Go through all items */

    for( i = 0; ip; ip = ip->NextItem, ++i )
    {
	switch( i )
	{
	    case ITEM_INFO:
	    case ITEM_COPYOPT:
	    case ITEM_DISCARD:
	    case ITEM_RENAME:
		ino = MENUITEMNO( 1,i,NOSUB );
		if( enable )
		    OnMenu( win, ino );
		else
		    OffMenu( win, ino );
		break;
	}
    }
}

/* Set menu items SELECT flag based on 'enable' for NEWGAME gadget */

void ChgNewGameItems( menup, enable )
    struct Menu *menup;
    int enable;
{
    struct MenuItem *ip;
    int i;
    int ino;

    /* Make sure the 'Game' menu is there. */

    if( !menup || !(menup = menup->NextMenu) || !( ip = menup->FirstItem ) )
    {
	errmsg( FLASH, "BUG: invalid menu to disable with" );
	return;
    }

    /* Go through all items */

    for( i = 0; ip; ip = ip->NextItem, ++i )
    {
	switch( i )
	{
	    case ITEM_RENAME:
	    case ITEM_DISCARD:
		ino = MENUITEMNO( 1,i,NOSUB );
		OffMenu( win, ino );
		break;

	    case ITEM_COPYOPT:
	    case ITEM_INFO:
		ino = MENUITEMNO( 1,i,NOSUB );
		if( enable )
		    OnMenu( win, ino );
		else
		    OffMenu( win, ino );
		break;
	}
    }
}

/* Edit the OPTIONS= line with a window.  The optr[] array is set up
 * for the editing already, and will be returned with the values
 * of the members changed based on the users input
 */

int EditOptions( optr, gptr )
    OPTR optr;
    GPTR gptr;
{
    int done = 0, quit = 0;
    register struct Window *cwin;
    register struct IntuiMessage *imsg;
    register struct Gadget *gd;
    long code, class, qual;
    struct IntuiText *ip;
    static int once = 0;
    int i;
    int txtdiff = scrn->RastPort.TxHeight - 8;

    if( !once )
    {
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOLITCORRIDOR ))
	{
	    struct Gadget *g;
	    for( g = Options_NewWindowStructure3.FirstGadget;
			    g; g = g->NextGadget )
	    {
		if( g == gd )
		    continue;
		if( g->TopEdge == gd->TopEdge )
		{
		    g->Height += txtdiff;
		}
	    }
	    gd->Height += txtdiff;
	}

	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOTIME ))
	{
	    struct Gadget *g;
	    for( g = Options_NewWindowStructure3.FirstGadget;
			    g; g = g->NextGadget )
	    {
		if( g == gd )
		    continue;
		if( g->TopEdge == gd->TopEdge )
		{
		    g->TopEdge += txtdiff;
		    g->Height += txtdiff;
		}
	    }
	    gd->TopEdge += txtdiff;
	    gd->Height += txtdiff;
	}

	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOPICKUP ))
	{
	    struct Gadget *g;
	    for( g = Options_NewWindowStructure3.FirstGadget; g; g = g->NextGadget )
	    {
		if( g == gd )
		    continue;
		if( g->TopEdge == gd->TopEdge )
		{
		    g->TopEdge += txtdiff*2;
		    g->Height += txtdiff;
		}
	    }
	    gd->TopEdge += txtdiff*2;
	    gd->Height += txtdiff;
	}

	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOHILITEPET ))
	{
	    struct Gadget *g;
	    for( g = Options_NewWindowStructure3.FirstGadget; g; g = g->NextGadget )
	    {
		if( g == gd )
		    continue;
		if( g->TopEdge == gd->TopEdge )
		{
		    g->TopEdge += txtdiff*3;
		    g->Height += txtdiff;
		}
	    }
	    gd->TopEdge += txtdiff*3;
	    gd->Height += txtdiff;
	}

	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOPACKORDER ))
	{
	    gd->TopEdge += txtdiff*4;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOPICKUPTYPES ))
	{
	    gd->TopEdge += txtdiff*4;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOCATNAME ))
	{
	    gd->TopEdge += txtdiff*5;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOWINDOWTYPE ))
	{
	    gd->TopEdge += txtdiff*5;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADODOGNAME ))
	{
	    gd->TopEdge += txtdiff*6;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOMSGHISTORY ))
	{
	    gd->TopEdge += txtdiff*6;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOFRUIT ))
	{
	    gd->TopEdge += txtdiff*7;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOPALETTE ))
	{
	    gd->TopEdge += txtdiff*7;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOOBJECTS ))
	{
	    gd->TopEdge += txtdiff*8;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOSCORE ))
	{
	    gd->TopEdge += txtdiff*8;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADONAME ))
	{
	    gd->TopEdge += txtdiff*9;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOPETTYPE ))
	{
	    gd->TopEdge += txtdiff*9;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOOKAY ))
	{
	    gd->TopEdge += txtdiff*10;
	    gd->Height += txtdiff;
	}
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, GADOCANCEL ))
	{
	    gd->TopEdge += txtdiff*10;
	    gd->Height += txtdiff;
	}
	Options_NewWindowStructure3.Height += txtdiff*12;
	Options_NewWindowStructure3.TopEdge -= txtdiff*12;
	if( Options_NewWindowStructure3.Height +
	    Options_NewWindowStructure3.TopEdge >= scrn->Height )
	{
	    Options_NewWindowStructure3.TopEdge = scrn->Height -
	    Options_NewWindowStructure3.Height - 1;
	}
	if( Options_NewWindowStructure3.TopEdge < 0 )
	    Options_NewWindowStructure3.TopEdge = 0;
	if( Options_NewWindowStructure3.Height > scrn->Height )
	    Options_NewWindowStructure3.Height = scrn->Height;

	/* Now that heights are correct, render borders */
	for( gd = Options_NewWindowStructure3.FirstGadget;
	    gd; gd = gd->NextGadget )
	{
	    if( gd->GadgetID != 0 )
	    {
		gd->TopEdge += txtdiff;
		SetBorder( gd, -1 );
	    }
	}

	for( ip = &Options_IntuiTextList3; ip; ip = ip->NextText )
	{
	    /* Pack Order:  and  Pickup: */
	    if( ( *ip->IText == 'P' && ip->IText[2] == 'c' ) )
	    {
		ip->TopEdge += txtdiff * 4;
	    }
	    /* Cat Name:  and  Window Type: */
	    else if( ( *ip->IText == 'C' ) ||
		    ( *ip->IText == 'W' ) )
	    {
		ip->TopEdge += txtdiff * 5;
	    }
	    /* Dog Name:  and  Msg History: */
	    else if( ( *ip->IText == 'D' ) ||
		    ( *ip->IText == 'M' ) )
	    {
		ip->TopEdge += txtdiff * 6;
	    }
	    /* Fruit:  and  Pallete: */
	    else if( ( *ip->IText == 'F' ) ||
		    ( *ip->IText == 'P' && ip->IText[2] == 'l' ) )
	    {
		ip->TopEdge += txtdiff * 7;
	    }
	    /* Objects:  and  Score: */
	    else if( ( *ip->IText == 'O' ) ||
	    		( *ip->IText == 'S' ) )
	    {
		ip->TopEdge += txtdiff * 8;
	    }
	    /* Name:  and  Pet Type: */
	    else if(( *ip->IText == 'N' ) ||
		    ( *ip->IText == 'P' && ip->IText[1] == 'e' ) )
	    {
		ip->TopEdge += txtdiff * 9;
	    }
	}
	once = 1;
    }

    /* Set Gadgets based on options settings */

    for( i = 0; optr[ i ].name; ++i )
    {
	if( gd = FindGadget( NULL, &Options_NewWindowStructure3, optr[i].id ))
	{
	    /* If string valued option, set string */
	    if( optr[ i ].optstr )
	    {
	    	char *t;

		if( optr[ i ].id == GADOPALETTE &&
				    ( optr[i].optstr == NULL || *optr[i].optstr == 0 ) )
		{
		    if( gptr && ( t = ToolsEntry( gptr, "PENS" ) ) )
		    {
			strcpy( Sbuff( gd ), t );
		    }
		    else
		    {
			strcpy( Sbuff( gd ), options[ PENS_IDX ] );
		    }
		    for( t = strchr( Sbuff( gd ), ',' ); t; t = strchr( t, ',' ) )
		    	*t = '/';
		}
		else
		    strcpy( Sbuff( gd ), optr[i].optstr );
	    }
	    else
	    {
		/* If binary option, set the gadget state */
		if( optr[i].optval )
		    gd->Flags |= SELECTED;
		else
		    gd->Flags &= ~SELECTED;
	    }
	}
	else
	{
	    errmsg( FLASH, "Can't find gadget %d in options window",
								optr[i].id);
	}
    }

    Options_NewWindowStructure3.Screen = scrn;
    if( ( cwin = MyOpenWindow( &Options_NewWindowStructure3 ) ) == NULL )
    {
	errmsg( FLASH, "Can't create requester window" );
	return(0);
    }
    PrintIText( cwin->RPort, &Options_IntuiTextList3, 0, txtdiff );

    while( !done )
    {
	WaitPort( cwin->UserPort );
	while( ( imsg = (void *) GetMsg( cwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *)imsg->IAddress;
	    ReplyMsg( (struct Message *) imsg );
	    switch( class )
	    {
		case ACTIVEWINDOW:
		    ActivateGadget(
			FindGadget( cwin, 0, GADOPACKORDER ), cwin, 0 );
		    break;

		case VANILLAKEY:
		    if( code == '\33' )
		    {
			done = 1;
			quit = 1;
		    }
		    else if( code == 'v' && (qual & AMIGALEFT) )
			done = 1;
		    else if( code == 'b' && (qual & AMIGALEFT) )
		    {
			done = 1;
			quit = 1;
		    }
		    break;

		case CLOSEWINDOW:
		    done = 1;
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
			case GADOPACKORDER:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOPICKUPTYPES ),
				cwin, 0 );
			    break;

			case GADOCATNAME:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOWINDOWTYPE ),
				cwin, 0 );
			    break;

			case GADODOGNAME:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOMSGHISTORY ),
				cwin, 0 );
			    break;

			case GADOFRUIT:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOPALETTE ),
				cwin, 0 );
			    break;

			case GADOOBJECTS:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOSCORE ),
				cwin, 0 );
			    break;

			case GADONAME:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOPETTYPE ),
				cwin, 0 );
			    break;

			case GADOPICKUPTYPES:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOCATNAME ),
				cwin, 0 );
			    break;

			case GADOWINDOWTYPE:
			    ActivateGadget(
				FindGadget( cwin, 0, GADODOGNAME ),
				cwin, 0 );
			    break;

			case GADOMSGHISTORY:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOFRUIT ),
				cwin, 0 );
			    break;

			case GADOPALETTE:
			    ActivateGadget(
				FindGadget( cwin, 0, GADOOBJECTS ),
				cwin, 0 );
			    break;

			case GADOSCORE:
			    ActivateGadget(
				FindGadget( cwin, 0, GADONAME ),
				cwin, 0 );
			    break;

			case GADOPETTYPE:
			    break;

			case GADOOKAY:
			    done = 1;
			    break;

			case GADOCANCEL:
			    quit = 1;
			    done = 1;
			    break;

			default:
			    for( i = 0; optr[i].name; ++i )
			    {
				if( optr[i].id == gd->GadgetID )
				    break;
			    }

			    if( optr[i].name )
			    {
				if( optr[ i ].optstr != NULL )
				{
				    if( *optr[i].optstr )
					free( optr[i].optstr );

				    if( *Sbuff(gd) == 0 )
				    {
					optr[i].optstr = "";
				    }
				    else
				    {
					optr[i].optstr = strdup(Sbuff(gd));
				    }
				}
				else
				{
				    optr[i].optval =
					    ( gd->Flags & SELECTED ) != 0;
				}
			    }
			    break;
		    }
		    break;
	    }
	}
    }

    SafeCloseWindow( cwin );
    return( quit == 0 );
}

/*
 * Put options structure into a string and then make that the
 * options[ OPTIONS_IDX ] value
 */

void PutOptions( optr )
    register OPTR optr;
{
    register struct Gadget *gd;
    register int i, olen = 4096, rlen, didone;
    register char *optbuf;

    while( olen > 256 )
    {
	if( ( optbuf = xmalloc( olen ) ) != NULL )
	{
	    break;
	}
	olen /= 2;
    }

    if( optbuf == NULL )
    {
	errmsg( FLASH, "No memory left for options buffer" );
	return;
    }

    /* Account for nul terminator */
    *optbuf = 0;

    for( i = 0; optr[i].name; ++i )
    {
	--olen;
	didone = 0;
	rlen = 0;
	gd = FindGadget( 0, &Options_NewWindowStructure3, optr[i].id );

	/* If name:value option */
	if( optr[i].optstr != NULL )
	{
	    /* If gadget contains some text */
	    if( gd && *Sbuff( gd ) )
	    {
		/* Free a previously allocated string */
		if( optr[i].optstr && *optr[i].optstr )
		    free( optr[i].optstr );

		/* Store "" or save string away */
		if( *Sbuff(gd) == 0 )
		    optr[i].optstr = "";
		else
		    optr[i].optstr = strdup(Sbuff(gd));

		rlen = strlen( optr[i].optstr ) + strlen( optr[i].name ) + 1;
		if( rlen <= olen )
		{
		    sprintf( optbuf + strlen(optbuf), "%s:%s",
			optr[i].name, optr[i].optstr );
		}
		didone = 1;
	    }
	}
	else
	{
	    if( optr[i].optval != optr[i].defval )
	    {
		if( olen >= (rlen = strlen( optr[i].name ) +
			(optr[i].optval == 0 ) ) )
		{
		    if( optr[i].optval == 0 )
			strcat( optbuf, "!" );
		    strcat( optbuf, optr[i].name );
		}
		didone = 1;
	    }
	}

	if( rlen > olen )
	{
	    errmsg( FLASH, "Out of space for options" );
	    break;
	}
	if( didone )
	    strcat( optbuf, "," );
    }

    /* Remove trailing ',' */

    if( *optbuf )
	optbuf[ strlen( optbuf ) - 1 ] = 0;

    setoneopt( OPTIONS_IDX, optbuf );
    free( optbuf );
}

char *basename( str )
    char *str;
{
    char *t;

    t = strrchr( str, '/' );
    if( !t )
	t = strrchr( str, ':' );
    if( !t )
	t = str;
    else
	++t;
    return( t );
}
