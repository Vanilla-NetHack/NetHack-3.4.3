/*	SCCS Id: @(#)mcastu.c	3.0	88/04/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

#ifdef OVL0

static void FDECL(cursetxt,(struct monst *));
const char *spelltyp[] = {
	"shower of missiles",
	"fireball",
	"sleep ray",
	"cone of cold",
	"finger of death",
	"bolt of lightning",
	"",
	"",
	"",
	""
};

static
void
cursetxt(mtmp)
	register struct monst *mtmp;
{
	if(canseemon(mtmp)) {
	    if ((Invis && !perceives(mtmp->data) &&
				(mtmp->mux != u.ux || mtmp->muy != u.uy))
#ifdef POLYSELF
			|| u.usym == S_MIMIC_DEF || u.uundetected
#endif
									)
		pline("%s points and curses in your general direction.",
				Monnam(mtmp));
	    else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
		pline("%s points and curses at your displaced image.",
				Monnam(mtmp));
	    else
		pline("%s points at you, then curses.", Monnam(mtmp));
	} else if((!(moves%4) || !rn2(4)) && flags.soundok) 
		You("hear a mumbled curse.");
}

#endif /* OVL0 */
#ifdef OVLB

int
castmu(mtmp, mattk)	/* monster casts spell at you */
	register struct monst *mtmp;
	register struct attack *mattk;
{
	int	dmg = 0, ml = mtmp->m_lev;

	if(mtmp->mcan || mtmp->mspec_used || !ml) {  /* could not attack */
	    cursetxt(mtmp);
	    return(0);
	} else {
	    nomul(0);
	    if(rn2(ml*10) < (mtmp->mconf ? 100 : 20)) {	/* fumbled attack */
		if(canseemon(mtmp)
#ifdef SOUNDS
				&& flags.soundok
#endif
							)
		    pline("The air crackles around %s.", mon_nam(mtmp));
		return(0);
	    }
	}
/*
 *	As these are spells, the damage is related to the level
 *	of the monster casting the spell.
 */
	if (mattk->damd)
		dmg = d((int)((ml/3) + mattk->damn), (int)mattk->damd);
	else dmg = d((int)((ml/3) + 1), 6);

	switch(mattk->adtyp)   {

	    case AD_FIRE:
		pline("You're enveloped in flames.");
		if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
		}
		break;
	    case AD_COLD:
		pline("You're covered in frost.");
		if(Cold_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
		}
		break;
#ifdef INFERNO
	    case AD_MAGM:
		You("are hit by a shower of missiles!");
		if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline("The missiles bounce off!");
			dmg = 0;
		} else dmg = d((int)mtmp->m_lev/2 + 1,6);
		break;
#endif
	    case AD_SPEL:	/* random spell */

		mtmp->mspec_used = 10 - mtmp->m_lev;
		if (mtmp->mspec_used < 2) mtmp->mspec_used = 2;
		switch(rn2((int)mtmp->m_lev)) {
		    case 22:
		    case 21:
		    case 20:
			pline("Oh no, %s's using the touch of death!",
			      is_female(mtmp) ? "she" :
			      is_human(mtmp->data) ? "he" : "it"
			     );
#ifdef POLYSELF
			if (is_undead(uasmon))
			    You("seem no deader than before.");
			else
#endif
			if(!Antimagic && rn2(ml) > 12)  {

			    if(Hallucination)
				You("have an out of body experience.");
			    else  {
				killer_format = KILLED_BY_AN;
				killer = "touch of death";
				done(DIED);
			    }
			} else {
				if(Antimagic) shieldeff(u.ux, u.uy);
				pline("Lucky for you, it didn't work!");
			}
			dmg = 0;
			break;
		    case 19:
		    case 18:
			if(mtmp->iswiz && flags.no_of_wizards == 1) {
				pline("Double Trouble...");
				clonewiz();
				dmg = 0;
				break;
			} /* else fall into the next case */
		    case 17:
		    case 16:
		    case 15:
			if(mtmp->iswiz && flags.soundok)
			    pline("\"Destroy the thief, my pets!\"");
#ifdef HARD
			nasty();	/* summon something nasty */
#endif
			/* fall into the next case */
		    case 14:		/* aggravate all monsters */
		    case 13:
			aggravate();
			dmg = 0;
			break;
		    case 12:		/* curse random items */
		    case 11:
		    case 10:
			rndcurse();
			dmg = 0;
			break;
		    case 9:
		    case 8:		/* destroy armor */
			if(Antimagic) {
				shieldeff(u.ux, u.uy);
				pline("A field of force surrounds you!");
			} else if(!destroy_arm(some_armor()))
				Your("skin itches.");
			dmg = 0;
			break;
		    case 7:
		    case 6:		/* drain strength */
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    You("feel momentarily weakened.");
			} else {
			    You("suddenly feel weaker!");
			    losestr(rnd(ml - 6));
			    if(u.uhp < 1)
				done_in_by(mtmp);
			}
			dmg = 0;
			break;
		    case 5:		/* make invisible if not */
		    case 4:
			if(!mtmp->minvis) {
			    if(canseemon(mtmp) && !See_invisible)
				pline("%s suddenly disappears!",
				      Monnam(mtmp));
			    mtmp->minvis = 1;
			    unpmon(mtmp);
			    dmg = 0;
			    break;
			} /* else fall into the next case */
		    case 3:		/* stun */
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    if(!Stunned)
				You("feel momentarily disoriented.");
			    make_stunned(1L, FALSE);
			} else {
			    if (Stunned)	You("struggle to keep your balance.");
			    else		You("reel...");
			    make_stunned(HStun + d(ACURR(A_DEX) < 12 ? 6 : 4, 4), FALSE);
			}
			dmg = 0;
			break;
		    case 2:		/* haste self */
			if(mtmp->mspeed == MSLOW)	mtmp->mspeed = 0;
			else				mtmp->mspeed = MFAST;
			dmg = 0;
			break;
		    case 1:		/* cure self */
			if(mtmp->mhp < mtmp->mhpmax) {
			    if((mtmp->mhp += rnd(8)) > mtmp->mhpmax)
				mtmp->mhp = mtmp->mhpmax;
			    dmg = 0;
			    break;
			} /* else fall through to default case */
		    default:		/* psi bolt */
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    You("get a slight %sache.",body_part(HEAD));
			    dmg = 1;
			} else {
			    if (dmg <= 10)
				Your("brain is on fire!");
			    else Your("%s suddenly aches!", body_part(HEAD));
			}
			break;
		}
		break;
		
	    case AD_CLRC:	/* clerical spell */

		mtmp->mspec_used = 10 - mtmp->m_lev;
		if (mtmp->mspec_used < 2) mtmp->mspec_used = 2;
		switch(rn2((int)mtmp->m_lev)) {
		    /* Other ideas: lightning bolts, towers of flame,
				    gush of water  -3. */

		    default:		/* confuse */
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    You("feel momentarily dizzy.");
			} else {
			    make_confused(HConfusion + (long)mtmp->m_lev, TRUE);
			}
			dmg = 0;
			break;
		    case 12:		/* curse random items */
		    case 11:
		    case 10:
			rndcurse();
			dmg = 0;
			break;
		    case 9:
		    case 8:		/* insects */
			/* Try for insects, and if there are none
			   left, go for (sticks to) snakes.  -3. */
			{
			int i;
			struct permonst *pm = mkclass(S_ANT);
			struct monst *mtmp2;
			char let = (pm ? S_ANT : S_SNAKE);
  
			for (i = 0; i <= mtmp->m_lev; i++)
			   if ((pm = mkclass(let)) &&
					(mtmp2 = makemon(pm, u.ux, u.uy)))
				mtmp2->msleep = mtmp2->mpeaceful =
					mtmp2->mtame = 0;
			}			
			dmg = 0;
			break;
		    case 6:
		    case 7:		/* blindness */
			if (!Blinded) {
			    pline("Scales cover your eyes!");
			    make_blinded(200L, FALSE);
			    dmg = 0;
			    break;
			}
		    case 4:
		    case 5:		/* wound */
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    Your("skin itches badly for a moment.");
			    dmg = 0;
			} else {
			    pline("Wounds appear on your body!");
			    dmg = d(2,8) + 1;
			}
			break;
		    case 3:		/* hold */
			if(Antimagic) {
			    shieldeff(u.ux, u.uy);
			    if(multi >= 0)
				You("stiffen briefly.");
			    nomul(-1);
		 	} else {
			    if (multi >= 0)	
			        You("are frozen in place!");
			    nomul(-4 - (int)mtmp->m_lev);
			}
			dmg = 0;
			break;
		    case 2:
		    case 1:		/* cure self */
			if(mtmp->mhp < mtmp->mhpmax) {
			    if((mtmp->mhp += rnd(8)) > mtmp->mhpmax)
				mtmp->mhp = mtmp->mhpmax;
			    dmg = 0;
			    break;
			} /* else fall through to default case */
		}
	}
	if(dmg) mdamageu(mtmp, dmg);
	return(1);
}

#endif /* OVLB */
#ifdef OVL0

int
buzzmu(mtmp, mattk)		/* monster uses spell (ranged) */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	if(mtmp->mcan || mattk->adtyp > AD_SPC2) {
	    cursetxt(mtmp);
	    return(0);
	}
	if(lined_up(mtmp) && rn2(3)) {
	    nomul(0);
	    if(mattk->adtyp && (mattk->adtyp < 11)) { /* no cf unsigned >0 */
		if(canseemon(mtmp))
		    pline("%s zaps you with a %s!", Monnam(mtmp),
			  spelltyp[mattk->adtyp-1]);
		buzz((int) (-10 - (mattk->adtyp-1)), (int)mattk->damn,
		     mtmp->mx, mtmp->my, sgn(tbx), sgn(tby));
	    } else impossible("Monster spell %d cast", mattk->adtyp-1);
	}
	return(1);
}

#endif /* OVL0 */

