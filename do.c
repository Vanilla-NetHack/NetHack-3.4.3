/*	SCCS Id: @(#)do.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* do.c - version 1.0.3 */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) and 't' (throw) */

#include "hack.h"

extern struct obj *splitobj(), *addinv();
extern boolean hmon();
extern boolean level_exists[];
extern struct monst youmonst;
extern char *Doname();
extern char *nomovemsg;
int	identify();
#ifdef KAA
extern char *xname();
#endif

dodrop() {
	return(drop(getobj("0$#", "drop")));
}

static
drop(obj) register struct obj *obj; {
	if(!obj) return(0);
	if(obj->olet == '$') {		/* pseudo object */
		register long amount = OGOLD(obj);

		if(amount == 0)
			pline("You didn't drop any gold pieces.");
/* Fix bug with dropping huge amounts of gold read as negative    KAA */
		else if(amount < 0) {
			u.ugold += amount;
	pline("The LRS would be very interested to know you have that much.");
		} else {
			/* uswallow test added by GAN 01/29/87 */
			pline("You dropped %ld gold piece%s.",
				 amount, plur(amount));
			if(u.uswallow)
				(u.ustuck)->mgold += amount;
			else {
				mkgold(amount, u.ux, u.uy);
				if(Invisible) newsym(u.ux, u.uy);
			}
		}
		free((char *) obj);
		return(1);
	}
	if(obj->owornmask & (W_ARMOR | W_RING)){
		pline("You cannot drop something you are wearing.");
		return(0);
	}
	if(obj == uwep) {
		if(uwep->cursed) {
			pline("Your weapon is welded to your hand!");
			return(0);
		}
		setuwep((struct obj *) 0);
	}
	pline("You dropped %s.", doname(obj));
	dropx(obj);
	return(1);
}

/* Called in several places - should not produce texts */
dropx(obj)
register struct obj *obj;
{
	freeinv(obj);
	dropy(obj);
}

dropy(obj)
register struct obj *obj;
{
	if(obj->otyp == CRYSKNIFE)
		obj->otyp = WORM_TOOTH;
	/* uswallow check done by GAN 01/29/87 */
	if(u.uswallow)
		mpickobj(u.ustuck,obj);
	else  {
		obj->ox = u.ux;
		obj->oy = u.uy;
		/* Blind check added by GAN 02/18/87 */
		if(Blind)  {
#ifdef KAA
			if(obj->olet != ')')
#endif
			    obj->dknown = index("/=!?*",obj->olet) ? 0 : 1;
			obj->known = 0;
		}
		obj->nobj = fobj;
		fobj = obj;
		if(Invisible) newsym(u.ux,u.uy);
		subfrombill(obj);
		stackobj(obj);
	}
}

/* drop several things */
doddrop() {
	return(ggetobj("drop", drop, 0));
}

dodown()
{
	if(u.ux != xdnstair || u.uy != ydnstair) {
		pline("You can't go down here.");
		return(0);
	}
	if(u.ustuck) {
		pline("You are being held, and cannot go down.");
		return(1);
	}
	if(Levitation) {
		pline("You're floating high above the stairs.");
		return(0);
	}

	goto_level(dlevel+1, TRUE);
	return(1);
}

doup()
{
	if(u.ux != xupstair || u.uy != yupstair) {
		pline("You can't go up here.");
		return(0);
	}
	if(u.ustuck) {
		pline("You are being held, and cannot go up.");
		return(1);
	}
	if(!Levitation && inv_weight() + 5 > 0) {
		pline("Your load is too heavy to climb the stairs.");
		return(1);
	}

	goto_level(dlevel-1, TRUE);
	return(1);
}

goto_level(newlevel, at_stairs)
register int newlevel;
register boolean at_stairs;
{
	register fd;
	register boolean up = (newlevel < dlevel);

	if(newlevel <= 0) done("escaped");    /* in fact < 0 is impossible */
	if(newlevel > MAXLEVEL) newlevel = MAXLEVEL;	/* strange ... */
	if(newlevel == dlevel) return;	      /* this can happen */

	glo(dlevel);
#ifdef DGK
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
	fd = open(lock, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FMASK);
#else
	fd = creat(lock, FMASK);
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
		(void) close(fd);
		(void) unlink(lock);
		pline("HACK is out of disk space for making levels!");
		pline("You can save, quit, or continue playing.");
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
	u.ux = FAR;				/* hack */
	(void) inshop();			/* probably was a trapdoor */

#ifdef DGK
	savelev(fd,dlevel, WRITE);
#else
	savelev(fd,dlevel);
#endif
	(void) close(fd);

	dlevel = newlevel;
	if(maxdlevel < dlevel)
		maxdlevel = dlevel;
	glo(dlevel);
#ifdef MSDOS
	/* If the level has no where yet, it hasn't been made
	 */
	if(!fileinfo[dlevel].where)
#else
	if(!level_exists[dlevel])
#endif
		mklev();
	else {
		extern int hackpid;
#ifdef DGK
		/* If not currently accessible, swap it in.
		 */
		if (fileinfo[dlevel].where != ACTIVE)
			swapin_file(dlevel);

		if((fd = open(lock, O_RDONLY | O_BINARY)) < 0) {
#else
		if((fd = open(lock,0)) < 0) {
#endif
			pline("Cannot open %s .", lock);
			pline("Probably someone removed it.");
			done("tricked");
		}
		getlev(fd, hackpid, dlevel);
		(void) close(fd);
	}

	if(at_stairs) {
	    if(up) {
		u.ux = xdnstair;
		u.uy = ydnstair;
		if(!u.ux) {		/* entering a maze from below? */
		    u.ux = xupstair;	/* this will confuse the player! */
		    u.uy = yupstair;
		}
/* Remove bug which crashes with levitation/punishment  KAA */
		if(Punished) {
		    if(!Levitation) 
			pline("With great effort you climb the stairs.");
		    placebc(1);
		}
	    } else {
		u.ux = xupstair;
		u.uy = yupstair;
		if(inv_weight() + 5 > 0 || Punished){
			pline("You fall down the stairs.");	/* %% */
			losehp(rnd(3), "fall");
			if(Punished) {
			    if(uwep != uball && rn2(3)){
				pline("... and are hit by the iron ball.");
				losehp(rnd(20), "iron ball");
			    }
			    placebc(1);
			}
			selftouch("Falling, you");
		}
	    }
	    { register struct monst *mtmp = m_at(u.ux, u.uy);
	      if(mtmp)
		mnexto(mtmp);
	    }
	} else {	/* trapdoor or level_tele */
	    do {
		u.ux = rnd(COLNO-1);
		u.uy = rn2(ROWNO);
	    } while(levl[u.ux][u.uy].typ != ROOM ||
			m_at(u.ux,u.uy));
	    if(Punished){
		if(uwep != uball && !up /* %% */ && rn2(5)){
			pline("The iron ball falls on your head.");
			losehp(rnd(25), "iron ball");
		}
		placebc(1);
	    }
	    selftouch("Falling, you");
	}
	(void) inshop();
	initrack();

	losedogs();
	{ register struct monst *mtmp;
	  if(mtmp = m_at(u.ux, u.uy)) mnexto(mtmp);	/* riv05!a3 */
	}
	flags.nscrinh = 0;
	setsee();
	seeobjs();	/* make old cadavers disappear - riv05!a3 */
	docrt();
	pickup(1);
	if (!Blind) read_engr_at(u.ux,u.uy);
}

donull() {
	return(1);	/* Do nothing, but let other things happen */
}

#if defined(KAA) && defined(KOPS)
wipeoff()
{
	u.ucreamed -= 4;
	if(u.ucreamed > 0)  {
		Blind -= 4;
		if(Blind <= 1) {
			pline("You've got the glop off.");
			u.ucreamed = 0;
			Blind = 1;
			return(0);
		}
		return(1);		/* still busy */
	}
	pline("You're face feels clean now.");
	u.ucreamed = 0;
	return(0);
}
	
dowipe()
{
	if(u.ucreamed)  {
#ifdef DGKMOD
		set_occupation(wipeoff, "wiping off your face", 0);
#else
		occupation = wipeoff;
		occtxt = "wiping off your face";
#endif
		return(1);
	}
	pline("You're face is already clean.");
	return(1);
}
#endif

/* split obj so that it gets size num */
/* remainder is put in the object structure delivered by this call */
struct obj *
splitobj(obj, num) register struct obj *obj; register int num; {
register struct obj *otmp;
	otmp = newobj(0);
	*otmp = *obj;		/* copies whole structure */
	otmp->o_id = flags.ident++;
	otmp->onamelth = 0;
	obj->quan = num;
	obj->owt = weight(obj);
	otmp->quan -= num;
	otmp->owt = weight(otmp);	/* -= obj->owt ? */
	obj->nobj = otmp;
	if(obj->unpaid) splitbill(obj,otmp);
	return(otmp);
}

more_experienced(exp,rexp)
register int exp, rexp;
{
	extern char pl_character[];

	u.uexp += exp;
	u.urexp += 4*exp + rexp;
	if(exp) flags.botl = 1;
	if(u.urexp >= ((pl_character[0] == 'W') ? 1000 : 2000))
		flags.beginner = 0;
}

set_wounded_legs(side, timex)
register long side;
register int timex;
{
	if(!Wounded_legs || (Wounded_legs & TIMEOUT))
		Wounded_legs |= side + timex;
	else
		Wounded_legs |= side;
}

heal_legs()
{
	if(Wounded_legs) {
		if((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
			pline("Your legs feel somewhat better.");
		else
			pline("Your leg feels somewhat better.");
		Wounded_legs = 0;
	}
}
