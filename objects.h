/*	SCCS Id: @(#)objects.h	2.3	87/12/16
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

/* objects have letter " % ) ( 0 _ ` [ ! ? / = * + */
#include "config.h"
#include "objclass.h"
#define	NULL	(char *)0

struct objclass objects[] = {

	{ "strange object", NULL, NULL, 1, 0,
		ILLOBJ_SYM, 0, 0, 0, 0, 0, 0 },
	{ "amulet of Yendor", NULL, NULL, 1, 0,
		AMULET_SYM, 100, 0, 2, 0, 0, 0 },

#define	FOOD(name,prob,delay,weight,nutrition)	{ name, NULL, NULL, 1, 1,\
		FOOD_SYM, prob, delay, weight, 0, 0, nutrition }

/* dog eats foods 0-4 but prefers 1 above 0,2,3,4 */
/* food 4 can be read */
/* food 5 improves your vision */
/* food 6 makes you stronger (like Popeye) */
/* foods CORPSE up to CORPSE+52 are cadavers */

	FOOD("food ration", 	46, 5, 4, 800),
	FOOD("tripe ration",	16, 1, 2, 200),
	FOOD("pancake",		3, 1, 1, 200),
	FOOD("dead lizard",	3, 0, 1, 40),
	FOOD("fortune cookie",	7, 0, 1, 40),
	FOOD("carrot",		2, 0, 1, 50),
	FOOD("slice of pizza",	5, 0, 1, 250),
	FOOD("cream pie",	3, 0, 1, 100),
	FOOD("tin",		7, 0, 1, 0),
	FOOD("K-ration",	0, 1, 1, 400),
	FOOD("C-ration",	0, 1, 1, 300),
	FOOD("orange",		1, 0, 1, 80),
	FOOD("apple",		1, 0, 1, 50),
	FOOD("pear",		1, 0, 1, 50),
	FOOD("melon",		1, 0, 1, 100),
	FOOD("banana",		1, 0, 1, 80),
	FOOD("candy bar",	1, 0, 1, 100),
	FOOD("egg",		1, 0, 1, 80),
	FOOD("clove of garlic",	1, 0, 1, 40),
	FOOD("lump of royal jelly", 0, 0, 1, 200),

	FOOD("dead human",	0, 4, 40, 400),
	FOOD("dead giant ant",	0, 1, 3, 30),
	FOOD("dead giant bat",	0, 1, 3, 30),
	FOOD("dead centaur",	0, 5, 50, 500),
	FOOD("dead dragon",	0, 15, 150, 1500),
	FOOD("dead floating eye",	0, 1, 1, 10),
	FOOD("dead freezing sphere",	0, 1, 1, 10),
	FOOD("dead gnome",	0, 1, 10, 100),
	FOOD("dead hobgoblin",	0, 2, 20, 200),
	FOOD("dead stalker",	0, 4, 40, 400),
	FOOD("dead jackal",	0, 1, 10, 100),
	FOOD("dead kobold",	0, 1, 10, 100),
	FOOD("dead leprechaun",	0, 4, 40, 400),
	FOOD("dead mimic",	0, 4, 40, 400),
	FOOD("dead nymph",	0, 4, 40, 400),
	FOOD("dead orc",	0, 2, 20, 200),
	FOOD("dead purple worm",	0, 7, 70, 700),
	FOOD("dead quantum mechanic", 0, 2, 20, 200),
	FOOD("dead rust monster",	0, 5, 50, 500),
	FOOD("dead snake",	0, 1, 10, 100),
	FOOD("dead troll",	0, 4, 40, 400),
	FOOD("dead umber hulk",	0, 5, 50, 500),
	FOOD("dead vampire",	0, 4, 40, 400),
	FOOD("dead wraith",	0, 1, 1, 10),
	FOOD("dead xorn",	0, 7, 70, 700),
	FOOD("dead yeti",	0, 7, 70, 700),
	FOOD("dead zombie",	0, 1, 3, 30),
	FOOD("dead acid blob",	0, 1, 3, 30),
	FOOD("dead giant beetle",	0, 1, 1, 10),
	FOOD("dead cockatrice",	0, 1, 3, 30),
	FOOD("dead dog",	0, 2, 20, 200),
	FOOD("dead ettin",	0, 1, 3, 30),
	FOOD("dead fog cloud",	0, 1, 1, 10),
	FOOD("dead gelatinous cube",	0, 1, 10, 100),
	FOOD("dead homunculus",	0, 2, 20, 200),
	FOOD("dead imp",	0, 1, 1, 10),
	FOOD("dead jaguar",	0, 3, 30, 300),
	FOOD("dead killer bee",	0, 1, 1, 10),
	FOOD("dead leocrotta",	0, 5, 50, 500),
	FOOD("dead minotaur",	0, 7, 70, 700),
	FOOD("dead nurse",	0, 4, 40, 400),
	FOOD("dead owlbear",	0, 7, 70, 700),
	FOOD("dead piercer",	0, 2, 20, 200),
	FOOD("dead quivering blob",	0, 1, 10, 100),
	FOOD("dead giant rat",	0, 1, 3, 30),
	FOOD("dead giant scorpion",	0, 1, 10, 100),
	FOOD("dead tengu",	0, 3, 30, 300),
	FOOD("dead unicorn",	0, 3, 30, 300),
	FOOD("dead violet fungus",	0, 1, 10, 100),
	FOOD("dead long worm",	0, 5, 50, 500),
/* %% wt of long worm should be proportional to its length */
	FOOD("dead xan",	0, 3, 30, 300),
	FOOD("dead yellow light",	0, 1, 1, 10),
	FOOD("dead zruty",	0, 6, 60, 600),
#ifdef SAC
	FOOD("dead soldier",	0, 4, 40, 400),
#endif /* SAC */
	FOOD("dead giant",	0, 7, 70, 700),
	FOOD("dead demon",	0, 8, 80, 800),

/* weapons ... - ROCK come several at a time */
/* weapons ... - (DART-1) are shot using idem+(BOW-ARROW) */
/* weapons AXE, SWORD, KATANA, THSWORD are good for worm-cutting */
/* weapons (PICK-)AXE, DAGGER, CRYSKNIFE are good for tin-opening */
#define WEAPON(name,prob,wt,sdam,ldam)	{ name, NULL, NULL, 1, 0 /*%%*/,\
		WEAPON_SYM, prob, 0, wt, sdam, ldam, 0 }

/* Note: for weapons that don't do an even die of damage (i.e. 2-7 or 3-18)
 * the extra damage is added on in fight.c, not here! */

	WEAPON("arrow",		6, 0, 6, 6),
	WEAPON("sling bullet",	6, 0, 4, 6),
	WEAPON("crossbow bolt",	6, 0, 4, 6),
	WEAPON("dart",		6, 0, 3, 2),
	WEAPON("shuriken",	3, 0, 8, 6),
	WEAPON("rock",		4, 1, 3, 3),
	WEAPON("boomerang",	1, 3, 9, 9),
	WEAPON("mace",		6, 3, 6, 7), /* +1 small */
	WEAPON("axe",		5, 3, 6, 4),
	WEAPON("flail",		5, 3, 6, 5), /* +1 small, +1d4 large */
	WEAPON("long sword",	5, 3, 8, 12),
	WEAPON("two-handed sword",	4, 4, 12, 6), /* +2d6 large */
	WEAPON("dagger",	4, 3, 4, 3),
	WEAPON("worm tooth",	0, 4, 2, 2),
	WEAPON("crysknife",	0, 3, 10, 10),
	WEAPON("aklys",		1, 3, 6, 3), 
	WEAPON("bardiche",	1, 3, 4, 4), /* +1d4 small, +2d4 large */
	WEAPON("bec de corbin",	1, 3, 8, 6),
	WEAPON("bill-guisarme",	1, 3, 4, 10), /* +1d4 small */
	WEAPON("club",		1, 3, 6, 3),
	WEAPON("fauchard",	1, 3, 6, 8),
	WEAPON("glaive",	1, 3, 6, 10),
	WEAPON("guisarme",	1, 3, 4, 8), /* +1d4 small */
	WEAPON("halberd",	1, 3, 10, 6), /* +1d6 large */
	WEAPON("lucern hammer",	1, 3, 4, 6), /* +1d4 small */
	WEAPON("javelin",	1, 3, 6, 6),
	WEAPON("katana",	1, 3, 12, 12),
	WEAPON("lance",		1, 3, 6, 8),
	WEAPON("morning star",	1, 3, 4, 6), /* +d4 small, +1 large */
	WEAPON("partisan",	1, 3, 6, 6), /* +1 large */
	WEAPON("ranseur",	1, 3, 4, 4), /* +d4 both */
	WEAPON("scimitar",	1, 3, 8, 8), 
	WEAPON("spetum",	1, 3, 6, 6), /* +1 small, +d6 large */
	WEAPON("broad sword",	1, 3, 4, 6), /* +d4 small, +1 large */
	WEAPON("short sword",	1, 3, 6, 8),
	WEAPON("trident",	1, 3, 6, 4), /* +1 small, +2d4 large */
	WEAPON("voulge",	1, 3, 4, 4), /* +d4 both */
	WEAPON("spear",		4, 3, 6, 8),
	WEAPON("bow",		4, 3, 4, 6),
	WEAPON("sling",		4, 3, 6, 6),
	WEAPON("crossbow",	5, 3, 4, 6),

#ifdef WALKIES
	{ "whistle", "whistle", NULL, 0, 0, TOOL_SYM, 40, 0, 2, 0, 0, 0 },
	{ "leash", NULL, NULL, 1, 0, TOOL_SYM, 20, 0, 6, 0, 0, 0 },
#else
	{ "whistle", "whistle", NULL, 0, 0, TOOL_SYM, 60, 0, 2, 0, 0, 0 },
	{ "leash", NULL, NULL, 1, 0, TOOL_SYM, 0, 0, 6, 0, 0, 0 },
#endif
	{ "magic whistle", "whistle", NULL, 0, 0, TOOL_SYM, 9, 0, 2, 0, 0, 0 },
#ifdef RPH
	{ "blindfold", "blindfold", NULL, 0, 0, TOOL_SYM, 5, 0, 2, 0, 0, 0 },
	{ "mirror", "mirror", NULL, 0, 0, TOOL_SYM, 5, 0, 3, 0, 0, 0},
#else
	{ "blindfold", "blindfold", NULL, 0, 0, TOOL_SYM, 10, 0, 2, 0, 0, 0 },
	{ "mirror", "mirror", NULL, 0, 0, TOOL_SYM, 0, 0, 3, 0, 0, 0},
#endif
	{ "expensive camera", NULL, NULL, 1, 1, TOOL_SYM, 1, 0, 3, 0, 0, 0 },
	{ "ice box", "large box", NULL, 0, 0, TOOL_SYM, 1, 0, 40, 0, 0, 0 },
	{ "pick-axe", NULL, NULL, 1, 1, TOOL_SYM, 1, 0, 5, 6, 3, 0 },
	{ "magic marker", NULL, NULL, 1, 0, TOOL_SYM, 1, 0, 1, 0, 0, 0 },
	{ "stethoscope", NULL, NULL, 1, 0, TOOL_SYM, 1, 0, 3, 0, 0, 0 },
	{ "can opener", NULL, NULL, 1, 1, TOOL_SYM, 1, 0, 1, 0, 0, 0 },
	{ "lamp", "lamp", NULL, 0, 0, TOOL_SYM, 12, 0, 10, 0, 0, 0 },
	{ "magic lamp", "lamp", NULL, 0, 0, TOOL_SYM, 2, 0, 10, 0, 0, 0 },
	{ "badge", "badge", NULL, 0, 0, TOOL_SYM, 1, 0, 2, 0, 0, 0 },

	{ "heavy iron ball", NULL, NULL, 1, 0,
		BALL_SYM, 100, 0, 20, 0, 0, 0 },
	{ "iron chain", NULL, NULL, 1, 0,
		CHAIN_SYM, 100, 0, 20, 0, 0, 0 },
	/* Because giants can throw rocks */
#ifdef HARD
# ifdef KAA
	{ "enormous rock", NULL, NULL, 1, 0,
		ROCK_SYM, 100, 0, 200 /* > MAX_CARR_CAP */, 0, 20, 20 },
# else
	{ "enormous rock", NULL, NULL, 1, 0,
		ROCK_SYM, 100, 0, 250 /* > MAX_CARR_CAP */, 0, 0, 0 },
# endif
#else
# ifdef KAA
	{ "enormous rock", NULL, NULL, 1, 0,
		ROCK_SYM, 100, 0, 400 /* > MAX_CARR_CAP */, 0, 20, 20 },
# else
	{ "enormous rock", NULL, NULL, 1, 0,
		ROCK_SYM, 100, 0, 550 /* > MAX_CARR_CAP */, 0, 0, 0 },
# endif
#endif

#define ARMOR(name,prob,delay,weight,ac,can)	{ name, NULL, NULL, 1, 0,\
		ARMOR_SYM, prob, delay, weight, ac, can, 0 }
/* Originally, weight was always 8, which is ridiculous.  (Plate mail weighs
   the same as a pair of gloves?) */
	ARMOR("helmet",			 3, 1, 2, 9, 0),
	ARMOR("plate mail",		 5, 5, 9, 3, 2),
	ARMOR("splint mail",		 7, 5, 8, 4, 1),
	ARMOR("banded mail",		 9, 5, 8, 4, 0),
	ARMOR("chain mail",		10, 5, 6, 5, 1),
	ARMOR("scale mail",		10, 5, 5, 6, 0),
	ARMOR("ring mail",		12, 5, 3, 7, 0),
	/* the armors below do not rust */
	ARMOR("studded leather armor",	12, 3, 3, 7, 1),
	ARMOR("elfin chain mail",	 1, 1, 2, 5, 3),
#ifdef SHIRT
	ARMOR("bronze plate mail",       5, 5, 9, 4, 0),
#else
	ARMOR("bronze plate mail",	 6, 5, 9, 4, 0),
#endif
	ARMOR("crystal plate mail",	 1, 5, 9, 3, 2),
	ARMOR("leather armor",		15, 3, 2, 8, 0),
	ARMOR("elven cloak",		 5, 0, 2, 9, 3),
	ARMOR("shield",			 3, 0, 2, 9, 0),
#ifdef SHIRT
	ARMOR("Hawaiian shirt",          1, 0, 2,10, 0),
#else
	ARMOR("Hawaiian shirt",          0, 0, 2,10, 0),
#endif
	ARMOR("pair of gloves",		 1, 1, 2, 9, 0),

#define POTION(name,color)	{ name, color, NULL, 0, 1,\
		POTION_SYM, 0, 0, 2, 0, 0, 0 }

	POTION("restore strength",	"orange"),
	POTION("gain energy", "cyan"),
	POTION("booze",		"bubbly"),
	POTION("invisibility",	"glowing"),
	POTION("fruit juice",	"smoky"),
	POTION("healing",	"pink"),
	POTION("paralysis",	"puce"),
	POTION("monster detection",	"purple"),
	POTION("object detection",	"yellow"),
	POTION("sickness",	"white"),
	POTION("confusion",	"swirly"),
	POTION("gain strength",	"purple-red"),
	POTION("speed",		"ruby"),
	POTION("blindness",	"dark green"),
	POTION("gain level",	"emerald"),
	POTION("extra healing",	"sky blue"),
	POTION("levitation",	"brown"),
	POTION("hallucination",	"brilliant blue"),
	POTION("holy water",	"clear"),
	POTION(NULL,	"magenta"),
	POTION(NULL,	"ebony"),

#define SCROLL(name,text,prob) { name, text, NULL, 0, 1,\
		SCROLL_SYM, prob, 0, 3, 0, 0, 0 }
	SCROLL("mail",	"KIRJE", 0),
	SCROLL("enchant armor", "ZELGO MER", 6),
	SCROLL("destroy armor", "JUYED AWK YACC", 5),
	SCROLL("confuse monster", "NR 9", 5),
	SCROLL("scare monster", "XIXAXA XOXAXA XUXAXA", 4),
	SCROLL("blank paper", "READ ME", 3),
	SCROLL("remove curse", "PRATYAVAYAH", 6),
	SCROLL("enchant weapon", "DAIYEN FOOELS", 6),
	SCROLL("damage weapon", "HACKEM MUCHE", 5),
	SCROLL("create monster", "LEP GEX VEN ZEA", 5),
	SCROLL("taming", "PRIRUTSENIE", 1),
	SCROLL("genocide", "ELBIB YLOH",2),
	SCROLL("light", "VERR YED HORRE", 10),
	SCROLL("teleportation", "VENZAR BORGAVVE", 5),
	SCROLL("gold detection", "THARR", 4),
	SCROLL("food detection", "YUM YUM", 1),
	SCROLL("identify", "KERNOD WEL", 18),
	SCROLL("magic mapping", "ELAM EBOW", 5),
	SCROLL("amnesia", "DUAM XNAHT", 3),
	SCROLL("fire", "ANDOVA BEGARIN", 5),
	SCROLL("punishment", "VE FORBRYDERNE", 1),
	SCROLL(NULL, "VELOX NEB", 0),
	SCROLL(NULL, "FOOBIE BLETCH", 0),
	SCROLL(NULL, "TEMOV", 0),
	SCROLL(NULL, "GARVEN DEH", 0),

#define	WAND(name,metal,prob,flags)	{ name, metal, NULL, 0, 0,\
		WAND_SYM, prob, 0, 3, flags, 0, 0 }

	WAND("light",	"iridium",		10,	NODIR),
	WAND("secret door detection",	"tin",	5,	NODIR),
	WAND("create monster",	"platinum",	5,	NODIR),
	WAND("wishing",		"glass",	1,	NODIR),
#ifdef KAA
	WAND("striking",	"zinc",		7,	IMMEDIATE),
	WAND("nothing",		"uranium",	2,	IMMEDIATE),
#else
	WAND("striking",	"zinc",		9,	IMMEDIATE),
	WAND("nothing",		"uranium",	0,	IMMEDIATE),
#endif 
	WAND("slow monster",	"balsa",	5,	IMMEDIATE),
	WAND("speed monster",	"copper",	5,	IMMEDIATE),
	WAND("undead turning",	"silver",	5,	IMMEDIATE),
	WAND("polymorph",	"brass",	5,	IMMEDIATE),
	WAND("cancellation",	"maple",	5,	IMMEDIATE),
	WAND("teleportation",	"pine",		5,	IMMEDIATE),
#ifdef PROBING
	WAND("make invisible",	"marble",	7,	IMMEDIATE),
	WAND("probing",		"oak",		2,	IMMEDIATE),
#else
	WAND("make invisible",	"marble",	9,	IMMEDIATE),
	WAND("probing",		"oak",		0,	IMMEDIATE),
#endif
	WAND("digging",		"iron",		5,	RAY),
	WAND("magic missile",	"aluminum",	5,	RAY),
	WAND("fire",		"steel",	5,	RAY),
	WAND("sleep",		"curved",	5,	RAY),
	WAND("cold",		"short",	5,	RAY),
	WAND("death",		"long",		1,	RAY),
	WAND("lightning",	"ebony",	5,	RAY),
	WAND(NULL,		"runed",	0,	0),

#ifdef SPELLS
/* books */
#define SPELL(name,desc,prob,delay,flags,level)	{ name, desc, NULL, 0, 0, SPBOOK_SYM, prob, delay, 5, flags, 0, level }
	SPELL("magic missile", "parchment", 4, 3, RAY, 2),
	SPELL("fireball", "shining", 2, 6, RAY, 4),
	SPELL("sleep", "glowing", 6, 1, RAY, 1),
	SPELL("cone of cold", "mottled", 1, 8, RAY, 5),
	SPELL("finger of death", "ragged", 1, 10, RAY, 7),

	SPELL("healing", "yellow", 6, 2, NODIR, 1),
	SPELL("detect monsters", "light green", 5, 1, NODIR, 1),
	SPELL("force bolt", "dark blue", 4, 2, IMMEDIATE, 1),
	SPELL("light", "copper", 5, 1, NODIR, 1),
	SPELL("confuse monster", "white", 5, 2, IMMEDIATE, 2),
	SPELL("cure blindness", "red", 3, 2, IMMEDIATE, 2),
	SPELL("slow monster", "dark brown", 4, 2, IMMEDIATE, 2),
	SPELL("create monster", "light brown", 4, 3, NODIR, 2),
	SPELL("detect food", "pink", 5, 3, NODIR, 2),
	SPELL("haste self", "light blue", 3, 4, NODIR, 3),
	SPELL("cause fear", "black", 4, 3, NODIR, 3),
	SPELL("cure sickness", "rusty", 3, 3, NODIR, 3),
	SPELL("detect unseen", "dark green", 4, 4, NODIR, 3),
	SPELL("extra healing", "magenta", 3, 5, NODIR, 3),
	SPELL("charm monster", "silver", 3, 3, IMMEDIATE, 3),
	SPELL("levitation", "indigo", 3, 4, NODIR, 4),
	SPELL("restore strength", "plaid", 2, 5, NODIR, 4),
	SPELL("invisibility", "orange", 3, 5, NODIR, 4),
	SPELL("detect treasure", "bronze", 3, 5, NODIR, 4),
	SPELL("dig", "cloth", 2, 6, RAY, 5),
	SPELL("remove curse", "grey", 2, 5, NODIR, 5),
	SPELL("magic mapping", "purple", 2, 7, NODIR, 5),
	SPELL("identify", "violet", 1, 8, NODIR, 5),
	SPELL("turn undead", "turquoise", 1, 8, IMMEDIATE, 6),
	SPELL("polymorph", "cyan", 1, 8, IMMEDIATE, 6),
	SPELL("create familiar", "tan", 1, 7, NODIR, 6),
	SPELL("teleport away", "paper", 2, 6, IMMEDIATE, 6),
	SPELL("cancellation", "leather", 1, 8, IMMEDIATE, 7),
	SPELL("genocide", "gold", 1, 10, NODIR, 7),
/* randomization */
	SPELL(NULL, "dog eared", 0, 0, 0, 0),
	SPELL(NULL, "thick", 0, 0, 0, 0),
	SPELL(NULL, "thin", 0, 0, 0, 0),
	SPELL(NULL, "stained", 0, 0, 0, 0),
#endif /* SPELLS /**/

#define	RING(name,stone,spec)	{ name, stone, NULL, 0, 0,\
		RING_SYM, 0, 0, 1, spec, 0, 0 }

	RING("adornment",	"engagement",	0),
	RING("teleportation",	"wooden",	0),
	RING("regeneration",	"black onyx",	0),
	RING("searching",	"topaz",	0),
	RING("see invisible",	"pearl",	0),
	RING("stealth",		"sapphire",	0),
	RING("levitation",	"moonstone",	0),
	RING("poison resistance", "agate",	0),
	RING("aggravate monster", "tiger eye",	0),
	RING("hunger",		"shining",	0),
	RING("fire resistance",	"gold",		0),
	RING("cold resistance",	"copper",	0),
	RING("protection from shape changers", "diamond", 0),
	RING("conflict",	"jade",		0),
	RING("gain strength",	"ruby",		SPEC),
	RING("increase damage",	"silver",	SPEC),
	RING("protection",	"granite",	SPEC),
	RING("warning",		"wire",		0),
	RING("teleport control", "iron",	0),
	RING("polymorph",	"ivory",	0),
	RING("polymorph control","blackened",	0),
	RING("shock resistance", "hematite",	0),
	RING(NULL,		"brass",	0),

/* gems ************************************************************/
#define	GEM(name,color,prob,gval)	{ name, color, NULL, 0, 1,\
		GEM_SYM, prob, 0, 1, 0, 0, gval }
	GEM("dilithium crystal", "white", 1, 4500),
	GEM("diamond", "white", 1, 4000),
	GEM("ruby", "red", 1, 3500),
	GEM("sapphire", "blue", 1, 3000),
	GEM("emerald", "green", 1, 2500),
	GEM("turquoise", "green", 1, 2000),
	GEM("aquamarine", "green", 1, 1500),
	GEM("amber", "yellowish brown", 1, 1000),
	GEM("topaz", "yellowish brown", 1, 900),
	GEM("opal", "white", 1, 800),
	GEM("garnet", "red", 1, 700),
	GEM("amethyst", "violet", 1, 600),
	GEM("jasper", "red", 2, 500),
	GEM("fluorite", "violet", 2, 400),
	GEM("jade", "green", 2, 300),
	GEM("worthless piece of white glass", "white", 16, 100),
	GEM("worthless piece of blue glass", "blue", 16, 0),
	GEM("worthless piece of red glass", "red", 16, 0),
	GEM("worthless piece of yellowish brown glass", "yellowish brown", 17, 0),
	GEM("worthless piece of green glass", "green", 17, 0),
	{ NULL, NULL, NULL, 0, 0, ILLOBJ_SYM, 0, 0, 0, 0, 0, 0 }
};

char obj_symbols[] = {
	ILLOBJ_SYM, AMULET_SYM, FOOD_SYM, WEAPON_SYM, TOOL_SYM,
	BALL_SYM, CHAIN_SYM, ROCK_SYM, ARMOR_SYM,
	POTION_SYM, SCROLL_SYM, WAND_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	RING_SYM, GEM_SYM, 0 };
int bases[sizeof(obj_symbols)];
