/*	SCCS Id: @(#)trap.c	3.1	93/06/20	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVLB
const char *traps[TRAPNUM] = {
	"",
	"n arrow trap",
	" dart trap",
	" falling rock trap",
	" squeaky board",
	" bear trap",
	" land mine",
	" sleeping gas trap",
	" rust trap",
	" fire trap",
	" pit",
	" spiked pit",
	" trapdoor",
	" teleportation trap",
	" level teleporter",
	" magic portal",
	" web",
	" statue trap",
	" magic trap",
	"n anti-magic field"
#ifdef POLYSELF
	," polymorph trap"
#endif
};

#endif /* OVLB */

static void FDECL(domagicportal,(struct trap *));
static void NDECL(dofiretrap);
static void NDECL(domagictrap);
static boolean FDECL(emergency_disrobe,(boolean *));
STATIC_DCL boolean FDECL(thitm, (int, struct monst *, struct obj *, int));

#ifdef OVLB

static int FDECL(teleok, (int,int,BOOLEAN_P));
static void NDECL(vtele);
static void FDECL(no_fall_through, (BOOLEAN_P));

/* Generic rust-armor function.  Returns TRUE if a message was printed;
 * "print", if set, means to print a message (and thus to return TRUE) even
 * if the item could not be rusted; otherwise a message is printed and TRUE is
 * returned only for rustable items.
 */
boolean
rust_dmg(otmp, ostr, type, print)
register struct obj *otmp;
register const char *ostr;
int type;
boolean print;
{
	static NEARDATA const char *action[] = { "smoulder", "rust", "rot", "corrode" };
	static NEARDATA const char *msg[] =  { "burnt", "rusted", "rotten", "corroded" };
	boolean vulnerable = FALSE;
	boolean plural;
	boolean grprot = FALSE;

	if (!otmp) return(FALSE);
	switch(type) {
		case 0:
		case 2: vulnerable = is_flammable(otmp); break;
		case 1: vulnerable = is_rustprone(otmp); grprot = TRUE; break;
		case 3: vulnerable = is_corrodeable(otmp); grprot = TRUE; break;
	}

	if (!print && (!vulnerable || otmp->oerodeproof || otmp->oeroded == MAX_ERODE))
		return FALSE;

	plural = is_gloves(otmp) || is_boots(otmp);

	if (!vulnerable) {
		if (flags.verbose)
		    Your("%s %s not affected.", ostr, plural ? "are" : "is");
	} else if (otmp->oeroded < MAX_ERODE) {
		if (grprot && otmp->greased) {
			grease_protect(otmp,ostr,plural);
		} else if (otmp->oerodeproof || (otmp->blessed && !rnl(4))) {
			if (flags.verbose)
				pline("Somehow, your %s %s not affected.",
					ostr, plural ? "are" : "is");
		} else {
			Your("%s %s%s%s!", ostr, action[type],
				plural ? "" : "s",
			        otmp->oeroded+1 == MAX_ERODE ? " completely" :
				otmp->oeroded ? " further" : "");
			otmp->oeroded++;
		}
	} else {
		if (flags.verbose)
			Your("%s %s%s completely %s.", ostr,
			     Blind ? "feel" : "look",
			     plural ? "" : "s", msg[type]);
	}
	return(TRUE);
}

void
grease_protect(otmp,ostr,plu)
register struct obj *otmp;
register const char *ostr;
register boolean plu;
{
	static const char txt[] = "protected by the layer of grease!";

	if (ostr)
		Your("%s %s %s",ostr,plu ? "are" : "is",txt);
	else
		Your("%s %s",aobjnam(otmp,"are"),txt);
	if (!rn2(2)) {
		pline("The grease dissolves.");
		otmp->greased = 0;
	}
}

struct trap *
maketrap(x,y,typ)
register int x, y, typ;
{
	register struct trap *ttmp;
	register struct rm *lev;
	register boolean oldplace;

	if ((ttmp = t_at(x,y)) != 0) {
	    if (ttmp->ttyp == MAGIC_PORTAL) return (struct trap *)0;
	    oldplace = TRUE;
	    if (u.utrap && (x == u.ux) && (y == u.uy) && 
	      ((u.utraptype == TT_BEARTRAP && typ != BEAR_TRAP) ||
	      (u.utraptype == TT_WEB && typ != WEB) ||
	      (u.utraptype == TT_PIT && typ != PIT && typ != SPIKED_PIT)))
		    u.utrap = 0;
	} else {
	    oldplace = FALSE;
	    ttmp = newtrap();
	    ttmp->tx = x;
	    ttmp->ty = y;
	}
	ttmp->ttyp = typ;
	switch(typ) {
	    case STATUE_TRAP:	    /* create a "living" statue */
		(void) mkcorpstat(STATUE, &mons[rndmonnum()], x, y, FALSE);
		break;
	    case PIT:
	    case SPIKED_PIT:
	    case TRAPDOOR:
		lev = &levl[x][y];
		lev->doormask = 0;	/* subsumes altarmask, icedpool... */
		if (IS_ROOM(lev->typ)) /* && !IS_AIR(lev->typ) */
		    lev->typ = ROOM;
#if defined(POLYSELF) || defined(MUSE)
		/*
		 * some cases which can happen when digging
		 * down while phazing thru solid areas
		 */
		else if (lev->typ == STONE || lev->typ == SCORR)
		    lev->typ = CORR;
		else if (IS_WALL(lev->typ) ||
			 IS_DOOR(lev->typ) || lev->typ == SDOOR)
		    lev->typ = level.flags.is_maze_lev ? ROOM :
			       level.flags.is_cavernous_lev ? CORR : DOOR;
#endif
		unearth_objs(x, y);
		break;
	}
	ttmp->tseen = 0;
	ttmp->once = 0;
	ttmp->dst.dnum = -1;
	ttmp->dst.dlevel = -1;
	if (!oldplace) {
	    ttmp->ntrap = ftrap;
	    ftrap = ttmp;
	}
	return(ttmp);
}

static int
teleok(x, y, trapok)
register int x, y;
boolean trapok;
{				/* might throw him into a POOL
				 * removed by GAN 10/20/86
				 */
#ifdef STUPID
	boolean	tmp1, tmp2, tmp3, tmp4;
# ifdef POLYSELF
	tmp1 = isok(x,y) && (!IS_ROCK(levl[x][y].typ) ||
		(passes_walls(uasmon) && may_passwall(x,y))) && !MON_AT(x, y);
# else
	tmp1 = isok(x,y) && !IS_ROCK(levl[x][y].typ) && !MON_AT(x, y);
# endif
	tmp2 = !sobj_at(BOULDER,x,y) && (trapok || !t_at(x,y));
	tmp3 = !(is_pool(x,y) &&
	       !(Levitation || Wwalking || Amphibious
# ifdef POLYSELF
		 || is_flyer(uasmon) || is_swimmer(uasmon)
		 || is_clinger(uasmon)
# endif
		)) && !closed_door(x,y);
	tmp4 = !is_lava(x,y);
	return(tmp1 && tmp2 && tmp3 && tmp4);
#else
	return( isok(x,y) &&
# ifdef POLYSELF
		(!IS_ROCK(levl[x][y].typ) ||
		 (passes_walls(uasmon) && may_passwall(x,y))) &&
# else
		!IS_ROCK(levl[x][y].typ) &&
# endif
		!MON_AT(x, y) &&
		!sobj_at(BOULDER,x,y) && (trapok || !t_at(x,y)) &&
		!(is_pool(x,y) &&
		!(Levitation || Wwalking || Amphibious
# ifdef POLYSELF
		  || is_flyer(uasmon) || is_swimmer(uasmon)
		  || is_clinger(uasmon)
# endif
		  )) && !is_lava(x,y) && !closed_door(x,y));
#endif
	/* Note: gold is permitted (because of vaults) */
}

boolean
safe_teleds()
{
	register int nux, nuy;
	short tcnt = 0;

	do {
		nux = rnd(COLNO-1);
		nuy = rn2(ROWNO);
	} while (!teleok(nux, nuy, tcnt>200) && tcnt++ < 400);

	if (tcnt < 400) {
		teleds(nux, nuy);
		return TRUE;
	} else
		return FALSE;
}

static void
vtele()
{
	register struct mkroom *croom = search_special(VAULT);
	coord c;

	if(croom && somexy(croom, &c) && teleok(c.x,c.y,FALSE)) {
		teleds(c.x,c.y);
		return;
	}
	tele();
}

static void
no_fall_through(td)
boolean td;
{
	/* floor objects get a chance of falling down.  the case
	 * where the hero does NOT fall down is treated here.  the
	 * case where the hero does fall down is treated in goto_level().
	 * reason: the target level of the fall is not determined here,
	 * and it need not be the next level.  if we want falling
	 * objects to arrive near the player, we must call impact_drop()
	 * _after_ the target level is determined.
	 */
	impact_drop((struct obj *)0, u.ux, u.uy, 0);
	if (!td) {
		display_nhwindow(WIN_MESSAGE, FALSE);
		pline("The opening under you closes up.");
	}
}

void
fall_through(td)
boolean td;	/* td == TRUE : trapdoor */
{
	register int newlevel = dunlev(&u.uz);

	if(Blind && Levitation) return;

	do {
	    newlevel++;
	} while(!rn2(4) && newlevel < dunlevs_in_dungeon(&u.uz));

	if(td) pline("A trap door opens up under you!");
	else pline("The %s opens up under you!", surface(u.ux,u.uy));

	if(Levitation || u.ustuck || !Can_fall_thru(&u.uz)
#ifdef POLYSELF
	   || is_flyer(uasmon) || is_clinger(uasmon)
#endif
	   || (Inhell && !u.uevent.invoked &&
					newlevel == dunlevs_in_dungeon(&u.uz))
		) {
		    You("don't fall in.");
		    no_fall_through(td);
		    return;
	}
#ifdef WALKIES
	if(!next_to_u()) {
	    You("are jerked back by your pet!");
	    no_fall_through(td);
	    return;
	}
#endif
	if(*u.ushops) shopdig(1);
	if(Is_stronghold(&u.uz)) goto_hell(TRUE, TRUE);
	else {
	    d_level	dtmp;
	    dtmp.dnum = u.uz.dnum;
	    dtmp.dlevel = newlevel;
	    goto_level(&dtmp, FALSE, TRUE, FALSE);
	    if(!td) pline("The hole in the ceiling above you closes up.");
	}
}

void
dotrap(trap)
register struct trap *trap;
{
	register int ttype = trap->ttyp;
	register struct monst *mtmp;
	register struct obj *otmp;

	nomul(0);
	if(trap->tseen && !Fumbling &&
#ifdef POLYSELF
	   !((ttype == PIT || ttype == SPIKED_PIT) && !is_clinger(uasmon)) &&
#else
	   !(ttype == PIT || ttype == SPIKED_PIT) &&
#endif
	   !(ttype == MAGIC_PORTAL || ttype == ANTI_MAGIC) && !rn2(5)) {
		You("escape a%s.", traps[ttype]);
	} else {
	    seetrap(trap);
	    switch(ttype) {
		case ARROW_TRAP:
		    pline("An arrow shoots out at you!");
		    otmp = mksobj(ARROW, TRUE, FALSE);
		    otmp->quan = 1L;
		    otmp->owt = weight(otmp);
		    if(thitu(8,dmgval(otmp,uasmon),otmp,"arrow")) {
			obfree(otmp, (struct obj *)0);
		    } else {
			place_object(otmp, u.ux, u.uy);
			otmp->nobj = fobj;
			fobj = otmp;		
			stackobj(otmp);
			newsym(u.ux, u.uy);
		    }
		    break;
		case DART_TRAP:
		    pline("A little dart shoots out at you!");
		    otmp = mksobj(DART, TRUE, FALSE);
		    otmp->quan = 1L;
		    otmp->owt = weight(otmp);
		    if (!rn2(6)) otmp->opoisoned = 1;
		    if(thitu(7,dmgval(otmp,uasmon),otmp,"little dart")) {
			if (otmp->opoisoned)
			    poisoned("dart",A_CON,"poison dart",10);
			obfree(otmp, (struct obj *)0);
		    } else {
			place_object(otmp, u.ux, u.uy);
			otmp->nobj = fobj;
			fobj = otmp;		
			stackobj(otmp);
			newsym(u.ux, u.uy);
		    }
		    break;
		case ROCKTRAP:
		    {
			int dmg = d(2,6); /* should be std ROCK dmg? */

			otmp = mksobj_at(ROCK, u.ux, u.uy, TRUE);
			otmp->quan = 1L;
			otmp->owt = weight(otmp);

	pline("A trap door in the ceiling opens and a rock falls on your %s!",
				body_part(HEAD));

			if (uarmh) {
			    if(is_metallic(uarmh)) {
				pline("Fortunately, you are wearing a hard helmet.");
				dmg = 2;
			    } else if (flags.verbose) {
 				Your("%s does not protect you.", xname(uarmh));
			    }
			}

			stackobj(otmp);
			newsym(u.ux,u.uy);	/* map the rock */

			losehp(dmg, "falling rock", KILLED_BY_AN);
			exercise(A_STR, FALSE);
		    }
		    break;

		case SQKY_BOARD:	    /* stepped on a squeaky board */
		    if (Levitation
#ifdef POLYSELF
			|| is_flyer(uasmon) || is_clinger(uasmon)
#endif
			) {
			if (Hallucination) You("notice a crease in the linoleum.");
			else You("notice a loose board below you.");
		    } else {
			pline("A board beneath you squeaks loudly.");
			wake_nearby();
		    }
		    break;

		case BEAR_TRAP:
		    if(Levitation
#ifdef POLYSELF
				|| is_flyer(uasmon)) {
			You("%s over a bear trap.",
			      Levitation ? "float" : "fly");
#else
				) {
			You("float over a bear trap.");
#endif
			break;
		    }
#ifdef POLYSELF
		    if(amorphous(uasmon)) {
			pline("A bear trap closes harmlessly through you.");
			break;
		    }
#endif
		    u.utrap = rn1(4, 4);
		    u.utraptype = TT_BEARTRAP;
		    pline("A bear trap closes on your %s!",
			body_part(FOOT));
#ifdef POLYSELF
		    if(u.umonnum == PM_OWLBEAR || u.umonnum == PM_BUGBEAR)
			You("howl in anger!");
#endif
		    exercise(A_DEX, FALSE);
		    break;

		case SLP_GAS_TRAP:
		    if(Sleep_resistance) {
			You("are enveloped in a cloud of gas!");
			break;
		    }
		    pline("A cloud of gas puts you to sleep!");
		    flags.soundok = 0;
		    nomul(-rnd(25));
		    u.usleep = 1;
		    nomovemsg = "You wake up.";
		    afternmv = Hear_again;
		    break;

		case RUST_TRAP:
#ifdef POLYSELF
		    if (u.umonnum == PM_IRON_GOLEM) {
			pline("A gush of water hits you!");
			You("are covered with rust!");
			rehumanize();
			break;
		    } else
		    if (u.umonnum == PM_GREMLIN && rn2(3)) {
			pline("A gush of water hits you!");
			if ((mtmp = cloneu()) != 0) {
			    mtmp->mhpmax = (u.mhmax /= 2);
			    You("multiply.");
			}
			break;
		    }
#endif
		/* Unlike monsters, traps cannot aim their rust attacks at
		 * you, so instead of looping through and taking either the
		 * first rustable one or the body, we take whatever we get,
		 * even if it is not rustable.
		 */
		    switch (rn2(5)) {
			case 0:
			    pline("A gush of water hits you on the %s!",
					body_part(HEAD));
			    (void) rust_dmg(uarmh, "helmet", 1, TRUE);
			    break;
			case 1:
			    pline("A gush of water hits your left %s!",
					body_part(ARM));
			    if (rust_dmg(uarms, "shield", 1, TRUE)) break;
			    if (uwep && bimanual(uwep))
				goto two_hand;
			    /* Two goto statements in a row--aaarrrgggh! */
glovecheck:		    (void) rust_dmg(uarmg, "gauntlets", 1, TRUE);
			    /* Not "metal gauntlets" since it gets called
			     * even if it's leather for the message
			     */
			    break;
			case 2:
			    pline("A gush of water hits your right %s!",
					body_part(ARM));
two_hand:		    erode_weapon(FALSE);
			    goto glovecheck;
			default:
			    pline("A gush of water hits you!");
			    if (uarmc) (void) rust_dmg(uarmc, "cloak", 1, TRUE);
			    else if (uarm)
				(void) rust_dmg(uarm, "armor", 1, TRUE);
#ifdef TOURIST
			    else if (uarmu)
				(void) rust_dmg(uarmu, "shirt", 1, TRUE);
#endif
		    }
		    break;

                case FIRE_TRAP:
	            dofiretrap();
	            break;

		case PIT:
		    if (Levitation
#ifdef POLYSELF
			|| is_flyer(uasmon) || is_clinger(uasmon)
#endif
			) {
			if(Blind) break;
			if(trap->tseen) {
			    You("see a pit below you.");
			} else {
			    pline("A pit opens up under you!");
			    You("don't fall in!");
			}
			break;
		    }
		    You("fall into a pit!");
#ifdef POLYSELF
		    if (!passes_walls(uasmon))
#endif
			u.utrap = rn1(6,2);
		    u.utraptype = TT_PIT;
		    losehp(rnd(6),"fell into a pit", NO_KILLER_PREFIX);
		    if (Punished && !carried(uball)) {
			unplacebc();
			ballfall();
			placebc();
		    }
		    selftouch("Falling, you");
		    exercise(A_STR, FALSE);
		    vision_full_recalc = 1;	/* vision limits change */
		    break;
		case SPIKED_PIT:
		    if (Levitation
#ifdef POLYSELF
			|| is_flyer(uasmon) || is_clinger(uasmon)
#endif
			) {
			if(Blind) break;
			pline("A pit full of spikes opens up under you!");
			You("don't fall in!");
			break;
		    }
		    You("fall into a pit!");
		    You("land on a set of sharp iron spikes!");
#ifdef POLYSELF
		    if (!passes_walls(uasmon))
#endif
			u.utrap = rn1(6,2);
		    u.utraptype = TT_PIT;
		    losehp(rnd(10),"fell into a pit of iron spikes",
			NO_KILLER_PREFIX);
		    if(!rn2(6)) poisoned("spikes",A_STR,"fall onto poison spikes",8);
		    if (Punished && !carried(uball)) {
			unplacebc();
			ballfall();
			placebc();
		    }
		    selftouch("Falling, you");
		    vision_full_recalc = 1;	/* vision limits change */
		    exercise(A_STR, FALSE);
		    exercise(A_DEX, FALSE);
		    break;

		case TRAPDOOR:
		    if(!Can_fall_thru(&u.uz))
			panic("Trapdoors cannot exist on this level.");
		    fall_through(TRUE);
		    break;

		case TELEP_TRAP:
		    if(In_endgame(&u.uz) || Antimagic) {
			if(Antimagic)
			    shieldeff(u.ux, u.uy);
			You("feel a wrenching sensation.");
#ifdef WALKIES
		    } else if(!next_to_u()) {
			    You(shudder_for_moment);
#endif
		    } else if(trap->once) {
			deltrap(trap);
			newsym(u.ux,u.uy);	/* get rid of trap symbol */
			vtele();
		    } else
			tele();
		    break;
		case LEVEL_TELEP:
		    You("%s onto a level teleport trap!",
			  Levitation ? (const char *)"float" :
#ifdef POLYSELF
			  locomotion(uasmon, "step"));
#else
			  (const char *)"step");
#endif
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
		    }
		    if(Antimagic || In_endgame(&u.uz)) {
			You("feel a wrenching sensation.");
			break;
		    }
		    if(!Blind)
		        You("are momentarily blinded by a flash of light.");
		    else
			You("are momentarily disoriented.");
		    deltrap(trap);
		    newsym(u.ux,u.uy);	/* get rid of trap symbol */
		    level_tele();
		    break;

		case WEB: /* Our luckless player has stumbled into a web. */
#ifdef POLYSELF
		    if (amorphous(uasmon)) {
		        if (acidic(uasmon) || u.umonnum == PM_GELATINOUS_CUBE)
			{
			    deltrap(trap);
			    newsym(u.ux,u.uy);/* update position */
			    You("dissolve a spider web.");
			    break;
			}
			You("flow through a spider web.");
			break;
		    }
		    if (uasmon->mlet == S_SPIDER) {
			pline("There is a spider web here.");
			break;
		    }
#endif
		    You("%s into a spider web!",
			  Levitation ? (const char *)"float" :
#ifdef POLYSELF
			  locomotion(uasmon, "stumble"));
#else
			  (const char *)"stumble");
#endif
		    u.utraptype = TT_WEB;

		    /* Time stuck in the web depends on your strength. */
		    {
		    register int str = ACURR(A_STR);

		    if (str == 3) u.utrap = rn1(6,6);
		    else if (str < 6) u.utrap = rn1(6,4);
		    else if (str < 9) u.utrap = rn1(4,4);
		    else if (str < 12) u.utrap = rn1(4,2);
		    else if (str < 15) u.utrap = rn1(2,2);
		    else if (str < 18) u.utrap = rnd(2);
		    else if (str < 69) u.utrap = 1;
		    else {
			u.utrap = 0;
			You("tear through the web!");
			deltrap(trap);
			newsym(u.ux,u.uy);	/* get rid of trap symbol */
		    }
		    }
		    break;

		case STATUE_TRAP:
		    deltrap(trap);
		    newsym(u.ux,u.uy);	/* get rid of trap symbol */
		    for (otmp = level.objects[u.ux][u.uy];
						otmp; otmp = otmp->nexthere)
			if (otmp->otyp == STATUE)
			    if ((mtmp = makemon(&mons[otmp->corpsenm],
						u.ux, u.uy)) != 0) {
				pline("The statue comes to life!");
				/* mimic statues become seen mimics */
				if(mtmp->m_ap_type) seemimic(mtmp);
				delobj(otmp);
				break;
			    }
		    break;

		case MAGIC_TRAP:	    /* A magic trap. */
		    if (!rn2(30)) {
			deltrap(trap);
			newsym(u.ux,u.uy);	/* update position */
			You("are caught in a magical explosion!");
			losehp(rnd(10), "magical explosion", KILLED_BY_AN);
			Your("body absorbs some of the magical energy!");
			u.uen = (u.uenmax += 2);
		    } else domagictrap();
		    break;

		case ANTI_MAGIC:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			You("feel momentarily lethargic.");
		    } else drain_en(rnd((int)u.ulevel) + 1);
		    break;

#ifdef POLYSELF
		case POLY_TRAP:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			You("feel momentarily different.");
			/* Trap did nothing; don't remove it --KAA */
		    } else {
			deltrap(trap);	/* delete trap before polymorph */
			newsym(u.ux,u.uy);	/* get rid of trap symbol */
			You("feel a change coming over you.");
			polyself();
		    }
		    break;
#endif

		case LANDMINE: {
		    if (Levitation
#ifdef POLYSELF
					|| is_flyer(uasmon)
#endif
								) {
			You("see a trigger in a pile of soil below you.");
			if (rn2(3)) break;
			pline("KAABLAMM!!!  The air currents set it off!");
		    } else {
			pline("KAABLAMM!!!  You triggered a land mine!");
			set_wounded_legs(LEFT_SIDE, rn1(35, 41));
			set_wounded_legs(RIGHT_SIDE, rn1(35, 41));
			exercise(A_DEX, FALSE);
		    }
		    losehp(rnd(16), "land mine", KILLED_BY_AN);
		    /* wake everything on the level */
		    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			if(mtmp->msleep) mtmp->msleep = 0;
		    }
		    deltrap(t_at(u.ux, u.uy));	/* mines only explode once */
		    newsym(u.ux,u.uy);		/* get rid of trap symbol */
		  }
		    break;

                case MAGIC_PORTAL:
#ifdef WALKIES
		    if(!next_to_u())
			    You(shudder_for_moment);
		    else
#endif
			    domagicportal(trap);
	            break;

		default:
		    impossible("You hit a trap of type %u", trap->ttyp);
	    }
	}
}

#endif /* OVLB */

#ifdef WALKIES

STATIC_DCL boolean FDECL(teleport_pet, (struct monst *));

#ifdef OVLB

STATIC_OVL boolean
teleport_pet(mtmp)
register struct monst *mtmp;
{
	register struct obj *otmp;

	if(mtmp->mleashed) {
	    otmp = get_mleash(mtmp);
	    if(!otmp)
		impossible("%s is leashed, without a leash.", Monnam(mtmp));
	    if(otmp->cursed) {
# ifdef SOUNDS
		yelp(mtmp);
# endif
		return FALSE;
	    } else {
		Your("leash goes slack.");
		m_unleash(mtmp);
		return TRUE;
	    }
	}
	return TRUE;
}

#endif /* OVLB */

#endif /* WALKIES */

#ifdef OVLB

void
seetrap(trap)
	register struct trap *trap;
{
	if(!trap->tseen) {
	    trap->tseen = 1;
	    newsym(trap->tx, trap->ty);
	}
}

#endif /* OVLB */
#ifdef OVL1

int
mintrap(mtmp)
register struct monst *mtmp;
{
	register struct trap *trap = t_at(mtmp->mx, mtmp->my);
	boolean trapkilled = FALSE;
	struct permonst *mptr = mtmp->data;
	struct obj *otmp;

	if(!trap) {
		mtmp->mtrapped = 0;	/* perhaps teleported? */
	} else if (mtmp->mtrapped) {	/* was in trap */
		if(!rn2(40)) {
			if (sobj_at(BOULDER, mtmp->mx, mtmp->my) && 
			    ((trap->ttyp == PIT) || 
			     (trap->ttyp == SPIKED_PIT))) {
				if (!rn2(2)) {
					mtmp->mtrapped = 0;
					fill_pit(mtmp->mx, mtmp->my);
				}
			} else
				mtmp->mtrapped = 0;
		}
	} else {
	    register int tt = trap->ttyp;

	    /* A bug fix for dumb messages by ab@unido.
	     */
	    int in_sight = canseemon(mtmp);

	    if(mtmp->mtrapseen & (1 << tt)) {
		/* it has been in such a trap - perhaps it escapes */
		if(rn2(4)) return(0);
	    }
	    mtmp->mtrapseen |= (1 << tt);
	    switch (tt) {
		case ARROW_TRAP:
			otmp = mksobj(ARROW, TRUE, FALSE);
			otmp->quan = 1L;
			otmp->owt = weight(otmp);
			if(in_sight) seetrap(trap);
			if(thitm(8, mtmp, otmp, 0)) trapkilled = TRUE;
			break;
		case DART_TRAP:
			otmp = mksobj(DART, TRUE, FALSE);
			otmp->quan = 1L;
			otmp->owt = weight(otmp);
			if (!rn2(6)) otmp->opoisoned = 1;
			if(in_sight) seetrap(trap);
			if(thitm(7, mtmp, otmp, 0)) trapkilled = TRUE;
			break;
		case ROCKTRAP:
			otmp = mksobj(ROCK, TRUE, FALSE);
			otmp->quan = 1L;
			otmp->owt = weight(otmp);
			if(in_sight) seetrap(trap);
			if(!is_whirly(mptr) && !passes_walls(mptr) &&
			   thitm(0, mtmp, otmp, d(2, 6)))
			    trapkilled = TRUE;
			break;

		case SQKY_BOARD: {
			register struct monst *ztmp = fmon;

			if(is_flyer(mptr)) break;
			/* stepped on a squeaky board */
			if (in_sight) {
			    pline("A board beneath %s squeaks loudly.", mon_nam(mtmp));
			    seetrap(trap);
			} else
			   You("hear a distant squeak.");
			/* wake up nearby monsters */
			while(ztmp) {
			    if(dist2(mtmp->mx,mtmp->my,ztmp->mx,ztmp->my) < 40)
				if(ztmp->msleep) ztmp->msleep = 0;
			    ztmp = ztmp->nmon;
			}
			break;
		}

		case BEAR_TRAP:
			if(mptr->msize > MZ_SMALL &&
			   !amorphous(mptr) && !is_flyer(mptr)) {
			    mtmp->mtrapped = 1;
			    if(in_sight) {
				pline("%s is caught in a bear trap!",
				      Monnam(mtmp));
				seetrap(trap);
			    } else {
				if((mptr == &mons[PM_OWLBEAR]
				    || mptr == &mons[PM_BUGBEAR])
				   && flags.soundok)
				    You("hear the roaring of an angry bear!");
			    }
			}
			break;

		case SLP_GAS_TRAP:
			if(!resists_sleep(mptr) &&
			   !mtmp->msleep && mtmp->mcanmove) {
				mtmp->mcanmove = 0;
				mtmp->mfrozen = rnd(25);
				if (in_sight) {
				    pline("%s suddenly falls asleep!",
								Monnam(mtmp));
				    seetrap(trap);
				}
			}
			break;

		case RUST_TRAP:
			if (in_sight) {
			    pline("A gush of water hits %s!", mon_nam(mtmp));
			    seetrap(trap);
			}
			if (mptr == &mons[PM_IRON_GOLEM]) {
				if (in_sight)
				    pline("%s falls to pieces!", Monnam(mtmp));
				else if(mtmp->mtame)
				    pline("May %s rust in peace.",
								mon_nam(mtmp));
				mondied(mtmp);
#ifdef MUSE
				if (mtmp->mhp <= 0)
#endif
					trapkilled = TRUE;
			} else if (mptr == &mons[PM_GREMLIN] && rn2(3)) {
				struct monst *mtmp2 = clone_mon(mtmp);

				if (mtmp2) {
				    mtmp2->mhpmax = (mtmp->mhpmax /= 2);
				    if(in_sight)
					pline("%s multiplies.", Monnam(mtmp));
				}
			}
			break;

		case FIRE_TRAP:
			if (in_sight)
			 pline("A tower of flame bursts from the %s under %s!",
				surface(mtmp->mx,mtmp->my), mon_nam(mtmp));
			if(resists_fire(mptr)) {
			    if (in_sight) {
				shieldeff(mtmp->mx,mtmp->my);
				pline("%s is uninjured.", Monnam(mtmp));
			    }
			} else {
			    int num=rnd(6);

			    if (thitm(0, mtmp, (struct obj *)0, num))
				trapkilled = TRUE;
			    else mtmp->mhpmax -= num;
			}
			(void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
			(void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
			(void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
			if (is_ice(mtmp->mx,mtmp->my))
			    melt_ice(mtmp->mx,mtmp->my);
			if (in_sight) seetrap(trap);
			break;

		case PIT:
		case SPIKED_PIT:
			if ( !is_flyer(mptr) && 
			     (!mtmp->wormno || (count_wsegs(mtmp) < 6)) &&
			     !is_clinger(mptr) ) {
				if (!passes_walls(mptr))
				    mtmp->mtrapped = 1;
				if(in_sight) {
				    pline("%s falls into a pit!", Monnam(mtmp));
				    seetrap(trap);
				}
#ifdef MUSE
				mselftouch(mtmp, "Falling, ", FALSE);
				if(mtmp->mhp <= 0 ||
					thitm(0, mtmp, (struct obj *)0,
					 rnd((tt==PIT) ? 6 : 10)))
#else
				if(thitm(0, mtmp, (struct obj *)0,
					 rnd((tt==PIT) ? 6 : 10)))
#endif
				    trapkilled = TRUE;
			}
			break;

		case TRAPDOOR:
			if(!Can_fall_thru(&u.uz))
			    panic("Trapdoors cannot exist on this level.");

			if (is_flyer(mptr) || mptr == &mons[PM_WUMPUS] ||
			    (mtmp->wormno && count_wsegs(mtmp) > 5)) break;
			/* Fall through */
		case LEVEL_TELEP:
		case MAGIC_PORTAL:
			if (mtmp == u.ustuck)	/* probably a vortex */
			    break;		/* temporary? kludge */
#ifdef WALKIES
			if (teleport_pet(mtmp))
#endif
			{
			    d_level tolevel;
			    int migrate_typ = 0;

			    if (tt == TRAPDOOR) {
				if (Is_stronghold(&u.uz)) {
				    assign_level(&tolevel, &valley_level);
				} else if (Is_botlevel(&u.uz)) {
				    pline("%s avoids the trap.", Monnam(mtmp));
				    break;
				} else {
				    get_level(&tolevel, depth(&u.uz) + 1);
				}
			    } else if (tt == MAGIC_PORTAL) {
				if (In_endgame(&u.uz) &&
				    (mon_has_amulet(mtmp) ||
					is_home_elemental(mptr))) {
				    if (in_sight && mptr->mlet != S_ELEMENTAL) {
				     pline("%s seems to shimmer for a moment.",
					   Monnam(mtmp));
					seetrap(trap);
				    }
				    break;
				} else {
				    assign_level(&tolevel, &trap->dst);
				    migrate_typ = 6; /* see dog.c */
				}
			    } else { /* (tt == LEVEL_TELEP) */
				register int nlev;

				if (mon_has_amulet(mtmp)) break;
				nlev = rnd(3);
				if (!rn2(2)) nlev = -nlev;
				nlev += dunlev(&u.uz);
				if (nlev > dunlevs_in_dungeon(&u.uz)) {
				    nlev = dunlevs_in_dungeon(&u.uz);
				    /* teleport up if already on bottom */
				    if (Is_botlevel(&u.uz)) nlev -= rnd(3);
				}
				if (nlev < 1) {
				    nlev = 1;
				    if (dunlev(&u.uz) == 1) {
					nlev += rnd(3);
					if (nlev > dunlevs_in_dungeon(&u.uz)) 
					    nlev = dunlevs_in_dungeon(&u.uz);
				    }
				}
				/* can't seem to go anywhere    */
				/* (possible in short dungeons) */
				if (nlev == dunlev(&u.uz)) {
				    rloc(mtmp);
				    break;
				}
				nlev += dungeons[u.uz.dnum].depth_start;
				get_level(&tolevel, nlev);
			    }

			    if (in_sight) {
				pline("Suddenly, %s disappears out of sight.",
				      mon_nam(mtmp));
				seetrap(trap);
			    }
			    migrate_to_level(mtmp, 
					     ledger_no(&tolevel),
					     migrate_typ);
			    return(3);	/* no longer on this level */
			}
			break;

		case TELEP_TRAP:
			if(tele_restrict(mtmp)) break;
#ifdef WALKIES
			if(teleport_pet(mtmp)) {
#endif
			    /* Note: don't remove the trap if a vault.  Other-
			     * wise the monster will be stuck there, since 
			     * the guard isn't going to come for it...
			     */
			    if (in_sight) {
				pline("%s suddenly disappears!", 
				                      Monnam(mtmp));
				seetrap(trap);
			    }
			    if (trap->once) vloc(mtmp);
			    else rloc(mtmp);
#ifdef WALKIES
			}
#endif
			break;

		case WEB:
			/* Monster in a web. */
			if (mptr->mlet == S_SPIDER) break;
			if (amorphous(mptr)) {
			    if (acidic(mptr) ||
				 mptr == &mons[PM_GELATINOUS_CUBE]) {
				if (in_sight)
				    pline("%s dissolves a spider web.",
						Monnam(mtmp));
				deltrap(trap);
				break;
			    }
			    if (in_sight)
				pline("%s flows through a spider web.",
						Monnam(mtmp));
			    break;
			}			
			switch (monsndx(mptr)) {
			    case PM_FIRE_ELEMENTAL:
				if (in_sight)
				    pline("%s burns a spider web!", Monnam(mtmp));
				deltrap(trap);
				break;
			    case PM_OWLBEAR: /* Eric Backus */
			    case PM_BUGBEAR:
				if (!in_sight) {
				    You("hear the roaring of a confused bear!");
				    mtmp->mtrapped = 1;
				    break;
				}
				/* fall though */
			    default:
				if (in_sight)
				    pline("%s is caught in a spider web.",
								Monnam(mtmp));
				mtmp->mtrapped = 1;
				break;
			}
			break;

		case STATUE_TRAP:
			break;

		case MAGIC_TRAP:
			/* A magic trap.  Monsters immune. */
			break;
		case ANTI_MAGIC:
			break;

		case LANDMINE: {
			register struct monst *mntmp = fmon;

			if(rn2(3))
				break; /* monsters usually don't set it off */
			if(is_flyer(mptr)) {
				if (in_sight) {
	pline("A trigger appears in a pile of soil below %s.", mon_nam(mtmp));
					seetrap(trap);
				}
				if (rn2(3)) break;
				if (in_sight)
					pline("The air currents set it off!");
			} else if(in_sight)
			    pline("KAABLAMM!!!  %s triggers a land mine!",
				  Monnam(mtmp));
			if (!in_sight)
				pline("Kaablamm!  You hear an explosion in the distance!");
			deltrap(trap);
			if(thitm(0, mtmp, (struct obj *)0, rnd(16)))
				trapkilled = TRUE;
			/* wake everything on the level */
			while(mntmp) {
				if(mntmp->msleep)
					mntmp->msleep = 0;
				mntmp = mntmp->nmon;
			}
			if (unconscious()) {
				multi = -1;
				nomovemsg="The explosion awakens you!";
			}
			break;
		}

#ifdef POLYSELF
		case POLY_TRAP:
		    if(!resist(mtmp, WAND_CLASS, 0, NOTELL)) {
			(void) newcham(mtmp, (struct permonst *)0);
			if (in_sight) seetrap(trap);
		    }
		    break;
#endif

		default:
			impossible("Some monster encountered a strange trap of type %d.", tt);
	    }
	}
	if(trapkilled) return 2;
	return mtmp->mtrapped;
}

#endif /* OVL1 */
#ifdef OVLB

void
selftouch(arg)
const char *arg;
{
	if(uwep && uwep->otyp == CORPSE && uwep->corpsenm == PM_COCKATRICE
#ifdef POLYSELF
			&& !resists_ston(uasmon)
#endif
	){
		pline("%s touch the cockatrice corpse.", arg);
#ifdef POLYSELF
		if(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))
		    return;
#endif
		You("turn to stone...");
		killer_format = KILLED_BY;
		killer = "touching a cockatrice corpse";
		done(STONING);
	}
}

#ifdef MUSE
void
mselftouch(mon,arg,byplayer)
struct monst *mon;
const char *arg;
boolean byplayer;
{
	struct obj *mwep = MON_WEP(mon);

	if (mwep && mwep->otyp == CORPSE && mwep->corpsenm == PM_COCKATRICE
			&& !resists_ston(mon->data)) {
		if (cansee(mon->mx, mon->my)) {
			pline("%s%s touches the cockatrice corpse.",
			    arg ? arg : "", arg ? mon_nam(mon) : Monnam(mon));
			pline("%s turns to stone.", Monnam(mon));
		}
		if (poly_when_stoned(mon->data)) {
			mon_to_stone(mon);
			return;
		}
		if (byplayer) {
			stoned = TRUE;
			xkilled(mon,0);
		} else monstone(mon);
	}
}
#endif

void
float_up()
{
	if(u.utrap) {
		if(u.utraptype == TT_PIT) {
			u.utrap = 0;
			You("float up, out of the pit!");
			vision_full_recalc = 1;	/* vision limits change */
			fill_pit(u.ux, u.uy);
		} else if (u.utraptype == TT_INFLOOR) {
			Your("body pulls upward, but your %s are still stuck.",
			     makeplural(body_part(LEG)));
		} else {
			You("float up, only your %s is still stuck.",
				body_part(LEG));
		}
	}
	else if(Is_waterlevel(&u.uz))
		pline("It feels as though you'd lost some weight.");
	else if(u.uinwater)
		spoteffects();
	else if (Hallucination)
		pline("Up, up, and awaaaay!  You're walking on air!");
	else if(Is_airlevel(&u.uz))
		You("gain control over your movements.");
	else
		You("start to float in the air!");
}

void
fill_pit(x, y)
int x, y;
{
	struct obj *otmp;
	struct trap *t;

	if ((t = t_at(x, y)) && 
	    ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT)) &&
	    (otmp = sobj_at(BOULDER, x, y))) {
		freeobj(otmp);
		(void) flooreffects(otmp, x, y, "settle");
	}
}

int
float_down()
{
	register struct trap *trap = (struct trap *)0;
	boolean no_msg = FALSE;

	if(Levitation) return(0); /* maybe another ring/potion/boots */

	if (Punished && !carried(uball) &&
	    (is_pool(uball->ox, uball->oy) || 
	     ((trap = t_at(uball->ox, uball->oy)) && 
	      ((trap->ttyp == PIT) || (trap->ttyp == SPIKED_PIT) ||
	       (trap->ttyp == TRAPDOOR))))) {
			u.ux0 = u.ux;
			u.uy0 = u.uy;
			u.ux = uball->ox;
			u.uy = uball->oy;
			movobj(uchain, uball->ox, uball->oy);
			newsym(u.ux0, u.uy0);
			vision_full_recalc = 1;	/* in case the hero moved. */
	}
	/* check for falling into pool - added by GAN 10/20/86 */
#ifdef POLYSELF
	if(!is_flyer(uasmon)) {
#endif
		/* kludge alert:
		 * drown() and lava_effects() print various messages almost
		 * every time they're called which conflict with the "fall
		 * into" message below.  Thus, we want to avoid printing
		 * confusing, duplicate or out-of-order messages.
		 * Use knowledge of the two routines as a hack -- this
		 * should really handled differently -dlc
		 */
		if(is_pool(u.ux,u.uy) && !Wwalking && !u.uinwater)
			no_msg = drown();

		if(is_lava(u.ux,u.uy)) {
			(void) lava_effects();
			no_msg = TRUE;
		}
#ifdef POLYSELF
	}
#endif
	if (!trap) {
		if(Is_airlevel(&u.uz))
			You("begin to tumble in place.");
		else if (Is_waterlevel(&u.uz) && !no_msg)
			You("feel heavier.");
		/* u.uinwater msgs already in spoteffects()/drown() */
		else if (!u.uinwater && !no_msg) {
			if (Hallucination)
				pline("Bummer!  You've %s.",
				      is_pool(u.ux,u.uy) ?
					"splashed down" : "hit the ground");
			else
				You("float gently to the %s.",
				    surface(u.ux, u.uy));
		}
		trap = t_at(u.ux,u.uy);
	}

	if(trap)
		switch(trap->ttyp) {
		case STATUE_TRAP:
			break;
		case TRAPDOOR:
			if(!Can_fall_thru(&u.uz) || u.ustuck)
				break;
			/* fall into next case */
		default:
			dotrap(trap);
	}
	if(!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz))
	    pickup(1);
	return 0;
}


void
tele()
{
	coord cc;

	/* Disable teleportation in stronghold && Vlad's Tower */
	if(level.flags.noteleport) {
#ifdef WIZARD
		if (!wizard) {
#endif
		    pline("A mysterious force prevents you from teleporting!");
		    return;
#ifdef WIZARD
		}
#endif
	}

	/* don't show trap if "Sorry..." */
	if(!Blinded) make_blinded(0L,FALSE);

	if((u.uhave.amulet || Is_wiz1_level(&u.uz) || Is_wiz2_level(&u.uz) ||
	       Is_wiz3_level(&u.uz)) && !rn2(3)) {
	    You("feel disoriented for a moment.");
	    return;
	}
	if(Teleport_control
#ifdef WIZARD
			    || wizard
#endif
					) {
	    if (unconscious()) {
		pline("Being unconscious, you cannot control your teleport.");
	    } else {
		    pline("To what position do you want to be teleported?");
		    cc.x = u.ux;
		    cc.y = u.uy;
		    getpos(&cc, TRUE, "the desired position");/* force valid*/
                    if(cc.x == -10) return; /* abort */
		    /* possible extensions: introduce a small error if
		       magic power is low; allow transfer to solid rock */
		    if(teleok(cc.x, cc.y, FALSE)){
			teleds(cc.x, cc.y);
			return;
		    }
		    pline("Sorry...");
		}
	}

	(void) safe_teleds();
}

boolean
tele_restrict(mon)
struct monst *mon;
{
	if(level.flags.noteleport) {
		if (canseemon(mon))
		    pline("A mysterious force prevents %s from teleporting!",
			mon_nam(mon));
		return TRUE;
	}
	return FALSE;
}

void
teleds(nux, nuy)
register int nux,nuy;
{
	if (Punished) unplacebc();
	u.utrap = 0;
	u.ustuck = 0;
	u.ux0 = u.ux;
	u.uy0 = u.uy;
	u.ux = nux;
	u.uy = nuy;
	fill_pit(u.ux0, u.uy0); /* do this now so that cansee() is correct */
#ifdef POLYSELF
	if (hides_under(uasmon))
		u.uundetected = OBJ_AT(nux, nuy);
	else 
		u.uundetected = 0;
	if (u.usym == S_MIMIC_DEF) u.usym = S_MIMIC;
#endif
	if(u.uswallow){
		u.uswldtim = u.uswallow = 0;
		docrt();
	}
	if(Punished) placebc();
	initrack(); /* teleports mess up tracking monsters without this */
	/*
	 *  Make sure the hero disappears from the old location.  This will
	 *  not happen if she is teleported within sight of her previous
	 *  location.  Force a full vision recalculation because the hero
	 *  is now in a new location.
	 */
	newsym(u.ux0,u.uy0);
	vision_full_recalc = 1;
	nomul(0);
	spoteffects();
}

int
dotele()
{
	struct trap *trap;
	boolean castit = FALSE;
	register int sp_no = 0, energy;

	trap = t_at(u.ux, u.uy);
	if (trap && (!trap->tseen || trap->ttyp != TELEP_TRAP))
		trap = 0;

	if (trap) {
		if (trap->once) {
			pline("This is a vault teleport, usable once only.");
			if (yn("Jump in?") == 'n')
				trap = 0;
			else {
				deltrap(trap);
				newsym(u.ux, u.uy);
			}
		}
		if (trap)
#ifdef POLYSELF
			You("%s onto the teleportation trap.",
			    locomotion(uasmon, "jump"));
#else
			You("jump onto the teleportation trap.");
#endif
	}
	if(!trap && (!Teleportation ||
	   (u.ulevel < (pl_character[0] == 'W' ? 8 : 12)
#ifdef POLYSELF
	    && !can_teleport(uasmon)
#endif
	   )
	  )) {
		/* Try to use teleport away spell. */
		castit = objects[SPE_TELEPORT_AWAY].oc_name_known;
		if (castit) {
		    for (sp_no = 0; sp_no < MAXSPELL &&
				spl_book[sp_no].sp_id != NO_SPELL &&
				spl_book[sp_no].sp_id != SPE_TELEPORT_AWAY; sp_no++);

		    if (sp_no == MAXSPELL ||
			spl_book[sp_no].sp_id != SPE_TELEPORT_AWAY)
			    castit = FALSE;
		}
#ifdef WIZARD
		if (!wizard) {
#endif
		    if (!castit) {
			if (!Teleportation)
			    You("don't know that spell.");
			else You("are not able to teleport at will.");
			return(0);
		    }
#ifdef WIZARD
		}
#endif
	}

	if(!trap && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
#ifdef WIZARD
		if (!wizard) {
#endif
			You("lack the strength %s.",
			    castit ? "for a teleport spell" : "to teleport");
			return 1;
#ifdef WIZARD
		}
#endif
	}

	energy = objects[SPE_TELEPORT_AWAY].oc_level * 7 / 2 - 2;

	if(!trap && u.uen <= energy) {
#ifdef WIZARD
		if (!wizard) {
#endif
			You("lack the energy %s.",
			    castit ? "for a teleport spell" : "to teleport");
			return 1;
#ifdef WIZARD
		} else u.uen = energy;
#endif
	}

	if(!trap &&
	  check_capacity("Your concentration falters from carrying so much."))
	    return 1;

	if (castit) {
		exercise(A_WIS, TRUE);
		if (spelleffects(++sp_no, TRUE))
			return(1);
		else
#ifdef WIZARD
		    if (!wizard)
#endif
			return(0);
	} else if (!trap)
		u.uen -= energy;

#ifdef WALKIES
	if(next_to_u()) {
#endif
		if (trap && trap->once) vtele();
		else tele();
#ifdef WALKIES
		(void) next_to_u();
	} else {
		You(shudder_for_moment);
		return(0);
	}
#endif
	if (!trap) morehungry(100);
	return(1);
}


void
level_tele()
{
	register int newlev;
	d_level newlevel;

	if((u.uhave.amulet || In_endgame(&u.uz))
#ifdef WIZARD
						&& !wizard
#endif
							) {
	    You("feel very disoriented for a moment.");
	    return;
	}
	if(Teleport_control
#ifdef WIZARD
	   || wizard
#endif
		) {
	    char buf[BUFSZ];

	    do {
		getlin("To what level do you want to teleport? [type a number]",
			buf);
		if (!strcmp(buf,"\033"))	/* cancelled */
		    return;
	    } while(!digit(buf[0]) && (buf[0] != '-' || !digit(buf[1])));
	    newlev = atoi(buf);

	    /* no dungeon escape via this route */
	    if(newlev == 0) {
	        if(ynq("Go to Nowhere.  Are you sure?") != 'y') return;
	        You("scream in agony as your body begins to warp...");
		display_nhwindow(WIN_MESSAGE, FALSE);
	        You("cease to exist.");
	        killer_format = NO_KILLER_PREFIX;
	        killer = "committed suicide";
	        done(DIED);
		return;  
	    }
#ifdef MULDGN
	    /* if in Knox and the requested level > 0, stay put.
	     * we let negative values requests fall into the "heaven" loop.
	     */
	    if(Is_knox(&u.uz) && newlev > 0) {
	        You(shudder_for_moment);
		return;
	    }
	    /* if in Quest, the player sees "Home 1", etc., on the status
	     * line, instead of the logical depth of the level.  controlled
	     * level teleport request is likely to be relativized to the
	     * status line, and consequently it should be incremented to 
	     * the value of the logical depth of the target level.
	     *
	     * we let negative values requests fall into the "heaven" loop.
	     */
	    if(In_quest(&u.uz) && newlev > 0)
	        newlev = newlev + dungeons[u.uz.dnum].depth_start - 1;
#endif
	} else { /* involuntary level tele */
#ifdef MULDGN
	    if(Is_knox(&u.uz)) {
	        You(shudder_for_moment);
		return;
	    }
#endif
	    if(rn2(5)) newlev = rnd((int)depth(&u.uz) + 3);
	    else {
		You(shudder_for_moment);
		return; 
	    }
	    if(newlev == depth(&u.uz)) {
		/* if in a single-level dungeon... */
		if(dunlevs_in_dungeon(&u.uz) == 1) {
		    You(shudder_for_moment);
		    return; 
		}
		else if(dunlev(&u.uz) == 1) newlev++;
		else if(dunlev(&u.uz) == dunlevs_in_dungeon(&u.uz)) newlev--;
	        else if(In_hell(&u.uz)) newlev--;
		else newlev++;
	    } 
	}

#ifdef WALKIES
	if(!next_to_u()) {
		You(shudder_for_moment);
		return;
	}
#endif
	if(newlev < 0) {
		if(newlev <= -10) {
			You("arrive in heaven.");
			verbalize("Thou art early, but we'll admit thee.");
			killer_format = NO_KILLER_PREFIX;
			killer = "went to heaven prematurely";
			done(DIED);
			return;
		} else	if (newlev == -9) {
			You("feel deliriously happy. ");
			pline("(In fact, you're on Cloud 9!) ");
			display_nhwindow(WIN_MESSAGE, FALSE);
		} else
			You("are now high above the clouds...");

		if(Levitation
#ifdef POLYSELF
		   || is_floater(uasmon)
#endif
		   ) {
		    You("float gently down to earth.");
		    u.uz.dnum = 0; /* he might have been in another dgn */
		    newlev = 1;
		}
#ifdef POLYSELF
		else if(is_flyer(uasmon)) {
		    You("fly down to earth.");
		    u.uz.dnum = 0; /* he might have been in another dgn */
		    newlev = 1;
		}
#endif
		else {
		    d_level save_dlevel;
		    
		    assign_level(&save_dlevel, &u.uz);
		    pline("Unfortunately, you don't know how to fly.");
		    You("plummet a few thousand feet to your death.");
		    u.uz.dnum = 0;
		    u.uz.dlevel = 0;
		    killer_format = NO_KILLER_PREFIX;
		    killer =
    self_pronoun("teleported out of the dungeon and fell to %s death","his");
		    done(DIED);
		    assign_level(&u.uz, &save_dlevel);
		    flags.botl = 1;
		    return;
		}
	}

# ifdef WIZARD
	if (In_endgame(&u.uz)) {	/* must already be wizard */
	    newlevel.dnum = u.uz.dnum;
	    newlevel.dlevel = newlev;
	    goto_level(&newlevel, FALSE, FALSE, FALSE);
	    return;
	}
# endif

	/* calls done(ESCAPED) if newlevel==0 */
	if(u.uz.dnum == medusa_level.dnum &&
	    newlev >= dungeons[u.uz.dnum].depth_start +
						dunlevs_in_dungeon(&u.uz)) {

		goto_hell(TRUE, FALSE);
	} else {
	    /* if invocation did not yet occur, teleporting into
	     * the last level of Gehennom is forbidden.
	     */
	    if(Inhell && !u.uevent.invoked &&
			newlev >= (dungeons[u.uz.dnum].depth_start +
					dunlevs_in_dungeon(&u.uz) - 1)) {
		newlev = dungeons[u.uz.dnum].depth_start +
					dunlevs_in_dungeon(&u.uz) - 2;
		pline("Sorry...");
	    }
#ifdef MULDGN
	    /* no teleporting out of quest dungeon */
	    if(In_quest(&u.uz) && newlev < depth(&qstart_level))
		newlev = depth(&qstart_level);
#endif
	    /* the player thinks of levels purely in logical terms, so
	     * we must translate newlev to a number relative to the
	     * current dungeon.
  	     */
	    get_level(&newlevel, newlev);
	    goto_level(&newlevel, FALSE, FALSE, FALSE);
	}
}

static void
dofiretrap()
{

	register int num;

	/* changed to be in conformance with
	 * SCR_FIRE by GAN 11/02/86
	 */

	pline("A tower of flame bursts from the %s!", surface(u.ux,u.uy));
	if(Fire_resistance) {
		shieldeff(u.ux, u.uy);
		You("are uninjured.");
	} else {
		num = rnd(6);
		u.uhpmax -= num;
		losehp(num,"burst of flame", KILLED_BY_AN);
	}
	destroy_item(SCROLL_CLASS, AD_FIRE);
	destroy_item(SPBOOK_CLASS, AD_FIRE);
	destroy_item(POTION_CLASS, AD_FIRE);
	if (is_ice(u.ux, u.uy))
		melt_ice(u.ux, u.uy);
}

static void
domagicportal(ttmp)
register struct trap *ttmp;
{
	struct d_level target_level;

	/* if landed from another portal, do nothing */
	/* problem: level teleport landing escapes the check */
	if(!on_level(&u.uz, &u.uz0)) return;

	You("activated a magic portal!");
	You("feel dizzy for a moment, but the sensation passes.");

	/* prevent the poor shnook, whose amulet was stolen  */
	/* while in the endgame, from accidently triggering  */
	/* the portal to the next level, and thus losing the */
	/* game                                              */
	if(In_endgame(&u.uz) && !u.uhave.amulet) return;

	target_level = ttmp->dst;
	goto_level(&target_level, FALSE, FALSE, TRUE);
}

static void
domagictrap()
{
	register int fate = rnd(20);

	/* What happened to the poor sucker? */

	if (fate < 10) {

	  /* Most of the time, it creates some monsters. */
	  register int cnt = rnd(4);

	  /* below checks for blindness added by GAN 10/30/86 */
	  if (!Blind)  {
		You("are momentarily blinded by a flash of light!");
		make_blinded((long)rn1(5,10),FALSE);
	  }  else
		You("hear a deafening roar!");
	  while(cnt--)
		(void) makemon((struct permonst *) 0, u.ux, u.uy);
	}
	else
	  switch (fate) {

	     case 10:
	     case 11:
		      /* sometimes nothing happens */
			break;
	     case 12: /* a flash of fire */
		        dofiretrap();
			break;

	     /* odd feelings */
	     case 13:	pline("A shiver runs up and down your %s!",
			      body_part(SPINE));
			break;
	     case 14:	You(Hallucination ?
				"hear the moon howling at you." :
				"hear distant howling.");
			break;
	     case 15:	You("suddenly yearn for %s.",
				Hallucination ? "Cleveland" :
						"your distant homeland");
			break;
	     case 16:   Your("pack shakes violently!");
			break;
	     case 17:	You(Hallucination ?
				"smell hamburgers." :
				"smell charred flesh.");
			break;

	     /* very occasionally something nice happens. */

	     case 19:
		    /* tame nearby monsters */
		   {   register int i,j;
		       register struct monst *mtmp;

		       /* below pline added by GAN 10/30/86 */
		       (void) adjattrib(A_CHA,1,FALSE);
		       for(i = -1; i <= 1; i++) for(j = -1; j <= 1; j++) {
			   if(!isok(u.ux+i, u.uy+j)) continue;
			   mtmp = m_at(u.ux+i, u.uy+j);
			   if(mtmp)
			       (void) tamedog(mtmp, (struct obj *)0);
		       }
		       break;
		   }

	     case 20:
		    /* uncurse stuff */
		   {  register struct obj *obj;

			/* below plines added by GAN 10/30/86 */
			You(Hallucination ?
				"feel in touch with the Universal Oneness." :
				"feel like someone is helping you.");
			for(obj = invent; obj ; obj = obj->nobj)
			       if(obj->owornmask || obj->otyp == LOADSTONE)
					uncurse(obj);
		       if(Punished) unpunish();
		       break;
		   }
	     default: break;
	  }
}

void
water_damage(obj, force, here)
register struct obj *obj;
register boolean force, here;
{
	/* Scrolls, spellbooks, potions, weapons and
	   pieces of armor may get affected by the water */
	for (; obj; obj = (here ? obj->nexthere : obj->nobj)) {

		(void) snuff_lit(obj);

		if(obj->greased) {
			if (force || !rn2(2)) obj->greased = 0;
		} else if(Is_container(obj) && !Is_box(obj) &&
			(obj->otyp != OILSKIN_SACK || (obj->cursed && !rn2(3)))) {
			water_damage(obj->cobj, force, FALSE);
		} else if(obj->oclass == SCROLL_CLASS && (force || rn2(12) > Luck)
#ifdef MAIL
			  && obj->otyp != SCR_MAIL
#endif
			  ) {
			obj->otyp = SCR_BLANK_PAPER;
		} else if(obj->oclass == SPBOOK_CLASS && (force || rn2(12) > Luck)) {
			if (obj->otyp == SPE_BOOK_OF_THE_DEAD)
				pline("Steam rises from %s.", the(xname(obj)));
			else obj->otyp = SPE_BLANK_PAPER;
		} else if(obj->oclass == POTION_CLASS && (force || rn2(12) > Luck)) {
			if (obj->spe == -1) {
				obj->otyp = POT_WATER;
				obj->blessed = obj->cursed = 0;
				obj->spe = 0;
			} else obj->spe--;
		} else if(is_rustprone(obj) && obj->oeroded < MAX_ERODE &&
			  !(obj->oerodeproof || (obj->blessed && !rnl(4))) &&
			  (force || rn2(12) > Luck)) {
			/* all metal stuff and armor except (body armor
			   protected by oilskin cloak) */
			if(obj->oclass != ARMOR_CLASS || obj != uarm ||
			   !uarmc || uarmc->otyp != OILSKIN_CLOAK ||
 			   (uarmc->cursed && !rn2(3)))
				obj->oeroded++;
		}
	}
}

/*
 * This function is potentially expensive - rolling
 * inventory list multiple times.  Luckily it's seldom needed.
 * Returns TRUE if disrobing made player unencumbered enough to
 * crawl out of the current predicament.
 */
static boolean
emergency_disrobe(lostsome)
boolean *lostsome;
{
	int invc = inv_cnt();

	while (near_capacity() > (Punished ? UNENCUMBERED : SLT_ENCUMBER)) {
		register struct obj *obj, *otmp = (struct obj *)0;
		register int i = rn2(invc);

		for (obj = invent; obj; obj = obj->nobj) {
			/*
			 * Undroppables are: body armor, boots, gloves,
			 * amulets, and rings because of the time and effort
			 * in removing them + loadstone and other cursed stuff
			 * for obvious reasons.
			 */
			if (!(obj->otyp == LOADSTONE ||
			      obj == uamul || obj == uleft || obj == uright ||
			      obj == ublindf || obj == uarm || obj == uarmc ||
			      obj == uarmg || obj == uarmf ||
#ifdef TOURIST
			      obj == uarmu ||
#endif
			      (obj->cursed && (obj == uarmh || obj == uarms)) ||
			      welded(obj)))
				otmp = obj;
			/* reached the mark and found some stuff to drop? */
			if (--i < 0 && otmp) break;

			/* else continue */
		}

		/* nothing to drop and still overweight */
		if (!otmp) return(FALSE);

		if (otmp == uarmh) (void) Helmet_off();
		else if (otmp == uarms) (void) Shield_off();
		else if (otmp == uwep) setuwep((struct obj *)0);
		*lostsome = TRUE;
		dropx(otmp);
		invc--;
	}
	return(TRUE);
}

/*
 *  return(TRUE) == player relocated
 */
boolean
drown()
{
	boolean inpool_ok = FALSE, crawl_ok;
	int i, x, y;

	/* happily wading in the same contiguous pool */
	if (u.uinwater && is_pool(u.ux-u.dx,u.uy-u.dy) &&
	    (
#ifdef POLYSELF
	     is_swimmer(uasmon) ||
#endif
	     Amphibious)) {
		/* water effects on objects every now and then */
		if (!rn2(5)) inpool_ok = TRUE;
		else return(FALSE);
	}

	if (!u.uinwater) {
	    You("%s into the water%c",
		Is_waterlevel(&u.uz) ? "plunge" : "fall",
		Amphibious ? '.' : '!');
#ifdef POLYSELF
	    if(!is_swimmer(uasmon))
#endif
		if (!Is_waterlevel(&u.uz))
		    You("sink like %s.",
			Hallucination ? "the Titanic" : "a rock");
	}

	water_damage(invent, FALSE, FALSE);

#ifdef POLYSELF
	if(u.umonnum == PM_GREMLIN && rn2(3)) {
		struct monst *mtmp;
		if ((mtmp = cloneu()) != 0) {
			mtmp->mhpmax = (u.mhmax /= 2);
			You("multiply.");
		}
	}
#endif
	if (inpool_ok) return(FALSE);
#ifdef WALKIES
	if ((i = number_leashed()) > 0) {
		pline("The leash%s slip%s loose.",
			(i > 1) ? "es" : "",
			(i > 1) ? "" : "s");
		unleash_all();
	}
#endif
	if (Amphibious
#ifdef POLYSELF
			|| is_swimmer(uasmon)
#endif
						) {
		if (Amphibious) {
			if (flags.verbose)
				pline("But you aren't drowning.");
			if (!Is_waterlevel(&u.uz))
				if (Hallucination) 
					Your("keel hits the bottom.");
				else
					You("touch bottom.");
		}
		if (Punished) {
			unplacebc();
			placebc();
		}
		u.uinwater = 1;
		under_water(1);
		vision_full_recalc = 1;
		return(FALSE);
	}
	if((Teleportation || can_teleport(uasmon)) &&
	   (Teleport_control || rn2(3) < Luck+2)) {
		You("attempt a teleport spell.");	/* utcsri!carroll */
		(void) dotele();
		if(!is_pool(u.ux,u.uy))
			return(TRUE);
	}
	crawl_ok = FALSE;
	/* look around for a place to crawl to */
	for (i = 0; i < 100; i++) {
		x = rn1(3,u.ux - 1);
		y = rn1(3,u.uy - 1);
		if (teleok(x,y,TRUE)) {
			crawl_ok = TRUE;
			goto crawl;
		}
	}
	/* one more scan */
	for (x = u.ux - 1; x <= u.ux + 1; x++)
		for (y = u.uy - 1; y <= u.uy + 1; y++)
			if (teleok(x,y,TRUE)) {
				crawl_ok = TRUE;
				goto crawl;
			}
crawl:;
	if (crawl_ok) {
		boolean lost = FALSE;
		/* time to do some strip-tease... */
		boolean succ = Is_waterlevel(&u.uz) ? TRUE :
				emergency_disrobe(&lost);

		You("try to crawl out of the water.");
		if (lost)
			You("dump some of your gear to lose weight...");
		if (succ) {
			pline("Pheew!  That was close.");
			teleds(x,y);
			return(TRUE);
		}
		/* still too much weight */
		pline("But in vain.");
	}
	u.uinwater = 1;
	You("drown.");
	killer_format = KILLED_BY_AN;
	killer = (levl[u.ux][u.uy].typ == POOL || Is_medusa_level(&u.uz)) ?
	    "pool of water" : "moat";
	done(DROWNING);
	/* oops, we're still alive.  better get out of the water. */
	if (!safe_teleds())
		while (1) {
			pline("You're still drowning.");
			done(DROWNING);
		}
	u.uinwater = 0;
	You("find yourself back %s.", Is_waterlevel(&u.uz) ?
		"in an air bubble" : "on land");
	return(TRUE);
}

void
drain_en(n)
register int n;
{
	if (!u.uenmax) return;
	You("feel your magical energy drain away!");
	u.uen -= n;
	if(u.uen < 0)  {
		u.uenmax += u.uen;
		if(u.uenmax < 0) u.uenmax = 0;
		u.uen = 0;
	}
	flags.botl = 1;
}

int
dountrap()	/* disarm a trapped object */
{
#ifdef POLYSELF
	if(nohands(uasmon)) {
	    pline("And just how do you expect to do that?");
	    return(0);
	}
#endif
	return untrap(FALSE);
}

int
untrap(force)
boolean force;
{
	register struct obj *otmp;
	register boolean confused = (Confusion > 0 || Hallucination > 0);
	register int x,y;
	int ch;
	struct trap *ttmp;
	struct monst *mtmp;
	boolean trap_skipped = FALSE;

	if(!getdir(NULL)) return(0);
	x = u.ux + u.dx;
	y = u.uy + u.dy;

	if(!u.dx && !u.dy) {
	    for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if(Is_box(otmp)) {
		    pline("There is %s here.", doname(otmp));

		    switch (ynq("Check for traps?")) {
			case 'q': return(0);
			case 'n': continue;
		    }

		    if((otmp->otrapped && (force || (!confused
				&& rn2(MAXULEV + 1 - (int)u.ulevel) < 10)))
		       || (!force && confused && !rn2(3))) {
			You("find a trap on %s!", the(xname(otmp)));
			exercise(A_WIS, TRUE);

			switch (ynq("Disarm it?")) {
			    case 'q': return(1);
			    case 'n': trap_skipped = TRUE;  continue;
			}

			if(otmp->otrapped) {
			    exercise(A_DEX, TRUE);
			    ch = ACURR(A_DEX) + u.ulevel;
			    if (pl_character[0] == 'R') ch *= 2;
			    if(!force && (confused || Fumbling ||
				rnd(75+level_difficulty()/2) > ch)) {
				(void) chest_trap(otmp, FINGER, TRUE);
			    } else {
				You("disarm it!");
				otmp->otrapped = 0;
			    }
			} else pline("That %s was not trapped.", doname(otmp));
			return(1);
		    } else {
			You("find no traps on %s.", the(xname(otmp)));
			return(1);
		    }
		}
	    if ((ttmp = t_at(x,y)) && ttmp->tseen)
		You("cannot disable this trap.");
	    else
		You(trap_skipped ? "find no other traps here."
				 : "know of no traps here.");
	    return(0);
	}

	if ((mtmp = m_at(x,y))				&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	if (!IS_DOOR(levl[x][y].typ)) {
	    if ((ttmp = t_at(x,y)) && ttmp->tseen)
		You("cannot disable that trap.");
	    else
		You("know of no traps there.");
	    return(0);
	}

	switch (levl[x][y].doormask) {
	    case D_NODOOR:
		You("%s no door there.", Blind ? "feel" : "see");
		return(0);
	    case D_ISOPEN:
		pline("This door is safely open.");
		return(0);
	    case D_BROKEN:
		pline("This door is broken.");
		return(0);
	}

	if ((levl[x][y].doormask & D_TRAPPED
	     && (force ||
		 (!confused && rn2(MAXULEV - (int)u.ulevel + 11) < 10)))
	    || (!force && confused && !rn2(3))) {
		You("find a trap on the door!");
		exercise(A_WIS, TRUE);
		if (ynq("Disarm it?") != 'y') return(1);
		if (levl[x][y].doormask & D_TRAPPED) {
		    ch = 15 +
			 (pl_character[0] == 'R') ? u.ulevel*3 :
			 u.ulevel;
		    exercise(A_DEX, TRUE);
		    if(!force && (confused || Fumbling || 
		                     rnd(75+level_difficulty()/2) > ch)) {
			    You("set it off!");
			    b_trapped("door", FINGER);
		    } else
			    You("disarm it!");
		    levl[x][y].doormask &= ~D_TRAPPED;
		} else pline("This door was not trapped.");
		return(1);
	} else {
		You("find no traps on the door.");
		return(1);
	}
}

/* only called when the player is doing something to the chest directly */
boolean
chest_trap(obj, bodypart, disarm)
register struct obj *obj;
register int bodypart;
boolean disarm;
{
	register struct obj *otmp = obj, *otmp2;
	char	buf[80];
	const char *msg;

	You(disarm ? "set it off!" : "trigger a trap!");
	display_nhwindow(WIN_MESSAGE, FALSE);
	if (Luck > -13 && rn2(13+Luck) > 7) {	/* saved by luck */
	    /* trap went off, but good luck prevents damage */
	    switch (rn2(13)) {
		case 12:
		case 11:  msg = "explosive charge is a dud";  break;
		case 10:
		case  9:  msg = "electric charge is grounded";  break;
		case  8:
		case  7:  msg = "flame fizzles out";  break;
		case  6:
		case  5:
		case  4:  msg = "poisoned needle misses";  break;
		case  3:
		case  2:
		case  1:
		case  0:  msg = "gas cloud blows away";  break;
		default:  impossible("chest disarm bug");  msg = NULL;  break;
	    }
	    if (msg) pline("But luckily the %s!", msg);
	} else {
	    switch(rn2(20) ? ((Luck >= 13) ? 0 : rn2(13-Luck)) : rn2(26)) {
		case 25:
		case 24:
		case 23:
		case 22:
		case 21: {
		          register struct monst *shkp;
			  long loss = 0L;
			  boolean costly, insider;
			  register xchar ox = obj->ox, oy = obj->oy;

#ifdef GCC_WARN
			  shkp = (struct monst *) 0;
#endif
			  /* the obj location need not be that of player */
			  costly = (costly_spot(ox, oy) && 
				   (shkp = shop_keeper(*in_rooms(ox, oy,
				    SHOPBASE))) != (struct monst *)0);
			  insider = (*u.ushops && inside_shop(u.ux, u.uy) &&
				    *in_rooms(ox, oy, SHOPBASE) == *u.ushops);

			  pline("%s explodes!", The(xname(obj)));
			  Sprintf(buf, "exploding %s", xname(obj));

			  if(costly)
			      loss += stolen_value(obj, ox, oy,
						(boolean)shkp->mpeaceful, TRUE);
			  delete_contents(obj);
			  for(otmp = level.objects[u.ux][u.uy];
							otmp; otmp = otmp2) {
			      otmp2 = otmp->nexthere;
			      if(costly)
			          loss += stolen_value(otmp, otmp->ox, 
					  otmp->oy, (boolean)shkp->mpeaceful,
					  TRUE);
			      delobj(otmp);
			  }
			  exercise(A_STR, FALSE);
			  losehp(d(6,6), buf, KILLED_BY_AN);
			  if(costly && loss) {
			      if(insider)
			      You("owe %ld zorkmids for objects destroyed.",
				                              loss);
			      else {
  		                  You("caused %ld zorkmids worth of damage!",
			                                      loss);
			          make_angry_shk(shkp, ox, oy);
			      }
			  }
			  wake_nearby();
			  return TRUE;
			}
		case 20:
		case 19:
		case 18:
		case 17:
			pline("A cloud of noxious gas billows from %s.",
			      the(xname(obj)));
			poisoned("gas cloud", A_STR, "cloud of poison gas",15);
			exercise(A_CON, FALSE);
			break;
		case 16:
		case 15:
		case 14:
		case 13:
			You("feel a needle prick your %s.",body_part(bodypart));
			poisoned("needle", A_CON, "poisoned needle",10);
			exercise(A_CON, FALSE);
			break;
		case 12:
		case 11:
		case 10:
		case 9:
			pline("A tower of flame erupts from %s!",
			      the(xname(obj)));
			if(Fire_resistance) {
			    shieldeff(u.ux, u.uy);
			    You("don't seem to be affected.");
			} else	losehp(d(4, 6), "tower of flame", KILLED_BY_AN);
			destroy_item(SCROLL_CLASS, AD_FIRE);
			destroy_item(SPBOOK_CLASS, AD_FIRE);
			destroy_item(POTION_CLASS, AD_FIRE);
			if (is_ice(u.ux, u.uy))
			    melt_ice(u.ux, u.uy);
			break;
		case 8:
		case 7:
		case 6:
			You("are jolted by a surge of electricity!");
			if(Shock_resistance)  {
			    shieldeff(u.ux, u.uy);
			    You("don't seem to be affected.");
			} else	losehp(d(4, 4), "electric shock", KILLED_BY_AN);
			destroy_item(RING_CLASS, AD_ELEC);
			destroy_item(WAND_CLASS, AD_ELEC);
			break;
		case 5:
		case 4:
		case 3:
			pline("Suddenly you are frozen in place!");
			nomul(-d(5, 6));
			exercise(A_DEX, FALSE);
			nomovemsg = "You can move again.";
			break;
		case 2:
		case 1:
		case 0:
			pline("A cloud of %s gas billows from %s.",
			      hcolor(), the(xname(obj)));
			if(!Stunned) {
			    if (Hallucination)
				pline("What a groovy feeling!");
			    else if (Blind)
				You("stagger and get dizzy...");
			    else
				You("stagger and your vision blurs...");
			}
			make_stunned(HStun + rn1(7, 16),FALSE);
			make_hallucinated(HHallucination + rn1(5, 16),FALSE,0L);
			break;
		default: impossible("bad chest trap");
			break;
	    }
	    bot();			/* to get immediate botl re-display */
	}
	otmp->otrapped = 0;		/* these traps are one-shot things */

	return FALSE;
}

#endif /* OVLB */
#ifdef OVL0

struct trap *
t_at(x,y)
register int x, y;
{
	register struct trap *trap = ftrap;
	while(trap) {
		if(trap->tx == x && trap->ty == y) return(trap);
		trap = trap->ntrap;
	}
	return((struct trap *)0);
}

#endif /* OVL0 */
#ifdef OVLB

void
deltrap(trap)
register struct trap *trap;
{
	register struct trap *ttmp;

	if(trap == ftrap)
		ftrap = ftrap->ntrap;
	else {
		for(ttmp = ftrap; ttmp->ntrap != trap; ttmp = ttmp->ntrap) ;
		ttmp->ntrap = trap->ntrap;
	}
	dealloc_trap(trap);
}

/* used for doors (also tins).  can be used for anything else that opens. */
void
b_trapped(item, bodypart)
register const char *item;
register int bodypart;
{
	register int lvl = level_difficulty();
	int dmg = rnd(5 + (lvl < 5 ? lvl : 2+lvl/2));

	pline("KABOOM!!  %s was booby-trapped!", The(item));
	losehp(dmg, "explosion", KILLED_BY_AN);
	exercise(A_STR, FALSE);
	if (bodypart) exercise(A_CON, FALSE);
	make_stunned(HStun + dmg, TRUE);
}

/* Monster is hit by trap. */
/* Note: doesn't work if both obj and d_override are null */
STATIC_OVL boolean
thitm(tlev, mon, obj, d_override)
register int tlev;
register struct monst *mon;
register struct obj *obj;
int d_override;
{
	register int strike;
	register boolean trapkilled = FALSE;

	if (d_override) strike = 1;
	else if (obj) strike = (find_mac(mon) + tlev + obj->spe <= rnd(20));
	else strike = (find_mac(mon) + tlev <= rnd(20));

	/* Actually more accurate than thitu, which doesn't take
	 * obj->spe into account.
	 */
	if(!strike) {
		if (cansee(mon->mx, mon->my))
			pline("%s is almost hit by %s!", Monnam(mon),
								doname(obj));
	} else {
		int dam = 1;

		if (obj && cansee(mon->mx, mon->my))
			pline("%s is hit by %s!", Monnam(mon), doname(obj));
		if (d_override) dam = d_override;
		else if (obj) {
			dam = dmgval(obj, mon->data);
			if (dam < 1) dam = 1;
		}
		if ((mon->mhp -= dam) <= 0) {
			int xx = mon->mx;
			int yy = mon->my;

			monkilled(mon, "", AD_PHYS);
#ifdef MUSE
			if (mon->mhp <= 0) {
#endif
				newsym(xx, yy);
				trapkilled = TRUE;
#ifdef MUSE
			}
#endif
		}
	}
	if (obj && (!strike || d_override)) {
		place_object(obj, mon->mx, mon->my);
		obj->nobj = fobj;
		fobj = obj;
		stackobj(fobj);
	} else if (obj) dealloc_obj(obj);

	return trapkilled;
}

boolean
unconscious()
{
	return((boolean)(multi < 0 && (!nomovemsg ||
		u.usleep ||
		!strncmp(nomovemsg,"You regain con", 15) ||
		!strncmp(nomovemsg,"You are consci", 15))));
}

static char lava_killer[] = "molten lava";

boolean
lava_effects()
{
    register struct obj *obj, *obj2;
    int dmg;

    if (!Fire_resistance) {
	if(Wwalking) {
	    dmg = d(6,6);
	    pline("The lava here burns you!");
	    if(dmg < u.uhp) {
		losehp(dmg, lava_killer, KILLED_BY);
		goto burn_stuff;
	    }
	} else
	    You("fall into the lava!");

	for(obj = invent; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(is_organic(obj) && !obj->oerodeproof) {
		if(obj->owornmask) {
		    if(obj == uarm) (void) Armor_gone();
		    else if(obj == uarmc) (void) Cloak_off();
		    else if(obj == uarmh) (void) Helmet_off();
		    else if(obj == uarms) (void) Shield_off();
		    else if(obj == uarmg) (void) Gloves_off();
		    else if(obj == uarmf) (void) Boots_off();
#ifdef TOURIST
		    else if(obj == uarmu) setnotworn(obj);
#endif
		    else if(obj == uleft) Ring_gone(obj);
		    else if(obj == uright) Ring_gone(obj);
		    else if(obj == ublindf) Blindf_off(obj);
		    else if(obj == uwep) uwepgone();
		    if(Lifesaved
#ifdef WIZARD
		       || wizard
#endif
		       ) Your("%s into flame!", aobjnam(obj, "burst"));
		}
		useup(obj);
	    }
	}

	/* s/he died... */
	u.uhp = -1;
	killer_format = KILLED_BY;
	killer = lava_killer;
	You("burn to a crisp...");
	done(BURNING);
	if (!safe_teleds())
		while (1) {
			pline("You're still burning.");
			done(BURNING);
		}
	You("find yourself back on solid %s.", surface(u.ux, u.uy));
	return(TRUE);
    }

    if (!Wwalking) {
	u.utrap = rn1(4, 4) + (rn1(4, 12) << 8);
	u.utraptype = TT_LAVA;
	You("sink into the lava, but it doesn't burn you!");
    }
    /* just want to burn boots, not all armor; destroy_item doesn't work on
       armor anyway */
burn_stuff:
    if(uarmf && !uarmf->oerodeproof && is_organic(uarmf)) {
	/* save uarmf value because Boots_off() sets uarmf to NULL */
	obj = uarmf;
	Your("%s burst into flame!", xname(obj));
	(void) Boots_off();
	useup(obj);
    }
    destroy_item(SCROLL_CLASS, AD_FIRE);
    destroy_item(SPBOOK_CLASS, AD_FIRE);
    destroy_item(POTION_CLASS, AD_FIRE);
    return(FALSE);
}

#endif /* OVLB */

/*trap.c*/
