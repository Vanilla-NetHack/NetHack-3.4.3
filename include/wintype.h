/*	SCCS Id: @(#)wintype.h	3.1	91/07/03	*/
/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINTYPE_H
#define WINTYPE_H

/* window types */
/* any additional port specific types should be defined in win*.h */
#define NHW_MESSAGE 1
#define NHW_STATUS  2
#define NHW_MAP     3
#define NHW_MENU    4
#define NHW_TEXT    5

/* attribute types for putstr; the same as the ANSI value, for convenience */
#define ATR_NONE    0
#define ATR_BOLD    1
#define ATR_DIM     2
#define ATR_ULINE   4
#define ATR_BLINK   5
#define ATR_INVERSE 7

/* nh_poskey() modifier types */
#define CLICK_1     1
#define CLICK_2	    2

/* invalid winid */
#define WIN_ERR ((winid) -1)

#endif /* WINTYPE_H */
