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

NEARDATA int bases[MAXOCLASSES] = DUMMY;

NEARDATA int multi = 0;
NEARDATA int warnlevel = 0;		/* used by movemon and dochugw */
NEARDATA int nroom = 0;
NEARDATA int nsubroom = 0;
NEARDATA int occtime = 0;

int x_maze_max, y_maze_max;	/* initialized in main, used in mkmaze.c */
int otg_temp;			/* used by object_to_glyph() [otg] */

#ifdef REDO
NEARDATA int in_doagain = 0;
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

NEARDATA int smeq[MAXNROFROOMS+1] = DUMMY;
NEARDATA int doorindex = 0;

NEARDATA char *save_cm = 0;
NEARDATA int killer_format = 0;
NEARDATA const char *killer = 0;
NEARDATA const char *nomovemsg = 0;
NEARDATA const char nul[40] = DUMMY;		/* contains zeros */
NEARDATA char plname[PL_NSIZ] = DUMMY;		/* player name */
NEARDATA char pl_character[PL_CSIZ] = DUMMY;

#ifdef TUTTI_FRUTTI
NEARDATA char pl_fruit[PL_FSIZ] = DUMMY;
NEARDATA int current_fruit = 0;
NEARDATA struct fruit *ffruit = (struct fruit *)0;
#endif

NEARDATA char tune[6] = DUMMY;

NEARDATA const char *occtxt = DUMMY;
NEARDATA const char quitchars[] = " \r\n\033";
NEARDATA const char vowels[] = "aeiouAEIOU";
NEARDATA const char ynchars[] = "yn";
NEARDATA const char ynqchars[] = "ynq";
NEARDATA const char ynaqchars[] = "ynaq";
NEARDATA const char ynNaqchars[] = "yn#aq";
NEARDATA long yn_number = 0L;

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
NEARDATA const char sdir[] = "hykulnjb><";
NEARDATA const char ndir[] = "47896321><";	/* number pad mode */
NEARDATA const schar xdir[10] = { -1,-1, 0, 1, 1, 1, 0,-1, 0, 0 };
NEARDATA const schar ydir[10] = {  0,-1,-1,-1, 0, 1, 1, 1, 0, 0 };
NEARDATA const schar zdir[10]	      = {  0, 0, 0, 0, 0, 0, 0, 0, 1,-1 };

NEARDATA schar tbx = 0, tby = 0;	/* mthrowu: target */
NEARDATA int dig_effort = 0;	/* effort expended on current pos */
NEARDATA d_level dig_level = { 0, 0 };
NEARDATA coord dig_pos = DUMMY;
NEARDATA boolean dig_down = FALSE;

NEARDATA dungeon dungeons[MAXDUNGEON];	/* ini'ed by init_dungeon() */
NEARDATA s_level *sp_levchn;
NEARDATA int done_stopprint = 0;
NEARDATA int done_hup = 0;
NEARDATA stairway upstair = { 0, 0 }, dnstair = { 0, 0 };
NEARDATA stairway upladder = { 0, 0 }, dnladder = { 0, 0 };
NEARDATA stairway sstairs = { 0, 0 };
NEARDATA dest_area updest = { 0, 0, 0, 0, 0, 0, 0, 0 };
NEARDATA dest_area dndest = { 0, 0, 0, 0, 0, 0, 0, 0 };
NEARDATA coord inv_pos = { 0, 0 };

NEARDATA boolean in_mklev = FALSE;
NEARDATA boolean stoned = FALSE;	/* done to monsters hit by 'c' */
NEARDATA boolean unweapon = FALSE;
NEARDATA boolean mrg_to_wielded = FALSE;
			 /* weapon picked is merged with wielded one */

#ifdef KOPS
NEARDATA boolean allow_kops = TRUE;
#endif

NEARDATA coord bhitpos = DUMMY;
NEARDATA coord doors[DOORMAX] = {DUMMY};

NEARDATA struct mkroom rooms[(MAXNROFROOMS+1)*2] = {DUMMY};
NEARDATA struct mkroom* subrooms = &rooms[MAXNROFROOMS+1];
struct mkroom *upstairs_room, *dnstairs_room, *sstairs_room;

dlevel_t level;		/* level map */
struct trap *ftrap = (struct trap *)0;
NEARDATA struct monst youmonst = DUMMY;
NEARDATA struct flag flags = DUMMY;
NEARDATA struct you u = DUMMY;

NEARDATA struct obj *invent = (struct obj *)0, 
	*uwep = (struct obj *)0, *uarm = (struct obj *)0,
#ifdef TOURIST
	*uarmu = (struct obj *)0, /* under-wear, so to speak */
#endif
#ifdef POLYSELF
	 *uskin = (struct obj *)0, /* dragon armor, if a dragon */
#endif
	*uarmc = (struct obj *)0, *uarmh = (struct obj *)0, 
        *uarms = (struct obj *)0, *uarmg = (struct obj *)0,
        *uarmf = (struct obj *)0, *uamul = (struct obj *)0,
	*uright = (struct obj *)0,
        *uleft = (struct obj *)0,
        *ublindf = (struct obj *)0,
	*uchain = (struct obj *)0,
        *uball = (struct obj *)0;

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

NEARDATA struct spell spl_book[MAXSPELL + 1] = {DUMMY};

NEARDATA long moves = 1L, monstermoves = 1L;
	 /* These diverge when player is Fast */
NEARDATA long wailmsg = 0L;

/* objects that are moving to another dungeon level */
NEARDATA struct obj *migrating_objs = (struct obj *)0;
/* objects not yet paid for */
NEARDATA struct obj *billobjs = (struct obj *)0;

/* used to zero all elements of a struct obj */
NEARDATA struct obj zeroobj = DUMMY;

/* monster pronouns, index is return value of gender(mtmp) */
NEARDATA const char *he[3]  = { "he",  "she", "it" };
NEARDATA const char *him[3] = { "him", "her", "it" };
NEARDATA const char *his[3] = { "his", "her", "its" };

/* originally from dog.c */
NEARDATA char dogname[63] = DUMMY;
NEARDATA char catname[63] = DUMMY;
char preferred_pet;	/* '\0', 'c', 'd' */
/* monsters that went down/up together with @ */
NEARDATA struct monst *mydogs = (struct monst *)0;
/* monsters that are moving to another dungeon level */
NEARDATA struct monst *migrating_mons = (struct monst *)0;

NEARDATA struct c_color_names c_color_names = {
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
NEARDATA boolean vision_full_recalc = 0;
NEARDATA char	 **viz_array = 0;/* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
NEARDATA winid WIN_MESSAGE = WIN_ERR, WIN_STATUS = WIN_ERR;
NEARDATA winid WIN_MAP = WIN_ERR, WIN_INVEN = WIN_ERR;
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
