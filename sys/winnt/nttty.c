/*	SCCS Id: @(#)nttty.c	3.2	96/10/21
/* Copyright (c) NetHack PC Development Team 1993    */
/* NetHack may be freely redistributed.  See license for details. */

/* tty.c - (Windows NT) version */
/*                                                  
 * Initial Creation 				M. Allison	93/01/31 
 *
 */

#ifdef WIN32CON
#define NEED_VARARGS /* Uses ... */
#include "hack.h"
#include "wintty.h"
#include <sys\types.h>
#include <sys\stat.h>
#include "win32api.h"
#include <wincon.h>

void FDECL(cmov, (int, int));
void FDECL(nocmov, (int, int));

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
 * WriteConsole
 */

/* Win32 Console handles for input and output */
HANDLE hConIn;
HANDLE hConOut;

/* Win32 Screen buffer,coordinate,console I/O information */
CONSOLE_SCREEN_BUFFER_INFO csbi;
COORD ntcoord;
INPUT_RECORD ir;

/* Flag for whether NetHack was launched via the GUI, not the command line.
 * The reason we care at all, is so that we can get
 * a final RETURN at the end of the game when launched from the GUI
 * to prevent the scoreboard (or panic message :-|) from vanishing
 * immediately after it is displayed, yet not bother when started
 * from the command line. 
 */
int GUILaunched;

# ifdef TEXTCOLOR
int ttycolors[CLR_MAX];
static void NDECL(init_ttycolor);
# endif

static char nullstr[] = "";
char erase_char,kill_char;

/*
 * Called after returning from ! or ^Z
 */
void
gettty(){

	erase_char = '\b';
	kill_char = 21;		/* cntl-U */
	iflags.cbreak = TRUE;
#ifdef TEXTCOLOR
	init_ttycolor();
#endif
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

/*
 *  Keyboard translation tables.
 *  (Adopted from the MSDOS port)
 */

#define KEYPADLO	0x47
#define KEYPADHI	0x53

#define PADKEYS 	(KEYPADHI - KEYPADLO + 1)
#define iskeypad(x)	(KEYPADLO <= (x) && (x) <= KEYPADHI)

/*
 * Keypad keys are translated to the normal values below.
 * Shifted keypad keys are translated to the
 *    shift values below.
 */

static const struct pad {
	uchar normal, shift, cntrl;
} keypad[PADKEYS] = {
			{'y', 'Y', C('y')},		/* 7 */
			{'k', 'K', C('k')},		/* 8 */
			{'u', 'U', C('u')},		/* 9 */
			{'m', C('p'), C('p')},		/* - */
			{'h', 'H', C('h')},		/* 4 */
			{'g', 'g', 'g'},		/* 5 */
			{'l', 'L', C('l')},		/* 6 */
			{'p', 'P', C('p')},		/* + */
			{'b', 'B', C('b')},		/* 1 */
			{'j', 'J', C('j')},		/* 2 */
			{'n', 'N', C('n')},		/* 3 */
			{'i', 'I', C('i')},		/* Ins */
			{'.', ':', ':'}			/* Del */
}, numpad[PADKEYS] = {
			{'7', M('7'), '7'},		/* 7 */
			{'8', M('8'), '8'},		/* 8 */
			{'9', M('9'), '9'},		/* 9 */
			{'m', C('p'), C('p')},		/* - */
			{'4', M('4'), '4'},		/* 4 */
			{'g', 'G', 'g'},		/* 5 */
			{'6', M('6'), '6'},		/* 6 */
			{'p', 'P', C('p')},		/* + */
			{'1', M('1'), '1'},		/* 1 */
			{'2', M('2'), '2'},		/* 2 */
			{'3', M('3'), '3'},		/* 3 */
			{'i', 'I', C('i')},		/* Ins */
			{'.', ':', ':'}			/* Del */
};
/*
 * Unlike Ctrl-letter, the Alt-letter keystrokes have no specific ASCII
 * meaning unless assigned one by a keyboard conversion table
 * To interpret Alt-letters, we use a
 * scan code table to translate the scan code into a letter, then set the
 * "meta" bit for it.  -3.
 */
#define SCANLO		0x10

static const char scanmap[] = { 	/* ... */
	'q','w','e','r','t','y','u','i','o','p','[',']', '\n',
	0, 'a','s','d','f','g','h','j','k','l',';','\'', '`',
	0, '\\', 'z','x','c','v','b','n','m',',','.','?'	/* ... */
};

#define inmap(x)	(SCANLO <= (x) && (x) < SCANLO + SIZE(scanmap))

int
tgetch()
{
	int valid = 0;
	int metaflags = 0;
	int count;
	unsigned short int scan;
	unsigned char ch;
	unsigned long shiftstate;
	int altseq;
	const struct pad *kpad;

	shiftstate = 0L;
	valid = 0;
	while (!valid)
	{
	   ReadConsoleInput(hConIn,&ir,1,&count);
	   if ((ir.EventType == KEY_EVENT) && ir.Event.KeyEvent.bKeyDown) {
		ch    = ir.Event.KeyEvent.uChar.AsciiChar;
		scan  = ir.Event.KeyEvent.wVirtualScanCode;
		shiftstate = ir.Event.KeyEvent.dwControlKeyState;
		altseq=(shiftstate & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) && inmap(scan);
		if (ch || (iskeypad(scan)) || altseq)
			valid = 1;
	   }
	}
	if (valid) {
    	    /*
	    * shiftstate can be checked to see if various special
            * keys were pressed at the same time as the key.
            * Currently we are using the ALT & SHIFT & CONTROLS.
            *
            *           RIGHT_ALT_PRESSED, LEFT_ALT_PRESSED,
            *           RIGHT_CTRL_PRESSED, LEFT_CTRL_PRESSED,
            *           SHIFT_PRESSED,NUMLOCK_ON, SCROLLLOCK_ON,
            *           CAPSLOCK_ON, ENHANCED_KEY
            *
            * are all valid bit masks to use on shiftstate.
            * eg. (shiftstate & LEFT_CTRL_PRESSED) is true if the
            *      left control key was pressed with the keystroke.
            */
            if (iskeypad(scan)) {
                kpad = iflags.num_pad ? numpad : keypad;
                if (shiftstate & SHIFT_PRESSED) {
                    ch = kpad[scan - KEYPADLO].shift;
                }
                else if (shiftstate & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
                    ch = kpad[scan - KEYPADLO].cntrl;
                }
                else {
                    ch = kpad[scan - KEYPADLO].normal;
                }
            }
            else if (shiftstate & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) { /* ALT sequence */
                    if (inmap(scan))
                        ch = scanmap[scan - SCANLO];
                    ch = (isprint(ch) ? M(ch) : ch);
            }
            return (ch == '\r') ? '\n' : ch;
	}
}

int
nttty_kbhit()
{
	int done = 0;	/* true =  "stop searching"        */
	int retval;	/* true =  "we had a match"        */
	int count;
	unsigned short int scan;
	unsigned char ch;
	unsigned long shiftstate;
	int altseq;
        
	done = 0;
	retval = 0;
	while (!done)
	{
	    count = 0;
	    PeekConsoleInput(hConIn,&ir,1,&count);
	    if (count > 0) {
		ch    = ir.Event.KeyEvent.uChar.AsciiChar;
		scan  = ir.Event.KeyEvent.wVirtualScanCode;
		shiftstate = ir.Event.KeyEvent.dwControlKeyState;
		altseq=(shiftstate & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) && inmap(scan);
		if (((ir.EventType == KEY_EVENT) && ir.Event.KeyEvent.bKeyDown) &&
		     (ch || (iskeypad(scan)) || altseq)) {
			done = 1;	    /* Stop looking         */
			retval = 1;         /* Found what we sought */
		}
		else /* Discard it, its an insignificant event */
			ReadConsoleInput(hConIn,&ir,1,&count);
		} else  /* There are no events in console event queue */ {
		done = 1;	  /* Stop looking               */
		retval = 0;
	    }
	}
	return retval;
}

/* called by init_tty in wintty.c for WIN32CON port only */
void
nttty_open()
{
        HANDLE hStdOut;
        long cmode;
        long mask;
        
	/* Initialize the function pointer that points to
         * the kbhit() equivalent, in this TTY case nttty_kbhit()
         */

	nt_kbhit = nttty_kbhit;

        /* The following 6 lines of code were suggested by 
         * Bob Landau of Microsoft WIN32 Developer support,
         * as the only current means of determining whether
         * we were launched from the command prompt, or from
         * the NT program manager. M. Allison
         */
        hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
        GetConsoleScreenBufferInfo( hStdOut, &csbi);
        GUILaunched = ((csbi.dwCursorPosition.X == 0) &&
                           (csbi.dwCursorPosition.Y == 0));
        if ((csbi.dwSize.X <= 0) || (csbi.dwSize.Y <= 0))
            GUILaunched = 0;


        /* Obtain handles for the standard Console I/O devices */
	hConIn = CreateFile("CONIN$",
			GENERIC_READ |GENERIC_WRITE,
			FILE_SHARE_READ |FILE_SHARE_WRITE,
			0, OPEN_EXISTING, 0, 0);					
	GetConsoleMode(hConIn,&cmode);
	mask = ENABLE_PROCESSED_INPUT | ENABLE_LINE_INPUT |
	       ENABLE_MOUSE_INPUT | ENABLE_ECHO_INPUT | ENABLE_WINDOW_INPUT;   
	cmode &= ~mask;
	SetConsoleMode(hConIn,cmode);
	
	hConOut = CreateFile("CONOUT$",
			GENERIC_READ |GENERIC_WRITE,
			FILE_SHARE_READ |FILE_SHARE_WRITE,
			0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);					        
        
	get_scr_size();
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


/* fatal error */
/*VARARGS1*/

void
error VA_DECL(const char *,s)
	VA_START(s);
	VA_INIT(s, const char *);
	/* error() may get called before tty is initialized */
	if (iflags.window_inited) end_screen();
	putchar('\n');
	Vprintf(s,VA_ARGS);
	putchar('\n');
	VA_END();
	exit(EXIT_FAILURE);
}


void
tty_startup(wid, hgt)
    int *wid, *hgt;
{

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
	if (iflags.num_pad) tty_number_pad(1);	/* make keypad send digits */
}

void
tty_end_screen()
{
	clear_screen();
}

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

	WriteConsole(hConOut,&c,1,&count,0);
}

void
xputs(s)
const char *s;
{
	int count;
	
	WriteConsole(hConOut,s,strlen(s),&count,0);
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
	tty_curs(BASE_WINDOW, 1, 0);
	ttyDisplay->curx = ttyDisplay->cury = 0;
}


void
backsp()
{
	DWORD count;

	GetConsoleScreenBufferInfo(hConOut,&csbi);
	if (csbi.dwCursorPosition.X > 0)
		ntcoord.X = csbi.dwCursorPosition.X-1;
	ntcoord.Y = csbi.dwCursorPosition.Y;
	SetConsoleCursorPosition(hConOut,ntcoord);	
	WriteConsole(hConOut," ",1,&count,0);
	SetConsoleCursorPosition(hConOut,ntcoord);	
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
	/* delay 50 ms - uses ANSI C clock() function now */
	clock_t goal;

	goal = 50 + clock();
	while (goal > clock()) {
	    /* Do nothing */
	}
}

void
cl_eos()
{

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


# ifdef TEXTCOLOR
/*
 * CLR_BLACK		0
 * CLR_RED		1
 * CLR_GREEN		2
 * CLR_BROWN		3	low-intensity yellow
 * CLR_BLUE		4
 * CLR_MAGENTA 		5
 * CLR_CYAN		6
 * CLR_GRAY		7	low-intensity white
 * NO_COLOR		8
 * CLR_ORANGE		9
 * CLR_BRIGHT_GREEN	10
 * CLR_YELLOW		11
 * CLR_BRIGHT_BLUE	12
 * CLR_BRIGHT_MAGENTA  	13
 * CLR_BRIGHT_CYAN	14
 * CLR_WHITE		15
 * CLR_MAX		16
 * BRIGHT		8
 */

static void
init_ttycolor()
{
	ttycolors[CLR_BLACK] = FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED;
	ttycolors[CLR_RED] = FOREGROUND_RED;
	ttycolors[CLR_GREEN] = FOREGROUND_GREEN;
	ttycolors[CLR_BROWN] = FOREGROUND_GREEN|FOREGROUND_RED;
	ttycolors[CLR_BLUE] = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
	ttycolors[CLR_MAGENTA] = FOREGROUND_BLUE|FOREGROUND_RED;
	ttycolors[CLR_CYAN] = FOREGROUND_GREEN|FOREGROUND_BLUE;
	ttycolors[CLR_GRAY] = FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE;
	ttycolors[BRIGHT] = FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED|\
						FOREGROUND_INTENSITY;
	ttycolors[CLR_ORANGE] = FOREGROUND_RED|FOREGROUND_INTENSITY;
	ttycolors[CLR_BRIGHT_GREEN] = FOREGROUND_GREEN|FOREGROUND_INTENSITY;
	ttycolors[CLR_YELLOW] = FOREGROUND_GREEN|FOREGROUND_RED|\
						FOREGROUND_INTENSITY;
	ttycolors[CLR_BRIGHT_BLUE] = FOREGROUND_BLUE|FOREGROUND_INTENSITY;
	ttycolors[CLR_BRIGHT_MAGENTA] = FOREGROUND_BLUE|FOREGROUND_RED|\
						FOREGROUND_INTENSITY;
	ttycolors[CLR_BRIGHT_CYAN] = FOREGROUND_GREEN|FOREGROUND_BLUE;
	ttycolors[CLR_WHITE] = FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED|\
						FOREGROUND_INTENSITY;
}

# endif /* TEXTCOLOR */

int
has_color(int color)
{
# ifdef TEXTCOLOR
    return 1;
# else
    return 0;
# endif 
}

void
term_end_attr(int attr)
{
	/* Mix all three colors for white on NT console */
	SetConsoleTextAttribute(hConOut,
		FOREGROUND_RED|FOREGROUND_BLUE|
		FOREGROUND_GREEN);    
}

void
term_start_attr(int attr)
{
    switch(attr){

        case ATR_ULINE:
        case ATR_BOLD:
    	        /* Mix all three colors for white on NT console */
	        SetConsoleTextAttribute(hConOut,
		    FOREGROUND_RED|FOREGROUND_BLUE|
		    FOREGROUND_GREEN|FOREGROUND_INTENSITY );
                break;
        case ATR_BLINK:
        case ATR_INVERSE:
        default:
                term_end_attr(0);
                break;
    }                
}

void
term_end_raw_bold(void)
{
    standoutend();    
}

void
term_start_raw_bold(void)
{
    standoutbeg();
}

void
term_start_color(int color)
{
# ifdef TEXTCOLOR
	WORD attr;

        if (color >= 0 && color < CLR_MAX) {
            attr = (WORD)ttycolors[color];
	    SetConsoleTextAttribute(hConOut,attr);
	}
# endif
}

void
term_end_color(void)
{
# ifdef TEXTCOLOR
	SetConsoleTextAttribute(hConOut,
		FOREGROUND_RED|FOREGROUND_BLUE|
		FOREGROUND_GREEN);
# endif       
}


void
standoutbeg()
{
	/* Mix all three colors for white on NT console */
	SetConsoleTextAttribute(hConOut,
		FOREGROUND_RED|FOREGROUND_BLUE|
		FOREGROUND_GREEN|FOREGROUND_INTENSITY );
}


void
standoutend()
{
	/* Mix all three colors for white on NT console */
	SetConsoleTextAttribute(hConOut,
		FOREGROUND_RED|FOREGROUND_BLUE|
		FOREGROUND_GREEN);
}

#endif /* WIN32CON */
