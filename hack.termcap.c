/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <stdio.h>
#include "config.h"	/* for ROWNO and COLNO */
extern char *tgetstr(), *tgoto(), *getenv();
extern long *alloc();

short ospeed;		/* terminal baudrate; used by tputs */
char tbuf[512];
char *HO, *CL, *CE, *UP, *CM, *ND, *XD, *BC, *SO, *SE;
char PC = '\0';

startup()
{
	register char *tmp;
	register char *tptr;
	char *tbufptr, *pc;

	gettty();	/* sets ospeed */

	tptr = (char *) alloc(1024);

	tbufptr = tbuf;
	if(!(tmp = getenv("TERM")))
		error("Can't get TERM.");
	if(tgetent(tptr,tmp) < 1)
		error("Unknown terminal type: %s.", tmp);
	if(pc = tgetstr("pc",&tbufptr))
		PC = *pc;
	if(!(BC = tgetstr("bc",&tbufptr))) {	
		if(!tgetflag("bs"))
			error("Terminal must backspace.");
		BC = tbufptr;
		tbufptr += 2;
		*BC = '\b';
	}
	HO = tgetstr("ho", &tbufptr);
	if(tgetnum("co") < COLNO || tgetnum("li") < ROWNO+2)
		error("Screen must be at least %d by %d!",
			ROWNO+2, COLNO);
	if(!(CL = tgetstr("cl",&tbufptr)) || !(CE = tgetstr("ce",&tbufptr)) ||
		!(ND = tgetstr("nd", &tbufptr)) ||
		!(UP = tgetstr("up",&tbufptr)) || tgetflag("os"))
		error("Hack needs CL, CE, UP, ND, and no OS.");
	if(!(CM = tgetstr("cm",&tbufptr)))
		printf("Use of hack on terminals without CM is suspect...\n");
	XD = tgetstr("xd",&tbufptr);
	SO = tgetstr("so",&tbufptr);
	SE = tgetstr("se",&tbufptr);
	if(!SO || !SE) SO = SE = 0;
	if(tbufptr-tbuf > sizeof(tbuf)) error("TERMCAP entry too big...\n");
	free(tptr);
}

/* Cursor movements */
extern xchar curx, cury;

curs(x,y)
register int x,y;	/* not xchar: perhaps xchar is unsigned and
			   curx-x would be unsigned as well */
{

	if (y == cury && x == curx) return;
	if(abs(cury-y)<= 3 && abs(curx-x)<= 3) nocmov(x,y);
	else if((x <= 3 && abs(cury-y)<= 3) || (!CM && x<abs(curx-x))) {
		(void) putchar('\r');
		curx = 1;
		nocmov(x,y);
	} else if(!CM) nocmov(x,y);
 else cmov(x,y);
}

nocmov(x,y)
{
	if (curx < x) {		/* Go to the right. */
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
	if (cury > y) {
		if(UP) {
			while (cury > y) {	/* Go up. */
				xputs(UP);
				cury--;
			}
		} else cmov(x,y);
	} else if (cury < y) {
		if(XD) {
			while(cury<y) {
				xputs(XD);
				cury++;
			}
		} else cmov(x,y);
	}
}

cmov(x,y)
register x,y;
{
	if(!CM) error("Tries to cmov from %d %d to %d %d\n",curx,cury,x,y);
	xputs(tgoto(CM,x-1,y-1));
	cury = y;
	curx = x;
}

xputc(c) char c; {
	(void) fputc(c,stdout);
}

xputs(s) char *s; {
	tputs(s, 1, xputc);
}

cl_end() {
	xputs(CE);
}

clear_screen() {
	xputs(CL);
	curx = cury = 1;
}

home()
{
	if(HO) xputs(HO);
	else xputs(tgoto(CM,0,0));
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
	putsym('\007');
}

delay_output() {
	/* delay 40 ms - could also use a 'nap'-system call */
	tputs("40", 1, xputc);
}
