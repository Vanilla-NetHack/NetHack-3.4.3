/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* savelev.h version 1.0.1 - also save engravings from MKLEV */

#include "hack.h"
#include <stdio.h>
extern struct monst *restmonchn();
extern struct obj *restobjchn();
extern struct obj *billobjs;
extern char *itoa();

extern char nul[];
#ifndef NOWORM
#include	"def.wseg.h"

extern struct wseg *wsegs[32], *wheads[32];
extern long wgrowtime[32];
#endif NOWORM

savelev(fd){
#ifndef NOWORM
	register struct wseg *wtmp, *wtmp2;
	register int tmp;
#endif NOWORM

	if(fd < 0)
           panic("Save on bad file!");

	bwrite(fd,(char *) levl,sizeof(levl));
	bwrite(fd,(char *) &moves,sizeof(long));
	bwrite(fd,(char *) &xupstair,sizeof(xupstair));
	bwrite(fd,(char *) &yupstair,sizeof(yupstair));
	bwrite(fd,(char *) &xdnstair,sizeof(xdnstair));
	bwrite(fd,(char *) &ydnstair,sizeof(ydnstair));
	savemonchn(fd, fmon);
	savegenchn(fd, fgold);
	savegenchn(fd, ftrap);
	saveobjchn(fd, fobj);
	saveobjchn(fd, billobjs);
/*	if (!ismklev) */
	   billobjs = 0;
#ifndef QUEST
	bwrite(fd,(char *) rooms,sizeof(rooms));
	bwrite(fd,(char *) doors,sizeof(doors));
#endif QUEST
	save_engravings(fd);
/* 	if (!ismklev) */
	   {
	   fgold = ftrap = 0;
	   fmon = 0;
	   fobj = 0;
	   }
/*--------------------------------------------------------------------*/
#ifndef NOWORM
	bwrite(fd,(char *) wsegs,sizeof(wsegs));
	for(tmp=1; tmp<32; tmp++){
		for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp2){
			wtmp2 = wtmp->nseg;
			bwrite(fd,(char *) wtmp,sizeof(struct wseg));
		}
/*		if (!ismklev) */
		   wsegs[tmp] = 0;
	}
	bwrite(fd,(char *) wgrowtime,sizeof(wgrowtime));
#endif NOWORM
/*--------------------------------------------------------------------*/
}

bwrite(fd,loc,num)
register int fd;
register char *loc;
register unsigned num;
{
/* lint wants the 3rd arg of write to be an int; lint -p an unsigned */
	if(write(fd, loc, (int) num) != num)
		panic("cannot write %d bytes to file #%d",num,fd);
}

saveobjchn(fd,otmp)
register int fd;
register struct obj *otmp;
{
	register struct obj *otmp2;
	unsigned xl;
	int minusone = -1;

	while(otmp) {
		otmp2 = otmp->nobj;
		xl = otmp->onamelth;
		bwrite(fd, (char *) &xl, sizeof(int));
		bwrite(fd, (char *) otmp, xl + sizeof(struct obj));
/*		if (!ismklev) */
			free((char *) otmp);
		otmp = otmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}

savemonchn(fd,mtmp)
register int fd;
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	unsigned xl;
	int minusone = -1;
	int monnum;
#ifdef FUNNYRELOC
	struct permonst *monbegin = &mons[0];

	bwrite(fd, (char *) &monbegin, sizeof(monbegin));
#endif

	while(mtmp) {
		mtmp2 = mtmp->nmon;
		xl = mtmp->mxlth + mtmp->mnamelth;
		bwrite(fd, (char *) &xl, sizeof(int));

		/* JAT - just save the offset into the monster table, */
		/* it will be relocated when read in */
		monnum = mtmp->data - &mons[0];
		mtmp->data = (struct permonst *)monnum;
#ifdef DEBUGMON
		myprintf("Wrote monster #%d", monnum);
#endif
		bwrite(fd, (char *) mtmp, xl + sizeof(struct monst));
		if(mtmp->minvent) saveobjchn(fd,mtmp->minvent);
/*		if (!ismklev) */
		   free((char *) mtmp);
		mtmp = mtmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}

savegenchn(fd,gtmp)
register int fd;
register struct gen *gtmp;
{
	register struct gen *gtmp2;
	while(gtmp) {
		gtmp2 = gtmp->ngen;
		bwrite(fd, (char *) gtmp, sizeof(struct gen));
/*		if (!ismklev) */
		   free((char *) gtmp);
		gtmp = gtmp2;
	}
	bwrite(fd, nul, sizeof(struct gen));
}


