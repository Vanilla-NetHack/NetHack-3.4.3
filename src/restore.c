/*	SCCS Id: @(#)restore.c	3.0	88/10/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"

#ifdef WORM
#include "wseg.h"
#endif

boolean restoring = FALSE;
#ifdef TUTTI_FRUTTI
static struct fruit *oldfruit;
#endif
static long omoves;

/*
 * "stuff" objects back into containers (relink the fcobj list).
 */
static void
stuff_objs(cobj)
register struct obj *cobj;
{
	register struct obj *otmp, *otmp2;

	for(; cobj; cobj = cobj->nobj)
	    if(Is_container(cobj))

		for(otmp = cobj->nobj;
		    otmp && otmp->cobj == (struct obj *) -1; otmp = otmp2) {

		    otmp2 = otmp->nobj;

		    otmp->cobj = cobj;
		    cobj->nobj = otmp2;
		    otmp->nobj = fcobj;
		    fcobj = otmp;
		}
}

/* Recalculate level.objects[x][y], since this info was not saved. */
static void
find_lev_obj()
{
	register struct obj *fobjtmp = (struct obj *)0;
	register struct obj *otmp;
	int x,y;

	for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++)
		level.objects[x][y] = (struct obj *)0;

	/* Reverse the entire fobj chain, which is necessary so that we can
	 * place the objects in the proper order.
	 */
	while(otmp = fobj) {
		fobj = otmp->nobj;
		otmp->nobj = fobjtmp;
		fobjtmp = otmp;
	}
	/* Set level.objects (as well as reversing the chain back again) */
	while(otmp = fobjtmp) {
		place_object(otmp, otmp->ox, otmp->oy);
		fobjtmp = otmp->nobj;
		otmp->nobj = fobj;
		fobj = otmp;
	}
}

static struct obj *
restobjchn(fd, ghostly)
register int fd;
boolean ghostly;
{
	register struct obj *otmp, *otmp2;
	register struct obj *first = 0;
#ifdef TUTTI_FRUTTI
	register struct fruit *oldf;
#endif
	int xl;
#ifdef LINT
	/* suppress "used before set" warning from lint */
	otmp2 = 0;
#endif
	while(1) {
		mread(fd, (genericptr_t) &xl, sizeof(xl));
		if(xl == -1) break;
		otmp = newobj(xl);
		if(!first) first = otmp;
		else otmp2->nobj = otmp;
		mread(fd, (genericptr_t) otmp, (unsigned) xl + sizeof(struct obj));
		if(!otmp->o_id) otmp->o_id = flags.ident++;
#ifdef TUTTI_FRUTTI
		if(ghostly && otmp->otyp == SLIME_MOLD) {
			for(oldf=oldfruit; oldf; oldf=oldf->nextf)
				if (oldf->fid == otmp->spe) break;
			if(!oldf) impossible("no old fruit?");
			else otmp->spe = fruitadd(oldf->fname);
		}
#endif
	/* Ghost levels get object age shifted from old player's clock to
	 * new player's clock.  Assumption: new player arrived immediately
	 * after old player died.
	 */
		if (ghostly) otmp->age = monstermoves-omoves+otmp->age;
		otmp2 = otmp;
	}
	if(first && otmp2->nobj){
		impossible("Restobjchn: error reading objchn.");
		otmp2->nobj = 0;
	}

	stuff_objs(first);
	return(first);
}

static struct monst *
restmonchn(fd, ghostly)
register int fd;
boolean ghostly;
{
	register struct monst *mtmp, *mtmp2;
	register struct monst *first = 0;
	int xl;

	struct permonst *monbegin;
	off_t differ;

	mread(fd, (genericptr_t)&monbegin, sizeof(monbegin));
#if !defined(MSDOS) && !defined(M_XENIX)
	differ = (genericptr_t)(&mons[0]) - (genericptr_t)(monbegin);
#else
	differ = (long)(&mons[0]) - (long)(monbegin);
#endif

#ifdef LINT
	/* suppress "used before set" warning from lint */
	mtmp2 = 0;
#endif
	while(1) {
		mread(fd, (genericptr_t) &xl, sizeof(xl));
		if(xl == -1) break;
		mtmp = newmonst(xl);
		if(!first) first = mtmp;
		else mtmp2->nmon = mtmp;
		mread(fd, (genericptr_t) mtmp, (unsigned) xl + sizeof(struct monst));
		if(!mtmp->m_id)
			mtmp->m_id = flags.ident++;
#if !defined(MSDOS) && !defined(M_XENIX)
		/* ANSI type for differ is ptrdiff_t --
		 * long may be wrong for segmented architecture --
		 * may be better to cast pointers to (struct permonst *)
		 * rather than (genericptr_t)
		 * this code handles save file -- so any bug should glow
		 * probably best not to keep lint from complaining
		 */
/*#ifdef LINT	/* possible compiler/hardware dependency - */
/*		if (differ) mtmp->data = NULL;*/
/*#else*/
		mtmp->data = (struct permonst *)
			((genericptr_t)mtmp->data + differ);
/*#endif	/*LINT*/
#else
		mtmp->data = (struct permonst *)
			((long) mtmp->data + differ);
#endif
		if(mtmp->minvent)
			mtmp->minvent = restobjchn(fd, ghostly);
		mtmp2 = mtmp;
	}
	if(first && mtmp2->nmon){
		impossible("Restmonchn: error reading monchn.");
		mtmp2->nmon = 0;
	}
	return(first);
}

static void
restgenoinfo(fd)
register int fd;
{
	register int i;

	for (i = 0; i < NUMMONS; i++)
		mread(fd, (genericptr_t) &(mons[i].geno), sizeof(unsigned));
}

int
dorecover(fd)
register int fd;
{
	register int nfd;
	int tmp;		/* not a register ! */
	xchar ltmp;
	unsigned int mid;		/* idem */
	struct obj *otmp;
#ifdef TUTTI_FRUTTI
	struct fruit *fruit;
#endif
#ifdef MSDOS
	struct flag oldflags;

	oldflags = flags;	/* Save flags set in the config file */
#endif
#ifdef ZEROCOMP
	minit();
#endif
	restoring = TRUE;
	getlev(fd, 0, (xchar)0, FALSE);
	invent = restobjchn(fd, FALSE);
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->owornmask)
			setworn(otmp, otmp->owornmask);
	fallen_down = restmonchn(fd, FALSE);
	restgenoinfo(fd);
	mread(fd, (genericptr_t) &tmp, sizeof tmp);
#ifdef WIZARD
	if(!wizard)
#endif
	    if(tmp != getuid()) {		/* strange ... */
		(void) close(fd);
		(void) unlink(SAVEF);
		(void) puts("Saved game was not yours.");
		restoring = FALSE;
		return(0);
	    }
	mread(fd, (genericptr_t) &flags, sizeof(struct flag));
	/* Some config file OPTIONS take precedence over those in save file.
	 */
#ifdef MSDOS
#ifdef DGK
	flags.rawio = oldflags.rawio;
#ifdef DECRAINBOW
	flags.DECRainbow = oldflags.DECRainbow;
#endif /* DECRAINBOW */
	flags.IBMBIOS = oldflags.IBMBIOS;
#endif
#endif
	mread(fd, (genericptr_t) &dlevel, sizeof dlevel);
	mread(fd, (genericptr_t) &maxdlevel, sizeof maxdlevel);
	mread(fd, (genericptr_t) &moves, sizeof moves);
	mread(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
	mread(fd, (genericptr_t) &wiz_level, sizeof wiz_level);
	mread(fd, (genericptr_t) &medusa_level, sizeof medusa_level);
	mread(fd, (genericptr_t) &bigroom_level, sizeof bigroom_level);
#ifdef ORACLE
	mread(fd, (genericptr_t) &oracle_level, sizeof oracle_level);
#endif
#ifdef REINCARNATION
	mread(fd, (genericptr_t) &rogue_level, sizeof rogue_level);
	if (dlevel==rogue_level)
		savesyms = showsyms;
#endif
#ifdef STRONGHOLD
	mread(fd, (genericptr_t) &stronghold_level, sizeof stronghold_level);
	mread(fd, (genericptr_t) &tower_level, sizeof tower_level);
	mread(fd, (genericptr_t) tune, sizeof tune);
#  ifdef MUSIC
	mread(fd, (genericptr_t) &music_heard, sizeof music_heard);
#  endif
#endif
	mread(fd, (genericptr_t) &is_maze_lev, sizeof is_maze_lev);
	mread(fd, (genericptr_t) &u, sizeof(struct you));
#ifdef SPELLS
	mread(fd, (genericptr_t) spl_book, sizeof(struct spell) * (MAXSPELL + 1));
#endif
	if(u.ustuck)
		mread(fd, (genericptr_t) &mid, sizeof mid);
	mread(fd, (genericptr_t) pl_character, sizeof pl_character);
#ifdef TUTTI_FRUTTI
	mread(fd, (genericptr_t) pl_fruit, sizeof pl_fruit);
	mread(fd, (genericptr_t) &current_fruit, sizeof current_fruit);
	ffruit = 0;
	while (fruit = newfruit(),
	       mread(fd, (genericptr_t)fruit, sizeof(struct fruit)),
	       fruit->fid) {
		fruit->nextf = ffruit;
		ffruit = fruit;
	}
	free((genericptr_t) fruit);
#endif

	restnames(fd);
#ifdef DGK
	msmsg("\n");
	cl_end();
	msmsg("You got as far as level %d%s.\n", maxdlevel,
		flags.debug ? " in WIZARD mode" :
		flags.explore ? " in discovery mode" : "");
	cl_end();
	msmsg("Restoring: ");
#endif
	while(1) {
#ifdef ZEROCOMP
		if(mread(fd, (genericptr_t) &ltmp, sizeof ltmp) < 0)
#else
		if(read(fd, (genericptr_t) &ltmp, sizeof ltmp) != sizeof ltmp)
#endif
			break;
		getlev(fd, 0, ltmp, FALSE);
		glo(ltmp);
#ifdef DGK
		msmsg(".");
#endif
#if defined(MSDOS) && !defined(TOS)
		nfd = open(lock, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
		nfd = creat(lock, FCMASK);
#endif
		if (nfd < 0)	panic("Cannot open temp file %s!\n", lock);
#if defined(DGK) && !defined(OLD_TOS)
		if (!savelev(nfd, ltmp, COUNT | WRITE)) {

			/* The savelev can't proceed because the size required
			 * is greater than the available disk space.
			 */
			msmsg("\nNot enough space on `%s' to restore your game.\n",
				levels);

			/* Remove levels and bones that may have been created.
			 */
			(void) close(nfd);
			eraseall(levels, alllevels);
			eraseall(levels, allbones);

			/* Perhaps the person would like to play without a
			 * RAMdisk.
			 */
			if (ramdisk) {
				/* PlaywoRAMdisk may not return, but if it does
				 * it is certain that ramdisk will be 0.
				 */
				playwoRAMdisk();
				/* Rewind save file and try again */
				(void) lseek(fd, (off_t)0, 0);
				return dorecover(fd);
			} else {
				msmsg("Be seeing you...\n");
				exit(0);
			}
		}
#else
		savelev(nfd, ltmp);
#endif
#ifdef ZEROCOMP
		bflush(nfd);
#endif
		(void) close(nfd);
	}
#ifdef BSD
	(void) lseek(fd, 0L, 0);
#else
	(void) lseek(fd, (off_t)0, 0);
#endif
#ifdef ZEROCOMP
	minit();
#endif
	getlev(fd, 0, (xchar)0, FALSE);
	(void) close(fd);
#if defined(WIZARD) || defined(EXPLORE_MODE)
	if(
# ifdef WIZARD
	   !wizard
#  ifdef EXPLORE_MODE
		   &&
#  endif
# endif
# ifdef EXPLORE_MODE
		      !discover
# endif
				)
#endif
		(void) unlink(SAVEF);
#ifdef REINCARNATION
	/* this can't be done earlier because we need to check the initial
	 * showsyms against the one saved in each of the non-rogue levels */
	if (dlevel==rogue_level)
		showsyms = defsyms;
#endif
	if(u.ustuck) {
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(mtmp->m_id == mid) goto monfnd;
		panic("Cannot find the monster ustuck.");
	monfnd:
		u.ustuck = mtmp;
	}
	setsee();  /* only to recompute seelx etc. - these weren't saved */
#ifdef DGK
	gameDiskPrompt();
#endif
	max_rank_sz(); /* to recompute mrank_sz (pri.c) */
#ifdef POLYSELF
	set_uasmon();
#endif
	/* take care of iron ball & chain */
	for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(otmp->owornmask)
			setworn(otmp, otmp->owornmask);
	docrt();
	restoring = FALSE;
	return(1);
}

void
getlev(fd, pid, lev, ghostly)
int fd, pid;
xchar lev;
boolean ghostly;
{
	register struct gold *gold;
	register struct trap *trap;
	register struct monst *mtmp;
#ifdef WORM
	register struct wseg *wtmp;
	register int tmp;
#endif
	long nhp;
	int hpid;
	xchar dlvl;
	struct symbols osymbol;
	int x, y;
	uchar osym, nsym;
#ifdef TOS
	short tlev;
#endif

#if defined(MSDOS) && !defined(TOS)
	setmode(fd, O_BINARY);	    /* is this required for TOS??? NO --ERS */
#endif
#ifdef TUTTI_FRUTTI
	/* Load the old fruit info.  We have to do it first, so the infor-
	 * mation is available when restoring the objects.  
	 */
	if (ghostly) {
		struct fruit *fruit;

		oldfruit = 0;
		while (fruit = newfruit(),
		       mread(fd, (genericptr_t)fruit, sizeof(struct fruit)),
		       fruit->fid) {
			fruit->nextf = oldfruit;
			oldfruit = fruit;
		}
		free((genericptr_t) fruit);
	}
#endif

	/* First some sanity checks */
	mread(fd, (genericptr_t) &hpid, sizeof(hpid));
#ifdef TOS
	mread(fd, (genericptr_t) &tlev, sizeof(tlev));
	dlvl=tlev&0x00ff;
#else
	mread(fd, (genericptr_t) &dlvl, sizeof(dlvl));
#endif
	if((pid && pid != hpid) || (lev && dlvl != lev)) {
#ifdef WIZARD
		if (wizard) {
			if (pid && pid != hpid)
				pline("PID (%d) doesn't match saved PID (%d)!", hpid, pid);
			else if (lev && dlvl != lev)
				pline("This is level %d, not %d!", dlvl, lev);
		}
#endif
		pline("Strange, this map is not as I remember it.");
		pline("Somebody is trying some trickery here...");
		pline("This game is void.");
		done(TRICKED);
	}

	mread(fd, (genericptr_t) levl, sizeof(levl));
	mread(fd, (genericptr_t) &osymbol, sizeof(osymbol));
	if (memcmp((genericptr_t) &osymbol,
		   (genericptr_t) &showsyms, sizeof (struct symbols))
#ifdef REINCARNATION
		&& dlvl != rogue_level
		/* rogue level always uses default syms, and showsyms will still
		 * have its initial value from environment when restoring a
		 * game */
#endif
	    ) {
		for (x = 0; x < COLNO; x++)
			for (y = 0; y < ROWNO; y++) {
				osym = levl[x][y].scrsym;
				nsym = 0;
				switch (levl[x][y].typ) {
				case STONE:
				case SCORR:
					if (osym == osymbol.stone)
						nsym = showsyms.stone;
					break;
				case ROOM:
#ifdef STRONGHOLD
				case DRAWBRIDGE_DOWN:
#endif /* STRONGHOLD /**/
					if (osym == osymbol.room)
						nsym = showsyms.room;
					break;
				case DOOR:
					if (osym == osymbol.door)
						nsym = showsyms.door;
					break;
				case CORR:
					if (osym == osymbol.corr)
						nsym = showsyms.corr;
					break;
				case VWALL:
					if (osym == osymbol.vwall)
						nsym = showsyms.vwall;
#ifdef STRONGHOLD
					else if (osym == osymbol.dbvwall)
						nsym = showsyms.dbvwall;
#endif
					break;
				case HWALL:
					if (osym == osymbol.hwall)
						nsym = showsyms.hwall;
#ifdef STRONGHOLD
					else if (osym == osymbol.dbhwall)
						nsym = showsyms.dbhwall;
#endif
					break;
				case TLCORNER:
					if (osym == osymbol.tlcorn)
						nsym = showsyms.tlcorn;
					break;
				case TRCORNER:
					if (osym == osymbol.trcorn)
						nsym = showsyms.trcorn;
					break;
				case BLCORNER:
					if (osym == osymbol.blcorn)
						nsym = showsyms.blcorn;
					break;
				case BRCORNER:
					if (osym == osymbol.brcorn)
						nsym = showsyms.brcorn;
					break;
				case SDOOR:
					if (osym == osymbol.vwall)
						nsym = showsyms.vwall;
					else if (osym == osymbol.hwall)
						nsym = showsyms.hwall;
					break;
				case CROSSWALL:
					if (osym == osymbol.crwall)
						nsym = showsyms.crwall;
					break;
				case TUWALL:
					if (osym == osymbol.tuwall)
						nsym = showsyms.tuwall;
					break;
				case TDWALL:
					if (osym == osymbol.tdwall)
						nsym = showsyms.tdwall;
					break;
				case TLWALL:
					if (osym == osymbol.tlwall)
						nsym = showsyms.tlwall;
					break;
				case TRWALL:
					if (osym == osymbol.trwall)
						nsym = showsyms.trwall;
					break;
				case STAIRS:
					if (osym == osymbol.upstair)
						nsym = showsyms.upstair;
					else if (osym == osymbol.dnstair)
						nsym = showsyms.dnstair;
					break;
#ifdef STRONGHOLD
				case LADDER:
					if (osym == osymbol.upladder)
						nsym = showsyms.upladder;
					else if (osym == osymbol.dnladder)
						nsym = showsyms.dnladder;
					break;
#endif /* STRONGHOLD /**/
				case POOL:
				case MOAT:
#ifdef STRONGHOLD
				case DRAWBRIDGE_UP:
#endif /* STRONGHOLD /**/
					if (osym == osymbol.pool)
						nsym = showsyms.pool;
					break;
#ifdef FOUNTAINS
				case FOUNTAIN:
					if (osym == osymbol.fountain)
						nsym = showsyms.fountain;
					break;
#endif /* FOUNTAINS /**/
#ifdef THRONES
				case THRONE:
					if (osym == osymbol.throne)
						nsym = showsyms.throne;
					break;
#endif /* THRONES /**/
#ifdef SINKS
				case SINK:
					if (osym == osymbol.sink)
						nsym = showsyms.sink;
					break;
#endif /* SINKS /**/
#ifdef ALTARS
				case ALTAR:
					if (osym == osymbol.altar)
						nsym = showsyms.altar;
					break;
#endif /* ALTARS /**/
				default:
					break;
				}
				if (nsym)
					levl[x][y].scrsym = nsym;
			}
	}

	mread(fd, (genericptr_t)&omoves, sizeof(omoves));
	mread(fd, (genericptr_t)&xupstair, sizeof(xupstair));
	mread(fd, (genericptr_t)&yupstair, sizeof(yupstair));
	mread(fd, (genericptr_t)&xdnstair, sizeof(xdnstair));
	mread(fd, (genericptr_t)&ydnstair, sizeof(ydnstair));
#ifdef STRONGHOLD
	mread(fd, (genericptr_t)&xupladder, sizeof(xupladder));
	mread(fd, (genericptr_t)&yupladder, sizeof(yupladder));
	mread(fd, (genericptr_t)&xdnladder, sizeof(xdnladder));
	mread(fd, (genericptr_t)&ydnladder, sizeof(ydnladder));
#endif
	mread(fd, (genericptr_t)&fountsound, sizeof(fountsound));
	mread(fd, (genericptr_t)&sinksound, sizeof(sinksound));
	fmon = restmonchn(fd, ghostly);

	/* regenerate animals while on another level */
	{ long tmoves = (moves > omoves) ? moves-omoves : 0;
	  register struct monst *mtmp, *mtmp2;

	  for(mtmp = fmon; mtmp; mtmp = mtmp2) {

		mtmp2 = mtmp->nmon;
		if(mtmp->data->geno & G_GENOD) {
#ifdef KOPS
			allow_kops = FALSE;
#endif
			mondead(mtmp);
#ifdef KOPS
			allow_kops = TRUE;
#endif
			continue;
		}

		if (ghostly) {
			/* reset peaceful/malign relative to new character */
			if(!mtmp->isshk)
				/* shopkeepers will reset based on name */
				mtmp->mpeaceful = peace_minded(mtmp->data);
			set_malign(mtmp);
		} else if (mtmp->mtame && tmoves > 250)
		  	mtmp->mtame = mtmp->mpeaceful = 0;

		/* restore shape changers - Maarten Jan Huisjes */
		if (mtmp->data == &mons[PM_CHAMELEON]
		    && !Protection_from_shape_changers
		    && !mtmp->cham)
			mtmp->cham = 1;
		else if(Protection_from_shape_changers) {
			if (mtmp->cham) {
				mtmp->cham = 0;
				(void) newcham(mtmp, &mons[PM_CHAMELEON]);
			} else if(is_were(mtmp->data) && !is_human(mtmp->data))
				(void) new_were(mtmp);
		}

		if (!ghostly) {
			nhp = mtmp->mhp +
				(regenerates(mtmp->data) ? tmoves : tmoves/20);
			if(nhp > mtmp->mhpmax)
				mtmp->mhp = mtmp->mhpmax;
			else
#ifdef LINT	/* (long)newhp -> (schar = short int) mhp; ok in context of text above */
				mtmp->mhp = 0;
#else
				mtmp->mhp = nhp;
#endif
		}
	  }
	}

	setgd();
	fgold = 0;
	while(gold = newgold(),
	      mread(fd, (genericptr_t)gold, sizeof(struct gold)),
	      gold->gx) {
		gold->ngold = fgold;
		fgold = gold;
	}
	free((genericptr_t) gold);
	ftrap = 0;
	while (trap = newtrap(),
	       mread(fd, (genericptr_t)trap, sizeof(struct trap)),
	       trap->tx) {
		trap->ntrap = ftrap;
		ftrap = trap;
	}
	free((genericptr_t) trap);
	fobj = restobjchn(fd, ghostly);
	find_lev_obj();
	billobjs = restobjchn(fd, ghostly);
	rest_engravings(fd);
	mread(fd, (genericptr_t)rooms, sizeof(rooms));
	mread(fd, (genericptr_t)doors, sizeof(doors));
#ifdef WORM
	mread(fd, (genericptr_t)wsegs, sizeof(wsegs));
	for(tmp = 1; tmp < 32; tmp++) if(wsegs[tmp]){
		wheads[tmp] = wsegs[tmp] = wtmp = newseg();
		while(1) {
			mread(fd, (genericptr_t)wtmp, sizeof(struct wseg));
			if(!wtmp->nseg) break;
			wheads[tmp]->nseg = wtmp = newseg();
			wheads[tmp] = wtmp;
		}
	}
	mread(fd, (genericptr_t)wgrowtime, sizeof(wgrowtime));
#endif

	/* reset level.monsters for new level */
	for (x = 0; x < COLNO; x++)
	    for (y = 0; y < ROWNO; y++)
		level.monsters[x][y] = (struct monst *) 0;
	for (mtmp = level.monlist; mtmp; mtmp = mtmp->nmon)
	    place_monster(mtmp, mtmp->mx, mtmp->my);

#ifdef TUTTI_FRUTTI
	/* Now get rid of all the temp fruits... */
	if (ghostly) {
		struct fruit *fruit;

		while(oldfruit) {
			fruit = oldfruit->nextf;
			free((genericptr_t) oldfruit);
			oldfruit = fruit;
		}
	}
#endif
}

#ifdef ZEROCOMP
#define RLESC '\0' 	/* Leading character for run of RLESC's */

static unsigned char inbuf[BUFSZ];
static unsigned short inbufp = 0;
static unsigned short inbufsz = 0;
static short inrunlength = -1;
static int mreadfd;

static int
mgetc()
{
    if (inbufp >= inbufsz) {
      inbufsz = read(mreadfd, (genericptr_t)inbuf, (int)sizeof inbuf);
      if (!inbufsz) {
	  if (inbufp > sizeof inbuf)
	      error("EOF on file #%d.\n", mreadfd);
	  inbufp = 1 + sizeof inbuf;  /* exactly one warning :-) */
	  return -1;
      }
      inbufp = 0;
    }
    return inbuf[inbufp++];
}

void
minit()
{
    inbufsz = 0;
    inbufp = 0;
    inrunlength = -1;
}

int
mread(fd, buf, len)
int fd;
genericptr_t buf;
register unsigned len;
{
    /*register int readlen = 0;*/
    mreadfd = fd;
    while (len--) {
      if (inrunlength > 0) {
	  inrunlength--;
	  *(*((char **)&buf))++ = '\0';
      } else {
	  register short ch = mgetc();
	  if (ch < 0) return -1; /*readlen;*/
	  if ((*(*(char **)&buf)++ = ch) == RLESC) {
	      inrunlength = mgetc();
	  }
      }
      /*readlen++;*/
    }
    return 0; /*readlen;*/
}

#else /* ZEROCOMP */

void
mread(fd, buf, len)
register int fd;
register genericptr_t buf;
register unsigned int len;
{
	register int rlen;

#if defined(BSD) || defined(ULTRIX)
	rlen = read(fd, buf, (int) len);
	if(rlen != len){
#else /* e.g. SYSV, __TURBOC__ */
	rlen = read(fd, buf, (unsigned) len);
	if((unsigned)rlen != len){
#endif
		pline("Read %d instead of %u bytes.\n", rlen, len);
		if(restoring) {
			(void) unlink(SAVEF);
			error("Error restoring old game.");
		}
		panic("Error reading level file.");
	}
}
#endif /* ZEROCOMP */
