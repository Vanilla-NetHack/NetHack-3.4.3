/*	SCCS Id: @(#)polyself.c	3.0	88/10/22
/* Polymorph self routine.  Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef POLYSELF
static void break_armor(), drop_weapon();
static void skinback();
static void uunstick();
static boolean sticky;
#endif

void
newman()
{
	int tmp, tmp2;
	char buf[BUFSZ];

	if (!rn2(10)) {
		flags.female = !flags.female;
		max_rank_sz();
		if (pl_character[0]=='P')
			Strcpy(pl_character+6, flags.female?"ess":"");
		if (pl_character[0]=='C')
			Strcpy(pl_character+5, flags.female ? "woman" : "man");
	}
#ifdef POLYSELF
	if (u.umonnum != -1) {
		u.acurr = u.macurr;	/* restore old attribs */
		u.amax = u.mamax;
	}
	u.usym = S_HUMAN;
	u.umonnum = -1;
	if (u.uundetected) u.uundetected = 0;
	prme();
	u.mtimedone = u.mh = u.mhmax = 0;
#endif
	tmp = u.uhpmax;
	tmp2 = u.ulevel;
	u.ulevel = u.ulevel-2+rn2(5);
	if (u.ulevel > 127 || u.ulevel == 0) u.ulevel = 1;
	if (u.ulevel > MAXULEV) u.ulevel = MAXULEV;

	for(tmp = u.ulevel; tmp != tmp2; tmp += (tmp2 < u.ulevel) ? -1 : 1)
		adjabil((tmp2 > u.ulevel) ? -1 : 1);
	tmp = u.uhpmax;

	/* random experience points for the new experience level */
	u.uexp = rndexp();
#ifndef LINT
	u.uhpmax = (u.uhpmax-10)*(long)u.ulevel/tmp2 + 19 - rn2(19);
#endif
/* If it was u.uhpmax*u.ulevel/tmp+9-rn2(19), then a 1st level character
   with 16 hp who polymorphed into a 3rd level one would have an average
   of 48 hp.  */
#ifndef LINT
	u.uhp = u.uhp * (long)u.uhpmax/tmp;
#endif
#ifdef SPELLS
	tmp = u.uenmax;
#  ifndef LINT
	u.uenmax = u.uenmax * (long)u.ulevel/tmp2 + 9 - rn2(19);
#  endif
	if (u.uenmax < 0) u.uenmax = 0;
#  ifndef LINT
	u.uen = (tmp ? u.uen * (long)u.uenmax / tmp : u.uenmax);
#  endif
#endif
	redist_attr();
	u.uhunger = rn1(500,500);
	Sick = 0;
	Stoned = 0;
	if (u.uhp <= 0 || u.uhpmax <= 0) {
#ifdef POLYSELF
		if(Polymorph_control) {
		    if (u.uhp <= 0) u.uhp = 1;
		    if (u.uhpmax <= 0) u.uhpmax = 1;
		} else {
#endif
		    Your("new form doesn't seem healthy enough to survive.");
		    killer="unsuccessful polymorph";
		    done(DIED);
#ifdef POLYSELF
		}
#endif
	}
#ifdef POLYSELF
	set_uasmon();
#endif
	You("feel like a new %sman!", flags.female ? "wo" : "");
#ifdef WIZARD
	if(!wizard) {
#endif
newname:	more();
		do {
		    pline("What is your new name? ");
		    getlin(buf);
		} while (buf[0]=='\033' || buf[0]==0);
		if (!strcmp(plname,buf)) {
		    pline("That is the same as your old name!");
		    goto newname;
		}
		(void)strncpy(plname, buf, sizeof(plname)-1);
		Sprintf(SAVEF, "save/%d%s", getuid(), plname);
		regularize(SAVEF+5);		/* avoid . or / in name */
#ifdef WIZARD
	}
#endif
	flags.botl = 1;
#ifdef POLYSELF
	skinback();
	find_ac();
	if (sticky) uunstick();
#endif
}

#ifdef POLYSELF
void
polyself()
{
	char buf[BUFSZ];
	int mntmp = -1;
	int tries=0;
	boolean draconian = (uarm && uarm->otyp==DRAGON_SCALE_MAIL &&
		uarm->corpsenm >= PM_GREY_DRAGON &&
		uarm->corpsenm <= PM_YELLOW_DRAGON);
	boolean iswere = (u.ulycn > -1 || is_were(uasmon));
	boolean isvamp = (u.usym == S_VAMPIRE || u.umonnum == PM_VAMPIRE_BAT);
	/* We have to calculate sticky in multiple places since we might go
	 * through any one of them without going through the others.
	 */
	sticky = sticks(uasmon) && u.ustuck && !u.uswallow;

	if(!Polymorph_control && !draconian && !iswere && !isvamp) {
	    if (rn2(20) > ACURR(A_CON)) {
		You("shudder for a moment.");
		losehp(rn2(30),"system shock");
		return;
	    }
	}

	if (Polymorph_control) {
		do {
			pline("Become what kind of monster? [type the name] ");
			getlin(buf);
			mntmp = name_to_mon(buf);
			if (mntmp < 0)
				pline("I've never heard of such monsters.");
			else if (!polyok(&mons[mntmp]))
				You("cannot polymorph into that.");
			else break;
		} while(++tries < 5);
		if (tries==5) pline(thats_enough_tries);
	} else if (draconian || iswere || isvamp) {
		/* special changes that don't require polyok() */
		if (draconian) {
			mntmp = uarm->corpsenm;
			if (!(mons[mntmp].geno & G_GENOD)) {
				You("merge with your scaly armor.");
				uskin = uarm;
				uarm = (struct obj *)0;
			}
		} else if (iswere) {
			if (is_were(uasmon))
				mntmp = PM_HUMAN; /* Illegal; force newman() */
			else
				mntmp = u.ulycn;
		} else {
			if (u.usym == S_VAMPIRE)
				mntmp = PM_VAMPIRE_BAT;
			else
				mntmp = PM_VAMPIRE;
		}
		if (polymon(mntmp))
			return;
	}

	if (mntmp < 0) {
		tries = 0;
		do {
			mntmp = rn2(PM_CHAMELEON);
			/* All valid monsters are from 0 to PM_CHAMELEON-1 */
		} while(!polyok(&mons[mntmp]) && tries++ < 200);
	}

	/* The below polyok() fails either if everything is genocided, or if
	 * we deliberately chose something illegal to force newman().
	 */
	if (!polyok(&mons[mntmp]) || !rn2(5))
		newman();
	else if(!polymon(mntmp)) return;

	if (!uarmg) selftouch("No longer petrify-resistant, you");
	if (Inhell && !Fire_resistance) {
	    You("burn to a crisp.");
	    killer = "unwise polymorph";
	    done(BURNING);
	}
}

int
polymon(mntmp)	/* returns 1 if polymorph successful */
	int	mntmp;
{
	int	tmp;
	sticky = sticks(uasmon) && u.ustuck && !u.uswallow;

	if (mons[mntmp].geno & G_GENOD) {
		You("feel rather %s-ish.",mons[mntmp].mname);
		return(0);
	}

	if (u.umonnum == -1) {
		/* Human to monster; save human stats */
		u.macurr = u.acurr;
		u.mamax = u.amax;
	} else {
		/* Monster to monster; restore human stats, to be
		 * immediately changed to provide stats for the new monster
		 */
		u.acurr = u.macurr;
		u.amax = u.mamax;
	}
	u.umonnum = mntmp;
	set_uasmon();
	u.usym = mons[mntmp].mlet;
	/* New stats for monster, to last only as long as polymorphed.
	 * Currently only strength gets changed.
	 */
	if(strongmonst(&mons[mntmp])) ABASE(A_STR) = AMAX(A_STR) = 118;

	if (resists_ston(uasmon) && Stoned) { /* parnes@eniac.seas.upenn.edu */
		Stoned = 0;
		You("no longer seem to be petrifying.");
	}
	if (u.usym == S_FUNGUS && Sick) {
		Sick = 0;
		You("no longer feel sick.");
	}
	if (u.usym == S_DRAGON && mntmp >= PM_GREY_DRAGON) u.mhmax = 80;
#ifdef GOLEMS
	else if (is_golem(uasmon)) u.mhmax = golemhp(mntmp);
#endif /* GOLEMS */
	else {

		/*
		tmp = adj_lev(&mons[mntmp]);
		 * We can't do this, since there's no such thing as an
		 * "experience level of you as a monster" for a polymorphed
		 * character.
		 */
		tmp = mons[mntmp].mlevel;
		if (!tmp) u.mhmax = rnd(4);
		else u.mhmax = d(tmp, 8);
	}
	u.mh = u.mhmax;
	You("turn into a%s %s!",
		index(vowels, *(mons[mntmp].mname)) ? "n" : "",
		mons[mntmp].mname);
	if (uskin && mntmp != uskin->corpsenm)
		skinback();
	break_armor();
	drop_weapon(1);
	if (u.uundetected && !hides_under(uasmon)) u.uundetected = 0;
	else if (hides_under(uasmon) && (OBJ_AT(u.ux, u.uy) ||
			levl[u.ux][u.uy].gmask))
		u.uundetected = 1;
	prme();
	if (!sticky && !u.uswallow && u.ustuck && sticks(uasmon)) u.ustuck = 0;
	else if (sticky && !sticks(uasmon)) uunstick();
	u.mtimedone = 500 + rn2(500);
	if (u.ulevel < mons[mntmp].mlevel)
	/* Low level characters can't become high level monsters for long */
		u.mtimedone = u.mtimedone * u.ulevel / mons[mntmp].mlevel;
	flags.botl = 1;
	if (can_breathe(uasmon))
		pline("Use the command #monster for breath weapon.");
	if (attacktype(uasmon, AT_SPIT))
		pline("Use the command #monster to spit venom.");
	if (u.usym == S_NYMPH)
		pline("Use the command #monster if you have to remove an iron ball.");
	if (u.usym == S_UMBER)
		pline("Use the command #monster to confuse monsters.");
	if (is_hider(uasmon))
		pline("Use the command #monster to hide.");
	if (is_were(uasmon))
		pline("Use the command #monster to summon help.");
	if (webmaker(uasmon))
		pline("Use the command #monster to spin a web.");
	if (lays_eggs(uasmon) || u.umonnum == PM_QUEEN_BEE)
		pline("Use the command #sit to lay an egg.");
	find_ac();
	return(1);
}

static void
break_armor() {
     struct obj *otmp;

     if (breakarm(uasmon)) {
	if (otmp = uarm) {
		You("break out of your armor!");
		(void) Armor_gone();
		useup(otmp);
	}
	if (otmp = uarmc) {
		Your("cloak tears apart!");
		(void) Cloak_off();
		useup(otmp);
	}
#ifdef SHIRT
	if (uarmu) {
		Your("shirt rips to shreds!");
		useup(uarmu);
	}
#endif
     } else if (sliparm(uasmon)) {
	if (otmp = uarm) {
		Your("armor falls around you!");
		(void) Armor_gone();
		dropx(otmp);
	}
	if (otmp = uarmc) {
		You("shrink out of your cloak!");
		(void) Cloak_off();
		dropx(otmp);
	}
#ifdef SHIRT
	if (otmp = uarmu) {
		You("become much too small for your shirt!");
		setworn((struct obj *)0, otmp->owornmask & W_ARMU);
		dropx(otmp);
	}
#endif
     }
     if (nohands(uasmon) || verysmall(uasmon)) {
	  if (otmp = uarmg) {
	       /* Drop weapon along with gloves */
	       You("drop your gloves%s!", uwep ? " and weapon" : "");
	       (void) Gloves_off();
	       dropx(otmp);
	       drop_weapon(0);
	  }
	  if (otmp = uarms) {
	       You("can no longer hold your shield!");
	       (void) Shield_off();
	       dropx(otmp);
	  }
	  if (otmp = uarmh) {
	       Your("helmet falls to the floor!");
	       (void) Helmet_off();
	       dropx(otmp);
	  }
	  if (otmp = uarmf) {
	       Your("boots %s off your feet!",
			verysmall(uasmon) ? "slide" : "get pushed");
	       (void) Boots_off();
	       dropx(otmp);
	  }
     }
}

static void
drop_weapon(alone)
int alone;
{
     struct obj *otmp;
     if (otmp = uwep) {
	  /* !alone check below is currently superfluous but in the
	   * future it might not be so if there are monsters which cannot
	   * wear gloves but can wield weapons
	   */
	  if (!alone || cantwield(uasmon)) {
	       if (alone) You("find you must drop your weapon!");
	       uwepgone();
	       dropx(otmp);
	  }
     }
}

void
rehumanize()
{
	sticky = sticks(uasmon) && u.ustuck && !u.uswallow;

	u.mh = u.mhmax = u.mtimedone = 0;
 	u.acurr = u.macurr;		/* restore old strength */
 	u.amax = u.mamax;
	u.usym = S_HUMAN;
	u.umonnum = -1;
	skinback();
	set_uasmon();
	You("return to %sn form!",(pl_character[0]=='E')?"elve":"huma");

	if (u.uhp < 1)	done(DIED);
	if (!Fire_resistance && Inhell) {
	    You("burn to a crisp.");
	    killer = "dissipating polymorph spell";
	    done(BURNING);
	}
	if (!uarmg) selftouch("No longer petrify-resistant, you");
	if (sticky) uunstick();
	nomul(0);
	if (u.uundetected) u.uundetected = 0;
	prme();
	flags.botl = 1;
	find_ac();
}

int
dobreathe() {
	if(!getdir(1)) return(0);
	if (rn2(4))
	    You("produce a loud and noxious belch.");
	else {
	    register struct attack *mattk;
	    register int i;

	    for(i = 0; i < NATTK; i++) {
		mattk = &(uasmon->mattk[i]);
		if(mattk->aatyp == AT_BREA) break;
	    }
	    buzz((int) (20 + mattk->adtyp-1), (int)mattk->damn,
		u.ux, u.uy, u.dx, u.dy);
	}
	return(1);
}

int
dospit() {
	struct obj *otmp;

	if (!getdir(1)) return(0);
	otmp = mksobj(u.umonnum==PM_COBRA ? BLINDING_VENOM : ACID_VENOM, FALSE);
	(void) throwit(otmp);
	return(1);
}

int
doremove() {
     if (!Punished) {
	  You("are not chained to anything!");
	  return(0);
     }
     unpunish();
     return(1);
}

int
dospinweb() {
	register struct trap *ttmp = t_at(u.ux,u.uy);

	if (Levitation) {
		You("must be on the ground to spin a web.");
		return(0);
	}
	if (u.uswallow) {
		You("release web fluid inside %s.", mon_nam(u.ustuck));
		pline("%s regurgitates you!", Monnam(u.ustuck));
		regurgitates(u.ustuck);
		return(1);
	}
	if (u.utrap) {
		You("cannot spin webs while stuck in a trap.");
		return(0);
	}
	if (ttmp) switch (ttmp->ttyp) {
		case SPIKED_PIT:
		case PIT: You("spin a web, covering up the pit.");
			deltrap(ttmp);
			if (Invisible) newsym(u.ux, u.uy);
			return(1);
		case WEB: You("make the web thicker.");
			return(1);
		case SQBRD: pline("The squeaky board is muffled.");
			deltrap(ttmp);
			if (Invisible) newsym(u.ux, u.uy);
			return(1);
		case TELEP_TRAP:
		case LEVEL_TELEP:
			Your("webbing vanishes!");
			return(0);
		case TRAPDOOR: if (!is_maze_lev) {
				You("web over the trapdoor.");
				deltrap(ttmp);
				if (Invisible) newsym(u.ux, u.uy);
				return 1;
			}
			/* Fall through */
		case MGTRP:
		case POLY_TRAP:
		case DART_TRAP:
		case ARROW_TRAP:
#ifdef SPELLS
		case ANTI_MAGIC:
#endif
		case LANDMINE:
		case SLP_GAS_TRAP:
		case BEAR_TRAP:
		case RUST_TRAP:
			You("have triggered a trap!");
			dotrap(ttmp);
			return(1);
		default:
			impossible("Webbing over trap type %d?",ttmp->ttyp);
			return(0);
	}
	ttmp = maketrap(u.ux, u.uy, WEB);
	ttmp->tseen = 1;
	if (Invisible) newsym(u.ux, u.uy);
	return(1);
}

int
dosummon()
{
	You("call upon your brethren for help!");
	if (!were_summon(uasmon,TRUE))
		pline("But none arrive.");
	return(1);
}

int
doconfuse()
{
	register struct monst *mtmp;
	int looked = 0;

	if (Blind) {
		You("can't see anything to gaze at.");
		return 0;
	}
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (canseemon(mtmp)) {
		looked = 1;
		if (Invis && !perceives(mtmp->data))
		    pline("%s seems not to notice your gaze.", Monnam(mtmp));
		else if (mtmp->minvis && !See_invisible)
		    You("can't see where to gaze at %s.", Monnam(mtmp));
		else if (mtmp->mimic)
		    continue;
		else if (flags.safe_dog && !Confusion && !Hallucination
		  && (mtmp->data->mlet == S_DOG || mtmp->data->mlet == S_FELINE)
		  && mtmp->mtame) {
		    if (mtmp->mnamelth)
			You("avoid gazing at %s.", NAME(mtmp));
		    else
			You("avoid gazing at your %s.",
						mtmp->data->mname);
		} else {
		    if (flags.confirm && mtmp->mpeaceful && !Confusion
							&& !Hallucination) {
			pline("Really confuse %s? ", mon_nam(mtmp));
			(void) fflush(stdout);
			if (yn() != 'y') continue;
			setmangry(mtmp);
		    }
		    if (mtmp->mfroz || mtmp->mstun || mtmp->msleep ||
							mtmp->mblinded)
			continue;
		    if (!mtmp->mconf)
			Your("gaze confuses %s!", mon_nam(mtmp));
		    else
			pline("%s is getting more and more confused.",
							Monnam(mtmp));
		    mtmp->mconf = 1;
		    if ((mtmp->data==&mons[PM_FLOATING_EYE]) && !mtmp->mcan) {
			You("are frozen by %s's gaze!", mon_nam(mtmp));
			nomul((u.ulevel > 6 || rn2(4)) ? 
				-d((int)mtmp->m_lev+1,
					(int)mtmp->data->mattk[0].damd)
				: -200);
			return 1;
		    }
#ifdef MEDUSA
		    if ((mtmp->data==&mons[PM_MEDUSA]) && !mtmp->mcan) {
			pline("Gazing at an awake medusa is not a very good idea...");
			/* as if gazing at a sleeping anything is fruitful... */
			You("turn to stone...");
			done(STONING);
		    }
#endif
		}
	    }
	}
	if (!looked) You("gaze at no place in particular.");
	return 1;
}

int
dohide()
{
	if (u.uundetected || u.usym == S_MIMIC_DEF) {
		pline("You are already hiding.");
		return(0);
	}
	if (u.usym == S_MIMIC) {
		u.usym = S_MIMIC_DEF;
		prme();
	} else {
		newsym(u.ux,u.uy);
		u.uundetected = 1;
	}
	return(1);
}

static void
uunstick()
{
	kludge("%s is no longer in your clutches...", Monnam(u.ustuck));
	u.ustuck = 0;
}

static void
skinback()
{
	if (uskin) {
		Your("skin returns to its original form.");
		uarm = uskin;
		uskin = (struct obj *)0;
	}	
}
#endif

char *
body_part(part)
{
	/* Note: it is assumed these will never be >22 characters long,
	 * plus the trailing null, after pluralizing (since sometimes a
	 * buffer is made a fixed size and must be able to hold it)
	 */
	static const char *humanoid_parts[] = { "arm", "eye", "face", "finger",
		"fingertip", "foot", "hand", "handed", "head", "leg",
		"light headed", "neck", "toe" };
#ifdef POLYSELF
	static const char *jelly_parts[] = { "pseudopod", "dark spot", "front",
		"pseudopod extension", "pseudopod extremity",
		"pseudopod root", "grasp", "grasped", "cerebral area",
		"lower pseudopod", "viscous", "middle",
		"pseudopod extremity" },
	*animal_parts[] = { "forelimb", "eye", "face", "foreclaw", "claw tip",
		"rear claw", "foreclaw", "clawed", "head", "rear limb",
		"light headed", "neck", "rear claw tip" },
	*horse_parts[] = { "forelimb", "eye", "face", "forehoof", "hoof tip",
		"rear hoof", "foreclaw", "hooved", "head", "rear limb",
		"light headed", "neck", "rear hoof tip" },
	*sphere_parts[] = { "appendage", "optic nerve", "body", "tentacle",
		"tentacle tip", "lower appendage", "tentacle", "tentacled",
		"body", "lower tentacle", "rotational", "equator",
		"lower tentacle tip" },
	*fungus_parts[] = { "mycelium", "visual area", "front", "hypha",
		"hypha", "root", "strand", "stranded", "cap area",
		"rhizome", "sporulated", "stalk", "rhizome tip" },
	*vortex_parts[] = { "region", "eye", "front", "minor current",
		"minor current", "lower current", "swirl", "swirled",
		"central core", "lower current", "addled", "center",
		"edge" },
	*snake_parts[] = { "vestigal limb", "eye", "face", "large scale",
		"large scale tip", "rear region", "scale gap", "scale gapped",
		"head", "rear region", "light headed", "neck", "rear scale" };
	
	if (humanoid(uasmon) || (u.usym==S_CENTAUR && 
		(part==ARM || part==FINGER || part==FINGERTIP
		|| part==HAND || part==HANDED))) return humanoid_parts[part];
	if (u.usym==S_CENTAUR || u.usym==S_UNICORN) return horse_parts[part];
	if (u.usym==S_SNAKE || u.usym==S_NAGA || u.usym==S_WORM)
		return snake_parts[part];
	if (u.usym==S_EYE) return sphere_parts[part];
	if (u.usym==S_JELLY || u.usym==S_PUDDING) return jelly_parts[part];
	if (u.usym==S_VORTEX || u.usym==S_ELEMENTAL) return vortex_parts[part];
	if (u.usym==S_FUNGUS) return fungus_parts[part];
	return animal_parts[part];
#else
	return humanoid_parts[part];
#endif
}

int
poly_gender()
{
/* Returns gender of polymorphed player; 0/1=same meaning as flags.female,
 * 2=none.
 * Used in:
 *	- Seduction by succubus/incubus
 *	- Talking to nymphs (sounds.c)
 * Not used in:
 *	- Messages given by nymphs stealing armor (they can't steal from
 *	  incubi/succubi/nymphs, and nonhumanoids can't wear armor).
 *	- Amulet of change (must refer to real gender no matter what
 *	  polymorphed into).
 *	- Priest/Priestess, Caveman/Cavewoman (ditto)
 *	- Polymorph self (only happens when human)
 *	- Shopkeeper messages (since referred to as "creature" and not "sir"
 *	  or "lady" when polymorphed)
 */
#ifdef POLYSELF
	if (uasmon->mflags1 & M1_FEM) return 1;
#ifdef HARD
	if (u.umonnum==PM_INCUBUS) return 0;
#endif
	if (!humanoid(uasmon)) return 2;
#endif
	return flags.female;
}

#ifdef POLYSELF
#ifdef GOLEMS
void
ugolemeffects(damtype, dam)
int damtype;
{
	int heal = 0;
	/* We won't bother with "slow"/"haste" since players do not
	 * have a monster-specific slow/haste so there is no way to
	 * restore the old velocity once they are back to human.
	 */
	if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
		return;
	switch (damtype) {
		case AD_ELEC: if (u.umonnum == PM_IRON_GOLEM)
				heal = dam / 6; /* Approx 1 per die */
			break;
		case AD_FIRE: if (u.umonnum == PM_IRON_GOLEM)
				heal = dam;
			break;
	}
	if (heal && (u.mh < u.mhmax)) {
		u.mh += heal;
		if (u.mh > u.mhmax) u.mh = u.mhmax;
		flags.botl = 1;
		pline("Strangely, you feel better than before.");
	}
}
#endif /* GOLEMS */
#endif
