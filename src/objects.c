/*	SCCS Id: @(#)objects.c	3.0	89/04/14
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright (c) Mike Threepoint, 1989 (890110) */

/* since this file is also used in auxiliary programs, don't include all the 
 * function declarations for all of nethack
 */
#define EXTERN_H
#include "config.h"
#include "obj.h"
#include "objclass.h"
#include "prop.h"

/* objects have letter " % ) ( 0 _ ` [ ! ? / = * + . */

struct objclass objects[] = {

	{ "strange object", NULL, NULL, 1,0,0,0, 0,
		ILLOBJ_SYM, 0, 0, 0, 0, 0, 0, 0 },
/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob,weight)	{ name, desc, NULL,\
		0,0,0,METAL, power, AMULET_SYM, prob, 0, weight, 100, 0, 0, 0 }

	AMULET("amulet of esp", 	  "circular",	TELEPAT,    190, 2),
	AMULET("amulet of life saving",   "spherical",	LIFESAVED,   90, 2),
	AMULET("amulet of strangulation", "oval",	STRANGLED,  150, 2),
	AMULET("amulet of restful sleep", "triangular",	SLEEPING,   150, 2),
	AMULET("amulet versus poison",	  "pyramidal",	POISON_RES, 180, 2),
	AMULET("amulet of change",	  "square",	0,	    150, 2),
								/* POLYMORPH */
	AMULET("amulet of reflection",	  "hexagonal",	REFLECTING,  90, 2),
	{ "Amulet of Yendor", NULL, NULL, 1,0,0,METAL, 0,
		AMULET_SYM, 0, 0, 2, 3500, 0, 0, 0 },
#undef AMULET

#define FOOD(name,prob,delay,wt,tin,nutrition)	{ name, NULL, NULL, 1,1,0,tin,\
	    0, FOOD_SYM, prob, delay, wt, nutrition/20 + 5, 0, 0, nutrition }

/* dog eats foods 0-4 but prefers tripe rations above all others */
/* fortune cookies can be read */
/* carrots improve your vision */
/* +0 tins contain monster meat */
/* +1 tins (of spinach) make you stronger (like Popeye) */
/* food CORPSE is a cadaver of some type */

	/* meat */
#ifdef TOLKIEN
	FOOD("tripe ration",	   145, 1, 2, 0, 200),
#else
	FOOD("tripe ration",	   150, 1, 2, 0, 200),
#endif
	FOOD("dead lizard",	    35, 0, 1, 0,  40),
	FOOD("corpse",		     0, 0, 0, 0,   0),
	FOOD("egg",		    75, 0, 1, 0,  80),
	/* fruits & veggies */
	FOOD("apple",		    10, 0, 1, 0,  50),
	FOOD("orange",		     7, 0, 1, 0,  80),
	FOOD("pear",		     7, 0, 1, 0,  50),
	FOOD("melon",		     7, 0, 1, 0, 100),
	FOOD("banana",		     7, 0, 1, 0,  80),
#ifdef TUTTI_FRUTTI
	FOOD("slime mold",	    75, 0, 1, 0, 250),
#else
	FOOD("slice of pizza",	    75, 0, 1, 0, 250),
#endif
	FOOD("carrot",		    15, 0, 1, 0,  50),
	FOOD("clove of garlic",      5, 0, 1, 0,  40),
	/* human food */
	FOOD("lump of royal jelly",  0, 0, 1, 0, 200),
	FOOD("cream pie",	    25, 0, 1, 0, 100),
	FOOD("candy bar",	     7, 0, 1, 0, 100),
	FOOD("fortune cookie",	    55, 0, 1, 0,  40),
#ifdef TOLKIEN
	FOOD("pancake", 	    25, 1, 1, 0, 200),
	FOOD("lembas wafer",	    20, 1, 1, 0, 800),
	FOOD("cram ration",	    20, 1, 3, 0, 600),
	FOOD("food ration",	   385, 5, 4, 0, 800),
#else
	FOOD("pancake", 	    40, 1, 1, 0, 200),
	FOOD("food ration",	   405, 5, 4, 0, 800),
#endif
#ifdef ARMY
	FOOD("K-ration",	     0, 1, 1, 0, 400),
	FOOD("C-ration",	     0, 1, 1, 0, 300),
#endif
	FOOD("tin",		    75, 0, 1, METAL, 0),
#undef FOOD

/* weapons ... - ROCK come several at a time */
/* weapons ... - (DART-1) are shot using idem+(BOW-ARROW) */
/* weapons AXE, SWORD, KATANA, THSWORD are good for worm-cutting */
/* weapons (PICK-)AXE, DAGGER, CRYSKNIFE are good for tin-opening */
#define WEAPON(name,app,kn,mg,bi,prob,wt,cost,sdam,ldam,metal) { name, app, \
	NULL, kn,mg,bi,metal, 0, WEAPON_SYM, prob, 0, wt, cost, sdam, ldam, 0 }
#define PROJECTILE(name,app,kn,bi,prob,wt,cost,sdam,ldam,metal,prop) { name, \
	app, NULL, kn,1,bi,metal, 0, WEAPON_SYM, prob, 0, wt, cost, sdam, \
	ldam, prop }
#define BOW(name,app,kn,bi,prob,wt,cost,sdam,ldam,metal,prop) { name, app, \
	NULL, kn,0,bi,metal, 0, WEAPON_SYM, prob, 0, wt, cost, sdam, ldam, \
	-(prop) }

/* Note: for weapons that don't do an even die of damage (i.e. 2-7 or 3-18)
 * the extra damage is added on in weapon.c, not here! */

/* missiles */
#ifdef TOLKIEN
PROJECTILE("arrow",		NULL,		1, 0, 45, 0,  2, 6, 6,
		   METAL, WP_BOW),
PROJECTILE("elven arrow",	"runed arrow",	0, 0, 10, 0,  2, 7, 6,
		   METAL, WP_BOW),
PROJECTILE("orcish arrow",	"black arrow",	0, 0, 11, 0,  2, 5, 6,
		   METAL, WP_BOW),
#else
PROJECTILE("arrow",		NULL,		1, 0, 66, 0,  2, 6, 6,
		   METAL, WP_BOW),
#endif
PROJECTILE("crossbow bolt",	NULL,		1, 0, 60, 0,  2, 4, 6,
		   METAL, WP_CROSSBOW),

WEAPON("dart",		NULL,		1, 1, 0, 60, 0,  2, 3, 2, METAL),
WEAPON("shuriken",	"throwing star",0, 1, 0, 30, 0,  2, 8, 6, METAL),
WEAPON("boomerang",	NULL,		1, 1, 0, 15, 3, 50, 9, 9, WOOD),

/* spears */
#ifdef TOLKIEN
WEAPON("spear", 	NULL,		1, 1, 0, 55, 3,  8, 6, 8, METAL),
WEAPON("elven spear",	"runed spear",	0, 1, 0, 10, 3,  8, 7, 8, METAL),
WEAPON("orcish spear",	"black spear",	0, 1, 0, 13, 3,  8, 5, 8, METAL),
WEAPON("dwarvish spear","shiny spear",	0, 1, 0, 12, 3,  8, 8, 8, METAL),
#else
WEAPON("spear", 	NULL,		1, 1, 0, 90, 3,  8, 6, 8, METAL),
#endif
WEAPON("javelin",	"throwing spear",0,1, 0, 10, 3,  8, 6, 6, METAL),
WEAPON("trident",	NULL,		1, 0, 0,  8, 4,  6, 6, 4, METAL),
						/* +1 small, +2d4 large */
WEAPON("lance", 	NULL,		1, 0, 0,  8, 4, 20, 6, 8, METAL),

/* blades */
#ifdef TOLKIEN
WEAPON("dagger",	NULL,		1, 1, 0, 25, 2, 20, 4, 3, METAL),
WEAPON("elven dagger",	"large runed knife", 0, 1, 0,  8, 2, 20, 5, 3, METAL),
WEAPON("orcish dagger", "large black knife", 0, 1, 0, 10, 2, 20, 3, 3, METAL),
#else
WEAPON("dagger",	NULL,		1, 1, 0, 43, 2, 20, 4, 3, METAL),
#endif
WEAPON("scalpel",	NULL,		1, 1, 0,  0, 1, 20, 4, 3, METAL),
WEAPON("knife", 	NULL,		1, 1, 0, 25, 2, 15, 3, 3, METAL),
WEAPON("axe",		NULL,		1, 0, 0, 50, 3, 50, 6, 4, METAL),
#ifdef WORM
WEAPON("worm tooth",	NULL,		1, 0, 0,  0, 3,  2, 2, 2, METAL),
WEAPON("crysknife",	NULL,		1, 0, 0,  0, 3,100,10,10, METAL),
#endif

/* swords */
#ifdef TOLKIEN
WEAPON("short sword",		NULL,	1, 0, 0,  6, 3, 80, 6, 8, METAL),
WEAPON("elven short sword",	"short runed sword",
					0, 0, 0,  2, 3, 80, 8, 8, METAL),
WEAPON("orcish short sword",	"short black sword",
					0, 0, 0,  3, 3, 80, 5, 8, METAL),
WEAPON("dwarvish short sword",	"short shiny sword",
					0, 0, 0,  2, 3, 80, 7, 8, METAL),
#else
WEAPON("short sword",		NULL,	1, 0, 0, 13, 3, 80, 6, 8, METAL),
#endif
WEAPON("scimitar", "curved sword",	0, 0, 0,  6, 4, 80, 8, 8, METAL),
#ifdef TOLKIEN
WEAPON("broadsword", "wide sword",	0, 0, 0,  8, 4, 80, 4, 6, METAL),
						/* +d4 small, +1 large */
WEAPON("elven broadsword",	"wide runed sword",
					0, 0, 0,  4, 4, 80, 6, 6, METAL),
						/* +d4 small, +1 large */
#else
WEAPON("broadsword", "wide sword",	0, 0, 0, 12, 4, 80, 4, 6, METAL),
						/* +d4 small, +1 large */
#endif
WEAPON("long sword",		NULL,	1, 0, 0, 60, 4, 80, 8, 12, METAL),
#ifdef TOLKIEN
WEAPON("two-handed sword",	NULL,	1, 0, 1, 25, 5, 80,12,	6, METAL),
						/* +2d6 large */
WEAPON("dwarvish mattock",	"huge shiny sword",
					0, 0, 1, 15, 6, 80,12,	8, METAL),
						/* +2d6 large */
#else
WEAPON("two-handed sword",	NULL,	1, 0, 1, 40, 5, 80,12,	6, METAL),
						/* +2d6 large */
#endif
WEAPON("katana", "samurai sword",	0, 0, 0,  6, 4,100,10, 12, METAL),

/* blunt */
WEAPON("mace",		NULL,		1, 0, 0, 55, 4, 150, 6, 6, METAL),
						/* +1 small */
WEAPON("morning star",	NULL,		1, 0, 0, 12, 4, 120, 4, 6, METAL),
						/* +d4 small, +1 large */
WEAPON("club",		NULL,		1, 0, 0, 10, 3, 100, 6, 3, WOOD),
#ifdef KOPS
WEAPON("rubber hose",	NULL,		1, 0, 0,  0, 2, 100, 6, 3, 0),
#endif
WEAPON("quarterstaff",	"staff",	0, 0, 1, 10, 3, 150, 6, 6, WOOD),

/* two-piece */
WEAPON("aklys", "thonged club", 	0, 0, 0,  8, 3, 30, 6, 3, METAL),
WEAPON("flail", 	NULL,		1, 0, 0, 40, 3, 30, 6, 4, METAL),
						/* +1 small, +1d4 large */
/* whip */
WEAPON("bullwhip",	NULL,		1, 0, 0,  5, 2, 20, 2, 1, 0),

/* polearms */
WEAPON("bardiche", "large poleaxe",	0, 0, 1,  8, 3, 70, 4, 4, METAL),
						/* +1d4 small, +2d4 large */
WEAPON("bec de corbin","beaked poleaxe",0, 0, 1,  8, 3, 16, 8, 6, METAL),
WEAPON("bill-guisarme","hooked polearm",0, 0, 1,  8, 3, 60, 4,10, METAL),
						/* +1d4 small */
WEAPON("fauchard",	"sickle",	0, 0, 1, 11, 3, 30, 6, 8, METAL),
WEAPON("glaive",	"pike", 	0, 0, 1, 15, 3, 60, 6,10, METAL),
WEAPON("guisarme",	"pruning hook", 0, 0, 1, 11, 3, 50, 4, 8, METAL),
						/* +1d4 small */
WEAPON("halberd",	"long poleaxe", 0, 0, 1, 16, 3, 90,10, 6, METAL),
						/* +1d6 large */
WEAPON("lucern hammer", "hammerhead polearm", 0, 0, 1, 10, 3, 70, 4, 6, METAL),
						/* +1d4 small */
WEAPON("partisan", "vulgar polearm",	0, 0, 1, 10, 3,100, 6, 6, METAL),
						/* +1 large */
WEAPON("ranseur", "hilted polearm",	0, 0, 1, 10, 3, 40, 4, 4, METAL),
						/* +d4 both */
WEAPON("spetum", "forked polearm",	0, 0, 1, 10, 3, 30, 6, 6, METAL),
						/* +1 small, +d6 large */
WEAPON("voulge", "poleaxe",		0, 0, 1,  8, 3, 20, 4, 4, METAL),
						/* +d4 both */
/* bows */
#ifdef TOLKIEN
BOW("bow",	  NULL, 	1, 0, 24, 3, 120, 4, 6, 0, WP_BOW),
BOW("elven bow",  "runed bow",	0, 0, 12, 3, 120, 5, 6, 0, WP_BOW),
BOW("orcish bow", "black bow",	0, 0, 12, 3, 120, 3, 6, 0, WP_BOW),
#else
BOW("bow",	  NULL, 	1, 0, 48, 3, 120, 4, 6, 0, WP_BOW),
#endif
BOW("sling", 	  NULL,	    	1, 0, 40, 2,  20, 6, 6, 0, WP_SLING),
BOW("crossbow",	  NULL,		1, 0, 45, 3,  40, 4, 6, 0, WP_CROSSBOW),
#undef WEAPON
#undef PROJECTILE
#undef BOW

/* tools ... - PICK AXE comes last because it has special characteristics */
#define TOOL(name,desc,kn,charge,prob,weight,cost,material) { name, desc, NULL,\
	kn,0,charge,material, 0, TOOL_SYM, prob, 0, weight, cost, 0, 0, 0 }

#ifdef WALKIES
	TOOL("leash",		NULL,		1, 0,  70,  3,	20, 0),
#endif
#ifdef MEDUSA
	TOOL("blindfold",	NULL,		1, 0,  55,  2,	20, 0),
	TOOL("mirror",		NULL,		1, 0,  50,  3,	40, GLASS),
#else
	TOOL("blindfold",	NULL,		1, 0, 105,  2,	20, 0),
#endif
	TOOL("tinning kit",	NULL,		1, 0,  15, 10,	30, METAL),
	TOOL("lock pick",	NULL,		1, 0,  55,  2,	20, METAL),
	TOOL("credit card",	NULL,		1, 0,	5,  1,	10, 0),
#ifdef WALKIES
	TOOL("key",		NULL,		1, 0, 100,  1,	10, METAL),
#else
	TOOL("key",		NULL,		1, 0, 155,  1,	10, METAL),
#endif
	TOOL("skeleton key",	"key",		0, 0,  60,  1,	10, METAL),
	TOOL("expensive camera", NULL,		1, 0,	5,  3, 200, 0),
	TOOL("magic marker",	NULL,		1, 1,  15,  1,	50, 0),
	TOOL("stethoscope",	NULL,		1, 0,  15,  2,	80, 0),
	TOOL("tin opener",	NULL,		1, 0,  25,  1,	30, METAL),
#ifdef WALKIES
	TOOL("lamp",		NULL,		1, 1,  90, 10,	50, 0),
#else
	TOOL("lamp",		NULL,		1, 1, 105, 10,	50, 0),
#endif
	TOOL("magic lamp",	"lamp", 	0, 1,  20, 10,	50, 0),
	TOOL("crystal ball",	NULL,		1, 1,  35, 15,	60, GLASS),
	TOOL("figurine",	NULL,		1, 0,  35,  5,	80, MINERAL),
	TOOL("ice box", 	NULL,		1, 0,	5, 40, 400, 0),
	TOOL("large box",	NULL,		1, 0,  40, 40, 400, WOOD),
	TOOL("chest",		NULL,		1, 0,  35, 40, 500, WOOD),
	TOOL("sack",		"bag",		0, 0,  40,  3, 200, 0),
	TOOL("bag of holding",	"bag",		0, 0,  20,  3, 250, 0),
	TOOL("bag of tricks",	"bag",		0, 1,  20,  3, 250, 0),
#ifndef MUSIC
	TOOL("whistle", 	NULL,		1, 0, 120,  2,	10, METAL),
	TOOL("magic whistle",	"whistle",	0, 0,  50,  2,	10, METAL),
#else
	TOOL("whistle", 	NULL,		1, 0, 105,  2,	10, METAL),
	TOOL("magic whistle",	"whistle",	0, 0,  30,  2,	10, METAL),
	TOOL("flute",		NULL,		1, 0,	6,  3,	12, WOOD),
	TOOL("magic flute",	"flute",	0, 1,	2,  3,	12, WOOD),
	TOOL("horn",		NULL,	 	1, 0,	5,  4,	15, METAL),
	TOOL("frost horn",	"horn", 	0, 1,	2,  4,	15, METAL),
	TOOL("fire horn",	"horn", 	0, 1,	2,  4,	15, METAL),
	TOOL("harp",		NULL,	 	1, 0,	4, 10,	50, METAL),
	TOOL("magic harp",	"harp", 	0, 1,	2, 10,	50, METAL),
	TOOL("bugle",		NULL,		1, 0,	6,  3,	15, METAL),
	TOOL("drum",		NULL,	 	1, 0,	4,  4,	25, 0),
	TOOL("drum of earthquake", "drum",	0, 1,	2,  4,	25, 0),
#endif
#undef TOOL
	{ "pick-axe", NULL, NULL, 1,0,1,METAL, 0, TOOL_SYM, 20,
						0, 10, 50, 6, 3, 0 },
	{ "blinding venom", "splash of venom", NULL, 0,1,0,0, 0, VENOM_SYM, 500,
						0, 0, 0, 0, 0, 0 },
	{ "acid venom", "splash of venom", NULL, 0,1,0,0, 0, VENOM_SYM, 500,
				0, 0, 0, 6, 6, 0 }, /* +d6 small or large */

	{ "heavy iron ball", NULL, NULL, 1,0,0,METAL, 0,
		BALL_SYM, 1000, 0, 20, 10, 0, 0, 0 },
	{ "iron chain", NULL, NULL, 1,0,0,METAL, 0,
		CHAIN_SYM, 1000, 0, 20, 0, 0, 0, 0 },

	/* Note: boulders and rocks normally do not appear at random; the
	 * probabilities only come into effect when you try to polymorph them.
	 */
	{ "boulder", NULL, NULL, 1,0,0,MINERAL, 0,
		ROCK_SYM, 100, 0, 200 /* > MAX_CARR_CAP */, 0, 20, 20, 0 },
	{ "statue", NULL, NULL, 1,0,0,MINERAL, 0,
		ROCK_SYM, 900, 0, 250, 0, 20, 20, 0 },

#define ARMOR(name,desc,kn,blk,power,prob,delay,weight,cost,ac,can,metal)  {\
	name, desc, NULL, kn,0,blk,metal, power, ARMOR_SYM, prob,\
	delay, weight, cost, ac, can, 0 }
#ifdef TOLKIEN
ARMOR("elven leather helm", "leather hat", 0, 0, 0,  6, 1, 2,  10, 9, 0, 0),
ARMOR("orcish helm", "black cap",	   0, 0, 0,  6, 1, 3,  10, 9, 0, METAL),
ARMOR("dwarvish iron helm", "hard hat",    0, 0, 0,  6, 1, 3,  10, 8, 0, METAL),
#else
ARMOR("orcish helm", "black cap",	   0, 0, 0, 18, 1, 3,  10, 9, 0, METAL),
#endif
ARMOR("fedora", NULL,			   1, 0, 0,  0, 1, 1,  10, 9, 0, 0),
ARMOR("helmet", "rusty pot",		   0, 0, 0, 12, 1, 2,  10, 9, 0, METAL),
ARMOR("helm of brilliance", "plumed hat",	   0, 0, 0,  6, 1, 2,  15, 9, 0, METAL),
ARMOR("helm of opposite alignment", "crested helmet",
					   0, 0, 0,  6, 1, 2,  15, 9, 0, METAL),
ARMOR("helm of telepathy", "visored helmet",
				0, 0, TELEPAT, 2, 1, 2, 15, 9, 0, METAL),

/* non-metal and (METAL | 1) armors do not rust */
ARMOR("dragon scale mail", NULL,  1, 1, 0,  0, 5, 5,1000, 1, 0, 0),
ARMOR("plate mail", NULL,	  1, 1, 0, 44, 5, 9, 400, 3, 2, METAL),
ARMOR("crystal plate mail", NULL, 1, 1, 0, 10, 5, 9, 820, 3, 2, 0),
#ifdef SHIRT
ARMOR("bronze plate mail", NULL,  1, 1, 0, 25, 5, 9, 600, 4, 0, (METAL | 1)),
#else
ARMOR("bronze plate mail", NULL,  1, 1, 0, 35, 5, 9, 600, 4, 0, (METAL | 1)),
#endif
ARMOR("splint mail", NULL,	1, 1, 0, 66, 5, 8,  80, 4, 1, METAL),
ARMOR("banded mail", NULL,	1, 1, 0, 76, 5, 8,  90, 4, 0, METAL),
#ifdef TOLKIEN
ARMOR("dwarvish mithril-coat", NULL, 1, 0, 0, 10, 1, 2, 160, 4, 3, (METAL | 1)),
ARMOR("elven mithril-coat", NULL, 1, 0, 0, 15, 1, 2, 160, 5, 3, (METAL | 1)),
ARMOR("chain mail", NULL,	1, 0, 0, 76, 5, 6,  75, 5, 1, METAL),
ARMOR("orcish chain mail", "black chain mail",
					0, 0, 0, 20, 5, 6,  75, 5, 1, METAL),
#else
ARMOR("dwarvish mithril-coat", NULL, 1, 0, 0, 25, 1, 2, 160, 4, 3, (METAL | 1)),
ARMOR("chain mail", NULL,	1, 0, 0, 96, 5, 6,  75, 5, 1, METAL),
#endif
ARMOR("scale mail", NULL,	1, 0, 0, 76, 5, 5,  45, 6, 0, METAL),
ARMOR("studded leather armor", NULL,
					1, 0, 0, 76, 3, 3,  15, 7, 1, 0),
#ifdef TOLKIEN
ARMOR("ring mail", NULL,	1, 0, 0, 76, 5, 4,  30, 7, 0, METAL),
ARMOR("orcish ring mail", "black ring mail",
					0, 0, 0, 20, 5, 5,  75, 8, 1, METAL),
#else
ARMOR("ring mail", NULL,	1, 0, 0, 96, 5, 4,  30, 7, 0, METAL),
#endif
ARMOR("leather armor", NULL,	1, 0, 0, 97, 3, 2,   5, 8, 0, 0),

/*  'cope' is not a spelling mistake... leave it be */
ARMOR("mummy wrapping", NULL,
				1, 0, 0,	     0, 0, 2,  5, 10, 2, 0),
#ifdef TOLKIEN
ARMOR("elven cloak", "ornamental cope",
				0, 0, STEALTH,	    12, 0, 2, 35,  9, 3, 0),
ARMOR("orcish cloak", "black mantelet",
				0, 0, 0,	    12, 0, 2, 35, 10, 3, 0),
ARMOR("dwarvish cloak", "colorful hooded cloak",
				0, 0, 0,	    12, 0, 2, 35, 10, 3, 0),
#else
ARMOR("elven cloak", "ornamental cope",
				0, 0, STEALTH,	    36, 0, 2, 35,  9, 3, 0),
#endif
ARMOR("cloak of protection", "tattered cape",
				0, 0, PROTECTION,   12, 0, 2, 15,  7, 3, 0),
ARMOR("cloak of invisibility", "opera hood",
				0, 0, INVIS,	    12, 0, 2, 35,  9, 3, 0),
ARMOR("cloak of magic resistance", "faded pall",
				0, 0, ANTIMAGIC,     2, 0, 2, 25,  9, 3, 0),
ARMOR("cloak of displacement", "piece of cloth",
				0, 0, DISPLACED,    12, 0, 2, 15,  9, 3, 0),

#ifdef TOLKIEN
ARMOR("small shield", NULL,
				1, 0, 0,	   6, 0, 2, 10,  9, 0, METAL),
ARMOR("elven shield", "blue and green shield",
				0, 0, 0,	   2, 0, 2, 15,  8, 0, METAL),
ARMOR("Uruk-hai shield", "white-handed shield",
				0, 0, 0,	   2, 0, 4, 10,  9, 0, METAL),
ARMOR("orcish shield", "red-eyed shield",
				0, 0, 0,	   2, 0, 3, 10,  9, 0, METAL),
ARMOR("large shield", NULL,
				1, 1, 0,	   7, 0, 4, 15,  8, 0, METAL),
ARMOR("dwarvish roundshield", "large round shield",
				0, 0, 0,	   4, 0, 4, 15,  8, 0, METAL),
#else
ARMOR("small shield", NULL,
				1, 0, 0,	  12, 0, 2, 10,  9, 0, METAL),
ARMOR("large shield", NULL,
				1, 1, 0,	  11, 0, 4, 15,  8, 0, METAL),
#endif
ARMOR("shield of reflection", "polished silver shield",
				0, 0, REFLECTING,  3, 0, 3, 50, 8, 0, (METAL | 1)),

#ifdef SHIRT
ARMOR("Hawaiian shirt", NULL,	1, 0, 0, 10, 0, 2, 5, 10, 0, 0),
#endif

ARMOR("leather gloves", "old gloves",
				0, 0, 0,	   16, 1, 2, 10, 9, 0, 0),
ARMOR("gauntlets of fumbling", "padded gloves",
				0, 0, FUMBLING,    8, 1, 2, 10, 9, 0, 0),
ARMOR("gauntlets of power", "riding gloves",
				0, 0, 0,	   8, 1, 2, 10, 9, 0, METAL),
ARMOR("gauntlets of dexterity", "fencing gloves",
				0, 0, 0,	   8, 1, 2, 10, 9, 0, 0),

ARMOR("low boots", "walking shoes",
				0, 0, 0,	   25, 2, 3, 20, 9, 0, 0),
ARMOR("iron shoes", "hard shoes",
				0, 0, 0,	    7, 2, 5, 20, 8, 0, METAL),
ARMOR("high boots", "jackboots",
				0, 0, 0,	   15, 2, 4, 50, 8, 0, 0),
ARMOR("speed boots", "combat boots",
				0, 0, FAST,	   12, 2, 4, 30, 9, 0, 0),
ARMOR("water walking boots", "jungle boots",
				0, 0, WWALKING,    12, 2, 4, 30, 9, 0, 0),
ARMOR("jumping boots", "hiking boots",
				0, 0, JUMPING,	   12, 2, 4, 30, 9, 0, 0),
ARMOR("elven boots", "mud boots",
				0, 0, STEALTH,	   12, 2, 3,  8, 9, 0, 0),
ARMOR("fumble boots", "riding boots",
				0, 0, FUMBLING,    12, 2, 4, 15, 9, 0, 0),
ARMOR("levitation boots", "snow boots",
				0, 0, LEVITATION,  12, 2, 4, 15, 9, 0, 0),
#undef ARMOR

#define POTION(name,color,power,prob)	{ name, color, NULL, 0,1,0,0, power,\
		POTION_SYM, prob, 0, 2, 100, 0, 0, 0 }

#ifdef SPELLS
	POTION("fruit juice",		"smoky",	0,	45),
	POTION("booze", 		"bubbly",	0,	45),
	POTION("gain energy",		"ebony", 	0,	45),
#else
	POTION("fruit juice",		"smoky",	0,	70),
	POTION("booze", 		"bubbly",	0,	65),
#endif
	POTION("gain ability",		"swirly",	0,	45),
	POTION("restore ability",	"pink",		0,	45),
	POTION("sickness",		"ruby",		SICK,	45),
	POTION("confusion",		"orange",	CONFUSION, 45),
	POTION("blindness",		"yellow",	BLINDED, 45),
	POTION("paralysis",		"emerald", 	0,	45),
	POTION("speed", 		"dark green", 	FAST,	45),
	POTION("levitation",		"cyan",		LEVITATION, 45),
	POTION("hallucination", 	"brilliant blue", HALLUC, 45),
	POTION("invisibility",		"sky blue",	INVIS,	45),
	POTION("see invisible", 	"magenta",	SEE_INVIS, 45),
	POTION("healing",		"purple", 	0,	65),
	POTION("extra healing", 	"purple-red",	0,	50),
	POTION("gain level",		"puce",	0,	20),
	POTION("enlightenment",		"brown",	0,	20),
	POTION("monster detection",	"white",	0,	45),
	POTION("object detection",	"glowing",	0,	45),
	POTION("water", 		"clear",	0,	125),
#undef POTION

#define SCROLL(name,text,prob,cost) { name, text, NULL, 0,1,0,0, 0,\
		SCROLL_SYM, prob, 0, 3, cost, 0, 0, 0 }
#ifdef MAIL
	SCROLL("mail",			"KIRJE",	     0,   0),
#endif
	SCROLL("enchant armor", 	"ZELGO MER",	    63,  80),
	SCROLL("destroy armor", 	"JUYED AWK YACC",   45, 100),
	SCROLL("confuse monster",	"NR 9", 	    53, 100),
	SCROLL("scare monster", 	"XIXAXA XOXAXA XUXAXA", 35, 100),
	SCROLL("blank paper",		"READ ME",	    28,  80),
	SCROLL("remove curse",		"PRATYAVAYAH",	    65,  80),
	SCROLL("enchant weapon",	"DAIYEN FOOELS",    85,  60),
	SCROLL("create monster",	"LEP GEX VEN ZEA",  45, 100),
	SCROLL("taming",		"PRIRUTSENIE",	    15, 200),
	SCROLL("genocide",		"ELBIB YLOH",	    15, 200),
	SCROLL("light", 		"VERR YED HORRE",   95,  50),
	SCROLL("teleportation", 	"VENZAR BORGAVVE",  55, 100),
	SCROLL("gold detection",	"THARR",	    33, 100),
	SCROLL("food detection",	"YUM YUM",	    25, 100),
	SCROLL("identify",		"KERNOD WEL",	    185, 20),
	SCROLL("magic mapping", 	"ELAM EBOW",	    45, 100),
	SCROLL("amnesia",		"DUAM XNAHT",	    35, 100),
	SCROLL("fire",			"ANDOVA BEGARIN",   48, 100),
	SCROLL("punishment",		"VE FORBRYDERNE",   15, 200),
	SCROLL("charging",		"HACKEM MUCHE",     15, 200),
	SCROLL(NULL,			"VELOX NEB",	     0, 100),
	SCROLL(NULL,			"FOOBIE BLETCH",     0, 100),
	SCROLL(NULL,			"TEMOV",	     0, 100),
	SCROLL(NULL,			"GARVEN DEH",	     0, 100),
#undef SCROLL

#define WAND(name,typ,prob,cost,flags,metal)	{ name, typ, NULL,\
		0,0,0,metal, 0, WAND_SYM, prob, 0, 3, cost, flags, 0, 0 }

	WAND("light",		"glass",	95, 100, NODIR,     GLASS),
	WAND("secret door detection", "balsa",	50, 150, NODIR,     WOOD),
	WAND("create monster",	"maple",	45, 200, NODIR,     WOOD),
	WAND("wishing", 	"pine",		 5, 500, NODIR,     WOOD),
	WAND("striking",	"oak",	 	75, 150, IMMEDIATE, WOOD),
	WAND("nothing", 	"ebony",	25, 100, IMMEDIATE, WOOD),
	WAND("make invisible",	"marble",	45, 150, IMMEDIATE, MINERAL),
	WAND("slow monster",	"tin",		55, 150, IMMEDIATE, METAL),
	WAND("speed monster",	"brass",	55, 150, IMMEDIATE, METAL),
	WAND("undead turning",	"copper",	55, 150, IMMEDIATE, METAL),
	WAND("polymorph",	"silver",	45, 200, IMMEDIATE, METAL),
	WAND("cancellation",	"platinum",	45, 200, IMMEDIATE, METAL),
	WAND("teleportation",	"iridium", 	45, 200, IMMEDIATE, METAL),
#ifdef PROBING
	WAND("probing", 	"uranium",	30, 150, IMMEDIATE, METAL),
	WAND("opening", 	"zinc",		25, 150, IMMEDIATE, METAL),
	WAND("locking", 	"aluminum",	25, 150, IMMEDIATE, METAL),
#else
	WAND("opening", 	"zinc",		40, 150, IMMEDIATE, METAL),
	WAND("locking", 	"aluminum",	40, 150, IMMEDIATE, METAL),
#endif
	WAND("digging", 	"iron", 	55, 150, RAY,	    METAL),
	WAND("magic missile",	"steel",	50, 150, RAY,	    METAL),
	WAND("fire",		"hexagonal",	40, 175, RAY,	    METAL),
	WAND("sleep",		"runed",	50, 175, RAY,	    METAL),
	WAND("cold",		"short",	40, 175, RAY,	    METAL),
	WAND("death",		"long", 	 5, 500, RAY,	    METAL),
	WAND("lightning",	"curved",	40, 175, RAY,	    METAL),
	WAND(NULL,		"forked",	 0, 150, 0,	    METAL),
	WAND(NULL,		"spiked",	 0, 150, 0,	    METAL),
	WAND(NULL,		"jeweled",	 0, 150, 0,	    METAL),
#undef WAND

#ifdef SPELLS
/* books */
#define SPELL(name,desc,prob,delay,level,flags) { name, desc, NULL,\
	0,1,0,0, 0, SPBOOK_SYM, prob, delay, 5, level*100, flags, 0, level }

	SPELL("magic missile",	 "parchment",	45,  3, 2, RAY),
	SPELL("fireball",	 "vellum",	20,  6, 4, RAY),
	SPELL("sleep",		 "dog eared",	50,  1, 1, RAY),
	SPELL("cone of cold",	 "ragged",	10,  8, 5, RAY),
	SPELL("finger of death", "stained",	 5, 10, 7, RAY),
	SPELL("light",		 "cloth",	45,  1, 1, NODIR),
	SPELL("detect monsters", "leather", 	45,  1, 1, NODIR),
	SPELL("healing",	 "white",	40,  2, 1, NODIR),
	SPELL("knock",		 "pink",	40,  1, 1, IMMEDIATE),
	SPELL("force bolt",	 "red",		40,  2, 1, IMMEDIATE),
	SPELL("confuse monster", "orange",	37,  2, 2, IMMEDIATE),
	SPELL("cure blindness",  "yellow", 	27,  2, 2, IMMEDIATE),
	SPELL("slow monster",	 "light green",	37,  2, 2, IMMEDIATE),
	SPELL("wizard lock",	 "dark green",	35,  3, 2, IMMEDIATE),
	SPELL("create monster",  "turquoise",	37,  3, 2, NODIR),
	SPELL("detect food",	 "cyan",	37,  3, 2, NODIR),
	SPELL("cause fear",	 "light blue",	25,  3, 3, NODIR),
	SPELL("clairvoyance",	 "dark blue",	15,  3, 3, NODIR),
	SPELL("cure sickness",	 "indigo",	32,  3, 3, NODIR),
	SPELL("charm monster",	 "magenta",	20,  3, 3, IMMEDIATE),
	SPELL("haste self",	 "purple",	33,  4, 3, NODIR),
	SPELL("detect unseen",	 "violet",	25,  4, 3, NODIR),
	SPELL("levitation",	 "tan",		25,  4, 4, NODIR),
	SPELL("extra healing",	 "plaid",	32,  5, 3, NODIR),
	SPELL("restore ability", "light brown",	25,  5, 4, NODIR),
	SPELL("invisibility",	 "dark brown",	32,  5, 4, NODIR),
	SPELL("detect treasure", "grey",	25,  5, 4, NODIR),
	SPELL("remove curse",	 "black",	25,  5, 5, NODIR),
	SPELL("dig",		 "mottled",	22,  6, 5, RAY),
	SPELL("magic mapping",	 "rusty",	18,  7, 5, NODIR),
	SPELL("identify",	 "bronze",	25,  8, 5, NODIR),
	SPELL("turn undead",	 "copper",	17,  8, 6, IMMEDIATE),
	SPELL("polymorph",	 "silver",	12,  8, 6, IMMEDIATE),
	SPELL("teleport away",	 "gold",	15,  6, 6, IMMEDIATE),
	SPELL("create familiar", "glittering", 	10,  7, 6, NODIR),
	SPELL("cancellation",	 "shining",	12,  8, 7, IMMEDIATE),
	SPELL("genocide",	 "glowing",	 5, 10, 7, NODIR),
	SPELL(NULL,		 "dull",	 0,  0, 0, 0),
	SPELL(NULL,		 "thin",	 0,  0, 0, 0),
	SPELL(NULL,		 "thick",	 0,  0, 0, 0),
#undef SPELL
#endif /* SPELLS /**/

#define RING(name,stone,power,spec,metal) { name, stone, NULL, 0,0,spec,metal,\
		power, RING_SYM, 0, 0, 1, 100, spec, 0, 0 }

	RING("adornment",	"wooden",	ADORNED,	1, WOOD),
	RING("gain strength",	"granite",	0,		1, MINERAL),
	RING("increase damage", "hematite",	0,		1, MINERAL),
	RING("protection",	"black onyx",	PROTECTION,	1, MINERAL),
	RING("regeneration",	"moonstone",	REGENERATION,	0, MINERAL),
	RING("searching",	"tiger eye",	SEARCHING,	0, MINERAL),
	RING("stealth", 	"jade",		STEALTH,	0, MINERAL),
	RING("levitation",	"agate",	LEVITATION,	0, MINERAL),
	RING("hunger",		"topaz",	HUNGER, 	0, MINERAL),
	RING("aggravate monster", "sapphire",	AGGRAVATE_MONSTER, 0, METAL),
	RING("conflict",	"ruby", 	CONFLICT,	0, METAL),
	RING("warning", 	"diamond", 	WARNING,	0, METAL),
	RING("poison resistance", "pearl",	POISON_RES,	0, METAL),
	RING("fire resistance", "iron",		FIRE_RES,	0, METAL),
	RING("cold resistance", "brass",	COLD_RES,	0, METAL),
	RING("shock resistance", "copper",	SHOCK_RES,	0, METAL),
	RING("teleportation",	"silver",	TELEPORT,	0, METAL),
	RING("teleport control", "gold",	TELEPORT_CONTROL, 0, METAL),
#ifdef POLYSELF
	RING("polymorph",	"ivory",	POLYMORPH,	0, 0),
	RING("polymorph control","blackened",	POLYMORPH_CONTROL, 0, METAL),
#endif
	RING("invisibility",	"wire",	INVIS,		0, METAL),
	RING("see invisible",	"engagement",	SEE_INVIS,	0, METAL),
	RING("protection from shape changers", "shining",
					PROT_FROM_SHAPE_CHANGERS, 0, METAL),
#undef RING

/* gems ************************************************************/
#define GEM(name,color,prob,wt,gval,glass) { name, color, NULL, 0,1,0,glass, 0,\
		GEM_SYM, prob, 0, wt, gval, 3, 3, WP_SLING }
	GEM("dilithium crystal", "white",	 3, 1, 4500, MINERAL),
	GEM("diamond", "white", 		 4, 1, 4000, MINERAL),
	GEM("ruby", "red",			 5, 1, 3500, MINERAL),
	GEM("sapphire", "blue", 		 6, 1, 3000, MINERAL),
	GEM("emerald", "green", 		 7, 1, 2500, MINERAL),
	GEM("turquoise", "green",		 8, 1, 2000, MINERAL),
	GEM("aquamarine", "green",		10, 1, 1500, MINERAL),
	GEM("amber", "yellowish brown", 	11, 1, 1000, MINERAL),
	GEM("topaz", "yellowish brown", 	13, 1,	900, MINERAL),
	GEM("opal", "white",			15, 1,	800, MINERAL),
	GEM("garnet", "red",			17, 1,	700, MINERAL),
	GEM("amethyst", "violet",		18, 1,	600, MINERAL),
	GEM("jasper", "red",			20, 1,	500, MINERAL),
	GEM("fluorite", "violet",		21, 1,	400, MINERAL),
	GEM("jade", "green",			22, 1,	300, MINERAL),
	GEM("worthless piece of white glass", "white",	158, 1, 0, GLASS),
	GEM("worthless piece of blue glass", "blue",	158, 1, 0, GLASS),
	GEM("worthless piece of red glass", "red",	158, 1, 0, GLASS),
	GEM("worthless piece of yellowish brown glass", "yellowish brown",
						158, 1, 0, GLASS),
	GEM("worthless piece of green glass", "green",	158, 1, 0, GLASS),
	GEM("luckstone", "grey",		 10, 1,  60, MINERAL),
	GEM("loadstone", "grey",		 10, 50,  1, MINERAL),
	{ "rock", NULL, NULL, 1,1,0,MINERAL, 0,
		GEM_SYM, 10, 0, 1, 0, 3, 3, WP_SLING },
#undef GEM

	{ NULL, NULL, NULL, 0,0,0,0, 0, ILLOBJ_SYM, 0, 0, 0, 0, 0, 0, 0 }
};
