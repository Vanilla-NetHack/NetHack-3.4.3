# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/*	SCCS Id: @(#)lev_lex.c	3.2	96/05/16	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#define LEV_LEX_C

#include "hack.h"
#include "lev_comp.h"
#include "sp_lev.h"

/* Most of these don't exist in flex, yywrap is macro and
 * yyunput is properly declared in flex.skel.
 */
#if !defined(FLEX_SCANNER) && !defined(FLEXHACK_SCANNER)
int FDECL(yyback, (int *,int));
int NDECL(yylook);
int NDECL(yyinput);
int NDECL(yywrap);
int NDECL(yylex);
	/* Traditional lexes let yyunput() and yyoutput() default to int;
	 * newer ones may declare them as void since they don't return
	 * values.  For even more fun, the lex supplied as part of the
	 * newer unbundled compiler for SunOS 4.x adds the void declarations
	 * (under __STDC__ or _cplusplus ifdefs -- otherwise they remain
	 * int) while the bundled lex and the one with the older unbundled
	 * compiler do not.  To detect this, we need help from outside --
	 * sys/unix/Makefile.utl.
	 */
# if defined(NeXT) || defined(SVR4) || defined(_AIX32)
#  define VOIDYYPUT
# endif
# if !defined(VOIDYYPUT)
#  if defined(POSIX_TYPES) && !defined(BOS) && !defined(HISX) && !defined(_M_UNIX) && !defined(VMS)
#   define VOIDYYPUT
#  endif
# endif
# if !defined(VOIDYYPUT) && defined(WEIRD_LEX)
#  if defined(SUNOS4) && defined(__STDC__) && (WEIRD_LEX > 1)
#   define VOIDYYPUT
#  endif
# endif
# ifdef VOIDYYPUT
void FDECL(yyunput, (int));
void FDECL(yyoutput, (int));
# else
int FDECL(yyunput, (int));
int FDECL(yyoutput, (int));
# endif
#endif	/* !FLEX_SCANNER && !FLEXHACK_SCANNER */

#ifdef FLEX_SCANNER
#define YY_MALLOC_DECL \
	      genericptr_t FDECL(malloc, (size_t)); \
	      genericptr_t FDECL(realloc, (genericptr_t,size_t));
#endif

void FDECL(init_yyin, (FILE *));
void FDECL(init_yyout, (FILE *));

/*
 * This doesn't always get put in lev_comp.h
 * (esp. when using older versions of bison).
 */
extern YYSTYPE yylval;

int line_number = 1, colon_line_number = 1;
static char map[4096];
static int map_cnt = 0;

/*
 *	This is a hack required by Michael Hamel to get things
 *	working on the Mac.
 */
#if defined(applec) && !defined(FLEX_SCANNER) && !defined(FLEXHACK_SCANNER)
#undef input
#undef unput
#define unput(c) { yytchar = (c); if (yytchar == 10) yylineno--; *yysptr++ = yytchar; }
# ifndef YYNEWLINE
# define YYNEWLINE 10
# endif

char
input()		/* Under MPW \n is chr(13)! Compensate for this. */
{
	if (yysptr > yysbuf) return(*--yysptr);
	else {
		yytchar = getc(yyin);
		if (yytchar == '\n') {
		    yylineno++;
		    return(YYNEWLINE);
		}
		if (yytchar == EOF) return(0);
		else		    return(yytchar);
	}
}
#endif	/* applec && !FLEX_SCANNER && !FLEXHACK_SCANNER */

# define MAPC 2
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
{
		  BEGIN(INITIAL);
		  yylval.map = (char *) alloc(map_cnt + 1);
		  (void) strncpy(yylval.map, map, map_cnt);
		  yylval.map[map_cnt] = 0;
		  map_cnt = 0;
		  return MAP_ID;
		}
break;
case 2:
{
		  line_number++;
		  (void) strncpy(map + map_cnt, yytext, yyleng);
		  map_cnt += yyleng;
		  map[map_cnt] = 0;
		}
break;
case 3:
	{ line_number++; }
break;
case 4:
	{ colon_line_number = line_number; return ':'; }
break;
case 5:
	return MESSAGE_ID;
break;
case 6:
	return MAZE_ID;
break;
case 7:
	return NOMAP_ID;
break;
case 8:
	return LEVEL_ID;
break;
case 9:
return LEV_INIT_ID;
break;
case 10:
	return FLAGS_ID;
break;
case 11:
return GEOMETRY_ID;
break;
case 12:
	{ BEGIN(MAPC); line_number++; }
break;
case 13:
	return OBJECT_ID;
break;
case 14:
return COBJECT_ID;
break;
case 15:
	return MONSTER_ID;
break;
case 16:
	return TRAP_ID;
break;
case 17:
	return DOOR_ID;
break;
case 18:
return DRAWBRIDGE_ID;
break;
case 19:
return MAZEWALK_ID;
break;
case 20:
	return WALLIFY_ID;
break;
case 21:
	return REGION_ID;
break;
case 22:
return RANDOM_OBJECTS_ID;
break;
case 23:
return RANDOM_MONSTERS_ID;
break;
case 24:
return RANDOM_PLACES_ID;
break;
case 25:
	return ALTAR_ID;
break;
case 26:
	return LADDER_ID;
break;
case 27:
	return STAIR_ID;
break;
case 28:
	return PORTAL_ID;
break;
case 29:
return TELEPRT_ID;
break;
case 30:
	return BRANCH_ID;
break;
case 31:
return FOUNTAIN_ID;
break;
case 32:
	return SINK_ID;
break;
case 33:
	return POOL_ID;
break;
case 34:
return NON_DIGGABLE_ID;
break;
case 35:
return NON_PASSWALL_ID;
break;
case 36:
	return ROOM_ID;
break;
case 37:
	return SUBROOM_ID;
break;
case 38:
return RAND_CORRIDOR_ID;
break;
case 39:
return CORRIDOR_ID;
break;
case 40:
	return GOLD_ID;
break;
case 41:
return ENGRAVING_ID;
break;
case 42:
	return NAME_ID;
break;
case 43:
	return CHANCE_ID;
break;
case 44:
return LEV;
break;
case 45:
	{ yylval.i=D_ISOPEN; return DOOR_STATE; }
break;
case 46:
	{ yylval.i=D_CLOSED; return DOOR_STATE; }
break;
case 47:
	{ yylval.i=D_LOCKED; return DOOR_STATE; }
break;
case 48:
	{ yylval.i=D_NODOOR; return DOOR_STATE; }
break;
case 49:
	{ yylval.i=D_BROKEN; return DOOR_STATE; }
break;
case 50:
	{ yylval.i=W_NORTH; return DIRECTION; }
break;
case 51:
	{ yylval.i=W_EAST; return DIRECTION; }
break;
case 52:
	{ yylval.i=W_SOUTH; return DIRECTION; }
break;
case 53:
	{ yylval.i=W_WEST; return DIRECTION; }
break;
case 54:
	{ yylval.i = -1; return RANDOM_TYPE; }
break;
case 55:
	{ yylval.i = -2; return NONE; }
break;
case 56:
	return O_REGISTER;
break;
case 57:
	return M_REGISTER;
break;
case 58:
	return P_REGISTER;
break;
case 59:
	return A_REGISTER;
break;
case 60:
	{ yylval.i=1; return LEFT_OR_RIGHT; }
break;
case 61:
{ yylval.i=2; return LEFT_OR_RIGHT; }
break;
case 62:
	{ yylval.i=3; return CENTER; }
break;
case 63:
{ yylval.i=4; return LEFT_OR_RIGHT; }
break;
case 64:
	{ yylval.i=5; return LEFT_OR_RIGHT; }
break;
case 65:
	{ yylval.i=1; return TOP_OR_BOT; }
break;
case 66:
	{ yylval.i=5; return TOP_OR_BOT; }
break;
case 67:
	{ yylval.i=1; return LIGHT_STATE; }
break;
case 68:
	{ yylval.i=0; return LIGHT_STATE; }
break;
case 69:
	{ yylval.i=0; return FILLING; }
break;
case 70:
{ yylval.i=1; return FILLING; }
break;
case 71:
	{ yylval.i= AM_NONE; return ALIGNMENT; }
break;
case 72:
	{ yylval.i= AM_LAWFUL; return ALIGNMENT; }
break;
case 73:
	{ yylval.i= AM_NEUTRAL; return ALIGNMENT; }
break;
case 74:
	{ yylval.i= AM_CHAOTIC; return ALIGNMENT; }
break;
case 75:
{ yylval.i=1; return MON_ATTITUDE; }
break;
case 76:
	{ yylval.i=0; return MON_ATTITUDE; }
break;
case 77:
	{ yylval.i=1; return MON_ALERTNESS; }
break;
case 78:
	{ yylval.i=0; return MON_ALERTNESS; }
break;
case 79:
{ yylval.i= M_AP_FURNITURE; return MON_APPEARANCE; }
break;
case 80:
{ yylval.i= M_AP_MONSTER;   return MON_APPEARANCE; }
break;
case 81:
{ yylval.i= M_AP_OBJECT;    return MON_APPEARANCE; }
break;
case 82:
	{ yylval.i=2; return ALTAR_TYPE; }
break;
case 83:
	{ yylval.i=1; return ALTAR_TYPE; }
break;
case 84:
	{ yylval.i=0; return ALTAR_TYPE; }
break;
case 85:
	{ yylval.i=1; return UP_OR_DOWN; }
break;
case 86:
	{ yylval.i=0; return UP_OR_DOWN; }
break;
case 87:
	{ yylval.i=0; return BOOLEAN; }
break;
case 88:
	{ yylval.i=1; return BOOLEAN; }
break;
case 89:
	{ yylval.i=DUST; return ENGRAVING_TYPE; }
break;
case 90:
	{ yylval.i=ENGRAVE; return ENGRAVING_TYPE; }
break;
case 91:
	{ yylval.i=BURN; return ENGRAVING_TYPE; }
break;
case 92:
	{ yylval.i=MARK; return ENGRAVING_TYPE; }
break;
case 93:
	{ yylval.i=1; return CURSE_TYPE; }
break;
case 94:
{ yylval.i=2; return CURSE_TYPE; }
break;
case 95:
	{ yylval.i=3; return CURSE_TYPE; }
break;
case 96:
{ return CONTAINED; }
break;
case 97:
{ yylval.i=NOTELEPORT; return FLAG_TYPE; }
break;
case 98:
{ yylval.i=HARDFLOOR; return FLAG_TYPE; }
break;
case 99:
	{ yylval.i=NOMMAP; return FLAG_TYPE; }
break;
case 100:
{ yylval.i=SHORTSIGHTED; return FLAG_TYPE; }
break;
case 101:
{ yylval.i = atoi(yytext + 1); return PERCENT; }
break;
case 102:
{ yylval.i=atoi(yytext); return INTEGER; }
break;
case 103:
{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  Strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
break;
case 104:
	{ line_number++; }
break;
case 105:
	;
break;
case 106:
	{ yylval.i = yytext[1]; return CHAR; }
break;
case 107:
	{ return yytext[0]; }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
#ifdef	AMIGA
long *alloc(n)
	unsigned n;
{
	return ((long *)malloc (n));
}
#endif

/* routine to switch to another input file; needed for flex */
void init_yyin( input_f )
FILE *input_f;
{
#if defined(FLEX_SCANNER) || defined(FLEXHACK_SCANNER)
	if (yyin)
	    yyrestart(input_f);
	else
#endif
	    yyin = input_f;
}
/* analogous routine (for completeness) */
void init_yyout( output_f )
FILE *output_f;
{
	yyout = output_f;
}

/*lev_comp.l*/
int yyvstop[] = {
0,

107,
0,

105,
107,
0,

104,
0,

107,
0,

107,
0,

107,
0,

102,
107,
0,

4,
107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

2,
104,
0,

105,
107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

107,
0,

105,
0,

103,
0,

102,
0,

85,
0,

3,
0,

2,
0,

105,
0,

2,
3,
0,

106,
0,

72,
0,

67,
0,

65,
0,

17,
0,

40,
0,

6,
0,

42,
0,

33,
0,

36,
0,

32,
0,

16,
0,

101,
0,

91,
0,

86,
0,

89,
0,

51,
0,

60,
0,

92,
0,

55,
0,

45,
0,

88,
0,

53,
0,

12,
0,

25,
0,

10,
0,

8,
0,

7,
0,

27,
0,

59,
0,

84,
0,

78,
0,

74,
0,

87,
0,

50,
0,

58,
0,

64,
0,

52,
0,

68,
0,

30,
0,

43,
0,

26,
0,

13,
0,

28,
0,

21,
0,

77,
0,

66,
0,

49,
0,

62,
0,

46,
0,

95,
0,

69,
0,

47,
0,

48,
0,

99,
0,

56,
0,

54,
0,

83,
0,

1,
0,

5,
0,

15,
0,

37,
0,

20,
0,

93,
0,

90,
0,

76,
0,

57,
0,

73,
0,

71,
0,

82,
0,

39,
0,

31,
0,

11,
0,

9,
0,

19,
0,

81,
0,

75,
0,

94,
0,

70,
0,

14,
0,

41,
0,

96,
0,

61,
0,

98,
0,

44,
0,

79,
0,

80,
0,

18,
0,

63,
0,

97,
0,

34,
0,

35,
0,

100,
0,

24,
0,

22,
0,

23,
0,

29,
0,

38,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,5,	0,0,	
0,0,	0,0,	0,0,	0,0,	
8,65,	0,0,	1,6,	1,7,	
0,0,	0,0,	6,64,	0,0,	
8,65,	8,65,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,6,	0,0,	1,8,	
1,5,	6,64,	0,0,	8,65,	
1,9,	8,66,	8,65,	0,0,	
1,10,	0,0,	0,0,	0,0,	
0,0,	1,11,	8,65,	0,0,	
0,0,	0,0,	0,0,	8,65,	
0,0,	0,0,	0,0,	1,12,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,13,	1,14,	
1,15,	1,16,	1,17,	1,18,	
1,19,	0,0,	1,20,	0,0,	
24,88,	1,21,	1,22,	1,23,	
1,24,	1,25,	29,98,	1,26,	
1,27,	1,28,	13,69,	15,71,	
1,29,	14,70,	16,73,	17,75,	
1,30,	16,74,	15,72,	20,80,	
19,78,	25,89,	1,31,	1,32,	
1,33,	1,34,	1,35,	1,36,	
18,76,	1,37,	19,79,	18,77,	
28,96,	1,38,	1,39,	1,40,	
1,41,	1,42,	2,5,	1,43,	
1,44,	1,45,	1,46,	47,143,	
1,47,	28,97,	2,6,	2,7,	
10,68,	10,68,	10,68,	10,68,	
10,68,	10,68,	10,68,	10,68,	
10,68,	10,68,	21,81,	22,83,	
23,86,	26,90,	21,82,	22,84,	
34,113,	26,91,	27,93,	49,146,	
35,115,	2,6,	34,114,	2,8,	
2,48,	22,85,	23,87,	26,92,	
2,9,	27,94,	27,95,	31,101,	
2,10,	35,116,	37,119,	2,5,	
36,117,	2,11,	31,102,	41,130,	
32,104,	40,128,	31,103,	32,105,	
36,118,	42,132,	32,106,	2,12,	
37,120,	32,107,	54,150,	40,129,	
42,133,	41,131,	2,13,	2,14,	
2,15,	2,16,	2,17,	2,18,	
2,19,	2,5,	2,20,	55,70,	
2,5,	2,21,	2,49,	2,23,	
2,24,	2,25,	38,121,	2,26,	
2,27,	2,28,	38,122,	43,134,	
2,29,	57,152,	38,123,	45,139,	
2,30,	2,5,	45,140,	43,135,	
38,124,	56,151,	2,31,	2,32,	
2,33,	2,34,	2,35,	2,36,	
56,72,	2,37,	3,50,	58,80,	
60,89,	2,38,	2,39,	2,40,	
2,41,	2,42,	62,155,	2,43,	
2,44,	2,45,	2,46,	67,158,	
2,47,	39,125,	44,136,	39,126,	
2,5,	2,5,	2,5,	30,99,	
3,51,	44,137,	46,141,	3,52,	
46,142,	69,159,	59,153,	3,9,	
44,138,	39,127,	59,82,	3,53,	
70,160,	71,161,	73,164,	30,100,	
30,100,	30,100,	30,100,	30,100,	
30,100,	30,100,	30,100,	30,100,	
30,100,	72,162,	3,12,	74,165,	
75,166,	72,163,	76,167,	61,154,	
77,168,	3,54,	3,55,	3,56,	
3,16,	3,57,	3,18,	3,19,	
78,169,	3,58,	61,94,	61,95,	
3,59,	3,22,	3,23,	3,24,	
3,60,	79,170,	3,26,	3,61,	
3,28,	80,171,	81,172,	3,62,	
82,173,	83,174,	84,175,	3,30,	
85,176,	86,177,	87,178,	87,179,	
4,50,	3,31,	3,32,	3,33,	
3,34,	3,35,	3,36,	88,180,	
3,37,	90,183,	63,156,	91,184,	
3,38,	3,39,	3,40,	3,41,	
3,42,	92,185,	3,43,	3,44,	
3,45,	3,46,	4,51,	3,47,	
33,108,	4,63,	93,186,	33,109,	
94,187,	4,9,	95,188,	33,110,	
63,157,	4,53,	33,111,	63,157,	
4,52,	96,189,	89,181,	97,190,	
33,112,	89,182,	98,191,	63,157,	
102,195,	103,196,	104,197,	105,198,	
4,12,	106,199,	107,200,	108,201,	
109,202,	101,193,	110,203,	4,54,	
4,55,	4,56,	4,16,	4,57,	
4,18,	4,19,	4,52,	4,58,	
101,194,	4,52,	4,59,	4,49,	
4,23,	4,24,	4,60,	111,204,	
4,26,	4,61,	4,28,	112,205,	
113,206,	4,62,	114,207,	115,208,	
116,209,	4,30,	4,52,	117,210,	
118,211,	120,214,	121,215,	4,31,	
4,32,	4,33,	4,34,	4,35,	
4,36,	123,218,	4,37,	124,219,	
126,223,	127,224,	4,38,	4,39,	
4,40,	4,41,	4,42,	9,67,	
4,43,	4,44,	4,45,	4,46,	
122,216,	4,47,	48,144,	9,67,	
9,0,	4,52,	4,52,	4,52,	
119,212,	128,225,	48,144,	48,145,	
125,220,	130,232,	119,213,	131,233,	
122,217,	51,64,	51,147,	125,221,	
132,234,	125,222,	133,235,	134,236,	
135,237,	136,238,	9,67,	137,239,	
9,67,	9,67,	137,240,	138,241,	
139,242,	48,144,	140,243,	48,144,	
48,144,	9,67,	141,244,	143,247,	
51,148,	141,245,	9,67,	51,149,	
48,144,	146,248,	150,159,	141,246,	
151,249,	48,144,	153,172,	51,149,	
154,186,	51,149,	51,149,	146,174,	
155,251,	52,147,	53,68,	53,68,	
53,68,	53,68,	53,68,	53,68,	
53,68,	53,68,	53,68,	53,68,	
152,250,	159,252,	160,253,	152,166,	
161,254,	51,149,	51,149,	51,149,	
162,255,	163,256,	164,257,	52,149,	
51,149,	51,149,	52,149,	51,149,	
51,149,	157,156,	129,226,	165,258,	
51,149,	129,227,	52,149,	51,149,	
52,149,	52,149,	166,259,	51,149,	
167,260,	168,261,	129,228,	129,229,	
51,149,	169,262,	170,263,	129,230,	
171,264,	129,231,	172,265,	157,157,	
173,266,	174,267,	157,157,	175,268,	
52,149,	52,149,	52,149,	176,269,	
177,270,	178,271,	157,157,	52,149,	
52,149,	179,272,	52,149,	52,149,	
180,273,	181,274,	182,275,	52,149,	
183,276,	100,192,	52,149,	51,149,	
51,149,	51,149,	52,149,	184,277,	
185,278,	186,279,	187,280,	52,149,	
100,100,	100,100,	100,100,	100,100,	
100,100,	100,100,	100,100,	100,100,	
100,100,	100,100,	188,281,	189,282,	
190,283,	191,284,	192,192,	193,286,	
194,287,	195,288,	196,289,	197,290,	
198,291,	199,292,	200,293,	201,294,	
202,295,	203,296,	204,297,	205,298,	
206,299,	207,300,	52,149,	52,149,	
52,149,	208,301,	209,302,	210,303,	
211,304,	212,305,	213,306,	214,307,	
216,308,	217,309,	219,310,	220,311,	
221,312,	222,313,	223,314,	224,315,	
225,316,	226,317,	227,318,	228,319,	
229,320,	230,321,	231,322,	232,323,	
233,324,	234,325,	235,326,	236,327,	
237,328,	238,329,	239,330,	240,331,	
241,332,	243,333,	244,334,	245,335,	
246,336,	247,337,	248,338,	249,254,	
250,339,	251,340,	252,341,	192,285,	
253,342,	254,343,	255,344,	256,345,	
258,346,	259,347,	260,348,	261,349,	
262,350,	264,351,	265,352,	266,353,	
267,354,	268,355,	269,356,	271,357,	
272,358,	273,360,	275,361,	276,362,	
277,363,	280,364,	281,365,	282,366,	
284,367,	286,368,	287,369,	288,370,	
272,359,	289,371,	290,372,	291,373,	
292,374,	294,375,	295,376,	296,377,	
297,378,	298,379,	302,380,	303,381,	
304,382,	305,383,	306,384,	307,385,	
309,386,	310,387,	311,388,	312,389,	
313,390,	315,391,	316,392,	317,393,	
318,394,	319,395,	321,396,	322,397,	
323,398,	325,399,	326,400,	327,401,	
328,402,	329,403,	330,404,	331,405,	
332,406,	334,407,	335,408,	336,409,	
339,410,	340,411,	342,412,	343,413,	
344,414,	345,415,	346,416,	347,417,	
349,418,	350,419,	351,420,	352,421,	
354,422,	355,423,	356,424,	358,425,	
359,426,	360,427,	361,428,	362,429,	
363,430,	365,431,	366,432,	367,433,	
370,434,	372,435,	373,436,	374,437,	
375,438,	377,439,	378,440,	379,441,	
380,442,	382,443,	383,444,	384,446,	
385,447,	386,448,	387,449,	388,450,	
383,445,	389,451,	390,452,	391,453,	
392,454,	393,455,	394,456,	395,457,	
397,458,	398,459,	399,460,	401,461,	
403,462,	404,463,	405,464,	407,465,	
408,466,	410,467,	411,433,	414,468,	
415,469,	416,470,	417,471,	418,472,	
419,473,	420,474,	422,475,	423,476,	
424,477,	425,478,	426,479,	429,480,	
431,481,	432,482,	433,483,	435,484,	
440,485,	442,486,	444,487,	445,488,	
446,489,	447,490,	448,491,	450,492,	
451,493,	452,494,	453,495,	454,496,	
455,497,	458,498,	460,499,	462,500,	
463,501,	465,502,	466,503,	468,504,	
469,505,	470,506,	471,507,	472,508,	
473,509,	474,510,	475,511,	478,512,	
479,513,	480,514,	482,518,	485,519,	
487,520,	488,521,	489,522,	491,523,	
492,524,	493,525,	494,526,	480,515,	
498,527,	480,516,	480,517,	499,528,	
501,529,	502,530,	503,531,	504,532,	
506,533,	507,534,	512,535,	513,536,	
514,537,	515,538,	516,539,	517,540,	
518,541,	519,542,	520,543,	521,544,	
522,545,	523,546,	524,547,	525,548,	
527,549,	529,550,	533,551,	535,552,	
536,553,	537,554,	538,555,	539,556,	
540,557,	541,558,	544,559,	549,560,	
550,561,	552,562,	553,563,	554,564,	
555,565,	556,566,	557,567,	558,568,	
561,569,	562,570,	563,571,	564,572,	
565,573,	566,574,	567,575,	568,576,	
569,577,	572,578,	573,579,	574,580,	
575,581,	576,582,	578,583,	579,584,	
580,585,	582,586,	583,587,	584,588,	
586,589,	587,590,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-113,	0,		0,	
yycrank+-208,	yysvec+1,	0,	
yycrank+-294,	yysvec+2,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+5,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+-7,	0,		yyvstop+8,
yycrank+-406,	0,		yyvstop+10,
yycrank+76,	0,		yyvstop+12,
yycrank+0,	yysvec+10,	yyvstop+14,
yycrank+0,	0,		yyvstop+17,
yycrank+10,	0,		yyvstop+20,
yycrank+7,	0,		yyvstop+22,
yycrank+15,	0,		yyvstop+24,
yycrank+11,	0,		yyvstop+26,
yycrank+13,	0,		yyvstop+28,
yycrank+28,	0,		yyvstop+30,
yycrank+27,	0,		yyvstop+32,
yycrank+17,	0,		yyvstop+34,
yycrank+69,	0,		yyvstop+36,
yycrank+70,	0,		yyvstop+38,
yycrank+71,	0,		yyvstop+40,
yycrank+10,	0,		yyvstop+42,
yycrank+18,	0,		yyvstop+44,
yycrank+72,	0,		yyvstop+46,
yycrank+69,	0,		yyvstop+48,
yycrank+39,	0,		yyvstop+50,
yycrank+17,	0,		yyvstop+52,
yycrank+207,	0,		yyvstop+54,
yycrank+47,	0,		yyvstop+56,
yycrank+56,	0,		yyvstop+58,
yycrank+227,	0,		yyvstop+60,
yycrank+29,	0,		yyvstop+62,
yycrank+47,	0,		yyvstop+64,
yycrank+63,	0,		yyvstop+66,
yycrank+61,	0,		yyvstop+68,
yycrank+97,	0,		yyvstop+70,
yycrank+138,	0,		yyvstop+72,
yycrank+64,	0,		yyvstop+74,
yycrank+65,	0,		yyvstop+76,
yycrank+68,	0,		yyvstop+78,
yycrank+102,	0,		yyvstop+80,
yycrank+137,	0,		yyvstop+82,
yycrank+92,	0,		yyvstop+84,
yycrank+132,	0,		yyvstop+86,
yycrank+18,	0,		yyvstop+88,
yycrank+-413,	0,		yyvstop+90,
yycrank+78,	yysvec+22,	yyvstop+92,
yycrank+0,	0,		yyvstop+94,
yycrank+420,	0,		yyvstop+97,
yycrank+459,	0,		yyvstop+100,
yycrank+422,	yysvec+52,	yyvstop+102,
yycrank+98,	yysvec+52,	yyvstop+104,
yycrank+105,	yysvec+52,	yyvstop+106,
yycrank+137,	yysvec+52,	yyvstop+108,
yycrank+123,	0,		yyvstop+110,
yycrank+141,	yysvec+52,	yyvstop+112,
yycrank+181,	yysvec+52,	yyvstop+114,
yycrank+141,	yysvec+52,	yyvstop+116,
yycrank+198,	yysvec+52,	yyvstop+118,
yycrank+161,	yysvec+52,	yyvstop+120,
yycrank+-304,	yysvec+48,	yyvstop+122,
yycrank+0,	yysvec+6,	yyvstop+124,
yycrank+0,	yysvec+8,	0,	
yycrank+0,	0,		yyvstop+126,
yycrank+192,	0,		0,	
yycrank+0,	yysvec+10,	yyvstop+128,
yycrank+161,	0,		0,	
yycrank+187,	0,		0,	
yycrank+188,	0,		0,	
yycrank+187,	0,		0,	
yycrank+175,	0,		0,	
yycrank+202,	0,		0,	
yycrank+197,	0,		0,	
yycrank+205,	0,		0,	
yycrank+187,	0,		0,	
yycrank+201,	0,		0,	
yycrank+213,	0,		0,	
yycrank+220,	0,		0,	
yycrank+226,	0,		0,	
yycrank+210,	0,		0,	
yycrank+207,	0,		0,	
yycrank+215,	0,		0,	
yycrank+222,	0,		0,	
yycrank+224,	0,		0,	
yycrank+225,	0,		0,	
yycrank+237,	0,		0,	
yycrank+263,	0,		0,	
yycrank+235,	0,		0,	
yycrank+244,	0,		0,	
yycrank+242,	0,		0,	
yycrank+252,	0,		0,	
yycrank+267,	0,		0,	
yycrank+268,	0,		0,	
yycrank+265,	0,		0,	
yycrank+278,	0,		0,	
yycrank+270,	0,		0,	
yycrank+0,	yysvec+30,	0,	
yycrank+504,	0,		0,	
yycrank+252,	0,		0,	
yycrank+240,	0,		0,	
yycrank+252,	0,		0,	
yycrank+249,	0,		0,	
yycrank+235,	0,		0,	
yycrank+242,	0,		0,	
yycrank+240,	0,		0,	
yycrank+245,	0,		0,	
yycrank+259,	0,		0,	
yycrank+247,	0,		0,	
yycrank+265,	0,		0,	
yycrank+265,	0,		0,	
yycrank+261,	0,		0,	
yycrank+267,	0,		0,	
yycrank+268,	0,		0,	
yycrank+281,	0,		0,	
yycrank+279,	0,		0,	
yycrank+280,	0,		0,	
yycrank+312,	0,		0,	
yycrank+274,	0,		0,	
yycrank+271,	0,		0,	
yycrank+310,	0,		0,	
yycrank+281,	0,		0,	
yycrank+300,	0,		0,	
yycrank+322,	0,		0,	
yycrank+286,	0,		0,	
yycrank+291,	0,		0,	
yycrank+304,	0,		0,	
yycrank+401,	0,		0,	
yycrank+319,	0,		0,	
yycrank+326,	0,		0,	
yycrank+335,	0,		0,	
yycrank+337,	0,		0,	
yycrank+325,	0,		0,	
yycrank+333,	0,		0,	
yycrank+327,	0,		0,	
yycrank+328,	0,		0,	
yycrank+326,	0,		0,	
yycrank+332,	0,		0,	
yycrank+329,	0,		0,	
yycrank+351,	0,		0,	
yycrank+0,	0,		yyvstop+130,
yycrank+336,	0,		0,	
yycrank+0,	yysvec+48,	0,	
yycrank+0,	0,		yyvstop+132,
yycrank+377,	0,		0,	
yycrank+0,	0,		yyvstop+134,
yycrank+0,	yysvec+51,	yyvstop+136,
yycrank+0,	yysvec+52,	0,	
yycrank+374,	yysvec+52,	0,	
yycrank+395,	yysvec+52,	0,	
yycrank+412,	0,		0,	
yycrank+394,	yysvec+52,	0,	
yycrank+386,	yysvec+52,	0,	
yycrank+392,	yysvec+52,	0,	
yycrank+0,	0,		yyvstop+138,
yycrank+-487,	yysvec+48,	0,	
yycrank+0,	0,		yyvstop+141,
yycrank+416,	0,		0,	
yycrank+404,	0,		0,	
yycrank+406,	0,		0,	
yycrank+404,	0,		0,	
yycrank+407,	0,		0,	
yycrank+408,	0,		0,	
yycrank+412,	0,		0,	
yycrank+424,	0,		0,	
yycrank+437,	0,		0,	
yycrank+431,	0,		0,	
yycrank+436,	0,		0,	
yycrank+446,	0,		0,	
yycrank+432,	0,		0,	
yycrank+450,	0,		0,	
yycrank+451,	0,		0,	
yycrank+452,	0,		0,	
yycrank+440,	0,		0,	
yycrank+444,	0,		0,	
yycrank+459,	0,		0,	
yycrank+464,	0,		0,	
yycrank+438,	0,		0,	
yycrank+467,	0,		0,	
yycrank+461,	0,		0,	
yycrank+454,	0,		0,	
yycrank+472,	0,		0,	
yycrank+474,	0,		0,	
yycrank+471,	0,		0,	
yycrank+474,	0,		0,	
yycrank+477,	0,		0,	
yycrank+480,	0,		0,	
yycrank+494,	0,		0,	
yycrank+484,	0,		0,	
yycrank+489,	0,		0,	
yycrank+534,	0,		0,	
yycrank+464,	0,		0,	
yycrank+471,	0,		0,	
yycrank+468,	0,		0,	
yycrank+463,	0,		0,	
yycrank+456,	0,		0,	
yycrank+456,	0,		0,	
yycrank+466,	0,		0,	
yycrank+464,	0,		0,	
yycrank+459,	0,		0,	
yycrank+465,	0,		0,	
yycrank+462,	0,		0,	
yycrank+462,	0,		0,	
yycrank+464,	0,		0,	
yycrank+470,	0,		0,	
yycrank+465,	0,		0,	
yycrank+469,	0,		0,	
yycrank+472,	0,		0,	
yycrank+472,	0,		0,	
yycrank+480,	0,		0,	
yycrank+487,	0,		0,	
yycrank+490,	0,		0,	
yycrank+475,	0,		0,	
yycrank+0,	0,		yyvstop+143,
yycrank+476,	0,		0,	
yycrank+479,	0,		0,	
yycrank+0,	0,		yyvstop+145,
yycrank+487,	0,		0,	
yycrank+494,	0,		0,	
yycrank+485,	0,		0,	
yycrank+499,	0,		0,	
yycrank+491,	0,		0,	
yycrank+484,	0,		0,	
yycrank+484,	0,		0,	
yycrank+493,	0,		0,	
yycrank+491,	0,		0,	
yycrank+494,	0,		0,	
yycrank+503,	0,		0,	
yycrank+489,	0,		0,	
yycrank+505,	0,		0,	
yycrank+506,	0,		0,	
yycrank+498,	0,		0,	
yycrank+510,	0,		0,	
yycrank+511,	0,		0,	
yycrank+511,	0,		0,	
yycrank+508,	0,		0,	
yycrank+514,	0,		0,	
yycrank+500,	0,		0,	
yycrank+510,	0,		0,	
yycrank+500,	0,		0,	
yycrank+0,	0,		yyvstop+147,
yycrank+516,	0,		0,	
yycrank+501,	0,		0,	
yycrank+514,	0,		0,	
yycrank+515,	0,		0,	
yycrank+505,	0,		0,	
yycrank+612,	0,		0,	
yycrank+545,	yysvec+52,	0,	
yycrank+547,	0,		0,	
yycrank+549,	yysvec+52,	0,	
yycrank+544,	0,		0,	
yycrank+561,	0,		0,	
yycrank+562,	0,		0,	
yycrank+565,	0,		0,	
yycrank+558,	0,		0,	
yycrank+0,	0,		yyvstop+149,
yycrank+566,	0,		0,	
yycrank+568,	0,		0,	
yycrank+551,	0,		0,	
yycrank+551,	0,		0,	
yycrank+567,	0,		0,	
yycrank+0,	0,		yyvstop+151,
yycrank+542,	0,		0,	
yycrank+569,	0,		0,	
yycrank+563,	0,		0,	
yycrank+553,	0,		yyvstop+153,
yycrank+576,	0,		0,	
yycrank+558,	0,		0,	
yycrank+0,	0,		yyvstop+155,
yycrank+563,	0,		0,	
yycrank+576,	0,		0,	
yycrank+578,	0,		0,	
yycrank+0,	0,		yyvstop+157,
yycrank+581,	0,		0,	
yycrank+568,	0,		0,	
yycrank+569,	0,		0,	
yycrank+0,	0,		yyvstop+159,
yycrank+0,	0,		yyvstop+161,
yycrank+567,	0,		0,	
yycrank+571,	0,		0,	
yycrank+571,	0,		0,	
yycrank+0,	0,		yyvstop+163,
yycrank+579,	0,		0,	
yycrank+0,	0,		yyvstop+165,
yycrank+543,	0,		0,	
yycrank+540,	0,		0,	
yycrank+554,	0,		0,	
yycrank+556,	0,		0,	
yycrank+543,	0,		0,	
yycrank+548,	0,		0,	
yycrank+559,	0,		0,	
yycrank+0,	0,		yyvstop+167,
yycrank+560,	0,		0,	
yycrank+547,	0,		0,	
yycrank+562,	0,		0,	
yycrank+567,	0,		0,	
yycrank+564,	0,		0,	
yycrank+0,	0,		yyvstop+169,
yycrank+0,	0,		yyvstop+171,
yycrank+0,	0,		yyvstop+173,
yycrank+569,	0,		0,	
yycrank+566,	0,		0,	
yycrank+567,	0,		0,	
yycrank+624,	0,		0,	
yycrank+568,	0,		0,	
yycrank+566,	0,		0,	
yycrank+0,	0,		yyvstop+175,
yycrank+571,	0,		0,	
yycrank+572,	0,		0,	
yycrank+577,	0,		0,	
yycrank+565,	0,		0,	
yycrank+570,	0,		0,	
yycrank+0,	0,		yyvstop+177,
yycrank+561,	0,		0,	
yycrank+564,	0,		0,	
yycrank+574,	0,		0,	
yycrank+569,	0,		0,	
yycrank+584,	0,		0,	
yycrank+0,	0,		yyvstop+179,
yycrank+578,	0,		0,	
yycrank+575,	0,		0,	
yycrank+585,	0,		0,	
yycrank+0,	0,		yyvstop+181,
yycrank+584,	0,		0,	
yycrank+585,	0,		0,	
yycrank+576,	0,		0,	
yycrank+572,	0,		0,	
yycrank+573,	0,		0,	
yycrank+574,	0,		0,	
yycrank+581,	0,		0,	
yycrank+588,	0,		0,	
yycrank+0,	0,		yyvstop+183,
yycrank+579,	0,		0,	
yycrank+586,	0,		0,	
yycrank+579,	0,		0,	
yycrank+0,	0,		yyvstop+185,
yycrank+0,	0,		yyvstop+187,
yycrank+631,	0,		0,	
yycrank+624,	yysvec+52,	0,	
yycrank+0,	0,		yyvstop+189,
yycrank+626,	0,		0,	
yycrank+630,	0,		0,	
yycrank+627,	0,		0,	
yycrank+633,	0,		0,	
yycrank+620,	0,		0,	
yycrank+617,	0,		0,	
yycrank+0,	0,		yyvstop+191,
yycrank+639,	0,		0,	
yycrank+621,	0,		0,	
yycrank+629,	0,		0,	
yycrank+625,	0,		0,	
yycrank+0,	0,		yyvstop+193,
yycrank+643,	0,		0,	
yycrank+638,	0,		0,	
yycrank+641,	0,		0,	
yycrank+0,	0,		yyvstop+195,
yycrank+638,	0,		0,	
yycrank+647,	0,		0,	
yycrank+629,	0,		0,	
yycrank+638,	0,		0,	
yycrank+638,	0,		0,	
yycrank+638,	0,		0,	
yycrank+0,	0,		yyvstop+197,
yycrank+638,	0,		0,	
yycrank+639,	0,		0,	
yycrank+649,	0,		0,	
yycrank+0,	0,		yyvstop+199,
yycrank+0,	0,		yyvstop+201,
yycrank+608,	0,		0,	
yycrank+0,	0,		yyvstop+203,
yycrank+620,	0,		0,	
yycrank+613,	0,		0,	
yycrank+613,	0,		0,	
yycrank+610,	0,		0,	
yycrank+0,	0,		yyvstop+205,
yycrank+625,	0,		0,	
yycrank+621,	0,		0,	
yycrank+627,	0,		0,	
yycrank+610,	0,		0,	
yycrank+0,	0,		yyvstop+207,
yycrank+629,	0,		0,	
yycrank+622,	0,		0,	
yycrank+623,	0,		0,	
yycrank+624,	0,		0,	
yycrank+630,	0,		0,	
yycrank+634,	0,		0,	
yycrank+619,	0,		0,	
yycrank+622,	0,		0,	
yycrank+637,	0,		0,	
yycrank+638,	0,		0,	
yycrank+643,	0,		0,	
yycrank+638,	0,		0,	
yycrank+628,	0,		0,	
yycrank+631,	0,		0,	
yycrank+0,	0,		yyvstop+209,
yycrank+643,	0,		0,	
yycrank+629,	0,		0,	
yycrank+644,	0,		0,	
yycrank+0,	0,		yyvstop+211,
yycrank+638,	0,		0,	
yycrank+0,	0,		yyvstop+213,
yycrank+631,	0,		0,	
yycrank+634,	0,		0,	
yycrank+649,	0,		0,	
yycrank+0,	0,		yyvstop+215,
yycrank+636,	0,		0,	
yycrank+644,	0,		0,	
yycrank+0,	0,		yyvstop+217,
yycrank+673,	0,		0,	
yycrank+684,	yysvec+52,	0,	
yycrank+0,	0,		yyvstop+219,
yycrank+0,	0,		yyvstop+221,
yycrank+677,	0,		0,	
yycrank+677,	0,		0,	
yycrank+684,	0,		0,	
yycrank+685,	0,		0,	
yycrank+686,	0,		0,	
yycrank+678,	0,		0,	
yycrank+696,	0,		0,	
yycrank+0,	0,		yyvstop+223,
yycrank+686,	0,		0,	
yycrank+694,	0,		0,	
yycrank+682,	0,		0,	
yycrank+694,	0,		0,	
yycrank+683,	0,		0,	
yycrank+0,	0,		yyvstop+225,
yycrank+0,	0,		yyvstop+227,
yycrank+672,	0,		0,	
yycrank+0,	0,		yyvstop+229,
yycrank+691,	0,		0,	
yycrank+687,	0,		0,	
yycrank+681,	0,		0,	
yycrank+0,	0,		yyvstop+231,
yycrank+671,	0,		0,	
yycrank+0,	0,		yyvstop+233,
yycrank+0,	0,		yyvstop+235,
yycrank+0,	0,		yyvstop+237,
yycrank+0,	0,		yyvstop+239,
yycrank+662,	0,		0,	
yycrank+0,	0,		yyvstop+241,
yycrank+672,	0,		0,	
yycrank+0,	0,		yyvstop+243,
yycrank+673,	0,		0,	
yycrank+670,	0,		0,	
yycrank+665,	0,		0,	
yycrank+676,	0,		0,	
yycrank+673,	0,		0,	
yycrank+0,	0,		yyvstop+245,
yycrank+662,	0,		0,	
yycrank+664,	0,		0,	
yycrank+682,	0,		0,	
yycrank+668,	0,		0,	
yycrank+675,	0,		0,	
yycrank+674,	0,		0,	
yycrank+0,	0,		yyvstop+247,
yycrank+0,	0,		yyvstop+249,
yycrank+673,	0,		0,	
yycrank+0,	0,		yyvstop+251,
yycrank+669,	0,		0,	
yycrank+0,	0,		yyvstop+253,
yycrank+678,	0,		0,	
yycrank+683,	0,		0,	
yycrank+0,	0,		yyvstop+255,
yycrank+688,	0,		0,	
yycrank+689,	0,		0,	
yycrank+0,	0,		yyvstop+257,
yycrank+722,	0,		0,	
yycrank+710,	0,		0,	
yycrank+725,	0,		0,	
yycrank+716,	0,		0,	
yycrank+717,	0,		0,	
yycrank+707,	0,		0,	
yycrank+717,	0,		0,	
yycrank+723,	0,		0,	
yycrank+0,	0,		yyvstop+259,
yycrank+0,	0,		yyvstop+261,
yycrank+728,	0,		0,	
yycrank+717,	0,		0,	
yycrank+734,	0,		0,	
yycrank+0,	0,		yyvstop+263,
yycrank+718,	0,		0,	
yycrank+0,	0,		yyvstop+265,
yycrank+0,	0,		yyvstop+267,
yycrank+702,	0,		0,	
yycrank+0,	0,		yyvstop+269,
yycrank+702,	0,		0,	
yycrank+702,	0,		0,	
yycrank+695,	0,		0,	
yycrank+0,	0,		yyvstop+271,
yycrank+696,	0,		0,	
yycrank+694,	0,		0,	
yycrank+708,	0,		0,	
yycrank+694,	0,		0,	
yycrank+0,	0,		yyvstop+273,
yycrank+0,	0,		yyvstop+275,
yycrank+0,	0,		yyvstop+277,
yycrank+701,	0,		0,	
yycrank+707,	0,		0,	
yycrank+0,	0,		yyvstop+279,
yycrank+713,	0,		0,	
yycrank+717,	0,		0,	
yycrank+718,	0,		0,	
yycrank+737,	0,		0,	
yycrank+0,	0,		yyvstop+281,
yycrank+749,	0,		0,	
yycrank+750,	0,		0,	
yycrank+0,	0,		yyvstop+283,
yycrank+0,	0,		yyvstop+285,
yycrank+0,	0,		yyvstop+287,
yycrank+0,	0,		yyvstop+289,
yycrank+757,	0,		0,	
yycrank+736,	0,		0,	
yycrank+745,	0,		0,	
yycrank+746,	0,		0,	
yycrank+760,	0,		0,	
yycrank+751,	0,		0,	
yycrank+733,	0,		0,	
yycrank+729,	0,		0,	
yycrank+714,	0,		0,	
yycrank+727,	0,		0,	
yycrank+718,	0,		0,	
yycrank+723,	0,		0,	
yycrank+733,	0,		0,	
yycrank+721,	0,		0,	
yycrank+0,	0,		yyvstop+291,
yycrank+722,	0,		0,	
yycrank+0,	0,		yyvstop+293,
yycrank+733,	0,		0,	
yycrank+0,	0,		yyvstop+295,
yycrank+0,	0,		yyvstop+297,
yycrank+0,	0,		yyvstop+299,
yycrank+769,	0,		0,	
yycrank+0,	0,		yyvstop+301,
yycrank+773,	0,		0,	
yycrank+775,	0,		0,	
yycrank+759,	0,		0,	
yycrank+764,	0,		0,	
yycrank+769,	0,		0,	
yycrank+779,	0,		0,	
yycrank+763,	0,		0,	
yycrank+0,	0,		yyvstop+303,
yycrank+0,	0,		yyvstop+305,
yycrank+730,	0,		0,	
yycrank+0,	0,		yyvstop+307,
yycrank+0,	0,		yyvstop+309,
yycrank+0,	0,		yyvstop+311,
yycrank+0,	0,		yyvstop+313,
yycrank+731,	0,		0,	
yycrank+732,	0,		0,	
yycrank+0,	0,		yyvstop+315,
yycrank+773,	0,		0,	
yycrank+774,	0,		0,	
yycrank+769,	0,		0,	
yycrank+769,	0,		0,	
yycrank+784,	0,		0,	
yycrank+787,	0,		0,	
yycrank+786,	0,		0,	
yycrank+0,	0,		yyvstop+317,
yycrank+0,	0,		yyvstop+319,
yycrank+755,	0,		0,	
yycrank+788,	0,		0,	
yycrank+782,	0,		0,	
yycrank+786,	0,		0,	
yycrank+776,	0,		0,	
yycrank+794,	0,		0,	
yycrank+793,	0,		0,	
yycrank+792,	0,		0,	
yycrank+764,	0,		0,	
yycrank+0,	0,		yyvstop+321,
yycrank+0,	0,		yyvstop+323,
yycrank+797,	0,		0,	
yycrank+797,	0,		0,	
yycrank+783,	0,		0,	
yycrank+785,	0,		0,	
yycrank+796,	0,		0,	
yycrank+0,	0,		yyvstop+325,
yycrank+791,	0,		0,	
yycrank+789,	0,		0,	
yycrank+789,	0,		0,	
yycrank+0,	0,		yyvstop+327,
yycrank+794,	0,		0,	
yycrank+792,	0,		0,	
yycrank+792,	0,		0,	
yycrank+0,	0,		yyvstop+329,
yycrank+798,	0,		0,	
yycrank+794,	0,		0,	
yycrank+0,	0,		yyvstop+331,
yycrank+0,	0,		yyvstop+333,
yycrank+0,	0,		yyvstop+335,
0,	0,	0};
struct yywork *yytop = yycrank+877;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
040 ,01  ,'"' ,'#' ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'+' ,01  ,'+' ,'#' ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'#' ,'#' ,'#' ,01  ,01  ,01  ,01  ,
'#' ,'#' ,01  ,'#' ,'#' ,01  ,01  ,01  ,
'#' ,01  ,01  ,'#' ,01  ,01  ,01  ,'#' ,
01  ,01  ,01  ,01  ,'#' ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'#' ,'#' ,'#' ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
#ifndef lint
static	char ncform_sccsid[] = "@(#)ncform 1.6 88/02/08 SMI"; /* from S5R2 1.2 */
#endif

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
