/*	SCCS Id: @(#)mhitm.c	3.1	93/02/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "edog.h"

#ifdef OVLB

static NEARDATA boolean vis, far_noise;
static NEARDATA long noisetime;
static NEARDATA struct obj *otmp;

static void FDECL(mrustm, (struct monst *, struct monst *, struct obj *));
static int FDECL(hitmm, (struct monst *,struct monst *,struct attack *));
static int FDECL(gazemm, (struct monst *,struct monst *,struct attack *));
static int FDECL(gulpmm, (struct monst *,struct monst *,struct attack *));
static int FDECL(explmm, (struct monst *,struct monst *,struct attack *));
static int FDECL(mdamagem, (struct monst *,struct monst *,struct attack *));
static void FDECL(mswingsm, (struct monst *, struct monst *, struct obj *));
static void FDECL(noises,(struct monst *,struct attack *));
static void FDECL(missmm,(struct monst *,struct monst *,struct attack *));
static int FDECL(passivemm, (struct monst *, struct monst *, BOOLEAN_P, int));

/* Needed for the special case of monsters wielding vorpal blades (rare).
 * If we use this a lot it should probably be a parameter to mdamagem()
 * instead of a global variable.
 */
static int dieroll;

static void
noises(magr, mattk)
	register struct monst *magr;
	register struct	attack *mattk;
{
	boolean farq = (distu(magr->mx, magr->my) > 15);

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
	const char *fmt;
	char buf[BUFSZ];

	if (vis) {
		if (mdef->m_ap_type) seemimic(mdef);
		if (magr->m_ap_type) seemimic(magr);
		fmt = (could_seduce(magr,mdef,mattk) && !magr->mcan) ?
			"%s pretends to be friendly to" : "%s misses";
		Sprintf(buf, fmt, Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	} else  noises(magr, mattk);
}

/*
 *  fightm()  -- fight some other monster
 *
 *  Returns:
 *	0 - Monster did nothing.
 *	1 - If the monster made an attack.  The monster might have died.
 *
 *  There is an exception to the above.  If mtmp has the hero swallowed,
 *  then we report that the monster did nothing so it will continue to
 *  digest the hero.
 */
int
fightm(mtmp)		/* have monsters fight each other */
	register struct monst *mtmp;
{
	register struct monst *mon, *nmon;
	int result, has_u_swallowed;
#ifdef LINT
	nmon = 0;
#endif
	/* perhaps the monster will resist Conflict */
	if(resist(mtmp, RING_CLASS, 0, 0))
	    return(0);
#ifdef POLYSELF
	if(u.ustuck == mtmp) {
	    /* perhaps we're holding it... */
	    if(itsstuck(mtmp))
		return(0);
	}
#endif
	has_u_swallowed = (u.uswallow && (mtmp == u.ustuck));

	for(mon = fmon; mon; mon = nmon) {
	    nmon = mon->nmon;
	    if(nmon == mtmp) nmon = mtmp->nmon;
	    if(mon != mtmp) {
		if(monnear(mtmp,mon->mx,mon->my)) {
		    if(!u.uswallow && (mtmp == u.ustuck)) {
			if(!rn2(4)) {
			    pline("%s releases you!", Monnam(mtmp));
			    u.ustuck = 0;
			} else
			    break;
		    }

		    /* mtmp can be killed */
		    bhitpos.x = mon->mx;
		    bhitpos.y = mon->my;
		    result = mattackm(mtmp,mon);

		    if (result & MM_AGR_DIED) return 1;	/* mtmp died */
		    /*
		     *  If mtmp has the hero swallowed, lie and say there
		     *  was no attack (this allows mtmp to digest the hero).
		     */
		    if (has_u_swallowed) return 0;

		    return ((result & MM_HIT) ? 1 : 0);
		}
	    }
	}
	return 0;
}

/*
 * mattackm() -- a monster attacks another monster.
 *
 * This function returns a result bitfield:
 *	   
 *	    --------- agressor died
 *	   /  ------- defender died
 *	  /  /  ----- defender was hit
 *	 /  /  /
 *	x  x  x
 *
 *	0x4	MM_AGR_DIED
 *	0x2	MM_DEF_DIED
 *	0x1	MM_HIT
 *	0x0	MM_MISS
 *
 * Each successive attack has a lower probability of hitting.  Some rely on the
 * success of previous attacks.  ** this doen't seem to be implemented -dl **
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(magr, mdef)
    register struct monst *magr,*mdef;
{
    int		    i,		/* loop counter */
		    tmp,	/* amour class difference */
		    strike,	/* hit this attack */
		    attk,	/* attack attempted this time */
		    struck = 0,	/* hit at least once */
		    res[NATTK];	/* results of all attacks */
    struct attack   *mattk;
    struct permonst *pa, *pd;

    if (!magr || !mdef) return(MM_MISS);		/* mike@genat */
    pa = magr->data; pd = mdef->data;
    if (!magr->mcanmove) return(MM_MISS);		/* riv05!a3 */

    /* Grid bugs cannot attack at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
						&& magr->my != mdef->my)
	return(MM_MISS);

    /* Calculate the armour class differential. */
    tmp = find_mac(mdef) + magr->m_lev;
    if (mdef->mconf || !mdef->mcanmove || mdef->msleep){
	tmp += 4;
	if (mdef->msleep) mdef->msleep = 0;
    }

    /* undetect monsters become un-hidden if they are attacked */
    if (mdef->mundetected) {
	mdef->mundetected = 0;
	newsym(mdef->mx, mdef->my);
	if(canseemon(mdef))
	    pline("Suddenly, you notice %s.", a_monnam(mdef));
    }

    /* Elves hate orcs. */
    if (is_elf(pa) && is_orc(pd)) tmp++;


    /* Set up the visibility of action */
    vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my));

    /*	Set flag indicating monster has moved this turn.  Necessary since a
     *	monster might get an attack out of sequence (i.e. before its move) in
     *	some cases, in which case this still counts as its move for the round
     *	and it shouldn't move again.
     */
    magr->mlstmv = monstermoves;

    /* Now perform all attacks for the monster. */
    for (i = 0; i < NATTK; i++) {
	res[i] = MM_MISS;
	mattk = &(pa->mattk[i]);
	otmp = (struct obj *)0;
	attk = 1;
	switch (mattk->aatyp) {
	    case AT_WEAP:		/* "hand to hand" attacks */
#ifdef MUSE
		if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
			magr->weapon_check = NEED_HTH_WEAPON;
			if (mon_wield_item(magr) != 0) return 0;
		}
		remove_cadavers(&magr->minvent);
		possibly_unwield(magr);
		otmp = MON_WEP(magr);
#else
		otmp = select_hwep(magr);
#endif
		if (otmp) {
		    if (vis) mswingsm(magr, mdef, otmp);
		    tmp += hitval(otmp, pd);
		}
		/* fall through */
	    case AT_CLAW:
	    case AT_KICK:
	    case AT_BITE:
	    case AT_STNG:
	    case AT_TUCH:
	    case AT_BUTT:
	    case AT_TENT:
		dieroll = rnd(20 + i);
		strike = (tmp > dieroll);
		if (strike)
		    res[i] = hitmm(magr, mdef, mattk);
		else
		    missmm(magr, mdef, mattk);
		break;

	    case AT_HUGS:	/* automatic if prev two attacks succeed */
		strike = (i >= 2 && res[i-1] == MM_HIT && res[i-2] == MM_HIT);
		if (strike)
		    res[i] = hitmm(magr, mdef, mattk);

		break;

	    case AT_GAZE:
		strike = 0;	/* will not wake up a sleeper */
		res[i] = gazemm(magr, mdef, mattk);
		break;

	    case AT_EXPL:
		strike = 1;	/* automatic hit */
		res[i] = explmm(magr, mdef, mattk);
		break;

	    case AT_ENGL:
		/* Engulfing attacks are directed at the hero if
		 * possible. -dlc
		 */
		if (u.uswallow && magr == u.ustuck)
		    strike = 0;
		else {
		    if ((strike = (tmp > rnd(20+i))))
			res[i] = gulpmm(magr, mdef, mattk);
		    else
			missmm(magr, mdef, mattk);
		}
		break;

	    default:		/* no attack */
		strike = 0;
		attk = 0;
		break;
	}

	if (attk && !(res[i] & MM_AGR_DIED))
	    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED);

	if (res[i] & MM_DEF_DIED) return res[i];

	/*
	 *  Wake up the defender.  NOTE:  this must follow the check
	 *  to see if the defender died.  We don't want to modify
	 *  unallocated monsters!
	 */
	if (strike) mdef->msleep = 0;

	if (res[i] & MM_AGR_DIED)  return res[i];
	/* return if aggressor can no longer attack */
	if (!magr->mcanmove || magr->msleep) return res[i];
	if (res[i] & MM_HIT) struck = 1;	/* at least one hit */
    }

    return(struck ? MM_HIT : MM_MISS);
}

/* Returns the result of mdamagem(). */
static int
hitmm(magr, mdef, mattk)
	register struct monst *magr,*mdef;
	struct	attack *mattk;
{
	if(vis){
		int compat;
		char buf[BUFSZ];

		if(mdef->m_ap_type) seemimic(mdef);
		if(magr->m_ap_type) seemimic(magr);
		if((compat = could_seduce(magr,mdef,mattk)) && !magr->mcan) {
			Sprintf(buf, "%s %s", Monnam(magr),
				mdef->mcansee ? "smiles at" : "talks to");
			pline("%s %s %s.", buf, mon_nam(mdef),
				compat == 2 ?
					"engagingly" : "seductively");
		} else {
		    char magr_name[BUFSZ];

		    Strcpy(magr_name, Monnam(magr));
		    switch (mattk->aatyp) {
			case AT_BITE:
				Sprintf(buf,"%s bites", magr_name);
				break;
			case AT_STNG:
				Sprintf(buf,"%s stings", magr_name);
				break;
			case AT_BUTT:
				Sprintf(buf,"%s butts", magr_name);
				break;
			case AT_TUCH:
				Sprintf(buf,"%s touches", magr_name);
				break;
			case AT_TENT:
				Sprintf(buf, "%s tentacles suck",
					s_suffix(magr_name));
				break;
			case AT_HUGS:
				if (magr != u.ustuck) {
				    Sprintf(buf,"%s squeezes", magr_name);
				    break;
				}
			default:
				Sprintf(buf,"%s hits", magr_name);
		    }
		}
		pline("%s %s.", buf, mon_nam(mdef));
	} else  noises(magr, mattk);
	return(mdamagem(magr, mdef, mattk));
}

/* Returns the same values as mdamagem(). */
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

	if (!mdef->mcansee || mdef->msleep) {
	    if(vis) pline("but nothing happens.");
	    return(MM_MISS);
	}

	return(mdamagem(magr, mdef, mattk));
}

/* Returns the same values as mattackm(). */
static int
gulpmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	xchar	ax, ay, dx, dy;
	int	status;
	char buf[BUFSZ];

	if (mdef->data->msize >= MZ_HUGE) return MM_MISS;

	if (vis) {
		Sprintf(buf,"%s swallows", Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	}

	/*
	 *  All of this maniuplation is needed to keep the display correct.
	 *  There is a flush at the next pline().
	 */
	ax = magr->mx;
	ay = magr->my;
	dx = mdef->mx;
	dy = mdef->my;
	/*
	 *  Leave the defender in the monster chain at it's current position,
	 *  but don't leave it on the screen.  Move the agressor to the def-
	 *  ender's position.
	 */
	remove_monster(ax, ay);
	place_monster(magr, dx, dy);
	newsym(ax,ay);			/* erase old position */
	newsym(dx,dy);			/* update new position */

	status = mdamagem(magr, mdef, mattk);

	if ((status & MM_AGR_DIED) && (status & MM_DEF_DIED)) {
	    ;					/* both died -- do nothing  */
	}
	else if (status & MM_DEF_DIED) {	/* defender died */
	    /*
	     *  Note:  remove_monster() was called in relmon(), wiping out
	     *  magr from level.monsters[mdef->mx][mdef->my].  We need to
	     *  put it back and display it.	-kd
	     */
	    place_monster(magr, dx, dy);
	    newsym(dx, dy);
	}
	else if (status & MM_AGR_DIED) {	/* agressor died */
	    place_monster(mdef, dx, dy);
	    newsym(dx, dy);
	}
	else {					/* both alive, put them back */
	    if (cansee(dx, dy))
		pline("%s is regurgitated!", Monnam(mdef));

	    place_monster(magr, ax, ay);
	    place_monster(mdef, dx, dy);
	    newsym(ax, ay);
	    newsym(dx, dy);
	}

	return status;
}

static int
explmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	int result, was_tame;

	if(cansee(magr->mx, magr->my))
		pline("%s explodes!", Monnam(magr));
	else	noises(magr, mattk);

	was_tame = magr->mtame;
	result = mdamagem(magr, mdef, mattk);

	/* The attacker could have died . . */
	if (was_tame)
	    You("have a sad feeling for a moment, then it passes.");

	/* Kill off agressor if it didn't die. */
	if (!(result & MM_AGR_DIED)) {
	    mondead(magr);
#ifdef MUSE
	    if (magr->mhp <= 0)
#endif
		result |= MM_AGR_DIED;
	}

	return result;
}

static const char psf[] =
	"have a peculiarly sad feeling for a moment, then it passes.";

/*
 *  See comment at top of mattackm(), for return values.
 */
static int
mdamagem(magr, mdef, mattk)
	register struct monst	*magr, *mdef;
	register struct attack	*mattk;
{
	struct	permonst *pa = magr->data, *pd = mdef->data;
	int	tmp = d((int)mattk->damn,(int)mattk->damd);
	char buf[BUFSZ];

	if (pd == &mons[PM_COCKATRICE] && !resists_ston(pa) &&
	   (mattk->aatyp != AT_WEAP || !otmp) &&
	   (mattk->aatyp != AT_GAZE && mattk->aatyp != AT_EXPL) &&
#ifdef MUSE
	   (!(magr->misc_worn_check & W_ARMG))) {
#else
	   (!is_mercenary(pa) || !m_carrying(magr, LEATHER_GLOVES))) {
	   /* Note: other monsters may carry gloves, only soldiers have them */
	   /* as their "armor" and can be said to wear them */
#endif
		if (poly_when_stoned(pa)) {
		    mon_to_stone(magr);
		    return MM_HIT; /* no damage during the polymorph */
		}
		if (vis) pline("%s turns to stone!", Monnam(magr));
		else if (magr->mtame) You(psf);
		monstone(magr);
		return MM_AGR_DIED;
	}

	switch(mattk->adtyp) {
	    case AD_DGST:
		if(flags.verbose && flags.soundok) verbalize("Burrrrp!");
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
		if (mattk->aatyp == AT_KICK && thick_skinned(pd))
			tmp = 0;
		else if(mattk->aatyp == AT_WEAP) {
		    if(otmp) {
#ifdef MUSE
			if (otmp->otyp == CORPSE &&
				otmp->corpsenm == PM_COCKATRICE)
			    goto do_stone_goto_label;
#endif
			tmp += dmgval(otmp, pd);
			if (otmp->oartifact) {
			    (void)artifact_hit(magr,mdef, otmp, &tmp, dieroll);
			    if (mdef->mhp <= 0)
				return (MM_DEF_DIED |
					(grow_up(magr,mdef) ? 0 : MM_AGR_DIED));
			}
			if (tmp)
				mrustm(magr, mdef, otmp);
		    }
		}
		break;
	    case AD_FIRE:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if(vis) pline("%s is on fire!", Monnam(mdef));
		tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if(resists_fire(pd)) {
		    if (vis)
			pline("The fire doesn't seem to burn %s!",
								mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_FIRE, tmp);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
	    case AD_COLD:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if(vis) pline("%s is covered in frost!", Monnam(mdef));
		if(resists_cold(pd)) {
		    if (vis)
			pline("The frost doesn't seem to chill %s!",
								mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
	    case AD_ELEC:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if(vis) pline("%s gets zapped!", Monnam(mdef));
		tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if(resists_elec(pd)) {
		    if (vis) pline("The zap doesn't shock %s!", mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_ELEC, tmp);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
	    case AD_ACID:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if(resists_acid(pd)) {
		    if (vis)
			pline("%s is covered in acid, but it seems harmless.",
							Monnam(mdef));
		    tmp = 0;
		} else if (vis) {
		    pline("%s is covered in acid!", Monnam(mdef));
		    pline("It burns %s!", mon_nam(mdef));
		}
		break;
	    case AD_RUST:
		if (!magr->mcan && pd == &mons[PM_IRON_GOLEM]) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			else if(mdef->mtame)
			     pline("May %s rust in peace.", mon_nam(mdef));
			mondied(mdef);
#ifdef MUSE
			if (mdef->mhp > 0) return 0;
#endif
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		tmp = 0;
		break;
	    case AD_DCAY:
		if (!magr->mcan && (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM])) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			else if(mdef->mtame)
			     pline("May %s rot in peace.", mon_nam(mdef));
			mondied(mdef);
#ifdef MUSE
			if (mdef->mhp > 0) return 0;
#endif
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		tmp = 0;
		break;
	    case AD_STON:
#ifdef MUSE
do_stone_goto_label:
#endif
		if(poly_when_stoned(pd)) {
		    mon_to_stone(mdef);
		    tmp = 0;
		    break;
		}
		if(!resists_ston(pd)) {
			if(vis) pline("%s turns to stone!", Monnam(mdef));
			else if(mdef->mtame) You(psf);
			monstone(mdef);
#ifdef MUSE
			if (mdef->mhp > 0) return 0;
#endif
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		tmp = (mattk->adtyp == AD_STON ? 0 : 1);
		break;
	    case AD_TLPT:
		if(!magr->mcan && tmp < mdef->mhp) {
		    rloc(mdef);
		    if(vis && !cansee(mdef->mx, mdef->my))
			pline("%s suddenly disappears!", Monnam(mdef));
		}
		break;
	    case AD_SLEE:
		if(!resists_sleep(pd) && !magr->mcan && !mdef->msleep
							&& mdef->mcanmove) {
		    if (vis) {
			Strcpy(buf, Monnam(mdef));
			pline("%s is put to sleep by %s.", buf, mon_nam(magr));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_PLYS:
		if(!magr->mcan && mdef->mcanmove) {
		    if (vis) {
			Strcpy(buf, Monnam(mdef));
			pline("%s is frozen by %s.", buf, mon_nam(magr));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_SLOW:
		if(!magr->mcan && vis && mdef->mspeed != MSLOW) {
		    if (vis) pline("%s slows down.", Monnam(mdef));
		    if (mdef->mspeed == MFAST) mdef->mspeed = 0;
		    else mdef->mspeed = MSLOW;
		}
		break;
	    case AD_CONF:
		/* Since confusing another monster doesn't have a real time
		 * limit, setting spec_used would not really be right (though
		 * we still should check for it).
		 */
		if (!magr->mcan && vis && !mdef->mconf && !magr->mspec_used) {
		    pline("%s looks confused.", Monnam(mdef));
		    mdef->mconf = 1;
		}
		break;
	    case AD_BLND:
		if (!magr->mcan && haseyes(pd)) {
		    register unsigned rnd_tmp;

		    if (vis && mdef->mcansee)
			pline("%s is blinded.", Monnam(mdef));
		    rnd_tmp = d((int)mattk->damn, (int)mattk->damd);
		    if ((rnd_tmp += mdef->mblinded) > 127) rnd_tmp = 127;
		    mdef->mblinded = rnd_tmp;
		    mdef->mcansee = 0;
		}
		tmp = 0;
		break;
	    case AD_CURS:
		if (!night() && (pa == &mons[PM_GREMLIN])) break;
		if (!magr->mcan && !rn2(10)) {
		    if (is_were(pd) && pd->mlet != S_HUMAN)
			were_change(mdef);
		    if (pd == &mons[PM_CLAY_GOLEM]) {
			    if (vis) {
				pline("Some writing vanishes from %s head!",
				    s_suffix(mon_nam(mdef)));
				pline("%s dies!", Monnam(mdef));
			    }
			    else if (mdef->mtame)
	You("have a strangely sad feeling for a moment, then it passes.");
			    mondied(mdef);
#ifdef MUSE
			    if (mdef->mhp > 0) return 0;
#endif
			    return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		      }
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
		if(rn2(2) && !resists_drli(pd)) {
			tmp = d(2,6);
			if (vis)
			    pline("%s suddenly seems weaker!", Monnam(mdef));
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
#ifdef MUSE
			possibly_unwield(mdef);
			if (otmp->owornmask) {
				mdef->misc_worn_check &= ~otmp->owornmask;
				otmp->owornmask = 0;
			}
			mselftouch(mdef, (const char *)0, FALSE);
			if (mdef->mhp <= 0)
				return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
#endif
		}
		tmp = 0;
		break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!magr->mcan && !rn2(8)) {
		    if (vis)
			pline("%s %s was poisoned!", s_suffix(Monnam(magr)),
				mattk->aatyp==AT_BITE ? "bite" : "sting");
		    if (resists_poison(pd)) {
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
	    case AD_DRIN:
		if (!has_head(pd)) {
		    if (vis) pline("%s doesn't seem harmed.", Monnam(mdef));
		    tmp = 0;
		    break;
		}
#ifdef MUSE
		if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
		    if (vis) {
			Strcpy(buf, s_suffix(Monnam(mdef)));
			pline("%s helmet blocks %s attack to his head.",
				buf, s_suffix(mon_nam(magr)));
		    }
		    break;
		}
#endif
		if (vis) pline("%s brain is eaten!", s_suffix(Monnam(mdef)));
		if (mindless(pd)) {
		    if (vis) pline("%s doesn't notice.", Monnam(mdef));
		    break;
		}
		tmp += rnd(10); /* fakery, since monsters lack INT scores */
		if (magr->mtame && !magr->isminion) {
		    EDOG(magr)->hungrytime += rnd(60);
		    magr->mconf = 0;
		}
		if (tmp >= mdef->mhp && vis)
		    pline("%s last thought fades away...", 
			          s_suffix(Monnam(mdef)));
		break;
	    case AD_STCK:
	    case AD_WRAP: /* monsters cannot grab one another, it's too hard */
		break;
	    default:	tmp = 0;
			break;
	}
	if(!tmp) return(MM_MISS);

	if((mdef->mhp -= tmp) < 1) {
	    if (m_at(mdef->mx, mdef->my) == magr) {  /* see gulpmm() */
		remove_monster(mdef->mx, mdef->my);
		place_monster(mdef, mdef->mx, mdef->my);
	    }
	    monkilled(mdef, "", mattk->adtyp);
	    if (mdef->mhp > 0) return 0; /* mdef lifesaved */
	    return (MM_DEF_DIED | (grow_up(magr,mdef) ? 0 : MM_AGR_DIED));
	}
	return(MM_HIT);
}

#endif /* OVLB */


#ifdef OVL0

int
noattacks(ptr)			/* returns 1 if monster doesn't attack */
	struct	permonst *ptr;
{
	int i;

	for(i = 0; i < NATTK; i++)
		if(ptr->mattk[i].aatyp) return(0);

	return(1);
}

#endif /* OVL0 */
#ifdef OVLB

static void
mrustm(magr, mdef, obj)
register struct monst *magr, *mdef;
register struct obj *obj;
{
	if (!magr || !mdef || !obj) return; /* just in case */
	if (mdef->data == &mons[PM_RUST_MONSTER] && !mdef->mcan &&
	    is_rustprone(obj) && obj->oeroded < MAX_ERODE) {
		if (obj->greased || obj->oerodeproof || (obj->blessed && rn2(3))) {
		    if (cansee(mdef->mx, mdef->my) && flags.verbose)
			pline("%s weapon is not affected.", 
			                 s_suffix(Monnam(magr)));
		    if (obj->greased && !rn2(2)) obj->greased = 0;
		} else {
		    if (cansee(mdef->mx, mdef->my)) {
			pline("%s %s%s!", s_suffix(Monnam(magr)),
			      aobjnam(obj, "rust"),
			      obj->oeroded ? " further" : "");
		    }
		    obj->oeroded++;
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
	if (!flags.verbose || Blind) return;
	pline("%s %s %s %s at %s.", Monnam(magr),
	      ((otemp->otyp >= SPEAR && otemp->otyp <= LANCE) ||
	       (otemp->otyp >= PARTISAN && otemp->otyp <= SPETUM) ||
	       otemp->otyp == TRIDENT) ? "thrusts" : "swings",
	      his[pronoun_gender(magr)], xname(otemp), buf);
}

/*
 * Passive responses by defenders.  Does not replicate responses already
 * handled above.  Returns same values as mattackm.
 */
static int
passivemm(magr,mdef,mhit,mdead)
register struct monst *magr, *mdef;
boolean mhit;
int mdead;
{
	register struct permonst *mddat = mdef->data;
	register struct permonst *madat = magr->data;
	char buf[BUFSZ];
	int i, tmp;

	for(i = 0; ; i++) {
	    if(i >= NATTK) return (mdead | mhit); /* no passive attacks */
	    if(mddat->mattk[i].aatyp == AT_NONE) break;
	}
	if (mddat->mattk[i].damn)
	    tmp = d((int)mddat->mattk[i].damn, 
                                    (int)mddat->mattk[i].damd);
	else if(mddat->mattk[i].damd)
	    tmp = d((int)mddat->mlevel+1, (int)mddat->mattk[i].damd);
	else
	    tmp = 0;

	/* These affect the enemy even if defender killed */
	switch(mddat->mattk[i].adtyp) {
	    case AD_ACID:
		if (mhit && !rn2(2)) {
		    Strcpy(buf, Monnam(magr));
		    if(canseemon(magr))
			pline("%s is splashed by %s acid!",
			      buf, s_suffix(mon_nam(mdef)));
		    if(resists_acid(madat)) {
			if(canseemon(magr))
			    pline("%s is not affected.", Monnam(magr));
			tmp = 0;
		    }
		} else tmp = 0;
		goto assess_dmg;
	    default:
		break;
	}
	if (mdead || mdef->mcan) return (mdead|mhit);

	/* These affect the enemy only if defender is still alive */
	if (rn2(3)) switch(mddat->mattk[i].adtyp) {
	    case AD_PLYS: /* Floating eye */
		if (mddat == &mons[PM_FLOATING_EYE]) {
		    if (magr->mcansee && haseyes(madat) && mdef->mcansee &&
			(perceives(madat) || !mdef->minvis)) {
			Strcpy(buf, Monnam(magr));
			if(canseemon(magr))
			    pline("%s is frozen by %s gaze!",
				  buf, s_suffix(mon_nam(mdef)));
			magr->mcanmove = 0;
			magr->mfrozen = tmp;
			return (mdead|mhit);
		    }
		} else { /* gelatinous cube */
		    Strcpy(buf, Monnam(magr));
		    if(canseemon(magr))
			pline("%s is frozen by %s.", buf, mon_nam(mdef));
		    magr->mcanmove = 0;
		    magr->mfrozen = tmp;
		    return (mdead|mhit);
		}
		return 1;
	    case AD_COLD:
		if (resists_cold(madat)) {
		    if (canseemon(magr)) {
			pline("%s is mildly chilly.", Monnam(magr));
			golemeffects(magr, AD_COLD, tmp);
			tmp = 0;
			break;
		    }
		}
		if(canseemon(magr))
		    pline("%s is suddenly very cold!", Monnam(magr));
		mdef->mhp += tmp / 2;
		if (mdef->mhpmax < mdef->mhp) mdef->mhpmax = mdef->mhp;
		if (mdef->mhpmax > ((int) (mdef->m_lev+1) * 8)) {
		    register struct monst *mtmp;

		    if ((mtmp = clone_mon(mdef)) != 0) {
			mtmp->mhpmax = mdef->mhpmax /= 2;
			if(canseemon(magr)) {
			    Strcpy(buf, Monnam(mdef));
			    pline("%s multiplies from %s heat!",
				    buf, s_suffix(mon_nam(magr)));
			}
		    }
		}
		break;
	    case AD_STUN:
		if (!magr->mstun) {
		    magr->mstun = 1;
		    if (canseemon(magr))
			pline("%s staggers....", Monnam(magr));
		}
		tmp = 0;
		break;
	    case AD_FIRE:
		if (resists_fire(madat)) {
		    if (canseemon(magr)) {
			pline("%s is mildly warmed.", Monnam(magr));
			golemeffects(magr, AD_FIRE, tmp);
			tmp = 0;
			break;
		    }
		}
		if(canseemon(magr))
		    pline("%s is suddenly very hot!", Monnam(magr));
		break;
	    case AD_ELEC:
		if (resists_elec(madat)) {
		    if (canseemon(magr)) {
			pline("%s is mildly tingled.", Monnam(magr));
			golemeffects(magr, AD_ELEC, tmp);
			tmp = 0;
			break;
		    }
		}
		if(canseemon(magr))
		    pline("%s is jolted with electricity!", Monnam(magr));
		break;
	    default: tmp = 0;
		break;
	}
	else tmp = 0;

    assess_dmg:
	if((magr->mhp -= tmp) <= 0) {
		monkilled(magr,"",mddat->mattk[i].adtyp);
		return (mdead | mhit | MM_AGR_DIED);
	}
	return (mdead | mhit);
}

#endif /* OVLB */

/*mhitm.c*/
