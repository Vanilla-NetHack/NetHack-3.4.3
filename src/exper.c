/*	SCCS Id: @(#)exper.c	3.2	96/06/16	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static long FDECL(newuexp, (int));
static int FDECL(enermod, (int));

static long
newuexp(lev)
int lev;
{
	if (lev < 10) return (10L * (1L << lev));
	if (lev < 20) return (10000L * (1L << (lev - 10)));
	return (10000000L * ((long)(lev - 19)));
}

static int
enermod(en)
int en;
{
	if(Role_is('W') || Role_is('P')) return(2 * en);
	else if(Role_is('H') || Role_is('K')) return((3 * en) / 2);
	else if(Role_is('B') || Role_is('V')) return((3 * en) / 4);
	else return(en);
}

int
experience(mtmp, nk)	/* return # of exp points for mtmp after nk killed */
	register struct	monst *mtmp;
	register int	nk;
#if defined(applec)
# pragma unused(nk)
#endif
{
	register struct permonst *ptr = mtmp->data;
	int	i, tmp, tmp2;

	tmp = 1 + mtmp->m_lev * mtmp->m_lev;

/*	For higher ac values, give extra experience */
	if((i = find_mac(mtmp)) < 3) tmp += (7 - i) * (i < 0) ? 2 : 1;

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
	    if((int)(ptr->mattk[i].damd * ptr->mattk[i].damn) > 23)
		tmp += mtmp->m_lev;
	}

/*	For certain "extra nasty" monsters, give even more */
	if (extra_nasty(ptr)) tmp += (7 * mtmp->m_lev);
	if (ptr->mlet == S_EEL && !Amphibious) tmp += 1000;

/*	For higher level monsters, an additional bonus is given */
	if(mtmp->m_lev > 8) tmp += 50;

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
	if(exp
#ifdef SCORE_ON_BOTL
	   || flags.showscore
#endif
	   ) flags.botl = 1;
	if (u.urexp >= (Role_is('W') ? 1000 : 2000))
		flags.beginner = 0;
}

void
losexp()		/* hit by drain life attack */
{
	register int num;

	if (resists_drli(&youmonst)) return;

	if(u.ulevel > 1) {
		pline("Goodbye level %d.", u.ulevel--);
		/* remove intrinsic abilities */
		adjabil(u.ulevel + 1, u.ulevel);
		reset_rndmonst(NON_PM);	/* new monster selection */
	} else
		u.uhp = -1;
	num = newhp();
	u.uhp -= num;
	u.uhpmax -= num;
	num = enermod(rn1(u.ulevel/2 + 1, 2));		/* M. Stephenson */
	u.uen -= num;
	if (u.uen < 0)		u.uen = 0;
	u.uenmax -= num;
	if (u.uenmax < 0)	u.uenmax = 0;
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
newexplevel()
{
	register int tmp;

	if(u.ulevel < MAXULEV && u.uexp >= newuexp(u.ulevel)) {

		u.ulevel++;
		if (u.uexp >= newuexp(u.ulevel)) u.uexp = newuexp(u.ulevel) - 1;
		pline("Welcome to experience level %d.", u.ulevel);
		/* give new intrinsics */
		adjabil(u.ulevel - 1, u.ulevel);
		reset_rndmonst(NON_PM);	/* new monster selection */
		tmp = newhp();
		u.uhpmax += tmp;
		u.uhp += tmp;
		tmp = enermod(rn1((int)ACURR(A_WIS)/2+1, 2)); /* M. Stephenson */
		u.uenmax += tmp;
		u.uen += tmp;
		flags.botl = 1;
	}
}

void
pluslvl()
{
	register int num;

	You_feel("more experienced.");
	num = newhp();
	u.uhpmax += num;
	u.uhp += num;
	num = enermod(rn1((int)ACURR(A_WIS)/2+1, 2));	/* M. Stephenson */
	u.uenmax += num;
	u.uen += num;
	if(u.ulevel < MAXULEV) {
		u.uexp = newuexp(u.ulevel);
		pline("Welcome to experience level %d.", ++u.ulevel);
		adjabil(u.ulevel - 1, u.ulevel);
		reset_rndmonst(NON_PM);	/* new monster selection */
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

/*exper.c*/
