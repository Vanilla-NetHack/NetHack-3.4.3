/*	SCCS Id: @(#)trap.c	3.0	89/11/20
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#include	"edog.h"

#ifdef OVLB

const char *traps[] = {
	"",
	" monster trap",
	" statue trap",
	" bear trap",
	"n arrow trap",
	" dart trap",
	" trapdoor",
	" teleportation trap",
	" pit",
	" sleeping gas trap"
	," magic trap"
	," squeaky board"
	," web"
	," spiked pit"
	," level teleporter"
#ifdef SPELLS
	,"n anti-magic field" 
#endif
	," rust trap"
#ifdef POLYSELF
	," polymorph trap"
#endif
	," land mine"
};

#endif /* OVLB */

void NDECL(domagictrap);
STATIC_DCL boolean FDECL(thitm, (int, struct monst *, struct obj *, int));

#ifdef OVLB

static void NDECL(vtele);

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
	static const char NEARDATA *gook[] = { "slag", "rust", "rot", "corrosion" };
	static const char NEARDATA *action[] = { "smolder", "rust", "rot", "corrode" };
	static const char NEARDATA *msg[] =  { "burnt", "rusted", "rotten", "corroded" };
	boolean vulnerable = FALSE;
	boolean plural;

	if (!otmp) return(FALSE);
	switch(type) {
		case 0:
		case 2: vulnerable = is_flammable(otmp); break;
		case 1: vulnerable = is_rustprone(otmp); break;
		case 3: vulnerable = is_corrodeable(otmp); break;
	}

	if (!print && (!vulnerable || otmp->rustfree || otmp->spe < -2))
		return FALSE;

	plural = is_gloves(otmp) || is_boots(otmp);

	if (!vulnerable)
		Your("%s %s not affected!", ostr, plural ? "are" : "is");
	else if (otmp->spe >= -2) {
		if (otmp->rustfree)
			pline("The %s on your %s vanishes instantly!",
						gook[type], ostr);
		else if (otmp->blessed && !rnl(4))
			pline("Somehow, your %s %s not affected!", ostr,
					plural ? "are" : "is");
		else {
			Your("%s %s%s!", ostr, action[type],
				plural ? "" : "s");
			otmp->spe--;
			adj_abon(otmp, -1);
		}
	} else Your("%s %s%s quite %s.", ostr, Blind ? "feel" : "look",
					     plural ? "" : "s", msg[type]);
	return(TRUE);
}

struct trap *
maketrap(x,y,typ)
register int x, y, typ;
{
	register struct trap *ttmp;
	register struct permonst *ptr;

	if (ttmp = t_at(x,y)) {
		if (u.utrap &&
		  ((u.utraptype == TT_BEARTRAP && typ != BEAR_TRAP) ||
		  (u.utraptype == TT_WEB && typ != WEB) ||
		  (u.utraptype == TT_PIT && typ != PIT && typ != SPIKED_PIT)))
			u.utrap = 0;
		ttmp->ttyp = typ;
		return ttmp;
	}
	ttmp = newtrap();
	ttmp->ttyp = typ;
	ttmp->tx = x;
	ttmp->ty = y;
	switch(typ) {
	    case MONST_TRAP:	    /* create a monster in "hiding" */
	    {	int tryct = 0;
		if(rn2(5) && (ptr = mkclass(S_PIERCER)))
			ttmp->pm = monsndx(ptr);
		else do {
			ttmp->pm = rndmonnum();
		} while ((noattacks(&mons[ttmp->pm]) ||
			!mons[ttmp->pm].mmove) && ++tryct < 100);
		if (tryct == 100) {
			free((genericptr_t)ttmp);
			return(struct trap *)0;
		}
		break;
	    }
	    case STATUE_TRAP:	    /* create a "living" statue */
		ttmp->pm = rndmonnum();
		(void) mkcorpstat(STATUE, &mons[ttmp->pm], x, y);
		break;
	    default:
		ttmp->pm = -1;
		break;
	}
	ttmp->tseen = 0;
	ttmp->once = 0;
	ttmp->ntrap = ftrap;
	ftrap = ttmp;
	return(ttmp);
}

int
teleok(x, y)
register int x, y;
{				/* might throw him into a POOL
				 * removed by GAN 10/20/86
				 */
#ifdef STUPID
	boolean	tmp1, tmp2, tmp3;
#  ifdef POLYSELF
	tmp1 = isok(x,y) && (!IS_ROCK(levl[x][y].typ) ||
		passes_walls(uasmon)) && !MON_AT(x, y);
#  else
	tmp1 = isok(x,y) && !IS_ROCK(levl[x][y].typ) && !MON_AT(x, y);
#  endif
	tmp2 = !sobj_at(BOULDER,x,y) && !t_at(x,y);
	tmp3 = !(is_pool(x,y) &&
	       !(Levitation || Wwalking
#ifdef POLYSELF
		 || is_flyer(uasmon)
#endif
		)) && !closed_door(x,y);
	return(tmp1 && tmp2 && tmp3);
#else
	return( isok(x,y) &&
#  ifdef POLYSELF
		(!IS_ROCK(levl[x][y].typ) || passes_walls(uasmon)) &&
#  else
		!IS_ROCK(levl[x][y].typ) &&
#  endif
		!MON_AT(x, y) &&
		!sobj_at(BOULDER,x,y) && !t_at(x,y) &&
		!(is_pool(x,y) &&
		!(Levitation || Wwalking
#ifdef POLYSELF
		  || is_flyer(uasmon)
#endif
		  )) && !closed_door(x,y));
#endif
	/* Note: gold is permitted (because of vaults) */
}

static void
vtele() {
	register struct mkroom *croom;

	for(croom = &rooms[0]; croom->hx >= 0; croom++)
	    if(croom->rtype == VAULT) {
		register int x, y;

		x = rn2(2) ? croom->lx : croom->hx;
		y = rn2(2) ? croom->ly : croom->hy;
		if(teleok(x,y)) {
		    teleds(x,y);
		    return;
		}
	    }
	tele();
}

void
fall_through(td)
boolean td;	/* td == TRUE : trapdoor */
{
	register int newlevel = dlevel + 1;

	while(!rn2(4) && newlevel < 29) newlevel++;
	if(td) pline("A trap door opens up under you!");
	else pline("The floor opens up under you!");
	if(Levitation || u.ustuck || dlevel == MAXLEVEL
#ifdef POLYSELF
		|| is_flyer(uasmon) || u.umonnum == PM_WUMPUS
#endif
#ifdef ENDGAME
		|| dlevel == ENDLEVEL
#endif
	) {
	    You("don't fall in.");
	    if(!td) {
		more();
		pline("The opening under you closes up.");
	    }
	    return;
	}
#ifdef WALKIES
	if(!next_to_u()) {
	    You("are jerked back by your pet!");
	    if(!td) {
		more();
		pline("The opening in the floor closes up.");
	    }
	} else {
#endif
	    if(in_shop(u.ux, u.uy)) shopdig(1);
	    unsee();
	    (void) fflush(stdout);
	    goto_level(newlevel, FALSE, TRUE);
	    if(!td) pline("The hole in the ceiling above you closes up.");
#ifdef WALKIES
	}
#endif
}

void
dotrap(trap)
register struct trap *trap;
{
	register int ttype = trap->ttyp;
	register struct monst *mtmp;
	register struct obj *otmp;

	nomul(0);
	if(trap->tseen && !Fumbling && !(ttype == PIT
	   || ttype == SPIKED_PIT
#ifdef SPELLS
	   || ttype == ANTI_MAGIC
#endif
		) && !rn2(5))
		You("escape a%s.", traps[ttype]);
	else {
	    trap->tseen = 1;
	    if(Invisible && ttype != MONST_TRAP)
		newsym(trap->tx,trap->ty);
	    switch(ttype) {
		case SLP_GAS_TRAP:
		    if(Sleep_resistance) {
			You("are enveloped in a cloud of gas!");
			break;
		    }
		    pline("A cloud of gas puts you to sleep!");
		    flags.soundok = 0;
		    nomul(-rnd(25));
		    afternmv = Hear_again;
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
		    u.utrap = 4 + rn2(4);
		    u.utraptype = TT_BEARTRAP;
		    pline("A bear trap closes on your %s!",
			body_part(FOOT));
#ifdef POLYSELF
		    if(u.umonnum == PM_OWLBEAR || u.umonnum == PM_BUGBEAR)
			You("howl in anger!");
#endif
		    break;
		case STATUE_TRAP:
		    deltrap(trap);
		    for(otmp=level.objects[u.ux][u.uy];
						otmp; otmp = otmp->nexthere)
			if(otmp->otyp == STATUE && otmp->corpsenm == trap->pm)
			    if(mtmp=makemon(&mons[trap->pm],u.ux,u.uy)) {
				pline("The statue comes to life!");
				delobj(otmp);
				break;
			    }
		    break;
		case MONST_TRAP:
		    if(mtmp=makemon(&mons[trap->pm],u.ux,u.uy)) {
		      mtmp->mpeaceful = FALSE;
		      switch(mtmp->data->mlet) {
			case S_PIERCER:
			    pline("%s suddenly drops from the ceiling!",
				  Xmonnam(mtmp));
			    if(uarmh)
				pline("Its blow glances off your helmet.");
			    else
				(void) thitu(3,d(4,6),(struct obj *)0,
					"falling piercer");
			    break;
			default:	/* monster surprises you. */
			    pline("%s attacks you by surprise!",
				  Xmonnam(mtmp));
			    break;
		      }
		    }
		    deltrap(trap);
		    break;
		case ARROW_TRAP:
		    pline("An arrow shoots out at you!");
		    if(!thitu(8,rnd(6),(struct obj *)0,"arrow")){
			(void) mksobj_at(ARROW, u.ux, u.uy);
			fobj->quan = 1;
			fobj->owt = weight(fobj);
		    }
		    break;
		case TRAPDOOR:
		    if(is_maze_lev
#ifdef STRONGHOLD
			 && (dlevel > stronghold_level)
#endif
		      ) {
	pline("A trap door in the ceiling opens and a rock falls on your %s!",
				body_part(HEAD));
			if(uarmh)
			    pline("Fortunately, you are wearing a helmet!");
			losehp(uarmh ? 2 : d(2,10),"falling rock", KILLED_BY_AN);
			(void) mksobj_at(ROCK, u.ux, u.uy);
			fobj->quan = 1;
			fobj->owt = weight(fobj);
			stackobj(fobj);
			if(Invisible
#ifdef POLYSELF
				|| u.uundetected
#endif
						) newsym(u.ux, u.uy);
		    } else fall_through(TRUE);
		    break;
		case DART_TRAP:
		    pline("A little dart shoots out at you!");
		    if(thitu(7,rnd(3),(struct obj *)0,"little dart")) {
			if(!rn2(6)) poisoned("dart",A_CON,"poison dart",10);
		    } else {
			(void) mksobj_at(DART, u.ux, u.uy);
			fobj->quan = 1;
			if(!rn2(6)) fobj->opoisoned = 1;
			fobj->owt = weight(fobj);
		    }
		    break;
		case TELEP_TRAP:
		    if(trap->once) {
#ifdef ENDGAME
			if(dlevel == ENDLEVEL) {
			    You("feel a wrenching sensation.");
			    break;
			}
#endif
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    You("feel a wrenching sensation.");
			} else {
			    deltrap(trap);
			    newsym(u.ux, u.uy);
			    vtele();
			}
		    } else {
#ifdef ENDGAME
			if(dlevel == ENDLEVEL) {
			    You("feel a wrenching sensation.");
			    break;
			}
#endif
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    You("feel a wrenching sensation.");
			} else {
			    newsym(u.ux, u.uy);
			    tele();
			}
		    }
		    break;
		case RUST_TRAP:
#ifdef POLYSELF
# ifdef GOLEMS
		    if (u.umonnum == PM_IRON_GOLEM) {
			pline("A gush of water hits you!");
			You("are covered with rust!");
			rehumanize();
			break;
		    } else
# endif /* GOLEMS */
		    if (u.umonnum == PM_GREMLIN && rn2(3)) {
			pline("A gush of water hits you!");
			if(mtmp = cloneu()) {
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
two_hand:		    corrode_weapon();
			    goto glovecheck;
			default:
			    pline("A gush of water hits you!");
			    if (uarmc) (void) rust_dmg(uarmc, "cloak", 1, TRUE);
			    else if (uarm)
				(void) rust_dmg(uarm, "armor", 1, TRUE);
#ifdef SHIRT
			    else if (uarmu)
				(void) rust_dmg(uarmu, "shirt", 1, TRUE);
#endif
		    }
		    break;
		case PIT:
		    if (Levitation
#ifdef POLYSELF
			|| is_flyer(uasmon) || u.umonnum == PM_WUMPUS
#endif
			) {
			pline("A pit opens up under you!");
			You("don't fall in!");
			break;
		    }
		    You("fall into a pit!");
#ifdef POLYSELF
		    if (!passes_walls(uasmon))
#endif
			u.utrap = rn1(6,2);
		    u.utraptype = TT_PIT;
		    losehp(rnd(6),"fell into a pit", NO_KILLER_PREFIX);
		    selftouch("Falling, you");
		    break;
		case SPIKED_PIT:
		    if (Levitation
#ifdef POLYSELF
			|| is_flyer(uasmon) || u.umonnum == PM_WUMPUS
#endif
			) {
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
		    selftouch("Falling, you");
		    break;
		case LEVEL_TELEP:

		    {	int oldl = dlevel;

		    You("%s onto a level teleport trap!",
			  Levitation ? "float" :
#ifdef POLYSELF
			  locomotion(uasmon, "step"));
#else
			  "step");
#endif
		    if(Antimagic) {
			pru();
			shieldeff(u.ux, u.uy);
		    }
		    if(Antimagic
#ifdef ENDGAME
				|| dlevel == ENDLEVEL
#endif
							) {
			You("feel a wrenching sensation.");
			break;
		    }
		    if(!Blind)
		      You("are momentarily blinded by a flash of light.");
		    else
			You("are momentarily disoriented.");
		    deltrap(trap);
		    newsym(u.ux,u.uy);
		    level_tele();
		    if(oldl == dlevel && !Invisible
#ifdef POLYSELF
						&& !u.uundetected
#endif
								) {
			levl[u.ux][u.uy].seen = 0; /* force atl */
			atl(u.ux,u.uy,(char)u.usym);
		    }
		}
		    break;
#ifdef SPELLS
		case ANTI_MAGIC:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			You("feel momentarily lethargic.");
		    } else drain_en(rnd((int)u.ulevel) + 1);
		    break;
#endif
#ifdef POLYSELF
		case POLY_TRAP:
		    if(Antimagic) {
			shieldeff(u.ux, u.uy);
			You("feel momentarily different.");
			/* Trap did nothing; don't remove it --KAA */
		    } else {
			You("feel a change coming over you.");
			polyself();
			deltrap(trap);
		    }
		    break;
#endif
		case MGTRP:	    /* A magic trap. */
		    if (!rn2(30)) {
			You("are caught in a magical explosion!");
			losehp(rnd(10), "magical explosion", KILLED_BY_AN);
#ifdef SPELLS
			Your("body absorbs some of the magical energy!");
			u.uen = (u.uenmax += 2);
#endif
			deltrap(trap);
			if(Invisible
#ifdef POLYSELF
				&& !u.uundetected
#endif
						) newsym(u.ux,u.uy);
		    } else domagictrap();
		    break;
		case SQBRD:	    /* stepped on a squeaky board */
		    if (Levitation
#ifdef POLYSELF
			|| is_flyer(uasmon)
#endif
			) {
			if (Hallucination) You("notice a crease in the linoleum.");
			else You("notice a loose board below you.");
		    } else {
			pline("A board beneath you squeaks loudly.");
			wake_nearby();
		    }
		    break;
		case WEB: /* Our luckless player has stumbled into a web. */

		    You("%s into a spider web!",
			  Levitation ? "float" :
#ifdef POLYSELF
			  locomotion(uasmon, "stumble"));
#else
			  "stumble");
#endif
		    u.utraptype = TT_WEB;

		    /* Time stuck in the web depends on your strength. */

		    if (ACURR(A_STR) == 3) u.utrap = rn1(6,6);
		    else if (ACURR(A_STR) < 6) u.utrap = rn1(6,4);
		    else if (ACURR(A_STR) < 9) u.utrap = rn1(4,4);
		    else if (ACURR(A_STR) < 12) u.utrap = rn1(4,2);
		    else if (ACURR(A_STR) < 15) u.utrap = rn1(2,2);
		    else if (ACURR(A_STR) < 18) u.utrap = rnd(2);
		    else if (ACURR(A_STR) < 69) u.utrap = 1;
		    else {
			u.utrap = 0;
			You("tear through the web!");
			deltrap(trap);
	   		if(Invisible) newsym(u.ux,u.uy);
		    }
		    break;

		case LANDMINE: {
#ifndef LINT
		    register struct monst *mtmp = fmon;
#endif

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
			set_wounded_legs(LEFT_SIDE, 40 + rnd(35));
			set_wounded_legs(RIGHT_SIDE, 40 + rnd(35));
		    }
		    losehp(rnd(16), "land mine", KILLED_BY_AN);
		    /* wake everything on the level */
		    while(mtmp) {
			if(mtmp->msleep) mtmp->msleep = 0;
			mtmp = mtmp->nmon;
		    }
		    deltrap(t_at(u.ux, u.uy)); /* mines only explode once */
		    if(Invisible) newsym(u.ux,u.uy);
		    }
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

STATIC_DCL void FDECL(seetrap, (struct trap *));

#ifdef OVLB

STATIC_OVL void
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
	register int newlev, wasintrap = mtmp->mtrapped;
	register boolean trapkilled = FALSE, tdoor = FALSE;
	struct obj *otmp;

	if(!trap) {
		mtmp->mtrapped = 0;	/* perhaps teleported? */
	} else if(wasintrap) {
		if(!rn2(40)) mtmp->mtrapped = 0;
	} else {
	    register int tt = trap->ttyp;

	/* A bug fix for dumb messages by ab@unido.
	 */
	    int in_sight = cansee(mtmp->mx,mtmp->my)
			   && (!mtmp->minvis || See_invisible);

	    if(mtmp->mtrapseen & (1 << tt)) {
		/* he has been in such a trap - perhaps he escapes */
		if(rn2(4)) return(0);
	    }
	    mtmp->mtrapseen |= (1 << tt);
	    switch (tt) {
		case BEAR_TRAP:
			if(mtmp->data->msize > MZ_SMALL &&
			   !amorphous(mtmp->data)) {
				mtmp->mtrapped = 1;
				if(in_sight) {
				  pline("%s is caught in a bear trap!",
					Monnam(mtmp));
				  seetrap(trap);
				} else
				    if((mtmp->data == &mons[PM_OWLBEAR]
					|| mtmp->data == &mons[PM_BUGBEAR])
					&& flags.soundok)
			    You("hear the roaring of an angry bear!");
			}
			break;
#ifdef POLYSELF
		case POLY_TRAP:
		    if(!resist(mtmp, WAND_SYM, 0, NOTELL)) {
			(void) newcham(mtmp, (struct permonst *)0);
			seetrap(trap);
		    }
		    break;
#endif
		case RUST_TRAP:
			if(in_sight)
			    pline("A gush of water hits %s!", mon_nam(mtmp));
			if(cansee(mtmp->mx,mtmp->my))
			    seetrap(trap);
#ifdef GOLEMS
			if (mtmp->data == &mons[PM_IRON_GOLEM]) {
				if (in_sight)
				    pline("%s falls to pieces!", Monnam(mtmp));
				else if(mtmp->mtame)
				    pline("May %s rust in peace.",
								mon_nam(mtmp));
				mondied(mtmp);
				trapkilled = TRUE;
			} else
#endif /* GOLEMS */
			if (mtmp->data == &mons[PM_GREMLIN] && rn2(3)) {
				struct monst *mtmp2 = clone_mon(mtmp);

				if (mtmp2) {
				    mtmp2->mhpmax = (mtmp->mhpmax /= 2);
				    if(in_sight)
					pline("%s multiplies.", Monnam(mtmp));
				}
			}
			break;
		case PIT:
		case SPIKED_PIT:
			/* TO DO: there should be a mtmp/data -> floating */
			if(!is_flyer(mtmp->data) &&
			   mtmp->data != &mons[PM_WUMPUS]) {
				if (!passes_walls(mtmp->data))
				    mtmp->mtrapped = 1;
				if(in_sight) {
				    pline("%s falls into a pit!", Monnam(mtmp));
				    seetrap(trap);
				}
				if(thitm(0, mtmp, (struct obj *)0,
					 rnd((tt==PIT) ? 6 : 10)))
				    trapkilled = TRUE;
			}
			break;
		case SLP_GAS_TRAP:
			if(!resists_sleep(mtmp->data) &&
			   !mtmp->msleep && mtmp->mcanmove) {
				mtmp->mcanmove = 0;
				mtmp->mfrozen = rnd(25);
				if(in_sight)
				  pline("%s suddenly falls asleep!",
					Monnam(mtmp));
				if(cansee(mtmp->mx,mtmp->my))
				    seetrap(trap);
			}
			break;
		case TELEP_TRAP:
#ifdef WALKIES
			if(teleport_pet(mtmp)) {
#endif
			    /* Note: don't remove the trap if a vault.  Other-
			     * the monster will be stuck there, since the guard
			     * isn't going to come for it...
			     */
			    if (trap->once) vloc(mtmp);
			    else rloc(mtmp);
			    if(in_sight && !cansee(mtmp->mx,mtmp->my)) {
				pline("%s suddenly disappears!",
					Monnam(mtmp));
				seetrap(trap);
			    }
#ifdef WALKIES
			}
#endif
			break;
		case ARROW_TRAP:
			otmp = mksobj(ARROW, FALSE);
			otmp->quan = 1;
			otmp->owt = weight(otmp);
			if(in_sight) seetrap(trap);
			if(thitm(8, mtmp, otmp, 0)) trapkilled = TRUE;
			break;
		case DART_TRAP:
			otmp = mksobj(DART, FALSE);
			otmp->quan = 1;
			if (!rn2(6)) otmp->opoisoned = 1;
			otmp->owt = weight(otmp);
			if(in_sight) seetrap(trap);
			if(thitm(7, mtmp, otmp, 0)) trapkilled = TRUE;
			break;
		case TRAPDOOR:
			if(is_maze_lev
#ifdef STRONGHOLD
			   && (dlevel > stronghold_level && dlevel < MAXLEVEL)
#endif
			  ) {
				otmp = mksobj(ROCK, FALSE);
				otmp->quan = 1;
				otmp->owt = weight(otmp);
				if(in_sight) seetrap(trap);
				if(thitm(0, mtmp, otmp, d(2, 10)))
					trapkilled = TRUE;
				break;
			}
			if (mtmp->data == &mons[PM_WUMPUS]) break;
			tdoor = TRUE;
			/* Fall through */
		case LEVEL_TELEP:
			if(!is_flyer(mtmp->data)
#ifdef WORM
				&& !mtmp->wormno
			    /* long worms with tails mustn't change levels */
#endif
			    ) {
#ifdef WALKIES
			    if(teleport_pet(mtmp)) {
#endif
				if(tdoor)
				    fall_down(mtmp, dlevel+1);
				else {
				    newlev = rnd(3);
				    if(!rn2(2)) newlev = -(newlev);
				    newlev = dlevel + newlev;
				    if(newlev > MAXLEVEL) {
					if(dlevel != MAXLEVEL)
					    newlev = MAXLEVEL;
					else newlev = MAXLEVEL - rnd(3);
				    }
				    if(newlev < 1) {
					if(dlevel != 1) newlev = 1;
					else newlev = 1 + rnd(3);
				    }
				    fall_down(mtmp, newlev);
				}
				if(in_sight) {
		pline("Suddenly, %s disappears out of sight.", mon_nam(mtmp));
				    seetrap(trap);
				}
				return(2);	/* no longer on this level */
#ifdef WALKIES
			    }
#endif
			}
			break;
		case MONST_TRAP:
		case STATUE_TRAP:
			break;
		case MGTRP:
			/* A magic trap.  Monsters immune. */
			break;
		case SQBRD: {
			register struct monst *ztmp = fmon;

			if(is_flyer(mtmp->data)) break;
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
	       case WEB:
			/* Monster in a web. */
			if(mtmp->data->mlet != S_SPIDER) {
			    if(in_sight)
				pline("%s is caught in a web.", Monnam(mtmp));
			    else /* Eric Backus */
				if(mtmp->data == &mons[PM_OWLBEAR] ||
					mtmp->data == &mons[PM_BUGBEAR])
				    You("hear the roaring of a confused bear!");
			    mtmp->mtrapped = 1;
			}
			break;
#ifdef SPELLS
		case ANTI_MAGIC:	break;
#endif
		case LANDMINE: {
			register struct monst *mntmp = fmon;

			if(rn2(3))
				break; /* monsters usually don't set it off */
			if(is_flyer(mtmp->data)) {
				if (in_sight) {
	pline("A trigger appears in a pile of soil below %s.", Monnam(mtmp));
					seetrap(trap);
				}
				if (rn2(3)) break;
				if (in_sight)
					pline("The air currents set it off!");
			} else if(in_sight)
			    pline("KAABLAMM!!!  %s triggers a land mine!",
				  Monnam(mtmp));
			if (!in_sight && flags.soundok)
				pline("Kaablamm!  You hear an explosion in the distance!");
			deltrap(t_at(mtmp->mx, mtmp->my));
			if(thitm(0, mtmp, (struct obj *)0, rnd(16)))
				trapkilled = TRUE;
			/* wake everything on the level */
			while(mntmp) {
				if(mntmp->msleep)
					mntmp->msleep = 0;
				mntmp = mntmp->nmon;
			}
			break;
		}
		default:
			impossible("Some monster encountered a strange trap of type %d.", tt);
	    }
	}
	if(trapkilled) return 2;
	else return mtmp->mtrapped;
}

#endif /* OVL1 */
#ifdef OVLB

void
selftouch(arg)
const char *arg;
{
	if(uwep && (uwep->otyp == CORPSE && uwep->corpsenm == PM_COCKATRICE)
#ifdef POLYSELF
			&& !resists_ston(uasmon)
#endif
	){
		pline("%s touch the cockatrice corpse.", arg);
		You("turn to stone...");
		killer_format = KILLED_BY;
		killer = "touching a cockatrice corpse";
		done(STONING);
	}
}

void
float_up() {
	if(u.utrap) {
		if(u.utraptype == TT_PIT) {
			u.utrap = 0;
			You("float up, out of the pit!");
		} else {
			You("float up, only your %s is still stuck.",
				body_part(LEG));
		}
	} else
		if (Hallucination)
			pline("Up, up, and awaaaay!  You're walking on air!");
		else
			You("start to float in the air!");
}

int
float_down() {
	register struct trap *trap;

	if(Levitation) return(0); /* maybe another ring/potion/boots */

	/* check for falling into pool - added by GAN 10/20/86 */
	if(is_pool(u.ux,u.uy) && !(Wwalking
#ifdef POLYSELF
				    || is_flyer(uasmon)
#endif
				    ))
		drown();

	You("float gently to the ground.");
	if(trap = t_at(u.ux,u.uy))
		switch(trap->ttyp) {
		case MONST_TRAP:
		case STATUE_TRAP:
			break;
		case TRAPDOOR:
			if(is_maze_lev
#ifdef STRONGHOLD
			   && (dlevel >= stronghold_level || dlevel < MAXLEVEL)
#endif
			   || u.ustuck) break;
			/* fall into next case */
		default:
			dotrap(trap);
	}
	if(!flags.nopick && (OBJ_AT(u.ux, u.uy) || levl[u.ux][u.uy].gmask))
	    pickup(1);
	return 0;
}


void
tele() {
	coord cc;
	register int nux,nuy;

#ifdef STRONGHOLD
	/* Disable teleportation in stronghold && Vlad's Tower */
	if(dlevel == stronghold_level ||
# ifdef ENDGAME
	   dlevel == ENDLEVEL ||
# endif
	   (dlevel >= tower_level && dlevel <= tower_level + 2)) {
# ifdef WIZARD
		if (!wizard) {
# endif
		    pline("A mysterious force prevents you from teleporting!");
		    return;
# ifdef WIZARD
		}
# endif
	}
#endif /* STRONGHOLD /**/
	if((u.uhave_amulet || dlevel == wiz_level) && !rn2(3)) {
	    You("feel disoriented for a moment.");
	    return;
	}
	if(Teleport_control) {
	    if (unconscious())
		pline("Being unconscious, you cannot control your teleport.");
	    else {
		    pline("To what position do you want to be teleported?");
		    cc.x = u.ux;
		    cc.y = u.uy;
		    getpos(&cc, 1, "the desired position"); /* 1: force valid */
		    /* possible extensions: introduce a small error if
		       magic power is low; allow transfer to solid rock */
		    if(teleok(cc.x, cc.y)){
			teleds(cc.x, cc.y);
			return;
		    }
		    pline("Sorry...");
		}
	}
	do {
		nux = rnd(COLNO-1);
		nuy = rn2(ROWNO);
	} while(!teleok(nux, nuy));
	teleds(nux, nuy);
}

void
teleds(nux, nuy)
register int nux,nuy;
{
	if(Punished) unplacebc();
	unsee();
	u.utrap = 0;
	u.ustuck = 0;
	u.ux0 = u.ux;
	u.uy0 = u.uy;
	u.ux = nux;
	u.uy = nuy;
#ifdef POLYSELF
	if (hides_under(uasmon))
		u.uundetected = (OBJ_AT(nux, nuy) || levl[nux][nuy].gmask);
	else 
		u.uundetected = 0;
	if (u.usym == S_MIMIC_DEF) u.usym = S_MIMIC;
#endif
	if(Punished) placebc(1);
	if(u.uswallow){
		u.uswldtim = u.uswallow = 0;
		docrt();
	}
	setsee();
	nomul(0);
	spoteffects();
}

int
dotele()
{
	struct trap *trap;
#ifdef SPELLS
	boolean castit = FALSE;
# ifdef __GNULINT__
	register int sp_no = 0;
# else
	register int sp_no;
# endif
#endif

	trap = t_at(u.ux, u.uy);
	if (trap && (!trap->tseen || trap->ttyp != TELEP_TRAP))
		trap = 0;

	if (trap) {
		if (trap->once) {
			pline("This is a vault teleport, usable once only.");
			pline("Jump in? ");
			if (yn() == 'n')
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
#ifdef SPELLS
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
#endif
#ifdef WIZARD
		if (!wizard) {
#endif
#ifdef SPELLS
		    if (!castit) {
			if (!Teleportation)
			    You("don't know that spell.");
			else
#endif
			    You("are not able to teleport at will.");
			return(0);
#ifdef SPELLS
		    }
#endif
#ifdef WIZARD
		}
#endif
	}

	if(!trap && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
		You("lack the strength for a teleport spell.");
#ifdef WIZARD
		if(!wizard)
#endif
		return(1);
	}

#ifdef SPELLS
	if (castit)
# ifdef WIZARD
		if (!spelleffects(++sp_no, TRUE) && !wizard) return(0);
# else
		return spelleffects(++sp_no, TRUE);
# endif
#endif

#ifdef WALKIES
	if(next_to_u()) {
#endif
		if (trap && trap->once) vtele();
		else tele();
#ifdef WALKIES
		(void) next_to_u();
	} else {
		You("shudder for a moment.");
		return(0);
	}
#endif
	if (!trap) morehungry(100);
	return(1);
}

void
placebc(attach)
int attach;
{
	if(!uchain || !uball){
		impossible("Where are your ball and chain?");
		return;
	}
	if(!carried(uball))
		place_object(uball, u.ux, u.uy);
	place_object(uchain, u.ux, u.uy);
	if(attach){
		uchain->nobj = fobj;
		fobj = uchain;
		if(!carried(uball)){
			uball->nobj = fobj;
			fobj = uball;
		}
	}
}

void
unplacebc(){
	if(!carried(uball)){
		freeobj(uball);
		unpobj(uball);
	}
	freeobj(uchain);
	unpobj(uchain);
}

void
level_tele() {
	register int newlevel;

	if(u.uhave_amulet
#ifdef ENDGAME
		|| dlevel == ENDLEVEL
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
	      pline("To what level do you want to teleport? [type a number] ");
	      getlin(buf);
	    } while(!digit(buf[0]) && (buf[0] != '-' || !digit(buf[1])));
	    newlevel = atoi(buf);
	} else {
#ifdef STRONGHOLD
	    /* We cannot send them to Hell if STRONGHOLD is defined, since
	     * they may find themselves trapped on the other side of the
	     * stronghold...
	     */
	    newlevel = rn2(5) ? rnd(dlevel + 3) : rnd(stronghold_level);
#else
	    newlevel = rn2(5) || !Fire_resistance ? rnd(dlevel + 3) : HELLLEVEL;
#endif
	    if(dlevel == newlevel)
		if(is_maze_lev) newlevel--; else newlevel++;
	}

#ifdef WALKIES
	if(!next_to_u()) {
		You("shudder for a moment...");
		return;
	}
#endif

	if(newlevel < 0) {
		if(newlevel <= -10) {
			You("arrive in heaven.");
			verbalize("Thou art early, but we'll admit thee.");
			killer_format = NO_KILLER_PREFIX;
			killer = "went to heaven prematurely";
			done(DIED);
			return;
		} else	if (newlevel == -9) {
			You("feel deliriously happy. ");
			pline("(In fact, you're on Cloud 9!) ");
			more();
		} else
			You("are now high above the clouds...");

		if(Levitation) {
		    You("float gently down to earth.");
#ifdef STRONGHOLD
		    newlevel = 1;
#else
		    done(ESCAPED);
#endif
		}
#ifdef POLYSELF
		else if(is_flyer(uasmon)) {
		    You("fly down to earth.");
# ifdef STRONGHOLD
		    newlevel = 1;
# else
		    done(ESCAPED);
# endif
		}
#endif
		else {
		    int save_dlevel;

		    save_dlevel = dlevel;
		    pline("Unfortunately, you don't know how to fly.");
		    You("plummet a few thousand feet to your death.");
		    dlevel = 0;
		    killer_format = NO_KILLER_PREFIX;
		    killer =
    self_pronoun("teleported out of the dungeon and fell to %s death","his");
		    done(DIED);
		    dlevel = save_dlevel;
		    return;  
		}
	}

	/* calls done(ESCAPED) if newlevel==0 */
	goto_level(newlevel, FALSE, FALSE);
}

void
domagictrap() {
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
	     case 12:
		      /* a flash of fire */
		      {
			register int num;

			/* changed to be in conformance with
			 * SCR_FIRE by GAN 11/02/86
			 */

			pline("A tower of flame bursts from the floor!");
			if(Fire_resistance) {
				shieldeff(u.ux, u.uy);
				You("are uninjured.");
				break;
			} else {
				num = rnd(6);
				u.uhpmax -= num;
				losehp(num,"burst of flame", KILLED_BY_AN);
				break;
			}
		      }

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
		       adjattrib(A_CHA,1,FALSE);
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
					obj->cursed = 0;
		       if(Punished) unpunish();
		       break;
		   }
	     default: break;
	  }
}

void
drown() {
	register struct obj *obj;

	/* Scrolls and potions get affected by the water */
	for(obj = invent; obj; obj = obj->nobj) {
		if(obj->olet == SCROLL_SYM && rn2(12) > Luck
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
								)
			obj->otyp = SCR_BLANK_PAPER;
		if(obj->olet == POTION_SYM && rn2(12) > Luck) {
			if (obj->spe == -1) {
				obj->otyp = POT_WATER;
				obj->blessed = obj->cursed = 0;
				obj->spe = 0;
			} else obj->spe--;
		}
	}

#ifdef POLYSELF
	if(u.umonnum == PM_GREMLIN && rn2(3)) {
		struct monst *mtmp;
		if(mtmp = cloneu()) {
			mtmp->mhpmax = (u.mhmax /= 2);
			You("multiply.");
		}
	}

	if(is_swimmer(uasmon)) return;
#endif

	You("fell into %s!",
	      levl[u.ux][u.uy].typ == POOL ? "a pool" : "the moat");
	You("can't swim!");
	if(
#ifdef WIZARD
	wizard ||
#endif
	rn2(3) < Luck+2) {
		You("attempt a teleport spell.");	/* utcsri!carroll */
		(void) dotele();
		if(!is_pool(u.ux,u.uy)) return;
	}
	You("drown.");
	killer_format = KILLED_BY_AN;
	killer = levl[u.ux][u.uy].typ == POOL ? "pool of water" : "moat";
	done(DROWNING);
}

#ifdef SPELLS
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
#endif

int
dountrap() {	/* disarm a trapped object */
	register struct obj *otmp;
	register boolean confused = (Confusion > 0 || Hallucination > 0);
	register int x,y;
	int ch;
	struct trap *ttmp;

#ifdef POLYSELF
	if(nohands(uasmon)) {
	    pline("And just how do you expect to do that?");
	    return(0);
	}
#endif
	if(!getdir(TRUE)) return(0);
	x = u.ux + u.dx;
	y = u.uy + u.dy;

	if(!u.dx && !u.dy) {
	    for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if(Is_box(otmp)) {
		    pline("There is %s here, check for traps? ", doname(otmp));

		    switch (ynq()) {
			case 'q': return(0);
			case 'n': continue;
		    }

		    if((otmp->otrapped && !confused 
				&& rn2(MAXLEVEL+2-dlevel) < 10)
		       || confused && !rn2(3)) {
			You("find a trap on the %s!  Disarm it? ", xname(otmp));

			switch (ynq()) {
			    case 'q': return(1);
			    case 'n': continue;
			}

			if(otmp->otrapped) {
			    ch = 15 + (pl_character[0] == 'R') ? u.ulevel*3
								: u.ulevel;
			    if(confused || Fumbling || rnd(75+dlevel/2) > ch) {
				You("set it off!");
				chest_trap(otmp, FINGER);
			    } else {
				You("disarm it!");
				otmp->otrapped = 0;
			    }
			} else pline("That %s was not trapped.", doname(otmp));
			return(1);
		    } else {
			You("find no traps on the %s.", xname(otmp));
			return(1);
		    }
		}
	    if ((ttmp = t_at(x,y)) && ttmp->tseen)
		You("cannot disable this trap.");
	    else
		You("know of no traps here.");
	    return(0);
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

	if ((levl[x][y].doormask & D_TRAPPED && !confused &&
	     rn2(MAXLEVEL+2-dlevel) < 10)
	    || confused && !rn2(3)) {
		You("find a trap on the door!  Disarm it? ");
		if (ynq() != 'y') return(1);
		if (levl[x][y].doormask & D_TRAPPED) {
		    ch = 15 +
			 (pl_character[0] == 'R') ? u.ulevel*3 :
			 u.ulevel;
		    if(confused || Fumbling || rnd(75+dlevel/2) > ch) {
			    You("set it off!");
			    b_trapped("door");
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
void
chest_trap(obj, bodypart)
register struct obj *obj;
register int bodypart;
{
	register struct obj *otmp,*otmp2;
	char	buf[80];

	if(Luck > -13 && rn2(13+Luck) > 7) return;

	otmp = obj;
	switch(rn2(20) ? ((Luck >= 13) ? 0 : rn2(13-Luck)) : rn2(26)) {
		case 25:
		case 24:
		case 23:
		case 22:
		case 21:
			pline("The %s explodes!", xname(obj));
			Sprintf(buf, "exploding %s", xname(obj));

			delete_contents(obj);
			for(otmp = level.objects[u.ux][u.uy];
							otmp; otmp = otmp2) {
			    otmp2 = otmp->nexthere;
			    delobj(otmp);
			}

			losehp(d(6,6), buf, KILLED_BY_AN);
			wake_nearby();
			return;
		case 20:
		case 19:
		case 18:
		case 17:
			pline("A cloud of noxious gas billows from the %s.",
			      xname(obj));
			poisoned("gas cloud", A_STR, "cloud of poison gas",15);
			break;
		case 16:
		case 15:
		case 14:
		case 13:
			You("feel a needle prick your %s.",body_part(bodypart));
			poisoned("needle", A_CON, "poison needle",10);
			break;
		case 12:
		case 11:
		case 10:
		case 9:
			pline("A tower of flame erupts from the %s",
			      xname(obj));
			if(Fire_resistance) {
			    shieldeff(u.ux, u.uy);
			    You("don't seem to be affected.");
			} else	losehp(d(4, 6), "tower of flame", KILLED_BY_AN);
			destroy_item(SCROLL_SYM, AD_FIRE);
#ifdef SPELLS
			destroy_item(SPBOOK_SYM, AD_FIRE);
#endif
			destroy_item(POTION_SYM, AD_FIRE);
			break;
		case 8:
		case 7:
		case 6:
			You("are jolted by a surge of electricity!");
			if(Shock_resistance)  {
			    shieldeff(u.ux, u.uy);
			    You("don't seem to be affected.");
			} else	losehp(d(4, 4), "electric shock", KILLED_BY_AN);
			destroy_item(RING_SYM, AD_ELEC);
			destroy_item(WAND_SYM, AD_ELEC);
			break;
		case 5:
		case 4:
		case 3:
			pline("Suddenly you are frozen in place!");
			nomul(-d(5, 6));
			nomovemsg = "You can move again.";
			break;
		case 2:
		case 1:
		case 0:
			pline("A cloud of %s gas billows from the %s",
			      hcolor(), xname(obj));
			if(!Stunned)
			    if (Hallucination)
				pline("What a groovy feeling!");
			    else
				You("stagger and your vision blurs...");
			make_stunned(HStun + rn1(7, 16),FALSE);
			make_hallucinated(Hallucination + rn1(5, 16),FALSE);
			break;
		default: impossible("bad chest trap");
			break;
	}
	bot(); 			/* to get immediate botl re-display */
	otmp->otrapped = 0;		/* these traps are one-shot things */
}

#endif /* OVLB */
#ifdef OVL2

void
wake_nearby() {			/* Wake up nearby monsters. */
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if(dist(mtmp->mx,mtmp->my) < u.ulevel*20) {
		if(mtmp->msleep)  mtmp->msleep = 0;
		if(mtmp->mtame)   EDOG(mtmp)->whistletime = moves;
	    }
	}
}

#endif /* OVL2 */
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
	free((genericptr_t) trap);
}

void
b_trapped(item)		/* used for doors. can be used */
register const char *item;    /* for anything else that opens */
{
	register int dmg = rnd(5+(dlevel < 5 ? dlevel : 2+dlevel/2));

	pline("KABOOM!!  The %s was booby-trapped!", item);
	if(u.ulevel < 4 && dlevel < 3 && !rnl(3))
		You("are shaken, but luckily unhurt.");		
	else losehp(dmg, "explosion", KILLED_BY_AN);
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
	else if (obj) strike = (mon->data->ac + tlev + obj->spe <= rnd(20));
	else strike = (mon->data->ac + tlev <= rnd(20));

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

			if (cansee(mon->mx, mon->my))
				pline("%s is killed!", Monnam(mon));
			else if (mon->mtame)
	You("have a sad feeling for a moment, then it passes.");
			mondied(mon);
			newsym(xx, yy);
			trapkilled = TRUE;
		}
	}
	if (obj && (!strike || d_override)) {
		place_object(obj, mon->mx, mon->my);
		obj->nobj = fobj;
		fobj = obj;
		stackobj(fobj);
	} else if (obj) free ((genericptr_t)obj);

	return trapkilled;
}

boolean
unconscious()
{
	return (multi < 0 && (!nomovemsg ||
		!strncmp(nomovemsg,"You wake", 8) ||
		!strncmp(nomovemsg,"You awake", 9) ||
		!strncmp(nomovemsg,"You regain con", 15) ||
		!strncmp(nomovemsg,"You are consci", 15)));
}

#endif /* OVLB */
