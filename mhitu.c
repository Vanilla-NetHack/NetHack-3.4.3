/*	SCCS Id: @(#)mhitu.c	2.1	87/10/18
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	"hack.h"
extern struct monst *makemon();
extern struct obj *carrying();
#ifdef KAA
extern char pl_character[];
#endif

/*
 * mhitu: monster hits you
 *	  returns 1 if monster dies (e.g. 'y', 'F'), 0 otherwise
 */
mhitu(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat = mtmp->data;
	register int tmp, ctmp;

	nomul(0);

	/* If swallowed, can only be affected by hissers and by u.ustuck */
	if(u.uswallow) {
		if(mtmp != u.ustuck) {
			if(mdat->mlet == 'c' && !rn2(13)) {
				pline("Outside, you hear %s's hissing!",
					monnam(mtmp));
				pline("%s gets turned to stone!",
					Monnam(u.ustuck));
				pline("And the same fate befalls you.");
				done_in_by(mtmp);
				/* "notreached": not return(1); */
			}
			return(0);
		}
		switch(mdat->mlet) {	/* now mtmp == u.ustuck */
		case ',':
			youswld(mtmp, (u.uac > 0) ? u.uac+4 : 4,
				5, Monnam(mtmp));
			break;
		case '\'':
			youswld(mtmp,rnd(6),7,Monnam(mtmp));
			break;
		case 'P':
			youswld(mtmp,d(2,4),12,Monnam(mtmp));
			break;
		default:
			/* This is not impossible! */
#ifdef DGKMOD
			/* If the swallowing monster changes into a monster
			 * that is not capable of swallowing you, you get
			 * regurgitated - dgk
			 */
			pline("You get regurgitated!");
			u.ux = mtmp->mx;
			u.uy = mtmp->my;
			u.uswallow = 0;
			u.ustuck = 0;
			mnexto(mtmp);
			setsee();
			docrt();
			break;
#else
			pline("The mysterious monster totally digests you.");
			u.uhp = 0;
#endif /* DGKMOD /**/
		}
		if(u.uhp < 1) done_in_by(mtmp);
		return(0);
	}

	if(mdat->mlet == 'c' && Stoned)
		return(0);

	/* make eels visible the moment they hit/miss us */
	if(mdat->mlet == ';' && mtmp->minvis && cansee(mtmp->mx,mtmp->my)){
		mtmp->minvis = 0;
		pmon(mtmp);
	}
	if(!index("1&DuxynNF",mdat->mlet))
		tmp = hitu(mtmp,d(mdat->damn,mdat->damd));
	else
		tmp = 0;
	if(index(UNDEAD, mdat->mlet) && midnight())
		tmp += hitu(mtmp,d(mdat->damn,mdat->damd));

	ctmp = tmp && !mtmp->mcan &&
	  (!uarm || objects[uarm->otyp].a_can < rnd(3) || !rn2(50));
	switch(mdat->mlet) {
	case '1':
		if(wiz_hit(mtmp)) return(1);	/* he disappeared */
		break;
	case '&':
		demon_hit(mtmp);
		break;
	case ',':
		if(tmp) justswld(mtmp,Monnam(mtmp));
		break;
	case '\'':
		if (tmp) justswld(mtmp,Monnam(mtmp));
		break;
	case ';':
		if(ctmp) {
			if(!u.ustuck && !rn2(10)) {
				pline("%s swings itself around you!",
					Monnam(mtmp));
				u.ustuck = mtmp;
			} else if(u.ustuck == mtmp &&
			    levl[mtmp->mx][mtmp->my].typ == POOL) {
				pline("%s drowns you ...", Monnam(mtmp));
				done("drowned");
			}
		}
		break;
	case 'A':
		if(ctmp && rn2(2)) {
		    if(Poison_resistance)
			pline("The sting doesn't seem to affect you.");
		    else {
			pline("You feel weaker!");
			losestr(1);
		    }
		}
		break;
	case 'C':
		(void) hitu(mtmp,rnd(6));
		break;
	case 'c':
		if(!rn2(5)) {
		    if (mtmp->mcan)
			pline("You hear a cough from %s!", monnam(mtmp));
		    else {
			pline("You hear %s's hissing!", monnam(mtmp));
			if(!rn2(20) || (flags.moonphase == NEW_MOON
			    && !carrying(DEAD_LIZARD) && u.usym != 'c')) {
				Stoned = 5;
				/* pline("You get turned to stone!"); */
				/* done_in_by(mtmp); */
			}
		    }
		}
		break;
	case 'D':
		if(rn2(6) || mtmp->mcan) {
			(void) hitu(mtmp,d(3,10));
			(void) hitu(mtmp,rnd(8));
			(void) hitu(mtmp,rnd(8));
			break;
		}
		kludge("%s breathes fire!",Monnam(mtmp));
		buzz(-1,mtmp->mx,mtmp->my,u.ux-mtmp->mx,u.uy-mtmp->my);
		break;
	case 'd':
		(void) hitu(mtmp,d(2, (flags.moonphase == FULL_MOON) ? 3 : 4));
		break;
	case 'e':
		(void) hitu(mtmp,d(3,6));
		break;
	case 'F':
		if(mtmp->mcan) break;
		kludge("%s explodes!", Monnam(mtmp));
		if(Cold_resistance) pline("You don't seem affected by it.");
		else {
			xchar dn;
			if(17-(u.ulevel/2) > rnd(20)) {
				pline("You get blasted!");
				dn = 6;
			} else {
				pline("You duck the blast...");
				dn = 3;
			}
			losehp_m(d(dn,6), mtmp);
		}
		mondead(mtmp);
		return(1);
	case 'g':
		if(ctmp && multi >= 0 && !rn2(3)) {
		/* fix so we don't know what hit us when blind  KAA */
		    if (Blind)
			pline("You are frozen by its juices!");
		    else
			pline("You are frozen by %s's juices!",monnam(mtmp));
		    nomul(-rnd(10));
		}
		break;
	case 'h':
		if(ctmp && multi >= 0 && !rn2(5)) {
		    nomul(-rnd(10));
		    if (Blind)
			pline("You are put to sleep by its bite!");
		    else
			pline("You are put to sleep by %s's bite!",monnam(mtmp));
		}
		break;
	case 'j':
		tmp = hitu(mtmp,rnd(3));
		tmp &= hitu(mtmp,rnd(3));
		if(tmp){
			(void) hitu(mtmp,rnd(4));
			(void) hitu(mtmp,rnd(4));
		}
		break;
	case 'k':
		if((hitu(mtmp,rnd(4)) || !rn2(3)) && ctmp){
			poisoned("bee's sting",mdat->mname);
		}
		break;
	case 'L':
#ifdef KAA
		if (u.usym=='L') break;
#endif
		if(!mtmp->mcan && tmp) stealgold(mtmp);
		break;
	case 'N':
#ifdef KAA
		if (u.usym=='N') {
			if (mtmp->minvent)
	pline("%s brags about the goods some dungeon explorer provided.",
	Monnam(mtmp));
			else
	pline("%s makes some remarks about how difficult theft is lately.",
	Monnam(mtmp));
			rloc(mtmp);
		} else
#endif
		if(mtmp->mcan && !Blind) {
		pline("%s tries to seduce you, but you seem not interested.",
			Amonnam(mtmp, "plain"));
			if(rn2(3)) rloc(mtmp);
		} else if(steal(mtmp)) {
			rloc(mtmp);
			mtmp->mflee = 1;
		}
		break;
	case 'n':
		if(!uwep
#ifdef KAA
		   && u.usym == '@'
#endif
		   && !uarm && !uarmh && !uarms && !uarmg) {
		    pline("%s hits! (I hope you don't mind)",
			Monnam(mtmp));
			u.uhp += rnd(7);
			if(!rn2(7)) u.uhpmax++;
			if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
			flags.botl = 1;
			if(!rn2(50)) rloc(mtmp);
		} else {
#ifdef KAA
			if (pl_character[0] == 'H' && u.usym == '@') {
			    if (!(moves % 5))
				pline("Doc, I can't help you unless you cooperate.");
			} else {
#endif
				(void) hitu(mtmp,d(2,6));
				(void) hitu(mtmp,d(2,6));
#ifdef KAA
			}
#endif
		}
		break;
	case 'o':
		tmp = hitu(mtmp,rnd(6));
		if(hitu(mtmp,rnd(6)) && tmp &&	/* hits with both paws */
		    !u.ustuck && rn2(2)) {
			u.ustuck = mtmp;
			kludge("%s has grabbed you!", Monnam(mtmp));
			losehp_m(d(2,8), mtmp);
		} else if(u.ustuck == mtmp) {
			losehp_m(d(2,8), mtmp);
			pline("You are being crushed.");
		}
		break;
	case 'P':
		if(ctmp && !rn2(4))
			justswld(mtmp,Monnam(mtmp));
		else
			(void) hitu(mtmp,d(2,4));
		break;
	case 'Q':
#ifdef KAA
		if(ctmp) {
			pline("Your position suddenly seems very uncertain!");
			tele();
		}
#else
		(void) hitu(mtmp,rnd(2));
		(void) hitu(mtmp,rnd(2));
#endif
		break;
	case 'R':
		if(ctmp && uarmh && !uarmh->rustfree &&
		   (int) uarmh->spe >= -1) {
			pline("Your helmet rusts!");
			uarmh->spe--;
		} else
		if(ctmp && uarm && !uarm->rustfree &&	/* Mike Newton */
		 uarm->otyp < STUDDED_LEATHER_ARMOR &&
		 (int) uarm->spe >= -1) {
			pline("Your armor rusts!");
			uarm->spe--;
		}
		break;
	case 'S':
		if(ctmp && !rn2(8)) {
			poisoned("snake's bite",mdat->mname);
		}
		break;
	case 's':
		if(ctmp && !rn2(8)) {
#ifdef SPIDERS
			poisoned("giant spider's bite",mdat->mname);
#else
			poisoned("scorpion's sting",mdat->mname);
#endif
		}
		(void) hitu(mtmp,rnd(8));
		(void) hitu(mtmp,rnd(8));
		break;
	case 'T':
		(void) hitu(mtmp,rnd(6));
		(void) hitu(mtmp,rnd(6));
		break;
	case 't':
		if(!rn2(5)) rloc(mtmp);
		break;
	case 'u':
		mtmp->mflee = 1;
		break;
	case 'U':
		(void) hitu(mtmp,d(3,4));
		(void) hitu(mtmp,d(3,4));
		break;
	case 'v':
		if(ctmp && !u.ustuck) u.ustuck = mtmp;
		break;
	case 'V':
		if(tmp)  losehp_m(4, mtmp);
		if(ctmp) losexp();
		break;
	case 'W':
		if(ctmp) losexp();
		break;
#ifndef NOWORM
	case 'w':
		if(tmp) wormhit(mtmp);
#endif
		break;
	case 'X':
		(void) hitu(mtmp,rnd(5));
		(void) hitu(mtmp,rnd(5));
		(void) hitu(mtmp,rnd(5));
		break;
	case 'x':
		{ register long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
#ifdef KAA
		  if (mtmp->mcan)
		    pline("%s nuzzles against your %s leg!",
			  Monnam(mtmp), (side==RIGHT_SIDE)?"right":"left");
		  else {
#endif
		    pline("%s pricks your %s leg!",
			  Monnam(mtmp), (side==RIGHT_SIDE)?"right":"left");
		    set_wounded_legs(side, rnd(50));
		    losehp_m(2, mtmp);
#ifdef KAA
		  }
#endif
		  break;
		}
	case 'y':
		if(mtmp->mcan) break;
		mondead(mtmp);
		if(!Blind && (u.usym != 'y')) {
			pline("You are blinded by a blast of light!");
			Blinded = d(4,12);
			seeoff(0);
		}
		return(1);
	case 'Y':
		(void) hitu(mtmp,rnd(6));
		break;
#ifdef RPH
	case '8':
		if (canseemon(mtmp) && !mtmp->mcan) {

		        pline ("You look upon %s.", monnam(mtmp));
			pline ("You turn to stone.");
			done_in_by(mtmp);
	    	}
		(void) hitu(mtmp,d(2,6));
		(void) hitu(mtmp,d(2,6));
		break;
#endif
	}
	if(u.uhp < 1) done_in_by(mtmp);
	return(0);
}

hitu(mtmp,dam)
register struct monst *mtmp;
register dam;
{
	register tmp, res;

	nomul(0);
	if (mtmp->mfroz || mtmp->mhp <= 0) return(0);
	/* If you are a 'a' or 'E' the monster might not get a second hit */
	if(u.uswallow) return(0);

	if(mtmp->mhide && mtmp->mundetected) {
		mtmp->mundetected = 0;
		if(!Blind) {
			register struct obj *obj;
			extern char * Xmonnam();
			if(obj = o_at(mtmp->mx,mtmp->my))
				pline("%s was hidden under %s!",
					Xmonnam(mtmp), doname(obj));
		}
	}

	tmp = u.uac;
	/* give people with Ac = -10 at least some vulnerability */
	if(tmp < 0) {
		dam += tmp;		/* decrease damage */
		if(dam <= 0) dam = 1;
		tmp = -rn2(-tmp);
	}
	tmp += mtmp->data->mlevel;
	if(multi < 0) tmp += 4;
	if((Invis && mtmp->data->mlet != 'I') || !mtmp->mcansee) tmp -= 2;
	if(mtmp->mtrapped) tmp -= 2;
	if(tmp <= rnd(20)) {
		if(Blind) pline("It misses.");
		else pline("%s misses.",Monnam(mtmp));
		res = 0;
	} else {
		if(Blind) pline("It hits!");
		else pline("%s hits!",Monnam(mtmp));
		if (u.usym == 'a' && !rn2(4)) {
			pline("%s is splashed by your acid!",Monnam(mtmp));
			mtmp->mhp -= rnd(10);
			if(mtmp->mhp <= 0) {
				pline("%s dies!",Monnam(mtmp));
				xkilled(mtmp,0);
			}
		}
		losehp_m(dam, mtmp);
		res = 1;
	}
	stop_occupation();
	if(u.usym=='E' && mtmp->mcansee && rn2(2)) {
		pline("%s is frozen by your gaze!",Monnam(mtmp));
		mtmp->mfroz = 1;
	}
	return(res);
}

#define	Athome	(Inhell && !mtmp->cham)

#ifdef HARD
demon_talk(mtmp)		/* returns 1 if we pay him off. */
register struct monst *mtmp;
{
	char	*xmonnam(), *Xmonnam();
	int	demand, offer;

	if(uwep && !strcmp(ONAME(uwep), "Excalibur")) {

	    pline("%s looks very angry.", Xmonnam(mtmp, 1));
	    mtmp->mpeaceful = mtmp->mtame = 0;
	    return(0);
	}
	if(!strcmp(mtmp->data->mname, "demon")) {  /* not for regular '&'s */

	    pline("%s mutters something about awful working conditions.",
		  Xmonnam(mtmp, 1));
	    return(0);
	}

	/* Slight advantage given. */
	if(!strcmp(mtmp->data->mname, "demon prince") && mtmp->minvis) {

	    if (!Blind) pline("%s appears before you.", Xmonnam(mtmp, 1));
	    mtmp->minvis = 0;
	    pmon(mtmp);
	}
	if(u.usym == '&') {	/* Won't blackmail their own. */

	    pline("%s says, 'Good hunting %s.' and vanishes",
		  Xmonnam(mtmp, 1), flags.female ? "Sister" : "Brother");
	    rloc(mtmp);
	    return(1);
	}
	demand = (u.ugold * (rnd(80) + 20 * Athome)) / 100;
	if(!demand)  {		/* you have no gold */
	    mtmp->mpeaceful = 0;
	    return(0);
	} else {
	    char buf[80];

	    pline("%s demands %d Zorkmids for safe passage.",
		  Xmonnam(mtmp, 1), demand);
	    pline("how many will you offer him?");
	    getlin(buf);
	    sscanf(buf, "%d", &offer);

	    if(offer >= u.ugold) {
		pline("You give %s all your gold.", xmonnam(mtmp, 0));
		offer = u.ugold;
	    } else pline("You give %s %d Zorkmids.", xmonnam(mtmp, 0), offer);
	    u.ugold -= offer;

	    if(offer >= demand) {
		pline("%s vanishes laughing about cowardly mortals.",
		      Xmonnam(mtmp));
	    } else {
		if(rnd(40) > (demand - offer)) {
		    pline("%s scowls at you menacingly, then vanishes.",
			  Xmonnam(mtmp));
		} else {
		    pline("%s gets angry...", Xmonnam(mtmp));
		    mtmp->mpeaceful = 0;
		    return(0);
		}
	    }
	}
	mondead(mtmp);
	return(1);
}
#endif

demon_hit(mtmp)
register struct monst *mtmp;
{
	register struct	obj	*otmp;
	int	onum, nobj = 0,
		ml = mtmp->data->mlevel;

	if(!mtmp->cham && !mtmp->mcan && !rn2(13)) {
		(void) makemon(PM_DEMON,u.ux,u.uy);
	} else {
	    switch((!mtmp->mcan) ? rn2(ml - 5 - !Athome) : 0)   {
#ifdef HARD
		case 12:
		case 11:
		case 10:
		case 9:			/* the wiz */
			(void) hitu(mtmp, 1);
			pline("Oh no, he's using the touch of death!");
			if (rn2(ml) > 12)  {

			    if(Confusion)
				pline("You have an out of body experience.");
			    else  {
				killer = "touch of death";
				done("died");
			    }
			} else pline("Lucky for you, it didn't work!");
			break;
		case 8:			/* demon princes */
			(void) hitu(mtmp, 1);
			if(!destroy_arm()) pline("Your skin itches.");
			break;
		case 7:
			(void) hitu(mtmp, 1);
			for (otmp = invent; otmp; otmp = otmp->nobj)  nobj++;
			onum = rn2(nobj);
			for(otmp = invent; onum != 0; onum--) otmp = otmp->nobj;
			otmp->cursed++;
			break;
		case 6:			/* demon lords */
			(void) hitu(mtmp, 1);
			pline("You suddenly feel weaker!");
			losestr(rnd(ml - 6));
			break;
		case 5:
			(void) hitu(mtmp, 1);
			if (Confusion)	pline("Hey, that tickles!");
			else		pline("Huh, What? Where am I?");
			HConfusion += rn1(7, 16);
			break;
#endif /* HARD /**/
		default:		/* demons and chamelons as demons */
			(void) hitu(mtmp,d(2,5 + Athome));
			(void) hitu(mtmp,d(2,5 + Athome));
			(void) hitu(mtmp,rnd(2 + Athome));
			(void) hitu(mtmp,rnd(2 + Athome));
			(void) hitu(mtmp,rn1(4,1 + Athome));
			break;
	    }
	}
	return(0);
} 
