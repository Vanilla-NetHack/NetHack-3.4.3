/*	SCCS Id: @(#)mhitm.c	3.0	88/11/10
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#ifdef NAMED_ITEMS
#  include "artifact.h"
#endif

static boolean vis, far_noise;
static long noisetime;
static struct obj *otmp;

static void mrustm P((struct monst *, struct monst *, struct obj *));
static int hitmm P((struct monst *,struct monst *,struct attack *));
static int gazemm P((struct monst *,struct monst *,struct attack *));
static int gulpmm P((struct monst *,struct monst *,struct attack *));
static int explmm P((struct monst *,struct monst *,struct attack *));
static int mdamagem P((struct monst *,struct monst *,struct attack *));
static void mswingsm P((struct monst *, struct monst *, struct obj *));

static void
noises(magr, mattk)
	register struct monst *magr;
	register struct	attack *mattk;
{
	boolean farq = (dist(magr->mx, magr->my) > 15);

	if(flags.soundok && (farq != far_noise || moves-noisetime > 10)) {
		far_noise = farq;
		noisetime = moves;
		You("hear %s%s.",
			(mattk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
			farq ? " in the distance" : "");
	}
}

static
void
missmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	struct attack *mattk;
{
	char buf[BUFSZ];

	if(vis) {
		if(mdef->mimic) seemimic(mdef);
		if(magr->mimic) seemimic(magr);
		if (could_seduce(magr,mdef,mattk) && !magr->mcan)
			Sprintf(buf, "%s pretends to be friendly to",
								Monnam(magr));
		else
			Sprintf(buf,"%s misses", Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	} else  noises(magr, mattk);
}

int
fightm(mtmp)		/* have monsters fight each other */
	register struct monst *mtmp;
{
register struct monst *mon;
/*	TODO:	this loop needs to be restructured, as we don't know if
 *		either "mon" or "mon->nmon" will exist after the attack.
 */
	for(mon = fmon; mon; mon = mon->nmon)
	    if(mon != mtmp) {
		if(dist2(mon->mx,mon->my,mtmp->mx,mtmp->my) < 3)
		    return(mattackm(mtmp,mon));
	    }
	return(-1);
}

/*
 * mattackm returns -1 (magr died), 0 (miss), 1 (mdef hit), or 2 (mdef killed)
 *
 * Each successive attack has a lower probability of hitting.  Some
 * rely on the success of previous attacks.
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(magr, mdef)
	register struct monst *magr,*mdef;
{
	int	i, tmp, nsum, sum[NATTK];
	struct	attack	*mattk;
	struct	permonst *pa, *pd;
	schar	strike;

	if(!magr || !mdef) return(0);		/* mike@genat */
	pa = magr->data; pd = mdef->data;
	if(magr->mfroz) return(0);		/* riv05!a3 */

/*	Calculate the armour class differential.	*/

	tmp = pd->ac + magr->m_lev;
	if(mdef->mconf || mdef->mfroz || mdef->msleep){
		tmp += 4;
		if(mdef->msleep) mdef->msleep = 0;
	}

	if (is_elf(magr->data) && is_orc(mdef->data)) tmp++;

/*	Set up visibility of action			*/
	vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my));

/*	Set flag indicating monster has moved this turn.  Necessary since a
 *	monster might get an attack out of sequence (i.e. before its move) in
 *	some cases, in which case this still counts as its move for the round
 *	and it shouldn't move again.
 */
	magr->mlstmv = moves;

/*	Now perform all attacks for the monster.	*/

	for(i=0; i<NATTK; i++) sum[i] = 0;
	for(i = nsum = 0; i < NATTK; nsum |= sum[i++]) {
	    mattk = &(pa->mattk[i]);
	    otmp = (struct obj *)0;
	    switch(mattk->aatyp) {

		case AT_WEAP:		/* "hand to hand" attacks */
			otmp = select_hwep(magr);
			if(otmp) {
				if (vis) mswingsm(magr, mdef, otmp);
				tmp += hitval(otmp, pd);
			}
		case AT_CLAW:
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
			if((strike = (tmp > rnd(20+i)))) {
				sum[i] = hitmm(magr, mdef, mattk);
				if(sum[i] == -1) return(-1);
			} else	missmm(magr, mdef, mattk);
			break;

		case AT_HUGS:	/* automatic if prev two attacks succeed */
			strike = 1;
			if(sum[i-1] && sum[i-2]) {
			    sum[i] = hitmm(magr, mdef, mattk);
			    if(sum[i] == -1) return(-1);
			}
			break;

		case AT_GAZE:	/* will not wake up a sleeper */
			strike = 0;
			sum[i] = gazemm(magr, mdef, mattk);
			break;

		case AT_EXPL:	/* automatic hit if next to */
			strike = -1;
			sum[i] = explmm(magr, mdef, mattk);
			break;

		case AT_ENGL:
			if((strike = (tmp > rnd(20+i))))
				sum[i]= gulpmm(magr, mdef, mattk);
			else	missmm(magr, mdef, mattk);
			break;

		default:		/* no attack */
			strike = 0;
			break;
	    }
	    if(sum[i] == 2) return(2);  	/* defender dead */
	    if(strike)	    mdef->msleep = 0;
	    if(strike == -1)   return(-1);		/* attacker dead */
	    nsum |= sum[i];
	}
	return(nsum);
}

/* hitmm returns 0 (miss), 1 (hit), 2 (kill), or -1 (magr died) */
static int
hitmm(magr, mdef, mattk)
	register struct monst *magr,*mdef;
	struct	attack *mattk;
{
	if(vis){
		int compat;
		char buf[BUFSZ];

		if(mdef->mimic) seemimic(mdef);
		if(magr->mimic) seemimic(magr);
		if((compat = could_seduce(magr,mdef,mattk)) && !magr->mcan) {
			Sprintf(buf, "%s %s", Monnam(magr),
				mdef->mblinded ? "talks to" : "smiles at");
			pline("%s %s %s.", buf, mon_nam(mdef),
				compat == 2 ?
					"engagingly" : "seductively");
		} else {
		    switch (mattk->aatyp) {
			case AT_BITE:
				Sprintf(buf,"%s bites", Monnam(magr));
				break;
			case AT_STNG:
				Sprintf(buf,"%s stings", Monnam(magr));
				break;
			case AT_BUTT:
				Sprintf(buf,"%s butts", Monnam(magr));
				break;
			case AT_TUCH:
				Sprintf(buf,"%s touches", Monnam(magr));
				break;
			case AT_HUGS:
				if (magr != u.ustuck) {
				    Sprintf(buf,"%s squeezes", Monnam(magr));
				    break;
				}
			default:
				Sprintf(buf,"%s hits", Monnam(magr));
		    }
		}
		pline("%s %s.", buf, mon_nam(mdef));
	} else  noises(magr, mattk);
	return(mdamagem(magr, mdef, mattk));
}

static int
gazemm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	struct attack *mattk;
{
	char buf[BUFSZ];

	if(vis) {
		Sprintf(buf,"%s gazes at", Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	}

	if (mdef->mblinded || mdef->msleep) {

	    if(vis) pline("but nothing happens.");
	    return(0);
	}

	return(mdamagem(magr, mdef, mattk));
}

static int
gulpmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	int	mx, my, tmp;
	char buf[BUFSZ];

	if(vis) {
		Sprintf(buf,"%s swallows", Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	}

	mx = magr->mx;
	my = magr->my;
	 /* move over top of the defender */
	if(cansee(mdef->mx, mdef->my))	unpmon(mdef);
	if(cansee(magr->mx, magr->my))	unpmon(magr);
	magr->mx = mdef->mx;
	magr->my = mdef->my;
	if(cansee(magr->mx, magr->my))	pmon(magr);
	if((tmp = mdamagem(magr, mdef, mattk)) == 2) {
		levl[mx][my].mmask = 0;
		levl[magr->mx][magr->my].mmask = 1;
		/* if mdamagem left a corpse it erased magr's symbol */
		unpmon(magr);
		pmon(magr);
		return(2);	/* defender died */
	} else {		/* defender survived */
		if(cansee(mdef->mx, mdef->my))
			pline("%s is regurgitated!", Monnam(mdef));
		if(cansee(magr->mx, magr->my))	unpmon(magr);
		magr->mx = mx;
		magr->my = my;
		/* move off of defender */
		if(cansee(magr->mx, magr->my))	pmon(magr);
		if(cansee(mdef->mx, mdef->my))	pmon(mdef);
		nscr();
		return(tmp);
	}
}

static int
explmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{

	if(cansee(magr->mx, magr->my))
		pline("%s explodes!", Monnam(magr));
	else	noises(magr, mattk);

	(void) mdamagem(magr, mdef, mattk);

	if(magr->mtame)
		You("have a sad feeling for a moment, then it passes.");
	mondead(magr);
	return(2);
}

static int
mdamagem(magr, mdef, mattk)
	register struct monst	*magr, *mdef;
	register struct attack	*mattk;
{
	struct	permonst *ptr, *pd = mdef->data;
	int	tmp = d((int)mattk->damn,(int)mattk->damd);
	char buf[BUFSZ];

	switch(mattk->adtyp) {
	    case AD_DGST:
		if(flags.verbose && flags.soundok) pline("\"Burrrrp!\"");
		tmp = mdef->mhp;
		break;
	    case AD_STUN:
		if (magr->mcan) break;
		if(vis) pline("%s staggers for a moment.", Monnam(mdef));
		mdef->mstun = 1;
		/* fall through */
	    case AD_WERE:
	    case AD_HEAL:
	    case AD_LEGS:
	    case AD_PHYS:
		if (mattk->aatyp == AT_KICK && thick_skinned(mdef->data))
			tmp = 0;
		else if(mattk->aatyp == AT_WEAP) {
		    if(otmp) {
			tmp += dmgval(otmp, pd);
#ifdef NAMED_ITEMS
			if(spec_ability(otmp, SPFX_DRLI) &&
			    !resists_drli(mdef->data)) {
			    int dam = rnd(8);

			    tmp += dam;
			    if(vis)
				pline("The %s blade drains the life from %s!",
					Hallucination ? hcolor() : black,
					mon_nam(mdef));
			    mdef->mhpmax -= dam;
			    if (mdef->m_lev == 0)
				tmp = mdef->mhp;
			    else mdef->m_lev--;
			}
#endif
			mrustm(magr, mdef, otmp);
		    }
		}
		break;
	    case AD_FIRE:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
#ifdef GOLEMS
		golemeffects(mdef, AD_FIRE, tmp);
#endif /* GOLEMS */
		if(resists_fire(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		} else {
		    tmp += destroy_mitem(mdef, SCROLL_SYM, AD_FIRE);
		    tmp += destroy_mitem(mdef, POTION_SYM, AD_FIRE);
#ifdef SPELLS
		    tmp += destroy_mitem(mdef, SPBOOK_SYM, AD_FIRE);
#endif
		}
		break;
	    case AD_COLD:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
#ifdef GOLEMS
		golemeffects(mdef, AD_COLD, tmp);
#endif /* GOLEMS */
		if(resists_cold(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		} else tmp += destroy_mitem(mdef, POTION_SYM, AD_COLD);
		break;
	    case AD_ELEC:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
#ifdef GOLEMS
		golemeffects(mdef, AD_ELEC, tmp);
#endif /* GOLEMS */
		if(resists_elec(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		break;
	    case AD_ACID:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if(resists_acid(pd)) tmp = 0;
		break;
	    case AD_RUST:
#ifdef GOLEMS
		if (!magr->mcan && pd == &mons[PM_IRON_GOLEM]) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			else if(mdef->mtame)
			     pline("May %s rust in peace.", mon_nam(mdef));
			mondied(mdef);
			magr->mhpmax += 1 + rn2((int)mdef->m_lev+1);
			ptr = grow_up(magr);
			if(!ptr) return(-1);
			return(2);
		}
#endif /* GOLEMS */
		tmp = 0;
		break;
	    case AD_DCAY:
#ifdef GOLEMS
		if (!magr->mcan && (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM])) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			else if(mdef->mtame)
			     pline("May %s rot in peace.", mon_nam(mdef));
			mondied(mdef);
			magr->mhpmax += 1 + rn2((int)mdef->m_lev+1);
			ptr = grow_up(magr);
			if(!ptr) return(-1);
			return(2);
		}
#endif /* GOLEMS */
		tmp = 0;
		break;
	    case AD_STON:
		if(!resists_ston(pd)) {
			magr->mhpmax += 1 + rn2((int)mdef->m_lev+1);
			if(vis) pline("%s turns to stone!", Monnam(mdef));
			else if(mdef->mtame)
     You("have a peculiarly sad feeling for a moment, then it passes.");
			monstone(mdef);
			ptr = grow_up(magr);
			if(!ptr) return(-1);
			return(2);
		}
		tmp = 0;	/* no damage if this fails */
		break;
	    case AD_TLPT:
		if(!magr->mcan && tmp < mdef->mhp) {
		    rloc(mdef);
		    if(vis && !cansee(mdef->mx, mdef->my))
			pline("%s suddenly disappears!", Monnam(mdef));
		}
		break;
	    case AD_SLEE:
		if(!resists_sleep(pd) && !magr->mcan && vis && !mdef->msleep) {
		    pline("%s falls asleep.", Monnam(mdef));
		    mdef->msleep = 1;
		}
		break;
	    case AD_PLYS:
		if(!magr->mcan && vis && !mdef->mfroz) {
		    pline("%s stops moving.", Monnam(mdef));
		    mdef->mfroz = 1;
		}
		break;
	    case AD_SLOW:
		if(!magr->mcan && vis && mdef->mspeed != MSLOW) {
		    pline("%s slows down.", Monnam(mdef));
		    if (mdef->mspeed == MFAST) mdef->mspeed = 0;
		    else mdef->mspeed = MSLOW;
		}
		break;
	    case AD_CONF:
		/* Since confusing another monster doesn't have a real time
		 * limit, setting spec_used would not really be right (though
		 * we still should check for it).
		 */
		if(!magr->mcan && vis && !mdef->mconf && !magr->mspec_used) {
		    pline("%s looks confused.", Monnam(mdef));
		    mdef->mconf = 1;
		}
		break;
	    case AD_BLND:
		if(!magr->mcan && haseyes(pd)) {

		    if(vis && !mdef->mblinded)
			pline("%s is blinded.", Monnam(mdef));
		    {
			register unsigned rnd_tmp;
			rnd_tmp = d((int)mattk->damn, (int)mattk->damd);
			mdef->mcansee = 0;
			if((mdef->mblinded + rnd_tmp) > 127)
				mdef->mblinded = 127;
			else mdef->mblinded += rnd_tmp;
		    }
		}
		tmp = 0;
		break;
	    case AD_CURS:
		if(!night() && (magr->data == &mons[PM_GREMLIN])) break;
		if(!magr->mcan && !rn2(10)) {
		    if (is_were(mdef->data) && mdef->data->mlet != S_HUMAN)
			were_change(mdef);
#ifdef GOLEMS
		    if (mdef->data == &mons[PM_CLAY_GOLEM]) {
			    if (vis) {
				pline("Some writing vanishes from %s's head!",
				    mon_nam(mdef));
				pline("%s dies!", Monnam(mdef));
			    }
			    else if (mdef->mtame)
	You("have a strangely sad feeling for a moment, then it passes.");
			    mondied(mdef);
			    magr->mhpmax += 1 + rn2((int)mdef->m_lev+1);
			    ptr = grow_up(magr);
			    if(!ptr) return(-1);
			    return(2);
		      }
#endif /* GOLEMS */
		    mdef->mcan = 1;
		    if (flags.soundok) {
			    if (!vis) You("hear laughter.");
			    else pline("%s chuckles.", Monnam(magr));
		    }
		}
		break;
	    case AD_SGLD:
		tmp = 0;
		if (magr->mcan || !mdef->mgold) break;
		/* technically incorrect; no check for stealing gold from
		 * between mdef's feet...
		 */
		magr->mgold += mdef->mgold;
		mdef->mgold = 0;
		if (vis) {
			Strcpy(buf, Monnam(magr));
			pline("%s steals some gold from %s.", buf,
								mon_nam(mdef));
		}
		break;
	    case AD_DRLI:
		if(rn2(2) && !resists_drli(mdef->data)) {
			tmp = d(2,6);
			if (vis)
			    kludge("%s suddenly seems weaker!", Monnam(mdef));
			mdef->mhpmax -= tmp;
			if (mdef->m_lev == 0)
				tmp = mdef->mhp;
			else mdef->m_lev--;
			/* Automatic kill if drained past level 0 */
		}
		break;
#ifdef SEDUCE
	    case AD_SSEX:
#endif
	    case AD_SITM:	/* for now these are the same */
	    case AD_SEDU:
		if (!magr->mcan && mdef->minvent) {
		   	otmp = mdef->minvent;
			mdef->minvent = otmp->nobj;
			otmp->nobj = magr->minvent;
			magr->minvent = otmp;
			if (vis) {
				Strcpy(buf, Monnam(magr));
				pline("%s steals %s from %s!", buf,
						doname(otmp), mon_nam(mdef));
			}
		}
		tmp = 0;
		break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!magr->mcan && !rn2(8)) {
		    if (vis)
			pline("%s's %s was poisoned!", Monnam(magr),
				mattk->aatyp==AT_BITE ? "bite" : "sting");
		    if (resists_poison(mdef->data)) {
			if (vis)
			    pline("The poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    } else {
			if (rn2(10)) tmp += rn1(10,6);
			else {
			    if (vis) pline("The poison was deadly...");
			    tmp = mdef->mhp;
			}
		    }
		}
		break;
	    case AD_STCK:
	    case AD_WRAP: /* monsters cannot grab one another, it's too hard */
		break;
	    default:	tmp = 0;
			break;
	}
	if(!tmp) return(1);

	if((mdef->mhp -= tmp) < 1) {
	    magr->mhpmax += 1 + rn2((int)mdef->m_lev+1);
	    if(vis) pline("%s is killed!", Monnam(mdef));
	    else if(mdef->mtame)
		You("have a sad feeling for a moment, then it passes.");
	    mondied(mdef);
	    ptr = grow_up(magr);
	    if(!ptr) return(-1);
	    return(2);
	}
	/* fixes a bug where max monster hp could overflow. */
	if(magr->mhpmax <= 0 || magr->mhpmax > MHPMAX) magr->mhpmax = MHPMAX;

	return(1);
}

int
noattacks(ptr)			/* returns 1 if monster doesn't attack */
	struct	permonst *ptr;
{
	int i;

	for(i = 0; i < NATTK; i++)
		if(ptr->mattk[i].aatyp) return(0);

	return(1);
}

static void
mrustm(magr, mdef, obj)
register struct monst *magr, *mdef;
register struct obj *obj;
{
	if (!magr || !mdef || !obj) return; /* just in case */
	if (mdef->data == &mons[PM_RUST_MONSTER] &&
				objects[obj->otyp].oc_material == METAL &&
				!obj->rustfree && obj->spe > -2) {
		if(obj->blessed && rn2(3)) {
		    if (cansee(mdef->mx, mdef->my))
			pline("%s's weapon is not affected.", Monnam(magr));
		} else {
		    if (cansee(mdef->mx, mdef->my))
			pline("%s's %s!", Monnam(magr),
						aobjnam(obj, "corrode"));
		    obj->spe--;
		}
	}
}

static void
mswingsm(magr, mdef, otemp)
register struct monst *magr, *mdef;
register struct obj *otemp;
{
	char buf[BUFSZ];
	Strcpy(buf, mon_nam(mdef));
	if (!flags.verbose || Blind || otemp->olet != WEAPON_SYM) return;
	pline("%s %s %s %s at %s.", Monnam(magr),
	      ((otemp->otyp >= SPEAR &&
	        otemp->otyp <= LANCE) ||
	       (otemp->otyp >= PARTISAN &&
	        otemp->otyp <= SPETUM) ||
	       otemp->otyp == TRIDENT) ? "thrusts" : "swings",
	      is_female(magr) ? "her" :
	      is_human(magr->data) ? "his" : "its",
	      xname(otemp), buf);
}
