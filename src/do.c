/*	SCCS Id: @(#)do.c	3.0	89/11/20
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) */

#include "hack.h"
#ifndef STUPID_CPP
/* fortunately, only errno is used from <errno.h> and all known STUPID_CPPs
 * are on UNIX SYSV and will thus all be using the extern int errno; declared
 * below 
 */
#include <errno.h>

# ifdef _MSC_VER	/* MSC 6.0 defines errno quite differently */
#  if (_MSC_VER >= 600)
#   define SKIP_ERRNO
#  endif
# endif
#endif /* STUPID_CPP */

#ifndef SKIP_ERRNO
extern int errno;
#endif

#if defined(DGK)
extern struct finfo fileinfo[];
#else
extern boolean level_exists[];
#endif

#ifdef SINKS
# ifdef OVLB
static void FDECL(trycall, (struct obj *));
# endif /* OVLB */
STATIC_DCL void FDECL(dosinkring, (struct obj *));
#endif /* SINKS */
STATIC_PTR int FDECL(drop, (struct obj *));
STATIC_DCL void NDECL(litter);
STATIC_PTR int NDECL(wipeoff);
boolean NDECL(drag_down);

#ifdef OVLB

static const char NEARDATA drop_types[] = { '0', GOLD_SYM, '#', 0 };

int
dodrop() {
	return(drop(getobj(drop_types, "drop")));
}

#endif /* OVLB */
#ifdef OVL0

/* Used for objects which sometimes do special things when dropped; must be
 * called with the object not in any chain.  Returns 1 if the object is
 * gone.
 */
boolean
flooreffects(obj,x,y)
struct obj *obj;
int x,y;
{
	struct trap *t = t_at(x,y);
	boolean pool = is_pool(x,y);

	if(obj->otyp == BOULDER && (pool ||
	  (t && (t->ttyp==PIT || t->ttyp==SPIKED_PIT || t->ttyp==TRAPDOOR)))) {
	    if (pool) {
#ifdef STRONGHOLD
		if(levl[x][y].typ == DRAWBRIDGE_UP)
		    levl[x][y].drawbridgemask |= DB_FLOOR;
		else
#endif
		    levl[x][y].typ = ROOM;
		if (cansee(x,y))
		  pline("There is a large splash as the boulder fills the %s.",
			(levl[x][y].typ==POOL) ? "pool" : "moat");
		else if (flags.soundok)
		    You("hear a splash.");
	    } else if (t) {
		if(is_maze_lev
#ifdef STRONGHOLD
		 	&& (dlevel > stronghold_level)
#endif
			&& t->ttyp == TRAPDOOR) return FALSE;
		if (Blind) You("hear the boulder roll.");
		else pline("The boulder %sfills a %s.",
			t->tseen ? "" : "triggers and ",
			t->ttyp == TRAPDOOR ? "trap door" : "pit");
		deltrap(t);
		if (u.utrap && x==u.ux && y==u.uy) {
		    u.utrap = 0;
#ifdef POLYSELF
		    if (!passes_walls(uasmon)) {
#endif
			pline("Unfortunately, you were still in it.");
			losehp(rnd(15),
			  self_pronoun("dropped a boulder onto %sself","him"),
			  NO_KILLER_PREFIX);
#ifdef POLYSELF
		    }
#endif
		}
	    }
	    obfree(obj, (struct obj *)0);
	    mnewsym(x,y);
	    if (!vism_at(x,y) && (x != u.ux || y != u.uy || Invisible) && !Blind)
		newsym(x,y);
	    return TRUE;
	}
	return FALSE;
}

#endif /* OVL0 */
#ifdef OVLB

#ifdef ALTARS
void
doaltarobj(obj)  /* obj is an object dropped on an altar */
	register struct obj *obj;
{
	if (Blind) return;
	if (obj->blessed || obj->cursed) {
		pline("There is %s flash as %s hit%s the altar.",
		      an(Hallucination ? hcolor() :
			 obj->blessed ? amber : black),
		      doname(obj),
		      (obj->quan==1) ? "s" : "");
		if (!Hallucination) obj->bknown = 1;
	} else {
		pline("%s land%s on the altar.", Doname2(obj),
			(obj->quan==1) ? "s" : "");
		obj->bknown = 1;
	}
}
#endif

#ifdef SINKS
static
void
trycall(obj)
register struct obj *obj;
{
	if(!objects[obj->otyp].oc_name_known &&
	   !objects[obj->otyp].oc_uname)
	   docall(obj);
}

STATIC_OVL
void
dosinkring(obj)  /* obj is a ring being dropped over a kitchen sink */
register struct obj *obj;
{
	register struct obj *otmp,*otmp2;
	register boolean ideed = TRUE;

	You("drop %s down the drain.", doname(obj));
	switch(obj->otyp) {	/* effects that can be noticed without eyes */
	    case RIN_SEARCHING:
		You("thought your %s got lost in the sink, but there it is!",
			xname(obj));
		dropx(obj);
		trycall(obj);
		return;
	    case RIN_LEVITATION:
		pline("The sink quivers upward for a moment.");
		break;
	    case RIN_POISON_RESISTANCE:
#ifdef TUTTI_FRUTTI
		You("smell rotten %s.", makeplural(pl_fruit));
#else
		You("smell rotten fruit.");
#endif
		break;
	    case RIN_AGGRAVATE_MONSTER:
		pline("Several flies buzz angrily around the sink.");
		break;
	    case RIN_SHOCK_RESISTANCE:
		pline("Static electricity surrounds the sink.");
		break;
	    case RIN_CONFLICT:
		You("hear loud noises coming from the drain.");
		break;
	    case RIN_GAIN_STRENGTH:
		pline("The water flow seems %ser now.",
			(obj->spe<0) ? "weak" : "strong");
		break;
	    case RIN_INCREASE_DAMAGE:
		pline("The water's force seems %ser now.",
			(obj->spe<0) ? "small" : "great");
		break;
	    default:
		ideed = FALSE;
		break;
	}
	if(!Blind && !ideed) {
	    ideed = TRUE;
	    switch(obj->otyp) {		/* effects that need eyes */
		case RIN_ADORNMENT:
		    pline("The faucets flash brightly for a moment.");
		    break;
		case RIN_REGENERATION:
		    pline("The sink looks as good as new.");
		    break;
		case RIN_INVISIBILITY:
		    You("don't see anything happen to the sink.");
		    break;
		case RIN_SEE_INVISIBLE:
		    You("see some air in the sink.");
		    break;
		case RIN_STEALTH:
		pline("The sink seems to blend into the floor for a moment.");
		    break;
		case RIN_HUNGER:
		    ideed = FALSE;
		    for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
			otmp2 = otmp->nexthere;
			if(otmp != uball && otmp != uchain) {
			    pline("Suddenly, %s vanishes from the sink!",
							doname(otmp));
			    delobj(otmp);
			    ideed = TRUE;
			}
		    }
		    break;
		case RIN_FIRE_RESISTANCE:
		pline("The hot water faucet flashes brightly for a moment.");
		    break;
		case RIN_COLD_RESISTANCE:
		pline("The cold water faucet flashes brightly for a moment.");
		    break;
		case RIN_PROTECTION_FROM_SHAPE_CHAN:
		    pline("The sink looks nothing like a fountain.");
		    break;
		case RIN_PROTECTION:
		    pline("The sink glows %s for a moment.",
			    Hallucination ? hcolor() :
			    (obj->spe<0) ? black : silver);
		    break;
		case RIN_WARNING:
		    pline("The sink glows %s for a moment.",
			    Hallucination ? hcolor() : white);
		    break;
		case RIN_TELEPORTATION:
		    pline("The sink momentarily vanishes.");
		    break;
		case RIN_TELEPORT_CONTROL:
	    pline("The sink looks like it is being beamed aboard somewhere.");
		    break;
#ifdef POLYSELF
		case RIN_POLYMORPH:
		    pline("The sink momentarily looks like a fountain.");
		    break;
		case RIN_POLYMORPH_CONTROL:
	pline("The sink momentarily looks like a regularly erupting geyser.");
		    break;
#endif
	    }
	}
	if(ideed)
	    trycall(obj);
	else
	    You("hear the ring bouncing down the drainpipe.");
	if (!rn2(20)) {
		pline("The sink backs up, leaving %s.", doname(obj));
		dropx(obj);
	}
	else
		useup(obj);
}
#endif

#endif /* OVLB */
#ifdef OVL0

/* some common tests when trying to drop or throw items */
boolean
canletgo(obj,word)
register struct obj *obj;
register const char *word;
{
	if(obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)){
	       if (*word)
			You("cannot %s something you are wearing.",word);
		return(FALSE);
	}
	if (obj->otyp == LOADSTONE && obj->cursed) {
		if (*word)
			pline("For some reason, you cannot %s the stone%s!",
				word,
		      		plur((long)obj->quan));
		/* Kludge -- see invent.c */
		if (obj->corpsenm) {
			struct obj *otmp;

			otmp = obj;
			obj = obj->nobj;
			obj->quan += otmp->quan;
			obj->owt = weight(obj);
			freeinv(otmp);
			obfree(otmp, obj);
		}
		obj->bknown = 1;
		return(FALSE);
	}
#ifdef WALKIES
	if (obj->otyp == LEASH && obj->leashmon != 0) {
	       if (*word)
			pline ("The leash is tied around your %s.",
					body_part(HAND));
		return(FALSE);
	}
#endif
	return(TRUE);
}

STATIC_PTR
int
drop(obj) register struct obj *obj; {
	if(!obj) return(0);
	if(obj->olet == GOLD_SYM) {		/* pseudo object */
		register long amount = OGOLD(obj);

/* Fix bug with dropping huge amounts of gold read as negative    KAA */
		if(amount < 0) {
			u.ugold += amount;
	pline("The LRS would be very interested to know you have that much.");
		} else {
			/* uswallow test added by GAN 01/29/87 */
			if(flags.verbose)
			    You("drop %ld gold piece%s.",
				   amount, plur(amount));
			if(u.uswallow)
				(u.ustuck)->mgold += amount;
			else {
				mkgold(amount, u.ux, u.uy);
				if(Invisible) newsym(u.ux, u.uy);
			}
		}
		free((genericptr_t) obj);
		return(1);
	}
	if(!canletgo(obj,"drop"))
		return(0);
	if(obj == uwep) {
		if(welded(uwep)) {
			weldmsg(obj, FALSE);
			return(0);
		}
		setuwep((struct obj *)0);
		if(uwep) return 0; /* lifesaved and rewielded */
	}
#ifdef SINKS
	if((obj->olet == RING_SYM) && IS_SINK(levl[u.ux][u.uy].typ)
							&& !u.uswallow) {
		dosinkring(obj);
		return(1);
	}
#endif
#ifdef ALTARS
	if (IS_ALTAR(levl[u.ux][u.uy].typ) && !u.uswallow) {
		/* turn water into [(un)holy] water */
		if (obj->otyp == POT_WATER) {
			obj->blessed = !!(levl[u.ux][u.uy].altarmask & A_LAW);
			obj->cursed =
			    !(levl[u.ux][u.uy].altarmask & (A_LAW | A_NEUTRAL));
		}
		doaltarobj(obj);	/* set bknown */
	} else
#endif
	if(flags.verbose) You("drop %s.", doname(obj));
	dropx(obj);
	return(1);
}

/* Called in several places - should not produce texts */
void
dropx(obj)
register struct obj *obj;
{
	freeinv(obj);
	dropy(obj);
}

void
dropy(obj)
register struct obj *obj;
{
	if (flooreffects(obj,u.ux,u.uy)) return;
#ifdef WORM
	if(obj->otyp == CRYSKNIFE)
		obj->otyp = WORM_TOOTH;
#endif
	/* uswallow check done by GAN 01/29/87 */
	if(u.uswallow)
		mpickobj(u.ustuck,obj);
	else  {
		obj->nobj = fobj;
		fobj = obj;
		place_object(obj, u.ux, u.uy);
		if(Invisible) newsym(u.ux,u.uy);
		if(obj != uball) sellobj(obj);
		stackobj(obj);
	}
}

/* drop several things */
int
doddrop() {
	return(ggetobj("drop", drop, 0));
}

#endif /* OVL0 */
#ifdef OVL2

#ifdef STRONGHOLD
static boolean NEARDATA at_ladder = FALSE;	/* on a ladder, used in goto_level */
#endif

int
dodown()
{
	struct trap *trap = 0;

	if((u.ux != xdnstair || u.uy != ydnstair)
#ifdef STRONGHOLD
	   && (!xdnladder || u.ux != xdnladder || u.uy != ydnladder)
#endif
	  ) {
		if (!(trap = t_at(u.ux,u.uy)) || trap->ttyp != TRAPDOOR
			|| (is_maze_lev
#ifdef STRONGHOLD
				&& (dlevel > stronghold_level)
#endif
								)
							|| !trap->tseen) {
			You("can't go down here.");
			return(0);
		}
	}
	if(u.ustuck) {
		You("are being held, and cannot go down.");
		return(1);
	}
	if(Levitation) {
		pline("You're floating high above the %s.",
			levl[u.ux][u.uy].typ == STAIRS ? "stairs" :
#ifdef STRONGHOLD
			levl[u.ux][u.uy].typ == LADDER ? "ladder" :
#endif
			"trap door");
		return(0);
	}

#ifdef WALKIES
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	} else {
#endif
		unsee();
#ifdef STRONGHOLD
		if (levl[u.ux][u.uy].typ == LADDER) at_ladder = TRUE;
#endif
		if (trap)
#ifdef POLYSELF
			You("%s into the trap door.",
				locomotion(uasmon, "jump"));
#else
			You("jump into the trap door.");
#endif
		goto_level(dlevel+1, !trap, TRUE);
#ifdef STRONGHOLD
		at_ladder = FALSE;
#endif
#ifdef WALKIES
	}
#endif
	return(1);
}

int
doup()
{
	if((u.ux != xupstair || u.uy != yupstair)
#ifdef STRONGHOLD
	   && (!xupladder || u.ux != xupladder || u.uy != yupladder)
#endif
	  ) {
		You("can't go up here.");
		return(0);
	}
	if(u.ustuck) {
		You("are being held, and cannot go up.");
		return(1);
	}
#ifdef POLYSELF
	/* Some monsters have carrying capacities less than 5, and we don't
	 * want to totally keep them from going upstairs.
	 */
	if((invent || u.ugold) && inv_weight() + 5 > 0) {
#else
	if(inv_weight() + 5 > 0) {
#endif
		/* No levitation check; inv_weight() already allows for it */
#ifdef STRONGHOLD
		Your("load is too heavy to climb the %s.",
		      levl[u.ux][u.uy].typ == STAIRS ? "stairs" : "ladder");
#else
		Your("load is too heavy to climb the stairs.");
#endif
		return(1);
	}
	if (dlevel == 1) {
#ifdef MACOS
		if(!flags.silent) SysBeep(20);
		if(UseMacAlertText(128,
		    "Beware, there will be no return!  Still climb?") != 1) {
			return 0;
		}
#else
		pline("Beware, there will be no return!  Still climb? ");
		if (yn() != 'y') return(0);
#endif /* MACOS */
	}
#ifdef WALKIES
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	} else {
#endif
		unsee();
#ifdef STRONGHOLD
		if (levl[u.ux][u.uy].typ == LADDER) at_ladder = TRUE;
		goto_level(dlevel-1, 
		    (dlevel-1 < stronghold_level || (at_ladder && 
		       dlevel-1 >= tower_level && dlevel-1 < tower_level+2)),
			   FALSE);
		at_ladder = FALSE;
#else
		goto_level(dlevel-1, (dlevel-1 <= medusa_level), FALSE);
#endif
#ifdef WALKIES
	}
#endif
	return(1);
}

#endif /* OVL2 */
#ifdef OVLB

STATIC_OVL
void
litter()
{
	struct obj *otmp = invent, *nextobj;
	int capacity = weight_cap();

	while (otmp) {
		nextobj = otmp->nobj;
		if ((otmp != uball) && (rnd(capacity) <= otmp->owt)) {
			if (otmp == uwep)
				setuwep((struct obj *)0);
			if ((otmp != uwep) && (canletgo(otmp, ""))) {
				Your("%s you down the stairs.",
				     aobjnam(otmp, "follow"));
				dropx(otmp);
			}
		}
		otmp = nextobj;
	}
}

boolean
drag_down()
{
	boolean forward;
	uchar dragchance = 3;


	/* 
		Assume that the ball falls forward if:

		a) the character is wielding it, or
		b) the character has both hands available to hold it (i.e. is 
		   not wielding any weapon), or 
		c) (perhaps) it falls forward out of his non-weapon hand
	*/

	forward = (!(carried(uball))? 
		  FALSE : ((uwep == uball) || (!uwep))? 
			  TRUE : (boolean)(rn2(3) / 2));

	if (carried(uball)) 
		You("lose your grip on the iron ball.");

	if(forward) {
		if(rn2(6)) {
			You("get dragged downstairs by the iron ball.");
			losehp(rnd(6), "dragged downstairs by an iron ball",
				NO_KILLER_PREFIX);
			return(TRUE);
		}
	} else {
		if(rn2(2)) {
			pline("The iron ball smacks into you!");
			losehp(rnd(20), "iron ball collision", KILLED_BY_AN);
			dragchance -= 2;
		} 
		if(dragchance >= rnd(6)) {
			You("get dragged downstairs by the iron ball.");
			losehp(rnd(3), "dragged downstairs by an iron ball",
				NO_KILLER_PREFIX);
			return(TRUE);
		}
	}
	return(FALSE);
}

#endif /* OVLB */
#ifdef OVL2

int save_dlevel = 0;

void
goto_level(newlevel, at_stairs, falling)
register int newlevel;
register boolean at_stairs, falling;
{
	register int fd;
	register boolean up = (newlevel < dlevel);

#ifdef ENDGAME
	if(dlevel == ENDLEVEL) return;	/* To be on the safe side.. */
#endif
	if(newlevel > MAXLEVEL) newlevel = MAXLEVEL;
	if(newlevel <= 0)
#ifdef ENDGAME
	    if(u.uhave_amulet)
		newlevel = ENDLEVEL;	/* Endgame Level !!! */
	    else
#endif
		done(ESCAPED);		/* in fact < 0 is impossible */

#ifdef MACOS
	freeSegs(&segments);
#endif
/*	If you have the amulet and are trying to get out of Hell, going
 *	up a set of stairs sometimes does some very strange things!
 */
#ifdef HARD
	if(Inhell && up &&
# ifdef STRONGHOLD
           !at_ladder &&
# endif
			(dlevel < MAXLEVEL-3) && u.uhave_amulet) {
	    int olev = newlevel;

	    newlevel = (rn2(4) ? newlevel :
/* neutral */	     !u.ualigntyp ? (rn2(2) ? dlevel : dlevel + (rnd(5) - 2)) :
/* lawful */	     (u.ualigntyp == U_LAWFUL) ? dlevel + (rnd(5) - 2) :
/* chaotic */	     dlevel);
	    if(newlevel < 1) newlevel = dlevel;
	    if(newlevel != olev)
	        pline("A mysterious force momentarily surrounds you...");
	    if(newlevel == dlevel) {
		(void) dotele();
		return;
	    }
	}
#endif /* HARD /* */
	if(newlevel == dlevel) return;	      /* this can happen */
#ifdef STRONGHOLD
	/* In Nethack 3.0, Hell starts after the stronghold.  Moreover,
	 * there are traps in the stronghold, that can send the player
	 * to hell (gnark, gnark)!  So we have to test here:
	 */
	if(!Inhell && newlevel > stronghold_level && !up && !at_ladder
# ifdef ENDGAME
	&& newlevel < ENDLEVEL
# endif
	) {
#else
	if(!Inhell && newlevel >= HELLLEVEL && !up) {
#endif /* STRONGHOLD /**/
	    You("arrive at the center of the earth...");
	    pline("Unfortunately, it is here that hell is located.");
#ifdef MSDOS
	    (void) fflush(stdout);
#endif
	    if(Fire_resistance) {
		pline("But the fire doesn't seem to harm you.");
	    } else {
		save_dlevel = dlevel;
		You("burn to a crisp.");
		You("die...");
		dlevel = maxdlevel = newlevel;
		killer_format = KILLED_BY_AN;
		killer = "visit to hell";
		done(BURNING);
		dlevel = newlevel = save_dlevel; /* in case they survive */
		save_dlevel = 0;
	    }
	}

	glo(dlevel);
#ifdef MSDOS
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
	fd = open(lock, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
# ifdef MACOS
	{
		Str255	fileName;
		OSErr	er;
		short	refNum;
		struct term_info	*t;
		extern WindowPtr	HackWindow;
		
		t = (term_info *)GetWRefCon(HackWindow);
		fileName[0] = (char)strlen(lock);
		Strcpy((char *)&fileName[1],lock);
		if (FSOpen(fileName, t->system.sysVRefNum, &refNum)) {
				if (er = Create(&fileName,t->system.sysVRefNum,
							CREATOR,LEVEL_TYPE))
					SysBeep(20);
		} else {
			(void)SetEOF(refNum,0L);
			(void)FSClose(refNum);
		}
		SetVol(0L,t->system.sysVRefNum);
		fd = open(lock,O_WRONLY | O_BINARY | ((er) ? O_CREAT : 0));
	}
# else
	fd = creat(lock, FCMASK);
# endif /* MACOS */
#endif
	if(fd < 0) {
		/*
		 * This is not quite impossible: e.g., we may have
		 * exceeded our quota. If that is the case then we
		 * cannot leave this level, and cannot save either.
		 * Another possibility is that the directory was not
		 * writable.
		 */
#ifdef DGK
		pline("Cannot create level file '%s'.", lock);
#else
		pline("A mysterious force prevents you from going %s.",
			up ? "up" : "down");
#endif
		return;
	}

#ifdef DGK
	if (!savelev(fd, dlevel, COUNT)) {
# ifdef ZEROCOMP
		bflush(fd);
# endif
		(void) close(fd);
		(void) unlink(lock);
		pline("NetHack is out of disk space for making levels!");
		You("can save, quit, or continue playing.");
		return;
	}
#endif
	if(Punished) unplacebc();
	u.utrap = 0;				/* needed in level_tele */
	u.ustuck = 0;				/* idem */
	keepdogs();
	seeoff(1);
	if(u.uswallow)				/* idem */
		u.uswldtim = u.uswallow = 0;
	flags.nscrinh = 1;
	u.ux = u.ux0 = FAR;				/* hack */
	(void) inshop();			/* probably was a trap door */

#ifdef DGK
# ifdef ZEROCOMP
	bflush(fd);	/* forget buffer */
# endif
	savelev(fd,dlevel, WRITE);
#else
	savelev(fd,dlevel);
#endif
#ifdef ZEROCOMP
	bflush(fd);	/* flush buffer */
#endif
	(void) close(fd);
#ifdef REINCARNATION
	if (newlevel == rogue_level || dlevel == rogue_level) {
		/* No graphics characters on Rogue levels */
		if (dlevel != rogue_level) {
			(void) memcpy((genericptr_t)savesyms,
				      (genericptr_t)showsyms, sizeof savesyms);
			(void) memcpy((genericptr_t)showsyms,
				      (genericptr_t)defsyms, sizeof showsyms);
			showsyms[S_vodoor] = showsyms[S_hodoor] = 
			    showsyms[S_ndoor] = '+';
		}
		if (newlevel != rogue_level)
			(void) memcpy((genericptr_t)showsyms,
				      (genericptr_t)savesyms, sizeof showsyms);
	}
#endif
	dlevel = newlevel;
	if(maxdlevel < dlevel)
		maxdlevel = dlevel;
	glo(dlevel);
	if(
# ifdef ENDGAME
	   dlevel == ENDLEVEL ||
# endif
#if defined(DGK)
	/* If the level has no .where yet, it hasn't been made */
	   !fileinfo[dlevel].where)
#else
	   !level_exists[dlevel])
#endif
		mklev();
	else {
#if defined(DGK)
		/* If not currently accessible, swap it in. */
		if (fileinfo[dlevel].where != ACTIVE)
			swapin_file(dlevel);
#endif
#if (defined(MSDOS) && !defined(TOS)) || defined(MACOS)
		if((fd = open(lock, O_RDONLY | O_BINARY)) < 0) {
#else
		if((fd = open(lock,0)) < 0) {
#endif
			pline("Cannot open \"%s\" (errno %d).", lock, errno);
			pline("Probably someone removed it.");
			done(TRICKED);
		}
#ifdef ZEROCOMP
		minit();
#endif
		getlev(fd, hackpid, dlevel, FALSE);
		(void) close(fd);
	}

#ifdef MACOS
	{
		OSErr	er;
		struct term_info	*t;
		extern WindowPtr	HackWindow;
		
		t = (term_info *)GetWRefCon(HackWindow);
		SetVol(0L,t->system.sysVRefNum);
	}
#endif
#ifdef ENDGAME
	if(dlevel != ENDLEVEL)
#endif
	if(at_stairs) {
	    if(up) {
#ifdef STRONGHOLD
		if (!at_ladder) {
#endif
		    u.ux = xdnstair;
		    u.uy = ydnstair;
#ifdef STRONGHOLD
		} else {
		    u.ux = xdnladder;
		    u.uy = ydnladder;
		}
#endif
/* Remove bug which crashes with levitation/punishment  KAA */
		if(Punished) {
		    if(!Levitation)
#ifdef STRONGHOLD
			pline("With great effort you climb the %s.",
			      !at_ladder ? "stairs" : "ladder");
#else
			pline("With great effort you climb the stairs.");
#endif
		    placebc(1);
		}
	    } else {
#ifdef STRONGHOLD
		if (!at_ladder) {
#endif
		    u.ux = xupstair;
		    u.uy = yupstair;
#ifdef STRONGHOLD
		} else {
		    u.ux = xupladder;
		    u.uy = yupladder;
		}
#endif
		if(at_stairs && !up && ((inv_weight() + 5 > 0) || 
					Punished || Fumbling)) {
#ifdef STRONGHOLD
			You("fall down the %s.",
			    !at_ladder ? "stairs" : "ladder");
#else
			You("fall down the stairs.");
#endif
			if (Punished) {
				if (drag_down())
					litter();
				if (carried(uball)) {
					if (uwep == uball)
						setuwep((struct obj *)0);
					if (uwep != uball)
						freeinv(uball);
				}
				placebc(1);
			} 
			losehp(rnd(3), "falling downstairs", KILLED_BY);
			selftouch("Falling, you");
		}
	    }
	} else {	/* trap door or level_tele */
	    register int tryct = 0;
	    do {
#ifdef STRONGHOLD
		/* Prevent teleport-landing inside the castle */
		if(dlevel == stronghold_level) {
			if(up) u.ux = (COLNO - rnd(8));
			else u.ux = rnd(6);
		}
		else
		/* Prevent teleport-landing inside Vlad's tower */
		if(dlevel >= tower_level && dlevel <= tower_level+2) {
			do {
			    u.ux = rnd(COLNO-1);
			} while (u.ux > 29 && u.ux < 47); 
		}
		else
#endif
		u.ux = rnd(COLNO-1);
		u.uy = rn2(ROWNO);
	    } while(tryct++ < 100 && (levl[u.ux][u.uy].typ != ROOM &&
		     levl[u.ux][u.uy].typ != CORR) || MON_AT(u.ux, u.uy));
	    if(tryct >= 100)
		panic("goto_level: could not relocate player!");
	    if(Punished) {
		if(falling) {
			boolean gets_hit;

			gets_hit = (uwep == uball)? FALSE : (boolean)rn2(5);
			if (carried(uball)) {
				pline("Startled, you drop the iron ball.");
				if (uwep == uball)
					setuwep((struct obj *)0);
				if (uwep != uball)
					freeinv(uball);
			} 
			if(gets_hit){
					pline("The iron ball falls on your %s.",
					body_part(HEAD));
				if (uarmh)
					Your("helmet doesn't help too much...");
				losehp(rnd(25),
					"Crunched in the head by an iron ball",
					NO_KILLER_PREFIX);
			}
		}
		placebc(1);
	    }
	    if(falling)
		selftouch("Falling, you");
	}
	(void) inshop();
	initrack();

	losedogs();
	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
	if(MON_AT(u.ux, u.uy)) {
		impossible("mnexto failed (do.c)?");
		rloc(m_at(u.ux, u.uy));
	}
	flags.nscrinh = 0;
	setsee();
	seeobjs();	/* make old cadavers disappear - riv05!a3 */
	docrt();
	if(!flags.nopick && (OBJ_AT(u.ux, u.uy) || levl[u.ux][u.uy].gmask))
	    pickup(1);
	else read_engr_at(u.ux,u.uy);
#ifdef HARD
	/* Final confrontation */
	if (dlevel == 1 && u.uhave_amulet && flags.no_of_wizards == 0)
	    resurrect();
#endif
	is_maze_lev = (rooms[0].hx < 0
#ifdef STRONGHOLD
		|| dlevel == stronghold_level
		|| (dlevel >= tower_level && dlevel <= tower_level + 2)
#endif
#ifdef ENDGAME
		|| dlevel == ENDLEVEL
#endif
		);
#ifdef MACOS
	freeSegs(&segments);
	segments = SEG_DO;
#endif
}

#endif /* OVL2 */
#ifdef OVL3

int
donull() {
	return(1);	/* Do nothing, but let other things happen */
}

#endif /* OVL3 */
#ifdef OVLB

STATIC_PTR int
wipeoff() {
	if(u.ucreamed < 4)	u.ucreamed = 0;
	else			u.ucreamed -= 4;
	if (Blinded < 4)	Blinded = 0;
	else			Blinded -= 4;
	if (!Blinded) {
		pline("You've got the glop off.");
		u.ucreamed = 0;
		make_blinded(0L,TRUE);
		return(0);
	} else if (!u.ucreamed) {
		Your("%s feels clean now.", body_part(FACE));
		return(0);
	}
	return(1);		/* still busy */
}

int
dowipe()
{
	if(u.ucreamed)  {
		static char NEARDATA buf[39];

		Sprintf(buf, "wiping off your %s", body_part(FACE));
		set_occupation(wipeoff, buf, 0);
		/* Not totally correct; what if they change back after now
		 * but before they're finished wiping?
		 */
		return(1);
	}
	Your("%s is already clean.", body_part(FACE));
	return(1);
}

#endif /* OVLB */
#ifdef OVL1

/* split obj so that it gets size num */
/* remainder is put in the object structure delivered by this call */
struct obj *
splitobj(obj, num) register struct obj *obj; register int num; {
register struct obj *otmp;
	otmp = newobj(obj->onamelth);
	*otmp = *obj;		/* copies whole structure */
	otmp->o_id = flags.ident++;
	obj->quan = num;
	obj->owt = weight(obj);
	otmp->quan -= num;
	otmp->owt = weight(otmp);	/* -= obj->owt ? */
	obj->nobj = obj->nexthere = otmp;
	if (obj->onamelth)
		(void)strncpy(ONAME(otmp), ONAME(obj), (int)obj->onamelth);
	if(obj->unpaid) splitbill(obj,otmp);
	return(otmp);
}

#endif /* OVL1 */
#ifdef OVLB

void
set_wounded_legs(side, timex)
register long side;
register int timex;
{
	if(!Wounded_legs) {
		ATEMP(A_DEX)--;
		flags.botl = 1;
	}

	if(!Wounded_legs || (Wounded_legs & TIMEOUT))
		Wounded_legs |= side + timex;
	else
		Wounded_legs |= side;
}

void
heal_legs()
{
	if(Wounded_legs) {
		if (ATEMP(A_DEX) < 0) ATEMP(A_DEX)++;

		if((Wounded_legs & BOTH_SIDES) == BOTH_SIDES) {
			Your("%s feel somewhat better.",
				makeplural(body_part(LEG)));
		} else {
			Your("%s feels somewhat better.",
				body_part(LEG));
		}
		Wounded_legs = 0;
	}
}

#endif /* OVLB */
