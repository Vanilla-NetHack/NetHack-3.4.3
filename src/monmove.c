/*	SCCS Id: @(#)monmove.c	3.1	93/05/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h"
#include "artifact.h"

#ifdef OVL0

static int FDECL(disturb,(struct monst *));
static void FDECL(distfleeck,(struct monst *,int *,int *,int *));
STATIC_DCL boolean FDECL(mdig_tunnel,(struct monst *));
static void FDECL(watch_on_duty,(struct monst *));

#endif /* OVL0 */
#ifdef OVLB

boolean /* TRUE : mtmp died */
mb_trapped(mtmp)
register struct monst *mtmp;
{
	if (flags.verbose) {
	    if (cansee(mtmp->mx, mtmp->my))
	       pline("KABOOM!!  You see a door explode.");
	    else if (flags.soundok)
               You("hear a distant explosion.");
	}
	mtmp->mstun = 1;
	mtmp->mhp -= rnd(15);
	if(mtmp->mhp <= 0) {
		mondied(mtmp);
#ifdef MUSE
		if (mtmp->mhp > 0) /* lifesaved */
			return(FALSE);
		else
#endif
			return(TRUE);
	}
	return(FALSE);
}

#endif /* OVLB */
#ifdef OVL0

static void
watch_on_duty(mtmp)
register struct monst *mtmp;
{
	register s_level *slev = Is_special(&u.uz);
	int	x, y;

	if(slev && slev->flags.town && mtmp->mpeaceful &&
	   m_canseeu(mtmp) && !rn2(3)) {

	    if(picking_lock(&x, &y) && IS_DOOR(levl[x][y].typ) &&
	       (levl[x][y].doormask & D_LOCKED)) {

		if(couldsee(mtmp->mx, mtmp->my)) {

		  pline("%s yells:", Amonnam(mtmp));
		  if(levl[x][y].looted & D_WARNED) {
			verbalize("Halt, thief!  You're under arrest!");
			(void) angry_guards(!(flags.soundok));
		  } else {
			verbalize("Hey, stop picking that lock!");
			levl[x][y].looted |=  D_WARNED;
		  }
		  stop_occupation();
		}
	    }
	}
}

/* Return TRUE if monster died, FALSE otherwise. */
STATIC_OVL boolean
mdig_tunnel(mtmp)
register struct monst *mtmp;
{
	register struct rm *here;
	register int pile;

	here = &levl[mtmp->mx][mtmp->my];
	if (here->typ == SDOOR) here->typ = DOOR;

	/* Eats away door if present & closed or locked */
	if(closed_door(mtmp->mx, mtmp->my)) {
	    if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
		add_damage(mtmp->mx, mtmp->my, 0L);
	    unblock_point(mtmp->mx,mtmp->my);	/* vision */
	    if(here->doormask & D_TRAPPED) {
		here->doormask = D_NODOOR;
		if(mb_trapped(mtmp)) {	/* mtmp is killed */
		    newsym(mtmp->mx,mtmp->my);
		    return TRUE;
		}
	    } else {
		if(!rn2(3) && flags.verbose)	/* not too often.. */
		    You("feel an unexpected draft.");
		here->doormask = D_BROKEN;
	    }
	    newsym(mtmp->mx,mtmp->my);
	    return FALSE;
	} else
	if (!IS_ROCK(here->typ)) /* no dig */
	    return(FALSE);

	/* Only rock and walls fall through to this point. */
	if ((here->diggable & W_NONDIGGABLE)) {
	    impossible("mdig_tunnel:  %s at (%d,%d) is undiggable",
		       (IS_WALL(here->typ) ? "wall" : "stone"),
		       (int) mtmp->mx, (int) mtmp->my);
		return FALSE;	/* still alive */
	}

	if(IS_WALL(here->typ)) {
	    if(flags.soundok && flags.verbose && !rn2(5))
		You("hear crashing rock.");
	    if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
	    	add_damage(mtmp->mx, mtmp->my, 0L);
	    if (level.flags.is_maze_lev) {
		here->typ = ROOM;
	    } else if (level.flags.is_cavernous_lev) {
		here->typ = CORR;
	    } else {
		here->typ = DOOR;
		here->doormask = D_NODOOR;
	    }
	} else	
	    here->typ = CORR;

	pile = rnd(12);
	if(pile < 5)	/* leave behind some rocks? */
		(void) mksobj_at((pile == 1)? BOULDER : ROCK, 
				 mtmp->mx, mtmp->my, TRUE);
	newsym(mtmp->mx,mtmp->my);
	if(sobj_at(BOULDER, mtmp->mx, mtmp->my) == (struct obj *)0)
	    unblock_point(mtmp->mx,mtmp->my);	/* vision */
	return FALSE ;
}

#endif /* OVL0 */
#ifdef OVL1

int
dochugw(mtmp)
	register struct monst *mtmp;
{
	register int x = mtmp->mx;
	register int y = mtmp->my;
	register int rd = dochug(mtmp);
	register int dd;

	if(Warning && !rd && !mtmp->mpeaceful &&
			(dd = distu(mtmp->mx,mtmp->my)) < distu(x,y) &&
			dd < 100 && !canseemon(mtmp)) {
	    /* Note: this assumes we only want to warn against the monster to
	     * which the weapon does extra damage, as there is no "monster
	     * which the weapon warns against" field.
	     */
	    if(spec_ability(uwep,SPFX_WARN) && spec_dbon(uwep,mtmp->data,1))
		warnlevel = 100;
	    else if ((int) (mtmp->m_lev / 4) > warnlevel)
		warnlevel = (mtmp->m_lev / 4);
	}
	/* check whether hero notices monster and stops current activity */
	if (occupation && !rd && !Confusion &&
	    (!mtmp->mpeaceful || Hallucination) &&
	    canseemon(mtmp) && !cansee(x,y) &&
	    distu(mtmp->mx,mtmp->my) <= (BOLT_LIM+1)*(BOLT_LIM+1))
		stop_occupation();

	return(rd);
}

#endif /* OVL1 */
#ifdef OVL2

boolean
onscary(x, y, mtmp)
int x, y;
struct monst *mtmp;
{
	if (mtmp->isshk || mtmp->isgd || mtmp->iswiz || !mtmp->mcansee ||
			is_lminion(mtmp->data) || is_rider(mtmp->data) ||
			mtmp->data->mlet == S_HUMAN || mtmp->mpeaceful ||
			mtmp->data == &mons[PM_MINOTAUR])
		return(FALSE);
	return((boolean)(
#ifdef ELBERETH
		   sengr_at("Elbereth", x, y) ||
#endif
		    sobj_at(SCR_SCARE_MONSTER, x, y) != (struct obj *)0));
}

#endif /* OVL2 */
#ifdef OVL0

/*
 * Possibly awaken the given monster.  Return a 1 if the monster has been
 * jolted awake.
 */
static int
disturb(mtmp)
	register struct monst *mtmp;
{
	/*
	 * + Ettins are hard to surprise.
	 * + Nymphs, jabberwocks, and leprechauns do not easily wake up.
	 *
	 * Wake up if:
	 *	in direct LOS						AND
	 *	within 10 squares					AND
	 *	not stealthy or (mon is an ettin and 9/10)		AND
	 *	(mon is not a nymph, jabberwock, or leprechaun) or 1/50	AND
	 *	Aggravate or mon is (dog or human) or
	 *	    (1/7 and mon is not mimicing furniture or object)
	 */
	if(couldsee(mtmp->mx,mtmp->my) &&
		distu(mtmp->mx,mtmp->my) <= 100 &&
		(!Stealth || (mtmp->data == &mons[PM_ETTIN] && rn2(10))) &&
		(!(mtmp->data->mlet == S_NYMPH
			|| mtmp->data == &mons[PM_JABBERWOCK]
			|| mtmp->data->mlet == S_LEPRECHAUN) || !rn2(50)) &&
		(Aggravate_monster
			|| (mtmp->data->mlet == S_DOG ||
				mtmp->data->mlet == S_HUMAN)
			|| (!rn2(7) && mtmp->m_ap_type != M_AP_FURNITURE &&
				mtmp->m_ap_type != M_AP_OBJECT) )) {

		mtmp->msleep = 0;
		return(1);
	}
	return(0);
}

static void
distfleeck(mtmp,inrange,nearby,scared)
register struct monst *mtmp;
int *inrange, *nearby, *scared;
{
	int seescaryx, seescaryy;

	*inrange = (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <=
							(BOLT_LIM * BOLT_LIM));
	*nearby = *inrange && monnear(mtmp, mtmp->mux, mtmp->muy);

	/* Note: if your image is displaced, the monster sees the Elbereth
	 * at your displaced position, thus never attacking your displaced
	 * position, but possibly attacking you by accident.  If you are
	 * invisible, it sees the Elbereth at your real position, thus never
	 * running into you by accident but possibly attacking the spot
	 * where it guesses you are.
	 */
	if (Invis && !perceives(mtmp->data)) {
		seescaryx = mtmp->mux;
		seescaryy = mtmp->muy;
	} else {
		seescaryx = u.ux;
		seescaryy = u.uy;
	}
	*scared = (*nearby && (onscary(seescaryx, seescaryy, mtmp) ||
			       (!mtmp->mpeaceful &&
				    in_your_sanctuary(mtmp->mx, mtmp->my))));

	if(*scared && !mtmp->mflee) {
#ifdef POLYSELF
		if (!sticks(uasmon))
#endif
			unstuck(mtmp);	/* monster lets go when fleeing */
		mtmp->mflee = 1;
#ifdef STUPID
		if (rn2(7))
		    mtmp->mfleetim = rnd(10);
		else
		    mtmp->mfleetim = rnd(100);
#else
		mtmp->mfleetim = rnd(rn2(7) ? 10 : 100);
#endif
	}

}

/* returns 1 if monster died moving, 0 otherwise */
/* The whole dochugw/m_move/distfleeck/mfndpos section is serious spaghetti
 * code. --KAA
 */
int
dochug(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat = mtmp->data;
	register int tmp=0;
	int inrange, nearby, scared;

/*	Pre-movement adjustments	*/

	if(mtmp->cham && !rn2(6))	/* polymorph chameleons */
	    (void) newcham(mtmp, (struct permonst *)0);

	/* regenerate monsters */
	if (mtmp->mhp < mtmp->mhpmax && (!(moves%20) || regenerates(mdat)))
		mtmp->mhp++;
	if(mtmp->mspec_used) mtmp->mspec_used--;

	/* polymorph lycanthropes */
	were_change(mtmp);

	/* check for waitmask status change */
	if((mtmp->data->mflags3 & M3_WAITFORU) &&
	   (m_canseeu(mtmp) || mtmp->mhp < mtmp->mhpmax))
	    mtmp->data->mflags3 &= ~M3_WAITFORU;

#ifdef MULDGN
	/* update quest status flags */
	quest_stat_check(mtmp);
#endif

	if(!mtmp->mcanmove || (mtmp->data->mflags3 & M3_WAITMASK)) {
	    if (Hallucination) newsym(mtmp->mx,mtmp->my);
#ifdef MULDGN
	    if(mtmp->mcanmove && (mtmp->data->mflags3 & M3_CLOSE) &&
	       !mtmp->msleep && monnear(mtmp, u.ux, u.uy))
		quest_talk(mtmp);	/* give the leaders a chance to speak */
#endif
	    return(0);	/* other frozen monsters can't do anything */
	}

	/* there is a chance we will wake it */
	if(mtmp->msleep && !disturb(mtmp)) {
		if (Hallucination) newsym(mtmp->mx,mtmp->my);
		return(0);
	}

	/* not frozen or sleeping: wipe out texts written in the dust */
	wipe_engr_at(mtmp->mx, mtmp->my, 1);

	/* confused monsters get unconfused with small probability */
	if (mtmp->mconf && !rn2(50)) mtmp->mconf = 0;

	/* stunned monsters get un-stunned with larger probability */
	if (mtmp->mstun && !rn2(10)) mtmp->mstun = 0;

	/* some monsters teleport */
	if (mtmp->mflee && !rn2(40) && can_teleport(mdat) && !mtmp->iswiz) {
		rloc(mtmp);
		return(0);
	}
	if (mdat->msound == MS_SHRIEK && !um_dist(mtmp->mx, mtmp->my, 1))
	    m_respond(mtmp);
	if (mdat == &mons[PM_MEDUSA] && cansee(mtmp->mx, mtmp->my))
	    m_respond(mtmp);
	if (mdat->mmove < rnd(6)) return(0);

	/* fleeing monsters might regain courage */
	if (mtmp->mflee && !mtmp->mfleetim
	   && mtmp->mhp == mtmp->mhpmax && !rn2(25)) mtmp->mflee = 0;

	set_apparxy(mtmp);
	/* Must be done after you move and before the monster does.  The
	 * set_apparxy() call in m_move() doesn't suffice since the variables
	 * inrange, etc. all depend on stuff set by set_apparxy().
	 */

	/* Monsters that want to acquire things */
	/* may teleport, so do it before inrange is set */
	if(is_covetous(mtmp->data)) (void) tactics(mtmp);

	/* check distance and scariness of attacks */
	distfleeck(mtmp,&inrange,&nearby,&scared);

#ifdef MUSE
	if(find_defensive(mtmp)) {
		if (use_defensive(mtmp) != 0)
			return 1;
	} else if(find_misc(mtmp)) {
		if (use_misc(mtmp) != 0)
			return 1;
	}
#endif

	/* Demonic Blackmail! */
	if(nearby && mdat->msound == MS_BRIBE &&
	   mtmp->mpeaceful && !mtmp->mtame) {
		if (mtmp->mux != u.ux || mtmp->muy != u.uy) {
			pline("%s whispers at thin air.",
			    cansee(mtmp->mux, mtmp->muy) ? Monnam(mtmp) : "It");
#ifdef POLYSELF
			if (is_demon(uasmon)) rloc(mtmp);
			  /* "Good hunting, brother" */
			else {
#endif
			    mtmp->minvis = 0;
			    /* Why?  For the same reason in real demon talk */
			    pline("%s gets angry!", Amonnam(mtmp));
			    mtmp->mpeaceful = 0;
			    /* since no way is an image going to pay it off */
#ifdef POLYSELF
			}
#endif
		} else if(demon_talk(mtmp)) return(1);	/* you paid it off */
	}

	/* the watch will look around and see if you are up to no good :-) */
	if (mdat == &mons[PM_WATCHMAN] || mdat == &mons[PM_WATCH_CAPTAIN]) 
		watch_on_duty(mtmp);

	else if (mdat == &mons[PM_MIND_FLAYER] && !rn2(20)) {
		struct monst *m2, *nmon = (struct monst *)0;

		if (canseemon(mtmp))
			pline("%s concentrates.", Monnam(mtmp));
		if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM) {
			You("sense a faint wave of psychic energy.");
			goto toofar;
		}
		pline("A wave of psychic energy pours over you!");
		if (mtmp->mpeaceful &&
		    (!Conflict || resist(mtmp, RING_CLASS, 0, 0)))
			pline("It feels quite soothing.");
		else {
			register boolean m_sen = sensemon(mtmp);

			if (m_sen || (Telepat && rn2(2)) || !rn2(10)) {
				int dmg;
				pline("It locks on to your %s!",
					m_sen ? "telepathy" :
					Telepat ? "latent telepathy" : "mind");
				dmg = rnd(15);
				if (Half_spell_damage) dmg = (dmg+1) / 2;
				losehp(dmg, "psychic blast", KILLED_BY_AN);
			}
		}
		for(m2=fmon; m2; m2 = nmon) {
			nmon = m2->nmon;
			if (m2->mpeaceful != mtmp->mpeaceful) continue;
			if (mindless(m2->data)) continue;
			if (m2 == mtmp) continue;
			if ((telepathic(m2->data) &&
			    (rn2(2) || m2->mblinded)) || !rn2(10)) {
				if (cansee(m2->mx, m2->my))
				    pline("It locks on to %s.", mon_nam(m2));
				m2->mhp -= rnd(15);
				if (m2->mhp <= 0)
				    monkilled(m2, "", AD_DRIN);
			}
		}
	}
toofar:
#ifdef MUSE
	/* If monster is nearby you, and has to wield a weapon, do so.   This
	 * costs the monster a move, of course.
	 */
	if((!mtmp->mpeaceful || Conflict) && inrange &&
	   dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8
	   && attacktype(mdat, AT_WEAP)) {
	    if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)) {
		mtmp->weapon_check = NEED_HTH_WEAPON;
		if (mon_wield_item(mtmp) != 0) return(0);
	    }
	}
#endif
/*	Now the actual movement phase	*/

	if(!nearby || mtmp->mflee || scared ||
	   mtmp->mconf || mtmp->mstun || (mtmp->minvis && !rn2(3)) ||
	   (mdat->mlet == S_LEPRECHAUN && !u.ugold && (mtmp->mgold || rn2(2))) ||
	   (is_wanderer(mdat) && !rn2(4)) || (Conflict && !mtmp->iswiz) ||
	   (!mtmp->mcansee && !rn2(4)) || mtmp->mpeaceful) {

		tmp = m_move(mtmp, 0);
		distfleeck(mtmp,&inrange,&nearby,&scared);	/* recalc */

		switch (tmp) {

		    case 0:	/* no movement, but it can still attack you */
		    case 3:	/* absolutely no movement */
				/* for pets, case 0 and 3 are equivalent */
 			/* During hallucination, monster appearance should
 			 * still change - even if it doesn't move.
  			 */
 			if(Hallucination) newsym(mtmp->mx,mtmp->my);
 			break;
 		    case 1:	/* monster moved */
			/* Maybe it stepped on a trap and fell asleep... */
			if(mtmp->msleep || !mtmp->mcanmove) return(0);
 			if(!nearby && ranged_attk(mdat)) break;
 			else if(mdat->mmove <= 12) {
			    /* a monster that's digesting you can move at the
			     * same time -dlc
			     */
			    if(u.uswallow && mtmp == u.ustuck)
				return(mattacku(mtmp));
			    return(0);
			}
 			break;
 		    case 2:	/* monster died */
 			return(1);
 		}
	}

/*	Now, attack the player if possible - one attack set per monst	*/

	if (!mtmp->mpeaceful ||
	    (Conflict && !resist(mtmp, RING_CLASS, 0, 0))) {
	    if(inrange && !noattacks(mdat) && u.uhp > 0 && !scared && tmp != 3)
		if(mattacku(mtmp)) return(1); /* monster died (e.g. exploded) */

	    if(mtmp->wormno) wormhitu(mtmp);
	}
#ifdef MULDGN
	/* special speeches for quest monsters */
	if(!mtmp->msleep && mtmp->mcanmove && nearby)
		quest_talk(mtmp);
	else
#endif
	    /* extra emotional attack for vile monsters */
	    if(inrange && mtmp->data->msound == MS_CUSS && !mtmp->mpeaceful &&
	       couldsee(mtmp->mx, mtmp->my) && !mtmp->minvis && !rn2(5))
		cuss(mtmp);

	/* extra movement for fast monsters */
	if(mdat->mmove-12 > rnd(12)) tmp = m_move(mtmp, 1);
	return(tmp == 2);
}

static NEARDATA const char practical[] = { WEAPON_CLASS, ARMOR_CLASS, GEM_CLASS, FOOD_CLASS, 0 };
static NEARDATA const char magical[] = {
	AMULET_CLASS, POTION_CLASS, SCROLL_CLASS, WAND_CLASS, RING_CLASS,
	SPBOOK_CLASS, 0 };
static NEARDATA const char indigestion[] = { BALL_CLASS, ROCK_CLASS, 0 };

#ifdef POLYSELF
boolean
itsstuck(mtmp)
register struct monst *mtmp;
{
	if (sticks(uasmon) && mtmp==u.ustuck && !u.uswallow) {
		pline("%s cannot escape from you!", Monnam(mtmp));
		return(TRUE);
	}
	return(FALSE);
}
#endif

int
m_move(mtmp, after)
register struct monst *mtmp;
register int after;
{
	register int appr;
	xchar gx,gy,nix,niy,chcnt;
	int chi;	/* could be schar except for stupid Sun-2 compiler */
	boolean likegold=0, likegems=0, likeobjs=0, likemagic=0, conceals=0;
	boolean likerock=0, can_tunnel=0;
	boolean can_open=0, can_unlock=0, doorbuster=0;
#ifdef MUSE
	boolean uses_items=0;
#endif
	struct permonst *ptr;
	schar mmoved = 0;	/* not strictly nec.: chi >= 0 will do */
	long info[9];
	long flag;
	int  omx = mtmp->mx, omy = mtmp->my;
#ifdef MUSE
	struct obj *mw_tmp;
#endif

	if(mtmp->mtrapped) {
	    int i = mintrap(mtmp);
	    if(i >= 2) { newsym(mtmp->mx,mtmp->my); return(2); }/* it died */
	    if(i == 1) return(0);	/* still in trap, so didn't move */
	}
	ptr = mtmp->data; /* mintrap() can change mtmp->data -dlc */
	if(hides_under(ptr) && OBJ_AT(mtmp->mx, mtmp->my) && rn2(10))
	    return(0);		/* do not leave hiding place */
	if(mtmp->meating) {
	    mtmp->meating--;
	    return(3);			/* still eating */
	}

	set_apparxy(mtmp);
	/* where does mtmp think you are? */
	/* Not necessary if m_move called from this file, but necessary in
	 * other calls of m_move (ex. leprechauns dodging)
	 */
	can_tunnel = tunnels(ptr) &&
#ifdef REINCARNATION
	    !Is_rogue_level(&u.uz) &&
#endif
#ifdef MUSE
		(!needspick(ptr) ||
		 (m_carrying(mtmp, PICK_AXE) &&
		  (mtmp->weapon_check != NO_WEAPON_WANTED ||
		   !(mw_tmp = MON_WEP(mtmp)) || mw_tmp->otyp == PICK_AXE)));
#else
		(!needspick(ptr) || m_carrying(mtmp, PICK_AXE));
#endif
	can_open = !(nohands(ptr) || verysmall(ptr));
	can_unlock = ((can_open && m_carrying(mtmp, SKELETON_KEY)) || mtmp->iswiz);
	doorbuster = is_giant(ptr);
	if(mtmp->wormno) goto not_special;
	/* my dog gets special treatment */
	if(mtmp->mtame) {
	    mmoved = dog_move(mtmp, after);
	    goto postmov;
	}

	/* likewise for shopkeeper */
	if(mtmp->isshk) {
	    mmoved = shk_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;		/* follow player outside shop */
	}

	/* and for the guard */
	if(mtmp->isgd) {
	    mmoved = gd_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;
	}

	/* and the acquisitive monsters get special treatment */
	if(is_covetous(ptr)) {
	    xchar tx = (xchar)((mtmp->mstrategy >> 16) & 0xff),
		  ty = (xchar)((mtmp->mstrategy >> 8) & 0xff);
	    struct monst *intruder = m_at(tx, ty);
	    /*
	     * if there's a monster on the object or in possesion of it,
	     * attack it.
	     */
	    if((dist2(mtmp->mx, mtmp->my, tx, ty) < 2) &&
	       intruder && (intruder != mtmp)) {

		if(mattackm(mtmp, intruder) == 2) return(2);
		mmoved = 1;
	    } else mmoved = 0;
	    goto postmov;
	}

	/* and for the priest */
	if(mtmp->ispriest) {
	    mmoved = pri_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;
	}

#ifdef MAIL
	if(ptr == &mons[PM_MAIL_DAEMON]) {
	    if(flags.soundok && canseemon(mtmp))
		verbalize("I'm late!");
	    mongone(mtmp);
	    return(2);	    
	}
#endif

	/* teleport if that lies in our nature */
	if(ptr == &mons[PM_TENGU] && !rn2(5) && !mtmp->mcan) {
	    if(mtmp->mhp < 7 || mtmp->mpeaceful || rn2(2))
		rloc(mtmp);
	    else
		mnexto(mtmp);
	    mmoved = 1;
	    goto postmov;
	}
not_special:
	if(u.uswallow && !mtmp->mflee && u.ustuck != mtmp) return(1);
	appr = 1;
	omx = mtmp->mx;
	omy = mtmp->my;
	gx = mtmp->mux;
	gy = mtmp->muy;
	if(mtmp->mflee) appr = -1;
	if (mtmp->mconf || (u.uswallow && mtmp == u.ustuck))
		appr = 0;
	else {
		boolean should_see = (couldsee(omx, omy) && 	
  			  	      (levl[gx][gy].lit || 
				       !levl[omx][omy].lit) &&
				      (dist2(omx, omy, gx, gy) <= 36));

		if (!mtmp->mcansee || 
	    	    (should_see && Invis && !perceives(ptr)) || 
#ifdef POLYSELF
	            (u.usym == S_MIMIC_DEF) || u.uundetected ||
#endif
	            (mtmp->mpeaceful && !mtmp->isshk) ||  /* allow shks to follow */
	            ((ptr->mlet == S_STALKER || ptr->mlet == S_BAT ||
		      ptr->mlet == S_LIGHT) && !rn2(3)))
			appr = 0;  
	
	    	if(monsndx(ptr) == PM_LEPRECHAUN && (appr == 1) && 
	           (mtmp->mgold > u.ugold))
			appr = -1;

		if (!should_see && can_track(ptr)) {
			register coord *cp;

			cp = gettrack(omx,omy);
			if (cp) {
		    		gx = cp->x;
		    		gy = cp->y;
			}
		}
	}

#ifdef REINCARNATION
	if (!Is_rogue_level(&u.uz))
#endif
	{
		register int pctload = (curr_mon_load(mtmp) * 100) /
			max_mon_load(mtmp);

		/* look for gold or jewels nearby */
		likegold = (likes_gold(ptr) && pctload < 95);
		likegems = (likes_gems(ptr) && pctload < 85);
#ifdef MUSE
		uses_items = (!mindless(ptr) && !is_animal(ptr)
			&& pctload < 75);
#endif
		likeobjs = (likes_objs(ptr) && pctload < 75);
		likemagic = (likes_magic(ptr) && pctload < 85);
		likerock = (throws_rocks(ptr) && pctload < 50);
		conceals = hides_under(ptr);
	}

#define SQSRCHRADIUS	5

      { register int minr = SQSRCHRADIUS;	/* not too far away */
	register struct obj *otmp;
	register int xx, yy;
	int oomx, oomy, lmx, lmy;

	/* cut down the search radius if it thinks character is closer. */
	if(distmin(mtmp->mux, mtmp->muy, omx, omy) < SQSRCHRADIUS &&
	    !mtmp->mpeaceful) minr--;
	/* guards shouldn't get too distracted */
	if(!mtmp->mpeaceful && is_mercenary(ptr)) minr = 1;

	if((likegold || likegems || likeobjs || likemagic || likerock || conceals)
	      && (!*in_rooms(omx, omy, SHOPBASE) || (!rn2(25) && !mtmp->isshk))) {
	look_for_obj:
	    oomx = min(COLNO-1, omx+minr);
	    oomy = min(ROWNO-1, omy+minr);
	    lmx = max(1, omx-minr);
	    lmy = max(0, omy-minr);
	    for(otmp = fobj; otmp; otmp = otmp->nobj) {
		xx = otmp->ox;
		yy = otmp->oy;
		/* Nymphs take everything.  Most other creatures should not
		 * pick up corpses except as a special case like in
		 * searches_for_item().  We need to do this check in
		 * mpickstuff() as well.
		 */
		if(xx >= lmx && xx <= oomx && yy >= lmy && yy <= oomy) {
		    if(((likegold && otmp->otyp == GOLD_PIECE) ||
		       (likeobjs && index(practical, otmp->oclass) &&
			(otmp->otyp != CORPSE || ptr->mlet == S_NYMPH)) ||
		       (likemagic && index(magical, otmp->oclass)) ||
#ifdef MUSE
		       (uses_items && searches_for_item(mtmp, otmp)) ||
#endif
		       (likerock && otmp->otyp == BOULDER) ||
		       (likegems && otmp->oclass == GEM_CLASS &&
			objects[otmp->otyp].oc_material != MINERAL) ||
		       (conceals && !cansee(otmp->ox,otmp->oy)) ||
		       (ptr == &mons[PM_GELATINOUS_CUBE] &&
			!index(indigestion, otmp->oclass) &&
			!(otmp->otyp == CORPSE &&
			  otmp->corpsenm == PM_COCKATRICE))
		      ) && touch_artifact(otmp,mtmp)) {
			if(can_carry(mtmp,otmp) &&
			   (throws_rocks(ptr) ||
				!sobj_at(BOULDER,xx,yy)) &&
			   (ptr->mlet != S_UNICORN ||
			    objects[otmp->otyp].oc_material == GEMSTONE)) {
			    minr = distmin(omx,omy,xx,yy);
			    oomx = min(COLNO-1, omx+minr);
			    oomy = min(ROWNO-1, omy+minr);
			    lmx = max(1, omx-minr);
			    lmy = max(0, omy-minr);
			    gx = otmp->ox;
			    gy = otmp->oy;
			}
		    }
		}
	    }
	} else if(likegold) {
	    /* don't try to pick up anything else, but use the same loop */
#ifdef MUSE
	    uses_items =
#endif
	    likegems = likeobjs = likemagic = likerock = conceals = 0;
	    goto look_for_obj;
	}

	if(minr < SQSRCHRADIUS && appr == -1) {
	    if(distmin(omx,omy,mtmp->mux,mtmp->muy) <= 3) {
		gx = mtmp->mux;
		gy = mtmp->muy;
	    } else
		appr = 1;
	}
      }
	nix = omx;
	niy = omy;
	flag = ALLOW_TRAPS;
	if (mtmp->mpeaceful && (!Conflict || resist(mtmp, RING_CLASS, 0, 0)))
	    flag |= (ALLOW_SANCT | ALLOW_SSM);
	else flag |= ALLOW_U;
	if (ptr->mlet == S_UNICORN) flag |= NOTONL;
	if (passes_walls(ptr)) flag |= (ALLOW_WALL | ALLOW_ROCK);
	if (can_tunnel) flag |= ALLOW_DIG;
	if (is_human(ptr) || ptr == &mons[PM_MINOTAUR]) flag |= ALLOW_SSM;
	if (is_undead(ptr)) flag |= NOGARLIC;
	if (throws_rocks(ptr)) flag |= ALLOW_ROCK;
	if (can_open) flag |= OPENDOOR;
	if (can_unlock) flag |= UNLOCKDOOR;
	if (doorbuster) flag |= BUSTDOOR;
	{
	    register int i, j, nx, ny, nearer;
	    int jcnt, cnt;
	    int ndist, nidist;
	    register coord *mtrk;
	    coord poss[9];

	    cnt = mfndpos(mtmp, poss, info, flag);
	    chcnt = 0;
	    jcnt = min(MTSZ, cnt-1);
	    chi = -1;
	    nidist = dist2(nix,niy,gx,gy);
	    /* allow monsters be shortsighted on some levels for balance */
	    if(!mtmp->mpeaceful && level.flags.shortsighted &&
	       nidist > (couldsee(nix,niy) ? 144 : 36) && appr == 1) appr = 0;

	    for(i=0; i < cnt; i++) {
		nx = poss[i].x;
		ny = poss[i].y;

		if (appr != 0) {
		    mtrk = &mtmp->mtrack[0];
		    for(j=0; j < jcnt; mtrk++, j++)
			if(nx == mtrk->x && ny == mtrk->y)
			    if(rn2(4*(cnt-j)))
				goto nxti;
		}

		nearer = ((ndist = dist2(nx,ny,gx,gy)) < nidist);

		if((appr == 1 && nearer) || (appr == -1 && !nearer) ||
		   (!appr && !rn2(++chcnt)) || !mmoved) {
		    nix = nx;
		    niy = ny;
		    nidist = ndist;
		    chi = i;
		    mmoved = 1;
		}
	    nxti:	;
	    }
	}

	if(mmoved) {
	    register int j;
#ifdef POLYSELF
	    if (mmoved==1 && (u.ux != nix || u.uy != niy) && itsstuck(mtmp))
		return(3);
#endif
#ifdef MUSE
	    if (mmoved==1 && can_tunnel && needspick(ptr) &&
		(!(mw_tmp = MON_WEP(mtmp)) || mw_tmp->otyp != PICK_AXE)) {
		mtmp->weapon_check = NEED_PICK_AXE;
		(void)mon_wield_item(mtmp);
	    }
#endif
	    /* If ALLOW_U is set, either it's trying to attack you, or it
	     * thinks it is.  In either case, attack this spot in preference to
	     * all others.
	     */
	    if(info[chi] & ALLOW_U) {
		nix = mtmp->mux;
		niy = mtmp->muy;
	    }
	    if (nix == u.ux && niy == u.uy) {
		mtmp->mux = u.ux;
		mtmp->muy = u.uy;
		return(0);
	    }
	    /* The monster may attack another based on 1 of 2 conditions:
	     * 1 - It may be confused.
	     * 2 - It may mistake the monster for your (displaced) image.
	     * Pets get taken care of above and shouldn't reach this code.
	     * Conflict gets handled even farther away (movemon()).
	     */
	    if((info[chi] & ALLOW_M) ||
		   (nix == mtmp->mux && niy == mtmp->muy)) {
		struct monst *mtmp2;
		int mstatus;
		mtmp2 = m_at(nix,niy);

		mstatus = mattackm(mtmp, mtmp2);

		if (mstatus & MM_AGR_DIED)		/* aggressor died */
		    return 2;

		if ((mstatus & MM_HIT) && !(mstatus & MM_DEF_DIED)  &&
		    rn2(4) && mtmp2->mlstmv != monstermoves) {
		    mstatus = mattackm(mtmp2, mtmp);	/* return attack */
		    if (mstatus & MM_DEF_DIED)
			return 2;
		}
		return 3;
	    }

	    remove_monster(omx, omy);
	    place_monster(mtmp, nix, niy);
	    for(j = MTSZ-1; j > 0; j--)
		mtmp->mtrack[j] = mtmp->mtrack[j-1];
	    mtmp->mtrack[0].x = omx;
	    mtmp->mtrack[0].y = omy;
	    /* Place a segment at the old position. */
	    if (mtmp->wormno) worm_move(mtmp);
	} else {
	    if(ptr->mlet == S_UNICORN && rn2(2)) {
		rloc(mtmp);
		return(1);
	    }
	    if(mtmp->wormno) worm_nomove(mtmp);
	}
postmov:
	if(mmoved == 1) {
	    boolean canseeit = cansee(mtmp->mx, mtmp->my);
	    boolean abstain = (mtmp->mpeaceful && !mtmp->mtame);

	    newsym(omx,omy);		/* update the old position */
	    if (mintrap(mtmp) >= 2) {
		if(mtmp->mx) newsym(mtmp->mx,mtmp->my);
		return(2);	/* it died */
	    }
	    ptr = mtmp->data;

	    /* open a door, or crash through it, if you can */
	    if(IS_DOOR(levl[mtmp->mx][mtmp->my].typ)
		    && !passes_walls(ptr) /* doesn't need to open doors */
		    && !can_tunnel /* taken care of below */
		  ) {
		struct rm *here = &levl[mtmp->mx][mtmp->my];
		boolean btrapped = (here->doormask & D_TRAPPED);

		if(here->doormask & (D_LOCKED|D_CLOSED) && amorphous(ptr)) {
		    if (flags.verbose && canseeit)
			pline("%s %ss under the door.", Monnam(mtmp),
			      (ptr == &mons[PM_FOG_CLOUD] ||
			       ptr == &mons[PM_YELLOW_LIGHT])
			      ? "flow" : "ooze");
		} else if(here->doormask & D_LOCKED && can_unlock) {
		    if(btrapped) {
			here->doormask = D_NODOOR;
			newsym(mtmp->mx, mtmp->my);
			unblock_point(mtmp->mx,mtmp->my); /* vision */
			if(mb_trapped(mtmp)) return(2);
		    } else {
			if (flags.verbose) {
			    if (canseeit)
			       You("see a door unlock and open.");
			    else if (flags.soundok)
			       You("hear a door unlock and open.");
		        }
		        here->doormask = D_ISOPEN;
			/* newsym(mtmp->mx, mtmp->my); */
			unblock_point(mtmp->mx,mtmp->my); /* vision */
		    }
		} else if (here->doormask == D_CLOSED && can_open) {
		    if(btrapped) {
			here->doormask = D_NODOOR;
			newsym(mtmp->mx, mtmp->my);
			unblock_point(mtmp->mx,mtmp->my); /* vision */
			if(mb_trapped(mtmp)) return(2);
		    } else {
		        if (flags.verbose) {
			    if (canseeit)
			         You("see a door open.");
			    else if (flags.soundok)
			         You("hear a door open.");
		        }
		        here->doormask = D_ISOPEN;
			/* newsym(mtmp->mx, mtmp->my); */  /* done below */
			unblock_point(mtmp->mx,mtmp->my); /* vision */
		    }
		} else if (here->doormask & (D_LOCKED|D_CLOSED)) {
		    /* mfndpos guarantees this must be a doorbuster */
		    if(btrapped) {
			here->doormask = D_NODOOR;
			newsym(mtmp->mx, mtmp->my);
			unblock_point(mtmp->mx,mtmp->my); /* vision */
			if(mb_trapped(mtmp)) return(2);
		    } else {
		        if (flags.verbose) {
			    if (canseeit)
			        You("see a door crash open.");
			    else if (flags.soundok)
			        You("hear a door crash open.");
		        }
		        if (here->doormask & D_LOCKED && !rn2(2))
			        here->doormask = D_NODOOR;
		        else here->doormask = D_BROKEN;
			/* newsym(mtmp->mx, mtmp->my); */ /* done below */
			unblock_point(mtmp->mx,mtmp->my); /* vision */
		    }
		}
	    }

	    /* possibly dig */
	    if (can_tunnel && mdig_tunnel(mtmp))
		    return(2);	/* mon died (position already updated) */

	    /* set also in domove(), hack.c */
	    if (u.uswallow && mtmp == u.ustuck &&
					(mtmp->mx != omx || mtmp->my != omy)) {
		/* If the monster moved, then update */
		u.ux0 = u.ux;
		u.uy0 = u.uy;
		u.ux = mtmp->mx;
		u.uy = mtmp->my;
		swallowed(0);
	    } else
		newsym(mtmp->mx,mtmp->my);

	    if(OBJ_AT(mtmp->mx, mtmp->my) && mtmp->mcanmove) {
		/* Maybe a rock mole just ate some metal object */
		if(metallivorous(ptr)) meatgold(mtmp);

		if(g_at(mtmp->mx,mtmp->my) && likegold &&
				    (!abstain || !rn2(10))) mpickgold(mtmp);

		/* Maybe a cube ate just about anything */
		if(ptr == &mons[PM_GELATINOUS_CUBE]) meatobj(mtmp);

		if((!abstain || !rn2(10)) && 
		   (!*in_rooms(mtmp->mx, mtmp->my, SHOPBASE) || !rn2(25))) {
		    if(likeobjs) mpickstuff(mtmp, practical);
		    if(likemagic) mpickstuff(mtmp, magical);
		    if(likerock || likegems) mpickgems(mtmp);
#ifdef MUSE
		    if(uses_items) mpickstuff(mtmp, (char *)0);
#endif
		}

		if(mtmp->minvis) {
		    newsym(mtmp->mx, mtmp->my);
		    if (mtmp->wormno) see_wsegs(mtmp);
		}
	    }

	    if(hides_under(ptr)) {
		mtmp->mundetected = OBJ_AT(mtmp->mx, mtmp->my);
		newsym(mtmp->mx, mtmp->my);
	    }
	}
	return(mmoved);
}

#endif /* OVL0 */
#ifdef OVL2

boolean
closed_door(x, y)
register int x, y;
{
	return((boolean)(IS_DOOR(levl[x][y].typ) &&
			(levl[x][y].doormask & (D_LOCKED | D_CLOSED))));
}

boolean
accessible(x, y)
register int x, y;
{
	return((boolean)(ACCESSIBLE(levl[x][y].typ) && !closed_door(x, y)));
}

#endif /* OVL2 */
#ifdef OVL0

/* decide where the monster thinks you are standing */
void
set_apparxy(mtmp)
register struct monst *mtmp;
{
	boolean notseen, gotu;
	register int disp, mx = mtmp->mux, my = mtmp->muy;

	/*
	 * do cheapest and/or most likely tests first
	 */

	/* pet knows your smell; grabber still has hold of you */
	if (mtmp->mtame || mtmp == u.ustuck) goto found_you;

	/* monsters which know where you are don't suddenly forget,
	   if you haven't moved away */
	if (mx == u.ux && my == u.uy) goto found_you;

	notseen = Invis && !perceives(mtmp->data);
	/* add cases as required.  eg. Displacement ... */
	disp = (notseen || Underwater ? 1 : Displaced ? 2 : 0);
	if (!disp) goto found_you;

	/* without something like the following, invis. and displ.
	   are too powerful */
	gotu = notseen ? !rn2(3) : Displaced ? !rn2(4) : FALSE;

	/* If invis but not displaced, staying around gets you 'discovered' */
	gotu |= (!Displaced && u.dx == 0 && u.dy == 0);

	if (!gotu) {
	    register int try_cnt = 0;
	    do {
		if (++try_cnt > 200) goto found_you;		/* punt */
		mx = u.ux - disp + rn2(2*disp+1);
		my = u.uy - disp + rn2(2*disp+1);
	    } while (!isok(mx,my)
		  || (disp != 2 && mx == mtmp->mx && my == mtmp->my)
		  || ((mx != u.ux || my != u.uy) &&
		      !passes_walls(mtmp->data) &&
		      (!ACCESSIBLE(levl[mx][my].typ) ||
			(closed_door(mx, my) && !amorphous(mtmp->data)))));
	} else {
found_you:
	    mx = u.ux;
	    my = u.uy;
	}

	mtmp->mux = mx;
	mtmp->muy = my;
}

#endif /* OVL0 */

/*monmove.c*/
