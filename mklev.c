/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* mklev.c version 1.0.1 - new makecorridor() */

#ifndef MKLEV
#define	MKLEV
#endif MKLEV

#include <stdio.h>
#include "mklev.h"
#include "def.trap.h"
#include "hack.onames.h"	/* for SCR_TELEPORTATION */

extern char *getlogin();
extern struct monst *makemon();

char *tfile,*tspe,**args;
char nul[40];

#include "savelev.h"

#ifdef WIZARD
boolean wizard;
#endif WIZARD

#define somex() ((rand()%(croom->hx-croom->lx+1))+croom->lx)
#define somey() ((rand()%(croom->hy-croom->ly+1))+croom->ly)

struct rm levl[COLNO][ROWNO];
struct monst *fmon;
struct obj *fobj;
struct gen *fgold, *ftrap;

char *fut_geno;		/* monsters that should not be created anymore */

struct mkroom rooms[MAXNROFROOMS+1], *croom, *troom;
int smeq[MAXNROFROOMS+1];
coord doors[DOORMAX];
int doorindex = 0;
int comp();

xchar dlevel;
schar nxcor;
boolean goldseen;
int nroom;

xchar xdnstair,xupstair,ydnstair,yupstair;

main(argc,argv)
char *argv[];
{
	register unsigned tryct;

	if(argc < 6) panic("Too few arguments!!");
	args = argv;
	tfile = argv[1];
	tspe = argv[2];
	dlevel = atoi(argv[3]);
	if(dlevel < 1) panic("Bad level");
	fut_geno = argv[4];
#ifdef WIZARD
	wizard = (argv[5][0] == 'w');
#endif WIZARD
	(void) srand(getpid());
	init_objects();
	rooms[0].hx = -1;	/* in case we are in a maze */

	/* a: normal; b: maze */
	if(*tspe == 'b') {
		makemaz();
		savelev();
		exit(0);
	}

	/* construct the rooms */
	while(nroom < (MAXNROFROOMS/3)) {
		croom = rooms;
		nroom = 0;
		(void) makerooms(0);		/* not secret */
	}

	/* for each room: put things inside */
	for(croom = rooms; croom->hx > 0; croom++) {

		/* put a sleeping monster inside */
		if(!rn2(3)) (void)
			makemon((struct permonst *) 0, somex(), somey());

		/* put traps and mimics inside */
		goldseen = FALSE;
		while(!rn2(8-(dlevel/6))) mktrap(0,0);
		if(!goldseen && !rn2(3)) mkgold(0,somex(),somey());
		if(!rn2(3)) {
			mkobj_at(0, somex(), somey());
			tryct = 0;
			while(!rn2(5)) {
				if(++tryct > 100){
					printf("tryct overflow4\n");
					break;
				}
 mkobj_at(0, somex(), somey());
			}
		}
	}
	tryct = 0;
	do {
		if(++tryct > 1000) panic("Cannot make dnstairs\n");
		croom = &rooms[rn2(nroom)];
		xdnstair = somex();
		ydnstair = somey();
	} while((*tspe =='n' && (!(xdnstair%2) || !(ydnstair%2))) ||
		g_at(xdnstair,ydnstair,ftrap));
	levl[xdnstair][ydnstair].scrsym ='>';
	levl[xdnstair][ydnstair].typ = STAIRS;
	troom = croom;
	do {
		if(++tryct > 2000) panic("Cannot make upstairs\n");
		croom = &rooms[rn2(nroom)];
		xupstair = somex();
		yupstair = somey();
	} while(croom == troom || m_at(xupstair,yupstair) ||
		g_at(xupstair,yupstair,ftrap));
	levl[xupstair][yupstair].scrsym ='<';
	levl[xupstair][yupstair].typ = STAIRS;

	qsort((char *) rooms, nroom, sizeof(struct mkroom), comp);
	makecorridors();
	make_niches();

	/* make a secret treasure vault, not connected to the rest */
	if(nroom < (2*MAXNROFROOMS/3)) if(!rn2(3)) {
		register int x,y;
		troom = croom = &rooms[nroom];
		if(makerooms(1)) {		/* make secret room */
			troom->rtype = 6;		/* treasure vault */
			for(x = troom->lx; x <= troom->hx; x++)
			for(y = troom->ly; y <= troom->hy; y++)
				mkgold(rnd(dlevel*100) + 50, x, y);
			if(!rn2(3))
				makevtele();
		}
	}

#ifdef WIZARD
	if(wizard){
		if(rn2(3)) mkshop(); else mkzoo();
	} else
#endif WIZARD
 	if(dlevel > 1 && dlevel < 20 && rn2(dlevel) < 2) mkshop();
	else
	if(dlevel > 6 && (!rn2(7) || !strcmp("david", getlogin())))
		mkzoo();
	savelev();
	exit(0);
}

makerooms(secret) int secret; {
register int lowx, lowy;
register int tryct = 0;
	while(nroom < (MAXNROFROOMS/2) || secret)
	    for(lowy = rn1(3,3); lowy < ROWNO-7; lowy += rn1(2,4)) {
		for(lowx = rn1(3,4); lowx < COLNO-10; lowx += rn1(2,7)) {
			if(tryct++ > 10000) return(0);
			if((lowy += (rn2(5)-2)) < 3) lowy = 3;
			else if(lowy > ROWNO-6) lowy = ROWNO-6;
			if(levl[lowx][lowy].typ) continue;
			if((secret && maker(lowx, 1, lowy, 1)) ||
			   (!secret && maker(lowx,rn1(9,2),lowy,rn1(4,2))
				&& nroom+2 > MAXNROFROOMS)) return(1);
		}
	}
 return(1);
}

comp(x,y)
register struct mkroom *x,*y;
{
	if(x->lx < y->lx) return(-1);
	return(x->lx > y->lx);
}

coord
finddpos(xl,yl,xh,yh) {
coord ff;
register x,y;
	ff.x = (xl == xh) ? xl : (xl + rn2(xh-xl+1));
	ff.y = (yl == yh) ? yl : (yl + rn2(yh-yl+1));
	if(okdoor(ff.x, ff.y)) return(ff);
	if(xl < xh) for(x = xl; x <= xh; x++)
		if(okdoor(x, ff.y)){
			ff.x = x;
			return(ff);
		}
	if(yl < yh) for(y = yl; y <= yh; y++)
		if(okdoor(ff.x, y)){
			ff.y = y;
			return(ff);
		}
 return(ff);
}

/* if allowable, create a door at [x,y] */
okdoor(x,y)
register x,y;
{
	if(levl[x-1][y].typ == DOOR || levl[x+1][y].typ == DOOR ||
	   levl[x][y+1].typ == DOOR || levl[x][y-1].typ == DOOR ||
	   levl[x-1][y].typ == SDOOR || levl[x+1][y].typ == SDOOR ||
	   levl[x][y-1].typ == SDOOR || levl[x][y+1].typ == SDOOR ||
	   (levl[x][y].typ != HWALL && levl[x][y].typ != VWALL) ||
	   doorindex >= DOORMAX)
		return(0);
	return(1);
}

dodoor(x,y,aroom)
register x,y;
register struct mkroom *aroom;
{
	if(doorindex >= DOORMAX) panic("DOORMAX exceeded?");
	if(!okdoor(x,y) && nxcor) return;
	dosdoor(x,y,aroom,rn2(8) ? DOOR : SDOOR);
}

dosdoor(x,y,aroom,type)
register x,y;
register struct mkroom *aroom;
register type;
{
	register struct mkroom *broom;
	register tmp;

	levl[x][y].typ = type;
	if(type == DOOR)
		levl[x][y].scrsym ='+';
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

/* Only called from makerooms() */
maker(lowx,ddx,lowy,ddy)
schar lowx,ddx,lowy,ddy;
{
	register x, y, hix = lowx+ddx, hiy = lowy+ddy;

	if(nroom >= MAXNROFROOMS) return(0);
	if(hix > COLNO-5) hix = COLNO-5;
	if(hiy > ROWNO-4) hiy = ROWNO-4;
chk:
	if(hix <= lowx || hiy <= lowy) return(0);

	/* check area around room (and make room smaller if necessary) */
	for(x = lowx-4; x <= hix+4; x++)
		for(y = lowy-3; y <= hiy+3; y++)
			if(levl[x][y].typ) {
				if(rn2(3)) return(0);
				lowx = x+5;
				lowy = y+4;
				goto chk;
			}

	/* on low levels the room is lit (usually) */
	/* secret vaults are always lit */
	if((rnd(dlevel) < 10 && rn2(77)) || (ddx == 1 && ddy == 1))
		for(x = lowx-1; x <= hix+1; x++)
			for(y = lowy-1; y <= hiy+1; y++)
				levl[x][y].lit = 1;
	croom->lx = lowx;
	croom->hx = hix;
	croom->ly = lowy;
	croom->hy = hiy;
	croom->rtype = croom->doorct = croom->fdoor = 0;
	for(x = lowx-1; x <= hix+1; x++)
	    for(y = lowy-1; y <= hiy+1; y += (hiy-lowy+2)) {
		levl[x][y].scrsym = '-';
		levl[x][y].typ = HWALL;
	}
	for(x = lowx-1; x <= hix+1; x += (hix-lowx+2))
	    for(y = lowy; y <= hiy; y++) {
		levl[x][y].scrsym = '|';
		levl[x][y].typ = VWALL;
	}
	for(x = lowx; x <= hix; x++)
	    for(y = lowy; y <= hiy; y++) {
		levl[x][y].scrsym = '.';
		levl[x][y].typ = ROOM;
	}
	croom++;
	croom->hx = -1;
	smeq[nroom] = nroom;
	nroom++;
	return(1);
}

makecorridors() {
	register a,b;

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

join(a,b)
register a,b;
{
	coord cc,tt;
	register tx, ty, xx, yy;
	register struct rm *crm;
	register dx, dy, dix, diy, cct;

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
		cc = finddpos(xx,croom->ly,xx,croom->hy);
		tt = finddpos(tx,troom->ly,tx,troom->hy);
	} else if(troom->hy < croom->ly) {
		dy = -1;
		dx = 0;
		yy = croom->ly-1;
		cc = finddpos(croom->lx,yy,croom->hx,yy);
		ty = troom->hy+1;
		tt = finddpos(troom->lx,ty,troom->hx,ty);
	} else if(troom->hx < croom->lx) {
		dx = -1;
		dy = 0;
		xx = croom->lx-1;
		tx = troom->hx+1;
		cc = finddpos(xx,croom->ly,xx,croom->hy);
		tt = finddpos(tx,troom->ly,tx,troom->hy);
	} else {
		dy = 1;
		dx = 0;
		yy = croom->hy+1;
		ty = troom->ly-1;
		cc = finddpos(croom->lx,yy,croom->hx,yy);
		tt = finddpos(troom->lx,ty,troom->hx,ty);
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
		} else {
			crm->typ = SCORR;
			crm->scrsym = ' ';
		}
		if(nxcor && !rn2(50)) {
			mkobj_at(ROCK_SYM, xx, yy);
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
		register ddx = (xx > tx) ? -1 : 1;

		crm = &levl[xx+ddx][yy];
		if(!crm->typ || crm->typ == CORR || crm->typ == SCORR) {
		    dx = ddx;
		    dy = 0;
		    continue;
		}
	    } else if(dx && diy > dix) {
		register ddy = (yy > ty) ? -1 : 1;

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

make_niches()
{
	register int ct = rn2(nroom/2 + 1)+1;
	while(ct--) makeniche(FALSE);
}

makevtele()
{
	makeniche(TRUE);
}

makeniche(with_trap)
boolean with_trap;
{
	register struct mkroom *aroom;
	register struct rm *rm;
	register int vct = 8;
	coord dd;
	register dy,xx,yy;
	register struct gen *gtmp;

	if(doorindex < DOORMAX)
	  while(vct--) {
	    aroom = &rooms[rn2(nroom-1)];
	    if(aroom->rtype != 0) continue;	/* not an ordinary room */
	    if(rn2(2)) {
		dy = 1;
		dd = finddpos(aroom->lx,aroom->hy+1,aroom->hx,aroom->hy+1);
	    } else {
		dy = -1;
		dd = finddpos(aroom->lx,aroom->ly-1,aroom->hx,aroom->ly-1);
	    }
	    xx = dd.x;
	    yy = dd.y;
	    if((rm = &levl[xx][yy+dy])->typ) continue;
	    if(with_trap || !rn2(4)) {
		rm->typ = SCORR;
		rm->scrsym = ' ';
		if(with_trap) {
		    gtmp = newgen();
		    gtmp->gx = xx;
		    gtmp->gy = yy+dy;
		    gtmp->gflag = TELEP_TRAP | ONCE;
		    gtmp->ngen = ftrap;
		    ftrap = gtmp;
		    make_engr_at(xx,yy-dy,"ad ae?ar um");
		}
		dosdoor(xx,yy,aroom,SDOOR);
	    } else {
		rm->typ = CORR;
		rm->scrsym = CORR_SYM;
		if(rn2(7))
		    dosdoor(xx,yy,aroom,rn2(5) ? SDOOR : DOOR);
		else {
		    mksobj_at(SCROLL_SYM,SCR_TELEPORTATION,xx,yy+dy);
		    if(!rn2(3)) mkobj_at(0,xx,yy+dy);
		}
	    }
	    return;
	}
}

/* make a trap somewhere (in croom if mazeflag = 0) */
mktrap(num,mazeflag)
register num,mazeflag;
{
	register struct gen *gtmp;
	register int kind,nopierc,nomimic,fakedoor,fakegold,tryct = 0;
	register xchar mx,my;

	if(!num || num >= TRAPNUM) {
		nopierc = (dlevel < 4) ? 1 : 0;
		nomimic = (dlevel < 9 || goldseen ) ? 1 : 0;
		if(index(fut_geno, 'M')) nomimic = 1;
		kind = rn2(TRAPNUM - nopierc - nomimic);
		/* note: PIERC = 7, MIMIC = 8, TRAPNUM = 9 */
	} else kind = num;

	if(kind == MIMIC) {
		register struct monst *mtmp;

		fakedoor = (!rn2(3) && !mazeflag);
		fakegold = (!fakedoor && !rn2(2));
		if(fakegold) goldseen = TRUE;
		do {
			if(++tryct > 200) return;
			if(fakedoor) {
				/* note: fakedoor maybe on actual door */
				if(rn2(2)){
					if(rn2(2))
						mx = croom->hx+1;
					else mx = croom->lx-1;
					my = somey();
				} else {
					if(rn2(2))
						my = croom->hy+1;
					else my = croom->ly-1;
					mx = somex();
				}
			} else if(mazeflag) {
				extern coord mazexy();
				coord mm;
				mm = mazexy();
				mx = mm.x;
				my = mm.y;
			} else {
				mx = somex();
				my = somey();
			}
		} while(m_at(mx,my));
		if(mtmp = makemon(PM_MIMIC,mx,my))
		    mtmp->mimic =
			fakegold ? '$' : fakedoor ? '+' :
			(mazeflag && rn2(2)) ? AMULET_SYM :
			"=/)%?![<>" [ rn2(9) ];
		return;
	}
	gtmp = newgen();
	gtmp->gflag = kind;
	do {
		if(++tryct > 200){
			free((char *) gtmp);
			return;
		}
		if(mazeflag){
			extern coord mazexy();
			coord mm;
			mm = mazexy();
			gtmp->gx = mm.x;
			gtmp->gy = mm.y;
		} else {
			gtmp->gx = somex();
			gtmp->gy = somey();
		}
	} while(g_at(gtmp->gx, gtmp->gy, ftrap));
	gtmp->ngen = ftrap;
	ftrap = gtmp;
	if(mazeflag && !rn2(10) && gtmp->gflag < PIERC) gtmp->gflag |= SEEN;
}

/*VARARGS1*/
panic(str,arg1,arg2,arg3)
char *str,*arg1,*arg2,*arg3;
{
	char bufr[BUFSZ];
	extern char *sprintf();
	(void) sprintf(bufr,str,arg1,arg2,arg3);
	(void) write(1,"\nMKLEV ERROR:  ",15);
	puts(bufr);
	(void) fflush(stdout);
	exit(1);
}

struct monst *
m_at(x,y)
register x,y;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->mx == x && mtmp->my == y) return(mtmp);
	return(0);
}

struct gen *
g_at(x,y,ptr)
register x,y;
register struct gen *ptr;
{
	while(ptr) {
		if(ptr->gx == x && ptr->gy == y) return(ptr);
		ptr = ptr->ngen;
	}
 return(0);
}
