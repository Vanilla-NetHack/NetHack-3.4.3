/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#ifdef MKLEV
#include	"mklev.h"
extern char *fut_geno;
#else MKLEV
#include	"hack.h"
extern char fut_geno[];
#endif MKLEV


extern char *index();

struct monst zeromonst;

/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: call mnexto (not in MKLEV)
 *
 *	In case we make an Orc or killer bee, we make an entire horde (swarm);
 *	note that in this case we return only one of them (the one at [x,y]).
 */
struct monst *
makemon(ptr,x,y)
register struct permonst *ptr;
{
	register struct monst *mtmp;
	register tmp, ct;
	boolean anything = (!ptr);

	if(x != 0 || y != 0) if(m_at(x,y)) return((struct monst *) 0);
	if(ptr){
		if(index(fut_geno, ptr->mlet)) return((struct monst *) 0);
	} else {
		ct = CMNUM - strlen(fut_geno);
		if(index(fut_geno, 'm')) ct++;  /* make only 1 minotaur */
		if(index(fut_geno, '@')) ct++;
		if(ct <= 0) return(0); 		  /* no more monsters! */
		tmp = rn2(ct*dlevel/24 + 7);
		if(tmp < dlevel - 4) tmp = rn2(ct*dlevel/24 + 12);
		if(tmp >= ct) tmp = rn1(ct - ct/2, ct/2);
		for(ct = 0; ct < CMNUM; ct++){
			ptr = &mons[ct];
			if(index(fut_geno, ptr->mlet))
				continue;
			if(!tmp--) goto gotmon;
		}
 panic("makemon?");
	}
gotmon:
	mtmp = newmonst(ptr->pxlth);
	*mtmp = zeromonst;	/* clear all entries in structure */
	for(ct = 0; ct < ptr->pxlth; ct++)
		((char *) &(mtmp->mextra[0]))[ct] = 0;
	mtmp->nmon = fmon;
	fmon = mtmp;
#ifndef MKLEV
	mtmp->m_id = flags.ident++;
#endif MKLEV
	mtmp->data = ptr;
	mtmp->mxlth = ptr->pxlth;
	if(ptr->mlet == 'D') mtmp->orig_hp = mtmp->mhp = 80;
	else if(!ptr->mlevel) mtmp->orig_hp = mtmp->mhp = rnd(4);
	else mtmp->orig_hp = mtmp->mhp = d(ptr->mlevel, 8);
	mtmp->mx = x;
	mtmp->my = y;
	mtmp->mcansee = 1;
	if(ptr->mlet == 'M')
		mtmp->mimic = ']';
#ifndef MKLEV
	if(x == u.ux && y == u.uy)
		mnexto(mtmp);
	if(x == 0 && y == 0)
		rloc(mtmp);
#endif MKLEV
	if(ptr->mlet == 's' || ptr->mlet == 'S') {
		mtmp->mhide = mtmp->mundetected = 1;
#ifdef MKLEV
		if(mtmp->mx && mtmp->my)
			mkobj_at(0, mtmp->mx, mtmp->my);
#endif MKLEV
	}
	if(ptr->mlet == ':') {
		mtmp->cham = 1;
#ifndef MKLEV
		(void) newcham(mtmp, &mons[dlevel+14+rn2(CMNUM-14-dlevel)]);
#endif MKLEV
	}
	if(ptr->mlet == 'I') mtmp->minvis = 1;
	if(ptr->mlet == 'L' || ptr->mlet == 'N'
#ifdef MKLEV
		|| rn2(5)
#endif MKLEV
	) mtmp->msleep = 1;

#ifndef NOWORM
#ifndef MKLEV
	if(ptr->mlet == 'w' && getwn(mtmp))
		initworm(mtmp);
#endif MKLEV
#endif NOWORM

	if(anything) if(ptr->mlet == 'O' || ptr->mlet == 'k') {
		coord enexto();
		coord mm;
		register int cnt = rnd(10);
		mm.x = x;
		mm.y = y;
		while(cnt--) {
			mm = enexto(mm.x, mm.y);
			(void) makemon(ptr, mm.x, mm.y);
		}
	}

	return(mtmp);
}

coord
enexto(xx,yy)
register xchar xx,yy;
{
	register xchar x,y;
	coord foo[15], *tfoo;
	int range;

	tfoo = foo;
	range = 1;
	do {	/* full kludge action. */
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x, yy-range)) {
				tfoo->x = x;
				tfoo++->y = yy-range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x,yy+range)) {
				tfoo->x = x;
				tfoo++->y = yy+range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx-range,y)) {
				tfoo->x = xx-range;
				tfoo++->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx+range,y)) {
				tfoo->x = xx+range;
				tfoo++->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
 range++;
	} while(tfoo == foo);
foofull:
	return( foo[rn2(tfoo-foo)] );
}

goodpos(x,y)	/* used only in mnexto and rloc */
{
	return(
	! (x < 1 || x > COLNO-2 || y < 1 || y > ROWNO-2 ||
	   m_at(x,y) || levl[x][y].typ < DOOR
#ifndef MKLEV
	   || (x == u.ux && y == u.uy)
	   || sobj_at(ENORMOUS_ROCK, x, y)
#endif MKLEV
	));
}
