/*	SCCS Id: @(#)eat.c	3.0	88/10/22
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

char corpsename[60];

/* hunger texts used on bottom line (each 8 chars long) */
#define	SATIATED	0
#define NOT_HUNGRY	1
#define	HUNGRY		2
#define	WEAK		3
#define	FAINTING	4
#define FAINTED		5
#define STARVED		6

const char *hu_stat[] = {
	"Satiated",
	"        ",
	"Hungry  ",
	"Weak    ",
	"Fainting",
	"Fainted ",
	"Starved "
};

static const char comestibles[] = { FOOD_SYM, 0 };

void
init_uhunger(){
	u.uhunger = 900;
	u.uhs = NOT_HUNGRY;
}

const struct { char *txt; int nut; } tintxts[] = {
	"deep fried",	60,
	"pickled",	40,
	"soup made from", 20,
	"pureed", 500,
	"rotten", -50,
	"",	0
};
#define	TTSZ	SIZE(tintxts)

static struct {
	struct obj *tin;
	int usedtime, reqtime;
} tin;

static int
Meatdone() {
	u.usym =
#ifdef POLYSELF
		u.mtimedone ? uasmon->mlet :
#endif
		S_HUMAN;
	prme();
	return 0;
}

static int
corpsefx(pm)
register int pm;
{
	register int tmp = 0, tp = 0;

	if ((pl_character[0]=='E') ? is_elf(&mons[pm]) : is_human(&mons[pm])) {
		You("cannibal!  You will be sorry for this!");
		Aggravate_monster |= INTRINSIC;
	}

	switch(pm) {
	    case PM_WRAITH:
		pluslvl();
		break;
#ifdef POLYSELF
	    case PM_WERERAT:
		u.ulycn = PM_RATWERE;
		break;
	    case PM_WEREJACKAL:
		u.ulycn = PM_JACKALWERE;
		break;
	    case PM_WEREWOLF:
		u.ulycn = PM_WOLFWERE;
		break;
#endif
	    case PM_NURSE:
		u.uhp = u.uhpmax;
		flags.botl = 1;
		break;
	    case PM_LITTLE_DOG:
	    case PM_DOG:
	    case PM_LARGE_DOG:
	    case PM_KITTEN:
	    case PM_HOUSECAT:
	    case PM_LARGE_CAT:
		Aggravate_monster |= INTRINSIC;
		break;
	    case PM_STALKER:
		if(!Invis) {
			HInvis = 50+rn2(100);
			if(!See_invisible)
				newsym(u.ux, u.uy);
		} else {
			if (!HInvis) You("feel hidden!");
			HInvis |= INTRINSIC;
			HSee_invisible |= INTRINSIC;
		}
		/* fall into next case */
	    case PM_YELLOW_LIGHT:
		/* fall into next case */
	    case PM_GIANT_BAT:
		make_stunned(HStun + 30,FALSE);
		/* fall into next case */
	    case PM_BAT:
		make_stunned(HStun + 30,FALSE);
		break;
	    case PM_COCKATRICE:
#ifdef MEDUSA
	    case PM_MEDUSA:
#endif
#ifdef POLYSELF
		if(!resists_ston(uasmon)) {
#endif
			killer = (char *) alloc(40);
			You("turn to stone.");
			Sprintf(killer, "%s meat",
				      mons[pm].mname);
			done(STONING);
#ifdef POLYSELF
		}
#endif
		break;
	    case PM_GIANT_MIMIC:
		tmp += 10;
		/* fall into next case */
	    case PM_LARGE_MIMIC:
		tmp += 20;
		/* fall into next case */
	    case PM_SMALL_MIMIC:
		tmp += 20;
		if(u.usym == S_HUMAN) {
		    You("cannot resist the temptation to mimic a treasure chest.");
		    tp++;
		    nomul(tmp);
		    afternmv = Meatdone;
		    nomovemsg = "You now again prefer mimicking a human.";
		    u.usym = GOLD_SYM;
		    prme();
		}
		break;
	    case PM_FLOATING_EYE:
		if (!(HTelepat & INTRINSIC)) {
			HTelepat |= INTRINSIC;
			You("feel a %s mental acuity.",
			Hallucination ? "normal" : "strange");
		}
		break;
	    case PM_QUANTUM_MECHANIC:
		Your("velocity suddenly seems very uncertain!");
		if (Fast & INTRINSIC) {
			Fast &= ~INTRINSIC;
			You("seem slower.");
		} else {
			Fast |= INTRINSIC;
			You("seem faster.");
		}
		break;
#ifdef POLYSELF
	    case PM_CHAMELEON:
		You("feel a change coming over you.");
		polyself();
		break;
#endif
	    default: {
		register struct permonst *ptr = &mons[pm];
		if(dmgtype(ptr, AD_STUN) || ptr==&mons[PM_VIOLET_FUNGUS]) {
		    pline ("Oh wow!  Great stuff!");
		    make_hallucinated(Hallucination + 200,FALSE);
		}
		if(dmgtype(ptr, AD_ACID)) {
		  if(Stoned) {
			pline("What a pity - you just destroyed a future piece of art!");
			tp++;
			Stoned = 0;
		  }
		}
		if(is_giant(ptr)) gainstr((struct obj *)0, 0);

		if(can_teleport(ptr) && ptr->mlevel > rn2(10)) {
		    if (!(HTeleportation & INTRINSIC)) {
			You("feel very jumpy.");
			HTeleportation |= INTRINSIC;
		    }
		} else if(control_teleport(ptr) && ptr->mlevel > rn2(20)) {
		    if (!(HTeleport_control & INTRINSIC)) {
			You("feel in control of yourself.");
			HTeleport_control |= INTRINSIC;
		    }
		} else if(resists_fire(ptr) && ptr->mlevel > rn2(20)) {
		    if (!(HFire_resistance & INTRINSIC)) {
			You("feel a momentary chill.");
			HFire_resistance |= INTRINSIC;
		    }
		} else if(resists_cold(ptr) && ptr->mlevel > rn2(20)) {
		    if (!(HCold_resistance & INTRINSIC)) {
			You("feel full of hot air.");
			HCold_resistance |= INTRINSIC;
		    }
		} else if((ptr->mflags1 & M1_POIS_RES) && ptr->mlevel>rn2(20)) {
		/* Monsters with only M1_POIS are poison resistant themselves,
		 * but do not confer resistance when eaten
		 */
		    if (!(HPoison_resistance & INTRINSIC)) {
			You("feel healthy.");
			HPoison_resistance |= INTRINSIC;
		    }
		} else if(resists_elec(ptr) && ptr->mlevel > rn2(20)) {
		    if (!(HShock_resistance & INTRINSIC)) {
			Your("health currently feels amplified!");
			HShock_resistance |= INTRINSIC;
		    }
		} else if((ptr->mflags1 & M1_SLEE_RES) && ptr->mlevel > rn2(20)) {
		/* Undead monsters never sleep,
		 * but also do not confer resistance when eaten
		 */
		    if (!(HSleep_resistance & INTRINSIC)) {
			You("feel wide awake.");
			HSleep_resistance |= INTRINSIC;
		    }
		} else if(resists_disint(ptr) && ptr->mlevel > rn2(20)) {
		    if (!(HDisint_resistance & INTRINSIC)) {
			You("feel very firm.");
			HDisint_resistance |= INTRINSIC;
		    }
		}
	    }
	    break;
	}
	return(tp);
}

static int
opentin(){
	register int r;

	if(!carried(tin.tin))		/* perhaps it was stolen? */
		return(0);		/* %% probably we should use tinoid */
	if(tin.usedtime++ >= 50) {
		You("give up your attempt to open the tin.");
		return(0);
	}
	if(tin.usedtime < tin.reqtime)
		return(1);		/* still busy */
	if(tin.tin->cursed && !rn2(8)) {
		b_trapped("tin");
		useup(tin.tin);
		return(0);
	}
	You("succeed in opening the tin.");
	if(!tin.tin->spe) {
	    if(tin.tin->corpsenm == -1) {
		pline("It turns out to be empty.");
		tin.tin->dknown = tin.tin->known = TRUE;
		useup(tin.tin);
		return(0);
	    }
	    r = tin.tin->cursed ? 4 : rn2(TTSZ-1); /* Always rotten if cursed */
	    pline("It smells like %s.", makeplural(
		  Hallucination ? rndmonnam() : mons[tin.tin->corpsenm].mname));
	    pline("Eat it? ");
	    if (yn() == 'n') {
		if (!Hallucination) tin.tin->dknown = tin.tin->known = TRUE;
		useup(tin.tin);
		return 0;
	    }
	    You("consume %s %s.", tintxts[r].txt,
		  mons[tin.tin->corpsenm].mname);
	    tin.tin->dknown = tin.tin->known = TRUE;
	    (void) corpsefx(tin.tin->corpsenm);
	    /* check for vomiting added by GAN 01/16/87 */
	    if(tintxts[r].nut < 0) {
		You("vomit.");
		vomit();
		morehungry(-tintxts[r].nut);
	    } else lesshungry(tintxts[r].nut);
	    if(r == 0) {			/* Deep Fried */
		Glib = rnd(15);
		pline("Eating deep fried food made your %s very slippery.",
			makeplural(body_part(FINGER)));
	    }
	} else {
	    if (tin.tin->cursed)
		pline("It contains some decaying %s substance.",
			Hallucination ? hcolor() : green);
	    else
		pline("It contains spinach - this makes you feel like %s!",
			Hallucination ? "Swee'pea" : "Popeye");

	    lesshungry(600);
	    gainstr(tin.tin, 0);
	}
	tin.tin->dknown = tin.tin->known = TRUE;
	useup(tin.tin);
	return(0);
}

int
Hear_again()
{
	flags.soundok = 1;
	return 0;
}

static void
rottenfood() {

	pline("Blecch!  Rotten food!");
	if(!rn2(4)) {
		if (Hallucination) You("feel rather trippy.");
		else
			You("feel rather %s.",
				body_part(LIGHT_HEADED));
		make_confused(HConfusion + d(2,4),FALSE);
	} else if(!rn2(4) && !Blind) {
		pline("Everything suddenly goes dark.");
		make_blinded((long)d(2,10),FALSE);
	} else if(!rn2(3)) {
		if(Blind)
		  pline("The world spins and you slap against the floor.");
		else
		  pline("The world spins and goes dark.");
		flags.soundok = 0;
		nomul(-rnd(10));
		nomovemsg = "You are conscious again.";
		afternmv = Hear_again;
	}
}

static void
eatcorpse(otmp) register struct obj *otmp; {
	register char *cname = mons[otmp->corpsenm].mname;
	register int tp, rotted;

	tp = 0;
#ifdef LINT	/* problem if more than 320K moves before try to eat */
	rotted = 0;
#else
	rotted = (moves - otmp->age)/((long)(10 + rn2(20)));	/* how decomposed? */
#endif

	if(otmp->cursed) rotted += 2;
	else if (otmp->blessed) rotted -= 2;

	if(otmp->corpsenm != PM_ACID_BLOB && (rotted > 5)) {
		tp++;
		pline("Ulch - that %s was tainted!",
		      mons[otmp->corpsenm].mlet != S_FUNGUS ?
				"meat" : "fungoid vegetation");
#ifdef POLYSELF
		if (u.usym == S_FUNGUS)
			pline("It doesn't seem at all sickening, though...");
		else {
#endif
			make_sick(10L + rn2(10),FALSE);
			Sprintf(corpsename, "rotted %s corpse", cname);
			u.usick_cause = corpsename;
			flags.botl = 1;
#ifdef POLYSELF
		}
#endif
	} else if(poisonous(&mons[otmp->corpsenm]) && rn2(5)){
		pline("Ecch - that must have been poisonous!");
		if(!Poison_resistance) {
			losestr(rnd(4));
			losehp(rnd(15), "poisonous corpse");
		} else	You("seem unaffected by the poison.");
		(void) corpsefx(otmp->corpsenm);
		tp++;
	/* now any corpse left too long will make you mildly ill */
	} else if(((rotted > 5) || ((rotted > 3) && rn2(5)))
#ifdef POLYSELF
		&& u.usym != S_FUNGUS
#endif
							){
		tp++;
		You("feel%s sick.", (Sick) ? " very" : "");
		losehp(rnd(8), "cadaver");
	} else	tp = corpsefx(otmp->corpsenm);
	if(!tp && !rn2(7)) {

	    rottenfood();
	    lesshungry((int)mons[otmp->corpsenm].cnutrit >> 2);
	} else {
#ifdef POLYSELF
	    pline("That %s corpse %s!", cname,
		carnivorous(uasmon) ? "was delicious" : "tasted terrible");
#else
	    pline("That %s corpse tasted terrible!", cname);
#endif
	    lesshungry((int)mons[otmp->corpsenm].cnutrit);
	}

	/* delay is weight dependant */
	multi = -(3 + (mons[otmp->corpsenm].cwt >> 2));
}

/* Created by GAN 01/28/87
 * Amended by AKP 09/22/87: if not hard, don't choke, just vomit.
 * Amended by 3.  06/12/89: if not hard, sometimes choke anyway, to keep risk.
 */
/*ARGSUSED*/
static void
choke(food)
register struct obj *food;
{
	/* only happens if you were satiated */
	if(u.uhs != SATIATED) return;

	if (pl_character[0] == 'K' && u.ualigntyp == U_LAWFUL)
		u.ualign--;	/* gluttony is unchivalrous */

#ifndef HARD
	if (rn2(20)) {
		You("stuff yourself and then vomit voluminously.");
		morehungry(1000);	/* you just got *very* sick! */
		vomit();
	} else {
#endif
		if(food) {
			int savequan = food->quan;
			food->quan = 1;
			killer = xname(food);
			food->quan = savequan;
		} else
			killer = "exuberant appetite";
		You("choke over your food.");
		You("die...");
		done(CHOKING);
#ifndef HARD
	}
#endif
}

int
doeat() {
	register struct obj *otmp;
	register struct objclass *ftmp;
	register int tmp;

	if (!(otmp = floorfood("eat", 0))) return 0;

	if(otmp->otyp == TIN) {
		if (otmp->blessed) {
			pline("The tin opens like magic!");
			tmp = 1;
		} else if(uwep) {
			switch(uwep->otyp) {
			case TIN_OPENER:
				tmp = 1;
				break;
			case DAGGER:
#ifdef WORM
			case CRYSKNIFE:
#endif
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
				aobjnam(uwep, NULL));
		} else {
		no_opener:
			pline("It is not so easy to open this tin.");
			if(Glib) {
				pline("The tin slips out of your hands.");
				if(otmp->quan > 1) {
					register struct obj *obj;
					obj = splitobj(otmp, 1);
					if(otmp == uwep) setuwep(obj);
				}
				dropx(otmp);
				return(1);
			}
			tmp = 10 + rn2(1 + 500/((int)(ACURR(A_DEX) + ACURR(A_STR))));
		}
		tin.reqtime = tmp;
		tin.usedtime = 0;
		tin.tin = otmp;
		set_occupation(opentin, "opening the tin", 0);
		return(1);
	}

	ftmp = &objects[otmp->otyp];
	multi = -ftmp->oc_delay;
	if(otmp->otyp == CORPSE) eatcorpse(otmp);
	else {
	    if (otmp->otyp != FORTUNE_COOKIE &&
		otmp->otyp != DEAD_LIZARD &&
		(otmp->cursed ||
		 ((moves - otmp->age) > otmp->blessed ? 50 : 30)) &&
		  !rn2(7)) {

		rottenfood();
		lesshungry(ftmp->nutrition >> 2);
	    } else {
		if(u.uhunger >= 1500) choke(otmp);

		switch(otmp->otyp){
		case FOOD_RATION:
			if(u.uhunger <= 200)
			    if (Hallucination)
				pline("Oh wow, like, superior, man!");
			    else
				pline("That food really hit the spot!");
			else if(u.uhunger <= 700)
				pline("That satiated your stomach!");
	/* Have lesshungry() report when you are nearly full so all eating
	 * warns when you are about to choke.
	 */
			lesshungry(ftmp->nutrition);
			if(multi < 0) nomovemsg = "You finished your meal.";
			break;
		case TRIPE_RATION:
#ifdef POLYSELF
			if (carnivorous(uasmon))
			    pline("That tripe ration was surprisingly good!");
			else {
#endif
			    pline("Yak - dog food!");
			    more_experienced(1,0);
			    flags.botl = 1;
#ifdef POLYSELF
			}
#endif
			if(rn2(2)
#ifdef POLYSELF
				&& u.usym == S_HUMAN
#endif
							){
				You("vomit.");
				morehungry(20);
				vomit();
			} else	lesshungry(ftmp->nutrition);
			break;
#ifdef POLYSELF
		case CLOVE_OF_GARLIC:
			if (is_undead(uasmon)) {
				You("cannot stand eating it.  You vomit.");
				vomit();
				break;
			}
			/* Fall through otherwise */
#endif
		default:
#ifdef TUTTI_FRUTTI
			if (otmp->otyp==SLIME_MOLD && !otmp->cursed
				&& otmp->spe == current_fruit
								)
			    pline(!Hallucination ?
				    "Mmm!  Your favorite!" :
				    "Yum!  Your fave fruit!");
			else
#endif
			{
			    int oldquan = otmp->quan;
			    otmp->quan = 1;
			    pline("That %s was %s!", xname(otmp),
			      otmp->cursed ?
				(Hallucination ? "grody" : "terrible"):
			      Hallucination ? "gnarly" : (
#ifdef TOLKIEN
			       otmp->otyp==CRAM_RATION ? "bland":
#endif
			       "delicious"));
			    otmp->quan = oldquan;
			}
			lesshungry(ftmp->nutrition);

			switch(otmp->otyp) {
#ifdef POLYSELF
			    case CLOVE_OF_GARLIC:
				if (u.ulycn != -1) {
					You("feel purified.");
					if(uasmon == &mons[u.ulycn] &&
					  !Polymorph_control)
						rehumanize();
					u.ulycn = -1;
				}
				break;
#endif
			    case DEAD_LIZARD:
				/* Relief from cockatrices -dgk */
				if (Stoned) {
					Stoned = 0;
					You("feel limber!");
				}
				if (HStun > 2)  make_stunned(2L,FALSE);
				if (HConfusion > 2)  make_confused(2L,FALSE);
				break;
			    case CARROT:
				make_blinded(0L,TRUE);
				break;
			    case FORTUNE_COOKIE:
				outrumor(bcsign(otmp), TRUE);
				break;
			    case LUMP_OF_ROYAL_JELLY:
				/* This stuff seems to be VERY healthy! */
				gainstr(otmp, 1);
				u.uhp += (otmp->cursed) ? -rnd(20) : rnd(20);
				if(u.uhp > u.uhpmax) {
					if(!rn2(17)) u.uhpmax++;
					u.uhp = u.uhpmax;
				} else if(u.uhp <= 0) {
					killer = "rotten jelly lump";
					done(POISONING);
				}
				if(!otmp->cursed) heal_legs();
				break;
			    case EGG:
				if(otmp->corpsenm == PM_COCKATRICE) {
#ifdef POLYSELF
				    if(!resists_ston(uasmon)) {
#endif
					if (!Stoned) Stoned = 5;
					killer = "cockatrice egg";
#ifdef POLYSELF
				    }
#endif
				}
				break;
			    default:	break;
			}
			break;
		}
	    }
	}
	

	if(multi < 0 && !nomovemsg){
#ifdef LINT	/* JAR		static char msgbuf[BUFSZ]; */
		char msgbuf[BUFSZ];
#else
		static char msgbuf[BUFSZ];
#endif
		/* note: ftmp->oc_name usually works, the exception being
		 * for fruits.  If fruits are changed to take more time to
		 * eat, this has to be modified.
		 */
		if (otmp->otyp != CORPSE)
			Sprintf(msgbuf, "You finish eating the %s.",
						ftmp->oc_name);
		else
			Sprintf(msgbuf, "You finish eating the %s corpse.",
						mons[otmp->corpsenm].mname);
		nomovemsg = msgbuf;
	}
	useup(otmp);
	return(1);
}

/* called in main.c */
void
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
		/* +0 rings don't do anything, so don't affect hunger */
		if(uleft && uleft->otyp && (!objects[uleft->otyp].oc_charged
			|| uleft->spe)) u.uhunger--;
		if(uright && uright->otyp && (!objects[uright->otyp].oc_charged
			|| uright->spe)) u.uhunger--;
		if(uamul) u.uhunger--;
		if(u.uhave_amulet) u.uhunger--;
	}
	newuhs(TRUE);
}

/* called after vomiting and after performing feats of magic */
void
morehungry(num)
register int num;
{
	u.uhunger -= num;
	newuhs(TRUE);
}

/* called after eating something (and after drinking fruit juice) */
void
lesshungry(num)
register int num;
{
	u.uhunger += num;
	if(u.uhunger >= 2000) choke((struct obj *) 0);
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
	newuhs(FALSE);
}

static int
unfaint() {
	(void) Hear_again();
	u.uhs = FAINTING;
	flags.botl = 1;
	return 0;
}

void
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
				You("faint from lack of food.");
				flags.soundok = 0;
				nomul(-10+(u.uhunger/10));
				nomovemsg = "You regain consciousness.";
				afternmv = unfaint;
				newhs = FAINTED;
			}
		} else
		if(u.uhunger < -(int)(200 + 20*ACURR(A_CON))) {
			u.uhs = STARVED;
			flags.botl = 1;
			bot();
			You("die from starvation.");
			done(STARVING);
		}
	}

	if(newhs != u.uhs) {
		if(newhs >= WEAK && u.uhs < WEAK)
			losestr(1);	/* this may kill you -- see below */
		else if(newhs < WEAK && u.uhs >= WEAK)
			losestr(-1);
		switch(newhs){
		case HUNGRY:
			if (Hallucination) {
			    pline((!incr) ?
				"You now have a lesser case of the munchies." :
				"You are getting the munchies.");
			} else
			    You((!incr) ? "only feel hungry now." :
				  (u.uhunger < 145) ? "feel hungry." :
				   "are beginning to feel hungry.");
			break;
		case WEAK:
			if (Hallucination)
			    pline((!incr) ?
				  "You still have the munchies." :
				  "The munchies are starting to interfere with your motor capabilities.");
			else
			    You((!incr) ? "feel weak now." :
				  (u.uhunger < 45) ? "feel weak." :
				   "are beginning to feel weak.");
			break;
		}
		u.uhs = newhs;
		flags.botl = 1;
		if(u.uhp < 1) {
			You("die from hunger and exhaustion.");
			killer = "exhaustion";
			done(STARVING);
		}
	}
}

struct obj *
floorfood(verb,corpseonly)
char *verb;
int corpseonly;
{
	register struct obj *otmp;

	/* Is there some food (probably a heavy corpse) here on the ground? */
	if(!Levitation && !u.uswallow) {
	if(levl[u.ux][u.uy].omask)
	    for(otmp = fobj; otmp; otmp = otmp->nobj) {
		if(otmp->ox == u.ux && otmp->oy == u.uy &&
		   (otmp->otyp==CORPSE ||
		   (!corpseonly && otmp->olet == FOOD_SYM))) {
			pline("There %s %s here; %s %s? ",
				(otmp->quan == 1) ? "is" : "are",
				doname(otmp), verb,
				(otmp->quan == 1) ? "it" : "one");
			if(yn() == 'y') {
				if(otmp->quan != 1)
					(void) splitobj(otmp, 1);
				freeobj(otmp);
				otmp = addinv(otmp);
				addtobill(otmp, TRUE);
				if(Invisible) newsym(u.ux, u.uy);
				return otmp;
			}
		}
	    }
	}
	return getobj(comestibles, verb);
}

/* Side effects of vomiting */
/* TO DO: regurgitate swallowed monsters when poly'd */
void
vomit() { /* A good idea from David Neves */
	make_sick(0L,TRUE);
}
