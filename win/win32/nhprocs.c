/*	SCCS Id: @(#)nhprocs.c	3.2	96/02/02	*/
/* Copyright (c) NetHack Windows Porting Team 1995      */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef WIN32_GRAPHICS

# ifndef NO_SIGNAL
#include <signal.h>
# endif
#include <ctype.h>
#include <sys\stat.h>
#include "win32api.h"
#include "nhwin32.h"

#define DEBUG

/* 
 *  Interface definition, for windows.c 
 */
struct window_procs win32_procs = {
    "win32",
    win32_init_nhwindows,
    win32_player_selection,
    win32_askname,
    win32_get_nh_event,
    win32_exit_nhwindows,
    win32_suspend_nhwindows,
    win32_resume_nhwindows,
    win32_create_nhwindow,
    win32_clear_nhwindow,
    win32_display_nhwindow,
    win32_destroy_nhwindow,
    win32_curs,
    win32_putstr,
    win32_display_file,
    win32_start_menu,
    win32_add_menu,
    win32_end_menu,
    win32_select_menu,
    genl_message_menu,
    win32_update_inventory,
    win32_mark_synch,
    win32_wait_synch,
#ifdef CLIPPING
    win32_cliparound,
#endif
#ifdef POSITIONBAR
    donull,
#endif
    win32_print_glyph,
    win32_raw_print,
    win32_raw_print_bold,
    win32_nhgetch,
    win32_nh_poskey,
    win32_nhbell,
    win32_doprev_message,
    win32_yn_function,
    win32_getlin,
    win32_get_ext_cmd,
    win32_number_pad,
    win32_delay_output,

    /* other defs that really should go away (they're tty specific) */
    win32_start_screen,
    win32_end_screen,
    genl_outrip,
};

/* Local functions */
static void win32_putsym(winid, int, int, int, unsigned char, boolean);
static int  has_color(int color);
static int key_from_buf();
static HANDLE new_dialog();
static void deferred_init(winid, int);

struct win32_WinDesc *wins[MAXWIN];
struct win32_DisplayDesc *win32Display;	/* the win32 display descriptor */
winid  WIN_BASE;

static int maxwin = 0;			/* number of windows in use */
static char winpanicstr[] = "Bad window id %d";
# ifdef CLIPPING
static boolean clipping = FALSE;	/* clipping on? */
static int clipx = 0, clipy = 0, clipxmax = 0, clipymax = 0;
# endif
#define PICK_PROMPT "Shall I pick a character for you? [Y, N, or Q(quit)] "
static char to_continue[] = "to continue";
winid window;
int x, y;
int color;
unsigned char ch;

#ifdef DEBUG_FULL
static char debugbuf[256];
#endif

void
win32_init_nhwindows(argcp, argv)
int *argcp;
char **argv;
{
	int status;
	static FARPROC pfnNameDialog;

        hDefFnt = GetStockObject(OEM_FIXED_FONT);
        if (!hDefFnt) hDefFnt = GetStockObject(SYSTEM_FIXED_FONT);
	WIN_BASE = win32_create_nhwindow(NHW_BASE);
	pfnNameDialog = MakeProcInstance(CopyrightProc,
					 wcNetHack.hInstance);
	DialogBox(wcNetHack.hInstance,"HACK_COPYRIGHT",BasehWnd,
				pfnNameDialog);
	status = InitTextWindow();
	status = InitPopupWindow();
	status = InitListboxWindow();
	ShowWindow(BasehWnd,SW_SHOWDEFAULT);
	UpdateWindow(BasehWnd);
	

	pchBuf = (char *)alloc(RINGBUFSIZE);   /* allocate the input buffer */
	pchGet = pchBuf;
	pchPut = pchBuf;
	pchCount = 0;
	return;
}

void
win32_player_selection()
{
	char pc;


	if ((pc = highc(pl_character[0])) != 0) {
		if (index(pl_classes, pc) != (char *)0) {
			pl_character[0] = pc;
			return;
		}
	}
	DialogBox(wcNetHack.hInstance,"CHAR_SEL_DIALOG",BasehWnd,
			(DLGPROC)PlayerSelectProc);
	return;	
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 * Always called after init_nhwindows() and before display_gamewindows().
 */
void
win32_askname()
{
	input_text_size = 0;
	while (input_text_size == 0) {
		DialogBox(wcNetHack.hInstance,"ASK_NAME_DIAL",BasehWnd,
			(DLGPROC)AskNameProc);
	}
	strncpy(plname, input_text, input_text_size);
	*(plname + input_text_size) = '\0';
	return;	
}

void
win32_get_nh_event()
{
        MSG  lpMsg;

        if (PeekMessage(&lpMsg,0,0,0,PM_REMOVE)) {
/*                if (msg.message == WM_QUIT) */
                TranslateMessage(&lpMsg);
                DispatchMessage(&lpMsg);
        }
}

void
win32_suspend_nhwindows(str)
    const char *str;
{
}

void
win32_resume_nhwindows()
{
}

void
win32_exit_nhwindows(str)
    const char *str;
{
}

winid
win32_create_nhwindow(type)
int type;
{
	RECT rcClient;
	struct win32_WinDesc *newwin;
	winid newid;
	char WindowName[80];
	int status;
	int msize = 0;
	int i;

	/* allocate the Window Descriptor */
	newwin = (struct win32_WinDesc *)alloc(sizeof(struct win32_WinDesc));
	memset(newwin,0,sizeof(struct win32_WinDesc));
	newwin->type = TYPE_INVALID;

	/* Get an unused Window ID number */
	for(newid = 0; newid < MAXWIN; newid++) {
		if(wins[newid] == 0) {
			wins[newid] = newwin;
			break;
		}
	}
	if (newid == MAXWIN) {
		panic("No free window slots!");
		return WIN_ERR;
	}

	newwin->WindowWidth = 0;
	newwin->WindowHeight = 0;
	newwin->dwCharX = DefCharWidth;
	newwin->dwCharY = DefCharHeight;
	newwin->morestr = (char *)0;

	/* Build the appropriate type of Win32 Window */
	switch(type) {	
		
		case NHW_BASE:
				InitBaseWindow();
				newwin->hWnd = BasehWnd;
				newwin->type = NHW_BASE;
				newwin->wflags = 0L;
				newwin->BackGroundColor = colormap[CLR_BLACK];
				newwin->NormalTextColor = colormap[CLR_GRAY];
				break;
	
		case NHW_MESSAGE:
				strcpy(WindowName,"NetHack Message Window");
 				newwin->hWnd=CreateWindowEx(
					0,
					NHTextClassName,
					WindowName,
					WS_CHILD|WS_VISIBLE
#if 0
					|WS_BORDER|WS_VSCROLL,
#endif
					|WS_BORDER,
					0,0,
					newwin->WindowWidth,
					newwin->WindowHeight,
					BasehWnd,(HMENU)(int)newid,
					wcNetHack.hInstance,
					(LPSTR)0);
				if (!newwin->hWnd) status = GetLastError();
				newwin->type = NHW_MESSAGE;
				newwin->wflags = 0L;
				newwin->maxrows = 5;
				newwin->maxcols = COLNO;
				newwin->BackGroundColor = colormap[CLR_BLACK];
				newwin->NormalTextColor = colormap[CLR_GRAY];
				newwin->cursy = 0;
				newwin->cursx = 0;
				break;
		case NHW_STATUS:
				strcpy(WindowName,"NetHack Status Window");
 				newwin->hWnd=CreateWindowEx(
					0,
					NHTextClassName,
					WindowName,
					WS_CHILD|WS_VISIBLE|WS_BORDER,
					0,0,
					newwin->WindowWidth,
					newwin->WindowHeight,
					BasehWnd,(HMENU)(int)newid,
					wcNetHack.hInstance,
					(LPSTR)0);
				if (!newwin->hWnd) status = GetLastError();
				newwin->type = NHW_STATUS;
				newwin->wflags = 0L;
				newwin->maxrows = 2;
				newwin->maxcols = COLNO;
				newwin->cursy = 0;
				newwin->cursx = 0;
				newwin->BackGroundColor = colormap[CLR_BLACK];
				newwin->NormalTextColor = colormap[CLR_GRAY];
				break;
		case NHW_MAP:
				strcpy(WindowName,"NetHack Map Window");
 				newwin->hWnd=CreateWindowEx(
					0,
					NHTextClassName,
					WindowName,
					WS_CHILD|WS_VISIBLE|WS_BORDER,
					0,0,
					newwin->WindowWidth,
					newwin->WindowHeight,
					BasehWnd,(HMENU)(int)newid,
					wcNetHack.hInstance,
					(LPSTR)0);
				if (!newwin->hWnd) status = GetLastError();
				newwin->type = NHW_MAP;
				newwin->wflags = 0;
				newwin->maxrows = ROWNO;
				newwin->maxcols = COLNO;
#if 0
				newwin->BackGroundColor = RGB(80,80,80);
				newwin->NormalTextColor = RGB(175,175,175);
#endif
				newwin->BackGroundColor = colormap[CLR_BLACK];
				newwin->NormalTextColor = colormap[CLR_GRAY];
				if (tiles_on)
					newwin->wflags |= WFLAGS_TILED;
				break;
		case NHW_TEXT:
		case NHW_MENU:
				/* This will be a menu or text window
				 * The first call to place something into 
				 * it is what determines the type, so all
				 * we can do for now is allocate the structure
			         * and flag that the type has not been
				 * determined yet.  The window will actually
				 * be created later, after type determination.
				 */
				newwin->type = TYPE_UNDETERMINED;
				newwin->BackGroundColor = colormap[CLR_BLACK];
				newwin->NormalTextColor = colormap[CLR_GRAY];
				newwin->extra = newid;
				newwin->widest = 0;
				newwin->wflags = 0L;
				newwin->morestr = "--more--";
				break;
	}
	if ((newwin->type != TYPE_UNDETERMINED) && 
	   ((newwin == (struct win32_WinDesc *)0) || (!newwin->hWnd))) {
		panic("Win32 Window creation failure!");
		return WIN_ERR;
	}
#if 0
	ShowWindow(newwin->hWnd,SW_SHOWDEFAULT);
	UpdateWindow(newwin->hWnd);
#endif
	if (newwin->type != TYPE_UNDETERMINED) {
		msize = newwin->maxrows * newwin->maxcols;
		newwin->data  = (uchar *)alloc(msize * sizeof(uchar));
		if (newwin->wflags & WFLAGS_TILED) 
			newwin->glyph = (int *)alloc(msize * sizeof(int));
		memset(newwin->data,' ',msize * sizeof(uchar));
		newwin->color = (int *)alloc(msize * sizeof(int));
		for (i = 0; i < msize; ++i)
			*(newwin->color + i) = newwin->NormalTextColor;
		newwin->hFnt = hDefFnt;
		if (newid == 3) {	/* the last of the default windows */
		    GetClientRect(BasehWnd, &rcClient);
		    EnumChildWindows(BasehWnd, EnumChildProc,(LPARAM)&rcClient);
		}
		newwin->active = TRUE;
	}
	return newid; 
}

void
win32_clear_nhwindow(window)
winid window;
{
	int i;
	int msize;

	if (window != NHW_MESSAGE) {
		msize = wins[window]->maxrows * wins[window]->maxcols;
		memset(wins[window]->data,' ',msize * sizeof(uchar));
		for (i = 0; i < msize ; ++i)
			*(wins[window]->color + i) =
				wins[window]->BackGroundColor;
	} else {
		/* gray old messages out */
		msize = (wins[window]->maxrows) * wins[window]->maxcols;
		for (i = 0; i < msize ; ++i)
			*(wins[window]->color + i) =
				RGB(140,140,140);
	}
	InvalidateRect(wins[window]->hWnd, (RECT *)0, FALSE);
}

/*ARGSUSED*/
void
win32_display_nhwindow(window, blocking)
winid window;
boolean blocking;
{
	if (window == WIN_MESSAGE) flags.window_inited = TRUE;
	if (blocking && wins[window]->morestr) 
		win32_putstr(window, 1, wins[window]->morestr);

	if (wins[window]->type == NHW_TEXT && !wins[window]->active) {
		deferred_init(window, 2);
	}
	ShowWindow(wins[window]->hWnd,SW_SHOWNORMAL);
	if (blocking) {
		win32_nhgetch();
	}
}

void
win32_destroy_nhwindow(window)
winid window;
{
	int i;
	int stat;

	if (wins[window]->type == NHW_MENU) {
		for (i = 0; i < MenuCount[window]; ++i) {
			if (MenuPtr[window][i]) {
				if (MenuPtr[window][i]->str)
				    free((genericptr_t)MenuPtr[window][i]->str);
				free((genericptr_t)MenuPtr[window][i]);
				MenuPtr[window][i] = (struct win32_menuitem *)0;
			}
		}	
		MenuCount[window] = 0;
		--MenuWindowCount;
	}
	if (wins[window]->hWnd != 0) {
		stat = DestroyWindow(wins[window]->hWnd);
	}
	if (wins[window]) free(wins[window]);
	else impossible("attempt to free wins[window] = 0 [%d]", window);
	wins[window] = 0;
}

void
win32_curs(window, col, row)
winid window;
register int col, row;	/* not xchar: perhaps xchar is unsigned and
			   curx-x would be unsigned as well */
{
	wins[window]->cursx = col;
	wins[window]->cursy = row;
	wins[window]->nCaretPosX = col * wins[window]->dwCharX;
	wins[window]->nCaretPosY = row * wins[window]->dwCharY;
	SetCaretPos(wins[window]->nCaretPosX,wins[window]->nCaretPosY);
}

static void
win32_putsym(window, col, row, rgbcolor, ch, flushflag)
winid window;
int col, row;
int rgbcolor;
unsigned char ch;
boolean flushflag;
{
	RECT rect;
/*	char tch[2]="X"; */
	int offset;
	uchar *pch;
	int *pcolor;

	offset = (row * wins[window]->maxcols) + col;
	pch = wins[window]->data + offset;
	pcolor = wins[window]->color + offset;
	*pch = ch;
	*pcolor = rgbcolor;

	if (flushflag) {
		/* signal to windows that the display is outdated */
		rect.left   = col * DefCharWidth;
		rect.top    = row * DefCharHeight;
		rect.right  = col * DefCharWidth + DefCharWidth;
		rect.bottom = row * DefCharHeight + DefCharHeight;

		InvalidateRect(wins[window]->hWnd, &rect, FALSE);
	}
}

void
win32_putstr(window, attr, str)
winid window;
int attr;
const char *str;
{
	int i, j;
	RECT rect;
	int cnt,start;
	char *tmp;

	int offset;
	uchar *pch;
	int *pcolor;

	if (wins[window]->type == TYPE_UNDETERMINED) {
	/* Okay, now we know...its destined to be a text window */
		deferred_init(window, 1);
	}

	switch(wins[window]->type) {
	    case NHW_MESSAGE:
		tmp = (char *)alloc(strlen(str)+1);
		Strcpy(toplines, str); 		/* for Norep() */
		Strcpy(tmp, str);
		if (MessageCount >= MAX_MESSAGE_COUNT) {
			free((genericptr_t)MessagePtr[0]);
			for (i = 0; i < (MessageCount - 1); ++i) {
				MessagePtr[i] = MessagePtr[i + 1];
			}
			MessagePtr[MessageCount - 1] = tmp;
		} else {
			MessagePtr[MessageCount++] = tmp;
		}
		/* clear out old data */
		i = wins[window]->maxrows * wins[window]->maxcols;
		memset(wins[window]->data,' ',i * sizeof(uchar));

		/* add the new data */
		for (i = 0; i < MessageCount; ++i) {
			offset = (i * wins[window]->maxcols);
			pch = wins[window]->data + offset;
			pcolor = wins[window]->color + offset;
			if (i == MessageCount - 1)  {
			    for (j = 0; j < wins[window]->maxcols; ++j)
				*(wins[window]->color + (offset + j)) =
					RGB(200,200,200);
			}
			memcpy(pch, MessagePtr[i], strlen(MessagePtr[i]));
		}
		InvalidateRect(wins[window]->hWnd, (RECT *)0, FALSE);
		break;
	    case NHW_MENU:
		impossible("putstr to a menu window (%d)", window);
		break;
	    case NHW_STATUS:
		for (cnt=0; cnt < strlen(str); ++cnt) {
		win32_putsym(window,wins[window]->cursx++,wins[window]->cursy,
				wins[window]->NormalTextColor,*(str + cnt), 
				TRUE);
		}
		break;
	    case NHW_TEXT:
		/* fall through */
	    default:
		if (strlen(str) > wins[window]->widest) {
			wins[window]->widest = strlen(str);
		}
		for (cnt=0; cnt < (int)strlen(str); ++cnt) {
		win32_putsym(window,wins[window]->cursx++,wins[window]->cursy,
				wins[window]->NormalTextColor,*(str + cnt), 
				FALSE);
		}
		win32_curs(window,0,wins[window]->cursy+1);
	}
}

void
win32_display_file(fname, complain)
const char *fname;
boolean complain;
{
}


void
win32_update_inventory()
{
#ifdef DEBUG_FULL
	DEBUG_MSG("into update_inventory");
#endif
}

void
win32_mark_synch()
{
}

void
win32_wait_synch()
{
}

void
#ifdef CLIPPING
win32_setclipped()
{
}

void
win32_cliparound(x, y)
int x, y;
{
}
#endif /* CLIPPING */

/*
 *  win32_print_glyph
 *
 *  Print the glyph to the output device.  Don't flush the output device.
 *
 *  Since this is only called from show_glyph(), it is assumed that the
 *  position and glyph are always correct (checked there)!
 */
void
win32_print_glyph(window, col, row, glyph)
winid window;
xchar col, row;
int glyph;
{
    uchar   ch;
    register int offset;
    int *ip;
    int color;

    /* In this routine 'color' is a NetHack color, not a WIN32 color */
    color = NO_COLOR;
    if (wins[window]->wflags & WFLAGS_TILED) {
	ip = wins[window]->glyph + ((row * wins[window]->maxcols) + col); 
	*ip = glyph;
    } else {
	    /*
	     *  Map the glyph back to a character.
	     *
	     *  Warning:  For speed, this makes an assumption on the order of
	     *		  offsets.  The order is set in display.h.
	     */
	    if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) {	  /* swallow */
		/* see swallow_to_glyph() in display.c */
		ch = (uchar) showsyms[S_sw_tl + (offset & 0x7)];
		mon_color(offset >> 3);
	    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) { /* zap beam*/
		/* see zapdir_to_glyph() in display.c */
		ch = showsyms[S_vbeam + (offset & 0x3)];
		zap_color((offset >> 2));
	    } else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) { /* cmap */
		ch = showsyms[offset];
		cmap_color(offset);
	    } else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) {  /* object */
		ch = oc_syms[(int)objects[offset].oc_class];
		obj_color(offset);
	    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) { /* corpse */
		ch = oc_syms[(int)objects[CORPSE].oc_class];
		mon_color(offset);
	    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {  /* pet */
		ch = monsyms[(int)mons[offset].mlet];
		pet_color(offset);
	    } else {						   /* monster */
		ch = monsyms[(int)mons[glyph].mlet];
		mon_color(glyph);
	    }
#ifdef REINCARNATION
    	    if (Is_rogue_level(&u.uz)) color = NO_COLOR;
#endif
	    win32_curs(window,col,row);
	    win32_putsym(window,wins[window]->cursx,
			 wins[window]->cursy,colormap[color],ch, TRUE);
    }
        
}

void
win32_raw_print(str)
const char *str;
{
	int button;

	if (str)
		if (strlen(str) != 0) {
			button = MessageBox((HWND)0,str,"raw_print",
			    MB_APPLMODAL|MB_ICONSTOP
#ifndef DEBUG
				|MB_OK);
#else
				|MB_OKCANCEL);
			if (button == IDCANCEL) 
				button = *((int *)0); /* Bombs away ! */
#endif
		}
}

void
win32_raw_print_bold(str)
const char *str;
{
	win32_raw_print(str);
}

int
win32_nhgetch()
{
	return key_from_buf();
}

/*
 * return a key, or 0, in which case a mouse button was pressed
 * mouse events should be returned as character postitions in the map window.
 */
/*ARGSUSED*/
int
win32_nh_poskey(x, y, mod)
int *x, *y, *mod;
{
	return key_from_buf();	/* mouse support to come later */
}

int
win32_doprev_message()
{
    register struct win32_WinDesc *cw = wins[WIN_MESSAGE];

    return 0;
}

char
win32_yn_function(query, choices, def)
const char *query,*choices;
char def;
/*
 *   Generic yes/no function. 'def' is the default (returned by space or
 *   return; 'esc' returns 'q', or 'n', or the default, depending on
 *   what's in the string. The 'query' string is printed before the user
 *   is asked about the string.
 *   If choices is NULL, any single character is accepted and returned.
 */
{
	unsigned int style;
	int button;
	char buf[QBUFSZ];
	int i;

	if (choices) {
	    char *cb, choicebuf[QBUFSZ];

	    Strcpy(choicebuf, choices);	/* anything beyond <esc> is hidden */
	    if ((cb = index(choicebuf, '\033')) != 0) *cb = '\0';
	    if ((1 + strlen(query) + 2 + strlen(choicebuf) + 4) >= QBUFSZ)
		panic("yn_function: question too long");
	    if (query) {
		Sprintf(buf, "%s [%s] ", query, choicebuf);
		if (def) Sprintf(eos(buf), "(%c) ", def);
	    }
	}else
		return key_from_buf();
	i = strlen(buf);
	style =	MB_APPLMODAL|MB_ICONQUESTION|MB_YESNOCANCEL;
	if ((def == 'y') || (def == 'Y'))
		style |= MB_DEFBUTTON1;
	else if((def == 'n') || (def == 'N'))
		style |= MB_DEFBUTTON2;
	else if((def == 'q') || (def == 'Q'))
		style |= MB_DEFBUTTON3;

	button = MessageBox(
			BasehWnd,
			buf,
			"Question",
			style);
	if (button == IDYES)
		return 'y';
	else if (button == IDNO)
		return 'n';
	else if (button == IDCANCEL)
		return 'q';
	else return button;
}
void
win32_delay_output()
{
}

void
win32_end_screen()
{
}

int
win32_get_ext_cmd()
{
	return 0;
}

void
win32_getlin(pr,inline)
const char *pr;
char *inline;
{
}

void
win32_nhbell()
{
}

void
win32_number_pad(x)
int x;
{
}
void
win32_start_screen()
{
}

static int has_color(int color)
{
	return 1;
}

int
key_from_buf()
{
	int ch;

	inputstatus = WAITING_FOR_KEY;
	while (!pchCount) {
		win32_get_nh_event();
	}
	inputstatus = 0;
	ch = *pchGet++;
	if (pchGet >= pchBuf + (RINGBUFSIZE - 1)) pchGet = pchBuf; /* wrap */
	--pchCount;
	return ch;
}

void deferred_init(window,part)
winid window;
int part;
{
	int i, msize, status;
	char WindowName[80];

#ifdef DEBUG_FULL
	DEBUG_MSG("Initializing a text window");
#endif
	/* Do all the initialization that we couldn't do earlier */
	if (part == 1) {
		/* the stuff NetHack depends on */
#ifdef DEBUG_FULL
		DEBUG_MSG("About to do deferred init part 1");
#endif
		wins[window]->active = FALSE;
		wins[window]->type = NHW_TEXT;
		wins[window]->wflags = 0L;
		wins[window]->maxrows = ROWNO;
		wins[window]->maxcols = COLNO;
		wins[window]->hFnt = hDefFnt;
		msize = wins[window]->maxrows * wins[window]->maxcols;
		wins[window]->data  = 
			(uchar *)alloc(msize * sizeof(uchar));
		memset(wins[window]->data,' ',msize * sizeof(uchar));
		wins[window]->color = 
			(int *)alloc(msize * sizeof(int));
		for (i = 0; i < msize; ++i)
		     *(wins[window]->color + i) = wins[window]->NormalTextColor;
	} else if (part == 2) {
		/* the stuff WIN32 depends on */
#ifdef DEBUG_FULL
		DEBUG_MSG("About to do deferred init part 2");
#endif
		wins[window]->WindowWidth = max((BaseWidth / COLNO) *
					wins[window]->widest, 
					(BaseWidth / COLNO) * 40);
		wins[window]->WindowHeight = (BaseHeight / ROWNO) *
					wins[window]->cursy;
		wins[window]->nWindowX = 0;
		wins[window]->nWindowY = 0;

		Sprintf(WindowName,"NetHack Text Window %d",window);
		wins[window]->hWnd=CreateWindowEx(
			0,
			NHTextClassName,
			WindowName,
			WS_CHILD|WS_VISIBLE|WS_BORDER,
			wins[window]->nWindowX,
			wins[window]->nWindowY,
			wins[window]->WindowWidth,
			wins[window]->WindowHeight,
			BasehWnd,(HMENU)(int)window,
			wcNetHack.hInstance,
			(LPSTR)0);
		if (!wins[window]->hWnd) {
			status = GetLastError();
			DEBUG_MSG("Deferred init: Init of Window failed");
		}
		wins[window]->active = TRUE;	/* mark it active */
	} 
#ifdef DEBUG_FULL
	else
		DEBUG_MSG("Deferred init: Unrecognized part");
#endif
}

/*
 * -----------------------------------------------------------------------
 * Menu support routines
 * -----------------------------------------------------------------------
 */

void
win32_start_menu(window)
winid window;
{
	int i,status;
	char WindowName[80];

	if (wins[window]->type == TYPE_UNDETERMINED) {
	/* Okay, now we know...its destined to be a menu window */
		strcpy(WindowName,"NetHack Menu");
		wins[window]->type = NHW_MENU;
		wins[window]->hFnt = hDefFnt;
		wins[window]->wflags = 0L;
		wins[window]->maxrows = 52;
		wins[window]->maxcols = COLNO;
		wins[window]->cursy = 0;
		wins[window]->cursx = 0;
		wins[window]->data = (uchar *)0;
		wins[window]->color = (int *)0;
 		wins[window]->hWnd = CreateWindowEx(
			0,
			NHListboxClassName,
			WindowName,
			WS_POPUP | WS_VISIBLE | WS_BORDER,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			(HWND)0,(HMENU)0,
			wcNetHack.hInstance,
			(LPSTR)0);
		if (!wins[window]->hWnd) status = GetLastError();
		else
		/* Add the list box to the window */
			wins[window]->hDlg = CreateWindowEx(
				0,
				"LISTBOX",
				"NetHack List",
LBS_NOTIFY | WS_BORDER | WS_VISIBLE | LBS_WANTKEYBOARDINPUT | LBS_USETABSTOPS,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				wins[window]->hWnd, (HMENU)0,
				wcNetHack.hInstance, (LPSTR)0);
		if (!wins[window]->hDlg) DEBUG_MSG("Problem with CreateDialog");
		MenuWindowCount++;
	} else if (wins[window]->type = NHW_MENU) {
#ifdef DEBUG_FULL
		DEBUG_MSG("Clearing already used menu out");
#endif
		for (i = 0; i < MenuCount[window]; ++i) {
			if (MenuPtr[window][i]) {
				if (MenuPtr[window][i]->str)
				    free((genericptr_t)MenuPtr[window][i]->str);
				free((genericptr_t)MenuPtr[window][i]);
				MenuPtr[window][i] = (struct win32_menuitem *)0;
			}
		}	
		MenuCount[window] = 0;
		SendMessage(wins[window]->hDlg,
				LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	} else
		impossible("Start_menu on window type %d", wins[window]->type);
}

/*
 * Add a text line str to the given menu window.  If identifier
 * is 0, then the line cannot be selected (e.g. a title).
 * Otherwise, identifier is the value returned if the line is
 * selected.  Accelerator (ch) is a keyboard key that can be used
 * to select the line.  If the accelerator of a selectable
 * item is 0, the window system is free to select its own
 * accelerator.  It is up to the window-port to make the
 * accelerator visible to the user (e.g. put "a - " in front
 * of str).  The value attr is the same as in putstr().
 * Glyph is an optional glyph to accompany the line.  If
 * the window port cannot or does not want to display it, this
 * is OK.  If there is no glyph applicable, then this
 * value will be NO_GLYPH.
 * All accelerators should be in the range [A-Za-z].
 * It is expected that callers do not mix accelerator
 * choices.  Either all selectable items have an accelerator
 * or let the window system pick them.  Don't do both.
 */

void
win32_add_menu(window, glyph, identifier, ch, attr, str, preselected)
winid window;
int glyph;                  /* unused (for now) */
const anything *identifier;
char ch;
int attr;
const char *str;
boolean preselected;
{
	struct win32_menuitem *menuitem;

/*	char buf[BUFSZ]; */
	char *tmp;
	char *extra = "? - ";
#ifdef DEBUG_FULL
	DEBUG_MSG("into add_menu");
#endif
	if (wins[window]->type == TYPE_UNDETERMINED) {
		impossible("Trying to add to a non-existant menu window %d",
				window);
		return;
	}
	if (wins[window]->type != NHW_MENU)
		impossible("Window type %d in add_menu", 
				wins[window]->type);
	if (str) {
		tmp = (char *)alloc(strlen(str) + 1 + strlen(extra));
	} else
		tmp = (char *)0;
	menuitem = 
		(struct win32_menuitem *)alloc(sizeof(struct win32_menuitem));
	if (!menuitem) return;
	menuitem->glyph = glyph;
	menuitem->identifier = *identifier;
	menuitem->ch = ch;
	menuitem->attr = attr;
	menuitem->str = tmp;
	if (MenuCount[window] >= MAX_INVENTORY) {
		impossible("Too many inventory items in add_menu (%d)",
				MenuCount[window]);
		return;
	}
	if (menuitem->identifier.a_void) {
	    if (menuitem->str)
		strcpy(menuitem->str, (str != (char *)0) ? str : "Oops!");
	} else 
		Sprintf(menuitem->str,"%s",
			 (str != (char *)0) ? str : "Oops");
	MenuPtr[window][MenuCount[window]] = menuitem;
	++MenuCount[window];
}

/*
 * Stop adding entries to the menu.
 * Prompt is a prompt to give the user.  If prompt is NULL, no prompt will
 * be printed.
 */
#if 0
char mbuf[BUFSZ * 8];
#endif

void
win32_end_menu(window, ch, str, morestr)
winid window;
char ch;
const char *str;
const char *morestr;
{
	int i, j, junk;
	char buf[BUFSZ];
#ifdef DEBUG_FULL
	char buf2[BUFSZ];
#endif
	boolean sflag;

	if (wins[window]->type != NHW_MENU)
		impossible("Window type %d in end_menu", 
				wins[window]->type);
/*	sprintf(buf,"Total items in menu: %d\r", MenuCount[window]); */
	for (i = 0, j = 'a', junk = 0; i < MenuCount[window]; ++i) {
		sflag = FALSE;
		if (MenuPtr[window][i]) {
			if (MenuPtr[window][i]->str) {
				if (MenuPtr[window][i]->identifier.a_void) {
					if (!MenuPtr[window][i]->ch)
						MenuPtr[window][i]->ch = j++;
					Sprintf(buf,"%c - %s",
						MenuPtr[window][i]->ch,
						MenuPtr[window][i]->str);
				} else {
					Sprintf(buf,"\t%s",
						MenuPtr[window][i]->str);
					MenuPtr[window][i]->ch = junk++;
				}
				/* Win32 Listbox control messages */
				SendMessage(wins[window]->hDlg,
					LB_ADDSTRING, (WPARAM)0, 
					(LPARAM)buf);
				SendMessage(wins[window]->hDlg,
					LB_SETITEMDATA, (WPARAM)i,
					(LPARAM)MenuPtr[window][i]->ch);
			}
#ifdef DEBUG_FULL
			else {
				Sprintf(buf2,"Listbox empty string: %d",i);
				DEBUG_MSG(buf2);
			}
#endif
		}
		if (j > 'z') j = 'A';
		if ((j > 'Z') && (j < 'a')) j = '?';
	}	
#ifdef DEBUG_FULL
	DEBUG_MSG(buf);
#endif
	/* TODO: add code to move menu data to the display buffer here */
}

/*
 * Return the number of items selected.  If the value is
 * zero, none were selected.  If items were selected, then
 * selected is filled in with an allocated array of menu_item
 * structures, one for each selected line.  The caller must
 * free this array when done with it.  The "count" field
 * of selected is a user supplied count.  If the user did
 * not supply a count, then the count field is filled with
 * -1 (meaning all).  A count of zero is equivalent to not
 * being selected and should not be in the list.  If no items
 * were selected, then selected is NULL'ed out.  
 *
 * How is the mode of the menu.
 * PICK_NONE	Nothing is selectable,
 * PICK_ONE	Only one thing is selectable
 * PICK_N	Any number valid items may selected 
 * (If how is PICK_NONE, this function should never return anything but 0)
 *
 * You may call select_menu() on a window multiple times --
 * the menu is saved until start_menu() or destroy_nhwindow()
 * is called on the window.
 * Note that NHW_MENU windows need not have select_menu()
 * called for them. There is no way of knowing whether
 * select_menu() will be called for the window at
 * create_nhwindow() time.
 */

int
win32_select_menu(window, how, menu_list)
winid window;
int how;
menu_item **menu_list;
{
/*	win32_menu_item *curr; */
/*	struct menu_info_t *menu_info; */

#if 0
	ShowWindow(wins[window]->hWnd,SW_SHOWDEFAULT);
#endif
	if (wins[window]->type != NHW_MENU)
		impossible("Window type %d in select_menu", 
				wins[window]->type);
	ShowWindow(wins[window]->hWnd,SW_SHOWDEFAULT);
#ifdef DEBUG_FULL
	POP_MESSAGE(mbuf);
#endif
	return 0;
}

#endif /* WIN32_GRAPHICS */
