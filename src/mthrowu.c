/*	SCCS Id: @(#)mthrowu.c	3.1	92/11/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

STATIC_DCL void FDECL(drop_throw,(struct obj *,BOOLEAN_P,int,int));
#ifndef MUSE
STATIC_DCL void FDECL(m_throw,(struct monst *,int,int,int,int,int,struct obj *));
#endif

#define URETREATING(x,y) (distmin(u.ux,u.uy,x,y) > distmin(u.ux0,u.uy0,x,y))

boolean FDECL(lined_up, (struct monst *));

#ifndef OVLB

STATIC_DCL const char *breathwep[];

#else /* OVLB */

/*
 * Keep consistent with breath weapons in zap.c, and AD_* in monattk.h.
 */
STATIC_OVL const char NEARDATA *breathwep[] = {
				"fragments",
				"fire",
				"frost",
				"sleep gas",
				"death",
				"lightning",
				"poison gas",
				"acid",
				"strange breath #8",
				"strange breath #9"
};

int
thitu(tlev, dam, obj, name)	/* u is hit by sth, but not a monster */
	register int tlev, dam;
	struct obj *obj;
	register const char *name;
{
	const char *onm = (obj && obj_is_pname(obj)) ? the(name) : an(name);
	boolean is_acid = (obj && obj->otyp == ACID_VENOM);

	if(u.uac + tlev <= rnd(20)) {
		if(Blind || !flags.verbose) pline("It misses.");
		else You("are almost hit by %s!", onm);
		return(0);
	} else {
		if(Blind || !flags.verbose) You("are hit!");
		else You("are hit by %s!", onm);
#ifdef POLYSELF
		if (obj && objects[obj->otyp].oc_material == SILVER
				&& hates_silver(uasmon)) {
			dam += rnd(20);
			pline("The silver sears your flesh!");
			exercise(A_CON, FALSE);
		}
		if (is_acid && resists_acid(uasmon))
			pline("It doesn't seem to hurt you.");
		else {
#endif
			if (is_acid) pline("It burns!");
			if (Half_physical_damage) dam = (dam+1) / 2;
			losehp(dam, name, (obj && obj_is_pname(obj)) ?
			       KILLED_BY : KILLED_BY_AN);
			exercise(A_STR, FALSE);
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
	struct monst *mtmp;
	struct trap *t;

	if (obj->otyp == CREAM_PIE || obj->oclass == VENOM_CLASS)
		create = 0;
	else if (ohit &&
		 ((obj->otyp >= ARROW && obj->otyp <= SHURIKEN) ||
		  obj->otyp == ROCK))
		create = !rn2(3);
	else create = 1;
	if (create && !((mtmp = m_at(x, y)) && (mtmp->mtrapped) && 
			(t = t_at(x, y)) && ((t->ttyp == PIT) || 
			(t->ttyp == SPIKED_PIT))) && 
	    !flooreffects(obj,x,y,"fall")) { /* don't double-dip on damage */
		place_object(obj, x, y);
		obj->nobj = fobj;
		fobj = obj;
		stackobj(fobj);
	} else obfree(obj, (struct obj*) 0);
}

#endif /* OVLB */
#ifdef OVL1

#ifndef MUSE
STATIC_OVL
#endif
void
m_throw(mon, x, y, dx, dy, range, obj)
	register struct monst *mon;
	register int x,y,dx,dy,range;		/* direction and range */
	register struct obj *obj;
{
	register struct monst *mtmp;
	struct obj *singleobj;
	char sym = obj->oclass;
	int damage;
	int hitu, blindinc=0;

	bhitpos.x = x;
	bhitpos.y = y;

	singleobj = splitobj(obj, obj->quan - 1L);
	/* splitobj leaves the new object in the chain (i.e. the monster's
	 * inventory).  Remove it.  We can do this in 1 line, but it's highly
	 * dependent on the fact that we know splitobj() places it immediately
	 * after obj.
	 */
	obj->nobj = singleobj->nobj;
	/* Get rid of object.  This cannot be done later on; what if the
	 * player dies before then, leaving the monster with 0 daggers?
	 * (This caused the infamous 2^32-1 orcish dagger bug).
	 */
	if (!obj->quan) {
	    if(obj->oclass == VENOM_CLASS) {
		/* venom is not in the monster's inventory chain */
		dealloc_obj(obj);
	    } else {
#ifdef MUSE
		/* not possibly_unwield, which checks the object's */
		/* location, not its existence */
		if (MON_WEP(mon) == obj)
			MON_NOWEP(mon);
#endif
		m_useup(mon, obj);
	    }
	}

	if (singleobj->cursed && (dx || dy) && !rn2(7)) {
	    if(canseemon(mon) && flags.verbose) {
		if((singleobj->oclass == WEAPON_CLASS ||
						singleobj->oclass == GEM_CLASS)
		   && objects[singleobj->otyp].w_propellor)
		    pline("%s misfires!", Monnam(mon));
		else
		    pline("The %s slips as %s throws it!",
			  xname(singleobj), mon_nam(mon));
	    }
	    dx = rn2(3)-1;
	    dy = rn2(3)-1;
	    /* pre-check validity of new direction */
	    if((!dx && !dy)
	       || !isok(bhitpos.x+dx,bhitpos.y+dy)
	       /* missile hits the wall */
	       || IS_WALL(levl[bhitpos.x+dx][bhitpos.y+dy].typ)
	       || levl[bhitpos.x+dx][bhitpos.y+dy].typ == SDOOR
	       || levl[bhitpos.x+dx][bhitpos.y+dy].typ == SCORR) {
		drop_throw(singleobj, 0, bhitpos.x, bhitpos.y);
		return;
	    }
	}

	/* Note: drop_throw may destroy singleobj.  Since obj must be destroyed
	 * early to avoid the dagger bug, anyone who modifies this code should
	 * be careful not to use either one after it's been freed.
	 */
	if (sym) tmp_at(DISP_FLASH, obj_to_glyph(singleobj));
	while(range-- > 0) { /* Actually the loop is always exited by break */
		boolean vis;

		bhitpos.x += dx;
		bhitpos.y += dy;
		vis = cansee(bhitpos.x, bhitpos.y);
		if(MON_AT(bhitpos.x, bhitpos.y)) {
		    boolean ismimic;

		    mtmp = m_at(bhitpos.x,bhitpos.y);
		    ismimic = mtmp->m_ap_type &&
			mtmp->m_ap_type != M_AP_MONSTER;

		    /* don't use distance/size modifiers since target was u */
		    if(find_mac(mtmp) + 8 + singleobj->spe <= rnd(20)) {
			if (!ismimic) {
			    if (!vis) pline("It is missed.");
			    else miss(distant_name(singleobj,xname), mtmp);
			}
			if (!range) { /* Last position; object drops */
			    drop_throw(singleobj, 0, mtmp->mx, mtmp->my);
			    break;
			}
#ifdef MUSE
		    } else if (singleobj->oclass == POTION_CLASS) {
			if (ismimic) seemimic(mtmp);
			if (vis) singleobj->dknown = 1;
			potionhit(mtmp, singleobj);
			break;
#endif
		    } else {
			damage = dmgval(singleobj, mtmp->data);
			if (damage < 1) damage = 1;
			if (singleobj->otyp==ACID_VENOM && resists_acid(mtmp->data))
			    damage = 0;
			if (ismimic) seemimic(mtmp);
			if (!vis) pline("It is hit%s", exclam(damage));
			else hit(distant_name(singleobj,xname),
							mtmp,exclam(damage));
			if (singleobj->opoisoned) {
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
			if (objects[singleobj->otyp].oc_material == SILVER
				&& hates_silver(mtmp->data)) {
			    if (vis) pline("The silver sears %s's flesh!",
				mon_nam(mtmp));
			    else pline("Its flesh is seared!");
			}
			if (singleobj->otyp==ACID_VENOM && cansee(mtmp->mx,mtmp->my)){
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

			if(((singleobj->otyp == CREAM_PIE) ||
			    (singleobj->otyp == BLINDING_VENOM))
			   && haseyes(mtmp->data)) {
			    if (vis)
				pline("%s is blinded by %s.",
				      Monnam(mtmp), the(xname(singleobj)));
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

#ifdef MUSE
			if (singleobj->oclass == POTION_CLASS) {
			    if (!Blind) singleobj->dknown = 1;
			    potionhit(&youmonst, singleobj);
			    break;
			}
#endif
			switch(singleobj->otyp) {
			    int dam, hitv;
			    case CREAM_PIE:
			    case BLINDING_VENOM:
				hitu = thitu(8, 0, singleobj, xname(singleobj));
				break;
			    default:
				dam = dmgval(singleobj, uasmon);
				hitv = 3 - distmin(u.ux,u.uy, mon->mx,mon->my);
				if (hitv < -4) hitv = -4;
				if (is_elf(mon->data) &&
				    objects[singleobj->otyp].w_propellor
								== WP_BOW) {
				    hitv++;
#ifdef MUSE
				    if (MON_WEP(mon) &&
					MON_WEP(mon)->otyp == ELVEN_BOW)
					hitv++;
#endif
				    if(singleobj->otyp == ELVEN_ARROW) dam++;
				}
#ifdef POLYSELF
				if (bigmonst(uasmon)) hitv++;
#endif
				hitv += 8+singleobj->spe;

				if (dam < 1) dam = 1;
				hitu = thitu(hitv, dam,
					singleobj, xname(singleobj));
			}
			if (hitu && singleobj->opoisoned)
			    /* it's safe to call xname twice because it's the
			       same object both times... */
			    poisoned(xname(singleobj), A_STR, xname(singleobj), 10);
			if(hitu && (singleobj->otyp == CREAM_PIE ||
				     singleobj->otyp == BLINDING_VENOM)) {
			    blindinc = rnd(25);
			    if(singleobj->otyp == CREAM_PIE) {
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
		delay_output();
	}
	tmp_at(bhitpos.x, bhitpos.y);
	delay_output();
	tmp_at(DISP_END, 0);
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

	if (obj->quan > 1L) {
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
			dealloc_obj(obj);
			break;
		}
		prev = otmp;
	}
}

#endif /* OVLB */
#ifdef OVL1

void
thrwmu(mtmp)	/* monster throws item at you */
register struct monst *mtmp;
{
	struct obj *otmp;
	register xchar x, y;

	if(lined_up(mtmp)) {
#ifdef MUSE
	    if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)) {
		mtmp->weapon_check = NEED_RANGED_WEAPON;
		/* mon_wield_item resets weapon_check as appropriate */
		if(mon_wield_item(mtmp) != 0) return;
	    }
#endif
	    if((otmp = select_rwep(mtmp))) {
		/* If you are coming toward the monster, the monster
		 * should try to soften you up with missiles.  If you are
		 * going away, you are probably hurt or running.  Give
		 * chase, but if you are getting too far away, throw.
		 */
		x = mtmp->mx;
		y = mtmp->my;
		if(!URETREATING(x,y) ||
		   !rn2(BOLT_LIM-distmin(x,y,mtmp->mux,mtmp->muy)))
		{
		    const char *verb = "throws";

		    if (otmp->otyp == ARROW
			|| otmp->otyp == ELVEN_ARROW
			|| otmp->otyp == ORCISH_ARROW
			|| otmp->otyp == CROSSBOW_BOLT) verb = "shoots";
		    if (canseemon(mtmp)) {
			pline("%s %s %s!", Monnam(mtmp), verb,
			      obj_is_pname(otmp) ?
			      the(singular(otmp, xname)) :
			      an(singular(otmp, xname)));
		    }
		    m_throw(mtmp, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby), 
			distmin(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy), otmp);
		    nomul(0);
		    return;
		}
	    }
	}
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
		pline("A dry rattle comes from %s throat", 
		                      s_suffix(mon_nam(mtmp)));
	    return 0;
	}
	if(lined_up(mtmp)) {
		switch (mattk->adtyp) {
		    case AD_BLND:
		    case AD_DRST:
			otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
			break;
		    default:
			impossible("bad attack type in spitmu");
				/* fall through */
		    case AD_ACID:
			otmp = mksobj(ACID_VENOM, TRUE, FALSE);
			break;
		}
		if(!rn2(BOLT_LIM-distmin(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy))) {
		    if (canseemon(mtmp))
			pline("%s spits venom!", Monnam(mtmp));
		    m_throw(mtmp, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby), 
			distmin(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy), otmp);
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
	/* if new breath types are added, change AD_ACID to max type */
	int typ = (mattk->adtyp == AD_RBRE) ? rnd(AD_ACID) : mattk->adtyp ;

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
	    if(!mtmp->mspec_used && rn2(3)) {

		if((typ >= AD_MAGM) && (typ <= AD_ACID)) {

		    if(canseemon(mtmp))
			pline("%s breathes %s!", Monnam(mtmp),
			      breathwep[typ-1]);
		    buzz((int) (-20 - (typ-1)), (int)mattk->damn,
			 mtmp->mx, mtmp->my, sgn(tbx), sgn(tby));
		    nomul(0);
		    /* breath runs out sometimes. Also, give monster some
		     * cunning; don't breath if the player fell asleep.
		     */
		    if(!rn2(3))
			mtmp->mspec_used = 10+rn2(20);
		    if(typ == AD_SLEE && !Sleep_resistance)
			mtmp->mspec_used += rnd(20);
		} else impossible("Breath weapon %d used", typ-1);
	    }
	}
	return(1);
}

boolean
linedup(ax, ay, bx, by)
register xchar ax, ay, bx, by;
{
	tbx = ax - bx;	/* These two values are set for use */
	tby = ay - by;	/* after successful return.	    */

	if((!tbx || !tby || abs(tbx) == abs(tby)) /* straight line or diagonal */
	   && distmin(tbx, tby, 0, 0) < BOLT_LIM) {

	    if(ax == u.ux && ay == u.uy) return couldsee(bx,by);
	    else if(clear_path(ax,ay,bx,by)) return TRUE;
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

/*mthrowu.c*/
