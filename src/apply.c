/*	SCCS Id: @(#)apply.c	3.0	88/10/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define MONATTK_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
#include	"hack.h"
#include	"edog.h"

#ifdef MUSIC
#define IS_INSTRUMENT(typ)	((typ) >= FLUTE && (typ) <= DRUM_OF_EARTHQUAKE)
#endif /* MUSIC /**/

#ifdef OVLB

static const char NEARDATA tools[] = { TOOL_SYM, 0 };

static boolean NEARDATA did_dig_msg;

static struct monst *FDECL(bchit, (int, int, int, CHAR_P));
static void FDECL(use_camera, (struct obj *));
static void FDECL(use_stethoscope, (struct obj *));
static void FDECL(use_whistle, (struct obj *));
static void FDECL(use_magic_whistle, (struct obj *));
#ifdef WALKIES
static void FDECL(use_leash, (struct obj *));
#endif
STATIC_DCL int NDECL(dig);
static boolean FDECL(wield_tool, (struct obj *));
static int FDECL(use_pick_axe, (struct obj *));
#ifdef MEDUSA
static void FDECL(use_mirror, (struct obj *));
#endif
static void FDECL(use_lamp, (struct obj *));
static void FDECL(use_crystal_ball, (struct obj *));
static void FDECL(use_tinning_kit, (struct obj *));

/* version of bhit for cameras and mirrors */
static
struct monst *
bchit(ddx,ddy,range,sym) register int ddx,ddy,range; char sym; {
	register struct monst *mtmp = (struct monst *) 0;
	register int bchx = u.ux, bchy = u.uy;

	if(sym) {
		Tmp_at2(-1, sym);	/* open call */
#ifdef TEXTCOLOR
		Tmp_at2(-3, WHITE);
#endif
	}
	while(range--) {
		bchx += ddx;
		bchy += ddy;
		if(MON_AT(bchx, bchy)) {
			mtmp = m_at(bchx,bchy);
			break;
		}
		if(!ZAP_POS(levl[bchx][bchy].typ) || closed_door(bchx, bchy)) {
			bchx -= ddx;
			bchy -= ddy;
			break;
		}
		if(sym) Tmp_at2(bchx, bchy);
	}
	if(sym) Tmp_at2(-1, -1);
	return(mtmp);
}

static void
use_camera(obj) /* register */ struct obj *obj; {
register struct monst *mtmp;
	if(!getdir(1)){		/* ask: in what direction? */
		flags.move = multi = 0;
		return;
	}
	if(u.uswallow) {
		You("take a picture of %s's %s.", mon_nam(u.ustuck),
		    is_animal(u.ustuck->data)? "stomach" : "interior");
		return;
	}
	if(obj->cursed && !rn2(2)) goto blindu;
	if(u.dz) {
		You("take a picture of the %s.",
			(u.dz > 0) ? "floor" : "ceiling");
		return;
	}
	if(!u.dx && !u.dy && !u.dz) {
blindu:
		if(!Blind) {
			You("are blinded by the flash!");
			make_blinded((long)rnd(25),FALSE);
		}
		return;
	}
	if(mtmp = bchit(u.dx, u.dy, COLNO, '!')) {
		if(mtmp->msleep){
			mtmp->msleep = 0;
			pline("The flash awakens %s.", mon_nam(mtmp)); /* a3 */
		} else
		if(mtmp->data->mlet != S_YLIGHT)
		if(mtmp->mcansee || mtmp->mblinded){
			register int tmp = dist(mtmp->mx,mtmp->my);
			register int tmp2;
			if(cansee(mtmp->mx,mtmp->my))
			  pline("%s is blinded by the flash!", Monnam(mtmp));
			if(mtmp->data == &mons[PM_GREMLIN]) {
			  /* Rule #1: Keep them out of the light. */
			  kludge("%s cries out in pain!", Monnam(mtmp));
			  if (mtmp->mhp > 1) mtmp->mhp--;
			}
			setmangry(mtmp);
			if(tmp < 9 && !mtmp->isshk && rn2(4)) {
				mtmp->mflee = 1;
				if(rn2(4)) mtmp->mfleetim = rnd(100);
			}
			if(tmp < 3) {
				mtmp->mcansee  = 0;
				mtmp->mblinded = 0;
			} else {
				tmp2 = mtmp->mblinded;
				tmp2 += rnd(1 + 50/tmp);
				if(tmp2 > 127) tmp2 = 127;
				mtmp->mblinded = tmp2;
				mtmp->mcansee = 0;
			}
		}
	}
}

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless. */
static void
use_stethoscope(obj) register struct obj *obj; {
register struct monst *mtmp;
register struct rm *lev;
register int rx, ry;
	if(!freehand()) {
		You("have no free %s!", body_part(HAND));
		return;
	}
	if (!getdir(1)) {
		flags.move=multi=0;
		return;
	}
	if(u.dz < 0 || (u.dz && Levitation)) {
		You("can't reach the %s!", u.dz<0 ? "ceiling" : "floor");
		return;
	}
	if(obj->cursed && !rn2(2)) {
		You("hear your heart beat.");
		return;
	}
	if(u.dz) {
#ifdef STRONGHOLD
		if (dlevel == stronghold_level)
			You("hear the crackling of hellfire.");
		else
#endif
			pline("The floor seems healthy enough.");
		return;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy && !u.dz) {
		ustatusline();
		return;
	}
	rx = u.ux + u.dx; ry = u.uy + u.dy;
	if(u.uswallow) {
		mstatusline(u.ustuck);
		return;
	}
	if (!isok(rx,ry)) {
		You("hear a faint typing noise.");
		return;
	}
	lev = &levl[rx][ry];
	if(MON_AT(rx, ry)) {
		mtmp = m_at(rx,ry);
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx,ry)) pmon(mtmp);
		}
		return;
	}
	if(lev->typ == SDOOR) {
		You("hear a hollow sound!  This must be a secret door!");
		lev->typ = DOOR;
		lev->seen = 0;		/* force prl */
		prl(rx,ry);
		return;
	}
	if(lev->typ == SCORR) {
		You("hear a hollow sound!  This must be a secret passage!");
		lev->typ = CORR;
		lev->seen = 0;		/* force prl */
		prl(rx,ry);
		return;
	}
	You("hear nothing special.");
}

/* ARGSUSED */
static void
use_whistle(obj)
struct obj *obj; {
	You("produce a high whistling sound.");
	wake_nearby();
}

static void
use_magic_whistle(obj)
struct obj *obj; {
	register struct monst *mtmp = fmon;

	if(obj->cursed && !rn2(2)) {
		You("produce a high-pitched humming noise.");
		wake_nearby();
	} else {
		You("produce a %s whistling sound.", Hallucination
			? "normal" : "strange");
		while(mtmp) {
			if(mtmp->mtame) mnexto(mtmp);
			mtmp = mtmp->nmon;
		}
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
o_unleash(otmp) 	/* otmp is about to be destroyed or stolen */
register struct obj *otmp;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->m_id == (unsigned)otmp->leashmon)
			mtmp->mleashed = 0;
	otmp->leashmon = 0;
}

void
m_unleash(mtmp) 	/* mtmp is about to die, or become untame */
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

	if(!obj->leashmon && number_leashed() >= MAXLEASHED) {
		You("can't leash additional pets.");
		return;
	}

	if(!getdir(1)) return;

	x = u.ux + u.dx;
	y = u.uy + u.dy;

	if((x == u.ux) && (y == u.uy)) {
		pline("Leash yourself?  Very funny...");
		return;
	}

	if(!MON_AT(x, y)) {
		pline("There is no creature here.");
		return;
	}

	mtmp = m_at(x, y);

	if(!mtmp->mtame) {
		pline("%s is not %s!", Monnam(mtmp), (!obj->leashmon) ?
				"leashable" : "leashed");
		return;
	}
	if(!obj->leashmon) {
		if(mtmp->mleashed) {
			pline("This %s is already leashed!", lmonnam(mtmp)+4);
			return;
		}
		You("slip the leash around your %s.", lmonnam(mtmp)+4);
		mtmp->mleashed = 1;
		obj->leashmon = (int)mtmp->m_id;
		if(mtmp->msleep)  mtmp->msleep = 0;
		return;
	}
	if(obj->leashmon != (int)mtmp->m_id) {
		pline("This leash is not attached to that creature!");
		return;
	} else {
		if(obj->cursed) {
			pline("The leash wouldn't come off!");
			return;
		}
		mtmp->mleashed = 0;
		obj->leashmon = 0;
		You("remove the leash from your %s.",
		/* a hack to include the dogs full name.  +4 eliminates */
		/* the 'the' at the start of the name */
				 lmonnam(mtmp)+4);
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
			if(dist(mtmp->mx,mtmp->my) > 2) mnexto(mtmp);
			if(dist(mtmp->mx,mtmp->my) > 2) {
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
get_mleash(mtmp) 	/* assuming mtmp->mleashed has been checked */
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
				pline("%s's leash snaps loose!",Monnam(mtmp));
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

STATIC_OVL int
dig() {
	register struct rm *lev;
	register int dpx = dig_pos.x, dpy = dig_pos.y;

	lev = &levl[dpx][dpy];
	/* perhaps a nymph stole his pick-axe while he was busy digging */
	/* or perhaps he teleported away */
	if(u.uswallow || !uwep || uwep->otyp != PICK_AXE ||
	    dig_level != dlevel ||
	    ((dig_down && (dpx != u.ux || dpy != u.uy)) ||
	     (!dig_down && dist(dpx,dpy) > 2)))
		return(0);

	if(dig_down && is_maze_lev) {
		pline("The floor here is too hard to dig in.");
		return(0);
	}
	if(!dig_down && IS_ROCK(lev->typ) && !may_dig(dpx,dpy)) {
		pline("This wall is too hard to dig into.");
		return(0);
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
		case 1:  pline("Bang!  You hit with the broad side of the %s!",
			 xname(uwep)); break;
		default: Your("swing misses its mark."); 
			 break;
		}
		return(0);
	}
	dig_effort += 10 + abon() + uwep->spe + rn2(5);
	if(dig_down) {
		if(dig_effort > 250) {
			dighole();
			dig_level = -1;
			return(0);	/* done with digging */
		}
		if(dig_effort > 50) {
			register struct trap *ttmp = t_at(dpx,dpy);

			if(!ttmp) {
				ttmp = maketrap(dpx,dpy,PIT);
				ttmp->tseen = 1;
				if(Invisible) newsym(ttmp->tx,ttmp->ty);
				You("have dug a pit.");
				u.utrap = rn1(4,2);
				u.utraptype = TT_PIT;
				dig_level = -1;
				return(0);
			}
		}
	} else
	if(dig_effort > 100) {
		register const char *digtxt;
		register struct obj *obj;

		if(obj = sobj_at(STATUE, dpx, dpy)) {
			if (break_statue(obj))
				digtxt = "The statue shatters.";
			else
				/* it was a statue trap; break_statue()
				 * printed a message and updated the screen
				 */
				digtxt = NULL;
		} else if(obj = sobj_at(BOULDER, dpx, dpy)) {
			fracture_rock(obj);
			digtxt = "The boulder falls apart.";
		} else if(!lev->typ || lev->typ == SCORR) {
			lev->typ = CORR;
			digtxt = "You succeeded in cutting away some rock.";
		} else if(IS_WALL(lev->typ)) {
#ifdef STUPID
		        if (is_maze_lev)
			    lev->typ = ROOM;
			else
			    lev->typ = DOOR;
#else
			lev->typ = is_maze_lev ? ROOM : DOOR;
#endif
			digtxt = "You just made an opening in the wall.";
		} else if(lev->typ == SDOOR) {
			lev->typ = DOOR;
			digtxt = "You just broke through a secret door.";
			if(!(lev->doormask & D_TRAPPED))
				lev->doormask = D_BROKEN;
		} else if(closed_door(dpx, dpy)) {
			digtxt = "You just broke a hole through the door.";
			if(!(lev->doormask & D_TRAPPED))
				lev->doormask = D_BROKEN;
		} else return(0); /* statue or boulder got taken */
		mnewsym(dpx, dpy);
		prl(dpx, dpy);
		if (digtxt) pline(digtxt);	/* after mnewsym & prl */
		if(IS_DOOR(lev->typ) && (lev->doormask & D_TRAPPED)) {
			b_trapped("door");
			lev->doormask = D_NODOOR;
			mnewsym(dpx, dpy);
			prl(dpx, dpy);
		}
		dig_level = -1;
		return(0);
	} else {
		if(IS_WALL(lev->typ) || closed_door(dpx, dpy)) {
		    register int rno = inroom(dpx,dpy);

		    if(rno >= 0 && rooms[rno].rtype >= SHOPBASE) {
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
	if(occupation != dig || !in_shop(u.ux, u.uy)) return(-1);
	return((250 - dig_effort)/20);
}

void
dighole()
{
	register struct trap *ttmp = t_at(u.ux, u.uy);

	if(is_maze_lev
#ifdef ENDGAME
			|| dlevel == ENDLEVEL
#endif
						) {
		pline("The floor here seems too hard to dig in.");
	} else {
		if(IS_FURNITURE(levl[u.ux][u.uy].typ)) {
#if defined(ALTARS) && defined(THEOLOGY)
	            if(IS_ALTAR(levl[u.ux][u.uy].typ)) {
		    	altar_wrath(u.ux, u.uy);
			if(in_temple(u.ux, u.uy)) angry_priest();
		    }
#endif
		    levl[u.ux][u.uy].typ = ROOM;
		    levl[u.ux][u.uy].altarmask = 0;
		}
		if(ttmp)
			ttmp->ttyp = TRAPDOOR;
		else
			ttmp = maketrap(u.ux, u.uy, TRAPDOOR);
		ttmp->tseen = 1;
		if(Invisible) newsym(ttmp->tx,ttmp->ty);
		pline("You've made a hole in the floor.");
		if(!u.ustuck && !Levitation) {			/* KAA */
			if(in_shop(u.ux, u.uy))
				shopdig(1);
#ifdef WALKIES
			if(!next_to_u())
			    You("are jerked back by your pet!");
			else {
#endif
			    You("fall through...");
			    if(u.utraptype == TT_PIT) {
				u.utrap = 0;
				u.utraptype = 0;
			    }
			    unsee();
#ifdef MACOS
			    segments |= SEG_APPLY;
#endif
			    goto_level(dlevel+1, FALSE, TRUE);
#ifdef WALKIES
			}
#endif
		}
	}
}

static boolean
wield_tool(obj)
struct obj *obj;
{
	if(uwep && uwep->cursed) {
		/* Andreas Bormann - ihnp4!decvax!mcvax!unido!ab */
		if(flags.verbose) {
			pline("Since your weapon is welded to your %s,",
				bimanual(uwep) ?
				makeplural(body_part(HAND))
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
	register char *dsp = dirsyms;
	register const char *sdp = flags.num_pad ? ndir : sdir;
	register struct rm *lev;
	register int rx, ry, res = 0;
	register boolean isclosedoor = FALSE;

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
	pline("In what direction do you want to dig? [%s] ", dirsyms);
	if(!getdir(0))		/* no txt */
		return(res);
	if(u.uswallow && attack(u.ustuck)) /* return(1) */;
	else if(u.dz < 0) You("cannot reach the ceiling.");
	else if(!u.dx && !u.dy && !u.dz) {
		char buf[BUFSZ];
		int dam;

		dam = rnd(2) + dbon() + obj->spe;
		if (dam <= 0) dam = 1;
		You("hit yourself with your own pick-axe.");
		/* self_pronoun() won't work twice in a sentence */
		Strcpy(buf, self_pronoun("killed %sself with %%s own pick-axe",
			"him"));
		losehp(dam, self_pronoun(buf, "his"), NO_KILLER_PREFIX);
		flags.botl=1;
		return(1);
	} else if(u.dz == 0) {
		if(Stunned || (Confusion && !rn2(5))) confdir();
		rx = u.ux + u.dx;
		ry = u.uy + u.dy;
		lev = &levl[rx][ry];
		if(MON_AT(rx, ry) && attack(m_at(rx, ry)))
			return(1);
		if(!isok(rx, ry)) {
			pline("Clash!");
			return(1);
		}
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
			    || dig_level != dlevel || dig_down) {
				dig_down = FALSE;
				dig_pos.x = rx;
				dig_pos.y = ry;
				dig_level = dlevel;
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
	} else if(Levitation) {
		You("cannot reach the floor.");
	} else {
		if(dig_pos.x != u.ux || dig_pos.y != u.uy
		    || dig_level != dlevel || !dig_down) {
			dig_down = TRUE;
			dig_pos.x = u.ux;
			dig_pos.y = u.uy;
			dig_level = dlevel;
			dig_effort = 0;
			You("start digging in the floor.");
			if(in_shop(u.ux, u.uy))
				shopdig(0);
		} else
			You("continue digging in the floor.");
		did_dig_msg = FALSE;
		set_occupation(dig, "digging", 0);
	}
	return(1);
}

#define WEAK	3	/* from eat.c */

#ifdef MEDUSA
static void
use_mirror(obj)
struct obj *obj;
{
	register struct monst *mtmp;
	register char mlet;

	if(!getdir(1)){		/* ask: in what direction? */
		flags.move = multi = 0;
		return;
	}
	if(obj->cursed && !rn2(2)) {
		if (!Blind)
			pline("The mirror gets foggy and doesn't reflect!");
		return;
	}
	if(!u.dx && !u.dy && !u.dz) {
		if(!Blind && !Invisible) {
#ifdef POLYSELF
		    if(u.umonnum == PM_FLOATING_EYE) {
			pline("Yikes!  You've frozen yourself!");
			nomul(-rnd((MAXULEV+6) - (int)u.ulevel));
		    } else if (u.usym == S_VAMPIRE || u.usym == S_DEMON)
			You("don't seem to reflect anything.");
		    else if(u.umonnum == PM_UMBER_HULK) {
			pline("Huh?  That doesn't look like you!");
			make_confused(HConfusion + d(3,4),FALSE);
		    } else
#endif
			   if (Hallucination) You("look %s.", hcolor());
		    else if (Sick)
			You("look peaked.");
		    else if (u.uhs >= WEAK)
			You("look undernourished.");
#ifdef POLYSELF
		    else if (u.usym == S_NYMPH
#ifdef INFERNO
			     || u.umonnum==PM_SUCCUBUS
#endif
			     )
			You("look beautiful in the mirror.");
#ifdef INFERNO
		    else if (u.umonnum == PM_INCUBUS)
			You("look handsome in the mirror.");
#endif
#endif
		    else You("look as %s as ever.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly");
		} else {
		if (Luck <= 10 && rn2(4-Luck/3) || !HTelepat ||
		    (u.ukilled_medusa
#ifdef HARD
			&& u.udemigod
#endif
		    )) {
			You("can't see your %s %s.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly",
				body_part(FACE));
		} else {
			static char buf[35];
			const char *tm, *tl; int ll;
			if (!u.ukilled_medusa && rn2(4)) {
			    tm = "n ugly snake-headed monster";
			    ll = dlevel - medusa_level;
			}
			else {
			    tm = " powerful wizard";
			    ll = dlevel - wiz_level;
			}
			if (ll < -10) tl = "far below you";
			else if (ll < -1) tl = "below you";
			else if (ll == -1) {
			    Sprintf(buf, "under your %s", makeplural(
				body_part(FOOT)));
			    tl = buf;
			} else if (ll == 0)  tl = "very close to you";
			else if (ll == 1) {
			    Sprintf(buf, "above your %s", body_part(HEAD));
			    tl = buf;
			} else if (ll > 10) tl = "far above you";
			else tl = "above you";
			You("get an impression that a%s lives %s.",
				tm, tl);
		    }
		}
		return;
	}
	if(u.uswallow) {
		You("reflect %s's %s.", mon_nam(u.ustuck), 
		    is_animal(u.ustuck->data)? "stomach" : "interior");
		return;
	}
	if(u.dz) {
		You("reflect the %s.",
			(u.dz > 0) ? "floor" : "ceiling");
		return;
	}
	if(!(mtmp = bchit(u.dx, u.dy, COLNO, 0)) || !haseyes(mtmp->data))
		return;

	mlet = mtmp->data->mlet;
	if(mtmp->msleep) {
		if(!Blind)
		    pline ("%s is tired and doesn't look at your mirror.",
			    Monnam(mtmp));
		mtmp->msleep = 0;
	} else if (!mtmp->mcansee) {
		if (!Blind)
		    pline("%s can't see anything at the moment.", Monnam(mtmp));
	/* some monsters do special things */
	} else if (mlet == S_VAMPIRE || mlet == S_DEMON || mlet == S_GHOST ||
		  (mtmp->minvis && !perceives(mtmp->data) && !See_invisible)) {
		if (!Blind)
		   pline ("%s doesn't seem to reflect anything.", Monnam(mtmp));
	} else if(!mtmp->mcan && mtmp->data == &mons[PM_MEDUSA]) {
		if (!Blind)
			pline("%s is turned to stone!", Monnam(mtmp));
		stoned = TRUE;
		killed(mtmp);
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_FLOATING_EYE]) {
		int tmp = d((int)mtmp->m_lev, (int)mtmp->data->mattk[0].damd);
		if (!rn2(4)) tmp = 120;
	/* Note: floating eyes cannot use their abilities while invisible,
	 * but medusas and umber hulks can.
	 */
		if (!Blind)
			pline("%s is frozen by its reflection.",Monnam(mtmp));
		mtmp->mcanmove = 0;
		if (mtmp->mfrozen + tmp > 127)
			mtmp->mfrozen = 127;
		else mtmp->mfrozen += tmp;
	} else if(!mtmp->mcan && mtmp->data == &mons[PM_UMBER_HULK]) {
		if (!Blind)
			pline ("%s has confused itself!", Monnam(mtmp));
	    	mtmp->mconf = 1;
	} else if(!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH
#ifdef INFERNO
			  || mtmp->data==&mons[PM_SUCCUBUS]
#endif
			  )) {
		if (!Blind) {
	    	    pline ("%s looks beautiful in your mirror.",Monnam(mtmp));
	    	    pline ("She decides to take it!");
		} else pline ("It steals your mirror!");
		setnotworn(obj); /* in case mirror was wielded */
	    	freeinv(obj);
	    	mpickobj(mtmp,obj);
	    	rloc(mtmp);
	} else if (mlet != S_UNICORN && !humanoid(mtmp->data) && 
			(!mtmp->minvis || perceives(mtmp->data)) && rn2(5)) {
		if (!Blind)
			pline ("%s is frightened by its reflection%s.",
				Monnam(mtmp), (mtmp->minvis && !See_invisible
					&& !Telepat) ?
				", though you see nothing" : "");
		mtmp->mflee = 1;
		mtmp->mfleetim += d(2,4);
	} else if (!Blind) {
		if (mtmp->minvis && !See_invisible)
		    pline("%s doesn't seem to reflect anything.",
			Monnam(mtmp));
		else if (mtmp->minvis && !perceives(mtmp->data))
		    pline("%s doesn't seem to be aware of its reflection.",
			Monnam(mtmp));
		else
		    pline("%s doesn't seem to mind %s reflection.",
			Monnam(mtmp), (is_female(mtmp) ? "her" :
		        is_human(mtmp->data) ? "his" : "its"));
	}
}/* use_mirror */

#endif

static void
use_lamp(obj)
struct obj *obj;
{
	if(obj->spe <= 0 || obj->otyp == MAGIC_LAMP ) {
		pline("This lamp has no oil.");
		return;
	}
	if(obj->cursed && !rn2(2))
		pline("The lamp flickers on for a moment and dies.");
	else litroom(TRUE);
	obj->spe -= 1;
}

static void
use_crystal_ball(obj)
	struct obj *obj;
{
	char buf[BUFSZ];
	int oops, ret;

	if (Blind) {
		pline("Too bad you can't see the crystal ball.");
		return;
	}
	oops = (rnd(20) > ACURR(A_INT) || obj->cursed);
	if (oops && (obj->spe > 0)) {
		switch(rnd(5)) {
		case 1 : pline("The crystal ball is too much to comprehend!");
			break;
		case 2 : pline("The crystal ball confuses you!");
			make_confused(HConfusion + rnd(100),FALSE);
			break;
		case 3 : pline("The crystal ball damages your vision!");
			make_blinded(Blinded + rnd(100),FALSE);
			break;
		case 4 : pline("The crystal ball zaps your mind!");
			make_hallucinated(Hallucination + rnd(100),FALSE);
			break;
		case 5 : pline("The crystal ball explodes!");
			useup(obj);
			losehp(rnd(30), "exploding crystal ball",
				KILLED_BY_AN);
			break;
		}
		obj->spe -= 1;
		return;
	}

	pline("What do you want to look for? ");
	getlin(buf);
	clrlin();
	if (!buf[0] || buf[0] == '\033') {
		if(flags.verbose) pline("Never mind.");
		return;
		}
	You("peer into the crystal ball.");
	nomul(-rnd(10));
	nomovemsg = "";
	if(obj->spe <= 0)
		pline("The vision is unclear.");
	else {
	  	obj->spe -= 1;
		switch(buf[0]) {
		case GOLD_SYM :	ret = gold_detect((struct obj *)0);
			break;
		case '^' :	ret = trap_detect((struct obj *)0);
			break;
		case FOOD_SYM :	ret = food_detect((struct obj *)0);
			break;
		case POTION_SYM :
		case GEM_SYM :
		case TOOL_SYM :
		case WEAPON_SYM :
		case WAND_SYM :
		case SCROLL_SYM :
#ifdef SPELLS
		case SPBOOK_SYM :
#endif
		case ARMOR_SYM :	ret = object_detect((struct obj *)0);
			break;
		default  : lcase(buf);
			if (!strncmp(buf,"gold",4) || !strncmp(buf,"money",5))
				ret = gold_detect((struct obj *)0);
			else if (!strncmp(buf,"trap",4))
				ret = trap_detect((struct obj *)0);
			else if (!strncmp(buf,"food",4) ||
				 !strncmp(buf,"dead",4) ||
				 !strncmp(buf,"corpse",6))
				ret = food_detect((struct obj *)0);
			else if (!strncmp(buf,"obj",3) ||
				 !strncmp(buf,"the",3) ||
				 !strncmp(buf,"a ",2) ||
				 !strncmp(buf,"an ",3))
				 /* || strstr(buf, " of") */
				ret = object_detect((struct obj *)0);
			else ret = monster_detect((struct obj *)0);
			break;
		}
		if (ret)
		    if (!rn2(100))  /* make them nervous */
			You("see the Wizard of Yendor gazing out at you.");
		    else pline("The vision is unclear.");
	}
	return;
}

static const char NEARDATA cuddly[] = { TOOL_SYM, 0 };

int
dorub()
{
	struct obj *obj = getobj(cuddly, "rub");

	if(!obj || (obj != uwep && !wield_tool(obj))) return 0;

	/* now uwep is obj */
	if (uwep->otyp == MAGIC_LAMP) {
	    if (uwep->spe > 0 && !rn2(3)) {
		uwep->spe = 0;
		djinni_from_bottle(uwep);
		makeknown(MAGIC_LAMP);
	    } else if (rn2(2) && !Blind)
		You("see a puff of smoke.");
	    else pline(nothing_happens);
	} else pline(nothing_happens);
	return 1;
}

int
dojump()
{
	coord cc;
	register struct monst *mtmp;
	if (!Jumping) {
		You("can't jump very far.");
		return 0;
	} else if (u.uswallow) {
		pline("You've got to be kidding!");
		return 0;
	} else if (u.ustuck) {
		kludge("You cannot escape from %s!",
			mon_nam(u.ustuck));
		return 0;
	} else if (inv_weight() > -5) {
		You("are carrying too much to jump!");
		return 0;
	} else if (u.uhunger <= 100 || ACURR(A_STR) < 6) {
		You("lack the strength to jump!");
		return 0;
	}
	pline("Where do you want to jump?");
	cc.x = u.ux;
	cc.y = u.uy;
	getpos(&cc, 1, "the desired position");
	if (dist(cc.x, cc.y) > 9) {
		pline("Too far!");
		return 0;
	} else if (!cansee(cc.x, cc.y)) {
		You("cannot see where to land!");
		return 0;
	} else if (MON_AT(cc.x, cc.y)) {
		mtmp = m_at(cc.x, cc.y);
		You("cannot trample %s!", mon_nam(mtmp));
		return 0;
	} else if (!isok(cc.x, cc.y) ||
#ifdef POLYSELF
		(IS_ROCK(levl[cc.x][cc.y].typ) && !passes_walls(uasmon)) ||
#else
		IS_ROCK(levl[cc.x][cc.y].typ) ||
#endif
		sobj_at(BOULDER, cc.x, cc.x) ) {
			You("cannot jump there!");
			return 0;
	} else {
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
	register struct obj *corpse, *can = (struct obj *)0;

	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (!(corpse = floorfood("can", 1))) return;
	if (corpse->oeaten) {
		You("cannot tin something which is partly eaten.");
		return;
	}
	if ((corpse->corpsenm == PM_COCKATRICE)
#ifdef POLYSELF
		&& !resists_ston(uasmon)
#endif
		&& !uarmg) {
pline("Tinning a cockatrice corpse without gloves was not a very wise move...");
		You("turn to stone...");
		killer_format = KILLED_BY;
		killer = "trying to tin a cockatrice without gloves";
		done(STONING);
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
		You("can't tin something that insubstantial!");
		return;
	}
	if(can = mksobj(TIN,FALSE)) {
	    can->corpsenm = corpse->corpsenm;
	    can->quan = 1; /*Defeat the occasional creation of pairs of tins */
	    can->owt = weight(can);
	    can->known = 1;
	    can->spe = -1; /* Mark tinned tins. No spinach allowed... */
	    can->cursed = obj->cursed;
	    can->blessed = obj->blessed;
	    can = addinv(can);
	    You("now have %s.", doname(can));
	    if (carried(corpse)) useup(corpse);
	    else useupf(corpse);
	} else pline("Tinning failed.");
}

int
use_unicorn_horn(obj)
struct obj *obj;
{
	boolean blessed = (obj && obj->blessed);
	boolean did_something = FALSE;

	if (obj && obj->cursed) {
		switch (rn2(6)) {
		    static char buf[BUFSZ];
		    case 0: make_sick(Sick ? 1L : (long)(20 + rn2(20)), TRUE);
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
		    case 4: adjattrib(rn2(6), -1, FALSE);
			    break;
		    case 5: make_hallucinated(Hallucination + (long) rnd(100),
				TRUE);
			    break;
		}
		return 1;
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
		make_hallucinated(0L, TRUE);
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
			if ((blessed || j==i) && ABASE(i) < AMAX(i)) {
				did_something++;
				/* They may have to use it several times... */
				if (!did_stat) {
					did_stat++;
					pline("This makes you feel good!");
				}
				ABASE(i)++;
				flags.botl = 1;
			}
		}
	}
	if (!did_something) pline(nothing_happens);
	return !!did_something;
}

int
doapply() {
	register struct obj *obj;
	register int res = 1;

	obj = getobj(tools, "use or apply");
	if(!obj) return 0;

	check_unpaid(obj);

#ifdef MUSIC
	if (IS_INSTRUMENT(obj->otyp)) {
		res = do_play_instrument(obj);
		return (res);
	}
#endif /* MUSIC /**/

	switch(obj->otyp){
	case EXPENSIVE_CAMERA:
		use_camera(obj); break;
	case CREDIT_CARD:
	case LOCK_PICK:
	case SKELETON_KEY:
	case KEY:
		(void) pick_lock(obj);
		break;
	case BAG_OF_TRICKS:
		if(obj->spe > 0) {
			register int cnt = 1;

			obj->spe -= 1;
			if(!rn2(23)) cnt += rn2(7) + 1;
			while(cnt--)
			    (void) makemon((struct permonst *) 0, u.ux, u.uy);
			makeknown(BAG_OF_TRICKS);
		}
		break;
	case LARGE_BOX:
	case CHEST:
	case ICE_BOX:
	case SACK:
	case BAG_OF_HOLDING:
		use_container(obj, 1); break;
	case PICK_AXE:
		res = use_pick_axe(obj);
		break;
	case TINNING_KIT:
		use_tinning_kit(obj);
		break;
	case MAGIC_WHISTLE:
		if(pl_character[0] == 'W' || u.ulevel > (MAXULEV/3)) {
			use_magic_whistle(obj);
			break;
		}
		/* fall into next case */
	case WHISTLE:
		use_whistle(obj);
		break;
#ifdef MEDUSA
	case MIRROR:
		use_mirror(obj);
		break;
#endif
	case LAMP:
	case MAGIC_LAMP:
		use_lamp(obj);
		break;
	case CRYSTAL_BALL:
		use_crystal_ball(obj);
		break;
#ifdef WALKIES
	case LEASH:
		use_leash(obj);
		break;
#endif
	case MAGIC_MARKER:
		dowrite(obj);
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

	case STETHOSCOPE:
		res = 0;
		use_stethoscope(obj);
		break;
	case FIGURINE:
		You("set the figurine on the ground and it transforms.");
		make_familiar(obj);
		useup(obj);
		break;
	case BLINDFOLD:
		if (obj == ublindf) {
		    if(cursed(obj)) break;
		    else Blindf_off(obj);
		} 
		else if (!ublindf) Blindf_on(obj);
		else You("are already wearing a blindfold!");
		break;
	case UNICORN_HORN:
		res = use_unicorn_horn(obj);
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
