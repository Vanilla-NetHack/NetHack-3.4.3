/*	SCCS Id: @(#)apply.c	3.2	96/01/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

#ifdef OVLB

static const char tools[] = { TOOL_CLASS, POTION_CLASS,
				WEAPON_CLASS, WAND_CLASS, 0 };

#ifdef TOURIST
static int FDECL(use_camera, (struct obj *));
#endif
static int FDECL(use_towel, (struct obj *));
static boolean FDECL(its_dead, (int,int,int *));
static int FDECL(use_stethoscope, (struct obj *));
static void FDECL(use_whistle, (struct obj *));
static void FDECL(use_magic_whistle, (struct obj *));
static void FDECL(use_leash, (struct obj *));
static int FDECL(use_mirror, (struct obj *));
static void FDECL(use_bell, (struct obj *));
static void FDECL(use_candelabrum, (struct obj *));
static void FDECL(use_candle, (struct obj *));
static void FDECL(use_lamp, (struct obj *));
static void FDECL(light_cocktail, (struct obj *));
static void FDECL(use_tinning_kit, (struct obj *));
static void FDECL(use_figurine, (struct obj *));
static void FDECL(use_grease, (struct obj *));
static void FDECL(use_trap, (struct obj *));
static int FDECL(use_whip, (struct obj *));
static int FDECL(do_break_wand, (struct obj *));

#ifdef	AMIGA
void FDECL( amii_speaker, ( struct obj *, char *, int ) );
#endif

static char no_elbow_room[] = "don't have enough elbow-room to maneuver.";

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
	if(!getdir((char *)0)) return(0);

	if (obj->cursed && !rn2(2)) {
		(void) zapyourself(obj, TRUE);
	} else if (u.uswallow) {
		You("take a picture of %s %s.", s_suffix(mon_nam(u.ustuck)),
		    is_animal(u.ustuck->data) ? "stomach" : "interior");
	} else if (u.dz) {
		You("take a picture of the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
	} else if (!u.dx && !u.dy) {
		(void) zapyourself(obj, TRUE);
	} else if ((mtmp = bhit(u.dx,u.dy,COLNO,FLASHED_LIGHT,
				(int(*)())0,(int(*)())0,obj)) != 0) {
		obj->ox = u.ux,  obj->oy = u.uy;
		(void) flash_hits_mon(mtmp, obj);
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

/* maybe give a stethoscope message based on floor objects */
static boolean
its_dead(rx, ry, resp)
int rx, ry, *resp;
{
	struct obj *otmp;
	struct trap *ttmp;

	/* additional stethoscope messages from jyoung@apanix.apana.org.au */
	if (Hallucination && sobj_at(CORPSE, rx, ry)) {
	    /* (a corpse doesn't retain the monster's sex,
	       so we're forced to use generic pronoun here) */
	    You_hear("a voice say, \"It's dead, Jim.\"");
	    *resp = 1;
	    return TRUE;
	} else if (Role_is('H') && ((otmp = sobj_at(CORPSE, rx, ry)) != 0 ||
				    (otmp = sobj_at(STATUE, rx, ry)) != 0)) {
	    /* possibly should check uppermost {corpse,statue} in the pile
	       if both types are present, but it's not worth the effort */
	    if (vobj_at(rx, ry)->otyp == STATUE) otmp = vobj_at(rx, ry);
	    if (otmp->otyp == CORPSE) {
		You("determine that %s unfortunate being is dead.",
		    (rx == u.ux && ry == u.uy) ? "this" : "that");
	    } else {
		ttmp = t_at(rx, ry);
		pline("%s appears to be in %s health for a statue.",
		      The(mons[otmp->corpsenm].mname),
		      (ttmp && ttmp->ttyp == STATUE_TRAP) ?
			"extraordinary" : "excellent");
	    }
	    return TRUE;
	}
	return FALSE;
}

static char hollow_str[] = "a hollow sound.  This must be a secret %s!";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
static int
use_stethoscope(obj)
	register struct obj *obj;
{
	static long last_used = 0;
	struct monst *mtmp;
	struct rm *lev;
	int rx, ry, res;

	if (nohands(uasmon)) {	/* should also check for no ears and/or deaf */
		You("have no hands!");	/* not `body_part(HAND)' */
		return 0;
	} else if (!freehand()) {
		You("have no free %s.", body_part(HAND));
		return 0;
	}
	if (!getdir((char *)0)) return 0;

	res = (moves + monstermoves == last_used);
	last_used = moves + monstermoves;

	if (u.uswallow && (u.dx || u.dy || u.dz)) {
		mstatusline(u.ustuck);
		return res;
	} else if (u.dz) {
		if (Underwater)
		    You_hear("faint splashing.");
		else if (u.dz < 0 || !can_reach_floor())
		    You_cant("reach the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		else if (its_dead(u.ux, u.uy, &res))
		    ;	/* message already given */
		else if (Is_stronghold(&u.uz))
		    You_hear("the crackling of hellfire.");
		else
		    pline_The("%s seems healthy enough.", surface(u.ux,u.uy));
		return res;
	} else if (obj->cursed && !rn2(2)) {
		You_hear("your heart beat.");
		return res;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy) {
		ustatusline();
		return res;
	}
	rx = u.ux + u.dx; ry = u.uy + u.dy;
	if (!isok(rx,ry)) {
		You_hear("a faint typing noise.");
		return 0;
	}
	if ((mtmp = m_at(rx,ry)) != 0) {
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx,ry)) newsym(mtmp->my,mtmp->my);
		}
		return res;
	}

	lev = &levl[rx][ry];
	switch(lev->typ) {
	case SDOOR:
		You_hear(hollow_str, "door");
		lev->typ = DOOR;
		lev->doormask = exposed_sdoor_mask(lev);
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	case SCORR:
		You_hear(hollow_str, "passage");
		lev->typ = CORR;
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	}

	if (!its_dead(rx, ry, &res))
		You_hear("nothing special.");
	return res;
}

static char whistle_str[] = "produce a %s whistling sound.";

static void
use_whistle(obj)
struct obj *obj;
{
	You(whistle_str, obj->cursed ? "shrill" : "high");
	wake_nearby();
}

static void
use_magic_whistle(obj)
struct obj *obj;
{
	register struct monst *mtmp, *nextmon;

	if(obj->cursed && !rn2(2)) {
		You("produce a high-pitched humming noise.");
		wake_nearby();
	} else {
		int pet_cnt = 0;
		You(whistle_str, Hallucination ? "normal" : "strange");
		for(mtmp = fmon; mtmp; mtmp = nextmon) {
		    nextmon = mtmp->nmon; /* trap might kill mon */
		    if (mtmp->mtame) {
			mnexto(mtmp);
			if (canspotmon(mtmp)) ++pet_cnt;
			/* No longer in previous trap.  Necessary since */
			/* mintrap acts differently for already-trapped mons */
			mtmp->mtrapped = 0;
			(void) mintrap(mtmp);
		    }
		}
		if (pet_cnt > 0) makeknown(MAGIC_WHISTLE);
	}
}

boolean
um_dist(x,y,n)
register xchar x, y, n;
{
	return((boolean)(abs(u.ux - x) > n  || abs(u.uy - y) > n));
}

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

#define MAXLEASHED	2

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

	if(!getdir((char *)0)) return;

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

	spotmon = canspotmon(mtmp);

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
			pline_The("leash would not come off!");
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
				    You_feel("%s leash go slack.",
					(number_leashed() > 1) ? "a" : "the");
				    mtmp->mleashed = 0;
				    otmp->leashmon = 0;
				}
			}
		}
	return(TRUE);
}

#endif /* OVL1 */
#ifdef OVL0

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
				    if (mtmp->data->msound != MS_SILENT)
					switch(rn2(3)) {
					    case 0:  growl(mtmp);	break;
					    case 1:  yelp(mtmp);	break;
					    default: whimper(mtmp); break;
					}
				}
			    }
			}
		    }
		    mtmp = mtmp->nmon;
		}
	    }
}

#endif /* OVL0 */
#ifdef OVLB

boolean
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
	if (cantwield(uasmon)) {
		You_cant("hold it strongly enough.");
		return(FALSE);
	}
	You("now wield %s.", doname(obj));
	setuwep(obj);
	if (uwep != obj) return(FALSE); /* rewielded old object after dying */
	unweapon = TRUE;
	return(TRUE);
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

	if(!getdir((char *)0)) return 0;
	if(obj->cursed && !rn2(2)) {
		if (!Blind)
			pline_The("mirror fogs up and doesn't reflect!");
		return 1;
	}
	if(!u.dx && !u.dy && !u.dz) {
		if(!Blind && !Invisible) {
		    if (u.umonnum == PM_FLOATING_EYE) {
			pline(Hallucination ?
			      "Yow!  The mirror stares back!" :
			      "Yikes!  You've frozen yourself!");
			nomul(-rnd((MAXULEV+6) - u.ulevel));
		    } else if (u.usym == S_VAMPIRE)
			You("don't have a reflection.");
		    else if (u.umonnum == PM_UMBER_HULK) {
			pline("Huh?  That doesn't look like you!");
			make_confused(HConfusion + d(3,4),FALSE);
		    } else if (Hallucination)
			You(look_str, hcolor((char *)0));
		    else if (Sick)
			You(look_str, "peaked");
		    else if (u.uhs >= WEAK)
			You(look_str, "undernourished");
		    else You("look as %s as ever.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly");
		} else {
			You_cant("see your %s %s.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly",
				body_part(FACE));
		}
		return 1;
	}
	if(u.uswallow) {
		if (!Blind) You("reflect %s %s.", s_suffix(mon_nam(u.ustuck)),
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
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
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
		else You_hear("%s stop moving.",something);
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
	struct monst *mtmp;
	boolean wakem = FALSE, learno = FALSE,
		ordinary = (obj->otyp != BELL_OF_OPENING || !obj->spe),
		invoking = (obj->otyp == BELL_OF_OPENING &&
			 invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy));

	You("ring %s.", the(xname(obj)));

	if (Underwater || (u.uswallow && ordinary)) {
#ifdef	AMIGA
	    amii_speaker( obj, "AhDhGqEqDhEhAqDqFhGw", AMII_MUFFLED_VOLUME );
#endif
	    pline("But the sound is muffled.");

	} else if (invoking && ordinary) {
	    /* needs to be recharged... */
	    pline("But it makes no sound.");
	    learno = TRUE;	/* help player figure out why */

	} else if (ordinary) {
#ifdef	AMIGA
	    amii_speaker( obj, "ahdhgqeqdhehaqdqfhgw", AMII_MUFFLED_VOLUME );
#endif
	    if (obj->cursed && !rn2(4) &&
		    /* note: once any of them are gone, we stop all of them */
		    !(mvitals[PM_WOOD_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_MOUNTAIN_NYMPH].mvflags & G_GONE) &&
		    (mtmp = makemon(mkclass(S_NYMPH, 0), u.ux, u.uy)) != 0) {
		You("summon %s!", a_monnam(mtmp));
		discard_minvent(mtmp);	/* treat this like reverse genocide */
		if (!obj_resists(obj, 93, 100)) {
		    pline("%s has shattered!", The(xname(obj)));
		    useup(obj);
		} else switch (rn2(3)) {
			default:
				break;
			case 1: mtmp->mspeed = MFAST;
				break;
			case 2: /* no explanation; it just happens... */
				nomovemsg = "";
				nomul(-rnd(2));
				break;
		}
	    }
	    wakem = TRUE;

	} else {
	    /* charged Bell of Opening */
	    check_unpaid(obj);
	    obj->spe--;

	    if (u.uswallow) {
		if (!obj->cursed)
		    (void) openit();
		else
		    pline(nothing_happens);

	    } else if (obj->cursed) {
		coord mm;

		mm.x = u.ux;
		mm.y = u.uy;
		mkundead(&mm);
		wakem = TRUE;

	    } else  if (invoking) {
		pline("%s issues an unsettling shrill sound...",
		      The(xname(obj)));
#ifdef	AMIGA
		amii_speaker( obj, "aefeaefeaefeaefeaefe", AMII_LOUDER_VOLUME );
#endif
		obj->age = moves;
		learno = TRUE;
		wakem = TRUE;

	    } else if (obj->blessed) {
#ifdef	AMIGA
		amii_speaker( obj, "ahahahDhEhCw", AMII_SOFT_VOLUME );
#endif
		switch (openit()) {
		  case 0:  pline(nothing_happens); break;
		  case 1:  pline("%s opens...", Something);
			   learno = TRUE; break;
		  default: pline("Things open around you...");
			   learno = TRUE; break;
		}

	    } else {  /* uncursed */
#ifdef	AMIGA
		amii_speaker( obj, "AeFeaeFeAefegw", AMII_OKAY_VOLUME );
#endif
		if (findit() != 0) learno = TRUE;
		else pline(nothing_happens);
	    }

	}	/* charged BofO */

	if (learno) {
	    makeknown(BELL_OF_OPENING);
	    obj->known = 1;
	}
	if (wakem) wake_nearby();
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
		end_burn(obj, TRUE);
		return;
	}
	if(obj->spe <= 0) {
		pline("This %s has no candles.", xname(obj));
		return;
	}
	if(u.uswallow || obj->cursed) {
		pline_The("candle%s flicker%s for a moment, then die%s.",
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
		pline_The("candle%s being rapidly consumed!",
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
	begin_burn(obj, FALSE);
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
		You(no_elbow_room);
		return;
	}
	if(Underwater) {
		pline("Sorry, fire and water don't mix.");
		return;
	}

	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if (otmp->otyp == CANDELABRUM_OF_INVOCATION)
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
			pline_The("new candle%s magically ignite%s!",
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
		    pline("%s now has seven%s candles attached.",
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
	    char buf[BUFSZ];
	    xchar x, y;
	    register boolean many = candle ? otmp->quan > 1L : otmp->spe > 1;

	    (void) get_obj_location(otmp, &x, &y, 0);
	    if (otmp->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		pline("%s %scandle%s flame%s extinguished.",
		      Shk_Your(buf, otmp),
		      (candle ? "" : "candelabrum's "),
		      (many ? "s'" : "'s"), (many ? "s are" : " is"));
	   end_burn(otmp, TRUE);
	   return(TRUE);
	}
	return(FALSE);
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
boolean
snuff_lit(obj)
struct obj *obj;
{
	char buf[BUFSZ];
	xchar x, y;

	if (obj->lamplit) {
	    if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		    obj->otyp == BRASS_LANTERN || obj->otyp == POT_OIL) {
		(void) get_obj_location(obj, &x, &y, 0);
		if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		    pline("%s %s goes out!", Shk_Your(buf, obj), xname(obj));
		end_burn(obj, TRUE);
		return TRUE;
	    }
	    if (snuff_candle(obj)) return TRUE;
	}
	return FALSE;
}

static void
use_lamp(obj)
struct obj *obj;
{
	char buf[BUFSZ];

	if(Underwater) {
		pline("This is not a diving lamp.");
		return;
	}
	if(obj->lamplit) {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN)
		    pline("%s lamp is now off.", Shk_Your(buf, obj));
		else
		    You("snuff out %s %s.", shk_your(buf, obj), xname(obj));
		end_burn(obj, TRUE);
		return;
	}
	/* magic lamps with an spe == 0 (wished for) cannot be lit */
	if ((!Is_candle(obj) && obj->age == 0)
			|| (obj->otyp == MAGIC_LAMP && obj->spe == 0)) {
		if (obj->otyp == BRASS_LANTERN)
			Your("lamp has run out of power.");
		else pline("This %s has no oil.", xname(obj));
		return;
	}
	if (obj->cursed && !rn2(2)) {
		pline("%s flicker%s for a moment, then die%s.",
		       The(xname(obj)),
		       obj->quan > 1L ? "" : "s",
		       obj->quan > 1L ? "" : "s");
	} else {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == BRASS_LANTERN) {
		    check_unpaid(obj);
		    pline("%s lamp is now on.", Shk_Your(buf, obj));
		} else {	/* candle(s) */
		    pline("%s %s flame%s burn%s%s",
			Shk_Your(buf, obj),
			s_suffix(xname(obj)),
			obj->quan > 1L ? "s" : "",
			obj->quan > 1L ? "" : "s",
			Blind ? "." : " brightly!");
		    if (obj->unpaid &&
			  obj->age == 20L * (long)objects[obj->otyp].oc_cost) {
			const char *ithem = obj->quan > 1L ? "them" : "it";
			verbalize("You burn %s, you bought %s!", ithem, ithem);
			bill_dummy_object(obj);
		    }
		}
		begin_burn(obj, FALSE);
	}
}

static void
light_cocktail(obj)
	struct obj *obj;	/* obj is a potion of oil */
{
	char buf[BUFSZ];

	if (u.uswallow) {
	    You(no_elbow_room);
	    return;
	}

	if (obj->lamplit) {
	    You("snuff the lit potion.");
	    end_burn(obj, TRUE);
	    /*
	     * Free & add to re-merge potion.  This will average the
	     * age of the potions.  Not exactly the best solution,
	     * but its easy.
	     */
	    freeinv(obj);
	    (void) addinv(obj);
	    return;
	}

	You("light %s potion.  It gives off a dim light.", shk_your(buf, obj));
	if (obj->unpaid) {
	    check_unpaid(obj);		/* surcharge for use of unpaid item */
	    bill_dummy_object(obj);	/* treat it as having been used up    */
	    obj->no_charge = 1;		/* you're now obligated to pay for it */
	    obj->unpaid = 0;
	}

	if (obj->quan > 1L) {
	    (void) splitobj(obj, 1L);
	    begin_burn(obj, FALSE);	/* burn before free to get position */
	    obj_extract_self(obj);	/* free from inv */

	    /* shouldn't merge */
	    obj = hold_another_object(obj, "You drop %s!",
				      doname(obj), (const char *)0);
	} else
	    begin_burn(obj, FALSE);
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
		check_unpaid(uwep); /* this is not adequate; it simply charges
				       the same amount as lighting the lamp! */
		djinni_from_bottle(uwep);
		makeknown(MAGIC_LAMP);
		uwep->otyp = OIL_LAMP;
		uwep->spe = 0; /* for safety */
		uwep->age = rn1(500,1000);
		if (uwep->lamplit) begin_burn(uwep, TRUE);
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
	struct monst *mtmp;

	if (!Jumping) {
		You_cant("jump very far.");
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
	} else if (Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		You("don't have enough traction to jump.");
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
		    && !(passes_walls(uasmon) && may_passwall(cc.x, cc.y)))) {
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

boolean
tinnable(corpse)
struct obj *corpse;
{
	if (corpse->oeaten) return 0;
	if (!mons[corpse->corpsenm].cnutrit) return 0;
	return 1;
}

static void
use_tinning_kit(obj)
register struct obj *obj;
{
	register struct obj *corpse, *can;

	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (!(corpse = floorfood("tin", 2))) return;
	if (corpse->oeaten) {
		You("cannot tin %s which is partly eaten.",something);
		return;
	}
	if ((corpse->corpsenm == PM_COCKATRICE)
		&& !resists_ston(&youmonst) && !uarmg) {
pline("Tinning a cockatrice without wearing gloves is a fatal mistake...");
		/* this will have to change if more monsters can poly */
		if (!(poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))) {
		    You("turn to stone...");
		    killer_format = KILLED_BY;
		    killer = "trying to tin a cockatrice without gloves";
		    done(STONING);
		}
	}
	if (is_rider(&mons[corpse->corpsenm])) {
		(void) revive_corpse(corpse);
		verbalize("Yes...  But War does not preserve its enemies...");
		return;
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
		pline("That's too insubstantial to tin.");
		return;
	}
	if ((can = mksobj(TIN, FALSE, FALSE)) != 0) {
	    static const char you_buy_it[] = "You tin it, you bought it!";

	    can->corpsenm = corpse->corpsenm;
	    can->cursed = obj->cursed;
	    can->blessed = obj->blessed;
	    can->owt = weight(can);
	    can->known = 1;
	    can->spe = -1;  /* Mark tinned tins. No spinach allowed... */
	    if (carried(corpse)) {
		if (corpse->unpaid)
		    verbalize(you_buy_it);
		useup(corpse);
	    } else {
		if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
		    verbalize(you_buy_it);
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
#define PROP_COUNT 6		/* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX*3)	/* number of attribute points we might fix */
	int idx, val, val_limit,
	    trouble_count, unfixable_trbl, did_prop, did_attr;
	int trouble_list[PROP_COUNT + ATTR_COUNT];

	if (obj && obj->cursed) {
	    long lcount = (long) rnd(100);

	    switch (rn2(6)) {
	    case 0: make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON),20),
			xname(obj), TRUE, SICK_NONVOMITABLE);
		    break;
	    case 1: make_blinded(Blinded + lcount, TRUE);
		    break;
	    case 2: if (!Confusion)
			You("suddenly feel %s.",
			    Hallucination ? "trippy" : "confused");
		    make_confused(HConfusion + lcount, TRUE);
		    break;
	    case 3: make_stunned(HStun + lcount, TRUE);
		    break;
	    case 4: (void) adjattrib(rn2(6), -1, FALSE);
		    break;
	    case 5: make_hallucinated(HHallucination + lcount, TRUE, 0L);
		    break;
	    }
	    return;
	}

/*
 * Entries in the trouble list use a very simple encoding scheme.
 */
#define prop2trbl(X)	((X) + A_MAX)
#define attr2trbl(Y)	(Y)
#define prop_trouble(X) trouble_list[trouble_count++] = prop2trbl(X)
#define attr_trouble(Y) trouble_list[trouble_count++] = attr2trbl(Y)

	trouble_count = unfixable_trbl = did_prop = did_attr = 0;

	/* collect property troubles */
	if (Sick) prop_trouble(SICK);
	if (Blinded > (long)(u.ucreamed + 1)) prop_trouble(BLINDED);
	if (HHallucination) prop_trouble(HALLUC);
	if (Vomiting) prop_trouble(VOMITING);
	if (HConfusion) prop_trouble(CONFUSION);
	if (HStun) prop_trouble(STUNNED);
	/* keep track of unfixed trouble, for message adjustment below
	   (can't "feel great" with these problems present) */
	if (Stoned) unfixable_trbl++;
	if (Strangled) unfixable_trbl++;
	if (Wounded_legs) unfixable_trbl++;

	/* collect attribute troubles */
	for (idx = 0; idx < A_MAX; idx++) {
	    val_limit = AMAX(idx);
	    /* don't recover strength lost from hunger */
	    if (idx == A_STR && u.uhs >= WEAK) val_limit--;
	    /* don't recover more than 3 points worth of any attribute */
	    if (val_limit > ABASE(idx) + 3) val_limit = ABASE(idx) + 3;

	    for (val = ABASE(idx); val < val_limit; val++)
		attr_trouble(idx);
	    /* keep track of unfixed trouble, for message adjustment below */
	    unfixable_trbl += (AMAX(idx) - val_limit);
	}

	if (trouble_count == 0) {
	    pline(nothing_happens);
	    return;
	} else if (trouble_count > 1) {		/* shuffle */
	    int i, j, k;

	    for (i = trouble_count - 1; i > 0; i--)
		if ((j = rn2(i + 1)) != i) {
		    k = trouble_list[j];
		    trouble_list[j] = trouble_list[i];
		    trouble_list[i] = k;
		}
	}

	/*
	 *		Chances for number of troubles to be fixed
	 *		 0	1      2      3      4	    5	   6	  7
	 *   blessed:  16/80  16/80  15/80  13/80  10/80   6/80   3/80	 1/80
	 *  uncursed:	4/12   4/12   3/12   1/12    0	    0	   0	  0
	 */
	val_limit = rn2( d(2, (obj && obj->blessed) ? 4 : 2) );
	if (val_limit > trouble_count) val_limit = trouble_count;

	/* fix [some of] the troubles */
	for (val = 0; val < val_limit; val++) {
	    idx = trouble_list[val];

	    switch (idx) {
	    case prop2trbl(SICK):
		make_sick(0L, (char *) 0, TRUE, SICK_ALL);
		did_prop++;
		break;
	    case prop2trbl(BLINDED):
		make_blinded(u.ucreamed ? (long)(u.ucreamed+1) : 0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(HALLUC):
		make_hallucinated(0L, TRUE, 0L);
		did_prop++;
		break;
	    case prop2trbl(VOMITING):
		make_vomiting(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(CONFUSION):
		make_confused(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(STUNNED):
		make_stunned(0L, TRUE);
		did_prop++;
		break;
	    default:
		if (idx >= 0 && idx < A_MAX) {
		    ABASE(idx) += 1;
		    did_attr++;
		} else
		    panic("use_unicorn_horn: bad trouble? (%d)", idx);
		break;
	    }
	}

	if (did_attr)
	    pline("This makes you feel %s!",
		  (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
		  "great" : "better");
	else if (!did_prop)
	    pline("Nothing seems to happen.");

	flags.botl = (did_attr || did_prop);
#undef PROP_COUNT
#undef ATTR_COUNT
#undef prop2trbl
#undef attr2trbl
#undef prop_trouble
#undef attr_trouble
}

static void
use_figurine(obj)
register struct obj *obj;
{
	xchar x, y;

	if(!getdir((char *)0)) {
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
static NEARDATA const char need_to_remove_outer_armor[] =
			"need to remove your %s to grease your %s.";

static void
use_grease(obj)
struct obj *obj;
{
	struct obj *otmp;
	char buf[BUFSZ];

	if (Glib) {
	    dropx(obj);
	    pline("%s slips from your %s.", The(xname(obj)),
		  makeplural(body_part(FINGER)));
	    return;
	}

	if (obj->spe > 0) {
		if ((obj->cursed || Fumbling) && !rn2(2)) {
			check_unpaid(obj);
			obj->spe--;
			dropx(obj);
			pline("%s slips from your %s.", The(xname(obj)),
			      makeplural(body_part(FINGER)));
			return;
		}
		otmp = getobj(lubricables, "grease");
		if (!otmp) return;
		if ((otmp->owornmask & WORN_ARMOR) && uarmc) {
			Strcpy(buf, xname(uarmc));
			You(need_to_remove_outer_armor, buf, xname(otmp));
			return;
		}
#ifdef TOURIST
		if ((otmp->owornmask & WORN_SHIRT) && (uarmc || uarm)) {
			Strcpy(buf, uarmc ? xname(uarmc) : "");
			if (uarmc && uarm) Strcat(buf, " and ");
			Strcat(buf, uarm ? xname(uarm) : "");
			You(need_to_remove_outer_armor, buf, xname(otmp));
			return;
		}
#endif
		check_unpaid(obj);
		obj->spe--;
		if (otmp != &zeroobj) {
			You("cover %s %s with a thick layer of grease.",
			    shk_your(buf, otmp),
			    xname(otmp));
			otmp->greased = 1;
			if (obj->cursed && !nohands(uasmon)) {
			    Glib += rnd(15);
			    pline("Some of the grease gets all over your %s.",
				makeplural(body_part(HAND)));
			}
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

/* Place a landmine/bear trap.  Helge Hafting */
static void
use_trap(otmp)
struct obj *otmp;
{
	struct trap *ttmp;
	int ttyp;
	const char *what = (char *)0;

	if (u.uswallow)
	    what = is_animal(u.ustuck->data) ? "while swallowed" :
			"while engulfed";
	else if (Underwater)
	    what = "underwater";
	else if (Levitation)
	    what = "while levitating";
	else if (On_stairs(u.ux, u.uy))
	    what = (u.ux == xdnladder || u.ux == xupladder) ?
			"on the ladder" : "on the stairs";
	else if (IS_FURNITURE(levl[u.ux][u.uy].typ) || t_at(u.ux, u.uy))
	    what = "here";
	if (what) {
	    You_cant("set a trap %s!",what);
	    return;
	}
	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	ttmp = maketrap(u.ux, u.uy, ttyp);
	if (ttmp) {
	    ttmp->tseen = 1;
	    ttmp->madeby_u = 1;
	    newsym(u.ux, u.uy); /* if our hero happens to be invisible */
	    You("set and arm %s.",
		an(defsyms[trap_to_defsym(ttyp)].explanation));
	    if ((otmp->cursed || Fumbling) && (rnl(10) > 5)) dotrap(ttmp);
	} else {
	    /* this shouldn't happen */
	    Your("attempt fails.");
	}
	useup(otmp);
}

static int
use_whip(obj)
struct obj *obj;
{
	char buf[BUFSZ];
	struct monst *mtmp;
	register int rx, ry;
	int res = 0;
	int proficient = 0;
	const char *msg_slipsfree = "The bullwhip slips free.";
	const char *msg_snap = "Snap!";
	struct obj *otmp;

	if (obj != uwep) {
	    if (!wield_tool(obj)) return(0);
	    else res = 1;
	    /* prevent bashing msg */
	    unweapon = FALSE;
	}
	if(!getdir((char *)0)) return(res);
	if (Stunned || (Confusion && !rn2(5))) confdir();
	rx = u.ux + u.dx;
	ry = u.uy + u.dy;
	mtmp = m_at(rx, ry);

	/* fake some proficiency checks */
	proficient = 0;
	if (Role_is('A')) ++proficient;
	if (ACURR(A_DEX) < 6) proficient--;
	else if (ACURR(A_DEX) >= 14) proficient += (ACURR(A_DEX) - 14);
	if (Fumbling) --proficient;
	if (proficient > 3) proficient = 3;
	if (proficient < 0) proficient = 0;

	if (u.uswallow && attack(u.ustuck))
		pline("There is not enough room to flick your bullwhip.");
	else if (Underwater)
		pline("There is too much resistance to flick your bullwhip.");
	else if (u.dz < 0)
		You("flick a bug off of the %s.",ceiling(u.ux,u.uy));
	else if((!u.dx && !u.dy) || (u.dz > 0)) {
		int dam;
		if (Levitation) {
			/* Have a shot at snaring something on the floor */
			otmp = level.objects[u.ux][u.uy];
			if (otmp && proficient) {
				You("wrap your bullwhip around %s on the %s.",
					an(singular(otmp,xname)),
					surface(u.ux, u.uy));
				if (!rnl(6))
					if (pickup_object(otmp, 1L, TRUE) > 0)
						return 1;
				pline(msg_slipsfree);
				return 1;
			}
		}
		dam = rnd(2) + dbon() + obj->spe;
		if (dam <= 0) dam = 1;
		You("hit your %s with your bullwhip.", body_part(FOOT));
		/* self_pronoun() won't work twice in a sentence */
		Strcpy(buf, self_pronoun("killed %sself with %%s bullwhip",
			"him"));
		losehp(dam, self_pronoun(buf, "his"), NO_KILLER_PREFIX);
		flags.botl=1;
		return(1);
	} else if ((Fumbling || Glib) && !rn2(5)) {
		pline_The("bullwhip slips out of your %s.",
			body_part(HAND));
		dropx(obj);
		setuwep((struct obj *)0);
	}
	/*
	 *     Assumptions:
	 *
	 *		if you're in a pit
	 *			- you are attempting to get out of the pit
	 *			- or, if you are applying it towards a small
	 *			  monster then it is assumed that you are
	 *			  trying to hit it.
	 *		else if the monster is wielding a weapon
	 *			- you are attempting to disarm a monster
	 *		else
	 *			- you are attempting to hit the monster
	 *
	 *		if you're confused (and thus off the mark)
	 *			- you only end up hitting.
	 *
	 */
	else if(u.utraptype == TT_PIT) {
		const char *wrapped_what = (char *)0;

		if (mtmp) {
			if (bigmonst(mtmp->data)) {
				Strcpy(buf, mon_nam(mtmp));
				wrapped_what = buf;
			} else if (proficient)
				if (attack(mtmp)) return(1);
				else pline(msg_snap);
		}
		if (!wrapped_what) {
			if (IS_FURNITURE(levl[rx][ry].typ))
				wrapped_what = something;
			else if (sobj_at(BOULDER, rx, ry))
				wrapped_what = "a boulder";
		}
		if (wrapped_what) {
			coord cc;

			cc.x = rx; cc.y = ry;
			You("wrap your bullwhip around %s.", wrapped_what);
			if (proficient && rn2(proficient + 2)) {
				if (!mtmp || enexto(&cc, rx, ry, &playermon)) {
					You("yank yourself out of the pit!");
					teleds(cc.x, cc.y);
					u.utrap = 0;
					vision_full_recalc = 1;
				}
			} else
				pline(msg_slipsfree);
		} else pline(msg_snap);
	} else if (mtmp) {
		otmp = MON_WEP(mtmp);	/* can be null */
		if (otmp) {
			You("wrap your bullwhip around %s %s.",
				s_suffix(mon_nam(mtmp)), xname(otmp));
			if (proficient && (!Fumbling || !rn2(10))) {
			    obj_extract_self(otmp);
			    possibly_unwield(mtmp);
			    otmp->owornmask &= ~W_WEP;
			    switch(rn2(proficient + 1)) {
				case 2:
				    /* to floor near you */
				    You("yank %s %s to the %s!",
					s_suffix(mon_nam(mtmp)),
					xname(otmp),
					surface(u.ux, u.uy));
				    if(otmp->otyp == CRYSKNIFE)
					otmp->otyp = WORM_TOOTH;
				    place_object(otmp,u.ux, u.uy);
				    break;
				case 3:
				    /* right into your inventory */
				    if (rn2(25)) {
					You("snatch %s %s!",
						s_suffix(mon_nam(mtmp)),
						xname(otmp));
						otmp = hold_another_object(otmp,
						   "You drop %s!", doname(otmp),
						   (const char *)0);
				    /* proficient with whip, but maybe not
				       so proficient at catching weapons */
				    }
#if 0
				    else {
					int hitu, hitvalu;

					hitvalu = 8 + otmp->spe;
					hitu = thitu(hitvalu,
						dmgval(otmp, &youmonst),
						otmp, xname(otmp));
					if (hitu) {
				You("The %s hits you as you try to snatch it!",
						the(xname(otmp)));
					}
					place_object(otmp, u.ux, u.uy);
				    }
#endif /* 0 */
				    break;
				default:
				    /* to floor beneath mon */
				    You("yank %s from %s %s!",
					the(xname(otmp)),
					s_suffix(mon_nam(mtmp)),
					body_part(HAND));
				    if(otmp->otyp == CRYSKNIFE)
					otmp->otyp = WORM_TOOTH;
				    place_object(otmp, mtmp->mx, mtmp->my);
			    }
			} else {
				pline(msg_slipsfree);
			}
		} else {
			You("flick your bullwhip towards %s.", mon_nam(mtmp));
			if (proficient)
				if (attack(mtmp)) return(1);
			else
				pline(msg_snap);
		}
	} else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		/* it must be air -- water checked above */
		You("snap your whip through thin air.");
	} else
		pline(msg_snap);
	return(1);
}


#define BY_OBJECT	((struct monst *)0)

/* return 1 if the wand is broken, hence some time elapsed */
static int
do_break_wand(obj)
    struct obj *obj;
{
    static const char nothing_else_happens[] = "But nothing else happens...";
    register int i, x, y;
    register struct monst *mon;
    int dmg, damage;
    boolean affects_objects;
    char confirm[QBUFSZ], the_wand[BUFSZ];

    Strcat(strcat(shk_your(the_wand, obj), " "), xname(obj));
    Sprintf(confirm, "Are you really sure you want to break %s?", the_wand);
    if (yn(confirm) == 'n' ) return 0;

    if (nohands(uasmon)) {
	You_cant("break %s without hands!", the_wand);
	return 0;
    } else if (ACURR(A_STR) < 10) {
	You("don't have the strength to break %s!", the_wand);
	return 0;
    }
    pline("Raising %s high above your %s, you break it in two!",
	  the_wand, body_part(HEAD));
    if (obj->spe <= 0) {
	pline(nothing_else_happens);
	goto discard_broken_wand;
    }
    obj->ox = u.ux;
    obj->oy = u.uy;
    current_wand = obj;		/* for destroy_item */
    dmg = obj->spe * 4;
    affects_objects = FALSE;

    switch (obj->otyp) {
    case WAN_WISHING:
    case WAN_NOTHING:
    case WAN_LOCKING:
    case WAN_PROBING:
    case WAN_OPENING:
    case WAN_SECRET_DOOR_DETECTION:
	pline(nothing_else_happens);
	goto discard_broken_wand;
    case WAN_DEATH:
    case WAN_LIGHTNING:
	dmg *= 2;
    case WAN_FIRE:
    case WAN_COLD:
	dmg *= 2;
    case WAN_MAGIC_MISSILE:
	explode(u.ux, u.uy, (obj->otyp - WAN_MAGIC_MISSILE), dmg, WAND_CLASS);
	makeknown(obj->otyp);	/* explode described the effect */
	goto discard_broken_wand;
    case WAN_STRIKING:
	/* we want this before the explosion instead of at the very end */
	pline("A wall of force smashes down around you!");
	dmg = d(1 + obj->spe,6);	/* normally 2d12 */
    case WAN_CANCELLATION:
    case WAN_POLYMORPH:
    case WAN_TELEPORTATION:
    case WAN_UNDEAD_TURNING:
	affects_objects = TRUE;
	break;
    default:
	break;
    }

    /* magical explosion and its visual effect occur before specific effects */
    explode(obj->ox, obj->oy, 0, rnd(dmg), WAND_CLASS);

    /* this makes it hit us last, so that we can see the action first */
    for (i = 0; i <= 8; i++) {
	bhitpos.x = x = obj->ox + xdir[i];
	bhitpos.y = y = obj->oy + ydir[i];
	if (!isok(x,y)) continue;

	if (obj->otyp == WAN_DIGGING) {
	    if(dig_check(BY_OBJECT, FALSE, x, y))
		digactualhole(x, y, BY_OBJECT,
			      (rn2(obj->spe)<3 || !Can_fall_thru(&u.uz)) ?
			       PIT : HOLE);
	    continue;
	} else if(obj->otyp == WAN_CREATE_MONSTER) {
	    (void) makemon((struct permonst *)0, x, y);
	    continue;
	} else {
	    if (x == u.ux && y == u.uy) {
		damage = zapyourself(obj, FALSE);
		if (damage)
		    losehp(damage,
			   self_pronoun("killed %sself by breaking a wand",
					"him"),
			   NO_KILLER_PREFIX);
		if (flags.botl) bot();		/* blindness */
	    } else if ((mon = m_at(x, y)) != 0) {
		(void) bhitm(mon, obj);
	     /* if (flags.botl) bot(); */
	    }
	    if (affects_objects && level.objects[x][y]) {
		(void) bhitpile(obj, bhito, x, y);
		if (flags.botl) bot();		/* potion effects */
	    }
	}
    }

    if (obj->otyp == WAN_LIGHT)
	litroom(TRUE, obj);	/* only needs to be done once */

 discard_broken_wand:
    current_wand = 0;
    check_unpaid(obj);	/* extra charge for _use_ prior to destruction */
    delobj(obj);
    nomul(0);
    return 1;
}

int
doapply()
{
	register struct obj *obj;
	register int res = 1;

	if(check_capacity((char *)0)) return (0);
	obj = getobj(tools, "use or apply");
	if(!obj) return 0;

	if (obj->oclass == WAND_CLASS)
	    return do_break_wand(obj);

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
	case BULLWHIP:
		res = use_whip(obj);
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

			check_unpaid(obj);
			obj->spe--;
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
	case LEASH:
		use_leash(obj);
		break;
	case MAGIC_WHISTLE:
		use_magic_whistle(obj);
		break;
	case TIN_WHISTLE:
		use_whistle(obj);
		break;
	case STETHOSCOPE:
		res = use_stethoscope(obj);
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
	case POT_OIL:
		light_cocktail(obj);
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

		    check_unpaid(obj);
		    obj->spe--;
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
	case LAND_MINE:
	case BEARTRAP:
		use_trap(obj);
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
