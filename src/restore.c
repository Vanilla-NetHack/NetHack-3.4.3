/*	SCCS Id: @(#)restore.c	3.1	93/01/23	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include "termcap.h" /* for TERMLIB and ASCIIGRAPH */

#ifdef MICRO
extern int dotcnt;	/* shared with save */
#endif

#ifdef ZEROCOMP
static int NDECL(mgetc);
#endif
static void NDECL(find_lev_obj);
#ifndef NO_SIGNAL
static void NDECL(inven_inuse);
#endif
static void FDECL(restlevchn, (int));
static void FDECL(restdamage, (int,BOOLEAN_P));
static struct obj * FDECL(restobjchn, (int,BOOLEAN_P));
static struct monst * FDECL(restmonchn, (int,BOOLEAN_P));
static void FDECL(restgenoinfo, (int));
static boolean FDECL(restgamestate, (int, unsigned int *));
static int FDECL(restlevelfile, (int,XCHAR_P));

#ifdef MULDGN
#include "quest.h"
#endif

boolean restoring = FALSE;
#ifdef TUTTI_FRUTTI
static struct fruit NEARDATA *oldfruit;
#endif
static long NEARDATA omoves;

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
	while ((otmp = fobj) != 0) {
		fobj = otmp->nobj;
		otmp->nobj = fobjtmp;
		fobjtmp = otmp;
	}
	/* Set level.objects (as well as reversing the chain back again) */
	while ((otmp = fobjtmp) != 0) {
		place_object(otmp, otmp->ox, otmp->oy);
		fobjtmp = otmp->nobj;
		otmp->nobj = fobj;
		fobj = otmp;
	}
}

#ifndef NO_SIGNAL
static void
inven_inuse()
/* Things that were marked "in_use" when the game was saved (ex. via the
 * infamous "HUP" cheat) get used up here.
 */
{
	register struct obj *otmp, *otmp2;

	for(otmp = invent; otmp; otmp = otmp2) {
		otmp2 = otmp->nobj;
		if(otmp->in_use) {
			/* in_use and oldcorpse share a bit, but we don't
			 * want nasty messages for old corpses --
			 * remove_cadavers() will clean them up nicely
			 */
			if (otmp->otyp == CORPSE &&
					mons[otmp->corpsenm].mlet == S_TROLL)
				continue;
			pline("Finishing off %s...", xname(otmp));
			useup(otmp);
		}
	}
}
#endif

static void
restlevchn(fd)
register int fd;
{
	int cnt;
	s_level	*tmplev, *x;

	sp_levchn = (s_level *) 0;
	mread(fd, (genericptr_t) &cnt, sizeof(int));
	for(; cnt > 0; cnt--) {

	    tmplev = (s_level *)alloc(sizeof(s_level));
	    mread(fd, (genericptr_t) tmplev, sizeof(s_level));
	    if(!sp_levchn) sp_levchn = tmplev;
	    else {

		for(x = sp_levchn; x->next; x = x->next);
		x->next = tmplev;
	    }
	    tmplev->next = (s_level *)0;
	}
}

static void
restdamage(fd, ghostly)
int fd;
boolean ghostly;
{
	int counter;
	struct damage *tmp_dam;

	mread(fd, (genericptr_t) &counter, sizeof(counter));
	if (!counter)
	    return;
	tmp_dam = (struct damage *)alloc(sizeof(struct damage));
	while (1) {
	    char damaged_shops[5], *shp = NULL;

	    mread(fd, (genericptr_t) tmp_dam, sizeof(*tmp_dam));
	    if (ghostly)
		tmp_dam->when += (monstermoves - omoves);
	    Strcpy(damaged_shops,
		   in_rooms(tmp_dam->place.x, tmp_dam->place.y, SHOPBASE));
	    if (u.uz.dlevel) {
		/* when restoring, there are two passes over the current
		 * level.  the first time, u.uz isn't set, so neither is
		 * shop_keeper().  just wait and process the damage on
		 * the second pass.
		 */
		for (shp = damaged_shops; *shp; shp++) {
		    struct monst *shkp = shop_keeper(*shp);

		    if (shkp && inhishop(shkp) && repair_damage(shkp, tmp_dam))
			break;
		}
	    }
	    if (!shp || !*shp) {
		tmp_dam->next = level.damagelist;
		level.damagelist = tmp_dam;
		tmp_dam = (struct damage *)alloc(sizeof(*tmp_dam));
	    }
	    if (!(--counter)) {
		free((genericptr_t)tmp_dam);
		return;
	    }
	}
}

static struct obj *
restobjchn(fd, ghostly)
register int fd;
boolean ghostly;
{
	register struct obj *otmp, *otmp2;
	register struct obj *first = (struct obj *)0;
#ifdef TUTTI_FRUTTI
	register struct fruit *oldf;
#endif
	int xl;

#if defined(LINT) || defined(GCC_WARN)
	/* suppress "used before set" warning from lint */
	otmp2 = 0;
#endif
	while(1) {
		mread(fd, (genericptr_t) &xl, sizeof(xl));
		if(xl == -1) break;
		otmp = newobj(xl);
		if(!first) first = otmp;
		else otmp2->nobj = otmp;
		mread(fd, (genericptr_t) otmp,
					(unsigned) xl + sizeof(struct obj));
		if(!otmp->o_id) otmp->o_id = flags.ident++;
#ifdef TUTTI_FRUTTI
		if(ghostly && otmp->otyp == SLIME_MOLD) {
			for(oldf=oldfruit; oldf; oldf=oldf->nextf)
				if (oldf->fid == otmp->spe) break;
			if(!oldf) impossible("no old fruit?");
			else otmp->spe = fruitadd(oldf->fname);
		}
#endif
		/* Ghost levels get object age shifted from old player's clock
		 * to new player's clock.  Assumption: new player arrived
		 * immediately after old player died.
		 */
		if (ghostly && otmp->otyp != OIL_LAMP
				&& otmp->otyp != BRASS_LANTERN
				&& otmp->otyp != CANDELABRUM_OF_INVOCATION
				&& !Is_candle(otmp))
			otmp->age = monstermoves-omoves+otmp->age;

		/* get contents of the container */
		if (Is_container(otmp) || otmp->otyp == STATUE)
		    otmp->cobj = restobjchn(fd,ghostly);

		otmp2 = otmp;
	}
	if(first && otmp2->nobj){
		impossible("Restobjchn: error reading objchn.");
		otmp2->nobj = 0;
	}

	return(first);
}

static struct monst *
restmonchn(fd, ghostly)
register int fd;
boolean ghostly;
{
	register struct monst *mtmp, *mtmp2;
	register struct monst *first = (struct monst *)0;
	int xl;
	struct permonst *monbegin;
	boolean moved;

	/* get the original base address */
	mread(fd, (genericptr_t)&monbegin, sizeof(monbegin));
	moved = (monbegin != mons);

#if defined(LINT) || defined(GCC_WARN)
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
		if (moved && mtmp->data) {
			int offset = mtmp->data - monbegin;	/*(ptrdiff_t)*/
			mtmp->data = mons + offset;  /* new permonst location */
		}
		if(mtmp->minvent)
			mtmp->minvent = restobjchn(fd, ghostly);
#ifdef MUSE
		if (mtmp->mw) mtmp->mw = mtmp->minvent;	/* wield 1st obj in inventory */
#endif
		if (mtmp->isshk) restshk(mtmp);

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
	unsigned genolist[NUMMONS];

	mread(fd, (genericptr_t) genolist, sizeof(genolist));

	for (i = 0; i < NUMMONS; i++)
		mons[i].geno = genolist[i];
}

static
boolean
restgamestate(fd, mid)
register int fd;
unsigned int *mid;
{
	struct obj *otmp;
	int tmp;		/* not a register ! */
	struct flag oldflags;
#ifdef TUTTI_FRUTTI
	struct fruit *fruit;
#endif

	invent = restobjchn(fd, FALSE);
	migrating_objs = restobjchn(fd, FALSE);
	migrating_mons = restmonchn(fd, FALSE);
	restgenoinfo(fd);

	mread(fd, (genericptr_t) &tmp, sizeof tmp);
#ifdef WIZARD
	if(!wizard)
#endif
	    if(tmp != getuid()) {		/* strange ... */
		pline("Saved game was not yours.");
		return(FALSE);
	    }

	oldflags = flags;
	mread(fd, (genericptr_t) &flags, sizeof(struct flag));
	/* Some config file and command line OPTIONS take precedence over
	 * those in save file.
	 */
#ifdef TERMLIB
	flags.DECgraphics = oldflags.DECgraphics;
#endif
#ifdef ASCIIGRAPH
	flags.IBMgraphics = oldflags.IBMgraphics;
#endif
#ifdef MICRO
	flags.rawio = oldflags.rawio;
	flags.BIOS = oldflags.BIOS;
#endif
#ifdef TEXTCOLOR
	flags.use_color = oldflags.use_color;
	flags.hilite_pet = oldflags.hilite_pet;
#endif
#ifdef MAC_GRAPHICS_ENV
	flags.MACgraphics = oldflags.MACgraphics;
	flags.large_font = oldflags.large_font;
#endif
	/* these come from the current environment; ignore saved values */
	flags.window_inited = oldflags.window_inited;
	flags.msg_history = oldflags.msg_history;
	flags.echo = oldflags.echo;
	flags.cbreak = oldflags.cbreak;

	mread(fd, (genericptr_t) &u, sizeof(struct you));
	if(u.uhp <= 0) {
	    You("were not healthy enough to survive restoration.");
	    /* wiz1_level.dlevel is used by mklev.c to see if lots of stuff is
	     * uninitialized, so we only have to set it and not the other stuff.
	     */
	    wiz1_level.dlevel = 0;
	    u.uz.dnum = 0;
	    u.uz.dlevel = 1;
	    return(FALSE);
	}

	/* don't do this earlier to avoid complicating abort above */
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->owornmask)
			setworn(otmp, otmp->owornmask);

	restore_dungeon(fd);
	mread(fd, (genericptr_t) &inv_pos, sizeof inv_pos);
	restlevchn(fd);
	mread(fd, (genericptr_t) &moves, sizeof moves);
	mread(fd, (genericptr_t) &monstermoves, sizeof monstermoves);
#ifdef MULDGN
	mread(fd, (genericptr_t) &quest_status, sizeof(struct q_score));
#endif
	mread(fd, (genericptr_t) spl_book,
				sizeof(struct spell) * (MAXSPELL + 1));
	restore_artifacts(fd);
	restore_oracles(fd);
	if(u.ustuck)
		mread(fd, (genericptr_t) mid, sizeof (*mid));
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
	dealloc_fruit(fruit);
#endif
	restnames(fd);
	restore_waterlevel(fd);
	return(TRUE);
}

/*ARGSUSED*/	/* fd used in MFLOPPY only */
static int
restlevelfile(fd, ltmp)
register int fd;
xchar ltmp;
{
	register int nfd;

	nfd = create_levelfile(ltmp);

	if (nfd < 0)	panic("Cannot open temp level %d!", ltmp);
#ifdef MFLOPPY
	if (!savelev(nfd, ltmp, COUNT_SAVE)) {

		/* The savelev can't proceed because the size required
		 * is greater than the available disk space.
		 */
		pline("Not enough space on `%s' to restore your game.",
			levels);

		/* Remove levels and bones that may have been created.
		 */
		(void) close(nfd);
		eraseall(levels, alllevels);
# ifndef AMIGA
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
			return dorecover(fd);	/* 0 or 1 */
		} else {
# endif
			pline("Be seeing you...");
			terminate(0);
# ifndef AMIGA
		}
# endif
	}
#endif
	bufon(nfd);
	savelev(nfd, ltmp, WRITE_SAVE | FREE_SAVE);
	bclose(nfd);
	return(2);
}

int
dorecover(fd)
register int fd;
{
	unsigned int mid;		/* not a register */
	xchar ltmp;
	int rtmp;
	struct obj *otmp;

	minit();	/* ZEROCOMP */
	restoring = TRUE;
	getlev(fd, 0, (xchar)0, FALSE);
	if (!restgamestate(fd, &mid)) {
		(void) close(fd);
		(void) delete_savefile();
		restoring = FALSE;
		return(0);
	}
#ifdef INSURANCE
	savestateinlock();
#endif
	rtmp = restlevelfile(fd, ledger_no(&u.uz));
	if (rtmp < 2) return(rtmp);  /* dorecover called recursively */

#ifdef MICRO
# ifdef AMIGA
	{
	extern winid WIN_BASE;
	clear_nhwindow(WIN_BASE);	/* hack until there's a hook for this */
	}
# else
	clear_nhwindow(WIN_MAP);
# endif
	clear_nhwindow(WIN_MESSAGE);
	You("got as far as level %d in %s%s.",
		depth(&u.uz), dungeons[u.uz.dnum].dname,
		flags.debug ? " while in WIZARD mode" :
		flags.explore ? " while in discovery mode" : "");
	curs(WIN_MAP, 1, 1);
	dotcnt = 0;
	putstr(WIN_MAP, 0, "Restoring:");
#endif
	while(1) {
#ifdef ZEROCOMP
		if(mread(fd, (genericptr_t) &ltmp, sizeof ltmp) < 0)
#else
		if(read(fd, (genericptr_t) &ltmp, sizeof ltmp) != sizeof ltmp)
#endif
			break;
		getlev(fd, 0, ltmp, FALSE);
#ifdef MICRO
		curs(WIN_MAP, 11 + dotcnt++, 1);
		putstr(WIN_MAP, 0, ".");
#endif
		rtmp = restlevelfile(fd, ltmp);
		if (rtmp < 2) return(rtmp);  /* dorecover called recursively */
	}

#ifdef BSD
	(void) lseek(fd, 0L, 0);
#else
	(void) lseek(fd, (off_t)0, 0);
#endif
	minit();	/* ZEROCOMP */
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
		(void) delete_savefile();
#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) assign_rogue_graphics(TRUE);
#endif
	if(u.ustuck) {
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(mtmp->m_id == mid) goto monfnd;
		panic("Cannot find the monster ustuck.");
	monfnd:
		u.ustuck = mtmp;
	}
#ifdef MFLOPPY
	gameDiskPrompt();
#endif
	max_rank_sz(); /* to recompute mrank_sz (botl.c) */
#ifdef POLYSELF
	set_uasmon();
#endif
	/* take care of iron ball & chain */
	for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(otmp->owornmask)
			setworn(otmp, otmp->owornmask);
#ifndef NO_SIGNAL
	/* in_use processing must be after:
	 *    + The inventory has been read so that freeinv() works.
	 *    + The current level has been restored so billing information
	 *	is available.
	 */
	inven_inuse();
#endif
#ifdef MULDGN
	load_qtlist();	/* re-load the quest text info */
#endif
	/* Set up the vision internals, after levl[] data is loaded */
	/* but before docrt().					    */
	vision_reset();
	vision_full_recalc = 1;	/* recompute vision (not saved) */
	docrt();
	restoring = FALSE;
	clear_nhwindow(WIN_MESSAGE);
	return(1);
}

void
trickery()
{
	pline("Strange, this map is not as I remember it.");
	pline("Somebody is trying some trickery here...");
	pline("This game is void.");
	done(TRICKED);
}

void
getlev(fd, pid, lev, ghostly)
int fd, pid;
xchar lev;
boolean ghostly;
{
	register struct trap *trap;
	register struct monst *mtmp;
	branch *br;
	int hpid;
	xchar dlvl;
	int x, y;
#ifdef TOS
	short tlev;
#endif

#if defined(MSDOS) || defined(OS2)
	setmode(fd, O_BINARY);
#endif
#ifdef TUTTI_FRUTTI
	/* Load the old fruit info.  We have to do it first, so the
	 * information is available when restoring the objects.
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
		dealloc_fruit(fruit);
	}
#endif

	/* First some sanity checks */
	mread(fd, (genericptr_t) &hpid, sizeof(hpid));
/* CHECK:  This may prevent restoration */
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
		trickery();
	}

#ifdef RLECOMP
	{
		short	i, j;
		uchar	len;
		struct rm r;
		
		i = 0; j = 0; len = 0;
		while(i < ROWNO) {
		    while(j < COLNO) {
			if(len > 0) {
			    levl[j][i] = r;
			    len -= 1;
			    j += 1;
			} else {
			    mread(fd, (genericptr_t)&len, sizeof(uchar));
			    mread(fd, (genericptr_t)&r, sizeof(struct rm));
			}
		    }
		    j = 0;
		    i += 1;
		}
	}
#else
	mread(fd, (genericptr_t) levl, sizeof(levl));
#endif	/* RLECOMP */

	mread(fd, (genericptr_t)&omoves, sizeof(omoves));
	mread(fd, (genericptr_t)&upstair, sizeof(stairway));
	mread(fd, (genericptr_t)&dnstair, sizeof(stairway));
	mread(fd, (genericptr_t)&upladder, sizeof(stairway));
	mread(fd, (genericptr_t)&dnladder, sizeof(stairway));
	mread(fd, (genericptr_t)&sstairs, sizeof(stairway));
	mread(fd, (genericptr_t)&updest, sizeof(dest_area));
	mread(fd, (genericptr_t)&dndest, sizeof(dest_area));
	mread(fd, (genericptr_t)&level.flags, sizeof(level.flags));

	fmon = restmonchn(fd, ghostly);

	/* regenerate animals while on another level */
	{ long tmoves = (monstermoves > omoves) ? monstermoves-omoves : 0;
	  register struct monst *mtmp2;

	  for(mtmp = fmon; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if(mtmp->data->geno & G_GENOD) {
			/* mondead() would try to link the monster's objects
			 * into fobj and the appropriate nexthere chain.
			 * unfortunately, such things will not have sane
			 * values until after find_lev_obj() well below
			 * here, so we'd go chasing random pointers if we
			 * tried that.  we could save the monster's objects
			 * in another chain and insert them in the level
			 * later, but that's a lot of work for very little
			 * gain.  hence, just throw the objects away via
			 * mongone() and pretend the monster wandered off
			 * somewhere private before the genocide.
			 */
			mongone(mtmp);
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
				new_were(mtmp);
		}

		if (!ghostly) {
			long nhp = mtmp->mhp +
				(regenerates(mtmp->data) ? tmoves : tmoves/20);

			if(!mtmp->mcansee && mtmp->mblinded) {
				if ((long) mtmp->mblinded <= tmoves) {
					mtmp->mblinded = 0;
					mtmp->mcansee = 1;
				} else mtmp->mblinded -= tmoves;
			}
			if(!mtmp->mcanmove && mtmp->mfrozen) {
				if ((long) mtmp->mfrozen <= tmoves) {
					mtmp->mfrozen = 0;
					mtmp->mcanmove = 1;
				} else mtmp->mfrozen -= tmoves;
			}
			if(mtmp->mflee && mtmp->mfleetim) {
				if ((long) mtmp->mfleetim <= tmoves) {
					mtmp->mfleetim = 0;
					mtmp->mflee = 0;
				} else mtmp->mfleetim -= tmoves;
			}
			if(nhp >= mtmp->mhpmax)
				mtmp->mhp = mtmp->mhpmax;
			else
				mtmp->mhp = nhp;
		}
	  }
	}

	rest_worm(fd);	/* restore worm information */
	ftrap = 0;
	while (trap = newtrap(),
	       mread(fd, (genericptr_t)trap, sizeof(struct trap)),
	       trap->tx) {
		trap->ntrap = ftrap;
		ftrap = trap;
	}
	dealloc_trap(trap);
	fobj = restobjchn(fd, ghostly);
	find_lev_obj();
	billobjs = restobjchn(fd, ghostly);
	rest_engravings(fd);
	rest_rooms(fd);		/* No joke :-) */
	mread(fd, (genericptr_t)doors, sizeof(doors));

	/* reset level.monsters for new level */
	for (x = 0; x < COLNO; x++)
	    for (y = 0; y < ROWNO; y++)
		level.monsters[x][y] = (struct monst *) 0;
	for (mtmp = level.monlist; mtmp; mtmp = mtmp->nmon) {
	    if (mtmp->isshk)
		set_residency(mtmp, FALSE);
	    place_monster(mtmp, mtmp->mx, mtmp->my);
	    if (mtmp->wormno) place_wsegs(mtmp);
	}
	restdamage(fd, ghostly);


#ifdef TUTTI_FRUTTI
	/* Now get rid of all the temp fruits... */
	if (ghostly) {
		struct fruit *fruit;

		while(oldfruit) {
			fruit = oldfruit->nextf;
			dealloc_fruit(oldfruit);
			oldfruit = fruit;
		}
	}
#endif
	if (ghostly && lev > ledger_no(&medusa_level) &&
			lev < ledger_no(&stronghold_level) && xdnstair == 0) {
		coord cc;

		mazexy(&cc);
		xdnstair = cc.x;
		ydnstair = cc.y;
		levl[cc.x][cc.y].typ = STAIRS;
	}
	if (ghostly && (br = Is_branchlev(&u.uz)) && u.uz.dlevel == 1) {
	    d_level ltmp;

	    if (on_level(&u.uz, &br->end1))
		assign_level(&ltmp, &br->end2);
	    else
		assign_level(&ltmp, &br->end1);

	    switch(br->type) {
	    case BR_STAIR:
	    case BR_NO_END1:
	    case BR_NO_END2: /* OK to assign to sstairs if it's not used */
		assign_level(&sstairs.tolev, &ltmp);
		break;		
	    case BR_PORTAL: /* max of 1 portal per level */
		{
		    register struct trap *ttmp;
		    for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
			if (ttmp->ttyp == MAGIC_PORTAL)
			    break;
		    if (!ttmp) panic("getlev: need portal but none found");
		    assign_level(&ttmp->dst, &ltmp);
		}
		break;
	    }
	}
}

#ifdef ZEROCOMP
#define RLESC '\0'	/* Leading character for run of RLESC's */

#ifndef ZEROCOMP_BUFSIZ
#define ZEROCOMP_BUFSIZ BUFSZ
#endif
static unsigned char NEARDATA inbuf[ZEROCOMP_BUFSIZ];
static unsigned short NEARDATA inbufp = 0;
static unsigned short NEARDATA inbufsz = 0;
static short NEARDATA inrunlength = -1;
static int NEARDATA mreadfd;

static int
mgetc()
{
    if (inbufp >= inbufsz) {
	inbufsz = read(mreadfd, (genericptr_t)inbuf, sizeof inbuf);
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
minit()
{
    return;
}

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
		pline("Read %d instead of %u bytes.", rlen, len);
		if(restoring) {
			(void) close(fd);
			(void) delete_savefile();
			error("Error restoring old game.");
		}
		panic("Error reading level file.");
	}
}
#endif /* ZEROCOMP */

/*restore.c*/
