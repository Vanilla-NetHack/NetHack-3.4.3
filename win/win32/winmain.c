/*	SCCS Id: @(#)winmain.c	3.2	95/09/06	*/
/* Copyright (c) NetHack PC Development Team 1995                 */
/* NetHack may be freely redistributed.  See license for details. */
/* Builds on original MS Windows 3.1.0 Port code by Bill Dyer       */

#include "hack.h"

#ifdef WIN32_GRAPHICS

# ifndef NO_SIGNAL
#include <signal.h>
# endif
#include <ctype.h>
#include <sys\stat.h>
#include "win32api.h"
#include "nhwin32.h"

extern struct window_procs win32_procs;
static int getargs(char *cmdline);
extern int main(int argc, char *argv[]);	/* in sys/share/pcmain.c */

int winmode;
int HaveConsole;
int argc;
int NHnCmdShow;
char *xargv[20];

WNDCLASS wcNetHack;
WNDCLASS wcNHText;
WNDCLASS wcNHPopup;
WNDCLASS wcNHListbox;
char GameName[]="NetHack";
char NHTextClassName[] = "NHTextWin";
char NHPopupClassName[] = "NHPopupWin";
char NHListboxClassName[] = "NHListboxWin";

HINSTANCE NHhPreInst;
HWND BasehWnd;
char winstyle[20];
char NHMenu[]="AboutMenu";

unsigned char *pchBuf;
unsigned char *pchGet;
unsigned char *pchPut;
int pchCount;
int BaseUnits;
int BaseHeight;
int BaseWidth;
int MessageHeight;
int MessageWidth;
int MessageX;
int MessageY;
int MessageCount;
char *MessagePtr[MAX_MESSAGE_COUNT];
int MapHeight;
int MapWidth;
int MapX;
int MapY;
int StatusHeight;
int StatusWidth;
int StatusX;
int StatusY;
int tiles_on;
int DefCharWidth;
int DefCharHeight;
int DefBackGroundColor;
int DefNormalTextColor;
struct win32_menuitem *MenuPtr[MAX_MENU_WINDOWS][MAX_INVENTORY];
int MenuCount[MAX_MENU_WINDOWS];
int MenuWindowCount = 0;
HFONT hDefFnt;
TEXTMETRIC tm;
RECT rcClient;
int inputstatus;
char input_text[BUFSZ];
int input_text_size;
	     
COLORREF colormap[] = {
	RGB(  0,  0,  0),	/* BLACK      */
	RGB(255,  0,  0),	/* RED        */
	RGB(  0,160,  0),	/* GREEN      */
	RGB(200,160, 20),	/* BROWN      */
	RGB(  0,  0,160),	/* BLUE       */
	RGB(160,  0,160),	/* MAGENTA    */
	RGB(  0,160,160),	/* CYAN       */
	RGB(200,200,200),	/* GRAY       */
	RGB(  0,  0,  0),	/* NO_COLOR   */
	RGB(100,255,  0),	/* ORANGE     */
	RGB(  0,255,  0),	/* BR GREEN   */
	RGB(255,255,  0),	/* YELLOW     */
	RGB(  0,  0,255),	/* BR BLUE    */
	RGB(255,  0,255),	/* BR MAGENTA */
	RGB(  0,255,255),	/* BR CYAN    */
	RGB(255,255,255)	/* WHITE      */
};

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPreInst, LPSTR lpszCmdLine,
		   int nCmdShow)
{
	int tmp;

	HaveConsole = 0;
	wcNetHack.hInstance = hInst;
	NHhPreInst = hPreInst; 
	NHnCmdShow = nCmdShow;
	argc = getargs(lpszCmdLine);
	/* temporary */
	if (getenv("WINMODE") != (char *)0) {
		if (strncmpi("WIN32",getenv("WINMODE"),5) == 0)
			winmode = WINMODE_WIN32;
	}
	switch(winmode) {
# ifndef WIN32S
		case WINMODE_TTY:
			strcpy(winstyle,"tty");
			tmp = AllocConsole();
			if (tmp) HaveConsole = 1;
			break;
# endif

		case WINMODE_WIN32:
			strcpy(winstyle,"win32");
			break;
		default:
			strcpy(winstyle,DEFAULT_WINDOW_SYS);
			break;
	}
	choose_windows(winstyle);
	return main(argc, xargv);
}

int
getargs(char *cmdline)
{
	int maxchars;
	int count;
	int argcount = 1;
	int between = 0;
	char *tmpbuf;


	xargv[0] = "nethack.exe";
	if (strlen(cmdline) == 0)
		return argcount;

	tmpbuf = (char *)alloc(strlen(cmdline)+1);
	strcpy(tmpbuf,cmdline);
	
	xargv[argcount] = (char *)&tmpbuf[0];
	++argcount;
	maxchars = strlen(tmpbuf);
	for (count = 0; count < maxchars; ++count)
	{
		if (!between) {
			if (isspace(tmpbuf[count])) {
				between = 1;
 				tmpbuf[count] = '\0';
			}
		} else {
			if (!isspace(tmpbuf[count])) {
				between = 0;
				xargv[argcount] = (char *)&tmpbuf[count];
				++argcount;
			}
		}		
	}
	return argcount;
}

BOOL InitBaseWindow(void)
{
	if (!NHhPreInst) {
		wcNetHack.lpszClassName="NHBaseWin";
		wcNetHack.lpfnWndProc  =BaseWndProc;
		wcNetHack.hCursor      =LoadCursor((HINSTANCE)0,IDC_ARROW);
		wcNetHack.hIcon        =LoadIcon(wcNetHack.hInstance,
					"NETHACK_");
		wcNetHack.lpszMenuName ="NHMenu";
		wcNetHack.hbrBackground=GetStockObject(BLACK_BRUSH);	
		wcNetHack.style        =CS_HREDRAW|CS_VREDRAW;
		wcNetHack.cbClsExtra   =0;
		wcNetHack.cbWndExtra   =0;
		if (!RegisterClass (&wcNetHack)) {
			return FALSE;
		}
	}
	BasehWnd = CreateWindow("NHBaseWin",
				"NetHack 3.1",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				(HWND)0,
				(HMENU)0,
				wcNetHack.hInstance,
				(LPSTR)0);
	return 1;
}

BOOL InitTextWindow(void)
{
	ATOM status;

	wcNHText.hInstance    =wcNetHack.hInstance;
	wcNHText.lpszClassName=NHTextClassName;
	wcNHText.lpfnWndProc  =TextWndProc;
	wcNHText.hCursor      =LoadCursor((HINSTANCE)0,IDC_ARROW);
	wcNHText.hIcon        =LoadIcon(0,IDI_APPLICATION);
	wcNHText.lpszMenuName =(char *)0;
	wcNHText.hbrBackground=GetStockObject(BLACK_BRUSH);	
	wcNHText.style        =CS_HREDRAW|CS_VREDRAW;
	wcNHText.cbClsExtra   =0;
	wcNHText.cbWndExtra   =0;
	
	status = RegisterClass(&wcNHText);
	if (status == (ATOM)0) return 0;
	else return 1;
}

BOOL InitPopupWindow(void)
{
	ATOM status;

	wcNHPopup.hInstance    =wcNetHack.hInstance;
	wcNHPopup.lpszClassName=NHPopupClassName;
	wcNHPopup.lpfnWndProc  =PopupWndProc;
	wcNHPopup.hCursor      =LoadCursor((HINSTANCE)0,IDC_ARROW);
	wcNHPopup.hIcon        =LoadIcon(0,IDI_APPLICATION);
	wcNHPopup.lpszMenuName =(char *)0;
	wcNHPopup.hbrBackground=GetStockObject(BLACK_BRUSH);	
	wcNHPopup.style        =CS_HREDRAW|CS_VREDRAW;
	wcNHPopup.cbClsExtra   =0;
	wcNHPopup.cbWndExtra   =0;
	
	status = RegisterClass(&wcNHPopup);
	if (status == (ATOM)0) return 0;
	else return 1;
}

BOOL InitListboxWindow(void)
{
	ATOM status;

	wcNHListbox.hInstance    =wcNetHack.hInstance;
	wcNHListbox.lpszClassName=NHListboxClassName;
	wcNHListbox.lpfnWndProc  =ListboxWndProc;
	wcNHListbox.hCursor      =LoadCursor((HINSTANCE)0,IDC_ARROW);
	wcNHListbox.hIcon        =LoadIcon(0,IDI_APPLICATION);
	wcNHListbox.lpszMenuName =(char *)0;
	wcNHListbox.hbrBackground=GetStockObject(WHITE_BRUSH);	
	wcNHListbox.style        =CS_HREDRAW|CS_VREDRAW;
	wcNHListbox.cbClsExtra   =0;
	wcNHListbox.cbWndExtra   =0;
	
	status = RegisterClass(&wcNHListbox);
	if (status == (ATOM)0) return 0;
	else return 1;
}

void
win_win32_init()
{
	nt_kbhit = win32_kbhit;
}

int
win32_kbhit()
{
	return pchCount;
}

/*
 *  Virtual Key translation tables.
 *  (Adopted from the MSDOS port)
 */

#define VKEYPADLO	0x60
#define VKEYPADHI	0x6E

#define VPADKEYS 	(VKEYPADHI - VKEYPADLO + 1)
#define isvkeypad(x)	(VKEYPADLO <= (x) && (x) <= VKEYPADHI)


/*
 * Keypad keys are translated to the normal values below from
 * Windows virtual keys. Shifted keypad keys are translated to the
 * shift values below.
 */
static const struct vkpad {
	uchar normal, shift, cntrl;
} vkeypad[VPADKEYS] = {
			{'i', 'I', C('i')},		/* Ins */
			{'b', 'B', C('b')},		/* 1 */
			{'j', 'J', C('j')},		/* 2 */
			{'n', 'N', C('n')},		/* 3 */
			{'h', 'H', C('h')},		/* 4 */
			{'g', 'g', 'g'},		/* 5 */
			{'l', 'L', C('l')},		/* 6 */
			{'y', 'Y', C('y')},		/* 7 */
			{'k', 'K', C('k')},		/* 8 */
			{'u', 'U', C('u')},		/* 9 */
			{ 0 ,   0,      0},		/* * */
			{'p', 'P', C('p')},		/* + */
			{ 0 ,   0,      0},		/* sep */
			{'m', C('p'), C('p')},		/* - */
			{'.', ':', ':'}			/* Del */
}, vnumpad[VPADKEYS] = {
			{'i', 'I', C('i')},		/* Ins */
			{'1', M('1'), '1'},		/* 1 */
			{'2', M('2'), '2'},		/* 2 */
			{'3', M('3'), '3'},		/* 3 */
			{'4', M('4'), '4'},		/* 4 */
			{'g', 'G', 'g'},		/* 5 */
			{'6', M('6'), '6'},		/* 6 */
			{'7', M('7'), '7'},		/* 7 */
			{'8', M('8'), '8'},		/* 8 */
			{'9', M('9'), '9'},		/* 9 */
			{ 0 ,   0,      0},		/* * */
			{'p', 'P', C('p')},		/* + */
			{ 0 ,   0,      0},		/* sep */
			{'m', C('p'), C('p')},		/* - */
			{'.', ':', ':'}			/* Del */
};

/*
 * Unlike Ctrl-letter, the Alt-letter keystrokes have no specific ASCII
 * meaning unless assigned one by a virtual key conversion table.
 * To interpret Alt-letters, we use a virtual key code table to
 * translate the virtual key code into a letter, then set the "meta"
 * bit for it.
 */
static const char vkmap[] = { 	/* ... */
	'a','b','c','d','e','f','g','h','i','j','k','l','m',
	'n','o','p','q','r','s','t','u','v','w','x','y','z'
};
#define VKLO		0x41
#define invkmap(x)      (VKLO <= (x) && (x) < VKLO + SIZE(vkmap))


#endif /* WIN32_GRAPHICS */
