/*	SCCS Id: @(#)uhitm.c	3.2	96/05/23	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static boolean FDECL(known_hitum, (struct monst *,int *,struct attack *));
static void FDECL(steal_it, (struct monst *, struct attack *));
static boolean FDECL(hitum, (struct monst *,int,struct attack *));
static boolean FDECL(m_slips_free, (struct monst *mtmp,struct attack *mattk));
static int FDECL(explum, (struct monst *,struct attack *));
static void FDECL(start_engulf, (struct monst *));
static void NDECL(end_engulf);
static int FDECL(gulpum, (struct monst *,struct attack *));
static boolean FDECL(hmonas, (struct monst *,int));
static void FDECL(nohandglow, (struct monst *));

extern boolean notonhead;	/* for long worms */
/* The below might become a parameter instead if we use it a lot */
static int dieroll;
/* Used to flag attacks caused by Stormbringer's maliciousness. */
static boolean override_confirmation = FALSE;


#ifdef WEAPON_SKILLS
#define PROJECTILE(obj)	((obj) && objects[(obj)->otyp].oc_wepcat == WEP_AMMO)
#endif

boolean
attack_checks(mtmp, wep)
register struct monst *mtmp;
struct obj *wep;	/* uwep for attack(), null for kick_monster() */
{
	char qbuf[QBUFSZ];

	/* if you're close enough to attack, alert any waiting monster */
	mtmp->mstrategy &= ~STRAT_WAITMASK;

	if(mtmp->m_ap_type && !Protection_from_shape_changers
						&& !sensemon(mtmp)) {
		stumble_onto_mimic(mtmp);
		return TRUE;
	}

	if (mtmp->mundetected && !canseemon(mtmp) &&
		(hides_under(mtmp->data) || mtmp->data->mlet == S_EEL)) {
	    mtmp->mundetected = mtmp->msleep = 0;
	    newsym(mtmp->mx, mtmp->my);
	    if (!(Blind ? Telepat : (HTelepat & ~INTRINSIC))) {
		struct obj *obj;

		if (Blind || (is_pool(mtmp->mx,mtmp->my) && !Underwater))
		    pline("Wait!  There's a hidden monster there!");
		else if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0)
		    pline("Wait!  There's %s hiding under %s!",
			  an(l_monnam(mtmp)), doname(obj));
		return TRUE;
	    }
	}

	if (flags.confirm && mtmp->mpeaceful
	    && !Confusion && !Hallucination && !Stunned) {
		/* Intelligent chaotic weapons (Stormbringer) want blood */
		if (wep && wep->oartifact == ART_STORMBRINGER) {
			override_confirmation = TRUE;
			return(FALSE);
		}
		if (canspotmon(mtmp)) {
			Sprintf(qbuf, "Really attack %s?", mon_nam(mtmp));
			if (yn(qbuf) != 'y') {
				flags.move = 0;
				return(TRUE);
			}
		}
	}

	return(FALSE);
}

schar
find_roll_to_hit(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	int tmp2;

	tmp = 1 + Luck + abon() + find_mac(mtmp) +
		maybe_polyd(uasmon->mlevel, u.ulevel);

/*	it is unchivalrous to attack the defenseless or from behind */
	if (Role_is('K') && u.ualign.type == A_LAWFUL &&
	    (!mtmp->mcanmove || mtmp->msleep || mtmp->mflee) &&
	    u.ualign.record > -10) adjalign(-1);

/*	attacking peaceful creatures is bad for the samurai's giri */
	if (Role_is('S') && mtmp->mpeaceful &&
	    u.ualign.record > -10) adjalign(-1);

/*	Adjust vs. (and possibly modify) monster state.		*/

	if(mtmp->mstun) tmp += 2;
	if(mtmp->mflee) tmp += 2;

	if(mtmp->msleep) {
		mtmp->msleep = 0;
		tmp += 2;
	}
	if(!mtmp->mcanmove) {
		tmp += 4;
		if(!rn2(10)) {
			mtmp->mcanmove = 1;
			mtmp->mfrozen = 0;
		}
	}
	if (is_orc(mtmp->data) && maybe_polyd(is_elf(uasmon), Role_is('E')))
	    tmp++;

/*	with a lot of luggage, your agility diminishes */
	if ((tmp2 = near_capacity()) != 0) tmp -= (tmp2*2) - 1;
	if (u.utrap) tmp -= 3;
/*	Some monsters have a combination of weapon attacks and non-weapon
 *	attacks.  It is therefore wrong to add hitval to tmp; we must add
 *	it only for the specific attack (in hmonas()).
 */
	if (uwep) tmp += maybe_polyd(0, hitval(uwep, mtmp));
	return tmp;
}

/* try to attack; return FALSE if monster evaded */
/* u.dx and u.dy must be set */
boolean
attack(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	register struct permonst *mdat = mtmp->data;

	/* This section of code provides protection against accidentally
	 * hitting peaceful (like '@') and tame (like 'd') monsters.
	 * Protection is provided as long as player is not: blind, confused,
	 * hallucinating or stunned.
	 * changes by wwp 5/16/85
	 * More changes 12/90, -dkh-. if its tame and safepet, (and protected
	 * 07/92) then we assume that you're not trying to attack. Instead,
	 * you'll usually just swap places if this is a movement command
	 */
	/* Intelligent chaotic weapons (Stormbringer) want blood */
	if (is_safepet(mtmp)) {
	    if (!uwep || uwep->oartifact != ART_STORMBRINGER) {
		/* there are some additional considerations: this won't work
		 * if in a shop or Punished or you miss a random roll or
		 * if you can walk thru walls and your pet cannot (KAA) or
		 * if your pet is a long worm (unless someone does better).
		 * there's also a chance of displacing a "frozen" monster.
		 * sleeping monsters might magically walk in their sleep.
		 */
		unsigned int foo = (Punished ||
				    !rn2(7) || is_longworm(mtmp->data));

		if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE) || foo
			|| (IS_ROCK(levl[u.ux][u.uy].typ) &&
					!passes_walls(mtmp->data))) {
		    mtmp->mflee = 1;
		    mtmp->mfleetim = rnd(6);
		    You("stop.  %s is in your way!",
			(mtmp->mnamelth ? NAME(mtmp) : Monnam(mtmp)));
		    return(TRUE);
		} else if ((mtmp->mfrozen || (! mtmp->mcanmove)
				|| (mtmp->data->mmove == 0)) && rn2(6)) {
		    pline("%s doesn't seem to move!", Monnam(mtmp));
		    return(TRUE);
		} else return(FALSE);
	    }
	}

	/* possibly set in attack_checks;
	   examined in known_hitum, called via hitum or hmonas below */
	override_confirmation = FALSE;
	if (attack_checks(mtmp, uwep)) return(TRUE);

	if (Upolyd) {
		/* certain "pacifist" monsters don't attack */
		if(noattacks(uasmon)) {
			You("have no way to attack monsters physically.");
			mtmp->mstrategy &= ~STRAT_WAITMASK;
			return(TRUE);
		}
	}

	if(check_capacity("You cannot fight while so heavily loaded."))
	    return (TRUE);

	if(unweapon) {
	    unweapon=FALSE;
	    if(flags.verbose)
		if(uwep)
		    You("begin bashing monsters with your %s.",
			aobjnam(uwep, (char *)0));
		else if (!cantwield(uasmon))
		    You("begin bashing monsters with your %s %s.",
			uarmg ? "gloved" : "bare",	/* Del Lamb */
			makeplural(body_part(HAND)));
	}
	exercise(A_STR, TRUE);		/* you're exercising muscles */
	/* andrew@orca: prevent unlimited pick-axe attacks */
	u_wipe_engr(3);

	if(mdat->mlet == S_LEPRECHAUN && mtmp->mfrozen && !mtmp->msleep &&
	   !mtmp->mconf && mtmp->mcansee && !rn2(7) &&
	   (m_move(mtmp, 0) == 2 ||			    /* it died */
	   mtmp->mx != u.ux+u.dx || mtmp->my != u.uy+u.dy)) /* it moved */
		return(FALSE);

	tmp = find_roll_to_hit(mtmp);
	if (Upolyd)
		(void) hmonas(mtmp, tmp);
	else
		(void) hitum(mtmp, tmp, playermon.mattk);
	mtmp->mstrategy &= ~STRAT_WAITMASK;
	return(TRUE);
}

static boolean
known_hitum(mon, mhit, uattk)	/* returns TRUE if monster still lives */
register struct monst *mon;
register int *mhit;
struct attack *uattk;
{
	register boolean malive = TRUE, special;

	/* we need to know whether the special monster was peaceful */
	/* before the attack, to save idle calls to angry_guards()  */
	special = (mon->mpeaceful && (mon->data == &mons[PM_WATCHMAN] ||
				mon->data == &mons[PM_WATCH_CAPTAIN] ||
				      mon->ispriest || mon->isshk));

	if (override_confirmation) {
	    /* this may need to be generalized if weapons other than
	       Stormbringer acquire similar anti-social behavior... */
	    if (flags.verbose) Your("bloodthirsty blade attacks!");
	}

	if(!*mhit) {
	    missum(mon, uattk);
	} else {
	    int oldhp = mon->mhp;

	    /* we hit the monster; be careful: it might die! */
	    notonhead = (mon->mx != u.ux+u.dx || mon->my != u.uy+u.dy);
	    if((malive = hmon(mon, uwep, 0)) == TRUE) {
		/* monster still alive */
		if(!rn2(25) && mon->mhp < mon->mhpmax/2) {
			mon->mflee = 1;
			if(!rn2(3)) mon->mfleetim = rnd(100);
			if(u.ustuck == mon && !u.uswallow && !sticks(uasmon))
				u.ustuck = 0;
		}
		/* Vorpal Blade hit converted to miss */
		/* could be headless monster or worm tail */
		if (mon->mhp == oldhp)
			*mhit = 0;
		if (mon->wormno && *mhit)
			cutworm(mon, u.ux+u.dx, u.uy+u.dy, uwep);
	    }
	    if(mon->ispriest && !rn2(2)) ghod_hitsu(mon);
	    if(special) (void) angry_guards(!flags.soundok);
	}
	return(malive);
}

static boolean
hitum(mon, tmp, uattk)		/* returns TRUE if monster still lives */
struct monst *mon;
int tmp;
struct attack *uattk;
{
	boolean malive;
	int mhit = (tmp > (dieroll = rnd(20)) || u.uswallow);

	if(tmp > dieroll) exercise(A_DEX, TRUE);
	malive = known_hitum(mon, &mhit, uattk);
	(void) passive(mon, mhit, malive, FALSE);
	return(malive);
}

boolean			/* general "damage monster" routine */
hmon(mon, obj, thrown)		/* return TRUE if mon still alive */
register struct monst *mon;
register struct obj *obj;
register int thrown;
{
	int tmp;
	struct permonst *mdat = mon->data;
	/* Why all these booleans?  This stuff has to be done in the
	 *      following order:
	 * 1) Know what we're attacking with, and print special hittxt for
	 *	unusual cases.
	 * 2a) Know whether we did damage (depends on 1)
	 * 2b) Know if it's poisoned (depends on 1)
	 * 2c) Know whether we get a normal damage bonus or not (depends on 1)
	 * 3a) Know what the value of the damage bonus is (depends on 2c)
	 * 3b) Know how much poison damage was taken (depends on 2b) and if the
	 *	poison instant-killed it
	 * 4) Know if it was killed (requires knowing 3a, 3b) except by instant-
	 *	kill poison
	 * 5) Print hit message (depends on 1 and 4)
	 * 6a) Print poison message (must be done after 5)
#if 0
	 * 6b) Rust weapon (must be done after 5)
#endif
	 * 7) Possibly kill monster (must be done after 6a, 6b)
	 * 8) Instant-kill from poison (can happen anywhere between 5 and 9)
	 * 9) Hands not glowing (must be done after 7 and 8)
	 * The major problem is that since we don't want a "hit" message
	 * when the monster dies, we have to know how much damage it did
	 * _before_ outputting a hit message, but any messages associated with
	 * the damage don't come out until _after_ outputting a hit message.
	 */
	boolean hittxt = FALSE, destroyed = FALSE;
	boolean get_dmg_bonus = TRUE;
	boolean ispoisoned = FALSE, needpoismsg = FALSE, poiskilled = FALSE;
	boolean silvermsg = FALSE;
#ifdef WEAPON_SKILLS
	boolean valid_weapon_attack = FALSE;
	int type;
	struct obj *monwep;
#endif /* WEAPON_SKILLS */

	wakeup(mon);
	if(!obj) {	/* attack with bare hands */
	    if (mdat == &mons[PM_SHADE])
		tmp = 0;
#ifdef WEAPON_SKILLS
	    else if (martial_bonus())
		tmp = rnd(4);	/* bonus for martial arts */
	    else
		tmp = rnd(2);
	    valid_weapon_attack = (tmp > 1);
#else
	    else
		tmp = rnd(2);
#endif /* WEAPON_SKILLS */
	} else {
	    if(obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
	       obj->oclass == GEM_CLASS) {

		/* is it not a melee weapon? */
		if (/* if you strike with a bow... */
		    objects[obj->otyp].oc_wepcat == WEP_BOW ||
		    /* or strike with a missile in your hand... */
		    (!thrown && (objects[obj->otyp].oc_wepcat == WEP_MISSILE ||
				 objects[obj->otyp].oc_wepcat == WEP_AMMO)) ||
		    /* or throw a missile without the proper bow... */
		    (objects[obj->otyp].oc_wepcat == WEP_AMMO &&
		     (!uwep || objects[obj->otyp].w_propellor !=
			       -objects[uwep->otyp].w_propellor))) {
		    /* then do only 1-2 points of damage */
		    if (mdat == &mons[PM_SHADE] && obj->otyp != SILVER_ARROW)
			tmp = 0;
		    else
			tmp = rnd(2);
		} else {
		    tmp = dmgval(obj, mon);
#ifdef WEAPON_SKILLS
		    /* a minimal hit doesn't exercise proficiency */
		    valid_weapon_attack = (tmp > 1);
		    if (!valid_weapon_attack || mon == u.ustuck) {
			;	/* no special bonuses */
		    } else if (mon->mflee && Role_is('R') && !Upolyd) {
			You("strike %s from behind!", mon_nam(mon));
			tmp += rnd(u.ulevel);
			hittxt = TRUE;
		    } else if (dieroll == 2 && obj == uwep &&
			  obj->oclass == WEAPON_CLASS &&
			  (bimanual(obj) ||
			    (Role_is('S') && obj->otyp == KATANA && !uarms)) &&
			  ((type = weapon_type(obj)) != P_NO_TYPE &&
			    P_SKILL(type) >= P_SKILLED) &&
			  ((monwep = MON_WEP(mon)) != 0 &&
			    !obj_resists(monwep,
					 50 + 15 * (int)obj->oeroded, 100))) {
			/*
			 * 2.5% chance of shattering defender's weapon when
			 * using a two-handed weapon; less if uwep is rusted.
			 * [dieroll == 2 is most successful non-beheading or
			 * -bisecting hit, in case of special artifact damage;
			 * the percentage chance is (1/20)*(50/100).]
			 */
			monwep->owornmask &= ~W_WEP;
			MON_NOWEP(mon);
			mon->weapon_check = NEED_WEAPON;
			pline("%s %s shatters from the force of your blow!",
			      s_suffix(Monnam(mon)), xname(monwep));
			m_useup(mon, monwep);
			/* If someone just shattered MY weapon, I'd flee! */
			if (rn2(4) && !mon->mflee) {
			    mon->mflee = 1;
			    mon->mfleetim = d(2,3);
			}
			hittxt = TRUE;
		    }
#endif /* WEAPON_SKILLS */
		    if (obj->oartifact &&
			artifact_hit(&youmonst, mon, obj, &tmp, dieroll)) {
			if(mon->mhp <= 0) /* artifact killed monster */
			    return FALSE;
			if (tmp == 0) return TRUE;
			hittxt = TRUE;
		    }
		    if (objects[obj->otyp].oc_material == SILVER
				&& hates_silver(mdat))
			silvermsg = TRUE;
		    if(!thrown && obj == uwep && obj->otyp == BOOMERANG &&
		       !rnl(3)) {
			pline("As you hit %s, %s breaks into splinters.",
			      mon_nam(mon), the(xname(obj)));
			useup(obj);
			obj = (struct obj *) 0;
			hittxt = TRUE;
			if (mdat != &mons[PM_SHADE])
			    tmp++;
		    } else if(thrown &&
			      (objects[obj->otyp].oc_wepcat == WEP_AMMO ||
			       objects[obj->otyp].oc_wepcat == WEP_MISSILE)) {
			if(objects[obj->otyp].oc_wepcat == WEP_AMMO &&
			   uwep && objects[obj->otyp].w_propellor ==
			   -objects[uwep->otyp].w_propellor) {
			    /* Elves and Samurai do extra damage using
			     * their bows&arrows; they're highly trained.
			     */
			    if (Role_is('S') &&
				obj->otyp == YA && uwep->otyp == YUMI)
				tmp++;
			    else if (Role_is('E') &&
				     obj->otyp == ELVEN_ARROW &&
				     uwep->otyp == ELVEN_BOW)
				tmp++;
			}
			if(obj->opoisoned &&
			   ((uwep && objects[obj->otyp].w_propellor ==
				     -objects[uwep->otyp].w_propellor) ||
			    objects[obj->otyp].oc_wepcat == WEP_MISSILE))
			    ispoisoned = TRUE;
		    }
		}
	    } else if(obj->oclass == POTION_CLASS) {
			if (obj->quan > 1L) setworn(splitobj(obj, 1L), W_WEP);
			else setuwep((struct obj *)0);
			freeinv(obj);
			potionhit(mon,obj);
			hittxt = TRUE;
			if (mdat == &mons[PM_SHADE])
			    tmp = 0;
			else
			    tmp = 1;
	    } else {
		switch(obj->otyp) {
		    case HEAVY_IRON_BALL:
			tmp = rnd(25); break;
		    case BOULDER:
			tmp = rnd(20); break;
		    case MIRROR:
			You("break your mirror.  That's bad luck!");
			change_luck(-2);
			useup(obj);
			obj = (struct obj *) 0;
			hittxt = TRUE;
			tmp = 1;
			break;
#ifdef TOURIST
		    case EXPENSIVE_CAMERA:
	You("succeed in destroying your camera.  Congratulations!");
			useup(obj);
			return(TRUE);
#endif
		    case CORPSE:		/* fixed by polder@cs.vu.nl */
			if (obj->corpsenm == PM_COCKATRICE) {
			    tmp = 1;
			    hittxt = TRUE;
			    You("hit %s with %s cockatrice corpse.",
				mon_nam(mon), obj->dknown ? "the" : "a");
			    if (!munstone(mon, TRUE))
				minstapetrify(mon, TRUE);
			    if (resists_ston(mon)) break;
			    /* note: hp may be <= 0 even if munstoned==TRUE */
			    return (boolean) (mon->mhp > 0);
#if 0
			} else if (mdat == &mons[PM_COCKATRICE]) {
			    /* maybe turn the corpse into a statue? */
#endif
			}
			tmp = (obj->corpsenm >= LOW_PM ?
					mons[obj->corpsenm].msize : 0) + 1;
			break;
		    case EGG:
		      {
			/* setting quantity to 1 forces complete `useup' */
#define useup_eggs(o)	{ o->quan = 1L; \
			  if (thrown) obfree(o,(struct obj *)0); \
			  else useup(o); \
			  o = (struct obj *)0; }	/* now gone */
			long cnt = obj->quan;

			tmp = 1;		/* nominal physical damage */
			get_dmg_bonus = FALSE;
			hittxt = TRUE;		/* message always given */
			/* egg is always either used up or transformed, so next
			   hand-to-hand attack should yield a "bashing" mesg */
			if (obj == uwep) unweapon = TRUE;
			if (obj->spe && obj->corpsenm >= LOW_PM)
			    if (obj->quan < 5)
				change_luck((schar) -(obj->quan));
			    else
				change_luck(-5);

			if (obj->corpsenm == PM_COCKATRICE) {
			    learn_egg_type(PM_COCKATRICE);
			    You("hit %s with %s cockatrice egg%s.  Splat!",
				mon_nam(mon),
				obj->known ? "the" : cnt > 1L ? "some" : "a",
				plur(cnt));
			    obj->known = 1;	/* (not much point...) */
			    useup_eggs(obj);
			    if (!munstone(mon, TRUE))
				minstapetrify(mon, TRUE);
			    if (resists_ston(mon)) break;
			    return (boolean) (mon->mhp > 0);
			} else {	/* ordinary egg(s) */
			    const char *eggp =
				     (obj->corpsenm != NON_PM && obj->known) ?
					      the(mons[obj->corpsenm].mname) :
					      (cnt > 1L) ? "some" : "an";
			    You("hit %s with %s egg%s.",
				mon_nam(mon), eggp, plur(cnt));
			    if (mdat == &mons[PM_COCKATRICE]) {
				pline_The("egg%s %s alive any more...",
				      plur(cnt),
				      (cnt == 1L) ? "isn't" : "aren't");
				if (obj->timed) obj_stop_timers(obj);
				obj->otyp = ROCK;
				obj->oclass = GEM_CLASS;
				obj->oartifact = 0;
				obj->spe = 0;
				obj->known = obj->dknown = obj->bknown = 0;
				obj->owt = weight(obj);
				if (thrown) place_object(obj, mon->mx, mon->my);
			    } else {
				pline("Splat!");
				useup_eggs(obj);
				exercise(A_WIS, FALSE);
			    }
			}
			break;
#undef useup_eggs
		      }
		    case CLOVE_OF_GARLIC:	/* no effect against demons */
			if (is_undead(mdat)) {
			    mon->mflee = 1;
			    mon->mfleetim += d(2,4);
			}
			tmp = 1;
			break;
		    case CREAM_PIE:
		    case BLINDING_VENOM:
			/* note: resists_blnd() does not apply here */
			if (Blind || !haseyes(mdat))
			    pline(obj->otyp==CREAM_PIE ? "Splat!" : "Splash!");
			else if (obj->otyp == BLINDING_VENOM)
			    pline_The("venom blinds %s%s!", mon_nam(mon),
					mon->mcansee ? "" : " further");
			else {
			    char *whom = mon_nam(mon);
			    /* note: s_suffix returns a modifiable buffer */
			    if (haseyes(mdat) && mdat != &mons[PM_FLOATING_EYE])
				whom = strcat(s_suffix(whom), " face");
			    pline_The("cream pie splashes over %s!", whom);
			}
			if(mon->msleep) mon->msleep = 0;
			setmangry(mon);
			if(haseyes(mon->data)) {
			    mon->mcansee = 0;
			    tmp = rn1(25, 21);
			    if(((int) mon->mblinded + tmp) > 127)
				mon->mblinded = 127;
			    else mon->mblinded += tmp;
			}
			if (thrown) obfree(obj, (struct obj *)0);
			else useup(obj);
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			tmp = 0;
			break;
		    case ACID_VENOM: /* thrown (or spit) */
			if (resists_acid(mon)) {
				Your("venom hits %s harmlessly.",
					mon_nam(mon));
				tmp = 0;
			} else {
				Your("venom burns %s!", mon_nam(mon));
				tmp = dmgval(obj, mon);
			}
			if (thrown) obfree(obj, (struct obj *)0);
			else useup(obj);
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			break;
		    default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/100;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
		}
		if (mdat == &mons[PM_SHADE] && obj &&
				objects[obj->otyp].oc_material != SILVER)
		    tmp = 0;
	    }
	}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG)
	 *      *OR* if attacking bare-handed!! */

	if (get_dmg_bonus && tmp > 0) {
		tmp += u.udaminc;
		/* If you throw using a propellor, you don't get a strength
		 * bonus but you do get an increase-damage bonus.
		 */
		if(!thrown || !obj || !uwep ||
		   (obj->oclass != GEM_CLASS && obj->oclass != WEAPON_CLASS) ||
		   !objects[obj->otyp].w_propellor ||
		   (objects[obj->otyp].w_propellor !=
				-objects[uwep->otyp].w_propellor))
		    tmp += dbon();
	}

#ifdef WEAPON_SKILLS
	if (valid_weapon_attack) {
	    struct obj *wep;

	    /* to be valid a projectile must have had the correct projector */
	    wep = PROJECTILE(obj) ? uwep : obj;
	    tmp += weapon_dam_bonus(wep);
	    use_skill(weapon_type(wep));
	}
#endif /* WEAPON_SKILLS */

	if (ispoisoned) {
	    if (resists_poison(mon))
		needpoismsg = TRUE;
	    else if (rn2(10))
		tmp += rnd(6);
	    else poiskilled = TRUE;
	}
	if (tmp < 1 && !hittxt) {
	    if (mdat == &mons[PM_SHADE]) {
		Your("attack passes harmlessly through %s.",
			mon_nam(mon));
		hittxt = TRUE;
		tmp = 0;
	    } else
		tmp = 1;
	}

#ifdef WEAPON_SKILLS
	/* VERY small chance of stunning opponent if unarmed. */
	if (tmp > 1 && !thrown && !obj && !uwep && !uarm && !uarms && !Upolyd) {
	    if (rnd(100) < P_SKILL(weapon_type((struct obj *) 0)) &&
			!bigmonst(mdat) && !thick_skinned(mdat)) {
		if (canspotmon(mon))
		    pline("%s staggers from your powerful strike!",
			  Monnam(mon));
		mon->mstun = 1;
		hittxt = TRUE;
		if (mon->mcanmove && mon != u.ustuck) {
		    xchar mdx, mdy;

		    /* see if the monster has a place to move into */
		    mdx = mon->mx + u.dx;
		    mdy = mon->my + u.dy;
		    if (goodpos(mdx, mdy, mon, mon->data)) {
			remove_monster(mon->mx, mon->my);
			newsym(mon->mx, mon->my);
			place_monster(mon, mdx, mdy);
			newsym(mon->mx, mon->my);
			set_apparxy(mon);
		    }
		}
	    }
	}
#endif /* WEAPON_SKILLS */

	mon->mhp -= tmp;
	if(mon->mhp < 1)
		destroyed = TRUE;
	if (mon->mtame && (!mon->mflee || mon->mfleetim) && tmp > 0) {
		unsigned fleetim;

		abuse_dog(mon);
		mon->mflee = TRUE;		/* Rick Richardson */
		fleetim = mon->mfleetim + (unsigned)(10 * rnd(tmp));
		mon->mfleetim = min(fleetim,127);
	}
	if((mdat == &mons[PM_BLACK_PUDDING] || mdat == &mons[PM_BROWN_PUDDING])
		   && obj && obj == uwep
		   && objects[obj->otyp].oc_material == IRON
		   && mon->mhp > 1 && !thrown && !mon->mcan
		   /* && !destroyed  -- guaranteed by mhp > 1 */ ) {
		if (clone_mon(mon)) {
			pline("%s divides as you hit it!", Monnam(mon));
			hittxt = TRUE;
		}
	}

	if(!hittxt && !destroyed) {
		if(thrown)
		    /* thrown => obj exists */
		    hit(xname(obj), mon, exclam(tmp) );
		else if(!flags.verbose) You("hit it.");
		else You("%s %s%s", Role_is('B') ? "smite" : "hit",
			 mon_nam(mon), canseemon(mon) ? exclam(tmp) : ".");
	}

	if (silvermsg) {
		const char *fmt;
		char *whom = mon_nam(mon);

		if (canspotmon(mon)) {
		    fmt = "The silver sears %s!";
		} else {
		    *whom = highc(*whom);	/* "it" -> "It" */
		    fmt = "%s is seared!";
		}
		/* note: s_suffix returns a modifiable buffer */
		if (!noncorporeal(mdat))
		    whom = strcat(s_suffix(whom), " flesh");
		pline(fmt, whom);
	}

	if (needpoismsg)
		pline_The("poison doesn't seem to affect %s.", mon_nam(mon));
	if (poiskilled) {
		pline_The("poison was deadly...");
		xkilled(mon, 0);
		return FALSE;
	} else if (destroyed) {
		killed(mon);	/* takes care of most messages */
	} else if(u.umconf && !thrown) {
		nohandglow(mon);
		if(!mon->mconf && !resist(mon, '+', 0, NOTELL)) {
			mon->mconf = 1;
			if(!mon->mstun && mon->mcanmove && !mon->msleep &&
			   canseemon(mon))
				pline("%s appears confused.", Monnam(mon));
		}
	}

#if 0
	if(mdat == &mons[PM_RUST_MONSTER] && obj && obj == uwep &&
		is_rustprone(obj) && obj->oeroded < MAX_ERODE) {
	    if (obj->greased)
		grease_protect(obj,(char *)0,FALSE);
	    else if (obj->oerodeproof || (obj->blessed && !rnl(4))) {
	        if (flags.verbose)
			pline("Somehow, your %s is not affected.",
			      is_sword(obj) ? "sword" : "weapon");
	    } else {
		Your("%s%s!", aobjnam(obj, "rust"),
		     obj->oeroded+1 == MAX_ERODE ? " completely" :
		     obj->oeroded ? " further" : "");
		obj->oeroded++;
	    }
	}
#endif

	return((boolean)(destroyed ? FALSE : TRUE));
}

/* check whether slippery clothing protects from hug or wrap attack */
/* [currently assumes that you are the attacker] */
static boolean
m_slips_free(mdef, mattk)
struct monst *mdef;
struct attack *mattk;
{
	struct obj *obj = which_armor(mdef, W_ARMC);

	if (!obj) obj = which_armor(mdef, W_ARM);
#ifdef TOURIST
	if (!obj) obj = which_armor(mdef, W_ARMU);
#endif
	/* if defender's cloak/armor is greased, attacker slips off */
	if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK)) {
	    You("%s %s %s %s!",
		mattk->adtyp == AD_WRAP ?
			"slip off of" : "grab, but cannot hold onto",
		s_suffix(mon_nam(mdef)),
		obj->greased ? "greased" : "slippery",
		/* avoid "slippery slippery cloak"
		   for undiscovered oilskin cloak */
		(obj->greased || objects[obj->otyp].oc_name_known) ?
			xname(obj) : "cloak");

	    if (obj->greased && !rn2(2)) {
		pline_The("grease wears off.");
		obj->greased = 0;
	    }
	    return TRUE;
	}
	return FALSE;
}

static void NDECL(demonpet);
/*
 * Send in a demon pet for the hero.  Exercise wisdom.
 *
 * This function used to be inline to damageum(), but the Metrowerks compiler
 * (DR4 and DR4.5) screws up with an internal error 5 "Expression Too Complex."
 * Pulling it out makes it work.
 */
static void
demonpet()
{
	struct permonst *pm;
	struct monst *dtmp;

	pline("Some hell-p has arrived!");
	pm = !rn2(6) ? &mons[ndemon(u.ualign.type)] : uasmon;
	if ((dtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS)) != 0)
	    (void)tamedog(dtmp, (struct obj *)0);
	exercise(A_WIS, TRUE);
}

static void
steal_it(mdef, mattk)
struct monst *mdef;
struct attack *mattk;
{
	
	if(mdef->minvent) {
	    struct obj *otmp, *stealoid;
	
	    stealoid = (struct obj *)0;
	    if (could_seduce(&youmonst,mdef,mattk)){
		for(otmp = mdef->minvent; otmp; otmp=otmp->nobj)
		    if (otmp->owornmask & W_ARM) {
			stealoid = otmp;
			/* unwear even if not stolen */
			mdef->misc_worn_check &= ~W_ARM;
			stealoid->owornmask = 0L;
			update_mon_intrinsics(mdef, stealoid, FALSE);
		    }
	    }
	    if (stealoid) {
		boolean whoops = FALSE, stolen = FALSE;
	
		if (gender(mdef) == (int) u.mfemale &&
					uasmon->mlet == S_NYMPH)
		    You(
		  "charm %s.  She gladly hands over her possessions.",
			mon_nam(mdef));
		else
		    You(
		    "seduce %s and %s starts to take off %s clothes.",
			mon_nam(mdef), he[pronoun_gender(mdef)],
			his[pronoun_gender(mdef)]);
		while ((otmp = mdef->minvent) != 0) {
			obj_extract_self(otmp);
			if (otmp->owornmask) {
			    mdef->misc_worn_check &= ~otmp->owornmask;
			    otmp->owornmask = 0L;
			    update_mon_intrinsics(mdef, otmp, FALSE);
			}
			if (!stolen && otmp==stealoid) {
			    otmp = hold_another_object(otmp,
				      (const char *)0, (const char *)0,
						      (const char *)0);
			    stealoid = otmp;
			    stolen = TRUE;
			} else {
			    otmp = hold_another_object(otmp,
					 "You steal %s.", doname(otmp),
							"You steal: ");
			}
			if (otmp->otyp == CORPSE &&
			    otmp->corpsenm == PM_COCKATRICE &&
			    !uarmg) {
				whoops = TRUE;
				break;
			}
		}
		pline("%s finishes taking off %s suit.",
		      Monnam(mdef), his[pronoun_gender(mdef)]);
		if (stolen) You("steal %s!", doname(stealoid));
		if (whoops) instapetrify("cockatrice corpse");
		possibly_unwield(mdef);
	   } else {
		otmp = mdef->minvent;
		obj_extract_self(otmp);
		if (otmp->owornmask) {
		    mdef->misc_worn_check &= !otmp->owornmask;
		    otmp->owornmask = 0L;
		    update_mon_intrinsics(mdef, otmp, FALSE);
		}
		otmp = hold_another_object(otmp, "You steal %s.",
					  doname(otmp), "You steal: ");
		if (!(mdef->misc_worn_check & W_ARMG))
		    mselftouch(mdef, (const char *)0, TRUE);
		possibly_unwield(mdef);
	   }
	}
}

int
damageum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register struct permonst *pd = mdef->data;
	register int	tmp = d((int)mattk->damn, (int)mattk->damd);

	if (is_demon(uasmon) && !rn2(13) && !uwep
		&& u.umonnum != PM_SUCCUBUS && u.umonnum != PM_INCUBUS
		&& u.umonnum != PM_BALROG) {
	    demonpet();
	    return(0);
	}
	switch(mattk->adtyp) {
	    case AD_STUN:
		if(!Blind)
		    pline("%s staggers for a moment.", Monnam(mdef));
		mdef->mstun = 1;
		/* fall through to next case */
	    case AD_WERE:	    /* no effect on monsters */
	    case AD_HEAL:
	    case AD_LEGS:
	    case AD_PHYS:
		if(mattk->aatyp == AT_WEAP) {
			if(uwep) tmp = 0;
		} else if(mattk->aatyp == AT_KICK)
			if(thick_skinned(mdef->data)) tmp = 0;
		break;
	    case AD_FIRE:
		if (!Blind)
		    pline("%s is %s!", Monnam(mdef),
			  mattk->aatyp == AT_HUGS ?
				"being roasted" : "on fire");
		tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if (resists_fire(mdef)) {
		    if (!Blind)
			pline_The("fire doesn't heat %s!", mon_nam(mdef));
		    golemeffects(mdef, AD_FIRE, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
	    case AD_COLD:
		if (!Blind) pline("%s is covered in frost!", Monnam(mdef));
		if (resists_cold(mdef)) {
		    shieldeff(mdef->mx, mdef->my);
		    if (!Blind)
			pline_The("frost doesn't chill %s!", mon_nam(mdef));
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
	    case AD_ELEC:
		if (!Blind) pline("%s is zapped!", Monnam(mdef));
		tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if (resists_elec(mdef)) {
		    if (!Blind)
			pline_The("zap doesn't shock %s!", mon_nam(mdef));
		    golemeffects(mdef, AD_ELEC, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
	    case AD_ACID:
		if (resists_acid(mdef)) tmp = 0;
		break;
	    case AD_STON:
		if (!munstone(mdef, TRUE))
		    minstapetrify(mdef, TRUE);
		tmp = 0;
		break;
#ifdef SEDUCE
	    case AD_SSEX:
#endif
	    case AD_SEDU:
	    case AD_SITM:
		steal_it(mdef, mattk);
		tmp = 0;
		break;
	    case AD_SGLD:
		if (mdef->mgold) {
		    u.ugold += mdef->mgold;
		    mdef->mgold = 0;
		    Your("purse feels heavier.");
		}
		exercise(A_DEX, TRUE);
		tmp = 0;
		break;
	    case AD_TLPT:
		if(tmp <= 0) tmp = 1;
		if(tmp < mdef->mhp) {
		    rloc(mdef);
		    if(!Blind) pline("%s suddenly disappears!", Monnam(mdef));
		}
		break;
	    case AD_BLND:
		if (!resists_blnd(mdef)) {
		    if(!Blind) pline("%s is blinded.", Monnam(mdef));
		    mdef->mcansee = 0;
		    mdef->mblinded += tmp;
		}
		tmp = 0;
		break;
	    case AD_CURS:
		if (night() && !rn2(10) && !mdef->mcan) {
		    if (mdef->data == &mons[PM_CLAY_GOLEM]) {
			if (!Blind)
			    pline("Some writing vanishes from %s head!",
				s_suffix(mon_nam(mdef)));
			xkilled(mdef, 0);
			/* Don't return yet; keep hp<1 and tmp=0 for pet msg */
		    } else {
			mdef->mcan = 1;
			You("chuckle.");
		    }
		}
		tmp = 0;
		break;
	    case AD_DRLI:
		if (rn2(2) && !resists_drli(mdef)) {
			int xtmp = d(2,6);
			pline("%s suddenly seems weaker!", Monnam(mdef));
			mdef->mhpmax -= xtmp;
			if ((mdef->mhp -= xtmp) <= 0 || !mdef->m_lev) {
				pline("%s dies!", Monnam(mdef));
				xkilled(mdef,0);
			} else
				mdef->m_lev--;
		}
		tmp = 0;
		break;
	    case AD_RUST:
		if (pd == &mons[PM_IRON_GOLEM]) {
			pline("%s falls to pieces!", Monnam(mdef));
			xkilled(mdef,0);
		}
		tmp = 0;
		break;
	    case AD_DCAY:
		if (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
			pline("%s falls to pieces!", Monnam(mdef));
			xkilled(mdef,0);
		}
		tmp = 0;
		break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!rn2(8)) {
		    Your("%s was poisoned!", mpoisons_subj(&youmonst, mattk));
		    if (resists_poison(mdef))
			pline_The("poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    else {
			if (!rn2(10)) {
			    Your("poison was deadly...");
			    tmp = mdef->mhp;
			} else tmp += rn1(10,6);
		    }
		}
		break;
	    case AD_DRIN:
		if (!has_head(mdef->data)) {
		    pline("%s doesn't seem harmed.", Monnam(mdef));
		    tmp = 0;
		    break;
		}
		if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
		    pline("%s helmet blocks your attack to %s head.",
			  s_suffix(Monnam(mdef)), his[pronoun_gender(mdef)]);
		    break;
		}

		You("eat %s brain!", s_suffix(mon_nam(mdef)));
		if (mindless(mdef->data)) {
		    pline("%s doesn't notice.", Monnam(mdef));
		    break;
		}
		tmp += rnd(10);
		morehungry(-rnd(30)); /* cannot choke */
		if (ABASE(A_INT) < AMAX(A_INT)) {
			ABASE(A_INT) += rnd(4);
			if (ABASE(A_INT) > AMAX(A_INT))
				ABASE(A_INT) = AMAX(A_INT);
			flags.botl = 1;
		}
		exercise(A_WIS, TRUE);
		break;
	    case AD_STCK:
		if (!sticks(mdef->data))
		    u.ustuck = mdef; /* it's now stuck to you */
		break;
	    case AD_WRAP:
		if (!sticks(mdef->data)) {
		    if (!u.ustuck && !rn2(10)) {
			if (m_slips_free(mdef, mattk)) {
			    tmp = 0;
			} else {
			    You("swing yourself around %s!",
				  mon_nam(mdef));
			    u.ustuck = mdef;
			}
		    } else if(u.ustuck == mdef) {
			/* Monsters don't wear amulets of magical breathing */
			if (is_pool(u.ux,u.uy) && !is_swimmer(mdef->data)) {
			    You("drown %s...", mon_nam(mdef));
			    tmp = mdef->mhp;
			} else if(mattk->aatyp == AT_HUGS)
			    pline("%s is being crushed.", Monnam(mdef));
		    } else {
			tmp = 0;
			if(flags.verbose) {
			    struct permonst *pu;

			    pu = uasmon;
			    uasmon = mdef->data;
			    You("brush against %s%ss %s.", mon_nam(mdef),
				canspotmon(mdef) ? "'" : "",
				body_part(LEG));
			    uasmon = pu;
			}
		    }
		} else tmp = 0;
		break;
	    case AD_PLYS:
		if (mdef->mcanmove && !rn2(3) && tmp < mdef->mhp) {
		    if (!Blind) pline("%s is frozen by you!", Monnam(mdef));
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_SLEE:
		if (!mdef->msleep && sleep_monst(mdef, rnd(10), -1)) {
		    if (!Blind)
			pline("%s is put to sleep by you!", Monnam(mdef));
		    slept_monst(mdef);
		}
		break;
	    default:	tmp = 0;
			break;
	}

	if((mdef->mhp -= tmp) < 1) {
	    if (mdef->mtame && !cansee(mdef->mx,mdef->my)) {
		You_feel("embarrassed for a moment.");
		if (tmp) xkilled(mdef, 0); /* !tmp but hp<1: already killed */
	    } else if (!flags.verbose) {
		You("destroy it!");
		if (tmp) xkilled(mdef, 0);
	    } else
		if (tmp) killed(mdef);
	    return(2);
	}
	return(1);
}

static int
explum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register int tmp = d((int)mattk->damn, (int)mattk->damd);

	You("explode!");
	switch(mattk->adtyp) {
	    case AD_BLND:
		if (!resists_blnd(mdef)) {
		    pline("%s is blinded by your flash of light!", Monnam(mdef));
		    mdef->mblinded = min((int)mdef->mblinded + tmp, 127);
		    mdef->mcansee = 0;
		}
		break;
	    case AD_HALU:
		if (haseyes(mdef->data) && mdef->mcansee) {
		    pline("%s is affected by your flash of light!",
			  Monnam(mdef));
		    mdef->mconf = 1;
		}
		break;
	    case AD_COLD:
		if (!resists_cold(mdef)) {
		    pline("%s gets blasted!", Monnam(mdef));
		    mdef->mhp -= tmp;
		    if (mdef->mhp <= 0) {
			 killed(mdef);
			 return(2);
		    }
		} else {
		    shieldeff(mdef->mx, mdef->my);
		    if (is_golem(mdef->data))
			golemeffects(mdef, AD_COLD, tmp);
		    else
			pline_The("blast doesn't seem to affect %s.",
				mon_nam(mdef));
		}
		break;
	    default:
		break;
	}
	return(1);
}

static void
start_engulf(mdef)
struct monst *mdef;
{
	if (!Invisible) {
		map_location(u.ux, u.uy, TRUE);
		tmp_at(DISP_ALWAYS, mon_to_glyph(&youmonst));
		tmp_at(mdef->mx, mdef->my);
	}
	You("engulf %s!", mon_nam(mdef));
	delay_output();
	delay_output();
}

static void
end_engulf()
{
	if (!Invisible) {
		tmp_at(DISP_END, 0);
		newsym(u.ux, u.uy);
	}
}

static int
gulpum(mdef,mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register int tmp;
	register int dam = d((int)mattk->damn, (int)mattk->damd);
	struct obj *otmp;
	/* Not totally the same as for real monsters.  Specifically, these
	 * don't take multiple moves.  (It's just too hard, for too little
	 * result, to program monsters which attack from inside you, which
	 * would be necessary if done accurately.)  Instead, we arbitrarily
	 * kill the monster immediately for AD_DGST and we regurgitate them
	 * after exactly 1 round of attack otherwise.  -KAA
	 */

	if(mdef->data->msize >= MZ_HUGE) return 0;

	if(u.uhunger < 1500 && !u.uswallow) {
	    for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
		(void) snuff_lit(otmp);

	    if(mdef->data->mlet != S_COCKATRICE || resists_ston(&youmonst)) {
#ifdef LINT	/* static char msgbuf[BUFSZ]; */
		char msgbuf[BUFSZ];
#else
		static char msgbuf[BUFSZ];
#endif
		start_engulf(mdef);
		switch(mattk->adtyp) {
		    case AD_DGST:
			/* eating a Rider or its corpse is fatal */
			if (is_rider(mdef->data)) {
			 pline("Unfortunately, digesting any of it is fatal.");
			    end_engulf();
			    Sprintf(msgbuf, "unwisely tried to eat %s",
				    mdef->data->mname);
			    killer = msgbuf;
			    killer_format = NO_KILLER_PREFIX;
			    done(DIED);
			    return 0;		/* lifesaved */
			}
			u.uhunger += mdef->data->cnutrit;
			newuhs(FALSE);
			xkilled(mdef,2);
			Sprintf(msgbuf, "You totally digest %s.",
					mon_nam(mdef));
			if ((tmp = 3 + (mdef->data->cwt >> 6)) != 0) {
			    /* setting afternmv = end_engulf is tempting,
			     * but will cause problems if the player is
			     * attacked (which uses his real location) or
			     * if his See_invisible wears off
			     */
			    You("digest %s.", mon_nam(mdef));
			    nomul(-tmp);
			    nomovemsg = msgbuf;
			} else pline(msgbuf);
			end_engulf();
			exercise(A_CON, TRUE);
			return(2);
		    case AD_PHYS:
			pline("%s is pummeled with your debris!",Monnam(mdef));
			break;
		    case AD_ACID:
			pline("%s is covered with your goo!", Monnam(mdef));
			if (resists_acid(mdef)) {
			    pline("It seems harmless to %s.", mon_nam(mdef));
			    dam = 0;
			}
			break;
		    case AD_BLND:
			if (!resists_blnd(mdef)) {
			    if (mdef->mcansee)
				pline("%s can't see in there!", Monnam(mdef));
			    mdef->mcansee = 0;
			    dam += mdef->mblinded;
			    if (dam > 127) dam = 127;
			    mdef->mblinded = dam;
			}
			dam = 0;
			break;
		    case AD_ELEC:
			if (rn2(2)) {
			    pline_The("air around %s crackles with electricity.", mon_nam(mdef));
			    if (resists_elec(mdef)) {
				pline("%s seems unhurt.", Monnam(mdef));
				dam = 0;
			    }
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_COLD:
			if (rn2(2)) {
			    if (resists_cold(mdef)) {
				pline("%s seems mildly chilly.", Monnam(mdef));
				dam = 0;
			    } else
				pline("%s is freezing to death!",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_FIRE:
			if (rn2(2)) {
			    if (resists_fire(mdef)) {
				pline("%s seems mildly hot.", Monnam(mdef));
				dam = 0;
			    } else
				pline("%s is burning to a crisp!",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		}
		end_engulf();
		if ((mdef->mhp -= dam) <= 0) {
		    killed(mdef);
		    return(2);
		}
		You("%s %s!", is_animal(uasmon) ? "regurgitate"
			: "expel", mon_nam(mdef));
		if (is_animal(uasmon)) {
		    pline("Obviously, you didn't like %s taste.",
			  s_suffix(mon_nam(mdef)));
		}
	    } else {
		You("bite into %s", mon_nam(mdef));
		You("turn to stone...");
		killer_format = KILLED_BY;
		killer = "swallowing a cockatrice whole";
		done(STONING);
	    }
	}
	return(0);
}

void
missum(mdef,mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	if (could_seduce(&youmonst, mdef, mattk))
		You("pretend to be friendly to %s.", mon_nam(mdef));
	else if(canspotmon(mdef) && flags.verbose)
		You("miss %s.", mon_nam(mdef));
	else
		You("miss it.");
	if(!mdef->msleep && mdef->mcanmove)
		wakeup(mdef);
}

static boolean
hmonas(mon, tmp)		/* attack monster as a monster. */
register struct monst *mon;
register int tmp;
{
	register struct attack *mattk;
	int	i, sum[NATTK];
	int	nsum = 0;
	int	dhit = 0;

	for(i = 0; i < NATTK; i++) {

	    sum[i] = 0;
	    mattk = &(uasmon->mattk[i]);
	    switch(mattk->aatyp) {
		case AT_WEAP:
use_weapon:
	/* Certain monsters don't use weapons when encountered as enemies,
	 * but players who polymorph into them have hands or claws and thus
	 * should be able to use weapons.  This shouldn't prohibit the use
	 * of most special abilities, either.
	 */
	/* Potential problem: if the monster gets multiple weapon attacks,
	 * we currently allow the player to get each of these as a weapon
	 * attack.  Is this really desirable?
	 */
			if(uwep) tmp += hitval(uwep, mon);
			dhit = (tmp > (dieroll = rnd(20)) || u.uswallow);
			/* Enemy dead, before any special abilities used */
			if (!known_hitum(mon,&dhit,mattk)) return FALSE;
			/* might be a worm that gets cut in half */
			if (m_at(u.ux+u.dx, u.uy+u.dy) != mon) return((boolean)(nsum != 0));
			/* Do not print "You hit" message, since known_hitum
			 * already did it.
			 */
			if (dhit && mattk->adtyp != AD_SPEL
				&& mattk->adtyp != AD_PHYS)
				sum[i] = damageum(mon,mattk);
			break;
		case AT_CLAW:
			if (i==0 && uwep && !cantwield(uasmon)) goto use_weapon;
#ifdef SEDUCE
			/* succubi/incubi are humanoid, but their _second_
			 * attack is AT_CLAW, not their first...
			 */
			if (i==1 && uwep && (u.umonnum == PM_SUCCUBUS ||
				u.umonnum == PM_INCUBUS)) goto use_weapon;
#endif
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_TUCH:
		case AT_BUTT:
		case AT_TENT:
			if (i==0 && uwep && (u.usym==S_LICH)) goto use_weapon;
			if ((dhit = (tmp > rnd(20) || u.uswallow)) != 0) {
			    int compat;

			    if (!u.uswallow &&
				(compat=could_seduce(&youmonst, mon, mattk))) {
				You("%s %s %s.",
				    mon->mcansee && haseyes(mon->data)
				    ? "smile at" : "talk to",
				    mon_nam(mon),
				    compat == 2 ? "engagingly":"seductively");
				/* doesn't anger it; no wakeup() */
				sum[i] = damageum(mon, mattk);
				break;
			    }
			    wakeup(mon);
			    if (mon->data == &mons[PM_SHADE]) {
				Your("attack passes harmlessly through %s.",
				    mon_nam(mon));
				break;
			    }
			    if (mattk->aatyp == AT_KICK)
				    You("kick %s.", mon_nam(mon));
			    else if (mattk->aatyp == AT_BITE)
				    You("bite %s.", mon_nam(mon));
			    else if (mattk->aatyp == AT_STNG)
				    You("sting %s.", mon_nam(mon));
			    else if (mattk->aatyp == AT_BUTT)
				    You("butt %s.", mon_nam(mon));
			    else if (mattk->aatyp == AT_TUCH)
				    You("touch %s.", mon_nam(mon));
			    else if (mattk->aatyp == AT_TENT)
				    Your("tentacles suck %s.", mon_nam(mon));
			    else You("hit %s.", mon_nam(mon));
			    sum[i] = damageum(mon, mattk);
			} else
			    missum(mon, mattk);
			break;

		case AT_HUGS:
			/* automatic if prev two attacks succeed, or if
			 * already grabbed in a previous attack
			 */
			dhit = 1;
			wakeup(mon);
			if (mon->data == &mons[PM_SHADE])
			    Your("hug passes harmlessly through %s.",
				mon_nam(mon));
			else if (!sticks(mon->data) && !u.uswallow)
			    if (mon==u.ustuck) {
				pline("%s is being %s.", Monnam(mon),
				    u.umonnum==PM_ROPE_GOLEM ? "choked":
				    "crushed");
				sum[i] = damageum(mon, mattk);
			    } else if(i >= 2 && sum[i-1] && sum[i-2]) {
				You("grab %s!", mon_nam(mon));
				u.ustuck = mon;
				sum[i] = damageum(mon, mattk);
			    }
			break;

		case AT_EXPL:	/* automatic hit if next to */
			dhit = -1;
			wakeup(mon);
			sum[i] = explum(mon, mattk);
			break;

		case AT_ENGL:
			if((dhit = (tmp > rnd(20+i)))) {
				wakeup(mon);
				if (mon->data == &mons[PM_SHADE])
				    Your("attempt to surround %s is harmless.",
					mon_nam(mon));
				else
				    sum[i]= gulpum(mon,mattk);
			} else
				missum(mon, mattk);
			break;

		case AT_MAGC:
			/* No check for uwep; if wielding nothing we want to
			 * do the normal 1-2 points bare hand damage...
			 */
			if (i==0 && (u.usym==S_KOBOLD
				|| u.usym==S_ORC
				|| u.usym==S_GNOME
				)) goto use_weapon;

		case AT_NONE:
			continue;
			/* Not break--avoid passive attacks from enemy */

		case AT_BREA:
		case AT_SPIT:
		case AT_GAZE:	/* all done using #monster command */
			dhit = 0;
			break;

		default: /* Strange... */
			impossible("strange attack of yours (%d)",
				 mattk->aatyp);
	    }
	    if (dhit == -1)
		rehumanize();
	    if (sum[i] == 2)
		return((boolean)passive(mon, 1, 0, (mattk->aatyp == AT_KICK)));
							/* defender dead */
	    else {
		(void) passive(mon, sum[i], 1, (mattk->aatyp==AT_KICK));
		nsum |= sum[i];
	    }
	    if (uasmon == &playermon)
		break; /* No extra attacks if no longer a monster */
	    if (multi < 0)
		break; /* If paralyzed while attacking, i.e. floating eye */
	}
	return((boolean)(nsum != 0));
}

/*	Special (passive) attacks on you by monsters done here.		*/

int
passive(mon, mhit, malive, kicked)
register struct monst *mon;
register boolean mhit;
register int malive;
boolean kicked;
{
	register struct permonst *ptr = mon->data;
	register int i, tmp;

	for(i = 0; ; i++) {
	    if(i >= NATTK) return(malive | mhit);	/* no passive attacks */
	    if(ptr->mattk[i].aatyp == AT_NONE) break;	/* try this one */
	}
	/* Note: tmp not always used */
	if (ptr->mattk[i].damn)
	    tmp = d((int)ptr->mattk[i].damn, (int)ptr->mattk[i].damd);
	else if(ptr->mattk[i].damd)
	    tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
	else
	    tmp = 0;

/*	These affect you even if they just died */

	switch(ptr->mattk[i].adtyp) {

	  case AD_ACID:
	    if(mhit && rn2(2)) {
		if (Blind || !flags.verbose) You("are splashed!");
		else	You("are splashed by %s acid!",
			                s_suffix(mon_nam(mon)));

		if (!resists_acid(&youmonst))
			mdamageu(mon, tmp);
		if(!rn2(30)) erode_armor(TRUE);
	    }
	    if(mhit && !rn2(6)) {
		if (kicked) {
		    if (uarmf)
			(void) rust_dmg(uarmf, xname(uarmf), 3, TRUE);
		} else erode_weapon(TRUE);
	    }
	    exercise(A_STR, FALSE);
	    break;
	  case AD_STON:
	    if(mhit)
	      if ((kicked && !uarmf) || (!kicked && !uwep && !uarmg)) {
		if (!resists_ston(&youmonst) &&
		    !(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))) {
			You("turn to stone...");
			done_in_by(mon);
			return 2;
		}
	      }
	    break;
	  case AD_RUST:
	    if(mhit && !mon->mcan)
	      if (kicked) {
		if (uarmf)
		    (void) rust_dmg(uarmf, xname(uarmf), 1, TRUE);
	      } else
		erode_weapon(FALSE);
	    break;
	  case AD_MAGM:
	    /* wrath of gods for attacking Oracle */
	    if(Antimagic) {
		shieldeff(u.ux, u.uy);
		pline("A hail of magic missiles narrowly misses you!");
	    } else {
		You("are hit by magic missiles appearing from thin air!");
		mdamageu(mon, tmp);
	    }
	    break;
	  default:
	    break;
	}

/*	These only affect you if they still live */

	if(malive && !mon->mcan && rn2(3)) {

	    switch(ptr->mattk[i].adtyp) {

	      case AD_PLYS:
		if(ptr == &mons[PM_FLOATING_EYE]) {
		    if (!canseemon(mon)) {
			break;
		    }
		    if(mon->mcansee) {
			if(Reflecting & W_AMUL) {
			    makeknown(AMULET_OF_REFLECTION);
			    pline("%s gaze is reflected by your medallion.",
				  s_suffix(Monnam(mon)));
			} else if(Reflecting & W_ARMS) {
			    makeknown(SHIELD_OF_REFLECTION);
			    pline("%s gaze is reflected by your shield.",
				  s_suffix(Monnam(mon)));
			} else {
			    You("are frozen by %s gaze!",
				  s_suffix(mon_nam(mon)));
			    nomul((ACURR(A_WIS) > 12 || rn2(4)) ? -tmp : -127);
			}
		    } else {
			pline("%s cannot defend itself.",
				Adjmonnam(mon,"blind"));
			if(!rn2(500)) change_luck(-1);
		    }
		} else { /* gelatinous cube */
		    You("are frozen by %s!", mon_nam(mon));
		    nomul(-tmp);
		    exercise(A_DEX, FALSE);
		}
		break;
	      case AD_COLD:		/* brown mold or blue jelly */
		if(monnear(mon, u.ux, u.uy)) {
		    if(Cold_resistance) {
			shieldeff(u.ux, u.uy);
			You_feel("a mild chill.");
			ugolemeffects(AD_COLD, tmp);
			break;
		    }
		    You("are suddenly very cold!");
		    mdamageu(mon, tmp);
		/* monster gets stronger with your heat! */
		    mon->mhp += tmp / 2;
		    if (mon->mhpmax < mon->mhp) mon->mhpmax = mon->mhp;
		/* at a certain point, the monster will reproduce! */
		    if(mon->mhpmax > ((int) (mon->m_lev+1) * 8)) {
			register struct monst *mtmp;

			if ((mtmp = clone_mon(mon)) != 0) {
			    mtmp->mhpmax = mon->mhpmax /= 2;
			    if(!Blind)
				pline("%s multiplies from your heat!",
								Monnam(mon));
			}
		    }
		}
		break;
	      case AD_STUN:		/* specifically yellow mold */
		if(!Stunned)
		    make_stunned((long)tmp, TRUE);
		break;
	      case AD_FIRE:
		if(monnear(mon, u.ux, u.uy)) {
		    if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			You_feel("mildly warm.");
			ugolemeffects(AD_FIRE, tmp);
			break;
		    }
		    You("are suddenly very hot!");
		    mdamageu(mon, tmp);
		}
		break;
	      case AD_ELEC:
		if(Shock_resistance) {
		    shieldeff(u.ux, u.uy);
		    You_feel("a mild tingle.");
		    ugolemeffects(AD_ELEC, tmp);
		    break;
		}
		You("are jolted with electricity!");
		mdamageu(mon, tmp);
		break;
	      default:
		break;
	    }
	}
	return(malive | mhit);
}

/* Note: caller must ascertain mtmp is mimicing... */
void
stumble_onto_mimic(mtmp)
register struct monst *mtmp;
{
	if(!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data,AD_STCK))
	    u.ustuck = mtmp;
	if (Blind) {
	    if(!Telepat)
		pline("Wait!  That's a monster!");
	} else if (glyph_is_cmap(levl[u.ux+u.dx][u.uy+u.dy].glyph) &&
		(glyph_to_cmap(levl[u.ux+u.dx][u.uy+u.dy].glyph) == S_hcdoor ||
		 glyph_to_cmap(levl[u.ux+u.dx][u.uy+u.dy].glyph) == S_vcdoor))
	    pline_The("door actually was %s!", a_monnam(mtmp));
	else if (glyph_is_object(levl[u.ux+u.dx][u.uy+u.dy].glyph) &&
		glyph_to_obj(levl[u.ux+u.dx][u.uy+u.dy].glyph) == GOLD_PIECE)
	    pline("That gold was %s!", a_monnam(mtmp));
	else {
	    pline("Wait!  That's %s!", a_monnam(mtmp));
	}

	wakeup(mtmp);	/* clears mimicing */
}

static void
nohandglow(mon)
struct monst *mon;
{
	char *hands=makeplural(body_part(HAND));

	if (!u.umconf || mon->mconf) return;
	if (u.umconf == 1) {
		if (Blind)
			Your("%s stop tingling.", hands);
		else
			Your("%s stop glowing %s.", hands, hcolor(red));
	} else {
		if (Blind)
			pline_The("tingling in your %s lessens.", hands);
		else
			Your("%s no longer glow so brightly %s.", hands,
				hcolor(red));
	}
	u.umconf--;
}

int flash_hits_mon(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;	/* source of flash */
{
	int tmp, amt, res = 0, useeit = canseemon(mtmp);

	if (mtmp->msleep) {
	    mtmp->msleep = 0;
	    if (useeit) {
		pline_The("flash awakens %s.", mon_nam(mtmp));
		res = 1;
	    }
	} else if (mtmp->data->mlet != S_LIGHT) {
	    if (!resists_blnd(mtmp)) {
		tmp = dist2(otmp->ox, otmp->oy, mtmp->mx, mtmp->my);
		if (useeit) {
		    pline("%s is blinded by the flash!", Monnam(mtmp));
		    res = 1;
		}
		if (mtmp->data == &mons[PM_GREMLIN]) {
		    /* Rule #1: Keep them out of the light. */
		    amt = otmp->otyp == WAN_LIGHT ? d(1 + otmp->spe, 4) :
		          rn2(min(mtmp->mhp,4));
		    pline("%s %s!", Monnam(mtmp), amt > mtmp->mhp / 2 ?
			  "wails in agony" : "cries out in pain");
		    if ((mtmp->mhp -= amt) <= 0) {
			if (flags.mon_moving)
			    monkilled(mtmp, (char *)0, AD_BLND);
			else
			    killed(mtmp);
		    }
		}
		if (mtmp->mhp > 0) {
		    if (!flags.mon_moving) setmangry(mtmp);
		    if (tmp < 9 && !mtmp->isshk && rn2(4)) {
			mtmp->mflee = 1;
			if (rn2(4)) mtmp->mfleetim = rnd(100);
		    }
		    mtmp->mcansee = 0;
		    mtmp->mblinded = (tmp < 3) ? 0 : rnd(1 + 50/tmp);
		}
	    }
	}
	return res;
}

/*uhitm.c*/
