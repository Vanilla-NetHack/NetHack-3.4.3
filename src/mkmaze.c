/*	SCCS Id: @(#)mkmaze.c	3.0	88/10/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern int x_maze_max, y_maze_max;

#if defined(WALLIFIED_MAZE) || defined(STRONGHOLD)
static int FDECL(iswall,(int,int));
static boolean FDECL(okay,(int,int,int));
static void FDECL(maze0xy,(coord *));

static int
iswall(x,y)
int x,y;
{
# ifndef WALLIFIED_MAZE
	if (x<0 || y<0 || x>COLNO-1 || y>ROWNO-1)
# else
	if (x<0 || y<0 || x>COLNO || y>ROWNO)
# endif
		return 0;
	return (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)
		|| levl[x][y].typ == SDOOR);
}

void
wallification(x1, y1, x2, y2, see)
int x1, y1, x2, y2;
boolean see;
{
	uchar type;
	short x,y;
	register struct rm *room;

	if (x1 < 0) x1 = 0;
	if (x2 < x1) x2 = x1;
	if (x2 > COLNO-1) x2 = COLNO-1;
	if (x1 > x2) x1 = x2;
	if (y1 < 0) y1 = 0;
	if (y2 < y1) y2 = y1;
	if (y2 > COLNO-1) y2 = ROWNO-1;
	if (y1 > y2) y1 = y2;
	for(x = x1; x <= x2; x++)
	    for(y = y1; y <= y2; y++) {
		room = &levl[x][y];
		type = room->typ;
		if (iswall(x,y)) {
		  if (IS_DOOR(type))
		    continue;
		  else
		    if (iswall(x,y-1))
			if (iswall(x,y+1))
			    if (iswall(x-1,y))
				if (iswall(x+1,y))
					room->typ = CROSSWALL;
				else
					room->typ = TLWALL;
			    else
				if (iswall(x+1,y))
					room->typ = TRWALL;
				else
					room->typ = VWALL;
			else
			    if (iswall(x-1,y))
				if (iswall(x+1,y))
					room->typ = TUWALL;
				else
					room->typ = BRCORNER;
			    else
				if (iswall(x+1,y))
					room->typ = BLCORNER;
				else
					room->typ = VWALL;
		    else
			if (iswall(x,y+1))
			    if (iswall(x-1,y))
				if (iswall(x+1,y))
					room->typ = TDWALL;
				else
					room->typ = TRCORNER;
			    else
				if (iswall(x+1,y))
					room->typ = TLCORNER;
				else
					room->typ = VWALL;
			else
				room->typ = HWALL;
		    if (type == SDOOR) room->typ = type;
		}
		room->scrsym = news0(x,y);
		if (see) room->seen = 0;
	    }
}
#endif /* WALLIFIED_MAZE /**/

static boolean
okay(x,y,dir)
int x,y;
register int dir;
{
	move(&x,&y,dir);
	move(&x,&y,dir);
	if(x<3 || y<3 || x>x_maze_max || y>y_maze_max || levl[x][y].typ != 0)
		return(FALSE);
	return(TRUE);
}

static void
maze0xy(cc)	/* find random starting point for maze generation */
	coord	*cc;
{
	cc->x = 3 + 2*rn2((x_maze_max>>1) - 1);
	cc->y = 3 + 2*rn2((y_maze_max>>1) - 1);
	return;
}

/* NCR towers define "tower".  Blecccch. */
#ifdef tower
#  undef tower
#endif

static const uchar NEARDATA tower[] = {
	MOAT,	  MOAT,     MOAT,     MOAT,	MOAT,	  MOAT,     MOAT,
	MOAT,	  MOAT,     TLCORNER, HWALL,	TRCORNER, MOAT,     MOAT,
	MOAT,	  TLCORNER, BRCORNER, ROOM,	BLCORNER, TRCORNER, MOAT,
	MOAT,	  VWALL,    ROOM,     ROOM,	ROOM,	  VWALL,    MOAT,
	MOAT,	  BLCORNER, TRCORNER, ROOM,	TLCORNER, BRCORNER, MOAT,
	MOAT,	  MOAT,     BLCORNER, HWALL,	BRCORNER, MOAT,     MOAT,
	MOAT,	  MOAT,     MOAT,     MOAT,	MOAT,	  MOAT,     MOAT,
};
 
void
makemaz()
{
	int x,y;
	register int zx,zy;
	coord mm;
	int i;

	is_maze_lev = TRUE;
#ifdef STRONGHOLD
	xdnladder = ydnladder = xupladder = yupladder = 0;
	if (dlevel == stronghold_level) {
		if (load_special("castle")) {
			xupstair = yupstair = 3;
			levl[xupstair][yupstair].scrsym = UP_SYM;
			levl[xupstair][yupstair].typ = STAIRS;
			return;
		}
		impossible("Cannot build the STRONGHOLD!");
	}
	if (dlevel == tower_level) {
		if (load_special("tower1")) {
			xupstair = yupstair = 3;
			levl[xupstair][yupstair].scrsym = UP_SYM;
			levl[xupstair][yupstair].typ = STAIRS;
			return;
		}
		impossible("Cannot build the TOWER!");
	}
	if (dlevel == tower_level+1) {
		if (load_special("tower2")) {
			xupstair = yupstair = 3;
			levl[xupstair][yupstair].scrsym = UP_SYM;
			levl[xupstair][yupstair].typ = STAIRS;
			return;
		}
		impossible("Cannot build the TOWER!");
	}
	if (dlevel == tower_level+2) {
		if (load_special("tower3")) {
			xupstair = yupstair = 3;
			levl[xupstair][yupstair].scrsym = UP_SYM;
			levl[xupstair][yupstair].typ = STAIRS;
			return;
		}
		impossible("Cannot build the TOWER!");
	}
# ifdef ENDGAME
	if (dlevel == ENDLEVEL) {	/* EndGame level */
		if (load_special("endgame")) {
			pline("Well done, mortal!");
			pline("But now thou must face the final Test...");
			pline("Prove thyself worthy or perish!");
			u.ux = x_maze_max - 1;
			u.uy = y_maze_max - 1;
			xupstair = yupstair = 0;
			return;
		}
		impossible("Cannot build the EndGame Level!");
		done(ESCAPED);
	}
# endif
#endif
#ifndef WALLIFIED_MAZE
	for(x = 2; x < x_maze_max; x++)
		for(y = 2; y < y_maze_max; y++)
			levl[x][y].typ = STONE;
#else
	for(x = 2; x <= x_maze_max; x++)
		for(y = 2; y <= y_maze_max; y++)
			levl[x][y].typ = ((x % 2) && (y % 2)) ? STONE : HWALL;
#endif

	/* make decoy wizard levels */
	if((dlevel == wiz_level) ||
#ifdef STRONGHOLD
	   (!rn2(3) && (dlevel > stronghold_level+1))) {
#else
	   (!rn2(3) && (dlevel > medusa_level+1))) {
#endif

	    register struct monst *mtmp;

	    zx = x_maze_max / 2;
	    zy = y_maze_max / 2;
	    if (!(zx % 2)) zx++;
	    if (!(zy % 2)) zy++;
	    for(y = zy-3, i=0; y <= zy+3; y++)
		for(x = zx-3; x <= zx+3; x++)
		    levl[x][y].typ = tower[i++];
	    walkfrom(zx+4, zy);
	    if(mtmp = makemon(&mons[PM_HELL_HOUND], zx+1, zy))
		mtmp->msleep = 1;
	    (void) makemon(&mons[PM_KRAKEN], zx+2, zy+2);
	    if (mtmp = makemon(&mons[PM_VAMPIRE_LORD], zx-1, zy))
		mtmp->msleep = 1;
	    if (dlevel == wiz_level) {

		(void) mksobj_at(AMULET_OF_YENDOR, zx, zy);
		flags.made_amulet = 1;
		if(mtmp = makemon(&mons[PM_WIZARD_OF_YENDOR], zx, zy))
			mtmp->msleep = 1;
	    } else {
		struct obj *ot;
	    	/* make a cheap plastic imitation */
		if (ot = mksobj_at(AMULET_OF_YENDOR, zx, zy))
		    ot-> spe = -1;
		(void) makemon(&mons[dprince()], zx, zy);
	    }
	    /* they should wake up when we intrude */
	    (void) maketrap(zx-1, zy, SQBRD);
	    (void) maketrap(zx+1, zy, SQBRD);
	    (void) maketrap(zx, zy-1, SQBRD);
	    (void) maketrap(zx, zy+1, SQBRD);
	} else {
	    maze0xy(&mm);
	    zx = mm.x;
	    zy = mm.y;
	    walkfrom(zx,zy);
#ifndef STRONGHOLD	/* it's in the castle */
# ifdef HARD		/* only one wand of wishing created */
	    if(!rn2(10) || (dlevel == medusa_level + 1))
# endif
		(void) mksobj_at(WAN_WISHING, zx, zy);
#endif
	    (void) mksobj_at(BOULDER, zx, zy);  /* put a boulder on top of it */
	}

#ifdef WALLIFIED_MAZE
	wallification(2, 2, x_maze_max, y_maze_max, TRUE);
#else
	for(x = 2; x < x_maze_max; x++)
		for(y = 2; y < y_maze_max; y++)
			levl[x][y].scrsym = news0(x,y);
#endif
	mazexy(&mm);
	levl[(xupstair = mm.x)][(yupstair = mm.y)].scrsym = UP_SYM;
	levl[xupstair][yupstair].typ = STAIRS;
	xdnstair = ydnstair = 0;
#ifdef STRONGHOLD
	if (dlevel < stronghold_level) {
		mazexy(&mm);
		levl[(xdnstair = mm.x)][(ydnstair = mm.y)].scrsym = DN_SYM;
		levl[xdnstair][ydnstair].typ = STAIRS;
	}
#endif
	for(x = rn1(8,11); x; x--) {
		mazexy(&mm);
		(void) mkobj_at(rn2(2) ? GEM_SYM : 0, mm.x, mm.y, TRUE);
	}
	for(x = rn1(10,2); x; x--) {
		mazexy(&mm);
		(void) mksobj_at(BOULDER, mm.x, mm.y);
	}
	mazexy(&mm);
	(void) makemon(&mons[PM_MINOTAUR], mm.x, mm.y);
	for(x = rn1(5,7); x; x--) {
		mazexy(&mm);
		(void) makemon((struct permonst *) 0, mm.x, mm.y);
	}
	for(x = rn1(6,7); x; x--) {
		mazexy(&mm);
		mkgold(0L,mm.x,mm.y);
	}
	for(x = rn1(6,7); x; x--)
		mktrap(0,1,(struct mkroom *) 0);
}

#ifdef MSDOS
/* Make the mazewalk iterative by faking a stack.  This is needed to
 * ensure the mazewalk is successful in the limited stack space of
 * the program.  This iterative version uses the mimumum amount of stack
 * that is totally safe.
 */
void
walkfrom(x,y)
int x,y;
{
#define CELLS (ROWNO * COLNO) / 4		/* a maze cell is 4 squares */
	char mazex[CELLS + 1], mazey[CELLS + 1];	/* char's are OK */
	int q, a, dir, pos;
	int dirs[4];

	pos = 1;
	mazex[pos] = (char) x;
	mazey[pos] = (char) y;
	while (pos) {
		x = (int) mazex[pos];
		y = (int) mazey[pos];
#ifndef WALLIFIED_MAZE
		levl[x][y].typ = CORR;
#else
		levl[x][y].typ = ROOM;
#endif
		q = 0;
		for (a = 0; a < 4; a++)
			if(okay(x, y, a)) dirs[q++]= a;
		if (!q)
			pos--;
		else {
			dir = dirs[rn2(q)];
			move(&x, &y, dir);
#ifndef WALLIFIED_MAZE
			levl[x][y].typ = CORR;
#else
			levl[x][y].typ = ROOM;
#endif
			move(&x, &y, dir);
			pos++;
			if (pos > CELLS)
				panic("Overflow in walkfrom");
			mazex[pos] = (char) x;
			mazey[pos] = (char) y;
		}
	}
}
#else

void
walkfrom(x,y) int x,y; {
register int q,a,dir;
int dirs[4];
#ifndef WALLIFIED_MAZE
	levl[x][y].typ = CORR;
#else
	levl[x][y].typ = ROOM;
#endif
	while(1) {
		q = 0;
		for(a = 0; a < 4; a++)
			if(okay(x,y,a)) dirs[q++]= a;
		if(!q) return;
		dir = dirs[rn2(q)];
		move(&x,&y,dir);
#ifndef WALLIFIED_MAZE
		levl[x][y].typ = CORR;
#else
		levl[x][y].typ = ROOM;
#endif
		move(&x,&y,dir);
		walkfrom(x,y);
	}
}
#endif /* MSDOS */

void
move(x,y,dir)
register int *x, *y;
register int dir;
{
	switch(dir){
		case 0: --(*y); break;
		case 1: (*x)++; break;
		case 2: (*y)++; break;
		case 3: --(*x); break;
	}
}

void
mazexy(cc)	/* find random point in generated corridors,
		   so we don't create items in moats, bunkers, or walls */
	coord	*cc;
{
	int cpt=0;

	do {
	    cc->x = 3 + 2*rn2((x_maze_max>>1) - 1);
	    cc->y = 3 + 2*rn2((y_maze_max>>1) - 1);
	    cpt++;
#ifndef WALLIFIED_MAZE
	} while (cpt < 100 && levl[cc->x][cc->y].typ != CORR);
#else
	} while (cpt < 100 && levl[cc->x][cc->y].typ != ROOM);
#endif
	if (cpt >= 100) panic("mazexy: can't find a place!");
	return;
}

void
bound_digging()
/* put a non-diggable boundary around the initial portion of a level map.
 * assumes that no level will initially put things beyond the isok() range.
 */
{
	register int x,y;
	register boolean found;
	int xmin,xmax,ymin,ymax;

	found = FALSE;
	for(xmin=1; !found; xmin++)
		for(y=0; y<=ROWNO-1; y++)
			if(levl[xmin][y].typ != STONE) found = TRUE;
	xmin -= 2;

	found = FALSE;
	for(xmax=COLNO-2; !found; xmax--)
		for(y=0; y<=ROWNO-1; y++)
			if(levl[xmax][y].typ != STONE) found = TRUE;
	xmax += 2;

	found = FALSE;
	for(ymin=1; !found; ymin++)
		for(x=xmin; x<=xmax; x++)
			if(levl[x][ymin].typ != STONE) found = TRUE;
	ymin -= 2;

	found = FALSE;
	for(ymax=ROWNO-2; !found; ymax--)
		for(x=xmin; x<=xmax; x++)
			if(levl[x][ymax].typ != STONE) found = TRUE;
	ymax += 2;

	for(x=xmin; x<=xmax; x++) {
		levl[x][ymin].diggable = W_NONDIGGABLE;
		levl[x][ymax].diggable = W_NONDIGGABLE;
	}

	for(y=ymin; y<=ymax; y++) {
		levl[xmin][y].diggable = W_NONDIGGABLE;
		levl[xmax][y].diggable = W_NONDIGGABLE;
	}
}
