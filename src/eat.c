/*	SCCS Id: @(#)eat.c	3.0	89/11/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
/*#define DEBUG 	/* uncomment to enable new eat code debugging */

#ifdef DEBUG
# ifdef WIZARD
#define debug	if (wizard) pline
# else
#define debug	pline
# endif
#endif

static void FDECL(fpostfx, (struct obj *));
static void FDECL(cpostfx, (int));

char corpsename[60];
char msgbuf[BUFSZ];

/* hunger texts used on bottom line (each 8 chars long) */
#define SATIATED	0
#define NOT_HUNGRY	1
#define HUNGRY		2
#define WEAK		3
#define FAINTING	4
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

/* calculate x/y, rounding as appropriate */

static long
rounddiv(x, y)
long x, y;
{
	int divsgn = 1;
	long r, m;

	if (y == 0)
		panic("division by zero in rounddiv");
	if (x < 0) {
		divsgn = -divsgn; x = -x;
	}
	if (y < 0) {
		divsgn = -divsgn; y = -y;
	}
	r = x/y;
	m = x%y;
	if (2*m >= y)
		r++;
	return divsgn*r;
}

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
	struct	obj *tin;
	int	usedtime, reqtime;
} tin;

static struct {
	struct	obj *piece;	/* the thing being eaten, or last thing that
				 * was partially eaten, unless that thing was
				 * a tin, which uses the tin structure above */
	int	usedtime,	/* turns spent eating */
		reqtime;	/* turns required to eat */
	int	nmod;		/* coded nutrition per turn */
	Bitfield(canchoke,1);	/* was satiated at beginning */
	Bitfield(fullwarn,1);	/* have warned about being full */
	Bitfield(eating,1);	/* victual currently being eaten */
	Bitfield(doreset,1);	/* stop eating at end of turn */
} victual;

#ifndef OVERLAY
static 
#endif
int
Meatdone() {		/* called after mimicing is over */
	u.usym =
#ifdef POLYSELF
		u.mtimedone ? uasmon->mlet :
#endif
		S_HUMAN;
	prme();
	return 0;
}

/* Created by GAN 01/28/87
 * Amended by AKP 09/22/87: if not hard, don't choke, just vomit.
 * Amended by 3.  06/12/89: if not hard, sometimes choke anyway, to keep risk.
 *		  11/10/89: if hard, rarely vomit anyway, for slim chance.
 */
/*ARGSUSED*/
static void
choke(food)	/* To a full belly all food is bad. (It.) */
	register struct obj *food;
{
	/* only happens if you were satiated */
	if(u.uhs != SATIATED) return;

	if (pl_character[0] == 'K' && u.ualigntyp == U_LAWFUL)
		u.ualign--;	/* gluttony is unchivalrous */

#ifdef HARD
	if (!rn2(20)) {
#else
	if (rn2(20)) {
#endif
		You("stuff yourself and then vomit voluminously.");
		morehungry(1000);	/* you just got *very* sick! */
		vomit();
	} else {
		if(food) {
			killer = singular(food, xname);
		} else
			killer = "exuberant appetite";
		You("choke over your food.");
		You("die...");
		done(CHOKING);
	}
}

static void
recalc_wt() {	/* modify object wt. depending on time spent consuming it */
	register struct obj *piece = victual.piece;

#ifdef DEBUG
	debug("Old weight = %d", piece->owt);
	debug("Used time = %d, Req'd time = %d",
		victual.usedtime, victual.reqtime);
#endif
	/* weight(piece) = weight of full item */
  	if(victual.usedtime)
	    piece->owt = eaten_stat(weight(piece), piece);
#ifdef DEBUG
	debug("New weight = %d", piece->owt);
#endif
}

void
reset_eat() {		/* called when eating interrupted by an event */

    /* we only set a flag here - the actual reset process is done after
     * the round is spent eating.
     */
	if(victual.eating && !victual.doreset) {
#ifdef DEBUG
	    debug("reset_eat...");
#endif
	    victual.doreset = TRUE;
	}
	return;
}

static struct obj *
touchfood(otmp)
register struct obj *otmp;
{
	if (otmp->quan > 1) {
	    otmp = splitobj(otmp, (int)otmp->quan-1);
#ifdef DEBUG
	    debug("split object,");
#endif
	}
	if (!otmp->oeaten)
	    otmp->oeaten = (otmp->otyp == CORPSE ?
				(int)mons[otmp->corpsenm].cnutrit :
				objects[otmp->otyp].nutrition);
	if (carried(otmp)) {
	    freeinv(otmp);
	    if(inv_cnt() >= 52)
		dropy(otmp);
	    else
		otmp = addinv(otmp); /* unlikely but a merge is possible */
	}
	return(otmp);
}

static void
do_reset_eat() {

#ifdef DEBUG
	debug("do_reset_eat...");
#endif
	victual.piece = touchfood(victual.piece);
	recalc_wt();
	victual.fullwarn = victual.eating = victual.doreset = 
		victual.canchoke = FALSE;
	stop_occupation();
}

#ifndef OVERLAY
static 
#endif
int
eatfood() {		/* called each move during eating process */
	if(!carried(victual.piece) && !obj_here(victual.piece, u.ux, u.uy)) {
		/* maybe it was stolen? */
		do_reset_eat();
		return(0);
	}
	if(!victual.eating) return(0);
	if(victual.usedtime++ < victual.reqtime) {
	/* You can only choke if you were satiated before you started
	 * eating.
	 */
	    if(victual.canchoke && u.uhunger >= 2000) {
		choke(victual.piece);
		do_reset_eat(); /* if they were lifesaved, stop eating */
		return 0;
	    }

	    if(victual.nmod < 0)
		lesshungry(-victual.nmod);
	    else if(victual.nmod > 0 && (victual.usedtime % victual.nmod))
		lesshungry(1);

	    if(victual.doreset) do_reset_eat();
	    else {
		/* We have to calculate this every move--sometimes the thing
		 * which interrupts eating also needs to know the state of your
		 * food, so it's no good to recalculate the state of your food
		 * only after you stopped eating.  Currently, the only meal-
		 * interruptor which needs to know it is troll revivication,
		 * and it only needs to know oeaten and the weight.
		 */
		if (victual.nmod < 0)
			victual.piece->oeaten -= -victual.nmod;
		else if (victual.nmod > 0 && (victual.usedtime % victual.nmod))
			victual.piece->oeaten--;
		recalc_wt();
	    }
	    return 1;	/* still busy */
	} else {	/* done */
#ifndef NO_SIGNAL
	    victual.piece->in_use = TRUE;
#endif
	    if (nomovemsg) {
		pline(nomovemsg);
		nomovemsg = 0;
	    } else You("finish eating the %s.", singular(victual.piece, xname));

	    if(victual.piece->otyp == CORPSE)
		cpostfx(victual.piece->corpsenm);
	    else
		fpostfx(victual.piece);

	    if (carried(victual.piece)) useup(victual.piece);
	    else useupf(victual.piece);
	    victual.piece = (struct obj *) 0;
	    victual.fullwarn = victual.eating = victual.doreset = 
		victual.canchoke = FALSE;
	    return(0);
	}
}

static void
cprefx(pm)		/* called at the "first bite" of a corpse */
register int pm;
{
	if ((pl_character[0]=='E') ? is_elf(&mons[pm]) : is_human(&mons[pm])) {
		You("cannibal!	You will regret this!");
		Aggravate_monster |= INTRINSIC;
	}

	switch(pm) {
	    case PM_LITTLE_DOG:
	    case PM_DOG:
	    case PM_LARGE_DOG:
	    case PM_KITTEN:
	    case PM_HOUSECAT:
	    case PM_LARGE_CAT:
		Aggravate_monster |= INTRINSIC;
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
			Sprintf(killer, "%s meat", mons[pm].mname);
			done(STONING);
#ifdef POLYSELF
		}
		break;
	    case PM_LIZARD:
		/* Relief from cockatrices -dgk */
		if (Stoned) {
			Stoned = 0;
			You("feel limber!");
		}
		break;
	    default:

		if(dmgtype(&mons[pm], AD_ACID) && Stoned) {
		    pline("What a pity - you just destroyed a future piece of art!");
		    Stoned = 0;
		}
#endif
	}
	return;
}

static void
cpostfx(pm)		/* called after completely consuming a corpse */
register int pm;
{
	register int tmp = 0;

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
	    case PM_STALKER:
		if(!Invis) {
			HInvis = 50+rn2(100);
			if(!See_invisible)
				newsym(u.ux, u.uy);
		} else {
			if (!(HInvis & INTRINSIC)) You("feel hidden!");
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
	    case PM_GIANT_MIMIC:
		tmp += 10;
		/* fall into next case */
	    case PM_LARGE_MIMIC:
		tmp += 20;
		/* fall into next case */
	    case PM_SMALL_MIMIC:
		tmp += 20;
		if(u.usym == S_HUMAN) {
		    You("cannot resist the temptation to mimic a pile of gold.");
		    nomul(-tmp);
		    afternmv = Meatdone;
		    if (pl_character[0]=='E')
			nomovemsg = "You now again prefer mimicking an elf.";
		    else
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
	    case PM_LIZARD:
		if (HStun > 2)  make_stunned(2L,FALSE);
		if (HConfusion > 2)  make_confused(2L,FALSE);
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
	return;
}

#ifndef OVERLAY
static 
#endif
int
opentin()		/* called during each move whilst opening a tin */
{
	register int r;

	if(!carried(tin.tin) && !obj_here(tin.tin, u.ux, u.uy))
					/* perhaps it was stolen? */
		return(0);		/* %% probably we should use tinoid */
	if(tin.usedtime++ >= 50) {
		You("give up your attempt to open the tin.");
		return(0);
	}
	if(tin.usedtime < tin.reqtime)
		return(1);		/* still busy */
	if(tin.tin->cursed && !rn2(8)) {
		b_trapped("tin");
		goto use_me;
	}
	You("succeed in opening the tin.");
	if(!tin.tin->spe) {
	    if(tin.tin->corpsenm == -1) {
		pline("It turns out to be empty.");
		tin.tin->dknown = tin.tin->known = TRUE;
		goto use_me;
	    }
	    r = tin.tin->cursed ? 4 : rn2(TTSZ-1); /* Always rotten if cursed */
	    pline("It smells like %s.", makeplural(
		  Hallucination ? rndmonnam() : mons[tin.tin->corpsenm].mname));
	    pline("Eat it? ");
	    if (yn() == 'n') {
		if (!Hallucination) tin.tin->dknown = tin.tin->known = TRUE;
		if (flags.verbose) You("discard the open tin.");
		goto use_me;
	    }
	    You("consume %s %s.", tintxts[r].txt,
		  mons[tin.tin->corpsenm].mname);
	    tin.tin->dknown = tin.tin->known = TRUE;
	    cprefx(tin.tin->corpsenm); cpostfx(tin.tin->corpsenm);

	    /* check for vomiting added by GAN 01/16/87 */
	    if(tintxts[r].nut < 0) make_vomiting((long)rn1(15,10), FALSE);
	    else lesshungry(tintxts[r].nut);

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
		pline("It contains spinach.");

	    pline("Eat it? ");
	    if (yn() == 'n') {
		if (!Hallucination && !tin.tin->cursed)
		    tin.tin->dknown = tin.tin->known = TRUE;
		if (flags.verbose)
		    You("discard the open tin.");
		goto use_me;
	    }
	    if (!tin.tin->cursed)
		pline("This makes you feel like %s!",
		      Hallucination ? "Swee'pea" : "Popeye");
	    lesshungry(600);
	    gainstr(tin.tin, 0);
	}
	tin.tin->dknown = tin.tin->known = TRUE;
use_me:
	if (carried(tin.tin)) useup(tin.tin);
	else useupf(tin.tin);
	return(0);
}

static void
start_tin(otmp)		/* called when starting to open a tin */
	register struct obj *otmp;
{
	register int tmp;

	if (otmp->blessed) {
		pline("The tin opens like magic!");
		tmp = 1;
	} else if(uwep) {
		switch(uwep->otyp) {
		case TIN_OPENER:
			tmp = 1;
			break;
		case DAGGER:
#ifdef TOLKIEN
		case ELVEN_DAGGER:
		case ORCISH_DAGGER:
#endif
		case ATHAME:
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
			if (carried(otmp)) dropx(otmp);
			else stackobj(otmp);
			return;
		}
		tmp = 10 + rn2(1 + 500/((int)(ACURR(A_DEX) + ACURR(A_STR))));
	}
	tin.reqtime = tmp;
	tin.usedtime = 0;
	tin.tin = otmp;
	set_occupation(opentin, "opening the tin", 0);
	return;
}

int
Hear_again() {		/* called when waking up after fainting */
	flags.soundok = 1;
	return 0;
}

static int
rottenfood() {		/* called on the "first bite" of rotten food */

	pline("Blecch!  Rotten food!");
	if(!rn2(4)) {
		if (Hallucination) You("feel rather trippy.");
		else You("feel rather %s.", body_part(LIGHT_HEADED));
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
		return(1);
	}
	return(0);
}

static int
eatcorpse(otmp)		/* called when a corpse is selected as food */
	register struct obj *otmp;
{
	register char *cname = mons[otmp->corpsenm].mname;
	register int tp, rotted = 0;

	tp = 0;

	if(otmp->corpsenm != PM_LIZARD) {
#ifndef LINT	/* problem if more than 320K moves before try to eat */
		rotted = (monstermoves - otmp->age)/((long)(10 + rn2(20)));
#endif

		if(otmp->cursed) rotted += 2;
		else if (otmp->blessed) rotted -= 2;
	}

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
		if (carried(otmp)) useup(otmp);
		else useupf(otmp);
		return(1);
	} else if(poisonous(&mons[otmp->corpsenm]) && rn2(5)){
		tp++;
		pline("Ecch - that must have been poisonous!");
		if(!Poison_resistance) {
			losestr(rnd(4));
			losehp(rnd(15), "poisonous corpse");
		} else	You("seem unaffected by the poison.");
	/* now any corpse left too long will make you mildly ill */
	} else if(((rotted > 5) || ((rotted > 3) && rn2(5)))
#ifdef POLYSELF
		&& u.usym != S_FUNGUS
#endif
							){
		tp++;
		You("feel%s sick.", (Sick) ? " very" : "");
		losehp(rnd(8), "cadaver");
	}
	if(!tp && otmp->corpsenm != PM_LIZARD && (otmp->orotten || !rn2(7))) {

	    if(rottenfood()) {
		otmp->orotten = TRUE;
		(void)touchfood(otmp);
		return(1);
	    }
	    otmp->oeaten >>= 2;
	} else {
#ifdef POLYSELF
	    pline("That %s corpse %s!", cname,
		carnivorous(uasmon) ? "was delicious" : "tasted terrible");
#else
	    pline("That %s corpse tasted terrible!", cname);
#endif
	}

	/* delay is weight dependent */
	victual.reqtime = 3 + (mons[otmp->corpsenm].cwt >> 2);
	return(0);
}

static void
start_eating(otmp)		/* called as you start to eat */
	register struct obj *otmp;
{
#ifdef DEBUG
	debug("start_eating: %lx (victual = %lx)", otmp, victual.piece);
	debug("reqtime = %d", victual.reqtime);
	debug("(original reqtime = %d)", objects[otmp->otyp].oc_delay);
	debug("nmod = %d", victual.nmod);
	debug("oeaten = %d", otmp->oeaten);
#endif
	victual.fullwarn = victual.doreset = FALSE;
	victual.canchoke = (u.uhs == SATIATED);
	victual.eating = TRUE;

	if (otmp->otyp == CORPSE)
	    cprefx(victual.piece->corpsenm);

	Sprintf(msgbuf, "eating the %s", singular(otmp, xname));
	set_occupation(eatfood, msgbuf, 0);
}


static void
fprefx(otmp)		/* called on "first bite" of (non-corpse) food */

	register struct obj *otmp;
{
	switch(otmp->otyp) {

	    case FOOD_RATION:
		if(u.uhunger <= 200)
		    if (Hallucination) pline("Oh wow, like, superior, man!");
		    else	       pline("That food really hit the spot!");
		else if(u.uhunger <= 700) pline("That satiated your stomach!");
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
			&& !carnivorous(uasmon)
#endif
						) {
			make_vomiting((long)rn1(victual.reqtime, 10), FALSE);
		}
		break;
#ifdef POLYSELF
	    case CLOVE_OF_GARLIC:
		if (is_undead(uasmon)) {
			make_vomiting((long)rn1(victual.reqtime, 5), FALSE);
			break;
		}
		/* Fall through otherwise */
#endif
	    default:
#ifdef TUTTI_FRUTTI
		if (otmp->otyp==SLIME_MOLD && !otmp->cursed
			&& otmp->spe == current_fruit)
		    pline(!Hallucination ? "Mmm!  Your favorite!" :
			    		   "Yum!  Your fave fruit!");
		else
#endif
#ifdef UNIX
		if (otmp->otyp == APPLE || otmp->otyp == PEAR) {
		    if (!Hallucination) pline("Core dumped.");
		    else {
/* This is based on an old Usenet joke, a fake a.out manual page */
			int x = rnd(100);
			if (x <= 75)
			    pline("Segmentation fault -- core dumped.");
			else if (x <= 99)
			    pline("Bus error -- core dumped.");
			else pline("Yo' mama -- core dumped.");
		    }
		} else 
#endif
		{
		    int oldquan = otmp->quan;
		    otmp->quan = 1;
		    pline("This %s is %s!", xname(otmp),
		      otmp->cursed ? (Hallucination ? "grody" : "terrible"):
		      Hallucination ? "gnarly" : (
#ifdef TOLKIEN
		       otmp->otyp==CRAM_RATION ? "bland":
#endif
		       "delicious"));
		    otmp->quan = oldquan;
		}
		break;
	}
}

static void
fpostfx(otmp)		/* called after consuming (non-corpse) food */

	register struct obj *otmp;
{
	switch(otmp->otyp) {
#ifdef POLYSELF
	    case CLOVE_OF_GARLIC:
		if (u.ulycn != -1) {
		    u.ulycn = -1;
		    You("feel purified.");
		    if(uasmon == &mons[u.ulycn] && !Polymorph_control)
			rehumanize();
		}
		break;
#endif
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
	}
	return;
}

int
doeat() {		/* generic "eat" command funtion (see cmd.c) */
	register struct obj *otmp;
	int basenutrit; 		/* nutrition of full item */

	if (!(otmp = floorfood("eat", 0))) return 0;

	if(otmp == victual.piece) {
	    You("resume your meal.");
	    if(!carried(victual.piece)) {
		if(victual.piece->quan != 1)
			(void) splitobj(victual.piece, 1);
	    }
	    start_eating(victual.piece);
	    return(1);
	}

	/* nothing in progress - so try to find something. */
	/* tins are a special case */
	if(otmp->otyp == TIN) {
	    start_tin(otmp);
	    return(1);
	}

	victual.piece = otmp = touchfood(otmp);
	victual.usedtime = 0;

	/* Now we need to calculate delay and nutritional info.
	 * The base nutrition calculated here and in eatcorpse() accounts
	 * for normal vs. rotten food.  The reqtime and nutrit values are
	 * then adjusted in accordance with the amount of food left.
	 */
	if(otmp->otyp == CORPSE) {
	    if(eatcorpse(otmp)) return(1);
	    /* else eatcorpse sets up reqtime and oeaten */
	} else {
	    victual.reqtime = objects[otmp->otyp].oc_delay;
	    if (otmp->otyp != FORTUNE_COOKIE &&
		(otmp->cursed ||
		 ((monstermoves - otmp->age) > otmp->blessed ? 50 : 30)) &&
		(otmp->orotten || !rn2(7))) {

		if(rottenfood()) {
		    otmp->orotten = TRUE;
		    return(1);
		}
		otmp->oeaten /= 2;
	    } else fprefx(otmp);
	}

	/* re-calc the nutrition */
	if (otmp->otyp == CORPSE) basenutrit = mons[otmp->corpsenm].cnutrit;
	else basenutrit = objects[otmp->otyp].nutrition;

#ifdef DEBUG
	debug("before rounddiv: victual.reqtime == %d", victual.reqtime);
	debug("oeaten == %d, basenutrit == %d", otmp->oeaten, basenutrit);
#endif
	victual.reqtime = (basenutrit == 0 ? 0 :
	  (int)rounddiv(victual.reqtime * (long)otmp->oeaten,(long)basenutrit));
#ifdef DEBUG
	debug("after rounddiv: victual.reqtime == %d", victual.reqtime);
#endif
	/* calculate the modulo value (nutrit. units per round eating)
	 * note: this isn't exact - you actually lose a little nutrition
	 *	 due to this method.
	 * TODO: add in a "remainder" value to be given at the end of the
	 *	 meal.
	 */
	if(victual.reqtime == 0)
	    /* possible if most has been eaten before */
	    victual.nmod = 0;
	else if (otmp->oeaten > victual.reqtime)
	    victual.nmod = -(otmp->oeaten / victual.reqtime);
	else
	    victual.nmod = victual.reqtime % otmp->oeaten;

	start_eating(otmp);
	return(1);
}

void
gethungry() {		/* as time goes by - called in main.c */
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


void
morehungry(num)	/* called after vomiting and after performing feats of magic */
register int num;
{
	u.uhunger -= num;
	newuhs(TRUE);
}


void
lesshungry(num)	/* called after eating (and after drinking fruit juice) */
register int num;
{
#ifdef DEBUG
	debug("lesshungry(%d)", num);
#endif
	u.uhunger += num;
	if(u.uhunger >= 2000) {
	    if (!victual.eating || victual.canchoke) {
		choke((struct obj *) 0);
		reset_eat(); /* stop eating if they were lifesaved */
	    }
	} else {
	    /* Have lesshungry() report when you're nearly full so all eating
	     * warns when you're about to choke.
	     */
	    if (u.uhunger >= 1500) {
	      if(!victual.eating || (victual.eating && !victual.fullwarn)) {
		pline("You're having a hard time getting all of it down.");
		nomovemsg = "You're finally finished.";
		if(!victual.eating)
		    multi = -2;
		else {
		    victual.fullwarn = TRUE;
		    if (victual.canchoke) {
			pline("Stop eating? ");
			if(yn() == 'y') reset_eat();
		    }
		}
	      }
	    }
	}
	newuhs(FALSE);
}

#ifndef OVERLAY
static 
#endif
int
unfaint() {
	(void) Hear_again();
	u.uhs = FAINTING;
	flags.botl = 1;
	return 0;
}

void
newuhs(incr)		/* compute and comment on your (new?) hunger status */
	boolean incr;
{
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
			killer = "starvation";
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

/* Returns an object representing food.  Object may be either on floor or
 * in inventory.
 */
struct obj *
floorfood(verb,corpseonly)	/* get food from floor or pack */
	char *verb;
	boolean corpseonly;
{
	register struct obj *otmp;

	/* Is there some food (probably a heavy corpse) here on the ground? */
	if(!Levitation && !u.uswallow) {
	    for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
		if(corpseonly ? otmp->otyp==CORPSE : otmp->olet==FOOD_SYM) {
			pline("There %s %s here; %s %s? ",
				(otmp->quan == 1) ? "is" : "are",
				doname(otmp), verb,
				(otmp->quan == 1) ? "it" : "one");
			if(yn() == 'y')
				return(otmp);
		}
	    }
	}
	return getobj(comestibles, verb);
}

/* Side effects of vomiting */
/* added nomul (MRS) - it makes sense, you're too busy being sick! */
/* TO DO: regurgitate swallowed monsters when poly'd */
void
vomit() {		/* A good idea from David Neves */
	make_sick(0L,TRUE);
	nomul(-2);
}

int
eaten_stat(base, obj)
register int base;
register struct obj *obj;
{
	base *= obj->oeaten;

	if (obj->otyp == CORPSE) base /= mons[obj->corpsenm].cnutrit;
	else base /= objects[obj->otyp].nutrition;
	return (base < 1) ? 1 : base;
}
