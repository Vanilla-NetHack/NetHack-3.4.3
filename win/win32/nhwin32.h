/*	SCCS Id: @(#)nhwin32.h	3.2	95/09/06	*/
/* Copyright (c) NetHack MS Windows Porting Team 1995 */ 
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WIN32_H
#define WIN32_H

struct win32_WinDesc {
    int type;			/* type of window */
    boolean active;		/* true if window is active */
    boolean wasup;		/* true if menu/text window was already open */
    short rowcount;		/* Rows displayed in window */
    int  widest;		/* largest string in window */
/* Win32 stuff */
    HWND hWnd;			/* win32 window handle */
    HWND hDlg;			/* dialog box hande (for menus mostly) */
    HFONT hFnt;			/* handle of current font */
    HFONT hOldFnt;		/* handle of old font */
    DWORD dwCharX;		/* average width of characters */
    DWORD dwCharY;		/* height of characters */
#if 0
    DWORD dwClientX;		/* width of client area */
    DWORD dwClientY;		/* height of client area */
#endif
    DWORD dwLineLen;		/* line length */
    DWORD dwLines;		/* text lines in client area */
    int   nWindowX;		/* horizontal position of window */
    int   nWindowY;             /* vertical position of window */
    int   WindowWidth;		/* width of window */
    int   WindowHeight;        /* height of window */
    int   nCaretPosX;		/* horizontal position of the carat */
    int   nCaretPosY;		/* vertical position of the carat */
    int   nCharWidth;		/* width of a character */
    int   nCurChar;		/* current character */
    int   BackGroundColor;	/* background color */
    int   NormalTextColor;	/* default text color */
    int   maxrows, maxcols;	/* the maximum size used */
				/* maxcols is also used by WIN_MESSAGE for */
				/* tracking the ^P command */
/* NetHack data */
    int  *glyph;		/* the glyph values for a tiled map window */
    uchar *data;		/* the character value if not tiled */
    int  *color;		/* the text color for the data in the window */
    char *resp;			/* valid menu responses (for NHW_INVEN) */
    char *canresp;		/* cancel responses; 1st is the return value */
    char *morestr;		/* string to display instead of default */
    long wflags;
    short cursx, cursy;		/* Where the cursor is displayed at */
    short curs_apen,		/* Color cursor is displayed in */
	  curs_bpen;
    short extra;		/* temporary values between window calls */
};

struct win32_DisplayDesc {
/* we need this for Screen size (which will vary with display mode) */
    uchar rows, cols;		/* width & height of display in text units */
    short xpix, ypix;		/* width and height of display in pixels */
    int toplin;			/* flag for topl stuff */
    int rawprint;		/* number of raw_printed lines since synch */
    winid lastwin;		/* last window used for I/O */
};

struct win32_menuitem {
        int glyph;
        anything identifier;
        char ch;
        int attr;
        char *str;
};

#ifdef TEXTCOLOR
#define zap_color(n)  color = iflags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) color = iflags.use_color ? defsyms[n].color : NO_COLOR
#define obj_color(n)  color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define pet_color(n)  color = iflags.use_color ? mons[n].mcolor :	      \
				/* If no color, try to hilite pets; black  */ \
				/* should be HI				   */ \
				((iflags.hilite_pet && has_color(CLR_BLACK)) ? \
							CLR_BLACK : NO_COLOR)
# else /* no text color */
#define zap_color(n)
#define cmap_color(n)
#define obj_color(n)
#define mon_color(n)
#define pet_color(c)
#endif

#define WINMODE_TTY	0
#define WINMODE_WIN32	1
#define MAXWIN 20		/* maximum number of windows, cop-out */
#define NHIcon    	500
#define MAX_KEYS  	100
#define BUFSIZE         65535
#define SHIFTED         0x8000
#define RINGBUFSIZE	132
#define MAX_MESSAGE_COUNT  5
#define MAX_INVENTORY   52	     /* maximum items allowed in inventory */
#define MAX_MENU_WINDOWS MAXWIN    /* max inventory windows on the go */

#define WFLAGS_TILED   0x0001L

#define TYPE_INVALID      -1
#define TYPE_UNDETERMINED -2

#define IDM_SAVE           100
#define IDM_QUIT           101
#define IDM_EXIT           102
#define IDM_GAMEOPT        300
#define IDM_ENVOPT         301
#define IDM_DISPOPT        302
#define IDM_HELPCONTENTS   400
#define IDM_HELPSEARCH     401
#define IDM_HELPHELP       402
#define IDM_ABOUT          403

#define IDD_INPUTTEXT   501
#define IDD_QUESTION    502
#define IDD_NAME        503
#define IDD_ARCH        601
#define IDD_BARB        602
#define IDD_CAVEMAN     603
#define IDD_ELF         604
#define IDD_HEAL        605
#define IDD_KNIGHT      606
#define IDD_PRIEST      607
#define IDD_ROGUE       608
#define IDD_SAM         609
#define IDD_TOUR        610
#define IDD_VAL         611
#define IDD_WIZ         612
#define IDD_RAND        613
#define IDD_ABOUT1	616
#define IDD_ABOUT2	617
#define IDD_ABOUT3	618
#define IDD_ABOUT4	619
#define IDD_ABOUT5	620
#define IDD_ABOUT6	621
#define IDD_ABOUT7	622
#define IDD_ABOUT8	623
#define IDD_ABOUT9	624

#define IDD_LB1		700


#define DEBUG_MSG(str) \
if (strlen(str) != 0) \
MessageBox(BasehWnd,str,"Debug",MB_SYSTEMMODAL|MB_ICONHAND);

#define POP_MESSAGE(str) \
if (strlen(str) != 0) \
MessageBox(BasehWnd,str,"NetHackMenu",MB_SYSTEMMODAL);

#define NHW_BASE 0

/*
 *  The following global variables are defined in nhprocs.c
 */
extern struct win32_WinDesc *wins[MAXWIN];
extern struct win32_DisplayDesc *win32Display;
extern char   morc;		/* last character typed to xwaitforspace */
extern char   defmorestr[];	/* default --more-- prompt */
extern winid  WIN_BASE;

/*
 *  The following global variables are defined in winmain.c
 */

extern WNDCLASS wcNetHack;
extern WNDCLASS wcNHText;
extern WNDCLASS wcNHPopup;
extern WNDCLASS wcNHListbox;
extern char GameName[];
extern char NHTextClassName[];
extern char NHPopupClassName[];
extern char NHListboxClassName[];
extern HANDLE hGlobInstance;
extern HWND BasehWnd;

extern unsigned char *pchBuf;		/* input "ring" buffer */
extern unsigned char *pchGet;		/* "Get" chars from here */
extern unsigned char *pchPut;		/* "Put" chars here */
extern int pchCount;			/* characters in ring buffer */
extern int BaseUnits;
extern int BaseHeight;
extern int BaseWidth;
extern int MessageHeight;
extern int MessageWidth;
extern int MessageX;
extern int MessageY;
extern int MessageCount;
extern char *MessagePtr[MAX_MESSAGE_COUNT];
extern int MapHeight;
extern int MapWidth;
extern int MapX;
extern int MapY;
extern int StatusHeight;
extern int StatusWidth;
extern int StatusX;
extern int StatusY;
extern int tiles_on;			/* tiles enabled */
extern int DefCharWidth;
extern int DefCharHeight;
extern int DefBackGroundColor;
extern int DefNormalTextColor;
extern struct win32_menuitem *MenuPtr[MAX_MENU_WINDOWS][MAX_INVENTORY];
extern int MenuCount[MAX_MENU_WINDOWS];
extern int MenuWindowCount;
extern COLORREF colormap[];
extern HFONT hDefFnt;
extern TEXTMETRIC tm;
extern RECT rcClient;
extern int inputstatus;

extern char input_text[BUFSZ];
extern int input_text_size;
# define WAITING_FOR_KEY		1
# define WAITING_FOR_MOUSE		2
# define WAITING_FOR_KEY_OR_MOUSE	(WAITING_FOR_KEY | WAITING_FOR_MOUSE)


/* JUNK TO GO */
/* extern winid WIN_VIEW; */
/* extern winid WIN_VIEWBOX; */
/* #define NHW_VIEW	7 */
/* #define NHW_VIEWBOX	8 */

/* extern struct win32_WinDesc *win32_wins[MAXWIN + 6]; */



/*############################################################
 * External function prototypes
 *############################################################
 */
#define E extern


/* #### nhproc.c #### */

E void NDECL(win32_init_nhwindows);
E void NDECL(win32_player_selection);
E void NDECL(win32_askname);
E void NDECL(win32_get_nh_event) ;
E void FDECL(win32_exit_nhwindows, (const char *));
E void FDECL(win32_suspend_nhwindows, (const char *));
E void NDECL(win32_resume_nhwindows);
E winid FDECL(win32_create_nhwindow, (int));
E void FDECL(win32_clear_nhwindow, (winid));
E void FDECL(win32_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(win32_dismiss_nhwindow, (winid));
E void FDECL(win32_destroy_nhwindow, (winid));
E void FDECL(win32_curs, (winid,int,int));
E void FDECL(win32_putstr, (winid, int, const char *));
E void FDECL(win32_display_file, (const char *, BOOLEAN_P));
E void FDECL(win32_start_menu, (winid));
E void FDECL(win32_add_menu, (winid,int,const ANY_P,
			CHAR_P,int,const char *, BOOLEAN_P));
E void FDECL(win32_end_menu, (winid, const char *));
E int FDECL(win32_select_menu, (winid, int, MENU_ITEM_P **));
E void NDECL(win32_update_inventory);
E void NDECL(win32_mark_synch);
E void NDECL(win32_wait_synch);
#ifdef CLIPPING
E void FDECL(win32_cliparound, (int, int));
#endif
E void FDECL(win32_print_glyph, (winid,XCHAR_P,XCHAR_P,int));
E void FDECL(win32_raw_print, (const char *));
E void FDECL(win32_raw_print_bold, (const char *));
E int  NDECL(win32_nhgetch);
E int  FDECL(win32_nh_poskey, (int *, int *, int *));
E void NDECL(win32_nhbell);
E int  NDECL(win32_doprev_message);
E char FDECL(win32_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(win32_getlin, (const char *,char *));
E int  NDECL(win32_get_ext_cmd);
E void FDECL(win32_number_pad, (int));
E void NDECL(win32_delay_output);

/* other defs that really should go away (they're win32 specific) */
E void NDECL(win32_start_screen);
E void NDECL(win32_end_screen);
E void FDECL(genl_outrip, (winid,int));

/* #### win32msg.c #### */

E LONG WINAPI BaseWndProc(HWND,UINT,UINT,LONG);
E LONG WINAPI TextWndProc(HWND,UINT,UINT,LONG);
E LONG WINAPI PopupWndProc(HWND,UINT,UINT,LONG);
E LONG WINAPI ListboxWndProc(HWND,UINT,UINT,LONG);
E BOOL WINAPI PlayerSelectProc(HWND,UINT,UINT,LONG);
E BOOL WINAPI CopyrightProc(HWND,UINT,UINT,LONG);
E BOOL WINAPI AskNameProc(HWND,UINT,UINT,LONG);
E LRESULT CALLBACK MenuDialogProc(HWND,UINT,UINT,LONG);
E int win32_kbhit(void);

/* #### winmain.c #### */

E void NDECL(win_win32_init);
E BOOL InitBaseWindow(void);
E BOOL InitTextWindow(void);
E BOOL InitPopupWindow(void);
E BOOL InitListboxWindow(void);
E BOOL CALLBACK EnumChildProc(HWND,LPARAM);
#if 0
E void WinClear(HWND);
E int WinGetChar(HWND);
E int WinPutChar(HWND,int,int,int);
E void WinMinSize(HWND,int,int);
E void WinHackGetText(const char *,char *);
#endif
#undef E

#endif /* WIN32_H */
