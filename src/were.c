/*	SCCS Id: @(#)were.c	3.1	93/01/17	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVL0

void
were_change(mon)
register struct monst *mon;
{
	register int pm = monsndx(mon->data);

	if(is_were(mon->data))
	    if(is_human(mon->data)) {
		if(Protection_from_shape_changers) return;
		if(!rn2(50-(night()*20)) || flags.moonphase == FULL_MOON) {
		    new_were(mon);
		    if(mons[pm].msound == MS_BARK && flags.soundok)
			You("hear a %s howling at the moon.",
			    pm == PM_HUMAN_WEREJACKAL ? "jackal" : "wolf");
		}
	    } else if(!rn2(30) || Protection_from_shape_changers) new_were(mon);
}

#endif /* OVL0 */
#ifdef OVLB

static int FDECL(counter_were,(int));

static int
counter_were(pm)
int pm;
{
	switch(pm) {
	    case PM_WEREWOLF:	      return(PM_HUMAN_WEREWOLF);
	    case PM_HUMAN_WEREWOLF:   return(PM_WEREWOLF);
	    case PM_WEREJACKAL:	      return(PM_HUMAN_WEREJACKAL);
	    case PM_HUMAN_WEREJACKAL: return(PM_WEREJACKAL);
	    case PM_WERERAT:	      return(PM_HUMAN_WERERAT);
	    case PM_HUMAN_WERERAT:    return(PM_WERERAT);
	    default:		      return(0);
	}
}

void
new_were(mon)
register struct monst *mon;
{
	register int pm;

	pm = counter_were(monsndx(mon->data));
	if(!pm) {
	    impossible("unknown lycanthrope %s.", mon->data->mname);
	    return;
	}

	if(canseemon(mon))
	    pline("%s changes into a %s.", Monnam(mon),
			Hallucination ? rndmonnam() :
			is_human(&mons[pm]) ? "human" :
			mons[pm].mname+4);

	mon->data = &mons[pm];
	if (mon->msleep || !mon->mcanmove) {
	    /* transformation wakens and/or revitalizes */
	    mon->msleep = 0;
	    mon->mfrozen = 0;	/* not asleep or paralyzed */
	    mon->mcanmove = 1;
	}
	/* regenerate by 1/4 of the lost hit points */
	mon->mhp += (mon->mhpmax - mon->mhp) / 4;
	newsym(mon->mx,mon->my);
#ifdef MUSE
	mon_break_armor(mon);
	possibly_unwield(mon);
#endif
}

boolean
were_summon(ptr,yours)	/* were-creature (even you) summons a horde */
register struct permonst *ptr;
register boolean yours;
{
	register int i, typ, pm = monsndx(ptr);
	register struct monst *mtmp;
	boolean success = FALSE;

	if(Protection_from_shape_changers && !yours)
		return FALSE;
	for(i = rnd(5); i > 0; i--) {
	   switch(pm) {

		case PM_WERERAT:
		case PM_HUMAN_WERERAT:
			typ = rn2(3) ? PM_SEWER_RAT : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT ;
			break;
		case PM_WEREJACKAL:
		case PM_HUMAN_WEREJACKAL:
			typ = PM_JACKAL;
			break;
		case PM_WEREWOLF:
		case PM_HUMAN_WEREWOLF:
			typ = rn2(5) ? PM_WOLF : PM_WINTER_WOLF ;
			break;
		default:
			continue;
	    }
	    mtmp = makemon(&mons[typ], u.ux, u.uy);
	    if (mtmp) success = TRUE;
	    if (yours && mtmp)
		(void) tamedog(mtmp, (struct obj *) 0);
	}
	return success;
}

#ifdef POLYSELF
void
you_were() {
	char qbuf[80];
	if(u.umonnum == u.ulycn) return;
	if(Polymorph_control) {
	    Sprintf(qbuf,"Do you want to change into a %s? ", mons[u.ulycn].mname+4);
	    if(yn(qbuf) == 'n') return;
	}
	(void) polymon(u.ulycn);
}
#endif

#endif /* OVLB */

/*were.c*/
