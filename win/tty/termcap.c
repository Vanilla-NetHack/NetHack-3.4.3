/*	SCCS Id: @(#)termcap.c	3.1	92/11/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "wintty.h"

#include "termcap.h"


#ifdef MICROPORT_286_BUG
#define Tgetstr(key) (tgetstr(key,tbuf))
#else
#define Tgetstr(key) (tgetstr(key,&tbufptr))
#endif /* MICROPORT_286_BUG **/

void FDECL(cmov, (int, int));
void FDECL(nocmov, (int, int));
#ifdef TEXTCOLOR
# ifdef TERMLIB
#  ifdef OVLB
#   ifndef TOS
static void FDECL(analyze_seq, (char *, int *, int *));
#   endif
static void NDECL(init_hilite);
#  endif /* OVLB */
# endif
#endif

#ifdef OVLB
	/* (see termcap.h) -- CM, ND, CD, HI,HE, US,UE, ul_hack */
struct tc_lcl_data tc_lcl_data = { 0, 0, 0, 0,0, 0,0, FALSE };
#endif /* OVLB */

STATIC_VAR char *HO, *CL, *CE, *UP, *XD, *BC, *SO, *SE, *TI, *TE;
STATIC_VAR char *VS, *VE;
#if 0
STATIC_VAR char *MR, *ME;
STATIC_VAR char *MB, *MH;
STATIC_VAR char *MD;     /* may already be in use below */
#endif
#ifdef TERMLIB
# ifdef TEXTCOLOR
STATIC_VAR char *MD;
# endif
STATIC_VAR int SG;
#ifdef OVLB
STATIC_OVL char PC = '\0';
#else /* OVLB */
STATIC_DCL char PC;
#endif /* OVLB */
STATIC_VAR char tbuf[512];
#endif

#ifdef TEXTCOLOR
# ifdef TOS
const char *hilites[MAXCOLORS];	/* terminal escapes for the various colors */
# else
char NEARDATA *hilites[MAXCOLORS]; /* terminal escapes for the various colors */
# endif
#endif

#ifdef OVLB
static char *KS = NULL, *KE = NULL;	/* keypad sequences */
static char nullstr[] = "";
#endif /* OVLB */

#ifndef TERMLIB
STATIC_VAR char tgotobuf[20];
# ifdef TOS
#define tgoto(fmt, x, y)	(Sprintf(tgotobuf, fmt, y+' ', x+' '), tgotobuf)
# else
#define tgoto(fmt, x, y)	(Sprintf(tgotobuf, fmt, y+1, x+1), tgotobuf)
# endif
#endif /* TERMLIB */

#ifdef OVLB

void
tty_startup(wid, hgt)
    int *wid, *hgt;
{
#ifdef TERMLIB
	register const char *term;
	register char *tptr;
	char *tbufptr, *pc;
#endif
	register int i;

#ifdef TERMLIB
# ifdef VMS
	if (!(term = verify_termcap()))
# endif
	term = getenv("TERM");
#endif

#ifdef TERMLIB
	if(!term)
#endif
#if defined(TOS) && defined(__GNUC__) && defined(TERMLIB)
		term = "builtin";		/* library has a default */
#else
#  ifdef ANSI_DEFAULT
#   ifdef TOS
	{
		CO = 80; LI = 25;
		TI = VS = VE = TE = nullstr;
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
	/* HI and HE will be updated in init_hilite if we're using color */
		HI = "\033p";
		HE = "\033q";
	}
#   else /* TOS */
	{
#    ifdef MICRO
		get_scr_size();
#     ifdef CLIPPING
		if(CO < COLNO || LI < ROWNO+3)
			setclipped();
#     endif
#    endif
		HO = "\033[H";
		CL = "\033[2J";		/* the ANSI termcap */
/*		CD = "\033[J"; */
		CE = "\033[K";
#    ifndef TERMLIB
		CM = "\033[%d;%dH";
#    else
		CM = "\033[%i%d;%dH";
#    endif
		UP = "\033[A";
		ND = "\033[C";
		XD = "\033[B";
#    ifdef MICRO	/* backspaces are non-destructive */
		BC = "\b";
#    else
		BC = "\033[D";
#    endif
		HI = SO = "\033[1m";
		US = "\033[4m";
#    if 0
		MR = "\033[7m";
		ME = "\033[0m";
#    endif
		TI = HE = SE = UE = "\033[0m";
		/* strictly, SE should be 2, and UE should be 24,
		   but we can't trust all ANSI emulators to be
		   that complete.  -3. */
#    ifndef MICRO
		AS = "\016";
		AE = "\017";
#    endif
		TE = VS = VE = nullstr;
#    ifdef TEXTCOLOR
		for (i = 0; i < MAXCOLORS / 2; i++)
		    if (i != BLACK) {
			hilites[i|BRIGHT] = (char *) alloc(sizeof("\033[1;3%dm"));
			Sprintf(hilites[i|BRIGHT], "\033[1;3%dm", i);
			if (i != GRAY)
#     ifdef MICRO
			    if (i == BLUE) hilites[BLUE] = hilites[BLUE|BRIGHT];
			    else
#     endif
			    {
				hilites[i] = (char *) alloc(sizeof("\033[0;3%dm"));
				Sprintf(hilites[i], "\033[0;3%dm", i);
			    }
		    }
#    endif
		*wid = CO;
		*hgt = LI;
		return;
	}
#   endif /* TOS */
#  else
		error("Can't get TERM.");
#  endif /* ANSI_DEFAULT */
#endif /* __GNUC__ && TOS && TERMCAP */
#ifdef TERMLIB
	tptr = (char *) alloc(1024);

	tbufptr = tbuf;
	if(!strncmp(term, "5620", 4))
		flags.null = FALSE;	/* this should be a termcap flag */
	if(tgetent(tptr, term) < 1)
		error("Unknown terminal type: %s.", term);
	if ((pc = Tgetstr("pc")) != 0)
		PC = *pc;

	if(!(BC = Tgetstr("le")))	/* both termcap and terminfo use le */	
# ifdef TERMINFO
	    error("Terminal must backspace.");
# else
	    if(!(BC = Tgetstr("bc"))) {	/* termcap also uses bc/bs */
#  if !defined(MINIMAL_TERM)
		if(!tgetflag("bs"))
			error("Terminal must backspace.");
#  endif
		BC = tbufptr;
		tbufptr += 2;
		*BC = '\b';
	    }
# endif

# ifdef MINIMAL_TERM
	HO = NULL;
# else
	HO = Tgetstr("ho");
# endif
	/*
	 * LI and CO are set in ioctl.c via a TIOCGWINSZ if available.  If
	 * the kernel has values for either we should use them rather than
	 * the values from TERMCAP ...
	 */
# ifndef MICRO
	if (!CO) CO = tgetnum("co");
	if (!LI) LI = tgetnum("li");
# else
#  if defined(TOS) && defined(__GNUC__)
	if (!strcmp(term, "builtin"))
		get_scr_size();
	else {
#  endif
	CO = tgetnum("co");
	LI = tgetnum("li");
	if (!LI || !CO)			/* if we don't override it */
		get_scr_size();
#  if defined(TOS) && defined(__GNUC__)
	}
#  endif
# endif
# ifdef CLIPPING
	if(CO < COLNO || LI < ROWNO+3)
		setclipped();
# endif
	if(!(CL = Tgetstr("cl")))
		error("Hack needs CL.");
	ND = Tgetstr("nd");
	if(tgetflag("os"))
		error("Hack can't have OS.");
	if(tgetflag("ul"))
		ul_hack = TRUE;
	CE = Tgetstr("ce");
	UP = Tgetstr("up");
	/* It seems that xd is no longer supported, and we should use
	   a linefeed instead; unfortunately this requires resetting
	   CRMOD, and many output routines will have to be modified
	   slightly. Let's leave that till the next release. */
	XD = Tgetstr("xd");
/* not: 		XD = Tgetstr("do"); */
	if(!(CM = Tgetstr("cm"))) {
		if(!UP && !HO)
			error("Hack needs CM or UP or HO.");
		tty_raw_print("Playing hack on terminals without cm is suspect...");
		tty_wait_synch();
	}
	SO = Tgetstr("so");
	SE = Tgetstr("se");
	US = Tgetstr("us");
	UE = Tgetstr("ue");
	SG = tgetnum("sg");	/* -1: not fnd; else # of spaces left by so */
	if(!SO || !SE || (SG > 0)) SO = SE = US = UE = nullstr;
	TI = Tgetstr("ti");
	TE = Tgetstr("te");
	VS = VE = nullstr;
# ifdef TERMINFO
	VS = Tgetstr("eA");	/* enable graphics */
# endif
	KS = Tgetstr("ks");	/* keypad start (special mode) */
	KE = Tgetstr("ke");	/* keypad end (ordinary mode [ie, digits]) */
# if 0
	MR = Tgetstr("mr");	/* reverse */
	MB = Tgetstr("mb");	/* blink */
	MD = Tgetstr("md");	/* boldface */
	MH = Tgetstr("mh");	/* dim */
	ME = Tgetstr("me");
# endif

	/* Get rid of padding numbers for HI and HE.  Hope they
	 * aren't really needed!!!  HI and HE are ouputted to the
	 * pager as a string - so how can you send it NULLS???
	 *  -jsb
	 */
	    HI = (char *) alloc((unsigned)(strlen(SO)+1));
	    HE = (char *) alloc((unsigned)(strlen(SE)+1));
	    i = 0;
	    while (digit(SO[i])) i++;
	    Strcpy(HI, &SO[i]);
	    i = 0;
	    while (digit(SE[i])) i++;
	    Strcpy(HE, &SE[i]);
	AS = Tgetstr("as");
	AE = Tgetstr("ae");
	CD = Tgetstr("cd");
# ifdef TEXTCOLOR
	MD = Tgetstr("md");
# endif
	if(tbufptr-tbuf > sizeof(tbuf)) error("TERMCAP entry too big...\n");
	free((genericptr_t)tptr);
# ifdef TEXTCOLOR
#  if defined(TOS) && defined(__GNUC__)
	if (!strcmp(term, "builtin") || !strcmp(term, "tw52") ||
	    !strcmp(term, "st52")) {
		init_hilite();
	}
#  else
	init_hilite();
#  endif
# endif
#endif /* TERMLIB */
	*wid = CO;
	*hgt = LI;
}

void
tty_number_pad(state)
int state;
{
	switch (state) {
	    case -1:	/* activate keypad mode (escape sequences) */
		    if (KS && *KS) xputs(KS);
		    break;
	    case  1:	/* activate numeric mode for keypad (digits) */
		    if (KE && *KE) xputs(KE);
		    break;
	    case  0:	/* don't need to do anything--leave terminal as-is */
	    default:
		    break;
	}
}

#ifdef TERMLIB
extern void NDECL((*decgraphics_mode_callback));    /* defined in drawing.c */
static void NDECL(tty_decgraphics_termcap_fixup);

/*
   We call this routine whenever DECgraphics mode is enabled, even if it
   has been previously set, in case the user manages to reset the fonts.
   The actual termcap fixup only needs to be done once, but we can't
   call xputs() from the option setting or graphics assigning routines,
   so this is a convenient hook.
 */
static void
tty_decgraphics_termcap_fixup()
{
	static char ctrlN[]   = "\016";
	static char ctrlO[]   = "\017";
	static char appMode[] = "\033=";
	static char numMode[] = "\033>";

	/* these values are missing from some termcaps */
	if (!AS) AS = ctrlN;	/* ^N (shift-out [graphics font]) */
	if (!AE) AE = ctrlO;	/* ^O (shift-in  [regular font])  */
	if (!KS) KS = appMode;	/* ESC= (application keypad mode) */
	if (!KE) KE = numMode;	/* ESC> (numeric keypad mode)	  */
	/*
	 * Select the line-drawing character set as the alternate font.
	 * Do not select NA ASCII as the primary font since people may
	 * reasonably be using the UK character set.
	 */
	if (flags.DECgraphics) xputs("\033)0");
}
#endif

void
tty_start_screen()
{
	xputs(TI);
	xputs(VS);
#ifdef TERMLIB
	if (flags.DECgraphics) tty_decgraphics_termcap_fixup();
	/* set up callback in case option is not set yet but toggled later */
	decgraphics_mode_callback = tty_decgraphics_termcap_fixup;
#endif
	if (flags.num_pad) tty_number_pad(1);	/* make keypad send digits */
}

void
tty_end_screen()
{
	clear_screen();
	xputs(VE);
	xputs(TE);
}

/* Cursor movements */

#endif /* OVLB */

#ifdef OVL0
/* Note to OVLx tinkerers.  The placement of this overlay controls the location
   of the function xputc().  This function is not currently in trampoli.[ch]
   files for what is deemed to be performance reasons.  If this define is moved
   and or xputc() is taken out of the ROOT overlay, then action must be taken
   in trampoli.[ch]. */

void
nocmov(x, y)
int x,y;
{
	if ((int) ttyDisplay->cury > y) {
		if(UP) {
			while ((int) ttyDisplay->cury > y) {	/* Go up. */
				xputs(UP);
				ttyDisplay->cury--;
			}
		} else if(CM) {
			cmov(x, y);
		} else if(HO) {
			home();
			tty_curs(BASE_WINDOW, x+1, y);
		} /* else impossible("..."); */
	} else if ((int) ttyDisplay->cury < y) {
		if(XD) {
			while((int) ttyDisplay->cury < y) {
				xputs(XD);
				ttyDisplay->cury++;
			}
		} else if(CM) {
			cmov(x, y);
		} else {
			while((int) ttyDisplay->cury < y) {
				xputc('\n');
				ttyDisplay->curx = 0;
				ttyDisplay->cury++;
			}
		}
	}
	if ((int) ttyDisplay->curx < x) {		/* Go to the right. */
		if(!ND) cmov(x, y); else	/* bah */
			/* should instead print what is there already */
		while ((int) ttyDisplay->curx < x) {
			xputs(ND);
			ttyDisplay->curx++;
		}
	} else if ((int) ttyDisplay->curx > x) {
		while ((int) ttyDisplay->curx > x) {	/* Go to the left. */
			xputs(BC);
			ttyDisplay->curx--;
		}
	}
}

void
cmov(x, y)
register int x, y;
{
	xputs(tgoto(CM, x, y));
	ttyDisplay->cury = y;
	ttyDisplay->curx = x;
}

/* See note at OVLx ifdef above.   xputc() is a special function. */
void
xputc(c)
#if defined(apollo)
int c;
#else
char c;
#endif
{
	(void) putchar(c);
}

void
xputs(s)
const char *s;
{
# ifndef TERMLIB
	(void) fputs(s, stdout);
# else
#  if defined(NHSTDC) || defined(ULTRIX_PROTO)
	tputs(s, 1, (int (*)())xputc);
#  else
	tputs(s, 1, xputc);
#  endif
# endif
}

void
cl_end()
{
	if(CE)
		xputs(CE);
	else {	/* no-CE fix - free after Harold Rynes */
		/* this looks terrible, especially on a slow terminal
		   but is better than nothing */
		register int cx = ttyDisplay->curx+1;

		while(cx < CO) {
			xputc(' ');
			cx++;
		}
		tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
	}
}

#endif /* OVL0 */
#ifdef OVLB

void
clear_screen()
{
	/* note: if CL is null, then termcap initialization failed,
		so don't attempt screen-oriented I/O during final cleanup.
	 */
	if (CL) {
		xputs(CL);
		home();
	}
}

#endif /* OVLB */
#ifdef OVL0

void
home()
{
	if(HO)
		xputs(HO);
	else if(CM)
		xputs(tgoto(CM, 0, 0));
	else
		tty_curs(BASE_WINDOW, 1, 0);	/* using UP ... */
	ttyDisplay->curx = ttyDisplay->cury = 0;
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
#endif

#endif /* OVL0 */
#ifdef OVLB

void
backsp()
{
	xputs(BC);
}

void
tty_nhbell()
{
	if (flags.silent) return;
	(void) putchar('\007');		/* curx does not change */
	(void) fflush(stdout);
}

#endif /* OVLB */
#ifdef OVL0

#ifdef ASCIIGRAPH
void
graph_on() {
	if (AS) xputs(AS);
}

void
graph_off() {
	if (AE) xputs(AE);
}
#endif

#endif /* OVL0 */
#ifdef OVL1

#if !defined(MICRO)
# ifdef VMS
static const short tmspc10[] = {		/* from termcap */
	0, 2000, 1333, 909, 743, 666, 333, 166, 83, 55, 50, 41, 27, 20, 13, 10,
	5
};
# else
static const short tmspc10[] = {		/* from termcap */
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5
};
# endif
#endif

void
tty_delay_output()
{
	/* delay 50 ms - could also use a 'nap'-system call */
	/* BUG: if the padding character is visible, as it is on the 5620
	   then this looks terrible. */
#if defined(MICRO)
	/* simulate the delay with "cursor here" */
	register int i;
	for (i = 0; i < 3; i++) {
		cmov(ttyDisplay->curx, ttyDisplay->cury);
		(void) fflush(stdout);
	}
#else /* MICRO */
	if(flags.null)
# ifdef TERMINFO
		/* cbosgd!cbcephus!pds for SYS V R2 */
#  ifdef NHSTDC
		tputs("$<50>", 1, (int (*)())xputc);
#  else
		tputs("$<50>", 1, xputc);
#  endif
# else
#  if defined(NHSTDC) || defined(ULTRIX_PROTO)
		tputs("50", 1, (int (*)())xputc);
#  else
		tputs("50", 1, xputc);
#  endif
# endif

	else if(ospeed > 0 && ospeed < SIZE(tmspc10)) if(CM) {
		/* delay by sending cm(here) an appropriate number of times */
		register int cmlen = strlen(tgoto(CM, ttyDisplay->curx, ttyDisplay->cury));
		register int i = 500 + tmspc10[ospeed]/2;

		while(i > 0) {
			cmov((int)ttyDisplay->curx, (int)ttyDisplay->cury);
			i -= cmlen*tmspc10[ospeed];
		}
	}
#endif /* MICRO */
}

#endif /* OVL1 */
#ifdef OVLB

void
cl_eos()			/* free after Robert Viduya */
{				/* must only be called with curx = 1 */

	if(CD)
		xputs(CD);
	else {
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
}

#if defined(TEXTCOLOR) && defined(TERMLIB)
# if defined(UNIX) && defined(TERMINFO)
/*
 * Sets up color highlighting, using terminfo(4) escape sequences (highlight
 * code found in print.c).  It is assumed that the background color is black.
 */
/* terminfo indexes for the basic colors it guarantees */
#define COLOR_BLACK   1		/* fake out to avoid black on black */
#define COLOR_BLUE    1
#define COLOR_GREEN   2
#define COLOR_CYAN    3
#define COLOR_RED     4
#define COLOR_MAGENTA 5
#define COLOR_YELLOW  6
#define COLOR_WHITE   7

/* map ANSI RGB to terminfo BGR */
const int ti_map[8] = {
	COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
	COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE };

static void
init_hilite()
{
	register int c;
	char *setf, *scratch;
	extern char *tparm();

	for (c = 0; c < SIZE(hilites); c++)
		hilites[c] = HI;
	hilites[GRAY] = hilites[NO_COLOR] = NULL;

	if (tgetnum("Co") < 8 || (setf = tgetstr("Sf", (char **)0)) == NULL)
		return;

	for (c = 0; c < MAXCOLORS / 2; c++) {
  		scratch = tparm(setf, ti_map[c]);
		if (c != GRAY) {
			hilites[c] = (char *) alloc(strlen(scratch) + 1);
			Strcpy(hilites[c], scratch);
		}
		if (c != BLACK) {
			hilites[c|BRIGHT] = (char*) alloc(strlen(scratch)+strlen(MD)+1);
			Strcpy(hilites[c|BRIGHT], MD);
			Strcat(hilites[c|BRIGHT], scratch);
	        }
		
	}
}

# else /* UNIX && TERMINFO */

#  ifndef TOS
/* find the foreground and background colors set by HI or HE */
static void
analyze_seq (str, fg, bg)
char *str;
int *fg, *bg;
{
	register int c, code;
	int len;

#   ifdef MICRO
	*fg = GRAY; *bg = BLACK;
#   else
	*fg = *bg = NO_COLOR;
#   endif

	if (str[0] != '\033' || str[1] != '[' ||
	    str[len = strlen(str) - 1] != 'm' || len < 3)
		return;

	c = 2;
	while (c < len) {
	    if ((code = atoi(&str[c])) == 0) { /* reset */
		/* this also catches errors */
#   ifdef MICRO
		*fg = GRAY; *bg = BLACK;
#   else
		*fg = *bg = NO_COLOR;
#   endif
	    } else if (code == 1) { /* bold */
		*fg |= BRIGHT;
#   if 0
	/* I doubt we'll ever resort to using blinking characters,
	   unless we want a pulsing glow for something.  But, in case
	   we do... - 3. */
	    } else if (code == 5) { /* blinking */
		*fg |= BLINK;
	    } else if (code == 25) { /* stop blinking */
		*fg &= ~BLINK;
#   endif
	    } else if (code == 7 || code == 27) { /* reverse */
		code = *fg & ~BRIGHT;
		*fg = *bg | (*fg & BRIGHT);
		*bg = code;
	    } else if (code >= 30 && code <= 37) { /* hi_foreground RGB */
		*fg = code - 30;
	    } else if (code >= 40 && code <= 47) { /* hi_background RGB */
		*bg = code - 40;
	    }
	    while (digit(str[++c]));
	    c++;
	}
}
#  endif

/*
 * Sets up highlighting sequences, using ANSI escape sequences (highlight code
 * found in print.c).  The HI and HE sequences (usually from SO) is scanned to
 * find foreground and background colors.
 */

static void
init_hilite()
{
	register int c;
#  ifdef TOS
	extern unsigned long tos_numcolors;	/* in tos.c */
	static char NOCOL[] = "\033b0", COLHE[] = "\033q\033b0";

	if (tos_numcolors <= 2) {
		return;
	}
/* Under TOS, the "bright" and "dim" colors are reversed. Moreover,
 * on the Falcon the dim colors are *really* dim; so we make most
 * of the colors the bright versions, with a few exceptions where
 * the dim ones look OK.
 */
	hilites[0] = NOCOL;
	for (c = 1; c < SIZE(hilites); c++) {
		char *foo;
		foo = (char *) alloc(sizeof("\033b0"));
		if (tos_numcolors > 4)
			Sprintf(foo, "\033b%c", (c&~BRIGHT)+'0');
		else
			Strcpy(foo, "\033b0");
		hilites[c] = foo;
	}

	if (tos_numcolors == 4) {
		TI = "\033b0\033c3\033E\033e";
		TE = "\033b3\033c0\033J";
		HE = COLHE;
		hilites[GREEN] = hilites[GREEN|BRIGHT] = "\033b2";
		hilites[RED] = hilites[RED|BRIGHT] = "\033b1";
	} else {
		sprintf(hilites[BROWN], "\033b%c", (BROWN^BRIGHT)+'0');
		sprintf(hilites[GREEN], "\033b%c", (GREEN^BRIGHT)+'0');

		TI = "\033b0\033c\017\033E\033e";
		TE = "\033b\017\033c0\033J";
		HE = COLHE;
		hilites[WHITE] = hilites[BLACK] = NOCOL;
		hilites[NO_COLOR] = hilites[GRAY];
	}

#  else /* TOS */

	int backg, foreg, hi_backg, hi_foreg;

 	for (c = 0; c < SIZE(hilites); c++)
	    hilites[c] = HI;
	hilites[GRAY] = hilites[NO_COLOR] = NULL;

	analyze_seq(HI, &hi_foreg, &hi_backg);
	analyze_seq(HE, &foreg, &backg);

	for (c = 0; c < SIZE(hilites); c++)
	    /* avoid invisibility */
	    if ((backg & ~BRIGHT) != c) {
#   ifdef MICRO
		if (c == BLUE) continue;
#   endif
		if (c == foreg)
		    hilites[c] = NULL;
		else if (c != hi_foreg && backg != hi_backg) {
		    hilites[c] = (char *) alloc(sizeof("\033[%d;3%d;4%dm"));
		    Sprintf(hilites[c], "\033[%d", !!(c & BRIGHT));
		    if ((c | BRIGHT) != (foreg | BRIGHT))
			Sprintf(eos(hilites[c]), ";3%d", c & ~BRIGHT);
		    if (backg != BLACK)
			Sprintf(eos(hilites[c]), ";4%d", backg & ~BRIGHT);
		    Strcat(hilites[c], "m");
		}
	    }

#   ifdef MICRO
	/* brighten low-visibility colors */
	hilites[BLUE] = hilites[BLUE|BRIGHT];
#   endif
#  endif /* TOS */
}
# endif /* UNIX */
#endif /* TEXTCOLOR */

#endif /* OVLB */

/*termcap.c*/
