/*	SCCS Id: @(#)decl.c	3.1	92/12/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

int NDECL((*afternmv));
int NDECL((*occupation));

/* from xxxmain.c */
const char *hname = 0;		/* name of the game (argv[0] of main) */
int hackpid = 0;		/* current process id */
#if defined(UNIX) || defined(VMS)
int locknum = 0;		/* max num of simultaneous users */
#endif
#ifdef DEF_PAGER
char *catmore = 0;		/* default pager */
#endif

int NEARDATA bases[MAXOCLASSES] = DUMMY;

int NEARDATA multi = 0;
int NEARDATA warnlevel = 0;		/* used by movemon and dochugw */
int NEARDATA nroom = 0;
int NEARDATA nsubroom = 0;
int NEARDATA occtime = 0;

int x_maze_max, y_maze_max;	/* initialized in main, used in mkmaze.c */
int otg_temp;			/* used by object_to_glyph() [otg] */

#ifdef REDO
int NEARDATA in_doagain = 0;
#endif

/*
 *	The following structure will be initialized at startup time with
 *	the level numbers of some "important" things in the game.
 */
struct dgn_topology dungeon_topology = {DUMMY};

#ifdef MULDGN
#include	"quest.h"
struct q_score	quest_status = DUMMY;
#endif

int NEARDATA smeq[MAXNROFROOMS+1] = DUMMY;
int NEARDATA doorindex = 0;

char NEARDATA *save_cm = 0;
int NEARDATA killer_format = 0;
const char NEARDATA *killer = 0;
const char NEARDATA *nomovemsg = 0;
const char NEARDATA nul[40] = DUMMY;		/* contains zeros */
char NEARDATA plname[PL_NSIZ] = DUMMY;		/* player name */
char NEARDATA pl_character[PL_CSIZ] = DUMMY;

#ifdef TUTTI_FRUTTI
char NEARDATA pl_fruit[PL_FSIZ] = DUMMY;
int NEARDATA current_fruit = 0;
struct fruit NEARDATA *ffruit = (struct fruit *)0;
#endif

char NEARDATA tune[6] = DUMMY;

const char NEARDATA *occtxt = DUMMY;
const char NEARDATA quitchars[] = " \r\n\033";
const char NEARDATA vowels[] = "aeiouAEIOU";
const char NEARDATA ynchars[] = "yn";
const char NEARDATA ynqchars[] = "ynq";
const char NEARDATA ynaqchars[] = "ynaq";
const char NEARDATA ynNaqchars[] = "yn#aq";
long NEARDATA yn_number = 0L;

#ifdef MICRO
char hackdir[PATHLEN];		/* where rumors, help, record are */
char levels[PATHLEN];		/* where levels are */
#endif /* MICRO */

#define INFOSIZE	MAXLEVEL * MAXDUNGEON

#ifdef MFLOPPY
char permbones[PATHLEN];	/* where permanent copy of bones go */
int ramdisk = FALSE;		/* whether to copy bones to levels or not */
int saveprompt = TRUE;
struct finfo fileinfo[INFOSIZE];
const char *alllevels = "levels.*";
const char *allbones = "bones*.*";
#else
boolean level_exists[INFOSIZE];
#endif

#undef INFOSIZE

/* 'rogue'-like direction commands (cmd.c) */
const char NEARDATA sdir[] = "hykulnjb><";
const char NEARDATA ndir[] = "47896321><";	/* number pad mode */
const schar NEARDATA xdir[10] = { -1,-1, 0, 1, 1, 1, 0,-1, 0, 0 };
const schar NEARDATA ydir[10] = {  0,-1,-1,-1, 0, 1, 1, 1, 0, 0 };
const schar zdir[10]	      = {  0, 0, 0, 0, 0, 0, 0, 0, 1,-1 };

schar NEARDATA tbx = 0, NEARDATA tby = 0;	/* mthrowu: target */
int NEARDATA dig_effort = 0;	/* effort expended on current pos */
d_level NEARDATA dig_level = { 0, 0 };
coord NEARDATA dig_pos = DUMMY;
boolean NEARDATA dig_down = FALSE;

dungeon NEARDATA dungeons[MAXDUNGEON];	/* ini'ed by init_dungeon() */
s_level NEARDATA *sp_levchn;
int NEARDATA done_stopprint = 0;
int NEARDATA done_hup = 0;
stairway NEARDATA upstair = { 0, 0 }, NEARDATA dnstair = { 0, 0 };
stairway NEARDATA upladder = { 0, 0 }, NEARDATA dnladder = { 0, 0 };
stairway NEARDATA sstairs = { 0, 0 };
dest_area NEARDATA updest = { 0, 0, 0, 0, 0, 0, 0, 0 };
dest_area NEARDATA dndest = { 0, 0, 0, 0, 0, 0, 0, 0 };
coord NEARDATA inv_pos = { 0, 0 };

boolean NEARDATA in_mklev = FALSE;
boolean	NEARDATA stoned = FALSE;	/* done to monsters hit by 'c' */
boolean	NEARDATA unweapon = FALSE;
boolean NEARDATA mrg_to_wielded = FALSE;
			 /* weapon picked is merged with wielded one */

#ifdef KOPS
boolean NEARDATA allow_kops = TRUE;
#endif

coord NEARDATA bhitpos = DUMMY;
coord NEARDATA doors[DOORMAX] = {DUMMY};

struct mkroom NEARDATA rooms[(MAXNROFROOMS+1)*2] = {DUMMY};
struct mkroom* NEARDATA subrooms = &rooms[MAXNROFROOMS+1];
struct mkroom *upstairs_room, *dnstairs_room, *sstairs_room;

dlevel_t level;		/* level map */
struct trap *ftrap = (struct trap *)0;
struct monst NEARDATA youmonst = DUMMY;
struct flag NEARDATA flags = DUMMY;
struct you NEARDATA u = DUMMY;

struct obj NEARDATA *invent = (struct obj *)0, 
        NEARDATA *uwep = (struct obj *)0, NEARDATA *uarm = (struct obj *)0,
#ifdef TOURIST
	NEARDATA *uarmu = (struct obj *)0, /* under-wear, so to speak */
#endif
#ifdef POLYSELF
	NEARDATA *uskin = (struct obj *)0, /* dragon armor, if a dragon */
#endif
	NEARDATA *uarmc = (struct obj *)0, NEARDATA *uarmh = (struct obj *)0, 
        NEARDATA *uarms = (struct obj *)0, NEARDATA *uarmg = (struct obj *)0,
        NEARDATA *uarmf = (struct obj *)0, NEARDATA *uamul = (struct obj *)0,
	NEARDATA *uright = (struct obj *)0,
        NEARDATA *uleft = (struct obj *)0,
        NEARDATA *ublindf = (struct obj *)0,
	NEARDATA *uchain = (struct obj *)0,
        NEARDATA *uball = (struct obj *)0;

#ifdef TEXTCOLOR
/*
 *  This must be the same order as used for buzz() in zap.c.
 */
const int zapcolors[NUM_ZAP] = {
    HI_ZAP,		/* 0 - missile */
    ORANGE_COLORED,	/* 1 - fire */
    WHITE,		/* 2 - frost */
    HI_ZAP,		/* 3 - sleep */
    BLACK,		/* 4 - death */
    HI_ZAP,		/* 5 - lightning */
    YELLOW,		/* 6 - poison gas */
    GREEN,		/* 7 - acid */
};
#endif /* text color */

const int shield_static[SHIELD_COUNT] = {
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,	/* 7 per row */
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
};

struct spell NEARDATA spl_book[MAXSPELL + 1] = {DUMMY};

long NEARDATA moves = 1L, NEARDATA monstermoves = 1L;
	 /* These diverge when player is Fast */
long NEARDATA wailmsg = 0L;

/* objects that are moving to another dungeon level */
struct obj NEARDATA *migrating_objs = (struct obj *)0;
/* objects not yet paid for */
struct obj NEARDATA *billobjs = (struct obj *)0;

/* used to zero all elements of a struct obj */
struct obj NEARDATA zeroobj = DUMMY;

/* originally from dog.c */
char NEARDATA dogname[63] = DUMMY;
char NEARDATA catname[63] = DUMMY;
char preferred_pet;	/* '\0', 'c', 'd' */
/* monsters that went down/up together with @ */
struct monst NEARDATA *mydogs = (struct monst *)0;
/* monsters that are moving to another dungeon level */
struct monst NEARDATA *migrating_mons = (struct monst *)0;

struct c_color_names NEARDATA c_color_names = {
	"black", "amber", "golden",
	"light blue", "red", "green",
	"silver", "blue", "purple",
	"white"
};

struct c_common_strings c_common_strings = {
	"Nothing happens.",		"That's enough tries!",
	"That is a silly thing to %s.",	"shudder for a moment."
};

/* Vision */
boolean	NEARDATA vision_full_recalc = 0;
char	NEARDATA **viz_array = 0;/* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
winid NEARDATA WIN_MESSAGE = WIN_ERR, NEARDATA WIN_STATUS = WIN_ERR;
winid NEARDATA WIN_MAP = WIN_ERR, NEARDATA WIN_INVEN = WIN_ERR;
char toplines[BUFSZ];
/* Windowing stuff that's really tty oriented, but present for all ports */
struct tc_gbl_data tc_gbl_data = { 0,0, 0,0 };	/* AS,AE, LI,CO */

#ifdef TOURIST
const char NEARDATA *pl_classes = "ABCEHKPRSTVW";
#else
const char NEARDATA *pl_classes = "ABCEHKPRSVW";
#endif

/* dummy routine used to force linkage */
void
decl_init()
{
    return;
}

/*decl.c*/
