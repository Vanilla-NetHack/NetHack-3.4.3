/*	SCCS Id: @(#)nttty.c	3.1	90/22/02
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* tty.c - (Windows NT) version */
/*                                                  
 * Initial Creation - Michael Allison January 31/93 
 *
 */

#ifdef WIN32CON

#define NEED_VARARGS /* Uses ... */	/* comment line for pre-compiled headers */
#include "hack.h"
#include "wintty.h"
#include "termcap.h"
#include <windows.h>
#include <wincon.h>
#include <sys\types.h>
#include <sys\stat.h>
/* #include <conio.h> */

void FDECL(cmov, (int, int));
void FDECL(nocmov, (int, int));
void FDECL(xputspecl, (char *));
void FDECL(xputcolor, (char));

/*
 * The following WIN32 Console API routines are used in this file.
 *
 * CreateFile
 * GetConsoleScreenBufferInfo
 * GetStdHandle
 * SetConsoleCursorPosition
 * SetConsoleTextAttribute
 * PeekConsoleInput
 * ReadConsoleInput
 * WriteConsoleOutputCharacter
 */

/* Win32 Console handles for input and output */
HANDLE hConIn;
HANDLE hConOut;

/* Win32 Screen buffer,coordinate,console I/O information */
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD ntcoord;
INPUT_RECORD ir;

/* Flag for whether NetHack was launched via progman, not command line.
 * The reason we care at all, is so that we can get
 * a final RETURN at the end of the game when launched from progman
 * to prevent the scoreboard (or panic message :-| from vanishing
 * immediately after it is displayed, yet not bother when started
 * from the command line. 
 */
int ProgmanLaunched;

	/* (see termcap.h) -- CM, ND, CD, HI,HE, US,UE, ul_hack */
struct tc_lcl_data tc_lcl_data = { 0, 0, 0, 0,0, 0,0, FALSE };
STATIC_VAR char *HO, *CL, *CE, *UP, *XD, *BC, *SO, *SE, *TI, *TE;
STATIC_VAR char *VS, *VE;

#ifdef TEXTCOLOR
char *hilites[MAXCOLORS]; /* terminal escapes for the various colors */
static void NDECL(init_hilite);
STATIC_VAR char *MD;
#endif

static char *KS = NULL, *KE = NULL;	/* keypad sequences */
static char nullstr[] = "";
char erase_char,kill_char;

/* STATIC_VAR char tgotobuf[20]; */
/* #define tgoto(fmt, x, y)	(Sprintf(tgotobuf, fmt, y+1, x+1), tgotobuf) */
#define tgoto(fmt, x, y) gotoxy(x,y)
/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty(){
	register int i;

	erase_char = '\b';
	kill_char = 21;		/* cntl-U */
	flags.cbreak = TRUE;
	TI = VS = VE = TE = nullstr;
	HO = "\033\001";
	CL = "\033\002";		/* the VT52 termcap */
	CE = "\033\003";
	UP = "\033\004";
	CM = "\033\005";	/* used with function tgoto() */
	ND = "\033\006";
	XD = "\033\007";
	BC = "\033\010";
	SO = "\033\011";
	SE = "\033\012";
	/* HI and HE will be updated in init_hilite if we're using color */
	HI = "\033\013";
	HE = "\033\014";
# ifdef TEXTCOLOR
	for (i = 0; i < MAXCOLORS / 2; i++)
	{
	     hilites[i|BRIGHT] = (char *) alloc(sizeof("\033\015\001"));
	     hilites[i] = (char *) alloc(sizeof("\033\015\001"));
	}
	init_hilite();
# endif
}

/* reset terminal to original state */
void
settty(s)
const char *s;
{
	end_screen();
	if(s) raw_print(s);
}

/* called by init_nhwindows() and resume_nhwindows() */
void
setftty()
{
	start_screen();
}

int
tgetch()
{
	int ch;
	int valid = 0;
	int metaflags = 0;
	int count;

	valid = 0;
	while (!valid)
	{
	    ReadConsoleInput(hConIn,&ir,1,&count);
	    /* We only care about ascii press-down KEY_EVENTs */
	    if ((ir.EventType == KEY_EVENT) &&
	       (ir.Event.KeyEvent.bKeyDown) &&	
	       (ir.Event.KeyEvent.uChar.AsciiChar)) valid = 1;
	}
	metaflags = ir.Event.KeyEvent.dwControlKeyState;
	/*
	 * metaflags can be checked to see if various special
         * keys were pressed at the same time as the key.
         * Currently we are using the ALT keys only.
	 *
	 *           RIGHT_ALT_PRESSED, LEFT_ALT_PRESSED,
         *           RIGHT_CTRL_PRESSED, LEFT_CTRL_PRESSED,
         *           SHIFT_PRESSED,NUMLOCK_ON, SCROLLLOCK_ON,
         *           CAPSLOCK_ON, ENHANCED_KEY
         *
	 * are all valid bit masks to use on metaflags
         * eg. (metaflags & LEFT_CTRL_PRESSED) is true if the
         *      left control key was pressed with the keystroke.
         */
	ch = (ir.Event.KeyEvent.uChar.AsciiChar == '\r') ?
			 '\n' :ir.Event.KeyEvent.uChar.AsciiChar;
	if (metaflags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
	   ch = tolower(ch) | 0x080;
	return ch;
}

int
kbhit()
{
	int done = 0;	/* true =  "stop searching"        */
	int retval;	/* true =  "we had a match"        */
	int count;	/* scratch-pad area for API call   */

	done = 0;
	retval = 0;
	while (!done)
	{
	    count = 0;
	    PeekConsoleInput(hConIn,&ir,1,&count);
	    if (count > 0)
            {
		/* Make sure its an ascii press-down KEY_EVENT */
	        if ((ir.EventType == KEY_EVENT) &&
	        (ir.Event.KeyEvent.bKeyDown) &&	
	        (ir.Event.KeyEvent.uChar.AsciiChar))
	        { 
		     /* This is what we were looking for        */
		     done = 1;	        /* Stop looking         */
		     retval = 1;        /* Found what we sought */
	        }
		/* Discard it, its an insignificant event */
	        else ReadConsoleInput(hConIn,&ir,1,&count);
	    }
	    else  /* There are no events in console event queue*/
	    {
		done = 1;	  /* Stop looking               */
		retval = 0;	  /* No ascii press-down key    */
	    }
	}
	return retval;
}

/* called by init_tty in wintty.c for WIN32CON port only */
void
nttty_open()
{
        HANDLE hStdOut;

        /* The following 6 lines of code were suggested by 
         * Bob Landau of Microsoft WIN32 Developer support,
         * as the only current means of determining whether
         * we were launched from the command prompt, or from
         * the NT program manager. M. Allison
         */
        hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
        GetConsoleScreenBufferInfo( hStdOut, &csbi);
        ProgmanLaunched = ((csbi.dwCursorPosition.X == 0) &&
                           (csbi.dwCursorPosition.Y == 0));
        if ((csbi.dwSize.X <= 0) || (csbi.dwSize.Y <= 0))
            ProgmanLaunched = 0;


        /* Obtain handles for the standard Console I/O devices */
	hConIn = CreateFile("CONIN$",
			GENERIC_READ |GENERIC_WRITE,
			FILE_SHARE_READ |FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);					

	hConOut = CreateFile("CONOUT$",
			GENERIC_READ |GENERIC_WRITE,
			FILE_SHARE_READ |FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);					
	get_scr_size();
}

void
gotoxy(x,y)
int x,y;
{
	COORD gtcoord;

	x--; y--;			/* (0,0) is upper right corner */
	gtcoord.X = x;
	gtcoord.Y = y;
	SetConsoleCursorPosition(hConOut, gtcoord);
}

void
get_scr_size()
{
	if (GetConsoleScreenBufferInfo(hConOut,&csbi))
	{
	    LI = csbi.dwSize.Y;
	    CO = csbi.dwSize.X;
	}
	else
	{	
		LI = 25;
		CO = 80;
	}
}

void
nttty_rubout()
{
	DWORD count;

	GetConsoleScreenBufferInfo(hConOut,&csbi);
	if (csbi.dwCursorPosition.X > 0)
		ntcoord.X = csbi.dwCursorPosition.X-1;
	ntcoord.Y = csbi.dwCursorPosition.Y;
	WriteConsoleOutputCharacter(hConOut,' ',1,ntcoord,&count);
	SetConsoleCursorPosition(hConOut,ntcoord);	
}

/* fatal error */
/*VARARGS1*/

void
error VA_DECL(const char *,s)
	VA_START(s);
	VA_INIT(s, const char *);
	/* error() may get called before tty is initialized */
	if (flags.window_inited) end_screen();
	putchar('\n');
	Vprintf(s,VA_ARGS);
	putchar('\n');
	VA_END();
	exit(1);
}


void
tty_startup(wid, hgt)
    int *wid, *hgt;
{
	register int i;

	*wid = CO;
	*hgt = LI;
}

void
tty_number_pad(state)
int state;
{
}

void
tty_start_screen()
{
/*	xputs(TI);
 *	xputs(VS);
 */
	if (flags.num_pad) tty_number_pad(1);	/* make keypad send digits */
}

void
tty_end_screen()
{
	clear_screen();
}

/* Cursor movements */

void
nocmov(x, y)
int x,y;
{
	ntcoord.X = x;
	ntcoord.Y = y;
	SetConsoleCursorPosition(hConOut,ntcoord);
}

void
cmov(x, y)
register int x, y;
{
	ntcoord.X = x;
	ntcoord.Y = y;
	SetConsoleCursorPosition(hConOut,ntcoord);
	ttyDisplay->cury = y;
	ttyDisplay->curx = x;
}

void
xputc(c)
char c;
{
	int count;

	ntcoord.X = ttyDisplay->curx;
	ntcoord.Y = ttyDisplay->cury;
	WriteConsoleOutputCharacter(hConOut,&c,1,ntcoord,&count);
}

void
xputs(s)
const char *s;
{
	int count;
	if (s[0]=='\033')xputspecl(s);
	else
	{
		ntcoord.X = ttyDisplay->curx;
		ntcoord.Y = ttyDisplay->cury;
		WriteConsoleOutputCharacter(hConOut,s,
			strlen(s),ntcoord,&count);
	}
}
void
cl_end()
{
		int count;

		ntcoord.X = ttyDisplay->curx;
		ntcoord.Y = ttyDisplay->cury;
		FillConsoleOutputCharacter(hConOut,' ',
			CO - ntcoord.X,ntcoord,&count);
		tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
}


void
clear_screen()
{
	int count;

	ntcoord.X = 0;
	ntcoord.Y = 0;
	FillConsoleOutputCharacter(hConOut,' ',CO * LI,
		ntcoord, &count);
	home();
}


void
home()
{
	tty_curs(BASE_WINDOW, 1, 0);	/* using UP ... */
	ttyDisplay->curx = ttyDisplay->cury = 0;
}

void
standoutbeg()
{
	/* Mix all three colors for white */
	SetConsoleTextAttribute(hConOut,
		FOREGROUND_RED|FOREGROUND_BLUE|
		FOREGROUND_GREEN|FOREGROUND_INTENSITY );
}

void
standoutend()
{
	/* Mix all three colors for white */
	SetConsoleTextAttribute(hConOut,
		FOREGROUND_RED|FOREGROUND_BLUE|
		FOREGROUND_GREEN);
}

#if 0	/* if you need one of these, uncomment it (here and in extern.h) */
void
revbeg()
{
	if(MR) xputs(MR);
}

void
boldbeg()
{
	if(MD) xputs(MD);
}

void
blinkbeg()
{
	if(MB) xputs(MB);
}

void
dimbeg()
/* not in most termcap entries */
{
	if(MH) xputs(MH);
}

void
m_end()
{
	if(ME) xputs(ME);
}
#endif /* 0              */


void
backsp()
{
	nttty_rubout();
}

void
tty_nhbell()
{
	if (flags.silent) return;
	Beep(8000,500);
}


void
tty_delay_output()
{
	/* delay 50 ms - could also use a 'nap'-system call */
	/* BUG: if the padding character is visible, as it is on the 5620
	   then this looks terrible. */
}

void
cl_eos()			/* free after Robert Viduya */
{				/* must only be called with curx = 1 */

		register int cy = ttyDisplay->cury+1;
		while(cy <= LI-2) {
			cl_end();
			xputc('\n');
			cy++;
		}
		cl_end();
		tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
}

/* Because the tty port assumes that hilites etc.
   can be done by blasting sequences to the screen
   and code to do that is scattered about, this
   routine will parse our special versions of those
   sequences to ensure compatibility.
*/
void
xputspecl(char *x)
{
      switch (x[1])
      {
	case 1:		/* HO */
		home();
		break;
	case 2:		/* CL */
	case 3:		/* CE */
	case 4:		/* UP */
	case 5:		/* CM */
	case 6:		/* ND */
	case 7:		/* XD */
	case 8:		/* BC */
	case 9:		/* SO */
	case 10:	/* SE */
		impossible("Unexpected termcap usage under NT");
		break;
	case 11:	/* HI */
		standoutbeg();
		break;
	case 12:	/* HE */
		standoutend();
		break;
#ifdef TEXTCOLOR
	case 13:
		xputcolor(x[2]);
		break;
#endif
	default:
		impossible("bad escape sequence");
      }
}

#ifdef TEXTCOLOR
/*
 * BLACK		0
 * RED			1
 * GREEN		2
 * BROWN		3	low-intensity yellow
 * BLUE			4
 * MAGENTA 		5
 * CYAN			6
 * GRAY			7	low-intensity white
 * NO_COLOR		8
 * ORANGE_COLORED	9
 * BRIGHT_GREEN		10
 * YELLOW		11
 * BRIGHT_BLUE		12
 * BRIGHT_MAGENTA  	13
 * BRIGHT_CYAN		14
 * WHITE		15
 * MAXCOLORS		16
 * BRIGHT		8
 */

void
xputcolor(char x)
{
	WORD attr;

	if ((x >= 0) && (x <= FOREGROUND_GREEN|FOREGROUND_BLUE|
			 FOREGROUND_RED|FOREGROUND_INTENSITY)) {
		attr = x;
		SetConsoleTextAttribute(hConOut,attr);
	}
	else impossible("xputcolor: bad color value");
}

#define CMAP(a,b) Sprintf(hilites[a],"\033\015%c",b)
static void
init_hilite()
{
	CMAP(BLACK,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED);
	CMAP(RED,FOREGROUND_RED);
	CMAP(GREEN,FOREGROUND_GREEN);
	CMAP(BROWN,FOREGROUND_GREEN|FOREGROUND_RED);
	CMAP(BLUE,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	CMAP(MAGENTA,FOREGROUND_BLUE|FOREGROUND_RED);
	CMAP(CYAN,FOREGROUND_GREEN|FOREGROUND_BLUE);
	CMAP(GRAY,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE);
	CMAP(BRIGHT,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
	CMAP(ORANGE_COLORED,FOREGROUND_RED|FOREGROUND_INTENSITY);
	CMAP(BRIGHT_GREEN,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	CMAP(YELLOW,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
	CMAP(BRIGHT_BLUE,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	CMAP(BRIGHT_MAGENTA,FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
	CMAP(BRIGHT_CYAN,FOREGROUND_GREEN|FOREGROUND_BLUE);
	CMAP(WHITE,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
}

#endif /* TEXTCOLOR */

#endif /* WIN32CON */
