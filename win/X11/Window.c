/*	SCCS Id: @(#)Window.c	3.1	92/3/7
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Data structures and support routines for the Window widget.  This is a
 * drawing canvas with 16 colors and one font.
 */
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "WindowP.h"

static XtResource resources[] = {
#define offset(field) XtOffset(WindowWidget, window.field)
    /* {name, class, type, size, offset, default_type, default_addr}, */
    { XtNrows, XtCRows, XtRDimension, sizeof(Dimension),
	  offset(rows), XtRImmediate, (XtPointer) 21},
    { XtNcolumns, XtCColumns, XtRDimension, sizeof(Dimension),
	  offset(columns), XtRImmediate, (XtPointer) 80},
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	  offset(foreground), XtRString, XtDefaultForeground },

    { XtNblack, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(black), XtRString, "black"},
    { XtNred, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(red), XtRString, "red" },
    { XtNgreen, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(green), XtRString, "pale green" },
    { XtNbrown, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(brown), XtRString, "brown" },
    { XtNblue, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(blue), XtRString, "blue" },
    { XtNmagenta, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(magenta), XtRString, "magenta" },
    { XtNcyan, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(cyan), XtRString, "light cyan" },
    { XtNgray, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(gray), XtRString, "gray" },
    { XtNorange, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(orange), XtRString, "orange" },
    { XtNbright_green, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(bright_green), XtRString, "green" },
    { XtNyellow, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(yellow), XtRString, "yellow" },
    { XtNbright_blue, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(bright_blue), XtRString, "royal blue" },
    { XtNbright_magenta, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(bright_magenta), XtRString, "violet" },
    { XtNbright_cyan, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(bright_cyan), XtRString, "cyan" },
    { XtNwhite, XtCColor, XtRPixel, sizeof(Pixel),
	  offset(white), XtRString, "white" },

    { XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *),
	  offset(font), XtRString, XtDefaultFont },
    { XtNexposeCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	  offset(expose_callback), XtRCallback, NULL },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	  offset(input_callback), XtRCallback, NULL },
    { XtNresizeCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList),
	  offset(resize_callback), XtRCallback, NULL },
#undef offset
};

/* ARGSUSED */
static void InputAction(w, event, params, num_params)
    Widget   w;
    XEvent   *event;
    String   *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
    XtCallCallbacks(w, XtNcallback, (caddr_t) event);
}

/* ARGSUSED */
static void no_op(w, event, params, num_params)
    Widget   w;			/* unused */
    XEvent   *event;		/* unused */
    String   *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
}

static XtActionsRec actions[] =
{
    {"input",	InputAction},
    {"no-op",	no_op},
};

static char translations[] =
"<Key>:		input()	\n\
 <BtnDown>:	input() \
";

/* ARGSUSED */
static void Redisplay(w, event, region)
    Widget w;
    XEvent *event;	/* unused */
    Region *region;
{
    XtCallCallbacks(w, XtNexposeCallback, (caddr_t) region);
}

/* ARGSUSED */
static void Resize(w)
    Widget w;
{
    XtCallCallbacks(w, XtNresizeCallback, (caddr_t) NULL);
}


WindowClassRec windowClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &widgetClassRec,
    /* class_name		*/	"Window",
    /* widget_size		*/	sizeof(WindowRec),
    /* class_initialize		*/	NULL,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	NULL,
    /* initialize_hook		*/	NULL,
    /* realize			*/	XtInheritRealize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	translations,
    /* query_geometry		*/	XtInheritQueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* window fields */
    /* empty			*/	0
  }
};

WidgetClass windowWidgetClass = (WidgetClass)&windowClassRec;

Font
WindowFont(w) Widget w; { return ((WindowWidget)w)->window.font->fid; }

XFontStruct *
WindowFontStruct(w) Widget w; { return ((WindowWidget)w)->window.font; }
