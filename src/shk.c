/*	SCCS Id: @(#)shk.c	3.1	93/05/19	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"

/*#define DEBUG*/

#define PAY_SOME    2
#define PAY_BUY     1
#define PAY_CANT    0	/* too poor */
#define PAY_SKIP  (-1)
#define PAY_BROKE (-2)

#ifdef KOPS
STATIC_DCL void FDECL(makekops, (coord *));
STATIC_DCL void FDECL(call_kops, (struct monst *,BOOLEAN_P));
# ifdef OVLB
static void FDECL(kops_gone, (BOOLEAN_P));
# endif /* OVLB */
#endif /* KOPS */

#define IS_SHOP(x)	(rooms[x].rtype >= SHOPBASE)

extern const struct shclass shtypes[];	/* defined in shknam.c */

STATIC_VAR NEARDATA long int followmsg;	/* last time of follow message */

STATIC_DCL void FDECL(setpaid, (struct monst *));
STATIC_DCL long FDECL(addupbill, (struct monst *));
STATIC_DCL void FDECL(pacify_shk, (struct monst *));

#ifdef OVLB

static void FDECL(clear_unpaid,(struct obj *));
static struct bill_x *FDECL(onbill, (struct obj *, struct monst *, BOOLEAN_P));
static long FDECL(check_credit, (long, struct monst *));
static void FDECL(pay, (long, struct monst *));
static long FDECL(get_cost, (struct obj *, struct monst *));
static long FDECL(set_cost, (struct obj *, struct monst *));
static const char *FDECL(shk_embellish, (struct obj *, long));
static long FDECL(cost_per_charge, (struct obj *));
static long FDECL(cheapest_item, (struct monst *));
static int FDECL(dopayobj, (struct monst *, struct bill_x *,
			    struct obj **, int, BOOLEAN_P));
static long FDECL(stolen_container, (struct obj *, struct monst *, long,
				     BOOLEAN_P));
static long FDECL(getprice, (struct obj *));
static struct obj *FDECL(bp_to_obj, (struct bill_x *));
static boolean FDECL(inherits, (struct monst *, int, BOOLEAN_P));
static struct monst *FDECL(next_shkp, (struct monst *, BOOLEAN_P));
static boolean NDECL(angry_shk_exists);
static void FDECL(rile_shk, (struct monst *));
static void FDECL(remove_damage, (struct monst *, BOOLEAN_P));
static void FDECL(sub_one_frombill, (struct obj *, struct monst *));
static void FDECL(add_one_tobill, (struct obj *, BOOLEAN_P));
static void FDECL(dropped_container, (struct obj *, struct monst *,
				      BOOLEAN_P));
static void FDECL(bill_box_content, (struct obj *, BOOLEAN_P, BOOLEAN_P,
				     struct monst *));
static void FDECL(shk_names_obj, (struct obj *));

/*
	invariants: obj->unpaid iff onbill(obj) [unless bp->useup]
		obj->quan <= bp->bquan
 */

static struct monst *
next_shkp(shkp, withbill)
register struct monst *shkp;
register boolean withbill;
{
	for (; shkp; shkp = shkp->nmon)
	    if (shkp->isshk)
		if (ESHK(shkp)->billct || !withbill) break;

	if (shkp) {
	    if (NOTANGRY(shkp)) {
		if (ESHK(shkp)->surcharge) pacify_shk(shkp);
	    } else {
		if (!ESHK(shkp)->surcharge) rile_shk(shkp);
	    }
	}
	return(shkp);
}

char *
shkname(mtmp)				/* called in do_name.c */
register struct monst *mtmp;
{
	return(ESHK(mtmp)->shknam);
}

void
shkgone(mtmp)				/* called in mon.c */
register struct monst *mtmp;
{
	register struct eshk *eshk = ESHK(mtmp);

	if(on_level(&(eshk->shoplevel), &u.uz)) {
		remove_damage(mtmp, TRUE);
		rooms[eshk->shoproom - ROOMOFFSET].resident
						  = (struct monst *)0;
		if(!search_special(ANY_SHOP))
		    level.flags.has_shop = 0;
	}
	/* make sure bill is set only when the
	 * dead shk is the resident shk.	*/
	if(*u.ushops == eshk->shoproom) {
	    setpaid(mtmp);
	    /* dump core when referenced */
	    ESHK(mtmp)->bill_p = (struct bill_x *) -1000;
	    u.ushops[0] = '\0';
	}
}

void
set_residency(shkp, zero_out)
register struct monst *shkp;
register boolean zero_out;
{
	if (on_level(&(ESHK(shkp)->shoplevel), &u.uz))
	    rooms[ESHK(shkp)->shoproom - ROOMOFFSET].resident =
		(zero_out)? (struct monst *)0 : shkp;
}

void
replshk(mtmp,mtmp2)
register struct monst *mtmp, *mtmp2;
{
	rooms[ESHK(mtmp2)->shoproom - ROOMOFFSET].resident = mtmp2;
	if (inhishop(mtmp) && *u.ushops == ESHK(mtmp)->shoproom) {
		ESHK(mtmp2)->bill_p = &(ESHK(mtmp2)->bill[0]);
	}
}

/* do shopkeeper specific structure munging -dlc */
void
restshk(mtmp, ghostly)
register struct monst *mtmp;
boolean ghostly;
{
    if(u.uz.dlevel) {
	if(ESHK(mtmp)->bill_p != (struct bill_x *) -1000)
	    ESHK(mtmp)->bill_p = &(ESHK(mtmp)->bill[0]);
	/* shoplevel can change as dungeons move around */
	/* savebones guarantees that non-homed shk's will be gone */
	if (ghostly)
	    assign_level(&(ESHK(mtmp)->shoplevel), &u.uz);
    }
}

/* Clear the unpaid bit on all of the objects in the list. */
static void
clear_unpaid(list)
register struct obj *list;
{
    while (list) {
	if (Has_contents(list)) clear_unpaid(list->cobj);
	list->unpaid = 0;
	list = list->nobj;
    }
}

STATIC_OVL void
setpaid(shkp)	/* either you paid or left the shop or the shopkeeper died */
register struct monst *shkp;
{
	register struct obj *obj;
	register struct monst *mtmp;

	clear_unpaid(invent);
	clear_unpaid(fobj);
	clear_unpaid(level.buriedobjlist);
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		clear_unpaid(mtmp->minvent);
	for(mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon)
		clear_unpaid(mtmp->minvent);

	while ((obj = billobjs) != 0) {
		billobjs = obj->nobj;
		dealloc_obj(obj);
	}
	if(shkp) {
		ESHK(shkp)->billct = 0;
		ESHK(shkp)->credit = 0L;
		ESHK(shkp)->debit = 0L;
		ESHK(shkp)->loan = 0L;
	}
}

STATIC_OVL long
addupbill(shkp)
register struct monst *shkp;
{
	register int ct = ESHK(shkp)->billct;
	register struct bill_x *bp = ESHK(shkp)->bill_p;
	register long total = 0L;

	while(ct--){
		total += bp->price * bp->bquan;
		bp++;
	}
	return(total);
}

#endif /* OVLB */
#ifdef OVL1

#ifdef KOPS
STATIC_OVL void
call_kops(shkp, nearshop)
register struct monst *shkp;
register boolean nearshop;
{
	/* Keystone Kops srt@ucla */
	register boolean nokops;

	if(!shkp) return;

	if (flags.soundok)
	    pline("An alarm sounds!");

	nokops = ((mons[PM_KEYSTONE_KOP].geno & (G_GENOD | G_EXTINCT)) &&
		  (mons[PM_KOP_SERGEANT].geno & (G_GENOD | G_EXTINCT)) &&
		  (mons[PM_KOP_LIEUTENANT].geno & (G_GENOD | G_EXTINCT)) &&
		  (mons[PM_KOP_KAPTAIN].geno & (G_GENOD | G_EXTINCT)));

	if(!angry_guards(!flags.soundok) && nokops) {
	    if(flags.verbose && flags.soundok)
		pline("But no one seems to respond to it.");
	    return;
	}

	if(nokops) return;

	{
	    coord mm;

	    if (nearshop) {
		/* Create swarm around you, if you merely "stepped out" */
		if (flags.verbose)
		    pline("The Keystone Kops appear!");
		mm.x = u.ux;
		mm.y = u.uy;
		makekops(&mm);
		return;
	    }
	    if (flags.verbose)
		 pline("The Keystone Kops are after you!");
	    /* Create swarm near down staircase (hinders return to level) */
	    mm.x = xdnstair;
	    mm.y = ydnstair;
	    makekops(&mm);
	    /* Create swarm near shopkeeper (hinders return to shop) */
	    mm.x = shkp->mx;
	    mm.y = shkp->my;
	    makekops(&mm);
	}
}
#endif	/* KOPS */

/* x,y is strictly inside shop */
char
inside_shop(x, y)
register xchar x, y;
{
	register char rno;

	rno = levl[x][y].roomno;
	if ((rno < ROOMOFFSET) || levl[x][y].edge || !IS_SHOP(rno-ROOMOFFSET))
	    return(NO_ROOM);
	else
	    return(rno);
}

void
u_left_shop(leavestring, newlev)
register char *leavestring;
register boolean newlev;
{
	register struct monst *shkp;
	register struct eshk *eshkp;
	register long total;

	/*
	 * IF player
	 * ((didn't leave outright) AND
	 *  ((he is now strictly-inside the shop) OR
	 *   (he wasn't strictly-inside last turn anyway)))
	 * THEN (there's nothing to do, so just return)
	 */
	if(!*leavestring &&
	   (!levl[u.ux][u.uy].edge || levl[u.ux0][u.uy0].edge))
	    return;

	shkp = shop_keeper(*u.ushops0);

	if(!shkp || !inhishop(shkp))
				/* shk died, teleported, changed levels... */
	    return;

	eshkp = ESHK(shkp);

	if(!eshkp->billct && !eshkp->debit)	/* bill is settled */
	    return;

	if (!*leavestring && shkp->mcanmove && !shkp->msleep) {
	    /*
	     * Player just stepped onto shop-boundary (known from above logic).
	     * Try to intimidate him into paying his bill
	     */
	    verbalize(NOTANGRY(shkp) ?
		      "%s!  Please pay before leaving." :
		      "%s!  Don't you leave without paying!",
		      plname);
	    return;
	}
	total = (addupbill(shkp) + eshkp->debit);
	if (eshkp->credit >= total) {
	    Your("credit of %ld zorkmid%s is used to cover your shopping bill.",
		 eshkp->credit, plur(eshkp->credit));
	    total = 0L;		/* credit gets cleared by setpaid() */
	} else {
	    You("escaped the shop without paying!");
	    total -= eshkp->credit;
	}
	setpaid(shkp);
	if (!total) return;

	/* by this point, we know an actual robbery has taken place */
	eshkp->robbed += total;
	You("stole %ld zorkmid%s worth of merchandise.",
	    total, plur(total));
	if (pl_character[0] != 'R') /* stealing is unlawful */
	    adjalign(-sgn(u.ualign.type));

	hot_pursuit(shkp);
#ifdef KOPS
	call_kops(shkp, (!newlev && levl[u.ux0][u.uy0].edge));
#else
	(void) angry_guards(FALSE);
#endif
}

void
u_entered_shop(enterstring)
register char *enterstring;
{

	register int rt;
	register struct monst *shkp;
	register struct eshk *eshkp;
	static const char no_shk[] = "This shop appears to be deserted.";
	static char empty_shops[5];

	if(!*enterstring)
		return;

	if(!(shkp = shop_keeper(*enterstring))) {
	    if (!index(empty_shops, *enterstring) &&
		in_rooms(u.ux, u.uy, SHOPBASE) !=
				  in_rooms(u.ux0, u.uy0, SHOPBASE))
		pline(no_shk);
	    Strcpy(empty_shops, u.ushops);
	    u.ushops[0] = '\0';
	    return;
	}

	eshkp = ESHK(shkp);

	if (!inhishop(shkp)) {
	    /* dump core when referenced */
	    eshkp->bill_p = (struct bill_x *) -1000;
	    if (!index(empty_shops, *enterstring))
		pline(no_shk);
	    Strcpy(empty_shops, u.ushops);
	    u.ushops[0] = '\0';
	    return;
	}

	eshkp->bill_p = &(eshkp->bill[0]);

	if (!eshkp->visitct || strncmpi(eshkp->customer, plname, PL_NSIZ)) {
	    /* You seem to be new here */
	    eshkp->visitct = 0;
	    eshkp->following = 0;
	    (void) strncpy(eshkp->customer,plname,PL_NSIZ);
	    pacify_shk(shkp);
	}

	if (shkp->msleep || !shkp->mcanmove || eshkp->following) /* no dialog */
	    return;

	if (Invis) {
	    pline("%s senses your presence.", shkname(shkp));
	    verbalize("Invisible customers are not welcome!");
	    return;
	}

	rt = rooms[*enterstring - ROOMOFFSET].rtype;

	if (ANGRY(shkp)) {
	    verbalize("So, %s, you dare return to %s %s?!",
		      plname,
		      s_suffix(shkname(shkp)),
		      shtypes[rt - SHOPBASE].name);
	} else if (eshkp->robbed) {
	    pline("%s mutters imprecations against shoplifters.", shkname(shkp));
	} else {
	    verbalize("Hello, %s!  Welcome%s to %s %s!",
		      plname,
		      eshkp->visitct++ ? " again" : "",
		      s_suffix(shkname(shkp)),
		      shtypes[rt - SHOPBASE].name);
	}
	if(carrying(PICK_AXE) != (struct obj *)0 &&
				 /* can't do anything if teleported in */
				 !inside_shop(u.ux, u.uy)) {
	    verbalize(NOTANGRY(shkp) ?
		      "Will you please leave your pick-axe outside?" :
		      "Leave the pick-axe outside.");
	    (void) dochug(shkp);
	}
	return;
}

/*
   Decide whether two unpaid items are mergable; caller is responsible for
   making sure they're unpaid and the same type of object; we check the price
   quoted by the shopkeeper and also that they both belong to the same shk.
 */
boolean same_price(obj1, obj2)
struct obj *obj1, *obj2;
{
	register struct monst *shkp1, *shkp2;
	register struct bill_x *bp1 = 0, *bp2 = 0;
	register boolean are_mergable = FALSE;

	/* look up the first object by finding shk whose bill it's on */
	for (shkp1 = next_shkp(fmon, TRUE); shkp1;
		shkp1 = next_shkp(shkp1, TRUE))
	    if ((bp1 = onbill(obj1, shkp1, TRUE)) != 0) break;
	/* second object is probably owned by same shk; if not, look harder */
	if (shkp1 && (bp2 = onbill(obj2, shkp1, TRUE)) != 0) {
	    shkp2 = shkp1;
	} else {
	    for (shkp2 = next_shkp(fmon, TRUE); shkp2;
		    shkp2 = next_shkp(shkp2, TRUE))
		if ((bp2 = onbill(obj2, shkp2, TRUE)) != 0) break;
	}

	if (!bp1 || !bp2) impossible("same_price: object wasn't on any bill!");
	else are_mergable = (shkp1 == shkp2 && bp1->price == bp2->price);
	return are_mergable;
}

#endif /* OVL1 */
#ifdef OVLB

int
inhishop(mtmp)
register struct monst *mtmp;
{
	return(index(in_rooms(mtmp->mx, mtmp->my, SHOPBASE),
		     ESHK(mtmp)->shoproom) &&
		on_level(&(ESHK(mtmp)->shoplevel), &u.uz));
}

struct monst *
shop_keeper(rmno)
register char rmno;
{
	struct monst *shkp = rmno >= ROOMOFFSET ?
				rooms[rmno - ROOMOFFSET].resident : 0;

	if (shkp) {
	    if (NOTANGRY(shkp)) {
		if (ESHK(shkp)->surcharge) pacify_shk(shkp);
	    } else {
		if (!ESHK(shkp)->surcharge) rile_shk(shkp);
	    }
	}
	return shkp;
}

#ifdef SOUNDS
boolean
tended_shop(sroom)
register struct mkroom *sroom;
{
	register struct monst *mtmp = sroom->resident;

	if (!mtmp)
		return(FALSE);
	else
		return(inhishop(mtmp));
}
#endif	/* SOUNDS */

static struct bill_x *
onbill(obj, shkp, silent)
register struct obj *obj;
register struct monst *shkp;
register boolean silent;
{
	if (shkp) {
		register struct bill_x *bp = ESHK(shkp)->bill_p;
		register int ct = ESHK(shkp)->billct;

		while (--ct >= 0)
		    if (bp->bo_id == obj->o_id) {
			if (!obj->unpaid) pline("onbill: paid obj on bill?");
			return bp;
		    } else bp++;
	}
	if(obj->unpaid & !silent) pline("onbill: unpaid obj not on bill?");
	return (struct bill_x *)0;
}

/* Delete the contents of the given object. */
void
delete_contents(obj)
register struct obj *obj;
{
	register struct obj *curr, *next;

	for (curr = obj->cobj; curr; curr = next) {
		next = curr->nobj;
		obfree(curr, (struct obj *)0);
	}
	obj->cobj = (struct obj *) 0;
}

/* called with two args on merge */
void
obfree(obj, merge)
register struct obj *obj, *merge;
{
	register struct bill_x *bp;
	register struct bill_x *bpm;
	register struct monst *shkp;

	if(obj->oclass == FOOD_CLASS) food_disappears(obj);

	if (obj->cobj) delete_contents(obj);

	shkp = shop_keeper(*u.ushops);

	if ((bp = onbill(obj, shkp, FALSE)) != 0) {
		if(!merge){
			bp->useup = 1;
			obj->unpaid = 0;	/* only for doinvbill */
			obj->nobj = billobjs;
			billobjs = obj;
			return;
		}
		bpm = onbill(merge, shkp, FALSE);
		if(!bpm){
			/* this used to be a rename */
			impossible("obfree: not on bill??");
			return;
		} else {
			/* this was a merger */
			bpm->bquan += bp->bquan;
			ESHK(shkp)->billct--;
#ifdef DUMB
			{
			/* DRS/NS 2.2.6 messes up -- Peter Kendell */
				int indx = ESHK(shkp)->billct;
				*bp = ESHK(shkp)->bill_p[indx];
			}
#else
			*bp = ESHK(shkp)->bill_p[ESHK(shkp)->billct];
#endif
		}
	}
	dealloc_obj(obj);
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
home_shk(shkp, killkops)
register struct monst *shkp;
register boolean killkops;
{
	register xchar x = ESHK(shkp)->shk.x, y = ESHK(shkp)->shk.y;

	(void) mnearto(shkp, x, y, TRUE);
	level.flags.has_shop = 1;
	if (killkops) {
#ifdef KOPS
		kops_gone(TRUE);
#else
		You("feel vaguely apprehensive.");
#endif
		pacify_guards();
	}
}

static boolean
angry_shk_exists()
{
	register struct monst *shkp;

	for (shkp = next_shkp(fmon, FALSE);
		shkp; shkp = next_shkp(shkp->nmon, FALSE))
	    if (ANGRY(shkp)) return(TRUE);
	return(FALSE);
}

/* remove previously applied surcharge from all billed items */
STATIC_OVL void
pacify_shk(shkp)
register struct monst *shkp;
{
	NOTANGRY(shkp) = TRUE;	/* make peaceful */
	if (ESHK(shkp)->surcharge) {
		register struct bill_x *bp = ESHK(shkp)->bill_p;
		register int ct = ESHK(shkp)->billct;

		ESHK(shkp)->surcharge = FALSE;
		while (ct-- > 0) {
			register long reduction = (bp->price + 3L) / 4L;
			bp->price -= reduction;		/* undo 33% increase */
			bp++;
		}
	}
}

/* add aggravation surcharge to all billed items */
static void
rile_shk(shkp)
register struct monst *shkp;
{
	NOTANGRY(shkp) = FALSE;	/* make angry */
	if (!ESHK(shkp)->surcharge) {
		register struct bill_x *bp = ESHK(shkp)->bill_p;
		register int ct = ESHK(shkp)->billct;

		ESHK(shkp)->surcharge = TRUE;
		while (ct-- > 0) {
			register long surcharge = (bp->price + 2L) / 3L;
			bp->price += surcharge;
			bp++;
		}
	}
}

void
make_happy_shk(shkp, silentkops)
register struct monst *shkp;
register boolean silentkops;
{
	register boolean wasmad = ANGRY(shkp);

	pacify_shk(shkp);
	ESHK(shkp)->following = 0;
	ESHK(shkp)->robbed = 0L;
	if (pl_character[0] != 'R')
		adjalign(sgn(u.ualign.type));
	if(!inhishop(shkp)) {
		pline("Satisfied, %s suddenly disappears!", mon_nam(shkp));
		if(on_level(&(ESHK(shkp)->shoplevel), &u.uz))
			home_shk(shkp, FALSE);
		else
			migrate_to_level(shkp,
				 ledger_no(&(ESHK(shkp)->shoplevel)), 0);
	} else if(wasmad)
		pline("%s calms down.", Monnam(shkp));

	if(!angry_shk_exists()) {
#ifdef KOPS
		kops_gone(silentkops);
#endif
		pacify_guards();
	}
}

void
hot_pursuit(shkp)
register struct monst *shkp;
{
	if(!shkp->isshk) return;

	rile_shk(shkp);
	ESHK(shkp)->following = 1;
}

/* used when the shkp is teleported out of his shop,
 * or when the player is not on a costly_spot and he
 * damages something inside the shop.  these conditions
 * must be checked by the calling function.
 */
void
make_angry_shk(shkp, ox, oy)
register struct monst *shkp;
register xchar ox,oy;
{
	if(index(in_rooms(ox, oy, SHOPBASE), ESHK(shkp)->shoproom) &&
	    !ANGRY(shkp)) {
		ESHK(shkp)->robbed += (addupbill(shkp) +
				       ESHK(shkp)->debit + ESHK(shkp)->loan);
		ESHK(shkp)->robbed -= ESHK(shkp)->credit;
		if(ESHK(shkp)->robbed < 0L)
		    ESHK(shkp)->robbed = 0L;
		ESHK(shkp)->credit = 0L;
		setpaid(shkp);
	}
	if(!ANGRY(shkp)) pline("%s gets angry!", Monnam(shkp));
	else pline("%s is furious!", Monnam(shkp));
	hot_pursuit(shkp);
}

static const char no_money[] = "Moreover, you%s have no money.";

static long
cheapest_item(shkp)   /* delivers the cheapest item on the list */
register struct monst *shkp;
{
	register int ct = ESHK(shkp)->billct;
	register struct bill_x *bp = ESHK(shkp)->bill_p;
	register long gmin = (bp->price * bp->bquan);

	while(ct--){
		if(bp->price * bp->bquan < gmin)
			gmin = bp->price * bp->bquan;
		bp++;
	}
	return(gmin);
}

int
dopay()
{
	long ltmp;
	register struct monst *nxtm = (struct monst *)0;
	register struct monst *shkp, *resident = (struct monst *)0;
	register struct eshk *eshkp;
	int pass, tmp, sk = 0, seensk = 0;
	register boolean paid = FALSE, stashed_gold = (hidden_gold() > 0L);

	multi = 0;

	/* find how many shk's there are, how many are in */
	/* sight, and are you in a shop room with one.    */
	for (shkp = next_shkp(fmon, FALSE);
		shkp; shkp = next_shkp(shkp->nmon, FALSE)) {
	    sk++;
	    if (ANGRY(shkp) && distu(shkp->mx, shkp->my) <= 2) nxtm = shkp;
	    if (canseemon(shkp) || sensemon(shkp)) seensk++;
	    if (inhishop(shkp) && (*u.ushops == ESHK(shkp)->shoproom))
		resident = shkp;
	}

	if (nxtm) {			/* Player should always appease an */
	     shkp = nxtm;		/* irate shk standing next to them. */
	     goto proceed;
	}

	if ((!sk && (!Blind || Telepat)) || (!Blind && !seensk)) {
      pline("There appears to be no shopkeeper here to receive your payment.");
		return(0);
	}

	if(!seensk) {
		You("can't see...");
		return(0);
	}

	/* the usual case.  allow paying at a distance when */
	/* inside a tended shop.  should we change that?    */
	if(sk == 1 && resident) {
		shkp = resident;
		goto proceed;
	}

	if (seensk == 1) {
		for (shkp = next_shkp(fmon, FALSE);
			shkp; shkp = next_shkp(shkp->nmon, FALSE))
		    if (canseemon(shkp) || sensemon(shkp)) break;
		if (shkp != resident && distu(shkp->mx, shkp->my) > 2) {
		    pline("%s is not near enough to receive your payment.",
					     Monnam(shkp));
		    return(0);
		}
	} else {
		struct monst *mtmp;
		coord cc;
		int cx, cy;

		pline("Pay whom?");
		cc.x = u.ux;
		cc.y = u.uy;
		getpos(&cc, TRUE, "the creature you want to pay");
		cx = cc.x;
		cy = cc.y;
		if(cx == -10) return(0); /* player pressed esc */
		if(cx < 0) {
		     pline("Try again...");
		     return(0);
		}
		if(u.ux == cx && u.uy == cy) {
		     You("are generous to yourself.");
		     return(0);
		}
		mtmp = m_at(cx, cy);
		if(!mtmp) {
		     pline("There is no one there to receive your payment.");
		     return(0);
		}
		if(!mtmp->isshk) {
		     pline("%s is not interested in your payment.",
				    Monnam(mtmp));
		     return(0);
		}
		if (mtmp != resident && distu(mtmp->mx, mtmp->my) > 2) {
		     pline("%s is too far to receive your payment.",
				    Monnam(mtmp));
		     return(0);
		}
		shkp = mtmp;
	}

	if(!shkp) {
#ifdef DEBUG
		pline("dopay: null shkp.");
#endif
		return(0);
	}
proceed:

	if (shkp->msleep || !shkp->mcanmove) {
		pline("%s %s.", Monnam(shkp),
		      rn2(2) ? "seems to be napping" : "doesn't respond");
		return 0;
	}
	eshkp = ESHK(shkp);

	ltmp = eshkp->robbed;
	if(shkp != resident && NOTANGRY(shkp)) {
		if(!ltmp)
		    You("do not owe %s anything.", mon_nam(shkp));
		else if(!u.ugold) {
		    You("%shave no money.", stashed_gold ? "seem to " : "");
		    if(stashed_gold)
			pline("But you have some gold stashed away.");
		} else {
		    const char *pronoun = shkp->female ? "she" : "he";
		    long ugold = u.ugold;

		    if(ugold > ltmp) {
			You("give %s the %ld gold piece%s %s asked for.",
			    mon_nam(shkp), ltmp, plur(ltmp), pronoun);
			pay(ltmp, shkp);
		    } else {
			You("give %s all your%s gold.", mon_nam(shkp),
					stashed_gold ? " openly kept" : "");
			pay(u.ugold, shkp);
			if (stashed_gold) pline("But you have hidden gold!");
		    }
		    if((ugold < ltmp/2L) || (ugold < ltmp && stashed_gold))
			pline("Unfortunately, %s doesn't look satisfied.",
			    pronoun);
		    else
			make_happy_shk(shkp, FALSE);
		}
		return(1);
	}

	/* ltmp is still eshkp->robbed here */
	if (!eshkp->billct && !eshkp->debit) {
		const char *pronoun = him[shkp->female];
		const char *possessive = his[shkp->female];

		if(!ltmp && NOTANGRY(shkp)) {
		    You("do not owe %s anything.", mon_nam(shkp));
		    if(!u.ugold) pline(no_money, stashed_gold ?
					   " seem to" : "");
		} else if(ltmp) {
		    pline("%s is after blood, not money!", Monnam(shkp));
		    if(u.ugold < ltmp/2L ||
				(u.ugold < ltmp && stashed_gold)) {
			if(!u.ugold) pline(no_money, stashed_gold ?
						       " seem to" : "");
			else pline("Besides, you don't have enough to interest %s.",
				pronoun);
			return(1);
		    }
		    pline("But since %s shop has been robbed recently,",
			possessive);
		    pline("you %scompensate %s for %s losses.",
			(u.ugold < ltmp) ? "partially " : "",
			mon_nam(shkp), possessive);
		    pay(u.ugold < ltmp ? u.ugold : ltmp, shkp);
		    make_happy_shk(shkp, FALSE);
		} else {
		    /* shopkeeper is angry, but has not been robbed --
		     * door broken, attacked, etc. */
		    pline("%s is after your hide, not your money!",
							 mon_nam(shkp));
		    if(u.ugold < 1000L) {
			if(!u.ugold) pline(no_money, stashed_gold ?
					     " seem to" : "");
			else pline("Besides, you don't have enough to interest %s.",
				    pronoun);
			return(1);
		    }
		    You("try to appease %s by giving %s 1000 gold pieces.",
			x_monnam(shkp, 1, "angry", 0), pronoun);
		    pay(1000L,shkp);
		    if (strncmp(eshkp->customer, plname, PL_NSIZ) || rn2(3))
			make_happy_shk(shkp, FALSE);
		    else
			pline("But %s is as angry as ever.", mon_nam(shkp));
		}
		return(1);
	}
	if(shkp != resident) {
		impossible("dopay: not to shopkeeper?");
		if(resident) setpaid(resident);
		return(0);
	}
	/* pay debt, if any, first */
	if(eshkp->debit) {
		long dtmp = eshkp->debit;
		long loan = eshkp->loan;
		char sbuf[BUFSZ];

		Sprintf(sbuf, "You owe %s %ld zorkmid%s ",
					   shkname(shkp), dtmp, plur(dtmp));
		if(loan) {
		    if(loan == dtmp)
			Strcat(sbuf, "you picked up in the store.");
		    else Strcat(sbuf,
			   "for gold picked up and the use of merchandise.");
		} else Strcat(sbuf, "for the use of merchandise.");
		pline(sbuf);
		if (u.ugold + eshkp->credit < dtmp) {
		    pline("But you don't%s have enough gold%s.",
			stashed_gold ? " seem to" : "",
			eshkp->credit ? " or credit" : "");
		    return(1);
		} else {
		    if (eshkp->credit >= dtmp) {
			eshkp->credit -= dtmp;
			eshkp->debit = 0L;
			eshkp->loan = 0L;
			Your("debt is covered by your credit.");
		    } else if (!eshkp->credit) {
			u.ugold -= dtmp;
			shkp->mgold += dtmp;
			eshkp->debit = 0L;
			eshkp->loan = 0L;
			You("pay that debt.");
			flags.botl = 1;
		    } else {
			dtmp -= eshkp->credit;
			eshkp->credit = 0L;
			u.ugold -= dtmp;
			shkp->mgold += dtmp;
			eshkp->debit = 0L;
			eshkp->loan = 0L;
			pline("That debt is partially offset by your credit.");
			You("pay the remainder.");
			flags.botl = 1;
		    }
		    paid = TRUE;
		}
	}
	/* now check items on bill */
	if (eshkp->billct) {
	    register boolean itemize;

	    if (!u.ugold && !eshkp->credit) {
		You("%shave no money or credit%s.",
				    stashed_gold ? "seem to " : "",
				    paid ? " left" : "");
		return(0);
	    }
	    if ((u.ugold + eshkp->credit) < cheapest_item(shkp)) {
		You("don't have enough money to buy%s the item%s you picked.",
		    eshkp->billct > 1 ? " any of" : "", plur(eshkp->billct));
		if(stashed_gold)
		    pline("Maybe you have some gold stashed away?");
		return(0);
	    }

	    /* this isn't quite right; it itemizes without asking if the
	     * single item on the bill is partly used up and partly unpaid */
	    itemize = (eshkp->billct > 1 ? yn("Itemized billing?") == 'y' : 1);

	    for (pass = 0; pass <= 1; pass++) {
		tmp = 0;
		while (tmp < eshkp->billct) {
		    struct obj *otmp;
		    register struct bill_x *bp = &(eshkp->bill_p[tmp]);

		    /* find the object on one of the lists */
		    if ((otmp = bp_to_obj(bp)) != 0) {
			/* if completely used up, object quantity is stale;
			   restoring it to its original value here avoids
			   making the partly-used-up code more complicated */
			if (bp->useup) otmp->quan = bp->bquan;
		    } else {
			impossible("Shopkeeper administration out of order.");
			setpaid(shkp);	/* be nice to the player */
			return 1;
		    }
		    if (pass == bp->useup && otmp->quan == bp->bquan) {
			/* pay for used-up items on first pass and others
			 * on second, so player will be stuck in the store
			 * less often; things which are partly used up
			 * are processed on both passes */
			tmp++;
		    } else {
			switch (dopayobj(shkp, bp, &otmp, pass, itemize)) {
			  case PAY_CANT:
				return 1;	/*break*/
			  case PAY_BROKE:
				paid = TRUE;
				goto thanks;	/*break*/
			  case PAY_SKIP:
				tmp++;
				continue;	/*break*/
			  case PAY_SOME:
				paid = TRUE;
				if (itemize) bot();
				continue;	/*break*/
			  case PAY_BUY:
				paid = TRUE;
				break;
			}
			if (itemize) bot();
			*bp = eshkp->bill_p[--eshkp->billct];
		    }
		}
	    }
	}
thanks:
	if(!ANGRY(shkp) && paid)
	    verbalize("Thank you for shopping in %s %s!",
		s_suffix(shkname(shkp)),
		shtypes[rooms[eshkp->shoproom - ROOMOFFSET].rtype - SHOPBASE].name);
	return(1);
}

/* return 2 if used-up portion paid */
/*	  1 if paid successfully    */
/*	  0 if not enough money     */
/*	 -1 if skip this object     */
/*	 -2 if no money/credit left */
static int
dopayobj(shkp, bp, obj_p, which, itemize)
register struct monst *shkp;
register struct bill_x *bp;
struct obj **obj_p;
int	which;		/* 0 => used-up item, 1 => other (unpaid or lost) */
boolean itemize;
{
	register struct obj *obj = *obj_p;
	long ltmp, quan, save_quan;
	int buy;
	boolean stashed_gold = (hidden_gold() > 0L),
		consumed = (which == 0);

	if(!obj->unpaid && !bp->useup){
		impossible("Paid object on bill??");
		return PAY_BUY;
	}
	if(itemize && u.ugold + ESHK(shkp)->credit == 0L){
		You("%shave no money or credit left.",
			     stashed_gold ? "seem to " : "");
		return PAY_BROKE;
	}
	/* we may need to temporarily adjust the object, if part of the
	   original quantity has been used up but part remains unpaid  */
	save_quan = obj->quan;
	if (consumed) {
	    /* either completely used up (simple), or split needed */
	    quan = bp->bquan;
	    if (quan > obj->quan)	/* difference is amount used up */
		quan -= obj->quan;
	} else {
	    /* dealing with ordinary unpaid item */
	    quan = obj->quan;
	}
	obj->quan = quan;	/* to be used by doname() */
	obj->unpaid = 0;	/* ditto */
	ltmp = bp->price * quan;
	buy = PAY_BUY;		/* flag; if changed then return early */

	if (itemize) {
	    char qbuf[BUFSZ];
	    Sprintf(qbuf,"%s for %ld zorkmid%s.  Pay?", quan == 1L ?
		    Doname2(obj) : doname(obj), ltmp, plur(ltmp));
	    if (yn(qbuf) == 'n') {
		buy = PAY_SKIP;		/* don't want to buy */
	    } else if (quan < bp->bquan && !consumed) { /* partly used goods */
		obj->quan = bp->bquan - save_quan;	/* used up amount */
		verbalize("%s for the other %s before buying %s.",
			  ANGRY(shkp) ? "Pay" : "Please pay", xname(obj),
			  save_quan > 1L ? "these" : "this one");
		buy = PAY_SKIP;		/* shk won't sell */
	    }
	}
	if (buy == PAY_BUY && u.ugold + ESHK(shkp)->credit < ltmp) {
	    You("don't%s have gold%s enough to pay for %s.",
		stashed_gold ? " seem to" : "",
		(ESHK(shkp)->credit > 0L) ? " or credit" : "",
		doname(obj));
	    buy = itemize ? PAY_SKIP : PAY_CANT;
	}

	if (buy != PAY_BUY) {
	    /* restore unpaid object to original state */
	    obj->quan = save_quan;
	    obj->unpaid = 1;
	    return buy;
	}

	pay(ltmp, shkp);
	shk_names_obj(obj);	/* identify some non-magic objects */
	You("bought %s for %ld gold piece%s.",
		doname(obj), ltmp, plur(ltmp));
	obj->quan = save_quan;		/* restore original count */
	/* quan => amount just bought, save_quan => remaining unpaid count */
	if (consumed) {
	    if (quan != bp->bquan) {
		/* eliminate used-up portion; remainder is still unpaid */
		bp->bquan = obj->quan;
		obj->unpaid = 1;
		bp->useup = 0;
		buy = PAY_SOME;
	    } else {	/* completely used-up, so get rid of it */
		if (obj == billobjs) {
		    billobjs = obj->nobj;
		} else {
		    register struct obj *otmp = billobjs;

		    while (otmp && otmp->nobj != obj) otmp = otmp->nobj;
		    if (otmp) otmp->nobj = obj->nobj;
		    else impossible("Error in shopkeeper administration.");
		}
	     /* assert( obj == *obj_p ); */
		dealloc_obj(obj);
		*obj_p = 0;	/* destroy pointer to freed object */
	    }
	}
	return buy;
}

/* routine called after dying (or quitting) */
boolean
paybill(croaked)
register boolean croaked;
{
	register struct monst *mtmp, *mtmp2, *resident= (struct monst *)0;
	register boolean taken = FALSE;
	register int numsk = 0;

	/* give shopkeeper first crack */
	if ((mtmp = shop_keeper(*u.ushops)) && inhishop(mtmp)) {
	    numsk++;
	    resident = mtmp;
	    taken = inherits(resident, numsk, croaked);
	}
	for (mtmp = next_shkp(fmon, FALSE);
		mtmp; mtmp = next_shkp(mtmp2, FALSE)) {
	    mtmp2 = mtmp->nmon;
	    if (mtmp != resident) {
		/* for bones: we don't want a shopless shk around */
		if(!on_level(&(ESHK(mtmp)->shoplevel), &u.uz))
			mongone(mtmp);
		else {
		    numsk++;
		    taken |= inherits(mtmp, numsk, croaked);
		}
	    }
	}
	if(numsk == 0) return(FALSE);
	return(taken);
}

static boolean
inherits(shkp, numsk, croaked)
register struct monst *shkp;
register int numsk;
register boolean croaked;
{
	register long loss = 0L;
	register struct obj *otmp;
	register struct eshk *eshkp = ESHK(shkp);
	register xchar ox, oy;
	register boolean take = FALSE, taken = FALSE;
	register int roomno = *u.ushops;

	/* the simplifying principle is that first-come */
	/* already took everything you had.		*/
	if(numsk > 1) {
	    if(cansee(shkp->mx, shkp->my) && croaked)
		pline("%s %slooks at your corpse%s%s", Monnam(shkp),
		     (shkp->msleep || !shkp->mcanmove) ?
				   "wakes up, " : "",
		     !rn2(2) ? (shkp->female ? ", shakes her head," :
				 ", shakes his head,") : "",
		     !inhishop(shkp) ? " and disappears. " : " and sighs.");
	    taken = (roomno == eshkp->shoproom);
	    goto skip;
	}

	/* get one case out of the way: you die in the shop, the */
	/* shopkeeper is peaceful, nothing stolen, nothing owed. */
	if(roomno == eshkp->shoproom && inhishop(shkp) &&
	    !IS_DOOR(levl[u.ux][u.uy].typ) && !eshkp->billct &&
	    !eshkp->robbed && !eshkp->debit &&
	     NOTANGRY(shkp) && !eshkp->following) {
		if (invent)
			pline("%s gratefully inherits all your possessions.",
				shkname(shkp));
		goto clear;
	}

	if (eshkp->billct || eshkp->debit || eshkp->robbed) {
		register long total = 0L;

		if(roomno == eshkp->shoproom && inhishop(shkp))
		    total = (addupbill(shkp) + eshkp->debit);
		loss = ((total >= eshkp->robbed) ? total : eshkp->robbed);
		take = TRUE;
	}

	if (eshkp->following || ANGRY(shkp) || take) {

		if(!invent && !u.ugold) goto skip;

		if((loss > u.ugold) || !loss) {
			pline("%s %s%stakes all your possessions.",
				shkname(shkp),
				(shkp->msleep || !shkp->mcanmove) ?
				   "wakes up and " : "",
				(distu(shkp->mx, shkp->my) > 2) ?
				    "comes and " : "");
			taken = TRUE;
			shkp->mgold += u.ugold;
			u.ugold = 0L;
			/* in case bones: make it be for real... */
			if(!*u.ushops ||
			     IS_DOOR(levl[u.ux][u.uy].typ)) {
			    /* shk.x,shk.y is the position immediately in
			     * front of the door -- move in one more space
			     */
			    ox = eshkp->shk.x;
			    oy = eshkp->shk.y;
			    ox += sgn(ox - eshkp->shd.x);
			    oy += sgn(oy - eshkp->shd.y);
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
			shkp->mgold += loss;
			pline("%s %sand takes %ld zorkmid%s %sowed %s.",
			      Monnam(shkp),
			      (shkp->msleep || !shkp->mcanmove) ?
					"wakes up " : "comes ",
			      loss, plur(loss),
			      strncmp(eshkp->customer,
					   plname, PL_NSIZ) ? "" : "you ",
			      shkp->female ? "her" : "him");
		}
skip:
		/* in case we create bones */
		if(!inhishop(shkp))
			home_shk(shkp, FALSE);
	}
clear:
	setpaid(shkp);
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
		!(obj = o_on(id, level.buriedobjlist)) &&
		!(obj = o_on(id, migrating_objs))) {
		    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if ((obj = o_on(id, mtmp->minvent)) != 0)
			    return obj;
		    for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon)
			if ((obj = o_on(id, mtmp->minvent)) != 0)
			    return obj;
		}
	return(obj);
}

/* calculate the value that the shk will charge for [one of] an object */
static long
get_cost(obj, shkp)
register struct obj *obj;
register struct monst *shkp;	/* if angry, impose a surcharge */
{
	register long tmp = getprice(obj);

	if (!tmp) tmp = 5L;
	/* shopkeeper may notice if the player isn't very knowledgeable -
	   especially when gem prices are concerned */
	if (!objects[obj->otyp].oc_name_known)
		if (obj->oclass == GEM_CLASS) {
			/* all gems are priced high - real or not */
			if (objects[obj->otyp].oc_material == GLASS) {
				/* real gem's cost (worthless gems come
				   after jade but before luckstone) */
				tmp = (long)objects[
					obj->otyp - LUCKSTONE + JADE + 1].oc_cost;
			}
		} else if (!(obj->o_id % 4)) /* arbitrarily impose surcharge */
			tmp += tmp / 3L;
#ifdef TOURIST
	if((pl_character[0] == 'T' && u.ulevel < (MAXULEV/2))
	    || (uarmu && !uarm && !uarmc))	/* Hawaiian shirt visible */
		tmp += tmp / 3L;
#endif
	if (ACURR(A_CHA) > 18)		tmp /= 2L;
	else if (ACURR(A_CHA) > 17)	tmp -= tmp / 3L;
	else if (ACURR(A_CHA) > 15)	tmp -= tmp / 4L;
	else if (ACURR(A_CHA) < 6)	tmp *= 2L;
	else if (ACURR(A_CHA) < 8)	tmp += tmp / 2L;
	else if (ACURR(A_CHA) < 11)	tmp += tmp / 3L;
	if (tmp <= 0L) tmp = 1L;
	else if (obj->oartifact) tmp *= 4L;
	/* anger surcharge should match rile_shk's */
	if (shkp && ESHK(shkp)->surcharge) tmp += (tmp + 2L) / 3L;
	return tmp;
}

/* returns the price of a container's content.  the price
 * of the "top" container is added in the calling functions.
 * a different price quoted for selling as vs. buying.
 */
long
contained_cost(obj, shkp, price, usell)
register struct obj *obj;
register struct monst *shkp;
long price;
register boolean usell;
{
	register struct obj *otmp;

	/* the price of contained objects */
	for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {
	    register boolean goods = saleable(rooms[ESHK(shkp)->shoproom -
					   ROOMOFFSET].rtype-SHOPBASE, otmp);

	    if(otmp->oclass == GOLD_CLASS) continue;

	    /* the "top" container is evaluated by caller */
	    if(usell) {
		if(goods && !otmp->unpaid &&
			otmp->oclass != BALL_CLASS &&
			!(otmp->oclass == FOOD_CLASS && otmp->oeaten) &&
			!(Is_candle(otmp) && otmp->age <
				20L * (long)objects[otmp->otyp].oc_cost))
		    price += set_cost(otmp, shkp);
	    } else if(!otmp->no_charge) {
		    price += get_cost(otmp, shkp);
	    }

	    if (Has_contents(otmp))
		    price += contained_cost(otmp, shkp, price, usell);
	}

	return(price);
}

long
contained_gold(obj)
register struct obj *obj;
{
	register struct obj *otmp;
	register long value = 0L;

	/* accumulate contained gold */
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
	    if (otmp->oclass == GOLD_CLASS)
		value += otmp->quan;
	    else if (Has_contents(otmp))
		value += contained_gold(otmp);

	return(value);
}

static void
dropped_container(obj, shkp, sale)
register struct obj *obj;
register struct monst *shkp;
register boolean sale;
{
	register struct obj *otmp;
	register boolean saleitem;

	/* the "top" container is treated in the calling fn */
	for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {

	    if(otmp->oclass == GOLD_CLASS) continue;

	    saleitem = saleable(rooms[ESHK(shkp)->shoproom -
					ROOMOFFSET].rtype-SHOPBASE, otmp);

	    if(!otmp->unpaid && !(sale && saleitem))
		otmp->no_charge = 1;

	    if (Has_contents(otmp))
		dropped_container(otmp, shkp, sale);
	}
}

void
picked_container(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	/* the "top" container is treated in the calling fn */
	for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {

	    if(otmp->oclass == GOLD_CLASS) continue;

	    if(otmp->no_charge)
		otmp->no_charge = 0;

	    if (Has_contents(otmp))
		picked_container(otmp);
	}
}

/* calculate how much the shk will pay when buying [all of] an object */
static long
set_cost(obj, shkp)
register struct obj *obj;
register struct monst *shkp;
{
	long tmp = getprice(obj) * obj->quan;

#ifdef TOURIST
	if ((pl_character[0] == 'T' && u.ulevel < (MAXULEV/2))
	    || (uarmu && !uarm && !uarmc))	/* Hawaiian shirt visible */
		tmp /= 3L;
	else
#endif
		tmp /= 2L;
	/* shopkeeper may notice if the player isn't very knowledgeable -
	   especially when gem prices are concerned */
	if (!objects[obj->otyp].oc_name_known) {
		if (obj->oclass == GEM_CLASS) {
			/* different shop keepers give different prices */
			if (objects[obj->otyp].oc_material == GEMSTONE ||
			    objects[obj->otyp].oc_material == GLASS) {
				tmp = (obj->otyp % (6 - shkp->m_id % 3));
				tmp = (tmp + 3) * obj->quan;
			}
		} else if (tmp > 1L && !rn2(4))
			tmp -= tmp / 4L;
	}
	return tmp;
}

/* called from doinv(invent.c) for inventory of unpaid objects */
long
unpaid_cost(unp_obj)
register struct obj *unp_obj;	/* known to be unpaid */
{
	register struct bill_x *bp = (struct bill_x *)0;
	register struct monst *shkp;

	for(shkp = next_shkp(fmon, TRUE); shkp;
					shkp = next_shkp(shkp->nmon, TRUE))
	    if ((bp = onbill(unp_obj, shkp, TRUE)) != 0) break;

	/* onbill() gave no message if unexpected problem occurred */
	if(!bp) impossible("unpaid_cost: object wasn't on any bill!");

	return bp ? unp_obj->quan * bp->price : 0L;
}

static void
add_one_tobill(obj, dummy)
register struct obj *obj;
register boolean dummy;
{
	register struct monst *shkp;
	register struct bill_x *bp;
	register int bct;
	register char roomno = *u.ushops;

	if(!*u.ushops) return;

	if(!(shkp = shop_keeper(roomno))) return;

	if(!inhishop(shkp)) return;

	if(onbill(obj, shkp, FALSE) || /* perhaps thrown away earlier */
		    (obj->oclass == FOOD_CLASS && obj->oeaten))
		return;

	if(ESHK(shkp)->billct == BILLSZ) {
		You("got that for free!");
		return;
	}

	/* To recognize objects the shopkeeper is not interested in. -dgk
	 */
	if (obj->no_charge) {
		obj->no_charge = 0;
		return;
	}

	bct = ESHK(shkp)->billct;
	bp = &(ESHK(shkp)->bill_p[bct]);
	bp->bo_id = obj->o_id;
	bp->bquan = obj->quan;
	if(dummy) {		  /* a dummy object must be inserted into  */
	    bp->useup = 1;	  /* the billobjs chain here.  crucial for */
	    obj->nobj = billobjs; /* eating floorfood in shop.  see eat.c  */
	    billobjs = obj;
	} else	bp->useup = 0;
	bp->price = get_cost(obj, shkp);
	ESHK(shkp)->billct++;
	obj->unpaid = 1;
}

/* recursive billing of objects within containers. */
static void
bill_box_content(obj, ininv, dummy, shkp)
register struct obj *obj;
register boolean ininv, dummy;
register struct monst *shkp;
{
	register struct obj *otmp;

	for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {

		if(otmp->oclass == GOLD_CLASS) continue;
		/* the "top" box is added in addtobill() */
		if(!otmp->no_charge)
		    add_one_tobill(otmp, dummy);
		if (Has_contents(otmp))
		    bill_box_content(otmp, ininv, dummy, shkp);
	}

}

static void
shk_names_obj(obj)
register struct obj *obj;
/* shopkeeper tells you what an object is */
{
	obj->dknown = TRUE;
	/* use real name for ordinary weapons/armor, and spell-less
	 * scrolls/books (that is, blank and mail).
	 */
	if (!objects[obj->otyp].oc_magic &&
	    (obj->oclass == WEAPON_CLASS || obj->oclass == ARMOR_CLASS ||
	     obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS))
	    makeknown(obj->otyp);
}

void
addtobill(obj, ininv, dummy, silent)
register struct obj *obj;
register boolean ininv, dummy, silent;
{
	register struct monst *shkp;
	register char roomno = *u.ushops;
	long ltmp = 0L, cltmp = 0L, gltmp = 0L;
	register boolean container = Has_contents(obj);

	if(!*u.ushops) return;

	if(!(shkp = shop_keeper(roomno))) return;

	if(!inhishop(shkp)) return;

	if(/* perhaps we threw it away earlier */
		 onbill(obj, shkp, FALSE) ||
		 (obj->oclass == FOOD_CLASS && obj->oeaten)
	      ) return;

	if(ESHK(shkp)->billct == BILLSZ) {
		You("got that for free!");
		return;
	}

	if(obj->oclass == GOLD_CLASS) {
		costly_gold(obj->ox, obj->oy, obj->quan);
		return;
	}

	if(!obj->no_charge)
	    ltmp = get_cost(obj, shkp);

	if (obj->no_charge && !container) {
		obj->no_charge = 0;
		return;
	}

	if(container) {
	    if(obj->cobj == (struct obj *)0) {
		if(obj->no_charge) {
		    obj->no_charge = 0;
		    return;
		} else {
		    add_one_tobill(obj, dummy);
		    goto speak;
		}
	    } else {
		cltmp += contained_cost(obj, shkp, cltmp, FALSE);
		gltmp += contained_gold(obj);
	    }

	    if(ltmp) add_one_tobill(obj, dummy);
	    if(cltmp) bill_box_content(obj, ininv, dummy, shkp);
	    picked_container(obj); /* reset contained obj->no_charge */

	    ltmp += cltmp;

	    if(gltmp) {
		costly_gold(obj->ox, obj->oy, gltmp);
		if(!ltmp) return;
	    }

	    if(obj->no_charge)
		obj->no_charge = 0;

	} else /* i.e., !container */
	    add_one_tobill(obj, dummy);
speak:
	if (shkp->mcanmove && !shkp->msleep && !silent) {
	    char buf[BUFSZ];

	    if(!ltmp) {
		pline("%s has no interest in %s.", Monnam(shkp),
					     the(xname(obj)));
		return;
	    }
	    Strcpy(buf, "\"For you, ");
	    if (ANGRY(shkp)) Strcat(buf, "scum ");
	    else {
		static const char *honored[5] = {
		  "good", "honored", "most gracious", "esteemed",
		  "most renowned and sacred"
		};
		Strcat(buf, honored[rn2(4) + u.uevent.udemigod]);
#ifdef POLYSELF
		if(!is_human(uasmon)) Strcat(buf, " creature");
		else
#endif
		    Strcat(buf, (flags.female) ? " lady" : " sir");
	    }
	    if(ininv) {
		long quan = obj->quan;
		obj->quan = 1L; /* fool xname() into giving singular */
		pline("%s; only %ld %s %s.\"", buf, ltmp,
			(quan > 1L) ? "per" : "for this", xname(obj));
		obj->quan = quan;
	    } else
		pline("%s will cost you %ld zorkmid%s%s.",
			The(xname(obj)), ltmp, plur(ltmp),
			(obj->quan > 1L) ? " each" : "");
	} else if(!silent) {
	    if(ltmp) pline("The list price of %s is %ld zorkmid%s%s.",
				   the(xname(obj)), ltmp, plur(ltmp),
				   (obj->quan > 1L) ? " each" : "");
	    else pline("%s does not notice.", Monnam(shkp));
	}
}

void
splitbill(obj, otmp)
register struct obj *obj, *otmp;
{
	/* otmp has been split off from obj */
	register struct bill_x *bp;
	register long tmp;
	register struct monst *shkp = shop_keeper(*u.ushops);

	if(!shkp || !inhishop(shkp)) {
		impossible("splitbill: no resident shopkeeper??");
		return;
	}
	bp = onbill(obj, shkp, FALSE);
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

	if(ESHK(shkp)->billct == BILLSZ) otmp->unpaid = 0;
	else {
		tmp = bp->price;
		bp = &(ESHK(shkp)->bill_p[ESHK(shkp)->billct]);
		bp->bo_id = otmp->o_id;
		bp->bquan = otmp->quan;
		bp->useup = 0;
		bp->price = tmp;
		ESHK(shkp)->billct++;
	}
}

static void
sub_one_frombill(obj, shkp)
register struct obj *obj;
register struct monst *shkp;
{
	register struct bill_x *bp;

	if((bp = onbill(obj, shkp, FALSE)) != 0) {
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
		ESHK(shkp)->billct--;
#ifdef DUMB
		{
		/* DRS/NS 2.2.6 messes up -- Peter Kendell */
			int indx = ESHK(shkp)->billct;
			*bp = ESHK(shkp)->bill_p[indx];
		}
#else
		*bp = ESHK(shkp)->bill_p[ESHK(shkp)->billct];
#endif
		return;
	} else if (obj->unpaid) {
		impossible("sub_one_frombill: unpaid object not on bill");
		obj->unpaid = 0;
	}
}

/* recursive check of unpaid objects within nested containers. */
void
subfrombill(obj, shkp)
register struct obj *obj;
register struct monst *shkp;
{
	register struct obj *otmp;

	sub_one_frombill(obj, shkp);

	if (Has_contents(obj))
	    for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {
		if(otmp->oclass == GOLD_CLASS) continue;

		if (Has_contents(otmp))
		    subfrombill(otmp, shkp);
		else
		    sub_one_frombill(otmp, shkp);
	    }
}

static long
stolen_container(obj, shkp, price, ininv)
register struct obj *obj;
register struct monst *shkp;
long price;
register boolean ininv;
{
	register struct obj *otmp;

	if(ininv && obj->unpaid)
	    price += get_cost(obj, shkp);
	else {
	    if(!obj->no_charge)
		price += get_cost(obj, shkp);
	    obj->no_charge = 0;
	}

	/* the price of contained objects, if any */
	for(otmp = obj->cobj; otmp; otmp = otmp->nobj) {

	    if(otmp->oclass == GOLD_CLASS) continue;

	    if (!Has_contents(otmp)) {
		if(ininv) {
		    if(otmp->unpaid)
			price += get_cost(otmp, shkp);
		} else {
		    if(!otmp->no_charge) {
			if(!(otmp->oclass == BALL_CLASS ||
			    (otmp->oclass == FOOD_CLASS && otmp->oeaten) ||
			    (Is_candle(otmp) && otmp->age <
				  20L * (long)objects[otmp->otyp].oc_cost))
			  ) price += get_cost(otmp, shkp);
		    }
		    otmp->no_charge = 0;
		}
	    } else
		price += stolen_container(otmp, shkp, price, ininv);
	}

	return(price);
}

long
stolen_value(obj, x, y, peaceful, silent)
register struct obj *obj;
register xchar x, y;
register boolean peaceful, silent;
{
	register long value = 0L, gvalue = 0L;
	register struct monst *shkp;
	register boolean goods;

	shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

	if (!shkp || !inhishop(shkp))
	    return (0L);

	goods = saleable(rooms[ESHK(shkp)->shoproom -
				   ROOMOFFSET].rtype-SHOPBASE, obj);
	goods = (goods && !obj->no_charge);

	if(obj->oclass == GOLD_CLASS) {
	    gvalue += obj->quan;
	} else if (Has_contents(obj)) {
	    register boolean ininv = !!count_unpaid(obj->cobj);

	    value += stolen_container(obj, shkp, value, ininv);
	    if(!ininv) gvalue += contained_gold(obj);
	} else if(goods) {
	    value += get_cost(obj, shkp);
	}

	if(gvalue + value == 0L) return(0L);

	value += gvalue;

	if(peaceful) {
	    value = check_credit(value, shkp);
	    ESHK(shkp)->debit += value;

	    if(!silent) {
		if(obj->oclass == GOLD_CLASS)
		    You("owe %s %ld zorkmids!", mon_nam(shkp), value);
		else You("owe %s %ld zorkmids for %s!",
			mon_nam(shkp),
			value,
			obj->quan > 1L ? "them" : "it");
	    }
	} else {
	    ESHK(shkp)->robbed += value;

	    if(!silent) {
		if(cansee(shkp->mx, shkp->my)) {
		    if(ESHK(shkp)->customer[0] == 0)
			(void) strncpy(ESHK(shkp)->customer,plname,PL_NSIZ);
		    Norep("%s booms: \"%s, you are a thief!\"",
				Monnam(shkp), plname);
		} else  Norep("You hear a scream, \"Thief!\"");
	    }
	    hot_pursuit(shkp);
	    (void) angry_guards(FALSE);
	}
	return(value);
}

/* auto-response flag for/from "sell foo?" 'a' => 'y', 'q' => 'n' */
static char sell_response = 'a';

void
sellobj_state(deliberate)	/* called from dodrop(do.c) and doddrop() */
register boolean deliberate;
{
	/* If we're deliberately dropping something, there's no automatic
	response to the shopkeeper's "want to sell" query; however, if we
	accidentally drop anything, the shk will buy it/them without asking.
	This retains the old pre-query risk that slippery fingers while in
	shops entailed:  you drop it, you've lost it.
	 */
	sell_response = deliberate ? '\0' : 'a';
}

void
sellobj(obj, x, y)
register struct obj *obj;
register xchar x, y;
{
	register struct monst *shkp;
	register struct eshk *eshkp;
	register long ltmp = 0L, cltmp = 0L, gltmp = 0L, offer;
	boolean saleitem, cgold = FALSE, container = Has_contents(obj);
	boolean isgold = (obj->oclass == GOLD_CLASS);

	if(!(shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) ||
	   !inhishop(shkp)) return;
	if(!costly_spot(x, y))	return;
	if(!*u.ushops) return;

	saleitem = saleable(rooms[ESHK(shkp)->shoproom -
					ROOMOFFSET].rtype-SHOPBASE, obj);

	if(obj->unpaid && !container && !isgold) {
	    sub_one_frombill(obj, shkp);
	    return;
	}
	if(container) {
		/* find the price of content before subfrombill */
		cltmp += contained_cost(obj, shkp, cltmp, TRUE);
		/* find the value of contained gold */
		gltmp += contained_gold(obj);
		cgold = (gltmp > 0L);
	}

	if(!isgold && !obj->unpaid && saleitem)
	    ltmp = set_cost(obj, shkp);

	offer = ltmp + cltmp;

	/* get one case out of the way: nothing to sell, and no gold */
	if(!isgold && (offer + gltmp) == 0L) {
		register boolean unpaid = (obj->unpaid ||
				  (container && count_unpaid(obj->cobj)));

		if(container) {
			dropped_container(obj, shkp, FALSE);
			if(!obj->unpaid && !saleitem)
			    obj->no_charge = 1;
			if(obj->unpaid || count_unpaid(obj->cobj))
			    subfrombill(obj, shkp);
		} else obj->no_charge = 1;

		if(!unpaid)
		    pline("%s seems uninterested.", Monnam(shkp));
		return;
	}

	/* you dropped something of your own - probably want to sell it */
	if (shkp->msleep || !shkp->mcanmove) {
		if (container)
		    dropped_container(obj, shkp, TRUE);
		if (!obj->unpaid)
		    obj->no_charge = 1;
		if (!shkp->mcanmove) {
		    if(ANGRY(shkp) && !rn2(4))
			pline("%s utters a curse.", Monnam(shkp));
		    else pline("%s is indisposed.", Monnam(shkp));
		} else if(!rn2(3)) {
		    pline("%s snores indifferently.", Monnam(shkp));
		}
		subfrombill(obj, shkp);
		return;
	}

	eshkp = ESHK(shkp);

	if (ANGRY(shkp)) { /* they become shop-objects, no pay */
		pline("Thank you, scum!");
		subfrombill(obj, shkp);
		return;
	}

	if(eshkp->robbed) {  /* shkp is not angry? */
		if(isgold) offer = obj->quan;
		else if(cgold) offer += cgold;
		if((eshkp->robbed -= offer < 0L))
			eshkp->robbed = 0L;
		if(offer) verbalize(
  "Thank you for your contribution to restock this recently plundered shop.");
		subfrombill(obj, shkp);
		return;
	}

	if(isgold || cgold) {
		if(!cgold) gltmp = obj->quan;

		if(eshkp->debit >= gltmp) {
		    if(eshkp->loan) { /* you carry shop's gold */
			 if(eshkp->loan >= gltmp)
			     eshkp->loan -= gltmp;
			 else eshkp->loan = 0L;
		    }
		    eshkp->debit -= gltmp;
		    Your("debt is %spaid off.",
				eshkp->debit ? "partially " : "");
		} else {
		    long delta = gltmp - eshkp->debit;

		    eshkp->credit += delta;
		    if(eshkp->debit) {
			eshkp->debit = 0L;
			eshkp->loan = 0L;
			Your("debt is paid off.");
		    }
		    pline("%ld zorkmid%s added to your credit.",
				delta, delta > 1L ? "s are" : " is");
		}
		if(offer) goto move_on;
		else {
		    if(!isgold) {
			if (container)
			    dropped_container(obj, shkp, FALSE);
			if (!obj->unpaid && !saleitem) obj->no_charge = 1;
			subfrombill(obj, shkp);
		    }
		    return;
		}
	}
move_on:
	if((!saleitem && !(container && cltmp > 0L))
	   || eshkp->billct == BILLSZ
	   || obj->oclass == BALL_CLASS
	   || obj->oclass == CHAIN_CLASS || offer == 0L
	   || (obj->oclass == FOOD_CLASS && obj->oeaten)
	   || (Is_candle(obj) &&
		   obj->age < 20L * (long)objects[obj->otyp].oc_cost)) {
		pline("%s seems not interested%s.", Monnam(shkp),
			cgold ? " in the rest" : "");
		if (container)
		    dropped_container(obj, shkp, FALSE);
		obj->no_charge = 1;
		return;
	}

	if(!shkp->mgold) {
		char c, qbuf[BUFSZ];
		long tmpcr = (ltmp + cltmp) * 2L;

		if (sell_response != 'n') {
		    pline("%s cannot pay you at present.", Monnam(shkp));
		    Sprintf(qbuf,
			    "Will you accept %ld zorkmids in credit for %s? ",
			    tmpcr, doname(obj));
		    /* won't accept 'a' response here */
		    c = ynq(qbuf);
		} else		/* previously specified "quit" */
		    c = 'n';

		if (c == 'y') {
		    You("have %ld zorkmids in %scredit.", tmpcr,
			ESHK(shkp)->credit > 0L ? "additional " : "");
		    ESHK(shkp)->credit += tmpcr;
		    subfrombill(obj, shkp);
		} else {
		    if (c == 'q') sell_response = 'n';
		    if (container)
			dropped_container(obj, shkp, FALSE);
		    if (!obj->unpaid) obj->no_charge = 1;
		    subfrombill(obj, shkp);
		}
	} else {
		int qlen;
		char qbuf[BUFSZ];
		boolean short_funds = (offer > shkp->mgold);

		if (short_funds) offer = shkp->mgold;

		if (!sell_response) {
		    Sprintf(qbuf,
			    "%s offers%s %ld gold piece%s for%s your %s.",
			    Monnam(shkp), short_funds ? " only" : "",
			    offer, plur(offer),
			    (!ltmp && cltmp) ? " the contents of" : "",
			    xname(obj));
		    qlen = strlen(qbuf);
		    /*  Will the prompt fit on the topline? (or would
		     *	"--more--" force line wrap anyway?)  If so, combine
		     *	the message and prompt; otherwise, flush message
		     *	and prompt separately.
		     */
		    if (qlen > COLNO - 24 && qlen <= COLNO - 8)
			pline(qbuf),  qbuf[0] = '\0';
		    else  Strcat(qbuf, "  ");
		    Strcat(strcat(qbuf, "Sell "),
			    (obj->quan == 1L ? "it?" : "them?"));
		} else  qbuf[0] = '\0';		/* just to pacify lint */

		switch (sell_response ? sell_response : ynaq(qbuf)) {
		 case 'q':  sell_response = 'n';
		 case 'n':  if (container)
				dropped_container(obj, shkp, FALSE);
			    if (!obj->unpaid) obj->no_charge = 1;
			    subfrombill(obj, shkp);
			    break;
		 case 'a':  sell_response = 'y';
		 case 'y':  if (container)
				dropped_container(obj, shkp, TRUE);
			    if (!obj->unpaid && !saleitem) obj->no_charge = 1;
			    subfrombill(obj, shkp);
			    pay(-offer, shkp);
			    You("sold %s for %ld gold piece%s.", doname(obj),
				offer, plur(offer));
			    break;
		 default:   impossible("invalid sell response");
		}
	}
}

int
doinvbill(mode)
int mode;		/* 0: deliver count 1: paged */
{
	register struct monst* shkp;
	register struct bill_x *bp, *end_bp;
	register struct obj *obj;
	long totused;
	char *buf_p;
	winid datawin;

	shkp = shop_keeper(*u.ushops);

	if(mode == 0) {
	    register int cnt = 0;

	    if(shkp && inhishop(shkp))
		for (bp = ESHK(shkp)->bill_p,
			end_bp = &(ESHK(shkp)->bill_p[ESHK(shkp)->billct]);
			bp < end_bp; bp++)
		    if(bp->useup ||
		       ((obj = bp_to_obj(bp)) && obj->quan < bp->bquan))
			cnt++;
	    return(cnt);
	}

	if(!shkp || !inhishop(shkp)) {
		impossible("doinvbill: no shopkeeper?");
		return(0);
	}

	datawin = create_nhwindow(NHW_MENU);
	putstr(datawin, 0, "Unpaid articles already used up:");
	putstr(datawin, 0, "");

	totused = 0L;
	for (bp = ESHK(shkp)->bill_p,
		end_bp = &(ESHK(shkp)->bill_p[ESHK(shkp)->billct]);
		bp < end_bp; bp++) {
	    obj = bp_to_obj(bp);
	    if(!obj) {
		impossible("Bad shopkeeper administration.");
		goto quit;
	    }
	    if(bp->useup || bp->bquan > obj->quan) {
		register long oquan, uquan, thisused;
		unsigned save_unpaid;

		save_unpaid = obj->unpaid;
		oquan = obj->quan;
		uquan = (bp->useup ? bp->bquan : bp->bquan - oquan);
		thisused = bp->price * uquan;
		totused += thisused;
		obj->quan = uquan;		/* cheat doname */
		obj->unpaid = 0;		/* ditto */
		buf_p = xprname(obj, ' ', FALSE, thisused);
		obj->quan = oquan;		/* restore value */
#ifdef __SASC
				/* SAS/C 6.2 can't cope for some reason */
		sasc_bug(obj,save_unpaid);
#else
		obj->unpaid = save_unpaid;
#endif
		putstr(datawin, 0, buf_p);
	    }
	}
	buf_p = xprname((struct obj *)0, '*', FALSE, totused);
	putstr(datawin, 0, "");
	putstr(datawin, 0, buf_p);
	display_nhwindow(datawin, FALSE);
    quit:
	destroy_nhwindow(datawin);
	return(0);
}

#define HUNGRY	2

static long
getprice(obj)
register struct obj *obj;
{
	register long tmp = (long) objects[obj->otyp].oc_cost;

	switch(obj->oclass) {
	case FOOD_CLASS:
		/* simpler hunger check, (2-4)*cost */
		if (u.uhs >= HUNGRY) tmp *= (long) u.uhs;
		if (obj->oeaten) tmp = 0L;
		break;
	case WAND_CLASS:
		if (obj->spe == -1) tmp = 0L;
		break;
	case POTION_CLASS:
		if (obj->otyp == POT_WATER && !obj->blessed && !obj->cursed)
			tmp = 0L;
		break;
	case ARMOR_CLASS:
	case WEAPON_CLASS:
		if (obj->spe > 0) tmp += 10L * (long) obj->spe;
		break;
	case TOOL_CLASS:
		if (Is_candle(obj) &&
			obj->age < 20L * (long)objects[obj->otyp].oc_cost)
		    tmp /= 2L;
		break;
	}
	if (obj->oartifact) tmp *= 25L;
	return tmp;
}

/* shk catches thrown pick-axe */
int
shkcatch(obj, x, y)
register struct obj *obj;
register xchar x, y;
{
	register struct monst *shkp;

	if (!(shkp = shop_keeper(inside_shop(x, y))) ||
	    !inhishop(shkp)) return(0);

	if (shkp->mcanmove && !shkp->msleep &&
	    (*u.ushops != ESHK(shkp)->shoproom || !inside_shop(u.ux, u.uy)) &&
	    dist2(shkp->mx, shkp->my, x, y) < 3 &&
	    /* if it is the shk's pos, you hit and anger him */
	    (shkp->mx != x || shkp->my != y)) {
		if (mnearto(shkp, x, y, TRUE))
		    verbalize("Out of my way, scum!");
		pline("%s nimbly catches %s.", Monnam(shkp), the(xname(obj)));
		mpickobj(shkp, obj);
		subfrombill(obj, shkp);
		return(1);
	}
	return(0);
}

void
add_damage(x, y, cost)
register xchar x, y;
long cost;
{
	struct damage *tmp_dam;
	char *shops;

	if (IS_DOOR(levl[x][y].typ))
	    /* Don't schedule for repair unless it's a real shop entrance */
	    for (shops = in_rooms(x, y, SHOPBASE); *shops; shops++) {
		struct monst *mtmp = shop_keeper(*shops);

		if (!mtmp)
		    continue;
		if ((x != ESHK(mtmp)->shd.x) || (y != ESHK(mtmp)->shd.y))
		    return;
	    }
	tmp_dam = (struct damage *)alloc((unsigned)sizeof(struct damage));
	tmp_dam->when = monstermoves;
	tmp_dam->place.x = x;
	tmp_dam->place.y = y;
	tmp_dam->cost = cost;
	tmp_dam->typ = levl[x][y].typ;
	tmp_dam->next = level.damagelist;
	level.damagelist = tmp_dam;
	/* If player saw damage, display as a wall forever */
	if (cansee(x, y))
	    levl[x][y].seen = 1;
}

/*
 * Do something about damage. Either (!croaked) try to repair it, or
 * (croaked) just discard damage structs for non-shared locations, since
 * they'll never get repaired. Assume that shared locations will get
 * repaired eventually by the other shopkeeper(s). This might be an erroneous
 * assumption (they might all be dead too), but we have no reasonable way of
 * telling that.
 */
static
void
remove_damage(shkp, croaked)
register struct monst *shkp;
register boolean croaked;
{
	register struct damage *tmp_dam, *tmp2_dam;
	register boolean did_repair = FALSE, saw_door = FALSE;
	register boolean saw_floor = FALSE, stop_picking = FALSE;
	uchar saw_walls = 0;

	tmp_dam = level.damagelist;
	tmp2_dam = 0;
	while (tmp_dam) {
	    register xchar x = tmp_dam->place.x, y = tmp_dam->place.y;
	    char shops[5];
	    uchar disposition;

	    disposition = 0;
	    Strcpy(shops, in_rooms(x, y, SHOPBASE));
	    if (index(shops, ESHK(shkp)->shoproom)) {
		if (croaked)
		    disposition = (shops[1])? 0 : 1;
		else if (stop_picking)
		    disposition = repair_damage(shkp, tmp_dam);
		else {
		    /* Defer the stop_occupation() until after repair msgs */
		    if (closed_door(x, y))
			stop_picking = picking_at(x, y);
		    disposition = repair_damage(shkp, tmp_dam);
		    if (!disposition)
			stop_picking = FALSE;
		}
	    }

	    if (!disposition) {
		tmp2_dam = tmp_dam;
		tmp_dam = tmp_dam->next;
		continue;
	    }

	    if (disposition > 1) {
		did_repair = TRUE;
		if (cansee(x, y)) {
		    if (IS_WALL(levl[x][y].typ))
			saw_walls++;
		    else if (IS_DOOR(levl[x][y].typ))
			saw_door = TRUE;
		    else
			saw_floor = TRUE;
		}
	    }

	    tmp_dam = tmp_dam->next;
	    if (!tmp2_dam) {
		free((genericptr_t)level.damagelist);
		level.damagelist = tmp_dam;
	    } else {
		free((genericptr_t)tmp2_dam->next);
		tmp2_dam->next = tmp_dam;
	    }
	}
	if (!did_repair)
	    return;
	if (saw_walls) {
	    pline("Suddenly, %s section%s of wall close%s up!",
		  (saw_walls == 1) ? "a" : (saw_walls <= 3) ?
						  "some" : "several",
		  (saw_walls == 1) ? "" : "s", (saw_walls == 1) ? "s" : "");
	    if (saw_door)
		pline("The shop door reappears!");
	    if (saw_floor)
		pline("The floor is repaired!");
	} else {
	    if (saw_door)
		pline("Suddenly, the shop door reappears!");
	    else if (saw_floor)
		pline("Suddenly, the floor damage is gone!");
	    else if (inside_shop(u.ux, u.uy) == ESHK(shkp)->shoproom)
		You("feel more claustrophobic than before.");
	    else if (flags.soundok && !rn2(10))
		Norep("The dungeon acoustics noticeably change.");
	}
	if (stop_picking)
		stop_occupation();
}

/*
 * 0: repair postponed, 1: silent repair (no messages), 2: normal repair
 */
char
repair_damage(shkp, tmp_dam)
register struct monst *shkp;
register struct damage *tmp_dam;
{
	register xchar x, y, i;
	xchar litter[9];
	register struct monst *mtmp;
	register struct obj *otmp;
	register struct trap *ttmp;

	if ((monstermoves - tmp_dam->when) < REPAIR_DELAY)
	    return(0);
	if (shkp->msleep || !shkp->mcanmove || ESHK(shkp)->following)
	    return(0);
	x = tmp_dam->place.x;
	y = tmp_dam->place.y;
	if (!IS_ROOM(tmp_dam->typ)) {
	    if ((x == u.ux) && (y == u.uy))
#ifdef POLYSELF
		if (!passes_walls(uasmon))
#endif
		    return(0);
	    if ((x == shkp->mx) && (y == shkp->my))
		return(0);
	    if ((mtmp = m_at(x, y)) && (!passes_walls(mtmp->data)))
		return(0);
	}
	if ((ttmp = t_at(x, y)) != 0)
	    deltrap(ttmp);
	if (IS_ROOM(tmp_dam->typ)) {
	    /* No messages if player already filled trapdoor */
	    if (!ttmp)
		return(1);
	    newsym(x, y);
	    return(2);
	}
	if (!ttmp && (tmp_dam->typ == levl[x][y].typ) &&
	    (!IS_DOOR(tmp_dam->typ) || (levl[x][y].doormask > D_BROKEN)))
	    /* No messages if player already replaced shop door */
	    return(1);
	levl[x][y].typ = tmp_dam->typ;
	(void) memset((genericptr_t)litter, 0, sizeof(litter));
	if ((otmp = level.objects[x][y]) != 0) {
	    /* Scatter objects haphazardly into the shop */
#define NEED_UPDATE 1
#define OPEN	    2
#define INSHOP	    4
#define horiz(i) ((i%3)-1)
#define vert(i)  ((i/3)-1)
	    for (i = 0; i < 9; i++) {
		if ((i == 4) || (!ZAP_POS(levl[x+horiz(i)][y+vert(i)].typ)))
		    continue;
		litter[i] = OPEN;
		if (inside_shop(x+horiz(i),
				y+vert(i)) == ESHK(shkp)->shoproom)
		    litter[i] |= INSHOP;
	    }
	    if (Punished && !u.uswallow &&
				((uchain->ox == x && uchain->oy == y) ||
				 (uball->ox == x && uball->oy == y))) {
		/*
		 * Either the ball or chain is in the repair location.
		 *
		 * Take the easy way out and put ball&chain under hero.
		 */
		verbalize("Get your junk out of my wall!");
		unplacebc();	/* pick 'em up */
		placebc();	/* put 'em down */
	    }
	    while ((otmp = level.objects[x][y]) != 0)
		/* Don't mess w/ boulders -- just merge into wall */
		if ((otmp->otyp == BOULDER) || (otmp->otyp == ROCK)) {
		    freeobj(otmp);
		    obfree(otmp, (struct obj *)0);
		} else {
		    while (!(litter[i = rn2(9)] & INSHOP));
			remove_object(otmp);
			place_object(otmp, x+horiz(i), y+vert(i));
			litter[i] |= NEED_UPDATE;
		}
	}
	block_point(x, y);
	if(IS_DOOR(tmp_dam->typ)) {
	    levl[x][y].doormask = D_CLOSED; /* arbitrary */
	    newsym(x, y);
	} else {
	    levl[x][y].doormask = D_NODOOR;
	    if (IS_WALL(tmp_dam->typ) && cansee(x, y)) {
	    /* Player sees actual repair process, so they KNOW it's a wall */
		levl[x][y].seen = 1;
		newsym(x, y);
	    } else if (levl[x][y].seen) {
		/* Force a display update */
		levl[x][y].diggable |= W_REPAIRED;
	    }
	}
	for (i = 0; i < 9; i++)
	    if (litter[i] & NEED_UPDATE)
		newsym(x+horiz(i), y+vert(i));
	return(2);
#undef NEED_UPDATE
#undef OPEN
#undef INSHOP
#undef vert
#undef horiz
}

/*
 * shk_move: return 1: moved  0: didn't  -1: let m_move do it  -2: died
 */
int
shk_move(shkp)
register struct monst *shkp;
{
	register xchar gx,gy,omx,omy;
	register int udist;
	register schar appr;
	register struct eshk *eshkp = ESHK(shkp);
	int z;
	boolean uondoor = FALSE, satdoor, avoid = FALSE, badinv;

	omx = shkp->mx;
	omy = shkp->my;

	if (inhishop(shkp))
	    remove_damage(shkp, FALSE);

	if((udist = distu(omx,omy)) < 3 &&
	   (shkp->data != &mons[PM_GRID_BUG] || (omx==u.ux || omy==u.uy))) {
		if(ANGRY(shkp) ||
		   (Conflict && !resist(shkp, RING_CLASS, 0, 0))) {
			if(Displaced)
			  Your("displaced image doesn't fool %s!",
				mon_nam(shkp));
			(void) mattacku(shkp);
			return(0);
		}
		if(eshkp->following) {
			if(strncmp(eshkp->customer, plname, PL_NSIZ)) {
			    verbalize("Hello, %s!  I was looking for %s.",
				    plname, eshkp->customer);
				    eshkp->following = 0;
			    return(0);
			}
			if(moves > followmsg+4) {
			    verbalize("Hello, %s!  Didn't you forget to pay?",
				    plname);
			    followmsg = moves;
			    if (!rn2(4)) {
	    pline ("%s doesn't like customers who don't pay.", Monnam(shkp));
				rile_shk(shkp);
			    }
			}
			if(udist < 2)
			    return(0);
		}
	}

	appr = 1;
	gx = eshkp->shk.x;
	gy = eshkp->shk.y;
	satdoor = (gx == omx && gy == omy);
	if(eshkp->following || ((z = holetime()) >= 0 && z*z <= udist)){
		if(udist > 4)
		    return(-1);	/* leave it to m_move */
		gx = u.ux;
		gy = u.uy;
	} else if(ANGRY(shkp)) {
		/* Move towards the hero if the shopkeeper can see him. */
		if(shkp->mcansee && m_canseeu(shkp)) {
			gx = u.ux;
			gy = u.uy;
		}
		avoid = FALSE;
	} else {
#define	GDIST(x,y)	(dist2(x,y,gx,gy))
		if(Invis) {
		    avoid = FALSE;
		} else {
		    uondoor = (u.ux == eshkp->shd.x && u.uy == eshkp->shd.y);
		    if(uondoor) {
			badinv = (!!carrying(PICK_AXE));
			if(satdoor && badinv)
			    return(0);
			avoid = !badinv;
		    } else {
			avoid = (*u.ushops && distu(gx,gy) > 8);
			badinv = FALSE;
		    }

		    if(((!eshkp->robbed && !eshkp->billct && !eshkp->debit)
			|| avoid) && GDIST(omx,omy) < 3) {
			if (!badinv && !onlineu(omx,omy))
			    return(0);
			if(satdoor)
			    appr = gx = gy = 0;
		    }
		}
	}

	return(move_special(shkp,inhishop(shkp),
			    appr,uondoor,avoid,omx,omy,gx,gy));
}

/* for use in levl_follower (mondata.c) */
boolean
is_fshk(mtmp)
register struct monst *mtmp;
{
	return(mtmp->isshk && ESHK(mtmp)->following);
}

/* You are digging in the shop. */
void
shopdig(fall)
register int fall;
{
    register struct monst *shkp = shop_keeper(*u.ushops);

    if(!shkp) return;

    if(!inhishop(shkp)) {
	if (pl_character[0] == 'K') adjalign(-sgn(u.ualign.type));
	return;
    }

    if(!fall) {
	if(u.utraptype == TT_PIT)
	    verbalize("Be careful, %s, or you might fall through the floor.",
		flags.female ? "madam" : "sir");
	else
	    verbalize("%s, do not damage the floor here!",
			flags.female ? "Madam" : "Sir");
	if (pl_character[0] == 'K') adjalign(-sgn(u.ualign.type));
    } else if(!um_dist(shkp->mx, shkp->my, 5) &&
		!shkp->msleep && shkp->mcanmove &&
		(ESHK(shkp)->billct || ESHK(shkp)->debit)) {
	    register struct obj *obj, *obj2;

	    if (distu(shkp->mx, shkp->my) > 2) {
		mnexto(shkp);
		/* for some reason the shopkeeper can't come next to you */
		if (distu(shkp->mx, shkp->my) > 2) {
		    pline("%s curses you in anger and frustration!",
					shkname(shkp));
		    rile_shk(shkp);
		    return;
		} else pline("%s leaps, and grabs your backpack!",
					shkname(shkp));
	    } else pline("%s grabs your backpack!", shkname(shkp));

	    for(obj = invent; obj; obj = obj2) {
		obj2 = obj->nobj;
		if(obj->owornmask) continue;
#ifdef WALKIES
		if(obj->otyp == LEASH && obj->leashmon) continue;
#endif
		freeinv(obj);
		obj->nobj = shkp->minvent;
		shkp->minvent = obj;
		subfrombill(obj, shkp);
	    }
    }
}

#ifdef KOPS
STATIC_OVL void
makekops(mm)		/* returns the number of (all types of) Kops  made */
coord *mm;
{
	register int cnt = abs(depth(&u.uz)) + rnd(5);
	register int scnt = (cnt / 3) + 1;	/* at least one sarge */
	register int lcnt = (cnt / 6);		/* maybe a lieutenant */
	register int kcnt = (cnt / 9);		/* and maybe a kaptain */

	if (!(mons[PM_KEYSTONE_KOP].geno & G_EXTINCT)) {
	    while(cnt--) {
		if (enexto(mm, mm->x, mm->y, &mons[PM_KEYSTONE_KOP]))
			(void) makemon(&mons[PM_KEYSTONE_KOP], mm->x, mm->y);
	    }
	}
	if (!(mons[PM_KOP_SERGEANT].geno & G_EXTINCT)) {
	    while(scnt--) {
		if (enexto(mm, mm->x, mm->y, &mons[PM_KOP_SERGEANT]))
			(void) makemon(&mons[PM_KOP_SERGEANT], mm->x, mm->y);
	    }
	}
	if (!(mons[PM_KOP_LIEUTENANT].geno & G_EXTINCT)) {
	    while(lcnt--) {
		if (enexto(mm, mm->x, mm->y, &mons[PM_KOP_LIEUTENANT]))
		    (void) makemon(&mons[PM_KOP_LIEUTENANT], mm->x, mm->y);
	    }
	}
	if (!(mons[PM_KOP_KAPTAIN].geno & G_EXTINCT)) {
	    while(kcnt--) {
		if (enexto(mm, mm->x, mm->y, &mons[PM_KOP_KAPTAIN]))
		    (void) makemon(&mons[PM_KOP_KAPTAIN], mm->x, mm->y);
	    }
	}
}
#endif	/* KOPS */

void
pay_for_damage(dmgstr)
const char *dmgstr;
{
	register struct monst *shkp = (struct monst *)0;
	char shops_affected[5];
	register boolean uinshp = (*u.ushops != '\0');
	char qbuf[80];
	register xchar x, y;
	register boolean dugwall = !strcmp(dmgstr, "dig into");
	struct damage *tmp_dam, *appear_here = 0;
	/* any number >= (80*80)+(24*24) would do, actually */
	long cost_of_damage = 0L;
	unsigned int nearest_shk = 7000, nearest_damage = 7000;
	int picks = 0;

	for (tmp_dam = level.damagelist;
	     (tmp_dam && (tmp_dam->when == monstermoves));
	     tmp_dam = tmp_dam->next) {
	    char *shp;

	    if (!tmp_dam->cost)
		continue;
	    cost_of_damage += tmp_dam->cost;
	    Strcpy(shops_affected,
		   in_rooms(tmp_dam->place.x, tmp_dam->place.y, SHOPBASE));
	    for (shp = shops_affected; *shp; shp++) {
		struct monst *tmp_shk;
		unsigned int shk_distance;

		if (!(tmp_shk = shop_keeper(*shp)))
		    continue;
		if (tmp_shk == shkp) {
		    unsigned int damage_distance =
				   distu(tmp_dam->place.x, tmp_dam->place.y);

		    if (damage_distance < nearest_damage) {
			nearest_damage = damage_distance;
			appear_here = tmp_dam;
		    }
		    continue;
		}
		if (!inhishop(tmp_shk))
		    continue;
		shk_distance = distu(tmp_shk->mx, tmp_shk->my);
		if (shk_distance > nearest_shk)
		    continue;
		if ((shk_distance == nearest_shk) && picks) {
		    if (rn2(++picks))
			continue;
		} else
		    picks = 1;
		shkp = tmp_shk;
		nearest_shk = shk_distance;
		appear_here = tmp_dam;
		nearest_damage = distu(tmp_dam->place.x, tmp_dam->place.y);
	    }
	}

	if (!cost_of_damage || !shkp)
	    return;

	x = appear_here->place.x;
	y = appear_here->place.y;

	/* not the best introduction to the shk... */
	(void) strncpy(ESHK(shkp)->customer,plname,PL_NSIZ);

	/* if the shk is already on the war path, be sure it's all out */
	if(ANGRY(shkp) || ESHK(shkp)->following) {
		hot_pursuit(shkp);
		return;
	}

	/* if the shk is not in their shop.. */
	if(!*in_rooms(shkp->mx,shkp->my,SHOPBASE)) {
		if(!cansee(shkp->mx, shkp->my))
			return;
		goto getcad;
	}

	if(uinshp) {
		if(um_dist(shkp->mx, shkp->my, 1) &&
			!um_dist(shkp->mx, shkp->my, 3)) {
		    pline("%s leaps towards you!", shkname(shkp));
		    mnexto(shkp);
		}
		if(um_dist(shkp->mx, shkp->my, 1)) goto getcad;
	} else {
	    /*
	     * Make shkp show up at the door.  Effect:  If there is a monster
	     * in the doorway, have the hero hear the shopkeeper yell a bit,
	     * pause, then have the shopkeeper appear at the door, having
	     * yanked the hapless critter out of the way.
	     */
	    if (MON_AT(x, y)) {
		if(flags.soundok) {
		    You("hear an angry voice:");
		    verbalize("Out of my way, scum!");
		    wait_synch();
#if defined(UNIX) || defined(VMS)
# if defined(SYSV) || defined(ULTRIX) || defined(VMS)
		    (void)
# endif
			sleep(1);
#endif
		}
	    }
	    (void) mnearto(shkp, x, y, TRUE);
	}

	if((um_dist(x, y, 1) && !uinshp) ||
			(u.ugold + ESHK(shkp)->credit) < cost_of_damage
				|| !rn2(50)) {
		if(um_dist(x, y, 1) && !uinshp) {
		    pline("%s shouts:", shkname(shkp));
		    verbalize("Who dared %s my %s?", dmgstr,
					 dugwall ? "shop" : "door");
		} else {
getcad:
		    verbalize("How dare you %s my %s?", dmgstr,
					 dugwall ? "shop" : "door");
		}
		hot_pursuit(shkp);
		return;
	}

	if(Invis) Your("invisibility does not fool %s!", shkname(shkp));
	Sprintf(qbuf,"\"Cad!  You did %ld zorkmids worth of damage!\"  Pay? ",
		 cost_of_damage);
	if(yn(qbuf) != 'n') {
		cost_of_damage = check_credit(cost_of_damage, shkp);
		u.ugold -= cost_of_damage;
		shkp->mgold += cost_of_damage;
		flags.botl = 1;
		pline("Mollified, %s accepts your restitution.",
			shkname(shkp));
		/* move shk back to his home loc */
		home_shk(shkp, FALSE);
		pacify_shk(shkp);
	} else {
		verbalize("Oh, yes!  You'll pay!");
		hot_pursuit(shkp);
		adjalign(-sgn(u.ualign.type));
	}
}

/* called in dokick.c when we kick an object that might be in a store */
boolean
costly_spot(x, y)
register xchar x, y;
{
	register struct monst *shkp;

	if (!level.flags.has_shop) return FALSE;
	shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));
	if(!shkp || !inhishop(shkp)) return(FALSE);

	return(inside_shop(x, y) &&
		!(x == ESHK(shkp)->shk.x &&
			y == ESHK(shkp)->shk.y));
}

/* called by dotalk(sounds.c) when #chatting; returns obj if location
   contains shop goods and shopkeeper is willing & able to speak */
struct obj *
shop_object(x, y)
register xchar x, y;
{
    register struct obj *otmp;
    register struct monst *shkp;

    if(!(shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) || !inhishop(shkp))
	return(struct obj *)0;

    for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
	if (otmp->otyp != GOLD_PIECE)
	    break;
    /* note: otmp might have ->no_charge set, but that's ok */
    return (otmp && costly_spot(x, y) && NOTANGRY(shkp)
	    && shkp->mcanmove && !shkp->msleep)
		? otmp : (struct obj *)0;
}

/* give price quotes for all objects linked to this one (ie, on this spot) */
void
price_quote(first_obj)
register struct obj *first_obj;
{
    register struct obj *otmp;
    char buf[BUFSZ], price[40];
    long cost;
    int cnt = 0;
    winid tmpwin;

    tmpwin = create_nhwindow(NHW_MENU);
    putstr(tmpwin, 0, "Fine goods for sale:");
    putstr(tmpwin, 0, "");
    for (otmp = first_obj; otmp; otmp = otmp->nexthere) {
	if (otmp->otyp == GOLD_PIECE) {
	 /* if (otmp == first_obj)  first_obj = otmp->nexthere; */
	    continue;	/* don't quote a price on this */
	} else if (otmp->no_charge || otmp == uball || otmp == uchain) {
	    Strcpy(price, "no charge");
	} else {
	    cost = get_cost(otmp, (struct monst *)0);
	    Sprintf(price, "%ld zorkmid%s%s", cost, plur(cost),
		    otmp->quan > 1L ? " each" : "");
	}
	Sprintf(buf, "%s, %s", doname(otmp), price);
	putstr(tmpwin, 0, buf),  cnt++;
    }
    if (cnt > 1) {
	display_nhwindow(tmpwin, TRUE);
    } else if (cnt == 1) {
	if (first_obj->no_charge || first_obj == uball || first_obj == uchain){
	    pline("%s!", buf);	/* buf still contains the string */
	} else {
	    /* print cost in slightly different format, so can't reuse buf */
	    cost = get_cost(first_obj, (struct monst *)0);
	    pline("%s, price %ld zorkmid%s%s%s", doname(first_obj),
		cost, plur(cost), first_obj->quan > 1L ? " each" : "",
		shk_embellish(first_obj, cost));
	}
    }
    destroy_nhwindow(tmpwin);
}

static const char *
shk_embellish(itm, cost)
register struct obj *itm;
long cost;
{
    if (!rn2(3)) {
	register int o, choice = rn2(5);
	if (choice == 0) choice = (cost < 100L ? 1 : cost < 500L ? 2 : 3);
	switch (choice) {
	    case 4:
		if (cost < 10L) break; else o = itm->oclass;
		if (o == FOOD_CLASS) return ", gourmets' delight!";
		if (objects[itm->otyp].oc_name_known
		    ? objects[itm->otyp].oc_magic
		    : (o == AMULET_CLASS || o == RING_CLASS   ||
		       o == WAND_CLASS   || o == POTION_CLASS ||
		       o == SCROLL_CLASS || o == SPBOOK_CLASS))
		    return ", painstakingly developed!";
		return ", superb craftsmanship!";
	    case 3: return ", finest quality.";
	    case 2: return ", an excellent choice.";
	    case 1: return ", a real bargain.";
	   default: break;
	}
    } else if (itm->oartifact) {
	return ", one of a kind!";
    }
    return ".";
}

#ifdef SOUNDS
void
shk_chat(shkp)
register struct monst *shkp;
{
	register struct eshk *eshk = ESHK(shkp);

	if (ANGRY(shkp))
		pline("%s mentions how much %s dislikes %s customers.",
			shkname(shkp), he[shkp->female],
			eshk->robbed ? "non-paying" : "rude");
	else if (eshk->following)
		if (strncmp(eshk->customer, plname, PL_NSIZ)) {
		    verbalize("Hello %s!  I was looking for %s.",
			    plname, eshk->customer);
		    eshk->following = 0;
		} else {
		    verbalize("Hello %s!  Didn't you forget to pay?", plname);
		}
	else if (eshk->billct) {
		register long total = addupbill(shkp) + eshk->debit;
		pline("%s says that your bill comes to %ld zorkmid%s.",
		      shkname(shkp), total, plur(total));
	} else if (eshk->debit)
		pline("%s reminds you that you owe %s %ld zorkmid%s.",
		      shkname(shkp), him[shkp->female],
		      eshk->debit, plur(eshk->debit));
	else if (eshk->credit)
		pline("%s encourages you to use your %ld zorkmid%s of credit.",
		      shkname(shkp), eshk->credit, plur(eshk->credit));
	else if (eshk->robbed)
		pline("%s complains about a recent robbery.", shkname(shkp));
	else if (shkp->mgold < 50)
		pline("%s complains that business is bad.", shkname(shkp));
	else if (shkp->mgold > 4000)
		pline("%s says that business is good.", shkname(shkp));
	else
		pline("%s talks about the problem of shoplifters.", shkname(shkp));
}
#endif  /* SOUNDS */

#ifdef KOPS
static void
kops_gone(silent)
register boolean silent;
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
	if(cnt && !silent)
		pline("The Kops (disappointed) disappear into thin air.");
	allow_kops = TRUE;
}
#endif	/* KOPS */

static long
cost_per_charge(otmp)
register struct obj *otmp;
{
	register long tmp = 0L;
	register struct monst *shkp = shop_keeper(*u.ushops);

	if(!shkp || !inhishop(shkp)) return(0L); /* insurance */
	tmp = get_cost(otmp, shkp);

	/* The idea is to make the exhaustive use of */
	/* an unpaid item more expensive than buying */
	/* it outright.				     */
	if(otmp->otyp == MAGIC_LAMP) {			 /* 1 */
		tmp += tmp / 3L;
	} else if(otmp->otyp == MAGIC_MARKER) {		 /* 70 - 100 */
		/* no way to determine in advance   */
		/* how many charges will be wasted. */
		/* so, arbitrarily, one half of the */
		/* price per use.		    */
		tmp /= 2L;
	} else if(otmp->otyp == BAG_OF_TRICKS ||	 /* 1 - 20 */
		  otmp->otyp == HORN_OF_PLENTY) {
		tmp /= 5L;
	} else if(otmp->otyp == CRYSTAL_BALL ||		 /* 1 - 5 */
		  otmp->otyp == OIL_LAMP ||		 /* 1 - 10 */
		  otmp->otyp == BRASS_LANTERN ||
		 (otmp->otyp >= MAGIC_FLUTE &&
		  otmp->otyp <= DRUM_OF_EARTHQUAKE) ||	 /* 5 - 9 */
		  otmp->oclass == WAND_CLASS) {		 /* 3 - 11 */
		    if (otmp->spe > 1) tmp /= 4L;
	} else if (otmp->oclass == SPBOOK_CLASS) {
		    tmp -= tmp / 5L;
	} else if (otmp->otyp == CAN_OF_GREASE)
		    tmp /= 10L;
	return(tmp);
}

/* for using charges of unpaid objects */
void
check_unpaid(otmp)
register struct obj *otmp;
{
	if(!*u.ushops) return;

	if(otmp->oclass != SPBOOK_CLASS && otmp->spe <= 0) return;

	if(otmp->unpaid) {
		register long tmp = cost_per_charge(otmp);
		register struct monst *shkp = shop_keeper(*u.ushops);

		if(!shkp || !inhishop(shkp)) return;

		if(otmp->oclass == SPBOOK_CLASS && tmp > 0L)
		    pline("\"%sYou owe%s %ld zorkmids.\"",
			rn2(2) ? "This is no free library, cad!  " : "",
			ESHK(shkp)->debit > 0L ? " additional" : "", tmp);
		ESHK(shkp)->debit += tmp;
		exercise(A_WIS, TRUE);		/* you just got info */
	}
}

void
costly_gold(x, y, amount)
register xchar x, y;
register long amount;
{
	register long delta;
	register struct monst *shkp;
	register struct eshk *eshkp;

	if(!costly_spot(x, y)) return;
	/* shkp now guaranteed to exist by costly_spot() */
	shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

	eshkp = ESHK(shkp);
	if(eshkp->credit >= amount) {
	    if(eshkp->credit > amount)
		Your("credit is reduced by %ld zorkmid%s.",
					amount, plur(amount));
	    else Your("credit is erased.");
	    eshkp->credit -= amount;
	} else {
	    delta = amount - eshkp->credit;
	    if(eshkp->credit)
		Your("credit is erased.");
	    if(eshkp->debit)
		Your("debt increases by %ld zorkmid%s.",
					delta, plur(delta));
	    else You("owe %s %ld zorkmid%s.",
				shkname(shkp), delta, plur(delta));
	    eshkp->debit += delta;
	    eshkp->loan += delta;
	    eshkp->credit = 0L;
	}
}

/* used in domove to block diagonal shop-exit */
/* x,y should always be a door */
boolean
block_door(x,y)
register xchar x, y;
{
	register int roomno = *in_rooms(x, y, SHOPBASE);
	register struct monst *shkp;

	if(roomno < 0 || !IS_SHOP(roomno)) return(FALSE);
	if(!IS_DOOR(levl[x][y].typ)) return(FALSE);
	if(roomno != *u.ushops) return(FALSE);

	if(!(shkp = shop_keeper((char)roomno)) || !inhishop(shkp))
		return(FALSE);

	if(shkp->mx == ESHK(shkp)->shk.x && shkp->my == ESHK(shkp)->shk.y
	    /* Actually, the shk should be made to block _any_
	     * door, including a door the player digs, if the
	     * shk is within a 'jumping' distance.
	     */
	    && ESHK(shkp)->shd.x == x && ESHK(shkp)->shd.y == y
	    && shkp->mcanmove && !shkp->msleep
	    && (ESHK(shkp)->debit || ESHK(shkp)->billct ||
		ESHK(shkp)->robbed)) {
		pline("%s%s blocks your way!", shkname(shkp),
				Invis ? " senses your motion and" : "");
		return(TRUE);
	}
	return(FALSE);
}

/* used in domove to block diagonal shop-entry */
/* u.ux, u.uy should always be a door */
boolean
block_entry(x,y)
register xchar x, y;
{
	register xchar sx, sy;
	register int roomno;
	register struct monst *shkp;

	if(!(IS_DOOR(levl[u.ux][u.uy].typ) &&
		levl[u.ux][u.uy].doormask == D_BROKEN)) return(FALSE);

	roomno = *in_rooms(x, y, SHOPBASE);
	if(roomno < 0 || !IS_SHOP(roomno)) return(FALSE);
	if(!(shkp = shop_keeper((char)roomno)) || !inhishop(shkp))
		return(FALSE);

	if(ESHK(shkp)->shd.x != u.ux || ESHK(shkp)->shd.y != u.uy)
		return(FALSE);

	sx = ESHK(shkp)->shk.x;
	sy = ESHK(shkp)->shk.y;

	if(shkp->mx == sx && shkp->my == sy
		&& shkp->mcanmove && !shkp->msleep
		&& (x == sx-1 || x == sx+1 || y == sy-1 || y == sy+1)
		&& (Invis || carrying(PICK_AXE))
	  ) {
		pline("%s%s blocks your way!", shkname(shkp),
				Invis ? " senses your motion and" : "");
		return(TRUE);
	}
	return(FALSE);
}

#endif /* OVLB */

#ifdef __SASC
sasc_bug(struct obj *op, unsigned x){
	op->unpaid=x;
}
#endif

/*shk.c*/
