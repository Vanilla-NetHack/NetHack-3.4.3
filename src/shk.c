/*	SCCS Id: @(#)shk.c	3.0	89/11/27
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define MONATTK_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
#include "hack.h"

#include "eshk.h"

#ifdef KOPS
static int FDECL(makekops, (coord *));
static void NDECL(kops_gone);
#endif /* KOPS */

#define	NOTANGRY(mon)	mon->mpeaceful
#define	ANGRY(mon)	!NOTANGRY(mon)

/* Descriptor of current shopkeeper. Note that the bill need not be
   per-shopkeeper, since it is valid only when in a shop. */
VSTATIC struct monst *shopkeeper;
VSTATIC struct bill_x *bill;
VSTATIC int shlevel;		/* level of this shopkeeper */
/* struct obj *billobjs;	/* objects on bill with bp->useup */
				/* only accessed here and by save & restore */
VSTATIC long int total; 	/* filled by addupbill() */
VSTATIC long int followmsg;	/* last time of follow message */

static void setpaid(), FDECL(findshk, (int));
static int FDECL(dopayobj, (struct bill_x *)), FDECL(getprice, (struct obj *));
static struct obj *FDECL(bp_to_obj, (struct bill_x *));

#ifdef OVLB

/*
	invariants: obj->unpaid iff onbill(obj) [unless bp->useup]
		obj->quan <= bp->bquan
 */

char *
shkname(mtmp)				/* called in do_name.c */
register struct monst *mtmp;
{
	return(ESHK(mtmp)->shknam);
}

void
shkdead(mtmp)				/* called in mon.c */
register struct monst *mtmp;
{
	register struct eshk *eshk = ESHK(mtmp);

	if(eshk->shoplevel == dlevel)
		rooms[eshk->shoproom].rtype = OROOM;
	if(mtmp == shopkeeper) {
		setpaid();
		shopkeeper = 0;
		bill = (struct bill_x *) -1000;	/* dump core when referenced */
	}
}

void
replshk(mtmp,mtmp2)
register struct monst *mtmp, *mtmp2;
{
	if(mtmp == shopkeeper) {
		shopkeeper = mtmp2;
		bill = &(ESHK(shopkeeper)->bill[0]);
	}
}

static void
setpaid(){	/* caller has checked that shopkeeper exists */
		/* either we paid or left the shop or he just died */
	register struct obj *obj;
	register struct monst *mtmp;
	for(obj = invent; obj; obj = obj->nobj)
		obj->unpaid = 0;
	for(obj = fobj; obj; obj = obj->nobj)
		obj->unpaid = 0;
	for(obj = fcobj; obj; obj = obj->nobj)
		obj->unpaid = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		for(obj = mtmp->minvent; obj; obj = obj->nobj)
			obj->unpaid = 0;
	for(mtmp = fallen_down; mtmp; mtmp = mtmp->nmon)
		for(obj = mtmp->minvent; obj; obj = obj->nobj)
			obj->unpaid = 0;
	while(obj = billobjs){
		billobjs = obj->nobj;
		free((genericptr_t) obj);
	}
	if(shopkeeper) {
		ESHK(shopkeeper)->billct = 0;
		ESHK(shopkeeper)->credit = 0L;
		ESHK(shopkeeper)->debit = 0L;
	}
}

static void
addupbill(){	/* delivers result in total */
		/* caller has checked that shopkeeper exists */
	register int ct = ESHK(shopkeeper)->billct;
	register struct bill_x *bp = bill;
	total = 0;
	while(ct--){
		total += bp->price * bp->bquan;
		bp++;
	}
}

#endif /* OVLB */
#ifdef OVL2

int
inshop() {
	register int roomno = inroom(u.ux,u.uy);

	/* Did we just leave a shop? */
	if(u.uinshop &&
	    (u.uinshop != roomno + 1 || shlevel != dlevel || !shopkeeper)) {

	/* This is part of the bugfix for shopkeepers not having their
	 * bill paid.  As reported by ab@unido -dgk
	 * I made this standard due to the KOPS code below. -mrs
	 */
		if(shopkeeper) {
		    if(ESHK(shopkeeper)->billct || ESHK(shopkeeper)->debit) {
			if(inroom(shopkeeper->mx, shopkeeper->my)
			    == u.uinshop - 1)	/* ab@unido */
			    You("escaped the shop without paying!");
			addupbill();
			total += ESHK(shopkeeper)->debit;
			You("stole %ld zorkmid%s worth of merchandise.",
				total, plur(total));
			ESHK(shopkeeper)->robbed += total;
			ESHK(shopkeeper)->credit = 0L;
			ESHK(shopkeeper)->debit = 0L;
			if (pl_character[0] != 'R') /* stealing is unlawful */
				adjalign(-sgn(u.ualigntyp));
			setpaid();
			if((rooms[ESHK(shopkeeper)->shoproom].rtype == SHOPBASE)
			    == (rn2(3) == 0))
			    ESHK(shopkeeper)->following = 1;
#ifdef KOPS
		    {   /* Keystone Kops srt@ucla */
			coord mm;

			if (flags.soundok)
			    pline("An alarm sounds throughout the dungeon!");
			if(flags.verbose) {
			    if((mons[PM_KEYSTONE_KOP].geno & G_GENOD) &&
 			       (mons[PM_KOP_SERGEANT].geno & G_GENOD) &&
 			       (mons[PM_KOP_LIEUTENANT].geno & G_GENOD) &&
			       (mons[PM_KOP_KAPTAIN].geno & G_GENOD)) {
				if (flags.soundok)
				    pline("But no one seems to respond to it.");
			    } else
				pline("The Keystone Kops are after you!");
			}
			/* Create a swarm near the staircase */
			mm.x = xdnstair;
			mm.y = ydnstair;
			(void) makekops(&mm);
			/* Create a swarm near the shopkeeper */
			mm.x = shopkeeper->mx;
			mm.y = shopkeeper->my;
			(void) makekops(&mm);
		    }
#endif
		    }
		    shopkeeper = 0;
		    shlevel = 0;
		}
		u.uinshop = 0;
	}

	/* Did we just enter a zoo of some kind? */
	/* This counts everything except shops and vaults
	   -- vault.c insists that a vault remain a VAULT */
	if(roomno >= 0) {
		register int rt = rooms[roomno].rtype;
		register struct monst *mtmp;

		switch (rt) {
		case ZOO:
		    pline("Welcome to David's treasure zoo!");
		    break;
		case SWAMP:
		    pline("It looks rather muddy down here.");
		    break;
#ifdef THRONES
		case COURT:
		    You("enter an opulent throne room!");
		    break;
#endif
		case MORGUE:
		    if(midnight())
			pline("Run away!  Run away!");
		    else
			You("have an uncanny feeling...");
		    break;
		case BEEHIVE:
		    You("enter a giant beehive!");
		    break;
#ifdef ARMY
		case BARRACKS:
		    if(!((mons[PM_SOLDIER].geno & G_GENOD) &&
		         (mons[PM_SERGEANT].geno & G_GENOD) &&
		         (mons[PM_LIEUTENANT].geno & G_GENOD) &&
		         (mons[PM_CAPTAIN].geno & G_GENOD)))
		    	You("enter a military barracks!");
		    else You("enter an abandoned barracks.");
		    break;
#endif
#ifdef ORACLE
		case DELPHI:
		    if(!(mons[PM_ORACLE].geno & G_GENOD))
		        pline("\"Hello, %s, welcome to Delphi!\"", plname);
		    break;
#endif
		default:
		    rt = 0;
		}

		if(rt != 0) {
		    rooms[roomno].rtype = OROOM;
		    if(rt==COURT || rt==SWAMP || rt==MORGUE || rt==ZOO)
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			    /* was if(rt != ZOO || !rn2(3)) -- why should ZOO
			       be different from COURT or MORGUE? */
			    if(!Stealth && !rn2(3))
				mtmp->msleep = 0;
		}
	}
#if defined(ALTARS) && defined(THEOLOGY)
	if(roomno >= 0 && rooms[roomno].rtype == TEMPLE) {
	    intemple();
	}
#endif
	/* Did we just enter a shop? */
	if(roomno >= 0 && rooms[roomno].rtype >= SHOPBASE) {
	    register int rt = rooms[roomno].rtype;

	    if(shlevel != dlevel || !shopkeeper
				 || ESHK(shopkeeper)->shoproom != roomno)
		findshk(roomno);
	    if(!shopkeeper) {
		rooms[roomno].rtype = OROOM;
		u.uinshop = 0;
	    } else if(!u.uinshop){
		if(!ESHK(shopkeeper)->visitct ||
		   strncmp(ESHK(shopkeeper)->customer, plname, PL_NSIZ)) {
		    /* He seems to be new here */
		    ESHK(shopkeeper)->visitct = 0;
		    ESHK(shopkeeper)->following = 0;
		    (void) strncpy(ESHK(shopkeeper)->customer,plname,PL_NSIZ);
		    NOTANGRY(shopkeeper) = 1;
		}
		if(!ESHK(shopkeeper)->following && inhishop(shopkeeper)) {
		    if(Invis) {
			pline("%s senses your presence.", shkname(shopkeeper));
			verbalize("Invisible customers are not welcome!");
		    }
		    else
		    if(ANGRY(shopkeeper))
			pline("\"So, %s, you dare return to %s's %s?!\"",
			    plname,
			    shkname(shopkeeper),
			    shtypes[rt - SHOPBASE].name);
		    else
		    if(ESHK(shopkeeper)->robbed)
			pline("\"Beware, %s!  I am upset about missing stock!\"",
			    plname);
		    else
			pline("\"Hello, %s!  Welcome%s to %s's %s!\"",
			    plname,
			    ESHK(shopkeeper)->visitct++ ? " again" : "",
			    shkname(shopkeeper),
			    shtypes[rt - SHOPBASE].name);
		    if(carrying(PICK_AXE) != (struct obj *)0 && !Invis) {
			verbalize(NOTANGRY(shopkeeper) ?
			   "Will you please leave your pick-axe outside?" :
			   "Leave the pick-axe outside.");
			if(dochug(shopkeeper)) {
			    u.uinshop = 0;	/* he died moving */
			    return(0);
			}
		    }
		}
		u.uinshop = (unsigned int)(roomno + 1);
	    }
	}
	return (int)u.uinshop;
}

#endif /* OVL2 */
#ifdef OVLB

int
inhishop(mtmp)
register struct monst *mtmp;
{
	return((ESHK(mtmp)->shoproom == inroom(mtmp->mx, mtmp->my) &&
		ESHK(mtmp)->shoplevel == dlevel));
}

boolean
tended_shop(sroom)
struct mkroom *sroom;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->isshk && &rooms[ESHK(mtmp)->shoproom] == sroom
		&& inhishop(mtmp)) return(TRUE);
	return(FALSE);
}

static void
findshk(roomno)
register int roomno;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->isshk && ESHK(mtmp)->shoproom == roomno
			   && ESHK(mtmp)->shoplevel == dlevel) {
		shopkeeper = mtmp;
		bill = &(ESHK(shopkeeper)->bill[0]);
		shlevel = dlevel;
		if(ANGRY(shopkeeper) &&
		   strncmp(ESHK(shopkeeper)->customer,plname,PL_NSIZ))
			NOTANGRY(shopkeeper) = 1;
		/* billobjs = 0; -- this is wrong if we save in a shop */
		/* (and it is harmless to have too many things in billobjs) */
		return;
	}
	shopkeeper = 0;
	shlevel = 0;
	bill = (struct bill_x *) -1000;	/* dump core when referenced */
}

static struct bill_x *
onbill(obj)
register struct obj *obj;
{
	register struct bill_x *bp;
	if(!shopkeeper) return (struct bill_x *)0;
	for(bp = bill; bp < &bill[ESHK(shopkeeper)->billct]; bp++)
		if(bp->bo_id == obj->o_id) {
			if(!obj->unpaid) pline("onbill: paid obj on bill?");
			return(bp);
		}
	if(obj->unpaid) pline("onbill: unpaid obj not on bill?");
	return (struct bill_x *)0;
}

/* called with two args on merge */
void
obfree(obj, merge)
register struct obj *obj, *merge;
{
	register struct bill_x *bp = onbill(obj);
	register struct bill_x *bpm;
	if(bp) {
		if(!merge){
			bp->useup = 1;
			obj->unpaid = 0;	/* only for doinvbill */
			obj->nobj = billobjs;
			billobjs = obj;
			return;
		}
		bpm = onbill(merge);
		if(!bpm){
			/* this used to be a rename */
			impossible("obfree: not on bill??");
			return;
		} else {
			/* this was a merger */
			bpm->bquan += bp->bquan;
			ESHK(shopkeeper)->billct--;
			*bp = bill[ESHK(shopkeeper)->billct];
		}
	}
	free((genericptr_t) obj);
}

static long
check_credit(tmp, shkp)
long tmp;
register struct monst *shkp;
{
	long credit = ESHK(shkp)->credit;

	if(credit == 0L) return(tmp);
	if(credit >= tmp) {
		pline("The price is deducted from your credit.");
		ESHK(shkp)->credit -=tmp;
		tmp = 0L;
	} else {
		pline("The price is partially covered by your credit.");
		ESHK(shkp)->credit = 0L;
		tmp -= credit;
	}
	return(tmp);
}

static void
pay(tmp,shkp)
long tmp;
register struct monst *shkp;
{
	long robbed = ESHK(shkp)->robbed;
	long balance = ((tmp <= 0L) ? tmp : check_credit(tmp, shkp));

	u.ugold -= balance;
	shkp->mgold += balance;
	flags.botl = 1;
	if(robbed) {
		robbed -= tmp;
		if(robbed < 0) robbed = 0L;
		ESHK(shkp)->robbed = robbed;
	}
}

/* return shkp to home position */
void
home_shk(shkp)
register struct monst *shkp;
{
	register xchar x = ESHK(shkp)->shk.x, y = ESHK(shkp)->shk.y;
	if(MON_AT(x, y))
		mnearto(m_at(x,y), x, y, FALSE);
	remove_monster(shkp->mx, shkp->my);
	place_monster(shkp, x, y);
	unpmon(shkp);
}

void
make_happy_shk(shkp)
struct monst *shkp;
{
	register boolean wasmad = ANGRY(shkp);

	NOTANGRY(shkp) = 1;
	ESHK(shkp)->following = 0;
	ESHK(shkp)->robbed = 0L;
	if (pl_character[0] != 'R')
		adjalign(sgn(u.ualigntyp));
	if(!inhishop(shkp)) {
		pline("Satisfied, %s suddenly disappears!", mon_nam(shkp));
		if(ESHK(shkp)->shoplevel == dlevel)
			home_shk(shkp);
		else
			fall_down(shkp, ESHK(shkp)->shoplevel);
	} else if(wasmad)
		pline("%s calms down.", Monnam(shkp));
#ifdef KOPS
	kops_gone();
#endif
}

static const char no_money[] = "Moreover, you have no money.";

int
dopay()
{
	long ltmp;
	register struct bill_x *bp;
	register struct monst *shkp;
	int pass, tmp;

	multi = 0;
	(void) inshop();
	for(shkp = fmon; shkp; shkp = shkp->nmon)
		if(shkp->isshk && dist(shkp->mx,shkp->my) < 3)
			break;
	if(!shkp && u.uinshop && inhishop(shopkeeper))
		shkp = shopkeeper;

	if(!shkp) {
		pline("There is nobody here to receive your payment.");
		return(0);
	}
	ltmp = ESHK(shkp)->robbed;
	if(shkp != shopkeeper && NOTANGRY(shkp)) {
		if(!ltmp)
		    You("do not owe %s anything.", mon_nam(shkp));
		else if(!u.ugold)
		    You("have no money.");
		else {
		    long ugold = u.ugold;

		    if(ugold > ltmp) {
			You("give %s the %ld gold piece%s %s asked for.",
			    mon_nam(shkp), ltmp, plur(ltmp),
			    ESHK(shkp)->ismale ? "he" : "she");
			pay(ltmp, shkp);
		    } else {
			You("give %s all your gold.", mon_nam(shkp));
			pay(u.ugold, shkp);
		    }
		    if(ugold < ltmp/2L)
			pline("Unfortunately, %s doesn't look satisfied.",
			    ESHK(shkp)->ismale ? "he" : "she");
		    else
			make_happy_shk(shkp);
		}
		return(1);
	}

	/* ltmp is still ESHK(shkp)->robbed here */
	if(!ESHK(shkp)->billct && !ESHK(shkp)->debit) {
		if(!ltmp && NOTANGRY(shkp)) {
		    You("do not owe %s anything.", mon_nam(shkp));
		    if(!u.ugold) pline(no_money);
		} else if(ltmp) {
		    pline("%s is after blood, not money!", mon_nam(shkp));
		    if(u.ugold < ltmp/2L) {
			if(!u.ugold) pline(no_money);
			else pline("Besides, you don't have enough to interest %s.",
				ESHK(shkp)->ismale ? "him" : "her");
			return(1);
		    }
		    pline("But since %s shop has been robbed recently,",
			ESHK(shkp)->ismale ? "his" : "her");
		    pline("you %scompensate %s for %s losses.",
			(u.ugold < ltmp) ? "partially " : "",
			mon_nam(shkp),
			ESHK(shkp)->ismale ? "his" : "her");
		    pay(u.ugold < ltmp ? u.ugold : ltmp, shkp);
		    make_happy_shk(shkp);
		} else {
		    /* shopkeeper is angry, but has not been robbed --
		     * door broken, attacked, etc. */
		    pline("%s is after your hide, not your money!",
					mon_nam(shkp));
		    if(u.ugold < 1000L) {
			if(!u.ugold) pline(no_money);
			else
		pline("Besides, you don't have enough to interest %s.",
				ESHK(shkp)->ismale ? "him" : "her");
			return(1);
		    }
		    You("try to appease %s by giving %s 1000 gold pieces.",
				a_monnam(shkp, "angry"),
				ESHK(shkp)->ismale ? "him" : "her");
		    pay(1000L,shkp);
		    if(strncmp(ESHK(shkp)->customer, plname, PL_NSIZ)
		    		|| rn2(3))
			make_happy_shk(shkp);
		    else
			pline("But %s is as angry as ever.", Monnam(shkp));
		}
		return(1);
	}
	if(shkp != shopkeeper) {
		impossible("dopay: not to shopkeeper?");
		if(shopkeeper) setpaid();
		return(0);
	}
	/* pay debt, if any, first */
	if(ESHK(shopkeeper)->debit) {
	        You("owe %s %ld zorkmid%s for the use of merchandise.",
			shkname(shopkeeper), ESHK(shopkeeper)->debit,
		        plur(ESHK(shopkeeper)->debit));
	        if(u.ugold + ESHK(shopkeeper)->credit < 
					ESHK(shopkeeper)->debit) {
		    pline("But you don't have enough gold%s.",
			ESHK(shopkeeper)->credit ? " or credit" : "");
		    return(1);
	        } else {
		    long dtmp = ESHK(shopkeeper)->debit;

		    if(ESHK(shopkeeper)->credit >= dtmp) {
			ESHK(shopkeeper)->credit -= dtmp;
			ESHK(shopkeeper)->debit = 0L;
	                Your("debt is covered by your credit.");
		    } else if(!ESHK(shopkeeper)->credit) {
			u.ugold -= dtmp;
			shopkeeper->mgold += dtmp;
			ESHK(shopkeeper)->debit = 0L;
			You("pay that debt.");
			flags.botl = 1;
		    } else {
			dtmp -= ESHK(shopkeeper)->credit;
			ESHK(shopkeeper)->credit = 0L;
			u.ugold -= dtmp;
			shopkeeper->mgold += dtmp;
			ESHK(shopkeeper)->debit = 0L;
			pline("That debt is partially offset by your credit.");
			You("pay the remainder.");
			flags.botl = 1;
		    }
		}
	}
	for(pass = 0; pass <= 1; pass++) {
		tmp = 0;
		while(tmp < ESHK(shopkeeper)->billct) {
		    bp = &bill[tmp];
		    if(!pass && !bp->useup) {
			tmp++;
			continue;
		    }
		    if(!dopayobj(bp)) return(1);
#ifdef MSDOS
		    *bp = bill[--ESHK(shopkeeper)->billct];
#else
		    bill[tmp] = bill[--ESHK(shopkeeper)->billct];
#endif /* MSDOS /**/
		}
	}
	if(!ANGRY(shopkeeper))
	    pline("\"Thank you for shopping in %s's %s!\"",
		shkname(shopkeeper),
		shtypes[rooms[ESHK(shopkeeper)->shoproom].rtype - SHOPBASE].name);
	return(1);
}

/* return 1 if paid successfully */
/*        0 if not enough money */
/*       -1 if object could not be found (but was paid) */
static int
dopayobj(bp)
register struct bill_x *bp;
{
	register struct obj *obj;
	long ltmp;

	/* find the object on one of the lists */
	obj = bp_to_obj(bp);

	if(!obj) {
		impossible("Shopkeeper administration out of order.");
		setpaid();	/* be nice to the player */
		return(0);
	}

	if(!obj->unpaid && !bp->useup){
		impossible("Paid object on bill??");
		return(1);
	}
	obj->unpaid = 0;
	ltmp = bp->price * bp->bquan;
	if(ANGRY(shopkeeper)) ltmp += ltmp/3L;
	if(u.ugold + ESHK(shopkeeper)->credit < ltmp){
		You("don't have gold%s enough to pay for %s.",
			(ESHK(shopkeeper)->credit > 0L) ? " or credit" : "",
			doname(obj));
		obj->unpaid = 1;
		return(0);
	}
	pay(ltmp, shopkeeper);
	You("bought %s for %ld gold piece%s.",
		doname(obj), ltmp, plur(ltmp));
	if(bp->useup) {
		register struct obj *otmp = billobjs;
		if(obj == billobjs)
			billobjs = obj->nobj;
		else {
			while(otmp && otmp->nobj != obj) otmp = otmp->nobj;
			if(otmp) otmp->nobj = obj->nobj;
			else pline("Error in shopkeeper administration.");
		}
		free((genericptr_t) obj);
	}
	return(1);
}

/* routine called after dying (or quitting) with nonempty bill or upset shk */
boolean
paybill(){
	register struct monst *mtmp;
	register long loss = 0L;
	register struct obj *otmp;
	register xchar ox, oy;
	register boolean take = FALSE;
	register boolean taken = FALSE;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->isshk) {
		/* for bones: we don't want a shopless shk around */
		if(ESHK(mtmp)->shoplevel != dlevel) mongone(mtmp);
		else shopkeeper = mtmp;
	    }

	if(!shopkeeper) return(FALSE);

	/* get one case out of the way: you die in the shop, the */
	/* shopkeeper is peaceful, nothing stolen, nothing owed. */
	if(in_shop(u.ux,u.uy) && !IS_DOOR(levl[u.ux][u.uy].typ) &&
	    !ESHK(shopkeeper)->billct && !ESHK(shopkeeper)->robbed &&
	    !ESHK(shopkeeper)->debit && inhishop(shopkeeper) && 
	     NOTANGRY(shopkeeper) && !ESHK(shopkeeper)->following) {
		pline("%s gratefully inherits all your possessions.",
				Monnam(shopkeeper));
		goto clear;
	}

	if(ESHK(shopkeeper)->billct || ESHK(shopkeeper)->debit ||
			ESHK(shopkeeper)->robbed) {
		addupbill();
		total += ESHK(shopkeeper)->debit;
		loss = ((total >= ESHK(shopkeeper)->robbed) ? total :
				ESHK(shopkeeper)->robbed);
		take = TRUE;
	}

	if(ESHK(shopkeeper)->following || ANGRY(shopkeeper) || take) {
		if((loss > u.ugold) || !loss) {
			pline("%s comes and takes all your possessions.",
					Monnam(shopkeeper));
			taken = TRUE;
			shopkeeper->mgold += u.ugold;
			u.ugold = 0L;
			/* in case bones: make it be for real... */
			if(!in_shop(u.ux, u.uy) || IS_DOOR(levl[u.ux][u.uy].typ)) {
			    /* shk.x,shk.y is the position immediately in
			     * front of the door -- move in one more space
			     */
			    ox = ESHK(shopkeeper)->shk.x;
			    oy = ESHK(shopkeeper)->shk.y;
			    ox += sgn(ox - ESHK(shopkeeper)->shd.x);
			    oy += sgn(oy - ESHK(shopkeeper)->shd.y);
			} else {
			    ox = u.ux;
			    oy = u.uy;
			}

			if (invent) {
			    for(otmp = invent; otmp; otmp = otmp->nobj)
				place_object(otmp, ox, oy);

			    /* add to main object list at end so invent is
			       still good */
			    if (fobj) {
				otmp = fobj;
				while(otmp->nobj)
				    otmp = otmp->nobj;
				otmp->nobj = invent;
			    } else
				fobj = invent;
			}
		} else {
			u.ugold -= loss;
			shopkeeper->mgold += loss;
			pline("%s comes and takes %ld zorkmid%s %sowed %s.",
			      Monnam(shopkeeper),
			      loss,
			      plur(loss),
			      strncmp(ESHK(shopkeeper)->customer, plname, PL_NSIZ) ? "" : "you ",
			      ESHK(shopkeeper)->ismale ? "him" : "her");
		}

		/* in case we create bones */
		if(!inhishop(shopkeeper))
			home_shk(shopkeeper);
	}
clear:
	setpaid();
	return(taken);
}

/* find obj on one of the lists */
static struct obj *
bp_to_obj(bp)
register struct bill_x *bp;
{
	register struct obj *obj;
	register struct monst *mtmp;
	register unsigned int id = bp->bo_id;

	if(bp->useup)
		obj = o_on(id, billobjs);
	else if(!(obj = o_on(id, invent)) &&
		!(obj = o_on(id, fobj)) &&
		!(obj = o_on(id, fcobj))) {
		    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(obj = o_on(id, mtmp->minvent))
			    break;
		    for(mtmp = fallen_down; mtmp; mtmp = mtmp->nmon)
			if(obj = o_on(id, mtmp->minvent))
			    break;
		}
	return(obj);
}

static long
get_cost(obj)
register struct obj *obj;
{
	register long tmp;

	tmp = (long) getprice(obj);
	if (!tmp) tmp = 5L;
	if (ANGRY(shopkeeper) || 
		(pl_character[0] == 'T' && u.ulevel < (MAXULEV/2))
#ifdef SHIRT
	    || (uarmu && !uarm) /* wearing just a Hawaiian shirt */
#endif
	   )
		tmp += tmp/3L;
	if (ACURR(A_CHA) > 18)		tmp /= 2L;
	else if (ACURR(A_CHA) > 17)	tmp = (tmp * 2L)/3L;
	else if (ACURR(A_CHA) > 15)	tmp = (tmp * 3L)/4L;
	else if (ACURR(A_CHA) < 6)	tmp *= 2L;
	else if (ACURR(A_CHA) < 8)	tmp = (tmp * 3L)/2L;
	else if (ACURR(A_CHA) < 11)	tmp = (tmp * 4L)/3L;
	if (!tmp) return 1;
	return(tmp);
}


/* called in hack.c when we pickup an object */
void
addtobill(obj, ininv)
register struct obj *obj;
register boolean ininv;
{
	register struct bill_x *bp;
	char	buf[40];
	if(!shopkeeper || !inhishop(shopkeeper)) return;

	if(!costly_spot(obj->ox,obj->oy) ||	/* either pickup or kick */
		onbill(obj) /* perhaps we threw it away earlier */
	      ) return;
	if(ESHK(shopkeeper)->billct == BILLSZ) {
		You("got that for free!");
		return;
	}
	/* To recognize objects the shopkeeper is not interested in. -dgk
	 */
	if (obj->no_charge) {
		obj->no_charge = 0;
		return;
	}
	bp = &bill[ESHK(shopkeeper)->billct];
	bp->bo_id = obj->o_id;
	bp->bquan = obj->quan;
	bp->useup = 0;
	bp->price = get_cost(obj);
	Strcpy(buf, "\"For you, ");
	if (ANGRY(shopkeeper)) Strcat(buf, "scum ");
	else {
	    switch(rnd(4)
#ifdef HARD
		   + u.udemigod
#endif
				) {
		case 1:	Strcat(buf, "good");
			break;
		case 2:	Strcat(buf, "honored");
			break;
		case 3:	Strcat(buf, "most gracious");
			break;
		case 4:	Strcat(buf, "esteemed");
			break;
		case 5: if (u.ualigntyp == U_CHAOTIC) Strcat(buf, "un");
			Strcat(buf, "holy");
			break;
	    }
#ifdef POLYSELF
	    if(!is_human(uasmon)) Strcat(buf, " creature");
	    else
#endif
		Strcat(buf, (flags.female) ? " lady" : " sir");
	}
	obj->dknown = 1; /* after all, the shk is telling you what it is */
	if(ininv) {
		obj->quan = 1; /* fool xname() into giving singular */
		pline("%s; only %d %s %s.\"", buf, bp->price,
			(bp->bquan > 1) ? "per" : "for this", xname(obj));
		obj->quan = bp->bquan;
	} else pline("The %s will cost you %d zorkmid%s%s.",
			xname(obj), bp->price, plur((long)bp->price),
			(bp->bquan > 1) ? " each" : "");
	ESHK(shopkeeper)->billct++;
	obj->unpaid = 1;
}

void
splitbill(obj, otmp)
register struct obj *obj, *otmp;
{
	/* otmp has been split off from obj */
	register struct bill_x *bp;
	register int tmp;
	bp = onbill(obj);
	if(!bp) {
		impossible("splitbill: not on bill?");
		return;
	}
	if(bp->bquan < otmp->quan) {
		impossible("Negative quantity on bill??");
	}
	if(bp->bquan == otmp->quan) {
		impossible("Zero quantity on bill??");
	}
	bp->bquan -= otmp->quan;

	if(ESHK(shopkeeper)->billct == BILLSZ) otmp->unpaid = 0;
	else {
		tmp = bp->price;
		bp = &bill[ESHK(shopkeeper)->billct];
		bp->bo_id = otmp->o_id;
		bp->bquan = otmp->quan;
		bp->useup = 0;
		bp->price = tmp;
		ESHK(shopkeeper)->billct++;
	}
}

static void
subfrombill(obj)
register struct obj *obj;
{
	register struct bill_x *bp;

	if((bp = onbill(obj)) != 0) {
		register struct obj *otmp;

		obj->unpaid = 0;
		if(bp->bquan > obj->quan){
			otmp = newobj(0);
			*otmp = *obj;
			bp->bo_id = otmp->o_id = flags.ident++;
			otmp->quan = (bp->bquan -= obj->quan);
			otmp->owt = 0;	/* superfluous */
			otmp->onamelth = 0;
			bp->useup = 1;
			otmp->nobj = billobjs;
			billobjs = otmp;
			return;
		}
		ESHK(shopkeeper)->billct--;
		*bp = bill[ESHK(shopkeeper)->billct];
		return;
	} else if (obj->unpaid) {
		impossible("subfrombill: unpaid object not on bill");
		obj->unpaid = 0;
	}
}

void
sellobj(obj)
register struct obj *obj;
{
	long ltmp;

	if(!costly_spot(u.ux,u.uy))
		return;
	if(obj->unpaid) {
		subfrombill(obj);
		return;
	}
	/* you dropped something of your own - probably want to sell it */
	if(shopkeeper->msleep || !shopkeeper->mcanmove || !inhishop(shopkeeper))
		return;
	ltmp = (long) getprice(obj) * (long) obj->quan;
	if (ANGRY(shopkeeper) || (pl_character[0] == 'T' && u.ulevel < (MAXULEV/2))
#ifdef SHIRT
	    || (uarmu && !uarm) /* wearing just a Hawaiian shirt */
#endif
	   ) {
		ltmp /= 3L;
		NOTANGRY(shopkeeper) = 1;
	} else	ltmp /= 2L;
	if(ESHK(shopkeeper)->billct == BILLSZ
	   || !saleable(rooms[ESHK(shopkeeper)->shoproom].rtype-SHOPBASE, obj)
	   || obj->olet == BALL_SYM || ltmp == 0L
	   || (obj->olet == FOOD_SYM && obj->oeaten)) {
		pline("%s seems not interested.", Monnam(shopkeeper));
		obj->no_charge = 1;
		return;
	}
	if(ESHK(shopkeeper)->robbed) {
		if((ESHK(shopkeeper)->robbed -= ltmp) < 0L)
			ESHK(shopkeeper)->robbed = 0L;
verbalize("Thank you for your contribution to restock this recently plundered shop.");
		return;
	}
	if(ltmp > shopkeeper->mgold)
		ltmp = shopkeeper->mgold;
	pay(-ltmp, shopkeeper);
	if(!ltmp) {
		pline("%s gladly accepts %s but cannot pay you at present.",
			Monnam(shopkeeper), doname(obj));
			obj->no_charge = 1;
	} else
	You("sold %s for %ld gold piece%s.", doname(obj), ltmp,
		plur(ltmp));
}

int
doinvbill(mode)
int mode;		/* 0: deliver count 1: paged */
{
	register struct bill_x *bp;
	register struct obj *obj;
#ifdef __GNULINT__
	long totused, thisused = 0L;
/* possibly a bug in the GCC; clearly thisused is always set before use */
#else
	long totused, thisused;
#endif
	char buf[BUFSZ];

	if(mode == 0) {
	    register int cnt = 0;

	    if(shopkeeper)
		for(bp = bill; bp - bill < ESHK(shopkeeper)->billct; bp++)
		    if(bp->useup ||
		      ((obj = bp_to_obj(bp)) && obj->quan < bp->bquan))
			cnt++;
	    return(cnt);
	}

	if(!shopkeeper) {
		impossible("doinvbill: no shopkeeper?");
		return(0);
	}

	set_pager(0);
	if(page_line("Unpaid articles already used up:") || page_line(""))
	    goto quit;

	totused = 0L;
	for(bp = bill; bp - bill < ESHK(shopkeeper)->billct; bp++) {
	    obj = bp_to_obj(bp);
	    if(!obj) {
		impossible("Bad shopkeeper administration.");
		goto quit;
	    }
	    if(bp->useup || bp->bquan > obj->quan) {
		register int cnt, oquan, uquan;

		oquan = obj->quan;
		uquan = (bp->useup ? bp->bquan : bp->bquan - oquan);
		thisused = bp->price * uquan;
		totused += thisused;
		obj->quan = uquan;		/* cheat doname */
		Sprintf(buf, "x -  %s", doname(obj));
		obj->quan = oquan;		/* restore value */
		for(cnt = 0; buf[cnt]; cnt++);
		while(cnt < 50)
			buf[cnt++] = ' ';
		Sprintf(&buf[cnt], " %5ld zorkmid%s", thisused, plur(thisused));
		if(page_line(buf))
			goto quit;
	    }
	}
	Sprintf(buf, "Total:%50ld zorkmid%s", totused, plur(totused));
	if(page_line("") || page_line(buf))
		goto quit;
	set_pager(1);
	return(0);
quit:
	set_pager(2);
	return(0);
}

#define HUNGRY	2
static int
getprice(obj)
register struct obj *obj;
{
	register int tmp = objects[obj->otyp].oc_cost;

	switch(obj->olet) {
	case AMULET_SYM:
		if(obj->otyp == AMULET_OF_YENDOR) {
			/* don't let the player get rich selling fakes */
			tmp = (obj->spe < 0 ? 0 : 3500);
		}
		break;
	case FOOD_SYM:
		/* simpler hunger check, (2-4)*cost */
		if (u.uhs >= HUNGRY) tmp *= u.uhs;
		if (obj->oeaten) tmp = eaten_stat(tmp, obj); /* partly eaten */
		break;
	case WAND_SYM:
		if (obj->spe == -1) tmp = 0;
		break;
	case POTION_SYM:
		if (obj->otyp == POT_WATER && !obj->blessed && !obj->cursed)
			tmp = 0;
		break;
	case ARMOR_SYM:
	case WEAPON_SYM:
		if (obj->spe > 0) tmp += 10 * obj->spe;
		break;
	case CHAIN_SYM:
		pline("Strange... carrying a chain?");
		break;
	}
	return(tmp);
}

int
shkcatch(obj)
register struct obj *obj;
{
	register struct monst *shkp = shopkeeper;

	if(obj->otyp != PICK_AXE) return(0);
	if(u.uinshop && shkp && shkp->mcanmove && !shkp->msleep &&
	    inroom(u.ux+u.dx, u.uy+u.dy) + 1 == u.uinshop &&
	    shkp->mx == ESHK(shkp)->shk.x && shkp->my == ESHK(shkp)->shk.y &&
	    u.ux == ESHK(shkp)->shd.x && u.uy == ESHK(shkp)->shd.y) {
		pline("%s nimbly catches the %s.", Monnam(shkp), xname(obj));
		obj->nobj = shkp->minvent;
		shkp->minvent = obj;
		subfrombill(obj);
		return(1);
	}
	return(0);
}

/*
 * shk_move: return 1: he moved  0: he didn't  -1: let m_move do it  -2: died
 */
int
shk_move(shkp)
register struct monst *shkp;
{
	register xchar gx,gy,omx,omy;
	register int udist;
	register schar appr;
	int z;
	schar shkroom;
	boolean uondoor = FALSE, satdoor, avoid = FALSE, badinv;

	omx = shkp->mx;
	omy = shkp->my;

	if((udist = dist(omx,omy)) < 3 &&
	   (shkp->data != &mons[PM_GRID_BUG] || (omx==u.ux || omy==u.uy))) {
		if(ANGRY(shkp)) {
			if(Displaced)
			  Your("displaced image doesn't fool %s!",
				Monnam(shkp));
			(void) mattacku(shkp);
			return(0);
		}
		if(ESHK(shkp)->following) {
			if(strncmp(ESHK(shkp)->customer, plname, PL_NSIZ)) {
			    pline("\"Hello, %s!  I was looking for %s.\"",
				    plname, ESHK(shkp)->customer);
				    ESHK(shkp)->following = 0;
			    return(0);
			}
			if(moves > followmsg+4) {
			    pline("\"Hello, %s!  Didn't you forget to pay?\"",
				    plname);
			    followmsg = moves;
#ifdef HARD
			    if (!rn2(4)) {
	    pline ("%s doesn't like customers who don't pay.", Monnam(shkp));
				NOTANGRY(shkp) = 0;
			    }
#endif
			}
			if(udist < 2)
			    return(0);
		}
	}

	shkroom = inroom(omx,omy);
	appr = 1;
	gx = ESHK(shkp)->shk.x;
	gy = ESHK(shkp)->shk.y;
	satdoor = (gx == omx && gy == omy);
	if(ESHK(shkp)->following || ((z = holetime()) >= 0 && z*z <= udist)){
		gx = u.ux;
		gy = u.uy;
		if(shkroom < 0 || shkroom != inroom(u.ux,u.uy))
		    if(udist > 4)
			return(-1);	/* leave it to m_move */
	} else if(ANGRY(shkp)) {
		long saveBlind = Blinded;
		struct obj *saveUblindf = ublindf;
		Blinded = 0;
		ublindf = (struct obj *)0;
		if(shkp->mcansee && !Invis && cansee(omx,omy)) {
			gx = u.ux;
			gy = u.uy;
		}
		Blinded = saveBlind;
		ublindf = saveUblindf;
		avoid = FALSE;
	} else {
#define	GDIST(x,y)	(dist2(x,y,gx,gy))
		if(Invis)
		    avoid = FALSE;
		else {
		    uondoor = (u.ux == ESHK(shkp)->shd.x &&
				u.uy == ESHK(shkp)->shd.y);
		    if(uondoor) {
			if((ESHK(shkp)->billct || ESHK(shkp)->debit) 
					&& inhishop(shkp))
			    pline(NOTANGRY(shkp) ?
				"\"Hello, %s!  Will you please pay before leaving?\"" :
				"\"Hey, %s!  Don't leave without paying!\"",
				plname);
			badinv = (!!carrying(PICK_AXE));
			if(satdoor && badinv)
			    return(0);
			avoid = !badinv;
		    } else {
			avoid = (u.uinshop && dist(gx,gy) > 8);
			badinv = FALSE;
		    }

		    if(((!ESHK(shkp)->robbed && !ESHK(shkp)->billct &&
				!ESHK(shkp)->debit) || avoid)
			&& GDIST(omx,omy) < 3) {
			if(!badinv && !online(omx,omy))
			    return(0);
			if(satdoor)
			    appr = gx = gy = 0;
		    }
		}
	}
	
	return(move_special(shkp,shkroom,appr,uondoor,avoid,omx,omy,gx,gy));
}

#endif /* OVLB */
#ifdef OVL0

int
online(x,y)		/*	New version to speed things up.
			 *	Compiler dependant, may not always work.
			 */
register xchar x, y;
{
	return((x-=u.ux) == 0 || (y-=u.uy) == 0 || x == y || (x+=y) == 0);
}

/*			Original version, just in case...
 *online(x,y) {
 *	return(x==u.ux || y==u.uy || (x-u.ux)*(x-u.ux) == (y-u.uy)*(y-u.uy));
 *}
 */

#endif /* OVL0 */
#ifdef OVLB

/* for use in levl_follower (mondata.c) */
boolean
is_fshk(mtmp)
register struct monst *mtmp;
{
	return(mtmp->isshk && ESHK(mtmp)->following);
}

/* He is digging in the shop. */
void
shopdig(fall)
register int fall;
{
    if(!shopkeeper) return;
    if(!inhishop(shopkeeper)) {
	if (pl_character[0] == 'K') adjalign(-sgn(u.ualigntyp));
	return;
    }

    if(!fall) {
	if(u.utraptype == TT_PIT)
	    pline("\"Be careful, %s, or you might fall through the floor.\"",
		flags.female ? "madam" : "sir");
	else
	    pline("\"%s, do not damage the floor here!\"",
			flags.female ? "Madam" : "Sir");
	if (pl_character[0] == 'K') adjalign(-sgn(u.ualigntyp));
    } else 
	if(!um_dist(shopkeeper->mx, shopkeeper->my, 5) &&
	      !shopkeeper->msleep && shopkeeper->mcanmove &&
	      (ESHK(shopkeeper)->billct || ESHK(shopkeeper)->debit)) {
	    register struct obj *obj, *obj2;

	    if(dist(shopkeeper->mx, shopkeeper->my) > 2) {
		mnexto(shopkeeper);
		/* for some reason he can't come next to you */
		if(dist(shopkeeper->mx, shopkeeper->my) > 2) {
		    pline("%s curses you in anger and frustration!",
					shkname(shopkeeper));
		    NOTANGRY(shopkeeper) = 0;
		    return;
		} else pline("%s leaps, and grabs your backpack!",
					shkname(shopkeeper));
	    } else pline("%s grabs your backpack!", shkname(shopkeeper));

	    for(obj = invent; obj; obj = obj2) {
		obj2 = obj->nobj;
		if(obj->owornmask) continue;
#ifdef WALKIES
		if(obj->otyp == LEASH && obj->leashmon) continue;
#endif
		freeinv(obj);
		obj->nobj = shopkeeper->minvent;
		shopkeeper->minvent = obj;
		subfrombill(obj);
	    }
    }
}

#ifdef KOPS
static int
makekops(mm)		/* returns the number of (all types of) Kops  made */
coord *mm;
{
	register int cnt = dlevel + rnd(5);
	register int scnt = (cnt / 3) + 1;	/* at least one sarge */
	register int lcnt = (cnt / 6);		/* maybe a lieutenant */
	register int kcnt = (cnt / 9);		/* and maybe a kaptain */

	while(cnt--) {
	    enexto(mm, mm->x, mm->y, &mons[PM_KEYSTONE_KOP]);
	    (void) makemon(&mons[PM_KEYSTONE_KOP], mm->x, mm->y);
	}
	while(scnt--) {
	    enexto(mm, mm->x, mm->y, &mons[PM_KOP_SERGEANT]);
	    (void) makemon(&mons[PM_KOP_SERGEANT], mm->x, mm->y);
	}
	while(lcnt--) {
	    enexto(mm, mm->x, mm->y, &mons[PM_KOP_LIEUTENANT]);
	    (void) makemon(&mons[PM_KOP_LIEUTENANT], mm->x, mm->y);
	}
	while(kcnt--) {
	    enexto(mm, mm->x, mm->y, &mons[PM_KOP_KAPTAIN]);
	    (void) makemon(&mons[PM_KOP_KAPTAIN], mm->x, mm->y);
	}
	return(cnt + scnt + lcnt + kcnt);
}
#endif

#endif /* OVLB */
#ifdef OVL1

boolean
in_shop(x,y)
register int x, y;
{
	register int roomno = inroom(x, y);

	if (roomno < 0) return(FALSE);
	return (IS_SHOP(rooms[roomno]));
}

#endif /* OVL1 */
#ifdef OVLB

void
pay_for_door(x,y,dmgstr)
int x, y;
const char *dmgstr;
{
	struct monst *mtmp;
	int roomno = inroom(x, y);
	long damage;
	boolean uinshp = in_shop(u.ux, u.uy);

	/* make sure this function is not used in the wrong place */
	if(!(IS_DOOR(levl[x][y].typ) && in_shop(x, y))) return;

	findshk(roomno);

	if(!shopkeeper) return;

	/* not the best introduction to the shk... */
	(void) strncpy(ESHK(shopkeeper)->customer,plname,PL_NSIZ);

	/* if he is already on the war path, be sure it's all out */
	if(ANGRY(shopkeeper) || ESHK(shopkeeper)->following) {
		NOTANGRY(shopkeeper) = 0;
		ESHK(shopkeeper)->following = 1;
		return;
	}

	/* if he's not in his shop.. */
	if(!in_shop(shopkeeper->mx ,shopkeeper->my)) {
		if(!cansee(shopkeeper->mx, shopkeeper->my)) return;
		goto gethim;
	}

	if(uinshp) {
		if(um_dist(shopkeeper->mx, shopkeeper->my, 1) &&
		       !um_dist(shopkeeper->mx, shopkeeper->my, 3)) { 
		    pline("%s leaps towards you!", shkname(shopkeeper));
		    mnexto(shopkeeper);
		}
		if(um_dist(shopkeeper->mx, shopkeeper->my, 1)) goto gethim;
	} else {
	    /* if a !shopkeeper shows up at the door, move him */
	    if(MON_AT(x, y) && (mtmp = m_at(x, y)) != shopkeeper) {
		if(flags.soundok) {
		    You("hear an angry voice:");
		    verbalize("Out of my way, scum!");
		    (void) fflush(stdout);
#if defined(SYSV) || defined(ULTRIX) || defined(VMS)
		    (void)
#endif
#if defined(UNIX) || defined(VMS)
			sleep(1);
#endif
		}
		mnearto(mtmp, x, y, FALSE);
	    }

	    /* make shk show up at the door */
	    remove_monster(shopkeeper->mx, shopkeeper->my);
	    place_monster(shopkeeper, x, y);
	    pmon(shopkeeper);
	}

	if(!strcmp(dmgstr, "destroy")) damage = 400L;
	else damage = (long)(ACURR(A_STR) > 18) ? 400 : 20 * ACURR(A_STR);

	if((um_dist(x, y, 1) && !uinshp) || 
			(u.ugold + ESHK(shopkeeper)->credit) < damage 
				|| !rn2(50)) {
		if(um_dist(x, y, 1) && !uinshp) {
		    pline("%s shouts:", shkname(shopkeeper));
		    pline("\"Who dared %s my door?\"", dmgstr);
		} 
		else
gethim:
		    pline("\"How dare you %s my door?\"", dmgstr);

		NOTANGRY(shopkeeper) = 0;
		ESHK(shopkeeper)->following = 1;
		return;
	}

	if(Invis) Your("invisibility does not fool %s!", shkname(shopkeeper));
	pline("\"Cad!  You did %ld zorkmids worth of damage!\"  Pay? ", damage);
	if(yn() != 'n') {
		damage = check_credit(damage, shopkeeper);
		u.ugold -= damage;
		shopkeeper->mgold += damage;
		flags.botl = 1;
		pline("Mollified, %s accepts your restitution.",
			shkname(shopkeeper));
		/* move shk back to his home loc */
		home_shk(shopkeeper);
		NOTANGRY(shopkeeper) = 1;
	} else {
		verbalize("Oh, yes!  You'll pay!");
		ESHK(shopkeeper)->following = 1;
		NOTANGRY(shopkeeper) = 0;
		adjalign(-sgn(u.ualigntyp));
	}
}

/* called in dokick.c when we kick an object in a store */
boolean
costly_spot(x, y)
register int x, y;
{
	register struct monst *shkp = shopkeeper;
	
	if(!shkp) return(FALSE);

	return(in_shop(x, y) && levl[x][y].typ != DOOR &&
		!(x == ESHK(shkp)->shk.x && y == ESHK(shkp)->shk.y));
}

#ifdef KOPS
static void
kops_gone()
{
	register int cnt = 0;
	register struct monst *mtmp, *mtmp2;

	/* turn off automatic resurrection of kops */
	allow_kops = FALSE;

	for(mtmp = fmon; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if(mtmp->data->mlet == S_KOP) {
			mongone(mtmp);
			cnt++;
		}
	}
	if(cnt) pline("The Kops (disappointed) disappear into thin air.");
	allow_kops = TRUE;
}
#endif

static long
cost_per_charge(otmp)
register struct obj *otmp;
{
	register long tmp = get_cost(otmp);

	/* The idea is to make the exhaustive use of */
	/* an unpaid item more expensive than buying */
	/* outright.				     */
	if(otmp->otyp == MAGIC_LAMP) {			 /* 1 */
		tmp += (tmp/3L);
	} else if(otmp->otyp == MAGIC_MARKER) {  	 /* 70 - 100 */
		/* no way to determine in advance   */
		/* how many charges will be wasted. */
		/* so, arbitrarily, one half of the */
		/* price per use.		    */
		tmp = (tmp/2L);
	} else if(otmp->otyp == BAG_OF_TRICKS) { 	 /* 1 - 20 */
		tmp = (tmp/5L);
	} else if(otmp->otyp == CRYSTAL_BALL ||  	 /* 1 - 5 */
		  otmp->otyp == LAMP ||	                 /* 1-10 */
#ifdef MUSIC
		 (otmp->otyp >= MAGIC_FLUTE &&
		  otmp->otyp <= DRUM_OF_EARTHQUAKE) || 	 /* 5 - 9 */
#endif
	  	  otmp->olet == WAND_SYM) {		 /* 3 - 11 */
		if(otmp->spe > 1) tmp = (tmp/4L);
	}
	else return(0L);
	return(tmp);
}

/* for using charges of unpaid objects */
void
check_unpaid(otmp)
register struct obj *otmp;
{
	if(!in_shop(u.ux, u.uy)) return;
	
	if(otmp->spe <= 0) return;

	if(otmp->unpaid) {
		ESHK(shopkeeper)->debit += cost_per_charge(otmp);
	}
}

boolean
block_door(x,y)  	/* used in domove to block diagonal shop-exit */
register int x, y;
{
	register int roomno = inroom(x, y);

	if(!in_shop(u.ux, u.uy)) return(FALSE);

	if(!IS_DOOR(levl[x][y].typ)) return(FALSE);

	if(roomno != inroom(u.ux,u.uy)) return(FALSE);

	findshk(roomno);

	if(inhishop(shopkeeper)
	    && shopkeeper->mx == ESHK(shopkeeper)->shk.x
	    && shopkeeper->my == ESHK(shopkeeper)->shk.y
	    /* Actually, the shk should be made to block _any_ */
	    /* door, including a door the player digs, if the  */
	    /* shk is within a 'jumping' distance.	       */
	    && ESHK(shopkeeper)->shd.x == x && ESHK(shopkeeper)->shd.y == y
	    && shopkeeper->mcanmove && !shopkeeper->msleep
	    && (ESHK(shopkeeper)->debit || ESHK(shopkeeper)->billct ||
		ESHK(shopkeeper)->robbed)) {
		pline("%s%s blocks your way!", shkname(shopkeeper),
				Invis ? " senses your motion and" : "");
		return(TRUE);
	}
	return(FALSE);
}

boolean
block_entry(x,y)  	/* used in domove to block diagonal shop-entry */
register int x, y;
{
	register int sx, sy, roomno = inroom(x, y);

	if(roomno != inroom(u.ux,u.uy)) return(FALSE);

	if(!(in_shop(u.ux, u.uy) && IS_DOOR(levl[u.ux][u.uy].typ) &&
		levl[u.ux][u.uy].doormask == D_BROKEN)) return(FALSE);

	findshk(roomno);
	if(!inhishop(shopkeeper)) return(FALSE);

	if(ESHK(shopkeeper)->shd.x != u.ux || ESHK(shopkeeper)->shd.y != u.uy)
		return(FALSE);

	sx = ESHK(shopkeeper)->shk.x;
	sy = ESHK(shopkeeper)->shk.y;

	if(shopkeeper->mx == sx && shopkeeper->my == sy
	    	&& shopkeeper->mcanmove && !shopkeeper->msleep
	    	&& in_shop(x, y)
	        && (x == sx-1 || x == sx+1 || y == sy-1 || y == sy+1)  
	    	&& (Invis || carrying(PICK_AXE))
          ) {
		pline("%s%s blocks your way!", shkname(shopkeeper),
				Invis ? " senses your motion and" : "");
		return(TRUE);
	}
	return(FALSE);
}

#endif /* OVLB */
