/*	SCCS Id: @(#)artifact.c	3.1	93/01/17	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#ifdef OVLB
#include "artilist.h"
#else
STATIC_DCL const struct artifact artilist[];
#endif
/*
 * Note:  both artilist[] and artiexist[] have a dummy element #0,
 *	  so loops over them should normally start at #1.  The primary
 *	  exception is the save & restore code, which doesn't care about
 *	  the contents, just the total size.
 */

extern boolean notonhead;	/* for long worms */

#define get_artifact(o) \
		(((o)&&(o)->oartifact) ? &artilist[(int) (o)->oartifact] : 0)
STATIC_DCL int FDECL(spec_applies, (const struct artifact*,struct permonst*));
STATIC_DCL int FDECL(arti_invoke, (struct obj*));

#ifndef OVLB
STATIC_DCL int spec_dbon_applies;

#else	/* OVLB */
/* coordinate effects from spec_dbon() with messages in artifact_hit() */
STATIC_OVL int spec_dbon_applies = 0;

/* flags including which artifacts have already been created */
static boolean artiexist[1+NROFARTIFACTS+1];

static boolean FDECL(attacks, (int,struct obj *));

/* zero out the artifact existence list */
void
init_artifacts()
{
	(void) memset((genericptr_t) artiexist, 0, sizeof artiexist);
}

void
save_artifacts(fd)
int fd;
{
	bwrite(fd, (genericptr_t) artiexist, sizeof artiexist);
}

void
restore_artifacts(fd)
int fd;
{
	mread(fd, (genericptr_t) artiexist, sizeof artiexist);
}

const char *
artiname(artinum)
int artinum;
{
	if (artinum <= 0 || artinum > NROFARTIFACTS) return("");
	return(artilist[artinum].name);
}

/*
   Make an artifact.  If a specific alignment is specified, then an object of
   the appropriate alignment is created from scratch, or NULL is returned if
   none is available.  If no alignment is given, then 'otmp' is converted
   into an artifact of matching type, or returned as-is if that's not possible.
   For the 2nd case, caller should use ``obj = mk_artifact(obj, A_NONE);''
   for the 1st, ``obj = mk_artifact(NULL, some_alignment);''.
 */
struct obj *
mk_artifact(otmp, alignment)
struct obj *otmp;	/* existing object; ignored if alignment specified */
aligntyp alignment;	/* target alignment, or A_NONE */
{
	register const struct artifact *a;
	register int n = 0, m;
	register boolean by_align = (alignment != A_NONE);
	register short o_typ = (by_align || !otmp) ? 0 : otmp->otyp;
	boolean unique = !by_align && otmp && objects[o_typ].oc_unique;

	/* count eligible artifacts */
	for (a = artilist+1,m = 1; a->otyp; a++,m++)
	    if ((by_align ? a->alignment == alignment : a->otyp == o_typ) &&
		(!(a->spfx & SPFX_NOGEN) || unique) && !artiexist[m]) {
		if (by_align && a->class == pl_character[0])
		    goto make_artif;	/* 'a' points to the desired one */
		else
		    n++;
	    }

	if (n) {		/* found at least one candidate */
	    /* select one, then find it again */
	    if (n > 1) n = rnd(n);	/* [1..n] */
	    for (a = artilist+1,m = 1; a->otyp; a++,m++)
		if ((by_align ? a->alignment == alignment : a->otyp == o_typ)&&
		    (!(a->spfx & SPFX_NOGEN) || unique) && !artiexist[m]) {
		    if (!--n) break;	/* stop when chosen one reached */
		}

	    /* make an appropriate object if necessary, then christen it */
make_artif: if (by_align) otmp = mksobj((int)a->otyp, TRUE, FALSE);
	    otmp = oname(otmp, a->name, 0);
	    otmp->oartifact = m;
	    artiexist[m] = TRUE;
	} else {
	    /* nothing appropriate could be found; return the original object */
	    if (by_align) otmp = 0;	/* (there was no original object) */
	}
	return otmp;
}

/*
 * Returns the full name (with articles and correct capitalization) of an
 * artifact named "name" if one exists, or NULL, it not.
 * The given name must be rather close to the real name for it to match.
 * The object type of the artifact is returned in otyp if the return value
 * is non-NULL.
 */
const char*
artifact_name(name, otyp)
const char *name;
short *otyp;
{
    register const struct artifact *a;
    register const char *aname;

    if(!strncmpi(name, "the ", 4)) name += 4;

    for (a = artilist+1; a->otyp; a++) {
	aname = a->name;
	if(!strncmpi(aname, "the ", 4)) aname += 4;
	if(!strcmpi(name, aname)) {
	    *otyp = a->otyp;
	    return a->name;
	}
    }

    return NULL;
}

boolean
exist_artifact(otyp, name)
register int otyp;
register const char *name;
{
	register const struct artifact *a;
	register boolean *arex;

	if (otyp && *name)
	    for (a = artilist+1,arex = artiexist+1; a->otyp; a++,arex++)
		if ((int) a->otyp == otyp && !strcmp(a->name, name))
		    return *arex;
	return FALSE;
}

void
artifact_exists(otmp, name, mod)
register struct obj *otmp;
register const char *name;
register boolean mod;
{
	register const struct artifact *a;

	if (otmp && *name)
	    for (a = artilist+1; a->otyp; a++)
		if (a->otyp == otmp->otyp && !strcmp(a->name, name)) {
		    register int m = a - artilist;
		    otmp->oartifact = (char)(mod ? m : 0);
		    otmp->age = 0;
		    if(otmp->otyp == RIN_INCREASE_DAMAGE)
			otmp->spe = 0;
		    artiexist[m] = mod;
		    break;
		}
	return;
}

int
nartifact_exist()
{
    int a = 0;
    int n = SIZE(artiexist);

    while(n > 1)
	if(artiexist[--n]) a++;

    return a;
}

#endif /* OVLB */
#ifdef OVL0

boolean
spec_ability(otmp, abil)
struct obj *otmp;
unsigned abil;
{
	const struct artifact *arti = get_artifact(otmp);

	return(arti && (arti->spfx & abil));
}

#endif /* OVL0 */
#ifdef OVLB

boolean
restrict_name(otmp, name)  /* returns 1 if name is restricted for otmp->otyp */
register struct obj *otmp;
register const char *name;
{
	register const struct artifact *a;

	if (!*name) return FALSE;

		/* Since almost every artifact is SPFX_RESTR, it doesn't cost
		   us much to do the string comparison before the spfx check.
		   Bug fix:  don't name multiple elven daggers "Sting".
		 */
	for (a = artilist+1; a->otyp; a++)
	    if (a->otyp == otmp->otyp && !strcmp(a->name, name))
		return ((a->spfx & (SPFX_NOGEN|SPFX_RESTR)) != 0 ||
			otmp->quan > 1L);

	return FALSE;
}

static boolean
attacks(adtyp, otmp)
register int adtyp;
register struct obj *otmp;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return(weap->attk.adtyp == adtyp);
	return(0);
}

boolean
defends(adtyp, otmp)
register int adtyp;
register struct obj *otmp;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return(weap->defn.adtyp == adtyp);
	return(0);
}

/*
 * a potential artifact has just been worn/wielded/picked-up or
 * unworn/unwielded/dropped.  Pickup/drop only set/reset the W_ART mask.
 */
void
set_artifact_intrinsic(otmp,on,wp_mask)
register struct obj *otmp;
boolean on;
long wp_mask;
{
	long *mask = 0;
	register const struct artifact *oart = get_artifact(otmp);
	uchar dtyp;
	long spfx;
	
	if (!oart) return;

	/* effects from the defn field */
	dtyp = (wp_mask != W_ART) ? oart->defn.adtyp : oart->cary.adtyp;

	if (dtyp == AD_FIRE)
	    mask = &HFire_resistance;
	else if (dtyp == AD_COLD)
	    mask = &HCold_resistance;
	else if (dtyp == AD_ELEC)
	    mask = &HShock_resistance;
	else if (dtyp == AD_MAGM)
	    mask = &Antimagic;
	else if (dtyp == AD_DISN)
	    mask = &HDisint_resistance;

	if(mask && wp_mask == W_ART && !on) {
	    /* find out if some other artifact also confers this intrinsic */
	    /* if so, leave the mask alone */
	    register struct obj* obj;
	    for(obj = invent; obj; obj = obj->nobj)
		if(obj != otmp && obj->oartifact) {
		    register const struct artifact *art = get_artifact(obj);
		    if(art->cary.adtyp == dtyp) {
			mask = (long *) 0;
			break;
		    }
		}
	}
	if(mask) {
	    if (on) *mask |= wp_mask;
	    else *mask &= ~wp_mask;
	}

	/* intrinsics from the spfx field; there could be more than one */
	spfx = (wp_mask != W_ART) ? oart->spfx : oart->cspfx;
	if(spfx && wp_mask == W_ART && !on) {
	    /* don't change any spfx also conferred by other artifacts */
	    register struct obj* obj;
	    for(obj = invent; obj; obj = obj->nobj)
		if(obj != otmp && obj->oartifact) {
		    register const struct artifact *art = get_artifact(obj);
		    spfx &= ~art->cspfx;
		}
	}

	if (spfx & SPFX_SEARCH) {
	    if(on) Searching |= wp_mask;
	    else Searching &= ~wp_mask;
	}
	if (spfx & SPFX_HALRES) {
	    /* make_hallucinated must (re)set the mask itself to get
	     * the display right */
	    make_hallucinated((long)!on, TRUE, wp_mask);
	}
	if (spfx & SPFX_ESP) {
	    if(on) HTelepat |= wp_mask;
	    else HTelepat &= ~wp_mask;
	    see_monsters();
	}
	if (spfx & SPFX_STLTH) {
	    if (on) Stealth |= wp_mask;
	    else Stealth &= ~wp_mask;
	}
	if (spfx & SPFX_REGEN) {
	    if (on) HRegeneration |= wp_mask;
	    else HRegeneration &= ~wp_mask;
	}
	if (spfx & SPFX_TCTRL) {
	    if (on) HTeleport_control |= wp_mask;
	    else HTeleport_control &= ~wp_mask;
	}
	if (spfx & SPFX_WARN) {
	    if (on) Warning |= wp_mask;
	    else Warning &= ~wp_mask;
	}
	if (spfx & SPFX_EREGEN) {
	    if (on) Energy_regeneration |= wp_mask;
	    else Energy_regeneration &= ~wp_mask;
	}
	if (spfx & SPFX_HSPDAM) {
	    if (on) Half_spell_damage |= wp_mask;
	    else Half_spell_damage &= ~wp_mask;
	}
	if (spfx & SPFX_HPHDAM) {
	    if (on) Half_physical_damage |= wp_mask;
	    else Half_physical_damage &= ~wp_mask;
	}

	if(wp_mask == W_ART && !on && oart->inv_prop) {
	    /* might have to turn off invoked power too */
	    if (oart->inv_prop <= LAST_PROP &&
		(u.uprops[oart->inv_prop].p_flgs & W_ARTI))
		(void) arti_invoke(otmp);
	}
}

/*
 * creature (usually player) tries to touch (pick up or wield) an artifact obj.
 * Returns 0 if the object refuses to be touched.
 * This routine does not change any object chains.
 * Ignores such things as gauntlets, assuming the artifact is not
 * fooled by such trappings.
 */
int
touch_artifact(obj,mon)
    struct obj *obj;
    struct monst *mon;
{
    register const struct artifact *oart = get_artifact(obj);
    boolean badclass, badalign;
    boolean yours = (mon == &youmonst);

    if(!oart) return 1;

    badclass = (oart->class && (!yours || oart->class != pl_character[0]));
    badalign = (oart->spfx & SPFX_RESTR) &&
	((oart->alignment !=
	  (yours ? u.ualign.type : sgn(mon->data->maligntyp))) ||
	 (yours && u.ualign.record < 0));
    /*
     * hack: Excalibur allows all lawfuls to touch it, but "class" is
     * set to 'K' to allow Knights to get it via sacrifice.  This requires an
     * additional artifact field to fix, or some similar treatment. -dlc
     */
    if (obj->oartifact == ART_EXCALIBUR && !badalign) badclass = FALSE;

    if(((badclass || badalign) && (oart->spfx & SPFX_INTEL)) ||
       (badalign && (!yours || !rn2(4))))  {
	int dmg;
	char buf[BUFSZ];

	if (!yours) return 0;
	You("feel a blast of power flow from %s!", the(xname(obj)));
	dmg = d((Antimagic ? 2 : 4) , ((oart->spfx & SPFX_INTEL) ? 10 : 4));
	Sprintf(buf, "touching %s", oart->name);
	losehp(dmg, buf, KILLED_BY);
	exercise(A_WIS, FALSE);
    }

    /* can pick it up unless you're totally non-synch'd with the artifact */
    if(badclass && badalign && (oart->spfx & SPFX_INTEL)) {
	if (yours) pline("%s refuses to be held by you!", The(xname(obj)));
	return 0;
    }

    return 1;
}

#endif /* OVLB */
#ifdef OVL1

STATIC_OVL int
spec_applies(weap, ptr)
register const struct artifact *weap;
struct permonst *ptr;
{
	boolean yours = (ptr == &playermon);

	if(!(weap->spfx & (SPFX_DBONUS | SPFX_ATTK)))
	    return(weap->attk.adtyp == AD_PHYS);

	if(weap->spfx & SPFX_DMONS)
	    return((ptr == &mons[(int)weap->mtype]));
	else if(weap->spfx & SPFX_DCLAS)
	    return((weap->mtype == ptr->mlet));
	else if(weap->spfx & SPFX_DFLAG1)
	    return((ptr->mflags1 & weap->mtype) != 0L);
	else if(weap->spfx & SPFX_DFLAG2)
	    return((ptr->mflags2 & weap->mtype) != 0L);
	else if(weap->spfx & SPFX_DALIGN)
	    return(ptr->maligntyp == A_NONE ||
		   sgn(ptr->maligntyp) != sgn(weap->alignment));
	else if(weap->spfx & SPFX_ATTK) {
	    switch(weap->attk.adtyp) {
		case AD_FIRE:
			return(!(yours ? Fire_resistance : resists_fire(ptr)));
		case AD_COLD:
			return(!(yours ? Cold_resistance : resists_cold(ptr)));
		case AD_ELEC:
			return(!(yours ? Shock_resistance : resists_elec(ptr)));
		case AD_MAGM:
		case AD_STUN:
			return(!(yours ? Antimagic : (rn2(101) < ptr->mr)));
		case AD_DRLI:
			if (!yours) return(!resists_drli(ptr));
			else return(
#ifdef POLYSELF
				resists_drli(uasmon) ||
#endif
				defends(AD_DRLI, uwep));
		case AD_STON:
#ifdef POLYSELF
			if (yours) return(!resists_ston(uasmon));
			else
#endif
				return(!resists_ston(ptr));
		default:	impossible("Weird weapon special attack.");
	    }
	}
	return(0);
}

int
spec_abon(otmp, ptr)
struct obj *otmp;
struct permonst *ptr;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		if(spec_applies(weap, ptr))
		    return((weap->attk.damn) ? rnd((int)weap->attk.damn) : 0);
	return(0);
}

int
spec_dbon(otmp, ptr, tmp)
register struct obj *otmp;
register struct permonst *ptr;
register int	tmp;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
	    if ((spec_dbon_applies = spec_applies(weap, ptr)) != 0)
		return((weap->attk.damd) ? rnd((int)weap->attk.damd) : tmp);
	else spec_dbon_applies = 0;
	return(0);
}

#endif /* OVL1 */

#ifdef OVLB

/* Function used when someone attacks someone else with an artifact
 * weapon.  Only adds the special (artifact) damage, and returns a 1 if it
 * did something special (in which case the caller won't print the normal
 * hit message).  This should be called once upon every artifact attack;
 * dmgval() no longer takes artifact bonuses into account.  Possible
 * extension: change the killer so that when an orc kills you with
 * Stormbringer it's "killed by Stormbringer" instead of "killed by an orc".
 */
boolean
artifact_hit(magr, mdef, otmp, dmgptr, dieroll)
struct monst *magr, *mdef;
struct obj *otmp;
int *dmgptr;
int dieroll; /* needed for Magicbane and vorpal blades */
{
	boolean youattack = (magr == &youmonst);
	boolean youdefend = (mdef == &youmonst);
	boolean vis = (!youattack && magr && cansee(magr->mx, magr->my))
		|| (!youdefend && cansee(mdef->mx, mdef->my));
	boolean realizes_damage;

	static const char you[] = "you";
	const char *hittee = youdefend ? you : mon_nam(mdef);

	/* The following takes care of most of the damage, but not all--
	 * the exception being for level draining, which is specially
	 * handled.  Messages are done in this function, however.
	 */
	*dmgptr += spec_dbon(otmp, youdefend ? &playermon
		: mdef->data, *dmgptr);

	if (youattack && youdefend) {
		impossible("attacking yourself with weapon?");
		return FALSE;
	}

	realizes_damage = (youdefend || vis) && spec_dbon_applies;

	/* the four basic attacks: fire, cold, shock and missiles */
	if (attacks(AD_FIRE, otmp)) {
		if (realizes_damage) {
			pline("The fiery blade burns %s!", hittee);
			return TRUE;
		}
	}
	if (attacks(AD_COLD, otmp)) {
		if (realizes_damage) {
			pline("The chilling blade freezes %s!", hittee);
			return TRUE;
		}
	}
	if (attacks(AD_ELEC, otmp)) {
		if (realizes_damage) {
			if(youattack && otmp != uwep)
			    pline("%s hits %s!", The(xname(otmp)), hittee);
			pline("A bolt of lightning zaps %s!", hittee);
			return TRUE;
		}
	}
	if (attacks(AD_MAGM, otmp)) {
		if (realizes_damage) {
			if(youattack && otmp != uwep)
			    pline("%s hits %s!", The(xname(otmp)), hittee);
			pline("A hail of magic missiles strikes %s!", hittee);
			return TRUE;
		}
	}

	/*
	 * Magicbane's intrinsic magic is incompatible with normal
	 * enchantment magic.  Thus, its effects have a negative
	 * dependence on spe.  Against low mr victims, it typically
	 * does "double athame" damage, 2d4.  Occasionally, it will
	 * cast unbalancing magic which effectively averages out to
	 * 4d4 damage (2.5d4 against high mr victims), for spe = 0.
	 */

#define MB_MAX_DIEROLL		8    /* rolls above this aren't magical */
#define MB_INDEX_INIT		(-1)
#define MB_INDEX_PROBE		0
#define MB_INDEX_STUN		1
#define MB_INDEX_SCARE		2
#define MB_INDEX_PURGE		3
#define MB_RESIST_ATTACK	(resist_index = attack_index)
#define MB_RESISTED_ATTACK	(resist_index == attack_index)
#define MB_UWEP_ATTACK		(youattack && (otmp == uwep))

	if (attacks(AD_STUN, otmp) && (dieroll <= MB_MAX_DIEROLL)) {
		int attack_index = MB_INDEX_INIT;
		int resist_index = MB_INDEX_INIT;
		int scare_dieroll = MB_MAX_DIEROLL / 2;

		if (otmp->spe >= 3)
			scare_dieroll /= (1 << (otmp->spe / 3));

		*dmgptr += rnd(4);			/* 3d4 */

		if (otmp->spe > rn2(10))		/* probe */
			attack_index = MB_INDEX_PROBE;
		else {					/* stun */
			attack_index = MB_INDEX_STUN;
			*dmgptr += rnd(4);		/* 4d4 */

			if (youdefend)
				make_stunned((HStun + 3), FALSE);
			else
				mdef->mstun = 1;
		}
		if (dieroll <= scare_dieroll) {		/* scare */
			attack_index = MB_INDEX_SCARE;
			*dmgptr += rnd(4);		/* 5d4 */

			if (youdefend) {
				if (Antimagic)
					MB_RESIST_ATTACK;
				else {
					nomul(-3);
					nomovemsg = "";
#ifdef POLYSELF
					if ((magr == u.ustuck)
						&& sticks(uasmon)) {
					    u.ustuck = (struct monst *)0;
					    You("release %s!", mon_nam(magr));
					}
#endif
				}
			} else if (youattack) {
				if (rn2(2) && resist(mdef,SPBOOK_CLASS,0,0)) {
				    MB_RESIST_ATTACK;
				} else {
				    if (mdef == u.ustuck) {
					if (u.uswallow)
					    expels(mdef,mdef->data,TRUE);
					else {
#ifdef POLYSELF
					    if (!sticks(uasmon))
#endif
					    {
						u.ustuck = (struct monst *)0;
						You("get released!");
					    }
					}
				    }
				    mdef->mflee = 1;
				    mdef->mfleetim += 3;
				}
			}
		}
		if (dieroll <= (scare_dieroll / 2)) {	/* purge */
			struct obj *ospell;
#ifdef POLYSELF
			struct permonst *old_uasmon = uasmon;
#endif
			attack_index = MB_INDEX_PURGE;
			*dmgptr += rnd(4);		/* 6d4 */

			/* Create a fake spell object, ala spell.c */
			ospell = mksobj(SPE_CANCELLATION, FALSE, FALSE);
			ospell->blessed = ospell->cursed = 0;
			ospell->quan = 20L;

			cancel_monst(mdef, ospell, youattack, FALSE, FALSE);

#ifdef POLYSELF
			if (youdefend && (old_uasmon != uasmon))
				/* rehumanized, no more damage */
				*dmgptr = 0;
#endif
			if (youdefend) {
				if (Antimagic)
					MB_RESIST_ATTACK;
			} else {
				if (!mdef->mcan)
					MB_RESIST_ATTACK;

				/* cancelled clay golems will die ... */
				else if (mdef->data == &mons[PM_CLAY_GOLEM])
					mdef->mhp = 1;
			}

			obfree(ospell, (struct obj *)0);
		}

		if (youdefend || mdef->mhp > 0) {  /* ??? -dkh- */
			static const char *mb_verb[4] =
				{"probe", "stun", "scare", "purge"};

			if (youattack || youdefend || vis) {
				pline("The magic-absorbing blade %ss %s!",
					mb_verb[attack_index], hittee);

				if (MB_RESISTED_ATTACK) {
					pline("%s resist%s!",
					youdefend ? "You" : Monnam(mdef),
					youdefend ? "" : "s");

					shieldeff(youdefend ? u.ux : mdef->mx,
						youdefend ? u.uy : mdef->my);
				}
			}

			/* Much ado about nothing.  More magic fanfare! */
			if (MB_UWEP_ATTACK) {
				if (attack_index == MB_INDEX_PURGE) {
				    if (!MB_RESISTED_ATTACK &&
					attacktype(mdef->data, AT_MAGC)) {
					You("absorb magical energy!");
					u.uenmax++;
					u.uen++;
					flags.botl = 1;
				    }
				} else if (attack_index == MB_INDEX_PROBE) {
				    if (!rn2(4 * otmp->spe)) {
					pline("The probe is insightful!");
					/* pre-damage status */
					mstatusline(mdef);
				    }
				}
			} else if (youdefend && !MB_RESISTED_ATTACK
				   && (attack_index == MB_INDEX_PURGE)) {
				You("lose some magical energy!");
				if (u.uenmax > 0) u.uenmax--;
				if (u.uen > 0) u.uen--;
					flags.botl = 1;
			}

			/* all this magic is confusing ... */
			if (!rn2(12)) {
			    if (youdefend)
				make_confused((HConfusion + 4), FALSE);
			    else
				mdef->mconf = 1;

			    if (youattack || youdefend || vis)
				pline("%s %s confused.",
				      youdefend ? "You" : Monnam(mdef),
				      youdefend ? "are" : "is");
			}
		}
		return TRUE;
	}
	/* end of Magicbane code */

	/* We really want "on a natural 19 or 20" but Nethack does it in */
	/* reverse from AD&D. */
	if (spec_ability(otmp, SPFX_BEHEAD)) {
#ifdef MULDGN
	    if (otmp->oartifact == ART_TSURUGI_OF_MURAMASA && dieroll <= 2) {
		/* not really beheading, but so close, why add another SPFX */
		if (youattack && u.uswallow && mdef == u.ustuck) {
		    You("slice %s wide open!", mon_nam(mdef));
		    *dmgptr = mdef->mhp;
		    return TRUE;
		}
		if (!youdefend) {
			/* allow normal cutworm() call to add extra damage */
			if(notonhead)
			    return FALSE;

			if (bigmonst(mdef->data)) {
				if (youattack)
					You("slice deeply into %s!",
						mon_nam(mdef));
				else if (vis)
					pline("%s cuts deeply into %s!",
					      Monnam(magr), mon_nam(mdef));
				*dmgptr *= 2;
				return TRUE;
			}
			*dmgptr = mdef->mhp;
			pline("The razorsharp blade cuts %s in half!",
			      mon_nam(mdef));
			otmp->dknown = TRUE;
			return TRUE;
		} else {
#ifdef POLYSELF
			if (bigmonst(uasmon)) {
				pline("%s cuts deeply into you!",
					Monnam(magr));
				*dmgptr *= 2;
				return TRUE;
			}
#endif
			*dmgptr = u.uhp;
			pline("The razorsharp blade cuts you in half!");
			otmp->dknown = TRUE;
			return TRUE;
		}
	    } else 
#endif /* MULDGN */
	        if (otmp->oartifact == ART_VORPAL_BLADE &&
			(dieroll <= 2 || mdef->data == &mons[PM_JABBERWOCK])) {
		if (youattack && u.uswallow && mdef == u.ustuck)
			return FALSE;
		if (!youdefend) {
			if (!has_head(mdef->data) || notonhead) {
				if (youattack)
					pline("Somehow you miss %s wildly.",
						mon_nam(mdef));
				else if (vis)
					pline("Somehow %s misses wildly.",
						mon_nam(magr));
				*dmgptr = 0;
				return (youattack || vis);
			}
			*dmgptr = mdef->mhp;
			pline("%s cuts off %s head!",
					artilist[ART_VORPAL_BLADE].name,
					s_suffix(mon_nam(mdef)));
			otmp->dknown = TRUE;
			return TRUE;
		} else {
#ifdef POLYSELF
			if (!has_head(uasmon)) {
				pline("Somehow %s misses you wildly.",
					mon_nam(magr));
				*dmgptr = 0;
				return TRUE;
			}
#endif
			*dmgptr = u.uhp;
			pline("%s cuts off your head!",
					artilist[ART_VORPAL_BLADE].name);
			otmp->dknown = TRUE;
			/* Should amulets fall off? */
			return TRUE;
		}
	    }
	}
	if (spec_ability(otmp, SPFX_DRLI)) {
		if (!youdefend && !resists_drli(mdef->data)) {
			if (vis) {
			    if(otmp->oartifact == ART_STORMBRINGER)
				pline("The %s blade draws the life from %s!",
				      Hallucination ? hcolor() : Black,
				      mon_nam(mdef));
			    else
				pline("%s draws the life from %s!",
				      The(distant_name(otmp, xname)),
				      mon_nam(mdef));
			}
			if (mdef->m_lev == 0) *dmgptr = mdef->mhp;
			else {
			    int drain = rnd(8);
			    *dmgptr += drain;
			    mdef->mhpmax -= drain;
			    mdef->m_lev--;
			    drain /= 2;
			    if (drain) healup(drain, 0, FALSE, FALSE);
			}
			return vis;
		} else if (youdefend
#ifdef POLYSELF
					&& !resists_drli(uasmon)
#endif
					&& !defends(AD_DRLI, uwep)) {
			if (Blind)
				You("feel an %s drain your life!",
				    otmp->oartifact == ART_STORMBRINGER ?
				    "unholy blade" : "object");
			else {
			    if(otmp->oartifact == ART_STORMBRINGER)
				pline("The %s blade drains your life!",
					Hallucination ? hcolor() : Black);
			    else
				pline("%s drains your life!",
				      The(distant_name(otmp, xname)));
			}
			losexp();
			if (magr->mhp < magr->mhpmax) {
			    magr->mhp += rnd(4);
			    /* TODO: Should be related to # of HP you lost. */
			    if (magr->mhp > magr->mhpmax) magr->mhp = magr->mhpmax;
			}
			return TRUE;
		}
	}
	return FALSE;
}

static const char recharge_type[] = { ALLOW_COUNT, ALL_CLASSES, 0 };
static const char NEARDATA invoke_types[] =
	{ ALL_CLASSES, WEAPON_CLASS, ARMOR_CLASS, RING_CLASS, AMULET_CLASS,
	      TOOL_CLASS, 0 };

int
doinvoke()
{
    register struct obj *obj;

    obj = getobj(invoke_types, "invoke");
    if(!obj) return 0;
    return arti_invoke(obj);
}

STATIC_OVL int
arti_invoke(obj)
    register struct obj *obj;
{
    register const struct artifact *oart = get_artifact(obj);

    if(!oart || !oart->inv_prop) {
	if(obj->otyp == CRYSTAL_BALL)
	    use_crystal_ball(obj);
	else
	    pline("Nothing happens.");
	return 1;
    }

    if(oart->inv_prop > LAST_PROP) {
	/* It's a special power, not "just" a property */
	if(obj->age > monstermoves) {
	    /* the artifact is tired :-) */
	    You("feel that %s is ignoring you.", the(xname(obj)));
	    return 1;
	}
	obj->age = monstermoves + rnz(100);

	switch(oart->inv_prop) {
	case TAMING: {
	    struct obj *pseudo = mksobj(SPE_CHARM_MONSTER, FALSE, FALSE);
	    pseudo->blessed = pseudo->cursed = 0;
	    pseudo->quan = 20L;			/* do not let useup get it */
	    (void) seffects(pseudo);
	    obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
	    break;
	  }
	case HEALING: {
	    int healamt = (u.uhpmax + 1 - u.uhp) / 2;
	    if(healamt || Sick || (Blinded > 1))
		You("feel better.");
	    else
		goto nothing_special;
	    if(healamt) u.uhp += healamt;
	    if(Sick) make_sick(0L,FALSE);
	    if(Blinded > 1) make_blinded(0L,FALSE);
	    flags.botl = 1;
	    break;
	  }
	case ENERGY_BOOST: {
	    int epboost = (u.uenmax + 1 - u.uen) / 2;
	    if(epboost) {
		You("feel re-energized.");
		u.uen += epboost;
	    } else
		goto nothing_special;
	    break;
	  }
	case UNTRAP: {
	    if(!untrap(TRUE)) {
		obj->age = 0; /* don't charge for changing their mind */
		return 0;
	    }
	    break;
	  }
	case CHARGE_OBJ: {
	    struct obj *otmp = getobj(recharge_type, "charge");
	    if (!otmp) {
		obj->age = 0;
		return 0;
	    }
	    recharge(otmp, obj->blessed ? 1 : obj->cursed ? -1 : 0);
	    break;
	  }
	case LEV_TELE:
	    level_tele();
	    break;
	case CREATE_PORTAL: {
	    register int i;
	    d_level newlev;
	    char buf[BUFSIZ];
	    extern int n_dgns; /* from dungeon.c */
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    char hc;

	    start_menu(tmpwin);
	    add_menu(tmpwin, 0, 0, "Dungeons:");
	    add_menu(tmpwin, 0, 0, "");
	    for (i = 0, hc = 'a'; i < n_dgns; i++) {
		if (!dungeons[i].dunlev_ureached) continue;
		Sprintf(buf, "%c - %s", hc, dungeons[i].dname);
		add_menu(tmpwin, hc, 0, buf);
		hc++;
	    }
	    add_menu(tmpwin, 0, 0, "");
	    end_menu(tmpwin, '\033', "\033","Open a portal to which dungeon?");
	    if (hc > 'b') {
		/* more than one entry; display menu for choices */
		hc = select_menu(tmpwin);
	    } else
		hc = 'a';
	    destroy_nhwindow(tmpwin);

	    /* assume there won't be more than 26 dungeons */
	    if (hc < 'a' || hc > 'z')
		goto nothing_special;

	    /* run thru dungeon array to find the one they selected */
	    for (i = 0; hc >= 'a'; i++)
		if (dungeons[i].dunlev_ureached) hc--;
	    i--; /* we added one extra */

	    /*
	     * i is now index into dungeon structure for the new dungeon.
	     * Find the closest level in the given dungeon, open
	     * a use-once portal to that dungeon and go there.
	     * The closest level is either the entry or dunlev_ureached.
	     */
	    newlev.dnum = i;
	    if(dungeons[i].depth_start >= depth(&u.uz))
		newlev.dlevel = dungeons[i].entry_lev;
	    else
		newlev.dlevel = dungeons[i].dunlev_ureached;
	    if(u.uhave.amulet || In_endgame(&u.uz) || In_endgame(&newlev) ||
	       newlev.dnum == u.uz.dnum) {
		You("feel very disoriented for a moment.");
	    } else {
		if(!Blind) You("are surrounded by a shimmering sphere!");
		else You("momentarily feel weightless.");
		goto_level(&newlev, FALSE, FALSE, FALSE);
	    }
	    break;
	  }
	}
    } else {
	boolean on;
	unsigned long cprop;
	cprop = u.uprops[oart->inv_prop].p_flgs ^= W_ARTI;
	on = (cprop & W_ARTI) != 0; /* did we just turn on the invoked prop? */

	if(on && obj->age > monstermoves) {
	    /* the artifact is tired :-) */
	    u.uprops[oart->inv_prop].p_flgs ^= W_ARTI;
	    You("feel that %s is ignoring you.", the(xname(obj)));
	    return 1;
	} else if(!on) {
	    /* when turning off property, determine downtime */
	    /* arbitrary for now until we can tune this -dlc */
	    obj->age = monstermoves + rnz(100);
	}

	if(cprop & ~W_ARTI) {
	nothing_special:
	    /* you had the property from some other source too */
	    if (carried(obj))
		You("feel a surge of power, but notice no effect.");
	    return 1;
	}
	switch(oart->inv_prop) {
	case CONFLICT:
	    if(on) You("feel like a rabble-rouser.");
	    else You("feel the tension decrease in your vicinity.");
	    break;
	case LEVITATION:
	    if(on) float_up();
	    else (void) float_down();
	    break;
	case INVIS:
	    if (!See_invisible && !Blind) {
		newsym(u.ux,u.uy);
		if (on) {
		    Your("body takes on a %s transparency...",
			 Hallucination ? "normal" : "strange");
		} else {
		    Your("body seems to unfade...");
		}
	    } else goto nothing_special;
	    break;
	}
    }

    return 1;
}

#endif /* OVLB */

/*artifact.c*/
