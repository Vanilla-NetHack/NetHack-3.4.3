/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.search.c version 1.0.1 - small correction in findit() */

#include "hack.h"
#include "def.trap.h"

extern struct monst *makemon();

findit()	/* returns number of things found */
{
	int num;
	register xchar zx,zy;
	register struct gen *gtmp;
	register struct monst *mtmp;
	xchar lx,hx,ly,hy;

	if(u.uswallow) return(0);
	for(lx = u.ux;(num = levl[lx-1][u.uy].typ) && num != CORR;lx--) ;
	for(hx = u.ux;(num = levl[hx+1][u.uy].typ) && num != CORR;hx++) ;
	for(ly = u.uy;(num = levl[u.ux][ly-1].typ) && num != CORR;ly--) ;
	for(hy = u.uy;(num = levl[u.ux][hy+1].typ) && num != CORR;hy++) ;
	num = 0;
	for(zy = ly;zy <= hy;zy++)
		for(zx = lx;zx <= hx;zx++) {
			if(levl[zx][zy].typ == SDOOR) {
				levl[zx][zy].typ = DOOR;
				atl(zx,zy,'+');
				num++;
			} else if(levl[zx][zy].typ == SCORR) {
				levl[zx][zy].typ = CORR;
				atl(zx,zy,CORR_SYM);
				num++;
			} else if(gtmp = g_at(zx,zy,ftrap)) {
				if(gtmp->gflag == PIERC){
					(void) makemon(PM_PIERC,zx,zy);
					num++;
					deltrap(gtmp);
				} else if(!(gtmp->gflag & SEEN)) {
					gtmp->gflag |= SEEN;
					if(!vism_at(zx,zy)) atl(zx,zy,'^');
					num++;
				}
			} else if(mtmp = m_at(zx,zy)) if(mtmp->mimic){
				seemimic(mtmp);
				num++;
			}
		}
 return(num);
}

dosearch()
{
	register xchar x,y;
	register struct gen *tgen;
	register struct monst *mtmp;

	for(x = u.ux-1; x < u.ux+2; x++)
	for(y = u.uy-1; y < u.uy+2; y++) if(x != u.ux || y != u.uy) {
		if(levl[x][y].typ == SDOOR && !rn2(7)) {
			levl[x][y].typ = DOOR;
			levl[x][y].seen = 0;	/* force prl */
			prl(x,y);
			nomul(0);
		} else if(levl[x][y].typ == SCORR && !rn2(7)) {
			levl[x][y].typ = CORR;
			levl[x][y].seen = 0;	/* force prl */
			prl(x,y);
			nomul(0);
		} else {
			if(mtmp = m_at(x,y)) if(mtmp->mimic){
				seemimic(mtmp);
				pline("You find a mimic.");
				return(1);
			}
			for(tgen = ftrap;tgen;tgen = tgen->ngen)
			if(tgen->gx == x && tgen->gy == y &&
			   !(tgen->gflag & SEEN) && !rn2(8)) {
				nomul(0);
				pline("You find a%s.",
					traps[tgen->gflag & TRAPTYPE]);
				if(tgen->gflag == PIERC) {
					deltrap(tgen);
					(void) makemon(PM_PIERC,x,y);
					return(1);
				}
				tgen->gflag |= SEEN;
				if(!vism_at(x,y)) atl(x,y,'^');
			}
		}
	}
 return(1);
}

/* ARGSUSED */
doidtrap(str) /* register */ char *str; {
register struct gen *tgen;
register int x,y;
	if(!getdir()) return(0);
	x = u.ux + u.dx;
	y = u.uy + u.dy;
	for(tgen = ftrap; tgen; tgen = tgen->ngen)
		if(tgen->gx == x && tgen->gy == y &&
		   (tgen->gflag & SEEN)) {
			pline("That is a%s.", traps[tgen->gflag & TRAPTYPE]);
			return(0);
		}
	pline("I can't see a trap there.");
	return(0);
}

wakeup(mtmp)
register struct monst *mtmp;
{
	mtmp->msleep = 0;
	setmangry(mtmp);
	if(mtmp->mimic) seemimic(mtmp);
}

/* NOTE: we must check if(mtmp->mimic) before calling this routine */
seemimic(mtmp)
register struct monst *mtmp;
{
		mtmp->mimic = 0;
		unpmon(mtmp);
		pmon(mtmp);
}
