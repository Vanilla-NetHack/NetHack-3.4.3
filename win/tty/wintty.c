/*	SCCS Id: @(#)wintty.c	3.2	96/03/16	*/
/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Neither a standard out nor character-based control codes should be
 * part of the "tty look" windowing implementation.
 * h+ 930227
 */

#include "hack.h"
#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

#ifdef TTY_GRAPHICS

#ifdef MAC
# define MICRO /* The Mac is a MICRO only for this file, not in general! */
# ifdef THINK_C
extern void msmsg(const char *,...);
# endif
#endif


#ifndef NO_TERMS
#include "termcap.h"
#endif

#include "wintty.h"

#ifdef CLIPPING		/* might want SIGWINCH */
# if defined(BSD) || defined(ULTRIX) || defined(AIX_31) || defined(_BULL_SOURCE)
#include <signal.h>
# endif
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
    tty_message_menu,
    tty_update_inventory,
    tty_mark_synch,
    tty_wait_synch,
#ifdef CLIPPING
    tty_cliparound,
#endif
#ifdef POSITIONBAR
    tty_update_positionbar,
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
    tty_get_ext_cmd,
    tty_number_pad,
    tty_delay_output,
#ifdef CHANGE_COLOR	/* the Mac uses a palette device */
    tty_change_color,
    tty_get_color_string,
#endif

    /* other defs that really should go away (they're tty specific) */
    tty_start_screen,
    tty_end_screen,
    genl_outrip
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

/*
 * Standard menu manipulation accelerators.  These may _not_ be:
 *
 *	+ a number - reserved for counts
 *	+ an upper or lower case US ASCII letter - used for accelerators
 *	+ ESC - reserved for escaping the menu
 *	+ NULL, CR or LF - reserved for commiting the selection(s).  NULL
 *	  is kind of odd, but xwaitforspace() will return it if someone
 *	  hits a <ret>.
 *
 * Standard letters (for now) are:
 *
 *		<  back 1 page
 *		>  forward 1 page
 *		^  first page
 *		$  last page
 *
 *		page		all
 *		 +    select	 *
 *		 -    deselect	 %
 *		 ~    invert	 @
 *
 * The above letter display a definite UNIX slant:
 * '$' would be great for gold, but is the obvious UNIX choice for last.
 * '^' would be good for one of the inverts, but is more logically "top"
 *	for most UNIX commands.
 * '!' would be nice for one of the inverts, but is also the UNIX shell escape.
 */
#define MENU_PREVIOUS_PAGE '<'
#define MENU_NEXT_PAGE '>'
#define MENU_FIRST_PAGE '^'
#define MENU_LAST_PAGE '$'
#define MENU_SET_PAGE '+'
#define MENU_UNSET_PAGE '-'
#define MENU_INVERT_PAGE '~'
#define MENU_SET_ALL '*'
#define MENU_UNSET_ALL '%'
#define MENU_INVERT_ALL '@'

static const char standard_menu_chars[] = {
    MENU_PREVIOUS_PAGE,
    MENU_NEXT_PAGE,
    MENU_FIRST_PAGE,
    MENU_LAST_PAGE,
    MENU_SET_PAGE,
    MENU_UNSET_PAGE,
    MENU_INVERT_PAGE,
    MENU_SET_ALL,
    MENU_UNSET_ALL,
    MENU_INVERT_ALL,
    0 /* must end in null */
};

/*
 * Allow the standard menu accelerators to have aliases.  Right now, this
 * is internal only.  We need a way to do it independently.
 */
#define MAX_MENU_MAPPED 16	/* some number */
static char mapped_menu_accelerators[MAX_MENU_MAPPED+1];
static char menu_mapped_op[MAX_MENU_MAPPED+1];
static short n_menu_mapped = 0;



#ifdef CLIPPING
# if defined(USE_TILES) && defined(MSDOS)
boolean clipping = FALSE;	/* clipping on? */
int clipx = 0, clipxmax = 0;
# else
static boolean clipping = FALSE;	/* clipping on? */
static int clipx = 0, clipxmax = 0;
# endif
static int clipy = 0, clipymax = 0;
#endif /* CLIPPING */

#if defined(USE_TILES) && defined(MSDOS)
# ifdef SIMULATE_CURSOR
extern int cursor_flag;
# endif
extern boolean tiles_on;
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
static void FDECL(erase_menu_or_text, (winid, struct WinDesc *, BOOLEAN_P));
static void FDECL(free_window_info, (struct WinDesc *, BOOLEAN_P));
static void FDECL(dmore,(struct WinDesc *, const char *));
static void FDECL(set_item_state, (winid, int, tty_menu_item *));
static void FDECL(set_all_on_page, (winid,tty_menu_item *,tty_menu_item *));
static void FDECL(unset_all_on_page, (winid,tty_menu_item *,tty_menu_item *));
static void FDECL(invert_all_on_page, (winid,tty_menu_item *,tty_menu_item *));
static void FDECL(add_menu_alias, (CHAR_P, CHAR_P));
static char FDECL(map_menu_accelerator, (CHAR_P));
static tty_menu_item *FDECL(reverse, (tty_menu_item *));
static const char * FDECL(compress_str, (const char *));
static void FDECL(tty_putsym, (winid, int, int, CHAR_P));
static char *FDECL(copy_of, (const char *));
static void FDECL(bail, (const char *));	/* __attribute__((noreturn)) */


/* clean up and quit */
static void
bail(mesg)
const char *mesg;
{
    clearlocks();
    tty_exit_nhwindows(mesg);
    terminate(EXIT_SUCCESS);
    /*NOTREACHED*/
}

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

/*ARGSUSED*/
void
tty_init_nhwindows(argcp,argv)
int* argcp;
char** argv;
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

    /* init accelerator list, then add one */
    mapped_menu_accelerators[0] = 0;
    menu_mapped_op[0] = 0;;
    add_menu_alias(' ', MENU_NEXT_PAGE);

    tty_clear_nhwindow(BASE_WINDOW);

    tty_putstr(BASE_WINDOW, 0, "");
    tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_A);
    tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_B);
    tty_putstr(BASE_WINDOW, 0, COPYRIGHT_BANNER_C);
    tty_putstr(BASE_WINDOW, 0, "");
    tty_display_nhwindow(BASE_WINDOW, FALSE);
}

void
tty_player_selection()
{
    char pbuf[QBUFSZ];
    char pick, pc;
    int i, echoline;

    if ((pc = highc(pl_character[0])) != 0) {
	if(index(pl_classes, pc) != (char*) 0)
	    goto got_suffix;
	tty_putstr(BASE_WINDOW, 0, "");
	Sprintf(pbuf, "Unknown role: %c", pc);
	tty_putstr(BASE_WINDOW, 0, pbuf);
	pl_character[0] = pc = 0;
    }

#define PICK_PROMPT "Shall I pick a character for you? [Y, N, or Q(quit)] "
    tty_putstr(BASE_WINDOW, 0, "");
    echoline = wins[BASE_WINDOW]->cury;
    tty_putstr(BASE_WINDOW, 0, PICK_PROMPT);

    while(!index("yYnNqQ", (pick = readchar())) && !index(quitchars, pick))
	tty_nhbell();

    pick = index(quitchars, pick) ? 'Y' : highc(pick);

    if ((int)strlen(PICK_PROMPT) + 1 < CO) {
	tty_putsym(BASE_WINDOW, (int)strlen(PICK_PROMPT)+1, echoline, pick);
	tty_putstr(BASE_WINDOW, 0, "");	/* move back down line after echo */
    } else {
	/* otherwise it's hard to tell where to echo, and things are
	 * wrapping a bit messily anyway, so (try to) make sure the next
	 * question shows up well and doesn't get wrapped at the bottom of
	 * the window
	 */
	tty_clear_nhwindow(BASE_WINDOW);
    }

    if (pick == 'Q') {
	bail((char *)0);
	/*NOTREACHED*/
    }

    if (pick == 'Y') {
	goto beginner;
    }

    tty_putstr(BASE_WINDOW, 0, "");
    tty_putstr(BASE_WINDOW, 0, "What kind of character are you:");
    tty_putstr(BASE_WINDOW, 0, "");
    Sprintf(pbuf, "        %s, ", An(roles[0]));
    for(i = 1; roles[i]; i++) {
	Strcat(pbuf, an(roles[i]));
	if (roles[i+1]) {
	    Strcat(pbuf, ", ");
	    if ((int)strlen(pbuf) + (int)strlen(roles[i+1]) + 8 > CO) {
		/* 8 for ", or an " */
		tty_putstr(BASE_WINDOW, 0, pbuf);
		Strcpy(pbuf, "        ");
	    }
	    if (!roles[i+2]) Strcat(pbuf, "or ");
	}
    }
    Strcat(pbuf ,"?");
    tty_putstr(BASE_WINDOW, 0, pbuf);
    Strcpy(pbuf, "        [");
    for(i = 0; roles[i]; i++)
	Sprintf(eos(pbuf), "%c,", pl_classes[i]);
    Strcat(pbuf, " or Q] ");
    echoline = wins[BASE_WINDOW]->cury;
    tty_putstr(BASE_WINDOW, 0, pbuf);

    while ((pc = readchar()) != 0) {
	if ((pc = highc(pc)) == 'Q') {
	    bail((char *)0);
	    /*NOTREACHED*/
	}
	if(index(pl_classes, pc) != (char *) 0) {
	    if ((int)strlen(pbuf) + 1 < CO) {
		tty_putsym(BASE_WINDOW, (int)strlen(pbuf)+1, echoline, pc);
		tty_putstr(BASE_WINDOW, 0, "");
	    }
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
    static char who_are_you[] = "Who are you? ";
    register int c, ct, tryct = 0;

    tty_putstr(BASE_WINDOW, 0, "");
    do {
	if (++tryct > 1) {
	    if (tryct > 10) bail("Giving up after 10 tries.\n");
	    tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury - 1);
	    tty_putstr(BASE_WINDOW, 0, "Enter a name for your character...");
	    /* erase previous prompt (in case of ESC after partial response) */
	    tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury),  cl_end();
	}
	tty_putstr(BASE_WINDOW, 0, who_are_you);
	tty_curs(BASE_WINDOW, (int)(sizeof who_are_you),
	         wins[BASE_WINDOW]->cury - 1);
	ct = 0;
	while((c = tty_nhgetch()) != '\n') {
		if(c == EOF) error("End of input\n");
		if (c == '\033') { ct = 0; break; }  /* continue outer loop */
		/* some people get confused when their erase char is not ^H */
		if (c == '\b' || c == '\177') {
			if(ct) {
				ct--;
#ifdef MICRO
# if defined(WIN32CON)
				backsp();       /* \b is visible on NT */
# else
#  if defined(MSDOS)
				if (flags.grmode) {
					backsp();
				} else

#  endif
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
		if (ct < (int)(sizeof plname) - 1) {
#if defined(MICRO)
# if defined(MSDOS)
			if (flags.grmode) {
				(void) putchar(c);
			} else
# endif
			msmsg("%c", c);
#else
			(void) putchar(c);
#endif
			plname[ct++] = c;
		}
	}
	plname[ct] = 0;
    } while (ct == 0);

    /* move to next line to simulate echo of user's <return> */
    tty_curs(BASE_WINDOW, 1, wins[BASE_WINDOW]->cury + 1);
}

void
tty_get_nh_event()
{
    return;
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
	xwaitforspace(" ");
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
	if (wins[i] && (i != BASE_WINDOW)) {
#ifdef FREE_ALL_MEMORY
	    free_window_info(wins[i], TRUE);
	    free((genericptr_t) wins[i]);
#endif
	    wins[i] = 0;
	}
#ifndef NO_TERMS	/*(until this gets added to the window interface)*/
    tty_shutdown();		/* cleanup termcap/terminfo/whatever */
#endif
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
    newwin->morestr = 0;
    newwin->mlist = (tty_menu_item *) 0;
    newwin->plist = (tty_menu_item **) 0;
    newwin->npages = newwin->plist_size = newwin->nitems = newwin->how = 0;
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
#if defined(USE_TILES) && defined(MSDOS)
	if (flags.grmode) {
		newwin->offy = ttyDisplay->rows-2;
	} else
#endif
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
	newwin->data =
		(char **) alloc(sizeof(char *) * (unsigned)newwin->maxrow);
	if(newwin->maxcol) {
	    for(i=0; i< newwin->maxrow; i++)
		newwin->data[i] =
		    (char *) alloc(sizeof(char) * (unsigned)newwin->maxcol);
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

static void
erase_menu_or_text(window, cw, clear)
    winid window;
    struct WinDesc *cw;
    boolean clear;
{
    if(cw->offx == 0)
	if(cw->offy) {
	    tty_curs(window, 1, 0);
	    cl_eos();
	} else if (clear)
	    clear_screen();
	else
	    docrt();
    else
	docorner((int)cw->offx, cw->maxrow+1);
}

static void
free_window_info(cw, free_data)
    struct WinDesc *cw;
    boolean free_data;
{
    int i;

    if (cw->data) {
	if (cw == wins[WIN_MESSAGE] && cw->rows > cw->maxrow)
	    cw->maxrow = cw->rows;		/* topl data */
	for(i=0; i<cw->maxrow; i++)
	    if(cw->data[i]) {
		free((genericptr_t)cw->data[i]);
		cw->data[i] = 0;
	    }
	if (free_data) {
	    free((genericptr_t)cw->data);
	    cw->data = 0;
	    cw->rows = 0;
	}
    }
    cw->maxrow = cw->maxcol = 0;
    if(cw->mlist) {
	tty_menu_item *temp;
	while ((temp = cw->mlist) != 0) {
	    cw->mlist = cw->mlist->next;
	    if (temp->str) free((genericptr_t)temp->str);
	    free((genericptr_t)temp);
	}
    }
    if (cw->plist) {
	free((genericptr_t)cw->plist);
	cw->plist = 0;
    }
    cw->plist_size = cw->npages = cw->nitems = cw->how = 0;
    if(cw->morestr) {
	free((genericptr_t)cw->morestr);
	cw->morestr = 0;
    }
}

void
tty_clear_nhwindow(window)
    winid window;
{
    register struct WinDesc *cw = 0;

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
	if(cw->active)
	    erase_menu_or_text(window, cw, TRUE);
	free_window_info(cw, FALSE);
	break;
    }
    cw->curx = cw->cury = 0;
}

static void
dmore(cw, s)
    register struct WinDesc *cw;
    const char *s;			/* valid responses */
{
    const char *prompt = cw->morestr ? cw->morestr : defmorestr;
    int offset = (cw->type == NHW_TEXT) ? 1 : 2;

    tty_curs(BASE_WINDOW,
	     (int)ttyDisplay->curx + offset, (int)ttyDisplay->cury);
    if(flags.standout)
	standoutbeg();
    xputs(prompt);
    ttyDisplay->curx += strlen(prompt);
    if(flags.standout)
	standoutend();

    xwaitforspace(s);
}

static void
set_item_state(window, lineno, item)
    winid window;
    int lineno;
    tty_menu_item *item;
{
    char ch = item->selected ? (item->count == -1L ? '+' : '#') : '-';
    tty_curs(window, 4, lineno);
    term_start_attr(item->attr);
    (void) putchar(ch);
    ttyDisplay->curx++;
    term_end_attr(item->attr);
}

static void
set_all_on_page(window, page_start, page_end)
    winid window;
    tty_menu_item *page_start, *page_end;
{
    tty_menu_item *curr;
    int n;

    for (n = 0, curr = page_start; curr != page_end; n++, curr = curr->next)
	if (curr->identifier.a_void && !curr->selected) {
	    curr->selected = TRUE;
	    set_item_state(window, n, curr);
	}
}

static void
unset_all_on_page(window, page_start, page_end)
    winid window;
    tty_menu_item *page_start, *page_end;
{
    tty_menu_item *curr;
    int n;

    for (n = 0, curr = page_start; curr != page_end; n++, curr = curr->next)
	if (curr->identifier.a_void && curr->selected) {
	    curr->selected = FALSE;
	    curr->count = -1L;
	    set_item_state(window, n, curr);
	}
}

static void
invert_all_on_page(window, page_start, page_end)
    winid window;
    tty_menu_item *page_start, *page_end;
{
    tty_menu_item *curr;
    int n;

    for (n = 0, curr = page_start; curr != page_end; n++, curr = curr->next)
	if (curr->identifier.a_void) {
	    if (curr->selected) {
		curr->selected = FALSE;
		curr->count = -1L;
	    } else
		curr->selected = TRUE;
	    set_item_state(window, n, curr);
	}
}

static void
add_menu_alias(from_ch, to_ch)
    char from_ch, to_ch;
{
    if (n_menu_mapped < MAX_MENU_MAPPED) {
	if (index(standard_menu_chars, to_ch)) {
	    mapped_menu_accelerators[n_menu_mapped] = from_ch;
	    menu_mapped_op[n_menu_mapped] = to_ch;
	    n_menu_mapped++;
	} else
	    pline("add_menu_alias: must map to standard char");
    } else
	pline("add_menu_alias: overflow");
}

static char
map_menu_accelerator(ch)
    char ch;
{
    char *found = index(mapped_menu_accelerators, ch);
    if (found) {
	int idx = found - mapped_menu_accelerators;
	ch = menu_mapped_op[idx];
    }
    return ch;
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

	if (cw->mlist) {
	    int curr_page, page_lines;
	    tty_menu_item *page_start, *page_end, *curr;
	    boolean finished, counting, reset_count;
	    char *rp, resp[QBUFSZ], *msave, morestr[QBUFSZ];
	    long count;

	    curr_page = page_lines = 0;
	    curr_page = 0;
	    page_start = page_end = 0;
	    msave = cw->morestr;	/* save the morestr */
	    cw->morestr = morestr;
	    counting = FALSE;
	    count = 0;
	    reset_count = TRUE;
	    finished = FALSE;

	    /* loop until finished */
	    while (!finished) {
		if (reset_count) {
		    counting = FALSE;
		    count = 0;
		} else
		    reset_count = TRUE;

		if (!page_start) {
		    /* new page to be displayed */
		    if (curr_page < 0 || curr_page >= cw->npages)
			panic("bad menu screen page #%d", curr_page);

		    /* clear screen */
		    if (!cw->offx) {	/* if not corner, do clearscreen */
			if(cw->offy) {
			    tty_curs(window, 1, 0);
			    cl_eos();
			} else
			    clear_screen();
		    }

		    /* collect accelerators */
		    page_start = cw->plist[curr_page];
		    page_end = cw->plist[curr_page+1];
		    rp = resp;
		    for (page_lines = 0, curr = page_start;
			    curr != page_end; page_lines++, curr = curr->next) {
			if (curr->selector)
			    *rp++ = curr->selector;

			tty_curs(window, 1, page_lines);
			if(cw->offx) cl_end();

			(void) putchar(' ');
			++ttyDisplay->curx;
			/*
			 * Don't use xputs() because (1) under unix it calls
			 * tputstr() which will interpret a '*' as some kind
			 * of padding information and (2) it calls xputc to
			 * actually output the character.  We're faster doing
			 * this.
			 */
			term_start_attr(curr->attr);
			for(n = 0, cp = curr->str; *cp &&
			    (int)++ttyDisplay->curx < (int) ttyDisplay->cols;
			    cp++, n++)
			    if (n == 2 && curr->identifier.a_void != 0 &&
							    curr->selected) {
				if (curr->count == -1L)
				    (void) putchar('+'); /* all selected */
				else
				    (void) putchar('#'); /* count selected */
			    } else
				(void) putchar(*cp);
			term_end_attr(curr->attr);
		    }
		    *rp = 0;

		    /* corner window - clear extra lines from last page */
		    if (cw->offx) {
			for (n = page_lines+1; n < cw->maxrow; n++) {
			    tty_curs(window, 1, n);
			    cl_end();
			}
		    }

		    /* set extra chars.. */
		    Strcat(resp, standard_menu_chars);
		    Strcat(resp, "0123456789\033\n\r");	/* counts, quit */
		    Strcat(resp, mapped_menu_accelerators);

		    if (cw->npages > 1)
			Sprintf(cw->morestr, "(%d of %d)",
				curr_page+1, (int) cw->npages);
		    else
			Strcpy(cw->morestr, msave);

		    tty_curs(window, 1, page_lines);
		    cl_end();
		    dmore(cw, resp);
		} else {
		    /* just put the cursor back... */
		    tty_curs(window, strlen(cw->morestr)+2, page_lines);
		    xwaitforspace(resp);
		}

		morc = map_menu_accelerator(morc);
		switch (morc) {
		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
			count = (count * 10L) + (long) (morc - '0');
			/*
			 * It is debatable whether we should allow 0 to
			 * start a count.  There is no difference if the
			 * item is selected.  If not selected, then
			 * "0b" could mean:
			 *
			 *	count starting zero:	"zero b's"
			 *	ignore starting zero:	"select b"
			 *
			 * At present I don't know which is better.
			 */
			if (count != 0L) {	/* ignore leading zeros */
			    counting = TRUE;
			    reset_count = FALSE;
			}
			break;
		    case '\033':	/* cancel - from counting or loop */
			if (!counting) {
			    /* deselect everything */
			    for (curr = cw->mlist; curr; curr = curr->next) {
				curr->selected = FALSE;
				curr->count = -1;
			    }
			    cw->flags |= WIN_CANCELLED;
			    finished = TRUE;
			}
			/* else only stop count */
			break;
		    case '\0':		/* finished (commit) */
		    case '\n':
		    case '\r':
			/* only finished if we are actually picking something */
			if (cw->how != PICK_NONE) {
			    finished = TRUE;
			    break;
			}
			/* else fall through */
		    case MENU_NEXT_PAGE:
			if (curr_page != cw->npages-1) {
			    curr_page++;
			    page_start = 0;
			} else
			    finished = TRUE;	/* questionable behavior */
			break;
		    case MENU_PREVIOUS_PAGE:
			if (curr_page != 0) {
			    --curr_page;
			    page_start = 0;
			}
			break;
		    case MENU_FIRST_PAGE:
			if (curr_page != 0) {
			    page_start = 0;
			    curr_page = 0;
			}
			break;
		    case MENU_LAST_PAGE:
			if (curr_page != cw->npages-1) {
			    page_start = 0;
			    curr_page = cw->npages-1;
			}
			break;
		    case MENU_SET_PAGE:
			if (cw->how != PICK_ONE)
			    set_all_on_page(window, page_start, page_end);
			break;
		    case MENU_UNSET_PAGE:
			if (cw->how != PICK_ONE)
			    unset_all_on_page(window, page_start, page_end);
			break;
		    case MENU_INVERT_PAGE:
			if (cw->how != PICK_ONE)
			    invert_all_on_page(window, page_start, page_end);
			break;
		    case MENU_SET_ALL:
			if (cw->how == PICK_ONE) break;

			set_all_on_page(window, page_start, page_end);
			/* set the rest */
			for (curr = cw->mlist; curr; curr = curr->next)
			    if (curr->identifier.a_void && !curr->selected)
				curr->selected = TRUE;
			break;
		    case MENU_UNSET_ALL:
			if (cw->how == PICK_ONE) break;

			unset_all_on_page(window, page_start, page_end);
			/* unset the rest */
			for (curr = cw->mlist; curr; curr = curr->next)
			    if (curr->identifier.a_void && curr->selected) {
				curr->selected = FALSE;
				curr->count = -1;
			    }
			break;
		    case MENU_INVERT_ALL: {
			boolean on_curr_page = FALSE;

			if (cw->how == PICK_ONE) break;

			invert_all_on_page(window, page_start, page_end);
			/* invert the rest */
			for (curr = cw->mlist; curr; curr = curr->next) {
			    if (curr == page_start)
				on_curr_page = TRUE;
			    else if (curr == page_end)
				on_curr_page = FALSE;
			    if (!on_curr_page && curr->identifier.a_void) {
				if (curr->selected) {
				    curr->selected = FALSE;
				    curr->count = -1;
				} else
				    curr->selected = TRUE;
			    }
			}
			break;
		    }
		    default:
			if (cw->how != PICK_NONE && index(resp, morc)) {
			    int nn;	/* nth row */
			    tty_menu_item *ncurr;

			    /* find, toggle, and possibly update */
			    for (nn = 0, ncurr = page_start; ncurr != page_end;
						    nn++, ncurr = ncurr->next)
				if (morc == ncurr->selector) {
				    if (ncurr->selected) {
					if (counting && count > 0) {
					    ncurr->count = count;
					    set_item_state(window, nn, ncurr);
					} else { /* change state */
					    ncurr->selected = FALSE;
					    ncurr->count = -1L;
					    set_item_state(window, nn, ncurr);
					}
				    } else {	/* !selected */
					if (counting && count > 0) {
					    ncurr->count = count;
					    ncurr->selected = TRUE;
					    set_item_state(window, nn, ncurr);
					} else if (!counting) {
					    ncurr->selected = TRUE;
					    set_item_state(window, nn, ncurr);
					}
					/* do nothing if counting&&count==0 */
				    }

				    if (cw->how == PICK_ONE)
					finished = TRUE;
				    break;
				}
			} else
			    tty_nhbell();
			break;
		}

	    } /* while */
	    cw->morestr = msave;

	} else {

	    for(n=0, i=0; i<cw->maxrow; i++) {
		if(!cw->offx && (n+cw->offy == ttyDisplay->rows-1)) {
		    tty_curs(window, 1, n);
		    cl_end();
		    dmore(cw, quitchars);
		    if(morc == '\033') {
			cw->flags |= WIN_CANCELLED;
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
		    if(cw->offx) {
			(void) putchar(' '); ++ttyDisplay->curx;
		    }
		    term_start_attr(attr);
		    for(cp = &cw->data[i][1]; *cp &&
			(int)++ttyDisplay->curx < (int) ttyDisplay->cols; cp++)
			    (void) putchar(*cp);
		    term_end_attr(attr);
		}
	    }
	    if(i == cw->maxrow) {
		if(cw->type == NHW_TEXT)
		    tty_curs(BASE_WINDOW, (int)cw->offx+1, (int)ttyDisplay->rows-1);
		else
		    tty_curs(BASE_WINDOW, (int)cw->offx+1, n);
		cl_end();
		dmore(cw, quitchars);
		if (morc == '\033')
		    cw->flags |= WIN_CANCELLED;
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
	    if (flags.window_inited) {
		/* otherwise dismissing the text endwin after other windows
		 * are dismissed tries to redraw the map and panics.  since
		 * the whole reason for dismissing the other windows was to
		 * leave the ending window on the screen, we don't want to
		 * erase it anyway.
		 */
		erase_menu_or_text(window, cw, FALSE);
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

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0)
	panic(winpanicstr,  window);

    if(cw->active)
	tty_dismiss_nhwindow(window);
    if(cw->type == NHW_MESSAGE)
	flags.window_inited = 0;
    if(cw->type == NHW_MAP)
	clear_screen();

    free_window_info(cw, TRUE);
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

#if defined(USE_TILES) && defined(MSDOS)
# ifdef SIMULATE_CURSOR
    if (cw->type == NHW_MAP) cursor_flag = 1;
    else cursor_flag = 0;
# endif
#endif
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

    if(str == (const char*)0 || (cw->flags & WIN_CANCELLED))
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
	tty_curs(window, cw->curx+1, cw->cury);
	term_start_attr(attr);
	while(*str && (int) ttyDisplay->curx < (int) ttyDisplay->cols-1) {
	    (void) putchar(*str);
	    str++;
	    ttyDisplay->curx++;
	}
	cw->curx = 0;
	cw->cury++;
	term_end_attr(attr);
	break;
    case NHW_BASE:
	tty_curs(window, cw->curx+1, cw->cury);
	term_start_attr(attr);
	while (*str) {
	    if ((int) ttyDisplay->curx >= (int) ttyDisplay->cols-1) {
		cw->curx = 0;
		cw->cury++;
		tty_curs(window, cw->curx+1, cw->cury);
	    }
	    (void) putchar(*str);
	    str++;
	    ttyDisplay->curx++;
	}
	cw->curx = 0;
	cw->cury++;
	term_end_attr(attr);
	break;
    case NHW_MENU:
    case NHW_TEXT:
	if(cw->type == NHW_TEXT && cw->cury == ttyDisplay->rows-1) {
	    /* not a menu, so save memory and output 1 page at a time */
	    cw->maxcol = ttyDisplay->cols; /* force full-screen mode */
	    tty_display_nhwindow(window, TRUE);
	    for(i=0; i<cw->maxrow; i++)
		if(cw->data[i]){
		    free((genericptr_t)cw->data[i]);
		    cw->data[i] = 0;
		}
	    cw->maxrow = cw->cury = 0;
	}
	/* always grows one at a time, but alloc 12 at a time */
	if(cw->cury >= cw->rows) {
	    char **tmp;

	    cw->rows += 12;
	    tmp = (char **) alloc(sizeof(char *) * (unsigned)cw->rows);
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
	n0 = strlen(str) + 1;
	ob = cw->data[cw->cury] = (char *)alloc((unsigned)n0 + 1);
	*ob++ = (char)(attr + 1);	/* avoid nuls, for convenience */
	Strcpy(ob, str);

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
		(void) execlp(catmore, "page", (char *)0);
		if(complain) raw_printf("Cannot exec %s.", catmore);
	    }
	    if(complain) sleep(10); /* want to wait_synch() but stdin is gone */
	    terminate(EXIT_FAILURE);
	}
	(void) close(fd);
    }
#else	/* DEF_PAGER */
    {
	dlb *f;
	char buf[BUFSZ];
	char *cr;

	tty_clear_nhwindow(WIN_MESSAGE);
	f = dlb_fopen(fname, "r");
	if (!f) {
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
	    while (dlb_fgets(buf, BUFSZ, f)) {
		if ((cr = index(buf, '\n')) != 0) *cr = 0;
#ifdef MSDOS
		if ((cr = index(buf, '\r')) != 0) *cr = 0;
#endif
		if (index(buf, '\t') != 0) (void) tabexpand(buf);
		tty_putstr(datawin, 0, buf);
		if(wins[datawin]->flags & WIN_CANCELLED)
		    break;
	    }
	    tty_display_nhwindow(datawin, FALSE);
	    tty_destroy_nhwindow(datawin);
	    (void) dlb_fclose(f);
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

/*ARGSUSED*/
/*
 * Add a menu item to the beginning of the menu list.  This list is reversed
 * later.
 */
void
tty_add_menu(window, glyph, identifier, ch, attr, str, preselected)
    winid window;	/* window to use, must be of type NHW_MENU */
    int glyph;		/* glyph to display with item (unused) */
    const anything *identifier;	/* what to return if selected */
    char ch;		/* keyboard accelerator (0 = pick our own) */
    int attr;		/* attribute for string (like tty_putstr()) */
    const char *str;	/* menu string */
    boolean preselected; /* item is marked as selected */
{
    register struct WinDesc *cw = 0;
    tty_menu_item *item;
    const char *newstr;
    char buf[QBUFSZ];

    if (str == (const char*) 0)
	return;

    if (window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0
		|| cw->type != NHW_MENU)
	panic(winpanicstr,  window);

    cw->nitems++;
    if (identifier->a_void) {
	Sprintf(buf, "%c - %s", ch ? ch : '?', str);
	newstr = buf;
    } else
	newstr = str;

    item = (tty_menu_item *) alloc(sizeof(tty_menu_item));
    item->identifier = *identifier;
    item->count = -1L;
    item->selected = preselected;
    item->selector = ch;
    item->attr = attr;
    item->str = copy_of(newstr);

    item->next = cw->mlist;
    cw->mlist = item;
}

/* Invert the given list, can handle NULL as an input. */
static tty_menu_item *
reverse(curr)
    tty_menu_item *curr;
{
    tty_menu_item *next, *head = 0;

    while (curr) {
	next = curr->next;
	curr->next = head;
	head = curr;
	curr = next;
    }
    return head;
}

/*
 * End a menu in this window, window must a type NHW_MENU.  This routine
 * processes the string list.  We calculate the # of pages, then assign
 * keyboard accelerators as needed.  Finally we decide on the width and
 * height of the window.
 */
void
tty_end_menu(window, prompt)
    winid window;	/* menu to use */
    const char *prompt;	/* prompt to for menu */
{
    struct WinDesc *cw = 0;
    tty_menu_item *curr;
    short len;
    int lmax, n;
    char menu_ch;

    if (window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0 ||
		cw->type != NHW_MENU)
	panic(winpanicstr,  window);

    /* Reverse the list so that items are in correct order. */
    cw->mlist = reverse(cw->mlist);

    /* Put the promt at the beginning of the menu. */
    if (prompt) {
	anything any;

	any.a_void = 0;	/* not selectable */
	tty_add_menu(window, NO_GLYPH, &any, 0, ATR_NONE, "", MENU_UNSELECTED);
	tty_add_menu(window, NO_GLYPH, &any, 0, ATR_NONE, prompt, MENU_UNSELECTED);
    }

    lmax = min(52, (int)ttyDisplay->rows - 1);		/* # lines per page */
    cw->npages = (cw->nitems + (lmax - 1)) / lmax;	/* # of pages */

    /* make sure page list is large enough */
    if (cw->plist_size < cw->npages+1 /*need 1 slot beyond last*/) {
	if (cw->plist) free((genericptr_t)cw->plist);
	cw->plist_size = cw->npages + 1;
	cw->plist = (tty_menu_item **)
			alloc(cw->plist_size * sizeof(tty_menu_item *));
    }

    cw->cols = 0; /* cols is set when the win is initialized... (why?) */
    menu_ch = '?';	/* lint suppression */
    for (n = 0, curr = cw->mlist; curr; n++, curr = curr->next) {
	/* set page boundaries and character accelerators */
	if ((n % lmax) == 0) {
	    menu_ch = 'a';
	    cw->plist[n/lmax] = curr;
	}
	if (curr->identifier.a_void && !curr->selector) {
	    curr->str[0] = curr->selector = menu_ch;
	    if (menu_ch++ == 'z') menu_ch = 'A';
	}

	/* cut off any lines that are too long */
	len = strlen(curr->str) + 2;	/* extra space at beg & end */
	if (len > (int)ttyDisplay->cols) {
	    curr->str[ttyDisplay->cols-2] = 0;
	    len = ttyDisplay->cols;
	}
	if (len > cw->cols) cw->cols = len;
    }
    cw->plist[cw->npages] = 0;	/* plist terminator */

    /*
     * If greater than 1 page, morestr is "(x of y) " otherwise, "(end) "
     */
    if (cw->npages > 1) {
	char buf[QBUFSZ];
	/* produce the largest demo string */
	Sprintf(buf, "(%d of %d) ", cw->npages, cw->npages);
	len = strlen(buf);
	cw->morestr = copy_of("");
    } else {
	cw->morestr = copy_of("(end) ");
	len = strlen(cw->morestr);
    }

    if (len > (int)ttyDisplay->cols) {
	/* truncate the prompt if its too long for the screen */
	if (cw->npages <= 1)	/* only str in single page case */
	    cw->morestr[ttyDisplay->cols] = 0;
	len = ttyDisplay->cols;
    }
    if (len > cw->cols) cw->cols = len;

    cw->maxcol = cw->cols;

    /*
     * The number of lines in the first page plus the morestr will be the
     * maximum size of the window.
     */
    if (cw->npages > 1)
	cw->maxrow = cw->rows = lmax + 1;
    else
	cw->maxrow = cw->rows = cw->nitems + 1;
}

int
tty_select_menu(window, how, menu_list)
    winid window;
    int how;
    menu_item **menu_list;
{
    register struct WinDesc *cw = 0;
    tty_menu_item *curr;
    menu_item *mi;
    int n, cancelled;

    if(window == WIN_ERR || (cw = wins[window]) == (struct WinDesc *) 0
       || cw->type != NHW_MENU)
	panic(winpanicstr,  window);

    *menu_list = (menu_item *) 0;
    cw->how = (short) how;
    morc = 0;
    tty_display_nhwindow(window, TRUE);
    cancelled = !!(cw->flags & WIN_CANCELLED);
    tty_dismiss_nhwindow(window);	/* does not destroy window data */

    if (cancelled) {
	n = -1;
    } else {
	for (n = 0, curr = cw->mlist; curr; curr = curr->next)
	    if (curr->selected) n++;
    }

    if (n > 0) {
	*menu_list = (menu_item *) alloc(n * sizeof(menu_item));
	for (mi = *menu_list, curr = cw->mlist; curr; curr = curr->next)
	    if (curr->selected) {
		mi->item = curr->identifier;
		mi->count = curr->count;
		mi++;
	    }
    }

    return n;
}

/* special hack for treating top line --More-- as a one item menu */
char
tty_message_menu(let, how, mesg)
char let;
int how;
const char *mesg;
{
    /* "menu" without selection; use ordinary pline, no more() */
    if (how == PICK_NONE) {
	pline(mesg);
	return 0;
    }

    ttyDisplay->dismiss_more = let;
    morc = 0;
    /* barebones pline(); since we're only supposed to be called after
       response to a prompt, we'll assume that the display is up to date */
    tty_putstr(WIN_MESSAGE, 0, mesg);
    /* if `mesg' didn't wrap (triggering --More--), force --More-- now */
    if (ttyDisplay->toplin == 1)
	more();
    /* normally <ESC> means skip further messages, but in this case
       it means cancel the current prompt; any other messages should
       continue to be output normally */
    wins[WIN_MESSAGE]->flags &= ~WIN_CANCELLED;
    ttyDisplay->dismiss_more = 0;

    return ((how == PICK_ONE && morc == let) || morc == '\033') ? morc : '\0';
}

void
tty_update_inventory()
{
    return;
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
#if defined(USE_TILES) && defined(MSDOS)
	if (tiles_on)
		row_refresh((xmin/2)+clipx-((int)cw->offx/2),COLNO-1,y+clipy-(int)cw->offy);
	else
#endif
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
}

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
	extern boolean restoring;
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
	    if (on_level(&u.uz0, &u.uz) && !restoring)
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
#define obj_color(n)  color = flags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = flags.use_color ? mons[n].mcolor : NO_COLOR
#define pet_color(n)  color = flags.use_color ? mons[n].mcolor :	      \
				/* If no color, try to hilite pets; black  */ \
				/* should be HI				   */ \
				((flags.hilite_pet && has_color(CLR_BLACK)) ?     \
							CLR_BLACK : NO_COLOR)

# else /* no text color */

#define zap_color(n)
#define cmap_color(n)
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
# ifdef REINCARNATION
    if (!has_color(color) || Is_rogue_level(&u.uz))
# else
    if (!has_color(color))
# endif
	color = NO_COLOR;

    if (color != ttyDisplay->color) {
	if(ttyDisplay->color != NO_COLOR)
	    term_end_color();
	ttyDisplay->color = color;
	if(color != NO_COLOR)
	    term_start_color(color);
    }
#endif /* TEXTCOLOR */
#if defined(USE_TILES) && defined(MSDOS)
    if (flags.grmode && tiles_on)
      xputg(glyph,(int)ch);
    else
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

#ifdef POSITIONBAR
void
tty_update_positionbar(posbar)
char *posbar;
{
# ifdef MSDOS
	video_update_positionbar(posbar);
# endif
}
#endif

/*
 * Allocate a copy of the given string.  If null, return a string of
 * zero length.
 *
 * This is an exact duplicate of copy_of() in X11/winmenu.c.
 */
static char *
copy_of(s)
    const char *s;
{
    if (!s) s = "";
    return strcpy((char *) alloc((unsigned) (strlen(s) + 1)), s);
}

#endif /* TTY_GRAPHICS */

/*wintty.c*/
