/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#ifndef QUEST
#include "mklev.h"
#include "def.eshk.h"
#define	ESHK	((struct eshk *)(&(shk->mextra[0])))
extern struct monst *makemon();

char shtypes[] = "=/)%?!["; /* 8 shoptypes: 7 specialised, 1 mixed */
schar shprobs[] = { 3,3,5,5,10,10,14,50 };	/* their probabilities */

mkshop(){
register struct mkroom *sroom;
register int sh,sx,sy,i;
register char let;
int roomno;
register struct monst *shk;
	for(sroom = &rooms[0], roomno = 0; ; sroom++, roomno++){
		if(sroom->hx < 0) return;
		if(sroom->lx <= xdnstair && xdnstair <= sroom->hx &&
		   sroom->ly <= ydnstair && ydnstair <= sroom->hy) continue;
		if(sroom->lx <= xupstair && xupstair <= sroom->hx &&
		   sroom->ly <= yupstair && yupstair <= sroom->hy) continue;
		if(
#ifdef WIZARD
		   wizard ||
#endif WIZARD
			sroom->doorct == 1) break;
	}
#ifdef WIZARD
	if(wizard){
		extern char *getenv();
		register char *ep = getenv("SHOPTYPE");
		if(ep){
			if(*ep == 'z' || *ep == 'Z'){
				mkzoo();
				return;
			}
			for(i=0; shtypes[i]; i++)
				if(*ep == shtypes[i]) break;
			let = i;
			goto gotlet;
		}
	}
#endif WIZARD
	for(i = rn2(100),let = 0; (i -= shprobs[let])>= 0; let++)
		if(!shtypes[let]) break;	/* superfluous */
#ifdef WIZARD
gotlet:
#endif WIZARD
	sroom->rtype = 8+let;
	let = shtypes[let];
	sh = sroom->fdoor;
	sx = doors[sh].x;
	sy = doors[sh].y;
	if(sx == sroom->lx-1) sx++; else
	if(sx == sroom->hx+1) sx--; else
	if(sy == sroom->ly-1) sy++; else
	if(sy == sroom->hy+1) sy--; else {
		printf("Where is shopdoor?");
		return;
	}
	if(!(shk = makemon(PM_SHK,sx,sy))) return;
	shk->isshk = shk->mpeaceful = 1;
	shk->msleep = 0;
	shk->mtrapseen = ~0;	/* we know all the traps already */
	ESHK->shoproom = roomno;
	ESHK->shd = doors[sh];
	ESHK->shk.x = sx;
	ESHK->shk.y = sy;
	ESHK->robbed = 0;
	ESHK->visitct = 0;
	shk->mgold = 1000 + 30*rnd(100);	/* initial capital */
	ESHK->billct = 0;
	findname(ESHK->shknam, let);
	for(sx = sroom->lx; sx <= sroom->hx; sx++)
	for(sy = sroom->ly; sy <= sroom->hy; sy++){
		register struct monst *mtmp;
		if((sx == sroom->lx && doors[sh].x == sx-1) ||
		   (sx == sroom->hx && doors[sh].x == sx+1) ||
		   (sy == sroom->ly && doors[sh].y == sy-1) ||
		   (sy == sroom->hy && doors[sh].y == sy+1)) continue;
		if(rn2(100) < dlevel && !m_at(sx,sy) &&
		   (mtmp = makemon(PM_MIMIC, sx, sy))){
			mtmp->mimic =
			    (let && rn2(10) < dlevel) ? let : ']';
			continue;
		}
 mkobj_at(let, sx, sy);
	}
#ifdef WIZARD
	if(wizard) printf("I made a %c-shop.", let ? let : 'g');
#endif WIZARD
}

mkzoo(){
register struct mkroom *sroom;
register int sh,sx,sy,i;
int goldlim = 500 * dlevel;
	for(sroom = &rooms[0]; ; sroom++){
		if(sroom->hx < 0) return;
		if(sroom->lx <= xdnstair && xdnstair <= sroom->hx &&
		   sroom->ly <= ydnstair && ydnstair <= sroom->hy) continue;
		if(sroom->lx <= xupstair && xupstair <= sroom->hx &&
		   sroom->ly <= yupstair && yupstair <= sroom->hy) continue;
		if(sroom->doorct == 1) break;
	}
	sroom->rtype = 7;
	sh = sroom->fdoor;
	for(sx = sroom->lx; sx <= sroom->hx; sx++)
	for(sy = sroom->ly; sy <= sroom->hy; sy++){
		if((sx == sroom->lx && doors[sh].x == sx-1) ||
		   (sx == sroom->hx && doors[sh].x == sx+1) ||
		   (sy == sroom->ly && doors[sh].y == sy-1) ||
		   (sy == sroom->hy && doors[sh].y == sy+1)) continue;
		(void) makemon((struct permonst *) 0,sx,sy);
		i = sq(dist2(sx,sy,doors[sh].x,doors[sh].y));
		if(i >= goldlim) i = 5*dlevel;
		goldlim -= i;
		mkgold(10 + rn2(i), sx, sy);
	}
#ifdef WIZARD
	if(wizard) printf("I made a zoo.");
#endif WIZARD
}

dist2(x0,y0,x1,y1){
	return((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1));
}

sq(a) int a; {
	return(a*a);
}
#endif QUEST
