/*	SCCS Id: @(#)bones.c	3.0	88/04/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#define OMASK	0

#ifdef DGK
char bones[FILENAME];
extern long bytes_counted;
#else
char bones[] = "bones.xxxx";
#endif

#ifdef COMPRESS
static char cmd[60], proxy[20];

static void NDECL(compress_bones);
#endif
static boolean FDECL(no_bones_level, (int));
static void FDECL(resetobjs,(struct obj *,BOOLEAN_P));
#ifdef TUTTI_FRUTTI
static void FDECL(goodfruit, (int));
#endif

#ifdef COMPRESS
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
	extern int save_dlevel;		/* in do.c */

	if (save_dlevel) lev = save_dlevel;

	return (lev == medusa_level ||
		lev == wiz_level
#ifdef REINCARNATION
		|| lev == rogue_level
#endif
#ifdef STRONGHOLD
		|| lev == stronghold_level ||
		(lev >= tower_level && lev <= tower_level+2)
#endif
#ifdef ENDGAME
		|| lev == ENDLEVEL
#endif
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
		if (((otmp->otyp != CORPSE && otmp->otyp != STATUE)
			|| otmp->corpsenm < PM_ARCHEOLOGIST)
#ifdef NAMED_ITEMS
			&& (!is_artifact(otmp) ||
			    (exist_artifact(otmp,ONAME(otmp)) && restore))
#endif
		) {
			otmp->onamelth = 0;
			*ONAME(otmp) = '\0';
		}
#ifdef NAMED_ITEMS
		else if (is_artifact(otmp) && restore)
			artifact_exists(otmp,ONAME(otmp),TRUE);
#endif
		if (!restore) {
			/* resetting the o_id's after getlev has carefully
			 * created proper new ones via restobjchn is a Bad
			 * Idea */
			otmp->o_id = 0;
			if(objects[otmp->otyp].oc_uses_known) otmp->known = 0;
			otmp->dknown = otmp->bknown = 0;
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
			if(otmp->otyp == AMULET_OF_YENDOR && !otmp->spe) {
				otmp->spe = -1;
				/* no longer the actual amulet */
				curse(otmp);
			}
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
#ifdef TUTTI_FRUTTI
	struct fruit *f;
#endif

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
#ifdef TUTTI_FRUTTI
	/* mark all fruits as nonexistent; when we come to them we'll mark
	 * them as existing (using goodfruit())
	 */
	for(f=ffruit; f; f=f->nextf) f->fid = -f->fid;
#endif

	/* check iron balls separately--maybe they're not carrying it */
	if (uball) uball->owornmask = uchain->owornmask = 0;

	/* drop everything; the corpse's possessions are usually cursed */
	otmp = invent;
	while(otmp) {
		place_object(otmp, u.ux, u.uy);
		otmp->owornmask = 0;
#ifdef TUTTI_FRUTTI
		if(otmp->otyp == SLIME_MOLD) goodfruit(otmp->spe);
#endif
		if(rn2(5)) curse(otmp);
		if(!otmp->nobj){
			otmp->nobj = fobj;
			fobj = invent;
			invent = 0;	/* superfluous */
			break;
		}
		otmp = otmp->nobj;
	}
	in_mklev = TRUE;
	/* tricks makemon() into allowing monster creation on your square */
	if (u.ugrave_arise == -1) {
		mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy);
		in_mklev = FALSE;
		if (!mtmp) return;
		Strcpy((char *) mtmp->mextra, plname);
	} else {
		mtmp = makemon(&mons[u.ugrave_arise], u.ux, u.uy);
		in_mklev = FALSE;
		if (!mtmp) return;
		mtmp = christen_monst(mtmp, plname);
		atl(u.ux, u.uy, mtmp->data->mlet);
		Your("body rises from the dead as %s...",
			an(mons[u.ugrave_arise].mname));
		more();
	}
	mtmp->m_lev = (u.ulevel ? u.ulevel : 1);
	mtmp->mhp = mtmp->mhpmax = u.uhpmax;
	mtmp->msleep = 1;
	if(u.ugold) mkgold(u.ugold, u.ux, u.uy);
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		resetobjs(mtmp->minvent,FALSE);
		mtmp->m_id = 0;
		mtmp->mlstmv = 0L;
		if(mtmp->mtame) mtmp->mtame = mtmp->mpeaceful = 0;
		if(mtmp->mdispl) unpmon(mtmp);
	}
	for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
		ttmp->tseen = 0;

	resetobjs(fobj,FALSE);
	/* let's (not) forget about these - KCD, 10/21/89 */
	resetobjs(fcobj,FALSE);

	for(x=0; x<COLNO; x++) for(y=0; y<ROWNO; y++)
		levl[x][y].seen = levl[x][y].new = levl[x][y].scrsym = 0;

#ifdef MSDOS
	fd = open(bones, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
# ifdef MACOS
	{
		Str255	fileName;
		OSErr	er;
		struct term_info	*t;
		extern WindowPtr	HackWindow;
		short	oldvolume;
		
		t = (term_info *)GetWRefCon(HackWindow);
		(void)GetVol(&fileName,&oldvolume);
		(void)SetVol(0L, t->recordVRefNum);
		fileName[0] = (uchar)strlen(bones);
		Strcpy((char *)&fileName[1],bones);
		
		if (er = Create(&fileName,0,CREATOR,BONES_TYPE))
			SysBeep(1);
		fd = open(bones,
			O_WRONLY | O_BINARY | O_TRUNC | ((er) ? O_CREAT : 0));
		(void)SetVol(0L, oldvolume);
	}
# else
	fd = creat(bones, FCMASK);
# endif /* MACOS */
#endif
	if(fd < 0) {
#ifdef WIZARD
		if(wizard)
			pline("Cannot create bones file - creat failed");
#endif
		return;
	}

#if defined(DGK)	/* check whether there is room */
	count_only = TRUE;
# ifdef TUTTI_FRUTTI
	savefruitchn(fd);
# endif
	savelev(fd, dlevel, COUNT);
# ifdef ZEROCOMP
	bflush(fd);
# endif
	if (bytes_counted > freediskspace(bones)) {	/* not enough room */
# ifdef WIZARD
		if (wizard)
			pline("Insufficient space to create bones file.");
# endif
		unlink(bones);
		return;
	}
	count_only = FALSE;
#endif /* DGK */

#ifdef TUTTI_FRUTTI
	savefruitchn(fd);
#endif
#if defined(DGK)
	savelev(fd, dlevel, WRITE);
#else
	savelev(fd,dlevel);
#endif
#ifdef ZEROCOMP
	bflush(fd);
#endif
	(void) close(fd);
#if defined(VMS) && !defined(SECURE)
	/*
	   Re-protect bones file with world:read+write+execute+delete access.
	   umask() doesn't seem very reliable; also, vaxcrtl won't let us set
	   delete access without write access, which is what's really wanted.
	 */
	(void) chmod(bones, FCMASK | 007);  /* allow other users full access */
#endif
#ifdef MACOS
	{
		FInfo	fndrInfo;
		Str255	name;
		term_info	*t;
		short	oldVol, error;
		
		t = (term_info *)GetWRefCon(HackWindow);
		GetVol(name, &oldVol);
		SetVol(0L, t->recordVRefNum);  
		Strcpy((char *)name, bones);
		CtoPstr((char *)name);
		error = GetFInfo(name, (short)0, &fndrInfo);
		fndrInfo.fdCreator = CREATOR;
		fndrInfo.fdType = BONES_TYPE;
		if (error == noErr)
			SetFInfo(name, (short)0, &fndrInfo);
		SetVol(0L, oldVol);
	}
#endif /* MACOS */
#ifdef COMPRESS
	compress_bones();
#endif
}

int
getbones() {
	register int fd;
	register int ok;
#ifdef MACOS
	Str255	name;
	short	oldVol;
	term_info *t;
	extern WindowPtr	HackWindow;
	
	t = (term_info *)GetWRefCon(HackWindow);
#endif
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
	if(no_bones_level(dlevel)) return(0);
	name_file(bones, dlevel);
#ifdef MACOS
	GetVol(name, &oldVol);
	SetVol(0L, t->recordVRefNum);
#endif
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
	if((fd = open(bones, OMASK)) < 0) {
#ifdef MACOS
		SetVol(0L, oldVol);
#endif
		return(0);
	}
#ifdef MACOS
	SetVol(0L, oldVol);
#endif
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
#ifdef NAMED_ITEMS
	/* to correctly reset named artifacts on the level */
	{
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			resetobjs(mtmp->minvent,TRUE);
		resetobjs(fobj,TRUE);
		resetobjs(fcobj,TRUE);
	}
#endif
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
#ifdef MACOS
	GetVol(name, &oldVol);
	SetVol(0L, t->recordVRefNum);
#endif
	if(unlink(bones) < 0){
		pline("Cannot unlink %s.", bones);
#ifdef MACOS
		SetVol(0L, oldVol);
#endif
		return(0);
	}
#ifdef MACOS
	SetVol(0L, oldVol);
#endif
	return(ok);
}

/* construct the string  file.level 
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'bones' and 'level'
 * but be careful if you use it for other things -dgk
 */
void
name_file(file, lev)
char *file;
int lev;
{
	char *tf;

	if (tf = rindex(file, '.'))
#ifdef VMS
	    Sprintf(tf+1, "%d;1", lev);
#else
  	    Sprintf(tf+1, "%d", lev);
#endif
#ifdef MSDOS /* for glo() */
	else if (tf = eos(file))
	    Sprintf(tf, ".%d", lev);
#endif
	return;
}
