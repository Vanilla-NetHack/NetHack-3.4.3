/*	SCCS Id: @(#)sp_lev.h	3.0	88/18/12
/* 	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#define W_NORTH     0
#define W_SOUTH     2
#define W_EAST	    1
#define W_WEST	    3

/* 
 * Structures manipulated by the special levels loader & compiler
 */

typedef struct {
	xchar x, y, mask;
} door;

typedef struct {
	xchar x, y, type;
} trap;

typedef struct {
	xchar x, y, class;
	short id;
} monster;

typedef struct {
	xchar x, y, class;
	short id;
} object;

typedef struct {
	xchar x, y, align, shrine;
} altar;

typedef struct {
	xchar x, y, dir, open;
} drawbridge;

typedef struct {
	xchar x, y, dir;
} walk;

typedef struct {
	xchar x1, y1, x2, y2;
} digpos;

typedef struct {
	xchar x, y, up;
} lad;

typedef struct {
	xchar x1, y1, x2, y2;
	xchar rtype, rlit;
} region;

typedef struct {
	char halign, valign;
	char xsize, ysize;
	char **map;
	char nrobjects;
	char *robjects;
	char nloc;
	char *rloc_x;
	char *rloc_y;
	char nrmonst;
	char *rmonst;
	char nreg;
	region **regions;
	char ndoor;
	door **doors;
	char ntraps;
	trap **traps;
	char nmonster;
	monster **monsters;
	char nobjects;
	object **objects;
	char ndrawbridge;
	drawbridge **drawbridges;
	char nwalk;
	walk **walks;
	char ndig;
	digpos **digs;
	char nlad;
	lad **lads;
#ifdef ALTARS
	char naltar;
	altar **altars;
#endif /* ALTARS /**/
} mazepart;
    
typedef struct {
	char numpart;
	mazepart **parts;
} specialmaze;

typedef struct {
	xchar x, y, w, h;
	xchar rtype, rlit;
	char ndoor;
	door **doors;
	char ntraps;
	trap **traps;
	char nmonster;
	monster **monsters;
	char nobjects;
	object **objects;
#ifdef ALTARS
	char naltar;
	altar **altars;
#endif /* ALTARS /**/
} room;

typedef struct {
	xchar x1,y1, x2,y2;
} corridor;

typedef struct {
	xchar nroom;
	room **rooms;
	xchar ncorr;
	corridor **corrs;
	char ntraps;
	trap **traps;
	char nmonster;
	monster **monsters;
	char nobjects;
	object **objects;
	xchar xdnstairs, ydnstairs;
	xchar xupstairs, yupstairs;
} splev;
