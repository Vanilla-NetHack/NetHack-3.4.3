/*	SCCS Id: @(#)u_init.c	3.2	96/06/16	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct trobj {
	short trotyp;
	schar trspe;
	char trclass;
	Bitfield(trquan,6);
	Bitfield(trbless,2);
};

static void FDECL(ini_inv, (struct trobj *));
static void FDECL(knows_object,(int));
static void FDECL(knows_class,(CHAR_P));
static int FDECL(role_index,(CHAR_P));

#define UNDEF_TYP	0
#define UNDEF_SPE	'\177'
#define UNDEF_BLESS	2

static boolean random_role = FALSE;

/* all roles must all have distinct first letter */
const char *roles[] = {	/* also used in options.c and winxxx.c */
			/* roles[2] and [6] are changed for females */
			/* in all cases, the corresponding male and female */
			/* roles must start with the same letter */
	"Archeologist", "Barbarian", "Caveman", "Elf", "Healer", "Knight",
	"Priest", "Rogue", "Samurai",
#ifdef TOURIST
	"Tourist",
#endif
	"Valkyrie", "Wizard", 0
};

/*
 *	Initial inventory for the various roles.
 */

static struct trobj Archeologist[] = {
	/* if adventure has a name...  idea from tan@uvm-gen */
	{ BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ PICK_AXE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TINNING_KIT, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Barbarian[] = {
#define B_MAJOR	0	/* two-handed sword or battle-axe  */
#define B_MINOR	1	/* matched with axe or short sword */
	{ TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RING_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Cave_man[] = {
#define C_ARROWS	2
	{ CLUB, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_CLASS, 25, UNDEF_BLESS },	/* quan is variable */
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Elf[] = {
#define E_ARROWS	2
#define E_ARMOR		3
	{ ELVEN_SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_BOW, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_ARROW, 0, WEAPON_CLASS, 25, UNDEF_BLESS },
	{ UNDEF_TYP, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEMBAS_WAFER, 0, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Healer[] = {
	{ SCALPEL, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ STETHOSCOPE, 0, TOOL_CLASS, 1, 0 },
	{ POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	/* always blessed, so it's guaranteed readable */
	{ SPE_HEALING, 0, SPBOOK_CLASS, 1, 1 },
	{ SPE_EXTRA_HEALING, 0, SPBOOK_CLASS, 1, 1 },
	{ APPLE, 0, FOOD_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Knight[] = {
	{ LONG_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SPEAR, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RING_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SMALL_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Priest[] = {
	{ MACE, 1, WEAPON_CLASS, 1, 1 },
	{ CHAIN_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SMALL_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },	/* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 0 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Rogue[] = {
#define R_DAGGERS	1
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_CLASS, 10, 0 },	/* quan is variable */
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 1, 0 },
	{ LOCK_PICK, 9, TOOL_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Samurai[] = {
#define S_ARROWS	3
	{ KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS }, /* wakizashi */
	{ YUMI, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ YA, 0, WEAPON_CLASS, 25, UNDEF_BLESS }, /* variable quan */
	{ SPLINT_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FORTUNE_COOKIE, 0, FOOD_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Tourist[] = {
#define T_DARTS		0
	{ DART, 2, WEAPON_CLASS, 25, UNDEF_BLESS },	/* quan is variable */
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 10, 0 },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 4, UNDEF_BLESS },
	{ HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ EXPENSIVE_CAMERA, 0, TOOL_CLASS, 1, 0 },
	{ CREDIT_CARD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj Valkyrie[] = {
	{ LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SMALL_SHIELD, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Wizard[] = {
#define W_MULTSTART	2
#define W_MULTEND	6
	{ ATHAME, 1, WEAPON_CLASS, 1, 1 },	/* for dealing with ghosts */
	{ CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

/*
 *	Optional extra inventory items.
 */

static struct trobj Tinopener[] = {
	{ TIN_OPENER, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Magicmarker[] = {
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Lamp[] = {
	{ OIL_LAMP, 1, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Blindfold[] = {
	{ BLINDFOLD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Instrument[] = {
	{ WOODEN_FLUTE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Leash[] = {
	{ LEASH, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Towel[] = {
	{ TOWEL, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif	/* TOURIST */
static struct trobj Wishing[] = {
	{ WAN_WISHING, 3, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

#ifdef WEAPON_SKILLS
static struct def_skill Skill_A[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_BASIC },
    { P_PICK_AXE, P_EXPERT },		{ P_SHORT_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_SKILLED },		{ P_QUARTERSTAFF, P_SKILLED },
    { P_SLING, P_SKILLED },		{ P_DART, P_BASIC },
    { P_BOOMERANG, P_EXPERT },		{ P_WHIP, P_EXPERT },
    { P_UNICORN_HORN, P_SKILLED },	{ P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_B[] = {
    { P_DAGGER, P_BASIC },		{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_EXPERT },		{ P_SHORT_SWORD, P_BASIC },
    { P_BROAD_SWORD, P_SKILLED },	{ P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_SCIMITAR, P_SKILLED },
    { P_SABER, P_BASIC },		{ P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },		{ P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },	{ P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },		{ P_BOW, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_C[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },		{ P_PICK_AXE, P_BASIC },
    { P_CLUB, P_EXPERT },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_SKILLED },
    { P_HAMMER, P_SKILLED },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_SKILLED },		{ P_TRIDENT, P_SKILLED },
    { P_BOW, P_EXPERT },		{ P_SLING, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },		{ P_UNICORN_HORN, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_E[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_SKILLED },	{ P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_SKILLED },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_BASIC },
    { P_BOW, P_EXPERT },		{ P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },		{ P_SHURIKEN, P_BASIC },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_H[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },	{ P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },		{ P_CLUB, P_SKILLED },
    { P_MACE, P_BASIC },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_BASIC },		{ P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },		{ P_TRIDENT, P_BASIC },
    { P_SLING, P_SKILLED },		{ P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },		{ P_UNICORN_HORN, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_K[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE, P_BASIC },
    { P_AXE, P_SKILLED },		{ P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_SKILLED },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_SKILLED },
    { P_SCIMITAR, P_BASIC },		{ P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },		{ P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_SKILLED },	{ P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },		{ P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },		{ P_JAVELIN, P_SKILLED },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_EXPERT },
    { P_BOW, P_BASIC },			{ P_CROSSBOW, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_P[] = {
    { P_CLUB, P_EXPERT },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_SKILLED },
    { P_JAVELIN, P_SKILLED },		{ P_TRIDENT, P_SKILLED },
    { P_LANCE, P_BASIC },		{ P_BOW, P_BASIC },
    { P_SLING, P_BASIC },		{ P_CROSSBOW, P_BASIC },
    { P_DART, P_BASIC },		{ P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },		{ P_UNICORN_HORN, P_SKILLED },
    { P_MARTIAL_ARTS, P_BASIC },	/* until Monk finally gets added */
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_R[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },	{ P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },		{ P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },		{ P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },		{ P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_S[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },		{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_BASIC },		{ P_SABER, P_BASIC },
    { P_FLAIL, P_SKILLED },		{ P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_BOW, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NO_TYPE, 0 }
};

#ifdef TOURIST
static struct def_skill Skill_T[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_SKILLED },
    { P_AXE, P_BASIC },			{ P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_BASIC },
    { P_LONG_SWORD, P_BASIC },		{ P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_SKILLED },
    { P_MACE, P_BASIC },		{ P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },		{ P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },	{ P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },			{ P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },		{ P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },		{ P_BOOMERANG, P_BASIC },
    { P_WHIP, P_BASIC },		{ P_UNICORN_HORN, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NO_TYPE, 0 }
};
#endif /* TOURIST */

static struct def_skill Skill_V[] = {
    { P_DAGGER, P_EXPERT },		{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },		{ P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },	{ P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },		{ P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },	{ P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_SLING, P_BASIC },		{ P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NO_TYPE, 0 }
};

static struct def_skill Skill_W[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },		{ P_SHORT_SWORD, P_BASIC },
    { P_CLUB, P_SKILLED },		{ P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_SLING, P_SKILLED },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NO_TYPE, 0 }
};
#endif /* WEAPON_SKILLS */


static void
knows_object(obj)
register int obj;
{
	makeknown(obj);
	objects[obj].oc_pre_discovered = 1;	/* not a "discovery" */
}

/* Know ordinary (non-magical) objects of a certain class,
 * like all gems except the loadstone and luckstone.
 */
static void
knows_class(sym)
register char sym;
{
	register int ct;
	for (ct = 1; ct < NUM_OBJECTS; ct++)
		if (objects[ct].oc_class == sym && !objects[ct].oc_magic)
			knows_object(ct);
}

static int
role_index(pc)
char pc;
{
	register const char *cp;

	if ((cp = index(pl_classes, pc)) != 0)
		return(cp - pl_classes);
	return(-1);
}

void
u_init()
{
	register int i;
	char pc;

	pc = pl_character[0];
	if(pc == '\0') {
	    /* should be unnecessary now */
	    exit_nhwindows((char *)0);
	    terminate(EXIT_SUCCESS);
	}
	i = role_index(pc);
	if (random_role) {
	    pline("This game you will be %s.", an(roles[i]));
	    display_nhwindow(WIN_MESSAGE, TRUE);
	}

	(void) strncpy(pl_character, roles[i], PL_CSIZ-1);
	pl_character[PL_CSIZ-1] = 0;
	flags.beginner = 1;

	/* zero u, including pointer values --
	 * necessary when aborting from a failed restore */
	(void) memset((genericptr_t)&u, 0, sizeof(u));
	u.ustuck = (struct monst *)0;

#if 0	/* documentation of more zero values as desirable */
	u.usick_cause[0] = 0;
	u.uluck  = u.moreluck = 0;
# ifdef TOURIST
	uarmu = 0;
# endif
	uarm = uarmc = uarmh = uarms = uarmg = uarmf = 0;
	uwep = uball = uchain = uleft = uright = 0;
	u.ublessed = 0;				/* not worthy yet */
	u.ugangr   = 0;				/* gods not angry */
# ifdef ELBERETH
	u.uevent.uhand_of_elbereth = 0;
# endif
	u.uevent.uheard_tune = 0;
	u.uevent.uopened_dbridge = 0;
	u.uevent.udemigod = 0;		/* not a demi-god yet... */
	u.udg_cnt = 0;
	u.mh = u.mhmax = u.mtimedone = 0;
	u.uz.dnum = u.uz0.dnum = 0;
	u.utotype = 0;
#endif	/* 0 */
	u.uz.dlevel = 1;
	u.uz0.dlevel = 0;
	u.utolev = u.uz;

	u.role = pl_character[0];
	u.usym = S_HUMAN;
	u.umoved = FALSE;
	u.umortality = 0;
	u.ugrave_arise = NON_PM;

	u.ulevel = 0;	/* set up some of the initial attributes */
	u.uhp = u.uhpmax = newhp();
	adjabil(0,1);
	u.ulevel = 1;

	init_uhunger();
	u.uen = u.uenmax = 2;
	for (i = 0; i <= MAXSPELL; i++) spl_book[i].sp_id = NO_SPELL;
	u.ublesscnt = 300;			/* no prayers just yet */
	u.umonnum = PM_PLAYERMON;
	u.ulycn = NON_PM;
	set_uasmon();

#ifdef BSD
	(void) time((long *)&u.ubirthday);
#else
	(void) time(&u.ubirthday);
#endif

	/*
	 *  For now, everyone starts out with a night vision range of 1 and
	 *  their xray range disabled.
	 */
	u.nv_range   =  1;
	u.xray_range = -1;


	switch(pc) {
	/* pc will always be in uppercase by this point */
	case 'A':
		u.umonster = PM_ARCHEOLOGIST;
		ini_inv(Archeologist);
		if(!rn2(10)) ini_inv(Tinopener);
		else if(!rn2(4)) ini_inv(Lamp);
		else if(!rn2(10)) ini_inv(Magicmarker);
		knows_class(GEM_CLASS);
		knows_object(SACK);
#ifdef WEAPON_SKILLS
		skill_init(Skill_A);
#endif /* WEAPON_SKILLS */
		break;
	case 'B':
		u.umonster = PM_BARBARIAN;
		if (rn2(100) >= 50) {	/* see Elf comment */
		    Barbarian[B_MAJOR].trotyp = BATTLE_AXE;
		    Barbarian[B_MINOR].trotyp = SHORT_SWORD;
		}
		ini_inv(Barbarian);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
#ifdef WEAPON_SKILLS
		skill_init(Skill_B);
#endif /* WEAPON_SKILLS */
		break;
	case 'C':
		u.umonster = flags.female ? PM_CAVEWOMAN : PM_CAVEMAN;
		Cave_man[C_ARROWS].trquan = rn1(30, 13);
		ini_inv(Cave_man);
#ifdef WEAPON_SKILLS
		skill_init(Skill_C);
#endif /* WEAPON_SKILLS */
		break;
	case 'E':
		u.umonster = PM_ELF;
		Elf[E_ARROWS].trquan = rn1(20, 16);
		Elf[E_ARMOR].trotyp = ((rn2(100) >= 50)
				 ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
			/* rn2(100) > 50 necessary because some random number
			 * generators are bad enough to seriously skew the
			 * results if we use rn2(2)...  --KAA
			 */
		ini_inv(Elf);
		/*
		 * Elves are people of music and song, or they are warriors.
		 * Warriors get mithril coats; non-warriors MAY get an
		 * instrument.  We use a kludge to get only non-magic
		 * instruments.
		 */
		if (Elf[E_ARMOR].trotyp == ELVEN_CLOAK && !rn2(5)) {
		    static int trotyp[] = {
			WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
			BELL, BUGLE, LEATHER_DRUM
		    };
		    Instrument[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		    ini_inv(Instrument);
		}
		if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(6)) ini_inv(Lamp);
		knows_object(ELVEN_SHORT_SWORD);
		knows_object(ELVEN_ARROW);
		knows_object(ELVEN_BOW);
		knows_object(ELVEN_SPEAR);
		knows_object(ELVEN_DAGGER);
		knows_object(ELVEN_BROADSWORD);
		knows_object(ELVEN_MITHRIL_COAT);
		knows_object(ELVEN_LEATHER_HELM);
		knows_object(ELVEN_SHIELD);
		knows_object(ELVEN_BOOTS);
		knows_object(ELVEN_CLOAK);
#ifdef WEAPON_SKILLS
		skill_init(Skill_E);
#endif /* WEAPON_SKILLS */
		break;
	case 'H':
		u.umonster = PM_HEALER;
		u.uen = u.uenmax += rn1(4, 1);
		u.ugold = u.ugold0 = rn1(1000, 1001);
		ini_inv(Healer);
		if(!rn2(25)) ini_inv(Lamp);
#ifdef WEAPON_SKILLS
		skill_init(Skill_H);
#endif /* WEAPON_SKILLS */
		break;
	case 'K':
		u.umonster = PM_KNIGHT;
		u.uen = u.uenmax += rn1(4, 1);
		ini_inv(Knight);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		/* give knights chess-like mobility
		 * -- idea from wooledge@skybridge.scl.cwru.edu */
		Jumping |= FROMOUTSIDE;
#ifdef WEAPON_SKILLS
		skill_init(Skill_K);
#endif /* WEAPON_SKILLS */
		break;
	case 'P':
		u.umonster = flags.female ? PM_PRIESTESS : PM_PRIEST;
		u.uen = u.uenmax += rn1(3, 4);
		ini_inv(Priest);
		if(!rn2(10)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		knows_object(POT_WATER);
#ifdef WEAPON_SKILLS
		skill_init(Skill_P);
#endif /* WEAPON_SKILLS */
		break;
	case 'R':
		u.umonster = PM_ROGUE;
		Rogue[R_DAGGERS].trquan = rn1(10, 6);
		u.ugold = u.ugold0 = 0;
		ini_inv(Rogue);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_object(SACK);
#ifdef WEAPON_SKILLS
		skill_init(Skill_R);
#endif /* WEAPON_SKILLS */
		break;
	case 'S':
		u.umonster = PM_SAMURAI;
		Samurai[S_ARROWS].trquan = rn1(20, 26);
		ini_inv(Samurai);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
#ifdef WEAPON_SKILLS
		skill_init(Skill_S);
#endif /* WEAPON_SKILLS */
		break;
#ifdef TOURIST
	case 'T':
		u.umonster = PM_TOURIST;
		Tourist[T_DARTS].trquan = rn1(20, 21);
		u.ugold = u.ugold0 = rnd(1000);
		ini_inv(Tourist);
		if(!rn2(25)) ini_inv(Tinopener);
		else if(!rn2(25)) ini_inv(Leash);
		else if(!rn2(25)) ini_inv(Towel);
		else if(!rn2(25)) ini_inv(Magicmarker);
#ifdef WEAPON_SKILLS
		skill_init(Skill_T);
#endif /* WEAPON_SKILLS */
		break;
#endif
	case 'V':
		u.umonster = PM_VALKYRIE;
		flags.female = TRUE;
		ini_inv(Valkyrie);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
#ifdef WEAPON_SKILLS
		skill_init(Skill_V);
#endif /* WEAPON_SKILLS */
		break;
	case 'W':
		u.umonster = PM_WIZARD;
		u.uen = u.uenmax += rn1(3, 4);
		ini_inv(Wizard);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
#ifdef WEAPON_SKILLS
		skill_init(Skill_W);
#endif /* WEAPON_SKILLS */
		break;

	default:	/* impossible */
		break;
	}
	if (discover)
		ini_inv(Wishing);

	u.ugold0 += hidden_gold();	/* in case sack has gold in it */

	find_ac();			/* get initial ac value */
	init_attr(75);			/* init attribute values */
	max_rank_sz();			/* set max str size for class ranks */
/*
 *	Do we really need this?
 */
	for(i = 0; i < A_MAX; i++)
	    if(!rn2(20)) {
		register int xd = rn2(7) - 2;	/* biased variation */
		(void) adjattrib(i, xd, TRUE);
		if (ABASE(i) < AMAX(i)) AMAX(i) = ABASE(i);
	    }

	/* make sure you can carry all you have - especially for Tourists */
	while(inv_weight() > 0 && ACURR(A_STR) < 118)
		(void) adjattrib(A_STR, 1, TRUE);
	/* undo exercise for wisdom due to `makeknown' of prediscovered items */
	AEXE(A_WIS) = 0;

	u.ualignbase[0] = u.ualignbase[1] = u.ualign.type;
}

static void
ini_inv(trop)
register struct trobj *trop;
{
	struct obj *obj;
	while(trop->trclass) {
		boolean undefined = (trop->trotyp == UNDEF_TYP);

		if (!undefined)
			obj = mksobj((int)trop->trotyp, TRUE, FALSE);
		else obj = mkobj(trop->trclass,FALSE);

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
			static NEARDATA short nocreate = STRANGE_OBJECT;
			static NEARDATA short nocreate2 = STRANGE_OBJECT;
			static NEARDATA short nocreate3 = STRANGE_OBJECT;

			while(obj->otyp == WAN_WISHING
				|| obj->otyp == nocreate
				|| obj->otyp == nocreate2
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
				/* powerful spells are either useless to
				   low level players or unbalancing */
				|| (obj->oclass == SPBOOK_CLASS &&
				    objects[obj->otyp].oc_level > 3)
							) {
				dealloc_obj(obj);
				obj = mkobj(trop->trclass, FALSE);
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
			switch (obj->otyp) {
			    case WAN_POLYMORPH:
			    case RIN_POLYMORPH:
				nocreate = RIN_POLYMORPH_CONTROL;
				break;
			    case RIN_POLYMORPH_CONTROL:
				nocreate = RIN_POLYMORPH;
				nocreate2 = SPE_POLYMORPH;
			}
			/* Don't have 2 of the same ring or spellbook */
			if (obj->oclass == RING_CLASS ||
			    obj->oclass == SPBOOK_CLASS)
				nocreate3 = obj->otyp;
		}

		obj->dknown = obj->bknown = obj->rknown = 1;
		if (objects[obj->otyp].oc_uses_known) obj->known = 1;
		obj->cursed = 0;
		if(obj->oclass == WEAPON_CLASS || obj->oclass == TOOL_CLASS) {
			obj->quan = (long) trop->trquan;
			trop->trquan = 1;
		}
		if(trop->trspe != UNDEF_SPE)
			obj->spe = trop->trspe;
		if(trop->trbless != UNDEF_BLESS)
			obj->blessed = trop->trbless;

		/* defined after setting otyp+quan + blessedness */
		obj->owt = weight(obj);
		obj = addinv(obj);

		/* Make the type known if necessary */
		if (OBJ_DESCR(objects[obj->otyp]) && obj->known)
			makeknown(obj->otyp);
		if (obj->otyp == OIL_LAMP)
			makeknown(POT_OIL);

		if(obj->oclass == ARMOR_CLASS){
			if (is_shield(obj) && !uarms)
				setworn(obj, W_ARMS);
			else if (is_helmet(obj) && !uarmh)
				setworn(obj, W_ARMH);
			else if (is_gloves(obj) && !uarmg)
				setworn(obj, W_ARMG);
#ifdef TOURIST
			else if (is_shirt(obj) && !uarmu)
				setworn(obj, W_ARMU);
#endif
			else if (is_cloak(obj) && !uarmc)
				setworn(obj, W_ARMC);
			else if (is_boots(obj) && !uarmf)
				setworn(obj, W_ARMF);
			else if (is_suit(obj) && !uarm)
				setworn(obj, W_ARM);
		}
		/* below changed by GAN 01/09/87 to allow wielding of
		 * pick-axe or can-opener if there is no weapon
		 */
		if(obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
		   obj->otyp == TIN_OPENER)
			if(!uwep) setuwep(obj);
#if !defined(PYRAMID_BUG) && !defined(MAC)
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
plnamesuffix()
{
	register char *p;
	if ((p = rindex(plname, '-')) != 0) {
		*p = '\0';
		pl_character[0] = p[1];
		pl_character[1] = '\0';
		random_role = FALSE;
		if(!plname[0]) {
			askname();
			plnamesuffix();
		}
	}
	if (pl_character[0] == '@') {	/* explicit request for random class */
		int i = rn2((int)strlen(pl_classes));
		pl_character[0] = pl_classes[i];
		pl_character[1] = '\0';
		random_role = TRUE;
	}
}

/*u_init.c*/
