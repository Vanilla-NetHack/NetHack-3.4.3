/*     SCCS Id: @(#)dbridge.c  3.0     88/18/12
/*     Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the drawbridge manipulation (create, open, close,
 * destroy).
 *
 * Added comprehensive monster-handling, and the "entity" structure to 
 * deal with players as well. - 11/89
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
		levl[x][y].seen = 0;            /* force prl */
		prl(x, y);
	} else {
		initsym(x,y);
		levl[x][y].seen = 0;
	}
}

/* 
 * We want to know whether a wall (or a door) is the portcullis (passageway)
 * of an eventual drawbridge.
 *
 * Return value:  the direction of the drawbridge.
 */

int
is_drawbridge_wall(x,y)
int x,y;
{
	struct rm *lev;

	lev = &levl[x][y];
	if (lev->typ != DOOR && !(lev->diggable & W_GATEWAY))
		return (-1);
	switch (lev->typ) {
	case DOOR:
	case VWALL:
		if (IS_DRAWBRIDGE(levl[x+1][y].typ) &&
	 	    (levl[x+1][y].drawbridgemask & DB_DIR) == DB_WEST)
			return (DB_WEST);
		if (IS_DRAWBRIDGE(levl[x-1][y].typ) && 
		    (levl[x-1][y].drawbridgemask & DB_DIR) == DB_EAST)
			return (DB_EAST);
		if (lev->typ == VWALL) break;
	case HWALL:
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
 * drawbridge "wall" is UP in the location x, y
 * (instead of UP or DOWN, as with is_drawbridge_wall). 
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
 *     dir is the direction.
 *     flag must be put to TRUE if we want the drawbridge to be opened.
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
	if (flag) {             /* We want the bridge open */
		levl[x][y].typ = DRAWBRIDGE_DOWN;
		levl[x2][y2].typ = DOOR;
		levl[x2][y2].doormask = D_NODOOR;
	} else {
		levl[x][y].typ = DRAWBRIDGE_UP;
		levl[x2][y2].typ = wall;
		/* Beware, drawbridges are non-diggable. */
		levl[x2][y2].diggable = (W_NONDIGGABLE | W_GATEWAY);
	}
	levl[x][y].drawbridgemask = dir;        /* always have DB_MOAT */
	initsym(x,y);
	initsym(x2,y2);
	return(TRUE);           
}

struct entity {
	struct monst *emon;	   /* youmonst for the player */
	struct permonst *edata;    /* must be non-zero for record to be valid */
	int ex, ey;
};

#define ENTITIES 2

static struct entity occupants[ENTITIES];

static
struct entity *
e_at(x, y)
int x, y;
{
	int entitycnt;
	
	for (entitycnt = 0; entitycnt < ENTITIES; entitycnt++)
		if ((occupants[entitycnt].edata) && 
		    (occupants[entitycnt].ex == x) &&
		    (occupants[entitycnt].ey == y))
			break;
#ifdef D_DEBUG
	pline("entitycnt = %d", entitycnt);
	fflush(stdout);
#endif
	return((entitycnt == ENTITIES)? 
	       (struct entity *)0 : &(occupants[entitycnt]));
}

static void
m_to_e(mtmp, etmp)
struct monst *mtmp;
struct entity *etmp;
{
	etmp->emon = mtmp;
	if (mtmp) {
		etmp->ex = mtmp->mx;
		etmp->ey = mtmp->my;
		etmp->edata = mtmp->data;
	} else
		etmp->edata = (struct permonst *)0;
}

static void
u_to_e(etmp)
struct entity *etmp;
{
	etmp->emon = &youmonst;
	etmp->ex = u.ux;
	etmp->ey = u.uy;
	etmp->edata = uasmon;
}

static void
set_entity(x, y, etmp)
int x, y;
struct entity *etmp;
{
	if ((x == u.ux) && (y == u.uy))
		u_to_e(etmp);
	else
		if (MON_AT(x, y))
			m_to_e(m_at(x, y), etmp);
		else
			etmp->edata = (struct permonst *)0;
}

#ifdef POLYSELF
#define is_u(etmp) (etmp->emon == &youmonst)
#else
#define is_u(x) FALSE
#endif

/* 
 * WARNING! THE FOLLOWING IS ONLY USEFUL FOR CANSEEMON, OR OTHER FUNCS WHICH 
 * ALWAYS RETURN TRUE FOR U.
 */

#define e_boolean(etmp, func) (is_u(etmp)? (boolean)TRUE : func(etmp->emon)) 

/*
 * e_strg is a utility routine which is not actually in use anywhere, since 
 * the specialized routines below suffice for all current purposes. 
 */

/* #define e_strg(etmp, func) (is_u(etmp)? (char *)0 : func(etmp->emon)) */

static char *
e_nam(etmp)
struct entity *etmp;
{
	return(is_u(etmp)? "you" : mon_nam(etmp->emon));
}

/*
 * Enam is another unused utility routine:  E_phrase is preferable.
 */

/*
static char *
Enam(etmp)
struct entity *etmp;
{
	return(is_u(etmp)? "You" : Monnam(etmp->emon));
}
*/

/*
 * Generates capitalized entity name, makes 2nd -> 3rd person conversion on 
 * verb, where necessary.
 */

static char *
E_phrase(etmp, verb)
struct entity *etmp;
char *verb;
{
	char wholebuf[80], verbbuf[30];

	if (is_u(etmp)) 
		Strcpy(wholebuf, "You");
	else
		Strcpy(wholebuf, Monnam(etmp->emon));
	if (!*verb)
		return(wholebuf);
	Strcat(wholebuf, " ");
	verbbuf[0] = '\0';
	if (is_u(etmp)) 
		Strcpy(verbbuf, verb);
	else {
		if (!strcmp(verb, "are"))
			Strcpy(verbbuf, "is");
		if (!strcmp(verb, "have"))
			Strcpy(verbbuf, "has");
		if (!verbbuf[0]) {
			Strcpy(verbbuf, verb);
			switch (verbbuf[strlen(verbbuf) - 1]) {
				case 'y':
					verbbuf[strlen(verbbuf) - 1] = '\0';
					Strcat(verbbuf, "ies");
					break;
				case 'h':
				case 'o':
				case 's':
					Strcat(verbbuf, "es");
					break;
				default:
					Strcat(verbbuf, "s");
					break;
			}
		}
	}
	Strcat(wholebuf, verbbuf);
	return(wholebuf);
}

/*
 * Simple-minded "can it be here?" routine
 */

static boolean
e_survives_at(etmp, x, y)
struct entity *etmp;
int x, y;
{
	if (noncorporeal(etmp->edata))
		return(TRUE);
	if (is_pool(x, y))
		return((is_u(etmp) && (Wwalking || Levitation)) ||
		       is_swimmer(etmp->edata) || is_flyer(etmp->edata) ||
		       is_floater(etmp->edata));
	if (is_db_wall(x, y))
		return(passes_walls(etmp->edata));
	return(TRUE);
}

static void
e_died(etmp, dest, how)
struct entity *etmp;
int dest, how;
{
	if (is_u(etmp)) {
		if (how == DROWNING)
			drown();
		else {
			coord xy;

			done(how);
			/* So, you didn't die */
			if (!e_survives_at(etmp, etmp->ex, etmp->ey)) {
				pline("A %s force teleports you away...",
		      		      Hallucination ? "normal" : "strange");
				enexto(&xy, etmp->ex, etmp->ey, etmp->edata);
				teleds(xy.x, xy.y);
			}
		}
	} else {
		xkilled(etmp->emon, dest);
		etmp->edata = (struct permonst *)0;	
	}
}


/*
 * These are never directly affected by a bridge or portcullis.
 */

static boolean
automiss(etmp)
struct entity *etmp;
{
	return(passes_walls(etmp->edata) || noncorporeal(etmp->edata));
}

/*
 * Does falling drawbridge or portcullis miss etmp?
 */

static boolean
e_missed(etmp, chunks)
struct entity *etmp;
boolean chunks;
{
	int misses;

#ifdef D_DEBUG
	if (chunks)
		pline("Do chunks miss?");
#endif
	if (automiss(etmp))
		return(TRUE);	

	if (is_flyer(etmp->edata) && 
	    (is_u(etmp)? !Sleeping : 
	     (!etmp->emon->mfroz && !etmp->emon->msleep)))
						  /* flying requires mobility */
		misses = 5;	/* out of 8 */	
	else
		if (is_floater(etmp->edata) ||
		    (is_u(etmp) && Levitation))	  /* doesn't require mobility */
			misses = 3;
		else
			if (chunks && is_pool(etmp->ex, etmp->ey))
				misses = 2; 		     /* sitting ducks */
			else
				misses = 0;	  

	if (is_db_wall(etmp->ex, etmp->ey))
		misses -= 3;				     /* less airspace */

#ifdef D_DEBUG
	pline("Miss chance = %d (out of 8)", misses);
#endif

	return((misses >= rnd(8))? TRUE : FALSE);
}

/*
 * Can etmp jump from death?
 */ 

static boolean
e_jumps(etmp)
struct entity *etmp;
{
	int tmp = 4; 		/* out of 10 */

	if (is_u(etmp)? (Sleeping || Fumbling) : 
		        (etmp->emon->mfroz || etmp->emon->msleep || 
			 !etmp->edata->mmove))
		return(FALSE);

	if (is_u(etmp)? Confusion : etmp->emon->mconf)
		tmp -= 2;

	if (is_u(etmp)? Stunned : etmp->emon->mstun)
		tmp -= 3;

	if (is_db_wall(etmp->ex, etmp->ey))
		tmp -= 2;			     /* less room to maneuver */
	
#ifdef D_DEBUG
	pline("%s to jump (%d chances in 10)", E_phrase(etmp, "try"), tmp);
#endif
	return((tmp >= rnd(10))? TRUE : FALSE);
}

static void
do_entity(etmp)
struct entity *etmp;
{
	int newx, newy, at_portcullis, oldx, oldy;
	boolean must_jump = FALSE, relocates = FALSE, e_inview;
	struct rm *crm;

	if (!etmp->edata)
		return;

	e_inview = e_boolean(etmp, canseemon);

	oldx = etmp->ex;
	oldy = etmp->ey;

	at_portcullis = is_db_wall(oldx, oldy);

	crm = &levl[oldx][oldy];

	if (automiss(etmp) && e_survives_at(etmp, oldx, oldy)) {
		char edifice[20];

		if (e_inview) {
			*edifice = '\0';
			if ((crm->typ == DRAWBRIDGE_DOWN) ||
		    	    (crm->typ == DRAWBRIDGE_UP))
				Strcpy(edifice, "drawbridge");
			else
     				if (at_portcullis) 
					Strcpy(edifice, "portcullis");
			if (*edifice)
				pline("The %s passes through %s!", edifice, 
			      	      e_nam(etmp));			
		}
		return;
	}
	if (e_missed(etmp, FALSE)) { 
		if (at_portcullis)
			pline("The portcullis misses %s!",
			      e_nam(etmp));
#ifdef D_DEBUG
		else
			pline("The drawbridge misses %s!", 
			      e_nam(etmp));
#endif
		if (e_survives_at(etmp, oldx, oldy)) 
			return;
		else {
#ifdef D_DEBUG
			pline("Mon can't survive here");
#endif
			if (at_portcullis)
				must_jump = TRUE;
			else
				relocates = TRUE;  /* just ride drawbridge in */
		}
	} else {
		if (crm->typ == DRAWBRIDGE_DOWN) {
			pline("%s crushed underneath the drawbridge.",
		      	      E_phrase(etmp, "are"));	   	   /* no jump */
			e_died(etmp, e_inview? 2 : 3, CRUSHING); /* no corpse */
			return;   /* Note: Beyond this point, we know we're   */
		}                 /* not at an opened drawbridge, since all   */
		must_jump = TRUE; /* *missable* creatures survive on the      */
	}			  /* square, and all the unmissed ones die.   */
	if (must_jump) 
		if (at_portcullis) {
			if (e_jumps(etmp)) {
				relocates = TRUE;
#ifdef D_DEBUG
				pline("Jump succeeds!");
#endif
			} else {
				if (e_inview)
			       pline("%s crushed by the falling portcullis!",
	      	      		     E_phrase(etmp, "are"));
				else
					if (flags.soundok)
						You("hear a crushing sound.");
				e_died(etmp, e_inview? 1 : 0, CRUSHING);
								    /* corpse */
				return;
			}
		} else {       /* tries to jump off bridge to original square */
			relocates = !e_jumps(etmp); 
#ifdef D_DEBUG
			pline("Jump %s!", (relocates)? "fails" : "succeeds");
#endif
		}

/*
 * Here's where we try to do relocation.  Assumes that etmp is not arriving
 * at the portcullis square while the drawbridge is falling, since this square
 * would be inaccessible (i.e. etmp started on drawbridge square) or 
 * unnecessary (i.e. etmp started here) in such a situation.
 */
#ifdef D_DEBUG
	pline("Doing relocation");
#endif
	newx = oldx;
	newy = oldy;
	(void)find_drawbridge(&newx, &newy);
	if ((newx == oldx) && (newy == oldy))
		get_wall_for_db(&newx, &newy);
#ifdef D_DEBUG
	pline("Checking new square for occupancy");
#endif
	if (relocates && (e_at(newx, newy))) { 

/* 
 * Standoff problem:  one or both entities must die, and/or both switch 
 * places.  Avoid infinite recursion by checking first whether the other 
 * entity is staying put.  Clean up if we happen to move/die in recursion.
 */
		struct entity *other;

		other = e_at(newx, newy);
#ifdef D_DEBUG
		pline("New square is occupied by %s", e_nam(other));
#endif
		if (e_survives_at(other, newx, newy) && automiss(other)) {
			relocates = FALSE;     	       /* "other" won't budge */
#ifdef D_DEBUG
			pline("%s suicide.", E_phrase(etmp, "commit"));
#endif
		} else {

#ifdef D_DEBUG
			pline("Handling %s", e_nam(other));
#endif
			while ((e_at(newx, newy)) && 
			       (e_at(newx, newy) != etmp))
		       		do_entity(other);
#ifdef D_DEBUG
			pline("Checking existence of %s", 
			      e_nam(etmp));
			fflush(stdout);
#endif
			if (e_at(oldx, oldy) != etmp) {
#ifdef D_DEBUG
			        pline("%s moved or died in recursion somewhere",
				      E_phrase(etmp, "have"));
				fflush(stdout);
#endif
				return;
			}
		}
	}
	if (relocates) {
#ifdef D_DEBUG
		pline("Moving %s", e_nam(etmp));
#endif
		if (!is_u(etmp)) {
			remove_monster(etmp->ex, etmp->ey);
			place_monster(etmp->emon, newx, newy);
		} else {
			u.ux = newx;
			u.uy = newy;
		}
		etmp->ex = newx;
		etmp->ey = newy;
		e_inview = e_boolean(etmp, canseemon);
	}
#ifdef D_DEBUG
	pline("Final disposition of %s", e_nam(etmp));
	fflush(stdout);
#endif
	if (is_db_wall(etmp->ex, etmp->ey)) {
#ifdef D_DEBUG
		pline("%s in portcullis chamber", E_phrase(etmp, "are"));
		fflush(stdout);
#endif
		if (e_inview)
			if (is_u(etmp)) {
				You("tumble towards the closed portcullis!"); 
				if (automiss(etmp))
					You("pass through it!");
				else
					pline("The drawbridge closes in...");
			} else
				pline("%s behind the drawbridge.",
		      	      	      E_phrase(etmp, "disappear"));
		if (!e_survives_at(etmp, etmp->ex, etmp->ey)) {
			killer = "closing drawbridge";
			e_died(etmp, 0, CRUSHING); 		/* no message */
			return;
		}
#ifdef D_DEBUG
		pline("%s in here", E_phrase(etmp, "survive"));
#endif
	} else {
#ifdef D_DEBUG
		pline("%s on drawbridge square", E_phrase(etmp, "are"));
#endif
		if (is_pool(etmp->ex, etmp->ey) && !e_inview)
			if (flags.soundok)
				You("hear a splash.");
		if (e_survives_at(etmp, etmp->ex, etmp->ey)) {
			if (e_inview && !is_flyer(etmp->edata) &&
			    !is_floater(etmp->edata))
				pline("%s from the bridge.",
		      	      	      E_phrase(etmp, "fall"));	
			return;	
		}
#ifdef D_DEBUG
		pline("%s cannot survive on the drawbridge square", Enam(etmp));
#endif
		if (is_pool(etmp->ex, etmp->ey))
			if (e_inview && 
			    !is_u(etmp))  /* drown() will supply msgs if nec. */
				if (Hallucination)
				      pline("%s the moat and disappears.",
					    E_phrase(etmp, "drink"));
				else
				      pline("%s into the moat.",
			      	            E_phrase(etmp, "fall"));
		killer = "fall from a drawbridge";
		e_died(etmp, e_inview? 1 : 0,        /* CRUSHING is arbitrary */
		       (is_pool(etmp->ex, etmp->ey))? DROWNING : CRUSHING);
		       						    /* corpse */
		return;
	}
}

/*
 * Close the drawbridge located at x,y
 */

void
close_drawbridge(x,y)
int x,y;
{
	register struct rm *lev1, *lev2;
	struct obj *otmp, *otmp2;
	int x2, y2;

	lev1 = &levl[x][y];
	if (lev1->typ != DRAWBRIDGE_DOWN) return;
	x2 = x; y2 = y;
	get_wall_for_db(&x2,&y2);
	if (cansee(x,y))   /* change msgs if you are a w-walker at portcullis */
		You("see a drawbridge %s up!", 
		    ((u.ux == x2) && (u.uy == y2))? "coming" : "going");
	lev1->typ = DRAWBRIDGE_UP;
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
	set_entity(x, y, &(occupants[0]));
	set_entity(x2, y2, &(occupants[1]));
	do_entity(&(occupants[0]));
	do_entity(&(occupants[1]));
	redosym(x, y);
	for (otmp = level.objects[x][y]; otmp; otmp = otmp2) {
		otmp2 = otmp->nexthere;
		delobj(otmp);
	}
	for (otmp = level.objects[x2][y2]; otmp; otmp = otmp2) {
		otmp2 = otmp->nexthere;
		delobj(otmp);
	}
	redosym(x2, y2);
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
	x2 = x; y2 = y;
	get_wall_for_db(&x2,&y2);
	if (cansee(x,y))   /* change msgs if you are a w-walker at portcullis */
		You("see a drawbridge %s down!",
		    ((x2 == u.ux) && (y2 == u.uy))? "going" : "coming");
	lev1->typ = DRAWBRIDGE_DOWN;
	lev2 = &levl[x2][y2];
	lev2->typ = DOOR;
	lev2->doormask = D_NODOOR;
	set_entity(x, y, &(occupants[0]));
	set_entity(x2, y2, &(occupants[1]));
	do_entity(&(occupants[0]));
	do_entity(&(occupants[1]));
	redosym(x, y);
	redosym(x2, y2);
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
	boolean e_inview;
	struct entity *etmp1 = &(occupants[0]), *etmp2 = &(occupants[1]);

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
				You("hear a loud *SPLASH*!");
		} else {
			if (cansee(x,y))
			    pline("The drawbridge collapses into the moat!");
			else if (flags.soundok)
				You("hear a loud *SPLASH*!");
		}
		lev1->typ = MOAT;
		lev1->drawbridgemask = 0;
	} else {
		if (cansee(x,y))
			pline("The drawbridge disintegrates!");
		else
			You("hear a loud *CRASH*!");
		lev1->typ = ROOM;
		lev1->icedpool =
			((lev1->drawbridgemask & DB_ICE) ? ICED_MOAT : 0);
	}
	set_entity(x2, y2, etmp2); /* currently, only automissers can be here */
	if (etmp2->edata) {
		e_inview = e_boolean(etmp2, canseemon);
		if (!automiss(etmp2)) {			   /* i.e. no-one yet */
			if (e_inview)
				pline("%s blown apart by flying debris",
			      	      E_phrase(etmp2, "are"));
			killer = "exploding drawbridge";
			e_died(etmp2, e_inview? 2 : 3, CRUSHING);/* no corpse */
		}	      /* nothing which is vulnerable can survive this */
	}
	lev2->typ = DOOR;
	lev2->doormask = D_NODOOR;
	set_entity(x, y, etmp1);
	e_inview = e_boolean(etmp1, canseemon);
	if (etmp1->edata) {
		if (e_missed(etmp1, TRUE)) {
#ifdef D_DEBUG
			pline("%s spared!", E_phrase(etmp1, "are"));
#endif
		} else {
			if (e_inview) 
				if (!is_u(etmp1) && Hallucination)
					pline("%s into some heavy metal",
					      E_phrase(etmp1, "get"));
				else
				    pline("%s hit by a huge chunk of metal!",
			      	          E_phrase(etmp1, "are"));
			else 
				if (flags.soundok && !is_u(etmp1) && 
				    !is_pool(x, y))
					You("hear a crushing sound");
#ifdef D_DEBUG
				else
					pline("%s from shrapnel", 
					      E_phrase(etmp1, "die"));
#endif
			killer = "collapsing drawbridge";
			e_died(etmp1, e_inview? 0 : 1, CRUSHING);   /* corpse */
		}
	}
	redosym(x,y);
	redosym(x2,y2);
}

#endif /* STRONGHOLD /**/
