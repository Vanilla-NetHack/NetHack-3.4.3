/*	SCCS Id: @(#)weapon.c	3.2	96/05/12	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	This module contains code for calculation of "to hit" and damage
 *	bonuses for any given weapon used, as well as weapons selection
 *	code for monsters.
 */
#include "hack.h"

#ifdef WEAPON_SKILLS
#ifndef OVLB

STATIC_DCL NEARDATA const short skill_names_indices[];
STATIC_DCL NEARDATA const char *odd_skill_names[];

#else	/* OVLB */

STATIC_OVL NEARDATA const short skill_names_indices[P_NUM_SKILLS] = {
	DAGGER,		KNIFE,		AXE,		PICK_AXE,
	SHORT_SWORD,	BROADSWORD,	LONG_SWORD,	TWO_HANDED_SWORD,
	SCIMITAR,	PN_SABER,	CLUB,		MACE,
	MORNING_STAR,	FLAIL,		WAR_HAMMER,	QUARTERSTAFF,
	PN_POLEARMS,	SPEAR,		JAVELIN,	TRIDENT,
	LANCE,		BOW,		SLING,		CROSSBOW,
	DART,		SHURIKEN,	BOOMERANG,	BULLWHIP,
	UNICORN_HORN,	PN_TWO_WEAPONS,	PN_BARE_HANDED,
};

/* note: entry [0] isn't used */
STATIC_OVL NEARDATA const char *odd_skill_names[] = {
    0, "polearms", "saber", "two weapon combat",
    "bare handed combat", "martial arts",
};

static NEARDATA const char may_advance_msg[] =
				"feel more confident in your fighting skills.";

#endif	/* OVLB */

STATIC_DCL boolean FDECL(can_advance, (int));

#ifdef OVL1

static char *FDECL(skill_level_name, (int,char *));
static int FDECL(slots_required, (int));
static void FDECL(skill_advance, (int));

#endif	/* OVL1 */

#define P_NAME(type) (skill_names_indices[type] >= 0 ? \
		      OBJ_NAME(objects[skill_names_indices[type]]) : \
		      (type == P_BARE_HANDED_COMBAT ? \
			(martial_bonus() ? odd_skill_names[-PN_MARTIAL_ARTS] \
					 : odd_skill_names[-PN_BARE_HANDED]) : \
		      odd_skill_names[-skill_names_indices[type]]))
#endif /* WEAPON_SKILLS */

#ifdef OVLB

static NEARDATA const char kebabable[] = {
	S_XORN, S_DRAGON, S_JABBERWOCK, S_NAGA, S_GIANT, '\0'
};

/*
 *	hitval returns an integer representing the "to hit" bonuses
 *	of "otmp" against the monster.
 */
int
hitval(otmp, mon)
struct obj *otmp;
struct monst *mon;
{
	int	tmp = 0;
	struct permonst *ptr = mon->data;
	boolean Is_weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp));

	if (Is_weapon)
		tmp += otmp->spe;

/*	Put weapon specific "to hit" bonuses in below:		*/
	tmp += objects[otmp->otyp].oc_hitbon;
#ifdef WEAPON_SKILLS
	tmp += weapon_hit_bonus(otmp);	/* weapon skill */
#endif /* WEAPON_SKILLS */

/*	Put weapon vs. monster type "to hit" bonuses in below:	*/

	/* Blessed weapons used against undead or demons */
	if (Is_weapon && otmp->blessed &&
	   (is_demon(ptr) || is_undead(ptr))) tmp += 2;

	if (objects[otmp->otyp].oc_wepcat == WEP_SPEAR &&
	   index(kebabable, ptr->mlet)) tmp += 2;

	/* trident is highly effective against swimmers */
	if (otmp->otyp == TRIDENT && is_swimmer(ptr)) {
	   if (is_pool(mon->mx, mon->my)) tmp += 4;
	   else if (ptr->mlet == S_EEL || ptr->mlet == S_SNAKE) tmp += 2;
	}

	/* pick-axe used against xorns and earth elementals */
	if ((otmp->otyp == PICK_AXE || otmp->otyp == DWARVISH_MATTOCK) &&
	   (passes_walls(ptr) && thick_skinned(ptr))) tmp += 2;

	/* Check specially named weapon "to hit" bonuses */
	if (otmp->oartifact) tmp += spec_abon(otmp, mon);

	return tmp;
}

/*
 *	dmgval returns an integer representing the damage bonuses
 *	of "otmp" against the monster.
 */
int
dmgval(otmp, mon)
struct obj *otmp;
struct monst *mon;
{
	int tmp = 0, otyp = otmp->otyp;
	struct permonst *ptr = mon->data;
	boolean Is_weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp));

	if (otyp == CREAM_PIE) return 0;

	if (bigmonst(ptr)) {
	    if (objects[otyp].oc_wldam)
		tmp = rnd(objects[otyp].oc_wldam);
	    switch (otyp) {
		case CROSSBOW_BOLT:
		case MORNING_STAR:
		case PARTISAN:
		case RUNESWORD:
		case ELVEN_BROADSWORD:
		case BROADSWORD:	tmp++; break;

		case FLAIL:
		case RANSEUR:
		case VOULGE:		tmp += rnd(4); break;

		case ACID_VENOM:
		case HALBERD:
		case SPETUM:		tmp += rnd(6); break;

		case BATTLE_AXE:
		case BARDICHE:
		case TRIDENT:		tmp += d(2,4); break;

		case TSURUGI:
		case DWARVISH_MATTOCK:
		case TWO_HANDED_SWORD:	tmp += d(2,6); break;
	    }
	} else {
	    if (objects[otyp].oc_wsdam)
		tmp = rnd(objects[otyp].oc_wsdam);
	    switch (otyp) {
		case CROSSBOW_BOLT:
		case MACE:
		case WAR_HAMMER:
		case FLAIL:
		case SPETUM:
		case TRIDENT:		tmp++; break;

		case BATTLE_AXE:
		case BARDICHE:
		case BILL_GUISARME:
		case GUISARME:
		case LUCERN_HAMMER:
		case MORNING_STAR:
		case RANSEUR:
		case BROADSWORD:
		case ELVEN_BROADSWORD:
		case RUNESWORD:
		case VOULGE:		tmp += rnd(4); break;

		case ACID_VENOM:	tmp += rnd(6); break;
	    }
	}
	if (Is_weapon)
		tmp += otmp->spe;

	if (objects[otyp].oc_material <= LEATHER && thick_skinned(ptr))
		/* thick skinned/scaled creatures don't feel it */
		tmp = 0;
	if (ptr == &mons[PM_SHADE] && objects[otyp].oc_material != SILVER)
		tmp = 0;

/*	Put weapon vs. monster type damage bonuses in below:	*/
	if (Is_weapon || otmp->oclass == GEM_CLASS) {
	    int bonus = 0;

	    if (otmp->blessed && (is_undead(ptr) || is_demon(ptr)))
		bonus += rnd(4);
	    if ((otyp == AXE || otyp == BATTLE_AXE) && is_wooden(ptr))
		bonus += rnd(4);
	    if (objects[otyp].oc_material == SILVER && hates_silver(ptr))
		bonus += rnd(20);

	    /* if the weapon is going to get a double damage bonus, adjust
	       this bonus so that effectively it's added after the doubling */
	    if (bonus > 1 && otmp->oartifact && spec_dbon(otmp, mon, 25) >= 25)
		bonus = (bonus + 1) / 2;

	    tmp += bonus;
	}

	if (tmp > 0) {
		tmp -= otmp->oeroded;
		if (tmp < 1) tmp = 1;
	}

	return(tmp);
}

#endif /* OVLB */
#ifdef OVL0

static struct obj *FDECL(oselect, (struct monst *,int));
#define Oselect(x)	if ((otmp = oselect(mtmp, x)) != 0) return(otmp);

static struct obj *
oselect(mtmp, x)
struct monst *mtmp;
int x;
{
	struct obj *otmp;

	for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
	    if (otmp->otyp == x &&
		    /* never select non-cockatrice corpses */
		    !(x == CORPSE && otmp->corpsenm != PM_COCKATRICE) &&
		    (!otmp->oartifact || touch_artifact(otmp,mtmp)))
		return otmp;
	}
	return (struct obj *)0;
}

static NEARDATA const int rwep[] =
	{ DWARVISH_SPEAR, ELVEN_SPEAR, SPEAR, ORCISH_SPEAR, JAVELIN,
	  SHURIKEN, YA, SILVER_ARROW, ELVEN_ARROW, ARROW, ORCISH_ARROW,
	  CROSSBOW_BOLT, ELVEN_DAGGER, DAGGER, ORCISH_DAGGER, KNIFE, ROCK,
	  LOADSTONE, LUCKSTONE, DART, /* BOOMERANG, */ CREAM_PIE
	  /* note: CREAM_PIE should NOT be #ifdef KOPS */
	  };

static struct obj *propellor;

struct obj *
select_rwep(mtmp)	/* select a ranged weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	int i;

#ifdef KOPS
	char mlet = mtmp->data->mlet;
#endif

	propellor = &zeroobj;
#ifdef KOPS
	if(mlet == S_KOP)	/* pies are first choice for Kops */
	    Oselect(CREAM_PIE);
#endif
	if(throws_rocks(mtmp->data))	/* ...boulders for giants */
	    Oselect(BOULDER);

	/*
	 * other than these two specific cases, always select the
	 * most potent ranged weapon to hand.
	 */
	for (i = 0; i < SIZE(rwep); i++) {
	    int prop;

	    propellor = &zeroobj;
	    /* shooting gems from slings; this goes just before the darts */
	    if (rwep[i]==DART && !likes_gems(mtmp->data)
			&& (propellor = m_carrying(mtmp, SLING))) {
		for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
		    if(otmp->oclass==GEM_CLASS &&
				(otmp->otyp != LOADSTONE || !otmp->cursed))
			return(otmp);
		}
	    }
	    prop = (objects[rwep[i]]).w_propellor;
	    if (prop > 0) {
		switch (prop) {
		case WP_BOW:
		  propellor = (oselect(mtmp, YUMI));
		  if (!propellor) propellor = (oselect(mtmp, ELVEN_BOW));
		  if (!propellor) propellor = (oselect(mtmp, BOW));
		  if (!propellor) propellor = (oselect(mtmp, ORCISH_BOW));
		  break;
		case WP_SLING:
		  propellor = (oselect(mtmp, SLING));
		  break;
		case WP_CROSSBOW:
		  propellor = (oselect(mtmp, CROSSBOW));
		}
		if ((otmp = MON_WEP(mtmp)) && otmp->cursed && otmp != propellor
				&& mtmp->weapon_check == NO_WEAPON_WANTED)
			propellor = 0;
	    }
	    /* propellor = obj, propellor to use
	     * propellor = &zeroobj, doesn't need a propellor
	     * propellor = 0, needed one and didn't have one
	     */
	    if (propellor != 0) {
		/* Note: cannot use m_carrying for loadstones, since it will
		 * always select the first object of a type, and maybe the
		 * monster is carrying two but only the first is unthrowable.
		 */
		if (rwep[i] != LOADSTONE) {
			/* Don't throw a cursed weapon-in-hand */
			if ((otmp = oselect(mtmp, rwep[i]))
			    && (!otmp->cursed || otmp != MON_WEP(mtmp)))
				return(otmp);
		} else for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
		    if (otmp->otyp == LOADSTONE && !otmp->cursed)
			return otmp;
		}
	    }
	  }

	/* failure */
	return (struct obj *)0;
}

/* Weapons in order of preference */
static NEARDATA short hwep[] = {
	  CORPSE,  /* cockatrice corpse */
	  TSURUGI, RUNESWORD, DWARVISH_MATTOCK, TWO_HANDED_SWORD, BATTLE_AXE,
	  KATANA, UNICORN_HORN, CRYSKNIFE, TRIDENT, LONG_SWORD,
	  ELVEN_BROADSWORD, BROADSWORD, LUCERN_HAMMER, SCIMITAR, SILVER_SABER,
	  HALBERD, PARTISAN, LANCE, FAUCHARD, BILL_GUISARME, BEC_DE_CORBIN,
	  GUISARME, RANSEUR, SPETUM, VOULGE, BARDICHE, MORNING_STAR, GLAIVE,
	  ELVEN_SHORT_SWORD, DWARVISH_SHORT_SWORD, SHORT_SWORD,
	  ORCISH_SHORT_SWORD, MACE, AXE, DWARVISH_SPEAR, ELVEN_SPEAR, SPEAR,
	  ORCISH_SPEAR, FLAIL, BULLWHIP, QUARTERSTAFF, JAVELIN, AKLYS, CLUB,
	  PICK_AXE,
#ifdef KOPS
	  RUBBER_HOSE,
#endif /* KOPS */
	  WAR_HAMMER, ELVEN_DAGGER, DAGGER, ORCISH_DAGGER, ATHAME, SCALPEL,
	  KNIFE, WORM_TOOTH
	};

struct obj *
select_hwep(mtmp)	/* select a hand to hand weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	register int i;
	boolean strong = strongmonst(mtmp->data);
	boolean wearing_shield = (mtmp->misc_worn_check & W_ARMS) != 0;

	/* prefer artifacts to everything else */
	for(otmp=mtmp->minvent; otmp; otmp = otmp->nobj) {
		if (otmp->oclass == WEAPON_CLASS
			&& otmp->oartifact && touch_artifact(otmp,mtmp)
			&& ((strong && !wearing_shield)
			    || !objects[otmp->otyp].oc_bimanual))
		    return otmp;
	}

	if(is_giant(mtmp->data))	/* giants just love to use clubs */
	    Oselect(CLUB);

	/* only strong monsters can wield big (esp. long) weapons */
	/* big weapon is basically the same as bimanual */
	/* all monsters can wield the remaining weapons */
	for (i = 0; i < SIZE(hwep); i++)
	    if (((strong && !wearing_shield)
			|| !objects[hwep[i]].oc_bimanual) &&
		    (objects[hwep[i]].oc_material != SILVER
			|| !hates_silver(mtmp->data)))
		Oselect(hwep[i]);

	/* failure */
	return (struct obj *)0;
}

/* Called after polymorphing a monster, robbing it, etc....  Monsters
 * otherwise never unwield stuff on their own.  Shouldn't print messages.
 */
void
possibly_unwield(mon)
register struct monst *mon;
{
	register struct obj *obj;
	struct obj *mw_tmp;

	if (!(mw_tmp = MON_WEP(mon)))
		return;
	for(obj=mon->minvent; obj; obj=obj->nobj)
		if (obj == mw_tmp) break;
	if (!obj) { /* The weapon was stolen or destroyed */
		MON_NOWEP(mon);
		mon->weapon_check = NEED_WEAPON;
		return;
	}
	if (!attacktype(mon->data, AT_WEAP)) {
		mw_tmp->owornmask &= ~W_WEP;
		MON_NOWEP(mon);
		mon->weapon_check = NO_WEAPON_WANTED;
		obj_extract_self(obj);
		/* flooreffects unnecessary, can't wield boulders */
		place_object(obj, mon->mx, mon->my);
		stackobj(obj);
		if (cansee(mon->mx, mon->my)) {
			pline("%s drops %s.", Monnam(mon),
				distant_name(obj, doname));
			newsym(mon->mx, mon->my);
		}
		return;
	}
	/* The remaining case where there is a change is where a monster
	 * is polymorphed into a stronger/weaker monster with a different
	 * choice of weapons.  This has no parallel for players.  It can
	 * be handled by waiting until mon_wield_item is actually called.
	 * Though the monster still wields the wrong weapon until then,
	 * this is OK since the player can't see it.
	 * Note that if there is no change, setting the check to NEED_WEAPON
	 * is harmless.
	 * Possible problem: big monster with big cursed weapon gets
	 * polymorphed into little monster.  But it's not quite clear how to
	 * handle this anyway....
	 */
	mon->weapon_check = NEED_WEAPON;
}

/* Let a monster try to wield a weapon, based on mon->weapon_check.
 * Returns 1 if the monster took time to do it, 0 if it did not.
 */
int
mon_wield_item(mon)
register struct monst *mon;
{
	struct obj *obj;

	/* This case actually should never happen */
	if (mon->weapon_check == NO_WEAPON_WANTED) return 0;

	switch(mon->weapon_check) {
		case NEED_HTH_WEAPON:
			obj = select_hwep(mon);
			break;
		case NEED_RANGED_WEAPON:
			(void)select_rwep(mon);
			obj = propellor;
			break;
		case NEED_PICK_AXE:
			obj = m_carrying(mon, PICK_AXE);
			break;
		default: impossible("weapon_check %d for %s?",
				mon->weapon_check, mon_nam(mon));
			return 0;
	}
	if (obj && obj != &zeroobj) {
		struct obj *mw_tmp = MON_WEP(mon);
		if (mw_tmp && mw_tmp->otyp == obj->otyp) {
		/* already wielding it */
			mon->weapon_check = NEED_WEAPON;
			return 0;
		}
		/* Actually, this isn't necessary--as soon as the monster
		 * wields the weapon, the weapon welds itself, so the monster
		 * can know it's cursed and needn't even bother trying.
		 * Still....
		 */
		if (mw_tmp && mw_tmp->cursed && mw_tmp->otyp != CORPSE) {
		    if (canseemon(mon)) {
			char welded_buf[BUFSZ];

			Sprintf(welded_buf, "%s welded to %s hand%s",
				(mw_tmp->quan == 1L) ? "is" : "are",
				his[pronoun_gender(mon)],
				objects[mw_tmp->otyp].oc_bimanual ? "s" : "");

			if (obj->otyp == PICK_AXE) {
			    pline("Since %s weapon%s %s,",
				  s_suffix(mon_nam(mon)),
				  plur(mw_tmp->quan), welded_buf);
			    pline("%s cannot wield that %s.",
				mon_nam(mon), xname(obj));
			} else {
			    pline("%s tries to wield %s.", Monnam(mon),
				doname(obj));
			    pline("%s %s %s!",
				  s_suffix(Monnam(mon)),
				  xname(mw_tmp), welded_buf);
			}
			mw_tmp->bknown = 1;
		    }
		    mon->weapon_check = NO_WEAPON_WANTED;
		    return 1;
		}
		mon->mw = obj;		/* wield obj */
		if (mw_tmp) mw_tmp->owornmask &= ~W_WEP;
		mon->weapon_check = NEED_WEAPON;
		if (canseemon(mon)) {
			pline("%s wields %s!", Monnam(mon), doname(obj));
			if (obj->cursed && obj->otyp != CORPSE) {
				pline("%s %s to %s hand!",
					The(xname(obj)),
					(obj->quan == 1L) ? "welds itself"
					    : "weld themselves",
					s_suffix(mon_nam(mon)));
				obj->bknown = 1;
			}
		}
		obj->owornmask = W_WEP;
		return 1;
	}
	mon->weapon_check = NEED_WEAPON;
	return 0;
}

int
abon()		/* attack bonus for strength & dexterity */
{
	int	sbon;
	register int	str = ACURR(A_STR), dex = ACURR(A_DEX);

	if (u.umonnum >= LOW_PM) return(adj_lev(&mons[u.umonnum]) - 3);
	if (str < 6) sbon = -2;
	else if (str < 8) sbon = -1;
	else if (str < 17) sbon = 0;
	else if (str < 69) sbon = 1;	/* up to 18/50 */
	else if (str < 118) sbon = 2;
	else sbon = 3;

/* Game tuning kludge: make it a bit easier for a low level character to hit */
	sbon += (u.ulevel < 3) ? 1 : 0;

	if (dex < 4) return(sbon-3);
	else if (dex < 6) return(sbon-2);
	else if (dex < 8) return(sbon-1);
	else if (dex < 14) return(sbon);
	else return(sbon + dex-14);
}

#endif /* OVL0 */
#ifdef OVL1

int
dbon()		/* damage bonus for strength */
{
	register int	str = ACURR(A_STR);

	if (u.umonnum >= LOW_PM) return(0);

	if (str < 6) return(-1);
	else if (str < 16) return(0);
	else if (str < 18) return(1);
	else if (str == 18) return(2);		/* up to 18 */
	else if (str < 94) return(3);		/* up to 18/75 */
	else if (str < 109) return(4);		/* up to 18/90 */
	else if (str < 118) return(5);		/* up to 18/99 */
	else return(6);
}


#ifdef WEAPON_SKILLS

/* copy the skill level name into the given buffer */
static char *
skill_level_name(skill, buf)
int skill;
char *buf;
{
    const char *ptr;

    switch (P_SKILL(skill)) {
	case P_UNSKILLED:    ptr = "Unskilled"; break;
	case P_BASIC:	     ptr = "Basic";     break;
	case P_SKILLED:	     ptr = "Skilled";   break;
	case P_EXPERT:	     ptr = "Expert";    break;
	/* these are for unarmed combat/martial arts only */
	case P_MASTER:	     ptr = "Master";    break;
	case P_GRAND_MASTER: ptr = "Grand Master"; break;
	default:	     ptr = "Unknown";	break;
    }
    Strcpy(buf, ptr);
    return buf;
}

/* return the # of slots required to advance the skill */
static int
slots_required(skill)
int skill;
{
    /* The more difficult the training, the more slots it takes. */
    if (skill <= P_LAST_WEAPON || skill == P_TWO_WEAPON_COMBAT)
	return P_SKILL(skill);

    return (P_SKILL(skill) > 5) ? 2 : 1;	/* unarmed or martial */
}

/* return true if this skill can be advanced */
STATIC_OVL boolean
can_advance(skill)
int skill;
{
    return !P_RESTRICTED(skill)
	    && P_SKILL(skill) < P_MAX_SKILL(skill)
	    && P_ADVANCE(skill) >=
		(unsigned) practice_needed_to_advance(P_SKILL(skill))
	    && u.skills_advanced < P_SKILL_LIMIT
	    && u.weapon_slots >= slots_required(skill);
}

static void
skill_advance(skill)
int skill;
{
    u.weapon_slots -= slots_required(skill);
    P_SKILL(skill)++;
    u.skill_record[u.skills_advanced++] = skill;
    /* subtly change the adavnce message to indicate no more advancement */
    You("are now %s skilled in %s.", 
    	P_SKILL(skill) >= P_MAX_SKILL(skill) ? "most" : "more",
    	P_NAME(skill));
}

/*
 * The `#enhance' extended command.  What we _really_ would like is
 * to keep being able to pick things to advance until we couldn't any
 * more.  This is currently not possible -- the menu code has no way
 * to call us back for instant action.  Even if it did, we would also need
 * to be able to update the menu since selecting one item could make
 * others unselectable.
 */
int
enhance_weapon_skill()
{
    int i, n, len, longest, to_advance;
    char buf[BUFSIZ], buf2[BUFSIZ];
    menu_item *selected;
    anything any;
    winid win;

    /* find longest available skill name, count those that can advance */
    for (longest = 0, to_advance = 0, i = 0; i < P_NUM_SKILLS; i++) {
	if (!P_RESTRICTED(i) && (len = strlen(P_NAME(i))) > longest)
	    longest = len;
	if (can_advance(i)) to_advance++;
    }

    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    /* list the skills, making ones that could be advanced selectable */
    for (any.a_void = 0, i = 0; i < P_NUM_SKILLS; i++) {
	if (P_RESTRICTED(i)) continue;
	if (i == P_TWO_WEAPON_COMBAT) continue;	/* skip for now */
	/*
	 * Sigh, this assumes a monospaced font.
	 * The 12 is the longest skill level name.
	 * The "    " is room for a selection letter and dash, "a - ".
	 */
#ifdef WIZARD
	if (wizard)
	    Sprintf(buf2, "%s%-*s %-12s %4d(%4d)",
		    to_advance == 0 || can_advance(i) ? "" : "    " ,
		    longest, P_NAME(i),
		    skill_level_name(i, buf),
		    P_ADVANCE(i), practice_needed_to_advance(P_SKILL(i)));
	else
#endif
	    Sprintf(buf2, "%s %-*s [%s]",
		    to_advance == 0 || can_advance(i) ? "" : "    ",
		    longest, P_NAME(i),
		    skill_level_name(i, buf));

	any.a_int = can_advance(i) ? i+1 : 0;
	add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, buf2, MENU_UNSELECTED);
    }

    end_menu(win, to_advance ?	"Pick a skill to advance:" :
				"Current skills:");
    n = select_menu(win, to_advance ? PICK_ONE : PICK_NONE, &selected);
    destroy_nhwindow(win);
    if (n > 0) {
	n = selected[0].item.a_int - 1;	/* get item selected */
	free((genericptr_t)selected);
	skill_advance(n);
	/* check for more skills able to advance, if so then .. */
	for (i = 0; i < P_NUM_SKILLS; i++) {
	    if (can_advance(i)) {
		You("feel you could be more dangerous!");
		break;
	    }
	}
    }
    return 0;
}

/*
 * Change from restricted to unrestricted, allowing P_BASIC as max.  This
 * function may be called with with P_NO_TYPE.  Used in pray.c.
 */
void
unrestrict_weapon_skill(skill)
int skill;
{
    if (skill < P_NUM_SKILLS && P_RESTRICTED(skill)) {
	P_SKILL(skill) = P_UNSKILLED;
	P_MAX_SKILL(skill) = P_BASIC;
	P_ADVANCE(skill) = 0;
    }
}

#endif /* WEAPON_SKILLS */

#endif /* OVL1 */
#ifdef OVLB

#ifdef WEAPON_SKILLS

void
use_skill(skill)
int skill;
{
    boolean advance_before;

    if (skill != P_NO_TYPE && !P_RESTRICTED(skill)) {
	advance_before = can_advance(skill);
	P_ADVANCE(skill)++;
	if (!advance_before && can_advance(skill))
	    You(may_advance_msg);
    }
}

void
add_weapon_skill()
{
    int i, before, after;

    for (i = 0, before = 0; i < P_NUM_SKILLS; i++)
	if (can_advance(i)) before++;
    u.weapon_slots++;
    for (i = 0, after = 0; i < P_NUM_SKILLS; i++)
	if (can_advance(i)) after++;
    if (before < after)
	You(may_advance_msg);
}

void
lose_weapon_skill()
{
    int skill;

    /* deduct first from unused slots, then from last placed slot, if any */
    if (u.weapon_slots) {
	u.weapon_slots--;
    } else if (u.skills_advanced) {
	skill = u.skill_record[--u.skills_advanced];
	if (P_SKILL(skill) <= P_UNSKILLED)
	    panic("lose_weapon_skill");

	P_SKILL(skill)--;	/* drop skill one level */

	/* Some skills take more than one slot, refund the rest. */
	if (skill <= P_LAST_WEAPON || skill == P_TWO_WEAPON_COMBAT)
	    u.weapon_slots = P_SKILL(skill) - 1;
	else if (P_SKILL(skill) >= 5)
	    u.weapon_slots = 1;
    }
}

int
weapon_type(obj)
struct obj *obj;
{
    int type;

    if (obj) {
	switch (obj->otyp) {
	    case DAGGER:		case ELVEN_DAGGER:
	    case ORCISH_DAGGER:		case ATHAME:
		type = P_DAGGER; break;
	    case KNIFE:			case STILETTO:
	    case WORM_TOOTH:		case CRYSKNIFE:
	    case SCALPEL:
		type = P_KNIFE; break;
	    case AXE:			case BATTLE_AXE:
		type = P_AXE; break;
	    case DWARVISH_MATTOCK:
	    case PICK_AXE:
		type = P_PICK_AXE; break;
	    case SHORT_SWORD:		case ELVEN_SHORT_SWORD:
	    case ORCISH_SHORT_SWORD:	case DWARVISH_SHORT_SWORD:
		type = P_SHORT_SWORD; break;
	    case BROADSWORD:		case ELVEN_BROADSWORD:
	    case RUNESWORD:
		type = P_BROAD_SWORD; break;
	    case LONG_SWORD:		case KATANA:
		type = P_LONG_SWORD; break;
	    case TWO_HANDED_SWORD:	case TSURUGI:
		type = P_TWO_HANDED_SWORD; break;
	    case SCIMITAR:
		type = P_SCIMITAR; break;
	    case SILVER_SABER:
		type = P_SABER; break;
	    case CLUB:			case AKLYS:
		type = P_CLUB; break;
	    case MACE:
		type = P_MACE; break;
	    case MORNING_STAR:
		type = P_MORNING_STAR; break;
	    case FLAIL:
		type = P_FLAIL; break;
	    case WAR_HAMMER:
		type = P_HAMMER; break;
	    case QUARTERSTAFF:
		type = P_QUARTERSTAFF; break;
	    case PARTISAN:		case RANSEUR:
	    case SPETUM:		case GLAIVE:
	    case HALBERD:		case BARDICHE:
	    case VOULGE:		case FAUCHARD:
	    case GUISARME:		case BILL_GUISARME:
	    case LUCERN_HAMMER:		case BEC_DE_CORBIN:
		type = P_POLEARMS; break;
	    case SPEAR:			case ELVEN_SPEAR:
	    case ORCISH_SPEAR:		case DWARVISH_SPEAR:
		type = P_SPEAR; break;
	    case JAVELIN:
		type = P_JAVELIN; break;
	    case TRIDENT:
		type = P_TRIDENT; break;
	    case LANCE:
		type = P_LANCE; break;
	    case BOW:			case ELVEN_BOW:
	    case ORCISH_BOW:		case YUMI:
		type = P_BOW; break;
	    case SLING:
		type = P_SLING; break;
	    case CROSSBOW:
		type = P_CROSSBOW; break;
	    case DART:
		type = P_DART; break;
	    case SHURIKEN:
		type = P_SHURIKEN; break;
	    case BOOMERANG:
		type = P_BOOMERANG; break;
	    case BULLWHIP:
#ifdef KOPS
	    case RUBBER_HOSE:
#endif
		type = P_WHIP; break;
	    case UNICORN_HORN:
		type = P_UNICORN_HORN; break;
	    default:
		type = P_NO_TYPE; break;
	}
	return type;
    }

    /* no object => */
    return P_BARE_HANDED_COMBAT;
}

/*
 * Return hit bonus/penalty based on skill of weapon.
 * Treat restricted weapons as unskilled.
 */
int
weapon_hit_bonus(weapon)
struct obj *weapon;
{
    int type, bonus = 0;
    static const char bad_skill[] = "weapon_hit_bonus: bad skill %d";

    type = weapon_type(weapon);
    if (type == P_NO_TYPE) {
	bonus = 0;
    } else if (type <= P_LAST_WEAPON) {
	switch (P_SKILL(type)) {
	    default: impossible(bad_skill, P_SKILL(type)); /* fall through */
	    case P_ISRESTRICTED:
	    case P_UNSKILLED:   bonus = -4; break;
	    case P_BASIC:       bonus =  0; break;
	    case P_SKILLED:     bonus =  2; break;
	    case P_EXPERT:      bonus =  3; break;
	}
    } else if (type == P_TWO_WEAPON_COMBAT) {
	switch (P_SKILL(type)) {
	    default: impossible(bad_skill, P_SKILL(type)); /* fall through */
	    case P_ISRESTRICTED:
	    case P_UNSKILLED:   bonus = -9; break;
	    case P_BASIC:	bonus = -7; break;
	    case P_SKILLED:	bonus = -5; break;
	    case P_EXPERT:	bonus = -3; break;
	}
    } else if (type == P_BARE_HANDED_COMBAT) {
	/* restricted == 0 */
	bonus = ((P_SKILL(type) + 1) * (martial_bonus() ? 2 : 1)) / 2;
    }
    return bonus;
}

/*
 * Return damage bonus/penalty based on skill of weapon.
 * Treat restricted weapons as unskilled.
 */
int
weapon_dam_bonus(weapon)
struct obj *weapon;
{
    int type, bonus = 0;

    type = weapon_type(weapon);
    if (type == P_NO_TYPE) {
	bonus = 0;
    } else if (P_RESTRICTED(type) || type <= P_LAST_WEAPON) {
	switch (P_SKILL(type)) {
	    default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type));
		     /* fall through */
	    case P_ISRESTRICTED:
	    case P_UNSKILLED:	bonus = -2; break;
	    case P_BASIC:	bonus =  0; break;
	    case P_SKILLED:	bonus =  1; break;
	    case P_EXPERT:	bonus =  2; break;
	}
    } else if (type == P_BARE_HANDED_COMBAT && P_SKILL(type)) {
	bonus = (P_SKILL(type) * (martial_bonus() ? 2 : 1)) / 2;
    }
    return bonus;
}

/*
 * Initialize weapon skill array for the game.  Start by setting all
 * skills to restricted, then set the skill for every weapon the
 * hero is holding, finally reading the given array that sets
 * maximums.
 */
void
skill_init(class_skill)
struct def_skill *class_skill;
{
	struct obj *obj;
	int skmax, skill;

	/* initialize skill array; by default, everything is restricted */
	for (skill = 0; skill < P_NUM_SKILLS; skill++) {
	    P_SKILL(skill) = P_ISRESTRICTED;
	    P_MAX_SKILL(skill) = P_ISRESTRICTED;
	    P_ADVANCE(skill) = 0;
	}

	/* set skill for all weapons in inventory to be basic */
	for (obj = invent; obj; obj = obj->nobj) {
	    skill = weapon_type(obj);
	    if (skill != P_NO_TYPE)
		P_SKILL(skill) = P_BASIC;
	}

	/* walk through array to set skill maximums */
	for (; class_skill->skill != P_NO_TYPE; class_skill++) {
	    skmax = class_skill->skmax;
	    skill = class_skill->skill;

	    P_MAX_SKILL(skill) = skmax;
	    if (P_SKILL(skill) == P_ISRESTRICTED)	/* skill pre-set */
		P_SKILL(skill) = P_UNSKILLED;
	}

	/* High potential fighters already know how to use their hands. */
	if (P_MAX_SKILL(P_BARE_HANDED_COMBAT) > P_EXPERT)
	    P_SKILL(P_BARE_HANDED_COMBAT) = P_BASIC;

	/*
	 * Make sure we haven't missed setting the max on a skill
	 * & set advance
	 */
	for (skill = 0; skill < P_NUM_SKILLS; skill++) {
	    if (!P_RESTRICTED(skill)) {
		if (P_MAX_SKILL(skill) < P_SKILL(skill)) {
		    impossible("skill_init: curr > max: %s", P_NAME(skill));
		    P_MAX_SKILL(skill) = P_SKILL(skill);
		}
		P_ADVANCE(skill) = practice_needed_to_advance(P_SKILL(skill)-1);
	    }
	}
}

#endif /* WEAPON_SKILLS */

#endif /* OVLB */

/*weapon.c*/
