/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
extern char genocided[60];	/* defined in Decl.c */
extern char fut_geno[60];	/* idem */
#include <signal.h>

extern char SAVEF[], nul[];
extern char pl_character[PL_CSIZ];
extern long lseek();
extern struct obj *restobjchn();
extern struct monst *restmonchn();

dosave(){
	if(dosave0(0)) {
		settty("Be seeing you ...\n");
		exit(0);
	}
#ifdef lint
	return(0);
#endif lint
}

#ifndef NOSAVEONHANGUP
hangup(){
	(void) dosave0(1);
	exit(1);
}
#endif NOSAVEONHANGUP

/* returns 1 if save successful */
dosave0(hu) int hu; {
	register fd, ofd;
	int tmp;		/* not register ! */

	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	if((fd = creat(SAVEF, FMASK)) < 0) {
		if(!hu) pline("Cannot open save file. (Continue or Quit)");
		return(0);
	}
	savelev(fd);
	saveobjchn(fd, invent);
	saveobjchn(fd, fcobj);
	savemonchn(fd, fallen_down);
	bwrite(fd, (char *) &flags, sizeof(struct flag));
	bwrite(fd, (char *) &dlevel, sizeof dlevel);
	bwrite(fd, (char *) &maxdlevel, sizeof maxdlevel);
	bwrite(fd, (char *) &moves, sizeof moves);
	bwrite(fd, (char *) &u, sizeof(struct you));
	bwrite(fd, (char *) pl_character, sizeof pl_character);
	bwrite(fd, (char *) genocided, sizeof genocided);
	bwrite(fd, (char *) fut_geno, sizeof fut_geno);
	savenames(fd);
	for(tmp = 1; tmp <= maxdlevel; tmp++) {
		glo(tmp);
		if((ofd = open(lock, 0)) < 0)
			continue;
		(void) getlev(ofd);
		(void) close(ofd);
		bwrite(fd, (char *) &tmp, sizeof tmp);	/* level number */
		savelev(fd);				/* actual level */
		(void) unlink(lock);
	}
	(void) close(fd);
	(*index(lock, '.')) = 0;
	(void) unlink(lock);
	return(1);
}

dorecover(fd)
register fd;
{
	register nfd;
	int tmp;		/* not a register ! */
	struct obj *otmp;

	(void) getlev(fd);
	invent = restobjchn(fd);
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->owornmask)
			setworn(otmp, otmp->owornmask);
	fcobj = restobjchn(fd);
	fallen_down = restmonchn(fd);
	mread(fd, (char *) &flags, sizeof(struct flag));
	mread(fd, (char *) &dlevel, sizeof dlevel);
	mread(fd, (char *) &maxdlevel, sizeof maxdlevel);
	mread(fd, (char *) &moves, sizeof moves);
	mread(fd, (char *) &u, sizeof(struct you));
	mread(fd, (char *) pl_character, sizeof pl_character);
	mread(fd, (char *) genocided, sizeof genocided);
	mread(fd, (char *) fut_geno, sizeof fut_geno);
	restnames(fd);
	while(1) {
		if(read(fd, (char *) &tmp, sizeof tmp) != sizeof tmp)
			break;
		if(getlev(fd))
			break;		/* this is actually an error */
		glo(tmp);
		if((nfd = creat(lock, FMASK)) < 0)
			panic("Cannot open temp file %s!\n", lock);
		savelev(nfd);
		(void) close(nfd);
	}
	(void) lseek(fd, 0L, 0);
	(void) getlev(fd);
	(void) close(fd);
	(void) unlink(SAVEF);
	if(Punished) {
		for(otmp = fobj; otmp; otmp = otmp->nobj)
			if(otmp->olet == CHAIN_SYM) goto chainfnd;
		panic("Cannot find the iron chain?");
	chainfnd:
		uchain = otmp;
		if(!uball){
			for(otmp = fobj; otmp; otmp = otmp->nobj)
				if(otmp->olet == BALL_SYM && otmp->spe)
					goto ballfnd;
			panic("Cannot find the iron ball?");
		ballfnd:
			uball = otmp;
		}
	}
#ifndef QUEST
	setsee();  /* only to recompute seelx etc. - these weren't saved */
#endif QUEST
	docrt();
}

struct obj *
restobjchn(fd)
register fd;
{
	register struct obj *otmp, *otmp2;
	register struct obj *first = 0;
	int xl;
#ifdef lint
	/* suppress "used before set" warning from lint */
	otmp2 = 0;
#endif lint
	while(1) {
		mread(fd, (char *) &xl, sizeof(xl));
		if(xl == -1) break;
		otmp = newobj(xl);
		if(!first) first = otmp;
		else otmp2->nobj = otmp;
		mread(fd, (char *) otmp, (unsigned) xl + sizeof(struct obj));
		if(!otmp->o_id)	/* from MKLEV */
			otmp->o_id = flags.ident++;
		otmp2 = otmp;
	}
	if(first && otmp2->nobj){
		pline("Restobjchn: error reading objchn.");
		impossible();
		otmp2->nobj = 0;
	}
 return(first);
}

struct monst *
restmonchn(fd)
register fd;
{
	register struct monst *mtmp, *mtmp2;
	register struct monst *first = 0;
	int xl;

	struct permonst *monbegin;
	long differ;

	mread(fd, (char *)&monbegin, sizeof(monbegin));
	differ = (char *)(&mons[0]) - (char *)(monbegin);

#ifdef lint
	/* suppress "used before set" warning from lint */
	mtmp2 = 0;
#endif lint
	while(1) {
		mread(fd, (char *) &xl, sizeof(xl));
		if(xl == -1) break;
		mtmp = newmonst(xl);
		if(!first) first = mtmp;
		else mtmp2->nmon = mtmp;
		mread(fd, (char *) mtmp, (unsigned) xl + sizeof(struct monst));
		mtmp->data = (struct permonst *)
			((char *) mtmp->data + differ);
		if(!mtmp->m_id) {			/* from MKLEV */
			mtmp->m_id = flags.ident++;
#ifndef NOWORM
			if(mtmp->data->mlet == 'w' && getwn(mtmp)){
				initworm(mtmp);
				mtmp->msleep = 0;
			}
 #endif NOWORM
		}
		if(mtmp->minvent)
			mtmp->minvent = restobjchn(fd);
		mtmp2 = mtmp;
	}
	if(first && mtmp2->nmon){
		pline("Restmonchn: error reading monchn.");
		impossible();
		mtmp2->nmon = 0;
	}
 return(first);
}
