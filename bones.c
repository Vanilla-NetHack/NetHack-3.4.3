/*	SCCS Id: @(#)bones.c	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* bones.c - version 1.0.3 */

#include "hack.h"
extern char plname[PL_NSIZ];
extern long somegold();
extern struct monst *makemon();
extern struct permonst pm_ghost;

#ifdef DGK
char bones[FILENAME];
#else
char bones[] = "bones_xx";
#endif

/* save bones and possessions of a deceased adventurer */
savebones(){
register fd;
register struct obj *otmp;
register struct trap *ttmp;
register struct monst *mtmp;
	if(dlevel <= 0 || dlevel > MAXLEVEL) return;
	if(!rn2(1 + dlevel/2)	/* not so many ghosts on low levels */
#ifdef WIZARD
		&& !wizard
#endif
		) return;
#ifdef DGK
	name_file(bones, dlevel);
#else
	bones[6] = '0' + (dlevel/10);
	bones[7] = '0' + (dlevel%10);
#endif
	if((fd = open(bones,0)) >= 0){
		(void) close(fd);
#ifdef WIZARD
		if(wizard)
			pline("Bones file already exists.");
#endif
		return;
	}
	/* drop everything; the corpse's possessions are usually cursed */
	otmp = invent;
	while(otmp){
		otmp->ox = u.ux;
		otmp->oy = u.uy;
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
	/* spill any contained objects - added by GAN 03/23/87 */
	otmp = fcobj;
	while(otmp)  {
		register struct obj *otmp2;

		otmp2 = otmp->nobj;
		spill_obj(otmp);
		otmp = otmp2;
	}
	if(!(mtmp = makemon(PM_GHOST, u.ux, u.uy))) return;
	mtmp->mx = u.ux;
	mtmp->my = u.uy;
	mtmp->msleep = 1;
	(void) strcpy((char *) mtmp->mextra, plname);
	mkgold(somegold() + d(dlevel,30), u.ux, u.uy);
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		mtmp->m_id = 0;
		if(mtmp->mtame) {
			mtmp->mtame = 0;
			mtmp->mpeaceful = 0;
		}
		mtmp->mlstmv = 0;
		if(mtmp->mdispl) unpmon(mtmp);
	}
	for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
		ttmp->tseen = 0;
	for(otmp = fobj; otmp; otmp = otmp->nobj) {
		otmp->o_id = 0;
	     /* otmp->o_cnt_id = 0; - superfluous */
		otmp->onamelth = 0;
		otmp->known = 0;
		otmp->invlet = 0;
		if(otmp->olet == AMULET_SYM && !otmp->spe) {
			otmp->spe = -1;      /* no longer the actual amulet */
			otmp->cursed = 1;    /* flag as gotten from a ghost */
		}
	}
#ifdef DGK
	fd = open(bones, O_WRONLY | O_BINARY | O_CREAT, FMASK);
#else
	fd = creat(bones, FMASK);
#endif
	if(fd < 0) {
#ifdef WIZARD
		if(wizard)
			pline("Cannot create bones file - creat failed");
#endif
		return;
	}
#ifdef DGK
	savelev(fd,dlevel, COUNT | WRITE);
#else
	savelev(fd,dlevel);
#endif
	(void) close(fd);
}

/*
 * "spill" object out of box onto floor
 */
spill_obj(obj)
struct obj *obj;
{
	struct obj *otmp;

	for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(obj->o_cnt_id == otmp->o_id)  {
			obj->ox = otmp->ox;
			obj->oy = otmp->oy;
			obj->age = 0;
			if(rn2(5))
				obj->cursed = 1;
			obj->nobj = otmp->nobj;
			otmp->nobj = obj;
			return;
		}
}
		
getbones(){
register fd,x,y,ok;
	/* wizard check added by GAN 02/05/87 */
	if(rn2(3)	/* only once in three times do we find bones */
#ifdef WIZARD
		&& !wizard
#endif
		) return(0);
#ifdef DGK
	name_file(bones, dlevel);
#else
	bones[6] = '0' + dlevel/10;
	bones[7] = '0' + dlevel%10;
#endif
	if((fd = open(bones, 0)) < 0) return(0);
	if((ok = uptodate(fd)) != 0){
#ifdef WIZARD
		if(wizard)  {
			char buf[BUFSZ];
			pline("Get bones? ");
			getlin(buf);
			if(buf[0] == 'n')  {
				(void) close(fd);
				return(0);
			}
		}
#endif
		getlev(fd, 0, dlevel);
		for(x = 0; x < COLNO; x++) for(y = 0; y < ROWNO; y++)
			levl[x][y].seen = levl[x][y].new = 0;
	}
	(void) close(fd);
#ifdef WIZARD
	if(wizard)  {
		char buf[BUFSZ];
		pline("Unlink bones? ");
		getlin(buf);
		if(buf[0] == 'n')
			return(ok);
	}
#endif
	if(unlink(bones) < 0){
		pline("Cannot unlink %s .", bones);
		return(0);
	}
	return(ok);
}
