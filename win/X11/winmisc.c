/*	SCCS Id: @(#)winmisc.c	3.1	92/10/21	*/
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Misc. popup windows: player selection and extended commands.
 * 
 * 	+ Global functions: player_selection() and get_ext_cmd().
 */
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xos.h>	/* for index() */

#include "hack.h"
#include "func_tab.h"
#include "winX.h"

extern const char *roles[];	/* from u_init.c */

static Widget extended_command_popup;
static Widget extended_command_form;
static int extended_command_selected;	/* index of the selected command; */
static int ps_selected;			/* index of selected role */
#define PS_RANDOM (-50)
#define PS_QUIT   (-75)

static const char extended_command_translations[] =
    "#override\n\
     <Key>: ec_key()";

static const char player_select_translations[] =
    "#override\n\
     <Key>: ps_key()";


static Widget make_menu();


/* Player Selection -------------------------------------------------------- */
/* ARGSUSED */
static void
ps_quit(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    ps_selected = PS_QUIT;
    exit_x_event = TRUE;		/* leave event loop */
}

/* ARGSUSED */
static void
ps_random(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    ps_selected = PS_RANDOM;
    exit_x_event = TRUE;		/* leave event loop */
}

/* ARGSUSED */
static void
ps_select(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    ps_selected = (int) client_data;
    exit_x_event = TRUE;		/* leave event loop */
}

/* ARGSUSED */
void
ps_key(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char ch, *mark;

    ch = key_event_to_char((XKeyEvent *) event);
    if (ch == '\0') {	/* don't accept nul char/modifier event */
	/* don't beep */
	return;
    }
    mark = index(pl_classes, highc(ch));
    if (!mark) {
	X11_nhbell();		/* no such class */
	return;
    }
    ps_selected = mark - pl_classes;
    exit_x_event = TRUE;
}


/* Global functions ========================================================= */
void
X11_player_selection()
{
    char buf[QBUFSZ];
    char pc;
    int num_roles;
    Widget popup, player_form;

    if ((pc = highc(pl_character[0])) != 0) {
	if (index(pl_classes, pc)) goto got_suffix;
	pl_character[0] = pc = 0;
    }

    for (num_roles = 0; roles[num_roles]; num_roles++)
	;	/* do nothing */

    popup = make_menu("player_selection", "Choose a Role",
		player_select_translations,
		"quit", ps_quit,
		"random", ps_random,
		num_roles, roles, ps_select, &player_form);

    ps_selected = 0;
    positionpopup(popup);
    nh_XtPopup(popup, XtGrabExclusive, player_form);

    /* The callbacks will enable the event loop exit. */
    (void) x_event(EXIT_ON_EXIT);

    nh_XtPopdown(popup);
    XtDestroyWidget(popup);

    if (ps_selected == PS_QUIT) {
	clearlocks();
	X11_exit_nhwindows(NULL);
	terminate(0);
    }

    if (ps_selected == PS_RANDOM) {
	winid tmpwin;

	ps_selected = rn2(strlen(pl_classes));
	pc = pl_classes[ps_selected];
	Sprintf(buf, "This game you will be %s.", an(roles[ps_selected]));

	tmpwin = X11_create_nhwindow(NHW_TEXT);
	X11_putstr(tmpwin, 0, "");
	X11_putstr(tmpwin, 0, buf);
	X11_putstr(tmpwin, 0, "");
	X11_display_nhwindow(tmpwin, TRUE);
	X11_destroy_nhwindow(tmpwin);

    } else if (ps_selected < 0 || ps_selected >= num_roles) {
	panic("player_selection: bad select value %d\n", ps_selected);
    } else {
	pc = pl_classes[ps_selected];
    }

got_suffix:
    pl_character[0] = pc;
}


void
X11_get_ext_cmd(input)
    char *input;
{
    extended_command_selected = -1;		/* reset selected value */

    positionpopup(extended_command_popup);	/* center on cursor */
    nh_XtPopup(extended_command_popup, XtGrabExclusive, extended_command_form);

    /* The callbacks will enable the event loop exit. */
    (void) x_event(EXIT_ON_EXIT);

    if (extended_command_selected < 0)
	*input = '\0';
    else
	Strcpy(input, extcmdlist[extended_command_selected].ef_txt);
}

/* End global functions ===================================================== */

/* Extended Command -------------------------------------------------------- */
/* ARGSUSED */
static void
extend_select(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    extended_command_selected = (int) client_data;
    nh_XtPopdown(extended_command_popup);
    exit_x_event = TRUE;		/* leave event loop */
}

/* ARGSUSED */
static void
extend_dismiss(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    extended_command_selected = -1;	/* dismiss */
    nh_XtPopdown(extended_command_popup);
    exit_x_event = TRUE;		/* leave event loop */
}

/* ARGSUSED */
static void
extend_help(w, client_data, call_data)
    Widget w;
    XtPointer client_data, call_data;
{
    /* We might need to make it known that we already have one listed. */
    (void) doextlist();
}

/* ARGSUSED */
void
ec_key(w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    char ch;
    int i, mark;

    ch = key_event_to_char((XKeyEvent *) event);

    if (ch == '\0') {	/* don't accept nul char/modifier event */
	/* don't beep */
	return;
    }
    if (index(quitchars, ch)) {
	extended_command_selected = -1;	/* dismiss */
	goto ec_key_done;
    }

    /*
     * Note: this depends on the fact that the help option "?" is known
     * to be last and not counted.
     */
    for (mark = -1, i = 0; extcmdlist[i].ef_txt; i++) {
	if (extcmdlist[i].ef_txt[0] == '?') continue;

	if (ch == extcmdlist[i].ef_txt[0]) {
	    if (mark != -1) {
		X11_nhbell(); /* another command with the same first letter */
		return;
	    }
	    mark = i;
	}
    }
    if (mark == -1) {
	X11_nhbell();
	return;
    }

    /*
     * It would be nice if we could set the selected command before
     * we pop the window down....  Maybe when I figure out how to do
     * it.
     */
    extended_command_selected = mark;
ec_key_done:
    nh_XtPopdown(extended_command_popup);
    exit_x_event = TRUE;		/* leave event loop */
}

/*
 * Use our own home-brewed version menu because simpleMenu is designed to
 * be used from a menubox.
 */
void
init_extended_commands_popup()
{
    int i, num_commands;
    const char **command_list;

    /* count commands */
    for (num_commands = 0; extcmdlist[num_commands].ef_txt; num_commands++)
	;	/* do nothing */

    /* If the last entry is "help", don't use it. */
    if (strcmp(extcmdlist[num_commands-1].ef_txt, "?") == 0)
	--num_commands;

    command_list = (const char **) alloc(num_commands * sizeof(char *));

    for (i = 0; i < num_commands; i++)
	command_list[i] = extcmdlist[i].ef_txt;

    extended_command_popup = make_menu("extended_commands",
				"Extended Commands",
				extended_command_translations,
				"dismiss", extend_dismiss,
				"help", extend_help,
				num_commands, command_list, extend_select,
				&extended_command_form);

    free((char *)command_list);
}

/* ------------------------------------------------------------------------- */

/*
 * Create a popup widget of the following form:
 * 
 *		      popup_label
 *		----------- ------------
 * 		|left_name| |right_name|
 *		----------- ------------
 *		------------------------
 *		|	name1	       |
 *		------------------------
 *		------------------------
 *		|	name2	       |
 *		------------------------
 *			  .
 *			  .
 *		------------------------
 *		|	nameN	       |
 *		------------------------
 */
static Widget
make_menu(popup_name, popup_label, popup_translations,
		left_name, left_callback,
		right_name, right_callback,
		num_names, widget_names, name_callback, formp)
    char	   *popup_name;
    char	   *popup_label;
    String	   popup_translations;
    char	   *left_name;
    XtCallbackProc left_callback;
    char	   *right_name;
    XtCallbackProc right_callback;
    int		   num_names;
    char	   **widget_names;
    XtCallbackProc name_callback;
    Widget	   *formp;	/* return */
{
    Widget popup, form, label, above, left, right;
    Widget *commands, *curr;
    int i;
    Arg args[8];
    Cardinal num_args;
    Dimension width, max_width;
    int distance, skip;


    commands = (Widget *) alloc(num_names * sizeof(Widget));


    num_args = 0;
    XtSetArg(args[num_args], XtNallowShellResize, True);	num_args++;

    popup = XtCreatePopupShell(popup_name,
				transientShellWidgetClass,
				toplevel, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNtranslations,
		XtParseTranslationTable(popup_translations));	num_args++;
    *formp = form = XtCreateManagedWidget("menuform",
				formWidgetClass,
				popup,
				args, num_args);

    /* Get the default distance between objects in the form widget. */
    num_args = 0;
    XtSetArg(args[num_args], XtNdefaultDistance, &distance);	num_args++;
    XtGetValues(form, args, num_args);

    /*
     * Create the label.
     */
    num_args = 0;
    XtSetArg(args[num_args], XtNborderWidth, 0);	num_args++;
    label = XtCreateManagedWidget(popup_label,
				labelWidgetClass,
				form,
				args, num_args);

    /*
     * Create the left button.
     */
    num_args = 0;
    XtSetArg(args[num_args], XtNfromVert, label);		num_args++;
/*
    XtSetArg(args[num_args], XtNshapeStyle,
				XmuShapeRoundedRectangle);	num_args++;
*/
    left = XtCreateManagedWidget(left_name,
		    commandWidgetClass,
		    form,
		    args, num_args);
    XtAddCallback(left, XtNcallback, left_callback, (XtPointer) 0);
    skip = 3*distance;	/* triple the spacing */
    if(!skip) skip = 3;

    /*
     * Create right button.
     */
    num_args = 0;
    XtSetArg(args[num_args], XtNfromHoriz, left);		num_args++;
    XtSetArg(args[num_args], XtNfromVert, label);		num_args++;
/*
    XtSetArg(args[num_args], XtNshapeStyle,
				XmuShapeRoundedRectangle);	num_args++;
*/
    right = XtCreateManagedWidget(right_name,
		    commandWidgetClass,
		    form,
		    args, num_args);
    XtAddCallback(right, XtNcallback, right_callback, (XtPointer) 0);


    /*
     * Create and place the command widgets.
     */
    for (i = 0, above = left, curr = commands; i < num_names;
					i++, above = *curr, curr++) {
	num_args = 0;
	XtSetArg(args[num_args], XtNfromVert, above);	num_args++;
	if (i == 0) {
	    /* if first, we are farther apart */
	    XtSetArg(args[num_args], XtNvertDistance, skip);	num_args++;
	}

	*curr = XtCreateManagedWidget(widget_names[i],
		    commandWidgetClass,
		    form,
		    args, num_args);
	XtAddCallback(*curr, XtNcallback, name_callback, (XtPointer) i);
    }

    /*
     * Now find the largest width.  Start with the width dismiss + help
     * buttons, since they are adjacent.
     */
    XtSetArg(args[0], XtNwidth, &max_width);
    XtGetValues(left, args, ONE);
    XtSetArg(args[0], XtNwidth, &width);
    XtGetValues(right, args, ONE);
    max_width = max_width + width + distance;

    /* Next, the title. */
    XtSetArg(args[0], XtNwidth, &width);
    XtGetValues(label, args, ONE);
    if (width > max_width) max_width = width;

    /* Finally, the commands. */
    for (i = 0, curr = commands; i < num_names; i++, curr++) {
	XtSetArg(args[0], XtNwidth, &width);
	XtGetValues(*curr, args, ONE);
	if (width > max_width) max_width = width;
    }

    /*
     * Finally, set all of the single line widgets to the largest width.
     */
    XtSetArg(args[0], XtNwidth, max_width);
    XtSetValues(label, args, ONE);

    for (i = 0, curr = commands; i < num_names; i++, curr++) {
	XtSetArg(args[0], XtNwidth, max_width);
	XtSetValues(*curr, args, ONE);
    }

    free((char *) commands);

    XtRealizeWidget(popup);

    return popup;
}
