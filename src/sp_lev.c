/*	SCCS Id: @(#)sp_lev.c	3.0	89/01/11
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the various functions that are related to the special
 * levels.
 * It contains also the special level loader.
 *
 */

#include "hack.h"

#ifdef STRONGHOLD
#include "sp_lev.h"

#if defined(MSDOS) && !defined(AMIGA)
# define RDMODE "rb"
#else
# define RDMODE "r"
#endif

#define LEFT	1
#define CENTER	2
#define RIGHT	3
#define TOP	1
#define BOTTOM	3

static walk walklist[50];
extern int x_maze_max, y_maze_max;

static char Map[COLNO][ROWNO];
static char robjects[10], rloc_x[10], rloc_y[10], rmonst[10],
	ralign[3] = { A_CHAOS, A_NEUTRAL, A_LAW };
static xchar xstart, ystart, xsize, ysize;

/*
 * Make walls of the area (x1, y1, x2, y2) non diggable
 */

static void
make_walls_nondiggable(x1,y1,x2,y2)
xchar x1, y1, x2, y2;
{
	register xchar x, y;

	for(y = y1; y <= y2; y++)
	    for(x = x1; x <= x2; x++)
		if(IS_WALL(levl[x][y].typ))
		    levl[x][y].diggable |= W_NONDIGGABLE;
}

/*
 * Choose randomly the state (nodoor, open, closed or locked) for a door
 */

static int
rnddoor()
{
	int i;
	
	i = 1 << rn2(5);
	i >>= 1;
	return i;
}

/* 
 * Select a random trap
 */

static int
rndtrap()
{
	return(rnd(TRAPNUM-1));
}

/* 
 * Coordinates in special level files are handled specially:
 *
 *	if x or y is -11, we generate a random coordinate.
 *	if x or y is between -1 and -10, we read one from the corresponding
 *	register (x0, x1, ... x9).
 *	if x or y is nonnegative, we convert it from relative to the local map
 *	to global coordinates.
 */

static void
get_location(x, y)
schar *x, *y;
{
	int cpt = 0;

	if (*x >= 0) {			/* normal locations */
		*x += xstart;
		*y += ystart;
	} else if (*x > -11) {		/* special locations */
		*y = ystart + rloc_y[ - *y - 1];
		*x = xstart + rloc_x[ - *x - 1];
	} else {			/* random location */
		do {
		    *x = xstart + rn2((int)xsize);
		    *y = ystart + rn2((int)ysize);
		} while (cpt < 100 &&
			 levl[*x][*y].typ != ROOM &&
			 levl[*x][*y].typ != CORR);
		if(cpt >= 100)
		    panic("get_location: can't find a place!");
	}

	if (*x < 0 || *x > x_maze_max || *y < 0 || *y > y_maze_max) {
	    impossible("get_location: (%d,%d) out of bounds", *x, *y);
	    *x = x_maze_max; *y = y_maze_max;
	}
}

/*
 * Shuffle the registers for locations, objects or monsters
 */

static void
shuffle(list, n)
char list[];
xchar n;
{
	int i, j;
	char k;

	for(i = n-1; i; i--) {
		j = rn2(i);

		k = list[j];
		list[j] = list[i];
		list[i] = k;
	}
}

/* 
 * Shuffle two arrays in the same order (for rloc_x & rloc_y)
 */

static void
shuffle2(list1, list2, n)
char list1[], list2[];
xchar n;
{
	int i, j;
	char k1, k2;

	for(i = n-1; i; i--) {
		j = rn2(i);

		k1 = list1[j];
		k2 = list2[j];

		list1[j] = list1[i];
		list2[j] = list2[i];

		list1[i] = k1;
		list2[i] = k2;
	}
}

/* 
 * NOT YET IMPLEMENTED!!!
 */

static boolean
load_rooms(fd)
FILE *fd;
{
	return FALSE;
}

/*
 * Select a random coordinate in the maze.
 *
 * We want a place not 'touched' by the loader.  That is, a place in
 * the maze outside every part of the special level.
 */

static void
maze1xy(m)
coord *m;
{
	do {
		m->x = rn1(x_maze_max - 3, 3);
		m->y = rn1(y_maze_max - 3, 3);
	} while (!(m->x % 2) || !(m->y % 2) || Map[m->x][m->y]);
}

/* 
 * The Big Thing: special maze loader
 *
 * Could be cleaner, but it works.
 */

static boolean
load_maze(fd)
FILE *fd;
{
    xchar   x, y, n, typ;
    char    c;

    xchar   numpart = 0, nwalk = 0;
    uchar   halign, valign;

    int     xi, yi, dir;
    coord   mm;
    int     mapcount, mapcountmax, mapfact;

    region  tmpregion;
    door    tmpdoor;
    trap    tmptrap;
    monster tmpmons;
    object  tmpobj;
    drawbridge tmpdb;
    walk    tmpwalk;
    dig     tmpdig;
    lad     tmplad;
#ifdef ALTARS
    altar   tmpaltar;
#endif

    /* shuffle alignments */
    shuffle(ralign,3);

    /* Initialize map */
    xupstair = yupstair = xdnstair = ydnstair = doorindex = 0;
    for(x = 2; x <= x_maze_max; x++)
	for(y = 2; y <= y_maze_max; y++) {
#ifndef WALLIFIED_MAZE
	    levl[x][y].typ = STONE;
#else
	    levl[x][y].typ = ((x % 2) && (y % 2)) ? STONE : HWALL;
#endif
	    Map[x][y] = 0;
	}

    /* Start reading the file */
    numpart = fgetc(fd); /* Number of parts */
    if (!numpart || numpart > 9)
	panic("load_maze error: numpart = %d", (int) numpart);

    while (numpart--) {
	halign = fgetc(fd); /* Horizontal alignment */
	valign = fgetc(fd); /* Vertical alignment */
	xsize  = fgetc(fd); /* size in X */
	ysize  = fgetc(fd); /* size in Y */

	switch((int) halign) {
	    case LEFT:	    xstart = 3; 				break;
	    case CENTER:    xstart = 2+((x_maze_max-2-xsize)/2);	break;
	    case RIGHT:     xstart = x_maze_max-xsize-1;		break;
	}
	switch((int) valign) {
	    case TOP:	    ystart = 3; 				break;
	    case CENTER:    ystart = 2+((y_maze_max-2-ysize)/2);	break;
	    case BOTTOM:    ystart = y_maze_max-ysize-1;		break;
	}
	if (!(xstart % 2)) xstart++;
	if (!(ystart % 2)) ystart++;

	/* Load the map */
	for(y = ystart; y < ystart+ysize; y++)
	    for(x = xstart; x < xstart+xsize; x++) {
		levl[x][y].typ = fgetc(fd);
		initsym(x,y);
		/* secret doors default closed */
		if (levl[x][y].typ == SDOOR)
		  levl[x][y].doormask = D_CLOSED;
		Map[x][y] = 1;
	    }

	n = fgetc(fd); /* Random objects */
	if(n) {
		(void) fread((genericptr_t)robjects, 1, (int) n, fd);
		shuffle(robjects, n);
	}

	n = fgetc(fd); /* Random locations */
	if(n) {
		(void) fread((genericptr_t)rloc_x, 1, (int) n, fd);
		(void) fread((genericptr_t)rloc_y, 1, (int) n, fd);
		shuffle2(rloc_x, rloc_y, n);
	}

	n = fgetc(fd); /* Random monsters */
	if(n) {
		(void) fread((genericptr_t)rmonst, 1, (int) n, fd);
		shuffle(rmonst, n);
	}

	n = fgetc(fd); /* Number of subrooms */
	while(n--) {
		(void) fread((genericptr_t)&tmpregion, sizeof(tmpregion), 1, fd);
		if (nroom >= MAXNROFROOMS) continue;

		get_location(&tmpregion.x1, &tmpregion.y1);
		get_location(&tmpregion.x2, &tmpregion.y2);

		rooms[nroom].lx = tmpregion.x1;
		rooms[nroom].ly = tmpregion.y1;
		rooms[nroom].hx = tmpregion.x2;
		rooms[nroom].hy = tmpregion.y2;
		rooms[nroom].rtype = tmpregion.rtype;
		rooms[nroom].rlit = tmpregion.rlit;
		if (tmpregion.rlit == 1)
			for(x = rooms[nroom].lx-1; x <= rooms[nroom].hx+1; x++)
				for(y = rooms[nroom].ly-1; y <= rooms[nroom].hy+1; y++)
					levl[x][y].lit = 1;

		rooms[nroom].fdoor = rooms[nroom].doorct = 0;

		++nroom;
		rooms[nroom].hx = -1;
	}

	n = fgetc(fd); /* Number of doors */
	while(n--) {
		struct mkroom *croom = &rooms[0], *broom;
		int tmp;

		(void) fread((genericptr_t)&tmpdoor, sizeof(tmpdoor), 1, fd);

		x = tmpdoor.x;	y = tmpdoor.y;
		typ = tmpdoor.mask == -1 ? rnddoor() : tmpdoor.mask;

		get_location(&x, &y);
		levl[x][y].doormask = typ;

		/* Now the complicated part, list it with each subroom */
		/* The dog move and mail daemon routines use this */
		while(croom->hx >= 0 && doorindex < DOORMAX) {
		    if(croom->hx >= x-1 && croom->lx <= x+1 &&
		       croom->hy >= y-1 && croom->ly <= y+1) {
			/* Found it */
			croom->doorct++;

			/* Append or insert into doors[] */
			broom = croom+1;
			if(broom->hx < 0) tmp = doorindex;
			else
			    for(tmp = doorindex; tmp > broom->fdoor; tmp--)
				doors[tmp] = doors[tmp-1];

			doors[tmp].x = x;
			doors[tmp].y = y;
			doorindex++;

			for( ; broom->hx >= 0; broom++) broom->fdoor++;
		    }
		    croom++;
		}
	}

	n = fgetc(fd); /* Number of traps */
	while(n--) {
		(void) fread((genericptr_t)&tmptrap, sizeof(tmptrap), 1, fd);

		x = tmptrap.x;	y = tmptrap.y;
		typ = (tmptrap.type == -1 ? rndtrap() : tmptrap.type);

		get_location(&x, &y);
		(void) maketrap(x, y, typ);
	}

	n = fgetc(fd);	/* Number of monsters */
	while(n--) {
		(void) fread((genericptr_t)&tmpmons, sizeof(tmpmons), 1, fd);

		x = tmpmons.x;	y = tmpmons.y;
		get_location(&x, &y);

		if	(tmpmons.class >= 0)
			c = tmpmons.class;
		else if (tmpmons.class > -11)
			c = rmonst[-tmpmons.class - 1];
		else
			c = 0;

		if (!c)
			(void) makemon((struct permonst *) 0, x, y);
		else if (tmpmons.id != -1)
			(void) makemon(&mons[tmpmons.id], x, y);
		else
			(void) makemon(mkclass(c), x, y);
	}

	n = fgetc(fd); /* Number of objects */
	while(n--) {
		(void) fread((genericptr_t) &tmpobj, sizeof(object),1, fd);

		x = tmpobj.x;  y = tmpobj.y;
		get_location(&x, &y);

		if	(tmpobj.class >= 0)
			c = tmpobj.class;
		else if (tmpobj.class > -11)
			c = robjects[-tmpobj.class - 1];
		else
			c = 0;

		if (!c)
			(void) mkobj_at(0, x, y);
		else if (tmpobj.id != -1)
			(void) mksobj_at(tmpobj.id, x, y);
		else
			(void) mkobj_at(c, x, y);
	}

	n = fgetc(fd); /* Number of drawbridges */
	while(n--) {
		(void) fread((genericptr_t)&tmpdb, sizeof(tmpdb), 1, fd);

		x = tmpdb.x;  y = tmpdb.y;
		get_location(&x, &y);

		if (!create_drawbridge(x, y, tmpdb.dir, tmpdb.open))
		    impossible("Cannot create drawbridge.");
	}

	n = fgetc(fd); /* Number of mazewalks */
	while(n--) {
		(void) fread((genericptr_t)&tmpwalk, sizeof(tmpwalk), 1, fd);

		get_location(&tmpwalk.x, &tmpwalk.y);

		walklist[nwalk++] = tmpwalk;
	}

	n = fgetc(fd); /* Number of non_diggables */
	while(n--) {
		(void) fread((genericptr_t)&tmpdig, sizeof(tmpdig), 1, fd);

		get_location(&tmpdig.x1, &tmpdig.y1);
		get_location(&tmpdig.x2, &tmpdig.y2);

		make_walls_nondiggable(tmpdig.x1, tmpdig.y1,
				       tmpdig.x2, tmpdig.y2);
	}

	n = fgetc(fd); /* Number of ladders */
	while(n--) {
		(void) fread((genericptr_t)&tmplad, sizeof(tmplad), 1, fd);

		x = tmplad.x;  y = tmplad.y;
		get_location(&x, &y);

		levl[x][y].typ = LADDER;
		if (tmplad.up == 1) {
			xupladder = x;	yupladder = y;
			levl[x][y].ladder = LA_UP;
		} else {
			xdnladder = x;	ydnladder = y;
			levl[x][y].ladder = LA_DOWN;
		}
	}

#ifdef ALTARS
	n = fgetc(fd); /* Number of altars */
	while(n--) {
		(void) fread((genericptr_t)&tmpaltar, sizeof(tmpaltar), 1, fd);

		x = tmpaltar.x;  y = tmpaltar.y;
		get_location(&x, &y);

		typ = tmpaltar.align == -11 ? rn2(3) :
		      tmpaltar.align < 0    ? ralign[-tmpaltar.align-1] :
					      tmpaltar.align;
		if (tmpaltar.shrine)
		    typ |= A_SHRINE;

		levl[x][y].typ = ALTAR;
		levl[x][y].altarmask = typ;
	}
#endif /* ALTARS /**/
    }

    while(nwalk--) {
	    xi = walklist[nwalk].x;
	    yi = walklist[nwalk].y;
	    dir = walklist[nwalk].dir;

	    move(&xi, &yi, dir);
	    x = xi;
	    y = yi;

#ifndef WALLIFIED_MAZE
	    levl[x][y].typ = CORR;
#else
	    levl[x][y].typ = ROOM;
#endif

	    /*
	     * We must be sure that the parity of the coordinates for
	     * walkfrom() is odd.  But we must also take into account
	     * what direction was chosen.
	     */
	    if(!(x % 2))
		if (dir == W_EAST)
		    x++;
		else
		    x--;

#ifndef WALLIFIED_MAZE
	    levl[x][y].typ = CORR;
#else
	    levl[x][y].typ = ROOM;
#endif

	    if (!(y % 2))
		if (dir == W_SOUTH)
		    y++;
		else
		    y--;

	    walkfrom(x, y);
    }
    wallification(2, 2, x_maze_max, y_maze_max, TRUE);

    /*
     * If there's a significant portion of maze unused by the special level,
     * we don't want it empty.
     *
     * Makes the number of traps, monsters, etc. proportional
     * to the size of the maze.
     */
    mapcountmax = mapcount = (x_maze_max - 2) * (y_maze_max - 2);

    for(x = 2; x < x_maze_max; x++)
	for(y = 2; y < y_maze_max; y++)
	    if(Map[x][y]) mapcount--;

    if (mapcount > (int) (mapcountmax / 10)) {
	    mapfact = (int) ((mapcount * 100L) / mapcountmax);
	    for(x = rnd((int) (20 * mapfact) / 100); x; x--) {
		    maze1xy(&mm);
		    (void) mkobj_at(rn2(2) ? GEM_SYM : 0, mm.x, mm.y);
	    }
	    for(x = rnd((int) (12 * mapfact) / 100); x; x--) {
		    maze1xy(&mm);
		    (void) mksobj_at(BOULDER, mm.x, mm.y);
	    }
	    maze1xy(&mm);
	    (void) makemon(&mons[PM_MINOTAUR], mm.x, mm.y);
	    for(x = rnd((int) (12 * mapfact) / 100); x; x--) {
		    maze1xy(&mm);
		    (void) makemon((struct permonst *) 0, mm.x, mm.y);
	    }
	    for(x = rn2((int) (15 * mapfact) / 100); x; x--) {
		    maze1xy(&mm);
		    mkgold(0L,mm.x,mm.y);
	    }
	    for(x = rn2((int) (15 * mapfact) / 100); x; x--) {
		    maze1xy(&mm);
		    (void) maketrap(mm.x, mm.y,rndtrap());
	    }
    }
    return TRUE;
}

/*
 * General loader
 */

boolean
load_special(name)
char *name;
{
	FILE *fd;
	boolean result;
	schar c;

#ifdef OS2_CODEVIEW
	{
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,name);
	fd = fopen(tmp, RDMODE);
#else
	fd = fopen(name, RDMODE);
#endif
#ifdef OS2_CODEVIEW
	}
#endif
	if (!fd) return FALSE;

	if ((c = fgetc(fd)) == EOF) {
		(void)fclose(fd);
		return FALSE;
	}

	switch (c) {
		case 1: 	/* Alas, this is not yet implemented. */
		    result = load_rooms(fd);
		    break;
		case 2: 	/* But this is implemented :-) */
		    result = load_maze(fd);
		    break;
		default:	/* ??? */
		    result = FALSE;
	}
	(void)fclose(fd);
	return result;
}
#endif /* STRONGHOLD /**/
