/*	SCCS Id: @(#)winmap.c	3.1	93/02/02		  */
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains:
 *	+ global functions print_glyph() and cliparound()
 * 	+ the map window routines
 *	+ the char and pointer input routines
 *
 * Notes:
 *	+ We don't really have a good way to get the compiled ROWNO and 
 *	  COLNO as defaults.  They are hardwired to the current "correct"
 *	  values in the Window widget.  I am _not_ in favor of including
 *	  some nethack include file for Window.c.
 */

#ifndef SYSV
#define PRESERVE_NO_SYSV	/* X11 include files may define SYSV */
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xatom.h>

#ifdef PRESERVE_NO_SYSV
# ifdef SYSV
#  undef SYSV
# endif
# undef PRESERVE_NO_SYSV
#endif

#include "Window.h"	/* map widget declarations */

#include "hack.h"
#include "winX.h"

/* Define these if you really want a lot of junk on your screen. */
/* #define VERBOSE		/* print various info & events as they happen */
/* #define VERBOSE_UPDATE	/* print screen update bounds */
/* #define VERBOSE_INPUT	/* print input events */

static void FDECL(set_button_values, (Widget,int,int,unsigned));
static void FDECL(map_check_size_change, (struct xwindow *));
static void FDECL(map_update, (struct xwindow *,int,int,int,int,BOOLEAN_P));
static void FDECL(map_exposed, (Widget,XtPointer,XtPointer));
static void FDECL(set_gc, (Widget,Font,char *,Pixel,GC *,GC *));
static void FDECL(get_gc, (struct xwindow *,Font));
static void FDECL(get_char_info, (struct xwindow *));
static void FDECL(display_cursor, (struct xwindow *));

/* Global functions ======================================================== */

void
X11_print_glyph(window, x, y, glyph)
    winid window;
    xchar x, y;
    int glyph;
{
    uchar	      ch;
    register int      offset;
    struct map_info_t *map_info;
    register unsigned char *ch_ptr;
#ifdef TEXTCOLOR
    int     color;
    register unsigned char *co_ptr;

#define zap_color(n)  color = zapcolors[n]
#define cmap_color(n) color = defsyms[n].color
#define trap_color(n) color = (n == WEB) ? defsyms[S_web ].color : \
					   defsyms[S_trap].color
#define obj_color(n)  color = objects[n].oc_color
#define mon_color(n)  color = mons[n].mcolor
#define pet_color(n)  color = mons[n].mcolor

# else /* no text color */

#define zap_color(n)
#define cmap_color(n)
#define trap_color(n)
#define obj_color(n)
#define mon_color(n)
#define pet_color(n)
#endif

    check_winid(window);
    if (window_list[window].type != NHW_MAP) {
	impossible("print_glyph: can (currently) only print to map windows");
	return;
    }
    map_info = window_list[window].map_information;

    /*
     *  Map the glyph back to a character.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *            offsets.  The order is set in display.h.
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
	ch = oc_syms[objects[offset].oc_class];
	obj_color(offset);
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) {	/* a corpse */
	ch = oc_syms[objects[CORPSE].oc_class];
	mon_color(offset);
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {	/* a pet */
	ch = monsyms[mons[offset].mlet];
	pet_color(offset);
    } else {							/* a monster */
	ch = monsyms[mons[glyph].mlet];
	mon_color(glyph);
    }

    /* Only update if we need to. */
    ch_ptr = &map_info->text[y][x];

#ifdef TEXTCOLOR
    co_ptr = &map_info->colors[y][x];
    if (*ch_ptr != ch || *co_ptr != color)
#else
    if (*ch_ptr != ch)
#endif
    {
	*ch_ptr = ch;
#ifdef TEXTCOLOR
	*co_ptr = color;
#endif
	/* update row bbox */
	if ((uchar) x < map_info->t_start[y]) map_info->t_start[y] = x;
	if ((uchar) x > map_info->t_stop[y])  map_info->t_stop[y]  = x;
    }

#undef zap_color
#undef cmap_color
#undef trap_color
#undef obj_color
#undef mon_color
#undef pet_color
}

#ifdef CLIPPING
/*
 * The is the tty clip call.  Since X can resize at any time, we can't depend
 * on this being defined.
 */
/*ARGSUSED*/
void X11_cliparound(x, y) int x, y; { }
#endif /* CLIPPING */

/* End global functions ==================================================== */


/*
 * Make sure the map's cursor is always visible.
 */
void
check_cursor_visibility(wp)
    struct xwindow *wp;
{
    Arg arg[2];
    Widget viewport, horiz_sb, vert_sb;
    float top, shown, cursor_middle;
    Boolean do_call, adjusted = False;
#ifdef VERBOSE
    char *s;
#endif

    viewport = XtParent(wp->w);
    horiz_sb = XtNameToWidget(viewport, "horizontal");
    vert_sb  = XtNameToWidget(viewport, "vertical");

#define V_BORDER 0.1		/* if this far from vert edge, shift */
#define H_BORDER 0.0625		/* if this from from horiz edge, shift */

#define H_DELTA 0.25		/* distance of horiz shift */
				/* vert shift is half of curr distance */
/* The V_DELTA is 1/2 the value of shown. */

    if (horiz_sb) {
	XtSetArg(arg[0], XtNshown,	&shown);
	XtSetArg(arg[1], XtNtopOfThumb, &top);
	XtGetValues(horiz_sb, arg, TWO);

	cursor_middle = (((float) wp->cursx) + 0.5) / (float) COLNO;
	do_call = True;

#ifdef VERBOSE
	if (cursor_middle < top) {
	    s = " outside left";
	} else if (cursor_middle < top + H_BORDER) {
	    s = " close to left";
	} else if (cursor_middle > (top + shown)) {
	    s = " outside right";
	} else if (cursor_middle > (top + shown - H_BORDER)) {
	    s = " close to right";
	} else {
	    s = "";
	}
	printf("Horiz: shown = %3.2f, top = %3.2f%s", shown, top, s);
#endif

	if (cursor_middle < top) {
	    top = cursor_middle - H_DELTA;
	    if (top < 0.0) top = 0;
	} else if (cursor_middle < top + H_BORDER) {
	    top -= H_DELTA;
	    if (top < 0.0) top = 0.0;
	} else if (cursor_middle > (top + shown)) {
	    top = cursor_middle + H_DELTA;
	    if (top + shown > 1.0) top = 1.0 - shown;
	} else if (cursor_middle > (top + shown - H_BORDER)) {
	    top += H_DELTA;
	    if (top + shown > 1.0) top = 1.0 - shown;
	} else {
	    do_call = False;
	}

	if (do_call) {
	    XtCallCallbacks(horiz_sb, XtNjumpProc, &top);
	    adjusted = True;
	}
    }

    if (vert_sb) {
	XtSetArg(arg[0], XtNshown,      &shown);
	XtSetArg(arg[1], XtNtopOfThumb, &top);
	XtGetValues(vert_sb, arg, TWO);

	cursor_middle = (((float) wp->cursy) + 0.5) / (float) ROWNO;
	do_call = True;

#ifdef VERBOSE
	if (cursor_middle < top) {
	    s = " above top";
	} else if (cursor_middle < top + V_BORDER) {
	    s = " close to top";
	} else if (cursor_middle > (top + shown)) {
	    s = " below bottom";
	} else if (cursor_middle > (top + shown - V_BORDER)) {
	    s = " close to bottom";
	} else {
	    s = "";
	}
	printf("%sVert: shown = %3.2f, top = %3.2f%s",
				    horiz_sb ? ";  " : "", shown, top, s);
#endif

	if (cursor_middle < top) {
	    top = cursor_middle - (shown / 2.0);
	    if (top < 0.0) top = 0;
	} else if (cursor_middle < top + V_BORDER) {
	    top -= shown / 2.0;
	    if (top < 0.0) top = 0;
	} else if (cursor_middle > (top + shown)) {
	    top = cursor_middle - (shown / 2.0);
	    if (top < 0.0) top = 0;
	    if (top + shown > 1.0) top = 1.0 - shown;
	} else if (cursor_middle > (top + shown - V_BORDER)) {
	    top += shown / 2.0;
	    if (top + shown > 1.0) top = 1.0 - shown;
	} else {
	    do_call = False;
	}

	if (do_call) {
	    XtCallCallbacks(vert_sb, XtNjumpProc, &top);
	    adjusted = True;
	}
    }

    /* make sure cursor is displayed during dowhatis.. */
    if (adjusted) display_cursor(wp);

#ifdef VERBOSE
    if (horiz_sb || vert_sb) printf("\n");
#endif
}


/*
 * Check to see if the viewport has grown smaller.  If so, then we want to make
 * sure that the cursor is still on the screen.  We do this to keep the cursor
 * on the screen when the user resizes the nethack window.
 */
static void
map_check_size_change(wp)
    struct xwindow *wp;
{
    struct map_info_t *map_info = wp->map_information;
    Arg arg[2];
    Dimension new_width, new_height;
    Widget viewport;

    viewport = XtParent(wp->w);

    XtSetArg(arg[0], XtNwidth,  &new_width);
    XtSetArg(arg[1], XtNheight, &new_height);
    XtGetValues(viewport, arg, TWO);

    /* Only do cursor check if new size is smaller. */
    if (new_width < map_info->viewport_width
		    || new_height < map_info->viewport_height) {
	check_cursor_visibility(wp);
    }

    map_info->viewport_width = new_width;
    map_info->viewport_height = new_height;
}

/*
 * Fill in parameters "regular" and "inverse" with newly created GCs.
 * Using the given background pixel and the foreground pixel optained
 * by querying the widget with the resource name.
 */
static void
set_gc(w, font, resource_name, bgpixel, regular, inverse)
    Widget w;
    Font font;
    char *resource_name;
    Pixel bgpixel;
    GC   *regular, *inverse;
{
    XGCValues values;
    XtGCMask mask = GCFunction | GCForeground | GCBackground | GCFont;
    Pixel curpixel;
    Arg arg[1];

    XtSetArg(arg[0], resource_name, &curpixel);
    XtGetValues(w, arg, ONE);

    values.foreground = curpixel;
    values.background = bgpixel;
    values.function   = GXcopy;
    values.font	      = font;
    *regular = XtGetGC(w, mask, &values);
    values.foreground = bgpixel;
    values.background = curpixel;
    values.function   = GXcopy;
    values.font	      = font;
    *inverse = XtGetGC(w, mask, &values);
}

/*
 * Create the GC's for each color.
 *
 * I'm not sure if it is a good idea to have a GC for each color (and
 * inverse). It might be faster to just modify the foreground and
 * background colors on the current GC as needed.
 */
static void
get_gc(wp, font)
    struct xwindow *wp;
    Font font;
{
    struct map_info_t *map_info = wp->map_information;
    Pixel bgpixel;
    Arg arg[1];

    /* Get background pixel. */
    XtSetArg(arg[0], XtNbackground, &bgpixel);
    XtGetValues(wp->w, arg, ONE);

#ifdef TEXTCOLOR
#define set_color_gc(nh_color, resource_name)			\
	    set_gc(wp->w, font, resource_name, bgpixel,		\
		    &map_info->color_gcs[nh_color],		\
		    &map_info->inv_color_gcs[nh_color]);

    set_color_gc(BLACK,		 XtNblack);
    set_color_gc(RED,		 XtNred);
    set_color_gc(GREEN,		 XtNgreen);
    set_color_gc(BROWN,		 XtNbrown);
    set_color_gc(BLUE,		 XtNblue);
    set_color_gc(MAGENTA,	 XtNmagenta);
    set_color_gc(CYAN,		 XtNcyan);
    set_color_gc(GRAY,		 XtNgray);
    set_color_gc(NO_COLOR,	 XtNforeground);
    set_color_gc(ORANGE_COLORED, XtNorange);
    set_color_gc(BRIGHT_GREEN,	 XtNbright_green);
    set_color_gc(YELLOW,	 XtNyellow);
    set_color_gc(BRIGHT_BLUE,	 XtNbright_blue);
    set_color_gc(BRIGHT_MAGENTA, XtNbright_magenta);
    set_color_gc(BRIGHT_CYAN,	 XtNbright_cyan);
    set_color_gc(WHITE,		 XtNwhite);
#else
    set_gc(wp->w, font, XtNforeground, bgpixel,
				&map_info->copy_gc, &map_info->inv_copy_gc);
#endif
}


/*
 * Display the cursor on the map window.
 */
static void
display_cursor(wp)
    struct xwindow *wp;
{
    /* Redisplay the cursor location inverted. */
    map_update(wp, wp->cursy, wp->cursy, wp->cursx, wp->cursx, TRUE);
}


/*
 * Check if there are any changed characters.  If so, then plaster them on
 * the screen.
 */
void
display_map_window(wp)
    struct xwindow *wp;
{
    register int row;
    struct map_info_t *map_info = wp->map_information;

    /*
     * If the previous cursor position is not the same as the current
     * cursor position, then update the old cursor position.
     */
    if (wp->prevx != wp->cursx || wp->prevy != wp->cursy) {
	register unsigned int x = wp->prevx, y = wp->prevy;
	if (x < map_info->t_start[y]) map_info->t_start[y] = x;
	if (x > map_info->t_stop[y])  map_info->t_stop[y]  = x;
    }

    for (row = 0; row < ROWNO; row++) {
	if (map_info->t_start[row] <= map_info->t_stop[row]) {
	    map_update(wp, row, row,
			(int) map_info->t_start[row],
			(int) map_info->t_stop[row], FALSE);
	    map_info->t_start[row] = COLNO-1;
	    map_info->t_stop[row] = 0;
	}
    }
    display_cursor(wp);
    wp->prevx = wp->cursx;	/* adjust old cursor position */
    wp->prevy = wp->cursy;
}

/*
 * Fill the saved screen characters with the "clear" character, and reset
 * all colors to the neutral color.  Flush out everything by resetting the
 * "new" bounds and calling display_map_window().
 */
void
clear_map_window(wp)
    struct xwindow *wp;
{
    struct map_info_t *map_info = wp->map_information;

    /* Fill with spaces, and update */
    (void) memset((genericptr_t) map_info->text, ' ',
			sizeof(map_info->text));
    (void) memset((genericptr_t) map_info->t_start, (char) 0,
			sizeof(map_info->t_start));
    (void) memset((genericptr_t) map_info->t_stop, (char) COLNO-1,
			sizeof(map_info->t_stop));
#ifdef TEXTCOLOR
    (void) memset((genericptr_t) map_info->colors, NO_COLOR,
			sizeof(map_info->colors));
#endif
    display_map_window(wp);
}

/*
 * Retreive the font associated with the map window and save attributes
 * that are used when updating it.
 */
static void
get_char_info(wp)
    struct xwindow *wp;
{
    XFontStruct *fs;

    fs = WindowFontStruct(wp->w);
    wp->map_information->char_width    = fs->max_bounds.width;
    wp->map_information->char_height   = fs->max_bounds.ascent +
						fs->max_bounds.descent;
    wp->map_information->char_ascent   = fs->max_bounds.ascent;
    wp->map_information->char_lbearing = -fs->min_bounds.lbearing;

#ifdef VERBOSE
    printf("Font information:\n");
    printf("fid = %d, direction = %d\n", fs->fid, fs->direction);
    printf("first = %d, last = %d\n",
			fs->min_char_or_byte2, fs->max_char_or_byte2);
    printf("all chars exist? %s\n", fs->all_chars_exist?"yes":"no");
    printf("min_bounds:lb=%d rb=%d width=%d asc=%d des=%d attr=%d\n",
		fs->min_bounds.lbearing, fs->min_bounds.rbearing,
		fs->min_bounds.width, fs->min_bounds.ascent,
		fs->min_bounds.descent, fs->min_bounds.attributes);
    printf("max_bounds:lb=%d rb=%d width=%d asc=%d des=%d attr=%d\n",
		fs->max_bounds.lbearing, fs->max_bounds.rbearing,
		fs->max_bounds.width, fs->max_bounds.ascent,
		fs->max_bounds.descent, fs->max_bounds.attributes);
    printf("per_char = 0x%x\n", fs->per_char);
    printf("Text: (max) width = %d, height = %d\n",
	    wp->map_information->char_width, wp->map_information->char_height);
#endif

    if (fs->min_bounds.width != fs->max_bounds.width)
	X11_raw_print("Warning:  map font is not monospaced!");
}

/*
 * keyhit buffer
 */
#define INBUF_SIZE 64
int inbuf[INBUF_SIZE];
int incount = 0;
int inptr = 0;	/* points to valid data */


/*
 * Keyboard and button event handler for map window.
 */
void
map_input(w, event, params, num_params)
    Widget   w;
    XEvent   *event;
    String   *params;
    Cardinal *num_params;
{
    XKeyEvent *key;
    XButtonEvent *button;
    boolean meta = FALSE;
    int i, nbytes;
    Cardinal in_nparams = (num_params ? *num_params : 0);
    char c;
    char keystring[MAX_KEY_STRING];

    switch (event->type) {
	case ButtonPress:
	    button = (XButtonEvent *) event;
#ifdef VERBOSE_INPUT
	    printf("button press\n");
#endif
	    if (in_nparams > 0 &&
		(nbytes = strlen(params[0])) < MAX_KEY_STRING) {
		Strcpy(keystring, params[0]);
		key = (XKeyEvent *) event; /* just in case */
		goto key_events;
	    }
	    set_button_values(w, button->x, button->y, button->button);
	    break;
	case KeyPress:
#ifdef VERBOSE_INPUT
	    printf("key: ");
#endif
	    if(appResources.slow && input_func) {
		(*input_func)(w, event, params, num_params);
		break;
	    }

	    /*
	     * Don't use key_event_to_char() because we want to be able
	     * to allow keys mapped to multiple characters.
	     */
	    key = (XKeyEvent *) event;
	    if (in_nparams > 0 &&
		(nbytes = strlen(params[0])) < MAX_KEY_STRING) {
		Strcpy(keystring, params[0]);
	    } else {
		/*
		 * Assume that mod1 is really the meta key.
		 */
		meta = !!(key->state & Mod1Mask);
		nbytes =
		    XLookupString(key, keystring, MAX_KEY_STRING,
				  (KeySym *)0, (XComposeStatus *)0);
	    }
	key_events:
	    /* Modifier keys return a zero length string when pressed. */
	    if (nbytes) {
#ifdef VERBOSE_INPUT
		printf("\"");
#endif
		for (i = 0; i < nbytes; i++) {
		    c = keystring[i];

		    if (incount < INBUF_SIZE) {
			inbuf[(inptr+incount)%INBUF_SIZE] =
			    ((int) c) + (meta ? 0x80 : 0);
			incount++;
		    } else {
			X11_nhbell();
		    }
#ifdef VERBOSE_INPUT
		    if (meta)			/* meta will print as M<c> */
			(void) putchar('M');
		    if (c < ' ') {		/* ctrl will print as ^<c> */
			(void) putchar('^');
			c += '@';
		    }
		    (void) putchar(c);
#endif
		}
#ifdef VERBOSE_INPUT
		printf("\" [%d bytes]\n", nbytes);
#endif
	    }
	    break;

	default:
	    impossible("unexpected X event, type = %d\n", (int) event->type);
	    break;
    }
}

static void
set_button_values(w, x, y, button)
    Widget w;
    int x;
    int y;
    unsigned int button;
{
    struct xwindow *wp;
    struct map_info_t *map_info;

    wp = find_widget(w);
    map_info = wp->map_information;

    click_x = x / map_info->char_width;
    click_y = y / map_info->char_height;

    /* The values can be out of range if the map window has been resized */
    /* to be larger than the max size.					 */
    if (click_x >= COLNO) click_x = COLNO-1;
    if (click_y >= ROWNO) click_x = ROWNO-1;

    /* Map all buttons but the first to the second click */
    click_button = (button == Button1) ? CLICK_1 : CLICK_2;
}

/*
 * Map window expose callback.
 */
/*ARGSUSED*/
static void
map_exposed(w, client_data, widget_data)
    Widget w;
    XtPointer client_data;	/* unused */
    XtPointer widget_data;	/* expose event from Window widget */
{
    int x, y;
    struct xwindow *wp;
    struct map_info_t *map_info;
    unsigned width, height;
    int start_row, stop_row, start_col, stop_col;
    XExposeEvent *event = (XExposeEvent *) widget_data;

    if (!XtIsRealized(w) || event->count > 0) return;

    wp = find_widget(w);
    map_info = wp->map_information;
    /*
     * The map is sent an expose event when the viewport resizes.  Make sure
     * that the cursor is still in the viewport after the resize.
     */
    map_check_size_change(wp);

    if (event) {		/* called from button-event */
	x      = event->x;
	y      = event->y;
	width  = event->width;
	height = event->height;
    } else {
	x     = 0;
	y     = 0;
	width = wp->pixel_width;
	height= wp->pixel_height;
    }
    /*
     * Convert pixels into INCLUSIVE text rows and columns.
     */
    start_row = y / map_info->char_height;
    stop_row = start_row + (height / map_info->char_height) +
			(((height % map_info->char_height) == 0) ? 0 : 1) - 1;

    start_col = x / map_info->char_width;
    stop_col = start_col + (width / map_info->char_width) +
			(((width % map_info->char_width) == 0) ? 0 : 1) - 1;

#ifdef VERBOSE
    printf("map_exposed: x = %d, y = %d, width = %d, height = %d\n",
						    x, y, width, height);
#endif

    /* Out of range values are possible if the map window is resized to be */
    /* bigger than the largest expected value.				   */
    if (stop_row >= ROWNO) stop_row = ROWNO-1;
    if (stop_col >= COLNO) stop_col = COLNO-1;

    map_update(wp, start_row, stop_row, start_col, stop_col, FALSE);
    display_cursor(wp);		/* make sure cursor shows up */
}

/*
 * Do the actual work of the putting characters onto our X window.  This
 * is called from the expose event routine, the display window (flush)
 * routine, and the display cursor routine.  The later is a kludge that
 * involves the inverted parameter of this function.  A better solution
 * would be to double the color count, with any color above MAXCOLORS
 * being inverted.
 *
 * This works for rectangular regions (this includes one line rectangles).
 * The start and stop columns are *inclusive*.
 */
static void
map_update(wp, start_row, stop_row, start_col, stop_col, inverted)
    struct xwindow *wp;
    int start_row, stop_row, start_col, stop_col;
    boolean inverted;
{
    int win_start_row, win_start_col;
    struct map_info_t *map_info = wp->map_information;
    int row;
    register int count;

    if (start_row < 0 || stop_row >= ROWNO) {
	impossible("map_update:  bad row range %d-%d\n", start_row, stop_row);
	return;
    }
    if (start_col < 0 || stop_col >=COLNO) {
	impossible("map_update:  bad col range %d-%d\n", start_col, stop_col);
	return;
    }

#ifdef VERBOSE_UPDATE
    printf("update: [0x%x] %d %d %d %d\n", 
		(int) wp->w, start_row, stop_row, start_col, stop_col);
#endif
    win_start_row = start_row;
    win_start_col = start_col;

#ifdef TEXTCOLOR
    if (flags.use_color) {
	register char *c_ptr;
	char *t_ptr;
	int cur_col, color, win_ystart;

	for (row = start_row; row <= stop_row; row++) {
	    win_ystart = map_info->char_ascent +
				    (row * map_info->char_height);

	    t_ptr = (char *) &(map_info->text[row][start_col]);
	    c_ptr = (char *) &(map_info->colors[row][start_col]);
	    cur_col = start_col;
	    while (cur_col <= stop_col) {
		color = *c_ptr++;
		count = 1;
		while ((cur_col + count) <= stop_col && *c_ptr == color) {
		    count++;
		    c_ptr++;
		}

		XDrawImageString(XtDisplay(wp->w), XtWindow(wp->w),
		    inverted ? map_info->inv_color_gcs[color] :
			       map_info->color_gcs[color],
		    map_info->char_lbearing + (map_info->char_width * cur_col),
		    win_ystart,
		    t_ptr, count);

		/* move text pointer and column count */
		t_ptr += count;
		cur_col += count;
	    } /* col loop */
	} /* row loop */
    } else
#endif /* TEXTCOLOR */
    {
	int win_row, win_xstart;

	/* We always start at the same x window position and have	*/
	/* the same character count.					*/
	win_xstart = map_info->char_lbearing +
				    (win_start_col * map_info->char_width);
	count = stop_col - start_col + 1;

	for (row = start_row, win_row = win_start_row;
					row <= stop_row; row++, win_row++) {

	    XDrawImageString(XtDisplay(wp->w), XtWindow(wp->w),
		inverted ? map_info->inv_copy_gc : map_info->copy_gc,
		win_xstart,
		map_info->char_ascent + (win_row * map_info->char_height),
		(char *) &(map_info->text[row][start_col]), count);
	}
    }
}

/* Adjust the number of rows and columns on the given map window */
void
set_map_size(wp, cols, rows)
    struct xwindow *wp;
    Dimension cols, rows;
{
    Arg args[4];
    Cardinal num_args;

    wp->pixel_width  = wp->map_information->char_width  * cols;
    wp->pixel_height = wp->map_information->char_height * rows;

    num_args = 0;
    XtSetArg(args[num_args], XtNwidth, wp->pixel_width);   num_args++;
    XtSetArg(args[num_args], XtNheight, wp->pixel_height); num_args++;
    XtSetValues(wp->w, args, num_args);
}

/*
 * The map window creation routine.
 */
void
create_map_window(wp, create_popup, parent)
    struct xwindow *wp;
    boolean create_popup;	/* parent is a popup shell that we create */
    Widget parent;
{
    struct map_info_t *map_info;	/* map info pointer */
    Widget map, viewport;
    Arg args[10];
    Cardinal num_args;
    Dimension rows, columns;

    wp->type = NHW_MAP;

    map_info = wp->map_information =
			(struct map_info_t *) alloc(sizeof(struct map_info_t));

    map_info->viewport_width = map_info->viewport_height = 0;
    (void) memset((genericptr_t) map_info->text, ' ', sizeof(map_info->text));
    (void) memset((genericptr_t) map_info->t_start, (char) COLNO,
			sizeof(map_info->t_start));
    (void) memset((genericptr_t) map_info->t_stop, (char) 0,
			sizeof(map_info->t_stop));
#ifdef TEXTCOLOR
    (void) memset((genericptr_t) map_info->colors, NO_COLOR,
			sizeof(map_info->colors));
#endif

    if (create_popup) {
	/*
	 * Create a popup that accepts key and button events.
	 */
	num_args = 0;
	XtSetArg(args[num_args], XtNinput, False);            num_args++;

	wp->popup = parent = XtCreatePopupShell("nethack",
					topLevelShellWidgetClass,
				       toplevel, args, num_args);
	/*
	 * If we're here, then this is an auxiliary map window.  If we're
	 * cancelled via a delete window message, we should just pop down.
	 */
    }

    num_args = 0;
    XtSetArg(args[num_args], XtNallowHoriz, True);	num_args++;
    XtSetArg(args[num_args], XtNallowVert,  True);	num_args++;
    /* XtSetArg(args[num_args], XtNforceBars,  True);	num_args++; */
    XtSetArg(args[num_args], XtNuseBottom,  True);	num_args++;
    viewport = XtCreateManagedWidget(
			"map_viewport",		/* name */
			viewportWidgetClass,	/* widget class from Window.h */
			parent,			/* parent widget */
			args,			/* set some values */
			num_args);		/* number of values to set */

    /*
     * Create a map window.  We need to set the width and height to some
     * value when we create it.  We will change it to the value we want
     * later
     */
    num_args = 0;
    XtSetArg(args[num_args], XtNwidth,  100); num_args++;
    XtSetArg(args[num_args], XtNheight, 100); num_args++;

    wp->w = map = XtCreateManagedWidget(
		"map",			/* name */
		windowWidgetClass,	/* widget class from Window.h */
		viewport,		/* parent widget */
		args,			/* set some values */
		num_args);		/* number of values to set */

    XtAddCallback(map, XtNexposeCallback, map_exposed, (XtPointer) 0);

    get_char_info(wp);
    get_gc(wp, WindowFont(map));

    /*
     * Initially, set the map widget to be the size specified by the
     * widget rows and columns resources.  We need to do this to
     * correctly set the viewport window size.  After the viewport is
     * realized, then the map can resize to its normal size.
     */
    num_args = 0;
    XtSetArg(args[num_args], XtNrows,    &rows);	num_args++;
    XtSetArg(args[num_args], XtNcolumns, &columns);	num_args++;
    XtGetValues(wp->w, args, num_args);

    /* Don't bother with windows larger than ROWNOxCOLNO. */
    if (columns > COLNO) columns = COLNO;
    if (rows    > ROWNO) rows = ROWNO;

    set_map_size(wp, columns, rows);

    /*
     * If we have created our own popup, then realize it so that the
     * viewport is also realized.  Then resize the map window.
     */
    if (create_popup) {
	XtRealizeWidget(wp->popup);
	XSetWMProtocols(XtDisplay(wp->popup), XtWindow(wp->popup),
			&wm_delete_window, 1);
	set_map_size(wp, COLNO, ROWNO);
    }
}

/*
 * Destroy this map window.
 */
void
destroy_map_window(wp)
    struct xwindow *wp;
{
    struct map_info_t *map_info = wp->map_information;
#ifdef TEXTCOLOR
    int i;
#endif

    if (wp->popup) {
	nh_XtPopdown(wp->popup);

	/* Free allocated GCs. */
#ifdef TEXTCOLOR
	for (i = 0; i < MAXCOLORS; i++) {
	    XtReleaseGC(wp->w, map_info->color_gcs[i]);
	    XtReleaseGC(wp->w, map_info->inv_color_gcs[i]);
	}
#else
	XtReleaseGC(wp->w, map_info->copy_gc);
	XtReleaseGC(wp->w, map_info->inv_copy_gc);
#endif

	/* Free malloc'ed space. */
	free((char *) map_info);

	/* Destroy map widget. */
	XtDestroyWidget(wp->popup);
    }

    wp->type = NHW_NONE;	/* allow re-use */
}



boolean exit_x_event;	/* exit condition for the event loop */
/*******
pkey(k)
    int k;
{
    printf("key = '%s%c'\n", (k<32) ? "^":"", (k<32) ? '@'+k : k);
}
******/

/*
 * Main X event loop.  Here we accept and dispatch X events.  We only exit
 * under certain circumstances.
 */
int
x_event(exit_condition)
    int exit_condition;
{
    XEvent  event;
    int     retval;
    boolean keep_going = TRUE;

#ifdef GCC_WARN
    retval = 0;
#endif

    click_button = NO_CLICK;	/* reset click exit condition */
    exit_x_event = FALSE;	/* reset callback exit condition */

    /*
     * Loop until we get a sent event, callback exit, or are accepting key
     * press and button press events and we receive one.
     */
    if((exit_condition == EXIT_ON_KEY_PRESS ||
	exit_condition == EXIT_ON_KEY_OR_BUTTON_PRESS) && incount)
	goto try_test;

    do {
	XtAppNextEvent(app_context, &event);
	XtDispatchEvent(&event);

	/* See if we can exit. */
    try_test:
	switch (exit_condition) {
	    case EXIT_ON_SENT_EVENT: {
		XAnyEvent *any = (XAnyEvent *) &event;
		if (any->send_event) {
		    retval = 0;
		    keep_going = FALSE;
		}
		break;
	    }
	    case EXIT_ON_EXIT:
		if (exit_x_event) {
		    incount = 0;
		    retval = 0;
		    keep_going = FALSE;
		}
		break;
	    case EXIT_ON_KEY_PRESS:
		if (incount != 0) {
		    /* get first pressed key */
		    --incount;
		    retval = inbuf[inptr];
		    inptr = (inptr+1) % INBUF_SIZE;
		    /* pkey(retval); */
		    keep_going = FALSE;
		}
		break;
	    case EXIT_ON_KEY_OR_BUTTON_PRESS:
		if (incount != 0 || click_button != NO_CLICK) {
		    if (click_button != NO_CLICK) {	/* button press */
			/* click values are already set */
			retval = 0;
		    } else {				/* key press */
			/* get first pressed key */
			--incount;
			retval = inbuf[inptr];
			inptr = (inptr+1) % INBUF_SIZE;
			/* pkey(retval); */
		    }
		    keep_going = FALSE;
		}
		break;
	    default:
		panic("x_event: unknown exit condition %d\n", exit_condition);
		break;
	}
    } while (keep_going);

    return retval;
}

/*winmap.c*/
