/*	SCCS Id: @(#)mon.c	3.1	93/05/26	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you're using precompiled headers, you don't want this either */
#ifdef MICROPORT_BUG
#define MKROOM_H
#endif

#include "hack.h"
#include "mfndpos.h"
#include "edog.h"
#include <ctype.h>

STATIC_DCL boolean FDECL(restrap,(struct monst *));
STATIC_DCL void NDECL(dmonsfree);

#ifdef OVL1
#define warnDelay 10
long lastwarntime;
int lastwarnlev;

const char *warnings[] = {
	"white", "pink", "red", "ruby", "purple", "black"
};

static void NDECL(warn_effects);

#endif /* OVL1 */

#ifdef OVLB
static struct obj *FDECL(make_corpse,(struct monst *));
static void FDECL(m_detach,(struct monst *));

struct monst *fdmon;	/* chain of dead monsters, need not be saved */
			/* otherwise used only in priest.c */

/* Creates a monster corpse, a "special" corpse, or nothing if it doesn't
 * leave corpses.  Monsters which leave "special" corpses should have
 * G_NOCORPSE set in order to prevent wishing for one, finding tins of one,
 * etc....
 */
static struct obj *
make_corpse(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat = mtmp->data;
	int num;
	struct obj *obj = (struct obj *)0;
	int x = mtmp->mx, y = mtmp->my;
	int mndx = monsndx(mdat);

	switch(mndx) {
	    case PM_GRAY_DRAGON:
	    case PM_RED_DRAGON:
	    case PM_ORANGE_DRAGON:
	    case PM_WHITE_DRAGON:
	    case PM_BLACK_DRAGON:
	    case PM_BLUE_DRAGON:
	    case PM_GREEN_DRAGON:
	    case PM_YELLOW_DRAGON:
		/* Make dragon scales.  This assumes that the order of the */
		/* dragons is the same as the order of the scales.	   */
		if (!rn2(3)) {
		    obj = mksobj_at(GRAY_DRAGON_SCALES +
				    monsndx(mdat)-PM_GRAY_DRAGON, x, y, FALSE);
		    obj->spe = 0;
		    obj->cursed = obj->blessed = FALSE;
		}
		goto default_1;

	    case PM_WHITE_UNICORN:
	    case PM_GRAY_UNICORN:
	    case PM_BLACK_UNICORN:
		(void) mksobj_at(UNICORN_HORN, x, y, TRUE);
		goto default_1;
	    case PM_LONG_WORM:
		(void) mksobj_at(WORM_TOOTH, x, y, TRUE);
		goto default_1;
	    case PM_KOBOLD_MUMMY:
	    case PM_GNOME_MUMMY:
	    case PM_ORC_MUMMY:
	    case PM_ELF_MUMMY:
	    case PM_HUMAN_MUMMY:
	    case PM_GIANT_MUMMY:
	    case PM_ETTIN_MUMMY:
		(void) mksobj_at(MUMMY_WRAPPING, x, y, TRUE); /* and fall through */
	    case PM_KOBOLD_ZOMBIE:
	    case PM_GNOME_ZOMBIE:
	    case PM_ORC_ZOMBIE:
	    case PM_ELF_ZOMBIE:
	    case PM_HUMAN_ZOMBIE:
	    case PM_GIANT_ZOMBIE:
	    case PM_ETTIN_ZOMBIE:
		switch (mndx) {
		    case PM_KOBOLD_ZOMBIE:
		    case PM_KOBOLD_MUMMY:
			num = PM_KOBOLD; break;
		    case PM_GNOME_MUMMY:
		    case PM_GNOME_ZOMBIE:
			num = PM_GNOME; break;
		    case PM_ORC_MUMMY:
		    case PM_ORC_ZOMBIE:
			num = PM_ORC; break;
		    case PM_ELF_MUMMY:
		    case PM_ELF_ZOMBIE:
			num = PM_ELF; break;
		    case PM_HUMAN_MUMMY:
		    case PM_HUMAN_ZOMBIE:
			num = PM_HUMAN; break;
		    case PM_GIANT_MUMMY:
		    case PM_GIANT_ZOMBIE:
			num = PM_GIANT; break;
		    case PM_ETTIN_MUMMY:
		    case PM_ETTIN_ZOMBIE:
#ifdef GCC_WARN
		    default:
#endif
			num = PM_ETTIN; break;
		}
		obj = mkcorpstat(CORPSE, &mons[num], x, y, TRUE);
		obj->age -= 100;		/* this is an *OLD* corpse */
		break;
	    case PM_IRON_GOLEM:
		num = d(2,6);
		while (num--)
			obj = mksobj_at(IRON_CHAIN, x, y, TRUE);
		mtmp->mnamelth = 0;
		break;
	    case PM_CLAY_GOLEM:
		obj = mksobj_at(ROCK, x, y, FALSE);
		obj->quan = (long)(rn2(20) + 50);
		obj->owt = weight(obj);
		mtmp->mnamelth = 0;
		break;
	    case PM_STONE_GOLEM:
		obj = mkcorpstat(STATUE, mdat, x, y, FALSE);
		break;
	    case PM_WOOD_GOLEM:
		num = d(2,4);
		while(num--) {
			obj = mksobj_at(QUARTERSTAFF, x, y, TRUE);
			if (obj && obj->oartifact) {	/* don't allow this */
				artifact_exists(obj, ONAME(obj), FALSE);
				Strcpy(ONAME(obj), "");  obj->onamelth = 0;
			}
		}
		mtmp->mnamelth = 0;
		break;
	    case PM_LEATHER_GOLEM:
		num = d(2,4);
		while(num--)
			obj = mksobj_at(LEATHER_ARMOR, x, y, TRUE);
		mtmp->mnamelth = 0;
		break;
	    default_1:
	    default:
		if (mdat->geno & G_NOCORPSE)
			return (struct obj *)0;
		else obj = mkcorpstat(CORPSE, mdat, x, y, TRUE);
		break;
	}
	/* All special cases should precede the G_NOCORPSE check */

	/* Note: oname() cannot be used generically for non-inventory objects
	 * unless you fix the link from the previous object in the chains.
	 * (Here we know it's the first one, so there was no link.)
	 */
	if (mtmp->mnamelth) {
		obj = oname(obj, NAME(mtmp), 0);
		fobj = obj;
		level.objects[x][y] = obj;
	}
	stackobj(fobj);
	newsym(x, y);
	return obj;
}

#endif /* OVLB */
#ifdef OVL1

static void
warn_effects()
{
    if (warnlevel == 100) {
	if(!Blind &&
	    (warnlevel > lastwarnlev || moves > lastwarntime + warnDelay)) {
	    Your("%s %s!", aobjnam(uwep, "glow"),
		 Hallucination ? hcolor() : light_blue);
	    lastwarnlev = warnlevel;
	    lastwarntime = moves;
	}
	warnlevel = 0;
	return;
    }

    if(warnlevel >= SIZE(warnings))
	warnlevel = SIZE(warnings)-1;
    if(!Blind && warnlevel >= 0)
	if(warnlevel > lastwarnlev || moves > lastwarntime + warnDelay) {
	    register const char *rr;

	    lastwarntime = moves;
	    lastwarnlev = warnlevel;
	    switch((int) (Warning & (LEFT_RING | RIGHT_RING))) {
	    case LEFT_RING:
		rr = Hallucination ? "left mood ring glows" : "left ring glows";
		break;
	    case RIGHT_RING:
		rr = Hallucination ? "right mood ring glows"
			: "right ring glows";
		break;
	    case LEFT_RING | RIGHT_RING:
		rr = Hallucination ? "mood rings glow" : "rings both glow";
		break;
	    default:
		if (Hallucination)
		    Your("spider-sense is tingling....");
		else
		    You("feel apprehensive as you sense a %s flash.",
			warnings[warnlevel]);
		return;
	    }
	    Your("%s %s!", rr, Hallucination ? hcolor() : warnings[warnlevel]);
	}
}

/* check mtmp and water for compatibility, 0 (survived), 1 (drowned) */
int
minwater(mtmp)
register struct monst *mtmp;
{
    boolean inpool, infountain;

    inpool = is_pool(mtmp->mx,mtmp->my) &&
	     !is_flyer(mtmp->data) && !is_floater(mtmp->data);
    infountain = IS_FOUNTAIN(levl[mtmp->mx][mtmp->my].typ);

    /* Gremlin multiplying won't go on forever since the hit points
     * keep going down, and when it gets to 1 hit point the clone
     * function will fail.
     */
    if(mtmp->data->mlet == S_GREMLIN && (inpool || infountain) && rn2(3)) {
	struct monst *mtmp2 = clone_mon(mtmp);

	if (mtmp2) {
	    mtmp2->mhpmax = (mtmp->mhpmax /= 2);
	    if(cansee(mtmp->mx,mtmp->my))
		pline("%s multiplies.", Monnam(mtmp));
	    dryup(mtmp->mx,mtmp->my);
	}
	if (inpool) water_damage(mtmp->minvent, FALSE, FALSE);
	return (0);
    }
    if (inpool) {
	/* Most monsters drown in pools.  flooreffects() will take care of
	 * water damage to dead monsters' inventory, but survivors need to
	 * be handled here.  Swimmers are able to protect their stuff...
	 */
	if (!is_clinger(mtmp->data)
	    && !is_swimmer(mtmp->data) && !amphibious(mtmp->data)) {
	    if (cansee(mtmp->mx,mtmp->my))
		pline("%s drowns.", Monnam(mtmp));
	    mondead(mtmp);
#ifdef MUSE
	    if (mtmp->mhp > 0) {
		rloc(mtmp);
		water_damage(mtmp->minvent, FALSE, FALSE);
		return 0;
	    }
#endif
	    return (1);
	}
    } else {
	/* but eels have a difficult time outside */
	if (mtmp->data->mlet == S_EEL) {
	    if(mtmp->mhp > 1) mtmp->mhp--;
	    mtmp->mflee = 1;
	    mtmp->mfleetim += 2;
	}
    }
    return (0);
}

void
movemon()
{
    register struct monst *mtmp;
    register boolean tametype = TRUE;

    warnlevel = 0;

    while(1) {
	/* Find a monster that we have not treated yet.
	 * Note that mtmp or mtmp->nmon might get killed
	 * while mtmp moves, so we cannot just walk down the
	 * chain (even new monsters might get created!)
	 *
	 * Do tame monsters first.  Necessary so that when the tame
	 * monster attacks something, the something gets a chance to
	 * attack the tame monster back (which it's permitted to do
	 * only if it hasn't made its move yet).
	 */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->mlstmv < monstermoves &&
	       ((mtmp->mtame>0) == tametype)) goto next_mon;
	if(tametype) {
	    /* checked all tame monsters, now do other ones */
	    tametype = FALSE;
	    continue;
	}
	/* treated all monsters */
	break;

    next_mon:
	mtmp->mlstmv = monstermoves;

	if(mtmp->mhp <= 0) {
	    impossible("Monster with zero hp?");
	    mtmp->mhp = mtmp->mhpmax = 1; /* repair */
	}
	if (minwater(mtmp)) continue;

	if(mtmp->mblinded && !--mtmp->mblinded)
	    mtmp->mcansee = 1;
	if(mtmp->mfrozen && !--mtmp->mfrozen)
	    mtmp->mcanmove = 1;
	if(mtmp->mfleetim && !--mtmp->mfleetim)
	    mtmp->mflee = 0;
	if (is_hider(mtmp->data)) {
	    /* unwatched mimics and piercers may hide again  [MRS] */
	    if(restrap(mtmp))   continue;
	    if(mtmp->m_ap_type == M_AP_FURNITURE ||
					mtmp->m_ap_type == M_AP_OBJECT)
		    continue;
	    if(mtmp->mundetected) continue;
	}
	if(mtmp->mspeed != MSLOW || !(moves%2)) {
	    /* continue if the monster died fighting */
	    if (Conflict && !mtmp->iswiz && mtmp->mcansee) {
		/* Note:
		 *  Conflict does not take effect in the first round.
		 *  Therefore, A monster when stepping into the area will
		 *  get to swing at you.
		 *
		 *  The call to fightm() must be _last_.  The monster might
		 *  have died if it returns 1.
		 */
		if (couldsee(mtmp->mx,mtmp->my) &&
		    (distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) &&
								fightm(mtmp))
		     continue;	/* mon might have died */
	    }
	    if(dochugw(mtmp))
		/* otherwise just move the monster */
		continue;
	}
	if(mtmp->mspeed == MFAST && dochugw(mtmp))
	    continue;
    }
    if(warnlevel > 0)
	warn_effects();

    dmonsfree(); /* remove all dead monsters */
}

#endif /* OVL1 */
#ifdef OVLB

void
meatgold(mtmp)
	register struct monst *mtmp;
{
	register struct obj *otmp;

	/* If a pet, eating is handled separately, in dog.c */
	if (mtmp->mtame) return;

	/* Eats topmost metal object if it is there */
	for (otmp = level.objects[mtmp->mx][mtmp->my];
						    otmp; otmp = otmp->nexthere)
	    if (is_metallic(otmp) && touch_artifact(otmp,mtmp)) {
		    if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
			pline("%s eats %s!", Monnam(mtmp),
				distant_name(otmp,doname));
		    else if (flags.soundok && flags.verbose)
			You("hear a crunching sound.");
		    mtmp->meating = otmp->owt/2 + 1;
		    /* Heal up to the object's weight in hp */
		    if (mtmp->mhp < mtmp->mhpmax) {
			mtmp->mhp += objects[otmp->otyp].oc_weight;
			if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
		    }
		    if(otmp == uball) {
			unpunish();
			delobj(otmp);
		    } else if(otmp == uchain)
			unpunish();	/* frees uchain */
		    else
			delobj(otmp);
		    /* Left behind a pile? */
		    if(rnd(25) < 3) (void) mksobj_at(ROCK, mtmp->mx, mtmp->my, TRUE);
		    newsym(mtmp->mx, mtmp->my);
		    break;
	    }
}

void
meatobj(mtmp)		/* for gelatinous cubes */
	register struct monst *mtmp;
{
	register struct obj *otmp, *otmp2;

	/* If a pet, eating is handled separately, in dog.c */
	if (mtmp->mtame) return;

	/* Eats organic objects, including cloth and wood, if there */
	/* Engulfs others, except huge rocks and metal attached to player */
	for (otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
	    otmp2 = otmp->nexthere;
	    if(is_organic(otmp) && touch_artifact(otmp,mtmp)) {
		if (otmp->otyp == CORPSE && otmp->corpsenm == PM_COCKATRICE
						&& !resists_ston(mtmp->data))
		    continue;
		if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
		    pline("%s eats %s!", Monnam(mtmp),
			    distant_name(otmp, doname));
		else if (flags.soundok && flags.verbose)
		    You("hear a slurping sound.");
		/* Heal up to the object's weight in hp */
		if (mtmp->mhp < mtmp->mhpmax) {
		    mtmp->mhp += objects[otmp->otyp].oc_weight;
		    if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
		}
		delobj(otmp);		/* munch */
	    } else if (otmp->oclass != ROCK_CLASS &&
				    otmp != uball && otmp != uchain) {
		if (cansee(mtmp->mx, mtmp->my) && flags.verbose)
		    pline("%s engulfs %s.", Monnam(mtmp),
			    distant_name(otmp,doname));
		freeobj(otmp);
		mpickobj(mtmp, otmp);	/* slurp */
	    }
	    /* Engulf & devour is instant, so don't set meating */
	    newsym(mtmp->mx, mtmp->my);
	}
}

void
mpickgold(mtmp)
	register struct monst *mtmp;
{
    register struct obj *gold;

    if ((gold = g_at(mtmp->mx, mtmp->my)) != 0) {
	mtmp->mgold += gold->quan;
	delobj(gold);
	if (cansee(mtmp->mx, mtmp->my) ) {
	    if (flags.verbose && !mtmp->isgd)
		pline("%s picks up some gold.", Monnam(mtmp));
	    newsym(mtmp->mx, mtmp->my);
	}
    }
}

/* Now includes giants which pick up enormous rocks.  KAA */
void
mpickgems(mtmp)
	register struct monst *mtmp;
{
	register struct obj *otmp;

	for(otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp=otmp->nexthere)
	    if(throws_rocks(mtmp->data) ? otmp->otyp == BOULDER :
		(otmp->oclass == GEM_CLASS &&
		 objects[otmp->otyp].oc_material != MINERAL))
		if (touch_artifact(otmp,mtmp))
		if(mtmp->data->mlet != S_UNICORN
			|| objects[otmp->otyp].oc_material == GEMSTONE){
		    if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
			pline("%s picks up %s.", Monnam(mtmp),
				distant_name(otmp, doname));
		    freeobj(otmp);
		    mpickobj(mtmp, otmp);
		    if (otmp->otyp == BOULDER)
			unblock_point(otmp->ox,otmp->oy);	/* vision */
		    newsym(mtmp->mx, mtmp->my);
		    return;	/* pick only one object */
		}
}

#endif /* OVLB */
#ifdef OVL2

void
mpickstuff(mtmp, str)
	register struct monst *mtmp;
	register const char *str;
{
	register struct obj *otmp, *otmp2;

/*	prevent shopkeepers from leaving the door of their shop */
	if(mtmp->isshk && inhishop(mtmp)) return;

	for(otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
	    otmp2 = otmp->nexthere;
/*	Nymphs take everything.  Most monsters don't pick up corpses. */
	    if (
#ifdef MUSE
		!str ? searches_for_item(mtmp,otmp) :
#endif
		  !!(index(str, otmp->oclass))) {
		if (otmp->otyp == CORPSE && mtmp->data->mlet != S_NYMPH
#ifdef MUSE
    && (otmp->corpsenm != PM_COCKATRICE || !(mtmp->misc_worn_check & W_ARMG))
#endif
									)
			continue;
		if (!touch_artifact(otmp,mtmp)) continue;
		if (!can_carry(mtmp,otmp)) continue;
		if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
			pline("%s picks up %s.", Monnam(mtmp), doname(otmp));
		freeobj(otmp);
		mpickobj(mtmp, otmp);
#ifdef MUSE
		m_dowear(mtmp, FALSE);
#endif
		newsym(mtmp->mx, mtmp->my);
		return;			/* pick only one object */
	    }
	}
}

#endif /* OVL2 */
#ifdef OVL0

int
curr_mon_load(mtmp)
register struct monst *mtmp;
{
	register int curload = 0;
	register struct obj *obj;

	for(obj = mtmp->minvent; obj; obj = obj->nobj) {
		if(obj->otyp != BOULDER || !throws_rocks(mtmp->data))
			curload += obj->owt;
	}

	return curload;
}

int
max_mon_load(mtmp)
register struct monst *mtmp;
{
	register long maxload;

	/* Base monster carrying capacity is equal to human maximum
	 * carrying capacity, or half human maximum if not strong.
	 * (for a polymorphed player, the value used would be the
	 * non-polymorphed carrying capacity instead of max/half max).
	 * This is then modified by the ratio between the monster weights
	 * and human weights.  Corpseless monsters are given a capacity
	 * proportional to their size instead of weight.
	 */
	if (!mtmp->data->cwt)
		maxload = (MAX_CARR_CAP * (long)mtmp->data->msize) / MZ_HUMAN;
	else if (!strongmonst(mtmp->data)
		|| (strongmonst(mtmp->data) && (mtmp->data->cwt > WT_HUMAN)))
		maxload = (MAX_CARR_CAP * (long)mtmp->data->cwt) / WT_HUMAN;
	else	maxload = MAX_CARR_CAP; /*strong monsters w/cwt <= WT_HUMAN*/

	if (!strongmonst(mtmp->data)) maxload /= 2;

	if (maxload < 1) maxload = 1;

	return (int) maxload;
}

/* for restricting monsters' object-pickup */
boolean
can_carry(mtmp,otmp)
struct monst *mtmp;
struct obj *otmp;
{
	register int newload = otmp->owt;

	if (otmp->otyp == CORPSE && otmp->corpsenm == PM_COCKATRICE
#ifdef MUSE
			&& !(mtmp->misc_worn_check & W_ARMG)
#endif
						&& !resists_ston(mtmp->data))
		return(FALSE);
	if (mtmp->isshk) return(TRUE); /* no limit */
	if (mtmp->mpeaceful && !mtmp->mtame) return(FALSE);
	/* otherwise players might find themselves obligated to violate
	 * their alignment if the monster takes something they need
	 */

	/* special--boulder throwers carry unlimited amounts of boulders */
	if (throws_rocks(mtmp->data) && otmp->otyp == BOULDER)
		return(TRUE);

	/* nymphs deal in stolen merchandise, but not boulders or statues */
	if (mtmp->data->mlet == S_NYMPH)
		return !(otmp->oclass == ROCK_CLASS);

	if(curr_mon_load(mtmp) + newload > max_mon_load(mtmp)) return(FALSE);

	return(TRUE);
}

/* return number of acceptable neighbour positions */
int
mfndpos(mon, poss, info, flag)
	register struct monst *mon;
	coord *poss;	/* coord poss[9] */
	long *info;	/* long info[9] */
	long flag;
{
	register xchar x,y,nx,ny;
	register int cnt = 0;
	register uchar ntyp;
	uchar nowtyp;
	boolean wantpool,poolok,lavaok,nodiag;
	int maxx, maxy;

	x = mon->mx;
	y = mon->my;
	nowtyp = levl[x][y].typ;

	nodiag = (mon->data == &mons[PM_GRID_BUG]);
	wantpool = mon->data->mlet == S_EEL;
	poolok = is_flyer(mon->data) || is_clinger(mon->data) ||
		 (is_swimmer(mon->data) && !wantpool);
	lavaok = is_flyer(mon->data) || is_clinger(mon->data) ||
		 (mon->data == &mons[PM_FIRE_ELEMENTAL]);
nexttry:	/* eels prefer the water, but if there is no water nearby,
		   they will crawl over land */
	if(mon->mconf) {
		flag |= ALLOW_ALL;
		flag &= ~NOTONL;
	}
	if(!mon->mcansee)
		flag |= ALLOW_SSM;
	maxx = min(x+1,COLNO-1);
	maxy = min(y+1,ROWNO-1);
	for(nx = max(1,x-1); nx <= maxx; nx++)
	  for(ny = max(0,y-1); ny <= maxy; ny++) {
	    if(nx == x && ny == y) continue;
	    if(IS_ROCK(ntyp = levl[nx][ny].typ) &&
	       !((flag & ALLOW_WALL) && may_passwall(nx,ny)) &&
	       !((flag & ALLOW_DIG) && may_dig(nx,ny))) continue;
	    if(IS_DOOR(ntyp) && !amorphous(mon->data) &&
	       ((levl[nx][ny].doormask & D_CLOSED && !(flag & OPENDOOR)) ||
		(levl[nx][ny].doormask & D_LOCKED && !(flag & UNLOCKDOOR))
	       ) && !(flag & (ALLOW_WALL|ALLOW_DIG|BUSTDOOR))) continue;
	    if(nx != x && ny != y && (nodiag ||
#ifdef REINCARNATION
	       ((IS_DOOR(nowtyp) &&
		 ((levl[x][y].doormask & ~D_BROKEN) || Is_rogue_level(&u.uz))) ||
		(IS_DOOR(ntyp) &&
		 ((levl[nx][ny].doormask & ~D_BROKEN) || Is_rogue_level(&u.uz))))
#else
	       ((IS_DOOR(nowtyp) && (levl[x][y].doormask & ~D_BROKEN)) ||
		(IS_DOOR(ntyp) && (levl[nx][ny].doormask & ~D_BROKEN)))
#endif
	       ))
		continue;
	    if((is_pool(nx,ny) == wantpool || poolok) &&
	       (lavaok || !is_lava(nx,ny))) {
		int dispx, dispy;
		boolean monseeu = (!Invis || perceives(mon->data));
		boolean checkobj = OBJ_AT(nx,ny);

		/* Displacement also displaces the Elbereth/scare monster,
		 * as long as you are visible.
		 */
		if(Displaced && monseeu && (mon->mux==nx) && (mon->muy==ny)) {
		    dispx = u.ux;
		    dispy = u.uy;
		} else {
		    dispx = nx;
		    dispy = ny;
		}

		info[cnt] = 0;
		if(((checkobj || Displaced) &&
		    sobj_at(SCR_SCARE_MONSTER, dispx, dispy))
#ifdef ELBERETH
		       || sengr_at("Elbereth", dispx, dispy)
#endif
		       ) {
		    if(!(flag & ALLOW_SSM)) continue;
		    info[cnt] |= ALLOW_SSM;
		}
		if((nx == u.ux && ny == u.uy) ||
		   (nx == mon->mux && ny == mon->muy)) {
			if (nx == u.ux && ny == u.uy) {
				/* If it's right next to you, it found you,
				 * displaced or no.  We must set mux and muy
				 * right now, so when we return we can tell
				 * that the ALLOW_U means to attack _you_ and
				 * not the image.
				 */
				mon->mux = u.ux;
				mon->muy = u.uy;
			}
			if(!(flag & ALLOW_U)) continue;
			info[cnt] |= ALLOW_U;
		} else {
			if(MON_AT(nx, ny)) {
				if(!(flag & ALLOW_M)) continue;
				info[cnt] |= ALLOW_M;
				if((m_at(nx,ny))->mtame) {
					if(!(flag & ALLOW_TM)) continue;
					info[cnt] |= ALLOW_TM;
				}
			}
			/* Note: ALLOW_SANCT only prevents movement, not */
			/* attack, into a temple. */
			if(level.flags.has_temple &&
			   *in_rooms(nx, ny, TEMPLE) &&
			   !*in_rooms(x, y, TEMPLE) &&
			   in_your_sanctuary(nx, ny)){
				if(!(flag & ALLOW_SANCT)) continue;
				info[cnt] |= ALLOW_SANCT;
			}
		}
		if(checkobj && sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
			if(flag & NOGARLIC) continue;
			info[cnt] |= NOGARLIC;
		}
		if(checkobj && sobj_at(BOULDER, nx, ny)) {
			if(!(flag & ALLOW_ROCK)) continue;
			info[cnt] |= ALLOW_ROCK;
		}
		if (monseeu && onlineu(nx,ny)) {
			if(flag & NOTONL) continue;
			info[cnt] |= NOTONL;
		}
		/* we cannot avoid traps of an unknown kind */
		{ register struct trap *ttmp = t_at(nx, ny);
		  register long tt;
			if(ttmp) {
				if(ttmp->ttyp >= TRAPNUM || ttmp->ttyp == 0)  {
impossible("A monster looked at a very strange trap of type %d.", ttmp->ttyp);
					continue;
				}
				tt = 1L << ttmp->ttyp;
				if(mon->mtrapseen & tt) {

					if(!(flag & tt)) continue;
					info[cnt] |= tt;
				}
			}
		}
		poss[cnt].x = nx;
		poss[cnt].y = ny;
		cnt++;
	    }
	}
	if(!cnt && wantpool && !is_pool(x,y)) {
		wantpool = FALSE;
		goto nexttry;
	}
	return(cnt);
}

#endif /* OVL0 */
#ifdef OVL1

boolean
monnear(mon, x, y)
register struct monst *mon;
register int x,y;
/* Is the square close enough for the monster to move or attack into? */
{
	register int distance = dist2(mon->mx, mon->my, x, y);
	if (distance==2 && mon->data==&mons[PM_GRID_BUG]) return 0;
	return (distance < 3);
}

#endif /* OVL1 */
#ifdef OVL2

STATIC_OVL void
dmonsfree()
{
register struct monst *mtmp;
	while ((mtmp = fdmon) != 0) {
		fdmon = mtmp->nmon;
		dealloc_monst(mtmp);
	}
}

#endif /* OVL2 */
#ifdef OVLB
/* we do not free monsters immediately, in order to have their name
   available shortly after their demise */
void
monfree(mtmp)
register struct monst *mtmp;
{
	mtmp->nmon = fdmon;
	fdmon = mtmp;
}

/* called when monster is moved to larger structure */
void
replmon(mtmp, mtmp2)
register struct monst *mtmp, *mtmp2;
{
    relmon(mtmp);
    monfree(mtmp);
    place_monster(mtmp2, mtmp2->mx, mtmp2->my);
    if (mtmp2->wormno)	    /* update level.monsters[wseg->wx][wseg->wy] */
	place_wsegs(mtmp2); /* locations to mtmp2 not mtmp. */
    mtmp2->nmon = fmon;
    fmon = mtmp2;
    if (u.ustuck == mtmp) u.ustuck = mtmp2;
    if (mtmp2->isshk) replshk(mtmp,mtmp2);
}

/* release mon from display and monster list */
void
relmon(mon)
register struct monst *mon;
{
	register struct monst *mtmp;

	if (fmon == (struct monst *)0)  panic ("relmon: no fmon available.");

	remove_monster(mon->mx, mon->my);

	if(mon == fmon) fmon = fmon->nmon;
	else {
		for(mtmp = fmon; mtmp && mtmp->nmon != mon; mtmp = mtmp->nmon) ;
		if(mtmp)    mtmp->nmon = mon->nmon;
		else	    panic("relmon: mon not in list.");
	}
}

/* remove effects of mtmp from other data structures */
static void
m_detach(mtmp)
register struct monst *mtmp;
{
#ifdef WALKIES
	if(mtmp->mleashed) m_unleash(mtmp);
#endif
	    /* to prevent an infinite relobj-flooreffects-hmon-killed loop */
	mtmp->mtrapped = 0;
	mtmp->mhp = 0; /* simplify some tests: force mhp to 0 */
	relobj(mtmp, 0, FALSE);
	relmon(mtmp);
	newsym(mtmp->mx,mtmp->my);
	unstuck(mtmp);
	fill_pit(mtmp->mx, mtmp->my);

	if(mtmp->isshk) shkgone(mtmp);
	if(mtmp->wormno) wormgone(mtmp);
}

#ifdef MUSE
static void FDECL(lifesaved_monster, (struct monst *));

static void
lifesaved_monster(mtmp)
struct monst *mtmp;
{
	struct obj *lifesave;

	if ((lifesave = which_armor(mtmp, W_AMUL))
			&& lifesave->otyp == AMULET_OF_LIFE_SAVING) {
		pline("But wait...");
		if (canseemon(mtmp)) {
			pline("%s's medallion begins to glow!",
				Monnam(mtmp));
			makeknown(AMULET_OF_LIFE_SAVING);
			pline("%s looks much better!", Monnam(mtmp));
			pline("The medallion crumbles to dust!");
		} else
			pline("Maybe not...");
		m_useup(mtmp, lifesave);
		if (mtmp->mhpmax <= 0) mtmp->mhpmax = 10;
		mtmp->mhp = mtmp->mhpmax;
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		if (mtmp->mtame && !mtmp->isminion) {
			struct edog *edog = EDOG(mtmp);
			if (edog->hungrytime < moves+500)
				edog->hungrytime = moves+500;
		}
		if (mtmp->data->geno & G_GENOD)
			pline("Unfortunately %s is still genocided...",
				mon_nam(mtmp));
		else
			return;
	}
	mtmp->mhp = 0;
}
#endif

void
mondead(mtmp)
register struct monst *mtmp;
{
	int tmp, nk;

	if(mtmp->isgd) {
		/* if we're going to abort the death, it *must* be before
		 * the m_detach or there will be relmon problems later */
		if(!grddead(mtmp)) return;
	}
#ifdef MUSE
	lifesaved_monster(mtmp);
	if (mtmp->mhp > 0) return;
#endif

	/* restore chameleon, lycanthropes to true form at death */
	if(mtmp->cham) mtmp->data = &mons[PM_CHAMELEON];
	if(mtmp->data == &mons[PM_WEREJACKAL])
		mtmp->data = &mons[PM_HUMAN_WEREJACKAL];
	if(mtmp->data == &mons[PM_WEREWOLF])
		mtmp->data = &mons[PM_HUMAN_WEREWOLF];
	if(mtmp->data == &mons[PM_WERERAT])
		mtmp->data = &mons[PM_HUMAN_WERERAT];

	/* if MAXMONNO monsters of a given type have died, and it
	 * can be done, extinguish that monster.
	 *
	 * u.nr_killed does double duty as total number of dead monsters
	 * and as experience factor for the player killing more monsters.
	 * this means that a dragon dying by other means reduces the
	 * experience the player gets for killing a dragon directly; this
	 * is probably not too bad, since the player likely finagled the
	 * first dead dragon via ring of conflict or pets, and extinguishing
	 * based on only player kills probably opens more avenues of abuse
	 * for rings of conflict and such.
	 */
	tmp = monsndx(mtmp->data);
	u.nr_killed[tmp]++;
	nk = u.nr_killed[tmp];
	if(nk > (tmp == PM_NAZGUL ? 9 : tmp == PM_ERINYES ? 3 : MAXMONNO) &&
				!(mons[tmp].geno & (G_NOGEN | G_EXTINCT))) {
#ifdef DEBUG
		pline("Automatically extinguished %s.", makeplural(mons[tmp].mname));
#endif
		mons[tmp].geno |= G_EXTINCT;
	}
#ifdef MAIL
	/* if the mail daemon dies, no more mail delivery.  -3. */
	else if(tmp==PM_MAIL_DAEMON) mons[tmp].geno |= G_GENOD;
#endif

#ifdef KOPS
	if(mtmp->data->mlet == S_KOP && allow_kops) {
	    /* Dead Kops may come back. */
	    switch(rnd(5)) {
		case 1:	     /* returns near the stairs */
			(void) makemon(mtmp->data,xdnstair,ydnstair);
			break;
		case 2:	     /* randomly */
			(void) makemon(mtmp->data,0,0);
			break;
		default:
			break;
	    }
	}
#endif
	if(mtmp->iswiz) wizdead(mtmp);
#ifdef MULDGN
	if(mtmp->data->msound == MS_NEMESIS) nemdead();
#endif
	m_detach(mtmp);
	monfree(mtmp);
}

/* drop (perhaps) a cadaver and remove monster */
void
mondied(mdef)
register struct monst *mdef;
{
	mondead(mdef);
	if(rn2(3)
#ifdef REINCARNATION
	   && !Is_rogue_level(&u.uz)
#endif
	   && !(level.flags.graveyard && is_undead(mdef->data) && rn2(3)))
		(void) make_corpse(mdef);
}

/* monster disappears, not dies */
void
mongone(mdef)
register struct monst *mdef;
{
	register struct obj *otmp, *otmp2;

	/* release monster's inventory */
	for (otmp = mdef->minvent; otmp; otmp = otmp2) {
		otmp2 = otmp->nobj;
		obfree(otmp, (struct obj *)0);
	}
	mdef->minvent = 0;
	mdef->mgold = 0;
	m_detach(mdef);
	monfree(mdef);
}

/* drop a statue or rock and remove monster */
void
monstone(mdef)
register struct monst *mdef;
{
	struct obj *otmp, *obj, *nxt, *contents;
	xchar x = mdef->mx, y = mdef->my;

#ifdef MUSE
	/* we have to make the statue before calling mondead, to be able to
	 * put inventory in it, and we have to check for lifesaving before
	 * making the statue....
	 */
	lifesaved_monster(mdef);
	if (mdef->mhp > 0) return;
#endif
	mdef->mtrapped = 0;	/* (see m_detach) */

	if((int)mdef->data->msize > MZ_TINY ||
	   !rn2(2 + ((mdef->data->geno & G_FREQ) > 2))) {
		otmp = mk_named_object(STATUE, mdef->data, x, y,
					NAME(mdef), (int)mdef->mnamelth);
		/* some objects may end up outside the statue */
		contents = 0;
		for (obj = mdef->minvent; obj; obj = nxt) {
		    nxt = obj->nobj;
		    obj->owornmask = 0L;
		    if (obj->otyp == BOULDER ||
#if 0				/* monsters don't carry statues */
     (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
				obj_resists(obj, 0, 0)) {
			if (flooreffects(obj, x, y, "fall")) continue;
			place_object(obj, x, y);
			obj->nobj = fobj;
			fobj = obj;
		    } else {
			obj->nobj = contents;
			contents = obj;
		    }
		}
		otmp->cobj = contents;
		mdef->minvent = 0;
		if (mdef->mgold) {
			struct obj *au;
			au = mksobj(GOLD_PIECE, FALSE, FALSE);
			au->quan = mdef->mgold;
			au->owt = weight(au);
			mdef->mgold = 0;
			au->nobj = otmp->cobj;
			otmp->cobj = au;
		}
		otmp->owt = weight(otmp);
	} else
		otmp = mksobj_at(ROCK, x, y, TRUE);

	stackobj(otmp);
	if (cansee(x, y)) newsym(x,y);
	mondead(mdef);
}

/* another monster has killed the monster mdef */
void
monkilled(mdef, fltxt, how)
register struct monst *mdef;
const char *fltxt;
int how;
{
	if (cansee(mdef->mx, mdef->my) && fltxt)
	    pline("%s is %s%s%s!", Monnam(mdef),
		    (is_demon(mdef->data) || is_undead(mdef->data)) ?
			 "destroyed" : "killed",
		    *fltxt ? " by the " : "",
		    fltxt
		 );
	else if(mdef->mtame)
	    You("have a sad feeling for a moment, then it passes.");

	/* no corpses if digested */
	if(how == AD_DGST)
	    mondead(mdef);
	else
	    mondied(mdef);
}

void
unstuck(mtmp)
register struct monst *mtmp;
{
	if(u.ustuck == mtmp) {
		if(u.uswallow){
			u.ux = mtmp->mx;
			u.uy = mtmp->my;
			u.uswallow = 0;
			u.uswldtim = 0;
			if (Punished) placebc();
			vision_full_recalc = 1;
			docrt();
		}
		u.ustuck = 0;
	}
}

void
killed(mtmp)
register struct monst *mtmp;
{
	xkilled(mtmp, 1);
}

/* the player has killed the monster mtmp */
void
xkilled(mtmp, dest)
	register struct monst *mtmp;
/*
 * Dest=1, normal; dest=0, don't print message; dest=2, don't drop corpse
 * either; dest=3, message but no corpse
 */
	int	dest;
{
	register int tmp, x = mtmp->mx, y = mtmp->my;
	register struct permonst *mdat;
	register struct obj *otmp;
	register struct trap *t;
	boolean chance, redisp = FALSE;
	boolean wasinside = u.uswallow && (u.ustuck == mtmp);

	if (dest & 1) {
	    if(!canseemon(mtmp) && !sensemon(mtmp)) You("destroy it!");
	    else {
		You("destroy %s!",
			mtmp->mtame ? x_monnam(mtmp, 0, "poor", 0)
			: mon_nam(mtmp));
	    }
	}

	if (mtmp->mtrapped &&
	    ((t = t_at(x, y)) && (t->ttyp == PIT || t->ttyp == SPIKED_PIT)) &&
	    sobj_at(BOULDER, x, y))
		dest ^= 2; /*
			    * Prevent corpses/treasure being created "on top"
			    * of the boulder that is about to fall in. This is
			    * out of order, but cannot be helped unless this
			    * whole routine is rearranged.
			    */

	/* dispose of monster and make cadaver */
	if(stoned) monstone(mtmp);
	else mondead(mtmp);

#ifdef MUSE
	if (mtmp->mhp > 0) return; /* monster lifesaved */
#endif

	mdat = mtmp->data; /* note: mondead can change mtmp->data */

	if (stoned) {
		stoned = FALSE;
		goto cleanup;
	}

	if((dest & 2)
#ifdef REINCARNATION
		 || Is_rogue_level(&u.uz)
#endif
	   || (level.flags.graveyard && is_undead(mdat) && rn2(3)))
		goto cleanup;

#ifdef MAIL
	if(mdat == &mons[PM_MAIL_DAEMON]) {
		(void) mksobj_at(SCR_MAIL, x, y, FALSE);
		stackobj(fobj);
		redisp = TRUE;
	}
#endif
	if(!accessible(x, y)) {
	    /* might be mimic in wall or dead eel or in a pool or lava */
	    redisp = TRUE;
	    if(wasinside) spoteffects();
	} else if(x != u.ux || y != u.uy) {
		/* might be here after swallowed */
		if (!rn2(6) && !(mdat->geno & G_NOCORPSE)
#ifdef KOPS
					&& mdat->mlet != S_KOP
#endif
							) {
			int typ;

			otmp = mkobj_at(RANDOM_CLASS, x, y, TRUE);
			/* Don't create large objects from small monsters */
			typ = otmp->otyp;
			if (mdat->msize < MZ_HUMAN && typ != FOOD_RATION
#ifdef WALKIES
			    && typ != LEASH
#endif
			    && typ != FIGURINE
			    && (otmp->owt > 3 ||
				(typ >= SPEAR && typ <= LANCE) ||
				(typ >= SCIMITAR && typ <= KATANA) ||
				(typ == MORNING_STAR || typ == QUARTERSTAFF) ||
				(typ >= BARDICHE && typ <= VOULGE) ||
				(typ >= PLATE_MAIL &&
					    typ <= YELLOW_DRAGON_SCALE_MAIL) ||
				(typ == LARGE_SHIELD))) {
			    delobj(otmp);
			} else redisp = TRUE;
		}
		/* Whether or not it always makes a corpse is, in theory,
		 * different from whether or not the corpse is "special";
		 * if we want both, we have to specify it explicitly.
		 */
		if (bigmonst(mdat) || mdat == &mons[PM_LIZARD]
			   || is_golem(mdat)
			   || is_mplayer(mdat)
			   || is_rider(mdat))
			chance = 1;
		else chance = !rn2((int)
			(2 + ((mdat->geno & G_FREQ)<2) + verysmall(mdat)));
		if (chance)
			(void) make_corpse(mtmp);
	}
	if(redisp) newsym(x,y);
cleanup:
	/* punish bad behaviour */
	if(is_human(mdat) && (!always_hostile(mdat) && mtmp->malign <= 0) &&
	   (monsndx(mdat) < PM_ARCHEOLOGIST || monsndx(mdat) > PM_WIZARD) &&
	   u.ualign.type != A_CHAOTIC) {
		HTelepat &= ~INTRINSIC;
		change_luck(-2);
		if (Blind && !Telepat)
		    see_monsters(); /* Can't sense monsters any more. */
	}
	if((mtmp->mpeaceful && !rn2(2)) || mtmp->mtame)	change_luck(-1);
	if (mdat->mlet == S_UNICORN &&
				sgn(u.ualign.type) == sgn(mdat->maligntyp))
		change_luck(-5);

	/* give experience points */
	tmp = experience(mtmp, u.nr_killed[monsndx(mdat)] + 1);
	more_experienced(tmp, 0);
	newexplevel();		/* will decide if you go up */

	/* adjust alignment points */
#ifdef MULDGN
	if(mdat->msound == MS_LEADER)		/* REAL BAD! */
	    adjalign(-(u.ualign.record+(int)ALIGNLIM/2));
	else if(mdat->msound == MS_NEMESIS)	/* Real good! */
	    adjalign((int)(ALIGNLIM/4));
	else if(mdat->msound == MS_GUARDIAN)	/* Bad */
	    adjalign(-(int)(ALIGNLIM/8));
	else
#endif
	    if (mtmp->ispriest) {
		adjalign((p_coaligned(mtmp)) ? -2 : 2);
		if(mdat->maligntyp == A_NONE)
			adjalign((int)(ALIGNLIM / 4));		/* BIG bonus */
	} else if(mtmp->mtame)
		adjalign(-15);	/* bad!! */
	else if (mtmp->mpeaceful)
		adjalign(-5);

	/* malign was already adjusted for u.ualign.type and randomization */
	adjalign(mtmp->malign);
}

/* changes the monster into a stone monster of the same type */
/* this should only be called when poly_when_stoned() is true */
void
mon_to_stone(mtmp)
    register struct monst *mtmp;
{
    if(mtmp->data->mlet == S_GOLEM) {
	/* it's a golem, and not a stone golem */
	if(canseemon(mtmp))
	    pline("%s solidifies...", Monnam(mtmp));
	(void) newcham(mtmp, &mons[PM_STONE_GOLEM]);
	if(canseemon(mtmp))
	    pline("Now it's %s", a_monnam(mtmp));
    } else
	impossible("Can't polystone %s", a_monnam(mtmp));
}

void
mnexto(mtmp)	/* Make monster mtmp next to you (if possible) */
	struct monst *mtmp;
{
	coord mm;

	if(!enexto(&mm, u.ux, u.uy, mtmp->data)) return;

	rloc_to(mtmp, mm.x, mm.y);
}

/* mnearto()
 * Put monster near (or at) location if possible.
 * Returns:
 *	1 - if a monster was moved from x, y to put mtmp at x, y.
 *	0 - in most cases.
 */
boolean
mnearto(mtmp,x,y,move_other)
register struct monst *mtmp;
xchar x, y;
boolean move_other;	/* make sure mtmp gets to x, y! so move m_at(x, y) */
{
	struct monst *othermon = (struct monst *)0;
	xchar newx, newy;
	coord mm;

	if ((mtmp->mx == x) && (mtmp->my == y)) return(FALSE);

	if (move_other && (othermon = m_at(x, y))) {
		if (othermon->wormno)
			remove_worm(othermon);
		else
			remove_monster(x, y);
	}

	newx = x;
	newy = y;

	if (!goodpos(newx, newy, mtmp, mtmp->data)) {
		/* actually we have real problems if enexto ever fails.
		 * migrating_mons that need to be placed will cause
		 * no end of trouble.
		 */
		if (!enexto(&mm, newx, newy, mtmp->data)) return(FALSE);
		newx = mm.x; newy = mm.y;
	}

	rloc_to(mtmp, newx, newy);

	if (move_other && othermon) {
	    othermon->mx = othermon->my = 0;
	    (void) mnearto(othermon, x, y, FALSE);
	    if ((othermon->mx != x) || (othermon->my != y))
		return(TRUE);
	}

	return(FALSE);
}


static const char *poiseff[] = {

	" feel very weak", "r brain is on fire",
	"r judgement is impaired", "r muscles won't obey you",
	" feel very sick", " break out in hives"
};

void
poisontell(typ)

	int	typ;
{
	pline("You%s.", poiseff[typ]);
}

void
poisoned(string, typ, pname, fatal)
register const char *string, *pname;
register int  typ, fatal;
{
	register int i, plural;
	boolean thrown_weapon = !strncmp(string, "poison", 6);
		/* admittedly a kludge... */

	if(strcmp(string, "blast") && !thrown_weapon) {
	    /* 'blast' has already given a 'poison gas' message */
	    /* so have "poison arrow", "poison dart", etc... */
	    plural = (string[strlen(string) - 1] == 's')? 1 : 0;
	    /* avoid "The" Orcus's sting was poisoned... */
	    pline("%s%s %s poisoned!", isupper(*string) ? "" : "The ",
			string, plural ? "were" : "was");
	}

	if(Poison_resistance) {
		if(!strcmp(string, "blast")) shieldeff(u.ux, u.uy);
		pline("The poison doesn't seem to affect you.");
		return;
	}
	i = rn2(fatal + 20*thrown_weapon);
	if(i == 0 && typ != A_CHA) {
		u.uhp = -1;
		pline("The poison was deadly...");
	} else if(i <= 5) {
		pline("You%s!", poiseff[typ]);
		(void) adjattrib(typ, thrown_weapon ? -1 : -rn1(3,3), TRUE);
	} else {
		i = thrown_weapon ? rnd(6) : rn1(10,6);
		if(Half_physical_damage) i = (i+1) / 2;
		losehp(i, pname, KILLED_BY_AN);
	}
	if(u.uhp < 1) {
		killer_format = KILLED_BY_AN;
		killer = pname;
		done(POISONING);
	}
}

/* monster responds to player action; not the same as a passive attack */
/* assumes reason for response has been tested, and response _must_ be made */
void
m_respond(mtmp)
register struct monst *mtmp;
{
    if(mtmp->data->msound == MS_SHRIEK) {
	if(flags.soundok)
	    pline("%s shrieks.", Monnam(mtmp));
	if (!rn2(10)) {
	    if (!rn2(13))
		(void) makemon(&mons[PM_PURPLE_WORM], 0, 0);
	    else
		(void) makemon((struct permonst *)0, 0, 0);

	}
	aggravate();
    }
    if(mtmp->data == &mons[PM_MEDUSA] && !mtmp->mcan) {
	register int i;
	for(i = 0; i < NATTK; i++)
	     if(mtmp->data->mattk[i].aatyp == AT_GAZE) {
		 (void) gazemu(mtmp, &mtmp->data->mattk[i]);
		 break;
	     }
    }
}

#endif /* OVLB */
#ifdef OVL2

void
setmangry(mtmp)
register struct monst *mtmp;
{
	mtmp->data->mflags3 &= ~M3_WAITMASK;
	if(!mtmp->mpeaceful) return;
	if(mtmp->mtame) return;
	mtmp->mpeaceful = 0;
	if(mtmp->ispriest) {
		if(p_coaligned(mtmp)) adjalign(-5); /* very bad */
		else adjalign(2);
	} else
		adjalign(-1);		/* attacking peaceful monsters is bad */
	if(humanoid(mtmp->data) || mtmp->isshk || mtmp->isgd)
		pline("%s gets angry!", Monnam(mtmp));
#ifdef SOUNDS
	else if (flags.verbose && flags.soundok) growl(mtmp);
#endif
}

void
wakeup(mtmp)
register struct monst *mtmp;
{
	mtmp->msleep = 0;
	mtmp->meating = 0;	/* assume there's no salvagable food left */
	setmangry(mtmp);
	if(mtmp->m_ap_type) seemimic(mtmp);
}

/* Wake up nearby monsters. */
void
wake_nearby()
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (distu(mtmp->mx,mtmp->my) < u.ulevel*20) {
		if(mtmp->msleep)  mtmp->msleep = 0;
		if(mtmp->mtame)   EDOG(mtmp)->whistletime = moves;
	    }
	}
}

/* NOTE: we must check for mimicry before calling this routine */
void
seemimic(mtmp)
register struct monst *mtmp;
{
	/*
	 *  Discovered mimics don't block light.
	 */
	if ((mtmp->m_ap_type == M_AP_FURNITURE &&
		(mtmp->mappearance==S_hcdoor || mtmp->mappearance==S_vcdoor))||
	    (mtmp->m_ap_type == M_AP_OBJECT && mtmp->mappearance == BOULDER))
	    unblock_point(mtmp->mx,mtmp->my);

	mtmp->m_ap_type = M_AP_NOTHING;
	mtmp->mappearance = 0;
	newsym(mtmp->mx,mtmp->my);
}

/* force all chameleons to become normal */
void
rescham()
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if(mtmp->cham) {
			mtmp->cham = 0;
			(void) newcham(mtmp, &mons[PM_CHAMELEON]);
		}
		if(is_were(mtmp->data) && mtmp->data->mlet != S_HUMAN)
			new_were(mtmp);
		if(mtmp->m_ap_type && cansee(mtmp->mx, mtmp->my)) {
			seemimic(mtmp);
			/* we pretend that the mimic doesn't */
			/* know that it has been unmasked.   */
			mtmp->msleep = 1;
		}
	}
}

/* Let the chameleons change again -dgk */
void
restartcham()
{
	register struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (mtmp->data == &mons[PM_CHAMELEON])
			mtmp->cham = 1;
		if(mtmp->data->mlet == S_MIMIC && mtmp->msleep &&
				cansee(mtmp->mx, mtmp->my)) {
			set_mimic_sym(mtmp);
			newsym(mtmp->mx,mtmp->my);
		}
	}
}

/* unwatched hiders may hide again; if so, a 1 is returned.  */
STATIC_OVL boolean
restrap(mtmp)
register struct monst *mtmp;
{
	if(mtmp->cham || mtmp->mcan || mtmp->m_ap_type ||
	   cansee(mtmp->mx, mtmp->my) || rn2(3) || (mtmp == u.ustuck))
		return(FALSE);

	if(mtmp->data->mlet == S_MIMIC) {
		set_mimic_sym(mtmp);
		return(TRUE);
	} else
	    if(levl[mtmp->mx][mtmp->my].typ == ROOM)  {
		mtmp->mundetected = 1;
		return(TRUE);
	    }

	return(FALSE);
}

/* make a chameleon look like a new monster; returns 1 if it actually changed */
int
newcham(mtmp, mdat)
register struct monst *mtmp;
register struct permonst *mdat;
{
	register int mhp, hpn, hpd;
	int tryct;
	struct permonst *olddata = mtmp->data;

	/* mdat = 0 -> caller wants a random monster shape */
	tryct = 0;
	if(mdat == 0) {
		while (++tryct < 100) {
			mdat = &mons[rn2(NUMMONS)];
			/* polyok rules out all M2_PNAME and M2_WERE's */
			if (!is_human(mdat) && polyok(mdat)
					&& !(mdat->geno & G_GENOD))
				break;
		}
		if (tryct >= 100) return(0); /* Should never happen */
	}

	if(is_male(mdat)) {
		if(mtmp->female) mtmp->female = FALSE;
	} else if (is_female(mdat)) {
		if(!mtmp->female) mtmp->female = TRUE;
	} else if (!is_neuter(mdat)) {
		if(!rn2(10)) mtmp->female = !mtmp->female;
	}

	if (In_endgame(&u.uz) && is_mplayer(olddata)) {
		/* mplayers start out as "Foo the Bar", but some of the
		 * titles are inappropriate when polymorphed, particularly
		 * into the opposite sex.  players don't use ranks when
		 * polymorphed, so dropping the rank for mplayers seems
		 * reasonable.
		 */
		char *p = index(NAME(mtmp), ' ');
		if (p) {
			*p = '\0';
			mtmp->mnamelth = p - NAME(mtmp) + 1;
		}
	}

	if(mdat == mtmp->data) return(0);	/* still the same monster */

	if(mtmp->wormno) {			/* throw tail away */
		wormgone(mtmp);
		place_monster(mtmp, mtmp->mx, mtmp->my);
	}

	hpn = mtmp->mhp;
	hpd = (mtmp->m_lev < 50) ? ((int)mtmp->m_lev)*8 : mdat->mlevel;
	if(!hpd) hpd = 4;

	mtmp->m_lev = adj_lev(mdat);		/* new monster level */

	mhp = (mtmp->m_lev < 50) ? ((int)mtmp->m_lev)*8 : mdat->mlevel;
	if(!mhp) mhp = 4;

	/* new hp: same fraction of max as before */
#ifndef LINT
	mtmp->mhp = (int)(((long)hpn*(long)mhp)/(long)hpd);
#endif
	if(mtmp->mhp < 0) mtmp->mhp = hpn;	/* overflow */
/* Unlikely but not impossible; a 1HD creature with 1HP that changes into a
   0HD creature will require this statement */
	if (!mtmp->mhp) mtmp->mhp = 1;

/* and the same for maximum hit points */
	hpn = mtmp->mhpmax;
#ifndef LINT
	mtmp->mhpmax = (int)(((long)hpn*(long)mhp)/(long)hpd);
#endif
	if(mtmp->mhpmax < 0) mtmp->mhpmax = hpn;	/* overflow */
	if (!mtmp->mhpmax) mtmp->mhpmax = 1;

	mtmp->data = mdat;
	mtmp->minvis = !!(mdat->mlet == S_STALKER);
	if (!hides_under(mdat) || !OBJ_AT(mtmp->mx, mtmp->my))
		mtmp->mundetected = 0;
	if (u.ustuck == mtmp) {
		if(u.uswallow) {
			if(!attacktype(mdat,AT_ENGL)) {
				/* Does mdat care? */
				if (!noncorporeal(mdat) && !amorphous(mdat) &&
				    !is_whirly(mdat) &&
				    (mdat != &mons[PM_YELLOW_LIGHT])) {
					You("break out of %s%s!", mon_nam(mtmp),
					    (is_animal(mdat)?
					    "'s stomach" : ""));
					mtmp->mhp = 1;  /* almost dead */
				}
				expels(mtmp, olddata, FALSE);
			}
		} else {
			if(!sticks(mdat)
#ifdef POLYSELF
				&& !sticks(uasmon)
#endif
				)
				unstuck(mtmp);
		}
	}

	if ( (mdat == &mons[PM_LONG_WORM]) && (mtmp->wormno = get_wormno()) ) {
	    /* we can now create worms with tails - 11/91 */
	    initworm(mtmp, rn2(5));
	    if (count_wsegs(mtmp))
		place_worm_tail_randomly(mtmp, mtmp->mx, mtmp->my);
	}

	newsym(mtmp->mx,mtmp->my);
#ifdef MUSE
	mon_break_armor(mtmp);
	/* Unfortunately, by now we forgot who did the polymorph, so we don't
	 * have any way to give the player credit if this was a polymorph wand.
	 */
	mselftouch(mtmp, "No longer petrify-resistant, ", FALSE);
	possibly_unwield(mtmp);
#endif
	return(1);
}

#endif /* OVL2 */
#ifdef OVLB

void
golemeffects(mon, damtype, dam)
register struct monst *mon;
int damtype, dam;
{
	int heal=0, slow=0;

	if (mon->data != &mons[PM_FLESH_GOLEM]
					&& mon->data != &mons[PM_IRON_GOLEM])
		return;

	if (mon->data == &mons[PM_FLESH_GOLEM]) {
		if (damtype == AD_ELEC) heal = dam / 6;
		else if (damtype == AD_FIRE || damtype == AD_COLD) slow = 1;
	} else {
		if (damtype == AD_ELEC) slow = 1;
		else if (damtype == AD_FIRE) heal = dam;
	}
	if (slow) {
		if (mon->mspeed != MSLOW) {
			if (mon->mspeed == MFAST) mon->mspeed = 0;
			else mon->mspeed = MSLOW;
			if (cansee(mon->mx, mon->my))
				pline("%s seems to be moving slower.",
					Monnam(mon));
		}
	}
	if (heal) {
		if (mon->mhp < mon->mhpmax) {
			mon->mhp += dam;
			if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
			if (cansee(mon->mx, mon->my))
				pline("%s seems healthier.", Monnam(mon));
		}
	}
}

boolean
angry_guards(silent)
register boolean silent;
{
	register struct monst *mtmp;
	register int ct = 0, nct = 0, sct = 0, slct = 0;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if((mtmp->data == &mons[PM_WATCHMAN] ||
			       mtmp->data == &mons[PM_WATCH_CAPTAIN])
					&& mtmp->mpeaceful) {
			ct++;
			if(cansee(mtmp->mx, mtmp->my) && mtmp->mcanmove) {
				if (distu(mtmp->mx, mtmp->my) == 2) nct++;
				else sct++;
			}
			if(mtmp->msleep || mtmp->mfrozen) {
				slct++;
				mtmp->msleep = mtmp->mfrozen = 0;
			}
			mtmp->mpeaceful = 0;
		}
	}
	if(ct) {
	    if(!silent) { /* do we want pline msgs? */
		if(slct) pline("The guard%s wake%s up!",
				 slct > 1 ? "s" : "", slct == 1 ? "s" : "");
		if(nct || sct) {
			if(nct) pline("The guard%s get%s angry!",
				nct == 1 ? "" : "s", nct == 1 ? "s" : "");
			else if(!Blind)
				You("see %sangry guard%s approaching!",
				  sct == 1 ? "an " : "", sct > 1 ? "s" : "");
		} else if(flags.soundok)
			You("hear the shrill sound of a guard's whistle.");
	    }
	    return(TRUE);
	}
	return(FALSE);
}

void
pacify_guards()
{
	register struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (mtmp->data == &mons[PM_WATCHMAN] ||
		mtmp->data == &mons[PM_WATCH_CAPTAIN])
	    mtmp->mpeaceful = 1;
	}
}
#endif /* OVLB */

/*mon.c*/
