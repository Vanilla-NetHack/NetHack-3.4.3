/*	SCCS Id: @(#)winmenu.c	3.1	93/02/04	*/
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * File for creating menus.
 * 
 * 	+ Global functions: start_menu, add_menu, end_menu, select_menu
 */

#ifndef SYSV
#define PRESERVE_NO_SYSV	/* X11 include files may define SYSV */
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Viewport.h>
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


static void FDECL(menu_select, (Widget,XtPointer,XtPointer));
static void FDECL(clear_old_menu, (struct xwindow *));
static char *FDECL(copy_of, (const char *));

#define check_menu(func_name)					\
{								\
    if (!menu_info->is_menu) {					\
	impossible("%s:  called before start_menu", func_name);	\
	return;							\
    }								\
}

static char menu_selected;	/* selected menu item */
static const char menu_translations[] =
    "#override\n\
     <Key>: menu_key()";

/*
 * Menu callback.
 */
/* ARGSUSED */
static void
menu_select(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    XawListReturnStruct *lrs = (XawListReturnStruct *) call_data;
    int i;
    struct menu_info_t *menu_info;
    struct menu_item *curr;
    struct xwindow *wp;

    wp = find_widget(w);
    menu_info  = wp->menu_information;

    for (i = 0, curr = menu_info->base; i < lrs->list_index; i++) {
	if (!curr) panic("menu_select: out of menu items!");
	curr = curr->next;
    }

    /* If we don't have a selector, try again. */
    if (!curr->selector) {
	XawListUnhighlight(w);	/* unhilight non-menu item */
	X11_nhbell();
	return;
    }
    menu_selected = curr->selector;

    nh_XtPopdown(wp->popup);	/* this removes the event grab */
    exit_x_event = TRUE;	/* exit our event handler */
}

/*
 * Called when menu window is deleted.
 */
/* ARGSUSED */
void
menu_delete(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    menu_selected = '\033';
    nh_XtPopdown(w);		/* this removes the event grab */
    exit_x_event = TRUE;	/* exit our event handler */
}

/*
 * Called when we get a key press event on a menu window.
 */
/* ARGSUSED */
void
menu_key(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    struct menu_info_t *menu_info;
    struct menu_item *curr;
    struct xwindow *wp;
    char ch;
    int count;

    wp = find_widget(w);
    menu_info  = wp->menu_information;

    ch = key_event_to_char((XKeyEvent *) event);

    if (ch == '\0') {	/* don't accept nul char/modifier event */
	/* don't beep */
	return;
    }

    for (count = 0, curr = menu_info->base; curr; curr = curr->next, count++)
	if (curr->selector == ch) break;

    if (curr) {
	XawListHighlight(w, count);	/* highlit item */
	menu_selected = ch;
    } else if (menu_info->other_valid && index(menu_info->other_valid, ch)) {
	menu_selected = menu_info->other_response;
    } else {
	X11_nhbell();		/* no match */
	return;
    }

    nh_XtPopdown(wp->popup);	/* this removes the event grab */
    exit_x_event = TRUE;	/* exit our event handler */
}


/* Global functions ======================================================== */

void
X11_start_menu(window)
    winid window;
{
    struct xwindow *wp;
    check_winid(window);

    wp = &window_list[window];

    if (wp->menu_information->is_menu) {
	/* clear old menu and widgets (if any) */
	clear_old_menu(wp);
    } else {
	wp->menu_information->is_menu = TRUE;
    }
}

void
X11_add_menu(window, ch, attr, str)
    winid window;
    char ch;
    int attr;
    const char *str;
{
    struct menu_item *item;
    struct menu_info_t *menu_info;

    check_winid(window);
    menu_info = window_list[window].menu_information;
    check_menu("add_menu");

    item = (struct menu_item *) alloc((unsigned)sizeof(struct menu_item));
    item->next = (struct menu_item *) 0;
    item->selector = ch;
    item->attr = attr;
    item->str = copy_of(str);

    if (menu_info->last) {
	menu_info->last->next = item;
    } else {
	menu_info->base = item;
    }
    menu_info->last = item;
    menu_info->count++;
}

void
X11_end_menu(window, cancel_ch, cancel_str, morestr)
    winid window;
    char cancel_ch;
    const char *cancel_str;
    const char *morestr;
{
    struct menu_info_t *menu_info;
    check_winid(window);
    menu_info = window_list[window].menu_information;
    check_menu("end_menu");

    if(morestr && strlen(morestr))
	X11_add_menu(window, 0, 0, morestr);
    menu_info->other_valid = cancel_str;
    menu_info->other_response = cancel_ch;
    menu_info->query = morestr;
}

char
X11_select_menu(window)
    winid window;
{
    struct menu_item *curr;
    struct xwindow *wp;
    struct menu_info_t *menu_info;
    Arg args[8];
    Cardinal num_args;
    String *ptr;
    int i;
    Widget viewport_widget;
    Dimension pixel_height, top_margin, spacing;
    XFontStruct *fs;

    check_winid(window);
    wp = &window_list[window];
    menu_info = wp->menu_information;

#if defined(LINT) || defined(GCC_WARN)
    {
	/* cannot use check_menu, since it doesn't return anything */
	if (!menu_info->is_menu) {
	    impossible("%s:  called before start_menu", "select_menu");
	    return '\0';
	}
    }
#else
    check_menu("select_menu");
#endif

#ifdef VERBOSE
    /* ********** */
    if (menu_info->other_valid) {
	char *cp;
	printf("select_menu: other_valid = \"");
	for (cp = menu_info->other_valid; *cp; cp++) {
	    if (*cp < 32) {
		printf("^%c", '@' + *cp);
	    } else
		printf("%c", *cp);
	}
	printf("\"\n");
    } else {
	printf("select_menu: other_valid = NULL\n");
    }
    if (menu_info->other_response < 32) {
	printf("select_menu: other_response = '^%c'\n",
				    '@' + menu_info->other_response);
    } else {
	printf("select_menu: other_response = '%c'\n",
						menu_info->other_response);
    }
    if (menu_info->query) {
	printf("select_menu: query = \"%s\"\n", menu_info->query);
    } else {
	printf("select_menu: query = NULL\n");
    }
    /* ********** */
#endif

    num_args = 0;
    XtSetArg(args[num_args], XtNallowShellResize, True); num_args++;

    wp->popup = XtCreatePopupShell("menu", transientShellWidgetClass,
				   toplevel, args, num_args);
    XtOverrideTranslations(wp->popup,
	XtParseTranslationTable("<Message>WM_PROTOCOLS: menu_delete()"));

    menu_info->list_pointer =
	(String *) alloc((unsigned) (sizeof(String) * (menu_info->count+1)));
    for (i = 0, ptr = menu_info->list_pointer, curr = menu_info->base;
			i < menu_info->count; i++, ptr++, curr = curr->next) {
	*ptr = (String) curr->str;
    }
    *ptr = (String) 0;

    num_args = 0;
    XtSetArg(args[num_args], XtNallowVert,      True);	       num_args++;

    viewport_widget = XtCreateManagedWidget(
		"menu_viewport",	/* name */
		viewportWidgetClass,
		wp->popup,		/* parent widget */
		args, num_args);	/* values, and number of values */

    num_args = 0;
    XtSetArg(args[num_args], XtNforceColumns, True);		num_args++;
    XtSetArg(args[num_args], XtNdefaultColumns, 1);		num_args++;
    XtSetArg(args[num_args], XtNlist, menu_info->list_pointer);	num_args++;
    XtSetArg(args[num_args], XtNtranslations,
		XtParseTranslationTable(menu_translations));	num_args++;

    wp->w = XtCreateManagedWidget(
		"menu_list",		/* name */
		listWidgetClass,
		viewport_widget,	/* parent widget */
		args,			/* set some values */
		num_args);		/* number of values to set */

    XtAddCallback(wp->w, XtNcallback, menu_select, (XtPointer) 0);

    menu_info->valid_widgets = TRUE;

    /* Get the font and margin information. */
    num_args = 0;
    XtSetArg(args[num_args], XtNfont,	      &fs);	 	num_args++;
    XtSetArg(args[num_args], XtNinternalHeight, &top_margin);	num_args++;
    XtSetArg(args[num_args], XtNrowSpacing,     &spacing);	num_args++;
    XtGetValues(wp->w, args, num_args);

    /* font height is ascent + descent */
    pixel_height = top_margin +
	((menu_info->count + 4) *
	 (fs->max_bounds.ascent + fs->max_bounds.descent + spacing));

    /* if viewport will be bigger than the screen, limit its height */
    if ((Dimension) XtScreen(wp->w)->height <= pixel_height) {
	pixel_height = XtScreen(wp->w)->height / 2;

	num_args = 0;
	XtSetArg(args[num_args], XtNheight, pixel_height); num_args++;
	XtSetValues(viewport_widget, args, num_args);
    }

    XtRealizeWidget(wp->popup);	/* need to realize before we position */
    positionpopup(wp->popup, FALSE);

    menu_selected = '\0';

    nh_XtPopup(wp->popup, (int)XtGrabExclusive, wp->w);
    (void) x_event(EXIT_ON_EXIT);

    return menu_selected;
}

/* End global functions ==================================================== */

static char *
copy_of(s)
    const char *s;
{
    char *copy;
    if (s) {
	copy = (char *) alloc((unsigned) (strlen(s)+1));
	Strcpy(copy,s);
    } else {
	copy = (char *) alloc((unsigned) 1);
	*copy = '\0';
    }

    return copy;
}

static void
clear_old_menu(wp)
    struct xwindow *wp;
{
    struct menu_info_t *menu_info = wp->menu_information;

    while (menu_info->base) {
	menu_info->last = menu_info->base;
	menu_info->base = menu_info->base->next;

	free(menu_info->last->str);
	free((char *)menu_info->last);
    }
    menu_info->last = (struct menu_item *) 0;
    menu_info->other_valid = (char *) 0;
    menu_info->other_response = '\0';
    menu_info->query = (char *) 0;
    menu_info->count = 0;

    if (menu_info->valid_widgets) {
	nh_XtPopdown(wp->popup);
	XtDestroyWidget(wp->popup);
	menu_info->valid_widgets = FALSE;
	free((char *) menu_info->list_pointer);
    }
}

void
create_menu_window(wp)
    struct xwindow *wp;
{
    struct menu_info_t *menu_info;

    wp->type = NHW_MENU;

    wp->menu_information = menu_info = 
		    (struct menu_info_t *) alloc(sizeof(struct menu_info_t));

    menu_info->base	      = (struct menu_item *) 0;
    menu_info->last	      = (struct menu_item *) 0;
    menu_info->query	      = (char *) 0;
    menu_info->other_valid    = (char *) 0;
    menu_info->other_response = '\0';
    menu_info->count	      = 0;
    menu_info->list_pointer   = (String *) 0;
    menu_info->valid_widgets  = FALSE;
    wp->w = wp->popup = (Widget) 0;
    menu_info->is_menu	      = FALSE;
}

void
destroy_menu_window(wp)
    struct xwindow *wp;
{
    /* printf("destroy_menu_window\n"); */

    clear_old_menu(wp);		/* this will also destroy the widgets */
    free((char *) wp->menu_information);

    wp->type = NHW_NONE;	/* allow re-use */
}


