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
/*	SCCS Id: @(#)lev_lex.c	3.2	96/03/02	*/
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

#ifdef MICRO
#undef exit
# if !defined(MSDOS) && !defined(WIN32)
extern void FDECL(exit, (int));
# endif
#endif

/*
 * This doesn't always get put in lev_comp.h
 * (esp. when using older versions of bison).
 */
extern YYSTYPE yylval;

int line_number = 1, colon_line_number = 1;

/*
 * This is *** UGLY *** but I can't think a better way to do it;
 * I really need a huge buffer to scan maps...
 * (This should probably be `#ifndef FLEX_SCANNER' since it's lex-specific.)
 */
#ifdef YYLMAX
#undef YYLMAX
#endif
#define YYLMAX	2048
#if defined(sun) && defined(SVR4) && !defined(FLEX_SCANNER)
char wwtext[YYLMAX];
#define yytext wwtext
#endif

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
#ifdef FLEX23_BUG
		  /*
		   * There is a bug in Flex 2.3 patch level < 6
		   * (absent in previous versions)
		   * that results in the following behaviour :
		   * Once you enter an yymore(), you never exit from it.
		   * This should do the trick!
		   */
		  extern int yy_more_len;

		  yy_more_len = 0;
#endif	/* FLEX23_BUG */
		  BEGIN(INITIAL);
		  yylval.map = (char *) alloc(yyleng-5);
		  (void) strncpy(yylval.map, yytext,yyleng-6);
		  yylval.map[yyleng-6] = 0;
		  return MAP_ID;
		}
break;
case 2:
{ line_number++; yymore(); }
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
{ yylval.i=atoi(yytext); return INTEGER; }
break;
case 102:
{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  Strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
break;
case 103:
	{ line_number++; }
break;
case 104:
	;
break;
case 105:
	{ yylval.i = yytext[1]; return CHAR; }
break;
case 106:
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

106,
0,

104,
106,
0,

103,
0,

106,
0,

106,
0,

106,
0,

101,
106,
0,

4,
106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

2,
103,
0,

104,
106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

106,
0,

104,
0,

102,
0,

101,
0,

85,
0,

3,
0,

2,
0,

104,
0,

2,
3,
0,

105,
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
8,64,	0,0,	1,6,	1,7,	
0,0,	0,0,	6,63,	0,0,	
8,64,	8,64,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,6,	0,0,	1,8,	
1,5,	6,63,	0,0,	8,64,	
1,9,	8,65,	8,64,	0,0,	
1,10,	0,0,	0,0,	0,0,	
0,0,	1,11,	8,64,	0,0,	
0,0,	0,0,	0,0,	8,64,	
0,0,	0,0,	0,0,	1,12,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,13,	1,14,	
1,15,	1,16,	1,17,	1,18,	
1,19,	24,87,	1,20,	29,97,	
0,0,	1,21,	1,22,	1,23,	
1,24,	1,25,	0,0,	1,26,	
1,27,	1,28,	13,68,	15,70,	
1,29,	14,69,	16,72,	17,74,	
18,75,	16,73,	15,71,	18,76,	
19,77,	20,79,	1,30,	1,31,	
1,32,	1,33,	1,34,	1,35,	
21,80,	1,36,	19,78,	23,85,	
21,81,	1,37,	1,38,	1,39,	
1,40,	1,41,	2,5,	1,42,	
1,43,	1,44,	1,45,	25,88,	
1,46,	23,86,	2,6,	2,7,	
10,67,	10,67,	10,67,	10,67,	
10,67,	10,67,	10,67,	10,67,	
10,67,	10,67,	41,129,	22,82,	
26,89,	33,110,	28,95,	22,83,	
26,90,	41,130,	27,92,	33,111,	
34,112,	2,6,	46,140,	2,8,	
2,47,	22,84,	26,91,	28,96,	
2,9,	27,93,	27,94,	30,98,	
2,10,	34,113,	36,116,	2,5,	
35,114,	2,11,	30,99,	40,127,	
31,101,	39,125,	30,100,	31,102,	
35,115,	48,143,	31,103,	2,12,	
36,117,	31,104,	45,138,	39,126,	
45,139,	40,128,	2,13,	2,14,	
2,15,	2,16,	2,17,	2,18,	
2,19,	2,5,	2,20,	53,147,	
2,5,	2,21,	2,48,	2,23,	
2,24,	2,25,	37,118,	2,26,	
2,27,	2,28,	37,119,	42,131,	
2,29,	54,69,	37,120,	56,149,	
38,122,	2,5,	38,123,	42,132,	
37,121,	55,148,	2,30,	2,31,	
2,32,	2,33,	2,34,	2,35,	
55,71,	2,36,	3,49,	43,133,	
38,124,	2,37,	2,38,	2,39,	
2,40,	2,41,	43,134,	2,42,	
2,43,	2,44,	2,45,	44,136,	
2,46,	43,135,	44,137,	57,79,	
2,5,	2,5,	2,5,	32,105,	
3,50,	58,150,	32,106,	3,51,	
59,88,	58,81,	32,107,	3,9,	
61,152,	32,108,	66,155,	3,52,	
68,156,	69,157,	70,158,	32,109,	
52,67,	52,67,	52,67,	52,67,	
52,67,	52,67,	52,67,	52,67,	
52,67,	52,67,	3,12,	72,161,	
71,159,	73,162,	74,163,	60,151,	
71,160,	3,53,	3,54,	3,55,	
3,16,	3,56,	3,18,	3,19,	
75,164,	3,57,	60,93,	60,94,	
3,58,	3,22,	3,23,	3,24,	
3,59,	76,165,	3,26,	3,60,	
3,28,	77,166,	78,167,	3,61,	
79,168,	80,169,	81,170,	82,171,	
83,172,	84,173,	85,174,	87,177,	
4,49,	3,30,	3,31,	3,32,	
3,33,	3,34,	3,35,	89,180,	
3,36,	62,153,	86,175,	86,176,	
3,37,	3,38,	3,39,	3,40,	
3,41,	90,181,	3,42,	3,43,	
3,44,	3,45,	4,50,	3,46,	
88,178,	4,62,	91,182,	88,179,	
92,183,	4,9,	93,184,	62,154,	
94,185,	4,52,	62,154,	95,186,	
4,51,	96,187,	97,188,	98,189,	
99,191,	100,192,	62,154,	101,193,	
102,194,	103,195,	104,196,	105,197,	
4,12,	106,198,	98,190,	107,199,	
108,200,	109,201,	110,202,	4,53,	
4,54,	4,55,	4,16,	4,56,	
4,18,	4,19,	4,51,	4,57,	
111,203,	4,51,	4,58,	4,48,	
4,23,	4,24,	4,59,	112,204,	
4,26,	4,60,	4,28,	113,205,	
114,206,	4,61,	115,207,	116,208,	
117,210,	118,211,	4,51,	120,214,	
121,215,	116,209,	122,216,	4,30,	
4,31,	4,32,	4,33,	4,34,	
4,35,	122,217,	4,36,	122,218,	
123,219,	124,220,	4,37,	4,38,	
4,39,	4,40,	4,41,	9,66,	
4,42,	4,43,	4,44,	4,45,	
119,212,	4,46,	47,141,	9,66,	
9,0,	4,51,	4,51,	4,51,	
125,221,	127,228,	47,141,	47,142,	
128,229,	129,230,	130,231,	131,232,	
119,213,	50,63,	50,144,	132,233,	
133,234,	134,235,	135,237,	136,238,	
134,236,	137,239,	9,66,	140,243,	
9,66,	9,66,	147,156,	143,244,	
148,245,	47,141,	150,169,	47,141,	
47,141,	9,66,	151,183,	152,247,	
50,145,	143,171,	9,66,	50,146,	
47,141,	149,246,	156,248,	126,222,	
149,163,	47,141,	126,223,	50,146,	
138,240,	50,146,	50,146,	138,241,	
157,249,	51,144,	158,250,	126,224,	
126,225,	138,242,	159,251,	160,252,	
126,226,	161,253,	126,227,	162,254,	
163,255,	164,256,	165,257,	166,258,	
154,153,	50,146,	50,146,	50,146,	
167,259,	168,260,	169,261,	51,146,	
50,146,	50,146,	51,146,	50,146,	
50,146,	170,262,	171,263,	172,264,	
50,146,	173,265,	51,146,	50,146,	
51,146,	51,146,	154,154,	50,146,	
174,266,	154,154,	175,267,	176,268,	
50,146,	177,269,	178,270,	179,271,	
180,272,	154,154,	181,273,	182,274,	
183,275,	184,276,	185,277,	186,278,	
51,146,	51,146,	51,146,	187,279,	
188,280,	189,281,	190,282,	51,146,	
51,146,	191,283,	51,146,	51,146,	
192,284,	193,285,	194,286,	51,146,	
195,287,	196,288,	51,146,	50,146,	
50,146,	50,146,	51,146,	197,289,	
198,290,	199,291,	200,292,	51,146,	
201,293,	202,294,	203,295,	204,296,	
205,297,	206,298,	207,299,	208,300,	
209,301,	210,302,	212,303,	213,304,	
215,305,	216,306,	217,307,	218,308,	
219,309,	220,310,	221,311,	222,312,	
223,313,	224,314,	225,315,	226,316,	
227,317,	228,318,	229,319,	230,320,	
231,321,	232,322,	51,146,	51,146,	
51,146,	233,323,	234,324,	235,325,	
236,326,	237,327,	239,328,	240,329,	
241,330,	242,331,	243,332,	244,333,	
245,250,	246,334,	247,335,	248,336,	
249,337,	250,338,	251,339,	252,340,	
254,341,	255,342,	256,343,	257,344,	
258,345,	260,346,	261,347,	262,348,	
263,349,	264,350,	265,351,	267,352,	
268,353,	269,355,	271,356,	272,357,	
273,358,	276,359,	277,360,	278,361,	
280,362,	281,363,	282,364,	283,365,	
268,354,	284,366,	285,367,	286,368,	
287,369,	289,370,	290,371,	291,372,	
292,373,	293,374,	297,375,	298,376,	
299,377,	300,378,	301,379,	302,380,	
304,381,	305,382,	306,383,	307,384,	
308,385,	310,386,	311,387,	312,388,	
313,389,	314,390,	316,391,	317,392,	
318,393,	320,394,	321,395,	322,396,	
323,397,	324,398,	325,399,	326,400,	
327,401,	329,402,	330,403,	331,404,	
334,405,	335,406,	337,407,	338,408,	
339,409,	340,410,	341,411,	342,412,	
344,413,	345,414,	346,415,	347,416,	
349,417,	350,418,	351,419,	353,420,	
354,421,	355,422,	356,423,	357,424,	
358,425,	360,426,	361,427,	362,428,	
365,429,	367,430,	368,431,	369,432,	
370,433,	372,434,	373,435,	374,436,	
375,437,	377,438,	378,439,	379,441,	
380,442,	381,443,	382,444,	383,445,	
378,440,	384,446,	385,447,	386,448,	
387,449,	388,450,	389,451,	390,452,	
392,453,	393,454,	394,455,	396,456,	
398,457,	399,458,	400,459,	402,460,	
403,461,	405,462,	406,428,	409,463,	
410,464,	411,465,	412,466,	413,467,	
414,468,	415,469,	417,470,	418,471,	
419,472,	420,473,	421,474,	424,475,	
426,476,	427,477,	428,478,	430,479,	
435,480,	437,481,	439,482,	440,483,	
441,484,	442,485,	443,486,	445,487,	
446,488,	447,489,	448,490,	449,491,	
450,492,	453,493,	455,494,	457,495,	
458,496,	460,497,	461,498,	463,499,	
464,500,	465,501,	466,502,	467,503,	
468,504,	469,505,	470,506,	473,507,	
474,508,	475,509,	477,513,	480,514,	
482,515,	483,516,	484,517,	486,518,	
487,519,	488,520,	489,521,	475,510,	
493,522,	475,511,	475,512,	494,523,	
496,524,	497,525,	498,526,	499,527,	
501,528,	502,529,	507,530,	508,531,	
509,532,	510,533,	511,534,	512,535,	
513,536,	514,537,	515,538,	516,539,	
517,540,	518,541,	519,542,	520,543,	
522,544,	524,545,	528,546,	530,547,	
531,548,	532,549,	533,550,	534,551,	
535,552,	536,553,	539,554,	544,555,	
545,556,	547,557,	548,558,	549,559,	
550,560,	551,561,	552,562,	553,563,	
556,564,	557,565,	558,566,	559,567,	
560,568,	561,569,	562,570,	563,571,	
564,572,	567,573,	568,574,	569,575,	
570,576,	571,577,	573,578,	574,579,	
575,580,	577,581,	578,582,	579,583,	
581,584,	582,585,	0,0,	0,0,	
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
yycrank+16,	0,		yyvstop+30,
yycrank+27,	0,		yyvstop+32,
yycrank+19,	0,		yyvstop+34,
yycrank+39,	0,		yyvstop+36,
yycrank+70,	0,		yyvstop+38,
yycrank+42,	0,		yyvstop+40,
yycrank+7,	0,		yyvstop+42,
yycrank+40,	0,		yyvstop+44,
yycrank+71,	0,		yyvstop+46,
yycrank+69,	0,		yyvstop+48,
yycrank+69,	0,		yyvstop+50,
yycrank+10,	0,		yyvstop+52,
yycrank+47,	0,		yyvstop+54,
yycrank+56,	0,		yyvstop+56,
yycrank+138,	0,		yyvstop+58,
yycrank+26,	0,		yyvstop+60,
yycrank+47,	0,		yyvstop+62,
yycrank+63,	0,		yyvstop+64,
yycrank+61,	0,		yyvstop+66,
yycrank+97,	0,		yyvstop+68,
yycrank+109,	0,		yyvstop+70,
yycrank+64,	0,		yyvstop+72,
yycrank+65,	0,		yyvstop+74,
yycrank+33,	0,		yyvstop+76,
yycrank+102,	0,		yyvstop+78,
yycrank+122,	0,		yyvstop+80,
yycrank+120,	0,		yyvstop+82,
yycrank+64,	0,		yyvstop+84,
yycrank+45,	0,		yyvstop+86,
yycrank+-413,	0,		yyvstop+88,
yycrank+104,	yysvec+22,	yyvstop+90,
yycrank+0,	0,		yyvstop+92,
yycrank+420,	0,		yyvstop+95,
yycrank+459,	0,		yyvstop+98,
yycrank+208,	yysvec+51,	yyvstop+100,
yycrank+111,	yysvec+51,	yyvstop+102,
yycrank+119,	yysvec+51,	yyvstop+104,
yycrank+137,	yysvec+51,	yyvstop+106,
yycrank+125,	0,		yyvstop+108,
yycrank+157,	yysvec+51,	yyvstop+110,
yycrank+176,	yysvec+51,	yyvstop+112,
yycrank+165,	yysvec+51,	yyvstop+114,
yycrank+198,	yysvec+51,	yyvstop+116,
yycrank+183,	yysvec+51,	yyvstop+118,
yycrank+-303,	yysvec+47,	yyvstop+120,
yycrank+0,	yysvec+6,	yyvstop+122,
yycrank+0,	yysvec+8,	0,	
yycrank+0,	0,		yyvstop+124,
yycrank+211,	0,		0,	
yycrank+0,	yysvec+10,	yyvstop+126,
yycrank+168,	0,		0,	
yycrank+188,	0,		0,	
yycrank+189,	0,		0,	
yycrank+190,	0,		0,	
yycrank+188,	0,		0,	
yycrank+204,	0,		0,	
yycrank+199,	0,		0,	
yycrank+215,	0,		0,	
yycrank+204,	0,		0,	
yycrank+214,	0,		0,	
yycrank+218,	0,		0,	
yycrank+223,	0,		0,	
yycrank+229,	0,		0,	
yycrank+212,	0,		0,	
yycrank+209,	0,		0,	
yycrank+217,	0,		0,	
yycrank+223,	0,		0,	
yycrank+225,	0,		0,	
yycrank+237,	0,		0,	
yycrank+229,	0,		0,	
yycrank+249,	0,		0,	
yycrank+233,	0,		0,	
yycrank+250,	0,		0,	
yycrank+251,	0,		0,	
yycrank+254,	0,		0,	
yycrank+269,	0,		0,	
yycrank+270,	0,		0,	
yycrank+263,	0,		0,	
yycrank+276,	0,		0,	
yycrank+266,	0,		0,	
yycrank+238,	0,		0,	
yycrank+236,	0,		0,	
yycrank+248,	0,		0,	
yycrank+246,	0,		0,	
yycrank+232,	0,		0,	
yycrank+238,	0,		0,	
yycrank+236,	0,		0,	
yycrank+241,	0,		0,	
yycrank+256,	0,		0,	
yycrank+244,	0,		0,	
yycrank+246,	0,		0,	
yycrank+243,	0,		0,	
yycrank+239,	0,		0,	
yycrank+253,	0,		0,	
yycrank+260,	0,		0,	
yycrank+276,	0,		0,	
yycrank+272,	0,		0,	
yycrank+274,	0,		0,	
yycrank+275,	0,		0,	
yycrank+269,	0,		0,	
yycrank+266,	0,		0,	
yycrank+310,	0,		0,	
yycrank+271,	0,		0,	
yycrank+289,	0,		0,	
yycrank+288,	0,		0,	
yycrank+286,	0,		0,	
yycrank+291,	0,		0,	
yycrank+303,	0,		0,	
yycrank+362,	0,		0,	
yycrank+315,	0,		0,	
yycrank+323,	0,		0,	
yycrank+328,	0,		0,	
yycrank+329,	0,		0,	
yycrank+317,	0,		0,	
yycrank+328,	0,		0,	
yycrank+322,	0,		0,	
yycrank+322,	0,		0,	
yycrank+317,	0,		0,	
yycrank+323,	0,		0,	
yycrank+320,	0,		0,	
yycrank+365,	0,		0,	
yycrank+0,	0,		yyvstop+128,
yycrank+324,	0,		0,	
yycrank+0,	yysvec+47,	0,	
yycrank+0,	0,		yyvstop+130,
yycrank+363,	0,		0,	
yycrank+0,	0,		yyvstop+132,
yycrank+0,	yysvec+50,	yyvstop+134,
yycrank+0,	yysvec+51,	0,	
yycrank+358,	yysvec+51,	0,	
yycrank+379,	yysvec+51,	0,	
yycrank+389,	0,		0,	
yycrank+378,	yysvec+51,	0,	
yycrank+372,	yysvec+51,	0,	
yycrank+375,	yysvec+51,	0,	
yycrank+0,	0,		yyvstop+136,
yycrank+-474,	yysvec+47,	0,	
yycrank+0,	0,		yyvstop+139,
yycrank+393,	0,		0,	
yycrank+390,	0,		0,	
yycrank+392,	0,		0,	
yycrank+390,	0,		0,	
yycrank+393,	0,		0,	
yycrank+395,	0,		0,	
yycrank+392,	0,		0,	
yycrank+398,	0,		0,	
yycrank+410,	0,		0,	
yycrank+404,	0,		0,	
yycrank+406,	0,		0,	
yycrank+420,	0,		0,	
yycrank+405,	0,		0,	
yycrank+422,	0,		0,	
yycrank+428,	0,		0,	
yycrank+429,	0,		0,	
yycrank+416,	0,		0,	
yycrank+418,	0,		0,	
yycrank+439,	0,		0,	
yycrank+445,	0,		0,	
yycrank+416,	0,		0,	
yycrank+444,	0,		0,	
yycrank+438,	0,		0,	
yycrank+431,	0,		0,	
yycrank+448,	0,		0,	
yycrank+445,	0,		0,	
yycrank+442,	0,		0,	
yycrank+445,	0,		0,	
yycrank+448,	0,		0,	
yycrank+440,	0,		0,	
yycrank+454,	0,		0,	
yycrank+447,	0,		0,	
yycrank+452,	0,		0,	
yycrank+426,	0,		0,	
yycrank+433,	0,		0,	
yycrank+432,	0,		0,	
yycrank+429,	0,		0,	
yycrank+422,	0,		0,	
yycrank+422,	0,		0,	
yycrank+433,	0,		0,	
yycrank+431,	0,		0,	
yycrank+431,	0,		0,	
yycrank+437,	0,		0,	
yycrank+434,	0,		0,	
yycrank+434,	0,		0,	
yycrank+437,	0,		0,	
yycrank+443,	0,		0,	
yycrank+438,	0,		0,	
yycrank+439,	0,		0,	
yycrank+442,	0,		0,	
yycrank+442,	0,		0,	
yycrank+450,	0,		0,	
yycrank+457,	0,		0,	
yycrank+460,	0,		0,	
yycrank+445,	0,		0,	
yycrank+0,	0,		yyvstop+141,
yycrank+446,	0,		0,	
yycrank+449,	0,		0,	
yycrank+0,	0,		yyvstop+143,
yycrank+457,	0,		0,	
yycrank+464,	0,		0,	
yycrank+455,	0,		0,	
yycrank+469,	0,		0,	
yycrank+461,	0,		0,	
yycrank+454,	0,		0,	
yycrank+454,	0,		0,	
yycrank+463,	0,		0,	
yycrank+461,	0,		0,	
yycrank+464,	0,		0,	
yycrank+473,	0,		0,	
yycrank+459,	0,		0,	
yycrank+475,	0,		0,	
yycrank+476,	0,		0,	
yycrank+468,	0,		0,	
yycrank+480,	0,		0,	
yycrank+481,	0,		0,	
yycrank+481,	0,		0,	
yycrank+481,	0,		0,	
yycrank+487,	0,		0,	
yycrank+473,	0,		0,	
yycrank+483,	0,		0,	
yycrank+473,	0,		0,	
yycrank+0,	0,		yyvstop+145,
yycrank+489,	0,		0,	
yycrank+474,	0,		0,	
yycrank+487,	0,		0,	
yycrank+488,	0,		0,	
yycrank+478,	0,		0,	
yycrank+585,	0,		0,	
yycrank+518,	yysvec+51,	0,	
yycrank+520,	0,		0,	
yycrank+522,	yysvec+51,	0,	
yycrank+517,	0,		0,	
yycrank+533,	0,		0,	
yycrank+534,	0,		0,	
yycrank+537,	0,		0,	
yycrank+530,	0,		0,	
yycrank+0,	0,		yyvstop+147,
yycrank+538,	0,		0,	
yycrank+540,	0,		0,	
yycrank+523,	0,		0,	
yycrank+523,	0,		0,	
yycrank+539,	0,		0,	
yycrank+0,	0,		yyvstop+149,
yycrank+514,	0,		0,	
yycrank+541,	0,		0,	
yycrank+535,	0,		0,	
yycrank+525,	0,		yyvstop+151,
yycrank+548,	0,		0,	
yycrank+530,	0,		0,	
yycrank+0,	0,		yyvstop+153,
yycrank+535,	0,		0,	
yycrank+548,	0,		0,	
yycrank+550,	0,		0,	
yycrank+0,	0,		yyvstop+155,
yycrank+553,	0,		0,	
yycrank+540,	0,		0,	
yycrank+541,	0,		0,	
yycrank+0,	0,		yyvstop+157,
yycrank+0,	0,		yyvstop+159,
yycrank+539,	0,		0,	
yycrank+543,	0,		0,	
yycrank+543,	0,		0,	
yycrank+0,	0,		yyvstop+161,
yycrank+551,	0,		0,	
yycrank+515,	0,		0,	
yycrank+512,	0,		0,	
yycrank+526,	0,		0,	
yycrank+528,	0,		0,	
yycrank+515,	0,		0,	
yycrank+520,	0,		0,	
yycrank+531,	0,		0,	
yycrank+0,	0,		yyvstop+163,
yycrank+532,	0,		0,	
yycrank+519,	0,		0,	
yycrank+534,	0,		0,	
yycrank+539,	0,		0,	
yycrank+536,	0,		0,	
yycrank+0,	0,		yyvstop+165,
yycrank+0,	0,		yyvstop+167,
yycrank+0,	0,		yyvstop+169,
yycrank+541,	0,		0,	
yycrank+538,	0,		0,	
yycrank+539,	0,		0,	
yycrank+596,	0,		0,	
yycrank+540,	0,		0,	
yycrank+538,	0,		0,	
yycrank+0,	0,		yyvstop+171,
yycrank+543,	0,		0,	
yycrank+544,	0,		0,	
yycrank+549,	0,		0,	
yycrank+537,	0,		0,	
yycrank+542,	0,		0,	
yycrank+0,	0,		yyvstop+173,
yycrank+533,	0,		0,	
yycrank+536,	0,		0,	
yycrank+546,	0,		0,	
yycrank+541,	0,		0,	
yycrank+556,	0,		0,	
yycrank+0,	0,		yyvstop+175,
yycrank+550,	0,		0,	
yycrank+547,	0,		0,	
yycrank+557,	0,		0,	
yycrank+0,	0,		yyvstop+177,
yycrank+556,	0,		0,	
yycrank+557,	0,		0,	
yycrank+548,	0,		0,	
yycrank+544,	0,		0,	
yycrank+545,	0,		0,	
yycrank+546,	0,		0,	
yycrank+553,	0,		0,	
yycrank+560,	0,		0,	
yycrank+0,	0,		yyvstop+179,
yycrank+551,	0,		0,	
yycrank+558,	0,		0,	
yycrank+551,	0,		0,	
yycrank+0,	0,		yyvstop+181,
yycrank+0,	0,		yyvstop+183,
yycrank+603,	0,		0,	
yycrank+596,	yysvec+51,	0,	
yycrank+0,	0,		yyvstop+185,
yycrank+598,	0,		0,	
yycrank+602,	0,		0,	
yycrank+599,	0,		0,	
yycrank+605,	0,		0,	
yycrank+592,	0,		0,	
yycrank+589,	0,		0,	
yycrank+0,	0,		yyvstop+187,
yycrank+611,	0,		0,	
yycrank+593,	0,		0,	
yycrank+601,	0,		0,	
yycrank+597,	0,		0,	
yycrank+0,	0,		yyvstop+189,
yycrank+615,	0,		0,	
yycrank+610,	0,		0,	
yycrank+613,	0,		0,	
yycrank+0,	0,		yyvstop+191,
yycrank+610,	0,		0,	
yycrank+619,	0,		0,	
yycrank+601,	0,		0,	
yycrank+610,	0,		0,	
yycrank+610,	0,		0,	
yycrank+610,	0,		0,	
yycrank+0,	0,		yyvstop+193,
yycrank+610,	0,		0,	
yycrank+611,	0,		0,	
yycrank+621,	0,		0,	
yycrank+0,	0,		yyvstop+195,
yycrank+0,	0,		yyvstop+197,
yycrank+580,	0,		0,	
yycrank+0,	0,		yyvstop+199,
yycrank+592,	0,		0,	
yycrank+585,	0,		0,	
yycrank+585,	0,		0,	
yycrank+582,	0,		0,	
yycrank+0,	0,		yyvstop+201,
yycrank+597,	0,		0,	
yycrank+593,	0,		0,	
yycrank+599,	0,		0,	
yycrank+582,	0,		0,	
yycrank+0,	0,		yyvstop+203,
yycrank+601,	0,		0,	
yycrank+594,	0,		0,	
yycrank+595,	0,		0,	
yycrank+596,	0,		0,	
yycrank+602,	0,		0,	
yycrank+606,	0,		0,	
yycrank+591,	0,		0,	
yycrank+594,	0,		0,	
yycrank+609,	0,		0,	
yycrank+610,	0,		0,	
yycrank+615,	0,		0,	
yycrank+610,	0,		0,	
yycrank+600,	0,		0,	
yycrank+603,	0,		0,	
yycrank+0,	0,		yyvstop+205,
yycrank+615,	0,		0,	
yycrank+601,	0,		0,	
yycrank+616,	0,		0,	
yycrank+0,	0,		yyvstop+207,
yycrank+610,	0,		0,	
yycrank+0,	0,		yyvstop+209,
yycrank+603,	0,		0,	
yycrank+606,	0,		0,	
yycrank+621,	0,		0,	
yycrank+0,	0,		yyvstop+211,
yycrank+608,	0,		0,	
yycrank+616,	0,		0,	
yycrank+0,	0,		yyvstop+213,
yycrank+645,	0,		0,	
yycrank+656,	yysvec+51,	0,	
yycrank+0,	0,		yyvstop+215,
yycrank+0,	0,		yyvstop+217,
yycrank+649,	0,		0,	
yycrank+649,	0,		0,	
yycrank+656,	0,		0,	
yycrank+657,	0,		0,	
yycrank+658,	0,		0,	
yycrank+650,	0,		0,	
yycrank+668,	0,		0,	
yycrank+0,	0,		yyvstop+219,
yycrank+658,	0,		0,	
yycrank+666,	0,		0,	
yycrank+654,	0,		0,	
yycrank+666,	0,		0,	
yycrank+655,	0,		0,	
yycrank+0,	0,		yyvstop+221,
yycrank+0,	0,		yyvstop+223,
yycrank+644,	0,		0,	
yycrank+0,	0,		yyvstop+225,
yycrank+663,	0,		0,	
yycrank+659,	0,		0,	
yycrank+653,	0,		0,	
yycrank+0,	0,		yyvstop+227,
yycrank+643,	0,		0,	
yycrank+0,	0,		yyvstop+229,
yycrank+0,	0,		yyvstop+231,
yycrank+0,	0,		yyvstop+233,
yycrank+0,	0,		yyvstop+235,
yycrank+634,	0,		0,	
yycrank+0,	0,		yyvstop+237,
yycrank+644,	0,		0,	
yycrank+0,	0,		yyvstop+239,
yycrank+645,	0,		0,	
yycrank+642,	0,		0,	
yycrank+637,	0,		0,	
yycrank+648,	0,		0,	
yycrank+645,	0,		0,	
yycrank+0,	0,		yyvstop+241,
yycrank+634,	0,		0,	
yycrank+636,	0,		0,	
yycrank+654,	0,		0,	
yycrank+640,	0,		0,	
yycrank+647,	0,		0,	
yycrank+646,	0,		0,	
yycrank+0,	0,		yyvstop+243,
yycrank+0,	0,		yyvstop+245,
yycrank+645,	0,		0,	
yycrank+0,	0,		yyvstop+247,
yycrank+641,	0,		0,	
yycrank+0,	0,		yyvstop+249,
yycrank+650,	0,		0,	
yycrank+655,	0,		0,	
yycrank+0,	0,		yyvstop+251,
yycrank+660,	0,		0,	
yycrank+661,	0,		0,	
yycrank+0,	0,		yyvstop+253,
yycrank+694,	0,		0,	
yycrank+682,	0,		0,	
yycrank+697,	0,		0,	
yycrank+688,	0,		0,	
yycrank+689,	0,		0,	
yycrank+679,	0,		0,	
yycrank+689,	0,		0,	
yycrank+695,	0,		0,	
yycrank+0,	0,		yyvstop+255,
yycrank+0,	0,		yyvstop+257,
yycrank+700,	0,		0,	
yycrank+689,	0,		0,	
yycrank+706,	0,		0,	
yycrank+0,	0,		yyvstop+259,
yycrank+690,	0,		0,	
yycrank+0,	0,		yyvstop+261,
yycrank+0,	0,		yyvstop+263,
yycrank+674,	0,		0,	
yycrank+0,	0,		yyvstop+265,
yycrank+674,	0,		0,	
yycrank+674,	0,		0,	
yycrank+667,	0,		0,	
yycrank+0,	0,		yyvstop+267,
yycrank+668,	0,		0,	
yycrank+666,	0,		0,	
yycrank+680,	0,		0,	
yycrank+666,	0,		0,	
yycrank+0,	0,		yyvstop+269,
yycrank+0,	0,		yyvstop+271,
yycrank+0,	0,		yyvstop+273,
yycrank+673,	0,		0,	
yycrank+679,	0,		0,	
yycrank+0,	0,		yyvstop+275,
yycrank+685,	0,		0,	
yycrank+689,	0,		0,	
yycrank+690,	0,		0,	
yycrank+709,	0,		0,	
yycrank+0,	0,		yyvstop+277,
yycrank+721,	0,		0,	
yycrank+722,	0,		0,	
yycrank+0,	0,		yyvstop+279,
yycrank+0,	0,		yyvstop+281,
yycrank+0,	0,		yyvstop+283,
yycrank+0,	0,		yyvstop+285,
yycrank+729,	0,		0,	
yycrank+708,	0,		0,	
yycrank+717,	0,		0,	
yycrank+718,	0,		0,	
yycrank+732,	0,		0,	
yycrank+723,	0,		0,	
yycrank+705,	0,		0,	
yycrank+701,	0,		0,	
yycrank+686,	0,		0,	
yycrank+699,	0,		0,	
yycrank+690,	0,		0,	
yycrank+695,	0,		0,	
yycrank+705,	0,		0,	
yycrank+693,	0,		0,	
yycrank+0,	0,		yyvstop+287,
yycrank+694,	0,		0,	
yycrank+0,	0,		yyvstop+289,
yycrank+705,	0,		0,	
yycrank+0,	0,		yyvstop+291,
yycrank+0,	0,		yyvstop+293,
yycrank+0,	0,		yyvstop+295,
yycrank+741,	0,		0,	
yycrank+0,	0,		yyvstop+297,
yycrank+745,	0,		0,	
yycrank+747,	0,		0,	
yycrank+731,	0,		0,	
yycrank+736,	0,		0,	
yycrank+741,	0,		0,	
yycrank+751,	0,		0,	
yycrank+735,	0,		0,	
yycrank+0,	0,		yyvstop+299,
yycrank+0,	0,		yyvstop+301,
yycrank+702,	0,		0,	
yycrank+0,	0,		yyvstop+303,
yycrank+0,	0,		yyvstop+305,
yycrank+0,	0,		yyvstop+307,
yycrank+0,	0,		yyvstop+309,
yycrank+703,	0,		0,	
yycrank+704,	0,		0,	
yycrank+0,	0,		yyvstop+311,
yycrank+745,	0,		0,	
yycrank+746,	0,		0,	
yycrank+741,	0,		0,	
yycrank+741,	0,		0,	
yycrank+756,	0,		0,	
yycrank+759,	0,		0,	
yycrank+758,	0,		0,	
yycrank+0,	0,		yyvstop+313,
yycrank+0,	0,		yyvstop+315,
yycrank+727,	0,		0,	
yycrank+760,	0,		0,	
yycrank+754,	0,		0,	
yycrank+758,	0,		0,	
yycrank+748,	0,		0,	
yycrank+766,	0,		0,	
yycrank+765,	0,		0,	
yycrank+764,	0,		0,	
yycrank+736,	0,		0,	
yycrank+0,	0,		yyvstop+317,
yycrank+0,	0,		yyvstop+319,
yycrank+769,	0,		0,	
yycrank+769,	0,		0,	
yycrank+755,	0,		0,	
yycrank+757,	0,		0,	
yycrank+768,	0,		0,	
yycrank+0,	0,		yyvstop+321,
yycrank+763,	0,		0,	
yycrank+761,	0,		0,	
yycrank+761,	0,		0,	
yycrank+0,	0,		yyvstop+323,
yycrank+766,	0,		0,	
yycrank+764,	0,		0,	
yycrank+764,	0,		0,	
yycrank+0,	0,		yyvstop+325,
yycrank+770,	0,		0,	
yycrank+766,	0,		0,	
yycrank+0,	0,		yyvstop+327,
yycrank+0,	0,		yyvstop+329,
yycrank+0,	0,		yyvstop+331,
0,	0,	0};
struct yywork *yytop = yycrank+849;
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
