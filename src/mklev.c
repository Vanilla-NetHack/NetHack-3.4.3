/*	SCCS Id: @(#)mklev.c	3.0	88/11/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* for UNIX, Rand #def'd to (long)lrand48() or (long)random() */
/* croom->lx etc are schar (width <= int), so % arith ensures that */
/* conversion of result to int is reasonable */

#ifdef SINKS
static void mksink();
#endif
#ifdef ALTARS
static void mkaltar();
#endif

int
somex(croom)
register struct mkroom *croom;
{
	return rn2(croom->hx-croom->lx+1) + croom->lx;
}

int
somey(croom)
register struct mkroom *croom;
{
	return rn2(croom->hy-croom->ly+1) + croom->ly;
}

#define	XLIM	4	/* define minimum required space around a room */
#define	YLIM	3
boolean secret;		/* TRUE while making a vault: increase [XY]LIM */
struct rm zerorm;
schar nxcor;
boolean goldseen;

/* Definitions used by makerooms() and addrs() */
#define	MAXRS	50	/* max lth of temp rectangle table - arbitrary */
struct rectangle {
	xchar rlx,rly,rhx,rhy;
} rs[MAXRS+1];
int rscnt,rsmax;	/* 0..rscnt-1: currently under consideration */
			/* rscnt..rsmax: discarded */

static void
addrsx(lx,ly,hx,hy,discarded)
register int lx,ly,hx,hy;
boolean discarded;		/* piece of a discarded area */
{
	register struct rectangle *rsp;

	/* check inclusions */
	for(rsp = rs; rsp < &rs[rsmax]; rsp++) {
		if(lx >= rsp->rlx && hx <= rsp->rhx &&
		   ly >= rsp->rly && hy <= rsp->rhy)
			return;
	}

	/* make a new entry */
	if(rsmax >= MAXRS) {
#ifdef WIZARD
		if(wizard) pline("MAXRS may be too small.");
#endif
		return;
	}
	rsmax++;
	if(!discarded) {
		*rsp = rs[rscnt];
		rsp = &rs[rscnt];
		rscnt++;
	}
	rsp->rlx = lx;
	rsp->rly = ly;
	rsp->rhx = hx;
	rsp->rhy = hy;
}

static void
addrs(lowx,lowy,hix,hiy)
register int lowx,lowy,hix,hiy;
{
	register struct rectangle *rsp;
	register int lx,ly,hx,hy,xlim,ylim;
	boolean discarded;

	xlim = XLIM + secret;
	ylim = YLIM + secret;

	/* walk down since rscnt and rsmax change */
	for(rsp = &rs[rsmax-1]; rsp >= rs; rsp--) {

		if((lx = rsp->rlx) > hix || (ly = rsp->rly) > hiy ||
		   (hx = rsp->rhx) < lowx || (hy = rsp->rhy) < lowy)
			continue;
		if((discarded = (rsp >= &rs[rscnt]))) {
			*rsp = rs[--rsmax];
		} else {
			rsmax--;
			rscnt--;
			*rsp = rs[rscnt];
			if(rscnt != rsmax)
				rs[rscnt] = rs[rsmax];
		}
		if(lowy - ly > 2*ylim + 4)
			addrsx(lx,ly,hx,lowy-2,discarded);
		if(lowx - lx > 2*xlim + 4)
			addrsx(lx,ly,lowx-2,hy,discarded);
		if(hy - hiy > 2*ylim + 4)
			addrsx(lx,hiy+2,hx,hy,discarded);
		if(hx - hix > 2*xlim + 4)
			addrsx(hix+2,ly,hx,hy,discarded);
	}
}

static int
comp(x,y)
register struct mkroom *x,*y;
{
	if(x->lx < y->lx) return(-1);
	return(x->lx > y->lx);
}

static void
finddpos(cc, xl,yl,xh,yh)
coord	*cc;
xchar	xl,yl,xh,yh;
{
	register xchar x, y;

	x = (xl == xh) ? xl : (xl + rn2(xh-xl+1));
	y = (yl == yh) ? yl : (yl + rn2(yh-yl+1));
	if(okdoor(x, y))
		goto gotit;

	for(x = xl; x <= xh; x++) for(y = yl; y <= yh; y++)
		if(okdoor(x, y))
			goto gotit;

	for(x = xl; x <= xh; x++) for(y = yl; y <= yh; y++)
		if(IS_DOOR(levl[x][y].typ) || levl[x][y].typ == SDOOR)
			goto gotit;
	/* cannot find something reasonable -- strange */
	x = xl;
	y = yh;
gotit:
	cc->x = x;
	cc->y = y;
	return;
}

/* Only called from makerooms() and makebigroom() */
static int
maker(lowx,ddx,lowy,ddy,lit)
schar lowx,ddx,lowy,ddy;
boolean lit;
{
	register struct mkroom *croom;
	register int x, y, hix = lowx+ddx, hiy = lowy+ddy;
	register int xlim = XLIM + secret, ylim = YLIM + secret;

	if(nroom >= MAXNROFROOMS) return(0);
	if(lowx < XLIM) lowx = XLIM;
	if(lowy < YLIM) lowy = YLIM;
	if(hix > COLNO-XLIM-1) hix = COLNO-XLIM-1;
	if(hiy > ROWNO-YLIM-1) hiy = ROWNO-YLIM-1;
chk:
	if(hix <= lowx || hiy <= lowy) return(0);

	/* check area around room (and make room smaller if necessary) */
	for(x = lowx - xlim; x <= hix + xlim; x++) {
		for(y = lowy - ylim; y <= hiy + ylim; y++) {
			if(levl[x][y].typ) {
#ifdef WIZARD
			    if(wizard && !secret)
				pline("Strange area [%d,%d] in maker().",x,y);
#endif
				if(!rn2(3)) return(0);
				if(x < lowx)
					lowx = x+xlim+1;
				else
					hix = x-xlim-1;
				if(y < lowy)
					lowy = y+ylim+1;
				else
					hiy = y-ylim-1;
				goto chk;
			}
		}
	}

	croom = &rooms[nroom];

	/* on low levels the room is lit (usually) */
	/* secret vaults are always lit */
	/* some other rooms may require lighting */
	if((rnd(dlevel) < 10 && rn2(77)) || secret || lit) {
		for(x = lowx-1; x <= hix+1; x++)
			for(y = lowy-1; y <= hiy+1; y++)
				levl[x][y].lit = 1;
		croom->rlit = 1;
	} else
		croom->rlit = 0;
	croom->lx = lowx;
	croom->hx = hix;
	croom->ly = lowy;
	croom->hy = hiy;
	croom->rtype = OROOM;
	croom->doorct = 0;
	/* if we're not making a vault, doorindex will still be 0
	 * if we are, we'll have problems adding niches to the previous room
	 * unless fdoor is at least doorindex
	 */
	croom->fdoor = doorindex;

	for(x = lowx-1; x <= hix+1; x++)
	    for(y = lowy-1; y <= hiy+1; y += (hiy-lowy+2)) {
		levl[x][y].typ = HWALL;
		levl[x][y].scrsym = HWALL_SYM;
	    }
	for(x = lowx-1; x <= hix+1; x += (hix-lowx+2))
	    for(y = lowy; y <= hiy; y++) {
		levl[x][y].typ = VWALL;
		levl[x][y].scrsym = VWALL_SYM;
	    }
	for(x = lowx; x <= hix; x++)
	    for(y = lowy; y <= hiy; y++) {
		levl[x][y].typ = ROOM;
		levl[x][y].scrsym = ROOM_SYM;
	    }
	levl[lowx-1][lowy-1].typ = TLCORNER;
	levl[hix+1][lowy-1].typ = TRCORNER;
	levl[lowx-1][hiy+1].typ = BLCORNER;
	levl[hix+1][hiy+1].typ = BRCORNER;
	levl[lowx-1][lowy-1].scrsym = TLCORN_SYM;
	levl[hix+1][lowy-1].scrsym = TRCORN_SYM;
	levl[lowx-1][hiy+1].scrsym = BLCORN_SYM;
	levl[hix+1][hiy+1].scrsym = BRCORN_SYM;

	smeq[nroom] = nroom;
	croom++;
	croom->hx = -1;
	nroom++;
	return(1);
}

static int
makerooms() {
register struct rectangle *rsp;
register int lx, ly, hx, hy, lowx, lowy, hix, hiy, dx, dy;
int tryct = 0, xlim, ylim;

	/* init */
	xlim = XLIM + secret;
	ylim = YLIM + secret;
	if(nroom == 0) {
		rsp = rs;
		rsp->rlx = rsp->rly = 0;
		rsp->rhx = COLNO-1;
		rsp->rhy = ROWNO-1;
		rsmax = 1;
	}
	rscnt = rsmax;

	/* make rooms until satisfied */
	while(rscnt > 0 && nroom < MAXNROFROOMS-1) {
		if(!secret && nroom > (MAXNROFROOMS/4) &&
		   !rn2((MAXNROFROOMS-nroom)*(MAXNROFROOMS-nroom)))
			return 0;

		/* pick a rectangle */
		rsp = &rs[rn2(rscnt)];
		hx = rsp->rhx;
		hy = rsp->rhy;
		lx = rsp->rlx;
		ly = rsp->rly;

		/* find size of room */
		if(secret)
			dx = dy = 1;
		else {
			dx = 2 + rn2((hx-lx-8 > 20) ? 12 : 8);
			dy = 2 + rn2(4);
			if(dx*dy > 50)
				dy = 50/dx;
		}

		/* look whether our room will fit */
		if(hx-lx < dx + (dx>>1) + 2*xlim ||
		   hy-ly < dy + dy/3 + 2*ylim) {
					/* no, too small */
					/* maybe we throw this area out */
			if(secret || !rn2(MAXNROFROOMS+1-nroom-tryct)) {
				rscnt--;
				rs[rsmax] = *rsp;
				*rsp = rs[rscnt];
				rs[rscnt] = rs[rsmax];
				tryct = 0;
			} else
				tryct++;
			continue;
		}

		lowx = lx + xlim + rn2(hx - lx - dx - 2*xlim + 1);
		lowy = ly + ylim + rn2(hy - ly - dy - 2*ylim + 1);
		hix = lowx + dx;
		hiy = lowy + dy;

		if(maker(lowx, dx, lowy, dy, FALSE)) {
			if(secret) return(1);
			addrs(lowx-1, lowy-1, hix+1, hiy+1);
			tryct = 0;
		} else
			if(tryct++ > 100)
				break;
	}
	return(0);	/* failed to make vault - very strange */
}

static void
join(a,b)
register int a, b;
{
	coord cc,tt;
	register int tx, ty, xx, yy;
	register struct rm *crm;
	register struct mkroom *croom, *troom;
	register int dx, dy, dix, diy, cct;

	croom = &rooms[a];
	troom = &rooms[b];

	/* find positions cc and tt for doors in croom and troom
	   and direction for a corridor between them */

	if(troom->hx < 0 || croom->hx < 0 || doorindex >= DOORMAX) return;
	if(troom->lx > croom->hx) {
		dx = 1;
		dy = 0;
		xx = croom->hx+1;
		tx = troom->lx-1;
		finddpos(&cc, xx, croom->ly, xx, croom->hy);
		finddpos(&tt, tx, troom->ly, tx, troom->hy);
	} else if(troom->hy < croom->ly) {
		dy = -1;
		dx = 0;
		yy = croom->ly-1;
		finddpos(&cc, croom->lx, yy, croom->hx, yy);
		ty = troom->hy+1;
		finddpos(&tt, troom->lx, ty, troom->hx, ty);
	} else if(troom->hx < croom->lx) {
		dx = -1;
		dy = 0;
		xx = croom->lx-1;
		tx = troom->hx+1;
		finddpos(&cc, xx, croom->ly, xx, croom->hy);
		finddpos(&tt, tx, troom->ly, tx, troom->hy);
	} else {
		dy = 1;
		dx = 0;
		yy = croom->hy+1;
		ty = troom->ly-1;
		finddpos(&cc, croom->lx, yy, croom->hx, yy);
		finddpos(&tt, troom->lx, ty, troom->hx, ty);
	}
	xx = cc.x;
	yy = cc.y;
	tx = tt.x - dx;
	ty = tt.y - dy;
	if(nxcor && levl[xx+dx][yy+dy].typ)
		return;
	dodoor(xx,yy,croom);

	cct = 0;
	while(xx != tx || yy != ty) {
	    xx += dx;
	    yy += dy;

	    /* loop: dig corridor at [xx,yy] and find new [xx,yy] */
	    if(cct++ > 500 || (nxcor && !rn2(35)))
		return;

	    if(xx == COLNO-1 || xx == 0 || yy == 0 || yy == ROWNO-1)
		return;		/* impossible */

	    crm = &levl[xx][yy];
	    if(!(crm->typ)) {
		if(rn2(100)) {
			crm->typ = CORR;
			crm->scrsym = CORR_SYM;
			if(nxcor && !rn2(50))
				(void) mksobj_at(BOULDER, xx, yy);
		} else {
			crm->typ = SCORR;
			crm->scrsym = STONE_SYM;
		}
	    } else
	    if(crm->typ != CORR && crm->typ != SCORR) {
		/* strange ... */
		return;
	    }

	    /* find next corridor position */
	    dix = abs(xx-tx);
	    diy = abs(yy-ty);

	    /* do we have to change direction ? */
	    if(dy && dix > diy) {
		register int ddx = (xx > tx) ? -1 : 1;

		crm = &levl[xx+ddx][yy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR) {
		    dx = ddx;
		    dy = 0;
		    continue;
		}
	    } else if(dx && diy > dix) {
		register int ddy = (yy > ty) ? -1 : 1;

		crm = &levl[xx][yy+ddy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR) {
		    dy = ddy;
		    dx = 0;
		    continue;
		}
	    }

	    /* continue straight on? */
	    crm = &levl[xx+dx][yy+dy];
	    if(!crm->typ || crm->typ == CORR || crm->typ == SCORR)
		continue;

	    /* no, what must we do now?? */
	    if(dx) {
		dx = 0;
		dy = (ty < yy) ? -1 : 1;
		crm = &levl[xx+dx][yy+dy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR)
		    continue;
		dy = -dy;
		continue;
	    } else {
		dy = 0;
		dx = (tx < xx) ? -1 : 1;
		crm = &levl[xx+dx][yy+dy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR)
		    continue;
		dx = -dx;
		continue;
	    }
	}

	/* we succeeded in digging the corridor */
	dodoor(tt.x, tt.y, troom);

	if(smeq[a] < smeq[b])
		smeq[b] = smeq[a];
	else
		smeq[a] = smeq[b];
}

static void
makecorridors() {
	register int a, b;

	nxcor = 0;
	for(a = 0; a < nroom-1; a++)
		join(a, a+1);
	for(a = 0; a < nroom-2; a++)
	    if(smeq[a] != smeq[a+2])
		join(a, a+2);
	for(a = 0; a < nroom; a++)
	    for(b = 0; b < nroom; b++)
		if(smeq[a] != smeq[b])
		    join(a, b);
	if(nroom > 2)
	    for(nxcor = rn2(nroom) + 4; nxcor; nxcor--) {
		a = rn2(nroom);
		b = rn2(nroom-2);
		if(b >= a) b += 2;
		join(a, b);
	    }
}

static void
dosdoor(x,y,aroom,type)
register int x, y;
register struct mkroom *aroom;
register int type;
{
	register struct mkroom *broom;
	register int tmp;
	boolean shdoor = in_shop(x, y);

	if(!IS_WALL(levl[x][y].typ)) /* avoid SDOORs with DOOR_SYM as scrsym */
		type = DOOR;
	levl[x][y].typ = type;
	if(type == DOOR) {
	    levl[x][y].scrsym = DOOR_SYM;
	    if(!rn2(3)) {      /* is it a locked door, closed, or a doorway? */
		if(!rn2(5))
		    levl[x][y].doormask = D_ISOPEN;
		else if(!rn2(4))
		    levl[x][y].doormask = D_LOCKED;
		else
		    levl[x][y].doormask = D_CLOSED;

		if (levl[x][y].doormask != D_ISOPEN && !shdoor && !rn2(25))
		    levl[x][y].doormask |= D_TRAPPED;
	    } else {
		if(shdoor)	levl[x][y].doormask = D_ISOPEN;
		else		levl[x][y].doormask = D_NODOOR;
	    }
	} else { /* SDOOR */
		if(shdoor || !rn2(5))	levl[x][y].doormask = D_LOCKED;
		else			levl[x][y].doormask = D_CLOSED;

		if(!shdoor && !rn2(20)) levl[x][y].doormask |= D_TRAPPED;
	}
	aroom->doorct++;
	broom = aroom+1;
	if(broom->hx < 0) tmp = doorindex; else
	for(tmp = doorindex; tmp > broom->fdoor; tmp--)
		doors[tmp] = doors[tmp-1];
	doorindex++;
	doors[tmp].x = x;
	doors[tmp].y = y;
	for( ; broom->hx >= 0; broom++) broom->fdoor++;
}

static boolean
place_niche(aroom,dy,xx,yy)
register struct mkroom *aroom;
int *dy, *xx, *yy;
{
	coord dd;

	if(rn2(2)) {
	    *dy = 1;
	    finddpos(&dd, aroom->lx, aroom->hy+1, aroom->hx, aroom->hy+1);
	} else {
	    *dy = -1;
	    finddpos(&dd, aroom->lx, aroom->ly-1, aroom->hx, aroom->ly-1);
	}
	*xx = dd.x;
	*yy = dd.y;
	return(levl[*xx][(*yy)+(*dy)].typ == STONE);
}

#ifdef ORACLE
boolean
place_oracle(aroom,dy,xx,yy)
register struct mkroom *aroom;
int *dy, *xx, *yy;
{
	if(!place_niche(aroom,dy,xx,yy)) return FALSE;

	dosdoor(*xx,*yy,aroom,DOOR);
	levl[*xx][*yy].doormask = D_NODOOR;
	return TRUE;
}
#endif

/* there should be one of these per trap */
const char *engravings[] = {	"", "", "", "", "", "",
				"?la? ?as ?er?", "ad ae?ar um",
				"", "", "", "" ,""
				, "", "ad ae?ar um"
#ifdef SPELLS
				,""
#endif
				,""
#ifdef POLYSELF
				,""
#endif
				,""
				};

static void
makeniche(trap_type)
int trap_type;
{
	register struct mkroom *aroom;
	register struct rm *rm;
	register int vct = 8;
	int dy, xx, yy;
	register struct trap *ttmp;

	if(doorindex < DOORMAX)
	  while(vct--) {
	    aroom = &rooms[rn2(nroom)];
	    if(aroom->rtype != OROOM) continue;	/* not an ordinary room */
	    if(aroom->doorct == 1 && rn2(5)) continue;
	    if(!place_niche(aroom,&dy,&xx,&yy)) continue;

	    rm = &levl[xx][yy+dy];
	    if(trap_type || !rn2(4)) {

		rm->typ = SCORR;
		rm->scrsym = STONE_SYM;
		if(trap_type) {
		    ttmp = maketrap(xx, yy+dy, trap_type);
		    ttmp->once = 1;
		    if (strlen(engravings[trap_type]) > 0)
			make_engr_at(xx, yy-dy, engravings[trap_type]);
		}
		dosdoor(xx, yy, aroom, SDOOR);
	    } else {
		rm->typ = CORR;
		rm->scrsym = CORR_SYM;
		if(rn2(7))
		    dosdoor(xx, yy, aroom, rn2(5) ? SDOOR : DOOR);
		else {
		    (void) mksobj_at(SCR_TELEPORTATION, xx, yy+dy);
		    if(!rn2(3)) (void) mkobj_at(0, xx, yy+dy);
		}
	    }
	    return;
	}
}

static void
make_niches()
{
	register int ct = rnd((nroom>>1) + 1);
	boolean	ltptr = TRUE,
		vamp = TRUE;

	while(ct--) {

		if(dlevel > 15 && !rn2(6) && ltptr) {

			ltptr = FALSE;
			makeniche(LEVEL_TELEP);
		} else if (dlevel > 5 && dlevel < 25
			   && !rn2(6) && vamp) {

			vamp = FALSE;
			makeniche(TRAPDOOR);
		} else	makeniche(NO_TRAP);
	}
}

static void
makebigroom()
{
	register int x,y,n;
	register struct mkroom *croom;
	register struct monst *tmonst;

	/* make biggest possible room; make sure it's lit */
	(void) maker(XLIM, COLNO - 2*XLIM - 1, YLIM, ROWNO - 2*YLIM - 1, TRUE);
	croom = &rooms[0];

	/* add extra monsters and goodies */
	n = 10 + rn2(15);
	while (n--) {
		x = somex(croom);
		y = somey(croom);
		tmonst = makemon((struct permonst *) 0,x,y);
		if (tmonst && tmonst->data==&mons[PM_GIANT_SPIDER])
			(void) maketrap(x,y,WEB);
		if (tmonst && rn2(2))
			tmonst->msleep = 1;
	}
	n = 6 + rn2(10);
	while (n--)
		(void) mkobj_at(0,somex(croom),somey(croom));
}

static void
makevtele()
{
	makeniche(TELEP_TRAP);
}

#define rntwixt(L1,L2)	rn1((L2)-(L1),L1)

static void
init_levels()
{
#if defined(STRONGHOLD) && defined(MUSIC)
	register int x;
#endif

#ifdef LINT	/* handle constant in conditional context */
	medusa_level = 0;
#else
	medusa_level = rn1(3, HELLLEVEL - 5);
#endif /* LINT */
#ifdef STRONGHOLD
	stronghold_level = rn1(5, medusa_level)+1;
# ifdef MUSIC
	for (x=0; x<5; x++)
		tune[x] = 'A' + rn2(7);
	tune[5] = 0;
# endif
	/* The tower will be on 3 levels */
	tower_level = rntwixt(stronghold_level, MAXLEVEL-2)+1;
	/* We don't want the wizard in Vlad's tower */
	do
		wiz_level = rntwixt(stronghold_level, MAXLEVEL)+1;
	while (wiz_level >= tower_level && wiz_level <= tower_level + 2);
#else
	wiz_level	 = rntwixt(medusa_level, MAXLEVEL)+1;
#endif /* STRONGHOLD /**/
#ifdef WIZARD
	if (!rn2(15) || wizard)
#else
	if (!rn2(15))
#endif
		/* between the middle of the dungeon and the medusa level */
		bigroom_level = rntwixt(HELLLEVEL>>1, medusa_level);
#ifdef REINCARNATION
# ifdef WIZARD
	if (!rn2(3) || wizard)
# else
	if (!rn2(3))
# endif
		rogue_level = rn1(5,10);
#endif
#ifdef ORACLE
	oracle_level = rn1(4,5);
#endif
}

#undef rntwixt

static void
makelevel() {
	register struct mkroom *croom, *troom;
	register unsigned int tryct;
	register int x,y;
	struct monst *tmonst;	/* always put a web with a spider */

	nroom = 0;
	doorindex = 0;
	rooms[0].hx = -1;	/* in case we are in a maze */

	for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++)
		levl[x][y] = zerorm;

	oinit();	/* assign level dependent obj probabilities */
	fountsound = 0;
	sinksound = 0;

	if (wiz_level == 0)
		init_levels();
	if (
#ifndef STRONGHOLD
	    Inhell
#else
	    dlevel >= stronghold_level || dlevel < 0
#endif
	    || (dlevel > medusa_level && rn2(5))
	   ) {
	    makemaz();
	    return;
	}

	/* construct the rooms */
	nroom = 0;
	secret = FALSE;

#ifdef REINCARNATION
	if (dlevel == rogue_level) {
	    makeroguerooms();
	    makerogueghost();
	} else
#endif
	if (dlevel == bigroom_level)
	    makebigroom();
	else
	    (void) makerooms();

	/* construct stairs (up and down in different rooms if possible) */
	croom = &rooms[rn2(nroom)];
	xdnstair = somex(croom);
	ydnstair = somey(croom);
	levl[xdnstair][ydnstair].scrsym = DN_SYM;
	levl[xdnstair][ydnstair].typ = STAIRS;
#ifdef MEDUSA
	if (dlevel == medusa_level) {
		struct monst *mtmp;

		if (mtmp = makemon(&mons[PM_MEDUSA], xdnstair, ydnstair))
			mtmp->msleep = 1;
		for (tryct = rn1(1,3); tryct; tryct--) {
			x = somex(croom); y = somey(croom);
			if (goodpos(x,y))
				(void) mk_tt_statue(x, y);
		}
	}
#endif
	if(nroom > 1) {
		troom = croom;
		croom = &rooms[rn2(nroom-1)];
		if(croom >= troom) croom++;
	}
	xupstair = somex(croom);    /* %% < and > might be in the same place */
	yupstair = somey(croom);
	levl[xupstair][yupstair].scrsym = UP_SYM;
	levl[xupstair][yupstair].typ = STAIRS;
#ifdef STRONGHOLD
	xdnladder = ydnladder = xupladder = yupladder = 0;
#endif
	is_maze_lev = FALSE;

#ifdef SYSV
	qsort((genericptr_t) rooms, (unsigned)nroom, sizeof(struct mkroom), comp);
#else
	qsort((genericptr_t) rooms, nroom, sizeof(struct mkroom), comp);
#endif
#ifdef REINCARNATION
	if (dlevel == rogue_level) {
	   You("feel as though you were here in a previous lifetime.");
	   return;
	}
#endif
	makecorridors();
	make_niches();

	/* make a secret treasure vault, not connected to the rest */
	if(nroom <= (MAXNROFROOMS/2)) if(rn2(3)) {

		troom = &rooms[nroom];
		secret = TRUE;
		if(makerooms()) {
			troom->rtype = VAULT;		/* treasure vault */
			for(x = troom->lx; x <= troom->hx; x++)
			for(y = troom->ly; y <= troom->hy; y++)
				mkgold((long)(rnd(dlevel*100) + 50), x, y);
			if(!rn2(3))
				makevtele();
		}
	}

#ifdef WIZARD
	if(wizard && getenv("SHOPTYPE")) mkroom(SHOPBASE); else
#endif
#ifdef ORACLE
	if(dlevel == oracle_level) mkroom(DELPHI);
	/*  It is possible that we find no good place to set up Delphi.
	 *  It is also possible to get more than one Delphi using bones levels.
	 *  The first is not a problem; the second is a minor nuisance.
	 */
	else
#endif
	if(dlevel > 1 && dlevel < medusa_level && rn2(dlevel) < 3) mkroom(SHOPBASE);
	else
#ifdef THRONES
	if(dlevel > 4 && !rn2(6)) mkroom(COURT);
	else
#endif
	if(dlevel > 6 && !rn2(7)) mkroom(ZOO);
	else
#ifdef ALTARS
	if(dlevel > 8 && !rn2(5)) mkroom(TEMPLE);
	else
#endif
	if(dlevel > 9 && !rn2(5) && !(mons[PM_KILLER_BEE].geno & G_GENOD))
		mkroom(BEEHIVE);
	else
	if(dlevel > 11 && !rn2(6)) mkroom(MORGUE);
	else
#ifdef ARMY
	if(dlevel > 14 && !rn2(4) && !(mons[PM_SOLDIER].geno & G_GENOD))
		mkroom(BARRACKS);
	else
#endif
	if(dlevel > 18 && !rn2(6)) mkroom(SWAMP);


	/* for each room: put things inside */
	for(croom = rooms; croom->hx > 0; croom++) {
		register boolean boxinlev = FALSE;

		if(croom->rtype != OROOM) continue;

		/* put a sleeping monster inside */
		/* Note: monster may be on the stairs. This cannot be
		   avoided: maybe the player fell through a trapdoor
		   while a monster was on the stairs. Conclusion:
		   we have to check for monsters on the stairs anyway. */

		if(u.uhave_amulet || !rn2(3)) {
		    x = somex(croom); y = somey(croom);
#ifdef REINCARNATION
		    if (dlevel == rogue_level)
			tmonst = makemon(roguemon(), x, y);
		    else
#endif
		    tmonst = makemon((struct permonst *) 0, x,y);
		    if (tmonst && tmonst->data == &mons[PM_GIANT_SPIDER])
			(void) maketrap (x,y,WEB);
		}
		/* put traps and mimics inside */
		goldseen = FALSE;
		while(!rn2(8-(dlevel/6))) mktrap(0,0,croom);
		if(!goldseen && !rn2(3)) mkgold(0L, somex(croom), somey(croom));
#ifdef REINCARNATION
		if (dlevel == rogue_level) goto skip_nonrogue;
#endif
#ifdef FOUNTAINS
		if(!rn2(10)) mkfount(0,croom);
#endif
#ifdef SINKS
		if(!rn2(60)) mksink(croom);
#endif
#ifdef ALTARS
		if(!rn2(60)) mkaltar(croom);
#endif
		/* put statues inside */
#ifdef MEDUSA
		if(!rn2(dlevel == medusa_level ? 1 : 20)) {
			if (!rn2(dlevel == medusa_level ? 2 : 50))
				(void) mk_tt_statue(somex(croom), somey(croom));
			else {
				struct obj *otmp =
					mkstatue((struct permonst *)0,
						somex(croom), somey(croom));
				if (dlevel == medusa_level && otmp) 
					otmp->spe = 0;
				/* Medusa statues don't contain books */
			}
		}
#else
		if(!rn2(20))
				(void) mkstatue((struct permonst *)0,
						somex(croom), somey(croom));
#endif

		/* put box/chest inside */
		if(!rn2(20) && !boxinlev) {

		    boxinlev = TRUE;
		    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST,
				     somex(croom), somey(croom));
		}

#ifdef REINCARNATION
	skip_nonrogue:
#endif
		if(!rn2(3)) {
			(void) mkobj_at(0, somex(croom), somey(croom));
			tryct = 0;
			while(!rn2(5)) {
				if(++tryct > 100){
					Printf("tryct overflow4\n");
					break;
				}
				(void) mkobj_at(0, somex(croom), somey(croom));
			}
		}
	}
}

void
mklev()
{
	if(getbones()) return;

	in_mklev = TRUE;
	makelevel();
	bound_digging();
	in_mklev = FALSE;
}

static boolean
bydoor(x, y)
register xchar x, y;
{
	register boolean tmp1, tmp2;

	/* break up large expression to help some compilers */
	tmp1 = (IS_DOOR(levl[x+1][y].typ) || levl[x+1][y].typ == SDOOR ||
		IS_DOOR(levl[x-1][y].typ) || levl[x-1][y].typ == SDOOR);
	tmp2 = (IS_DOOR(levl[x][y+1].typ) || levl[x][y+1].typ == SDOOR ||
		IS_DOOR(levl[x][y-1].typ) || levl[x][y-1].typ == SDOOR);
	return(tmp1 || tmp2);
}

/* see whether it is allowable to create a door at [x,y] */
int
okdoor(x,y)
register xchar x, y;
{
	register boolean near_door = bydoor(x, y);

	return((levl[x][y].typ == HWALL || levl[x][y].typ == VWALL) &&
	   		doorindex < DOORMAX && !near_door);
}

void
dodoor(x,y,aroom)
register int x, y;
register struct mkroom *aroom;
{
	if(doorindex >= DOORMAX) {
		impossible("DOORMAX exceeded?");
		return;
	}
	if(!okdoor(x,y) && nxcor)
		return;
	dosdoor(x,y,aroom,rn2(8) ? DOOR : SDOOR);
}

static boolean
occupied(x, y)
register xchar x, y;
{
	return(t_at(x, y) || levl[x][y].typ == STAIRS
#ifdef FOUNTAINS
		|| IS_FOUNTAIN(levl[x][y].typ)
#endif
#ifdef THRONES
		|| IS_THRONE(levl[x][y].typ)
#endif
#ifdef SINKS
		|| IS_SINK(levl[x][y].typ)
#endif
#ifdef ALTARS
		|| levl[x][y].typ == ALTAR
#endif
		);
}

/* make a trap somewhere (in croom if mazeflag = 0) */
void
mktrap(num, mazeflag, croom)
register int num, mazeflag;
register struct mkroom *croom;
{
	register struct trap *ttmp;
	register int kind,nomonst,nomimic,nospider,
#ifdef POLYSELF
		    nopoly,
#endif
		    nospikes, nolevltp,
		    nolandmine,
		    tryct = 0;

	xchar mx,my;

	if(!num || num >= TRAPNUM) {
		nomonst = (dlevel < 4) ? 1 : 0;
		nolevltp = (dlevel < 5) ? 1 : 0;
		nospikes = (dlevel < 6) ? 1 : 0;
		nospider = (dlevel < 7) ? 1 : 0;
#ifdef POLYSELF
		nopoly = (dlevel < 6) ? 1 : 0;
#endif
		nolandmine = (dlevel < 5) ? 1 : 0;
		nomimic = (dlevel < 9 || goldseen ) ? 1 : 0;
		if((mons[PM_SMALL_MIMIC].geno & G_GENOD) &&
		   (mons[PM_LARGE_MIMIC].geno & G_GENOD) &&
		   (mons[PM_GIANT_MIMIC].geno & G_GENOD))
			nomimic = 1;
		if(mons[PM_GIANT_SPIDER].geno & G_GENOD)
			nospider = 1;

		do {
#ifdef REINCARNATION
		    if (dlevel==rogue_level) {
			switch(rn2(7)) {
			     case 0: kind = BEAR_TRAP; break;
			     case 1: kind = ARROW_TRAP; break;
			     case 2: kind = DART_TRAP; break;
			     case 3: kind = TRAPDOOR; break;
			     case 4: kind = PIT; break;
			     case 5: kind = SLP_GAS_TRAP; break;
			     case 6: kind = RUST_TRAP; break;
			}
		    } else
#endif
			    kind = rnd(TRAPNUM-1);
		    if((kind == MONST_TRAP && (nomonst && nomimic))
			|| ((kind == WEB) && nospider)
			|| (kind == SPIKED_PIT && nospikes)
			|| (kind == LEVEL_TELEP && nolevltp)
#ifdef POLYSELF
			|| (kind == POLY_TRAP && nopoly)
#endif
			|| (kind == LANDMINE && nolandmine)
			)  kind = NO_TRAP;
		} while(kind == NO_TRAP);
	} else kind = num;

	if(kind == MONST_TRAP && !nomimic && !rn2(4) && !mazeflag) {
		register struct monst *mtmp;

		do {
			if(++tryct > 200) return;
			/* note: fakedoor maybe on actual door */
			if(rn2(2)){
			    if(rn2(2))	mx = croom->hx+1;
			    else	mx = croom->lx-1;
			    my = somey(croom);
			} else {
			    if(rn2(2))	my = croom->hy+1;
			    else	my = croom->ly-1;
			    mx = somex(croom);
			}
		} while(levl[mx][my].mmask);

		if((mtmp = makemon(mkclass(S_MIMIC), mx, my))) {
		    mtmp->mimic = 1;
		    mtmp->mappearance = DOOR_SYM;
		}
		return;
	}

	do {
		if(++tryct > 200)
			return;
		if(mazeflag){
			coord mm;
			mazexy(&mm);
			mx = mm.x;
			my = mm.y;
		} else {
			mx = somex(croom);
			my = somey(croom);
		}
	} while(occupied(mx, my));

	ttmp = maketrap(mx, my, kind);
	if (kind == WEB) (void) makemon(&mons[PM_GIANT_SPIDER], mx, my);
	if(mazeflag && !rn2(10) && ttmp->ttyp < MONST_TRAP)
		ttmp->tseen = 1;
}

#ifdef FOUNTAINS
void
mkfount(mazeflag,croom)
register struct mkroom *croom;
register int mazeflag;
{
	register xchar mx,my;
	register int tryct = 0;

	do {
	    if(++tryct > 200) return;
	    if(mazeflag) {
		 coord mm;
		 mazexy(&mm);
		 mx = mm.x;
		 my = mm.y;
	    } else {
		 mx = somex(croom);
		 my = somey(croom);
	    }
	} while(occupied(mx, my) || bydoor(mx, my));

	/* Put a fountain at mx, my */
	levl[mx][my].typ = FOUNTAIN;
	levl[mx][my].scrsym = FOUNTAIN_SYM;

	fountsound++;
}
#endif /* FOUNTAINS /**/

#ifdef SINKS
static void
mksink(croom)
register struct mkroom *croom;
{
	register xchar mx,my;
	register int tryct = 0;

	do {
	    if(++tryct > 200) return;
	    mx = somex(croom);
	    my = somey(croom);
	} while(occupied(mx, my) || bydoor(mx, my));

	/* Put a sink at mx, my */
	levl[mx][my].typ = SINK;
	levl[mx][my].scrsym = SINK_SYM;

	sinksound++;
}
#endif /* SINKS /**/


#ifdef ALTARS
static void
mkaltar(croom)
register struct mkroom *croom;
{
	register xchar mx,my;
	register int tryct = 0;

	if(croom->rtype != OROOM) return;

	do {
	    if(++tryct > 200) return;
	    mx = somex(croom);
	    my = somey(croom);
	} while(occupied(mx, my) || bydoor(mx, my));

	/* Put an altar at mx, my */
	levl[mx][my].typ = ALTAR;
	levl[mx][my].scrsym = ALTAR_SYM;
	/* 0 - A_CHAOS, 1 - A_NEUTRAL, 2 - A_LAW */
	levl[mx][my].altarmask = rn2((int)A_LAW+1);
}
#endif /* ALTARS /**/
