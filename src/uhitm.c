/*	SCCS Id: @(#)uhitm.c	3.0	89/11/27
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#ifdef NAMED_ITEMS
#  include "artifact.h"
#endif

static boolean hitum();
#ifdef POLYSELF
static boolean hmonas();
#endif
static void nohandglow();

#ifdef WORM
extern boolean notonhead;
#endif

struct monst *
clone_mon(mon)
struct monst *mon;
{
	coord mm;
	struct monst *m2;

	mm.x = mon->mx;
	mm.y = mon->my;
	enexto(&mm, mm.x, mm.y, mon->data);
	if (MON_AT(mm.x, mm.y) || mon->mhp <= 1) return (struct monst *)0;
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
#if defined(ALTARS) && defined(THEOLOGY)
	if (mon->ispriest) m2->ispriest = FALSE;
#endif
	m2->mxlth = 0;
	m2->mnamelth = 0;
	m2->mdispl = 0;
	pmon(m2);	/* display the new monster */
	place_monster(m2, m2->mx, m2->my);
	if (mon->mtame) (void) tamedog(m2, (struct obj *)0);
	return m2;
}

boolean
special_case(mtmp)
/* Moved this code from attack() in order to 	*/
/* avoid having to duplicate it in dokick.	*/
register struct monst *mtmp;
{
	if (flags.confirm && (mtmp->mpeaceful || mtmp->mtame) && !Confusion
	    && !Hallucination && (!mtmp->mhide || !mtmp->mundetected)
	    && (!mtmp->mimic || Protection_from_shape_changers)) {
		if (Blind ? Telepat : (!mtmp->minvis || See_invisible)) {
#ifdef MACOS
			char mac_tbuf[80];
			if(!flags.silent) SysBeep(1);
			sprintf(mac_tbuf, "Really attack %s?", mon_nam(mtmp));
			if(UseMacAlertText(128, mac_tbuf) != 1) {
#else
			pline("Really attack %s? ", mon_nam(mtmp));
			(void) fflush(stdout);
			if (yn() != 'y') {
#endif
				flags.move = 0;
				return(TRUE);
			}
		}
	}

	if(mtmp->mimic && !Protection_from_shape_changers) {
		stumble_onto_mimic(mtmp);
		return(TRUE);
	}

	if(mtmp->mhide && mtmp->mundetected && !canseemon(mtmp)) {
		mtmp->mundetected = 0;
		if (!(Blind ? Telepat : (HTelepat & WORN_HELMET))) {
		    register struct obj *obj;

		    if(Blind) pline("Wait!  There's a hidden monster there!");
		    else if(OBJ_AT(mtmp->mx, mtmp->my)) {
			if(obj = level.objects[mtmp->mx][mtmp->my])
				pline("Wait!  There's %s hiding under %s!",
					defmonnam(mtmp), doname(obj));
		    } else if (levl[mtmp->mx][mtmp->my].gmask == 1)
				pline("Wait!  There's %s hiding under some gold!",
					defmonnam(mtmp));
		    wakeup(mtmp);
		    return(TRUE);
		}
	}
	return(FALSE);
}

schar
find_roll_to_hit(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	struct permonst *mdat = mtmp->data;

#ifdef POLYSELF
	tmp = Luck + mdat->ac + abon() +
		     ((u.umonnum >= 0) ? uasmon->mlevel : u.ulevel);
#else
	tmp = Luck + u.ulevel + mdat->ac + abon();
#endif
/*	it is unchivalrous to attack the defenseless or from behind */
	if (pl_character[0] == 'K' && u.ualigntyp == U_LAWFUL &&
	    (!mtmp->mcanmove || mtmp->msleep || mtmp->mflee) &&
	    u.ualign > -10) adjalign(-1);

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
	tmp -= (inv_weight() + 40)/20;
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
	schar tmp = 0;
	register struct permonst *mdat = mtmp->data;

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
	/* andrew@orca: prevent unlimited pick-axe attacks */
	u_wipe_engr(3);

	if(mdat->mlet == S_LEPRECHAUN && mtmp->mfrozen && !mtmp->msleep &&
	   !mtmp->mconf && mtmp->mcansee && !rn2(7) &&
	   (m_move(mtmp, 0) == 2 ||			    /* he died */
	   mtmp->mx != u.ux+u.dx || mtmp->my != u.uy+u.dy)) /* he moved */
		return(FALSE);

	/* This section of code provides protection against accidentally
	 * hitting peaceful (like '@') and tame (like 'd') monsters.
	 * There is protection only if you're not blind, confused, or hallu-
	 * cinating.
	 */
	/*  changes by wwp 5/16/85 */
	if (!Confusion && !Hallucination && flags.safe_dog &&
	    (Blind ? Telepat : (!mtmp->minvis || See_invisible)) &&
								mtmp->mtame) {
		mtmp->mflee = 1;
		mtmp->mfleetim = rnd(6);
		if (mtmp->mnamelth)
		    You("stop to avoid hitting %s.", NAME(mtmp));
		else
		    You("stop to avoid hitting your %s.",
			  mdat->mname);
		return(TRUE);
	}

	/* moved code to a separate function to share with dokick */
	if(special_case(mtmp)) return(TRUE);

#ifdef POLYSELF
	if(u.umonnum >= 0) {	/* certain "pacifist" monsters don't attack */
		set_uasmon();
		if(noattacks(uasmon)) {
			You("have no way to attack monsters physically.");
			return(TRUE);
		}
	}
#endif

	tmp = find_roll_to_hit(mtmp);
#ifdef POLYSELF
	if (u.umonnum >= 0) (void) hmonas(mtmp, tmp);
	else
#endif
	    (void) hitum(mtmp, tmp);
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
	register boolean malive = TRUE;

	stoned = FALSE;		/* this refers to the thing hit, not you */

	if(!mhit) {
	    if(!Blind && flags.verbose) You("miss %s.", mon_nam(mon));
	    else			You("miss it.");
	    if(!mon->msleep && mon->mcanmove)
		wakeup(mon);
	} else {
	    /* we hit the monster; be careful: it might die! */

#ifdef WORM
	    if (mon->mx != u.ux+u.dx || mon->my != u.uy+u.dy)
		notonhead = TRUE;
#endif
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
#ifdef WORM
		if(mon->wormno)
		    cutworm(mon, u.ux+u.dx, u.uy+u.dy,
			    uwep ? uwep->otyp : 0);
#endif
	    }
#if defined(ALTARS) && defined(THEOLOGY)
	    if(mon->ispriest && !rn2(2)) ghod_hitsu();
#endif
	}
	return(malive);
}

static boolean
hitum(mon, tmp)		/* returns TRUE if monster still lives */
struct monst *mon;
int tmp;
{
	static int malive;
	boolean mhit = (tmp > rnd(20) || u.uswallow);

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
	register int tmp;
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
	if(!obj) {
	    tmp = rnd(2);	/* attack with bare hands */
#if 0
	    if(mdat == &mons[PM_COCKATRICE] && !uarmg
#ifdef POLYSELF
		&& !resists_ston(uasmon)
#endif
		) {

		kludge("You hit %s with your bare %s.",
			mon_nam(mon), makeplural(body_part(HAND)));
		You("turn to stone...");
		done_in_by(mon);
		hittxt = TRUE; /* maybe lifesaved */
	    }
#endif
	} else {
	    if(obj->olet == WEAPON_SYM || obj->otyp == PICK_AXE ||
	       obj->otyp == UNICORN_HORN || obj->olet == ROCK_SYM) {

		if(obj == uwep && (obj->otyp >= BOW || obj->otyp < BOOMERANG)
			&& obj->otyp != PICK_AXE && obj->otyp != UNICORN_HORN)
		    tmp = rnd(2);
		else {
		    tmp = dmgval(obj, mdat);
#ifdef NAMED_ITEMS
		    if(spec_ability(obj, SPFX_DRLI) && !resists_drli(mdat)) {
			if (!Blind) {
			    pline("The %s blade draws the life from %s!",
				Hallucination ? hcolor() : black,
				mon_nam(mon));
			    hittxt = TRUE;
			}
			if (mon->m_lev == 0) tmp = mon->mhp;
			else {
			    int drain = rnd(8);
			    tmp += drain;
			    mon->mhpmax -= drain;
			    mon->m_lev--;
			}
		    }
#endif
		    if(!thrown && obj == uwep && obj->otyp == BOOMERANG &&
		       !rnl(3)) {
			kludge("As you hit %s, the boomerang breaks into splinters.",
			      mon_nam(mon));
			useup(obj);
			hittxt = TRUE;
			tmp++;
		    }
		    if(thrown && (obj->otyp >= ARROW && obj->otyp <= SHURIKEN)){
			if(((uwep && objects[obj->otyp].w_propellor ==
				-objects[uwep->otyp].w_propellor)
				|| obj->otyp==DART || obj->otyp==SHURIKEN) &&
				obj->opoisoned)
			    ispoisoned = TRUE;
		    }
		    if(thrown && obj->otyp == SILVER_ARROW) {
			if (is_were(mdat) || mdat->mlet==S_VAMPIRE
			    || (mdat->mlet==S_IMP && mdat != &mons[PM_TENGU])
			    || is_demon(mdat)) {
				silvermsg = TRUE;
				tmp += rnd(20);
			}
		    }
		}
	    } else if(obj->olet == POTION_SYM) {
			if (obj->quan > 1) setuwep(splitobj(obj, 1));
			else setuwep((struct obj *)0);
			freeinv(obj);
			potionhit(mon,obj);
			hittxt = TRUE;
			tmp = 1;
	    } else	switch(obj->otyp) {
		case HEAVY_IRON_BALL:
			tmp = rnd(25); break;
		case BOULDER:
			tmp = rnd(20); break;
#ifdef MEDUSA
		case MIRROR:
			You("break your mirror.  That's bad luck!");
			change_luck(-2);
			useup(obj);
			hittxt = TRUE;
			tmp = 1;
#endif
		case EXPENSIVE_CAMERA:
	You("succeed in destroying your camera.  Congratulations!");
			useup(obj);
			return(TRUE);
		case CORPSE:		/* fixed by polder@cs.vu.nl */
			if(obj->corpsenm == PM_COCKATRICE) {
			    kludge("You hit %s with the cockatrice corpse.",
				  mon_nam(mon));
			    if(resists_ston(mdat)) {
				tmp = 1;
				hittxt = TRUE;
				break;
			    }
			    kludge("%s turns to stone.", Monnam(mon));
			    stoned = TRUE;
			    xkilled(mon,0);
			    nohandglow();
			    return(FALSE);
			}
			tmp = mons[obj->corpsenm].msize + 1;
			break;
		case EGG: /* only possible if hand-to-hand */
			if(obj->corpsenm > -1
					&& obj->corpsenm != PM_COCKATRICE
					&& mdat == &mons[PM_COCKATRICE]) {
				kludge("You hit %s with the %s egg%s.",
					mon_nam(mon),
					mons[obj->corpsenm].mname,
					plur((long)obj->quan));
				hittxt = TRUE;
				pline("The egg%sn't live any more...",
					(obj->quan==1) ? " is" : "s are");
				obj->otyp = ROCK;
				obj->olet = GEM_SYM;
				obj->known = obj->dknown = 0;
				obj->owt = weight(obj);
			}
			tmp = 1;
			break;
		case CLOVE_OF_GARLIC:		/* no effect against demons */
			if(is_undead(mdat)) mon->mflee = 1;
			tmp = 1;
			break;
		case CREAM_PIE:
#ifdef POLYSELF
		case BLINDING_VENOM:
			if(Blind)
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
				? "'s face" : "");
			if(mon->msleep) mon->msleep = 0;
			setmangry(mon);
			mon->mcansee = 0;
			tmp = rnd(25) + 20;
			if((mon->mblinded + tmp) > 127) mon->mblinded = 127;
			else mon->mblinded += tmp;
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			tmp = 0;
			break;
#ifdef POLYSELF
		case ACID_VENOM: /* only possible if thrown */
			if(resists_acid(mdat)) {
				kludge("Your venom hits %s harmlessly.",
					mon_nam(mon));
				tmp = 0;
			} else {
				kludge("Your venom burns %s!", mon_nam(mon));
				tmp = dmgval(obj, mdat);
			}
			hittxt = TRUE;
			get_dmg_bonus = FALSE;
			break;
#endif
		default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/10;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
	    }
	}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG)
	 *      *OR* if attacking bare-handed!! */

	if (get_dmg_bonus) {
		tmp += u.udaminc;
		/* If you throw using a propellor, you don't get a strength
		 * bonus but you do get an increase-damage bonus.
		 */
		if(!thrown || !obj || !uwep ||
			(obj->olet != GEM_SYM && obj->olet != WEAPON_SYM) ||
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
	if(tmp < 1) tmp = 1;

	mon->mhp -= tmp;
	if(mon->mhp < 1)
		destroyed = TRUE;
	if(mon->mtame && (!mon->mflee || mon->mfleetim)) {
#ifdef SOUNDS
		if (rn2(8)) yelp(mon);
		else growl(mon); /* give them a moment's worry */
#endif
		mon->mtame--;
		mon->mflee = 1;			/* Rick Richardson */
		mon->mfleetim += 10*rnd(tmp);
	}
	if((mdat == &mons[PM_BLACK_PUDDING] || mdat == &mons[PM_BROWN_PUDDING])
		   && obj && obj == uwep
		   && objects[obj->otyp].oc_material == METAL
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
		else if(Blind || !flags.verbose) You("hit it.");
		else	You("hit %s%s", mon_nam(mon), exclam(tmp));
	}

	if (silvermsg) {
		if (cansee(mon->mx, mon->my))
			pline("The silver arrow sears %s's flesh!",
				mon_nam(mon));
		else
			pline("Its flesh is seared!");
	}

	if (needpoismsg)
		kludge("The poison doesn't seem to affect %s.", mon_nam(mon));
	if (poiskilled) {
		pline("The poison was deadly...");
		xkilled(mon, 0);
		nohandglow();
		return FALSE;
	} else if (destroyed) {
		killed(mon);	/* takes care of most messages */
		nohandglow();
	} else if(u.umconf && !thrown) {
		nohandglow();
		if(!resist(mon, '+', 0, NOTELL)) mon->mconf = 1;
		if(!mon->mstun && mon->mcanmove && !mon->msleep &&
		   !Blind && mon->mconf)
			pline("%s appears confused.", Monnam(mon));
	}

#if 0
	if(mdat == &mons[PM_RUST_MONSTER] && obj && obj == uwep &&
		objects[obj->otyp].oc_material == METAL &&
		obj->olet == WEAPON_SYM && obj->spe > -2) {
	    if(obj->rustfree) {
		pline("The rust on your %s vanishes instantly!",
		      is_sword(obj) ? "sword" : "weapon");
	    } else if(obj->blessed && !rnl(4))
		pline("Somehow your %s is not affected!",
		      is_sword(obj) ? "sword" : "weapon");
	    else {
		Your("%s!", aobjnam(obj, "corrode"));
		obj->spe--;
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

	stoned = FALSE;
	if (is_demon(uasmon) && !rn2(13) && !uwep
# ifdef INFERNO
		&& u.umonnum != PM_SUCCUBUS && u.umonnum != PM_INCUBUS
		&& u.umonnum != PM_BALROG
# endif
	    ) {
	    struct monst *dtmp;
	    pline("Some hell-p has arrived!");
	    if((dtmp = makemon(!rn2(6) ? &mons[ndemon()] : uasmon, u.ux, u.uy)))
		(void)tamedog(dtmp, (struct obj *)0);
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
# ifdef GOLEMS
		golemeffects(mdef, AD_FIRE, tmp);
# endif /* GOLEMS */
		if(resists_fire(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		} else {
		    if(!Blind) pline("%s is on fire!", Monnam(mdef));
		    tmp += destroy_mitem(mdef, SCROLL_SYM, AD_FIRE);
		    tmp += destroy_mitem(mdef, POTION_SYM, AD_FIRE);
# ifdef SPELLS
		    tmp += destroy_mitem(mdef, SPBOOK_SYM, AD_FIRE);
# endif
		}
		break;
	    case AD_COLD:
# ifdef GOLEMS
		golemeffects(mdef, AD_COLD, tmp);
# endif /* GOLEMS */
		if(resists_cold(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		} else {
		    if(!Blind) pline("%s is covered in frost.", Monnam(mdef));
		    tmp += destroy_mitem(mdef, POTION_SYM, AD_COLD);
		}
		break;
	    case AD_ELEC:
# ifdef GOLEMS
		golemeffects(mdef, AD_ELEC, tmp);
# endif /* GOLEMS */
		if(resists_elec(pd)) {
		    shieldeff(mdef->mx, mdef->my);
		    tmp = 0;
		}
		break;
	    case AD_ACID:
		if(resists_acid(pd)) tmp = 0;
		break;
	    case AD_STON:
		if(!resists_ston(pd)) {
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
		    struct obj *otmp, *addinv(), *stealoid;
		    int isize = inv_cnt();

		    stealoid = (struct obj *)0;
		    if(is_mercenary(pd) && could_seduce(&youmonst,mdef,mattk)){
			for(otmp = mdef->minvent; otmp; otmp=otmp->nobj)
			    if (otmp->otyp >= PLATE_MAIL && otmp->otyp
				<= ELVEN_CLOAK) stealoid = otmp;
		    }
		    if (stealoid) {
			boolean stolen = FALSE;
			/* Is "he"/"his" always correct? */
	 kludge("You seduce %s and he starts to take off his clothes.",
				mon_nam(mdef));
			while(mdef->minvent) {
				otmp = mdef->minvent;
				mdef->minvent = otmp->nobj;
				if (!stolen && otmp==stealoid) {
					if(isize < 52) {
						otmp = addinv(otmp);
						/* might not increase isize */
						isize = inv_cnt();
					} else dropy(otmp);
					stealoid = otmp;
					stolen = TRUE;
				} else {
					if(isize < 52) {
						otmp = addinv(otmp);
						isize = inv_cnt();
						You("steal: ");
						prinv(otmp);
					} else {
						dropy(otmp);
						You("steal %s.", doname(otmp));
					}
				}
			}
			if (!stolen)
				impossible("Player steal fails!");
			else {
				kludge("%s finishes taking off his suit.",
							Monnam(mdef));
				You("steal %s!", doname(stealoid));
# ifdef ARMY
				mdef->data = &mons[PM_UNARMORED_SOLDIER];
# endif
			}
		   } else {
		   	   otmp = mdef->minvent;
			   mdef->minvent = otmp->nobj;
			   if(isize < 52) {
				otmp = addinv(otmp);
				You("steal: ");
				prinv(otmp);
			   } else {
				dropy(otmp);
				You("steal %s.", doname(otmp));
			   }
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
# ifdef GOLEMS
		    if (mdef->data == &mons[PM_CLAY_GOLEM]) {
			if (!Blind)
			    pline("Some writing vanishes from %s's head!",
				mon_nam(mdef));
			xkilled(mdef, 0);
			return 2;
		      }
# endif /* GOLEMS */
		    mdef->mcan = 1;
		    You("chuckle.");
		}
		tmp = 0;
		break;
	    case AD_DRLI:
		if(rn2(2) && !resists_drli(pd)) {
			int xtmp = d(2,6);
			kludge("%s suddenly seems weaker!", Monnam(mdef));
			mdef->mhpmax -= xtmp;
			if ((mdef->mhp -= xtmp) <= 0 || !mdef->m_lev--) {
				kludge("%s dies!", Monnam(mdef));
				xkilled(mdef,0);
				return(2);
			}
		}
		tmp = 0;
		break;
	    case AD_RUST:
# ifdef GOLEMS
		if (pd == &mons[PM_IRON_GOLEM]) {
			kludge("%s falls to pieces!", Monnam(mdef));
			xkilled(mdef,0);
			return(2);
		}
# endif /* GOLEMS */
		tmp = 0;
		break;
	    case AD_DCAY:
# ifdef GOLEMS
		if (pd == &mons[PM_WOOD_GOLEM] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
			kludge("%s falls to pieces!", Monnam(mdef));
			xkilled(mdef,0);
			return(2);
		}
# endif /* GOLEMS */
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!rn2(8)) {
		    Your("%s was poisoned!", mattk->aatyp==AT_BITE ?
			"bite" : "sting");
		    if (resists_poison(mdef->data))
			kludge("The poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    else {
			if (!rn2(10)) {
			    Your("poison was deadly...");
			    tmp = mdef->mhp;
			} else tmp += rn1(10,6);
		    }
		}
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

	    if(mdef->mtame) {
		if(!Blind) You("killed your %s!", lmonnam(mdef) + 4);
		else	You("feel embarrassed for a moment.");
	    } else {
		if(!Blind && flags.verbose)  pline("%s is killed!", Monnam(mdef));
		else	    You("kill it!");
	    }
	    xkilled(mdef, 0);
	    return(2);
	}
	return(1);
}

static int
explum(mdef, mattk)
register struct monst *mdef;
register struct attack *mattk;
{
	switch(mattk->adtyp) {
	    case AD_BLND:
		if(mdef->data->mlet != S_YLIGHT) {
		    kludge("%s is blinded by your flash of light!", Monnam(mdef));
		    if (mdef->mcansee) {
			mdef->mblinded += rn2(25);
			mdef->mcansee = 0;
		    }
		}
		break;
	    case AD_COLD:
		You("explode!");
		if (!resists_cold(mdef->data)) {
		    kludge("%s gets blasted!", Monnam(mdef));
		    mdef->mhp -= d(6,6);
		    if (mdef->mhp <= 0) {
			 killed(mdef);
			 return(2);
		    }
# ifdef GOLEMS
		} else if (is_golem(mdef->data)) {
		    golemeffects(mdef, AD_COLD, d(6,6));
		    shieldeff(mdef->mx, mdef->my);
# endif /* GOLEMS */
		} else {
		    shieldeff(mdef->mx, mdef->my);
		    kludge("The blast doesn't seem to affect %s.",
			   mon_nam(mdef));
		}
		break;
	    default:	break;
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
		kludge("You engulf %s!", mon_nam(mdef));
		switch(mattk->adtyp) {
		    case AD_DGST:
			u.uhunger += mdef->data->cnutrit;
			newuhs(FALSE);
			xkilled(mdef,2);
			Sprintf(msgbuf, "You totally digest %s.",
					Blind ? "it" : mon_nam(mdef));
			if (tmp = 3 + (mdef->data->cwt >> 2)) {
			    kludge("You digest %s.", mon_nam(mdef));
			    nomul(-tmp);
			    nomovemsg = msgbuf;
			} else pline(msgbuf);
			return(2);
		    case AD_PHYS:
			kludge("%s is pummeled with your debris!",Monnam(mdef));
			break;
		    case AD_ACID:
			kludge("%s is covered with your goo!", Monnam(mdef));
			if (resists_acid(mdef->data)) {
			    kludge("It seems harmless to %s.", mon_nam(mdef));
			    dam = 0;
			}
			break;
		    case AD_BLND:
			if (mdef->mcansee)
			    kludge("%s can't see in there!", Monnam(mdef));
			mdef->mcansee = 0;
			dam += mdef->mblinded;
			if (dam > 127) dam = 127;
			mdef->mblinded = dam;
			dam = 0;
			break;
		    case AD_ELEC:
			if (rn2(2)) {
			    kludge("The air around %s crackles with electricity.", mon_nam(mdef));
			    if (resists_elec(mdef->data)) {
				kludge("%s seems unhurt.", Monnam(mdef));
				dam = 0;
			    }
# ifdef GOLEMS
			    golemeffects(mdef,(int)mattk->adtyp,dam);
# endif
			} else dam = 0;
			break;
		    case AD_COLD:
			if (rn2(2)) {
			    if (resists_cold(mdef->data)) {
				kludge("%s seems mildly chilly.", Monnam(mdef));
				dam = 0;
			    } else
				kludge("%s is freezing to death!",Monnam(mdef));
# ifdef GOLEMS
			    golemeffects(mdef,(int)mattk->adtyp,dam);
# endif
			} else dam = 0;
			break;
		    case AD_FIRE:
			if (rn2(2)) {
			    if (resists_fire(mdef->data)) {
				kludge("%s seems mildly hot.", Monnam(mdef));
				dam = 0;
			    } else
				kludge("%s is burning to a crisp!",Monnam(mdef));
# ifdef GOLEMS
			    golemeffects(mdef,(int)mattk->adtyp,dam);
# endif
			} else dam = 0;
			break;
		}
		if ((mdef->mhp -= dam) <= 0) {
		    kludge("%s is killed!", Monnam(mdef));
		    xkilled(mdef,0);
		    return(2);
		}
		kludge("You %s %s!", is_animal(uasmon) ? "regurgitate"
			: "expel", mon_nam(mdef));
		if (is_animal(uasmon)) {
		    if (Blind)
			pline("Obviously, you didn't like its taste.");
		    else
			pline("Obviously, you didn't like %s's taste.",
								mon_nam(mdef));
		}
	    } else {
		kludge("You bite into %s", mon_nam(mdef));
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
		kludge("You pretend to be friendly to %s.", mon_nam(mdef));
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
			dhit = (tmp > rnd(20) || u.uswallow);
			/* Enemy dead, before any special abilities used */
			if (!known_hitum(mon,dhit)) return 0;
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
			if (i==0 && uwep && (u.usym==S_LICH)) goto use_weapon;
			if(dhit = (tmp > rnd(20) || u.uswallow)) {
				int compat;

				if (!u.uswallow &&
				    (compat = could_seduce(&youmonst,
							    mon, mattk)))
				You("%s %s %s.",
				    mon->mcansee ? "smile at" : "talk to",
				    Blind ? "it" : mon_nam(mon),
				    compat == 2 ? "engagingly" : "seductively");
				else if (mattk->aatyp == AT_KICK)
					kludge("You kick %s.", mon_nam(mon));
				else if (mattk->aatyp == AT_BITE)
					kludge("You bite %s.", mon_nam(mon));
				else if (mattk->aatyp == AT_STNG)
					kludge("You sting %s.", mon_nam(mon));
				else if (mattk->aatyp == AT_BUTT)
					kludge("You butt %s.", mon_nam(mon));
				else if (mattk->aatyp == AT_TUCH)
					kludge("You touch %s.", mon_nam(mon));
				else kludge("You hit %s.", mon_nam(mon));
				sum[i] = damageum(mon, mattk);
			} else
				missum(mon, mattk);
			break;

		case AT_HUGS:
			/* automatic if prev two attacks succeed, or if
			 * already grabbed in a previous attack
			 */
			dhit = 1;
			if (!sticks(mon->data))
			    if (mon==u.ustuck) {
				kludge("%s is being %s.", Monnam(mon),
# ifdef GOLEMS
				    u.umonnum==PM_ROPE_GOLEM ? "choked":
# endif
				    "crushed");
				sum[i] = damageum(mon, mattk);
			    } else if(sum[i-1] && sum[i-2]) {
				kludge("You grab %s!", mon_nam(mon));
				u.ustuck = mon;
				sum[i] = damageum(mon, mattk);
			    }
			break;

		case AT_EXPL:	/* automatic hit if next to */
			dhit = -1;
			sum[i] = explum(mon, mattk);
			break;

		case AT_ENGL:
			if((dhit = (tmp > rnd(20+i))))
				sum[i]= gulpum(mon,mattk);
			else
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

		default: /* Strange... */
			impossible("strange attack of yours (%d)",
				 mattk->aatyp);
		case AT_BREA:
		case AT_SPIT:
		case AT_GAZE:	/* all done using #monster command */
			dhit = 0;
			break;
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

/*	These affect you even if they just died */

	switch(ptr->mattk[i].adtyp) {

	  case AD_ACID:
	    if(mhit && rn2(2)) {
		if (Blind || !flags.verbose) You("are splashed!");
		else	You("are splashed by %s's acid!", mon_nam(mon));

		tmp = d((int)mon->m_lev, (int)ptr->mattk[i].damd);
#ifdef POLYSELF
		if(!resists_acid(uasmon))
#endif
			mdamageu(mon, tmp);
		if(!rn2(30)) corrode_armor();
	    }
	    if(mhit && !rn2(6)) {
		if (kicked) {
		    if (uarmf)
			(void) rust_dmg(uarmf, xname(uarmf), 3, TRUE);
		} else corrode_weapon();
	    }
	    break;
	  case AD_STON:
	    if(mhit)
	      if (!kicked)
		if (!uwep && !uarmg
#ifdef POLYSELF
		    && !resists_ston(uasmon)
#endif
		   ) {
		    You("turn to stone...");
		    done_in_by(mon);
		    return 2;
		}
	    break;
	  case AD_RUST:
	    if(mhit)
	      if (kicked) {
		if (uarmf)
		    (void) rust_dmg(uarmf, xname(uarmf), 1, TRUE);
	      } else
		corrode_weapon();
	    break;
	  case AD_MAGM:
	    /* wrath of gods for attacking Oracle */
	    if(Antimagic) {
		shieldeff(u.ux, u.uy);
		pline("A hail of magic missiles narrowly misses you!");
	    } else {
		tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
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
		tmp = -d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
		if(ptr == &mons[PM_FLOATING_EYE]) {
		    if (!canseemon(mon)) {
			tmp = 0;
			break;
		    }
		    if(mon->mcansee) {
			if(Reflecting & W_AMUL) {
			    makeknown(AMULET_OF_REFLECTION);
			    pline("%s's gaze is reflected by your medallion.",
				  Monnam(mon));
			} else if(Reflecting & W_ARMS) {
			    makeknown(SHIELD_OF_REFLECTION);
			    pline("%s's gaze is reflected by your shield.",
				  Monnam(mon));
			} else {
			    You("are frozen by %s's gaze!", mon_nam(mon));
			    nomul((ACURR(A_WIS) > 12 || rn2(4)) ? tmp : -120);
			}
		    } else {
			pline("%s cannot defend itself.", Amonnam(mon,"blind"));
			if(!rn2(500)) change_luck(-1);
		    }
		} else { /* gelatinous cube */
		    You("are frozen by %s!", mon_nam(mon));
		    nomul(tmp);
		    tmp = 0;
		}
		break;
	      case AD_COLD:		/* brown mold or blue jelly */
		if(monnear(mon, u.ux, u.uy)) {
		    tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
		    if(Cold_resistance) {
  			shieldeff(u.ux, u.uy);
			You("feel a mild chill.");
#ifdef POLYSELF
#ifdef GOLEMS
			ugolemeffects(AD_COLD, tmp);
#endif /* GOLEMS */
#endif
			tmp = 0;
			break;
		    }
		    You("are suddenly very cold!");
		    mdamageu(mon, tmp);
		/* monster gets stronger with your heat! */
		    mon->mhp += tmp / 2;
		    if (mon->mhpmax < mon->mhp) mon->mhpmax = mon->mhp;
		/* at a certain point, the monster will reproduce! */
		    if(mon->mhpmax > ((mon->m_lev+1) * 8)) {
			register struct monst *mtmp;

			if(mtmp = clone_mon(mon)) {
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
		    make_stunned((long)d((int)mon->m_lev+1, (int)ptr->mattk[i].damd), TRUE);
		break;
	      case AD_FIRE:
		if(monnear(mon, u.ux, u.uy)) {
		    tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
		    if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			You("feel mildly warm.");
#if defined(POLYSELF) && defined(GOLEMS)
			ugolemeffects(AD_FIRE, tmp);
#endif
			tmp = 0;
			break;
		    }
		    You("are suddenly very hot!");
		    mdamageu(mon, tmp);
		}
		break;
	      case AD_ELEC:
		tmp = d((int)mon->m_lev+1, (int)ptr->mattk[i].damd);
		if(Shock_resistance) {
		    shieldeff(u.ux, u.uy);
		    You("feel a mild tingle.");
#if defined(POLYSELF) && defined(GOLEMS)
		    ugolemeffects(AD_ELEC, tmp);
#endif
		    tmp = 0;
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

/* Note: caller must ascertain mtmp->mimic... */
void
stumble_onto_mimic(mtmp)
register struct monst *mtmp;
{
	if(!u.ustuck && !mtmp->mflee && dmgtype(mtmp->data,AD_STCK))
		u.ustuck = mtmp;
	if (Blind) goto generic;
	else if (levl[u.ux+u.dx][u.uy+u.dy].scrsym == CLOSED_DOOR_SYM)
#ifdef SPELLS
	{
		if (IS_ROCK(levl[u.ux+u.dx][u.uy+u.dy].typ) ||
		    IS_DOOR(levl[u.ux+u.dx][u.uy+u.dy].typ))
#endif
			pline("The door actually was %s.", defmonnam(mtmp));
#ifdef SPELLS
		else
			pline("That spellbook was %s.", defmonnam(mtmp));
	}
#endif
	else if (levl[u.ux+u.dx][u.uy+u.dy].scrsym == GOLD_SYM)
		pline("That gold was %s!", defmonnam(mtmp));
	else {
generic:
		pline("Wait!  That's %s!", defmonnam(mtmp));
	}
	wakeup(mtmp);	/* clears mtmp->mimic */
}

static void
nohandglow()
{
	if (!u.umconf) return; /* for safety */
	if (u.umconf == 1) {
		if (Blind)
			Your("%s stop tingling.", makeplural(body_part(HAND)));
		else
			Your("%s stop glowing %s.",
				makeplural(body_part(HAND)),
				Hallucination ? hcolor() : red);
	} else {
		if (Blind)
			pline("The tingling in your %s lessens.",
				makeplural(body_part(HAND)));
		else
			Your("%s no longer glow so brightly %s.",
				makeplural(body_part(HAND)),
				Hallucination ? hcolor() : red);
	}
	u.umconf--;
}
