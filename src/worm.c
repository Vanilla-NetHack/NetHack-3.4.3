/*	SCCS Id: @(#)worm.c	3.0	88/11/11
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef WORM
#include "wseg.h"

static void FDECL(remseg,(struct monst *,struct wseg *));

/* Each tailed worm has a wormno.  This is used as an index for the following
 * arrays:
 * wsegs: the start of a a linked list of segments, located at the tail.
 * wheads: the end of a linked list of segments, located at the head.  Putting
 *	the tail of the list at the head of the worm and vice versa is an
 *	endless source of confusion, but necessary.  From now on, we will use
 *	"start" and "end" to refer to the list, and "head" and "tail" to refer
 *	to the worm.
 * wgrowtime: obvious.
 *
 * When a worm is moved, we add a new segment at the head (end of the list)
 * and (unless we want it to grow) delete the segment at the tail (beginning
 * of the list).  This new head segment is located in the same square as
 * the actual head of the worm (thus requiring a special case when setting
 * level.monsters upon worm movement).  If we do want to grow the worm, we
 * don't delete the tail segment, and we give the worm extra hit points,
 * which possibly go into its maximum.
 *
 * Non-moving worms (worm_nomove) shrink instead of grow as their tails keep
 * going while their heads are stopped short.  Delete the tail segment,
 * and remove hit points from the worm.
 */
struct wseg *wsegs[32] = DUMMY, *wheads[32] = DUMMY, *m_atseg = 0;
long wgrowtime[32] = DUMMY;

int
getwn(mtmp)
struct monst *mtmp;
{
	register int tmp;

	for(tmp = 1; tmp < 32; tmp++)
	    if(!wsegs[tmp]) {
		mtmp->wormno = tmp;
		return(1);
	    }
	return(0);	/* level infested with worms */
}

/* called to initialize a worm unless cut in half */
void
initworm(mtmp)
struct monst *mtmp;
{
	register struct wseg *wtmp;
	register int tmp = mtmp->wormno;

	if(!tmp) return;
	wheads[tmp] = wsegs[tmp] = wtmp = newseg();
	wgrowtime[tmp] = 0;
	wtmp->wx = mtmp->mx;
	wtmp->wy = mtmp->my;
/*	wtmp->wdispl = 0; */
	wtmp->nseg = 0;
}

static void
remseg(mtmp,wtmp)
struct monst *mtmp;
register struct wseg *wtmp;
{
	if (mtmp->mx != wtmp->wx || mtmp->my != wtmp->wy)
		remove_monster(wtmp->wx, wtmp->wy);
	if(wtmp->wdispl) newsym(wtmp->wx, wtmp->wy);
	free((genericptr_t) wtmp);
}

void
worm_move(mtmp)
struct monst *mtmp;
{
	register struct wseg *wtmp, *whd;
	register int tmp = mtmp->wormno;

	wtmp = newseg();
	wtmp->wx = mtmp->mx;
	wtmp->wy = mtmp->my;
	wtmp->nseg = 0;
/*	wtmp->wdispl = 0; */
	(whd = wheads[tmp])->nseg = wtmp;
	wheads[tmp] = wtmp;
	if(cansee(whd->wx,whd->wy)){
		unpmon(mtmp);
		atl(whd->wx, whd->wy, S_WORM_TAIL);
		whd->wdispl = 1;
	} else	whd->wdispl = 0;
	if(wgrowtime[tmp] <= moves) {
		if(!wgrowtime[tmp]) wgrowtime[tmp] = moves + rnd(5);
		else wgrowtime[tmp] += 2+rnd(15);
		mtmp->mhp += 3;
		if (mtmp->mhp > MHPMAX) mtmp->mhp = MHPMAX;
		if (mtmp->mhp > mtmp->mhpmax) mtmp->mhpmax = mtmp->mhp;
		return;
	}
	whd = wsegs[tmp];
	wsegs[tmp] = whd->nseg;
	remseg(mtmp, whd);
}

void
worm_nomove(mtmp)
register struct monst *mtmp;
{
	register int tmp;
	register struct wseg *wtmp;

	tmp = mtmp->wormno;
	wtmp = wsegs[tmp];
	if(wtmp == wheads[tmp]) return;
	if(wtmp == 0 || wtmp->nseg == 0) panic("worm_nomove?");
	wsegs[tmp] = wtmp->nseg;
	remseg(mtmp, wtmp);
	if (mtmp->mhp > 3) mtmp->mhp -= 3;	/* mhpmax not changed ! */
	else mtmp->mhp = 1;
}

void
wormdead(mtmp)
register struct monst *mtmp;
{
	register int tmp = mtmp->wormno;
	register struct wseg *wtmp, *wtmp2;

	if(!tmp) return;
	mtmp->wormno = 0;
	for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp2) {
		wtmp2 = wtmp->nseg;
		remseg(mtmp, wtmp);
	}
	wsegs[tmp] = 0;
}

void
wormhit(mtmp)
register struct monst *mtmp;
{
	register int tmp = mtmp->wormno;
	register struct wseg *wtmp;

	if(!tmp) return;	/* worm without tail */
	for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp->nseg)
		if (dist(wtmp->wx, wtmp->wy) < 3) (void) mattacku(mtmp);
}

void
wormsee(tmp)
register unsigned int tmp;
{
	register struct wseg *wtmp = wsegs[tmp];

	if(!wtmp) panic("wormsee: wtmp==0");

	for(; wtmp->nseg; wtmp = wtmp->nseg)
		if(!cansee(wtmp->wx,wtmp->wy) && wtmp->wdispl) {
			newsym(wtmp->wx, wtmp->wy);
			wtmp->wdispl = 0;
		}
		else if (cansee(wtmp->wx, wtmp->wy) && !wtmp->wdispl) {
			atl(wtmp->wx, wtmp->wy, S_WORM_TAIL);
			wtmp->wdispl = 1;
		}
}

void
cutworm(mtmp, x, y, weptyp)
struct monst *mtmp;
xchar x,y;
unsigned weptyp;		/* uwep->otyp or 0 */
{
	register struct wseg *wtmp, *wtmp2;
	struct monst *mtmp2;
	int tmp, tmp2;

	if(mtmp->mx == x && mtmp->my == y) return;	/* hit headon */

	/* cutting goes best with axe or sword */
	tmp = rnd(20);
	if(weptyp >= SHORT_SWORD && weptyp <= KATANA ||
	   weptyp == AXE)
		tmp += 5;

	if(tmp < 12) return;

	/* if tail then worm just loses a tail segment */
	tmp = mtmp->wormno;
	wtmp = wsegs[tmp];
	if(wtmp->wx == x && wtmp->wy == y){
		wsegs[tmp] = wtmp->nseg;
		remseg(mtmp, wtmp);
		return;
	}

	/* cut the worm in two halves */
	mtmp2 = newmonst(0);
	*mtmp2 = *mtmp;
	mtmp2->mxlth = mtmp2->mnamelth = 0;

	/* sometimes the tail end dies */
	if(rn2(3) || !getwn(mtmp2)){
		monfree(mtmp2);
		place_worm_seg(mtmp, mtmp2->mx, mtmp2->my);
			/* since mtmp is still on that spot */
		tmp2 = 0;
	} else {
		tmp2 = mtmp2->wormno;
		wsegs[tmp2] = wsegs[tmp];
		wgrowtime[tmp2] = 0;
	}
	/* do-loop: go from the tail to the head.  Segments close to the tail
	 * either die or become part of worm 2.  We stop at the hit segment
	 * and this loop never goes down the entire length of the worm.
	 */
	do {
	    /* The segment immediately next to (tailwards) the one hit, */
	    /* becoes the head of the new second worm.  Note: at this point, */
	    /* wtmp->nseg is the one you hit, wtmp is immediately tailwards, */
	    /* and wtmp->nseg->nseg is immediately headwards. */
	    if(wtmp->nseg->wx == x && wtmp->nseg->wy == y){
		if(tmp2) wheads[tmp2] = wtmp;
		wsegs[tmp] = wtmp->nseg->nseg;
		remseg(mtmp, wtmp->nseg);
		wtmp->nseg = 0;
		if(tmp2) {
		    kludge("You cut %s in half.", mon_nam(mtmp));
		/* devalue the monster level of both halves of the worm */
		    mtmp->m_lev = (mtmp->m_lev <= 3) ? 2 : mtmp->m_lev - 2;
		    mtmp2->m_lev = mtmp->m_lev;
		/* calculate the mhp on the new (lower) monster level */
		    mtmp2->mhpmax = mtmp2->mhp = d((int)mtmp2->m_lev, 8);
		    place_monster(mtmp2, wtmp->wx, wtmp->wy);
		    mtmp2->nmon = fmon;
		    fmon = mtmp2;
		    mtmp2->mdispl = 0;
		    pmon(mtmp2);
		} else {
			if (Blind) You("cut off part of its tail.");
			else You("cut off part of %s's tail.", mon_nam(mtmp));
			remseg(mtmp, wtmp);
		}
		mtmp->mhp /= 2;
		return;
	    }
	/* Worm segments which are closer to the tail than the one you hit, */
	/* get either deleted or transferred from the old to new worms */
	    wtmp2 = wtmp->nseg;
	    if(!tmp2) remseg(mtmp, wtmp);
	    else place_worm_seg(mtmp2, wtmp->wx, wtmp->wy);
	    wtmp = wtmp2;
	} while(wtmp->nseg);
	panic("Cannot find worm segment");
}

#endif /* WORM /**/
