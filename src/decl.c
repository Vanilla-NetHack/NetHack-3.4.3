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
char *killer = 0;
char *nomovemsg = 0;
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

char *occtxt = DUMMY;
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char nyaqchars[] = "nyaq";
char *HI = DUMMY, *HE = DUMMY, *AS = DUMMY, *AE = DUMMY, *CD = DUMMY;
	/* set up in termcap.c */
int CO = 0, LI = 0;	/* set up in termcap.c: usually COLNO and ROWNO+3 */

#ifdef MSDOSCOLOR
char *HI_RED, *HI_YELLOW, *HI_GREEN, *HI_BLUE, *HI_WHITE; /* termcap.c */
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
char lock[PL_NSIZ+4] = "1lock";	/* long enough for login name .99 */
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
struct rm levl[COLNO][ROWNO] = DUMMY;		/* level map */
struct monst *fmon = 0;
struct trap *ftrap = 0;
struct gold *fgold = 0;
struct monst youmonst = DUMMY;	/* dummy; used as return value for boomhit */
struct flag flags = DUMMY;
struct you u = DUMMY;

struct obj *fobj = 0, *fcobj = 0, *invent = 0, *uwep = 0, *uarm = 0,
#ifdef SHIRT
	*uarmu = 0,		/* under-wear, so to speak */
#endif
#ifdef POLYSELF
	*uskin = 0,		/* dragon armor, if a dragon */
#endif
	*uarmc = 0, *uarmh = 0, *uarms = 0, *uarmg = 0, *uarmf = 0, *uamul = 0,
	*uright = 0, *uleft = 0, *ublindf = 0, *uchain = 0, *uball = 0;

const struct symbols defsyms = {
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
    '+', /* door */
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
struct symbols showsyms = DUMMY; /* will contain the symbols actually used */
#ifdef REINCARNATION
struct symbols savesyms = DUMMY;
#endif

#ifdef SPELLS
struct spell spl_book[MAXSPELL + 1] = DUMMY;
#endif

long moves = 1;
long wailmsg = 0;

struct obj zeroobj = DUMMY;	/* used to zero all elements of a struct obj */

struct obj *billobjs = 0;

const char black[] = "black";
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
const char white[] = "white";

const char nothing_happens[] = "Nothing happens.";
const char thats_enough_tries[] = "That's enough tries!";
