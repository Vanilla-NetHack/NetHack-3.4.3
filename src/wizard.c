/*	SCCS Id: @(#)wizard.c	2.3	88/02/11
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

/* wizard code - inspired by rogue code from Merlyn Leroy (digi-g!brian) */
/*	       - heavily modified to give the wiz balls.  (genat!mike)   */

#include "hack.h"
extern struct permonst pm_wizard;
extern struct monst *makemon();
extern struct obj *carrying(), *mksobj_at();

#if defined(HARD) || defined(DGKMOD)
# ifdef SAC
char	nasties[] = "cdDeImoPTUVwxXz3&,:;";
# else
char	nasties[] = "cdDeImoPTUVwxXz&,:;";
# endif
#define WIZSHOT	    2
#else
#define	WIZSHOT	    6	/* one chance in WIZSHOT that wizard will try magic */
#endif

#define	BOLT_LIM    8	/* from this distance D and 1 will try to hit you */

char wizapp[] = "@&DNPTUVXcemntx";

#ifdef DGKMOD
#define URETREATING(x,y) (movedist(u.ux,u.uy,x,y) > movedist(u.ux0,u.uy0,x,y))
extern char mlarge[];

movedist(x0, x1, y0, y1)
{
	register int absdx, absdy;

	absdx = abs(x1 - x0);
	absdy = abs(y1 - y0);

	return (absdx + absdy - min(absdx, absdy));
}
#endif

/* If he has found the Amulet, make the wizard appear after some time */
amulet(){
	register struct obj *otmp;
	register struct monst *mtmp;

	if(!flags.made_amulet || !flags.no_of_wizards)
		return;
	/* find wizard, and wake him if necessary */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->data->mlet == '1' && mtmp->msleep && !rn2(40))
		for(otmp = invent; otmp; otmp = otmp->nobj)
		    if(otmp->olet == AMULET_SYM && !otmp->spe) {
			mtmp->msleep = 0;
			if(dist(mtmp->mx,mtmp->my) > 2)
			    pline(
    "You get the creepy feeling that somebody noticed your taking the Amulet."
			    );
			return;
		    }
}

wiz_hit(mtmp)
register struct monst *mtmp;
{
	/* if we have stolen or found the amulet, we disappear */
	if(mtmp->minvent && mtmp->minvent->olet == AMULET_SYM &&
	    mtmp->minvent->spe == 0) {
		/* vanish -- very primitive */
		fall_down(mtmp);
		return(1);
	}

	/* if it is lying around someplace, we teleport to it */
	if(!carrying(AMULET_SYM)) {
	    register struct obj *otmp;

	    for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(otmp->olet == AMULET_SYM && !otmp->spe) {
		    if((u.ux != otmp->ox || u.uy != otmp->oy) &&
		       !m_at(otmp->ox, otmp->oy)) {

			/* teleport to it and pick it up */
			mtmp->mx = otmp->ox;
			mtmp->my = otmp->oy;
			freeobj(otmp);
			mpickobj(mtmp, otmp);
			pmon(mtmp);
			return(0);
		    }
		    goto hithim;
		}
	    return(0);				/* we don't know where it is */
	}
hithim:
	if(rn2(2)) {				/* hit - perhaps steal */

	    /* if hit 1/20 chance of stealing amulet & vanish
		- amulet is on level 26 again. */
	    if(hitu(mtmp, d(mtmp->data->damn,mtmp->data->damd))
		&& !rn2(20) && stealamulet(mtmp))
		;
	}
	else    inrange(mtmp);			/* try magic */
	return(0);
}

#ifdef DGKMOD
/* Check if a monster is carrying a particular item.
 */
struct obj *
m_carrying(mtmp, type)
struct monst *mtmp;
int type;
{
	register struct obj *otmp;

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == type)
			return(otmp);
	return((struct obj *) 0);
}

/* Remove an item from the monster's inventory.
 */
m_useup(mon, obj)
struct monst *mon;
struct obj *obj;
{
	struct obj *otmp, *prev;

	prev = ((struct obj *) 0);
	for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
		if (otmp == obj) {
			if (prev)
				prev->nobj = obj->nobj;
			else
				mon->minvent = obj->nobj;
			free((char *) obj);
			break;
		}
		prev = otmp;
	}
}

m_throw(x, y, dx, dy, range, obj)
register int x,y,dx,dy,range;		/* direction and range */
register struct obj *obj;
{
	register struct monst *mtmp;
	struct objclass *oclass = &objects[obj->otyp];
	char sym = obj->olet;
	int damage;
	extern char *exclam();

	bhitpos.x = x;
	bhitpos.y = y;

	if(sym) tmp_at(-1, sym);	/* open call */
	while(range-- > 0) {
		bhitpos.x += dx;
		bhitpos.y += dy;
		if(mtmp = m_at(bhitpos.x,bhitpos.y)) {
			damage = index(mlarge, mtmp->data->mlet)
				? oclass->wldam
				: oclass->wsdam;
#ifdef KAA
# ifdef KOPS
			if(obj->otyp == CREAM_PIE) damage = 0;
# endif
			if(mtmp->data->ac + 8 <= rnd(20))
				miss(oclass->oc_name, mtmp);
			else {
#endif
				hit(oclass->oc_name, mtmp, exclam(damage));
				mtmp->mhp -= damage;
				if(mtmp->mhp < 1) {
					pline("%s is killed!", (Blind) ? "It" : Monnam(mtmp));
					mondied(mtmp);
				}
				range = 0;
#ifdef KAA
# ifdef KOPS
				if(obj->otyp == CREAM_PIE) {

					pline("%s is blinded by the pie.", (Blind) ? "It" : Monnam(mtmp));
					if(mtmp->msleep) mtmp->msleep = 0;
					setmangry(mtmp);
					mtmp->mcansee = 0;
					mtmp->mblinded += rnd(25);
					if (mtmp->mblinded <= 0)
						mtmp->mblinded = 127;
				} else
# endif
				    if(obj->otyp == ENORMOUS_ROCK) {
					mksobj_at(ENORMOUS_ROCK, bhitpos.x, bhitpos.y);
					fobj->quan=1;
					stackobj(fobj);
				}
			}
#endif
		}
		if (bhitpos.x == u.ux && bhitpos.y == u.uy) {
			if (multi)
				nomul(0);
#ifdef KAA
/* For giants throwing rocks, the rock which hits you shouldn't disappear. */
# ifdef KOPS
/* Cream pies must disappear if they hit or miss. */
			{ int hit, blindinc, thitu();
			 if (!(hit = thitu(8, (obj->otyp != CREAM_PIE) ? rnd(oclass->wldam) : 0, oclass->oc_name))
			    && obj->otyp != CREAM_PIE
# else
			 if (!thitu(8, rnd(oclass->wldam), oclass->oc_name)
# endif /* KOPS /**/
			    || obj->otyp == ENORMOUS_ROCK) {
#else
			 if (!thitu(8, rnd(oclass->wldam), oclass->oc_name)) {
#endif /* KAA /**/
				mksobj_at(obj->otyp, u.ux, u.uy);
				fobj->quan = 1;
				stackobj(fobj);
			 }
#if defined(KAA) && defined(KOPS)
			 if(hit && obj->otyp == CREAM_PIE) {
			    if(!Blind)	pline("Yeech! You've been creamed.");
			    else	pline("There's something sticky all over your face.");
			    /* blindfold keeps pie filling out of your eyes */
			    if (!Blindfolded) {
				u.ucreamed += (blindinc = rnd(25));
				Blinded += blindinc;
				seeoff(0);
			    }
			 }
			}
#endif
			range = 0;
		}
		tmp_at(bhitpos.x, bhitpos.y);
#ifdef SINKS
		if(IS_SINK(levl[bhitpos.x][bhitpos.y].typ))
			break;	/* thrown objects fall on sink */
#endif
	}
	tmp_at(-1, -1);
}
#endif

/* Return 1 if it's OK for the monster to move as well as (throw,
 * zap, etc).
 */
inrange(mtmp)
register struct monst *mtmp;
{
	register schar tx,ty;
#ifdef DGKMOD
	struct obj *otmp;
	register xchar x, y;
#endif
	/* do nothing if cancelled (but make '1' say something) */
	if(mtmp->data->mlet != '1' && mtmp->mcan) return(1);

	/* spit fire only when both in a room or both in a corridor */
#ifndef RPH
	if(inroom(u.ux,u.uy) != inroom(mtmp->mx,mtmp->my)) return(1);
#endif
	tx = u.ux - mtmp->mx;
	ty = u.uy - mtmp->my;
#ifdef DGKMOD
	if ((!tx || !ty || abs(tx) == abs(ty))	/* straight line or diagonal */
		&& movedist(tx, 0,  ty, 0) < BOLT_LIM) {
		/* Check if there are any dead squares between.  If so,
		 * it won't be possible to shoot.
		 */
		for (x = mtmp->mx, y = mtmp->my; x != u.ux || y != u.uy;
				x += sgn(tx), y += sgn(ty))
			if (!ACCESSIBLE(levl[x][y].typ))
				return(1);

		switch(mtmp->data->mlet) {
#ifdef KOPS
		case 'O':
#endif
#ifdef KAA
		case '9':
#endif
		case 'K':
		case 'C':
		/* If you're coming toward the monster, the monster
		 * should try to soften you up with arrows.  If you're
		 * going away, you are probably hurt or running.  Give
		 * chase, but if you're getting too far away, throw.
		 */
		x = mtmp->mx;
		y = mtmp->my;
#ifdef KOPS
		otmp = (mtmp->data->mlet == 'O') ? m_carrying(mtmp, DART)
# ifdef KAA
		       : (mtmp->data->mlet == 'K') ? m_carrying(mtmp, CREAM_PIE)
# endif
#else
		otmp = (mtmp->data->mlet == 'K') ? m_carrying(mtmp, DART)
#endif
#ifdef KAA
			: (mtmp->data->mlet == '9') ? m_carrying(mtmp, ENORMOUS_ROCK)
#endif
			: m_carrying(mtmp, CROSSBOW_BOLT);
		if (otmp && (!URETREATING(x,y)
			|| !rn2(BOLT_LIM - movedist(x, u.ux, y, u.uy)))) {
				m_throw(mtmp->mx, mtmp->my, sgn(tx), sgn(ty),
					BOLT_LIM, otmp);
				if (!--otmp->quan )
					m_useup(mtmp, otmp);
				return(0);
			}
		break;
#else
	if((!tx && abs(ty) < BOLT_LIM) || (!ty && abs(tx) < BOLT_LIM)
	    || (abs(tx) == abs(ty) && abs(tx) < BOLT_LIM)){
	    switch(mtmp->mappearance) {
#endif
	    case 'D':
		/* spit fire in the direction of @ (not nec. hitting) */
		buzz((int) - 10 - (mtmp->dragon),
			mtmp->mx,mtmp->my,sgn(tx),sgn(ty));
		break;
#ifdef HARD
	    case '&':
		demon_hit(mtmp);
		break;
#endif
	    case '1':
		if(rn2(WIZSHOT)) break;
		/* if you zapped wizard with wand of cancellation,
		he has to shake off the effects before he can throw
		spells successfully.  Sometimes they fail anyway */
		if(mtmp->mcan ||
#ifdef HARD
		   !rn2(10)
#else
		   !rn2(2)
#endif
		   ) {
		    if(canseemon(mtmp))
				pline("%s makes a gesture, then curses.",
					Monnam(mtmp));
		    else	pline("You hear mumbled cursing.");

		    if(!rn2(3)) {
			mtmp->mspeed = 0;
			mtmp->minvis = 0;
		    }
		    if(!rn2(3))	mtmp->mcan = 0;

		} else {
		    if(canseemon(mtmp)){
			if(!rn2(6) && !Invis) {
			    pline("%s hypnotizes you.", Monnam(mtmp));
			    nomul(-rn2(3) + 3);	/* bug fix by ab@unido */
			    break;
			} else
			    pline("%s chants an incantation.", Monnam(mtmp));
		    } else
			    pline("You hear a mumbled incantation.");
		    switch(rn2(Invis ? 5 : 6)) {
		    case 0:
			/* create a nasty monster from a deep level */
			nasty();
			break;
		    case 1:
			pline("\"Destroy the thief, my pets!\"");
#ifdef HARD
			nasty();
#endif
			aggravate();	/* aggravate all the monsters */
			/* fall into next case */
		    case 2:
			if (flags.no_of_wizards == 1 && !rn2(3)) {
			    /* if only 1 wizard, clone himself */
			    pline("Double Trouble...");
			    clonewiz(mtmp);
			}
			break;
		    case 3:
			if(mtmp->mspeed == MSLOW)	mtmp->mspeed = 0;
			else				mtmp->mspeed = MFAST;
			break;
		    case 4:
			mtmp->minvis = 1;
			break;
		    case 5:
			/* Only if not Invisible */
			pline("You hear a clap of thunder!");
			/* shoot a bolt of fire or cold, or a sleep ray */
			/* or death, or lightning, but not  magic missile */
			buzz(-rnd(5),mtmp->mx,mtmp->my,sgn(tx),sgn(ty));
			break;
		    }
		}
	    }
	    if(u.uhp < 1) done_in_by(mtmp);
	}
	return(1);
}

aggravate()
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		mtmp->msleep = 0;
		if(mtmp->mfroz && !rn2(5))
			mtmp->mfroz = 0;
	}
}

clonewiz(mtmp)
register struct monst *mtmp;
{
	register struct monst *mtmp2;

	if(mtmp2 = makemon(PM_WIZARD, u.ux, u.uy)) {
		flags.no_of_wizards = 2;
		mtmp2->mtame = mtmp2->mpeaceful = 0;
		unpmon(mtmp2);
		mtmp2->mappearance = wizapp[rn2(sizeof(wizapp)-1)];
		pmon(mtmp2);
	}
}

nasty() {

#ifdef HARD
	register struct monst	*mtmp;
	struct monst	*mkmon_at();
	register int	i, nastynum, tmp;

	nastynum = sizeof(nasties) - 1;
	tmp = (u.ulevel > 3) ? u.ulevel/3 : 1;	/* just in case -- rph */

	for(i = rnd(tmp); i > 0; --i)
	    if((mtmp = mkmon_at(nasties[rn2(nastynum)], u.ux, u.uy)))  {

		mtmp->msleep = 0;
		mtmp->mpeaceful = 0;
	    }
#else
	(void) makemon((struct permonst *)0, u.ux, u.uy);
#endif
	return(0);
}

#ifdef HARD
/*	Here, we make trouble for the poor shmuck who actually	*/
/*	managed to do in the Wizard.				*/
intervene() {

	switch(rn2(6)) {

	    case 0:
	    case 1:	pline("You feel vaguely nervous.");
			break;
	    case 2:	pline("You notice a black glow surrounding you.");
			rndcurse();
			break;
	    case 3:	aggravate();
			break;
	    case 4:	nasty();
			break;
	    case 5:	resurrect();
			break;
	}
}

wizdead(mtmp)
register struct monst	*mtmp;
{
	flags.no_of_wizards--;
	if(! u.udemigod)  {

		u.udemigod = TRUE;
		u.udg_cnt = rn1(250, 50);

	/*  Make the wizard meaner the next time he appears  */
		mtmp->data->mlevel++;
		mtmp->data->ac--;
	} else  
		mtmp->data->mlevel++;
}


/*	Let's resurrect the wizard, for some unexpected fun.	*/
resurrect() {
register struct monst	*mtmp;

	    if(mtmp = makemon(PM_WIZARD, u.ux, u.uy)) {

		mtmp->msleep = mtmp->mtame = mtmp->mpeaceful = 0;
		flags.no_of_wizards++;
		pline("A voice booms out...");
		pline("\"So you thought you could kill me, fool.\"");
	    }

}
#endif /* HARD /**/
