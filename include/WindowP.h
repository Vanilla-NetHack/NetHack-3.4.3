/*	SCCS Id: @(#)WindowP.h	3.1	92/3/7	*/
/* Copyright (c) Dean Luick, 1992				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef _WindowP_h
#define _WindowP_h

#include "Window.h"
/* include superclass private header file */
#include <X11/CoreP.h>

/* define unique representation types not found in <X11/StringDefs.h> */

#define XtRWindowResource "WindowResource"

typedef struct {
    int empty;
} WindowClassPart;

typedef struct _WindowClassRec {
    CoreClassPart	core_class;
    WindowClassPart	window_class;
} WindowClassRec;

extern WindowClassRec windowClassRec;

typedef struct {
    /* resources */
    Dimension	   rows;
    Dimension	   columns;
    Pixel	   foreground;
    Pixel	   black;
    Pixel	   red;
    Pixel	   green;
    Pixel	   brown;
    Pixel	   blue;
    Pixel	   magenta;
    Pixel	   cyan;
    Pixel	   gray;
    Pixel	   orange;
    Pixel	   bright_green;
    Pixel	   yellow;
    Pixel	   bright_blue;
    Pixel	   bright_magenta;
    Pixel	   bright_cyan;
    Pixel	   white;
    XFontStruct	   *font;
    XtCallbackList expose_callback;
    XtCallbackList input_callback;
    XtCallbackList resize_callback;
    /* private state */
    /* (none) */
} WindowPart;

typedef struct _WindowRec {
    CorePart	core;
    WindowPart	window;
} WindowRec;

#endif /* _WindowP_h */
