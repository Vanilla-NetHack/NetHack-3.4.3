/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
#ifdef QUEST
setgd(/* mtmp */) /* struct monst *mtmp; */ {}
gd_move() { return(2); }
gddead(mtmp) struct monst *mtmp; {}
invault(){}

#else


extern struct monst *makemon();
#define	VAULT	6
#define	FCSIZ	(ROWNO+COLNO)
struct fakecorr {
	xchar fx,fy,ftyp;
};

struct egd {
	int fcbeg, fcend;	/* fcend: first unused pos */
	xchar gdx, gdy;		/* goal of guard's walk */
	unsigned gddone:1;
	struct fakecorr fakecorr[FCSIZ];
};

struct permonst pm_guard =
	{ "guard", '@', 12, 12, -1, 4, 10, sizeof(struct egd) };

struct monst *guard;
int gdlevel;
#define	EGD	((struct egd *)(&(guard->mextra[0])))

restfakecorr(){
register fcx,fcy,fcbeg;
register struct rm *crm;

	while((fcbeg = EGD->fcbeg) < EGD->fcend) {
		fcx = EGD->fakecorr[fcbeg].fx;
		fcy = EGD->fakecorr[fcbeg].fy;
		if((u.ux == fcx && u.uy == fcy) || cansee(fcx,fcy) ||
		   m_at(fcx,fcy))
			return;
		crm = &levl[fcx][fcy];
		crm->typ = EGD->fakecorr[fcbeg].ftyp;
		if(!crm->typ) crm->seen = 0;
		newsym(fcx,fcy);
		EGD->fcbeg++;
	}
	/* it seems he left the corridor - let the guard disappear */
	mondead(guard);
	guard = 0;
}

setgd(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(mtmp->isgd){
		guard = mtmp;
		gdlevel = dlevel;
		return;
	}
 guard = 0;
}

invault(){
register tmp = inroom(u.ux, u.uy);
	if(tmp < 0 || rooms[tmp].rtype != VAULT) {
		u.uinvault = 0;
		return;
	}
	if(++u.uinvault % 50 == 0 && (!guard || gdlevel != dlevel)) {
	char buf[BUFSZ];
	register x,y,dx,dy,gx,gy;

		/* first find the goal for the guard */
		for(dy = 0; dy < ROWNO; dy++)
		  for(y = u.uy-dy; y <= u.uy+dy; y++) {
		    if(y > u.uy-dy) y = u.uy+dy;
		    if(y < 0 || y > ROWNO-1) continue;
		    for(x = u.ux; x < COLNO; x++)
			if(levl[x][y].typ == CORR) goto fnd;
		    for(x = u.ux-1; x > 0; x--)
			if(levl[x][y].typ == CORR) goto fnd;
		}
		impossible();
		tele();
		return;
	fnd:
		gx = x; gy = y;

		/* next find a good place for a door in the wall */
		x = u.ux; y = u.uy;
		while(levl[x][y].typ > DOOR) {
			dx = (gx > x) ? 1 : (gx < x) ? -1 : 0;
			dy = (gy > y) ? 1 : (gy < y) ? -1 : 0;
			if(abs(gx-x) >= abs(gy-y)) x += dx;
			else y += dy;
		}

		/* make something interesting happen */
		if(!(guard = makemon(&pm_guard,x,y))) return;
		guard->isgd = guard->mpeaceful = 1;
		EGD->gddone = 0;
		gdlevel = dlevel;
		if(!cansee(guard->mx, guard->my)) {
			mondead(guard);
			guard = 0;
			return;
		}
		EGD->gdx = gx;
		EGD->gdy = gy;
		EGD->fcbeg = 0;
		EGD->fakecorr[0].fx = x;
		EGD->fakecorr[0].fy = y;
		EGD->fakecorr[0].ftyp = levl[x][y].typ;
		levl[x][y].typ = DOOR;
		EGD->fcend = 1;

		pline("Suddenly one of the Vault's guards enters!");
		pmon(guard);
		pline("\"Hello stranger, who are you?\" - ");
		getlin(buf);
		clrlin();
		pline("\"I don't know you.\"");
		if(!u.ugold) pline("\"Please follow me.\"");
		else {
		    pline("\"Most likely all that gold was stolen from this vault.\"");
		    pline("\"Please drop your gold (say d$ ) and follow me.\"");
		}
	}
}

gd_move(){
register int x,y,dx,dy,gx,gy,nx,ny,tmp;
register struct fakecorr *fcp;
register struct rm *crm;
	if(!guard || gdlevel != dlevel){
		pline("Where is the guard?");
		impossible();
		return(2);	/* died */
	}
	if(u.ugold || dist(guard->mx,guard->my) > 2 || EGD->gddone){
		restfakecorr();
		return(0);	/* didnt move */
	}
	x = guard->mx;
	y = guard->my;
	/* look around (hor & vert only) for accessible places */
	for(nx = x-1; nx <= x+1; nx++) for(ny = y-1; ny <= y+1; ny++)
	    if(nx == x || ny == y) if(nx != x || ny != y)
	    if(isok(nx,ny))
	    if((tmp = (crm = &levl[nx][ny])->typ) >= SDOOR) {
		register int i;
		for(i = EGD->fcbeg; i < EGD->fcend; i++)
			if(EGD->fakecorr[i].fx == nx &&
			   EGD->fakecorr[i].fy == ny)
				goto nextnxy;
		if((i = inroom(nx,ny)) >= 0 && rooms[i].rtype == VAULT)
			goto nextnxy;
		/* seems we found a good place to leave him alone */
		EGD->gddone = 1;
		if(tmp >= DOOR) goto newpos;
		crm->typ = (tmp == SCORR) ? CORR : DOOR;
		goto proceed;
	nextnxy:	;
	}
	nx = x;
	ny = y;
	gx = EGD->gdx;
	gy = EGD->gdy;
	dx = (gx > x) ? 1 : (gx < x) ? -1 : 0;
	dy = (gy > y) ? 1 : (gy < y) ? -1 : 0;
	if(abs(gx-x) >= abs(gy-y)) nx += dx; else ny += dy;

	while((tmp = (crm = &levl[nx][ny])->typ) != 0) {
	/* in view of the above we must have  tmp < SDOOR */
	/* must be a wall here */
		if(isok(nx+nx-x,ny+ny-y) && levl[nx+nx-x][ny+ny-y].typ > DOOR){
			crm->typ = DOOR;
			goto proceed;
		}
		if(dy && nx != x) {
			nx = x; ny = y+dy; dx = 0;
			continue;
		}
		if(dx && ny != y) {
			ny = y; nx = x+dx; dy = 0;
			continue;
		}
		/* I don't like this, but ... */
		crm->typ = DOOR;
		goto proceed;
	}
	crm->typ = CORR;
proceed:
	fcp = &(EGD->fakecorr[EGD->fcend]);
	if(EGD->fcend++ == FCSIZ) panic("fakecorr overflow");
	fcp->fx = nx;
	fcp->fy = ny;
	fcp->ftyp = tmp;
newpos:
	if(EGD->gddone) nx = ny = 0;
	guard->mx = nx;
	guard->my = ny;
	pmon(guard);
	restfakecorr();
	return(1);
}

gddead(){
	guard = 0;
}


#endif QUEST
