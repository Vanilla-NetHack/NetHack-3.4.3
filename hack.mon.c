/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#include "hack.mfndpos.h"
#define	SIZE(x)	(int)(sizeof(x) / sizeof(x[0]))
#define	NULL	(char *) 0
extern struct monst *makemon();

int warnlevel;		/* used by movemon and dochugw */
long lastwarntime;
int lastwarnlev;
char *warnings[] = {
	"white", "pink", "red", "ruby", "purple", "black"
};

movemon()
{
	register struct monst *mtmp;
	register int fr;

	warnlevel = 0;

	while(1) {
		/* find a monster that we haven't treated yet */
		/* note that mtmp or mtmp->nmon might get killed
		   while mtmp moves, so we cannot just walk down the
		   chain (even new monsters might get created!) */
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(mtmp->mlstmv < moves) goto next_mon;
		/* treated all monsters */
		break;

	next_mon:
		mtmp->mlstmv = moves;
		if(mtmp->mblinded && !--mtmp->mblinded)
			mtmp->mcansee = 1;
		if(mtmp->mimic) continue;
		if(mtmp->mspeed != MSLOW || !(moves%2)){
			/* continue if the monster died fighting */
			fr = -1;
			if(Conflict && cansee(mtmp->mx,mtmp->my)
				&& (fr = fightm(mtmp)) == 2)
				continue;
			if(fr<0 && dochugw(mtmp))
				continue;
		}
		if(mtmp->mspeed == MFAST && dochugw(mtmp))
			continue;
	}

	warnlevel -= u.ulevel;
	if(warnlevel >= SIZE(warnings))
		warnlevel = SIZE(warnings)-1;
	if(warnlevel >= 0)
	if(warnlevel > lastwarnlev || moves > lastwarntime + 5){
	    register char *rr;
	    switch(Warning & (LEFT_RING | RIGHT_RING)){
	    case LEFT_RING:
		rr = "Your left ring glows";
		break;
	    case RIGHT_RING:
		rr = "Your right ring glows";
		break;
	    case LEFT_RING | RIGHT_RING:
		rr = "Both your rings glow";
		break;
	    default:
		rr = "Your fingertips glow";
		break;
	    }
	    pline("%s %s!", rr, warnings[warnlevel]);
	    lastwarntime = moves;
	    lastwarnlev = warnlevel;
	}

	dmonsfree();	/* remove all dead monsters */
}

justswld(mtmp,name)
register struct monst *mtmp;
char *name;
{

	mtmp->mx = u.ux;
	mtmp->my = u.uy;
	u.ustuck = mtmp;
	pmon(mtmp);
	kludge("%s swallows you!",name);
	more();
	seeoff(1);
	u.uswallow = 1;
	swallowed();
}

youswld(mtmp,dam,die,name)
register struct monst *mtmp;
register dam,die;
char *name;
{
	if(mtmp != u.ustuck) return;
	kludge("%s digests you!",name);
	u.uhp -= dam;
	if(u.uswldtim++ == die){
		pline("It totally digests you!");
		u.uhp = -1;
	}
 if(u.uhp < 1) done_in_by(mtmp);
}

dochugw(mtmp) register struct monst *mtmp; {
register x = mtmp->mx;
register y = mtmp->my;
register d = dochug(mtmp);
register dd;
	if(!d)		/* monster still alive */
	if(Warning)
	if(!mtmp->mpeaceful)
	if((dd = dist(mtmp->mx,mtmp->my)) < dist(x,y))
	if(dd < 100)
	if(!cansee(mtmp->mx, mtmp->my) || (mtmp->minvis && !See_invisible))
	if(mtmp->data->mlevel > warnlevel)
		warnlevel = mtmp->data->mlevel;
	return(d);
}

/* returns 1 if monster died moving, 0 otherwise */
dochug(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat;
	register tmp;

	if(mtmp->cham && !rn2(6))
		(void) newcham(mtmp, &mons[dlevel+14+rn2(CMNUM-14-dlevel)]);
	mdat = mtmp->data;
	if(mdat->mlevel < 0)
		panic("bad monster %c (%d)",mdat->mlet,mdat->mlevel);
	if((!(moves%20) || index("ViT",mdat->mlet)) &&
	    mtmp->mhp<mtmp->orig_hp)
		mtmp->mhp++; /* regenerate monsters. */
	if(mtmp->mfroz) return(0); /* frozen monsters don't do anything. */
	if(mtmp->msleep) {/* wake up a monster, or get out of here. */
		if(cansee(mtmp->mx,mtmp->my) && !Stealth &&
			(!index("NL",mdat->mlet) || !rn2(50)) &&
			(Aggravate_monster || (!rn2(7) && !mtmp->mimic)))
			mtmp->msleep = 0;
		else return(0);
	}

	/* not frozen or sleeping: wipe out texts written in the dust */
	wipe_engr_at(mtmp->mx, mtmp->my, 1);

	/* confused monsters get unconfused with small probability */
	if(mtmp->mconf && !rn2(50)) mtmp->mconf = 0;

	/* some monsters teleport */
	if(mtmp->mflee && index("tNL", mdat->mlet) && !rn2(40)){
		rloc(mtmp);
		return(0);
	}
	if(mdat->mmove < rnd(6)) return(0);
	if((mtmp->mflee ||
		mtmp->mconf ||
		(index("BIuy", mdat->mlet) && !rn2(4)) ||
		(mdat->mlet == 'L' && !u.ugold && (mtmp->mgold || rn2(2))) ||
		dist(mtmp->mx,mtmp->my) > 2 ||
		(!mtmp->mcansee && !rn2(4)) ||
		mtmp->mpeaceful
	   ) && (tmp = m_move(mtmp,0)) && mdat->mmove <= 12)
		return(tmp == 2);
	if(tmp == 2) return(1);	/* monster died moving */

	if(!index("Ea", mdat->mlet) && dist(mtmp->mx, mtmp->my) < 3 &&
	 !mtmp->mpeaceful && u.uhp > 0 &&
	 !sengr_at("Elbereth", u.ux, u.uy) &&
	 !sobj_at(SCR_SCARE_MONSTER, u.ux, u.uy)) {
		if(mhitu(mtmp))
			return(1);	/* monster died (e.g. 'y' or 'F') */
	}
	/* extra movement for fast monsters */
	if(mdat->mmove-12 > rnd(12)) tmp = m_move(mtmp,1);
	return(tmp == 2);
}

inrange(mtmp)
register struct monst *mtmp;
{
	register schar tx,ty;

	/* spit fire only when both in a room or both in a corridor */
	if(inroom(u.ux,u.uy) != inroom(mtmp->mx,mtmp->my)) return;
	tx = u.ux - mtmp->mx;
	ty = u.uy - mtmp->my;
	if((!tx && abs(ty) < 8) || (!ty && abs(tx) < 8)
	    || (abs(tx) == abs(ty) && abs(tx) < 8)){
		/* spit fire in the direction of @ (not nec. hitting) */
		buzz(-1,mtmp->mx,mtmp->my,sgn(tx),sgn(ty));
		if(u.uhp < 1) done_in_by(mtmp);
	}
}

m_move(mtmp,after)
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	register nx,ny,omx,omy,appr,nearer,cnt,i,j;
	xchar gx,gy,nix,niy,chcnt;
	schar chi;
	boolean likegold, likegems, likeobjs;
	schar mmoved = 0;	/* not strictly nec.: chi >= 0 will do */
	coord poss[9];
	int info[9];

	if(mtmp->mtrapped) {
		i = mintrap(mtmp);
		if(i == 2) return(2);	/* he died */
		if(i == 1) return(0);	/* still in trap, so didnt move */
	}
	if(mtmp->mhide && o_at(mtmp->mx,mtmp->my) && rn2(10))
		return(0);		/* do not leave hiding place */

	/* my dog gets a special treatment */
	if(mtmp->mtame) {
		return( dog_move(mtmp, after) );
	}

	/* likewise for shopkeeper */
	if(mtmp->isshk) {
		mmoved = shk_move();
		goto postmov;
	}

	/* and for the guard */
	if(mtmp->isgd) {
		mmoved = gd_move();
		goto postmov;
	}

	if(mtmp->data->mlet == 't' && !rn2(5)) {
		if(rn2(2))
			mnexto(mtmp);
		else
			rloc(mtmp);
		mmoved = 1;
		goto postmov;
	}
	if(mtmp->data->mlet == 'D' && !mtmp->mcan)
		inrange(mtmp);
	if(!Blind && !Confusion && mtmp->data->mlet == 'U' && !mtmp->mcan
		&& cansee(mtmp->mx,mtmp->my) && rn2(5)) {
		pline("%s's gaze has confused you!", Monnam(mtmp));
		if(rn2(5)) mtmp->mcan = 1;
		Confusion = d(3,4);		/* timeout */
	}
	if(!mtmp->mflee && u.uswallow && u.ustuck != mtmp) return(1);
	appr = 1;
	if(mtmp->mflee) appr = -1;
	if(mtmp->mconf || Invis ||  !mtmp->mcansee ||
		(index("BIy",mtmp->data->mlet) && !rn2(3)))
		appr = 0;
	omx = mtmp->mx;
	omy = mtmp->my;
	gx = u.ux;
	gy = u.uy;
	if(mtmp->data->mlet == 'L' && appr == 1 && mtmp->mgold > u.ugold)
		appr = -1;
#ifdef TRACK
	/* random criterion for 'smell'
	   should use mtmp->msmell
	 */
	if('a' <= mtmp->data->mlet && mtmp->data->mlet <= 'z') {
	extern coord *gettrack();
	register coord *cp;
	schar mroom;
		mroom = inroom(omx,omy);
		if(mroom < 0 || mroom != inroom(u.ux,u.uy)){
		    cp = gettrack(omx,omy);
		    if(cp){
			gx = cp->x;
			gy = cp->y;
		    }
		}
	}
#endif TRACK
	/* look for gold or jewels nearby */
	likegold = (index("LOD", mtmp->data->mlet) != NULL);
	likegems = (index("ODu", mtmp->data->mlet) != NULL);
	likeobjs = mtmp->mhide;
#define	SRCHRADIUS	25
	{ xchar mind = SRCHRADIUS;		/* not too far away */
	  register int dd;
	  if(likegold){
		register struct gen *gold;
		for(gold = fgold; gold; gold = gold->ngen)
		  if((dd = DIST(omx,omy,gold->gx,gold->gy)) < mind){
		    mind = dd;
		    gx = gold->gx;
		    gy = gold->gy;
		}
	  }
	  if(likegems || likeobjs){
		register struct obj *otmp;
		for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(likeobjs || otmp->olet == GEM_SYM)
		if(mtmp->data->mlet != 'u' ||
			objects[otmp->otyp].g_val != 0)
		if((dd = DIST(omx,omy,otmp->ox,otmp->oy)) < mind){
		    mind = dd;
		    gx = otmp->ox;
		    gy = otmp->oy;
		}
	  }
	  if(mind < SRCHRADIUS && appr == -1) {
		if(dist(omx,omy) < 10) {
		    gx = u.ux;
		    gy = u.uy;
		} else
   appr = 1;
	  }
	}
	nix = omx;
	niy = omy;
	cnt = mfndpos(mtmp,poss,info,
		mtmp->data->mlet == 'u' ? NOTONL :
		index(" VWZ", mtmp->data->mlet) ? NOGARLIC : ALLOW_TRAPS);
		/* ALLOW_ROCK for some monsters ? */
	chcnt = 0;
	chi = -1;
	for(i=0; i<cnt; i++) {
		nx = poss[i].x;
		ny = poss[i].y;
		for(j=0; j<MTSZ && j<cnt-1; j++)
			if(nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
				if(rn2(4*(cnt-j))) goto nxti;
#ifdef STUPID
		/* some stupid compilers think that this is too complicated */
		{ int d1 = DIST(nx,ny,gx,gy);
		  int d2 = DIST(nix,niy,gx,gy);
		  nearer = (d1 < d2);
		}
#else
		nearer = (DIST(nx,ny,gx,gy) < DIST(nix,niy,gx,gy));
#endif STUPID
		if((appr == 1 && nearer) || (appr == -1 && !nearer) ||
			!mmoved ||
			(!appr && !rn2(++chcnt))){
			nix = nx;
			niy = ny;
			chi = i;
			mmoved = 1;
		}
 nxti:	;
	}
	if(mmoved){
		if(info[chi] & ALLOW_M){
			mtmp2 = m_at(nix,niy);
			if(hitmm(mtmp,mtmp2) == 1 && rn2(4) &&
			  hitmm(mtmp2,mtmp) == 2) return(2);
			return(0);
		}
		if(info[chi] & ALLOW_U){
		  (void) hitu(mtmp, d(mtmp->data->damn, mtmp->data->damd)+1);
		  return(0);
		}
		mtmp->mx = nix;
		mtmp->my = niy;
		for(j=MTSZ-1; j>0; j--) mtmp->mtrack[j] = mtmp->mtrack[j-1];
		mtmp->mtrack[0].x = omx;
		mtmp->mtrack[0].y = omy;
#ifndef NOWORM
		if(mtmp->wormno) worm_move(mtmp);
#endif NOWORM
	} else {
		if(mtmp->data->mlet == 'u' && rn2(2)){
			rloc(mtmp);
			return(0);
		}
#ifndef NOWORM
		if(mtmp->wormno) worm_nomove(mtmp);
#endif NOWORM
	}
postmov:
	if(mmoved == 1) {
		if(mintrap(mtmp) == 2)	/* he died */
			return(2);
		if(likegold) mpickgold(mtmp);
		if(likegems) mpickgems(mtmp);
		if(mtmp->mhide) mtmp->mundetected = 1;
	}
	pmon(mtmp);
	return(mmoved);
}

mpickgold(mtmp) register struct monst *mtmp; {
register struct gen *gold;
	while(gold = g_at(mtmp->mx, mtmp->my, fgold)){
		mtmp->mgold += gold->gflag;
		freegold(gold);
		if(levl[mtmp->mx][mtmp->my].scrsym == '$')
			newsym(mtmp->mx, mtmp->my);
	}
}

mpickgems(mtmp) register struct monst *mtmp; {
register struct obj *otmp;
	for(otmp = fobj; otmp; otmp = otmp->nobj)
	if(otmp->olet == GEM_SYM)
	if(otmp->ox == mtmp->mx && otmp->oy == mtmp->my)
	if(mtmp->data->mlet != 'u' || objects[otmp->otyp].g_val != 0){
		freeobj(otmp);
		mpickobj(mtmp, otmp);
		if(levl[mtmp->mx][mtmp->my].scrsym == GEM_SYM)
			newsym(mtmp->mx, mtmp->my);	/* %% */
		return;	/* pick only one object */
	}
}

/* return number of acceptable neighbour positions */
mfndpos(mon,poss,info,flag)
register struct monst *mon; coord poss[9]; int info[9], flag; {
register int x,y,nx,ny,cnt = 0,tmp;
register struct monst *mtmp;
	x = mon->mx;
	y = mon->my;
	if(mon->mconf) {
		flag |= ALLOW_ALL;
		flag &= ~NOTONL;
	}
	for(nx = x-1; nx <= x+1; nx++) for(ny = y-1; ny <= y+1; ny++)
	if(nx != x || ny != y) if(isok(nx,ny))
	if((tmp = levl[nx][ny].typ) >= DOOR)
	if(!(nx != x && ny != y &&
		(levl[x][y].typ == DOOR || tmp == DOOR))){
		info[cnt] = 0;
		if(nx == u.ux && ny == u.uy){
			if(!(flag & ALLOW_U)) continue;
			info[cnt] = ALLOW_U;
		} else if(mtmp = m_at(nx,ny)){
			if(!(flag & ALLOW_M)) continue;
			info[cnt] = ALLOW_M;
			if(mtmp->mtame){
				if(!(flag & ALLOW_TM)) continue;
				info[cnt] |= ALLOW_TM;
			}
		}
		if(sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
			if(flag & NOGARLIC) continue;
			info[cnt] |= NOGARLIC;
		}
		if(sobj_at(SCR_SCARE_MONSTER, nx, ny) ||
		   (!mon->mpeaceful && sengr_at("Elbereth", nx, ny))) {
			if(!(flag & ALLOW_SSM)) continue;
			info[cnt] |= ALLOW_SSM;
		}
		if(sobj_at(ENORMOUS_ROCK, nx, ny)) {
			if(!(flag & ALLOW_ROCK)) continue;
			info[cnt] |= ALLOW_ROCK;
		}
		if(!Invis && online(nx,ny)){
			if(flag & NOTONL) continue;
			info[cnt] |= NOTONL;
		}
		/* we cannot avoid traps of an unknown kind */
		{ register struct gen *gtmp = g_at(nx, ny, ftrap);
		  register int tt;
			if(gtmp) {
				tt = 1 << (gtmp->gflag & ~SEEN);
				if(mon->mtrapseen & tt){
					if(!(flag & tt)) continue;
					info[cnt] |= tt;
				}
			}
		}
		poss[cnt].x = nx;
		poss[cnt].y = ny;
		cnt++;
	}
	return(cnt);
}

dist(x,y) int x,y; {
	return((x-u.ux)*(x-u.ux) + (y-u.uy)*(y-u.uy));
}

poisoned(string, pname)
register char *string, *pname;
{
	if(Blind) pline("It was poisoned.");
	else pline("The %s was poisoned!",string);
	if(Poison_resistance) {
		pline("The poison doesn't seem to affect you.");
		return;
	}
	switch(rnd(6)) {
	case 1:
		u.uhp = -1;
		break;
	case 2:
	case 3:
	case 4:
		losestr(rn1(3,3));
		break;
	case 5:
	case 6:
		losehp(rn1(10,6), pname);
		return;
	}
	if(u.uhp < 1) killer = pname;
}

mondead(mtmp)
register struct monst *mtmp;
{
	relobj(mtmp,1);
	unpmon(mtmp);
	relmon(mtmp);
	if(u.ustuck == mtmp) {
		u.ustuck = 0;
		if(u.uswallow) {
			u.uswallow = 0;
			setsee();
			docrt();
		}
	}
	if(mtmp->isshk) shkdead();
	if(mtmp->isgd) gddead();
#ifndef NOWORM
	if(mtmp->wormno) wormdead(mtmp);
#endif NOWORM
	monfree(mtmp);
}

/* called when monster is moved to larger structure */
replmon(mtmp,mtmp2)
register struct monst *mtmp, *mtmp2;
{
	relmon(mtmp);
	monfree(mtmp);
	mtmp2->nmon = fmon;
	fmon = mtmp2;
}

relmon(mon)
register struct monst *mon;
{
	register struct monst *mtmp;

	if(mon == fmon) fmon = fmon->nmon;
	else {
		for(mtmp = fmon; mtmp->nmon != mon; mtmp = mtmp->nmon) ;
		mtmp->nmon = mon->nmon;
	}
}

/* we do not free monsters immediately, in order to have their name
   available shortly after their demise */
struct monst *fdmon;	/* chain of dead monsters, need not to be saved */

monfree(mtmp) register struct monst *mtmp; {
	mtmp->nmon = fdmon;
	fdmon = mtmp;
}

dmonsfree(){
register struct monst *mtmp;
	while(mtmp = fdmon){
		fdmon = mtmp->nmon;
		free((char *) mtmp);
	}
}

killed(mtmp) struct monst *mtmp; {
#ifdef lint
#define	NEW_SCORING
#endif lint
register int tmp,tmp2,nk,x,y;
register struct permonst *mdat = mtmp->data;
	if(mtmp->cham) mdat = PM_CHAM;
	if(Blind) pline("You destroy it!");
	else {
		pline("You destroy %s!",
			mtmp->mtame ? amonnam(mtmp, "poor") : monnam(mtmp));
	}
	if(u.umconf) {
		if(!Blind) pline("Your hands stop glowing blue.");
		u.umconf = 0;
	}

	/* count killed monsters */
#define	MAXMONNO	100
	nk = 1;		      /* in case we cannot find it in mons */
	tmp = mdat - mons;    /* index in mons array (if not 'd', '@', ...) */
	if(tmp >= 0 && tmp < CMNUM+2) {
	    extern char fut_geno[];
	    u.nr_killed[tmp]++;
	    if((nk = u.nr_killed[tmp]) > MAXMONNO &&
		!index(fut_geno, mdat->mlet))
		    charcat(fut_geno,  mdat->mlet);
	}

	/* punish bad behaviour */
	if(mdat->mlet == '@') Telepat = 0, u.uluck -= 2;
	if(mtmp->mpeaceful || mtmp->mtame) u.uluck--;
	if(mdat->mlet == 'u') u.uluck -= 5;

	/* give experience points */
	tmp = 1 + mdat->mlevel * mdat->mlevel;
	if(mdat->ac < 3) tmp += 2*(7 - mdat->ac);
	if(index("AcsSDXaeRTVWU&In:P", mdat->mlet))
		tmp += 2*mdat->mlevel;
	if(index("DeV&P",mdat->mlet)) tmp += (7*mdat->mlevel);
	if(mdat->mlevel > 6) tmp += 50;

#ifdef NEW_SCORING
	/* ------- recent addition: make nr of points decrease
		   when this is not the first of this kind */
	{ int ul = u.ulevel;
	  int ml = mdat->mlevel;

	if(ul < 14)    /* points are given based on present and future level */
	    for(tmp2 = 0; !tmp2 || ul + tmp2 <= ml; tmp2++)
		if(u.uexp + 1 + (tmp + ((tmp2 <= 0) ? 0 : 4<<(tmp2-1)))/nk
		    >= 10*pow((unsigned)(ul-1)))
			if(++ul == 14) break;

	tmp2 = ml - ul -1;
	tmp = (tmp + ((tmp2 < 0) ? 0 : 4<<tmp2))/nk;
	if(!tmp) tmp = 1;
	}
	/* note: ul is not necessarily the future value of u.ulevel */
	/* ------- end of recent valuation change ------- */
#endif NEW_SCORING

	u.uexp += tmp;
	u.urexp += 4*tmp;
	flags.botl = 1;
	while(u.ulevel < 14 && u.uexp >= 10*pow(u.ulevel-1)){
		pline("Welcome to level %d.", ++u.ulevel);
		tmp = rnd(10);
		if(tmp < 3) tmp = rnd(10);
		u.uhpmax += tmp;
		u.uhp += tmp;
		flags.botl = 1;
	}

	/* dispose of monster and make cadaver */
	x = mtmp->mx;	y = mtmp->my;
	mondead(mtmp);
	tmp = mdat->mlet;
	if(tmp == 'm') { /* he killed a minotaur, give him a wand of digging */
			/* note: the dead minotaur will be on top of it! */
		mksobj_at(WAND_SYM, WAN_DIGGING, x, y);
		/* if(cansee(x,y)) atl(x,y,fobj->olet); */
		stackobj(fobj);
	} else
#ifndef NOWORM
	if(tmp == 'w') {
		mksobj_at(WEAPON_SYM, WORM_TOOTH, x, y);
		stackobj(fobj);
	} else
#endif	NOWORM
	if(!letter(tmp) || !rn2(3)) tmp = 0;

	if(levl[x][y].typ >= DOOR)	/* might be mimic in wall */
	    if(x != u.ux || y != u.uy) /* might be here after swallowed */
		if(index("NTVm&",mdat->mlet) || rn2(5)) {
		mkobj_at(tmp,x,y);
		if(cansee(x,y)) atl(x,y,fobj->olet);
		stackobj(fobj);
	}
}

kludge(str,arg)
register char *str,*arg;
{
	if(Blind) {
		if(*str == '%') pline(str,"It");
		else pline(str,"it");
	} else pline(str,arg);
}

rescham()	/* force all chameleons to become normal */
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->cham) {
			mtmp->cham = 0;
			(void) newcham(mtmp,PM_CHAM);
		}
}

newcham(mtmp,mdat)	/* make a chameleon look like a new monster */
			/* returns 1 if the monster actually changed */
register struct monst *mtmp;
register struct permonst *mdat;
{
	register mhp, hpn, hpd;

	if(mdat == mtmp->data) return(0);	/* still the same monster */
#ifndef NOWORM
	if(mtmp->wormno) wormdead(mtmp);	/* throw tail away */
#endif NOWORM
	hpn = mtmp->mhp;
	hpd = (mtmp->data->mlevel)*8;
	if(!hpd) hpd = 4;
	mtmp->data = mdat;
	mhp = (mdat->mlevel)*8;
	/* new hp: same fraction of max as before */
	mtmp->mhp = 2 + (hpn*mhp)/hpd;
	hpn = mtmp->orig_hp;
	mtmp->orig_hp = 2 + (hpn*mhp)/hpd;
	mtmp->minvis = (mdat->mlet == 'I') ? 1 : 0;
#ifndef NOWORM
	if(mdat->mlet == 'w' && getwn(mtmp)) initworm(mtmp);
#endif NOWORM
	unpmon(mtmp);	/* necessary for 'I' and to force pmon */
	pmon(mtmp);
	return(1);
}

mnexto(mtmp)	/* Make monster mtmp next to you (if possible) */
struct monst *mtmp;
{
	extern coord enexto();
	coord mm;
	mm = enexto(u.ux, u.uy);
	mtmp->mx = mm.x;
	mtmp->my = mm.y;
	pmon(mtmp);
}

rloc(mtmp)
struct monst *mtmp;
{
	register tx,ty;
	register char ch = mtmp->data->mlet;

#ifndef NOWORM
	if(ch == 'w' && mtmp->mx) return;	/* do not relocate worms */
#endif NOWORM
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

ishuman(mtmp) register struct monst *mtmp; {
	return(mtmp->data->mlet == '@');
}

setmangry(mtmp) register struct monst *mtmp; {
	if(!mtmp->mpeaceful) return;
	if(mtmp->mtame) return;
	mtmp->mpeaceful = 0;
	if(ishuman(mtmp)) pline("%s gets angry!", Monnam(mtmp));
}
