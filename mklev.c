/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <stdio.h>
#include "mklev.h"
#include "def.trap.h"

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
coord doors[DOORMAX];
int doorindex = 0;
int comp();

xchar dlevel;
schar nxcor,xx,yy,dx,dy,tx,ty; /* for corridors and other things... */
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
	croom = rooms;
	troom = croom+1;
	nxcor = 0;
	mkpos();
	do makecor();
	while (croom->hx > 0 && troom->hx > 0);

	/* make a secret treasure vault, not connected to the rest */
	if(nroom < (2*MAXNROFROOMS/3)) if(!rn2(3)) {
		register int x,y;
		troom = croom = &rooms[nroom];
		if(makerooms(1)) {		/* make secret room */
			troom->rtype = 6;		/* treasure vault */
			for(x = troom->lx; x <= troom->hx; x++)
			for(y = troom->ly; y <= troom->hy; y++)
				mkgold(rnd(dlevel*100) + 50, x, y);
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

/* when croom and troom exist, find position for a door in croom
   and direction for a corridor towards position [tx,ty] in the wall
   of troom */
mkpos()
{
coord cc,tt;
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
	tx = tt.x;
	ty = tt.y;
	if(levl[xx+dx][yy+dy].typ) {
		if(nxcor) newloc();
		else {
			dodoor(xx,yy,croom);
			xx += dx;
			yy += dy;
		}
 return;
	}
 dodoor(xx,yy,croom);
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
	register struct mkroom *broom;
	register tmp;
	if(doorindex >= DOORMAX) panic("DOORMAX exceeded?");
	if(!okdoor(x,y) && nxcor) return;
	if(!rn2(8)) levl[x][y].typ = SDOOR;
	else {
		levl[x][y].scrsym ='+';
		levl[x][y].typ = DOOR;
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

newloc()
{
	register a,b;
	register int tryct = 0;

	++croom;
	++troom;
	if(nxcor || croom->hx < 0 || troom->hx < 0) {
		if(nxcor++ > rn1(nroom,4)) {
			croom = &rooms[nroom];
			return;
		}
		do {
			if(++tryct > 100){
				printf("tryct overflow5\n");
				croom = &rooms[nroom];
				return;
			}
			a = rn2(nroom);
			b = rn2(nroom);
			croom = &rooms[a];
			troom = &rooms[b];
		} while(croom == troom || (troom == croom+1 && !rn2(3)));
	}
 mkpos();
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
			printf("tryct overflow7\n");
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
	nroom++;
	return(1);
}

makecor() {
	register nx, ny;
	register struct rm *crm;
	register dix, diy, secondtry = 0;

tryagain:
	nx = xx + dx;
	ny = yy + dy;

	if(nxcor && !rn2(35)) {
		newloc();
		return;
	}
	if(nx == COLNO-1 || nx == 0 || ny == 0 || ny == ROWNO-1) {
		if(nxcor) {
			newloc();
			return;
		} else {
			printf("something went wrong. we try again...\n");
		execl("./mklev",args[0],tfile,tspe,args[3],args[4],args[5],0);
			panic("cannot execute ./mklev\n");
		}
	}

	dix = abs(nx-tx);
	diy = abs(ny-ty);
	if(dy && dix > diy) {
		dy = 0;
		dx = (nx > tx) ? -1 : 1;
	} else if(dx && diy > dix) {
		dx = 0;
		dy = (ny > ty) ? -1 : 1;
	}

	crm = &levl[nx][ny];
	if(!(crm->typ)) {
		if(rn2(100)) {
			crm->typ = CORR;
			crm->scrsym = CORR_SYM;
		} else {
			crm->typ = SCORR;
			crm->scrsym = ' ';
		}
		xx = nx;
		yy = ny;
		if(nxcor && !rn2(50)) {
			mkobj_at(ROCK_SYM, nx, ny);
		}
 return;
	}
	if(crm->typ == CORR || crm->typ == SCORR) {
		xx = nx;
		yy = ny;
		return;
	}
	if(nx == tx && ny == ty) {
		dodoor(nx,ny,troom);
		newloc();
		return;
	}
	if(!secondtry++ && (nx != xx+dx || ny != yy+dy))
		goto tryagain;
	if(dx) {
		if(ty < ny) dy = -1;
		else dy = levl[nx+dx][ny-1].typ == ROOM?1:-1;
		dx = 0;
	} else {
		if(tx < nx) dx = -1;
		else dx = levl[nx-1][ny+dy].typ == ROOM?1:-1;
		dy = 0;
	}
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
