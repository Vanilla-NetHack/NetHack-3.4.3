/*	SCCS Id: @(#)wintext.c	3.1	92/3/7
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * File for dealing with text windows.
 * 
 * 	+ No global functions.
 */

#ifndef SYSV
#define PRESERVE_NO_SYSV	/* X11 include files may define SYSV */
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xos.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xatom.h>

#ifdef PRESERVE_NO_SYSV
# ifdef SYSV
#  undef SYSV
# endif
# undef PRESERVE_NO_SYSV
#endif

#include "hack.h"
#include "winX.h"


#define TRANSIENT_TEXT	/* text window is a transient window (no positioning) */

static const char text_translations[] =
    "#override\n\
     <BtnDown>: dismiss_text()\n\
     <Key>: key_dismiss_text()";


/*ARGSUSED*/
void
delete_text(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    struct xwindow *wp;
    struct text_info_t *text_info;

    wp = find_widget(w);
    text_info = wp->text_information;

    nh_XtPopdown(w);

    if (text_info->blocked) {
	exit_x_event = TRUE;
    } else if (text_info->destroy_on_ack) {
	destroy_text_window(wp);
    }
}

/*
 * Callback used for all text windows.  The window is poped down on any key
 * or button down event.  It is destroyed if the main nethack code is done
 * with it.
 */
/*ARGSUSED*/
void
dismiss_text(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    struct xwindow *wp;
    struct text_info_t *text_info;
    Widget popup = XtParent(w);

    wp = find_widget(w);
    text_info = wp->text_information;

    nh_XtPopdown(popup);

    if (text_info->blocked) {
	exit_x_event = TRUE;
    } else if (text_info->destroy_on_ack) {
	destroy_text_window(wp);
    }
}

/* Dismiss when a non-modifier key pressed. */
void
key_dismiss_text(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char ch = key_event_to_char((XKeyEvent *) event);
    if (ch) dismiss_text(w, event, params, num_params);
}

/* ARGSUSED */
void
add_to_text_window(wp, attr, str)
    struct xwindow *wp;
    int attr;	/* currently unused */
    const char *str;
{
    struct text_info_t *text_info = wp->text_information;
    int width;

    append_text_buffer(&text_info->text, str, FALSE);

    /* Calculate text width and save longest line */
    width = XTextWidth(text_info->fs, str, (int) strlen(str));
    if (width > text_info->max_width)
	text_info->max_width = width;
}

void
display_text_window(wp, blocking)
    struct xwindow *wp;
    boolean blocking;
{
    struct text_info_t *text_info;
    Arg args[8];
    Cardinal num_args;
    Dimension width, height;
    int nlines;

    text_info = wp->text_information;
    width  = text_info->max_width + text_info->extra_width;
    text_info->blocked = blocking;
    text_info->destroy_on_ack = FALSE;

    /*
     * Calculate the number of lines to use.  First, find the number of
     * lines that would fit on the screen.  Next, remove four of these
     * lines to give room for a possible window manager titlebar (some
     * wm's put a titlebar on transient windows).  Make sure we have
     * _some_ lines.  Finally, use the number of lines in the text if
     * there are fewer than the max.
     */
    nlines = (XtScreen(wp->w)->height - text_info->extra_height) /
			(text_info->fs->ascent + text_info->fs->descent);
    nlines -= 4;
    if (nlines <= 0) nlines = 1;

    if (nlines > text_info->text.num_lines)
	nlines = text_info->text.num_lines;

    /* Font height is ascent + descent. */
    height = (nlines * (text_info->fs->ascent + text_info->fs->descent))
						    + text_info->extra_height;

    num_args = 0;

    if (nlines < text_info->text.num_lines) {
	/* add on width of scrollbar.  Really should look this up,
	 * but can't until the window is realized.  Chicken-and-egg problem.
	 */
	width += 20;
    }

    if (width > (Dimension) XtScreen(wp->w)->width) { /* too wide for screen */
	/* Back off some amount - we really need to back off the scrollbar */
	/* width plus some extra.					   */
	width = XtScreen(wp->w)->width - 20;
    }
    XtSetArg(args[num_args], XtNstring, text_info->text.text);	num_args++;
    XtSetArg(args[num_args], XtNwidth,  width);			num_args++;
    XtSetArg(args[num_args], XtNheight, height);		num_args++;
    XtSetValues(wp->w, args, num_args);

#ifdef TRANSIENT_TEXT
    XtRealizeWidget(wp->popup);
    XSetWMProtocols(XtDisplay(wp->popup), XtWindow(wp->popup), 
		    &wm_delete_window, 1);
    positionpopup(wp->popup, FALSE);
#endif

    nh_XtPopup(wp->popup, (int)XtGrabNone, wp->w);

    /* Kludge alert.  Scrollbars are not sized correctly by the Text widget */
    /* if added before the window is displayed, so do it afterward. */
    num_args = 0;
    if (nlines < text_info->text.num_lines) {	/* add vert scrollbar */
	XtSetArg(args[num_args], XtNscrollVertical, XawtextScrollAlways);
								num_args++;
    }
    if (width >= (Dimension) (XtScreen(wp->w)->width-20)) {	/* too wide */
	XtSetArg(args[num_args], XtNscrollHorizontal, XawtextScrollAlways);
								num_args++;
    }
    if (num_args) XtSetValues(wp->w, args, num_args);

    /* We want the user to acknowlege. */
    if (blocking) {
	(void) x_event(EXIT_ON_EXIT);
	nh_XtPopdown(wp->popup);
    }
}


void
create_text_window(wp)
    struct xwindow *wp;
{
    struct text_info_t *text_info;
    Arg args[8];
    Cardinal num_args;
    Position top_margin, bottom_margin, left_margin, right_margin;

    wp->type = NHW_TEXT;

    wp->text_information = text_info = 
		    (struct text_info_t *) alloc(sizeof(struct text_info_t));

    init_text_buffer(&text_info->text);
    text_info->max_width      = 0;
    text_info->extra_width    = 0;
    text_info->extra_height   = 0;
    text_info->blocked	      = FALSE;
    text_info->destroy_on_ack = TRUE;	/* Ok to destroy before display */

    num_args = 0;
    XtSetArg(args[num_args], XtNallowShellResize, True); num_args++;

#ifdef TRANSIENT_TEXT
    wp->popup = XtCreatePopupShell("text", transientShellWidgetClass,
				   toplevel, args, num_args);
#else
    wp->popup = XtCreatePopupShell("text", topLevelShellWidgetClass,
				   toplevel, args, num_args);
#endif
    XtOverrideTranslations(wp->popup,
	XtParseTranslationTable("<Message>WM_PROTOCOLS: delete_text()"));

    num_args = 0;
    XtSetArg(args[num_args], XtNdisplayCaret, False);		num_args++;
    XtSetArg(args[num_args], XtNresize, XawtextResizeBoth);	num_args++;
    XtSetArg(args[num_args], XtNtranslations,
		XtParseTranslationTable(text_translations));	num_args++;

    wp->w = XtCreateManagedWidget(
		killer && WIN_MAP == WIN_ERR ?
				  "tombstone" : "text_text", /* name */
		asciiTextWidgetClass,
		wp->popup,		/* parent widget */
		args,			/* set some values */
		num_args);		/* number of values to set */

    /* Get the font and margin information. */
    num_args = 0;
    XtSetArg(args[num_args], XtNfont,	      &text_info->fs); num_args++;
    XtSetArg(args[num_args], XtNtopMargin,    &top_margin);    num_args++;
    XtSetArg(args[num_args], XtNbottomMargin, &bottom_margin); num_args++;
    XtSetArg(args[num_args], XtNleftMargin,   &left_margin);   num_args++;
    XtSetArg(args[num_args], XtNrightMargin,  &right_margin);  num_args++;
    XtGetValues(wp->w, args, num_args);

    text_info->extra_width  = left_margin + right_margin;
    text_info->extra_height = top_margin + bottom_margin;
}

void
destroy_text_window(wp)
    struct xwindow *wp;
{
    /* Don't need to pop down, this only called from dismiss_text(). */

    struct text_info_t *text_info = wp->text_information;

    /*
     * If the text window was blocked, then the user has already ACK'ed
     * it and we are free to really destroy the window.  Otherwise, don't
     * destroy until the user dismisses the window via a key or button
     * press.
     */
    if (text_info->blocked || text_info->destroy_on_ack) {
	XtDestroyWidget(wp->popup);
	free_text_buffer(&text_info->text);
	free((char *) text_info);
	wp->type = NHW_NONE;	/* allow reuse */
    } else {
	text_info->destroy_on_ack = TRUE;	/* destroy on next ACK */
    }
}


/* text buffer routines ---------------------------------------------------- */

/* Append a line to the text buffer. */
void
append_text_buffer(tb, str, concat)
    struct text_buffer *tb;
    const char *str;
    boolean concat;
{
    char *copy;
    int length;

    if (!tb->text) panic("append_text_buffer:  null text buffer");

    if (str) {
	length = strlen(str);
    } else {
	length = 0;
    }

    if (length + tb->text_last + 1 >= tb->text_size) {
	/* we need to go to a bigger buffer! */
#ifdef VERBOSE
	printf("append_text_buffer: text buffer growing from %d to %d bytes\n",
				tb->text_size, 2*tb->text_size);
#endif
	copy = (char *) alloc(tb->text_size*2);
	(void) memcpy(copy, tb->text, tb->text_last);
	free(tb->text);
	tb->text = copy;
	tb->text_size *= 2;
    }

    if (tb->num_lines) {	/* not first --- append a newline */
	char appchar = '\n';

	if(concat && !index("!.?'\")", tb->text[tb->text_last-1])) {
	    appchar = ' ';
	    tb->num_lines--; /* offset increment at end of function */
	}

	*(tb->text + tb->text_last) = appchar;
	tb->text_last++;
    }

    if (str) {
	(void) memcpy((tb->text+tb->text_last), str, length+1);
	if(length) {
	    /* Remove all newlines. Otherwise we have a confused line count. */
	    copy = (tb->text+tb->text_last);
	    while (copy = index(copy, '\n'))
		*copy = ' ';
	}

	tb->text_last += length;
    }
    tb->text[tb->text_last] = '\0';
    tb->num_lines++;
}

/* Initialize text buffer. */
void
init_text_buffer(tb)
    struct text_buffer *tb;
{
    tb->text	  = (char *) alloc(START_SIZE);
    tb->text[0]   = '\0';
    tb->text_size = START_SIZE;
    tb->text_last = 0;
    tb->num_lines = 0;
}

/* Empty the text buffer */
void
clear_text_buffer(tb)
    struct text_buffer *tb;
{
    tb->text_last = 0;
    tb->text[0]   = '\0';
    tb->num_lines = 0;
}

/* Free up allocated memory. */
void
free_text_buffer(tb)
    struct text_buffer *tb;
{
    free(tb->text);
    tb->text = (char *) 0;
    tb->text_size = 0;
    tb->text_last = 0;
    tb->num_lines = 0;
}
