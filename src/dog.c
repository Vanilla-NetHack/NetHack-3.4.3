/*	SCCS Id: @(#)dog.c	3.0	89/06/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

char dogname[63] = DUMMY;
char catname[63] = DUMMY;

#define domestic(mtmp)	(mtmp->data->msound == MS_BARK || mtmp->data->msound == MS_MEW)

void
initedog(mtmp)
register struct monst *mtmp;
{
	mtmp->mtame = domestic(mtmp) ? 10 : 5;
	mtmp->mpeaceful = 1;
	mtmp->mleashed = 0;
	mtmp->meating = 0;
	EDOG(mtmp)->droptime = 0;
	EDOG(mtmp)->dropdist = 10000;
	EDOG(mtmp)->apport = 10;
	EDOG(mtmp)->whistletime = 0;
	EDOG(mtmp)->hungrytime = 1000 + moves;
}

void
make_familiar(otmp)
register struct obj *otmp;
{
	register struct monst *mtmp;
	register struct permonst *pm;

top:
	if (otmp) pm = &mons[otmp->corpsenm]; /* Figurine; otherwise spell */
	else if (rn2(3)) {
	    if (!(pm = rndmonst())) {
		pline("There seems to be nothing available for a familiar.");
		return;
	    }
	}
	else if ((pl_character[0]=='W' || rn2(2)) && pl_character[0]!='C')
		pm = &mons[PM_KITTEN];
	else pm = &mons[PM_LITTLE_DOG];

	pm->pxlth += sizeof(struct edog);
	mtmp = makemon(pm, u.ux, u.uy);
	pm->pxlth -= sizeof(struct edog);
	if (!mtmp) { /* monster was genocided */
	    if (otmp)
		pline("The figurine writhes and then shatters into pieces!");
	    else goto top;
		/* rndmonst() returns something not genocided always, so this
		 * means it was a cat or dog; loop back to try again until
		 * either rndmonst() is called, or if only one of cat/dog
		 * was genocided, they get the other.
		 */
	    return;
	}
	initedog(mtmp);
	if (otmp && otmp->cursed) { /* cursed figurine */
		You("get a bad feeling about this.");
		mtmp->mtame = mtmp->mpeaceful = 0;
	}
}

struct monst *
makedog() {
	register struct monst *mtmp;
	register char *petname;

	if (pl_character[0]=='C' || (pl_character[0] != 'W' && rn2(2))) {
		mons[PM_LITTLE_DOG].pxlth = sizeof(struct edog);
		mtmp = makemon(&mons[PM_LITTLE_DOG], u.ux, u.uy);
		mons[PM_LITTLE_DOG].pxlth = 0;
		petname = dogname;
	} else {
		mons[PM_KITTEN].pxlth = sizeof(struct edog);
		mtmp = makemon(&mons[PM_KITTEN], u.ux, u.uy);
		mons[PM_KITTEN].pxlth = 0;
		petname = catname;
	}

	if(!mtmp) return((struct monst *) 0); /* dogs were genocided */

	if (petname[0]) {
		register struct monst *mtmp2;

		mtmp->mnamelth = strlen(petname) + 1;
		mtmp2 = newmonst(sizeof(struct edog) + mtmp->mnamelth);
		*mtmp2 = *mtmp;

		replmon(mtmp, mtmp2);
		mtmp = mtmp2;
		Strcpy(NAME(mtmp), petname);
		petname[0] = '\0'; /* name first only; actually unnecessary */
	}
	initedog(mtmp);
	return(mtmp);
}

/* attach the monsters that went down (or up) together with @ */
struct monst *mydogs = 0;
/* monsters that fell through a trapdoor or stepped on a tele-trap. */
/* 'down' is now true only of trapdooor falling, not for tele-trap. */
struct monst *fallen_down = 0;
				
void
losedogs(){
	register struct monst *mtmp,*mtmp0,*mtmp2;

	while(mtmp = mydogs){
		mydogs = mtmp->nmon;
		mtmp->nmon = fmon;
		fmon = mtmp;
		mnexto(mtmp);
	}
#ifdef LINT
	mtmp0 = (struct monst *)0;
#endif
	for(mtmp = fallen_down; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if(mtmp->mx == dlevel) {
		    mtmp->mx = 0;
		    if(mtmp == fallen_down)
			fallen_down = mtmp->nmon;
		    else
			mtmp0->nmon = mtmp->nmon;
		    mtmp->nmon = fmon;
		    fmon = mtmp;
		    if (mtmp->data->geno & G_GENOD) {
#ifdef KOPS
			allow_kops = FALSE;
#endif
			mondead(mtmp);	/* must put in fmon list first */
#ifdef KOPS
			allow_kops = TRUE;
#endif
		    } else if (mtmp->isshk)
			home_shk(mtmp);
		    else
			rloc(mtmp);
		} else
		    mtmp0 = mtmp;
	}
}

void
keepdogs(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(((dist(mtmp->mx,mtmp->my) < 3 && levl_follower(mtmp)) ||
		/* the wiz will level t-port from anywhere to chase
		   the amulet; if you don't have it, will chase you
		   only if in range. -3. */
			(u.uhave_amulet && mtmp->iswiz))
			&& !mtmp->msleep && !mtmp->mfroz) {
#ifdef WORM
		/* Bug "fix" for worm changing levels collapsing dungeon
		 */
		if (mtmp->data == &mons[PM_LONG_WORM]) {
			if (canseemon(mtmp) || (Blind && Telepat))
				pline("The worm can't fit down the stairwell.");
# ifdef WALKIES
			if (mtmp->mleashed) {
				pline("The leash slides off the slimy worm.");
				m_unleash(mtmp);
			}
# endif
			continue;
		}
#endif
		if (mon_has_amulet(mtmp)) {
			pline("%s seems very disoriented for a moment.",
				Monnam(mtmp));
#ifdef WALKIES
			if (mtmp->mleashed) {
				pline("%s leash suddenly comes loose.",
					is_female(mtmp) ? "Her" :
					humanoid(mtmp->data) ? "His" : "Its");
				m_unleash(mtmp);
			}
#endif
			continue;
		}
		relmon(mtmp);
		mtmp->mx = mtmp->my = 0; /* to avoid mnexto()/mmask problem */
		mtmp->nmon = mydogs;
		mydogs = mtmp;
		unpmon(mtmp);
		keepdogs();	/* we destroyed the link, so use recursion */
		return;		/* (admittedly somewhat primitive) */
	}
}

void
fall_down(mtmp, tolev) 
register struct monst *mtmp; 
register int tolev;
{
	relmon(mtmp);
	mtmp->nmon = fallen_down;
	fallen_down = mtmp;
#ifdef WALKIES
	if (mtmp->mleashed)  {
		pline("The leash comes off!");
		m_unleash(mtmp);
	}
#endif
	unpmon(mtmp);
	mtmp->mtame = 0;
	mtmp->mx = tolev; 
	mtmp->my = 0;
		/* make sure to reset mtmp->mx to 0 when releasing, */
		/* so rloc() on next level doesn't affect mmask */
}

/* return quality of food; the lower the better */
/* fungi will eat even tainted food */
int
dogfood(mon,obj)
struct monst *mon;
register struct obj *obj;
{
	boolean carni = carnivorous(mon->data);
	boolean herbi = herbivorous(mon->data);

	switch(obj->olet) {
	case FOOD_SYM:
	    if (obj->otyp == CORPSE && obj->corpsenm == PM_COCKATRICE &&
		!resists_ston(mon->data))
		    return TABU;

	    if (!carni && !herbi)
		    return (obj->cursed ? UNDEF : APPORT);

	    switch (obj->otyp) {
		case TRIPE_RATION:
		    return (carni ? DOGFOOD : MANFOOD);
		case EGG:
		    if (obj->corpsenm == PM_COCKATRICE &&
						!resists_ston(mon->data))
			return POISON;
		    return (carni ? CADAVER : MANFOOD);
		case CORPSE:
		    if ((obj->age+50 <= moves && mon->data->mlet != S_FUNGUS) ||
			(poisonous(&mons[obj->corpsenm]) &&
						!resists_poison(mon->data)))
			return POISON;
		    else return (carni ? CADAVER : MANFOOD);
		case DEAD_LIZARD:
		    return (carni ? ACCFOOD : MANFOOD);
		case CLOVE_OF_GARLIC:
		    return (is_undead(mon->data) ? TABU :
			    (herbi ? ACCFOOD : MANFOOD));
		case TIN:
		    return MANFOOD;
		case APPLE:
		case CARROT:
		    return (herbi ? DOGFOOD : MANFOOD);
		default:
#ifdef TUTTI_FRUTTI
		    return (obj->otyp > SLIME_MOLD ?
#else
		    return (obj->otyp > CLOVE_OF_GARLIC ?
#endif
			    (carni ? ACCFOOD : MANFOOD) :
			    (herbi ? ACCFOOD : MANFOOD));
	    }
	default:
	    if(!obj->cursed) return(APPORT);
	    /* fall into next case */
	case BALL_SYM:
	case CHAIN_SYM:
	case ROCK_SYM:
	    return(UNDEF);
	}
}

/* return roomnumber or -1 */
int
inroom(x,y) xchar x,y; {
	register struct mkroom *croom = &rooms[0];
	while(croom->hx >= 0){
		if(croom->hx >= x-1 && croom->lx <= x+1 &&
		   croom->hy >= y-1 && croom->ly <= y+1)
			return(croom - rooms);
		croom++;
	}
	return(-1);	/* not in room or on door */
}

int
tamedog(mtmp, obj)
register struct monst *mtmp;
register struct obj *obj;
{
	register struct monst *mtmp2;

	/* worst case, at least he'll be peaceful. */
	mtmp->mpeaceful = 1;
	if(flags.moonphase == FULL_MOON && night() && rn2(6) && obj
						&& mtmp->data->mlet == S_DOG)
		return(0);

	/* If we cannot tame him, at least he's no longer afraid. */
	mtmp->mflee = 0;
	mtmp->mfleetim = 0;
	if(mtmp->mtame || mtmp->mfroz ||
#ifdef MEDUSA
	   mtmp->data == &mons[PM_MEDUSA] ||
#endif
	   mtmp->isshk || mtmp->isgd ||
#if defined(ALTARS) && defined(THEOLOGY)
	   mtmp->ispriest ||
#endif
#ifdef POLYSELF
	   is_human(mtmp->data) || (is_demon(mtmp->data) && !is_demon(uasmon)))
#else
	   is_human(mtmp->data) || is_demon(mtmp->data))
#endif
		return(0);
	if(obj) {
		if(dogfood(mtmp, obj) >= MANFOOD) return(0);
		if(cansee(mtmp->mx,mtmp->my))
			pline("%s devours the %s.", Monnam(mtmp), xname(obj));
		obfree(obj, (struct obj *)0);
	}
	mtmp2 = newmonst(sizeof(struct edog) + mtmp->mnamelth);
	*mtmp2 = *mtmp;
	mtmp2->mxlth = sizeof(struct edog);
	if(mtmp->mnamelth) Strcpy(NAME(mtmp2), NAME(mtmp));
	initedog(mtmp2);
	replmon(mtmp,mtmp2);
	return(1);
}
