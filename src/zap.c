/*	SCCS Id: @(#)zap.c	3.2	96/05/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Disintegration rays have special treatment; corpses are never left.
 * But the routine which calculates the damage is separate from the routine
 * which kills the monster.  The damage routine returns this cookie to
 * indicate that the monster should be disintegrated.
 */
#define MAGIC_COOKIE 1000

#ifdef OVLB
static NEARDATA boolean obj_zapped;
static NEARDATA int poly_zapped;
#endif

extern boolean notonhead;	/* for long worms */

/* kludge to use mondied instead of killed */
extern boolean m_using;

STATIC_DCL void FDECL(costly_cancel, (struct obj *));
STATIC_DCL void FDECL(polyuse, (struct obj*, int, int));
STATIC_DCL void FDECL(create_polymon, (struct obj *));
STATIC_DCL boolean FDECL(zap_updown, (struct obj *));
STATIC_DCL int FDECL(zhitm, (struct monst *,int,int));
STATIC_DCL void FDECL(zhitu, (int,int,const char *,XCHAR_P,XCHAR_P));
STATIC_DCL void FDECL(revive_egg, (struct obj *));

#ifdef OVLB
static int FDECL(zap_hit, (int));
#endif
#ifdef OVL0
static void FDECL(backfire, (struct obj *));
#endif

#define ZT_MAGIC_MISSILE	(AD_MAGM-1)
#define ZT_FIRE			(AD_FIRE-1)
#define ZT_COLD			(AD_COLD-1)
#define ZT_SLEEP		(AD_SLEE-1)
#define ZT_DEATH		(AD_DISN-1)	/* or disintegration */
#define ZT_LIGHTNING		(AD_ELEC-1)
#define ZT_POISON_GAS		(AD_DRST-1)
#define ZT_ACID			(AD_ACID-1)
/* 8 and 9 are currently unassigned */

#define ZT_WAND(x)		(x)
#define ZT_SPELL(x)		(10+(x))
#define ZT_BREATH(x)	(20+(x))

#ifndef OVLB
STATIC_VAR const char are_blinded_by_the_flash[];
extern const char *flash_types[];
#else
STATIC_VAR const char are_blinded_by_the_flash[] = "are blinded by the flash!";

const char *flash_types[] = {		/* also used in buzzmu(mcastu.c) */
	"magic missile",	/* Wands must be 0-9 */
	"bolt of fire",
	"bolt of cold",
	"sleep ray",
	"death ray",
	"bolt of lightning",
	"",
	"",
	"",
	"",

	"magic missile",	/* Spell equivalents must be 10-19 */
	"fireball",
	"cone of cold",
	"sleep ray",
	"finger of death",
	"bolt of lightning",	/* There is no spell, used for retribution */
	"",
	"",
	"",
	"",

	"blast of missiles",	/* Dragon breath equivalents 20-29*/
	"blast of fire",
	"blast of frost",
	"blast of sleep gas",
	"blast of disintegration",
	"blast of lightning",
	"blast of poison gas",
	"blast of acid",
	"",
	""
};

/* Routines for IMMEDIATE wands and spells. */
/* bhitm: monster mtmp was hit by the effect of wand or spell otmp */
int
bhitm(mtmp, otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	register boolean wake = TRUE;	/* Most 'zaps' should wake monster */
	register int otyp = otmp->otyp;
	boolean dbldam = Role_is('K') && u.uhave.questart;
	register int dmg;

	switch(otyp) {
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
		if (resists_magm(mtmp)) {	/* match effect on player */
			shieldeff(mtmp->mx, mtmp->my);
			break;	/* skip makeknown */
		} else if (u.uswallow || rnd(20) < 10 + find_mac(mtmp)) {
			dmg = d(2,12);
			if(dbldam) dmg *= 2;
			hit((otyp == WAN_STRIKING) ? "wand" : "spell",
			    mtmp, exclam(dmg));
			(void) resist(mtmp, otmp->oclass, dmg, TELL);
		} else miss((otyp == WAN_STRIKING) ? "wand" : "spell", mtmp);
		makeknown(otyp);
		break;
	case WAN_SLOW_MONSTER:
	case SPE_SLOW_MONSTER:
		if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
			if (mtmp->mspeed == MFAST) mtmp->mspeed = 0;
			else mtmp->mspeed = MSLOW;
			if (u.uswallow && (mtmp == u.ustuck) &&
			    is_whirly(mtmp->data)) {
				You("disrupt %s!", mon_nam(mtmp));
				pline("A huge hole opens up...");
				expels(mtmp, mtmp->data, TRUE);
			}
		}
		break;
	case WAN_SPEED_MONSTER:
		if (!resist(mtmp, otmp->oclass, 0, NOTELL))
			if (mtmp->mspeed == MSLOW) mtmp->mspeed = 0;
			else mtmp->mspeed = MFAST;
		break;
	case WAN_UNDEAD_TURNING:
	case SPE_TURN_UNDEAD:
		wake = FALSE;
		if (unturn_dead(mtmp)) wake = TRUE;
		if (is_undead(mtmp->data)) {
			wake = TRUE;
			dmg = rnd(8);
			if(dbldam) dmg *= 2;
			if(!resist(mtmp, otmp->oclass, dmg, NOTELL))
				mtmp->mflee = TRUE;
		}
		break;
	case WAN_POLYMORPH:
	case SPE_POLYMORPH:
		if (resists_magm(mtmp)) {
		    /* magic resistance protects from polymorph traps, so make
		       it guard against involuntary polymorph attacks too... */
		    shieldeff(mtmp->mx, mtmp->my);
		} else if (!resist(mtmp, otmp->oclass, 0, NOTELL)) {
		    if (!rn2(25)) {
			if (canseemon(mtmp)) {
			    pline("%s shudders!", Monnam(mtmp));
			    makeknown(otyp);
			}
			/* no corpse after system shock */
			xkilled(mtmp, 3);
		    }
		    else if (newcham(mtmp, (struct permonst *)0) )
			if (!Hallucination && canspotmon(mtmp))
			    makeknown(otyp);
		}
		break;
	case WAN_CANCELLATION:
	case SPE_CANCELLATION:
		cancel_monst(mtmp, otmp, TRUE, TRUE, FALSE);
		break;
	case WAN_TELEPORTATION:
	case SPE_TELEPORT_AWAY:
		if(mtmp->ispriest && *in_rooms(mtmp->mx, mtmp->my, TEMPLE)) {
		    pline("%s resists your magic!", Monnam(mtmp));
		    break;
		}
		rloc(mtmp);
		break;
	case WAN_MAKE_INVISIBLE:
		mon_set_minvis(mtmp);
		break;
	case WAN_NOTHING:
		wake = FALSE;
		break;
	case WAN_PROBING:
		wake = FALSE;
		probe_monster(mtmp);
		makeknown(otyp);
		break;
	case WAN_OPENING:
	case SPE_KNOCK:
		wake = FALSE;	/* don't want immediate counterattack */
		if(u.uswallow && mtmp == u.ustuck) {
			if (is_animal(mtmp->data)) {
				if (Blind) You_feel("a sudden rush of air!");
				else pline("%s opens its mouth!", Monnam(mtmp));
			}
			expels(mtmp, mtmp->data, TRUE);
		}
			break;
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
		wake = FALSE;		/* wakeup() makes the target angry */
		mtmp->mhp += d(6, otyp == SPE_EXTRA_HEALING ? 8 : 4);
		if (mtmp->mhp > mtmp->mhpmax)
		    mtmp->mhp = mtmp->mhpmax;
		if (canseemon(mtmp))
		    pline("%s looks%s better.", Monnam(mtmp),
			  otyp == SPE_EXTRA_HEALING ? " much" : "" );
		if (mtmp->mtame || mtmp->mpeaceful) {
		    adjalign(Role_is('H') ? 1 : sgn(u.ualign.type));
		}
		break;
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
		wake = FALSE;
		break;
	case WAN_LIGHT:	/* (broken wand) */
		if (flash_hits_mon(mtmp, otmp))
		    makeknown(WAN_LIGHT);
		break;
	case WAN_SLEEP:	/* (broken wand) */
		/* [wakeup() doesn't rouse victims of temporary sleep,
		    so it's okay to leave `wake' set to TRUE here] */
		if (sleep_monst(mtmp, d(1 + otmp->spe, 12), WAND_CLASS))
		    slept_monst(mtmp);
		if (!Blind) makeknown(WAN_SLEEP);
		break;
	default:
		impossible("What an interesting effect (%d)", otyp);
		break;
	}
	if(wake) {
	    if(mtmp->mhp > 0) {
		wakeup(mtmp);
		m_respond(mtmp);
		if(mtmp->isshk && !*u.ushops) hot_pursuit(mtmp);
	    } else if(mtmp->m_ap_type)
		seemimic(mtmp); /* might unblock if mimicing a boulder/door */
	}
	return 0;
}

void
probe_monster(mtmp)
struct monst *mtmp;
{
	struct obj *otmp;

	mstatusline(mtmp);
	if (notonhead) return;	/* don't show minvent for long worm tail */

	if (mtmp->minvent || mtmp->mgold) {
	    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		otmp->dknown = 1;	/* treat as "seen" */
	    (void) display_minventory(mtmp, MINV_ALL);
	} else {
	    pline("%s is not carrying anything.", Monnam(mtmp));
	}
}

#endif /*OVLB*/
#ifdef OVL1

/*
 * Return the object's physical location.  This only makes sense for
 * objects that are currently on the level (i.e. migrating objects
 * are nowhere).  By default, only things that can be seen (in hero's
 * inventory, monster's inventory, or on the ground) are reported.
 * By adding BURIED_TOO and/or CONTAINED_TOO flags, you can also get
 * the location of buried and contained objects.  Note that if an
 * object is carried by a monster, its reported position may change
 * from turn to turn.  This function returns FALSE if the position
 * is not available or subject to the constraints above.
 */
boolean
get_obj_location(obj, xp, yp, locflags)
struct obj *obj;
xchar *xp, *yp;
int locflags;
{
	switch (obj->where) {
	    case OBJ_INVENT:
		*xp = u.ux;
		*yp = u.uy;
		return TRUE;
	    case OBJ_FLOOR:
		*xp = obj->ox;
		*yp = obj->oy;
		return TRUE;
	    case OBJ_MINVENT:
		if (obj->ocarry->mx) {
		    *xp = obj->ocarry->mx;
		    *yp = obj->ocarry->my;
		    return TRUE;
		}
		break;	/* !mx => migrating monster */
	    case OBJ_BURIED:
		if (locflags & BURIED_TOO) {
		    *xp = obj->ox;
		    *yp = obj->oy;
		    return TRUE;
		}
		break;
	    case OBJ_CONTAINED:
		if (locflags & CONTAINED_TOO)
		    return get_obj_location(obj->ocontainer, xp, yp, locflags);
		break;
	}
	*xp = *yp = 0;
	return FALSE;
}

boolean
get_mon_location(mon, xp, yp, locflags)
struct monst *mon;
xchar *xp, *yp;
int locflags;	/* non-zero means get location even if monster is buried */
{
	if (mon == &youmonst) {
	    *xp = u.ux;
	    *yp = u.uy;
	    return TRUE;
	} else if (mon->mx > 0 && (!mon->mburied || locflags)) {
	    *xp = mon->mx;
	    *yp = mon->my;
	    return TRUE;
	} else {	/* migrating or buried */
	    *xp = *yp = 0;
	    return FALSE;
	}
}

/*
 * Attempt to revive the given corpse, return the revived monster if
 * successful.  Note: this does NOT use up the corpse if it fails.
 */
struct monst *
revive(obj)
register struct obj *obj;
{
	register struct monst *mtmp = (struct monst *)0;

	if(obj->otyp == CORPSE) {
		int montype = obj->corpsenm;
		xchar x, y;

		/* only for invent, minvent, or floor */
		if (!get_obj_location(obj, &x, &y, 0))
		    return (struct monst *) 0;

		if (MON_AT(x,y)) {
		    coord new_xy;

		    if (enexto(&new_xy, x, y, &mons[montype]))
			x = new_xy.x,  y = new_xy.y;
		}

		if(cant_create(&montype)) {
			/* make a zombie or worm instead */
			mtmp = makemon(&mons[montype], x, y, NO_MM_FLAGS);
			if (mtmp) {
				mtmp->mhp = mtmp->mhpmax = 100;
				mtmp->mspeed = MFAST;
			}
		} else {
#ifdef OEXTRA
		    struct monst *mtraits = (struct monst *)0;

		    if (obj->oxlth && obj->mtraits) mtraits = get_mtraits(obj);
		    if (mtraits) {
		    	struct monst *mtmp2;
		    	mtmp = makemon(mtraits->data, x, y, NO_MINVENT);
			mtmp2 = newmonst(mtraits->mxlth + mtraits->mnamelth);
			*mtmp2 = *mtraits;
			(void) memcpy((genericptr_t) mtmp2->mextra,
				(genericptr_t) mtraits->mextra,
				mtraits->mxlth);
			/* Fix up the monster's hit points */
			mtmp2->mhp = mtmp2->mhpmax;
			/* Now fix some other things up*/
			mtmp2->minvent = (struct obj *)0;
			mtmp2->mx = mtmp->mx;
			mtmp2->my = mtmp->my;
			mtmp2->mux = mtmp->mux;
			mtmp2->muy = mtmp->muy;
			mtmp2->mw = mtmp->mw;
			mtmp2->wormno = mtmp->wormno;
			replmon(mtmp,mtmp2);
			/* the object name overrides original monster name */
			if (!obj->onamelth && mtmp2->mnamelth)
				mtmp2 = christen_monst(mtmp2, NAME(mtraits));
			mtmp = mtmp2;
		    } else {
#endif
			mtmp = makemon(&mons[montype], x, y, NO_MINVENT);
#ifdef OEXTRA
		    }
#endif
		    if (mtmp) {
			/* Monster retains its name */
			if (obj->onamelth)
			    mtmp = christen_monst(mtmp, ONAME(obj));
		    }
		}
		if (mtmp) {
			if (obj->oeaten)
				mtmp->mhp = eaten_stat(mtmp->mhp, obj);

			switch (obj->where) {
			    case OBJ_INVENT:
				useup(obj);
				break;
			    case OBJ_FLOOR:
				/* in case MON_AT+enexto for invisible mon */
				x = obj->ox,  y = obj->oy;
				/* not useupf(), which charges */
				if (obj->quan > 1L)
				    (void) splitobj(obj, 1L);
				delobj(obj);
				newsym(x, y);
				break;
			    case OBJ_MINVENT:
				m_useup(obj->ocarry, obj);
				break;
			    default:
				panic("revive");
			}
		}
	}
	return mtmp;
}

STATIC_OVL void
revive_egg(obj)
struct obj *obj;
{
	
	/*
	 * Note: generic eggs with corpsenm set to NON_PM will never hatch.
	 */
	if (obj->otyp != EGG) return;
	if (obj->corpsenm != NON_PM) attach_egg_hatch_timeout(obj);
}

/* try to revive all corpses and eggs carried by `mon' */
int
unturn_dead(mon)
struct monst *mon;
{
	struct obj *otmp, *otmp2;
	struct monst *mtmp2;
	char owner[BUFSIZ], corpse[BUFSIZ];
	boolean youseeit;
	int once = 0, res = 0;

	youseeit = (mon == &youmonst) ? TRUE : canseemon(mon);
	otmp2 = (mon == &youmonst) ? invent : mon->minvent;

	while ((otmp = otmp2) != 0) {
	    otmp2 = otmp->nobj;
	    if (otmp->otyp == EGG)
		revive_egg(otmp);
	    if (otmp->otyp != CORPSE) continue;
	    /* save the name; the object is liable to go away */
	    if (youseeit) Strcpy(corpse, corpse_xname(otmp, TRUE));

	    /* for a merged group, only one is revived; should this be fixed? */
	    if ((mtmp2 = revive(otmp)) != 0) {
		++res;
		if (youseeit) {
		    if (!once++) Strcpy(owner,
					(mon == &youmonst) ? "Your" :
					s_suffix(Monnam(mon)));
		    pline("%s %s suddenly comes alive!", owner, corpse);
		} else if (canseemon(mtmp2))
		    pline("%s suddenly appears!", Amonnam(mtmp2));
	    }
	}
	return res;
}
#endif /*OVL1*/

#ifdef OVLB
static const char charged_objs[] = { WAND_CLASS, WEAPON_CLASS, ARMOR_CLASS, 0 };

STATIC_OVL void
costly_cancel(obj)
register struct obj *obj;
{
	char objroom;
	struct monst *shkp = (struct monst *)0;

	if (obj->no_charge) return;

	switch (obj->where) {
	case OBJ_INVENT:
		if (obj->unpaid) {
		    shkp = shop_keeper(*u.ushops);
		    if (!shkp) return;
		    Norep("You cancel an unpaid object, you pay for it!");
		    bill_dummy_object(obj);
		}
		break;
	case OBJ_FLOOR:
		objroom = *in_rooms(obj->ox, obj->oy, SHOPBASE);
		shkp = shop_keeper(objroom);
		if (!shkp || !inhishop(shkp)) return;
		if (costly_spot(u.ux, u.uy) && objroom == *u.ushops) {
		    Norep("You cancel it, you pay for it!");
		    bill_dummy_object(obj);
		} else
		    (void) stolen_value(obj, obj->ox, obj->oy, FALSE, FALSE);
		break;
	}
}

/* cancel obj, possibly carried by you or a monster */
void
cancel_item(obj)
register struct obj *obj;
{
	boolean	u_ring = (obj == uleft) || (obj == uright);
	register boolean unpaid = (carried(obj) && obj->unpaid);
	register boolean holy = (obj->otyp == POT_WATER && obj->blessed);

	switch(obj->otyp) {
		case RIN_GAIN_STRENGTH:
			if ((obj->owornmask & W_RING) && u_ring) {
				ABON(A_STR) -= obj->spe;
				flags.botl = 1;
			}
			break;
		case RIN_ADORNMENT:
			if ((obj->owornmask & W_RING) && u_ring) {
				ABON(A_CHA) -= obj->spe;
				flags.botl = 1;
			}
			break;
		case RIN_INCREASE_DAMAGE:
			if ((obj->owornmask & W_RING) && u_ring)
				u.udaminc -= obj->spe;
			break;
		case GAUNTLETS_OF_DEXTERITY:
			if ((obj->owornmask & W_ARMG) && (obj == uarmg)) {
				ABON(A_DEX) -= obj->spe;
				flags.botl = 1;
			}
			break;
		case HELM_OF_BRILLIANCE:
			if ((obj->owornmask & W_ARMH) && (obj == uarmh)) {
				ABON(A_INT) -= obj->spe;
				ABON(A_WIS) -= obj->spe;
				flags.botl = 1;
			}
			break;
		/* case RIN_PROTECTION: /* not needed */
	}
	if (objects[obj->otyp].oc_magic
	    || (obj->spe && (obj->oclass == ARMOR_CLASS ||
			     obj->oclass == WEAPON_CLASS || is_weptool(obj)))
	    || obj->otyp == POT_SICKNESS) {
	    if (obj->spe != ((obj->oclass == WAND_CLASS) ? -1 : 0) &&
	       obj->otyp != WAN_CANCELLATION &&
		 /* can't cancel cancellation */
		 obj->otyp != MAGIC_LAMP &&
		 obj->otyp != CANDELABRUM_OF_INVOCATION) {
		costly_cancel(obj);
		obj->spe = (obj->oclass == WAND_CLASS) ? -1 : 0;
		if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
	    }
	    switch (obj->oclass) {
	      case SCROLL_CLASS:
		costly_cancel(obj);
		obj->otyp = SCR_BLANK_PAPER;
		obj->spe = 0;
		if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
		break;
	      case SPBOOK_CLASS:
		if (obj->otyp != SPE_CANCELLATION &&
			obj->otyp != SPE_BOOK_OF_THE_DEAD) {
		    costly_cancel(obj);
		    obj->otyp = SPE_BLANK_PAPER;
		    if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
		}
		break;
	      case POTION_CLASS:
		costly_cancel(obj);
		if (obj->otyp == POT_SICKNESS ||
		    obj->otyp == POT_SEE_INVISIBLE) {
	    /* sickness is "biologically contaminated" fruit juice; cancel it
	     * and it just becomes fruit juice... whereas see invisible
	     * tastes like "enchanted" fruit juice, it similarly cancels.
	     */
		    obj->otyp = POT_FRUIT_JUICE;
		} else {
	            obj->otyp = POT_WATER;
		    obj->odiluted = 0; /* same as any other water */
		}
		if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
		break;
	    }
	}
	if (holy) costly_cancel(obj);
	unbless(obj);
	if (unpaid && holy) addtobill(obj, TRUE, FALSE, TRUE);
	uncurse(obj);
}
#endif /*OVLB*/
#ifdef OVL0

boolean
obj_resists(obj, ochance, achance)
struct obj *obj;
int ochance, achance;	/* percent chance for ordinary objects, artifacts */
{
	if (obj->otyp == AMULET_OF_YENDOR ||
	    obj->otyp == SPE_BOOK_OF_THE_DEAD ||
	    obj->otyp == CANDELABRUM_OF_INVOCATION ||
	    obj->otyp == BELL_OF_OPENING ||
	    (obj->otyp == CORPSE && is_rider(&mons[obj->corpsenm]))) {
		return TRUE;
	} else {
		int chance = rn2(100);

		return((boolean)(chance < (obj->oartifact ? achance : ochance)));
	}
}

boolean
obj_shudders(obj)
struct obj *obj;
{
	int	zap_odds;

	if (obj->oclass == WAND_CLASS)
		zap_odds = 3;	/* half-life = 2 zaps */
	else if (obj->cursed)
		zap_odds = 3;	/* half-life = 2 zaps */
	else if (obj->blessed)
		zap_odds = 12;	/* half-life = 8 zaps */
	else
		zap_odds = 8;	/* half-life = 6 zaps */

	/* adjust for "large" quantities of identical things */
	if(obj->quan > 4L) zap_odds /= 2;

	return((boolean)(! rn2(zap_odds)));
}
#endif /*OVL0*/
#ifdef OVLB

/* Use up at least minwt number of things made of material mat.
 * There's also a chance that other stuff will be used up.  Finally,
 * there's a random factor here to keep from always using the stuff
 * at the top of the pile.
 */
STATIC_OVL void
polyuse(objhdr, mat, minwt)
    struct obj *objhdr;
    int mat, minwt;
{
    register struct obj *otmp, *otmp2;

    for(otmp = objhdr; minwt > 0 && otmp; otmp = otmp2) {
	otmp2 = otmp->nexthere;
	if (((int) objects[otmp->otyp].oc_material == mat) ==
		(rn2(minwt + 1) != 0)) {
	    /* appropriately add damage to bill */
	    if (costly_spot(otmp->ox, otmp->oy))
		if (*u.ushops)
			addtobill(otmp, FALSE, FALSE, FALSE);
		else
			(void)stolen_value(otmp,
					   otmp->ox, otmp->oy, FALSE, FALSE);
	    minwt -= (int)otmp->quan;
	    delobj(otmp);
	}
    }
}

/*
 * Polymorph some of the stuff in this pile into a monster, preferably
 * a golem of some sort.
 */
STATIC_OVL void
create_polymon(obj)
    struct obj *obj;
{
	struct permonst *mdat = (struct permonst *)0;
	struct monst *mtmp;
	const char *material;
	int pm_index;

	/* no golems if you zap only one object -- not enough stuff */
	if(!obj || (!obj->nexthere && obj->quan == 1L)) return;

	/* some of these choices are arbitrary */
	switch(poly_zapped) {
	case IRON:
	case METAL:
	case MITHRIL:
	    pm_index = PM_IRON_GOLEM;
	    material = "metal ";
	    break;
	case COPPER:
	case SILVER:
	case GOLD:
	case PLATINUM:
	case GEMSTONE:
	case GLASS:
	case MINERAL:
	    pm_index = rn2(2) ? PM_STONE_GOLEM : PM_CLAY_GOLEM;
	    material = "lithic ";
	    break;
	case 0:
	    /* there is no flesh type, but all food is type 0, so we use it */
	    pm_index = PM_FLESH_GOLEM;
	    material = "organic ";
	    break;
	case WOOD:
	    pm_index = PM_WOOD_GOLEM;
	    material = "wood ";
	    break;
	case LEATHER:
	    pm_index = PM_LEATHER_GOLEM;
	    material = "leather ";
	    break;
	case CLOTH:
	    pm_index = PM_ROPE_GOLEM;
	    material = "cloth ";
	    break;
	default:
	    /* if all else fails... */
	    pm_index = PM_STRAW_GOLEM;
	    material = "";
	    break;
	}

	if (!(mvitals[pm_index].mvflags & G_GENOD))
		mdat = &mons[pm_index];

	mtmp = makemon(mdat, obj->ox, obj->oy, NO_MM_FLAGS);
	polyuse(obj, poly_zapped, (int)mons[pm_index].cwt);

	if(!Blind && mtmp) {
	    pline("Some %sobjects meld, and %s arises from the pile!",
		  material, a_monnam(mtmp));
	}
}

/* Assumes obj is on the floor. */
void
do_osshock(obj)
struct obj *obj;
{
	long i;
	obj_zapped = TRUE;

	if(poly_zapped < 0) {
	    /* some may metamorphosize */
	    for(i=obj->quan; i; i--)
		if (! rn2(Luck + 45)) {
		    poly_zapped = objects[obj->otyp].oc_material;
		    break;
		}
	}

	/* if quan > 1 then some will survive intact */
	if (obj->quan > 1L)
		(void) splitobj(obj, (long)rnd((int)obj->quan - 1));

	/* appropriately add damage to bill */
	if (costly_spot(obj->ox, obj->oy))
		if (*u.ushops)
			addtobill(obj, FALSE, FALSE, FALSE);
		else
			(void)stolen_value(obj,
					   obj->ox, obj->oy, FALSE, FALSE);

	/* zap the object */
	delobj(obj);
}

void
poly_obj(obj)
	register struct obj *obj;
{
	register struct obj *otmp;

	/* preserve symbol and quantity */
	otmp = mkobj_at(obj->oclass, obj->ox, obj->oy, FALSE);
	otmp->quan = obj->quan;
	/* preserve the shopkeepers (lack of) interest */
	otmp->no_charge = obj->no_charge;
#ifdef MAIL
	/* You can't send yourself 100 mail messages and then
	 * polymorph them into useful scrolls
	 */
	if (obj->otyp == SCR_MAIL) {
		otmp->otyp = SCR_MAIL;
		otmp->spe = 1;
	}
#endif

	/* avoid abusing eggs laid by you */
	if (obj->otyp == EGG && obj->spe) {
		int mnum, tryct = 100;

		/* first, turn into a generic egg */
		if (otmp->otyp == EGG)
		    kill_egg(otmp);
		else {
		    otmp->otyp = EGG;
		    otmp->owt = weight(otmp);
		}
		otmp->corpsenm = NON_PM;
		otmp->spe = 0;

		/* now change it into something layed by the hero */
		while (tryct--) {
		    mnum = can_be_hatched(random_monster());
		    if (mnum != NON_PM && !dead_species(mnum, TRUE)) {
			otmp->spe = 1;	/* layed by hero */
			otmp->corpsenm = mnum;
			attach_egg_hatch_timeout(otmp);
			break;
		    }
		}
	}

	/* keep special fields (including charges on wands) */
	if (index(charged_objs, otmp->oclass)) otmp->spe = obj->spe;

	otmp->cursed = obj->cursed;
	otmp->blessed = obj->blessed;
	otmp->oeroded = obj->oeroded;
	otmp->oerodeproof = obj->oerodeproof;

	/* reduce spellbook abuse */
	if (obj->oclass == SPBOOK_CLASS)
	    otmp->spestudied = obj->spestudied + 1;

	/* Keep chest/box traps and poisoned ammo if we may */
	if (obj->otrapped && Is_box(otmp)) otmp->otrapped = TRUE;

	if (obj->opoisoned &&
	    (otmp->oclass == WEAPON_CLASS && otmp->otyp <= SHURIKEN))
		otmp->opoisoned = TRUE;

	if (obj->otyp == CORPSE) {
	/* turn crocodile corpses into shoes */
	    if (obj->corpsenm == PM_CROCODILE) {
		otmp->otyp = LOW_BOOTS;
		otmp->oclass = ARMOR_CLASS;
		otmp->spe = 0;
		otmp->oeroded = 0;
		otmp->oerodeproof = TRUE;
		otmp->quan = 1L;
		otmp->cursed = FALSE;
	    }
	}

	/* no box contents --KAA */
	if (Has_contents(otmp)) delete_contents(otmp);

	/* 'n' merged objects may be fused into 1 object */
	if (otmp->quan > 1L && (!objects[otmp->otyp].oc_merge ||
				otmp->quan > (long)rn2(1000)))
		otmp->quan = 1L;

	if(otmp->otyp == MAGIC_LAMP) {
		otmp->otyp = OIL_LAMP;
		otmp->age = (long) rn1(500,1000);
	}

	while(otmp->otyp == WAN_WISHING || otmp->otyp == WAN_POLYMORPH)
		otmp->otyp = rnd_class(WAN_LIGHT, WAN_LIGHTNING);

	if (otmp->oclass == GEM_CLASS) {
	    if (otmp->quan > (long) rnd(4) &&
		    objects[obj->otyp].oc_material == MINERAL &&
		    objects[otmp->otyp].oc_material != MINERAL) {
		otmp->otyp = ROCK;	/* transmutation backfired */
		otmp->quan /= 2L;	/* some material has been lost */
	    }
	}

	/* update the weight */
	otmp->owt = weight(otmp);

	if(costly_spot(obj->ox, obj->oy)) {
	    register struct monst *shkp =
		  shop_keeper(*in_rooms(obj->ox, obj->oy, SHOPBASE));

	    if ((!obj->no_charge ||
		 (Has_contents(obj) &&
		    (contained_cost(obj, shkp, 0L, FALSE) != 0L)))
	       && inhishop(shkp)) {
		if(shkp->mpeaceful) {
		    if(*u.ushops && *in_rooms(u.ux, u.uy, 0) ==
			    *in_rooms(shkp->mx, shkp->my, 0) &&
			    !costly_spot(u.ux, u.uy))
			make_angry_shk(shkp, obj->ox, obj->oy);
		    else {
			pline("%s gets angry!", Monnam(shkp));
			hot_pursuit(shkp);
		    }
		} else Norep("%s is furious!", Monnam(shkp));
	    }
	}
	delobj(obj);
	return;
}

int
bhito(obj, otmp)	/* object obj was hit by the effect of wand otmp */
register struct obj *obj, *otmp;	/* returns TRUE if sth was done */
{
	register int res = 1;

	/* bhito expects obj->{ox,oy} to be valid */
	if (obj->where != OBJ_FLOOR)
	    impossible("bhito: obj is at %d, not floor", obj->where);

	if (obj == uball) {
		res = 0;
	} else if (obj == uchain) {
		if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK) {
		    unpunish();
		    res = 1;
		    makeknown(otmp->otyp);
		} else
		    res = 0;
	} else
	switch(otmp->otyp) {
	case WAN_POLYMORPH:
	case SPE_POLYMORPH:
		if (obj_resists(obj, 5, 95)) {
		    res = 0;
		    break;
		}
		/* any saved lock context will be dangerously obsolete */
		if (Is_box(obj)) (void) boxlock(obj, otmp);

		if (obj_shudders(obj)) {
		    if (cansee(obj->ox, obj->oy))
			makeknown(otmp->otyp);
		    do_osshock(obj);
		    break;
		}
		poly_obj(obj);
		break;
	case WAN_PROBING:
		res = !obj->dknown;
		/* target object has now been "seen (up close)" */
		obj->dknown = 1;
		if (Has_contents(obj)) {
		    if (!obj->cobj)
			pline("%s is empty.", The(xname(obj)));
		    else {
			struct obj *o;
			/* view contents (not recursively) */
			for (o = obj->cobj; o; o = o->nobj)
			    o->dknown = 1;	/* "seen", even if blind */
			(void) display_cinventory(obj);
		    }
		    res = 1;
		}
		if (res) makeknown(WAN_PROBING);
		break;
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
		if (obj->otyp == BOULDER)
			fracture_rock(obj);
		else if (obj->otyp == STATUE)
			(void) break_statue(obj);
		else {
			(void)breaks(obj, obj->ox, obj->oy);
			res = 0;
		}
		makeknown(otmp->otyp);
		break;
	case WAN_DIGGING:
	case SPE_DIG:
		/* vaporize boulders */
		if (obj->otyp == BOULDER) {
			delobj(obj);
			res = 0;
		}
		break;
	case WAN_CANCELLATION:
	case SPE_CANCELLATION:
		cancel_item(obj);
#ifdef TEXTCOLOR
		newsym(obj->ox,obj->oy);	/* might change color */
#endif
		break;
	case WAN_TELEPORTATION:
	case SPE_TELEPORT_AWAY:
		rloco(obj);
		break;
	case WAN_MAKE_INVISIBLE:
		obj->oinvis = TRUE;
		newsym(obj->ox,obj->oy);	/* make object disappear */
		break;
	case WAN_UNDEAD_TURNING:
	case SPE_TURN_UNDEAD:
		if (obj->otyp == EGG)
			revive_egg(obj);
		else
			res = !!revive(obj);
		break;
	case WAN_OPENING:
	case SPE_KNOCK:
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
		if(Is_box(obj))
			res = boxlock(obj, otmp);
		else
			res = 0;
		if (res /* && otmp->oclass == WAND_CLASS */)
			makeknown(otmp->otyp);
		break;
	case WAN_SLOW_MONSTER:		/* no effect on objects */
	case SPE_SLOW_MONSTER:
	case WAN_SPEED_MONSTER:
	case WAN_NOTHING:
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
		res = 0;
		break;
	default:
		impossible("What an interesting effect (%d)", otmp->otyp);
		break;
	}
	return(res);
}

/* returns nonzero if something was hit */
int
bhitpile(obj,fhito,tx,ty)
    struct obj *obj;
    int FDECL((*fhito), (OBJ_P,OBJ_P));
    int tx, ty;
{
    int hitanything = 0;
    register struct obj *otmp, *next_obj = (struct obj *)0;

    /* modified by GAN to hit all objects */
    /* pre-reverse the polymorph pile,  -dave- 3/90 */
    poly_zapped = -1;
    if(obj->otyp == SPE_POLYMORPH || obj->otyp == WAN_POLYMORPH) {
	otmp = level.objects[tx][ty];
	level.objects[tx][ty] = next_obj;
	while(otmp) {
	    next_obj = otmp->nexthere;
	    otmp->nexthere = level.objects[tx][ty];
	    level.objects[tx][ty] = otmp;
	    otmp = next_obj;
	}
    }
    for(otmp = level.objects[tx][ty]; otmp; otmp = next_obj) {
	/* Fix for polymorph bug, Tim Wright */
	next_obj = otmp->nexthere;
	hitanything += (*fhito)(otmp, obj);
    }
    if(poly_zapped >= 0)
	create_polymon(level.objects[tx][ty]);

    return hitanything;
}
#endif /*OVLB*/
#ifdef OVL1

/*
 * zappable - returns 1 if zap is available, 0 otherwise.
 *	      it removes a charge from the wand if zappable.
 * added by GAN 11/03/86
 */
int
zappable(wand)
register struct obj *wand;
{
	if(wand->spe < 0 || (wand->spe == 0 && rn2(121)))
		return 0;
	if(wand->spe == 0)
		You("wrest one last charge from the worn-out wand.");
	wand->spe--;
	return 1;
}

/*
 * zapnodir - zaps a NODIR wand/spell.
 * added by GAN 11/03/86
 */
void
zapnodir(obj)
register struct obj *obj;
{
	boolean known = FALSE;

	switch(obj->otyp) {
		case WAN_LIGHT:
		case SPE_LIGHT:
			litroom(TRUE,obj);
			if (!Blind) known = TRUE;
			break;
		case WAN_SECRET_DOOR_DETECTION:
		case SPE_DETECT_UNSEEN:
			if(!findit()) return;
			if (!Blind) known = TRUE;
			break;
		case WAN_CREATE_MONSTER:
			known = create_critters(rn2(23) ? 1 : rn1(7,2),
					(struct permonst *)0);
			break;
		case WAN_WISHING:
			known = TRUE;
			if(Luck + rn2(5) < 0) {
				pline("Unfortunately, nothing happens.");
				break;
			}
			makewish();
			break;
	}
	if (known && !objects[obj->otyp].oc_name_known) {
		makeknown(obj->otyp);
		more_experienced(0,10);
	}
}
#endif /*OVL1*/
#ifdef OVL0

static void
backfire(otmp)

	register struct obj * otmp;
{
	pline("%s suddenly explodes!", The(xname(otmp)));
	losehp(d(otmp->spe+2,6), "exploding wand", KILLED_BY_AN);
	useup(otmp);
}

static NEARDATA const char zap_syms[] = { WAND_CLASS, 0 };

int
dozap()
{
	register struct obj *obj;
	int	damage;

	if(check_capacity((char *)0)) return(0);
	obj = getobj(zap_syms, "zap");
	if(!obj) return(0);

	check_unpaid(obj);

	/* zappable addition done by GAN 11/03/86 */
	if(!zappable(obj)) pline(nothing_happens);
	else if(obj->cursed && !rn2(100)) {
		backfire(obj);	/* the wand blows up in your face! */
		exercise(A_STR, FALSE);
		return(1);
	} else if(!(objects[obj->otyp].oc_dir == NODIR) && !getdir((char *)0)) {
		if (!Blind)
		    pline("%s glows and fades.", The(xname(obj)));
		/* make him pay for knowing !NODIR */
	} else if(!u.dx && !u.dy && !u.dz && !(objects[obj->otyp].oc_dir == NODIR)) {
	    if ((damage = zapyourself(obj, TRUE)) != 0)
		losehp(damage, self_pronoun("zapped %sself with a wand", "him"),
			NO_KILLER_PREFIX);
	} else {

		/*	Are we having fun yet?
		 * weffects -> buzz(obj->otyp) -> zhitm (temple priest) ->
		 * attack -> hitum -> known_hitum -> ghod_hitsu ->
		 * buzz(AD_ELEC) -> destroy_item(WAND_CLASS) ->
		 * useup -> obfree -> dealloc_obj -> free(obj)
		 */
		current_wand = obj;
		weffects(obj);
		obj = current_wand;
		current_wand = 0;
	}
	if (obj && obj->spe < 0) {
	    pline("%s turns to dust.", The(xname(obj)));
	    useup(obj);
	}
	update_inventory();	/* maybe used a charge */
	return(1);
}

int
zapyourself(obj, ordinary)
struct obj *obj;
boolean ordinary;
{
	int	damage = 0;

	switch(obj->otyp) {
		case WAN_STRIKING:
		    makeknown(WAN_STRIKING);
		case SPE_FORCE_BOLT:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline("Boing!");
		    } else {
			if (ordinary) {
			    You("bash yourself!");
			    damage = d(2,12);
			} else
			    damage = d(1 + obj->spe,6);
			exercise(A_STR, FALSE);
		    }
		    break;
		case WAN_LIGHTNING:
		    makeknown(WAN_LIGHTNING);
		    if (!Shock_resistance) {
			You("shock yourself!");
			damage = d(12,6);
			exercise(A_CON, FALSE);
		    } else {
			shieldeff(u.ux, u.uy);
			You("zap yourself, but seem unharmed.");
			ugolemeffects(AD_ELEC, d(12,6));
		    }
		    destroy_item(WAND_CLASS, AD_ELEC);
		    destroy_item(RING_CLASS, AD_ELEC);
		    if (!resists_blnd(&youmonst)) {
			    You(are_blinded_by_the_flash);
			    make_blinded((long)rnd(100),FALSE);
		    }
		    break;
		case SPE_FIREBALL:
		    You("explode a fireball on top of yourself!");
		    explode(u.ux, u.uy, 11, d(6,6), WAND_CLASS);
		    break;
		case WAN_FIRE:
		    makeknown(WAN_FIRE);
		case FIRE_HORN:
		    if (Fire_resistance) {
			shieldeff(u.ux, u.uy);
			You_feel("rather warm.");
			ugolemeffects(AD_FIRE, d(12,6));
		    } else {
			pline("You've set yourself afire!");
			damage = d(12,6);
		    }
		    (void) burnarmor();
		    destroy_item(SCROLL_CLASS, AD_FIRE);
		    destroy_item(POTION_CLASS, AD_FIRE);
		    destroy_item(SPBOOK_CLASS, AD_FIRE);
		    break;
		case WAN_COLD:
		    makeknown(WAN_COLD);
		case SPE_CONE_OF_COLD:
		case FROST_HORN:
		    if (Cold_resistance) {
			shieldeff(u.ux, u.uy);
			You_feel("a little chill.");
			ugolemeffects(AD_COLD, d(12,6));
		    } else {
			You("imitate a popsicle!");
			damage = d(12,6);
		    }
		    destroy_item(POTION_CLASS, AD_COLD);
		    break;
		case WAN_MAGIC_MISSILE:
		    makeknown(WAN_MAGIC_MISSILE);
		case SPE_MAGIC_MISSILE:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline_The("missiles bounce!");
		    } else {
			damage = d(4,6);
			pline("Idiot!  You've shot yourself!");
		    }
		    break;
		case WAN_POLYMORPH:
		    makeknown(WAN_POLYMORPH);
		case SPE_POLYMORPH:
		    polyself();
		    break;
		case WAN_CANCELLATION:
		case SPE_CANCELLATION:
		    cancel_monst(&youmonst, obj, TRUE, FALSE, TRUE);
		    break;
		case WAN_MAKE_INVISIBLE: {
		    /* have to test before changing HInvis but must change
		     * HInvis before doing newsym().
		     */
		    int msg = (!Invis && !See_invisible);

		    if (ordinary || !rn2(10)) {	/* permanent */
			HInvis |= FROMOUTSIDE;
		    } else {			/* temporary */
			long amt = d(obj->spe, 250) + (HInvis & TIMEOUT);

			HInvis &= ~TIMEOUT;
			HInvis |= min(amt, TIMEOUT);
		    }
		    if (msg && (Blind || !Invis)) {
			You_feel("rather airy.");
		    } else if (msg) {
			makeknown(WAN_MAKE_INVISIBLE);
			newsym(u.ux, u.uy);
			pline(Hallucination ?
			 "Far out, man!  You can see right through yourself!" :
			 "Gee!  All of a sudden, you can't see yourself.");
		    }
		    break;
		}
		case WAN_SPEED_MONSTER:
		    if (!(Fast & INTRINSIC)) {
			if (!Fast)
			    You("speed up.");
			else
			    Your("quickness feels more natural.");
			makeknown(WAN_SPEED_MONSTER);
			exercise(A_DEX, TRUE);
		    }
		    Fast |= FROMOUTSIDE;
		    break;
		case WAN_SLEEP:
		    makeknown(WAN_SLEEP);
		case SPE_SLEEP:
		    if(Sleep_resistance) {
			shieldeff(u.ux, u.uy);
			You("don't feel sleepy!");
		    } else {
			pline_The("sleep ray hits you!");
			fall_asleep(-rnd(50), TRUE);
		    }
		    break;
		case WAN_SLOW_MONSTER:
		case SPE_SLOW_MONSTER:
		    if(Fast & (TIMEOUT | INTRINSIC)) {
			u_slow_down();
			makeknown(obj->otyp);
		    }
		    break;
		case WAN_TELEPORTATION:
		case SPE_TELEPORT_AWAY:
		    tele();
		    break;
		case WAN_DEATH:
		case SPE_FINGER_OF_DEATH:
		    if (nonliving(uasmon) || is_demon(uasmon)) {
			pline((obj->otyp == WAN_DEATH) ?
			  "The wand shoots an apparently harmless beam at you."
			  : "You seem no deader than before.");
			break;
		    }
		    killer_format = NO_KILLER_PREFIX;
		    killer = self_pronoun("shot %sself with a death ray","him");
		    You("irradiate yourself with pure energy!");
		    You("die.");
		    makeknown(obj->otyp);
			/* They might survive with an amulet of life saving */
		    done(DIED);
		    break;
		case WAN_UNDEAD_TURNING:
		    makeknown(WAN_UNDEAD_TURNING);
		case SPE_TURN_UNDEAD:
		    (void) unturn_dead(&youmonst);
		    if (is_undead(uasmon)) {
			You_feel("frightened and %sstunned.",
			     Stunned ? "even more " : "");
			make_stunned(HStun + rnd(30), FALSE);
		    } else
			You("shudder in dread.");
		    break;
		case SPE_HEALING:
		case SPE_EXTRA_HEALING:
		    healup(d(6, obj->otyp == SPE_EXTRA_HEALING ? 8 : 4),
			   0, FALSE, (obj->otyp == SPE_EXTRA_HEALING));
		    You("feel%s better.",
			obj->otyp == SPE_EXTRA_HEALING ? " much" : "");
		    break;
		case WAN_LIGHT:	/* (broken wand) */
		 /* assert( !ordinary ); */
		    damage = d(obj->spe, 25);
#ifdef TOURIST
		case EXPENSIVE_CAMERA:
#endif
		    damage += rnd(25);
		    if (!resists_blnd(&youmonst)) {
			You(are_blinded_by_the_flash);
			make_blinded((long)damage, FALSE);
			makeknown(obj->otyp);
		    }
		    damage = 0;	/* reset */
		    break;
		case WAN_OPENING:
		    if (Punished) makeknown(WAN_OPENING);
		case SPE_KNOCK:
		    if (Punished) Your("chain quivers for a moment.");
		    break;
		case WAN_DIGGING:
		case SPE_DIG:
		case SPE_DETECT_UNSEEN:
		case WAN_NOTHING:
		case WAN_LOCKING:
		case SPE_WIZARD_LOCK:
		    break;
		case WAN_PROBING:
		    for (obj = invent; obj; obj = obj->nobj)
			obj->dknown = 1;
		    /* note: `obj' reused; doesn't point at wand anymore */
		    makeknown(WAN_PROBING);
		    ustatusline();
		    break;
		default: impossible("object %d used?",obj->otyp);
		    break;
	}
	return(damage);
}

#endif /*OVL0*/
#ifdef OVL3

/*
 * cancel a monster (possibly the hero).  inventory is cancelled only
 * if the monster is zapping itself directly, since otherwise the
 * effect is too strong.  currently non-hero monsters do not zap
 * themselves with cancellation.
 */
void
cancel_monst(mdef, obj, youattack, allow_cancel_kill, self_cancel)
register struct monst	*mdef;
register struct obj	*obj;
boolean			youattack, allow_cancel_kill, self_cancel;
{
	boolean	youdefend = (mdef == &youmonst);
	static const char writing_vanishes[] =
				"Some writing vanishes from %s head!";
	static const char your[] = "your";	/* should be extern */

	if (youdefend ? (!youattack && Antimagic)
		      : resist(mdef, obj->oclass, 0, NOTELL))
		return;		/* resisted cancellation */

	if (self_cancel) {	/* 1st cancel inventory */
	    struct obj *otmp;

	    for (otmp = (youdefend ? invent : mdef->minvent);
			    otmp; otmp = otmp->nobj)
		cancel_item(otmp);
	    if (youdefend) {
		flags.botl = 1;	/* potential AC change */
		find_ac();
	    }
	}

	/* now handle special cases */
	if (youdefend) {
	    if (u.mtimedone) {
		if ((u.umonnum == PM_CLAY_GOLEM) && !Blind)
		    pline(writing_vanishes, your);
		rehumanize();
	    }
	} else {
	    mdef->mcan = TRUE;

	    if (is_were(mdef->data) && mdef->data->mlet != S_HUMAN)
		were_change(mdef);

	    if (mdef->data == &mons[PM_CLAY_GOLEM]) {
		if (canseemon(mdef))
		    pline(writing_vanishes, s_suffix(mon_nam(mdef)));

		if (allow_cancel_kill) {
		    if (youattack)
			killed(mdef);
		    else
			monkilled(mdef, "", AD_SPEL);
		}
	    }
	}
}

/* you've zapped an immediate type wand up or down */
STATIC_OVL boolean
zap_updown(obj)
struct obj *obj;	/* wand or spell */
{
	boolean striking = FALSE, disclose = FALSE;
	int x, y, xx, yy, ptmp;
	struct obj *otmp;
	struct engr *e;
	char buf[BUFSZ];

	/* some wands have special effects other than normal bhitpile */
	/* drawbridge might change <u.ux,u.uy> */
	x = xx = u.ux;	/* <x,y> is zap location */
	y = yy = u.uy;	/* <xx,yy> is drawbridge (portcullis) position */
	switch (obj->otyp) {
	case WAN_PROBING:
	    if (u.dz < 0) {
		You("probe towards the %s.", ceiling(x,y));
		ptmp = 0;
	    } else {
		You("probe beneath the %s.", surface(x,y));
		ptmp = display_binventory(x, y, TRUE);
		ptmp += bhitpile(obj, bhito, x, y);
	    }
	    if (!ptmp) Your("probe reveals nothing.");
	    return TRUE;	/* we've done our own bhitpile */
	case WAN_OPENING:
	case SPE_KNOCK:
	    /* up or down, but at closed portcullis only */
	    if (is_db_wall(x,y) && find_drawbridge(&xx, &yy)) {
		open_drawbridge(xx, yy);
		disclose = TRUE;
	    } else if (u.dz > 0 && (x == xdnstair && y == ydnstair) &&
			/* can't use the stairs down to quest level 2 until
			   leader "unlocks" them; give feedback if you try */
			on_level(&u.uz, &qstart_level) && !ok_to_quest()) {
		pline("The stairs seem to ripple momentarily.");
		disclose = TRUE;
	    }
	    break;
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
	    striking = TRUE;
	    /*FALLTHRU*/
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
	    /* down at open bridge or up or down at open portcullis */
	    if ((levl[x][y].typ == DRAWBRIDGE_DOWN) ? (u.dz > 0) :
			(is_drawbridge_wall(x,y) && !is_db_wall(x,y)) &&
		    find_drawbridge(&xx, &yy)) {
		if (!striking)
		    close_drawbridge(xx, yy);
		else
		    destroy_drawbridge(xx, yy);
		disclose = TRUE;
	    } else if (striking && u.dz < 0 && rn2(3) &&
			!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz)) {
		/* similar to zap_dig() */
		pline("A rock is dislodged from the %s and falls on your %s.",
		      ceiling(x, y), body_part(HEAD));
		losehp(rnd((uarmh && is_metallic(uarmh)) ? 2 : 6),
		       "falling rock", KILLED_BY_AN);
		if ((otmp = mksobj_at(ROCK, x, y, FALSE)) != 0) {
		    (void)xname(otmp);	/* set dknown, maybe bknown */
		    stackobj(otmp);
		}
		newsym(x, y);
	    }
	    break;
	default:
	    break;
	}

	if (u.dz > 0) {
	    /* zapping downward */
	    (void) bhitpile(obj, bhito, x, y);

	    /* subset of engraving effects; none sets `disclose' */
	    if ((e = engr_at(x, y)) != 0) {
		switch (obj->otyp) {
		case WAN_POLYMORPH:
		case SPE_POLYMORPH:
		    del_engr(e);
		    make_engr_at(x, y, random_engraving(buf), moves, (xchar)0);
		    break;
		case WAN_CANCELLATION:
		case SPE_CANCELLATION:
		case WAN_MAKE_INVISIBLE:
		    del_engr(e);
		    break;
		case WAN_TELEPORTATION:
		case SPE_TELEPORT_AWAY:
		    rloc_engr(e);
		    break;
		case WAN_STRIKING:
		case SPE_FORCE_BOLT:
		    wipe_engr_at(x, y, d(2,4));
		    break;
		default:
		    break;
		}
	    }
	}

	return disclose;
}

#endif /*OVL3*/
#ifdef OVLB

/* called for various wand and spell effects - M. Stephenson */
void
weffects(obj)
register struct	obj	*obj;
{
	int otyp = obj->otyp;
	boolean disclose = FALSE, was_unkn = !objects[otyp].oc_name_known;

	exercise(A_WIS, TRUE);
	if (objects[otyp].oc_dir == IMMEDIATE) {
	    obj_zapped = FALSE;
	    if (u.uswallow) {
		(void) bhitm(u.ustuck, obj);
		/* [how about `bhitpile(u.ustuck->minvent)' effect?] */
	    } else if (u.dz) {
		disclose = zap_updown(obj);
	    } else {
		(void) bhit(u.dx,u.dy, rn1(8,6),ZAPPED_WAND, bhitm,bhito, obj);
	    }
	    /* give a clue if obj_zapped */
	    if (obj_zapped)
		You_feel("shuddering vibrations.");

	} else if (objects[otyp].oc_dir == NODIR) {
	    zapnodir(obj);

	} else {
	    /* neither immediate nor directionless */

	    if (otyp == WAN_DIGGING || otyp == SPE_DIG)
		zap_dig();
	    else if (otyp >= SPE_MAGIC_MISSILE && otyp <= SPE_FINGER_OF_DEATH)
		buzz(otyp - SPE_MAGIC_MISSILE + 10,
		     u.ulevel / 2 + 1,
		     u.ux, u.uy, u.dx, u.dy);
	    else if (otyp >= WAN_MAGIC_MISSILE && otyp <= WAN_LIGHTNING)
		buzz(otyp - WAN_MAGIC_MISSILE,
		     (otyp == WAN_MAGIC_MISSILE) ? 2 : 6,
		     u.ux, u.uy, u.dx, u.dy);
	    else
		impossible("weffects: unexpected spell or wand");
	    disclose = TRUE;
	}

	if (disclose && was_unkn) {
	    makeknown(otyp);
	    more_experienced(0,10);
	}
	return;
}
#endif /*OVLB*/
#ifdef OVL0

const char *
exclam(force)
register int force;
{
	/* force == 0 occurs e.g. with sleep ray */
	/* note that large force is usual with wands so that !! would
		require information about hand/weapon/wand */
	return (const char *)((force < 0) ? "?" : (force <= 4) ? "." : "!");
}

void
hit(str,mtmp,force)
register const char *str;
register struct monst *mtmp;
register const char *force;		/* usually either "." or "!" */
{
	if(!cansee(bhitpos.x,bhitpos.y) || !flags.verbose)
	    pline("%s hits it.", The(str));
	else pline("%s hits %s%s", The(str), mon_nam(mtmp), force);
}

void
miss(str,mtmp)
register const char *str;
register struct monst *mtmp;
{
	pline("%s misses %s.", The(str),
	      (cansee(bhitpos.x,bhitpos.y) && flags.verbose) ?
	      mon_nam(mtmp) : "it");
}
#endif /*OVL0*/
#ifdef OVL1

/*
 *  Called for the following distance effects:
 *	when a weapon is thrown (weapon == THROWN_WEAPON)
 *	when an object is kicked (KICKED_WEAPON)
 *	when an IMMEDIATE wand is zapped (ZAPPED_WAND)
 *	when a light beam is flashed (FLASHED_LIGHT)
 *	for some invisible effect on a monster (INVIS_BEAM)
 *  A thrown/kicked object falls down at the end of its range or when a monster
 *  is hit.  The variable 'bhitpos' is set to the final position of the weapon
 *  thrown/zapped.  The ray of a wand may affect (by calling a provided
 *  function) several objects and monsters on its path.  The return value
 *  is the monster hit (weapon != ZAPPED_WAND), or a null monster pointer.
 *
 *  Check !u.uswallow before calling bhit().
 */
struct monst *
bhit(ddx,ddy,range,weapon,fhitm,fhito,obj)
register int ddx,ddy,range;		/* direction and range */
int weapon;				/* see values in hack.h */
int FDECL((*fhitm), (MONST_P, OBJ_P)),	/* fns called when mon/obj hit */
    FDECL((*fhito), (OBJ_P, OBJ_P));
struct obj *obj;			/* object tossed/used */
{
	register struct monst *mtmp;
	register uchar typ;
	register boolean shopdoor = FALSE;

	if (weapon == KICKED_WEAPON) {
	    /* object starts one square in front of player */
	    bhitpos.x = u.ux + ddx;
	    bhitpos.y = u.uy + ddy;
	    range--;
	} else {
	    bhitpos.x = u.ux;
	    bhitpos.y = u.uy;
	}

	if (weapon == FLASHED_LIGHT) {
	    tmp_at(DISP_BEAM, cmap_to_glyph(S_flashbeam));
	} else if (weapon != ZAPPED_WAND && weapon != INVIS_BEAM)
	    tmp_at(DISP_FLASH, obj_to_glyph(obj));
	while(range-- > 0) {
		int x,y;

		bhitpos.x += ddx;
		bhitpos.y += ddy;
		x = bhitpos.x; y = bhitpos.y;

		if(!isok(x, y)) {
		    bhitpos.x -= ddx;
		    bhitpos.y -= ddy;
		    break;
		}
		if(obj->otyp == PICK_AXE && inside_shop(x, y) &&
					       shkcatch(obj, x, y)) {
		    tmp_at(DISP_END, 0);
		    return(m_at(x, y));
		}

		typ = levl[bhitpos.x][bhitpos.y].typ;

		if (weapon == ZAPPED_WAND && find_drawbridge(&x,&y))
		    switch (obj->otyp) {
			case WAN_OPENING:
			case SPE_KNOCK:
			    if (is_db_wall(bhitpos.x, bhitpos.y)) {
				if (cansee(x,y) || cansee(bhitpos.x,bhitpos.y))
				    makeknown(obj->otyp);
				open_drawbridge(x,y);
			    }
			    break;
			case WAN_LOCKING:
			case SPE_WIZARD_LOCK:
			    if ((cansee(x,y) || cansee(bhitpos.x, bhitpos.y))
				&& levl[x][y].typ == DRAWBRIDGE_DOWN)
				makeknown(obj->otyp);
			    close_drawbridge(x,y);
			    break;
			case WAN_STRIKING:
			case SPE_FORCE_BOLT:
			    if (typ != DRAWBRIDGE_UP)
				destroy_drawbridge(x,y);
			    makeknown(obj->otyp);
			    break;
		    }

		if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
			notonhead = (bhitpos.x != mtmp->mx ||
				     bhitpos.y != mtmp->my);
			if(weapon != ZAPPED_WAND) {
				if(weapon != INVIS_BEAM) tmp_at(DISP_END, 0);
				return(mtmp);
			}
			(*fhitm)(mtmp, obj);
			range -= 3;
		}
		if(fhito) {
		    if(bhitpile(obj,fhito,bhitpos.x,bhitpos.y))
			range--;
		} else {
    boolean costly = shop_keeper(*in_rooms(bhitpos.x, bhitpos.y, SHOPBASE)) &&
				    costly_spot(bhitpos.x, bhitpos.y);

			if(weapon == KICKED_WEAPON &&
			      ((obj->oclass == GOLD_CLASS &&
			         OBJ_AT(bhitpos.x, bhitpos.y)) ||
			    ship_object(obj, bhitpos.x, bhitpos.y, costly))) {
				tmp_at(DISP_END, 0);
				return (struct monst *)0;
			}
		}
		if(weapon == ZAPPED_WAND && (IS_DOOR(typ) || typ == SDOOR)) {
		    switch (obj->otyp) {
		    case WAN_OPENING:
		    case WAN_LOCKING:
		    case WAN_STRIKING:
		    case SPE_KNOCK:
		    case SPE_WIZARD_LOCK:
		    case SPE_FORCE_BOLT:
			if (doorlock(obj, bhitpos.x, bhitpos.y)) {
			    if (cansee(bhitpos.x, bhitpos.y) ||
				(obj->otyp == WAN_STRIKING))
				makeknown(obj->otyp);
			    if (levl[bhitpos.x][bhitpos.y].doormask == D_BROKEN
				&& *in_rooms(bhitpos.x, bhitpos.y, SHOPBASE)) {
				shopdoor = TRUE;
				add_damage(bhitpos.x, bhitpos.y, 400L);
			    }
			}
			break;
		    }
		}
		if(!ZAP_POS(typ) || closed_door(bhitpos.x, bhitpos.y)) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		}
		if(weapon != ZAPPED_WAND && weapon != INVIS_BEAM) {
			tmp_at(bhitpos.x, bhitpos.y);
			delay_output();
			/* kicked objects fall in pools */
			if((weapon == KICKED_WEAPON) &&
			   is_pool(bhitpos.x, bhitpos.y))
			    break;
#ifdef SINKS
			if(IS_SINK(typ) && weapon != FLASHED_LIGHT)
			    break;	/* physical objects fall onto sink */
#endif
		}
	}

	if (weapon != ZAPPED_WAND && weapon != INVIS_BEAM) tmp_at(DISP_END, 0);

	if(shopdoor)
	    pay_for_damage("destroy");

	return (struct monst *)0;
}

struct monst *
boomhit(dx, dy)
int dx, dy;
{
	register int i, ct;
	int boom = S_boomleft;	/* showsym[] index  */
	struct monst *mtmp;

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	for(i=0; i<8; i++) if(xdir[i] == dx && ydir[i] == dy) break;
	tmp_at(DISP_FLASH, cmap_to_glyph(boom));
	for(ct=0; ct<10; ct++) {
		if(i == 8) i = 0;
		boom = (boom == S_boomleft) ? S_boomright : S_boomleft;
		tmp_at(DISP_CHANGE, cmap_to_glyph(boom));/* change glyph */
		dx = xdir[i];
		dy = ydir[i];
		bhitpos.x += dx;
		bhitpos.y += dy;
		if(MON_AT(bhitpos.x, bhitpos.y)) {
			mtmp = m_at(bhitpos.x,bhitpos.y);
			m_respond(mtmp);
			tmp_at(DISP_END, 0);
			return(mtmp);
		}
		if(!ZAP_POS(levl[bhitpos.x][bhitpos.y].typ) ||
		   closed_door(bhitpos.x, bhitpos.y)) {
			bhitpos.x -= dx;
			bhitpos.y -= dy;
			break;
		}
		if(bhitpos.x == u.ux && bhitpos.y == u.uy) { /* ct == 9 */
			if(Fumbling || rn2(20) >= ACURR(A_DEX)) {
				/* we hit ourselves */
				(void) thitu(10, rnd(10), (struct obj *)0,
					"boomerang");
				break;
			} else {	/* we catch it */
				tmp_at(DISP_END, 0);
				You("skillfully catch the boomerang.");
				return(&youmonst);
			}
		}
		tmp_at(bhitpos.x, bhitpos.y);
		delay_output();
		if(ct % 5 != 0) i++;
#ifdef SINKS
		if(IS_SINK(levl[bhitpos.x][bhitpos.y].typ))
			break;	/* boomerang falls on sink */
#endif
	}
	tmp_at(DISP_END, 0);	/* do not leave last symbol */
	return (struct monst *)0;
}

STATIC_OVL int
zhitm(mon, type, nd)			/* returns damage to mon */
register struct monst *mon;
register int type, nd;
{
	register int tmp = 0;
	register int abstype = abs(type) % 10;
	boolean sho_shieldeff = FALSE;

	switch(abstype) {
	case ZT_MAGIC_MISSILE:
		if (resists_magm(mon)) {
		    sho_shieldeff = TRUE;
		    break;
		}
		tmp = d(nd,6);
		break;
	case ZT_FIRE:
		if (resists_fire(mon)) {
		    sho_shieldeff = TRUE;
		    break;
		}
		tmp = d(nd,6);
		if (resists_cold(mon)) tmp += 7;
		break;
	case ZT_COLD:
		if (resists_cold(mon)) {
		    sho_shieldeff = TRUE;
		    break;
		}
		tmp = d(nd,6);
		if (resists_fire(mon)) tmp += d(nd, 3);
		break;
	case ZT_SLEEP:
		tmp = 0;
		(void)sleep_monst(mon, d(nd, 25),
				type == ZT_WAND(ZT_SLEEP) ? WAND_CLASS : '\0');
		break;
	case ZT_DEATH:		/* death/disintegration */
		if(abs(type) != ZT_BREATH(ZT_DEATH)) {	/* death */
		    if(mon->data == &mons[PM_DEATH]) {
			mon->mhpmax += mon->mhpmax/2;
			mon->mhp = mon->mhpmax;
			tmp = 0;
			break;
		    }
		    if (nonliving(mon->data) || is_demon(mon->data) ||
			    resists_magm(mon)) {	/* similar to player */
			sho_shieldeff = TRUE;
			break;
		    }
		    type = -1; /* so they don't get saving throws */
		} else {
		    if (resists_disint(mon)) {
			sho_shieldeff = TRUE;
			break;
		    } else {
			tmp = MAGIC_COOKIE;
			break;
		    }
		}
		tmp = mon->mhp+1;
		break;
	case ZT_LIGHTNING:
		if (resists_elec(mon)) {
		    sho_shieldeff = TRUE;
		    tmp = 0;
		    /* can still blind the monster */
		} else
		    tmp = d(nd,6);
		if (!resists_blnd(mon) &&
				!(type > 0 && u.uswallow && mon == u.ustuck)) {
			register unsigned rnd_tmp = rnd(50);
			mon->mcansee = 0;
			if((mon->mblinded + rnd_tmp) > 127)
				mon->mblinded = 127;
			else mon->mblinded += rnd_tmp;
		}
		break;
	case ZT_POISON_GAS:
		if (resists_poison(mon)) {
		    sho_shieldeff = TRUE;
		    break;
		}
		tmp = d(nd,6);
		break;
	case ZT_ACID:
		if (resists_acid(mon)) {
		    sho_shieldeff = TRUE;
		    break;
		}
		tmp = d(nd,6);
		break;
	}
	if (sho_shieldeff) shieldeff(mon->mx, mon->my);
	if ((type >= 10 && type <= 19) && (Role_is('K') && u.uhave.questart))
	    tmp *= 2;
	if (type >= 0)
	    if (resist(mon, (type < ZT_SPELL(0)) ? WAND_CLASS : '\0',
		       0, NOTELL)) tmp /= 2;
	mon->mhp -= tmp;
	return(tmp);
}

STATIC_OVL void
zhitu(type, nd, fltxt, sx, sy)
int type, nd;
const char *fltxt;
xchar sx, sy;
{
	int dam = 0;

	switch (abs(type) % 10) {
	case ZT_MAGIC_MISSILE:
	    if (Antimagic) {
		shieldeff(sx, sy);
		pline_The("missiles bounce off!");
	    } else {
		dam = d(nd,6);
		exercise(A_STR, FALSE);
	    }
	    break;
	case ZT_FIRE:
	    if (Fire_resistance) {
		shieldeff(sx, sy);
		You("don't feel hot!");
		ugolemeffects(AD_FIRE, d(nd, 6));
	    } else {
		dam = d(nd, 6);
	    }
	    if (burnarmor()) {	/* "body hit" */
		if (!rn2(3)) destroy_item(POTION_CLASS, AD_FIRE);
		if (!rn2(3)) destroy_item(SCROLL_CLASS, AD_FIRE);
		if (!rn2(5)) destroy_item(SPBOOK_CLASS, AD_FIRE);
	    }
	    break;
	case ZT_COLD:
	    if (Cold_resistance) {
		shieldeff(sx, sy);
		You("don't feel cold.");
		ugolemeffects(AD_COLD, d(nd, 6));
	    } else {
		dam = d(nd, 6);
	    }
	    if (!rn2(3)) destroy_item(POTION_CLASS, AD_COLD);
	    break;
	case ZT_SLEEP:
	    if (Sleep_resistance) {
		shieldeff(u.ux, u.uy);
		You("don't feel sleepy.");
	    } else {
		fall_asleep(-d(nd,25), TRUE); /* sleep ray */
	    }
	    break;
	case ZT_DEATH:
	    if (abs(type) == ZT_BREATH(ZT_DEATH)) {
		if (Disint_resistance) {
		    You("are not disintegrated.");
		    break;
		} else if (uarms) {
		    /* destroy shield; other possessions are safe */
		    (void) destroy_arm(uarms);
		    break;
		} else if (uarm) {
		    /* destroy suit; if present, cloak goes too */
		    if (uarmc) (void) destroy_arm(uarmc);
		    (void) destroy_arm(uarm);
		    break;
		}
		/* no shield or suit, you're dead; wipe out cloak
		   and/or shirt in case of life-saving or bones */
		if (uarmc) (void) destroy_arm(uarmc);
#ifdef TOURIST
		if (uarmu) (void) destroy_arm(uarmu);
#endif
	    } else if (nonliving(uasmon) || is_demon(uasmon)) {
		shieldeff(sx, sy);
		You("seem unaffected.");
		break;
	    } else if (Antimagic) {
		shieldeff(sx, sy);
		You("aren't affected.");
		break;
	    }
	    u.uhp = -1;
	    break;
	case ZT_LIGHTNING:
	    if (Shock_resistance) {
		shieldeff(sx, sy);
		You("aren't affected.");
		ugolemeffects(AD_ELEC, d(nd, 6));
	    } else {
		dam = d(nd, 6);
		exercise(A_CON, FALSE);
	    }
	    if (!rn2(3)) destroy_item(WAND_CLASS, AD_ELEC);
	    if (!rn2(3)) destroy_item(RING_CLASS, AD_ELEC);
	    break;
	case ZT_POISON_GAS:
	    poisoned("blast", A_DEX, "poisoned blast", 15);
	    break;
	case ZT_ACID:
	    if (resists_acid(&youmonst)) {
		dam = 0;
	    } else {
		pline_The("acid burns!");
		dam = d(nd,6);
		exercise(A_STR, FALSE);
	    }
	    if (!rn2(6)) erode_weapon(TRUE);
	    if (!rn2(6)) erode_armor(TRUE);
	    break;
	}

	if (Half_spell_damage && dam &&
	   type < 0 && (type > -20 || type < -29)) /* !Breath */
	    dam = (dam + 1) / 2;
	/* when killed by disintegration breath, don't leave corpse */
	u.ugrave_arise = (type == -ZT_BREATH(ZT_DEATH)) ? -3 : -1;
	losehp(dam, fltxt, KILLED_BY_AN);
	return;
}

#endif /*OVL1*/
#ifdef OVLB

/*
 * burn scrolls and spell books on floor at position x,y
 * return the number of scrolls and spell books burned
 */
int
burn_floor_paper(x, y, give_feedback)
int x, y;
boolean give_feedback;	/* caller needs to decide about visibility checks */
{
	struct obj *obj, *obj2;
	long i, scrquan, delquan;
	const char *what;
	int cnt = 0;

	for (obj = level.objects[x][y]; obj; obj = obj2) {
	    obj2 = obj->nexthere;
	    if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS) {
		if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL ||
			obj_resists(obj, 2, 100))
		    continue;
		scrquan = obj->quan;	/* number present */
		delquan = 0;		/* number to destroy */
		for (i = scrquan; i > 0; i--)
		    if (!rn2(3)) delquan++;
		if (delquan) {
		    /* save name before potential delobj() */
		    what = !give_feedback ? 0 : (x == u.ux && y == u.uy) ?
				xname(obj) : distant_name(obj, xname);
		    /* not useupf(), which charges */
		    if (delquan < scrquan) obj->quan -= delquan;
		    else delobj(obj);
		    cnt += delquan;
		    if (give_feedback) {
			if (delquan > 1)
			    pline("%ld %s burn.", delquan, what);
			else
			    pline("%s burns.", An(what));
		    }
		}
	    }
	}
	return cnt;
}

/* will zap/spell/breath attack score a hit against armor class `ac'? */
static int
zap_hit(ac)
int ac;
{
    int chance = rn2(20);

    /* small chance for naked target to avoid being hit */
    if (!chance) return rnd(10) < ac;

    /* very high armor protection does not achieve invulnerability */
    ac = AC_VALUE(ac);

    return (3 - chance) < ac;
}

/* type ==   0 to   9 : you shooting a wand */
/* type ==  10 to  19 : you casting a spell */
/* type ==  20 to  29 : you breathing as a monster */
/* type == -10 to -19 : monster casting spell */
/* type == -20 to -29 : monster breathing at you */
/* type == -30 to -39 : monster shooting a wand */
/* called with dx = dy = 0 with vertical bolts */
void
buzz(type,nd,sx,sy,dx,dy)
register int type, nd;
register xchar sx,sy;
register int dx,dy;
{
    int range, abstype = abs(type) % 10;
    struct rm *lev;
    register xchar lsx, lsy;
    struct monst *mon;
    coord save_bhitpos;
    boolean shopdamage = FALSE;
    register const char *fltxt;

    fltxt = flash_types[(type <= -30) ? abstype : abs(type)];
    if(u.uswallow) {
	register int tmp;

	if(type < 0) return;
	tmp = zhitm(u.ustuck, type, nd);
	if(!u.ustuck)	u.uswallow = 0;
	else	pline("%s rips into %s%s",
		      The(fltxt), mon_nam(u.ustuck), exclam(tmp));
	/* Using disintegration from the inside only makes a hole... */
	if (tmp == MAGIC_COOKIE)
	    u.ustuck->mhp = 0;
	if (u.ustuck->mhp < 1)
	    killed(u.ustuck);
	return;
    }
    if(type < 0) newsym(u.ux,u.uy);
    range = rn1(7,7);
    if(dx == 0 && dy == 0) range = 1;
    save_bhitpos = bhitpos;

    tmp_at(DISP_BEAM, zapdir_to_glyph(dx, dy, abstype));
    while(range-- > 0) {
	lsx = sx; sx += dx;
	lsy = sy; sy += dy;
	if(isok(sx,sy) && (lev = &levl[sx][sy])->typ) {
	    if(cansee(sx,sy)) {
		if(ZAP_POS(lev->typ) || cansee(lsx,lsy))
		    tmp_at(sx,sy);
		delay_output(); /* wait a little */
	    }
	} else
	    goto make_bounce;

	/* hit() and miss() need bhitpos to match the target */
	bhitpos.x = sx,  bhitpos.y = sy;
	/* Fireballs only damage when they explode */
	if (type != ZT_SPELL(ZT_FIRE))
	    range += zap_over_floor(sx, sy, type, &shopdamage);

	if ((mon = m_at(sx, sy)) != 0) {
	    if (type == ZT_SPELL(ZT_FIRE)) break;
	    if (type >= 0) mon->mstrategy &= ~STRAT_WAITMASK;
	    if (zap_hit(find_mac(mon))) {
		if (mon_reflects(mon, (char *)0)) {
		    if(cansee(mon->mx,mon->my)) {
			hit(fltxt, mon, exclam(0));
			shieldeff(mon->mx, mon->my);
			(void) mon_reflects(mon, "But it reflects from %s %s!");
		    }
		    dx = -dx;
		    dy = -dy;
		} else {
		    boolean mon_could_move = mon->mcanmove;
		    int tmp = zhitm(mon, type, nd);

		    if (is_rider(mon->data) && type == ZT_BREATH(ZT_DEATH)) {
			if (canseemon(mon)) {
			    hit(fltxt, mon, exclam(tmp));
			    pline("%s disintegrates.", Monnam(mon));
			    pline("%s body reintegrates before your %s!",
			         s_suffix(Monnam(mon)),
			         makeplural(body_part(EYE)));
		            pline("%s resurrects!", Monnam(mon));
			}
		        mon->mhp = mon->mhpmax;
			break; /* Out of while loop */
		    }
		    if(mon->data == &mons[PM_DEATH] && abstype == ZT_DEATH) {
		        if(cansee(mon->mx,mon->my)) {
			    hit(fltxt, mon, exclam(tmp));
		            pline("Death absorbs the deadly %s!",
				        type == ZT_BREATH(ZT_DEATH) ?
				        "blast" : "ray");
		            pline("It seems even stronger than before.");
		        }
		        break; /* Out of while loop */
		    }

		    if (tmp == MAGIC_COOKIE) { /* disintegration */
			struct obj *otmp, *otmp2;
			pline("%s is disintegrated!", Monnam(mon));
			mon->mgold = 0L;

#define oresist_disintegration(obj) \
		(objects[obj->otyp].oc_oprop == DISINT_RES || \
		 obj_resists(obj, 5, 50) || is_quest_artifact(obj))

			for (otmp = mon->minvent; otmp; otmp = otmp2) {
			    otmp2 = otmp->nobj;
			    if (!oresist_disintegration(otmp)) {
				obj_extract_self(otmp);
				obfree(otmp, (struct obj *)0);
			    }
			}

			if (type < 0)
			    monkilled(mon, (char *)0, -AD_RBRE);
			else
			    xkilled(mon, 2);
		    } else if(mon->mhp < 1) {
			if(type < 0)
			    monkilled(mon, fltxt, AD_RBRE);
			else
			    killed(mon);
		    } else {
			hit(fltxt, mon, exclam(tmp));

			if (mon_could_move && !mon->mcanmove)	/* ZT_SLEEP */
			    slept_monst(mon);
		    }
		}
		range -= 2;
	    } else {
		miss(fltxt,mon);
	    }
	} else if (sx == u.ux && sy == u.uy && range >= 0) {
	    nomul(0);
	    if (zap_hit((int) u.uac)) {
		range -= 2;
		pline("%s hits you!", The(fltxt));
		if (Reflecting) {
		    if (!Blind) {
			if(Reflecting & WORN_AMUL)
			    makeknown(AMULET_OF_REFLECTION);
			else
			    makeknown(SHIELD_OF_REFLECTION);
			pline("But it reflects from your %s!",
			      (Reflecting & W_AMUL) ? "amulet" : "shield");
		    } else
			pline("For some reason you are not affected.");
		    dx = -dx;
		    dy = -dy;
		    shieldeff(sx, sy);
		} else {
		    zhitu(type, nd, fltxt, sx, sy);
		}
	    } else {
		pline("%s whizzes by you!", The(fltxt));
	    }
	    if (abstype == ZT_LIGHTNING && !resists_blnd(&youmonst)) {
		You(are_blinded_by_the_flash);
		make_blinded((long)d(nd,50),FALSE);
	    }
	    stop_occupation();
	    nomul(0);
	}

	if(!ZAP_POS(lev->typ) || (closed_door(sx, sy) && (range >= 0))) {
	    int bounce;
	    uchar rmn;

 make_bounce:
	    if (type == ZT_SPELL(ZT_FIRE)) {
		sx = lsx;
		sy = lsy;
		break; /* fireballs explode before the wall */
	    }
	    bounce = 0;
	    range--;
	    if(range && isok(lsx, lsy) && cansee(lsx,lsy))
		pline("%s bounces!", The(fltxt));
	    if(!dx || !dy || !rn2(20)) {
		dx = -dx;
		dy = -dy;
	    } else {
		if(isok(sx,lsy) && ZAP_POS(rmn = levl[sx][lsy].typ) &&
		   (IS_ROOM(rmn) || (isok(sx+dx,lsy) &&
				     ZAP_POS(levl[sx+dx][lsy].typ))))
		    bounce = 1;
		if(isok(lsx,sy) && ZAP_POS(rmn = levl[lsx][sy].typ) &&
		   (IS_ROOM(rmn) || (isok(lsx,sy+dy) &&
				     ZAP_POS(levl[lsx][sy+dy].typ))))
		    if(!bounce || rn2(2))
			bounce = 2;

		switch(bounce) {
		case 0: dx = -dx; /* fall into... */
		case 1: dy = -dy; break;
		case 2: dx = -dx; break;
		}
		tmp_at(DISP_CHANGE, zapdir_to_glyph(dx,dy,abstype));
	    }
	}
    }
    tmp_at(DISP_END,0);
    if (type == ZT_SPELL(ZT_FIRE))
	explode(sx, sy, type, d(12,6), 0);
    if (shopdamage)
	pay_for_damage(abstype == ZT_FIRE ?  "burn away" :
		       abstype == ZT_COLD ?  "shatter" :
		       abstype == ZT_DEATH ? "disintegrate" : "destroy");
    bhitpos = save_bhitpos;
}
#endif /*OVLB*/
#ifdef OVL0

void
melt_ice(x, y)
xchar x, y;
{
	struct rm *lev = &levl[x][y];
	struct obj *otmp;

	if (lev->typ == DRAWBRIDGE_UP)
	    lev->drawbridgemask &= ~DB_ICE;	/* revert to DB_MOAT */
	else {	/* lev->typ == ICE */
#ifdef STUPID
	    if (lev->icedpool == ICED_POOL) lev->typ = POOL;
	    else lev->typ = MOAT;
#else
	    lev->typ = (lev->icedpool == ICED_POOL ? POOL : MOAT);
#endif
	    lev->icedpool = 0;
	}
	obj_ice_effects(x, y, FALSE);
	unearth_objs(x, y);
	if (Underwater) vision_recalc(1);
	newsym(x,y);
	if (cansee(x,y)) Norep("The ice crackles and melts.");
	if ((otmp = sobj_at(BOULDER, x, y)) != 0) {
	    if (cansee(x,y)) pline("%s settles...", An(xname(otmp)));
	    do {
		obj_extract_self(otmp);	/* boulder isn't being pushed */
		if (!boulder_hits_pool(otmp, x, y, FALSE))
		    impossible("melt_ice: no pool?");
		/* try again if there's another boulder and pool didn't fill */
	    } while (is_pool(x,y) && (otmp = sobj_at(BOULDER, x, y)) != 0);
	    newsym(x,y);
	}
	if (x == u.ux && y == u.uy)
		spoteffects();	/* possibly drown, notice objects */
}

/* Burn floor scrolls, evaporate pools, etc...  in a single square.  Used
 * both for normal bolts of fire, cold, etc... and for fireballs.
 * Sets shopdamage to TRUE if a shop door is destroyed, and returns the
 * amount by which range is reduced (the latter is just ignored by fireballs)
 */
int
zap_over_floor(x, y, type, shopdamage)
xchar x, y;
int type;
boolean *shopdamage;
{
	struct monst *mon;
	int abstype = abs(type) % 10;
	struct rm *lev = &levl[x][y];
	int rangemod = 0;

	if(abstype == ZT_FIRE) {
	    if(is_ice(x, y)) {
		melt_ice(x, y);
	    } else if(is_pool(x,y)) {
		const char *msgtxt = "You hear hissing gas.";
		if(lev->typ != POOL) {	/* MOAT or DRAWBRIDGE_UP */
		    if (cansee(x,y)) msgtxt = "Some water evaporates.";
		} else {
		    register struct trap *ttmp;

		    rangemod -= 3;
		    lev->typ = ROOM;
		    ttmp = maketrap(x, y, PIT);
		    if (ttmp) ttmp->tseen = 1;
		    if (cansee(x,y)) msgtxt = "The water evaporates.";
		}
		Norep(msgtxt);
		if (lev->typ == ROOM) newsym(x,y);
	    } else if(IS_FOUNTAIN(lev->typ)) {
		    if (cansee(x,y))
			pline("Steam billows from the fountain.");
		    rangemod -= 1;
		    dryup(x,y);
	    }
	}
	else if(abstype == ZT_COLD && (is_pool(x,y) || is_lava(x,y))) {
		boolean lava = is_lava(x,y);
		boolean moat = (!lava && (lev->typ != POOL) &&
				(lev->typ != WATER) &&
				!Is_medusa_level(&u.uz) &&
				!Is_waterlevel(&u.uz));

		if (lev->typ == WATER) {
		    /* For now, don't let WATER freeze. */
		    if (cansee(x,y))
			pline_The("water freezes for a moment.");
		    else
			You_hear("a soft crackling.");
		    rangemod -= 1000;	/* stop */
		} else {
		    rangemod -= 3;
		    if (lev->typ == DRAWBRIDGE_UP) {
			lev->drawbridgemask &= ~DB_UNDER;  /* clear lava */
			lev->drawbridgemask |= (lava ? DB_FLOOR : DB_ICE);
		    } else {
			if (!lava)
			    lev->icedpool =
				    (lev->typ == POOL ? ICED_POOL : ICED_MOAT);
			lev->typ = (lava ? ROOM : ICE);
		    }
		    bury_objs(x,y);
		    if(cansee(x,y)) {
			if(moat)
				Norep("The moat is bridged with ice!");
			else if(lava)
				Norep("The lava cools and solidifies.");
			else
				Norep("The water freezes.");
			newsym(x,y);
		    } else if(flags.soundok && !lava)
			You_hear("a crackling sound.");

		    if (x == u.ux && y == u.uy) {
			if (u.uinwater) {   /* not just `if (Underwater)' */
			    /* leave the no longer existent water */
			    u.uinwater = 0;
			    docrt();
			    vision_full_recalc = 1;
			} else if (u.utrap && u.utraptype == TT_LAVA) {
			    if (passes_walls(uasmon)) {
				You("pass through the now-solid rock.");
			    } else {
				u.utrap = rn1(50,20);
				u.utraptype = TT_INFLOOR;
				You("are firmly stuck in the cooling rock.");
			    }
			}
		    } else if ((mon = m_at(x,y)) != 0) {
			/* probably ought to do some hefty damage to any
			   non-ice creature caught in freezing water;
			   at a minimum, eels are forced out of hiding */
			if (is_swimmer(mon->data) && mon->mundetected) {
			    mon->mundetected = 0;
			    newsym(x,y);
			}
		    }
		}
		obj_ice_effects(x,y,TRUE);
	}
	if(closed_door(x, y)) {
		int new_doormask = -1;
		const char *see_txt = 0, *sense_txt = 0, *hear_txt = 0;
		rangemod = -1000;
		switch(abstype) {
		case ZT_FIRE:
		    new_doormask = D_NODOOR;
		    see_txt = "The door is consumed in flames!";
		    sense_txt = "smell smoke.";
		    break;
		case ZT_COLD:
		    new_doormask = D_NODOOR;
		    see_txt = "The door freezes and shatters!";
		    sense_txt = "feel cold.";
		    break;
		case ZT_DEATH:
		    /* death spells/wands don't disintegrate */
		    if(abs(type) != ZT_BREATH(ZT_DEATH))
			goto def_case;
		    new_doormask = D_NODOOR;
		    see_txt = "The door disintegrates!";
		    hear_txt = "crashing wood.";
		    break;
		case ZT_LIGHTNING:
		    new_doormask = D_BROKEN;
		    see_txt = "The door splinters!";
		    hear_txt = "crackling.";
		    break;
		default:
		def_case:
		    if(cansee(x,y)) {
			pline_The("door absorbs %s %s!",
			      (type < 0) ? "the" : "your",
			      abs(type) < ZT_SPELL(0) ? "bolt" :
			      abs(type) < ZT_BREATH(0) ? "spell" :
			      "blast");
		    } else You_feel("vibrations.");
		    break;
		}
		if (new_doormask >= 0) {	/* door gets broken */
		    if (*in_rooms(x, y, SHOPBASE)) {
			if (type >= 0) {
			    add_damage(x, y, 400L);
			    *shopdamage = TRUE;
			} else	/* caused by monster */
			    add_damage(x, y, 0L);
		    }
		    lev->doormask = new_doormask;
		    unblock_point(x, y);	/* vision */
		    if (cansee(x, y)) {
			pline(see_txt);
			newsym(x, y);
		    } else if (sense_txt) {
			You(sense_txt);
		    } else if (hear_txt) {
			if (flags.soundok) You_hear(hear_txt);
		    }
		    if (picking_at(x, y)) {
			stop_occupation();
			reset_pick();
		    }
		}
	}

	if(OBJ_AT(x, y) && abstype == ZT_FIRE)
		if (burn_floor_paper(x, y, FALSE) && couldsee(x, y))  {
		    newsym(x,y);
		    You("%s of smoke.",
			!Blind ? "see a puff" : "smell a whiff");
		}
	if ((mon = m_at(x,y)) != 0) {
		/* Cannot use wakeup() which also angers the monster */
		mon->msleep = 0;
		if(mon->m_ap_type) seemimic(mon);
		if(type >= 0) {
		    setmangry(mon);
		    if(mon->ispriest && *in_rooms(mon->mx, mon->my, TEMPLE))
			ghod_hitsu(mon);
		    if(mon->isshk && !*u.ushops)
			hot_pursuit(mon);
		}
	}
	return rangemod;
}
#endif /*OVL0*/
#ifdef OVL3
void
fracture_rock(obj)	/* fractured by pick-axe or wand of striking */
register struct obj *obj;		   /* no texts here! */
{
	obj->otyp = ROCK;
	obj->quan = (long) rn1(60, 7);
	obj->owt = weight(obj);
	obj->oclass = GEM_CLASS;
	obj->known = FALSE;
	obj->onamelth = 0;		/* no names */
	obj->oxlth = 0;			/* no extra data */
	obj->mtraits = 0;
	if(!does_block(obj->ox,obj->oy,&levl[obj->ox][obj->oy]))
	    unblock_point(obj->ox,obj->oy);
	if(cansee(obj->ox,obj->oy))
	    newsym(obj->ox,obj->oy);
}

boolean
break_statue(obj)
register struct obj *obj;
{
	struct trap *trap;
	struct obj *item;

	if((trap = t_at(obj->ox,obj->oy)) && trap->ttyp == STATUE_TRAP)
	    if(makemon(&mons[obj->corpsenm], obj->ox, obj->oy, NO_MM_FLAGS)) {
		pline("Instead of shattering, the statue suddenly comes alive!");
		delobj(obj);
		deltrap(trap);
		return FALSE;
	    }
	while ((item = obj->cobj) != 0) {
	    obj_extract_self(item);
	    place_object(item, obj->ox, obj->oy);
	}
	fracture_rock(obj);
	return TRUE;
}

const char *destroy_strings[] = {
	"freezes and shatters", "freeze and shatter", "shattered potion",
	"boils and explodes", "boil and explode", "boiling potion",
	"catches fire and burns", "catch fire and burn", "burning scroll",
	"catches fire and burns", "catch fire and burn", "burning book",
	"turns to dust and vanishes", "turn to dust and vanish", "",
	"breaks apart and explodes", "break apart and explode", "exploding wand"
};

void
destroy_item(osym, dmgtyp)
register int osym, dmgtyp;
{
	register struct obj *obj, *obj2;
	register int dmg, xresist, skip;
	register long i, cnt, quan;
	register int dindx;
	const char *mult;

	for(obj = invent; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(obj->oclass != osym) continue; /* test only objs of type osym */
	    if(obj->oartifact) continue; /* don't destroy artifacts */
	    xresist = skip = 0;
#ifdef GCC_WARN
	    dmg = dindx = 0;
	    quan = 0L;
#endif
	    switch(dmgtyp) {
		case AD_COLD:
		    if(osym == POTION_CLASS && obj->otyp != POT_OIL) {
			quan = obj->quan;
			dindx = 0;
			dmg = rnd(4);
		    } else skip++;
		    break;
		case AD_FIRE:
		    xresist = (Fire_resistance && obj->oclass != POTION_CLASS);

		    if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
			skip++;
		    if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
			skip++;
			if (!Blind)
			    pline("%s glows a strange %s, but remains intact.",
				The(xname(obj)), hcolor("dark red"));
		    }
		    quan = obj->quan;
		    switch(osym) {
			case POTION_CLASS:
			    dindx = 1;
			    dmg = rnd(6);
			    break;
			case SCROLL_CLASS:
			    dindx = 2;
			    dmg = 1;
			    break;
			case SPBOOK_CLASS:
			    dindx = 3;
			    dmg = 1;
			    break;
			default:
			    skip++;
			    break;
		    }
		    break;
		case AD_ELEC:
		    xresist = (Shock_resistance && obj->oclass != RING_CLASS);
		    quan = obj->quan;
		    switch(osym) {
			case RING_CLASS:
			    if(obj->otyp == RIN_SHOCK_RESISTANCE)
				    { skip++; break; }
			    dindx = 4;
			    dmg = 0;
			    break;
			case WAND_CLASS:
			    if(obj->otyp == WAN_LIGHTNING) { skip++; break; }
#if 0
			    if (obj == current_wand) { skip++; break; }
#endif
			    dindx = 5;
			    dmg = rnd(10);
			    break;
			default:
			    skip++;
			    break;
		    }
		    break;
		default:
		    skip++;
		    break;
	    }
	    if(!skip) {
		for(i = cnt = 0L; i < quan; i++)
		    if(!rn2(3)) cnt++;

		if(!cnt) continue;
		if(cnt == quan)	mult = "Your";
		else	mult = (cnt == 1L) ? "One of your" : "Some of your";
		pline("%s %s %s!", mult, xname(obj),
			(cnt > 1L) ? destroy_strings[dindx*3 + 1]
				  : destroy_strings[dindx*3]);
		if(osym == POTION_CLASS && dmgtyp != AD_COLD)
		    potionbreathe(obj);
		if (obj->owornmask) {
		    if (obj->owornmask & W_RING) /* ring being worn */
			Ring_gone(obj);
		    else
			setnotworn(obj);
		}
		if (obj == current_wand) current_wand = 0;	/* destroyed */
		for (i = 0; i < cnt; i++)
		    useup(obj);
		if(dmg) {
		    if(xresist)	You("aren't hurt!");
		    else {
		        losehp(dmg, (cnt==1L) ? destroy_strings[dindx*3 + 2] :
			       (const char *)makeplural(destroy_strings[dindx*3 + 2]),
			       (cnt==1L) ? KILLED_BY_AN : KILLED_BY);
			exercise(A_STR, FALSE);
		   }
		}
	    }
	}
	return;
}

int
destroy_mitem(mtmp, osym, dmgtyp)
register struct monst *mtmp;
register int osym, dmgtyp;
{
	register struct obj *obj, *obj2;
	register int skip, tmp = 0;
	register long i, cnt, quan;
	register int dindx;
	boolean vis=canseemon(mtmp);

	for(obj = mtmp->minvent; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(obj->oclass != osym) continue; /* test only objs of type osym */
	    skip = 0;
	    quan = 0L;
	    dindx = 0;

	    switch(dmgtyp) {
		case AD_COLD:
		    if(osym == POTION_CLASS && obj->otyp != POT_OIL) {
			quan = obj->quan;
			dindx = 0;
			tmp++;
		    } else skip++;
		    break;
		case AD_FIRE:
		    if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
			skip++;
		    if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
			skip++;
			if (vis)
			    pline("%s glows a strange %s, but remains intact.",
				The(distant_name(obj, xname)),
				hcolor("dark red"));
		    }
		    quan = obj->quan;
		    switch(osym) {
			case POTION_CLASS:
			    dindx = 1;
			    tmp++;
			    break;
			case SCROLL_CLASS:
			    dindx = 2;
			    tmp++;
			    break;
			case SPBOOK_CLASS:
			    dindx = 3;
			    tmp++;
			    break;
			default:
			    skip++;
			    break;
		    }
		    break;
		case AD_ELEC:
		    quan = obj->quan;
		    switch(osym) {
			case RING_CLASS:
			    if(obj->otyp == RIN_SHOCK_RESISTANCE)
				    { skip++; break; }
			    dindx = 4;
			    break;
			case WAND_CLASS:
			    if(obj->otyp == WAN_LIGHTNING) { skip++; break; }
			    dindx = 5;
			    tmp++;
			    break;
			default:
			    skip++;
			    break;
		    }
		    break;
		default:
		    skip++;
		    break;
	    }
	    if(!skip) {
		for(i = cnt = 0L; i < quan; i++)
		    if(!rn2(3)) cnt++;

		if(!cnt) continue;
		if (vis) pline("%s %s %s!",
			s_suffix(Monnam(mtmp)), xname(obj),
			(cnt > 1L) ? destroy_strings[dindx*3 + 1]
				  : destroy_strings[dindx*3]);
		for(i = 0; i < cnt; i++) m_useup(mtmp, obj);
	    }
	}
	return(tmp);
}

#endif /*OVL3*/
#ifdef OVL2

int
resist(mtmp, oclass, damage, tell)
struct monst *mtmp;
char oclass;
int damage, tell;
{
	int resisted;
	int alev, dlev;

	/* attack level */
	switch (oclass) {
	    case WAND_CLASS:	alev = 12;	 break;
	    case SCROLL_CLASS:	alev =  9;	 break;
	    case POTION_CLASS:	alev =  6;	 break;
	    default:		alev = u.ulevel; break;		/* spell */
	}
	/* defense level */
	dlev = (int)mtmp->m_lev;
	if (dlev > 50) dlev = 50;
	else if (dlev < 1) dlev = is_mplayer(mtmp->data) ? u.ulevel : 1;

	resisted = rn2(100 + alev - dlev) < mtmp->data->mr;
	if(resisted) {

		if(tell) {
		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s resists!", Monnam(mtmp));
		}
		mtmp->mhp -= damage/2;
	} else  mtmp->mhp -= damage;

	if(mtmp->mhp < 1) {
		if(m_using) monkilled(mtmp, "", AD_RBRE);
		else killed(mtmp);
	}
	return(resisted);
}

void
makewish()
{
	char buf[BUFSZ];
	register struct obj *otmp;
	int tries = 0;

	if (flags.verbose) You("may wish for an object.");
retry:
	getlin("For what do you wish?", buf);
	if(buf[0] == '\033') buf[0] = 0;
	/*
	 *  Note: if they wished for and got a non-object successfully,
	 *  otmp == &zeroobj
	 */
	otmp = readobjnam(buf);
	if (!otmp) {
	    pline("Nothing fitting that description exists in the game.");
	    if (++tries < 5) goto retry;
	    pline(thats_enough_tries);
	    if (!(otmp = readobjnam((char *)0)))
		return; /* for safety; should never happen */
	}
	if (otmp != &zeroobj) {
	    if(otmp->oartifact && !touch_artifact(otmp,&youmonst))
		dropy(otmp);
	    else
		/* The(aobjnam()) is safe since otmp is unidentified -dlc */
		(void) hold_another_object(otmp, u.uswallow ?
				       "Oops!  %s out of your reach!" :
				       Is_airlevel(&u.uz) || u.uinwater ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(otmp,
					     Is_airlevel(&u.uz) || u.uinwater ?
						   "slip" : "drop")),
				       (const char *)0);
	    u.ublesscnt += rn1(100,50);  /* the gods take notice */
	}
}

#endif /*OVL2*/

/*zap.c*/
