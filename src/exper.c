/*	SCCS Id: @(#)exper.c	3.0	89/11/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef LINT
#define	NEW_SCORING
#endif
long
newuexp(lev)
register unsigned lev;
{
#ifdef LINT	/* long conversion */
	return(0L * lev);
#else
	if(lev < 10) return (10L*(1L << lev));
	if(lev < 20) return (10000L*(1L << (lev-10)));
	return (10000000L*(lev-19));
#endif
}

int
experience(mtmp, nk)	/* return # of exp points for mtmp after nk killed */
	register struct	monst *mtmp;
	register int	nk;
{
	register struct permonst *ptr = mtmp->data;
	int	i, tmp, tmp2;

	tmp = 1 + mtmp->m_lev * mtmp->m_lev;

/*	For higher ac values, give extra experience */
	if(ptr->ac < 3) tmp += (7 - ptr->ac) * (ptr->ac < 0) ? 2 : 1;

/*	For very fast monsters, give extra experience */
	if(ptr->mmove >= 12) tmp += (ptr->mmove >= 18) ? 5 : 3;

/*	For each "special" attack type give extra experience */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].aatyp;
	    if(tmp2 > AT_BUTT) {

		if(tmp2 == AT_WEAP) tmp += 5;
		else if(tmp2 == AT_MAGC) tmp += 10;
		else tmp += 3;
	    }
	}

/*	For each "special" damage type give extra experience */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].adtyp;
	    if(tmp2 > AD_PHYS && tmp2 < AD_BLND) tmp += 2*mtmp->m_lev;
	    else if((tmp2 == AD_DRLI) || (tmp2 == AD_STON)) tmp += 50;
	    else if(tmp != AD_PHYS) tmp += mtmp->m_lev;
		/* extra heavy damage bonus */
	    if((ptr->mattk[i].damd * ptr->mattk[i].damn) > 23)
		tmp += mtmp->m_lev;
	}

/*	For certain "extra nasty" monsters, give even more */
	if(extra_nasty(ptr)) tmp += (7*mtmp->m_lev);
	if(ptr->mlet == S_EEL) tmp += 1000;

/*	For higher level monsters, an additional bonus is given */
	if(mtmp->m_lev > 8) tmp += 50;

#ifdef NEW_SCORING
	/* ------- recent addition: make nr of points decrease
		   when this is not the first of this kind */
	{ unsigned ul = u.ulevel;
	  int ml = mtmp->m_lev;
	/* points are given based on present and future level */
	  if(ul < MAXULEV)
	    for(tmp2 = 0; !tmp2 || ul + tmp2 <= ml; tmp2++)
		if(u.uexp + 1 + (tmp + ((tmp2 <= 0) ? 0 : 4<<(tmp2-1)))/nk
		    >= newuexp(ul) )
			if(++ul == MAXULEV) break;

	  tmp2 = ml - ul -1;
	  tmp = (tmp + ((tmp2 < 0) ? 0 : 4<<tmp2))/nk;
	  if(tmp <= 0) tmp = 1;
	}
	/* note: ul is not necessarily the future value of u.ulevel */
	/* ------- end of recent valuation change ------- */
#endif /* NEW_SCORING /**/

#ifdef MAIL
	/* Mail daemons put up no fight. */
	if(mtmp->data == &mons[PM_MAIL_DAEMON]) tmp = 1;
#endif

	return(tmp);
}

void
more_experienced(exp, rexp)
	register int exp, rexp;
{
	u.uexp += exp;
	u.urexp += 4*exp + rexp;
	if(exp) flags.botl = 1;
	if(u.urexp >= ((pl_character[0] == 'W') ? 1000 : 2000))
		flags.beginner = 0;
}

void
losexp() {	/* hit by drain life attack */

	register int num;

#ifdef POLYSELF
	if(resists_drli(uasmon)) return;
#endif

	if(u.ulevel > 1) {
		pline("Goodbye level %u.", u.ulevel--);
		adjabil(-1);	/* remove intrinsic abilities */
	} else
		u.uhp = -1;
	num = newhp();
	u.uhp -= num;
	u.uhpmax -= num;
#ifdef SPELLS
	num = rnd((int)u.ulevel/2+1) + 1;		/* M. Stephenson */
	u.uen -= num;
	if (u.uen < 0)		u.uen = 0;
	u.uenmax -= num;
	if (u.uenmax < 0)	u.uenmax = 0;
#endif
	u.uexp = newuexp(u.ulevel) - 1;
	flags.botl = 1;
}

/*
 * Make experience gaining similar to AD&D(tm), whereby you can at most go
 * up by one level at a time, extra expr possibly helping you along.
 * After all, how much real experience does one get shooting a wand of death
 * at a dragon created with a wand of polymorph??
 */
void
newexplevel() {

	register int tmp;

	if(u.ulevel < MAXULEV && u.uexp >= newuexp(u.ulevel)) {

		u.ulevel++;
		if (u.uexp >= newuexp(u.ulevel)) u.uexp = newuexp(u.ulevel) - 1;
		pline("Welcome to experience level %u.", u.ulevel);
		set_uasmon();	/* set up for the new level. */
		adjabil(1);	/* give new intrinsic abilities */
		tmp = newhp();
		u.uhpmax += tmp;
		u.uhp += tmp;
#ifdef SPELLS
		tmp = rnd((int)ACURR(A_WIS)/2+1) + 1; /* M. Stephenson */
		u.uenmax += tmp;
		u.uen += tmp;
#endif
		flags.botl = 1;
	}
}

void
pluslvl() {
	register int num;

	You("feel more experienced.");
	num = newhp();
	u.uhpmax += num;
	u.uhp += num;
#ifdef SPELLS
	num = rnd((int)ACURR(A_WIS)/2+1) + 1;	/* M. Stephenson */
	u.uenmax += num;
	u.uen += num;
#endif
	if(u.ulevel < MAXULEV) {
		u.uexp = newuexp(u.ulevel);
		pline("Welcome to experience level %u.", ++u.ulevel);
		adjabil(1);
	}
	flags.botl = 1;
}

long
rndexp()
{
	register long minexp,maxexp;

	if(u.ulevel == 1)
		return rn2((int)newuexp(1));
	else {
		minexp = newuexp(u.ulevel - 1);
		maxexp = newuexp(u.ulevel);
		return(minexp + rn2((int)(maxexp - minexp)));
	}
}
