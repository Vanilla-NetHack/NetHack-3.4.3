/*	SCCS Id: @(#)mthrowu.c	3.0	89/11/22
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

STATIC_DCL int FDECL(movedist,(int,int,int,int));
STATIC_DCL void FDECL(drop_throw,(struct obj *,BOOLEAN_P,int,int));
STATIC_DCL void FDECL(m_throw,(int,int,int,int,int,struct obj *));

#define URETREATING(x,y) (movedist(u.ux,u.uy,x,y) > movedist(u.ux0,u.uy0,x,y))

boolean FDECL(lined_up, (struct monst *));

#ifndef OVLB

STATIC_DCL const char *breathwep[];

#else /* OVLB */

schar NEARDATA tbx = 0, NEARDATA tby = 0;
	/* used for direction of throw, buzz, etc. */

STATIC_OVL const char NEARDATA *breathwep[] = {    "fragments",
				"fire",
				"sleep gas",
				"frost",
				"death",
				"lightning",
				"poison gas",
				"acid"
};

int
thitu(tlev, dam, obj, name)	/* u is hit by sth, but not a monster */
	register int tlev, dam;
	struct obj *obj;
	register const char *name;
{
	const char *onm = an(name);
	boolean is_acid = (obj && obj->otyp == ACID_VENOM);

	if(u.uac + tlev <= rnd(20)) {
		if(Blind || !flags.verbose) pline("It misses.");
		else You("are almost hit by %s!", onm);
		return(0);
	} else {
		if(Blind || !flags.verbose) You("are hit!");
		else You("are hit by %s!", onm);
#ifdef POLYSELF
		if (obj && obj->otyp == SILVER_ARROW && (u.ulycn != -1 ||
				is_demon(uasmon) || u.usym == S_VAMPIRE ||
				(u.usym == S_IMP && u.umonnum != PM_TENGU))) {
			dam += rnd(20);
			pline("The %sarrow sears your flesh!",
				Blind ? "" : "silver ");
		}
		if (is_acid && resists_acid(uasmon))
			pline("It doesn't seem to hurt you.");
		else {
#endif
			if (is_acid) pline("It burns!");
			losehp(dam, name, KILLED_BY_AN);
#ifdef POLYSELF
		}
#endif
		return(1);
	}
}

/* Be sure this corresponds with what happens to player-thrown objects in
 * dothrow.c (for consistency). --KAA
 */

STATIC_OVL void
drop_throw(obj, ohit, x, y)
register struct obj *obj;
boolean ohit;
int x,y;
{
	int create;

	if (obj->otyp == CREAM_PIE || obj->olet == VENOM_SYM)
		create = 0;
	else if (ohit &&
		 ((obj->otyp >= ARROW && obj->otyp <= SHURIKEN) ||
		  obj->otyp == ROCK))
		create = !rn2(3);
	else create = 1;
	if (create && !flooreffects(obj,x,y)) {
		place_object(obj, x, y);
		obj->nobj = fobj;
		fobj = obj;
		stackobj(fobj);
	} else free((genericptr_t)obj);
}

#endif /* OVLB */
#ifdef OVL1

STATIC_OVL void
m_throw(x, y, dx, dy, range, obj)
	register int x,y,dx,dy,range;		/* direction and range */
	register struct obj *obj;
{
	register struct monst *mtmp;
	struct obj *singleobj;
	char sym = obj->olet;
	int damage;
	int hitu, blindinc=0;

	bhitpos.x = x;
	bhitpos.y = y;

	singleobj = splitobj(obj, (int)obj->quan-1);
	/* splitobj leaves the new object in the chain (i.e. the monster's
	 * inventory).  Remove it.  We can do this in 1 line, but it's highly
	 * dependent on the fact that we know splitobj() places it immediately
	 * after obj.
	 */
	obj->nobj = singleobj->nobj;

	if(sym) {
		tmp_at(-1, sym);	/* open call */
#ifdef TEXTCOLOR
		tmp_at(-3, (int)objects[obj->otyp].oc_color);
#else
		tmp_at(-3, (int)AT_OBJ);
#endif
	}
	while(range-- > 0) { /* Actually the loop is always exited by break */
		boolean vis;

		bhitpos.x += dx;
		bhitpos.y += dy;
		vis = cansee(bhitpos.x, bhitpos.y);
		if(MON_AT(bhitpos.x, bhitpos.y)) {
		    mtmp = m_at(bhitpos.x,bhitpos.y);

		    if(mtmp->data->ac + 8 + obj->spe <= rnd(20)) {
			if (!vis) pline("It is missed.");
			else miss(distant_name(singleobj,xname), mtmp);
			if (!range) { /* Last position; object drops */
			    drop_throw(singleobj, 0, mtmp->mx, mtmp->my);
			    break;
			}
		    } else {
			damage = dmgval(obj, mtmp->data);
			if (damage < 1) damage = 1;
			if (obj->otyp==ACID_VENOM && resists_acid(mtmp->data))
			    damage = 0;
			if (!vis) pline("It is hit%s", exclam(damage));
			else hit(distant_name(singleobj,xname),
							mtmp,exclam(damage));
			if (obj->opoisoned) {
			    if (resists_poison(mtmp->data)) {
				if (vis)
				  pline("The poison doesn't seem to affect %s.",
								mon_nam(mtmp));
			    } else {
				if (rn2(30)) damage += rnd(6);
				else {
				    if (vis)
					pline("The poison was deadly...");
				    damage = mtmp->mhp;
				}
			    }
			}
			if (obj->otyp==SILVER_ARROW && (is_were(mtmp->data)
				|| is_demon(mtmp->data)
				|| mtmp->data->mlet == S_VAMPIRE
				|| (mtmp->data->mlet==S_IMP
					&& mtmp->data != &mons[PM_TENGU]))) {
			    if (vis) pline("The silver arrow sears %s's flesh!",
				mon_nam(mtmp));
			    else pline("Its flesh is seared!");
			    damage += rnd(20);
			}
			if (obj->otyp==ACID_VENOM && cansee(mtmp->mx,mtmp->my)){
			    if (resists_acid(mtmp->data)) {
				pline("%s is unaffected.", vis ? Monnam(mtmp)
					: "It");
				damage = 0;
			    } else if (vis)
				pline("The acid burns %s!", mon_nam(mtmp));
			    else pline("It is burned!");
			}
			mtmp->mhp -= damage;
			if(mtmp->mhp < 1) {
			    pline("%s is %s!", vis ? Monnam(mtmp) : "It",
			       (is_demon(mtmp->data) || 
					is_undead(mtmp->data) || !vis) ?
				 "destroyed" : "killed");
			    mondied(mtmp);
			}

			if((obj->otyp == CREAM_PIE) ||
			   (obj->otyp == BLINDING_VENOM)) {
			    if (vis)
				pline("%s is blinded by the %s.",
				      Monnam(mtmp), xname(singleobj));
			    if(mtmp->msleep) mtmp->msleep = 0;
			    mtmp->mcansee = 0;
			    {
				register unsigned rnd_tmp = rnd(25) + 20;
				if((mtmp->mblinded + rnd_tmp) > 127)
					mtmp->mblinded = 127;
				else mtmp->mblinded += rnd_tmp;
			    }
			}
			drop_throw(singleobj, 1, bhitpos.x, bhitpos.y);
			break;
		    }
		}
		if (bhitpos.x == u.ux && bhitpos.y == u.uy) {
			if (multi) nomul(0);

			switch(obj->otyp) {
			    int dam;
			    case CREAM_PIE:
			    case BLINDING_VENOM:
				hitu = thitu(8, 0, singleobj, xname(singleobj));
				break;
			    default:
				dam = dmgval(obj, uasmon);
				if (dam < 1) dam = 1;
				hitu = thitu(8+obj->spe, dam, singleobj,
					xname(singleobj));
			}
			if (hitu && obj->opoisoned)
			    /* it's safe to call xname twice because it's the
			       same object both times... */
			    poisoned(xname(singleobj), A_STR, xname(singleobj), 10);
			if(hitu && (obj->otyp == CREAM_PIE ||
				     obj->otyp == BLINDING_VENOM)) {
			    blindinc = rnd(25);
			    if(obj->otyp == CREAM_PIE) {
				if(!Blind) pline("Yecch!  You've been creamed.");
				else	pline("There's something sticky all over your %s.", body_part(FACE));
			    } else {	/* venom in the eyes */
				if(Blindfolded) /* nothing */ ;
				else if(!Blind) pline("The venom blinds you.");
				else	Your("%s sting.",
					makeplural(body_part(EYE)));
			    }
			}
			stop_occupation();
			if (hitu || !range) {
			    drop_throw(singleobj, hitu, u.ux, u.uy);
			    break;
			}
		} else if (!range	/* reached end of path */
			/* missile hits edge of screen */
			|| !isok(bhitpos.x+dx,bhitpos.y+dy)
			/* missile hits the wall */
			|| IS_WALL(levl[bhitpos.x+dx][bhitpos.y+dy].typ)
			|| levl[bhitpos.x+dx][bhitpos.y+dy].typ == SDOOR
			|| levl[bhitpos.x+dx][bhitpos.y+dy].typ == SCORR
#ifdef SINKS
			/* Thrown objects "sink" */
			|| IS_SINK(levl[bhitpos.x][bhitpos.y].typ)
#endif
								) {
		    drop_throw(singleobj, 0, bhitpos.x, bhitpos.y);
		    break;
		}
		tmp_at(bhitpos.x, bhitpos.y);
	}
	tmp_at(bhitpos.x, bhitpos.y);
	tmp_at(-1, -1);
	/* blindfold keeps substances out of your eyes */
	if (blindinc && !Blindfolded) {
		u.ucreamed += blindinc;
		make_blinded(Blinded + blindinc,FALSE);
	}
}

#endif /* OVL1 */
#ifdef OVLB

/* Remove an item from the monster's inventory.
 */
void
m_useup(mon, obj)
struct monst *mon;
struct obj *obj;
{
	struct obj *otmp, *prev;

	if (obj->quan > 1) {
		obj->quan--;
		return;
	}
	prev = ((struct obj *) 0);
	for (otmp = mon->minvent; otmp; otmp = otmp->nobj) {
		if (otmp == obj) {
			if (prev)
				prev->nobj = obj->nobj;
			else
				mon->minvent = obj->nobj;
			free((genericptr_t) obj);
			break;
		}
		prev = otmp;
	}
}

#endif /* OVLB */
#ifdef OVL1

/* Always returns 0??? -SAC */
int
thrwmu(mtmp)	/* monster throws item at you */
register struct monst *mtmp;
{
	struct obj *otmp, *select_rwep();
	register xchar x, y;

	if(lined_up(mtmp)) {

	    if((otmp = select_rwep(mtmp))) {

		/* If you are coming toward the monster, the monster
		 * should try to soften you up with missiles.  If you are
		 * going away, you are probably hurt or running.  Give
		 * chase, but if you are getting too far away, throw.
		 */
		x = mtmp->mx;
		y = mtmp->my;
		if(!URETREATING(x,y) ||
		   !rn2(BOLT_LIM-movedist(x,mtmp->mux,y,mtmp->muy)))
		{
		    unsigned savequan = otmp->quan;
		    const char *verb = "throws";

		    if (otmp->otyp == ARROW
#ifdef TOLKIEN
			|| otmp->otyp == ELVEN_ARROW
			|| otmp->otyp == ORCISH_ARROW
#endif
			|| otmp->otyp == CROSSBOW_BOLT) verb = "shoots";
		    otmp->quan = 1;
		    if (canseemon(mtmp))
			pline("%s %s %s!", Monnam(mtmp), verb, an(xname(otmp)));
		    otmp->quan = savequan;
		    m_throw(mtmp->mx, mtmp->my, sgn(tbx), sgn(tby), 
			movedist(mtmp->mx,mtmp->mux,mtmp->my,mtmp->muy), otmp);
		    if (!otmp->quan) m_useup(mtmp, otmp);
		    nomul(0);
		    return 0;
		}
	    }
	}
	return 0;
}

#endif /* OVL1 */
#ifdef OVLB

int
spitmu(mtmp, mattk)		/* monster spits substance at you */
register struct monst *mtmp;
register struct attack *mattk;
{
	register struct obj *otmp;

	if(mtmp->mcan) {

	    if(flags.soundok)
		pline("A dry rattle comes from %s's throat", mon_nam(mtmp));
	    return 0;
	}
	if(lined_up(mtmp)) {
		switch (mattk->adtyp) {
		    case AD_BLND:
		    case AD_DRST:
			otmp = mksobj(BLINDING_VENOM, FALSE);
			break;
		    default:
			impossible("bad attack type in spitmu");
				/* fall through */
		    case AD_ACID:
			otmp = mksobj(ACID_VENOM, FALSE);
			break;
		}
		if(!rn2(BOLT_LIM-movedist(mtmp->mx,mtmp->mux,mtmp->my,mtmp->muy))) {
		    if (canseemon(mtmp))
			pline("%s spits venom!", Monnam(mtmp));
		    m_throw(mtmp->mx, mtmp->my, sgn(tbx), sgn(tby), 
			movedist(mtmp->mx,mtmp->mux,mtmp->my,mtmp->muy), otmp);
		    nomul(0);
		    return 0;
		}
	}
	return 0;
}

#endif /* OVLB */
#ifdef OVL1

int
breamu(mtmp, mattk)			/* monster breathes at you (ranged) */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	if(lined_up(mtmp)) {

	    if(mtmp->mcan) {
		if(flags.soundok) {
		    if(canseemon(mtmp))
			pline("%s coughs.", Monnam(mtmp));
		    else
			You("hear a cough.");
		}
		return(0);
	    }
	    if(rn2(3)) {

		if((mattk->adtyp >= 1) && (mattk->adtyp < 11)) {

		    if(canseemon(mtmp))
			pline("%s breathes %s!", Monnam(mtmp),
			      breathwep[mattk->adtyp-1]);
		    buzz((int) (-20 - (mattk->adtyp-1)), (int)mattk->damn,
			 mtmp->mx, mtmp->my, sgn(tbx), sgn(tby));
		    nomul(0);
		} else impossible("Breath weapon %d used", mattk->adtyp-1);
	    }
	}
	return(1);
}

boolean
linedup(ax, ay, bx, by)
register xchar ax, ay, bx, by;
{
	register xchar x, y;

	tbx = ax - bx;	/* These two values are set for use */
	tby = ay - by;	/* after successful return.	    */

	if((!tbx || !tby || abs(tbx) == abs(tby)) /* straight line or diagonal */
	   && movedist(tbx, 0,  tby, 0) < BOLT_LIM) {

		/* Check if there are any dead squares between.  If so,
		 * it will not be possible to shoot.
		 */
		x = bx; y = by;
		while(x != ax || y != ay) {

		    if(!accessible(x, y)) return FALSE;
		    x += sgn(tbx), y += sgn(tby);
		}
		return TRUE;
	}
	return FALSE;
}

boolean
lined_up(mtmp)		/* is mtmp in position to use ranged attack? */
	register struct monst *mtmp;
{
	return(linedup(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my));
}

#endif /* OVL1 */
#ifdef OVL0

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

#endif /* OVL0 */
#ifdef OVL1

STATIC_OVL int
movedist(x0, x1, y0, y1)
int x0, x1, y0, y1;
{
	register int absdx, absdy;

	absdx = abs(x1 - x0);
	absdy = abs(y1 - y0);

	return (max(absdx,absdy));
}

#endif /* OVL1 */
