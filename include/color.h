/*	SCCS Id: @(#)color.h	3.0	89/09/30
/* Copyright (c) Steve Linhart, Eric Raymond, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef COLOR_H
#define COLOR_H

#define BLACK		0
#define RED		1
#define GREEN		2
#define BROWN		3	/* on IBM, lo-intensity yellow is brown */
#define BLUE		4
#define MAGENTA 	5
#define CYAN		6
#define GRAY		7	/* lo-intensity white */
#define BRIGHT		8
#define ORANGE_COLORED	9
#define YELLOW		11
#define WHITE		15
#define MAXCOLORS	16	/* 8 basic + 8 bright */

/* these can be configured */
#define HI_OBJ		MAGENTA
#define HI_METAL	CYAN
#define HI_COPPER	YELLOW
#define HI_SILVER	GRAY
#define HI_GOLD 	YELLOW
#define HI_LEATHER	BROWN
#define HI_CLOTH	BROWN
#define HI_ORGANIC	GREEN
#define HI_WOOD 	BROWN
#define HI_PAPER	WHITE
#define HI_GLASS	CYAN + BRIGHT
#define HI_MINERAL	GRAY
#define HI_ZAP		BLUE + BRIGHT

#endif /* COLOR_H */
