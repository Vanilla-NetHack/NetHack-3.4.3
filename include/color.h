/*	SCCS Id: @(#)color.h	3.1	92/02/02
/* Copyright (c) Steve Linhart, Eric Raymond, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef COLOR_H
#define COLOR_H

/*
 * The color scheme used is tailored for an IBM PC.  It consists of the
 * standard 8 colors, folowed by their bright counterparts.  There are
 * exceptions, these are listed below.  Bright black doesn't mean very
 * much, so it is used as the "default" foreground color of the screen.
 */
#define BLACK		0
#define RED		1
#define GREEN		2
#define BROWN		3	/* on IBM, low-intensity yellow is brown */
#define BLUE		4
#define MAGENTA 	5
#define CYAN		6
#define GRAY		7	/* low-intensity white */
#define NO_COLOR	8
#define ORANGE_COLORED	9	/* "orange" conflicts with the object */
#define BRIGHT_GREEN	10
#define YELLOW		11
#define BRIGHT_BLUE	12
#define BRIGHT_MAGENTA  13
#define BRIGHT_CYAN	14
#define WHITE		15
#define MAXCOLORS	16

/* The "half-way" point for tty based color systems.  This is used in */
/* the tty color setup code.  (IMHO, it should be removed - dean).    */
#define BRIGHT		8

/* these can be configured */
#define HI_OBJ		MAGENTA
#define HI_METAL	CYAN
#define HI_COPPER	YELLOW
#define HI_SILVER	GRAY
#define HI_GOLD 	YELLOW
#define HI_LEATHER	BROWN
#define HI_CLOTH	BROWN
#define HI_ORGANIC	BROWN
#define HI_WOOD 	BROWN
#define HI_PAPER	WHITE
#define HI_GLASS	BRIGHT_CYAN
#define HI_MINERAL	GRAY
#define HI_ZAP		BRIGHT_BLUE

#endif /* COLOR_H */
