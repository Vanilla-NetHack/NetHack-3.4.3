/*	SCCS Id: @(#)invent.c	3.1	92/12/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

#define	NOINVSYM	'#'
#define	CONTAINED_SYM	'>'	/* designator for inside a container */

#ifdef OVL1
static void NDECL(reorder_invent);
static boolean FDECL(mergable,(struct obj *,struct obj *));
static int FDECL(merged,(struct obj *,struct obj *,int));
#endif /* OVL1 */
STATIC_DCL void FDECL(assigninvlet,(struct obj *));
STATIC_DCL void FDECL(unlinkinv,(struct obj*));
STATIC_DCL void FDECL(compactify,(char *));
STATIC_PTR int FDECL(ckunpaid,(struct obj *));
#ifdef OVLB
static struct obj *FDECL(find_unpaid,(struct obj *,struct obj **));
static boolean NDECL(wearing_armor);
static boolean FDECL(is_worn,(struct obj *));
#endif /* OVLB */
STATIC_DCL char FDECL(obj_to_let,(struct obj *));

#ifdef OVLB

static int lastinvnr = 51;	/* 0 ... 51 (never saved&restored) */

char inv_order[] = {	/* manipulated in options.c, used below */
	AMULET_CLASS, WEAPON_CLASS, ARMOR_CLASS, FOOD_CLASS, SCROLL_CLASS,
	SPBOOK_CLASS, POTION_CLASS, RING_CLASS, WAND_CLASS, TOOL_CLASS, 
	GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, 0 };

#ifdef WIZARD
/* wizards can wish for venom, which will become an invisible inventory
 * item without this.  putting it in inv_order would mean venom would
 * suddenly become a choice for all the inventory-class commands, which
 * would probably cause mass confusion.  the test for inventory venom
 * is only WIZARD and not wizard because the wizard can leave venom lying
 * around on a bones level for normal players to find.
 */
static char venom_inv[] = { VENOM_CLASS, 0 };	/* (constant) */
#endif

STATIC_OVL void
assigninvlet(otmp)
register struct obj *otmp;
{
	boolean inuse[52];
	register int i;
	register struct obj *obj;

	for(i = 0; i < 52; i++) inuse[i] = FALSE;
	for(obj = invent; obj; obj = obj->nobj) if(obj != otmp) {
		i = obj->invlet;
		if('a' <= i && i <= 'z') inuse[i - 'a'] = TRUE; else
		if('A' <= i && i <= 'Z') inuse[i - 'A' + 26] = TRUE;
		if(i == otmp->invlet) otmp->invlet = 0;
	}
	if((i = otmp->invlet) &&
	    (('a' <= i && i <= 'z') || ('A' <= i && i <= 'Z')))
		return;
	for(i = lastinvnr+1; i != lastinvnr; i++) {
		if(i == 52) { i = -1; continue; }
		if(!inuse[i]) break;
	}
	otmp->invlet = (inuse[i] ? NOINVSYM :
			(i < 26) ? ('a'+i) : ('A'+i-26));
	lastinvnr = i;
}

#endif /* OVLB */
#ifdef OVL1

/* note: assumes ASCII; toggling a bit puts lowercase in front of uppercase */
#define inv_rank(o) ((o)->invlet ^ 040)

/* sort the inventory; used by addinv() and doorganize() */
static void
reorder_invent()
{
	struct obj *otmp, *prev, *next;
	boolean need_more_sorting;

	do {
	    /*
	     * We expect at most one item to be out of order, so this
	     * isn't nearly as inefficient as it may first appear.
	     */
	    need_more_sorting = FALSE;
	    for (otmp = invent, prev = 0; otmp; ) {
		next = otmp->nobj;
		if (next && inv_rank(next) < inv_rank(otmp)) {
		    need_more_sorting = TRUE;
		    if (prev) prev->nobj = next;
		    else      invent = next;
		    otmp->nobj = next->nobj;
		    next->nobj = otmp;
		    prev = next;
		} else {
		    prev = otmp;
		    otmp = next;
		}
	    }
	} while (need_more_sorting);
}

#undef inv_rank

/* merge obj with otmp and delete obj if types agree */
STATIC_OVL int
merged(otmp, obj, lose)
register struct obj *otmp, *obj;
register int lose;
{
	if(mergable(otmp, obj)) {
		/* Approximate age: we do it this way because if we were to
		 * do it "accurately" (merge only when ages are identical)
		 * we'd wind up never merging any corpses.
		 * otmp->age = otmp->age*(1-proportion) + obj->age*proportion;
		 */
		otmp->age = ((otmp->age*otmp->quan) + (obj->age*obj->quan))
			/ (otmp->quan + obj->quan);
		otmp->quan += obj->quan;
		otmp->owt += obj->owt;
		if(!otmp->onamelth && obj->onamelth)
			otmp = oname(otmp, ONAME(obj), 1);
		if(lose) freeobj(obj);
		obfree(obj,otmp);	/* free(obj), bill->otmp */
		return(1);
	} else	return(0);
}

struct obj *
addinv(obj)
register struct obj *obj;
{
	register struct obj *otmp, *prev;

	if (obj->otyp == GOLD_PIECE) {
		u.ugold += obj->quan;
		flags.botl = 1;
		return obj;
	} else if (obj->otyp == AMULET_OF_YENDOR) {
		if (u.uhave.amulet) impossible ("already have amulet?");
		u.uhave.amulet = 1;
	} else if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
		if (u.uhave.menorah) impossible ("already have candelabrum?");
		u.uhave.menorah = 1;
	} else if (obj->otyp == BELL_OF_OPENING) {
		if (u.uhave.bell) impossible ("already have silver bell?");
		u.uhave.bell = 1;
	} else if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
		if (u.uhave.book) impossible ("already have the book?");
		u.uhave.book = 1;
#ifdef MULDGN
	} else if (is_quest_artifact(obj)) {
		if (u.uhave.questart) impossible ("already have the artifact?");
		u.uhave.questart = 1;
		artitouch();
		set_artifact_intrinsic(obj, 1, W_ART);
#endif
	} else if(obj->oartifact) {
		set_artifact_intrinsic(obj, 1, W_ART);
	}
	/* merge if possible; find end of chain in the process */
	for (prev = 0, otmp = invent; otmp; prev = otmp, otmp = otmp->nobj)
	    if (merged(otmp, obj, 0)) {
		obj = otmp;
		goto added;
	    }
	/* didn't merge, so insert into chain */
	if (flags.invlet_constant || !prev) {
	    if (flags.invlet_constant) assigninvlet(obj);
	    obj->nobj = invent;		/* insert at beginning */
	    invent = obj;
	    if (flags.invlet_constant) reorder_invent();
	} else {
	    prev->nobj = obj;		/* insert at end */
	    obj->nobj = 0;
	}

added:
	if (obj->otyp == LUCKSTONE
	    || (obj->oartifact && spec_ability(obj, SPFX_LUCK))) {
		/* new luckstone must be in inventory by this point
		 * for correct calculation */
		if (stone_luck(TRUE) >= 0) u.moreluck = LUCKADD;
		else u.moreluck = -LUCKADD;
	} else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP)
		check_lamps();
	update_inventory();
	return(obj);
}

#endif /* OVL1 */
#ifdef OVLB

/* Add an item to the inventory unless we're fumbling, and give a message.
 * If there aren't any free inventory slots, we'll drop it instead.
 * If both success and failure messages are NULL, then we're just doing the
 * fumbling/slot-limit checking for a silent grab.
 * Note: will drop the whole bunch if the object merges first.
 */
struct obj *
hold_another_object(obj, drop_fmt, drop_arg, hold_msg)
struct obj *obj;
const char *drop_fmt, *drop_arg, *hold_msg;
{
	long oquan = obj->quan;
	if (!Blind) obj->dknown = 1;	/* maximize mergibility */
	if (Fumbling) {
		if (drop_fmt) pline(drop_fmt, drop_arg);
		dropy(obj);
	} else {
		obj = addinv(obj);
		if (inv_cnt() > 52
		    || ((obj->otyp != LOADSTONE || !obj->cursed)
			&& near_capacity() >= OVERLOADED)) {
			if (drop_fmt) pline(drop_fmt, drop_arg);
			dropx(obj);
		} else {
			if (hold_msg || drop_fmt) prinv(hold_msg, obj, oquan);
		}
	}
	return obj;
}

void
useup(obj)
register struct obj *obj;
{
	/*  Note:  This works correctly for containers because they */
	/*	   (containers) don't merge.			    */
	if(obj->quan > 1L){
#ifndef NO_SIGNAL
		obj->in_use = FALSE;	/* no longer in use */
#endif
		obj->quan--;
		obj->owt = weight(obj);
	} else {
		setnotworn(obj);
		freeinv(obj);
		obfree(obj, (struct obj *) 0);	/* deletes contents also */
	}
}

#endif /* OVLB */
#ifdef OVL3

/* used by freeinv and doorganize to do list manipulation */
STATIC_OVL
void
unlinkinv(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if(obj == invent)
		invent = invent->nobj;
	else {
		for(otmp = invent; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp->nobj) panic("unlinkinv");
		otmp->nobj = obj->nobj;
	}
	obj->nobj = 0;
}

void
freeinv(obj)
register struct obj *obj;
{
	unlinkinv(obj);

	if (obj->otyp == GOLD_PIECE) {
		u.ugold -= obj->quan;
		flags.botl = 1;
		return;
	} else if (obj->otyp == AMULET_OF_YENDOR) {
		if (!u.uhave.amulet) impossible ("don't have amulet?");
		u.uhave.amulet = 0;
	} else if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
		if (!u.uhave.menorah) impossible ("don't have candelabrum?");
		u.uhave.menorah = 0;
	} else if (obj->otyp == BELL_OF_OPENING) {
		if (!u.uhave.bell) impossible ("don't have silver bell?");
		u.uhave.bell = 0;
	} else if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
		if (!u.uhave.book) impossible ("don't have the book?");
		u.uhave.book = 0;
#ifdef MULDGN
	} else if (is_quest_artifact(obj)) {
		if(!u.uhave.questart) impossible ("don't have the artifact?");
		u.uhave.questart = 0;
		set_artifact_intrinsic(obj, 0, W_ART);
#endif
	} else if (obj->oartifact) {
		set_artifact_intrinsic(obj, 0, W_ART);
	} else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP
		   || obj->otyp == BRASS_LANTERN) {
		if (obj->lamplit) {
			obj->lamplit = 0;
			if (!Blind) pline("%s goes out!", The(xname(obj)));
		}
		check_lamps();
	} else if (obj->otyp == LOADSTONE) {
		curse(obj);
	} else if (obj->otyp == LUCKSTONE
		   || (obj->oartifact && spec_ability(obj, SPFX_LUCK))) {
		int luckbon = stone_luck(TRUE);
		if (!luckbon && !carrying(LUCKSTONE)) u.moreluck = 0;
		else if (luckbon >= 0) u.moreluck = LUCKADD;
		else u.moreluck = -LUCKADD;
		flags.botl = 1;
	}
	update_inventory();
}

void
delallobj(x, y)
int x, y;
{
	struct obj *otmp, *otmp2;

	for (otmp = level.objects[x][y]; otmp; otmp = otmp2) {
		otmp2 = otmp->nexthere;
		if (otmp == uball)
			unpunish();
		if (otmp == uchain)
			continue;
		delobj(otmp);
	}
}

#endif /* OVL3 */
#ifdef OVL2

/* destroy object in fobj chain (if unpaid, it remains on the bill) */
void
delobj(obj)
register struct obj *obj;
{
#ifdef WALKIES
	if(obj->otyp == LEASH && obj->leashmon != 0) o_unleash(obj);
#endif
	freeobj(obj);
	newsym(obj->ox,obj->oy);
	obfree(obj, (struct obj *) 0);	/* frees contents also */
}

/* unlink obj from chain starting with fobj */
void
freeobj(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if (obj == fobj)
	    fobj = fobj->nobj;
	else {
	    for(otmp = fobj; otmp; otmp = otmp->nobj)
		if (otmp->nobj == obj) {
		    otmp->nobj = obj->nobj;
		    break;
		}
	    if (!otmp) panic("error in freeobj");
	}
	remove_object(obj);
}

#endif /* OVL2 */
#ifdef OVL0

struct obj *
sobj_at(n,x,y)
register int n, x, y;
{
	register struct obj *otmp;

	for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if(otmp->otyp == n)
		    return(otmp);
	return((struct obj *)0);
}

#endif /* OVL0 */
#ifdef OVLB

int
carried(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp == obj) return(1);
	return(0);
}

struct obj *
carrying(type)
register int type;
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == type)
			return(otmp);
	return((struct obj *) 0);
}

boolean
have_lizard()
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == CORPSE && otmp->corpsenm == PM_LIZARD)
			return(TRUE);
	return(FALSE);
}

struct obj *
o_on(id, objchn)
unsigned int id;
register struct obj *objchn;
{
	struct obj *temp;

	while(objchn) {
		if(objchn->o_id == id) return(objchn);
		if (Is_container(objchn) && (temp = o_on(id,objchn->cobj)))
			return temp;
		objchn = objchn->nobj;
	}
	return((struct obj *) 0);
}

boolean
obj_here(obj, x, y)
register struct obj *obj;
int x, y;
{
	register struct obj *otmp;

	for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if(obj == otmp) return(TRUE);
	return(FALSE);
}

#endif /* OVLB */
#ifdef OVL2

struct obj *
g_at(x,y)
register int x, y;
{
	register struct obj *obj = level.objects[x][y];
	while(obj) {
	    if (obj->otyp == GOLD_PIECE) return obj;
	    obj = obj->nexthere;
	}
	return((struct obj *)0);
}

#endif /* OVL2 */
#ifdef OVLB

/* Make a gold object from the hero's gold. */
struct obj *
mkgoldobj(q)
register long q;
{
	register struct obj *otmp;

	otmp = mksobj(GOLD_PIECE, FALSE, FALSE);
	u.ugold -= q;
	otmp->quan = q;
	otmp->owt = weight(otmp);
	flags.botl = 1;
	return(otmp);
}

#endif /* OVLB */
#ifdef OVL1

STATIC_OVL void
compactify(buf)
register char *buf;
/* compact a string of inventory letters by dashing runs of letters */
{
	register int i1 = 1, i2 = 1;
	register char ilet, ilet1, ilet2;

	ilet2 = buf[0];
	ilet1 = buf[1];
	buf[++i2] = buf[++i1];
	ilet = buf[i1];
	while(ilet) {
		if(ilet == ilet1+1) {
			if(ilet1 == ilet2+1)
				buf[i2 - 1] = ilet1 = '-';
			else if(ilet2 == '-') {
				buf[i2 - 1] = ++ilet1;
				buf[i2] = buf[++i1];
				ilet = buf[i1];
				continue;
			}
		}
		ilet2 = ilet1;
		ilet1 = ilet;
		buf[++i2] = buf[++i1];
		ilet = buf[i1];
	}
}

/*
 * getobj returns:
 *	struct obj *xxx:	object to do something with.
 *	(struct obj *) 0	error return: no object.
 *	&zeroobj		explicitly no object (as in w-).
 */
struct obj *
getobj(let,word)
register const char *let,*word;
{
	register struct obj *otmp;
	register char ilet;
	char buf[BUFSZ], qbuf[QBUFSZ];
	char lets[BUFSZ];
	register int foo = 0;
	register char *bp = buf;
	xchar allowcnt = 0;	/* 0, 1 or 2 */
	boolean allowgold = FALSE, usegold = FALSE;
		/* Two possibilities: they can't use gold because it's illegal,
		 * or they can't use gold because they don't have any.
		 */
	boolean allowall = FALSE;
	boolean allownone = FALSE;
	xchar foox = 0;
	long cnt;
	boolean prezero = FALSE;

	if(*let == ALLOW_COUNT) let++, allowcnt = 1;
	if(*let == GOLD_CLASS) let++,
		usegold = TRUE, allowgold = (u.ugold ? TRUE : FALSE);
#ifdef POLYSELF
	/* Equivalent of an "ugly check" for gold */
	if (usegold && !strcmp(word, "eat") && !metallivorous(uasmon))
		usegold = allowgold = FALSE;
#endif
	if(*let == ALL_CLASSES) let++, allowall = TRUE;
	if(*let == ALLOW_NONE) let++, allownone = TRUE;
	/* "ugly check" for reading fortune cookies, part 1 */
	if(allowall && !strcmp(word, "read")) allowall = FALSE;

	if(allownone) *bp++ = '-';
	if(allowgold) *bp++ = def_oc_syms[GOLD_CLASS];
	if(bp > buf && bp[-1] == '-') *bp++ = ' ';

	ilet = 'a';
	for(otmp = invent; otmp; otmp = otmp->nobj){
	    if(!*let || index(let, otmp->oclass)) {
		bp[foo++] = flags.invlet_constant ? otmp->invlet : ilet;

		/* ugly check: remove inappropriate things */
		if((!strcmp(word, "take off") &&
		    (!(otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))
		     || (otmp==uarm && uarmc)
#ifdef TOURIST
		     || (otmp==uarmu && (uarm || uarmc))
#endif
		    ))
		|| (!strcmp(word, "wear") &&
		     (otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)))
							/* already worn */
		|| (!strcmp(word, "wield") &&
		    (otmp->owornmask & W_WEP))
		    ) {
			foo--;
			foox++;
		}

		/* Second ugly check; unlike the first it won't trigger an
		 * "else" in "you don't have anything else to ___".
		 */
		else if ((!strcmp(word, "wear") &&
		    (otmp->oclass == TOOL_CLASS &&
		     otmp->otyp != BLINDFOLD && otmp->otyp != TOWEL))
#ifdef POLYSELF
		|| (!strcmp(word, "eat") && !is_edible(otmp))
#endif
		|| (!strcmp(word, "can") &&
		    (otmp->otyp != CORPSE))
		|| (!strcmp(word, "write with") &&
		    (otmp->oclass == TOOL_CLASS &&
		     otmp->otyp != MAGIC_MARKER && otmp->otyp != TOWEL))
		|| (!strcmp(word, "rub") &&
		    (otmp->oclass == TOOL_CLASS &&
		     otmp->otyp != OIL_LAMP && otmp->otyp != MAGIC_LAMP &&
		     otmp->otyp != BRASS_LANTERN))
		|| (!strcmp(word, "wield") &&
		    (otmp->oclass == TOOL_CLASS &&
		     otmp->otyp != PICK_AXE && otmp->otyp != UNICORN_HORN))
		    )
			foo--;
	    } else {

		/* "ugly check" for reading fortune cookies, part 2 */
		if ((!strcmp(word, "read") && otmp->otyp == FORTUNE_COOKIE))
			allowall = TRUE;
	    }

	    if(ilet == 'z') ilet = 'A'; else ilet++;
	}
	bp[foo] = 0;
	if(foo == 0 && bp > buf && bp[-1] == ' ') *--bp = 0;
	Strcpy(lets, bp);	/* necessary since we destroy buf */
	if(foo > 5)			/* compactify string */
		compactify(bp);

	if(!foo && !allowall && !allowgold && !allownone) {
		You("don't have anything %sto %s.",
			foox ? "else " : "", word);
		return((struct obj *)0);
	}
	for(;;) {
		cnt = 0;
		if (allowcnt == 2) allowcnt = 1;  /* abort previous count */
		if(!buf[0]) {
			Sprintf(qbuf, "What do you want to %s? [*]", word);
		} else {
			Sprintf(qbuf, "What do you want to %s? [%s or ?*]",
				word, buf);
		}
#ifdef REDO
		if(!in_doagain)
		    ilet = yn_function(qbuf, NULL, '\0');
		else
#endif
		    ilet = readchar();
		if(ilet == '0') prezero = TRUE;
		while(digit(ilet) && allowcnt) {
#ifdef REDO
			if (ilet != '?' && ilet != '*')	savech(ilet);
#endif
			cnt = 10*cnt + (ilet - '0');
			allowcnt = 2;	/* signal presence of cnt */
			ilet = readchar();
		}
		if(digit(ilet)) {
			pline("No count allowed with this command.");
			continue;
		}
		if(index(quitchars,ilet)) {
		    if(flags.verbose)
			pline("Never mind.");
		    return((struct obj *)0);
		}
		if(ilet == '-') {
			return(allownone ? &zeroobj : (struct obj *) 0);
		}
		if(ilet == def_oc_syms[GOLD_CLASS]) {
			if(!usegold){
				You("cannot %s gold.", word);
				return(struct obj *)0;
			} else if (!allowgold) {
				You("are not carrying any gold.");
				return(struct obj *)0;
			}
			if(cnt == 0 && prezero) return((struct obj *)0);
			/* Historic note: early Nethack had a bug which was
			 * first reported for Larn, where trying to drop 2^32-n
			 * gold pieces was allowed, and did interesting things
			 * to your money supply.  The LRS is the tax bureau
			 * from Larn.
			 */
			if(cnt < 0) {
	pline("The LRS would be very interested to know you have that much.");
				return(struct obj *)0;
			}

			if(!(allowcnt == 2 && cnt < u.ugold))
				cnt = u.ugold;
			return(mkgoldobj(cnt));
		}
		if(allowcnt == 2 && !strcmp(word,"throw")) {
			/* permit counts for throwing gold, but don't accept
			 * counts for other things since the throw code will
			 * split off a single item anyway */
			allowcnt = 1;
			if(cnt == 0 && prezero) return((struct obj *)0);
			if(cnt > 1) {
			    You("can only throw one item at a time.");
			    continue;
			}
		}
		if(ilet == '?' || ilet == '*') {
		    ilet = display_inventory(ilet == '?' ? lets : NULL, FALSE);
		    if(!ilet) continue;
		    if(ilet == '\033') {
			if(flags.verbose)
			    pline("Never mind.");
			return((struct obj *)0);
		    }
		    /* they typed a letter (not a space) at the prompt */
		}
#ifdef REDO
		savech(ilet);
#endif
		if(flags.invlet_constant) {
			for(otmp = invent; otmp; otmp = otmp->nobj)
				if(otmp->invlet == ilet) break;
		} else {
			if(ilet >= 'A' && ilet <= 'Z') ilet += 'z' - 'A' + 1;
			ilet -= 'a';
			for(otmp = invent; otmp && ilet;
					ilet--, otmp = otmp->nobj) ;
		}
		if(!otmp) {
			You("don't have that object.");
			continue;
		} else if (cnt < 0 || otmp->quan < cnt) {
			You("don't have that many!  You have only %ld.",
			    otmp->quan);
			continue;
		}
		break;
	}
	if(!allowall && let && !index(let,otmp->oclass)) {
		pline(silly_thing_to, word);
		return((struct obj *)0);
	}
	if(allowcnt == 2) {	/* cnt given */
		if(cnt == 0) return (struct obj *)0;
		if(cnt != otmp->quan) {
			register struct obj *obj = splitobj(otmp, cnt);
		/* Very ugly kludge necessary to prevent someone from trying
		 * to drop one of several loadstones and having the loadstone
		 * now be separate.
		 */
			if (!strcmp(word, "drop") &&
			    obj->otyp == LOADSTONE && obj->cursed)
				otmp->corpsenm = obj->invlet;
			if(otmp == uwep) setuwep(obj);
		}
	}
	return(otmp);
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_PTR int
ckunpaid(otmp)
register struct obj *otmp;
{
	return((int)(otmp->unpaid));
}

static boolean
wearing_armor() {
	return(uarm || uarmc || uarmf || uarmg || uarmh || uarms
#ifdef TOURIST
		|| uarmu
#endif
		);
}

static boolean
is_worn(otmp)
register struct obj *otmp;
{
    return(!!(otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL | W_WEP)));
}

static const char NEARDATA removeables[] =
	{ ARMOR_CLASS, WEAPON_CLASS, RING_CLASS, AMULET_CLASS, TOOL_CLASS, 0 };

/* interactive version of getobj - used for Drop, Identify and */
/* Takeoff (A). Return the number of times fn was called successfully */
int
ggetobj(word, fn, mx)
register const char *word;
register int FDECL((*fn),(OBJ_P)), mx;
{
	char buf[BUFSZ], qbuf[QBUFSZ];
	register char *ip;
	register char sym;
	register int oletct = 0, iletct = 0;
	register boolean allflag = FALSE;
	char olets[20], ilets[20];
	int FDECL((*ckfn),(OBJ_P)) = (int (*)()) 0;
	xchar allowgold = (u.ugold && !strcmp(word, "drop")) ? 1 : 0; /* BAH */
	register boolean takeoff = !strcmp(word, "take off");
	struct obj *obj;
	int unpaid, oc_of_sym;

	if(takeoff && !wearing_armor() && !uwep && !uamul &&
			!uleft && !uright && !ublindf) {
		You("are not wearing anything.");
		return(0);
	}
	if(!invent && !allowgold){
		You("have nothing to %s.", word);
		return(0);
	}

	if (allowgold) ilets[iletct++] = def_oc_syms[GOLD_CLASS];
	ilets[iletct] = '\0';	/* terminate for index() */
	unpaid = 0;
	for (obj = invent; obj; obj = obj->nobj) {
		sym = (char) def_oc_syms[(int) obj->oclass];
		if (!index(ilets, sym) && (!takeoff || is_worn(obj))) {
			ilets[iletct++] = sym;
			/* necessary because of index() being used above */
			ilets[iletct] = '\0';
		}

		if (obj->unpaid) unpaid = 1;
	}

	if (!takeoff && (unpaid || invent)) {
	    ilets[iletct++] = ' ';
	    if (unpaid) ilets[iletct++] = 'u';
	    if (invent) ilets[iletct++] = 'a';
	}
	ilets[iletct] = '\0';	/* outside the if to catch iletct==0 case */

	Sprintf(qbuf,"What kinds of thing do you want to %s? [%s]",
		word, ilets);
	getlin(qbuf, buf);
	if(buf[0] == '\033') {
		clear_nhwindow(WIN_MESSAGE);
		return(0);
	}
	ip = buf;
	olets[0] = 0;
	while ((sym = *ip++) != 0) {
		if(sym == ' ') continue;
		oc_of_sym = def_char_to_objclass(sym);
		if(takeoff && !(uwep && oc_of_sym == uwep->oclass)
		   && (oc_of_sym != MAXOCLASSES)) {
		    if(!index(removeables,oc_of_sym)) {
			pline("Not applicable.");
			return(0);
		    } else if(oc_of_sym == ARMOR_CLASS && !wearing_armor()) {
			You("are not wearing any armor.");
			return(0);
		    } else if(oc_of_sym == WEAPON_CLASS && !uwep) {
			You("are not wielding anything.");
			return(0);
		    } else if(oc_of_sym == RING_CLASS && !uright && !uleft) {
			You("are not wearing rings.");
			return(0);
		    } else if(oc_of_sym == AMULET_CLASS && !uamul) {
			You("are not wearing an amulet.");
			return(0);
		    } else if(oc_of_sym == TOOL_CLASS && !ublindf) {
			You("are not wearing a blindfold.");
			return(0);
		    }
		}
		if(oc_of_sym == GOLD_CLASS) {
			if(allowgold == 1)
				(*fn)(mkgoldobj(u.ugold));
			else if(!u.ugold)
				You("have no gold.");
			allowgold = 2;
		} else if(sym == 'a' || sym == 'A')
		    allflag = TRUE;
		else if(sym == 'u' || sym == 'U')
		    ckfn = ckunpaid;
		else if (oc_of_sym == MAXOCLASSES)
			You("don't have any %c's.", sym);
		else if (oc_of_sym != VENOM_CLASS) {/* venom doesn't show up */
			if (!index(olets, oc_of_sym)) {
				olets[oletct++] = oc_of_sym;
				olets[oletct] = 0;
			}
		}
	}
	if(allowgold == 2 && !oletct)
		return 1;	/* you dropped gold (or at least tried to) */
	else
		return askchain((struct obj **)&invent, olets, allflag,
				fn, ckfn, mx, word);
}

/*
 * Walk through the chain starting at objchn and ask for all objects
 * with olet in olets (if nonNULL) and satisfying ckfn (if nonNULL)
 * whether the action in question (i.e., fn) has to be performed.
 * If allflag then no questions are asked. Max gives the max nr of
 * objects to be treated. Return the number of objects treated.
 */
int
askchain(objchn, olets, allflag, fn, ckfn, mx, word)
struct obj **objchn;
register int allflag, mx;
register const char *olets, *word;	/* olets is an Obj Class char array */
register int FDECL((*fn),(OBJ_P)), FDECL((*ckfn),(OBJ_P));
{
	register struct obj *otmp, *otmp2;
	register char sym, ilet;
	register int cnt = 0, dud = 0, tmp;
	boolean takeoff, nodot, ident, ininv;
	char qbuf[QBUFSZ];

	takeoff = !strcmp(word, "take off");
	ident = !strcmp(word, "identify");
	nodot = (!strcmp(word, "nodot") || !strcmp(word, "drop") ||
		 ident || takeoff);
	ininv = (*objchn == invent);
	/* Changed so the askchain is interrogated in the order specified.
	 * For example, if a person specifies =/ then first all rings will be
	 * asked about followed by all wands -dgk
	 */
nextclass:
	ilet = 'a'-1;
	if ((*objchn)->otyp == GOLD_PIECE) ilet--;	/* extra iteration */
	for (otmp = *objchn; otmp; otmp = otmp2) {
		if(ilet == 'z') ilet = 'A'; else ilet++;
		otmp2 = otmp->nobj;
		if (olets && *olets && otmp->oclass != *olets) continue;
		if(takeoff && !is_worn(otmp)) continue;
		if(ckfn && !(*ckfn)(otmp)) continue;
		if(!allflag) {
			Strcpy(qbuf, ininv ?
				xprname(otmp, ilet, !nodot, 0L) : doname(otmp));
			Strcat(qbuf, "?");
			sym = (takeoff || ident || otmp->quan < 2L) ?
				nyaq(qbuf) : nyNaq(qbuf);
		}
		else	sym = 'y';

		if (sym == '#') {
		 /* Number was entered; split the object unless it corresponds
		    to 'none' or 'all'.  2 special cases: cursed loadstones and
		    welded weapons (eg, multiple daggers) will remain as merged
		    unit; done to avoid splitting an object that won't be
		    droppable (even if we're picking up rather than dropping).
		  */
		    if (!yn_number)
			sym = 'n';
		    else {
			sym = 'y';
			if (yn_number < otmp->quan && !welded(otmp) &&
			    (!otmp->cursed || otmp->otyp != LOADSTONE)) {
			    struct obj *otmpx = splitobj(otmp, yn_number);
			    if (!otmpx || otmpx->nobj != otmp2)
				impossible("bad object split in askchain");
			}
		    }
		}
		switch(sym){
		case 'a':
			allflag = 1;
		case 'y':
			tmp = (*fn)(otmp);
			if(tmp < 0) goto ret;
			cnt += tmp;
			if(--mx == 0) goto ret;
		case 'n':
			if(nodot) dud++;
		default:
			break;
		case 'q':
			goto ret;
		}
	}
	if (olets && *olets && *++olets)
		goto nextclass;
	if(!takeoff && (dud || cnt)) pline("That was all.");
	else if(!dud && !cnt) pline("No applicable objects.");
ret:
	return(cnt);
}

#endif /* OVLB */
#ifdef OVL2

STATIC_OVL char
obj_to_let(obj)	/* should of course only be called for things in invent */
register struct obj *obj;
{
	register struct obj *otmp;
	register char ilet;

	if (obj->otyp == GOLD_PIECE)
		return GOLD_SYM;
	else if (flags.invlet_constant)
		return obj->invlet;
	ilet = 'a';
	for(otmp = invent; otmp && otmp != obj; otmp = otmp->nobj)
		if(++ilet > 'z') ilet = 'A';
	return(otmp ? ilet : NOINVSYM);
}

/*
 * Print the indicated quantity of the given object.  If quan == 0L then use
 * the current quantity.
 */
void
prinv(prefix, obj, quan)
const char *prefix;
register struct obj *obj;
long quan;
{
#ifdef GCC_WARN
	long savequan = 0;
#else
	long savequan;
#endif
	if ( !prefix ) prefix = "";
	if (quan) {
		savequan = obj->quan;
		obj->quan = quan;
	}
	pline("%s%s%s",
	      prefix, *prefix ? " " : "",
	      xprname(obj, obj_to_let(obj), TRUE, 0L));
	if (quan) obj->quan = savequan;
}

#endif /* OVL2 */
#ifdef OVL1

char *
xprname(obj,let,dot,cost)
register struct obj *obj;
register char let;
register boolean dot;   /* append period; (dot && cost => Iu) */
register long cost;     /* cost (for inventory of unpaid or expended items) */
{
#ifdef LINT	/* handle static char li[BUFSZ]; */
	char li[BUFSZ];
#else
	static char li[BUFSZ];
#endif
	boolean use_invlet = flags.invlet_constant && let != CONTAINED_SYM;
    /*
     * If let is:
     *	*  Then obj == NULL and we are printing a total amount.
     *	>  Then the object is contained and doesn't have an inventory letter.
     */
    if (cost != 0 || let == '*') {
	/* if dot is true, we're doing Iu, otherwise Ix */
	Sprintf(li, "%c - %-45s %6ld zorkmid%s",
		(dot && use_invlet ? obj->invlet : let),
		(let != '*' ? doname(obj) : "Total:"), cost, plur(cost));
    } else if (obj->otyp == GOLD_PIECE) {
	Sprintf(li, "%ld gold piece%s%s", obj->quan, plur(obj->quan),
		(dot ? "." : ""));
    } else {
	/* ordinary inventory display or pickup message */
	Sprintf(li, "%c - %s",
		(use_invlet ? obj->invlet : let),
		doname(obj));
	if(dot) Strcat(li,".");
    }
    return li;
}

#endif /* OVL1 */
#ifdef OVLB

int
ddoinv()
{
	(void) display_inventory(NULL, FALSE);
	return 0;
}

/*
 *  find_unpaid()
 *
 *  Scan the given list of objects.  If last_found is NULL, return the first
 *  unpaid object found.  If last_found is not NULL, then skip over unpaid
 *  objects until last_found is reached, then set last_found to NULL so the
 *  next unpaid object is returned.  This routine recursively follows
 *  containers.
 */
static struct obj *
find_unpaid(list, last_found)
    struct obj *list, **last_found;
{
    struct obj *obj;

    while (list) {
	if (list->unpaid) {
	    if (*last_found) {
		/* still looking for previous unpaid object */
		if (list == *last_found)
		    *last_found = (struct obj *) 0;
	    } else
		return (*last_found = list);
	}
	if (Is_container(list) && list->cobj) {
	    if ((obj = find_unpaid(list->cobj, last_found)) != 0)
		return obj;
	}
	list = list->nobj;
    }
    return (struct obj *) 0;
}

/*
 *  If lets == NULL or "", list all objects in the inventory.  Otherwise,
 *  list all objects with object classes that match the order in lets.
 *  The last letter could possibly be a '>' which means list unpaid contained
 *  objects.
 *  Returns the letter identifier of a selected item, or 0 (nothing was
 *  selected), or '\033' (the menu was cancelled).
 */
char
display_inventory(lets,show_cost)
register const char *lets;
boolean show_cost;
{
	register struct obj *otmp;
	struct obj *z_obj;
	register char ilet;
	char *invlet = inv_order;
	int classcount;
#if defined(LINT) || defined(GCC_WARN)
	int save_unpaid = 0;
#else
	int save_unpaid;
#endif
	long cost, totcost;
	boolean do_containers = FALSE;

	if(!invent){
		pline("Not carrying anything.");
		return 0;
	}
	if (lets != NULL) {
	    int ct = strlen(lets); /* count number of inventory slots to show */
	    /* If we've got unpaid items in containers, count all unpaid
	       objects.  At least one won't be in any inventory slot. */
	    do_containers = (ct && lets[ct-1] == CONTAINED_SYM);
	    if (do_containers && ct == 1) ct = count_unpaid(invent);
	    /* if only one item of interest, use pline instead of menus */
	    if (ct == 1) {
		if (do_containers) {	/* single non-inventory object */
		    z_obj = (struct obj *) 0;
		    if ((otmp = find_unpaid(invent, &z_obj)) != 0)
			pline(xprname(otmp, CONTAINED_SYM, TRUE,
				     (show_cost ? unpaid_cost(otmp) : 0L)));
		    else
			impossible(
		    "display_inventory: no singular unpaid contained objects");
		} else {
		    for(otmp = invent; otmp; otmp = otmp->nobj) {
			if (otmp->invlet == lets[0]) {
			    pline(xprname(otmp, lets[0], TRUE,
					 (show_cost ? unpaid_cost(otmp) : 0L)));
			    break;
			}
		    }
		}
		return 0;
	    }
	}

	start_menu(WIN_INVEN);
	cost = totcost = 0;
nextclass:
	classcount = 0;
	ilet = 'a';
	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if(flags.invlet_constant) ilet = otmp->invlet;
		if(!lets || !*lets || index(lets, ilet)) {
			if (!flags.sortpack || otmp->oclass == *invlet) {
			    if (flags.sortpack && !classcount) {
				add_menu(WIN_INVEN, 0, ATR_INVERSE,
					 let_to_name(*invlet, show_cost));
				classcount++;
			    }
			    if (show_cost) {
				totcost += cost = unpaid_cost(otmp);
				/* suppress "(unpaid)" suffix */
				save_unpaid = otmp->unpaid;
				otmp->unpaid = 0;
			    }
			    add_menu(WIN_INVEN, ilet, 0,
				     xprname(otmp, ilet, TRUE, cost));
			    if (show_cost)  otmp->unpaid = save_unpaid;
			}
		}
		if(!flags.invlet_constant) if(++ilet > 'z') ilet = 'A';
	}
	if (flags.sortpack) {
		if (*++invlet) goto nextclass;
#ifdef WIZARD
		if (--invlet != venom_inv) {
			invlet = venom_inv;
			goto nextclass;
		}
#endif
	}

	/* right now, we only do this for unpaid items, so always do cost */
	if (do_containers) {
	    if (flags.sortpack) add_menu(WIN_INVEN, 0, ATR_INVERSE,
					 let_to_name(CONTAINED_SYM, 1));
	    /*
	     *  Search through the container objects in the inventory for
	     *  unpaid items.  Note that we check each container, not
	     *  the invent list.  This is because the invent list could
	     *  have unpaid items that have been already listed.
	     */
	    for (otmp = invent; otmp; otmp = otmp->nobj) {
		if (Is_container(otmp) && otmp->cobj) {
		    z_obj = (struct obj *) 0;	/* haven't found any */
		    while (find_unpaid(otmp->cobj, (struct obj **)&z_obj)) {
			totcost += cost = unpaid_cost(z_obj);
			save_unpaid = z_obj->unpaid;
			z_obj->unpaid = 0;    /* suppress "(unpaid)" suffix */
			add_menu(WIN_INVEN, 0, 0,
				 xprname(z_obj, CONTAINED_SYM, TRUE, cost));
			z_obj->unpaid = save_unpaid;
		    }
		}
	    }
	}

	if (show_cost) {
	    /* give 'Totals' line */
	    add_menu(WIN_INVEN, 0, 0, "");
	    add_menu(WIN_INVEN, 0, 0,
		     xprname((struct obj *)0, '*', FALSE, totcost));
	}
	end_menu(WIN_INVEN, '\033', "\033 ", NULL);
	return select_menu(WIN_INVEN);
}

/*
 *  Returns the number of unpaid items within the given list.  This includes
 *  contained objects.
 */
int
count_unpaid(list)
    struct obj *list;
{
    int count = 0;

    while (list) {
	if (list->unpaid) count++;
	if (Is_container(list) && list->cobj)
	    count += count_unpaid(list->cobj);
	list = list->nobj;
    }
    return count;
}

int
dotypeinv()				/* free after Robert Viduya */
/* Changed to one type only, so you don't have to type return */
{
	char c, ilet;
	char stuff[BUFSZ];
	register int stct;
	register struct obj *otmp;
	boolean billx = *u.ushops && doinvbill(0);
	boolean do_unpd = FALSE;
	int unpd, class;

	if (!invent && !u.ugold && !billx) {
	    You("aren't carrying anything.");
	    return 0;
	}

	Strcpy(stuff, "What type of object do you want an inventory of? [");
	/* collect a list of classes of objects carried, for use as a prompt */
	stct = collect_obj_classes(eos(stuff), invent, FALSE, (u.ugold != 0));
	unpd = count_unpaid(invent);
	if(unpd) Strcat(stuff, "u");
	if(billx) Strcat(stuff, "x");
	Strcat(stuff, "]");

	if(stct > 1) {
	  c = yn_function(stuff, NULL, '\0');
#ifdef REDO
	    savech(c);
#endif
	    if(c == '\0') {
		clear_nhwindow(WIN_MESSAGE);
		return 0;
	    }
	} else
	    c = stuff[0];

	if(c == 'x' || c == 'X') {
	    if(billx)
		(void) doinvbill(1);
	    else
		pline("No used-up objects on the shopping bill.");
	    return(0);
	}

	if (c == 'u' || c == 'U') {
	    if (!unpd) {
		You("are not carrying any unpaid objects.");
		return(0);
	    }
	    do_unpd = TRUE;
	}

	class = def_char_to_objclass(c);	/* change to object class */

	if(class == GOLD_CLASS)
	    return(doprgold());

	/* collect all items which match the selected objclass */
	stct = 0;
	ilet = 'a';
	for (otmp = invent; otmp; otmp = otmp->nobj) {
	    if(flags.invlet_constant) ilet = otmp->invlet;
	    if (class == otmp->oclass || (do_unpd && otmp->unpaid)) {
		stuff[stct++] = ilet;
		--unpd;	/* decrement unpaid count */
	    }
	    if(!flags.invlet_constant) if(++ilet > 'z') ilet = 'A';
	}
	/* unpd is now the number of unpaid contained objects */
	if (do_unpd && unpd)
	    stuff[stct++] = CONTAINED_SYM;	/* list contained unpaid items */

	stuff[stct] = '\0';
	if(stct == 0)
		You("have no such objects.");
	else
		(void) display_inventory(stuff, do_unpd);

	return(0);
}

/* look at what is here */
int
dolook()
{
	register struct obj *otmp, *otmp0;
	struct trap *trap;
	const char *verb = Blind ? "feel" : "see";
	const char *dfeature = (char*) 0;
	char fbuf[BUFSZ], fbuf2[BUFSZ];
	int ct;
	boolean no_article = FALSE;
	winid tmpwin;

	if(u.uswallow) {
		You("%s no objects here.", verb);
		return(!!Blind);
	}
	read_engr_at(u.ux, u.uy); /* Eric Backus */
	if ((trap = t_at(u.ux,u.uy)) && trap->tseen)
		pline("There is a%s here.", traps[trap->ttyp]);

	otmp0 = level.objects[u.ux][u.uy];

	if(IS_DOOR(levl[u.ux][u.uy].typ))  {
		switch(levl[u.ux][u.uy].doormask) {
		    case D_NODOOR:
			dfeature = "doorway"; break;
		    case D_ISOPEN:
			dfeature = "open door"; break;
		    case D_BROKEN:
			dfeature = "broken door"; break;
		    default:
			dfeature = "closed door";
		}
	} else if(IS_FOUNTAIN(levl[u.ux][u.uy].typ))
		/* added by GAN 10/30/86 */
		dfeature = "fountain";
	else if(IS_THRONE(levl[u.ux][u.uy].typ))
		dfeature = "opulent throne";
	else if(is_lava(u.ux,u.uy))
		dfeature = "molten lava",  no_article = TRUE;
	else if(is_ice(u.ux,u.uy))
		dfeature = "ice",  no_article = TRUE;
	else if(is_pool(u.ux,u.uy) && !Underwater)
		dfeature = "pool of water";
#ifdef SINKS
	else if(IS_SINK(levl[u.ux][u.uy].typ))
		dfeature = "kitchen sink";
#endif
	else if(IS_ALTAR(levl[u.ux][u.uy].typ))  {
		Sprintf(fbuf2, "altar to %s (%s)",
			a_gname(),
			align_str(Amask2align(levl[u.ux][u.uy].altarmask
							    & ~AM_SHRINE)));
		dfeature = fbuf2;
	} else if(u.ux == xupstair && u.uy == yupstair)
		dfeature = "stairway up";
	else if(u.ux == xdnstair && u.uy == ydnstair)
		dfeature = "stairway down";
	else if(u.ux == sstairs.sx && u.uy == sstairs.sy) {
		if (sstairs.up)
			dfeature = "stairway up";
		else
			dfeature = "stairway down";
	} else if(u.ux == xupladder && u.uy == yupladder)
		dfeature = "ladder up";
	else if(u.ux == xdnladder && u.uy == ydnladder)
		dfeature = "ladder down";
	else if (levl[u.ux][u.uy].typ == DRAWBRIDGE_DOWN)
		dfeature = "lowered drawbridge";

	if (Blind) {
		You("try to feel what is %s.",
		    Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ?
			"floating here" :
			"lying here on the floor");
	 	if (Levitation &&
		    !Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz)) {
			pline("But you can't reach it!");
			return(0);
	 	}
	}

	if (dfeature)
		Sprintf(fbuf, "There is %s%s here.",
			no_article ? "" :
				index(vowels,dfeature[0]) ? "an " : "a ",
			dfeature);

	if(!otmp0 || (is_pool(u.ux,u.uy) && !Underwater)) {
	    	if(dfeature) pline(fbuf);
		if(Blind || !dfeature)
			You("%s no objects here.", verb);
		return(!!Blind);
	}
	/* we know there is something here */

	/* find out if there is more than one object there */
	for (ct = 0, otmp = otmp0; otmp; otmp = otmp->nexthere)
	    if (++ct > 1) break;

	if (ct == 1) {
	    if (dfeature) pline(fbuf);
	    You("%s here %s.", verb, doname(otmp0));
	} else {
	    display_nhwindow(NHW_MESSAGE, FALSE);
	    tmpwin = create_nhwindow(NHW_MENU);
	    if(dfeature) {
		putstr(tmpwin, 0, fbuf);
		putstr(tmpwin, 0, "");
	    }
	    putstr(tmpwin, 0, "Things that are here:");
	    for(otmp = otmp0; otmp; otmp = otmp->nexthere) {
		putstr(tmpwin, 0, doname(otmp));

		if(Blind  && !uarmg &&
#ifdef POLYSELF
			    !resists_ston(uasmon) &&
#endif
			    (otmp->otyp == CORPSE &&
					otmp->corpsenm == PM_COCKATRICE)) {
#if defined(POLYSELF)
		    if(poly_when_stoned(uasmon)) {
			You("touched the cockatrice corpse with your bare hands.");
			(void) polymon(PM_STONE_GOLEM);
		    } else
#endif
		    {
			pline("Touching the cockatrice corpse is a fatal mistake...");
			You("turn to stone...");
			killer_format = KILLED_BY_AN;
			killer = "cockatrice corpse";
			done(STONING);
		    }
		}
	    }
	    display_nhwindow(tmpwin, TRUE);
	    destroy_nhwindow(tmpwin);
	}
	return(!!Blind);
}

#endif /* OVLB */
#ifdef OVL1

void
stackobj(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	for(otmp = level.objects[obj->ox][obj->oy]; otmp; otmp = otmp->nexthere)
		if(otmp != obj && merged(obj,otmp,1))
			break;
	return;
}

static boolean
mergable(otmp, obj)	/* returns TRUE if obj  & otmp can be merged */
	register struct obj *otmp, *obj;
{
	if(obj->otyp != otmp->otyp || obj->unpaid != otmp->unpaid ||
	   obj->spe != otmp->spe || obj->dknown != otmp->dknown ||
	   (obj->bknown != otmp->bknown && pl_character[0] != 'P') ||
	   obj->cursed != otmp->cursed || obj->blessed != otmp->blessed ||
	   obj->no_charge != otmp->no_charge ||
	   obj->obroken != otmp->obroken ||
	   obj->otrapped != otmp->otrapped ||
	   obj->oeroded != otmp->oeroded)
	    return(FALSE);

	if((obj->oclass==WEAPON_CLASS || obj->oclass==ARMOR_CLASS) &&
	   (obj->oerodeproof!=otmp->oerodeproof || obj->rknown!=otmp->rknown))
		return FALSE;

	if(obj->oclass == FOOD_CLASS && (obj->oeaten != otmp->oeaten ||
		obj->orotten != otmp->orotten))
		return(FALSE);

	if(obj->otyp == CORPSE || obj->otyp == EGG || obj->otyp == TIN) {
		if((obj->corpsenm != otmp->corpsenm) ||
			(ONAME(obj) && strcmp(ONAME(obj), ONAME(otmp))))
				return FALSE;
	}

/* if they have names, make sure they're the same */
	if ( (obj->onamelth != otmp->onamelth &&
		((obj->onamelth && otmp->onamelth) || obj->otyp == CORPSE)
	     ) ||
	    (obj->onamelth &&
		    strncmp(ONAME(obj), ONAME(otmp), (int)obj->onamelth)))
		return FALSE;

	if(obj->oartifact != otmp->oartifact) return FALSE;

	if(obj->known == otmp->known ||
		!objects[otmp->otyp].oc_uses_known) {
		return(objects[obj->otyp].oc_merge);
	} else return(FALSE);
}

#endif /* OVL1 */
#ifdef OVLB

int
doprgold(){
	if(!u.ugold)
		You("do not carry any gold.");
	else
		You("are carrying %ld gold piece%s.", u.ugold, plur(u.ugold));
	return 0;
}

int
doprwep()
{
	if(!uwep) You("are empty %s.", body_part(HANDED));
	else prinv(NULL, uwep, 0L);
	return 0;
}

int
doprarm(){
	if(!wearing_armor())
		You("are not wearing any armor.");
	else {
#ifdef TOURIST
		char lets[8];
#else
		char lets[7];
#endif
		register int ct = 0;

#ifdef TOURIST
		if(uarmu) lets[ct++] = obj_to_let(uarmu);
#endif
		if(uarm) lets[ct++] = obj_to_let(uarm);
		if(uarmc) lets[ct++] = obj_to_let(uarmc);
		if(uarmh) lets[ct++] = obj_to_let(uarmh);
		if(uarms) lets[ct++] = obj_to_let(uarms);
		if(uarmg) lets[ct++] = obj_to_let(uarmg);
		if(uarmf) lets[ct++] = obj_to_let(uarmf);
		lets[ct] = 0;
		(void) display_inventory(lets, FALSE);
	}
	return 0;
}

int
doprring(){
	if(!uleft && !uright)
		You("are not wearing any rings.");
	else {
		char lets[3];
		register int ct = 0;

		if(uleft) lets[ct++] = obj_to_let(uleft);
		if(uright) lets[ct++] = obj_to_let(uright);
		lets[ct] = 0;
		(void) display_inventory(lets, FALSE);
	}
	return 0;
}

int
dopramulet(){
	if (!uamul)
		You("are not wearing an amulet.");
	else
		prinv(NULL, uamul, 0L);
	return 0;
}

int
doprtool(){
	register struct obj *otmp;
	register int ct=0;
	char lets[3]; /* Maximum: pick-axe, blindfold */

	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if (((otmp->owornmask & W_TOOL) && otmp->oclass==TOOL_CLASS)
		   || (otmp==uwep &&
		   (otmp->otyp==PICK_AXE || otmp->otyp==TIN_OPENER)))
			lets[ct++] = obj_to_let(otmp);
	}
	lets[ct] = 0;
	if (!ct) You("are not using any tools.");
	else (void) display_inventory(lets, FALSE);
	return 0;
}

/*
 * uses up an object that's on the floor, charging for it as necessary
 */
void
useupf(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	/* burn_floor_paper() keeps an object pointer that it tries to
	 * useupf() multiple times, so obj must survive if plural */
	if(obj->quan > 1L)
		otmp = splitobj(obj, obj->quan - 1L);
	else
		otmp = obj;
	if(costly_spot(otmp->ox, otmp->oy)) {
	    if(index(u.urooms, *in_rooms(otmp->ox, otmp->oy, 0)))
	        addtobill(otmp, FALSE, FALSE, FALSE);
	    else (void)stolen_value(otmp, otmp->ox, otmp->oy, FALSE, FALSE);
	}
	delobj(otmp);
}

#endif /* OVLB */
#ifdef OVL1

extern const char obj_symbols[];	/* o_init.c */
/*
 * Conversion from a symbol to a string for printing object classes.
 * This must match the array obj_symbols[].
 */
static const char NEARDATA *names[] = {
	"Illegal objects", "Amulets", "Coins", "Comestibles", "Weapons",
	"Tools", "Iron balls", "Chains", "Boulders/Statues", "Armor",
	"Potions", "Scrolls", "Wands", "Spellbooks", "Rings", "Gems"
};

static const char NEARDATA oth_symbols[] = {
#ifdef WIZARD
	VENOM_CLASS,
#endif
	CONTAINED_SYM,
	'\0'
};

static const char NEARDATA *oth_names[] = {
#ifdef WIZARD
	"Venoms",
#endif
	"Bagged/Boxed items"
};

char *
let_to_name(let,unpaid)
char let;
boolean unpaid;
{
	const char *class_name;
	const char *pos = index(obj_symbols, let);
	int len;
	static char NEARDATA *buf = NULL;
	static unsigned NEARDATA bufsiz = 0;

	if (pos)
	    class_name = names[pos - obj_symbols];
	else if ((pos = index(oth_symbols, let)) != 0)
	    class_name = oth_names[pos - oth_symbols];
	else
	    class_name = names[0];

	len = strlen(class_name)
	     + (unpaid ? sizeof("unpaid_") : 1);    /* count terminating NUL */
	if (len > bufsiz) {
	    if (buf)  free((genericptr_t)buf),  buf = NULL;
	    bufsiz = len + 10; /* add slop to avoid incremental reallocations */
	    buf = (char *) alloc(bufsiz);
	}
	if (unpaid)
	    Strcat(strcpy(buf, "Unpaid "), class_name);
	else
	    Strcpy(buf, class_name);
	return (buf);
}

#endif /* OVL1 */
#ifdef OVLB 

void
reassign()
{
	register int i;
	register struct obj *obj;

	for(obj = invent, i = 0; obj; obj = obj->nobj, i++)
		obj->invlet = (i < 26) ? ('a'+i) : ('A'+i-26);
	lastinvnr = i;
}

#endif /* OVLB */
#ifdef OVL1
int
doorganize()	/* inventory organizer by Del Lamb */
{
	register struct obj *obj, *otmp;
	register int ix, cur;
	register char let;
	char alphabet[52+1], buf[52+1];
	char qbuf[QBUFSZ];
	char allow_all[2];
	const char *adj_type;

	/* check to see if the inventory is of the fixed kind */
	if (!flags.invlet_constant ) {
		pline("Sorry, only fixed inventories can be adjusted.");
		return(0);
	}
#if 0
	/* check to see if the inventory has any gaps left in it */
	if (inv_cnt() >= 52) {
		pline("Sorry, no available letters for adjustment.");
		return(0);
	}
#endif
	/* get a pointer to the object the user wants to organize */
	allow_all[0] = ALL_CLASSES; allow_all[1] = '\0';
	if (!(obj = getobj(allow_all,"adjust"))) return(0);

	/* initialize the list with all upper and lower case letters */
	for (let = 'a', ix = 0;  let <= 'z';) alphabet[ix++] = let++;
	for (let = 'A', ix = 26; let <= 'Z';) alphabet[ix++] = let++;
	alphabet[52] = 0;

	/* blank out all the letters currently in use in the inventory */
	/* except those that will be merged with the selected object   */
	for (otmp = invent; otmp; otmp = otmp->nobj)
		if (otmp != obj && !mergable(otmp,obj))
			if (otmp->invlet <= 'Z')
				alphabet[(otmp->invlet) - 'A' + 26] = ' ';
			else	alphabet[(otmp->invlet) - 'a']	    = ' ';

	/* compact the list by removing all the blanks */
	for (ix = cur = 0; ix <= 52; ix++)
		if (alphabet[ix] != ' ') buf[cur++] = alphabet[ix];

	/* and by dashing runs of letters */
	if(cur > 5) compactify(buf);

	/* get new letter to use as inventory letter */
	for (;;) {
		Sprintf(qbuf, "Adjust letter to what [%s]?",buf);
		let = yn_function(qbuf, NULL, '\0');
		if(index(quitchars,let)) {
			pline("Never mind.");
			return(0);
		}
		if (let == '@' || !letter(let))
			pline("Select an inventory slot letter.");
		else
			break;
	}

	/* change the inventory and print the resulting item */
	adj_type = "Moving:";

	/*
	 * don't use freeinv/addinv to avoid double-touching artifacts,
	 * dousing lamps, losing luck, cursing loadstone, etc.
	 */
	unlinkinv(obj);

	for (otmp = invent; otmp;)
		if (merged(otmp,obj,0)) {
			adj_type = "Merging:";
			obj = otmp;
			otmp = otmp->nobj;
			unlinkinv(obj);
		} else {
			if (otmp->invlet == let) {
				adj_type = "Swapping:";
				otmp->invlet = obj->invlet;
			}
			otmp = otmp->nobj;
		}

	/* inline addinv (assuming flags.invlet_constant and !merged) */
	obj->invlet = let;
	obj->nobj = invent; /* insert at beginning */
	invent = obj;
	reorder_invent();

	prinv(adj_type, obj, 0L);
	update_inventory();
	return(0);
}

#endif /* OVL1 */

/*invent.c*/
