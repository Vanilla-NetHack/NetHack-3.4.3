/*	SCCS Id: @(#)invent.c	3.0	88/10/22
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

#ifdef WORM
#include "lev.h"
#include "wseg.h"
#endif

#define	NOINVSYM	'#'

static boolean FDECL(mergable,(struct obj *,struct obj *));
OSTATIC void FDECL(assigninvlet,(struct obj *));
static int FDECL(merged,(struct obj *,struct obj *,int));
OSTATIC struct obj *FDECL(mkgoldobj,(long));
#ifndef OVERLAY
static int FDECL(ckunpaid,(struct obj *));
#else
int FDECL(ckunpaid,(struct obj *));
#endif
static boolean NDECL(wearing_armor);
static boolean FDECL(is_worn,(struct obj *));
static char FDECL(obj_to_let,(struct obj *));

OSTATIC char *FDECL(xprname,(struct obj *,CHAR_P,BOOLEAN_P));

#ifdef OVLB

int lastinvnr = 51;	/* 0 ... 51 */

char inv_order[] = {
	AMULET_SYM, WEAPON_SYM, ARMOR_SYM, FOOD_SYM, SCROLL_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	POTION_SYM, RING_SYM, WAND_SYM, TOOL_SYM, GEM_SYM,
	ROCK_SYM, BALL_SYM, CHAIN_SYM, 0 };

XSTATIC void
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

/* merge obj with otmp and delete obj if types agree */
static int
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
			(void)oname(otmp, ONAME(obj), 1);
		if(lose) freeobj(obj);
		obfree(obj,otmp);	/* free(obj), bill->otmp */
		return(1);
	} else	return(0);
}

struct obj *
addinv(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if (obj->otyp == AMULET_OF_YENDOR && !obj->spe) {
		if (u.uhave_amulet) impossible ("already have amulet?");
		u.uhave_amulet = 1;
	}
	/* merge or attach to end of chain */
	if(!invent) {
		invent = obj;
		otmp = 0;
	} else
	for(otmp = invent; /* otmp */; otmp = otmp->nobj) {
		if(merged(otmp, obj, 0)) {
			obj = otmp;
			goto added;
		}
		if(!otmp->nobj) {
			otmp->nobj = obj;
			break;
		}
	}
	obj->nobj = 0;

	if(flags.invlet_constant) {
		assigninvlet(obj);
		/*
		 * The ordering of the chain is nowhere significant
		 * so in case you prefer some other order than the
		 * historical one, change the code below.
		 */
		if(otmp) {	/* find proper place in chain */
			otmp->nobj = 0;
			if((invent->invlet ^ 040) > (obj->invlet ^ 040)) {
				obj->nobj = invent;
				invent = obj;
			} else
			for(otmp = invent; ; otmp = otmp->nobj) {
			    if(!otmp->nobj ||
				(otmp->nobj->invlet ^ 040) > (obj->invlet ^ 040)){
				obj->nobj = otmp->nobj;
				otmp->nobj = obj;
				break;
			    }
			}
		}
	}

added:
	if (obj->otyp == LUCKSTONE) {
		/* new luckstone must be in inventory by this point
		 * for correct calculation */
		if (stone_luck(TRUE) >= 0) u.moreluck = LUCKADD;
		else u.moreluck = -LUCKADD;
	}
	return(obj);
}

#endif /* OVL1 */
#ifdef OVLB

void
useup(obj)
register struct obj *obj;
{
	if(obj->quan > 1){
#ifndef NO_SIGNAL
		obj->in_use = FALSE;	/* no longer in use */
#endif
		obj->quan--;
		obj->owt = weight(obj);
	} else {
		if(obj->otyp == CORPSE) food_disappears(obj);
		setnotworn(obj);
		freeinv(obj);
		delete_contents(obj);
		obfree(obj, (struct obj *) 0);
	}
}

void
freeinv(obj)
register struct obj *obj;
{
	register struct obj *otmp;

	if(obj == invent)
		invent = invent->nobj;
	else {
		for(otmp = invent; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp->nobj) panic("freeinv");
		otmp->nobj = obj->nobj;
	}
	if (obj->otyp == AMULET_OF_YENDOR && !obj->spe) {
		if (!u.uhave_amulet) impossible ("don't have amulet?");
		u.uhave_amulet = 0;
	}
	if (obj->otyp == LOADSTONE)
		curse(obj);
	if (obj->otyp == LUCKSTONE) {
		if (!carrying(LUCKSTONE)) u.moreluck = 0;
		else if (stone_luck(TRUE) >= 0) u.moreluck = LUCKADD;
		else u.moreluck = -LUCKADD;
		flags.botl = 1;
	}
}

/* destroy object in fobj chain (if unpaid, it remains on the bill) */
void
delobj(obj)
register struct obj *obj;
{
#ifdef WALKIES
	if(obj->otyp == LEASH && obj->leashmon != 0) o_unleash(obj);
#endif
	if(obj->otyp == CORPSE) food_disappears(obj);

	freeobj(obj);
	unpobj(obj);

/*	if a container, get rid of the contents */
	delete_contents(obj);

	obfree(obj, (struct obj *) 0);
}

/* unlink obj from chain starting with fobj */
void
freeobj(obj)
register struct obj *obj;
{
	register struct obj *otmp;
	register int found = 0;

	if(obj == fobj) {
		fobj = fobj->nobj;
		found = 1;
	}
	for(otmp = fobj; otmp; otmp = otmp->nobj)
 	    if (otmp->nobj == obj) {
		otmp->nobj = obj->nobj;
		found = 1;
	    }
  	if (!found) panic("error in freeobj");
	remove_object(obj);
#ifdef POLYSELF
	if (!OBJ_AT(u.ux, u.uy) && !levl[u.ux][u.uy].gmask) {
		u.uundetected = 0;
		if (!Invisible) pru();
	}
#endif
}

/* Note: freegold throws away its argument! */
void
freegold(gold)
register struct gold *gold;
{
	register struct gold *gtmp;

	levl[gold->gx][gold->gy].gmask = 0;

	if(gold == fgold) fgold = gold->ngold;
	else {
		for(gtmp = fgold; gtmp->ngold != gold; gtmp = gtmp->ngold)
			if(!gtmp) panic("error in freegold");
		gtmp->ngold = gold->ngold;
	}
	free((genericptr_t) gold);
#ifdef POLYSELF
	if (!OBJ_AT(u.ux, u.uy) && !levl[u.ux][u.uy].gmask) {
		u.uundetected = 0;
		if (!Invisible) pru();
	}
#endif
}

#endif /* OVLB */
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
	while(objchn) {
		if(objchn->o_id == id) return(objchn);
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

struct gold *
g_at(x,y)
register int x, y;
{
	register struct gold *gold = fgold;
	while(gold) {
		if(gold->gx == x && gold->gy == y) return(gold);
		gold = gold->ngold;
	}
	return((struct gold *)0);
}

/* make dummy object structure containing gold - for temporary use only */
XSTATIC
struct obj *
mkgoldobj(q)
register long q;
{
	register struct obj *otmp;

	otmp = newobj(0);
	/* should set o_id etc. but otmp will be freed soon */
	otmp->olet = GOLD_SYM;
#ifdef POLYSELF
	otmp->ox = 0; /* necessary for eating gold */
#endif
	u.ugold -= q;
	OGOLD(otmp) = q;
	flags.botl = 1;
	return(otmp);
}

#endif /* OVLB */
#ifdef OVL1

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
	register char ilet,ilet1,ilet2;
	char buf[BUFSZ];
	char lets[BUFSZ];
	register int foo = 0, foo2;
	register char *bp = buf;
	xchar allowcnt = 0;	/* 0, 1 or 2 */
	boolean allowgold = FALSE, usegold = FALSE;
		/* usegold is needed so that they are given a different message
		 * if gold is prohibited because it's inappropriate, or because
		 * it's appropriate if only they had any.
		 */
	boolean allowall = FALSE;
	boolean allownone = FALSE;
	xchar foox = 0;
	long cnt;
	boolean prezero = FALSE;

	if(*let == '0') let++, allowcnt = 1;
	if(*let == GOLD_SYM) let++,
		usegold = TRUE, allowgold = (u.ugold ? TRUE : FALSE);
#ifdef POLYSELF
	/* Equivalent of an "ugly check" for gold */
	if (usegold && !strcmp(word, "eat") && !metallivorous(uasmon))
		usegold = allowgold = FALSE;
#endif
	if(*let == '#') let++, allowall = TRUE;
	if(*let == '-') let++, allownone = TRUE;
	if(allownone) *bp++ = '-';
	if(allowgold) *bp++ = GOLD_SYM;
	if(bp > buf && bp[-1] == '-') *bp++ = ' ';

	ilet = 'a';
	for(otmp = invent; otmp; otmp = otmp->nobj){
	    if(!*let || index(let, otmp->olet)) {
		bp[foo++] = flags.invlet_constant ? otmp->invlet : ilet;

		/* ugly check: remove inappropriate things */
		if((!strcmp(word, "take off") &&
		    (!(otmp->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))
		     || (otmp==uarm && uarmc)
#ifdef SHIRT
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
		    (otmp->olet == TOOL_SYM && otmp->otyp != BLINDFOLD))
#ifdef POLYSELF
		|| (!strcmp(word, "eat") && !is_edible(otmp))
#endif
		|| (!strcmp(word, "can") &&
		    (otmp->otyp != CORPSE))
		|| (!strcmp(word, "write with") &&
		    (otmp->olet == TOOL_SYM && otmp->otyp != MAGIC_MARKER))
		|| (!strcmp(word, "rub") &&
		    (otmp->olet == TOOL_SYM &&
		     otmp->otyp != LAMP && otmp->otyp != MAGIC_LAMP))
		|| (!strcmp(word, "wield") &&
		    (otmp->olet == TOOL_SYM &&
		     otmp->otyp != PICK_AXE && otmp->otyp != UNICORN_HORN))
		    )
			foo--;
	    }
	    if(ilet == 'z') ilet = 'A'; else ilet++;
	}
	bp[foo] = 0;
	if(foo == 0 && bp > buf && bp[-1] == ' ') *--bp = 0;
	Strcpy(lets, bp);	/* necessary since we destroy buf */
	if(foo > 5) {			/* compactify string */
		foo = foo2 = 1;
		ilet2 = bp[0];
		ilet1 = bp[1];
		while(ilet = bp[++foo2] = bp[++foo]){
			if(ilet == ilet1+1){
				if(ilet1 == ilet2+1)
					bp[foo2 - 1] = ilet1 = '-';
				else if(ilet2 == '-') {
					bp[--foo2] = ++ilet1;
					continue;
				}
			}
			ilet2 = ilet1;
			ilet1 = ilet;
		}
	}
	if(!foo && !allowall && !allowgold && !allownone) {
		You("don't have anything %sto %s.",
			foox ? "else " : "", word);
		return((struct obj *)0);
	}
	for(;;) {
		if(!buf[0]) {
#ifdef REDO
		    if(!in_doagain)
#endif
			pline("What do you want to %s? [*] ", word);
		} else {
#ifdef REDO
		    if(!in_doagain)
#endif
			pline("What do you want to %s? [%s or ?*] ",
				word, buf);
		}
		cnt = 0;
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
		if(ilet == GOLD_SYM) {
			if(!usegold){
				You("cannot %s gold.", word);
				return(struct obj *)0;
			} else if (!allowgold) {
				You("are not carrying any gold.");
				return(struct obj *)0;
			}
			if(cnt == 0 && prezero) return((struct obj *)0);
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
		if(ilet == '?') {
			doinv(lets);
			if(!(ilet = morc)) continue;
			/* he typed a letter (not a space) to more() */
		} else if(ilet == '*') {
			doinv(NULL);
			if(!(ilet = morc)) continue;
			/* ... */
		}
#ifdef REDO
		if (ilet != '?' && ilet != '*')	savech(ilet);
#endif
		if(flags.invlet_constant) {
			for(otmp = invent; otmp; otmp = otmp->nobj)
				if(otmp->invlet == ilet) break;
		} else {
			if(ilet >= 'A' && ilet <= 'Z') ilet += 'z'-'A'+1;
			ilet -= 'a';
			for(otmp = invent; otmp && ilet;
					ilet--, otmp = otmp->nobj) ;
		}
		if(!otmp) {
			You("don't have that object.");
			continue;
		}
		if(cnt < 0 || otmp->quan < cnt) {
			You("don't have that many!  You have only %u."
			, otmp->quan);
			continue;
		}
		break;
	}
	if(!allowall && let && !index(let,otmp->olet)) {
		pline("That is a silly thing to %s.",word);
		return((struct obj *)0);
	}
	if(allowcnt == 2) {	/* cnt given */
		if(cnt == 0) return (struct obj *)0;
		if(cnt != otmp->quan) {
			register struct obj *obj;
			
#ifdef LINT	/*splitobj for (long )cnt > 30000 && sizeof(int) == 2*/
			obj = (struct obj *)0;
#else
			if (sizeof(int) == 2)
				obj = splitobj(otmp,
					(int )(cnt >30000 ? 30000 : cnt));
			else
				obj = splitobj(otmp, (int) cnt);
#endif
		/* Very ugly kludge necessary to prevent someone from trying
		 * to drop one of several loadstones and having the loadstone
		 * now be separate.  If putting items in containers is ever
		 * changed to allow putting in counts of individual items, a
		 * similar kludge will be needed.
		 */
			if (!strcmp(word, "drop") && obj->otyp == LOADSTONE
					&& obj->cursed)
				otmp->corpsenm = obj->invlet;
			if(otmp == uwep) setuwep(obj);
		}
	}
	return(otmp);
}

#endif /* OVL1 */
#ifdef OVLB

#ifndef OVERLAY
static 
#endif
int
ckunpaid(otmp)
register struct obj *otmp;
{
	return((int)(otmp->unpaid));
}

static boolean
wearing_armor() {
	return(uarm || uarmc || uarmf || uarmg || uarmh || uarms
#ifdef SHIRT
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

static const char removeables[] =
	{ ARMOR_SYM, WEAPON_SYM, RING_SYM, AMULET_SYM, TOOL_SYM, ' ', 0 };

/* interactive version of getobj - used for Drop, Identify and */
/* Takeoff (A). Return the number of times fn was called successfully */
int
ggetobj(word, fn, mx)
register const char *word;
register int FDECL((*fn),(struct obj *)), mx;
{
	char buf[BUFSZ];
	register char *ip;
	register char sym;
	register int oletct = 0, iletct = 0;
	register boolean allflag = FALSE;
	char olets[20], ilets[20];
	int FDECL((*ckfn),(struct obj *)) = (int (*)()) 0;
	xchar allowgold = (u.ugold && !strcmp(word, "drop")) ? 1 : 0; /* BAH */
	register boolean takeoff = !strcmp(word, "take off");

	if(takeoff && !wearing_armor() && !uwep && !uamul &&
			!uleft && !uright && !ublindf) {
		You("are not wearing anything.");
		return(0);
	}
	if(!invent && !allowgold){
		You("have nothing to %s.", word);
		return(0);
	} else {
		register struct obj *otmp = invent;
		register int uflg = 0;

		if(allowgold) ilets[iletct++] = GOLD_SYM;
		ilets[iletct] = 0;
		while(otmp) {
			if(!index(ilets, otmp->olet)){
			    if(!takeoff || is_worn(otmp))
				ilets[iletct++] = otmp->olet;
			    ilets[iletct] = 0;
			}
			if(otmp->unpaid) uflg = 1;
			otmp = otmp->nobj;
		}
		ilets[iletct++] = ' ';
		if(uflg && !takeoff) ilets[iletct++] = 'u';
		if(invent && !takeoff) ilets[iletct++] = 'a';
		ilets[iletct] = 0;
	}
	pline("What kinds of thing do you want to %s? [%s] ",
		word, ilets);
	getlin(buf);
	if(buf[0] == '\033') {
		clrlin();
		return(0);
	}
	ip = buf;
	olets[0] = 0;
	while(sym = *ip++){
		if(sym == ' ') continue;
		if(takeoff && !(uwep && sym == uwep->olet)) {
		    if(!index(removeables,sym)) {
			pline("Not applicable.");
			return(0);
		    } else if(sym == ARMOR_SYM && !wearing_armor()) {
			You("are not wearing any armor.");
			return(0);
		    } else if(sym == WEAPON_SYM && !uwep) {
			You("are not wielding anything.");
			return(0);
		    } else if(sym == RING_SYM && !uright && !uleft) {
			You("are not wearing rings.");
			return(0);
		    } else if(sym == AMULET_SYM && !uamul) {
			You("are not wearing an amulet.");
			return(0);
		    } else if(sym == TOOL_SYM && !ublindf) {
			You("are not wearing a blindfold.");
			return(0);
		    }
		}
		if(sym == GOLD_SYM) {
			if(allowgold == 1)
				(*fn)(mkgoldobj(u.ugold));
			else if(!u.ugold)
				You("have no gold.");
			allowgold = 2;
		} else if(sym == 'a' || sym == 'A')
			allflag = TRUE;
		else if(sym == 'u' || sym == 'U')
			ckfn = ckunpaid;
		else if(index(inv_order, sym)) {
			if(!index(olets, sym)) {
				olets[oletct++] = sym;
				olets[oletct] = 0;
			}
		} else You("don't have any %c's.", sym);
	}
	if(allowgold == 2 && !oletct)
		return(1);	/* he dropped gold (or at least tried to) */
	else
		return(askchain(invent, TRUE, olets, allflag, fn, ckfn, mx, word));
}

/*
 * Walk through the chain starting at objchn and ask for all objects
 * with olet in olets (if nonNULL) and satisfying ckfn (if nonNULL)
 * whether the action in question (i.e., fn) has to be performed.
 * If allflag then no questions are asked. Max gives the max nr of
 * objects to be treated. Return the number of objects treated.
 * If ininv is TRUE, the objects are in the player's inventory.
 */
int
askchain(objchn, ininv, olets, allflag, fn, ckfn, mx, word)
register struct obj *objchn;
register int ininv, allflag, mx;
register const char *olets, *word;
register int FDECL((*fn),(struct obj *)), FDECL((*ckfn),(struct obj *));
{
	register struct obj *otmp, *otmp2;
	register char sym, ilet;
	register int cnt = 0, dud = 0;
	register boolean takeoff, nodot;

	takeoff = !strcmp(word, "take off");
	nodot = (!strcmp(word, "nodot") || !strcmp(word, "drop") ||
	   		!strcmp(word, "identify") || takeoff);
	/* changes so the askchain is interrogated in the order specified.
	 * For example, if a person specifies =/ then first all rings will be
	 * asked about followed by all wands -dgk
	 */
nextclass:
	ilet = 'a'-1;
	for(otmp = (ininv ? invent : objchn); otmp; otmp = otmp2){
		if(ilet == 'z') ilet = 'A'; else ilet++;
		otmp2 = otmp->nobj;
		if (olets && *olets && otmp->olet != *olets) continue;
		if(takeoff && !is_worn(otmp)) continue;
		if(ckfn && !(*ckfn)(otmp)) continue;
		if(!allflag) {
			if(ininv)
			    pline("%s", xprname(otmp, ilet,
							nodot ? FALSE : TRUE));
			else
			    pline(doname(otmp));
			addtopl("? ");
			sym = nyaq();
		}
		else	sym = 'y';

		switch(sym){
		case 'a':
			allflag = 1;
		case 'y':
			cnt += (*fn)(otmp);
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

static char
obj_to_let(obj)	/* should of course only be called for things in invent */
register struct obj *obj;
{
	register struct obj *otmp;
	register char ilet;

	if(flags.invlet_constant)
		return(obj->invlet);
	ilet = 'a';
	for(otmp = invent; otmp && otmp != obj; otmp = otmp->nobj)
		if(++ilet > 'z') ilet = 'A';
	return(otmp ? ilet : NOINVSYM);
}

void
prinv(obj)
register struct obj *obj;
{
	pline(xprname(obj, obj_to_let(obj), TRUE));
}

#endif /* OVLB */
#ifdef OVL1

XSTATIC char *
xprname(obj,let,dot)
register struct obj *obj;
register char let;
register boolean dot;
{
#ifdef LINT	/* handle static char li[BUFSZ]; */
	char li[BUFSZ];
#else
	static char li[BUFSZ];
#endif

	Sprintf(li, "%c - %s",
		(flags.invlet_constant ? obj->invlet : let),
		doname(obj));
	if(dot) Sprintf(eos(li),".");
	return(li);
}

#endif /* OVL1 */
#ifdef OVLB

int
ddoinv()
{
	doinv(NULL);
	return 0;
}

#endif /* OVLB */
#ifdef OVL1

/* called with 0 or "": all objects in inventory */
/* otherwise: all objects with (serial) letter in lets */
void
doinv(lets)
register const char *lets;
{
	register struct obj *otmp;
	register char ilet;
	int ct = 0;
	char any[BUFSZ];
	char *invlet = inv_order;
	int classcount = 0;

	morc = 0;		/* just to be sure */

	if(!invent){
		pline("Not carrying anything.");
		return;
	}
	if (lets != NULL) {
		for(ct=0; lets[ct]; ct++) {
			if (ct >= 52) {
				impossible("bad lets contents");
				break;
			}
		}
		if (ct==1) {
			ilet = 'a';
			for(otmp = invent; otmp; otmp = otmp->nobj) {
				if(flags.invlet_constant) ilet = otmp->invlet;

				if (ilet == lets[0])
					pline(xprname(otmp, lets[0], TRUE));

				if(!flags.invlet_constant)
					if(++ilet > 'z') ilet = 'A';
			}
			return;
		}
	}
	ct = 0;

	cornline(0, NULL);
nextclass:
	classcount = 0;
	ilet = 'a';
	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if(flags.invlet_constant) ilet = otmp->invlet;
		if(!lets || !*lets || index(lets, ilet)) {
			if (!flags.sortpack || otmp->olet == *invlet) {
				if (flags.sortpack && !classcount) {
					cornline(1, let_to_name(*invlet));
					classcount++;
				}
				cornline(1, xprname(otmp, ilet, TRUE));
				any[ct++] = ilet;
			}
		}
		if(!flags.invlet_constant) if(++ilet > 'z') ilet = 'A';
	}
	if (flags.sortpack && *++invlet) goto nextclass;
	any[ct] = 0;
	cornline(2, any);
}

#endif /* OVL1 */
#ifdef OVLB

int
dotypeinv()				/* free after Robert Viduya */
/* Changed to one type only, so he doesn't have to type cr */
{
    	char c, ilet;
    	char stuff[BUFSZ];
    	register int stct;
    	register struct obj *otmp;
    	boolean billx = in_shop(u.ux, u.uy) && doinvbill(0);
    	boolean unpd = FALSE;

	if (!invent && !u.ugold && !billx) {
	    You("aren't carrying anything.");
	    return 0;
	}

	stct = 0;
	if(u.ugold) stuff[stct++] = GOLD_SYM;
	stuff[stct] = 0;
	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if (!index (stuff, otmp->olet)) {
		stuff[stct++] = otmp->olet;
		stuff[stct] = 0;
	    }
	    if(otmp->unpaid)
		unpd = TRUE;
	}
	if(unpd) stuff[stct++] = 'u';
	if(billx) stuff[stct++] = 'x';
	stuff[stct] = 0;

	if(stct > 1) {
#ifdef REDO
	  if (!in_doagain)
#endif
	    pline ("What type of object do you want an inventory of? [%s] ",
			stuff);
	    c = readchar();
#ifdef REDO
	    savech(c);
#endif
	    if(index(quitchars,c)) {
	    	    clrlin();
	    	    return 0;
	    }
	} else
	    c = stuff[0];

	if(c == GOLD_SYM)
	    return(doprgold());

	if(c == 'x' || c == 'X') {
	    if(billx)
		(void) doinvbill(1);
	    else
		pline("No used-up objects on the shopping bill.");
	    return(0);
	}

	if((c == 'u' || c == 'U') && !unpd) {
		You("are not carrying any unpaid objects.");
		return(0);
	}

	stct = 0;
	ilet = 'a';
	for (otmp = invent; otmp; otmp = otmp -> nobj) {
	    if(flags.invlet_constant) ilet = otmp->invlet;
	    if (c == otmp -> olet || (c == 'u' && otmp -> unpaid))
		stuff[stct++] = ilet;
	    if(!flags.invlet_constant) if(++ilet > 'z') ilet = 'A';
	}
	stuff[stct] = '\0';
	if(stct == 0)
		You("have no such objects.");
	else
		doinv (stuff);

	return(0);
}

/* look at what is here */
int
dolook() {
    	register struct obj *otmp, *otmp0;
    	register struct gold *gold;
    	const char *verb = Blind ? "feel" : "see";
    	int ct = 0;
    	int fd = 0;

    	read_engr_at(u.ux, u.uy); /* Eric Backus */
    	if(!u.uswallow) {
		otmp0 = level.objects[u.ux][u.uy];
		gold = g_at(u.ux, u.uy);
    	}  else  {
		You("%s no objects here.", verb);
		return(!!Blind);
    	}

	if(IS_DOOR(levl[u.ux][u.uy].typ))  {
		fd++;
		switch(levl[u.ux][u.uy].doormask) {
		    case D_NODOOR:
			pline("There is a doorway here."); break;
		    case D_ISOPEN:
			pline("There is an open door here."); break;
		    case D_BROKEN:
			pline("There is a broken door here."); break;
		    default:
			pline("There is a closed door here.");
		}
	}
    	/* added by GAN 10/30/86 */
#ifdef FOUNTAINS
    	if(IS_FOUNTAIN(levl[u.ux][u.uy].typ))  {
		fd++;
		pline("There is a fountain here.");
    	}
#endif
#ifdef THRONES
    	if(IS_THRONE(levl[u.ux][u.uy].typ))  {
		fd++;
		pline("There is an opulent throne here.");
    	}
#endif
#ifdef SINKS
    	if(IS_SINK(levl[u.ux][u.uy].typ))  {
		fd++;
		pline("There is a kitchen sink here.");
    	}
#endif
#ifdef ALTARS
    	if(IS_ALTAR(levl[u.ux][u.uy].typ))  {
		const char *al;

		fd++;
		switch (levl[u.ux][u.uy].altarmask & ~A_SHRINE) {
			case 0: al = "chaotic"; break;
			case 1: al = "neutral"; break;
			default: al = "lawful"; break;
		}
		pline("There is an altar to %s here (%s).", a_gname(), al);
    	}
#endif

    	if(u.ux == xupstair && u.uy == yupstair)  {
		fd++;
		pline("There is a stairway up here.");
    	}
    	if(u.ux == xdnstair && u.uy == ydnstair)  {
		fd++;
		pline("There is a stairway down here.");
    	}
#ifdef STRONGHOLD
    	if(u.ux == xupladder && u.uy == yupladder)	{
		fd++;
		pline("There is a ladder up here.");
    	}
    	if(u.ux == xdnladder && u.uy == ydnladder)	{
		fd++;
		pline("There is a ladder down here.");
    	}
    	if (levl[u.ux][u.uy].typ == DRAWBRIDGE_DOWN) {
		fd++;
		pline("There is a lowered drawbridge here.");
    	}
#endif /* STRONGHOLD /**/

    	if(Blind)  {
		You("try to feel what is lying here on the floor.");
	 	if(Levitation)  {
			pline("But you can't reach it!");
			return(0);
	 	}
    	}

    	if(!otmp0 && !gold) {
		if(Blind || !fd)
			You("%s no objects here.", verb);
		return(!!Blind);
    	}

    	cornline(0, "Things that are here:");
    	for(otmp = otmp0; otmp; otmp = otmp->nexthere) {
		ct++;
		cornline(1, doname(otmp));

		if(Blind  && !uarmg &&
#ifdef POLYSELF
		    !resists_ston(uasmon) &&
#endif
		    (otmp->otyp == CORPSE && otmp->corpsenm == PM_COCKATRICE)) {
			pline("Touching the dead cockatrice is a fatal mistake...");
			You("turn to stone...");
			killer_format = KILLED_BY_AN;
			killer = "cockatrice corpse";
			done(STONING);
		}
	}

    	if(gold) {
		char gbuf[30];

		Sprintf(gbuf, "%ld gold piece%s",
			gold->amount, plur(gold->amount));
		if(!ct++)
	    		You("%s here %s.", verb, gbuf);
		else
	    		cornline(1, gbuf);
    	}

    	if(ct == 1 && !gold) {
		You("%s here %s.", verb, doname(otmp0));
		cornline(3, NULL);
    	}
    	if(ct > 1)
		cornline(2, NULL);
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
	   obj->otrapped != otmp->otrapped)
	    return(FALSE);

	if((obj->olet==WEAPON_SYM || obj->olet==ARMOR_SYM) &&
		obj->rustfree != otmp->rustfree) return FALSE;

	if(obj->olet == FOOD_SYM && (obj->oeaten != otmp->oeaten ||
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

#ifdef NAMED_ITEMS
	if(is_artifact(obj) != is_artifact(otmp)) return FALSE;
#endif

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
	else prinv(uwep);
	return 0;
}

int
doprarm(){
	if(!wearing_armor())
		You("are not wearing any armor.");
	else {
#ifdef SHIRT
		char lets[8];
#else
		char lets[7];
#endif
		register int ct = 0;

#ifdef SHIRT
		if(uarmu) lets[ct++] = obj_to_let(uarmu);
#endif
		if(uarm) lets[ct++] = obj_to_let(uarm);
		if(uarmc) lets[ct++] = obj_to_let(uarmc);
		if(uarmh) lets[ct++] = obj_to_let(uarmh);
		if(uarms) lets[ct++] = obj_to_let(uarms);
		if(uarmg) lets[ct++] = obj_to_let(uarmg);
		if(uarmf) lets[ct++] = obj_to_let(uarmf);
		lets[ct] = 0;
		doinv(lets);
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
		doinv(lets);
	}
	return 0;
}

int
dopramulet(){
	if (!uamul)
		You("are not wearing an amulet.");
	else
		prinv(uamul);
	return 0;
}

int
doprtool(){
	register struct obj *otmp;
	register int ct=0;
	char lets[3]; /* Maximum: pick-axe, blindfold */

	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if (((otmp->owornmask & W_TOOL) && otmp->olet==TOOL_SYM)
		   || (otmp==uwep &&
		   (otmp->otyp==PICK_AXE || otmp->otyp==TIN_OPENER)))
			lets[ct++] = obj_to_let(otmp);
	}
	lets[ct] = 0;
	if (!ct) You("are not using any tools.");
	else doinv(lets);
	return 0;
}

int
digit(c)
char c;
{
	return(c >= '0' && c <= '9');
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
	if(obj->quan > 1)
		otmp = splitobj(obj, (int)obj->quan - 1);
	else
		otmp = obj;
	addtobill(otmp, FALSE);
	delobj(otmp);
}

/*
 * Convert from a symbol to a string for printing object classes
 *
 * Names from objects.h
 * char obj_symbols[] = {
 *	ILLOBJ_SYM, AMULET_SYM, FOOD_SYM, WEAPON_SYM, TOOL_SYM,
 *	BALL_SYM, CHAIN_SYM, ROCK_SYM, ARMOR_SYM, POTION_SYM, SCROLL_SYM,
 *	WAND_SYM, [SPBOOK_SYM], RING_SYM, GEM_SYM, 0 };
 */

static const char *names[] = {
	"Illegal objects", "Amulets", "Comestibles", "Weapons",
	"Tools", "Iron balls", "Chains", "Boulders/Statues", "Armor",
	"Potions", "Scrolls", "Wands",
#ifdef SPELLS
	"Spellbooks",
#endif
	"Rings", "Gems"};

char *
let_to_name(let)
char let;
{
	const char *pos = index(obj_symbols, let);
	/* arbitrary buffer size by Tom May (tom@uw-warp) */
	static char *buf = NULL;

	if (buf == NULL)
	    buf = (char *) alloc ((unsigned)(strlen(HI)+17+strlen(HE)));
/* 
   THE ALLOC() *MUST* BE BIG ENOUGH TO ACCOMODATE THE LONGEST NAME PLUS A
   NULL BYTE: 
			Boulders/Statues   +  '\0'
			1234567890123456 = 16 + 1 = 17
*/
	if (pos == NULL) pos = obj_symbols;
	if (HI && HE)
	    Sprintf(buf, "%s%s%s", HI, names[pos - obj_symbols], HE);
	else
	    Sprintf(buf, "%s", names[pos - obj_symbols]);
	return (buf);
}

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
