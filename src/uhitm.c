/*	SCCS Id: @(#)uhitm.c	3.1	92/12/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static boolean FDECL(known_hitum, (struct monst *,int));
static boolean FDECL(hitum, (struct monst *,int));
#ifdef POLYSELF
static int FDECL(explum, (struct monst *,struct attack *));
static int FDECL(gulpum, (struct monst *,struct attack *));
static boolean FDECL(hmonas, (struct monst *,int));
#endif
static void FDECL(nohandglow, (struct monst *));

extern boolean notonhead;	/* for long worms */
/* The below might become a parameter instead if we use it a lot */
static int dieroll;

struct monst *
clone_mon(mon)
struct monst *mon;
{
	coord mm;
	struct monst *m2;

	mm.x = mon->mx;
	mm.y = mon->my;
	if (!enexto(&mm, mm.x, mm.y, mon->data)) return (struct monst *)0;
	if (MON_AT(mm.x, mm.y) || mon->mhp <= 1) return (struct monst *)0;
	/* may have been extinguished for population control */
	if(mon->data->geno & G_EXTINCT) return((struct monst *) 0);
	m2 = newmonst(0);
	*m2 = *mon;			/* copy condition of old monster */
	m2->nmon = fmon;
	fmon = m2;
	m2->m_id = flags.ident++;
	m2->mx = mm.x;
	m2->my = mm.y;

	m2->minvent = (struct obj *) 0; /* objects don't clone */
	m2->mleashed = FALSE;
	m2->mgold = 0L;
	/* Max HP the same, but current HP halved for both.  The caller
	 * might want to override this by halving the max HP also.
	 */
	m2->mhpmax = mon->mhpmax;
	m2->mhp = mon->mhp /= 2;

	/* since shopkeepers and guards will only be cloned if they've been
	 * polymorphed away from their original forms, the clone doesn't have
	 * room for the extra information.  we also don't want two shopkeepers
	 * around for the same shop.
	 * similarly, clones of named monsters don't have room for the name,
	 * so we just make the clone unnamed instead of bothering to create
	 * a clone with room and copying over the name from the right place
	 * (which changes if the original was a shopkeeper or guard).
	 */
	if (mon->isshk) m2->isshk = FALSE;
	if (mon->isgd) m2->isgd = FALSE;
	if (mon->ispriest) m2->ispriest = FALSE;
	m2->mxlth = 0;
	m2->mnamelth = 0;
	place_monster(m2, m2->mx, m2->my);
	newsym(m2->mx,m2->my);	/* display the new monster */
	if (mon->mtame) {
	    struct monst *m3;

	    /* because m2 is a copy of mon it is tame but not init'ed.
	     * however, tamedog will not re-tame a tame dog, so m2
	     * must be made non-tame to get initialized properly.
	     */
	    m2->mtame = 0;
	    if ((m3 = tamedog(m2, (struct obj *)0)) != 0)
		m2 = m3;
	}
	return m2;
}

boolean
special_case(mtmp)
/* Moved this code from attack() in order to 	*/
/* avoid having to duplicate it in dokick.	*/
register struct monst *mtmp;
{
	char qbuf[QBUFSZ];

	if(mtmp->m_ap_type && !Protection_from_shape_changers
						&& !sensemon(mtmp)) {
		stumble_onto_mimic(mtmp);
		mtmp->data->mflags3 &= ~M3_WAITMASK;
		return(1);
	}

	if(mtmp->mundetected && hides_under(mtmp->data) && !canseemon(mtmp)) {
		mtmp->mundetected = 0;
		if (!(Blind ? Telepat : (HTelepat & (W_ARMH|W_AMUL|W_ART)))) {
			register struct obj *obj;

			if(Blind)
			    pline("Wait!  There's a hidden monster there!");
			else if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0)
			    pline("Wait!  There's %s hiding under %s!",
					an(l_monnam(mtmp)), doname(obj));
			wakeup(mtmp);
			mtmp->data->mflags3 &= ~M3_WAITMASK;
			return(TRUE);
		}
	}

	if (flags.confirm && mtmp->mpeaceful
	    && !Confusion && !Hallucination && !Stunned) {
		/* Intelligent chaotic weapons (Stormbringer) want blood */
		if (uwep && uwep->oartifact == ART_STORMBRINGER)
			return(FALSE);

		if (canspotmon(mtmp)) {
			Sprintf(qbuf, "Really attack %s?", mon_nam(mtmp));
			if (yn(qbuf) != 'y') {
				flags.move = 0;
				mtmp->data->mflags3 &= ~M3_WAITMASK;
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
	struct permonst *mdat = mtmp->data;

	tmp = 1 + Luck + abon() +
		find_mac(mtmp) +
#ifdef POLYSELF
		((u.umonnum >= 0) ? uasmon->mlevel : u.ulevel);
#else
		u.ulevel;
#endif

/*	it is unchivalrous to attack the defenseless or from behind */
	if (pl_character[0] == 'K' && u.ualign.type == A_LAWFUL &&
	    (!mtmp->mcanmove || mtmp->msleep || mtmp->mflee) &&
	    u.ualign.record > -10) adjalign(-1);

/*	attacking peaceful creatures is bad for the samurai's giri */
	if (pl_character[0] == 'S' && mtmp->mpeaceful &&
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
	if (is_orc(mtmp->data) && pl_character[0]=='E') tmp++;

/*	with a lot of luggage, your agility diminishes */
	if(tmp2 = near_capacity()) tmp -= (tmp2*2) - 1;
	if(u.utrap) tmp -= 3;
#ifdef POLYSELF
/*	Some monsters have a combination of weapon attacks and non-weapon
 *	attacks.  It is therefore wrong to add hitval to tmp; we must add it
 *	only for the specific attack (in hmonas()).
 */
	if(uwep && u.umonnum == -1) tmp += hitval(uwep, mdat);
#else
	if(uwep) tmp += hitval(uwep, mdat);
#endif
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
	if (is_safepet(mtmp) &&
	    (!uwep || uwep->oartifact != ART_STORMBRINGER)) {
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
#ifdef POLYSELF
			|| (IS_ROCK(levl[u.ux][u.uy].typ) &&
					!passes_walls(mtmp->data))
#endif
			) {
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

	/* moved code to a separate function to share with dokick */
	if(special_case(mtmp)) return(TRUE);

#ifdef POLYSELF
	if(u.umonnum >= 0) {	/* certain "pacifist" monsters don't attack */
		set_uasmon();
		if(noattacks(uasmon)) {
			You("have no way to attack monsters physically.");
			mtmp->data->mflags3 &= ~M3_WAITMASK;
			return(TRUE);
		}
	}
#endif

	if(check_capacity("You cannot fight while so heavily loaded."))
	    return (TRUE);

	if(unweapon) {
	    unweapon=FALSE;
	    if(flags.verbose)
		if(uwep)
		    You("begin bashing monsters with your %s.",
			aobjnam(uwep, NULL));
		else
#ifdef POLYSELF
		    if (!cantwield(uasmon))
#endif
		    You("begin bashing monsters with your %s hands.",
			uarmg ? "gloved" : "bare");		/* Del Lamb */
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
#ifdef POLYSELF
	if (u.umonnum >= 0) (void) hmonas(mtmp, tmp);
	else
#endif
	    (void) hitum(mtmp, tmp);

	mtmp->data->mflags3 &= ~M3_WAITMASK;
	return(TRUE);
}

static boolean
known_hitum(mon, mhit)	/* returns TRUE if monster still lives */
/* Made into a separate function because in some cases we want to know
 * in the calling function whether we hit.
 */
register struct monst *mon;
register int mhit;
{
	register boolean malive = TRUE, special;

	/* we need to know whether the special monster was peaceful */
	/* before the attack, to save idle calls to angry_guards()  */
	special = (mon->mpeaceful && (mon->data == &mons[PM_WATCHMAN] ||
				mon->data == &mons[PM_WATCH_CAPTAIN] ||
				      mon->ispriest || mon->isshk));

	if(!mhit) {
	    if(flags.verbose) You("miss %s.", mon_nam(mon));
	    else			You("miss it.");
	    if(!mon->msleep && mon->mcanmove)
		wakeup(mon);
#ifdef MUSE
	    else if (uwep && uwep->otyp == TSURUGI &&
		     MON_WEP(mon) && !rn2(20)) {
		/* 1/20 chance of shattering defender's weapon */
		struct obj *obj = MON_WEP(mon);

		MON_NOWEP(mon);
		m_useup(mon, obj);
		pline("%s weapon shatters!", s_suffix(Monnam(mon)));
		/* perhaps this will freak out the monster */
		if (!rn2(3)) {
		    mon->mflee = 1;
		    mon->mfleetim += rnd(20);
		}
	    }
#endif
	} else {
	    /* we hit the monster; be careful: it might die! */
	    notonhead = (mon->mx != u.ux+u.dx || mon->my != u.uy+u.dy);
	    if((malive = hmon(mon, uwep, 0)) == TRUE) {
		/* monster still alive */
		if(!rn2(25) && mon->mhp < mon->mhpmax/2) {
			mon->mflee = 1;
			if(!rn2(3)) mon->mfleetim = rnd(100);
			if(u.ustuck == mon && !u.uswallow
#ifdef POLYSELF
						&& !sticks(uasmon)
#endif
								)
				u.ustuck = 0;
		}
		if (mon->wormno) cutworm(mon, u.ux+u.dx, u.uy+u.dy, uwep);
	    }
	    if(mon->ispriest && !rn2(2)) ghod_hitsu(mon);
	    if(special) (void) angry_guards(!flags.soundok);
	}
	return(malive);
}

static boolean
hitum(mon, tmp)		/* returns TRUE if monster still lives */
struct monst *mon;
int tmp;
{
	static int NEARDATA malive;
	boolean mhit = (tmp > (dieroll = rnd(20)) || u.uswallow);

	if(tmp > dieroll) exercise(A_DEX, TRUE);
	malive = known_hitum(mon, mhit);
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

	wakeup(mon);
	if(!obj) {	/* attack with bare hands */
	    if (mdat == &mons[PM_SHADE])
		tmp = 0;
	    else
		tmp = rnd(2);
#if 0
	    if(mdat == &mons[PM_COCKATRICE] && !uarmg
#ifdef POLYSELF
		&& !resists_ston(uasmon)
#endif
		) {

		You("hit %s with your bare %s.",
			mon_nam(mon), makeplural(body_part(HAND)));
# ifdef POLYSELF
		if(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))
		    return TRUE;
# endif
		You("turn to stone...");
		done_in_by(mon);
		hittxt = TRUE; /* maybe lifesaved */
	    }
#endif
	} else {
	    if(obj->oclass == WEAPON_CLASS || obj->otyp == PICK_AXE ||
	       obj->otyp == UNICORN_HORN || obj->oclass == ROCK_CLASS) {

		/* If not a melee weapon, and either not thrown, or thrown */
		/* and a bow (bows are >BOOMERANG), or thrown and a missile */
		/* without a propellor (missiles are <DART), do 1-2 points */
		if((obj->otyp >= BOW || obj->otyp < DART)
			&& obj->otyp != PICK_AXE && obj->otyp != UNICORN_HORN
			&& (!thrown ||
			    (obj->oclass != ROCK_CLASS &&
			    (obj->otyp > BOOMERANG ||
				(obj->otyp < DART &&
				    (!uwep ||
				    objects[obj->otyp].w_propellor !=
				    -objects[uwep->otyp].w_propellor)
				))))) {
		    if (mdat == &mons[PM_SHADE] && obj->otyp != SILVER_ARROW)
			tmp = 0;
		    else
			tmp = rnd(2);
		} else {
		    tmp = dmgval(obj, mdat);
		    if (obj->oartifact &&
			artifact_hit(&youmonst, mon, obj, &tmp, dieroll)) {
			if(mon->mhp <= 0) /* artifact killed monster */
			    return FALSE;
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
			      (obj->otyp >= ARROW && obj->otyp <= SHURIKEN)) {
			if(uwep && obj->otyp < DART &&
			   objects[obj->otyp].w_propellor ==
			   -objects[uwep->otyp].w_propellor) {
			    /* Elves and Samurai do extra damage using
			     * their bows&arrows; they're highly trained.
			     */
			    if (pl_character[0] == 'S' &&
				obj->otyp == YA && uwep->otyp == YUMI)
				tmp++;
			    else if (pl_character[0] == 'E' &&
				     obj->otyp == ELVEN_ARROW &&
				     uwep->otyp == ELVEN_BOW)
				tmp++;
			}
			if(((uwep && objects[obj->otyp].w_propellor ==
				-objects[uwep->otyp].w_propellor)
				|| obj->otyp==DART || obj->otyp==SHURIKEN) &&
				obj->opoisoned)
			    ispoisoned = TRUE;
		    }
		}
	    } else if(obj->oclass == POTION_CLASS) {
			if (obj->quan > 1L) setuwep(splitobj(obj, 1L));
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
			if(obj->corpsenm == PM_COCKATRICE) {
			    You("hit %s with the cockatrice corpse.",
				  mon_nam(mon));
			    if(resists_ston(mdat)) {
				tmp = 1;
				hittxt = TRUE;
				break;
			    }
			    if(poly_when_stoned(mdat)) {
				mon_to_stone(mon);
				tmp = 1;
				hittxt = TRUE;
				break;
			    }
			    pline("%s turns to stone.", Monnam(mon));
			    stoned = TRUE;
			    xkilled(mon,0);
			    return(FALSE);
			}
			tmp = mons[obj->corpsenm].msize + 1;
			break;
		    case EGG: /* only possible if hand-to-hand */
			if(obj->corpsenm > -1
					&& obj->corpsenm != PM_COCKATRICE
					&& mdat == &mons[PM_COCKATRICE]) {
				You("hit %s with the %s egg%s.",
					mon_nam(mon),
					mons[obj->corpsenm].mname,
					plur(obj->quan));
				hittxt = TRUE;
				pline("The egg%sn't live any more...",
					(obj->quan == 1L) ? " is" : "s are");
				obj->otyp = ROCK;
				obj->oclass = GEM_CLASS;
				obj->known = obj->dknown = 0;
				obj->owt = weight(obj);
			}
			tmp = 1;
			break;
		    case CLOVE_OF_GARLIC:	/* no effect against demons */
			if(is_undead(mdat)) mon->mflee = 1;
			tmp = 1;
			break;
		    case CREAM_PIE:
#ifdef POLYSELF
		    case BLINDING_VENOM:
			if(Blind || !haseyes(mon->data))
			    pline(obj->otyp==CREAM_PIE ? "Splat!" : "Splash!");
			else if (obj->otyp == BLINDING_VENOM)
			    pline("The venom blinds %s%s!", mon_nam(mon),
					mon->mcansee ? "" : " further");
#else
			if(Blind) pline("Splat!");
#endif
			else
			    pline("The cream pie splashes over %s%s!",
				mon_nam(mon),
				(haseyes(mdat) &&
				    mdat != &mons[PM_FLOATING_EYE])
				? (*(eos(mon_nam(mon))-1) == 's' ? "' face" :
					 "'s face") : "");
			if(mon->msleep) mon->msleep = 0;
			setmangry(mon);
			if(haseyes(mon->data)) {
			    mon->mcansee = 0;
			    tmp = rn1(25, 21);
			    if(((int) mon->mblinded + tmp) > 127)
				mon->mblinded = 127;
			    else mon->mblinded += tmp;
			}
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			tmp = 0;
			break;
#ifdef POLYSELF
		    case ACID_VENOM: /* only possible if thrown */
			if(resists_acid(mdat)) {
				Your("venom hits %s harmlessly.",
					mon_nam(mon));
				tmp = 0;
			} else {
				Your("venom burns %s!", mon_nam(mon));
				tmp = dmgval(obj, mdat);
			}
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			break;
#endif
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

	if (get_dmg_bonus && tmp) {
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

/* TODO:	Fix this up.  multiple engulf attacks now exist.
	if(u.uswallow) {
	    if((tmp -= u.uswldtim) <= 0) {
		Your("%s are no longer able to hit.",
			makeplural(body_part(ARM)));
		return(TRUE);
	    }
	}
 */
	if (ispoisoned) {
	    if(resists_poison(mdat))
		needpoismsg = TRUE;
	    else if (rn2(10))
		tmp += rnd(6);
	    else poiskilled = TRUE;
	}
	if(tmp < 1)
	    if (mdat == &mons[PM_SHADE]) {
		Your("attack passes harmlessly through %s.",
			mon_nam(mon));
		hittxt = TRUE;
	    } else
		tmp = 1;

	mon->mhp -= tmp;
	if(mon->mhp < 1)
		destroyed = TRUE;
	if(mon->mtame && (!mon->mflee || mon->mfleetim)) {
#ifdef SOUNDS
		if (rn2(8)) yelp(mon);
		else growl(mon); /* give them a moment's worry */
#endif
		mon->mtame--;
		if(!mon->mtame) newsym(mon->mx, mon->my);
		mon->mflee = TRUE;		/* Rick Richardson */
		mon->mfleetim += 10*rnd(tmp);
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
		else	You("hit %s%s", mon_nam(mon), canseemon(mon)
			? exclam(tmp) : ".");
	}

	if (silvermsg) {
		if (canseemon(mon) || sensemon(mon))
			pline("The silver sears %s%s!",
				mon_nam(mon),
				noncorporeal(mdat) ? "" : 
			          (*(eos(mon_nam(mon))-1) == 's' ?
				       "' flesh" : "'s flesh"));
		else
			pline("It%s is seared!",
				noncorporeal(mdat) ? "" : "s flesh");
	}

	if (needpoismsg)
		pline("The poison doesn't seem to affect %s.", mon_nam(mon));
	if (poiskilled) {
		pline("The poison was deadly...");
		xkilled(mon, 0);
		return FALSE;
	} else if (destroyed) {
		killed(mon);	/* takes care of most messages */
	} else if(u.umconf && !thrown) {
		nohandglow(mon);
		if(!mon->mconf && !resist(mon, '+', 0, NOTELL)) {
			mon->mconf = 1;
			if(!mon->mstun && mon->mcanmove && !mon->msleep &&
			   !Blind)
				pline("%s appears confused.", Monnam(mon));
		}
	}

#if 0
	if(mdat == &mons[PM_RUST_MONSTER] && obj && obj == uwep &&
		is_rustprone(obj) && obj->oeroded < MAX_ERODE) {
	    if (obj->greased)
		grease_protect(obj,NULL,FALSE);
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

	return(destroyed ? FALSE : TRUE);
}

#ifdef POLYSELF

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
	    struct monst *dtmp;
	    pline("Some hell-p has arrived!");
	    if((dtmp = makemon(!rn2(6) ? &mons[ndemon()] : uasmon, u.ux, u.uy)))
		(void)tamedog(dtmp, (struct obj *)0);
	    exercise(A_WIS, TRUE);
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
		if(!Blind) pline("%s is on fire!", Monnam(mdef));
		tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if(resists_fire(pd)) {
		    if (!Blind)
			pline("The fire doesn't heat %s!", mon_nam(mdef));
		    golemeffects(mdef, AD_FIRE, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
	    case AD_COLD:
		if(!Blind) pline("%s is covered in frost!", Monnam(mdef));
		if(resists_cold(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    if (!Blind)
			pline("The frost doesn't chill %s!", mon_nam(mdef));
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
	    case AD_ELEC:
		if (!Blind) pline("%s is zapped!", Monnam(mdef));
		tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if(resists_elec(pd)) {
		    if (!Blind)
			pline("The zap doesn't shock %s!", mon_nam(mdef));
		    golemeffects(mdef, AD_ELEC, tmp);
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
	    case AD_ACID:
		if(resists_acid(pd)) tmp = 0;
		break;
	    case AD_STON:
		if(poly_when_stoned(pd))
		   mon_to_stone(mdef);
		else if(!resists_ston(pd)) {
		    stoned = TRUE;
		    if(!Blind) pline("%s turns to stone.", Monnam(mdef));
		    xkilled(mdef, 0);
		    return(2);
		}
		tmp = 0;	/* no damage if this fails */
		break;
# ifdef SEDUCE
	    case AD_SSEX:
# endif
	    case AD_SEDU:
	    case AD_SITM:
		if(mdef->minvent) {
		    struct obj *otmp, *stealoid;

		    stealoid = (struct obj *)0;
		/* Without MUSE we can only change a monster's AC by stealing
		 * armor with the "unarmored soldier" kludge.  With it there
		 * are many monsters which wear armor, and all can be stripped.
		 */
		    if(
#ifndef MUSE
			is_mercenary(pd) &&
#endif
					could_seduce(&youmonst,mdef,mattk)){
			for(otmp = mdef->minvent; otmp; otmp=otmp->nobj)
#ifdef MUSE
			    if (otmp->owornmask & W_ARM) stealoid = otmp;
#else
			    if (otmp->otyp >= PLATE_MAIL && otmp->otyp
				<= ELVEN_CLOAK) stealoid = otmp;
#endif
		    }
		    if (stealoid) {
			boolean stolen = FALSE;
			/* Is "he"/"his" always correct? */
			if (gender(mdef) == u.mfemale &&
						uasmon->mlet == S_NYMPH)
	You("charm %s.  She gladly hands over her possessions.", mon_nam(mdef));
			else
		You("seduce %s and %s starts to take off %s clothes.",
				mon_nam(mdef),
				gender(mdef) ? "she" : "he",
				gender(mdef) ? "her" : "his");
			while(mdef->minvent) {
				otmp = mdef->minvent;
				mdef->minvent = otmp->nobj;
				/* set dknown to insure proper merge */
				if (!Blind) otmp->dknown = 1;
#ifdef MUSE
				otmp->owornmask = 0L;
#endif
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
			}
			if (!stolen)
				impossible("Player steal fails!");
			else {
				pline("%s finishes taking off %s suit.",
				   Monnam(mdef), gender(mdef) ? "her" : "his");
				You("steal %s!", doname(stealoid));
# if defined(ARMY) && !defined(MUSE)
				mdef->data = &mons[PM_UNARMORED_SOLDIER];
# endif
			}
#ifdef MUSE
			possibly_unwield(mdef);
			mdef->misc_worn_check = 0L;
#endif
		   } else {
			otmp = mdef->minvent;
			mdef->minvent = otmp->nobj;
			otmp = hold_another_object(otmp, "You steal %s.",
						  doname(otmp), "You steal: ");
#ifdef MUSE
			possibly_unwield(mdef);
			otmp->owornmask = 0L;
#endif
		   }
		}
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
		if(haseyes(pd)) {

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
			return 2;
		    }
		    mdef->mcan = 1;
		    You("chuckle.");
		}
		tmp = 0;
		break;
	    case AD_DRLI:
		if(rn2(2) && !resists_drli(pd)) {
			int xtmp = d(2,6);
			pline("%s suddenly seems weaker!", Monnam(mdef));
			mdef->mhpmax -= xtmp;
			if ((mdef->mhp -= xtmp) <= 0 || !mdef->m_lev--) {
				pline("%s dies!", Monnam(mdef));
				xkilled(mdef,0);
				return(2);
			}
		}
		tmp = 0;
		break;
	    case AD_RUST:
		if (pd == &mons[PM_IRON_GOLEM]) {
			pline("%s falls to pieces!", Monnam(mdef));
			xkilled(mdef,0);
			return(2);
		}
		tmp = 0;
		break;
	    case AD_DCAY:
		if (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
			pline("%s falls to pieces!", Monnam(mdef));
			xkilled(mdef,0);
			return(2);
		}
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!rn2(8)) {
		    Your("%s was poisoned!", mattk->aatyp==AT_BITE ?
			"bite" : "sting");
		    if (resists_poison(mdef->data))
			pline("The poison doesn't seem to affect %s.",
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
#ifdef MUSE
		if ((mdef->misc_worn_check & W_ARMH) && rn2(8)) {
		    pline("%s helmet blocks your attack to %s head.",
			  s_suffix(Monnam(mdef)),
			  (Blind || !humanoid(mdef->data)) ? "its" :
				(mdef->female ? "her" : "his"));
		    break;
		}
#endif
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
	    case AD_WRAP:
	    case AD_STCK:
		if (!sticks(mdef->data))
		    u.ustuck = mdef; /* it's now stuck to you */
		break;
	    case AD_PLYS:
		if (mdef->mcanmove && !rn2(3) && tmp < mdef->mhp) {
		    if (!Blind) pline("%s is frozen by you!", Monnam(mdef));
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    case AD_SLEE:
		if (!resists_sleep(mdef->data) && !mdef->msleep &&
							mdef->mcanmove) {
		    if (!Blind)
			pline("%s suddenly falls asleep!", Monnam(mdef));
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		}
		break;
	    default:	tmp = 0;
			break;
	}
	if(!tmp) return(1);

	if((mdef->mhp -= tmp) < 1) {

	    if (mdef->mtame && !cansee(mdef->mx,mdef->my)) {
		You("feel embarrassed for a moment.");
		xkilled(mdef, 0);
	    } else if (!flags.verbose) {
		You("destroy it!");
		xkilled(mdef, 0);
	    } else
		killed(mdef);
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
		if (haseyes(mdef->data)) {
		    pline("%s is blinded by your flash of light!", Monnam(mdef));
		    if (mdef->mcansee) {
			mdef->mblinded += tmp;
			mdef->mcansee = 0;
		    }
		}
		break;
	    case AD_COLD:
		if (!resists_cold(mdef->data)) {
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
			pline("The blast doesn't seem to affect %s.",
				mon_nam(mdef));
		}
		break;
	    default:
		break;
	}
	return(1);
}

static int
gulpum(mdef,mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	register int tmp;
	register int dam = d((int)mattk->damn, (int)mattk->damd);
	/* Not totally the same as for real monsters.  Specifically, these
	 * don't take multiple moves.  (It's just too hard, for too little
	 * result, to program monsters which attack from inside you, which
	 * would be necessary if done accurately.)  Instead, we arbitrarily
	 * kill the monster immediately for AD_DGST and we regurgitate them
	 * after exactly 1 round of attack otherwise.  -KAA
	 */

	if(mdef->data->msize >= MZ_HUGE) return 0;

	if(u.uhunger < 1500 && !u.uswallow) {

	    if(mdef->data->mlet != S_COCKATRICE) {
# ifdef LINT	/* static char msgbuf[BUFSZ]; */
		char msgbuf[BUFSZ];
# else
		static char msgbuf[BUFSZ];
# endif
/* TODO: get the symbol display also to work (monster symbol is removed from
 * the screen and you moved onto it, then you get moved back and it gets
 * moved back if the monster survives--just like when monsters swallow you.
 */
		You("engulf %s!", mon_nam(mdef));
		switch(mattk->adtyp) {
		    case AD_DGST:
			u.uhunger += mdef->data->cnutrit;
			newuhs(FALSE);
			xkilled(mdef,2);
			Sprintf(msgbuf, "You totally digest %s.",
					mon_nam(mdef));
			if ((tmp = 3 + (mdef->data->cwt >> 6)) != 0) {
			    You("digest %s.", mon_nam(mdef));
			    nomul(-tmp);
			    nomovemsg = msgbuf;
			} else pline(msgbuf);
			exercise(A_CON, TRUE);
			return(2);
		    case AD_PHYS:
			pline("%s is pummeled with your debris!",Monnam(mdef));
			break;
		    case AD_ACID:
			pline("%s is covered with your goo!", Monnam(mdef));
			if (resists_acid(mdef->data)) {
			    pline("It seems harmless to %s.", mon_nam(mdef));
			    dam = 0;
			}
			break;
		    case AD_BLND:
			if(haseyes(mdef->data)) {
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
			    pline("The air around %s crackles with electricity.", mon_nam(mdef));
			    if (resists_elec(mdef->data)) {
				pline("%s seems unhurt.", Monnam(mdef));
				dam = 0;
			    }
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_COLD:
			if (rn2(2)) {
			    if (resists_cold(mdef->data)) {
				pline("%s seems mildly chilly.", Monnam(mdef));
				dam = 0;
			    } else
				pline("%s is freezing to death!",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		    case AD_FIRE:
			if (rn2(2)) {
			    if (resists_fire(mdef->data)) {
				pline("%s seems mildly hot.", Monnam(mdef));
				dam = 0;
			    } else
				pline("%s is burning to a crisp!",Monnam(mdef));
			    golemeffects(mdef,(int)mattk->adtyp,dam);
			} else dam = 0;
			break;
		}
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
	else if(!Blind && flags.verbose)
		You("miss %s.", mon_nam(mdef));
	else
		You("miss it.");
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
	schar	dhit;

#ifdef GCC_WARN
	dhit = 0;
#endif

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
			if(uwep) tmp += hitval(uwep, mon->data);
			dhit = (tmp > (dieroll = rnd(20)) || u.uswallow);
			/* Enemy dead, before any special abilities used */
			if (!known_hitum(mon,dhit)) return 0;
			/* might be a worm that gets cut in half */
			if (m_at(u.ux+u.dx, u.uy+u.dy) != mon) return(nsum);
			/* Do not print "You hit" message, since known_hitum
			 * already did it.
			 */
			if (dhit && mattk->adtyp != AD_SPEL
				&& mattk->adtyp != AD_PHYS)
				sum[i] = damageum(mon,mattk);
			break;
		case AT_CLAW:
			if (i==0 && uwep && !cantwield(uasmon)) goto use_weapon;
# ifdef SEDUCE
			/* succubi/incubi are humanoid, but their _second_
			 * attack is AT_CLAW, not their first...
			 */
			if (i==1 && uwep && (u.umonnum == PM_SUCCUBUS ||
				u.umonnum == PM_INCUBUS)) goto use_weapon;
# endif
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
			else if (!sticks(mon->data))
			    if (mon==u.ustuck) {
				pline("%s is being %s.", Monnam(mon),
				    u.umonnum==PM_ROPE_GOLEM ? "choked":
				    "crushed");
				sum[i] = damageum(mon, mattk);
			    } else if(sum[i-1] && sum[i-2]) {
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
	    if(sum[i] == 2) return(passive(mon, 1, 0, (mattk->aatyp==AT_KICK)));
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
	return(nsum);
}

#endif /* POLYSELF */

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

#ifdef POLYSELF
		if(!resists_acid(uasmon))
#endif
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
	      if (!kicked)
		if (!uwep && !uarmg
#ifdef POLYSELF
		    && !resists_ston(uasmon)
		    && !(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))
#endif
		   ) {
		    You("turn to stone...");
		    done_in_by(mon);
		    return 2;
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
			    nomul((ACURR(A_WIS) > 12 || rn2(4)) ? -tmp : -120);
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
			You("feel a mild chill.");
#ifdef POLYSELF
			ugolemeffects(AD_COLD, tmp);
#endif
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
			You("feel mildly warm.");
#ifdef POLYSELF
			ugolemeffects(AD_FIRE, tmp);
#endif
			break;
		    }
		    You("are suddenly very hot!");
		    mdamageu(mon, tmp);
		}
		break;
	      case AD_ELEC:
		if(Shock_resistance) {
		    shieldeff(u.ux, u.uy);
		    You("feel a mild tingle.");
#ifdef POLYSELF
		    ugolemeffects(AD_ELEC, tmp);
#endif
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
	    pline("The door actually was %s!", a_monnam(mtmp));
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
			Your("%s stop glowing %s.", hands,
				Hallucination ? hcolor() : red);
	} else {
		if (Blind)
			pline("The tingling in your %s lessens.", hands);
		else
			Your("%s no longer glow so brightly %s.", hands,
				Hallucination ? hcolor() : red);
	}
	u.umconf--;
}

/*uhitm.c*/
