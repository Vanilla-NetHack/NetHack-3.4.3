/*	SCCS Id: @(#)dogmove.c	3.0	88/04/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include "mfndpos.h"
#include "edog.h"

#ifdef OVL0

static const char NEARDATA nofetch[] = { BALL_SYM, CHAIN_SYM, ROCK_SYM, 0 };

#endif /* OVL0 */

STATIC_DCL void FDECL(dog_eat, (struct monst *, struct obj *, XCHAR_P, int, int));

#ifdef OVLB

STATIC_OVL void
dog_eat(mtmp, obj, otyp, x, y)
register struct monst *mtmp;
register struct obj * obj;
xchar otyp;
int x, y;
{
	register struct edog *edog = EDOG(mtmp);
	int nutrit;

	if(edog->hungrytime < moves)
	    edog->hungrytime = moves;
	/* Note: to get the correct percentage-eaten in case oeaten is set,
	 * use "obj->owt / obj->quan / base-weight".  It so happens that here
	 * we want to multiply by obj->quan, which thus cancels out.
	 * It is arbitrary that the pet takes the same length of time to eat
	 * as a human, but gets 5X as much nutrition.
	 */
	if(obj->otyp == CORPSE) {
	    mtmp->meating = 3 + (mons[obj->corpsenm].cwt >> 2);
	    nutrit = 5 * mons[obj->corpsenm].cnutrit;
	} else {
	    mtmp->meating = objects[obj->otyp].oc_delay;
	    nutrit = 5 * objects[obj->otyp].nutrition;
	}
	if(obj->oeaten) {
	    mtmp->meating = eaten_stat(mtmp->meating, obj);
	    nutrit = eaten_stat(nutrit, obj);
	}
	edog->hungrytime += nutrit;
	mtmp->mconf = 0;
	if (mtmp->mtame < 20) mtmp->mtame++;
	if(cansee(x,y))
	    pline("%s eats %s.", Monnam(mtmp), doname(obj));
	/* perhaps this was a reward */
	if(otyp != CADAVER)
#ifdef LINT
	    edog->apport = 0;
#else
	    edog->apport += (unsigned)(200L/
		((long)edog->dropdist+moves-edog->droptime));
#endif
	delobj(obj);
}

#endif /* OVLB */
#ifdef OVL0

/* return 0 (no move), 1 (move) or 2 (dead) */
int
dog_move(mtmp, after)
register struct monst *mtmp;
register int after;
{
register int nx,ny,omx,omy,appr,nearer,j;
int udist,chi,i,whappr;
/* XLINT register struct permonst *mdat = mtmp->data; */
register struct edog *edog = EDOG(mtmp);
struct obj *obj;
struct trap *trap;
xchar cnt,chcnt,nix,niy;
schar dogroom,uroom;
xchar gx,gy,gtyp,otyp;	/* current goal */
coord poss[9];
long info[9];
long allowflags;
#define GDIST(x,y) (dist2(x,y,gx,gy))
#define DDIST(x,y) (dist2(x,y,omx,omy))

#ifdef __GNULINT__
	chi = -1;	/* gcc warning from 'goto newdogpos' */
#endif
	omx = mtmp->mx;
	omy = mtmp->my;
	whappr = (moves - edog->whistletime < 5);
	if(moves > edog->hungrytime + 500) {
		if(!carnivorous(mtmp->data) && !herbivorous(mtmp->data)) {
			edog->hungrytime = moves + 500;
			/* but not too high; it might polymorph */
		} else if (!mtmp->mconf) {
			mtmp->mconf = 1;
			mtmp->mhpmax /= 3;
			if(mtmp->mhp > mtmp->mhpmax)
				mtmp->mhp = mtmp->mhpmax;
			if(cansee(omx,omy))
			    pline("%s is confused from hunger.", Monnam(mtmp));
			else You("feel worried about %s.", mon_nam(mtmp));
		} else if(moves > edog->hungrytime + 750 ||
							mtmp->mhp < 1) {
#ifdef WALKIES
			if(mtmp->mleashed)
				Your("leash goes slack.");
#endif
			if(cansee(omx,omy))
				pline("%s dies%s.", Monnam(mtmp),
				      (mtmp->mhp >= 1) ? "" : " from hunger");
			else
		You("have a sad feeling for a moment, then it passes.");
			mondied(mtmp);
			return(2);
		}
	}
	dogroom = inroom(omx,omy);
	uroom = inroom(u.ux,u.uy);
	udist = dist(omx,omy);

	/* maybe we tamed him while being swallowed --jgm */
	if(!udist) return(0);

	/* if we are carrying sth then we drop it (perhaps near @) */
	/* Note: if apport == 1 then our behaviour is independent of udist */
	if(mtmp->minvent){
		if(!rn2(udist) || !rn2((int) edog->apport))
		if(rn2(10) < edog->apport){
			if (cansee(omx,omy) && flags.verbose)
			    pline("%s drops %s.", Monnam(mtmp),
					distant_name(mtmp->minvent, doname));
			relobj(mtmp, (int) mtmp->minvis);
			if(edog->apport > 1) edog->apport--;
			edog->dropdist = udist;		/* hpscdi!jon */
			edog->droptime = moves;
		}
	} else {
		if((obj=level.objects[omx][omy]) && !index(nofetch,obj->olet)
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
									){
		    if((otyp = dogfood(mtmp, obj)) <= CADAVER){
			nix = omx;
			niy = omy;
			dog_eat(mtmp, obj, otyp, nix, niy);
			goto newdogpos;
		    }
		    if(can_carry(mtmp, obj))
		    if(!obj->cursed)
		    if(rn2(20) < edog->apport+3)
		    if(rn2(udist) || !rn2((int) edog->apport)){
			if (cansee(omx, omy) && flags.verbose)
			    pline("%s picks up %s.", Monnam(mtmp),
				distant_name(obj, doname));
			freeobj(obj);
			unpobj(obj);
			/* if(levl[omx][omy].scrsym == obj->olet)
				newsym(omx,omy); */
			mpickobj(mtmp,obj);
		    }
		}
	}

	gtyp = UNDEF;	/* no goal as yet */
	gx = gy = 0;	/* suppress 'used before set' message */
#ifdef WALKIES
	/* If he's on a leash, he's not going anywhere. */
	if(mtmp->mleashed) {

		gtyp = APPORT;
		gx = u.ux;
		gy = u.uy;
	} else
#endif
	/* first we look for food, then objects */
	    for(obj = fobj; obj; obj = obj->nobj) {
		otyp = dogfood(mtmp, obj);
		if(otyp > gtyp || otyp == UNDEF) continue;
		if(inroom(obj->ox,obj->oy) != dogroom) continue;
		if(otyp < MANFOOD &&
		 (dogroom >= 0 || DDIST(obj->ox,obj->oy) < 10)) {
			if(otyp < gtyp || (otyp == gtyp &&
				DDIST(obj->ox,obj->oy) < DDIST(gx,gy))){
				gx = obj->ox;
				gy = obj->oy;
				gtyp = otyp;
			}
		} else if(gtyp == UNDEF && dogroom >= 0 &&
		   uroom == dogroom &&
		   !mtmp->minvent && edog->apport > rn2(8) &&
		   can_carry(mtmp,obj)){
			gx = obj->ox;
			gy = obj->oy;
			gtyp = APPORT;
		}
	    }

	if(gtyp == UNDEF ||
	  (gtyp != DOGFOOD && gtyp != APPORT && moves < edog->hungrytime)){
		if(dogroom < 0 || dogroom == uroom){
			gx = u.ux;
			gy = u.uy;
		} else {
			int tmp = rooms[dogroom].fdoor;
			    cnt = rooms[dogroom].doorct;

			gx = gy = FAR;	/* random, far away */
			while(cnt--){
			    if(dist(gx,gy) >
				dist(doors[tmp].x, doors[tmp].y)){
					gx = doors[tmp].x;
					gy = doors[tmp].y;
				}
				tmp++;
			}
			/* here gx == FAR e.g. when dog is in a vault */
			if(gx == FAR || (gx == omx && gy == omy)){
				gx = u.ux;
				gy = u.uy;
			}
		}
		appr = (udist >= 9) ? 1 : (mtmp->mflee) ? -1 : 0;
		if(after && udist <= 4 && gx == u.ux && gy == u.uy)
			return(0);
		if(udist > 1){
			if(!IS_ROOM(levl[u.ux][u.uy].typ) || !rn2(4) ||
			   whappr ||
			   (mtmp->minvent && rn2((int) edog->apport)))
				appr = 1;
		}
		/* if you have dog food it'll follow you more closely */
		if(appr == 0){
			obj = invent;
			while(obj){
				if(dogfood(mtmp, obj) == DOGFOOD) {
					appr = 1;
					break;
				}
				obj = obj->nobj;
			}
		}
	} else	appr = 1;	/* gtyp != UNDEF */
	if(mtmp->mconf) appr = 0;

	if(gx == u.ux && gy == u.uy && (dogroom != uroom || dogroom < 0)) {
	register coord *cp;
		cp = gettrack(omx,omy);
		if(cp){
			gx = cp->x;
			gy = cp->y;
		}
	}

	nix = omx;
	niy = omy;
	
	allowflags = ALLOW_M | ALLOW_TRAPS | ALLOW_SSM | ALLOW_SANCT;
	if (passes_walls(mtmp->data)) allowflags |= (ALLOW_ROCK|ALLOW_WALL);
	if (throws_rocks(mtmp->data)) allowflags |= ALLOW_ROCK;
	if (!nohands(mtmp->data) && !verysmall(mtmp->data)) {
		allowflags |= OPENDOOR;
		if (m_carrying(mtmp, SKELETON_KEY)) allowflags |= BUSTDOOR;
	}
	if (is_giant(mtmp->data)) allowflags |= BUSTDOOR;
	if (tunnels(mtmp->data) && !needspick(mtmp->data))
		allowflags |= ALLOW_DIG;
	cnt = mfndpos(mtmp, poss, info, allowflags);
	if (allowflags & ALLOW_DIG) if(!mdig_tunnel(mtmp)) return(2);
	chcnt = 0;
	chi = -1;
	for(i=0; i<cnt; i++){
		nx = poss[i].x;
		ny = poss[i].y;
#ifdef WALKIES
		/* if leashed, we drag him along. */
		if(dist(nx, ny) > 4 && mtmp->mleashed) continue;
#endif
		if(info[i] & ALLOW_M) {
			if(MON_AT(nx, ny)) {
			    int stat;
			    register struct monst *mtmp2 = m_at(nx,ny);

			    if(mtmp2->m_lev >= mtmp->m_lev+2 ||
			       (mtmp2->data->mlet == S_COCKATRICE &&
				!resists_ston(mtmp->data)))
				continue;
			    if(after) return(0); /* hit only once each move */

			    if((stat = mattackm(mtmp, mtmp2)) == 1 && rn2(4) &&
			      mtmp2->mlstmv != moves &&
			      mattackm(mtmp2, mtmp) == 2) return(2);
			    if(stat == -1) return(2);
			    return(0);
			}
		}

		/* dog avoids traps */
		/* but perhaps we have to pass a trap in order to follow @ */
		if((info[i] & ALLOW_TRAPS) && (trap = t_at(nx,ny))){
#ifdef WALKIES
			if(!mtmp->mleashed) {
#endif
			    if(!trap->tseen && rn2(40)) continue;
			    if(rn2(10)) continue;
#ifdef WALKIES
			}
# ifdef SOUNDS
			else if(flags.soundok)
				whimper(mtmp);
# endif
#endif
		}

		/* dog eschews cursed objects */
		/* but likes dog food */
		for(obj = level.objects[nx][ny]; obj; obj = obj->nexthere) {
		    if(obj->cursed && !mtmp->mleashed) goto nxti;
		    if(obj->olet == FOOD_SYM &&
			(otyp = dogfood(mtmp, obj)) < MANFOOD &&
			(otyp < ACCFOOD || edog->hungrytime <= moves)){
			/* Note: our dog likes the food so much that he
			might eat it even when it conceals a cursed object */
			nix = nx;
			niy = ny;
			chi = i;
			dog_eat(mtmp, obj, otyp, nix, niy);
			goto newdogpos;
		    }
		}

		for(j=0; j<MTSZ && j<cnt-1; j++)
			if(nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
				if(rn2(4*(cnt-j))) goto nxti;

		nearer = (GDIST(nx,ny) - GDIST(nix,niy)) * appr;
		if((nearer == 0 && !rn2(++chcnt)) || nearer<0 ||
			(nearer > 0 && !whappr &&
				((omx == nix && omy == niy && !rn2(3))
				|| !rn2(12))
			)){
			nix = nx;
			niy = ny;
			if(nearer < 0) chcnt = 0;
			chi = i;
		}
	nxti:	;
	}
newdogpos:
	if(nix != omx || niy != omy) {
		if(info[chi] & ALLOW_U) {
#ifdef WALKIES
			if(mtmp->mleashed) { /* play it safe */
				pline("%s breaks loose of %s leash!", 
					Monnam(mtmp),
					is_female(mtmp) ? "her" :
					is_human(mtmp->data) ? "his" : "its");
				m_unleash(mtmp);
			}
#endif
			(void) mattacku(mtmp);
			return(0);
		}
		remove_monster(omx, omy);
		place_monster(mtmp, nix, niy);
		for(j=MTSZ-1; j>0; j--) mtmp->mtrack[j] = mtmp->mtrack[j-1];
		mtmp->mtrack[0].x = omx;
		mtmp->mtrack[0].y = omy;
	}
#ifdef WALKIES
	  /* an incredible kluge, but the only way to keep pooch near
	   * after he spends time eating or in a trap, etc...
	   */
	  else  if(mtmp->mleashed && dist(omx, omy) > 4) {
		coord cc;	

		nx = sgn(omx - u.ux);
		ny = sgn(omy - u.uy);
		if(goodpos((cc.x = u.ux+nx), (cc.y = u.uy+ny), mtmp->data))
			goto dognext;

	 	i  = xytod(nx, ny);
		for(j = (i + 7)%8; j < (i + 1)%8; j++) {
			dtoxy(&cc, j);
			if(goodpos(cc.x, cc.y, mtmp->data)) goto dognext;
		}
		for(j = (i + 6)%8; j < (i + 2)%8; j++) {
			dtoxy(&cc, j);
			if(goodpos(cc.x, cc.y, mtmp->data)) goto dognext;
		}
		cc.x = mtmp->mx;
		cc.y = mtmp->my;
dognext:
		remove_monster(mtmp->mx, mtmp->my);
		place_monster(mtmp, cc.x, cc.y);
		pmon(mtmp);
		set_apparxy(mtmp);
	}
#endif
	return(1);
}

#endif /* OVL0 */
