/*	SCCS Id: @(#)monst.c	2.3	87/12/16
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include "hack.h"
#include "eshk.h"
#include "edog.h"
extern char plname[PL_NSIZ];

struct permonst mons[CMNUM+2] = {
	{ "bat",		'B',  1, 22, 8,  0, 1,  4, 0 },
	{ "gnome",		'G',  1,  6, 5,  0, 1,  6, 0 },
	{ "hobgoblin",		'H',  1,  9, 5,  0, 1,  8, 0 },
	{ "jackal",		'J',  0, 12, 7,  0, 1,  2, 0 },
#ifdef KOPS
	{ "Keystone Kop",       'K',  1,  6, 7, 10, 1,  4, 0 },
#else
	{ "kobold",		'K',  1,  6, 7,  0, 1,  4, 0 },
#endif
#ifndef ROCKMOLE
	{ "giant rat",		'r',  0, 12, 7,  0, 1,  3, 0 },
#endif
	{ "acid blob",		'a',  2,  3, 8,  0, 0,  0, 0 },
	{ "floating eye",	'E',  2,  1, 9, 10, 0,  0, 0 },
	{ "homunculus",		'h',  2,  6, 6, 10, 1,  3, 0 },
	{ "imp",		'i',  2,  6, 2, 20, 1,  4, 0 },
	{ "leprechaun",		'L',  5, 15, 8, 20, 1,  2, 0 },
	{ "orc",		'O',  2,  9, 6,  0, 1,  8, 0 },
	{ "yellow light",	'y',  3, 15, 0,  0, 0,  0, 0 },
	{ "zombie",		'Z',  2,  6, 8,  0, 1,  8, 0 },
	{ "giant ant",		'A',  3, 18, 3,  0, 1,  6, 0 },
#ifdef ROCKMOLE
	{ "rock mole",          'r',  3,  3, 0, 20, 1,  6, 0 },
#endif
	{ "fog cloud",		'f',  3,  1, 0,  0, 1,  6, 0 },
	{ "nymph",		'N',  6, 12, 9, 20, 1,  2, 0 },
	{ "piercer",		'p',  3,  1, 3,  0, 2,  6, 0 },
#ifdef KAA
	{ "quantum mechanic",	'Q',  6, 12, 3, 10, 1,  4, 0 },
#else
	{ "quasit",		'Q',  3, 15, 3, 20, 1,  4, 0 },
#endif
	{ "quivering blob",	'q',  3,  1, 8,  0, 1,  8, 0 },
#ifdef KAA
	{ "violet fungus",	'v',  3,  1, 7,  0, 1,  4, 0 },
#else
	{ "violet fungi",	'v',  3,  1, 7,  0, 1,  4, 0 },
#endif
	{ "giant beetle",	'b',  4,  6, 4,  0, 3,  4, 0 },
	{ "centaur",		'C',  4, 18, 4, 10, 1,  6, 0 },
	{ "cockatrice",		'c',  4,  6, 6, 30, 1,  3, 0 },
	{ "gelatinous cube",	'g',  4,  6, 8,  0, 2,  4, 0 },
	{ "jaguar",		'j',  4, 15, 6,  0, 1,  8, 0 },
	{ "killer bee",		'k',  4, 14, 4,  0, 2,  4, 0 },
	{ "snake",		'S',  4, 15, 3,  0, 1,  6, 0 },
	{ "freezing sphere",	'F',  2, 13, 4,  0, 0,  0, 0 },
	{ "owlbear",		'o',  5, 12, 5,  0, 2,  6, 0 },
	{ "rust monster",	'R', 10, 18, 3,  0, 0,  0, 0 },
#ifdef SPIDERS
	{ "giant spider",	's',  5, 15, 3,  0, 1,  4, 0 },
#else
	{ "scorpion",		's',  5, 15, 3,  0, 1,  4, 0 },
#endif
	{ "tengu",		't',  5, 13, 5, 30, 1,  7, 0 },
	{ "wraith",		'W',  5, 12, 5, 15, 1,  6, 0 },
#ifdef NOWORM
	{ "wumpus",		'w',  8,  3, 2, 10, 3,  6, 0 },
#else
	{ "long worm",		'w',  8,  3, 5, 10, 1,  4, 0 },
#endif
	{ "large dog",		'd',  6, 15, 4,  0, 2,  4, 0 },
	{ "leocrotta",		'l',  6, 18, 4, 10, 3,  6, 0 },
	{ "mimic",		'M',  7,  3, 7,  0, 3,  4, 0 },
	{ "troll",		'T',  7, 12, 4,  0, 2,  7, 0 },
	{ "unicorn",		'u',  8, 24, 5, 70, 1, 10, 0 },
	{ "yeti",		'Y',  5, 15, 6,  0, 1,  6, 0 },
	{ "stalker",		'I',  8, 12, 3,  0, 4,  4, 0 },
	{ "umber hulk",		'U',  9,  6, 2, 25, 2, 10, 0 },
	{ "vampire",		'V',  8, 12, 1, 25, 1,  6, 0 },
	{ "xorn",		'X',  8,  9,-2, 20, 4,  6, 0 },
	{ "xan",		'x',  7, 18,-2,  0, 2,  4, 0 },
	{ "zruty",		'z',  9,  8, 3,  0, 3,  6, 0 },
	{ "chameleon",		':',  6,  5, 6, 10, 4,  2, 0 },
	{ "giant",		'9',  9, 18, 5,  0, 2, 12, 0 },
	{ "dragon",		'D', 10,  9,-1, 20, 3,  8, 0 },
	{ "ettin",		'e', 10, 12, 3,  0, 2,  8, 0 },
	{ "lurker above",	'\'',10,  3, 3,  0, 0,  0, 0 },
	{ "nurse",		'n', 11,  6, 0,  0, 2,  6, 0 },
	{ "trapper",		',', 12,  3, 3,  0, 0,  0, 0 },
	{ "purple worm",	'P', 15,  9, 6, 20, 2,  8, 0 },
	{ "demon",		'&', 10, 12,-4, 30, 1,  4, 0 },
	{ "minotaur",		'm', 15, 15, 6,  0, 4, 10, 0 },
	{ "shopkeeper", 	'@', 12, 18, 0, 50, 4,  8, sizeof(struct eshk) }
};

struct permonst pm_ghost = { "ghost", ' ', 10, 3, -5, 50, 1, 1, sizeof(plname) };
#ifdef SAC
struct permonst pm_soldier = { "soldier", '3', 12, 4, -3, 15, 10, 4, 0 };
struct permonst pm_wizard = { "wizard of Yendor", '1', 20, 12, -8, 100, 2, 12, 0 };
#else
struct permonst pm_wizard = { "wizard of Yendor", '1', 15, 12, -2, 70, 1, 12, 0 };
#endif
#ifdef RPH
struct permonst pm_medusa = {"medusa", '8', 15, 12, 2, 50, 1, 8, 0};
#endif
#ifdef MAIL
struct permonst pm_mail_daemon = { "mail daemon", '2', 100, 1, 10, 127, 0, 0, 0 };
#endif
struct permonst pm_eel    = {"electric eel", ';', 15, 6, -3, 0, 3, 6, 0};
struct permonst pm_djinni = {"djinni",  '&', 10, 12, 0, 30, 2, 8, 0};
struct permonst pm_gremlin= {"gremlin", 'G', 3, 12, 2, 25, 1, 8, 0};
#ifdef STOOGES
struct permonst pm_larry  = {"Larry",   '@', 3, 12, 10, 0, 1, 6, 0};
struct permonst pm_curly  = {"Curly",   '@', 3, 12, 10, 0, 1, 6, 0};
struct permonst pm_moe    = {"Moe",     '@', 3, 12, 10, 0, 1, 6, 0};
#endif
