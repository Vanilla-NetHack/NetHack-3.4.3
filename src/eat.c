/*      SCCS Id: @(#)eat.c      3.0     89/11/21
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

STATIC_PTR int NDECL(Meatdone);
STATIC_PTR int NDECL(eatfood);
STATIC_PTR int NDECL(opentin);
STATIC_PTR int NDECL(unfaint);

#ifdef OVLB
static int FDECL(rounddiv, (long, int));
static void FDECL(choke, (struct obj *));
static void NDECL(recalc_wt);
static struct obj *FDECL(touchfood, (struct obj *));
static void NDECL(do_reset_eat);
static void FDECL(done_eating, (BOOLEAN_P));
static void FDECL(cprefx, (int));
static void FDECL(cpostfx, (int));
static void FDECL(start_tin, (struct obj *));
static int FDECL(eatcorpse, (struct obj *));
static void FDECL(start_eating, (struct obj *));
static void FDECL(fprefx, (struct obj *));
static void FDECL(fpostfx, (struct obj *));
static int NDECL(bite);

#ifdef POLYSELF
static int FDECL(rottenfood, (struct obj *));
static void NDECL(eatspecial);
static const char * FDECL(foodword, (struct obj *));
#else
static int NDECL(rottenfood);
#endif /* POLYSELF */

char corpsename[60];
char msgbuf[BUFSZ];

#endif /* OVLB */

/* hunger texts used on bottom line (each 8 chars long) */
#define SATIATED	0
#define NOT_HUNGRY	1
#define HUNGRY		2
#define WEAK		3
#define FAINTING	4
#define FAINTED		5
#define STARVED		6

#ifdef OVLB

const char *hu_stat[] = {
	"Satiated",
	"        ",
	"Hungry  ",
	"Weak    ",
	"Fainting",
	"Fainted ",
	"Starved "
};

#endif /* OVLB */

#ifndef OVLB

STATIC_DCL const char NEARDATA comestibles[];

#else

STATIC_OVL const char NEARDATA comestibles[] = { FOOD_SYM, 0 };
#ifdef POLYSELF
STATIC_OVL const char NEARDATA everything[] = { GOLD_SYM, /* must come first */
	WEAPON_SYM, ARMOR_SYM, POTION_SYM, SCROLL_SYM, WAND_SYM,
# ifdef SPELLS
	SPBOOK_SYM,
# endif
	RING_SYM, WAND_SYM, AMULET_SYM, FOOD_SYM, TOOL_SYM, GEM_SYM,
	ROCK_SYM, BALL_SYM, CHAIN_SYM, 0 };

#endif /* POLYSELF */
#endif /* OVLB */
#ifdef OVL1
# ifdef POLYSELF

boolean
is_edible(obj)
register struct obj *obj;
{
	if (metallivorous(uasmon) && (obj->olet == GOLD_SYM ||
			(objects[obj->otyp].oc_material > WOOD &&
			objects[obj->otyp].oc_material < MINERAL)))
		return TRUE;
	if (u.umonnum == PM_GELATINOUS_CUBE &&
			objects[obj->otyp].oc_material <= WOOD)
		return TRUE;
	return !!index(comestibles, obj->olet);
}
# endif /* POLYSELF */
#endif /* OVL1 */
#ifdef OVLB

/* calculate x/y, rounding as appropriate */

static int
rounddiv(x, y)
long x;
int y;
{
	int divsgn = 1;
	int r, m;

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

const struct { const char *txt; int nut; } tintxts[] = {
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
} NEARDATA tin;

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
} NEARDATA victual;

STATIC_PTR
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
		killer_format = KILLED_BY_AN;
		if(food) {
#ifdef POLYSELF
			if (food->olet == GOLD_SYM) {
				killer_format = KILLED_BY;
				killer = "eating too rich a meal";
			} else
#endif
				killer = singular(food, xname);
		} else
			killer = "exuberant appetite";
		if (!food) You("choke over it.");
#ifdef POLYSELF
		else You("choke over your %s.", foodword(food));
#else
		else You("choke over your food.");
#endif
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

/* When food decays, in the middle of your meal, we don't want to dereference
 * any dangling pointers, so set it to null (which should still trigger
 * do_reset_eat() at the beginning of eatfood()) and check for null pointers
 * in do_reset_eat().
 */
void
food_disappears(obj)
register struct obj *obj;
{
	if (obj == victual.piece) victual.piece = (struct obj *)0;
}

static void
do_reset_eat() {

#ifdef DEBUG
	debug("do_reset_eat...");
#endif
	if (victual.piece) {
		victual.piece = touchfood(victual.piece);
		recalc_wt();
	}
	victual.fullwarn = victual.eating = victual.doreset = FALSE;
	/* Do not set canchoke to FALSE; if we continue eating the same object
	 * we need to know if canchoke was set when they started eating it the
	 * previous time.  And if we don't continue eating the same object
	 * canchoke always gets recalculated anyway.
	 */
	stop_occupation();
}

STATIC_PTR
int
eatfood() {		/* called each move during eating process */
	if(!carried(victual.piece) && !obj_here(victual.piece, u.ux, u.uy)) {
		/* maybe it was stolen? */
		do_reset_eat();
		return(0);
	}
	if(!victual.eating) return(0);

	if(++victual.usedtime < victual.reqtime) {
	    if(bite()) return(0);
	    return(1);	/* still busy */
	} else {	/* done */
	    done_eating(TRUE);
	    return(0);
	}
}

static void
done_eating(message)
boolean message;
{
#ifndef NO_SIGNAL
	victual.piece->in_use = TRUE;
#endif
	if (nomovemsg) {
		if (message) pline(nomovemsg);
		nomovemsg = 0;
	} else if (message)
		You("finish eating the %s.", singular(victual.piece, xname));

	if(victual.piece->otyp == CORPSE)
		cpostfx(victual.piece->corpsenm);
	else
		fpostfx(victual.piece);

	if (carried(victual.piece)) useup(victual.piece);
	else useupf(victual.piece);
	victual.piece = (struct obj *) 0;
	victual.fullwarn = victual.eating = victual.doreset = FALSE;
}

static void
cprefx(pm)		/* called at the "first bite" of a corpse */
register int pm;
{
	if ((pl_character[0]=='E') ? is_elf(&mons[pm]) : is_human(&mons[pm])) {
		You("cannibal!  You will regret this!");
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
		if(!resists_ston(uasmon))
#endif
			{
			char *cruft;	/* killer is const char * */
			killer_format = KILLED_BY;
			killer = cruft = (char *) alloc(40);
			You("turn to stone.");
			Sprintf(cruft, "%s meat", mons[pm].mname);
			done(STONING);
			}
			break;
#ifdef POLYSELF
	    case PM_LIZARD:
		/* Relief from cockatrices -dgk */
		if (Stoned) {
			Stoned = 0;
			You("feel limber!");
		}
		break;
	    default:
		if(acidic(&mons[pm]) && Stoned) {
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
	    case PM_CHAMELEON:
		You("feel a change coming over you.");
#ifdef POLYSELF
		polyself();
#else
		newman();
#endif
		break;
	    default: {
		register struct permonst *ptr = &mons[pm];
		if(dmgtype(ptr, AD_STUN) || ptr==&mons[PM_VIOLET_FUNGUS]) {
		    pline ("Oh wow!  Great stuff!");
		    make_hallucinated(Hallucination + 200,FALSE);
		}
		/* prevent polymorph abuse by killing/eating your offspring */
		if(ptr >= &mons[PM_BABY_GRAY_DRAGON] &&
		   ptr <= &mons[PM_BABY_YELLOW_DRAGON]) return;
		if(is_giant(ptr)) gainstr((struct obj *)0, 0);

		if(can_teleport(ptr) && ptr->mlevel > rn2(10)) {
		    if (!(HTeleportation & INTRINSIC)) {
			You("feel very jumpy.");
			HTeleportation |= INTRINSIC;
		    }
		} else if(control_teleport(ptr) && ptr->mlevel > rn2(15)) {
		    if (!(HTeleport_control & INTRINSIC)) {
			You("feel in control of yourself.");
			HTeleport_control |= INTRINSIC;
		    }
		} else if(resists_fire(ptr) && ptr->mlevel > rn2(15)) {
		    if (!(HFire_resistance & INTRINSIC)) {
			You("feel a momentary chill.");
			HFire_resistance |= INTRINSIC;
		    }
		} else if(resists_cold(ptr) && ptr->mlevel > rn2(15)) {
		    if (!(HCold_resistance & INTRINSIC)) {
			You("feel full of hot air.");
			HCold_resistance |= INTRINSIC;
		    }
		} else if(((ptr->mflags1 & M1_POIS_RES) && ptr->mlevel>rn2(15))
		   || ((pm == PM_KILLER_BEE || pm == PM_SCORPION) && !rn2(4))) {
		/* Monsters with only M1_POIS are poison resistant themselves,
		 * but do not confer resistance when eaten
		 */
		    if (!(HPoison_resistance & INTRINSIC)) {
			You("feel healthy.");
			HPoison_resistance |= INTRINSIC;
		    }
		} else if(resists_elec(ptr) && ptr->mlevel > rn2(15)) {
		    if (!(HShock_resistance & INTRINSIC)) {
			Your("health currently feels amplified!");
			HShock_resistance |= INTRINSIC;
		    }
		} else if((ptr->mflags1 & M1_SLEE_RES) && ptr->mlevel > rn2(15)) {
		/* Undead monsters never sleep,
		 * but also do not confer resistance when eaten
		 */
		    if (!(HSleep_resistance & INTRINSIC)) {
			You("feel wide awake.");
			HSleep_resistance |= INTRINSIC;
		    }
		} else if(resists_disint(ptr) && ptr->mlevel > rn2(15)) {
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

STATIC_PTR
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
	if(tin.tin->cursed && tin.tin->spe != -1 && !rn2(8)) {
		b_trapped("tin");
		goto use_me;
	}
	You("succeed in opening the tin.");
	if(tin.tin->spe != 1) {
	    if(tin.tin->corpsenm == -1) {
		pline("It turns out to be empty.");
		tin.tin->dknown = tin.tin->known = TRUE;
		goto use_me;
	    }
	    r = tin.tin->cursed ? 4 : rn2(TTSZ-1); /* Always rotten if cursed */
#ifdef MACOS
	{
		char tmp[128];
		if(!flags.silent) SysBeep(20);
		Sprintf(tmp, "It smells like %s. Eat it ?", makeplural(
		  Hallucination ? rndmonnam() : mons[tin.tin->corpsenm].mname));
		if(UseMacAlertText(128, tmp) == 2) {
#else
	    pline("It smells like %s.", makeplural(
		  Hallucination ? rndmonnam() : mons[tin.tin->corpsenm].mname));
	    pline("Eat it? ");
	    if (yn() == 'n') {
#endif
		if (!Hallucination) tin.tin->dknown = tin.tin->known = TRUE;
		if (flags.verbose) You("discard the open tin.");
		goto use_me;
	    }
#ifdef MACOS
	}
#endif
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
#ifdef MACOS
	{
		char tmp[128];
		if(!flags.silent) SysBeep(20);
	    if (tin.tin->cursed)
		Sprintf(tmp, "It contains some decaying %s substance. Eat it ?",
			Hallucination ? hcolor() : green);
	    else
		Sprintf(tmp, "It contains spinach. Eat it ?");
		if(UseMacAlertText(128, tmp) == 2) {
#else
	    if (tin.tin->cursed)
		pline("It contains some decaying %s substance.",
			Hallucination ? hcolor() : green);
	    else
		pline("It contains spinach.");

	    pline("Eat it? ");
	    if (yn() == 'n') {
#endif
		if (!Hallucination && !tin.tin->cursed)
		    tin.tin->dknown = tin.tin->known = TRUE;
		if (flags.verbose)
		    You("discard the open tin.");
		goto use_me;
	    }
#ifdef MACOS
	}
#endif
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

#ifdef POLYSELF
	if (metallivorous(uasmon)) {
		You("bite right into the metal can....");
		tmp = 1;
	} else
#endif
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
#ifdef POLYSELF
rottenfood(obj)
struct obj *obj;
#else
rottenfood()
#endif
{		/* called on the "first bite" of rotten food */
#ifdef POLYSELF
	pline("Blecch!  Rotten %s!", foodword(obj));
#else
	pline("Blecch!  Rotten food!");
#endif
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
	register const char *cname = mons[otmp->corpsenm].mname;
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
			u.usick_cause = (const char *)corpsename;
			flags.botl = 1;
#ifdef POLYSELF
		}
#endif
		if (carried(otmp)) useup(otmp);
		else useupf(otmp);
		return(1);
	} else if(acidic(&mons[otmp->corpsenm])
#ifdef POLYSELF
		  && !resists_acid(uasmon)
#endif
		 ) {
		tp++;
		You("have a very bad case of stomach acid.");
		losehp(rnd(15), "acidic corpse", KILLED_BY_AN);
	} else if(poisonous(&mons[otmp->corpsenm]) && rn2(5)) {
		tp++;
		pline("Ecch - that must have been poisonous!");
		if(!Poison_resistance) {
			losestr(rnd(4));
			losehp(rnd(15), "poisonous corpse", KILLED_BY_AN);
		} else	You("seem unaffected by the poison.");
	/* now any corpse left too long will make you mildly ill */
	} else if(((rotted > 5) || ((rotted > 3) && rn2(5)))
#ifdef POLYSELF
		&& u.usym != S_FUNGUS
#endif
							){
		tp++;
		You("feel%s sick.", (Sick) ? " very" : "");
		losehp(rnd(8), "cadaver", KILLED_BY_AN);
	}
	if(!tp && otmp->corpsenm != PM_LIZARD && (otmp->orotten || !rn2(7))) {
#ifdef POLYSELF
	    if(rottenfood(otmp)) {
#else
	    if(rottenfood()) {
#endif
		otmp->orotten = TRUE;
		(void)touchfood(otmp);
		return(1);
	    }
	    otmp->oeaten >>= 2;
	} else {
#ifdef POLYSELF
	    pline("This %s corpse %s!", cname,
		carnivorous(uasmon) ? "is delicious" : "tastes terrible");
#else
	    pline("This %s corpse tastes terrible!", cname);
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
	victual.eating = TRUE;

	if (otmp->otyp == CORPSE)
	    cprefx(victual.piece->corpsenm);

	if (bite()) return;

	if(++victual.usedtime >= victual.reqtime) {
	    done_eating(FALSE);
	    return;
	}

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
		    unsigned oldquan = otmp->quan;
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

#ifdef POLYSELF
static void
eatspecial() /* called after eating non-food */
{
	register struct obj *otmp = victual.piece;

	lesshungry(victual.nmod);
	victual.piece = (struct obj *)0;
	victual.eating = 0;
	if (otmp->olet == GOLD_SYM) { /* temporary gold object */
		free ((genericptr_t)otmp);
		return;
	}
	if (otmp->olet == POTION_SYM) {
		otmp->quan++; /* dopotion() does a useup() */
#ifdef MACOS
		segments |= SEG_EAT;
#endif
		(void)dopotion(otmp);
	}
	if (otmp == uball) unpunish();
	if (otmp == uchain) unpunish(); /* but no useup() */
	else if (carried(otmp)) useup(otmp);
	else useupf(otmp);
}

static const char *
foodword(otmp)
register struct obj *otmp;
{
	if (otmp->olet == FOOD_SYM) return "food";
	if (otmp->olet == GOLD_SYM) return "gold";
	if (objects[otmp->otyp].oc_material == GLASS) {
		if (otmp->olet == GEM_SYM && otmp->dknown)
			makeknown(otmp->otyp);
		return "glass";
	}
	if (objects[otmp->otyp].oc_material > WOOD &&
			objects[otmp->otyp].oc_material < MINERAL)
		return "metal";
	if (objects[otmp->otyp].oc_material == MINERAL)
		return (otmp->otyp <= LAST_GEM && otmp->olet == GEM_SYM)
			? "rich food" : "stone";
	if (objects[otmp->otyp].oc_material == WOOD) return "wood";
	if (otmp->olet == ARMOR_SYM) return "armor";
	return "stuff";
}
#endif

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
			killer_format = KILLED_BY_AN;
			killer = "rotten lump of royal jelly";
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
			killer_format = KILLED_BY_AN;
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

	if (Strangled) {
		pline("If you can't breathe air, how can you consume solids?");
		return 0;
	}
	if (!(otmp = floorfood("eat", 0))) return 0;
#ifdef POLYSELF
	/* We have to make non-foods take no time to eat, unless we want to
	 * do ridiculous amounts of coding to deal with partly eaten plate
	 * mails, players who polymorph back to human in the middle of their
	 * metallic meal, etc....
	 */
	if (!is_edible(otmp)) {
	    You("cannot eat that!");
	    if (otmp->olet == GOLD_SYM) { /* temp gold object */
		if (otmp->ox) mkgold(OGOLD(otmp), u.ux, u.uy);
		else u.ugold += OGOLD(otmp);
		free((genericptr_t) otmp);
	    }
	    return 0;
	}
	if (otmp->olet != FOOD_SYM) {
	    victual.reqtime = 1;
	    victual.piece = otmp;
		/* Don't split it, we don't need to if it's 1 move */
	    victual.usedtime = 0;
	    victual.canchoke = (u.uhs == SATIATED);
	    if (otmp->olet == GOLD_SYM)
		basenutrit = ((OGOLD(otmp) > 5000L) ? 5000 : (int)OGOLD(otmp));
	    else basenutrit = otmp->owt * 10 / otmp->quan;
	    victual.nmod = basenutrit;
	    victual.eating = TRUE; /* needed for lesshungry() */
		
	    if (otmp->cursed) 
		(void) rottenfood(otmp);

	    if (otmp->olet == WEAPON_SYM && otmp->opoisoned) {
		pline("Ecch - that must have been poisonous!");
		if(!Poison_resistance) {
		    losestr(rnd(4));
		    losehp(rnd(15), xname(otmp), KILLED_BY_AN);
		} else
		    You("seem unaffected by the poison.");
	    } else if (!otmp->cursed)
		pline("This %s is delicious!",
		    otmp->olet == GOLD_SYM ? "gold" : xname(otmp));
	    eatspecial();
	    return 1;
	}
#endif

	if(otmp == victual.piece) {
	/* If they weren't able to choke, they don't suddenly become able to
	 * choke just because they were interrupted.  On the other hand, if
	 * they were able to choke before, if they lost food it's possible
	 * they shouldn't be able to choke now.
	 */
	    if (u.uhs != SATIATED) victual.canchoke = FALSE;
	    if(!carried(victual.piece)) {
		if(victual.piece->quan > 1)
			(void) splitobj(victual.piece, 1);
	    }
	    You("resume your meal.");
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
		 (((monstermoves - otmp->age) > otmp->blessed ? 50 : 30) &&
		(otmp->orotten || !rn2(7))))) {

#ifdef POLYSELF
		if(rottenfood(otmp)) {
#else
  		if(rottenfood()) {
#endif
		    otmp->orotten = TRUE;
		    return(1);
		}
		otmp->oeaten >>= 1;
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
		rounddiv(victual.reqtime * (long)otmp->oeaten, basenutrit));
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
	victual.canchoke = (u.uhs == SATIATED);

	start_eating(otmp);
	return(1);
}

/* Take a single bite from a piece of food, checking for choking and
 * modifying usedtime.  Returns 1 if they choked and survived, 0 otherwise.
 */
static int
bite()
{
	if(victual.canchoke && u.uhunger >= 2000) {
		choke(victual.piece);
		return 1;
	}
	if (victual.doreset) {
		do_reset_eat();
		return 0;
	}
	if(victual.nmod < 0) {
		lesshungry(-victual.nmod);
		victual.piece->oeaten -= -victual.nmod;
	} else if(victual.nmod > 0 && (victual.usedtime % victual.nmod)) {
		lesshungry(1);
		victual.piece->oeaten--;
	}
	recalc_wt();
	return 0;
}

#endif /* OVLB */
#ifdef OVL0

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

#endif /* OVL0 */
#ifdef OVLB

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
	    if (!victual.eating || victual.canchoke)
		if (victual.eating) {
			choke(victual.piece);
			reset_eat();
		} else
			choke((struct obj *) 0);
			/* no reset_eat(); it was a non-food such as juice */
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
#ifdef MACOS
			if(!flags.silent) SysBeep(20);
			if(UseMacAlertText(128, "Stop eating ?") == 1)
#else
			pline("Stop eating? ");
			if(yn() == 'y')
#endif
				reset_eat();
		    }
		}
	      }
	    }
	}
	newuhs(FALSE);
}

STATIC_PTR
int
unfaint() {
	(void) Hear_again();
	u.uhs = FAINTING;
	flags.botl = 1;
	return 0;
}

#endif /* OVLB */
#ifdef OVL0

boolean
is_fainted() {
	return(u.uhs == FAINTED);
}

void
reset_faint() {	/* call when a faint must be prematurely terminated */
	if(is_fainted()) nomul(0);
}

void
sync_hunger() {

	if(is_fainted()) {

		flags.soundok = 0;
		nomul(-10+(u.uhunger/10));
		nomovemsg = "You regain consciousness.";
		afternmv = unfaint;
	}
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
		if(is_fainted()) newhs = FAINTED;
		if(u.uhs <= WEAK || rn2(20-u.uhunger/10) >= 19) {
			if(!is_fainted() && multi >= 0 /* %% */) {
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
			killer_format = KILLED_BY;
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
			killer_format = KILLED_BY;
			killer = "exhaustion";
			done(STARVING);
		}
	}
}

#endif /* OVL0 */
#ifdef OVLB

/* Returns an object representing food.  Object may be either on floor or
 * in inventory.
 */
struct obj *
floorfood(verb,corpseonly)	/* get food from floor or pack */
	const char *verb;
	boolean corpseonly;
{
	register struct obj *otmp;
#ifdef POLYSELF
	struct gold *gold = g_at(u.ux, u.uy);
	boolean feeding = (!strcmp(verb, "eat"));
#endif
#ifdef POLYSELF
	if (feeding && gold && metallivorous(uasmon)) {
#ifdef MACOS
		char tmp[128];
	    if (gold->amount == 1)
		Sprintf(tmp, "There is 1 gold piece here. Eat it ?");
	    else Sprintf(tmp, "There are %ld gold pieces here. Eat them ?",
								gold->amount);
		if(UseMacAlertText(128, tmp) == 1) {
#else
	    if (gold->amount == 1)
		pline("There is 1 gold piece here; eat it? ");
	    else pline("There are %ld gold pieces here; eat them? ",
								gold->amount);
	    if (yn() == 'y') {
#endif
		otmp = newobj(0);
		otmp->olet = GOLD_SYM;
		otmp->ox = u.ux;
		otmp->oy = u.uy;
		OGOLD(otmp) = gold->amount;
		freegold(gold);
		return otmp;
	    }
	}
#endif
	/* Is there some food (probably a heavy corpse) here on the ground? */
	if(!Levitation && !u.uswallow) {
	    for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
		if(corpseonly ? otmp->otyp==CORPSE : 
#ifdef POLYSELF
		    feeding ? is_edible(otmp) :
#endif
						otmp->olet==FOOD_SYM) {
#ifdef MACOS
			if(!flags.silent) SysBeep(20);
		{
			char tmp[128];
			Sprintf(tmp, "There %s %s here. %s %s ?",
				(otmp->quan == 1) ? "is" : "are",
				doname(otmp), verb,
				(otmp->quan == 1) ? "it" : "one");
			if(UseMacAlertText(128, tmp) == 1)
#else
			pline("There %s %s here; %s %s? ",
				(otmp->quan == 1) ? "is" : "are",
				doname(otmp), verb,
				(otmp->quan == 1) ? "it" : "one");
			if(yn() == 'y')
#endif
				return(otmp);
#ifdef MACOS
		}
#endif
		}
	    }
	}
#ifdef POLYSELF
	/* We cannot use "#" since that causes getobj() to skip its
	 * "ugly checks" and we need to check for inedible items.
	 */
	return getobj (feeding ? (const char *)everything :
				 (const char *)comestibles, verb);
#else
	return getobj(comestibles, verb);
#endif
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

	if (obj->otyp == CORPSE) 
	    base = mons[obj->corpsenm].cnutrit ?
				base / mons[obj->corpsenm].cnutrit : 0;
	else base = objects[obj->otyp].nutrition ?
				base / objects[obj->otyp].nutrition : 0;
	return (base < 1) ? 1 : base;
}

#endif /* OVLB */
