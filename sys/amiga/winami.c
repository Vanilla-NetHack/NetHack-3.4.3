/*    SCCS Id: @(#)winami.c    3.1    93/01/07 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "wintype.h"
#include "winami.h"
#include "func_tab.h"

#ifdef AMIGA_INTUITION

/* These should probably not even be options, but, I will leave them
 * for now... GGW
 */

/*#define   TOPL_GETLINE	/* Don't use a window for getlin() */
/*#define   WINDOW_YN		/* Use a window for y/n questions */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/devices.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dosextens.h>
#include <ctype.h>
#undef  strcmpi
#include <string.h>
#include <errno.h>

#ifdef  IDCMP_CLOSEWINDOW
# define	INTUI_NEW_LOOK
#endif

#ifdef AZTEC_C
#include <functions.h>
#else
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/console.h>
#include <proto/diskfont.h>

/* kludge - see amirip for why */
#undef red
#undef green
#undef blue
#include <proto/graphics.h>
#ifndef __SASC_60
#undef index
# define index strchr
#endif

#include <proto/intuition.h>
#endif

static void RandomWindow( char * );

/* cw->data[x] contains 2 characters worth of special information.  These
 * characters are stored at the offsets as described here.
 */
#define VATTR	  0	/* Video attribute is in this slot */
#define SEL_ITEM  1	/* If this is a select item, slot is 1 else 0 */
#define SOFF	  2	/* The string starts here.  */

/* Nethack defines NULL as ((char *)0) which is very inconvienent... */
#undef NULL
#define NULL 0L

/*
 * Versions we need of various libraries.  We can't use LIBRARY_VERSION
 * as defined in <exec/types.h> because some of the libraries we need
 * don't have that version number in the 1.2 ROM.
 */

#define INTUITION_VERSION	33L
#define GRAPHICS_VERSION	33L
#define DISKFONT_VERSION	34L
#define ICON_VERSION		34L

/* These values are just sorta suggestions in use, but are minimum requirements
 * in reality...
 */
#define WINDOWHEIGHT	192
#define SCREENHEIGHT	200
#define WIDTH		640

/* This character is a solid block (cursor) in Hack.font */
#define CURSOR_CHAR	0x90

/* All we currently support is hack.font at 8x8... */
#define FONTWIDTH	8
#define FONTHEIGHT	8
#define FONTBASELINE	8

/* If Compiling with the "New Look", redefine these now */
#ifdef  INTUI_NEW_LOOK
#define NewWindow ExtNewWindow
#define NewScreen ExtNewScreen
#endif

/* Get the prototypes for our files */
#include "Amiga:winami.p"
#include "Amiga:amiwind.p"
#include "Amiga:amidos.p"

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
    /* other defs that really should go away (they're tty specific) */
    amii_delay_output,
    amii_delay_output,
};

extern struct GfxBase *GfxBase;
extern struct Library *DiskfontBase;
extern struct IntuitionBase *IntuitionBase;

/* All kinds of shared stuff */
extern struct TextAttr Hack80;
extern struct Screen *HackScreen;
extern struct Window *pr_WindowPtr;
extern struct Menu HackMenu[];
extern unsigned char KbdBuffered;
extern struct TextFont *HackFont;
extern struct IOStdReq ConsoleIO;
extern struct Library *ConsoleDevice;
extern struct MsgPort *HackPort;
extern char Initialized;
extern char toplines[BUFSZ];

char morc;  /* the character typed in response to a --more-- prompt */
char spaces[] =
"                                                                           ";
extern NEARDATA winid WIN_MESSAGE;
extern NEARDATA winid WIN_MAP;
extern NEARDATA winid WIN_STATUS;
extern NEARDATA winid WIN_INVEN;

winid WIN_BASE = WIN_ERR;

/* Changed later during window/screen opens... */
int txwidth=FONTWIDTH, txheight=FONTHEIGHT, txbaseline = FONTBASELINE;

/* If a 240 or more row screen is in front when we start, this will be
 * set to 1, and the windows will be given borders to allow them to be
 * arranged differently.  The Message window may eventually get a scroller...
 */
int bigscreen = 0;

static void outmore(struct WinDesc *);
static void outsubstr(struct WinDesc *,char *,int);
static void dismiss_nhwindow(winid);
static void removetopl(int);

/* This gadget data is replicated for menu/text windows... */
static struct PropInfo PropScroll = { AUTOKNOB+FREEVERT,
					0xffff,0xffff, 0xffff,0xffff, };
static struct Image Image1 = { 0,0, 7,102, 0, NULL, 0x0000,0x0000, NULL };
static struct Gadget MenuScroll = {
    NULL, -15,10, 15,-19, GRELRIGHT|GRELHEIGHT,
    RELVERIFY|FOLLOWMOUSE|RIGHTBORDER,
    PROPGADGET, (APTR)&Image1, NULL, NULL, NULL, (APTR)&PropScroll,
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
struct win_setup {
    struct NewWindow newwin;
    xchar offx,offy,maxrow,rows,maxcol,cols;	/* CHECK TYPES */
} new_wins[] = {

    /* First entry not used, types are based at 1 */
    {{0}},

    /* NHW_MESSAGE */
    {{0,1,640,11,0xff,0xff,
    RAWKEY,
    BORDERLESS|ACTIVATE|SMART_REFRESH
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    NULL,NULL,(UBYTE*)"Messages",NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,1,1,80,80},

    /* NHW_STATUS */
    {{0,181,640,24,0xff,0xff,RAWKEY|MENUPICK|DISKINSERTED,
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
    RAWKEY|MENUPICK|MOUSEBUTTONS|ACTIVEWINDOW
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    BORDERLESS|ACTIVATE|SMART_REFRESH|BACKDROP,
    NULL,NULL,(UBYTE*)"Dungeon Map",NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,22,22,80,80},

    /* NHW_MENU */
    {{400,10,10,10,80,30,
    RAWKEY|MENUPICK|DISKINSERTED|MOUSEMOVE|MOUSEBUTTONS|
    GADGETUP|GADGETDOWN|CLOSEWINDOW|VANILLAKEY|NEWSIZE
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    WINDOWSIZING|WINDOWCLOSE|WINDOWDRAG|ACTIVATE|SMART_REFRESH,
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
    GADGETUP|CLOSEWINDOW|VANILLAKEY|NEWSIZE
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    WINDOWSIZING|WINDOWCLOSE|WINDOWDRAG|ACTIVATE|SMART_REFRESH,
    &MenuScroll,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,1,1,22,78},

    /* NHW_BASE */
    {{0,0,WIDTH,WINDOWHEIGHT,0xff,0xff,
    RAWKEY|MENUPICK|MOUSEBUTTONS
#ifdef  INTUI_NEW_LOOK
    |WFLG_NW_EXTENDED
#endif
    ,
    BORDERLESS|ACTIVATE|SMART_REFRESH|BACKDROP,
    NULL,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,0xffff,0xffff,CUSTOMSCREEN,
#ifdef  INTUI_NEW_LOOK
    tags,
#endif
    },
    0,0,22,22,80,80},
};

/* The physical screen information */
struct DisplayDesc *amiIDisplay;

/* The opened windows information */
struct WinDesc *wins[MAXWIN + 1];

/* The last Window event is stored here for reference. */
extern WEVENT lastevent;

static const char winpanicstr[]="Bad winid %d in %s()";
#define         SIZEOF_DISKNAME 8

/* The current color map */
unsigned short amii_initmap[] = {
#define C_BLACK		0
#define C_WHITE		1
#define C_BROWN		2
#define C_CYAN		3
#define C_GREEN		4
#define C_MAGENTA	5
#define C_BLUE		6
#define C_RED		7
    0x0AAA, /* color #0 */
    0x0FFF, /* color #1 */
    0x0620, /* color #2 */
    0x0B08, /* color #3 */
    0x0181, /* color #4 */
    0x0C06, /* color #5 */
    0x023E, /* color #6 */
    0x0D00  /* color #7 */
};

#ifdef  INTUI_NEW_LOOK
UWORD scrnpens[] = {
    C_BROWN,
    C_BLACK,
    C_WHITE,
    C_WHITE,
    C_BROWN,
    C_CYAN,
    C_BROWN,
    C_BLACK,
    C_RED,
};
struct TagItem scrntags[] =
{
    { SA_PubName, (ULONG)"NetHack" },
    { SA_Pens, (ULONG)scrnpens },
    { TAG_DONE, 0 },
};
#endif

struct NewScreen NewHackScreen =
{
    0, 0, WIDTH, SCREENHEIGHT, DEPTH,
    C_BLACK, C_WHITE,     /* DetailPen, BlockPen */
    HIRES,
    CUSTOMSCREEN
#ifdef  INTUI_NEW_LOOK
    |NS_EXTENDED
#endif
    ,
    &Hack80,  /* Font */
    (UBYTE *)" NetHack 3.1.1",
    NULL,     /* Gadgets */
    NULL,     /* CustomBitmap */
#ifdef  INTUI_NEW_LOOK
    scrntags,
#endif
};

/* topl kludges */
#define TOPL_NOSPACE	topl_addspace=0
#define TOPL_SPACE	topl_addspace=1
int topl_addspace=1;

/* Make sure the user sees a text string when no windowing is available */

void
amii_raw_print(s)
    register const char *s;
{
    if( !s )
	return;
    if(amiIDisplay)amiIDisplay->rawprint++;

    if( Initialized == 0 && WIN_BASE == WIN_ERR )
	    init_nhwindows();

    if( WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
	amii_putstr( WIN_MAP, 0, s );
    else if( WIN_BASE != WIN_ERR && wins[ WIN_BASE ] )
	amii_putstr( WIN_BASE, 0, s );
    else {
	printf("%s\n", s);
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

    if(amiIDisplay)amiIDisplay->rawprint++;

    if( Initialized == 0 && WIN_BASE == WIN_ERR )
	    init_nhwindows();

    if( WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
	amii_putstr( WIN_MAP, 1, s );
    else if( WIN_BASE != WIN_ERR && wins[ WIN_BASE ] )
	amii_putstr( WIN_BASE, 1, s );
    else {
	printf("\33[1m%s\33[0m\n",s);
	fflush(stdout);
    }
}

/* Start building the text for a menu */
void
amii_start_menu(window)
    register winid window;
{
    register struct WinDesc *cw;

    if(window == WIN_ERR || (cw = wins[window]) == NULL || cw->type != NHW_MENU)
	panic(winpanicstr,window, "start_menu");

    amii_clear_nhwindow(window);
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
    register struct WinDesc *cw;
    char tmpbuf[2];

    if(str == NULL)return;

    if(window == WIN_ERR || (cw = wins[window]) == NULL || cw->type != NHW_MENU)
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
    register struct WinDesc *cw;

    if(window == WIN_ERR || (cw=wins[window]) == NULL || cw->type != NHW_MENU
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
    register struct WinDesc *cw;

    if( window == WIN_ERR || ( cw=wins[window] ) == NULL ||
	  cw->type != NHW_MENU )
	panic(winpanicstr,window, "select_menu");

    morc = 0;                       /* very ugly global variable */
    amii_display_nhwindow(window,TRUE); /* this waits for input */
    dismiss_nhwindow(window);       /* Now tear it down */
    return morc;
}

/* Rebuild/update the inventory if the window is up.  This is currently
 * a noop for us because we always take any menu window off of the
 * screen by deleting it when the user makes a selection, or cancels
 * the menu.
 */
void
amii_update_inventory()
{
    register struct WinDesc *cw;

    if( WIN_INVEN != WIN_ERR && ( cw = wins[ WIN_INVEN ] ) &&
      cw->type == NHW_MENU && cw->win )
    {
	display_inventory( NULL, FALSE );
	WindowToFront( cw->win );
	ActivateWindow( cw->win );
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
	flush_glyph_buffer( wins[ WIN_MAP ]->win );
    }
}

#ifdef CLIPPING
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
#endif

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
    extern const char *roles[];
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

static void
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
		(strlen(Rnd_IText4.IText)+1)*txwidth;
    Rnd_NewWindowStructure1.Width = (
	    (strlen( Rnd_IText2.IText )+1) * txwidth ) +
	    HackScreen->WBorLeft + HackScreen->WBorRight;
    Rnd_IText5.LeftEdge = (Rnd_NewWindowStructure1.Width -
	    (strlen(name)*txwidth))/2;

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

void
amii_destroy_nhwindow(win)      /* just hide */
    register winid win;
{
    register struct WinDesc *cw;
    register int i;

    if( win == WIN_ERR || ( cw = wins[win] ) == NULL )
    {
	panic(winpanicstr,win,"destroy_nhwindow");
    }

    /* Tear down the Intuition stuff */
    dismiss_nhwindow(win);

    if( cw->data && cw->type == NHW_MESSAGE ||
			    cw->type == NHW_MENU || cw->type == NHW_TEXT )
    {
	for( i = 0; i < cw->maxrow; ++i )
	{
	    if( cw->data[ i ] )
		free( cw->data[ i ] );
	}
	free( cw->data );
	cw->data = NULL;
    }

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
    wins[win] = NULL;
}

amii_create_nhwindow(type)
    register int type;
{
    register struct Window *w = NULL;
    register struct NewWindow *nw = NULL;
    register struct WinDesc *wd = NULL;
    struct Window *mapwin = NULL, *stwin = NULL, *msgwin = NULL;
    register int newid;

    if( WIN_STATUS != WIN_ERR && wins[ WIN_STATUS ] )
	stwin = wins[ WIN_STATUS ]->win;

    if( WIN_MESSAGE != WIN_ERR && wins[ WIN_MESSAGE ] )
	msgwin = wins[ WIN_MESSAGE ]->win;

    if( WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
	mapwin = wins[ WIN_MAP ]->win;

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

    if( type == NHW_MAP || type == NHW_BASE )
    {
	nw->DetailPen = C_WHITE;
	nw->BlockPen = C_MAGENTA;
	nw->TopEdge = 1;
	nw->LeftEdge = 0;
	nw->Height = amiIDisplay->ypix - nw->TopEdge;
	if( bigscreen && type == NHW_MAP )
	{
	    int h;
	    if( msgwin && stwin )
	    {
		h = (stwin->TopEdge - nw->TopEdge - 1) -
				(msgwin->TopEdge + msgwin->Height + 1);
		h = h - ( (22 * stwin->RPort->TxHeight) +
				    stwin->BorderTop + stwin->BorderBottom );
		/* h is now the available space excluding the map window so
		 * divide by 2 and use it to space out the map window.
		 */
		if( h > 0 )
		    h /= 2;
		else
		    h = 0;
		nw->TopEdge = msgwin->TopEdge + msgwin->Height + 1 + h;
		nw->Height = stwin->TopEdge + 1 - nw->TopEdge - h;
	    }
	    else
	    {
		h = amiIDisplay->ypix - (22 * FONTHEIGHT) - 12 - 10;
		if( h > 0 )
		{
		    nw->TopEdge = h / 2;
		    nw->Height = (22 * FONTHEIGHT) + 12 + 10;
		}
		else
		{
		    nw->Height -= 85;
		    nw->TopEdge += 35;
		}
	    }
	}
    }
    else if( type == NHW_STATUS )
    {
	nw->DetailPen = C_CYAN;
	nw->BlockPen = C_CYAN;
	if( WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
	    w = wins[ WIN_MAP ]->win;
	else if( WIN_BASE != WIN_ERR && wins[ WIN_BASE ] )
	    w = wins[ WIN_BASE ]->win;
	else
	    panic( "No window to base STATUS location from" );

	/* Status window is relative to bottom of WIN_BASE/WIN_MAP */

	if( mapwin && bigscreen )
	{
	    nw->TopEdge = mapwin->TopEdge + mapwin->Height;
	    nw->Height = amiIDisplay->ypix - nw->TopEdge - 1;
	}
	else
	{
	    /* Expand the height of window by borders */
	    if( bigscreen )
		nw->Height = (txheight * 2) + 17;

	    nw->TopEdge = amiIDisplay->ypix - nw->Height;
	    nw->LeftEdge = w->LeftEdge;
	    if( nw->LeftEdge + nw->Width >= amiIDisplay->xpix )
		nw->LeftEdge = 0;
	    if( nw->Width >= amiIDisplay->xpix - nw->LeftEdge )
		nw->Width = amiIDisplay->xpix - nw->LeftEdge;
	}
    }
    else if( type == NHW_MESSAGE )
    {
	nw->DetailPen = C_RED;
	nw->BlockPen = C_RED;
	if( WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
	    w = wins[ WIN_MAP ]->win;
	else if( WIN_BASE != WIN_ERR && wins[ WIN_BASE ] )
	    w = wins[ WIN_BASE ]->win;
	else
	    panic( "No window to base STATUS location from" );

	nw->TopEdge = 1;
	if( bigscreen )
	{
	    if( mapwin )
	    {
		nw->Height = mapwin->TopEdge - 2;
	    }
	    else
	    {
		nw->Height += 15;
	    }
	}
    }
    else
    {
	nw->DetailPen = C_WHITE;
	nw->BlockPen = C_MAGENTA;
    }

    /* When Interlaced, there is "Room" for all this stuff */
/* WRONG - still not enough space on MAP for right border */
    if( bigscreen && type != NHW_BASE )
    {
	nw->Flags &= ~( BORDERLESS | BACKDROP );
#if 1
	nw->Flags |= ( WINDOWDRAG | WINDOWDEPTH );
#else
	nw->Flags |= ( WINDOWDRAG | WINDOWDEPTH | SIZEBRIGHT );
	if( type == NHW_MAP )
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

	sprintf( buf, "nw is l: %d, t: %d, w: %d, h: %d",
		nw->LeftEdge, nw->TopEdge,
		nw->Width, nw->Height );
	raw_print( buf );
	panic("bad openwin %d",type);
    }

    /* Check for an empty slot */

    for(newid = 0; newid<MAXWIN + 1; newid++)
    {
	if(wins[newid] == 0)
	    break;
    }

    if(newid==MAXWIN+1)
	panic("time to write re-alloc code\n");

    /* Set wincnt accordingly */

    if( newid > wincnt )
	wincnt = newid;

    /* Do common initialization */

    wd = (struct WinDesc *)alloc(sizeof(struct WinDesc));
    memset( wd, 0, sizeof( struct WinDesc ) );
    wins[newid] = wd;

    wd->newwin = NULL;
    wd->win = w;
    wd->type = type;
    wd->flags = 0;
    wd->active = FALSE;
    wd->curx=wd->cury = 0;
    wd->resp = wd->canresp = wd->morestr = 0;   /* CHECK THESE */
    wd->offx = new_wins[type].offx;
    wd->offy = new_wins[type].offy;
    wd->maxrow = new_wins[type].maxrow;
    wd->maxcol = new_wins[type].maxcol;

    if( type != NHW_TEXT && type != NHW_MENU )
    {
	wd->rows = ( w->Height - w->BorderTop - w->BorderBottom ) /(txheight+1);
	wd->cols = ( w->Width - w->BorderLeft - w->BorderRight ) / txwidth;
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
	    if(flags.msg_history<20)flags.msg_history=20;
	    if(flags.msg_history>200)flags.msg_history=200;
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
	    wd->maxcol = 0;
	    wd->data = NULL;
	    break;

	    /* See the explanation of MENU above.  Except, wd->resp[] is not
	     * used for TEXT windows since there is no selection of a
	     * a line performed/allowed.  The window is always full
	     * screen width.
	     */
	case NHW_TEXT:
	    wd->rows = wd->maxrow = 0;
	    wd->maxcol = wd->cols = amiIDisplay->cols;
	    wd->data = NULL;
	    wd->morestr = NULL;
	    break;

	    /* The status window has only two lines.  These are stored in
	     * wd->data[], and here we allocate the space for them.
	     */
	case NHW_STATUS:
	    /* wd->cols is the number of characters which fit across the
	     * screen.
	     */
	    wd->data=(char **)alloc(3*sizeof(char *));
	    wd->data[0] = (char *)alloc(wd->cols + 10);
	    wd->data[1] = (char *)alloc(wd->cols + 10);
	    wd->data[2] = NULL;
	    break;

	    /* NHW_MAP does not use wd->data[] or the other text
	     * manipulating members of the WinDesc structure.
	     */
	case NHW_MAP:
	    SetMenuStrip(w, HackMenu);
	    break;

	    /* The base window must exist until CleanUp() deletes it. */
	case NHW_BASE:
	    /* Make our requesters come to our screen */
	    {
		register struct Process *myProcess =
					(struct Process *) FindTask(NULL);
		pr_WindowPtr = (struct Window *)(myProcess->pr_WindowPtr);
		myProcess->pr_WindowPtr = (APTR) w;
	    }

	    /* Need this for RawKeyConvert() */

	    ConsoleIO.io_Data = (APTR) w;
	    ConsoleIO.io_Length = sizeof( struct Window * );
	    ConsoleIO.io_Message.mn_ReplyPort = CreatePort(NULL, 0L);
	    if( OpenDevice("console.device", 0L,
				(struct IORequest *) &ConsoleIO, 0L) != 0)
	    {
		Abort(AG_OpenDev | AO_ConsoleDev);
	    }

	    ConsoleDevice = (struct Library *) ConsoleIO.io_Device;

	    KbdBuffered = 0;

#ifdef HACKFONT
	    if( HackFont )
		SetFont(w->RPort, HackFont);
#endif
	    txwidth = w->RPort->TxWidth;
	    txheight = w->RPort->TxHeight;
	    txbaseline = w->RPort->TxBaseline;
	    break;

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
    if (IntuitionBase)
	panic( "init_nhwindow() called twice", 0 );

    WIN_MESSAGE = WIN_ERR;
    WIN_MAP = WIN_ERR;
    WIN_STATUS = WIN_ERR;
    WIN_INVEN = WIN_ERR;
    WIN_BASE = WIN_ERR;

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

#ifdef HACKFONT
    /*
     *  Force our own font to be loaded, if possible.
     *  If we can open diskfont.library, but not our font, we can close
     *  the diskfont.library again since it just wastes memory.
     *  Even if we can open the font, we don't need the diskfont.library
     *  anymore, since CloseFont is a graphics.library function.
     */

    if ((HackFont = OpenFont(&Hack80)) == NULL)
    {
	if (DiskfontBase =
			OpenLibrary("diskfont.library", DISKFONT_VERSION))
	{
	    Hack80.ta_Name -= SIZEOF_DISKNAME;
	    HackFont = OpenDiskFont(&Hack80);
	    Hack80.ta_Name += SIZEOF_DISKNAME;
	    CloseLibrary(DiskfontBase);
	    DiskfontBase = NULL;
	}
    }
#endif

    amiIDisplay=(struct DisplayDesc *)alloc(sizeof(struct DisplayDesc));
    memset( amiIDisplay, 0, sizeof( struct DisplayDesc ) );

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
	else if( GfxBase->NormalDisplayRows >= 240 )
	{
	    bigscreen = 1;
	}
    }
    else if( bigscreen == -1 )
	bigscreen = 0;
    else if( bigscreen )
    {
	/* If bigscreen requested and we don't have enough rows in
	 * noninterlaced mode, switch to interlaced...
	 */
	if( GfxBase->NormalDisplayRows < 240 )
	{
	    amiIDisplay->ypix *= 2;
	    NewHackScreen.ViewModes |= LACE;
	}
    }
    amiIDisplay->rows = amiIDisplay->ypix / FONTHEIGHT;

    /* This is the size screen we want to open, within reason... */

    NewHackScreen.Width = max( WIDTH, amiIDisplay->xpix );
    NewHackScreen.Height = max( SCREENHEIGHT, amiIDisplay->ypix );

    if( ( HackScreen = OpenScreen( (void *)&NewHackScreen ) ) == NULL )
	Abort( AN_OpenScreen & ~AT_DeadEnd );
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	PubScreenStatus( HackScreen, 0 );
    }
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
    struct WinDesc *cw;
    struct Window *w;
    int colx;

    if( WIN_MESSAGE == WIN_ERR || ( cw = wins[ WIN_MESSAGE ] ) == NULL )
	panic(winpanicstr, WIN_MESSAGE, "get_ext_cmd");
    amii_clear_nhwindow( NHW_MESSAGE );
    pline("# ");
    colx = 3;
    w = cw->win;

    while((c = WindowGetchar()) != EOF)
    {
	amii_curs( WIN_MESSAGE, colx, 0 );
	if(c == '?' )
	{
	    int win, i, sel;
	    char buf[ 100 ];

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

	    if( sel == '\33' )
	    {
		*obufp = '\33';
		obufp[ 1 ] = 0;
	    }
	    else
	    {
		for( i = 0; extcmdlist[ i ].ef_txt != NULL; ++i )
		{
		    if( sel == extcmdlist[i].ef_txt[0] )
			break;
		}

		/* copy in the text */
		amii_clear_nhwindow( WIN_MESSAGE );
		strcpy( obufp, extcmdlist[ i ].ef_txt );
		colx = 0;
		pline( "# " );
		pline( obufp );
		bufp = obufp + 2;
	    }
	    return;
	}
	else if(c == '\033')
	{
	    *obufp = c;
	    obufp[1] = 0;
	    return;
	}
	else if(c == '\b')
	{
	    if(bufp != obufp)
	    {
		bufp--;
		amii_curs( WIN_MESSAGE, --colx, 0);
		Text( w->RPort, spaces, 1 );
		amii_curs( WIN_MESSAGE, colx, 0);
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
		if(!strncmpi(obufp, extcmdlist[oindex].ef_txt, strlen(obufp)))
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
		Text( w->RPort, bufp, strlen( bufp ) );
		amii_curs( WIN_MESSAGE, colx += strlen( bufp ), 0);
		bufp = obufp; /* reset it */
		if(strlen(obufp) < BUFSZ-1 && strlen(obufp) < COLNO)
		    bufp += strlen(obufp);
	    }
	    else
	    {
		Text( w->RPort, bufp, strlen( bufp ) );
		amii_curs( WIN_MESSAGE, colx += strlen( bufp ), 0);
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

	if(resp) {
	    allow_num = (index(resp, '#') != 0);
	    if(def)
		Sprintf(prompt, "%s [%s] (%c) ", query, resp, def);
	    else
		Sprintf(prompt, "%s [%s] ", query, resp);
	    amii_addtopl(prompt);
	} else {
	    amii_addtopl(query);
	    q = WindowGetchar();
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
	    digit_ok = allow_num && digit(q);
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
		    if (digit(z)) {
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
		amii_addtopl(rtmp);
	}
    clean_up:
	cursor_off(WIN_MESSAGE);
	clear_nhwindow(WIN_MESSAGE);
	return q;
}

#endif

/* Add a line in the message window */

void
amii_addtopl(s)
    const char *s;
{
    amii_putstr(WIN_MESSAGE,0,s);   /* is this right? */
}

void
TextSpaces( rp, nr )
    struct RastPort *rp;
    int nr;
{
    if( nr < 1 )
	return;

    while (nr > sizeof(spaces) - 1)
    {
	Text(rp, spaces, (long)sizeof(spaces) - 1);
	nr -= sizeof(spaces) - 1;
    }
    if (nr > 0)
	Text(rp, spaces, (long)nr);
}

/* Put a string into the indicated window using the indicated attribute */

void
amii_putstr(window,attr,str)
    winid window;
    int attr;
    const char *str;
{
    struct Window *w;
    register struct WinDesc *cw;
    register char *ob;
    int i, j, n0;

    /* Always try to avoid a panic when there is no window */
    if( window == WIN_ERR )
    {
	window = WIN_BASE;
	if( window == WIN_ERR )
	    window = WIN_BASE = amii_create_nhwindow( NHW_BASE );
    }

    if( window == WIN_ERR || ( cw = wins[window] ) == NULL )
    {
	flags.window_inited=0;
	panic(winpanicstr,window, "putstr");
    }

    w = cw->win;

    if(!str) return;
    amiIDisplay->lastwin = window;    /* do we care??? */

    /* NHW_MENU windows are not opened immediately, so check if we
     * have the window pointer yet
     */

    if( w )
    {
	/* Force the drawing mode and pen colors */

	SetDrMd( w->RPort, JAM2 );
	if( cw->type == NHW_STATUS )
	    SetAPen( w->RPort, attr ? C_BLUE : C_CYAN );
	else if( cw->type == NHW_MESSAGE )
	    SetAPen( w->RPort, attr ? C_RED : C_WHITE );
	else
	    SetAPen( w->RPort, attr ? C_WHITE : C_RED );
	SetBPen( w->RPort, C_BLACK );
    }
    else if( cw->type != NHW_MENU && cw->type != NHW_TEXT )
    {
	panic( "NULL window pointer in putstr 2" );
    }

    /* Okay now do the work for each type */

    switch(cw->type)
    {
#define MORE_FUDGE  10  /* 8 for --more--, 1 for preceeding sp, 1 for */
		/* putstr pad */
    case NHW_MESSAGE:
	strncpy( toplines, str, sizeof( toplines ) );
	toplines[ sizeof( toplines ) - 1 ] = 0;
	    /* Needed for initial message to be visible */
	if( cw->curx == -1 )
	{
	    amii_curs( WIN_MESSAGE, 1, 0 );
	    cw->curx = 0;
	}

	if( strlen(str) >= (cw->cols-MORE_FUDGE) )
	{
	    int i;
	    char *p;

	    while( strlen( str ) >= (cw->cols-MORE_FUDGE) )
	    {
		for(p=(&str[ cw->cols ])-MORE_FUDGE; !isspace(*p) && p != str;)
		{
		    --p;
		}
		if( p == str )
		    p = &str[ cw->cols ];
		outsubstr( cw, str, i = (long)p-(long)str );
		cw->curx += i;
		amii_cl_end( cw, cw->curx );
		str = p+1;
	    }
	    if( *str )
	    {
		outsubstr( cw, str, i = strlen( str ) );
		cw->curx += i;
		amii_cl_end( cw, cw->curx );
	    }
	}
	else
	{
	    outsubstr( cw, str, i = strlen( str ) );
	    cw->curx += i;
	    amii_cl_end( cw, cw->curx );
	}

	/* If used all of history lines, move them down */

	if( cw->maxrow == flags.msg_history )
	{
	    if( cw->data[ 0 ] )
		free( cw->data[ 0 ] );
	    memcpy( cw->data, &cw->data[ 1 ],
		( flags.msg_history - 1 ) * sizeof( char * ) );
	    cw->data[ flags.msg_history - 1 ] =
			    (char *) alloc( strlen( toplines ) + 1 );
	    strcpy( cw->data[ flags.msg_history - 1 ], toplines );
	}
	else
	{
	    /* Otherwise, allocate a new one and copy the line in */
	    cw->data[ cw->maxrow ] = (char *)
					alloc( strlen( toplines ) + 1 );
	    strcpy( cw->data[ cw->maxrow++ ], toplines );
	}
	cw->maxcol = cw->maxrow;
	break;

    case NHW_STATUS:
	if( cw->data[ cw->cury ] == NULL )
	    panic( "NULL pointer for status window" );
	ob = &cw->data[cw->cury][j = cw->curx];
	if(flags.botlx) *ob = 0;

	    /* Display when beam at top to avoid flicker... */
	WaitTOF();
	Text(w->RPort,str,strlen(str));
	if( cw->cols > strlen( str ) )
	    TextSpaces( w->RPort, cw->cols - strlen( str ) );

	(void) strncpy(cw->data[cw->cury], str, cw->cols );
	cw->data[cw->cury][cw->cols-1] = '\0'; /* null terminate */
	cw->cury = (cw->cury+1) % 2;
	cw->curx = 0;
	break;

    case NHW_MAP:
    case NHW_BASE:
	amii_curs(window, cw->curx+1, cw->cury);
	Text(w->RPort,str,strlen(str));
	cw->curx = 0;
	    /* CR-LF is automatic in these windows */
	cw->cury++;
	break;

    case NHW_MENU:
    case NHW_TEXT:

	/* always grows one at a time, but alloc 12 at a time */

	if( cw->cury >= cw->rows || !cw->data ) {
	    char **tmp;

		/* Allocate 12 more rows */
	    cw->rows += 12;
	    tmp = (char**) alloc(sizeof(char*) * cw->rows);

		/* Copy the old lines */
	    for(i=0; i<cw->cury; i++)
		tmp[i] = cw->data[i];

	    if( cw->data )
		free( cw->data );

	    cw->data = tmp;

		/* Null out the unused entries. */
	    for(i=cw->cury; i<cw->rows; i++)
		cw->data[i] = 0;
	}

	if( !cw->data )
	    panic("no data storage");

	    /* Shouldn't need to do this, but... */

	if( cw->data && cw->data[cw->cury] )
	    free( cw->data[cw->cury] );

	n0 = strlen(str)+1;
	cw->data[cw->cury] = (char*) alloc(n0+SOFF);

	    /* avoid nuls, for convenience */
	cw->data[cw->cury][VATTR] = attr+1;
	cw->data[cw->cury][SEL_ITEM] = 0;
	Strcpy( cw->data[cw->cury] + SOFF, str);

	if(n0 > cw->maxcol) cw->maxcol = n0;
	if(++cw->cury > cw->maxrow) cw->maxrow = cw->cury;
	break;

    default:
	panic("Invalid or unset window type in putstr()");
    }
}

static void
outmore( cw )
    register struct WinDesc *cw;
{
    struct Window *w = cw->win;

    Text( w->RPort, " --more--", 9 );
	/* Allow mouse clicks to clean --more-- */
    WindowGetevent( );
    amii_curs( WIN_MESSAGE, 1, 0 );
    cw->curx = 0;
    amii_cl_end( cw, cw->curx );
}

static void
outsubstr( cw, str, len )
    register struct WinDesc *cw;
    char *str;
    int len;
{
    struct Window *w = cw->win;

    if( cw->curx )
    {
	/* Check if this string and --more-- fit, if not,
	 * then put out --more-- and wait for a key.
	 */
	if( (len + MORE_FUDGE ) + cw->curx >= cw->cols )
	{
	    outmore( cw );
	}
	else
	if(topl_addspace){
	    /* Otherwise, move and put out a blank separator */
	    Text( w->RPort, spaces, 1 );
	    cw->curx += 1;
	}
    }

    Text(w->RPort,str,len);
}

/* Put a graphics character onto the screen */

void
amii_putsym( st, i, y, c )
    winid st;
    int i, y;
    CHAR_P c;
{
    char buf[ 2 ];
    amii_curs( st, i, y );
    buf[ 0 ] = c;
    buf[ 1 ] = 0;
    amii_putstr( st, 0, buf );
}

/* Clear the indicated window */

void
amii_clear_nhwindow(win)
    register winid win;
{
    register struct WinDesc *cw;
    register struct Window *w;

    if( win == WIN_ERR || ( cw = wins[win] ) == NULL )
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
	}
    }

    cw->curx = cw->cury = 0;
    amii_curs( win, 1, 0 );
}

/* Dismiss the window from the screen */

static void
dismiss_nhwindow(win)
    register winid win;
{
    register struct Window *w;
    register struct WinDesc *cw;

    if( win == WIN_ERR || ( cw = wins[win] ) == NULL )
    {
	panic(winpanicstr,win, "dismiss_nhwindow");
    }

    w = cw->win;

    if( w )
    {
	/* Map Window has this stuff attached to it. */
	if( win == WIN_MAP )
	    ClearMenuStrip( w );

	/* Close the window? */

	CloseShWindow( w );
	cw->win = NULL;

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
    if( str ){
	raw_print( "\n");	/* be sure we're not under the top margin */
	raw_print( str );
    }
}

amii_nh_poskey(x, y, mod)
    int*x, *y, *mod;
{
    struct WinDesc *cw;
    WETYPE type;
    struct RastPort *rp;
    struct Window *w;

    if( WIN_MAP != WIN_ERR && (cw = wins[ WIN_MAP ]) && ( w = cw->win ) )
	cursor_on( WIN_MAP );
    else
	panic( "no MAP window opened for nh_poskey\n" );

    rp = w->RPort;
    while( 1 )
    {
	type = WindowGetevent( );
	if( type == WEMOUSE )
	{
	    *mod = CLICK_1;
	    if( lastevent.u.mouse.qual )
		*mod = 0;

	    /* X coordinates are 1 based, Y are zero based. */
	    *x = ( (lastevent.u.mouse.x - w->BorderLeft) / txwidth ) + 1;
	    *y = ( ( lastevent.u.mouse.y - w->BorderTop-txbaseline ) /
			txheight );
	    return( 0 );
	}
	else if( type == WEKEY )
	{
	    lastevent.type = WEUNK;
	    return( lastevent.u.key );
	}
    }
}

int
amii_nhgetch()
{
    int ch;

    if( WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
	cursor_on( WIN_MAP );
    ch = WindowGetchar();
    return( ch );
}

void
amii_get_nh_event()
{
    /* nothing now - later I have no idea.  Is this just a Mac hook? */
}

void
amii_remember_topl()
{
    /* ignore for now.  I think this will be done automatically by
     * the code writing to the message window, but I could be wrong.
     */
}

int
amii_doprev_message()
{
    struct WinDesc *cw;
    struct Window *w;
    char *str;

    if( WIN_MESSAGE == WIN_ERR ||
	    ( cw = wins[ WIN_MESSAGE ] ) == NULL || ( w = cw->win ) == NULL )
    {
	panic(winpanicstr,WIN_MESSAGE, "doprev_message");
    }

    if( --cw->maxcol < 0 )
    {
	cw->maxcol = 0;
	DisplayBeep( NULL );
	str = "No more history saved...";
    }
    else
	str = cw->data[ cw->maxcol ];

    amii_cl_end(cw, 0);
    amii_curs( WIN_MESSAGE, 1, 0 );
    Text(w->RPort,str,strlen(str));
    cw->curx = cw->cols + 1;

    return( 0 );
}

void
amii_display_nhwindow(win,blocking)
    winid win;
    boolean blocking;
{
    int i;
    static int lastwin = -1;
    struct Window *w;
    struct WinDesc *cw;

    if( !Initialized )
	return;
    lastwin = win;

    if( win == WIN_ERR || ( cw = wins[win] ) == NULL )
	panic(winpanicstr,win,"display_nhwindow");

    if( cw->type == NHW_STATUS || cw->type == NHW_MESSAGE )
    {
	return;
    }

    if(cw->type==NHW_MESSAGE)
	flags.window_inited=TRUE;

    /* should be:
	if type != map, WindowToFront
	if type == map && autoshow, unblock area around cursor
	    (or something like that)
     */

    w = cw->win;
    if( w )
    {
	WindowToFront( w );
	ActivateWindow( w );
    }

    if( blocking && WIN_MAP != WIN_ERR && wins[ WIN_MAP ] )
    {
	flush_glyph_buffer( wins[ WIN_MAP ]->win );
    }

    if( cw->type == NHW_MENU || cw->type == NHW_TEXT )
    {
	DoMenuScroll( win, blocking );
    }
    else if( cw->type==NHW_MAP )
    {
	end_glyphout( win );
	for( i = 0; i < MAXWIN; ++i )
	{
	    if( cw = wins[i] )
	    {
		if( cw->type == NHW_STATUS || cw->type == NHW_MESSAGE )
		{
		    if( cw->win )
		    {
			WindowToFront(cw->win);
		    }
		}
	    }
	}

	/* Do more if it is time... */
	if( blocking == TRUE && wins[ WIN_MESSAGE ]->curx ){
	    outmore( wins[ WIN_MESSAGE ] );
	}
    }
}

void
amii_display_file(fn, complain)
const char *fn;
boolean complain;
{
    register struct WinDesc *cw;
    register int win;
    register FILE *fp;
    register char *t;
    register char buf[ 200 ];

    if( fn == NULL )
	panic("NULL file name in display_file()");

    if( ( fp = fopen( fn, "r" ) ) == NULL )
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
    if( cw = wins[ win ] )
	cw->morestr = fn;

    while( fgets( buf, sizeof( buf ), fp ) != NULL )
    {
	if( t = index( buf, '\n' ) )
	    *t = 0;
	amii_putstr( win, 0, buf );
    }
    fclose( fp );

    /* If there were lines in the file, display those lines */

    if( wins[ win ]->cury > 0 )
	amii_display_nhwindow( win, TRUE );

    wins[win]->morestr = NULL;		/* don't free title string */
    amii_destroy_nhwindow( win );
}

void
amii_curs(window, x, y)
winid window;
register int x, y;  /* not xchar: perhaps xchar is unsigned and
	       curx-x would be unsigned as well */
{
    register struct WinDesc *cw;
    register struct Window *w;
    register struct RastPort *rp;

    if( window == WIN_ERR || ( cw = wins[window] ) == NULL )
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
    if(x<0 || y<0 || y >= cw->rows || x >= cw->cols)
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
    x += cw->offx;
    y += cw->offy;

#ifdef CLIPPING
    if(clipping && window == WIN_MAP)
    {
	x -= clipx;
	y -= clipy;
    }
#endif

    /* Output all saved output before doing cursor movements for MAP */

    if( cw->type == NHW_MAP )
	flush_glyph_buffer( w );

    /* Actually do it */

    rp = w->RPort;
    if( cw->type == NHW_MENU || cw->type == NHW_TEXT )
    {
	Move( rp, (x * txwidth) + w->BorderLeft + 3,
	    (y*(txheight+1) ) + w->RPort->TxBaseline + w->BorderTop + 1 );
    }
    else if( cw->type == NHW_MAP || cw->type == NHW_BASE )
    {
	/* These coordinate calculations must be synced with those
	 * in flush_glyph_buffer() in amiwind.c.  curs_on_u() will
	 * use this code, all other drawing occurs through the glyph
	 * code.  In order for the cursor to appear on top of the hero,
	 * the code must compute X,Y in the same manner relative to
	 * the RastPort coordinates.
	 */
	Move( rp, (x * txwidth) + w->BorderLeft,
		    w->BorderTop + ( (y + 1) * txheight ) + txbaseline + 1 );
    }
    else if( cw->type == NHW_MESSAGE )
    {
	Move( rp, (x * txwidth) + w->BorderLeft + 2,
					    w->BorderTop + txbaseline + 3 );
    }
    else
    {
	Move( rp, (x * txwidth) + w->BorderLeft + 2,
			    (y*txheight) + w->BorderTop + txbaseline + 2 );
    }
}

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
    struct WinDesc *cw;
    uchar   ch;
    register int offset;
#ifdef TEXTCOLOR
    int     color;
    extern int zapcolors[];

    if( win == WIN_ERR || (cw=wins[win]) == NULL || cw->type != NHW_MAP)
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

void
DoMenuScroll( win, blocking )
    int win, blocking;
{
    register struct Window *w;
    register struct NewWindow *nw;
    struct PropInfo *pip;
    register struct WinDesc *cw;
    struct IntuiMessage *imsg;
    struct Gadget *gd;
    register int wheight, xsize, ysize, aredone = 0;
    register int txwd, txh;
    long mics, secs, class, code;
    long oldmics = 0, oldsecs = 0;
    int aidx, oidx, topidx, hidden;
    char *t;
    SHORT mx, my;
    char title[ 100 ];
    int dosize = 1;
    struct Screen *scrn = HackScreen;

    if( win == WIN_ERR || ( cw = wins[ win ] ) == NULL )
	panic(winpanicstr,win,"DoMenuScroll");

    /* Check to see if we should open the window, should usually need to */

    txwd = txwidth;
    txh = txheight + 1; /* +1 for interline space */
    w = cw->win;
    if( w == NULL )
    {
	/* Humm, there is only so much... */
	if( scrn )
	     xsize = scrn->WBorLeft + scrn->WBorRight + MenuScroll.Width + 2;
	else
	     xsize = 5 + 5 + MenuScroll.Width + 2;
	xsize += (txwd * cw->maxcol);
	if( xsize > amiIDisplay->xpix )
	    xsize = amiIDisplay->xpix;

	    /* If next row not off window, use it, else use the bottom */

	ysize = ( txh * cw->maxrow ) +          /* The text space */
	  HackScreen->WBorTop + txheight + 1 +    /* Top border */
	  HackScreen->WBorBottom + 1;     /* The bottom border */
	if( ysize > amiIDisplay->ypix )
	    ysize = amiIDisplay->ypix;

	/* Adjust the size of the menu scroll gadget */

	nw = (void *)DupNewWindow( (void *)(&new_wins[ cw->type ].newwin) );
	cw->newwin = (void *)nw;
	if( nw == NULL )
	    panic("No NewWindow Allocated" );

	nw->Screen = HackScreen;

	if( win == WIN_INVEN ){
	    sprintf( title, "%s the %s's Inventory", plname, pl_character );
	    nw->Title = title;
	}
	else if( cw->morestr )
	    nw->Title = cw->morestr;

	/* Adjust the window coordinates and size now that we know
	 * how many items are to be displayed.
	 */

	nw->Width = xsize;
	nw->Height = ysize;
	nw->TopEdge = 0;
	if( cw->type == NHW_TEXT && ysize < amiIDisplay->ypix )
	    nw->TopEdge += ( amiIDisplay->ypix - ysize ) / 2;
	nw->LeftEdge = amiIDisplay->xpix - xsize;
	if( cw->type == NHW_TEXT && xsize < amiIDisplay->xpix )
	    nw->LeftEdge -= ( amiIDisplay->xpix - xsize ) / 2;


	/* Now, open the window */
	w = cw->win = OpenShWindow( (void *)nw );

	if( w == NULL )	panic("No Window Opened For Menu" );
    }

    /* Find the scroll gadget */

    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; gd = gd->NextGadget )
	continue;

    if( !gd ) panic("Can't find scroll gadget" );

    wheight = ( w->Height - w->BorderTop - w->BorderBottom ) / txh;
    cw->cols = ( w->Width - w->BorderLeft - w->BorderRight
	    /* + MenuScroll.Width*/ ) / txwd;
    morc = 0;
    oidx = -1;
    topidx = 0;

    /* Make the prop gadget the right size and place */

    DisplayData( win, topidx, -1 );

    SetPropInfo( w, gd, wheight, cw->maxrow, topidx );
    oldsecs = oldmics = 0;

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

		    /* Find the gadget */

		    for( gd = w->FirstGadget; gd && gd->GadgetID != 1; )
			gd = gd->NextGadget;

		    if( !gd )
			panic("Can't find scroll gadget" );

		    wheight = ( w->Height - w->BorderTop -
						w->BorderBottom - 1) / txh;
		    cw->cols = ( w->Width -
				w->BorderLeft - w->BorderRight ) / txwd;
		    if( wheight < 2 )
			wheight = 2;

		    /* Make the prop gadget the right size and place */

		    DisplayData( win, topidx, oidx );
		    SetPropInfo( w, gd, wheight, cw->maxrow, topidx );

		    /* Force the window to a text line boundry <= to
		     * what the user dragged it to.  This eliminates
		     * having to clean things up on the bottom edge.
		     */

		    SizeWindow( w, 0, ( wheight * txh) +
			    w->BorderTop + w->BorderBottom + 1 - w->Height );

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
			    SetPropInfo( w,gd, wheight, cw->maxrow, topidx );
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
			SetPropInfo( w, gd, wheight, cw->maxrow, topidx );
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
						    cw->maxrow, topidx );
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
						    cw->maxrow, topidx );
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
		    aredone = 1;
		    morc = '\33';
		    break;

		case GADGETUP:
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

		case MOUSEBUTTONS:
		    if( ( code == SELECTUP || code == SELECTDOWN ) &&
						    cw->type == NHW_MENU )
		    {
			/* Which one is the mouse pointing at? */

			aidx = (( (my - w->TopEdge) - w->BorderTop + 1 ) /
							txh) + topidx;

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
				amii_curs( win, 1, oidx - topidx );
				SetDrMd( w->RPort, JAM2 );
				SetAPen( w->RPort, C_WHITE );
				SetBPen( w->RPort, C_BLACK );
				Text( w->RPort, t, strlen( t ) );
				oidx = -1;
			    }

			    t = cw->data[ aidx ];
			    if( t[ SEL_ITEM ] == 1 )
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
}

void
DisplayData( win, start, where )
    int win;
    int start;
    int where;
{
    register struct WinDesc *cw;
    register struct Window *w;
    register int i, len, wheight;
    int col = -1;

    if( win == WIN_ERR || !(cw = wins[win]) || !( w = cw->win ) )
    {
	panic( winpanicstr, win, "No Window in DisplayData" );
    }

    SetDrMd( w->RPort, JAM2 );
    wheight = ( w->Height - w->BorderTop - w->BorderBottom ) / ( txheight + 1 );

    for( i = start; i < start + wheight; ++i )
    {
	amii_curs( win, 1, i - start );

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
	if( i < cw->maxrow )
	{
	    register char *t;

	    t = cw->data[ i ] + SOFF;
	    len = strlen( t );
	    if( len > cw->cols )
		len = cw->cols;
	    Text( w->RPort, t, len );
	}
	amii_cl_end( cw, len );
    }
    return;
}

void SetPropInfo( win, gad, vis, total, top )
    register struct Window *win;
    register struct Gadget *gad;
    register long vis, total, top;
{
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

    NewModifyProp( gad, win, NULL,
			    AUTOKNOB|FREEVERT, 0, pot, MAXBODY, body, 1 );
}

void
kill_nhwindows( all )
    register int all;
{
    register int i;
    register struct WinDesc *cw;

    /* Foreach open window in all of wins[], CloseShWindow, free memory */

    for( i = 0; i < MAXWIN; ++i )
    {
	if( (cw = wins[ i ]) && (cw->type != NHW_BASE || all) )
	{
	    CloseShWindow( cw->win );
	    free( cw );
	    wins[ i ] = NULL;
	}
    }
}

void
amii_cl_end( cw, i )
    register struct WinDesc *cw;
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

void cursor_off( window )
    winid window;
{
    register struct WinDesc *cw;
    register struct Window *w;
    register struct RastPort *rp;
    int curx, cury;
    long dmode;
    short apen, bpen;
    unsigned char ch;

    if( window == WIN_ERR || ( cw = wins[window] ) == NULL )
    {
	/* tty does this differently - is this OK? */
	flags.window_inited=0;
	panic(winpanicstr,window, "cursor_off");
    }

    if( !(cw->flags & FLMAP_CURSUP ) )
	return;
    w = cw->win;

    if( !w )
	return;

    cw->flags &= ~FLMAP_CURSUP;
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

    /* Put back the other stuff */

    Move( rp, curx, cury );
    SetDrMd( rp, dmode );
    SetAPen( rp, apen );
    SetBPen( rp, bpen );
}

void cursor_on( window )
    winid window;
{
    register struct WinDesc *cw;
    register struct Window *w;
    register struct RastPort *rp;
    unsigned char ch;
    long dmode;
    short apen, bpen;

    if( window == WIN_ERR || ( cw = wins[window] ) == NULL )
    {
	/* tty does this differently - is this OK? */
	flags.window_inited=0;
	panic(winpanicstr,window, "cursor_on");
    }

    if( (cw->flags & FLMAP_CURSUP ) )
	cursor_off( window );

    w = cw->win;

    if( !w )
	return;

    cw->flags |= FLMAP_CURSUP;
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

    SetDrMd( rp, dmode );
    SetAPen( rp, apen );
    SetBPen( rp, bpen );
}

void
amii_getret()
{
    register int c;

    raw_print( "" );
    raw_print( "Press Return..." );

    c = 0;

    while( c != '\n' && c != '\r' )
    {
	if( HackPort )
	    c = WindowGetchar();
	else
	    c = getchar();
    }
    return;
}

UBYTE UNDOBUFFER[300];
SHORT BorderVectors1[] = { 0,0, 57,0, 57,11, 0,11, 0,0 };
struct Border Border1 = { -1,-1, 3,0,JAM1, 5, BorderVectors1, NULL };
struct IntuiText IText1 = { 3,0,JAM2, 4,1, NULL, (UBYTE *)"Cancel", NULL };
struct Gadget Gadget2 = {
    NULL, 9,15, 56,10, NULL, RELVERIFY, BOOLGADGET, (APTR)&Border1,
    NULL, &IText1, NULL, NULL, 1, NULL
};
UBYTE StrStringSIBuff[300];
struct StringInfo StrStringSInfo = {
    StrStringSIBuff, UNDOBUFFER, 0, 300, 0, 0,0,0,0,0, 0, 0, NULL
};
SHORT BorderVectors2[] = { 0,0, 439,0, 439,11, 0,11, 0,0 };
struct Border Border2 = { -1,-1, 3,0,JAM1, 5, BorderVectors2, NULL };
struct Gadget String = {
    &Gadget2, 77,15, 438,10, NULL, RELVERIFY+STRINGCENTER, STRGADGET,
    (APTR)&Border2, NULL, NULL, NULL, (APTR)&StrStringSInfo, 2, NULL
};

#define StrString \
   ((char *)(((struct StringInfo *)(String.SpecialInfo))->Buffer))

struct NewWindow StrWindow = {
    57,74, 526,31, 0,1, GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
    WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
    &String, NULL, NULL, NULL, NULL, 5,5, 0xffff,0xffff, CUSTOMSCREEN
};

/* Generate a requester for a string value. */

void amii_getlin(prompt,bufp)
    const char *prompt;
    char *bufp;
{
    getlind(prompt,bufp,0);
}

/* and with default */
void getlind(prompt,bufp, dflt)
    const char *prompt;
    char *bufp;
    const char *dflt;
{
#ifndef TOPL_GETLINE
    register struct Window *cwin;
    register struct IntuiMessage *imsg;
    register long class, code, qual;
    register int aredone = 0;
    register struct Gadget *gd;
    static int once;

    *StrString = 0;
    if( dflt )
	strcpy( StrString, dflt );
    StrWindow.Title = (UBYTE *)prompt;
    StrWindow.Screen = HackScreen;

    if( !once )
    {
	if( bigscreen )
	    StrWindow.TopEdge = (HackScreen->Height/2) - (StrWindow.Height/2);
	SetBorder( &String );
	SetBorder( &Gadget2 );
	once = 1;
    }

    if( ( cwin = OpenWindow( (void *)&StrWindow ) ) == NULL )
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
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *) imsg->IAddress;

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == '\033' && (qual &
			    (IEQUALIFIER_LALT|IEQUALIFIER_RALT|
			    IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND) ) == 0 )
		    {
			if( bufp )
			{
			    bufp[0]='\033';
			    bufp[1]=0;
			}
			aredone = 1;
		    }
		    else
		    {
			ActivateGadget( &String, cwin, NULL );
		    }
		    break;

		case ACTIVEWINDOW:
		    ActivateGadget( &String, cwin, NULL );
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
			case 2:
			    aredone = 1;
			    if( bufp )
				strcpy( bufp, StrString );
			    break;

			case 1:
			    if( bufp )
			    {
				bufp[0]='\033';
				bufp[1]=0;
			    }
			    aredone = 1;
			    break;
		    }
		    break;

		case CLOSEWINDOW:
		    if( bufp )
			strcpy( bufp, StrString );
		    aredone = 1;
		    break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }

    CloseWindow( cwin );
#else
    struct WinDesc *cw;
    struct Window *w;
    int colx, ocolx, c;
    char *obufp;

    amii_clear_nhwindow( WIN_MESSAGE );
    amii_putstr( WIN_MESSAGE, 0, prompt );
    cw = wins[ WIN_MESSAGE ];
    w = cw->win;
    ocolx = colx = strlen( prompt ) + 1;

    obufp = bufp;
    while((c = WindowGetchar()) != EOF)
    {
	amii_curs( WIN_MESSAGE, colx, 0 );
	if(c == '\033')
	{
	    *obufp = c;
	    obufp[1] = 0;
	    return;
	}
	else if(c == '\b')
	{
	    if(bufp != obufp)
	    {
		bufp--;
		amii_curs( WIN_MESSAGE, --colx, 0);
		Text( w->RPort, "\177 ", 2 );
		amii_curs( WIN_MESSAGE, colx, 0);
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

	    Text( w->RPort, bufp, 1 );
	    Text( w->RPort, "\177", 1 );
	    if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
	    {
		colx++;
		bufp++;
	    }
	}
	else if(c == ('X'-64) || c == '\177')
	{
	    amii_curs( WIN_MESSAGE, ocolx, 0 );
	    Text( w->RPort,
		"                                                            ",
		colx - ocolx );
	    amii_curs( WIN_MESSAGE, colx = ocolx, 0 );
	} else
	    DisplayBeep( NULL );
    }
    *bufp = 0;
#endif
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

#define GADBLUEPEN      2
#define GADREDPEN       3
#define GADGREENPEN     4
#define GADCOLOKAY      5
#define GADCOLCANCEL    6
#define GADCOLSAVE      7

#include "colorwin.c"

void
EditColor()
{
    int i, done = 0, okay = 0;
    long code, qual, class;
    register struct Gadget *gd, *dgad;
    register struct Window *nw;
    register struct IntuiMessage *imsg;
    register struct PropInfo *pip;
    register struct Screen *scrn;
    long aidx;
    int msx, msy;
    int curcol = 0, drag = 0;
    int bxorx, bxory, bxxlen, bxylen;
    UWORD colors[ 1L << DEPTH ];
    static int once = 0;

    bxylen = Col_NewWindowStructure1.Height -
			    ( Col_BluePen.TopEdge + Col_BluePen.Height + 6 );
    bxxlen = Col_BluePen.Width;
    bxorx = Col_BluePen.LeftEdge;
    bxory = Col_BluePen.TopEdge + Col_BluePen.Height + 2;
    scrn = HackScreen;

    if( !once )
    {
	SetBorder( &Col_Okay );
	SetBorder( &Col_Cancel );
	SetBorder( &Col_Save );
	once = 1;
    }

    for( i = 0; i < (1L << DEPTH); ++i )
    {
	colors[ i ] = GetRGB4( scrn->ViewPort.ColorMap, i );
    }

    Col_NewWindowStructure1.Screen = scrn;
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	((struct PropInfo *)Col_BluePen.SpecialInfo)->Flags |= PROPNEWLOOK;
	((struct PropInfo *)Col_RedPen.SpecialInfo)->Flags |=  PROPNEWLOOK;
	((struct PropInfo *)Col_GreenPen.SpecialInfo)->Flags |= PROPNEWLOOK;
    }
#endif
    nw = OpenWindow( (void *)&Col_NewWindowStructure1 );

    DrawCol( nw, curcol, colors );
    while( !done )
    {
	WaitPort( nw->UserPort );

	while( imsg = (struct IntuiMessage * )GetMsg( nw->UserPort ) )
	{
	    gd = (struct Gadget *)imsg->IAddress;
	    code = imsg->Code;
	    class = imsg->Class;
	    qual = imsg->Qualifier;
	    msx = imsg->MouseX;
	    msy = imsg->MouseY;

	    ReplyMsg( (struct Message *)imsg );

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == 'v' && qual == AMIGALEFT )
			okay = done = 1;
		    else if( code == 'b' && qual == AMIGALEFT )
			okay = 0, done = 1;
		    else if( code == 'o' || code == 'O' )
			okay = done = 1;
		    else if( code == 'c' || code == 'C' )
			okay = 0, done = 1;
		    break;

		case CLOSEWINDOW:
		    done = 1;
		    break;

		case GADGETUP:
		    drag = 0;
		    if( gd->GadgetID == GADREDPEN ||
					    gd->GadgetID == GADBLUEPEN ||
						gd->GadgetID == GADGREENPEN )
		    {
			pip = (struct PropInfo *)gd->SpecialInfo;
			aidx = pip->HorizPot / (MAXPOT/15);
			aidx = aidx;
			if( gd->GadgetID == GADREDPEN )
			{
			    colors[ curcol ] =
				( colors[ curcol ] & ~0xf00 ) | (aidx << 8);
			    LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
			}
			else if( gd->GadgetID == GADBLUEPEN )
			{
			    colors[ curcol ] =
					( colors[ curcol ] & ~0xf ) | aidx;
			    LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
			}
			else if( gd->GadgetID == GADGREENPEN )
			{
			    colors[ curcol ] =
				( colors[ curcol ] & ~0x0f0 ) | (aidx << 4);
			    LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
			}
			DispCol( nw, curcol, colors );
		    }
		    else if( gd->GadgetID == GADCOLOKAY )
		    {
			done = 1;
			okay = 1;
		    }
		    else if( gd->GadgetID == GADCOLSAVE )
		    {
		    	FILE *fp, *nfp;
		    	char buf[ 300 ];
				int once = 0;

    			fp = fopen( "nethack.cnf", "r" );
    			if( !fp )
    			{
			    pline( "can't find nethack.cnf" );
                    	    break;
                    	}

    			nfp = fopen( "new_nethack.cnf", "w" );
    			if( !nfp )
    			{
			    pline( "can't write to new_nethack.cnf" );
                    	    fclose( fp );
                    	    break;
                    	}
			while( fgets( buf, sizeof( buf ), fp ) )
			{
			    if( strncmp( buf, "PENS=", 5 ) == 0 )
			    {
				once = 1;
			    	fputs( "PENS=", nfp );
			    	for( i = 0; i < (1l << DEPTH); ++i )
			    	{
			    	    fprintf( nfp, "%03x", colors[i] );
			    	    if(( i + 1 ) < (1l << DEPTH))
			    	        putc( ',', nfp );
			    	}
		    	        putc( '\n', nfp );
			    }
			    else
			    {
			    	fputs( buf, nfp );
			    }
			}

			/* If none in the file yet, now write it */
			if( !once )
			{
    		    	    fputs( "PENS=", nfp );
    		    	    for( i = 0; i < (1l << DEPTH); ++i )
    		    	    {
    		    	        fprintf( nfp, "%03x", colors[i] );
    		    	        if(( i + 1 ) < (1l << DEPTH))
    		    	            putc( ',', nfp );
    		    	    }
    	    	            putc( '\n', nfp );
			}
			fclose( fp );
			fclose( nfp );
			unlink( "old_nethack.cnf" );
			rename( "nethack.cnf", "old_nethack.cnf" );
			rename( "new_nethack.cnf", "nethack.cnf" );
			done = 1;
			okay = 1;
		    }
		    else if( gd->GadgetID == GADCOLCANCEL )
		    {
			done = 1;
			okay = 0;
		    }
		    break;

		case GADGETDOWN:
		    drag = 1;
		    dgad = gd;
		    break;

		case MOUSEMOVE:
		    if( !drag )
			break;
		    pip = (struct PropInfo *)dgad->SpecialInfo;
		    aidx = pip->HorizPot / (MAXPOT/15);
		    aidx = aidx;
		    if( dgad->GadgetID == GADREDPEN )
		    {
			colors[ curcol ] =
				( colors[ curcol ] & ~0xf00 ) | (aidx << 8);
			LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
		    }
		    else if( dgad->GadgetID == GADBLUEPEN )
		    {
			colors[ curcol ] = ( colors[ curcol ] & ~0xf ) | aidx;
			LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
		    }
		    else if( dgad->GadgetID == GADGREENPEN )
		    {
			colors[ curcol ] =
				( colors[ curcol ] & ~0x0f0 ) | (aidx << 4);
			LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
		    }
		    DispCol( nw, curcol, colors );
		    break;

		case MOUSEBUTTONS:
		    if( code == SELECTDOWN )
		    {
			if( msy > bxory && msy < bxory + bxylen - 1 &&
				msx > bxorx && msx < bxorx + bxxlen - 1 )
			{
			    curcol = ( msx - bxorx )/(bxxlen / (1l << DEPTH));
			    DrawCol( nw, curcol, colors );
			}
		    }
		    break;
	    }
	}
    }

    if( okay )
    {
	for( i = 0; i < ( 1L << DEPTH ); ++i )
	    flags.amii_curmap[ i ] = colors[ i ];
    }

    LoadRGB4( &scrn->ViewPort, flags.amii_curmap, 1L << DEPTH );
    CloseWindow( nw );
}

/* The colornames, and the default values for the pens */
static struct
{
    char *name, *defval;
} colnames[] =
{
    "Black","(aaa)",
    "White","(fff)",
    "Brown","(620)",
    "Cyan","(b08)",
    "Green","(181)",
    "Magenta","(c06)",
    "Blue","(23e)",
    "Red","(d00)",
};

void
DrawCol( w, idx, colors )
    struct Window *w;
    int idx;
    UWORD *colors;
{
    int bxorx, bxory, bxxlen, bxylen;
    int i, incx, incy, r, g, b;
    long flags;

    bxylen = Col_NewWindowStructure1.Height - (Col_Okay.Height + txheight + 8) -
		    ( Col_BluePen.TopEdge + Col_BluePen.Height + 6 );
    bxxlen = Col_BluePen.Width;
    bxorx = Col_BluePen.LeftEdge + 2;
    bxory = Col_BluePen.TopEdge + Col_BluePen.Height + 2;

    incx = bxxlen / (1L << DEPTH);
    incy = bxylen - 2;

    SetAPen( w->RPort, C_WHITE );
    SetBPen( w->RPort, C_BLACK );
    SetDrMd( w->RPort, JAM2 );
    RectFill( w->RPort, bxorx, bxory, bxorx + bxxlen - 1, bxory + bxylen );

    SetAPen( w->RPort, C_BLACK );
    RectFill( w->RPort, bxorx+2, bxory+1,
				    bxorx + bxxlen - 4, bxory + bxylen - 1);

    for( i = 0; i < (1L << DEPTH); ++i )
    {
	if( i == idx )
	{
	    SetAPen( w->RPort, scrnpens[ SHADOWPEN ] );
	    Move( w->RPort, bxorx + 3 + (i*incx)+0, bxory+bxylen - 1);
	    Draw( w->RPort, bxorx + 3 + (i*incx)+0, bxory + 1 );
	    Draw( w->RPort, bxorx + ((i+1)*incx)-1, bxory + 1 );

	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + 3 + (i*incx)+1, bxory + 2 );
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2 );

	    SetAPen( w->RPort, scrnpens[ SHINEPEN ] );
	    Move( w->RPort, bxorx + 3 + (i*incx)+0, bxory+bxylen - 1);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-1, bxory+bxylen - 1);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-1, bxory + 1 );

	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2);
	}
	else
	{
	    SetAPen( w->RPort, scrnpens[ SHINEPEN ] );
	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + 3 + (i*incx)+1, bxory + 2 );
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2 );
	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2);
	}

	SetAPen( w->RPort, i );
	RectFill( w->RPort, bxorx + 3 + (i*incx)+3, bxory + 4,
				    bxorx + ((i+1)*incx)-4, bxory+bxylen - 4);
    }

    DispCol( w, idx, colors );

    r = (colors[ idx ] & 0xf00) >> 8;
    g = (colors[ idx ] & 0x0f0) >> 4;
    b = colors[ idx ] & 0x00f;

    flags = AUTOKNOB|FREEHORIZ;
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	flags |= PROPNEWLOOK;
    }
#endif
    NewModifyProp( &Col_RedPen, w, NULL, flags, (r * MAXPOT ) / 15, 0,
							    MAXPOT/15, 0, 1 );
    NewModifyProp( &Col_GreenPen, w, NULL, flags, (g * MAXPOT ) / 15, 0,
							    MAXPOT/15, 0, 1 );
    NewModifyProp( &Col_BluePen, w, NULL, flags, (b * MAXPOT ) / 15, 0,
							    MAXPOT/15, 0, 1 );
}

void
DispCol( w, idx, colors )
    struct Window *w;
    int idx;
    UWORD *colors;
{
    char buf[ 50 ];

    Move( w->RPort, Col_Save.LeftEdge,
	Col_Save.TopEdge - 4 );
    sprintf( buf, "%s=%03x default=%s%s", colnames[idx].name, colors[idx],
	colnames[idx].defval,
	"        "+strlen(colnames[idx].name)+1 );
    SetAPen( w->RPort, C_WHITE );
    SetBPen( w->RPort, 0 );
    SetDrMd( w->RPort, JAM2 );
    Text( w->RPort, buf, strlen( buf ) );
}

void
amii_setpens()
{
    /* If the pens are set in NetHack.cnf, we can get called before
     * HackScreen has been opened...
     */
    if( HackScreen != NULL )
    {
	LoadRGB4( &HackScreen->ViewPort, flags.amii_curmap, 1L << DEPTH );
    }
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
	    bp[ i ].FrontPen = ( i == 0 || i == 1 ) ? C_BROWN : C_WHITE;

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
			    ( i == 1 || i == 2 ) ? C_BROWN : C_WHITE;
	    }
	    else
	    {
		bp[ i ].FrontPen =
			    ( i == 1 || i == 3 ) ? C_WHITE : C_BROWN;
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

#ifdef  PORT_HELP
void
port_help()
{
    display_file( PORT_HELP, 1 );
}
#endif

static void
removetopl(cnt)
	int cnt;
{
    struct WinDesc *cw=wins[WIN_MESSAGE];
					/* NB - this is sufficient for
					 * yn_function, but that's it
					 */
    if(cw->curx < cnt)cw->curx=0;
    else cw->curx -= cnt;

    amii_curs(WIN_MESSAGE, cw->curx+1, 0);
    amii_cl_end(cw, cw->curx);
}
#endif /* AMIGA_INTUITION */
