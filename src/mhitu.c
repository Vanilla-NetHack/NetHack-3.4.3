/*	SCCS Id: \@(#)mhitu.c	3.0	88/10/28
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#ifdef NAMED_ITEMS
#  include "artifact.h"
#endif

static struct obj *otmp;
#ifdef POLYSELF
static void urustm P((struct monst *, struct obj *));
static int passiveum P((struct permonst *, struct monst *));
#endif
#ifdef SEDUCE
static void mayberem P((struct obj *, char *));
#endif
static int hitmu P((struct monst *,struct attack *));
static int gulpmu P((struct monst *,struct attack *));
static int explmu P((struct monst *,struct attack *));
static int gazemu P((struct monst *,struct attack *));

#ifdef POLYSELF
boolean
/* also needed in uhitm.c */
#else
static boolean
#endif
incompatible(mon)
register struct monst *mon;
{
	return(poly_gender() != 1-gender(mon));
}

boolean
is_nymph(mon)
register struct monst *mon;
{
	return( mon->data->mlet == S_NYMPH );
}

boolean
sp_melee(mon)
register struct monst *mon;
{
	return(
#ifdef SEDUCE
		(mon->data == &mons[PM_SUCCUBUS] && !mon->minvis) ||
		(mon->data == &mons[PM_INCUBUS] && !mon->minvis) ||
#endif
		is_nymph(mon));
}

static void
hitmsg(mtmp, attyp)
register struct monst *mtmp;
register uchar attyp;
{
	/* Note: if opposite gender, "seductively" */
	/* If same gender, "engagingly" for nymph, normal msg for others */
	if(sp_melee(mtmp) && !mtmp->mcan && !mtmp->mspec_used) {
		if(!is_nymph(mtmp) && incompatible(mtmp)) goto strike;
	    	kludge("%s %s you %s.", Monnam(mtmp),
			Blind ? "talks to" : "smiles at",
			incompatible(mtmp) ? "engagingly" : "seductively");
	} else
strike:
	    switch (attyp) {
		case AT_BITE:
			kludge("%s bites!", Monnam(mtmp));
			break;
		case AT_KICK:
#ifdef POLYSELF
			kludge("%s kicks%c", Monnam(mtmp), thick_skinned(uasmon) ? '.' : '!');
#else
			kludge("%s kicks!", Monnam(mtmp));
#endif
			break;
		case AT_STNG:
			kludge("%s stings!", Monnam(mtmp));
			break;
		case AT_BUTT:
			kludge("%s butts!", Monnam(mtmp));
			break;
		case AT_TUCH:
			kludge("%s touches you!", Monnam(mtmp));
			break;
		default:
			kludge("%s hits!", Monnam(mtmp));
	    }
}

static void
missmu(mtmp, nearmiss)		/* monster missed you */
register struct monst *mtmp;
register boolean nearmiss;
{
	if(sp_melee(mtmp) && !mtmp->mcan) {
	    if(!is_nymph(mtmp) && incompatible(mtmp)) goto strike;
	    kludge("%s pretends to be friendly.", Monnam(mtmp));
	} else {
strike:
	    if (!flags.verbose || !nearmiss)
		kludge("%s misses.", Monnam(mtmp));
	    else
		kludge("%s just misses!", Monnam(mtmp));
	}
}

static void
mswings(mtmp, otemp)		/* monster swings obj */
register struct monst *mtmp;
register struct obj *otemp;
{
	if (!flags.verbose || Blind || otemp->olet != WEAPON_SYM) return;
	pline("%s %s %s %s.", Monnam(mtmp),
	      (otemp->otyp == SPEAR ||
	       otemp->otyp == LANCE ||
	       otemp->otyp == GLAIVE ||
	       otemp->otyp == TRIDENT) ? "thrusts" : "swings",
	      is_female(mtmp) ? "her" :
	      is_human(mtmp->data) ? "his" : "its",
	      xname(otemp));
}

static void
wildmiss(mtmp)		/* monster attacked your displaced image */
	register struct monst *mtmp;
{
	if (!flags.verbose) return;
	if (!cansee(mtmp->mx, mtmp->my)) return;
		/* maybe it's attacking an image around the corner? */
	if(Invis && !perceives(mtmp->data)) {
	    if(sp_melee(mtmp) && !mtmp->mcan) {
		if(!is_nymph(mtmp) && incompatible(mtmp)) goto strike;
		kludge("%s tries to touch you and misses!", Monnam(mtmp));
	    } else
strike:
		switch(rn2(3)) {
		case 0: kludge("%s swings wildly and misses!", Monnam(mtmp));
		    break;
		case 1: kludge("%s attacks a spot beside you.", Monnam(mtmp));
		    break;
		case 2: kludge("%s strikes at thin air!", Monnam(mtmp));
		    break;
		default:kludge("%s swings wildly!", Monnam(mtmp));
		    break;
		}
	}
	else if(Displaced) {
	    if(sp_melee(mtmp) && !mtmp->mcan) {
		if(!is_nymph(mtmp) && incompatible(mtmp)) goto strikem;
		kludge("%s smiles %s at your %sdisplaced image...",
			Monnam(mtmp),
			incompatible(mtmp) ? "engagingly" : "seductively",
			Invis ? "invisible " : "");
	   } else
strikem:
		kludge("%s strikes at your %sdisplaced image and misses you!",
			/* Note: if you're both invisible and displaced,
			 * only monsters which see invisible will attack your
			 * displaced image, since the displaced image is also
			 * invisible.
			 */
			Monnam(mtmp),
			Invis ? "invisible " : "");
	}
	else impossible("%s attacks you without knowing your location?",
		Monnam(mtmp));
}

static void
regurgitates(mtmp)
register struct monst *mtmp;
{
	u.ux = mtmp->mx;
	u.uy = mtmp->my;
	u.uswallow = 0;
	u.ustuck = 0;
	mnexto(mtmp);
	setsee();
	docrt();
	spoteffects();
	/* to cover for a case where mtmp is not in a next square */
	if(um_dist(mtmp->mx,mtmp->my,1))
		pline("Brrooaa...  You land hard at some distance.");
}

/*
 * mattacku: monster attacks you
 *	returns 1 if monster dies (e.g. "yellow light"), 0 otherwise
 *	Note: if you're displaced or invisible the monster might attack the
 *		wrong position...
 *	Assumption: it's attacking you or an empty square; if there's another
 *		monster which it attacks by mistake, the caller had better
 *		take care of it...
 */
int
mattacku(mtmp)
	register struct monst *mtmp;
{
	struct	attack	*mattk;
	int	i, j, tmp, sum[NATTK];
	struct	permonst *mdat = mtmp->data;
	boolean ranged = (dist(mtmp->mx, mtmp->my) > 3);
		/* Is it near you?  Affects your actions */
	boolean range2 = (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 3);
		/* Does it think it's near you?  Affects its actions */
	boolean foundyou = (mtmp->mux==u.ux && mtmp->muy==u.uy);
		/* Is it attacking you or your image? */
	boolean youseeit = (cansee(mtmp->mx, mtmp->my));
		/* Necessary since if it's attacking your image around a
		 * corner, you might not see it
		 */

	if(!ranged) nomul(0);
	if(mtmp->mhp <= 0) return(0);

	/* If swallowed, can only be affected by u.ustuck */
	if(u.uswallow) {
	    if(mtmp != u.ustuck)
		return(0);
	    u.ustuck->mux = u.ux;
	    u.ustuck->muy = u.uy;
	    range2 = 0;
	    foundyou = 1;
	    /* This is not impossible! */
	    /* If the swallowing monster changes into a monster
	     * that is not capable of swallowing you, you get
	     * regurgitated - dgk
	     */
	    for(i = 0; i < NATTK; i++)
		if(mdat->mattk[i].aatyp == AT_ENGL) goto doattack;

	    You("get regurgitated!");
	    regurgitates(mtmp);
	    return(0);
	}
doattack:
#ifdef POLYSELF
	if (u.uundetected && !range2 && foundyou) {
		u.uundetected = 0;
		if (u.usym == S_PIERCER) {
		    coord cc; /* maybe we need a unexto() function? */

		    unpmon(mtmp);
		    levl[mtmp->mx][mtmp->my].mmask = 0;
		    mtmp->mx = u.ux; mtmp->my = u.uy;
		    levl[mtmp->mx][mtmp->my].mmask = 1;
		    pmon(mtmp);
		    enexto(&cc, u.ux, u.uy, &playermon);
		    teleds(cc.x, cc.y);
		    You("fall from the ceiling!");
		    if (is_mercenary(mtmp->data) && m_carrying(mtmp,HELMET)) {
			kludge("Your blow glances off %s's helmet.",
								mon_nam(mtmp));
		    } else {
			if (3 + mtmp->data->ac <= rnd(20)) {
			    kludge("%s is hit by a falling piercer (you)!",
								Monnam(mtmp));
			    if ((mtmp->mhp -= d(3,6)) < 1)
				killed(mtmp);
			} else
			  kludge("%s is almost hit by a falling piercer (you)!",
			    					Monnam(mtmp));
		    }
		} else {
		    if (Blind) pline("It tries to move where you are hiding.");
		    else
		     pline("Wait, %s!  There's a %s named %s hiding under %s!",
			mtmp->mnamelth ? NAME(mtmp) : mtmp->data->mname,
			uasmon->mname, plname,
			levl[u.ux][u.uy].omask ? doname(o_at(u.ux,u.uy)) :
			"some gold");
		    prme();
		}
		return(0);
	}
	if (u.usym == S_MIMIC_DEF && !range2 && foundyou) {
		if (Blind) pline("It gets stuck on you.");
		    else pline("Wait, %s!  That's a %s named %s!",
			mtmp->mnamelth ? NAME(mtmp) : mtmp->data->mname,
			uasmon->mname, plname);
		u.ustuck = mtmp;
		u.usym = S_MIMIC;
		prme();
		return(0);
	}
#endif
/*	Work out the armor class differential	*/
	tmp = u.uac + 10;		/* tmp ~= 0 - 20 */
/*	give people with Ac < -9 at least some vulnerability */
/*	negative AC gives an actual AC of somewhere from -1 to the AC */
	if (tmp < 10) tmp = 10 - rnd(10-tmp);
	tmp += mtmp->m_lev;
	if(multi < 0) tmp += 4;
	if((Invis && !perceives(mdat)) || !mtmp->mcansee)
		tmp -= 2;
	if(mtmp->mtrapped) tmp -= 2;
	if(tmp <= 0) tmp = 1;

	/* make eels visible the moment they hit/miss us */
	if(mdat->mlet == S_EEL && mtmp->minvis && cansee(mtmp->mx,mtmp->my)) {
		mtmp->minvis = 0;
		pmon(mtmp);
	}

/*	Special demon handling code */
	if(!mtmp->cham && is_demon(mdat) && !range2
#ifdef HARD
	   && mtmp->data != &mons[PM_BALROG]
	   && mtmp->data != &mons[PM_SUCCUBUS]
	   && mtmp->data != &mons[PM_INCUBUS]
#endif
	   )
	    if(!mtmp->mcan && !rn2(13))	dsummon(mdat);

/*	Special lycanthrope handling code */
	if(!mtmp->cham && is_were(mdat) && !range2) {

	    if(is_human(mdat)) {
		if(!rn2(5 - (night() * 2)) && !mtmp->mcan) new_were(mtmp);
	    } else if(!rn2(30) && !mtmp->mcan) new_were(mtmp);

	    if(!rn2(10) && !mtmp->mcan) {
		if(!Blind) {
			pline("%s summons help!",youseeit ?
				Monnam(mtmp) : "It");
		} else
			You("feel hemmed in.");
		/* Technically wrong; we really should check if you can see the
		 * help, but close enough...
		 */
		if (!were_summon(mdat,FALSE) && !Blind)
		    pline("But none comes.");
	    }
	}

	for(i = 0; i < NATTK; i++) {

	    mattk = &(mdat->mattk[i]);
	    switch(mattk->aatyp) {
		case AT_CLAW:	/* "hand to hand" attacks */
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
			if(!range2) {
			    if (!foundyou) {
				wildmiss(mtmp);
				sum[i] = 0;
			    } else if(tmp > (j = rnd(20+i)))
#ifdef POLYSELF
				if (mattk->aatyp == AT_KICK &&
					thick_skinned(uasmon)) sum[i] = 0;
			        else
#endif
					sum[i] = hitmu(mtmp, mattk);
			    else {
				missmu(mtmp, (tmp == j));
				sum[i] = 0;
			    }
			} else	sum[i] = 0;
			break;

		case AT_HUGS:	/* automatic if prev two attacks succeed */
			/* Note: if displaced, prev attacks never succeeded */
			if(!range2) {
			    if(sum[i-1] && sum[i-2])
				sum[i]= hitmu(mtmp, mattk);
			    else sum[i] = 0;
			} else	 sum[i] = 0;
			break;

		case AT_GAZE:	/* can affect you either ranged or not */
			if (!youseeit) sum[i] = 0;
			    /* Displaced and around a corner so not visible */
			else sum[i] = gazemu(mtmp, mattk);
			break;

		case AT_EXPL:	/* automatic hit if next to, and aimed at you */
			if(!range2) {
			    if (!foundyou) {
				if (!mtmp->mcan) {
				    pline("%s explodes at a spot in thin air!",
					youseeit ? Monnam(mtmp) : "It");
				    mondead(mtmp);
				    sum[i] = 2;
				} else sum[i] = 0;
			    } else    sum[i] = explmu(mtmp, mattk);
			} else sum[i] = 0;
			break;

		case AT_ENGL:
			if (!range2) {
			    if(foundyou) {
				if(u.uswallow || tmp > (j = rnd(20+i))) {
				    /* Force swallowing monster to be
				     * displayed even when player is
				     * moving away */
				    nscr();
				    sum[i] = gulpmu(mtmp, mattk);
				} else {
				    missmu(mtmp, (tmp == j));
				    sum[i] = 0;
				}
			    } else pline("%s gulps some air!", youseeit ?
				Monnam(mtmp) : "It");
			} else	sum[i] = 0;
			break;
		case AT_BREA:
			if(range2) sum[i] = breamu(mtmp, mattk);
			/* Note: breamu takes care of displacement */
			else	   sum[i] = 0;
			break;
		case AT_SPIT:
			if(range2) sum[i] = spitmu(mtmp);
			/* Note: spitmu takes care of displacement */
			else	   sum[i] = 0;
			break;
		case AT_WEAP:
			if(range2) {
#ifdef REINCARNATION
				if (dlevel != rogue_level)
#endif
					sum[i] = thrwmu(mtmp);
			} else {
			    if (!foundyou) {
				wildmiss(mtmp);
				sum[i] = 0;
			    } else {
				set_uasmon();
				otmp = select_hwep(mtmp);
				if(otmp) {
				    tmp += hitval(otmp, uasmon);
				    mswings(mtmp, otmp);
				}
				if(tmp > (j = rnd(20+i)))
				    sum[i] = hitmu(mtmp, mattk);
				else {
				    missmu(mtmp, (tmp == j));
				    sum[i] = 0;
				}
			    }
			}
			break;
		case AT_MAGC:
			if(!range2) {
			    if (!foundyou) {
				pline("%s casts a spell at thin air!",
					youseeit ? Monnam(mtmp) : "It");
				sum[i] = 0;
				/* Not totally right since castmu allows other
				 * spells, such as the monster healing itself,
				 * that should work even when not next to you--
				 * but the previous code was just as wrong.
				 * --KAA
				 */
			    } else sum[i] = castmu(mtmp, mattk);
			} else	sum[i] = buzzmu(mtmp, mattk);
			break;

		default:		/* no attack */
			sum[i] = 0;
			break;
	    }
	    if(flags.botl) bot();
	    if(sum[i] == 2)  return(1);  	/* attacker dead */
	}
	return(0);
}

/*
 * hitmu: monster hits you
 *	  returns 2 if monster dies (e.g. "yellow light"), 0 otherwise
 */
static
int
hitmu(mtmp, mattk)
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	register struct permonst *mdat = mtmp->data;
	register int dmg, ctmp, ptmp;
	register boolean getbronze;
	char	 buf[BUFSZ];
#ifdef POLYSELF
	struct permonst *olduasmon = uasmon;
	int res;
#endif

/*	If the monster is undetected & hits you.  You should know where
 *	the attack came from.
 */
	if(mtmp->mhide && mtmp->mundetected) {
	    mtmp->mundetected = 0;
	    if(!(Blind ? Telepat : (HTelepat & WORN_HELMET))) {
		register struct obj *obj;

		if(levl[mtmp->mx][mtmp->my].omask == 1) {
		    if(obj = o_at(mtmp->mx,mtmp->my))
			pline("%s was hidden under %s!",
				  Xmonnam(mtmp), doname(obj));
		} else if (levl[mtmp->mx][mtmp->my].gmask == 1)
			pline("%s was hidden under some gold!",
				  Xmonnam(mtmp));
	    }
	}

/*	First determine the base damage done */
	dmg = d((int)mattk->damn, (int)mattk->damd);
	if(is_undead(mdat) && midnight())
		dmg += d((int)mattk->damn, (int)mattk->damd); /* extra damage */

/*	Next a cancellation factor	*/
/*	Use ctmp when the cancellation factor takes into account certain
 *	armor's special magic protection.  Otherwise just use !mtmp->mcan.
 */
	ctmp = !mtmp->mcan &&
		(!uarm || (rn2(3) >= objects[uarm->otyp].a_can) || !rn2(50))
	     && (!uarmc || (rn2(3) >= objects[uarmc->otyp].a_can) || !rn2(50));

/*	Now, adjust damages via resistances or specific attacks */
	switch(mattk->adtyp) {
	    case AD_PHYS:
		if(mattk->aatyp == AT_HUGS
#ifdef POLYSELF
					   && !sticks(uasmon)
#endif
								) {
		    if(!u.ustuck && rn2(2)) {
			u.ustuck = mtmp;
			kludge("%s grabs you!", Monnam(mtmp));
		    } else if(u.ustuck == mtmp)
			You("are being %s.",
#ifdef GOLEMS
			      (mtmp->data == &mons[PM_ROPE_GOLEM])
			      ? "choked" :
#endif /* GOLEMS */
			      "crushed");

		} else {			  /* hand to hand weapon */
		    hitmsg(mtmp,mattk->aatyp);
		    if(mattk->aatyp == AT_WEAP && otmp) {
			dmg += dmgval(otmp, uasmon);
			if (dmg <= 0) dmg = 1;
#ifdef NAMED_ITEMS
			if (spec_ability(otmp, SPFX_DRLI)
#  ifdef POLYSELF
						&& !resists_drli(uasmon)
#  endif
									) {
				pline("The %s blade drains your life!",
					Hallucination ? hcolor() : black);
				losexp();
			}
#endif
#ifdef POLYSELF
			if (u.mh > 1 && u.mh > ((u.uac>0) ? dmg : dmg+u.uac) &&
					(u.umonnum==PM_BLACK_PUDDING
					|| u.umonnum==PM_BROWN_PUDDING)) {
			    /* This redundancy necessary because you have to
			     * take the damage _before_ being cloned.
			     */
			    if (u.uac < 0) dmg += u.uac;
			    if (dmg < 1) dmg = 1;
			    u.mh -= dmg;
			    flags.botl = 1;
			    dmg = 0;
			    if(cloneu())
			    kludge("You divide as %s hits you!",mon_nam(mtmp));
			}
			urustm(mtmp, otmp);
#endif
		    }
		}
		break;
	    case AD_DISE:
		hitmsg(mtmp, mattk->aatyp);
		You("feel very sick.");
		make_sick((long)rn1(25-ACURR(A_CON),15),FALSE);
		u.usick_cause = mdat->mname;
		break;
	    case AD_FIRE:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp && rn2(2)) {
		    pline("You're on fire!");
		    if (Fire_resistance) {
			pline("The fire doesn't feel hot!");
			dmg = 0;
		    }
		    if(mtmp->m_lev > rn2(20))
			destroy_item(SCROLL_SYM, AD_FIRE);
		    if(mtmp->m_lev > rn2(20))
			destroy_item(POTION_SYM, AD_FIRE);
#ifdef SPELLS
		    if(mtmp->m_lev > rn2(25))
			destroy_item(SPBOOK_SYM, AD_FIRE);
#endif
		}
		break;
	    case AD_COLD:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp && rn2(2)) {
		    pline("You're covered in frost!");
		    if (Cold_resistance) {
			pline("The frost doesn't seem cold!");
			dmg = 0;
		    }
		    if(mtmp->m_lev > rn2(20))
			destroy_item(POTION_SYM, AD_COLD);
		}
		break;
	    case AD_ELEC:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp && rn2(2)) {
		    You("get zapped!");
		    if (Shock_resistance) {
			pline("The zap doesn't shock you!");
			dmg = 0;
		    }
		    if(mtmp->m_lev > rn2(20))
			destroy_item(WAND_SYM, AD_ELEC);
		    if(mtmp->m_lev > rn2(20))
			destroy_item(RING_SYM, AD_ELEC);
		}
		break;
	    case AD_SLEE:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp && multi >= 0 && !rn2(5)) {
		    if (Sleep_resistance) break;
		    nomul(-rnd(10));
		    if (Blind)	You("are put to sleep!");
		    else	You("are put to sleep by %s!",mon_nam(mtmp));
		}
		break;
	    case AD_DRST:
		ptmp = A_STR;
		goto dopois;
	    case AD_DRDX:
		ptmp = A_DEX;
		goto dopois;
	    case AD_DRCO:
		ptmp = A_CON;
dopois:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp && !rn2(8)) {
			Sprintf(buf, "%s's %s",
				Hallucination ? rndmonnam() : mdat->mname,
				(mattk->aatyp == AT_BITE) ? "bite" : "sting");
			poisoned(buf, ptmp, mdat->mname);
		}
		break;
	    case AD_PLYS:
		hitmsg(mtmp, mattk->aatyp);
		if(ctmp && multi >= 0 && !rn2(3)) {
		    if (Blind)	You("are frozen!");
		    else	You("are frozen by %s!", mon_nam(mtmp));
		    nomul(-rnd(10));
		}
		break;
	    case AD_DRLI:
		hitmsg(mtmp, mattk->aatyp);
		if (ctmp && !rn2(3)
#ifdef POLYSELF
		    && !resists_drli(uasmon)
#endif
#ifdef NAMED_ITEMS
		    && !defends(AD_DRLI, uwep)
#endif
		    ) losexp();
		break;
	    case AD_LEGS:
		{ register long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
		  if (mtmp->mcan) {
		    pline("%s nuzzles against your %s %s!", Monnam(mtmp),
			  (side == RIGHT_SIDE) ? "right" : "left",
			  body_part(LEG));
		  } else {
		    if (uarmf) {
			pline("%s scratches your %s boot!", Monnam(mtmp),
				(side == RIGHT_SIDE) ? "right" : "left");
			break;
		    }
		    pline("%s pricks your %s %s!", Monnam(mtmp),
			  (side == RIGHT_SIDE) ? "right" : "left",
			  body_part(LEG));
		    set_wounded_legs(side, rnd(60-ACURR(A_DEX)));
		  }
		  break;
		}
	    case AD_STON:	/* at present only a cockatrice */
		hitmsg(mtmp,mattk->aatyp);
		if(!rn2(3) && !Stoned) {
		    if (mtmp->mcan) {
			if (flags.soundok)
			    You("hear a cough from %s!", mon_nam(mtmp));
		    } else {
			if (flags.soundok)
			    You("hear %s's hissing!", mon_nam(mtmp));
			if((!rn2(10) ||
			    (flags.moonphase == NEW_MOON &&
			     !carrying(DEAD_LIZARD)))
#ifdef POLYSELF
			    && !resists_ston(uasmon)
#endif
			    ) {
				Stoned = 5;
				return(1);
				/* You("turn to stone..."); */
				/* done_in_by(mtmp); */
			}
		    }
		}
		break;
	    case AD_STCK:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp && !u.ustuck
#ifdef POLYSELF
				     && !sticks(uasmon)
#endif
							) u.ustuck = mtmp;
		break;
	    case AD_WRAP:
		if(ctmp
#ifdef POLYSELF
			&& !sticks(uasmon)
#endif
					  ) {
		    if(!u.ustuck && !rn2(10)) {
			pline("%s swings itself around you!",
				Monnam(mtmp));
			u.ustuck = mtmp;
		    } else if(u.ustuck == mtmp) {
			if (is_pool(mtmp->mx,mtmp->my)
#ifdef POLYSELF
			    && !is_swimmer(uasmon)
#endif
			   ) {
			    pline("%s drowns you...", Monnam(mtmp));
			    done("drowned");
			} else if(mattk->aatyp == AT_HUGS)
			    You("are being crushed.");
		    } else dmg = 0;
		} else dmg = 0;
		break;
	    case AD_WERE:
		hitmsg(mtmp,mattk->aatyp);
#ifdef POLYSELF
		if (ctmp && !rn2(4) && u.ulycn == -1
# ifdef NAMED_ITEMS
		    && !defends(AD_WERE,uwep)
# endif
		    ) {
		    You("feel feverish.");
		    u.ulycn = monsndx(mdat);
		}
#endif
		break;
	    case AD_SGLD:
		hitmsg(mtmp,mattk->aatyp);
#ifdef POLYSELF
		if (u.usym == mdat->mlet) break;
#endif
		if(!mtmp->mcan) stealgold(mtmp);
		break;

	    case AD_SITM:	/* for now these are the same */
	    case AD_SEDU:
#ifdef POLYSELF
		if (dmgtype(uasmon, AD_SEDU)
#  ifdef SEDUCE
			|| dmgtype(uasmon, AD_SSEX)
#  endif
						) {
			if (mtmp->minvent)
	pline("%s brags about the goods some dungeon explorer provided.",
	Monnam(mtmp));
			else
	pline("%s makes some remarks about how difficult theft is lately.",
	Monnam(mtmp));
			rloc(mtmp);
		} else
#endif
		if(mtmp->mcan) {
		    if (!Blind) {
			pline("%s tries to %s you, but you seem %s.",
			    Amonnam(mtmp, "plain"),
			    flags.female ? "charm" : "seduce",
			    flags.female ? "unaffected" : "uninterested");
		    }
		    if(rn2(3)) rloc(mtmp);
		} else if(steal(mtmp)) {
			rloc(mtmp);
			mtmp->mflee = 1;
		}
		break;
#ifdef SEDUCE
	    case AD_SSEX:
		if(!mtmp->mcan && !mtmp->minvis) doseduce(mtmp);
		break;
#endif
	    case AD_SAMU:
		hitmsg(mtmp,mattk->aatyp);
		/* when the Wiz hits, 1/20 steals the amulet */
		if (!carrying(AMULET_OF_YENDOR)) break;
		if (!rn2(20)) stealamulet(mtmp);
		break;

	    case AD_TLPT:
		hitmsg(mtmp,mattk->aatyp);
		if(ctmp) {
		    if(flags.verbose)
			Your("position suddenly seems very uncertain!");
		    tele();
		}
		break;
	    case AD_RUST:
		hitmsg(mtmp,mattk->aatyp);
		if (mtmp->mcan) break;
#ifdef POLYSELF
#ifdef GOLEMS
		if (u.umonnum == PM_IRON_GOLEM) {
			You("rust!");
			rehumanize();
			break;
		}
#endif /* GOLEMS */
#endif
		/* What the following code does: it keeps looping until it
		 * finds a target for the rust monster.
		 * Head, feet, etc... not covered by metal, or covered by
		 * rusty metal, are not targets.  However, your body always
		 * is, no matter what covers it.
		 */
		getbronze = (mdat == &mons[PM_BLACK_PUDDING] &&
			     uarm && uarm->otyp == BRONZE_PLATE_MAIL);
		while (1) {
		    switch(rn2(5)) {
		    case 0:
			if (!rust_dmg(uarmh, "helmet", 1, FALSE)) continue;
			break;
		    case 1:
			if (uarmc) break;
			/* Note the difference between break and continue;
			 * break means it was hit and didn't rust; continue
			 * means it wasn't a target and though it didn't rust
			 * something else did.
			 */
			if (getbronze)
			    (void)rust_dmg(uarm, "bronze armor", 3, TRUE);
			else
			    (void)rust_dmg(uarm, "armor", 1, TRUE);
			break;
		    case 2:
			if (!rust_dmg(uarms, "shield", 1, FALSE)) continue;
			break;
		    case 3:
			if (!rust_dmg(uarmg, "metal gauntlets", 1, FALSE))
			    continue;
			break;
		    case 4:
			if (!rust_dmg(uarmf, "metal boots", 1, FALSE)) continue;
			break;
		    }
		    break; /* Out of while loop */
		}
		break;
	    case AD_DCAY:
		hitmsg(mtmp,mattk->aatyp);
		if (mtmp->mcan) break;
#ifdef POLYSELF
#ifdef GOLEMS
		if (u.umonnum == PM_WOOD_GOLEM ||
		    u.umonnum == PM_LEATHER_GOLEM) {
			You("rot!");
			rehumanize();
			break;
		}
#endif /* GOLEMS */
#endif
		while (1) {
		    switch(rn2(5)) {
		    case 0:
			if (!rust_dmg(uarmh, "leather helmet", 2, FALSE))
				continue;
			break;
		    case 1:
			if (uarmc) break;
			(void)rust_dmg(uarm, "leather armor", 2, TRUE);
			break;
		    case 2:
			if (!rust_dmg(uarms, "wooden shield", 2, FALSE))
				continue;
			break;
		    case 3:
			if (!rust_dmg(uarmg, "gloves", 2, FALSE)) continue;
			break;
		    case 4:
			if (!rust_dmg(uarmf, "boots", 2, FALSE)) continue;
			break;
		    }
		    break; /* Out of while loop */
		}
		break;
	    case AD_HEAL:
		if(!uwep
#ifdef SHIRT
		   && !uarmu
#endif
		   && !uarm && !uarmh && !uarms && !uarmg && !uarmc && !uarmf) {
		    kludge("%s hits! (I hope you don't mind.)", Monnam(mtmp));
#ifdef POLYSELF
			if (u.mtimedone) {
				u.mh += rnd(7);
				if(!rn2(7)) u.mhmax++;
				if(u.mh > u.mhmax) u.mh = u.mhmax;
				if(u.mh == u.mhmax && !rn2(50)) mongone(mtmp);
			} else {
#endif
				u.uhp += rnd(7);
				if(!rn2(7)) u.uhpmax++;
				if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
				if(u.uhp == u.uhpmax && !rn2(50)) mongone(mtmp);
#ifdef POLYSELF
			}
#endif
			flags.botl = 1;
			if(!rn2(50)) rloc(mtmp);
			dmg = 0;
		} else
		    if(pl_character[0] == 'H') {
			    if (flags.soundok && !(moves % 5))
				pline("'Doc, I can't help you unless you cooperate.'");
			    dmg = 0;
		    } else hitmsg(mtmp,mattk->aatyp);
		break;
	    case AD_CURS:
		hitmsg(mtmp,mattk->aatyp);
		if(!night() && mdat == &mons[PM_GREMLIN]) break;
		if(!mtmp->mcan && !rn2(10)) {
		    if (flags.soundok)
			if (Blind) You("hear laughter.");
			else       pline("%s chuckles.", Monnam(mtmp));
#ifdef POLYSELF
#ifdef GOLEMS
		    if (u.umonnum == PM_CLAY_GOLEM) {
			pline("Some writing vanishes from your head!");
			rehumanize();
			break;
		    }
#endif /* GOLEMS */
#endif
		    attrcurse();
		}
		break;
	    case AD_STUN:
		hitmsg(mtmp,mattk->aatyp);
		if(!mtmp->mcan && !rn2(4)) {
		    make_stunned(HStun + dmg, TRUE);
		    dmg /= 2;
		}
		break;
	    case AD_ACID:
		hitmsg(mtmp,mattk->aatyp);
		if(!mtmp->mcan && !rn2(3))
#ifdef POLYSELF
		    if (resists_acid(uasmon)) {
			pline("You're covered in acid, but it seems harmless.");
			dmg = 0;
		    } else
#endif
			pline("You're covered in acid!	It burns!");
		else		dmg = 0;
		break;
	    case AD_SLOW:
		hitmsg(mtmp,mattk->aatyp);
		if(!ctmp && (Fast & (INTRINSIC|TIMEOUT)) && !rn2(4)) {
		    Fast &= ~(INTRINSIC|TIMEOUT);
		    You("feel yourself slowing down.");
		}
		break;
	    case AD_DREN:
		hitmsg(mtmp,mattk->aatyp);
#ifdef SPELLS
		if(!ctmp && !rn2(4)) drain_en(dmg);
#endif
		dmg = 0;
		break;
	    case AD_CUSS:
		if(flags.soundok && !rn2(3)) cuss(mtmp);
		dmg = 0;
		break;
#ifdef HARD /* a non-gaze AD_CONF exists only for one of the demons */
	    case AD_CONF:
		hitmsg(mtmp,mattk->aatyp);
		if(!mtmp->mcan && !rn2(4) && !mtmp->mspec_used) {
		    mtmp->mspec_used += (dmg + rn2(6));
		    if(Confusion)
			 You("are getting even more confused.");
		    else You("are getting confused.");
		    make_confused(HConfusion + dmg, FALSE);
		}
#endif
		/* fall through to next case */
	    default:	dmg = 0;
			break;
	}
	if(u.uhp < 1) done_in_by(mtmp);

/*	Negative armor class reduces damage done instead of fully protecting
 *	against hits.
 */
	if (dmg && u.uac < 0) {
		dmg -= rnd(-u.uac);
		if (dmg < 1) dmg = 1;
	}

	if(dmg) mdamageu(mtmp, dmg);

#ifdef POLYSELF
	res = passiveum(olduasmon, mtmp);
	stop_occupation();
	return res;
#else
	stop_occupation();
	return 1;
#endif
}

static
int
gulpmu(mtmp, mattk)	/* monster swallows you, or damage if u.uswallow */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	int	tmp = d((int)mattk->damn, (int)mattk->damd);
	int	tim_tmp;
#ifdef WALKIES
	int	i;
#endif

	if(!u.uswallow) {	/* swallow him */
		levl[mtmp->mx][mtmp->my].mmask = 0;
		mtmp->mx = u.ux;
		mtmp->my = u.uy;
		levl[mtmp->mx][mtmp->my].mmask = 1;
		u.ustuck = mtmp;
		pmon(mtmp);
		kludge("%s engulfs you!", Monnam(mtmp));
#ifdef WALKIES
		if((i = number_leashed()) > 0) {
			pline("The leash%s snap%s loose...",
					(i > 1) ? "es" : "",
					(i > 1) ? "" : "s");
			unleash_all();
		}
#endif
#ifdef POLYSELF
		if (u.umonnum==PM_COCKATRICE) {
			kludge("%s turns to stone!", Monnam(mtmp));
			stoned = 1;
			xkilled(mtmp, 0);
			return 2;
		}
#endif
		more();
		seeoff(1);
		u.uswallow = 1;
		/*assume that u.uswldtim always set >=0*/
		u.uswldtim = (tim_tmp =
			(-u.uac + 10 + rnd(25 - (int)mtmp->m_lev)) >> 1) > 0 ?
			    tim_tmp : 0;
		swallowed(1);
	} else {

	    if(mtmp != u.ustuck) return(0);
	    switch(mattk->adtyp) {

		case AD_DGST:
		    if(!u.uswldtim) {	/* a3 *//*no cf unsigned <=0*/
			kludge("%s totally digests you!", Monnam(mtmp));
			tmp = u.uhp;
		    } else {
			kludge("%s digests you!", Monnam(mtmp));
		    }
		    break;
		case AD_PHYS:
		    You("are pummeled with debris!");
		    break;
		case AD_ACID:
#ifdef POLYSELF
		    if (resists_acid(uasmon)) {
			You("are covered with a seemingly harmless goo.");
			tmp = 0;
		    } else
#endif
		    if (Hallucination) pline("Ouch!  You've been slimed!");
		    else You("are covered in slime!  It burns!!!");
		    break;
		case AD_BLND:
		    if(!Blind) {
			You("can't see in here!");
			make_blinded((long)tmp,FALSE);
		    } else make_blinded(Blinded+1,FALSE);	/* keep him blind until disgorged */
		    tmp = 0;
		    break;
		case AD_ELEC:
		    if(!mtmp->mcan && rn2(2)) {
			pline("The air around you crackles with electricity.");
			if (Shock_resistance) {
				shieldeff(u.ux, u.uy);
				You("seem unhurt.");
#if defined(POLYSELF) && defined(GOLEMS)
				ugolemeffects(AD_ELEC,tmp);
#endif
				tmp = 0;
			}
		    } else tmp = 0;
		    break;
		case AD_COLD:
		    if(!mtmp->mcan && rn2(2)) {
			if (Cold_resistance) {
				shieldeff(u.ux, u.uy);
				You("feel mildly chilly.");
#if defined(POLYSELF) && defined(GOLEMS)
				ugolemeffects(AD_COLD,tmp);
#endif
				tmp = 0;
			} else You("are freezing to death!");
		    } else tmp = 0;
		    break;
		case AD_FIRE:
		    if(!mtmp->mcan && rn2(2)) {
			if (Fire_resistance) {
				shieldeff(u.ux, u.uy);
				You("feel mildly hot.");
#if defined(POLYSELF) && defined(GOLEMS)
				ugolemeffects(AD_FIRE,tmp);
#endif
				tmp = 0;
			} else You("are burning to a crisp!");
		    } else tmp = 0;
		    break;
#ifdef HARD
		case AD_DISE:
		    if (!Sick) You("feel very sick.");
		    make_sick(Sick + (long)rn1(25-ACURR(A_CON),15),FALSE);
		    u.usick_cause = mtmp->data->mname;
		    break;
#endif
		default:	tmp = 0;
				break;
	    }
	}

	mdamageu(mtmp, tmp);
	if(u.uswldtim) --u.uswldtim;
	if(!u.uswldtim
#ifdef POLYSELF
	    || u.umonnum==PM_COCKATRICE
#endif
	    ) {
#ifdef POLYSELF
	    if (u.umonnum == PM_COCKATRICE) {
		kludge("%s very hurriedly regurgitates you!",
		       Monnam(mtmp));
		u.uswldtim = 0;
	    } else {
#endif
		You("get regurgitated!");
		if(flags.verbose)
		    kludge("Obviously %s doesn't like your taste.",
							mon_nam(mtmp));
#ifdef POLYSELF
	    }
#endif
	    regurgitates(mtmp);
	}
	return(1);
}

static
int
explmu(mtmp, mattk)	/* monster explodes in your face */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	register int tmp = d((int)mattk->damn, (int)mattk->damd);

	if(mtmp->mcan) return(0);
	if(!Blind)	kludge("%s explodes!", Monnam(mtmp));
	else		pline("It explodes!");

	switch(mattk->adtyp) {
	    case AD_COLD:
		if(Cold_resistance) {
			You("seem unaffected by it.");
#if defined(POLYSELF) && defined(GOLEMS)
			ugolemeffects(AD_COLD,tmp);
#endif
			tmp = 0;
		} else {
			if(ACURR(A_DEX) > rnd(20)) {
				if (!flags.verbose)
				    You("are caught in the blast!");
			} else {
				You("duck the blast...");
				tmp /= 2;
			}
		}
		break;
	    case AD_BLND:
		if(!Blind
#ifdef POLYSELF
			&& u.usym != S_YLIGHT
#endif
			) {
			You("are blinded by a blast of light!");
			make_blinded((long)tmp,FALSE);
		}
		tmp = 0;
		break;
	    default: break;
	}
	mdamageu(mtmp, tmp);
	mondead(mtmp);
	return(2);	/* it dies */
}

static
int
gazemu(mtmp, mattk)	/* monster gazes at you */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	switch(mattk->adtyp) {
#ifdef MEDUSA
	    case AD_STON:
		if (mtmp->mcan) {
		    You("notice that %s isn't all that ugly.",mon_nam(mtmp));
		   break;
		}
		if (canseemon(mtmp)) {
			You("look upon %s.", mon_nam(mtmp));
#ifdef POLYSELF
			if (resists_ston(uasmon)) {
				pline("So what?");
				break;
			}
#endif
			You("turn to stone...");
			killer = mons[PM_MEDUSA].mname;
			done("stoned");
	    	}
		break;
#endif
	    case AD_CONF:
		if(!mtmp->mcan && canseemon(mtmp) && mtmp->mcansee && 
					!mtmp->mspec_used && rn2(5)) {
		    int conf = d(3,4);

		    mtmp->mspec_used += (conf + rn2(6));
		    if(!Confusion)
			pline("%s's gaze confuses you!", Monnam(mtmp));
		    else
			You("are getting more and more confused.");
		    make_confused(HConfusion + conf, FALSE);
		}
		break;
	    default: impossible("Gaze attack %d?", mattk->adtyp);
		break;
	}
	return(1);
}

void
mdamageu(mtmp, n)	/* mtmp hits you for n points damage */
	register struct monst *mtmp;
	register int n;
{
#ifdef POLYSELF
	if (u.mtimedone) {
		u.mh -= n;
		flags.botl = 1;
		if (u.mh < 1) rehumanize();
		return;
	}
#endif
	u.uhp -= n;
	flags.botl = 1;
	if(u.uhp < 1)
		done_in_by(mtmp);
}

#ifdef POLYSELF
static void
urustm(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	boolean vis = cansee(mon->mx, mon->my);

	if (!mon || !obj) return; /* just in case */
	if (u.umonnum == PM_RUST_MONSTER &&
				objects[obj->otyp].oc_material == METAL &&
				obj->spe > -2) {
		if(obj->rustfree) {
		    if (vis)
			pline("The rust vanishes from %s's weapon!",
								mon_nam(mon));
		} else if(obj->blessed && rn2(3)) {
		    if (vis)
			pline("Somehow, %s's weapon is not affected!",
								mon_nam(mon));
		} else {
		    if (vis)
			pline("%s's %s!", Monnam(mon), aobjnam(obj,"corrode"));
		    obj->spe--;
		}
	}
}
#endif

#ifdef SEDUCE
void
doseduce(mon)
register struct monst *mon;
{
	register struct obj *ring;
	boolean fem = (mon->data == &mons[PM_SUCCUBUS]); /* otherwise incubus */

	if (fem == poly_gender() || poly_gender()==2) return;

	if (mon->mcan || mon->mspec_used) {
  		pline("%s acts as though %s has got a %sheadache.",
  			Blind ? "It" : Monnam(mon), Blind ? "it" :
  			fem ? "she" : "he",
			mon->mcan ? "severe " : "");
		return;
	}

	if (unconscious()) {
		kludge("%s seems dismayed at your lack of response.",
			Monnam(mon));
		return;
	}

	if (Blind) pline("It caresses you...");
	else You("feel very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = ring->nobj) {
	    if (ring->otyp != RIN_ADORNMENT) continue;
	    if (fem) {
		if (rn2(20) < ACURR(A_CHA)) {
		    pline("\"That %s looks pretty.  May I have it?\" ",
			xname(ring));
		    makeknown(RIN_ADORNMENT);
		    if (yn() == 'n') continue;
		} else pline("%s decides she'd like your %s, and takes it.",
			Blind ? "She" : Monnam(mon), xname(ring));
		if (ring->known) makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		freeinv(ring);
		mpickobj(mon,ring);
	    } else {
		char buf[BUFSZ];

		if (uleft && uright && uleft->otyp == RIN_ADORNMENT
				&& uright->otyp==RIN_ADORNMENT)
			break;
		if (ring==uleft || ring==uright) continue;
		if (rn2(20) < ACURR(A_CHA)) {
		    pline("\"That %s looks pretty.  Would you wear it for me?\" ",
			xname(ring));
		    makeknown(RIN_ADORNMENT);
		    if (yn() == 'n') continue;
		} else {
		    pline("%s decides you'd look prettier wearing your %s,",
			Blind ? "He" : Monnam(mon), xname(ring));
		    pline("and puts it on your finger.");
		}
		makeknown(RIN_ADORNMENT);
		if (!uright) {
		    pline("%s puts the %s on your right hand.",
			Blind ? "He" : Monnam(mon), xname(ring));
		    setworn(ring, RIGHT_RING);
		} else if (!uleft) {
		    pline("%s puts the %s on your left hand.",
			Blind ? "He" : Monnam(mon), xname(ring));
		    setworn(ring, LEFT_RING);
		} else if (uright && uright->otyp != RIN_ADORNMENT) {
		    Strcpy(buf, xname(uright));
		    pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
		    Ring_gone(uright);
		    setworn(ring, RIGHT_RING);
		} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
		    Strcpy(buf, xname(uleft));
		    pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
		    Ring_gone(uleft);
		    setworn(ring, LEFT_RING);
		} else impossible("ring replacement");
		Ring_on(ring);
	    	prinv(ring);
	    }
	}

	pline("%s murmurs in your ear, while helping you undress.",
		Blind ? (fem ? "She" : "He") : Monnam(mon));
	mayberem(uarmc, "cloak");
	if(!uarmc)
		mayberem(uarm, "suit");
	mayberem(uarmf, "boots");
	mayberem(uarmg, "gloves");
	mayberem(uarms, "shield");
#ifdef SHIRT
	if(!uarmc && !uarm)
		mayberem(uarmu, "shirt");
#endif

	if (uarm || uarmc) {
		pline("\"You're such a %s; I wish...\"",
				flags.female ? "sweet lady" : "nice guy");
		rloc(mon);
		return;
	}
#define ALIGNLIM 	(5L + (moves/200L)) /* from pray.c */
	if (u.ualigntyp == U_CHAOTIC && u.ualign < ALIGNLIM) u.ualign++;

	/* by this point you have discovered mon's identity, blind or not... */
	pline("Time stands still while you and %s lie in each other's arms...",
		mon_nam(mon));
	if (rn2(35) > ACURR(A_CHA) + ACURR(A_INT)) {
		/* Don't bother with mspec_used here... it didn't get tired! */
		pline("%s seems to have enjoyed it more than you...",
			Monnam(mon));
#ifdef SPELLS
		switch (rn2(5)) {
			case 0: You("feel drained of energy.");
				u.uen = 0;
				u.uenmax -= rnd(10);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
#else
		switch (rnd(4)) {
#endif
			case 1: You("are down in the dumps.");
				adjattrib(A_CON, -1, TRUE);
				flags.botl = 1;
				break;
			case 2: Your("senses are dulled.");
				adjattrib(A_WIS, -1, TRUE);
				flags.botl = 1;
				break;
			case 3:
#ifdef POLYSELF
				if (resists_drli(uasmon))
				    You("have a curious feeling...");
				else {
#endif
				    You("feel out of shape.");
				    losexp();
#ifdef POLYSELF
				}
#endif
				break;
			case 4: You("feel exhausted.");
				losehp(5+rnd(10), "bout of exhaustion");
				break;
		}
	} else {
		mon->mspec_used = rnd(100); /* monster is worn out */
		You("seem to have enjoyed it more than %s...", mon_nam(mon));
#ifdef SPELLS
		switch (rn2(5)) {
			case 0: You("feel raised to your full potential.");
				u.uen = (u.uenmax += rnd(5));
				break;
#else
		switch (rnd(4)) {
#endif
			case 1: You("feel good enough to do it again.");
				adjattrib(A_CON, 1, TRUE);
				flags.botl = 1;
				break;
			case 2: You("will always remember %s...", mon_nam(mon));
				adjattrib(A_WIS, 1, TRUE);
				flags.botl = 1;
				break;
			case 3: pline("That was a very educational experience.");
				pluslvl();
				break;
			case 4: You("feel restored to health!");
				u.uhp = u.uhpmax;
#ifdef POLYSELF
				if (u.mtimedone) u.mh = u.mhmax;
#endif
				flags.botl = 1;
				break;
		}
	}

	if (mon->mtame) /* don't charge */ ;
	else if (rn2(20) < ACURR(A_CHA)) {
		pline("%s demands that you pay %s, but you refuse...",
			Monnam(mon), (fem ? "her" : "him"));
	}
#ifdef POLYSELF
	else if (u.umonnum == PM_LEPRECHAUN)
		pline("%s tries to take your money, but fails...",
				Monnam(mon));
#endif
	else {
		long cost = (long)rnd(
			(int)(u.ugold > 32767L ? 32767 : u.ugold) +10) + 500;

		if (mon->mpeaceful) {
			cost /= 5;
			if (!cost) cost=1;
		}
		if (cost > u.ugold) cost = u.ugold;
		if (!cost) pline("%s says: \"It's on the house!\"", Monnam(mon));
		else {
		    pline("%s takes %ld Zorkmid%s for services rendered!",
			    Monnam(mon), cost, (cost==1) ? "" : "s");
		    u.ugold -= cost;
		    mon->mgold += cost;
		    flags.botl = 1;
		}
	}
	if (!rn2(25)) mon->mcan = 1; /* monster is worn out */
	rloc(mon);
}

static void
mayberem(obj, str)
register struct obj *obj;
char *str;
{
	if (!obj || !obj->owornmask) return;

	if (rn2(20) < ACURR(A_CHA)) {
		pline("\"Shall I remove your %s, %s?\" ",
			str,
			(!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
		if (yn() == 'n') return;
	} else pline("\"Take off your %s; %s.\"", str,
			(obj == uarm)  ? "let's get a little closer" :
			(obj == uarmc) ? "it's in the way" :
			(obj == uarmf) ? "let me rub your feet" :
			(obj == uarmg) ? "they're too clumsy" :
#ifdef SHIRT
			(obj == uarmu) ? "let me massage you" :
#endif
			/* obj == uarmh */
			"so I can run my fingers through your hair");

	if (obj == uarm)  (void) Armor_off();
	else if (obj == uarmc) (void) Cloak_off();
	else if (obj == uarmf) (void) Boots_off();
	else if (obj == uarmg) (void) Gloves_off();
	else if (obj == uarmh) (void) Helmet_off();
	else setworn((struct obj *)0, obj->owornmask & W_ARMOR);
}
#endif  /* SEDUCE */

#ifdef POLYSELF
static int
passiveum(olduasmon,mtmp)
struct permonst *olduasmon;
register struct monst *mtmp;
{
	register struct permonst *mdat = mtmp->data;
	int dam = 0;

	if (olduasmon->mattk[0].aatyp != AT_NONE) return 1;

	/* These affect the enemy even if you were "killed" (rehumanized) */
	switch(olduasmon->mattk[0].adtyp) {
	    case AD_ACID:
		if (!rn2(2)) {
		    kludge("%s is splashed by your acid!", Monnam(mtmp));
		    if(!resists_acid(mdat))
			dam = d((int)olduasmon->mlevel+1,
					(int)olduasmon->mattk[0].damd);
		    else pline("%s is not affected.", Monnam(mtmp));
		}
		break;
	    default:
		break;
	}
	if (!u.mtimedone) return 1;

	/* These affect the enemy only if you are still a monster */
	if (rn2(3)) switch(uasmon->mattk[0].adtyp) {
	    case AD_PLYS: /* Floating eye */
		if (!mtmp->mblinded && rn2(3)) {
		    if (Blind)
			pline("As a blind %s, you cannot defend yourself.",
							uasmon->mname);
		    else {
			pline("%s is frozen by your gaze!", Monnam(mtmp));
			mtmp->mfroz = 1;
		    }
		}
		break;
	    case AD_COLD: /* Brown mold or blue jelly */
		dam = d((int)uasmon->mlevel+1, (int)uasmon->mattk[0].damd);
		if(resists_cold(mdat)) {
  		    shieldeff(mtmp->mx, mtmp->my);
		    kludge("%s is mildly chilly.", Monnam(mtmp));
#ifdef GOLEMS
		    golemeffects(mtmp, AD_COLD, dam);
#endif /* GOLEMS */
		    dam = 0;
		    break;
		}
		kludge("%s is suddenly very cold!", Monnam(mtmp));
		u.mh += dam / 2;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		if (u.mhmax > ((uasmon->mlevel+1) * 8)) {
			register struct monst *mon;

			if (mon = cloneu()) {
			    mon->mhpmax = u.mhmax /= 2;
			    if (Blind)
				You("multiply from its heat!");
			    else
				You("multiply from %s's heat!", mon_nam(mtmp));
			}
		}
		break;
	    case AD_STUN: /* Yellow mold */
		if (!mtmp->mstun) {
		    mtmp->mstun = 1;
		    kludge("%s staggers...", Monnam(mtmp));
		}
		break;
	    case AD_FIRE: /* Red mold */
		dam = d((int)uasmon->mlevel+1, (int)uasmon->mattk[0].damd);
		if(resists_fire(mdat)) {
  		    shieldeff(mtmp->mx, mtmp->my);
		    kludge("%s is mildly warm.", Monnam(mtmp));
#ifdef GOLEMS
		    golemeffects(mtmp, AD_FIRE, dam);
#endif /* GOLEMS */
		    dam = 0;
		    break;
		}
		kludge("%s is suddenly very hot!", Monnam(mtmp));
	}

	if((mtmp->mhp -= dam) <= 0) {
		kludge("%s dies!", Monnam(mtmp));
		xkilled(mtmp,0);
		return(2);
	}
	return 1;
}

#include "edog.h"
struct monst *
cloneu()
{
	register struct monst *mon;
	register int nmlth = strlen(plname) + 1;

	if (u.mh <= 1) return(struct monst *)0;
	uasmon->pxlth += sizeof(struct edog) + nmlth;
	mon = makemon(uasmon, u.ux, u.uy);
	uasmon->pxlth -= sizeof(struct edog) + nmlth;
	mon->mxlth = sizeof(struct edog);
	Strcpy(NAME(mon), plname);
	mon->mnamelth = nmlth;
	initedog(mon);
	mon->m_lev = uasmon->mlevel;
	mon->mhp = u.mh /= 2;
	mon->mhpmax = u.mhmax;
	return(mon);
}
#endif /* POLYSELF */
