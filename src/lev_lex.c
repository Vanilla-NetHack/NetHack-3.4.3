# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
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
FILE *yyin ={stdin}, *yyout ={stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
/*	SCCS Id: @(#)lev_lex.c	3.0	89/07/02
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev_comp.h"
#include "sp_lev.h"

int line_number = 1;

/* This is *** UGLY *** but I can't think a better way to do it
 * I really need a huge buffer to scan maps...
 */

#undef YYLMAX
#define YYLMAX	2048

# define MAPC 2
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	{ line_number++; }
break;
case 2:
	return MAZE_ID;
break;
case 3:
	return LEVEL_ID;
break;
case 4:
return GEOMETRY_ID;
break;
case 5:
	{ BEGIN MAPC; }
break;
case 6:
{ line_number++; yymore(); }
break;
case 7:
{ BEGIN 0;
		  yytext[yyleng-7] = 0; /* Discard \nENDMAP */
		  yylval.map = (char *) alloc(strlen(yytext)+1);
		  strcpy(yylval.map, yytext+1);
		  return MAP_ID;
		}
break;
case 8:
	return OBJECT_ID;
break;
case 9:
	return MONSTER_ID;
break;
case 10:
	return TRAP_ID;
break;
case 11:
	return DOOR_ID;
break;
case 12:
return DRAWBRIDGE_ID;
break;
case 13:
return MAZEWALK_ID;
break;
case 14:
	return REGION_ID;
break;
case 15:
return RANDOM_OBJECTS_ID;
break;
case 16:
return RANDOM_MONSTERS_ID;
break;
case 17:
return RANDOM_PLACES_ID;
break;
case 18:
	return ALTAR_ID;
break;
case 19:
	return LADDER_ID;
break;
case 20:
return NON_DIGGABLE_ID;
break;
case 21:
	return ROOM_ID;
break;
case 22:
	{ yylval.i=D_ISOPEN; return DOOR_STATE; }
break;
case 23:
	{ yylval.i=D_CLOSED; return DOOR_STATE; }
break;
case 24:
	{ yylval.i=D_LOCKED; return DOOR_STATE; }
break;
case 25:
	{ yylval.i=D_NODOOR; return DOOR_STATE; }
break;
case 26:
	{ yylval.i=D_BROKEN; return DOOR_STATE; }
break;
case 27:
	{ yylval.i=W_NORTH; return DIRECTION; }
break;
case 28:
	{ yylval.i=W_EAST; return DIRECTION; }
break;
case 29:
	{ yylval.i=W_SOUTH; return DIRECTION; }
break;
case 30:
	{ yylval.i=W_WEST; return DIRECTION; }
break;
case 31:
	{ yylval.i = -1; return RANDOM_TYPE; }
break;
case 32:
	return O_REGISTER;
break;
case 33:
	return M_REGISTER;
break;
case 34:
	return P_REGISTER;
break;
case 35:
	return A_REGISTER;
break;
case 36:
	{ yylval.i=1; return LEFT_OR_RIGHT; }
break;
case 37:
	{ yylval.i=3; return LEFT_OR_RIGHT; }
break;
case 38:
	{ yylval.i=2; return CENTER; }
break;
case 39:
	{ yylval.i=1; return TOP_OR_BOT; }
break;
case 40:
	{ yylval.i=3; return TOP_OR_BOT; }
break;
case 41:
	{ yylval.i=1; return LIGHT_STATE; }
break;
case 42:
	{ yylval.i=0; return LIGHT_STATE; }
break;
case 43:
	{ yylval.i=A_LAW; return ALIGNMENT; }
break;
case 44:
	{ yylval.i=A_NEUTRAL; return ALIGNMENT; }
break;
case 45:
	{ yylval.i=A_CHAOS; return ALIGNMENT; }
break;
case 46:
	{ yylval.i=1; return ALTAR_TYPE; }
break;
case 47:
	{ yylval.i=0; return ALTAR_TYPE; }
break;
case 48:
	{ yylval.i=1; return UP_OR_DOWN; }
break;
case 49:
	{ yylval.i=0; return UP_OR_DOWN; }
break;
case 50:
	{ yylval.i=atoi(yytext); return INTEGER; }
break;
case 51:
{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
break;
case 52:
	{ line_number++; }
break;
case 53:
	;
break;
case 54:
	{ yylval.i = yytext[1]; return CHAR; }
break;
case 55:
	{ return yytext[0]; }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] ={
0,

55,
0,

53,
55,
0,

52,
0,

55,
0,

55,
0,

50,
55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

55,
0,

6,
52,
0,

53,
55,
0,

55,
0,

55,
0,

55,
0,

53,
0,

51,
0,

50,
0,

48,
0,

1,
0,

6,
0,

53,
0,

1,
6,
0,

54,
0,

43,
0,

41,
0,

39,
0,

-5,
0,

11,
0,

2,
0,

21,
0,

10,
0,

49,
0,

28,
0,

36,
0,

22,
0,

30,
0,

5,
0,

18,
0,

3,
0,

35,
0,

47,
0,

45,
0,

27,
0,

34,
0,

37,
0,

29,
0,

42,
0,

19,
0,

8,
0,

14,
0,

40,
0,

26,
0,

38,
0,

23,
0,

24,
0,

25,
0,

32,
0,

31,
0,

46,
0,

-7,
0,

9,
0,

33,
0,

44,
0,

7,
0,

4,
0,

13,
0,

12,
0,

20,
0,

17,
0,

15,
0,

16,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	1,5,	0,0,	
0,0,	0,0,	0,0,	0,0,	
8,43,	0,0,	1,6,	1,7,	
9,45,	0,0,	6,42,	0,0,	
8,43,	8,43,	0,0,	0,0,	
9,45,	9,0,	41,94,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,6,	0,0,	1,8,	
1,5,	6,42,	0,0,	8,43,	
1,9,	8,44,	8,43,	9,45,	
41,95,	9,45,	9,45,	41,95,	
45,96,	1,10,	0,0,	0,0,	
0,0,	0,0,	0,0,	8,43,	
0,0,	0,0,	0,0,	9,45,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,11,	14,51,	
15,53,	1,12,	13,50,	14,52,	
1,13,	17,56,	36,89,	49,99,	
51,101,	1,14,	1,15,	1,16,	
1,17,	11,47,	15,54,	1,18,	
12,48,	1,19,	16,55,	12,49,	
2,5,	18,57,	19,60,	40,93,	
47,97,	18,58,	48,98,	50,100,	
2,6,	2,7,	1,20,	1,21,	
1,22,	1,23,	1,24,	18,59,	
24,68,	34,86,	52,102,	53,103,	
54,104,	1,25,	1,26,	1,27,	
1,28,	1,29,	20,61,	1,30,	
1,31,	1,32,	1,33,	2,6,	
1,34,	2,8,	2,35,	23,67,	
21,62,	26,73,	2,9,	21,63,	
29,78,	32,83,	2,5,	55,105,	
2,5,	2,5,	28,76,	2,10,	
10,46,	10,46,	10,46,	10,46,	
10,46,	10,46,	10,46,	10,46,	
10,46,	10,46,	22,64,	27,74,	
28,77,	22,65,	56,106,	30,79,	
2,11,	22,66,	31,81,	2,12,	
57,107,	27,75,	2,13,	30,80,	
58,108,	31,82,	2,5,	2,14,	
2,36,	2,16,	2,17,	59,109,	
25,69,	2,18,	2,5,	2,19,	
25,70,	33,84,	60,110,	33,85,	
25,71,	62,113,	61,111,	2,5,	
63,114,	64,115,	25,72,	65,116,	
2,20,	2,21,	2,22,	2,23,	
2,24,	61,112,	66,117,	3,37,	
67,118,	68,119,	69,120,	2,25,	
2,26,	2,27,	2,28,	2,29,	
35,87,	2,30,	2,31,	2,32,	
2,33,	70,121,	2,34,	71,122,	
35,87,	35,88,	2,5,	2,5,	
2,5,	3,38,	72,123,	73,124,	
3,39,	74,125,	75,126,	76,128,	
3,9,	77,129,	78,130,	79,131,	
80,132,	81,133,	82,134,	83,135,	
84,136,	86,137,	89,138,	35,87,	
75,127,	35,87,	35,87,	93,139,	
97,140,	98,141,	99,142,	95,94,	
89,103,	100,143,	101,144,	102,145,	
103,146,	104,147,	3,11,	35,87,	
105,148,	3,12,	3,40,	106,149,	
3,13,	107,150,	108,151,	109,152,	
110,153,	3,14,	3,15,	3,16,	
3,17,	95,95,	111,154,	3,18,	
95,95,	3,19,	112,155,	113,156,	
114,157,	115,158,	116,159,	117,160,	
118,161,	119,162,	121,163,	123,164,	
124,165,	4,37,	3,20,	3,21,	
3,22,	3,23,	3,24,	125,166,	
126,167,	127,168,	128,169,	129,170,	
130,171,	3,25,	3,26,	3,27,	
3,28,	3,29,	131,172,	3,30,	
3,31,	3,32,	3,33,	4,38,	
3,34,	132,173,	4,41,	133,174,	
134,175,	136,176,	4,9,	137,177,	
138,178,	139,179,	4,39,	140,180,	
4,39,	4,39,	142,181,	143,182,	
144,183,	145,184,	146,185,	147,186,	
148,187,	149,188,	150,189,	151,190,	
154,191,	155,192,	156,193,	157,194,	
158,195,	159,196,	160,197,	164,198,	
4,11,	165,199,	166,200,	4,12,	
4,40,	167,201,	4,13,	168,202,	
169,203,	171,204,	4,39,	4,14,	
4,36,	4,16,	4,17,	172,205,	
173,206,	4,18,	4,39,	4,19,	
174,207,	175,208,	176,209,	179,210,	
181,211,	182,212,	183,213,	4,39,	
185,214,	186,215,	38,42,	38,90,	
4,20,	4,21,	4,22,	4,23,	
4,24,	187,216,	188,217,	189,218,	
190,219,	193,220,	194,221,	4,25,	
4,26,	4,27,	4,28,	4,29,	
195,222,	4,30,	4,31,	4,32,	
4,33,	38,91,	4,34,	39,90,	
38,92,	197,223,	4,39,	4,39,	
4,39,	198,224,	199,225,	200,226,	
38,92,	201,227,	38,92,	38,92,	
203,228,	205,229,	207,230,	210,231,	
211,232,	212,233,	214,234,	215,235,	
216,236,	39,92,	218,237,	225,238,	
39,92,	226,239,	231,240,	232,241,	
233,242,	234,243,	236,244,	241,248,	
39,92,	244,249,	39,92,	39,92,	
237,245,	245,250,	237,246,	237,247,	
38,92,	246,251,	247,252,	248,253,	
249,254,	250,255,	251,256,	252,257,	
38,92,	254,258,	255,259,	256,260,	
257,261,	258,262,	259,263,	260,264,	
261,265,	38,92,	263,266,	264,267,	
265,268,	266,269,	267,270,	269,271,	
39,92,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
39,92,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	39,92,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
38,92,	38,92,	38,92,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
39,92,	39,92,	39,92,	0,0,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-87,	0,		0,	
yycrank+-181,	yysvec+1,	0,	
yycrank+-267,	yysvec+2,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+5,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+-7,	0,		yyvstop+8,
yycrank+-11,	0,		yyvstop+10,
yycrank+88,	0,		yyvstop+12,
yycrank+5,	0,		yyvstop+15,
yycrank+5,	0,		yyvstop+17,
yycrank+1,	0,		yyvstop+19,
yycrank+2,	0,		yyvstop+21,
yycrank+3,	0,		yyvstop+23,
yycrank+7,	0,		yyvstop+25,
yycrank+7,	0,		yyvstop+27,
yycrank+24,	0,		yyvstop+29,
yycrank+8,	0,		yyvstop+31,
yycrank+6,	0,		yyvstop+33,
yycrank+13,	0,		yyvstop+35,
yycrank+45,	0,		yyvstop+37,
yycrank+12,	0,		yyvstop+39,
yycrank+7,	0,		yyvstop+41,
yycrank+71,	0,		yyvstop+43,
yycrank+14,	0,		yyvstop+45,
yycrank+46,	0,		yyvstop+47,
yycrank+36,	0,		yyvstop+49,
yycrank+20,	0,		yyvstop+51,
yycrank+54,	0,		yyvstop+53,
yycrank+50,	0,		yyvstop+55,
yycrank+18,	0,		yyvstop+57,
yycrank+63,	0,		yyvstop+59,
yycrank+4,	0,		yyvstop+61,
yycrank+-199,	0,		yyvstop+63,
yycrank+9,	yysvec+15,	yyvstop+65,
yycrank+0,	0,		yyvstop+67,
yycrank+353,	0,		yyvstop+70,
yycrank+377,	0,		yyvstop+73,
yycrank+13,	0,		yyvstop+75,
yycrank+-12,	yysvec+35,	yyvstop+77,
yycrank+0,	yysvec+6,	yyvstop+79,
yycrank+0,	yysvec+8,	0,	
yycrank+0,	0,		yyvstop+81,
yycrank+9,	0,		0,	
yycrank+0,	yysvec+10,	yyvstop+83,
yycrank+8,	0,		0,	
yycrank+15,	0,		0,	
yycrank+10,	0,		0,	
yycrank+16,	0,		0,	
yycrank+8,	0,		0,	
yycrank+20,	0,		0,	
yycrank+17,	0,		0,	
yycrank+30,	0,		0,	
yycrank+53,	0,		0,	
yycrank+76,	0,		0,	
yycrank+78,	0,		0,	
yycrank+89,	0,		0,	
yycrank+88,	0,		0,	
yycrank+109,	0,		0,	
yycrank+73,	0,		0,	
yycrank+61,	0,		0,	
yycrank+69,	0,		0,	
yycrank+71,	0,		0,	
yycrank+86,	0,		0,	
yycrank+79,	0,		0,	
yycrank+73,	0,		0,	
yycrank+78,	0,		0,	
yycrank+75,	0,		0,	
yycrank+103,	0,		0,	
yycrank+91,	0,		0,	
yycrank+115,	0,		0,	
yycrank+105,	0,		0,	
yycrank+100,	0,		0,	
yycrank+118,	0,		0,	
yycrank+113,	0,		0,	
yycrank+120,	0,		0,	
yycrank+125,	0,		0,	
yycrank+113,	0,		0,	
yycrank+121,	0,		0,	
yycrank+111,	0,		0,	
yycrank+109,	0,		0,	
yycrank+115,	0,		0,	
yycrank+120,	0,		0,	
yycrank+0,	0,		yyvstop+85,
yycrank+114,	0,		0,	
yycrank+0,	yysvec+35,	0,	
yycrank+0,	0,		yyvstop+87,
yycrank+150,	0,		0,	
yycrank+0,	0,		yyvstop+89,
yycrank+0,	yysvec+38,	yyvstop+91,
yycrank+0,	yysvec+39,	0,	
yycrank+167,	0,		0,	
yycrank+0,	0,		yyvstop+93,
yycrank+-229,	yysvec+35,	0,	
yycrank+0,	0,		yyvstop+96,
yycrank+171,	0,		0,	
yycrank+155,	0,		0,	
yycrank+151,	0,		0,	
yycrank+164,	0,		0,	
yycrank+174,	0,		0,	
yycrank+174,	0,		0,	
yycrank+175,	0,		0,	
yycrank+162,	0,		0,	
yycrank+153,	0,		0,	
yycrank+182,	0,		0,	
yycrank+185,	0,		0,	
yycrank+181,	0,		0,	
yycrank+178,	0,		0,	
yycrank+176,	0,		0,	
yycrank+159,	0,		0,	
yycrank+169,	0,		0,	
yycrank+151,	0,		0,	
yycrank+161,	0,		0,	
yycrank+153,	0,		0,	
yycrank+159,	0,		0,	
yycrank+156,	0,		0,	
yycrank+162,	0,		0,	
yycrank+157,	0,		0,	
yycrank+0,	0,		yyvstop+98,
yycrank+158,	0,		0,	
yycrank+0,	0,		yyvstop+100,
yycrank+168,	0,		0,	
yycrank+161,	0,		0,	
yycrank+167,	0,		0,	
yycrank+173,	0,		0,	
yycrank+169,	0,		0,	
yycrank+185,	0,		0,	
yycrank+177,	0,		0,	
yycrank+189,	0,		0,	
yycrank+194,	0,		0,	
yycrank+197,	0,		0,	
yycrank+198,	0,		0,	
yycrank+188,	0,		0,	
yycrank+0,	0,		yyvstop+102,
yycrank+200,	0,		0,	
yycrank+191,	0,		0,	
yycrank+298,	0,		yyvstop+104,
yycrank+232,	0,		0,	
yycrank+229,	0,		0,	
yycrank+0,	0,		yyvstop+106,
yycrank+248,	0,		0,	
yycrank+246,	0,		0,	
yycrank+247,	0,		0,	
yycrank+241,	0,		0,	
yycrank+231,	0,		yyvstop+108,
yycrank+235,	0,		0,	
yycrank+252,	0,		0,	
yycrank+254,	0,		0,	
yycrank+243,	0,		0,	
yycrank+244,	0,		0,	
yycrank+0,	0,		yyvstop+110,
yycrank+0,	0,		yyvstop+112,
yycrank+214,	0,		0,	
yycrank+211,	0,		0,	
yycrank+215,	0,		0,	
yycrank+226,	0,		0,	
yycrank+227,	0,		0,	
yycrank+214,	0,		0,	
yycrank+229,	0,		0,	
yycrank+0,	0,		yyvstop+114,
yycrank+0,	0,		yyvstop+116,
yycrank+0,	0,		yyvstop+118,
yycrank+230,	0,		0,	
yycrank+217,	0,		0,	
yycrank+220,	0,		0,	
yycrank+226,	0,		0,	
yycrank+235,	0,		0,	
yycrank+241,	0,		0,	
yycrank+0,	0,		yyvstop+120,
yycrank+240,	0,		0,	
yycrank+236,	0,		0,	
yycrank+232,	0,		0,	
yycrank+242,	0,		0,	
yycrank+249,	0,		0,	
yycrank+238,	0,		0,	
yycrank+0,	0,		yyvstop+122,
yycrank+0,	0,		yyvstop+124,
yycrank+290,	0,		0,	
yycrank+0,	0,		yyvstop+126,
yycrank+274,	0,		0,	
yycrank+273,	0,		0,	
yycrank+276,	0,		0,	
yycrank+0,	0,		yyvstop+128,
yycrank+295,	0,		0,	
yycrank+292,	0,		0,	
yycrank+296,	0,		0,	
yycrank+286,	0,		0,	
yycrank+294,	0,		0,	
yycrank+294,	0,		0,	
yycrank+0,	0,		yyvstop+130,
yycrank+0,	0,		yyvstop+132,
yycrank+264,	0,		0,	
yycrank+264,	0,		0,	
yycrank+266,	0,		0,	
yycrank+0,	0,		yyvstop+134,
yycrank+289,	0,		0,	
yycrank+293,	0,		0,	
yycrank+293,	0,		0,	
yycrank+298,	0,		0,	
yycrank+283,	0,		0,	
yycrank+0,	0,		yyvstop+136,
yycrank+284,	0,		0,	
yycrank+0,	0,		yyvstop+138,
yycrank+292,	0,		0,	
yycrank+0,	0,		yyvstop+140,
yycrank+301,	0,		0,	
yycrank+0,	0,		yyvstop+142,
yycrank+0,	0,		yyvstop+144,
yycrank+323,	0,		0,	
yycrank+331,	0,		0,	
yycrank+323,	0,		0,	
yycrank+0,	0,		yyvstop+146,
yycrank+330,	0,		0,	
yycrank+325,	0,		0,	
yycrank+337,	0,		0,	
yycrank+0,	0,		yyvstop+148,
yycrank+315,	0,		0,	
yycrank+0,	0,		yyvstop+150,
yycrank+0,	0,		yyvstop+152,
yycrank+0,	0,		yyvstop+154,
yycrank+0,	0,		yyvstop+156,
yycrank+0,	0,		yyvstop+158,
yycrank+0,	0,		yyvstop+160,
yycrank+297,	0,		0,	
yycrank+305,	0,		0,	
yycrank+0,	0,		yyvstop+162,
yycrank+0,	0,		yyvstop+164,
yycrank+0,	0,		yyvstop+166,
yycrank+0,	0,		yyvstop+168,
yycrank+404,	0,		yyvstop+170,
yycrank+347,	0,		0,	
yycrank+327,	0,		0,	
yycrank+342,	0,		0,	
yycrank+0,	0,		yyvstop+172,
yycrank+347,	0,		0,	
yycrank+347,	0,		0,	
yycrank+0,	0,		yyvstop+174,
yycrank+0,	0,		yyvstop+176,
yycrank+0,	0,		yyvstop+178,
yycrank+348,	0,		0,	
yycrank+0,	0,		yyvstop+180,
yycrank+0,	0,		yyvstop+182,
yycrank+356,	0,		0,	
yycrank+346,	0,		0,	
yycrank+363,	0,		0,	
yycrank+354,	0,		0,	
yycrank+362,	0,		0,	
yycrank+366,	0,		0,	
yycrank+355,	0,		0,	
yycrank+360,	0,		0,	
yycrank+370,	0,		0,	
yycrank+0,	0,		yyvstop+184,
yycrank+361,	0,		0,	
yycrank+355,	0,		0,	
yycrank+370,	0,		0,	
yycrank+373,	0,		0,	
yycrank+372,	0,		0,	
yycrank+358,	0,		0,	
yycrank+376,	0,		0,	
yycrank+375,	0,		0,	
yycrank+0,	0,		yyvstop+186,
yycrank+377,	0,		0,	
yycrank+363,	0,		0,	
yycrank+365,	0,		0,	
yycrank+367,	0,		0,	
yycrank+367,	0,		0,	
yycrank+0,	0,		yyvstop+188,
yycrank+368,	0,		0,	
yycrank+0,	0,		yyvstop+190,
yycrank+0,	0,		yyvstop+192,
0,	0,	0};
struct yywork *yytop = yycrank+502;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
040 ,01  ,'"' ,'#' ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'#' ,01  ,'#' ,'#' ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'#' ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'#' ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,'#' ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,'#' ,'#' ,'#' ,01  ,01  ,
0};
char yyextra[] ={
0,0,0,0,0,1,0,1,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	ncform	4.1	83/08/11	*/

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
	int yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
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
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
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
