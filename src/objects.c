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
#include "decl.h"
#undef BOW

/* objects have letter " % ) ( 0 _ ` [ ! ? / = * + . */

#ifdef C
#undef C
#endif
#ifdef TEXTCOLOR
#define C(n)	n
#else
#define C(n)
#endif

struct objclass objects[] = {

	{ "strange object", NULL, NULL, 1,0,0,0,0, 0,
		ILLOBJ_SYM, 0, 0, 0, 0, 0, 0, 0, C(0) },
/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob,weight) { \
	name, desc, NULL, 0,0,0,0,METAL, power, AMULET_SYM, prob, 0, weight, \
	150, 0, 0, 0, C(HI_METAL) }

	AMULET("amulet of esp", 	  "circular",	TELEPAT,    190, 2),
	AMULET("amulet of life saving",   "spherical",	LIFESAVED,   90, 2),
	AMULET("amulet of strangulation", "oval",	STRANGLED,  150, 2),
	AMULET("amulet of restful sleep", "triangular",	SLEEPING,   150, 2),
	AMULET("amulet versus poison",	  "pyramidal",	POISON_RES, 180, 2),
	AMULET("amulet of change",	  "square",	0,	    150, 2),
								/* POLYMORPH */
	AMULET("amulet of reflection",	  "hexagonal",	REFLECTING,  90, 2),
	{ "Amulet of Yendor", NULL, NULL, 1,0,1,0,METAL, 0,
		AMULET_SYM, 0, 0, 2, 3500, 0, 0, 0, C(HI_METAL) },
#undef AMULET

#define FOOD(name,prob,delay,wt,uk,tin,nutrition,color) { \
	name, NULL, NULL, 1,1,uk,0,tin, 0, FOOD_SYM, prob, delay, wt, \
	nutrition/20 + 5, 0, 0, nutrition, C(color) }

/* dog eats foods 0-4 but prefers tripe rations above all others */
/* fortune cookies can be read */
/* carrots improve your vision */
/* +0 tins contain monster meat */
/* +1 tins (of spinach) make you stronger (like Popeye) */
/* food CORPSE is a cadaver of some type */

	/* meat */
#ifdef TOLKIEN
	FOOD("tripe ration",	   145, 1, 2, 0, 0, 200, BROWN),
#else
	FOOD("tripe ration",	   150, 1, 2, 0, 0, 200, BROWN),
#endif
	FOOD("dead lizard",	    35, 0, 1, 0, 0,  40, GREEN),
	FOOD("corpse",		     0, 0, 0, 0, 0,   0, BROWN),
	FOOD("egg",		    75, 0, 1, 1, 0,  80, WHITE),
	/* fruits & veggies */
	FOOD("apple",		    10, 0, 1, 0, 0,  50, RED),
	FOOD("orange",		     7, 0, 1, 0, 0,  80, ORANGE_COLORED),
	FOOD("pear",		     7, 0, 1, 0, 0,  50, GREEN+BRIGHT),
	FOOD("melon",		     7, 0, 1, 0, 0, 100, GREEN+BRIGHT),
	FOOD("banana",		     7, 0, 1, 0, 0,  80, YELLOW),
	FOOD("carrot",		    15, 0, 1, 0, 0,  50, ORANGE_COLORED),
	FOOD("clove of garlic",      5, 0, 1, 0, 0,  40, WHITE),
#ifdef TUTTI_FRUTTI
	FOOD("slime mold",	    75, 0, 1, 0, 0, 250, BROWN),
#else
	FOOD("slice of pizza",	    75, 0, 1, 0, 0, 250, RED),
#endif
	/* human food */
	FOOD("lump of royal jelly",  0, 0, 1, 0, 0, 200, YELLOW),
	FOOD("cream pie",	    25, 0, 1, 0, 0, 100, WHITE),
	FOOD("candy bar",	     7, 0, 1, 0, 0, 100, BROWN),
	FOOD("fortune cookie",	    55, 0, 1, 0, 0,  40, BROWN),
#ifdef TOLKIEN
	FOOD("pancake", 	    25, 1, 1, 0, 0, 200, BROWN),
	FOOD("lembas wafer",	    20, 1, 1, 0, 0, 800, WHITE+BRIGHT),
	FOOD("cram ration",	    20, 1, 3, 0, 0, 600, HI_ORGANIC),
	FOOD("food ration",	   385, 5, 4, 0, 0, 800, HI_ORGANIC),
#else
	FOOD("pancake", 	    40, 1, 1, 0, 0, 200, BROWN),
	FOOD("food ration",	   405, 5, 4, 0, 0, 800, HI_ORGANIC),
#endif
#ifdef ARMY
	FOOD("K-ration",	     0, 1, 1, 0, 0, 400, HI_ORGANIC),
	FOOD("C-ration",	     0, 1, 1, 0, 0, 300, HI_ORGANIC),
#endif
	FOOD("tin",		    75, 0, 1, 1, METAL, 0, HI_METAL),
#undef FOOD

#define WEAPON(name,app,kn,mg,bi,prob,wt,cost,sdam,ldam,metal,color) { \
	name, app, NULL, kn,mg,1,bi,metal, 0, WEAPON_SYM, prob, 0, wt, \
	cost, sdam, ldam, 0, C(color) }
#define PROJECTILE(name,app,kn,bi,prob,wt,cost,sdam,ldam,metal,prop,color) { \
	name, app, NULL, kn,1,1,bi,metal, 0, WEAPON_SYM, prob, 0, wt, \
	cost, sdam, ldam, prop, C(color) }
#define BOW(name,app,kn,bi,prob,wt,cost,sdam,ldam,metal,prop,color) { \
	name, app, NULL, kn,0,1,bi,metal, 0, WEAPON_SYM, prob, 0, wt, \
	cost, sdam, ldam, -(prop), C(color) }

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
 * the extra damage is added on in weapon.c, not here! */

/* missiles */
#ifdef TOLKIEN
PROJECTILE("arrow",		NULL,		1, 0, 37, 0,  2, 6, 6,
		   METAL, WP_BOW, HI_METAL),
PROJECTILE("elven arrow",	"runed arrow",	0, 0, 10, 0,  2, 7, 6,
		   METAL, WP_BOW, HI_METAL),
PROJECTILE("orcish arrow",	"black arrow",	0, 0, 11, 0,  2, 5, 6,
		   METAL, WP_BOW, BLACK),
#else
PROJECTILE("arrow",		NULL,		1, 0, 58, 0,  2, 6, 6,
		   METAL, WP_BOW, HI_METAL),
#endif
PROJECTILE("silver arrow",	NULL,		1, 0,  8, 0,  2, 6, 6,
		   SILVER, WP_BOW, HI_SILVER),
PROJECTILE("crossbow bolt",	NULL,		1, 0, 60, 0,  2, 4, 6,
		   METAL, WP_CROSSBOW, HI_METAL),

WEAPON("dart",		NULL,		1, 1, 0, 60, 0,  2, 3, 2, METAL, HI_METAL),
WEAPON("shuriken",	"throwing star",0, 1, 0, 30, 0,  5, 8, 6, METAL, HI_METAL),
WEAPON("boomerang",	NULL,		1, 1, 0, 15, 3, 20, 9, 9, WOOD, HI_WOOD),

/* spears */
#ifdef TOLKIEN
WEAPON("spear", 	NULL,		1, 1, 0, 55, 3,  5, 6, 8, METAL, HI_METAL),
WEAPON("elven spear",	"runed spear",	0, 1, 0, 10, 3,  5, 7, 8, METAL, HI_METAL),
WEAPON("orcish spear",	"black spear",	0, 1, 0, 13, 3,  5, 5, 8, METAL, BLACK),
WEAPON("dwarvish spear","shiny spear",	0, 1, 0, 12, 3,  5, 8, 8, METAL, HI_METAL),
#else
WEAPON("spear", 	NULL,		1, 1, 0, 90, 3,  5, 6, 8, METAL, HI_METAL),
#endif
WEAPON("javelin",	"throwing spear",0,1, 0, 10, 3,  5, 6, 6, METAL, HI_METAL),
WEAPON("trident",	NULL,		1, 0, 0,  8, 4, 15, 6, 4, METAL, HI_METAL),
						/* +1 small, +2d4 large */
WEAPON("lance", 	NULL,		1, 0, 0,  8, 4, 10, 6, 8, METAL, HI_METAL),

/* blades */
#ifdef TOLKIEN
WEAPON("dagger",	NULL,		1, 1, 0, 25, 2,  4, 4, 3, METAL, HI_METAL),
WEAPON("elven dagger",	"large runed knife", 0, 1, 0,  8, 2, 4, 5, 3, METAL, HI_METAL),
WEAPON("orcish dagger", "large black knife", 0, 1, 0, 10, 2, 4, 3, 3, METAL, BLACK),
#else
WEAPON("dagger",	NULL,		1, 1, 0, 43, 2,  4, 4, 3, METAL, HI_METAL),
#endif
WEAPON("athame",	NULL,		1, 1, 0,  0, 2,  4, 4, 3, METAL, HI_METAL),
WEAPON("scalpel",	NULL,		1, 1, 0,  0, 1,  4, 4, 3, METAL, HI_METAL),
WEAPON("knife", 	NULL,		1, 1, 0, 25, 2,  4, 3, 3, METAL, HI_METAL),
WEAPON("axe",		NULL,		1, 0, 0, 50, 3,  8, 6, 4, METAL, HI_METAL),
#ifdef WORM
WEAPON("worm tooth",	NULL,		1, 0, 0,  0, 3,  2, 2, 2, 0, WHITE),
WEAPON("crysknife",	NULL,		1, 0, 0,  0, 3,100,10,10, METAL, HI_METAL),
#endif

/* swords */
#ifdef TOLKIEN
WEAPON("short sword",		NULL,	1, 0, 0,  6, 3, 10, 6, 8, METAL, HI_METAL),
WEAPON("elven short sword",	"short runed sword",
					0, 0, 0,  2, 3, 10, 8, 8, METAL, HI_METAL),
WEAPON("orcish short sword",	"short black sword",
					0, 0, 0,  3, 3, 10, 5, 8, METAL, BLACK),
WEAPON("dwarvish short sword",	"short shiny sword",
					0, 0, 0,  2, 3, 10, 7, 8, METAL, HI_METAL),
#else
WEAPON("short sword",		NULL,	1, 0, 0, 13, 3, 10, 6, 8, METAL, HI_METAL),
#endif
WEAPON("scimitar", "curved sword",	0, 0, 0,  6, 4, 15, 8, 8, METAL, HI_METAL),
#ifdef TOLKIEN
WEAPON("broadsword", "wide sword",	0, 0, 0,  8, 4, 10, 4, 6, METAL, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("elven broadsword",	"wide runed sword",
					0, 0, 0,  4, 4, 10, 6, 6, METAL, HI_METAL),
						/* +d4 small, +1 large */
#else
WEAPON("broadsword", "wide sword",	0, 0, 0, 12, 4, 10, 4, 6, METAL, HI_METAL),
						/* +d4 small, +1 large */
#endif
WEAPON("long sword",		NULL,	1, 0, 0, 60, 4, 15, 8, 12, METAL, HI_METAL),
#ifdef TOLKIEN
WEAPON("two-handed sword",	NULL,	1, 0, 1, 25, 5, 50,12,	6, METAL, HI_METAL),
						/* +2d6 large */
WEAPON("dwarvish mattock",	"huge shiny sword",
					0, 0, 1, 15, 6, 50,12,	8, METAL, HI_METAL),
						/* +2d6 large */
#else
WEAPON("two-handed sword",	NULL,	1, 0, 1, 40, 5, 50,12,	6, METAL, HI_METAL),
						/* +2d6 large */
#endif
WEAPON("katana", "samurai sword",	0, 0, 0,  6, 4,100,10, 12, METAL, HI_METAL),

/* blunt */
WEAPON("mace",		NULL,		1, 0, 0, 40, 4,  8, 6, 6, METAL, HI_METAL),
						/* +1 small */
WEAPON("morning star",	NULL,		1, 0, 0, 12, 4, 10, 4, 6, METAL, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("war hammer",	NULL,		1, 0, 0, 15, 3,  5, 4, 4, METAL, HI_METAL),
						/* +1 small */
WEAPON("club",		NULL,		1, 0, 0, 10, 3,  4, 6, 3, WOOD, HI_WOOD),
#ifdef KOPS
WEAPON("rubber hose",	NULL,		1, 0, 0,  0, 2,  4, 6, 3, 0, BROWN),
#endif
WEAPON("quarterstaff",	"staff",	0, 0, 1, 10, 3,  8, 6, 6, WOOD, HI_WOOD),
/* two-piece */
WEAPON("aklys", 	"thonged club", 0, 0, 0,  8, 3,  4, 6, 3, METAL, HI_METAL),
WEAPON("flail", 	NULL,		1, 0, 0, 40, 3,  4, 6, 4, METAL, HI_METAL),
						/* +1 small, +1d4 large */
/* whip */
WEAPON("bullwhip",	NULL,		1, 0, 0,  5, 2,  4, 2, 1, 0, BROWN),

/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm",	0, 0, 1, 10, 3, 10, 6, 6, METAL, HI_METAL),
						/* +1 large */
WEAPON("ranseur", "hilted polearm",	0, 0, 1, 10, 3,  6, 4, 4, METAL, HI_METAL),
						/* +d4 both */
WEAPON("spetum", "forked polearm",	0, 0, 1, 10, 3,  5, 6, 6, METAL, HI_METAL),
						/* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm", 0, 0, 1, 15, 3, 6, 6,10, METAL, HI_METAL),
/* axe-type */
WEAPON("halberd", "angled poleaxe",	0, 0, 1, 16, 3, 10,10, 6, METAL, HI_METAL),
						/* +1d6 large */
WEAPON("bardiche", "long poleaxe",	0, 0, 1,  8, 3,  7, 4, 4, METAL, HI_METAL),
						/* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver",	0, 0, 1,  8, 3,  5, 4, 4, METAL, HI_METAL),
						/* +d4 both */
/* curved/hooked */
WEAPON("fauchard",	"pole sickle",	0, 0, 1, 11, 3,  5, 6, 8, METAL, HI_METAL),
WEAPON("guisarme",	"pruning hook", 0, 0, 1, 11, 3,  5, 4, 8, METAL, HI_METAL),
						/* +1d4 small */
WEAPON("bill-guisarme","hooked polearm",0, 0, 1,  8, 3,  7, 4,10, METAL, HI_METAL),
						/* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm", 0, 0, 1, 10, 3,  7, 4, 6, METAL, HI_METAL),
						/* +1d4 small */
WEAPON("bec de corbin","beaked polearm",0, 0, 1,  8, 3,  8, 8, 6, METAL, HI_METAL),

/* bows */
#ifdef TOLKIEN
BOW("bow",	  NULL, 	1, 0, 24, 3, 120, 30, 6, 0, WP_BOW, HI_WOOD),
BOW("elven bow",  "runed bow",	0, 0, 12, 3, 120, 35, 6, 0, WP_BOW, HI_WOOD),
BOW("orcish bow", "black bow",	0, 0, 12, 3, 120, 25, 6, 0, WP_BOW, BLACK),
#else
BOW("bow",	  NULL, 	1, 0, 48, 3, 120, 30, 6, 0, WP_BOW, HI_WOOD),
#endif
BOW("sling", 	  NULL,	    	1, 0, 40, 2,  20,  4, 6, 0, WP_SLING, HI_WOOD),
BOW("crossbow",	  NULL,		1, 0, 45, 3,  40, 35, 6, 0, WP_CROSSBOW, HI_WOOD),
#undef WEAPON
#undef PROJECTILE
#undef BOW

/* tools ... - PICK AXE comes last because it has special characteristics */
#define TOOL(name,desc,kn,chg,prob,weight,cost,material,color) {\
	name, desc, NULL, kn,0,chg,chg,material, 0, TOOL_SYM, prob, 0, \
	weight, cost, 0, 0, 0, C(color)}

#ifdef WALKIES
	TOOL("leash",		NULL,	1, 0,  70,  3,	20, 0, HI_LEATHER),
#endif
#ifdef MEDUSA
	TOOL("blindfold",	NULL,	1, 0,  55,  2,	20, 0, BLACK),
	TOOL("mirror",		NULL,	1, 0,  50,  3,	40, GLASS, BLUE),
#else
	TOOL("blindfold",	NULL,	1, 0, 105,  2,	20, 0, BLACK),
#endif
	TOOL("tinning kit",	NULL,	1, 0,  15, 10,	30, METAL, HI_METAL),
	TOOL("lock pick",	NULL,	1, 0,  55,  2,	20, METAL, HI_METAL),
	TOOL("credit card",	NULL,	1, 0,	5,  1,	10, 0, WHITE),
#ifdef WALKIES
	TOOL("key",		NULL,	1, 0, 100,  1,	10, METAL, HI_METAL),
#else
	TOOL("key",		NULL,	1, 0, 155,  1,	10, METAL, HI_METAL),
#endif
	TOOL("skeleton key",	"key",	0, 0,  60,  1,	10, METAL, HI_METAL),
	TOOL("expensive camera", NULL,	1, 0,	5,  3, 200, 0, BLACK),
	TOOL("magic marker",	NULL,	1, 1,  15,  1,	50, 0, RED),
	TOOL("stethoscope",	NULL,	1, 0,  15,  2,	75, 0, HI_METAL),
	TOOL("tin opener",	NULL,	1, 0,  25,  1,	30, METAL, HI_METAL),
#ifdef WALKIES
	TOOL("lamp",		NULL,	1, 1,  90, 10,	50, 0, YELLOW),
#else
	TOOL("lamp",		NULL,	1, 1, 105, 10,	50, 0, YELLOW),
#endif
	TOOL("magic lamp",	"lamp", 0, 1,  20, 10,	50, 0, YELLOW),
	TOOL("crystal ball",	NULL,	1, 1,  35, 15,	60, GLASS, HI_GLASS),
	TOOL("figurine",	NULL,	1, 0,  35,  5,	80, MINERAL, HI_MINERAL),
	TOOL("ice box", 	NULL,	1, 0,	5, 40,  30, 0, WHITE),
	TOOL("large box",	NULL,	1, 0,  40, 40,  20, WOOD, HI_WOOD),
	TOOL("chest",		NULL,	1, 0,  35, 40,  20, WOOD, HI_WOOD),
	TOOL("sack",		"bag",	0, 0,  40,  3,  20, 0, HI_CLOTH),
	TOOL("bag of holding",	"bag",	0, 0,  20,  3, 100, 0, HI_CLOTH),
	TOOL("bag of tricks",	"bag",	0, 1,  20,  3, 100, 0, HI_CLOTH),
#ifndef MUSIC
	TOOL("whistle", 	NULL,	1, 0, 120,  2,	10, METAL, HI_METAL),
	TOOL("magic whistle","whistle",	0, 0,  50,  2,	10, METAL, HI_METAL),
#else
	TOOL("whistle", 	NULL,	1, 0, 105,  2,	10, METAL, HI_METAL),
	TOOL("magic whistle","whistle",	0, 0,  30,  2,	10, METAL, HI_METAL),
	TOOL("flute",		NULL,	1, 0,	6,  3,	12, WOOD, HI_WOOD),
	TOOL("magic flute",   "flute",	0, 1,	2,  3,	12, WOOD, HI_WOOD),
	TOOL("horn",		NULL,	1, 0,	5,  4,	15, METAL, HI_METAL),
	TOOL("frost horn",	"horn", 0, 1,	2,  4,	15, METAL, HI_METAL),
	TOOL("fire horn",	"horn", 0, 1,	2,  4,	15, METAL, HI_METAL),
	TOOL("harp",		NULL,	1, 0,	4, 10,	50, METAL, HI_METAL),
	TOOL("magic harp",	"harp", 0, 1,	2, 10,	50, METAL, HI_METAL),
	TOOL("bugle",		NULL,	1, 0,	6,  3,	15, METAL, HI_METAL),
	TOOL("drum",		NULL,	1, 0,	4,  4,	25, 0, BROWN),
	TOOL("drum of earthquake", "drum", 0, 1, 2,  4,	25, 0, BROWN),
#endif
#undef TOOL
	{ "pick-axe", NULL, NULL, 1,0,1,1,METAL, 0, TOOL_SYM, 20,
						0, 10, 50, 6, 3, 0, C(HI_METAL)},
	{ "blinding venom", "splash of venom", NULL,
		0,1,0,0,0, 0, VENOM_SYM, 500, 0, 0, 0, 0, 0, 0, C(HI_ORGANIC)},
	{ "acid venom", "splash of venom", NULL,
	        0,1,0,0,0, 0, VENOM_SYM, 500, 0, 0, 0, 6, 6, 0, C(HI_ORGANIC)},
	    /* +d6 small or large */
	{ "heavy iron ball", NULL, NULL, 1,0,0,0,METAL, 0,
		BALL_SYM, 1000, 0, 20, 10, 0, 0, 0, C(HI_METAL)},
	{ "iron chain", NULL, NULL, 1,0,0,0,METAL, 0,
		CHAIN_SYM, 1000, 0, 20, 0, 0, 0, 0, C(HI_METAL)},

	/* Note: boulders and rocks normally do not appear at random; the
	 * probabilities only come into effect when you try to polymorph them.
	 */
	{ "boulder", NULL, NULL, 1,0,0,0,MINERAL, 0, ROCK_SYM, 100, 0,
		200 /* > MAX_CARR_CAP */, 0, 20, 20, 0, C(HI_MINERAL)},
	{ "statue", NULL, NULL, 1,0,0,0,MINERAL, 0, ROCK_SYM, 900, 0,
	        250, 0, 20, 20, 0, C(HI_MINERAL)},

#define ARMOR(name,desc,kn,blk,power,prob,delay,weight,cost,ac,can,metal,c) \
	{name, desc, NULL, kn,0,1,blk,metal, power, ARMOR_SYM, prob,\
	delay, weight, cost, ac, can, 0, C(c)}
#ifdef TOLKIEN
ARMOR("elven leather helm", "leather hat",
			0, 0, 0,  6, 1, 2,   8, 9, 0, 0, HI_LEATHER),
ARMOR("orcish helm", "black cap",
			0, 0, 0,  6, 1, 3,  10, 9, 0, METAL, BLACK),
ARMOR("dwarvish iron helm", "hard hat",
			0, 0, 0,  6, 1, 3,  20, 8, 0, METAL, HI_METAL),
#else
ARMOR("orcish helm", "black cap",
			0, 0, 0, 18, 1, 3,  10, 9, 0, METAL, BLACK),
#endif
ARMOR("fedora", NULL,	1, 0, 0,  0, 1, 1,   8, 9, 0, 0, BROWN),
ARMOR("helmet", "rusty pot",
			0, 0, 0, 12, 1, 2,  10, 9, 0, METAL, HI_METAL),
ARMOR("helm of brilliance", "plumed hat",
			0, 0, 0,  6, 1, 2,  50, 9, 0, METAL, GREEN),
ARMOR("helm of opposite alignment", "crested helmet",
			0, 0, 0,  6, 1, 2,  50, 9, 0, METAL, HI_METAL),
ARMOR("helm of telepathy", "visored helmet",
			0, 0, TELEPAT, 2, 1, 2, 50, 9, 0, METAL, HI_METAL),

/* non-metal and (METAL | 1) armors do not rust */
ARMOR("dragon scale mail", NULL,
			1, 1, 0,  0, 5, 5,1000, 1, 0, 0, HI_LEATHER),
ARMOR("plate mail", NULL,
			1, 1, 0, 44, 5, 9, 600, 3, 2, METAL, HI_METAL),
ARMOR("crystal plate mail", NULL,
			1, 1, 0, 10, 5, 9, 820, 3, 2, 0, WHITE+BRIGHT),
#ifdef SHIRT
ARMOR("bronze plate mail", NULL,
			1, 1, 0, 25, 5, 9, 400, 4, 0, COPPER, HI_COPPER),
#else
ARMOR("bronze plate mail", NULL,
			1, 1, 0, 35, 5, 9, 400, 4, 0, COPPER, HI_COPPER),
#endif
ARMOR("splint mail", NULL,	1, 1, 0, 66, 5, 8,  80, 4, 1, METAL, HI_METAL),
ARMOR("banded mail", NULL,	1, 1, 0, 76, 5, 8,  90, 4, 0, METAL, HI_METAL),
#ifdef TOLKIEN
ARMOR("dwarvish mithril-coat", NULL,
			1, 0, 0, 10, 1, 2, 240, 4, 3, MITHRIL, HI_METAL),
ARMOR("elven mithril-coat", NULL, 1, 0, 0, 15, 1, 2, 240, 5, 3, MITHRIL, HI_METAL),
ARMOR("chain mail", NULL,	1, 0, 0, 76, 5, 6,  75, 5, 1, METAL, HI_METAL),
ARMOR("orcish chain mail", "black chain mail",
				0, 0, 0, 20, 5, 6,  75, 5, 1, METAL, BLACK),
#else
ARMOR("dwarvish mithril-coat", NULL,
			1, 0, 0, 25, 1, 2, 240, 4, 3, MITHRIL, HI_METAL),
ARMOR("chain mail", NULL,	1, 0, 0, 96, 5, 6,  75, 5, 1, METAL, HI_METAL),
#endif
ARMOR("scale mail", NULL,	1, 0, 0, 76, 5, 5,  45, 6, 0, METAL, HI_METAL),
ARMOR("studded leather armor", NULL,
				1, 0, 0, 76, 3, 3,  15, 7, 1, 0, HI_LEATHER),
#ifdef TOLKIEN
ARMOR("ring mail", NULL,	1, 0, 0, 76, 5, 4, 100, 7, 0, METAL, HI_METAL),
ARMOR("orcish ring mail", "black ring mail",
				0, 0, 0, 20, 5, 5,  80, 8, 1, METAL, BLACK),
#else
ARMOR("ring mail", NULL,	1, 0, 0, 96, 5, 4, 100, 7, 0, METAL, HI_METAL),
#endif
ARMOR("leather armor", NULL,	1, 0, 0, 97, 3, 2,   5, 8, 0, 0, HI_LEATHER),

/*  'cope' is not a spelling mistake... leave it be */
ARMOR("mummy wrapping", NULL,
			1, 0, 0,	     0, 0, 2,  2, 10, 2, 0, HI_CLOTH),
#ifdef TOLKIEN
ARMOR("elven cloak", "ornamental cope",
			0, 0, STEALTH,	    12, 0, 2, 60,  9, 3, 0, HI_CLOTH),
ARMOR("orcish cloak", "black mantelet",
			0, 0, 0,	    12, 0, 2, 40, 10, 3, 0, BLACK),
ARMOR("dwarvish cloak", "colorful hooded cloak",
			0, 0, 0,	    12, 0, 2, 50, 10, 3, 0, HI_CLOTH),
#else
ARMOR("elven cloak", "ornamental cope",
			0, 0, STEALTH,	    36, 0, 2, 60,  9, 3, 0, HI_CLOTH),
#endif
ARMOR("cloak of protection", "tattered cape",
			0, 0, PROTECTION,   12, 0, 2, 50,  7, 3, 0, HI_CLOTH),
ARMOR("cloak of invisibility", "opera hood",
			0, 0, INVIS,	    12, 0, 2, 60,  9, 3, 0, HI_CLOTH),
ARMOR("cloak of magic resistance", "faded pall",
			0, 0, ANTIMAGIC,     2, 0, 2, 60,  9, 3, 0, HI_CLOTH),
ARMOR("cloak of displacement", "piece of cloth",
			0, 0, DISPLACED,    12, 0, 2, 50,  9, 3, 0, HI_CLOTH),

#ifdef TOLKIEN
ARMOR("small shield", NULL,
			1, 0, 0,	6, 0, 2,  3,  9, 0, METAL, HI_METAL),
ARMOR("elven shield", "blue and green shield",
			0, 0, 0,	2, 0, 2,  3,  8, 0, METAL, GREEN),
ARMOR("Uruk-hai shield", "white-handed shield",
			0, 0, 0,	2, 0, 4,  3,  9, 0, METAL, HI_METAL),
ARMOR("orcish shield", "red-eyed shield",
			0, 0, 0,	2, 0, 3,  3,  9, 0, METAL, RED),
ARMOR("large shield", NULL,
			1, 1, 0,	7, 0, 4,  7,  8, 0, METAL, HI_METAL),
ARMOR("dwarvish roundshield", "large round shield",
			0, 0, 0,	4, 0, 4,  7,  8, 0, METAL, HI_METAL),
#else
ARMOR("small shield", NULL,
			1, 0, 0,       12, 0, 2,  3,  9, 0, METAL, HI_METAL),
ARMOR("large shield", NULL,
			1, 1, 0,       11, 0, 4,  7,  8, 0, METAL, HI_METAL),
#endif
ARMOR("shield of reflection", "polished silver shield",
			0, 0, REFLECTING,  3, 0, 3, 50, 8, 0, SILVER, HI_SILVER),

#ifdef SHIRT
ARMOR("Hawaiian shirt", NULL,	1, 0, 0, 10, 0, 2, 5, 10, 0, 0, MAGENTA),
#endif

ARMOR("leather gloves", "old gloves",
			0, 0, 0,	   16, 1, 2,  8, 9, 0, 0, BROWN),
ARMOR("gauntlets of fumbling", "padded gloves",
			0, 0, FUMBLING,     8, 1, 2, 50, 9, 0, 0, BROWN),
ARMOR("gauntlets of power", "riding gloves",
			0, 0, 0,	    8, 1, 2, 50, 9, 0, METAL, BROWN),
ARMOR("gauntlets of dexterity", "fencing gloves",
			0, 0, 0,	    8, 1, 2, 50, 9, 0, 0, BROWN),

ARMOR("low boots", "walking shoes",
			0, 0, 0,	   25, 2, 3,  8, 9, 0, 0, BROWN),
ARMOR("iron shoes", "hard shoes",
			0, 0, 0,	    7, 2, 5, 16, 8, 0, METAL, HI_METAL),
ARMOR("high boots", "jackboots",
			0, 0, 0,	   15, 2, 4, 12, 8, 0, 0, BROWN),
ARMOR("speed boots", "combat boots",
			0, 0, FAST,	   12, 2, 4, 50, 9, 0, 0, BROWN),
ARMOR("water walking boots", "jungle boots",
			0, 0, WWALKING,    12, 2, 4, 50, 9, 0, 0, BROWN),
ARMOR("jumping boots", "hiking boots",
			0, 0, JUMPING,	   12, 2, 4, 50, 9, 0, 0, BROWN),
ARMOR("elven boots", "mud boots",
			0, 0, STEALTH,	   12, 2, 3,  8, 9, 0, 0, BROWN),
ARMOR("fumble boots", "riding boots",
			0, 0, FUMBLING,    12, 2, 4, 30, 9, 0, 0, BROWN),
ARMOR("levitation boots", "snow boots",
			0, 0, LEVITATION,  12, 2, 4, 30, 9, 0, 0, BROWN),
#undef ARMOR

#define POTION(name,desc,power,prob,cost,color) \
		{ name, desc, NULL, 0,1,0,0,0, power,\
		POTION_SYM, prob, 0, 2, cost, 0, 0, 0, C(color)}

#ifdef SPELLS
POTION("fruit juice",		"smoky",	0,	45, 50,	WHITE),
POTION("booze", 		"bubbly",	0,	45, 50,	WHITE),
POTION("gain energy",		"ebony", 	0,	45,150,	BLACK),
#else
POTION("fruit juice",		"smoky",	0,	70, 50,	WHITE),
POTION("booze", 		"bubbly",	0,	65, 50,	WHITE),
#endif
POTION("gain ability",		"swirly",	0,	45,300,	WHITE),
POTION("restore ability",	"pink",		0,	45,100,	MAGENTA+BRIGHT),
POTION("sickness",		"ruby",		SICK,	45, 50,	RED),
POTION("confusion",		"orange",	CONFUSION, 45, 100, ORANGE_COLORED),
POTION("blindness",		"yellow",	BLINDED, 45,150, YELLOW),
POTION("paralysis",		"emerald", 	0,	45,300,	GREEN+BRIGHT),
POTION("speed", 		"dark green", 	FAST,	45,200,	GREEN),
POTION("levitation",		"cyan",		LEVITATION, 45,200, CYAN),
POTION("hallucination", 	"brilliant blue", HALLUC, 45,100, BLUE+BRIGHT),
POTION("invisibility",		"sky blue",	INVIS,	45,150,	CYAN),
POTION("see invisible", 	"magenta",	SEE_INVIS, 45,50, MAGENTA),
POTION("healing",		"purple", 	0,	65,100,	MAGENTA),
POTION("extra healing", 	"purple-red",	0,	50,100,	MAGENTA),
POTION("gain level",		"puce",		0,	20,300,	MAGENTA+BRIGHT),
POTION("enlightenment",		"brown",	0,	20,200,	BROWN),
POTION("monster detection",	"white",	0,	45,150,	WHITE),
POTION("object detection",	"glowing",	0,	45,150,	WHITE+BRIGHT),
POTION("water", 		"clear",	0,	125,100,CYAN),
#undef POTION

#define SCROLL(name,text,prob,cost) { name, text, NULL, 0,1,0,0,0, 0,\
		SCROLL_SYM, prob, 0, 3, cost, 0, 0, 0, C(HI_PAPER)}
#ifdef MAIL
	SCROLL("mail",			"KIRJE",	     0,   0),
#endif
	SCROLL("enchant armor", 	"ZELGO MER",	    63,  80),
	SCROLL("destroy armor", 	"JUYED AWK YACC",   45, 100),
	SCROLL("confuse monster",	"NR 9", 	    53, 100),
	SCROLL("scare monster", 	"XIXAXA XOXAXA XUXAXA", 35, 100),
	SCROLL("blank paper",		"READ ME",	    28,  60),
	SCROLL("remove curse",		"PRATYAVAYAH",	    65,  80),
	SCROLL("enchant weapon",	"DAIYEN FOOELS",    85,  60),
	SCROLL("create monster",	"LEP GEX VEN ZEA",  45, 200),
	SCROLL("taming",		"PRIRUTSENIE",	    15, 200),
	SCROLL("genocide",		"ELBIB YLOH",	    15, 300),
	SCROLL("light", 		"VERR YED HORRE",   95,  50),
	SCROLL("teleportation", 	"VENZAR BORGAVVE",  55, 100),
	SCROLL("gold detection",	"THARR",	    33, 100),
	SCROLL("food detection",	"YUM YUM",	    25, 100),
	SCROLL("identify",		"KERNOD WEL",	    185, 20),
	SCROLL("magic mapping", 	"ELAM EBOW",	    45, 100),
	SCROLL("amnesia",		"DUAM XNAHT",	    35, 200),
	SCROLL("fire",			"ANDOVA BEGARIN",   48, 100),
	SCROLL("punishment",		"VE FORBRYDERNE",   15, 300),
	SCROLL("charging",		"HACKEM MUCHE",     15, 300),
	SCROLL(NULL,			"VELOX NEB",	     0, 100),
	SCROLL(NULL,			"FOOBIE BLETCH",     0, 100),
	SCROLL(NULL,			"TEMOV",	     0, 100),
	SCROLL(NULL,			"GARVEN DEH",	     0, 100),
#undef SCROLL

#define WAND(name,typ,prob,cost,flags,metal,c)	{ \
	name, typ, NULL, 0,0,1,0,metal, 0, WAND_SYM, \
	prob, 0, 3, cost, flags, 0, 0, C(c) }

WAND("light",		"glass",	95, 100, NODIR,    GLASS,HI_GLASS),
WAND("secret door detection", "balsa",	50, 150, NODIR,    WOOD,HI_WOOD),
WAND("create monster",	"maple",	45, 200, NODIR,    WOOD,HI_WOOD),
WAND("wishing", 	"pine",		 5, 500, NODIR,    WOOD,HI_WOOD),
WAND("striking",	"oak",	 	75, 150, IMMEDIATE,WOOD,HI_WOOD),
WAND("nothing", 	"ebony",	25, 100, IMMEDIATE,WOOD,HI_WOOD),
WAND("make invisible",	"marble",	45, 150, IMMEDIATE,MINERAL,HI_MINERAL),
WAND("slow monster",	"tin",		55, 150, IMMEDIATE,METAL,HI_METAL),
WAND("speed monster",	"brass",	55, 150, IMMEDIATE,COPPER,HI_COPPER),
WAND("undead turning",	"copper",	55, 150, IMMEDIATE,COPPER,HI_COPPER),
WAND("polymorph",	"silver",	45, 200, IMMEDIATE,SILVER,HI_SILVER),
WAND("cancellation",	"platinum",	45, 200, IMMEDIATE,PLATINUM,HI_SILVER|BRIGHT),
WAND("teleportation",	"iridium", 	45, 200, IMMEDIATE,METAL,CYAN|BRIGHT),
#ifdef PROBING
WAND("opening", 	"zinc",		25, 150, IMMEDIATE,METAL,HI_METAL),
WAND("locking", 	"aluminum",	25, 150, IMMEDIATE,METAL,HI_METAL),
WAND("probing", 	"uranium",	30, 150, IMMEDIATE,METAL,HI_METAL),
#else
WAND("opening", 	"zinc",		40, 150, IMMEDIATE,METAL,HI_METAL),
WAND("locking", 	"aluminum",	40, 150, IMMEDIATE,METAL,HI_METAL),
#endif
WAND("digging", 	"iron", 	55, 150, RAY,	   METAL,HI_METAL),
WAND("magic missile",	"steel",	50, 150, RAY,	   METAL,HI_METAL),
WAND("fire",		"hexagonal",	40, 175, RAY,	   METAL,HI_METAL),
WAND("sleep",		"runed",	50, 175, RAY,	   METAL,HI_METAL),
WAND("cold",		"short",	40, 175, RAY,	   METAL,HI_METAL),
WAND("death",		"long", 	 5, 500, RAY,	   METAL,HI_METAL),
WAND("lightning",	"curved",	40, 175, RAY,	   METAL,HI_METAL),
WAND(NULL,		"forked",	 0, 150, 0,	   WOOD,HI_WOOD),
WAND(NULL,		"spiked",	 0, 150, 0,	   METAL,HI_METAL),
WAND(NULL,		"jeweled",	 0, 150, 0,	   METAL,HI_MINERAL),
#undef WAND

#ifdef SPELLS
/* books */
#define SPELL(name,desc,prob,delay,level,flags,color) \
	{ name, desc, NULL, 0,1,0,0,0, 0, SPBOOK_SYM, prob, delay, \
	5, level*100, flags, 0, level, C(color)}

SPELL("magic missile",	 "parchment",	45,  3, 2, RAY,		HI_PAPER),
SPELL("fireball",	 "vellum",	20,  6, 4, RAY,		HI_PAPER),
SPELL("sleep",		 "dog eared",	50,  1, 1, RAY,		HI_PAPER),
SPELL("cone of cold",	 "ragged",	10,  8, 5, RAY,		HI_PAPER),
SPELL("finger of death", "stained",	 5, 10, 7, RAY,		HI_PAPER),
SPELL("light",		 "cloth",	45,  1, 1, NODIR,	HI_CLOTH),
SPELL("detect monsters", "leather", 	45,  1, 1, NODIR,	HI_LEATHER),
SPELL("healing",	 "white",	40,  2, 1, NODIR,	WHITE),
SPELL("knock",		 "pink",	40,  1, 1, IMMEDIATE,	MAGENTA+BRIGHT),
SPELL("force bolt",	 "red",		40,  2, 1, IMMEDIATE,	RED),
SPELL("confuse monster", "orange",	37,  2, 2, IMMEDIATE,	ORANGE_COLORED),
SPELL("cure blindness",  "yellow", 	27,  2, 2, IMMEDIATE,	YELLOW),
SPELL("slow monster",	 "light green",	37,  2, 2, IMMEDIATE,	GREEN+BRIGHT),
SPELL("wizard lock",	 "dark green",	35,  3, 2, IMMEDIATE,	GREEN),
SPELL("create monster",  "turquoise",	37,  3, 2, NODIR,	CYAN+BRIGHT),
SPELL("detect food",	 "cyan",	37,  3, 2, NODIR,	CYAN),
SPELL("cause fear",	 "light blue",	25,  3, 3, NODIR,	BLUE+BRIGHT),
SPELL("clairvoyance",	 "dark blue",	15,  3, 3, NODIR,	BLUE),
SPELL("cure sickness",	 "indigo",	32,  3, 3, NODIR,	BLUE),
SPELL("charm monster",	 "magenta",	20,  3, 3, IMMEDIATE,	MAGENTA),
SPELL("haste self",	 "purple",	33,  4, 3, NODIR,	MAGENTA),
SPELL("detect unseen",	 "violet",	25,  4, 3, NODIR,	MAGENTA),
SPELL("levitation",	 "tan",		25,  4, 4, NODIR,	BROWN),
SPELL("extra healing",	 "plaid",	32,  5, 3, NODIR,	GREEN),
SPELL("restore ability", "light brown",	25,  5, 4, NODIR,	BROWN),
SPELL("invisibility",	 "dark brown",	32,  5, 4, NODIR,	BROWN),
SPELL("detect treasure", "gray",	25,  5, 4, NODIR,	GRAY),
SPELL("remove curse",	 "white",	25,  5, 5, NODIR,	WHITE),
SPELL("dig",		 "mottled",	22,  6, 5, RAY,		HI_PAPER),
SPELL("magic mapping",	 "rusty",	18,  7, 5, NODIR,	RED),
SPELL("identify",	 "bronze",	25,  8, 5, NODIR,	HI_COPPER),
SPELL("turn undead",	 "copper",	17,  8, 6, IMMEDIATE,	HI_COPPER),
SPELL("polymorph",	 "silver",	12,  8, 6, IMMEDIATE,	HI_SILVER),
SPELL("teleport away",	 "gold",	15,  6, 6, IMMEDIATE,	HI_GOLD),
SPELL("create familiar", "glittering", 	10,  7, 6, NODIR,	WHITE+BRIGHT),
SPELL("cancellation",	 "shining",	12,  8, 7, IMMEDIATE,	WHITE+BRIGHT),
SPELL("genocide",	 "glowing",	 5, 10, 7, NODIR,	WHITE+BRIGHT),
SPELL(NULL,		 "dull",	 0,  0, 0, 0,		HI_PAPER),
SPELL(NULL,		 "thin",	 0,  0, 0, 0,		HI_PAPER),
SPELL(NULL,		 "thick",	 0,  0, 0, 0,		HI_PAPER),
#undef SPELL
#endif /* SPELLS /**/

#define RING(name,stone,power,cost,spec,metal,color) \
		{ name, stone, NULL, 0,0,spec,spec,metal, \
		power, RING_SYM, 0, 0, 1, cost, 0, 0, 0, C(color)}

RING("adornment",	"wooden",	ADORNED,	100, 1, WOOD, HI_WOOD),
RING("gain strength",	"granite",	0,		150, 1, MINERAL, HI_MINERAL),
RING("increase damage", "coral",	0,		150, 1, MINERAL, RED+BRIGHT),
RING("protection",	"black onyx",	PROTECTION,	100, 1, MINERAL, BLACK),
RING("regeneration",	"moonstone",	REGENERATION,	200, 0, MINERAL, HI_MINERAL),
RING("searching",	"tiger eye",	SEARCHING,	200, 0, MINERAL, BROWN),
RING("stealth", 	"jade",		STEALTH,	100, 0, MINERAL, GREEN),
RING("levitation",	"agate",	LEVITATION,	200, 0, MINERAL, RED),
RING("hunger",		"topaz",	HUNGER, 	100, 0, MINERAL, CYAN),
RING("aggravate monster", "sapphire",	AGGRAVATE_MONSTER, 150, 0, METAL, BLUE),
RING("conflict",	"ruby", 	CONFLICT,	300, 0, METAL, RED),
RING("warning", 	"diamond", 	WARNING,	100, 0, METAL, WHITE+BRIGHT),
RING("poison resistance", "pearl",	POISON_RES,	150, 0, METAL, WHITE),
RING("fire resistance", "iron",		FIRE_RES,	200, 0, METAL, HI_METAL),
RING("cold resistance", "brass",	COLD_RES,	150, 0, COPPER, HI_COPPER),
RING("shock resistance", "copper",	SHOCK_RES,	150, 0, COPPER, HI_COPPER),
RING("teleportation",	"silver",	TELEPORT,	200, 0, SILVER, HI_SILVER),
RING("teleport control", "gold",	TELEPORT_CONTROL,
							300, 0, GOLD, HI_GOLD),
#ifdef POLYSELF
RING("polymorph",	"ivory",	POLYMORPH,	300, 0, 0, WHITE),
RING("polymorph control","emerald",	POLYMORPH_CONTROL,
							300, 0, METAL, GREEN+BRIGHT),
#endif
RING("invisibility",	"wire",		INVIS,		150, 0, METAL, HI_METAL),
RING("see invisible",	"engagement",	SEE_INVIS,	150, 0, METAL, HI_METAL),
RING("protection from shape changers", "shining", PROT_FROM_SHAPE_CHANGERS,
							100, 0, METAL, HI_METAL|BRIGHT),
#undef RING

/* gems ************************************************************/
#define GEM(name,desc,prob,wt,gval,glass, color) \
		{ name, desc, NULL, 0,1,0,0,glass, 0,\
		GEM_SYM, prob, 0, wt, gval, 3, 3, WP_SLING, C(color)}
GEM("dilithium crystal", "white",	 3, 1, 4500, MINERAL, WHITE),
GEM("diamond", "white", 		 4, 1, 4000, MINERAL, WHITE),
GEM("ruby", "red",			 5, 1, 3500, MINERAL, RED),
GEM("sapphire", "blue", 		 6, 1, 3000, MINERAL, BLUE),
GEM("emerald", "green", 		 7, 1, 2500, MINERAL, GREEN),
GEM("turquoise", "green",		 8, 1, 2000, MINERAL, GREEN),
GEM("aquamarine", "green",		10, 1, 1500, MINERAL, GREEN),
GEM("amber", "yellowish brown", 	11, 1, 1000, MINERAL, BROWN),
GEM("topaz", "yellowish brown", 	13, 1,	900, MINERAL, BROWN),
GEM("opal", "white",			15, 1,	800, MINERAL, WHITE),
GEM("garnet", "red",			17, 1,	700, MINERAL, RED),
GEM("amethyst", "violet",		19, 1,  600, MINERAL, MAGENTA),
GEM("jasper", "red",			21, 1,	500, MINERAL, RED),
GEM("fluorite", "violet",		22, 1,	400, MINERAL, MAGENTA),
GEM("jade", "green",			23, 1,	300, MINERAL, GREEN),
GEM("worthless piece of white glass", "white",	131, 1, 0, GLASS, WHITE),
GEM("worthless piece of blue glass", "blue",	131, 1, 0, GLASS, BLUE),
GEM("worthless piece of red glass", "red",	131, 1, 0, GLASS, RED),
GEM("worthless piece of yellowish brown glass", "yellowish brown",
						131, 1, 0, GLASS, BROWN),
GEM("worthless piece of green glass", "green",	131, 1, 0, GLASS, GREEN),
GEM("worthless piece of violet glass", "violet",131, 1, 0, GLASS, MAGENTA),
GEM("luckstone", "gray",		 10, 1,  60, MINERAL, GRAY),
GEM("loadstone", "gray",		 10, 50,  1, MINERAL, GRAY),
{ "rock", NULL, NULL, 1,1,0,0,MINERAL, 0,
		GEM_SYM, 10, 0, 1, 0, 3, 3, WP_SLING, C(HI_MINERAL)},
#undef GEM

	{ NULL, NULL, NULL, 0,0,0,0,0, 0, ILLOBJ_SYM, 0, 0, 0, 0, 0, 0, 0, C(0) }
};

#undef C
