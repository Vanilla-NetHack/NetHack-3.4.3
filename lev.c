/*	SCCS Id: @(#)lev.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* lev.c - version 1.0.3 */

#include <stdio.h>
#include "hack.h"
#include "mkroom.h"
extern struct monst *restmonchn();
extern struct obj *restobjchn();
extern struct obj *billobjs;
extern char *itoa();
extern char SAVEF[];
extern int hackpid;
extern xchar dlevel;
extern char nul[];

#ifndef NOWORM
#include	"wseg.h"
extern struct wseg *wsegs[32], *wheads[32];
extern long wgrowtime[32];
#endif

#ifdef DGK
struct finfo fileinfo[MAXLEVEL+1];
long bytes_counted;
int count_only;
#else
boolean level_exists[MAXLEVEL+1];
#endif

#ifdef DGK
savelev(fd, lev, mode)
int fd, mode;
xchar lev;
{
	if (mode & COUNT) {
		count_only = TRUE;
		bytes_counted = 0;
		savelev0(fd, lev);
		while (bytes_counted > freediskspace(levels))
			if (!swapout_oldest())
				return FALSE;
	}
	if (mode & WRITE) {
		count_only = FALSE;
		bytes_counted = 0;
		savelev0(fd, lev);
	}
	fileinfo[lev].where = ACTIVE;
	fileinfo[lev].time = moves;
	fileinfo[lev].size = bytes_counted;
	return TRUE;
}

savelev0(fd,lev)
#else
savelev(fd,lev)
#endif
int fd;
xchar lev;
{
#ifndef NOWORM
	register struct wseg *wtmp, *wtmp2;
	register tmp;
#endif

	if(fd < 0) panic("Save on bad file!");	/* impossible */
#ifndef DGK
	if(lev >= 0 && lev <= MAXLEVEL)
		level_exists[lev] = TRUE;
#endif
	bwrite(fd,(char *) &hackpid,sizeof(hackpid));
	bwrite(fd,(char *) &lev,sizeof(lev));
	bwrite(fd,(char *) levl,sizeof(levl));
#ifdef DGK
	bwrite(fd, (char *) &symbol, sizeof(symbol));
#endif
	bwrite(fd,(char *) &moves,sizeof(long));
	bwrite(fd,(char *) &xupstair,sizeof(xupstair));
	bwrite(fd,(char *) &yupstair,sizeof(yupstair));
	bwrite(fd,(char *) &xdnstair,sizeof(xdnstair));
	bwrite(fd,(char *) &ydnstair,sizeof(ydnstair));
	savemonchn(fd, fmon);
	savegoldchn(fd, fgold);
	savetrapchn(fd, ftrap);
	saveobjchn(fd, fobj);
	saveobjchn(fd, billobjs);
	billobjs = 0;
	save_engravings(fd);
#ifndef QUEST
	bwrite(fd,(char *) rooms,sizeof(rooms));
	bwrite(fd,(char *) doors,sizeof(doors));
#endif
	fgold = 0;
	ftrap = 0;
	fmon = 0;
	fobj = 0;
#ifndef NOWORM
	bwrite(fd,(char *) wsegs,sizeof(wsegs));
	for(tmp=1; tmp<32; tmp++){
		for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp2){
			wtmp2 = wtmp->nseg;
			bwrite(fd,(char *) wtmp,sizeof(struct wseg));
		}
#ifdef DGK
		if (!count_only)
#endif
			wsegs[tmp] = 0;
	}
	bwrite(fd,(char *) wgrowtime,sizeof(wgrowtime));
#endif /* NOWORM /**/
#ifdef DGK
	if (count_only)	return(0);
#endif
	billobjs = 0;
	fgold = 0;
	ftrap = 0;
	fmon = 0;
	fobj = 0;
}

bwrite(fd,loc,num)
register fd;
register char *loc;
register unsigned num;
{
#ifdef DGK
	bytes_counted += num;
	if (!count_only)
#endif
/* lint wants the 3rd arg of write to be an int; lint -p an unsigned */
	    if(write(fd, loc, (int) num) != num)
		panic("cannot write %u bytes to file #%d", num, fd);
}

saveobjchn(fd,otmp)
register fd;
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
#ifdef DGK
		if (!count_only)
#endif
			free((char *) otmp);
		otmp = otmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}

#ifdef MSDOS
/* We don't want to save any pointers in any files, so convert
 * the pointers to indices before writing the monsters to disk -dgk
 */
savemonchn(fd,mtmp)
register fd;
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	unsigned xl;
	int minusone = -1;
	struct permonst *permonstp;
	int monsindex;
	extern struct permonst li_dog, dog, la_dog;
#ifdef KAA
	extern struct permonst hell_hound;
# ifdef HARD
	extern struct permonst d_lord, d_prince;
# endif
#endif

	while(mtmp) {
		mtmp2 = mtmp->nmon;
		xl = mtmp->mxlth + mtmp->mnamelth;
		bwrite(fd, (char *) &xl, sizeof(int));
		/* store an index where the pointer used to be */
		permonstp = mtmp->data;
		if (permonstp == &li_dog)
			monsindex = -1;		/* fake index */
		else if (permonstp == &dog)
			monsindex = -2;		/* fake index */
		else if (permonstp == &la_dog)
			monsindex = -3;		/* fake index */
#ifdef KAA
		else if (permonstp == &hell_hound)
			monsindex = -4;
# ifdef HARD
		else if (permonstp == &d_lord)
			monsindex = -5;

		else if (permonstp == &d_prince)
			monsindex = -6;
# endif
#endif
		else			
			monsindex = permonstp - &mons[0];
		*((int *)&mtmp->data) = monsindex;
		bwrite(fd, (char *) mtmp, xl + sizeof(struct monst));
		mtmp->data = permonstp;		/* restore the pointer */
		if(mtmp->minvent) saveobjchn(fd,mtmp->minvent);
#ifdef DGK
		if (!count_only)
#endif
			free((char *) mtmp);
		mtmp = mtmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}
#else

savemonchn(fd,mtmp)
register fd;
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	unsigned xl;
	int minusone = -1;
	struct permonst *monbegin = &mons[0];

	bwrite(fd, (char *) &monbegin, sizeof(monbegin));

	while(mtmp) {
		mtmp2 = mtmp->nmon;
		xl = mtmp->mxlth + mtmp->mnamelth;
		bwrite(fd, (char *) &xl, sizeof(int));
		bwrite(fd, (char *) mtmp, xl + sizeof(struct monst));
		if(mtmp->minvent) saveobjchn(fd,mtmp->minvent);
		free((char *) mtmp);
		mtmp = mtmp2;
	}
	bwrite(fd, (char *) &minusone, sizeof(int));
}
#endif

savegoldchn(fd,gold)
register fd;
register struct gold *gold;
{
	register struct gold *gold2;
	while(gold) {
		gold2 = gold->ngold;
		bwrite(fd, (char *) gold, sizeof(struct gold));
#ifdef DGK
		if (!count_only)
#endif
			free((char *) gold);
		gold = gold2;
	}
	bwrite(fd, nul, sizeof(struct gold));
}

savetrapchn(fd,trap)
register fd;
register struct trap *trap;
{
	register struct trap *trap2;
	while(trap) {
		trap2 = trap->ntrap;
		bwrite(fd, (char *) trap, sizeof(struct trap));
#ifdef DGK
		if (!count_only)
#endif
			free((char *) trap);
		trap = trap2;
	}
	bwrite(fd, nul, sizeof(struct trap));
}

getlev(fd,pid,lev)
int fd,pid;
xchar lev;
{
	register struct gold *gold;
	register struct trap *trap;
#ifndef NOWORM
	register struct wseg *wtmp;
#endif
	register tmp;
	long omoves;
	int hpid;
	xchar dlvl;
#ifdef DGK
	struct symbols osymbol;
	int x, y, up, dn, lt, rt;
	uchar osym, nsym;
#endif

#ifdef MSDOS
	setmode(fd,O_BINARY);
#endif
	/* First some sanity checks */
	mread(fd, (char *) &hpid, sizeof(hpid));
	mread(fd, (char *) &dlvl, sizeof(dlvl));
	if((pid && pid != hpid) || (lev && dlvl != lev)) {
		pline("Strange, this map is not as I remember it.");
		pline("Somebody is trying some trickery here ...");
		pline("This game is void ...");
		done("tricked");
	}

	fgold = 0;
	ftrap = 0;
	mread(fd, (char *) levl, sizeof(levl));
#ifdef DGK
	/* Corners are poorly implemented.  They only exist in the
	 * scrsym field of each dungeon element.  So we have to go
	 * through the previous level, looking for scrsym with the
	 * old corner values, checking to make sure that they are
	 * where corners should be, then replace them with the scrsym
	 * of the new GRAPHICS character set.  Ugly.
	 */
	mread(fd, (char *) &osymbol, sizeof(osymbol));
	if (memcmp((char *) &osymbol, (char *) &symbol, sizeof (symbol))) {
		for (x = 0; x < COLNO; x++)
			for (y = 0; y < ROWNO; y++) {
				osym = levl[x][y].scrsym;
				nsym = 0;
				switch (levl[x][y].typ) {
				case 0:
				case SCORR:
					break;
				case ROOM:
					if (osym == osymbol.room)
						nsym = symbol.room;
					break;
				case DOOR:
					if (osym == osymbol.door)
						nsym = symbol.door;
					break;
				case CORR:
					if (osym == osymbol.corr)
						nsym = symbol.corr;
					break;
				case VWALL:
					if (osym == osymbol.vwall)
						nsym = symbol.vwall;
					break;
				case SDOOR:
					if (osym == osymbol.vwall)
						nsym = symbol.vwall;
					else if (osym == osymbol.hwall)
						nsym = symbol.hwall;
					break;
				/* Now the ugly stuff */
				case HWALL:
				  up = (y > 0) ? levl[x][y-1].typ : 0;
				  dn = (y < ROWNO-1) ?levl[x][y+1].typ : 0;
				  lt = (x > 0) ? levl[x-1][y].typ : 0;
				  rt = (x < COLNO-1) ?levl[x+1][y].typ : 0;
				  up = up && (up == VWALL || up == DOOR
					|| up == SDOOR);
				  dn = dn && (dn == VWALL || dn == DOOR
					|| dn == SDOOR);
				  lt = lt && (lt == HWALL || lt == DOOR
					|| lt == SDOOR);
				  rt = rt && (rt == HWALL || rt == DOOR
					|| rt == SDOOR);
				  if (rt && dn && osym == osymbol.tlcorn)
					nsym = symbol.tlcorn;
				  else if (lt && dn && osym == osymbol.trcorn)
					nsym = symbol.trcorn;
				  else if (rt && up && osym == osymbol.blcorn)
					nsym = symbol.blcorn;
				  else if (lt && up && osym == osymbol.brcorn)
					nsym = symbol.brcorn;
				  else if (osym == osymbol.hwall)
					nsym = symbol.hwall;
				  break;
				default:
					break;
				}
				if (nsym)
					levl[x][y].scrsym = nsym;
			}
	}
#endif
	mread(fd, (char *)&omoves, sizeof(omoves));
	mread(fd, (char *)&xupstair, sizeof(xupstair));
	mread(fd, (char *)&yupstair, sizeof(yupstair));
	mread(fd, (char *)&xdnstair, sizeof(xdnstair));
	mread(fd, (char *)&ydnstair, sizeof(ydnstair));

	fmon = restmonchn(fd);

	/* regenerate animals while on another level */
	{ long tmoves = (moves > omoves) ? moves-omoves : 0;
	  register struct monst *mtmp, *mtmp2;
	  extern char genocided[];

	  for(mtmp = fmon; mtmp; mtmp = mtmp2) {
		long newhp;		/* tmoves may be very large */

		mtmp2 = mtmp->nmon;
		if(index(genocided, mtmp->data->mlet)) {
			mondead(mtmp);
			continue;
		}

		if(mtmp->mtame && tmoves > 250) {
			mtmp->mtame = 0;
			mtmp->mpeaceful = 0;
		}

		newhp = mtmp->mhp +
			(index(MREGEN, mtmp->data->mlet) ? tmoves : tmoves/20);
		if(newhp > mtmp->mhpmax)
			mtmp->mhp = mtmp->mhpmax;
		else
			mtmp->mhp = newhp;
	  }
	}

	setgd();
	gold = newgold();
	mread(fd, (char *)gold, sizeof(struct gold));
	while(gold->gx) {
		gold->ngold = fgold;
		fgold = gold;
		gold = newgold();
		mread(fd, (char *)gold, sizeof(struct gold));
	}
	free((char *) gold);
	trap = newtrap();
	mread(fd, (char *)trap, sizeof(struct trap));
	while(trap->tx) {
		trap->ntrap = ftrap;
		ftrap = trap;
		trap = newtrap();
		mread(fd, (char *)trap, sizeof(struct trap));
	}
	free((char *) trap);
	fobj = restobjchn(fd);
	billobjs = restobjchn(fd);
	rest_engravings(fd);
#ifndef QUEST
	mread(fd, (char *)rooms, sizeof(rooms));
	mread(fd, (char *)doors, sizeof(doors));
#endif
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
#endif
}

mread(fd, buf, len)
register fd;
register char *buf;
register unsigned len;
{
	register int rlen;
	extern boolean restoring;

	rlen = read(fd, buf, (int) len);
	if(rlen != len){
		pline("Read %d instead of %u bytes.\n", rlen, len);
		if(restoring) {
			(void) unlink(SAVEF);
			error("Error restoring old game.");
		}
		panic("Error reading level file.");
	}
}

mklev()
{
	extern boolean in_mklev;

	if(getbones()) return;

	in_mklev = TRUE;
	makelevel();
	in_mklev = FALSE;
}

#ifdef DGK
swapin_file(lev) {
	char to[PATHLEN], from[PATHLEN];

	sprintf(from, "%s%s", permbones, alllevels);
	sprintf(to, "%s%s", levels, alllevels);
	name_file(from, lev);
	name_file(to, lev);
	while (fileinfo[lev].size > freediskspace(to)) 
		if (!swapout_oldest())
			return FALSE;
#ifdef WIZARD
	if (wizard) {
		pline("Swapping in `%s'", from);
		fflush(stdout);
	}
#endif
	copyfile(from, to);
	(void) unlink(from);
	fileinfo[lev].where = ACTIVE;
	return TRUE;
}


swapout_oldest() {
	char to[PATHLEN], from[PATHLEN];
	int i, oldest;
	long oldtime;

	if (!ramdisk)
		return FALSE;
	for (i = 1, oldtime = 0, oldest = 0; i <= maxdlevel; i++)
		if (fileinfo[i].where == ACTIVE
		&& (!oldtime || fileinfo[i].time < oldtime)) {
			oldest = i;
			oldtime = fileinfo[i].time;
		}
	if (!oldest)
		return FALSE;
	sprintf(from, "%s%s", levels, alllevels);
	sprintf(to, "%s%s", permbones, alllevels);
	name_file(from, oldest);
	name_file(to, oldest);
#ifdef WIZARD
	if (wizard) {
		pline("Swapping out `%s'.", from);
		fflush(stdout);
	}
#endif
	copyfile(from, to);
	unlink(from);
	fileinfo[oldest].where = SWAPPED;
	return TRUE;
}

copyfile(from, to)
char *from, *to;
{
	char buf[BUFSIZ];
	int nfrom, nto, fdfrom, fdto;

	if ((fdfrom = open(from, O_RDONLY | O_BINARY | O_CREAT, FMASK)) < 0)
		panic("Can't copy from %s !?", from);
	if ((fdto = open(to, O_WRONLY | O_BINARY | O_CREAT, FMASK)) < 0)
		panic("Can't copy to %s", to);
	do {
		nfrom = read(fdfrom, buf, BUFSIZ);
		nto = write(fdto, buf, nfrom);
		if (nto != nfrom)
			panic("Copyfile failed!");
	} while (nfrom == BUFSIZ);
	close(fdfrom);
	close(fdto);
}
#endif
