/*	SCCS Id: @(#)apply.c	3.1	93/05/25	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

#ifdef OVLB

static NEARDATA const char tools[] = { TOOL_CLASS, 0 };

static NEARDATA boolean did_dig_msg;

#ifdef TOURIST
static int FDECL(use_camera, (struct obj *));
#endif
static int FDECL(use_towel, (struct obj *));
static void FDECL(use_stethoscope, (struct obj *));
static void FDECL(use_whistle, (struct obj *));
static void FDECL(use_magic_whistle, (struct obj *));
#ifdef WALKIES
static void FDECL(use_leash, (struct obj *));
#endif
STATIC_DCL int NDECL(dig);
#ifdef OVLB
STATIC_DCL schar FDECL(fillholetyp, (int, int));
#endif
static boolean FDECL(wield_tool, (struct obj *));
static int FDECL(use_pick_axe, (struct obj *));
static int FDECL(use_mirror, (struct obj *));
static void FDECL(use_bell, (struct obj *));
static void FDECL(use_candelabrum, (struct obj *));
static void FDECL(use_candle, (struct obj *));
static void FDECL(use_lamp, (struct obj *));
static void FDECL(use_tinning_kit, (struct obj *));
static void FDECL(use_figurine, (struct obj *));
static void FDECL(use_grease, (struct obj *));
static boolean NDECL(rm_waslit);
static void FDECL(mkcavepos, (XCHAR_P,XCHAR_P,int,BOOLEAN_P,BOOLEAN_P));
static void FDECL(mkcavearea, (BOOLEAN_P));
static void FDECL(digactualhole, (int));

#ifdef TOURIST
static int
use_camera(obj)
	struct obj *obj;
{
	register struct monst *mtmp;

	if(Underwater) {
		pline("Using your camera underwater would void the warranty.");
		return(0);
	}
	if(!getdir(NULL)) return(0);
	if(u.uswallow) {
		You("take a picture of %s's %s.", mon_nam(u.ustuck),
		    is_animal(u.ustuck->data) ? "stomach" : "interior");
	} else if(obj->cursed && !rn2(2)) goto blindu;
	else if(u.dz) {
		You("take a picture of the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : "ceiling");
	} else if(!u.dx && !u.dy) {
blindu:
		if(!Blind) {
			You("are blinded by the flash!");
			make_blinded((long)rnd(25),FALSE);
		}
	} else if ((mtmp = bhit(u.dx,u.dy,COLNO,FLASHED_LIGHT,
				(int(*)())0,(int(*)())0,obj)) != 0) {
		if(mtmp->msleep) {
		    mtmp->msleep = 0;
		    if(cansee(mtmp->mx,mtmp->my))
			pline("The flash awakens %s.", mon_nam(mtmp)); /* a3 */
		} else if (mtmp->data->mlet != S_LIGHT)
		    if(mtmp->mcansee && haseyes(mtmp->data)) {
			register int tmp = distu(mtmp->mx,mtmp->my);

			if(cansee(mtmp->mx,mtmp->my))
			    pline("%s is blinded by the flash!", Monnam(mtmp));
			if(mtmp->data == &mons[PM_GREMLIN]) {
			    /* Rule #1: Keep them out of the light. */
			    pline("%s cries out in pain!", Monnam(mtmp));
			    if (mtmp->mhp > 1) mtmp->mhp--;
			}
			setmangry(mtmp);
			if(tmp < 9 && !mtmp->isshk && rn2(4)) {
				mtmp->mflee = 1;
				if(rn2(4)) mtmp->mfleetim = rnd(100);
			}
			mtmp->mcansee = 0;
			if(tmp < 3) {
				mtmp->mblinded = 0;
			} else {
				mtmp->mblinded = rnd(1 + 50/tmp);
			}
		    }
	}
	return 1;
}
#endif

static int
use_towel(obj)
	struct obj *obj;
{
	if(!freehand()) {
		You("have no free %s!", body_part(HAND));
		return 0;
	} else if (obj->owornmask) {
		You("cannot use it while you're wearing it!");
		return 0;
	} else if (obj->cursed) {
		long old;
		switch (rn2(3)) {
		case 2:
		    old = Glib;
		    Glib += rn1(10, 3);
		    Your("%s %s!", makeplural(body_part(HAND)),
			(old ? "are filthier than ever" : "get slimy"));
		    return 1;
		case 1:
		    if (!Blindfolded) {
			old = u.ucreamed;
			u.ucreamed += rn1(10, 3);
			pline("Yecch! Your %s %s gunk on it!", body_part(FACE),
			      (old ? "has more" : "now has"));
			make_blinded(Blinded + (long)u.ucreamed - old, TRUE);
		    } else {
			if (ublindf->cursed) {
			    You("push your blindfold %s.",
				rn2(2) ? "cock-eyed" : "crooked");
			} else {
			    You("push your blindfold off.");
			    Blindf_off(ublindf);
			    dropx(ublindf);
			}
		    }
		    return 1;
		case 0:
		    break;
		}
	}

	if (Glib) {
		Glib = 0;
		You("wipe off your %s.", makeplural(body_part(HAND)));
		return 1;
	} else if(u.ucreamed) {
		Blinded -= u.ucreamed;
		u.ucreamed = 0;

		if (!Blinded) {
			pline("You've got the glop off.");
			Blinded = 1;
			make_blinded(0L,TRUE);
		} else {
			Your("%s feels clean now.", body_part(FACE));
		}
		return 1;
	}

	Your("%s and %s are already clean.",
		body_part(FACE), makeplural(body_part(HAND)));

	return 0;
}

static char hollow_str[] = "hear a hollow sound.  This must be a secret %s!";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless. */
static void
use_stethoscope(obj)
	register struct obj *obj;
{
	register struct monst *mtmp;
	register struct rm *lev;
	register int rx, ry;

	if(!freehand()) {
		You("have no free %s.", body_part(HAND));
		return;
	}
	if (!getdir(NULL)) return;
	if (u.uswallow && (u.dx || u.dy || u.dz)) {
		mstatusline(u.ustuck);
		return;
	} else if (u.dz) {
		if (Underwater)
		    You("hear faint splashing.");
		else if (u.dz < 0 || Levitation)
		    You("can't reach the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : "ceiling");
		else if (Is_stronghold(&u.uz))
		    You("hear the crackling of hellfire.");
		else
		    pline("The %s seems healthy enough.", surface(u.ux,u.uy));
		return;
	} else if (obj->cursed && !rn2(2)) {
		You("hear your heart beat.");
		return;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy) {
		ustatusline();
		return;
	}
	rx = u.ux + u.dx; ry = u.uy + u.dy;
	if (!isok(rx,ry)) {
		You("hear a faint typing noise.");
		return;
	}
	if ((mtmp = m_at(rx,ry)) != 0) {
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx,ry)) newsym(mtmp->my,mtmp->my);
		}
		return;
	}
	lev = &levl[rx][ry];
	switch(lev->typ) {
	case SDOOR:
		You(hollow_str, "door");
		lev->typ = DOOR;
		newsym(rx,ry);
		return;
	case SCORR:
		You(hollow_str, "passage");
		lev->typ = CORR;
		newsym(rx,ry);
		return;
	}
	You("hear nothing special.");
}

static char whistle_str[] = "produce a %s whistling sound.";

/*ARGSUSED*/
static void
use_whistle(obj)
struct obj *obj;
#if defined(applec)
# pragma unused(obj)
#endif
{
	You(whistle_str, "high");
	wake_nearby();
}

static void
use_magic_whistle(obj)
struct obj *obj;
{
	register struct monst *mtmp;

	if(obj->cursed && !rn2(2)) {
		You("produce a high-pitched humming noise.");
		wake_nearby();
	} else {
		makeknown(MAGIC_WHISTLE);
		You(whistle_str, Hallucination ? "normal" : "strange");
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(mtmp->mtame) mnexto(mtmp);
	}
}

boolean
um_dist(x,y,n)
register xchar x, y, n;
{
	return(abs(u.ux - x) > n  || abs(u.uy - y) > n);
}

#endif /* OVLB */

#ifdef WALKIES
#define MAXLEASHED	2

#ifdef OVLB

int
number_leashed()
{
	register int i = 0;
	register struct obj *obj;

	for(obj = invent; obj; obj = obj->nobj)
		if(obj->otyp == LEASH && obj->leashmon != 0) i++;
	return(i);
}

void
o_unleash(otmp)		/* otmp is about to be destroyed or stolen */
register struct obj *otmp;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->m_id == (unsigned)otmp->leashmon)
			mtmp->mleashed = 0;
	otmp->leashmon = 0;
}

void
m_unleash(mtmp)		/* mtmp is about to die, or become untame */
register struct monst *mtmp;
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH &&
				otmp->leashmon == (int)mtmp->m_id)
			otmp->leashmon = 0;
	mtmp->mleashed = 0;
}

void
unleash_all()		/* player is about to die (for bones) */
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH) otmp->leashmon = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->mtame) mtmp->mleashed = 0;
}

/* ARGSUSED */
static void
use_leash(obj)
struct obj *obj;
{
	register int x, y;
	register struct monst *mtmp;
	int spotmon;

	if(!obj->leashmon && number_leashed() >= MAXLEASHED) {
		You("cannot leash any more pets.");
		return;
	}

	if(!getdir(NULL)) return;

	x = u.ux + u.dx;
	y = u.uy + u.dy;

	if((x == u.ux) && (y == u.uy)) {
		pline("Leash yourself?  Very funny...");
		return;
	}

	if(!(mtmp = m_at(x, y))) {
		pline("There is no creature there.");
		return;
	}

	spotmon = canseemon(mtmp) || sensemon(mtmp);

	if(!mtmp->mtame) {
	    if(!spotmon)
		pline("There is no creature there.");
	    else
		pline("%s %s leashed!", Monnam(mtmp), (!obj->leashmon) ?
				"cannot be" : "is not");
	    return;
	}
	if(!obj->leashmon) {
		if(mtmp->mleashed) {
			pline("This %s is already leashed.",
			      spotmon ? l_monnam(mtmp) : "monster");
			return;
		}
		You("slip the leash around %s%s.",
		    spotmon ? "your " : "", l_monnam(mtmp));
		mtmp->mleashed = 1;
		obj->leashmon = (int)mtmp->m_id;
		if(mtmp->msleep)  mtmp->msleep = 0;
		return;
	}
	if(obj->leashmon != (int)mtmp->m_id) {
		pline("This leash is not attached to that creature.");
		return;
	} else {
		if(obj->cursed) {
			pline("The leash would not come off!");
			obj->bknown = TRUE;
			return;
		}
		mtmp->mleashed = 0;
		obj->leashmon = 0;
		You("remove the leash from %s%s.",
		    spotmon ? "your " : "", l_monnam(mtmp));
	}
	return;
}

#endif /* OVLB */
#ifdef OVL1

boolean
next_to_u()
{
	register struct monst *mtmp;
	register struct obj *otmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->mleashed) {
			if (distu(mtmp->mx,mtmp->my) > 2) mnexto(mtmp);
			if (distu(mtmp->mx,mtmp->my) > 2) {
			    for(otmp = invent; otmp; otmp = otmp->nobj)
				if(otmp->otyp == LEASH &&
					otmp->leashmon == (int)mtmp->m_id) {
				    if(otmp->cursed) return(FALSE);
				    You("feel %s leash go slack.",
					(number_leashed() > 1) ? "a" : "the");
				    mtmp->mleashed = 0;
				    otmp->leashmon = 0;
				}
			}
		}
	return(TRUE);
}

#endif /* OVL1 */
#ifdef OVLB
struct obj *
get_mleash(mtmp)	/* assuming mtmp->mleashed has been checked */
register struct monst *mtmp;
{
	register struct obj *otmp;

	otmp = invent;
	while(otmp) {
		if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
			return(otmp);
		otmp = otmp->nobj;
	}
	return((struct obj *)0);
}
#endif /* OVLB */

#endif /* WALKIES */
#ifdef OVL0

#ifdef WALKIES
void
check_leash(x, y)
register xchar x, y;
{
	register struct obj *otmp;
	register struct monst *mtmp = fmon;

	for(otmp = invent; otmp; otmp = otmp->nobj)
	    if(otmp->otyp == LEASH && otmp->leashmon != 0) {
		while(mtmp) {
		    if((int)mtmp->m_id == otmp->leashmon &&
			    (dist2(u.ux,u.uy,mtmp->mx,mtmp->my) >
				dist2(x,y,mtmp->mx,mtmp->my))
			) {
			if(otmp->cursed) {
			    if(um_dist(mtmp->mx, mtmp->my, 5)) {
				pline("%s chokes to death!",Monnam(mtmp));
				mondied(mtmp);
			    } else
				if(um_dist(mtmp->mx, mtmp->my, 3))
					pline("%s chokes on the leash!",
						Monnam(mtmp));
			} else {
			    if(um_dist(mtmp->mx, mtmp->my, 5)) {
				pline("%s leash snaps loose!",
					s_suffix(Monnam(mtmp)));
				m_unleash(mtmp);
			    } else {
				if(um_dist(mtmp->mx, mtmp->my, 3)) {
				    You("pull on the leash.");
# ifdef SOUNDS
				    if (mtmp->data->msound != MS_SILENT)
				    switch(rn2(3)) {
					case 0:  growl(mtmp);	break;
					case 1:  yelp(mtmp);	break;
					default: whimper(mtmp); break;
				    }
# endif
				}
			    }
			}
		    }
		    mtmp = mtmp->nmon;
		}
	    }
}

#endif /* WALKIES */

#endif /* OVL0 */
#ifdef OVLB

static boolean
rm_waslit() {
    register xchar x, y;

    if(levl[u.ux][u.uy].typ == ROOM && levl[u.ux][u.uy].waslit)
	return(TRUE);
    for(x = u.ux-2; x < u.ux+3; x++)
	for(y = u.uy-1; y < u.uy+2; y++)
	    if(isok(x,y) && levl[x][y].waslit) return(TRUE);
    return(FALSE);
}

/* Change level topology.  Messes with vision tables and ignores things like
 * boulders in the name of a nice effect.  Vision will get fixed up again
 * immediately after the effect is complete.
 */
static void
mkcavepos(x, y, dist, waslit, rockit)
    xchar x,y;
    int dist;
    boolean waslit, rockit;
{
    register struct rm *lev;

    if(!isok(x,y)) return;
    lev = &levl[x][y];

    if(rockit) {
	register struct monst *mtmp;

	if(IS_ROCK(lev->typ)) return;
	if(t_at(x, y)) return; /* don't cover the portal */
	if ((mtmp = m_at(x, y)) != 0)	/* make sure crucial monsters survive */
	    if(!passes_walls(mtmp->data)) rloc(mtmp);
    } else if(lev->typ == ROOM) return;

    unblock_point(x,y);	/* make sure vision knows this location is open */

    /* fake out saved state */
    lev->seen = FALSE;
    lev->doormask = 0;
    if(dist < 3) lev->lit = (rockit ? FALSE : TRUE);
    if(waslit) lev->waslit = (rockit ? FALSE : TRUE);
    lev->horizontal = FALSE;
    viz_array[y][x] = (dist < 3 ) ?
	(IN_SIGHT|COULD_SEE) : /* short-circuit vision recalc */
	COULD_SEE;
    lev->typ = (rockit ? STONE : ROOM);
    if(dist >= 3)
	impossible("mkcavepos called with dist %d", dist);
    if(Blind)
	feel_location(x, y);
    else newsym(x,y);
}

static void
mkcavearea(rockit)
register boolean rockit;
{
    int dist;
    xchar xmin = u.ux, xmax = u.ux;
    xchar ymin = u.uy, ymax = u.uy;
    register xchar i;
    register boolean waslit = rm_waslit();

    if(rockit) pline("Crash!  The ceiling collapses around you!");
    else pline("A mysterious force %s cave around you!",
	     (levl[u.ux][u.uy].typ == CORR) ? "creates a" : "extends the");
    display_nhwindow(WIN_MESSAGE, TRUE);

    for(dist = 1; dist <= 2; dist++) {
	xmin--; xmax++;

	/* top and bottom */
	if(dist < 2) { /* the area is wider that it is high */
	    ymin--; ymax++;
	    for(i = xmin+1; i < xmax; i++) {
		mkcavepos(i, ymin, dist, waslit, rockit);
		mkcavepos(i, ymax, dist, waslit, rockit);
	    }
	}

	/* left and right */
	for(i = ymin; i <= ymax; i++) {
	    mkcavepos(xmin, i, dist, waslit, rockit);
	    mkcavepos(xmax, i, dist, waslit, rockit);
	}

	flush_screen(1);	/* make sure the new glyphs shows up */
	delay_output();
    }

    if(!rockit && levl[u.ux][u.uy].typ == CORR) {
	levl[u.ux][u.uy].typ = ROOM;
	if(waslit) levl[u.ux][u.uy].waslit = TRUE;
	newsym(u.ux, u.uy); /* in case player is invisible */
    }

    vision_full_recalc = 1;	/* everything changed */
}

STATIC_OVL int
dig()
{
	register struct rm *lev;
	register xchar dpx = dig_pos.x, dpy = dig_pos.y;

	lev = &levl[dpx][dpy];
	/* perhaps a nymph stole your pick-axe while you were busy digging */
	/* or perhaps you teleported away */
	if(u.uswallow || !uwep || uwep->otyp != PICK_AXE ||
	    !on_level(&dig_level, &u.uz) ||
	    ((dig_down && (dpx != u.ux || dpy != u.uy)) ||
	     (!dig_down && distu(dpx,dpy) > 2)))
		return(0);

	if (dig_down) {
	    struct trap *ttmp;

	    if (On_stairs(u.ux, u.uy)) {
		if (u.ux == xdnladder || u.ux == xupladder)
		     pline("The ladder resists your effort.");
		else pline("The stairs are too hard to dig in.");
		return(0);
	    } else if (IS_THRONE(levl[u.ux][u.uy].typ)) {
		pline("The throne is too hard to break apart.");
		return (0);
	    } else if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
		pline("The altar is too hard to break apart.");
		return (0);
	    } else if (Is_airlevel(&u.uz)) {
		You("cannot dig in thin air.");
		return(0);
	    } else if (Is_waterlevel(&u.uz)) {
		pline("The water splashes and subsides.");
		return(0);
	    } else if ((ttmp = t_at(dpx, dpy)) &&
			(ttmp->ttyp == MAGIC_PORTAL || !Can_dig_down(&u.uz))) {
		pline("The %s here is too hard to dig in.",
			surface(dpx,dpy));
		return(0);
	    } else if (sobj_at(BOULDER, dpx, dpy)) {
		pline("There isn't enough room to dig here.");
		return(0);
	    }
	} else { /* !dig_down */
	    if(IS_ROCK(lev->typ) && !may_dig(dpx,dpy)) {
		pline("This wall is too hard to dig into.");
		return(0);
	    }
	}
	if(Fumbling && !rn2(3)) {
		switch(rn2(3)) {
		case 0:  if(!welded(uwep)) {
			     You("fumble and drop your %s.", xname(uwep));
			     dropx(uwep);
			     setuwep((struct obj *)0);
			 } else {
			     pline("Ouch!  Your %s bounces and hits you!",
				xname(uwep));
			     set_wounded_legs(RIGHT_SIDE, 5 + rnd(5));
			 }
			 break;
		case 1:  pline("Bang!  You hit with the broad side of %s!",
			       the(xname(uwep)));
			 break;
		default: Your("swing misses its mark.");
			 break;
		}
		return(0);
	}
	dig_effort += 10 + abon() + uwep->spe - uwep->oeroded + rn2(5);
	if(dig_down) {
		register struct trap *ttmp;

		if(dig_effort > 250) {
			dighole();
			dig_level.dnum = 0;
			dig_level.dlevel = -1;
			return(0);	/* done with digging */
		}

		if (dig_effort <= 50)
			return(1);

		if ((ttmp = t_at(dpx,dpy)) &&
		    ((ttmp->ttyp == PIT) || (ttmp->ttyp == SPIKED_PIT) ||
		     (ttmp->ttyp == TRAPDOOR)))
			return(1);

		if (IS_ALTAR(lev->typ)) {
			altar_wrath(dpx, dpy);
			angry_priest();
		}

		digactualhole(PIT);	/* at u.ux, u.uy */
		dig_level.dnum = 0;
		dig_level.dlevel = -1;
		return(0);
	}
	if(dig_effort > 100) {
		register const char *digtxt, *dmgtxt = (const char*) 0;
		register struct obj *obj;
		register boolean shopedge = *in_rooms(dpx, dpy, SHOPBASE);

		if ((obj = sobj_at(STATUE, dpx, dpy)) != 0) {
			if (break_statue(obj))
				digtxt = "The statue shatters.";
			else
				/* it was a statue trap; break_statue()
				 * printed a message and updated the screen
				 */
				digtxt = NULL;
		} else if ((obj = sobj_at(BOULDER, dpx, dpy)) != 0) {
			fracture_rock(obj);
			digtxt = "The boulder falls apart.";
		} else if(!lev->typ || lev->typ == SCORR) {
			if(Is_earthlevel(&u.uz)) {
			    if(uwep->blessed && !rn2(3)) {
				mkcavearea(FALSE);
				goto cleanup;
			    } else if((uwep->cursed && !rn2(4)) ||
					  (!uwep->blessed && !rn2(6))) {
				mkcavearea(TRUE);
				goto cleanup;
			    }
			}
			lev->typ = CORR;
			digtxt = "You succeed in cutting away some rock.";
		} else if(IS_WALL(lev->typ)) {
			if(shopedge) {
			    add_damage(dpx, dpy, 10L * ACURRSTR);
			    dmgtxt = "damage";
			}
			if (level.flags.is_maze_lev) {
			    lev->typ = ROOM;
			} else if (level.flags.is_cavernous_lev) {
			    lev->typ = CORR;
			} else {
			    lev->typ = DOOR;
			    lev->doormask = D_NODOOR;
			}
			digtxt = "You make an opening in the wall.";
		} else if(lev->typ == SDOOR) {
			lev->typ = DOOR;
			digtxt = "You break through a secret door!";
			if(!(lev->doormask & D_TRAPPED))
				lev->doormask = D_BROKEN;
		} else if(closed_door(dpx, dpy)) {
			digtxt = "You break through the door.";
			if(shopedge) {
			    add_damage(dpx, dpy, 400L);
			    dmgtxt = "break";
			}
			if(!(lev->doormask & D_TRAPPED))
				lev->doormask = D_BROKEN;
		} else return(0); /* statue or boulder got taken */

		unblock_point(dpx,dpy);	/* vision:  can see through */
		if(Blind)
		    feel_location(dpx, dpy);
		else
		    newsym(dpx, dpy);
		if(digtxt) pline(digtxt);	/* after newsym */
		if(dmgtxt)
		    pay_for_damage(dmgtxt);

		if(Is_earthlevel(&u.uz) && !rn2(3)) {
		    register struct monst *mtmp;

		    switch(rn2(2)) {
		      case 0:
			mtmp = makemon(&mons[PM_EARTH_ELEMENTAL], dpx, dpy);
			break;
		      default:
			mtmp = makemon(&mons[PM_XORN], dpx, dpy);
			break;
		    }
		    if(mtmp) pline("The debris from your digging comes to life!");
		}
		if(IS_DOOR(lev->typ) && (lev->doormask & D_TRAPPED)) {
			lev->doormask = D_NODOOR;
			b_trapped("door", 0);
			newsym(dpx, dpy);
		}
cleanup:
		dig_level.dnum = 0;
		dig_level.dlevel = -1;
		return(0);
	} else {
		if(IS_WALL(lev->typ) || closed_door(dpx, dpy)) {
		    if(*in_rooms(dpx, dpy, SHOPBASE)) {
			pline("This %s seems too hard to dig into.",
			      IS_DOOR(lev->typ) ? "door" : "wall");
			return(0);
		    }
		} else if (!IS_ROCK(lev->typ) && !sobj_at(STATUE, dpx, dpy)
				&& !sobj_at(BOULDER, dpx, dpy))
			return(0); /* statue or boulder got taken */
		if(!did_dig_msg) {
		    You("hit the %s with all your might.",
			sobj_at(STATUE, dpx, dpy) ? "statue" :
			sobj_at(BOULDER, dpx, dpy) ? "boulder" :
			IS_DOOR(lev->typ) ? "door" : "rock");
		    did_dig_msg = TRUE;
		}
	}
	return(1);
}

/* When will hole be finished? Very rough indication used by shopkeeper. */
int
holetime() {
	if(occupation != dig || !*u.ushops) return(-1);
	return((250 - dig_effort)/20);
}

/* Return typ of liquid to fill a hole with, or ROOM, if no liquid nearby */
STATIC_OVL
schar
fillholetyp(x,y)
int x, y;
{
    register int x1, y1;
    int lo_x = max(1,x-1), hi_x = min(x+1,COLNO-1),
	lo_y = max(0,y-1), hi_y = min(y+1,ROWNO-1);

    for (x1 = lo_x; x1 <= hi_x; x1++)
	for (y1 = lo_y; y1 <= hi_y; y1++)
	    if(levl[x1][y1].typ == MOAT || levl[x1][y1].typ == LAVAPOOL)
		return levl[x1][y1].typ;

    return ROOM;
}

static void
digactualhole(ttyp)
int ttyp;
{
	struct obj *oldobjs, *newobjs;
	register struct trap *ttmp;
	boolean wont_fall = !!Levitation;
#ifdef POLYSELF
		wont_fall |= !!is_flyer(uasmon);
#endif

	oldobjs = level.objects[u.ux][u.uy];
	ttmp = maketrap(u.ux, u.uy, ttyp);
	if (!ttmp) return;
	newobjs = level.objects[u.ux][u.uy];
	ttmp->tseen = 1;
	if (Invisible) newsym(ttmp->tx,ttmp->ty);

	if (ttyp == PIT) {
		if (!wont_fall) {
			u.utrap = rn1(4,2);
			u.utraptype = TT_PIT;
			vision_full_recalc = 1;	/* vision limits change */
		} else
			u.utrap = 0;
		if (oldobjs != newobjs)	/* something unearthed */
			pickup(1);	/* detects pit */
		else
			You("dig a pit.");

	} else {	/* TRAPDOOR */
		pline("You dig a hole through the %s.", surface(u.ux,u.uy));

		if (*u.ushops)
			add_damage(u.ux, u.uy, 0L);

		/* floor objects get a chance of falling down.
		 * the case where the hero does NOT fall down
		 * is treated here.  the case where the hero
		 * does fall down is treated in goto_level().
		 */
		if (u.ustuck || wont_fall) {
			if (newobjs)
				impact_drop((struct obj *)0, u.ux, u.uy, 0);
			if (oldobjs != newobjs)
				pickup(1);
		} else {
			if (*u.ushops)
				shopdig(1);
#ifdef WALKIES
			if (!next_to_u()) {
			    You("are jerked back by your pet!");
			    if (newobjs)
				impact_drop((struct obj *)0, u.ux, u.uy, 0);
			    if (oldobjs != newobjs)
				pickup(1);
			} else
#endif
			{
			    d_level newlevel;

			    You("fall through...");

			    /* earlier checks must ensure that the
			     * destination level exists and is in the
			     * present dungeon.
			     */

			    newlevel.dnum = u.uz.dnum;
			    newlevel.dlevel = u.uz.dlevel + 1;
			    goto_level(&newlevel, FALSE, TRUE, FALSE);
			}
		}
	}
}

void
dighole()
{
	register struct trap *ttmp = t_at(u.ux, u.uy);
	struct rm *lev = &levl[u.ux][u.uy];
	struct obj *boulder_here;
	schar typ;
	boolean nohole = !Can_dig_down(&u.uz);

	if (ttmp && (ttmp->ttyp == MAGIC_PORTAL || nohole)) {
		pline("The %s here is too hard to dig in.", surface(u.ux,u.uy));

	} else if (is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) {
		pline("The %s sloshes furiously for a moment, then subsides.",
			is_lava(u.ux, u.uy) ? "lava" : "water");
		wake_nearby();	/* splashing */

	} else if (lev->typ == DRAWBRIDGE_DOWN) {
		destroy_drawbridge(u.ux,u.uy);

	} else if ((boulder_here = sobj_at(BOULDER, u.ux, u.uy)) != 0) {
		if (ttmp && (ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT)) {
			pline("The boulder settles into the pit.");
			ttmp->ttyp = PIT;	 /* crush spikes */
		} else {
			/*
			 * digging makes a hole, but the boulder immediately
			 * fills it.  Final outcome:  no hole, no boulder.
			 */
			pline("KADOOM! The boulder falls in!");

			/* destroy traps that emanate from the floor */
			/* some of these are arbitrary -dlc */
			if (ttmp && ((ttmp->ttyp == SQKY_BOARD) ||
				     (ttmp->ttyp == BEAR_TRAP) ||
				     (ttmp->ttyp == LANDMINE) ||
				     (ttmp->ttyp == FIRE_TRAP) ||
				     (ttmp->ttyp == TRAPDOOR) ||
				     (ttmp->ttyp == TELEP_TRAP) ||
				     (ttmp->ttyp == LEVEL_TELEP) ||
				     (ttmp->ttyp == WEB) ||
				     (ttmp->ttyp == MAGIC_TRAP) ||
				     (ttmp->ttyp == ANTI_MAGIC))) {
				deltrap(ttmp);
				u.utrap = 0;
				u.utraptype = 0;
			}
		}
		delobj(boulder_here);

	} else if (lev->typ == DRAWBRIDGE_UP) {
		/* must be floor or ice, other cases handled above */
		/* dig "pit" and let fluid flow in (if possible) */
		typ = fillholetyp(u.ux,u.uy);

		if (typ == ROOM) {
		    if (lev->drawbridgemask & DB_ICE)
			typ = MOAT;
		    else {
			/*
			 * We can't dig a pit here since that will destroy
			 * the drawbridge.  The following is a cop-out. --dlc
			 */
			pline("The floor here is too hard to dig in.");
			return;
		    }
		}

		lev->drawbridgemask &= DB_DIR;
		if (typ == LAVAPOOL) lev->drawbridgemask |= DB_LAVA;
	    liquid_flow:
		newsym(u.ux,u.uy);

		pline("As you dig a pit, it fills with %s!",
		      typ == LAVAPOOL ? "lava" : "water");
		if (!Levitation
#ifdef POLYSELF
		   && !is_flyer(uasmon)
#endif
					) {
		    if (typ == LAVAPOOL)
			(void) lava_effects();
		    else if (!Wwalking)
			(void) drown();
		}

	} else if (IS_FOUNTAIN(lev->typ)) {
		dogushforth(FALSE);
		dryup(u.ux,u.uy);
#ifdef SINKS
	} else if (IS_SINK(lev->typ)) {
		breaksink(u.ux, u.uy);
#endif
	/* the following two are here for the wand of digging */
	} else if (IS_THRONE(lev->typ)) {
		pline("The throne is too hard to break apart.");

	} else if (IS_ALTAR(lev->typ)) {
		pline("The altar is too hard to break apart.");

	} else {
		if (lev->typ == ICE) {
			typ = fillholetyp(u.ux,u.uy);

			if (typ != ROOM) {
			    lev->typ = typ;
			    goto liquid_flow;
			}
		}

		/* finally we get to make a hole */
		if (nohole) {	/* can't make a trapdoor, so make a pit */
			digactualhole(PIT);
		} else
			digactualhole(TRAPDOOR);
	}
}

static boolean
wield_tool(obj)
struct obj *obj;
{
	if(welded(uwep)) {
		/* Andreas Bormann - ihnp4!decvax!mcvax!unido!ab */
		if(flags.verbose) {
			pline("Since your weapon is welded to your %s,",
				bimanual(uwep) ?
				(const char *)makeplural(body_part(HAND))
				: body_part(HAND));
			pline("you cannot wield that %s.", xname(obj));
		}
		return(FALSE);
	}
# ifdef POLYSELF
	if(cantwield(uasmon)) {
		You("can't hold it strongly enough.");
		return(FALSE);
	}
# endif
	unweapon = TRUE;
	You("now wield %s.", doname(obj));
	setuwep(obj);
	if (uwep != obj) return(FALSE); /* rewielded old object after dying */
	return(TRUE);
}

static int
use_pick_axe(obj)
struct obj *obj;
{
	char dirsyms[12];
	char qbuf[QBUFSZ];
	register char *dsp = dirsyms;
	register const char *sdp = flags.num_pad ? ndir : sdir;
	register struct rm *lev;
	register int rx, ry, res = 0;
	register boolean isclosedoor;

	if(obj != uwep)
	    if (!wield_tool(obj)) return(0);
	    else res = 1;

	while(*sdp) {
		(void) movecmd(*sdp);	/* sets u.dx and u.dy and u.dz */
		rx = u.ux + u.dx;
		ry = u.uy + u.dy;
		if(u.dz > 0 || (u.dz == 0 && isok(rx, ry) &&
		    (IS_ROCK(levl[rx][ry].typ)
		    || sobj_at(STATUE, rx, ry)
		    || sobj_at(BOULDER, rx, ry))))
			*dsp++ = *sdp;
		sdp++;
	}
	*dsp = 0;
	Sprintf(qbuf, "In what direction do you want to dig? [%s]", dirsyms);
	if(!getdir(qbuf))
		return(res);
	if(u.uswallow && attack(u.ustuck)) /* return(1) */;
	else if(Underwater) {
		pline("Turbulence torpedoes your digging attempts.");
	} else if(u.dz < 0) {
		if(Levitation)
			You("don't have enough leverage.");
		else
			You("can't reach the ceiling.");
	} else if(!u.dx && !u.dy && !u.dz) {
		char buf[BUFSZ];
		int dam;

		dam = rnd(2) + dbon() + obj->spe;
		if (dam <= 0) dam = 1;
		You("hit yourself with your pick-axe.");
		/* self_pronoun() won't work twice in a sentence */
		Strcpy(buf, self_pronoun("killed %sself with %%s pick-axe",
			"him"));
		losehp(dam, self_pronoun(buf, "his"), NO_KILLER_PREFIX);
		flags.botl=1;
		return(1);
	} else if(u.dz == 0) {
		if(Stunned || (Confusion && !rn2(5))) confdir();
		rx = u.ux + u.dx;
		ry = u.uy + u.dy;
		if(!isok(rx, ry)) {
			pline("Clash!");
			return(1);
		}
		lev = &levl[rx][ry];
		if(MON_AT(rx, ry) && attack(m_at(rx, ry)))
			return(1);
		isclosedoor = closed_door(rx, ry);
		if(!IS_ROCK(lev->typ)
		     && !isclosedoor
		     && !sobj_at(STATUE, rx, ry)
		     && !sobj_at(BOULDER, rx, ry)) {
			/* ACCESSIBLE or POOL */
			You("swing your %s through thin air.",
				aobjnam(obj, NULL));
		} else {
			if(dig_pos.x != rx || dig_pos.y != ry
			    || !on_level(&dig_level, &u.uz) || dig_down) {
				dig_down = FALSE;
				dig_pos.x = rx;
				dig_pos.y = ry;
				assign_level(&dig_level, &u.uz);
				dig_effort = 0;
				You("start %s.",
				   sobj_at(STATUE, rx, ry) ?
						"chipping the statue" :
				   sobj_at(BOULDER, rx, ry) ?
						"hitting the boulder" :
				   isclosedoor ? "chopping at the door" :
						"digging");
			} else
				You("continue %s.",
				   sobj_at(STATUE, rx, ry) ?
						"chipping the statue" :
				   sobj_at(BOULDER, rx, ry) ?
						"hitting the boulder" :
				   isclosedoor ? "chopping at the door" :
						"digging");
			did_dig_msg = FALSE;
			set_occupation(dig, "digging", 0);
		}
	} else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		/* it must be air -- water checked above */
		You("swing your %s through thin air.", aobjnam(obj, NULL));
	} else if(Levitation) {
		You("can't reach the %s.", surface(u.ux,u.uy));
	} else if (is_pool(u.ux, u.uy)) {
		/* Monsters which swim also happen not to be able to dig */
		You("cannot stay underwater long enough.");
	} else {
		if(dig_pos.x != u.ux || dig_pos.y != u.uy
		    || !on_level(&dig_level, &u.uz) || !dig_down) {
			dig_down = TRUE;
			dig_pos.x = u.ux;
			dig_pos.y = u.uy;
			assign_level(&dig_level, &u.uz);
			dig_effort = 0;
			You("start digging in the %s.", surface(u.ux,u.uy));
			if(*u.ushops)
				shopdig(0);
		} else
			You("continue digging in the %s.", surface(u.ux,u.uy));
		did_dig_msg = FALSE;
		set_occupation(dig, "digging", 0);
	}
	return(1);
}

#define WEAK	3	/* from eat.c */

static char look_str[] = "look %s.";

static int
use_mirror(obj)
struct obj *obj;
{
	register struct monst *mtmp;
	register char mlet;
	boolean vis;

	if(!getdir(NULL)) return 0;
	if(obj->cursed && !rn2(2)) {
		if (!Blind)
			pline("The mirror fogs up and doesn't reflect!");
		return 1;
	}
	if(!u.dx && !u.dy && !u.dz) {
		if(!Blind && !Invisible) {
#ifdef POLYSELF
		    if(u.umonnum == PM_FLOATING_EYE) {
			pline(Hallucination ?
			      "Yow!  The mirror stares back!" :
			      "Yikes!  You've frozen yourself!");
			nomul(-rnd((MAXULEV+6) - (int)u.ulevel));
		    } else if (u.usym == S_VAMPIRE)
			You("don't have a reflection.");
		    else if(u.umonnum == PM_UMBER_HULK) {
			pline("Huh?  That doesn't look like you!");
			make_confused(HConfusion + d(3,4),FALSE);
		    } else
#endif
			   if (Hallucination) You(look_str, hcolor());
		    else if (Sick)
			You(look_str, "peaked");
		    else if (u.uhs >= WEAK)
			You(look_str, "undernourished");
		    else You("look as %s as ever.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly");
		} else {
			You("can't see your %s %s.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly",
				body_part(FACE));
		}
		return 1;
	}
	if(u.uswallow) {
		if (!Blind) You("reflect %s's %s.", mon_nam(u.ustuck),
		    is_animal(u.ustuck->data)? "stomach" : "interior");
		return 1;
	}
	if(Underwater) {
		You(Hallucination ?
		    "give the fish a chance to fix their makeup." :
		    "reflect the murky water.");
		return 1;
	}
	if(u.dz) {
		if (!Blind)
		    You("reflect the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : "ceiling");
		return 1;
	}
	if(!(mtmp = bhit(u.dx,u.dy,COLNO,INVIS_BEAM,
					(int(*)())0,(int(*)())0,obj)) ||
	   !haseyes(mtmp->data))
		return 1;

	vis = canseemon(mtmp);
	mlet = mtmp->data->mlet;
	if(mtmp->msleep) {
		if (vis)
		    pline ("%s is too tired to look at your mirror.",
			    Monnam(mtmp));
	} else if (!mtmp->mcansee) {
	    if (vis)
		pline("%s can't see anything right now.", Monnam(mtmp));
	/* some monsters do special things */
	} else if (mlet == S_VAMPIRE || mlet == S_GHOST) {
	    if (vis)
		pline ("%s doesn't have a reflection.", Monnam(mtmp));
	} else if(!mtmp->mcan && mtmp->data == &mons[PM_MEDUSA]) {
		if (vis)
			pline("%s is turned to stone!", Monnam(mtmp));
		stoned = TRUE;
		killed(mtmp);
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_FLOATING_EYE]) {
		int tmp = d((int)mtmp->m_lev, (int)mtmp->data->mattk[0].damd);
		if (!rn2(4)) tmp = 120;
	/* Note: floating eyes cannot use their abilities while invisible,
	 * but Medusa and umber hulks can.
	 */
		if (vis)
			pline("%s is frozen by its reflection.", Monnam(mtmp));
		else You("hear something stop moving.");
		mtmp->mcanmove = 0;
		if ( (int) mtmp->mfrozen + tmp > 127)
			mtmp->mfrozen = 127;
		else mtmp->mfrozen += tmp;
	} else if(!mtmp->mcan && mtmp->data == &mons[PM_UMBER_HULK]) {
		if (vis)
			pline ("%s confuses itself!", Monnam(mtmp));
		mtmp->mconf = 1;
	} else if(!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH
				     || mtmp->data==&mons[PM_SUCCUBUS])) {
		if (vis) {
		    pline ("%s admires herself in your mirror.", Monnam(mtmp));
		    pline ("She takes it!");
		} else pline ("It steals your mirror!");
		setnotworn(obj); /* in case mirror was wielded */
		freeinv(obj);
		mpickobj(mtmp,obj);
		rloc(mtmp);
	} else if (mlet != S_UNICORN && !humanoid(mtmp->data) &&
			(!mtmp->minvis || perceives(mtmp->data)) && rn2(5)) {
		if (vis)
			pline ("%s is frightened by its reflection.",
				Monnam(mtmp));
		mtmp->mflee = 1;
		mtmp->mfleetim += d(2,4);
	} else if (!Blind) {
		if (mtmp->minvis && !See_invisible)
		    ;
		else if ((mtmp->minvis && !perceives(mtmp->data))
			 || !haseyes(mtmp->data))
		    pline("%s doesn't seem to notice its reflection.",
			Monnam(mtmp));
		else
		    pline("%s ignores %s reflection.",
			  Monnam(mtmp), his[pronoun_gender(mtmp)]);
	}
	return 1;
}

static void
use_bell(obj)
register struct obj *obj;
{
	You("ring %s.", the(xname(obj)));

	if(Underwater) {
#ifdef	AMIGA
	    amii_speaker( obj, "AwDwGwEwDhEhAqDqFwGw", AMII_MUFFLED_VOLUME );
#endif
	    pline("But the sound is muffled.");
	    return;
	}
	if(obj->otyp == BELL) {
	    if(u.uswallow) {
		pline(nothing_happens);
		return;
	    }
#ifdef	AMIGA
	    amii_speaker( obj, "awdwgwewdhehaqdqfwgw", AMII_MUFFLED_VOLUME );
#endif
	    if(obj->cursed && !rn2(3)) {
		register struct monst *mtmp;

		if ((mtmp = makemon(&mons[PM_WOOD_NYMPH], u.ux, u.uy)) != 0)
		   You("summon %s!", a_monnam(mtmp));
	    }
	    wake_nearby();
	    return;
	}

	/* bell of opening */
	if(u.uswallow && !obj->blessed) {
	    pline(nothing_happens);
	    return;
	}
	if(obj->cursed) {
	    coord mm;
	    mm.x = u.ux;
	    mm.y = u.uy;
	    mkundead(&mm);
cursed_bell:
	    wake_nearby();
	    if(obj->spe > 0) obj->spe--;
	    return;
	}
	if(invocation_pos(u.ux, u.uy) &&
		     !On_stairs(u.ux, u.uy) && !u.uswallow) {
	    pline("%s issues an unsettling shrill sound...", The(xname(obj)));
#ifdef	AMIGA
	    amii_speaker( obj, "aefeaefeaefeaefeaefe", AMII_LOUDER_VOLUME );
#endif
	    obj->age = moves;
	    if(obj->spe > 0) obj->spe--;
	    wake_nearby();
	    obj->known = 1;
	    return;
	}
	if(obj->blessed) {
	    if(obj->spe > 0) {
		register int cnt = openit();
		if(cnt == -1) return; /* was swallowed */
#ifdef	AMIGA
		amii_speaker( obj, "awawawDwEwCw", AMII_SOFT_VOLUME );
#endif
		switch(cnt) {
		  case 0:  pline(nothing_happens); break;
		  case 1:  pline("Something opens..."); break;
		  default: pline("Things open around you..."); break;
		}
		if(cnt > 0) obj->known = 1;
		obj->spe--;
	    } else pline(nothing_happens);
	} else {  /* uncursed */
#ifdef	AMIGA
	    amii_speaker( obj, "AeFeaeFeAefegW", AMII_OKAY_VOLUME );
#endif
	    if(obj->spe > 0) {
		register int cnt = findit();
		if(cnt == 0) pline(nothing_happens);
		else obj->known = 1;
		obj->spe--;
	    } else {
		if(!rn2(3)) goto cursed_bell;
		else pline(nothing_happens);
	    }
	}
}

static void
use_candelabrum(obj)
register struct obj *obj;
{
	if(Underwater) {
		You("cannot make fire under water.");
		return;
	}
	if(obj->lamplit) {
		You("snuff the candle%s.", obj->spe > 1 ? "s" : "");
		obj->lamplit = 0;
		check_lamps();
		return;
	}
	if(obj->spe <= 0) {
		pline("This %s has no candles.", xname(obj));
		return;
	}
	if(u.uswallow || obj->cursed) {
		pline("The candle%s flicker%s for a moment, then die%s.",
			obj->spe > 1 ? "s" : "",
			obj->spe > 1 ? "" : "s",
			obj->spe > 1 ? "" : "s");
		return;
	}
	if(obj->spe < 7) {
		pline("There %s only %d candle%s in %s.",
		       obj->spe == 1 ? "is" : "are",
		       obj->spe,
		       obj->spe > 1 ? "s" : "",
		       the(xname(obj)));
		if (!Blind)
		    pline("%s lit.  %s shines dimly.",
		       obj->spe == 1 ? "It is" : "They are", The(xname(obj)));
	} else {
		pline("%s's candles burn%s", The(xname(obj)),
			(Blind ? "." : " brightly!"));
	}
	if (!invocation_pos(u.ux, u.uy)) {
		pline("The candle%s being rapidly consumed!",
			(obj->spe > 1 ? "s are" : " is"));
		obj->age /= 2;
	} else {
		if(obj->spe == 7) {
		    if (Blind)
		      pline("%s radiates a strange warmth!", The(xname(obj)));
		    else
		      pline("%s glows with a strange light!", The(xname(obj)));
		}
		obj->known = 1;
	}
	obj->lamplit = 1;
	check_lamps();
}

static void
use_candle(obj)
register struct obj *obj;
{

	register struct obj *otmp;
	char qbuf[QBUFSZ];

	if(obj->lamplit) {
		use_lamp(obj);
		return;
	}

	if(u.uswallow) {
		You("don't have enough elbow-room to maneuver.");
		return;
	}
	if(Underwater) {
		pline("Sorry, fire and water don't mix.");
		return;
	}

	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if(otmp->otyp == CANDELABRUM_OF_INVOCATION && otmp->spe < 7)
			break;
	}
	if(!otmp || otmp->spe == 7) {
		use_lamp(obj);
		return;
	}

	Sprintf(qbuf, "Attach %s", the(xname(obj)));
	Sprintf(eos(qbuf), " to %s?", the(xname(otmp)));
	if(yn(qbuf) == 'n') {
		You("try to light %s...", the(xname(obj)));
		use_lamp(obj);
		return;
	} else {
		register long needed = 7L - (long)otmp->spe;

		You("attach %ld%s candle%s to %s.",
			obj->quan >= needed ? needed : obj->quan,
			!otmp->spe ? "" : " more",
			(needed > 1L && obj->quan > 1L) ? "s" : "",
			the(xname(otmp)));
		if(otmp->lamplit)
			pline("The new candle%s magically ignite%s!",
			    (needed > 1L && obj->quan > 1L) ? "s" : "",
			    (needed > 1L && obj->quan > 1L) ? "" : "s");
		if(obj->unpaid)
			verbalize("You burn %s, you bought %s!",
			    (needed > 1L && obj->quan > 1L) ? "them" : "it",
			    (needed > 1L && obj->quan > 1L) ? "them" : "it");
		if(!otmp->spe || otmp->age > obj->age)
			otmp->age = obj->age;
		if(obj->quan > needed) {
		    if(obj->unpaid) {
			/* this is a hack, until we re-write the billing */
			/* code to accommodate such cases directly. IM*/
			register long delta = obj->quan - needed;

			subfrombill(obj, shop_keeper(*u.ushops));
			obj->quan = needed;
			addtobill(obj, TRUE, FALSE, TRUE);
			bill_dummy_object(obj);
			obj->quan = delta;
			addtobill(obj, TRUE, FALSE, TRUE);
		     } else {
			obj->quan -= needed;
		     }
		     otmp->spe += (int)needed;
		} else {
		    otmp->spe += (int)obj->quan;
		    freeinv(obj);
		    obfree(obj, (struct obj *)0);
		}
		if(needed < 7L && otmp->spe == 7)
		    pline("%s has now seven%s candles attached.",
			The(xname(otmp)), otmp->lamplit ? " lit" : "");
	}
}

boolean
snuff_candle(otmp)  /* call in drop, throw, and put in box, etc. */
register struct obj *otmp;
{
	register boolean candle = Is_candle(otmp);

	if ((candle || otmp->otyp == CANDELABRUM_OF_INVOCATION) &&
		otmp->lamplit) {
	    register boolean many = candle ? otmp->quan > 1L : otmp->spe > 1;
	    if (!Blind)
		pline("The %scandle%s flame%s extinguished.",
		      (candle ? "" : "candelabrum's "),
		      (many ? "s'" : "'s"), (many ? "s are" : " is"));
	   otmp->lamplit = 0;
	   check_lamps();
	   return(TRUE);
	}
	return(FALSE);
}

boolean
snuff_lit(obj)
struct obj *obj;
{
	if(obj->lamplit) {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN) {
			if (!Blind) Your("lamp is now off.");
			obj->lamplit = 0;
			check_lamps();
			return(TRUE);
		}

		if(snuff_candle(obj)) return(TRUE);
	}

	return(FALSE);
}

static void
use_lamp(obj)
struct obj *obj;
{
	if(Underwater) {
		pline("This is not a diving lamp.");
		return;
	}
	if(obj->lamplit) {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN)
		    Your("lamp is now off.");
		else
		    You("snuff out %s.", the(xname(obj)));
		obj->lamplit = 0;
		check_lamps();
		return;
	}
	if (!Is_candle(obj) && obj->spe <= 0) {
		if (obj->otyp == BRASS_LANTERN)
			Your("lamp has run out of power.");
		else pline("This %s has no oil.", xname(obj));
		return;
	}
	if(obj->cursed && !rn2(2))
		pline("%s flicker%s for a moment, then die%s.",
		       The(xname(obj)),
		       obj->quan > 1L ? "" : "s",
		       obj->quan > 1L ? "" : "s");
	else {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN)
		    Your("lamp is now on.");
		else
		    pline("%s%s flame%s burn%s%s", The(xname(obj)),
			obj->quan > 1L ? "'" : "'s",
			obj->quan > 1L ? "s" : "",
			obj->quan > 1L ? "" : "s",
			Blind ? "." : " brightly!");
		obj->lamplit = 1;
		check_lamps();
		if (obj->unpaid && Is_candle(obj) &&
			obj->age == 20L * (long)objects[obj->otyp].oc_cost) {
		    const char *it_them = obj->quan > 1L ? "them" : "it";
		    verbalize("You burn %s, you bought %s!", it_them, it_them);
		    bill_dummy_object(obj);
		}
	}
}

void
check_lamps()
{
	register struct obj *obj;
	int lamps = 0;

	for(obj = invent; obj; obj = obj->nobj)
		if (obj->lamplit) {
			lamps++;
			break;
		}

	if (lamps && u.nv_range == 1) {
		u.nv_range = 3;
		vision_full_recalc = 1;
	} else if (!lamps && u.nv_range == 3) {
		u.nv_range = 1;
		vision_full_recalc = 1;
	}
}

static NEARDATA const char cuddly[] = { TOOL_CLASS, 0 };

int
dorub()
{
	struct obj *obj = getobj(cuddly, "rub");

	if(!obj || (obj != uwep && !wield_tool(obj))) return 0;

	/* now uwep is obj */
	if (uwep->otyp == MAGIC_LAMP) {
	    if (uwep->spe > 0 && !rn2(3)) {
		djinni_from_bottle(uwep);
		makeknown(MAGIC_LAMP);
		uwep->otyp = OIL_LAMP;
		uwep->spe = 1; /* for safety */
		uwep->age = rn1(500,1000);
	    } else if (rn2(2) && !Blind)
		You("see a puff of smoke.");
	    else pline(nothing_happens);
	} else if (obj->otyp == BRASS_LANTERN) {
	    /* message from Adventure */
	    pline("Rubbing the electric lamp is not particularly rewarding.");
	    pline("Anyway, nothing exciting happens.");
	} else pline(nothing_happens);
	return 1;
}

int
dojump()
{
	coord cc;
	register struct monst *mtmp;
	if (!Jumping || Levitation) {
		You("can't jump very far.");
		return 0;
	} else if (u.uswallow) {
		pline("You've got to be kidding!");
		return 0;
	} else if (u.uinwater) {
		pline("This calls for swimming, not jumping!");
		return 0;
	} else if (u.ustuck) {
		You("cannot escape from %s!", mon_nam(u.ustuck));
		return 0;
	} else if (near_capacity() > UNENCUMBERED) {
		You("are carrying too much to jump!");
		return 0;
	} else if (u.uhunger <= 100 || ACURR(A_STR) < 6) {
		You("lack the strength to jump!");
		return 0;
	}
	pline("Where do you want to jump?");
	cc.x = u.ux;
	cc.y = u.uy;
	getpos(&cc, TRUE, "the desired position");
	if(cc.x == -10) return 0; /* user pressed esc */
	if (!(Jumping & ~INTRINSIC) && distu(cc.x, cc.y) != 5) {
		pline("Illegal move!");
		return 0;
	} else if (distu(cc.x, cc.y) > 9) {
		pline("Too far!");
		return 0;
	} else if (!cansee(cc.x, cc.y)) {
		You("cannot see where to land!");
		return 0;
	} else if ((mtmp = m_at(cc.x, cc.y)) != 0) {
		You("cannot trample %s!", mon_nam(mtmp));
		return 0;
	} else if (!isok(cc.x, cc.y) ||
		   ((IS_ROCK(levl[cc.x][cc.y].typ) ||
		     sobj_at(BOULDER, cc.x, cc.y) || closed_door(cc.x, cc.y))
#ifdef POLYSELF
		    && !(passes_walls(uasmon) && may_passwall(cc.x, cc.y))
#endif
		    )) {
			You("cannot jump there!");
			return 0;
	} else {
	    if(u.utrap)
		switch(u.utraptype) {
		case TT_BEARTRAP: {
		    register long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
		    You("rip yourself free of the bear trap!  Ouch!");
		    losehp(rnd(10), "jumping out of a bear trap", KILLED_BY);
		    set_wounded_legs(side, rn1(1000,500));
		    break;
		  }
		case TT_PIT:
		    You("leap from the pit!");
		    break;
		case TT_WEB:
		    You("tear the web apart as you pull yourself free!");
		    deltrap(t_at(u.ux,u.uy));
		    break;
		case TT_LAVA:
		    You("pull yourself above the lava!");
		    u.utrap = 0;
		    return 1;
		case TT_INFLOOR:
		    You("strain your %s, but you're still stuck in the floor.",
			makeplural(body_part(LEG)));
		    set_wounded_legs(LEFT_SIDE, rn1(10, 11));
		    set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
		    return 1;
		}

	    teleds(cc.x, cc.y);
	    nomul(-1);
	    nomovemsg = "";
	    morehungry(rnd(25));
	    return 1;
	}
}

static void
use_tinning_kit(obj)
register struct obj *obj;
{
	register struct obj *corpse, *can;

	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (!(corpse = floorfood("tin", 1))) return;
	if (corpse->oeaten) {
		You("cannot tin something which is partly eaten.");
		return;
	}
	if ((corpse->corpsenm == PM_COCKATRICE)
#ifdef POLYSELF
		&& !resists_ston(uasmon)
#endif
		&& !uarmg) {
pline("Tinning a cockatrice without wearing gloves is a fatal mistake...");
#if defined(POLYSELF)
/* this will have to change if more monsters can poly */
		if(!(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM)))
#endif
	    {
		You("turn to stone...");
		killer_format = KILLED_BY;
		killer = "trying to tin a cockatrice without gloves";
		done(STONING);
	    }
	}
	if (is_rider(&mons[corpse->corpsenm])) {
		revive_corpse(corpse, 0, FALSE);
		verbalize("Yes...  But War does not preserve its enemies...");
		return;
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
		pline("That's too insubstantial to tin.");
		return;
	}
	if ((can = mksobj(TIN, FALSE, FALSE)) != 0) {
	    can->corpsenm = corpse->corpsenm;
	    can->cursed = obj->cursed;
	    can->blessed = obj->blessed;
	    can->owt = weight(can);
	    can->known = 1;
	    can->spe = -1;  /* Mark tinned tins. No spinach allowed... */
	    if (carried(corpse)) {
		if(corpse->unpaid) {
		    verbalize("You tin it, you bought it!");
		    bill_dummy_object(corpse);
		}
		useup(corpse);
	    } else {
		if(costly_spot(corpse->ox, corpse->oy) &&
		      !corpse->no_charge) {
		    verbalize("You tin it, you bought it!");
		    bill_dummy_object(corpse);
		}
		useupf(corpse);
	    }
	    can = hold_another_object(can, "You make, but cannot pick up, %s.",
				      doname(can), (const char *)0);
	} else impossible("Tinning failed.");
}

void
use_unicorn_horn(obj)
struct obj *obj;
{
	boolean blessed = (obj && obj->blessed);
	boolean did_something = FALSE;

	if (obj && obj->cursed) {
		switch (rn2(6)) {
		    static char buf[BUFSZ];
		    case 0: make_sick(Sick ? Sick/4 + 1L : (long) rn1(ACURR(A_CON), 20), TRUE);
			    Strcpy(buf, xname(obj));
			    u.usick_cause = (const char *)buf;
			    break;
		    case 1: make_blinded(Blinded + (long) rnd(100), TRUE);
			    break;
		    case 2: if (!Confusion)
				You("suddenly feel %s.",
					Hallucination ? "trippy" : "confused");
			    make_confused(HConfusion + (long) rnd(100), TRUE);
			    break;
		    case 3: make_stunned(HStun + (long) rnd(100), TRUE);
			    break;
		    case 4: (void) adjattrib(rn2(6), -1, FALSE);
			    break;
		    case 5: make_hallucinated(HHallucination + (long) rnd(100),
				TRUE, 0L);
			    break;
		}
		return;
	}

	if (Sick) {
		make_sick(0L, TRUE);
		did_something++;
	}
	if (Blinded > (long)(u.ucreamed+1) && (!did_something || blessed)) {
		make_blinded(u.ucreamed ? (long)(u.ucreamed+1) : 0L, TRUE);
		did_something++;
	}
	if (Hallucination && (!did_something || blessed)) {
		make_hallucinated(0L, TRUE, 0L);
		did_something++;
	}
	if (Vomiting && (!did_something || blessed)) {
		make_vomiting(0L, TRUE);
		did_something++;
	}
	if (HConfusion && (!did_something || blessed)) {
		make_confused(0L, TRUE);
		did_something++;
	}
	if (HStun && (!did_something || blessed)) {
		make_stunned(0L, TRUE);
		did_something++;
	}
	if (!did_something || blessed) {
		register int j;
		int did_stat = 0;
		int i = rn2(A_MAX);
		for(j=0; j<A_MAX; j++) {
			/* don't recover strength lost while hungry */
			if ((blessed || j==i) &&
				((j != A_STR || u.uhs < WEAK)
				? (ABASE(j) < AMAX(j))
				: (ABASE(A_STR) < (AMAX(A_STR) - 1)))) {
				did_something++;
				/* They may have to use it several times... */
				if (!did_stat) {
					did_stat++;
					pline("This makes you feel good!");
				}
				ABASE(j)++;
				flags.botl = 1;
			}
		}
	}
	if (!did_something) pline(nothing_happens);
}

static void
use_figurine(obj)
register struct obj *obj;
{
	xchar x, y;

	if(!getdir(NULL)) {
		flags.move = multi = 0;
		return;
	}
	x = u.ux + u.dx; y = u.uy + u.dy;
	if (!isok(x,y)) {
		You("cannot put the figurine there.");
		return;
	}
	if (IS_ROCK(levl[x][y].typ) &&
	    !(passes_walls(&mons[obj->corpsenm]) && may_passwall(x,y))) {
		You("cannot place a figurine in solid rock!");
		return;
	}
	if (sobj_at(BOULDER,x,y) && !passes_walls(&mons[obj->corpsenm])
			&& !throws_rocks(&mons[obj->corpsenm])) {
		You("cannot fit the figurine on the boulder.");
		return;
	}
	You("%s and it transforms.",
	    (u.dx||u.dy) ? "set the figurine beside you" :
	    (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) ?
		"release the figurine" :
	    (u.dz < 0 ?
		"toss the figurine into the air" :
		"set the figurine on the ground"));
	make_familiar(obj, u.ux+u.dx, u.uy+u.dy);
	useup(obj);
}

static NEARDATA const char lubricables[] = { ALL_CLASSES, ALLOW_NONE, 0 };

static void
use_grease(obj)
struct obj *obj;
{
	struct obj *otmp;

	if (Glib) {
	    dropx(obj);
	    pline("%s slips from your %s.", The(xname(obj)),
		  makeplural(body_part(FINGER)));
	    return;
	}

	if (obj->spe > 0) {
		if ((obj->cursed || Fumbling) && !rn2(2)) {
			obj->spe--;
			check_unpaid(obj);
			dropx(obj);
			pline("%s slips from your %s.", The(xname(obj)),
			      makeplural(body_part(FINGER)));
			return;
		}
		otmp = getobj(lubricables, "grease");
		if (!otmp) return;
		obj->spe--;
		check_unpaid(obj);
		if (otmp != &zeroobj) {
			You("cover your %s with a thick layer of grease.",
			    xname(otmp));
			otmp->greased = 1;
		} else {
			Glib += rnd(15);
			You("coat your %s with grease.",
			    makeplural(body_part(FINGER)));
		}
	} else {
		pline("%s %s empty.", The(xname(obj)),
			obj->known ? "is" : "seems to be");
	}
}

int
doapply()
{
	register struct obj *obj;
	register int res = 1;

	if(check_capacity(NULL)) return (0);
	obj = getobj(tools, "use or apply");
	if(!obj) return 0;

	check_unpaid(obj);

	switch(obj->otyp){
	case BLINDFOLD:
		if (obj == ublindf) {
		    if(cursed(obj)) break;
		    else Blindf_off(obj);
		}
		else if (!ublindf) Blindf_on(obj);
		else You("are already %s.", ublindf->otyp == TOWEL ?
			 "covered by a towel" : "wearing a blindfold");
		break;
	case LARGE_BOX:
	case CHEST:
	case ICE_BOX:
	case SACK:
	case BAG_OF_HOLDING:
	case OILSKIN_SACK:
		res = use_container(obj, 1);
		break;
	case BAG_OF_TRICKS:
		if(obj->spe > 0) {
			register int cnt = 1;

			obj->spe--;
			check_unpaid(obj);
			if(!rn2(23)) cnt += rn2(7) + 1;
			while(cnt--)
			    (void) makemon((struct permonst *) 0, u.ux, u.uy);
			makeknown(BAG_OF_TRICKS);
		} else
			pline(nothing_happens);
		break;
	case CAN_OF_GREASE:
		use_grease(obj);
		break;
	case LOCK_PICK:
#ifdef TOURIST
	case CREDIT_CARD:
#endif
	case SKELETON_KEY:
		(void) pick_lock(obj);
		break;
	case PICK_AXE:
		res = use_pick_axe(obj);
		break;
	case TINNING_KIT:
		use_tinning_kit(obj);
		break;
#ifdef WALKIES
	case LEASH:
		use_leash(obj);
		break;
#endif
	case MAGIC_WHISTLE:
		use_magic_whistle(obj);
		break;
	case TIN_WHISTLE:
		use_whistle(obj);
		break;
	case STETHOSCOPE:
		res = 0;
		use_stethoscope(obj);
		break;
	case MIRROR:
		res = use_mirror(obj);
		break;
	case BELL:
	case BELL_OF_OPENING:
		use_bell(obj);
		break;
	case CANDELABRUM_OF_INVOCATION:
		use_candelabrum(obj);
		break;
	case WAX_CANDLE:
	case TALLOW_CANDLE:
		use_candle(obj);
		break;
	case OIL_LAMP:
	case MAGIC_LAMP:
	case BRASS_LANTERN:
		use_lamp(obj);
		break;
#ifdef TOURIST
	case EXPENSIVE_CAMERA:
		res = use_camera(obj);
		break;
#endif
	case TOWEL:
		res = use_towel(obj);
		break;
	case CRYSTAL_BALL:
		use_crystal_ball(obj);
		break;
	case MAGIC_MARKER:
		res = dowrite(obj);
		break;
	case TIN_OPENER:
		if(!carrying(TIN)) {
			You("have no tin to open.");
			goto xit;
		}
		You("cannot open a tin without eating or discarding its contents.");
		if(flags.verbose)
			pline("In order to eat, use the 'e' command.");
		if(obj != uwep)
    pline("Opening the tin will be much easier if you wield the tin opener.");
		goto xit;

	case FIGURINE:
		use_figurine(obj);
		break;
	case UNICORN_HORN:
		use_unicorn_horn(obj);
		break;
	case WOODEN_FLUTE:
	case MAGIC_FLUTE:
	case TOOLED_HORN:
	case FROST_HORN:
	case FIRE_HORN:
	case WOODEN_HARP:
	case MAGIC_HARP:
	case BUGLE:
	case LEATHER_DRUM:
	case DRUM_OF_EARTHQUAKE:
		res = do_play_instrument(obj);
		break;
	case HORN_OF_PLENTY:	/* not a musical instrument */
		if (obj->spe > 0) {
		    struct obj *otmp;
		    const char *what;

		    obj->spe--;
		    check_unpaid(obj);
		    if (!rn2(13)) {
			otmp = mkobj(POTION_CLASS, FALSE);
			if (objects[otmp->otyp].oc_magic) do {
			    otmp->otyp = rnd_class(POT_BOOZE, POT_WATER);
			} while (otmp->otyp == POT_SICKNESS);
			what = "A potion";
		    } else {
			otmp = mkobj(FOOD_CLASS, FALSE);
			if (otmp->otyp == FOOD_RATION && !rn2(7))
			    otmp->otyp = LUMP_OF_ROYAL_JELLY;
			what = "Some food";
		    }
		    pline("%s spills out.", what);
		    otmp->blessed = obj->blessed;
		    otmp->cursed = obj->cursed;
		    otmp->owt = weight(otmp);
		    otmp = hold_another_object(otmp,
					(u.uswallow || Is_airlevel(&u.uz) ||
					 u.uinwater || Is_waterlevel(&u.uz)) ?
					       "Oops!  %s away from you!" :
					       "Oops!  %s to the floor!",
					       The(aobjnam(otmp, "slip")),
					       (const char *)0);
		    makeknown(HORN_OF_PLENTY);
		} else
		    pline(nothing_happens);
		break;
	default:
		pline("Sorry, I don't know how to use that.");
	xit:
		nomul(0);
		return 0;
	}
	nomul(0);
	return res;
}

#endif /* OVLB */

/*apply.c*/
