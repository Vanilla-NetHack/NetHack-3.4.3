/*
 *  amiwind.c	(C) Copyright 1989 by Olaf Seibert (KosmoSoft)
 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Here is some very Amiga specific stuff, dealing with
 *  screens, windows, menus, and input via IntuiMessages.
 */

#include "hack.h"

#undef TRUE
#undef FALSE
#undef COUNT
#undef NULL

#include <exec/types.h>
#include <exec/alerts.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <intuition/intuition.h>
#include <libraries/dosextens.h>

#ifdef LATTICE
#include <dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/diskfont.h>
#include <proto/console.h>
#endif

#include "Amiga:amimenu.c"

/*
 * Versions we need of various libraries.  We can't use LIBRARY_VERSION
 * as defined in <exec/types.h> because some of the libraries we need
 * don't have that version number in the 1.2 ROM.
 */

#define INTUITION_VERSION 33L
#define GRAPHICS_VERSION  33L
#define DISKFONT_VERSION  34L
#define ICON_VERSION	  34L

/*  First, external declarations... */

extern struct Library *IconBase;
struct Library *ConsoleDevice;

#ifdef AZTEC_C
void FDECL(Alert, (long, char *));
void NDECL(Forbid);
void NDECL(Permit);
struct Process *FDECL(FindTask, (char *));
struct Library *FDECL(OpenLibrary, (char *, long));
void FDECL(CloseLibrary, (struct Library *));
struct Message *FDECL(GetMsg, (struct MsgPort *));
void FDECL(ReplyMsg, (struct Message *));
long FDECL(OpenDevice, (char *, long, struct IORequest *, long));
void FDECL(CloseDevice, (struct IORequest *));
long FDECL(DoIO, (struct IORequest *));
struct TextFont *FDECL(OpenDiskFont, (struct TextAttr *));
struct TextFont *FDECL(OpenFont, (struct TextAttr *));
void FDECL(CloseFont, (struct TextFont *));
void FDECL(LoadRGB4, (struct ViewPort *, unsigned short *, long));
long FDECL(SetFont, (struct RastPort *, struct TextFont*));
struct MsgPort *FDECL(CreatePort, (char *, long));
void FDECL(DeletePort, (struct MsgPort *));
struct Screen *FDECL(OpenScreen, (struct NewScreen *));
struct Window *FDECL(OpenWindow, (struct NewWindow *));
void FDECL(CloseWindow, (struct Window *));
void FDECL(SetMenuStrip, (struct Window *, struct Menu *));
void FDECL(ClearMenuStrip, (struct Window *));
struct MenuItem *FDECL(ItemAddress, (struct Menu *, long));
long FDECL(RawKeyConvert, (struct InputEvent *, char *, long, struct KeyMap *));
#endif

static int NDECL(BufferGetchar);
static void FDECL(ConvertKey, (register struct IntuiMessage *));
static void FDECL(ProcessMessage, (register struct IntuiMessage *));
#ifdef AMIFLUSH
static struct Message *FDECL(GetFMsg,(struct MsgPort *));
#endif
void NDECL(Initialize);

/*  Now our own variables */

struct Library *IntuitionBase;
struct Screen *HackScreen;
struct Window *HackWindow;
struct Window *pr_WindowPtr;
struct IOStdReq ConsoleIO;
char Initialized = 0;

#ifdef HACKFONT
struct Library *GfxBase;
struct Library *DiskfontBase;
#endif

extern struct Library *ConsoleDevice;

#define CSI	    '\x9b'
#define NO_CHAR     -1
#define RAWHELP     0x5F	/* Rawkey code of the HELP key */

/*
 *  It is assumed that all multiple-character outputs are
 *  at most CONBUFFER characters each.
 */

#define CONBUFFER   512
static char ConsoleBuffer[CONBUFFER];
static unsigned short Buffered;

#define KBDBUFFER   10
static unsigned char KbdBuffer[KBDBUFFER];
static unsigned char KbdBuffered;

#define BufferQueueChar(ch) (KbdBuffer[KbdBuffered++] = ch)

/*
 *  It seems Intuition won't OpenDiskFont our diskFont, so we get the
 *  closest match, which is of course topaz/8. (and if not, it is still
 *  an 8-pixel font, so everything still looks ok)
 */

#ifdef HACKFONT

struct TextFont *HackFont;
UBYTE FontName[] = "NetHack:hack.font";
#define 	    SIZEOF_DISKNAME	8

#endif

struct TextAttr Hack80 = {
#ifdef HACKFONT
    &FontName[SIZEOF_DISKNAME],
#else
    (UBYTE *) "topaz.font",
#endif
    TOPAZ_EIGHTY, FS_NORMAL, FPF_DISKFONT | FPF_ROMFONT
};

#define BARHEIGHT	11
#define WINDOWHEIGHT	192
#define WIDTH		640

#ifdef TEXTCOLOR
#define DEPTH       3
static unsigned short palette[] = {
	0x0000,	/* Black   */
	0x0DDD, /* White   */
    	0x0C75, /* Brown   */
	0x0B08,	/* Cyan    */
	0x00B0,	/* Green   */
	0x0F08,	/* Magenta */
	0x055F,	/* Blue    */
	0x0F00,	/* Red     */
};
#else
#define DEPTH		2
#endif

struct NewScreen NewHackScreen = {
    0, 0, WIDTH, BARHEIGHT + WINDOWHEIGHT, DEPTH,
    0, 1,     /* DetailPen, BlockPen */
    HIRES,
    CUSTOMSCREEN,
    &Hack80,  /* Font */
    (UBYTE *) " NetHack 3.0 - Ported by Olaf Seibert (KosmoSoft)",
    NULL,     /* Gadgets */
    NULL,     /* CustomBitmap */
};

struct NewWindow NewHackWindow = {
    /* left, top, width, height, detailpen, blockpen */
    0, BARHEIGHT, WIDTH, WINDOWHEIGHT, -1, -1,
    RAWKEY | MENUPICK
#ifdef MAIL
		      | DISKINSERTED
#endif
    , BORDERLESS | BACKDROP | ACTIVATE,
    NULL, NULL, NULL,
    NULL, NULL, -1,-1,-1,-1, CUSTOMSCREEN
};

static int BufferGetchar()
{
    register unsigned char *from, *to;
    register int c;
    register short i;

    if (KbdBuffered) {
	c = KbdBuffer[0];
	KbdBuffered--;
	to = KbdBuffer;
	from = to + 1;
	/* Move the remaining characters */
	for (i = KbdBuffered; i > 0; i--) {
	    *to++ = *from++;
	}
	return c;
    }

    return NO_CHAR;
}

/*
 *  This should remind you remotely of DeadKeyConvert,
 *  but we are cheating a bit.
 *  We want complete control over the numeric keypad, and no
 *  dead keys... (they are assumed to be on Alted keys)
 *  Also assumed is that the IntuiMessage is of type RAWKEY.
 *  For some reason, IECODE_UP_PREFIX events seem to be lost when they
 *  occur while our console window is inactive. This is particulary
 *  troublesome with qualifier keys... Is this because I never
 *  RawKeyConvert those events???
 */

static void ConvertKey(message)
register struct IntuiMessage *message;
{
    static struct InputEvent theEvent;
    static char       numpad[] = "bjnh.lyku";
    static char  ctrl_numpad[] = "\x02\x0A\x0E\x08.\x0C\x19\x0B\x15";
    static char shift_numpad[] = "BJNH.LYKU";

    unsigned char buffer[1];
    register char length;
    register ULONG qualifier = message->Qualifier;
    char numeric_pad, shift, control, alt;

    control = (qualifier &  IEQUALIFIER_CONTROL) != 0;
    shift   = (qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)) != 0;
    alt     = (qualifier & (IEQUALIFIER_LALT   | IEQUALIFIER_RALT  )) != 0;
    /* Allow ALT to function as a META key ... */
    qualifier &= ~(IEQUALIFIER_LALT | IEQUALIFIER_RALT);
    numeric_pad = (qualifier & IEQUALIFIER_NUMERICPAD) != 0;

    /*
     *	Shortcut for HELP and arrow keys. I suppose this is allowed.
     *	The defines are in intuition/intuition.h, and the keys don't
     *	serve 'text' input, normally. Also, parsing their escape
     *	sequences is such a mess...
     */

    switch (message->Code) {
    case RAWHELP:
	length = '?';
	goto no_arrow;
    case CURSORLEFT:
	length = 'h'; goto arrow;
    case CURSORDOWN:
	length = 'j'; goto arrow;
    case CURSORUP:
	length = 'k'; goto arrow;
    case CURSORRIGHT:
	length = 'l';
    arrow:
	if (!flags.num_pad)	/* Give digits if set, letters otherwise */
	    goto wasarrow;
    no_arrow:
	BufferQueueChar(length);
	return;
    }

#ifdef BETA
    if (!ConsoleDevice) { /* Should never happen */
	Abort(AG_IOError | AO_ConsoleDev);
	return;
    }
#endif

    theEvent.ie_Class = IECLASS_RAWKEY;
    theEvent.ie_Code = message->Code;
    theEvent.ie_Qualifier = numeric_pad ? IEQUALIFIER_NUMERICPAD :
					  qualifier;
    theEvent.ie_EventAddress = (APTR) *(message->IAddress);

    length = RawKeyConvert(&theEvent, buffer, (long) sizeof(buffer), NULL);

    if (length == 1) {   /* Plain ASCII character */
	length = buffer[0];
	if (!flags.num_pad && numeric_pad && length >= '1' && length <= '9') {
wasarrow:
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
	BufferQueueChar(length);
    } /* else shift, ctrl, alt, amiga, F-key, shift-tab, etc */
}

/*
 *  Process an incoming IntuiMessage.
 *  It would certainly look nicer if this could be done using a
 *  PA_SOFTINT message port, but we cannot call RawKeyConvert()
 *  during a software interrupt.
 *  Anyway, kbhit() is called often enough, and usually gets
 *  ahead of input demands, when the user types ahead.
 */

static void ProcessMessage(message)
register struct IntuiMessage *message;
{
    switch(message->Class) {
    case MENUPICK:
	{
	    USHORT thismenu;
	    struct MenuItem *item = NULL;

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
	if (!(message->Code & IECODE_UP_PREFIX))
	    ConvertKey(message);    /* May queue multiple characters */
	break;			    /* but doesn't do that yet       */
#ifdef MAIL
    case DISKINSERTED:
	{
	    extern int mustgetmail;

	    if (mustgetmail < 0)
		mustgetmail = rn1(100,50);
	}
#endif
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

int kbhit()
{
    register struct IntuiMessage *message;

    while( (KbdBuffered < KBDBUFFER / 2) &&
#ifdef AMIFLUSH
	    (message = (struct IntuiMessage *) GetFMsg(HackWindow->UserPort)))
#else
	    (message = (struct IntuiMessage *) GetMsg(HackWindow->UserPort)) )
#endif
	ProcessMessage(message);

    return (int) KbdBuffered;
}

/*
 *  Get a character from the keyboard buffer, waiting if
 *  not available.
 */

int WindowGetchar()
{
    while (!kbhit()) {
	WaitPort(HackWindow->UserPort);
    }
    return BufferGetchar();
}

/*
 *  Flush the output waiting in the console output buffer.
 */

void WindowFlush()
{
#ifdef BETA
    if (!ConsoleDevice) { /* Should never happen */
	Abort(AG_IOError | AO_ConsoleDev);
	return;
    }
#endif

    if (Buffered) {
	ConsoleIO.io_Command = CMD_WRITE;
	ConsoleIO.io_Data = (APTR)ConsoleBuffer;
	ConsoleIO.io_Length = Buffered;
	DoIO((struct IORequest *) &ConsoleIO);
	Buffered = 0;
    }
}

/*
 *  Queue a single character for output to the console screen.
 */

void WindowPutchar(c)
char c;
{
    if (Buffered >= CONBUFFER)
	WindowFlush();

    ConsoleBuffer[Buffered++] = c;
}

/*
 *  Queue an entire string for output to the console screen,
 *  flushing the existing characters first, if necessary.
 *  Do not append a newline.
 */

void WindowFPuts(string)
char *string;
{
    register int len = strlen(string);

    if (len + Buffered >= CONBUFFER)
	WindowFlush();

    strcpy(ConsoleBuffer + Buffered, string);
    Buffered += len;
}

/*
 *  Queue an entire string for output to the console screen,
 *  flushing the existing characters first, if necessary.
 *  Append a newline.
 */

void WindowPuts(string)
char *string;
{
    WindowFPuts(string);
    WindowPutchar('\n');
}

/*
 *  Queue a formatted string for output to the console screen,
 *  flushing the existing characters first, if necessary.
 */

/*VARARGS1*/
#if defined(USE_STDARG) || defined(USE_VARARGS)
void
WindowPrintf VA_DECL(char *, fmt)
    VA_START(fmt);
    VA_INIT(fmt, char *);
    WindowFlush();  /* Don't know if all will fit */
    vsprintf(ConsoleBuffer, fmt, VA_ARGS);
    ConsoleIO.io_Command = CMD_WRITE;
    ConsoleIO.io_Data = (APTR)ConsoleBuffer;
    ConsoleIO.io_Length = -1;
    DoIO((struct IORequest *) &ConsoleIO);
    VA_END();
}
#else
void WindowPrintf(fmt, args, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
char *fmt;
long args, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9;
{
# ifdef AZTEC_36 	/* Efficient but not portable */
    format(WindowPutchar, fmt, &args);
#else
    WindowFlush();  /* Don't know if all will fit */
    sprintf(ConsoleBuffer, fmt, args, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    ConsoleIO.io_Command = CMD_WRITE;
    ConsoleIO.io_Data = (APTR)ConsoleBuffer;
    ConsoleIO.io_Length = -1;
    DoIO((struct IORequest *) &ConsoleIO);
#endif
}
#endif

/*
 *  Clean up everything. But before we do, ask the user to hit return
 *  when there is something that s/he should read.
 */

void CleanUp()
{
    /* Clean up resources */
    if (ConsoleIO.io_Device) {
	register struct ConUnit *cu;

	cu = (struct ConUnit *)ConsoleIO.io_Unit;
	if (cu->cu_XCCP != 1 || cu->cu_YCCP != 1)
	    getret();

	CloseDevice((struct IORequest *) &ConsoleIO);
	ConsoleDevice = NULL;
    }
    if (ConsoleIO.io_Message.mn_ReplyPort)
	DeletePort(ConsoleIO.io_Message.mn_ReplyPort);
    if (HackWindow) {
	register struct IntuiMessage *msg;

	((struct Process *) FindTask(NULL))->pr_WindowPtr = (APTR) pr_WindowPtr;
	ClearMenuStrip(HackWindow);
	Forbid();
	while (msg = (struct IntuiMessage *) GetMsg(HackWindow->UserPort))
	    ReplyMsg((struct Message *) msg);
	CloseWindow(HackWindow);
	Permit();
	HackWindow = NULL;
    }
    if (HackScreen) {
	CloseScreen(HackScreen);
	HackScreen = NULL;
    }
    if (IconBase) {
	CloseLibrary(IconBase);
	IconBase = NULL;
    }
#ifdef HACKFONT
    if (HackFont) {
	CloseFont(HackFont);
	HackFont = NULL;
    }
    if (DiskfontBase) {
	CloseLibrary(DiskfontBase);
	DiskfontBase = NULL;
    }
    if (GfxBase) {
	CloseLibrary(GfxBase);
	GfxBase = NULL;
    }
#endif
    if (IntuitionBase) {
	CloseLibrary(IntuitionBase);
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
	getret();
    } else
	Alert(rc, 0L);
#ifdef LATTICE
	{
/*	__emit(0x4afc);		/* illegal instruction */
	__emit(0x40fc);		/* divide by */
	__emit(0x0000);		/*  #0	*/
		/* NOTE: don't move CleanUp() above here - */
		/* it is too likely to kill the system     */
		/* before it can get the SnapShot out, if  */
		/* there is something really wrong.	   */
__builtin_printf("abort botch");				/* (KL)TEMP */
	}
#endif
    CleanUp();
#undef exit
#ifdef AZTEC_C
    _abort();
#endif
    exit((int) rc);
}

/*
 *  Open everything we need.
 */

void Initialize()
{
    if (Initialized)
	return;

    if ( (IntuitionBase = OpenLibrary("intuition.library", INTUITION_VERSION))
	  == NULL)
	Abort(AG_OpenLib | AO_Intuition);

#ifdef HACKFONT

    if ( (GfxBase = OpenLibrary("graphics.library", GRAPHICS_VERSION)) == NULL)
	Abort(AG_OpenLib | AO_GraphicsLib);

    /*
     *	Force our own font to be loaded, if possible.
     *	If we can open diskfont.library, but not our font, we can close
     *	the diskfont.library again since it just wastes memory.
     *	Even if we can open the font, we don't need the diskfont.library
     *	anymore, since CloseFont is a graphics.library function.
     */

    if ((HackFont = OpenFont(&Hack80)) == NULL) {
	if (DiskfontBase = OpenLibrary("diskfont.library", DISKFONT_VERSION)) {
	    Hack80.ta_Name -= SIZEOF_DISKNAME;
	    HackFont = OpenDiskFont(&Hack80);
	    Hack80.ta_Name += SIZEOF_DISKNAME;
	    CloseLibrary(DiskfontBase);
	    DiskfontBase = NULL;
	}
    }
#endif

    /* if ( (IconBase = OpenLibrary("icon.library", ICON_VERSION)) == NULL)
	Abort(AG_OpenLib | AO_IconLib); */

    /*
     *	Now Intuition is supposed to use our HackFont for the screen,
     *	since we have a corresponding TextAttr, but it *doesn't*.
     *	So, we need to do a SetFont() a bit later on.
     */
    if ( (HackScreen = OpenScreen(&NewHackScreen)) == NULL)
	Abort(AN_OpenScreen & ~AT_DeadEnd);

#ifdef TEXTCOLOR
    LoadRGB4(&HackScreen->ViewPort, palette, 8L);
#endif

    NewHackWindow.Screen = HackScreen;

    if ( (HackWindow = OpenWindow(&NewHackWindow)) == NULL)
	Abort(AN_OpenWindow & ~AT_DeadEnd);

    SetMenuStrip(HackWindow, HackMenu);
    {
	register struct Process *myProcess = (struct Process *) FindTask(NULL);
	pr_WindowPtr = (struct Window *)myProcess->pr_WindowPtr;
	myProcess->pr_WindowPtr = (APTR) HackWindow;
    }
#ifdef HACKFONT
    if (HackFont)
	SetFont(HackWindow->RPort, HackFont);
#endif

    ConsoleIO.io_Data = (APTR) HackWindow;
    ConsoleIO.io_Length = sizeof(*HackWindow);
    ConsoleIO.io_Message.mn_ReplyPort = CreatePort(NULL, 0L);
    if (OpenDevice("console.device", 0L, (struct IORequest *) &ConsoleIO, 0L) != 0)
	Abort(AG_OpenDev | AO_ConsoleDev);

    ConsoleDevice = (struct Library *) ConsoleIO.io_Device;

    Buffered = 0;
    KbdBuffered = 0;

    /* set CRMOD on */
    WindowFPuts("\23320h");

    Initialized = 1;
}

#ifdef AMIFLUSH
/* This routine adapted from AmigaMail IV-37 by Michael Sinz */
static struct Message *
GetFMsg(port)
	struct MsgPort *port;
	{
	struct IntuiMessage *msg,*succ,*succ1;

	if(msg=(struct IntuiMessage *)GetMsg(port)){
		if(!flags.amiflush)return(msg);
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
	return(msg);
}
#endif
