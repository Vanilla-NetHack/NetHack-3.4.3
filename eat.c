/*	SCCS Id: @(#)eat.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* eat.c - version 1.0.3 */

#include	"hack.h"
#ifdef KAA
char POISONOUS[] = "ADKSVabhks&";
#else
char POISONOUS[] = "ADKSVabhks";
#endif
extern char *nomovemsg;
extern int (*afternmv)();
extern int (*occupation)();
extern char *occtxt;
extern struct obj *splitobj(), *addinv();

/* hunger texts used on bottom line (each 8 chars long) */
#define	SATIATED	0
#define NOT_HUNGRY	1
#define	HUNGRY		2
#define	WEAK		3
#define	FAINTING	4
#define FAINTED		5
#define STARVED		6

char *hu_stat[] = {
	"Satiated",
	"        ",
	"Hungry  ",
	"Weak    ",
	"Fainting",
	"Fainted ",
	"Starved "
};

init_uhunger(){
	u.uhunger = 900;
	u.uhs = NOT_HUNGRY;
}

struct { char *txt; int nut; } tintxts[] = {
	"It contains salmon - not bad!",	60,
	"It contains first quality peaches - what a surprise!",	40,
	"It contains apple juice - perhaps not what you hoped for.", 20,
	"It contains some nondescript substance, tasting awfully.", 500,
	"It contains rotten meat. You vomit.", -50,
	"It turns out to be empty.",	0
};
#define	TTSZ	SIZE(tintxts)

static struct {
	struct obj *tin;
	int usedtime, reqtime;
} tin;

opentin(){
	register int r;

	if(!carried(tin.tin))		/* perhaps it was stolen? */
		return(0);		/* %% probably we should use tinoid */
	if(tin.usedtime++ >= 50) {
		pline("You give up your attempt to open the tin.");
		return(0);
	}
	if(tin.usedtime < tin.reqtime)
		return(1);		/* still busy */

	pline("You succeed in opening the tin.");
	useup(tin.tin);
	r = rn2(2*TTSZ);
	if(r < TTSZ) {
	    pline(tintxts[r].txt);
	    lesshungry(tintxts[r].nut);
	    /* check for vomiting added by GAN 01/16/87 */
	    if(tintxts[r].nut < 0 && Sick) {
		Sick = 0;
		pline("What a relief!");
	    }
	    if(r == 0) {			/* Salmon */
		Glib = rnd(15);
		pline("Eating salmon made your fingers very slippery.");
	    }
	} else {
	    pline("It contains spinach - this makes you feel like %s!",
		Hallucination ? "Swee'pea" : "Popeye");

	    lesshungry(600);
	    gainstr(0);
	}
	return(0);
}

Meatdone(){
	u.usym = '@';
	prme();
}

doeat(){
	register struct obj *otmp;
	register struct objclass *ftmp;
	register tmp;

	/* Is there some food (probably a heavy corpse) here on the ground? */
	if(!Levitation)
	for(otmp = fobj; otmp; otmp = otmp->nobj) {
		if(otmp->ox == u.ux && otmp->oy == u.uy &&
		   otmp->olet == FOOD_SYM) {
			pline("There %s %s here; eat %s? [ny] ",
				(otmp->quan == 1) ? "is" : "are",
				doname(otmp),
				(otmp->quan == 1) ? "it" : "one");
			if(readchar() == 'y') {
				if(otmp->quan != 1)
					(void) splitobj(otmp, 1);
				freeobj(otmp);
				otmp = addinv(otmp);
				addtobill(otmp);
				if(Invisible) newsym(u.ux, u.uy);
				goto gotit;
			}
		}
	}
	otmp = getobj("%", "eat");
	if(!otmp) return(0);
gotit:
	if(otmp->otyp == TIN) {
		if(uwep) {
			switch(uwep->otyp) {
			case CAN_OPENER:
				tmp = 1;
				break;
			case DAGGER:
			case CRYSKNIFE:
				tmp = 3;
				break;
			case PICK_AXE:
			case AXE:
				tmp = 6;
				break;
			default:
				goto no_opener;
			}
			pline("Using your %s you try to open the tin.",
				aobjnam(uwep, (char *) 0));
		} else {
		no_opener:
			pline("It is not so easy to open this tin.");
			if(Glib) {
				pline("The tin slips out of your hands.");
				if(otmp->quan > 1) {
					register struct obj *obj;
					extern struct obj *splitobj();

					obj = splitobj(otmp, 1);
					if(otmp == uwep) setuwep(obj);
				}
				dropx(otmp);
				return(1);
			}
			tmp = 10 + rn2(1 + 500/((int)(u.ulevel + u.ustr)));
		}
		tin.reqtime = tmp;
		tin.usedtime = 0;
		tin.tin = otmp;
#ifdef DGK
		set_occupation(opentin, "opening the tin", 0);
#else
		occupation = opentin;
		occtxt = "opening the tin";
#endif
		return(1);
	}
	ftmp = &objects[otmp->otyp];
	multi = -ftmp->oc_delay;
	if(otmp->otyp >= CORPSE && eatcorpse(otmp)) goto eatx;
#ifdef DGKMOD
	if(!rn2(7) && otmp->otyp != FORTUNE_COOKIE && otmp->otyp != DEAD_LIZARD) {
#else
	if(!rn2(7) && otmp->otyp != FORTUNE_COOKIE) {
#endif
#ifdef KAA
		if (otmp->otyp == DEAD_VIOLET_FUNGUS)
			pline("Seems rather stale though...");
		else
#endif
		pline("Blecch!  Rotten food!");
		if(!rn2(4)) {
			if (Hallucination) pline("You feel rather trippy.");
			else
				pline("You feel rather light headed.");
			HConfusion += d(2,4);
		} else if(!rn2(4) && !Blind) {
			pline("Everything suddenly goes dark.");
			Blind = d(2,10);
			seeoff(0);
		} else if(!rn2(3)) {
			if(Blind)
			  pline("The world spins and you slap against the floor.");
			else
			  pline("The world spins and goes dark.");
			nomul(-rnd(10));
			nomovemsg = "You are conscious again.";
		}
		lesshungry(ftmp->nutrition / 4);
	} else {
		if(u.uhunger >= 1500) choke(ftmp);

		switch(otmp->otyp){
		case FOOD_RATION:
			if(u.uhunger <= 200)
			    if (Hallucination)
				pline("Oh wow, like superior man!");
			    else
				pline("That food really hit the spot!");
			else if(u.uhunger <= 700)
				pline("That satiated your stomach!");
#ifdef DGKMOD
	/* Have lesshungry() report when you're nearly full so all eating
	 * warns when you're about to choke.
	 */
			lesshungry(ftmp->nutrition);
#else
			else {
	pline("You're having a hard time getting all that food down.");
				multi -= 2;
			}
			lesshungry(ftmp->nutrition);
			if(multi < 0) nomovemsg = "You finished your meal.";
#endif /* DGKMOD /**/
			break;
		case TRIPE_RATION:
			if (u.usym != '@')
			    pline("That tripe ration was surprisingly good!");
			else {
			    pline("Yak - dog food!");
			    more_experienced(1,0);
			    flags.botl = 1;
			}
			if(rn2(2) && u.usym == '@'){
				pline("You vomit.");
				morehungry(20);
				if(Sick) {
					Sick = 0;	/* David Neves */
					pline("What a relief!");
				}
			} else	lesshungry(ftmp->nutrition);
			break;
		default:
			if(u.usym == '@' && otmp->otyp >= CORPSE) {
#ifdef KAA
			    if(otmp->otyp != DEAD_VIOLET_FUNGUS)
#endif
			    pline("That %s tasted terrible!",ftmp->oc_name);
			} else
			pline("That %s was delicious!",ftmp->oc_name);
			lesshungry(ftmp->nutrition);
#ifdef DGKMOD
			/* Relief from cockatrices -dgk */
			if (otmp->otyp == DEAD_LIZARD) {
				if (Stoned) {
					Stoned = 0;
					pline("You feel more limber!");
				}
				if (HConfusion > 2)
					HConfusion = 2;
			}
#else
			if(otmp->otyp == DEAD_LIZARD && (HConfusion > 2))
				HConfusion = 2;
#endif /* DGKMOD /**/
			else
#ifdef QUEST
			if(otmp->otyp == CARROT && !Blind){
				u.uhorizon++;
				setsee();
				pline("Your vision improves.");
			} else
#endif
#ifdef KAA
			if(otmp->otyp == CARROT && Blind) Blind=1;
			else
#endif
			if(otmp->otyp == FORTUNE_COOKIE) {
			  if(Blind) {
			    pline("This cookie has a scrap of paper inside!");
			    pline("What a pity, that you cannot read it!");
			  } else
			    outrumor();
			} else
			if(otmp->otyp == LUMP_OF_ROYAL_JELLY) {
				/* This stuff seems to be VERY healthy! */
				gainstr(1);
				u.uhp += rnd(20);
				if(u.uhp > u.uhpmax) {
					if(!rn2(17)) u.uhpmax++;
					u.uhp = u.uhpmax;
				}
				heal_legs();
			}
			break;
		}
	}
eatx:
	if(multi<0 && !nomovemsg){
		static char msgbuf[BUFSZ];
		(void) sprintf(msgbuf, "You finished eating the %s.",
				ftmp->oc_name);
		nomovemsg = msgbuf;
	}
	useup(otmp);
	return(1);
}

/* called in main.c */
gethungry(){
	--u.uhunger;
	if(moves % 2) {
		if(HRegeneration) u.uhunger--;
		if(Hunger) u.uhunger--;
		/* a3:  if(Hunger & LEFT_RING) u.uhunger--;
			if(Hunger & RIGHT_RING) u.uhunger--;
		   etc. */
	}
	if(moves % 20 == 0) {			/* jimt@asgb */
		if(uleft) u.uhunger--;
		if(uright) u.uhunger--;
	}
	newuhs(TRUE);
}

/* called after vomiting and after performing feats of magic */
morehungry(num) register num; {
	u.uhunger -= num;
	newuhs(TRUE);
}

/* called after eating something (and after drinking fruit juice) */
lesshungry(num) register num; {
	u.uhunger += num;
	if(u.uhunger >= 2000) choke(FALSE);
#ifdef DGKMOD
	else {
	    /* Have lesshungry() report when you're nearly full so all eating
	     * warns when you're about to choke.
	     */
	    if (u.uhunger >= 1500) {
		pline("You're having a hard time getting all of it down.");
		multi -= 2;
		nomovemsg = "You're finally finished.";
	    }
	}
#endif /* DGKMOD /**/
	newuhs(FALSE);
}

unfaint(){
	u.uhs = FAINTING;
	flags.botl = 1;
}

newuhs(incr) boolean incr; {
	register int newhs, h = u.uhunger;

	newhs = (h > 1000) ? SATIATED :
		(h > 150) ? NOT_HUNGRY :
		(h > 50) ? HUNGRY :
		(h > 0) ? WEAK : FAINTING;

	if(newhs == FAINTING) {
		if(u.uhs == FAINTED) newhs = FAINTED;
		if(u.uhs <= WEAK || rn2(20-u.uhunger/10) >= 19) {
			if(u.uhs != FAINTED && multi >= 0 /* %% */) {
				pline("You faint from lack of food.");
				nomul(-10+(u.uhunger/10));
				nomovemsg = "You regain consciousness.";
				afternmv = unfaint;
				newhs = FAINTED;
			}
		} else
		if(u.uhunger < -(int)(200 + 25*u.ulevel)) {
			u.uhs = STARVED;
			flags.botl = 1;
			bot();
			pline("You die from starvation.");
			done("starved");
		}
	}

	if(newhs != u.uhs) {
		if(newhs >= WEAK && u.uhs < WEAK)
			losestr(1);	/* this may kill you -- see below */
		else
		if(newhs < WEAK && u.uhs >= WEAK && u.ustr < u.ustrmax)
			losestr(-1);
		switch(newhs){
		case HUNGRY:
			if (Hallucination) {
			    pline((!incr) ?
				"You now have a lesser case of the munchies." :
				"You are getting the munchies.");
			} else
			    pline((!incr) ? "You only feel hungry now." :
				  (u.uhunger < 145) ? "You feel hungry." :
				   "You are beginning to feel hungry.");
			break;
		case WEAK:
			if (Hallucination)
			    pline((!incr) ?
				  "You still have the munchies." :
				  "The munchies are starting to interfere with your motor capabilities.");
			else
			    pline((!incr) ? "You feel weak now." :
				  (u.uhunger < 45) ? "You feel weak." :
				   "You are beginning to feel weak.");
			break;
		}
		u.uhs = newhs;
		flags.botl = 1;
		if(u.uhp < 1) {
			pline("You die from hunger and exhaustion.");
			killer = "exhaustion";
			done("starved");
		}
	}
}

#define	CORPSE_I_TO_C(otyp)	(char) ((otyp >= DEAD_ACID_BLOB)\
		     ?  'a' + (otyp - DEAD_ACID_BLOB)\
		     :	'@' + (otyp - DEAD_HUMAN))
poisonous(otmp)
register struct obj *otmp;
{
#ifdef KAA
	if(otmp->otyp == DEAD_DEMON) return(1);
#endif
	return(index(POISONOUS, CORPSE_I_TO_C(otmp->otyp)) != 0);
}

/* returns 1 if some text was printed */
eatcorpse(otmp) register struct obj *otmp; {
#ifdef KAA
register char let;
#else
register char let = CORPSE_I_TO_C(otmp->otyp);
#endif
register tp = 0;
#ifdef KAA
	if(otmp->otyp == DEAD_DEMON) let='&';
	else if (otmp->otyp == DEAD_GIANT) let='9';
	else let = CORPSE_I_TO_C(otmp->otyp);
#endif
	if(let != 'a' && moves > otmp->age + 50 + rn2(100)) {
		tp++;
		pline("Ulch -- that meat was tainted!");
		pline("You get very sick.");
		Sick = 10 + rn2(10);
		u.usick_cause = objects[otmp->otyp].oc_name;
	} else if(index(POISONOUS, let) && rn2(5)){
		tp++;
		pline("Ecch -- that must have been poisonous!");
		if(!Poison_resistance){
			losestr(rnd(4));
			losehp(rnd(15), "poisonous corpse");
		} else
			pline("You don't seem affected by the poison.");
	} else if(index("ELNOPQRUuxz", let) && rn2(5)){
		tp++;
		pline("You feel sick.");
		losehp(rnd(8), "cadaver");
	}
	switch(let) {
	case 'L':
	case 'N':
	case 't':
#ifdef KAA
	case 'Q':
#endif
		HTeleportation |= INTRINSIC;
		break;
	case 'W':
		pluslvl();
		break;
	case 'n':
		u.uhp = u.uhpmax;
		flags.botl = 1;
		/* fall into next case */
	case '@':
		pline("You cannibal! You will be sorry for this!");
		/* not tp++; */
		/* fall into next case */
	case 'd':
		Aggravate_monster |= INTRINSIC;
		break;
	case 'I':
		if(!Invis) {
			HInvis = 50+rn2(100);
			if(!See_invisible)
				newsym(u.ux, u.uy);
		} else {
			HInvis |= INTRINSIC;
			HSee_invisible |= INTRINSIC;
		}
		/* fall into next case */
	case 'y':
#ifdef QUEST
		u.uhorizon++;
#endif
		/* fall into next case */
	case 'B':
		HConfusion += 50;
		break;
	case 'D':
		HFire_resistance |= INTRINSIC;
		break;
	case 'E':
		HTelepat |= INTRINSIC;
		break;
	case 'F':
	case 'Y':
		HCold_resistance |= INTRINSIC;
		break;
#ifdef KAA
	case '9':
		gainstr(1);
		break;
#endif
	case 'k':
	case 's':
		HPoison_resistance |= INTRINSIC;
		break;
	case 'c':
		if (u.usym != 'c') {

			pline("You turn to stone.");
			killer = "dead cockatrice";
			done("died");
		}
		break;
	case 'a':
	  if(Stoned) {
	      pline("What a pity - you just destroyed a future piece of art!");
	      tp++;
	      Stoned = 0;
	  }
	  break;
#ifdef KAA
	case 'v':
		pline ("Oh wow!  Great stuff!");
		Hallucination += 200;
		setsee();
		break;
#endif
	case 'M':
		if(u.usym == '@') {
		    pline("You cannot resist the temptation to mimic a treasure chest.");
		    tp++;
		    nomul(-30);
		    afternmv = Meatdone;
		    nomovemsg = "You now again prefer mimicking a human.";
		    u.usym = '$';
		    prme();
		}
		break;
	}
	return(tp);
}

/* added by GAN 01/28/87 */
choke(food)
register struct objclass *food;
{
	/* only happens if you were satiated */
	if(u.uhs != SATIATED) return;

	if(food)	killer = food->oc_name;
	else		killer = "exuberant appetite";
	pline("You choke over your food.");
	pline("You die...");
	done("choked");
}
