/*	SCCS Id: @(#)bones.c	3.0	88/04/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef TOS
#define OMASK	0x8000
#else
#define OMASK	0
#endif

#ifdef DGK
char bones[FILENAME];
#else
char bones[] = "bones.xx";
#endif

#ifdef COMPRESS
static char cmd[60], proxy[20];

static void
compress_bones()
{
	Strcpy(cmd, COMPRESS);
	Strcat(cmd, " ");
# ifdef COMPRESS_OPTIONS
	Strcat(cmd, COMPRESS_OPTIONS);
	Strcat(cmd, " ");
# endif
	Strcat(cmd, bones);
	(void) system(cmd);
}
#endif /* COMPRESS */

static boolean
no_bones_level(lev)
int lev;
{
	return (lev == medusa_level ||
		lev == wiz_level
#ifdef STRONGHOLD
		|| lev == stronghold_level ||
		(lev >= tower_level && lev <= tower_level+2)
#endif
#ifdef ENDGAME
		|| lev == ENDLEVEL
#endif
		);
}

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

/* save bones and possessions of a deceased adventurer */
void
savebones(){
	register int fd, x, y;
	register struct obj *otmp;
	register struct trap *ttmp;
	register struct monst *mtmp, *mtmp2;
	struct fruit *f;

	if(dlevel <= 0 || dlevel > MAXLEVEL) return;
	if(no_bones_level(dlevel)) return; /* no bones for specific levels */
	if(!rn2(1 + (dlevel>>2)) /* not so many ghosts on low levels */
#ifdef WIZARD
		&& !wizard
#endif
		) return;
#ifdef EXPLORE_MODE
	/* don't let multiple restarts generate multiple copies of objects
	 * in bones files */
	if(discover) return;
#endif

	name_file(bones, dlevel);
#ifdef COMPRESS
	Strcpy(proxy, bones);
	Strcat(proxy, ".Z");

	if((fd = open(proxy, OMASK)) >= 0) {
#else
	if((fd = open(bones, OMASK)) >= 0) {
#endif
		(void) close(fd);
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
		if(mtmp->iswiz) mongone(mtmp);
#ifdef MEDUSA
		if(mtmp->data == &mons[PM_MEDUSA]) mongone(mtmp);
#endif
	}
	/* mark all fruits as nonexistent; when we come to them we'll mark
	 * them as existing (using goodfruit())
	 */
	for(f=ffruit; f; f=f->nextf) f->fid = -f->fid;

	/* drop everything; the corpse's possessions are usually cursed */
	otmp = invent;
	while(otmp) {
		otmp->ox = u.ux;
		otmp->oy = u.uy;
		otmp->owornmask = 0;
		if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
		if(rn2(5)) curse(otmp);
		if(!otmp->nobj){
			otmp->nobj = fobj;
			fobj = invent;
			invent = 0;	/* superfluous */
			levl[u.ux][u.uy].omask = 1;
			break;
		}
		otmp = otmp->nobj;
	}
	if (u.ugrave_arise == -1) {
		if(!(mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy))) return;
		Strcpy((char *) mtmp->mextra, plname);
	} else {
		in_mklev = TRUE;
	/* tricks makemon() into allowing monster creation on your square */
		mons[u.ugrave_arise].pxlth += strlen(plname);
		mtmp = makemon(&mons[u.ugrave_arise], u.ux, u.uy);
		mons[u.ugrave_arise].pxlth -= strlen(plname);
		in_mklev = FALSE;
		if (!mtmp) return;
		Strcpy(NAME(mtmp), plname);
		mtmp->mnamelth = strlen(plname);
		atl(u.ux, u.uy, mtmp->data->mlet);
		Your("body rises from the dead as a%s %s...",
			index(vowels, *(mons[u.ugrave_arise].mname)) ? "n" : "",
			mons[u.ugrave_arise].mname);
	}
	mtmp->m_lev = (u.ulevel ? u.ulevel : 1);
	mtmp->mhp = mtmp->mhpmax = u.uhpmax;
	mtmp->msleep = 1;
	if(u.ugold) mkgold(u.ugold, u.ux, u.uy);
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
		    otmp->dknown = otmp->bknown = 0;
		    if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
		    if(uses_known(otmp)) otmp->known = 0;
		    if(otmp->otyp == AMULET_OF_YENDOR && !otmp->spe) {
			otmp->spe = -1;  /* no longer the actual amulet */
			curse(otmp);
		    }
		}
		mtmp->m_id = 0;
		mtmp->mlstmv = 0L;
		if(mtmp->mtame) mtmp->mtame = mtmp->mpeaceful = 0;
		if(mtmp->mdispl) unpmon(mtmp);
	}
	for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
		ttmp->tseen = 0;

	for(otmp = fobj; otmp; otmp = otmp->nobj)  {

		otmp->o_id = 0;
		if (((otmp->otyp != CORPSE && otmp->otyp != STATUE)
				|| otmp->corpsenm < PM_ARCHEOLOGIST)
#ifdef NAMED_ITEMS
				&& !is_artifact(otmp)
#endif
		   )
			otmp->onamelth = 0;
		if(uses_known(otmp)) otmp->known = 0;
		if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
		otmp->dknown = otmp->bknown = 0;
		otmp->invlet = 0;
#ifdef MAIL
		if (otmp->otyp == SCR_MAIL)
			otmp->spe = 1;
#endif
#ifdef POLYSELF
		if (otmp->otyp == EGG)
			otmp->spe = 0;
#endif
		if(otmp->otyp == AMULET_OF_YENDOR && !otmp->spe) {
			otmp->spe = -1;      /* no longer the actual amulet */
			curse(otmp);
		}
	}

	for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++)
		levl[x][y].seen = levl[x][y].new = levl[x][y].scrsym = 0;

#ifdef MSDOS
	fd = open(bones, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
	fd = creat(bones, FCMASK);
#endif
	if(fd < 0) {
#ifdef WIZARD
		if(wizard)
			pline("Cannot create bones file - creat failed");
#endif
		return;
	}
	savefruitchn(fd);
#ifdef DGK
	savelev(fd,dlevel, COUNT | WRITE);
#else
	savelev(fd,dlevel);
#endif
#ifdef ZEROCOMP
	bflush(fd);
#endif
	(void) close(fd);
#ifdef COMPRESS
	compress_bones();
#endif
}

int
getbones() {
	register int fd;
	register int ok;

	/* wizard check added by GAN 02/05/87 */
	if(rn2(3)	/* only once in three times do we find bones */
#ifdef WIZARD
		&& !wizard
#endif
		) return(0);
	if(no_bones_level(dlevel)) return(0);
	name_file(bones, dlevel);
#ifdef COMPRESS
	if((fd = open(bones, OMASK)) >= 0) goto gotbones;
	Strcpy(proxy, bones);
	Strcat(proxy, ".Z");
	if((fd = open(proxy, OMASK)) < 0) return(0);
	else {
	    (void) close(fd);
	    Strcpy(cmd, COMPRESS);
	    Strcat(cmd, " -d ");	/* uncompress */
# ifdef COMPRESS_OPTIONS
	    Strcat(cmd, COMPRESS_OPTIONS);
	    Strcat(cmd, " ");
# endif
	    Strcat(cmd,proxy);
	    (void) system(cmd);
	}
#endif
	if((fd = open(bones, OMASK)) < 0) return(0);
#ifdef COMPRESS
gotbones:
#endif
	if((ok = uptodate(fd)) != 0){
#ifdef WIZARD
		if(wizard)  {
			pline("Get bones? ");
			if(yn() == 'n') {
				(void) close(fd);
# ifdef COMPRESS
				compress_bones();
# endif
				return(0);
			}
		}
#endif
#ifdef ZEROCOMP
		minit();
#endif
		getlev(fd, 0, dlevel, TRUE);
	}
	(void) close(fd);
#ifdef WIZARD
	if(wizard) {
		pline("Unlink bones? ");
		if(yn() == 'n') {
# ifdef COMPRESS
			compress_bones();
# endif
			return(ok);
		}
	}
#endif
	if(unlink(bones) < 0){
		pline("Cannot unlink %s.", bones);
		return(0);
	}
	return(ok);
}

/* construct the string  file.level 
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'bones' and 'level'
 * but be careful if you use it for other things -dgk
 */
void
name_file(file, level)
char *file;
int level;
{
	char *tf;

	if (tf = rindex(file, '.'))
	    Sprintf(tf+1, "%d", level);
#ifdef MSDOS /* for glo() */
	else if (tf = eos(file))
	    Sprintf(tf, ".%d", level);
#endif
	return;
}
