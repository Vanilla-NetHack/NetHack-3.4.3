/*	SCCS Id: @(#)minion.c	3.1	92/11/01	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "emin.h"
#include "epri.h"

void
msummon(ptr)		/* ptr summons a monster */
	register struct permonst *ptr;
{
	register int dtype = 0, cnt = 0;

	if(is_dprince(ptr) || (ptr == &mons[PM_WIZARD_OF_YENDOR])) {

	    dtype = (!rn2(20)) ? dprince() : (!rn2(4)) ? dlord() : ndemon();
	    cnt = (!rn2(4) && !is_dprince(&mons[dtype])) ? 2 : 1;

	} else if(is_dlord(ptr)) {

	    dtype = (!rn2(50)) ? dprince() : (!rn2(20)) ? dlord() : ndemon();
	    cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;

	} else if(is_ndemon(ptr)) {

	    dtype = (!rn2(20)) ? dlord() : (!rn2(6)) ? ndemon() : monsndx(ptr);
	    cnt = 1;
	} else if(is_lminion(ptr)) {

	    dtype = (is_lord(ptr) && !rn2(20)) ? llord() :
		     (is_lord(ptr) || !rn2(6)) ? lminion() : monsndx(ptr);
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;

	}

	if(!dtype) return;

	while(cnt > 0) {

	    (void)makemon(&mons[dtype], u.ux, u.uy);
	    cnt--;
	}
	return;
}

void
summon_minion(alignment, talk)
    aligntyp alignment;
    boolean talk;
{
    register struct monst *mon;
    int mnum;

    switch(alignment) {
    case A_LAWFUL: {
	mnum = lminion();
	break;
    }
    case A_NEUTRAL: {
	mnum = PM_AIR_ELEMENTAL + rn2(4);
	break;
    }
    case A_CHAOTIC:
	mnum = ndemon();
	break;
    default:
	impossible("unaligned player?");
	mnum = ndemon();
	break;
    }
    if(mons[mnum].pxlth == 0) {
	struct permonst *pm = &mons[mnum];
	pm->pxlth = sizeof(struct emin);
	mon = makemon(pm, u.ux, u.uy);
	pm->pxlth = 0;
	if(mon) {
	    mon->isminion = TRUE;
	    EMIN(mon)->min_align = alignment;
	}
    } else if (mnum == PM_ANGEL) {
	mon = makemon(&mons[mnum], u.ux, u.uy);
	if (mon) {
	    mon->isminion = TRUE;
	    EPRI(mon)->shralign = alignment;	/* always A_LAWFUL here */
	}
    } else
	mon = makemon(&mons[mnum], u.ux, u.uy);
    if(mon) {
	if(talk) {
	    pline("The voice of %s booms:", align_gname(alignment));
	    verbalize("Thou shalt pay for thy indiscretion!");
	    if(!Blind)
		pline("%s appears before you.", Amonnam(mon));
	}
	mon->mpeaceful = FALSE;
	/* don't call set_malign(); player was naughty */
    }
}

#define	Athome	(Inhell && !mtmp->cham)

int
demon_talk(mtmp)		/* returns 1 if it won't attack. */
register struct monst *mtmp;
{
	long	demand, offer;

	if(uwep && uwep->oartifact == ART_EXCALIBUR) {
	    pline("%s looks very angry.", Amonnam(mtmp));
	    mtmp->mpeaceful = mtmp->mtame = 0;
	    newsym(mtmp->mx, mtmp->my);
	    return 0;
	}

	/* Slight advantage given. */
	if(is_dprince(mtmp->data) && mtmp->minvis) {
	    mtmp->minvis = 0;
	    if (!Blind) pline("%s appears before you.", Amonnam(mtmp));
	    newsym(mtmp->mx,mtmp->my);
	}
	if(u.usym == S_DEMON) {	/* Won't blackmail their own. */

	    pline("%s says, \"Good hunting, %s.\" and vanishes.",
		  Amonnam(mtmp), flags.female ? "Sister" : "Brother");
	    rloc(mtmp);
	    return(1);
	}
	demand = (u.ugold * (rnd(80) + 20 * Athome)) / 100;
	if(!demand)  		/* you have no gold */
	    return mtmp->mpeaceful = 0;
	else {

	    pline("%s demands %ld zorkmid%s for safe passage.",
		  Amonnam(mtmp), demand, plur(demand));

	    if((offer = bribe(mtmp)) >= demand) {
		pline("%s vanishes, laughing about cowardly mortals.",
		      Amonnam(mtmp));
	    } else {
		if((long)rnd(40) > (demand - offer)) {
		    pline("%s scowls at you menacingly, then vanishes.",
			  Amonnam(mtmp));
		} else {
		    pline("%s gets angry...", Amonnam(mtmp));
		    return mtmp->mpeaceful = 0;
		}
	    }
	}
	mongone(mtmp);
	return(1);
}

long
bribe(mtmp)
struct monst *mtmp;
{
	char buf[80];
	long offer;

	getlin("How much will you offer?", buf);
	(void) sscanf(buf, "%ld", &offer);

/*Michael Paddon -- fix for negative offer to monster*/	/*JAR880815 - */
 	if(offer < 0L) {
 		You("try to shortchange %s, but fumble.", 
 			mon_nam(mtmp));
 		offer = 0L;
 	} else if(offer == 0L) {
		You("refuse.");
 	} else if(offer >= u.ugold) {
		You("give %s all your gold.", mon_nam(mtmp));
		offer = u.ugold;
	} else You("give %s %ld zorkmid%s.", mon_nam(mtmp), offer,
		   plur(offer));

	u.ugold -= offer;
	mtmp->mgold += offer;
	flags.botl = 1;
	return(offer);
}

int
dprince() {
	int	tryct, pm;

	for(tryct = 0; tryct < 20; tryct++) {
	    pm = rn1(PM_DEMOGORGON + 1 - PM_ORCUS, PM_ORCUS);
	    if(!(mons[pm].geno & (G_GENOD | G_EXTINCT)))
		return(pm);
	}
	return(dlord());	/* approximate */
}

int
dlord()
{
	int	tryct, pm;

	for(tryct = 0; tryct < 20; tryct++) {
	    pm = rn1(PM_YEENOGHU + 1 - PM_JUIBLEX, PM_JUIBLEX);
	    if(!(mons[pm].geno & (G_GENOD | G_EXTINCT)))
		return(pm);
	}
	return(ndemon());	/* approximate */
}

/* create lawful (good) lord */
int
llord()
{
	if(!(mons[PM_ARCHON].geno & (G_GENOD | G_EXTINCT)))
		return(PM_ARCHON);

	return(lminion());	/* approximate */
}

int
lminion()
{
	int	tryct;
	struct	permonst *ptr;

	for(tryct = 0; tryct < 20; tryct++)
	    if((ptr = mkclass(S_ANGEL,0)) && !is_lord(ptr))
		return(monsndx(ptr));

	return(0);
}

int
ndemon()
{
	int	tryct;
	struct	permonst *ptr;

	for (tryct = 0; tryct < 20; tryct++) {
	    ptr = mkclass(S_DEMON, 0);
	    if (is_ndemon(ptr))
		return(monsndx(ptr));
	}

	return(0);
}

/*minion.c*/
