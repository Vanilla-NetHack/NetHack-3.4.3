/*	SCCS Id: @(#)fight.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* fight.c - version 1.0.3 */

#include	<stdio.h>
#include	"hack.h"

extern struct permonst li_dog, dog, la_dog;
extern char *exclam(), *hcolor(), *xname();
extern struct obj *mkobj_at();
#ifdef KAA
extern boolean stoned;
extern boolean unweapon;
extern char *nomovemsg, *defmonnam();
extern struct monst *mkmon_at();
#endif

static boolean far_noise;
static long noisetime;

/* hitmm returns 0 (miss), 1 (hit), or 2 (kill) */
hitmm(magr,mdef) register struct monst *magr,*mdef; {
register struct permonst *pa = magr->data, *pd = mdef->data;
int hit;
schar tmp;
boolean vis;
	if(index("Eauy", pa->mlet)) return(0);
	if(magr->mfroz) return(0);		/* riv05!a3 */
	tmp = pd->ac + pa->mlevel;
	if(mdef->mconf || mdef->mfroz || mdef->msleep){
		tmp += 4;
		if(mdef->msleep) mdef->msleep = 0;
	}
	hit = (tmp > rnd(20));
	if(hit) mdef->msleep = 0;
	vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my));
	if(vis){
		char buf[BUFSZ];
		if(mdef->mimic) seemimic(mdef);
		if(magr->mimic) seemimic(magr);
		(void) sprintf(buf,"%s %s", Monnam(magr),
			hit ? "hits" : "misses");
		pline("%s %s.", buf, monnam(mdef));
	} else {
		boolean farq = (dist(magr->mx, magr->my) > 15);
		if(farq != far_noise || moves-noisetime > 10) {
			far_noise = farq;
			noisetime = moves;
			pline("You hear some noises%s.",
				farq ? " in the distance" : "");
		}
	}
	if(hit){
		if(magr->data->mlet == 'c' && !magr->cham) {
			magr->mhpmax += 3;
			if(vis) pline("%s is turned to stone!", Monnam(mdef));
			else if(mdef->mtame)
     pline("You have a peculiarly sad feeling for a moment, then it passes.");
			monstone(mdef);
			hit = 2;
		} else
		if((mdef->mhp -= d(pa->damn,pa->damd)) < 1) {
			magr->mhpmax += 1 + rn2(pd->mlevel+1);
			if(magr->mtame && magr->mhpmax > 8*pa->mlevel){
				if(pa == &li_dog) magr->data = pa = &dog;
				else if(pa == &dog) magr->data = pa = &la_dog;
			}
			if(vis) pline("%s is killed!", Monnam(mdef));
			else if(mdef->mtame)
		pline("You have a sad feeling for a moment, then it passes.");
			mondied(mdef);
			hit = 2;
		}
		/* fixes a bug where max monster hp could overflow. */
		if(magr->mhpmax <= 0) magr->mhpmax = 127;
	}
#ifdef KAA
	if(hit == 1 && magr->data->mlet == 'Q') {
		rloc(mdef);
		if(vis && !cansee(mdef->mx,mdef->my))
			pline("%s suddenly disappears!",Monnam(mdef));
	}
#endif
	return(hit);
}

/* drop (perhaps) a cadaver and remove monster */
mondied(mdef) register struct monst *mdef; {
register struct permonst *pd = mdef->data;
#ifdef KOPS
	if(letter(pd->mlet) && rn2(3) && pd->mlet != 'K'){
#else
	if(letter(pd->mlet) && rn2(3)){
#endif
		if (pd->mlet == '1') panic("mondied: making obj '1'");
		(void) mkobj_at(pd->mlet,mdef->mx,mdef->my);
		if(cansee(mdef->mx,mdef->my)){
			unpmon(mdef);
			atl(mdef->mx,mdef->my,fobj->olet);
		}
		stackobj(fobj);
	}
	mondead(mdef);
}

/* drop a rock and remove monster */
monstone(mdef) register struct monst *mdef; {
	extern char mlarge[];
	if(index(mlarge, mdef->data->mlet))
		mksobj_at(ENORMOUS_ROCK, mdef->mx, mdef->my);
	else
		mksobj_at(ROCK, mdef->mx, mdef->my);
	if(cansee(mdef->mx, mdef->my)){
		unpmon(mdef);
		atl(mdef->mx,mdef->my,fobj->olet);
	}
	mondead(mdef);
}
		

fightm(mtmp) register struct monst *mtmp; {
register struct monst *mon;
	for(mon = fmon; mon; mon = mon->nmon) if(mon != mtmp) {
		if(DIST(mon->mx,mon->my,mtmp->mx,mtmp->my) < 3)
		    if(rn2(4))
			return(hitmm(mtmp,mon));
	}
	return(-1);
}

/* u is hit by sth, but not a monster */
thitu(tlev,dam,name)
register tlev,dam;
register char *name;
{
char buf[BUFSZ];
	setan(name,buf);
	if(u.uac + tlev <= rnd(20)) {
		if(Blind) pline("It misses.");
		else pline("You are almost hit by %s!", buf);
		return(0);
	} else {
		if(Blind) pline("You are hit!");
		else pline("You are hit by %s!", buf);
		losehp(dam,name);
		return(1);
	}
}

#ifdef KAA
char mlarge[] = "bCDdegIlmnoPSsTUwY',&9";
#else
char mlarge[] = "bCDdegIlmnoPSsTUwY',&";
#endif

boolean
hmon(mon,obj,thrown)	/* return TRUE if mon still alive */
register struct monst *mon;
register struct obj *obj;
register thrown;
{
	register tmp;
	boolean hittxt = FALSE;

#ifdef KAA
	stoned = FALSE;		/* this refers to the thing hit, not you */
#endif
	if(!obj){
#ifdef KAA
/* Note that c, y, and F can never wield weapons anyway */
	  if (u.usym == 'c' && mon->data->mlet != 'c') {
	       pline("You turn %s to stone!", monnam(mon));
	       stoned = TRUE;
	       xkilled(mon,0);
	       return(FALSE);
	  } else if (u.usym == 'y' && mon->data->mlet != 'y') {
	       pline("%s is blinded by your flash of light!",Monnam(mon));
	       if (!mon->mblinded) {
		    mon->mblinded += rn2(25);
		    mon->mcansee = 0;
	       }
	       rehumanize();
	       return(TRUE);
	  } else if (u.usym == 'F') {
	       pline("You explode!");
	       if (!index("gFY",mon->data->mlet)) {
		    pline("%s gets blasted!", Monnam(mon));
		    mon->mhp -= d(6,6);
		    rehumanize();
		    if (mon->mhp <= 0) {
			 killed(mon);
			 return(FALSE);
		    } else return(TRUE);
	       } else {
		    pline("The blast doesn't seem to affect %s.", monnam(mon));
		    rehumanize();
		    return(TRUE);
	       }
	  } else if (index("P,'", u.usym) && u.uhunger < 1500
		  && !u.uswallow && mon->data->mlet != 'c') {
	       static char msgbuf[BUFSZ];
	       pline("You swallow %s whole!", monnam(mon));
	       u.uhunger += 20*mon->mhpmax;
	       newuhs(FALSE);
	       xkilled(mon,2);
	       if (tmp = mon->mhpmax/5) {
		    nomul(-tmp);
		    (void)sprintf(msgbuf, "You finished digesting %s.",
			 monnam(mon));
		    nomovemsg = msgbuf;
	       }
	       return(FALSE);
	  } else if (u.usym != '@') {
	       if (u.usym == '&' && !rn2(5)) {
		    struct monst *dtmp;
		    pline("Some hell-p has arrived!");
		    dtmp = mkmon_at('&',u.ux,u.uy);
		    (void)tamedog(dtmp,(struct obj *)0);
	       }
	       tmp = d(mons[u.umonnum].damn, mons[u.umonnum].damd);
	  } else
#endif
		tmp = rnd(2);	/* attack with bare hands */
#ifdef KAA
		if (mon->data->mlet == 'c' && !uarmg && u.usym != 'c'){
#else
		if(mon->data->mlet == 'c' && !uarmg){
#endif
			pline("You hit the cockatrice with your bare hands.");
			pline("You turn to stone ...");
			done_in_by(mon);
		}
	} else if(obj->olet == WEAPON_SYM || obj->otyp == PICK_AXE) {
	    if(obj == uwep && (obj->otyp > SPEAR || obj->otyp < BOOMERANG))
		tmp = rnd(2);
	    else {
		if(index(mlarge, mon->data->mlet)) {
		    tmp = rnd(objects[obj->otyp].wldam);
		    switch (obj->otyp) {
			case SLING_BULLET:
			case CROSSBOW_BOLT:
			case MORNING_STAR:
			case PARTISAN:
			case BROAD_SWORD:	tmp += 1; break;

			case FLAIL:
			case RANSEUR:
			case VOULGE:		tmp += rnd(4); break;

			case HALBERD:
			case SPETUM:		tmp += rnd(6); break;

			case BARDICHE:
			case TRIDENT:		tmp += d(2,4); break;

			case TWO_HANDED_SWORD: 
			case KATANA: 		tmp += d(2,6); break;
		    }
		} else {
		    tmp = rnd(objects[obj->otyp].wsdam);
		    switch (obj->otyp) {
			case SLING_BULLET:
			case CROSSBOW_BOLT:
			case MACE:
			case FLAIL:
			case SPETUM:
			case TRIDENT:		tmp += 1; break;

			case BARDICHE:
			case BILL_GUISARME:
			case GUISARME:
			case LUCERN_HAMMER:
			case MORNING_STAR:
			case RANSEUR:
			case BROAD_SWORD:
			case VOULGE:		tmp += rnd(4); break;
		    }
		}
		tmp += obj->spe;
#ifdef KAA
		if(obj->olet == WEAPON_SYM && obj->dknown && index("VWZ &",
				mon->data->mlet)) tmp += rn2(4);
#endif
		if(!thrown && obj == uwep && obj->otyp == BOOMERANG
		 && !rn2(3)){
		  pline("As you hit %s, the boomerang breaks into splinters.",
				monnam(mon));
			freeinv(obj);
			setworn((struct obj *) 0, obj->owornmask);
			obfree(obj, (struct obj *) 0);
			tmp++;
		}
	    }
	    if(mon->data->mlet == 'O' && obj->otyp == TWO_HANDED_SWORD &&
		!strcmp(ONAME(obj), "Orcrist"))
		tmp += rnd(10);
	} else	switch(obj->otyp) {
		case HEAVY_IRON_BALL:
			tmp = rnd(25); break;
		case ENORMOUS_ROCK:
			tmp = rnd(20); break;
		case EXPENSIVE_CAMERA:
	pline("You succeed in destroying your camera. Congratulations!");
			freeinv(obj);
			if(obj->owornmask)
				setworn((struct obj *) 0, obj->owornmask);
			obfree(obj, (struct obj *) 0);
			return(TRUE);
		case DEAD_COCKATRICE:
			pline("You hit %s with the cockatrice corpse.",
				monnam(mon));
			if(mon->data->mlet == 'c') {
				tmp = 1;
				hittxt = TRUE;
#ifdef KAA
				stoned = TRUE;
				xkilled(mon,0);
#endif
				break;
			}
			pline("%s is turned to stone!", Monnam(mon));
			killed(mon);
			return(FALSE);
		case CLOVE_OF_GARLIC:		/* no effect against demons */
			if(index(UNDEAD, mon->data->mlet))
				mon->mflee = 1;
			tmp = 1;
			break;
		default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/10;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
		}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG) */

	tmp += u.udaminc + dbon();
	if(u.uswallow) {
		if((tmp -= u.uswldtim) <= 0) {
			pline("Your arms are no longer able to hit.");
			return(TRUE);
		}
	}
	if(tmp < 1) tmp = 1;
	mon->mhp -= tmp;
	if(mon->mhp < 1) {
		killed(mon);
		return(FALSE);
	}
	if(mon->mtame && (!mon->mflee || mon->mfleetim)) {
		mon->mflee = 1;			/* Rick Richardson */
		mon->mfleetim += 10*rnd(tmp);
	}

	if(!hittxt) {
		if(thrown)
			/* this assumes that we cannot throw plural things */
			hit( xname(obj)  /* or: objects[obj->otyp].oc_name */,
				mon, exclam(tmp) );
		else if(Blind)
			pline("You hit it.");
		else
			pline("You hit %s%s", monnam(mon), exclam(tmp));
	}

	if(u.umconf && !thrown) {
		if(!Blind) {
			pline("Your hands stop glowing %s.",
			Hallucination ? hcolor() : "blue");
		}
		if (!resist(mon, '+', 0, NOTELL)) mon->mconf = 1;
		if(!mon->mfroz && !mon->msleep && !Blind && mon->mconf)
			pline("%s appears confused.",Monnam(mon));
		u.umconf = 0;
	}
	if(!thrown && rn2(2) && index("VW",u.usym) &&
	   !index("VW",mon->data->mlet)){
		int tmp=d(2,6);
		pline("%s suddenly seems weaker!",Monnam(mon));
		mon->mhpmax -= tmp;
		if ((mon->mhp -= tmp) <= 0) {
			pline("%s dies!",Monnam(mon));
			xkilled(mon,0);
			return(FALSE);
		}
	}
	return(TRUE);	/* mon still alive */
}

/* try to attack; return FALSE if monster evaded */
/* u.dx and u.dy must be set */
attack(mtmp)
register struct monst *mtmp;
{
	schar tmp;
	boolean malive = TRUE;
	register struct permonst *mdat;
	mdat = mtmp->data;

#ifdef KAA
	if(unweapon) {
		unweapon=FALSE;
		if(uwep)
			pline("You begin bashing monsters with your %s.",
				aobjnam(uwep,(char *)0));
	}
#endif
	u_wipe_engr(3);   /* andrew@orca: prevent unlimited pick-axe attacks */

	if(mdat->mlet == 'L' && !mtmp->mfroz && !mtmp->msleep &&
	   !mtmp->mconf && mtmp->mcansee && !rn2(7) &&
	   (m_move(mtmp, 0) == 2 /* he died */ || /* he moved: */
		mtmp->mx != u.ux+u.dx || mtmp->my != u.uy+u.dy))
		return(FALSE);
#ifdef SAFE_ATTACK
	/* This section of code provides protection against accidentally
	 * hitting peaceful (like '@') and tame (like 'd') monsters.
	 * There is protection only if you're not blind, confused or
	 * invisible.
	 */
	/*  changes by wwp 5/16/85 */
	if (!Blind && !Confusion && !Hallucination
	    && mdat->mlet == 'd' && mtmp->mtame) {
		mtmp->mflee = 1;			
		mtmp->mfleetim = rnd(6);
		pline("You stop to avoid hitting your dog");
		return(TRUE);
	}
	if (flags.confirm && (mtmp->mpeaceful || mtmp->mtame) && ! Confusion
	    && !Hallucination && !Invisible)

		if (Blind ? Telepat : (!mtmp->minvis || See_invisible)) {
			pline("Really attack?");
			(void) fflush(stdout);
			if (readchar() != 'y') {
				flags.move = 0;
				return(TRUE);
			}
		}
#endif /* SAFE_ATTACK /**/

	if(mtmp->mimic){
		if(!u.ustuck && !mtmp->mflee) u.ustuck = mtmp;
#ifdef DGK
		if (levl[u.ux+u.dx][u.uy+u.dy].scrsym == symbol.door)
		    if (okdoor(u.ux+u.dx, u.uy+u.dy))
			pline("The door actually was %s.", defmonnam(mtmp));
		    else  pline("That spellbook was %s.", defmonnam(mtmp));
		else if (levl[u.ux+u.dx][u.uy+u.dy].scrsym == '$')
			pline("The chest was %s!", defmonnam(mtmp));
		else
			pline("Wait! That's %s!",defmonnam(mtmp));
#else
		switch(levl[u.ux+u.dx][u.uy+u.dy].scrsym){
		case '+':
			if (okdoor(u.ux+u.dx, u.uy+u.dy))
				pline("The door actually was %s.", defmonnam(mtmp));
			else	pline("That spellbook was %s.", defmonnam(mtmp));
			break;
		case '$':
			pline("The chest was %s!", defmonnam(mtmp));
			break;
		default:
			pline("Wait! That's %s!",defmonnam(mtmp));
		}
#endif
		wakeup(mtmp);	/* clears mtmp->mimic */
		return(TRUE);
	}

	wakeup(mtmp);

	if(mtmp->mhide && mtmp->mundetected){
		register struct obj *obj;

		mtmp->mundetected = 0;
		if((obj = o_at(mtmp->mx,mtmp->my)) && !Blind)
			pline("Wait! There's %s hiding under %s!",
				defmonnam(mtmp), doname(obj));
		return(TRUE);
	}
#ifdef KAA
	tmp = u.uluck + (u.mtimedone ? mons[u.umonnum].mlevel : u.ulevel) +
			mdat->ac + abon();
	if (u.usym=='y' || u.usym=='F') tmp=100;
	if (index("uEa",u.usym)) return(TRUE);
#endif
	if(uwep) {
#ifdef KAA	/* Blessed weapons used against undead or demons */
		if(uwep->olet == WEAPON_SYM && uwep->dknown && index("VWZ &",
			mtmp->data->mlet)) tmp += 2;
#endif
		if(uwep->olet == WEAPON_SYM || uwep->otyp == PICK_AXE)
			tmp += uwep->spe;
		if(uwep->otyp == TWO_HANDED_SWORD) tmp -= 1;
		else if(uwep->otyp == KATANA) tmp += 1;
		else if(uwep->otyp == DAGGER ||
			uwep->otyp == SHURIKEN) tmp += 2;
		else if(uwep->otyp == CRYSKNIFE) tmp += 3;
		else if(uwep->otyp == SPEAR &&
			index("XDne", mdat->mlet)) tmp += 2;
	}
	if(mtmp->msleep) {
		mtmp->msleep = 0;
		tmp += 2;
	}
	if(mtmp->mfroz) {
		tmp += 4;
		if(!rn2(10)) mtmp->mfroz = 0;
	}
	if(mtmp->mflee) tmp += 2;
	if(u.utrap) tmp -= 3;

	/* with a lot of luggage, your agility diminishes */
	tmp -= (inv_weight() + 40)/20;

	if(tmp <= rnd(20) && !u.uswallow){
		if(Blind) pline("You miss it.");
		else pline("You miss %s.",monnam(mtmp));
	} else {
		/* we hit the monster; be careful: it might die! */

		if((malive = hmon(mtmp,uwep,0)) == TRUE) {
		/* monster still alive */
			if(!rn2(25) && mtmp->mhp < mtmp->mhpmax/2) {
				mtmp->mflee = 1;
				if(!rn2(3)) mtmp->mfleetim = rnd(100);
				if(u.ustuck == mtmp && !u.uswallow)
					u.ustuck = 0;
			}
#ifndef NOWORM
			if(mtmp->wormno)
				cutworm(mtmp, u.ux+u.dx, u.uy+u.dy,
					uwep ? uwep->otyp : 0);
#endif
		}
		if(mdat->mlet == 'a') {
			if(rn2(2)) {
				if (Blind) pline("You are splashed!");
				else	   pline("You are splashed by %s's acid!",monnam(mtmp));
				if (u.usym != 'a') {
					losehp_m(rnd(6), mtmp);
					if(!rn2(30)) corrode_armor();
				}
			}
			if(!rn2(6)) corrode_weapon();
		}
	}
#ifdef KAA
	if (malive) if (u.usym=='N' && mtmp->minvent) {
		struct obj *otmp, *addinv();
		otmp = mtmp->minvent;
		mtmp->minvent = otmp->nobj;
		otmp = addinv(otmp);
		pline("You steal:");
		prinv(otmp);
	} else if (u.usym=='L' && mtmp->mgold) {
		u.ugold += mtmp->mgold;
		mtmp->mgold = 0;
		pline("Your purse feels heavier.");
	} else if (u.usym=='Q') rloc(mtmp);
#endif
	if(malive && mdat->mlet == 'E' && canseemon(mtmp)
	   && !mtmp->mcan && rn2(3)) {
	    if(mtmp->mcansee) {
	      pline("You are frozen by %s's gaze!",monnam(mtmp));
	      nomul((u.ulevel > 6 || rn2(4)) ? rn1(20,-21) : -200);
	    } else {
	      pline("%s cannot defend itself.", Amonnam(mtmp,"blinded"));
	      if(!rn2(500)) if((int)u.uluck > LUCKMIN) u.uluck--;
	    }
	}
	return(TRUE);
}
