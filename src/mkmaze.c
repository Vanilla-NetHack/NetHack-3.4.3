/*	SCCS Id: @(#)mkmaze.c	3.1	93/01/17	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "sp_lev.h"

#define	UP	1
#define DOWN	0

/* from sp_lev.c, for fixup_special() */
extern char *lev_message;
extern lev_region *lregions;
extern int num_lregions;

static int FDECL(iswall,(int,int));
static boolean FDECL(okay,(int,int,int));
static void FDECL(maze0xy,(coord *));
static boolean FDECL(put_lregion_here,(XCHAR_P,XCHAR_P,XCHAR_P,
	XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,BOOLEAN_P,d_level *));
static void NDECL(fixup_special);
static void NDECL(setup_waterlevel);
static void NDECL(unsetup_waterlevel);

static int
iswall(x,y)
int x,y;
{
	if (x<=0 || y<0 || x>COLNO-1 || y>ROWNO-1)
		return 0;
	return (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)
		|| levl[x][y].typ == SDOOR);
}

void
wallification(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
	uchar type;
	short x,y;
	register struct rm *lev;

	if (x1 < 0) x1 = 0;
	if (x2 < x1) x2 = x1;
	if (x2 > COLNO-1) x2 = COLNO-1;
	if (x1 > x2) x1 = x2;
	if (y1 < 0) y1 = 0;
	if (y2 < y1) y2 = y1;
	if (y2 > ROWNO-1) y2 = ROWNO-1;
	if (y1 > y2) y1 = y2;
	for(x = x1; x <= x2; x++)
	    for(y = y1; y <= y2; y++) {
		lev = &levl[x][y];
		type = lev->typ;
		if (iswall(x,y)) {
		  if (IS_DOOR(type) || type == SDOOR || type == DBWALL)
		    continue;
		  else
		    if (iswall(x,y-1))
			if (iswall(x,y+1))
			    if (iswall(x-1,y))
				if (iswall(x+1,y))
					lev->typ = CROSSWALL;
				else
					lev->typ = TLWALL;
			    else
				if (iswall(x+1,y))
					lev->typ = TRWALL;
				else
					lev->typ = VWALL;
			else
			    if (iswall(x-1,y))
				if (iswall(x+1,y))
					lev->typ = TUWALL;
				else
					lev->typ = BRCORNER;
			    else
				if (iswall(x+1,y))
					lev->typ = BLCORNER;
				else
					lev->typ = VWALL;
		    else
			if (iswall(x,y+1))
			    if (iswall(x-1,y))
				if (iswall(x+1,y))
					lev->typ = TDWALL;
				else
					lev->typ = TRCORNER;
			    else
				if (iswall(x+1,y))
					lev->typ = TLCORNER;
				else
					lev->typ = VWALL;
			else
				lev->typ = HWALL;
		}
	    }
}

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

/*
 * Bad if:
 *	pos is occupied OR
 *	pos is inside restricted region (lx,ly,hx,hy) OR
 *	NOT (pos is corridor and a maze level OR pos is a room OR pos is air)
 */
boolean
bad_location(x, y, lx, ly, hx, hy)
    xchar x, y;
    xchar lx, ly, hx, hy;
{
    return(occupied(x, y) ||
	   ((x >= lx) && (x <= hx) && (y >= ly) && (y <= hy)) ||
	   !((levl[x][y].typ == CORR && level.flags.is_maze_lev) ||
	       levl[x][y].typ == ROOM || levl[x][y].typ == AIR));
}

/* pick a location in area (lx, ly, hx, hy) but not in (nlx, nly, nhx, nhy) */
/* and place something (based on rtype) in that region */
void
place_lregion(lx, ly, hx, hy, nlx, nly, nhx, nhy, rtype, lev)
    xchar	lx, ly, hx, hy;
    xchar	nlx, nly, nhx, nhy;
    xchar	rtype;
    d_level	*lev;
{
    int trycnt;
    boolean oneshot;
    xchar x, y;

    if(!lx) { /* default to whole level */
	/*
	 * if there are rooms and this a branch, let place_branch choose
	 * the branch location (to avoid putting branches in corridors).
	 */
	if(rtype == LR_BRANCH && nroom) {
	    place_branch(Is_branchlev(&u.uz), 0, 0);
	    return;
	}

	lx = 1; hx = COLNO-1;
	ly = 1; hy = ROWNO-1;
    }

    /* first a probabilistic approach */

    oneshot = (lx == hx && ly == hy);
    for(trycnt = 0; trycnt < 100; trycnt ++) {

	x = rn1((hx - lx) + 1, lx);
	y = rn1((hy - ly) + 1, ly);

	if (put_lregion_here(x,y,nlx,nly,nhx,nhy,rtype,oneshot,lev))
	    return;
    }

    /* then a deterministic one */

    oneshot = TRUE;
    for (x = lx; x <= hx; x++)
	for (y = ly; y <= hy; y++)
	    if (put_lregion_here(x,y,nlx,nly,nhx,nhy,rtype,oneshot,lev))
		return;

    impossible("Couldn't place lregion type %d!", rtype);
}

static boolean
put_lregion_here(x,y,nlx,nly,nhx,nhy,rtype,oneshot,lev)
xchar x, y;
xchar nlx, nly, nhx, nhy;
xchar rtype;
boolean oneshot;
d_level *lev;
{
    if(oneshot) {
	/* must make due with the only location possible */
	/* avoid failure due to a misplaced trap */
	/* it might still fail if there's a dungeon feature here */
	struct trap *t = t_at(x,y);
	if (t) deltrap(t);
    }
    if(bad_location(x, y, nlx, nly, nhx, nhy)) return(FALSE);
    switch (rtype) {
    case LR_TELE:
    case LR_UPTELE:
    case LR_DOWNTELE:
	/* "something" means the player in this case */
	if(MON_AT(x, y)) {
	    /* move the monster if no choice, or just try again */
	    if(oneshot) rloc(m_at(x,y));
	    else return(FALSE);
	}
	u.ux = x; u.uy = y;
	break;
    case LR_PORTAL:
	mkportal(x, y, lev->dnum, lev->dlevel);
	break;
    case LR_DOWNSTAIR:
    case LR_UPSTAIR:
	mkstairs(x, y, (char)rtype, (struct mkroom *)0);
	break;
    case LR_BRANCH:
	place_branch(Is_branchlev(&u.uz), x, y);
	break;
    }
    return(TRUE);
}

static boolean was_waterlevel; /* ugh... this shouldn't be needed */

/* this is special stuff that the level compiler cannot (yet) handle */
static void
fixup_special()
{
    register lev_region *r = lregions;
    struct d_level lev;
    register int x, y;
    struct mkroom *croom;
    boolean added_branch = FALSE;

    if (was_waterlevel) {
	was_waterlevel = FALSE;
	u.uinwater = 0;
	unsetup_waterlevel();
    } else if (Is_waterlevel(&u.uz)) {
	level.flags.hero_memory = 0;
	was_waterlevel = TRUE;
	/* water level is an odd beast - it has to be set up
	   before calling place_lregions etc. */
	setup_waterlevel();
    }
    for(x = 0; x < num_lregions; x++, r++) {
	switch(r->rtype) {
	case LR_BRANCH:
	    added_branch = TRUE;
	    goto place_it;

	case LR_PORTAL:
	    if(*r->rname >= '0' && *r->rname <= '9') {
		/* "chutes and ladders" */
		lev = u.uz;
		lev.dlevel = atoi(r->rname);
	    } else
		lev = find_level(r->rname)->dlevel;
	    /* fall into... */

	case LR_UPSTAIR:
	case LR_DOWNSTAIR:
	place_it:
	    place_lregion(r->inarea.x1, r->inarea.y1,
			  r->inarea.x2, r->inarea.y2,
			  r->delarea.x1, r->delarea.y1,
			  r->delarea.x2, r->delarea.y2,
			  r->rtype, &lev);
	    break;

	case LR_TELE:
	case LR_UPTELE:
	case LR_DOWNTELE:
	    /* save the region outlines for goto_level() */
	    if(r->rtype == LR_TELE || r->rtype == LR_UPTELE) {
		    updest.lx = r->inarea.x1; updest.ly = r->inarea.y1;
		    updest.hx = r->inarea.x2; updest.hy = r->inarea.y2;
		    updest.nlx = r->delarea.x1; updest.nly = r->delarea.y1;
		    updest.nhx = r->delarea.x2; updest.nhy = r->delarea.y2;
	    }
	    if(r->rtype == LR_TELE || r->rtype == LR_DOWNTELE) {
		    dndest.lx = r->inarea.x1; dndest.ly = r->inarea.y1;
		    dndest.hx = r->inarea.x2; dndest.hy = r->inarea.y2;
		    dndest.nlx = r->delarea.x1; dndest.nly = r->delarea.y1;
		    dndest.nhx = r->delarea.x2; dndest.nhy = r->delarea.y2;
	    }
	    /* place_lregion gets called from goto_level() */
	    break;
	}
    }

    /* place dungeon branch if not placed above */
    if (!added_branch && Is_branchlev(&u.uz)) {
	place_lregion(0,0,0,0,0,0,0,0,LR_BRANCH,(d_level *)0);
    }

    /* Still need to add some stuff to level file */
    if (Is_medusa_level(&u.uz)) {
	struct obj *otmp;
	int tryct;

	croom = &rooms[0]; /* only one room on the medusa level */
	for (tryct = rn1(1,3); tryct; tryct--) {
	    x = somex(croom); y = somey(croom);
	    if (goodpos(x, y, (struct monst *)0, (struct permonst *)0)) {
		otmp = mk_tt_object(STATUE, x, y);
		while (otmp && (poly_when_stoned(&mons[otmp->corpsenm]) ||
				resists_ston(&mons[otmp->corpsenm]))) {
		    otmp->corpsenm = rndmonnum();
		    otmp->owt = weight(otmp);
		}
	    }
	}

	if (rn2(2))
	    otmp = mk_tt_object(STATUE, somex(croom), somey(croom));
	else /* Medusa statues don't contain books */
	    otmp = mkcorpstat(STATUE, (struct permonst *)0,
			      somex(croom), somey(croom), FALSE);
	if (otmp) {
	    while (resists_ston(&mons[otmp->corpsenm])
		   || poly_when_stoned(&mons[otmp->corpsenm])) {
		otmp->corpsenm = rndmonnum();
		otmp->owt = weight(otmp);
	    }
	}
    } else if(Is_wiz1_level(&u.uz)) {
	croom = search_special(MORGUE);

	create_secret_door(croom, W_SOUTH|W_EAST|W_WEST);
#ifdef MULDGN
    } else if(Is_knox(&u.uz)) {
	/* using an unfilled morgue for rm id */
	croom = search_special(MORGUE);
	/* stock the main vault */
	for(x = croom->lx; x <= croom->hx; x++)
	    for(y = croom->ly; y <= croom->hy; y++) {
		mkgold((long) rn1(300, 600), x, y);
		if(!rn2(3) && !is_pool(x,y)) (void)maketrap(x, y, LANDMINE);
	    }
#endif
    } else if(Is_sanctum(&u.uz)) {
	croom = search_special(TEMPLE);

	create_secret_door(croom, W_ANY);
    } else if(on_level(&u.uz, &orcus_level)) {
	   register struct monst *mtmp, *mtmp2;

	   /* it's a ghost town, get rid of shopkeepers */
	    for(mtmp = fmon; mtmp; mtmp = mtmp2) {
		    mtmp2 = mtmp->nmon;
		    if(mtmp->isshk) mongone(mtmp);
	    }
    }

    if(lev_message) {
	char *str, *nl;
	for(str = lev_message; (nl = index(str, '\n')) != 0; str = nl+1) {
	    *nl = '\0';
	    pline("%s", str);
	}
	if(*str)
	    pline("%s", str);
	free((genericptr_t)lev_message);
	lev_message = 0;
    }
}

void
makemaz(s)
register const char *s;
{
	int x,y;
	char protofile[20];
	s_level	*sp = Is_special(&u.uz);
	coord mm;

	if(*s) {
	    if(sp && sp->rndlevs) Sprintf(protofile, "%s-%d", s,
						rnd((int) sp->rndlevs));
	    else		 Strcpy(protofile, s);
	} else if(*(dungeons[u.uz.dnum].proto)) {
	    if(dunlevs_in_dungeon(&u.uz) > 1) {
		if(sp && sp->rndlevs)
		     Sprintf(protofile, "%s%d-%d", dungeons[u.uz.dnum].proto,
						dunlev(&u.uz),
						rnd((int) sp->rndlevs));
		else Sprintf(protofile, "%s%d", dungeons[u.uz.dnum].proto,
						dunlev(&u.uz));
	    } else if(sp && sp->rndlevs) {
		     Sprintf(protofile, "%s-%d", dungeons[u.uz.dnum].proto,
						rnd((int) sp->rndlevs));
	    } else Strcpy(protofile, dungeons[u.uz.dnum].proto);

	} else Strcpy(protofile, "");

	if(*protofile) {
	    Strcat(protofile, LEV_EXT);
	    if(load_special(protofile)) {
		fixup_special();
		return;	/* no mazification right now */
	    }
	    impossible("Couldn't load '%s' - making a maze.", protofile);
	}

	level.flags.is_maze_lev = TRUE;

#ifndef WALLIFIED_MAZE
	for(x = 2; x < x_maze_max; x++)
		for(y = 2; y < y_maze_max; y++)
			levl[x][y].typ = STONE;
#else
	for(x = 2; x <= x_maze_max; x++)
		for(y = 2; y <= y_maze_max; y++)
			levl[x][y].typ = ((x % 2) && (y % 2)) ? STONE : HWALL;
#endif

	maze0xy(&mm);
	walkfrom((int) mm.x, (int) mm.y);
	/* put a boulder at the maze center */
	(void) mksobj_at(BOULDER, (int) mm.x, (int) mm.y, TRUE);

#ifdef WALLIFIED_MAZE
	wallification(2, 2, x_maze_max, y_maze_max);
#else
	for(x = 2; x < x_maze_max; x++)
		for(y = 2; y < y_maze_max; y++)
			levl[x][y].seen = 1;	/* start out seen */
#endif
	if(Invocation_lev(&u.uz)) {
		place_lregion(0,0,0,0, 30, 0, 46, ROWNO, UP, (d_level *)0);
		do {
			if(xupstair < 30)
				x = rn1(COLNO-16-xupstair, xupstair+7);
			else
				x = rn1(xupstair-16, 9);
			y = rn1(7, 8);
		} while((levl[x][y].typ != CORR && levl[x][y].typ != ROOM)
			|| occupied(x,y));
		inv_pos.x = x;
		inv_pos.y = y;
	} else {
	    /* no regular up stairs on the first level of a dungeon) */
	    if(u.uz.dlevel != 1) {
		mazexy(&mm);
		mkstairs(mm.x, mm.y, UP, (struct mkroom *)0);
	    }

	    /* no regular down stairs on the last level of a dungeon */
	    if(dunlev(&u.uz) != dunlevs_in_dungeon(&u.uz)) {
		mazexy(&mm);
		mkstairs(mm.x, mm.y, DOWN, (struct mkroom *)0);
	    }
	}

	/* place branch stair or portal */
	place_branch(Is_branchlev(&u.uz), 0, 0);

	for(x = rn1(8,11); x; x--) {
		mazexy(&mm);
		(void) mkobj_at(rn2(2) ? GEM_CLASS : 0, mm.x, mm.y, TRUE);
	}
	for(x = rn1(10,2); x; x--) {
		mazexy(&mm);
		(void) mksobj_at(BOULDER, mm.x, mm.y, TRUE);
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
		mktrap(0,1,(struct mkroom *) 0, (coord*) 0);
}

#ifdef MICRO
/* Make the mazewalk iterative by faking a stack.  This is needed to
 * ensure the mazewalk is successful in the limited stack space of
 * the program.  This iterative version uses the minimum amount of stack
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
		if(!IS_DOOR(levl[x][y].typ)) {
		    /* might still be on edge of MAP, so don't overwrite */
#ifndef WALLIFIED_MAZE
		    levl[x][y].typ = CORR;
#else
		    levl[x][y].typ = ROOM;
#endif
		    levl[x][y].flags = 0;
		}
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
walkfrom(x,y)
int x,y;
{
	register int q,a,dir;
	int dirs[4];

	if(!IS_DOOR(levl[x][y].typ)) {
	    /* might still be on edge of MAP, so don't overwrite */
#ifndef WALLIFIED_MAZE
	    levl[x][y].typ = CORR;
#else
	    levl[x][y].typ = ROOM;
#endif
	    levl[x][y].flags = 0;
	}

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
#endif /* MICRO */

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
	} while (cpt < 100 && levl[cc->x][cc->y].typ !=
#ifndef WALLIFIED_MAZE
		 CORR
#else
		 ROOM
#endif
		);
	if (cpt >= 100) {
		register int x, y;
		/* last try */
		for (x = 0; x < (x_maze_max>>1) - 1; x++)
		    for (y = 0; y < (y_maze_max>>1) - 1; y++) {
			cc->x = 3 + 2 * x;
			cc->y = 3 + 2 * y;
			if (levl[cc->x][cc->y].typ ==
#ifndef WALLIFIED_MAZE
			    CORR
#else
			    ROOM
#endif
			   ) return;
		    }
		panic("mazexy: can't find a place!");
	}
	return;
}

void
bound_digging()
/* put a non-diggable boundary around the initial portion of a level map.
 * assumes that no level will initially put things beyond the isok() range.
 *
 * we can't bound unconditionally on the last line with something in it,
 * because that something might be a niche which was already reachable,
 * so the boundary would be breached
 *
 * we can't bound unconditionally on one beyond the last line, because
 * that provides a window of abuse for WALLIFIED_MAZE special levels
 */
{
	register int x,y;
	register unsigned typ;
	register struct rm *lev;
	boolean found, nonwall;
	int xmin,xmax,ymin,ymax;

	if(Is_earthlevel(&u.uz)) return; /* everything diggable here */

	found = nonwall = FALSE;
	for(xmin=0; !found; xmin++) {
		lev = &levl[xmin][0];
		for(y=0; y<=ROWNO-1; y++, lev++) {
			typ = lev->typ;
			if(typ != STONE) {
				found = TRUE;
				if(!IS_WALL(typ)) nonwall = TRUE;
			}
		}
	}
	xmin -= (nonwall || !level.flags.is_maze_lev) ? 2 : 1;
	if (xmin < 0) xmin = 0;

	found = nonwall = FALSE;
	for(xmax=COLNO-1; !found; xmax--) {
		lev = &levl[xmax][0];
		for(y=0; y<=ROWNO-1; y++, lev++) {
			typ = lev->typ;
			if(typ != STONE) {
				found = TRUE;
				if(!IS_WALL(typ)) nonwall = TRUE;
			}
		}
	}
	xmax += (nonwall || !level.flags.is_maze_lev) ? 2 : 1;
	if (xmax >= COLNO) xmax = COLNO-1;

	found = nonwall = FALSE;
	for(ymin=0; !found; ymin++) {
		lev = &levl[xmin][ymin];
		for(x=xmin; x<=xmax; x++, lev += ROWNO) {
			typ = lev->typ;
			if(typ != STONE) {
				found = TRUE;
				if(!IS_WALL(typ)) nonwall = TRUE;
			}
		}
	}
	ymin -= (nonwall || !level.flags.is_maze_lev) ? 2 : 1;

	found = nonwall = FALSE;
	for(ymax=ROWNO-1; !found; ymax--) {
		lev = &levl[xmin][ymax];
		for(x=xmin; x<=xmax; x++, lev += ROWNO) {
			typ = lev->typ;
			if(typ != STONE) {
				found = TRUE;
				if(!IS_WALL(typ)) nonwall = TRUE;
			}
		}
	}
	ymax += (nonwall || !level.flags.is_maze_lev) ? 2 : 1;

	if(ymin >= 0)
	    for(x=xmin; x<=xmax; x++) levl[x][ymin].diggable = W_NONDIGGABLE;
	if(ymax < ROWNO)
	    for(x=xmin; x<=xmax; x++) levl[x][ymax].diggable = W_NONDIGGABLE;

	/* Don't bound these until _after_ the previous loops to avoid "ice" */
	/* Normal rooms become ice by setting W_NONDIGGABLE -dlc */
	if (ymin < 0) ymin = 0;
	if (ymax >= ROWNO) ymax = ROWNO-1;

	for(y=ymin; y<=ymax; y++) {
		levl[xmin][y].diggable = W_NONDIGGABLE;
		levl[xmax][y].diggable = W_NONDIGGABLE;
	}
}

void
mkportal(x, y, todnum, todlevel)
register xchar x, y, todnum, todlevel;
{
	/* a portal "trap" must be matched by a */
	/* portal in the destination dungeon/dlevel */
	register struct trap *ttmp = maketrap(x, y, MAGIC_PORTAL);

#ifdef DEBUG
	pline("mkportal: at (%d,%d), to %s, level %d",
		x, y, dungeons[todnum].dname, todlevel);
#endif
	ttmp->dst.dnum = todnum;
	ttmp->dst.dlevel = todlevel;
	return;
}

/*
 * Special waterlevel stuff in endgame (TH).
 *
 * Some of these functions would probably logically belong to some
 * other source files, but they are all so nicely encapsulated here.
 */

/* to ease the work of debuggers at this stage */
#define register

struct container {
	struct container *next;
	xchar x, y;
	short what;
	genericptr_t list;
};
#define CONS_OBJ   0
#define CONS_MON   1
#define CONS_HERO  2
#define CONS_TRAP  3

static struct bubble {
	xchar x, y;	/* coordinates of the upper left corner */
	schar dx, dy;	/* the general direction of the bubble's movement */
	uchar *bm;	/* pointer to the bubble bit mask */
	struct bubble *prev, *next; /* need to traverse the list up and down */
	struct container *cons;
} *bbubbles, *ebubbles;

static struct trap *wportal;
static int xmin, ymin, xmax, ymax;	/* level boundaries */
/* bubble movement boundaries */
#define bxmin (xmin + 1)
#define bymin (ymin + 1)
#define bxmax (xmax - 1)
#define bymax (ymax - 1)

static void NDECL(set_wportal);
static void FDECL(mk_bubble, (int,int,int));
static void FDECL(mv_bubble, (struct bubble *,int,int,BOOLEAN_P));

void
movebubbles()
{
	static boolean up;
	register struct bubble *b;
	register int x, y, i, j;
	struct trap *btrap;
	static const struct rm water_pos =
		{ cmap_to_glyph(S_water), WATER, 0, 0, 0, 0, 0, 0, 0 };

	/* set up the portal the first time bubbles are moved */
	if (!wportal) set_wportal();

	vision_recalc(2);

	/*
	 * Pick up everything inside of a bubble then fill all bubble
	 * locations.
	 */

	for (b = up ? bbubbles : ebubbles; b; b = up ? b->next : b->prev) {
	    if (b->cons) panic("movebubbles: cons != null");
	    for (i = 0, x = b->x; i < (int) b->bm[0]; i++, x++)
		for (j = 0, y = b->y; j < (int) b->bm[1]; j++, y++)
		    if (b->bm[j + 2] & (1 << i)) {
			if (!isok(x,y)) {
			    impossible("movebubbles: bad pos (%d,%d)", x,y);
			    continue;
			}

			/* pick up objects, monsters, hero, and traps */
			if (OBJ_AT(x,y)) {
			    struct obj *olist = (struct obj *) 0, *otmp;
			    struct container *cons = (struct container *)
				alloc(sizeof(struct container));

			    while ((otmp = level.objects[x][y]) != 0) {
				remove_object(otmp);
				otmp->ox = otmp->oy = 0;
				otmp->nexthere = olist;
				olist = otmp;
			    }

			    cons->x = x;
			    cons->y = y;
			    cons->what = CONS_OBJ;
			    cons->list = (genericptr_t) olist;
			    cons->next = b->cons;
			    b->cons = cons;
			}
			if (MON_AT(x,y)) {
			    struct monst *mon = m_at(x,y);
			    struct container *cons = (struct container *)
				alloc(sizeof(struct container));

			    cons->x = x;
			    cons->y = y;
			    cons->what = CONS_MON;
			    cons->list = (genericptr_t) mon;

			    cons->next = b->cons;
			    b->cons = cons;

			    if(mon->wormno)
				remove_worm(mon);
			    else
				remove_monster(x, y);

			    newsym(x,y);	/* clean up old position */
			    mon->mx = mon->my = 0;
			}
			if (!u.uswallow && x == u.ux && y == u.uy) {
			    struct container *cons = (struct container *)
				alloc(sizeof(struct container));

			    cons->x = x;
			    cons->y = y;
			    cons->what = CONS_HERO;
			    cons->list = (genericptr_t) 0;

			    cons->next = b->cons;
			    b->cons = cons;
			}
			if ((btrap = t_at(x,y)) != 0) {
			    struct container *cons = (struct container *)
				alloc(sizeof(struct container));

			    cons->x = x;
			    cons->y = y;
			    cons->what = CONS_TRAP;
			    cons->list = (genericptr_t) btrap;

			    cons->next = b->cons;
			    b->cons = cons;
			}

			levl[x][y] = water_pos;
			block_point(x,y);
		    }
	}

	/*
	 * Every second time traverse down.  This is because otherwise
	 * all the junk that changes owners when bubbles overlap
	 * would eventually end up in the last bubble in the chain.
	 */

	up = !up;
	for (b = up ? bbubbles : ebubbles; b; b = up ? b->next : b->prev) {
		register int rx = rn2(3), ry = rn2(3);

		mv_bubble(b,b->dx + 1 - (!b->dx ? rx : (rx ? 1 : 0)),
			    b->dy + 1 - (!b->dy ? ry : (ry ? 1 : 0)),
			    FALSE);
	}

	vision_full_recalc = 1;
}

void
water_friction()
{
	register boolean eff = FALSE;

	if (u.dx && !rn2(3)) {
		eff = TRUE;
		u.dx = 0;
	}
	if (u.dy && !rn2(3)) {
		eff = TRUE;
		u.dy = 0;
	}
	if (eff) pline("Water turbulence affects your movements.");
}

void
save_waterlevel(fd)
register int fd;
{
	register struct bubble *b;
	int n;

	if (!Is_waterlevel(&u.uz)) return;

	for (b = bbubbles, n = 0; b; b = b->next, n++) ;
	bwrite(fd,(genericptr_t)&n,sizeof(int));
	bwrite(fd,(genericptr_t)&xmin,sizeof(int));
	bwrite(fd,(genericptr_t)&ymin,sizeof(int));
	bwrite(fd,(genericptr_t)&xmax,sizeof(int));
	bwrite(fd,(genericptr_t)&ymax,sizeof(int));
	for (b = bbubbles; b; b = b->next)
		bwrite(fd,(genericptr_t)b,sizeof(struct bubble));
}

void
restore_waterlevel(fd)
register int fd;
{
	register struct bubble *b = (struct bubble *)0, *btmp;
	register int i;
	int n;

	if (!Is_waterlevel(&u.uz)) return;

	set_wportal();
	mread(fd,(genericptr_t)&n,sizeof(int));
	mread(fd,(genericptr_t)&xmin,sizeof(int));
	mread(fd,(genericptr_t)&ymin,sizeof(int));
	mread(fd,(genericptr_t)&xmax,sizeof(int));
	mread(fd,(genericptr_t)&ymax,sizeof(int));
	for (i = 0; i < n; i++) {
		btmp = b;
		b = (struct bubble *)alloc(sizeof(struct bubble));
		mread(fd,(genericptr_t)b,sizeof(struct bubble));
		if (bbubbles) {
			btmp->next = b;
			b->prev = btmp;
		} else {
			bbubbles = b;
			b->prev = (struct bubble *)0;
		}
		mv_bubble(b,0,0,TRUE);
	}
	ebubbles = b;
	b->next = (struct bubble *)0;
	was_waterlevel = TRUE;
}

static void
set_wportal()
{
	/* there better be only one magic portal on water level... */
	for (wportal = ftrap; wportal; wportal = wportal->ntrap)
		if (wportal->ttyp == MAGIC_PORTAL) return;
	impossible("set_wportal(): no portal!");
}

static void
setup_waterlevel()
{
	register int x, y;
	register int xskip, yskip;
	register int water_glyph = cmap_to_glyph(S_water);

	/* ouch, hardcoded... */

	xmin = 3;
	ymin = 1;
	xmax = 78;
	ymax = 20;

	/* set hero's memory to water */

	for (x = xmin; x <= xmax; x++)
		for (y = ymin; y <= ymax; y++)
			levl[x][y].glyph = water_glyph;

	/* make bubbles */

	xskip = 10 + rn2(10);
	yskip = 4 + rn2(4);
	for (x = bxmin; x <= bxmax; x += xskip)
		for (y = bymin; y <= bymax; y += yskip)
			mk_bubble(x,y,rn2(7));
}

static void
unsetup_waterlevel()
{
	register struct bubble *b, *bb;

	/* free bubbles */

	for (b = bbubbles; b; b = bb) {
		bb = b->next;
		free((genericptr_t)b);
	}
	bbubbles = ebubbles = (struct bubble *)0;
}

static void
mk_bubble(x,y,n)
register int x, y, n;
{
	/*
	 * These bit masks make visually pleasing bubbles on a normal aspect
	 * 25x80 terminal, which naturally results in them being mathematically
	 * anything but symmetric.  For this reason they cannot be computed
	 * in situ, either.  The first two elements tell the dimensions of
	 * the bubble's bounding box.
	 */
	static uchar
		bm2[] = {2,1,0x3},
		bm3[] = {3,2,0x7,0x7},
		bm4[] = {4,3,0x6,0xf,0x6},
		bm5[] = {5,3,0xe,0x1f,0xe},
		bm6[] = {6,4,0x1e,0x3f,0x3f,0x1e},
		bm7[] = {7,4,0x3e,0x7f,0x7f,0x3e},
		bm8[] = {8,4,0x7e,0xff,0xff,0x7e},
		*bmask[] = {bm2,bm3,bm4,bm5,bm6,bm7,bm8};

	register struct bubble *b;

	if (x >= bxmax || y >= bymax) return;
	if (n >= SIZE(bmask)) {
		impossible("n too large (mk_bubble)");
		n = SIZE(bmask) - 1;
	}
	b = (struct bubble *)alloc(sizeof(struct bubble));
	if ((x + (int) bmask[n][0] - 1) > bxmax) x = bxmax - bmask[n][0] + 1;
	if ((y + (int) bmask[n][1] - 1) > bymax) y = bymax - bmask[n][1] + 1;
	b->x = x;
	b->y = y;
	b->dx = 1 - rn2(3);
	b->dy = 1 - rn2(3);
	b->bm = bmask[n];
	b->cons = 0;
	if (!bbubbles) bbubbles = b;
	if (ebubbles) {
		ebubbles->next = b;
		b->prev = ebubbles;
	}
	else
		b->prev = (struct bubble *)0;
	b->next =  (struct bubble *)0;
	ebubbles = b;
	mv_bubble(b,0,0,TRUE);
}

/*
 * The player, the portal and all other objects and monsters
 * float along with their associated bubbles.  Bubbles may overlap
 * freely, and the contents may get associated with other bubbles in
 * the process.  Bubbles are "sticky", meaning that if the player is
 * in the immediate neighborhood of one, he/she may get sucked inside.
 * This property also makes leaving a bubble slightly difficult.
 */
static void
mv_bubble(b,dx,dy,ini)
register struct bubble *b;
register int dx, dy;
register boolean ini;
{
	register int x, y, i, j, colli = 0;
	struct bubble ob;
	struct container *cons, *ctemp;

	/* some old data for reference */

	ob.x = b->x;
	ob.y = b->y;
	ob.bm = b->bm;

	/* move bubble */
	if (dx < -1 || dx > 1 || dy < -1 || dy > 1) {
	    /* pline("mv_bubble: dx = %d, dy = %d", dx, dy); */
	    dx = sgn(dx);
	    dy = sgn(dy);
	}

	/*
	 * collision with level borders?
	 *	1 = horizontal border, 2 = vertical, 3 = corner
	 */
	if (b->x <= bxmin) colli |= 2;
	if (b->y <= bymin) colli |= 1;
	if ((int) (b->x + b->bm[0] - 1) >= bxmax) colli |= 2;
	if ((int) (b->y + b->bm[1] - 1) >= bymax) colli |= 1;

	if (b->x < bxmin) {
	    pline("bubble xmin: x = %d, xmin = %d", b->x, bxmin);
	    b->x = bxmin;
	}
	if (b->y < bymin) {
	    pline("bubble ymin: y = %d, ymin = %d", b->y, bymin);
	    b->y = bymin;
	}
	if ((int) (b->x + b->bm[0] - 1) > bxmax) {
	    pline("bubble xmax: x = %d, xmax = %d",
			b->x + b->bm[0] - 1, bxmax);
	    b->x = bxmax - b->bm[0] + 1;
	}
	if ((int) (b->y + b->bm[1] - 1) > bymax) {
	    pline("bubble ymax: y = %d, ymax = %d",
			b->y + b->bm[1] - 1, bymax);
	    b->y = bymax - b->bm[1] + 1;
	}

	/* bounce if we're trying to move off the border */
	if (b->x == bxmin && dx < 0) dx = -dx;
	if (b->x + b->bm[0] - 1 == bxmax && dx > 0) dx = -dx;
	if (b->y == bymin && dy < 0) dy = -dy;
	if (b->y + b->bm[1] - 1 == bymax && dy > 0) dy = -dy;

	b->x += dx;
	b->y += dy;

	/* void positions inside bubble */

	for (i = 0, x = b->x; i < (int) b->bm[0]; i++, x++)
	    for (j = 0, y = b->y; j < (int) b->bm[1]; j++, y++)
		if (b->bm[j + 2] & (1 << i)) {
		    levl[x][y].typ = AIR;
		    levl[x][y].lit = 1;
		    unblock_point(x,y);
		}

	/* replace contents of bubble */
	for (cons = b->cons; cons; cons = ctemp) {
	    ctemp = cons->next;
	    cons->x += dx;
	    cons->y += dy;

	    switch(cons->what) {
		case CONS_OBJ: {
		    struct obj *olist, *otmp;

		    for (olist=(struct obj *)cons->list; olist; olist=otmp) {
			otmp = olist->nexthere;
			place_object(olist, cons->x, cons->y);
		    }
		    break;
		}

		case CONS_MON: {
		    struct monst *mon = (struct monst *) cons->list;
		    (void) mnearto(mon, cons->x, cons->y, TRUE);
		    break;
		}

		case CONS_HERO: {
		    int ux0 = u.ux, uy0 = u.uy;

		    /* change u.ux0 and u.uy0? */
		    u.ux = cons->x;
		    u.uy = cons->y;
		    newsym(ux0, uy0);	/* clean up old position */

		    if (MON_AT(cons->x, cons->y)) {
				mnexto(m_at(cons->x,cons->y));
			}
		    if (Punished) placebc();	/* do this for now */
		    break;
		}

		case CONS_TRAP: {
		    struct trap *btrap = (struct trap *) cons->list;
		    btrap->tx = cons->x;
		    btrap->ty = cons->y;
		    break;
		}

		default:
		    impossible("mv_bubble: unknown bubble contents");
		    break;
	    }
	    free((genericptr_t)cons);
	}
	b->cons = 0;

	/* boing? */

	switch (colli) {
	    case 1: b->dy = -b->dy;	break;
	    case 3: b->dy = -b->dy;	/* fall through */
	    case 2: b->dx = -b->dx;	break;
	    default:
		/* sometimes alter direction for fun anyway
		   (higher probability for stationary bubbles) */
		if (!ini && ((b->dx || b->dy) ? !rn2(20) : !rn2(5))) {
			b->dx = 1 - rn2(3);
			b->dy = 1 - rn2(3);
		}
	}
}


/*mkmaze.c*/
