/*	SCCS Id: @(#)makemon.c	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	"hack.h"
extern char fut_geno[];
extern char *index();
extern struct obj *mkobj_at(), *mksobj(), *mkobj();
struct monst zeromonst;
extern boolean in_mklev;

#ifdef HARD		/* used in hell for bigger, badder demons! */

struct permonst d_lord   = { "demon lord",	'&',12,13,-5,50,1,5,0 },
		d_prince = { "demon prince",	'&',14,14,-6,70,1,6,0 };
#endif
#ifdef KJSMODS
# ifdef KOPS
struct permonst kobold = { "kobold",'K',1,6,7,0,1,4,0 };
# endif
# ifdef ROCKMOLE
struct permonst giant_rat = { "giant rat",'r',0,12,7,0,1,3,0 };
# endif
#endif /* KJSMODS /**/

struct permonst grey_dragon   = { "grey dragon",  'D',10,9,-1,20,3,8,0 };
struct permonst red_dragon    = { "red dragon",   'D',10,9,-1,20,3,8,0 };
struct permonst orange_dragon = { "orange dragon",'D',10,9,-1,20,3,8,0 };
struct permonst white_dragon  = { "white dragon", 'D',10,9,-1,20,3,8,0 };
struct permonst black_dragon  = { "black dragon", 'D',10,9,-1,20,3,8,0 };
struct permonst blue_dragon   = { "blue dragon",  'D',10,9,-1,20,3,8,0 };
struct permonst green_dragon  = { "green dragon", 'D',10,9,-1,20,3,8,0 };
struct permonst yellow_dragon = { "yellow dragon",'D',10,9,-1,20,3,8,0 };
extern struct permonst pm_gremlin;

/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: call mnexto (if !in_mklev)
 *
 *	In case we make an Orc or killer bee, we make an entire horde
 *	(swarm); note that in this case we return only one of them
 *	(the one at [x,y]).
 */
struct monst *
makemon(ptr,x,y)
register struct permonst *ptr;
{
	register struct monst *mtmp;
	register nleft, deep, ct;
	boolean anything = (!ptr);
	int zlevel = dlevel;
#ifdef BVH
	if(has_amulet()) zlevel = MAXLEVEL;
#endif
	/* if a monster already exists at the position, return */
	if(x != 0 || y != 0) if(m_at(x,y)) return((struct monst *) 0);
	if(ptr){
		/* if you are to make a specific monster and it has 
		   already been genocided, return */
		if(index(fut_geno, ptr->mlet)) return((struct monst *) 0);
	} else {
		/* make a random (common) monster. */
		nleft = CMNUM - strlen(fut_geno);
		if(index(fut_geno, 'm')) nleft++;  /* only 1 minotaur */
		if(index(fut_geno, '@')) nleft++;
		if(nleft <= 0)
		    return((struct monst *) 0);	/* no more monsters! */

		/* determine the strongest monster to make. */
#ifdef ROCKMOLE
		deep = rn2(nleft*zlevel/24 + 6);
#else
		deep = rn2(nleft*zlevel/24 + 7);
#endif
		if(deep < zlevel - 4) deep = rn2(nleft*zlevel/24 + 12);
		/* if deep is greater than the number of monsters left 
		   to create, set deep to a random number between half 
		   the number left and the number left. */
		if(deep >= nleft) deep = rn1(nleft - nleft/2, nleft/2);

		for(ct = 0 ; ct < CMNUM ; ct++){
			ptr = &mons[ct];
			if(index(fut_geno, ptr->mlet)) continue;
#ifdef KOPS
			if(ptr->mlet == 'K') {
# ifdef KJSMODS
				/* since this is a random monster, make 
				   a Kobold instead of a Kop. */
				ptr = &kobold;
# else
				deep--;
# endif
				continue;
			}
#endif /* KOPS /**/
			if(deep-- <= 0) goto gotmon;
		}
		/* this can happen if you are deep in the dungeon and 
		   mostly weak monsters have been genocided. */
		return((struct monst *) 0);
	}
gotmon:
#if defined(KJSMODS) && defined(ROCKMOLE)
	/* make a giant rat */
	if((zlevel < 4 && ptr->mlet == 'r')
	   || (zlevel == 1 && (ptr->mlet == 'h' || ptr->mlet == 'i'))
	   || (zlevel == 2 && (ptr->mlet == 'o' || ptr->mlet == 'y'))
	) ptr = &giant_rat;
#endif
	mtmp = newmonst(ptr->pxlth);
	*mtmp = zeromonst;	/* clear all entries in structure */
	for(ct = 0; ct < ptr->pxlth; ct++)
		((char *) &(mtmp->mextra[0]))[ct] = 0;
	mtmp->nmon = fmon;
	fmon = mtmp;
	mtmp->m_id = flags.ident++;
	mtmp->data = ptr;
	mtmp->mxlth = ptr->pxlth;
	if(ptr->mlet == 'D') mtmp->mhpmax = mtmp->mhp = 80;
	else if(!ptr->mlevel) mtmp->mhpmax = mtmp->mhp = rnd(4);
	else mtmp->mhpmax = mtmp->mhp = d(ptr->mlevel, 8);
	mtmp->mx = x;
	mtmp->my = y;
	mtmp->mcansee = 1;
	if(ptr->mlet == 'D') {
		mtmp->dragon = rn2(8);
		switch(mtmp->dragon) {
			case 0:	mtmp->data = &grey_dragon;	break;
			case 1:	mtmp->data = &red_dragon;	break;
			case 2:	mtmp->data = &orange_dragon;	break;
			case 3:	mtmp->data = &white_dragon;	break;
			case 4:	mtmp->data = &black_dragon;	break;
			case 5:	mtmp->data = &blue_dragon;	break;
			case 6:	mtmp->data = &green_dragon;	break;
			case 7:	mtmp->data = &yellow_dragon;	break;
		}
	}
	/* if gnome, make a gremlin or if gremlin make sure it stays gremlin */
	if((ptr->mlet == 'G' && zlevel >= 10 && rn2(4)) || 
		!strcmp(ptr->mname, "gremlin")) {
		ptr = PM_GREMLIN;
		mtmp->data = PM_GREMLIN;
		mtmp->isgremlin = 1;
	}
	if(ptr->mlet == 'M'){
		mtmp->mimic = 1;
		mtmp->mappearance = ']';
	}
	if(!in_mklev) {
		if(x == u.ux && y == u.uy && ptr->mlet != ' ')
			mnexto(mtmp);
		if(x == 0 && y == 0)
			rloc(mtmp);
	}
	if(ptr->mlet == 's' || ptr->mlet == 'S') {
		mtmp->mhide = mtmp->mundetected = 1;
		if(in_mklev)
		if(mtmp->mx && mtmp->my)
			(void) mkobj_at(0, mtmp->mx, mtmp->my);
	}
	if(ptr->mlet == ':') {
#ifdef DGKMOD
		/* If you're protected with a ring, don't create
		 * any shape-changing chameleons -dgk
		 */
		if (Protection_from_shape_changers)
			mtmp->cham = 0;
		else {
			mtmp->cham = 1;
			(void) newcham(mtmp,
# ifndef RPH
				&mons[zlevel+14+rn2(CMNUM-14-zlevel)]);
# else
				(struct permonst *)0);
# endif
		}
#else
		mtmp->cham = 1;
		(void) newcham (mtmp,
# ifndef RPH
				&mons[zlevel+14+rn2(CMNUM-14-zlevel)]);
# else
				0);
# endif
#endif
	}
	if(ptr->mlet == 'I' || ptr->mlet == ';')
		mtmp->minvis = 1;
	if(ptr->mlet == 'L' || ptr->mlet == 'N'
	    || (in_mklev && index("&w;", ptr->mlet) && rn2(5))
	) mtmp->msleep = 1;
#ifdef HARD
	if(ptr->mlet == '&' && (Inhell || u.udemigod)) {

		if(!rn2(3 + !Inhell + !u.udemigod)) {
		    if (rn2(3 + Inhell)) mtmp->data = &d_lord;
		    else  {
			mtmp->data = &d_prince;
			mtmp->mpeaceful = 1;
			mtmp->minvis = 1;
		    }
		}
#ifdef RPH
		if(uwep)
		    if(!strcmp(ONAME(uwep), "Excalibur"))
			mtmp->mpeaceful = mtmp->mtame = 0;
#endif
	}
#endif /* HARD /**/
#ifndef NOWORM
	if(ptr->mlet == 'w' && getwn(mtmp))  initworm(mtmp);
#endif

	if(anything)
	    if(ptr->mlet == 'O' || ptr->mlet == 'k'
#ifdef SAC
	       || ptr->mlet == '3'
#endif /* SAC /**/
	       || (ptr->mlet == 'G' && mtmp->isgremlin)
				  ) {

		coord mm;
		register int cnt = rnd(10);
		mm.x = x;
		mm.y = y;
		while(cnt--) {
			enexto(&mm, mm.x, mm.y);
			(void) makemon(ptr, mm.x, mm.y);
		}
	}
#ifdef DGKMOD
	m_initinv(mtmp);
#endif
	return(mtmp);
}

#ifdef DGKMOD
/* Give some monsters an initial inventory to use */
m_initinv(mtmp)
struct monst *mtmp;
{
	struct obj *otmp;

	switch (mtmp->data->mlet) {
# ifdef KAA
	case '9':
		if (rn2(2)) {
			otmp = mksobj(ENORMOUS_ROCK);
			mpickobj(mtmp, otmp);
		}
# endif
# ifdef SAC
	case '3':			/* Outfit the troops */
		if (!rn2(5)) {
			otmp = mksobj(HELMET);
			mpickobj(mtmp, otmp); }
		if (!rn2(5)) {
			otmp = mksobj(CHAIN_MAIL);
			mpickobj(mtmp, otmp); }
		if (!rn2(4)) {
			otmp = mksobj(DAGGER);
			mpickobj(mtmp, otmp); }
		if (!rn2(7)) {
			otmp = mksobj(SPEAR);
			mpickobj(mtmp, otmp); }
		if (!rn2(3)) {
			otmp = mksobj(K_RATION);
			mpickobj(mtmp, otmp); }
		if (!rn2(2)) {
			otmp = mksobj(C_RATION);
			mpickobj(mtmp, otmp); }
# endif /* SAC /**/
# ifdef KOPS
	case 'K':		/* create Keystone Kops with cream pies to
				 * throw. As suggested by KAA.	   [MRS]
				 */
		if (!rn2(4)
#  ifdef KJSMODS
  		    && !strcmp(mtmp->data->mname, "Keystone Kop")
#  endif
								) {
			otmp = mksobj(CREAM_PIE);
			otmp->quan = 2 + rnd(2);
			otmp->owt = weight(otmp);
			mpickobj(mtmp, otmp);
		}
		break;
	case 'O':
# else
	case 'K':
# endif
		if (!rn2(4)) {
			otmp = mksobj(DART);
			otmp->quan = 2 + rnd(12);
			otmp->owt = weight(otmp);
			mpickobj(mtmp, otmp);
		}
		break;

	case 'C':
		if (rn2(2)) {
			otmp = mksobj(CROSSBOW);
			otmp->cursed = rn2(2);
			mpickobj(mtmp, otmp);
			otmp = mksobj(CROSSBOW_BOLT);
			otmp->quan = 2 + rnd(12);
			otmp->owt = weight(otmp);
			mpickobj(mtmp, otmp);
		}
		break;
	default:
		break;
	}
}
#endif

enexto(cc, xx,yy)
coord	*cc;
register xchar xx,yy;
{
	register xchar x,y;
	coord foo[15], *tfoo;
	int range, i;

	tfoo = foo;
	range = 1;
	do {	/* full kludge action. */
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x, yy-range)) {
				tfoo->x = x;
				(tfoo++)->y = yy-range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(x = xx-range; x <= xx+range; x++)
			if(goodpos(x,yy+range)) {
				tfoo->x = x;
				(tfoo++)->y = yy+range;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx-range,y)) {
				tfoo->x = xx-range;
				(tfoo++)->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		for(y = yy+1-range; y < yy+range; y++)
			if(goodpos(xx+range,y)) {
				tfoo->x = xx+range;
				(tfoo++)->y = y;
				if(tfoo == &foo[15]) goto foofull;
			}
		range++;
	} while(tfoo == foo);
foofull:
	i = rn2(tfoo - foo);
	cc->x = foo[i].x;
	cc->y = foo[i].y;
	return(0);
}

goodpos(x,y)	/* used only in mnexto and rloc */
{
	return(
	! (x < 1 || x > COLNO-2 || y < 1 || y > ROWNO-2 ||
	   m_at(x,y) || !ACCESSIBLE(levl[x][y].typ)
	   || (x == u.ux && y == u.uy)
	   || sobj_at(ENORMOUS_ROCK, x, y)
	));
}

rloc(mtmp)
struct monst *mtmp;
{
	register tx,ty;
	register char ch = mtmp->data->mlet;

#ifndef NOWORM
	if(ch == 'w' && mtmp->mx) return;	/* do not relocate worms */
#endif
	do {
		tx = rn1(COLNO-3,2);
		ty = rn2(ROWNO);
	} while(!goodpos(tx,ty));
	mtmp->mx = tx;
	mtmp->my = ty;
	if(u.ustuck == mtmp){
		if(u.uswallow) {
			u.ux = tx;
			u.uy = ty;
			docrt();
		} else	u.ustuck = 0;
	}
	pmon(mtmp);
}

struct monst *
mkmon_at(let,x,y)
char let;
register int x,y;
{
	register int ct;
	register struct permonst *ptr;

	for(ct = 0; ct < CMNUM; ct++) {
		ptr = &mons[ct];
		if(ptr->mlet == let)
			return(makemon(ptr,x,y));
	}
	return((struct monst *)0);
}
