/*	SCCS Id: @(#)bones.c	3.1	93/01/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"

#ifdef MFLOPPY
extern char bones[];	/* from files.c */
extern long bytes_counted;
#endif

static boolean FDECL(no_bones_level, (d_level *));
#ifdef TUTTI_FRUTTI
static void FDECL(goodfruit, (int));
#endif
static void FDECL(resetobjs,(struct obj *,BOOLEAN_P));
static void FDECL(drop_upon_death, (struct monst *, struct obj *));

static boolean
no_bones_level(lev)
d_level *lev;
{
	extern d_level save_dlevel;		/* in do.c */
	s_level *sptr;

	if (ledger_no(&save_dlevel)) assign_level(lev, &save_dlevel);

	return (((sptr = Is_special(lev)) && !sptr->boneid)
		|| !dungeons[lev->dnum].boneid
		   /* no bones on the last or multiway branch levels */
		   /* in any dungeon (level 1 isn't multiway).       */
		|| Is_botlevel(lev) || (Is_branchlev(lev) && lev->dlevel > 1)
		   /* no bones in the invocation level               */
		|| (In_hell(lev) && lev->dlevel == dunlevs_in_dungeon(lev) - 1)
		);
}

#ifdef TUTTI_FRUTTI
static void
goodfruit(id)
int id;
{
	register struct fruit *f;

	for(f=ffruit; f; f=f->nextf) {
		if(f->fid == -id) {
			f->fid = id;
			return;
		}
	}
}
#endif

static void
resetobjs(ochain,restore)
struct obj *ochain;
boolean restore;
{
	struct obj *otmp;

	for (otmp = ochain; otmp; otmp = otmp->nobj) {
		if (otmp->cobj)
		    resetobjs(otmp->cobj,restore);

		if (((otmp->otyp != CORPSE || otmp->corpsenm < PM_ARCHEOLOGIST)
			&& otmp->otyp != STATUE)
			&& (!otmp->oartifact ||
			    (exist_artifact(otmp->otyp,ONAME(otmp)) && restore))) {
			otmp->oartifact = 0;
			otmp->onamelth = 0;
			*ONAME(otmp) = '\0';
		} else if (otmp->oartifact && restore)
			artifact_exists(otmp,ONAME(otmp),TRUE);
		if (!restore) {
			/* resetting the o_id's after getlev has carefully
			 * created proper new ones via restobjchn is a Bad
			 * Idea */
			otmp->o_id = 0;
			if(objects[otmp->otyp].oc_uses_known) otmp->known = 0;
			otmp->dknown = otmp->bknown = 0;
			otmp->rknown = 0;
			otmp->invlet = 0;
#ifdef TUTTI_FRUTTI
			if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
#endif
#ifdef MAIL
			if (otmp->otyp == SCR_MAIL) otmp->spe = 1;
#endif
#ifdef POLYSELF
			if (otmp->otyp == EGG) otmp->spe = 0;
#endif
			if(otmp->otyp == AMULET_OF_YENDOR) {
				/* no longer the actual amulet */
				otmp->otyp = FAKE_AMULET_OF_YENDOR;
				curse(otmp);
			}
			if(otmp->otyp == CANDELABRUM_OF_INVOCATION) {
			    if(otmp->spe > 0) { /* leave candles, if any */
			        otmp->otyp = WAX_CANDLE;
				otmp->age = 50L;  /* assume used */
				otmp->quan = (long)otmp->spe;
				otmp->lamplit = 0;
				otmp->spe = 0;
			    } else obfree(otmp, (struct obj *)0);
			}
			if(otmp->otyp == BELL_OF_OPENING) otmp->otyp = BELL;
			if(otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
			    otmp->otyp = SPE_MAGIC_MISSILE +
			                    rn2(SPE_BLANK_PAPER -
						  SPE_MAGIC_MISSILE + 1);
			    curse(otmp);
			}
		}
	}			
}

static void
drop_upon_death(mtmp, cont)
struct monst *mtmp;
struct obj *cont;
{
	struct obj *otmp = invent;
	while(otmp) {
		otmp->owornmask = 0;
		otmp->lamplit = 0;
#ifdef TUTTI_FRUTTI
		if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
#endif
		if(rn2(5)) curse(otmp);
		if(!mtmp && !cont) place_object(otmp, u.ux, u.uy);
		if(!otmp->nobj) {
			if (mtmp) {
				otmp->nobj = mtmp->minvent;
				mtmp->minvent = invent;
			} else if (cont) {
				otmp->nobj = cont->cobj;
				cont->cobj = invent;
			} else {
				otmp->nobj = fobj;
				fobj = invent;
			}
			invent = 0;	/* superfluous */
			break;
		}
		otmp = otmp->nobj;
	}
	if(u.ugold) {
		if (mtmp) mtmp->mgold = u.ugold;
		else mkgold(u.ugold, u.ux, u.uy);
	}
}

/* save bones and possessions of a deceased adventurer */
void
savebones()
{
	register int fd, x, y;
	register struct trap *ttmp;
	register struct monst *mtmp, *mtmp2;
#ifdef TUTTI_FRUTTI
	struct fruit *f;
#endif
	char *bonesid;

	if(ledger_no(&u.uz) <= 0 || ledger_no(&u.uz) > maxledgerno()) return;
	if(no_bones_level(&u.uz)) return; /* no bones for specific levels */
	if(!Is_branchlev(&u.uz)) {
	    /* no bones on non-branches with portals */
	    for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
		if (ttmp->ttyp == MAGIC_PORTAL) return;
	}

	if(depth(&u.uz) <= 0 ||		/* bulletproofing for endgame */
	   !rn2(1 + (depth(&u.uz)>>2)) /* fewer ghosts on low levels */
#ifdef WIZARD
		&& !wizard
#endif
		) return;
#ifdef EXPLORE_MODE
	/* don't let multiple restarts generate multiple copies of objects
	 * in bones files */
	if(discover) return;
#endif

	fd = open_bonesfile(&u.uz, &bonesid);
	if (fd >= 0) {
		(void) close(fd);
		compress_bonesfile();
#ifdef WIZARD
		if(wizard)
			pline("Bones file already exists.");
#endif
		return;
	}

#ifdef WALKIES
	unleash_all();
#endif
	/* in case these characters are not in their home bases */
	mtmp2 = fmon;
	while((mtmp = mtmp2)) {
		mtmp2 = mtmp->nmon;
		if(mtmp->iswiz || mtmp->data == &mons[PM_MEDUSA]) mongone(mtmp);
	}
#ifdef TUTTI_FRUTTI
	/* mark all fruits as nonexistent; when we come to them we'll mark
	 * them as existing (using goodfruit())
	 */
	for(f=ffruit; f; f=f->nextf) f->fid = -f->fid;
#endif

	/* check iron balls separately--maybe they're not carrying it */
	if (uball) uball->owornmask = uchain->owornmask = 0;

	/* dispose of your possessions, usually cursed */
	if (u.ugrave_arise == -2) {
		struct obj *otmp;

		/* embed your possessions in your statue */
		otmp = mk_named_object(STATUE,
#ifdef POLYSELF
					u.mtimedone ? uasmon :
#endif
					player_mon(), 
					u.ux, u.uy, plname,
					(int)strlen(plname));
		if (!otmp) return;
		drop_upon_death(mtmp = (struct monst *)0, otmp);
	} else if (u.ugrave_arise == -1) {
		/* drop everything */
		drop_upon_death((struct monst *)0, (struct obj *)0);
		/* trick makemon() into allowing monster creation
		 * on your location
		 */
		in_mklev = TRUE;
		mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy);
		in_mklev = FALSE;
		if (!mtmp) return;
		Strcpy((char *) mtmp->mextra, plname);
	} else {
		/* give your possessions to the monster you become */
		in_mklev = TRUE;
		mtmp = makemon(&mons[u.ugrave_arise], u.ux, u.uy);
		in_mklev = FALSE;
		if (!mtmp) return;
		mtmp = christen_monst(mtmp, plname);
		newsym(u.ux, u.uy);
		Your("body rises from the dead as %s...",
			an(mons[u.ugrave_arise].mname));
		display_nhwindow(WIN_MESSAGE, FALSE);
		drop_upon_death(mtmp, (struct obj *)0);
#ifdef MUSE
		m_dowear(mtmp, TRUE);
#endif
	}
	if (mtmp) {
		mtmp->m_lev = (u.ulevel ? u.ulevel : 1);
		mtmp->mhp = mtmp->mhpmax = u.uhpmax;
		mtmp->msleep = 1;
	}
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		resetobjs(mtmp->minvent,FALSE);
		mtmp->m_id = 0;
		mtmp->mlstmv = 0L;
		if(mtmp->mtame) mtmp->mtame = mtmp->mpeaceful = 0;
	}
	for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
		ttmp->tseen = 0;
	}
	resetobjs(fobj,FALSE);

	/* Clear all memory from the level. */
	for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++) {
	    levl[x][y].seen = levl[x][y].waslit = 0;
	    levl[x][y].glyph = cmap_to_glyph(S_stone);
	}

	fd = create_bonesfile(&u.uz, &bonesid);
	if(fd < 0) {
#ifdef WIZARD
		if(wizard)
			pline("Cannot create bones file - create failed");
#endif
		return;
	}

	bufon(fd);
#ifdef MFLOPPY  /* check whether there is room */
	savelev(fd, ledger_no(&u.uz), COUNT_SAVE);
# ifdef TUTTI_FRUTTI
	/* this is in the opposite order from the real save, but savelev()
	 * initializes bytes_counted to 0, so doing savefruitchn() first is
	 * useless; the extra bflush() at the end of savelev() may increase
	 * bytes_counted by a couple over what the real usage will be
	 */
	savefruitchn(fd, COUNT_SAVE);
	bflush(fd);
# endif
	if (bytes_counted > freediskspace(bones)) {	/* not enough room */
# ifdef WIZARD
		if (wizard)
			pline("Insufficient space to create bones file.");
# endif
		(void) close(fd);
		delete_bonesfile(&u.uz);
		return;
	}
	co_false();	/* make sure bonesid and savefruitchn get written */
#endif /* MFLOPPY */

	bwrite(fd, (genericptr_t) bonesid, 7);	/* DD.nnn */
#ifdef TUTTI_FRUTTI
	savefruitchn(fd, WRITE_SAVE | FREE_SAVE);
#endif
	savelev(fd, ledger_no(&u.uz), WRITE_SAVE | FREE_SAVE);
	bclose(fd);
	compress_bonesfile();
}

int
getbones()
{
	register int fd;
	register int ok;
	char *bonesid, oldbonesid[7];

#ifdef EXPLORE_MODE
	if(discover)		/* save bones files for real games */
		return(0);
#endif
	/* wizard check added by GAN 02/05/87 */
	if(rn2(3)	/* only once in three times do we find bones */
#ifdef WIZARD
		&& !wizard
#endif
		) return(0);
	if(no_bones_level(&u.uz)) return(0);
	fd = open_bonesfile(&u.uz, &bonesid);
	if (fd < 0) return(0);

	if((ok = uptodate(fd)) != 0){
#ifdef WIZARD
		if(wizard)  {
			if(yn("Get bones?") == 'n') {
				(void) close(fd);
				compress_bonesfile();
				return(0);
			}
		}
#endif
		minit();	/* ZEROCOMP */
		mread(fd, (genericptr_t) oldbonesid, 7);	/* DD.nnn */
		if (strcmp(bonesid, oldbonesid)) {
#ifdef WIZARD
			if (wizard) {
				pline("This is bones level '%s', not '%s'!",
					oldbonesid, bonesid);
				ok = FALSE;	/* won't die of trickery */
			}
#endif
			trickery();
		} else {
			register struct monst *mtmp;

			getlev(fd, 0, 0, TRUE);

			/* to correctly reset named artifacts on the level */
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				resetobjs(mtmp->minvent,TRUE);
			resetobjs(fobj,TRUE);
		}
	}
	(void) close(fd);

#ifdef WIZARD
	if(wizard) {
		if(yn("Unlink bones?") == 'n') {
			compress_bonesfile();
			return(ok);
		}
	}
#endif
	if (!delete_bonesfile(&u.uz)) {
		pline("Cannot unlink bones.");
		return(0);
	}
	return(ok);
}

/*bones.c*/
