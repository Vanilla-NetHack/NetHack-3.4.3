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
/*	SCCS Id: @(#)dgn_comp.c	3.1	93/05/15	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/*	Copyright (c) 1990 by M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Dungeon Compiler code
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

#include "config.h"
#include "dgn_file.h"

void FDECL(yyerror, (const char *));
void FDECL(yywarning, (const char *));
int NDECL(yylex);
int NDECL(yyparse);
int FDECL(getchain, (char *));
int NDECL(check_dungeon);
int NDECL(check_branch);
int NDECL(check_level);
void NDECL(init_dungeon);
void NDECL(init_branch);
void NDECL(init_level);
void NDECL(output_dgn);

#ifdef AMIGA
# undef	printf
#ifndef	LATTICE
# define    memset(addr,val,len)    setmem(addr,len,val)
#endif
#endif

#ifdef MICRO
# undef exit
extern void FDECL(exit, (int));
#endif

#undef NULL

#define ERR		(-1)

static struct couple couple;
static struct tmpdungeon tmpdungeon[MAXDUNGEON];
static struct tmplevel tmplevel[LEV_LIMIT];
static struct tmpbranch tmpbranch[BRANCH_LIMIT];

static int in_dungeon = 0, n_dgns = -1, n_levs = -1, n_brs = -1;

extern int fatal_error;
extern const char *fname;

typedef union
{
	int	i;
	char*	str;
} YYSTYPE;
#define INTEGER 257
#define A_DUNGEON 258
#define BRANCH 259
#define CHBRANCH 260
#define LEVEL 261
#define RNDLEVEL 262
#define CHLEVEL 263
#define RNDCHLEVEL 264
#define UP_OR_DOWN 265
#define PROTOFILE 266
#define DESCRIPTION 267
#define DESCRIPTOR 268
#define LEVELDESC 269
#define ALIGNMENT 270
#define LEVALIGN 271
#define ENTRY 272
#define STAIR 273
#define NO_UP 274
#define NO_DOWN 275
#define PORTAL 276
#define STRING 277
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    4,    4,    5,    5,    5,    5,    6,    1,
    1,    7,    7,    7,   11,   12,   14,   14,   13,    9,
    9,    9,    9,    9,   15,   15,   16,   16,   17,   17,
   18,   18,   19,   19,    8,    8,   21,   22,    3,    3,
    3,    3,    3,    2,    2,   20,   10,
};
short yylen[] = {                                         2,
    0,    1,    1,    2,    1,    1,    1,    1,    6,    0,
    1,    1,    1,    1,    3,    1,    3,    3,    3,    1,
    1,    1,    1,    1,    6,    7,    7,    8,    3,    3,
    7,    8,    8,    9,    1,    1,    7,    8,    0,    1,
    1,    1,    1,    0,    1,    5,    5,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    3,    5,    6,    7,    8,
   12,   13,   14,   16,   20,   21,   22,   23,   24,   35,
   36,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    4,    0,    0,    0,    0,    0,
    0,    0,   19,   17,   29,   18,   30,   15,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   11,    9,    0,   40,   41,
   42,   43,    0,    0,    0,    0,    0,    0,    0,    0,
   45,   37,    0,   27,    0,    0,    0,    0,    0,   38,
   28,   33,    0,   47,   46,   34,
};
short yydgoto[] = {                                      14,
   77,   92,   83,   15,   16,   17,   18,   19,   20,   67,
   21,   22,   23,   24,   25,   26,   27,   28,   29,   69,
   30,   31,
};
short yysindex[] = {                                   -237,
  -50,  -49,  -48,  -47,  -46,  -45,  -44,  -43,  -39,  -38,
  -30,  -22,  -21,    0, -237,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -239, -238, -236, -235, -234, -233, -232, -230, -228,
 -220, -219, -218, -206,    0, -225,  -11, -223, -222, -221,
 -217, -215,    0,    0,    0,    0,    0,    0,   17,   18,
   20,   -5,    2, -213, -212, -190, -189, -188, -271,   17,
   18,   18,   27,   28,   29,    0,    0,   30,    0,    0,
    0,    0, -193, -271, -182, -180,   17,   17, -179, -178,
    0,    0, -193,    0, -177, -176, -175,   42,   43,    0,
    0,    0, -172,    0,    0,    0,
};
short yyrindex[] = {                                     86,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   87,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   16,    0,    1,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   31,    1,   46,    0,    0,    0,    0,    0,
    0,    0,   31,    0,   61,   76,    0,    0,    0,    0,
    0,    0,   91,    0,    0,    0,
};
short yygindex[] = {                                      0,
    0,   -4,    4,    0,   75,    0,    0,    0,    0,  -70,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  -65,
    0,    0,
};
#define YYTABLESIZE 363
short yytable[] = {                                      84,
   39,   79,   80,   81,   82,   85,   86,   32,   33,   34,
   35,   36,   37,   38,   39,   10,   96,   97,   40,   41,
    1,    2,    3,    4,    5,    6,    7,   42,    8,    9,
   44,   10,   11,   12,   13,   43,   44,   46,   47,   54,
   48,   49,   50,   51,   52,   25,   53,   55,   56,   57,
   58,   59,   60,   61,   62,   63,   66,   68,   71,   64,
   26,   65,   70,   73,   74,   72,   75,   76,   78,   87,
   88,   91,   89,   90,   94,   31,   95,   98,   99,  101,
  102,  103,  104,  105,  106,    1,    2,   93,  100,   45,
   32,    0,    0,    0,    0,    0,    0,    0,    0,    0,
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
    0,    0,    0,    0,    0,    0,    0,    0,   39,   39,
   39,   39,   39,   39,   39,   39,   39,   39,    0,   39,
   39,   39,   39,   10,   10,   10,   10,   10,   10,   10,
    0,   10,   10,    0,   10,   10,   10,   10,   44,   44,
   44,   44,   44,   44,   44,    0,   44,   44,    0,   44,
   44,   44,   44,   25,   25,   25,   25,   25,   25,   25,
    0,   25,   25,    0,   25,   25,   25,   25,   26,   26,
   26,   26,   26,   26,   26,    0,   26,   26,    0,   26,
   26,   26,   26,   31,   31,   31,   31,   31,   31,   31,
    0,   31,   31,    0,   31,   31,   31,   31,   32,   32,
   32,   32,   32,   32,   32,    0,   32,   32,    0,   32,
   32,   32,   32,
};
short yycheck[] = {                                      70,
    0,  273,  274,  275,  276,   71,   72,   58,   58,   58,
   58,   58,   58,   58,   58,    0,   87,   88,   58,   58,
  258,  259,  260,  261,  262,  263,  264,   58,  266,  267,
    0,  269,  270,  271,  272,   58,   58,  277,  277,  268,
  277,  277,  277,  277,  277,    0,  277,  268,  268,  268,
  257,  277,   64,  277,  277,  277,   40,   40,   64,  277,
    0,  277,   43,  277,  277,   64,  257,  257,  257,   43,
   43,  265,   44,   44,  257,    0,  257,  257,  257,  257,
  257,  257,   41,   41,  257,    0,    0,   84,   93,   15,
    0,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  258,  259,
  260,  261,  262,  263,  264,  265,  266,  267,   -1,  269,
  270,  271,  272,  258,  259,  260,  261,  262,  263,  264,
   -1,  266,  267,   -1,  269,  270,  271,  272,  258,  259,
  260,  261,  262,  263,  264,   -1,  266,  267,   -1,  269,
  270,  271,  272,  258,  259,  260,  261,  262,  263,  264,
   -1,  266,  267,   -1,  269,  270,  271,  272,  258,  259,
  260,  261,  262,  263,  264,   -1,  266,  267,   -1,  269,
  270,  271,  272,  258,  259,  260,  261,  262,  263,  264,
   -1,  266,  267,   -1,  269,  270,  271,  272,  258,  259,
  260,  261,  262,  263,  264,   -1,  266,  267,   -1,  269,
  270,  271,  272,
};
#define YYFINAL 14
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 277
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,"'+'","','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,0,0,0,
"'@'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER",
"A_DUNGEON","BRANCH","CHBRANCH","LEVEL","RNDLEVEL","CHLEVEL","RNDCHLEVEL",
"UP_OR_DOWN","PROTOFILE","DESCRIPTION","DESCRIPTOR","LEVELDESC","ALIGNMENT",
"LEVALIGN","ENTRY","STAIR","NO_UP","NO_DOWN","PORTAL","STRING",
};
char *yyrule[] = {
"$accept : file",
"file :",
"file : dungeons",
"dungeons : dungeon",
"dungeons : dungeons dungeon",
"dungeon : dungeonline",
"dungeon : dungeondesc",
"dungeon : branches",
"dungeon : levels",
"dungeonline : A_DUNGEON ':' STRING STRING rcouple optional_int",
"optional_int :",
"optional_int : INTEGER",
"dungeondesc : entry",
"dungeondesc : descriptions",
"dungeondesc : prototype",
"entry : ENTRY ':' INTEGER",
"descriptions : desc",
"desc : DESCRIPTION ':' DESCRIPTOR",
"desc : ALIGNMENT ':' DESCRIPTOR",
"prototype : PROTOFILE ':' STRING",
"levels : level1",
"levels : level2",
"levels : levdesc",
"levels : chlevel1",
"levels : chlevel2",
"level1 : LEVEL ':' STRING STRING '@' acouple",
"level1 : RNDLEVEL ':' STRING STRING '@' acouple INTEGER",
"level2 : LEVEL ':' STRING STRING '@' acouple INTEGER",
"level2 : RNDLEVEL ':' STRING STRING '@' acouple INTEGER INTEGER",
"levdesc : LEVELDESC ':' DESCRIPTOR",
"levdesc : LEVALIGN ':' DESCRIPTOR",
"chlevel1 : CHLEVEL ':' STRING STRING STRING '+' rcouple",
"chlevel1 : RNDCHLEVEL ':' STRING STRING STRING '+' rcouple INTEGER",
"chlevel2 : CHLEVEL ':' STRING STRING STRING '+' rcouple INTEGER",
"chlevel2 : RNDCHLEVEL ':' STRING STRING STRING '+' rcouple INTEGER INTEGER",
"branches : branch",
"branches : chbranch",
"branch : BRANCH ':' STRING '@' acouple branch_type direction",
"chbranch : CHBRANCH ':' STRING STRING '+' rcouple branch_type direction",
"branch_type :",
"branch_type : STAIR",
"branch_type : NO_UP",
"branch_type : NO_DOWN",
"branch_type : PORTAL",
"direction :",
"direction : UP_OR_DOWN",
"acouple : '(' INTEGER ',' INTEGER ')'",
"rcouple : '(' INTEGER ',' INTEGER ')'",
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

void
init_dungeon()
{
	if(++n_dgns > MAXDUNGEON) {
	    fprintf(stderr, "FATAL - Too many dungeons (limit: %d).\n",
		    MAXDUNGEON);
	    fprintf(stderr, "To increase the limit edit MAXDUNGEON in global.h\n");
	    exit(1);
	}

	in_dungeon = 1;
	tmpdungeon[n_dgns].lev.base = 0;
	tmpdungeon[n_dgns].lev.rand = 0;
	tmpdungeon[n_dgns].chance = 100;
	strcpy(tmpdungeon[n_dgns].name, "");
	strcpy(tmpdungeon[n_dgns].protoname, "");
	tmpdungeon[n_dgns].flags = 0;
	tmpdungeon[n_dgns].levels = 0;
	tmpdungeon[n_dgns].branches = 0;
	tmpdungeon[n_dgns].entry_lev = 0;
}

void
init_level()
{
	if(++n_levs > LEV_LIMIT) {

		yyerror("FATAL - Too many special levels defined.");
		exit(1);
	}
	tmplevel[n_levs].lev.base = 0;
	tmplevel[n_levs].lev.rand = 0;
	tmplevel[n_levs].chance = 100;
	tmplevel[n_levs].rndlevs = 0;
	tmplevel[n_levs].flags = 0;
	strcpy(tmplevel[n_levs].name, "");
	tmplevel[n_levs].chain = -1;
}

void
init_branch()
{
	if(++n_brs > BRANCH_LIMIT) {

		yyerror("FATAL - Too many special levels defined.");
		exit(1);
	}
	tmpbranch[n_brs].lev.base = 0;
	tmpbranch[n_brs].lev.rand = 0;
	strcpy(tmpbranch[n_brs].name, "");
	tmpbranch[n_brs].chain = -1;
}

int
getchain(s)
	char	*s;
{
	int i;

	if(strlen(s)) {

	    for(i = n_levs - tmpdungeon[n_dgns].levels + 1; i <= n_levs; i++)
		if(!strcmp(tmplevel[i].name, s)) return i;

	    yyerror("Can't locate the specified chain level.");
	    return(-2);
	}
	return(-1);
}

/*
 *	Consistancy checking routines:
 *
 *	- A dungeon must have a unique name.
 *	- A dungeon must have a originating "branch" command
 *	  (except, of course, for the first dungeon).
 *	- A dungeon must have a proper depth (at least (1, 0)).
 */

int
check_dungeon()
{
	int i;

	for(i = 0; i < n_dgns; i++)
	    if(!strcmp(tmpdungeon[i].name, tmpdungeon[n_dgns].name)) {
		yyerror("Duplicate dungeon name.");
		return(0);
	    }

	if(n_dgns)
	  for(i = 0; i < n_brs - tmpdungeon[n_dgns].branches; i++) {
	    if(!strcmp(tmpbranch[i].name, tmpdungeon[n_dgns].name)) break;

	    if(i >= n_brs - tmpdungeon[n_dgns].branches) {
		yyerror("Dungeon cannot be reached.");
		return(0);
	    }
	  }

	if(tmpdungeon[n_dgns].lev.base <= 0 ||
	   tmpdungeon[n_dgns].lev.rand < 0) {
		yyerror("Invalid dungeon depth specified.");
		return(0);
	}
	return(1);	/* OK */
}

/*
 *	- A level must have a unique level name.
 *	- If chained, the level used as reference for the chain
 *	  must be in this dungeon, must be previously defined, and
 *	  the level chained from must be "non-probabalistic" (ie.
 *	  have a 100% chance of existing).
 */

int
check_level()
{
	int i;

	if(!in_dungeon) {
		yyerror("Level defined outside of dungeon.");
		return(0);
	}

	for(i = 0; i < n_levs; i++)
	    if(!strcmp(tmplevel[i].name, tmplevel[n_levs].name)) {
		yyerror("Duplicate level name.");
		return(0);
	    }

	if(tmplevel[i].chain == -2) {
		yyerror("Invaild level chain reference.");
		return(0);
	} else if(tmplevel[i].chain != -1) {	/* there is a chain */
	    if(tmplevel[tmpbranch[i].chain].chance != 100) {
		yyerror("Level cannot chain from a probabalistic level.");
		return(0);
	    } else if(tmplevel[i].chain == n_levs) {
		yyerror("A level cannot chain to itself!");
		return(0);
	    }
	}
	return(1);	/* OK */
}

/*
 *	- A branch may not branch backwards - to avoid branch loops.
 *	- A branch name must be unique.
 *	  (ie. You can only have one entry point to each dungeon).
 *	- If chained, the level used as reference for the chain
 *	  must be in this dungeon, must be previously defined, and
 *	  the level chained from must be "non-probabalistic" (ie.
 *	  have a 100% chance of existing).
 */

int
check_branch()
{
	int i;

	if(!in_dungeon) {
		yyerror("Branch defined outside of dungeon.");
		return(0);
	}

	for(i = 0; i < n_dgns; i++)
	    if(!strcmp(tmpdungeon[i].name, tmpbranch[n_brs].name)) {

		yyerror("Reverse branching not allowed.");
		return(0);
	    }

	if(tmpbranch[i].chain == -2) {

		yyerror("Invaild branch chain reference.");
		return(0);
	} else if(tmpbranch[i].chain != -1) {	/* it is chained */

	    if(tmplevel[tmpbranch[i].chain].chance != 100) {
		yyerror("Branch cannot chain from a probabalistic level.");
		return(0);
	    }
	}
	return(1);	/* OK */
}

/*
 *	Output the dungon definition into a file.
 *
 *	The file will have the following format:
 *
 *	[ number of dungeons ]
 *	[ first dungeon struct ]
 *	[ levels for the first dungeon ]
 *	  ...
 *	[ branches for the first dungeon ]
 *	  ...
 *	[ second dungeon struct ]
 *	  ...
 */

void
output_dgn()
{
	int	nd, cl = 0, nl = 0,
		    cb = 0, nb = 0;

	if(++n_dgns <= 0) {

	    yyerror("FATAL - no dungeons were defined.");
	    exit(1);
	}

	fwrite((char *)(&n_dgns), sizeof(int), 1, stdout);
	for(nd = 0; nd < n_dgns; nd++) {

	    fwrite((char *)&tmpdungeon[nd], sizeof(struct tmpdungeon), 1,
								stdout);

	    nl += tmpdungeon[nd].levels;
	    for(; cl < nl; cl++)
		fwrite((char *)&tmplevel[cl], sizeof(struct tmplevel), 1,
								stdout);

	    nb += tmpdungeon[nd].branches;
	    for(; cb < nb; cb++)
		fwrite((char *)&tmpbranch[cb], sizeof(struct tmpbranch), 1,
								stdout);
	}
}
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
case 2:
{
			output_dgn();
		  }
break;
case 9:
{
			init_dungeon();
			strcpy(tmpdungeon[n_dgns].name, yyvsp[-3].str);
			if (!strcmp(yyvsp[-2].str, "none"))
				tmpdungeon[n_levs].boneschar = '\0';
			else if (yyvsp[-2].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmpdungeon[n_dgns].boneschar = yyvsp[-2].str[0];
			tmpdungeon[n_dgns].lev.base = couple.base;
			tmpdungeon[n_dgns].lev.rand = couple.rand;
			tmpdungeon[n_dgns].chance = yyvsp[0].i;
		  }
break;
case 10:
{
			yyval.i = 0;
		  }
break;
case 11:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 15:
{
			tmpdungeon[n_dgns].entry_lev = yyvsp[0].i;
		  }
break;
case 17:
{
			if(yyvsp[0].i <= TOWN || yyvsp[0].i >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmpdungeon[n_dgns].flags |= yyvsp[0].i ;
		  }
break;
case 18:
{
			if(yyvsp[0].i && yyvsp[0].i < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmpdungeon[n_dgns].flags |= yyvsp[0].i ;
		  }
break;
case 19:
{
			strcpy(tmpdungeon[n_dgns].protoname, yyvsp[0].str);
		  }
break;
case 25:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-3].str);
			if (!strcmp(yyvsp[-2].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-2].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-2].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmpdungeon[n_dgns].levels++;
		  }
break;
case 26:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-4].str);
			if (!strcmp(yyvsp[-3].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-3].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-3].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			tmpdungeon[n_dgns].levels++;
		  }
break;
case 27:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-4].str);
			if (!strcmp(yyvsp[-3].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-3].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-3].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[0].i;
			tmpdungeon[n_dgns].levels++;
		  }
break;
case 28:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-5].str);
			if (!strcmp(yyvsp[-4].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-4].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-4].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[-1].i;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			tmpdungeon[n_dgns].levels++;
		  }
break;
case 29:
{
			if(yyvsp[0].i >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmplevel[n_levs].flags |= yyvsp[0].i ;
		  }
break;
case 30:
{
			if(yyvsp[0].i && yyvsp[0].i < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmplevel[n_levs].flags |= yyvsp[0].i ;
		  }
break;
case 31:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-4].str);
			if (!strcmp(yyvsp[-3].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-3].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-3].str[0];
			tmplevel[n_levs].chain = getchain(yyvsp[-2].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  }
break;
case 32:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-5].str);
			if (!strcmp(yyvsp[-4].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-4].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-4].str[0];
			tmplevel[n_levs].chain = getchain(yyvsp[-3].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  }
break;
case 33:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-5].str);
			if (!strcmp(yyvsp[-4].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-4].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-4].str[0];
			tmplevel[n_levs].chain = getchain(yyvsp[-3].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  }
break;
case 34:
{
			init_level();
			strcpy(tmplevel[n_levs].name, yyvsp[-6].str);
			if (!strcmp(yyvsp[-5].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yyvsp[-5].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yyvsp[-5].str[0];
			tmplevel[n_levs].chain = getchain(yyvsp[-4].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yyvsp[-1].i;
			tmplevel[n_levs].rndlevs = yyvsp[0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  }
break;
case 37:
{
			init_branch();
			strcpy(tmpbranch[n_brs].name, yyvsp[-4].str);
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = yyvsp[-1].i;
			tmpbranch[n_brs].up = yyvsp[0].i;
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
		  }
break;
case 38:
{
			init_branch();
			strcpy(tmpbranch[n_brs].name, yyvsp[-5].str);
			tmpbranch[n_brs].chain = getchain(yyvsp[-4].str);
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = yyvsp[-1].i;
			tmpbranch[n_brs].up = yyvsp[0].i;
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
		  }
break;
case 39:
{
			yyval.i = TBR_STAIR;	/* two way stair */
		  }
break;
case 40:
{
			yyval.i = TBR_STAIR;	/* two way stair */
		  }
break;
case 41:
{
			yyval.i = TBR_NO_UP;	/* no up staircase */
		  }
break;
case 42:
{
			yyval.i = TBR_NO_DOWN;	/* no down staircase */
		  }
break;
case 43:
{
			yyval.i = TBR_PORTAL;	/* portal connection */
		  }
break;
case 44:
{
			yyval.i = 0;	/* defaults to down */
		  }
break;
case 45:
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 46:
{
			if (yyvsp[-3].i < -MAXLEVEL || yyvsp[-3].i > MAXLEVEL) {
			    yyerror("Abs base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else if (yyvsp[-1].i < -1 ||
				((yyvsp[-3].i < 0) ? (MAXLEVEL + yyvsp[-3].i + yyvsp[-1].i + 1) > MAXLEVEL :
					(yyvsp[-3].i + yyvsp[-1].i) > MAXLEVEL)) {
			    yyerror("Abs range out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = yyvsp[-3].i;
			    couple.rand = yyvsp[-1].i;
			}
		  }
break;
case 47:
{
			if (yyvsp[-3].i < -MAXLEVEL || yyvsp[-3].i > MAXLEVEL) {
			    yyerror("Rel base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = yyvsp[-3].i;
			    couple.rand = yyvsp[-1].i;
			}
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
