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
/*	SCCS Id: @(#)lev_lex.c	3.1	92/07/12	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#define LEV_LEX_C

#include "hack.h"
#include "lev_comp.h"
#include "sp_lev.h"

/* Most of these don't exist in flex, yywrap is macro and
 * yyunput is properly declared in flex.skel.
 */
#ifndef FLEX_SCANNER
int FDECL (yyback, (int *, int));
int NDECL (yylook);
int NDECL (yyinput);
int NDECL (yywrap);
int NDECL (yylex);
	/* Traditional lexes let yyunput() and yyoutput() default to int;
	 * newer ones may declare them as void since they don't return
	 * values.  For even more fun, the lex supplied as part of the
	 * newer unbundled compiler for SunOS 4.x adds the void declarations
	 * (under __STDC__ or _cplusplus ifdefs -- otherwise they remain
	 * int) while the bundled lex and the one with the older unbundled
	 * compiler do not.  To detect this, we need help from outside --
	 * sys/unix/Makefile.utl.
	 */
# if defined(NeXT) || defined(SVR4)
#  define VOIDYYPUT
# endif
# if !defined(VOIDYYPUT)
#  if defined(POSIX_TYPES) && !defined(BOS) && !defined(HISX)
#   define VOIDYYPUT
#  endif
# endif
# if !defined(VOIDYYPUT) && defined(WEIRD_LEX)
#  if defined(SUNOS4) && defined(__STDC__) && (WEIRD_LEX != 0) 
#   define VOIDYYPUT
#  endif
# endif
# ifdef VOIDYYPUT
void FDECL (yyunput, (int));
void FDECL (yyoutput, (int));
# else
int FDECL (yyunput, (int));
int FDECL (yyoutput, (int));
# endif
#endif	/* FLEX_SCANNER */

void FDECL (init_yyin, (FILE *));
void FDECL (init_yyout, (FILE *));

#ifdef MICRO
#undef exit
extern void FDECL(exit, (int));
#endif

/* this doesn't always get put in lev_comp.h
 * (esp. when using older versions of bison)
 */

extern YYSTYPE yylval;

int line_number = 1, colon_line_number = 1;

/* This is *** UGLY *** but I can't think a better way to do it
 * I really need a huge buffer to scan maps...
 */

#undef YYLMAX
#define YYLMAX	2048

/*
 *	This is a hack required by Michael Hamel to get things
 *	working on the Mac.
 */
#if defined(applec) && !defined(FLEX_SCANNER)
#undef input
#undef unput
#define unput(c) { yytchar = (c); if (yytchar == 10) yylineno--; *yysptr++ = yytchar; }
# ifndef YYNEWLINE
# define YYNEWLINE 10
# endif

char
input() {	/* Under MPW \n is chr(13)! Compensate for this. */

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
#endif	/* applec && !FLEX_SCANNER */

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
#ifdef FLEX_SCANNER
		  /*
		   * There is a bug in Flex 2.3 patch level < 6
		   * (absent in previous versions)
		   * that results in the following behaviour :
		   * Once you enter an yymore(), you never exit from it.
		   * This should do the trick!
		   */
		  extern int yy_more_len;

		  yy_more_len = 0;
#endif
		  BEGIN(INITIAL);
		  yylval.map = (char *) alloc(yyleng-5);
		  strncpy(yylval.map, yytext,yyleng-6);
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
	return MONSTER_ID;
break;
case 15:
	return TRAP_ID;
break;
case 16:
	return DOOR_ID;
break;
case 17:
return DRAWBRIDGE_ID;
break;
case 18:
return MAZEWALK_ID;
break;
case 19:
	return WALLIFY_ID;
break;
case 20:
	return REGION_ID;
break;
case 21:
return RANDOM_OBJECTS_ID;
break;
case 22:
return RANDOM_MONSTERS_ID;
break;
case 23:
return RANDOM_PLACES_ID;
break;
case 24:
	return ALTAR_ID;
break;
case 25:
	return LADDER_ID;
break;
case 26:
	return STAIR_ID;
break;
case 27:
	return PORTAL_ID;
break;
case 28:
return TELEPRT_ID;
break;
case 29:
	return BRANCH_ID;
break;
case 30:
return FOUNTAIN_ID;
break;
case 31:
	return SINK_ID;
break;
case 32:
	return POOL_ID;
break;
case 33:
return NON_DIGGABLE_ID;
break;
case 34:
	return ROOM_ID;
break;
case 35:
	return SUBROOM_ID;
break;
case 36:
return RAND_CORRIDOR_ID;
break;
case 37:
return CORRIDOR_ID;
break;
case 38:
	return GOLD_ID;
break;
case 39:
return ENGRAVING_ID;
break;
case 40:
	return NAME_ID;
break;
case 41:
	return CHANCE_ID;
break;
case 42:
return LEV;
break;
case 43:
	{ yylval.i=D_ISOPEN; return DOOR_STATE; }
break;
case 44:
	{ yylval.i=D_CLOSED; return DOOR_STATE; }
break;
case 45:
	{ yylval.i=D_LOCKED; return DOOR_STATE; }
break;
case 46:
	{ yylval.i=D_NODOOR; return DOOR_STATE; }
break;
case 47:
	{ yylval.i=D_BROKEN; return DOOR_STATE; }
break;
case 48:
	{ yylval.i=W_NORTH; return DIRECTION; }
break;
case 49:
	{ yylval.i=W_EAST; return DIRECTION; }
break;
case 50:
	{ yylval.i=W_SOUTH; return DIRECTION; }
break;
case 51:
	{ yylval.i=W_WEST; return DIRECTION; }
break;
case 52:
	{ yylval.i = -1; return RANDOM_TYPE; }
break;
case 53:
	{ yylval.i = -2; return NONE; }
break;
case 54:
	return O_REGISTER;
break;
case 55:
	return M_REGISTER;
break;
case 56:
	return P_REGISTER;
break;
case 57:
	return A_REGISTER;
break;
case 58:
	{ yylval.i=1; return LEFT_OR_RIGHT; }
break;
case 59:
{ yylval.i=2; return LEFT_OR_RIGHT; }
break;
case 60:
	{ yylval.i=3; return CENTER; }
break;
case 61:
{ yylval.i=4; return LEFT_OR_RIGHT; }
break;
case 62:
	{ yylval.i=5; return LEFT_OR_RIGHT; }
break;
case 63:
	{ yylval.i=1; return TOP_OR_BOT; }
break;
case 64:
	{ yylval.i=5; return TOP_OR_BOT; }
break;
case 65:
	{ yylval.i=1; return LIGHT_STATE; }
break;
case 66:
	{ yylval.i=0; return LIGHT_STATE; }
break;
case 67:
	{ yylval.i=0; return FILLING; }
break;
case 68:
{ yylval.i=1; return FILLING; }
break;
case 69:
	{ yylval.i= AM_NONE; return ALIGNMENT; }
break;
case 70:
	{ yylval.i= AM_LAWFUL; return ALIGNMENT; }
break;
case 71:
	{ yylval.i= AM_NEUTRAL; return ALIGNMENT; }
break;
case 72:
	{ yylval.i= AM_CHAOTIC; return ALIGNMENT; }
break;
case 73:
{ yylval.i=1; return MON_ATTITUDE; }
break;
case 74:
	{ yylval.i=0; return MON_ATTITUDE; }
break;
case 75:
	{ yylval.i=1; return MON_ALERTNESS; }
break;
case 76:
	{ yylval.i=0; return MON_ALERTNESS; }
break;
case 77:
{ yylval.i= M_AP_FURNITURE; return MON_APPEARANCE; }
break;
case 78:
{ yylval.i= M_AP_MONSTER;   return MON_APPEARANCE; }
break;
case 79:
{ yylval.i= M_AP_OBJECT;    return MON_APPEARANCE; }
break;
case 80:
	{ yylval.i=2; return ALTAR_TYPE; }
break;
case 81:
	{ yylval.i=1; return ALTAR_TYPE; }
break;
case 82:
	{ yylval.i=0; return ALTAR_TYPE; }
break;
case 83:
	{ yylval.i=1; return UP_OR_DOWN; }
break;
case 84:
	{ yylval.i=0; return UP_OR_DOWN; }
break;
case 85:
	{ yylval.i=0; return BOOLEAN; }
break;
case 86:
	{ yylval.i=1; return BOOLEAN; }
break;
case 87:
	{ yylval.i=DUST; return ENGRAVING_TYPE; }
break;
case 88:
	{ yylval.i=ENGRAVE; return ENGRAVING_TYPE; }
break;
case 89:
	{ yylval.i=BURN; return ENGRAVING_TYPE; }
break;
case 90:
	{ yylval.i=MARK; return ENGRAVING_TYPE; }
break;
case 91:
	{ yylval.i=1; return CURSE_TYPE; }
break;
case 92:
{ yylval.i=2; return CURSE_TYPE; }
break;
case 93:
	{ yylval.i=3; return CURSE_TYPE; }
break;
case 94:
{ yylval.i=NOTELEPORT; return FLAG_TYPE; }
break;
case 95:
{ yylval.i=HARDFLOOR; return FLAG_TYPE; }
break;
case 96:
	{ yylval.i=NOMMAP; return FLAG_TYPE; }
break;
case 97:
{ yylval.i=SHORTSIGHTED; return FLAG_TYPE; }
break;
case 98:
{ yylval.i=atoi(yytext); return INTEGER; }
break;
case 99:
{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
break;
case 100:
	{ line_number++; }
break;
case 101:
	;
break;
case 102:
	{ yylval.i = yytext[1]; return CHAR; }
break;
case 103:
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
#ifdef FLEX_SCANNER
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

int yyvstop[] = {
0,

103,
0,

101,
103,
0,

100,
0,

103,
0,

103,
0,

103,
0,

98,
103,
0,

4,
103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

2,
100,
0,

101,
103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

103,
0,

101,
0,

99,
0,

98,
0,

83,
0,

3,
0,

2,
0,

101,
0,

2,
3,
0,

102,
0,

70,
0,

65,
0,

63,
0,

16,
0,

38,
0,

6,
0,

40,
0,

32,
0,

34,
0,

31,
0,

15,
0,

89,
0,

84,
0,

87,
0,

49,
0,

58,
0,

90,
0,

53,
0,

43,
0,

86,
0,

51,
0,

12,
0,

24,
0,

10,
0,

8,
0,

7,
0,

26,
0,

57,
0,

82,
0,

76,
0,

72,
0,

85,
0,

48,
0,

56,
0,

62,
0,

50,
0,

66,
0,

29,
0,

41,
0,

25,
0,

13,
0,

27,
0,

20,
0,

75,
0,

64,
0,

47,
0,

60,
0,

44,
0,

93,
0,

67,
0,

45,
0,

46,
0,

96,
0,

54,
0,

52,
0,

81,
0,

1,
0,

5,
0,

14,
0,

35,
0,

19,
0,

91,
0,

88,
0,

74,
0,

55,
0,

71,
0,

69,
0,

80,
0,

37,
0,

30,
0,

11,
0,

9,
0,

18,
0,

79,
0,

73,
0,

92,
0,

68,
0,

39,
0,

59,
0,

95,
0,

42,
0,

77,
0,

78,
0,

17,
0,

61,
0,

94,
0,

33,
0,

97,
0,

23,
0,

21,
0,

22,
0,

28,
0,

36,
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
1,24,	1,25,	13,68,	1,26,	
1,27,	1,28,	14,69,	15,70,	
1,29,	16,72,	17,74,	20,79,	
16,73,	18,75,	15,71,	25,88,	
18,76,	19,77,	1,30,	1,31,	
1,32,	1,33,	1,34,	1,35,	
21,80,	1,36,	48,142,	19,78,	
21,81,	1,37,	1,38,	1,39,	
1,40,	1,41,	23,85,	1,42,	
1,43,	1,44,	1,45,	2,47,	
1,46,	33,109,	41,128,	2,9,	
45,137,	46,139,	45,138,	33,110,	
23,86,	41,129,	2,5,	10,67,	
10,67,	10,67,	10,67,	10,67,	
10,67,	10,67,	10,67,	10,67,	
10,67,	53,146,	2,12,	28,95,	
22,82,	54,69,	56,147,	57,79,	
22,83,	2,13,	2,14,	2,15,	
2,16,	2,17,	2,18,	2,19,	
28,96,	2,20,	22,84,	2,5,	
2,21,	2,48,	2,23,	2,24,	
2,25,	26,89,	2,26,	2,27,	
2,28,	26,90,	35,113,	2,29,	
44,135,	59,88,	61,150,	44,136,	
2,5,	34,111,	35,114,	26,91,	
30,98,	2,30,	2,31,	2,32,	
2,33,	2,34,	2,35,	30,99,	
2,36,	3,49,	34,112,	30,100,	
2,37,	2,38,	2,39,	2,40,	
2,41,	66,153,	2,42,	2,43,	
2,44,	2,45,	39,124,	2,46,	
27,92,	36,115,	55,70,	2,5,	
2,5,	2,5,	68,154,	3,50,	
39,125,	55,71,	3,51,	27,93,	
27,94,	31,101,	3,9,	36,116,	
31,102,	37,117,	3,52,	31,103,	
32,105,	37,118,	31,104,	32,106,	
40,126,	37,119,	42,130,	32,107,	
58,148,	69,155,	70,156,	37,120,	
58,81,	3,12,	42,131,	71,157,	
32,108,	72,158,	40,127,	73,159,	
3,53,	3,54,	3,55,	3,16,	
3,56,	3,18,	3,19,	38,121,	
3,57,	38,122,	74,160,	3,58,	
3,22,	3,23,	3,24,	3,59,	
60,149,	3,26,	3,60,	3,28,	
75,161,	76,162,	3,61,	38,123,	
43,132,	77,163,	78,164,	60,93,	
60,94,	79,165,	80,166,	43,133,	
3,30,	3,31,	3,32,	3,33,	
3,34,	3,35,	43,134,	3,36,	
81,167,	82,168,	83,169,	3,37,	
3,38,	3,39,	3,40,	3,41,	
4,5,	3,42,	3,43,	3,44,	
3,45,	84,170,	3,46,	62,151,	
4,6,	4,49,	52,67,	52,67,	
52,67,	52,67,	52,67,	52,67,	
52,67,	52,67,	52,67,	52,67,	
85,171,	86,172,	86,173,	87,174,	
88,175,	89,177,	90,178,	88,176,	
91,179,	62,152,	92,180,	4,50,	
62,152,	4,8,	4,62,	93,181,	
94,182,	95,183,	4,9,	96,184,	
62,152,	97,185,	4,52,	98,186,	
99,188,	4,51,	100,189,	4,11,	
101,190,	102,191,	103,192,	104,193,	
105,194,	106,195,	98,187,	107,196,	
108,197,	4,12,	109,198,	110,199,	
111,200,	112,201,	113,202,	114,203,	
4,53,	4,54,	4,55,	4,16,	
4,56,	4,18,	4,19,	116,206,	
4,57,	117,207,	4,51,	4,58,	
4,48,	4,23,	4,24,	4,59,	
119,210,	4,26,	4,60,	4,28,	
115,204,	120,211,	4,61,	122,215,	
118,208,	123,216,	115,205,	4,51,	
124,217,	126,224,	127,225,	128,226,	
4,30,	4,31,	4,32,	4,33,	
4,34,	4,35,	129,227,	4,36,	
118,209,	130,228,	131,229,	4,37,	
4,38,	4,39,	4,40,	4,41,	
9,66,	4,42,	4,43,	4,44,	
4,45,	132,230,	4,46,	47,140,	
9,66,	9,0,	4,51,	4,51,	
4,51,	134,233,	135,234,	47,140,	
47,141,	121,212,	133,231,	136,235,	
139,239,	133,232,	50,63,	50,143,	
121,213,	142,240,	121,214,	137,236,	
146,154,	147,241,	137,237,	9,66,	
147,160,	9,66,	9,66,	142,168,	
137,238,	148,166,	47,140,	149,180,	
47,140,	47,140,	9,66,	150,242,	
154,243,	50,144,	155,244,	9,66,	
50,145,	47,140,	152,151,	156,245,	
125,218,	157,246,	47,140,	125,219,	
50,145,	158,247,	50,145,	50,145,	
159,248,	160,249,	51,143,	161,250,	
125,220,	125,221,	162,251,	163,252,	
164,253,	125,222,	165,254,	125,223,	
152,152,	166,255,	167,256,	152,152,	
168,257,	169,258,	50,145,	50,145,	
50,145,	170,259,	171,260,	152,152,	
51,145,	172,261,	50,145,	51,145,	
50,145,	50,145,	173,262,	174,263,	
175,264,	50,145,	176,265,	51,145,	
50,145,	51,145,	51,145,	177,266,	
50,145,	178,267,	179,268,	180,269,	
181,270,	50,145,	182,271,	183,272,	
184,273,	185,274,	186,275,	187,276,	
188,277,	189,278,	190,279,	191,280,	
192,281,	51,145,	51,145,	51,145,	
193,282,	194,283,	195,284,	196,285,	
197,286,	51,145,	198,287,	51,145,	
51,145,	199,288,	200,289,	201,290,	
51,145,	202,291,	203,292,	51,145,	
50,145,	50,145,	50,145,	51,145,	
204,293,	205,294,	206,295,	208,296,	
51,145,	209,297,	211,298,	212,299,	
213,300,	214,301,	215,302,	216,303,	
217,304,	218,305,	219,306,	220,307,	
221,308,	222,309,	223,310,	224,311,	
225,312,	226,313,	227,314,	228,315,	
229,316,	230,317,	231,318,	232,319,	
233,320,	235,321,	236,322,	237,323,	
238,324,	239,325,	240,326,	51,145,	
51,145,	51,145,	241,327,	242,328,	
243,329,	244,330,	245,331,	246,332,	
248,333,	249,334,	250,335,	251,336,	
252,337,	254,338,	255,339,	256,340,	
257,341,	258,342,	259,343,	261,344,	
262,345,	263,346,	265,347,	266,348,	
267,349,	270,350,	271,351,	272,352,	
274,353,	275,354,	276,355,	277,356,	
278,357,	279,358,	280,359,	281,360,	
283,361,	284,362,	285,363,	286,364,	
290,365,	291,366,	292,367,	293,368,	
294,369,	295,370,	297,371,	298,372,	
299,373,	300,374,	301,375,	303,376,	
304,377,	305,378,	306,379,	307,380,	
309,381,	310,382,	311,383,	313,384,	
314,385,	315,386,	316,387,	317,388,	
318,389,	319,390,	320,391,	322,392,	
323,393,	324,394,	327,395,	328,396,	
330,397,	331,398,	332,399,	333,400,	
334,401,	336,402,	337,403,	338,404,	
339,405,	341,406,	342,407,	343,408,	
345,409,	346,410,	347,411,	348,412,	
349,413,	351,414,	352,415,	353,416,	
356,417,	358,418,	359,419,	360,420,	
361,421,	363,422,	364,423,	365,424,	
367,425,	368,426,	369,428,	370,429,	
371,430,	372,431,	373,432,	368,427,	
374,433,	375,434,	376,435,	377,436,	
378,437,	379,438,	380,439,	382,440,	
383,441,	384,442,	386,443,	388,444,	
389,445,	390,446,	392,447,	393,448,	
395,449,	396,416,	399,450,	400,451,	
401,452,	402,453,	403,454,	404,455,	
406,456,	407,457,	408,458,	409,459,	
412,460,	414,461,	415,462,	416,463,	
418,464,	424,465,	426,466,	427,467,	
428,468,	429,469,	430,470,	432,471,	
433,472,	434,473,	435,474,	436,475,	
437,476,	440,477,	442,478,	444,479,	
445,480,	447,481,	448,482,	450,483,	
451,484,	452,485,	453,486,	454,487,	
455,488,	456,489,	459,490,	460,491,	
462,495,	466,496,	467,497,	468,498,	
470,499,	471,500,	472,501,	473,502,	
477,503,	460,492,	478,504,	460,493,	
460,494,	480,505,	481,506,	482,507,	
484,508,	485,509,	490,510,	491,511,	
492,512,	493,513,	494,514,	495,515,	
496,516,	497,517,	498,518,	499,519,	
500,520,	501,521,	503,522,	505,523,	
508,524,	510,525,	511,526,	512,527,	
513,528,	514,529,	515,530,	517,531,	
522,532,	523,533,	525,534,	526,535,	
527,536,	528,537,	529,538,	530,539,	
533,540,	534,541,	535,542,	536,543,	
537,544,	538,545,	539,546,	540,547,	
542,548,	543,549,	544,550,	545,551,	
546,552,	548,553,	549,554,	550,555,	
552,556,	553,557,	554,558,	556,559,	
557,560,	0,0,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-84,	yysvec+1,	0,	
yycrank+-179,	yysvec+1,	0,	
yycrank+-291,	0,		0,	
yycrank+0,	0,		yyvstop+1,
yycrank+5,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+-7,	0,		yyvstop+8,
yycrank+-403,	0,		yyvstop+10,
yycrank+83,	0,		yyvstop+12,
yycrank+0,	yysvec+10,	yyvstop+14,
yycrank+0,	0,		yyvstop+17,
yycrank+6,	0,		yyvstop+20,
yycrank+4,	0,		yyvstop+22,
yycrank+15,	0,		yyvstop+24,
yycrank+10,	0,		yyvstop+26,
yycrank+12,	0,		yyvstop+28,
yycrank+17,	0,		yyvstop+30,
yycrank+28,	0,		yyvstop+32,
yycrank+13,	0,		yyvstop+34,
yycrank+39,	0,		yyvstop+36,
yycrank+79,	0,		yyvstop+38,
yycrank+49,	0,		yyvstop+40,
yycrank+7,	0,		yyvstop+42,
yycrank+16,	0,		yyvstop+44,
yycrank+100,	0,		yyvstop+46,
yycrank+131,	0,		yyvstop+48,
yycrank+74,	0,		yyvstop+50,
yycrank+10,	0,		yyvstop+52,
yycrank+72,	0,		yyvstop+54,
yycrank+109,	0,		yyvstop+56,
yycrank+123,	0,		yyvstop+58,
yycrank+10,	0,		yyvstop+60,
yycrank+80,	0,		yyvstop+62,
yycrank+73,	0,		yyvstop+64,
yycrank+108,	0,		yyvstop+66,
yycrank+124,	0,		yyvstop+68,
yycrank+156,	0,		yyvstop+70,
yycrank+101,	0,		yyvstop+72,
yycrank+130,	0,		yyvstop+74,
yycrank+21,	0,		yyvstop+76,
yycrank+133,	0,		yyvstop+78,
yycrank+171,	0,		yyvstop+80,
yycrank+61,	0,		yyvstop+82,
yycrank+14,	0,		yyvstop+84,
yycrank+24,	0,		yyvstop+86,
yycrank+-410,	0,		yyvstop+88,
yycrank+41,	yysvec+22,	yyvstop+90,
yycrank+0,	0,		yyvstop+92,
yycrank+417,	0,		yyvstop+95,
yycrank+456,	0,		yyvstop+98,
yycrank+254,	yysvec+51,	yyvstop+100,
yycrank+65,	yysvec+51,	yyvstop+102,
yycrank+63,	yysvec+51,	yyvstop+104,
yycrank+134,	yysvec+51,	yyvstop+106,
yycrank+68,	0,		yyvstop+108,
yycrank+69,	yysvec+51,	yyvstop+110,
yycrank+167,	yysvec+51,	yyvstop+112,
yycrank+94,	yysvec+51,	yyvstop+114,
yycrank+187,	yysvec+51,	yyvstop+116,
yycrank+109,	yysvec+51,	yyvstop+118,
yycrank+-289,	yysvec+47,	yyvstop+120,
yycrank+0,	yysvec+6,	yyvstop+122,
yycrank+0,	yysvec+8,	0,	
yycrank+0,	0,		yyvstop+124,
yycrank+158,	0,		0,	
yycrank+0,	yysvec+10,	yyvstop+126,
yycrank+126,	0,		0,	
yycrank+168,	0,		0,	
yycrank+169,	0,		0,	
yycrank+157,	0,		0,	
yycrank+162,	0,		0,	
yycrank+178,	0,		0,	
yycrank+183,	0,		0,	
yycrank+199,	0,		0,	
yycrank+180,	0,		0,	
yycrank+190,	0,		0,	
yycrank+194,	0,		0,	
yycrank+200,	0,		0,	
yycrank+206,	0,		0,	
yycrank+198,	0,		0,	
yycrank+195,	0,		0,	
yycrank+203,	0,		0,	
yycrank+219,	0,		0,	
yycrank+235,	0,		0,	
yycrank+236,	0,		0,	
yycrank+241,	0,		0,	
yycrank+237,	0,		0,	
yycrank+239,	0,		0,	
yycrank+247,	0,		0,	
yycrank+241,	0,		0,	
yycrank+244,	0,		0,	
yycrank+262,	0,		0,	
yycrank+262,	0,		0,	
yycrank+253,	0,		0,	
yycrank+266,	0,		0,	
yycrank+257,	0,		0,	
yycrank+230,	0,		0,	
yycrank+228,	0,		0,	
yycrank+241,	0,		0,	
yycrank+239,	0,		0,	
yycrank+225,	0,		0,	
yycrank+231,	0,		0,	
yycrank+229,	0,		0,	
yycrank+234,	0,		0,	
yycrank+248,	0,		0,	
yycrank+236,	0,		0,	
yycrank+234,	0,		0,	
yycrank+231,	0,		0,	
yycrank+236,	0,		0,	
yycrank+237,	0,		0,	
yycrank+250,	0,		0,	
yycrank+246,	0,		0,	
yycrank+247,	0,		0,	
yycrank+268,	0,		0,	
yycrank+248,	0,		0,	
yycrank+246,	0,		0,	
yycrank+278,	0,		0,	
yycrank+256,	0,		0,	
yycrank+278,	0,		0,	
yycrank+319,	0,		0,	
yycrank+265,	0,		0,	
yycrank+271,	0,		0,	
yycrank+267,	0,		0,	
yycrank+359,	0,		0,	
yycrank+279,	0,		0,	
yycrank+285,	0,		0,	
yycrank+290,	0,		0,	
yycrank+297,	0,		0,	
yycrank+287,	0,		0,	
yycrank+295,	0,		0,	
yycrank+299,	0,		0,	
yycrank+311,	0,		0,	
yycrank+300,	0,		0,	
yycrank+306,	0,		0,	
yycrank+306,	0,		0,	
yycrank+332,	0,		0,	
yycrank+0,	0,		yyvstop+128,
yycrank+309,	0,		0,	
yycrank+0,	yysvec+47,	0,	
yycrank+0,	0,		yyvstop+130,
yycrank+349,	0,		0,	
yycrank+0,	0,		yyvstop+132,
yycrank+0,	yysvec+50,	yyvstop+134,
yycrank+0,	yysvec+51,	0,	
yycrank+348,	yysvec+51,	0,	
yycrank+365,	0,		0,	
yycrank+373,	yysvec+51,	0,	
yycrank+365,	yysvec+51,	0,	
yycrank+371,	yysvec+51,	0,	
yycrank+0,	0,		yyvstop+136,
yycrank+-444,	yysvec+47,	0,	
yycrank+0,	0,		yyvstop+139,
yycrank+383,	0,		0,	
yycrank+372,	0,		0,	
yycrank+377,	0,		0,	
yycrank+375,	0,		0,	
yycrank+379,	0,		0,	
yycrank+377,	0,		0,	
yycrank+383,	0,		0,	
yycrank+396,	0,		0,	
yycrank+392,	0,		0,	
yycrank+394,	0,		0,	
yycrank+404,	0,		0,	
yycrank+390,	0,		0,	
yycrank+409,	0,		0,	
yycrank+409,	0,		0,	
yycrank+411,	0,		0,	
yycrank+398,	0,		0,	
yycrank+402,	0,		0,	
yycrank+417,	0,		0,	
yycrank+424,	0,		0,	
yycrank+399,	0,		0,	
yycrank+426,	0,		0,	
yycrank+420,	0,		0,	
yycrank+414,	0,		0,	
yycrank+435,	0,		0,	
yycrank+432,	0,		0,	
yycrank+429,	0,		0,	
yycrank+432,	0,		0,	
yycrank+435,	0,		0,	
yycrank+428,	0,		0,	
yycrank+442,	0,		0,	
yycrank+432,	0,		0,	
yycrank+437,	0,		0,	
yycrank+411,	0,		0,	
yycrank+418,	0,		0,	
yycrank+415,	0,		0,	
yycrank+410,	0,		0,	
yycrank+403,	0,		0,	
yycrank+403,	0,		0,	
yycrank+413,	0,		0,	
yycrank+414,	0,		0,	
yycrank+409,	0,		0,	
yycrank+415,	0,		0,	
yycrank+412,	0,		0,	
yycrank+413,	0,		0,	
yycrank+420,	0,		0,	
yycrank+417,	0,		0,	
yycrank+418,	0,		0,	
yycrank+421,	0,		0,	
yycrank+422,	0,		0,	
yycrank+430,	0,		0,	
yycrank+442,	0,		0,	
yycrank+445,	0,		0,	
yycrank+430,	0,		0,	
yycrank+0,	0,		yyvstop+141,
yycrank+431,	0,		0,	
yycrank+435,	0,		0,	
yycrank+0,	0,		yyvstop+143,
yycrank+443,	0,		0,	
yycrank+450,	0,		0,	
yycrank+441,	0,		0,	
yycrank+455,	0,		0,	
yycrank+447,	0,		0,	
yycrank+440,	0,		0,	
yycrank+440,	0,		0,	
yycrank+449,	0,		0,	
yycrank+447,	0,		0,	
yycrank+450,	0,		0,	
yycrank+459,	0,		0,	
yycrank+445,	0,		0,	
yycrank+461,	0,		0,	
yycrank+462,	0,		0,	
yycrank+454,	0,		0,	
yycrank+466,	0,		0,	
yycrank+467,	0,		0,	
yycrank+467,	0,		0,	
yycrank+464,	0,		0,	
yycrank+470,	0,		0,	
yycrank+456,	0,		0,	
yycrank+466,	0,		0,	
yycrank+456,	0,		0,	
yycrank+0,	0,		yyvstop+145,
yycrank+472,	0,		0,	
yycrank+457,	0,		0,	
yycrank+470,	0,		0,	
yycrank+471,	0,		0,	
yycrank+461,	0,		0,	
yycrank+568,	0,		0,	
yycrank+505,	0,		0,	
yycrank+507,	yysvec+51,	0,	
yycrank+502,	0,		0,	
yycrank+518,	0,		0,	
yycrank+519,	0,		0,	
yycrank+514,	0,		0,	
yycrank+0,	0,		yyvstop+147,
yycrank+522,	0,		0,	
yycrank+524,	0,		0,	
yycrank+507,	0,		0,	
yycrank+507,	0,		0,	
yycrank+523,	0,		0,	
yycrank+0,	0,		yyvstop+149,
yycrank+498,	0,		0,	
yycrank+525,	0,		0,	
yycrank+519,	0,		0,	
yycrank+509,	0,		yyvstop+151,
yycrank+532,	0,		0,	
yycrank+514,	0,		0,	
yycrank+0,	0,		yyvstop+153,
yycrank+519,	0,		0,	
yycrank+532,	0,		0,	
yycrank+534,	0,		0,	
yycrank+0,	0,		yyvstop+155,
yycrank+537,	0,		0,	
yycrank+524,	0,		0,	
yycrank+525,	0,		0,	
yycrank+0,	0,		yyvstop+157,
yycrank+0,	0,		yyvstop+159,
yycrank+523,	0,		0,	
yycrank+527,	0,		0,	
yycrank+527,	0,		0,	
yycrank+0,	0,		yyvstop+161,
yycrank+535,	0,		0,	
yycrank+499,	0,		0,	
yycrank+496,	0,		0,	
yycrank+510,	0,		0,	
yycrank+511,	0,		0,	
yycrank+498,	0,		0,	
yycrank+503,	0,		0,	
yycrank+514,	0,		0,	
yycrank+0,	0,		yyvstop+163,
yycrank+515,	0,		0,	
yycrank+502,	0,		0,	
yycrank+517,	0,		0,	
yycrank+518,	0,		0,	
yycrank+0,	0,		yyvstop+165,
yycrank+0,	0,		yyvstop+167,
yycrank+0,	0,		yyvstop+169,
yycrank+523,	0,		0,	
yycrank+520,	0,		0,	
yycrank+521,	0,		0,	
yycrank+578,	0,		0,	
yycrank+522,	0,		0,	
yycrank+520,	0,		0,	
yycrank+0,	0,		yyvstop+171,
yycrank+525,	0,		0,	
yycrank+526,	0,		0,	
yycrank+531,	0,		0,	
yycrank+519,	0,		0,	
yycrank+524,	0,		0,	
yycrank+0,	0,		yyvstop+173,
yycrank+515,	0,		0,	
yycrank+518,	0,		0,	
yycrank+528,	0,		0,	
yycrank+523,	0,		0,	
yycrank+538,	0,		0,	
yycrank+0,	0,		yyvstop+175,
yycrank+532,	0,		0,	
yycrank+529,	0,		0,	
yycrank+539,	0,		0,	
yycrank+0,	0,		yyvstop+177,
yycrank+538,	0,		0,	
yycrank+539,	0,		0,	
yycrank+530,	0,		0,	
yycrank+526,	0,		0,	
yycrank+527,	0,		0,	
yycrank+528,	0,		0,	
yycrank+535,	0,		0,	
yycrank+542,	0,		0,	
yycrank+0,	0,		yyvstop+179,
yycrank+533,	0,		0,	
yycrank+540,	0,		0,	
yycrank+533,	0,		0,	
yycrank+0,	0,		yyvstop+181,
yycrank+0,	0,		yyvstop+183,
yycrank+585,	0,		0,	
yycrank+578,	yysvec+51,	0,	
yycrank+0,	0,		yyvstop+185,
yycrank+580,	0,		0,	
yycrank+584,	0,		0,	
yycrank+586,	0,		0,	
yycrank+573,	0,		0,	
yycrank+570,	0,		0,	
yycrank+0,	0,		yyvstop+187,
yycrank+592,	0,		0,	
yycrank+574,	0,		0,	
yycrank+582,	0,		0,	
yycrank+578,	0,		0,	
yycrank+0,	0,		yyvstop+189,
yycrank+596,	0,		0,	
yycrank+591,	0,		0,	
yycrank+594,	0,		0,	
yycrank+0,	0,		yyvstop+191,
yycrank+591,	0,		0,	
yycrank+581,	0,		0,	
yycrank+590,	0,		0,	
yycrank+590,	0,		0,	
yycrank+590,	0,		0,	
yycrank+0,	0,		yyvstop+193,
yycrank+590,	0,		0,	
yycrank+591,	0,		0,	
yycrank+601,	0,		0,	
yycrank+0,	0,		yyvstop+195,
yycrank+0,	0,		yyvstop+197,
yycrank+560,	0,		0,	
yycrank+0,	0,		yyvstop+199,
yycrank+572,	0,		0,	
yycrank+565,	0,		0,	
yycrank+565,	0,		0,	
yycrank+562,	0,		0,	
yycrank+0,	0,		yyvstop+201,
yycrank+577,	0,		0,	
yycrank+578,	0,		0,	
yycrank+561,	0,		0,	
yycrank+0,	0,		yyvstop+203,
yycrank+580,	0,		0,	
yycrank+573,	0,		0,	
yycrank+574,	0,		0,	
yycrank+575,	0,		0,	
yycrank+581,	0,		0,	
yycrank+585,	0,		0,	
yycrank+570,	0,		0,	
yycrank+573,	0,		0,	
yycrank+588,	0,		0,	
yycrank+589,	0,		0,	
yycrank+594,	0,		0,	
yycrank+589,	0,		0,	
yycrank+579,	0,		0,	
yycrank+582,	0,		0,	
yycrank+0,	0,		yyvstop+205,
yycrank+594,	0,		0,	
yycrank+580,	0,		0,	
yycrank+595,	0,		0,	
yycrank+0,	0,		yyvstop+207,
yycrank+589,	0,		0,	
yycrank+0,	0,		yyvstop+209,
yycrank+582,	0,		0,	
yycrank+585,	0,		0,	
yycrank+600,	0,		0,	
yycrank+0,	0,		yyvstop+211,
yycrank+587,	0,		0,	
yycrank+595,	0,		0,	
yycrank+0,	0,		yyvstop+213,
yycrank+624,	0,		0,	
yycrank+635,	yysvec+51,	0,	
yycrank+0,	0,		yyvstop+215,
yycrank+0,	0,		yyvstop+217,
yycrank+627,	0,		0,	
yycrank+634,	0,		0,	
yycrank+635,	0,		0,	
yycrank+636,	0,		0,	
yycrank+628,	0,		0,	
yycrank+646,	0,		0,	
yycrank+0,	0,		yyvstop+219,
yycrank+636,	0,		0,	
yycrank+644,	0,		0,	
yycrank+632,	0,		0,	
yycrank+644,	0,		0,	
yycrank+0,	0,		yyvstop+221,
yycrank+0,	0,		yyvstop+223,
yycrank+621,	0,		0,	
yycrank+0,	0,		yyvstop+225,
yycrank+640,	0,		0,	
yycrank+636,	0,		0,	
yycrank+630,	0,		0,	
yycrank+0,	0,		yyvstop+227,
yycrank+620,	0,		0,	
yycrank+0,	0,		yyvstop+229,
yycrank+0,	0,		yyvstop+231,
yycrank+0,	0,		yyvstop+233,
yycrank+0,	0,		yyvstop+235,
yycrank+0,	0,		yyvstop+237,
yycrank+620,	0,		0,	
yycrank+0,	0,		yyvstop+239,
yycrank+621,	0,		0,	
yycrank+618,	0,		0,	
yycrank+613,	0,		0,	
yycrank+624,	0,		0,	
yycrank+621,	0,		0,	
yycrank+0,	0,		yyvstop+241,
yycrank+610,	0,		0,	
yycrank+612,	0,		0,	
yycrank+630,	0,		0,	
yycrank+616,	0,		0,	
yycrank+623,	0,		0,	
yycrank+622,	0,		0,	
yycrank+0,	0,		yyvstop+243,
yycrank+0,	0,		yyvstop+245,
yycrank+621,	0,		0,	
yycrank+0,	0,		yyvstop+247,
yycrank+617,	0,		0,	
yycrank+0,	0,		yyvstop+249,
yycrank+626,	0,		0,	
yycrank+631,	0,		0,	
yycrank+0,	0,		yyvstop+251,
yycrank+636,	0,		0,	
yycrank+637,	0,		0,	
yycrank+0,	0,		yyvstop+253,
yycrank+657,	0,		0,	
yycrank+672,	0,		0,	
yycrank+663,	0,		0,	
yycrank+664,	0,		0,	
yycrank+654,	0,		0,	
yycrank+664,	0,		0,	
yycrank+670,	0,		0,	
yycrank+0,	0,		yyvstop+255,
yycrank+0,	0,		yyvstop+257,
yycrank+675,	0,		0,	
yycrank+680,	0,		0,	
yycrank+0,	0,		yyvstop+259,
yycrank+664,	0,		0,	
yycrank+0,	0,		yyvstop+261,
yycrank+0,	0,		yyvstop+263,
yycrank+0,	0,		yyvstop+265,
yycrank+647,	0,		0,	
yycrank+647,	0,		0,	
yycrank+640,	0,		0,	
yycrank+0,	0,		yyvstop+267,
yycrank+641,	0,		0,	
yycrank+639,	0,		0,	
yycrank+653,	0,		0,	
yycrank+639,	0,		0,	
yycrank+0,	0,		yyvstop+269,
yycrank+0,	0,		yyvstop+271,
yycrank+0,	0,		yyvstop+273,
yycrank+645,	0,		0,	
yycrank+650,	0,		0,	
yycrank+0,	0,		yyvstop+275,
yycrank+658,	0,		0,	
yycrank+662,	0,		0,	
yycrank+663,	0,		0,	
yycrank+0,	0,		yyvstop+277,
yycrank+693,	0,		0,	
yycrank+694,	0,		0,	
yycrank+0,	0,		yyvstop+279,
yycrank+0,	0,		yyvstop+281,
yycrank+0,	0,		yyvstop+283,
yycrank+0,	0,		yyvstop+285,
yycrank+701,	0,		0,	
yycrank+688,	0,		0,	
yycrank+689,	0,		0,	
yycrank+703,	0,		0,	
yycrank+694,	0,		0,	
yycrank+676,	0,		0,	
yycrank+656,	0,		0,	
yycrank+669,	0,		0,	
yycrank+660,	0,		0,	
yycrank+665,	0,		0,	
yycrank+675,	0,		0,	
yycrank+663,	0,		0,	
yycrank+0,	0,		yyvstop+287,
yycrank+664,	0,		0,	
yycrank+0,	0,		yyvstop+289,
yycrank+675,	0,		0,	
yycrank+0,	0,		yyvstop+291,
yycrank+0,	0,		yyvstop+293,
yycrank+711,	0,		0,	
yycrank+0,	0,		yyvstop+295,
yycrank+715,	0,		0,	
yycrank+700,	0,		0,	
yycrank+705,	0,		0,	
yycrank+710,	0,		0,	
yycrank+720,	0,		0,	
yycrank+704,	0,		0,	
yycrank+0,	0,		yyvstop+297,
yycrank+671,	0,		0,	
yycrank+0,	0,		yyvstop+299,
yycrank+0,	0,		yyvstop+301,
yycrank+0,	0,		yyvstop+303,
yycrank+0,	0,		yyvstop+305,
yycrank+672,	0,		0,	
yycrank+673,	0,		0,	
yycrank+0,	0,		yyvstop+307,
yycrank+714,	0,		0,	
yycrank+709,	0,		0,	
yycrank+709,	0,		0,	
yycrank+724,	0,		0,	
yycrank+727,	0,		0,	
yycrank+726,	0,		0,	
yycrank+0,	0,		yyvstop+309,
yycrank+0,	0,		yyvstop+311,
yycrank+695,	0,		0,	
yycrank+728,	0,		0,	
yycrank+725,	0,		0,	
yycrank+715,	0,		0,	
yycrank+733,	0,		0,	
yycrank+732,	0,		0,	
yycrank+731,	0,		0,	
yycrank+703,	0,		0,	
yycrank+0,	0,		yyvstop+313,
yycrank+736,	0,		0,	
yycrank+736,	0,		0,	
yycrank+722,	0,		0,	
yycrank+724,	0,		0,	
yycrank+735,	0,		0,	
yycrank+0,	0,		yyvstop+315,
yycrank+730,	0,		0,	
yycrank+728,	0,		0,	
yycrank+728,	0,		0,	
yycrank+0,	0,		yyvstop+317,
yycrank+733,	0,		0,	
yycrank+731,	0,		0,	
yycrank+731,	0,		0,	
yycrank+0,	0,		yyvstop+319,
yycrank+737,	0,		0,	
yycrank+733,	0,		0,	
yycrank+0,	0,		yyvstop+321,
yycrank+0,	0,		yyvstop+323,
yycrank+0,	0,		yyvstop+325,
0,	0,	0};
struct yywork *yytop = yycrank+816;
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
01  ,'#' ,01  ,'#' ,'#' ,01  ,01  ,01  ,
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
