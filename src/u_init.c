/*	SCCS Id: @(#)u_init.c	3.0	88/04/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct trobj {
	unsigned short int trotyp;
	schar trspe;
	char trolet;
	Bitfield(trquan,6);
	Bitfield(trknown,1);
	Bitfield(trbless,2);
};

static void ini_inv P((struct trobj *));

#define	UNDEF_TYP	0
#define	UNDEF_SPE	'\177'
#define	UNDEF_BLESS	2

char *(roles[]) = {	/* must all have distinct first letter */
			/* roles[2] and [6] are changed for females */
			/* in all cases, the corresponding male and female */
			/* roles must start with the same letter */
	"Archeologist", "Barbarian", "Cave-man", "Elf", "Healer", "Knight",
	"Priest", "Rogue", "Samurai", "Tourist", "Valkyrie", "Wizard"
};

struct you zerou;

#define	NR_OF_ROLES	SIZE(roles)
char rolesyms[NR_OF_ROLES + 1];		/* filled by u_init() */

struct trobj Cave_man[] = {
#define C_ARROWS	2
	{ CLUB, 1, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ BOW, 1, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_SYM, 25, 1, UNDEF_BLESS },	/* quan is variable */
	{ LEATHER_ARMOR, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Barbarian[] = {
	{ TWO_HANDED_SWORD, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ AXE, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ RING_MAIL, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Knight[] = {
	{ LONG_SWORD, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ SPEAR, 2, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ RING_MAIL, 1, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ SMALL_SHIELD, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Elf[] = {
#define E_ARROWS	2
#define E_ARMOR		3
#ifdef TOLKIEN
	{ ELVEN_SHORT_SWORD, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ ELVEN_BOW, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ ELVEN_ARROW, 0, WEAPON_SYM, 25, 1, UNDEF_BLESS },
	{ UNDEF_TYP, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ LEMBAS_WAFER, 0, FOOD_SYM, 2, 1, 0 },
#else
	{ SHORT_SWORD, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ BOW, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_SYM, 25, 1, UNDEF_BLESS },
	{ ELVEN_CLOAK, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_SYM, 2, 1, 0 },
#endif
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Valkyrie[] = {
	{ LONG_SWORD, 1, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ SMALL_SHIELD, 3, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Healer[] = {
	{ SCALPEL, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 1, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ STETHOSCOPE, 0, TOOL_SYM, 1, 1, 0 },
	{ POT_HEALING, 0, POTION_SYM, 4, 1, UNDEF_BLESS },
	{ POT_EXTRA_HEALING, 0, POTION_SYM, 4, 1, UNDEF_BLESS },
	{ WAN_SLEEP, UNDEF_SPE, WAND_SYM, 1, 1, UNDEF_BLESS },
#ifdef SPELLS
	/* always blessed, so it's guaranteed readable */
	{ SPE_HEALING, 0, SPBOOK_SYM, 1, 1, 1 },
	{ SPE_EXTRA_HEALING, 0, SPBOOK_SYM, 1, 1, 1 },
#endif
	{ APPLE, 0, FOOD_SYM, 5, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Archeologist[] = {
	/* if adventure has a name...  idea from tan@uvm-gen */
	{ BULLWHIP, 2, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ LEATHER_ARMOR, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ FEDORA, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_SYM, 3, 1, 0 },
	{ PICK_AXE, UNDEF_SPE, TOOL_SYM, 1, 1, UNDEF_BLESS },
	{ TINNING_KIT, 0, TOOL_SYM, 1, 1, UNDEF_BLESS },
	{ SACK, 0, TOOL_SYM, 1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Tinopener[] = {
	{ TIN_OPENER, 0, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Magicmarker[] = {
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Lamp[] = {
	{ LAMP, 5, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

#ifndef HARD
struct trobj Saving[] = {
	{ AMULET_OF_LIFE_SAVING, 0, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};
#endif

#ifdef EXPLORE_MODE
struct trobj Wishing[] = {
	{ WAN_WISHING, 3, WAND_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};
#endif

#ifdef WALKIES
struct trobj Leash[] = {
	{ LEASH, 0, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};
#endif

struct trobj Blindfold[] = {
	{ BLINDFOLD, 0, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Tourist[] = {
#define	T_DARTS		0
	{ DART, 2, WEAPON_SYM, 25, 1, UNDEF_BLESS },	/* quan is variable */
	{ UNDEF_TYP, UNDEF_SPE, FOOD_SYM, 10, 1, 0 },
	{ POT_EXTRA_HEALING, 0, POTION_SYM, 2, 1, UNDEF_BLESS },
	{ SCR_MAGIC_MAPPING, 0, SCROLL_SYM, 4, 1, UNDEF_BLESS },
	{ EXPENSIVE_CAMERA, 0, TOOL_SYM, 1, 1, 0 },
#ifdef SHIRT
	{ HAWAIIAN_SHIRT, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
#endif
	{ CREDIT_CARD, 0, TOOL_SYM, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Rogue[] = {
#define R_DAGGERS	1
	{ SHORT_SWORD, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_SYM, 10, 1, 0 },	/* quan is variable */
	{ LEATHER_ARMOR, 1, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_SYM, 1, 1, 0 },
	{ LOCK_PICK, 9, TOOL_SYM, 1, 1, 0 },
	{ SACK, 0, TOOL_SYM, 1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct trobj Wizard[] = {
#define W_MULTSTART	2
#ifdef SPELLS
#  define W_MULTEND	6
#else
#  define W_MULTEND	5
#endif
	{ ATHAME, 1, WEAPON_SYM, 1, 1, 1 },	/* for dealing with ghosts */
	{ CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, WAND_SYM, 1, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_SYM, 2, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_SYM, 3, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_SYM, 3, 1, UNDEF_BLESS },
#ifdef SPELLS
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_SYM, 1, 1, UNDEF_BLESS },
#endif
	{ 0, 0, 0, 0, 0, 0 }
};

struct	trobj	Samurai[] = {
	{ KATANA, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },
	{ SHORT_SWORD, 0, WEAPON_SYM, 1, 1, UNDEF_BLESS },	/* the wakizashi */
	{ SHURIKEN, 0, WEAPON_SYM, 9, 1, UNDEF_BLESS },        /* quan is variable */
	{ SPLINT_MAIL, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ FORTUNE_COOKIE, 0, FOOD_SYM, 3, 1, 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct	trobj	Priest[] = {
	{ MACE, 1, WEAPON_SYM, 1, 1, 1 },
	{ CHAIN_MAIL, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ SMALL_SHIELD, 0, ARMOR_SYM, 1, 1, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_SYM, 4, 1, 1 },	/* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_SYM, 1, 1, 0 },
#ifdef SPELLS
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_SYM, 2, 1, UNDEF_BLESS },
#endif
	{ 0, 0, 0, 0, 0, 0 }
};

static void
knows_class(sym)
register char sym;
{
	register unsigned ct;
	for (ct = 1; ct <= NROFOBJECTS; ct++)
		if (objects[ct].oc_olet == sym) {
			makeknown(ct);
			objects[ct].oc_descr = NULL;	/* not a "discovery" */
		}
}

static int
role_index(pc)
char pc;
{		/* must be called only from u_init() */
		/* so that rolesyms[] is defined */
	register char *cp;

	if(cp = index(rolesyms, pc))
		return(cp - rolesyms);
	return(-1);
}

void
u_init()
{
	register int i;
	char pick, pc;

	Printf("\nNetHack, Copyright 1985, 1986, 1987, 1988, 1989.");
	Printf("\n         By Stichting Mathematisch Centrum and M. Stephenson.");
	Printf("\n         See license for details.\n\n");

	if(flags.female)  {	/* should have been set in NETHACKOPTIONS */
		roles[2] = "Cave-woman";
		roles[6] = "Priestess";
	}
	for(i = 0; i < NR_OF_ROLES; i++)
		rolesyms[i] = roles[i][0];
	rolesyms[i] = 0;

	if(pc = pl_character[0]) {
		if('a' <= pc && pc <= 'z') pc += 'A'-'a';
		if((i = role_index(pc)) >= 0)
			goto got_suffix;
		Printf("\nUnknown role: %c\n", pc);
		pl_character[0] = pc = 0;
	}

	Printf("\nShall I pick a character for you? [Y,N, or Q(quit)] ");

	while(!index("yYnNqQ", (pick = readchar())) && !index(quitchars, pick))
		bell();

	if(index(quitchars, pick)) pick = 'Y';
	else if('a' <= pick && pick <= 'z') pick += 'A'-'a';

	Printf("%c\n", pick);		/* echo */

	if (pick == 'Q') {
		clearlocks();
		settty(NULL);
		exit(0);
	}

	if (pick == 'Y')
		goto beginner;

	Printf("\nWhat kind of character are you:\n\n");
	Printf("         An");
	Printf(" %s,",roles[0]);
	for(i = 1; i < NR_OF_ROLES; i++) {
		Printf(" a%s %s", index(vowels,roles[i][0]) ? "n" : "", roles[i]);
		if((((i + 1) % 4) == 0) && (i != NR_OF_ROLES -1)) Printf(",\n        ");
		else if(i < NR_OF_ROLES - 2)	Printf(",");
		if(i == NR_OF_ROLES - 2)	Printf(" or");
	}
	Printf("?\n         [");
	for(i = 0; i < NR_OF_ROLES; i++) Printf("%c,", rolesyms[i]);
	Printf(" or Q] ");

	while(pc = readchar()) {
		if('a' <= pc && pc <= 'z') pc += 'A'-'a';
		if (pc == 'Q') {
			clearlocks();
			settty(NULL);
			exit(0);
		}
		if((i = role_index(pc)) >= 0) {
			Printf("%c\n", pc);	/* echo */
			(void) fflush(stdout);	/* should be seen */
			break;
		}
		if(pc == '\n') break;
		bell();
	}
	if(pc == '\n')	pc = 0;

beginner:
	if(!pc) {
		i = rn2(NR_OF_ROLES);
		pc = rolesyms[i];
		Printf("\nThis game you will be %s %s.\n",
			index("AEIOU", roles[i][0]) ? "an" : "a",
			roles[i]);
		getret();
		/* give him some feedback in case mklev takes much time */
		(void) putchar('\n');
		(void) fflush(stdout);
	}

got_suffix:

	(void) strncpy(pl_character, roles[i], PL_CSIZ-1);
	pl_character[PL_CSIZ-1] = 0;
	flags.beginner = 1;
	u = zerou;
	u.usym = S_HUMAN;
	u.umoved = FALSE;
	u.ugrave_arise = -1;

	u.ulevel = 0;	/* set up some of the initial attributes */
	u.uhp = u.uhpmax = newhp();
	adjabil(1);
	u.ulevel = 1;

	u.uluck  = u.moreluck = 0;
	init_uhunger();
	uarm = uarmc = uarmh = uarms = uarmg = uarmf =
#ifdef SHIRT
	uarmu =
#endif
	uwep = uball = uchain = uleft = uright = 0;

#ifdef SPELLS
	u.uen = u.uenmax = 1;
	for (i = 0; i <= MAXSPELL; i++) spl_book[i].sp_id = NO_SPELL;
#endif
#ifdef THEOLOGY
	u.ublesscnt = 300;			/* no prayers just yet */
	u.ublessed = 0;				/* not worthy yet */
	u.ugangr   = 0;				/* gods not angry */
#endif
#if defined(THEOLOGY) && defined(ELBERETH)
	u.uhand_of_elbereth = 0;
#endif
#ifdef MEDUSA
	u.ukilled_medusa = 0;
#endif
#ifdef HARD
	u.udemigod = u.udg_cnt = 0;		/* not a demi-god yet... */
#endif
#ifdef POLYSELF
	u.umonnum = u.ulycn = -1;
	u.mh = u.mhmax = u.mtimedone = 0;
	set_uasmon();
#endif
	switch(pc) {
	/* pc will always be in uppercase by this point */
	case 'C':
		Cave_man[C_ARROWS].trquan = 12 + rnd(30);
		ini_inv(Cave_man);
		break;
	case 'T':
		Tourist[T_DARTS].trquan = 20 + rnd(20);
		u.ugold = u.ugold0 = rnd(1000);
		ini_inv(Tourist);
		if(!rn2(25)) ini_inv(Tinopener);
		else if(!rn2(25)) ini_inv(Magicmarker);
#ifdef WALKIES
		else if(!rn2(25)) ini_inv(Leash);
#endif
		break;
	case 'R':
		Rogue[R_DAGGERS].trquan = 5 + rnd(10);
		u.ugold = u.ugold0 = 0;
		ini_inv(Rogue);
		if(!rn2(5)) ini_inv(Blindfold);
		makeknown(SACK);
		break;
	case 'W':
#ifdef SPELLS
		u.uen = u.uenmax += rn2(4);
#endif
		ini_inv(Wizard);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		break;
	case 'A':
		ini_inv(Archeologist);
		if(!rn2(10)) ini_inv(Tinopener);
		else if(!rn2(4)) ini_inv(Lamp);
		else if(!rn2(10)) ini_inv(Magicmarker);
		knows_class(GEM_SYM);
		makeknown(SACK);
		/* We can't set trknown for it, then it'd be "uncursed"
		 * sack...
		 */
		break;
	case 'E':
		Elf[E_ARROWS].trquan = 15+rnd(20);
#ifdef TOLKIEN
		Elf[E_ARMOR].trotyp = ((rn2(100) >= 50)
				 ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
			/* rn2(100) > 50 necessary because some random number
			 * generators are bad enough to seriously skew the
			 * results if we use rn2(2)...  --KAA
			 */
#endif
		ini_inv(Elf);
		if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(6)) ini_inv(Lamp);
#ifdef TOLKIEN
		/* makeknown(ELVEN_SHORT_SWORD);
		 * no need to do this since the initial inventory contains one,
		 * so ini_inv already did it for us
		 */
		objects[ELVEN_SHORT_SWORD].oc_descr = NULL;
		/* makeknown(ELVEN_ARROW); */
		objects[ELVEN_ARROW].oc_descr = NULL;
		/* makeknown(ELVEN_BOW); */
		objects[ELVEN_BOW].oc_descr = NULL;
		makeknown(ELVEN_SPEAR);
		objects[ELVEN_SPEAR].oc_descr = NULL;
		makeknown(ELVEN_DAGGER);
		objects[ELVEN_DAGGER].oc_descr = NULL;
		makeknown(ELVEN_BROADSWORD);
		objects[ELVEN_BROADSWORD].oc_descr = NULL;
#endif
		makeknown(ELVEN_CLOAK);
		objects[ELVEN_CLOAK].oc_descr = NULL;
		break;
	case 'V':
		flags.female = TRUE;
		ini_inv(Valkyrie);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_SYM);
		break;
	case 'H':
		u.ugold = u.ugold0 = rnd(1000)+1000;
		ini_inv(Healer);
		if(!rn2(25)) ini_inv(Lamp);
		break;
	case 'K':
		ini_inv(Knight);
		knows_class(WEAPON_SYM);
		break;
	case 'B':
		ini_inv(Barbarian);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_SYM);
		break;
	case 'S':
		ini_inv(Samurai);
		if(!rn2(5)) ini_inv(Blindfold);
		objects[SHORT_SWORD].oc_name = "wakizashi";
		objects[BROADSWORD].oc_name = "ninja-to";
		objects[GLAIVE].oc_name = "naginata";
		/* objects[BOW].oc_name = "yumi"; */
		objects[LOCK_PICK].oc_name = "osaku";
		knows_class(WEAPON_SYM);
		break;
	case 'P':
#ifdef SPELLS
		u.uen = u.uenmax += rn2(4);
#endif
		ini_inv(Priest);
		if(!rn2(10)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		break;

	default:	/* impossible */
		break;
	}
#ifndef HARD
	ini_inv(Saving);	/* give beginners an extra chance */
#endif
#ifdef EXPLORE_MODE
	if (discover)
		ini_inv(Wishing);
#endif
	find_ac();			/* get initial ac value */
	init_attr((pick != 'Y') ? 69 : 72);	/* init attribute values */
	max_rank_sz();			/* set max str size for class ranks */
/*
 *	Do we really need this?
 */
	for(i = 0; i < A_MAX; i++)
	    if(!rn2(20)) {
		register int xd = rn2(7) - 2;	/* biased variation */
		adjattrib(i, xd, TRUE);
		if (ABASE(i) < AMAX(i)) AMAX(i) = ABASE(i);
	    }

	/* make sure he can carry all he has - especially for T's */
	while(inv_weight() > 0)
		adjattrib(A_STR, 1, TRUE);

#ifdef THEOLOGY
	u.ualignbase[0] = u.ualignbase[1] = u.ualigntyp;
#endif
}

static void
ini_inv(trop)
register struct trobj *trop;
{
	struct obj *obj;
	while(trop->trolet) {
		boolean undefined = (trop->trotyp == UNDEF_TYP);

		if (!undefined)
			obj = mksobj((int)trop->trotyp,FALSE);
		else obj = mkobj(trop->trolet,FALSE);

		/* For random objects, do not create certain overly powerful
		 * items: wand of wishing, ring of levitation, or the
		 * polymorph/polymorph control combination.  Specific objects,
		 * i.e. the discovery wishing, are still OK.
		 * Also, don't get a couple of really useless items.  (Note:
		 * punishment isn't "useless".  Some players who start out with
		 * one will immediately read it and use the iron ball as a
		 * weapon.)
		 */
		if (undefined) {
#ifdef POLYSELF
			static unsigned nocreate = STRANGE_OBJECT;
#  ifdef SPELLS
			static unsigned nocreate2 = STRANGE_OBJECT;
#  endif
#endif
			static unsigned nocreate3 = STRANGE_OBJECT;

			while(obj->otyp == WAN_WISHING
#ifdef POLYSELF
				|| obj->otyp == nocreate
#  ifdef SPELLS
				|| obj->otyp == nocreate2
#  endif
#endif
				|| obj->otyp == nocreate3
#ifdef ELBERETH
				|| obj->otyp == RIN_LEVITATION
#endif
				/* 'useless' items */
				|| obj->otyp == POT_HALLUCINATION
				|| obj->otyp == SCR_AMNESIA
				|| obj->otyp == SCR_FIRE
				|| obj->otyp == RIN_AGGRAVATE_MONSTER
				|| obj->otyp == RIN_HUNGER
				|| obj->otyp == WAN_NOTHING
							) {
				free((genericptr_t) obj);
				obj = mkobj(trop->trolet, FALSE);
			}

			/* Don't start with +0 or negative rings */
			if(objects[obj->otyp].oc_charged && obj->spe <= 0)
				obj->spe = rne(3);

			/* Heavily relies on the fact that 1) we create wands
			 * before rings, 2) that we create rings before
			 * spellbooks, and that 3) not more than 1 object of a
			 * particular symbol is to be prohibited.  (For more
			 * objects, we need more nocreate variables...)
			 */
#ifdef POLYSELF
			switch (obj->otyp) {
			    case WAN_POLYMORPH:
			    case RIN_POLYMORPH:
				nocreate = RIN_POLYMORPH_CONTROL;
				break;
			    case RIN_POLYMORPH_CONTROL:
				nocreate = RIN_POLYMORPH;
#  ifdef SPELLS
				nocreate2 = SPE_POLYMORPH;
#  endif /* SPELLS */
			}
#endif /* POLYSELF */
			/* Don't have 2 of the same ring */
			if (obj->olet == RING_SYM)
				nocreate3 = obj->otyp;
		}

		obj->bknown = trop->trknown;
		if(objects[obj->otyp].oc_uses_known) obj->known = trop->trknown;
		/* not obj->dknown = 1; - let him look at it at least once */
		obj->cursed = 0;
		if(obj->olet == TOOL_SYM){ /* problem with multiple tools */
			obj->quan = 1;     /* might be > because of grenades */
		}
		if(obj->olet == WEAPON_SYM) {
			obj->quan = trop->trquan;
			trop->trquan = 1;
		}
		if(obj->olet == FOOD_SYM && undefined) {
			obj->known = 1;
			/* needed for tins and eggs; harmless otherwise */
			obj->bknown = (obj->otyp != DEAD_LIZARD);
			/* only for dead lizards does the blessing not matter */
		}
		/*
		 * The below lines not needed because they don't correspond
		 * to any actual inventory; nobody gets random tools.
		else if(obj->olet == TOOL_SYM && undefined) {
			obj->bknown = (obj->otyp != BAG_OF_TRICKS
				&& obj->otyp != SACK
				&& obj->otyp != CHEST
				&& obj->otyp != LARGE_BOX
				&& obj->otyp != ICE_BOX);
		}
		*/
		if(trop->trspe != UNDEF_SPE)
			obj->spe = trop->trspe;
		if(trop->trbless != UNDEF_BLESS)
			obj->blessed = trop->trbless;

		if (!Is_container(obj))
			obj->owt = weight(obj);
			/* defined after setting otyp+quan */
		obj = addinv(obj);

		/* Make the type known if necessary */
		if (objects[obj->otyp].oc_descr && obj->known)
		    	makeknown(obj->otyp);

		if(obj->olet == ARMOR_SYM){
			if (is_shield(obj) && !uarms)
				setworn(obj, W_ARMS);
			else if (is_helmet(obj) && !uarmh)
				setworn(obj, W_ARMH);
			else if (is_gloves(obj) && !uarmg)
				setworn(obj, W_ARMG);
#ifdef SHIRT
			else if (obj->otyp == HAWAIIAN_SHIRT && !uarmu)
				setworn(obj, W_ARMU);
#endif
			else if (is_cloak(obj) && !uarmc)
				setworn(obj, W_ARMC);
			else if (is_boots(obj) && !uarmf)
				setworn(obj, W_ARMF);
			else if (!uarm)
				setworn(obj, W_ARM);
		}
		/* below changed by GAN 01/09/87 to allow wielding of
		 * pick-axe or can-opener if there is no weapon
		 */
		if(obj->olet == WEAPON_SYM || obj->otyp == PICK_AXE ||
		   obj->otyp == TIN_OPENER)
			if(!uwep) setuwep(obj);
#ifndef PYRAMID_BUG
		if(--trop->trquan) continue;	/* make a similar object */
#else
		if(trop->trquan) {		/* check if zero first */
			--trop->trquan;
			if(trop->trquan)
				continue;	/* make a similar object */
		}
#endif
		trop++;
	}
}

void
plnamesuffix() {
	register char *p;
	if(p = rindex(plname, '-')) {
		*p = 0;
		pl_character[0] = p[1];
		pl_character[1] = 0;
		if(!plname[0]) {
			askname();
			plnamesuffix();
		}
	}
}
