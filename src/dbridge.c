/*	SCCS Id: @(#)dbridge.c	3.0	88/18/12
/* 	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the drawbridge manipulation (create, open, close,
 * destroy).
 */

#include "hack.h"

boolean
is_pool(x,y)
int x,y;
{
	if(levl[x][y].typ == POOL || levl[x][y].typ == MOAT) return TRUE;
#ifdef STRONGHOLD
	if(levl[x][y].typ == DRAWBRIDGE_UP &&
		(levl[x][y].drawbridgemask & DB_UNDER) == DB_MOAT) return TRUE;
#endif
	return FALSE;
}

#ifdef STRONGHOLD
void
initsym(x,y)
int x,y;
{
	char oldseen;
	struct rm *crm = &levl[x][y];

	oldseen = crm->seen;
	crm->seen = 1;
	crm->scrsym = news0(x,y);
	crm->seen = oldseen;
}

static void
redosym(x,y)
int x,y;
{
	if(cansee(x,y)) {
		if (Invisible || x != u.ux || y != u.uy)
			newsym(x,y);
	} else {
		initsym(x,y);
		levl[x][y].seen = 0;
	}
}

/* 
 * We want to know whether a wall (or a door) is the portcullis (passageway)
 * of an eventual drawbridge.
 *
 * Return value: the direction of the drawbridge.
 */

int
is_drawbridge_wall(x,y)
int x,y;
{
	struct rm *lev;

	lev = &levl[x][y];
	if ( lev->typ == VWALL || lev->typ == DOOR) {
		if (IS_DRAWBRIDGE(levl[x+1][y].typ) && 
		    (levl[x+1][y].drawbridgemask & DB_DIR) == DB_WEST)
			return (DB_WEST);
		if (IS_DRAWBRIDGE(levl[x-1][y].typ) && 
		    (levl[x-1][y].drawbridgemask & DB_DIR) == DB_EAST)
			return (DB_EAST);
	}
	if ( lev->typ == HWALL || lev->typ == DOOR) {
		if (IS_DRAWBRIDGE(levl[x][y-1].typ) && 
		    (levl[x][y-1].drawbridgemask & DB_DIR) == DB_SOUTH)
			return (DB_SOUTH);
		if (IS_DRAWBRIDGE(levl[x][y+1].typ) && 
		    (levl[x][y+1].drawbridgemask & DB_DIR) == DB_NORTH)
			return (DB_NORTH);
	}
	return (-1);
}

/*
* Use is_db_wall where you want to verify that a
*  drawbridge "wall" is UP in the location x, y
*  (instead of UP or DOWN, as with is_drawbridge_wall). 
*/ 
boolean
is_db_wall(x,y)
int x,y;
{
	return( (levl[x][y].typ == VWALL || levl[x][y].typ == HWALL) &&
		levl[x][y].diggable & W_GATEWAY);
}

/*
 * Return true with x,y pointing to the drawbridge if x,y initially indicate
 * a drawbridge or drawbridge wall.
 */
boolean
find_drawbridge(x,y)
int *x,*y;
{
	int dir;

	if (IS_DRAWBRIDGE(levl[*x][*y].typ))
		return TRUE;
	dir = is_drawbridge_wall(*x,*y);
	if (dir >= 0) {
		switch(dir) {
		      case DB_NORTH: (*y)++; break;
		      case DB_SOUTH: (*y)--; break;
		      case DB_EAST:  (*x)--; break;
		      case DB_WEST:  (*x)++; break;
		}
		return TRUE;
	}
	return FALSE;
}

/* 
 * Find the drawbridge wall associated with a drawbridge.
 */
static void
get_wall_for_db(x,y)
int *x,*y;
{
	switch (levl[*x][*y].drawbridgemask & DB_DIR) {
	      case DB_NORTH: (*y)--; break;
	      case DB_SOUTH: (*y)++; break;
	      case DB_EAST:  (*x)++; break;
	      case DB_WEST:  (*x)--; break;
	}
}

/*
 * Creation of a drawbridge at pos x,y.
 *	dir is the direction.
 *	flag must be put to TRUE if we want the drawbridge to be opened.
 */

boolean
create_drawbridge(x,y,dir,flag)
int x,y,dir;
boolean flag;
{
	int x2,y2;
	uchar wall;

	x2 = x; y2 = y;
	switch(dir) {
		case DB_NORTH:
			wall = HWALL;
			y2--;
			break;
		case DB_SOUTH:
			wall = HWALL;
			y2++;
			break;
		case DB_EAST:
			wall = VWALL;
			x2++;
			break;
		case DB_WEST:
			wall = VWALL;
			x2--;
			break;
	}
	if (!IS_WALL(levl[x2][y2].typ))
		return(FALSE);
	if (flag) {		/* We want the bridge open */
		levl[x][y].typ = DRAWBRIDGE_DOWN;
		levl[x2][y2].typ = DOOR;
		levl[x2][y2].doormask = D_NODOOR;
	} else {
		levl[x][y].typ = DRAWBRIDGE_UP;
		levl[x2][y2].typ = wall;
		/* Beware, drawbridges are non-diggable. */
		levl[x2][y2].diggable = (W_NONDIGGABLE | W_GATEWAY);
	}
	levl[x][y].drawbridgemask = dir;	/* always have DB_MOAT */
	initsym(x,y);
	initsym(x2,y2);
	return(TRUE);		
}

/*
 * Close the drawbridge located at x,y
 */

void
close_drawbridge(x,y)
int x,y;
{
	register struct rm *lev1, *lev2;
	struct monst *mtmp;
	struct obj *otmp, *otmp2;
	int x2, y2;

	lev1 = &levl[x][y];
	if (lev1->typ != DRAWBRIDGE_DOWN) return;
	if (cansee(x,y))
		You("see a drawbridge going up!");
	lev1->typ = DRAWBRIDGE_UP;
	x2 = x; y2 = y;
	get_wall_for_db(&x2,&y2);
	lev2 = &levl[x2][y2];
	switch (lev1->drawbridgemask & DB_DIR) {
		case DB_NORTH:
		case DB_SOUTH:
			lev2->typ = HWALL;
			break;
		case DB_WEST:
		case DB_EAST:
			lev2->typ = VWALL;
			break;
	}
	lev2->diggable = (W_NONDIGGABLE | W_GATEWAY);
	if ((lev1->drawbridgemask & DB_UNDER) == DB_MOAT) {
	    if (lev1->mmask && !is_flyer((mtmp = m_at(x,y))->data)) {
		if (is_swimmer(mtmp->data)) {
		    if (flags.soundok) You("hear a splash.");
		} else {
		    if (cansee(x,y))
			pline("%s drowns.",Monnam(mtmp));
		    else if (flags.soundok)
			You("hear a splash!");
		    xkilled(mtmp,2);
		}
	    }
	    if (u.ux == x && u.uy == y &&
		!(Levitation
# ifdef POLYSELF
		|| is_flyer(uasmon)
# endif
		)) {
		    /* is the player *THAT* stupid ? */
		    You("fall from the drawbridge.");
		    if (!Wwalking) drown();
	    }
	}
	if (lev2->mmask && !noncorporeal((mtmp = m_at(x2, y2))->data)) {
		if (cansee(x2,y2))
		    pline("%s is crushed by the portcullis.",Monnam(mtmp));
		else if (flags.soundok)
		    You("hear a crushing sound.");
		xkilled(mtmp,2);
	}
	if (u.ux == x2 && u.uy == y2) {	/* More stupidity ? */
		coord xy;

		You("are crushed by a falling portcullis.");
		killer = "closing drawbridge";
		done("died");
		/* So, you didn't die */
		pline("A %s force teleports you away...",
		      Hallucination ? "normal" : "strange");
		enexto(&xy, x2, y2);
		teleds(xy.x, xy.y);
	}
	redosym(x,y);
	for (otmp=fobj;otmp;otmp = otmp2) {
		otmp2 = otmp->nobj;
		if ((otmp->ox == x && otmp->oy == y) || 
		     (otmp->ox == x2 && otmp->oy == y2))
		  delobj(otmp);
	}
	redosym(x2,y2);
}

/* 
 * Open the drawbridge located at x,y
 */

void
open_drawbridge(x,y)
int x,y;
{
	register struct rm *lev1, *lev2;
	int x2, y2;

	lev1 = &levl[x][y];
	if (lev1->typ != DRAWBRIDGE_UP) return;
	if (cansee(x,y))
		You("see a drawbridge coming down!");
	lev1->typ = DRAWBRIDGE_DOWN;
	x2 = x; y2 = y;
	get_wall_for_db(&x2,&y2);
	lev2 = &levl[x2][y2];
	lev2->typ = DOOR;
	lev2->doormask = D_NODOOR;
	if (u.ux == x && u.uy == y) {
		if (cansee(x2,y2))
			newsym(x2,y2);
		You("are hit by the descending drawbridge!");
		killer = "descending drawbridge";
		done("died");
	}
	redosym(x,y);
	redosym(x2,y2);
}

/*
 * Let's destroy the drawbridge located at x,y
 */

void
destroy_drawbridge(x,y)
int x,y;
{
	register struct rm *lev1, *lev2;
	int x2, y2;

	lev1 = &levl[x][y];
	if (!IS_DRAWBRIDGE(lev1->typ))
	  return;
	x2 = x; y2 = y;
	get_wall_for_db(&x2,&y2);
	lev2 = &levl[x2][y2];
	if ((lev1->drawbridgemask & DB_UNDER) == DB_MOAT) {
	    if (lev1->typ == DRAWBRIDGE_UP) {
		if (cansee(x2,y2))
		    pline("The portcullis of the drawbridge falls into the moat!");
		else if (flags.soundok)
		    You("hear a big *SPLASH*!");
	    } else {
		if (cansee(x,y))
		    pline("The drawbridge collapses into the moat!");
		else if (flags.soundok)
		    You("hear a big *SPLASH*!");
	    }
	    lev1->typ = MOAT;
	    if (u.ux == x && u.uy == y && !(Levitation
# ifdef POLYSELF
		|| is_flyer(uasmon)
# endif
		)) {
		    /* is the player *THAT* stupid ? */
		    You("fall from the drawbridge.");
		    if (!Wwalking) drown();
	    }
	} else {
	    if (cansee(x,y))
		pline("The drawbridge disintegrates!");
	    else
		You("hear a big *CRASH*!");
	    lev1->typ = ROOM;
	}
	lev2->typ = DOOR;
	lev2->doormask = D_NODOOR;
	if (u.ux == x2 && u.uy == y2) {	/* More stupidity ? */
		coord xy;

		You("are crushed by a falling portcullis.");
		killer = "collapsing drawbridge";
		done("died");
		/* So, you didn't die */
		pline("A %s force teleports you away...",
		      Hallucination ? "normal" : "strange");
		enexto(&xy, x2, y2);
		teleds(xy.x, xy.y);
	}
	redosym(x,y);
	redosym(x2,y2);
}

#endif /* STRONGHOLD /**/
