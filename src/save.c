/*	SCCS Id: @(#)save.c	3.1	93/02/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif /* !NO_SIGNAL */
#if defined(EXPLORE_MODE) && !defined(LSC) && !defined(O_WRONLY) && !defined(AZTEC_C)
#include <fcntl.h>
#endif /* EXPLORE_MODE */

boolean hu;		/* set during hang-up */

#ifdef MULDGN
#include "quest.h"
#endif

#ifdef MFLOPPY
extern struct finfo fileinfo[];
long bytes_counted;
static int count_only;
#else
extern boolean level_exists[];
#endif

#ifdef MICRO
int dotcnt;	/* also used in restore */
#endif

#ifdef ZEROCOMP
static void FDECL(bputc, (UCHAR_P));
#endif
static void FDECL(savelevchn, (int, int));
static void FDECL(savedamage, (int,struct damage *, int));
static void FDECL(saveobjchn, (int,struct obj *, int));
static void FDECL(savemonchn, (int,struct monst *, int));
static void FDECL(savetrapchn, (int,struct trap *, int));
static void FDECL(savegenoinfo, (int));
static void FDECL(savegamestate, (int, int));
#ifdef MFLOPPY
static void FDECL(savelev0, (int,XCHAR_P,int));
static boolean NDECL(swapout_oldest);
static void FDECL(copyfile, (char *,char *));
#endif /* MFLOPPY */
#ifdef GCC_WARN
static long nulls[10];
#else
#define nulls nul
#endif

int
dosave()
{
	clear_nhwindow(WIN_MESSAGE);
	if(yn("Really save?") == 'n') {
		clear_nhwindow(WIN_MESSAGE);
		if(multi > 0) nomul(0);
	} else {
		clear_nhwindow(WIN_MESSAGE);
		pline("Saving...");
		hu = FALSE;
		if(dosave0()) {
			/* make sure they see the Saving message */
			display_nhwindow(WIN_MESSAGE, TRUE);
			exit_nhwindows("Be seeing you...");
			terminate(0);
		} else (void)doredraw();
	}
	return 0;
}

#ifndef NOSAVEONHANGUP
int
hangup() {
	if(!hu) {
		hu = TRUE;
		(void) dosave0();
# ifndef VMS
		terminate(1);
# endif
	}
	return 0;
}
#endif

/* returns 1 if save successful */
int
dosave0()
{
	register int fd, ofd;
	xchar ltmp;
	d_level uz_save;
#ifdef MFLOPPY
	long fds, needed;
#endif

	if (!SAVEF[0])
		return 0;

#if defined(UNIX) || defined(VMS)
	(void) signal(SIGHUP, SIG_IGN);
#endif
#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#endif

#if defined(MICRO) && defined(MFLOPPY)
	if(!hu && !saveDiskPrompt(0))	return 0;
#endif

#ifdef EXPLORE_MODE
	if(!hu && flags.window_inited) {
	    fd = open_savefile();
	    if (fd > 0) {
		(void) close(fd);
		clear_nhwindow(WIN_MESSAGE);
		pline("There seems to be an old save file.");
		if (yn("Overwrite the old file?") == 'n') return 0;
	    }
	}
#endif
	if (!hu) mark_synch();	/* flush any buffered screen output */

	fd = create_savefile();

	if(fd < 0) {
		if(!hu) pline("Cannot open save file.");
		(void) delete_savefile();	/* ab@unido */
		return(0);
	}
	if(flags.moonphase == FULL_MOON)	/* ut-sally!fletcher */
		change_luck(-1);		/* and unido!ab */
	if(flags.friday13)
		change_luck(1);
	if(flags.window_inited)
	    clear_nhwindow(WIN_MESSAGE);

#ifdef MFLOPPY
	if(!hu) {
	    dotcnt = 0;
	    curs(WIN_MAP, 1, 1);
	    putstr(WIN_MAP, 0, "Saving:");
	}
	/* make sure there is enough disk space */
	savelev(fd, ledger_no(&u.uz), COUNT_SAVE);
	savegamestate(fd, COUNT_SAVE);
	needed = bytes_counted;
	for (ltmp = 1; ltmp <= maxledgerno(); ltmp++)
		if (ltmp != ledger_no(&u.uz) && fileinfo[ltmp].where)
			needed += fileinfo[ltmp].size + (sizeof ltmp);
# ifdef AMIGA
	needed+=ami_wbench_iconsize(SAVEF);
# endif
	fds = freediskspace(SAVEF);
	if(needed > fds) {
	    if(!hu) {
		pline("There is insufficient space on SAVE disk.");
		pline("Require %ld bytes but only have %ld.", needed, fds);
	    }
	    flushout();
	    (void) close(fd);
	    (void) delete_savefile();
	    return 0;
	}
#endif /* MFLOPPY */

	bufon(fd);
	savelev(fd, ledger_no(&u.uz), WRITE_SAVE | FREE_SAVE);
	savegamestate(fd, WRITE_SAVE | FREE_SAVE);

	/* While copying level files around, zero out u.uz to keep
	 * parts of the restore code from completely initializing all
	 * in-core data structures, since all we're doing is copying.
	 * This also avoids at least one nasty core dump.
	 */
	uz_save = u.uz;
	u.uz.dnum = u.uz.dlevel = 0;

	for(ltmp = (xchar)1; ltmp <= maxledgerno(); ltmp++) {
		if (ltmp == ledger_no(&uz_save)) continue;
#ifdef MFLOPPY
		if (!fileinfo[ltmp].where) continue;
#else
		if(!level_exists[ltmp]) continue;
#endif
#ifdef MICRO
		if(!hu) {
		    curs(WIN_MAP, 1 + dotcnt++, 2);
		    putstr(WIN_MAP, 0, ".");
		    mark_synch();
		}
#endif
		ofd = open_levelfile(ltmp);
		if(ofd < 0) {
		    if(!hu) pline("Cannot read level %d.", ltmp);
		    (void) close(fd);
		    (void) delete_savefile();
		    if(!hu) done(TRICKED);
		    return(0);
		}
		minit();	/* ZEROCOMP */
		getlev(ofd, hackpid, ltmp, FALSE);
		(void) close(ofd);
		bwrite(fd, (genericptr_t) &ltmp, sizeof ltmp); /* level number*/
		savelev(fd, ltmp, WRITE_SAVE | FREE_SAVE);     /* actual level*/
		delete_levelfile(ltmp);
	}
	bclose(fd);

	u.uz = uz_save;

	/* get rid of current level --jgm */
	delete_levelfile(ledger_no(&u.uz));
	delete_levelfile(0);
	compress(SAVEF);
#ifdef AMIGA
	ami_wbench_iconwrite(SAVEF);
#endif
	return(1);
}

static void
savegamestate(fd, mode)
register int fd, mode;
{
	int tmp;		/* not register ! */

#ifdef MFLOPPY
	count_only = (mode & COUNT_SAVE);
#endif
	saveobjchn(fd, invent, mode);
	saveobjchn(fd, migrating_objs, mode);
	savemonchn(fd, migrating_mons, mode);
	savegenoinfo(fd);
	tmp = getuid();
	bwrite(fd, (genericptr_t) &tmp, sizeof tmp);
	bwrite(fd, (genericptr_t) &flags, sizeof(struct flag));
	bwrite(fd, (genericptr_t) &u, sizeof(struct you));
	save_dungeon(fd);
	bwrite(fd, (genericptr_t) &inv_pos, sizeof inv_pos);
	savelevchn(fd, mode);
	bwrite(fd, (genericptr_t) &moves, sizeof moves);
	bwrite(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
#ifdef MULDGN
	bwrite(fd, (genericptr_t) &quest_status, sizeof(struct q_score));
#endif
	bwrite(fd, (genericptr_t) spl_book, 
				sizeof(struct spell) * (MAXSPELL + 1));
	save_artifacts(fd);
	save_oracles(fd);
	if(u.ustuck)
	    bwrite(fd, (genericptr_t) &(u.ustuck->m_id), sizeof u.ustuck->m_id);
	bwrite(fd, (genericptr_t) pl_character, sizeof pl_character);
#ifdef TUTTI_FRUTTI
	bwrite(fd, (genericptr_t) pl_fruit, sizeof pl_fruit);
	bwrite(fd, (genericptr_t) &current_fruit, sizeof current_fruit);
	savefruitchn(fd, mode);
#endif
	savenames(fd);
	save_waterlevel(fd);
	bflush(fd);
}

#ifdef INSURANCE
void
savestateinlock()
{
	int fd, hpid;
	static boolean havestate = TRUE;

	/* When checkpointing is on, the full state needs to be written
	 * on each checkpoint.  When checkpointing is off, only the pid
	 * needs to be in the level.0 file, so it does not need to be
	 * constantly rewritten.  When checkpointing is turned off during
	 * a game, however, the file has to be rewritten once to truncate
	 * it and avoid restoring from outdated information.
	 *
	 * Restricting havestate to this routine means that an additional
	 * noop pid rewriting will take place on the first "checkpoint" after
	 * the game is started or restored, if checkpointing is off.
	 */
	if (flags.ins_chkpt || havestate) {
		/* save the rest of the current game state in the lock file,
		 * following the original int pid, the current level number,
		 * and the current savefile name, which should not be subject
		 * to any internal compression schemes since they must be
		 * readable by an external utility
		 */
		fd = open_levelfile(0);
		if (fd < 0) {
		    pline("Cannot open level 0.");
		    pline("Probably someone removed it.");
		    done(TRICKED);
		    return;
		}

		(void) read(fd, (genericptr_t) &hpid, sizeof(hpid));
		if (hackpid != hpid) {
		    pline("Level 0 pid bad!");
		    done(TRICKED);
		}
		(void) close(fd);

		fd = create_levelfile(0);
		if (fd < 0) {
		    pline("Cannot rewrite level 0.");
		    done(TRICKED);
		    return;
		}
		(void) write(fd, (genericptr_t) &hackpid, sizeof(hackpid));
		if (flags.ins_chkpt) {
		    int currlev = ledger_no(&u.uz);

		    (void) write(fd, (genericptr_t) &currlev, sizeof(currlev));
		    save_savefile_name(fd);
		    bufon(fd);
		    savegamestate(fd, WRITE_SAVE);
		}
		bclose(fd);
	}
	havestate = flags.ins_chkpt;
}
#endif

#ifdef MFLOPPY
boolean
savelev(fd, lev, mode)
int fd;
xchar lev;
int mode;
{
	if (mode & COUNT_SAVE) {
		bytes_counted = 0;
		savelev0(fd, lev, COUNT_SAVE);
		while (bytes_counted > freediskspace(levels))
			if (!swapout_oldest())
				return FALSE;
	}
	if (mode & WRITE_SAVE) {
		bytes_counted = 0;
		/* mode is WRITE_SAVE and possibly FREE_SAVE */
		savelev0(fd, lev, mode);
	}
	fileinfo[lev].where = ACTIVE;
	fileinfo[lev].time = moves;
	fileinfo[lev].size = bytes_counted;
	return TRUE;
}

static void
savelev0(fd,lev,mode)
#else
void
savelev(fd,lev,mode)
#endif
int fd;
xchar lev;
int mode;
{
#ifdef TOS
	short tlev;
#endif

	if(fd < 0) panic("Save on bad file!");	/* impossible */
#ifdef MFLOPPY
	count_only = (mode & COUNT_SAVE);
#else
	if(lev >= 0 && lev <= maxledgerno()) level_exists[lev] = TRUE;
#endif
	bwrite(fd,(genericptr_t) &hackpid,sizeof(hackpid));
#ifdef TOS
	tlev=lev; tlev &= 0x00ff;
	bwrite(fd,(genericptr_t) &tlev,sizeof(tlev));
#else
	bwrite(fd,(genericptr_t) &lev,sizeof(lev));
#endif
#ifdef RLECOMP
	{
	    /* perform run-length encoding of rm structs */
	    struct rm *prm, *rgrm;
	    int x, y;
	    uchar match;
	    
	    rgrm = &levl[0][0];		/* start matching at first rm */
	    match = 0;

	    for (y = 0; y < ROWNO; y++) {
		for (x = 0; x < COLNO; x++) {
		    prm = &levl[x][y];
		    if (prm->glyph == rgrm->glyph
			&& prm->typ == rgrm->typ
			&& prm->seen == rgrm->seen
			&& prm->lit == rgrm->lit
			&& prm->doormask == rgrm->doormask
			&& prm->horizontal == rgrm->horizontal
			&& prm->waslit == rgrm->waslit
			&& prm->roomno == rgrm->roomno
			&& prm->edge == rgrm->edge) {
			match++;
			if (match > 254) {
			    match = 254;	/* undo this match */
			    goto writeout;
			}
		    } else {
			/* the run has been broken,
			 * write out run-length encoding */
		    writeout:
			bwrite(fd, (genericptr_t)&match, sizeof(uchar));
			bwrite(fd, (genericptr_t)rgrm, sizeof(struct rm));
			/* start encoding again. we have at least 1 rm
			 * in the next run, viz. this one. */
			match = 1;	
			rgrm = prm;
		    }
		}
	    }
	    if (match > 0) {
		bwrite(fd, (genericptr_t)&match, sizeof(uchar));
		bwrite(fd, (genericptr_t)rgrm, sizeof(struct rm));
	    }
	}
#else
	bwrite(fd,(genericptr_t) levl,sizeof(levl));
#endif /* RLECOMP */

	bwrite(fd,(genericptr_t) &monstermoves,sizeof(monstermoves));
	bwrite(fd,(genericptr_t) &upstair,sizeof(stairway));
	bwrite(fd,(genericptr_t) &dnstair,sizeof(stairway));
	bwrite(fd,(genericptr_t) &upladder,sizeof(stairway));
	bwrite(fd,(genericptr_t) &dnladder,sizeof(stairway));
	bwrite(fd,(genericptr_t) &sstairs,sizeof(stairway));
	bwrite(fd,(genericptr_t) &updest,sizeof(dest_area));
	bwrite(fd,(genericptr_t) &dndest,sizeof(dest_area));
	bwrite(fd,(genericptr_t) &level.flags,sizeof(level.flags));
	savemonchn(fd, fmon, mode);
	save_worm(fd, mode);	/* save worm information */
	savetrapchn(fd, ftrap, mode);
	saveobjchn(fd, fobj, mode);
	saveobjchn(fd, billobjs, mode);

	save_engravings(fd, mode);
	save_rooms(fd);
	bwrite(fd,(genericptr_t) doors,sizeof(doors));
	savedamage(fd, level.damagelist, mode);
	if (mode & FREE_SAVE) {
		billobjs = 0;
		ftrap = 0;
		fmon = 0;
		fobj = 0;
	}
	bflush(fd);
}

#ifdef ZEROCOMP
/* The runs of zero-run compression are flushed after the game state or a
 * level is written out.  This adds a couple bytes to a save file, where
 * the runs could be mashed together, but it allows gluing together game
 * state and level files to form a save file, and it means the flushing
 * does not need to be specifically called for every other time a level
 * file is written out.
 */

#define RLESC '\0'    /* Leading character for run of LRESC's */
#define flushoutrun(ln) (bputc(RLESC), bputc(ln), ln = -1)

#ifndef ZEROCOMP_BUFSIZ
# define ZEROCOMP_BUFSIZ BUFSZ
#endif
static NEARDATA unsigned char outbuf[ZEROCOMP_BUFSIZ];
static NEARDATA unsigned short outbufp = 0;
static NEARDATA short outrunlength = -1;
static NEARDATA int bwritefd;

/*dbg()
{
   if(!hu) printf("outbufp %d outrunlength %d\n", outbufp,outrunlength);
}*/

static void
bputc(c)
unsigned char c;
{
#ifdef MFLOPPY
    bytes_counted++;
    if (count_only)
      return;
#endif
    if (outbufp >= sizeof outbuf) {
	(void) write(bwritefd, outbuf, sizeof outbuf);
	outbufp = 0;
    }
    outbuf[outbufp++] = c;
}

/*ARGSUSED*/
void
bufon(fd)
    int fd;
{
    return;
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
#ifdef MFLOPPY
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

void
bclose(fd)
    int fd;
{
    if (outbufp)
	panic("closing file with buffered data still unwritten");
    (void) close(fd);
}

#else /* ZEROCOMP */

static int bw_fd = -1;
static FILE *bw_FILE = 0;

void
bufon(fd)
    int fd;
{
#ifdef UNIX
    if(bw_fd >= 0)
	panic("double buffering unexpected");
    bw_fd = fd;
    if((bw_FILE = fdopen(fd, "w")) == 0)
	panic("buffering of file %d failed", fd);
#endif
}

void
bflush(fd)
    int fd;
{
#ifdef UNIX
    if(fd == bw_fd) {
	if(fflush(bw_FILE) == EOF)
	    panic("flush of savefile failed!");
    }
#endif
    return;
}

void
bwrite(fd,loc,num)
register int fd;
register genericptr_t loc;
register unsigned num;
{
#ifdef MFLOPPY
	bytes_counted += num;
	if (!count_only)
#endif
	{
#ifdef UNIX
	    if(fd != bw_fd)
		panic("unbuffered write to fd %d (!= %d)", fd, bw_fd);

	    if(fwrite(loc, (int)num, 1, bw_FILE) != 1)
/* lint wants the 3rd arg of write to be an int; lint -p an unsigned */
#else
# if defined(BSD) || defined(ULTRIX)
	    if(write(fd, loc, (int)num) != (int)num)
# else /* e.g. SYSV, __TURBOC__ */
	    if(write(fd, loc, num) != num)
# endif
#endif
	    {
		if(!hu) panic("cannot write %u bytes to file #%d", num, fd);
		else	terminate(1);
	    }
	}
}

void
bclose(fd)
    int fd;
{
    bflush(fd);
#ifdef UNIX
    if (fd == bw_fd) {
	(void) fclose(bw_FILE);
	bw_fd = -1;
	bw_FILE = 0;
	return;
    }
#endif
    (void) close(fd);
}
#endif /* ZEROCOMP */

static void
savelevchn(fd, mode)
register int fd, mode;
{
	int cnt = 0;
	s_level	*tmplev, *tmplev2;

	for(tmplev = sp_levchn; tmplev; tmplev = tmplev->next) cnt++;
	bwrite(fd, (genericptr_t) &cnt, sizeof(int));

	for(tmplev = sp_levchn; tmplev; tmplev = tmplev2) {

	    tmplev2 = tmplev->next;
	    bwrite(fd, (genericptr_t) tmplev, sizeof(s_level));
	    if (mode & FREE_SAVE)
		free((genericptr_t) tmplev);
	}
}

static void
savedamage(fd, damageptr, mode)
register int fd, mode;
register struct damage *damageptr;
{
	register struct damage *tmp_dam;
	unsigned int xl = 0;

	for (tmp_dam = damageptr; tmp_dam; tmp_dam = tmp_dam->next) 
	    xl++;
	bwrite(fd, (genericptr_t) &xl, sizeof(xl));
	while (xl--) {
	    bwrite(fd, (genericptr_t) damageptr, sizeof(*damageptr)); 
	    tmp_dam = damageptr;
	    damageptr = damageptr->next;
	    if (mode & FREE_SAVE)
		free((genericptr_t)tmp_dam);
	}
	if (mode & FREE_SAVE)
	    level.damagelist = 0;
}

static void
saveobjchn(fd,otmp,mode)
register int fd, mode;
register struct obj *otmp;
{
	register struct obj *otmp2;
	unsigned int xl;
	int minusone = -1;

	while(otmp) {
	    otmp2 = otmp->nobj;
	    xl = otmp->onamelth;
	    bwrite(fd, (genericptr_t) &xl, sizeof(int));
	    bwrite(fd, (genericptr_t) otmp, xl + sizeof(struct obj));

	    if (Is_container(otmp) || otmp->otyp == STATUE)
		saveobjchn(fd,otmp->cobj,mode);
	    if (mode & FREE_SAVE) {
		if(otmp->oclass == FOOD_CLASS) food_disappears(otmp);
		dealloc_obj(otmp);
	    }
	    otmp = otmp2;
	}
	bwrite(fd, (genericptr_t) &minusone, sizeof(int));
}

static void
savemonchn(fd,mtmp,mode)
register int fd, mode;
register struct monst *mtmp;
{
	register struct monst *mtmp2;
	unsigned int xl;
	int minusone = -1;
	struct permonst *monbegin = &mons[0];

	bwrite(fd, (genericptr_t) &monbegin, sizeof(monbegin));

	while(mtmp) {
		mtmp2 = mtmp->nmon;
#ifdef MUSE
		if (mtmp->mw && mtmp->mw != mtmp->minvent) sort_mwep(mtmp);
#endif
		xl = mtmp->mxlth + mtmp->mnamelth;
		bwrite(fd, (genericptr_t) &xl, sizeof(int));
		bwrite(fd, (genericptr_t) mtmp, xl + sizeof(struct monst));
		if(mtmp->minvent) saveobjchn(fd,mtmp->minvent,mode);
		if (mode & FREE_SAVE)
		    dealloc_monst(mtmp);
		mtmp = mtmp2;
	}
	bwrite(fd, (genericptr_t) &minusone, sizeof(int));
}

static void
savetrapchn(fd,trap,mode)
register int fd,mode;
register struct trap *trap;
{
	register struct trap *trap2;
	while(trap) {
		trap2 = trap->ntrap;
		bwrite(fd, (genericptr_t) trap, sizeof(struct trap));
		if (mode & FREE_SAVE)
			dealloc_trap(trap);
		trap = trap2;
	}
	bwrite(fd, (genericptr_t)nulls, sizeof(struct trap));
}

#ifdef TUTTI_FRUTTI
/* save all the fruit names and ID's; this is used only in saving whole games
 * (not levels) and in saving bones levels.  When saving a bones level,
 * we only want to save the fruits which exist on the bones level; the bones
 * level routine marks nonexistent fruits by making the fid negative.
 */
void
savefruitchn(fd, mode)
register int fd, mode;
{
	register struct fruit *f2, *f1;

	f1 = ffruit;
	while(f1) {
		f2 = f1->nextf;
		if (f1->fid >= 0) {
			bwrite(fd, (genericptr_t) f1, sizeof(struct fruit));
		}
		if (mode & FREE_SAVE)
			dealloc_fruit(f1);
		f1 = f2;
	}
	bwrite(fd, (genericptr_t)nulls, sizeof(struct fruit));
}
#endif

static void
savegenoinfo(fd)
register int fd;
{
	register int i;
	unsigned genolist[NUMMONS];

	for (i = 0; i < NUMMONS; i++)
		genolist[i] = mons[i].geno;

	bwrite(fd, (genericptr_t) genolist, sizeof(genolist));
}

#ifdef MFLOPPY
boolean
swapin_file(lev)
int lev;
{
	char to[PATHLEN], from[PATHLEN];

	Sprintf(from, "%s%s", permbones, alllevels);
	Sprintf(to, "%s%s", levels, alllevels);
	set_levelfile_name(from, lev);
	set_levelfile_name(to, lev);
	while (fileinfo[lev].size > freediskspace(to))
		if (!swapout_oldest())
			return FALSE;
# ifdef WIZARD
	if (wizard) {
		pline("Swapping in `%s'", from);
		wait_synch();
	}
# endif
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
	for (i = 1, oldtime = 0, oldest = 0; i <= maxledgerno(); i++)
		if (fileinfo[i].where == ACTIVE
		&& (!oldtime || fileinfo[i].time < oldtime)) {
			oldest = i;
			oldtime = fileinfo[i].time;
		}
	if (!oldest)
		return FALSE;
	Sprintf(from, "%s%s", levels, alllevels);
	Sprintf(to, "%s%s", permbones, alllevels);
	set_levelfile_name(from, oldest);
	set_levelfile_name(to, oldest);
# ifdef WIZARD
	if (wizard) {
		pline("Swapping out `%s'.", from);
		wait_synch();
	}
# endif
	copyfile(from, to);
	(void) unlink(from);
	fileinfo[oldest].where = SWAPPED;
	return TRUE;
}

static void
copyfile(from, to)
char *from, *to;
{
# ifdef TOS

	if (_copyfile(from, to))
		panic("Can't copy %s to %s", from, to);
# else
	char buf[BUFSIZ];	/* this is system interaction, therefore
				 * BUFSIZ instead of NetHack's BUFSZ */
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
# endif /* TOS */
}

void
co_false()	    /* see comment in bones.c */
{
    count_only = FALSE;
    return;
}

#endif /* MFLOPPY */

/*save.c*/
