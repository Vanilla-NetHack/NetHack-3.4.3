/*
 *  amiwind.c	(C) Copyright 1989 by Olaf Seibert (KosmoSoft)
 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Here is some very Amiga specific stuff, dealing with
 *  screens, windows, menus, and input via IntuiMessages.
 */

#define MANX			/* Define for the Manx compiler */

#include <exec/types.h>
#include <exec/alerts.h>
#include <exec/io.h>
#include <exec/devices.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <intuition/intuition.h>
#include <libraries/dosextens.h>

#undef TRUE			/* All these are also defined in */
#undef FALSE			/* the Amiga system include files */
#undef COUNT
#undef NULL

#include "hack.h"

#include "amimenu.c"

/*  First, external declarations... */

struct Library *OpenLibrary();
struct Screen *OpenScreen();
struct Window *OpenWindow();
struct TextFont *OpenDiskFont(), *OpenFont();
struct IntuiMessage *GetMsg();
struct MenuItem *ItemAddress();
struct Process *FindTask();         /* Cheating */
long DeadKeyConvert(), OpenDevice(), CloseDevice();
struct MsgPort *CreatePort();
extern struct Library *IconBase;
void abort();

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

struct Device *ConsoleDevice;

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
#define DEPTH		2

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

int ConvertKey(message)
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
	abort(AG_IOError | AO_ConsoleDev);
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

static char ProcessMessage(message)
register struct IntuiMessage *message;
{
    switch(message->Class) {
    case MENUPICK:
	{
	    USHORT thismenu;
	    struct MenuItem *item = NULL;

	    thismenu = message->Code;
	    while (thismenu != MENUNULL) {
		item = ItemAddress(&HackMenu, (ULONG) thismenu);
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
    ReplyMsg(message);
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
	    (message = GetMsg(HackWindow->UserPort)) )
	ProcessMessage(message);

    return KbdBuffered;
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
	abort(AG_IOError | AO_ConsoleDev);
	return;
    }
#endif

    if (Buffered) {
	ConsoleIO.io_Command = CMD_WRITE;
	ConsoleIO.io_Data = (APTR)ConsoleBuffer;
	ConsoleIO.io_Length = Buffered;
	DoIO(&ConsoleIO);
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
    register int len = _BUILTIN_strlen(string);

    if (len + Buffered >= CONBUFFER)
	WindowFlush();

    _BUILTIN_strcpy(ConsoleBuffer + Buffered, string);
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

void WindowPrintf(fmt, args, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
char *fmt;
long args, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9;
{
#ifdef MANX	    /* Efficient but not portable */
    format(WindowPutchar, fmt, &args);
#else
    WindowFlush();  /* Don't know if all will fit */
# ifdef __STDC__    /* Cheap and portable way */
    vsprintf(ConsoleBuffer, fmt, &args);
# else		    /* Expensive... */
    sprintf(ConsoleBuffer, fmt, args, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
# endif
    ConsoleIO.io_Command = CMD_WRITE;
    ConsoleIO.io_Data = (APTR)ConsoleBuffer;
    ConsoleIO.io_Length = -1;
    DoIO(&ConsoleIO);
#endif
}

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

	CloseDevice(&ConsoleIO);
	ConsoleDevice = NULL;
    }
    if (ConsoleIO.io_Message.mn_ReplyPort)
	DeletePort(ConsoleIO.io_Message.mn_ReplyPort);
    if (HackWindow) {
	register struct IntuiMessage *msg;

	FindTask(NULL)->pr_WindowPtr = (APTR) pr_WindowPtr;
	ClearMenuStrip(HackWindow);
	Forbid();
	while (msg = GetMsg(HackWindow->UserPort))
	    ReplyMsg(msg);
	CloseWindow(HackWindow);
	Permit();
	HackWindow = NULL;
    }
    if (HackScreen) {
	CloseScreen(HackScreen);
	HackScreen = NULL;
    }
    /* if (IconBase) {
	CloseLibrary(IconBase);
	IconBase = NULL;
    } */
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

void abort(rc)
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
    CleanUp();
#undef exit
    exit(rc);
}

/*  Used by library routines, and the debugger */

void _abort()
{
    abort(-10L);
}

/*
 *  Open everything we need.
 */

void Initialize()
{
    if (Initialized)
	return;

    if ( (IntuitionBase = OpenLibrary("intuition.library", LIBRARY_VERSION))
	  == NULL)
	abort(AG_OpenLib | AO_Intuition);

#ifdef HACKFONT

    if ( (GfxBase = OpenLibrary("graphics.library", LIBRARY_VERSION)) == NULL)
	abort(AG_OpenLib | AO_GraphicsLib);

    /*
     *	Force our own font to be loaded, if possible.
     *	If we can open diskfont.library, but not our font, we can close
     *	the diskfont.library again since it just wastes memory.
     *	Even if we can open the font, we don't need the diskfont.library
     *	anymore, since CloseFont is a graphics.library function.
     */

    if ((HackFont = OpenFont(&Hack80)) == NULL) {
	if (DiskfontBase = OpenLibrary("diskfont.library", LIBRARY_VERSION)) {
	    Hack80.ta_Name -= SIZEOF_DISKNAME;
	    HackFont = OpenDiskFont(&Hack80);
	    Hack80.ta_Name += SIZEOF_DISKNAME;
	    CloseLibrary(DiskfontBase);
	    DiskfontBase = NULL;
	}
    }
#endif

    /* if ( (IconBase = OpenLibrary("icon.library", LIBRARY_VERSION)) == NULL)
	abort(AG_OpenLib | AO_IconLib); */

    /*
     *	Now Intuition is supposed to use our HackFont for the screen,
     *	since we have a corresponding TextAttr, but it *doesn't*.
     *	So, we need to do a SetFont() a bit later on.
     */
    if ( (HackScreen = OpenScreen(&NewHackScreen)) == NULL)
	abort(AN_OpenScreen & ~AT_DeadEnd);

    NewHackWindow.Screen = HackScreen;

    if ( (HackWindow = OpenWindow(&NewHackWindow)) == NULL)
	abort(AN_OpenWindow & ~AT_DeadEnd);

    SetMenuStrip(HackWindow, &HackMenu);
    {
	register struct Process *myProcess = FindTask(NULL);
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
    if (OpenDevice("console.device", 0L, &ConsoleIO, 0L) != 0)
	abort(AG_OpenDev | AO_ConsoleDev);

    ConsoleDevice = ConsoleIO.io_Device;

    Buffered = 0;
    KbdBuffered = 0;

    /* set CRMOD on */
    WindowFPuts("\23320h");

    Initialized = 1;
}
