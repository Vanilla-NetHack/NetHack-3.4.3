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
/*	SCCS Id: @(#)lev_yacc.c	3.2	95/11/10	*/
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
 * and AIX will still see the directive.
 */
#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "hack.h"
#include "sp_lev.h"

#define MAX_REGISTERS	10
#define ERR		(-1)
/* many types of things are put in chars for transference to NetHack.
 * since some systems will use signed chars, limit everybody to the
 * same number for portability.
 */
#define MAX_OF_TYPE	128

#define New(type)		(type *) alloc(sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)
#define Free(ptr)		free((genericptr_t)ptr)

#ifdef MICRO
# undef exit
# if !defined(MSDOS) && !defined(WIN32)
extern void FDECL(exit, (int));
# endif
#endif

extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);

extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *,CHAR_P));
extern int FDECL(get_object_id, (char *));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *));
extern void NDECL(wallify_map);
extern boolean NDECL(check_subrooms);
extern void FDECL(check_coord, (int,int,const char *));
extern void NDECL(store_part);
extern void NDECL(store_room);
extern boolean FDECL(write_level_file, (char *,splev *,specialmaze *));
extern void FDECL(free_rooms, (splev *));

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
digpos *tmppass[32];
char *tmpmap[ROWNO];

digpos *tmpdig[MAX_OF_TYPE];
region *tmpreg[MAX_OF_TYPE];
lev_region *tmplreg[MAX_OF_TYPE];
door *tmpdoor[MAX_OF_TYPE];
drawbridge *tmpdb[MAX_OF_TYPE];
walk *tmpwalk[MAX_OF_TYPE];

room_door *tmprdoor[MAX_OF_TYPE];
trap *tmptrap[MAX_OF_TYPE];
monster *tmpmonst[MAX_OF_TYPE];
object *tmpobj[MAX_OF_TYPE];
altar *tmpaltar[MAX_OF_TYPE];
lad *tmplad[MAX_OF_TYPE];
stair *tmpstair[MAX_OF_TYPE];
gold *tmpgold[MAX_OF_TYPE];
engraving *tmpengraving[MAX_OF_TYPE];
fountain *tmpfountain[MAX_OF_TYPE];
sink *tmpsink[MAX_OF_TYPE];
pool *tmppool[MAX_OF_TYPE];

mazepart *tmppart[10];
room *tmproom[MAXNROFROOMS*2];
corridor *tmpcor[MAX_OF_TYPE];

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

static int lev_flags = 0;

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
#define COBJECT_ID 267
#define MONSTER_ID 268
#define TRAP_ID 269
#define DOOR_ID 270
#define DRAWBRIDGE_ID 271
#define MAZEWALK_ID 272
#define WALLIFY_ID 273
#define REGION_ID 274
#define FILLING 275
#define RANDOM_OBJECTS_ID 276
#define RANDOM_MONSTERS_ID 277
#define RANDOM_PLACES_ID 278
#define ALTAR_ID 279
#define LADDER_ID 280
#define STAIR_ID 281
#define NON_DIGGABLE_ID 282
#define NON_PASSWALL_ID 283
#define ROOM_ID 284
#define PORTAL_ID 285
#define TELEPRT_ID 286
#define BRANCH_ID 287
#define LEV 288
#define CHANCE_ID 289
#define CORRIDOR_ID 290
#define GOLD_ID 291
#define ENGRAVING_ID 292
#define FOUNTAIN_ID 293
#define POOL_ID 294
#define SINK_ID 295
#define NONE 296
#define RAND_CORRIDOR_ID 297
#define DOOR_STATE 298
#define LIGHT_STATE 299
#define CURSE_TYPE 300
#define ENGRAVING_TYPE 301
#define DIRECTION 302
#define RANDOM_TYPE 303
#define O_REGISTER 304
#define M_REGISTER 305
#define P_REGISTER 306
#define A_REGISTER 307
#define ALIGNMENT 308
#define LEFT_OR_RIGHT 309
#define CENTER 310
#define TOP_OR_BOT 311
#define ALTAR_TYPE 312
#define UP_OR_DOWN 313
#define SUBROOM_ID 314
#define NAME_ID 315
#define FLAGS_ID 316
#define FLAG_TYPE 317
#define MON_ATTITUDE 318
#define MON_ALERTNESS 319
#define MON_APPEARANCE 320
#define CONTAINED 321
#define STRING 322
#define MAP_ID 323
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   35,   35,   36,   36,   37,   38,   31,   22,
   22,   14,   14,   18,   18,   19,   19,   39,   39,   44,
   41,   41,   45,   45,   42,   42,   48,   48,   43,   43,
   50,   51,   51,   52,   52,   34,   49,   49,   55,   53,
   10,   10,   58,   58,   56,   56,   59,   59,   57,   57,
   54,   54,   60,   60,   60,   60,   60,   60,   60,   60,
   60,   60,   60,   60,   60,   61,   62,   63,   15,   15,
   13,   13,   12,   12,   30,   11,   11,   40,   40,   74,
   75,   75,   78,    1,    1,    2,    2,   76,   76,   79,
   79,   79,   46,   46,   47,   47,   80,   82,   80,   77,
   77,   83,   83,   83,   83,   83,   83,   83,   83,   83,
   83,   83,   83,   83,   83,   83,   83,   83,   83,   83,
   83,   98,   64,   97,   97,   99,   99,   99,   99,   99,
   65,   65,  101,  100,  102,  102,  103,  103,  103,  103,
  104,  104,  105,  106,  106,  107,  107,  107,   84,   66,
   66,   85,   91,   92,   93,   73,  109,   87,  110,   88,
  111,  113,   89,  114,   90,  112,  112,   21,   21,   68,
   69,   70,   94,   95,   86,   67,   71,   72,   24,   24,
   24,   27,   27,   27,   32,   32,   33,   33,    3,    3,
  108,    4,    4,   20,   20,   20,   96,   96,   96,    5,
    5,    6,    6,    7,    7,    7,    8,    8,  117,   28,
   25,    9,   81,   23,   26,   29,   16,   16,   17,   17,
  116,  115,
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
    3,    3,    0,    7,    1,    1,    0,    7,    5,    5,
    1,    1,    1,    1,    1,    0,    2,    2,    5,    5,
    7,    7,    5,    1,    5,    5,    0,    8,    0,    8,
    0,    0,    8,    0,    6,    0,    2,    1,   10,    3,
    3,    3,    3,    3,    8,    7,    5,    7,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    3,    1,    1,    0,    2,    4,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    4,    4,
    4,    4,    1,    1,    1,    1,    1,    1,    1,    1,
    5,    9,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    2,    0,    5,    6,    0,
    0,    0,    0,    0,    4,  216,    0,    9,    0,    0,
    0,    0,    0,    0,   15,    0,    0,    0,    0,   21,
   76,   77,   75,    0,    0,    0,    0,   81,    7,    0,
   88,    0,   19,    0,   16,    0,   20,    0,   79,    0,
   82,    0,    0,    0,    0,    0,   22,   26,    0,   51,
   51,    0,   84,   85,    0,    0,    0,    0,    0,   89,
    0,    0,    0,    0,   31,    8,   29,    0,   28,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  154,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  102,  103,  105,  112,
  113,  118,  119,  117,  101,  104,  106,  107,  108,  109,
  110,  111,  114,  115,  116,  120,  121,  215,    0,   23,
  214,    0,   24,  193,    0,  192,    0,    0,   33,    0,
    0,    0,    0,    0,    0,   52,   53,   54,   55,   56,
   57,   58,   59,   60,   61,   62,   63,   64,   65,    0,
   87,   86,   83,   90,   92,    0,   91,    0,  213,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  183,    0,  182,    0,  184,  131,  132,  180,
    0,  179,    0,  181,  190,    0,  189,  200,  201,    0,
  199,    0,    0,  197,  198,    0,    0,    0,    0,    0,
    0,    0,  157,    0,  168,  173,  174,  159,  161,  164,
  217,  218,    0,    0,  170,   94,   96,  202,  203,    0,
    0,    0,    0,   69,   70,    0,   67,  172,  171,   66,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   99,    0,  188,  187,  133,    0,  186,  185,
    0,    0,  149,    0,    0,  153,    0,    0,  206,    0,
  204,    0,  205,  155,    0,    0,    0,  156,    0,    0,
    0,  177,  219,  220,    0,   44,    0,    0,   46,    0,
    0,    0,   35,   34,    0,    0,  221,  210,    0,  211,
    0,    0,  209,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  162,  165,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  122,    0,  151,  152,    0,    0,    0,
  208,  207,  176,    0,    0,    0,    0,  178,    0,   48,
    0,    0,    0,   50,    0,    0,    0,   71,   72,    0,
   12,   13,   11,  136,  135,    0,  124,    0,    0,    0,
  175,  212,    0,  158,  160,    0,  163,    0,    0,    0,
    0,    0,    0,   73,   74,    0,    0,    0,  134,    0,
  191,    0,    0,    0,  167,   43,    0,    0,   45,    0,
    0,   36,   68,  142,  141,  143,    0,    0,    0,  125,
    0,    0,    0,    0,    0,   40,    0,   39,    0,    0,
  127,  128,    0,  129,  126,  222,  196,    0,   47,   42,
   49,  145,  144,    0,    0,    0,  130,  169,    0,    0,
  139,  140,    0,  147,  148,  138,
};
short yydgoto[] = {                                       3,
   65,  163,  216,  135,  220,  250,  312,  373,  313,  446,
   33,  416,  390,  393,  256,  243,  325,   13,   25,  401,
  233,   21,  132,  213,  214,  129,  206,  207,  136,    4,
    5,  301,  297,  253,    6,    7,    8,    9,   28,   39,
   44,   56,   76,   29,   57,  130,  133,   58,   59,   77,
   78,  139,   60,   80,   61,  331,  386,  328,  382,  146,
  147,  148,  149,  150,  151,  152,  153,  154,  155,  156,
  157,  158,  159,   40,   41,   50,   69,   42,   70,  167,
  168,  202,  115,  116,  117,  118,  119,  120,  121,  122,
  123,  124,  125,  126,  127,  234,  420,  397,  440,  208,
  339,  396,  419,  437,  438,  465,  471,  366,  279,  281,
  282,  407,  377,  283,  235,  224,  225,
};
short yysindex[] = {                                   -122,
   -7,    6,    0, -240, -240,    0, -122,    0,    0, -206,
 -206,   60, -141, -141,    0,    0,   86,    0, -185,   88,
 -107, -107, -232,  100,    0,  -79,  126, -100, -107,    0,
    0,    0,    0, -185,  144, -127,  138,    0,    0, -100,
    0, -125,    0, -216,    0,  -60,    0, -133,    0, -126,
    0,  141,  142,  143,  146,  -95,    0,    0, -221,    0,
    0,  161,    0,    0,  162,  149,  150,  151, -112,    0,
  -47,  -46, -255, -255,    0,    0,    0,  -76,    0, -236,
 -236,  -44, -128,  -47,  -46,  172,  158,  159,  160,  163,
  165,  167,  168,    0,  169,  170,  171,  173,  174,  175,
  176,  177,  178,  179,  180,  181,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  186,    0,
    0,  196,    0,    0,  197,    0,  198,  185,    0,  187,
  188,  189,  190,  191,  192,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  200,
    0,    0,    0,    0,    0,  -39,    0,    0,    0, -220,
 -220, -233, -249, -160,   34,   34,  184,   34,   34,   54,
  184,  184,  -37,  -37,  -37, -223,   34,   34,  -47,  -46,
 -261, -261,  212, -215,   34,  -38,   34,   34, -206,   -6,
  210,  215,    0,  207,    0,  216,    0,    0,    0,    0,
  211,    0,  217,    0,    0,  218,    0,    0,    0,  221,
    0,  224,  283,    0,    0,  291,   -2,  295,  308,  315,
  318,  123,    0,  317,    0,    0,    0,    0,    0,    0,
    0,    0,  357,  358,    0,    0,    0,    0,    0,  360,
  361,  152,  364,    0,    0,  365,    0,    0,    0,    0,
  368,  155,  172,  157, -225,  164, -211,   34,   34,  183,
   97,  114,  375, -261, -174,  107,  208,  377,  379,  111,
  381,  382,  398,   34, -189,  -25,  -24,  423,  -36, -160,
 -261,  427,    0,  383,    0,    0,    0,  384,    0,    0,
  434,  435,    0,  390,  441,    0,  228,  445,    0,  397,
    0,  446,    0,    0,  447,  241,  -37,    0,  -37,  -37,
  -37,    0,    0,    0,  457,    0,  244,  460,    0,  247,
  463,  204,    0,    0,  464,  465,    0,    0,  467,    0,
   34,  223,    0, -160,  470, -255,  257, -263,  263,   21,
  472,  489,    0,    0, -206,  503,  -23,  508,  -13,  509,
 -116, -213,   35,    0,  496,    0,    0,  298,  514,  466,
    0,    0,    0,  516,  248, -206,  518,    0,  305,    0,
 -133,  520,  307,    0,  309,  522, -222,    0,    0,  524,
    0,    0,    0,    0,    0,  525,    0,  312,  527,  299,
    0,    0,  319,    0,    0,  260,    0,  561,  559,  -13,
  567,  565, -206,    0,    0,  569, -222, -253,    0,  568,
    0,  356,  577,  579,    0,    0, -128,  580,    0,  367,
  580,    0,    0,    0,    0,    0,  582,  583, -212,    0,
  572,  370,  372,  592,  376,    0,  593,    0, -237, -217,
    0,    0, -206,    0,    0,    0,    0,  595,    0,    0,
    0,    0,    0,  594,  600,  600,    0,    0, -217, -270,
    0,    0,  600,    0,    0,    0,
};
short yyrindex[] = {                                    637,
    0,    0,    0, -137,  380,    0,  646,    0,    0,    0,
    0,    0, -129,  406,    0,    0,    0,    0,    0,    0,
  -74,  454,    0,  355,    0,    0,    0,    0,  411,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   87,
    0,    0,    0,  117,    0,    0,    0,    0,    0,  546,
    0,    0,    0,    0,    0,  101,    0,    0,  213,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   82,    0,
    0,    0,    0,    0,    0,    0,    0,   89,    0,  444,
  462,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  258,    0,
    0,  314,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  513,    0,    0,
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
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  589,    0,
    0,    0,    0,    0,    0,    0,  622,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   39,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  103,
    0,    0,  655,    0,    0,    0,    0,  203,    0,    0,
  203,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  166,  166,    0,    0,    0,    0,
    0,    0,  166,    0,    0,    0,
};
short yygindex[] = {                                      0,
  266,  222,    0,  -69, -267, -170,  209,    0,    0,  219,
    0,  234,    0,    0,    0,    0,    0,  648,  620,    0,
 -172,  644,  487,    0,    0,   22,    0,    0,  -10,    0,
    0,    0,    0,  371,  654,    0,    0,    0,   80,  623,
    0,    0,    0,    0,    0,  -70,  -65,  603,    0,    0,
    0,    0,    0,  604,    0,    0,  261,    0,    0,    0,
    0,    0,    0,  605,  609,  610,  611,  612,    0,    0,
  615,  616,  617,    0,    0,    0,    0,    0,    0,  426,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -169,    0,    0,    0,  521,
    0,    0,    0,    0,  242, -345, -353,    0,    0,    0,
    0,    0,    0,    0,  -40,  -78,    0,
};
#define YYTABLESIZE 948
short yytable[] = {                                      17,
   18,  150,  227,  252,  137,  223,  226,  169,  229,  230,
  238,  239,  240,  164,  327,  330,  381,  244,  245,  165,
  462,  251,  335,  131,   31,  474,  385,  258,  259,   87,
   88,   89,   90,  140,  241,  414,  128,  248,  137,  371,
  462,  249,   96,  254,  141,  391,  434,  134,  372,  435,
   10,  475,  142,  215,  104,  105,  106,  143,  144,   52,
   53,  337,   54,   11,  368,  463,   16,   54,  436,  210,
   32,  211,   16,  166,  166,   12,  367,  295,  145,  242,
  415,   80,  203,  204,  436,  463,   78,  255,   30,  392,
  309,  299,   55,  232,  310,  311,   16,   55,  302,  303,
   32,   30,  123,  308,  466,  451,  452,  453,   43,   16,
   16,  323,  472,  324,  322,   16,   25,   19,  246,  476,
  336,   20,   14,  473,  247,   14,   14,   14,  309,   23,
   10,   24,  310,  311,   10,   10,  228,  218,    1,    2,
  236,  237,  219,   34,  351,   26,  352,  353,  354,   66,
   67,   68,   27,   87,   88,   89,   90,   91,   92,   93,
   94,   95,  217,   37,   38,  146,   96,   97,   98,   99,
  100,  364,  101,  102,  103,   63,   64,   35,  104,  105,
  106,  161,  162,   36,  169,  388,  389,   46,  260,   18,
   18,  205,  205,  395,   47,   48,   62,   51,   71,   72,
   73,   75,   41,   74,   82,   83,   84,   85,   86,  128,
  131,  166,   27,  138,  160,  170,  171,  172,  201,  257,
  173,  333,  174,  227,  175,  176,  177,  178,  179,  189,
  180,  181,  182,  183,  184,  185,  186,  187,  188,  190,
  191,  192,  193,  200,  194,  195,  196,  197,  198,  199,
  231,  252,  261,  262,  296,  273,  300,   93,  263,  265,
  267,  268,  150,  150,  269,  150,  150,  150,  150,  150,
  150,  150,  150,  150,  150,  150,  369,  326,  329,  380,
  150,  150,  150,  150,  150,  150,  150,  150,  150,  384,
  150,  150,  150,  150,  150,  150,  150,  264,  150,  137,
  137,  266,  137,  137,  137,  137,  137,  137,  137,  137,
  137,  137,  137,   95,  270,  150,  150,  137,  137,  137,
  137,  137,  137,  137,  137,  137,  271,  137,  137,  137,
  137,  137,  137,  137,  272,  137,  221,  221,  274,  222,
  222,  231,   80,   80,  378,   80,   80,   78,   78,   30,
   30,  275,  137,  137,   17,  394,  221,  277,  276,  222,
  280,   32,   32,  123,  123,  405,  123,  123,  123,  123,
  123,  123,  123,  123,  123,  123,  123,   25,   25,   14,
  278,  123,  123,  123,  123,  123,  123,  123,  123,  123,
   32,  123,  123,  123,  123,  123,  123,  123,  305,  123,
  284,  285,  431,  286,  287,   10,   25,  289,  290,  288,
   18,  291,  292,   25,  294,  306,  123,  123,  307,  314,
  316,  298,  317,  318,  319,  320,  146,  146,  455,  146,
  146,  146,  146,  146,  146,  146,  146,  146,  146,  146,
  304,  321,  467,   37,  146,  146,  146,  146,  146,  146,
  146,  146,  146,   18,  146,  146,  146,  146,  146,  146,
  146,   38,  146,   41,   41,  315,  332,  337,   41,   41,
   41,   41,   41,   27,   27,  338,  340,  341,  342,  146,
  146,   41,  343,   41,  344,  345,   41,  347,  346,  348,
  349,   41,   41,   41,   41,   41,   41,   41,  350,   41,
  355,  356,   27,  357,  358,  360,  359,  361,  362,   27,
  363,  365,   97,  368,  370,  375,   41,   41,   93,   93,
  374,   93,   93,   93,   93,   93,   93,   93,   93,   93,
   93,   93,  376,   93,   93,   93,   93,   93,   93,   93,
   93,   93,   93,   93,   93,  100,  379,   93,   93,   93,
   93,  383,  387,  398,   93,  399,   98,  400,  402,  403,
  404,  406,  408,  410,  411,  413,  412,  417,  418,  421,
  422,   93,  425,  423,   95,   95,  424,   95,   95,   95,
   95,   95,   95,   95,   95,   95,   95,   95,  194,   95,
   95,   95,   95,   95,   95,   95,   95,   95,   95,   95,
   95,  426,  427,   95,   95,   95,   95,  429,  430,  432,
   95,  439,  456,  441,   17,   17,   17,   17,   17,   17,
  442,  166,  443,  445,  447,  449,  450,   95,  457,  458,
   17,   17,  459,  461,  460,  468,    1,  469,   17,   14,
   14,   14,   14,  470,   17,    3,  409,  454,  444,  448,
  433,   17,   14,   45,  195,   14,   14,   22,  212,  334,
   15,   79,   49,   14,   81,   10,   10,   10,   17,   14,
  428,   18,   18,  107,   18,   18,   14,  108,  109,  110,
  111,   10,   10,  112,  113,  114,   18,   18,  293,   10,
  464,  209,    0,   14,   18,   10,    0,    0,    0,    0,
   18,    0,   10,    0,   37,   37,    0,   18,    0,    0,
    0,    0,    0,    0,   18,   18,    0,    0,    0,   10,
    0,    0,   38,   38,   18,    0,    0,   37,    0,   18,
   18,    0,    0,   37,    0,    0,    0,   18,    0,    0,
   37,    0,    0,   18,    0,   38,    0,    0,    0,    0,
   18,   38,    0,    0,    0,    0,    0,   37,   38,    0,
    0,    0,    0,    0,    0,    0,    0,   18,    0,    0,
    0,    0,    0,   97,   97,   38,   97,   97,   97,   97,
   97,   97,   97,   97,   97,   97,   97,    0,   97,   97,
   97,   97,   97,   97,   97,   97,    0,   97,   97,   97,
    0,    0,    0,   97,   97,   97,  100,  100,    0,  100,
  100,  100,  100,  100,  100,  100,  100,  100,  100,  100,
    0,    0,    0,    0,  100,  100,  100,  100,  100,    0,
  100,  100,  100,    0,    0,    0,  100,  100,  100,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  194,
  194,    0,  194,  194,  194,  194,  194,  194,  194,  194,
  194,  194,  194,    0,    0,    0,    0,  194,  194,  194,
  194,  194,    0,  194,  194,  194,    0,    0,    0,  194,
  194,  194,  166,  166,    0,  166,  166,  166,  166,  166,
  166,  166,  166,  166,  166,  166,    0,    0,    0,    0,
  166,  166,  166,  166,  166,    0,  166,  166,  166,    0,
    0,    0,  166,  166,  166,  195,  195,    0,  195,  195,
  195,  195,  195,  195,  195,  195,  195,  195,  195,    0,
    0,    0,    0,  195,  195,  195,  195,  195,    0,  195,
  195,  195,    0,    0,    0,  195,  195,  195,
};
short yycheck[] = {                                      10,
   11,    0,   40,   40,   74,  175,  176,   86,  178,  179,
  183,  184,  185,   84,   40,   40,   40,  187,  188,   85,
  258,  192,  290,  257,  257,  296,   40,  197,  198,  266,
  267,  268,  269,  270,  258,  258,  257,  299,    0,  303,
  258,  303,  279,  259,  281,  259,  300,  303,  312,  303,
   58,  322,  289,  303,  291,  292,  293,  294,  295,  276,
  277,   41,  284,   58,   44,  303,  322,  284,  322,  303,
  303,  305,  322,   40,   40,  316,  344,  303,  315,  303,
  303,    0,  303,  304,  322,  303,    0,  303,    0,  303,
  303,  303,  314,   40,  307,  308,  322,  314,  268,  269,
    0,   22,    0,  274,  450,  318,  319,  320,   29,  322,
  322,  301,  466,  303,  284,  322,    0,   58,  189,  473,
  291,  263,  260,  469,  190,  263,  264,  265,  303,   44,
  260,  317,  307,  308,  264,  265,  177,  298,  261,  262,
  181,  182,  303,   44,  317,   58,  319,  320,  321,  276,
  277,  278,  260,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  173,  264,  265,    0,  279,  280,  281,  282,
  283,  341,  285,  286,  287,  309,  310,  257,  291,  292,
  293,  310,  311,   58,  263,  302,  303,   44,  199,  264,
  265,  170,  171,  363,  322,   58,  257,  323,   58,   58,
   58,  297,    0,   58,   44,   44,   58,   58,   58,  257,
  257,   40,    0,  290,  259,   58,   58,   58,  258,  258,
   58,  258,   58,   40,   58,   58,   58,   58,   58,   44,
   58,   58,   58,   58,   58,   58,   58,   58,   58,   44,
   44,   44,   58,   44,   58,   58,   58,   58,   58,   58,
  288,   40,  259,   44,  265,  258,  267,    0,   44,   44,
   44,   44,  261,  262,   44,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  346,  303,  303,  303,
  279,  280,  281,  282,  283,  284,  285,  286,  287,  303,
  289,  290,  291,  292,  293,  294,  295,   91,  297,  261,
  262,   91,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,    0,   91,  314,  315,  279,  280,  281,
  282,  283,  284,  285,  286,  287,   44,  289,  290,  291,
  292,  293,  294,  295,   44,  297,  303,  303,   44,  306,
  306,  288,  261,  262,  355,  264,  265,  261,  262,  261,
  262,   44,  314,  315,    0,  321,  303,   40,   44,  306,
   44,  261,  262,  261,  262,  376,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  261,  262,    0,
  258,  279,  280,  281,  282,  283,  284,  285,  286,  287,
  290,  289,  290,  291,  292,  293,  294,  295,  302,  297,
   44,   44,  413,   44,   44,    0,  290,   44,   44,  258,
    0,   44,  258,  297,  258,  302,  314,  315,   44,  313,
   44,  258,   44,  313,   44,   44,  261,  262,  439,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  258,   44,  453,    0,  279,  280,  281,  282,  283,  284,
  285,  286,  287,    0,  289,  290,  291,  292,  293,  294,
  295,    0,  297,  261,  262,  258,   44,   41,  266,  267,
  268,  269,  270,  261,  262,   93,   93,   44,   44,  314,
  315,  279,   93,  281,   44,  258,  284,   91,   44,   44,
   44,  289,  290,  291,  292,  293,  294,  295,  258,  297,
   44,  258,  290,   44,  258,  302,   44,   44,   44,  297,
   44,  289,    0,   44,  258,   44,  314,  315,  261,  262,
  258,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,   44,  276,  277,  278,  279,  280,  281,  282,
  283,  284,  285,  286,  287,    0,   44,  290,  291,  292,
  293,   44,   44,   58,  297,  258,   44,   44,   93,   44,
  313,   44,  258,   44,  258,   44,  258,   44,   44,  258,
   44,  314,  313,  275,  261,  262,  258,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,    0,  276,
  277,  278,  279,  280,  281,  282,  283,  284,  285,  286,
  287,   41,   44,  290,  291,  292,  293,   41,   44,   41,
  297,   44,   41,  258,  260,  261,  262,  263,  264,  265,
   44,    0,   44,   44,  258,   44,   44,  314,  259,  258,
  276,  277,   41,   41,  259,   41,    0,   44,  284,  260,
  261,  262,  263,   44,  290,    0,  381,  439,  427,  431,
  417,  297,    5,   34,    0,  276,  277,   14,  172,  289,
    7,   59,   40,  284,   61,  260,  261,  262,  314,  290,
  410,  261,  262,   69,  264,  265,  297,   69,   69,   69,
   69,  276,  277,   69,   69,   69,  276,  277,  263,  284,
  449,  171,   -1,  314,  284,  290,   -1,   -1,   -1,   -1,
  290,   -1,  297,   -1,  261,  262,   -1,  297,   -1,   -1,
   -1,   -1,   -1,   -1,  261,  262,   -1,   -1,   -1,  314,
   -1,   -1,  261,  262,  314,   -1,   -1,  284,   -1,  276,
  277,   -1,   -1,  290,   -1,   -1,   -1,  284,   -1,   -1,
  297,   -1,   -1,  290,   -1,  284,   -1,   -1,   -1,   -1,
  297,  290,   -1,   -1,   -1,   -1,   -1,  314,  297,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  314,   -1,   -1,
   -1,   -1,   -1,  261,  262,  314,  264,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,   -1,  276,  277,
  278,  279,  280,  281,  282,  283,   -1,  285,  286,  287,
   -1,   -1,   -1,  291,  292,  293,  261,  262,   -1,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
   -1,   -1,   -1,   -1,  279,  280,  281,  282,  283,   -1,
  285,  286,  287,   -1,   -1,   -1,  291,  292,  293,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  261,
  262,   -1,  264,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,   -1,   -1,   -1,   -1,  279,  280,  281,
  282,  283,   -1,  285,  286,  287,   -1,   -1,   -1,  291,
  292,  293,  261,  262,   -1,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,   -1,   -1,   -1,   -1,
  279,  280,  281,  282,  283,   -1,  285,  286,  287,   -1,
   -1,   -1,  291,  292,  293,  261,  262,   -1,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,   -1,
   -1,   -1,   -1,  279,  280,  281,  282,  283,   -1,  285,
  286,  287,   -1,   -1,   -1,  291,  292,  293,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 323
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
"GEOMETRY_ID","NOMAP_ID","OBJECT_ID","COBJECT_ID","MONSTER_ID","TRAP_ID",
"DOOR_ID","DRAWBRIDGE_ID","MAZEWALK_ID","WALLIFY_ID","REGION_ID","FILLING",
"RANDOM_OBJECTS_ID","RANDOM_MONSTERS_ID","RANDOM_PLACES_ID","ALTAR_ID",
"LADDER_ID","STAIR_ID","NON_DIGGABLE_ID","NON_PASSWALL_ID","ROOM_ID",
"PORTAL_ID","TELEPRT_ID","BRANCH_ID","LEV","CHANCE_ID","CORRIDOR_ID","GOLD_ID",
"ENGRAVING_ID","FOUNTAIN_ID","POOL_ID","SINK_ID","NONE","RAND_CORRIDOR_ID",
"DOOR_STATE","LIGHT_STATE","CURSE_TYPE","ENGRAVING_TYPE","DIRECTION",
"RANDOM_TYPE","O_REGISTER","M_REGISTER","P_REGISTER","A_REGISTER","ALIGNMENT",
"LEFT_OR_RIGHT","CENTER","TOP_OR_BOT","ALTAR_TYPE","UP_OR_DOWN","SUBROOM_ID",
"NAME_ID","FLAGS_ID","FLAG_TYPE","MON_ATTITUDE","MON_ALERTNESS",
"MON_APPEARANCE","CONTAINED","STRING","MAP_ID",
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
"object_detail : OBJECT_ID ':' object_desc",
"object_detail : COBJECT_ID ':' object_desc",
"$$3 :",
"object_desc : object_c ',' o_name $$3 ',' object_where object_infos",
"object_where : coordinate",
"object_where : CONTAINED",
"object_infos :",
"object_infos : ',' curse_state ',' monster_id ',' enchantment optional_name",
"object_infos : ',' curse_state ',' enchantment optional_name",
"object_infos : ',' monster_id ',' enchantment optional_name",
"curse_state : RANDOM_TYPE",
"curse_state : CURSE_TYPE",
"monster_id : STRING",
"enchantment : RANDOM_TYPE",
"enchantment : INTEGER",
"optional_name :",
"optional_name : ',' NONE",
"optional_name : ',' STRING",
"door_detail : DOOR_ID ':' door_state ',' coordinate",
"trap_detail : TRAP_ID ':' trap_name ',' coordinate",
"trap_detail : TRAP_ID ':' trap_name ',' coordinate ',' trap_chance",
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
"trap_chance : CHANCE_ID ':' INTEGER",
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

/*lev_comp.y*/
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
			unsigned i;

			if (fatal_error > 0) {
				(void) fprintf(stderr,
				"%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				maze.flags = yyvsp[-3].i;
				(void) memcpy((genericptr_t)&(maze.init_lev),
						(genericptr_t)&(init_lev),
						sizeof(lev_init));
				maze.numpart = npart;
				maze.parts = NewTab(mazepart, npart);
				for(i=0;i<npart;i++)
				    maze.parts[i] = tmppart[i];
				if (!write_level_file(yyvsp[-4].map, (splev *)0, &maze)) {
					yyerror("Can't open output file!!");
					exit(EXIT_FAILURE);
				}
				npart = 0;
			}
			Free(yyvsp[-4].map);
		  }
break;
case 8:
{
			unsigned i;

			if (fatal_error > 0) {
			    (void) fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				special_lev.flags = (long) yyvsp[-5].i;
				(void) memcpy(
					(genericptr_t)&(special_lev.init_lev),
					(genericptr_t)&(init_lev),
					sizeof(lev_init));
				special_lev.nroom = nrooms;
				special_lev.rooms = NewTab(room, nrooms);
				for(i=0; i<nrooms; i++)
				    special_lev.rooms[i] = tmproom[i];
				special_lev.ncorr = ncorridor;
				special_lev.corrs = NewTab(corridor, ncorridor);
				for(i=0; i<ncorridor; i++)
				    special_lev.corrs[i] = tmpcor[i];
				if (check_subrooms()) {
				    if (!write_level_file(yyvsp[-6].map, &special_lev,
							  (specialmaze *)0)) {
					yyerror("Can't open output file!!");
					exit(EXIT_FAILURE);
				    }
				}
				free_rooms(&special_lev);
				nrooms = 0;
				ncorridor = 0;
			}
			Free(yyvsp[-6].map);
		  }
break;
case 9:
{
			if (index(yyvsp[0].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen(yyvsp[0].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[0].map;
			special_lev.nrmonst = special_lev.nrobjects = 0;
			n_mlist = n_olist = 0;
		  }
break;
case 10:
{
			/* in case we're processing multiple files,
			   explicitly clear any stale settings */
			(void) memset((genericptr_t) &init_lev, 0,
					sizeof init_lev);
			init_lev.init_present = FALSE;
			yyval.i = 0;
		  }
break;
case 11:
{
			init_lev.init_present = TRUE;
			init_lev.fg = what_map_char((char) yyvsp[-10].i);
			if (init_lev.fg == INVALID_TYPE)
			    yyerror("Invalid foreground type.");
			init_lev.bg = what_map_char((char) yyvsp[-8].i);
			if (init_lev.bg == INVALID_TYPE)
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

			i = (int) strlen(yyvsp[0].map) + 1;
			j = (int) strlen(tmpmessage);
			if (i + j > 255) {
			   yyerror("Message string too long (>256 characters)");
			} else {
			    if (j) tmpmessage[j++] = '\n';
			    (void) strncpy(tmpmessage+j, yyvsp[0].map, i - 1);
			    tmpmessage[j + i - 1] = 0;
			}
			Free(yyvsp[0].map);
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
			if (ncorridor >= MAX_OF_TYPE) {
				yyerror("Too many corridors in level!");
				ncorridor--;
			}
		  }
break;
case 35:
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = -1;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].i;
			ncorridor++;
			if (ncorridor >= MAX_OF_TYPE) {
				yyerror("Too many corridors in level!");
				ncorridor--;
			}
		  }
break;
case 36:
{
			if ((unsigned) yyvsp[-5].i >= nrooms)
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
			tmproom[nrooms]->parent = yyvsp[-1].map;
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
			    tmproom[nrooms]->name = yyvsp[0].map;
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
			    if (ndoor >= MAX_OF_TYPE) {
				    yyerror("Too many doors in room!");
				    ndoor--;
			    }
			}
		  }
break;
case 75:
{
			maze.filling = yyvsp[0].i;
			if (index(yyvsp[-2].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen(yyvsp[-2].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[-2].map;
			in_room = 0;
			n_plist = n_mlist = n_olist = 0;
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
			Free(yyvsp[0].map);
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
			tmpmonst[nmons]->name.str = 0;
			tmpmonst[nmons]->appear = 0;
			tmpmonst[nmons]->appear_as.str = 0;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Monster");
			if (!yyvsp[-2].map)
			    tmpmonst[nmons]->id = NON_PM;
			else {
				int token = get_monster_id(yyvsp[-2].map, (char) yyvsp[-4].i);
				if (token == ERR) {
				    yywarning(
			      "Invalid monster name!  Making random monster.");
				    tmpmonst[nmons]->id = NON_PM;
				} else
				    tmpmonst[nmons]->id = token;
				Free(yyvsp[-2].map);
			}
		  }
break;
case 123:
{
			nmons++;
			if (nmons >= MAX_OF_TYPE) {
			    yyerror("Too many monsters in room or mazepart!");
			    nmons--;
			}
		  }
break;
case 126:
{
			tmpmonst[nmons]->name.str = yyvsp[0].map;
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
			tmpmonst[nmons]->appear_as.str = yyvsp[0].map;
		  }
break;
case 131:
{
		  }
break;
case 132:
{
			/* 1: is contents of next object with 2 */
			/* 2: is a container */
			/* 0: neither */
			tmpobj[nobj-1]->containment = 2;
		  }
break;
case 133:
{
			tmpobj[nobj] = New(object);
			tmpobj[nobj]->class = yyvsp[-2].i;
			tmpobj[nobj]->corpsenm = NON_PM;
			tmpobj[nobj]->curse_state = -1;
			tmpobj[nobj]->name.str = 0;
			if (!yyvsp[0].map)
			    tmpobj[nobj]->id = -1;
			else {
				int token = get_object_id(yyvsp[0].map);
				if (token == ERR) {
				    yywarning("Illegal object name!  Making random object.");
				    tmpobj[nobj]->id = -1;
				} else
				    tmpobj[nobj]->id = token;
				Free(yyvsp[0].map);
			}
		  }
break;
case 134:
{
			nobj++;
			if (nobj >= MAX_OF_TYPE) {
				yyerror("Too many objects in room or mazepart!");
				nobj--;
			}
		  }
break;
case 135:
{
			tmpobj[nobj]->containment = 0;
			tmpobj[nobj]->x = current_coord.x;
			tmpobj[nobj]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Object");
		  }
break;
case 136:
{
			tmpobj[nobj]->containment = 1;
			/* random coordinate, will be overridden anyway */
			tmpobj[nobj]->x = -MAX_REGISTERS-1;
			tmpobj[nobj]->y = -MAX_REGISTERS-1;
		  }
break;
case 137:
{
			tmpobj[nobj]->spe = -127;
	/* Note below: we're trying to make as many of these optional as
	 * possible.  We clearly can't make curse_state, enchantment, and
	 * monster_id _all_ optional, since ",random" would be ambiguous.
	 * We can't even just make enchantment mandatory, since if we do that
	 * alone, ",random" requires too much lookahead to parse.
	 */
		  }
break;
case 138:
{
		  }
break;
case 139:
{
		  }
break;
case 140:
{
		  }
break;
case 141:
{
			tmpobj[nobj]->curse_state = -1;
		  }
break;
case 142:
{
			tmpobj[nobj]->curse_state = yyvsp[0].i;
		  }
break;
case 143:
{
			int token = get_monster_id(yyvsp[0].map, (char)0);
			if (token == ERR)	/* "random" */
			    tmpobj[nobj]->corpsenm = NON_PM - 1;
			else
			    tmpobj[nobj]->corpsenm = token;
			Free(yyvsp[0].map);
		  }
break;
case 144:
{
			tmpobj[nobj]->spe = -127;
		  }
break;
case 145:
{
			tmpobj[nobj]->spe = yyvsp[0].i;
		  }
break;
case 147:
{
		  }
break;
case 148:
{
			tmpobj[nobj]->name.str = yyvsp[0].map;
		  }
break;
case 149:
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
			if (ndoor >= MAX_OF_TYPE) {
				yyerror("Too many doors in mazepart!");
				ndoor--;
			}
		  }
break;
case 150:
{
			tmptrap[ntrap] = New(trap);
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yyvsp[-2].i;
			tmptrap[ntrap]->chance = 100;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Trap");
			ntrap++;
			if (ntrap >= MAX_OF_TYPE) {
				yyerror("Too many traps in room or mazepart!");
				ntrap--;
			}
		  }
break;
case 151:
{
			tmptrap[ntrap] = New(trap);
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yyvsp[-4].i;
			tmptrap[ntrap]->chance = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Trap");
			ntrap++;
			if (ntrap >= MAX_OF_TYPE) {
				yyerror("Too many traps in room or mazepart!");
				ntrap--;
			}
		  }
break;
case 152:
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
			if (ndb >= MAX_OF_TYPE) {
				yyerror("Too many drawbridges in mazepart!");
				ndb--;
			}
		   }
break;
case 153:
{
			tmpwalk[nwalk] = New(walk);
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = yyvsp[0].i;
			nwalk++;
			if (nwalk >= MAX_OF_TYPE) {
				yyerror("Too many mazewalks in mazepart!");
				nwalk--;
			}
		  }
break;
case 154:
{
			wallify_map();
		  }
break;
case 155:
{
			tmplad[nlad] = New(lad);
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Ladder");
			nlad++;
			if (nlad >= MAX_OF_TYPE) {
				yyerror("Too many ladders in mazepart!");
				nlad--;
			}
		  }
break;
case 156:
{
			tmpstair[nstair] = New(stair);
			tmpstair[nstair]->x = current_coord.x;
			tmpstair[nstair]->y = current_coord.y;
			tmpstair[nstair]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Stairway");
			nstair++;
			if (nstair >= MAX_OF_TYPE) {
				yyerror("Too many stairs in room or mazepart!");
				nstair--;
			}
		  }
break;
case 157:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 158:
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
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 159:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 160:
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_PORTAL;
			tmplreg[nlreg]->rname.str = yyvsp[0].map;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 161:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 162:
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
		  }
break;
case 163:
{
			switch(yyvsp[0].i) {
			case -1: tmplreg[nlreg]->rtype = LR_TELE; break;
			case 0: tmplreg[nlreg]->rtype = LR_DOWNTELE; break;
			case 1: tmplreg[nlreg]->rtype = LR_UPTELE; break;
			}
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 164:
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 165:
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_BRANCH;
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 166:
{
			yyval.i = -1;
		  }
break;
case 167:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 168:
{
			yyval.i = 0;
		  }
break;
case 169:
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
case 170:
{
			tmpfountain[nfountain] = New(fountain);
			tmpfountain[nfountain]->x = current_coord.x;
			tmpfountain[nfountain]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Fountain");
			nfountain++;
			if (nfountain >= MAX_OF_TYPE) {
			    yyerror("Too many fountains in room or mazepart!");
			    nfountain--;
			}
		  }
break;
case 171:
{
			tmpsink[nsink] = New(sink);
			tmpsink[nsink]->x = current_coord.x;
			tmpsink[nsink]->y = current_coord.y;
			nsink++;
			if (nsink >= MAX_OF_TYPE) {
				yyerror("Too many sinks in room!");
				nsink--;
			}
		  }
break;
case 172:
{
			tmppool[npool] = New(pool);
			tmppool[npool]->x = current_coord.x;
			tmppool[npool]->y = current_coord.y;
			npool++;
			if (npool >= MAX_OF_TYPE) {
				yyerror("Too many pools in room!");
				npool--;
			}
		  }
break;
case 173:
{
			tmpdig[ndig] = New(digpos);
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
			if (ndig >= MAX_OF_TYPE) {
				yyerror("Too many diggables in mazepart!");
				ndig--;
			}
		  }
break;
case 174:
{
			tmppass[npass] = New(digpos);
			tmppass[npass]->x1 = current_region.x1;
			tmppass[npass]->y1 = current_region.y1;
			tmppass[npass]->x2 = current_region.x2;
			tmppass[npass]->y2 = current_region.y2;
			npass++;
			if (npass >= 32) {
				yyerror("Too many passwalls in mazepart!");
				npass--;
			}
		  }
break;
case 175:
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
			   current_region.x2 < (int)max_x_map &&
			   current_region.y2 < (int)max_y_map) {
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
			if (nreg >= MAX_OF_TYPE) {
				yyerror("Too many regions in mazepart!");
				nreg--;
			}
		  }
break;
case 176:
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
			if (naltar >= MAX_OF_TYPE) {
				yyerror("Too many altars in room or mazepart!");
				naltar--;
			}
		  }
break;
case 177:
{
			tmpgold[ngold] = New(gold);
			tmpgold[ngold]->x = current_coord.x;
			tmpgold[ngold]->y = current_coord.y;
			tmpgold[ngold]->amount = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Gold");
			ngold++;
			if (ngold >= MAX_OF_TYPE) {
				yyerror("Too many golds in room or mazepart!");
				ngold--;
			}
		  }
break;
case 178:
{
			tmpengraving[nengraving] = New(engraving);
			tmpengraving[nengraving]->x = current_coord.x;
			tmpengraving[nengraving]->y = current_coord.y;
			tmpengraving[nengraving]->engr.str = yyvsp[0].map;
			tmpengraving[nengraving]->etype = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Engraving");
			nengraving++;
			if (nengraving >= MAX_OF_TYPE) {
			    yyerror("Too many engravings in room or mazepart!");
			    nengraving--;
			}
		  }
break;
case 180:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 183:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 186:
{
			yyval.map = (char *) 0;
		  }
break;
case 188:
{
			yyval.map = (char *) 0;
		  }
break;
case 189:
{
			int token = get_trap_type(yyvsp[0].map);
			if (token == ERR)
				yyerror("Unknown trap type!");
			yyval.i = token;
			Free(yyvsp[0].map);
		  }
break;
case 191:
{
			if (tmptrap[ntrap]->chance)
			    yyerror("This trap already assigned a chance!");
			else if (yyvsp[0].i < 1 || yyvsp[0].i > 99)
			    yyerror("The chance is supposed to be percentile.");
			else
			    tmptrap[ntrap]->chance = yyvsp[0].i;
		   }
break;
case 192:
{
			int token = get_room_type(yyvsp[0].map);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				yyval.i = OROOM;
			} else
				yyval.i = token;
			Free(yyvsp[0].map);
		  }
break;
case 194:
{
			yyval.i = 0;
		  }
break;
case 195:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 196:
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i << 1);
		  }
break;
case 199:
{
			current_coord.x = current_coord.y = -MAX_REGISTERS-1;
		  }
break;
case 206:
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 209:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				current_coord.x = current_coord.y = - yyvsp[-1].i - 1;
		  }
break;
case 210:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 211:
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 212:
{
			if ( yyvsp[-1].i >= 3 )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 214:
{
			if (check_monster_char((char) yyvsp[0].i))
				yyval.i = yyvsp[0].i ;
			else {
				yyerror("Unknown monster class!");
				yyval.i = ERR;
			}
		  }
break;
case 215:
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
case 221:
{
			if (!in_room && !init_lev.init_present &&
			    (yyvsp[-3].i < 0 || yyvsp[-3].i > (int)max_x_map ||
			     yyvsp[-1].i < 0 || yyvsp[-1].i > (int)max_y_map))
			    yyerror("Coordinates out of map range!");
			current_coord.x = yyvsp[-3].i;
			current_coord.y = yyvsp[-1].i;
		  }
break;
case 222:
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i < 0 || yyvsp[-7].i > (int)max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i > (int)max_y_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-3].i < 0 || yyvsp[-3].i > (int)max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i > (int)max_y_map)
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
