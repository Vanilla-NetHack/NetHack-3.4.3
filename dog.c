/*	SCCS Id: @(#)dog.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* dog.c - version 1.0.3 */

#include	"hack.h"
extern struct monst *makemon();
#include "edog.h"
#include "mkroom.h"

struct permonst li_dog =
	{ "little dog", 'd',2,18,6,0,1,6,sizeof(struct edog) };
struct permonst dog =
	{ "dog", 'd',4,16,5,0,1,6,sizeof(struct edog) };
struct permonst la_dog =
	{ "large dog", 'd',6,15,4,0,2,4,sizeof(struct edog) };

struct monst *
makedog(){
register struct monst *mtmp = makemon(&li_dog,u.ux,u.uy);
	if(!mtmp) return((struct monst *) 0); /* dogs were genocided */
	initedog(mtmp);
	return(mtmp);
}

initedog(mtmp) register struct monst *mtmp; {
	mtmp->mtame = mtmp->mpeaceful = 1;
#ifdef WALKIES
	mtmp->mleashed = 0;
#endif
	EDOG(mtmp)->hungrytime = 1000 + moves;
	EDOG(mtmp)->eattime = 0;
	EDOG(mtmp)->droptime = 0;
	EDOG(mtmp)->dropdist = 10000;
	EDOG(mtmp)->apport = 10;
	EDOG(mtmp)->whistletime = 0;
}

/* attach the monsters that went down (or up) together with @ */
struct monst *mydogs = 0;
struct monst *fallen_down = 0;	/* monsters that fell through a trapdoor */
	/* they will appear on the next level @ goes to, even if he goes up! */

losedogs(){
register struct monst *mtmp;
	while(mtmp = mydogs){
		mydogs = mtmp->nmon;
		mtmp->nmon = fmon;
		fmon = mtmp;
		mnexto(mtmp);
	}
	while(mtmp = fallen_down){
		fallen_down = mtmp->nmon;
		mtmp->nmon = fmon;
#ifdef WALKIES
		mtmp->mleashed = 0;
#endif
		fmon = mtmp;
		rloc(mtmp);
	}
}

keepdogs(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(dist(mtmp->mx,mtmp->my) < 3 && follower(mtmp)
		&& !mtmp->msleep && !mtmp->mfroz) {
#ifdef DGKMOD
		/* Bug "fix" for worm changing levels collapsing dungeon
		 */
		if (mtmp->data->mlet == 'w') {
			if (canseemon(mtmp) || (Blind && Telepat))
				pline("The worm can't fit down the stairwell!");
#ifdef WALKIES
			pline("The leash slides off the slimy worm!");
			mtmp->mleashed = 0;
#endif
			continue;
		}
#endif
		relmon(mtmp);
		mtmp->nmon = mydogs;
		mydogs = mtmp;
		unpmon(mtmp);
		keepdogs();	/* we destroyed the link, so use recursion */
		return;		/* (admittedly somewhat primitive) */
	}
}

fall_down(mtmp) register struct monst *mtmp; {
	relmon(mtmp);
	mtmp->nmon = fallen_down;
	fallen_down = mtmp;
#ifdef WALKIES
	if (mtmp->mleashed)  {

		pline("The leash comes off!");
		mtmp->mleashed = 0;
	}
#endif
	unpmon(mtmp);
	mtmp->mtame = 0;
}

/* return quality of food; the lower the better */
dogfood(obj) register struct obj *obj; {
	switch(obj->olet) {
	case FOOD_SYM:
	    return(
		(obj->otyp == TRIPE_RATION) ? DOGFOOD :
		(obj->otyp < CARROT) ? ACCFOOD :
		(obj->otyp < CORPSE) ? MANFOOD :
		(poisonous(obj) || obj->age + 50 <= moves ||
		    obj->otyp == DEAD_COCKATRICE)
			? POISON : CADAVER
	    );
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
inroom(x,y) xchar x,y; {
#ifndef QUEST
	register struct mkroom *croom = &rooms[0];
	while(croom->hx >= 0){
		if(croom->hx >= x-1 && croom->lx <= x+1 &&
		   croom->hy >= y-1 && croom->ly <= y+1)
			return(croom - rooms);
		croom++;
	}
#endif
	return(-1);	/* not in room or on door */
}

tamedog(mtmp, obj)
register struct monst *mtmp;
register struct obj *obj;
{
	register struct monst *mtmp2;

	/* worst case, at least he'll be peaceful. */
	mtmp->mpeaceful = 1;
	if(flags.moonphase == FULL_MOON && night() && rn2(6))
		return(0);

	/* If we cannot tame him, at least he's no longer afraid. */
	mtmp->mflee = 0;
	mtmp->mfleetim = 0;
	if(mtmp->mtame || mtmp->mfroz ||
#ifndef NOWORM
	   mtmp->wormno ||
#endif
	   mtmp->isshk || mtmp->isgd || index(" @12", mtmp->data->mlet))
		return(0);			/* no tame long worms? */
	if(obj) {
		if(dogfood(obj) >= MANFOOD) return(0);
		if(cansee(mtmp->mx,mtmp->my)){
			pline("%s devours the %s.", Monnam(mtmp),
				objects[obj->otyp].oc_name);
		}
		obfree(obj, (struct obj *) 0);
	}
	mtmp2 = newmonst(sizeof(struct edog) + mtmp->mnamelth);
	*mtmp2 = *mtmp;
	mtmp2->mxlth = sizeof(struct edog);
	if(mtmp->mnamelth) (void) strcpy(NAME(mtmp2), NAME(mtmp));
	initedog(mtmp2);
	replmon(mtmp,mtmp2);
	return(1);
}
