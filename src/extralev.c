/*	SCCS Id: @(#)extralev.c	3.0	88/04/11			*/
/*	Copyright 1988, 1989 by Ken Arromdee				*/
/* NetHack may be freely redistributed.  See license for details. */

#define MONATTK_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
#include "hack.h"

#ifdef REINCARNATION

struct rogueroom {
	xchar rlx, rly;
	xchar dx, dy;
	boolean real;
	uchar doortable;
	int nroom; /* Only meaningful for "real" rooms */
};
#define UP 1
#define DOWN 2
#define LEFT 4
#define RIGHT 8

static struct rogueroom NEARDATA r[3][3];
static void FDECL(roguejoin,(int,int,int,int,int));
static void FDECL(roguecorr,(int,int,int));
static void FDECL(miniwalk,(int,int));

static
void
roguejoin(x1,y1,x2,y2, horiz)
int x1,y1,x2,y2;
int horiz;
{
	register int x,y,middle;
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
	if (horiz) {
		middle = x1 + rn2(x2-x1+1);
		for(x=MIN(x1,middle); x<=MAX(x1,middle); x++)
			corr(x, y1);
		for(y=MIN(y1,y2); y<=MAX(y1,y2); y++)
			corr(middle,y);
		for(x=MIN(middle,x2); x<=MAX(middle,x2); x++)
			corr(x, y2);
	} else {
		middle = y1 + rn2(y2-y1+1);
		for(y=MIN(y1,middle); y<=MAX(y1,middle); y++)
			corr(x1, y);
		for(x=MIN(x1,x2); x<=MAX(x1,x2); x++)
			corr(x, middle);
		for(y=MIN(middle,y2); y<=MAX(middle,y2); y++)
			corr(x2,y);
	}
}

static
void
roguecorr(x, y, dir)
int x,y,dir;
{
	register int fromx, fromy, tox, toy;

	if (dir==DOWN) {
		r[x][y].doortable &= ~DOWN;
		if (!r[x][y].real) {
			fromx = r[x][y].rlx; fromy = r[x][y].rly;
			fromx += 1 + 26*x; fromy += 7*y;
		} else {
			fromx = r[x][y].rlx + rn2(r[x][y].dx);
			fromy = r[x][y].rly + r[x][y].dy;
			fromx += 1 + 26*x; fromy += 7*y;
			if (!IS_WALL(levl[fromx][fromy].typ))
				impossible("down: no wall at %d,%d?",fromx,
									fromy);
			dodoor(fromx, fromy, &rooms[r[x][y].nroom]);
			levl[fromx][fromy].doormask = D_NODOOR;
			mnewsym(fromx,fromy);
			fromy++;
		}
		if(y >= 2) {
			impossible("down door from %d,%d going nowhere?",x,y);
			return;
		}
		y++;
		r[x][y].doortable &= ~UP;
		if (!r[x][y].real) {
			tox = r[x][y].rlx; toy = r[x][y].rly;
			tox += 1 + 26*x; toy += 7*y;
		} else {
			tox = r[x][y].rlx + rn2(r[x][y].dx);
			toy = r[x][y].rly - 1;
			tox += 1 + 26*x; toy += 7*y;
			if (!IS_WALL(levl[tox][toy].typ))
				impossible("up: no wall at %d,%d?",tox,toy);
			dodoor(tox, toy, &rooms[r[x][y].nroom]);
			levl[tox][toy].doormask = D_NODOOR;
			mnewsym(tox,toy);
			toy--;
		}
		roguejoin(fromx, fromy, tox, toy, FALSE);
		return;
	} else if (dir == RIGHT) {
		r[x][y].doortable &= ~RIGHT;
		if (!r[x][y].real) {
			fromx = r[x][y].rlx; fromy = r[x][y].rly;
			fromx += 1 + 26*x; fromy += 7*y;
		} else {
			fromx = r[x][y].rlx + r[x][y].dx;
			fromy = r[x][y].rly + rn2(r[x][y].dy);
			fromx += 1 + 26*x; fromy += 7*y;
			if (!IS_WALL(levl[fromx][fromy].typ))
				impossible("down: no wall at %d,%d?",fromx,
									fromy);
			dodoor(fromx, fromy, &rooms[r[x][y].nroom]);
			levl[fromx][fromy].doormask = D_NODOOR;
			mnewsym(fromx,fromy);
			fromx++;
		}
		if(x >= 2) {
			impossible("right door from %d,%d going nowhere?",x,y);
			return;
		}
		x++;
		r[x][y].doortable &= ~LEFT;
		if (!r[x][y].real) {
			tox = r[x][y].rlx; toy = r[x][y].rly;
			tox += 1 + 26*x; toy += 7*y;
		} else {
			tox = r[x][y].rlx - 1;
			toy = r[x][y].rly + rn2(r[x][y].dy);
			tox += 1 + 26*x; toy += 7*y;
			if (!IS_WALL(levl[tox][toy].typ))
				impossible("left: no wall at %d,%d?",tox,toy);
			dodoor(tox, toy, &rooms[r[x][y].nroom]);
			levl[tox][toy].doormask = D_NODOOR;
			mnewsym(tox,toy);
			tox--;
		}
		roguejoin(fromx, fromy, tox, toy, TRUE);
		return;
	} else impossible("corridor in direction %d?",dir);
}
			
/* Modified walkfrom() from mkmaze.c */
static
void
miniwalk(x, y)
int x,y;
{
	register int q, dir;
	int dirs[4];

	while(1) {
		q = 0;
#define doorhere (r[x][y].doortable)
		if (x>0 && (!(doorhere & LEFT)) &&
					(!r[x-1][y].doortable || !rn2(10)))
			dirs[q++] = 0;
		if (x<2 && (!(doorhere & RIGHT)) &&
					(!r[x+1][y].doortable || !rn2(10)))
			dirs[q++] = 1;
		if (y>0 && (!(doorhere & UP)) &&
					(!r[x][y-1].doortable || !rn2(10)))
			dirs[q++] = 2;
		if (y<2 && (!(doorhere & DOWN)) &&
					(!r[x][y+1].doortable || !rn2(10)))
			dirs[q++] = 3;
	/* Rogue levels aren't just 3 by 3 mazes; they have some extra
	 * connections, thus that 1/10 chance
	 */
		if (!q) return;
		dir = dirs[rn2(q)];
		switch(dir) { /* Move in direction */
			case 0: doorhere |= LEFT;
				x--;
				doorhere |= RIGHT;
				break;
			case 1: doorhere |= RIGHT;
				x++;
				doorhere |= LEFT;
				break;
			case 2: doorhere |= UP;
				y--;
				doorhere |= DOWN;
				break;
			case 3: doorhere |= DOWN;
				y++;
				doorhere |= UP;
				break;
		}
		miniwalk(x,y);
	}
}

void
makeroguerooms() {
	register struct mkroom *croom;
	register int x,y;
	int x2, y2;
	/* Rogue levels are structured 3 by 3, with each section containing
	 * a room or an intersection.  The minimum width is 2 each way.
	 * One difference between these and "real" Rogue levels: real Rogue
	 * uses 24 rows and NetHack only 23.  So we cheat a bit by making the
	 * second row of rooms not as deep.
	 *
	 * Each normal space has 6/7 rows and 25 columns in which a room may
	 * actually be placed.  Walls go from rows 0-5/6 and columns 0-24.
	 * Not counting walls, the room may go in
	 * rows 1-5 and columns 1-23 (numbering starting at 0).  A room
	 * coordinate of this type may be converted to a level coordinate
	 * by adding 1+28*x to the column, and 7*y to the row.  (The 1
	 * is because column 0 isn't used [we only use 1-78]).
	 * Room height may be 2-4 (2-5 on last row), length 2-23 (not
	 * counting walls)
	 */
#define here r[x][y]

	nroom = 0;
	for(y=0; y<3; y++) for(x=0; x<3; x++) {
		/* Note: we want to insure at least 1 room.  So, if the
		 * first 8 are all dummies, force the last to be a room.
		 */
		if (!rn2(5) && (nroom || (x<2 && y<2))) {
			/* Arbitrary: dummy rooms may only go where real
			 * ones do.
			 */
			here.real = FALSE;
			here.rlx = rn1(22, 2);
			here.rly = rn1((y==2)?4:3, 2);
		} else {
			here.real = TRUE;
			here.dx = rn1(22, 2); /* 2-23 long, plus walls */
			here.dy = rn1((y==2)?4:3, 2); /* 2-5 high, plus walls */

			/* boundaries of room floor */
			here.rlx = rnd(23 - here.dx + 1);
			here.rly = rnd(((y==2) ? 5 : 4)- here.dy + 1);
			nroom++;
		}
		here.doortable = 0;
	}
	miniwalk(rn2(3), rn2(3));
	nroom = 0;
	for(y=0; y<3; y++) for(x=0; x<3; x++) {
		if (here.real) { /* Make a room */
			r[x][y].nroom = nroom;
			croom = &rooms[nroom];
			/* Illumination.  Strictly speaking, it should be lit
			 * only if above level 10, but since Rogue rooms are
			 * only encountered below level 10...
			 */
			if (!rn2(7)) {
				for(x2 = 1+26*x+here.rlx-1;
				    x2 <= 1+26*x+here.rlx+here.dx; x2++)
				for(y2 = 7*y+here.rly-1;
				    y2 <= 7*y+here.rly+here.dy; y2++)
					levl[x2][y2].lit = 1;
				croom->rlit = 1;
			} else croom->rlit = 0;
			croom->lx = 1 + 26*x + here.rlx;
			croom->ly = 7*y + here.rly;
			croom->hx = 1 + 26*x + here.rlx + here.dx - 1;
			croom->hy = 7*y + here.rly + here.dy - 1;
			/* Walls, doors, and floors. */
#define lowx croom->lx
#define lowy croom->ly
#define hix croom->hx
#define hiy croom->hy
			for(x2 = lowx-1; x2 <= hix+1; x2++)
			    for(y2 = lowy-1; y2 <= hiy+1; y2 += (hiy-lowy+2)) {
				levl[x2][y2].scrsym = HWALL_SYM;
				levl[x2][y2].typ = HWALL;
			}
			for(x2 = lowx-1; x2 <= hix+1; x2 += (hix-lowx+2))
			    for(y2 = lowy; y2 <= hiy; y2++) {
				levl[x2][y2].scrsym = VWALL_SYM;
				levl[x2][y2].typ = VWALL;
			}
			for(x2 = lowx; x2 <= hix; x2++)
			    for(y2 = lowy; y2 <= hiy; y2++) {
				levl[x2][y2].scrsym = ROOM_SYM;
				levl[x2][y2].typ = ROOM;
			}
			levl[lowx-1][lowy-1].typ = TLCORNER;
			levl[hix+1][lowy-1].typ = TRCORNER;
			levl[lowx-1][hiy+1].typ = BLCORNER;
			levl[hix+1][hiy+1].typ = BRCORNER;
			levl[lowx-1][lowy-1].scrsym = TLCORN_SYM;
			levl[hix+1][lowy-1].scrsym = TRCORN_SYM;
			levl[lowx-1][hiy+1].scrsym = BLCORN_SYM;
			levl[hix+1][hiy+1].scrsym = BRCORN_SYM;

			/* Misc. */
			smeq[nroom] = nroom;
			croom->rtype = OROOM;
			croom++;
			croom->hx = -1;
			nroom++;
		}
	}

	/* Now, add connecting corridors. */
	for(y=0; y<3; y++) for(x=0; x<3; x++) {
		if (here.doortable & DOWN)
			roguecorr(x, y, DOWN);
		if (here.doortable & RIGHT)
			roguecorr(x, y, RIGHT);
		if (here.doortable & LEFT)
			impossible ("left end of %d, %d never connected?",x,y);
		if (here.doortable & UP)
			impossible ("up end of %d, %d never connected?",x,y);
	}
}

void
corr(x,y)
int x, y;
{
	if (rn2(50)) {
		levl[x][y].typ = CORR;
		levl[x][y].scrsym = CORR_SYM;
	} else {
		levl[x][y].typ = SCORR;
		levl[x][y].scrsym = ' ';	/* _not_ STONE_SYM */
	}
}

void
makerogueghost()
{
	register struct monst *ghost;
	struct obj *ghostobj;
	struct mkroom *croom;
	int x,y;

	if (!nroom) return; /* Should never happen */
	croom = &rooms[rn2(nroom)];
	x = somex(croom); y = somey(croom);
	if (!(ghost = makemon(&mons[PM_GHOST], x, y)))
		return;
	ghost->msleep = 1;
	Strcpy((char *)ghost->mextra, roguename());

	if (rn2(4)) {
		ghostobj = mksobj_at(FOOD_RATION,x,y);
		ghostobj->quan = rnd(7);
		ghostobj->owt = weight(ghostobj);
	}
	if (rn2(2)) {
		ghostobj = mksobj_at(MACE,x,y);
		ghostobj->spe = rnd(3);
		if (rn2(4)) curse(ghostobj);
	} else {
		ghostobj = mksobj_at(TWO_HANDED_SWORD,x,y);
		ghostobj->spe = rnd(5) - 2;
		if (rn2(4)) curse(ghostobj);
	}
	ghostobj = mksobj_at(BOW,x,y);
	ghostobj->spe = 1;
	if (rn2(4)) curse(ghostobj);

	ghostobj = mksobj_at(ARROW,x,y);
	ghostobj->spe = 0;
	ghostobj->quan = rn1(10,25);
	ghostobj->owt = weight(ghostobj);
	if (rn2(4)) curse(ghostobj);

	if (rn2(2)) {
		ghostobj = mksobj_at(RING_MAIL,x,y);
		ghostobj->spe = rn2(3);
		if (!rn2(3)) ghostobj->rustfree = 1;
		if (rn2(4)) curse(ghostobj);
	} else {
		ghostobj = mksobj_at(PLATE_MAIL,x,y);
		ghostobj->spe = rnd(5) - 2;
		if (!rn2(3)) ghostobj->rustfree = 1;
		if (rn2(4)) curse(ghostobj);
	}
	if (rn2(2)) {
		ghostobj = mksobj_at(AMULET_OF_YENDOR,x,y);
		ghostobj->spe = -1;
		ghostobj->known = TRUE;
	}
}
#endif /* REINCARNATION /**/
