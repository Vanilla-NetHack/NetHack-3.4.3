/*	SCCS Id: @(#)objects.c	3.1	92/12/13	*/
/*	Copyright (c) Mike Threepoint, 1989 (890110) */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJECTS_PASS_2_
/* first pass */
# include "config.h"
# include "obj.h"
# include "objclass.h"
# include "prop.h"

#else	/* !OBJECTS_PASS_2_ */
/* second pass */
# ifdef TEXTCOLOR
#include "color.h"
# endif
#endif	/* !OBJECTS_PASS_2_ */

/* objects have symbols: ) [ = " ( % ! ? + / $ * ` 0 _ . */

#ifndef OBJECTS_PASS_2_
/* first pass -- object descriptive text */
# define OBJ(name,desc) name,desc
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
	{obj}
/* note:  OBJ and BITS macros used to get around compiler argument limits */
/*	the ctnr field of BITS currently does not map into struct objclass,
 *	and is ignored in the expansion */

NEARDATA struct objdescr obj_descr[] = {
#else
/* second pass -- object definitions */

# define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,dir,mtrl) nmkn,mrg,uskn,mgc,chrg,uniq,nwsh,big,dir,mtrl /* SCO ODT 1.1 cpp fodder */
/* SCO's version of MSC 5.x barfs on the line above without a trailing space */

#ifdef TEXTCOLOR
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
	{0, 0, NULL, bits, prp, sym, dly, color,\
	 prob, wt, cost, sdam, ldam, oc1, oc2, nut}
#else
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
	{0, 0, NULL, bits, prp, sym, dly,\
	 prob, wt, cost, sdam, ldam, oc1, oc2, nut}
#endif

NEARDATA struct objclass objects[] = {
#endif
/* dummy object[0] -- description [2nd arg] *must* be NULL */
	OBJECT(OBJ("strange object",NULL), BITS(1,0,0,0,0,0,0,0,0,0,0), 0,
			ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

/* weapons ... */
#define WEAPON(name,app,kn,mg,bi,prob,wt,cost,sdam,ldam,hitbon,metal,color) OBJECT( \
		OBJ(name,app), BITS(kn,mg,1,0,0,1,0,0,bi,0,metal), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, 0, wt, color )
#define PROJECTILE(name,app,kn,prob,wt,cost,sdam,ldam,hitbon,metal,prop,color) OBJECT( \
		OBJ(name,app), BITS(kn,1,1,0,0,1,0,0,0,0,metal), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, prop, wt, color )
#define BOW(name,app,kn,prob,wt,cost,sdam,ldam,hitbon,metal,prop,color) OBJECT( \
		OBJ(name,app), BITS(kn,0,1,0,0,1,0,0,0,0,metal), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, -(prop), wt, color )

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
 * the extra damage is added on in weapon.c, not here! */

/* missiles */
PROJECTILE("arrow",             NULL,           1,  42, 1,  2, 6, 6, 0,
		   IRON, WP_BOW, HI_METAL),
PROJECTILE("elven arrow",       "runed arrow",  0,  10, 1,  2, 7, 6, 0,
		   IRON, WP_BOW, HI_METAL),
PROJECTILE("orcish arrow",      "crude arrow",  0,  11, 1,  2, 5, 6, 0,
		   IRON, WP_BOW, BLACK),
PROJECTILE("silver arrow",      NULL,           1,   8, 1,  2, 6, 6, 0,
		   SILVER, WP_BOW, HI_SILVER),
PROJECTILE("ya",                "bamboo arrow", 0,   5, 1,  4, 7, 7, 1,
		   METAL, WP_BOW, HI_METAL),
PROJECTILE("crossbow bolt",     NULL,           1,  50, 1,  2, 4, 6, 0,
		   IRON, WP_CROSSBOW, HI_METAL),

WEAPON("dart",          NULL,           1, 1, 0, 60,  1,  2, 3, 2, 0, IRON, HI_METAL),
WEAPON("shuriken",      "throwing star",0, 1, 0, 35,  1,  5, 8, 6, 2, IRON, HI_METAL),
WEAPON("boomerang",     NULL,           1, 1, 0, 15,  5, 20, 9, 9, 0, WOOD, HI_WOOD),

/* spears */
WEAPON("spear",         NULL,           1, 1, 0, 50, 30,  3, 6, 8, 0, IRON, HI_METAL),
WEAPON("elven spear",   "runed spear",  0, 1, 0, 10, 30,  3, 7, 8, 0, IRON, HI_METAL),
WEAPON("orcish spear",  "crude spear",  0, 1, 0, 13, 30,  3, 5, 8, 0, IRON, BLACK),
WEAPON("dwarvish spear", "stout spear", 0, 1, 0, 12, 35,  3, 8, 8, 0, IRON, HI_METAL),
WEAPON("javelin",       "throwing spear",0,1, 0, 10, 20,  3, 6, 6, 0, IRON, HI_METAL),
WEAPON("trident",       NULL,           1, 0, 0,  8, 25,  5, 6, 4, 0, IRON, HI_METAL),
						/* +1 small, +2d4 large */
WEAPON("lance",         NULL,           1, 0, 0,  8,180, 10, 6, 8, 0, IRON, HI_METAL),

/* blades */
WEAPON("dagger",        NULL,           1, 1, 0, 25, 10,  4,  4,  3, 2, IRON, HI_METAL),
WEAPON("elven dagger",  "runed dagger", 0, 1, 0,  8, 10,  4,  5,  3, 2, IRON, HI_METAL),
WEAPON("orcish dagger", "crude dagger", 0, 1, 0, 10, 10,  4,  3,  3, 2, IRON, BLACK),
WEAPON("athame",        NULL,           1, 1, 0,  0, 10,  4,  4,  3, 2, IRON, HI_METAL),
WEAPON("scalpel",       NULL,           1, 1, 0,  0,  5,  4,  3,  3, 2, IRON, HI_METAL),
WEAPON("knife",         NULL,           1, 1, 0, 20,  5,  4,  3,  2, 0, IRON, HI_METAL),
WEAPON("stiletto",      NULL,           1, 1, 0,  5,  5,  4,  3,  2, 0, IRON, HI_METAL),
WEAPON("worm tooth",    NULL,           1, 0, 0,  0, 20,  2,  2,  2, 0, 0, WHITE),
WEAPON("crysknife",     NULL,           1, 0, 0,  0, 20,100, 10, 10, 3, MINERAL, WHITE),
WEAPON("dwarvish mattock",      "heavy pick",
					0, 0, 1, 13,120, 50,12, 8,-1, IRON, HI_METAL),
WEAPON("axe",           NULL,           1, 0, 0, 40, 60,  8,  6,  4, 0, IRON, HI_METAL),
WEAPON("battle-axe","double-headed axe",0, 0, 1, 10,120, 40,  8,  6, 0, IRON, HI_METAL),
						/* "double-bitted" ? */

/* swords */
WEAPON("short sword",           NULL,   1, 0, 0,  8, 30, 10,  6,  8, 0, IRON, HI_METAL),
WEAPON("elven short sword",     "runed short sword",
					0, 0, 0,  2, 30, 10,  8,  8, 0, IRON, HI_METAL),
WEAPON("orcish short sword", "crude short sword",
					0, 0, 0,  3, 30, 10,  5,  8, 0, IRON, BLACK),
WEAPON("dwarvish short sword", "broad short sword",
					0, 0, 0,  2, 30, 10,  7,  8, 0, IRON, HI_METAL),
WEAPON("scimitar", "curved sword",	0, 0, 0, 15, 40, 15,  8,  8, 0, IRON, HI_METAL),
WEAPON("silver saber", NULL,		1, 0, 0,  6, 40, 75,  8,  8, 0, SILVER, HI_SILVER),
WEAPON("broadsword", NULL,		1, 0, 0,  8, 70, 10,  4,  6, 0, IRON, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("elven broadsword", "runed broadsword",
					0, 0, 0,  4, 70, 10,  6,  6, 0, IRON, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("long sword", NULL,		1, 0, 0, 50, 40, 15,  8, 12, 0, IRON, HI_METAL),
WEAPON("two-handed sword", NULL,	1, 0, 1, 22,150, 50, 12,  6, 0, IRON, HI_METAL),
						/* +2d6 large */
WEAPON("katana", "samurai sword",	0, 0, 0,  4, 40, 80, 10, 12, 1, IRON, HI_METAL),
/* two set-up-for-artifacts swords */
						/* +2d6 large */
WEAPON("tsurugi", "long samurai sword", 0, 0, 1,  0, 60,500, 16,  8, 2, METAL, HI_METAL),
						/* +5 */
WEAPON("runesword", "runed broadsword", 0, 0, 0,  0, 40,300,  4,  6, 0, IRON, BLACK),
						/* +d4 small, +1 large */
						/* +5d2 +d8 from level drain */

/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm",    0, 0, 1, 10, 80, 10,  6, 6, 0, IRON, HI_METAL),
						/* +1 large */
WEAPON("ranseur", "hilted polearm",     0, 0, 1, 10, 50,  6,  4, 4, 0, IRON, HI_METAL),
						/* +d4 both */
WEAPON("spetum", "forked polearm",      0, 0, 1, 10, 50,  5,  6, 6, 0, IRON, HI_METAL),
						/* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm",0, 0, 1, 15, 75,  6,  6,10, 0, IRON, HI_METAL),
/* axe-type */
WEAPON("halberd", "angled poleaxe",     0, 0, 1, 16,150, 10, 10, 6, 0, IRON, HI_METAL),
						/* +1d6 large */
WEAPON("bardiche", "long poleaxe",      0, 0, 1,  8,120,  7,  4, 4, 0, IRON, HI_METAL),
						/* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver",        0, 0, 1,  8,125,  5,  4, 4, 0, IRON, HI_METAL),
						/* +d4 both */
/* curved/hooked */
WEAPON("fauchard",      "pole sickle",  0, 0, 1, 11, 60,  5,  6, 8, 0, IRON, HI_METAL),
WEAPON("guisarme",      "pruning hook", 0, 0, 1, 11, 80,  5,  4, 8, 0, IRON, HI_METAL),
						/* +1d4 small */
WEAPON("bill-guisarme", "hooked polearm",0,0, 1,  8,120,  7,  4,10, 0, IRON, HI_METAL),
						/* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm",0,0,1, 10,150,  7,  4, 6, 0, IRON, HI_METAL),
						/* +1d4 small */
WEAPON("bec de corbin", "beaked polearm",0,0, 1,  8,100,  8,  8, 6, 0, IRON, HI_METAL),

/* blunt */
WEAPON("mace",          NULL,           1, 0, 0, 40, 30,  5,  6, 6, 0, IRON, HI_METAL),
						/* +1 small */
WEAPON("morning star",  NULL,           1, 0, 0, 12,120, 10,  4, 6, 0, IRON, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("war hammer",    NULL,           1, 0, 0, 15, 50,  5,  4, 4, 0, IRON, HI_METAL),
						/* +1 small */
WEAPON("club",          NULL,           1, 0, 0, 12, 30,  3,  6, 3, 0, WOOD, HI_WOOD),
#ifdef KOPS
WEAPON("rubber hose",   NULL,           1, 0, 0,  0, 20,  3,  4, 3, 0, 0, BROWN),
#endif
WEAPON("quarterstaff",  "staff",        0, 0, 1, 11, 40,  5,  6, 6, 0, WOOD, HI_WOOD),
/* two-piece */
WEAPON("aklys",         "thonged club", 0, 0, 0,  8, 15,  4,  6, 3, 0, IRON, HI_METAL),
WEAPON("flail",         NULL,           1, 0, 0, 40, 15,  4,  6, 4, 0, IRON, HI_METAL),
						/* +1 small, +1d4 large */
/* whip */
WEAPON("bullwhip",      NULL,           1, 0, 0,  2, 20,  4,  2, 1, 0, LEATHER, BROWN),

/* bows */
BOW("bow",        NULL,         1,  24, 30,  60, 30, 6, 0, WOOD, WP_BOW, HI_WOOD),
BOW("elven bow",  "runed bow",  0,  12, 30,  60, 35, 6, 0, WOOD, WP_BOW, HI_WOOD),
BOW("orcish bow", "crude bow",  0,  12, 30,  60, 25, 6, 0, WOOD, WP_BOW, BLACK),
BOW("yumi",       "long bow",   0,   0, 30,  60, 35, 6, 0, WOOD, WP_BOW, HI_WOOD),
BOW("sling",      NULL,         1,  40,  3,  20,  4, 6, 0, WOOD, WP_SLING, HI_WOOD),
BOW("crossbow",   NULL,         1,  45, 50,  40, 35, 6, 0, WOOD, WP_CROSSBOW, HI_WOOD),
#undef WEAPON
#undef PROJECTILE
#undef BOW

/* armor ... */
/* IRON denotes ferrous metals, including steel.
 * Only IRON weapons and armor can rust.
 * Only COPPER (including brass) corrodes.
 * Some creatures are vulnerable to SILVER.
 */
#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c) OBJECT( \
		OBJ(name,desc), BITS(kn,0,1,0,mgc,1,0,0,blk,0,metal), power, \
		ARMOR_CLASS, prob, delay, wt, cost, \
		0, 0, 10 - ac, can, wt, c )

/* helmets */
ARMOR("elven leather helm", "leather hat",
		0, 0, 0, 0,  6, 1,  3,	 8, 9, 0, LEATHER, HI_LEATHER),
ARMOR("orcish helm", "iron skull cap",
		0, 0, 0, 0,  6, 1, 30,	10, 9, 0, IRON, BLACK),
ARMOR("dwarvish iron helm", "hard hat",
		0, 0, 0, 0,  6, 1, 40,	20, 8, 0, IRON, HI_METAL),
ARMOR("fedora", NULL,
		1, 0, 0, 0,  0, 0,  3,	 1,10, 0, CLOTH, BROWN),
ARMOR("dented pot", NULL,
		1, 0, 0, 0,  2, 0, 10,	 8, 9, 0, IRON, BLACK),
ARMOR("helmet", "plumed helmet",
		0, 0, 0, 0, 10, 1, 30,	10, 9, 0, IRON, HI_METAL),
ARMOR("helm of brilliance", "etched helmet",
		0, 1, 0, 0,  6, 1, 50,	50, 9, 0, IRON, GREEN),
ARMOR("helm of opposite alignment", "crested helmet",
		0, 1, 0, 0,  6, 1, 50,	50, 9, 0, IRON, HI_METAL),
ARMOR("helm of telepathy", "visored helmet",
		0, 1, 0, TELEPAT, 2, 1, 50, 50, 9, 0, IRON, HI_METAL),

/* suits of armor */
/*
 * There is code in polyself.c that assumes (1) and (2).
 * There is code in objnam.c, mon.c, read.c that assumes (2).
 *
 *	(1) The dragon scale mails and the dragon scales are together.
 *	(2) That the order of the dragon scale mail and dragon scales is the
 *	    the same defined in monst.c.
 */
#define DRGN_ARMR(name,power,cost,ac,color) \
	ARMOR(name,NULL,1,0,1,power,0,5,40,cost,ac,0,DRAGON_HIDE,color)
DRGN_ARMR("gray dragon scale mail",   ANTIMAGIC,  1200, 1, GRAY),
DRGN_ARMR("red dragon scale mail",    FIRE_RES,    900, 1, RED),
DRGN_ARMR("white dragon scale mail",  COLD_RES,    900, 1, WHITE),
DRGN_ARMR("orange dragon scale mail", SLEEP_RES,   900, 1, ORANGE_COLORED),
DRGN_ARMR("black dragon scale mail",  DISINT_RES, 1200, 1, BLACK),
DRGN_ARMR("blue dragon scale mail",   SHOCK_RES,   900, 1, BLUE),
DRGN_ARMR("green dragon scale mail",  POISON_RES,  900, 1, GREEN),
DRGN_ARMR("yellow dragon scale mail", 0,           900, 1, YELLOW),

/* For now, only dragons leave these. */
DRGN_ARMR("gray dragon scales",   ANTIMAGIC,  700, 7, GRAY),
DRGN_ARMR("red dragon scales",    FIRE_RES,   500, 7, RED),
DRGN_ARMR("white dragon scales",  COLD_RES,   500, 7, WHITE),
DRGN_ARMR("orange dragon scales", SLEEP_RES,  500, 7, ORANGE_COLORED),
DRGN_ARMR("black dragon scales",  DISINT_RES, 700, 7, BLACK),
DRGN_ARMR("blue dragon scales",   SHOCK_RES,  500, 7, BLUE),
DRGN_ARMR("green dragon scales",  POISON_RES, 500, 7, GREEN),
DRGN_ARMR("yellow dragon scales", 0,          500, 7, YELLOW),
#undef DRGN_ARMR

ARMOR("plate mail", NULL,
		1, 0, 1, 0,	44, 5, 450, 600,  3, 2, IRON, HI_METAL),
ARMOR("crystal plate mail", NULL,
		1, 0, 1, 0,	10, 5, 450, 820,  3, 2, GLASS, WHITE),
#ifdef TOURIST
ARMOR("bronze plate mail", NULL,
		1, 0, 1, 0,	25, 5, 450, 400,  4, 0, COPPER, HI_COPPER),
#else
ARMOR("bronze plate mail", NULL,
		1, 0, 1, 0,	35, 5, 450, 400,  4, 0, COPPER, HI_COPPER),
#endif
ARMOR("splint mail", NULL,
		1, 0, 1, 0,	66, 5, 400,  80,  4, 1, IRON, HI_METAL),
ARMOR("banded mail", NULL,
		1, 0, 1, 0,	76, 5, 350,  90,  4, 0, IRON, HI_METAL),
ARMOR("dwarvish mithril-coat", NULL,
		1, 0, 0, 0,	10, 1, 150, 240,  4, 3, MITHRIL, HI_METAL),
ARMOR("elven mithril-coat", NULL,
		1, 0, 0, 0,	15, 1, 150, 240,  5, 3, MITHRIL, HI_METAL),
ARMOR("chain mail", NULL,
		1, 0, 0, 0,	76, 5, 300,  75,  5, 1, IRON, HI_METAL),
ARMOR("orcish chain mail", "crude chain mail",
		0, 0, 0, 0,	20, 5, 300,  75,  5, 1, IRON, BLACK),
ARMOR("scale mail", NULL,
		1, 0, 0, 0,	76, 5, 250,  45,  6, 0, IRON, HI_METAL),
ARMOR("studded leather armor", NULL,
		1, 0, 0, 0,	76, 3, 200,  15,  7, 1, LEATHER, HI_LEATHER),
ARMOR("ring mail", NULL,
		1, 0, 0, 0,	76, 5, 250, 100,  7, 0, IRON, HI_METAL),
ARMOR("orcish ring mail", "crude ring mail",
		0, 0, 0, 0,	20, 5, 250,  80,  8, 1, IRON, BLACK),
ARMOR("leather armor", NULL,
		1, 0, 0, 0,	85, 3, 150,   5,  8, 0, LEATHER, HI_LEATHER),
ARMOR("leather jacket", NULL,
		1, 0, 0, 0,	12, 0,  30,  10,  9, 0, LEATHER, BLACK),

/* cloaks */
/*  'cope' is not a spelling mistake... leave it be */
ARMOR("mummy wrapping", NULL,
		1, 0, 0, 0,		0, 0,  3,   2, 10, 1, CLOTH, GRAY),
ARMOR("elven cloak", "faded pall",
		0, 1, 0, STEALTH,      10, 0, 10,  60,	9, 3, CLOTH, BLACK),
ARMOR("orcish cloak", "coarse mantelet",
		0, 0, 0, 0,	       10, 0, 10,  40, 10, 2, CLOTH, BLACK),
ARMOR("dwarvish cloak", "hooded cloak",
		0, 0, 0, 0,	       10, 0, 10,  50, 10, 2, CLOTH, HI_CLOTH),
ARMOR("oilskin cloak", "slippery cloak",
		0, 0, 0, 0,	       10, 0, 10,  50,  9, 3, CLOTH, HI_CLOTH),
ARMOR("cloak of protection", "tattered cape",
		0, 1, 0, PROTECTION,   10, 0, 10,  50,	7, 3, CLOTH, HI_CLOTH),
ARMOR("cloak of invisibility", "opera cloak",
		0, 1, 0, INVIS,        11, 0, 10,  60,	9, 2, CLOTH, BRIGHT_MAGENTA),
ARMOR("cloak of magic resistance", "ornamental cope",
		0, 1, 0, ANTIMAGIC,	2, 0, 10,  60,	9, 3, CLOTH, WHITE),
ARMOR("cloak of displacement", "piece of cloth",
		0, 1, 0, DISPLACED,    11, 0, 10,  50,	9, 2, CLOTH, HI_CLOTH),

/* shields */
ARMOR("small shield", NULL,
			1, 0, 0, 0,	6, 0, 30,   3,	9, 0, WOOD, HI_WOOD),
/* TODO: these shield descriptions should be changed,
 * because you can't see colors when blind.
 */
ARMOR("elven shield", "blue and green shield",
			0, 0, 0, 0,	2, 0, 50,   7,	8, 0, IRON, GREEN),
ARMOR("Uruk-hai shield", "white-handed shield",
			0, 0, 0, 0,	2, 0, 50,   7,	9, 0, IRON, HI_METAL),
ARMOR("orcish shield", "red-eyed shield",
			0, 0, 0, 0,	2, 0, 50,   7,	9, 0, IRON, RED),
ARMOR("large shield", NULL,
			1, 0, 1, 0,	7, 0,100,  10,	8, 0, IRON, HI_METAL),
ARMOR("dwarvish roundshield", "large round shield",
			0, 0, 0, 0,	4, 0,100,  10,	8, 0, IRON, HI_METAL),
ARMOR("shield of reflection", "polished silver shield",
		0, 1, 0, REFLECTING,	3, 0, 50,  50,	8, 0, SILVER,  HI_SILVER),

#ifdef TOURIST
/* shirt */
ARMOR("Hawaiian shirt", NULL,
		1, 0, 0, 0,	       10, 0,  5,   3, 10, 0, CLOTH,   MAGENTA),
#endif

/* gloves */
ARMOR("leather gloves", "old gloves",
	0, 0, 0, 0,	       16, 1, 10,   8,	9, 0, LEATHER, HI_LEATHER),
ARMOR("gauntlets of fumbling", "padded gloves",
	0, 1, 0, FUMBLING,	8, 1, 10,  50,	9, 0, LEATHER, HI_LEATHER),
ARMOR("gauntlets of power", "riding gloves",
	0, 1, 0, 0,		8, 1, 30,  50,	9, 0, IRON,    BROWN),
ARMOR("gauntlets of dexterity", "fencing gloves",
	0, 1, 0, 0,		8, 1, 10,  50,	9, 0, LEATHER, HI_LEATHER),

/* boots */
ARMOR("low boots", "walking shoes",
	0, 0, 0, 0,	       25, 2, 10,   8,	9, 0, LEATHER, HI_LEATHER),
ARMOR("iron shoes", "hard shoes",
	0, 0, 0, 0,		7, 2, 50,  16,	8, 0, IRON,    HI_METAL),
ARMOR("high boots", "jackboots",
	0, 0, 0, 0,	       15, 2, 20,  12,	8, 0, LEATHER, HI_LEATHER),
ARMOR("speed boots", "combat boots",
	0, 1, 0, FAST,	       12, 2, 20,  50,	9, 0, LEATHER, HI_LEATHER),
ARMOR("water walking boots", "jungle boots",
	0, 1, 0, WWALKING,     12, 2, 20,  50,	9, 0, LEATHER, HI_LEATHER),
ARMOR("jumping boots", "hiking boots",
	0, 1, 0, JUMPING,      12, 2, 20,  50,	9, 0, LEATHER, HI_LEATHER),
ARMOR("elven boots", "mud boots",
	0, 1, 0, STEALTH,      12, 2, 15,   8,	9, 0, LEATHER, HI_LEATHER),
ARMOR("fumble boots", "riding boots",
	0, 1, 0, FUMBLING,     12, 2, 20,  30,	9, 0, LEATHER, HI_LEATHER),
ARMOR("levitation boots", "snow boots",
	0, 1, 0, LEVITATION,   12, 2, 15,  30,	9, 0, LEATHER, HI_LEATHER),
#undef ARMOR

/* rings ... */
#define RING(name,stone,power,cost,mgc,spec,metal,color) OBJECT( \
		OBJ(name,stone), BITS(0,0,spec,0,mgc,spec,0,0,0,0,metal), \
		power, RING_CLASS, 0, 0, 3, cost, 0, 0, 0, 0, 15, color )
RING("adornment",         "wooden",     ADORNED,        100, 0, 1, WOOD,     HI_WOOD),
RING("gain strength",     "granite",    0,              150, 1, 1, MINERAL,  HI_MINERAL),
RING("increase damage",   "coral",      0,              150, 1, 1, MINERAL,  ORANGE_COLORED),
RING("protection",        "black onyx", PROTECTION,     100, 1, 1, MINERAL,  BLACK),
RING("regeneration",      "moonstone",  REGENERATION,   200, 1, 0, MINERAL,  HI_MINERAL),
RING("searching",         "tiger eye",  SEARCHING,      200, 1, 0, GEMSTONE, BROWN),
RING("stealth",           "jade",       STEALTH,        100, 1, 0, GEMSTONE, GREEN),
RING("levitation",        "agate",      LEVITATION,     200, 1, 0, GEMSTONE, RED),
RING("hunger",            "topaz",      HUNGER,         100, 1, 0, GEMSTONE, CYAN),
RING("aggravate monster", "sapphire",   AGGRAVATE_MONSTER,
							150, 1, 0, GEMSTONE, BLUE),
RING("conflict",          "ruby",       CONFLICT,       300, 1, 0, GEMSTONE, RED),
RING("warning",           "diamond",    WARNING,        100, 1, 0, GEMSTONE, WHITE),
RING("poison resistance", "pearl",      POISON_RES,     150, 1, 0, IRON,     WHITE),
RING("fire resistance",   "iron",       FIRE_RES,       200, 1, 0, IRON,     HI_METAL),
RING("cold resistance",   "brass",      COLD_RES,       150, 1, 0, COPPER,   HI_COPPER),
RING("shock resistance",  "copper",     SHOCK_RES,      150, 1, 0, COPPER,   HI_COPPER),
RING("teleportation",     "silver",     TELEPORT,       200, 1, 0, SILVER,   HI_SILVER),
RING("teleport control",  "gold",       TELEPORT_CONTROL,
							300, 1, 0, GOLD,     HI_GOLD),
#ifdef POLYSELF
RING("polymorph",         "ivory",      POLYMORPH,      300, 1, 0, 0,        WHITE),
RING("polymorph control", "emerald",    POLYMORPH_CONTROL,
							300, 1, 0, GEMSTONE, BRIGHT_GREEN),
#endif
RING("invisibility",      "wire",       INVIS,          150, 1, 0, IRON,     HI_METAL),
RING("see invisible",     "engagement", SEE_INVIS,      150, 1, 0, IRON,     HI_METAL),
RING("protection from shape changers", "shiny", PROT_FROM_SHAPE_CHANGERS,
							100, 1, 0, IRON,     BRIGHT_CYAN),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob) OBJECT( \
		OBJ(name,desc), BITS(0,0,0,0,1,0,0,0,0,0,IRON), power, \
		AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL )

AMULET("amulet of ESP",           "circular",   TELEPAT,    180),
AMULET("amulet of life saving",   "spherical",  LIFESAVED,   80),
AMULET("amulet of strangulation", "oval",       STRANGLED,  140),
AMULET("amulet of restful sleep", "triangular", SLEEPING,   140),
AMULET("amulet versus poison",    "pyramidal",  POISON_RES, 170),
AMULET("amulet of change",        "square",     0,          140),
						/* POLYMORPH */
AMULET("amulet of reflection",    "hexagonal",  REFLECTING,  80),
AMULET("amulet of magical breathing", "octagonal",	MAGICAL_BREATHING, 70),
OBJECT(OBJ("cheap plastic imitation of the Amulet of Yendor",
	"Amulet of Yendor"), BITS(0,0,1,0,0,0,0,0,0,0,PLASTIC), 0,
	AMULET_CLASS, 0, 0, 20,    0, 0, 0, 0, 0,  1, HI_METAL),
OBJECT(OBJ("Amulet of Yendor",NULL), BITS(1,0,1,0,1,0,1,1,0,0,MITHRIL), 0,
	AMULET_CLASS, 0, 0, 20, 3500, 0, 0, 0, 0, 20, HI_METAL),
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,material,color) OBJECT( \
		OBJ(name,desc), BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,material), 0, \
		TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, wt, color )
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,material,color) OBJECT( \
		OBJ(name,desc), BITS(kn,0,chg,1,mgc,chg,0,0,0,0,material), 0, \
		TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, wt, color )
#define WEPTOOL(name,desc,kn,mgc,bi,prob,wt,cost,sdam,ldam,hitbon,material,color) OBJECT( \
		OBJ(name,desc), BITS(kn,0,1,0,mgc,1,0,0,bi,0,material), 0, \
		TOOL_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, 0, wt, color )
/* containers */
CONTAINER("large box",	    NULL,   1, 0, 0,  40,350,   8, WOOD, HI_WOOD),
CONTAINER("chest",	    NULL,   1, 0, 0,  35,600,  16, WOOD, HI_WOOD),
CONTAINER("ice box",	    NULL,   1, 0, 0,   5,900,  42, PLASTIC, WHITE),
CONTAINER("sack",	    "bag",  0, 0, 0,  35, 15,   2, CLOTH, HI_CLOTH),
CONTAINER("oilskin sack",   "bag",  0, 0, 0,   5, 15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of holding", "bag",  0, 1, 0,  20, 15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of tricks",  "bag",  0, 1, 1,  20, 15, 100, CLOTH, HI_CLOTH),
#undef CONTAINER
/* lock opening tools */
TOOL("skeleton key",    "key",  0, 0, 0, 0,  80,  3,    10, IRON, HI_METAL),
#ifdef TOURIST
TOOL("lock pick",       NULL,   1, 0, 0, 0,  60,  4,    20, IRON, HI_METAL),
TOOL("credit card",     NULL,   1, 0, 0, 0,  15,  1,    10, PLASTIC, WHITE),
#else
TOOL("lock pick",       NULL,   1, 0, 0, 0,  75,  4,    20, IRON, HI_METAL),
#endif
/* light sources */
TOOL("tallow candle", "candle", 0, 1, 0, 0,  20,  2,    10, WAX, WHITE),
TOOL("wax candle",    "candle", 0, 1, 0, 0,   5,  2,    20, WAX, WHITE),
#ifdef WALKIES
TOOL("brass lantern",   NULL,   1, 0, 0, 0,  30,100,    10, COPPER, YELLOW),
TOOL("oil lamp",        "lamp", 0, 0, 0, 0,  45, 80,    10, COPPER, YELLOW),
#else
TOOL("brass lantern",   NULL,   1, 0, 0, 0,  45,100,    10, COPPER, YELLOW),
TOOL("oil lamp",        "lamp", 0, 0, 0, 0,  75, 80,    10, COPPER, YELLOW),
#endif
TOOL("magic lamp",      "lamp", 0, 0, 1, 0,  15, 80,    50, COPPER, YELLOW),
/* other tools */
#ifdef TOURIST
TOOL("expensive camera", NULL,  1, 0, 0, 0,  15, 30,   200, PLASTIC, BLACK),
TOOL("mirror", "looking glass", 0, 0, 0, 0,  45, 13,    10, GLASS, SILVER),
#else
TOOL("mirror", "looking glass", 0, 0, 0, 0,  60, 13,    10, GLASS, SILVER),
#endif
TOOL("crystal ball", "glass orb",
				0, 0, 1, 1,  15,150,	60, GLASS, HI_GLASS),
TOOL("blindfold",       NULL,   1, 0, 0, 0,  55,  2,    20, CLOTH, BLACK),
#ifdef WALKIES
TOOL("towel",           NULL,   1, 0, 0, 0,  50,  2,    50, CLOTH, MAGENTA),
TOOL("leash",           NULL,   1, 0, 0, 0,  70, 12,    20, LEATHER, HI_LEATHER),
#else
TOOL("towel",           NULL,   1, 0, 0, 0,  75,  3,    50, CLOTH, MAGENTA),
#endif
TOOL("stethoscope",     NULL,   1, 0, 0, 0,  25,  4,    75, IRON, HI_METAL),
TOOL("tinning kit",     NULL,   1, 0, 0, 0,  15,100,    30, IRON, HI_METAL),
TOOL("tin opener",      NULL,   1, 0, 0, 0,  35,  4,    30, IRON, HI_METAL),
TOOL("can of grease",   NULL,   1, 0, 0, 1,  15, 15,    20, IRON, HI_METAL),
TOOL("figurine",        NULL,   1, 0, 1, 0,  25, 50,    80, MINERAL, HI_MINERAL),
TOOL("magic marker",    NULL,   1, 0, 1, 1,  15,  2,    50, PLASTIC, RED),
/* instruments */
TOOL("tin whistle",   "whistle",
				0, 0, 0, 0, 105,  3,	10, METAL, HI_METAL),
TOOL("magic whistle", "whistle",
				0, 0, 1, 0,  30,  3,	10, METAL, HI_METAL),
/* "If tin whistles are made out of tin, what do they make foghorns out of?" */
TOOL("wooden flute",   "flute", 0, 0, 0, 0,   4,  5,    12, WOOD, HI_WOOD),
TOOL("magic flute",    "flute", 0, 0, 1, 1,   2,  5,    36, WOOD, HI_WOOD),
TOOL("tooled horn",     "horn", 0, 0, 0, 0,   5, 18,    15, BONE, WHITE),
TOOL("frost horn",      "horn", 0, 0, 1, 1,   2, 18,    50, BONE, WHITE),
TOOL("fire horn",       "horn", 0, 0, 1, 1,   2, 18,    50, BONE, WHITE),
TOOL("horn of plenty",  "horn", 0, 0, 1, 1,   2, 18,    50, BONE, WHITE),
TOOL("wooden harp",     "harp", 0, 0, 1, 0,   4, 30,    50, WOOD, HI_WOOD),
TOOL("magic harp",      "harp", 0, 0, 1, 1,   2, 30,    50, WOOD, HI_WOOD),
TOOL("bell",            NULL,   1, 0, 0, 0,   2, 30,    50, COPPER, HI_COPPER),
TOOL("bugle",           NULL,   1, 0, 0, 0,   4, 10,    15, COPPER, HI_COPPER),
TOOL("leather drum",    "drum", 0, 0, 0, 0,   4, 25,    25, LEATHER, HI_LEATHER),
TOOL("drum of earthquake", "drum",
				0, 0, 1, 1,   2, 25,	25, LEATHER, HI_LEATHER),
/* tools useful as weapons */
WEPTOOL("pick-axe",     NULL,   1, 0, 1,        20, 100,  50,
			 6,  3, 0,	IRON, HI_METAL),
WEPTOOL("unicorn horn", NULL,   1, 1, 1,         0,  20, 100,
			12, 12, 0,	BONE, WHITE),
/* two special, one of each kind, "tools" */
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum"),
		BITS(0,0,1,0,1,0,1,1,0,0,GOLD), 0,
		TOOL_CLASS, 0, 0,10, 3000, 0, 0, 0, 0, 200, HI_GOLD),
OBJECT(OBJ("Bell of Opening", "silver bell"),
		BITS(0,0,1,0,1,0,1,1,0,0,SILVER), 0,
		TOOL_CLASS, 0, 0,10, 1000, 0, 0, 0, 0, 50, HI_SILVER),
#undef TOOL
#undef WEPTOOL

/* comestibles ... */
#define FOOD(name,prob,delay,wt,uk,tin,nutrition,color) OBJECT( \
		OBJ(name,NULL), BITS(1,1,uk,0,0,0,0,0,0,0,tin), 0, \
		FOOD_CLASS, prob, delay, \
		wt, nutrition/20 + 5, 0, 0, 0, 0, nutrition, color )
/* all types of food (except tins & corpses) must have a delay of at least 1. */
/* delay on corpses is computed and is weight dependant */
/* dog eats foods 0-4 but prefers tripe rations above all others */
/* fortune cookies can be read */
/* carrots improve your vision */
/* +0 tins contain monster meat */
/* +1 tins (of spinach) make you stronger (like Popeye) */
/* food CORPSE is a cadaver of some type */

	/* meat */
	FOOD("tripe ration",       142, 2, 10, 0, FLESH, 200, BROWN),
	FOOD("corpse",               0, 1,  0, 0, FLESH,   0, BROWN),
	FOOD("egg",                 85, 1,  1, 1, FLESH,  80, WHITE),
	/* fruits & veggies */
	FOOD("apple",               15, 1,  2, 0, VEGGY,  50, RED),
	FOOD("orange",              10, 1,  2, 0, VEGGY,  80, ORANGE_COLORED),
	FOOD("pear",                10, 1,  2, 0, VEGGY,  50, BRIGHT_GREEN),
	FOOD("melon",               10, 1,  5, 0, VEGGY, 100, BRIGHT_GREEN),
	FOOD("banana",              10, 1,  2, 0, VEGGY,  80, YELLOW),
	FOOD("carrot",              15, 1,  2, 0, VEGGY,  50, ORANGE_COLORED),
	FOOD("sprig of wolfsbane",   7, 1,  1, 0, VEGGY,  40, GREEN),
	FOOD("clove of garlic",      7, 1,  1, 0, VEGGY,  40, WHITE),
#ifdef TUTTI_FRUTTI
	FOOD("slime mold",          75, 1,  5, 0, VEGGY, 250, HI_ORGANIC),
#else
	FOOD("slice of pizza",      75, 1,  3, 0, VEGGY, 250, RED),
#endif
	/* human food */
	FOOD("lump of royal jelly",  0, 1,  2, 0, VEGGY, 200, YELLOW),
	FOOD("cream pie",           25, 1, 10, 0, VEGGY, 100, WHITE),
	FOOD("candy bar",           13, 1,  2, 0, VEGGY, 100, BROWN),
	FOOD("fortune cookie",      55, 1,  1, 0, VEGGY,  40, YELLOW),
	FOOD("pancake",             25, 2,  2, 0, VEGGY, 200, YELLOW),
	FOOD("lembas wafer",        20, 2,  5, 0, VEGGY, 800, WHITE),
	FOOD("cram ration",         20, 3, 15, 0, VEGGY, 600, HI_ORGANIC),
	FOOD("food ration",        381, 5, 20, 0, VEGGY, 800, HI_ORGANIC),
#ifdef ARMY
	FOOD("K-ration",             0, 1, 10, 0, VEGGY, 400, HI_ORGANIC),
	FOOD("C-ration",             0, 1, 10, 0, VEGGY, 300, HI_ORGANIC),
#endif
	FOOD("tin",                 75, 0, 10, 1, METAL,   0, HI_METAL),
#undef FOOD

/* potions ... */
#define POTION(name,desc,mgc,power,prob,cost,color) OBJECT( \
		OBJ(name,desc), BITS(0,1,0,0,mgc,0,0,0,0,0,GLASS), power, \
		POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color )
POTION("gain ability",      "ruby",           1, 0,          45, 300, RED),
POTION("restore ability",   "pink",           1, 0,          45, 100, BRIGHT_MAGENTA),
POTION("confusion",         "orange",         1, CONFUSION,  45, 100, ORANGE_COLORED),
POTION("blindness",         "yellow",         1, BLINDED,    45, 150, YELLOW),
POTION("paralysis",         "emerald",        1, 0,          45, 300, BRIGHT_GREEN),
POTION("speed",             "dark green",     1, FAST,       45, 200, GREEN),
POTION("levitation",        "cyan",           1, LEVITATION, 45, 200, CYAN),
POTION("hallucination",     "sky blue",       1, HALLUC,     45, 100, CYAN),
POTION("invisibility",      "brilliant blue", 1, INVIS,      45, 150, BRIGHT_BLUE),
POTION("see invisible",     "magenta",        1, SEE_INVIS,  45,  50, MAGENTA),
POTION("healing",           "purple-red",     1, 0,          65, 100, MAGENTA),
POTION("extra healing",     "puce",           1, 0,          50, 100, RED),
POTION("gain level",        "milky",          1, 0,          20, 300, WHITE),
POTION("enlightenment",     "swirly",         1, 0,          20, 200, BROWN),
POTION("monster detection", "bubbly",         1, 0,          45, 150, WHITE),
POTION("object detection",  "smoky",          1, 0,          45, 150, GRAY),
POTION("gain energy",       "cloudy",         1, 0,          45, 150, WHITE),
POTION("booze",             "brown",          0, 0,          45,  50, BROWN),
POTION("sickness",          "fizzy",          0, 0,          45,  50, CYAN),
POTION("fruit juice",       "dark",           0, 0,          45,  50, BLACK),
POTION("water",             "clear",          0, 0,         125, 100, CYAN),
#undef POTION

/* scrolls ... */
#define SCROLL(name,text,mgc,prob,cost) OBJECT( \
		OBJ(name,text), BITS(0,1,0,0,mgc,0,0,0,0,0,PAPER), 0, \
		SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, HI_PAPER )
	SCROLL("enchant armor",         "ZELGO MER",            1,  63,  80),
	SCROLL("destroy armor",         "JUYED AWK YACC",       1,  45, 100),
	SCROLL("confuse monster",       "NR 9",                 1,  53, 100),
	SCROLL("scare monster",         "XIXAXA XOXAXA XUXAXA", 1,  35, 100),
	SCROLL("remove curse",          "PRATYAVAYAH",          1,  65,  80),
	SCROLL("enchant weapon",        "DAIYEN FOOELS",        1,  85,  60),
	SCROLL("create monster",        "LEP GEX VEN ZEA",      1,  45, 200),
	SCROLL("taming",                "PRIRUTSENIE",          1,  15, 200),
	SCROLL("genocide",              "ELBIB YLOH",           1,  15, 300),
	SCROLL("light",                 "VERR YED HORRE",       1,  95,  50),
	SCROLL("teleportation",         "VENZAR BORGAVVE",      1,  55, 100),
	SCROLL("gold detection",        "THARR",                1,  33, 100),
	SCROLL("food detection",        "YUM YUM",              1,  25, 100),
	SCROLL("identify",              "KERNOD WEL",           1, 185,  20),
	SCROLL("magic mapping",         "ELAM EBOW",            1,  45, 100),
	SCROLL("amnesia",               "DUAM XNAHT",           1,  35, 200),
	SCROLL("fire",                  "ANDOVA BEGARIN",       1,  48, 100),
	SCROLL("punishment",            "VE FORBRYDERNE",       1,  15, 300),
	SCROLL("charging",              "HACKEM MUCHE",         1,  15, 300),
	SCROLL(NULL,			"VELOX NEB",            1,   0, 100),
	SCROLL(NULL,			"FOOBIE BLETCH",        1,   0, 100),
	SCROLL(NULL,			"TEMOV",                1,   0, 100),
	SCROLL(NULL,			"GARVEN DEH",           1,   0, 100),
	SCROLL(NULL,			"READ ME",              1,   0, 100),
	SCROLL(NULL,			"KIRJE",                1,   0, 100),
	/* these must come last because they have special descriptions */
#ifdef MAIL
	SCROLL("mail",                  "stamped",          0,   0,   0),
#endif
	SCROLL("blank paper",           "unlabeled",        0,  28,  60),
#undef SCROLL

/* spellbooks ... */
#define SPELL(name,desc,prob,delay,level,mgc,dir,color) OBJECT( \
		OBJ(name,desc), BITS(0,0,0,0,mgc,0,0,0,0,dir,PAPER), 0, \
		SPBOOK_CLASS, prob, delay, \
		50, level*100, 0, 0, 0, level, 20, color )
SPELL("dig",             "parchment",   22,  6, 5, 1, RAY,       HI_PAPER),
SPELL("magic missile",   "vellum",      45,  3, 2, 1, RAY,       HI_PAPER),
SPELL("fireball",        "ragged",      20,  6, 4, 1, RAY,       HI_PAPER),
SPELL("cone of cold",    "dog eared",   10,  8, 5, 1, RAY,       HI_PAPER),
SPELL("sleep",           "mottled",     50,  1, 1, 1, RAY,       HI_PAPER),
SPELL("finger of death", "stained",      5, 10, 7, 1, RAY,       HI_PAPER),
SPELL("light",           "cloth",       45,  1, 1, 1, NODIR,     HI_CLOTH),
SPELL("detect monsters", "leather",     45,  1, 1, 1, NODIR,     HI_LEATHER),
SPELL("healing",         "white",       40,  2, 1, 1, IMMEDIATE, WHITE),
SPELL("knock",           "pink",        36,  1, 1, 1, IMMEDIATE, BRIGHT_MAGENTA),
SPELL("force bolt",      "red",         35,  2, 1, 1, IMMEDIATE, RED),
SPELL("confuse monster", "orange",      37,  2, 2, 1, IMMEDIATE, ORANGE_COLORED),
SPELL("cure blindness",  "yellow",      27,  2, 2, 1, IMMEDIATE, YELLOW),
SPELL("slow monster",    "light green", 37,  2, 2, 1, IMMEDIATE, BRIGHT_GREEN),
SPELL("wizard lock",     "dark green",  35,  3, 2, 1, IMMEDIATE, GREEN),
SPELL("create monster",  "turquoise",   37,  3, 2, 1, NODIR,     BRIGHT_CYAN),
SPELL("detect food",     "cyan",        37,  3, 2, 1, NODIR,     CYAN),
SPELL("cause fear",      "light blue",  25,  3, 3, 1, NODIR,     BRIGHT_BLUE),
SPELL("clairvoyance",    "dark blue",   15,  3, 3, 1, NODIR,     BLUE),
SPELL("cure sickness",   "indigo",      32,  3, 3, 1, NODIR,     BLUE),
SPELL("charm monster",   "magenta",     20,  3, 3, 1, IMMEDIATE, MAGENTA),
SPELL("haste self",      "purple",      33,  4, 3, 1, NODIR,     MAGENTA),
SPELL("detect unseen",   "violet",      20,  4, 3, 1, NODIR,     MAGENTA),
SPELL("levitation",      "tan",         20,  4, 4, 1, NODIR,     BROWN),
SPELL("extra healing",   "plaid",       35,  5, 3, 1, IMMEDIATE, GREEN),
SPELL("restore ability", "light brown", 25,  5, 4, 1, NODIR,     BROWN),
SPELL("invisibility",    "dark brown",  32,  5, 4, 1, NODIR,     BROWN),
SPELL("detect treasure", "gray",        25,  5, 4, 1, NODIR,     GRAY),
SPELL("remove curse",    "white",       25,  5, 5, 1, NODIR,     WHITE),
SPELL("magic mapping",   "dusty",       18,  7, 5, 1, NODIR,     HI_PAPER),
SPELL("identify",        "bronze",      25,  8, 5, 1, NODIR,     HI_COPPER),
SPELL("turn undead",     "copper",      17,  8, 6, 1, IMMEDIATE, HI_COPPER),
SPELL("polymorph",       "silver",      10,  8, 6, 1, IMMEDIATE, HI_SILVER),
SPELL("teleport away",   "gold",        15,  6, 6, 1, IMMEDIATE, HI_GOLD),
SPELL("create familiar", "glittering",  10,  7, 6, 1, NODIR,     WHITE),
SPELL("cancellation",    "shining",     15,  8, 7, 1, IMMEDIATE, WHITE),
SPELL(NULL,		 "dull",         0,  0, 0, 1, 0,         HI_PAPER),
SPELL(NULL,		 "thin",         0,  0, 0, 1, 0,         HI_PAPER),
SPELL(NULL,		 "thick",        0,  0, 0, 1, 0,         HI_PAPER),
/* blank spellbook must come last because it retains its description */
SPELL("blank paper",     "plain",       20,  0, 0, 0, 0,         HI_PAPER),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Book of the Dead", "papyrus"), BITS(0,0,1,0,1,0,1,1,0,0,PAPER), 0,
	SPBOOK_CLASS, 0, 0,20, 3500, 0, 0, 0, 7, 20, HI_PAPER),
#undef SPELL

/* wands ... */
#define WAND(name,typ,prob,cost,mgc,dir,metal,color) OBJECT( \
		OBJ(name,typ), BITS(0,0,1,0,mgc,1,0,0,0,dir,metal), 0, WAND_CLASS, \
		prob, 0, 7, cost, 0, 0, 0, 0, 30, color )
WAND("light",           "glass",        95, 100, 1, NODIR,     GLASS,    HI_GLASS),
WAND("secret door detection", "balsa",  50, 150, 1, NODIR,     WOOD,     HI_WOOD),
WAND("create monster",  "maple",        45, 200, 1, NODIR,     WOOD,     HI_WOOD),
WAND("wishing",         "pine",          5, 500, 1, NODIR,     WOOD,     HI_WOOD),
WAND("nothing",         "oak",          25, 100, 0, IMMEDIATE, WOOD,     HI_WOOD),
WAND("striking",        "ebony",        75, 150, 1, IMMEDIATE, WOOD,     HI_WOOD),
WAND("make invisible",  "marble",       45, 150, 1, IMMEDIATE, MINERAL,  HI_MINERAL),
WAND("slow monster",    "tin",          55, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("speed monster",   "brass",        55, 150, 1, IMMEDIATE, COPPER,   HI_COPPER),
WAND("undead turning",  "copper",       55, 150, 1, IMMEDIATE, COPPER,   HI_COPPER),
WAND("polymorph",       "silver",       45, 200, 1, IMMEDIATE, SILVER,   HI_SILVER),
WAND("cancellation",    "platinum",     45, 200, 1, IMMEDIATE, PLATINUM, WHITE),
WAND("teleportation",   "iridium",      45, 200, 1, IMMEDIATE, METAL,    BRIGHT_CYAN),
WAND("opening",         "zinc",         25, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("locking",         "aluminum",     25, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("probing",         "uranium",      30, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("digging",         "iron",         55, 150, 1, RAY,       IRON,     HI_METAL),
WAND("magic missile",   "steel",        50, 150, 1, RAY,       IRON,     HI_METAL),
WAND("fire",            "hexagonal",    40, 175, 1, RAY,       IRON,     HI_METAL),
WAND("cold",            "short",        40, 175, 1, RAY,       IRON,     HI_METAL),
WAND("sleep",           "runed",        50, 175, 1, RAY,       IRON,     HI_METAL),
WAND("death",           "long",          5, 500, 1, RAY,       IRON,     HI_METAL),
WAND("lightning",       "curved",       40, 175, 1, RAY,       IRON,     HI_METAL),
WAND(NULL,		"forked",        0, 150, 1, 0,         WOOD,     HI_WOOD),
WAND(NULL,		"spiked",        0, 150, 1, 0,         IRON,     HI_METAL),
WAND(NULL,		"jeweled",       0, 150, 1, 0,         IRON,     HI_MINERAL),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(name,prob,metal) OBJECT( \
		OBJ(name,NULL), BITS(0,1,0,0,0,0,0,0,0,0,metal), 0, \
		GOLD_CLASS, prob, 0, 1, 0, 0, 0, 0, 0, 0, HI_GOLD )
	COIN("gold piece",      1000, GOLD),
#undef COIN

/* gems ... - includes stones but not boulders */
#define GEM(name,desc,prob,wt,gval,nutr,glass,color) OBJECT( \
		OBJ(name,desc), BITS(0,1,0,0,0,0,0,0,0,0,glass), 0, \
		GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, WP_SLING, nutr, color )
#define ROCK(name,desc,kn,prob,wt,gval,mgc,nutr,glass,color) OBJECT( \
		OBJ(name,desc), BITS(kn,1,0,0,mgc,0,0,0,0,0,glass), 0, \
		GEM_CLASS, prob, 0, wt, gval, 3, 3, 0, WP_SLING, nutr, color )
GEM("dilithium crystal", "white",        3, 1, 4500, 15, GEMSTONE, WHITE),
GEM("diamond", "white",                  4, 1, 4000, 15, GEMSTONE, WHITE),
GEM("ruby", "red",                       5, 1, 3500, 15, GEMSTONE, RED),
GEM("sapphire", "blue",                  6, 1, 3000, 15, GEMSTONE, BLUE),
GEM("emerald", "green",                  7, 1, 2500, 15, GEMSTONE, GREEN),
GEM("turquoise", "green",                8, 1, 2000, 15, GEMSTONE, GREEN),
GEM("aquamarine", "green",              10, 1, 1500, 15, GEMSTONE, GREEN),
GEM("amber", "yellowish brown",         11, 1, 1000, 15, GEMSTONE, BROWN),
GEM("topaz", "yellowish brown",         13, 1,  900, 15, GEMSTONE, BROWN),
GEM("opal", "white",                    15, 1,  800, 15, GEMSTONE, WHITE),
GEM("garnet", "red",                    17, 1,  700, 15, GEMSTONE, RED),
GEM("amethyst", "violet",               19, 1,  600, 15, GEMSTONE, MAGENTA),
GEM("jasper", "red",                    21, 1,  500, 15, GEMSTONE, RED),
GEM("fluorite", "violet",               22, 1,  400, 15, GEMSTONE, MAGENTA),
GEM("jade", "green",                    23, 1,  300, 15, GEMSTONE, GREEN),
GEM("worthless piece of white glass", "white",  116, 1, 0, 6, GLASS, WHITE),
GEM("worthless piece of blue glass", "blue",    116, 1, 0, 6, GLASS, BLUE),
GEM("worthless piece of red glass", "red",      116, 1, 0, 6, GLASS, RED),
GEM("worthless piece of yellowish brown glass", "yellowish brown",
						116, 1, 0, 6, GLASS, BROWN),
GEM("worthless piece of green glass", "green",  116, 1, 0, 6, GLASS, GREEN),
GEM("worthless piece of violet glass", "violet",116, 1, 0, 6, GLASS, MAGENTA),
ROCK("luckstone", "gray",                0, 10, 10, 60, 1, 10, MINERAL, GRAY),
ROCK("loadstone", "gray",                0, 10,500,  1, 1, 10, MINERAL, GRAY),
ROCK("rock", NULL,                       1,100, 10,  0, 0, 10, MINERAL, GRAY),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders and statues weigh more than MAX_CARR_CAP.
 */
OBJECT(OBJ("boulder",NULL), BITS(1,0,0,0,0,0,0,0,1,0,MINERAL), 0,
		ROCK_CLASS,   100, 0, 6000,  0, 20, 20, 0, 0, 2000, HI_MINERAL),
OBJECT(OBJ("statue", NULL), BITS(1,0,0,1,0,0,0,0,0,0,MINERAL), 0,
		ROCK_CLASS,   900, 0, 2500,  0, 20, 20, 0, 0, 2500, WHITE),
OBJECT(OBJ("heavy iron ball", NULL), BITS(1,0,0,0,0,0,0,0,0,0,IRON), 0,
		BALL_CLASS,  1000, 0,  480, 10,  0,  0, 0, 0,  200, HI_METAL),
OBJECT(OBJ("iron chain", NULL), BITS(1,0,0,0,0,0,0,0,0,0,IRON), 0,
		CHAIN_CLASS, 1000, 0,  120,  0,  0,  0, 0, 0,  200, HI_METAL),
OBJECT(OBJ("blinding venom", "splash of venom"),
		BITS(0,1,0,0,0,0,0,1,0,0,LIQUID), 0,
		VENOM_CLASS,  500, 0,	 1,  0,  0,  0, 0, 0,	 0, HI_ORGANIC),
OBJECT(OBJ("acid venom", "splash of venom"),
		BITS(0,1,0,0,0,0,0,1,0,0,LIQUID), 0,
		VENOM_CLASS,  500, 0,	 1,  0,  6,  6, 0, 0,	 0, HI_ORGANIC),
		/* +d6 small or large */

/* fencepost -- name [1st arg] *must* be NULL */
	OBJECT(OBJ(NULL,NULL), BITS(0,0,0,0,0,0,0,0,0,0,0), 0,
		ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
}; /* objects[] */

#ifndef OBJECTS_PASS_2_

/* perform recursive compilation for second structure */
#  undef OBJ
#  undef OBJECT
#  define OBJECTS_PASS_2_
#include "objects.c"

void NDECL(objects_init);

/* dummy routine used to force linkage */
void
objects_init()
{
    return;
}

#endif	/* !OBJECTS_PASS_2_ */

/*objects.c*/
