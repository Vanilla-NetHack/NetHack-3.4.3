/*   SCCS Id: @(#)video.c   3.1     93/06/28                        */
/*   Copyright (c) NetHack PC Development Team 1993                 */
/*   NetHack may be freely redistributed.  See license for details. */
/*                                                                  */
/*
 * video.c - Hardware video support
 *                                                  
 *Edit History:
 *     Initial Creation              M. Allison      93/04/04
 *     Add colour support            M. Allison      93/04/06
 *     Fix cl_end()                  M. Allison      93/04/11
 *     Use CO,LI in decl.c           M. Allison      93/04/24
 *     Add djgpp support             K. Smolkowski   93/04/26
 *     Add runtime monoadapter check M. Allison      93/05/09
 *     Fix grays                     M. Allison      93/06/15
 *     MONO_CHECK not BIOS specific  M. Allison      93/06/19
 *     Add .cnf videoshades support  M. Allison      93/06/25
 *     Add .cnf videocolors support  M. Allison      93/06/27
 *     Make tty_delay_output() work  M. Allison      93/06/28
 */

#include "hack.h"

#include <dos.h>

/*
 * Choose compile options for different compilers/environments.
 * Current optional features in video.c :
 *
 *     MONO_CHECK     Enables runtime checking for monochrome
 *                    adapter. 93/06/19
 *     ENABLE_SLEEP   Enables the tty_delay_output() function. 93/06/28
 *
 */

# ifdef SCREEN_BIOS
# define MONO_CHECK		/* Video BIOS can do the check       */ 
#  if defined(_MSC_VER) && _MSC_VER >= 700
# define ENABLE_SLEEP		/* enable napping for visual effects */
#  endif
# endif

# ifdef SCREEN_DJGPPFAST
/*# define MONO_CHECK 		/* djgpp should be able to do check  */ 
# define ENABLE_SLEEP		/* enable napping for visual effects */
# endif

# ifdef PC9801
#  ifdef MONO_CHECK
#undef MONO_CHECK		/* Don't suppose it can do the check */
#  endif                        /* Can it?                           */
# endif

/*
 * PC interrupts
 */
#ifdef PC9801
#define CRT_BIOS    0x18
#else
#define VIDEO_BIOS  0x10
#endif
#define DOSCALL	    0x21

/*
 * Character Attributes
 */
#define ATTRIB_NORMAL         0x07	/* Normal attribute */
#define ATTRIB_INTENSE 	      0x0f	/* Intense White */

#ifdef MONO_CHECK
#define ATTRIB_MONO_UNDERLINE 0x01	/* Underlined,white */
#define ATTRIB_MONO_BLINK     0x87	/* Flash bit, white */
#define ATTRIB_MONO_REVERSE   0x70	/* Black on white */
#endif

/*
 * Video BIOS functions
 */
#if defined(PC9801)
#define SETCURPOS   0x13    /* Set Cursor Position */
#define SENSEMODE   0x0b    /* Sense CRT Mode */
#else
#define SETCURPOS   0x02    /* Set Cursor Position */
#endif

#define GETCURPOS   0x03    /* Get Cursor Position */
#define GETMODE     0x0f    /* Get Video Mode */
#define SETMODE     0x00    /* Set Video Mode */
#define SETPAGE     0x05    /* Set Video Page */
#define FONTINFO    0x1130  /* Get Font Info */
#define SCROLL      0x06    /* Scroll or initialize window */
#define PUTCHARATT  0x09    /* Write attribute & char at cursor */


#ifdef OVLB

void
get_scr_size()
{
	union REGS regs;

	if (!flags.BIOS) {		/* assume standard screen size */
		CO = 80;
		LI = 24;
		return;
	}

# ifdef PC9801
	regs.h.ah = SENSEMODE;
	int86(CRT_BIOS, &regs, &regs);

	LI = (regs.h.al & 0x01) ? 20 : 25;
	CO = (regs.h.al & 0x02) ? 40 : 80;
# else 
	regs.x.ax = FONTINFO;
	regs.x.bx = 0;			/* current ROM BIOS font */
	regs.h.dl = 24;			/* default row count */
					/* in case no EGA/MCGA/VGA */
	int86(VIDEO_BIOS, &regs, &regs); /* Get Font Information */

	/* MDA/CGA/PCjr ignore INT 10h, Function 11h, but since we
	 * cleverly loaded up DL with the default, everything's fine.
	 *
	 * Otherwise, DL now contains rows - 1.  Also, CX contains the
	 * points (bytes per character) and ES:BP points to the font
	 * table.  -3.
	 */

	regs.h.ah = GETMODE;
	int86(VIDEO_BIOS, &regs, &regs); /* Get Video Mode */

	/* This goes back all the way to the original PC.  Completely
	 * safe.  AH contains # of columns, AL contains display mode,
	 * and BH contains the active display page.
	 */

	LI = regs.h.dl + 1;
	CO = regs.h.ah;
# endif /* PC9801 */
}
#endif /*OVLB*/

#ifdef NO_TERMS

#include "wintty.h"

# ifdef SCREEN_DJGPPFAST
#include <pc.h>
# endif

void FDECL(cmov, (int, int));
void FDECL(nocmov, (int, int));

# ifdef TEXTCOLOR
char ttycolors[MAXCOLORS];
static void NDECL(init_ttycolor);
# endif /* TEXTCOLOR */

# ifdef SCREEN_BIOS
void FDECL(gotoxy, (int,int));
void FDECL(get_cursor, (int *, int *));
# endif

# ifdef SCREEN_DJGPPFAST
#define gotoxy(x,y) ScreenSetCursor(y,x)
#define get_cursor(x,y) ScreenGetCursor(y,x)
# endif

# ifdef MONO_CHECK
int  NDECL(monoadapt_check);
# endif

/* 
 *  LI, CO are ifdefs of a data structure in decl.c, and are initialized
 *  by get_scr_size()
 */
char g_attribute;		/* Current attribute to use */

# ifdef MONO_CHECK
int monoflag;			/* 0 = not monochrome, else monochrome */
# endif

#ifdef OVLB

void
backsp()
{
	int x,y;

	get_cursor(&x, &y);
	if (x > 0) x = x-1;
	gotoxy(x,y);
	xputc(' ');
	gotoxy(x,y);
}

#endif /* OVLB */
#ifdef OVL0

void
clear_screen()
/* djgpp provides ScreenClear(), but in version 1.09 it is broken
 * so for now we just use the BIOS Routines
 */
{

	union REGS regs;

	regs.h.dl = CO - 1;	  /* columns */
	regs.h.dh = LI - 1;		  /* rows */
	regs.x.cx = 0;			  /* CL,CH = x,y of upper left */
	regs.x.ax = 0;
	regs.x.bx = 0;
	regs.h.bh = ATTRIB_NORMAL;
	regs.h.ah = SCROLL;
					  /* DL,DH = x,y of lower right */
	int86(VIDEO_BIOS, &regs, &regs);  /* Scroll or init window */
	gotoxy(0,0);
}

void
cl_end()	/* clear to end of line */
{
	union REGS regs;
	int x,y,count;

	x = ttyDisplay->curx;
	y = ttyDisplay->cury;
	gotoxy(x,y);
	count = CO - x;
	regs.h.ah = PUTCHARATT;	/* write attribute & character */	
	regs.h.al = ' ';		/* character */
	regs.h.bh = 0;			/* display page */
					/* BL = attribute */
	regs.h.bl = ATTRIB_NORMAL;
	regs.x.cx = count;
	if (count != 0)
		int86(VIDEO_BIOS, &regs, &regs); /* write attribute 
							& character */
	tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
}

void
cl_eos()	/* clear to end of screen */
{
	union REGS regs;
	int x,y;

	get_cursor(&x, &y);
	cl_end();			/* clear to end of line */
	gotoxy(0,(y < (LI-1) ? y+1 : (LI-1)));		
	regs.h.dl = CO-1;	  /* X  of lower right */
	regs.h.dh = LI-1;		  /* Y  of lower right */
	regs.h.cl = 0;			  /* X  of upper left */
  					  /* Y (row)  of upper left */
	regs.h.ch = (y < (LI-1) ? y+1 :(LI-1));
	regs.x.cx = 0; 
	regs.x.ax = 0;
	regs.x.bx = 0;
	regs.h.bh = ATTRIB_NORMAL;
	regs.h.ah = SCROLL;
	int86(VIDEO_BIOS, &regs, &regs); /* Scroll or initialize window */
	tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1,
						(int)ttyDisplay->cury);
}

void
cmov(x, y)
register int x, y;
{
	ttyDisplay->cury = y;
	ttyDisplay->curx = x;
	gotoxy(x,y);
}

# endif /* OVL0 */
# ifdef OVLB

int
has_color(int color)
{
#  ifdef TEXTCOLOR
	return
#   ifdef MONO_CHECK
	(monoflag) ? 0 :
#   endif
	1;
#  else
	return 0;
#  endif
}

# endif /* OVLB */
# ifdef OVL0

void
home()
{
	tty_curs(BASE_WINDOW, 1, 0);
	ttyDisplay->curx = ttyDisplay->cury = 0;
	gotoxy(0,0);
}

void
nocmov(x, y)
int x,y;
{
	gotoxy(x,y);
}

void
standoutbeg()
{
	g_attribute = ATTRIB_INTENSE;	/* intense white */
}

void
standoutend()
{
	g_attribute = ATTRIB_NORMAL;	/* normal white */
}

# endif /* OVL0 */
# ifdef OVLB

void
term_end_attr(int attr)
{
    	g_attribute = ATTRIB_NORMAL;	/* normal white */
}

void
term_end_color(void)
{
	g_attribute = ATTRIB_NORMAL;	/* normal white  (harmless) */
}

void
term_end_raw_bold(void)
{
    standoutend(); 
}

void
term_start_attr(int attr)
{
    switch(attr){

	case ATR_ULINE:
#  ifdef MONO_CHECK
		if (monoflag) {
			g_attribute = ATTRIB_MONO_UNDERLINE;
		} else {
			g_attribute = ATTRIB_INTENSE;
		}
		break;
#  endif
	case ATR_BOLD:
		g_attribute = ATTRIB_INTENSE;
		break;
	case ATR_BLINK:
#  ifdef MONO_CHECK
		if (monoflag) {
			g_attribute = ATTRIB_MONO_BLINK;
		} else {
			g_attribute = ATTRIB_INTENSE;
		}
		break;
#  endif
	case ATR_INVERSE:
#  ifdef MONO_CHECK
		if (monoflag) {
			g_attribute = ATTRIB_MONO_REVERSE;
		} else {
			g_attribute = ATTRIB_INTENSE;
		}
		break;
#  endif
	default:
		g_attribute = ATTRIB_NORMAL;
		break;
    }                
}


void
term_start_color(int color)
{
#  ifdef TEXTCOLOR
	short attr;

#   ifdef MONO_CHECK
	if (monoflag) {
		g_attribute = ATTRIB_NORMAL;
	} else {
#   endif
		if (color >= 0 && color < MAXCOLORS) {
			g_attribute = ttycolors[color];
	  	}
#   ifdef MONO_CHECK
	}
#   endif
#  endif
}

void
term_start_raw_bold(void)
{
    standoutbeg();
}

# endif /* OVLB */
# ifdef OVL0

void
tty_delay_output()
{
	/* delay 50 ms - now uses clock() which is ANSI C */
#  if defined(ENABLE_SLEEP) || defined(__STDC__)
	clock_t goal;

	goal = 50 + clock();
	while ( goal > clock()) {
	    /* do nothing */
	}
#  endif /* ENABLE_SLEEP || __STDC__*/
}
# endif /* OVL0 */

# ifdef OVLB
void
tty_end_screen()
{
	clear_screen();
}

void
tty_nhbell()
{
        union REGS regs;

        if (flags.silent) return;
        regs.h.dl = 0x07;		/* bell */
        regs.h.ah = 0x02; 		/* Character Output */
        int86(DOSCALL, &regs, &regs);
}

void
tty_number_pad(state)
int state;
{
}

void
tty_startup(wid, hgt)
    int *wid, *hgt;
{
	if (CO && LI) {
		*wid = CO;
		*hgt = LI;
	} else {
		*wid = CO = 80;
		*hgt = LI = 25;
	}
#  ifdef MONO_CHECK
	monoflag = monoadapt_check();
	if (!monoflag) {
#  endif
#  ifdef TEXTCOLOR
	   init_ttycolor();
#  endif
#  ifdef MONO_CHECK
	}
#  endif
	g_attribute = ATTRIB_NORMAL;
}

void
tty_start_screen()
{
	if (flags.num_pad) tty_number_pad(1);   /* make keypad send digits */
}

# endif /* OVLB */
# ifdef OVL0

void
xputs(s)
const char *s;
{
	char c;
	int x,y;

	x = ttyDisplay->curx;
	y = ttyDisplay->cury;
	if (s != NULL) {
		while (*s != '\0') {
			gotoxy(x,y);
			c = *s++;
			xputc(c);
			if (x < (CO-1)) x++;
			gotoxy(x,y);
		}
	}
}

void
xputc(ch)	/* write out character (and attribute) */
char ch;
{
#  ifdef SCREEN_BIOS
	union REGS regs;
#  endif
	int x,y;
	char attribute;

#  ifdef MONO_CHECK
	attribute = ((g_attribute == 0) ? ATTRIB_NORMAL : g_attribute);
#  else
	attribute = (((g_attribute > 0) && (g_attribute < MAXCOLORS)) ?
			g_attribute : ATTRIB_NORMAL);
#  endif

#  ifdef SCREEN_DJGPPFAST
	get_cursor(&x,&y);
	ScreenPutChar((int)ch,(int)attribute,x,y);
#  endif

#  ifdef SCREEN_BIOS
	regs.h.ah = PUTCHARATT;	/* write attribute & character */
	regs.h.al = ch;			/* character */
	regs.h.bh = 0;			/* display page */
	regs.h.bl = attribute;		/* BL = attribute */
	regs.x.cx = 1;			/* one character */
	int86(VIDEO_BIOS, &regs, &regs); /* write attribute & character */
	get_cursor(&x,&y);
#  endif /* SCREEN_BIOS */
	if (x < (CO -1 )) ++x;
	gotoxy(x,y);
}

/*
 *  Supporting routines.
 */
#  ifdef SCREEN_BIOS
void
get_cursor(x,y)	/* get cursor position */
int *x, *y;
{
	union REGS regs;

	regs.x.dx = 0;
	regs.h.ah = GETCURPOS;		/* get cursor position */
	regs.x.cx = 0;
	regs.x.bx = 0;	
	int86(VIDEO_BIOS, &regs, &regs); /* Get Cursor Position */
	*x = regs.h.dl;
	*y = regs.h.dh;
}

void
gotoxy(x,y)
int x,y;
{
	union REGS regs;

	regs.h.ah = SETCURPOS;
# ifdef PC9801
	regs.x.dx = 2 * (80 * y + x);
	int86(CRT_BIOS, &regs, &regs);	/* Set Cursor Position */
# else
	regs.h.bh = 0;			/* display page */
	regs.h.dh = y;			/* row */
	regs.h.dl = x;			/* column */
	int86(VIDEO_BIOS, &regs, &regs); /* Set Cursor Position */
# endif

	/* This, too, goes back all the way to the original PC.  If
	 * we ever get so fancy as to swap display pages (i doubt it),
	 * then we'll need to set BH appropriately.  This function
	 * returns nothing.  -3.
	 */
}
#  endif /* SCREEN_BIOS */ 

#  ifdef MONO_CHECK
int monoadapt_check()
{
	union REGS regs;

	regs.h.al = 0;
	regs.h.ah = GETMODE;			/* get video mode */
	int86(VIDEO_BIOS, &regs, &regs);
	return (regs.h.al == 7) ? 1 : 0;	/* 7 means monochrome mode */
}
#  endif /* MONO_CHECK */

# endif /* OVL0 */

# ifdef TEXTCOLOR
#  ifdef OVLB

/*
 * BLACK                0
 * RED                  1
 * GREEN                2
 * BROWN                3       low-intensity yellow
 * BLUE                 4
 * MAGENTA              5
 * CYAN                 6
 * GRAY                 7       low-intensity white
 * NO_COLOR             8
 * ORANGE_COLORED       9
 * BRIGHT_GREEN         10
 * YELLOW               11
 * BRIGHT_BLUE          12
 * BRIGHT_MAGENTA       13
 * BRIGHT_CYAN          14
 * WHITE                15
 * MAXCOLORS            16
 * BRIGHT               8
 */

#  ifdef VIDEOSHADES
/* assign_videoshades() is prototyped in extern.h */
int shadeflag;					/* shades are initialized */
int colorflag;					/* colors are initialized */
char *schoice[3] = {"dark","normal","light"};
char *shade[3];
#  endif /* VIDEOSHADES */

static void
init_ttycolor()
{
#   ifdef VIDEOSHADES
	if (!shadeflag) {
		ttycolors[BLACK] = 8;           /*  8 = dark gray */
		ttycolors[WHITE] = 15;          /* 15 = bright white */
		ttycolors[GRAY]  = 7;		/*  7 = normal white */
		shade[0] = schoice[0];
		shade[1] = schoice[1];
		shade[2] = schoice[2];
	}
#   else
	ttycolors[BLACK] = 7;			/*  mapped to white */
	ttycolors[WHITE] = 7;			/*  mapped to white */
	ttycolors[GRAY]  = 7;			/*  mapped to white */
#   endif

#   ifdef VIDEOSHADES
    	if (!colorflag) {
#   endif
		ttycolors[RED] = 4;			/*  4 = red */
		ttycolors[GREEN] = 2;			/*  2 = green */
		ttycolors[BROWN] = 6;			/*  6 = brown */
		ttycolors[BLUE] = 1;			/*  1 = blue */
		ttycolors[MAGENTA] = 5;			/*  5 = magenta */
		ttycolors[CYAN] = 3;			/*  3 = cyan */
		ttycolors[BRIGHT] = 15;			/* 15 = bright white */
		ttycolors[ORANGE_COLORED] = 12;		/* 12 = light red */
		ttycolors[BRIGHT_GREEN] = 10;		/* 10 = light green */
		ttycolors[YELLOW] = 14;			/* 14 = yellow */
		ttycolors[BRIGHT_BLUE] = 9;		/*  9 = light blue */
		ttycolors[BRIGHT_MAGENTA] = 13;		/* 13 = light magenta */
		ttycolors[BRIGHT_CYAN] = 11;		/* 11 = light cyan */
#   ifdef VIDEOSHADES
	}
#   endif
}

#   ifdef VIDEOSHADES
int assign_videoshades(uchar *choiceptr,int linelen)
{
	char choices[120];
	char *cptr, *cvalue[3];
	int i,icolor;

	strncpy(choices,choiceptr,linelen);
	choices[linelen] = '\0';
	cvalue[0] = choices;

        /* find the next ' ' or tab */
        cptr = index(cvalue[0], ' ');
        if (!cptr) cptr = index(cvalue[0], '\t');
        if (!cptr) return 0;
	*cptr = '\0';
        /* skip  whitespace between '=' and value */
        do { ++cptr; } while (isspace(*cptr));
	cvalue[1] = cptr;

        cptr = index(cvalue[1], ' ');
        if (!cptr) cptr = index(cvalue[1], '\t');
        if (!cptr) return 0;
	*cptr = '\0';
        do { ++cptr; } while (isspace(*cptr));
	cvalue[2] = cptr;

	for (i=0; i < 3; ++i) {
		switch(i) {
			case 0: icolor = BLACK;
				break;
			case 1: icolor = GRAY;
				break;
			case 2: icolor = WHITE;
				break;
		}

		shadeflag = 1;			
		if ((strncmpi(cvalue[i],"black",5) == 0) ||
		    (strncmpi(cvalue[i],"dark",4) == 0)) {
			shade[i] = schoice[0];
			ttycolors[icolor] = 8;  /* dark gray */
		} else if ((strncmpi(cvalue[i],"gray",4) == 0) ||
		           (strncmpi(cvalue[i],"grey",4) == 0) ||
			   (strncmpi(cvalue[i],"medium",6) == 0) ||
			   (strncmpi(cvalue[i],"normal",6) == 0)) {
			shade[i] = schoice[1];
			ttycolors[icolor] = 7;  /* regular gray */
		} else if ((strncmpi(cvalue[i],"white",5) == 0) ||
			   (strncmpi(cvalue[i],"light",5) == 0)) {
			shade[i] = schoice[2];
			ttycolors[icolor] = 15;  /* bright white */
		} else {
			shadeflag = 0;
			return 0;
		}
	}
	return 1;
}

/*
 * Process NetHack.cnf option VIDEOCOLORS=
 * Left to right assignments for: 
 * 	red green brown blue magenta cyan orange br.green yellow 
 * 	br.blue br.mag br.cyan
 *
 * Default Mapping: 4 2 6 1 5 3 12 10 14 9 13 11
 */
int assign_videocolors(uchar *tmpcolor,int len)
{
	int i,icolor,max1,max2;

	init_ttycolor();	/* in case nethack.cnf entry wasn't complete */
	icolor = RED;
	for( i = 0; i < len; ++i) {
		if (icolor < (WHITE)) {
			ttycolors[icolor++] = tmpcolor[i];
			if ((icolor > CYAN) && (icolor < ORANGE_COLORED)) {
				 icolor = ORANGE_COLORED;
			}
		}
	}
	colorflag = 1;
	return 1;
}
#   endif /* VIDEOSHADES */

#  endif /* OVLB */
# endif /* TEXTCOLOR */

#endif /* NO_TERMS */

/* video.c */
