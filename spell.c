/*	SCCS Id: @(#)spell.c	2.1	87/10/07
 */

#include "hack.h"
#ifdef SPELLS
extern char *nomovemsg;

doxcribe() {
	register struct obj *book;
	struct	 obj	*getobj();
	register boolean confused = (Confusion != 0);
	register boolean oops;
	register schar	 delay;
	register int   booktype;
	register int	 i;

	book = getobj("+", "transcribe");
	if(!book) return(0);

	if(Blind) {
	    pline("Being blind, you cannot read the mystic runes.");
	    useup(book);		/* well, if you are stupid... */
	    return(0);
	}

	if(confused) {
	    pline("Being confused, you cannot grasp the meaning of this tome.");
	    useup(book);		/* and more stupidity... */
	    return(0);
	}
	booktype = book->otyp;
	oops = !rn2(u.ulevel - objects[booktype].spl_lev + 7);
	switch(booktype)  {

/* level 1 spells */
	case SPE_HEALING:
	case SPE_DETECT_MONSTERS:
	case SPE_FORCE_BOLT:
	case SPE_LIGHT:
	case SPE_SLEEP:
/* level 2 spells */
	case SPE_MAGIC_MISSILE:
	case SPE_CONFUSE_MONSTER:
	case SPE_SLOW_MONSTER:
	case SPE_CURE_BLINDNESS:
	case SPE_CREATE_MONSTER:
	case SPE_DETECT_FOOD:
		delay = -objects[booktype].oc_delay;
		break;
/* level 3 spells */
	case SPE_HASTE_SELF:
	case SPE_CAUSE_FEAR:
	case SPE_CURE_SICKNESS:
	case SPE_DETECT_UNSEEN:
	case SPE_EXTRA_HEALING:
	case SPE_CHARM_MONSTER:
/* level 4 spells */
	case SPE_LEVITATION:
	case SPE_RESTORE_STRENGTH:
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
		impossible("Unknown spell-book, %d;", booktype);
		return(0);
	}

	pline("You begin to transcribe the spell.");
	if(oops || book->cursed)  {
		cursed_book(objects[booktype].spl_lev);
		nomul(delay);			/* study time */
	} else  {
		nomul(delay);			/* study time */
		for(i = 0; i < MAXSPELL; i++)  {
		    if(spl_book[i].sp_id == booktype)  {
#ifdef HARD
			nomovemsg = "You make that spell more legible.";
			spl_book[i].sp_uses += rn1(3,8-spl_book[i].sp_lev);
#else			
			nomovemsg = "Oh, you already know that one!";
#endif
			useup(book);
			return(1);
		    } else if (spl_book[i].sp_id == NO_SPELL)  {
			spl_book[i].sp_id = booktype;
			spl_book[i].sp_lev = objects[booktype].spl_lev;
			spl_book[i].sp_flags = objects[booktype].bits;
#ifdef HARD
			/* spells have 2 .. 10-level uses. */
			/* ie 2 or 3 uses w/ most potent */
			spl_book[i].sp_uses = rn1(3,8-spl_book[i].sp_lev);
#endif
			nomovemsg = "You add the spell to your books.";
			objects[booktype].oc_name_known = 1;
			useup(book);
			return(1);
		    }
		}
		impossible("Too many spells in spellbook!");
	}
	useup(book);
	return(1);
}

cursed_book(level)
	register int	level;
{
	switch(rn2(level)) {
	case 0:
		pline("You feel a wrenching sensation.");
		tele();		/* teleport him */
		break;
	case 1:
		pline("You feel threatened.");
		aggravate();
		break;
	case 2:
		if(!Blind)	pline("A cloud of darkness falls upon you.");
		Blinded += rn1(100,250);
		seeoff(0);
		break;
	case 3:
		if (u.ugold <= 0)  {
			pline("You feel a strange sensation.");
		} else {
			pline("You notice you have no gold!");
			u.ugold = 0;
			flags.botl = 1;
		}
		break;
	case 4:
		pline("These runes were just too much to comprehend.");
		HConfusion += rn1(7,16);
		break;
	case 5:
		pline("The book was coated with contact poison!");
		if(Poison_resistance) {
		    losestr(rn1(1,2));
		    losehp(rnd(6), "contact poison");
		} else {
		    losestr(rn1(4,3));
		    losehp(rnd(10), "contact poison");
		}
		break;
	case 6:
		pline("As you read the book, it explodes in your face!");
		losehp (2*rnd(10)+5, "exploding rune");
		break;
	default:
		rndcurse();
		break;
	}
	return(0);
}

docast()
{
	register int	 spell, energy, damage;
	register boolean confused = (Confusion != 0);
	register struct  obj	*pseudo;
	struct	 obj	 *mksobj();

	spell = getspell();
	if (!spell) return(0);
	else  {
#ifdef HARD
		/* note that turning to the page decrements the # of uses,  */
		/* even if the mage does not have enough food/energy to use */
		/* the spell */
		switch (spelluses(spell)) {
		case 0:
		    pline ("That page is too faint to read at the moment.");
		    return(0);
		case 1:
		    pline ("You can barely make out the runes on this page.");
		    break;
		case 2:
		    pline ("This spell is starting to look well used.");
		    break;
		default:
		    break;
		}
		decrnuses(spell);
#endif		
		energy = spellev(spell);
#ifdef BVH
		if (has_amulet()) {

		    pline("You feel the amulet draining your energy away.");
		    energy *= rnd(6);
		}
#endif
		if(energy > u.uen)  {
			pline("You are too weak to cast that spell.");
			return(0);
		} else  if ((u.uhunger <= 100) || (u.ustr < 6))  {
			pline("You miss the strength for that spell.");
			return(0);
		} else	{
			morehungry(energy * 10);
			u.uen -= energy;
		}
		flags.botl = 1;
	}
#ifdef HARD
	if (confused ||
	    (rn2(10) + (int)(u.ulevel + u.uluck) - 3*spellev(spell)) < 0) {

		if (Hallucination)
			pline("Far out... a light show!");
		else	pline("The air around you crackles as you goof up.");
		return(0);
	}
#endif

/*	pseudo is a temporary "false" object containing the spell stats. */
	pseudo = mksobj(spellid(spell));
	pseudo->quan = 20;			/* do not let useup get it */
	switch(pseudo->otyp)  {

/* These spells are all duplicates of wand effects */
	case SPE_FORCE_BOLT:
	case SPE_SLEEP:
	case SPE_MAGIC_MISSILE:
	case SPE_SLOW_MONSTER:
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
			getdir(1);
			if(!u.dx && !u.dy && !u.dz && (u.ulevel > 8)) {
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
		seffects(pseudo);
		break;
	case SPE_HASTE_SELF:
	case SPE_DETECT_TREASURE:
	case SPE_DETECT_MONSTERS:
	case SPE_LEVITATION:
	case SPE_RESTORE_STRENGTH:
	case SPE_INVISIBILITY:
		peffects(pseudo);
		break;
	case SPE_HEALING:
		pline("You feel a bit better.");
		healup(rnd(8), 0, 0, 0);
		break;
	case SPE_CURE_BLINDNESS:
		healup(0, 0, 0, 1);
		break;
	case SPE_CURE_SICKNESS:
		pline("You are no longer ill.");
		healup(0, 0, 1, 0);
		break;
	case SPE_EXTRA_HEALING:
		pline("You feel a fair bit better.");
		healup(d(2,8), 1, 0, 0);
		break;
	case SPE_CREATE_FAMILIAR:
		{	register struct monst *mtmp;
			struct   monst  *makedog();

			mtmp = makedog();
			if(mtmp) {
				/* make it into something else */
				(void) newcham(mtmp, &mons[dlevel+14+rn2(CMNUM-14-dlevel)]);
				if(confused)
					mtmp->mtame = mtmp->mpeaceful = 0;
			}
		}
		break;
	default:
		impossible("Unknown spell %d attempted.", spell);
		obfree(pseudo, (struct obj *)0);
		return(0);
	}
	obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
	return(1);
}

getspell()  {

	register int	max, ilet, i;
	char	 lets[BUFSZ], buf[BUFSZ];

	if (spl_book[0].sp_id == NO_SPELL)  {

		pline("You don't know any spells right now.");
		return(0);
	} else  {

	    for(max = 1; (max < MAXSPELL) && (spl_book[max].sp_id != NO_SPELL); max++);
	    if (max >= MAXSPELL)  {

		impossible("Too many spells memorized.");
		return(0);
	    }

	    for(i = 0; (i < max) && (i < 26); buf[++i] = 0)  buf[i] = 'a' + i;
	    for(i = 26; (i < max) && (i < 52); buf[++i] = 0) buf[i] = 'A' + i - 26;

	    if (max == 1)  strcpy(lets, "a");
	    else if (max < 27)  sprintf(lets, "a-%c", 'a' + max - 1);
	    else if (max == 27)  sprintf(lets, "a-z A");
	    else sprintf(lets, "a-z A-%c", 'A' + max - 27);
	    for(;;)  {

		pline("Cast which spell [%s ?]: ", lets);
		if ((ilet = readchar()) == '?')  {
			dovspell();
			continue;
		} else if ((ilet == '\033')||(ilet == '\n')||(ilet == ' '))
			return(0);
		else for(i = 0; buf[i] != 0; i++)  if(ilet == buf[i])  return(++i);
		pline("You don't know that spell.");
	    }
	}
}

losespells() {
	register boolean confused = (Confusion != 0);
	register int	 n, nzap, i;

	for(n = 0;(spl_book[n].sp_id != NO_SPELL) && (n < MAXSPELL); n++);
	if (!n) return;
	if (n < MAXSPELL) {
		nzap = rnd(n);
		if (nzap < n) nzap += confused;
		for (i = 0; i < nzap; i++) spl_book[n-i-1].sp_id = NO_SPELL;
	} else impossible("Too many spells in spellbook!");
	return;
}

dovspell() {

	register int max, i, side;
	char     buf[BUFSZ],
		 *spellname();

	if (spl_book[0].sp_id == NO_SPELL)  {

		pline("You don't know any spells right now.");
		return(0);
	} else  {

	    for(max = 1; (max < MAXSPELL) && (spl_book[max].sp_id != NO_SPELL); max++);
	    if (max >= MAXSPELL)  {

		impossible("Too many spells memorized.");
		return(0);
	    }
	}
	set_pager(0);
	side = (max + 1) / 2;
	if(page_line("Currently known spells:") || page_line(""))  goto quit;

	for(i = 1; i <= side; i++) {

		if((i < side) || !(max % 2))  {

		    (void) sprintf(buf, "%c - (%d) %22s          %c - (%d) %22s",
				   spellet(i), spellev(i), spellname(i),
				   spellet(i + side), spellev(i + side), spellname(i + side));
		} else {

		    (void) sprintf(buf, "%c - (%d) %22s", spellet(i), spellev(i), spellname(i));
		}
		if(page_line(buf)) goto quit;
	}

	set_pager(1);
	return(0);
quit:
	set_pager(2);
	return(0);
}

spellet(spl)  {

	if (spl < 27)	return('a' + spl - 1);
	else		return('A' + spl - 27);
}

spellev(spl)  {

	return(spl_book[spl-1].sp_lev);
}

char *
spellname(spl)  {

	return(objects[spl_book[spl-1].sp_id].oc_name);
}

spellid(spl)  {		return(spl_book[spl-1].sp_id);		}

#ifdef HARD
spelluses(spell) {	return(spl_book[spell-1].sp_uses);	}
decrnuses(spell) {	spl_book[spell-1].sp_uses--;		}
#endif

#endif /* SPELLS /**/
