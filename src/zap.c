/*	SCCS Id: @(#)zap.c	3.0	88/10/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#if defined(ALTARS) && defined(THEOLOGY)
static boolean priesthit = FALSE;
#endif

const char *fl[]= {
	"magic missile",	/* Wands must be 0-9 */
	"bolt of fire",
	"sleep ray",
	"bolt of cold",
	"death ray",
	"bolt of lightning",
	"",
	"",
	"",
	"",

	"magic missile",	/* Spell equivalents must be 10-19 */
	"fireball",
	"sleep ray",
	"cone of cold",
	"finger of death",
	"bolt of lightning",
	"",
	"",
	"",
	"",

	"blast of missiles",	/* Dragon breath equivalents 20-29*/
	"blast of fire",
	"blast of sleep gas",
	"blast of frost",
	"blast of disintegration",
	"blast of lightning",
	"blast of poison gas",
	"blast of acid",
	"",
	""
};

#ifdef TEXTCOLOR
static const int zapcolor[10] = {
	AT_ZAP, RED, AT_ZAP, WHITE, AT_ZAP, WHITE,
	AT_ZAP, AT_ZAP, AT_ZAP, AT_ZAP
};
#endif

/* Routines for IMMEDIATE wands and spells. */
/* bhitm: monster mtmp was hit by the effect of wand or spell otmp */
static int
bhitm(mtmp, otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
	wakeup(mtmp);
	switch(otmp->otyp) {
	case WAN_STRIKING:
#ifdef SPELLS
	case SPE_FORCE_BOLT:
#endif
		if(u.uswallow || rnd(20) < 10+mtmp->data->ac) {
			register int tmp = d(2,12);
			hit((otmp->otyp == WAN_STRIKING) ? "wand" : "spell", mtmp, exclam(tmp));
			(void) resist(mtmp, otmp->olet, tmp, TELL);
		} else miss((otmp->otyp == WAN_STRIKING) ? "wand" : "spell", mtmp);
		break;
	case WAN_SLOW_MONSTER:
#ifdef SPELLS
	case SPE_SLOW_MONSTER:
#endif
		if(! resist(mtmp, otmp->olet, 0, NOTELL))
			if (mtmp->mspeed == MFAST) mtmp->mspeed = 0;
			else mtmp->mspeed = MSLOW;
		break;
	case WAN_SPEED_MONSTER:
		if (!resist(mtmp, otmp->olet, 0, NOTELL))
			if (mtmp->mspeed == MSLOW) mtmp->mspeed = 0;
			else mtmp->mspeed = MFAST;
		break;
	case WAN_UNDEAD_TURNING:
#ifdef SPELLS
	case SPE_TURN_UNDEAD:
#endif
		if(is_undead(mtmp->data)) {

			if(!resist(mtmp, otmp->olet, rnd(8), NOTELL))
				mtmp->mflee = 1;
		}
		break;
	case WAN_POLYMORPH:
#ifdef SPELLS
	case SPE_POLYMORPH:
#endif
		if(!resist(mtmp, otmp->olet, 0, NOTELL))
		    if( newcham(mtmp, (struct permonst *)0) )
			if (!Hallucination)
			    makeknown(otmp->otyp);
		break;
	case WAN_CANCELLATION:
#ifdef SPELLS
	case SPE_CANCELLATION:
#endif
		if(!resist(mtmp, otmp->olet, 0, NOTELL)) {
		  if (is_were(mtmp->data) && mtmp->data->mlet != S_HUMAN)
		    were_change(mtmp);
#ifdef GOLEMS
		  if (mtmp->data == &mons[PM_CLAY_GOLEM]) {
		    if (!Blind)
		      pline("Some writing vanishes from %s's head!",
			mon_nam(mtmp));
		    killed(mtmp);
		  }
		  else
#endif /* GOLEMS */
		    mtmp->mcan = 1;
		}
		break;
	case WAN_TELEPORTATION:
#ifdef SPELLS
	case SPE_TELEPORT_AWAY:
#endif
#if defined(ALTARS) && defined(THEOLOGY)
		if(mtmp->ispriest && in_temple(mtmp->mx, mtmp->my)) {
		    kludge("%s resists your magic!", Monnam(mtmp));
		    wakeup(mtmp);
		    break;
		}
#endif
		rloc(mtmp);
		break;
	case WAN_MAKE_INVISIBLE:
		mtmp->minvis = 1;
		break;
	case WAN_NOTHING:
		break;
#ifdef PROBING
	case WAN_PROBING:
		makeknown(otmp->otyp);
		mstatusline(mtmp);
		break;
#endif
	case WAN_OPENING:
	case WAN_LOCKING:
#ifdef SPELLS
	case SPE_KNOCK:
	case SPE_WIZARD_LOCK:
#endif
		break;
	default:
		impossible("What an interesting effect (%u)", otmp->otyp);
	}
	return 0;
}

struct monst *
revive(obj,ininv)
register struct obj *obj;
boolean ininv;
{
	register struct monst *mtmp;

	if(obj->otyp == CORPSE) {
		int montype,x,y,nl;
		char buf[BUFSZ];

		if (nl = obj->onamelth) Strcpy(buf, ONAME(obj));
		montype = obj->corpsenm;
		if (ininv) {
			x = u.ux; y = u.uy;
			useup(obj);
		} else {
			x = obj->ox; y = obj->oy;
			useupf(obj);
		}
		if (cant_create(&montype)) { /* will make zombie instead */
			mtmp = makemon(&mons[PM_HUMAN_ZOMBIE], x, y);
			if (mtmp) {
				mtmp->mhp = mtmp->mhpmax = 100;
				mtmp->mspeed = MFAST;
			}
		} else {
#ifdef ARMY
			if (is_mercenary(&mons[montype]))
				montype = PM_UNARMORED_SOLDIER;
#endif
			mons[montype].pxlth += nl;
			mtmp = mkmon_at(mons[montype].mname, x, y);
			mons[montype].pxlth -= nl;
			if (mtmp) {
				/* Monster retains its name */
				mtmp->mnamelth = nl;
				if (nl) Strcpy(NAME(mtmp), buf);
				/* No inventory for newly revived monsters */
				while(obj = (mtmp->minvent)) {
					mtmp->minvent = obj->nobj;
					free((genericptr_t)obj);
				}
			}
		}
	}
	return mtmp;
}

static const char charged_objs[] = { WAND_SYM, WEAPON_SYM, ARMOR_SYM, 0 };

static void
cancel_item(obj)
register struct obj *obj;
{
	if(obj->spe &&
	  !(obj->otyp == AMULET_OF_YENDOR ||
	    obj->otyp == WAN_CANCELLATION || /* can't cancel cancellation */
	    obj->otyp == TIN || obj->otyp == EGG ||
	    obj->otyp == STATUE ||
#ifdef MAIL
	    obj->otyp == SCR_MAIL ||
#endif
	    obj->otyp == KEY || obj->otyp == SKELETON_KEY ||
	    obj->otyp == LARGE_BOX || obj->otyp == CHEST))
		obj->spe = (obj->olet == WAND_SYM) ? -1 : 0;
	if (obj->olet == SCROLL_SYM
#ifdef MAIL
	    && obj->otyp != SCR_MAIL
#endif
	   )
	    obj->otyp = SCR_BLANK_PAPER;
	if (obj->olet == POTION_SYM && obj->otyp > POT_BOOZE)
	    obj->otyp = (obj->otyp==POT_SICKNESS) ? POT_FRUIT_JUICE : POT_WATER;
	    /* sickness is "biologically contaminated" fruit juice; cancel it
	     * and it just becomes fruit juice...
	     */
	obj->blessed = obj->cursed = FALSE;
}

static int
bhito(obj, otmp)	/* object obj was hit by the effect of wand otmp */
register struct obj *obj, *otmp;	/* returns TRUE if sth was done */
{
	register int res = 1;
	struct obj *otmp2;

	if(obj == uball || obj == uchain)
		res = 0;
	else
	switch(otmp->otyp) {
	case WAN_POLYMORPH:
#ifdef SPELLS
	case SPE_POLYMORPH:
#endif
		/* preserve symbol and quantity */
		otmp2 = mkobj_at(obj->olet, obj->ox, obj->oy);
		otmp2->quan = obj->quan;
#ifdef MAIL
		/* You can't send yourself 100 mail messages and then
		 * polymorph them into useful scrolls
		 */
		if (obj->otyp == SCR_MAIL) {
			otmp2->otyp = SCR_MAIL;
			obj->spe = 1;
		}
#endif
		/* keep special fields (including charges on wands) */
		if (index(charged_objs, otmp2->olet)) otmp2->spe = obj->spe;

		/* Amulet gets cheap   stewr 870807 */
		if (obj->otyp == AMULET_OF_YENDOR) otmp2->spe = -1;

		/* Wands of wishing max 3 stewr 870808 */
		if ((otmp2->otyp == WAN_WISHING)
		    && (obj->spe > 3)) otmp2->spe = 3;

		otmp2->cursed = obj->cursed;
		otmp2->blessed = obj->blessed;
		otmp2->rustfree = obj->rustfree;

		/* Keep chest/box traps and poisoned ammo if we may */
		if (obj->otrapped && Is_box(otmp2))
			otmp2->otrapped = 1;
		if (obj->opoisoned && 
		    (otmp2->olet == WEAPON_SYM && otmp2->otyp <= SHURIKEN))
			otmp2->opoisoned = 1;

		/* Turn dragon corpses into dragon armors */
		if (obj->otyp == CORPSE && obj->corpsenm >= PM_GREY_DRAGON
				&& obj->corpsenm <= PM_YELLOW_DRAGON) {
			if (!rn2(10)) { /* Random failure */
				otmp2->otyp = TIN;
				otmp2->known = otmp2->dknown = 1;
			} else {
				otmp2->otyp = DRAGON_SCALE_MAIL;
				otmp2->olet = ARMOR_SYM;
				otmp2->spe = 0;
				otmp2->rustfree = 1;
				otmp2->quan = 1;
				otmp2->cursed = 0;
			}
			otmp2->corpsenm = obj->corpsenm;
		}

		/* update the weight */
		otmp2->owt = weight(otmp2);
		delobj(obj);
		break;
	case WAN_STRIKING:
#ifdef SPELLS
	case SPE_FORCE_BOLT:
#endif
		if(obj->otyp == BOULDER)
			fracture_rock(obj);
		else if(obj->otyp == STATUE)
			(void) break_statue(obj);
		else
			res = 0;
		break;
	case WAN_CANCELLATION:
#ifdef SPELLS
	case SPE_CANCELLATION:
#endif
		cancel_item(obj);
		break;
	case WAN_TELEPORTATION:
#ifdef SPELLS
	case SPE_TELEPORT_AWAY:
#endif
		rloco(obj);
		break;
	case WAN_MAKE_INVISIBLE:
		obj->oinvis = 1;
		break;
	case WAN_UNDEAD_TURNING:
#ifdef SPELLS
	case SPE_TURN_UNDEAD:
#endif
		res = !!revive(obj,FALSE);
		break;
	case WAN_OPENING:
	case WAN_LOCKING:
#ifdef SPELLS
	case SPE_KNOCK:
	case SPE_WIZARD_LOCK:
#endif
		if(Is_box(obj))
			res = boxlock(obj, otmp);
		else
			res = 0;
		if (res /* && obj->olet == WAND_SYM */)
			makeknown(obj->otyp);
		break;
	case WAN_SLOW_MONSTER:		/* no effect on objects */
#ifdef SPELLS
	case SPE_SLOW_MONSTER:
#endif
	case WAN_SPEED_MONSTER:
	case WAN_NOTHING:
#ifdef PROBING
	case WAN_PROBING:
#endif
		res = 0;
		break;
	default:
		impossible("What an interesting effect (%u)", otmp->otyp);
	}
	return(res);
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
		You("wrest one more spell from the worn-out wand.");
	wand->spe--;
	return 1;
}

/*
 * zapnodir - zaps an NODIR wand.
 * added by GAN 11/03/86
 */
void
zapnodir(wand)
register struct obj *wand;
{
	switch(wand->otyp){
		case WAN_LIGHT:
			litroom(TRUE);
			break;
		case WAN_SECRET_DOOR_DETECTION:
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

			if(u.uluck + rn2(5) < 0) {
				pline("Unfortunately, nothing happens.");
				break;
			}
			makewish();
			break;
	}
	if(!objects[wand->otyp].oc_name_known) {
			makeknown(wand->otyp);
			more_experienced(0,10);
	}
}

static void
backfire(otmp)

	register struct obj * otmp;
{
	pline("The %s suddenly explodes!", xname(otmp));
	losehp(d(otmp->spe+2,6), "exploding wand");
	useup(otmp);
}

static const char zap_syms[] = { WAND_SYM, 0 };

int
dozap()
{
	register struct obj *obj;
	int	damage;

	obj = getobj(zap_syms, "zap");
	if(!obj) return(0);

	check_unpaid(obj);

	/* zappable addition done by GAN 11/03/86 */
	if(!zappable(obj)) pline(nothing_happens);
	else if(obj->cursed && !rn2(100)) {
		backfire(obj);	/* the wand blows up in your face! */
		return(1);
	} else if(!(objects[obj->otyp].bits & NODIR) && !getdir(1)) {
		if (!Blind)
		    pline("The %s glows and fades.", xname(obj));
		/* make him pay for knowing !NODIR */
	} else if(!u.dx && !u.dy && !u.dz && !(objects[obj->otyp].bits & NODIR)) {
	    if((damage = zapyourself(obj)))
		losehp(damage,"self-inflicted injury");
	}
	else {
		weffects(obj);
#if defined(ALTARS) && defined(THEOLOGY)
		if(priesthit) ghod_hitsu();
		priesthit = FALSE;
#endif
	}
	if (obj->spe < 0) {
	    pline ("The %s %sturns to dust.",
		   xname(obj), Blind ? "" : "glows violently, then ");
	    useup(obj);
	}
	return(1);
}

int
zapyourself(obj)
	register struct obj	*obj;
{
	struct obj	*otmp;
	int	damage = 0;

	switch(obj->otyp) {
		case WAN_STRIKING:
#ifdef SPELLS
		case SPE_FORCE_BOLT:
#endif
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline("Boing!");
		    } else {
			You("magically bash yourself!");
			damage=d(8,6);
		    }
		    break;
		case WAN_LIGHTNING:
		    makeknown(WAN_LIGHTNING);
		    if (!Shock_resistance) {
			pline("Idiot!  You've shocked yourself!");
			damage = d(12,6);
		    } else {
			shieldeff(u.ux, u.uy);
			You("zap yourself, but seem unharmed.");
#ifdef POLYSELF
#ifdef GOLEMS
			ugolemeffects(AD_ELEC, d(12,6));
#endif /* GOLEMS */
#endif
		    }
		    if(!Blind) {
			    You("are blinded by the flash!");
			    make_blinded((long)rnd(100),FALSE);
		    }
		    break;
		case WAN_FIRE:
		    makeknown(WAN_FIRE);
#ifdef SPELLS
		case SPE_FIREBALL:
#endif
#ifdef MUSIC
		case FIRE_HORN:
#endif
		    pline("You've set light to yourself!");
		    if (Fire_resistance) {
			shieldeff(u.ux, u.uy);
			You("feel mildly hot.");
#ifdef POLYSELF
#ifdef GOLEMS
			ugolemeffects(AD_FIRE, d(12,6));
#endif /* GOLEMS */
#endif
		    } else
			damage = d(12,6);
		    destroy_item(SCROLL_SYM, AD_FIRE);
		    destroy_item(POTION_SYM, AD_FIRE);
#ifdef SPELLS
		    destroy_item(SPBOOK_SYM, AD_FIRE);
#endif
		    break;
		case WAN_COLD:
		    makeknown(WAN_COLD);
#ifdef SPELLS
		case SPE_CONE_OF_COLD:
#endif
#ifdef MUSIC
		case FROST_HORN:
#endif
		    if (Cold_resistance) {
			shieldeff(u.ux, u.uy);
			You("feel mildly chilly.");
#ifdef POLYSELF
#ifdef GOLEMS
			ugolemeffects(AD_COLD, d(12,6));
#endif /* GOLEMS */
#endif
		    } else {
			You("imitate a popsicle!");
			damage = d(12,6);
		    }
		    destroy_item(POTION_SYM, AD_COLD);
		    break;
		case WAN_MAGIC_MISSILE:
		    makeknown(WAN_MAGIC_MISSILE);
#ifdef SPELLS
		case SPE_MAGIC_MISSILE:
#endif
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
#ifdef SPELLS
		case SPE_POLYMORPH:
#endif
#ifdef POLYSELF
		    polyself();
#else
		    newman();
#endif
		    break;
		case WAN_CANCELLATION:
#ifdef SPELLS
		case SPE_CANCELLATION:
#endif
#ifdef POLYSELF
#ifdef GOLEMS
		    if (u.umonnum == PM_CLAY_GOLEM) {
			if (!Blind)
			    pline("Some writing vanishes from your head!");
			rehumanize();
			break;
		    }
#endif /* GOLEMS */
#endif
		    for(otmp = invent; otmp; otmp = otmp->nobj)
			cancel_item(otmp);
#ifdef POLYSELF
		    if(u.mtimedone) rehumanize();
#endif
		    flags.botl = 1;  /* because of potential AC change */
		    find_ac();
		    break;
	       case WAN_MAKE_INVISIBLE:
		    if (!Invisible) makeknown(WAN_MAKE_INVISIBLE);
		    HInvis |= INTRINSIC;
		    if (!See_invisible) newsym(u.ux, u.uy);
		    break;
	       case WAN_SPEED_MONSTER:
		    if (!(Fast & INTRINSIC)) {
			Fast |= INTRINSIC;
			You("seem to be moving faster.");
			makeknown(WAN_SPEED_MONSTER);
		    }
		    break;
	       case WAN_SLEEP:
		    makeknown(WAN_SLEEP);
#ifdef SPELLS
		case SPE_SLEEP:
#endif
		    if(Sleep_resistance) {
			shieldeff(u.ux, u.uy);
			You("don't feel sleepy!");
		    } else {
			pline("The sleep ray hits you!");
			nomul(-rn2(50));
		    }
		    break;
		case WAN_SLOW_MONSTER:
#ifdef SPELLS
		case SPE_SLOW_MONSTER:
#endif
		    if(Fast & (TIMEOUT | INTRINSIC)) {
			Fast &= ~(TIMEOUT | INTRINSIC);
			You("seem to be moving slower.");
		    }
		    break;
		case WAN_TELEPORTATION:
#ifdef SPELLS
		case SPE_TELEPORT_AWAY:
#endif
		    tele();
		    break;
		case WAN_DEATH:
#ifdef SPELLS
		case SPE_FINGER_OF_DEATH:
#endif
#ifdef POLYSELF
		    if (is_undead(uasmon)) {
			pline((obj->otyp == WAN_DEATH) ?
			  "The wand shoots an apparently harmless beam at you."
			  : "You seem no deader than before.");
			break;
		    }
#endif
		    killer = "death ray";
		    You("irradiate yourself with pure energy!");
		    You("die.");
		    makeknown(WAN_DEATH);
			/* They might survive with an amulet of life saving */
		    done(DIED);
		    break;
#ifdef SPELLS
		case SPE_TURN_UNDEAD:
#endif
		case WAN_UNDEAD_TURNING:
#ifdef POLYSELF
		    if (is_undead(uasmon)) {
			Printf("You feel frightened and %sstunned.",
			     Stunned ? "even more " : "");
			make_stunned(HStun + rnd(30), FALSE);
		    }
#endif
		    break;
#ifdef SPELLS
		case SPE_DIG:
		case SPE_DETECT_UNSEEN:
#endif
		case WAN_DIGGING:
		case WAN_NOTHING:
		case WAN_OPENING:
		case WAN_LOCKING:
#ifdef SPELLS
		case SPE_KNOCK:
		case SPE_WIZARD_LOCK:
#endif
		    break;
#ifdef PROBING
		case WAN_PROBING:
		    makeknown(WAN_PROBING);
		    ustatusline();
		    break;
#endif
		default: impossible("object %d used?",obj->otyp);
	}
	return(damage);
}

/* called for various wand and spell effects - M. Stephenson */
void
weffects(obj)
register struct	obj	*obj;
{
	xchar zx,zy;

	if(objects[obj->otyp].bits & IMMEDIATE) {
	    if(u.uswallow)	(void)bhitm(u.ustuck, obj);
	    else if(u.dz) {
		if(u.dz > 0) {
#ifdef STRONGHOLD
		    if(levl[u.ux][u.uy].typ == DRAWBRIDGE_DOWN &&
		       (obj->otyp == WAN_LOCKING
# ifdef SPELLS
			|| obj->otyp == SPE_WIZARD_LOCK
# endif
			))
				(void)close_drawbridge(u.ux, u.uy);
		    else
#endif
		    if (OBJ_AT(u.ux, u.uy)) {
			register struct obj *otmp,*otmp2;

			/* changed by GAN to hit all objects there */
			for(otmp = fobj; otmp ; otmp = otmp2) {
				otmp2 = otmp->nobj;
				/* save pointer as bhito may destroy otmp */
				if(otmp->ox == u.ux && otmp->oy == u.uy)
			    		(void) bhito(otmp, obj);
			}
		    }
		}
	    } else (void) bhit(u.dx,u.dy,rn1(8,6),0,bhitm,bhito,obj);
	} else {
	    switch(obj->otyp){
		case WAN_LIGHT:
#ifdef SPELLS
		case SPE_LIGHT:
#endif
			litroom(TRUE);
			break;
		case WAN_SECRET_DOOR_DETECTION:
#ifdef SPELLS
		case SPE_DETECT_UNSEEN:
#endif
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
			if(u.uluck + rn2(5) < 0) {
			    pline("Unfortunately, nothing happens.");
			    break;
			}
			makewish();
			break;
		case WAN_DIGGING:
#ifdef SPELLS
		case SPE_DIG:
#endif
			/* Original effect (approximately):
			 * from CORR: dig until we pierce a wall
			 * from ROOM: piece wall and dig until we reach
			 * an ACCESSIBLE place.
			 * Currently: dig for digdepth positions;
			 * also down on request of Lennart Augustsson.
			 */
			{ register struct rm *room;
			  register int digdepth,dlx,dly;
			  register boolean shopdoor = FALSE;
			if(u.uswallow) {
				register struct monst *mtmp = u.ustuck;

				You("pierce %s's stomach wall!", mon_nam(mtmp));
				mtmp->mhp = 1;	/* almost dead */
				regurgitates(mtmp);
				break;
			}
			if(u.dz) {
			    if(u.dz < 0) {
				You("loosen a rock from the ceiling.");
				pline("It falls on your %s!",
					body_part(HEAD));
				losehp(1, "falling rock");
				(void) mksobj_at((int)ROCK, u.ux, u.uy);
				fobj->quan = 1;
				stackobj(fobj);
				if(Invisible) newsym(u.ux, u.uy);
			    } else {
				dighole();
			    }
			    break;
			}
			zx = u.ux+u.dx;
			zy = u.uy+u.dy;
			digdepth = 8 + rn2(18);
			Tmp_at2(-1, '*');	/* open call */
			while(--digdepth >= 0) {
			    if(!isok(zx,zy)) break;
			    room = &levl[zx][zy];
			    Tmp_at2(zx,zy);
			    if(is_maze_lev) {
				if(IS_WALL(room->typ)) {
				    if(room->diggable == W_DIGGABLE)
					room->typ = ROOM;
				    else if(!Blind)
					pline("The wall glows then fades.");
				    break;
				}
				if(room->typ == STONE) {
				    if(room->diggable == W_DIGGABLE)
					room->typ = CORR;
				    else if (!Blind)
					pline("The rock glows then fades.");
				    break;
				}
			    } else
				if(IS_ROCK(room->typ))
				    if(may_dig(zx,zy))
					if(IS_WALL(room->typ) ||
						room->typ == SDOOR) {
					    room->typ = DOOR;
					    room->doormask = D_NODOOR;
					    if(in_shop(zx,zy)) {
						shopdoor = TRUE;
						dlx = zx;
						dly = zy;
					    }
					    digdepth -= 2;
					} else {
					    room->typ = CORR;
					    digdepth--;
					}
				    else
					break;
				else if(room->typ == DOOR &&
					(room->doormask & (D_LOCKED | D_CLOSED))) {
				    room->doormask = D_NODOOR;
				    if(in_shop(zx,zy)) {
					shopdoor = TRUE;
					dlx = zx;
					dly = zy;
				    }
				    digdepth -= 2;
				}
			    mnewsym(zx,zy);
			    zx += u.dx;
			    zy += u.dy;
			}
			mnewsym(zx,zy);	/* not always necessary */
			Tmp_at2(-1,-1);	/* closing call */
			if(shopdoor && !in_shop(u.ux, u.uy))
				pay_for_door(dlx, dly, "destroy");
			break;
			}
		default:
#ifdef SPELLS
			if((int) obj->otyp >= SPE_MAGIC_MISSILE) {

			    buzz((int) obj->otyp - SPE_MAGIC_MISSILE + 10,
				 (int)u.ulevel / 2 + 1, u.ux, u.uy, u.dx, u.dy);
			} else
#endif

			    buzz((int) obj->otyp - WAN_MAGIC_MISSILE, 6,
				 u.ux, u.uy, u.dx, u.dy);
			break;
		}
		if(!objects[obj->otyp].oc_name_known) {
			makeknown(obj->otyp);
			more_experienced(0,10);
		}
	}
	return;
}

char *
exclam(force)
register int force;
{
	/* force == 0 occurs e.g. with sleep ray */
	/* note that large force is usual with wands so that !! would
		require information about hand/weapon/wand */
	return( (force < 0) ? "?" : (force <= 4) ? "." : "!" );
}

void
hit(str,mtmp,force)
register char *str;
register struct monst *mtmp;
register char *force;		/* usually either "." or "!" */
{
	if(!cansee(mtmp->mx,mtmp->my) || !flags.verbose) pline("The %s hits it.", str);
	else pline("The %s hits %s%s", str, mon_nam(mtmp), force);
}

void
miss(str,mtmp)
register char *str;
register struct monst *mtmp;
{
	pline("The %s misses %s.", str,
	      (cansee(mtmp->mx,mtmp->my) && flags.verbose) ? mon_nam(mtmp) : "it");
}

/* bhit: called when a weapon is thrown (sym = obj->olet) or when an
   IMMEDIATE wand is zapped (sym = 0); the weapon falls down at end of
   range or when a monster is hit; the monster is returned, and bhitpos
   is set to the final position of the weapon thrown; the ray of a wand
   may affect several objects and monsters on its path - for each of
   these an argument function is called. */
/* check !u.uswallow before calling bhit() */

struct monst *
bhit(ddx,ddy,range,sym,fhitm,fhito,obj)
register int ddx,ddy,range;		/* direction and range */
char sym;				/* symbol displayed on path */
int (*fhitm)(), (*fhito)();		/* fns called when mon/obj hit */
struct obj *obj;			/* 2nd arg to fhitm/fhito */
{
	register struct monst *mtmp;
	register struct obj *otmp;
	register int typ;
	boolean shopdoor = FALSE;
	xchar dlx, dly;

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	if(sym) {
		tmp_at(-1, sym);	/* open call */
		tmp_at(-3, (int)AT_OBJ);
	}
	while(range-- > 0) {
#ifdef STRONGHOLD
		int x,y;
#endif
		bhitpos.x += ddx;
		bhitpos.y += ddy;
#ifdef STRONGHOLD
		x = bhitpos.x; y = bhitpos.y;
		if (find_drawbridge(&x,&y))
		    switch (obj->otyp) {
			case WAN_OPENING:
# ifdef SPELLS
			case SPE_KNOCK:
# endif
			    (void) open_drawbridge(x,y);
			    break;
			case WAN_LOCKING:
# ifdef SPELLS
			case SPE_WIZARD_LOCK:
# endif
			    (void) close_drawbridge(x,y);
			    break;
			case WAN_STRIKING:
# ifdef SPELLS
			case SPE_FORCE_BOLT:
# endif
			    destroy_drawbridge(x,y);
		    }
#endif /* STRONGHOLD /**/
		if(levl[bhitpos.x][bhitpos.y].mmask){
			mtmp = m_at(bhitpos.x,bhitpos.y);
			if(sym) {
				tmp_at(-1, -1);	/* close call */
				return(mtmp);
			}
			(*fhitm)(mtmp, obj);
			range -= 3;
		}
		/* modified by GAN to hit all objects */
		if(fhito && OBJ_AT(bhitpos.x, bhitpos.y)){
		    int hitanything = 0;
		    otmp = fobj;
		    /* Fix for polymorph bug, Tim Wright */
		    while(otmp) { /* was a "for" loop.  */
			register struct obj *next_obj;

			next_obj = otmp->nobj;
			if(otmp->ox == bhitpos.x && otmp->oy == bhitpos.y)
			    hitanything += (*fhito)(otmp, obj);
			otmp = next_obj;
		    }
		    if(hitanything)	range--;
		}
		typ = levl[bhitpos.x][bhitpos.y].typ;
		if(IS_DOOR(typ) || typ == SDOOR) {
		    switch (obj->otyp) {
			case WAN_OPENING:
			case WAN_LOCKING:
			case WAN_STRIKING:
#ifdef SPELLS
			case SPE_KNOCK:
			case SPE_WIZARD_LOCK:
			case SPE_FORCE_BOLT:
#endif
			    if (doorlock(obj, bhitpos.x, bhitpos.y)) {
				makeknown(obj->otyp);
				if (levl[bhitpos.x][bhitpos.y].doormask == D_BROKEN
				    && in_shop(bhitpos.x, bhitpos.y)) {
					shopdoor = TRUE;
					dlx = bhitpos.x; dly = bhitpos.y;
				}
			    }
			    break;
		    }
		}
		if(!ZAP_POS(typ) || (IS_DOOR(typ) &&
		   (levl[bhitpos.x][bhitpos.y].doormask & (D_LOCKED | D_CLOSED)))
		  ) {
			bhitpos.x -= ddx;
			bhitpos.y -= ddy;
			break;
		}
		if(sym) tmp_at(bhitpos.x, bhitpos.y);
#ifdef SINKS
		if(sym && IS_SINK(typ))
			break;	/* physical objects fall onto sink */
#endif
	}

	/* leave last symbol unless in a pool */
	if(sym)
	   tmp_at(-1, is_pool(bhitpos.x,bhitpos.y) ? -1 : 0);

	if(shopdoor && !in_shop(u.ux, u.uy))
		pay_for_door(dlx, dly, "destroy");

	return (struct monst *)0;
}

struct monst *
boomhit(dx, dy)
int dx, dy;
{
	register int i, ct;
	char sym = ')';

	bhitpos.x = u.ux;
	bhitpos.y = u.uy;

	for(i=0; i<8; i++) if(xdir[i] == dx && ydir[i] == dy) break;
	tmp_at(-1, sym);	/* open call */
#ifndef TEXTCOLOR
	tmp_at(-3, (int)AT_OBJ);
#else
	tmp_at(-3, HI_METAL);
#endif
	for(ct=0; ct<10; ct++) {
		if(i == 8) i = 0;
		sym = ')' + '(' - sym;
		tmp_at(-2, sym);	/* change let call */
		dx = xdir[i];
		dy = ydir[i];
		bhitpos.x += dx;
		bhitpos.y += dy;
		if(levl[bhitpos.x][bhitpos.y].mmask){
			tmp_at(-1,-1);
			return(m_at(bhitpos.x,bhitpos.y));
		}
		if(!ZAP_POS(levl[bhitpos.x][bhitpos.y].typ)) {
			bhitpos.x -= dx;
			bhitpos.y -= dy;
			break;
		}
		if(bhitpos.x == u.ux && bhitpos.y == u.uy) { /* ct == 9 */
			if(rn2(20) >= ACURR(A_DEX)){ /* we hit ourselves */
				(void) thitu(10, rnd(10), "boomerang");
				break;
			} else {	/* we catch it */
				tmp_at(-1,-1);
				pline("Skillfully, you catch the boomerang.");
				return(&youmonst);
			}
		}
		tmp_at(bhitpos.x, bhitpos.y);
		if(ct % 5 != 0) i++;
#ifdef SINKS
		if(IS_SINK(levl[bhitpos.x][bhitpos.y].typ))
			break;	/* boomerang falls on sink */
#endif
	}
	tmp_at(-1, -1);	/* do not leave last symbol */
	return (struct monst *)0;
}

static uchar
dirlet(dx, dy)
register int dx, dy;
{
	return
	    (dx == dy) ? LSLANT_SYM :
	    (dx && dy) ? RSLANT_SYM :
	    dx ? HBEAM_SYM : VBEAM_SYM;
}

static int
zhit(mon, type, nd)			/* returns damage to mon */
register struct monst *mon;
register int type, nd;
{
	register int tmp = 0;
	register int abstype = abs(type) % 10;

	switch(abstype) {
	case 0:			/* magic missile */
		tmp = d(nd,6);
		break;
	case 1:			/* fire */
		if(resists_fire(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		if(resists_cold(mon->data)) tmp += 7;
		break;
	case 2:			/* sleep */
		tmp = 0;
		if(resists_sleep(mon->data) ||
		   resist(mon, (type == 2) ? WAND_SYM : '\0', 0, NOTELL))
			shieldeff(mon->mx, mon->my);
		else	mon->mfroz = 1;
		break;
	case 3:			/* cold */
		if(resists_cold(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		if(resists_fire(mon->data)) tmp += d(nd, 3);
		break;
	case 4:			/* death/disintegration */
		if(abs(type) != 24) {	/* death */
		    if(is_undead(mon->data)) {
			shieldeff(mon->mx, mon->my);
			break;
		    }
		    type = 0; /* so they don't get saving throws */
		}
		tmp = mon->mhp+1;
		break;
	case 5:			/* lightning */
		if(resists_elec(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		{
			register unsigned rnd_tmp = rnd(50);
			mon->mcansee = 0;
			if((mon->mblinded + rnd_tmp) > 127)
				mon->mblinded = 127;
			else mon->mblinded += rnd_tmp;
		}
		break;
	case 6:			/* poison */
		if(resists_poison(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		break;
	case 7:			/* acid */
		if(resists_acid(mon->data)) {
		    shieldeff(mon->mx, mon->my);
		    break;
		}
		tmp = d(nd,6);
		break;
	}
	if (type >= 0)
	    if (resist(mon, (type < 10) ? WAND_SYM : '\0', 0, NOTELL)) tmp /= 2;
	mon->mhp -= tmp;
	return(tmp);
}

/*
 * burn scrolls on floor at position x,y
 * return the number of scrolls burned
 */
static int
burn_floor_scrolls(x, y)
int x, y;
{
	register struct obj *obj, *obj2;
	register int scrquan, i, cnt = 0;

	if(OBJ_AT(x, y))
	for(obj = fobj; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    /* Bug fix - KAA */
	    if(obj->ox == x && obj->oy == y && obj->olet == SCROLL_SYM) {
		scrquan = obj->quan;
		for(i = 1; i <= scrquan ; i++)
		    if(!rn2(3))  {
			cnt++;
			if(in_shop(u.ux, u.uy))
				addtobill(obj, FALSE);
			useupf(obj);
		    }
	    }
	}
	return(cnt);
}

/* type == 0 to 9     : you shooting a wand */
/* type == 10 to 19   : you casting a spell */
/* type == 20 to 29   : you breathing as a monster */
/* type == -10 to -19   : monster casting spell */
/* type == -20 to -29 : monster breathing at you */
/* called with dx = dy = 0 with vertical bolts */
void
buzz(type,nd,sx,sy,dx,dy)
register int type, nd;
register xchar sx,sy;
register int dx,dy;
{
	int abstype = abs(type) % 10;
	register char *fltxt = fl[abs(type)];
	struct rm *lev;
	xchar range, olx, oly;
	struct monst *mon;
	register boolean bodyhit = FALSE;
	register boolean shopdoor = FALSE;

	if(u.uswallow) {
		register int tmp;

		if(type < 0) return;
		tmp = zhit(u.ustuck, type, nd);
		if(!u.ustuck)	u.uswallow = 0;
		else	pline("The %s rips into %s%s",
				fltxt, mon_nam(u.ustuck), exclam(tmp));
		if (u.ustuck->mhp < 1)
			killed(u.ustuck);
		return;
	}
	if(type < 0) pru();
	range = rn1(7,7);
	Tmp_at2(-1, (int) dirlet(dx,dy));	/* open call */
#ifdef TEXTCOLOR
	Tmp_at2(-3, zapcolor[abstype]);
#endif
	while(range-- > 0) {
		sx += dx;
		sy += dy;
		if((lev = &levl[sx][sy])->typ) Tmp_at2(sx,sy);
		else {
			int bounce = 0;
			if(cansee(sx-dx,sy-dy))
				pline("The %s bounces!", fltxt);
			if(ZAP_POS(levl[sx][sy-dy].typ))
				bounce = 1;
			if(ZAP_POS(levl[sx-dx][sy].typ)) {
				if(!bounce || rn2(2)) bounce = 2;
			}
			switch(bounce){
			case 0:
				dx = -dx;
				dy = -dy;
				continue;
			case 1:
				dy = -dy;
				sx -= dx;
				break;
			case 2:
				dx = -dx;
				sy -= dy;
				break;
			}
			Tmp_at2(-2,(int) dirlet(dx,dy));
			continue;
		}
		if(is_pool(sx,sy) && abstype == 1 /* fire */) {
#ifdef STRONGHOLD
			if(lev->typ != POOL) {	/* MOAT or DRAWBRIDGE_UP */
			    if(cansee(sx,sy))
				pline("Some water evaporates.");
			    else if(flags.soundok)
				You("hear a hissing sound.");
			} else {
#endif
			    register struct trap *ttmp;

			    range -= 3;
			    lev->typ = ROOM;
			    mnewsym(sx,sy);
			    if(cansee(sx,sy)) pline("The water evaporates.");
			    else if(flags.soundok)
				You("hear a hissing sound.");
			    ttmp = maketrap(sx, sy, PIT);
			    ttmp->tseen = 1;
#ifdef STRONGHOLD
			}
#endif
		}
		if(is_pool(sx,sy) && abstype == 3 /* cold */) {
			boolean moat = (lev->typ != POOL);

			range -= 3;
#ifdef STRONGHOLD
			if(lev->typ == DRAWBRIDGE_UP)
				lev->drawbridgemask |= DB_FLOOR;
			else
#endif
				lev->typ = ROOM;
			mnewsym(sx,sy);
			if(cansee(sx,sy)) {
				if(moat)
					pline("The moat is bridged with ice!");
				else 	pline("The water freezes.");
			} else if(flags.soundok)
				You("hear a crackling sound.");
		}
		if(IS_DOOR(lev->typ) && (lev->doormask & (D_LOCKED | D_CLOSED))) {
			range = 0;
			switch(abstype) {
			case 1:
			   lev->doormask = D_NODOOR;
			   if(cansee(sx,sy))
				pline("The door is consumed in flames!");
			   else You("smell smoke.");
			   if(type >= 0 && in_shop(sx, sy)) {
				shopdoor = TRUE;
				olx = sx;
				oly = sy;
			   }
			   break;
			case 3:
			   lev->doormask = D_NODOOR;
			   if(cansee(sx,sy))
				pline("The door freezes and shatters!");
			   else You("feel cold.");
			   if(type >= 0 && in_shop(sx, sy)) {
				shopdoor = TRUE;
				olx = sx;
				oly = sy;
			   }
			   break;
			case 4:
			   lev->doormask = D_NODOOR;
			   if(cansee(sx,sy))
				pline("The door disintegrates!");
			   else if(flags.soundok)
				You("hear a crashing sound.");
			   if(type >= 0 && in_shop(sx, sy)) {
				shopdoor = TRUE;
				olx = sx;
				oly = sy;
			   }
			   break;
			case 5:
			   lev->doormask = D_BROKEN;
			   if(cansee(sx,sy))
				pline("The door splinters!");
			   else if(flags.soundok)
				You("hear a crackling sound.");
			   if(type >= 0 && in_shop(sx, sy)) {
				shopdoor = TRUE;
				olx = sx;
				oly = sy;
			   }
			   break;
			default:
			   if(cansee(sx,sy)) {
			       if (type >= 0 && type <= 9)
				   pline("The door absorbs your bolt!");
			       else if (type >= 10 && type <= 19)
				   pline("The door absorbs your spell!");
#ifdef POLYSELF
			       else if (type >= 20 && type <= 29)
				   pline("The door absorbs your blast!");
#endif
			       else if (type >= -19 && type <= -10)
				   pline("The door absorbs the spell!");
			       else
				   pline("The door absorbs the blast!");
			   } else You("feel vibrations.");
			   break;
			}
		}
		if(OBJ_AT(sx, sy) && abstype == 1)
			if(burn_floor_scrolls(sx,sy) && cansee(sx,sy))  {
			    mnewsym(sx,sy);
			    if(!Blind)
				You("see a puff of smoke.");
			}
		if(levl[sx][sy].mmask){
			mon = m_at(sx,sy);
			/* Cannot use wakeup() which also angers the monster */
			mon->msleep = 0;
			if(mon->mimic) seemimic(mon);
			if(type >= 0) {
			    setmangry(mon);
#if defined(ALTARS) && defined(THEOLOGY)
			    if(mon->ispriest && in_temple(mon->mx, mon->my))
				priesthit = TRUE;
#endif
			}
			if(rnd(20) < 18 + mon->data->ac) {
			    register int tmp = zhit(mon, type, nd);
			    if(mon->mhp < 1) {
				if(type < 0) {
				    if(cansee(mon->mx,mon->my))
				      pline("%s is killed by the %s!",
					    Monnam(mon), fltxt);
				    mondied(mon);
				} else
				    killed(mon);
			     } else
				hit(fltxt, mon, exclam(tmp));
			    range -= 2;
			} else
				miss(fltxt,mon);
		} else if(sx == u.ux && sy == u.uy) {
			nomul(0);
			if(rnd(20) < 18+u.uac) {
				register int dam = 0;
				range -= 2;
				pline("The %s hits you!",fltxt);
				if (Reflecting) {
				    if (!Blind) {
					if(Reflecting & WORN_AMUL)
					    makeknown(AMULET_OF_REFLECTION);
					else
					    makeknown(SHIELD_OF_REFLECTION);
			pline("But it reflects from your %s!",
				(Reflecting & W_AMUL) ? "amulet" : "shield");
				    } else
			pline("For some reason you are not affected!");
				    if (dx) dx = -dx;
				    if (dy) dy = -dy;
				    shieldeff(sx, sy);
				}
				else switch(abstype) {
				case 0:		/* magic missile */
				    if(Antimagic) {
					shieldeff(sx, sy);
					pline("The missiles bounce off!");
				    } else dam = d(nd,6);
				    break;
				case 1:		/* fire */
				    if(Fire_resistance) {
					shieldeff(sx, sy);
					You("don't feel hot!");
#ifdef POLYSELF
#ifdef GOLEMS
					ugolemeffects(AD_FIRE, d(nd, 6));
#endif /* GOLEMS */
#endif
				    } else dam = d(nd, 6);
				    while (1) {
					switch(rn2(5)) {
					case 0:
		if (!rust_dmg(uarmh, "leather helmet", 0, FALSE)) continue;
					    break;
					case 1:
					    bodyhit = TRUE;
					    if (uarmc) break;
		(void)(rust_dmg(uarm, "leather armor", 0, FALSE));
					    break;
					case 2:
		if (!rust_dmg(uarms, "wooden shield", 0, FALSE)) continue;
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
					destroy_item(POTION_SYM, AD_FIRE);
				    if(!rn2(3) && bodyhit)
					destroy_item(SCROLL_SYM, AD_FIRE);
#ifdef SPELLS
				    if(!rn2(5) && bodyhit)
					destroy_item(SPBOOK_SYM, AD_FIRE);
#endif
				    break;
				case 2:		/* sleep */
				    if(Sleep_resistance) {
					shieldeff(u.ux, u.uy);
					You("don't feel sleepy.");
				    } else nomul(-d(nd,25)); /* sleep ray */
				    break;
				case 3:		/* cold */
				    if(Cold_resistance) {
					shieldeff(sx, sy);
					You("don't feel cold.");
#ifdef POLYSELF
#ifdef GOLEMS
					ugolemeffects(AD_COLD, d(nd, 6));
#endif /* GOLEMS */
#endif
				    } else
					dam = d(nd, 6);
				    if(!rn2(3))
					destroy_item(POTION_SYM, AD_COLD);
				    break;
				case 4:		/* death */
				    if(type == -24) { /* disintegration */
					if(uarms) {
					    (void) destroy_arm(uarms);
					} else if (uarm) {
				if (Disint_resistance & WORN_ARMOR)
					Your("armor absorbs the blast!");
				else (void) destroy_arm(uarm);
					}
				    break;
				    }
#ifdef POLYSELF
				    else if(is_undead(uasmon)) {
					shieldeff(sx, sy);
					You("seem unaffected.");
					break;
				    }
#endif
				    u.uhp = -1;
				    break;
				case 5:		/* lightning */
				    if (Shock_resistance) {
					shieldeff(sx, sy);
					You("aren't affected.");
#ifdef POLYSELF
#ifdef GOLEMS
					ugolemeffects(AD_ELEC, d(nd, 6));
#endif /* GOLEMS */
#endif
				    } else
					dam = d(nd, 6);
				    break;
				case 6:		/* poison */
				    poisoned("blast", A_DEX, "poisoned blast");
				    break;
				case 7:		/* acid */
				    pline("The acid burns!");
				    dam = d(nd,6);
				    if(!rn2(6)) corrode_weapon();
				    if(!rn2(6)) corrode_armor();
				    break;
				}
				losehp(dam,fltxt);
			} else pline("The %s whizzes by you!",fltxt);
			if (abstype == 5 && !Blind) { /* LIGHTNING */
		    		You("are blinded by the flash!");
				make_blinded((long)d(nd,50),FALSE);
				seeoff(0);
			}
			stop_occupation();
		}
		if(!ZAP_POS(lev->typ)) {
			int bounce = 0, rmn;
			if(cansee(sx,sy)) pline("The %s bounces!", fltxt);
			range--;
			if(!dx || !dy || !rn2(20)){
				dx = -dx;
				dy = -dy;
			} else {
			  if(ZAP_POS(rmn = levl[sx][sy-dy].typ) &&
			    (IS_ROOM(rmn) || ZAP_POS(levl[sx+dx][sy-dy].typ)))
				bounce = 1;
			  if(ZAP_POS(rmn = levl[sx-dx][sy].typ) &&
			    (IS_ROOM(rmn) || ZAP_POS(levl[sx-dx][sy+dy].typ)))
				if(!bounce || rn2(2))
					bounce = 2;

			  switch(bounce){
			  case 0:
				dy = -dy;
				dx = -dx;
				break;
			  case 1:
				dy = -dy;
				break;
			  case 2:
				dx = -dx;
				break;
			  }
			  Tmp_at2(-2, (int) dirlet(dx,dy));
			}
		}
	}
	Tmp_at2(-1,-1);
	if(shopdoor && !in_shop(u.ux, u.uy))
		pay_for_door(olx, oly, abstype == 1 ? "burn away" :
				       abstype == 3 ? "shatter" :
				       abstype == 4 ? "disintegrate" :
				       "destroy");
}

void
rloco(obj)
register struct obj *obj;
{
	register int tx, ty, otx, oty;

	otx = obj->ox;
	oty = obj->oy;
	do {
		tx = rn1(COLNO-3,2);
		ty = rn2(ROWNO);
	} while(!goodpos(tx,ty,(struct permonst *)0));
	move_object(obj, tx, ty);
	if(cansee(otx,oty))
		newsym(otx,oty);
	if(cansee(tx,ty))
		newsym(tx,ty);
}

void
fracture_rock(obj)	/* fractured by pick-axe or wand of striking */
register struct obj *obj;		   /* no texts here! */
{
	/* unpobj(obj); */
	obj->otyp = ROCK;
	obj->quan = 7 + rn2(60);
	obj->owt = weight(obj);
	obj->olet = GEM_SYM;
	obj->known = FALSE;
	if(cansee(obj->ox,obj->oy))
		prl(obj->ox,obj->oy);
}

boolean
break_statue(obj)
register struct obj *obj;
{
	struct trap *trap;

	if(trap = t_at(obj->ox,obj->oy))
	    if(obj->corpsenm == trap->pm)
		if(makemon(&mons[trap->pm], obj->ox, obj->oy)) {
		    pline("Instead of shattering, the statue suddenly comes alive!");
		    delobj(obj);
		    deltrap(trap);
		    return FALSE;
		}
	if (obj->spe) {
	    struct obj *magazine;
#ifdef SPELLS
	    magazine = mkobj_at(SPBOOK_SYM, obj->ox, obj->oy);
#else
	    magazine = mkobj_at(SCROLL_SYM, obj->ox, obj->oy);
#endif
	    magazine->blessed = obj->blessed;
	    magazine->cursed = obj->cursed;
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
	register int quan, i, cnt, dmg, xresist, skip;
	register int dindx;
	char *mult;

	for(obj = invent; obj; obj = obj2) {

	    obj2 = obj->nobj;
	    if(obj->olet != osym) continue; /* test only objs of type osym */
	    xresist = skip = 0;

	    switch(dmgtyp) {
		case AD_COLD:
		    if(osym == POTION_SYM) {
			quan = obj->quan;
			dindx = 0;
			dmg = rnd(4);
		    } else skip++;
	    	    break;
		case AD_FIRE:
		    xresist = (Fire_resistance && obj->olet != POTION_SYM);

		    /* Let's say scrolls of fire are fire resistant */

		    if (obj->otyp == SCR_FIRE)
		      skip++;
		    quan = obj->quan;
		    switch(osym) {
			case POTION_SYM:
			    dindx = 1;
			    dmg = rnd(6);
			    break;
			case SCROLL_SYM:
			    dindx = 2;
			    dmg = 1;
			    break;
#ifdef SPELLS
			case SPBOOK_SYM:
			    dindx = 3;
			    dmg = 1;
			    break;
#endif
			default:
			    skip++;
			    break;
		    }
		    break;
		case AD_ELEC:
		    xresist = (Shock_resistance && obj->olet != RING_SYM);
		    quan = obj->quan;
		    switch(osym) {
			case RING_SYM:
			    if(obj->otyp == RIN_SHOCK_RESISTANCE)
				    { skip++; break; }
			    dindx = 4;
			    dmg = 0;
			    break;
			case WAND_SYM:
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
		for(i = cnt = 0; i < quan; i++)
		    if(!rn2(3)) cnt++;

		if(!cnt) continue;
		if(cnt == quan)	mult = "Your";
		else	mult = (cnt == 1) ? "One of your" : "Some of your";
		pline("%s %s %s!", mult, xname(obj),
			(cnt > 1) ? destroy_strings[dindx*3 + 1]
				  : destroy_strings[dindx*3]);
		if(osym == POTION_SYM && dmgtyp != AD_COLD)
		    potionbreathe(obj);
		for(i = 0; i < cnt; i++) useup(obj);
		if(dmg) {
		    if(xresist)	You("aren't hurt!");
		    else	losehp(dmg, destroy_strings[dindx*3 + 2]);
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
	register int quan, i, cnt, skip, tmp = 0;
	register int dindx;

	for(obj = mtmp->minvent; obj; obj = obj2) {

	    obj2 = obj->nobj;
	    if(obj->olet != osym) continue; /* test only objs of type osym */
	    skip = 0;

	    switch(dmgtyp) {
		case AD_COLD:
		    if(osym == POTION_SYM) {
			quan = obj->quan;
			dindx = 0;
			tmp++;
		    } else skip++;
	    	    break;
		case AD_FIRE:
		    /* Let's say scrolls of fire are fire resistant */

		    if (obj->otyp == SCR_FIRE)
		      skip++;
		    quan = obj->quan;
		    switch(osym) {
			case POTION_SYM:
			    dindx = 1;
			    tmp++;
			    break;
			case SCROLL_SYM:
			    dindx = 2;
			    tmp++;
			    break;
#ifdef SPELLS
			case SPBOOK_SYM:
			    dindx = 3;
			    tmp++;
			    break;
#endif
			default:
			    skip++;
			    break;
		    }
		    break;
		case AD_ELEC:
		    quan = obj->quan;
		    switch(osym) {
			case RING_SYM:
			    if(obj->otyp == RIN_SHOCK_RESISTANCE)
				    { skip++; break; }
			    dindx = 4;
			    break;
			case WAND_SYM:
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
		for(i = cnt = 0; i < quan; i++)
		    if(!rn2(3)) cnt++;

		if(!cnt) continue;
		pline("%s's %s %s!", Monnam(mtmp), xname(obj),
			(cnt > 1) ? destroy_strings[dindx*3 + 1]
				  : destroy_strings[dindx*3]);
		for(i = 0; i < cnt; i++) m_useup(mtmp, obj);
	    }
	}
	return(tmp);
}

/*ARGSUSED*/
int
resist(mtmp, olet, damage, tell)
register struct monst	*mtmp;
register char	olet;
register int	damage, tell;
{
	register int	resisted = 0;
#ifdef HARD
	register int	level;

	switch(olet)  {

	    case WAND_SYM:
			level = 8;
			break;

	    case SCROLL_SYM:
			level = 6;
			break;

	    case POTION_SYM:
			level = 5;
			break;

	    default:	level = u.ulevel;
			break;
	}

	resisted = (rn2(100) - mtmp->m_lev + level) < mtmp->data->mr;
	if(resisted) {

		if(tell) {
		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s resists!", canseemon(mtmp) ? Monnam(mtmp) : "It");
		}
		mtmp->mhp -= damage/2;
	} else
#endif
		mtmp->mhp -= damage;

	if(mtmp->mhp < 1) killed(mtmp);
	return(resisted);
}

void
makewish()
{
	char buf[BUFSZ];
	register struct obj *otmp;
	int wishquan, mergquan;
	register boolean dropit = (inv_cnt() >= 52);
	int tries = 0;

retry:
	You("may wish for an object.  What do you want? ");
	getlin(buf);
	if(buf[0] == '\033') buf[0] = 0;
/* Note: if they wished for and got a non-object successfully, such as gold,
 * otmp = &zeroobj
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
	    if(dropit) {
		pline("Oops!  The %s drop%s to the floor!", xname(otmp),
			otmp->quan > 1 ? "" : "s");
		dropy(otmp);
	    } else {
	    	wishquan = otmp->quan;
	    	otmp = addinv(otmp);
	    	mergquan = otmp->quan;
	    	otmp->quan = wishquan; /* to fool prinv() */
	    	prinv(otmp);
	    	otmp->quan = mergquan;
	    }
#ifdef WIZARD
 	if (!wizard)
#endif
	    if(otmp->otyp == WAN_WISHING) otmp->recharged = 1;

#ifdef THEOLOGY
	    u.ublesscnt += rn1(100,50);  /* the gods take notice */
#endif
	}
}
