/*	SCCS Id: @(#)drawing.c	3.1	92/12/20	*/
/* Copyright (c) NetHack Development Team 1992.			  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "termcap.h"

/* Relevent header information in rm.h and objclass.h. */

#ifdef C
#undef C
#endif

#ifdef TEXTCOLOR
#define C(n) n
#else
#define C(n)
#endif

#define g_FILLER(symbol) 0

uchar oc_syms[MAXOCLASSES] = DUMMY; /* the current object  display symbols */
uchar showsyms[MAXPCHARS]  = DUMMY; /* the current feature display symbols */
uchar monsyms[MAXMCLASSES] = DUMMY; /* the current monster display symbols */

/* Default object class symbols.  See objclass.h. */
const char def_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0',		/* placeholder for the "random class" */
	ILLOBJ_SYM,
	WEAPON_SYM,
	ARMOR_SYM,
	RING_SYM,
/* 5*/	AMULET_SYM,
	TOOL_SYM,
	FOOD_SYM,
	POTION_SYM,
	SCROLL_SYM,
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	GOLD_SYM,
	GEM_SYM,
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};

/* Object descriptions.  Used in do_look(). */
const char *objexplain[] = {	/* these match def_oc_syms, above */
/* 0*/	0,
	"strange object",
	"weapon",
	"suit or piece of armor",
	"ring",
/* 5*/	"amulet",
	"useful item (pick-axe, key, lamp...)",
	"piece of food",
	"potion",
	"scroll",
/*10*/	"spell book",
	"wand",
	"pile of coins",
	"gem or rock",
	"boulder or statue",
/*15*/	"iron ball",
	"iron chain",
	"splash of venom"
};

/* Object class names.  Used in object_detect(). */
const char *oclass_names[] = {
/* 0*/	0,
	"illegal objects",
	"weapons",
	"armor",
	"rings",
/* 5*/	"amulets",
	"tools",
	"food",
	"potions",
	"scrolls",
/*10*/	"spell books",
	"wands",
	"coins",
	"rocks",
	"large stones",
/*15*/	"iron balls",
	"chains",
	"venoms"
};

/* Default monster class symbols.  See monsym.h. */
const char def_monsyms[MAXMCLASSES] = {
	'\0',		/* holder */
	DEF_ANT,
	DEF_BLOB,
	DEF_COCKATRICE,
	DEF_DOG,
	DEF_EYE,
	DEF_FELINE,
	DEF_GREMLIN,
	DEF_HUMANOID,
	DEF_IMP,
	DEF_JELLY,		/* 10 */
	DEF_KOBOLD,
	DEF_LEPRECHAUN,
	DEF_MIMIC,
	DEF_NYMPH,
	DEF_ORC,
	DEF_PIERCER,
	DEF_QUADRUPED,
	DEF_RODENT,
	DEF_SPIDER,
	DEF_TRAPPER,		/* 20 */
	DEF_UNICORN,
	DEF_VORTEX,
	DEF_WORM,
	DEF_XAN,
	DEF_LIGHT,
	DEF_ZRUTY,
	DEF_ANGEL,
	DEF_BAT,
	DEF_CENTAUR,
	DEF_DRAGON,		/* 30 */
	DEF_ELEMENTAL,
	DEF_FUNGUS,
	DEF_GNOME,
	DEF_GIANT,
	DEF_STALKER,
	DEF_JABBERWOCK,
	DEF_KOP,
	DEF_LICH,
	DEF_MUMMY,
	DEF_NAGA,		/* 40 */
	DEF_OGRE,
	DEF_PUDDING,
	DEF_QUANTMECH,
	DEF_RUSTMONST,
	DEF_SNAKE,
	DEF_TROLL,
	DEF_UMBER,
	DEF_VAMPIRE,
	DEF_WRAITH,
	DEF_XORN,		/* 50 */
	DEF_YETI,
	DEF_ZOMBIE,
	DEF_HUMAN,
	DEF_GHOST,
	DEF_GOLEM,
	DEF_DEMON,
	DEF_EEL,
	DEF_LIZARD,
	DEF_WORM_TAIL,
	DEF_MIMIC_DEF,		/* 60 */
};

const char *monexplain[MAXMCLASSES] = {
    0,
    "ant or other insect",	"blob",			"cockatrice",
    "dog or other canine",	"eye or sphere",	"feline",
    "gremlin",			"humanoid",		"imp or minor demon",
    "jelly",			"kobold",		"leprechaun",
    "mimic",			"nymph",		"orc",
    "piercer",			"quadruped",		"rodent",
    "spider",			"trapper or lurker above", "unicorn",
    "vortex",		"worm", "xan or other mythical/fantastic insect",
    "light",			"zruty",

    "angelic being",		"bat",			"centaur",
    "dragon",			"elemental",		"fungus or mold",
    "gnome",			"giant humanoid",	"invisible stalker",
    "jabberwock",		"Keystone Kop",		"lich",
    "mummy",			"naga",			"ogre",
    "pudding or ooze",		"quantum mechanic",	"rust monster",
    "snake",			"troll",		"umber hulk",
    "vampire",			"wraith",		"xorn",
    "yeti, ape or other large beast", "zombie",

    "human",			"ghost",		"golem",
    "demon",			"sea monster",		"lizard",
    "long worm tail",		"mimic"
};

/*
 *  Default screen symbols with explanations and colors.
 *  Note:  {ibm|dec|mac}_graphics[] arrays also depend on this symbol order.
 */
const struct symdef defsyms[MAXPCHARS] = {
/* 0*/	{' ', "dark part of a room",C(NO_COLOR)},	/* stone */
	{'|', "wall",		C(GRAY)},	/* vwall */
	{'-', "wall",		C(GRAY)},	/* hwall */
	{'-', "wall",		C(GRAY)},	/* tlcorn */
	{'-', "wall",		C(GRAY)},	/* trcorn */
	{'-', "wall",		C(GRAY)},	/* blcorn */
	{'-', "wall",		C(GRAY)},	/* brcorn */
	{'-', "wall",		C(GRAY)},	/* crwall */
	{'-', "wall",		C(GRAY)},	/* tuwall */
	{'-', "wall",		C(GRAY)},	/* tdwall */
/*10*/	{'|', "wall",		C(GRAY)},	/* tlwall */
	{'|', "wall",		C(GRAY)},	/* trwall */
	{'.', "doorway",	C(GRAY)},	/* ndoor */
	{'-', "open door",	C(BROWN)},	/* vodoor */
	{'|', "open door",	C(BROWN)},	/* hodoor */
	{'+', "closed door",	C(BROWN)},	/* vcdoor */
	{'+', "closed door",	C(BROWN)},	/* hcdoor */
	{'.', "floor of a room",C(GRAY)},	/* room */
	{'#', "corridor",	C(GRAY)},	/* dark corr */
	{'#', "lit corridor",	C(GRAY)},	/* lit corr */
/*20*/	{'<', "staircase up",	C(GRAY)},	/* upstair */
	{'>', "staircase down",	C(GRAY)},	/* dnstair */
	{'<', "ladder up",	C(BROWN)},	/* upladder */
	{'>', "ladder down",	C(BROWN)},	/* dnladder */
	{'^', "trap",		C(RED)},	/* trap */
	{'"', "web",		C(GRAY)},	/* web */
	{'_', "altar",		C(GRAY)},	/* altar */
	{'\\', "opulent throne",C(HI_GOLD)},	/* throne */
#ifdef SINKS
	{'#', "sink",		C(GRAY)},	/* sink */
#else
	{'#', "",		C(GRAY)},	/* sink */
#endif
	{'{', "fountain",	C(BLUE)},	/* fountain */
/*30*/	{'}', "water filled area",C(BLUE)},	/* pool */
	{'.', "ice",		C(CYAN)},	/* ice */
	{'}', "molten lava",	C(RED)},	/* lava */
	{'.', "lowered drawbridge",C(BROWN)},	/* vodbridge */
	{'.', "lowered drawbridge",C(BROWN)},	/* hodbridge */
	{'#', "raised drawbridge",C(BROWN)},	/* vcdbridge */
	{'#', "raised drawbridge",C(BROWN)},	/* hcdbridge */
	{' ', "air filled area",C(CYAN)},	/* open air */
	{'#', "cloud filled area",C(GRAY)},	/* part of a cloud */
	{'}', "water filled area",C(BLUE)},	/* under water */
/*40*/	{'|', "wall",		C(GRAY)},	/* vbeam */
	{'-', "wall",		C(GRAY)},	/* hbeam */
	{'\\',"wall",		C(GRAY)},	/* lslant */
	{'/', "wall",		C(GRAY)},	/* rslant */
	{'*', "",		C(WHITE)},	/* dig beam */
	{'!', "",		C(WHITE)},	/* camera flash beam */
	{')', "",		C(HI_METAL)},	/* boomerang open left */
	{'(', "",		C(HI_METAL)},	/* boomerang open right */
	{'0', "",		C(HI_ZAP)},	/* 4 magic shield symbols */
	{'#', "",		C(HI_ZAP)},
/*50*/	{'@', "",		C(HI_ZAP)},
	{'*', "",		C(HI_ZAP)},
	{'/', "",		C(GREEN)},	/* swallow top left	*/
	{'-', "",		C(GREEN)},	/* swallow top center	*/
	{'\\', "",		C(GREEN)},	/* swallow top right	*/
	{'|', "",		C(GREEN)},	/* swallow middle left	*/
	{'|', "",		C(GREEN)},	/* swallow middle right	*/
	{'\\', "",		C(GREEN)},	/* swallow bottom left	*/
	{'-', "",		C(GREEN)},	/* swallow bottom center*/
	{'/', "",		C(GREEN)},	/* swallow bottom right	*/
/*60*/	{'/', "",		C(ORANGE_COLORED)},/* explosion top left     */
	{'-', "",		C(ORANGE_COLORED)},/* explosion top center   */
	{'\\', "",		C(ORANGE_COLORED)},/* explosion top right    */
	{'|', "",		C(ORANGE_COLORED)},/* explosion middle left  */
	{' ', "",		C(ORANGE_COLORED)},/* explosion middle center*/
	{'|', "",		C(ORANGE_COLORED)},/* explosion middle right */
	{'\\', "",		C(ORANGE_COLORED)},/* explosion bottom left  */
	{'-', "",		C(ORANGE_COLORED)},/* explosion bottom center*/
	{'/', "",		C(ORANGE_COLORED)},/* explosion bottom right */
/*
 *  Note: Additions to this array should be reflected in the
 *	  {ibm,dec,mac}_graphics[] arrays below.
 */
};

#undef C

#ifdef ASCIIGRAPH
static uchar ibm_graphics[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xb3,	/* S_vwall:	meta-3, vertical rule */
	0xc4,	/* S_hwall:	meta-D, horizontal rule */
	0xda,	/* S_tlcorn:	meta-Z, top left corner */
	0xbf,	/* S_trcorn:	meta-?, top right corner */
	0xc0,	/* S_blcorn:	meta-@, bottom left */
	0xd9,	/* S_brcorn:	meta-Y, bottom right */
	0xc5,	/* S_crwall:	meta-E, cross */
	0xc1,	/* S_tuwall:	meta-A, T up */
	0xc2,	/* S_tdwall:	meta-B, T down */
/*10*/	0xb4,	/* S_tlwall:	meta-4, T left */
	0xc3,	/* S_trwall:	meta-C, T right */
	0xfa,	/* S_ndoor:	meta-z, centered dot */
	0xfe,	/* S_vodoor:	meta-~, small centered square */
	0xfe,	/* S_hodoor:	meta-~, small centered square */
	g_FILLER(S_vcdoor),
	g_FILLER(S_hcdoor),
	0xfa,	/* S_room:	meta-z, centered dot */
	0xb0,	/* S_corr:	meta-0, light shading */
	0xb1,	/* S_litcorr:	meta-1, medium shading */
/*20*/	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_trap),
	g_FILLER(S_web),
	g_FILLER(S_altar),
	g_FILLER(S_throne),
	g_FILLER(S_sink),
	0xf4,   /* S_fountain:  meta-t, integral top half */
/*30*/	0xf7,	/* S_pool:	meta-w, approx. equals */
	0xfa,	/* S_ice:	meta-z, centered dot */
	0xf7,	/* S_lava:	meta-w, approx. equals */
	0xfa,	/* S_vodbridge:	meta-z, centered dot */
	0xfa,	/* S_hodbridge:	meta-z, centered dot */
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
	0xf7,	/* S_water:	meta-w, approx. equals */
/*40*/	0xb3,   /* S_vbeam:     meta-3, vertical rule */
	0xc4,	/* S_hbeam:	meta-D, horizontal rule */
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
/*50*/	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	g_FILLER(S_sw_tc),
	g_FILLER(S_sw_tr),
	0xb3,	/* S_sw_ml:	meta-3, vertical rule */
	0xb3,	/* S_sw_mr:	meta-3, vertical rule */
	g_FILLER(S_sw_bl),
	g_FILLER(S_sw_bc),
	g_FILLER(S_sw_br),
/*60*/	g_FILLER(S_explode1),
	g_FILLER(S_explode2),
	g_FILLER(S_explode3),
	0xb3,	/* S_explode4:	meta-3, vertical rule */
	g_FILLER(S_explode5),
	0xb3,	/* S_explode6:	meta-3, vertical rule */
	g_FILLER(S_explode7),
	g_FILLER(S_explode8),
	g_FILLER(S_explode9)
};
#endif  /* ASCIIGRAPH */

#ifdef TERMLIB
void NDECL((*decgraphics_mode_callback)) = 0;  /* set in tty_start_screen() */

static uchar dec_graphics[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xf8,	/* S_vwall:	meta-x, vertical rule */
	0xf1,	/* S_hwall:	meta-q, horizontal rule */
	0xec,	/* S_tlcorn:	meta-l, top left corner */
	0xeb,	/* S_trcorn:	meta-k, top right corner */
	0xed,	/* S_blcorn:	meta-m, bottom left */
	0xea,	/* S_brcorn:	meta-j, bottom right */
	0xee,	/* S_crwall:	meta-n, cross */
	0xf6,	/* S_tuwall:	meta-v, T up */
	0xf7,	/* S_tdwall:	meta-w, T down */
/*10*/	0xf5,	/* S_tlwall:	meta-u, T left */
	0xf4,	/* S_trwall:	meta-t, T right */
	0xfe,	/* S_ndoor:	meta-~, centered dot */
	0xe1,	/* S_vodoor:	meta-a, solid block */
	0xe1,	/* S_hodoor:	meta-a, solid block */
	g_FILLER(S_vcdoor),
	g_FILLER(S_hcdoor),
	0xfe,	/* S_room:	meta-~, centered dot */
	g_FILLER(S_corr),
	g_FILLER(S_litcorr),
/*20*/	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_trap),
	g_FILLER(S_web),	/* 0xbd, /* \E)3: meta-=, int'l currency */
	g_FILLER(S_altar),	/* 0xc3, /* \E)3: meta-C, dagger */
	g_FILLER(S_throne),
	g_FILLER(S_sink),
	g_FILLER(S_fountain),   /* 0xdb, /* \E)3: meta-[, integral top half */
/*30*/	0xe0,	/* S_pool:	meta-\, diamond */
	0xfe,	/* S_ice:	meta-~, centered dot */
	0xe0,	/* S_lava:	meta-\, diamond */
	0xfe,	/* S_vodbridge:	meta-~, centered dot */
	0xfe,	/* S_hodbridge:	meta-~, centered dot */
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
	0xe0,	/* S_water:	meta-\, diamond */
/*40*/	0xf8,   /* S_vbeam:     meta-x, vertical rule */
	0xf1,	/* S_hbeam:	meta-q, horizontal rule */
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
/*50*/	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	0xef,	/* S_sw_tc:	meta-o, high horizontal line */
	g_FILLER(S_sw_tr),
	0xf8,	/* S_sw_ml:	meta-x, vertical rule */
	0xf8,	/* S_sw_mr:	meta-x, vertical rule */
	g_FILLER(S_sw_bl),
	0xf3,	/* S_sw_bc:	meta-s, low horizontal line */
	g_FILLER(S_sw_br),
/*60*/	g_FILLER(S_explode1),
	0xef,	/* S_explode2:	meta-o, high horizontal line */
	g_FILLER(S_explode3),
	0xf8,	/* S_explode4:	meta-x, vertical rule */
	g_FILLER(S_explode5),
	0xf8,	/* S_explode6:	meta-x, vertical rule */
	g_FILLER(S_explode7),
	0xf3,	/* S_explode8:	meta-s, low horizontal line */
	g_FILLER(S_explode9)
};
#endif  /* TERMLIB */

#ifdef MAC_GRAPHICS_ENV
static uchar mac_graphics[MAXPCHARS] = {
/* 0*/	g_FILLER(S_stone),
	0xba,	/* S_vwall */
	0xcd,	/* S_hwall */
	0xc9,	/* S_tlcorn */
	0xbb,	/* S_trcorn */
	0xc8,	/* S_blcorn */
	0xbc,	/* S_brcorn */
	0xce,	/* S_crwall */
	0xca,	/* S_tuwall */
	0xcb,	/* S_tdwall */
/*10*/	0xb9,	/* S_tlwall */
	0xcc,	/* S_trwall */
	0xb0,	/* S_ndoor */
	0xee,	/* S_vodoor */
	0xee,	/* S_hodoor */
	0xef,	/* S_vcdoor */
	0xef,	/* S_hcdoor */
	g_FILLER(S_Room),
	0xB0,	/* S_corr */
	g_FILLER(S_litcorr),
/*20*/	g_FILLER(S_upstair),
	g_FILLER(S_dnstair),
	g_FILLER(S_upladder),
	g_FILLER(S_dnladder),
	g_FILLER(S_trap),
	g_FILLER(S_web),
	g_FILLER(S_altar),
	g_FILLER(S_throne),
	g_FILLER(S_sink),
	g_FILLER(S_fountain),
/*30*/	0xe0,
	g_FILLER(S_ice),
	g_FILLER(S_lava),
	g_FILLER(S_vodbridge),
	g_FILLER(S_hodbridge),
	g_FILLER(S_vcdbridge),
	g_FILLER(S_hcdbridge),
	g_FILLER(S_air),
	g_FILLER(S_cloud),
	g_FILLER(S_water),
/*40*/	g_FILLER(S_vbeam),
	g_FILLER(S_hbeam),
	g_FILLER(S_lslant),
	g_FILLER(S_rslant),
	g_FILLER(S_digbeam),
	g_FILLER(S_flashbeam),
	g_FILLER(S_boomleft),
	g_FILLER(S_boomright),
	g_FILLER(S_ss1),
	g_FILLER(S_ss2),
/*50*/	g_FILLER(S_ss3),
	g_FILLER(S_ss4),
	g_FILLER(S_sw_tl),
	g_FILLER(S_sw_tc),
	g_FILLER(S_sw_tr),
	g_FILLER(S_sw_ml),
	g_FILLER(S_sw_mr),
	g_FILLER(S_sw_bl),
	g_FILLER(S_sw_bc),
	g_FILLER(S_sw_br),
/*60*/	g_FILLER(S_explode1),
	g_FILLER(S_explode2),
	g_FILLER(S_explode3),
	g_FILLER(S_explode4),
	g_FILLER(S_explode5),
	g_FILLER(S_explode6),
	g_FILLER(S_explode7),
	g_FILLER(S_explode8),
	g_FILLER(S_explode9)
};
#endif  /* MAC_GRAPHICS_ENV */

/*
 * Convert the given character to an object class.  If the character is not
 * recognized, then MAXOCLASSES is returned.  Used in invent.c, options.c,
 * pickup.c, sp_lev.c, and lev_main.c.
 */
int
def_char_to_objclass(ch)
    char ch;
{
    int i;
    for (i = 1; i < MAXOCLASSES; i++)
	if (ch == def_oc_syms[i]) break;
    return i;
}

/*
 * Convert a character into a monster class.  This returns the _first_
 * match made.  If there are are no matches, return MAXMCLASSES.
 */
int
def_char_to_monclass(ch)
    char ch;
{
    int i;
    for (i = 1; i < MAXMCLASSES; i++)
	if (def_monsyms[i] == ch) break;
    return i;
}

void
assign_graphics(graph_chars, glth)
register uchar *graph_chars;
register int glth;
{
    register int i;

    for (i = 0; i < MAXPCHARS; i++)
	showsyms[i] = (((i < glth) && graph_chars[i]) ?
		       graph_chars[i] : defsyms[i].sym);
}

void
switch_graphics(gr_set_flag)
int gr_set_flag;
{
    switch (gr_set_flag) {
	default:
	case ASCII_GRAPHICS:
	    assign_graphics((uchar *)0, 0);
	    break;
#ifdef ASCIIGRAPH
	case IBM_GRAPHICS:
/*
 * Use the nice IBM Extended ASCII line-drawing characters (codepage 437).
 *
 * OS/2 defaults to a multilingual character set (codepage 850, corresponding
 * to the ISO 8859 character set.  We should probably do a VioSetCp() call to
 * set the codepage to 437.
 */
	    flags.IBMgraphics = TRUE;
	    flags.DECgraphics = FALSE;
	    assign_graphics(ibm_graphics, SIZE(ibm_graphics));
	    break;
#endif /* ASCIIGRAPH */
#ifdef TERMLIB
	case DEC_GRAPHICS:
/*
 * Use the VT100 line drawing character set.
 */
	    flags.DECgraphics = TRUE;
	    flags.IBMgraphics = FALSE;
	    assign_graphics(dec_graphics, SIZE(dec_graphics));
	    if (decgraphics_mode_callback) (*decgraphics_mode_callback)();
	    break;
#endif /* TERMLIB */
#ifdef MAC_GRAPHICS_ENV
	case MAC_GRAPHICS:
	    assign_graphics(mac_graphics, SIZE(mac_graphics));
	    break;
#endif
	}
    return;
}


#ifdef REINCARNATION

/*
 * saved display symbols for objects & features.
 */
static uchar save_oc_syms[MAXOCLASSES] = DUMMY;
static uchar save_showsyms[MAXPCHARS]  = DUMMY;
static uchar save_monsyms[MAXPCHARS]   = DUMMY;

static const uchar r_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0',
	ILLOBJ_SYM,
	WEAPON_SYM,
	']',			/* armor */
	RING_SYM,
/* 5*/	',',			/* amulet */
	TOOL_SYM,
	':',			/* food */
	POTION_SYM,
	SCROLL_SYM,
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	GEM_SYM,		/* gold -- yes it's the same as gems */
	GEM_SYM,
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};

# ifdef ASCIIGRAPH
static const uchar IBM_r_oc_syms[MAXOCLASSES] = {	/* a la EPYX Rogue */
/* 0*/	'\0',
	ILLOBJ_SYM,
	0x18,			/* weapon: up arrow */
/*	0x0a, */ ARMOR_SYM,	/* armor:  Vert rect with o */
/*	0x09, */ RING_SYM,	/* ring:   circle with arrow */
/* 5*/	0x0c,			/* amulet: "female" symbol */
	TOOL_SYM,
	0x05,			/* food:   club (as in cards) */
	0xad,			/* potion: upside down '!' */
	0x0e,			/* scroll: musical note */
/*10*/	SPBOOK_SYM,
	0xe7,			/* wand:   greek tau */
	0x0f,			/* gold:   yes it's the same as gems */
	0x0f,			/* gems:   fancy '*' */
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM
};
# endif /* ASCIIGRAPH */

void
assign_rogue_graphics(is_rlevel)
boolean is_rlevel;
{
    /* Adjust graphics display characters on Rogue levels */

    if (is_rlevel) {
	register int i;

	(void) memcpy((genericptr_t)save_showsyms,
		      (genericptr_t)showsyms, sizeof showsyms);
	(void) memcpy((genericptr_t)save_oc_syms,
		      (genericptr_t)oc_syms, sizeof oc_syms);
	(void) memcpy((genericptr_t)save_monsyms,
		      (genericptr_t)monsyms, sizeof monsyms);

	/* Use a loop: char != uchar on some machines. */
	for (i = 0; i < MAXMCLASSES; i++)
	    monsyms[i] = def_monsyms[i];
	for (i = 0; i < MAXPCHARS; i++)
	    showsyms[i] = defsyms[i].sym;

/*
 * Some day if these rogue showsyms get much more extensive than this,
 * we may want to create r_showsyms, and IBM_r_showsyms arrays to hold
 * all of this info and to simply initialize it via a for() loop like r_oc_syms.
 */

# ifdef ASCIIGRAPH
	if (!flags.IBMgraphics) {
# endif
	    showsyms[S_vodoor]  = showsyms[S_hodoor]  = showsyms[S_ndoor] = '+';
	    showsyms[S_upstair] = showsyms[S_dnstair] = '%';
# ifdef ASCIIGRAPH
	} else {
	    /* a la EPYX Rogue */
	    showsyms[S_vwall]   = 0xba; /* all walls now use	*/
	    showsyms[S_hwall]   = 0xcd; /* double line graphics	*/
	    showsyms[S_tlcorn]  = 0xc9;
	    showsyms[S_trcorn]  = 0xbb;
	    showsyms[S_blcorn]  = 0xc8;
	    showsyms[S_brcorn]  = 0xbc;
	    showsyms[S_crwall]  = 0xce;
	    showsyms[S_tuwall]  = 0xca;
	    showsyms[S_tdwall]  = 0xcb;
	    showsyms[S_trwall]  = 0xcc;
	    showsyms[S_tlwall]  = 0xb9;
	    showsyms[S_corr]    = 0xb1;
	    showsyms[S_litcorr] = 0xb2;
	    showsyms[S_upstair] = 0xf0; /* Greek Xi */
	    showsyms[S_dnstair] = 0xf0;
	    showsyms[S_trap]    = 0x04; /* diamond (cards) */
	    showsyms[S_vodoor]  = 0xce;
	    showsyms[S_hodoor]  = 0xce;
	    showsyms[S_ndoor]   = 0xce;
	}
#endif /* ASCIIGRAPH */

	for (i = 0; i < MAXOCLASSES; i++) {
#ifdef ASCIIGRAPH
	    if (flags.IBMgraphics)
		oc_syms[i] = IBM_r_oc_syms[i];
	    else
#endif /* ASCIIGRAPH */
		oc_syms[i] = r_oc_syms[i];
	}

    } else {
	(void) memcpy((genericptr_t)showsyms,
		      (genericptr_t)save_showsyms, sizeof showsyms);
	(void) memcpy((genericptr_t)oc_syms,
		      (genericptr_t)save_oc_syms, sizeof oc_syms);
	(void) memcpy((genericptr_t)monsyms,
		      (genericptr_t)save_monsyms, sizeof monsyms);
    }
}
#endif /* REINCARNATION */

/*drawing.c*/
