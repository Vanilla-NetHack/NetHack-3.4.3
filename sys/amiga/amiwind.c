/*    SCCS Id: @(#)amiwind.c     3.1    93/01/08
/*    Copyright (c) Olaf Seibert (KosmoSoft), 1989, 1992	  */
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland 1993	  */
/* NetHack may be freely redistributed.  See license for details. */

#include "amiga:windefs.h"
#include "amiga:winext.h"
#include "amiga:winproto.h"

/* Have to undef CLOSE as display.h and intuition.h both use it */
#undef CLOSE

#ifdef AMII_GRAPHICS	/* too early in the file? too late? */

#ifndef	SHAREDLIB
struct Library *ConsoleDevice;
#endif

#include "Amiga:amimenu.c"

static int BufferGetchar(void);
static void ProcessMessage( register struct IntuiMessage *message );

#ifdef AMIFLUSH
static struct Message *FDECL(GetFMsg,(struct MsgPort *));
#endif

/*  Now our own variables */

struct IntuitionBase *IntuitionBase;
#ifndef	SHAREDLIB
struct Screen *HackScreen;
#endif
struct Window *pr_WindowPtr;
struct MsgPort *HackPort;
struct IOStdReq ConsoleIO;
#ifndef	SHAREDLIB
char Initialized = 0;
#endif
WEVENT lastevent;

#ifdef HACKFONT
struct GfxBase *GfxBase;
struct Library *DiskfontBase;
#endif

#ifndef	SHAREDLIB
extern struct Library *ConsoleDevice;
#endif

#define KBDBUFFER   10
static unsigned char KbdBuffer[KBDBUFFER];
unsigned char KbdBuffered;

#define BufferQueueChar(ch) (KbdBuffer[KbdBuffered++] = (ch))

/*
 * Define some stuff for our special glyph drawing routines
 */
static unsigned short glyph_node_index, glyph_buffer_index;
#define NUMBER_GLYPH_NODES  80
#define GLYPH_BUFFER_SIZE   512
struct glyph_node {
    short	x;
    short	y;
    short	len;
    unsigned char   bg_color;
    unsigned char   fg_color;
    char	*buffer;
};
static struct glyph_node g_nodes[NUMBER_GLYPH_NODES];
static char glyph_buffer[GLYPH_BUFFER_SIZE];

#ifdef TEXTCOLOR
/*
 * Map our amiga-specific colormap into the colormap specified in color.h.
 * See amiwind.c for the amiga specific colormap.
 */

#ifdef	VIEWWINDOW
int foreg[16] = { 8, 7, 4, 2, 6, 5, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
int backg[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 1, 6, 5, 3, 1 };
#else
int foreg[16] = { 0, 7, 4, 2, 6, 5, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
int backg[16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 1, 6, 5, 3, 1 };
#endif
#if 0
	#define BLACK		0
	#define RED		1
	#define GREEN		2
	#define BROWN		3	/* on IBM, low-intensity yellow is brown */
	#define BLUE		4
	#define MAGENTA 	5
	#define CYAN		6
	#define GRAY		7	/* low-intensity white */
	#define NO_COLOR	8
	#define ORANGE_COLORED	9	/* "orange" conflicts with the object */
	#define BRIGHT_GREEN	10
	#define YELLOW		11
	#define BRIGHT_BLUE	12
	#define BRIGHT_MAGENTA  13
	#define BRIGHT_CYAN	14
	#define WHITE		15
	#define MAXCOLORS	16
#endif
#endif

#ifdef HACKFONT

struct TextFont *TextsFont;
struct TextFont *HackFont;
#ifdef	VIEWWINDOW
struct TextFont *HackFont4;
struct TextFont *HackFont16;
#endif
UBYTE FontName[] = "NetHack:hack.font";
    /* # chars in "NetHack:": */
#define         SIZEOF_DISKNAME 8

#endif

struct TextAttr Hack80 = {
#ifdef HACKFONT
    &FontName[SIZEOF_DISKNAME],
#else
    (UBYTE *) "topaz.font",
#endif
    8, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED
	| FPF_ROMFONT
};

#ifdef	VIEWWINDOW
struct TextAttr Hack40 = {
#ifdef HACKFONT
    &FontName[SIZEOF_DISKNAME],
#else
    (UBYTE *) "topaz.font",
#endif
    4, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED
#ifndef	HACKFONT
	| FPF_ROMFONT
#endif
};

struct TextAttr Hack160 = {
#ifdef HACKFONT
    &FontName[SIZEOF_DISKNAME],
#else
    (UBYTE *) "topaz.font",
#endif
    16, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED
#ifndef	HACKFONT
	| FPF_ROMFONT
#endif
};
#endif

struct TextAttr TextsFont13 = {
    (UBYTE *) "courier.font",
    13, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED
#ifndef	HACKFONT
	| FPF_ROMFONT
#endif
};

/*
 * Open a window that shares the HackPort IDCMP. Use CloseShWindow()
 * to close.
 */

struct Window *OpenShWindow(nw)
struct NewWindow *nw;
{
    register struct Window *win;
    register ULONG idcmpflags;

    if (!HackPort)  /* Sanity check */
	return (struct Window *) 0;

    idcmpflags = nw->IDCMPFlags;
    nw->IDCMPFlags = 0;
    if (!(win = OpenWindow((void *)nw)))
	return (struct Window *) 0;

    win->UserPort = HackPort;
    ModifyIDCMP(win, idcmpflags);
    return win;
}


/*
 * Close a window that shared the HackPort IDCMP port.
 */

void FDECL(CloseShWindow, (struct Window *));
void CloseShWindow(win)
struct Window *win;
{
    register struct IntuiMessage *msg;

    if( !HackPort )
	panic("HackPort NULL in CloseShWindow" );
    if (!win)
	return;

    Forbid();
    /* Flush all messages for all windows to avoid typeahead and other
     * similar problems...
     */
    while( msg = (struct IntuiMessage *)GetMsg( win->UserPort ) )
	ReplyMsg( (struct Message *) msg );
    KbdBuffered = 0;
    win->UserPort = (struct MsgPort *) 0;
    ModifyIDCMP(win, 0L);
    Permit();
    CloseWindow(win);
}

static int BufferGetchar()
{
    register int c;

    if (KbdBuffered > 0) {
	c = KbdBuffer[0];
	KbdBuffered--;
	/* Move the remaining characters */
	if( KbdBuffered < sizeof( KbdBuffer ) )
	    memcpy( KbdBuffer, KbdBuffer+1, KbdBuffered );
	return c;
    }

    return NO_CHAR;
}

/*
 *  This should remind you remotely of DeadKeyConvert, but we are cheating
 *  a bit. We want complete control over the numeric keypad, and no dead
 *  keys... (they are assumed to be on Alted keys).
 *
 *  Also assumed is that the IntuiMessage is of type RAWKEY.  For some
 *  reason, IECODE_UP_PREFIX events seem to be lost when they  occur while
 *  our console window is inactive. This is particulary  troublesome with
 *  qualifier keys... Is this because I never RawKeyConvert those events???
 */

int ConvertKey(message)
register struct IntuiMessage *message;
{
    static struct InputEvent theEvent;
    static char       numpad[] = "bjnh.lyku";
    static char  ctrl_numpad[] = "\x02\x0A\x0E\x08.\x0C\x19\x0B\x15";
    static char shift_numpad[] = "BJNH.LYKU";

    unsigned char buffer[1];
    register int length;
    register ULONG qualifier;
    char numeric_pad, shift, control, alt;

    qualifier = message->Qualifier;

    control = (qualifier &  IEQUALIFIER_CONTROL) != 0;
    shift   = (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)) != 0;
    alt     = (qualifier & (IEQUALIFIER_LALT   | IEQUALIFIER_RALT  )) != 0;

    /* Allow ALT to function as a META key ... */

    qualifier &= ~(IEQUALIFIER_LALT | IEQUALIFIER_RALT);
    numeric_pad = (qualifier & IEQUALIFIER_NUMERICPAD) != 0;

    /*
     *  Shortcut for HELP and arrow keys. I suppose this is allowed.
     *  The defines are in intuition/intuition.h, and the keys don't
     *  serve 'text' input, normally. Also, parsing their escape
     *  sequences is such a mess...
     */

    switch (message->Code) {
	case RAWHELP:
	    if( alt )
	    {
		EditColor();
		return( -1 );
	    }
	    return( '?' );
	    break;
	case CURSORLEFT:
	    length = '4';
	    numeric_pad = 1;
	    goto arrow;
	case CURSORDOWN:
	    length = '2';
	    numeric_pad = 1;
	    goto arrow;
	case CURSORUP:
	    length = '8';
	    numeric_pad = 1;
	    goto arrow;
	case CURSORRIGHT:
	    length = '6';
	    numeric_pad = 1;
	    goto arrow;
    }

    theEvent.ie_Class = IECLASS_RAWKEY;
    theEvent.ie_Code = message->Code;
    theEvent.ie_Qualifier = numeric_pad ? IEQUALIFIER_NUMERICPAD : qualifier;
    theEvent.ie_EventAddress = (APTR) (message->IAddress);

    length = RawKeyConvert(&theEvent, (char *)buffer, 
      (long) sizeof(buffer), NULL);

    if (length == 1) {   /* Plain ASCII character */
	length = buffer[0];
	/*
	 *  If flags.num_pad is set, movement is by 4286.
	 *  If not set, translate 4286 into hjkl.
	 *  This way, the numeric pad can /always/ be used
	 *  for moving, though best results are when it is off.
	 */
arrow:
	if (!flags.num_pad && numeric_pad && length >= '1' && length <= '9') {
	    length -= '1';
	    if (control) {
		length = ctrl_numpad[length];
	    } else if (shift) {
		length = shift_numpad[length];
	    } else {
		length = numpad[length];
	    }
	}
	if (alt)
	    length |= 0x80;
	return(length);
    } /* else shift, ctrl, alt, amiga, F-key, shift-tab, etc */
    else
	return( -1 );
}

/*
 *  Process an incoming IntuiMessage.
 *  It would certainly look nicer if this could be done using a
 *  PA_SOFTINT message port, but we cannot call RawKeyConvert()
 *  during a software interrupt.
 *  Anyway, amikbhit()/kbhit() is called often enough, and usually gets
 *  ahead of input demands, when the user types ahead.
 */

static void ProcessMessage(message)
register struct IntuiMessage *message;
{
    int c;
    static int skip_mouse=0;    /* need to ignore next mouse event on
				 * a window activation */
    struct Window *w = message->IDCMPWindow;

    switch(message->Class) {
    case ACTIVEWINDOW:
	if( alwaysinvent && WIN_INVEN != WIN_ERR &&
			    message->IDCMPWindow ==
			    amii_wins[ WIN_INVEN ]->win )
	{
	    DoMenuScroll( WIN_INVEN, 0 );
	}
	else if( scrollmsg && WIN_MESSAGE != WIN_ERR &&
			    message->IDCMPWindow ==
			    amii_wins[ WIN_MESSAGE ]->win )
	{
	    DoMenuScroll( WIN_MESSAGE, 0 );
	}
	else
	{
	    skip_mouse=1;
	}
	break;

    case MOUSEBUTTONS:
	{
	    if( skip_mouse )
	    {
		skip_mouse=0;
		break;
	    }

	    if( !amii_wins[ WIN_MAP ] || w != amii_wins[ WIN_MAP ]->win )
		break;

	    if( message->Code == SELECTUP )
	    {
#ifdef	VIEWWINDOW
		amii_putstr( WIN_MESSAGE, 0, "done..." );
		w->Flags &= ~REPORTMOUSE;
#endif
	    }
	    else if( message->Code == SELECTDOWN )
	    {
		lastevent.type = WEMOUSE;
		lastevent.un.mouse.x = message->MouseX;
		lastevent.un.mouse.y = message->MouseY;
		    /* With shift equals RUN */
		lastevent.un.mouse.qual = (message->Qualifier &
		  (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) != 0;
#ifdef	VIEWWINDOW
		w->Flags |= REPORTMOUSE;
		amii_putstr( WIN_MESSAGE, 0,
			"drag mouse to see other areas of this level" );
#endif
	    }
	}
	break;

    case MENUPICK:
	{
	    USHORT thismenu;
	    struct MenuItem *item;

	    thismenu = message->Code;
	    while (thismenu != MENUNULL)
	    {
		item = ItemAddress(HackMenu, (ULONG) thismenu);
		if (KbdBuffered < KBDBUFFER)
		    BufferQueueChar(item->Command); /* Unused: No COMMSEQ */
		thismenu = item->NextSelect;
	    }
	}
	break;

    case REFRESHWINDOW:
#ifdef	VIEWWINDOW
	{
	    struct Window *vw, *vbw;
	    if( amii_wins[ WIN_VIEWBOX ] && amii_wins[ WIN_VIEW ] &&
		    w == amii_wins[ WIN_VIEWBOX ]->win )
	    {
		vw = amii_wins[ WIN_VIEW ]->win;
		vbw = amii_wins[ WIN_VIEWBOX ]->win;

		if( vw->LeftEdge != (vbw->LeftEdge+vbw->BorderLeft) ||
		    vw->TopEdge != ( vbw->TopEdge + vbw->BorderTop ) ||
		    vw->Width != (vbw->Width -vbw->BorderLeft - vbw->BorderRight ) ||
		    vw->Height != (vbw->Height - vbw->BorderTop - vbw->BorderBottom ) )
		{
		    MoveWindow( vw, (vbw->LeftEdge+vbw->BorderLeft) - vw->LeftEdge,
			( vbw->TopEdge + vbw->BorderTop ) - vw->TopEdge );
		    SizeWindow( vw,
			( vbw->Width -vbw->BorderLeft -
				vbw->BorderRight ) - vw->Width,
			( vbw->Height - vbw->BorderTop -
				vbw->BorderBottom - vw->Height ) );
		}
	    }
	    else if( amii_wins[ WIN_MESSAGE ] && w == amii_wins[ WIN_MESSAGE ]->win )
	    {
		DoMenuScroll( WIN_MESSAGE, 0 );
	    }
	}
#endif
	break;

    case CLOSEWINDOW:
	if( WIN_INVEN != WIN_ERR && message->IDCMPWindow ==
				amii_wins[ WIN_INVEN ]->win )
	{
	    dismiss_nhwindow( WIN_INVEN );
	}
	break;

    case RAWKEY:
	if (!(message->Code & IECODE_UP_PREFIX)){
	    /* May queue multiple characters
	     * but doesn't do that yet...
	     */
	    if( ( c = ConvertKey(message) ) > 0 )
		BufferQueueChar( c );
        }
        break;

    case MOUSEMOVE:
#ifdef	VIEWWINDOW
	if( w == amii_wins[ WIN_MAP ]->win )
	{
	    int posx, posy, dx, dy;
	    register struct MsgPort *port = w->UserPort;
	    struct amii_WinDesc *cw;

	    posx = message->MouseX;
	    posy = message->MouseY;
	    cursor_on( WIN_MAP );
	    cw = amii_wins[ WIN_MAP ];

	    do {
		if( message->Class == MOUSEBUTTONS ||
					message->Class == INACTIVEWINDOW )
		{
		    w->Flags &= ~REPORTMOUSE;
		    break;
		}
		else if( message->Class == MOUSEMOVE )
		{
		    if( posx != message->MouseX || posy != message->MouseY )
		    {
			dx = message->MouseX - posx;
			dy = message->MouseY - posy;
			dx /= MAPFTWIDTH;
			dy /= MAPFTHEIGHT;
			if( dx != 0 || dy != 0 )
			{
			    posx = message->MouseX;
			    posy = message->MouseY;
			    amii_curs( WIN_MAP,
				(posx - w->BorderLeft)/MAPFTWIDTH+dx,
				(posy - w->BorderTop)/MAPFTHEIGHT+dy );
			    cursor_on( WIN_MAP );
			}
		    }
		}
		ReplyMsg( (struct Message *) message );
		while( !(message = (struct IntuiMessage *)GetMsg( port ) ) )
		    WaitPort( port );
	    } while( message );
	    amii_putstr( WIN_MESSAGE, 0, "done..." );
	    break;
	}
#endif
	/* FALL through for MESSAGE or INVEN windows */
    case GADGETDOWN:
	if( WIN_MESSAGE != WIN_ERR && message->IDCMPWindow ==
			amii_wins[ WIN_MESSAGE ]->win )
	{
	    DoMenuScroll( WIN_MESSAGE, 0 );
	}
	else if( WIN_INVEN != WIN_ERR && message->IDCMPWindow ==
			amii_wins[ WIN_INVEN ]->win )
	{
	    DoMenuScroll( WIN_INVEN, 0 );
	}
	break;

    case NEWSIZE:
	if( WIN_MESSAGE != WIN_ERR && message->IDCMPWindow ==
			amii_wins[ WIN_MESSAGE ]->win )
	{
	    ReDisplayData( WIN_MESSAGE );
	}
	else if( WIN_INVEN != WIN_ERR && message->IDCMPWindow ==
			amii_wins[ WIN_INVEN ]->win )
	{
	    ReDisplayData( WIN_INVEN );
	}
	break;
    }
    ReplyMsg((struct Message *) message);
}

#endif /* AMII_GRAPHICS */
/*
 *  Get all incoming messages and fill up the keyboard buffer,
 *  thus allowing Intuition to (maybe) free up the IntuiMessages.
 *  Return when no more messages left, or keyboard buffer half full.
 *  We need to do this since there is no one-to-one correspondence
 *  between characters and incoming messages.
 */

#if defined(TTY_GRAPHICS) && !defined(AMII_GRAPHICS)
int kbhit(){return 0};
#else
int
kbhit()
{
    int c;
#ifdef TTY_GRAPHICS
		/* a kludge to defuse the mess in allmain.c */
		/* I hope this is the right approach */
    if(windowprocs.win_init_nhwindows==amii_procs.win_init_nhwindows)return 0;
#endif
    c = amikbhit();
    if( c <= 0 )
    	return( 0 );
    return( c );
}
#endif

#ifdef AMII_GRAPHICS

int
amikbhit()
{
    register struct IntuiMessage *message;
    while( KbdBuffered < KBDBUFFER / 2 )
    {
#ifdef AMIFLUSH
	message = (struct IntuiMessage *) GetFMsg(HackPort);
#else
	message = (struct IntuiMessage *) GetMsg(HackPort);
#endif
	if(message)
	{
	    ProcessMessage(message);
	    if( lastevent.type != WEUNK && lastevent.type != WEKEY )
		break;
	}
	else
	    break;
    }
    return ( lastevent.type == WEUNK ) ? KbdBuffered : -1;
}

/*
 *  Get a character from the keyboard buffer, waiting if not available.
 *  Ignore other kinds of events that happen in the mean time.
 */

int WindowGetchar( )
{
    while ((lastevent.type = WEUNK), amikbhit() <= 0) {
	WaitPort(HackPort);
    }
    return BufferGetchar();
}

WETYPE WindowGetevent()
{
    lastevent.type = WEUNK;
    while (amikbhit() == 0)
    {
	WaitPort(HackPort);
    }

    if( KbdBuffered )
    {
	lastevent.type = WEKEY;
	lastevent.un.key = BufferGetchar();
    }
    return( lastevent.type );
}

/*
 *  Clean up everything. But before we do, ask the user to hit return
 *  when there is something that s/he should read.
 */

void amii_cleanup()
{
    register struct IntuiMessage *msg;

    /* Close things up */
    if( HackPort )
    {
	amii_raw_print("");
	amii_getret();
    }

    if (ConsoleIO.io_Device)
	CloseDevice( (struct IORequest *)&ConsoleIO );
    ConsoleIO.io_Device = 0;

    if( ConsoleIO.io_Message.mn_ReplyPort )
	DeletePort( ConsoleIO.io_Message.mn_ReplyPort );
    ConsoleIO.io_Message.mn_ReplyPort = 0;

    /* Strip messages before deleting the port */
    if( HackPort )
    {
	Forbid();
	while (msg = (struct IntuiMessage *) GetMsg(HackPort))
	    ReplyMsg((struct Message *) msg);
	kill_nhwindows( 1 );
	DeletePort( HackPort );
	HackPort = NULL;
	Permit();
    }

    /* Close the screen, under v37 or greater it is a pub screen and there may be
     * visitors, so check close status and wait till everyone is gone.
     */
    if( HackScreen )
    {
#ifdef  INTUI_NEW_LOOK
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	{
	    while( CloseScreen( HackScreen ) == FALSE )
	    {
		struct EasyStruct easy =
		{
		    sizeof( struct EasyStruct ),
		    0,
		    "Nethack Problem",
		    "Can't Close Screen, Close Visiting Windows",
		    "Okay",
		};
		EasyRequest( NULL, &easy, NULL, NULL );
	    }
	}
	else
#endif
	{
	    CloseScreen(HackScreen);
	}
	HackScreen = NULL;
    }

#ifdef HACKFONT
    if (HackFont)
    {
	CloseFont(HackFont);
	HackFont = NULL;
    }

#ifdef	VIEWWINDOW
    if (HackFont4)
    {
	CloseFont(HackFont4);
	HackFont4 = NULL;
    }

    if (HackFont16)
    {
	CloseFont(HackFont16);
	HackFont16 = NULL;
    }
#endif

    if( TextsFont )
    {
	CloseFont( TextsFont );
	TextsFont = NULL;
    }

    if( DiskfontBase )
    {
	CloseLibrary(DiskfontBase);
	DiskfontBase = NULL;
    }
#endif

#ifdef	VIEWWINDOW
    if (LayersBase) {
	CloseLibrary((struct Library *)LayersBase);
	LayersBase = NULL;
    }
#endif

    if (GfxBase) {
	CloseLibrary((struct Library *)GfxBase);
	GfxBase = NULL;
    }

    if (IntuitionBase) {
	CloseLibrary((struct Library *)IntuitionBase);
	IntuitionBase = NULL;
    }

#ifdef	SHAREDLIB
    if (DOSBase) {
	CloseLibrary((struct Library *)DOSBase);
	DOSBase = NULL;
    }
#endif

    ((struct Process *) FindTask(NULL))->pr_WindowPtr = (APTR) pr_WindowPtr;

    Initialized = 0;
}

#ifndef	SHAREDLIB
void Abort(rc)
long rc;
{
    int fault = 1;
#ifdef CHDIR
    extern char orgdir[];
    chdir(orgdir);
#endif
#ifdef AMII_GRAPHICS
    if (Initialized
      && ConsoleDevice
      && windowprocs.win_init_nhwindows==amii_procs.win_init_nhwindows) {
      printf("\n\nAbort with alert code %08lx...\n", rc);
      amii_getret();
    } else
#endif
      printf("\n\nAbort with alert code %08lx...\n",rc);
#if 0
      Alert(rc);              /* this is too severe */
#endif
#ifdef __SASC
#ifdef	INTUI_NEW_LOOK
    {
    	struct EasyStruct es =
    	{
    		sizeof( struct EasyStruct ),
    		0,
    		"NetHack Panic Request",
    		"NetHack is Aborting with code == 0x%08lx",
		"Continue Abort|Return to Program|Clean up and exit",
    	};
    	fault = EasyRequest( NULL, &es, NULL, (long)rc );
    	if( fault == 2 )
    	    return;
    }
#endif
    if( fault == 1 )
    {
/*  __emit(0x4afc);     /* illegal instruction */
    __emit(0x40fc);     /* divide by */
    __emit(0x0000);     /*  #0  */
      /* NOTE: don't move amii_cleanup() above here - */
      /* it is too likely to kill the system     */
      /* before it can get the SnapShot out, if  */
      /* there is something really wrong.    */
    }
#endif
#ifdef AMII_GRAPHICS
    if(windowprocs.win_init_nhwindows==amii_procs.win_init_nhwindows)
      amii_cleanup();
#endif
#undef exit
#ifdef AZTEC_C
    _abort();
#endif
    exit((int) rc);
}

void
CleanUp()
{
	amii_cleanup();
}
#endif

#ifdef AMII_GRAPHICS

#ifdef AMIFLUSH
/* This routine adapted from AmigaMail IV-37 by Michael Sinz */
static struct Message *
GetFMsg(port)
    struct MsgPort *port;
    {
    struct IntuiMessage *msg,*succ,*succ1;

    if(msg=(struct IntuiMessage *)GetMsg(port)){
	if(!flags.amiflush)return((struct Message *)msg);
	if(msg->Class==RAWKEY){
	    Forbid();
	    succ=(struct IntuiMessage *)(port->mp_MsgList.lh_Head);
	    while(succ1=(struct IntuiMessage *)
	      (succ->ExecMessage.mn_Node.ln_Succ)){
		if(succ->Class==RAWKEY){
		    Remove((struct Node *)succ);
		    ReplyMsg((struct Message *)succ);
		}
		succ=succ1;
	    }
	    Permit();
	}
    }
    return((struct Message *)msg);
}
#endif

/*
 * Begin Revamped Text display routines
 *
 * Up until version 3.1, the only method for displaying text on the playing
 * field was by using the console.device.  This was nice for a number of
 * reasons, the most signifigant of which was a lot of the nuts and bolts was
 * done for you via escape sequences interpreted by said device.  This did
 * not come without a price however.  And that price was speed. It has now
 * come to a point where the speed has now been deemed unacceptable.
 *
 * The following series of routines are designed to drop into the current
 * nethack display code, using hooks provided for such a measure. It works
 * on similar principals as the WindowPuts(), buffering I/O internally
 * until either an explicit flush or internal buffering is exceeded, thereby
 * forcing the flush.  The output (or glyphs) does not go to the
 * console.device, however.  It is driven directly to the rasterport of the
 * nethack window via the low-level Text() calls, increasing the speed by
 * a very signifigant factor.
 */
/*
 * Routine to simply flush whatever is buffered
 */
void
flush_glyph_buffer( w )
    struct Window *w;
{
    short i, x, y;
#ifdef	VIEWWINDOW
    struct Window *vw = amii_wins[ WIN_VIEW ]->win;
    register struct RastPort *vrp = vw->RPort;
#endif
    register struct RastPort *rp = w->RPort;

    /* If nothing is buffered, return before we do anything */
    if(glyph_node_index == 0)
	return;

    cursor_off( WIN_MAP );
    start_glyphout( WIN_MAP );
#ifdef	VIEWWINDOW
    cursor_off( WIN_VIEW );
    start_glyphout( WIN_VIEW );
#endif

    /* Set up the drawing mode */
    SetDrMd( rp, JAM2);
#ifdef	VIEWWINDOW
    SetDrMd( vrp, JAM2);
#endif

    /* Go ahead and start dumping the stuff */
    for(i=0; i<glyph_node_index; ++i) {
	/* These coordinate calculations must be synced with the
	 * code in amii_curs() in winami.c.  curs_on_u() calls amii_curs()
	 * to draw the cursor on top of the player
	 */
	y = w->BorderTop + (g_nodes[i].y-1) * rp->TxHeight +
	    rp->TxBaseline + 1;
	x = g_nodes[i].x * rp->TxWidth + w->BorderLeft;

	/* Move pens to correct location */
	Move( rp, (long)x, (long)y);

	/* Setup the colors */
	SetAPen( rp, (long)g_nodes[i].fg_color);
	SetBPen( rp, (long)g_nodes[i].bg_color);

	/* Do it */
	Text( rp, g_nodes[i].buffer, g_nodes[i].len);

#ifdef	VIEWWINDOW
	y = vw->BorderTop + (g_nodes[i].y-1) * vrp->TxHeight + vrp->TxBaseline + 1;
	x = g_nodes[i].x * vrp->TxWidth + vw->BorderLeft;

	/* Move pens to correct location */
	Move( vrp, (long)x, (long)y);

	/* Setup the colors */
	SetAPen( vrp, (long)g_nodes[i].fg_color);
	SetBPen( vrp, (long)g_nodes[i].bg_color);

	/* Do it */
	Text( vrp, g_nodes[i].buffer, g_nodes[i].len);
#endif
    }

    amii_end_glyphout( WIN_MAP );
#ifdef	VIEWWINDOW
    amii_end_glyphout( WIN_VIEW );
#endif
    /* Clean up */
    glyph_node_index = glyph_buffer_index = 0;
}

/*
 * Glyph buffering routine.  Called instead of WindowPuts().
 */
void
amiga_print_glyph(window,color_index, glyph)
    winid window;
    int color_index, glyph;
{
    int fg_color, bg_color;
    struct amii_WinDesc *cw;
    struct Window *w;
    int curx;
    int cury;

    if( ( cw=amii_wins[window] ) == (struct amii_WinDesc *)NULL )
	panic("bad winid in amiga_print_glyph: %d", window );

    w = cw->win;
    curx=cw->curx;
    cury=cw->cury;

#ifdef TEXTCOLOR
    fg_color = foreg[color_index];
    bg_color = backg[color_index];
#else
    fg_color = 1;
    bg_color = 0;
#endif /* TEXTCOLOR */

    /* See if we have enough character buffer space... */
    if(glyph_buffer_index  >= GLYPH_BUFFER_SIZE)
	flush_glyph_buffer( w );

    /*
     * See if we can append it to the current active node of glyph buffer. It
     * must satisfy the following conditions:
     *
     *    * background colors are the same, AND
     *    * foreground colors are the same, AND
     *    * they are precisely side by side
     */
    if((glyph_buffer_index != 0) &&
       (fg_color == g_nodes[glyph_node_index-1].fg_color) &&
       (bg_color == g_nodes[glyph_node_index-1].bg_color) &&
       (g_nodes[glyph_node_index-1].x+
	g_nodes[glyph_node_index-1].len == curx) &&
       (g_nodes[glyph_node_index-1].y == cury)) {
	/*
	 * Add it to the end of the buffer
	 */
	glyph_buffer[glyph_buffer_index++] = glyph;
	g_nodes[glyph_node_index-1].len ++;
     } else {
	/* See if we're out of glyph nodes */
	if(glyph_node_index >= NUMBER_GLYPH_NODES)
	    flush_glyph_buffer( w );
	g_nodes[glyph_node_index].len = 1;
	g_nodes[glyph_node_index].x = curx;
	g_nodes[glyph_node_index].y = cury;
	g_nodes[glyph_node_index].fg_color = fg_color;
	g_nodes[glyph_node_index].bg_color = bg_color;
	g_nodes[glyph_node_index].buffer = &glyph_buffer[glyph_buffer_index];
	glyph_buffer[glyph_buffer_index] = glyph;
	++glyph_buffer_index;
	++glyph_node_index;
    }
}

/*
 * Define some variables which will be used to save context when toggling
 * back and forth between low level text and console I/O.
 */
static long xsave, ysave, modesave, apensave, bpensave;
static int usecolor;

/*
 * The function is called before any glyphs are driven to the screen.  It
 * removes the cursor, saves internal state of the window, then returns.
 */

void
start_glyphout(window)
    winid window;
{
    struct amii_WinDesc *cw;
    struct Window *w;

    if( ( cw=amii_wins[window] ) == (struct amii_WinDesc *)NULL )
	panic( "bad winid %d in start_glyphout()", window );

    if( cw->wflags & FLMAP_INGLYPH )
	return;

    if( !(w = cw->win ) )
	panic( "bad winid %d, no window ptr set", window );

    /*
     * Save the context of the window
     */
    xsave = w->RPort->cp_x;
    ysave = w->RPort->cp_y;
    modesave = w->RPort->DrawMode;
    apensave = w->RPort->FgPen;
    bpensave = w->RPort->BgPen;

    /*
     * Set the mode, and be done with it
     */
    usecolor = flags.use_color;
    flags.use_color = FALSE;
    cw->wflags |= FLMAP_INGLYPH;
}

/*
 * General cleanup routine -- flushes and restores cursor
 */
void
amii_end_glyphout(window)
    winid window;
{
    struct amii_WinDesc *cw;
    struct Window *w;

    if( ( cw = amii_wins[ window ] ) == (struct amii_WinDesc *)NULL )
	panic("bad window id %d in amii_end_glyphout()", window );

    if( ( cw->wflags & FLMAP_INGLYPH ) == 0 )
	return;
    cw->wflags &= ~(FLMAP_INGLYPH);

    if( !(w = cw->win ) )
	panic( "bad winid %d, no window ptr set", window );

    /*
     * Clean up whatever is left in the buffer
     */
    flags.use_color = usecolor;

    /*
     * Reset internal data structs
     */
    SetAPen(w->RPort, apensave);
    SetBPen(w->RPort, bpensave);
    SetDrMd(w->RPort, modesave);

    Move(w->RPort, xsave, ysave);
}

struct NewWindow *
DupNewWindow( win )
    struct NewWindow *win;
{
    struct NewWindow *nwin;
    struct Gadget *ngd, *gd, *pgd = NULL;
    struct PropInfo *pip;
    struct StringInfo *sip;

    /* Copy the (Ext)NewWindow structure */

    nwin = (struct NewWindow *)alloc( sizeof( struct NewWindow ) );
    *nwin = *win;

    /* Now do the gadget list */

    nwin->FirstGadget = NULL;
    for( gd = win->FirstGadget; gd; gd = gd->NextGadget )
    {
	ngd = (struct Gadget *)alloc( sizeof( struct Gadget ) );
	*ngd = *gd;
	if( gd->GadgetType == STRGADGET )
	{
	    sip = (struct StringInfo *)alloc( sizeof( struct StringInfo ) );
	    *sip = *((struct StringInfo *)gd->SpecialInfo);
	    sip->Buffer = (UBYTE *) alloc( sip->MaxChars );
	    *sip->Buffer = 0;
	    ngd->SpecialInfo = (APTR)sip;
	}
	else if( gd->GadgetType == PROPGADGET )
	{
	    pip = (struct PropInfo *)alloc( sizeof( struct PropInfo ) );
	    *pip = *((struct PropInfo *)gd->SpecialInfo);
	    ngd->SpecialInfo = (APTR)pip;
	}
	if( pgd )
	    pgd->NextGadget = ngd;
	else
	    nwin->FirstGadget = ngd;
	pgd = ngd;
	ngd->NextGadget = NULL;
    }
    return( nwin );
}

void
FreeNewWindow( win )
    struct NewWindow *win;
{
    register struct Gadget *gd, *pgd;
    register struct StringInfo *sip;

    for( gd = win->FirstGadget; gd; gd = pgd )
    {
	pgd = gd->NextGadget;
	if( gd->GadgetType == STRGADGET )
	{
	    sip = (struct StringInfo *)gd->SpecialInfo;
	    free( sip->Buffer );
	    free( sip );
	}
	else if( gd->GadgetType == PROPGADGET )

	{
	    free( (struct PropInfo *)gd->SpecialInfo );
	}
	free( gd );
    }
    free( win );
}

void
bell()
{
    if (flags.silent) return;
    DisplayBeep(NULL);
}

void
amii_delay_output()
{
    /* delay 50 ms */
    Delay(2L);
}

void
amii_number_pad(state)
int state;
{
}
#endif  /* AMII_GRAPHICS */

#ifndef	SHAREDLIB
void
amiv_loadlib( void )
{
}

void
amii_loadlib( void )
{
}

/* fatal error */
/*VARARGS1*/
void error VA_DECL(const char *, s)
    VA_START(s);
    VA_INIT(s, char *);

    putchar('\n');
    vprintf(s, VA_ARGS);
    putchar('\n');

    VA_END();
    Abort(0L);
}
#endif
