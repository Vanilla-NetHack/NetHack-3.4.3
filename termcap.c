/*	SCCS Id: @(#)termcap.c	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include <stdio.h>
#include <ctype.h>	/* for isdigit() */
#include "hack.h"	/* for ROWNO, COLNO, *HI, *HE */
#ifdef GENIX
#define	void	int	/* jhn - mod to prevent compiler from bombing */
#endif

extern char *tgetstr(), *tgoto(), *getenv();
extern long *alloc();

#ifndef SYSV
# ifndef LINT
extern			/* it is defined in libtermlib (libtermcap) */
# endif
	short ospeed;	/* terminal baudrate; used by tputs */
#else
short	ospeed = 0;	/* gets around "not defined" error message */
#endif

static char tbuf[512];
static char *HO, *CL, *CE, *UP, *CM, *ND, *XD, *BC, *SO, *SE, *TI, *TE;
static char *VS, *VE, *US, *UE;
static int SG;
static char PC = '\0';
char *CD;		/* tested in pri.c: docorner() */
int CO, LI;		/* used in pri.c and whatis.c */

#if defined(MSDOS) && !defined(TERMLIB)
static char tgotobuf[20];
#define tgoto(fmt, x, y)	(sprintf(tgotobuf, fmt, y+1, x+1), tgotobuf)
#endif /* MSDOS /**/

startup()
{
	register char *term;
	register char *tptr;
	char *tbufptr, *pc;
	register int i;

	tptr = (char *) alloc(1024);

	tbufptr = tbuf;
	if(!(term = getenv("TERM")))
		error("Can't get TERM.");
	if(!strncmp(term, "5620", 4))
		flags.nonull = 1;	/* this should be a termcap flag */
	if(tgetent(tptr, term) < 1)
		error("Unknown terminal type: %s.", term);
	if(pc = tgetstr("pc", &tbufptr))
		PC = *pc;
	if(!(BC = tgetstr("bc", &tbufptr))) {	
		if(!tgetflag("bs"))
			error("Terminal must backspace.");
		BC = tbufptr;
		tbufptr += 2;
		*BC = '\b';
	}
	HO = tgetstr("ho", &tbufptr);
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
		printf("Playing hack on terminals without cm is suspect...\n");
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
#if defined(SORTING) || defined(MSDOSCOLOR)
	/* Get rid of padding numbers for HI and HE.  Hope they
	 * aren't really needed!!!  HI and HE are ouputted to the
	 * pager as a string - so how can you send it NULLS???
	 *  -jsb
	 */
	    HI = (char *) alloc((unsigned)(strlen(SO)+1));
	    HE = (char *) alloc((unsigned)(strlen(SE)+1));
	    i = 0;
	    while(isdigit(SO[i])) i++;
	    strcpy(HI, &SO[i]);
	    i = 0;
	    while(isdigit(SE[i])) i++;
	    strcpy(HE, &SE[i]);
#endif
	CD = tgetstr("cd", &tbufptr);
	set_whole_screen();		/* uses LI and CD */
	if(tbufptr-tbuf > sizeof(tbuf)) error("TERMCAP entry too big...\n");
	free(tptr);
#ifdef MSDOSCOLOR
	init_hilite();
#endif
}

start_screen()
{
	xputs(TI);
	xputs(VS);
#ifdef DGK
	/* Select normal ASCII and line drawing character sets.
	 */
	if (flags.DECRainbow)
		xputs("\033(B\033)0");
#endif
}

end_screen()
{
	clear_screen();
	xputs(VE);
	xputs(TE);
}

/* Cursor movements */
extern xchar curx, cury;

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

cmov(x, y)
register x, y;
{
	xputs(tgoto(CM, x-1, y-1));
	cury = y;
	curx = x;
}

xputc(c) char c; {
	(void) fputc(c, stdout);
}

xputs(s) char *s; {
#if defined(MSDOS) && !defined(TERMLIB)
	fputs(s, stdout);
#else
	tputs(s, 1, xputc);
#endif
}

cl_end() {
	if(CE)
		xputs(CE);
	else {	/* no-CE fix - free after Harold Rynes */
		/* this looks terrible, especially on a slow terminal
		   but is better than nothing */
		register cx = curx, cy = cury;

		while(curx < COLNO) {
			xputc(' ');
			curx++;
		}
		curs(cx, cy);
	}
}

clear_screen() {
	xputs(CL);
	home();
}

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

standoutbeg()
{
	if(SO) xputs(SO);
}

standoutend()
{
	if(SE) xputs(SE);
}

backsp()
{
	xputs(BC);
	curx--;
}

bell()
{
#ifdef DGKMOD
	if (flags.silent) return;
#endif /* DGKMOD /**/
	(void) putchar('\007');		/* curx does not change */
	(void) fflush(stdout);
}

static short tmspc10[] = {		/* from termcap */
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5
};

delay_output() {
	/* delay 50 ms - could also use a 'nap'-system call */
	/* BUG: if the padding character is visible, as it is on the 5620
	   then this looks terrible. */
#ifdef MSDOS
	/* simulate the delay with "cursor here" */
	register i;
	for (i = 0; i < 3; i++) {
		cmov(curx, cury);
		(void) fflush(stdout);
	}
#else /* MSDOS /**/
	if(!flags.nonull)
#ifdef TERMINFO
		tputs("$<50>", 1, xputs);
#else
		tputs("50", 1, xputs);
#endif
		/* cbosgd!cbcephus!pds for SYS V R2 */
		/* is this terminfo, or what? */
		/* tputs("$<50>", 1, xputc); */

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
/* Sets up highlighting, using ANSI escape sequences, for monsters and 
 * objects (highlight code found in pri.c). 
 * The termcap entry for HI (from SO) is scanned to find the background 
 * color. If everything is o.k., monsters are displayed in the color 
 * used to define HILITE_MONSTER and objects are displayed in the color 
 * used to define HILITE_OBJECT. */

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

#define HILITE_ATTRIB	NONE
#define HILITE_MONSTER	RED
#define HILITE_OBJECT	YELLOW

init_hilite()
{
	register int backg, len, mfore, ofore;

	backg = BLACK;
	mfore = ofore = WHITE;
	/* find the background color, HI[len] == 'm' */
	len = strlen(HI) - 1;
	if (len > 3) 
	if (isdigit(HI[len-1]) && 
	    isdigit(HI[len-2]) && HI[len-2] != 3) {
			backg = HI[len-1] - '0';
			mfore = HILITE_MONSTER;
			ofore = HILITE_OBJECT;
	}
	if (mfore == backg || ofore == backg) {
		if (len < 7) mfore = ofore = WHITE;
		else {
			if (HI[2] == '3') mfore = ofore = HI[3] - '0';
			else if (HI[4] == '3') mfore = ofore = HI[5] - '0';
			else mfore = ofore = WHITE; /* give up! */
		}
	}

	HI_MON = (char *) alloc(sizeof("E[0;33;44;54m"));
	sprintf(HI_MON, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB, 
	        mfore, backg);
	HI_OBJ = (char *) alloc(sizeof("E[0;33;44;54m"));
	sprintf(HI_OBJ, "%c[%d;3%d;4%dm", ESC, HILITE_ATTRIB, 
	        ofore, backg);
}

#endif
