/*	SCCS Id: @(#)monmove.c	3.0	89/11/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef LINT
# ifndef __STDC__
#define TRAP_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
# endif
#endif

#include "hack.h"
#include "mfndpos.h"
#ifdef NAMED_ITEMS
#  include "artifact.h"
#endif

static void FDECL(distfleeck,(struct monst *,int *,int *,int *));
#ifdef POLYSELF
static boolean FDECL(itsstuck,(struct monst *));
#endif

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
		return(TRUE);
	}
	return(FALSE);
}

boolean  
mdig_tunnel(mtmp)  /* FALSE: monster died */
register struct monst *mtmp;
{
	register struct rm *here;
	register int pile = rnd(12);
	boolean canseeit = cansee(mtmp->mx, mtmp->my);
	here = &levl[mtmp->mx][mtmp->my];

	if (here->typ == SDOOR)
		here->typ = DOOR;
	if(IS_ROCK(here->typ)) {
	    /* Just ate something. */
	    if(IS_WALL(here->typ)) {
		if (!(here->diggable & W_NONDIGGABLE)) {
			if(flags.soundok && flags.verbose && !rn2(5))
		       		You("hear the sound of crashing rock.");
			if(!is_maze_lev) {
		  		here->typ = DOOR;
		  		here->doormask = D_NODOOR;
			}
			else
		  		here->typ = ROOM;
		}
	    } else	
	    	here->typ = CORR;
	    mnewsym(mtmp->mx, mtmp->my);
	} else 		/* Eats away door if present & closed or locked */
		if(closed_door(mtmp->mx, mtmp->my)) {
			if(here->doormask & D_TRAPPED) {
		    		here->doormask = D_NODOOR;
		    		if(mb_trapped(mtmp)) return(FALSE);
			} else {
		    		if(!rn2(3) && flags.verbose)
				    /* not too often.. */
		        		You("feel an unexpected draft of air.");
		    		here->doormask = D_BROKEN;
			}
		    	mnewsym(mtmp->mx, mtmp->my);
	    	} else 
		    /* it doesn't leave rocks if it didn't dig */
			return TRUE; 

	/* Left behind a pile? */
	if(pile < 5) {
	    if(pile == 1)
		(void) mksobj_at(BOULDER, mtmp->mx, mtmp->my);
	    else
		(void) mksobj_at(ROCK, mtmp->mx, mtmp->my);
	}
	here->seen = TRUE; /* required for newsym and mnewsym to work */
	if(canseeit && mtmp->minvis && !See_invisible)
	    newsym(mtmp->mx,mtmp->my);
	else
	    mnewsym(mtmp->mx,mtmp->my);
	here->seen = FALSE;
	return(TRUE);
}

#endif /* OVLB */
#ifdef OVL1

int
dochugw(mtmp)
	register struct monst *mtmp;
{
	register int x = mtmp->mx;
	register int y = mtmp->my;
	register int rd = dochug(mtmp);
	register int dd;

	if(!rd && !mtmp->mpeaceful &&
			(dd = dist(mtmp->mx,mtmp->my)) < dist(x,y) &&
			dd < 100 && !canseemon(mtmp)) {
#ifdef NAMED_ITEMS
	    /* Note: this assumes we only want to warn against the monster which
	     * the weapon does extra damage to, as there is no "monster which
	     * the weapon warns against" field.
	     */
		if(spec_ability(uwep,SPFX_WARN) && spec_dbon(uwep,mtmp->data,1))
			warnlevel = 100;
		else
#endif
		if (Warning && mtmp->m_lev > warnlevel)
			warnlevel = mtmp->m_lev;
	}
	return(rd);
}

#endif /* OVL1 */
#ifdef OVL2

boolean
onscary(x, y, mtmp)
int x, y;
struct monst *mtmp;
{
	/* Note: minotaurs must be immune to scare monster to avoid abuse
	 * from creating them and taking their wands, then polymorphing 60
	 * or so wands to get wishing...
	 */
	if (mtmp->isshk || mtmp->isgd || mtmp->iswiz || !mtmp->mcansee ||
			mtmp->data->mlet == S_HUMAN || mtmp->mpeaceful ||
			mtmp->data == &mons[PM_MINOTAUR])
		return(FALSE);
	return(
#ifdef ELBERETH
		   sengr_at("Elbereth", x, y) ||
#endif
		    sobj_at(SCR_SCARE_MONSTER, x, y) != (struct obj *)0);
}

#endif /* OVL2 */
#ifdef OVL1

static void
distfleeck(mtmp,inrange,nearby,scared)
register struct monst *mtmp;
int *inrange, *nearby, *scared;
{
	int seescaryx, seescaryy;

	*inrange = (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <=
							(BOLT_LIM * BOLT_LIM));
	*nearby = monnear(mtmp, mtmp->mux, mtmp->muy);

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
	*scared = (*nearby && onscary(seescaryx, seescaryy, mtmp));

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
		mtmp->mfleetim = (rn2(7) ? rnd(10) : rnd(100));
#endif
	}

}

/* returns 1 if monster died moving, 0 otherwise */
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
	if((!(moves%20) || regenerates(mdat)) && mtmp->mhp < mtmp->mhpmax)
		mtmp->mhp++;
	if(mtmp->mspec_used) mtmp->mspec_used--;

	/* polymorph lycanthropes */
	were_change(mtmp);

	if(!mtmp->mcanmove) {
		if (Hallucination) pmon(mtmp);
		return(0);	/* frozen monsters don't do anything */
	}

	if(mtmp->msleep)	/* there is a chance we will wake it */
		if(!disturb(mtmp)) return(0);

	/* not frozen or sleeping: wipe out texts written in the dust */
	wipe_engr_at(mtmp->mx, mtmp->my, 1);

	/* confused monsters get unconfused with small probability */
	if(mtmp->mconf && !rn2(50)) mtmp->mconf = 0;

	/* stunned monsters get un-stunned with larger probability */
	if(mtmp->mstun && !rn2(10)) mtmp->mstun = 0;

	/* some monsters teleport */
	if(mtmp->mflee && !rn2(40) && can_teleport(mdat) && !mtmp->iswiz) {
		rloc(mtmp);
		return(0);
	}
	if(mdat->mmove < rnd(6)) return(0);

	/* fleeing monsters might regain courage */
	if(mtmp->mflee && !mtmp->mfleetim
	   && mtmp->mhp == mtmp->mhpmax && !rn2(25)) mtmp->mflee = 0;

	set_apparxy(mtmp);
	/* Must be done after you move and before the monster does.  The
	 * set_apparxy() call in m_move() doesn't suffice since the variables
	 * inrange, etc... all depend on stuff set by set_apparxy().
	 */

	/* The Wizard's prime directive */
	/* may teleport, so do it before inrange is set */
	if(mtmp->iswiz)
		(void) wiz_get_amulet(mtmp);

	/* check distance and scariness of attacks */
	distfleeck(mtmp,&inrange,&nearby,&scared);

#ifdef INFERNO		/* Demonic Blackmail! */
	if(nearby && mdat->msound == MS_BRIBE &&
	   mtmp->mpeaceful && !mtmp->mtame) {
		if (mtmp->mux != u.ux || mtmp->muy != u.uy) {
			pline("%s whispers something to thin air.",
			    cansee(mtmp->mux, mtmp->muy) ? Monnam(mtmp) : "It");
# ifdef POLYSELF
			if (is_demon(uasmon)) rloc(mtmp);
			  /* "Good hunting, brother" */
			else {
# endif
			    mtmp->minvis = 0;
			    /* Why?  For the same reason in real demon talk */
			    pline("%s gets angry.", Xmonnam(mtmp));
			    mtmp->mpeaceful = 0;
			    /* since no way is an image going to pay it off */
# ifdef POLYSELF
			}
# endif
		} else if(demon_talk(mtmp)) return(1);	/* you paid it off */
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
 			if(Hallucination) pmon(mtmp);
 			break;
 		    case 1:	/* monster moved */
			/* Maybe it stepped on a trap and fell asleep... */
			if(mtmp->msleep || !mtmp->mcanmove) return(0);
 			if(!nearby && ranged_attk(mdat)) break;
 			else if(mdat->mmove <= 12) return(0);
 			break;
 		    case 2:	/* monster died */
 			return(1);
 		}
	}

/*	Now, attack the player if possible - one attack set per monst	*/

	if(inrange && !noattacks(mdat) &&
	   !mtmp->mpeaceful && !mtmp->mtame && u.uhp > 0 && !scared && tmp != 3)
	    if(mattacku(mtmp)) return(1); /* monster died (e.g. exploded) */

#ifdef WORM
	if(mtmp->wormno && !mtmp->mtame) wormhit(mtmp);
#endif

	/* extra emotional attack for vile monsters */
	if(inrange && mtmp->data->msound == MS_CUSS &&
	   !mtmp->minvis && !mtmp->mpeaceful && !rn2(5))
	    cuss(mtmp);

	/* extra movement for fast monsters */
	if(mdat->mmove-12 > rnd(12)) tmp = m_move(mtmp, 1);
	return(tmp == 2);
}

static const char practical[] = { WEAPON_SYM, GEM_SYM, FOOD_SYM, 0 };
static const char magical[] = {
	AMULET_SYM, POTION_SYM, SCROLL_SYM, WAND_SYM, RING_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	0 };
static const char indigestion[] = { BALL_SYM, ROCK_SYM, 0 };

#ifdef POLYSELF
static boolean
itsstuck(mtmp)
register struct monst *mtmp;
{
	if (sticks(uasmon) && mtmp==u.ustuck && !u.uswallow) {
		kludge("%s cannot escape from you!", Monnam(mtmp));
		return(TRUE);
	}
	return(FALSE);
}
#endif

#endif /* OVL1 */
#ifdef OVL0

int
m_move(mtmp, after)
register struct monst *mtmp;
register int after;
{
	register struct monst *mtmp2;
	register int nx,ny,omx,omy,appr,nearer,cnt,i,j;
	xchar gx,gy,nix,niy,chcnt;
	schar chi;
	boolean likegold=0, likegems=0, likeobjs=0, likemagic=0, conceals=0;
	boolean likerock=0, can_tunnel=0;
	boolean can_open=0, can_unlock=0, doorbuster=0;
	struct permonst *ptr = mtmp->data;
	schar mmoved = 0;	/* not strictly nec.: chi >= 0 will do */
	coord poss[9];
	long info[9];
	long flag;

	if(mtmp->mtrapped) {
	    i = mintrap(mtmp);
	    if(i == 2) return(2);	/* it died */
	    if(i == 1) return(0);	/* still in trap, so didn't move */
	}
	if(mtmp->mhide &&
	   (OBJ_AT(mtmp->mx, mtmp->my) || levl[mtmp->mx][mtmp->my].gmask) &&
	   rn2(10))
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
		     dlevel != rogue_level &&
#endif
		     (!needspick(ptr) || m_carrying(mtmp, PICK_AXE));
	can_open = !(nohands(ptr) || verysmall(ptr));
	can_unlock = ((can_open && m_carrying(mtmp, SKELETON_KEY)) || mtmp->iswiz);
	doorbuster = is_giant(ptr);
#ifdef WORM
	if(mtmp->wormno) goto not_special;
#endif
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

	/* and the wiz already got special treatment */
	if(mtmp->iswiz) {
	    mmoved = 0;
	    goto postmov;
	}
#if defined(ALTARS) && defined(THEOLOGY)
	/* and for the priest */
	if(mtmp->ispriest) {
	    mmoved = pri_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;
	}
#endif
#ifdef MAIL
	if(ptr == &mons[PM_MAIL_DAEMON]) {
	    if(flags.soundok && canseemon(mtmp))
		verbalize("I'm late!");
	    mongone(mtmp);
	    return(2);	    
	}
#endif
	/* teleport if that lies in our nature */
	if(ptr == &mons[PM_TENGU] && !rn2(5)) {
	    if(mtmp->mhp < 7 || mtmp->mpeaceful || rn2(2))
		rloc(mtmp);
	    else
		mnexto(mtmp);
	    mmoved = 1;
	    goto postmov;
	}
#ifdef WORM
not_special:
#endif
	if(!mtmp->mflee && u.uswallow && u.ustuck != mtmp) return(1);
	appr = 1;
	if(mtmp->mflee) appr = -1;
	if(mtmp->mconf || (Invis && !perceives(ptr)) ||  !mtmp->mcansee ||
#ifdef POLYSELF
	   (u.usym == S_MIMIC_DEF) || u.uundetected ||
#endif
	   (mtmp->mpeaceful && !mtmp->isshk) ||	/* allow shks to follow */
	   ((ptr->mlet == S_STALKER || ptr->mlet == S_BAT ||
	     ptr->mlet == S_YLIGHT) && !rn2(3)))
	    appr = 0;
	omx = mtmp->mx;
	omy = mtmp->my;
	gx = mtmp->mux;
	gy = mtmp->muy;
	if(ptr == &mons[PM_LEPRECHAUN] && appr == 1 && mtmp->mgold > u.ugold)
	    appr = -1;

	if(can_track(ptr)) {
	    register coord *cp;
	    schar mroom;

	    mroom = inroom(omx,omy);
	    if(mroom < 0 || mroom != inroom(u.ux,u.uy)){
		cp = gettrack(omx,omy);
		if(cp){
		    gx = cp->x;
		    gy = cp->y;
		}
	    }
	}

#ifdef REINCARNATION
	if (dlevel != rogue_level)
#endif
	{
		register int pctload = (curr_mon_load(mtmp) * 100) /
			max_mon_load(mtmp);

		/* look for gold or jewels nearby */
		likegold = (likes_gold(ptr) && pctload < 95);
		likegems = (likes_gems(ptr) && pctload < 85);
		likeobjs = (likes_objs(ptr) && pctload < 75);
		likemagic = (likes_magic(ptr) && pctload < 85);
		likerock = (throws_rocks(ptr) && pctload < 50);
		conceals = hides_under(ptr);
	}

#define	SRCHRADIUS	25

      { xchar mind = SRCHRADIUS;		/* not too far away */
	register int dd;

	/* cut down the search radius if it thinks character is closer. */
	if(dist2(mtmp->mux, mtmp->muy, omx, omy) < SRCHRADIUS &&
	    !mtmp->mtame && !mtmp->mpeaceful)	 mind /= 2;

	if(likegold){
	    register struct gold *gold;

	    for(gold = fgold; gold; gold = gold->ngold)
		if((dd = dist2(omx,omy,gold->gx,gold->gy)) < mind){
		    mind = dd;
		    gx = gold->gx;
		    gy = gold->gy;
		}
	}
	if((likegems || likeobjs || likemagic || likerock || conceals)
	      && (!in_shop(omx, omy) || (!rn2(25) && !mtmp->isshk))) {
	    register struct obj *otmp;

	    for(otmp = fobj; otmp; otmp = otmp->nobj)
		if((likeobjs && index(practical, otmp->olet)) ||
		   (likemagic && index(magical, otmp->olet)) ||
		   (likerock && otmp->otyp == BOULDER) ||
		   (likegems && otmp->olet == GEM_SYM &&
			otmp->otyp < LAST_GEM + 6) ||
		   (conceals && !cansee(otmp->ox,otmp->oy)) ||
		   (ptr == &mons[PM_GELATINOUS_CUBE] &&
					!index(indigestion, otmp->olet))
		  ) {
			if(can_carry(mtmp,otmp))
			if(ptr->mlet != S_UNICORN ||
					objects[otmp->otyp].g_val != 0)
			    if((dd = dist2(omx,omy,otmp->ox,otmp->oy)) < mind){
				mind = dd;
				gx = otmp->ox;
				gy = otmp->oy;
			    }
		}
	}
	if(mind < SRCHRADIUS && appr == -1) {
	    if(dist2(omx,omy,mtmp->mux,mtmp->muy) < 10) {
		gx = mtmp->mux;
		gy = mtmp->muy;
	    } else
		appr = 1;
	}
      }
	nix = omx;
	niy = omy;
	flag = ALLOW_TRAPS;
	if (mtmp->mpeaceful) flag |= (ALLOW_SANCT | ALLOW_SSM);
	else flag |= ALLOW_U;
	if (ptr->mlet == S_UNICORN) flag |= NOTONL;
	if (passes_walls(ptr)) flag |= (ALLOW_WALL | ALLOW_ROCK);
	if (can_tunnel) flag |= ALLOW_DIG;
	if (is_human(ptr)) flag |= ALLOW_SSM;
	if (is_undead(ptr)) flag |= NOGARLIC;
	if (throws_rocks(ptr)) flag |= ALLOW_ROCK;
	if (can_open) flag |= OPENDOOR;
	if (can_unlock) flag |= UNLOCKDOOR;
	if (doorbuster) flag |= BUSTDOOR;
	cnt = mfndpos(mtmp, poss, info, flag);
	chcnt = 0;
	chi = -1;

	for(i=0; i < cnt; i++) {
	    nx = poss[i].x;
	    ny = poss[i].y;

	    if (appr != 0) for(j=0; j < MTSZ && j < cnt-1; j++)
		if(nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
		    if(rn2(4*(cnt-j))) goto nxti;

	    nearer = (dist2(nx,ny,gx,gy) < dist2(nix,niy,gx,gy));

	    if((appr == 1 && nearer) || (appr == -1 && !nearer) ||
		   	(!appr && !rn2(++chcnt)) || !mmoved) {
		nix = nx;
		niy = ny;
		chi = i;
		mmoved = 1;
	    }
	nxti:	;
	}

	if(mmoved) {
#ifdef POLYSELF
	    if (mmoved==1 && (u.ux != nix || u.uy != niy) && itsstuck(mtmp))
		return(3);
#endif
	    if((info[chi] & ALLOW_U) || (nix == u.ux && niy == u.uy)) {
		mtmp->mux = u.ux;
		mtmp->muy = u.uy;
		return(0);
	    }
	    /* The monster may attack another based on 1 of 2 conditions:
	     * 1 - He may be under the "conflict" influence.
	     * 2 - He may mistake the monster for your (displaced) image.
	     * Pets get taken care of above and shouldn't reach this code.
	     */
	    if((info[chi] & ALLOW_M) ||
		   (nix == mtmp->mux && niy == mtmp->muy)) {
		int stat;
		mtmp2 = 
		    (MON_AT(nix, niy) ? m_at(nix,niy) : (struct monst *)0);
		if((stat = mattackm(mtmp, mtmp2)) == 1 && rn2(4) &&
			mtmp2->mlstmv != moves && mattackm(mtmp2, mtmp) == 2)
		    return(2);
		if(stat == -1) return(2);
		return(3);
	    }
#ifdef WORM
	    /* The square now has a worm segment and must keep its MON_AT() state */
	    if (!mtmp->wormno)
#endif
		    remove_monster(omx, omy);
	    place_monster(mtmp, nix, niy);
	    for(j = MTSZ-1; j > 0; j--)
		mtmp->mtrack[j] = mtmp->mtrack[j-1];
	    mtmp->mtrack[0].x = omx;
	    mtmp->mtrack[0].y = omy;
#ifdef WORM
	    if(mtmp->wormno) worm_move(mtmp);
#endif
	} else {
	    if(ptr->mlet == S_UNICORN && rn2(2)) {
		rloc(mtmp);
		return(1);
	    }
#ifdef WORM
	    if(mtmp->wormno) worm_nomove(mtmp);
#endif
	}
postmov:
	if(mmoved == 1) {
	    boolean canseeit = cansee(mtmp->mx, mtmp->my);
	    boolean abstain = (mtmp->mpeaceful && !mtmp->mtame);

	    if(mintrap(mtmp) == 2) return(2);	/* he died */

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
			      ptr == &mons[PM_FOG_CLOUD] ? "flow" : "ooze");
		} else if(here->doormask & D_LOCKED && can_unlock) {
		    if(btrapped) {
			here->doormask = D_NODOOR;
			mnewsym(mtmp->mx, mtmp->my);
			if (canseeit) prl(mtmp->mx,mtmp->my);
			if(mb_trapped(mtmp)) return(2);
		    } else {
			if (flags.verbose) {
			    if (canseeit)
			      You("see a door being unlocked and opened.");
			    else if (flags.soundok)
			       You("hear a door being unlocked and opened.");
		        }
		        here->doormask = D_ISOPEN;
			mnewsym(mtmp->mx, mtmp->my);
			if (canseeit) prl(mtmp->mx,mtmp->my);
		    }
		} else if (here->doormask == D_CLOSED && can_open) {
		    if(btrapped) {
			here->doormask = D_NODOOR;
			mnewsym(mtmp->mx, mtmp->my);
			if (canseeit) prl(mtmp->mx,mtmp->my);
			if(mb_trapped(mtmp)) return(2);
		    } else {
		        if (flags.verbose) {
			    if (canseeit)
			         You("see a door being opened.");
			    else if (flags.soundok)
			         You("hear the sound of a door opening.");
		        }
		        here->doormask = D_ISOPEN;
			mnewsym(mtmp->mx, mtmp->my);
			if (canseeit) prl(mtmp->mx,mtmp->my);
		    }
		} else if (here->doormask & (D_LOCKED|D_CLOSED)) {
		       /* mfndpos guarantees this must be a doorbuster */
		    if(btrapped) {
			here->doormask = D_NODOOR;
			mnewsym(mtmp->mx, mtmp->my);
			if (canseeit) prl(mtmp->mx,mtmp->my);
			if(mb_trapped(mtmp)) return(2);
		    } else {
		        if (flags.verbose) {
			    if (canseeit)
			        You("see a door crash open.");
			    else if (flags.soundok)
			        You("hear the sound of a door crashing open.");
		        }
		        if (here->doormask & D_LOCKED && !rn2(2))
			        here->doormask = D_NODOOR;
		        else here->doormask = D_BROKEN;
			mnewsym(mtmp->mx, mtmp->my);
			if (canseeit) prl(mtmp->mx,mtmp->my);
		    }
		}
	      }
	    /* Maybe a rock mole just ate something? */
	    if(can_tunnel) if(!mdig_tunnel(mtmp)) return(2); /* died? */

	    if(levl[mtmp->mx][mtmp->my].gmask == TRUE) {
		/* Maybe a rock mole just ate some gold */
		if(metallivorous(ptr)) meatgold(mtmp);
		if(likegold && (!abstain || !rn2(10))) mpickgold(mtmp);
	    }
	    if(OBJ_AT(mtmp->mx, mtmp->my)) {
		/* Maybe a rock mole just ate some metal object */
		if(metallivorous(ptr)) meatgold(mtmp);
		/* Maybe a cube ate just about anything */
		if(ptr == &mons[PM_GELATINOUS_CUBE]) meatobj(mtmp);

		if ((!abstain || !rn2(10)) 
			&& (!in_shop(mtmp->mx, mtmp->my) || !rn2(25))) {
		    if(likeobjs) mpickstuff(mtmp, practical);
		    if(likemagic) mpickstuff(mtmp, magical);
		    if(likerock || likegems) mpickgems(mtmp);
		}
	    }
	    if(mtmp->mhide) mtmp->mundetected = (OBJ_AT(mtmp->mx, mtmp->my)
					|| levl[mtmp->mx][mtmp->my].gmask);

	    /* set also in domove(), hack.c */
	    if(u.uswallow && mtmp == u.ustuck) {
		u.ux = mtmp->mx;
		u.uy = mtmp->my;
	        if(mtmp->mx != mtmp->mdx || mtmp->my != mtmp->mdy) {
		    swallowed(0);
		    newsym(mtmp->mdx,mtmp->mdy);
		    mtmp->mdx = mtmp->mx;
		    mtmp->mdy = mtmp->my;
		}
	    }
	    pmon(mtmp);
	}
	return(mmoved);
}

#endif /* OVL0 */
#ifdef OVL2

boolean
closed_door(x, y)
register int x, y;
{
	return(IS_DOOR(levl[x][y].typ) &&
			(levl[x][y].doormask & (D_LOCKED | D_CLOSED)));
}

boolean
accessible(x, y)
register int x, y;
{
	return(ACCESSIBLE(levl[x][y].typ) && !closed_door(x, y));
}

#endif /* OVL2 */
#ifdef OVL1

void
set_apparxy(mtmp)		/* where does mtmp think you are standing? */
	register struct monst *mtmp;
{
#define notseen (Invis && !perceives(mtmp->data))
/*	add cases as required.  eg. Displacement ... */
	register int disp = (notseen ? 1 : Displaced ? 2 : 0);

/* 	without something like the following, invis. and displ. are too */
/*	powerful. */
	register boolean gotu =
		(notseen ? !rn2(3) : Displaced ? !rn2(4) : FALSE);

/*	Monsters which know where you are don't suddenly forget, if you
	didn't move away. */
	if (mtmp->mux==u.ux && mtmp->muy==u.uy) gotu = 1;

/* 	your dog follows your smell */
	if(!disp || mtmp->mtame || gotu ||
/*	If invisible but not displaced, staying around gets you 'discovered' */
	    (!Displaced && u.dx == 0 && u.dy == 0)) {
		mtmp->mux = u.ux;
		mtmp->muy = u.uy;
	}
	else do {
		mtmp->mux = u.ux - disp + rn2(2*disp+1);
		mtmp->muy = u.uy - disp + rn2(2*disp+1);
	} while((mtmp->mux != u.ux || mtmp->muy != u.uy) &&
	        ( (!passes_walls(mtmp->data) &&
		      (!ACCESSIBLE(levl[mtmp->mux][mtmp->muy].typ) ||
		       (closed_door(mtmp->mux, mtmp->muy) &&
			!amorphous(mtmp->data)
		       )
		      )
		  ) ||
		  (disp==1 && mtmp->mux == mtmp->mx && mtmp->muy == mtmp->my)
	        )
	       );
}

#endif /* OVL1 */
#ifdef OVLB

#ifdef STUPID_CPP	/* otherwise these functions are macros in rm.h */
/*
 * Functions for encapsulation of level.monsters references.
 */
boolean
MON_AT(x, y)
int x, y;
{
    return(level.monsters[x][y] != (struct monst *)0);
}

void place_monster(mtmp, x, y)
register struct monst *mtmp;
int x, y;
{
    level.monsters[x][y] = mtmp;
    mtmp->mx = x;
    mtmp->my = y;
}

void place_worm_seg(mtmp, x, y)
register struct monst *mtmp;
int x, y;
{
    level.monsters[x][y] = mtmp;
}

void remove_monster(x, y)
int x, y;
{
    level.monsters[x][y] = (struct monst *)0;
}

struct monst *m_at(x, y)
int x, y;
{
    return(level.monsters[x][y]);
}
#endif	/* STUPID_CPP */

#endif /* OVLB */
