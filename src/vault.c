/*	SCCS Id: @(#)vault.c	3.0	88/10/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "vault.h"

static void FDECL(restfakecorr,(struct monst *));
OSTATIC boolean FDECL(in_vault,(int,int));
OSTATIC struct monst *NDECL(findgd);
static boolean FDECL(in_fcorridor, (struct monst *,int,int));

#ifdef OVLB

static void
restfakecorr(grd) 
register struct monst *grd;
{
	register int fcx, fcy, fcbeg;
	register struct rm *crm;

	while((fcbeg = EGD(grd)->fcbeg) < EGD(grd)->fcend) {
		fcx = EGD(grd)->fakecorr[fcbeg].fx;
		fcy = EGD(grd)->fakecorr[fcbeg].fy;
		if((u.ux == fcx && u.uy == fcy) || cansee(fcx,fcy) ||
		   m_at(fcx,fcy))
			return;
		crm = &levl[fcx][fcy];
		crm->typ = EGD(grd)->fakecorr[fcbeg].ftyp;
		if(!crm->typ) crm->seen = 0;
		newsym(fcx,fcy);
		if(cansee(fcx,fcy)) prl(fcx,fcy);
		EGD(grd)->fcbeg++;
	}
	/* it seems he left the corridor - let the guard disappear */
	mongone(grd);
}

static boolean
in_fcorridor(grd, x, y)
register struct monst *grd;
int x, y; 
{
	register int fci;

	for(fci = EGD(grd)->fcbeg; fci < EGD(grd)->fcend; fci++)
		if(x == EGD(grd)->fakecorr[fci].fx &&
				y == EGD(grd)->fakecorr[fci].fy)
			return(TRUE);
	return(FALSE);
}

XSTATIC 
struct monst *
findgd() {

	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->isgd && EGD(mtmp)->gdlevel == dlevel)
		return(mtmp);
	return((struct monst *)0);
}

#endif /* OVLB */
#ifdef OVL0

XSTATIC
boolean
in_vault(x, y)
int x, y;
{
    register int roomno = inroom(x, y);

    if(roomno < 0) return(FALSE);
    return(rooms[roomno].rtype == VAULT);
}

void
invault() {

#ifdef BSD_43_BUG
    int dummy;		/* hack to avoid schain botch */
#endif
    struct monst *guard;

    if(!in_vault(u.ux, u.uy)) {
	u.uinvault = 0;
	return;
    }

    guard = findgd();
    if(++u.uinvault % 30 == 0 && !guard) { /* if time ok and no guard now. */
	char buf[BUFSZ];
	register int x, y, dd, gx, gy;

	/* first find the goal for the guard */
	for(dd = 1; (dd < ROWNO || dd < COLNO); dd++) {
	  for(y = u.uy-dd; y <= u.uy+dd; y++) {
	    if(y < 0 || y > ROWNO-1) continue;
	    for(x = u.ux-dd; x <= u.ux+dd; x++) {
	      if(y != u.uy-dd && y != u.uy+dd && x != u.ux-dd)
		x = u.ux+dd;
	      if(x < 0 || x > COLNO-1) continue;
	      if(levl[x][y].typ == CORR) goto fnd;
	    }
	  }
	}
	impossible("Not a single corridor on this level??");
	tele();
	return;
fnd:
	gx = x; gy = y;

	/* next find a good place for a door in the wall */
	x = u.ux; y = u.uy;
	while(levl[x][y].typ == ROOM) {
		register int dx,dy;

		dx = (gx > x) ? 1 : (gx < x) ? -1 : 0;
		dy = (gy > y) ? 1 : (gy < y) ? -1 : 0;
		if(abs(gx-x) >= abs(gy-y))
			x += dx;
		else
			y += dy;
	}

	/* make something interesting happen */
	if(!(guard = makemon(&mons[PM_GUARD], x, y))) return;
	guard->isgd = 1;
	guard->mpeaceful = 1;
	EGD(guard)->gddone = 0;
	EGD(guard)->ogx = x;
	EGD(guard)->ogy = y;
	EGD(guard)->gdlevel = dlevel;
	EGD(guard)->warncnt = 0;

	if(!cansee(guard->mx, guard->my)) {
		mongone(guard);
		return;
	}

	reset_faint();			/* if fainted - wake up */
	pline("Suddenly one of the Vault's guards enters!");
	pmon(guard);
	stop_occupation();		/* if occupied, stop it *now* */
	do {
		pline("\"Hello stranger, who are you?\" - ");
		getlin(buf);
	} while (!letter(buf[0]));

	if(!strcmp(buf, "Croesus") || !strcmp(buf, "Kroisos")) {
		verbalize("Oh, yes, of course.  Sorry to have disturbed you.");
		mongone(guard);
		return;
	}
	clrlin();
	verbalize("I don't know you.");
	if(!u.ugold)
	    verbalize("Please follow me.");
	else {
	    verbalize("Most likely all that gold was stolen from this vault.");
	    verbalize("Please drop that gold and follow me.");
	}
	EGD(guard)->gdx = gx;
	EGD(guard)->gdy = gy;
	EGD(guard)->fcbeg = 0;
	EGD(guard)->fakecorr[0].fx = x;
	EGD(guard)->fakecorr[0].fy = y;
	EGD(guard)->fakecorr[0].ftyp = levl[x][y].typ;
	levl[x][y].typ = DOOR;
	levl[x][y].doormask = D_NODOOR;
	EGD(guard)->fcend = 1;
	EGD(guard)->warncnt = 1;
    }
}

#endif /* OVL0 */
#ifdef OVLB

/*
 * return  1: he moved,  0: he didn't,  -1: let m_move do it,  -2: died
 */
int
gd_move(grd)
register struct monst *grd;
{
	int x, y, nx, ny, m, n;
	int dx, dy, gx, gy, i, fci;
	uchar typ;
	struct fakecorridor *fcp;
	register struct rm *crm;
	register struct gold *gold;
	register boolean goldincorridor = FALSE;

#ifdef __GNULINT__
	m = n = 0;
#endif
	if(EGD(grd)->gdlevel != dlevel) return(-1);
	if(!grd->mpeaceful && in_vault(grd->mx, grd->my) &&
			!in_vault(u.ux, u.uy)) {
		rloc(grd);
		goto letknow;
	}
	if(!grd->mpeaceful) return(-1);
	if(abs(EGD(grd)->ogx - grd->mx) > 1 || 
			abs(EGD(grd)->ogy - grd->my) > 1)
		return(-1);	/* teleported guard - treat as monster */
	if(EGD(grd)->fcend == 1) {
	    if(in_vault(u.ux, u.uy) && 
			(u.ugold || um_dist(grd->mx, grd->my, 1))) {
		if(EGD(grd)->warncnt == 3)
			pline("\"Again, %sfollow me!\"", 
				u.ugold ? "drop that gold and " : "");
		if(EGD(grd)->warncnt == 6) {
			register int m = grd->mx, n = grd->my;
			verbalize("You've been warned, knave!");
			mnexto(grd);
			levl[m][n].typ = EGD(grd)->fakecorr[0].ftyp;
			newsym(m,n);
			if(cansee(m,n)) prl(m,n);
			grd->mpeaceful = 0;
			return(-1);
		}
		/* not fair to get mad when (s)he's fainted */
		if(!is_fainted()) EGD(grd)->warncnt++;
		return(0);
	    }
	    if(!in_vault(u.ux,u.uy) && u.ugold) { /* player teleported */
		register int m = grd->mx, n = grd->my;
		rloc(grd);
		levl[m][n].typ = EGD(grd)->fakecorr[0].ftyp;
		newsym(m,n);
		if(!Blind) prl(m,n);
		grd->mpeaceful = 0;
letknow:
		if(!cansee(grd->mx, grd->my))
		    You("hear the shrill sound of a guard's whistle.");
		else
		    You(um_dist(grd->mx, grd->my, 2) ?
			"see an angry %s approaching." :
			"are confronted by an angry %s.",
			lmonnam(grd)+4);
		return(-1);
	    }
	}
	if(u.ugold && (in_fcorridor(grd, u.ux, u.uy) || /*cover 'blind' spot*/
		    (EGD(grd)->fcend > 1 && in_vault(u.ux, u.uy)))) {
		if(EGD(grd)->warncnt < 6) {
			EGD(grd)->warncnt = 6;
			verbalize("Drop all your gold, scoundrel!");
			return(0);
		} else {
			verbalize("So be it, rogue!");
			grd->mpeaceful = 0;
			return(-1);
		}	
	} 
	for(fci = EGD(grd)->fcbeg; fci < EGD(grd)->fcend; fci++)
	    if(g_at(EGD(grd)->fakecorr[fci].fx, EGD(grd)->fakecorr[fci].fy)){
		m = EGD(grd)->fakecorr[fci].fx;
		n = EGD(grd)->fakecorr[fci].fy;
		goldincorridor = TRUE; 
	    }
	if(goldincorridor && !EGD(grd)->gddone) {
		x = grd->mx;
		y = grd->my;
		if(m == x && n == y) mpickgold(grd);
		else if(m == u.ux && n == u.uy) {
		    gold = g_at(u.ux, u.uy);
 		    grd->mgold += gold->amount;
		    freegold(gold);
		} else {
		    /* just for insurance... */
		    if(MON_AT(m, n) && m != grd->mx && n != grd->my) {
			verbalize("Out of my way, scum!");
			rloc(m_at(m, n));
		    }
		    remove_monster(grd->mx, grd->my);
		    place_monster(grd, m, n);
		    pmon(grd);
		    mpickgold(grd);
		}
		pline("The %s%s picks the gold.", lmonnam(grd)+4,
				grd->mpeaceful ? " calms down and" : "");
		if(x != grd->mx || y != grd->my) {
		    remove_monster(grd->mx, grd->my);
		    place_monster(grd, x, y);
		    pmon(grd);
		}
		goldincorridor = FALSE;
		if(!grd->mpeaceful) return(-1);
		else {
		    EGD(grd)->warncnt = 5;
		    return(0);
		}
	}
	if(um_dist(grd->mx, grd->my, 1) || EGD(grd)->gddone) {
		restfakecorr(grd);
		return(0);	/* didn't move */
	}
	x = grd->mx;
	y = grd->my;
	/* look around (hor & vert only) for accessible places */
	for(nx = x-1; nx <= x+1; nx++) for(ny = y-1; ny <= y+1; ny++) {
	  if((nx == x || ny == y) && (nx != x || ny != y) && isok(nx, ny)) {

	    typ = (crm = &levl[nx][ny])->typ;
	    if(!IS_STWALL(typ) && !IS_POOL(typ)) {

		for(i = EGD(grd)->fcbeg; i < EGD(grd)->fcend; i++)
		    if(EGD(grd)->fakecorr[i].fx == nx && 
				EGD(grd)->fakecorr[i].fy == ny)
			goto nextnxy;

		if((i = inroom(nx,ny)) >= 0 && rooms[i].rtype == VAULT)
			continue;

		/* seems we found a good place to leave him alone */
		EGD(grd)->gddone = 1;
		if(ACCESSIBLE(typ)) goto newpos;
#ifdef STUPID
		if (typ == SCORR)
		    crm->typ = CORR;
		else
		    crm->typ = DOOR;
#else
		crm->typ = (typ == SCORR) ? CORR : DOOR;
#endif
		if(crm->typ == DOOR) crm->doormask = D_NODOOR;
		goto proceed;
	    }
	  }
nextnxy:	;
	}
	nx = x;
	ny = y;
	gx = EGD(grd)->gdx;
	gy = EGD(grd)->gdy;
	dx = (gx > x) ? 1 : (gx < x) ? -1 : 0;
	dy = (gy > y) ? 1 : (gy < y) ? -1 : 0;
	if(abs(gx-x) >= abs(gy-y)) nx += dx; else ny += dy;

	while((typ = (crm = &levl[nx][ny])->typ) != 0) {
	/* in view of the above we must have IS_WALL(typ) or typ == POOL */
	/* must be a wall here */
		if(isok(nx+nx-x,ny+ny-y) && !IS_POOL(typ) &&
		    SPACE_POS(levl[nx+nx-x][ny+ny-y].typ)){
			crm->typ = DOOR;
			crm->doormask = D_NODOOR;
			goto proceed;
		}
		if(dy && nx != x) {
			nx = x; ny = y+dy;
			continue;
		}
		if(dx && ny != y) {
			ny = y; nx = x+dx; dy = 0;
			continue;
		}
		/* I don't like this, but ... */
		crm->typ = DOOR;
		crm->doormask = D_NODOOR;
		goto proceed;
	}
	crm->typ = CORR;
proceed:
	if(cansee(nx,ny)) {
		mnewsym(nx,ny);
		prl(nx,ny);
	}
	fcp = &(EGD(grd)->fakecorr[EGD(grd)->fcend]);
	if(EGD(grd)->fcend++ == FCSIZ) panic("fakecorr overflow");
	fcp->fx = nx;
	fcp->fy = ny;
	fcp->ftyp = typ;
newpos:
	if(EGD(grd)->gddone) {
		/* The following is a kluge.  We need to keep     */
		/* the guard around in order to be able to make   */
		/* the fake corridor disappear as the player      */
		/* moves out of it, but we also need the guard    */
		/* out of the way.  We send the guard to never-   */
		/* never land.  We set ogx ogy to mx my in order  */
		/* to avoid a check at the top of this function.  */
		/* At the end of the process, the guard is killed */
		/* in restfakecorr().				  */
		remove_monster(grd->mx, grd->my);
		place_monster(grd, 0, 0);
		EGD(grd)->ogx = grd->mx;
		EGD(grd)->ogy = grd->my;
		restfakecorr(grd);
		if(in_fcorridor(grd, u.ux, u.uy) || cansee(grd->mx, grd->my))
		    pline("Suddenly, the guard disappears.");
		return(-2);
	}
	EGD(grd)->ogx = grd->mx;	/* update old positions */
	EGD(grd)->ogy = grd->my;
	remove_monster(grd->mx, grd->my);
	place_monster(grd, nx, ny);
	pmon(grd);
	restfakecorr(grd);
	return(1);
}

/* Routine when dying or quitting with a vault guard around */
void
paygd() {

	struct monst *guard;
	register int i;
	int gx,gy;
	char buf[BUFSZ];

	guard = findgd();
	if (!u.ugold || !guard) return;

	if (u.uinvault) {
	    Your("%ld zorkmid%s goes into the Magic Memory Vault.",
		u.ugold, plur(u.ugold));
	    mkgold(u.ugold, u.ux, u.uy);
	    u.ugold = 0L;
	} else {
	    if(guard->mpeaceful) { /* he has no "right" to your gold */
		mongone(guard);
		return;
	    }
	    mnexto(guard);
	    pmon(guard);
	    pline("%s remits your gold to the vault.", Monnam(guard));
	    for(i=0; i<=nroom; i++)
		if (rooms[i].rtype==VAULT) break;
	    if (i > nroom) {
		impossible("no vault?");
		return;
	    }
	    gx = rooms[i].lx + rn2(2);
	    gy = rooms[i].ly + rn2(2);
	    mkgold(u.ugold, gx, gy);
	    u.ugold = 0L;
	    Sprintf(buf,
		"To Croesus: here's the gold recovered from the %s %s...",
		player_mon()->mname, plname);
	    make_engr_at(gx, gy, buf);
	}
	mongone(guard);
}

boolean
gd_sound() {  /* prevent "You hear footsteps.." when inappropriate */
	register struct monst *grd = findgd();

	return(grd == (struct monst *)0);
}

#endif /* OVLB */
