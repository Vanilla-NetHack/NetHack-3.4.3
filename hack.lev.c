/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#include <signal.h>
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

#include "savelev.h"

getlev(fd)
{
	register struct gen *gtmp;
#ifndef NOWORM
	register struct wseg *wtmp;
#endif NOWORM
	register tmp;
	long omoves;

	if(fd<0 || read(fd, (char *) levl, sizeof(levl)) != sizeof(levl))
		return(1);
	fgold = 0;
	ftrap = 0;
	mread(fd, (char *)&omoves, sizeof(omoves));	/* 0 from MKLEV */
	mread(fd, (char *)&xupstair, sizeof(xupstair));
	mread(fd, (char *)&yupstair, sizeof(yupstair));
	mread(fd, (char *)&xdnstair, sizeof(xdnstair));
	mread(fd, (char *)&ydnstair, sizeof(ydnstair));

	fmon = restmonchn(fd);
	if(omoves) {
	    /* regenerate animals while on another level */
	    long tmoves = (moves > omoves) ? moves-omoves : 0;
	    register struct monst *mtmp, *mtmp2;
	    extern char genocided[];

	    for(mtmp = fmon; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if(index(genocided, mtmp->data->mlet)) {
			mondead(mtmp);
			continue;
		}
		if(index("ViT", mtmp->data->mlet))
			mtmp->mhp += tmoves;
		else
			mtmp->mhp += tmoves/20;
		if(mtmp->mhp > mtmp->orig_hp)
			mtmp->mhp = mtmp->orig_hp;
	    }
	}

	setshk();
	setgd();
	gtmp = newgen();
	mread(fd, (char *)gtmp, sizeof(struct gen));
	while(gtmp->gx) {
		gtmp->ngen = fgold;
		fgold = gtmp;
		gtmp = newgen();
		mread(fd, (char *)gtmp, sizeof(struct gen));
	}
	mread(fd, (char *)gtmp, sizeof(struct gen));
	while(gtmp->gx) {
		gtmp->ngen = ftrap;
		ftrap = gtmp;
		gtmp = newgen();
		mread(fd, (char *)gtmp, sizeof(struct gen));
	}
	free((char *) gtmp);
	fobj = restobjchn(fd);
	billobjs = restobjchn(fd);
	rest_engravings(fd);
#ifndef QUEST
	mread(fd, (char *)rooms, sizeof(rooms));
	mread(fd, (char *)doors, sizeof(doors));
#endif QUEST
	if(!omoves) return(0);	/* from MKLEV */
#ifndef NOWORM
	mread(fd, (char *)wsegs, sizeof(wsegs));
	for(tmp = 1; tmp < 32; tmp++) if(wsegs[tmp]){
		wheads[tmp] = wsegs[tmp] = wtmp = newseg();
		while(1) {
			mread(fd, (char *)wtmp, sizeof(struct wseg));
			if(!wtmp->nseg) break;
			wheads[tmp]->nseg = wtmp = newseg();
			wheads[tmp] = wtmp;
		}
	}
	mread(fd, (char *)wgrowtime, sizeof(wgrowtime));
#endif NOWORM
	return(0);
}

mread(fd, buf, len)
register fd;
register char *buf;
register unsigned len;
{
register int rlen;
	rlen = read(fd, buf, (int) len);
	if(rlen != len){
		pline("Read %d instead of %d bytes\n", rlen, len);
		panic("Cannot read %d bytes from file #%d\n", len, fd);
	}
}

#ifdef BSD
#include	<sys/wait.h>
#else
#include	<wait.h>
#endif BSD

mklev()
{
	register int fd;
	char type[2];
	union wait status;
	extern char fut_geno[];

	if(getbones()) return;
	if(dlevel < rn1(3, 26)) type[0] = 'a';	/* normal level */
	else type[0] = 'b';			/* maze */
	type[1] = 0;
	switch(fork()){
	case 0:
		(void) signal(SIGINT, SIG_IGN);
		(void) signal(SIGQUIT, SIG_IGN);
		execl("./mklev", "mklev", lock, type, itoa(dlevel), fut_geno, 
#ifdef WIZARD
			wizard ? "w" :
#endif WIZARD
					"", (char *) 0);
		exit(2);
	case -1:
		settty("Cannot fork!\n");
		exit(1);
	default:
		(void) fflush(stdout);	/* You fell into a trap ... */
		(void) wait(&status);
	}
	if(status.w_status) {
		if(status.w_coredump) {
			settty("Mklev dumped core. Exiting...\n");
			exit(1);
		}
		if(status.w_termsig) {
			settty("Mklev killed by a signal. Exiting...\n");
			exit(1);
		}
		if(status.w_retcode) {
			if(status.w_retcode == 2) {
				settty("Cannot execl mklev.\n");
				exit(1);
			}
			pline("Mklev failed. Let's try again.");
			mklev();
			return;
		}
	}
	if((fd = open(lock, 0)) < 0) {
		pline("Can't open %s!", lock);
		mklev();
		return;
	}
	(void) getlev(fd);
	(void) close(fd);
}
