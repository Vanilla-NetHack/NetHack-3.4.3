/*	SCCS Id: @(#)spell.c	3.2	96/05/19	*/
/*	Copyright (c) M. Stephenson 1988			  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static NEARDATA schar delay;		/* moves left for this spell */
static NEARDATA struct obj *book;	/* last/current book being xscribed */

#define spelluses(spell)	spl_book[spell].sp_uses
#define decrnuses(spell)	spl_book[spell].sp_uses--
#define spellev(spell)		spl_book[spell].sp_lev
#define spellid(spell)		spl_book[spell].sp_id
#define spellname(spell)	OBJ_NAME(objects[spellid(spell)])
#define spellet(spell)	\
	((char)((spell < 26) ? ('a' + spell) : ('A' + spell - 26)))

static void FDECL(cursed_book, (int));
static void FDECL(deadbook, (struct obj *));
STATIC_PTR int NDECL(learn);
static boolean FDECL(getspell, (int *));
static boolean FDECL(dospellmenu, (int, int *));
static int FDECL(percent_success, (int));

/* The cl_sptmp table lists the class-specific values for tuning
 * percent_success().
 *
 * Reasoning:
 *   splcaster, special:
 *	A are aware of magic through historical research
 *	B abhor magic (Conan finds it "interferes with his animal instincts")
 *	C are ignorant to magic
 *	E are from a magical realm
 *	H are very aware of healing magic through medical research
 *	K are moderately aware of healing from Paladin training
 *	P are very aware of healing magic through theological research
 *	R are moderately aware of magic through trickery
 *	S have limited magical awareness, prefering meditation to conjuring
 *	T are aware of magic from all the great films they have seen
 *	V have limited magical awareness, prefering fighting
 *	W are trained mages
 *
 *	The arms penalty is lessened for trained fighters B, K, S, V -
 *	the penalty is its metal interference, not encumberance.
 *	The `specspel' is a single spell which is fundamentally easier
 *	 for that class to cast.
 *
 *  specspel, specbon:
 *	A map masters (SPE_MAGIC_MAPPING)
 *	B fugue/berserker (SPE_HASTE_SELF)
 *	C born to dig (SPE_DIG)
 *	E infra-like vision (SPE_DETECT_UNSEEN)
 *	H to heal (SPE_CURE_SICKNESS)
 *	K to turn back evil (SPE_TURN_UNDEAD)
 *	P to bless (SPE_REMOVE_CURSE)
 *	R to find loot (SPE_DETECT_TREASURE)
 *	S to be At One (SPE_CLAIRVOYANCE)
 *	T to smile (SPE_CHARM_MONSTER)
 *	V control the cold (SPE_CONE_OF_COLD)
 *	W all really, but SPE_MAGIC_MISSILE is their party trick
 *
 *	See percent_success() below for more comments.
 *
 *  uarmbon, uarmsbon, uarmhbon, uarmgbon, uarmfbon:
 *	Fighters find body armour & shield a little less limiting.
 *	Headgear, Gauntlets and Footwear are not class-specific (but
 *	still have an effect, except helm of brilliance, which is designed
 *	to permit magic-use).
 */
static struct sptmp {
	    char	class;		/* key */
	    int		splcaster;	/* base spellcasting ability */
	    int		special;	/* healing spell bonus */
	    int		uarmsbon;	/* penalty for wearing a (small) shield */
	    int		uarmbon;	/* penalty for wearing metal armour */
	    int		statused;	/* which stat is used */
	    int		specspel;	/* spell the class excels at */
	    int		specbon;	/* bonus when casting specspel */
} cl_sptmp[] = {
	    { 'A',  5, 0, 2, 10, A_INT, SPE_MAGIC_MAPPING,   -4 },
	    { 'B', 14, 0, 0,  8, A_INT, SPE_HASTE_SELF,      -4 },
	    { 'C', 12, 0, 1,  8, A_INT, SPE_DIG,             -4 },
	    { 'E',  5, 0, 1, 10, A_INT, SPE_DETECT_UNSEEN,   -4 },
	    { 'H',  3,-3, 2, 10, A_WIS, SPE_CURE_SICKNESS,   -4 },
	    { 'K',  8,-2, 0,  9, A_WIS, SPE_TURN_UNDEAD,     -4 },
	    { 'P',  3,-2, 2, 10, A_WIS, SPE_REMOVE_CURSE,    -4 },
	    { 'R',  8, 0, 1,  9, A_INT, SPE_DETECT_TREASURE, -4 },
	    { 'S', 10, 0, 0,  8, A_INT, SPE_CLAIRVOYANCE,    -4 },
#ifdef TOURIST
	    { 'T',  5, 1, 2, 10, A_INT, SPE_CHARM_MONSTER,   -4 },
#endif
	    { 'V', 10,-2, 0,  9, A_WIS, SPE_CONE_OF_COLD,    -4 },
	    { 'W',  1, 0, 3, 10, A_INT, SPE_MAGIC_MISSILE,   -4 },
	    {   0, 10, 0, 0,  4, A_INT, 0, -3 }
};

#define uarmhbon 4 /* Metal helmets interfere with the mind */
#define uarmgbon 6 /* Casting channels through the hands */
#define uarmfbon 2 /* All metal interferes to some degree */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
static const char explodes[] = "radiates explosive energy";

static void
cursed_book(lev)
	register int	lev;
{
	switch(rn2(lev)) {
	case 0:
		You_feel("a wrenching sensation.");
		tele();		/* teleport him */
		break;
	case 1:
		You_feel("threatened.");
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
		pline_The("book was coated with contact poison!");
		if (uarmg) {
		    /* Note: at this writing, there are no corrodeable
		     * gloves in the game.  If no one plans on adding
		     * copper gauntlets, most of this could be removed. -3.
		     */
		    if (uarmg->oerodeproof || !is_corrodeable(uarmg)) {
			Your("gloves seem unaffected.");
		    } else if (uarmg->oeroded < MAX_ERODE) {
			Your("gloves corrode%s!",
			     uarmg->oeroded+1 == MAX_ERODE ? " completely" :
			     uarmg->oeroded ? " further" : "");
			uarmg->oeroded++;
		    } else
			Your("gloves %s completely corroded.",
			     Blind ? "feel" : "look");
		    break;
		}
		losestr(Poison_resistance ? rn1(2,1) : rn1(4,3));
		losehp(rnd(Poison_resistance ? 6 : 10),
		       "contact-poisoned spellbook", KILLED_BY_AN);
		break;
	case 6:
		if(Antimagic) {
		    shieldeff(u.ux, u.uy);
		    pline_The("book %s, but you are unharmed!", explodes);
		} else {
		    pline("As you read the book, it %s in your %s!",
			  explodes, body_part(FACE));
		    losehp (2*rnd(10)+5, "exploding rune", KILLED_BY_AN);
		}
		break;
	default:
		rndcurse();
		break;
	}
	return;
}

/* special effects for The Book of the Dead */
static void
deadbook(book2)
struct obj *book2;
{
    struct monst *mtmp, *mtmp2;
    coord mm;

    You("turn the pages of the Book of the Dead...");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    if(invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
	register struct obj *otmp;
	register boolean arti1_primed = FALSE, arti2_primed = FALSE,
			 arti_cursed = FALSE;

	if(book2->cursed) {
	    pline_The("runes appear scrambled.  You can't read them!");
	    return;
	}

	if(!u.uhave.bell || !u.uhave.menorah) {
	    pline("A chill runs down your %s.", body_part(SPINE));
	    if(!u.uhave.bell) You_hear("a faint chime...");
	    if(!u.uhave.menorah) pline("Vlad's doppelganger is amused.");
	    return;
	}

	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->otyp == CANDELABRUM_OF_INVOCATION &&
	       otmp->spe == 7 && otmp->lamplit) {
		if(!otmp->cursed) arti1_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	    if(otmp->otyp == BELL_OF_OPENING &&
	       (moves - otmp->age) < 5L) { /* you rang it recently */
		if(!otmp->cursed) arti2_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	}

	if(arti_cursed) {
	    pline_The("invocation fails!");
	    pline("At least one of your artifacts is cursed...");
	} else if(arti1_primed && arti2_primed) {
	    mkinvokearea();
	    u.uevent.invoked = 1;
	} else {	/* at least one artifact not prepared properly */
	    You("have a feeling that %s is amiss...", something);
	    goto raise_dead;
	}
	return;
    }

    /* when not an invocation situation */
    if (book2->cursed) {
raise_dead:

	You("raised the dead!");
	/* first maybe place a dangerous adversary */
	if (!rn2(3) && ((mtmp = makemon(&mons[PM_MASTER_LICH],
					u.ux, u.uy, NO_MINVENT)) != 0 ||
			(mtmp = makemon(&mons[PM_NALFESHNEE],
					u.ux, u.uy, NO_MINVENT)) != 0)) {
	    mtmp->mpeaceful = 0;
	    set_malign(mtmp);
	}
	/* next handle the affect on things you're carrying */
	(void) unturn_dead(&youmonst);
	/* last place some monsters around you */
	mm.x = u.ux;
	mm.y = u.uy;
	mkundead(&mm, TRUE, NO_MINVENT);
    } else if(book2->blessed) {
	for(mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;		/* tamedog() changes chain */
	    if(is_undead(mtmp->data) && cansee(mtmp->mx, mtmp->my)) {
		mtmp->mpeaceful = TRUE;
		if(sgn(mtmp->data->maligntyp) == sgn(u.ualign.type)
		   && distu(mtmp->mx, mtmp->my) < 4)
		    if (mtmp->mtame)
			mtmp->mtame++;
		    else
			(void) tamedog(mtmp, (struct obj *)0);
		else mtmp->mflee = TRUE;
	    }
	}
    } else {
	switch(rn2(3)) {
	case 0:
	    Your("ancestors are annoyed with you!");
	    break;
	case 1:
	    pline_The("headstones in the cemetery begin to move!");
	    break;
	default:
	    pline("Oh my!  Your name appears in the book!");
	}
    }
    return;
}

STATIC_PTR int
learn()
{
	int i;
	short booktype;

	if (delay) {	/* not if (delay++), so at end delay == 0 */
		delay++;
		return(1); /* still busy */
	}
	exercise(A_WIS, TRUE);		/* you're studying. */
	booktype = book->otyp;
	if(booktype == SPE_BOOK_OF_THE_DEAD) {
	    deadbook(book);
	    return(0);
	}

	for (i = 0; i < MAXSPELL; i++)  {
		if (spellid(i) == booktype)  {
			if (book->spestudied >= rnd(30 - spellev(i))) {
			    pline("This spellbook is too faint to be read anymore.");
			    book->otyp = booktype = SPE_BLANK_PAPER;
			}  else if (spelluses(i) < 20 - spellev(i)) {
			    Your("knowledge of that spell is keener.");
			    spl_book[i].sp_uses += 10 - spellev(i);
			    book->spestudied++;
			    exercise(A_WIS, TRUE);	/* extra study */
			} else
			    You("know that spell quite well already.");
			/* make book become known even when spell is already
			   known, in case amnesia made you forget the book */
			makeknown((int)booktype);
			break;
		} else if (spellid(i) == NO_SPELL)  {
			spl_book[i].sp_id = booktype;
			spl_book[i].sp_lev = objects[booktype].oc_level;
			spl_book[i].sp_uses = 30 - spellev(i);
			book->spestudied++;
			You("add the spell to your repertoire.");
			makeknown((int)booktype);
			break;
		}
	}
	if (i == MAXSPELL) impossible("Too many spells memorized!");

	if (book->cursed) {	/* maybe a demon cursed it */
		cursed_book(objects[booktype].oc_level);
	}
	check_unpaid(book);
	book = 0;
	return(0);
}

int
study_book(spellbook)
register struct obj *spellbook;
{
	register int	 booktype = spellbook->otyp;
	register boolean confused = (Confusion != 0);

	if (delay && spellbook == book)
		You("continue your efforts to memorize the spell.");
	else {
		switch(booktype)  {

	/* blank spellbook */
		case SPE_BLANK_PAPER:
			pline("This spellbook is all blank.");
			makeknown(SPE_BLANK_PAPER);
			return(1);
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
			delay = -(objects[booktype].oc_level - 1) * objects[booktype].oc_delay;
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
			delay = -objects[booktype].oc_level * objects[booktype].oc_delay;
			break;
	/* level 7 spells */
		case SPE_CANCELLATION:
		case SPE_FINGER_OF_DEATH:
		case SPE_BOOK_OF_THE_DEAD:
			delay = -8 * objects[booktype].oc_delay;
			break;
	/* impossible */
		default:
			impossible("Unknown spellbook, %d;", booktype);
		return(0);
		}

		/* Books are often wiser than their readers (Rus.) */
#ifndef NO_SIGNAL
		spellbook->in_use = TRUE;
#endif
		if(!spellbook->blessed &&
			spellbook->otyp != SPE_BOOK_OF_THE_DEAD &&
			(spellbook->cursed ||
			    rn2(20) > (ACURR(A_INT) + 4 + u.ulevel/2
					- 2*objects[booktype].oc_level))) {
			cursed_book(objects[booktype].oc_level);
			nomul(delay);			/* study time */
			delay = 0;
			if(!rn2(3)) {
			    pline_The("spellbook crumbles to dust!");
			    if (!objects[spellbook->otyp].oc_name_known &&
				   !objects[spellbook->otyp].oc_uname)
				docall(spellbook);
			    useup(spellbook);
			}
#ifndef NO_SIGNAL
			else
			    spellbook->in_use = FALSE;
#endif
			return(1);
		} else if (confused) {
			if (!rn2(3) &&
				spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
			    pline(
	  "Being confused you have difficulties in controlling your actions.");
			    display_nhwindow(WIN_MESSAGE, FALSE);
			    You("accidentally tear the spellbook to pieces.");
			    if (!objects[spellbook->otyp].oc_name_known &&
				   !objects[spellbook->otyp].oc_uname)
				docall(spellbook);
			    useup(spellbook);
			} else {
			    You(
		  "find yourself reading the first line over and over again.");
#ifndef NO_SIGNAL
			    spellbook->in_use = FALSE;
#endif
			}
			nomul(delay);
			delay = 0;
			return(1);
		}
#ifndef NO_SIGNAL
		spellbook->in_use = FALSE;
#endif

		You("begin to %s the runes.",
		    spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" :
		    "memorize");
	}

	book = spellbook;
	set_occupation(learn, "studying", 0);
	return(1);
}

/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
static boolean
getspell(spell_no)
	int *spell_no;
{
	int nspells, idx;
	char ilet, lets[BUFSZ], qbuf[QBUFSZ];

	if (spellid(0) == NO_SPELL)  {
	    You("don't know any spells right now.");
	    return FALSE;
	}
	if (flags.menu_style == MENU_TRADITIONAL) {
	    /* we know there is at least 1 known spell */
	    for (nspells = 1; nspells < MAXSPELL
			    && spellid(nspells) != NO_SPELL; nspells++)
		continue;

	    if (nspells == 1)  Strcpy(lets, "a");
	    else if (nspells < 27)  Sprintf(lets, "a-%c", 'a' + nspells - 1);
	    else if (nspells == 27)  Sprintf(lets, "a-z A");
	    else Sprintf(lets, "a-z A-%c", 'A' + nspells - 27);

	    for(;;)  {
		Sprintf(qbuf, "Cast which spell? [%s ?]", lets);
		if ((ilet = yn_function(qbuf, (char *)0, '\0')) == '?')
		    break;

		if (index(quitchars, ilet))
		    return FALSE;

		if (letter(ilet) && ilet != '@') {
		    /* in a-zA-Z, convert back to an index */
		    if (lowc(ilet) == ilet)	/* lower case */
			idx = ilet - 'a';
		    else
			idx = ilet - 'A' + 26;

		    if (idx < nspells) {
			*spell_no = idx;
			return TRUE;
		    }
		}
		You("don't know that spell.");
	    }
	}
	return dospellmenu(PICK_ONE, spell_no);
}

int
docast()
{
	int spell_no;

	if (getspell(&spell_no))
	    return spelleffects(spell_no, FALSE);
	return 0;
}

int
spelleffects(spell, atme)
int spell;
boolean atme;
{
	int energy, damage, chance;
	boolean confused = (Confusion != 0);
	struct obj *pseudo;

	/* note that trying to cast it decrements the # of uses,    */
	/* even if the mage does not have enough food/energy to use */
	/* the spell */
	switch (spelluses(spell)) {
		case 0:
		    pline ("Curdled magical energy twists through you...");
		    pline ("...you have overloaded and burned out this spell.");
		    make_confused((long)spellev(spell) * 3, FALSE);
		    return(0);
		case 1:
		case 2:
		case 3:
		    Your("nerves tingle warningly.");
		    break;
		case 4:
		case 5:
		case 6:
		    pline ("This spell is starting to be over-used.");
		    break;
		default:
		    break;
	}
	decrnuses(spell);
	energy = (spellev(spell) * 5);    /* 5 <= energy <= 35 */
	
	if (u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
		You("are too hungry to cast that spell.");
		return(0);
	} else if (ACURR(A_STR) < 4)  {
		You("lack the strength to cast spells.");
		return(0);
	} else if(check_capacity(
		"Your concentration falters while carrying so much stuff.")) {
	    return (1);
	} else if (!freehand()) {
		Your("arms are not free to cast!");
		return (0);
	}


	if (u.uhave.amulet) {
		You_feel("the amulet draining your energy away.");
		energy += rnd(2*energy);
	}
	if(energy > u.uen)  {
		You("don't have enough energy to cast that spell.");
		return(0);
	} else {
		if (spellid(spell) != SPE_DETECT_FOOD) {
			int hungr = energy * 2;

			/* don't put player (quite) into fainting from
			 * casting a spell, particularly since they might
			 * not even be hungry at the beginning; however,
			 * this is low enough that they must eat before
			 * casting anything else except detect food
			 */
			if (hungr > u.uhunger-3)
				hungr = u.uhunger-3;
			morehungry(hungr);
		}
	}

	/* u.uen _will_ reduce once here reached */

	flags.botl = 1;

	chance = percent_success(spell);

	if (confused || (rnd(100) > chance)) {
		You("fail to cast the spell correctly.");
		u.uen -= energy / 2;
		return(1);
	}

	u.uen -= energy;
	exercise(A_WIS, TRUE);
/*	pseudo is a temporary "false" object containing the spell stats. */
	pseudo = mksobj(spellid(spell), FALSE, FALSE);
	pseudo->blessed = pseudo->cursed = 0;
	pseudo->quan = 20L;			/* do not let useup get it */
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
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
		if (!(objects[pseudo->otyp].oc_dir == NODIR)) {
			if (atme) u.dx = u.dy = u.dz = 0;
			else (void) getdir((char *)0);
			if(!u.dx && !u.dy && !u.dz) {
			    if ((damage = zapyourself(pseudo, TRUE)) != 0)
				losehp(damage,
				     self_pronoun("zapped %sself with a spell",
						  "him"),
				     NO_KILLER_PREFIX);
			} else weffects(pseudo);
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
	case SPE_CURE_BLINDNESS:
		healup(0, 0, FALSE, TRUE);
		break;
	case SPE_CURE_SICKNESS:
		if (Sick) You("are no longer ill.");
		healup(0, 0, TRUE, FALSE);
		break;
	case SPE_CREATE_FAMILIAR:
		make_familiar((struct obj *)0, u.ux, u.uy);
		break;
	case SPE_CLAIRVOYANCE:
		if (!(HClairvoyant & I_BLOCKED))
		    do_vicinity_map();
		/* at present, only one thing blocks clairvoyance */
		else if (uarmh && uarmh->otyp == CORNUTHAUM)
		    You("sense a pointy hat on top of your %s.",
			body_part(HEAD));
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
losespells()
{
	boolean confused = (Confusion != 0);
	int  n, nzap, i;

	book = 0;
	for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; n++)
		continue;
	if (n) {
		nzap = rnd(n) + confused ? 1 : 0;
		if (nzap > n) nzap = n;
		for (i = n - nzap; i < n; i++) {
		    spellid(i) = NO_SPELL;
		    exercise(A_WIS, FALSE);	/* ouch! */
		}
	}
}

int
dovspell()
{
	int dummy;

	if (spellid(0) == NO_SPELL)
	    You("don't know any spells right now.");
	else
	    (void) dospellmenu(PICK_NONE, &dummy);
	return 0;
}

static boolean
dospellmenu(how, spell_no)
	int how;
	int *spell_no;
{
	winid tmpwin;
	int i, n;
	char buf[BUFSZ];
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	/*
	 * The correct spacing of the columns depends on the
	 * following that (1) the font is monospaced and (2)
	 * that selection letters are pre-pended to the given
	 * string and are of the form "a - ".
	 *
	 * To do it right would require that we implement columns
	 * in the window-ports (say via a tab character).
	 */
	Sprintf(buf, "%-20s     Level Fail", "Name");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
		Sprintf(buf, "%-20s  %2d%s  %3d%%",
			spellname(i), spellev(i),
			spelluses(i) ? " " : "*", 100 - percent_success(i));

		any.a_int = i+1;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			 spellet(i), 0, ATR_NONE, buf, MENU_UNSELECTED);
	      }
	end_menu(tmpwin, how == PICK_ONE ? "Choose a spell" :
					   "Currently known spells");

	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	if (n > 0) {
		*spell_no = selected[0].item.a_int - 1;
		free((genericptr_t)selected);
		return TRUE;
	}
	return FALSE;
}

static int
percent_success(spell)
int spell;
{
	/* Intrinsic and learned ability are combined to calculate
	 * the probability of player's success at cast a given spell.
	 */

	int i, chance, splcaster, special, statused;
	int difficulty;

	/* Calculate intrinsic ability (splcaster) */

	for (i = 0; cl_sptmp[i].class; i++)
		if (cl_sptmp[i].class == pl_character[0]) break;

	splcaster = cl_sptmp[i].splcaster;
	special = cl_sptmp[i].special;

	if (uarm && is_metallic(uarm)) splcaster += cl_sptmp[i].uarmbon;
	if (uarms) splcaster += cl_sptmp[i].uarmsbon;

	if (uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE)
		splcaster += uarmhbon;
	if (uarmg && is_metallic(uarmg)) splcaster += uarmgbon;
	if (uarmf && is_metallic(uarmf)) splcaster += uarmfbon;

	if (spellid(spell) == cl_sptmp[i].specspel)
		splcaster += cl_sptmp[i].specbon;

	statused = ACURR(cl_sptmp[i].statused);

	/* `healing spell' bonus */
	if (spellid(spell) == SPE_HEALING ||
	    spellid(spell) == SPE_EXTRA_HEALING ||
	    spellid(spell) == SPE_CURE_BLINDNESS ||
	    spellid(spell) == SPE_CURE_SICKNESS ||
	    spellid(spell) == SPE_RESTORE_ABILITY ||
	    spellid(spell) == SPE_REMOVE_CURSE) splcaster += special;

	if (splcaster > 20) splcaster = 20;

	/* Calculate learned ability */

	/* Players basic likelihood of being able to cast any spell
	 * is based of their `magic' statistic. (Int or Wis)
	 */
	chance = 11 * statused / 2;

	/* High level spells are harder.  Easier for higher level casters */
	difficulty = (spellev(spell) - 1) * 4 - (u.ulevel - 1);

	if (difficulty > 0) {
		/* Player is too low level.  Exponential chance reduction */
		chance -= 7 * difficulty * difficulty;
	} else {
		/* Player is above level.  Learning continues, but the
		 * law of diminishing returns sets in quickly for
		 * low-level spells.  That is, a player quickly gains
		 * no advantage for raising level.
		 */
		int learning = 15 * -difficulty / spellev(spell);
		chance += learning > 20 ? 20 : learning;
	}

	/* Clamp the chance: >18 stat and advanced learning only help
	 * to a limit, while chances below "hopeless" only raise the
	 * specter of overflowing 16-bit ints (and permit wearing a
	 * shield to raise the chances :-).
	 */
	if (chance < 0) chance = 0;
	if (chance > 120) chance = 120;

	/* Wearing anything but a light shield makes it very awkward
	 * to cast a spell.  The penalty is not quite so bad for the
	 * player's class-specific spell.
	 */
	if (uarms && weight(uarms) > (int) objects[SMALL_SHIELD].oc_weight) {
		if (spellid(spell) == cl_sptmp[i].specspel) {
			chance /= 2;
		} else {
			chance /= 4;
		}
	}

	/* Finally, chance (based on player intell/wisdom and level) is
	 * combined with ability (based on player intrinsics and
	 * encumberances).  No matter how intelligent/wise and advanced
	 * a player is, intrinsics and encumberance can prevent casting;
	 * and no matter how able, learning is always required.
	 */
	chance = chance * (20-splcaster) / 15 - splcaster;

	/* Clamp to percentile */
	if (chance > 100) chance = 100;
	if (chance < 0) chance = 0;

	return chance;
}

/*spell.c*/
