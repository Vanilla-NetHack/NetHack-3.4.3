/*	SCCS Id: @(#)monmove.c	2.1	87/10/18
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include "hack.h"
#include "mfndpos.h"
#define	NULL	(char *) 0

extern int warnlevel;	/* defined in mon.c */

dochugw(mtmp) register struct monst *mtmp; {
register x = mtmp->mx;
register y = mtmp->my;
register d = dochug(mtmp);
register dd;
	if(!d)		/* monster still alive */
	if(Warning)
	if(!mtmp->mpeaceful)
	if(mtmp->data->mlevel > warnlevel)
	if((dd = dist(mtmp->mx,mtmp->my)) < dist(x,y))
	if(dd < 100)
	if(!canseemon(mtmp))
		warnlevel = mtmp->data->mlevel;
	return(d);
}

/* returns 1 if monster died moving, 0 otherwise */
dochug(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat;
	register tmp, nearby, scared, onscary;

	if(mtmp->cham && !rn2(6))
		(void) newcham(mtmp, &mons[dlevel+14+rn2(CMNUM-14-dlevel)]);
	mdat = mtmp->data;
	if(mdat->mlevel < 0)
		panic("bad monster %c (%d)",mdat->mlet,mdat->mlevel);

	/* regenerate monsters */
	if((!(moves%20) || index(MREGEN, mdat->mlet)) &&
	    mtmp->mhp < mtmp->mhpmax)
		mtmp->mhp++;

	if(mtmp->mfroz) {
		if (Hallucination) pmon(mtmp);
		return(0);	/* frozen monsters don't do anything */
	}

	if(mtmp->msleep)	/* there is a chance we will wake it */
		if(!disturb(mtmp)) return(0);

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

	/* fleeing monsters might regain courage */
	if(mtmp->mflee && !mtmp->mfleetim
	    && mtmp->mhp == mtmp->mhpmax && !rn2(25))
		mtmp->mflee = 0;

	nearby = (dist(mtmp->mx, mtmp->my) < 3);
	onscary = (sengr_at("Elbereth", u.ux, u.uy) ||
			sobj_at(SCR_SCARE_MONSTER, u.ux, u.uy));
	scared = (nearby && onscary && !mtmp->mtame && mtmp->mcansee)
		 && (mdat->mlet != '1');  /* RPH: the wiz is never scared */
	if(scared && !mtmp->mflee) {
		mtmp->mflee = 1;
		mtmp->mfleetim = (rn2(7) ? rnd(10) : rnd(100));
	}

	if(!nearby ||
		mtmp->mflee || scared ||
		mtmp->mconf ||
		(mtmp->minvis && !rn2(3)) ||
#ifndef KOPS
		(index("BIuy", mdat->mlet) && !rn2(4)) ||
#else
		(index("KBIuy", mdat->mlet) && !rn2(4)) ||
#endif
		(mdat->mlet == 'L' && !u.ugold && (mtmp->mgold || rn2(2))) ||
		(!mtmp->mcansee && !rn2(4)) ||
		mtmp->mpeaceful
	   ) {
		tmp = m_move(mtmp,0);	/* 2: monster died moving */
		if(tmp == 2 || (tmp && mdat->mmove <= 12))
			return(tmp == 2);

		if(Hallucination && tmp==0) pmon(mtmp);
/* If 0, this means the monster didn't move.  During hallucination, its
   appearance should still change. */

#ifdef HARD
		/* Without this line, fast monsters don't hit you when they've
		 * caught up to you. -dgk
		 */
		nearby = (dist(mtmp->mx, mtmp->my) < 3);
		scared = (nearby && onscary);
		if(scared && !mtmp->mflee) {
			mtmp->mflee = 1;
			mtmp->mfleetim = (rn2(7) ? rnd(10) : rnd(100));
		}
#endif
	}
#ifdef HARD	/* Demonic Blackmail!!! */
	if(mdat->mlet == '&' && mtmp->mpeaceful && !mtmp->mtame)
		if(demon_talk(mtmp))
			 return(1);	/* you paid it off */
#endif
	if(!index("Ea", mdat->mlet) && nearby &&
	 !mtmp->mpeaceful && u.uhp > 0 && !scared) {
		if(mhitu(mtmp))
			return(1);	/* monster died (e.g. 'y' or 'F') */
	}
	/* extra movement for fast monsters */
	if(mdat->mmove-12 > rnd(12)) tmp = m_move(mtmp,1);
	return(tmp == 2);
}

m_move(mtmp,after)
register struct monst *mtmp;
{
#ifndef REGBUG
	register
#endif
		 struct monst *mtmp2;
#ifndef REGBUG
	register
#endif
		int nx,ny,omx,omy,appr,nearer,cnt,i,j;
	xchar gx,gy,nix,niy,chcnt;
	schar chi;
	boolean likegold, likegems, likeobjs;
#ifdef KAA
	boolean likerock;
#endif
	char msym = mtmp->data->mlet;
	schar mmoved = 0;	/* not strictly nec.: chi >= 0 will do */
	coord poss[9];
	long info[9];

	if(mtmp->mfroz || mtmp->msleep)
		return(0);
	if(mtmp->mtrapped) {
		i = mintrap(mtmp);
		if(i == 2) return(2);	/* he died */
		if(i == 1) return(0);	/* still in trap, so didnt move */
	}
	if(mtmp->mhide && o_at(mtmp->mx,mtmp->my) && rn2(10))
		return(0);		/* do not leave hiding place */

#ifndef NOWORM
	if(mtmp->wormno)
		goto not_special;
#endif

	/* my dog gets a special treatment */
	if(mtmp->mtame) {
		return( dog_move(mtmp, after) );
	}

	/* likewise for shopkeeper */
	if(mtmp->isshk) {
		mmoved = shk_move(mtmp);
		if(mmoved >= 0)
			goto postmov;
		mmoved = 0;		/* follow player outside shop */
	}

	/* and for the guard */
	if(mtmp->isgd) {
		mmoved = gd_move();
		goto postmov;
	}

/* teleport if that lies in our nature ('t') or when badly wounded ('1') */
	if((msym == 't' && !rn2(5))
	|| (msym == '1' && (mtmp->mhp < 7 || (!xdnstair && !rn2(5))
		|| levl[u.ux][u.uy].typ == STAIRS))) {
		if(mtmp->mhp < 7 || (msym == 't' && rn2(2)))
			rloc(mtmp);
		else
			mnexto(mtmp);
		mmoved = 1;
		goto postmov;
	}

	/* spit fire ('D') or use a wand ('1') when appropriate */
#ifdef DGKMOD
	/* Add arrow and bolt throwing monsters */
	if (index(
# ifdef KAA
#  ifdef KOPS
		"D1OKC9",
#  else
		"D1KC9",
#  endif
# else
#  ifdef KOPS
		"D1OKC",
#  else
		"D1KC",
#  endif
# endif
			  msym))	

		if (!inrange(mtmp))	/* inrange returns 1 if OK for mon */
			return(0);	/* to move after it zaps or throws */
#else
	if(index("D1", msym))
		inrange(mtmp);
#endif

	if(msym == 'U' && !mtmp->mcan && canseemon(mtmp) &&
	    mtmp->mcansee && rn2(5)) {
		if(!Confusion)
			pline("%s's gaze has confused you!", Monnam(mtmp));
		else
			pline("You are getting more and more confused.");
		if(rn2(3)) mtmp->mcan = 1;
		HConfusion += d(3,4);		/* timeout */
	}
#ifdef RPH
	if (msym == '8' && canseemon(mtmp)) {
	    if (mtmp->mcan)
	        pline ("You notice that %s isn't all that ugly.",monnam(mtmp));
	    else if (rn2(3)) 
		pline ("You see the ugly back of %s.", monnam(mtmp));
  	    else {
	        pline ("You look upon %s.", monnam(mtmp));
		pline ("You turn to stone.");
		done_in_by(mtmp);
	    }
	}
#endif
not_special:
	if(!mtmp->mflee && u.uswallow && u.ustuck != mtmp) return(1);
	appr = 1;
	if(mtmp->mflee) appr = -1;
	if(mtmp->mconf || Invis ||  !mtmp->mcansee ||
		(index("BIy", msym) && !rn2(3)))
		appr = 0;
	omx = mtmp->mx;
	omy = mtmp->my;
	gx = u.ux;
	gy = u.uy;
	if(msym == 'L' && appr == 1 && mtmp->mgold > u.ugold)
		appr = -1;

	/* random criterion for 'smell' or track finding ability
	   should use mtmp->msmell or sth
	 */
	if(msym == '@' ||
#ifdef RPH
	  uwep && !strcmp(ONAME(uwep), "Excalibur") ||
#endif
	  ('a' <= msym && msym <= 'z')) {
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

	/* look for gold or jewels nearby */
#ifdef ROCKMOLE
	likegold = (index("LODr", msym) != NULL);
	likegems = (index("ODu", msym) != NULL);
# ifdef KJSMODS
	likeobjs = (mtmp->mhide || (msym == 'r' && dlevel > 3));
# else
	likeobjs = (mtmp->mhide || msym == 'r');
# endif
#else
	likegold = (index("LOD", msym) != NULL);
	likegems = (index("ODu", msym) != NULL);
	likeobjs = mtmp->mhide;
#endif
#ifdef KAA
	likerock = (msym == '9');
#endif
#define	SRCHRADIUS	25
	{ xchar mind = SRCHRADIUS;		/* not too far away */
	  register int dd;
	  if(likegold){
		register struct gold *gold;
		for(gold = fgold; gold; gold = gold->ngold)
		  if((dd = DIST(omx,omy,gold->gx,gold->gy)) < mind){
		    mind = dd;
		    gx = gold->gx;
		    gy = gold->gy;
		}
	  }
	  if(likegems || likeobjs
#ifdef KAA
				  || likerock
#endif
	    )  {
		register struct obj *otmp;
		for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(likeobjs
		   || (likegems && otmp->olet == GEM_SYM)
#ifdef KAA
		   || (likerock && otmp->olet == ROCK_SYM)
#endif
			)  {
			if(msym != 'u' || objects[otmp->otyp].g_val != 0)
			    if((dd = DIST(omx,omy,otmp->ox,otmp->oy)) < mind){
				mind = dd;
				gx = otmp->ox;
				gy = otmp->oy;
			    }
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
		msym == 'u' ? NOTONL :
#ifdef ROCKMOLE
# ifdef KJSMODS
		(msym == 'r' && dlevel > 3) ? ALLOW_WALL :
# else
		msym == 'r' ? ALLOW_WALL :
# endif
#endif
		(msym == '@' || msym == '1') ? (ALLOW_SSM | ALLOW_TRAPS) :
		index(UNDEAD, msym) ? NOGARLIC :
#ifdef KAA
		    (msym == '9') ? (ALLOW_ROCK | ALLOW_TRAPS) : ALLOW_TRAPS);
#else
		     ALLOW_TRAPS);
#endif
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
#endif
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
#endif
	} else {
		if(msym == 'u' && rn2(2)){
			rloc(mtmp);
			return(0);
		}
#ifndef NOWORM
		if(mtmp->wormno) worm_nomove(mtmp);
#endif
	}
postmov:
	if(mmoved == 1) {
		if(mintrap(mtmp) == 2)	/* he died */
			return(2);
#ifdef ROCKMOLE
	       /* Maybe a rock mole just ate something? */
	       if(msym == 'r'
# ifdef KJSMODS
		  && dlevel > 3
#endif
		  && IS_ROCK(levl[mtmp->mx][mtmp->my].typ) &&
		  levl[mtmp->mx][mtmp->my].typ != POOL){
		   register int pile = rnd(25);
		   /* Just ate something. */
		   if(levl[mtmp->mx][mtmp->my].typ == 0)
		     levl[mtmp->mx][mtmp->my].typ = CORR;
		   else if(IS_WALL(levl[mtmp->mx][mtmp->my].typ))
		     levl[mtmp->mx][mtmp->my].typ = DOOR;
		   mnewsym(mtmp->mx,mtmp->my);
		   /* Left behind a pile? */
		   if(pile < 5) {
		       if(pile == 1)
			mksobj_at(ENORMOUS_ROCK, mtmp->mx, mtmp->my);
		      else
			mksobj_at(ROCK, mtmp->mx, mtmp->my);
		   }
		  if(cansee(mtmp->mx, mtmp->my))
		    if(fobj)	atl(mtmp->mx,mtmp->my,fobj->olet);
	       }
	       /* Maybe a rock mole just ate some gold or armor? */
	       if(msym == 'r') meatgold(mtmp);
#endif /* ROCKMOLE /**/
		if(likegold) mpickgold(mtmp);
#ifdef KAA
		if(likerock || likegems) mpickgems(mtmp);
#else
		if(likegems) mpickgems(mtmp);
#endif
		if(mtmp->mhide) mtmp->mundetected = 1;
	}
	pmon(mtmp);
	return(mmoved);
}

