/*	SCCS Id: @(#)mhitu.c	3.1	93/03/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

STATIC_VAR NEARDATA struct obj *otmp;

#ifdef POLYSELF
STATIC_DCL void FDECL(urustm, (struct monst *, struct obj *));
# ifdef OVL1
static int FDECL(passiveum, (struct permonst *,struct monst *,struct attack *));
# endif /* OVL1 */
#endif /* POLYSELF */

#ifdef OVLB
# ifdef SEDUCE
static void FDECL(mayberem, (struct obj *, const char *));
# endif
#endif /* OVLB */

STATIC_DCL boolean FDECL(diseasemu, (struct permonst *));
STATIC_DCL int FDECL(hitmu, (struct monst *,struct attack *));
STATIC_DCL int FDECL(gulpmu, (struct monst *,struct attack *));
STATIC_DCL int FDECL(explmu, (struct monst *,struct attack *,BOOLEAN_P));
STATIC_DCL void FDECL(missmu,(struct monst *,BOOLEAN_P,struct attack *));
STATIC_DCL void FDECL(mswings,(struct monst *,struct obj *));
STATIC_DCL void FDECL(wildmiss,(struct monst *));

STATIC_DCL void FDECL(hurtarmor,(struct permonst *,int));
STATIC_DCL void FDECL(hitmsg,(struct monst *,struct attack *));

/* See comment in mhitm.c.  If we use this a lot it probably should be */
/* changed to a parameter to mhitu. */
static int dieroll;

#ifdef OVL1


STATIC_OVL void
hitmsg(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
	int compat;

	/* Note: if opposite gender, "seductively" */
	/* If same gender, "engagingly" for nymph, normal msg for others */
	if((compat = could_seduce(mtmp, &youmonst, mattk))
			&& !mtmp->mcan && !mtmp->mspec_used) {
	    	pline("%s %s you %s.", Monnam(mtmp),
			Blind ? "talks to" : "smiles at",
			compat == 2 ? "engagingly" : "seductively");
	} else
	    switch (mattk->aatyp) {
		case AT_BITE:
			pline("%s bites!", Monnam(mtmp));
			break;
		case AT_KICK:
#ifdef POLYSELF
			pline("%s kicks%c", Monnam(mtmp), thick_skinned(uasmon) ? '.' : '!');
#else
			pline("%s kicks!", Monnam(mtmp));
#endif
			break;
		case AT_STNG:
			pline("%s stings!", Monnam(mtmp));
			break;
		case AT_BUTT:
			pline("%s butts!", Monnam(mtmp));
			break;
		case AT_TUCH:
			pline("%s touches you!", Monnam(mtmp));
			break;
		case AT_TENT:
			pline("%s tentacles suck you!", 
				        s_suffix(Monnam(mtmp)));
			break;
		case AT_EXPL:
			pline("%s explodes!", Monnam(mtmp));
			break;
		default:
			pline("%s hits!", Monnam(mtmp));
	    }
}

STATIC_OVL void
missmu(mtmp, nearmiss, mattk)		/* monster missed you */
register struct monst *mtmp;
register boolean nearmiss;
register struct attack *mattk;
{
	if(could_seduce(mtmp, &youmonst, mattk) && !mtmp->mcan)
	    pline("%s pretends to be friendly.", Monnam(mtmp));
	else {
	    if (!flags.verbose || !nearmiss)
		pline("%s misses.", Monnam(mtmp));
	    else
		pline("%s just misses!", Monnam(mtmp));
	}
}

STATIC_OVL void
mswings(mtmp, otemp)		/* monster swings obj */
register struct monst *mtmp;
register struct obj *otemp;
{
	if (!flags.verbose || Blind || !mon_visible(mtmp))
		return;
	pline("%s %s %s %s.", Monnam(mtmp),
	      ((otemp->otyp >= SPEAR && otemp->otyp <= LANCE) ||
	       (otemp->otyp >= PARTISAN && otemp->otyp <= SPETUM) ||
	       otemp->otyp == TRIDENT) ? "thrusts" : "swings",
	      his[pronoun_gender(mtmp)], xname(otemp));
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL void
wildmiss(mtmp)		/* monster attacked your displaced image */
	register struct monst *mtmp;
{
	int compat;

	if (!flags.verbose) return;
	if (!cansee(mtmp->mx, mtmp->my)) return;
		/* maybe it's attacking an image around the corner? */

	compat = could_seduce(mtmp, &youmonst, (struct attack *)0);
		/* we really want to have the attack here to pass --
		 * the previous code checked whether mtmp was a nymph,
		 * which was not correct either since the attack type of
		 * succubi/incubi varies with SEDUCE
		 */

	if(Invis && !perceives(mtmp->data)) {
	    if(compat)
		pline("%s tries to touch you and misses!", Monnam(mtmp));
	    else
		switch(rn2(3)) {
		case 0: pline("%s %s wildly and misses!", Monnam(mtmp),
			      nolimbs(mtmp->data) ? "lunges" : "swings");
		    break;
		case 1: pline("%s attacks a spot beside you.", Monnam(mtmp));
		    break;
		case 2: pline("%s strikes at %s!", Monnam(mtmp),
				Underwater ? "empty water" : "thin air");
		    break;
		default:pline("%s %s wildly!", Monnam(mtmp),
			      nolimbs(mtmp->data) ? "lunges" : "swings");
		    break;
		}
	}
	else if(Displaced) {
	    if(compat)
		pline("%s smiles %s at your %sdisplaced image...",
			Monnam(mtmp),
			compat == 2 ? "engagingly" : "seductively",
			Invis ? "invisible " : "");
	    else
		pline("%s strikes at your %sdisplaced image and misses you!",
			/* Note: if you're both invisible and displaced,
			 * only monsters which see invisible will attack your
			 * displaced image, since the displaced image is also
			 * invisible.
			 */
			Monnam(mtmp),
			Invis ? "invisible " : "");
	}
	/* monsters may miss especially on water level where
	   bubbles shake the player here and there */
	else if(Underwater) {
	    if(compat)
		pline("%s reaches towards your distorted image.",Monnam(mtmp));
	    else
		pline("%s is fooled by water reflections and misses!",Monnam(mtmp));
	}
	else impossible("%s attacks you without knowing your location?",
		Monnam(mtmp));
}

void
expels(mtmp, mdat, message)
register struct monst *mtmp;
register struct permonst *mdat; /* if mtmp is polymorphed, mdat != mtmp->data */
boolean message;
{
	if (message) 
		if (is_animal(mdat)) 
			You("get regurgitated!");
		else {
			char blast[40];
			register int i;

			blast[0] = '\0';
			for(i = 0; i < NATTK; i++)
				if(mdat->mattk[i].aatyp == AT_ENGL) 
					break;
			if (mdat->mattk[i].aatyp != AT_ENGL)
			      impossible("Swallower has no engulfing attack?"); 
			else {
				if (is_whirly(mdat)) {
					switch (mdat->mattk[i].adtyp) {
						case AD_ELEC:
							Strcpy(blast, 
						      " in a shower of sparks");
							break;
						case AD_COLD:
							Strcpy(blast, 
							" in a blast of frost");
							break;
					}
				} else
					Strcpy(blast, " with a squelch");
				You("get expelled from %s%s!", 
				    mon_nam(mtmp), blast);
			}
		}
	unstuck(mtmp);	/* ball&chain returned in unstuck() */
	mnexto(mtmp);
	newsym(u.ux,u.uy);
	spoteffects();
	/* to cover for a case where mtmp is not in a next square */
	if(um_dist(mtmp->mx,mtmp->my,1))
		pline("Brrooaa...  You land hard at some distance.");
}

#endif /* OVLB */
#ifdef OVL0

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
	boolean ranged = (distu(mtmp->mx, mtmp->my) > 3);
		/* Is it near you?  Affects your actions */
	boolean range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
		/* Does it think it's near you?  Affects its actions */
	boolean foundyou = (mtmp->mux==u.ux && mtmp->muy==u.uy);
		/* Is it attacking you or your image? */
	boolean youseeit = canseemon(mtmp);
		/* Might be attacking your image around the corner, or
		 * invisible, or you might be blind....
		 */

	if(!ranged) nomul(0);
	if(mtmp->mhp <= 0 || (Underwater && !is_swimmer(mtmp->data)))
	    return(0);

	/* If swallowed, can only be affected by u.ustuck */
	if(u.uswallow) {
	    if(mtmp != u.ustuck)
		return(0);
	    u.ustuck->mux = u.ux;
	    u.ustuck->muy = u.uy;
	    range2 = 0;
	    foundyou = 1;
	    if(u.uinvulnerable) return (0); /* stomachs can't hurt you! */
	    /* This is not impossible! */
	    /* If the swallowing monster changes into a monster
	     * that is not capable of swallowing you, you get
	     * regurgitated - dgk
	     *
	     * This code is obsolete: newcham() will handle this contingency 
	     * as soon as it occurs in the course of a round. - kcd
	     *
	     * for(i = 0; i < NATTK; i++)
	     *     if(mdat->mattk[i].aatyp == AT_ENGL) goto doattack;
	     *
	     * You("get regurgitated!");
	     * regurgitates(mtmp);
             * return(0);
	     */
	}
/* doattack:		use commented out above */
#ifdef POLYSELF
	if (u.uundetected && !range2 && foundyou && !u.uswallow) {
		u.uundetected = 0;
		if (is_hider(uasmon)) {
		    coord cc; /* maybe we need a unexto() function? */

		    You("fall from the ceiling!");
		    if (enexto(&cc, u.ux, u.uy, &playermon)) {
			remove_monster(mtmp->mx, mtmp->my);
			newsym(mtmp->mx,mtmp->my);
			place_monster(mtmp, u.ux, u.uy);
			if(mtmp->wormno) worm_move(mtmp);
			teleds(cc.x, cc.y);
			set_apparxy(mtmp);
			newsym(u.ux,u.uy);
		    } else {
			pline("%s is killed by a falling %s (you)!",
						Monnam(mtmp), uasmon->mname);
			killed(mtmp);
			newsym(u.ux,u.uy);
#ifdef MUSE
			if (mtmp->mhp > 0) return(0);
			else
#endif
				return(1);
		    }
		    if (u.usym != S_PIERCER)
			return(0);	/* trappers don't attack */
#ifdef MUSE
		    if (which_armor(mtmp, WORN_HELMET)) {
#else
		    if (is_mercenary(mtmp->data) && m_carrying(mtmp,HELMET)) {
#endif
			Your("blow glances off %s helmet.", 
			               s_suffix(mon_nam(mtmp)));
		    } else {
			if (3 + find_mac(mtmp) <= rnd(20)) {
			    pline("%s is hit by a falling piercer (you)!",
								Monnam(mtmp));
			    if ((mtmp->mhp -= d(3,6)) < 1)
				killed(mtmp);
			} else
			  pline("%s is almost hit by a falling piercer (you)!",
								Monnam(mtmp));
		    }
		} else {
		    if (!youseeit)
			pline("It tries to move where you are hiding.");
		    else {
			/* Ugly kludge for eggs.  The message is phrased so as
			 * to be directed at the monster, not the player,
			 * which makes "laid by you" wrong.  For the
			 * parallelism to work, we can't rephrase it, so we
			 * zap the "laid by you" momentarily instead.
			 */
			struct obj *obj = level.objects[u.ux][u.uy];

			if (obj) {
			    int save_spe = obj->spe;
			    if (obj->otyp == EGG) obj->spe = 0;
	     pline("Wait, %s!  There's a %s named %s hiding under %s!",
				mtmp->mnamelth ? (const char *)NAME(mtmp)
					       : mtmp->data->mname,
				uasmon->mname, plname,
				doname(level.objects[u.ux][u.uy]));
			    obj->spe = save_spe;
			} else
			    impossible("hiding under nothing?");
		    }
		    newsym(u.ux,u.uy);
		}
		return(0);
	}
	if (u.usym == S_MIMIC_DEF && !range2 && foundyou && !u.uswallow) {
		if (!youseeit) pline("It gets stuck on you.");
		    else pline("Wait, %s!  That's a %s named %s!",
			mtmp->mnamelth ? (const char *)NAME(mtmp) : mtmp->data->mname,
			uasmon->mname, plname);
		u.ustuck = mtmp;
		u.usym = S_MIMIC;
		newsym(u.ux,u.uy);
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
		newsym(mtmp->mx,mtmp->my);
	}

/*	Special demon handling code */
	if(!mtmp->cham && is_demon(mdat) && !range2
	   && mtmp->data != &mons[PM_BALROG]
	   && mtmp->data != &mons[PM_SUCCUBUS]
	   && mtmp->data != &mons[PM_INCUBUS])
	    if(!mtmp->mcan && !rn2(13))	msummon(mdat);

/*	Special lycanthrope handling code */
	if(!mtmp->cham && is_were(mdat) && !range2) {

	    if(is_human(mdat)) {
		if(!rn2(5 - (night() * 2)) && !mtmp->mcan) new_were(mtmp);
	    } else if(!rn2(30) && !mtmp->mcan) new_were(mtmp);

	    if(!rn2(10) && !mtmp->mcan) {
		if(youseeit) {
			pline("%s summons help!", Monnam(mtmp));
		} else
			You("feel hemmed in.");
		/* Technically wrong; we really should check if you can see the
		 * help, but close enough...
		 */
		if (!were_summon(mdat,FALSE) && youseeit)
		    pline("But none comes.");
	    }
	}

	if(u.uinvulnerable) {
	    /* monster's won't attack you */
	    if(mtmp == u.ustuck)
		pline("%s loosens its grip slightly.", Monnam(mtmp));
	    else if(!range2) {
		if(youseeit)
		    pline("%s starts to attack you, but pulls back.",
			  Monnam(mtmp));
		else
		    You("feel something move nearby.");
	    }
	    return (0);
	}

#ifdef MUSE
	/* Unlike defensive stuff, don't let them use item _and_ attack. */
	/* Exception:  Medusa; her gaze is automatic.  (We actually kludge
	 * by permitting a full attack sequence, not just a gaze attack.)
	 */
	if(find_offensive(mtmp)) {
		int foo = use_offensive(mtmp);

		if (mtmp->data != &mons[PM_MEDUSA] && foo != 0) return(foo==1);
	}
#endif

	for(i = 0; i < NATTK; i++) {

	    sum[i] = 0;
	    mattk = &(mdat->mattk[i]);
	    if (u.uswallow && (mattk->aatyp != AT_ENGL))
		continue;
	    switch(mattk->aatyp) {
		case AT_CLAW:	/* "hand to hand" attacks */
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
		case AT_TENT:
			if(!range2) {
			    if (foundyou) {
				if(tmp > (j = rnd(20+i))) {
#ifdef POLYSELF
				    if (mattk->aatyp != AT_KICK ||
					    !thick_skinned(uasmon))
#endif
					sum[i] = hitmu(mtmp, mattk);
				} else
				    missmu(mtmp, (tmp == j), mattk);
			    } else
				wildmiss(mtmp);
			}
			break;

		case AT_HUGS:	/* automatic if prev two attacks succeed */
			/* Note: if displaced, prev attacks never succeeded */
			if((!range2 && i>=2 && sum[i-1] && sum[i-2])
							|| mtmp == u.ustuck)
				sum[i]= hitmu(mtmp, mattk);
			break;

		case AT_GAZE:	/* can affect you either ranged or not */
			if (youseeit)
			    /* not displaced around a corner so not visible */
			    sum[i] = gazemu(mtmp, mattk);
			/* if gazemu returns, the player isn't dead.
			 * can't put this in gazemu() because youseeit might
			 * not be set
			 */
			if(Reflecting && m_canseeu(mtmp) &&
			   !mtmp->mcan && mtmp->data == &mons[PM_MEDUSA]) {
			    if(!Blind) {
				if(Reflecting & W_AMUL)
				    makeknown(AMULET_OF_REFLECTION);
				else
				    makeknown(SHIELD_OF_REFLECTION);
				pline("%s gaze is reflected by your %s.",
				      s_suffix(Monnam(mtmp)),
				      (Reflecting & W_AMUL) ?
				      "medallion" : "shield");
				pline("%s is turned to stone!", Monnam(mtmp));
			    }
			    stoned = TRUE;
			    killed(mtmp);
#ifdef MUSE
			    if (mtmp->mhp > 0)
				sum[i] = 0;
			    else
#endif
				sum[i] = 2;
			}
			break;

		case AT_EXPL:	/* automatic hit if next to, and aimed at you */
			if(!range2) sum[i] = explmu(mtmp, mattk, foundyou);
			break;

		case AT_ENGL:
			if (!range2) {
			    if(foundyou) {
				if(u.uswallow || tmp > (j = rnd(20+i))) {
				    /* Force swallowing monster to be
				     * displayed even when player is
				     * moving away */
				    flush_screen(1);
				    sum[i] = gulpmu(mtmp, mattk);
				} else {
				    missmu(mtmp, (tmp == j), mattk);
				}
                           } else if (is_animal(mtmp->data))
					pline("%s gulps some air!", youseeit ?
					      Monnam(mtmp) : "It");
				  else
					if (youseeit)
					 pline("%s lunges forward and recoils!",
					       Monnam(mtmp));
					else
						You("hear a %s nearby.", 
						    is_whirly(mtmp->data)? 
						    "rushing noise" : 
						    "splat");
			}
			break;
		case AT_BREA:
			if(range2) sum[i] = breamu(mtmp, mattk);
			/* Note: breamu takes care of displacement */
			break;
		case AT_SPIT:
			if(range2) sum[i] = spitmu(mtmp, mattk);
			/* Note: spitmu takes care of displacement */
			break;
		case AT_WEAP:
			if(range2) {
#ifdef REINCARNATION
				if (!Is_rogue_level(&u.uz))
#endif
					thrwmu(mtmp);
			} else {
#ifdef MUSE
			    /* Rare but not impossible.  Normally the monster
			     * wields when 2 spaces away, but it can be
			     * teleported or whatever....
			     */
			    if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)) {
				mtmp->weapon_check = NEED_HTH_WEAPON;
				/* mon_wield_item resets weapon_check as appropriate */
				if (mon_wield_item(mtmp) != 0) break;
			    }
#endif
			    if (foundyou) {
				set_uasmon();
#ifdef MUSE
				remove_cadavers(&mtmp->minvent);
				possibly_unwield(mtmp);
				otmp = MON_WEP(mtmp);
#else
				otmp = select_hwep(mtmp);
#endif
				if(otmp) {
				    tmp += hitval(otmp, uasmon);
				    mswings(mtmp, otmp);
				}
				if(tmp > (j = dieroll = rnd(20+i)))
				    sum[i] = hitmu(mtmp, mattk);
				else
				    missmu(mtmp, (tmp == j), mattk);
			    } else
				wildmiss(mtmp);
			}
			break;
		case AT_MAGC:
			if (range2)
			    sum[i] = buzzmu(mtmp, mattk);
			else
			    if (foundyou)
				sum[i] = castmu(mtmp, mattk);
			    else
				pline("%s casts a spell at thin air!",
					youseeit ? Monnam(mtmp) : "It");
				/* Not totally right since castmu allows other
				 * spells, such as the monster healing itself,
				 * that should work even when not next to you--
				 * but the previous code was just as wrong.
				 * --KAA
				 */
			break;

		default:		/* no attack */
			break;
	    }
	    if(flags.botl) bot();
	/* give player a chance of waking up before dying -kaa */
	    if(sum[i] == 1) {	    /* successful attack */
		if (u.usleep && !rn2(10)) {
		    multi = -1;
		    nomovemsg = "The combat suddenly awakens you.";
		}
	    }
	    if(sum[i] == 2)  return(1); 	/* attacker dead */
	    if(sum[i] == 3) break;  /* attacker teleported, no more attacks */
	    /* sum[i] == 0: unsuccessful attack */
	}
	return(0);
}

#endif /* OVL0 */
#ifdef OVLB

/*
 * helper function for some compilers that have trouble with hitmu
 */

STATIC_OVL void
hurtarmor(mdat, attk)
struct permonst *mdat;
int attk;
{
	boolean getbronze, rusting;
	int	hurt;

	rusting = (attk == AD_RUST);
	if (rusting) {
		hurt = 1;
		getbronze = (mdat == &mons[PM_BLACK_PUDDING] &&
			     uarm && is_corrodeable(uarm));
	}
	else {
		hurt=2;
		getbronze = FALSE;
	}
	/* What the following code does: it keeps looping until it
	 * finds a target for the rust monster.
	 * Head, feet, etc... not covered by metal, or covered by
	 * rusty metal, are not targets.  However, your body always
	 * is, no matter what covers it.
	 */
	while (1) {
	    switch(rn2(5)) {
	    case 0:
		if (!rust_dmg(uarmh, rusting ? "helmet" : "leather helmet",
					 hurt, FALSE))
			continue;
		break;
	    case 1:
		if (uarmc) {
		    if (!rusting)
			(void)rust_dmg(uarmc, "cloak", hurt, TRUE);
		    break;
		}
		/* Note the difference between break and continue;
		 * break means it was hit and didn't rust; continue
		 * means it wasn't a target and though it didn't rust
		 * something else did.
		 */
		if (getbronze)
		    (void)rust_dmg(uarm, "bronze armor", 3, TRUE);
		else if (uarm)
		    (void)rust_dmg(uarm, xname(uarm), hurt, TRUE);
		break;
	    case 2:
		if (!rust_dmg(uarms, rusting ? "shield" : "wooden shield",
					 hurt, FALSE))
			continue;
		break;
	    case 3:
		if (!rust_dmg(uarmg, rusting ? "metal gauntlets" : "gloves",
					 hurt, FALSE))
			continue;
		break;
	    case 4:
		if (!rust_dmg(uarmf, rusting ? "metal boots" : "boots",
					 hurt, FALSE))
			continue;
		break;
	    }
	    break; /* Out of while loop */
	}
}

#endif /* OVLB */
#ifdef OVL1

STATIC_OVL boolean
diseasemu(mdat)
struct permonst *mdat;
{
	if (defends(AD_DISE,uwep)
#ifdef POLYSELF
			|| u.usym == S_FUNGUS
#endif
						) {
		You("feel a slight illness.");
		return FALSE;
	} else {
		if (!Sick) You("feel very sick.");
		exercise(A_CON, FALSE);
		make_sick(Sick ? Sick/4 + 1L : (long)rn1(ACURR(A_CON), 20), FALSE);
		u.usick_cause = mdat->mname;
		return TRUE;
	}
}

/*
 * hitmu: monster hits you
 *	  returns 2 if monster dies (e.g. "yellow light"), 1 otherwise
 *	  3 if the monster lives but teleported/paralyzed, so it can't keep
 *	       attacking you
 */
STATIC_OVL int
hitmu(mtmp, mattk)
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	register struct permonst *mdat = mtmp->data;
	register int ctmp, ptmp;
	int dmg, armpro;
	char	 buf[BUFSZ];
#ifdef POLYSELF
	struct permonst *olduasmon = uasmon;
	int res;
#endif

/*	If the monster is undetected & hits you.  You should know where
 *	the attack came from.
 */
	if(mtmp->mundetected && hides_under(mdat)) {
	    mtmp->mundetected = 0;
	    if(!(Blind ? Telepat : (HTelepat & (WORN_HELMET|WORN_AMUL)))) {
		register struct obj *obj;

		if(OBJ_AT(mtmp->mx, mtmp->my)) {
		    if((obj = level.objects[mtmp->mx][mtmp->my]) != 0)
			pline("%s was hidden under %s!",
				  Amonnam(mtmp), doname(obj));
		}
		newsym(mtmp->mx, mtmp->my);
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
	armpro = 0;
	if (uarm && armpro < objects[uarm->otyp].a_can)
		armpro = objects[uarm->otyp].a_can;
	if (uarmc && armpro < objects[uarmc->otyp].a_can)
		armpro = objects[uarmc->otyp].a_can;
	ctmp = !mtmp->mcan && ((rn2(3) >= armpro) || !rn2(50));

/*	Now, adjust damages via resistances or specific attacks */
	switch(mattk->adtyp) {
	    case AD_PHYS:
		if(mattk->aatyp == AT_HUGS
#ifdef POLYSELF
					   && !sticks(uasmon)
#endif
								) {
		    if(!u.ustuck && rn2(2)) {
			register struct obj *obj = (uarmc ? uarmc : uarm);

			/* if your cloak/armor is greased, monster slips off */
			if (obj && obj->greased) {
			    dmg = 0;
			    pline("%s grabs you, but cannot hold onto your greased %s!",
				  Monnam(mtmp), xname(obj));
			    if (!rn2(2)) {
				pline("The grease wears off.");
				obj->greased = 0;
			    }
			} else {
			    u.ustuck = mtmp;
			    pline("%s grabs you!", Monnam(mtmp));
			}
		    } else if(u.ustuck == mtmp) {
		        exercise(A_STR, FALSE);
			You("are being %s.",
			      (mtmp->data == &mons[PM_ROPE_GOLEM])
			      ? "choked" : "crushed");
		    }
		} else {			  /* hand to hand weapon */
		    if(mattk->aatyp == AT_WEAP && otmp) {
#ifdef MUSE
			if (otmp->otyp == CORPSE
				&& otmp->corpsenm == PM_COCKATRICE) {
			    dmg = 1;
			    pline("%s hits you with the cockatrice corpse.",
				Monnam(mtmp));
			    if (!Stoned)
			        goto do_stone;
			}
#endif
			dmg += dmgval(otmp, uasmon);
			if (dmg <= 0) dmg = 1;
			if (!(otmp->oartifact &&
				artifact_hit(mtmp, &youmonst, otmp, &dmg,dieroll)))
			     hitmsg(mtmp, mattk);
#ifdef POLYSELF
			if (!dmg) break;
			if (u.mh > 1 && u.mh > ((u.uac>0) ? dmg : dmg+u.uac) &&
					(u.umonnum==PM_BLACK_PUDDING
					|| u.umonnum==PM_BROWN_PUDDING)) {
			    /* This redundancy necessary because you have to
			     * take the damage _before_ being cloned.
			     */
			    if (u.uac < 0) dmg += u.uac;
			    if (dmg < 1) dmg = 1;
			    if (dmg > 1) exercise(A_STR, FALSE);
			    u.mh -= dmg;
			    flags.botl = 1;
			    dmg = 0;
			    if(cloneu())
			    You("divide as %s hits you!",mon_nam(mtmp));
			}
			urustm(mtmp, otmp);
#endif
		    } else
			hitmsg(mtmp, mattk);
		}
		break;
	    case AD_DISE:
		hitmsg(mtmp, mattk);
		if (!diseasemu(mdat)) dmg = 0;
		break;
	    case AD_FIRE:
		hitmsg(mtmp, mattk);
		if(ctmp) {
		    pline("You're on fire!");
		    if (Fire_resistance) {
			pline("The fire doesn't feel hot!");
			dmg = 0;
		    }
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(SCROLL_CLASS, AD_FIRE);
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(POTION_CLASS, AD_FIRE);
		    if((int) mtmp->m_lev > rn2(25))
			destroy_item(SPBOOK_CLASS, AD_FIRE);
		} else dmg = 0;
		break;
	    case AD_COLD:
		hitmsg(mtmp, mattk);
		if(ctmp) {
		    pline("You're covered in frost!");
		    if (Cold_resistance) {
			pline("The frost doesn't seem cold!");
			dmg = 0;
		    }
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(POTION_CLASS, AD_COLD);
		} else dmg = 0;
		break;
	    case AD_ELEC:
		hitmsg(mtmp, mattk);
		if(ctmp) {
		    You("get zapped!");
		    if (Shock_resistance) {
			pline("The zap doesn't shock you!");
			dmg = 0;
		    }
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(WAND_CLASS, AD_ELEC);
		    if((int) mtmp->m_lev > rn2(20))
			destroy_item(RING_CLASS, AD_ELEC);
		} else dmg = 0;
		break;
	    case AD_SLEE:
		hitmsg(mtmp, mattk);
		if(ctmp && multi >= 0 && !rn2(5)) {
		    if (Sleep_resistance) break;
		    nomul(-rnd(10));
		    u.usleep = 1;
		    nomovemsg = "You wake up.";
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
		hitmsg(mtmp, mattk);
		if(ctmp && !rn2(8)) {
			Sprintf(buf, "%s %s",
				!(canseemon(mtmp) || sensemon(mtmp)) ? "Its" :
				Hallucination ? s_suffix(rndmonnam()) : 
				                s_suffix(mdat->mname),
				(mattk->aatyp == AT_BITE) ? "bite" : "sting");
			poisoned(buf, ptmp, mdat->mname, 30);
		}
		break;
	    case AD_DRIN:
		hitmsg(mtmp, mattk);
		if (defends(AD_DRIN, uwep)
#ifdef POLYSELF
					|| !has_head(uasmon)
#endif
								) {
		    You("don't seem harmed.");
		    break;
		}
		if (uarmh && rn2(8)) {
		    Your("helmet blocks the attack to your head.");
		    break;
		}
		if (Half_physical_damage) dmg = (dmg+1) / 2;
		losehp(dmg, mon_nam(mtmp), KILLED_BY_AN);
		Your("brain is eaten!");
		/* No such thing as mindless players... */
		if (ABASE(A_INT) <= ATTRMIN(A_INT)) {
		    int lifesaved = 0;
		    while(1) {
			if (lifesaved)
			    pline("Unfortunately your brain is still gone.");
			else
			    Your("last thought fades away.");
			killer = "brainlessness";
			killer_format = KILLED_BY;
			done(DIED);
			lifesaved = 1;
#ifdef WIZARD
			if (wizard) break;
#endif
		    }
		}
		(void) adjattrib(A_INT, -rnd(2), FALSE);
		exercise(A_WIS, FALSE);
		break;
	    case AD_PLYS:
		hitmsg(mtmp, mattk);
		if(ctmp && multi >= 0 && !rn2(3)) {
		    if (Blind)	You("are frozen!");
		    else	You("are frozen by %s!", mon_nam(mtmp));
		    nomul(-rnd(10));
		    exercise(A_DEX, FALSE);
		}
		break;
	    case AD_DRLI:
		hitmsg(mtmp, mattk);
		if (ctmp && !rn2(3)
#ifdef POLYSELF
		    && !resists_drli(uasmon)
#endif
		    && !defends(AD_DRLI, uwep)
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
		    exercise(A_STR, FALSE);
		    exercise(A_DEX, FALSE);
		  }
		  break;
		}
	    case AD_STON:	/* at present only a cockatrice */
		hitmsg(mtmp, mattk);
		if(!rn2(3) && !Stoned) {
		    if (mtmp->mcan) {
			if (flags.soundok)
			    You("hear a cough from %s!", mon_nam(mtmp));
		    } else {
			if (flags.soundok)
			    You("hear %s hissing!", s_suffix(mon_nam(mtmp)));
#ifdef MUSE
do_stone:
#endif
			if((!rn2(10) ||
			    (flags.moonphase == NEW_MOON && !have_lizard()))
#ifdef POLYSELF
			    && !resists_ston(uasmon)
			    && !(poly_when_stoned(uasmon) &&
				 polymon(PM_STONE_GOLEM))
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
		hitmsg(mtmp, mattk);
		if(ctmp && !u.ustuck
#ifdef POLYSELF
				     && !sticks(uasmon)
#endif
							) u.ustuck = mtmp;
		break;
	    case AD_WRAP:
		if((!mtmp->mcan || (u.ustuck == mtmp))
#ifdef POLYSELF
			&& !sticks(uasmon)
#endif
					  ) {
		    if(!u.ustuck && !rn2(10)) {
			register struct obj *obj = (uarmc ? uarmc : uarm);

			/* if your cloak/armor is greased, monster slips off */
			if (obj && obj->greased) {
			    dmg = 0;
			    pline("%s slips off of your greased %s!",
				  Monnam(mtmp), xname(obj));
			    if (!rn2(2)) {
				pline("The grease wears off.");
				obj->greased = 0;
			    }
			} else {
			    pline("%s swings itself around you!",
				  Monnam(mtmp));
			    u.ustuck = mtmp;
			}
		    } else if(u.ustuck == mtmp) {
			if (is_pool(mtmp->mx,mtmp->my)
#ifdef POLYSELF
			    && !is_swimmer(uasmon)
#endif
			    && !Amphibious
			   ) {
			    boolean moat = (levl[u.ux][u.uy].typ != POOL) &&
				(levl[u.ux][u.uy].typ != WATER) &&
				!Is_medusa_level(&u.uz) &&
				!Is_waterlevel(&u.uz);

			    pline("%s drowns you....", Monnam(mtmp));
			    killer_format = KILLED_BY_AN;
			    Sprintf(buf, "%s by %s",
				    moat ? "moat" : "pool of water",
				    a_monnam(mtmp));
			    killer = buf;
			    done(DROWNING);
			} else if(mattk->aatyp == AT_HUGS)
			    You("are being crushed.");
		    } else {
			dmg = 0;
			if(flags.verbose)
			    pline("%s brushes against your %s.", Monnam(mtmp),
				   body_part(LEG));
		    }
		} else dmg = 0;
		break;
	    case AD_WERE:
		hitmsg(mtmp, mattk);
#ifdef POLYSELF
		if (ctmp && !rn2(4) && u.ulycn == -1
		    && !Protection_from_shape_changers
		    && !defends(AD_WERE,uwep)) {
		    You("feel feverish.");
		    exercise(A_CON, FALSE);
		    u.ulycn = monsndx(mdat);
		}
#endif
		break;
	    case AD_SGLD:
		hitmsg(mtmp, mattk);
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
			if (!tele_restrict(mtmp)) rloc(mtmp);
			return 3;
		} else
#endif
		if(mtmp->mcan) {
		    if (!Blind) {
			pline("%s tries to %s you, but you seem %s.",
			    Adjmonnam(mtmp, "plain"),
			    flags.female ? "charm" : "seduce",
			    flags.female ? "unaffected" : "uninterested");
		    }
		    if(rn2(3)) {
			if (!tele_restrict(mtmp)) rloc(mtmp);
			return 3;
		    }
		} else {
		    switch (steal(mtmp)) {
		      case -1:
			return 2;
		      case 0:
			break;
		      default:
			if (!tele_restrict(mtmp)) rloc(mtmp);
			mtmp->mflee = 1;
			return 3;
		    }
		}
		break;
#ifdef SEDUCE
	    case AD_SSEX:
		if(could_seduce(mtmp, &youmonst, mattk) == 1
			&& !mtmp->mcan)
		    if (doseduce(mtmp))
			return 3;
		break;
#endif
	    case AD_SAMU:
		hitmsg(mtmp, mattk);
		/* when the Wiz hits, 1/20 steals the amulet */
		if ( u.uhave.amulet ||
		     u.uhave.bell || u.uhave.book || u.uhave.menorah
#ifdef MULDGN
		     || u.uhave.questart /* carrying the Quest Artifact */
#endif
		   )
		    if (!rn2(20)) stealamulet(mtmp);
		break;

	    case AD_TLPT:
		hitmsg(mtmp, mattk);
		if(ctmp) {
		    if(flags.verbose)
			Your("position suddenly seems very uncertain!");
		    tele();
		}
		break;
	    case AD_RUST:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
#if defined(POLYSELF)
		if (u.umonnum == PM_IRON_GOLEM) {
			You("rust!");
			rehumanize();
			break;
		}
#endif
		hurtarmor(mdat, AD_RUST);
		break;
	    case AD_DCAY:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
#if defined(POLYSELF)
		if (u.umonnum == PM_WOOD_GOLEM ||
		    u.umonnum == PM_LEATHER_GOLEM) {
			You("rot!");
			rehumanize();
			break;
		}
#endif
		hurtarmor(mdat, AD_DCAY);
		break;
	    case AD_HEAL:
		if(!uwep
#ifdef TOURIST
		   && !uarmu
#endif
		   && !uarm && !uarmh && !uarms && !uarmg && !uarmc && !uarmf) {
		    pline("%s hits!  (I hope you don't mind.)", Monnam(mtmp));
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
		        exercise(A_STR, TRUE);
		        exercise(A_CON, TRUE);
			if (Sick) make_sick(0L, FALSE);
			flags.botl = 1;
			if (mtmp->mhp == 0)
			    return 2; /* mongone() was called above */
			if(!rn2(50)) {
			    if (!tele_restrict(mtmp)) rloc(mtmp);
			    return 3;
			}
			dmg = 0;
		} else
		    if(pl_character[0] == 'H') {
			    if (flags.soundok && !(moves % 5))
				verbalize("Doc, I can't help you unless you cooperate.");
			    dmg = 0;
		    } else hitmsg(mtmp, mattk);
		break;
	    case AD_CURS:
		hitmsg(mtmp, mattk);
		if(!night() && mdat == &mons[PM_GREMLIN]) break;
		if(!mtmp->mcan && !rn2(10)) {
		    if (flags.soundok)
			if (Blind) You("hear laughter.");
			else       pline("%s chuckles.", Monnam(mtmp));
#ifdef POLYSELF
		    if (u.umonnum == PM_CLAY_GOLEM) {
			pline("Some writing vanishes from your head!");
			rehumanize();
			break;
		    }
#endif
		    attrcurse();
		}
		break;
	    case AD_STUN:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(4)) {
		    make_stunned(HStun + dmg, TRUE);
		    dmg /= 2;
		}
		break;
	    case AD_ACID:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(3))
#ifdef POLYSELF
		    if (resists_acid(uasmon)) {
			pline("You're covered in acid, but it seems harmless.");
			dmg = 0;
		    } else
#endif
		      {
			pline("You're covered in acid!	It burns!");
			exercise(A_STR, FALSE);
		      }
		else		dmg = 0;
		break;
	    case AD_SLOW:
		hitmsg(mtmp, mattk);
		if(!ctmp && (Fast & (INTRINSIC|TIMEOUT)) &&
					!defends(AD_SLOW, uwep) && !rn2(4)) {
		    Fast &= ~(INTRINSIC|TIMEOUT);
		    You("feel yourself slowing down.");
		    exercise(A_DEX, FALSE);
		}
		break;
	    case AD_DREN:
		hitmsg(mtmp, mattk);
		if(!ctmp && !rn2(4)) drain_en(dmg);
		dmg = 0;
		break;
	    case AD_CONF:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(4) && !mtmp->mspec_used) {
		    mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
		    if(Confusion)
			 You("are getting even more confused.");
		    else You("are getting confused.");
		    make_confused(HConfusion + dmg, FALSE);
		}
		/* fall through to next case */
	    case AD_DETH:
		pline("%s reaches out with its deadly touch.", Monnam(mtmp));
#ifdef POLYSELF
		if (is_undead(uasmon)) {
		    /* Still does normal damage */
		    pline("Was that the touch of death?");
		    break;
		}
#endif
		if(!Antimagic && rn2(20) > 16)  {
		    killer_format = KILLED_BY_AN;
		    killer = "touch of death";
		    done(DIED);
		} else {
		    if(!rn2(5)) {
			if(Antimagic) shieldeff(u.ux, u.uy);
			pline("Lucky for you, it didn't work!");
			dmg = 0;
		    } else You("feel your life force draining away...");
		}
		break;
	    case AD_PEST:
		pline("%s reaches out, and you feel fever and chills.",
			Monnam(mtmp));
		(void) diseasemu(mdat); /* plus the normal damage */
		break;
	    case AD_FAMN:
		pline("%s reaches out, and your body shrivels.",
			Monnam(mtmp));
		exercise(A_CON, FALSE);
		morehungry(rn1(40,40));
		/* plus the normal damage */
		break;
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

	if(dmg) {
	    if(Half_physical_damage)
		dmg = (dmg+1) / 2;
#ifdef MULDGN
	    else if(pl_character[0] == 'P' && uwep && is_quest_artifact(uwep)
		    && is_undead(mtmp->data))
		dmg = (dmg+1) / 2;
#endif
	    mdamageu(mtmp, dmg);
	}

#ifdef POLYSELF
	if (dmg) {
	    res = passiveum(olduasmon, mtmp, mattk);
	    stop_occupation();
	    return res;
	} else
	    return 1;
#else
	stop_occupation();
	return 1;
#endif
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL int
gulpmu(mtmp, mattk)	/* monster swallows you, or damage if u.uswallow */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	struct trap *t = t_at(u.ux, u.uy);
	int	tmp = d((int)mattk->damn, (int)mattk->damd);
	int	tim_tmp;
	register struct obj *otmp2;
#ifdef WALKIES
	int	i;
#endif

	if(!u.uswallow) {	/* swallows you */
#ifdef POLYSELF
		if (uasmon->msize >= MZ_HUGE) return(0);
#endif
		if ((t && ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT))) &&
		    sobj_at(BOULDER, u.ux, u.uy))
			return(0);	
		if (Punished) unplacebc();	/* ball&chain go away */
		remove_monster(mtmp->mx, mtmp->my);
		place_monster(mtmp, u.ux, u.uy);
		u.ustuck = mtmp;
		newsym(mtmp->mx,mtmp->my);
		pline("%s engulfs you!", Monnam(mtmp));
		stop_occupation();
		if (u.utrap) {
			You("are released from the %s!",
				u.utraptype==TT_WEB ? "web" : "trap");
			u.utrap = 0;
		}
#ifdef WALKIES
		if((i = number_leashed()) > 0) {
			pline("The leash%s snap%s loose.",
					(i > 1) ? "es" : "",
					(i > 1) ? "" : "s");
			unleash_all();
		}
#endif
#ifdef POLYSELF
		if (u.umonnum==PM_COCKATRICE && !resists_ston(mtmp->data)) {
			pline("%s turns to stone!", Monnam(mtmp));
			stoned = 1;
			xkilled(mtmp, 0);
# ifdef MUSE
			if (mtmp->mhp > 0) return 0;
			else
# endif
				return 2;
		}
#endif
		display_nhwindow(WIN_MESSAGE, FALSE);
		vision_recalc(2);	/* hero can't see anything */
		u.uswallow = 1;
		/*assume that u.uswldtim always set >=0*/
		u.uswldtim = (tim_tmp =
			(-u.uac + 10 + rnd(25 - (int)mtmp->m_lev)) >> 1) > 0 ?
			    tim_tmp : 0;
		swallowed(1);
		for(otmp2 = invent; otmp2; otmp2 = otmp2->nobj) {
			(void) snuff_lit(otmp2);
		}
	} else {

	    if(mtmp != u.ustuck) return(0);
	    switch(mattk->adtyp) {

		case AD_DGST:
		    if(u.uswldtim <= 1) {	/* a3 *//*no cf unsigned <=0*/
			pline("%s totally digests you!", Monnam(mtmp));
			tmp = u.uhp;
			if (Half_physical_damage) tmp *= 2; /* sorry */
		    } else {
			pline("%s digests you!", Monnam(mtmp));
		        exercise(A_STR, FALSE);
		    }
		    break;
		case AD_PHYS:
		    You("are pummeled with debris!");
		    exercise(A_STR, FALSE);
		    break;
		case AD_ACID:
#ifdef POLYSELF
		    if (resists_acid(uasmon)) {
			You("are covered with a seemingly harmless goo.");
			tmp = 0;
		    } else
#endif
		    {
		      if (Hallucination) pline("Ouch!  You've been slimed!");
		      else You("are covered in slime!  It burns!");
		      exercise(A_STR, FALSE);
		    }
		    break;
		case AD_BLND:
		    if (!defends(AD_BLND, uwep)) {
			if(!Blind) {
			    You("can't see in here!");
			    make_blinded((long)tmp,FALSE);
			} else
			    /* keep him blind until disgorged */
			    make_blinded(Blinded+1,FALSE);
		    }
		    tmp = 0;
		    break;
		case AD_ELEC:
		    if(!mtmp->mcan && rn2(2)) {
			pline("The air around you crackles with electricity.");
			if (Shock_resistance) {
				shieldeff(u.ux, u.uy);
				You("seem unhurt.");
#if defined(POLYSELF)
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
#if defined(POLYSELF)
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
#if defined(POLYSELF)
				ugolemeffects(AD_FIRE,tmp);
#endif
				tmp = 0;
			} else You("are burning to a crisp!");
		    } else tmp = 0;
		    break;
		case AD_DISE:
		    if (!diseasemu(mtmp->data)) tmp = 0;
		    break;
		default:	tmp = 0;
				break;
	    }
	}

	if (Half_physical_damage) tmp = (tmp+1) / 2;

	mdamageu(mtmp, tmp);
	if(tmp) stop_occupation();
	if(u.uswldtim) --u.uswldtim;
	if(!u.uswldtim
#ifdef POLYSELF
	    || u.umonnum==PM_COCKATRICE
	    || uasmon->msize >= MZ_HUGE
#endif
	    ) {
#ifdef POLYSELF
	    if (u.umonnum == PM_COCKATRICE) {
		pline("%s very hurriedly %s you!", Monnam(mtmp), 
		       is_animal(mtmp->data)? "regurgitates" : "expels");
	    } else {
#endif
		You("get %s!", 
		    is_animal(mtmp->data)? "regurgitated" : "expelled");
		if(flags.verbose && is_animal(mtmp->data))
			pline("Obviously %s doesn't like your taste.",
			       mon_nam(mtmp));
#ifdef POLYSELF
	    }
#endif
	    expels(mtmp, mtmp->data, FALSE);
	}
	return(1);
}

STATIC_OVL int
explmu(mtmp, mattk, ufound)	/* monster explodes in your face */
register struct monst *mtmp;
register struct attack  *mattk;
boolean ufound;
{
    if (mtmp->mcan) return(0);

    if (!ufound)
	pline("%s explodes at a spot in thin air!",
	      canseemon(mtmp) ? Monnam(mtmp) : "It");
    else {
	register int tmp = d((int)mattk->damn, (int)mattk->damd);
	register boolean not_affected = defends((int)mattk->adtyp, uwep);

	hitmsg(mtmp, mattk);

	switch (mattk->adtyp) {
	    case AD_COLD:
		not_affected |= Cold_resistance;

		if (!not_affected) {
		    if (ACURR(A_DEX) > rnd(20)) {
			You("duck some of the blast.");
			tmp = (tmp+1) / 2;
		    } else {
		        if (flags.verbose) You("get blasted!");
		    }
		    if (Half_physical_damage) tmp = (tmp+1) / 2;
		    mdamageu(mtmp, tmp);
		}
		break;

	    case AD_BLND:
		not_affected |=
#ifdef POLYSELF
			(u.umonnum == PM_YELLOW_LIGHT) ||
#endif
			Blind;
		if (!not_affected) {
		    if (mon_visible(mtmp)) {
			You("are blinded by a blast of light!");
			make_blinded((long)tmp, FALSE);
		    } else
			if (flags.verbose)
			You("get the impression it was not terribly bright.");
		}
		break;

	    default:
		break;
	}
	if (not_affected) {
	    You("seem unaffected by it.");
#if defined(POLYSELF)
	    ugolemeffects((int)mattk->adtyp, tmp);
#endif
	}
    }
    mondead(mtmp);
#ifdef MUSE
    if (mtmp->mhp > 0) return(0);
#endif
    return(2);	/* it dies */
}

int
gazemu(mtmp, mattk)	/* monster gazes at you */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	switch(mattk->adtyp) {
	    case AD_STON:
		if (mtmp->mcan) {
		    You("notice that %s isn't all that ugly.",mon_nam(mtmp));
		   break;
		}
		if (canseemon(mtmp)) {
			You("look upon %s.", mon_nam(mtmp));
# ifdef POLYSELF
			if (resists_ston(uasmon)) {
				pline("So what?");
				break;
			}
			if(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))
			    break;
# endif
			You("turn to stone...");
			killer_format = KILLED_BY_AN;
			killer = mons[PM_MEDUSA].mname;
			done(STONING);
	    	}
		break;
	    case AD_CONF:
		if(!mtmp->mcan && canseemon(mtmp) && mtmp->mcansee && 
					!mtmp->mspec_used && rn2(5)) {
		    int conf = d(3,4);

		    mtmp->mspec_used = mtmp->mspec_used + (conf + rn2(6));
		    if(!Confusion)
			pline("%s gaze confuses you!", 
			                  s_suffix(Monnam(mtmp)));
		    else
			You("are getting more and more confused.");
		    make_confused(HConfusion + conf, FALSE);
		}
		break;
	    case AD_STUN:
		if(!mtmp->mcan && canseemon(mtmp) && mtmp->mcansee &&
					!mtmp->mspec_used && rn2(5)) {
		    int stun = d(2,6);

		    pline("%s stares piercingly at you!", Monnam(mtmp));
		    mtmp->mspec_used = mtmp->mspec_used + (stun + rn2(6));
		    make_stunned(HStun + stun, TRUE);
		}
		break;
	    case AD_BLND:
		if(!mtmp->mcan && canseemon(mtmp) && !defends(AD_BLND, uwep) &&
		   distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) {
		    int blnd = d((int)mattk->damn, (int)mattk->damd);
		    You("are blinded by %s radiance!", 
			              s_suffix(mon_nam(mtmp)));
		    make_blinded((long)blnd,FALSE);
		    make_stunned((long)d(1,3),TRUE);
		}
		break;
	    default: impossible("Gaze attack %d?", mattk->adtyp);
		break;
	}
	return(1);
}

#endif /* OVLB */
#ifdef OVL1

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

#endif /* OVL1 */
#ifdef OVLB

#ifdef POLYSELF
STATIC_OVL void
urustm(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	boolean vis;

	if (!mon || !obj) return; /* just in case */
	vis = cansee(mon->mx, mon->my);
	if (u.umonnum == PM_RUST_MONSTER && 
	    is_rustprone(obj) && obj->oeroded < MAX_ERODE) {
		if (obj->greased || obj->oerodeproof || (obj->blessed && rn2(3))) {
		    if (vis)
			pline("Somehow, %s weapon is not affected.",
						s_suffix(mon_nam(mon)));
		    if (obj->greased && !rn2(2)) obj->greased = 0;
		} else {
		    if (vis)
			pline("%s %s%s!",
			        s_suffix(Monnam(mon)), aobjnam(obj, "rust"),
			        obj->oeroded ? " further" : "");
		    obj->oeroded++;
		}
	}
}
#endif

#endif /* OVLB */
#ifdef OVL1

int
could_seduce(magr,mdef,mattk)
struct monst *magr, *mdef;
struct attack *mattk;
/* returns 0 if seduction impossible,
 *	   1 if fine,
 *	   2 if wrong gender for nymph */
{
	register struct permonst *pagr;
	boolean agrinvis, defperc;
	xchar genagr, gendef;

	if(magr == &youmonst) {
		pagr = uasmon;
		agrinvis = (Invis != 0);
		genagr = poly_gender();
	} else {
		pagr = magr->data;
		agrinvis = magr->minvis;
		genagr = gender(magr);
	}
	if(mdef == &youmonst) {
		defperc = (See_invisible != 0);
		gendef = poly_gender();
	} else {
		defperc = perceives(mdef->data);
		gendef = gender(mdef);
	}

	if(agrinvis && !defperc
#ifdef SEDUCE
		&& mattk && mattk->adtyp != AD_SSEX
#endif
		)
		return 0;

	if(pagr->mlet != S_NYMPH
		&& ((pagr != &mons[PM_INCUBUS] && pagr != &mons[PM_SUCCUBUS])
#ifdef SEDUCE
		    || (mattk && mattk->adtyp != AD_SSEX)
#endif
		   ))
		return 0;
	
	if(genagr == 1 - gendef)
		return 1;
	else
		return (pagr->mlet == S_NYMPH) ? 2 : 0;
}

#endif /* OVL1 */
#ifdef OVLB

#ifdef SEDUCE
/* Returns 1 if monster teleported */
int
doseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = (mon->data == &mons[PM_SUCCUBUS]); /* otherwise incubus */
	char qbuf[QBUFSZ];

	if (mon->mcan || mon->mspec_used) {
  		pline("%s acts as though %s has got a %sheadache.",
		      Monnam(mon), he[pronoun_gender(mon)],
		      mon->mcan ? "severe " : "");
		return 0;
	}

	if (unconscious()) {
		pline("%s seems dismayed at your lack of response.",
		      Monnam(mon));
		return 0;
	}

	if (Blind) pline("It caresses you...");
	else You("feel very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
	    nring = ring->nobj;
	    if (ring->otyp != RIN_ADORNMENT) continue;
	    if (fem) {
		if (rn2(20) < ACURR(A_CHA)) {
		    Sprintf(qbuf, "\"That %s looks pretty.  May I have it?\"",
			xname(ring));
		    makeknown(RIN_ADORNMENT);
		    if (yn(qbuf) == 'n') continue;
		} else pline("%s decides she'd like your %s, and takes it.",
			Blind ? "She" : Monnam(mon), xname(ring));
		makeknown(RIN_ADORNMENT);
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
		    Sprintf(qbuf,"\"That %s looks pretty.  Would you wear it for me?\"",
			xname(ring));
		    makeknown(RIN_ADORNMENT);
		    if (yn(qbuf) == 'n') continue;
		} else {
		    pline("%s decides you'd look prettier wearing your %s,",
			Blind ? "He" : Monnam(mon), xname(ring));
		    pline("and puts it on your finger.");
		}
		makeknown(RIN_ADORNMENT);
		if (!uright) {
		    pline("%s puts %s on your right hand.",
			Blind ? "He" : Monnam(mon), the(xname(ring)));
		    setworn(ring, RIGHT_RING);
		} else if (!uleft) {
		    pline("%s puts %s on your left hand.",
			Blind ? "He" : Monnam(mon), the(xname(ring)));
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
	    	prinv(NULL, ring, 0L);
	    }
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("%s murmurs sweet nothings into your ear.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
	else
		pline("%s murmurs in your ear, while helping you undress.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
	mayberem(uarmc, "cloak");
	if(!uarmc)
		mayberem(uarm, "suit");
	mayberem(uarmf, "boots");
	if(!uwep || !welded(uwep))
		mayberem(uarmg, "gloves");
	mayberem(uarms, "shield");
	mayberem(uarmh, "helmet");
#ifdef TOURIST
	if(!uarmc && !uarm)
		mayberem(uarmu, "shirt");
#endif

	if (uarm || uarmc) {
		verbalize("You're such a %s; I wish...",
				flags.female ? "sweet lady" : "nice guy");
		if (!tele_restrict(mon)) rloc(mon);
		return 1;
	}
	if (u.ualign.type == A_CHAOTIC && u.ualign.record < ALIGNLIM)
	    u.ualign.record++;

	/* by this point you have discovered mon's identity, blind or not... */
	pline("Time stands still while you and %s lie in each other's arms...",
		mon_nam(mon));
	if (rn2(35) > ACURR(A_CHA) + ACURR(A_INT)) {
		/* Don't bother with mspec_used here... it didn't get tired! */
		pline("%s seems to have enjoyed it more than you...",
			Monnam(mon));
		switch (rn2(5)) {
			case 0: You("feel drained of energy.");
				u.uen = 0;
				u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
			        exercise(A_CON, FALSE);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
			case 1: You("are down in the dumps.");
				(void) adjattrib(A_CON, -1, TRUE);
			        exercise(A_CON, FALSE);
				flags.botl = 1;
				break;
			case 2: Your("senses are dulled.");
				(void) adjattrib(A_WIS, -1, TRUE);
			        exercise(A_WIS, FALSE);
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
				    if(u.uhp <= 0) {
					killer_format = KILLED_BY;
					killer = "overexertion";
					done(DIED);
				    }
#ifdef POLYSELF
				}
#endif
				break;
			case 4: {
				int tmp;
				You("feel exhausted.");
			        exercise(A_STR, FALSE);
				tmp = rn1(10, 6);
				if(Half_physical_damage) tmp = (tmp+1) / 2;
				losehp(tmp, "exhaustion", KILLED_BY);
				break;
			}
		}
	} else {
		mon->mspec_used = rnd(100); /* monster is worn out */
		You("seem to have enjoyed it more than %s...", mon_nam(mon));
		switch (rn2(5)) {
			case 0: You("feel raised to your full potential.");
			        exercise(A_CON, TRUE);
				u.uen = (u.uenmax += rnd(5));
				break;
			case 1: You("feel good enough to do it again.");
				(void) adjattrib(A_CON, 1, TRUE);
			        exercise(A_CON, TRUE);
				flags.botl = 1;
				break;
			case 2: You("will always remember %s...", mon_nam(mon));
				(void) adjattrib(A_WIS, 1, TRUE);
			        exercise(A_WIS, TRUE);
				flags.botl = 1;
				break;
			case 3: pline("That was a very educational experience.");
				pluslvl();
			        exercise(A_WIS, TRUE);
				break;
			case 4: You("feel restored to health!");
				u.uhp = u.uhpmax;
#ifdef POLYSELF
				if (u.mtimedone) u.mh = u.mhmax;
#endif
			        exercise(A_STR, TRUE);
				flags.botl = 1;
				break;
		}
	}

	if (mon->mtame) /* don't charge */ ;
	else if (rn2(20) < ACURR(A_CHA)) {
		pline("%s demands that you pay %s, but you refuse...",
			Monnam(mon), him[fem]);
	}
#ifdef POLYSELF
	else if (u.umonnum == PM_LEPRECHAUN)
		pline("%s tries to take your money, but fails...",
				Monnam(mon));
#endif
	else {
		long cost;

		if (u.ugold > (long)LARGEST_INT - 10L)
			cost = (long) rnd(LARGEST_INT) + 500L;
		else
			cost = (long) rnd((int)u.ugold + 10) + 500L;
		if (mon->mpeaceful) {
			cost /= 5L;
			if (!cost) cost = 1L;
		}
		if (cost > u.ugold) cost = u.ugold;
		if (!cost) verbalize("It's on the house!");
		else {
		    pline("%s takes %ld zorkmid%s for services rendered!",
			    Monnam(mon), cost, plur(cost));
		    u.ugold -= cost;
		    mon->mgold += cost;
		    flags.botl = 1;
		}
	}
	if (!rn2(25)) mon->mcan = 1; /* monster is worn out */
	if (!tele_restrict(mon)) rloc(mon);
	return 1;
}

static void
mayberem(obj, str)
register struct obj *obj;
const char *str;
{
	char qbuf[QBUFSZ];

	if (!obj || !obj->owornmask) return;

	if (rn2(20) < ACURR(A_CHA)) {
		Sprintf(qbuf,"\"Shall I remove your %s, %s?\"",
			str,
			(!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
		if (yn(qbuf) == 'n') return;
	} else verbalize("Take off your %s; %s.", str,
			(obj == uarm)  ? "let's get a little closer" :
			(obj == uarmc || obj == uarms) ? "it's in the way" :
			(obj == uarmf) ? "let me rub your feet" :
			(obj == uarmg) ? "they're too clumsy" :
#ifdef TOURIST
			(obj == uarmu) ? "let me massage you" :
#endif
			/* obj == uarmh */
			"let me run my fingers through your hair");

	if (donning(obj)) cancel_don();
	if (obj == uarm)  (void) Armor_off();
	else if (obj == uarmc) (void) Cloak_off();
	else if (obj == uarmf) (void) Boots_off();
	else if (obj == uarmg) (void) Gloves_off();
	else if (obj == uarmh) (void) Helmet_off();
	else setworn((struct obj *)0, obj->owornmask & W_ARMOR);
}
#endif  /* SEDUCE */

#endif /* OVLB */

#ifdef POLYSELF

#ifdef OVL1

static int
passiveum(olduasmon,mtmp,mattk)
struct permonst *olduasmon;
register struct monst *mtmp;
register struct attack *mattk;
{
	register struct permonst *mdat = mtmp->data;
	int i, tmp;

	for(i = 0; ; i++) {
	    if(i >= NATTK) return 1;
	    if(olduasmon->mattk[i].aatyp == AT_NONE) break;
	}
	if (olduasmon->mattk[i].damn)
	    tmp = d((int)olduasmon->mattk[i].damn, 
                                    (int)olduasmon->mattk[i].damd);
	else if(olduasmon->mattk[i].damd)
	    tmp = d((int)olduasmon->mlevel+1, (int)olduasmon->mattk[i].damd);
	else
	    tmp = 0;

	/* These affect the enemy even if you were "killed" (rehumanized) */
	switch(olduasmon->mattk[i].adtyp) {
	    case AD_ACID:
		if (!rn2(2)) {
		    pline("%s is splashed by your acid!", Monnam(mtmp));
		    if(resists_acid(mdat)) {
			pline("%s is not affected.", Monnam(mtmp));
			tmp = 0;
		    }
		} else tmp = 0;
		goto assess_dmg;
	    case AD_STON: /* cockatrice */
		if (!resists_ston(mdat) &&
#ifdef MUSE
		    (mattk->aatyp != AT_WEAP || !MON_WEP(mtmp)) &&
#else
		    (mattk->aatyp != AT_WEAP || !select_hwep(mtmp)) &&
#endif
		    mattk->aatyp != AT_GAZE && mattk->aatyp != AT_EXPL &&
		    mattk->aatyp != AT_MAGC &&
#ifdef MUSE
		    (!(mtmp->misc_worn_check & W_ARMG))) {
#else
		    (!is_mercenary(mdat) ||
				      !m_carrying(mtmp, LEATHER_GLOVES))) {
#endif
		    if(poly_when_stoned(mdat)) {
			mon_to_stone(mtmp);
			return (1);
		    }
		    pline("%s turns to stone!", Monnam(mtmp));
		    stoned = 1;
		    xkilled(mtmp, 0);
#ifdef MUSE
		    if (mtmp->mhp > 0) return 1;
#endif
		    return 2;
		}
		return 1;
	    default:
		break;
	}
	if (!u.mtimedone) return 1;

	/* These affect the enemy only if you are still a monster */
	if (rn2(3)) switch(uasmon->mattk[i].adtyp) {
	    case AD_PLYS: /* Floating eye */
		if (tmp > 127) tmp = 127;
		if (u.umonnum == PM_FLOATING_EYE) {
		    if (!rn2(4)) tmp = 127;
		    if (mtmp->mcansee && haseyes(mtmp->data) && rn2(3) &&
				(perceives(mdat) || !Invis)) {
			if (Blind)
			    pline("As a blind %s, you cannot defend yourself.",
							uasmon->mname);
		        else {
#ifdef MUSE
			    if (mon_reflects(mtmp, 
					    "Your gaze is reflected by %s %s."))
				return 1;
#endif
			    pline("%s is frozen by your gaze!", Monnam(mtmp));
			    mtmp->mcanmove = 0;
			    mtmp->mfrozen = tmp;
			    return 3;
			}
		    }
		} else { /* gelatinous cube */
		    pline("%s is frozen by you.", Monnam(mtmp));
		    mtmp->mcanmove = 0;
		    mtmp->mfrozen = tmp;
		    return 3;
		}
		return 1;
	    case AD_COLD: /* Brown mold or blue jelly */
		if(resists_cold(mdat)) {
  		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s is mildly chilly.", Monnam(mtmp));
		    golemeffects(mtmp, AD_COLD, tmp);
		    tmp = 0;
		    break;
		}
		pline("%s is suddenly very cold!", Monnam(mtmp));
		u.mh += tmp / 2;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		if (u.mhmax > ((uasmon->mlevel+1) * 8)) {
			register struct monst *mon;

			if ((mon = cloneu()) != 0) {
			    mon->mhpmax = u.mhmax /= 2;
			    You("multiply from %s heat!", 
				           s_suffix(mon_nam(mtmp)));
			}
		}
		break;
	    case AD_STUN: /* Yellow mold */
		if (!mtmp->mstun) {
		    mtmp->mstun = 1;
		    pline("%s staggers.", Monnam(mtmp));
		}
		tmp = 0;
		break;
	    case AD_FIRE: /* Red mold */
		if(resists_fire(mdat)) {
  		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s is mildly warm.", Monnam(mtmp));
		    golemeffects(mtmp, AD_FIRE, tmp);
		    tmp = 0;
		    break;
		}
		pline("%s is suddenly very hot!", Monnam(mtmp));
		break;
	    case AD_ELEC:
		if(resists_elec(mdat)) {
  		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s is slightly tingled.", Monnam(mtmp));
		    golemeffects(mtmp, AD_ELEC, tmp);
		    tmp = 0;
		    break;
		}
		pline("%s is jolted with your electricity!", Monnam(mtmp));
		break;
	    default: tmp = 0;
		break;
	}
	else tmp = 0;

    assess_dmg:
	if((mtmp->mhp -= tmp) <= 0) {
		pline("%s dies!", Monnam(mtmp));
		xkilled(mtmp,0);
#ifdef MUSE
		if (mtmp->mhp > 0) return 1;
#endif
		return 2;
	}
	return 1;
}

#endif /* OVL1 */
#ifdef OVLB

#include "edog.h"
struct monst *
cloneu()
{
	register struct monst *mon;

	if (u.mh <= 1) return(struct monst *)0;
	if (uasmon->geno & G_EXTINCT) return(struct monst *)0;
	uasmon->pxlth += sizeof(struct edog);
	mon = makemon(uasmon, u.ux, u.uy);
	uasmon->pxlth -= sizeof(struct edog);
	mon = christen_monst(mon, plname);
	initedog(mon);
	mon->m_lev = uasmon->mlevel;
	mon->mhp = u.mh /= 2;
	mon->mhpmax = u.mhmax;
	return(mon);
}

#endif /* OVLB */

#endif /* POLYSELF */

/*mhitu.c*/
