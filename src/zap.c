/*	SCCS Id: @(#)zap.c	3.1	93/06/16	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Disintegration rays have special treatment; corpses are never left.
 * But the routine which calculates the damage is separate from the routine
 * which kills the monster.  The damage routine returns this cookie to
 * indicate that the monster should be disintegrated.
 */
#define MAGIC_COOKIE 1000

static NEARDATA boolean obj_zapped;
static NEARDATA int poly_zapped;

#ifdef MUSE
/* kludge to use mondied instead of killed */
extern boolean m_using;
#endif

static boolean FDECL(obj_shudders, (struct obj *));
static void FDECL(polyuse, (struct obj*, int, int));
static void FDECL(do_osshock, (struct obj *));
static void FDECL(create_polymon, (struct obj *));
static int FDECL(burn_floor_paper,(int,int));
static void FDECL(costly_cancel, (struct obj *));
static void FDECL(cancel_item, (struct obj *));
static int FDECL(bhitm, (struct monst *,struct obj *));
#ifndef MUSE
STATIC_PTR int FDECL(bhito, (struct obj *,struct obj *));
#endif
STATIC_PTR int FDECL(bhitpile, (struct obj *,int (*)(OBJ_P,OBJ_P),int,int));
static void FDECL(backfire, (struct obj *));
static int FDECL(zhit, (struct monst *,int,int));

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
	"bolt of lightning",
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
static int
bhitm(mtmp, otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	register boolean wake = FALSE;
	register int otyp = otmp->otyp;
#ifdef MULDGN
	boolean dbldam = (pl_character[0] == 'K') && u.uhave.questart;
#endif
	register int dmg;

	switch(otyp) {
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
		wake = TRUE;
		if (u.uswallow || rnd(20) < 10 + find_mac(mtmp)) {
			dmg = d(2,12);
#ifdef MULDGN
			if(dbldam) dmg *= 2;
#endif
			hit((otyp == WAN_STRIKING) ? "wand" : "spell",
			    mtmp, exclam(dmg));
			(void) resist(mtmp, otmp->oclass, dmg, TELL);
		} else miss((otyp == WAN_STRIKING) ? "wand" : "spell", mtmp);
		makeknown(otyp);
		break;
	case WAN_SLOW_MONSTER:
	case SPE_SLOW_MONSTER:
		wake = TRUE;
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
		wake = TRUE;
		break;
	case WAN_UNDEAD_TURNING:
	case SPE_TURN_UNDEAD:
		if (is_undead(mtmp->data)) {
			dmg = rnd(8);
#ifdef MULDGN
			if(dbldam) dmg *= 2;
#endif
			if(!resist(mtmp, otmp->oclass, dmg, NOTELL))
				mtmp->mflee = TRUE;
			wake = TRUE;
		}
		break;
	case WAN_POLYMORPH:
	case SPE_POLYMORPH:
		wake = TRUE;
		if(!resist(mtmp, otmp->oclass, 0, NOTELL)) {
		    if (!rn2(25)) {
			if (canseemon(mtmp)) {
			    pline("%s shudders!", Monnam(mtmp));
			    makeknown(otyp);
			}
			/* no corpse after system shock */
			xkilled(mtmp, 3);
		    }
		    else if (newcham(mtmp, (struct permonst *)0) )
			if (!Hallucination && (!Blind || sensemon(mtmp)))
			    makeknown(otyp);
		}
		break;
	case WAN_CANCELLATION:
	case SPE_CANCELLATION:
		wake = TRUE;
		cancel_monst(mtmp, otmp, TRUE, TRUE, FALSE);
		break;
	case WAN_TELEPORTATION:
	case SPE_TELEPORT_AWAY:
		if(mtmp->ispriest && *in_rooms(mtmp->mx, mtmp->my, TEMPLE)) {
		    pline("%s resists your magic!", Monnam(mtmp));
		    wake = TRUE;
		    break;
		}
		wake = TRUE;
		if(mtmp->isshk) rloc_shk(mtmp);
		else rloc(mtmp);
		break;
	case WAN_MAKE_INVISIBLE:
		mtmp->minvis = TRUE;
		newsym(mtmp->mx,mtmp->my);	/* make monster disappear */
		if (mtmp->wormno) see_wsegs(mtmp); /* and tail too */
		wake = TRUE;
		break;
	case WAN_NOTHING:
		break;
	case WAN_PROBING:
		makeknown(otyp);
		mstatusline(mtmp);
		break;
	case WAN_OPENING:
		if(u.uswallow && mtmp == u.ustuck) {
			if (is_animal(mtmp->data)) {
				if (Blind) pline("Its mouth opens!");
				else pline("%s opens its mouth!", Monnam(mtmp));
			}
			expels(mtmp, mtmp->data, TRUE);
		}
			break;
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
		mtmp->mhp += (otyp == SPE_HEALING) ? rnd(8) : d(2,8)+2;
		if (mtmp->mhp > mtmp->mhpmax) 
		    mtmp->mhp = mtmp->mhpmax;
		if (canseemon(mtmp))
		    pline(otyp == SPE_HEALING ? "%s begins to look better." :
			  "%s looks much better.", Monnam(mtmp));
		if (mtmp->mtame || mtmp->mpeaceful) {		    
		    adjalign((pl_character[0] == 'H') ? 1 :
			     sgn(u.ualign.type));
		}
 		break;
	case WAN_LOCKING:
	case SPE_KNOCK:
	case SPE_WIZARD_LOCK:
		break;
	default:
		impossible("What an interesting effect (%u)", otyp);
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

struct monst *
revive(obj,ininv)
register struct obj *obj;
boolean ininv;
{
	register struct monst *mtmp = (struct monst *)0;

	if(obj->otyp == CORPSE) {
		int montype = obj->corpsenm;
		int x = ininv ? u.ux : obj->ox;
		int y = ininv ? u.uy : obj->oy;

		if (cant_create(&montype)) { /* will make zombie instead */
			mtmp = makemon(&mons[PM_HUMAN_ZOMBIE], x, y);
			if (mtmp) {
				mtmp->mhp = mtmp->mhpmax = 100;
				mtmp->mspeed = MFAST;
			}
		} else {
			struct obj *otmp;
#if defined(ARMY) && !defined(MUSE)
			if (is_mercenary(&mons[montype]))
				montype = PM_UNARMORED_SOLDIER;
#endif
			mtmp = makemon(&mons[montype], x, y);
			if (mtmp) {
				/* Monster retains its name */
				if (obj->onamelth)
					mtmp = christen_monst(mtmp, ONAME(obj));
				/* No inventory for newly revived monsters */
				while ((otmp = (mtmp->minvent)) != 0) {
					mtmp->minvent = otmp->nobj;
					dealloc_obj(otmp);
				}
			}
		}
		if (mtmp && obj->oeaten)
			mtmp->mhp = eaten_stat(mtmp->mhp, obj);
		if (ininv) useup(obj);
		else {
			/* not useupf(), which charges */
			if (obj->quan > 1L) obj->quan--;
			else delobj(obj);
		}
		if (x != u.ux || y != u.uy || Invisible)
			newsym(x, y);
	}
	return mtmp;
}

static NEARDATA const char charged_objs[] = { WAND_CLASS, WEAPON_CLASS, ARMOR_CLASS, 0 };

static void
costly_cancel(obj)
register struct obj *obj;
{
	register struct monst *shkp = (struct monst *)0;

	if (carried(obj)) {
		if (obj->unpaid) {
	            shkp = shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));
		    if (!shkp) return;
		    Norep("You cancel an unpaid object, you pay for it!");
		    bill_dummy_object(obj);
		}
	} else {
	        shkp = shop_keeper(*in_rooms(obj->ox, obj->oy, SHOPBASE));
		if (!shkp || !inhishop(shkp)) return;
		if (!costly_spot(obj->ox, obj->oy)) return;
		if (costly_spot(u.ux, u.uy) && 
				*in_rooms(u.ux, u.uy, 0) ==
					*in_rooms(shkp->mx, shkp->my, 0)) {
		    Norep("You cancel it, you pay for it!");
		    bill_dummy_object(obj);
		} else
		    (void) stolen_value(obj, obj->ox, obj->oy, FALSE, FALSE);
	}
}

/* cancel obj, possibly carried by you or a monster */
static void
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
	if(obj->spe &&
	  !(obj->otyp == WAN_CANCELLATION || /* can't cancel cancellation */
	    obj->otyp == TIN || obj->otyp == EGG ||
	    obj->otyp == STATUE ||
	    obj->otyp == MAGIC_LAMP ||
#ifdef MAIL
	    obj->otyp == SCR_MAIL ||
#endif
#ifdef TUTTI_FRUTTI
	    obj->otyp == SLIME_MOLD ||
#endif
	    obj->otyp == SKELETON_KEY ||
	    obj->otyp == LARGE_BOX || obj->otyp == CHEST ||
	    obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN ||
	    Is_candle(obj) || obj->otyp == CANDELABRUM_OF_INVOCATION)) {
	        costly_cancel(obj);
		obj->spe = (obj->oclass == WAND_CLASS) ? -1 : 0;
		if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
	    }
	if (obj->oclass == SCROLL_CLASS
	    && obj->otyp != SCR_BLANK_PAPER
#ifdef MAIL
	    && obj->otyp != SCR_MAIL
#endif
	   ) {
	    costly_cancel(obj);
	    obj->otyp = SCR_BLANK_PAPER;
	    if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
        }
	if (obj->oclass == SPBOOK_CLASS &&
	                   obj->otyp != SPE_BOOK_OF_THE_DEAD &&
	                   obj->otyp != SPE_BLANK_PAPER) {
	    costly_cancel(obj);
	    obj->otyp = SPE_BLANK_PAPER;
	    if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
	}
	if (obj->oclass == POTION_CLASS && obj->otyp != POT_BOOZE) {
	    if (obj->otyp==POT_SICKNESS ||
	                 obj->otyp==POT_SEE_INVISIBLE) {
	            costly_cancel(obj);
	            obj->otyp = POT_FRUIT_JUICE;
		    if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
	    } else {
	            if (obj->otyp != POT_FRUIT_JUICE &&
			                     obj->otyp != POT_WATER) {
		        costly_cancel(obj);
	                obj->otyp = POT_WATER;
			if (unpaid) addtobill(obj, TRUE, FALSE, TRUE);
		    }
	    }
	    /* sickness is "biologically contaminated" fruit juice; cancel it
	     * and it just becomes fruit juice... whereas see invisible
	     * tastes like "enchanted" fruit juice, it similarly cancels.
	     */
	}
	if (holy) costly_cancel(obj);
	unbless(obj);
	if (unpaid && holy) addtobill(obj, TRUE, FALSE, TRUE);
	uncurse(obj);
}

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

static boolean
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

/* Use up at least minwt number of things made of material mat.
 * There's also a chance that other stuff will be used up.  Finally,
 * there's a random factor here to keep from always using the stuff
 * at the top of the pile.
 */
static void
polyuse(objhdr, mat, minwt)
    struct obj *objhdr;
    int mat, minwt;
{
    register struct obj *otmp, *otmp2;

    for(otmp = objhdr; minwt > 0 && otmp; otmp = otmp2) {
	otmp2 = otmp->nexthere;
	if((objects[otmp->otyp].oc_material == mat) == (rn2(minwt+1) != 0)) {
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
static void
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

	if (! (mons[pm_index].geno & G_GENOD))
		mdat = &mons[pm_index];

	mtmp = makemon(mdat, obj->ox, obj->oy);
	polyuse(obj, poly_zapped, (int)mons[pm_index].cwt);

	if(!Blind && mtmp) {
	    pline("Some %sobjects meld, and %s arises from the pile!",
		  material, a_monnam(mtmp));
	}
}

static void
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
	if (obj->quan > 1L) {
		struct obj *obj2;

		obj2 = splitobj(obj, (long)rnd((int)obj->quan - 1));
		move_object(obj2, obj->ox, obj->oy);
	}

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

#ifndef MUSE
STATIC_PTR
#endif
int
bhito(obj, otmp)	/* object obj was hit by the effect of wand otmp */
register struct obj *obj, *otmp;	/* returns TRUE if sth was done */
{
	register int res = 1;
	struct obj *otmp2;

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
		} else if (obj_shudders(obj)) {
		    if (cansee(obj->ox, obj->oy))
			makeknown(otmp->otyp);
		    do_osshock(obj);
		    break;
		}

		/* preserve symbol and quantity */
		otmp2 = mkobj_at(obj->oclass, obj->ox, obj->oy, FALSE);
		otmp2->quan = obj->quan;
		/* preserve the shopkeepers (lack of) interest */
		otmp2->no_charge = obj->no_charge;
#ifdef MAIL
		/* You can't send yourself 100 mail messages and then
		 * polymorph them into useful scrolls
		 */
		if (obj->otyp == SCR_MAIL) {
			otmp2->otyp = SCR_MAIL;
			otmp2->spe = 1;
		}
#endif

		/* avoid abusing eggs laid by you */
		if (obj->otyp == EGG && obj->spe) {
			otmp2->otyp = EGG;
			otmp2->spe = 1;
			otmp2->corpsenm = random_monster();
			while (!lays_eggs(&mons[otmp2->corpsenm]))
				otmp2->corpsenm = random_monster();
		}

		/* keep special fields (including charges on wands) */
		if (index(charged_objs, otmp2->oclass)) otmp2->spe = obj->spe;

		otmp2->cursed = obj->cursed;
		otmp2->blessed = obj->blessed;
		otmp2->oeroded = obj->oeroded;
		otmp2->oerodeproof = obj->oerodeproof;

		/* reduce spellbook abuse */
		if (obj->oclass == SPBOOK_CLASS)
		    otmp2->spestudied = obj->spestudied + 1;

		/* Keep chest/box traps and poisoned ammo if we may */
		if (obj->otrapped && Is_box(otmp2))
			otmp2->otrapped = TRUE;
		if (obj->opoisoned &&
		    (otmp2->oclass == WEAPON_CLASS && otmp2->otyp <= SHURIKEN))
			otmp2->opoisoned = TRUE;

		if (obj->otyp == CORPSE) {
		/* turn crocodile corpses into shoes */
		    if (obj->corpsenm == PM_CROCODILE) {
			otmp2->otyp = LOW_BOOTS;
			otmp2->oclass = ARMOR_CLASS;
			otmp2->spe = 0;
			otmp2->oeroded = 0;
			otmp2->oerodeproof = TRUE;
			otmp2->quan = 1L;
			otmp2->cursed = FALSE;
		    }
		}

		/* no box contents --KAA */
		if (Has_contents(otmp2))
			delete_contents(otmp2);

		/* 'n' merged objects may be fused into 1 object */
		if (otmp2->quan > 1L &&
			(!objects[otmp2->otyp].oc_merge ||
				otmp2->quan > (long)rn2(1000)))
			otmp2->quan = 1L;

		if(otmp2->otyp == MAGIC_LAMP) otmp2->otyp = OIL_LAMP;

		while(otmp2->otyp == WAN_WISHING ||
					    otmp2->otyp == WAN_POLYMORPH)
			otmp2->otyp = rnd_class(WAN_LIGHT, WAN_LIGHTNING);

		/* update the weight */
		otmp2->owt = weight(otmp2);

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
		break;
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
		if (obj->otyp == BOULDER)
			fracture_rock(obj);
		else if (obj->otyp == STATUE)
			(void) break_statue(obj);
		else {
			(void)breaks(obj, FALSE);
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
		res = !!revive(obj,FALSE);
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
	case WAN_PROBING:
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
		res = 0;
		break;
	default:
		impossible("What an interesting effect (%u)", otmp->otyp);
	}
	return(res);
}

STATIC_PTR
int
bhitpile(obj,fhito,tx,ty)
    register struct obj *obj;	/* returns nonzero of something was hit */
    int FDECL((*fhito), (OBJ_P, OBJ_P));
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
	switch(obj->otyp) {
		case WAN_LIGHT:
		case SPE_LIGHT:
			litroom(TRUE,obj);
			break;
		case WAN_SECRET_DOOR_DETECTION:
		case SPE_DETECT_UNSEEN:
			if(!findit()) return;
			break;
		case WAN_CREATE_MONSTER:
			{ register int cnt = 1;
			if(!rn2(23)) cnt += rn2(7) + 1;
			while(cnt--)
			    (void) makemon((struct permonst *) 0, u.ux, u.uy);
			}
			break;
		case WAN_WISHING:
			if(Luck + rn2(5) < 0) {
				pline("Unfortunately, nothing happens.");
				break;
			}
			makewish();
			break;
	}
	if (!objects[obj->otyp].oc_name_known &&
	    (!Blind || obj->otyp == WAN_WISHING)) {
			makeknown(obj->otyp);
			more_experienced(0,10);
	}
}

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

	if(check_capacity(NULL)) return(0);
	obj = getobj(zap_syms, "zap");
	if(!obj) return(0);

	check_unpaid(obj);

	/* zappable addition done by GAN 11/03/86 */
	if(!zappable(obj)) pline(nothing_happens);
	else if(obj->cursed && !rn2(100)) {
		backfire(obj);	/* the wand blows up in your face! */
		exercise(A_STR, FALSE);
		return(1);
	} else if(!(objects[obj->otyp].oc_dir == NODIR) && !getdir(NULL)) {
		if (!Blind)
		    pline("%s glows and fades.", The(xname(obj)));
		/* make him pay for knowing !NODIR */
	} else if(!u.dx && !u.dy && !u.dz && !(objects[obj->otyp].oc_dir == NODIR)) {
	    if((damage = zapyourself(obj)))
		losehp(damage, self_pronoun("zapped %sself with a wand", "him"),
			NO_KILLER_PREFIX);
	} else {
		weffects(obj);
	}
	if (obj->spe < 0) {
	    pline("%s turns to dust.", The(xname(obj)));
	    useup(obj);
	}
	return(1);
}

int
zapyourself(obj)
	register struct obj	*obj;
{
	int	damage = 0;

	switch(obj->otyp) {
		case WAN_STRIKING:
		case SPE_FORCE_BOLT:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline("Boing!");
		    } else {
			You("bash yourself!");
			damage=d(8,6);
			exercise(A_STR, FALSE);
		    }
		    makeknown(obj->otyp);
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
#ifdef POLYSELF
			ugolemeffects(AD_ELEC, d(12,6));
#endif
		    }
		    destroy_item(WAND_CLASS, AD_ELEC);
		    destroy_item(RING_CLASS, AD_ELEC);
		    if(!Blind) {
			    You("are blinded by the flash!");
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
			You("feel rather warm.");
#ifdef POLYSELF
			ugolemeffects(AD_FIRE, d(12,6));
#endif
		    } else {
			pline("You've set yourself afire!");
			damage = d(12,6);
		    }
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
			You("feel a little chill.");
#ifdef POLYSELF
			ugolemeffects(AD_COLD, d(12,6));
#endif
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
			pline("The missiles bounce!");
		    } else {
			damage = d(4,6);
			pline("Idiot!  You've shot yourself!");
		    }
		    break;
		case WAN_POLYMORPH:
#ifdef POLYSELF
		    makeknown(WAN_POLYMORPH);
#endif
		case SPE_POLYMORPH:
#ifdef POLYSELF
		    polyself();
#else
		    newman();
#endif
		    break;
		case WAN_CANCELLATION:
		case SPE_CANCELLATION:
		    cancel_monst(&youmonst, obj, TRUE, FALSE, TRUE);
		    break;
		case WAN_MAKE_INVISIBLE: {
		    /* have to test before changing HInvis but must change
		     * HInvis before doing newsym().
		     */
		    int msg = (!Blind && !Invis && !See_invisible);

		    HInvis |= FROMOUTSIDE;
		    if (msg) {
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
			You("seem to be moving faster.");
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
			pline("The sleep ray hits you!");
			nomul(-rn2(50));
			u.usleep = 1;
			nomovemsg = "You wake up.";
		    }
		    break;
		case WAN_SLOW_MONSTER:
		case SPE_SLOW_MONSTER:
		    if(Fast & (TIMEOUT | INTRINSIC)) {
			Fast &= ~(TIMEOUT | INTRINSIC);
			You("seem to be moving slower.");
			exercise(A_DEX, FALSE);
		    }
		    break;
		case WAN_TELEPORTATION:
		case SPE_TELEPORT_AWAY:
		    tele();
		    break;
		case WAN_DEATH:
		case SPE_FINGER_OF_DEATH:
#ifdef POLYSELF
		    if (is_undead(uasmon)) {
			pline((obj->otyp == WAN_DEATH) ?
			  "The wand shoots an apparently harmless beam at you."
			  : "You seem no deader than before.");
			break;
		    }
#endif
		    killer_format = NO_KILLER_PREFIX;
		    killer = self_pronoun("shot %sself with a death ray","him");
		    You("irradiate yourself with pure energy!");
		    You("die.");
		    makeknown(WAN_DEATH);
			/* They might survive with an amulet of life saving */
		    done(DIED);
		    break;
		case SPE_TURN_UNDEAD:
		case WAN_UNDEAD_TURNING:
#ifdef POLYSELF
		    if (is_undead(uasmon)) {
			You("feel frightened and %sstunned.",
			     Stunned ? "even more " : "");
			make_stunned(HStun + rnd(30), FALSE);
		    }
#endif
		    break;
		case SPE_HEALING:
		case SPE_EXTRA_HEALING:
		    healup(obj->otyp == SPE_HEALING ? rnd(8) : d(2,8)+2,
			   0, FALSE, FALSE);
		    You(obj->otyp == SPE_HEALING ? "begin to feel better." :
			"feel a fair bit better.");
		    break;
		case WAN_DIGGING:
		case SPE_DIG:
		case SPE_DETECT_UNSEEN:
		case WAN_NOTHING:
		case WAN_OPENING:
		case WAN_LOCKING:
		case SPE_KNOCK:
		case SPE_WIZARD_LOCK:
		    break;
		case WAN_PROBING:
		    makeknown(WAN_PROBING);
		    ustatusline();
		    break;
		default: impossible("object %d used?",obj->otyp);
	}
	return(damage);
}

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
#ifdef POLYSELF
	    if (u.mtimedone) {
		if ((u.umonnum == PM_CLAY_GOLEM) && !Blind)
		    pline(writing_vanishes, your);
		rehumanize();
	    }
#endif
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

/* called for various wand and spell effects - M. Stephenson */
void
weffects(obj)
register struct	obj	*obj;
{
	xchar zx,zy;

	exercise(A_WIS, TRUE);
	if(objects[obj->otyp].oc_dir == IMMEDIATE) {
	    obj_zapped = FALSE;

	    if(u.uswallow)	(void)bhitm(u.ustuck, obj);
	    else if(u.dz) {
		if(u.dz > 0) {
		    if(levl[u.ux][u.uy].typ == DRAWBRIDGE_DOWN &&
		       (obj->otyp == WAN_LOCKING
			   || obj->otyp == SPE_WIZARD_LOCK))
				close_drawbridge(u.ux, u.uy);
		    else
			(void) bhitpile(obj, bhito, u.ux, u.uy);
		}
	    } else (void) bhit(u.dx,u.dy,rn1(8,6),ZAPPED_WAND,bhitm,bhito,obj);

	    /* give a clue if obj_zapped */
	    if (obj_zapped)
		You("feel shuddering vibrations.");

	} else if(objects[obj->otyp].oc_dir == NODIR) {
		zapnodir(obj);
	} else {
	    switch(obj->otyp) {
		case WAN_DIGGING:
		case SPE_DIG:
			/* Original effect (approximately):
			 * from CORR: dig until we pierce a wall
			 * from ROOM: piece wall and dig until we reach
			 * an ACCESSIBLE place.
			 * Currently: dig for digdepth positions;
			 * also down on request of Lennart Augustsson.
			 */
		    {   register struct rm *room;
			register int digdepth; 
			register boolean shopdoor, shopwall;

			shopdoor = shopwall = FALSE;
			if(u.uswallow) {
				register struct monst *mtmp = u.ustuck;

				if (!is_whirly(mtmp->data)) {
					if (is_animal(mtmp->data))
					    You("pierce %s stomach wall!",
				  	 	    s_suffix(mon_nam(mtmp)));
					mtmp->mhp = 1;	/* almost dead */
					expels(mtmp, mtmp->data,
					       !is_animal(mtmp->data));
				}
				break;
			}
			if(u.dz) {
			    if(!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) &&
			       !Underwater) {
				if(u.dz < 0 || On_stairs(u.ux, u.uy)) {
				    if(On_stairs(u.ux, u.uy))
					pline(
			"The beam bounces off the %s and hits the ceiling.",
					      (u.ux == xdnladder ||
					       u.ux == xupladder) ?
					      "ladder" : "stairs");
				    You("loosen a rock from the ceiling.");
				    pline("It falls on your %s!",
					  body_part(HEAD));
				    losehp(1, "falling rock", KILLED_BY_AN);
				    (void) mksobj_at((int)ROCK, u.ux, u.uy, FALSE);
				    stackobj(fobj);
				    if(Invisible) newsym(u.ux, u.uy);
				} else {
				    (void) dighole(FALSE);
				}
			    }
			    break;
			}
			zx = u.ux+u.dx;
			zy = u.uy+u.dy;
			digdepth = 8 + rn2(18);
			tmp_at(DISP_BEAM, cmap_to_glyph(S_digbeam));
			while(--digdepth >= 0) {
			    if(!isok(zx,zy)) break;
			    room = &levl[zx][zy];
			    tmp_at(zx,zy);
			    delay_output();	/* wait a little bit */
			    if(level.flags.is_maze_lev &&
			                    !Is_earthlevel(&u.uz)) {
				if(IS_WALL(room->typ)) {
				    if(!(room->diggable & W_NONDIGGABLE)) {
					if(*in_rooms(zx,zy,SHOPBASE)) { 
					    add_damage(zx, zy, 200L);
					    shopwall = TRUE;
					}
					room->typ = ROOM;
					unblock_point(zx,zy); /* vision */
				    } else if(!Blind)
					pline("The wall glows then fades.");
				    break;
				}
				if(room->typ == STONE) {
				    if(!(room->diggable & W_NONDIGGABLE)) {
					room->typ = CORR;
					unblock_point(zx,zy); /* vision */
				    }else if (!Blind && !Is_airlevel(&u.uz))
					pline("The rock glows then fades.");
				    break;
				}
			    } else if(IS_ROCK(room->typ)) {
				if(may_dig(zx,zy)) {
				    if(IS_WALL(room->typ) ||
				       room->typ == SDOOR) {
					if(*in_rooms(zx,zy,SHOPBASE)) {
					    add_damage(zx, zy, 200L);
					    shopwall = TRUE;
					}
					if (level.flags.is_cavernous_lev) {
					    room->typ = CORR;
					} else {
					    room->typ = DOOR;
					    room->doormask = D_NODOOR;
					}
					digdepth -= 2;
				    } else {
					room->typ = CORR;
					digdepth--;
				    }
				    unblock_point(zx,zy); /* vision */
				} else
				    break;
			    } else if(closed_door(zx, zy)) {
				if(*in_rooms(zx,zy,SHOPBASE)) {
				    shopdoor = TRUE;
				    add_damage(zx, zy, 400L);
				}
				room->doormask = D_NODOOR;
				unblock_point(zx,zy); /* vision */
				digdepth -= 2;
			    }
			    zx += u.dx;
			    zy += u.dy;
			} /* while */
			tmp_at(DISP_END,0);	/* closing call */
			if(shopdoor || shopwall)
			    pay_for_damage(shopdoor? "destroy" : "dig into");
			break;
		    }
		default:
			if((int) obj->otyp >= SPE_MAGIC_MISSILE &&
				(int) obj->otyp <= SPE_FINGER_OF_DEATH) {

			    buzz((int) obj->otyp - SPE_MAGIC_MISSILE + 10,
				 (int)u.ulevel / 2 + 1, u.ux, u.uy, u.dx, u.dy);

			} else if((int) obj->otyp >= WAN_MAGIC_MISSILE &&
					(int) obj->otyp <= WAN_LIGHTNING) {

			    buzz((int) obj->otyp - WAN_MAGIC_MISSILE,
				(obj->otyp == WAN_MAGIC_MISSILE) ? 2 : 6,
				 u.ux, u.uy, u.dx, u.dy);
			} else
			    impossible("weffects: unexpected spell or wand");
			break;
		}
		if(!objects[obj->otyp].oc_name_known) {
			makeknown(obj->otyp);
			more_experienced(0,10);
		}
	}
	return;
}

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
					/* fns called when mon/obj hit */
int FDECL((*fhitm), (MONST_P, OBJ_P)),
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

		if (weapon == THROWN_WEAPON || weapon == KICKED_WEAPON) {
		    /* can't hit monsters/objects in rock w/solid weapons */
		    /* but beams/zaps _can_, so we need an extra pre-check */
		    if(!ZAP_POS(typ) || closed_door(bhitpos.x, bhitpos.y)) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		    }
		}
		if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
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
		} else if(weapon == KICKED_WEAPON && 
			      ((obj->otyp == GOLD_PIECE && 
			         OBJ_AT(bhitpos.x, bhitpos.y)) ||
			            down_gate(bhitpos.x, bhitpos.y) != -1)) {
			tmp_at(DISP_END, 0);
			return (struct monst *)0;
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
				pline("You skillfully catch the boomerang.");
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

static int
zhit(mon, type, nd)			/* returns damage to mon */
register struct monst *mon;
register int type, nd;
{
	register int tmp = 0;
	register int abstype = abs(type) % 10;

	switch(abstype) {
	case ZT_MAGIC_MISSILE:
		tmp = d(nd,6);
		break;
	case ZT_FIRE:
		if(resists_fire(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		if(resists_cold(mon->data)) tmp += 7;
		break;
	case ZT_COLD:
		if(resists_cold(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		if(resists_fire(mon->data)) tmp += d(nd, 3);
		break;
	case ZT_SLEEP:
		tmp = 0;
		if(resists_sleep(mon->data) ||
		   resist(mon, (type == ZT_WAND(ZT_SLEEP)) ?
			  WAND_CLASS : '\0', 0, NOTELL))
			shieldeff(mon->mx, mon->my);
		else if (mon->mcanmove) {
			int tmp2 = d(nd,25);
			mon->mcanmove = 0;
			if ((unsigned)mon->mfrozen + tmp2 > 127) 
				mon->mfrozen = 127;
			else mon->mfrozen += tmp2;
		}
		break;
	case ZT_DEATH:		/* death/disintegration */
		if(abs(type) != ZT_BREATH(ZT_DEATH)) {	/* death */
		    if(mon->data == &mons[PM_DEATH]) {
			mon->mhpmax += mon->mhpmax/2;
			mon->mhp = mon->mhpmax;
			tmp = 0;
			break;
		    }
		    if(is_undead(mon->data)) {
			shieldeff(mon->mx, mon->my);
			break;
		    }
		    type = -1; /* so they don't get saving throws */
		} else {
		    if (resists_disint(mon->data)) {
			shieldeff(mon->mx, mon->my);
			break;
		    } else {
			tmp = MAGIC_COOKIE;
			break;
		    }
		}
		tmp = mon->mhp+1;
		break;
	case ZT_LIGHTNING:
		if(resists_elec(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		if (haseyes(mon->data)) {
			register unsigned rnd_tmp = rnd(50);
			mon->mcansee = 0;
			if((mon->mblinded + rnd_tmp) > 127)
				mon->mblinded = 127;
			else mon->mblinded += rnd_tmp;
		}
		break;
	case ZT_POISON_GAS:
		if(resists_poison(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		break;
	case ZT_ACID:
		if(resists_acid(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		break;
	}
#ifdef MULDGN
	if(pl_character[0] == 'K' && type >= 10 && type <= 19 &&
	   u.uhave.questart) tmp *= 2;
#endif
	if (type >= 0)
	    if (resist(mon, (type < ZT_SPELL(0)) ? WAND_CLASS : '\0',
		       0, NOTELL)) tmp /= 2;
	mon->mhp -= tmp;
	return(tmp);
}

/*
 * burn scrolls and spell books on floor at position x,y
 * return the number of scrolls and spell books burned
 */
static int
burn_floor_paper(x, y)
int x, y;
{
	register struct obj *obj, *obj2;
	register int cnt = 0;
	register long i, scrquan;

	for(obj = level.objects[x][y]; obj; obj = obj2) {
	    obj2 = obj->nexthere;
	    /* Bug fix - KAA */
	    if(obj->oclass == SCROLL_CLASS
			|| obj->oclass == SPBOOK_CLASS) {
		if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL
			|| obj->otyp == SPE_BOOK_OF_THE_DEAD
		        || obj->oartifact)
		    continue;
		scrquan = obj->quan;
		for(i = 1; i <= scrquan ; i++)
		    if(!rn2(3))  {
			cnt++;
			/* not useupf(), which charges */
			if (obj->quan > 1L) obj->quan--;
			else delobj(obj);
		    }
	    }
	}
	return(cnt);
}

/* type ==   0 to   9 : you shooting a wand */
/* type ==  10 to  19 : you casting a spell */
/* type ==  20 to  29 : you breathing as a monster */
/* type == -10 to -19 : monster casting spell */
/* type == -20 to -29 : monster breathing at you */
/* type == -30 to -39 : monster shooting a wand (MUSE only) */
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
    boolean bodyhit = FALSE, shopdamage = FALSE;
    register const char *fltxt;

#ifdef MUSE
    fltxt = flash_types[(type <= -30) ? abstype : abs(type)];
#else
    fltxt = flash_types[abs(type)];
#endif
    if(u.uswallow) {
	register int tmp;

	if(type < 0) return;
	tmp = zhit(u.ustuck, type, nd);
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

	if (type != ZT_SPELL(ZT_FIRE))
	    /* Fireballs only damage when they explode */
	    range += zap_over_floor(sx, sy, type, &shopdamage);
	if ((mon = m_at(sx, sy)) != 0) {
	    if (type == ZT_SPELL(ZT_FIRE)) break;
	    if (type >= 0) mon->data->mflags3 &= ~M3_WAITMASK;
	    if (rnd(20) < 18 + find_mac(mon)) {
#ifdef MUSE
		if (mon_reflects(mon, "")) {
		    if(cansee(mon->mx,mon->my)) {
			hit(fltxt, mon, exclam(0));
			shieldeff(mon->mx, mon->my);
			(void) mon_reflects(mon, "But it reflects from %s %s!");
		    }
		    dx = -dx;
		    dy = -dy;
		} else
#endif
		{
		    register int tmp = zhit(mon, type, nd);

		    if (is_rider(mon->data) && type == ZT_BREATH(ZT_DEATH)) {
		        if(cansee(mon->mx, mon->my)) {
			    hit(fltxt, mon, exclam(tmp));
			    pline("%s disintegrates.", Monnam(mon));
			    if(Blind)
		       You("hear the fragments of %s body reassembling!",
			         s_suffix(mon_nam(mon)));
			    else
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
			mon->mgold = 0;
			otmp = mon->minvent;
			while(otmp) {
#ifdef MULDGN
			    if (is_quest_artifact(otmp))
				otmp = otmp->nobj;
			    else {
#endif
				otmp2 = otmp;
				if (otmp == mon->minvent)
				    mon->minvent = otmp->nobj;
				otmp = otmp->nobj;
				obfree(otmp2, (struct obj *)0);
#ifdef MULDGN
			    }
#endif
			}
			if (type < 0)
			    monkilled(mon, (char *)0, AD_RBRE);
			else
			    xkilled(mon, 2);
		    } else if(mon->mhp < 1) {
			if(type < 0)
			    monkilled(mon, fltxt, AD_RBRE);
			else
			    killed(mon);
		    } else
			hit(fltxt, mon, exclam(tmp));
		}
		range -= 2;
	    } else
		miss(fltxt,mon);
	} else if(sx == u.ux && sy == u.uy) {
	    nomul(0);
	    if(rnd(20) < 18+u.uac) {
		register int dam = 0;
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
		}
		else switch(abstype) {
		case ZT_MAGIC_MISSILE:
		    if(Antimagic) {
			shieldeff(sx, sy);
			pline("The missiles bounce off!");
		    } else {
		        dam = d(nd,6);
			exercise(A_STR, FALSE);
		    }
		    break;
		case ZT_FIRE:
		    if(Fire_resistance) {
			shieldeff(sx, sy);
			You("don't feel hot!");
#ifdef POLYSELF
			ugolemeffects(AD_FIRE, d(nd, 6));
#endif
		    } else dam = d(nd, 6);
		    while (1) {
			switch(rn2(5)) {
			case 0:
			    if (!rust_dmg(uarmh, "leather helmet", 0, FALSE))
				continue;
			    break;
			case 1:
			    bodyhit = TRUE;
			    if (uarmc) break;
			    if (uarm)
				(void)(rust_dmg(uarm, xname(uarm), 0, FALSE));
			    break;
			case 2:
			    if (!rust_dmg(uarms, "wooden shield", 0, FALSE))
				continue;
			    break;
			case 3:
			    if (!rust_dmg(uarmg, "gloves", 0, FALSE)) continue;
			    break;
			case 4:
			    if (!rust_dmg(uarmf, "boots", 0, FALSE)) continue;
			    break;
			}
			break; /* Out of while loop */
		    }
		    if(!rn2(3) && bodyhit)
			destroy_item(POTION_CLASS, AD_FIRE);
		    if(!rn2(3) && bodyhit)
			destroy_item(SCROLL_CLASS, AD_FIRE);
		    if(!rn2(5) && bodyhit)
			destroy_item(SPBOOK_CLASS, AD_FIRE);
		    break;
		case ZT_COLD:
		    if(Cold_resistance) {
			shieldeff(sx, sy);
			You("don't feel cold.");
#ifdef POLYSELF
			ugolemeffects(AD_COLD, d(nd, 6));
#endif
		    } else
			dam = d(nd, 6);
		    if(!rn2(3))
			destroy_item(POTION_CLASS, AD_COLD);
		    break;
		case ZT_SLEEP:
		    if(Sleep_resistance) {
			shieldeff(u.ux, u.uy);
			You("don't feel sleepy.");
		    } else {
			/* have to do this _before_ we reset multi */
			stop_occupation();
			nomul(-d(nd,25)); /* sleep ray */
			u.usleep = 1;
			nomovemsg = "You wake up.";
		    }
		    break;
		case ZT_DEATH:
		    if(abs(type) == ZT_BREATH(ZT_DEATH)) {
			if (Disint_resistance) {
			    You("are not disintegrated.");
			    break;
			} else if(uarms) {
			    (void) destroy_arm(uarms);
			    break;
			} else if (uarm)  {
			    (void) destroy_arm(uarm);
			    break;
			}
		    }
#ifdef POLYSELF
		    else if(is_undead(uasmon)) {
			shieldeff(sx, sy);
			You("seem unaffected.");
			break;
		    }
#endif
		    else if(Antimagic) {
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
#ifdef POLYSELF
			ugolemeffects(AD_ELEC, d(nd, 6));
#endif
		    } else {
			dam = d(nd, 6);
			exercise(A_CON, FALSE);
		    }
		    if(!rn2(3))
			destroy_item(WAND_CLASS, AD_ELEC);
		    if(!rn2(3))
			destroy_item(RING_CLASS, AD_ELEC);
		    break;
		case ZT_POISON_GAS:
		    poisoned("blast", A_DEX, "poisoned blast", 15);
		    break;
		case ZT_ACID:
#ifdef POLYSELF
		    if (resists_acid(uasmon))
			dam = 0;
		    else
#endif
			{
			    pline("The acid burns!");
			    dam = d(nd,6);
			    exercise(A_STR, FALSE);
			}
		    if(!rn2(6)) erode_weapon(TRUE);
		    if(!rn2(6)) erode_armor(TRUE);
		    break;
		}
		if(Half_spell_damage && dam &&
		   type < 0 && (type > -20 || type < -29)) /* !Breath */
		    dam = (dam+1) / 2;
		losehp(dam, fltxt, KILLED_BY_AN);
	    } else pline("%s whizzes by you!", The(fltxt));
	    if (abstype == ZT_LIGHTNING && !Blind) {
		You("are blinded by the flash!");
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
}

void
melt_ice(x, y)
xchar x, y;
{
	struct rm *lev = &levl[x][y];

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
	unearth_objs(x, y);
	newsym(x,y);
	if (cansee(x,y)) Norep("The ice crackles and melts.");
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
			pline("The water freezes for a moment.");
		    else
			You("hear a soft crackling.");
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
			You("hear a crackling sound.");
		    if(x == u.ux && y == u.uy &&
			       u.utrap && u.utraptype == TT_LAVA) {
#ifdef POLYSELF
			if (passes_walls(uasmon))
			    You("pass through the now-solid rock.");
			else {
#endif
			    u.utrap = rn1(50,20);
			    u.utraptype = TT_INFLOOR;
			    You("are firmly stuck in the cooling rock.");
#ifdef POLYSELF
			}
#endif
		    }
		}
	}
	if(closed_door(x, y)) {
		rangemod = -1000;
		switch(abstype) {
		case ZT_FIRE:
		    if(type >= 0 && *in_rooms(x, y, SHOPBASE)) {
			add_damage(x, y, 400L);
			*shopdamage = TRUE;
		    }
		    lev->doormask = D_NODOOR;
		    unblock_point(x,y);	/* vision */
		    if(cansee(x,y)) {
			pline("The door is consumed in flames!");
			newsym(x,y);
		    }
		    else You("smell smoke.");
		    break;
		case ZT_COLD:
		    if(type >= 0 && *in_rooms(x, y, SHOPBASE)) {
			add_damage(x, y, 400L);
			*shopdamage = TRUE;
		    }
		    lev->doormask = D_NODOOR;
		    unblock_point(x,y);	/* vision */
		    if(cansee(x,y)) {
			pline("The door freezes and shatters!");
			newsym(x,y);
		    }
		    else You("feel cold.");
		    break;
		case ZT_DEATH:
		    /* death spells/wands don't disintegrate */
		    if(abs(type) != ZT_BREATH(ZT_DEATH))
			goto def_case;
		    if(type >= 0 && *in_rooms(x, y, SHOPBASE)) {
			add_damage(x, y, 400L);
			*shopdamage = TRUE;
		    }
		    lev->doormask = D_NODOOR;
		    unblock_point(x,y);	/* vision */
		    if(cansee(x,y)) {
			pline("The door disintegrates!");
			newsym(x,y);
		    }
		    else if(flags.soundok)
			You("hear crashing wood.");
		    break;
		case ZT_LIGHTNING:
		    if(type >= 0 && *in_rooms(x, y, SHOPBASE)) {
			add_damage(x, y, 400L);
			*shopdamage = TRUE;
		    }
		    lev->doormask = D_BROKEN;
		    unblock_point(x,y);	/* vision */
		    if(cansee(x,y)) {
			pline("The door splinters!");
			newsym(x,y);
		    }
		    else if(flags.soundok)
			You("hear crackling.");
		    break;
		default:
		def_case:
		    if(cansee(x,y)) {
			pline("The door absorbs %s %s!",
			      (type < 0) ? "the" : "your",
			      abs(type) < ZT_SPELL(0) ? "bolt" :
			      abs(type) < ZT_BREATH(0) ? "spell" :
			      "blast");
		    } else You("feel vibrations.");
		    break;
		}
	}
	if(OBJ_AT(x, y) && abstype == ZT_FIRE)
		if(burn_floor_paper(x,y) && cansee(x,y))  {
		    newsym(x,y);
		    if(!Blind)
			You("see a puff of smoke.");
		    else
			You("smell a whiff of smoke.");
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

void
rloco(obj)
register struct obj *obj;
{
	register xchar tx, ty, otx, oty;

	otx = obj->ox;
	oty = obj->oy;
	do {
		tx = rn1(COLNO-3,2);
		ty = rn2(ROWNO);
	} while(!goodpos(tx,ty,(struct monst *)0, (struct permonst *)0));
	freeobj(obj);
	if (flooreffects(obj, tx, ty, "fall"))
		return;
	if (otx == 0 && oty == 0) {
	    ;	/* fell through a trapdoor; no update of old loc needed */
	} else {
	    if (costly_spot(otx, oty)
	      && (!costly_spot(tx, ty) ||
		  !index(in_rooms(tx, ty, 0), *in_rooms(otx, oty, 0)))) {
		if(costly_spot(u.ux, u.uy) &&
			    index(u.urooms, *in_rooms(otx, oty, 0)))
		    addtobill(obj, FALSE, FALSE, FALSE);
		else (void)stolen_value(obj, otx, oty, FALSE, FALSE);
	    }
	    newsym(otx, oty);	/* update old location */
	}
	obj->nobj = fobj;
	fobj = obj;
	place_object(obj, tx, ty);
	newsym(tx,ty);
}

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
	struct obj *item, *nitem;

	if((trap = t_at(obj->ox,obj->oy)) && trap->ttyp == STATUE_TRAP)
	    if(makemon(&mons[obj->corpsenm], obj->ox, obj->oy)) {
		pline("Instead of shattering, the statue suddenly comes alive!");
		delobj(obj);
		deltrap(trap);
		return FALSE;
	    }
	for(item = obj->cobj; item; item = nitem) {
	    nitem = item->nobj;
	    item->nobj = fobj;
	    fobj = item;
	    place_object(item, obj->ox, obj->oy);
	}
	obj->cobj = (struct obj *)0;
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
		    if(osym == POTION_CLASS) {
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
				The(xname(obj)),
				Hallucination ? hcolor() : "dark red");
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
		for(i = 0; i < cnt; i++) {
		    if (obj->owornmask) {
			if (obj->owornmask & W_RING) /* ring being worn */
			    Ring_gone(obj);
			else
			    setnotworn(obj);
		    }
		    useup(obj);
		}
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
#ifdef GCC_WARN
	    quan = 0L;
	    dindx = 0;
#endif
	    switch(dmgtyp) {
		case AD_COLD:
		    if(osym == POTION_CLASS) {
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
				Hallucination ? hcolor() : "dark red");
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

/*ARGSUSED*/
int
resist(mtmp, class, damage, tell)
register struct monst	*mtmp;
register char	class;
register int	damage, tell;
{
	register int	resisted;
	register int	lev;

	switch(class)  {

	    case WAND_CLASS:
			lev = 8;
			break;

	    case SCROLL_CLASS:
			lev = 6;
			break;

	    case POTION_CLASS:
			lev = 5;
			break;

	    default:	lev = u.ulevel;
			break;
	}

	resisted = (rn2(100) - (unsigned)mtmp->m_lev + lev) < mtmp->data->mr;
	if(resisted) {

		if(tell) {
		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s resists!", Monnam(mtmp));
		}
		mtmp->mhp -= damage/2;
	} else  mtmp->mhp -= damage;

#ifdef MUSE
	if(mtmp->mhp < 1) {
		if(m_using) monkilled(mtmp, "", AD_RBRE);
		else killed(mtmp);
	}
#else
	if(mtmp->mhp < 1) killed(mtmp);
#endif
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

/*zap.c*/
