/*	SCCS Id: @(#)winami.h	3.1	93/01/17	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1991. */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois, 1992, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINAMI_H
#define WINAMI_H

/* descriptor for Amiga Intuition-based windows.  If we decide to cope with
 * tty-style windows also, then things will need to change. */
/* per-window data */
struct WinDesc {
    xchar type;			/* type of window */
    boolean active;		/* true if window is active */
    xchar offx, offy;		/* offset from topleft of display */
    short rows, cols;		/* dimensions */
    short curx, cury;		/* current cursor position */
    short maxrow, maxcol;	/* the maximum size used -- for INVEN wins */
				/* maxcol is also used by WIN_MESSAGE for */
				/* tracking the ^P command */
    char **data;		/* window data [row][column] */
    char *resp;			/* valid menu responses (for NHW_INVEN) */
    char *canresp;		/* cancel responses; 1st is the return value */
    char *morestr;		/* string to display instead of default */
/* amiga stuff */
    struct Window *win;		/* Intuition window pointer */
#ifdef	INTUI_NEW_LOOK
    struct ExtNewWindow *newwin;	/* NewWindow alloc'd */
#else
    struct NewWindow *newwin;	/* ExtNewWindow alloc'd */
#endif
#define FLMAP_INGLYPH	1	/* An NHW_MAP window is in glyph mode */
#define FLMAP_CURSUP	2	/* An NHW_MAP window has the cursor displayed */
    long flags;
    short cursx, cursy;		/* Where the cursor is displayed at */
    short curs_apen,		/* Color cursor is displayed in */
	  curs_bpen;
};

/* descriptor for intuition-based displays -- all the per-display data */
/* this is a generic thing - think of it as Screen level */

struct DisplayDesc {
/* we need this for Screen size (which will vary with display mode) */
    uchar rows, cols;		/* width & height of display in text units */
    short xpix, ypix;		/* width and height of display in pixels */
    int toplin;			/* flag for topl stuff */
    int rawprint;		/* number of raw_printed lines since synch */
    winid lastwin;		/* last window used for I/O */
};

typedef enum {
	WEUNK, WEKEY, WEMOUSE, WEMENU,
} WETYPE;

typedef struct WEVENT
{
	WETYPE type;
	union {
		int key;
		struct {
			int x, y;
			int qual;
		} mouse;
		long menucode;
	} u;
} WEVENT;

#define MAXWIN 20		/* maximum number of windows, cop-out */

/* port specific variable declarations */
extern winid WIN_BASE;
#define NHW_BASE	6

extern struct WinDesc *wins[MAXWIN + 1];

extern struct DisplayDesc *amiIDisplay;	/* the Amiga Intuition descriptor */

extern char morc;		/* last character typed to xwaitforspace */
extern char defmorestr[];	/* default --more-- prompt */

#endif /* WINAMI_H */
