#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
/*	SCCS Id: @(#)lev_comp.c	3.1	93/05/15	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive in its non-standard locale.
 */

#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "hack.h"
#include "sp_lev.h"
#ifndef O_WRONLY
# include <fcntl.h>
#endif
#ifndef O_CREAT	/* some older BSD systems do not define O_CREAT in <fcntl.h> */
# include <sys/file.h>
#endif
#ifndef O_BINARY	/* used for micros, no-op for others */
# define O_BINARY 0
#endif

#ifdef MICRO
# define OMASK FCMASK
#else
# define OMASK 0644
#endif

#define MAX_REGISTERS	10
#define ERR		(-1)

#define New(type)		(type *) alloc(sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)

#ifdef MICRO
# undef exit
extern void FDECL(exit, (int));
#endif

extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);

extern char *FDECL(dup_string,(char *));
extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *, CHAR_P));
extern int FDECL(get_object_id, (char *));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *));
extern void NDECL(wallify_map);
extern boolean NDECL(check_subrooms);
extern void FDECL(check_coord, (int, int, const char *));
extern void NDECL(store_part);
extern void NDECL(store_room);
extern void FDECL(write_maze, (int, specialmaze *));
extern void FDECL(write_lev, (int, splev *));
extern void FDECL(free_rooms, (room **, int));

static struct reg {
	int x1, y1;
	int x2, y2;
}		current_region;

static struct coord {
	int x;
	int y;
}		current_coord, current_align;

static struct size {
	int height;
	int width;
}		current_size;

char tmpmessage[256];
altar *tmpaltar[256];
lad *tmplad[256];
stair *tmpstair[256];
digpos *tmpdig[256];
digpos *tmppass[32];
char *tmpmap[ROWNO];
region *tmpreg[256];
lev_region *tmplreg[32];
door *tmpdoor[256];
room_door *tmprdoor[256];
trap *tmptrap[256];
monster *tmpmonst[256];
object *tmpobj[256];
drawbridge *tmpdb[256];
walk *tmpwalk[256];
gold *tmpgold[256];
fountain *tmpfountain[256];
sink *tmpsink[256];
pool *tmppool[256];
engraving *tmpengraving[256];
mazepart *tmppart[10];
room *tmproom[MAXNROFROOMS*2];
corridor *tmpcor[256];

static specialmaze maze;
static splev special_lev;
static lev_init init_lev;

static char olist[MAX_REGISTERS], mlist[MAX_REGISTERS];
static struct coord plist[MAX_REGISTERS];

int n_olist = 0, n_mlist = 0, n_plist = 0;

unsigned int nlreg = 0, nreg = 0, ndoor = 0, ntrap = 0, nmons = 0, nobj = 0;
unsigned int ndb = 0, nwalk = 0, npart = 0, ndig = 0, nlad = 0, nstair = 0;
unsigned int naltar = 0, ncorridor = 0, nrooms = 0, ngold = 0, nengraving = 0;
unsigned int nfountain = 0, npool = 0, nsink = 0, npass = 0;

static unsigned long lev_flags = 0;

unsigned int max_x_map, max_y_map;

static xchar in_room;

extern int fatal_error;
extern int want_warnings;
extern const char *fname;

typedef union
{
	int	i;
	char*	map;
	struct {
		xchar room;
		xchar wall;
		xchar door;
	} corpos;
} YYSTYPE;
#define CHAR 257
#define INTEGER 258
#define BOOLEAN 259
#define MESSAGE_ID 260
#define MAZE_ID 261
#define LEVEL_ID 262
#define LEV_INIT_ID 263
#define GEOMETRY_ID 264
#define NOMAP_ID 265
#define OBJECT_ID 266
#define MONSTER_ID 267
#define TRAP_ID 268
#define DOOR_ID 269
#define DRAWBRIDGE_ID 270
#define MAZEWALK_ID 271
#define WALLIFY_ID 272
#define REGION_ID 273
#define FILLING 274
#define RANDOM_OBJECTS_ID 275
#define RANDOM_MONSTERS_ID 276
#define RANDOM_PLACES_ID 277
#define ALTAR_ID 278
#define LADDER_ID 279
#define STAIR_ID 280
#define NON_DIGGABLE_ID 281
#define NON_PASSWALL_ID 282
#define ROOM_ID 283
#define PORTAL_ID 284
#define TELEPRT_ID 285
#define BRANCH_ID 286
#define LEV 287
#define CHANCE_ID 288
#define CORRIDOR_ID 289
#define GOLD_ID 290
#define ENGRAVING_ID 291
#define FOUNTAIN_ID 292
#define POOL_ID 293
#define SINK_ID 294
#define NONE 295
#define RAND_CORRIDOR_ID 296
#define DOOR_STATE 297
#define LIGHT_STATE 298
#define CURSE_TYPE 299
#define ENGRAVING_TYPE 300
#define DIRECTION 301
#define RANDOM_TYPE 302
#define O_REGISTER 303
#define M_REGISTER 304
#define P_REGISTER 305
#define A_REGISTER 306
#define ALIGNMENT 307
#define LEFT_OR_RIGHT 308
#define CENTER 309
#define TOP_OR_BOT 310
#define ALTAR_TYPE 311
#define UP_OR_DOWN 312
#define SUBROOM_ID 313
#define NAME_ID 314
#define FLAGS_ID 315
#define FLAG_TYPE 316
#define MON_ATTITUDE 317
#define MON_ALERTNESS 318
#define MON_APPEARANCE 319
#define STRING 320
#define MAP_ID 321
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   38,   38,   39,   39,   40,   41,   33,   24,
   24,   14,   14,   20,   20,   21,   21,   42,   42,   47,
   44,   44,   48,   48,   45,   45,   51,   51,   46,   46,
   53,   54,   54,   55,   55,   37,   52,   52,   58,   56,
   10,   10,   61,   61,   59,   59,   62,   62,   60,   60,
   57,   57,   63,   63,   63,   63,   63,   63,   63,   63,
   63,   63,   63,   63,   63,   64,   65,   66,   15,   15,
   13,   13,   12,   12,   32,   11,   11,   43,   43,   77,
   78,   78,   81,    1,    1,    2,    2,   79,   79,   82,
   82,   82,   49,   49,   50,   50,   83,   85,   83,   80,
   80,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,   86,   86,   86,   86,   86,   86,   86,   86,   86,
   86,  101,   67,  100,  100,  102,  102,  102,  102,  102,
  104,   68,  103,  103,  103,   16,   16,   17,   17,   87,
   69,   88,   94,   95,   96,   76,  105,   90,  106,   91,
  107,  109,   92,  110,   93,  108,  108,   23,   23,   71,
   72,   73,   97,   98,   89,   70,   74,   75,   26,   26,
   26,   29,   29,   29,   34,   34,   35,   35,    3,    3,
    4,    4,   22,   22,   22,   99,   99,   99,    5,    5,
    6,    6,    7,    7,    7,    8,    8,  113,   30,   27,
    9,   84,   25,   28,   31,   36,   36,   18,   18,   19,
   19,  112,  111,
};
short yylen[] = {                                         2,
    0,    1,    1,    2,    1,    1,    5,    7,    3,    0,
   13,    1,    1,    0,    3,    3,    1,    0,    2,    3,
    0,    2,    3,    3,    0,    1,    1,    2,    1,    1,
    1,    0,    2,    5,    5,    7,    2,    2,   12,   12,
    0,    2,    5,    1,    5,    1,    5,    1,    5,    1,
    0,    2,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    3,    3,    9,    1,    1,
    1,    1,    1,    1,    5,    1,    1,    1,    2,    3,
    1,    2,    5,    1,    1,    1,    1,    0,    2,    3,
    3,    3,    1,    3,    1,    3,    1,    0,    4,    0,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    0,    9,    0,    2,    2,    2,    2,    2,    3,
    0,    9,    0,    4,    6,    1,    1,    1,    1,    5,
    5,    7,    5,    1,    5,    5,    0,    8,    0,    8,
    0,    0,    8,    0,    6,    0,    2,    1,   10,    3,
    3,    3,    3,    3,    8,    7,    5,    7,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    0,    2,    4,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    4,    4,    4,
    4,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    5,    9,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    2,    0,    5,    6,    0,
    0,    0,    0,    0,    4,  205,    0,    9,    0,    0,
    0,    0,    0,    0,   15,    0,    0,    0,    0,   21,
   76,   77,   75,    0,    0,    0,    0,   81,    7,    0,
   88,    0,   19,    0,   16,    0,   20,    0,   79,    0,
   82,    0,    0,    0,    0,    0,   22,   26,    0,   51,
   51,    0,   84,   85,    0,    0,    0,    0,    0,   89,
    0,    0,    0,    0,   31,    8,   29,    0,   28,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  144,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  102,  103,  105,  112,  113,
  118,  119,  117,  101,  104,  106,  107,  108,  109,  110,
  111,  114,  115,  116,  120,  121,  204,    0,   23,  203,
    0,   24,  182,    0,  181,    0,    0,   33,    0,    0,
    0,    0,    0,    0,   52,   53,   54,   55,   56,   57,
   58,   59,   60,   61,   62,   63,   64,   65,    0,   87,
   86,   83,   90,   92,    0,   91,    0,  202,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  173,    0,  172,    0,  174,  170,    0,  169,    0,  171,
  180,    0,  179,  189,  190,    0,  188,    0,    0,  186,
  187,    0,    0,    0,    0,    0,    0,    0,  147,    0,
  158,  163,  164,  149,  151,  154,  208,  209,    0,    0,
  160,   94,   96,  191,  192,    0,    0,    0,    0,   69,
   70,    0,   67,  162,  161,   66,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   99,    0,
  178,  177,    0,    0,  176,  175,    0,  141,  140,    0,
    0,  143,    0,    0,  195,    0,  193,    0,  194,  145,
    0,    0,    0,  146,    0,    0,    0,  167,  210,  211,
    0,   44,    0,    0,   46,    0,    0,    0,   35,   34,
    0,    0,  212,  199,    0,  200,    0,  198,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  152,  155,    0,
    0,    0,    0,    0,    0,    0,    0,  131,  122,  142,
    0,    0,    0,  197,  196,  166,    0,    0,    0,    0,
  168,    0,   48,    0,    0,    0,   50,    0,    0,    0,
   71,   72,    0,   12,   13,   11,    0,  124,    0,    0,
  165,  201,    0,  148,  150,    0,  153,    0,    0,    0,
    0,    0,    0,   73,   74,    0,    0,    0,  132,    0,
    0,    0,    0,  157,   43,    0,    0,   45,    0,    0,
   36,   68,  137,  136,    0,    0,    0,  125,    0,    0,
    0,    0,    0,   40,    0,   39,    0,    0,  127,  128,
    0,  129,  126,  213,  185,    0,   47,   42,   49,  138,
  139,  134,    0,  130,  159,    0,  207,  206,  135,
};
short yydgoto[] = {                                       3,
   65,  162,  212,  134,  216,  246,  308,  366,  309,  434,
   33,  406,  383,  386,  252,  426,  452,  239,  321,   13,
   25,  391,  229,   21,  131,  209,  210,  128,  204,  205,
  135,    4,    5,  297,  293,  459,  249,    6,    7,    8,
    9,   28,   39,   44,   56,   76,   29,   57,  129,  132,
   58,   59,   77,   78,  138,   60,   80,   61,  327,  379,
  324,  375,  145,  146,  147,  148,  149,  150,  151,  152,
  153,  154,  155,  156,  157,  158,   40,   41,   50,   69,
   42,   70,  166,  167,  200,  114,  115,  116,  117,  118,
  119,  120,  121,  122,  123,  124,  125,  126,  230,  410,
  388,  428,  409,  387,  275,  277,  278,  397,  370,  279,
  231,  220,  221,
};
short yysindex[] = {                                   -122,
  -26,    4,    0, -231, -231,    0, -122,    0,    0, -267,
 -267,   37, -156, -156,    0,    0,   70,    0, -199,   72,
 -103, -103, -226,  107,    0,  -99,  101, -123, -103,    0,
    0,    0,    0, -199,  116, -158,  105,    0,    0, -123,
    0, -157,    0, -234,    0,  -92,    0, -164,    0, -139,
    0,  108,  109,  110,  111, -126,    0,    0, -262,    0,
    0,  127,    0,    0,  128,  118,  119,  120,  -78,    0,
  -83,  -74, -272, -272,    0,    0,    0, -108,    0, -223,
 -223,  -79, -161,  -83,  -74,  144,  138,  139,  140,  141,
  147,  151,    0,  152,  153,  157,  158,  159,  160,  161,
  162,  165,  166,  167,  168,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  142,    0,    0,
  183,    0,    0,  184,    0,  185,  173,    0,  174,  175,
  176,  177,  178,  179,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  194,    0,
    0,    0,    0,    0,  -73,    0,    0,    0, -229, -241,
 -242, -201,   79,   79,  199,   79,   79,   95,  199,  199,
  -35,  -35,  -35, -222,   79,   79,  -83,  -74, -276, -276,
  201, -230,   79,  -16,   79,   79, -267,  -15,  202,  203,
    0,  154,    0,  204,    0,    0,  163,    0,  205,    0,
    0,  211,    0,    0,    0,  212,    0,  169,  213,    0,
    0,  214,    1,  217,  218,  258,  294,   84,    0,  303,
    0,    0,    0,    0,    0,    0,    0,    0,  305,  312,
    0,    0,    0,    0,    0,  314,  319,  125,  343,    0,
    0,  344,    0,    0,    0,    0,  348,  136,  144,  137,
 -216,  145, -209,   79,   79,  148,  -58,   98,  357, -276,
 -248,  124,  180,  393,  396,  129,  398,  401,  426,   79,
 -177,  -37,  -13,  431,  -36, -201, -276,  435,    0,  384,
    0,    0,  436,  394,    0,    0,  444,    0,    0,  405,
  445,    0,  236,  455,    0,  388,    0,  463,    0,    0,
  464,  251,  -35,    0,  -35,  -35,  -35,    0,    0,    0,
  470,    0,  260,  476,    0,  263,  478,  223,    0,    0,
  481,  483,    0,    0,   79,    0,   79,    0, -201,  484,
 -272,  272, -255,  273,   65,  488,  491,    0,    0, -267,
  498,   -5,  500,   24,  501, -155, -219,    0,    0,    0,
  289,  504,  456,    0,    0,    0,  506,  242, -267,  511,
    0,  298,    0, -164,  513,  300,    0,  304,  520, -221,
    0,    0,  522,    0,    0,    0,  523,    0,  524,  295,
    0,    0,  313,    0,    0,  261,    0,  529,  532,   24,
  536,  534, -267,    0,    0,  538, -221, -212,    0,  537,
  322,  539,  540,    0,    0, -161,  541,    0,  328,  541,
    0,    0,    0,    0,  543,  546, -186,    0,  550,  333,
  335,  553,  337,    0,  556,    0, -220, -220,    0,    0,
 -267,    0,    0,    0,    0,  557,    0,    0,    0,    0,
    0,    0,  555,    0,    0, -270,    0,    0,    0,
};
short yyrindex[] = {                                    600,
    0,    0,    0, -166,  230,    0,  601,    0,    0,    0,
    0,    0, -138,  240,    0,    0,    0,    0,    0,    0,
 -109,  299,    0,  221,    0,    0,    0,    0,  276,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   17,
    0,    0,    0,   52,    0,    0,    0,    0,    0,  386,
    0,    0,    0,    0,    0,   75,    0,    0,  100,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  115,    0,
    0,    0,    0,    0,    0,    0,    0,  113,    0,   89,
  250,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  143,    0,    0,
  182,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  354,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  430,    0,    0,    0,    0,    0,    0,    0,  469,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    2,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   39,
    0,  503,    0,    0,    0,    0,   77,    0,    0,   77,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
  228,  187,    0,  -64, -273, -182,  181,    0,    0,  186,
    0,  197,    0,    0,    0,    0,  171,    0,    0,  602,
  571,    0, -163,  596,  441,    0,    0,  448,    0,    0,
  -10,    0,    0,    0,    0,    0,  329,  606,    0,    0,
    0,   63,  588,    0,    0,    0,    0,    0,  -77,  -76,
  578,    0,    0,    0,    0,    0,  580,    0,    0,  243,
    0,    0,    0,    0,    0,    0,  573,  591,  592,  593,
  594,    0,    0,  604,  605,  610,    0,    0,    0,    0,
    0,    0,  390,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -162,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -51,  -80,    0,
};
#define YYTABLESIZE 795
short yytable[] = {                                      17,
   18,  133,  323,  248,  223,  168,  163,  247,  164,  136,
  219,  222,  331,  225,  226,  130,   78,  234,  235,  236,
   54,  244,  240,  241,  457,  245,  326,  127,  250,  133,
   31,   10,  254,  255,  374,  237,  404,  450,  123,  384,
   52,   53,   87,   88,   89,  139,  364,   16,   54,  458,
   55,   25,   16,  305,   95,  365,  140,  306,  307,  211,
  206,   11,  207,  378,  141,  360,  103,  104,  105,  142,
  143,  251,  201,  202,   32,   32,   41,   16,   55,  238,
  405,  451,  385,   12,   30,  291,  423,  304,   37,  424,
  144,   43,  295,   14,   19,  214,   14,   14,   14,   27,
  215,  298,  299,   16,  332,  333,   20,  425,  361,  242,
   16,  243,   30,   23,   80,  305,   24,  318,  165,  306,
  307,   10,  319,  224,  320,   10,   10,  232,  233,   26,
  439,  440,  441,   16,  228,   66,   67,   68,    1,    2,
   37,   38,   93,   63,   64,  381,  382,  160,  161,  346,
   34,  347,  348,  349,   18,   18,   27,   35,   36,   46,
  213,   47,   48,   51,   62,   71,   72,   73,   74,   75,
   82,   83,  358,  127,  359,   84,   85,   86,  168,  159,
  137,   95,  130,  165,  199,  187,  256,   87,   88,   89,
   90,   91,   92,   93,   94,  169,  170,  171,  172,   95,
   96,   97,   98,   99,  173,  100,  101,  102,  174,  175,
  176,  103,  104,  105,  177,  178,  179,  180,  181,  182,
   17,  329,  183,  184,  185,  186,  188,  189,  190,   14,
  191,  192,  193,  194,  195,  196,  197,  198,  223,   10,
  248,  253,  301,  257,  260,  258,  259,  261,  263,   38,
  292,  227,  296,  262,  264,  265,  267,  268,  269,  266,
  270,  271,  133,  133,  322,  133,  133,  133,  133,  133,
  133,  133,  133,  133,  133,   18,  362,   78,   78,  133,
  133,  133,  133,  133,  133,  133,  133,  133,  325,  133,
  133,  133,  133,  133,  133,  133,  373,  133,   18,  123,
  123,  272,  123,  123,  123,  123,  123,  123,  123,  123,
  123,  123,   25,   25,  133,  133,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  377,  123,  123,  123,  123,
  123,  123,  123,  273,  123,   32,   32,   41,   41,  371,
   25,  274,   41,   41,   41,   41,  276,   25,  280,   37,
   37,  123,  123,   97,   41,  281,   41,  282,  395,   41,
   27,   27,  283,   32,   41,   41,   41,   41,   41,   41,
   41,   37,   41,   30,   30,   80,   80,   37,   80,   80,
  217,  227,  284,  218,   37,  100,  285,  286,   27,   41,
   41,  287,  420,  288,  290,   27,  217,   98,  302,  218,
  303,   37,  294,   93,   93,  300,   93,   93,   93,   93,
   93,   93,   93,   93,   93,   93,  443,   93,   93,   93,
   93,   93,   93,   93,   93,   93,   93,   93,   93,  183,
  454,   93,   93,   93,   93,  310,  312,  311,   93,  313,
  314,  315,   95,   95,  316,   95,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   93,   95,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,  156,  317,
   95,   95,   95,   95,  328,  333,  334,   95,  342,  335,
   17,   17,   17,   17,   17,   17,  336,  337,  339,   14,
   14,   14,   14,  340,   95,   17,   17,  338,  341,   10,
   10,   10,  184,   17,   14,   14,  343,  344,  345,   17,
   38,   38,   14,  350,   10,   10,   17,  351,   14,  352,
  353,  354,   10,  355,  356,   14,  357,  361,   10,  363,
  367,  368,   38,   17,  369,   10,   18,   18,   38,   18,
   18,  372,   14,  376,  380,   38,  389,  390,  392,  393,
   18,   18,   10,  394,  396,  398,  400,  401,   18,   18,
   18,  402,   38,  403,   18,  407,  408,  411,  412,  415,
  413,   18,  414,   18,   18,  416,  418,  419,  421,  429,
  427,   18,  430,  431,  433,  435,  437,   18,   18,  438,
  444,  445,  446,  447,   18,  448,  449,  455,  456,    1,
    3,  399,  432,  422,   45,  436,   14,  442,  453,   22,
  208,   18,   15,  330,   97,   97,  203,   97,   97,   97,
   97,   97,   97,   97,   97,   97,   97,   49,   97,   97,
   97,   97,   97,   97,   97,   97,   79,   97,   97,   97,
   81,  106,  417,   97,   97,   97,  100,  100,  289,  100,
  100,  100,  100,  100,  100,  100,  100,  100,  100,  107,
  108,  109,  110,  100,  100,  100,  100,  100,    0,  100,
  100,  100,  111,  112,    0,  100,  100,  100,  113,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  183,  183,    0,  183,  183,  183,  183,  183,  183,  183,
  183,  183,  183,    0,    0,    0,    0,  183,  183,  183,
  183,  183,    0,  183,  183,  183,    0,    0,    0,  183,
  183,  183,    0,    0,    0,    0,    0,    0,    0,  156,
  156,    0,  156,  156,  156,  156,  156,  156,  156,  156,
  156,  156,    0,    0,    0,    0,  156,  156,  156,  156,
  156,    0,  156,  156,  156,    0,    0,    0,  156,  156,
  156,    0,    0,  184,  184,    0,  184,  184,  184,  184,
  184,  184,  184,  184,  184,  184,    0,    0,    0,    0,
  184,  184,  184,  184,  184,    0,  184,  184,  184,    0,
    0,    0,  184,  184,  184,
};
short yycheck[] = {                                      10,
   11,    0,   40,   40,   40,   86,   84,  190,   85,   74,
  173,  174,  286,  176,  177,  257,    0,  181,  182,  183,
  283,  298,  185,  186,  295,  302,   40,  257,  259,  302,
  257,   58,  195,  196,   40,  258,  258,  258,    0,  259,
  275,  276,  266,  267,  268,  269,  302,  320,  283,  320,
  313,    0,  320,  302,  278,  311,  280,  306,  307,  302,
  302,   58,  304,   40,  288,  339,  290,  291,  292,  293,
  294,  302,  302,  303,    0,  302,    0,  320,  313,  302,
  302,  302,  302,  315,   22,  302,  299,  270,    0,  302,
  314,   29,  302,  260,   58,  297,  263,  264,  265,    0,
  302,  264,  265,  320,  287,   41,  263,  320,   44,  187,
  320,  188,    0,   44,    0,  302,  316,  280,   40,  306,
  307,  260,  300,  175,  302,  264,  265,  179,  180,   58,
  317,  318,  319,  320,   40,  275,  276,  277,  261,  262,
  264,  265,    0,  308,  309,  301,  302,  309,  310,  313,
   44,  315,  316,  317,  264,  265,  260,  257,   58,   44,
  171,  320,   58,  321,  257,   58,   58,   58,   58,  296,
   44,   44,  335,  257,  337,   58,   58,   58,  259,  259,
  289,    0,  257,   40,  258,   44,  197,  266,  267,  268,
  269,  270,  271,  272,  273,   58,   58,   58,   58,  278,
  279,  280,  281,  282,   58,  284,  285,  286,   58,   58,
   58,  290,  291,  292,   58,   58,   58,   58,   58,   58,
    0,  258,   58,   58,   58,   58,   44,   44,   44,    0,
   58,   58,   58,   58,   58,   58,   58,   44,   40,    0,
   40,  258,  301,  259,   91,   44,   44,   44,   44,    0,
  261,  287,  263,   91,   44,   44,   44,   44,  258,   91,
   44,   44,  261,  262,  302,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,    0,  341,  261,  262,  278,
  279,  280,  281,  282,  283,  284,  285,  286,  302,  288,
  289,  290,  291,  292,  293,  294,  302,  296,    0,  261,
  262,   44,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  261,  262,  313,  314,  278,  279,  280,  281,
  282,  283,  284,  285,  286,  302,  288,  289,  290,  291,
  292,  293,  294,   40,  296,  261,  262,  261,  262,  350,
  289,  258,  266,  267,  268,  269,   44,  296,   44,  261,
  262,  313,  314,    0,  278,   44,  280,   44,  369,  283,
  261,  262,   44,  289,  288,  289,  290,  291,  292,  293,
  294,  283,  296,  261,  262,  261,  262,  289,  264,  265,
  302,  287,  258,  305,  296,    0,   44,   44,  289,  313,
  314,   44,  403,  258,  258,  296,  302,   44,  301,  305,
   44,  313,  258,  261,  262,  258,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  427,  275,  276,  277,
  278,  279,  280,  281,  282,  283,  284,  285,  286,    0,
  441,  289,  290,  291,  292,  312,   44,  258,  296,   44,
  312,   44,  261,  262,   44,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  313,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  285,  286,    0,   44,
  289,  290,  291,  292,   44,   41,   93,  296,   91,   44,
  260,  261,  262,  263,  264,  265,   93,   44,   44,  260,
  261,  262,  263,  258,  313,  275,  276,   93,   44,  260,
  261,  262,    0,  283,  275,  276,   44,   44,  258,  289,
  261,  262,  283,   44,  275,  276,  296,  258,  289,   44,
  258,   44,  283,  301,   44,  296,   44,   44,  289,  258,
  258,   44,  283,  313,   44,  296,  261,  262,  289,  264,
  265,   44,  313,   44,   44,  296,  258,   44,   93,   44,
  275,  276,  313,  312,   44,  258,   44,  258,  283,  261,
  262,  258,  313,   44,  289,   44,   44,   44,  274,   41,
  258,  296,  312,  275,  276,   44,   41,   44,   41,  258,
   44,  283,   44,   44,   44,  258,   44,  289,  313,   44,
   41,  259,  258,   41,  296,  259,   41,   41,   44,    0,
    0,  374,  416,  407,   34,  420,    5,  427,  438,   14,
  170,  313,    7,  285,  261,  262,  169,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,   40,  275,  276,
  277,  278,  279,  280,  281,  282,   59,  284,  285,  286,
   61,   69,  400,  290,  291,  292,  261,  262,  259,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,   69,
   69,   69,   69,  278,  279,  280,  281,  282,   -1,  284,
  285,  286,   69,   69,   -1,  290,  291,  292,   69,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  261,  262,   -1,  264,  265,  266,  267,  268,  269,  270,
  271,  272,  273,   -1,   -1,   -1,   -1,  278,  279,  280,
  281,  282,   -1,  284,  285,  286,   -1,   -1,   -1,  290,
  291,  292,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  261,
  262,   -1,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,   -1,   -1,   -1,   -1,  278,  279,  280,  281,
  282,   -1,  284,  285,  286,   -1,   -1,   -1,  290,  291,
  292,   -1,   -1,  261,  262,   -1,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,   -1,   -1,   -1,   -1,
  278,  279,  280,  281,  282,   -1,  284,  285,  286,   -1,
   -1,   -1,  290,  291,  292,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 321
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CHAR",
"INTEGER","BOOLEAN","MESSAGE_ID","MAZE_ID","LEVEL_ID","LEV_INIT_ID",
"GEOMETRY_ID","NOMAP_ID","OBJECT_ID","MONSTER_ID","TRAP_ID","DOOR_ID",
"DRAWBRIDGE_ID","MAZEWALK_ID","WALLIFY_ID","REGION_ID","FILLING",
"RANDOM_OBJECTS_ID","RANDOM_MONSTERS_ID","RANDOM_PLACES_ID","ALTAR_ID",
"LADDER_ID","STAIR_ID","NON_DIGGABLE_ID","NON_PASSWALL_ID","ROOM_ID",
"PORTAL_ID","TELEPRT_ID","BRANCH_ID","LEV","CHANCE_ID","CORRIDOR_ID","GOLD_ID",
"ENGRAVING_ID","FOUNTAIN_ID","POOL_ID","SINK_ID","NONE","RAND_CORRIDOR_ID",
"DOOR_STATE","LIGHT_STATE","CURSE_TYPE","ENGRAVING_TYPE","DIRECTION",
"RANDOM_TYPE","O_REGISTER","M_REGISTER","P_REGISTER","A_REGISTER","ALIGNMENT",
"LEFT_OR_RIGHT","CENTER","TOP_OR_BOT","ALTAR_TYPE","UP_OR_DOWN","SUBROOM_ID",
"NAME_ID","FLAGS_ID","FLAG_TYPE","MON_ATTITUDE","MON_ALERTNESS",
"MON_APPEARANCE","STRING","MAP_ID",
};
char *yyrule[] = {
"$accept : file",
"file :",
"file : levels",
"levels : level",
"levels : level levels",
"level : maze_level",
"level : room_level",
"maze_level : maze_def flags lev_init messages regions",
"room_level : level_def flags lev_init messages rreg_init rooms corridors_def",
"level_def : LEVEL_ID ':' string",
"lev_init :",
"lev_init : LEV_INIT_ID ':' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled",
"walled : BOOLEAN",
"walled : RANDOM_TYPE",
"flags :",
"flags : FLAGS_ID ':' flag_list",
"flag_list : FLAG_TYPE ',' flag_list",
"flag_list : FLAG_TYPE",
"messages :",
"messages : message messages",
"message : MESSAGE_ID ':' STRING",
"rreg_init :",
"rreg_init : rreg_init init_rreg",
"init_rreg : RANDOM_OBJECTS_ID ':' object_list",
"init_rreg : RANDOM_MONSTERS_ID ':' monster_list",
"rooms :",
"rooms : roomlist",
"roomlist : aroom",
"roomlist : aroom roomlist",
"corridors_def : random_corridors",
"corridors_def : corridors",
"random_corridors : RAND_CORRIDOR_ID",
"corridors :",
"corridors : corridors corridor",
"corridor : CORRIDOR_ID ':' corr_spec ',' corr_spec",
"corridor : CORRIDOR_ID ':' corr_spec ',' INTEGER",
"corr_spec : '(' INTEGER ',' DIRECTION ',' door_pos ')'",
"aroom : room_def room_details",
"aroom : subroom_def room_details",
"subroom_def : SUBROOM_ID ':' room_type ',' light_state ',' subroom_pos ',' room_size ',' string roomfill",
"room_def : ROOM_ID ':' room_type ',' light_state ',' room_pos ',' room_align ',' room_size roomfill",
"roomfill :",
"roomfill : ',' BOOLEAN",
"room_pos : '(' INTEGER ',' INTEGER ')'",
"room_pos : RANDOM_TYPE",
"subroom_pos : '(' INTEGER ',' INTEGER ')'",
"subroom_pos : RANDOM_TYPE",
"room_align : '(' h_justif ',' v_justif ')'",
"room_align : RANDOM_TYPE",
"room_size : '(' INTEGER ',' INTEGER ')'",
"room_size : RANDOM_TYPE",
"room_details :",
"room_details : room_details room_detail",
"room_detail : room_name",
"room_detail : room_chance",
"room_detail : room_door",
"room_detail : monster_detail",
"room_detail : object_detail",
"room_detail : trap_detail",
"room_detail : altar_detail",
"room_detail : fountain_detail",
"room_detail : sink_detail",
"room_detail : pool_detail",
"room_detail : gold_detail",
"room_detail : engraving_detail",
"room_detail : stair_detail",
"room_name : NAME_ID ':' string",
"room_chance : CHANCE_ID ':' INTEGER",
"room_door : DOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos",
"secret : BOOLEAN",
"secret : RANDOM_TYPE",
"door_wall : DIRECTION",
"door_wall : RANDOM_TYPE",
"door_pos : INTEGER",
"door_pos : RANDOM_TYPE",
"maze_def : MAZE_ID ':' string ',' filling",
"filling : CHAR",
"filling : RANDOM_TYPE",
"regions : aregion",
"regions : aregion regions",
"aregion : map_definition reg_init map_details",
"map_definition : NOMAP_ID",
"map_definition : map_geometry MAP_ID",
"map_geometry : GEOMETRY_ID ':' h_justif ',' v_justif",
"h_justif : LEFT_OR_RIGHT",
"h_justif : CENTER",
"v_justif : TOP_OR_BOT",
"v_justif : CENTER",
"reg_init :",
"reg_init : reg_init init_reg",
"init_reg : RANDOM_OBJECTS_ID ':' object_list",
"init_reg : RANDOM_PLACES_ID ':' place_list",
"init_reg : RANDOM_MONSTERS_ID ':' monster_list",
"object_list : object",
"object_list : object ',' object_list",
"monster_list : monster",
"monster_list : monster ',' monster_list",
"place_list : place",
"$$1 :",
"place_list : place $$1 ',' place_list",
"map_details :",
"map_details : map_details map_detail",
"map_detail : monster_detail",
"map_detail : object_detail",
"map_detail : door_detail",
"map_detail : trap_detail",
"map_detail : drawbridge_detail",
"map_detail : region_detail",
"map_detail : stair_region",
"map_detail : portal_region",
"map_detail : teleprt_region",
"map_detail : branch_region",
"map_detail : altar_detail",
"map_detail : fountain_detail",
"map_detail : mazewalk_detail",
"map_detail : wallify_detail",
"map_detail : ladder_detail",
"map_detail : stair_detail",
"map_detail : gold_detail",
"map_detail : engraving_detail",
"map_detail : diggable_detail",
"map_detail : passwall_detail",
"$$2 :",
"monster_detail : MONSTER_ID ':' monster_c ',' m_name ',' coordinate $$2 monster_infos",
"monster_infos :",
"monster_infos : monster_infos monster_info",
"monster_info : ',' string",
"monster_info : ',' MON_ATTITUDE",
"monster_info : ',' MON_ALERTNESS",
"monster_info : ',' alignment",
"monster_info : ',' MON_APPEARANCE string",
"$$3 :",
"object_detail : OBJECT_ID ':' object_c ',' o_name ',' coordinate $$3 object_infos",
"object_infos :",
"object_infos : ',' STRING ',' enchantment",
"object_infos : ',' curse_state ',' enchantment ',' art_name",
"curse_state : RANDOM_TYPE",
"curse_state : CURSE_TYPE",
"enchantment : INTEGER",
"enchantment : RANDOM_TYPE",
"door_detail : DOOR_ID ':' door_state ',' coordinate",
"trap_detail : TRAP_ID ':' trap_name ',' coordinate",
"drawbridge_detail : DRAWBRIDGE_ID ':' coordinate ',' DIRECTION ',' door_state",
"mazewalk_detail : MAZEWALK_ID ':' coordinate ',' DIRECTION",
"wallify_detail : WALLIFY_ID",
"ladder_detail : LADDER_ID ':' coordinate ',' UP_OR_DOWN",
"stair_detail : STAIR_ID ':' coordinate ',' UP_OR_DOWN",
"$$4 :",
"stair_region : STAIR_ID ':' lev_region $$4 ',' lev_region ',' UP_OR_DOWN",
"$$5 :",
"portal_region : PORTAL_ID ':' lev_region $$5 ',' lev_region ',' string",
"$$6 :",
"$$7 :",
"teleprt_region : TELEPRT_ID ':' lev_region $$6 ',' lev_region $$7 teleprt_detail",
"$$8 :",
"branch_region : BRANCH_ID ':' lev_region $$8 ',' lev_region",
"teleprt_detail :",
"teleprt_detail : ',' UP_OR_DOWN",
"lev_region : region",
"lev_region : LEV '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'",
"fountain_detail : FOUNTAIN_ID ':' coordinate",
"sink_detail : SINK_ID ':' coordinate",
"pool_detail : POOL_ID ':' coordinate",
"diggable_detail : NON_DIGGABLE_ID ':' region",
"passwall_detail : NON_PASSWALL_ID ':' region",
"region_detail : REGION_ID ':' region ',' light_state ',' room_type prefilled",
"altar_detail : ALTAR_ID ':' coordinate ',' alignment ',' altar_type",
"gold_detail : GOLD_ID ':' amount ',' coordinate",
"engraving_detail : ENGRAVING_ID ':' coordinate ',' engraving_type ',' string",
"monster_c : monster",
"monster_c : RANDOM_TYPE",
"monster_c : m_register",
"object_c : object",
"object_c : RANDOM_TYPE",
"object_c : o_register",
"m_name : string",
"m_name : RANDOM_TYPE",
"o_name : string",
"o_name : RANDOM_TYPE",
"trap_name : string",
"trap_name : RANDOM_TYPE",
"room_type : string",
"room_type : RANDOM_TYPE",
"prefilled :",
"prefilled : ',' FILLING",
"prefilled : ',' FILLING ',' BOOLEAN",
"coordinate : coord",
"coordinate : p_register",
"coordinate : RANDOM_TYPE",
"door_state : DOOR_STATE",
"door_state : RANDOM_TYPE",
"light_state : LIGHT_STATE",
"light_state : RANDOM_TYPE",
"alignment : ALIGNMENT",
"alignment : a_register",
"alignment : RANDOM_TYPE",
"altar_type : ALTAR_TYPE",
"altar_type : RANDOM_TYPE",
"p_register : P_REGISTER '[' INTEGER ']'",
"o_register : O_REGISTER '[' INTEGER ']'",
"m_register : M_REGISTER '[' INTEGER ']'",
"a_register : A_REGISTER '[' INTEGER ']'",
"place : coord",
"monster : CHAR",
"object : CHAR",
"string : STRING",
"art_name : STRING",
"art_name : NONE",
"amount : INTEGER",
"amount : RANDOM_TYPE",
"engraving_type : ENGRAVING_TYPE",
"engraving_type : RANDOM_TYPE",
"coord : '(' INTEGER ',' INTEGER ')'",
"region : '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) != 0 && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) != 0 && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) != 0 && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 7:
{
			  int fout, i;

			if (fatal_error > 0) {
				fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				char lbuf[20];
				Strcpy(lbuf, yyvsp[-4].map);
				Strcat(lbuf, LEV_EXT);
#ifdef THINK_C
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY);
#else
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY, OMASK);
#endif
				if (fout < 0) {
					yyerror("Can't open output file!!");
					exit(1);
				}
				maze.flags = yyvsp[-3].i;
				memcpy(&(maze.init_lev), &(init_lev),
				       sizeof(lev_init));
				maze.numpart = npart;
				maze.parts = NewTab(mazepart, npart);
				for(i=0;i<npart;i++)
				    maze.parts[i] = tmppart[i];
				write_maze(fout, &maze);
				(void) close(fout);
				npart = 0;
			}
		  }
break;
case 8:
{
			int fout, i;

			if (fatal_error > 0) {
			    fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				char lbuf[20];
				Strcpy(lbuf, yyvsp[-6].map);
				Strcat(lbuf, LEV_EXT);
#ifdef THINK_C
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY);
#else
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY, OMASK);
#endif
				if (fout < 0) {
					yyerror("Can't open output file!!");
					exit(1);
				}
				special_lev.flags = yyvsp[-5].i;
				memcpy(&(special_lev.init_lev), &(init_lev),
				       sizeof(lev_init));
				special_lev.nroom = nrooms;
				special_lev.rooms = NewTab(room, nrooms);
				for(i=0; i<nrooms; i++)
				    special_lev.rooms[i] = tmproom[i];
				special_lev.ncorr = ncorridor;
				special_lev.corrs = NewTab(corridor, ncorridor);
				for(i=0; i<ncorridor; i++)
				    special_lev.corrs[i] = tmpcor[i];
				if (check_subrooms())
				    write_lev(fout, &special_lev);
				free_rooms(special_lev.rooms,special_lev.nroom);
				nrooms = 0;
				ncorridor = 0;
				(void) close(fout);
			}
		  }
break;
case 9:
{
			if (index(yyvsp[0].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if (strlen(yyvsp[0].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[0].map;
			special_lev.nrobjects = 0;
			special_lev.nrmonst = 0;
		  }
break;
case 10:
{
			init_lev.init_present = FALSE;
			yyval.i = 0;
		  }
break;
case 11:
{
			init_lev.init_present = TRUE;
			if((init_lev.fg = what_map_char(yyvsp[-10].i)) == INVALID_TYPE)
			    yyerror("Invalid foreground type.");
			if((init_lev.bg = what_map_char(yyvsp[-8].i)) == INVALID_TYPE)
			    yyerror("Invalid background type.");
			init_lev.smoothed = yyvsp[-6].i;
			init_lev.joined = yyvsp[-4].i;
			init_lev.lit = yyvsp[-2].i;
			init_lev.walled = yyvsp[0].i;
			yyval.i = 1;
		  }
break;
case 14:
{
			yyval.i = 0;
		  }
break;
case 15:
{
			yyval.i = lev_flags;
			lev_flags = 0;	/* clear for next user */
		  }
break;
case 16:
{
			lev_flags |= yyvsp[-2].i;
		  }
break;
case 17:
{
			lev_flags |= yyvsp[0].i;
		  }
break;
case 20:
{
			int i, j;

			i = strlen(yyvsp[0].map) + 1;
			j = tmpmessage[0] ? strlen(tmpmessage) : 0;
			if(i+j > 255) {
			   yyerror("Message string too long (>256 characters)");
			} else {
			    if(j) tmpmessage[j++] = '\n';
			    strncpy(tmpmessage+j, yyvsp[0].map, i-1);
			    tmpmessage[j+i-1] = 0;
			}
		  }
break;
case 23:
{
			if(special_lev.nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    special_lev.nrobjects = n_olist;
			    special_lev.robjects = (char *) alloc(n_olist);
			    (void) memcpy((genericptr_t)special_lev.robjects,
					  (genericptr_t)olist, n_olist);
			}
		  }
break;
case 24:
{
			if(special_lev.nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    special_lev.nrmonst = n_mlist;
			    special_lev.rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)special_lev.rmonst,
					  (genericptr_t)mlist, n_mlist);
			  }
		  }
break;
case 25:
{
			tmproom[nrooms] = New(room);
			(void) memset((genericptr_t) tmproom[nrooms], 0,
					sizeof *tmproom[nrooms]);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = 0;
			tmproom[nrooms]->rlit = 0;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = 0;
			tmproom[nrooms]->y = 0;
			tmproom[nrooms]->w = 2;
			tmproom[nrooms]->h = 2;
			in_room = 1;
		  }
break;
case 31:
{
			tmpcor[0] = New(corridor);
			tmpcor[0]->src.room = -1;
			ncorridor = 1;
		  }
break;
case 34:
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = yyvsp[0].corpos.room;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].corpos.wall;
			tmpcor[ncorridor]->dest.door = yyvsp[0].corpos.door;
			ncorridor++;
		  }
break;
case 35:
{
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = -1;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].i;
			ncorridor++;
		  }
break;
case 36:
{
			if (yyvsp[-5].i >= nrooms)
			    yyerror("Wrong room number!");
			yyval.corpos.room = yyvsp[-5].i;
			yyval.corpos.wall = yyvsp[-3].i;
			yyval.corpos.door = yyvsp[-1].i;
		  }
break;
case 37:
{
			store_room();
		  }
break;
case 38:
{
			store_room();
		  }
break;
case 39:
{
			tmproom[nrooms] = New(room);
			(void) memset((genericptr_t) tmproom[nrooms], 0,
					sizeof *tmproom[nrooms]);
			tmproom[nrooms]->parent = dup_string(yyvsp[-1].map);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->rtype = yyvsp[-9].i;
			tmproom[nrooms]->rlit = yyvsp[-7].i;
			tmproom[nrooms]->filled = yyvsp[0].i;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  }
break;
case 40:
{
			tmproom[nrooms] = New(room);
			(void) memset((genericptr_t) tmproom[nrooms], 0,
					sizeof *tmproom[nrooms]);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = yyvsp[-9].i;
			tmproom[nrooms]->rlit = yyvsp[-7].i;
			tmproom[nrooms]->filled = yyvsp[0].i;
			tmproom[nrooms]->xalign = current_align.x;
			tmproom[nrooms]->yalign = current_align.y;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  }
break;
case 41:
{
			yyval.i = 1;
		  }
break;
case 42:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 43:
{
			if ( yyvsp[-3].i < 1 || yyvsp[-3].i > 5 ||
			    yyvsp[-1].i < 1 || yyvsp[-1].i > 5 ) {
			    yyerror("Room position should be between 1 & 5!");
			} else {
			    current_coord.x = yyvsp[-3].i;
			    current_coord.y = yyvsp[-1].i;
			}
		  }
break;
case 44:
{
			current_coord.x = current_coord.y = ERR;
		  }
break;
case 45:
{
			if ( yyvsp[-3].i < 0 || yyvsp[-1].i < 0) {
			    yyerror("Invalid subroom position !");
			} else {
			    current_coord.x = yyvsp[-3].i;
			    current_coord.y = yyvsp[-1].i;
			}
		  }
break;
case 46:
{
			current_coord.x = current_coord.y = ERR;
		  }
break;
case 47:
{
			current_align.x = yyvsp[-3].i;
			current_align.y = yyvsp[-1].i;
		  }
break;
case 48:
{
			current_align.x = current_align.y = ERR;
		  }
break;
case 49:
{
			current_size.width = yyvsp[-3].i;
			current_size.height = yyvsp[-1].i;
		  }
break;
case 50:
{
			current_size.height = current_size.width = ERR;
		  }
break;
case 66:
{
			if (tmproom[nrooms]->name)
			    yyerror("This room already has a name!");
			else
			    tmproom[nrooms]->name = dup_string(yyvsp[0].map);
		  }
break;
case 67:
{
			if (tmproom[nrooms]->chance)
			    yyerror("This room already assigned a chance!");
			else if (tmproom[nrooms]->rtype == OROOM)
			    yyerror("Only typed rooms can have a chance!");
			else if (yyvsp[0].i < 1 || yyvsp[0].i > 99)
			    yyerror("The chance is supposed to be precentile.");
			else
			    tmproom[nrooms]->chance = yyvsp[0].i;
		   }
break;
case 68:
{
			/* ERR means random here */
			if (yyvsp[-2].i == ERR && yyvsp[0].i != ERR) {
		     yyerror("If the door wall is random, so must be its pos!");
			} else {
			    tmprdoor[ndoor] = New(room_door);
			    tmprdoor[ndoor]->secret = yyvsp[-6].i;
			    tmprdoor[ndoor]->mask = yyvsp[-4].i;
			    tmprdoor[ndoor]->wall = yyvsp[-2].i;
			    tmprdoor[ndoor]->pos = yyvsp[0].i;
			    ndoor++;
			}
		  }
break;
case 75:
{
			maze.filling = yyvsp[0].i;
			if (index(yyvsp[-2].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if (strlen(yyvsp[-2].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[-2].map;
			in_room = 0;
		  }
break;
case 76:
{
			yyval.i = get_floor_type((char)yyvsp[0].i);
		  }
break;
case 77:
{
			yyval.i = -1;
		  }
break;
case 80:
{
			store_part();
		  }
break;
case 81:
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = 1;
			tmppart[npart]->valign = 1;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			tmppart[npart]->xsize = 1;
			tmppart[npart]->ysize = 1;
			tmppart[npart]->map = (char **) alloc(sizeof(char *));
			tmppart[npart]->map[0] = (char *) alloc(1);
			tmppart[npart]->map[0][0] = STONE;
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
break;
case 82:
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = yyvsp[-1].i % 10;
			tmppart[npart]->valign = yyvsp[-1].i / 10;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			scan_map(yyvsp[0].map);
		  }
break;
case 83:
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i * 10);
		  }
break;
case 90:
{
			if (tmppart[npart]->nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    tmppart[npart]->robjects = (char *)alloc(n_olist);
			    (void) memcpy((genericptr_t)tmppart[npart]->robjects,
					  (genericptr_t)olist, n_olist);
			    tmppart[npart]->nrobjects = n_olist;
			}
		  }
break;
case 91:
{
			if (tmppart[npart]->nloc) {
			    yyerror("Location registers already initialized!");
			} else {
			    register int i;
			    tmppart[npart]->rloc_x = (char *) alloc(n_plist);
			    tmppart[npart]->rloc_y = (char *) alloc(n_plist);
			    for(i=0;i<n_plist;i++) {
				tmppart[npart]->rloc_x[i] = plist[i].x;
				tmppart[npart]->rloc_y[i] = plist[i].y;
			    }
			    tmppart[npart]->nloc = n_plist;
			}
		  }
break;
case 92:
{
			if (tmppart[npart]->nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    tmppart[npart]->rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)tmppart[npart]->rmonst,
					  (genericptr_t)mlist, n_mlist);
			    tmppart[npart]->nrmonst = n_mlist;
			}
		  }
break;
case 93:
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yyvsp[0].i;
			else
			    yyerror("Object list too long!");
		  }
break;
case 94:
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yyvsp[-2].i;
			else
			    yyerror("Object list too long!");
		  }
break;
case 95:
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yyvsp[0].i;
			else
			    yyerror("Monster list too long!");
		  }
break;
case 96:
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yyvsp[-2].i;
			else
			    yyerror("Monster list too long!");
		  }
break;
case 97:
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
break;
case 98:
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
break;
case 122:
{
			tmpmonst[nmons] = New(monster);
			tmpmonst[nmons]->x = current_coord.x;
			tmpmonst[nmons]->y = current_coord.y;
			tmpmonst[nmons]->class = yyvsp[-4].i;
			tmpmonst[nmons]->peaceful = -1; /* no override */
			tmpmonst[nmons]->asleep = -1;
			tmpmonst[nmons]->align = - MAX_REGISTERS - 2;
			tmpmonst[nmons]->name = (char *) 0;
			tmpmonst[nmons]->appear = 0;
			tmpmonst[nmons]->appear_as = (char *) 0;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Monster");
			if (!yyvsp[-2].map)
			    tmpmonst[nmons]->id = -1;
			else {
				int token = get_monster_id(yyvsp[-2].map, (char) yyvsp[-4].i);
				if (token == ERR) {
				    yywarning("Illegal monster name!  Making random monster.");
				    tmpmonst[nmons]->id = -1;
				} else
				    tmpmonst[nmons]->id = token;
			}
		  }
break;
case 123:
{
			nmons++;
		  }
break;
case 126:
{
			tmpmonst[nmons]->name = dup_string(yyvsp[0].map);
		  }
break;
case 127:
{
			tmpmonst[nmons]->peaceful = yyvsp[0].i;
		  }
break;
case 128:
{
			tmpmonst[nmons]->asleep = yyvsp[0].i;
		  }
break;
case 129:
{
			tmpmonst[nmons]->align = yyvsp[0].i;
		  }
break;
case 130:
{
			tmpmonst[nmons]->appear = yyvsp[-1].i;
			tmpmonst[nmons]->appear_as = dup_string(yyvsp[0].map);
		  }
break;
case 131:
{
			tmpobj[nobj] = New(object);
			tmpobj[nobj]->x = current_coord.x;
			tmpobj[nobj]->y = current_coord.y;
			tmpobj[nobj]->class = yyvsp[-4].i;
			tmpobj[nobj]->corpsenm = -1;	/* init as none */
			tmpobj[nobj]->curse_state = -1;
			tmpobj[nobj]->name = (char *) 0;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Object");
			if (!yyvsp[-2].map)
			    tmpobj[nobj]->id = -1;
			else {
				int token = get_object_id(yyvsp[-2].map);
				if (token == ERR) {
				    yywarning("Illegal object name!  Making random object.");
				    tmpobj[nobj]->id = -1;
				} else
				    tmpobj[nobj]->id = token;
			}
		  }
break;
case 132:
{
			nobj++;
		  }
break;
case 133:
{
			tmpobj[nobj]->spe = -127;
		  }
break;
case 134:
{
			int token = get_monster_id(yyvsp[-2].map, (char)0);
			if (token == ERR)	/* "random" */
			    tmpobj[nobj]->corpsenm = -2;
			else
			    tmpobj[nobj]->corpsenm = token;
			tmpobj[nobj]->spe = yyvsp[0].i;
		  }
break;
case 135:
{
			tmpobj[nobj]->curse_state = yyvsp[-4].i;
			tmpobj[nobj]->spe = yyvsp[-2].i;
			if (yyvsp[0].map)
			    tmpobj[nobj]->name = dup_string(yyvsp[0].map);
			else
			    tmpobj[nobj]->name = (char *) 0;
		  }
break;
case 139:
{
			yyval.i = -127;
		  }
break;
case 140:
{
			tmpdoor[ndoor] = New(door);
			tmpdoor[ndoor]->x = current_coord.x;
			tmpdoor[ndoor]->y = current_coord.y;
			tmpdoor[ndoor]->mask = yyvsp[-2].i;
			if(current_coord.x >= 0 && current_coord.y >= 0 &&
			   tmpmap[current_coord.y][current_coord.x] != DOOR &&
			   tmpmap[current_coord.y][current_coord.x] != SDOOR)
			    yyerror("Door decl doesn't match the map");
			ndoor++;
		  }
break;
case 141:
{
			tmptrap[ntrap] = New(trap);
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Trap");
			ntrap++;
		  }
break;
case 142:
{
		        int x, y, dir;

			tmpdb[ndb] = New(drawbridge);
			x = tmpdb[ndb]->x = current_coord.x;
			y = tmpdb[ndb]->y = current_coord.y;
			/* convert dir from a DIRECTION to a DB_DIR */
			dir = yyvsp[-2].i;
			switch(dir) {
			case W_NORTH: dir = DB_NORTH; y--; break;
			case W_SOUTH: dir = DB_SOUTH; y++; break;
			case W_EAST:  dir = DB_EAST;  x++; break;
			case W_WEST:  dir = DB_WEST;  x--; break;
			default:
			    yyerror("Invalid drawbridge direction");
			    break;
			}
			tmpdb[ndb]->dir = dir;
			if (current_coord.x >= 0 && current_coord.y >= 0 &&
			    !IS_WALL(tmpmap[y][x])) {
			    char ebuf[60];
			    Sprintf(ebuf,
				    "Wall needed for drawbridge (%02d, %02d)",
				    current_coord.x, current_coord.y);
			    yyerror(ebuf);
			}

			if ( yyvsp[0].i == D_ISOPEN )
			    tmpdb[ndb]->db_open = 1;
			else if ( yyvsp[0].i == D_CLOSED )
			    tmpdb[ndb]->db_open = 0;
			else
			    yyerror("A drawbridge can only be open or closed!");
			ndb++;
		   }
break;
case 143:
{
			tmpwalk[nwalk] = New(walk);
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = yyvsp[0].i;
			nwalk++;
		  }
break;
case 144:
{
			wallify_map();
		  }
break;
case 145:
{
			tmplad[nlad] = New(lad);
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Ladder");
			nlad++;
		  }
break;
case 146:
{
			tmpstair[nstair] = New(stair);
			tmpstair[nstair]->x = current_coord.x;
			tmpstair[nstair]->y = current_coord.y;
			tmpstair[nstair]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Stairway");
			nstair++;
		  }
break;
case 147:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 148:
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			if(yyvsp[0].i)
			    tmplreg[nlreg]->rtype = LR_UPSTAIR;
			else
			    tmplreg[nlreg]->rtype = LR_DOWNSTAIR;
			tmplreg[nlreg]->rname = 0;
			nlreg++;
		  }
break;
case 149:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 150:
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_PORTAL;
			tmplreg[nlreg]->rname = yyvsp[0].map;
			nlreg++;
		  }
break;
case 151:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 152:
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
		  }
break;
case 153:
{
			switch(yyvsp[0].i) {
			case -1: tmplreg[nlreg]->rtype = LR_TELE; break;
			case 0: tmplreg[nlreg]->rtype = LR_DOWNTELE; break;
			case 1: tmplreg[nlreg]->rtype = LR_UPTELE; break;
			}
			tmplreg[nlreg]->rname = 0;
			nlreg++;
		  }
break;
case 154:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 155:
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_BRANCH;
			tmplreg[nlreg]->rname = 0;
			nlreg++;
		  }
break;
case 156:
{
			yyval.i = -1;
		  }
break;
case 157:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 158:
{
			yyval.i = 0;
		  }
break;
case 159:
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i <= 0 || yyvsp[-7].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i >= ROWNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-3].i <= 0 || yyvsp[-3].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i >= ROWNO)
				yyerror("Region out of level range!");
			current_region.x1 = yyvsp[-7].i;
			current_region.y1 = yyvsp[-5].i;
			current_region.x2 = yyvsp[-3].i;
			current_region.y2 = yyvsp[-1].i;
			yyval.i = 1;
		  }
break;
case 160:
{
			tmpfountain[nfountain] = New(fountain);
			tmpfountain[nfountain]->x = current_coord.x;
			tmpfountain[nfountain]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Fountain");
			nfountain++;
		  }
break;
case 161:
{
			tmpsink[nsink] = New(sink);
			tmpsink[nsink]->x = current_coord.x;
			tmpsink[nsink]->y = current_coord.y;
			nsink++;
		  }
break;
case 162:
{
			tmppool[npool] = New(pool);
			tmppool[npool]->x = current_coord.x;
			tmppool[npool]->y = current_coord.y;
			npool++;
		  }
break;
case 163:
{
			tmpdig[ndig] = New(digpos);
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
		  }
break;
case 164:
{
			tmppass[npass] = New(digpos);
			tmppass[npass]->x1 = current_region.x1;
			tmppass[npass]->y1 = current_region.y1;
			tmppass[npass]->x2 = current_region.x2;
			tmppass[npass]->y2 = current_region.y2;
			npass++;
		  }
break;
case 165:
{
			tmpreg[nreg] = New(region);
			tmpreg[nreg]->x1 = current_region.x1;
			tmpreg[nreg]->y1 = current_region.y1;
			tmpreg[nreg]->x2 = current_region.x2;
			tmpreg[nreg]->y2 = current_region.y2;
			tmpreg[nreg]->rlit = yyvsp[-3].i;
			tmpreg[nreg]->rtype = yyvsp[-1].i;
			if(yyvsp[0].i & 1) tmpreg[nreg]->rtype += MAXRTYPE+1;
			tmpreg[nreg]->rirreg = ((yyvsp[0].i & 2) != 0);
			if(current_region.x1 > current_region.x2 ||
			   current_region.y1 > current_region.y2)
			   yyerror("Region start > end!");
			if(tmpreg[nreg]->rtype == VAULT &&
			   (tmpreg[nreg]->rirreg ||
			    (tmpreg[nreg]->x2 - tmpreg[nreg]->x1 != 1) ||
			    (tmpreg[nreg]->y2 - tmpreg[nreg]->y1 != 1)))
				yyerror("Vaults must be exactly 2x2!");
			if(want_warnings && !tmpreg[nreg]->rirreg &&
			   current_region.x1 > 0 && current_region.y1 > 0 &&
			   current_region.x2 < max_x_map &&
			   current_region.y2 < max_y_map) {
			    /* check for walls in the room */
			    char ebuf[60];
			    register int x, y, nrock = 0;

			    for(y=current_region.y1; y<=current_region.y2; y++)
				for(x=current_region.x1;
				    x<=current_region.x2; x++)
				    if(IS_ROCK(tmpmap[y][x]) ||
				       IS_DOOR(tmpmap[y][x])) nrock++;
			    if(nrock) {
				Sprintf(ebuf,
					"Rock in room (%02d,%02d,%02d,%02d)?!",
					current_region.x1, current_region.y1,
					current_region.x2, current_region.y2);
				yywarning(ebuf);
			    }
			    if (
		!IS_ROCK(tmpmap[current_region.y1-1][current_region.x1-1]) ||
		!IS_ROCK(tmpmap[current_region.y2+1][current_region.x1-1]) ||
		!IS_ROCK(tmpmap[current_region.y1-1][current_region.x2+1]) ||
		!IS_ROCK(tmpmap[current_region.y2+1][current_region.x2+1])) {
				Sprintf(ebuf,
				"NonRock edge in room (%02d,%02d,%02d,%02d)?!",
					current_region.x1, current_region.y1,
					current_region.x2, current_region.y2);
				yywarning(ebuf);
			    }
			} else if(tmpreg[nreg]->rirreg &&
		!IS_ROOM(tmpmap[current_region.y1][current_region.x1])) {
			    char ebuf[60];
			    Sprintf(ebuf,
				    "Rock in irregular room (%02d,%02d)?!",
				    current_region.x1, current_region.y1);
			    yyerror(ebuf);
			}
			nreg++;
		  }
break;
case 166:
{
			tmpaltar[naltar] = New(altar);
			tmpaltar[naltar]->x = current_coord.x;
			tmpaltar[naltar]->y = current_coord.y;
			tmpaltar[naltar]->align = yyvsp[-2].i;
			tmpaltar[naltar]->shrine = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Altar");
			naltar++;
		  }
break;
case 167:
{
			tmpgold[ngold] = New(gold);
			tmpgold[ngold]->x = current_coord.x;
			tmpgold[ngold]->y = current_coord.y;
			tmpgold[ngold]->amount = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Gold");
			ngold++;
		  }
break;
case 168:
{
			tmpengraving[nengraving] = New(engraving);
			tmpengraving[nengraving]->x = current_coord.x;
			tmpengraving[nengraving]->y = current_coord.y;
			tmpengraving[nengraving]->e.text = yyvsp[0].map;
			tmpengraving[nengraving]->etype = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Engraving");
			nengraving++;
		  }
break;
case 170:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 173:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 176:
{
			yyval.map = (char *) 0;
		  }
break;
case 178:
{
			yyval.map = (char *) 0;
		  }
break;
case 179:
{
			int token = get_trap_type(yyvsp[0].map);
			if (token == ERR)
				yyerror("Unknown trap type!");
			yyval.i = token;
		  }
break;
case 181:
{
			int token = get_room_type(yyvsp[0].map);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				yyval.i = OROOM;
			} else
				yyval.i = token;
		  }
break;
case 183:
{
			yyval.i = 0;
		  }
break;
case 184:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 185:
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i << 1);
		  }
break;
case 188:
{
			current_coord.x = current_coord.y = -MAX_REGISTERS-1;
		  }
break;
case 195:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 198:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				current_coord.x = current_coord.y = - yyvsp[-1].i - 1;
		  }
break;
case 199:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 200:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 201:
{
			if ( yyvsp[-1].i >= 3 )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 203:
{
			if (check_monster_char((char) yyvsp[0].i))
				yyval.i = yyvsp[0].i ;
			else {
				yyerror("Unknown monster class!");
				yyval.i = ERR;
			}
		  }
break;
case 204:
{
			char c = yyvsp[0].i;
			if (check_object_char(c))
				yyval.i = c;
			else {
				yyerror("Unknown char class!");
				yyval.i = ERR;
			}
		  }
break;
case 207:
{
			yyval.map = (char *) 0;
		  }
break;
case 212:
{
			if (!in_room && !init_lev.init_present &&
			    (yyvsp[-3].i < 0 || yyvsp[-3].i > max_x_map ||
			     yyvsp[-1].i < 0 || yyvsp[-1].i > max_y_map))
			    yyerror("Coordinates out of map range!");
			current_coord.x = yyvsp[-3].i;
			current_coord.y = yyvsp[-1].i;
		  }
break;
case 213:
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i < 0 || yyvsp[-7].i > max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i > max_y_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-3].i < 0 || yyvsp[-3].i > max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i > max_y_map)
				yyerror("Region out of map range!");
			current_region.x1 = yyvsp[-7].i;
			current_region.y1 = yyvsp[-5].i;
			current_region.x2 = yyvsp[-3].i;
			current_region.y2 = yyvsp[-1].i;
		  }
break;
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) != 0 && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
