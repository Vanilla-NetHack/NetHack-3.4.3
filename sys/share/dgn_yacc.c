extern char *malloc(), *realloc();

# line 2 "dgn_comp.y"
/*	SCCS Id: @(#)dgn_comp.c	3.1	93/01/17	*/
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

void FDECL(yyerror, (char *));
void FDECL(yywarning, (char *));
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
extern char* fname;


# line 69 "dgn_comp.y"
typedef union 
{
	int	i;
	char*	str;
} YYSTYPE;
# define INTEGER 257
# define A_DUNGEON 258
# define BRANCH 259
# define CHBRANCH 260
# define LEVEL 261
# define RNDLEVEL 262
# define CHLEVEL 263
# define RNDCHLEVEL 264
# define UP_OR_DOWN 265
# define PROTOFILE 266
# define DESCRIPTION 267
# define DESCRIPTOR 268
# define LEVELDESC 269
# define ALIGNMENT 270
# define LEVALIGN 271
# define ENTRY 272
# define STAIR 273
# define NO_UP 274
# define NO_DOWN 275
# define PORTAL 276
# define STRING 277
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 450 "dgn_comp.y"


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
int yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 48
# define YYLAST 145
int yyact[]={

     8,    22,    23,    24,    25,    28,    29,    74,    21,    30,
    73,    26,    31,    27,    19,    65,    79,    80,    81,    82,
   106,    64,    63,    62,    61,    59,    56,    55,    52,    51,
    50,    49,    48,    46,    58,    57,    54,    53,    91,   103,
   102,   101,    99,    98,    95,    94,    83,    77,    76,    47,
    90,    66,    78,    68,    72,    71,    60,    45,    44,    43,
    42,    41,    40,    39,    38,    37,    36,    35,    34,    33,
    92,    89,    88,    87,    70,   105,   104,    67,    69,     3,
    13,    12,    32,    18,    17,    16,    15,    14,    20,    11,
    10,     9,     7,     6,     5,     4,     2,     1,    75,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    84,     0,     0,    85,    86,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    93,     0,    96,
    97,     0,     0,     0,   100 };
int yypact[]={

  -258, -1000,  -258, -1000, -1000, -1000, -1000, -1000,    11, -1000,
 -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,    10,
 -1000,     9,     8,     7,     6,     5,     4,     3,     2,     1,
     0,    -1, -1000,  -244,  -208,  -245,  -246,  -247,  -248,  -249,
  -231,  -232,  -250,  -251,  -233,  -234,  -252, -1000, -1000,    -8,
  -253,  -254,  -255, -1000, -1000,  -256,  -262, -1000, -1000,    37,
    38,    31,    -9,   -10,  -267,  -270,  -209,  -210,  -257,  -211,
    37,    38,    38,    30,    29, -1000, -1000,    27,  -227, -1000,
 -1000, -1000, -1000,    26,  -257,  -212,  -213,    37,    37,  -214,
 -1000, -1000,  -215,  -227, -1000,  -216,  -217,  -218,    35,    34,
 -1000, -1000, -1000,  -237, -1000, -1000, -1000 };
int yypgo[]={

     0,    98,    50,    52,    97,    96,    79,    95,    94,    93,
    92,    51,    91,    90,    89,    88,    87,    86,    85,    84,
    83,    53,    81,    80 };
int yyr1[]={

     0,     4,     4,     5,     5,     6,     6,     6,     6,     7,
     1,     1,     8,     8,     8,    12,    13,    15,    15,    14,
    10,    10,    10,    10,    10,    16,    16,    17,    17,    18,
    18,    19,    19,    20,    20,     9,     9,    22,    23,     3,
     3,     3,     3,     3,     2,     2,    21,    11 };
int yyr2[]={

     0,     0,     3,     2,     4,     2,     2,     2,     2,    13,
     1,     3,     2,     2,     2,     7,     2,     7,     7,     7,
     2,     2,     2,     2,     2,    13,    15,    15,    17,     7,
     7,    15,    17,    17,    19,     2,     2,    15,    17,     1,
     3,     3,     3,     3,     1,     3,    11,    11 };
int yychk[]={

 -1000,    -4,    -5,    -6,    -7,    -8,    -9,   -10,   258,   -12,
   -13,   -14,   -22,   -23,   -16,   -17,   -18,   -19,   -20,   272,
   -15,   266,   259,   260,   261,   262,   269,   271,   263,   264,
   267,   270,    -6,    58,    58,    58,    58,    58,    58,    58,
    58,    58,    58,    58,    58,    58,   277,   257,   277,   277,
   277,   277,   277,   268,   268,   277,   277,   268,   268,   277,
    64,   277,   277,   277,   277,   277,   -11,    40,   -21,    40,
    43,    64,    64,   277,   277,    -1,   257,   257,    -3,   273,
   274,   275,   276,   257,   -11,   -21,   -21,    43,    43,    44,
    -2,   265,    44,    -3,   257,   257,   -11,   -11,   257,   257,
    -2,   257,   257,   257,    41,    41,   257 };
int yydef[]={

     1,    -2,     2,     3,     5,     6,     7,     8,     0,    12,
    13,    14,    35,    36,    20,    21,    22,    23,    24,     0,
    16,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     4,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    15,    19,     0,
     0,     0,     0,    29,    30,     0,     0,    17,    18,     0,
     0,     0,     0,     0,     0,     0,    10,     0,    39,     0,
     0,     0,     0,     0,     0,     9,    11,     0,    44,    40,
    41,    42,    43,     0,    39,    25,     0,     0,     0,     0,
    37,    45,     0,    44,    27,    26,    31,     0,     0,     0,
    38,    28,    33,    32,    47,    46,    34 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"INTEGER",	257,
	"A_DUNGEON",	258,
	"BRANCH",	259,
	"CHBRANCH",	260,
	"LEVEL",	261,
	"RNDLEVEL",	262,
	"CHLEVEL",	263,
	"RNDCHLEVEL",	264,
	"UP_OR_DOWN",	265,
	"PROTOFILE",	266,
	"DESCRIPTION",	267,
	"DESCRIPTOR",	268,
	"LEVELDESC",	269,
	"ALIGNMENT",	270,
	"LEVALIGN",	271,
	"ENTRY",	272,
	"STAIR",	273,
	"NO_UP",	274,
	"NO_DOWN",	275,
	"PORTAL",	276,
	"STRING",	277,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"file : /* empty */",
	"file : dungeons",
	"dungeons : dungeon",
	"dungeons : dungeons dungeon",
	"dungeon : dungeonline",
	"dungeon : dungeondesc",
	"dungeon : branches",
	"dungeon : levels",
	"dungeonline : A_DUNGEON ':' STRING STRING rcouple optional_int",
	"optional_int : /* empty */",
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
	"branch_type : /* empty */",
	"branch_type : STAIR",
	"branch_type : NO_UP",
	"branch_type : NO_DOWN",
	"branch_type : PORTAL",
	"direction : /* empty */",
	"direction : UP_OR_DOWN",
	"acouple : '(' INTEGER ',' INTEGER ')'",
	"rcouple : '(' INTEGER ',' INTEGER ')'",
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
		
case 2:
# line 86 "dgn_comp.y"
{
			output_dgn();
		  } break;
case 9:
# line 102 "dgn_comp.y"
{
			init_dungeon();
			strcpy(tmpdungeon[n_dgns].name, yypvt[-3].str);
			if (!strcmp(yypvt[-2].str, "none"))
				tmpdungeon[n_levs].boneschar = '\0';
			else if (yypvt[-2].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmpdungeon[n_dgns].boneschar = yypvt[-2].str[0];
			tmpdungeon[n_dgns].lev.base = couple.base;
			tmpdungeon[n_dgns].lev.rand = couple.rand;
			tmpdungeon[n_dgns].chance = yypvt[-0].i;
		  } break;
case 10:
# line 118 "dgn_comp.y"
{
			yyval.i = 0;
		  } break;
case 11:
# line 122 "dgn_comp.y"
{
			yyval.i = yypvt[-0].i;
		  } break;
case 15:
# line 133 "dgn_comp.y"
{
			tmpdungeon[n_dgns].entry_lev = yypvt[-0].i;
		  } break;
case 17:
# line 142 "dgn_comp.y"
{
			if(yypvt[-0].i <= TOWN || yypvt[-0].i >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmpdungeon[n_dgns].flags |= yypvt[-0].i ;
		  } break;
case 18:
# line 149 "dgn_comp.y"
{
			if(yypvt[-0].i && yypvt[-0].i < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmpdungeon[n_dgns].flags |= yypvt[-0].i ;
		  } break;
case 19:
# line 158 "dgn_comp.y"
{
			strcpy(tmpdungeon[n_dgns].protoname, yypvt[-0].str);
		  } break;
case 25:
# line 171 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-3].str);
			if (!strcmp(yypvt[-2].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-2].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-2].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmpdungeon[n_dgns].levels++;
		  } break;
case 26:
# line 185 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-4].str);
			if (!strcmp(yypvt[-3].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-3].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-3].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = yypvt[-0].i;
			tmpdungeon[n_dgns].levels++;
		  } break;
case 27:
# line 202 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-4].str);
			if (!strcmp(yypvt[-3].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-3].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-3].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yypvt[-0].i;
			tmpdungeon[n_dgns].levels++;
		  } break;
case 28:
# line 217 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-5].str);
			if (!strcmp(yypvt[-4].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-4].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-4].str[0];
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yypvt[-1].i;
			tmplevel[n_levs].rndlevs = yypvt[-0].i;
			tmpdungeon[n_dgns].levels++;
		  } break;
case 29:
# line 235 "dgn_comp.y"
{
			if(yypvt[-0].i >= D_ALIGN_CHAOTIC)
			    yyerror("Illegal description - ignoring!");
			else
			    tmplevel[n_levs].flags |= yypvt[-0].i ;
		  } break;
case 30:
# line 242 "dgn_comp.y"
{
			if(yypvt[-0].i && yypvt[-0].i < D_ALIGN_CHAOTIC)
			    yyerror("Illegal alignment - ignoring!");
			else
			    tmplevel[n_levs].flags |= yypvt[-0].i ;
		  } break;
case 31:
# line 251 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-4].str);
			if (!strcmp(yypvt[-3].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-3].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-3].str[0];
			tmplevel[n_levs].chain = getchain(yypvt[-2].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  } break;
case 32:
# line 267 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-5].str);
			if (!strcmp(yypvt[-4].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-4].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-4].str[0];
			tmplevel[n_levs].chain = getchain(yypvt[-3].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].rndlevs = yypvt[-0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  } break;
case 33:
# line 286 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-5].str);
			if (!strcmp(yypvt[-4].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-4].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-4].str[0];
			tmplevel[n_levs].chain = getchain(yypvt[-3].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yypvt[-0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  } break;
case 34:
# line 303 "dgn_comp.y"
{
			init_level();
			strcpy(tmplevel[n_levs].name, yypvt[-6].str);
			if (!strcmp(yypvt[-5].str, "none"))
				tmplevel[n_levs].boneschar = '\0';
			else if (yypvt[-5].str[1])
				yyerror("Bones marker must be a single char, or \"none\"!");
			else
				tmplevel[n_levs].boneschar = yypvt[-5].str[0];
			tmplevel[n_levs].chain = getchain(yypvt[-4].str);
			tmplevel[n_levs].lev.base = couple.base;
			tmplevel[n_levs].lev.rand = couple.rand;
			tmplevel[n_levs].chance = yypvt[-1].i;
			tmplevel[n_levs].rndlevs = yypvt[-0].i;
			if(!check_level()) n_levs--;
			else tmpdungeon[n_dgns].levels++;
		  } break;
case 37:
# line 327 "dgn_comp.y"
{
			init_branch();
			strcpy(tmpbranch[n_brs].name, yypvt[-4].str);
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = yypvt[-1].i;
			tmpbranch[n_brs].up = yypvt[-0].i;
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
		  } break;
case 38:
# line 340 "dgn_comp.y"
{
			init_branch();
			strcpy(tmpbranch[n_brs].name, yypvt[-5].str);
			tmpbranch[n_brs].chain = getchain(yypvt[-4].str);
			tmpbranch[n_brs].lev.base = couple.base;
			tmpbranch[n_brs].lev.rand = couple.rand;
			tmpbranch[n_brs].type = yypvt[-1].i;
			tmpbranch[n_brs].up = yypvt[-0].i;
			if(!check_branch()) n_brs--;
			else tmpdungeon[n_dgns].branches++;
		  } break;
case 39:
# line 354 "dgn_comp.y"
{
			yyval.i = TBR_STAIR;	/* two way stair */
		  } break;
case 40:
# line 358 "dgn_comp.y"
{
			yyval.i = TBR_STAIR;	/* two way stair */
		  } break;
case 41:
# line 362 "dgn_comp.y"
{
			yyval.i = TBR_NO_UP;	/* no up staircase */
		  } break;
case 42:
# line 366 "dgn_comp.y"
{
			yyval.i = TBR_NO_DOWN;	/* no down staircase */
		  } break;
case 43:
# line 370 "dgn_comp.y"
{
			yyval.i = TBR_PORTAL;	/* portal connection */
		  } break;
case 44:
# line 376 "dgn_comp.y"
{
			yyval.i = 0;	/* defaults to down */
		  } break;
case 45:
# line 380 "dgn_comp.y"
{
			yyval.i = yypvt[-0].i;
		  } break;
case 46:
# line 403 "dgn_comp.y"
{
			if (yypvt[-3].i < -MAXLEVEL || yypvt[-3].i > MAXLEVEL) {
			    yyerror("Abs base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else if (yypvt[-1].i < -1 ||
				((yypvt[-3].i < 0) ? (MAXLEVEL + yypvt[-3].i + yypvt[-1].i + 1) > MAXLEVEL :
					(yypvt[-3].i + yypvt[-1].i) > MAXLEVEL)) {
			    yyerror("Abs range out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = yypvt[-3].i;
			    couple.rand = yypvt[-1].i;
			}
		  } break;
case 47:
# line 440 "dgn_comp.y"
{
			if (yypvt[-3].i < -MAXLEVEL || yypvt[-3].i > MAXLEVEL) {
			    yyerror("Rel base out of dlevel range - zeroing!");
			    couple.base = couple.rand = 0;
			} else {
			    couple.base = yypvt[-3].i;
			    couple.rand = yypvt[-1].i;
			}
		  } break;
	}
	goto yystack;		/* reset registers in driver code */
}
