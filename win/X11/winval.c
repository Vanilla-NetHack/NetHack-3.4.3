/*	SCCS Id: @(#)winval.c	3.1	92/3/7
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Routines that define a name-value label widget pair that fit inside a
 * form widget.
 */
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Cardinals.h>

#include "config.h"	/* #define for const for non __STDC__ compilers */

#define NAME "name"
#define VALUE "value"


Widget
create_value(parent, name_value)
    Widget parent;
    const char *name_value;
{
    Widget form, name;
    Arg args[8];
    Cardinal num_args;

    num_args = 0;
    XtSetArg(args[num_args], XtNborderWidth, 0);		num_args++;
    XtSetArg(args[num_args], XtNdefaultDistance, 0);		num_args++;
    form = XtCreateManagedWidget(name_value,
				formWidgetClass,
				parent, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNjustify, XtJustifyRight);	num_args++;
    XtSetArg(args[num_args], XtNborderWidth, 0);		num_args++;
    XtSetArg(args[num_args], XtNlabel, name_value);		num_args++;
    XtSetArg(args[num_args], XtNinternalHeight, 0);		num_args++;
    name = XtCreateManagedWidget(NAME,
				labelWidgetClass,
				form, args, num_args);

    num_args = 0;
    XtSetArg(args[num_args], XtNjustify, XtJustifyRight);	num_args++;
    XtSetArg(args[num_args], XtNborderWidth, 0);		num_args++;
    XtSetArg(args[num_args], XtNfromHoriz, name);		num_args++;
    XtSetArg(args[num_args], XtNinternalHeight, 0);		num_args++;
    (void) XtCreateManagedWidget(VALUE,
				labelWidgetClass,
				form, args, num_args);
    return form;
}

void
set_name(w, new_label)
    Widget w;
    char *new_label;
{
    Arg args[1];
    Widget name;

    name = XtNameToWidget(w, NAME);
    XtSetArg(args[0], XtNlabel, new_label);
    XtSetValues(name, args, ONE);
}

void
set_name_width(w, new_width)
    Widget w;
    int new_width;
{
    Arg args[1];
    Widget name;

    name = XtNameToWidget(w, NAME);
    XtSetArg(args[0], XtNwidth, new_width);
    XtSetValues(name, args, ONE);
}

int
get_name_width(w)
    Widget w;
{
    Arg args[1];
    Dimension width;
    Widget name;

    name = XtNameToWidget(w, NAME);
    XtSetArg(args[0], XtNwidth, &width);
    XtGetValues(name, args, ONE);
    return (int) width;
}


void
set_value(w, new_value)
    Widget w;
    const char *new_value;
{
    Arg args[1];
    Widget val;
    
    val = XtNameToWidget(w, VALUE);
    XtSetArg(args[0], XtNlabel, new_value);
    XtSetValues(val, args, ONE);
}

void
set_value_width(w, new_width)
    Widget w;
    int new_width;
{
    Arg args[1];
    Widget val;
    
    val = XtNameToWidget(w, VALUE);
    XtSetArg(args[0], XtNwidth, new_width);
    XtSetValues(val, args, ONE);
}

int
get_value_width(w)
    Widget w;
{
    Arg args[1];
    Widget val;
    Dimension width;

    val = XtNameToWidget(w, VALUE);
    XtSetArg(args[0], XtNwidth, &width);
    XtGetValues(val, args, ONE);
    return (int) width;
}

/* Swap foreground and background colors (this is the best I can do with */
/* a label widget, unless I can get some init hook in there).		 */
void
hilight_value(w)
    Widget w;
{
    Arg args[2];
    Widget val;
    Pixel fg, bg;

    val = XtNameToWidget(w, VALUE);
    XtSetArg(args[0], XtNforeground, &fg);
    XtSetArg(args[1], XtNbackground, &bg);
    XtGetValues(val, args, TWO);

    XtSetArg(args[0], XtNforeground, bg);
    XtSetArg(args[1], XtNbackground, fg);
    XtSetValues(val, args, TWO);
}

