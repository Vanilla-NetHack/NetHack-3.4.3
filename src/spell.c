/*	SCCS Id: @(#)spell.c	3.0	88/09/18
 *
 *	Copyright (c) M. Stephenson 1988
 */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef SPELLS
static schar delay;		/* moves left for this spell */
static struct obj *book;	/* last/current book being xscribed */

#ifdef HARD
#define spelluses(spell)	spl_book[spell-1].sp_uses
#define decrnuses(spell)	spl_book[spell-1].sp_uses--
#endif /* HARD */
#define spellev(spell)		spl_book[spell-1].sp_lev
#define spellname(spell)	objects[spl_book[spell-1].sp_id].oc_name
#define spellid(spell)		spl_book[spell-1].sp_id

static void
cursed_book(lev)
	register int	lev;
{
	switch(rn2(lev)) {
	case 0:
		You("feel a wrenching sensation.");
		tele();		/* teleport him */
		break;
	case 1:
		You("feel threatened.");
		aggravate();
		break;
	case 2:
		make_blinded(Blinded + rn1(100,250),TRUE);
		break;
	case 3:
		take_gold();
		break;
	case 4:
		pline("These runes were just too much to comprehend.");
		make_confused(HConfusion + rn1(7,16),FALSE);
		break;
	case 5:
		pline("The book was coated with contact poison!");
		if (uarmg) {
		    if (uarmg->rustfree)
			Your("gloves seem unaffected.");
		    else if (uarmg->spe > -6) {
			Your("gloves corrode!");
			uarmg->spe--;
		    } else
			Your("gloves look quite corroded.");
		    break;
		}
		if(Poison_resistance) {
		    losestr(rn1(1,2));
		    losehp(rnd(6), "contact poison");
		} else {
		    losestr(rn1(4,3));
		    losehp(rnd(10), "contact poison");
		}
		break;
	case 6:
		if(Antimagic) {
		    shieldeff(u.ux, u.uy);
		    pline("The book explodes, but you are unharmed!");
		} else {
		    pline("As you read the book, it explodes in your %s!",
			body_part(FACE));
		    losehp (2*rnd(10)+5, "exploding rune");
		}
		break;
	default:
		rndcurse();
		break;
	}
	return;
}

#ifndef OVERLAY
static 
#endif
int
learn()
{
	register int	i;
	register unsigned booktype;

	if (delay) {	/* not if (delay++), so at end delay == 0 */
		delay++;
		return(1); /* still busy */
	}

	booktype = book->otyp;
	for (i = 0; i < MAXSPELL; i++)  {
		if (spl_book[i].sp_id == booktype)  {
#ifdef HARD
			Your("knowledge of that spell is keener.");
			spl_book[i].sp_uses += rn1(3,8-spl_book[i].sp_lev);
#else
			pline("Oh, you already know that one!");
#endif
			break;
		} else if (spl_book[i].sp_id == NO_SPELL)  {
			spl_book[i].sp_id = booktype;
			spl_book[i].sp_lev = objects[booktype].spl_lev;
			spl_book[i].sp_flags = objects[booktype].bits;
#ifdef HARD
			/* spells have 2 .. 10-level uses. */
			/* ie 2 or 3 uses w/ most potent */
			spl_book[i].sp_uses = rn1(3,8-spl_book[i].sp_lev);
#endif
			You("add the spell to your repertoire.");
			makeknown(booktype);
			break;
		}
	}
	if (i == MAXSPELL) impossible("Too many spells memorized!");

	if (book->cursed) {	/* maybe a demon cursed it */
		cursed_book(objects[booktype].spl_lev);
	}

	useup(book);
	book = 0;
	return(0);
}

int
study_book(spellbook)
register struct obj *spellbook;
{
	register int	 booktype = spellbook->otyp;

	if (delay && spellbook == book)
		You("continue your efforts to memorize the spell.");
	else {
		switch(booktype)  {

/* level 1 spells */
	case SPE_HEALING:
	case SPE_DETECT_MONSTERS:
	case SPE_FORCE_BOLT:
	case SPE_LIGHT:
	case SPE_SLEEP:
	case SPE_KNOCK:
/* level 2 spells */
	case SPE_MAGIC_MISSILE:
	case SPE_CONFUSE_MONSTER:
	case SPE_SLOW_MONSTER:
	case SPE_CURE_BLINDNESS:
	case SPE_CREATE_MONSTER:
	case SPE_DETECT_FOOD:
	case SPE_WIZARD_LOCK:
		delay = -objects[booktype].oc_delay;
		break;
/* level 3 spells */
	case SPE_HASTE_SELF:
	case SPE_CAUSE_FEAR:
	case SPE_CURE_SICKNESS:
	case SPE_DETECT_UNSEEN:
	case SPE_EXTRA_HEALING:
	case SPE_CHARM_MONSTER:
	case SPE_CLAIRVOYANCE:
/* level 4 spells */
	case SPE_LEVITATION:
	case SPE_RESTORE_ABILITY:
	case SPE_INVISIBILITY:
	case SPE_FIREBALL:
	case SPE_DETECT_TREASURE:
		delay = -(objects[booktype].spl_lev - 1) * objects[booktype].oc_delay;
		break;
/* level 5 spells */
	case SPE_REMOVE_CURSE:
	case SPE_MAGIC_MAPPING:
	case SPE_CONE_OF_COLD:
	case SPE_IDENTIFY:
	case SPE_DIG:
/* level 6 spells */
	case SPE_TURN_UNDEAD:
	case SPE_POLYMORPH:
	case SPE_CREATE_FAMILIAR:
	case SPE_TELEPORT_AWAY:
		delay = -objects[booktype].spl_lev * objects[booktype].oc_delay;
		break;
/* level 7 spells */
	case SPE_CANCELLATION:
	case SPE_FINGER_OF_DEATH:
	case SPE_GENOCIDE:
		delay = -8 * objects[booktype].oc_delay;
		break;
/* impossible */
	default:
		impossible("Unknown spellbook, %d;", booktype);
		return(0);
	}

		if(!spellbook->blessed &&
			(spellbook->cursed ||
			    rn2(20) > (ACURR(A_INT) + 4 + (int)(u.ulevel/2)
					- 2*objects[booktype].spl_lev))) {
			cursed_book(objects[booktype].spl_lev);
			nomul(delay);			/* study time */
			delay = 0;
			useup(spellbook);
			return(1);
		}

		You("begin to memorize the runes.");
	}

	book = spellbook;
	set_occupation(learn, "studying", 0);
	return(1);
}

static int
getspell()  {

	register int	maxs, ilet, i;
	char	 lets[BUFSZ], buf[BUFSZ];

	if (spl_book[0].sp_id == NO_SPELL)  {

		You("don't know any spells right now.");
		return(0);
	} else  {

	    for(maxs = 1; (maxs < MAXSPELL) && (spl_book[maxs].sp_id != NO_SPELL); maxs++);
	    if (maxs >= MAXSPELL)  {

		impossible("Too many spells memorized.");
		return(0);
	    }

	    for(i = 0; (i < maxs) && (i < 26); buf[++i] = 0)  buf[i] = 'a' + i;
	    for(i = 26; (i < maxs) && (i < 52); buf[++i] = 0) buf[i] = 'A' + i - 26;

	    if (maxs == 1)  Strcpy(lets, "a");
	    else if (maxs < 27)  Sprintf(lets, "a-%c", 'a' + maxs - 1);
	    else if (maxs == 27)  Sprintf(lets, "a-z A");
	    else Sprintf(lets, "a-z A-%c", 'A' + maxs - 27);
	    for(;;)  {

		pline("Cast which spell? [%s ?] ", lets);
		if ((ilet = readchar()) == '?')  {
			(void) dovspell();
			continue;
		} else if ((ilet == '\033')||(ilet == '\n')||(ilet == ' '))
			return(0);
		else for(i = 0; buf[i] != 0; i++)  if(ilet == buf[i])  return(++i);
		You("don't know that spell.");
	    }
	}
}

int
docast()
{
	register int	 spell;

	spell = getspell();
	if (!spell) return(0);

	return(spelleffects(spell,FALSE));
}

int
spelleffects(spell,atme)
register int spell;
boolean atme;
{
	register int energy, damage;
#ifdef HARD
	boolean confused = (Confusion != 0);
#endif
	struct obj *pseudo;

#ifdef HARD
	/* note that trying to cast it decrements the # of uses,    */
	/* even if the mage does not have enough food/energy to use */
	/* the spell */
	switch (spelluses(spell)) {
		case 0:
		    pline ("That spell is too hard to recall at the moment.");
		    return(0);
		case 1:
		    pline ("You can barely remember the runes of this spell.");
		    break;
		case 2:
		    pline ("This spell is starting to be over-used.");
		    break;
		default:
		    break;
	}
	decrnuses(spell);
#endif
	energy = spellev(spell);
	if (u.uhave_amulet) {
		You("feel the amulet draining your energy away.");
		energy *= rnd(6);
	}
	if(energy > u.uen)  {
		You("don't have enough energy to cast that spell.");
		return(0);
	} else	if ((u.uhunger <= 100 && spell != SPE_DETECT_FOOD) ||
						(ACURR(A_STR) < 6))  {
		You("lack the strength for that spell.");
		return(0);
	} else	{
		if (spell != SPE_DETECT_FOOD)
			morehungry(energy * 10);
		u.uen -= energy;
	}
	flags.botl = 1;

#ifdef HARD
	if (confused ||
	    ((int)(ACURR(A_INT) + u.uluck) - 3 * spellev(spell)) < 0) {

		if (Hallucination)
			pline("Far out... a light show!");
		else	pline("The air around you crackles as you goof up.");
		return(0);
	}
#endif

/*	pseudo is a temporary "false" object containing the spell stats. */
	pseudo = mksobj(spellid(spell),FALSE);
	pseudo->blessed = pseudo->cursed = 0;
	pseudo->quan = 20;			/* do not let useup get it */
	switch(pseudo->otyp)  {

/* These spells are all duplicates of wand effects */
	case SPE_FORCE_BOLT:
	case SPE_SLEEP:
	case SPE_MAGIC_MISSILE:
	case SPE_KNOCK:
	case SPE_SLOW_MONSTER:
	case SPE_WIZARD_LOCK:
	case SPE_FIREBALL:
	case SPE_CONE_OF_COLD:
	case SPE_DIG:
	case SPE_TURN_UNDEAD:
	case SPE_POLYMORPH:
	case SPE_TELEPORT_AWAY:
	case SPE_CANCELLATION:
	case SPE_FINGER_OF_DEATH:
	case SPE_LIGHT:
	case SPE_DETECT_UNSEEN:
		if (!(objects[pseudo->otyp].bits & NODIR)) {
			if (atme) u.dx = u.dy = u.dz = 0;
			else (void) getdir(1);
			if(!u.dx && !u.dy && !u.dz) {
			    if((damage = zapyourself(pseudo)))
				losehp(damage, "self-inflicted injury");
			} else	weffects(pseudo);
		} else weffects(pseudo);
		break;
/* These are all duplicates of scroll effects */
	case SPE_CONFUSE_MONSTER:
	case SPE_DETECT_FOOD:
	case SPE_CAUSE_FEAR:
	case SPE_CHARM_MONSTER:
	case SPE_REMOVE_CURSE:
	case SPE_MAGIC_MAPPING:
	case SPE_CREATE_MONSTER:
	case SPE_IDENTIFY:
	case SPE_GENOCIDE:
		(void) seffects(pseudo);
		break;
	case SPE_HASTE_SELF:
	case SPE_DETECT_TREASURE:
	case SPE_DETECT_MONSTERS:
	case SPE_LEVITATION:
	case SPE_RESTORE_ABILITY:
	case SPE_INVISIBILITY:
		(void) peffects(pseudo);
		break;
	case SPE_HEALING:
		You("feel a bit better.");
		healup(rnd(8), 0, 0, 0);
		break;
	case SPE_CURE_BLINDNESS:
		healup(0, 0, 0, 1);
		break;
	case SPE_CURE_SICKNESS:
		if (Sick) You("are no longer ill.");
		healup(0, 0, 1, 0);
		break;
	case SPE_EXTRA_HEALING:
		You("feel a fair bit better.");
		healup(d(2,8), 1, 0, 0);
		break;
	case SPE_CREATE_FAMILIAR:
		make_familiar((struct obj *)0);
		break;
	case SPE_CLAIRVOYANCE:
		do_vicinity_map();
		break;
	default:
		impossible("Unknown spell %d attempted.", spell);
		obfree(pseudo, (struct obj *)0);
		return(0);
	}
	obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
	return(1);
}

void
losespells() {
	register boolean confused = (Confusion != 0);
	register int	 n, nzap, i;

	book = 0;
	for(n = 0;(spl_book[n].sp_id != NO_SPELL) && (n < MAXSPELL); n++);
	if (!n) return;
	if (n < MAXSPELL) {
		nzap = rnd(n);
		if (nzap < n) nzap += confused;
		for (i = 0; i < nzap; i++) spl_book[n-i-1].sp_id = NO_SPELL;
	} else impossible("Too many spells memorized!");
	return;
}

static char
spellet(spl)
{
	return (spl < 27) ? ('a' + spl - 1) : ('A' + spl - 27);
}

int
dovspell() {

	register int maxs, i;
	char     buf[BUFSZ], any[BUFSZ];

	if (spl_book[0].sp_id == NO_SPELL)  {

		You("don't know any spells right now.");
		return 0;
	}

	for(maxs = 1; (maxs < MAXSPELL) && (spl_book[maxs].sp_id != NO_SPELL); maxs++);
	if (maxs >= MAXSPELL)  {

		impossible("Too many spells memorized.");
		return 0;
	}
	morc = 0;		/* just to be sure */
	cornline(0, "Currently known spells:");

	for(i = 1; i <= maxs; i++) {

#ifdef HARD
		Sprintf(buf, "%c %c %s (%d)",
			spellet(i), (spelluses(i)) ? '-' : '*',
			spellname(i), spellev(i));
#else
		Sprintf(buf, "%c %s (%d)",
			spellet(i),
			spellname(i), spellev(i));
#endif
		cornline(1, buf);
		any[i-1] = spellet(i);
  	}
	any[i-1] = 0;
	cornline(2, any);

	return 0;
}


#endif /* SPELLS /**/
