/*    SCCS Id: @(#)amiwind.c     3.1    93/01/08
/*    Copyright (c) Olaf Seibert (KosmoSoft), 1989, 1992	  */
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland 1993	  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Here is some very Amiga specific stuff, dealing with
 *  screens, windows, menus, and input via IntuiMessages.
 */

#include "hack.h"
#include "winami.h"

/* Have to undef CLOSE as display.h and intuition.h both use it */
#undef CLOSE

#include <exec/types.h>
#include <exec/alerts.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dosextens.h>

#ifdef __SASC
# undef COUNT

# include <dos.h>       /* for __emit */
# include <string.h>
# include <proto/dos.h>
# include <proto/exec.h>

/* kludge - see amirip for why */
# undef red
# undef green
# undef blue
# undef index
# include <proto/graphics.h>

# include <proto/intuition.h>
# include <proto/diskfont.h>
# include <proto/console.h>
#endif

#undef  NULL
#define NULL    0L

#include "Amiga:amimenu.c"

/*  First, external declarations... */

struct Library *ConsoleDevice;

#ifdef AZTEC_50
# include <functions.h>
#endif

#ifdef  INTUI_NEW_LOOK
#define NewWindow ExtNewWindow
#endif

#include "Amiga:winami.p"
#include "Amiga:amiwind.p"
#include "Amiga:amidos.p"

static int BufferGetchar(void);
static void ProcessMessage( register struct IntuiMessage *message );

#ifdef AMIFLUSH
static struct Message *FDECL(GetFMsg,(struct MsgPort *));
#endif

/*  Now our own variables */

struct IntuitionBase *IntuitionBase;
struct Screen *HackScreen;
struct Window *pr_WindowPtr;
struct MsgPort *HackPort;
struct IOStdReq ConsoleIO;
char Initialized = 0;
WEVENT lastevent;

#ifdef HACKFONT
struct GfxBase *GfxBase;
struct Library *DiskfontBase;
#endif

extern struct Library *ConsoleDevice;

#define CSI     '\x9b'
#define NO_CHAR     -1
#define RAWHELP     0x5F    /* Rawkey code of the HELP key */

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

int foreg[16] = { 0, 7, 4, 2, 6, 5, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
int backg[16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 4, 1, 6, 5, 3, 1 };
#endif

#ifdef HACKFONT

struct TextFont *HackFont;
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
    TOPAZ_EIGHTY, FS_NORMAL, FPF_DISKFONT | FPF_ROMFONT
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
    register struct IntuiMessage *msg, *nxt;

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
    switch(message->Class) {
    case ACTIVEWINDOW:
	skip_mouse=1;break;
    case MOUSEBUTTONS:
	{
	    if(skip_mouse){
		skip_mouse=0;
		break;
	    }
	    if( message->Code == SELECTDOWN ){
		lastevent.type = WEMOUSE;
		lastevent.u.mouse.x = message->MouseX;
		lastevent.u.mouse.y = message->MouseY;
		    /* With shift equals RUN */
		lastevent.u.mouse.qual = (message->Qualifier &
		  (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) != 0;
	    }
	}
	break;

    case MENUPICK:
	{
	    USHORT thismenu;
	    struct MenuItem *item;

	    thismenu = message->Code;
	    while (thismenu != MENUNULL) {
		item = ItemAddress(HackMenu, (ULONG) thismenu);
		if (KbdBuffered < KBDBUFFER)
		    BufferQueueChar(item->Command); /* Unused: No COMMSEQ */
		thismenu = item->NextSelect;
	    }
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
    }
    ReplyMsg((struct Message *) message);
}

/*
 *  Get all incoming messages and fill up the keyboard buffer,
 *  thus allowing Intuition to (maybe) free up the IntuiMessages.
 *  Return when no more messages left, or keyboard buffer half full.
 *  We need to do this since there is no one-to-one correspondence
 *  between characters and incoming messages.
 */

int
kbhit()
{
    int c;
    c = amikbhit();
    if( c <= 0 )
    	return( 0 );
    return( c );
}

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
	lastevent.u.key = BufferGetchar();
    }
    return( lastevent.type );
}

/*
 *  Clean up everything. But before we do, ask the user to hit return
 *  when there is something that s/he should read.
 */

void CleanUp()
{
    register struct IntuiMessage *msg;

    /* Finish closing things up */

    amii_raw_print("");
    amii_getret();

    ((struct Process *) FindTask(NULL))->pr_WindowPtr = (APTR) pr_WindowPtr;
    if (ConsoleIO.io_Device)
	CloseDevice( (struct IORequest *)&ConsoleIO );
    ConsoleIO.io_Device = 0;

    if( ConsoleIO.io_Message.mn_ReplyPort )
	DeletePort( ConsoleIO.io_Message.mn_ReplyPort );
    ConsoleIO.io_Message.mn_ReplyPort = 0;

    if (HackPort) {
	Forbid();
	while (msg = (struct IntuiMessage *) GetMsg(HackPort))
	    ReplyMsg((struct Message *) msg);
	kill_nhwindows( 1 );
	DeletePort( HackPort );
	HackPort = NULL;
	Permit();
    }

    if (HackScreen) {
#ifdef  INTUI_NEW_LOOK
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	{
	    while( CloseScreen(HackScreen) == FALSE )
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
#else
	CloseScreen(HackScreen);
#endif
	HackScreen = NULL;
    }

#ifdef HACKFONT

    if (HackFont)
    {
	CloseFont(HackFont);
	HackFont = NULL;
    }

    if( DiskfontBase )
    {
	CloseLibrary(DiskfontBase);
	DiskfontBase = NULL;
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

    Initialized = 0;
}

void Abort(rc)
long rc;
{
#ifdef CHDIR
    extern char orgdir[];
    chdir(orgdir);
#endif
    if (Initialized && ConsoleDevice) {
	printf("\n\nAbort with alert code %08lx...\n", rc);
	amii_getret();
    } else
	Alert(rc);
#ifdef __SASC
    {
/*  __emit(0x4afc);     /* illegal instruction */
    __emit(0x40fc);     /* divide by */
    __emit(0x0000);     /*  #0  */
	/* NOTE: don't move CleanUp() above here - */
	/* it is too likely to kill the system     */
	/* before it can get the SnapShot out, if  */
	/* there is something really wrong.    */
    }
#endif
    CleanUp();
#undef exit
#ifdef AZTEC_C
    _abort();
#endif
    exit((int) rc);
}

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

    /* If nothing is buffered, return before we do anything */
    if(glyph_node_index == 0)
	return;

    cursor_off( WIN_MAP );
    start_glyphout( WIN_MAP );
    /* Set up the drawing mode */
    SetDrMd( w->RPort, JAM2);

    /* Go ahead and start dumping the stuff */
    for(i=0; i<glyph_node_index; ++i) {
	/* These coordinate calculations must be synced with the
	 * code in curs() in winami.c.  curs_on_u() calls curs()
	 * to draw the cursor on top of the player
	 */
	y = w->BorderTop + (g_nodes[i].y-1) * w->RPort->TxHeight +
	    w->RPort->TxBaseline + 1;
	x = g_nodes[i].x * w->RPort->TxWidth + w->BorderLeft;

	/* Move pens to correct location */
	Move(w->RPort, (long)x, (long)y);

	/* Setup the colors */
	SetAPen(w->RPort, (long)g_nodes[i].fg_color);
	SetBPen(w->RPort, (long)g_nodes[i].bg_color);

	/* Do it */
	Text(w->RPort, g_nodes[i].buffer, g_nodes[i].len);
    }

    end_glyphout( WIN_MAP );
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
    struct WinDesc *cw;
    struct Window *w;
    int curx;
    int cury;

    if( ( cw=wins[window] ) == (struct WinDesc *)NULL )
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
    struct WinDesc *cw;
    struct Window *w;

    if( ( cw=wins[window] ) == (struct WinDesc *)NULL )
	panic( "bad winid %d in start_glyphout()", window );

    if( cw->flags & FLMAP_INGLYPH )
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
    cw->flags |= FLMAP_INGLYPH;
}

/*
 * General cleanup routine -- flushes and restores cursor
 */
void
end_glyphout(window)
    winid window;
{
    struct WinDesc *cw;
    struct Window *w;

    if( ( cw = wins[ window ] ) == (struct WinDesc *)NULL )
	panic("bad window id %d in end_glyphout()", window );

    if( ( cw->flags & FLMAP_INGLYPH ) == 0 )
	return;
    cw->flags &= ~(FLMAP_INGLYPH);

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
