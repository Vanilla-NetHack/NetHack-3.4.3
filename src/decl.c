/*	SCCS Id: @(#)decl.c	3.0	88/10/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

int multi = 0;
int warnlevel = 0;		/* used by movemon and dochugw */
int nroom = 0;
int occtime = 0;

int x_maze_max, y_maze_max;	/* initialized in main, used in mkmaze.c */

#ifdef REDO
int in_doagain = 0;
#endif

/*
 *	The following integers will be initialized at load time with
 *	the level numbers of some "important" things in the game.
 */

 int
	medusa_level = 0,	/* level that the medusa lives on */
	bigroom_level = 0,	/* level consisting of a single big room */
#ifdef REINCARNATION
	rogue_level = 0,	/* level near which rogue level gen'd */
#endif
#ifdef ORACLE
	oracle_level = 0,	/* level near which Oracle gen'd */
#endif
#ifdef STRONGHOLD
	stronghold_level = 3,	/* level the castle is on */
	/* Not 0, otherwise they start the game in Hell and burn immediately */
	tower_level = 0,	/* level of the top of Vlad's 3-level tower */
#endif
	wiz_level = 0;		/* level that the wiz lives on */
boolean is_maze_lev = FALSE;    /* if this is a maze level */

int smeq[MAXNROFROOMS+1] = DUMMY;
int doorindex = 0;

char *save_cm = 0;
int killer_format = 0;
const char *killer = 0;
const char *nomovemsg = 0;
const char nul[40] = DUMMY;		/* contains zeros */
char plname[PL_NSIZ] = DUMMY;		/* player name */
char pl_character[PL_CSIZ] = DUMMY;

#ifdef TUTTI_FRUTTI
char pl_fruit[PL_FSIZ] = DUMMY;
int current_fruit = 0;
struct fruit *ffruit = 0;
#endif

#ifdef STRONGHOLD
char tune[6] = DUMMY;
#  ifdef MUSIC
schar music_heard = 0;
#  endif
#endif

#ifdef SMALLDATA
const char *occtxt = 0;
#else
const char *occtxt = DUMMY;
#endif
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char nyaqchars[] = "nyaq";

#ifdef SMALLDATA
char *HI = 0, *HE = 0, *AS = 0, *AE = 0, *CD = 0;
	/* set up in termcap.c */
#else
char *HI = DUMMY, *HE = DUMMY, *AS = DUMMY, *AE = DUMMY, *CD = DUMMY;
	/* set up in termcap.c */
#endif
int CO = 0, LI = 0;	/* set up in termcap.c: usually COLNO and ROWNO+3 */

#ifdef CLIPPING
boolean clipping;	/* clipping on? */
int clipx, clipy, clipxmax, clipymax;
#endif

#ifdef TEXTCOLOR
# ifdef TOS
const char *hilites[MAXCOLORS];	/* terminal escapes for the various colors */
# else
char *hilites[MAXCOLORS];	/* terminal escapes for the various colors */
# endif
#endif
#ifdef MSDOS
char hackdir[PATHLEN];		/* where rumors, help, record are */
const char *configfile = "NetHack.cnf";	/* read by read_config_file() */
char levels[PATHLEN];		/* where levels are */
#endif /* MSDOS */

#ifdef DGK
char lock[FILENAME];		/* pathname of level files */
char permbones[PATHLEN];	/* where permanent copy of bones go */
int ramdisk = FALSE;		/* whether to copy bones to levels or not */
int saveprompt = TRUE;
const char *alllevels = "levels.*";
const char *allbones = "bones.*";
#else
# ifdef VMS
char lock[PL_NSIZ+16] = "1lock";/* long enough for uic+login_name+.99;1 */
# else
char lock[PL_NSIZ+14] = "1lock";/* long enough for uic+login_name+.99 */
# endif
#endif

int dig_effort = 0;	/* effort expended on current pos */
uchar dig_level = 0;
coord dig_pos = DUMMY;
boolean dig_down = FALSE;

xchar dlevel = 1;
xchar maxdlevel = 1;
int done_stopprint = 0;
int done_hup = 0;
xchar xupstair = 0, yupstair = 0, xdnstair = 0, ydnstair = 0;
#ifdef STRONGHOLD
xchar xupladder = 0, yupladder = 0, xdnladder = 0, ydnladder = 0;
#endif
xchar curx = 0, cury = 0;
xchar seelx = 0, seehx = 0, seely = 0, seehy = 0; /* corners of lit room */
xchar seelx2 = 0, seehx2 = 0, seely2 = 0, seehy2 = 0; /* corners of lit room */
xchar scrlx = 0, scrhx = 0, scrly = 0, scrhy = 0; /* corners of new scr. area*/
xchar fountsound = 0;
xchar sinksound = 0; /* numbers of other things that make noise */

boolean in_mklev = FALSE;
boolean	stoned = FALSE;			/* done to monsters hit by 'c' */
boolean	unweapon = FALSE;
boolean mrg_to_wielded = FALSE; /* weapon picked is merged with wielded one */

#ifdef KOPS
boolean allow_kops = TRUE;
#endif

coord bhitpos = DUMMY;
coord doors[DOORMAX] = DUMMY;

struct mkroom rooms[MAXNROFROOMS+1] = DUMMY;
dlevel_t level;		/* level map */
struct trap *ftrap = 0;
struct gold *fgold = 0;
struct monst youmonst = DUMMY;	/* dummy; used as return value for boomhit */
struct flag flags = DUMMY;
struct you u = DUMMY;

struct obj *fcobj = 0, *invent = 0, *uwep = 0, *uarm = 0,
#ifdef SHIRT
	*uarmu = 0,		/* under-wear, so to speak */
#endif
#ifdef POLYSELF
	*uskin = 0,		/* dragon armor, if a dragon */
#endif
	*uarmc = 0, *uarmh = 0, *uarms = 0, *uarmg = 0, *uarmf = 0, *uamul = 0,
	*uright = 0, *uleft = 0, *ublindf = 0, *uchain = 0, *uball = 0;

symbol_array defsyms = {
    ' ', /* stone */
    '|', /* vwall */
    '-', /* hwall */
    '-', /* tlcorn */
    '-', /* trcorn */
    '-', /* blcorn */
    '-', /* brcorn */
    '-', /* crwall */
    '-', /* tuwall */
    '-', /* tdwall */
    '|', /* tlwall */
    '|', /* trwall */
    '|', /* vbeam */
    '-', /* hbeam */
    '\\', /* lslant */
    '/', /* rslant */
    '.', /* ndoor */
    '-', /* vodoor */
    '|', /* hodoor */
    '+', /* cdoor */
    '.', /* room */
    '#', /* corr */
    '<', /* upstair */
    '>', /* dnstair */
    '^', /* trap */
    '"', /* web */
    '}', /* pool */
    '{', /* fountain */	/* used ifdef FOUNTAINS */
    '#', /* sink */	/* used ifdef SINKS */
    '\\', /* throne */	/* used ifdef THRONES */
    '_', /* altar */	/* used ifdef ALTARS */
    '<', /* upladder */	/* used ifdef STRONGHOLD */
    '>', /* dnladder */	/* used ifdef STRONGHOLD */
    '#', /* dbvwall */	/* used ifdef STRONGHOLD */
    '#', /* dbhwall */	/* used ifdef STRONGHOLD */
};
symbol_array showsyms = DUMMY; /* will contain the symbols actually used */
#ifdef REINCARNATION
symbol_array savesyms = DUMMY;
#endif

const char *explainsyms[MAXPCHARS] = {
	"dark part of a room", "wall", "wall",
	"wall", "wall", "wall",
	"wall", "wall", "wall",
	"wall", "wall", "wall",
	"wall", "wall", "wall",
	"wall", "doorway", "open door",
	"open door", "closed door", "floor of a room",
	"corridor", "staircase up", "staircase down",
	"trap", "web", "water filled area",
#ifdef FOUNTAINS
	"fountain",
#else
	"",
#endif
#ifdef SINKS
	"sink",
#else
	"",
#endif
#ifdef THRONES
	"opulent throne",
#else
	"",
#endif
#ifdef ALTARS
	"altar",
#else
	"",
#endif
#ifdef STRONGHOLD
	"ladder up", "ladder down", "drawbridge", "drawbridge"
#else
	"", "", "", ""
#endif
};

#ifdef SPELLS
struct spell spl_book[MAXSPELL + 1] = DUMMY;
#endif

long moves = 1, monstermoves = 1; /* These diverge when player is Fast */
long wailmsg = 0;

struct obj zeroobj = DUMMY;	/* used to zero all elements of a struct obj */

struct obj *billobjs = 0;

#ifdef THINK_C
const char Black[] = "black";
#else
const char black[] = "black";
#endif
const char amber[] = "amber";
#ifdef THEOLOGY
const char golden[] = "golden";
#endif
const char light_blue[] = "light blue";
const char red[] = "red";
const char green[] = "green";
const char silver[] = "silver";
const char blue[] = "blue";
const char purple[] = "purple";
#ifdef THINK_C
const char White[] = "white";
#else
const char white[] = "white";
#endif

const char nothing_happens[] = "Nothing happens.";
const char thats_enough_tries[] = "That's enough tries!";

const char monsyms[] = { S_HUMAN, S_GHOST, S_ANT, S_BLOB, S_COCKATRICE, S_DOG,
S_EYE, S_FELINE, S_GREMLIN, S_HUMANOID, S_IMP, S_JELLY, S_KOBOLD,
S_LEPRECHAUN, S_MIMIC, S_NYMPH, S_ORC, S_PIERCER, S_QUADRUPED, S_RODENT,
S_SPIDER, S_TRAPPER, S_UNICORN, S_VORTEX, S_WORM, S_XAN, S_YLIGHT, S_ZRUTY,
S_APE, S_BAT, S_CENTAUR, S_DRAGON, S_ELEMENTAL, S_FUNGUS, S_GNOME, S_GIANT,
S_STALKER, S_JABBERWOCK,
#ifdef KOPS
S_KOP,
#endif
S_LICH, S_MUMMY, S_NAGA, S_OGRE, S_PUDDING, S_QUANTMECH, S_RUSTMONST, S_SNAKE,
S_TROLL, S_UMBER, S_VAMPIRE, S_WRAITH, S_XORN, S_YETI, S_ZOMBIE,
#ifdef GOLEMS
S_GOLEM,
#endif
S_DEMON, S_EEL, S_LIZARD,
#ifdef WORM
S_WORM_TAIL,
#endif
0 };

const char objsyms[] = { WEAPON_SYM, ARMOR_SYM, POTION_SYM, SCROLL_SYM,
WAND_SYM,
#ifdef SPELLS
SPBOOK_SYM,
#endif
RING_SYM, AMULET_SYM, FOOD_SYM, TOOL_SYM, GEM_SYM, GOLD_SYM,
ROCK_SYM, BALL_SYM, CHAIN_SYM, 0 };

const char *monexplain[] = {
"human", "ghost", "ant or other insect", "blob", "cockatrice",
"dog or other canine", "eye or sphere", "feline", "gremlin", "humanoid",
"imp or minor demon", "jelly", "kobold", "leprechaun", "mimic",
"nymph", "orc", "piercer", "quadruped", "rodent",
"spider", "trapper or lurker above", "unicorn", "vortex", "worm",
"xan or other mythical/fantastic insect", "yellow light", "zruty",
"ape", "bat", "centaur", "dragon", "elemental",
"fungus or mold", "gnome", "giant humanoid", "invisible stalker", "jabberwock",
#ifdef KOPS
"Keystone Kop",
#endif
"lich", "mummy", "naga", "ogre", "pudding or ooze",
"quantum mechanic", "rust monster", "snake", "troll", "umber hulk",
"vampire", "wraith", "xorn", "yeti", "zombie",
#ifdef GOLEMS
"golem",
#endif
"demon",  "sea monster", "lizard",
#ifdef WORM
"long worm tail",
#endif
};

const char *objexplain[] = {
"weapon", "suit or piece of armor", "potion", "scroll", "wand",
#ifdef SPELLS
"spell book",
#endif
"ring", "amulet", "piece of food", "useful item (pick-axe, key, lamp...)",
"gem or rock", "pile of gold", "boulder or statue", "iron ball", "iron chain"
};
