/*	SCCS Id: @(#)termcap.c	3.0	88/11/20
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include <ctype.h>	/* for isdigit() */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"	/* for ROWNO, COLNO, *HI, *HE, *AS, *AE */

#if !defined(SYSV) || defined(TOS) || defined(UNIXPC)
# ifndef LINT
extern			/* it is defined in libtermlib (libtermcap) */
# endif
	short ospeed;	/* terminal baudrate; used by tputs */
#else
short	ospeed = 0;	/* gets around "not defined" error message */
#endif

static void nocmov();
#ifdef MSDOSCOLOR
static void init_hilite();
#endif /* MSDOSCOLOR */

static char tbuf[512];
static char *HO, *CL, *CE, *UP, *CM, *ND, *XD, *BC, *SO, *SE, *TI, *TE;
static char *VS, *VE, *US, *UE;
static char *MR, *ME;
#if 0
static char *MB, *MD, *MH;
#endif
static int SG;
static char PC = '\0';

#if defined(MSDOS) && !defined(TERMLIB)
static char tgotobuf[20];
#ifdef TOS
#define tgoto(fmt, x, y)	(Sprintf(tgotobuf, fmt, y+' ', x+' '), tgotobuf)
#else
#define tgoto(fmt, x, y)	(Sprintf(tgotobuf, fmt, y+1, x+1), tgotobuf)
#endif
#endif /* MSDOS /**/

void
startup()
{
#ifdef TOS
	HO = "\033H";
	CL = "\033E";		/* the VT52 termcap */
	CE = "\033K";
	UP = "\033A";
	CM = "\033Y%c%c";	/* used with function tgoto() */
	ND = "\033C";
	XD = "\033B";
	BC = "\033D";
	SO = "\033p";
	SE = "\033q";
	HI = "\033p";
	HE = "\033q";
#else
	register char *term;
	register char *tptr;
	char *tbufptr, *pc;
	register int i;

	tptr = (char *) alloc(1024);

	tbufptr = tbuf;
	if(!(term = getenv("TERM")))
# ifdef ANSI_DEFAULT
	{
		HO = "\033[H";
		CL = "\033[2J";		/* the ANSI termcap */
/*		CD = "\033[J"; */
		CE = "\033[K";
		CM = "\033[%i%d;%dH";
		UP = "\033[A";
		ND = "\033[C";
		XD = "\033[B";
		BC = "\033[D";
		HI = SO = "\033[1m";
		US = "\033[4m";
		TI = HE = SE = UE = "\033[0m";
		/* strictly, SE should be 2, and UE should be 24,
		   but we can't trust all ANSI emulators to be
		   that complete.  -3. */
		AS = "\016";
		AE = "\017";
		VS = VE = "";
	} else {
# else
		error("Can't get TERM.");
# endif
	if(!strncmp(term, "5620", 4))
		flags.nonull = 1;	/* this should be a termcap flag */
	if(tgetent(tptr, term) < 1)
		error("Unknown terminal type: %s.", term);
	if(pc = tgetstr("pc", &tbufptr))
		PC = *pc;
#ifdef TERMINFO
	if(!(BC = tgetstr("le", &tbufptr))) {	
#else
	if(!(BC = tgetstr("bc", &tbufptr))) {	
#endif
#if !defined(MINIMAL_TERM) && !defined(HISX)
		if(!tgetflag("bs"))
			error("Terminal must backspace.");
#endif
		BC = tbufptr;
		tbufptr += 2;
		*BC = '\b';
	}
#ifdef MINIMAL_TERM
	HO = NULL;
#else
	HO = tgetstr("ho", &tbufptr);
#endif
	CO = tgetnum("co");
	LI = tgetnum("li");
	if(CO < COLNO || LI < ROWNO+2)
		setclipped();
	if(!(CL = tgetstr("cl", &tbufptr)))
		error("Hack needs CL.");
	ND = tgetstr("nd", &tbufptr);
	if(tgetflag("os"))
		error("Hack can't have OS.");
	CE = tgetstr("ce", &tbufptr);
	UP = tgetstr("up", &tbufptr);
	/* It seems that xd is no longer supported, and we should use
	   a linefeed instead; unfortunately this requires resetting
	   CRMOD, and many output routines will have to be modified
	   slightly. Let's leave that till the next release. */
	XD = tgetstr("xd", &tbufptr);
/* not: 		XD = tgetstr("do", &tbufptr); */
	if(!(CM = tgetstr("cm", &tbufptr))) {
		if(!UP && !HO)
			error("Hack needs CM or UP or HO.");
		Printf("Playing hack on terminals without cm is suspect...\n");
		getret();
	}
	SO = tgetstr("so", &tbufptr);
	SE = tgetstr("se", &tbufptr);
	US = tgetstr("us", &tbufptr);
	UE = tgetstr("ue", &tbufptr);
	SG = tgetnum("sg");	/* -1: not fnd; else # of spaces left by so */
	if(!SO || !SE || (SG > 0)) SO = SE = US = UE = "";
	TI = tgetstr("ti", &tbufptr);
	TE = tgetstr("te", &tbufptr);
	VS = VE = "";
#if 0
	MB = tgetstr("mb", &tbufptr);	/* blink */
	MD = tgetstr("md", &tbufptr);	/* boldface */
	MH = tgetstr("mh", &tbufptr);	/* dim */
#endif
	MR = tgetstr("mr", &tbufptr);	/* reverse */
	ME = tgetstr("me", &tbufptr);

	/* Get rid of padding numbers for HI and HE.  Hope they
	 * aren't really needed!!!  HI and HE are ouputted to the
	 * pager as a string - so how can you send it NULLS???
	 *  -jsb
	 */
	    HI = (char *) alloc((unsigned)(strlen(SO)+1));
	    HE = (char *) alloc((unsigned)(strlen(SE)+1));
	    i = 0;
	    while(isdigit(SO[i])) i++;
	    Strcpy(HI, &SO[i]);
	    i = 0;
	    while(isdigit(SE[i])) i++;
	    Strcpy(HE, &SE[i]);
	AS = tgetstr("as", &tbufptr);
	AE = tgetstr("ae", &tbufptr);
	CD = tgetstr("cd", &tbufptr);
# ifdef ANSI_DEFAULT
	}
# endif
	set_whole_screen();		/* uses LI and CD */
	if(tbufptr-tbuf > sizeof(tbuf)) error("TERMCAP entry too big...\n");
	free((genericptr_t)tptr);
#ifdef MSDOSCOLOR
	init_hilite();
#endif
#endif /* TOS /* */
}

void
start_screen()
{
	xputs(TI);
	xputs(VS);
#ifdef DECRAINBOW
	/* Select normal ASCII and line drawing character sets.
	 */
	if (flags.DECRainbow) {
		xputs("\033(B\033)0");
		if (!AS) {
			AS = "\016";
			AE = "\017";
		}
	}
#endif /* DECRAINBOW */
}

void
end_screen()
{
	clear_screen();
	xputs(VE);
	xputs(TE);
}

/* Cursor movements */

void
curs(x, y)
register int x, y;	/* not xchar: perhaps xchar is unsigned and
			   curx-x would be unsigned as well */
{

	if (y == cury && x == curx)
		return;
	if(!ND && (curx != x || x <= 3)) {	/* Extremely primitive */
		cmov(x, y);			/* bunker!wtm */
		return;
	}
	if(abs(cury-y) <= 3 && abs(curx-x) <= 3)
		nocmov(x, y);
	else if((x <= 3 && abs(cury-y)<= 3) || (!CM && x<abs(curx-x))) {
		(void) putchar('\r');
		curx = 1;
		nocmov(x, y);
	} else if(!CM) {
		nocmov(x, y);
	} else
		cmov(x, y);
}

static void
nocmov(x, y)
{
	if (cury > y) {
		if(UP) {
			while (cury > y) {	/* Go up. */
				xputs(UP);
				cury--;
			}
		} else if(CM) {
			cmov(x, y);
		} else if(HO) {
			home();
			curs(x, y);
		} /* else impossible("..."); */
	} else if (cury < y) {
		if(XD) {
			while(cury < y) {
				xputs(XD);
				cury++;
			}
		} else if(CM) {
			cmov(x, y);
		} else {
			while(cury < y) {
				xputc('\n');
				curx = 1;
				cury++;
			}
		}
	}
	if (curx < x) {		/* Go to the right. */
		if(!ND) cmov(x, y); else	/* bah */
			/* should instead print what is there already */
		while (curx < x) {
			xputs(ND);
			curx++;
		}
	} else if (curx > x) {
		while (curx > x) {	/* Go to the left. */
			xputs(BC);
			curx--;
		}
	}
}

void
cmov(x, y)
register int x, y;
{
	xputs(tgoto(CM, x-1, y-1));
	cury = y;
	curx = x;
}

void
xputc(c)
char c;
{
	(void) fputc(c, stdout);
}

void
xputs(s)
char *s;
{
#if defined(MSDOS) && !defined(TERMLIB)
	(void) fputs(s, stdout);
#else
# ifdef __STDC__
	tputs(s, 1, (int (*)())xputc);
# else
	tputs(s, 1, xputc);
# endif
#endif
}

void
cl_end() {
	if(CE)
		xputs(CE);
	else {	/* no-CE fix - free after Harold Rynes */
		/* this looks terrible, especially on a slow terminal
		   but is better than nothing */
		register int cx = curx, cy = cury;

		while(curx < COLNO) {
			xputc(' ');
			curx++;
		}
		curs(cx, cy);
	}
}

void
clear_screen() {
	xputs(CL);
	home();
}

void
home()
{
	if(HO)
		xputs(HO);
	else if(CM)
		xputs(tgoto(CM, 0, 0));
	else
		curs(1, 1);	/* using UP ... */
	curx = cury = 1;
}

void
standoutbeg()
{
	if(SO) xputs(SO);
}

void
standoutend()
{
	if(SE) xputs(SE);
}

void
revbeg()
{
	if(MR) xputs(MR);
}

#if 0	/* if you need one of these, uncomment it (here and in extern.h) */
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
#endif

void
m_end()
{
	if(ME) xputs(ME);
}

void
backsp()
{
	xputs(BC);
}

void
bell()
{
	if (flags.silent) return;
	(void) putchar('\007');		/* curx does not change */
	(void) fflush(stdout);
}

void
graph_on() {
	if (AS) xputs(AS);
}

void
graph_off() {
	if (AE) xputs(AE);
}

#ifndef MSDOS
static const short tmspc10[] = {		/* from termcap */
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5
};
#endif

void
delay_output() {
	/* delay 50 ms - could also use a 'nap'-system call */
	/* BUG: if the padding character is visible, as it is on the 5620
	   then this looks terrible. */
#ifdef MSDOS
	/* simulate the delay with "cursor here" */
	register int i;
	for (i = 0; i < 3; i++) {
		cmov(curx, cury);
		(void) fflush(stdout);
	}
#else /* MSDOS /**/
	if(!flags.nonull)
#ifdef TERMINFO
		/* cbosgd!cbcephus!pds for SYS V R2 */
# ifdef __STDC__
		tputs("$<50>", 1, (int (*)())xputc);
# else
		tputs("$<50>", 1, xputc);
# endif
#else
		tputs("50", 1, xputs);
#endif

	else if(ospeed > 0 || ospeed < SIZE(tmspc10)) if(CM) {
		/* delay by sending cm(here) an appropriate number of times */
		register int cmlen = strlen(tgoto(CM, curx-1, cury-1));
		register int i = 500 + tmspc10[ospeed]/2;

		while(i > 0) {
			cmov(curx, cury);
			i -= cmlen*tmspc10[ospeed];
		}
	}
#endif /* MSDOS /**/
}

void
cl_eos()			/* free after Robert Viduya */
{				/* must only be called with curx = 1 */

	if(CD)
		xputs(CD);
	else {
		register int cx = curx, cy = cury;
		while(cury <= LI-2) {
			cl_end();
			xputc('\n');
			curx = 1;
			cury++;
		}
		cl_end();
		curs(cx, cy);
	}
}

#ifdef MSDOSCOLOR
/* Sets up highlighting, using ANSI escape sequences, for monsters,
 * objects, and gold (highlight code found in pri.c).
 * The termcap entry for HI (from SO) is scanned to find the background 
 * color. If everything is OK, monsters are displayed in the color
 * used to define HILITE_MONSTER, objects are displayed in the color
 * used to define HILITE_OBJECT, and gold is displayed in the color
 * used to define HILITE_GOLD. -3. */

#define ESC		0x1b
#define NONE		0
#define HIGH_INTENSITY	1
#define BLACK		0
#define RED		1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6
#define WHITE		7

#define HILITE_ATTRIB	HIGH_INTENSITY

static void
init_hilite()
{
	int backg = BLACK, foreg = WHITE, len;
	register int c, color;

	HI_RED = HI_YELLOW = HI_GREEN = HI_BLUE = HI_WHITE = HI;

	/* find the background color, HI[len] == 'm' */
	len = strlen(HI) - 1;

	if (HI[len] != 'm' || len < 3) return;

	c = 2;
	while (c < len) {
	    if ((color = atoi(&HI[c])) == 0) {
		/* this also catches errors */
		foreg = WHITE; backg = BLACK;
	    } else if (color >= 30 && color <= 37) {
		foreg = color - 30;
	    } else if (color >= 40 && color <= 47) {
		backg = color - 40;
	    }
	    while (isdigit(HI[++c]));
	    c++;
	}

	/* avoid invisibility */
	if (foreg != RED && backg != RED) {
	    HI_RED = (char *) alloc(sizeof("E[0;33;44;54m"));
	    Sprintf(HI_RED, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB,
		    RED, backg);
	}

	if (foreg != YELLOW && backg != YELLOW) {
	    HI_YELLOW = (char *) alloc(sizeof("E[0;33;44;54m"));
	    Sprintf(HI_YELLOW, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB,
		    YELLOW, backg);
	}

	if (foreg != GREEN && backg != GREEN) {
	    HI_GREEN = (char *) alloc(sizeof("E[0;33;44;54m"));
	    Sprintf(HI_GREEN, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB,
		    GREEN, backg);
	}

	if (foreg != BLUE && backg != BLUE) {
	    HI_BLUE = (char *) alloc(sizeof("E[0;33;44;54m"));
	    Sprintf(HI_BLUE, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB,
		    BLUE, backg);
	}

	if (foreg != WHITE && backg != WHITE) {
	    HI_WHITE = (char *) alloc(sizeof("E[0;33;44;54m"));
	    Sprintf(HI_WHITE, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB,
		    WHITE, backg);
	}
}

#endif
