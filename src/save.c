/*	SCCS Id: @(#)save.c	3.0	89/04/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"
#include "lev.h"

#ifdef WORM
#include "wseg.h"
#endif

#ifndef NO_SIGNAL
#include <signal.h>
#endif /* !NO_SIGNAL */
#if defined(EXPLORE_MODE) && !defined(LSC) && !defined(O_RDONLY)
#include <fcntl.h>
#endif /* EXPLORE_MODE */

boolean hu;		/* set during hang-up */

#if defined(DGK) && !defined(OLD_TOS)
struct finfo fileinfo[MAXLEVEL+1];
long bytes_counted;
int count_only;
#else
boolean level_exists[MAXLEVEL+1];
#endif

#if defined(DGK) && !defined(OLD_TOS)
static void savelev0();
#endif /* DGK && !OLD_TOS */
static void saveobjchn();
static void savemonchn();
static void savegoldchn();
static void savetrapchn();
static void savegenoinfo();
#if defined(DGK) && !defined(OLD_TOS)
static boolean swapout_oldest();
static void copyfile();
#endif /* defined(DGK) && !defined(OLD_TOS) */
static void spill_objs();

int
dosave(){
	clrlin();
	pline("Really save? ");	/* especially useful if COMPRESS defined */
	if(yn() == 'n') {
		clrlin();
		(void) fflush(stdout);
		if(multi > 0) nomul(0);
	} else {
#ifdef EXPLORE_MODE
# ifdef WIZARD
		if(!discover && !wizard) {
# else
		if(!discover) {
# endif
	pline("Do you want to create a non-scoring, restartable save file? ");
			if(yn() == 'y')  discover = TRUE;
		}
#endif
		clear_screen();
		(void) fflush(stdout);
		hu = FALSE;
		if(dosave0()) {
			settty("Be seeing you...\n");
			exit(0);
		} else (void)doredraw();
	}
	return 0;
}

#ifndef NOSAVEONHANGUP
int
hangup(){
	if (!hu)
	{
		hu = TRUE;
		(void) dosave0();
# ifndef VMS
		exit(1);
# endif
	}
	return 0;
}
#endif

/* returns 1 if save successful */
int
dosave0() {
	register int fd, ofd;
	int tmp;		/* not register ! */
	xchar ltmp;
#if defined(DGK) && !defined(OLD_TOS)
	long fds, needed;
	int mode;
#endif
#ifdef COMPRESS
	char	cmd[80];
#endif
#ifdef MACOS
	short	savenum;
#endif

	if (!SAVEF[0])
		return 0;

#if defined(UNIX) || defined(VMS)
	(void) signal(SIGHUP, SIG_IGN);
#endif
#if !defined(__TURBOC__) && !defined(OLD_TOS) && !defined(NO_SIGNAL)
	(void) signal(SIGINT, SIG_IGN);
#endif

#ifdef MSDOS
# ifdef DGK
	if(!hu && !saveDiskPrompt(0))	return 0;
# endif
# ifdef EXPLORE_MODE
	if(!hu) {

	    fd = open(SAVEF, O_RDONLY);
	    if (fd > 0) {
		(void) close(fd);
		clrlin();
		pline("There seems to be an old save file.  Overwrite it? ");
		if (yn() == 'n') return 0;
	    }
	}
# endif
# ifdef TOS
	fd = creat(SAVEF, FCMASK);
# else
	fd = open(SAVEF, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
# endif
#else /* MSDOS */
# ifdef EXPLORE_MODE
	if(!hu) {
	    fd = open(SAVEF, O_RDONLY);
	    if (fd > 0) {
		(void) close(fd);
		clrlin();
		pline("There seems to be an old save file.  Overwrite it? ");
		if (yn() == 'n') return 0;
	    }
	}
# endif
# ifdef MACOS
	{
		Str255	fileName;
		OSErr	er;
		OSType	fileType;
		Point	where;
		SFReply	reply;
		char	*prompt;
		
		savenum = 0;
		(void)GetVol(&fileName, &tmp);
		Strcpy((char *)&fileName[1], SAVEF);
		fileName[0] = strlen(SAVEF);
		where.h = where.v =
		    (SCREEN_BITS.bounds.bottom - SCREEN_BITS.bounds.top) / 4;
		prompt = "\022Save character in:";
		SFPutFile(where, prompt, fileName, 0L, &reply);
		if (reply.good) {
			SetVol(0L, savenum = reply.vRefNum);
			strncpy(SAVEF, (char *)&reply.fName[1],
					(short)reply.fName[0]);
			SAVEF[(short)reply.fName[0]] = '\0';
			Strcpy((char *)fileName, (char *)reply.fName);
		}
		
		fileType = (discover == TRUE) ? EXPLORE_TYPE : SAVE_TYPE;
		if (er = Create(&fileName, 0, CREATOR, fileType))
			SysBeep(1);
	}
	fd = open(SAVEF, O_WRONLY | O_BINARY);
# else
	fd = creat(SAVEF, FCMASK);
# endif /* MACOS */
#endif /* MSDOS */
	if(fd < 0) {
		if(!hu) pline("Cannot open save file.");
		(void) unlink(SAVEF);		/* ab@unido */
		return(0);
	}
#ifdef MACOS
	(void)SetVol(0L,tmp);
#endif
	if(flags.moonphase == FULL_MOON)	/* ut-sally!fletcher */
		change_luck(-1);		/* and unido!ab */
	home();
	cl_end();
#if defined(DGK) && !defined(OLD_TOS)
	if(!hu) msmsg("Saving: ");
	mode = COUNT;
again:
	savelev(fd, dlevel, mode);
	/* count_only will be set properly by savelev */
#else
# ifdef MACOS
	printf("Saving: ");
# endif
	savelev(fd,dlevel);
#endif
	saveobjchn(fd, invent);
	savemonchn(fd, fallen_down);
	savegenoinfo(fd);
	tmp = getuid();
	bwrite(fd, (genericptr_t) &tmp, sizeof tmp);
	bwrite(fd, (genericptr_t) &flags, sizeof(struct flag));
	bwrite(fd, (genericptr_t) &dlevel, sizeof dlevel);
	bwrite(fd, (genericptr_t) &maxdlevel, sizeof maxdlevel);
	bwrite(fd, (genericptr_t) &moves, sizeof moves);
	bwrite(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
	bwrite(fd, (genericptr_t) &wiz_level, sizeof wiz_level);
	bwrite(fd, (genericptr_t) &medusa_level, sizeof medusa_level);
	bwrite(fd, (genericptr_t) &bigroom_level, sizeof bigroom_level);
#ifdef ORACLE
	bwrite(fd, (genericptr_t) &oracle_level, sizeof oracle_level);
#endif
#ifdef REINCARNATION
	bwrite(fd, (genericptr_t) &rogue_level, sizeof rogue_level);
#endif
#ifdef STRONGHOLD
	bwrite(fd, (genericptr_t) &stronghold_level, sizeof stronghold_level);
	bwrite(fd, (genericptr_t) &tower_level, sizeof tower_level);
	bwrite(fd, (genericptr_t) tune, sizeof tune);
#  ifdef MUSIC
	bwrite(fd, (genericptr_t) &music_heard, sizeof music_heard);
#  endif
#endif
	bwrite(fd, (genericptr_t) &is_maze_lev, sizeof is_maze_lev);
	bwrite(fd, (genericptr_t) &u, sizeof(struct you));
#ifdef SPELLS
	bwrite(fd, (genericptr_t) spl_book, sizeof(struct spell) * (MAXSPELL + 1));
#endif
	if(u.ustuck)
		bwrite(fd, (genericptr_t) &(u.ustuck->m_id), sizeof u.ustuck->m_id);
	bwrite(fd, (genericptr_t) pl_character, sizeof pl_character);
#ifdef TUTTI_FRUTTI
	bwrite(fd, (genericptr_t) pl_fruit, sizeof pl_fruit);
	bwrite(fd, (genericptr_t) &current_fruit, sizeof current_fruit);
	savefruitchn(fd);
#endif
	savenames(fd);
#if defined(DGK) && !defined(OLD_TOS)
	if (mode == COUNT) {
# ifdef ZEROCOMP
		bflush(fd);
# endif
		/* make sure there is enough disk space */
		needed = bytes_counted;
		for (ltmp = 1; ltmp <= maxdlevel; ltmp++)
			if (ltmp != dlevel && fileinfo[ltmp].where)
				needed += fileinfo[ltmp].size + (sizeof ltmp);
		fds = freediskspace(SAVEF);
		if(needed > fds) {
		    if(!hu) {
			pline("There is insufficient space on SAVE disk.");
			pline("Require %ld bytes but only have %ld.", needed,
				fds);
		    }
		    flushout();
		    (void) close(fd);
		    (void) unlink(SAVEF);
		    return 0;
		}
		mode = WRITE;
		goto again;
	}
#endif
	for(ltmp = (xchar)1; ltmp <= maxdlevel; ltmp++) {
#if defined(DGK) && !defined(OLD_TOS)
		if (ltmp == dlevel || !fileinfo[ltmp].where) continue;
		if (fileinfo[ltmp].where != ACTIVE)
			swapin_file(ltmp);
#else
		if(ltmp == dlevel || !level_exists[ltmp]) continue;
#endif
		glo(ltmp);
#if defined(DGK) || defined(MACOS)
# ifdef MACOS
#define msmsg printf
# endif
		if(!hu) msmsg(".");
#endif
		if((ofd = open(lock, OMASK)) < 0) {
		    if(!hu) pline("Error while saving: cannot read %s.", lock);
		    (void) close(fd);
#ifdef MACOS
			(void)SetVol(0L, savenum);
#endif
		    (void) unlink(SAVEF);
		    if(!hu) done(TRICKED);
		    return(0);
		}
#ifdef ZEROCOMP
		minit();
#endif
		getlev(ofd, hackpid, ltmp, FALSE);
		(void) close(ofd);
		bwrite(fd, (genericptr_t) &ltmp, sizeof ltmp);  /* level number */
#if defined(DGK) && !defined(OLD_TOS)
		savelev(fd, ltmp, WRITE);			/* actual level */
#else
		savelev(fd, ltmp);			/* actual level */
#endif
		(void) unlink(lock);
	}
#ifdef ZEROCOMP
	bflush(fd);
#endif
	(void) close(fd);
	glo(dlevel);
	(void) unlink(lock);	/* get rid of current level --jgm */
	glo(0);
	(void) unlink(lock);
#ifdef COMPRESS
	Strcpy(cmd, COMPRESS);
	Strcat(cmd, " ");
# ifdef COMPRESS_OPTIONS
	Strcat(cmd, COMPRESS_OPTIONS);
	Strcat(cmd, " ");
# endif
	Strcat(cmd, SAVEF);
	(void) system(cmd);
#endif
	return(1);
}

#if defined(DGK) && !defined(OLD_TOS)
boolean
savelev(fd, lev, mode)
int fd;
xchar lev;
int mode;
{
	if (mode & COUNT) {
# ifdef ZEROCOMP /* should be superfluous */
		if (!count_only)	/* did we just write? */
			bflush(0);
		/*dbg();*/
# endif
		count_only = TRUE;
		bytes_counted = 0;
		savelev0(fd, lev);
		while (bytes_counted > freediskspace(levels))
			if (!swapout_oldest())
				return FALSE;
	}
	if (mode & WRITE) {
# ifdef ZEROCOMP
		if (mode & COUNT)	/* did we just count? */
			bflush(fd);
# endif
		count_only = FALSE;
		bytes_counted = 0;
		savelev0(fd, lev);
	}
	fileinfo[lev].where = ACTIVE;
	fileinfo[lev].time = moves;
	fileinfo[lev].size = bytes_counted;
	return TRUE;
}

static
void
savelev0(fd,lev)
#else
void
savelev(fd,lev)
#endif
int fd;
xchar lev;
{
#ifdef WORM
	register struct wseg *wtmp;
	register int tmp;
#endif
#ifdef TOS
	short tlev;
#endif

	if(fd < 0) panic("Save on bad file!");	/* impossible */
#if !defined(DGK) || defined(OLD_TOS)
	if(lev >= 0 && lev <= MAXLEVEL)
		level_exists[lev] = TRUE;
#endif
	bwrite(fd,(genericptr_t) &hackpid,sizeof(hackpid));
#ifdef TOS
	tlev=lev; tlev &= 0x00ff;
	bwrite(fd,(genericptr_t) &tlev,sizeof(tlev));
#else
	bwrite(fd,(genericptr_t) &lev,sizeof(lev));
#endif
#if defined(SMALLDATA) && defined(MACOS)
	/* asssumes ROWNO*sizeof(struct rm) < 128 bytes */
	{
		short	i;
		char	length;
		char	bufr[256],*ptr,*src,*d,*p;
		
		d = calloc(ROWNO*COLNO, sizeof(struct rm));
		p = d;
		for (i = 0; i < COLNO; i++) {
			ptr = &bufr[0];
			src = (char *)&levl[i][0];
			PackBits(&src, &ptr, ROWNO * sizeof(struct rm));
			length = (char)(ptr - &bufr[0]);
			BlockMove(&length, p++, (Size)1);
			BlockMove(bufr, p, (Size)length);
			p += (long)length;
		}
		i = (short)(p - d);
		bwrite(fd, (genericptr_t)&i, sizeof(short));
		bwrite(fd, (genericptr_t)d, i);
		free(d);
	}
#else
	bwrite(fd,(genericptr_t) levl,sizeof(levl));
#endif /* SMALLDATA */
#ifdef REINCARNATION
	if(dlevel == rogue_level && lev != rogue_level)
		/* save the symbols actually used to represent the level, not
		 * those in use for the current level (the default symbols used
		 * for rogue), since we will need to know whether to update
		 * the display of the screen when the game is restored under
		 * a potentially different value of showsyms from the
		 * environment */
		/* if a game is saved off the rogue level, the usual showsyms
		 * will be written out for the rogue level too, but they will
		 * be ignored on restore so it doesn't matter */
		bwrite(fd, (genericptr_t) savesyms, sizeof savesyms);
	else
#endif
		bwrite(fd, (genericptr_t) showsyms, sizeof showsyms);
	bwrite(fd,(genericptr_t) &monstermoves,sizeof(monstermoves));
	bwrite(fd,(genericptr_t) &xupstair,sizeof(xupstair));
	bwrite(fd,(genericptr_t) &yupstair,sizeof(yupstair));
	bwrite(fd,(genericptr_t) &xdnstair,sizeof(xdnstair));
	bwrite(fd,(genericptr_t) &ydnstair,sizeof(ydnstair));
#ifdef STRONGHOLD
	bwrite(fd,(genericptr_t) &xupladder,sizeof(xupladder));
	bwrite(fd,(genericptr_t) &yupladder,sizeof(yupladder));
	bwrite(fd,(genericptr_t) &xdnladder,sizeof(xdnladder));
	bwrite(fd,(genericptr_t) &ydnladder,sizeof(ydnladder));
#endif
	bwrite(fd,(genericptr_t) &fountsound,sizeof(fountsound));
	bwrite(fd,(genericptr_t) &sinksound,sizeof(sinksound));
	savemonchn(fd, fmon);
	savegoldchn(fd, fgold);
	savetrapchn(fd, ftrap);

	saveobjchn(fd, fobj);
	saveobjchn(fd, billobjs);

	save_engravings(fd);
	bwrite(fd,(genericptr_t) rooms,sizeof(rooms));
	bwrite(fd,(genericptr_t) doors,sizeof(doors));
#ifdef WORM
	bwrite(fd,(genericptr_t) wsegs,sizeof(wsegs));
	for(tmp=1; tmp<32; tmp++){
		for(wtmp = wsegs[tmp]; wtmp; wtmp = wtmp->nseg){
			bwrite(fd,(genericptr_t) wtmp,sizeof(struct wseg));
		}
#if defined(DGK) && !defined(OLD_TOS)
		if (!count_only)
#endif
			wsegs[tmp] = 0;
	}
	bwrite(fd,(genericptr_t) wgrowtime,sizeof(wgrowtime));
#endif /* WORM /**/
#if defined(DGK) && !defined(OLD_TOS)
	if (count_only)	return;
#endif
	billobjs = 0;
	fgold = 0;
	ftrap = 0;
	fmon = 0;
	fobj = 0;
}

#ifdef ZEROCOMP

#define RLESC '\0'    /* Leading character for run of LRESC's */
#define flushoutrun(ln) bputc(RLESC); bputc(ln); ln = -1;

static unsigned char outbuf[BUFSZ];
static unsigned short outbufp = 0;
static short outrunlength = -1;
static int bwritefd;

/*dbg()
{
   if(!hu) printf("outbufp %d outrunlength %d\n", outbufp,outrunlength);
}*/

static void bputc(c)
unsigned char c;
{
# ifdef DGK
    bytes_counted++;
    if (count_only)
      return;
# endif
    if (outbufp >= BUFSZ) {
      (void) write(bwritefd, outbuf, (int) BUFSZ);
      outbufp = 0;
    }
    outbuf[outbufp++] = c;
}

void
bflush(fd)  /* flush run and buffer */
register int fd;
{
      bwritefd = fd;
      if (outrunlength >= 0) {    /* flush run */
	  flushoutrun(outrunlength);
      }
      if (outbufp) {
#if defined(DGK) && !defined(OLD_TOS)
	  if (!count_only)    /* flush buffer */
#endif
		  (void) write(fd, outbuf, outbufp);
	  outbufp = 0;
      }
      /*printf("bflush()"); getret();*/
}

void
bwrite(fd, loc, num)
register int fd;
genericptr_t loc;
register unsigned num;
{
      bwritefd = fd;
      for (; num; num--, (*(char **)&loc)++) {
	      if (*((char *)loc) == RLESC) { /* One more char in run */
		  if (++outrunlength == 0xFF) {
		      flushoutrun(outrunlength);
		  }
	      } else { /* end of run */
		  if (outrunlength >= 0) {    /* flush run */
		      flushoutrun(outrunlength);
		  }
		  bputc(*((char *)loc));
	      }
      }
}

#else /* ZEROCOMP */

void
bwrite(fd,loc,num)
register int fd;
register genericptr_t loc;
register unsigned num;
{
#if defined(DGK) && !defined(OLD_TOS)
	bytes_counted += num;
	if (!count_only)
#endif
/* lint wants the 3rd arg of write to be an int; lint -p an unsigned */
#if defined(BSD) || defined(ULTRIX)
	    if(write(fd, loc, (int)num) != (int)num) {
#else /* e.g. SYSV, __TURBOC__ */
	    if(write(fd, loc, num) != num) {
#endif
		if(!hu) panic("cannot write %u bytes to file #%d", num, fd);
		else	exit(1);
	    }
}
#endif /* ZEROCOMP */

static void
saveobjchn(fd,otmp)
register int fd;
register struct obj *otmp;
{
	register struct obj *otmp2;
	unsigned int xl;
	int minusone = -1;

	while(otmp) {
	    if(Is_container(otmp))	/* unlink contained objects */
		spill_objs(otmp);	/* (this rearranges the list) */

	    otmp2 = otmp->nobj;
	    xl = otmp->onamelth;
	    bwrite(fd, (genericptr_t) &xl, sizeof(int));
	    bwrite(fd, (genericptr_t) otmp, xl + sizeof(struct obj));
#if defined(DGK) && !defined(OLD_TOS)
	    if (!count_only)
#endif
		free((genericptr_t) otmp);
	    otmp = otmp2;
	}
	bwrite(fd, (genericptr_t) &minusone, sizeof(int));
}

static void
savemonchn(fd,mtmp)
register int fd;
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	unsigned int xl;
	int minusone = -1;
	struct permonst *monbegin = &mons[0];

	bwrite(fd, (genericptr_t) &monbegin, sizeof(monbegin));

	while(mtmp) {
		mtmp2 = mtmp->nmon;
		xl = mtmp->mxlth + mtmp->mnamelth;
		bwrite(fd, (genericptr_t) &xl, sizeof(int));
		bwrite(fd, (genericptr_t) mtmp, xl + sizeof(struct monst));
		if(mtmp->minvent) saveobjchn(fd,mtmp->minvent);
#if defined(DGK) && !defined(OLD_TOS)
		if (!count_only)
#endif
		free((genericptr_t) mtmp);
		mtmp = mtmp2;
	}
	bwrite(fd, (genericptr_t) &minusone, sizeof(int));
}

static void
savegoldchn(fd,gold)
register int fd;
register struct gold *gold;
{
	register struct gold *gold2;
	while(gold) {
		gold2 = gold->ngold;
		bwrite(fd, (genericptr_t) gold, sizeof(struct gold));
#if defined(DGK) && !defined(OLD_TOS)
		if (!count_only)
#endif
			free((genericptr_t) gold);
		gold = gold2;
	}
	bwrite(fd, (genericptr_t)nul, sizeof(struct gold));
}

static void
savetrapchn(fd,trap)
register int fd;
register struct trap *trap;
{
	register struct trap *trap2;
	while(trap) {
		trap2 = trap->ntrap;
		bwrite(fd, (genericptr_t) trap, sizeof(struct trap));
#if defined(DGK) && !defined(OLD_TOS)
		if (!count_only)
#endif
			free((genericptr_t) trap);
		trap = trap2;
	}
	bwrite(fd, (genericptr_t)nul, sizeof(struct trap));
}

#ifdef TUTTI_FRUTTI
/* save all the fruit names and ID's; this is used only in saving whole games
 * (not levels) and in saving bones levels.  When saving a bones level,
 * we only want to save the fruits which exist on the bones level; the bones
 * level routine marks nonexistent fruits by making the fid negative.
 */
void
savefruitchn(fd)
register int fd;
{
	register struct fruit *f2, *f1;

	f1 = ffruit;
	while(f1) {
		f2 = f1->nextf;
		if (f1->fid >= 0) {
			bwrite(fd, (genericptr_t) f1, sizeof(struct fruit));
		}
#if defined(DGK) && !defined(OLD_TOS)
		if (!count_only)
#endif
			free((genericptr_t) f1);
		f1 = f2;
	}
	bwrite(fd, (genericptr_t)nul, sizeof(struct fruit));
}
#endif

static void
savegenoinfo(fd)
register int fd;
{
	register int i;

	for (i = 0; i < NUMMONS; i++)
		bwrite(fd, (genericptr_t) &(mons[i].geno), sizeof(unsigned));
}

#if defined(DGK) && !defined(OLD_TOS)
boolean
swapin_file(lev)
int lev;
{
	char to[PATHLEN], from[PATHLEN];

	Sprintf(from, "%s%s", permbones, alllevels);
	Sprintf(to, "%s%s", levels, alllevels);
	name_file(from, lev);
	name_file(to, lev);
	while (fileinfo[lev].size > freediskspace(to))
		if (!swapout_oldest())
			return FALSE;
#ifdef WIZARD
	if (wizard) {
		pline("Swapping in `%s'", from);
		(void) fflush(stdout);
	}
#endif
	copyfile(from, to);
	(void) unlink(from);
	fileinfo[lev].where = ACTIVE;
	return TRUE;
}

static boolean
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
	Sprintf(from, "%s%s", levels, alllevels);
	Sprintf(to, "%s%s", permbones, alllevels);
	name_file(from, oldest);
	name_file(to, oldest);
#ifdef WIZARD
	if (wizard) {
		pline("Swapping out `%s'.", from);
		(void) fflush(stdout);
	}
#endif
	copyfile(from, to);
	(void) unlink(from);
	fileinfo[oldest].where = SWAPPED;
	return TRUE;
}

static
void
copyfile(from, to)
char *from, *to;
{
#ifdef TOS

	if (_copyfile(from, to))
		panic("Can't copy %s to %s\n", from, to);
#else
	char buf[BUFSIZ];
	int nfrom, nto, fdfrom, fdto;

	if ((fdfrom = open(from, O_RDONLY | O_BINARY, FCMASK)) < 0)
		panic("Can't copy from %s !?", from);
	if ((fdto = open(to, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK)) < 0)
		panic("Can't copy to %s", to);
	do {
		nfrom = read(fdfrom, buf, BUFSIZ);
		nto = write(fdto, buf, nfrom);
		if (nto != nfrom)
			panic("Copyfile failed!");
	} while (nfrom == BUFSIZ);
	(void) close(fdfrom);
	(void) close(fdto);
#endif /* TOS */
}
#endif

/*
 * "spill" objects out of containers (unlinking from the fcobj list).
 *
 * The objects will be rearranged, and properly aged.  When we restore, they
 * can be put back into their containers.  By the time all of the calls to
 * saveobjchn() been made, the fcobj list should be empty.  Thus it need not
 * be saved, and doing so could cause some strange addressing problems.
 *
 * NOTE:  The cobj field is set to -1.  It will be used as a flag to indicate
 *	  that this object was previously in a container.
 */

static void
spill_objs(cobj)
register struct obj *cobj;
{
	register struct obj *otmp, *otmp2, *probj;

#ifdef LINT
	probj = (struct obj *)0;    /* suppress "used before set" error */
#endif
	for(otmp = fcobj; otmp; otmp = otmp2) {

	    otmp2 = otmp->nobj;
	    if(otmp->cobj == cobj) {

		if(cobj->cursed && rn2(2))	otmp->cursed = 1;
	/*
	 * Place all of the objects in a given container after that container
	 * in the list.  On restore, they should be able to be picked up and
	 * put back in.
	 */
		if(otmp == fcobj) fcobj = otmp2;
		else		  probj->nobj = otmp2;

		otmp->nobj = cobj->nobj;
		cobj->nobj = otmp;
		otmp->cobj = (struct obj *)-1;
	    } else probj = otmp;
	}

}
