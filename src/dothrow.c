/*	SCCS Id: @(#)dothrow.c	3.1	92/12/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 't' (throw) */

#include "hack.h"

static void FDECL(hitfloor, (struct obj *));
static int FDECL(gem_accept, (struct monst *, struct obj *));
static int FDECL(throw_gold, (struct obj *));
static void FDECL(check_shop_obj, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));

static const char NEARDATA toss_objs[] =
	{ ALLOW_COUNT, GOLD_CLASS, ALL_CLASSES, WEAPON_CLASS, 0 };
extern boolean notonhead;	/* for long worms */

int
dothrow()
{
	register struct obj *obj;

	if(check_capacity(NULL)) return(0);
	obj = getobj(toss_objs, "throw");
	/* it is also possible to throw food */
	/* (or jewels, or iron balls... ) */

	if(!obj || !getdir(NULL)) {       /* ask "in what direction?" */
		if (obj && obj->oclass == GOLD_CLASS) {
		    u.ugold += obj->quan;
		    flags.botl = 1;
		    dealloc_obj(obj);
		}
		return(0);
	}

	if(obj->oclass == GOLD_CLASS) return(throw_gold(obj));

	if(!canletgo(obj,"throw"))
		return(0);
	if (obj->oartifact == ART_MJOLLNIR && obj != uwep) {
		You("must be wielding %s in order to throw it.", xname(obj));
		return(0);
	}
	if ((obj->oartifact == ART_MJOLLNIR && ACURR(A_STR) != 125)
	   || (obj->otyp == BOULDER
#ifdef POLYSELF
					&& !throws_rocks(uasmon)
#endif
								)) {
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
	    if(obj->quan > 1L)
		setuwep(splitobj(obj, 1L));
	    else {
		setuwep((struct obj *)0);
		if (uwep) return(1); /* unwielded, died, rewielded */
	    }
	}
	else if(obj->quan > 1L)
		(void) splitobj(obj, 1L);
	freeinv(obj);
	return(throwit(obj));
}

static void
hitfloor(obj)
register struct obj *obj;
{
	if (IS_SOFT(levl[u.ux][u.uy].typ) || u.uinwater) {
		dropy(obj);
		if(*u.ushops)
		    check_shop_obj(obj, obj->ox, obj->oy, FALSE);
		return;
	}
	if (IS_ALTAR(levl[u.ux][u.uy].typ)) doaltarobj(obj);
	else
		pline("%s hits the floor.", Doname2(obj));
	if (breaks(obj, TRUE)) return;
	else if(obj->oclass == POTION_CLASS) {
		pline("The flask breaks, and you smell a peculiar odor...");
		potionbreathe(obj);
		if(*u.ushops)
		    check_shop_obj(obj, u.ux, u.uy, TRUE);
		obfree(obj, (struct obj *)0);
	} else {
		if(ship_object(obj, u.ux, u.uy, FALSE)) 
		    return;
		dropy(obj);
		if(*u.ushops)
		    check_shop_obj(obj, obj->ox, obj->oy, FALSE);
	}
}

/*
 * The player moves through the air for a few squares as a result of
 * throwing or kicking something.  To simplify matters, bumping into monsters
 * won't cause damage but will wake them and make them angry.
 * Auto-pickup isn't done, since you don't have control over your movements
 * at the time.
 * dx and dy should be the direction of the hurtle, not of the original
 * kick or throw.
 */
void
hurtle(dx, dy, range)
    int dx, dy, range;
{
    register struct monst *mon;
    struct obj *obj;
    int nx, ny;

    if(!range || (!dx && !dy) || u.ustuck) return; /* paranoia */

    nomul(-range);
    You("%s in the opposite direction.", range > 1 ? "hurtle" : "float");
    while(range--) {
	nx = u.ux + dx;
	ny = u.uy + dy;

	if(!isok(nx,ny)) break;
	if(IS_ROCK(levl[nx][ny].typ) || closed_door(nx,ny)) {
	    pline("Ouch!");
	    losehp(rnd(2+range), IS_ROCK(levl[nx][ny].typ) ?
		   "bumping to a wall" : "bumping into a door", KILLED_BY);
	    break;
	}

	if (obj = sobj_at(BOULDER,nx,ny)) {
	    You("bump into a %s.  Ouch!", xname(obj));
	    losehp(rnd(2+range), "bumping to a boulder", KILLED_BY);
	    break;
	}

	u.ux = nx;
	u.uy = ny;
	newsym(u.ux - dx, u.uy - dy);
	if(mon = m_at(u.ux, u.uy)) {
	    You("bump into %s.", a_monnam(mon));
	    wakeup(mon);
	    if(Is_airlevel(&u.uz))
		mnexto(mon);
	    else {
		/* sorry, not ricochets */
		u.ux -= dx;
		u.uy -= dy;
	    }
	    range = 0;
	}

	newsym(u.ux, u.uy);

	if(range) {
	    flush_screen(1);
	    delay_output();
	}
    }
}

static void
check_shop_obj(obj, x, y, broken)
register struct obj *obj;
register xchar x, y;
register boolean broken;
{
	register struct monst *shkp = 
			shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE));

	if(!shkp) return;
	if(!inside_shop(u.ux, u.uy)) return;

	if(broken) {
	        if(obj->unpaid) {
		    (void)stolen_value(obj, u.ux, u.uy, 
				 (shkp && shkp->mpeaceful), FALSE);
		    subfrombill(obj, shkp);
		}
		return;
	}

        if(!costly_spot(x, y) ||
	           *in_rooms(u.ux, u.uy, 0) != *in_rooms(x, y, 0)) { 
	        if(!inside_shop(x, y) && obj->unpaid) {
		    (void)stolen_value(obj, u.ux, u.uy, 
				 (shkp && shkp->mpeaceful), FALSE);
		    subfrombill(obj, shkp);
		}
	} else
	        if(costly_spot(u.ux, u.uy) && costly_spot(x, y)) {
		    if(obj->unpaid) subfrombill(obj, shkp);
		    else if(!(x == shkp->mx && y == shkp->my))
		            sellobj(obj, x, y);
		}
}


int
throwit(obj)
register struct obj *obj;
{
	register struct monst *mon;
	register int range, urange;
	boolean impaired = (Confusion || Stunned || Blind ||
			   Hallucination || Fumbling);
	int do_death = 0;

	if (obj->cursed && (u.dx || u.dy) && !rn2(7)) {
		boolean slipok = TRUE;
	    if ((obj->oclass == WEAPON_CLASS || obj->oclass == GEM_CLASS)
		&& uwep && (objects[obj->otyp].w_propellor > 0) &&
		(objects[obj->otyp].w_propellor ==
                                             -objects[uwep->otyp].w_propellor))
		pline("The %s misfires!", xname(obj));
	    else {
		/* only slip if it's meant to be thrown */
		if((obj->otyp >= DART && obj->otyp <= JAVELIN) ||
		   (obj->otyp >= DAGGER && obj->otyp <= CRYSKNIFE &&
		    obj->otyp != ATHAME) || obj->otyp == WAR_HAMMER)
		    pline("The %s slips as you throw it!", xname(obj));
		else slipok = FALSE;
	    }
	    if (slipok) {
		u.dx = rn2(3)-1;
		u.dy = rn2(3)-1;
		if (!u.dx && !u.dy) u.dz = 1;
		impaired = TRUE;
	    }
	}

	if(u.uswallow) {
		mon = u.ustuck;
		bhitpos.x = mon->mx;
		bhitpos.y = mon->my;
	} else if(u.dz) {
	  if (u.dz < 0 && pl_character[0] == 'V' &&
	      obj->oartifact == ART_MJOLLNIR && !impaired) {
	      pline("%s hits the ceiling and returns to your hand!",
		    The(xname(obj)));
	      obj = addinv(obj);
	      setuwep(obj);
	      return(1);
	  }
	  if (u.dz < 0 && !Is_airlevel(&u.uz) && !Underwater && !Is_waterlevel(&u.uz)) {
	    pline("%s hits the ceiling, then falls back on top of your %s.",
		Doname2(obj),		/* note: obj->quan == 1 */
		body_part(HEAD));
	    if(obj->oclass == POTION_CLASS)
		potionhit(&youmonst, obj);
	    else {
	        int dmg = rnd((int)(obj->owt));
	      
		if (uarmh) {
		    if(is_metallic(uarmh)) {
		        pline("Fortunately, you are wearing a hard helmet.");
			dmg = 1;
		    } else if (flags.verbose)
		        Your("%s does not protect you.", xname(uarmh));
		} else if (obj->otyp == CORPSE &&
				obj->corpsenm == PM_COCKATRICE) {
#ifdef POLYSELF
		    if(!resists_ston(uasmon))
			if(!(poly_when_stoned(uasmon) &&
					polymon(PM_STONE_GOLEM))) {
#endif
			killer = doname(obj);
			You("turn to stone.");
			do_death = STONING;
#ifdef POLYSELF
		    }
#endif
		}

		if (!breaks(obj, TRUE)) {
		    if(!ship_object(obj, u.ux, u.uy, FALSE)) {
			dropy(obj);
			if(*u.ushops)
			    check_shop_obj(obj, obj->ox, obj->oy, FALSE);
		    }
		}
		if (do_death == STONING)
		    done(STONING);
		else
		    losehp(dmg, "falling object", KILLED_BY_AN);
	    }
	  } else hitfloor(obj);
	  return(1);

	} else if(obj->otyp == BOOMERANG && !Underwater) {
		if(Is_airlevel(&u.uz) || Levitation) hurtle(-u.dx, -u.dy, 1);
		mon = boomhit(u.dx, u.dy);
		if(mon == &youmonst) {		/* the thing was caught */
			exercise(A_DEX, TRUE);
			(void) addinv(obj);
			return(1);
		}
	} else {
		urange = (int)(ACURRSTR)/2;
		range = urange - (int)(obj->owt/40);
		if (obj == uball) {
			if (u.ustuck) range = 1;
			else if (range >= 5) range = 5;
		}
		if (range < 1) range = 1;

		if ((obj->oclass == WEAPON_CLASS || obj->oclass == GEM_CLASS)
                    && uwep && objects[obj->otyp].w_propellor) {
		    if (objects[obj->otyp].w_propellor ==
			                     -objects[uwep->otyp].w_propellor)
			range++;
		    else
			range /= 2;
		}

		if (Is_airlevel(&u.uz) || Levitation) {
		    /* action, reaction... */
		    urange -= range;
		    if(urange < 1) urange = 1;
		    range -= urange;
		    if(range < 1) range = 1;
		}

#ifdef POLYSELF
		if (obj->otyp == BOULDER) range = 20;
#endif
		if (obj == uball && u.utrap && u.utraptype == TT_INFLOOR)
		    range = 1;

		if (Underwater) range = 1;

		mon = bhit(u.dx,u.dy,range,THROWN_WEAPON,
			   (int (*)()) 0,(int (*)()) 0,obj);

		/* have to do this after bhit() so u.ux & u.uy are correct */
		if(Is_airlevel(&u.uz) || Levitation)
		    hurtle(-u.dx, -u.dy, urange);
	}
	if(mon) {
		if(mon->isshk && (!inside_shop(u.ux, u.uy) ||
		   !index(in_rooms(mon->mx, mon->my, SHOPBASE), *u.ushops))) { 
		    if(obj->otyp == PICK_AXE) {
		        register struct obj *otmp;

			/* check if the pick axe was caught through  */
			/* a successful call to shkcatch() in bhit() */
	                for (otmp = mon->minvent; otmp; otmp = otmp->nobj)
		             if (otmp == obj) return(1);
		    }
		    wakeup(mon);
		    hot_pursuit(mon);
		}
		(void) snuff_candle(obj);
		/* awake monster if sleeping */
		wakeup(mon);
		notonhead = (bhitpos.x != mon->mx || bhitpos.y != mon->my);
		if(thitmonst(mon, obj)) return(1);
	}
	if(!u.uswallow) {
		/* the code following might become part of dropy() */
		int obj_glyph = obj_to_glyph(obj);
		boolean gone = FALSE;

		if (obj->oartifact == ART_MJOLLNIR && pl_character[0] == 'V') {
		    /* we must be wearing Gauntlets of Power to get here */
		    int x = bhitpos.x - u.dx, y = bhitpos.y - u.dy;

		    tmp_at(DISP_FLASH, obj_glyph);
		    while(x != u.ux || y != u.uy) {
			tmp_at(x, y);
			delay_output();
			x -= u.dx; y -= u.dy;
		    }
		    tmp_at(DISP_END, 0);

		    if(!impaired) {
			pline("%s returns to your hand!", The(xname(obj)));
			obj = addinv(obj);
			setuwep(obj);
			if(cansee(bhitpos.x, bhitpos.y))
			    newsym(bhitpos.x,bhitpos.y);
		    } else {
			int dmg = rnd(4);
			if (Blind)
			    pline("%s hits your %s!",
				  The(xname(obj)), body_part(ARM));
			else
			    pline("%s flies back toward you, hitting your %s!",
				  The(xname(obj)), body_part(ARM));
			(void) artifact_hit((struct monst *) 0, &youmonst,
					    obj, &dmg, 0);
			losehp(dmg, xname(obj), KILLED_BY);
			if(ship_object(obj, u.ux, u.uy, FALSE)) 
		            return (1);
			dropy(obj);
		    }
		    return (1);
		}
		if (!IS_SOFT(levl[bhitpos.x][bhitpos.y].typ) && !u.uinwater &&
		    obj->oclass == POTION_CLASS && rn2(2)) {
		    if(distu(bhitpos.x, bhitpos.y) < 3 && rn2(5)) {
			pline("The flask breaks, and you smell a peculiar odor...");
			potionbreathe(obj);
		    } else if(!Blind)
			pline("The flask breaks.");
		    else pline("Crash!");
		    if(*u.ushops)
		        check_shop_obj(obj, bhitpos.x, bhitpos.y, TRUE);
		    obfree(obj, (struct obj *)0);
		    gone = TRUE;
		}
		if (gone || (!IS_SOFT(levl[bhitpos.x][bhitpos.y].typ) &&
			     breaks(obj, TRUE))) {
		    tmp_at(DISP_FLASH, obj_glyph);
		    tmp_at(bhitpos.x, bhitpos.y);
		    delay_output();
		    tmp_at(DISP_END, 0);
		    return(1);
		}
		if(flooreffects(obj,bhitpos.x,bhitpos.y,"fall")) return(1);
		if(obj->otyp == CRYSKNIFE)
			obj->otyp = WORM_TOOTH;
	        if(mon && mon->isshk && obj->otyp == PICK_AXE) {
		        mpickobj(mon, obj);
			if(*u.ushops)
			    check_shop_obj(obj, bhitpos.x, bhitpos.y, FALSE);
			return(1);
		}
		(void) snuff_candle(obj);
		if(!mon && obj != uball) {
		    if(ship_object(obj, bhitpos.x, bhitpos.y, FALSE))
		        return(1);
		}
		obj->nobj = fobj;
		fobj = obj;
		place_object(obj, bhitpos.x, bhitpos.y);
		if(*u.ushops && obj != uball)
		    check_shop_obj(obj, bhitpos.x, bhitpos.y, FALSE);
		stackobj(obj);
		if (obj == uball) drop_ball(bhitpos.x, bhitpos.y);
		if(cansee(bhitpos.x, bhitpos.y)) newsym(bhitpos.x,bhitpos.y);
	} else {
		/* ball is not picked up by monster */
		if (obj != uball) mpickobj(u.ustuck,obj);
	}
	return(1);
}

int
thitmonst(mon, obj)
register struct monst *mon;
register struct obj   *obj;
{
	register int	tmp; /* Base chance to hit */
	register int	disttmp; /* distance modifier */

	/* Differences from melee weapons:
	 *
	 * Dex still gives a bonus, but strength does not.
	 * Polymorphed players lacking attacks may still throw.
	 * There's a base -1 to hit.
	 * No bonuses for fleeing or stunned targets (they don't dodge
	 *    melee blows as readily, but dodging arrows is hard anyway).
	 * Not affected by traps, etc.
	 * Certain items which don't in themselves do damage ignore tmp.
	 * Distance and monster size affect chance to hit.
	 */
	tmp = -1 + Luck + find_mac(mon);
#ifdef POLYSELF
	if (u.umonnum >= 0) tmp += uasmon->mlevel;
	else
#endif
		tmp += u.ulevel;
	if(ACURR(A_DEX) < 4) tmp -= 3;
	else if(ACURR(A_DEX) < 6) tmp -= 2;
	else if(ACURR(A_DEX) < 8) tmp -= 1;
	else if(ACURR(A_DEX) >= 14) tmp += (ACURR(A_DEX) - 14);

	/* modify to-hit depending on distance; but keep it sane */
	disttmp = 3 - distmin(u.ux, u.uy, mon->mx, mon->my);
	if(disttmp < -4) disttmp = -4;
	tmp += disttmp;

	/* it's easier to hit a larger target */
	if(bigmonst(mon->data)) tmp++;

	if(mon->msleep) {
		mon->msleep = 0;
		tmp += 2;
	}
	if(!mon->mcanmove || !mon->data->mmove) {
		tmp += 4;
		if(!rn2(10)) {
			mon->mcanmove = 1;
			mon->mfrozen = 0;
		}
	}
	if (is_orc(mon->data) && pl_character[0]=='E') tmp++;
	if (u.uswallow && mon == u.ustuck) tmp += 1000; /* Guaranteed hit */

	if(obj->oclass == GEM_CLASS && mon->data->mlet == S_UNICORN) {
		if (mon->mtame) {
			pline("%s catches and drops %s.",
				Monnam(mon), the(xname(obj)));
			return(0);
		} else {
			pline("%s catches %s.", Monnam(mon), the(xname(obj)));
			return(gem_accept(mon, obj));
		}
	}
	if(obj->oclass == WEAPON_CLASS || obj->otyp == PICK_AXE ||
	   obj->otyp == UNICORN_HORN || obj->oclass == GEM_CLASS) {
		if(obj->otyp < DART || obj->oclass == GEM_CLASS) {
		    if (!uwep ||
			objects[obj->otyp].w_propellor !=
			-objects[uwep->otyp].w_propellor) {
			tmp -= 4;
		    } else {
			tmp += uwep->spe - uwep->oeroded;
			/*
			 * Elves and Samurais are highly trained w/bows,
			 * especially their own special types of bow.
			 * Polymorphing won't make you a bow expert.
			 */
			if ((pl_character[0] == 'E' || pl_character[0] == 'S')
			     && -objects[uwep->otyp].w_propellor == WP_BOW)
			    tmp++;
			if (pl_character[0] == 'E' && uwep->otyp == ELVEN_BOW)
			    tmp++;
			if (pl_character[0] == 'S' && uwep->otyp == YUMI)
			    tmp++;
		    }
		} else if(obj->otyp == BOOMERANG) tmp += 4;
		tmp += obj->spe;
		tmp += hitval(obj, mon->data);
		if(tmp >= rnd(20)) {
			if(hmon(mon,obj,1)){
			  /* mon still alive */
			  cutworm(mon, bhitpos.x, bhitpos.y, obj);
			}
			exercise(A_DEX, TRUE);
			/* projectiles thrown disappear sometimes */
			if((obj->otyp < BOOMERANG || obj->oclass == GEM_CLASS)
								&& rn2(3)) {
			        if(*u.ushops)
		                     check_shop_obj(obj, bhitpos.x, 
						           bhitpos.y, TRUE);
				/* check bill; free */
				obfree(obj, (struct obj *)0);
				return(1);
			}
		} else miss(xname(obj), mon);
	} else if(obj->otyp == HEAVY_IRON_BALL) {
		if(obj != uball) tmp += 2;
		exercise(A_STR, TRUE);
		if(tmp >= rnd(20)) {
			(void) hmon(mon,obj,1);
			exercise(A_DEX, TRUE);
		} else miss(xname(obj), mon);
	} else if (obj->otyp == BOULDER) {
		tmp += 6;  /* Likely to hit! */
		exercise(A_STR, TRUE);
		if(tmp >= rnd(20)) {
			(void) hmon(mon,obj,1);
			exercise(A_DEX, TRUE);
		} else miss(xname(obj), mon);
	} else if((obj->otyp == CREAM_PIE
#ifdef POLYSELF
			|| obj->otyp == BLINDING_VENOM
#endif
					) && ACURR(A_DEX) >= rnd(10)) {
		(void) hmon(mon,obj,1); /* can't die from it */
#ifdef POLYSELF
	} else if(obj->otyp == ACID_VENOM && ACURR(A_DEX) >= rnd(10)) {
		(void) hmon(mon,obj,1);
#endif
	} else if(obj->oclass == POTION_CLASS && ACURR(A_DEX) >= rnd(15)) {
		potionhit(mon, obj);
		return(1);
	} else {
		pline("%s misses %s.", The(xname(obj)), mon_nam(mon));
		if(obj->oclass == FOOD_CLASS && is_domestic(mon->data))
			if(tamedog(mon,obj)) return(1);
	}
	return(0);
}

static int
gem_accept(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	char buf[BUFSZ];
	boolean is_buddy = sgn(mon->data->maligntyp) == sgn(u.ualign.type);
	boolean is_gem = objects[obj->otyp].oc_material == GEMSTONE;
	int ret = 0;
	static const char NEARDATA nogood[] = " is not interested in your junk.";
	static const char NEARDATA acceptgift[] = " accepts your gift.";
	static const char NEARDATA maybeluck[] = " hesitatingly";
	static const char NEARDATA noluck[] = " graciously";
	static const char NEARDATA addluck[] = " gratefully";

	Strcpy(buf,Monnam(mon));

	mon->mpeaceful = 1;

	/* object properly identified */
	if(obj->dknown && objects[obj->otyp].oc_name_known) {
		if(is_gem) {
			if(is_buddy) {
				Strcat(buf,addluck);
				change_luck(5);
			} else {
				Strcat(buf,maybeluck);
				change_luck(rn2(7)-3);
			}
		} else {
			Strcat(buf,nogood);
			goto nopick;
		}
	/* making guesses */
	} else if(obj->onamelth || objects[obj->otyp].oc_uname) {
		if(is_gem) {
			if(is_buddy) {
				Strcat(buf,addluck);
				change_luck(2);
			} else {
				Strcat(buf,maybeluck);
				change_luck(rn2(3)-1);
			}
		} else {
			Strcat(buf,nogood);
			goto nopick;
		}
	/* value completely unknown to @ */
	} else {
		if(is_gem) {
			if(is_buddy) {
				Strcat(buf,addluck);
				change_luck(1);
			} else {
				Strcat(buf,maybeluck);
				change_luck(rn2(3)-1);
			}
		} else {
			Strcat(buf,noluck);
		}
	}
 	Strcat(buf,acceptgift);
	mpickobj(mon, obj);
	if(*u.ushops) check_shop_obj(obj, mon->mx, mon->my, TRUE);
	ret = 1;

nopick:
	if(!Blind) pline(buf);
	rloc(mon);
	return(ret);
}

/* returns 0 if object doesn't break	*/
/* returns 1 if object broke 		*/
int
breaks(obj, loose)
register struct obj   *obj;
register boolean loose;		/* if not loose, obj is in fobj chain */
{
	switch(obj->otyp) {
		case MIRROR:
			change_luck(-2);	/* and fall through */
		case CRYSTAL_BALL:
#ifdef TOURIST
		case EXPENSIVE_CAMERA:
#endif
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

	/* it is currently assumed that 'loose' is co-extensive
	 * with 'thrown'.  if this changes, an explicit 'thrown'
	 * arg must be added to breaks() to ensure proper 
	 * treatment of shop objs.
	 */
	if(loose) {
		newsym(obj->ox,obj->oy);
		if(*u.ushops) 
	            check_shop_obj(obj, obj->ox, obj->oy, TRUE);
		obfree(obj, (struct obj *)0);
	} else {
	        /* it is assumed that the obj is a floor-object */ 
	        register struct monst *shkp;
	        boolean costly, insider;
		long loss = 0L;

#ifdef GCC_WARN
		shkp = (struct monst *) 0;
#endif

		costly = (costly_spot(obj->ox, obj->oy) && 
				   (shkp = shop_keeper(*in_rooms(obj->ox,
				  obj->oy, SHOPBASE))) != (struct monst *)0);
		insider = (*u.ushops && inside_shop(u.ux, u.uy) &&
			 *in_rooms(obj->ox, obj->oy, SHOPBASE) == *u.ushops);

		if(costly)
		    loss = stolen_value(obj, u.ux, u.uy, 
				 (shkp && shkp->mpeaceful), FALSE);
		if(loss && !insider)
		    make_angry_shk(shkp, obj->ox, obj->oy);

		delobj(obj);
	}
	return(1);
}

/*
 *  Note that the gold object is *not* attached to the fobj chain.
 */
static int
throw_gold(obj)
struct obj *obj;
{
	int range, odx, ody;
	long zorks = obj->quan;
	register struct monst *mon;

	if(u.uswallow) {
		pline(is_animal(u.ustuck->data) ?
			"%s in the %s's entrails." : "%s into %s.",
			"The gold disappears", mon_nam(u.ustuck));
		u.ustuck->mgold += zorks;
		dealloc_obj(obj);
		return(1);
	}

	if(u.dz) {
	  	if(u.dz < 0 && !Is_airlevel(&u.uz) && !Underwater && !Is_waterlevel(&u.uz)) {
	pline("The gold hits the ceiling, then falls back on top of your %s.",
		    body_part(HEAD));
		    /* some self damage? */
		    if(uarmh) pline("Fortunately, you are wearing a helmet!");
		}
		if(flooreffects(obj,u.ux,u.uy,"fall")) return(1);
		if(u.dz > 0) pline("The gold hits the floor.");
		obj->nobj = fobj;	/* add the gold to the object list */
		fobj = obj;
		place_object(obj,u.ux,u.uy);
                if(*u.ushops) sellobj(obj, u.ux, u.uy);
		stackobj(obj);
		newsym(u.ux,u.uy);
		return 1;
	}

	/* consistent with range for normal objects */
	range = (int)((ACURRSTR)/2 - obj->owt/40);

	/* see if the gold has a place to move into */
	odx = u.ux + u.dx;
	ody = u.uy + u.dy;
	if(!ZAP_POS(levl[odx][ody].typ) || closed_door(odx, ody)) {
		bhitpos.x = u.ux;
		bhitpos.y = u.uy;
	} else {
		mon = bhit(u.dx, u.dy, range, THROWN_WEAPON,
			       (int (*)()) 0, (int (*)()) 0, obj);
		if(mon) {
		    if (ghitm(mon, obj))	/* was it caught? */
			return 1;
		} else {
		    if(ship_object(obj, bhitpos.x, bhitpos.y, FALSE)) 
		        return 1;
		}
	}

	if(flooreffects(obj,bhitpos.x,bhitpos.y,"fall")) return(1);
	obj->nobj = fobj;	/* add the gold to the object list */
	fobj = obj;
	place_object(obj,bhitpos.x,bhitpos.y);
        if(*u.ushops) sellobj(obj, bhitpos.x, bhitpos.y);
	stackobj(obj);
	newsym(bhitpos.x,bhitpos.y);
	return(1);
}

/*dothrow.c*/
