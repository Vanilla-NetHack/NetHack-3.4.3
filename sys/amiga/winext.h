/*    SCCS Id: @(#)winext.h    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

extern int clipping;
extern int clipx;
extern int clipy;
extern int clipxmax;
extern int clipymax;
extern int scrollmsg;
extern int alwaysinvent;

#ifndef	SHAREDLIB
extern struct amii_DisplayDesc *amiIDisplay;	/* the Amiga Intuition descriptor */
extern struct window_procs amii_procs;
extern struct window_procs amiv_procs;
extern unsigned short amii_initmap[ 1L << DEPTH ];
extern int bigscreen;
extern winid amii_rawprwin;
extern const char *roles[];
extern struct Screen *HackScreen;
extern struct Library *ConsoleDevice;
extern char Initialized;
extern char toplines[ BUFSZ ];
extern NEARDATA winid WIN_MESSAGE;
extern NEARDATA winid WIN_MAP;
extern NEARDATA winid WIN_STATUS;
extern NEARDATA winid WIN_INVEN;
#else
extern WinamiBASE *WinamiBase;
#endif

extern struct GfxBase *GfxBase;
extern struct Library *DiskfontBase;
extern struct IntuitionBase *IntuitionBase;

/* All kinds of shared stuff */
#ifdef	VIEWWINDOW
extern struct TextAttr Hack160;
extern struct TextAttr Hack40;
#endif
extern struct TextAttr Hack80;
extern struct TextAttr TextsFont13;
extern struct Window *pr_WindowPtr;
extern struct Menu HackMenu[];
extern unsigned char KbdBuffered;
extern struct TextFont *TextsFont;
extern struct TextFont *HackFont;
#ifdef	VIEWWINDOW
extern struct TextFont *HackFont4;
extern struct TextFont *HackFont16;
#endif
extern struct IOStdReq ConsoleIO;
extern struct MsgPort *HackPort;

extern int txwidth, txheight, txbaseline;
extern struct BitMap amii_vbm;

/* This gadget data is replicated for menu/text windows... */
extern struct PropInfo PropScroll;
extern struct Image Image1;
extern struct Gadget MenuScroll;

/* This gadget is for the message window... */
extern struct PropInfo MsgPropScroll;
extern struct Image MsgImage1;
extern struct Gadget MsgScroll;

extern struct TagItem tags[];

extern struct win_setup
{
    struct NewWindow newwin;
    UWORD offx,offy,maxrow,rows,maxcol,cols;	/* CHECK TYPES */
} new_wins[];

extern UWORD scrnpens[];
/* The last Window event is stored here for reference. */
extern WEVENT lastevent;
extern const char winpanicstr[];
extern struct TagItem scrntags[];
extern struct NewScreen NewHackScreen;

extern int topl_addspace;
extern char spaces[ 76 ];
extern int wincnt;   /* # of nh windows opened */
extern struct Rectangle lastinvent, lastmsg;
