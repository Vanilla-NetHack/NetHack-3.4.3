extern char *malloc(), *realloc();

# line 2 "lev_comp.y"
/*	SCCS Id: @(#)lev_comp.c	3.1	93/02/13	*/
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

extern void FDECL(yyerror, (char *));
extern void FDECL(yywarning, (char *));
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
extern void FDECL(check_coord, (int, int, char *));
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
extern char* fname;


# line 144 "lev_comp.y"
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
# define CHAR 257
# define INTEGER 258
# define BOOLEAN 259
# define MESSAGE_ID 260
# define MAZE_ID 261
# define LEVEL_ID 262
# define LEV_INIT_ID 263
# define GEOMETRY_ID 264
# define NOMAP_ID 265
# define OBJECT_ID 266
# define MONSTER_ID 267
# define TRAP_ID 268
# define DOOR_ID 269
# define DRAWBRIDGE_ID 270
# define MAZEWALK_ID 271
# define WALLIFY_ID 272
# define REGION_ID 273
# define FILLING 274
# define RANDOM_OBJECTS_ID 275
# define RANDOM_MONSTERS_ID 276
# define RANDOM_PLACES_ID 277
# define ALTAR_ID 278
# define LADDER_ID 279
# define STAIR_ID 280
# define NON_DIGGABLE_ID 281
# define NON_PASSWALL_ID 282
# define ROOM_ID 283
# define PORTAL_ID 284
# define TELEPRT_ID 285
# define BRANCH_ID 286
# define LEV 287
# define CHANCE_ID 288
# define CORRIDOR_ID 289
# define GOLD_ID 290
# define ENGRAVING_ID 291
# define FOUNTAIN_ID 292
# define POOL_ID 293
# define SINK_ID 294
# define NONE 295
# define RAND_CORRIDOR_ID 296
# define DOOR_STATE 297
# define LIGHT_STATE 298
# define CURSE_TYPE 299
# define ENGRAVING_TYPE 300
# define DIRECTION 301
# define RANDOM_TYPE 302
# define O_REGISTER 303
# define M_REGISTER 304
# define P_REGISTER 305
# define A_REGISTER 306
# define ALIGNMENT 307
# define LEFT_OR_RIGHT 308
# define CENTER 309
# define TOP_OR_BOT 310
# define ALTAR_TYPE 311
# define UP_OR_DOWN 312
# define SUBROOM_ID 313
# define NAME_ID 314
# define FLAGS_ID 315
# define FLAG_TYPE 316
# define MON_ATTITUDE 317
# define MON_ALERTNESS 318
# define MON_APPEARANCE 319
# define STRING 320
# define MAP_ID 321
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 1542 "lev_comp.y"

int yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 179,
	44, 98,
	-2, 97,
	};
# define YYNPROD 214
# define YYLAST 487
int yyact[]={

   156,   451,   308,   432,   183,   380,    67,   398,   221,   155,
   208,   253,   459,   311,    47,    20,    22,   312,   309,    21,
   104,   103,   106,   151,    49,   157,    28,    12,   438,   439,
   441,    21,   113,   428,   154,   293,   427,   458,   289,   213,
    60,   414,   150,    21,   118,   119,   114,   153,   152,   222,
    55,    56,   389,    21,   425,   368,    21,    21,    60,   314,
   301,   185,   184,   225,   367,   181,    68,    69,   149,   311,
    61,   452,   134,   312,   309,   131,   381,   377,   399,   234,
   104,   103,   106,   105,   107,   115,   116,   108,    61,   396,
   249,   158,   113,   117,   109,   120,   121,   330,   110,   111,
   112,   374,   375,   209,   118,   119,   114,   254,   210,   327,
   317,   255,   318,    43,   128,   453,   220,   200,   351,   202,
   205,   207,   400,   235,   313,   297,   411,   242,    74,    36,
    34,   215,   397,   250,   178,    64,    66,    65,    17,     8,
     9,    25,   447,   444,   243,   281,   186,   446,   436,   430,
   413,   407,   406,   404,   388,   369,   363,   356,    44,   354,
   342,   339,   320,   212,   303,   298,   294,   290,   283,   270,
   266,   247,   214,   240,   134,   131,    70,    39,   229,   230,
   231,   232,   132,   236,   130,   129,   133,   394,   337,   335,
   333,   246,   347,   223,   264,   195,   260,   258,   148,   147,
   146,   143,   251,   252,   142,   141,   140,   194,   193,   256,
   139,   192,    78,   191,   190,   187,   226,   227,   228,   176,
   175,   174,   173,   172,   171,   170,   169,   168,   167,   166,
   165,   164,   163,   162,   161,   160,   159,   124,   123,   122,
    81,    80,    76,    75,    48,    38,    26,    18,    15,    14,
   349,    54,   456,   361,   443,   180,   442,   433,   288,   431,
   292,    98,   100,    99,    94,   429,   424,    93,    86,    84,
   295,   296,    79,    83,   421,   418,   412,   299,   410,   405,
   402,   401,   393,   390,   387,   383,   315,   379,   372,   371,
   364,   362,   361,   355,   353,   325,   352,   348,   346,   341,
   338,   336,   334,   332,   324,   321,   307,   182,   177,    77,
   224,   306,   305,   304,   302,   300,   286,   285,   284,   282,
   280,   279,   278,   277,   276,   217,   275,   217,   218,   274,
   218,   267,   265,   350,   263,   262,   261,   259,   382,   378,
   257,   357,   241,   358,   197,   204,   199,   359,   360,   370,
   196,   189,   340,   188,   126,   343,   344,   345,   219,   331,
   125,    50,   323,   224,    40,   391,    30,   455,   449,   237,
   238,   328,   245,   448,   244,   445,   422,   419,   415,   349,
    27,   181,   269,   243,   403,   220,    31,    23,    16,    11,
     2,   216,   273,   392,    10,   365,   272,    13,   271,   268,
   409,   385,    19,   423,   408,   384,   420,    29,   417,   416,
   322,   180,    37,   102,   319,   101,    97,    96,    95,    45,
    92,    51,    91,   434,   435,   437,    90,   440,    89,    88,
    87,    85,    82,   239,   179,    63,    35,    62,    46,    33,
    32,   145,   450,   144,   138,   454,   137,   136,   135,   376,
   326,   329,    59,    58,   127,    73,    72,    57,    53,    24,
    71,    52,    41,     5,     4,     3,     1,   457,   291,   287,
     7,     6,   206,   203,   201,   198,   386,   316,   233,   426,
   248,   395,   373,    42,   310,   366,   211 };
int yypact[]={

  -122, -1000, -1000,  -122, -1000, -1000,  -288,  -288,   191,   190,
 -1000,  -125,   189,  -125,  -301,  -301,  -119,   188,  -290,  -119,
   322, -1000, -1000,  -135,  -119,   187,   -80, -1000,   320, -1000,
  -144, -1000,  -135, -1000, -1000,  -307,   186, -1000,  -296,   317,
  -290,  -225, -1000, -1000, -1000, -1000,  -140, -1000,  -242, -1000,
   -81, -1000,  -168, -1000, -1000,   185,   184,  -243, -1000, -1000,
   183,   182,  -186, -1000,   181,   180,   179,   316, -1000, -1000,
   310, -1000, -1000,  -175, -1000,   -82,   -83, -1000,  -246,  -246,
  -277,  -277, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000,   178,   177,   176,   175,   174,   173,   172,
   171,   170,   169,   168,   167,   166, -1000,   165,   164,   163,
   162,   161,   -82,   341,   -83,  -248,  -113, -1000,   157, -1000,
   309, -1000, -1000,   307, -1000, -1000, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,   156,
   155,   153,   150,   149,   137,   306, -1000, -1000,   300,  -185,
  -182,  -194,  -263,    25,   345,    23,    76,    76,    76,    25,
    25,    25,    25,  -179,    25,   345,   345, -1000, -1000, -1000,
 -1000,   -85, -1000, -1000, -1000, -1000,   298,   343,   -82,   -83,
  -301,   -87,  -169,    25,    25,    25,  -191,  -191,   296, -1000,
 -1000, -1000,   106,   293, -1000, -1000, -1000,   105,   292, -1000,
 -1000,   291, -1000, -1000,   290, -1000, -1000, -1000,   103,   288,
   -88,   287, -1000, -1000,   342,   -89, -1000, -1000, -1000,   285,
 -1000,   282,   280,   279, -1000, -1000,   278, -1000, -1000,   277,
   276,  -114,   275,   -90, -1000, -1000, -1000, -1000,   274, -1000,
 -1000, -1000, -1000,   273, -1000, -1000,   272,  -264,   -91,  -267,
   -92,    25,    25,  -176,   -93,  -191,   271,  -252,   270,   -94,
   269,   268,   267,   262,  -233,  -177,  -253,    25,  -190,   341,
   -96,   261,   104,   260,  -194,    69,    57,   259, -1000, -1000,
    97,   258, -1000, -1000,    96, -1000, -1000,   257,    95,   256,
   -97, -1000,    76,   255,   -98,    76,    76,    76,   254, -1000,
 -1000, -1000,   101, -1000, -1000, -1000,   253, -1000, -1000, -1000,
   338,  -191, -1000, -1000,  -183,   252,   250,   -99, -1000,   249,
  -101, -1000,    25, -1000,    25, -1000,  -194, -1000,  -277,   248,
   247,  -102,   209,   246, -1000, -1000,  -247,  -103,  -301, -1000,
   245,   244,  -200,    37,   243,    36,   241, -1000, -1000, -1000,
   240,  -104,  -260,   239,  -301,   238, -1000, -1000, -1000,    94,
 -1000,  -170,  -180,   237, -1000, -1000,   236,  -242, -1000,  -105,
   235,  -106, -1000,  -107, -1000,   234, -1000,  -148,   232, -1000,
  -108, -1000, -1000,  -271, -1000, -1000, -1000, -1000,   337, -1000,
 -1000,  -180,    36,   231,   336,  -301,   230,   335,   222, -1000,
  -266,   221,  -109,   215, -1000, -1000, -1000,   213,  -248, -1000,
   213,  -110, -1000, -1000,  -289,   212,   210, -1000, -1000,  -116,
   334,  -111, -1000,  -117,   332, -1000,   327, -1000, -1000, -1000,
 -1000,  -301,  -187,  -187, -1000, -1000,   326, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000,   208, -1000,  -283, -1000, -1000, -1000 };
int yypgo[]={

     0,     6,     4,   486,     9,    10,    11,     2,   485,   484,
     3,   483,     7,   482,   481,   480,   479,     1,   478,   477,
   389,   380,   476,    49,   388,   186,   475,   474,   184,   473,
   472,     0,   471,   470,   469,   468,   467,   127,   466,   390,
   465,   464,   463,   387,   386,   462,   461,   460,   459,   458,
   185,   182,   251,   457,   456,   455,   454,   453,   212,   452,
   451,     5,   450,   449,   448,   447,   446,   444,   210,   206,
   205,   204,   201,   443,   441,   200,   199,   198,   440,   439,
   438,   437,   436,   435,   134,   434,   433,   432,   431,   430,
   429,   428,   426,   422,   420,   418,   417,   416,   415,   413,
     8,   405,   404,   403,   401,   400,   399,   398,   396,   395,
   393,   392,   193,   131,   391 };
int yyr1[]={

     0,    38,    38,    39,    39,    40,    40,    41,    42,    33,
    24,    24,    14,    14,    20,    20,    21,    21,    43,    43,
    48,    45,    45,    49,    49,    46,    46,    52,    52,    47,
    47,    54,    55,    55,    56,    56,    37,    53,    53,    59,
    57,    10,    10,    62,    62,    60,    60,    63,    63,    61,
    61,    58,    58,    64,    64,    64,    64,    64,    64,    64,
    64,    64,    64,    64,    64,    64,    65,    66,    67,    15,
    15,    13,    13,    12,    12,    32,    11,    11,    44,    44,
    78,    79,    79,    82,     1,     1,     2,     2,    80,    80,
    83,    83,    83,    50,    50,    51,    51,    84,    86,    84,
    81,    81,    87,    87,    87,    87,    87,    87,    87,    87,
    87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
    87,    87,   101,    68,   102,   102,   103,   103,   103,   103,
   103,   104,    69,   105,   105,   105,    16,    16,    17,    17,
    88,    70,    89,    95,    96,    97,    77,   106,    91,   107,
    92,   108,   109,    93,   111,    94,   110,   110,    23,    23,
    72,    73,    74,    98,    99,    90,    71,    75,    76,    26,
    26,    26,    29,    29,    29,    34,    34,    35,    35,     3,
     3,     4,     4,    22,    22,    22,   100,   100,   100,     5,
     5,     6,     6,     7,     7,     7,     8,     8,   114,    30,
    27,     9,    85,    25,    28,    31,    36,    36,    18,    18,
    19,    19,   113,   112 };
int yyr2[]={

     0,     0,     2,     2,     4,     2,     2,    11,    15,     7,
     1,    27,     2,     2,     1,     7,     7,     3,     0,     4,
     7,     0,     4,     7,     7,     1,     2,     2,     4,     2,
     2,     3,     0,     4,    11,    11,    15,     5,     5,    25,
    25,     1,     5,    11,     3,    11,     3,    11,     3,    11,
     3,     0,     4,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     7,     7,    19,     2,
     2,     2,     2,     2,     2,    11,     3,     3,     2,     4,
     7,     3,     5,    11,     2,     2,     2,     2,     0,     4,
     7,     7,     7,     3,     7,     3,     7,     3,     1,     8,
     0,     4,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     1,    19,     0,     4,     5,     5,     5,     5,
     7,     1,    19,     1,     9,    13,     2,     2,     2,     3,
    11,    11,    15,    11,     3,    11,    11,     1,    17,     1,
    17,     1,     1,    17,     1,    13,     1,     5,     3,    21,
     7,     7,     7,     7,     7,    17,    15,    11,    15,     2,
     3,     2,     2,     3,     2,     2,     3,     2,     3,     3,
     2,     3,     2,     1,     5,     9,     2,     2,     3,     2,
     2,     2,     2,     2,     2,     3,     2,     2,     9,     9,
     9,     9,     2,     3,     3,     2,     2,     3,     2,     2,
     2,     2,    11,    19 };
int yychk[]={

 -1000,   -38,   -39,   -40,   -41,   -42,   -32,   -33,   261,   262,
   -39,   -20,   315,   -20,    58,    58,   -24,   263,    58,   -24,
   -31,   320,   -31,   -43,   -48,   260,    58,   -21,   316,   -43,
    44,   -44,   -78,   -79,   265,   -82,   264,   -43,    58,   257,
    44,   -45,   -11,   257,   302,   -44,   -80,   321,    58,   320,
    44,   -21,   -46,   -49,   -52,   275,   276,   -53,   -57,   -59,
   283,   313,   -81,   -83,   275,   277,   276,    -1,   308,   309,
   257,   -47,   -54,   -55,   296,    58,    58,   -52,   -58,   -58,
    58,    58,   -87,   -68,   -69,   -88,   -70,   -89,   -90,   -91,
   -92,   -93,   -94,   -71,   -72,   -95,   -96,   -97,   -77,   -75,
   -76,   -98,   -99,   267,   266,   269,   268,   270,   273,   280,
   284,   285,   286,   278,   292,   271,   272,   279,   290,   291,
   281,   282,    58,    58,    58,    44,    44,   -56,   289,   -50,
   -28,   257,   -51,   -25,   257,   -64,   -65,   -66,   -67,   -68,
   -69,   -70,   -71,   -72,   -73,   -74,   -75,   -76,   -77,   314,
   288,   269,   294,   293,   280,    -4,   -31,   302,    -4,    58,
    58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
    58,    58,    58,    58,    58,    58,    58,   -50,   -84,   -85,
  -113,    40,   -51,    -2,   310,   309,   259,    58,    44,    44,
    58,    58,    58,    58,    58,    58,    44,    44,   -26,   -25,
   302,   -27,   304,   -29,   -28,   302,   -30,   303,    -5,   297,
   302,    -3,   -31,   302,  -100,  -113,  -114,   302,   305,  -112,
    40,  -100,   -23,  -112,   287,    40,   -23,   -23,   -23,  -100,
  -100,  -100,  -100,   -18,   258,   302,  -100,  -112,  -112,   -86,
   258,    44,   -37,    40,   -50,   -51,   -31,   258,   -15,   259,
   302,  -100,  -100,    -6,   298,   302,    -6,    44,    91,    44,
    91,    44,    44,    44,    91,    44,   258,    44,  -106,    40,
   258,  -107,  -108,  -111,    44,    44,    44,    44,    44,    44,
    44,   259,    44,   258,    44,    44,    44,   -34,   -31,   302,
   258,   -35,   -31,   302,   258,  -100,  -100,   301,   258,    -6,
    44,   312,    44,   258,    44,    44,    44,    44,    -7,   307,
    -9,   302,   306,   301,   312,  -100,   -19,   300,   302,   -84,
   258,    44,   -37,   258,    44,    -5,   -62,    40,   302,   -60,
    40,   302,    44,    93,    44,    93,    44,    93,    44,   258,
   -23,    44,   258,   -23,   -23,   -23,    44,    91,    44,    41,
    -6,   301,    44,    44,   258,    44,   258,  -100,  -100,    -5,
    -4,    44,    44,   258,    44,  -109,    -8,   311,   302,   258,
   -31,    44,    44,   -13,   301,   302,   -63,    40,   302,    44,
   -61,    40,   302,    44,  -101,  -104,   -22,    44,   258,   312,
    44,   -31,  -110,    44,    93,   -14,   259,   302,   -12,   258,
   302,    44,    44,    -1,   258,    44,   258,   258,  -102,  -105,
    44,   274,    44,   258,   312,    41,   -12,   -61,    44,    41,
   -31,    44,    41,  -103,    44,   320,   -16,   302,   299,    44,
   258,    44,   -10,    44,    -2,   -10,   258,   -31,   317,   318,
    -7,   319,    44,    44,   259,    41,   258,   259,    41,    41,
   -31,   -17,   258,   302,   -17,    41,    44,   -36,   320,   295 };
int yydef[]={

     1,    -2,     2,     3,     5,     6,    14,    14,     0,     0,
     4,    10,     0,    10,     0,     0,    18,     0,     0,    18,
     0,   205,     9,     0,    18,     0,     0,    15,    17,    21,
     0,     7,    78,    88,    81,     0,     0,    19,     0,     0,
     0,    25,    75,    76,    77,    79,   100,    82,     0,    20,
     0,    16,    32,    22,    26,     0,     0,    27,    51,    51,
     0,     0,    80,    89,     0,     0,     0,     0,    84,    85,
     0,     8,    29,    30,    31,     0,     0,    28,    37,    38,
     0,     0,   101,   102,   103,   104,   105,   106,   107,   108,
   109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
   119,   120,   121,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   144,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    33,     0,    23,
    93,   204,    24,    95,   203,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64,    65,     0,
     0,     0,     0,     0,     0,     0,   181,   182,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    90,    91,    -2,
   202,     0,    92,    83,    86,    87,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   169,
   170,   171,     0,     0,   172,   173,   174,     0,     0,   189,
   190,     0,   179,   180,     0,   186,   187,   188,     0,     0,
     0,     0,   147,   158,     0,     0,   149,   151,   154,     0,
   160,     0,     0,     0,   208,   209,     0,   163,   164,     0,
     0,     0,     0,     0,    94,    96,    66,    67,     0,    69,
    70,   161,   162,     0,   191,   192,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,   175,   176,
     0,     0,   177,   178,     0,   140,   141,     0,     0,     0,
     0,   146,     0,     0,     0,     0,     0,     0,     0,   193,
   194,   195,     0,   143,   145,   167,     0,   210,   211,    99,
     0,     0,    34,    35,     0,     0,     0,     0,    44,     0,
     0,    46,     0,   200,     0,   199,     0,   198,     0,     0,
     0,     0,     0,     0,   152,   155,     0,     0,     0,   212,
     0,     0,     0,     0,     0,     0,     0,   122,   131,   142,
   183,     0,     0,     0,     0,   156,   166,   196,   197,     0,
   168,     0,     0,     0,    71,    72,     0,     0,    48,     0,
     0,     0,    50,     0,   124,   133,   165,     0,     0,   148,
     0,   150,   153,     0,   201,    11,    12,    13,     0,    73,
    74,     0,     0,     0,     0,     0,     0,     0,   123,   132,
     0,   184,     0,     0,   157,    36,    68,    41,     0,    43,
    41,     0,    45,   125,     0,     0,     0,   136,   137,     0,
     0,     0,    40,     0,     0,    39,     0,   126,   127,   128,
   129,     0,     0,     0,   185,   213,     0,    42,    47,    49,
   130,   134,   138,   139,     0,   159,     0,   135,   206,   207 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"CHAR",	257,
	"INTEGER",	258,
	"BOOLEAN",	259,
	"MESSAGE_ID",	260,
	"MAZE_ID",	261,
	"LEVEL_ID",	262,
	"LEV_INIT_ID",	263,
	"GEOMETRY_ID",	264,
	"NOMAP_ID",	265,
	"OBJECT_ID",	266,
	"MONSTER_ID",	267,
	"TRAP_ID",	268,
	"DOOR_ID",	269,
	"DRAWBRIDGE_ID",	270,
	"MAZEWALK_ID",	271,
	"WALLIFY_ID",	272,
	"REGION_ID",	273,
	"FILLING",	274,
	"RANDOM_OBJECTS_ID",	275,
	"RANDOM_MONSTERS_ID",	276,
	"RANDOM_PLACES_ID",	277,
	"ALTAR_ID",	278,
	"LADDER_ID",	279,
	"STAIR_ID",	280,
	"NON_DIGGABLE_ID",	281,
	"NON_PASSWALL_ID",	282,
	"ROOM_ID",	283,
	"PORTAL_ID",	284,
	"TELEPRT_ID",	285,
	"BRANCH_ID",	286,
	"LEV",	287,
	"CHANCE_ID",	288,
	"CORRIDOR_ID",	289,
	"GOLD_ID",	290,
	"ENGRAVING_ID",	291,
	"FOUNTAIN_ID",	292,
	"POOL_ID",	293,
	"SINK_ID",	294,
	"NONE",	295,
	"RAND_CORRIDOR_ID",	296,
	"DOOR_STATE",	297,
	"LIGHT_STATE",	298,
	"CURSE_TYPE",	299,
	"ENGRAVING_TYPE",	300,
	"DIRECTION",	301,
	"RANDOM_TYPE",	302,
	"O_REGISTER",	303,
	"M_REGISTER",	304,
	"P_REGISTER",	305,
	"A_REGISTER",	306,
	"ALIGNMENT",	307,
	"LEFT_OR_RIGHT",	308,
	"CENTER",	309,
	"TOP_OR_BOT",	310,
	"ALTAR_TYPE",	311,
	"UP_OR_DOWN",	312,
	"SUBROOM_ID",	313,
	"NAME_ID",	314,
	"FLAGS_ID",	315,
	"FLAG_TYPE",	316,
	"MON_ATTITUDE",	317,
	"MON_ALERTNESS",	318,
	"MON_APPEARANCE",	319,
	",",	44,
	":",	58,
	"(",	40,
	")",	41,
	"[",	91,
	"]",	93,
	"STRING",	320,
	"MAP_ID",	321,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"file : /* empty */",
	"file : levels",
	"levels : level",
	"levels : level levels",
	"level : maze_level",
	"level : room_level",
	"maze_level : maze_def flags lev_init messages regions",
	"room_level : level_def flags lev_init messages rreg_init rooms corridors_def",
	"level_def : LEVEL_ID ':' string",
	"lev_init : /* empty */",
	"lev_init : LEV_INIT_ID ':' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled",
	"walled : BOOLEAN",
	"walled : RANDOM_TYPE",
	"flags : /* empty */",
	"flags : FLAGS_ID ':' flag_list",
	"flag_list : FLAG_TYPE ',' flag_list",
	"flag_list : FLAG_TYPE",
	"messages : /* empty */",
	"messages : message messages",
	"message : MESSAGE_ID ':' STRING",
	"rreg_init : /* empty */",
	"rreg_init : rreg_init init_rreg",
	"init_rreg : RANDOM_OBJECTS_ID ':' object_list",
	"init_rreg : RANDOM_MONSTERS_ID ':' monster_list",
	"rooms : /* empty */",
	"rooms : roomlist",
	"roomlist : aroom",
	"roomlist : aroom roomlist",
	"corridors_def : random_corridors",
	"corridors_def : corridors",
	"random_corridors : RAND_CORRIDOR_ID",
	"corridors : /* empty */",
	"corridors : corridors corridor",
	"corridor : CORRIDOR_ID ':' corr_spec ',' corr_spec",
	"corridor : CORRIDOR_ID ':' corr_spec ',' INTEGER",
	"corr_spec : '(' INTEGER ',' DIRECTION ',' door_pos ')'",
	"aroom : room_def room_details",
	"aroom : subroom_def room_details",
	"subroom_def : SUBROOM_ID ':' room_type ',' light_state ',' subroom_pos ',' room_size ',' string roomfill",
	"room_def : ROOM_ID ':' room_type ',' light_state ',' room_pos ',' room_align ',' room_size roomfill",
	"roomfill : /* empty */",
	"roomfill : ',' BOOLEAN",
	"room_pos : '(' INTEGER ',' INTEGER ')'",
	"room_pos : RANDOM_TYPE",
	"subroom_pos : '(' INTEGER ',' INTEGER ')'",
	"subroom_pos : RANDOM_TYPE",
	"room_align : '(' h_justif ',' v_justif ')'",
	"room_align : RANDOM_TYPE",
	"room_size : '(' INTEGER ',' INTEGER ')'",
	"room_size : RANDOM_TYPE",
	"room_details : /* empty */",
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
	"reg_init : /* empty */",
	"reg_init : reg_init init_reg",
	"init_reg : RANDOM_OBJECTS_ID ':' object_list",
	"init_reg : RANDOM_PLACES_ID ':' place_list",
	"init_reg : RANDOM_MONSTERS_ID ':' monster_list",
	"object_list : object",
	"object_list : object ',' object_list",
	"monster_list : monster",
	"monster_list : monster ',' monster_list",
	"place_list : place",
	"place_list : place",
	"place_list : place ',' place_list",
	"map_details : /* empty */",
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
	"monster_detail : MONSTER_ID ':' monster_c ',' m_name ',' coordinate",
	"monster_detail : MONSTER_ID ':' monster_c ',' m_name ',' coordinate monster_infos",
	"monster_infos : /* empty */",
	"monster_infos : monster_infos monster_info",
	"monster_info : ',' string",
	"monster_info : ',' MON_ATTITUDE",
	"monster_info : ',' MON_ALERTNESS",
	"monster_info : ',' alignment",
	"monster_info : ',' MON_APPEARANCE string",
	"object_detail : OBJECT_ID ':' object_c ',' o_name ',' coordinate",
	"object_detail : OBJECT_ID ':' object_c ',' o_name ',' coordinate object_infos",
	"object_infos : /* empty */",
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
	"stair_region : STAIR_ID ':' lev_region",
	"stair_region : STAIR_ID ':' lev_region ',' lev_region ',' UP_OR_DOWN",
	"portal_region : PORTAL_ID ':' lev_region",
	"portal_region : PORTAL_ID ':' lev_region ',' lev_region ',' string",
	"teleprt_region : TELEPRT_ID ':' lev_region",
	"teleprt_region : TELEPRT_ID ':' lev_region ',' lev_region",
	"teleprt_region : TELEPRT_ID ':' lev_region ',' lev_region teleprt_detail",
	"branch_region : BRANCH_ID ':' lev_region",
	"branch_region : BRANCH_ID ':' lev_region ',' lev_region",
	"teleprt_detail : /* empty */",
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
	"prefilled : /* empty */",
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
#endif /* YYDEBUG */
#line 1 "/usr/lib/yaccpar"
/*	@(#)yaccpar 1.10 89/04/04 SMI; from S5R3 1.10	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	{ free(yys); free(yyv); return(0); }
#define YYABORT		{ free(yys); free(yyv); return(1); }
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** static variables used by the parser
*/
static YYSTYPE *yyv;			/* value stack */
static int *yys;			/* state stack */

static YYSTYPE *yypv;			/* top of value stack */
static int *yyps;			/* top of state stack */

static int yystate;			/* current state */
static int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */

int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */


/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */
	unsigned yymaxdepth = YYMAXDEPTH;

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yyv = (YYSTYPE*)malloc(yymaxdepth*sizeof(YYSTYPE));
	yys = (int*)malloc(yymaxdepth*sizeof(int));
	if (!yyv || !yys)
	{
		yyerror( "out of memory" );
		return(1);
	}
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			(void)printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			yymaxdepth += YYMAXDEPTH;
			yyv = (YYSTYPE*)realloc((char*)yyv,
				yymaxdepth * sizeof(YYSTYPE));
			yys = (int*)realloc((char*)yys,
				yymaxdepth * sizeof(int));
			if (!yyv || !yys)
			{
				yyerror( "yacc stack overflow" );
				return(1);
			}
			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			(void)printf( "Received token " );
			if ( yychar == 0 )
				(void)printf( "end-of-file\n" );
			else if ( yychar < 0 )
				(void)printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				(void)printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				(void)printf( "Received token " );
				if ( yychar == 0 )
					(void)printf( "end-of-file\n" );
				else if ( yychar < 0 )
					(void)printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					(void)printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						(void)printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					(void)printf( "Error recovery discards " );
					if ( yychar == 0 )
						(void)printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						(void)printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						(void)printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			(void)printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 7:
# line 194 "lev_comp.y"
{
			  int fout, i;

			if (fatal_error > 0) {
				fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				char lbuf[20];
				Strcpy(lbuf, yypvt[-4].map);
				Strcat(lbuf, LEV_EXT);
#ifdef MAC_THINKC5
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY);
#else
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY, OMASK);
#endif
				if (fout < 0) {
					yyerror("Can't open output file!!");
					exit(1);
				}
				maze.flags = yypvt[-3].i;
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
		  } break;
case 8:
# line 229 "lev_comp.y"
{
			int fout, i;

			if (fatal_error > 0) {
			    fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				char lbuf[20];
				Strcpy(lbuf, yypvt[-6].map);
				Strcat(lbuf, LEV_EXT);
#ifdef MAC_THINKC5
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY);
#else
				fout = open(lbuf, O_WRONLY|O_CREAT|O_BINARY, OMASK);
#endif
				if (fout < 0) {
					yyerror("Can't open output file!!");
					exit(1);
				}
				special_lev.flags = yypvt[-5].i;
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
		  } break;
case 9:
# line 271 "lev_comp.y"
{
			if (index(yypvt[-0].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if (strlen(yypvt[-0].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yypvt[-0].map;
			special_lev.nrobjects = 0;
			special_lev.nrmonst = 0;
		  } break;
case 10:
# line 283 "lev_comp.y"
{
			init_lev.init_present = FALSE;
			yyval.i = 0;
		  } break;
case 11:
# line 288 "lev_comp.y"
{
			init_lev.init_present = TRUE;
			if((init_lev.fg = what_map_char(yypvt[-10].i)) == INVALID_TYPE)
			    yyerror("Invalid foreground type.");
			if((init_lev.bg = what_map_char(yypvt[-8].i)) == INVALID_TYPE)
			    yyerror("Invalid background type.");
			init_lev.smoothed = yypvt[-6].i;
			init_lev.joined = yypvt[-4].i;
			init_lev.lit = yypvt[-2].i;
			init_lev.walled = yypvt[-0].i;
			yyval.i = 1;
		  } break;
case 14:
# line 307 "lev_comp.y"
{
			yyval.i = 0;
		  } break;
case 15:
# line 311 "lev_comp.y"
{
			yyval.i = lev_flags;
			lev_flags = 0;	/* clear for next user */
		  } break;
case 16:
# line 318 "lev_comp.y"
{
			lev_flags |= yypvt[-2].i;
		  } break;
case 17:
# line 322 "lev_comp.y"
{
			lev_flags |= yypvt[-0].i;
		  } break;
case 20:
# line 332 "lev_comp.y"
{
			int i, j;

			i = strlen(yypvt[-0].map) + 1;
			j = tmpmessage[0] ? strlen(tmpmessage) : 0;
			if(i+j > 255) {
			   yyerror("Message string too long (>256 characters)");
			} else {
			    if(j) tmpmessage[j++] = '\n';
			    strncpy(tmpmessage+j, yypvt[-0].map, i-1);
			    tmpmessage[j+i-1] = 0;
			}
		  } break;
case 23:
# line 352 "lev_comp.y"
{
			if(special_lev.nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    special_lev.nrobjects = n_olist;
			    special_lev.robjects = (char *) alloc(n_olist);
			    (void) memcpy((genericptr_t)special_lev.robjects,
					  (genericptr_t)olist, n_olist);
			}
		  } break;
case 24:
# line 363 "lev_comp.y"
{
			if(special_lev.nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    special_lev.nrmonst = n_mlist;
			    special_lev.rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)special_lev.rmonst,
					  (genericptr_t)mlist, n_mlist);
			  }
		  } break;
case 25:
# line 376 "lev_comp.y"
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
		  } break;
case 31:
# line 404 "lev_comp.y"
{
			tmpcor[0] = New(corridor);
			tmpcor[0]->src.room = -1;
			ncorridor = 1;
		  } break;
case 34:
# line 416 "lev_comp.y"
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yypvt[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yypvt[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yypvt[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = yypvt[-0].corpos.room;
			tmpcor[ncorridor]->dest.wall = yypvt[-0].corpos.wall;
			tmpcor[ncorridor]->dest.door = yypvt[-0].corpos.door;
			ncorridor++;
		  } break;
case 35:
# line 427 "lev_comp.y"
{
			tmpcor[ncorridor]->src.room = yypvt[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yypvt[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yypvt[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = -1;
			tmpcor[ncorridor]->dest.wall = yypvt[-0].i;
			ncorridor++;
		  } break;
case 36:
# line 438 "lev_comp.y"
{
			if (yypvt[-5].i >= nrooms)
			    yyerror("Wrong room number!");
			yyval.corpos.room = yypvt[-5].i;
			yyval.corpos.wall = yypvt[-3].i;
			yyval.corpos.door = yypvt[-1].i;
		  } break;
case 37:
# line 448 "lev_comp.y"
{
			store_room();
		  } break;
case 38:
# line 452 "lev_comp.y"
{
			store_room();
		  } break;
case 39:
# line 458 "lev_comp.y"
{
			tmproom[nrooms] = New(room);
			(void) memset((genericptr_t) tmproom[nrooms], 0,
					sizeof *tmproom[nrooms]);
			tmproom[nrooms]->parent = dup_string(yypvt[-1].map);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->rtype = yypvt[-9].i;
			tmproom[nrooms]->rlit = yypvt[-7].i;
			tmproom[nrooms]->filled = yypvt[-0].i;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  } break;
case 40:
# line 478 "lev_comp.y"
{
			tmproom[nrooms] = New(room);
			(void) memset((genericptr_t) tmproom[nrooms], 0,
					sizeof *tmproom[nrooms]);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = yypvt[-9].i;
			tmproom[nrooms]->rlit = yypvt[-7].i;
			tmproom[nrooms]->filled = yypvt[-0].i;
			tmproom[nrooms]->xalign = current_align.x;
			tmproom[nrooms]->yalign = current_align.y;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  } break;
case 41:
# line 498 "lev_comp.y"
{
			yyval.i = 1;
		  } break;
case 42:
# line 502 "lev_comp.y"
{
			yyval.i = yypvt[-0].i;
		  } break;
case 43:
# line 508 "lev_comp.y"
{
			if ( yypvt[-3].i < 1 || yypvt[-3].i > 5 ||
			    yypvt[-1].i < 1 || yypvt[-1].i > 5 ) {
			    yyerror("Room position should be between 1 & 5!");
			} else {
			    current_coord.x = yypvt[-3].i;
			    current_coord.y = yypvt[-1].i;
			}
		  } break;
case 44:
# line 518 "lev_comp.y"
{
			current_coord.x = current_coord.y = ERR;
		  } break;
case 45:
# line 524 "lev_comp.y"
{
			if ( yypvt[-3].i < 0 || yypvt[-1].i < 0) {
			    yyerror("Invalid subroom position !");
			} else {
			    current_coord.x = yypvt[-3].i;
			    current_coord.y = yypvt[-1].i;
			}
		  } break;
case 46:
# line 533 "lev_comp.y"
{
			current_coord.x = current_coord.y = ERR;
		  } break;
case 47:
# line 539 "lev_comp.y"
{
			current_align.x = yypvt[-3].i;
			current_align.y = yypvt[-1].i;
		  } break;
case 48:
# line 544 "lev_comp.y"
{
			current_align.x = current_align.y = ERR;
		  } break;
case 49:
# line 550 "lev_comp.y"
{
			current_size.width = yypvt[-3].i;
			current_size.height = yypvt[-1].i;
		  } break;
case 50:
# line 555 "lev_comp.y"
{
			current_size.height = current_size.width = ERR;
		  } break;
case 66:
# line 580 "lev_comp.y"
{
			if (tmproom[nrooms]->name)
			    yyerror("This room already has a name!");
			else
			    tmproom[nrooms]->name = dup_string(yypvt[-0].map);
		  } break;
case 67:
# line 589 "lev_comp.y"
{
			if (tmproom[nrooms]->chance)
			    yyerror("This room already assigned a chance!");
			else if (tmproom[nrooms]->rtype == OROOM)
			    yyerror("Only typed rooms can have a chance!");
			else if (yypvt[-0].i < 1 || yypvt[-0].i > 99)
			    yyerror("The chance is supposed to be precentile.");
			else
			    tmproom[nrooms]->chance = yypvt[-0].i;
		   } break;
case 68:
# line 602 "lev_comp.y"
{
			/* ERR means random here */
			if (yypvt[-2].i == ERR && yypvt[-0].i != ERR) {
		     yyerror("If the door wall is random, so must be its pos!");
			} else {
			    tmprdoor[ndoor] = New(room_door);
			    tmprdoor[ndoor]->secret = yypvt[-6].i;
			    tmprdoor[ndoor]->mask = yypvt[-4].i;
			    tmprdoor[ndoor]->wall = yypvt[-2].i;
			    tmprdoor[ndoor]->pos = yypvt[-0].i;
			    ndoor++;
			}
		  } break;
case 75:
# line 630 "lev_comp.y"
{
			maze.filling = yypvt[-0].i;
			if (index(yypvt[-2].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if (strlen(yypvt[-2].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yypvt[-2].map;
			in_room = 0;
		  } break;
case 76:
# line 642 "lev_comp.y"
{
			yyval.i = get_floor_type((char)yypvt[-0].i);
		  } break;
case 77:
# line 646 "lev_comp.y"
{
			yyval.i = -1;
		  } break;
case 80:
# line 656 "lev_comp.y"
{
			store_part();
		  } break;
case 81:
# line 662 "lev_comp.y"
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
		  } break;
case 82:
# line 678 "lev_comp.y"
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = yypvt[-1].i % 10;
			tmppart[npart]->valign = yypvt[-1].i / 10;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			scan_map(yypvt[-0].map);
		  } break;
case 83:
# line 690 "lev_comp.y"
{
			yyval.i = yypvt[-2].i + (yypvt[-0].i * 10);
		  } break;
case 90:
# line 708 "lev_comp.y"
{
			if (tmppart[npart]->nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    tmppart[npart]->robjects = (char *)alloc(n_olist);
			    (void) memcpy((genericptr_t)tmppart[npart]->robjects,
					  (genericptr_t)olist, n_olist);
			    tmppart[npart]->nrobjects = n_olist;
			}
		  } break;
case 91:
# line 719 "lev_comp.y"
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
		  } break;
case 92:
# line 734 "lev_comp.y"
{
			if (tmppart[npart]->nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    tmppart[npart]->rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)tmppart[npart]->rmonst,
					  (genericptr_t)mlist, n_mlist);
			    tmppart[npart]->nrmonst = n_mlist;
			}
		  } break;
case 93:
# line 747 "lev_comp.y"
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yypvt[-0].i;
			else
			    yyerror("Object list too long!");
		  } break;
case 94:
# line 754 "lev_comp.y"
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yypvt[-2].i;
			else
			    yyerror("Object list too long!");
		  } break;
case 95:
# line 763 "lev_comp.y"
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yypvt[-0].i;
			else
			    yyerror("Monster list too long!");
		  } break;
case 96:
# line 770 "lev_comp.y"
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yypvt[-2].i;
			else
			    yyerror("Monster list too long!");
		  } break;
case 97:
# line 779 "lev_comp.y"
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  } break;
case 98:
# line 786 "lev_comp.y"
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  } break;
case 122:
# line 822 "lev_comp.y"
{
			tmpmonst[nmons] = New(monster);
			tmpmonst[nmons]->x = current_coord.x;
			tmpmonst[nmons]->y = current_coord.y;
			tmpmonst[nmons]->class = yypvt[-4].i;
			tmpmonst[nmons]->peaceful = -1; /* no override */
			tmpmonst[nmons]->asleep = -1;
			tmpmonst[nmons]->align = - MAX_REGISTERS - 2;
			tmpmonst[nmons]->name = (char *) 0;
			tmpmonst[nmons]->appear = 0;
			tmpmonst[nmons]->appear_as = (char *) 0;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Monster");
			if (!yypvt[-2].map)
			    tmpmonst[nmons]->id = -1;
			else {
				int token = get_monster_id(yypvt[-2].map, (char) yypvt[-4].i);
				if (token == ERR) {
				    yywarning("Illegal monster name!  Making random monster.");
				    tmpmonst[nmons]->id = -1;
				} else
				    tmpmonst[nmons]->id = token;
			}
		  } break;
case 123:
# line 848 "lev_comp.y"
{
			nmons++;
		  } break;
case 126:
# line 858 "lev_comp.y"
{
			tmpmonst[nmons]->name = dup_string(yypvt[-0].map);
		  } break;
case 127:
# line 862 "lev_comp.y"
{
			tmpmonst[nmons]->peaceful = yypvt[-0].i;
		  } break;
case 128:
# line 866 "lev_comp.y"
{
			tmpmonst[nmons]->asleep = yypvt[-0].i;
		  } break;
case 129:
# line 870 "lev_comp.y"
{
			tmpmonst[nmons]->align = yypvt[-0].i;
		  } break;
case 130:
# line 874 "lev_comp.y"
{
			tmpmonst[nmons]->appear = yypvt[-1].i;
			tmpmonst[nmons]->appear_as = dup_string(yypvt[-0].map);
		  } break;
case 131:
# line 881 "lev_comp.y"
{
			tmpobj[nobj] = New(object);
			tmpobj[nobj]->x = current_coord.x;
			tmpobj[nobj]->y = current_coord.y;
			tmpobj[nobj]->class = yypvt[-4].i;
			tmpobj[nobj]->corpsenm = -1;	/* init as none */
			tmpobj[nobj]->curse_state = -1;
			tmpobj[nobj]->name = (char *) 0;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Object");
			if (!yypvt[-2].map)
			    tmpobj[nobj]->id = -1;
			else {
				int token = get_object_id(yypvt[-2].map);
				if (token == ERR) {
				    yywarning("Illegal object name!  Making random object.");
				    tmpobj[nobj]->id = -1;
				} else
				    tmpobj[nobj]->id = token;
			}
		  } break;
case 132:
# line 904 "lev_comp.y"
{
			nobj++;
		  } break;
case 133:
# line 910 "lev_comp.y"
{
			tmpobj[nobj]->spe = -127;
		  } break;
case 134:
# line 914 "lev_comp.y"
{
			int token = get_monster_id(yypvt[-2].map, (char)0);
			if (token == ERR)	/* "random" */
			    tmpobj[nobj]->corpsenm = -2;
			else
			    tmpobj[nobj]->corpsenm = token;
			tmpobj[nobj]->spe = yypvt[-0].i;
		  } break;
case 135:
# line 923 "lev_comp.y"
{
			tmpobj[nobj]->curse_state = yypvt[-4].i;
			tmpobj[nobj]->spe = yypvt[-2].i;
			if (yypvt[-0].map)
			    tmpobj[nobj]->name = dup_string(yypvt[-0].map);
			else
			    tmpobj[nobj]->name = (char *) 0;
		  } break;
case 139:
# line 939 "lev_comp.y"
{
			yyval.i = -127;
		  } break;
case 140:
# line 945 "lev_comp.y"
{
			tmpdoor[ndoor] = New(door);
			tmpdoor[ndoor]->x = current_coord.x;
			tmpdoor[ndoor]->y = current_coord.y;
			tmpdoor[ndoor]->mask = yypvt[-2].i;
			if(current_coord.x >= 0 && current_coord.y >= 0 &&
			   tmpmap[current_coord.y][current_coord.x] != DOOR &&
			   tmpmap[current_coord.y][current_coord.x] != SDOOR)
			    yyerror("Door decl doesn't match the map");
			ndoor++;
		  } break;
case 141:
# line 959 "lev_comp.y"
{
			tmptrap[ntrap] = New(trap);
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yypvt[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Trap");
			ntrap++;
		  } break;
case 142:
# line 972 "lev_comp.y"
{
		        int x, y, dir;

			tmpdb[ndb] = New(drawbridge);
			x = tmpdb[ndb]->x = current_coord.x;
			y = tmpdb[ndb]->y = current_coord.y;
			/* convert dir from a DIRECTION to a DB_DIR */
			dir = yypvt[-2].i;
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

			if ( yypvt[-0].i == D_ISOPEN )
			    tmpdb[ndb]->db_open = 1;
			else if ( yypvt[-0].i == D_CLOSED )
			    tmpdb[ndb]->db_open = 0;
			else
			    yyerror("A drawbridge can only be open or closed!");
			ndb++;
		   } break;
case 143:
# line 1010 "lev_comp.y"
{
			tmpwalk[nwalk] = New(walk);
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = yypvt[-0].i;
			nwalk++;
		  } break;
case 144:
# line 1020 "lev_comp.y"
{
			wallify_map();
		  } break;
case 145:
# line 1026 "lev_comp.y"
{
			tmplad[nlad] = New(lad);
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = yypvt[-0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Ladder");
			nlad++;
		  } break;
case 146:
# line 1039 "lev_comp.y"
{
			tmpstair[nstair] = New(stair);
			tmpstair[nstair]->x = current_coord.x;
			tmpstair[nstair]->y = current_coord.y;
			tmpstair[nstair]->up = yypvt[-0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Stairway");
			nstair++;
		  } break;
case 147:
# line 1052 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yypvt[-0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  } break;
case 148:
# line 1061 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yypvt[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			if(yypvt[-0].i)
			    tmplreg[nlreg]->rtype = LR_UPSTAIR;
			else
			    tmplreg[nlreg]->rtype = LR_DOWNSTAIR;
			tmplreg[nlreg]->rname = 0;
			nlreg++;
		  } break;
case 149:
# line 1077 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yypvt[-0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  } break;
case 150:
# line 1086 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yypvt[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_PORTAL;
			tmplreg[nlreg]->rname = yypvt[-0].map;
			nlreg++;
		  } break;
case 151:
# line 1099 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yypvt[-0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  } break;
case 152:
# line 1108 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yypvt[-0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
		  } break;
case 153:
# line 1116 "lev_comp.y"
{
			switch(yypvt[-0].i) {
			case -1: tmplreg[nlreg]->rtype = LR_TELE; break;
			case 0: tmplreg[nlreg]->rtype = LR_DOWNTELE; break;
			case 1: tmplreg[nlreg]->rtype = LR_UPTELE; break;
			}
			tmplreg[nlreg]->rname = 0;
			nlreg++;
		  } break;
case 154:
# line 1128 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yypvt[-0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  } break;
case 155:
# line 1137 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yypvt[-0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_BRANCH;
			tmplreg[nlreg]->rname = 0;
			nlreg++;
		  } break;
case 156:
# line 1150 "lev_comp.y"
{
			yyval.i = -1;
		  } break;
case 157:
# line 1154 "lev_comp.y"
{
			yyval.i = yypvt[-0].i;
		  } break;
case 158:
# line 1160 "lev_comp.y"
{
			yyval.i = 0;
		  } break;
case 159:
# line 1164 "lev_comp.y"
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yypvt[-7].i <= 0 || yypvt[-7].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yypvt[-5].i < 0 || yypvt[-5].i >= ROWNO)
				yyerror("Region out of level range!");
			else if (yypvt[-3].i <= 0 || yypvt[-3].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yypvt[-1].i < 0 || yypvt[-1].i >= ROWNO)
				yyerror("Region out of level range!");
			current_region.x1 = yypvt[-7].i;
			current_region.y1 = yypvt[-5].i;
			current_region.x2 = yypvt[-3].i;
			current_region.y2 = yypvt[-1].i;
			yyval.i = 1;
		  } break;
case 160:
# line 1184 "lev_comp.y"
{
			tmpfountain[nfountain] = New(fountain);
			tmpfountain[nfountain]->x = current_coord.x;
			tmpfountain[nfountain]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Fountain");
			nfountain++;
		  } break;
case 161:
# line 1196 "lev_comp.y"
{
			tmpsink[nsink] = New(sink);
			tmpsink[nsink]->x = current_coord.x;
			tmpsink[nsink]->y = current_coord.y;
			nsink++;
		  } break;
case 162:
# line 1205 "lev_comp.y"
{
			tmppool[npool] = New(pool);
			tmppool[npool]->x = current_coord.x;
			tmppool[npool]->y = current_coord.y;
			npool++;
		  } break;
case 163:
# line 1214 "lev_comp.y"
{
			tmpdig[ndig] = New(digpos);
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
		  } break;
case 164:
# line 1225 "lev_comp.y"
{
			tmppass[npass] = New(digpos);
			tmppass[npass]->x1 = current_region.x1;
			tmppass[npass]->y1 = current_region.y1;
			tmppass[npass]->x2 = current_region.x2;
			tmppass[npass]->y2 = current_region.y2;
			npass++;
		  } break;
case 165:
# line 1236 "lev_comp.y"
{
			tmpreg[nreg] = New(region);
			tmpreg[nreg]->x1 = current_region.x1;
			tmpreg[nreg]->y1 = current_region.y1;
			tmpreg[nreg]->x2 = current_region.x2;
			tmpreg[nreg]->y2 = current_region.y2;
			tmpreg[nreg]->rlit = yypvt[-3].i;
			tmpreg[nreg]->rtype = yypvt[-1].i;
			if(yypvt[-0].i & 1) tmpreg[nreg]->rtype += MAXRTYPE+1;
			tmpreg[nreg]->rirreg = ((yypvt[-0].i & 2) != 0);
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
		  } break;
case 166:
# line 1298 "lev_comp.y"
{
			tmpaltar[naltar] = New(altar);
			tmpaltar[naltar]->x = current_coord.x;
			tmpaltar[naltar]->y = current_coord.y;
			tmpaltar[naltar]->align = yypvt[-2].i;
			tmpaltar[naltar]->shrine = yypvt[-0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Altar");
			naltar++;
		  } break;
case 167:
# line 1312 "lev_comp.y"
{
			tmpgold[ngold] = New(gold);
			tmpgold[ngold]->x = current_coord.x;
			tmpgold[ngold]->y = current_coord.y;
			tmpgold[ngold]->amount = yypvt[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Gold");
			ngold++;
		  } break;
case 168:
# line 1325 "lev_comp.y"
{
			tmpengraving[nengraving] = New(engraving);
			tmpengraving[nengraving]->x = current_coord.x;
			tmpengraving[nengraving]->y = current_coord.y;
			tmpengraving[nengraving]->e.text = yypvt[-0].map;
			tmpengraving[nengraving]->etype = yypvt[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Engraving");
			nengraving++;
		  } break;
case 170:
# line 1340 "lev_comp.y"
{
			yyval.i = - MAX_REGISTERS - 1;
		  } break;
case 173:
# line 1348 "lev_comp.y"
{
			yyval.i = - MAX_REGISTERS - 1;
		  } break;
case 176:
# line 1356 "lev_comp.y"
{
			yyval.map = (char *) 0;
		  } break;
case 178:
# line 1363 "lev_comp.y"
{
			yyval.map = (char *) 0;
		  } break;
case 179:
# line 1369 "lev_comp.y"
{
			int token = get_trap_type(yypvt[-0].map);
			if (token == ERR)
				yyerror("Unknown trap type!");
			yyval.i = token;
		  } break;
case 181:
# line 1379 "lev_comp.y"
{
			int token = get_room_type(yypvt[-0].map);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				yyval.i = OROOM;
			} else
				yyval.i = token;
		  } break;
case 183:
# line 1391 "lev_comp.y"
{
			yyval.i = 0;
		  } break;
case 184:
# line 1395 "lev_comp.y"
{
			yyval.i = yypvt[-0].i;
		  } break;
case 185:
# line 1399 "lev_comp.y"
{
			yyval.i = yypvt[-2].i + (yypvt[-0].i << 1);
		  } break;
case 188:
# line 1407 "lev_comp.y"
{
			current_coord.x = current_coord.y = -MAX_REGISTERS-1;
		  } break;
case 195:
# line 1423 "lev_comp.y"
{
			yyval.i = - MAX_REGISTERS - 1;
		  } break;
case 198:
# line 1433 "lev_comp.y"
{
			if ( yypvt[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				current_coord.x = current_coord.y = - yypvt[-1].i - 1;
		  } break;
case 199:
# line 1442 "lev_comp.y"
{
			if ( yypvt[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yypvt[-1].i - 1;
		  } break;
case 200:
# line 1451 "lev_comp.y"
{
			if ( yypvt[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yypvt[-1].i - 1;
		  } break;
case 201:
# line 1460 "lev_comp.y"
{
			if ( yypvt[-1].i >= 3 )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yypvt[-1].i - 1;
		  } break;
case 203:
# line 1472 "lev_comp.y"
{
			if (check_monster_char((char) yypvt[-0].i))
				yyval.i = yypvt[-0].i ;
			else {
				yyerror("Unknown monster class!");
				yyval.i = ERR;
			}
		  } break;
case 204:
# line 1483 "lev_comp.y"
{
			char c = yypvt[-0].i;
			if (check_object_char(c))
				yyval.i = c;
			else {
				yyerror("Unknown char class!");
				yyval.i = ERR;
			}
		  } break;
case 207:
# line 1499 "lev_comp.y"
{
			yyval.map = (char *) 0;
		  } break;
case 212:
# line 1513 "lev_comp.y"
{
			if (!in_room && !init_lev.init_present &&
			    (yypvt[-3].i < 0 || yypvt[-3].i > max_x_map ||
			     yypvt[-1].i < 0 || yypvt[-1].i > max_y_map))
			    yyerror("Coordinates out of map range!");
			current_coord.x = yypvt[-3].i;
			current_coord.y = yypvt[-1].i;
		  } break;
case 213:
# line 1524 "lev_comp.y"
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yypvt[-7].i < 0 || yypvt[-7].i > max_x_map)
				yyerror("Region out of map range!");
			else if (yypvt[-5].i < 0 || yypvt[-5].i > max_y_map)
				yyerror("Region out of map range!");
			else if (yypvt[-3].i < 0 || yypvt[-3].i > max_x_map)
				yyerror("Region out of map range!");
			else if (yypvt[-1].i < 0 || yypvt[-1].i > max_y_map)
				yyerror("Region out of map range!");
			current_region.x1 = yypvt[-7].i;
			current_region.y1 = yypvt[-5].i;
			current_region.x2 = yypvt[-3].i;
			current_region.y2 = yypvt[-1].i;
		  } break;
	}
	goto yystack;		/* reset registers in driver code */
}

