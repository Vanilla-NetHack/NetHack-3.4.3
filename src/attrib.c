/*	SCCS Id: @(#)attrib.c	3.2	96/03/28	*/
/*	Copyright 1988, 1989, 1990, 1992, M. Stephenson		  */
/* NetHack may be freely redistributed.  See license for details. */

/*  attribute modification routines. */

#include "hack.h"
#include "artifact.h"

/* #define DEBUG	/* uncomment for debugging info */

#ifdef OVLB

	/* part of the output on gain or loss of attribute */
static
const char	*plusattr[] = {
	"strong", "smart", "wise", "agile", "tough", "charismatic"
},
		*minusattr[] = {
	"weak", "stupid", "foolish", "clumsy", "vulnerable", "ugly"
};

	/* maximum and minimum values for the attributes */
struct attribs	attrmax = {
	{ 118, 18, 18, 18, 18, 18 }
},
		attrmin = {
	{ 3, 3, 3, 3, 3, 3 }
};

static
const struct innate {
	schar	ulevel;
	long	*ability;
	const char *gainstr, *losestr;
}	a_abil[] = { {	 1, &(Stealth), "", "" },
		     {   1, &(Fast), "", "" },
		     {  10, &(Searching), "perceptive", "" },
		     {	 0, 0, 0, 0 } },

	b_abil[] = { {	 1, &(HPoison_resistance), "", "" },
		     {   7, &(Fast), "quick", "slow" },
		     {  15, &(Stealth), "stealthy", "" },
		     {	 0, 0, 0, 0 } },

	c_abil[] = { {	 7, &(Fast), "quick", "slow" },
		     {	15, &(Warning), "sensitive", "" },
		     {	 0, 0, 0, 0 } },

	e_abil[] = { {   1, &(Fast), "", "" },
		     {	 1, &(HSee_invisible), "", "" },
		     {	 1, &(Searching), "", "" },
		     {	 1, &(HSleep_resistance), "", "" },
		     {	 0, 0, 0, 0 } },

	h_abil[] = { {	 1, &(HPoison_resistance), "", "" },
		     {	15, &(Warning), "sensitive", "" },
		     {	 0, 0, 0, 0 } },

	k_abil[] = { {	 7, &(Fast), "quick", "slow" },
		     {	 0, 0, 0, 0 } },

	p_abil[] = { {	15, &(Warning), "sensitive", "" },
		     {  20, &(HFire_resistance), "cool", "warmer" },
		     {	 0, 0, 0, 0 } },

	r_abil[] = { {	 1, &(Stealth), "", ""  },
		     {  10, &(Searching), "perceptive", "" },
		     {	 0, 0, 0, 0 } },

	s_abil[] = { {	 1, &(Fast), "", "" },
		     {  15, &(Stealth), "stealthy", "" },
		     {	 0, 0, 0, 0 } },

	t_abil[] = { {	10, &(Searching), "perceptive", "" },
		     {	20, &(HPoison_resistance), "hardy", "" },
		     {	 0, 0, 0, 0 } },

	v_abil[] = { {	 1, &(HCold_resistance), "", "" },
		     {	 1, &(Stealth), "", "" },
		     {   7, &(Fast), "quick", "slow" },
		     {	 0, 0, 0, 0 } },

	w_abil[] = { {	15, &(Warning), "sensitive", "" },
		     {  17, &(HTeleport_control), "controlled","uncontrolled" },
		     {	 0, 0, 0, 0 } };

static
const struct clattr {
	struct	attribs	base, cldist;
	align	align;
	schar	shp, hd, xlev, ndx;
/* According to AD&D, HD for some classes (ex. Wizard) should be smaller
 * (4-sided for wizards).  But this is not AD&D, and using the AD&D
 * rule here produces an unplayable character.  Thus I have used a minimum
 * of an 10-sided hit die for everything.  Another AD&D change: wizards get
 * a minimum strength of 6 since without one you can't teleport or cast
 * spells. --KAA
 */
	const struct	innate *abil;
}	a_attr = { {{  7, 10, 10,  7,  7,  7 }},  /* Archeologist */
		   {{ 20, 20, 20, 10, 20, 10 }},
		   { A_LAWFUL,  10 },  13, 10, 14,  2, a_abil },

	b_attr = { {{ 16,  7,  7, 15, 16,  6 }},  /* Barbarian */
		   {{ 30,  6,  7, 20, 30,  7 }},
		   { A_NEUTRAL, 10 },  16, 12, 10,  3, b_abil },

	c_attr = { {{ 10,  7,  7,  7,  8,  6 }},  /* Caveman (fighter) */
		   {{ 30,  6,  7, 20, 30,  7 }},
		   { A_LAWFUL,   0 },  16, 10, 10,  3, c_abil },

	e_attr = { {{ 13, 13, 13,  9, 13,  7 }},  /* Elf (ranger) */
		   {{ 30, 10, 10, 20, 20, 10 }},
		   { A_CHAOTIC, 10 },  15, 10, 11,  2, e_abil },

	h_attr = { {{  7,  7, 13,  7, 11, 16 }},  /* Healer (druid) */
		   {{ 15, 20, 20, 15, 25, 10 }},
		   { A_NEUTRAL, 10 },  13, 10, 20,  2, h_abil },

	k_attr = { {{ 13,  7, 14,  8, 10, 17 }},  /* Knight (paladin) */
		   {{ 20, 15, 15, 10, 20, 10 }},
		   { A_LAWFUL,  10 },  16, 10, 10,  3, k_abil },

	p_attr = { {{  7,  7, 10,  7,  7,  7 }},  /* Priest (cleric) */
		   {{ 15, 10, 30, 15, 20, 10 }},
		   { A_NEUTRAL,  0 },  14, 10, 10,  2, p_abil },

	r_attr = { {{  7,  7,  7, 10,  7,  6 }},  /* Rogue (thief) */
		   {{ 20, 10, 10, 30, 20, 10 }},
		   { A_CHAOTIC, 10 },  12, 10, 11,  2, r_abil },

	s_attr = { {{ 10,  8,  7, 10, 17,  6 }},  /* Samurai (fighter/thief) */
		   {{ 30, 10, 10, 30, 14, 10 }},
		   { A_LAWFUL,  10 },  15, 10, 11,  2, s_abil },

#ifdef TOURIST
	t_attr = { {{  7, 10,  6,  7,  7, 10 }},  /* Tourist */
		   {{ 15, 10, 10, 15, 30, 20 }},
		   { A_NEUTRAL,  0 },  10, 10, 14,  1, t_abil },
#endif

	v_attr = { {{ 10,  7,  7,  7, 10,  7 }},  /* Valkyrie (fighter) */
		   {{ 30,  6,  7, 20, 30,  7 }},
		   { A_NEUTRAL,  0 },  16, 10, 10,  3, v_abil },

	w_attr = { {{  7, 10,  7,  7,  7,  7 }},  /* Wizard (magic-user) */
		   {{ 10, 30, 10, 20, 20, 10 }},
		   { A_NEUTRAL,  0 },  12, 10, 12,  1, w_abil },

	X_attr = { {{  3,  3,  3,  3,  3,  3 }},
		   {{ 20, 15, 15, 15, 20, 15 }},
		   { A_NEUTRAL,  0 },  12, 10, 14,  1,  0 };

static long next_check = 600L;	/* arbitrary first setting */
static NEARDATA const struct clattr *NDECL(clx);
static void NDECL(init_align);
static void NDECL(exerper);

/* adjust an attribute; return TRUE if change is made, FALSE otherwise */
boolean
adjattrib(ndx, incr, msgflg)
	int	ndx, incr;
	int	msgflg;	    /* positive => no message, zero => message, and */
{			    /* negative => conditional (msg if change made) */
	if (!incr) return FALSE;

	if ((ndx == A_INT || ndx == A_WIS)
				&& uarmh && uarmh->otyp == DUNCE_CAP) {
		if (msgflg == 0)
		    Your("cap constricts briefly, then relaxes again.");
		return FALSE;
	}

	if (incr > 0) {
	    if ((AMAX(ndx) >= ATTRMAX(ndx)) && (ACURR(ndx) >= AMAX(ndx))) {
		if (msgflg == 0 && flags.verbose)
		    pline("You're already as %s as you can get.",
			  plusattr[ndx]);
		ABASE(ndx) = AMAX(ndx) = ATTRMAX(ndx); /* just in case */
		return FALSE;
	    }

	    ABASE(ndx) += incr;
	    if(ABASE(ndx) > AMAX(ndx)) {
		incr = ABASE(ndx) - AMAX(ndx);
		AMAX(ndx) += incr;
		if(AMAX(ndx) > ATTRMAX(ndx))
		    AMAX(ndx) = ATTRMAX(ndx);
		ABASE(ndx) = AMAX(ndx);
	    }
	} else {
	    if (ABASE(ndx) <= ATTRMIN(ndx)) {
		if (msgflg == 0 && flags.verbose)
		    pline("You're already as %s as you can get.",
			  minusattr[ndx]);
		ABASE(ndx) = ATTRMIN(ndx); /* just in case */
		return FALSE;
	    }

	    ABASE(ndx) += incr;
	    if(ABASE(ndx) < ATTRMIN(ndx)) {
		incr = ABASE(ndx) - ATTRMIN(ndx);
		ABASE(ndx) = ATTRMIN(ndx);
		AMAX(ndx) += incr;
		if(AMAX(ndx) < ATTRMIN(ndx))
		    AMAX(ndx) = ATTRMIN(ndx);
	    }
	}
	if (msgflg <= 0)
	    You_feel("%s%s!",
		  (incr > 1 || incr < -1) ? "very ": "",
		  (incr > 0) ? plusattr[ndx] : minusattr[ndx]);
	flags.botl = 1;
	if (moves > 0 && (ndx == A_STR || ndx == A_CON))
		(void)encumber_msg();
	return TRUE;
}

void
gainstr(otmp, incr)
	register struct obj *otmp;
	register int incr;
{
	int num = 1;

	if(incr) num = incr;
	else {
	    if(ABASE(A_STR) < 18) num = (rn2(4) ? 1 : rnd(6) );
	    else if (ABASE(A_STR) < 103) num = rnd(10);
	}
	(void) adjattrib(A_STR, (otmp && otmp->cursed) ? -num : num, TRUE);
}

void
losestr(num)	/* may kill you; cause may be poison or monster like 'a' */
	register int num;
{
	int ustr = ABASE(A_STR) - num;

	while(ustr < 3) {
		ustr++;
		num--;
		u.uhp -= 6;
		u.uhpmax -= 6;
	}
	(void) adjattrib(A_STR, -num, TRUE);
}

void
change_luck(n)
	register schar n;
{
	u.uluck += n;
	if (u.uluck < 0 && u.uluck < LUCKMIN)	u.uluck = LUCKMIN;
	if (u.uluck > 0 && u.uluck > LUCKMAX)	u.uluck = LUCKMAX;
}

int
stone_luck(parameter)
boolean parameter; /* So I can't think up of a good name.  So sue me. --KAA */
{
	register struct obj *otmp;
	register long bonchance = 0;

	for(otmp = invent; otmp; otmp=otmp->nobj)
	    if (otmp->otyp == LUCKSTONE
		|| (otmp->oartifact && spec_ability(otmp, SPFX_LUCK))) {
		if (otmp->cursed) bonchance -= otmp->quan;
		else if (otmp->blessed) bonchance += otmp->quan;
		else if (parameter) bonchance += otmp->quan;
	    }

	return sgn((int)bonchance);
}

/* there has just been an inventory change affecting a luck-granting item */
void
set_moreluck()
{
	int luckbon = stone_luck(TRUE);

	if (!luckbon && !carrying(LUCKSTONE)) u.moreluck = 0;
	else if (luckbon >= 0) u.moreluck = LUCKADD;
	else u.moreluck = -LUCKADD;
}

#endif /* OVLB */
#ifdef OVL1

void
restore_attrib()
{
	int	i;

	for(i = 0; i < A_MAX; i++) {	/* all temporary losses/gains */

	   if(ATEMP(i) && ATIME(i)) {
		if(!(--(ATIME(i)))) { /* countdown for change */
		    ATEMP(i) += ATEMP(i) > 0 ? -1 : 1;

		    if(ATEMP(i)) /* reset timer */
			ATIME(i) = 100 / ACURR(A_CON);
		}
	    }
	}
	(void)encumber_msg();
}

#endif /* OVL1 */
#ifdef OVLB

#define AVAL	50		/* tune value for exercise gains */

void
exercise(i, inc_or_dec)
int	i;
boolean	inc_or_dec;
{
#ifdef DEBUG
	pline("Exercise:");
#endif
	if (i == A_INT || i == A_CHA) return;	/* can't exercise these */

	/* no physical exercise while polymorphed; the body's temporary */
	if (u.umonnum >= LOW_PM && i != A_WIS) return;

	if(abs(AEXE(i)) < AVAL) {
		/*
		 *	Law of diminishing returns (Part I):
		 *
		 *	Gain is harder at higher attribute values.
		 *	79% at "3" --> 0% at "18"
		 *	Loss is even at all levels (50%).
		 *
		 *	Note: *YES* ACURR is the right one to use.
		 */
		AEXE(i) += (inc_or_dec) ? (rn2(19) > ACURR(i)) : -rn2(2);
#ifdef DEBUG
		pline("%s, %s AEXE = %d",
			(i == A_STR) ? "Str" : (i == A_WIS) ? "Wis" :
			(i == A_DEX) ? "Dex" : "Con",
			(inc_or_dec) ? "inc" : "dec", AEXE(i));
#endif
	}
	if (moves > 0 && (i == A_STR || i == A_CON)) (void)encumber_msg();
}

/* hunger values - from eat.c */
#define SATIATED	0
#define NOT_HUNGRY	1
#define HUNGRY		2
#define WEAK		3
#define FAINTING	4
#define FAINTED		5
#define STARVED		6

static void
exerper()
{
	if(!(moves % 10)) {
		/* Hunger Checks */

		int hs = (u.uhunger > 1000) ? SATIATED :
			 (u.uhunger > 150) ? NOT_HUNGRY :
			 (u.uhunger > 50) ? HUNGRY :
			 (u.uhunger > 0) ? WEAK : FAINTING;

#ifdef DEBUG
		pline("exerper: Hunger checks");
#endif
		switch (hs) {
		    case SATIATED:	exercise(A_DEX, FALSE); break;
		    case NOT_HUNGRY:	exercise(A_CON, TRUE); break;
		    case WEAK:		exercise(A_STR, FALSE); break;
		    case FAINTING:
		    case FAINTED:	exercise(A_CON, FALSE); break;
		}

		/* Encumberance Checks */
#ifdef DEBUG
		pline("exerper: Encumber checks");
#endif
		switch (near_capacity()) {
		    case MOD_ENCUMBER:	exercise(A_STR, TRUE); break;
		    case HVY_ENCUMBER:	exercise(A_STR, TRUE);
					exercise(A_DEX, FALSE); break;
		    case EXT_ENCUMBER:	exercise(A_DEX, FALSE);
					exercise(A_CON, FALSE); break;
		}

	}

	/* status checks */
	if(!(moves % 5)) {
#ifdef DEBUG
		pline("exerper: Status checks");
#endif
		if ((HClairvoyant & (INTRINSIC|TIMEOUT)) &&
			!(HClairvoyant & I_BLOCKED))	exercise(A_WIS, TRUE);
		if (HRegeneration)			exercise(A_STR, TRUE);

		if(Sick || Vomiting)			exercise(A_CON, FALSE);
		if(Confusion || Hallucination)		exercise(A_WIS, FALSE);
		if(Wounded_legs || Fumbling || HStun)	exercise(A_DEX, FALSE);
	}
}

void
exerchk()
{
	int	i, mod_val;

	/*	Check out the periodic accumulations */
	exerper();

#ifdef DEBUG
	if(moves >= next_check)
		pline("exerchk: ready to test. multi = %d.", multi);
#endif
	/*	Are we ready for a test?	*/
	if(moves >= next_check && !multi) {
#ifdef DEBUG
	    pline("exerchk: testing.");
#endif
	    /*
	     *	Law of diminishing returns (Part II):
	     *
	     *	The effects of "exercise" and "abuse" wear
	     *	off over time.  Even if you *don't* get an
	     *	increase/decrease, you lose some of the
	     *	accumulated effects.
	     */
	    for(i = 0; i < A_MAX; AEXE(i++) /= 2) {

		if(ABASE(i) >= 18 || !AEXE(i)) continue;
		if(i == A_INT || i == A_CHA) continue;/* can't exercise these */

#ifdef DEBUG
		pline("exerchk: testing %s (%d).",
			(i == A_STR) ? "Str" : (i == A_WIS) ? "Wis" :
			(i == A_DEX) ? "Dex" : "Con", AEXE(i));
#endif
		/*
		 *	Law of diminishing returns (Part III):
		 *
		 *	You don't *always* gain by exercising.
		 *	[MRS 92/10/28 - Treat Wisdom specially for balance.]
		 */
		if(rn2(AVAL) > ((i != A_WIS) ? abs(AEXE(i)*2/3) : abs(AEXE(i))))
		    continue;
		mod_val = sgn(AEXE(i));

#ifdef DEBUG
		pline("exerchk: changing %d.", i);
#endif
		if(adjattrib(i, mod_val, -1)) {
#ifdef DEBUG
		    pline("exerchk: changed %d.", i);
#endif
		    /* if you actually changed an attrib - zero accumulation */
		    AEXE(i) = 0;
		    /* then print an explanation */
		    switch(i) {
		    case A_STR: You((mod_val >0) ?
				    "must have been exercising." :
				    "must have been abusing your body.");
				break;
		    case A_WIS: You((mod_val >0) ?
				    "must have been very observant." :
				    "haven't been paying attention.");
				break;
		    case A_DEX: You((mod_val >0) ?
				    "must have been working on your reflexes." :
				    "haven't been working on reflexes lately.");
				break;
		    case A_CON: You((mod_val >0) ?
				    "must be leading a healthy life-style." :
				    "haven't been watching your health.");
				break;
		    }
		}
	    }
	    next_check += rn1(200,800);
#ifdef DEBUG
	    pline("exerchk: next check at %ld.", next_check);
#endif
	}
}

/* next_check will otherwise have its initial 600L after a game restore */
void
reset_attribute_clock()
{
	if (moves > 600L) next_check = moves + rn1(50,800);
}

static const struct	clattr *
clx()
{
	register const struct	clattr	*attr;

	switch (u.role) {
	    case 'A':	attr = &a_attr;
			break;
	    case 'B':	attr = &b_attr;
			break;
	    case 'C':	attr = &c_attr;
			break;
	    case 'E':	attr = &e_attr;
			break;
	    case 'H':	attr = &h_attr;
			break;
	    case 'K':	attr = &k_attr;
			break;
	    case 'P':	attr = &p_attr;
			break;
	    case 'R':	attr = &r_attr;
			break;
	    case 'S':	attr = &s_attr;
			break;
#ifdef TOURIST
	    case 'T':	attr = &t_attr;
			break;
#endif
	    case 'V':	attr = &v_attr;
			break;
	    case 'W':	attr = &w_attr;
			break;
	    default:	/* unknown type */
			attr = &X_attr;
			break;
	}
	return(attr);
}

static void
init_align()	/* called from newhp if u.ulevel is 0 */
{
	register const struct	clattr	*attr = clx();

	u.ualign = attr->align;
	/* there should be priests of every stripe */
	if (Role_is('P'))
	     u.ualign.type = (rn2(2)) ? attr->align.type : (rn2(2)) ? 1 : -1;
	else u.ualign.type = attr->align.type;
}

void
init_attr(np)
	register int	np;
{
	register int	i, x, tryct;
	register const struct	clattr	*attr = clx();

	for(i = 0; i < A_MAX; i++) {

	    ABASE(i) = AMAX(i) = attr->base.a[i];
	    ATEMP(i) = ATIME(i) = 0;
	    np -= attr->base.a[i];
	}

	tryct = 0;
	while(np > 0 && tryct < 100) {

	    x = rn2(100);
	    for (i = 0; (i < A_MAX) && ((x -= attr->cldist.a[i]) > 0); i++) ;
	    if(i >= A_MAX) continue; /* impossible */

	    if(ABASE(i) >= ATTRMAX(i)) {

		tryct++;
		continue;
	    }
	    tryct = 0;
	    ABASE(i)++;
	    AMAX(i)++;
	    np--;
	}

	tryct = 0;
	while(np < 0 && tryct < 100) {		/* for redistribution */

	    x = rn2(100);
	    for (i = 0; (i < A_MAX) && ((x -= attr->cldist.a[i]) > 0); i++) ;
	    if(i >= A_MAX) continue; /* impossible */

	    if(ABASE(i) <= ATTRMIN(i)) {

		tryct++;
		continue;
	    }
	    tryct = 0;
	    ABASE(i)--;
	    AMAX(i)--;
	    np++;
	}
}

void
redist_attr()
{
	register int i, tmp;

	for(i = 0; i < A_MAX; i++) {
	    if (i==A_INT || i==A_WIS) continue;
		/* Polymorphing doesn't change your mind */
	    tmp = AMAX(i);
	    AMAX(i) += (rn2(5)-2);
	    if (AMAX(i) > ATTRMAX(i)) AMAX(i) = ATTRMAX(i);
	    if (AMAX(i) < ATTRMIN(i)) AMAX(i) = ATTRMIN(i);
	    ABASE(i) = ABASE(i) * AMAX(i) / tmp;
	    /* ABASE(i) > ATTRMAX(i) is impossible */
	    if (ABASE(i) < ATTRMIN(i)) ABASE(i) = ATTRMIN(i);
	}
	(void)encumber_msg();
}

void
adjabil(oldlevel,newlevel)
int oldlevel, newlevel;
{
	register const struct clattr	*attr = clx();
#ifdef GCC_WARN
	/* this is the "right" definition */
	register const struct innate	*abil = attr->abil;
#else
	/* this one satisfies more compilers */
	register struct innate	*abil = (struct innate *)attr->abil;
#endif

	if(abil) {
	    for(; abil->ability; abil++) {
		if(oldlevel < abil->ulevel && newlevel >= abil->ulevel) {
			/* Abilities gained at level 1 can never be lost
			 * via level loss, only via means that remove _any_
			 * sort of ability.  A "gain" of such an ability from
			 * an outside source is devoid of meaning, so we set
			 * FROMOUTSIDE to avoid such gains.
			 */
			if (abil->ulevel == 1)
				*(abil->ability) |= (FROMEXPER|FROMOUTSIDE);
			else
				*(abil->ability) |= FROMEXPER;
			if(!(*(abil->ability) & FROMOUTSIDE)) {
			    if(*(abil->gainstr))
				You_feel("%s!", abil->gainstr);
			}
		} else if (oldlevel >= abil->ulevel && newlevel < abil->ulevel) {
			*(abil->ability) &= ~FROMEXPER;
			if((*(abil->ability) & INTRINSIC)) {
			    if(*(abil->losestr))
				You_feel("%s!", abil->losestr);
			    else if(*(abil->gainstr))
				You_feel("less %s!", abil->gainstr);
			}
		}
	    }
	}
}

int
newhp()
{
	register const struct clattr	*attr = clx();
	int	hp, conplus;

	if(u.ulevel == 0) {

		hp = attr->shp;
		init_align();	/* initialize alignment stuff */
		return hp;
	} else {

	    if(u.ulevel < attr->xlev)
		hp = rnd(attr->hd);
	    else
		hp = attr->ndx;
	}

	switch(ACURR(A_CON)) {
		case	3:	conplus = -2; break;
		case	4:
		case	5:
		case	6:	conplus = -1; break;
		case	15:
		case	16:	conplus = 1; break;
		case	17:	conplus = 2; break;
		case	18:	conplus = 3; break;
		default:	conplus = 0;
	}
	hp += conplus;
	return((hp <= 0) ? 1 : hp);
}

#endif /* OVLB */
#ifdef OVL0

schar
acurr(x)
int x;
{
	register int tmp = (u.abon.a[x] + u.atemp.a[x] + u.acurr.a[x]);

	if (x == A_STR) {
		if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER) return(125);
#ifdef WIN32_BUG
		else return(x=((tmp >= 125) ? 125 : (tmp <= 3) ? 3 : tmp));
#else
		else return((schar)((tmp >= 125) ? 125 : (tmp <= 3) ? 3 : tmp));
#endif
	} else if (x == A_CHA) {
		if (tmp < 18 && (u.usym == S_NYMPH ||
		    u.umonnum==PM_SUCCUBUS || u.umonnum == PM_INCUBUS))
		    return 18;
	} else if (x == A_INT || x == A_WIS) {
		/* yes, this may raise int/wis if player is sufficiently
		 * stupid.  there are lower levels of cognition than "dunce".
		 */
		if (uarmh && uarmh->otyp == DUNCE_CAP) return(6);
	}
#ifdef WIN32_BUG
	return(x=((tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp));
#else
	return((schar)((tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp));
#endif
}

/* condense clumsy ACURR(A_STR) value into value that fits into game formulas
 */
schar
acurrstr()
{
	register int str = ACURR(A_STR);

	if (str <= 18) return((schar)str);
	if (str <= 121) return((schar)(19 + str / 50)); /* map to 19-21 */
	else return((schar)(str - 100));
}

#endif /* OVL0 */
#ifdef OVL2

/* avoid possible problems with alignment overflow, and provide a centralized
 * location for any future alignment limits
 */
void
adjalign(n)
register int n;
{
	register int newalign = u.ualign.record + n;

	if(n < 0) {
		if(newalign < u.ualign.record)
			u.ualign.record = newalign;
	} else
		if(newalign > u.ualign.record) {
			u.ualign.record = newalign;
			if(u.ualign.record > ALIGNLIM)
				u.ualign.record = ALIGNLIM;
		}
}

#endif /* OVL2 */

/*attrib.c*/
