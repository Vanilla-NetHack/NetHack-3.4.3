/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
extern char plname[PL_NSIZ];
extern struct monst *makemon();

struct permonst pm_ghost = { "ghost", ' ', 10, 3, -5, 1, 1, sizeof(plname) };


char bones[] = "bones_xx";

/* save bones and possessions of a deceased adventurer */
savebones(){
register fd;
register struct obj *otmp;
register struct gen *gtmp;
register struct monst *mtmp;
	if(!rn2(1 + dlevel/2)) return;	/* not so many ghosts on low levels */
	bones[6] = '0' + (dlevel/10);
	bones[7] = '0' + (dlevel%10);
	if((fd = open(bones,0)) >= 0){
		(void) close(fd);
		return;
	}
	/* drop everything; the corpse's possessions are usually cursed */
	otmp = invent;
	while(otmp){
		otmp->ox = u.ux;
		otmp->oy = u.uy;
		otmp->known = 0;
		otmp->age = 0;		/* very long ago */
		otmp->owornmask = 0;
		if(rn2(5)) otmp->cursed = 1;
		if(!otmp->nobj){
			otmp->nobj = fobj;
			fobj = invent;
			invent = 0;	/* superfluous */
			break;
		}
 otmp = otmp->nobj;
	}
	if(!(mtmp = makemon(&pm_ghost, u.ux, u.uy))) return;
	mtmp->mx = u.ux;
	mtmp->my = u.uy;
	mtmp->msleep = 1;
	(void) strcpy((char *) mtmp->mextra, plname);
	mkgold(somegold() + d(dlevel,30), u.ux, u.uy);
	u.ux = FAR;		/* avoid animals standing next to us */
	keepdogs();		/* all tame animals become wild again */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		mtmp->mlstmv = 0;
		if(mtmp->mdispl) unpmon(mtmp);
	}
	for(gtmp = ftrap; gtmp; gtmp = gtmp->ngen)
		gtmp->gflag &= ~SEEN;
	for(otmp = fobj; otmp; otmp = otmp->nobj)
		otmp->onamelth = 0;
	if((fd = creat(bones, FMASK)) < 0) return;
	savelev(fd);
	(void) close(fd);
}

getbones(){
register fd,x,y,ok;
	if(rn2(3)) return(0);	/* only once in three times do we find bones */
	bones[6] = '0' + dlevel/10;
	bones[7] = '0' + dlevel%10;
	if((fd = open(bones, 0)) < 0) return(0);
	if((ok = uptodate(fd)) != 0){
		(void) getlev(fd);
		(void) close(fd);
		for(x = 0; x < COLNO; x++) for(y = 0; y < ROWNO; y++)
			levl[x][y].seen = levl[x][y].new = 0;
	}
	if(unlink(bones) < 0){
		pline("Cannot unlink %s", bones);
		return(0);
	}
 return(ok);
}
