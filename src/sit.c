/*	SCCS Id: @(#)sit.c	3.0	89/06/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#if defined(THRONES) || defined(SPELLS)
void
take_gold()
{
	if (u.ugold <= 0)  {
		You("feel a strange sensation.");
	} else {
		You("notice you have no gold!");
		u.ugold = 0;
		flags.botl = 1;
	}
}
#endif

int
dosit() {
#ifdef THRONES
	register int cnt;
#endif

	if(Levitation)  {
	    pline("There's nothing to sit on up here.");
#ifdef THRONES
	} else  if(IS_THRONE(levl[u.ux][u.uy].typ)) {

	    if (rnd(6) > 4)  {
		switch (rnd(13))  {
		    case 1:
			adjattrib(rn2(A_MAX), -rn1(4,3), FALSE);
			losehp(rnd(10), "cursed throne", KILLED_BY_AN);
			break;
		    case 2:
			adjattrib(rn2(A_MAX), 1, FALSE);
			break;
		    case 3:
		pline("A%s charge of electricity shoots through your body!",
			      (Shock_resistance) ? "" : " massive");
			if(Shock_resistance)
				losehp(rnd(6), "electric chair", KILLED_BY_AN);
			else	losehp(rnd(30), "electric chair", KILLED_BY_AN);
			break;
		    case 4:
			You("feel much, much better!");
			if(u.uhp >= (u.uhpmax - 5))  u.uhpmax += 4;
			u.uhp = u.uhpmax;
			make_blinded(0L,TRUE);
			make_sick(0L,FALSE);
			heal_legs();
			flags.botl = 1;
			break;
		    case 5:
			take_gold();
			break;
		    case 6:
			if(u.uluck + rn2(5) < 0) {
			    You("feel your luck is changing.");
			    change_luck(1);
			} else	    makewish();
			break;
		    case 7:
			cnt = rnd(10);
			You("hear a voice echo:");
			pline("\"Thy audience hath been summoned, Sire!\"");
			while(cnt--)
			    (void) makemon(courtmon(), u.ux, u.uy);
			break;
		    case 8:
			You("hear a voice echo:");
			pline("\"By thy Imperious order, Sire...\"");
			do_genocide(1);
			break;
		    case 9:
			You("hear a voice echo:");
	pline("\"A curse upon thee for sitting upon this most holy throne!\"");
			if (Luck > 0)  {
			    make_blinded(Blinded + rn1(100,250),TRUE);
			} else	    rndcurse();
			break;
		    case 10:
			if (Luck < 0 || (HSee_invisible & INTRINSIC))  {
				pline("An image forms in your mind.");
				do_mapping();
			} else  {
				Your("vision clarifies.");
				HSee_invisible |= INTRINSIC;
			}
			break;
		    case 11:
			if (Luck < 0)  {
			    You("feel threatened.");
			    aggravate();
			} else  {

			    You("feel a wrenching sensation.");
			    tele();		/* teleport him */
			}
			break;
		    case 12:
			You("are granted a gift of insight!");
			while (!ggetobj("identify", identify, rn2(5))
				&& invent);
			break;
		    case 13:
			Your("mind turns into a pretzel!");
			make_confused(HConfusion + rn1(7,16),FALSE);
			break;
		    default:	impossible("throne effect");
				break;
		}
	    } else	You("feel somehow out of place...");

	    if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ))	{
		pline("The throne vanishes in a puff of logic.");
/*		levl[u.ux][u.uy].scrsym = ROOM_SYM; */
		levl[u.ux][u.uy].typ = ROOM;
		if(Invisible) newsym(u.ux,u.uy);
	    }
#endif
#ifdef POLYSELF
	} else if (lays_eggs(uasmon) || u.umonnum == PM_QUEEN_BEE) {
		struct obj *uegg;

		if (u.uhunger < objects[EGG].nutrition) {
			You("are too weak to lay an egg.");
			return 0;
		}

		uegg = mksobj(EGG, 0);
		uegg->spe = 1;
		uegg->quan = 1;
		uegg->owt = weight(uegg);
		uegg->corpsenm =
		    (u.umonnum==PM_QUEEN_BEE ? PM_KILLER_BEE : monsndx(uasmon));
		uegg->known = uegg->dknown = 1;
		You("lay an egg.");
		dropy(uegg);
		stackobj(uegg);
		morehungry(objects[EGG].nutrition);
#endif
	} else
		pline("Having fun sitting on the floor?");
	return(1);
}

void
rndcurse() {			/* curse a few inventory items at random! */

	int	nobj = 0;
	int	cnt, onum;
	struct	obj	*otmp;

	if(Antimagic) {
	    shieldeff(u.ux, u.uy);
	    You("feel a malignant aura surround you.");
	}

	for (otmp = invent; otmp; otmp = otmp->nobj)  nobj++;
	    if (nobj) for (cnt = rnd(6/((!!Antimagic) + 1)); cnt > 0; cnt--)  {

		onum = rn2(nobj);
		for(otmp = invent; onum != 0; onum--)
		    otmp = otmp->nobj;
		if(otmp->blessed)
			otmp->blessed = 0;
		else
			otmp->cursed++;
	    }
}

void
attrcurse() {			/* remove a random INTRINSIC ability */
	switch(rnd(10)) {
	case 1 : if (HFire_resistance & INTRINSIC) {
			HFire_resistance &= ~INTRINSIC;
			if (Inhell && !Fire_resistance) {
			    You("burn to a crisp.");
			    killer_format = NO_KILLER_PREFIX;
			    killer = self_pronoun("a gremlin stole %s fire resistance in hell", "his");
			    done(BURNING);
			} else You("feel warmer.");
			break;
		}
	case 2 : if (HTeleportation & INTRINSIC) {
			HTeleportation &= ~INTRINSIC;
			You("feel less jumpy.");
			break;
		}
	case 3 : if (HPoison_resistance & INTRINSIC) {
			HPoison_resistance &= ~INTRINSIC;
			You("feel a little sick!");
			break;
		}
	case 4 : if (HTelepat & INTRINSIC) {
			HTelepat &= ~INTRINSIC;
			Your("senses fail!");
			break;
		}
	case 5 : if (HCold_resistance & INTRINSIC) {
			HCold_resistance &= ~INTRINSIC;
			You("feel cooler.");
			break;
		}
	case 6 : if (HInvis & INTRINSIC) {
			HInvis &= ~INTRINSIC;
			You("feel paranoid.");
			break;
		}
	case 7 : if (HSee_invisible & INTRINSIC) {
			HSee_invisible &= ~INTRINSIC;
			You("thought you saw something!");
			break;
		}
	case 8 : if (Fast & INTRINSIC) {
			Fast &= ~INTRINSIC;
			You("feel slower.");
			break;
		}
	case 9 : if (Stealth & INTRINSIC) {
			Stealth &= ~INTRINSIC;
			You("feel clumsy.");
			break;
		}
	case 10: if (Protection & INTRINSIC) {
			Protection &= ~INTRINSIC;
			You("feel vulnerable.");
			break;
		}
	default: break;
	}
}
