/*	SCCS Id: @(#)dogmove.c	3.1	92/11/26	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include "mfndpos.h"
#include "edog.h"

#ifdef OVL0

static boolean FDECL(dog_hunger,(struct monst *,struct edog *));
static boolean FDECL(dog_invent,(struct monst *,struct edog *,int));
static int FDECL(dog_goal,(struct monst *,struct edog *,int,int,int));

#ifndef MUSE
#define DROPPABLES(mtmp) mtmp->minvent
#else
static struct obj *FDECL(DROPPABLES, (struct monst *));

static struct obj *
DROPPABLES(mon)
register struct monst *mon;
{
	register struct obj *obj;
	struct obj *wep = MON_WEP(mon);

	for(obj = mon->minvent; obj; obj = obj->nobj)
		if (!obj->owornmask && obj != wep) return obj;
	return (struct obj *)0;
}
#endif

static const char NEARDATA nofetch[] = { BALL_CLASS, CHAIN_CLASS, ROCK_CLASS, 0 };

#endif /* OVL0 */

STATIC_VAR xchar gtyp, gx, gy;	/* type and position of dog's current goal */

STATIC_DCL void FDECL(dog_eat, (struct monst *, struct obj *, int, int));
STATIC_PTR void FDECL(wantdoor, (int, int, genericptr_t));

#ifdef OVLB

STATIC_OVL void
dog_eat(mtmp, obj, x, y)
register struct monst *mtmp;
register struct obj * obj;
int x, y;
{
	register struct edog *edog = EDOG(mtmp);
	int nutrit;

	if(edog->hungrytime < moves)
	    edog->hungrytime = moves;
	/*
	 * It is arbitrary that the pet takes the same length of time to eat
	 * as a human, but gets more nutritional value.
	 */
	if (obj->oclass == FOOD_CLASS) {
	    if(obj->otyp == CORPSE) {
		mtmp->meating = 3 + (mons[obj->corpsenm].cwt >> 6);
		nutrit = mons[obj->corpsenm].cnutrit;
	    } else {
		mtmp->meating = objects[obj->otyp].oc_delay;
		nutrit = objects[obj->otyp].oc_nutrition;
	    }
	    switch(mtmp->data->msize) {
		case MZ_TINY: nutrit *= 8; break;
		case MZ_SMALL: nutrit *= 6; break;
		default:
		case MZ_MEDIUM: nutrit *= 5; break;
		case MZ_LARGE: nutrit *= 4; break;
		case MZ_HUGE: nutrit *= 3; break;
		case MZ_GIGANTIC: nutrit *= 2; break;
	    }
	    if(obj->oeaten) {
		mtmp->meating = eaten_stat(mtmp->meating, obj);
		nutrit = eaten_stat(nutrit, obj);
	    }
	} else if (obj->oclass == GOLD_CLASS) {
	    mtmp->meating = ((int)obj->quan/2000) + 1;
	    nutrit = ((int)obj->quan/20);
	} else {
	    /* Unusual pet such as gelatinous cube eating odd stuff.
	     * meating made consistent with wild monsters in mon.c.
	     * nutrit made consistent with polymorphed player nutrit in
	     * eat.c.  (This also applies to pets eating gold.)
	     */
	    mtmp->meating = obj->owt/20 + 1;
	    nutrit = 5*objects[obj->otyp].oc_nutrition;
	}
	edog->hungrytime += nutrit;
	mtmp->mconf = 0;
	if (mtmp->mtame < 20) mtmp->mtame++;
	if(cansee(x,y))
	    pline("%s eats %s.", Monnam(mtmp), (obj->oclass==FOOD_CLASS)
		? singular(obj, doname) : doname(obj));
	/* It's a reward if it's DOGFOOD and the player dropped/threw it. */
	/* We know the player had it if invlet is set -dlc */
	if(dogfood(mtmp,obj) == DOGFOOD && obj->invlet)
#ifdef LINT
	    edog->apport = 0;
#else
	    edog->apport += (unsigned)(200L/
		((long)edog->dropdist+moves-edog->droptime));
#endif
	if (obj == uball) {
	    unpunish();
	    delobj(obj);
	} else if (obj == uchain)
	    unpunish();
	else if (obj->quan > 1L && obj->oclass == FOOD_CLASS)
	    obj->quan--;
	else
	    delobj(obj);
}

#endif /* OVLB */
#ifdef OVL0

/* hunger effects -- returns TRUE on starvation */
static boolean
dog_hunger(mtmp, edog)
register struct monst *mtmp;
register struct edog *edog;
{
	if(moves > edog->hungrytime + 500) {
	    if(!carnivorous(mtmp->data) && !herbivorous(mtmp->data)) {
		edog->hungrytime = moves + 500;
		/* but not too high; it might polymorph */
	    } else if (!mtmp->mconf) {
		mtmp->mconf = 1;
		mtmp->mhpmax /= 3;
		if(mtmp->mhp > mtmp->mhpmax)
		    mtmp->mhp = mtmp->mhpmax;
		if(mtmp->mhp < 1) goto dog_died;
		if(cansee(mtmp->mx, mtmp->my))
		    pline("%s is confused from hunger.", Monnam(mtmp));
		else {
		    char buf[BUFSZ];

		    Strcpy(buf, "the ");
		    You("feel worried about %s.", mtmp->mnamelth ?
			NAME(mtmp) : strcat(buf, Hallucination
			? rndmonnam() : mtmp->data->mname));
		}
	    } else if(moves > edog->hungrytime + 750 || mtmp->mhp < 1) {
	    dog_died:
#ifdef WALKIES
		if(mtmp->mleashed)
		    Your("leash goes slack.");
#endif
		if(cansee(mtmp->mx, mtmp->my))
		    pline("%s dies%s.", Monnam(mtmp),
			    (mtmp->mhp >= 1) ? "" : " from hunger");
		else
		    You("have a sad feeling for a moment, then it passes.");
		mondied(mtmp);
		return(TRUE);
	    }
	}
	return(FALSE);
}

/* do something with object (drop, pick up, eat) at current position
 * returns TRUE if object eaten (since that counts as dog's move)
 */
static boolean
dog_invent(mtmp, edog, udist)
register struct monst *mtmp;
register struct edog *edog;
int udist;
{
	register int omx, omy;
	struct obj *obj;

	omx = mtmp->mx;
	omy = mtmp->my;

	/* if we are carrying sth then we drop it (perhaps near @) */
	/* Note: if apport == 1 then our behaviour is independent of udist */
	if(DROPPABLES(mtmp) || mtmp->mgold) {
	    if(!rn2(udist) || !rn2((int) edog->apport))
		if(rn2(10) < edog->apport){
		    relobj(mtmp, (int)mtmp->minvis, TRUE);
		    if(edog->apport > 1) edog->apport--;
		    edog->dropdist = udist;		/* hpscdi!jon */
		    edog->droptime = moves;
		}
	} else {
	    if((obj=level.objects[omx][omy]) && !index(nofetch,obj->oclass)
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
									){
		if (dogfood(mtmp, obj) <= CADAVER) {
		    dog_eat(mtmp, obj, omx, omy);
		    return TRUE;
		}
		if(can_carry(mtmp, obj) && !obj->cursed)
		    if(rn2(20) < edog->apport+3)
			if(rn2(udist) || !rn2((int) edog->apport)) {
			    if (cansee(omx, omy) && flags.verbose)
				pline("%s picks up %s.", Monnam(mtmp),
				    distant_name(obj, doname));
			    freeobj(obj);
			    newsym(omx,omy);
			    mpickobj(mtmp,obj);
			}
	    }
	}
	return FALSE;
}

/* set dog's goal -- gtyp, gx, gy
 * returns -1/0/1 (dog's desire to approach player) or -2 (abort move)
 */
static int
dog_goal(mtmp, edog, after, udist, whappr)
register struct monst *mtmp;
struct edog *edog;
int after, udist, whappr;
{
	register int omx, omy;
	boolean in_masters_sight;
	register struct obj *obj;
	xchar otyp;
	int appr;

	omx = mtmp->mx;
	omy = mtmp->my;

	in_masters_sight = couldsee(omx, omy);

	if (!edog
#ifdef WALKIES
		    || mtmp->mleashed	/* he's not going anywhere... */
#endif
					) {
	    gtyp = APPORT;
	    gx = u.ux;
	    gy = u.uy;
	} else {
#define DDIST(x,y) (dist2(x,y,omx,omy))
#define SQSRCHRADIUS 5
	    int min_x, max_x, min_y, max_y;
	    register int nx, ny;

	    gtyp = UNDEF;	/* no goal as yet */
	    gx = gy = 0;	/* suppress 'used before set' message */

	    if ((min_x = omx - SQSRCHRADIUS) < 0) min_x = 0;
	    if ((max_x = omx + SQSRCHRADIUS) >= COLNO) max_x = COLNO - 1;
	    if ((min_y = omy - SQSRCHRADIUS) < 0) min_y = 0;
	    if ((max_y = omy + SQSRCHRADIUS) >= ROWNO) max_y = ROWNO - 1;

	    /* nearby food is the first choice, then other objects */
	    for (obj = fobj; obj; obj = obj->nobj) {
		nx = obj->ox;
		ny = obj->oy;
		if (nx >= min_x && nx <= max_x && ny >= min_y && ny <= max_y) {
		    otyp = dogfood(mtmp, obj);
		    if (otyp > gtyp || otyp == UNDEF)
			continue;
		    if (otyp < MANFOOD) {
			if (otyp < gtyp || DDIST(nx,ny) < DDIST(gx,gy)) {
			    gx = nx;
			    gy = ny;
			    gtyp = otyp;
			}
		    } else if(gtyp == UNDEF && in_masters_sight &&
			      !mtmp->minvent &&
			      (!levl[omx][omy].lit || levl[u.ux][u.uy].lit) &&
			      (otyp == MANFOOD || m_cansee(mtmp, nx, ny)) &&
			      edog->apport > rn2(8) &&
			      can_carry(mtmp,obj)) {
			gx = nx;
			gy = ny;
			gtyp = APPORT;
		    }
		}
	    }
	}

	/* follow player if appropriate */
	if (gtyp == UNDEF ||
	    (gtyp != DOGFOOD && gtyp != APPORT && moves < edog->hungrytime)) {
		gx = u.ux;
		gy = u.uy;
		if (after && udist <= 4 && gx == u.ux && gy == u.uy)
			return(-2);
		appr = (udist >= 9) ? 1 : (mtmp->mflee) ? -1 : 0;
		if (udist > 1) {
			if (!IS_ROOM(levl[u.ux][u.uy].typ) || !rn2(4) ||
			   whappr ||
			   (mtmp->minvent && rn2((int) edog->apport)))
				appr = 1;
		}
		/* if you have dog food it'll follow you more closely */
		if (appr == 0) {
			obj = invent;
			while (obj) {
				if(dogfood(mtmp, obj) == DOGFOOD) {
					appr = 1;
					break;
				}
				obj = obj->nobj;
			}
		}
	} else
	    appr = 1;	/* gtyp != UNDEF */
	if(mtmp->mconf)
	    appr = 0;

#define FARAWAY (COLNO + 2)		/* position outside screen */
	if (gx == u.ux && gy == u.uy && !in_masters_sight) {
	    register coord *cp;

	    cp = gettrack(omx,omy);
	    if (cp) {
		gx = cp->x;
		gy = cp->y;
		if(edog) edog->ogoal.x = 0;
	    } else {
		/* assume master hasn't moved far, and reuse previous goal */
		if(edog && edog->ogoal.x &&
		   ((edog->ogoal.x != omx) || (edog->ogoal.y != omy))) {
		    gx = edog->ogoal.x;
		    gy = edog->ogoal.y;
		    edog->ogoal.x = 0;
		} else {
		    int fardist = FARAWAY * FARAWAY;
		    gx = gy = FARAWAY; /* random */
		    do_clear_area(omx, omy, 9, wantdoor,
				  (genericptr_t)&fardist);

		    /* here gx == FARAWAY e.g. when dog is in a vault */
		    if (gx == FARAWAY || (gx == omx && gy == omy)) {
			gx = u.ux;
			gy = u.uy;
		    } else if(edog) {
			edog->ogoal.x = gx;
			edog->ogoal.y = gy;
		    }
		}
	    }
	} else if(edog) {
	    edog->ogoal.x = 0;
	}
	return appr;
}

/* return 0 (no move), 1 (move) or 2 (dead) */
int
dog_move(mtmp, after)
register struct monst *mtmp;
register int after;	/* this is extra fast monster movement */
{
	int omx, omy;		/* original mtmp position */
	int appr, whappr, udist;
	int i, j;
	register struct edog *edog = EDOG(mtmp);
	struct obj *obj = (struct obj *) 0;
	xchar otyp;
	boolean has_edog, cursemsg = FALSE, do_eat = FALSE;
	xchar nix, niy;		/* position mtmp is (considering) moving to */
	register int nx, ny;	/* temporary coordinates */
	xchar cnt, uncursedcnt, chcnt;
	int chi = -1, nidist, ndist;
	coord poss[9];
	long info[9], allowflags;
#define GDIST(x,y) (dist2(x,y,gx,gy))

	/*
	 * Tame Angels have isminion set and an ispriest structure instead of
	 * an edog structure.  Fortunately, guardian Angels need not worry
	 * about mundane things like eating and fetching objects, and can
	 * spend all their energy defending the player.  (They are the only
	 * monsters with other structures that can be tame.)
	 */
	has_edog = !mtmp->isminion;

	omx = mtmp->mx;
	omy = mtmp->my;
	if (has_edog && dog_hunger(mtmp, edog)) return(2);	/* starved */

	udist = distu(omx,omy);
	/* maybe we tamed him while being swallowed --jgm */
	if (!udist) return(0);

	nix = omx;	/* set before newdogpos */
	niy = omy;

	if (has_edog && dog_invent(mtmp, edog, udist))	/* eating something */
		goto newdogpos;

	if (has_edog)
	    whappr = (moves - edog->whistletime < 5);
	else
	    whappr = 0;

	appr = dog_goal(mtmp, has_edog ? edog : (struct edog *)0,
							after, udist, whappr);
	if (appr == -2) return(0);

	allowflags = ALLOW_M | ALLOW_TRAPS | ALLOW_SSM | ALLOW_SANCT;
	if (passes_walls(mtmp->data)) allowflags |= (ALLOW_ROCK|ALLOW_WALL);
	if (throws_rocks(mtmp->data)) allowflags |= ALLOW_ROCK;
	if (Conflict && !resist(mtmp, RING_CLASS, 0, 0)) {
	    allowflags |= ALLOW_U;
	    if (!has_edog) {
		coord mm;
		/* Guardian angel refuses to be conflicted; rather,
		 * it disappears, angrily, and sends in some nasties
		 */
		if (canseemon(mtmp) || sensemon(mtmp)) {
		    pline("%s rebukes you, saying:", Monnam(mtmp));
		    verbalize("Since you desire conflict, have some more!");
		}
		mongone(mtmp);
		i = rnd(4);
		while(i--) {
		    mm.x = u.ux;
		    mm.y = u.uy;
		    if(enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
			(void) mk_roamer(&mons[PM_ANGEL], u.ualign.type,
					 mm.x, mm.y, FALSE);
		}
		return(2);

	    }
	}
	if (!nohands(mtmp->data) && !verysmall(mtmp->data)) {
		allowflags |= OPENDOOR;
		if (m_carrying(mtmp, SKELETON_KEY)) allowflags |= BUSTDOOR;
	}
	if (is_giant(mtmp->data)) allowflags |= BUSTDOOR;
	if (tunnels(mtmp->data) && !needspick(mtmp->data))
		allowflags |= ALLOW_DIG;
	cnt = mfndpos(mtmp, poss, info, allowflags);

	/* Normally dogs don't step on cursed items, but if they have no
	 * other choice they will.  This requires checking ahead of time
	 * to see how many cursed item squares are around.
	 */
	uncursedcnt = 0;
	for (i = 0; i < cnt; i++) {
		nx = poss[i].x; ny = poss[i].y;
		if (MON_AT(nx,ny)) continue;
		for (obj = level.objects[nx][ny]; obj; obj = obj->nexthere)
			if (obj->cursed) goto skipu;
		uncursedcnt++;
skipu:;
	}

	chcnt = 0;
	chi = -1;
	nidist = GDIST(nix,niy);

	for (i = 0; i < cnt; i++) {
		nx = poss[i].x;
		ny = poss[i].y;
#ifdef WALKIES
		/* if leashed, we drag him along. */
		if (mtmp->mleashed && distu(nx, ny) > 4) continue;
#endif
		/* if a guardian, try to stay close by choice */
		if (!has_edog &&
		    (j = distu(nx, ny)) > 16 && j >= udist) continue;

		if ((info[i] & ALLOW_M) && MON_AT(nx, ny)) {
		    int stat;
		    register struct monst *mtmp2 = m_at(nx,ny);

		    if ((int)mtmp2->m_lev >= (int)mtmp->m_lev+2 ||
			(mtmp2->data == &mons[PM_FLOATING_EYE] && rn2(10) &&
			 mtmp->mcansee && haseyes(mtmp->data) && mtmp2->mcansee
			 && (perceives(mtmp->data) || !mtmp2->minvis)) ||
			(mtmp2->data==&mons[PM_GELATINOUS_CUBE] && rn2(10)) ||
			(max_passive_dmg(mtmp2, mtmp) >= mtmp->mhp) ||
			(mtmp->mhp*4 < mtmp->mhpmax &&
			 mtmp2->mpeaceful && !Conflict) ||
			   (mtmp2->data->mlet == S_COCKATRICE &&
				!resists_ston(mtmp->data)))
			continue;

		    if (after) return(0); /* hit only once each move */

		    stat = mattackm(mtmp, mtmp2);

		    /* aggressor (pet) died */
		    if (stat & MM_AGR_DIED) return 2;

		    if ((stat & MM_HIT) && !(stat & MM_DEF_DIED) &&
			rn2(4) && mtmp2->mlstmv != monstermoves) {
			stat = mattackm(mtmp2, mtmp);	/* return attack */
			if (stat & MM_DEF_DIED) return 2;
		    }

		    return 0;
		}

		{   /* dog avoids traps, but perhaps it has to pass a trap
		     * in order to follow player
		     */
		    struct trap *trap;

		    if ((info[i] & ALLOW_TRAPS) && (trap = t_at(nx,ny))) {
			if ((trap->ttyp == RUST_TRAP
					&& mtmp->data != &mons[PM_IRON_GOLEM])
				|| trap->ttyp == STATUE_TRAP
				|| ((trap->ttyp == PIT
				    || trap->ttyp == SPIKED_PIT
				    || (trap->ttyp == TRAPDOOR &&
					!Can_fall_thru(&u.uz)))
				    && (is_flyer(mtmp->data) ||
					is_clinger(mtmp->data)))
				|| (trap->ttyp == SLP_GAS_TRAP &&
				    resists_sleep(mtmp->data)))
			    if(!trap->tseen || rn2(3)) continue;
#ifdef WALKIES
			if (!mtmp->mleashed) {
#endif
			    if (!trap->tseen && rn2(40)) continue;
			    if (rn2(10)) continue;
#ifdef WALKIES
			}
# ifdef SOUNDS
			else if (flags.soundok)
				whimper(mtmp);
# endif
#endif
		    }
		}

		/* dog eschews cursed objects, but likes dog food */
		for (obj = level.objects[nx][ny]; obj; obj = obj->nexthere) {
		    if (obj->cursed && !mtmp->mleashed && uncursedcnt)
			goto nxti;
		    if (obj->cursed) cursemsg = TRUE;
		    if (has_edog && (otyp = dogfood(mtmp, obj)) < MANFOOD &&
				(otyp < ACCFOOD || edog->hungrytime <= moves)){
			/* Note: our dog likes the food so much that he
			 * might eat it even when it conceals a cursed object */
			nix = nx;
			niy = ny;
			chi = i;
			do_eat = TRUE;
			goto newdogpos;
		    }
		}

		for (j = 0; j < MTSZ && j < cnt-1; j++)
			if (nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
				if (rn2(4*(cnt-j))) goto nxti;

		j = ((ndist = GDIST(nx,ny)) - nidist) * appr;
		if ((j == 0 && !rn2(++chcnt)) || j < 0 ||
			(j > 0 && !whappr &&
				((omx == nix && omy == niy && !rn2(3))
					|| !rn2(12))
			)) {
			nix = nx;
			niy = ny;
			nidist = ndist;
			if(j < 0) chcnt = 0;
			chi = i;
		}
	nxti:	;
	}
newdogpos:
	if (nix != omx || niy != omy) {
		if (info[chi] & ALLOW_U) {
#ifdef WALKIES
			if (mtmp->mleashed) { /* play it safe */
				pline("%s breaks loose of %s leash!",
					Monnam(mtmp),
					humanoid(mtmp->data)
					    ? (mtmp->female ? "her" : "his")
					    : "its");
				m_unleash(mtmp);
			}
#endif
			(void) mattacku(mtmp);
			return(0);
		}
		/* insert a worm_move() if worms ever begin to eat things */
		remove_monster(omx, omy);
		place_monster(mtmp, nix, niy);
		if (cursemsg && (cansee(omx,omy) || cansee(nix,niy)))
			pline("%s moves only reluctantly.", Monnam(mtmp));
		for (j=MTSZ-1; j>0; j--) mtmp->mtrack[j] = mtmp->mtrack[j-1];
		mtmp->mtrack[0].x = omx;
		mtmp->mtrack[0].y = omy;
		/* We have to know if the pet's gonna do a combined eat and
		 * move before moving it, but it can't eat until after being
		 * moved.  Thus the do_eat flag.
		 */
		if (do_eat)
			dog_eat(mtmp, obj, nix, niy);
	}
#ifdef WALKIES
	  /* an incredible kludge, but the only way to keep pooch near
	   * after it spends time eating or in a trap, etc.
	   */
	  else if (mtmp->mleashed && distu(omx, omy) > 4) {
		coord cc;

		nx = sgn(omx - u.ux);
		ny = sgn(omy - u.uy);
		cc.x = u.ux + nx;
		cc.y = u.uy + ny;
		if (goodpos(cc.x, cc.y, mtmp, mtmp->data)) goto dognext;

	 	i  = xytod(nx, ny);
		for (j = (i + 7)%8; j < (i + 1)%8; j++) {
			dtoxy(&cc, j);
			if (goodpos(cc.x, cc.y, mtmp, mtmp->data)) goto dognext;
		}
		for (j = (i + 6)%8; j < (i + 2)%8; j++) {
			dtoxy(&cc, j);
			if (goodpos(cc.x, cc.y, mtmp, mtmp->data)) goto dognext;
		}
		cc.x = mtmp->mx;
		cc.y = mtmp->my;
dognext:
		remove_monster(mtmp->mx, mtmp->my);
		place_monster(mtmp, cc.x, cc.y);
		newsym(cc.x,cc.y);
		set_apparxy(mtmp);
	}
#endif
	return(1);
}

#endif /* OVL0 */
#ifdef OVLB

/*ARGSUSED*/	/* do_clear_area client */
STATIC_PTR void
wantdoor(x, y, distance)
int x, y;
genericptr_t distance;
{
    register ndist;

    if (*(int*)distance > (ndist = distu(x, y))) {
	gx = x;
	gy = y;
	*(int*)distance = ndist;
    }
}

#endif /* OVLB */

/*dogmove.c*/
