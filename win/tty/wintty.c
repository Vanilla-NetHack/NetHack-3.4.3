/*	SCCS Id: @(#)wintty.c	3.1	93/05/26	*/
/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Neither a standard out nor character-based control codes should be
 * part of the "tty look" windowing implementation.
 * h+ 930227
 */

#include "hack.h"

#ifdef TTY_GRAPHICS

#ifdef MAC
# define MICRO /* The Mac is a MICRO only for this file, not in general! */
#endif

#ifndef NO_TERMS
# include "termcap.h"
#endif

#include "wintty.h"

#if (defined(BSD) || defined(ULTRIX) || defined(AIX_31) || defined(_BULL_SOURCE)) && defined(CLIPPING)
#include <signal.h>
#endif

#define DEBUG

extern const char *roles[];	/* from u_init.c */

/* Interface definition, for windows.c */
struct window_procs tty_procs = {
    "tty",
    tty_init_nhwindows,
    tty_player_selection,
    tty_askname,
    tty_get_nh_event,
    tty_exit_nhwindows,
    tty_suspend_nhwindows,
    tty_resume_nhwindows,
    tty_create_nhwindow,
    tty_clear_nhwindow,
    tty_display_nhwindow,
    tty_destroy_nhwindow,
    tty_curs,
    tty_putstr,
    tty_display_file,
    tty_start_menu,
    tty_add_menu,
    tty_end_menu,
    tty_select_menu,
    tty_update_inventory,
    tty_mark_synch,
    tty_wait_synch,
#ifdef CLIPPING
    tty_cliparound,
#endif
    tty_print_glyph,
    tty_raw_print,
    tty_raw_print_bold,
    tty_nhgetch,
    tty_nh_poskey,
    tty_nhbell,
    tty_doprev_message,
    tty_yn_function,
    tty_getlin,
#ifdef COM_COMPL
    tty_get_ext_cmd,
#endif /* COM_COMPL */
    tty_number_pad,
    tty_delay_output,
#ifdef CHANGE_COLOR	/* the Mac uses a palette device */
    tty_change_color,
    tty_get_color_string,
#endif

    /* other defs that really should go away (they're tty specific) */
    tty_start_screen,
    tty_end_screen,
    genl_outrip,
};

static int maxwin = 0;			/* number of windows in use */
winid BASE_WINDOW;
struct WinDesc *wins[MAXWIN];
struct DisplayDesc *ttyDisplay;	/* the tty display descriptor */

extern void FDECL(cmov, (int,int)); /* from termcap.c */
extern void FDECL(nocmov, (int,int)); /* from termcap.c */
#if defined(UNIX) || defined(VMS)
static char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */
#endif

static char winpanicstr[] = "Bad window id %d";
char defmorestr[] = "--More--";

#ifdef CLIPPING
static boolean clipping = FALSE;	/* clipping on? */
static int clipx = 0, clipy = 0, clipxmax = 0, clipymax = 0;
#endif

#if defined(ASCIIGRAPH) && !defined(NO_TERMS)
boolean GFlag = FALSE;
#endif

#ifdef MICRO
static char to_continue[] = "to continue";
#define getret() getreturn(to_continue)
#else
static void NDECL(getret);
#endif
static void FDECL(dmore,(struct WinDesc *));
static const char * FDECL(compress_str, (const char *));
static void FDECL(tty_putsym, (winid, int, int, CHAR_P));

#if defined(SIGWINCH) && defined(CLIPPING)
static void
winch()
{
    int oldLI = LI, oldCO = CO, i;
    register struct WinDesc *cw;

    getwindowsz();
    if((oldLI != LI || oldCO != CO) && ttyDisplay) {
	ttyDisplay->rows = LI;
	ttyDisplay->cols = CO;

	cw = wins[BASE_WINDOW];
	cw->rows = ttyDisplay->rows;
	cw->cols = ttyDisplay->cols;

	if(flags.window_inited) {
	    cw = wins[WIN_MESSAGE];
	    cw->curx = cw->cury = 0;

	    tty_destroy_nhwindow(WIN_STATUS);
	    WIN_STATUS = tty_create_nhwindow(NHW_STATUS);

	    if(u.ux) {
#ifdef CLIPPING
		if(CO < COLNO || LI < ROWNO+3) {
		    setclipped();
		    tty_cliparound(u.ux, u.uy);
		} else {
		    clipping = FALSE;
		    clipx = clipy = 0;
		}
#endif
		i = ttyDisplay->toplin;
		ttyDisplay->toplin = 0;
		docrt();
		bot();
		ttyDisplay->toplin = i;
		flush_screen(1);
		if(i) {
		    addtopl(toplines);
		} else
		    for(i=WIN_INVEN; i < MAXWIN; i++)
			if(wins[i] && wins[i]->active) {
			    /* cop-out */
			    addtopl("Press Return to continue: ");
			    break;
			}
		(void) fflush(stdout);
		if(i < 2) flush_screen(1);
	    }
	}
    }
}
#endif

void
tty_init_nhwindows()
{
    int wid, hgt;

    /*
     *  Remember tty modes, to be restored on exit.
     *
     *  gettty() must be called before tty_startup()
     *    due to ordering of LI/CO settings
     *  tty_startup() must be called before initoptions()
     *    due to ordering of graphics settings
     */
#if defined(UNIX) || defined(VMS)
    setbuf(stdout,obuf);
#endif
    gettty();

    /* to port dependant tty setup */
    tty_startup(&wid, &hgt);
    setftty();			/* calls start_screen */

    /* set up tty descriptor */
    ttyDisplay = (struct DisplayDesc*) alloc(sizeof(struct DisplayDesc));
    ttyDisplay->toplin = 0;
    ttyDisplay->rows = hgt;
    ttyDisplay->cols = wid;
    ttyDisplay->curx = ttyDisplay->cury = 0;
    ttyDisplay->inmore = ttyDisplay->inread = ttyDisplay->intr = 0;
#ifdef TEXTCOLOR
    ttyDisplay->color = NO_COLOR;
#endif
    ttyDisplay->attrs = 0;

    /* set up the default windows */
    BASE_WINDOW = tty_create_nhwindow(NHW_BASE);
    wins[BASE_WINDOW]->active = 1;

    ttyDisplay->lastwin = WIN_ERR;

#if defined(SIGWINCH) && defined(CLIPPING)
    (void) signal(SIGWINCH, winch);
#endif

    tty_clear_nhwindow(BASE_WINDOW);

    tty_putstr(BASE_WINDOW, 0, "");
    tty_putstr(BASE_WINDOW, 0,
	 "NetHack, Copyright 1985, 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993");
    tty_putstr(BASE_WINDOW, 0,
	 "         By Stichting Mathematisch Centrum and M. Stephenson.");
    tty_putstr(BASE_WINDOW, 0, "         See license for details.");
    tty_putstr(BASE_WINDOW, 0, "");
    tty_display_nhwindow(BASE_WINDOW, FALSE);
}

void
tty_player_selection()
{
    char pbuf[QBUFSZ];
    char pick, pc;
    int i, linecount;

    linecount = wins[BASE_WINDOW]->cury+1;
    if ((pc = highc(pl_character[0])) != 0) {
	if(index(pl_classes, pc) != (char*) 0)
	    goto got_suffix;
	tty_putstr(BASE_WINDOW, 0, "");
	Sprintf(pbuf, "Unknown role: %c", pc);
	tty_putstr(BASE_WINDOW, 0, pbuf);
	linecount += 2;
	pl_character[0] = pc = 0;
    }

#define PICK_PROMPT "Shall I pick a character for you? [Y, N, or Q(quit)] "
    tty_putstr(BASE_WINDOW, 0, "");
    tty_putstr(BASE_WINDOW, 0, PICK_PROMPT);

    while(!index("yYnNqQ", (pick = readchar())) && !index(quitchars, pick))
	tty_nhbell();

    pick = index(quitchars, pick) ? 'Y' : highc(pick);

    tty_putsym(BASE_WINDOW, (int)strlen(PICK_PROMPT)+1, linecount, pick); /* echo */

    if (pick == 'Q') {
	clearlocks();
	tty_exit_nhwindows(NULL);
	terminate(0);
    }

    if (pick == 'Y') {
	tty_putstr(BASE_WINDOW, 0, "");
	goto beginner;
    }

    tty_curs(BASE_WINDOW, 1, linecount+2);
    tty_putstr(BASE_WINDOW, 0, "What kind of character are you:");
    tty_putstr(BASE_WINDOW, 0, "");
    Sprintf(pbuf, " 	 %s,", An(roles[0]));
    for(i = 1; roles[i]; i++) {
	Sprintf(eos(pbuf), " %s", an(roles[i]));
	if((((i + 1) % 4) == 0) && roles[i+1]) {
	    Strcat(pbuf, ",");
	    tty_putstr(BASE_WINDOW, 0, pbuf);
	    linecount++;
	    Strcpy(pbuf, "        ");
	}
	else if(roles[i+1] && roles[i+2])	Strcat(pbuf, ",");
	if(roles[i+1] && !roles[i+2])	Strcat(pbuf, " or");
    }
    Strcat(pbuf ,"?");
    tty_putstr(BASE_WINDOW, 0, pbuf);
    Strcpy(pbuf, "         [");
    for(i = 0; roles[i]; i++)
	Sprintf(eos(pbuf), "%c,", pl_classes[i]);
    Strcat(pbuf, " or Q] ");
    tty_putstr(BASE_WINDOW, 0, pbuf);
    linecount += 5;

    while ((pc = readchar()) != 0) {
	if ((pc = highc(pc)) == 'Q') {
	    clearlocks();
	    tty_exit_nhwindows(NULL);
	    terminate(0);
	}
	if(index(pl_classes, pc) != (char *) 0) {
	    tty_putsym(BASE_WINDOW, (int)strlen(pbuf)+1, linecount, pc); /* echo */
	    tty_putstr(BASE_WINDOW, 0, "");
	    tty_display_nhwindow(BASE_WINDOW, TRUE);
	    break;
	}
	if(pc == '\n') {
	    pc = 0;
	    break;
	}
	tty_nhbell();
    }

beginner:
    if(!pc) {
	i = rn2((int)strlen(pl_classes));
	pc = pl_classes[i];
	tty_putstr(BASE_WINDOW, 0, "");
	Sprintf(pbuf, "This game you will be %s.", an(roles[i]));
	tty_putstr(BASE_WINDOW, 0, pbuf);
	tty_putstr(BASE_WINDOW, 0, "");
	tty_display_nhwindow(BASE_WINDOW, TRUE);
	getret();
    }
got_suffix:

    tty_clear_nhwindow(BASE_WINDOW);
    pl_character[0] = pc;
    return;
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 * Always called after init_nhwindows() and before display_gamewindows().
 */
void
tty_askname()
{
	register int c, ct;

	tty_putstr(BASE_WINDOW, 0, "");
	tty_putstr(BASE_WINDOW, 0, "Who are you? ");
	tty_curs(BASE_WINDOW, 14, wins[BASE_WINDOW]->cury-1);
	ct = 0;
	while((c = tty_nhgetch()) != '\n') {
		if(c == EOF) error("End of input\n");
		/* some people get confused when their erase char is not ^H */
		if (c == '\b' || c == '\177') {
			if(ct) {
				ct--;
#ifdef MICRO
# if defined(WIN32CON)
				backsp();       /* \b is visible on NT */
# else
				msmsg("\b \b");
# endif
#else
				(void) putchar('\b');
				(void) putchar(' ');
				(void) putchar('\b');
#endif
			}
			continue;
		}
#if defined(UNIX) || defined(VMS)
		if(c != '-' && c != '@')
		if(c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
#endif
		if(ct < sizeof(plname)-1) {
#if defined(MICRO)
			msmsg("%c", c);
#else
			(void) putchar(c);
#endif
			plname[ct++] = c;
		}
	}
	plname[ct] = 0;
	tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury+1);
	if(ct == 0) tty_askname();
}

void
tty_get_nh_event()
{
#ifdef LINT
    /*
     * We should do absolutely nothing here - but lint
     * complains about that, so we call donull().
     */
     (void) donull();
#endif
}

#ifndef MICRO
static void
getret()
{
	xputs("\n");
	if(flags.standout)
		standoutbeg();
	xputs("Hit ");
	xputs(flags.cbreak ? "space" : "return");
	xputs(" to continue: ");
	if(flags.standout)
		standoutend();
	xwaitforspace("");
}
#endif

void
tty_suspend_nhwindows(str)
    const char *str;
{
    settty(str);		/* calls end_screen, perhaps raw_print */
    if (!str) tty_raw_print("");	/* calls fflush(stdout) */
}

void
tty_resume_nhwindows()
{
    gettty();
    setftty();			/* calls start_screen */
    docrt();
}

void
tty_exit_nhwindows(str)
    const char *str;
{
    winid i;

    tty_suspend_nhwindows(str);
    /* Just forget any windows existed, since we're about to exit anyway.
     * Disable windows to avoid calls to window routines.
     */
    for(i=0; i<MAXWIN; i++)
	if(i != BASE_WINDOW)
	    wins[i] = 0;
    flags.window_inited = 0;
}

winid
tty_create_nhwindow(type)
    int type;
{
    struct WinDesc* newwin;
    int i;
    int newid;

    if(maxwin == MAXWIN)
	return WIN_ERR;

    newwin = (struct WinDesc*) alloc(sizeof(struct WinDesc));
    newwin->type = type;
    newwin->flags = 0;
    newwin->active = FALSE;
    newwin->curx = newwin->cury = 0;
    newwin->resp = newwin->canresp = newwin->morestr = 0;
    switch(type) {
    case NHW_BASE:
	/* base window, used for absolute movement on the screen */
	newwin->offx = newwin->offy = 0;
	newwin->rows = ttyDisplay->rows;
	newwin->cols = ttyDisplay->cols;
	newwin->maxrow = newwin->maxcol = 0;
	break;
    case NHW_MESSAGE:
	/* message window, 1 line long, very wide, top of screen */
	newwin->offx = newwin->offy = 0;
	/* sanity check */
	if(flags.msg_history < 20) flags.msg_history = 20;
	else if(flags.msg_history > 60) flags.msg_history = 60;
	newwin->maxrow = newwin->rows = flags.msg_history;
	newwin->maxcol = newwin->cols = 0;
	break;
    case NHW_STATUS:
	/* status window, 2 lines long, full width, bottom of screen */
	newwin->offx = 0;
	newwin->offy = min((int)ttyDisplay->rows-2, ROWNO+1);
	newwin->rows = newwin->maxrow = 2;
	newwin->cols = newwin->maxcol = min(ttyDisplay->cols, COLNO);
	break;
    case NHW_MAP:
	/* map window, ROWNO lines long, full width, below message window */
	newwin->offx = 0;
	newwin->offy = 1;
	newwin->rows = ROWNO;
	newwin->cols = COLNO;
	newwin->maxrow = 0;	/* no buffering done -- let gbuf do it */
	newwin->maxcol = 0;
	break;
    case NHW_MENU:
	newwin->resp = (char*) alloc(256);
	newwin->resp[0] = 0;
    case NHW_TEXT:
	/* inventory/menu window, variable length, full width, top of screen */
	/* help window, the same, different semantics for display, etc */
	newwin->offx = newwin->offy = 0;
	newwin->rows = 0;
	newwin->cols = ttyDisplay->cols;
	newwin->maxrow = newwin->maxcol = 0;
	break;
   default:
	panic("Tried to create window type %d\n", (int) type);
	return WIN_ERR;
    }

    for(newid = 0; newid<MAXWIN; newid++) {
	if(wins[newid] == 0) {
	    wins[newid] = newwin;
	    break;
	}
    }
    if(newid == MAXWIN) {
	panic("No window slots!");
	return WIN_ERR;
    }

    if(newwin->maxrow) {
	newwin->data = (char**) alloc(sizeof(char**) * newwin->maxrow);
	if(newwin->maxcol) {
	    for(i=0; i< newwin->maxrow; i++)
		newwin->data[i] = (char*)alloc(sizeof(char*) * newwin->maxcol);
	} else {
	    for(i=0; i< newwin->maxrow; i++)
		newwin->data[i] = 0;
	}
	if(newwin->type == NHW_MESSAGE)
	    newwin->maxrow = 0;
    } else
	newwin->data = 0;

    return newid;
}

void
tty_clear_nhwindow(window)
    winid window;
{
    register struct WinDesc *cw = 0;
    int i;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);
    ttyDisplay->lastwin = window;

    switch(cw->type) {
    case NHW_MESSAGE:
	if(ttyDisplay->toplin) {
	    home();
	    cl_end();
	    if(cw->cury)
		docorner(1, cw->cury+1);
	    ttyDisplay->toplin = 0;
	}
	break;
    case NHW_STATUS:
	tty_curs(window, 1, 0);
	cl_end();
	tty_curs(window, 1, 1);
	cl_end();
	break;
    case NHW_MAP:
	/* cheap -- clear the whole thing and tell nethack to redraw botl */
	flags.botlx = 1;
	/* fall into ... */
    case NHW_BASE:
	clear_screen();
	break;
    case NHW_MENU:
    case NHW_TEXT:
	if(cw->active) {
	    if(cw->offx == 0)
		if(cw->offy) {
		    tty_curs(window, 1, 0);
		    cl_eos();
		} else
		    clear_screen();
	    else
		docorner((int)cw->offx, cw->maxrow+1);
	}
	for(i=0; i<cw->maxrow; i++)
	    if(cw->data[i]) {
		free((genericptr_t)cw->data[i]);
		cw->data[i] = 0;
	    }
	cw->maxrow = cw->maxcol = 0;
	if(cw->resp)
	    cw->resp[0] = 0;
	if(cw->canresp) {
	    free((genericptr_t)cw->canresp);
	    cw->canresp = 0;
	}
	if(cw->morestr) {
	    free((genericptr_t)cw->morestr);
	    cw->morestr = 0;
	}
	break;
    }
    cw->curx = cw->cury = 0;
}

static void
dmore(cw)
    register struct WinDesc *cw;
{
    const char *s = (cw->resp && *cw->resp) ? cw->resp : quitchars;
    const char *prompt = cw->morestr ? cw->morestr : defmorestr;
    if(cw->type == NHW_TEXT)
	tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+1, (int)ttyDisplay->cury);
    else
	tty_curs(BASE_WINDOW, (int)ttyDisplay->curx+2, (int)ttyDisplay->cury);
    if(flags.standout)
	standoutbeg();
    xputs(prompt);
    ttyDisplay->curx += strlen(prompt);
    if(flags.standout)
	standoutend();

    xwaitforspace(s);
}

/*ARGSUSED*/
void
tty_display_nhwindow(window, blocking)
    winid window;
    boolean blocking;	/* with ttys, all windows are blocking */
{
    register struct WinDesc *cw = 0;
    int i, n, attr;
    register char *cp;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);
    if(cw->flags & WIN_CANCELLED)
	return;
    ttyDisplay->lastwin = window;
    ttyDisplay->rawprint = 0;

    switch(cw->type) {
    case NHW_MESSAGE:
	if(ttyDisplay->toplin == 1) {
	    more();
	    ttyDisplay->toplin = 1; /* more resets this */
	    tty_clear_nhwindow(window);
	} else
	    ttyDisplay->toplin = 0;
	cw->curx = cw->cury = 0;
	if(!cw->active)
	    flags.window_inited = TRUE;
	break;
    case NHW_MAP:
	end_glyphout();
	if(blocking) {
	    if(!ttyDisplay->toplin) ttyDisplay->toplin = 1;
	    tty_display_nhwindow(WIN_MESSAGE, TRUE);
	    return;
	}
    case NHW_BASE:
	(void) fflush(stdout);
	break;
    case NHW_TEXT:
	cw->maxcol = ttyDisplay->cols; /* force full-screen mode */
    case NHW_MENU:
	cw->active = 1;
	/* avoid converting to uchar before calculations are finished */
	cw->offx = (uchar) (int)
	    max((int) 10, (int) (ttyDisplay->cols - cw->maxcol - 1));
	if(cw->type == NHW_MENU)
	    cw->offy = 0;
	if(ttyDisplay->toplin == 1)
	    tty_display_nhwindow(WIN_MESSAGE, TRUE);
	if(cw->offx == 10 || cw->maxrow >= (int) ttyDisplay->rows) {
	    cw->offx = 0;
	    if(cw->offy) {
		tty_curs(window, 1, 0);
		cl_eos();
	    } else
		clear_screen();
	    ttyDisplay->toplin = 0;
	} else
	    tty_clear_nhwindow(WIN_MESSAGE);

	for(n=0, i=0; i<cw->maxrow; i++) {
	    if(!cw->offx && (n+cw->offy == ttyDisplay->rows-1)) {
		tty_curs(window, 1, n);
		cl_end();
		dmore(cw);
		if(morc) {
		    if(!cw->canresp && (morc == '\033'))
			cw->flags |= WIN_CANCELLED;
		    else if(cw->canresp && index(&cw->canresp[1], morc)) {
			morc = cw->canresp[0];
			cw->flags |= WIN_CANCELLED;
		    }
		    break;
		}
		if(cw->offy) {
		    tty_curs(window, 1, 0);
		    cl_eos();
		} else
		    clear_screen();
		n = 0;
	    }
	    tty_curs(window, 1, n++);
	    if(cw->offx) cl_end();
	    if(cw->data[i]) {
		attr = cw->data[i][0]-1;
		if(cw->type == NHW_MENU) {
		    (void) putchar(' '); ++ttyDisplay->curx;
		}
		term_start_attr(attr);
		for(cp = &cw->data[i][1];
		    *cp && (int)++ttyDisplay->curx < (int) ttyDisplay->cols; )
#ifdef __SASC
			(void) fputchar(*cp++);
#else
			(void) putchar(*cp++);
#endif
		term_end_attr(attr);
	    }
	}
	if(i == cw->maxrow) {
	    if(cw->type == NHW_TEXT)
		tty_curs(BASE_WINDOW, (int)cw->offx+1, (int)ttyDisplay->rows-1);
	    else
		tty_curs(BASE_WINDOW, (int)cw->offx+1, n);
	    cl_end();
	    dmore(cw);
	    if(morc) {
		if(!cw->canresp && (morc == '\033'))
		    cw->flags |= WIN_CANCELLED;
		else if(cw->canresp && index(&cw->canresp[1], morc)) {
		    morc = cw->canresp[0];
		    cw->flags |= WIN_CANCELLED;
		}
	    }
	}
	break;
    }
    cw->active = 1;
}

void
tty_dismiss_nhwindow(window)
    winid window;
{
    register struct WinDesc *cw = 0;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);

    switch(cw->type) {
    case NHW_STATUS:
    case NHW_BASE:
    case NHW_MESSAGE:
    case NHW_MAP:
	/*
	 * these should only get dismissed when the game is going away
	 * or suspending
	 */
	tty_curs(BASE_WINDOW, 1, (int)ttyDisplay->rows-1);
	cw->active = 0;
	break;
    case NHW_MENU:
    case NHW_TEXT:
	if(cw->active) {
	    if(cw->offx == 0) {
		if(cw->offy) {
		    tty_curs(window, 1, 0);
		    cl_eos();
		} else
		    docrt();
	    } else {
		docorner((int)cw->offx, cw->maxrow+1);
	    }
	    cw->active = 0;
	}
	break;
    }
    cw->flags = 0;
}

void
tty_destroy_nhwindow(window)
    winid window;
{
    register struct WinDesc *cw = 0;
    int i;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);

    if(cw->active)
	tty_dismiss_nhwindow(window);
    if(cw->type == NHW_MESSAGE)
	flags.window_inited = 0;
    if(cw->type == NHW_MAP)
	clear_screen();

    if(cw->data) {
	for(i=0; i<cw->rows; i++)
	    if(cw->data[i])
		free((genericptr_t)cw->data[i]);
	free((genericptr_t)cw->data);
    }
    if(cw->resp)
	free((genericptr_t)cw->resp);
    if(cw->canresp)
	free((genericptr_t)cw->canresp);
    free((genericptr_t)cw);
    wins[window] = 0;
}

void
tty_curs(window, x, y)
winid window;
register int x, y;	/* not xchar: perhaps xchar is unsigned and
			   curx-x would be unsigned as well */
{
    struct WinDesc *cw = 0;
    int cx = ttyDisplay->curx;
    int cy = ttyDisplay->cury;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);
    ttyDisplay->lastwin = window;

    cw->curx = --x;	/* column 0 is never used */
    cw->cury = y;
#ifdef DEBUG
    if(x<0 || y<0 || y >= cw->rows || x >= cw->cols) {
	const char *s = "[unknown type]";
	switch(cw->type) {
	case NHW_MESSAGE: s = "[topl window]"; break;
	case NHW_STATUS: s = "[status window]"; break;
	case NHW_MAP: s = "[map window]"; break;
	case NHW_MENU: s = "[corner window]"; break;
	case NHW_TEXT: s = "[text window]"; break;
	case NHW_BASE: s = "[base window]"; break;
	}
	impossible("bad curs positioning win %d %s (%d,%d)", window, s, x, y);
	return;
    }
#endif
    x += cw->offx;
    y += cw->offy;

#ifdef CLIPPING
    if(clipping && window == WIN_MAP) {
	x -= clipx;
	y -= clipy;
    }
#endif

    if (y == cy && x == cx)
	return;

    if(cw->type == NHW_MAP)
	end_glyphout();

#ifndef NO_TERMS
    if(!ND && (cx != x || x <= 3)) { /* Extremely primitive */
	cmov(x, y); /* bunker!wtm */
	return;
    }
#endif

    if((cy -= y) < 0) cy = -cy;
    if((cx -= x) < 0) cx = -cx;
    if(cy <= 3 && cx <= 3) {
	nocmov(x, y);
#ifndef NO_TERMS
    } else if ((x <= 3 && cy <= 3) || (!CM && x < cx)) {
	(void) putchar('\r');
	ttyDisplay->curx = 0;
	nocmov(x, y);
    } else if (!CM) {
	nocmov(x, y);
#endif
    } else
	cmov(x, y);

    ttyDisplay->curx = x;
    ttyDisplay->cury = y;
}

static void
tty_putsym(window, x, y, ch)
    winid window;
    int x, y;
    char ch;
{
    register struct WinDesc *cw = 0;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);

    switch(cw->type) {
    case NHW_STATUS:
    case NHW_MAP:
    case NHW_BASE:
	tty_curs(window, x, y);
	(void) putchar(ch);
	ttyDisplay->curx++;
	cw->curx++;
	break;
    case NHW_MESSAGE:
    case NHW_MENU:
    case NHW_TEXT:
	impossible("Can't putsym to window type %d", cw->type);
	break;
    }
}


static const char*
compress_str(str)
const char *str;
{
	static char cbuf[BUFSZ];
	/* compress in case line too long */
	if((int)strlen(str) >= CO) {
		register const char *bp0 = str;
		register char *bp1 = cbuf;

		do {
#ifdef CLIPPING
			if(*bp0 != ' ' || bp0[1] != ' ')
#else
			if(*bp0 != ' ' || bp0[1] != ' ' || bp0[2] != ' ')
#endif
				*bp1++ = *bp0;
		} while(*bp0++);
	} else
	    return str;
	return cbuf;
}

void
tty_putstr(window, attr, str)
    winid window;
    int attr;
    const char *str;
{
    register struct WinDesc *cw = 0;
    register char *ob;
    register const char *nb;
    register int i, j, n0;

    /* Assume there's a real problem if the window is missing --
     * probably a panic message
     */
    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0) {
	tty_raw_print(str);
	return;
    }

    if(str == (const char*)NULL || (cw->flags & WIN_CANCELLED))
	return;
    if(cw->type != NHW_MESSAGE)
	str = compress_str(str);

    ttyDisplay->lastwin = window;

    switch(cw->type) {
    case NHW_MESSAGE:
	/* really do this later */
	update_topl(str);
	break;

    case NHW_STATUS:
	ob = &cw->data[cw->cury][j = cw->curx];
	if(flags.botlx) *ob = 0;
	if(!cw->cury && (int)strlen(str) >= CO) {
	    /* the characters before "St:" are unnecessary */
	    nb = index(str, ':');
	    if(nb && nb > str+2)
		str = nb - 2;
	}
	nb = str;
	for(i = cw->curx+1, n0 = cw->cols; i < n0; i++, nb++) {
	    if(!*nb) {
		if(*ob || flags.botlx) {
		    /* last char printed may be in middle of line */
		    tty_curs(WIN_STATUS, i, cw->cury);
		    cl_end();
		}
		break;
	    }
	    if(*ob != *nb)
		tty_putsym(WIN_STATUS, i, cw->cury, *nb);
	    if(*ob) ob++;
	}

	(void) strncpy(&cw->data[cw->cury][j], str, cw->cols - j - 1);
	cw->data[cw->cury][cw->cols-1] = '\0'; /* null terminate */
	cw->cury = (cw->cury+1) % 2;
	cw->curx = 0;
	break;
    case NHW_MAP:
    case NHW_BASE:
	tty_curs(window, cw->curx+1, cw->cury);
	term_start_attr(attr);
	while(*str && (int) ttyDisplay->curx < (int) ttyDisplay->cols-1) {
#ifdef __SASC
	    (void) fputchar(*str++);
#else
	    (void) putchar(*str++);
#endif
	    ttyDisplay->curx++;
	}
	cw->curx = 0;
	cw->cury++;
	term_end_attr(attr);
	break;
    case NHW_MENU:
    case NHW_TEXT:
	if(!(cw->resp && cw->resp[0]) && cw->cury == ttyDisplay->rows-1) {
	    /* not a menu, so save memory and output 1 page at a time */
	    cw->maxcol = ttyDisplay->cols; /* force full-screen mode */
	    tty_display_nhwindow(window, TRUE);
	    cw->maxrow = cw->cury = 0;
	}
	/* always grows one at a time, but alloc 12 at a time */
	if(cw->cury >= cw->rows) {
	    char **tmp;

	    cw->rows += 12;
	    tmp = (char**) alloc(sizeof(char*) * cw->rows);
	    for(i=0; i<cw->maxrow; i++)
		tmp[i] = cw->data[i];
	    if(cw->data)
		free((genericptr_t)cw->data);
	    cw->data = tmp;

	    for(i=cw->maxrow; i<cw->rows; i++)
		cw->data[i] = 0;
	}
	if(cw->data[cw->cury])
	    free((genericptr_t)cw->data[cw->cury]);
	n0 = strlen(str)+1;
	cw->data[cw->cury] = (char*) alloc(n0+1);
	cw->data[cw->cury][0] = attr+1;	/* avoid nuls, for convenience */
	Strcpy(&cw->data[cw->cury][1], str);

	if(n0 > cw->maxcol)
	    cw->maxcol = n0;
	if(++cw->cury > cw->maxrow)
	    cw->maxrow = cw->cury;
	if(n0 > CO) {
	    /* attempt to break the line */
	    for(i = CO-1; i && str[i] != ' ';)
		i--;
	    if(i) {
		cw->data[cw->cury-1][++i] = '\0';
		tty_putstr(window, attr, &str[i]);
	    }

	}
	break;
    }
}

void
tty_display_file(fname, complain)
const char *fname;
boolean complain;
{
#ifdef DEF_PAGER			/* this implies that UNIX is defined */
    {
	/* use external pager; this may give security problems */
	register int fd = open(fname, 0);

	if(fd < 0) {
	    if(complain) pline("Cannot open %s.", fname);
	    else docrt();
	    return;
	}
	if(child(1)) {
	    /* Now that child() does a setuid(getuid()) and a chdir(),
	       we may not be able to open file fname anymore, so make
	       it stdin. */
	    (void) close(0);
	    if(dup(fd)) {
		if(complain) raw_printf("Cannot open %s as stdin.", fname);
	    } else {
		(void) execlp(catmore, "page", NULL);
		if(complain) raw_printf("Cannot exec %s.", catmore);
	    }
	    if(complain) sleep(10); /* want to wait_synch() but stdin is gone */
	    terminate(1);
	}
	(void) close(fd);
    }
#else
    {
#ifdef MAC
	int fd;
#else
	FILE *f;
#endif
	char buf[BUFSZ];
	char *cr;

	tty_clear_nhwindow(WIN_MESSAGE);
#ifdef MAC
	fd = open(fname, O_RDONLY, TEXT_TYPE);
	if (fd < 0) {
#else
	f = fopen_datafile(fname, "r");
	if (!f) {
#endif
	    if(complain) {
		home();  tty_mark_synch();  tty_raw_print("");
		perror(fname);  tty_wait_synch();
		pline("Cannot open \"%s\".", fname);
	    } else if(u.ux) docrt();
	} else {
	    winid datawin = tty_create_nhwindow(NHW_TEXT);
	    if(complain
#ifndef NO_TERMS
		&& CD
#endif
	    ) {
		/* attempt to scroll text below map window if there's room */
		wins[datawin]->offy = wins[WIN_STATUS]->offy+3;
		if((int) wins[datawin]->offy + 12 > (int) ttyDisplay->rows)
		    wins[datawin]->offy = 0;
	    }
#ifdef MAC
	    while (macgets(fd, buf, BUFSZ)) {
#else
	    while (fgets(buf, BUFSZ, f)) {
#endif
		if ((cr = index(buf, '\n')) != 0) *cr = 0;
		if (index(buf, '\t') != 0) (void) tabexpand(buf);
		tty_putstr(datawin, 0, buf);
		if(wins[datawin]->flags & WIN_CANCELLED)
		    break;
	    }
	    tty_display_nhwindow(datawin, FALSE);
	    tty_destroy_nhwindow(datawin);
#ifdef MAC
	    (void) close(fd);
#else
	    (void) fclose(f);
#endif
	}
    }
#endif /* DEF_PAGER */
}

void
tty_start_menu(window)
    winid window;
{
    tty_clear_nhwindow(window);
    return;
}

/*
 * Add a menu item.  window must be an NHW_MENU type,
 * ch is the value to return if this entry is selected.
 * attr are attributes to set for this line (like tty_putstr())
 * str is the value to display on this menu line
 */
void
tty_add_menu(window, ch, attr, str)
    winid window;
    char ch;
    int attr;
    const char *str;
{
    register struct WinDesc *cw = 0;
    char tmpbuf[2];

    if(str == (const char*)NULL)
	return;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0
       || cw->type != NHW_MENU)
	panic(winpanicstr,  window);

    tty_putstr(window, attr, str);
    if(ch != '\0') {
	tmpbuf[0] = ch;
	tmpbuf[1] = 0;
	Strcat(cw->resp, tmpbuf);
    }
}

/*
 * End a menu in this window, window must a type NHW_MENU.
 * ch is the value to return if the menu is canceled,
 * str is a list of cancel characters (values that may be input)
 * morestr is a prompt to display, rather than the default.
 * str and morestr might be ignored by some ports.
 */
void
tty_end_menu(window, ch, str, morestr)
    winid window;
    char ch;
    const char *str;
    const char *morestr;
{
    register struct WinDesc *cw = 0;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0 ||
       cw->type != NHW_MENU || cw->canresp)
	panic(winpanicstr,  window);

    if(str) {
	cw->canresp = (char*) alloc(strlen(str)+2);
	cw->canresp[0] = ch;			/* this could be NUL? */
	Strcpy(&cw->canresp[1], str);
	Strcat(cw->resp, str);
    }
    if(morestr) {
	unsigned int len = strlen(morestr) + 1;
	cw->morestr = (char*) alloc(len);
	Strcpy(cw->morestr, morestr);
	if(++len > cw->maxcol)	/* add one to avoid using the rtmost column */
	    cw->maxcol = len;
    }
}

char
tty_select_menu(window)
    winid window;
{
    register struct WinDesc *cw = 0;

    morc = 0;
    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0
       || cw->type != NHW_MENU)
	panic(winpanicstr,  window);
    tty_display_nhwindow(window, TRUE);
    tty_dismiss_nhwindow(window);

    return morc;
}

void
tty_update_inventory()
{
}

void
tty_mark_synch()
{
    (void) fflush(stdout);
}

void
tty_wait_synch()
{
    /* we just need to make sure all windows are synch'd */
    if(!ttyDisplay || ttyDisplay->rawprint) {
	getret();
	if(ttyDisplay) ttyDisplay->rawprint = 0;
    } else {
	tty_display_nhwindow(WIN_MAP, FALSE);
	if(ttyDisplay->inmore) {
	    addtopl("--More--");
	    (void) fflush(stdout);
	} else if(ttyDisplay->inread) {
	    /* this can only happen if we were reading and got interrupted */
	    ttyDisplay->toplin = 3;
	    /* do this twice; 1st time gets the Quit? message again */
	    (void) tty_doprev_message();
	    (void) tty_doprev_message();
	    ttyDisplay->intr++;
	    (void) fflush(stdout);
	}
    }
}

void
docorner(xmin, ymax)
    register int xmin, ymax;
{
    register int y;
    register struct WinDesc *cw = wins[WIN_MAP];

    if (u.uswallow) {	/* Can be done more efficiently */
	swallowed(1);
	return;
    }

#if defined(SIGWINCH) && defined(CLIPPING)
    if(ymax > LI) ymax = LI;		/* can happen if window gets smaller */
#endif
    for (y = 0; y < ymax; y++) {
	tty_curs(BASE_WINDOW, xmin,y);	/* move cursor */
	cl_end();			/* clear to end of line */
#ifdef CLIPPING
	if (y<(int) cw->offy || y+clipy > ROWNO)
		continue; /* only refresh board */
	row_refresh(xmin+clipx-(int)cw->offx,COLNO-1,y+clipy-(int)cw->offy);
#else
	if (y<cw->offy || y > ROWNO) continue; /* only refresh board  */
	row_refresh(xmin-(int)cw->offx,COLNO-1,y-(int)cw->offy);
#endif
    }

    end_glyphout();
    if (ymax >= (int) wins[WIN_STATUS]->offy) {
					/* we have wrecked the bottom line */
	flags.botlx = 1;
	bot();
    }
}

void
end_glyphout()
{
#if defined(ASCIIGRAPH) && !defined(NO_TERMS)
    if (GFlag) {
	GFlag = FALSE;
	graph_off();
    }
#endif
#ifdef TEXTCOLOR
    if(ttyDisplay->color != NO_COLOR) {
	term_end_color();
	ttyDisplay->color = NO_COLOR;
    }
#endif
}

void
g_putch(in_ch)
int in_ch;
{
    register char ch = (char)in_ch;

# if defined(ASCIIGRAPH) && !defined(NO_TERMS)
    if (flags.IBMgraphics)
	/* IBM-compatible displays don't need other stuff */
	(void) putchar(ch);
    else if (ch & 0x80) {
	if (!GFlag) {
	    graph_on();
	    GFlag = TRUE;
	}
	(void) putchar((ch ^ 0x80)); /* Strip 8th bit */
    } else {
	if (GFlag) {
	    graph_off();
	    GFlag = FALSE;
	}
	(void) putchar(ch);
    }

#else
    (void) putchar(ch);

#endif	/* ASCIIGRAPH && !NO_TERMS */

    return;
};

#ifdef CLIPPING
void
setclipped()
{
	clipping = TRUE;
	clipx = clipy = 0;
	clipxmax = CO;
	clipymax = LI - 3;
}

void
tty_cliparound(x, y)
int x, y;
{
	int oldx = clipx, oldy = clipy;

	if (!clipping) return;
	if (x < clipx + 5) {
		clipx = max(0, x - 20);
		clipxmax = clipx + CO;
	}
	else if (x > clipxmax - 5) {
		clipxmax = min(COLNO, clipxmax + 20);
		clipx = clipxmax - CO;
	}
	if (y < clipy + 2) {
		clipy = max(0, y - (clipymax - clipy) / 2);
		clipymax = clipy + (LI - 3);
	}
	else if (y > clipymax - 2) {
		clipymax = min(ROWNO, clipymax + (clipymax - clipy) / 2);
		clipy = clipymax - (LI - 3);
	}
	if (clipx != oldx || clipy != oldy) {
		(void) doredraw();
	}
}
#endif /* CLIPPING */


/*
 *  tty_print_glyph
 *
 *  Print the glyph to the output device.  Don't flush the output device.
 *
 *  Since this is only called from show_glyph(), it is assumed that the
 *  position and glyph are always correct (checked there)!
 */
void
tty_print_glyph(window, x, y, glyph)
    winid window;
    xchar x, y;
    int glyph;
{
    uchar   ch;
    register int offset;
#ifdef TEXTCOLOR
    int	    color;

#define zap_color(n)  color = flags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) color = flags.use_color ? defsyms[n].color : NO_COLOR
#define trap_color(n) color = flags.use_color ? \
				(((n) == WEB) ? defsyms[S_web ].color  : \
						defsyms[S_trap].color) : \
						NO_COLOR
#define obj_color(n)  color = flags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = flags.use_color ? mons[n].mcolor : NO_COLOR
#define pet_color(n)  color = flags.use_color ? mons[n].mcolor :	      \
				/* If no color, try to hilite pets; black  */ \
				/* should be HI				   */ \
				((flags.hilite_pet && has_color(BLACK)) ?     \
							BLACK : NO_COLOR)

# else /* no text color */

#define zap_color(n)
#define cmap_color(n)
#define trap_color(n)
#define obj_color(n)
#define mon_color(n)
#define pet_color(c)
#endif

#ifdef CLIPPING
    if(clipping) {
	if(x <= clipx || y < clipy || x >= clipxmax || y >= clipymax)
	    return;
    }
#endif
    /*
     *  Map the glyph back to a character.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *		  offsets.  The order is set in display.h.
     */
    if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) {		/* swallow */
	/* see swallow_to_glyph() in display.c */
	ch = (uchar) showsyms[S_sw_tl + (offset & 0x7)];
	mon_color(offset >> 3);
    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) {	/* zap beam */
	/* see zapdir_to_glyph() in display.c */
	ch = showsyms[S_vbeam + (offset & 0x3)];
	zap_color((offset >> 2));
    } else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) {	/* cmap */
	ch = showsyms[offset];
	cmap_color(offset);
    } else if ((offset = (glyph - GLYPH_TRAP_OFF)) >= 0) {	/* trap */
	ch = (offset == WEB) ? showsyms[S_web] : showsyms[S_trap];
	trap_color(offset);
    } else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) {	/* object */
	ch = oc_syms[(int)objects[offset].oc_class];
	obj_color(offset);
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) {	/* a corpse */
	ch = oc_syms[(int)objects[CORPSE].oc_class];
	mon_color(offset);
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {	/* a pet */
	ch = monsyms[(int)mons[offset].mlet];
	pet_color(offset);
    } else {							/* a monster */
	ch = monsyms[(int)mons[glyph].mlet];
	mon_color(glyph);
    }

    /* Move the cursor. */
    tty_curs(window, x,y);

#ifndef NO_TERMS
    if (ul_hack && ch == '_') {		/* non-destructive underscore */
	(void) putchar((char) ' ');
	backsp();
    }
#endif

#ifdef TEXTCOLOR
    /* Turn off color if no color defined, or rogue level. */
#  ifdef REINCARNATION
    if (!has_color(color) || Is_rogue_level(&u.uz))
#  else
    if (!has_color(color))
#  endif
	color = NO_COLOR;

    if (color != ttyDisplay->color) {
	if(ttyDisplay->color != NO_COLOR)
	    term_end_color();
	ttyDisplay->color = color;
	if(color != NO_COLOR)
	    term_start_color(color);
    }
#endif
    g_putch((int)ch);		/* print the character */
    wins[window]->curx++;	/* one character over */
    ttyDisplay->curx++;		/* the real cursor moved too */
}

void
tty_raw_print(str)
    const char *str;
{
    if(ttyDisplay) ttyDisplay->rawprint++;
#ifdef MICRO
    msmsg("%s\n", str);
#else
    puts(str); (void) fflush(stdout);
#endif
}

void
tty_raw_print_bold(str)
    const char *str;
{
    if(ttyDisplay) ttyDisplay->rawprint++;
    term_start_raw_bold();
#ifdef MICRO
    msmsg("%s", str);
#else
    (void) fputs(str, stdout);
#endif
    term_end_raw_bold();
#ifdef MICRO
    msmsg("\n");
#else
    puts("");
    (void) fflush(stdout);
#endif
}

int
tty_nhgetch()
{
    int i;
#ifdef UNIX
    /* kludge alert: Some Unix variants return funny values if getc()
     * is called, interrupted, and then called again.  There
     * is non-reentrant code in the internal _filbuf() routine, called by
     * getc().
     */
    static volatile int nesting = 0;
    char nestbuf;
#endif

    (void) fflush(stdout);
    /* Note: if raw_print() and wait_synch() get called to report terminal
     * initialization problems, then wins[] and ttyDisplay might not be
     * available yet.  Such problems will probably be fatal before we get
     * here, but validate those pointers just in case...
     */
    if (WIN_MESSAGE != WIN_ERR && wins[WIN_MESSAGE])
	    wins[WIN_MESSAGE]->flags &= ~WIN_STOP;
#ifdef UNIX
    i = ((++nesting == 1) ? tgetch() :
	 (read(fileno(stdin), (genericptr_t)&nestbuf,1) == 1 ? (int)nestbuf :
								EOF));
    --nesting;
#else
    i = tgetch();
#endif
    if (!i) i = '\033'; /* map NUL to ESC since nethack doesn't expect NUL */
    if (ttyDisplay && ttyDisplay->toplin == 1)
	ttyDisplay->toplin = 2;
    return i;
}

/*
 * return a key, or 0, in which case a mouse button was pressed
 * mouse events should be returned as character postitions in the map window.
 * Since normal tty's don't have mice, just return a key.
 */
/*ARGSUSED*/
int
tty_nh_poskey(x, y, mod)
    int *x, *y, *mod;
{
    return tty_nhgetch();
}

void
win_tty_init()
{
# if defined(WIN32CON)
    nttty_open();
# endif
    return;
}

#endif /* TTY_GRAPHICS */
/*wintty.c*/
