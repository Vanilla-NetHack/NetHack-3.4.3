/*    SCCS Id: @(#)winami.c    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "amiga:windefs.h"
#include "amiga:winext.h"
#include "amiga:winproto.h"

#ifdef AMIGA_INTUITION

#ifdef	SHAREDLIB
struct DosLibrary *DOSBase;
#else
struct amii_DisplayDesc *amiIDisplay;	/* the Amiga Intuition descriptor */
#endif

int clipping = 0;
int clipx=0;
int clipy=0;
int clipxmax=0;
int clipymax=0;
int scrollmsg = 1;
int alwaysinvent = 0;

#ifndef	SHAREDLIB

/* Interface definition, for use by windows.c and winprocs.h to provide
 * the intuition interface for the amiga...
 */
struct window_procs amii_procs =
{
    "amii",
    amii_init_nhwindows,
    amii_player_selection,
    amii_askname,
    amii_get_nh_event,
    amii_exit_nhwindows,
    amii_suspend_nhwindows,
    amii_resume_nhwindows,
    amii_create_nhwindow,
    amii_clear_nhwindow,
    amii_display_nhwindow,
    amii_destroy_nhwindow,
    amii_curs,
    amii_putstr,
    amii_display_file,
    amii_start_menu,
    amii_add_menu,
    amii_end_menu,
    amii_select_menu,
    amii_update_inventory,
    amii_mark_synch,
    amii_wait_synch,
#ifdef CLIPPING
    amii_cliparound,
#endif
    amii_print_glyph,
    amii_raw_print,
    amii_raw_print_bold,
    amii_nhgetch,
    amii_nh_poskey,
    amii_bell,
    amii_doprev_message,
    amii_yn_function,
    amii_getlin,
#ifdef COM_COMPL
    amii_get_ext_cmd,
#endif /* COM_COMPL */
    amii_number_pad,
    amii_delay_output,
#ifdef CHANGE_COLOR	/* only a Mac option currently */
    amii_change_color,
    amii_get_color_string,
#endif
    /* other defs that really should go away (they're tty specific) */
    amii_delay_output,
    amii_delay_output,
    amii_outrip,
};

/* The view window layout uses the same function names so we can use
 * a shared library to allow the executable to be smaller.
 */
struct window_procs amiv_procs =
{
    "amiv",
    amii_init_nhwindows,
    amii_player_selection,
    amii_askname,
    amii_get_nh_event,
    amii_exit_nhwindows,
    amii_suspend_nhwindows,
    amii_resume_nhwindows,
    amii_create_nhwindow,
    amii_clear_nhwindow,
    amii_display_nhwindow,
    amii_destroy_nhwindow,
    amii_curs,
    amii_putstr,
    amii_display_file,
    amii_start_menu,
    amii_add_menu,
    amii_end_menu,
    amii_select_menu,
    amii_update_inventory,
    amii_mark_synch,
    amii_wait_synch,
#ifdef CLIPPING
    amii_cliparound,
#endif
    amii_print_glyph,
    amii_raw_print,
    amii_raw_print_bold,
    amii_nhgetch,
    amii_nh_poskey,
    amii_bell,
    amii_doprev_message,
    amii_yn_function,
    amii_getlin,
#ifdef COM_COMPL
    amii_get_ext_cmd,
#endif /* COM_COMPL */
    amii_number_pad,
    amii_delay_output,
#ifdef CHANGE_COLOR	/* only a Mac option currently */
    amii_change_color,
    amii_get_color_string,
#endif
    /* other defs that really should go away (they're tty specific) */
    amii_delay_output,
    amii_delay_output,
    amii_outrip,
};
#endif

#ifndef	SHAREDLIB
unsigned short amii_initmap[ 1L << DEPTH ] =
{
    0x0000, /* color #0 */
    0x0FFF, /* color #1 */
    0x0830, /* color #2 */
    0x07ac, /* color #3 */
    0x0181, /* color #4 */
    0x0C06, /* color #5 */
    0x023E, /* color #6 */
    0x0c00  /* color #7 */
#ifdef	VIEWWINDOW
    0x0AAA,
    0x0fff,
    0x0444,
    0x0666,
    0x0888,
    0x0bbb,
    0x0ddd,
    0x0222,
#endif
};
#endif

struct Rectangle lastinvent, lastmsg;

static int FDECL( put_ext_cmd, ( char *, int, struct amii_WinDesc *, int ) );

#ifdef	SHAREDLIB
WinamiBASE *WinamiBase;

int
__UserLibInit( void )
{
	WinamiBase = (WinamiBASE *)getreg( REG_A6 );
    if ( (DOSBase = (struct DosLibrary *)
	    OpenLibrary("dos.library", 0)) == NULL)
    {
	Abort(AG_OpenLib | AO_DOSLib);
    }

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

    return( 0 );
}

void
__UserLibCleanup( void )
{
    amii_cleanup();
}

char _ProgramName[ 100 ] = "Nethack";
long __curdir;
long _WBenchMsg;
int _OSERR;
#endif

#ifndef TTY_GRAPHICS	/* this should be shared better */
char morc;  /* the character typed in response to a --more-- prompt */
#endif
char spaces[ 76 ] =
"                                                                           ";

#ifndef	SHAREDLIB
winid WIN_BASE = WIN_ERR;
winid amii_rawprwin = WIN_ERR;
#endif

#ifdef	VIEWWINDOW
winid WIN_VIEW = WIN_ERR;
winid WIN_VIEWBOX = WIN_ERR;
#endif

/* Changed later during window/screen opens... */
int txwidth = FONTWIDTH, txheight = FONTHEIGHT, txbaseline = FONTBASELINE;

/* If a 240 or more row screen is in front when we start, this will be
 * set to 1, and the windows will be given borders to allow them to be
 * arranged differently.  The Message window may eventually get a scroller...
 */
#ifndef	SHAREDLIB
int bigscreen = 0;
#endif
#ifdef	VIEWWINDOW
struct BitMap amii_vbm;
#endif

/* This gadget data is replicated for menu/text windows... */
struct PropInfo PropScroll = { AUTOKNOB|FREEVERT,
					0xffff,0xffff, 0xffff,0xffff, };
struct Image Image1 = { 0,0, 7,102, 0, NULL, 0x0000,0x0000, NULL };
struct Gadget MenuScroll = {
    NULL, -15,10, 15,-19, GRELRIGHT|GRELHEIGHT,
    RELVERIFY|FOLLOWMOUSE|RIGHTBORDER|GADGIMMEDIATE|RELVERIFY,
    PROPGADGET, (APTR)&Image1, NULL, NULL, NULL, (APTR)&PropScroll,
    1, NULL
};

/* This gadget is for the message window... */
struct PropInfo MsgPropScroll = { AUTOKNOB|FREEVERT,
					0xffff,0xffff, 0xffff,0xffff, };
struct Image MsgImage1 = { 0,0, 7,102, 0, NULL, 0x0000,0x0000, NULL };
struct Gadget MsgScroll = {
    NULL, -14,10, 13,-19, GRELRIGHT|GRELHEIGHT,
    RELVERIFY|FOLLOWMOUSE|RIGHTBORDER|GADGIMMEDIATE|RELVERIFY,
    PROPGADGET, (APTR)&MsgImage1, NULL, NULL, NULL, (APTR)&MsgPropScroll,
    1, NULL
};

int wincnt=0;   /* # of nh windows opened */

/* We advertise a public screen to allow some people to do other things
 * while they are playing...  like compiling...
 */

#ifdef  INTUI_NEW_LOOK
struct TagItem tags[] =
{
    { WA_PubScreenName, (ULONG)"NetHack", },
    { TAG_DONE, 0, },
};
#endif

/*
 * The default dimensions and status values for each window type.  The
 * data here is generally changed in create_nhwindow(), so beware that
 * what you see here may not be exactly what you get.
 */
struct win_setup new_wins[] =
{

    /* First entry not used, types are based at 1 */
    {{0}},

    /* NHW_MESSAGE */
    {{0,1,640,11,0xff,0xff,
    NEWSIZE|GADGETUP|GADGETDOWN|MOUSEMOVE|MOUSEBUTTONS|RAWKEY,
    BORDERLESS|ACTIVATE|SMART_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)"Messages",NULL,NULL,640,40,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,1,1,80,80},

    /* NHW_STATUS */
    {{0,181,640,24,0xff,0xff, RAWKEY|MENUPICK|DISKINSERTED,
    BORDERLESS|ACTIVATE|SMART_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)"Game Status",NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,2,2,78,78},

    /* NHW_MAP */
    {{0,0,WIDTH,WINDOWHEIGHT,0xff,0xff,
    RAWKEY|MENUPICK|MOUSEBUTTONS|ACTIVEWINDOW|MOUSEMOVE,
    BORDERLESS|ACTIVATE|SMART_REFRESH|BACKDROP
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)"Dungeon Map",NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,22,22,80,80},

    /* NHW_MENU */
    {{400,10,10,10,80,30,
    RAWKEY|MENUPICK|DISKINSERTED|MOUSEMOVE|MOUSEBUTTONS|
    GADGETUP|GADGETDOWN|CLOSEWINDOW|VANILLAKEY|NEWSIZE|INACTIVEWINDOW,
    WINDOWSIZING|WINDOWCLOSE|WINDOWDRAG|ACTIVATE|SMART_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    &MenuScroll,NULL,(UBYTE*)"Pick an Item",
    NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,1,1,22,78},

    /* NHW_TEXT */
    {{0,0,640,200,0xff,0xff,
    RAWKEY|MENUPICK|DISKINSERTED|MOUSEMOVE|
    GADGETUP|CLOSEWINDOW|VANILLAKEY|NEWSIZE,
    WINDOWSIZING|WINDOWCLOSE|WINDOWDRAG|ACTIVATE|SMART_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    &MenuScroll,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,1,1,22,78},

    /* NHW_BASE */
    {{0,0,WIDTH,WINDOWHEIGHT,0xff,0xff,
    RAWKEY|MENUPICK|MOUSEBUTTONS,
    BORDERLESS|ACTIVATE|SMART_REFRESH|BACKDROP
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,22,22,80,80},

#ifdef	VIEWWINDOW
    /* NHW_VIEW */
    {{0,0,WIDTH,WINDOWHEIGHT,0xff,0xff,
    RAWKEY|MENUPICK|MOUSEBUTTONS,
    BORDERLESS|ACTIVATE|SMART_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,VIEWCHARWIDTH,VIEWCHARHEIGHT,VIEWCHARWIDTH,VIEWCHARHEIGHT},

    /* NHW_VIEWBOX */
    {{0,0,WIDTH,WINDOWHEIGHT,0xff,0xff,
    RAWKEY|MENUPICK|MOUSEBUTTONS|REFRESHWINDOW,
    WINDOWSIZING|WINDOWDRAG|ACTIVATE|SIMPLE_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,VIEWCHARWIDTH,VIEWCHARHEIGHT,VIEWCHARWIDTH,VIEWCHARHEIGHT},
#endif
};

const char winpanicstr[] = "Bad winid %d in %s()";

/* The opened windows information */
struct amii_WinDesc *amii_wins[ MAXWIN + 1 ];

#ifdef  INTUI_NEW_LOOK
UWORD scrnpens[] =
{
#ifndef	VIEWWINDOW
    C_BLACK,		/* DETAILPEN        */
    C_BLUE, 		/* BLOCKPEN         */
    C_BROWN,		/* TEXTPEN          */
    C_WHITE,		/* SHINEPEN         */
    C_BLUE,		/* SHADOWPEN        */
    C_CYAN,		/* FILLPEN          */
    C_WHITE,		/* FILLTEXTPEN      */
    C_CYAN,		/* BACKGROUNDPEN    */
    C_RED,		/* HIGHLIGHTTEXTPEN */
#else
    C_BLACK,		/* DETAILPEN        */
    C_BLUE,		/* BLOCKPEN         */
    C_BROWN,		/* TEXTPEN          */
    C_WHITE,		/* SHINEPEN         */
    C_BLUE,		/* SHADOWPEN        */
    C_CYAN,		/* FILLPEN          */
    C_WHITE,		/* FILLTEXTPEN      */
    C_CYAN,		/* BACKGROUNDPEN    */
    C_RED,		/* HIGHLIGHTTEXTPEN */
#endif
};

struct TagItem scrntags[] =
{
    { SA_PubName, (ULONG)"NetHack" },
    { SA_Overscan, OSCAN_TEXT },
    { SA_Pens, (ULONG)scrnpens },
    { SA_DisplayID, 0 },
    { TAG_DONE, 0 },
};
#endif

struct NewScreen NewHackScreen =
{
    0, 0, WIDTH, SCREENHEIGHT, DEPTH,
    C_BROWN, C_BLUE,     /* DetailPen, BlockPen */
    HIRES,
    CUSTOMSCREEN
#ifdef  INTUI_NEW_LOOK
    |NS_EXTENDED
#endif
    ,
    &Hack80,  /* Font */
    NULL,     /*(UBYTE *)" NetHack X.Y.Z" */
    NULL,     /* Gadgets */
    NULL,     /* CustomBitmap */
#ifdef  INTUI_NEW_LOOK
    scrntags,
#endif
};

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 * Always called after init_nhwindows() and before display_gamewindows().
 */
void
amii_askname()
{
    *plname = 0;
    do {
	getlin( "Who are you?", plname );
    } while( strlen( plname ) == 0 );

    if( *plname == '\33' )
    {
	clearlocks();
	exit_nhwindows(NULL);
	terminate(0);
    }
}

#include "Amiga:char.c"

/* Get the player selection character */

void
amii_player_selection()
{
    register struct Window *cwin;
    register struct IntuiMessage *imsg;
    register int aredone = 0;
    register struct Gadget *gd;
    static int once=0;
    long class, code;

    amii_clear_nhwindow( WIN_BASE );
    if( *pl_character ){
	pl_character[ 0 ] = toupper( pl_character[ 0 ] );
	if( index( pl_classes, pl_character[ 0 ] ) )
	    return;
    }

    if( !once ){
	if( bigscreen ){
	    Type_NewWindowStructure1.TopEdge =
	      (HackScreen->Height/2) - (Type_NewWindowStructure1.Height/2);
	}
	for( gd = Type_NewWindowStructure1.FirstGadget; gd;
	  gd = gd->NextGadget )
	{
	    if( gd->GadgetID != 0 )
		SetBorder( gd );
	}
	once = 1;
    }

    Type_NewWindowStructure1.Screen = HackScreen;
    if( ( cwin = OpenShWindow( (void *)&Type_NewWindowStructure1 ) ) == NULL )
    {
	return;
    }
    WindowToFront( cwin );

    while( !aredone )
    {
	WaitPort( cwin->UserPort );
	while( ( imsg = (void *) GetMsg( cwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    gd = (struct Gadget *)imsg->IAddress;
	    ReplyMsg( (struct Message *)imsg );

	    switch( class )
	    {
	    case VANILLAKEY:
		if( index( pl_classes, toupper( code ) ) )
		{
		    pl_character[0] = toupper( code );
		    aredone = 1;
		}
		else if( code == ' ' || code == '\n' || code == '\r' )
		{
#ifdef  TOURIST
		    strcpy( pl_character, roles[ rnd( 11 ) ] );
#else
		    strcpy( pl_character, roles[ rnd( 10 ) ] );
#endif
		    aredone = 1;
		    amii_clear_nhwindow( WIN_BASE );
		    CloseShWindow( cwin );
		    RandomWindow( pl_character );
		    return;
		}
		else if( code == 'q' || code == 'Q' )
		{
		CloseShWindow( cwin );
		clearlocks();
		exit_nhwindows(NULL);
		terminate(0);
		}
		else
		    DisplayBeep( NULL );
		break;

	    case GADGETUP:
		switch( gd->GadgetID )
		{
		case 1: /* Random Character */
#ifdef  TOURIST
		    strcpy( pl_character, roles[ rnd( 11 ) ] );
#else
		    strcpy( pl_character, roles[ rnd( 10 ) ] );
#endif
		    amii_clear_nhwindow( WIN_BASE );
		    CloseShWindow( cwin );
		    RandomWindow( pl_character );
		    return;

		default:
		    pl_character[0] = gd->GadgetID;
		    break;
		}
		aredone = 1;
		break;

	    case CLOSEWINDOW:
		CloseShWindow( cwin );
		clearlocks();
		exit_nhwindows(NULL);
		terminate(0);
		break;
	    }
	}
    }
    amii_clear_nhwindow( WIN_BASE );
    CloseShWindow( cwin );
}

#include "Amiga:randwin.c"

void
RandomWindow( name )
    char *name;
{
    struct MsgPort *tport;
    struct timerequest *trq;
    static int once = 0;
    struct Gadget *gd;
    struct Window *w;
    struct IntuiMessage *imsg;
    int ticks = 0, aredone = 0, timerdone = 0;
    long mask, got;

    tport = CreatePort( 0, 0 );
    trq = (struct timerequest *)CreateExtIO( tport, sizeof( *trq ) );
    if( tport == NULL || trq == NULL )
    {
allocerr:
	if( tport ) DeletePort( tport );
	if( trq ) DeleteExtIO( (struct IORequest *)trq );
	Delay( 8 * 50 );
	return;
    }

    if( OpenDevice( TIMERNAME, UNIT_VBLANK, (struct IORequest *)trq, 0L ) != 0 )
	goto allocerr;

    trq->tr_node.io_Command = TR_ADDREQUEST;
    trq->tr_time.tv_secs = 8;
    trq->tr_time.tv_micro = 0;

    SendIO( (struct IORequest *)trq );

    /* Place the name in the center of the screen */
    Rnd_IText5.IText = name;
    Rnd_IText6.LeftEdge = Rnd_IText4.LeftEdge +
		(strlen(Rnd_IText4.IText)+1)*8;
    Rnd_NewWindowStructure1.Width = (
	    (strlen( Rnd_IText2.IText )+1) * 8 ) +
	    HackScreen->WBorLeft + HackScreen->WBorRight;
    Rnd_IText5.LeftEdge = (Rnd_NewWindowStructure1.Width -
	    (strlen(name)*8))/2;

    gd = Rnd_NewWindowStructure1.FirstGadget;
    gd->LeftEdge = (Rnd_NewWindowStructure1.Width - gd->Width)/2;
	/* Chose correct modifier */
    Rnd_IText6.IText = "a";
    switch( *name )
    {
    case 'a': case 'e': case 'i': case 'o':
    case 'u': case 'A': case 'E': case 'I':
    case 'O': case 'U':
	Rnd_IText6.IText = "an";
	break;
    }

    if( !once )
    {
	if( bigscreen )
	{
	    Rnd_NewWindowStructure1.TopEdge =
		(HackScreen->Height/2) - (Rnd_NewWindowStructure1.Height/2);
	}
	for( gd = Rnd_NewWindowStructure1.FirstGadget; gd; gd = gd->NextGadget )
	{
	    if( gd->GadgetID != 0 )
		SetBorder( gd );
	}
	Rnd_NewWindowStructure1.IDCMPFlags |= VANILLAKEY;

	once = 1;
    }

    Rnd_NewWindowStructure1.Screen = HackScreen;
    if( ( w = OpenShWindow( (void *)&Rnd_NewWindowStructure1 ) ) == NULL )
    {
	AbortIO( (struct IORequest *)trq );
	WaitIO( (struct IORequest *)trq );
	CloseDevice( (struct IORequest *)trq );
	DeleteExtIO( (struct IORequest *) trq );
	DeletePort( tport );
	Delay( 50 * 8 );
	return;
    }

    PrintIText( w->RPort, &Rnd_IntuiTextList1, 0, 0 );

    mask = (1L << tport->mp_SigBit)|(1L << w->UserPort->mp_SigBit);
    while( !aredone )
    {
	got = Wait( mask );
	if( got & (1L << tport->mp_SigBit ) )
	{
	    aredone = 1;
	    timerdone = 1;
	    GetMsg( tport );
        }
        while( w && ( imsg = (struct IntuiMessage *) GetMsg( w->UserPort ) ) )
        {
	    switch( imsg->Class )
	    {
		/* Must be up for a little while... */
	    case INACTIVEWINDOW:
		if( ticks >= 40 )
		    aredone = 1;
		break;

	    case INTUITICKS:
		++ticks;
		break;

	    case GADGETUP:
		aredone = 1;
		break;

	    case VANILLAKEY:
		if(imsg->Code=='\n' || imsg->Code==' ' || imsg->Code=='\r')
		    aredone = 1;
		break;
	    }
	    ReplyMsg( (struct Message *)imsg );
        }
    }

    if( !timerdone )
    {
	AbortIO( (struct IORequest *)trq );
	WaitIO( (struct IORequest *)trq );
    }

    CloseDevice( (struct IORequest *)trq );
    DeleteExtIO( (struct IORequest *) trq );
    DeletePort( tport );
    if(w) CloseShWindow( w );
}

/* this should probably not be needed (or be renamed)
void
flush_output(){} */

#ifdef COM_COMPL
/* Read in an extended command - doing command line completion for
 * when enough characters have been entered to make a unique command.
 */
void
amii_get_ext_cmd(bufp)
register char *bufp;
{
    register char *obufp = bufp;
    register int c;
    int com_index, oindex;
    struct amii_WinDesc *cw;
    struct Window *w;
    int colx;
    int did_comp=0;	/* did successful completion? */
    int bottom = 0;

    if( WIN_MESSAGE == WIN_ERR || ( cw = amii_wins[ WIN_MESSAGE ] ) == NULL )
	panic(winpanicstr, WIN_MESSAGE, "get_ext_cmd");
    amii_clear_nhwindow( NHW_MESSAGE );
    if( scrollmsg )
    {
	pline("#");
	amii_putstr( WIN_MESSAGE, -1, " " );
    }
    else
    {
	pline("# ");
    }
    colx = 3;
    w = cw->win;

    if( bigscreen )
    {
    	bottom = amii_msgborder( w );
    }

    while((c = WindowGetchar()) != EOF)
    {
	amii_curs( WIN_MESSAGE, colx, bottom );
	if(c == '?' )
	{
	    int win, i, sel;
	    char buf[ 100 ];

	    if(did_comp){
		while(bufp!=obufp){
		    bufp--;
		    amii_curs(WIN_MESSAGE, --colx, bottom);
		    Text(w->RPort,spaces,1);
		    amii_curs(WIN_MESSAGE,colx,bottom);
		    did_comp=0;
		}
	    }

	    win = amii_create_nhwindow( NHW_MENU );
	    amii_start_menu( win );

	    for( i = 0; extcmdlist[ i ].ef_txt != NULL; ++i )
	    {
		sprintf( buf, "%-10s - %s ",
			 extcmdlist[ i ].ef_txt,
			 extcmdlist[ i ].ef_desc );
		amii_add_menu( win, extcmdlist[i].ef_txt[0], 0, buf );
	    }

	    amii_end_menu( win, i, "\33", NULL );
	    sel = amii_select_menu( win );
	    amii_destroy_nhwindow( win );

	    if( sel == '\33' || !sel )
	    {
		*obufp = '\33';
		obufp[ 1 ] = 0;
		return;
	    }
	    else
	    {
		for( i = 0; extcmdlist[ i ].ef_txt != NULL; ++i )
		{
		    if( sel == extcmdlist[i].ef_txt[0] )
			break;
		}

		/* copy in the text */
		if( extcmdlist[ i ].ef_txt != NULL )
		{
		    amii_clear_nhwindow( WIN_MESSAGE );
		    strcpy( obufp = bufp, extcmdlist[ i ].ef_txt );
		    (void) put_ext_cmd( obufp, colx, cw, bottom );
		    return;
		}
		else
		    DisplayBeep( NULL );
	    }
	}
	else if(c == '\033')
	{
	    *obufp = c;
	    obufp[1] = 0;
	    return;
	}
	else if(c == '\b')
	{
	    if(did_comp){
		while(bufp!=obufp){
		    bufp--;
		    amii_curs(WIN_MESSAGE, --colx, bottom);
		    Text(w->RPort,spaces,1);
		    amii_curs(WIN_MESSAGE,colx,bottom);
		    did_comp=0;
		}
	    }else
	    if(bufp != obufp)
	    {
		bufp--;
		amii_curs( WIN_MESSAGE, --colx, bottom);
		Text( w->RPort, spaces, 1 );
		amii_curs( WIN_MESSAGE, colx, bottom);
	    }
	    else
		DisplayBeep( NULL );
	}
	else if( c == '\n' || c == '\r' )
	{
	    *bufp = 0;
	    return;
	}
	else if(' ' <= c && c < '\177')
	{
		/* avoid isprint() - some people don't have it
		   ' ' is not always a printing char */
	    *bufp = c;
	    bufp[1] = 0;
	    oindex = 0;
	    com_index = -1;

	    while(extcmdlist[oindex].ef_txt != NULL)
	    {
		if(!strnicmp(obufp, extcmdlist[oindex].ef_txt, strlen(obufp)))
		{
		    if(com_index == -1) /* No matches yet*/
			com_index = oindex;
		    else /* More than 1 match */
			com_index = -2;
		}
		oindex++;
	    }

	    if(com_index >= 0 && *obufp )
	    {
		Strcpy(obufp, extcmdlist[com_index].ef_txt);
		/* finish printing our string */
		colx = put_ext_cmd( obufp, colx, cw, bottom );
		bufp = obufp; /* reset it */
		if(strlen(obufp) < BUFSZ-1 && strlen(obufp) < COLNO)
		    bufp += strlen(obufp);
		did_comp=1;
	    }
	    else
	    {
		colx = put_ext_cmd( obufp, colx, cw, bottom );
		if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
		    bufp++;
	    }
	}
	else if(c == ('X'-64) || c == '\177')
	{
	    colx = 0;
	    amii_clear_nhwindow( WIN_MESSAGE );
	    pline( "# " );
	    bufp = obufp;
	} else
	    DisplayBeep( NULL );
    }
    *bufp = 0;
    return;
}

static int
put_ext_cmd( obufp, colx, cw, bottom )
    char *obufp;
    int colx, bottom;
    struct amii_WinDesc *cw;
{
    struct Window *w = cw->win;
    char *t;

    t = malloc( strlen( obufp ) + 7 );
    if( t != NULL )
    {
	if( scrollmsg )
	{
	    sprintf( t, "xxx%s", obufp );
	    t[0] = 1;
	    t[1] = -1;
	    t[2] = ' ';
	    amii_curs( WIN_MESSAGE, 0, bottom);
	    SetAPen( w->RPort, C_WHITE );
	    Text(w->RPort, "># ", 3 );
	    SetAPen( w->RPort, C_RED );
	    Text(w->RPort, t+3, strlen( t ) - 3 );
	}
	else
	{
	    sprintf( t, "# %s", obufp );
	    amii_curs( WIN_MESSAGE, 0, bottom);
	    SetAPen( w->RPort, C_WHITE );
	    Text(w->RPort, t, strlen( t ) );
	}
	if( scrollmsg )
	    SetAPen( w->RPort, C_WHITE );
	if( cw->data[ cw->maxrow - 1 ] )
	    free( cw->data[ cw->maxrow - 1 ] );
	cw->data[ cw->maxrow - 1 ] = t;
    }
    else
    {
	amii_curs( WIN_MESSAGE, 0, bottom);
	SetAPen( w->RPort, C_WHITE );
	Text(w->RPort, "# ", 2 );
	SetAPen( w->RPort, C_RED );
	Text(w->RPort, obufp, strlen( obufp ) );
	SetAPen( w->RPort, C_WHITE );
    }
    amii_curs( WIN_MESSAGE, colx = strlen( obufp ) + 3 + ( scrollmsg != 0 ), bottom);
    return( colx );
}
#endif /* COM_COMPL */

#ifdef WINDOW_YN
SHORT Ask_BorderVectors1[] = { 0,0, 29,0, 29,11, 0,11, 0,0 };
struct Border Ask_Border1 = { -1,-1, 3,0,JAM1, 5, Ask_BorderVectors1, NULL };
struct IntuiText Ask_IText1 = { 3,0,JAM2, 2,1, NULL, "(?)", NULL };

struct Gadget Ask_Gadget1 = {
    NULL, 9,4, 28,10, NULL, RELVERIFY, BOOLGADGET, (APTR)&Ask_Border1,
    NULL, &Ask_IText1, NULL, NULL, NULL, NULL
};

#define Ask_GadgetList1 Ask_Gadget1

struct IntuiText Ask_IText2 = { 1,0,JAM2, 44,5, NULL, NULL, NULL };

#define Ask_IntuiTextList1 Ask_IText2

struct NewWindow Ask_Window = {
    75,85, 524,18, 0,1, GADGETUP+VANILLAKEY, ACTIVATE+NOCAREREFRESH,
    &Ask_Gadget1, NULL, NULL, NULL, NULL, 5,5, -1,-1, CUSTOMSCREEN
};
#endif

/* Ask a question and get a response */

#ifdef OLDCODE

char amii_yn_function( prompt, resp, def )
    const char *prompt,*resp;
    char def;
{
    char ch;
    char buf[ 80 ];

    if( def && def!='q')
    {
	sprintf( buf, "%s [%c] ", prompt, def );
	amii_addtopl( buf );
    } else {
	amii_addtopl( prompt );
    }

    cursor_on( WIN_MESSAGE );
    do {
	ch = WindowGetchar();
	if( ch == '\33' )
	    break;
	else if( def && ( ch == '\n' || ch == '\r' ) )
	{
	    ch = def;
	    break;
	}
	else if( isdigit(ch) && index(resp, '#')){
		
	}
    } while( resp && *resp && index( resp, ch ) == 0 );

    cursor_off( WIN_MESSAGE );
    if( ch == '\33' )
    {
	if(index(resp, 'q'))
		ch = 'q';
	else if(index(resp, 'n'))
		ch = 'n';
	else ch = def;
    }
    /* Try this to make topl behave more appropriately? */
    clear_nhwindow( WIN_MESSAGE );
    return( ch );
}
#else
char amii_yn_function(query,resp, def)
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
	register char q;
	char rtmp[40];
	boolean digit_ok, allow_num;
	char prompt[QBUFSZ];
	register struct amii_WinDesc *cw;

	if( cw = amii_wins[ WIN_MESSAGE ] )
	    cw->disprows = 0;
	if(resp) {
	    allow_num = (index(resp, '#') != 0);
	    if(def)
		Sprintf(prompt, "%s [%s] (%c) ", query, resp, def);
	    else
		Sprintf(prompt, "%s [%s] ", query, resp);
	    amii_addtopl(prompt);
	} else {
	    amii_addtopl(query);
	    cursor_on(WIN_MESSAGE);
	    q = WindowGetchar();
	    cursor_off(WIN_MESSAGE);
#if 1
	    TOPL_NOSPACE;
	    *rtmp = q;
	    rtmp[ 1 ] = 0;
	    amii_putstr(WIN_MESSAGE,-1,rtmp);
	    TOPL_SPACE;
#endif
	    goto clean_up;
	}

	do {	/* loop until we get valid input */
	    cursor_on(WIN_MESSAGE);
	    q = lowc(WindowGetchar());
	    cursor_off(WIN_MESSAGE);
#if 0
/* fix for PL2 */
	    if (q == '\020') { /* ctrl-P */
		if(!doprev) (void) tty_doprev_message(); /* need two initially */
		(void) tty_doprev_message();
		q = (char)0;
		doprev = 1;
		continue;
	    } else if(doprev) {
		tty_clear_nhwindow(WIN_MESSAGE);
		cw->maxcol = cw->maxrow;
		doprev = 0;
		amii_addtopl(prompt);
		continue;
	    }
#endif
	    digit_ok = allow_num && isdigit(q);
	    if (q == '\033') {
		if (index(resp, 'q'))
		    q = 'q';
		else if (index(resp, 'n'))
		    q = 'n';
		else
		    q = def;
		break;
	    } else if (index(quitchars, q)) {
		q = def;
		break;
	    }
	    if (!index(resp, q) && !digit_ok) {
		amii_bell();
		q = (char)0;
	    } else if (q == '#' || digit_ok) {
		char z, digit_string[2];
		int n_len = 0;
		long value = 0;
		TOPL_NOSPACE;
		amii_addtopl("#"),  n_len++;
		TOPL_SPACE;
		digit_string[1] = '\0';
		if (q != '#') {
		    digit_string[0] = q;
		    TOPL_NOSPACE;
		    amii_addtopl(digit_string),  n_len++;
		    TOPL_SPACE;
		    value = q - '0';
		    q = '#';
		}
		do {	/* loop until we get a non-digit */
		    cursor_on(WIN_MESSAGE);
		    z = lowc(WindowGetchar());
		    cursor_off(WIN_MESSAGE);
		    if (isdigit(z)) {
			value = (10 * value) + (z - '0');
			if (value < 0) break;	/* overflow: try again */
			digit_string[0] = z;
			TOPL_NOSPACE;
			amii_addtopl(digit_string),  n_len++;
			TOPL_SPACE;
		    } else if (z == 'y' || index(quitchars, z)) {
			if (z == '\033')  value = -1;	/* abort */
			z = '\n';	/* break */
		    } else if ( z == '\b') {
			if (n_len <= 1) { value = -1;  break; }
			else { value /= 10;  removetopl(1),  n_len--; }
		    } else {
			value = -1;	/* abort */
			amii_bell();
			break;
		    }
		} while (z != '\n');
		if (value > 0) yn_number = value;
		else if (value == 0) q = 'n';		/* 0 => "no" */
		else {	/* remove number from top line, then try again */
			removetopl(n_len),  n_len = 0;
			q = '\0';
		}
	    }
	} while(!q);

	if (q != '#') {
	    Sprintf(rtmp, "%c", q);
#if 0
	    amii_addtopl(rtmp);
#else
	    TOPL_NOSPACE;
	    amii_putstr(WIN_MESSAGE,-1,rtmp);
	    TOPL_SPACE;
#endif
	}
    clean_up:
	cursor_off(WIN_MESSAGE);
	clear_nhwindow(WIN_MESSAGE);
	return q;
}

#endif


void
amii_display_file(fn, complain)
const char *fn;
boolean complain;
{
    register struct amii_WinDesc *cw;
    register int win;
    register FILE *fp;
    register char *t;
    register char buf[ 200 ];

    if( fn == NULL )
	panic("NULL file name in display_file()");

    if( ( fp = fopenp( fn, "r" ) ) == NULL )
    {
	if (complain) {
	    sprintf( buf, "Can't display %s: %s", fn,
#ifdef  __SASC_60
			__sys_errlist[ errno ]
#else
			sys_errlist[ errno ]
#endif
			);
	    amii_addtopl( buf );
	}
	return;
    }
    win = amii_create_nhwindow( NHW_TEXT );

    /* Set window title to file name */
    if( cw = amii_wins[ win ] )
	cw->morestr = fn;

    while( fgets( buf, sizeof( buf ), fp ) != NULL )
    {
	if( t = index( buf, '\n' ) )
	    *t = 0;
	amii_putstr( win, 0, buf );
    }
    fclose( fp );

    /* If there were lines in the file, display those lines */

    if( amii_wins[ win ]->cury > 0 )
	amii_display_nhwindow( win, TRUE );

    amii_wins[win]->morestr = NULL;		/* don't free title string */
    amii_destroy_nhwindow( win );
}

/* Put a 3-D motif border around the gadget.  String gadgets or those
 * which do not have highlighting are rendered down.  Boolean gadgets
 * are rendered in the up position by default.
 */

void
SetBorder( gd )
    register struct Gadget *gd;
{
    register struct Border *bp;
    register short *sp;
    register int i, inc = -1, dec = -1;
    int borders = 6;

    /* Allocate two border structures one for up image and one for down
     * image, plus vector arrays for the border lines.
     */

    if( gd->GadgetType == STRGADGET )
	borders = 12;

    if( ( bp = (struct Border *)alloc( ( ( sizeof( struct Border ) * 2 ) +
			( sizeof( short ) * borders ) ) * 2 ) ) == NULL )
    {
	return;
    }

    /* For a string gadget, we expand the border beyond the area where
     * the text will be entered.
     */

    /* Remove any special rendering flags to avoid confusing intuition
     */

    gd->Flags &= ~(GADGHIGHBITS|GADGIMAGE);

    sp = (short *)(bp + 4);
    if( gd->GadgetType == STRGADGET || ( gd->GadgetType == BOOLGADGET &&
			    ( gd->Flags & GADGHIGHBITS ) == GADGHNONE ) )
    {
	sp[0] = -1;
	sp[1] = gd->Height - 1;
	sp[2] = -1;
	sp[3] = -1;
	sp[4] = gd->Width - 1;
	sp[5] = -1;

	sp[6] = gd->Width + 1;
	sp[7] = -2;
	sp[8] = gd->Width + 1;
	sp[9] = gd->Height + 1;
	sp[10] = -2;
	sp[11] = gd->Height + 1;

	sp[12] = -2;
	sp[13] = gd->Height;
	sp[14] = -2;
	sp[15] = -2;
	sp[16] = gd->Width;
	sp[17] = -2;
	sp[18] = gd->Width;
	sp[19] = gd->Height;
	sp[20] = -2;
	sp[21] = gd->Height;

	for( i = 0; i < 3; ++i )
	{
	    bp[ i ].LeftEdge = bp[ i ].TopEdge = -1;
	    bp[ i ].FrontPen = ( i == 0 || i == 1 ) ? C_BLUE : C_WHITE;

	    /* Have to use JAM2 so that the old colors disappear. */
	    bp[ i ].BackPen = C_BLACK;
	    bp[ i ].DrawMode = JAM2;
	    bp[ i ].Count = ( i == 0 || i == 1 ) ? 3 : 5;
	    bp[ i ].XY = &sp[ i*6 ];
	    bp[ i ].NextBorder = ( i == 2 ) ? NULL : &bp[ i + 1 ];
	}

	/* bp[0] and bp[1] two pieces for the up image */
	gd->GadgetRender = (APTR) bp;

	/* No image change for select */
	gd->SelectRender = (APTR) bp;

	gd->LeftEdge++;
	gd->TopEdge++;
	gd->Flags |= GADGHCOMP;
    }
    else
    {
	/* Create the border vector values for up and left side, and
	 * also the lower and right side.
	 */

	sp[0] = dec;
	sp[1] = gd->Height + inc;
	sp[2] = dec;
	sp[3] = dec;
	sp[4] = gd->Width + inc;
	sp[5] = dec;

	sp[6] = gd->Width + inc;
	sp[7] = dec;
	sp[8] = gd->Width + inc;
	sp[9] = gd->Height + inc;
	sp[10] = dec;
	sp[11] = gd->Height + inc;

	/* We are creating 4 sets of borders, the two sides of the
	 * rectangle share the border vectors with the opposite image,
	 * but specify different colors.
	 */

	for( i = 0; i < 4; ++i )
	{
	    bp[ i ].TopEdge = bp[ i ].LeftEdge = 0;

	    /* A GADGHNONE is always down */

	    if( gd->GadgetType == BOOLGADGET &&
			    ( gd->Flags & GADGHIGHBITS ) != GADGHNONE )
	    {
		bp[ i ].FrontPen =
			    ( i == 1 || i == 2 ) ? C_BLUE : C_WHITE;
	    }
	    else
	    {
		bp[ i ].FrontPen =
			    ( i == 1 || i == 3 ) ? C_WHITE : C_BLUE;
	    }

	    /* Have to use JAM2 so that the old colors disappear. */
	    bp[ i ].BackPen = C_BLACK;
	    bp[ i ].DrawMode = JAM2;
	    bp[ i ].Count = 3;
	    bp[ i ].XY = &sp[ 6 * ((i &1) != 0) ];
	    bp[ i ].NextBorder =
			    ( i == 1 || i == 3 ) ? NULL : &bp[ i + 1 ];
	}

	/* bp[0] and bp[1] two pieces for the up image */
	gd->GadgetRender = (APTR) bp;

	/* bp[2] and bp[3] two pieces for the down image */
	gd->SelectRender = (APTR) (bp + 2);
	gd->Flags |= GADGHIMAGE;
    }
}

#ifndef	SHAREDLIB
#if 0
void *
malloc( register unsigned size )
{
    register long *p;

    size += 4;
    p = AllocMem( size, MEMF_PUBLIC );
    if( p ) *p++ = size;
    else panic( "No memory left" );
    return( p );
}

void
free( void *q )
{
    register long *p = q;

    if( !q )
	panic( "free of NULL pointer" );
    FreeMem( p-1, p[-1] );
}
#endif
#endif
