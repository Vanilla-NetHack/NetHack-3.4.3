/*	SCCS Id: @(#)dothrow.c	3.0	89/11/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 't' (throw) */

#include "hack.h"

static void FDECL(gem_accept, (struct monst *, struct obj *));
static int FDECL(throw_gold, (struct obj *));
static const char toss_objs[] = { '0', GOLD_SYM, '#', WEAPON_SYM, 0 };
#ifdef WORM
extern boolean notonhead;
#endif

int
dothrow() {
	register struct obj *obj;

	obj = getobj(toss_objs, "throw");
	/* it is also possible to throw food */
	/* (or jewels, or iron balls... ) */

	if(!obj || !getdir(1)) {       /* ask "in what direction?" */
		if(obj && obj->olet == GOLD_SYM) u.ugold += OGOLD(obj);
		return(0);
	}

	if(obj->olet == GOLD_SYM) return(throw_gold(obj));

	if(!canletgo(obj,"throw"))
		return(0);
	if(obj->otyp == BOULDER
#ifdef POLYSELF
					&& !throws_rocks(uasmon)
#endif
								) {
		pline("It's too heavy.");
		return(1);
	}
	if(!u.dx && !u.dy && !u.dz) {
		You("cannot throw an object at yourself.");
		return(0);
	}
	u_wipe_engr(2);

	if(obj == uwep) {
	    if(welded(obj)) {
		weldmsg(obj, FALSE);
		return(1);
	    }
	    if(obj->quan > 1)
		setuwep(splitobj(obj, 1));
	    else {
		setuwep((struct obj *)0);
		if (uwep) return(1); /* unwielded, died, rewielded */
	    }
	}
	else if(obj->quan > 1)
		(void) splitobj(obj, 1);
	freeinv(obj);
	return(throwit(obj));
}

static void
hitfloor(obj)
register struct obj *obj;
{
#ifdef ALTARS
	if (IS_ALTAR(levl[u.ux][u.uy].typ)) doaltarobj(obj);
	else
#endif
		pline("%s hits the floor.", Doname2(obj));
	if (breaks(obj, TRUE)) return;
	else if(obj->olet == POTION_SYM) {
		pline("The flask breaks, and you smell a peculiar odor...");
		potionbreathe(obj);
		obfree(obj, (struct obj *)0);
	} else
		dropy(obj);
}

int
throwit(obj)
register struct obj *obj;
{
	register struct monst *mon;
	register int range;

	if(u.uswallow) {
		mon = u.ustuck;
		bhitpos.x = mon->mx;
		bhitpos.y = mon->my;
	} else if(u.dz) {
	  if(u.dz < 0) {
	    pline("%s hits the ceiling, then falls back on top of your %s.",
		Doname2(obj),		/* note: obj->quan == 1 */
		body_part(HEAD));
	    if(obj->olet == POTION_SYM)
		potionhit(&youmonst, obj);
	    else {
		if(uarmh) pline("Fortunately, you are wearing a helmet!");
		losehp(uarmh ? 1 : rnd((int)(obj->owt)), "falling object");
		if (!breaks(obj, TRUE)) dropy(obj);
	    }
	  } else hitfloor(obj);
	  return(1);

	} else if(obj->otyp == BOOMERANG) {
		mon = boomhit(u.dx, u.dy);
		if(mon == &youmonst) {		/* the thing was caught */
			(void) addinv(obj);
			return(1);
		}
	} else {
		if(shkcatch(obj))
		    return(1);

		range = (int)((ACURR(A_STR) > 18 ? 20 : ACURR(A_STR))/2 - obj->owt/4);
		if (obj == uball) {
			if (u.ustuck) range = 1;
			else if (range >= 5) range = 5;
		}
		if (range < 1) range = 1;

		if ((obj->olet == WEAPON_SYM || obj->olet == GEM_SYM) &&
		    uwep &&
		    objects[obj->otyp].w_propellor ==
			-objects[uwep->otyp].w_propellor)
				range++;
#ifdef POLYSELF
		if (obj->otyp == BOULDER) range = 20;
#endif

		mon = bhit(u.dx, u.dy, range, obj->olet,
			(int (*)()) 0, (int (*)()) 0, obj);
	}
	if(mon) {
		/* awake monster if sleeping */
		wakeup(mon);
#ifdef WORM
		if(bhitpos.x != mon->mx || bhitpos.y != mon->my)
			notonhead = TRUE;
#endif
		if(thitmonst(mon, obj)) return(1);
	}
	if(!u.uswallow)  {
		char let = obj->olet;

		/* the code following might become part of dropy() */
		if (breaks(obj, TRUE)) {
			tmp_at(-1, let);
#ifdef TEXTCOLOR
			tmp_at(-3, (int)objects[obj->otyp].oc_color);
#else
			tmp_at(-3, (int)AT_OBJ);
#endif
			tmp_at(bhitpos.x, bhitpos.y);
			tmp_at(-1, -1);
			return(1);
		}
		if(flooreffects(obj,bhitpos.x,bhitpos.y)) return(1);
#ifdef WORM
		if(obj->otyp == CRYSKNIFE)
			obj->otyp = WORM_TOOTH;
#endif
		obj->nobj = fobj;
		fobj = obj;
		place_object(obj, bhitpos.x, bhitpos.y);
		if(obj != uball && costly_spot(bhitpos.x, bhitpos.y) &&
		   !(mon && mon->isshk && bhitpos.x == mon->mx &&
		     bhitpos.y == mon->my && !(obj->unpaid)))
			sellobj(obj);
		stackobj(obj);
		if(obj == uball &&
			(bhitpos.x != u.ux || bhitpos.y != u.uy)){
			if(u.utrap){
				if(u.utraptype == TT_PIT)
					pline("The ball pulls you out of the pit!");
				else if(u.utraptype == TT_WEB)  {
					pline("The ball pulls you out of the web!");
					pline("The web is destroyed!");
					deltrap(t_at(u.ux,u.uy));
				} else {
				register long side =
					rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
				pline("The ball pulls you out of the bear trap.");
				Your("%s %s is severely damaged.",
					(side == LEFT_SIDE) ? "left" : "right",
					body_part(LEG));
				set_wounded_legs(side, 500+rn2(1000));
				losehp(2, "thrown ball");
				}
				u.utrap = 0;
			}
			unsee();
			u.ux = bhitpos.x - u.dx;
			u.uy = bhitpos.y - u.dy;
			movobj(uchain,u.ux,u.uy);
			setsee();
			spoteffects();
		}
		if(cansee(bhitpos.x, bhitpos.y)) prl(bhitpos.x,bhitpos.y);
	}  else
		mpickobj(u.ustuck,obj);
	return(1);
}

int
thitmonst(mon, obj)
register struct monst *mon;
register struct obj   *obj;
{
	register int	tmp; /* Base chance to hit */

	/* Differences from melee weapons:
	 *
	 * Dex still gives a bonus, but strength does not.
	 * Polymorphed players lacking attacks may still throw.
	 * There's a base -2 to hit.
	 * No bonuses for fleeing or stunned targets (they don't dodge
	 *    melee blows as readily, but dodging arrows is hard anyway).
	 * Not affected by traps, etc...
	 * Certain items which don't in themselves do damage ignore tmp.
	 */
	tmp = -2 + Luck + mon->data->ac;
#ifdef POLYSELF
	if (u.umonnum >= 0) tmp += uasmon->mlevel;
	else
#endif
		tmp += u.ulevel;
	if(ACURR(A_DEX) < 4) tmp -= 3;
	else if(ACURR(A_DEX) < 6) tmp -= 2;
	else if(ACURR(A_DEX) < 8) tmp -= 1;
	else if(ACURR(A_DEX) > 15) tmp += (ACURR(A_DEX) - 15);

	if(mon->msleep) {
		mon->msleep = 0;
		tmp += 2;
	}
	if(mon->mfroz) {
		tmp += 4;
		if(!rn2(10)) mon->mfroz = 0;
	}
	if (is_orc(mon->data) && pl_character[0]=='E') tmp++;
	if (u.uswallow && mon == u.ustuck) tmp += 1000; /* Guaranteed hit */

	if(obj->olet == GEM_SYM && mon->data->mlet == S_UNICORN) {
		if (mon->mtame)
			kludge("%s catches and drops the %s.",
				Monnam(mon), xname(obj));
		else {
			kludge("%s catches the %s.", Monnam(mon), xname(obj));
			gem_accept(mon, obj);
		}
		return(1);
	}
	if(obj->olet == WEAPON_SYM || obj->otyp == PICK_AXE ||
	   obj->otyp == UNICORN_HORN || obj->olet == GEM_SYM) {
		if(obj->otyp < DART || obj->olet == GEM_SYM) {
		    if (!uwep ||
			objects[obj->otyp].w_propellor !=
			-objects[uwep->otyp].w_propellor)
			    tmp -= 4;
		    else    tmp += uwep->spe;
		} else if(obj->otyp == BOOMERANG) tmp += 4;
		tmp += obj->spe;
		tmp += hitval(obj, mon->data);
		if(tmp >= rnd(20)) {
			if(hmon(mon,obj,1) == TRUE){
			  /* mon still alive */
#ifdef WORM
			  cutworm(mon,bhitpos.x,bhitpos.y,obj->otyp);
#endif
			} else mon = 0;
			/* projectiles thrown disappear sometimes */
			if((obj->otyp < BOOMERANG || obj->olet == GEM_SYM)
								&& rn2(3)) {
				/* check bill; free */
				obfree(obj, (struct obj *)0);
				return(1);
			}
		} else miss(xname(obj), mon);
	} else if(obj->otyp == HEAVY_IRON_BALL) {
		if(obj != uball) tmp += 2;
		if(tmp >= rnd(20)) {
			if(hmon(mon,obj,1) == FALSE)
				mon = 0;	/* he died */
		} else miss(xname(obj), mon);
	} else if (obj->otyp == BOULDER) {
		tmp += 6;  /* Likely to hit! */
		if(tmp >= rnd(20)) {
			if(hmon(mon,obj,1) == FALSE)
				mon = 0;	/* he died */
		} else miss(xname(obj), mon);
	} else if((obj->otyp == CREAM_PIE
#ifdef POLYSELF
			|| obj->otyp == BLINDING_VENOM
#endif
					) && ACURR(A_DEX) >= rnd(10)) {
		(void) hmon(mon,obj,1); /* can't die from it */
#ifdef POLYSELF
	} else if(obj->otyp == ACID_VENOM && ACURR(A_DEX) >= rnd(10)) {
		if(hmon(mon,obj,1) == FALSE)
			mon = 0;
#endif
	} else if(obj->olet == POTION_SYM && ACURR(A_DEX) >= rnd(15)) {
		potionhit(mon, obj);
		return(1);
	} else {
		pline("The %s misses %s.", xname(obj),
			cansee(bhitpos.x,bhitpos.y) ? mon_nam(mon) : "it");
		if(obj->olet == FOOD_SYM &&
		  (mon->data->mlet == S_DOG || mon->data->mlet == S_FELINE))
			if(tamedog(mon,obj)) return(1);
	}
	return(0);
}

static void
gem_accept(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	char buf[BUFSZ];
	static const char nogood[] = " is not interested in your junk.";
	static const char maybeluck[] = " hesitatingly accepts your gift.";
	static const char addluck[] = " graciously accepts your gift.";

	Strcpy(buf,Monnam(mon));

	mon->mpeaceful = 1;
	if(obj->dknown && objects[obj->otyp].oc_name_known)  {
		if(objects[obj->otyp].g_val > 0)  {
		    if(mon->data == &mons[
				((u.ualigntyp== U_CHAOTIC) ? PM_BLACK_UNICORN :
				 (u.ualigntyp == U_LAWFUL) ? PM_WHITE_UNICORN
						  : PM_GRAY_UNICORN)]) {
			    Strcat(buf, addluck);
			    change_luck(5);
		    } else {
			    Strcat(buf, maybeluck);
			    change_luck(rn2(7)-3);
		    }
		} else {
		    Strcat(buf,nogood);
		    goto nopick;
		}
	}  else  {  /* value unknown to @ */
		change_luck(1);
		Strcat(buf,addluck);
	}
	mpickobj(mon, obj);
nopick:
	if(!Blind) pline(buf);
	rloc(mon);
}

/* returns 0 if object doesn't break	*/
/* returns 1 if object broke 		*/
int
breaks(obj, loose)
register struct obj   *obj;
register boolean loose;		/* if not loose, obj is in fobj chain */
{
	switch(obj->otyp) {
#ifdef MEDUSA
		case MIRROR:
			change_luck(-2);	/* and fall through */
#endif
		case EXPENSIVE_CAMERA:
		case CRYSTAL_BALL:
			if(!Blind)
			    pline("%s shatters into a thousand pieces!",
				Doname2(obj));
			else You("hear something shatter!");
			break;
		case EGG:
			pline("Splat!");
			break;
		case CREAM_PIE:
			pline("What a mess!");
			break;
		case ACID_VENOM:
		case BLINDING_VENOM:
			pline("Splash!");
			break;
		default:
			return 0;
	}

	if(loose) {
		unpobj(obj);
		obfree(obj, (struct obj *)0);
	} else {
		addtobill(obj, FALSE);
		delobj(obj);
	}
	return(1);
}

static boolean 
martial() 
{
	return((pl_character[0] == 'S' || pl_character[0] == 'P'));
}

static int
throw_gold(obj)
struct obj *obj;
{
	int range = 0, odx, ody;
	long zorks = OGOLD(obj);
	register struct monst *mon;

	free((genericptr_t) obj);
	if(zorks < 0) {
		/* watch negative overflows a la drop() */
		u.ugold += zorks;
	pline("The LRS would be very interested to know you have that much.");
		return(0);
	}

	if(u.uswallow) {
		pline("The gold disappears in the %s's entrails.", 
					mon_nam(u.ustuck));
		u.ustuck->mgold += zorks;
		return(1);
	}

	if(u.dz) {
	  	if(u.dz < 0) {
	    pline("The gold hits the ceiling, then falls back on top of your %s.",
		    body_part(HEAD));
		    /* some self damage? */
		    if(uarmh) pline("Fortunately, you are wearing a helmet!");
		} else pline("The gold hits the floor.");
		bhitpos.x = u.ux; /* a msg is needed here */
		bhitpos.y = u.uy;
		goto skip;
	}

	range = rnd((int)ACURR(A_STR));
	if(martial()) range = range + rnd(3);

	/* see if the gold has a place to move into */
	odx = u.ux + u.dx;
	ody = u.uy + u.dy;
	if(bad_kick_throw_pos(odx,ody)) {
		bhitpos.x = u.ux;
		bhitpos.y = u.uy;
	} else {
		if (mon = ghit(u.dx, u.dy, range))
		    if (ghitm(mon, zorks))	/* was it caught? */
			zorks = 0;
	}
skip:
	if (zorks)	/* perhaps it was caught */
	    mkgold(zorks, bhitpos.x, bhitpos.y);
	if(cansee(bhitpos.x, bhitpos.y)) prl(bhitpos.x,bhitpos.y);
	return(1);
}
