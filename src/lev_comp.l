%{
/*	SCCS Id: @(#)lev_lex.c	3.0	90/01/04
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

#define LEV_LEX_C

/* block some unused #defines to avoid overloading some cpp's */
#define MONDATA_H	/* comment this line for pre-compiled headers */
#define MONFLAG_H	/* comment this line for pre-compiled headers */

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
int FDECL (yyunput, (int));
int FDECL (yyoutput, (int));
#endif

#ifdef MSDOS
#undef exit
#endif

/* this doesn't always get put in lev_comp.h
 * (esp. when using older versions of bison)
 */

extern YYSTYPE yylval;

#ifdef MACOS
#undef putchar
#undef putc
#undef printf
#undef Printf
#define Printf printf
# ifdef LSC
#define	putc(c,stream)	(fputc(c,stream))
#define	putchar(c)	(fputc(c,stdout))
# endif
#endif
int line_number = 1;

/* This is *** UGLY *** but I can't think a better way to do it
 * I really need a huge buffer to scan maps...
 */

#undef YYLMAX
#define YYLMAX	2048

%}
%s MAPC
%%
<MAPC>[-|}{+SK\\#. ]*\n	{ line_number++; yymore(); }	
<MAPC>ENDMAP\n	{ BEGIN 0;
		  line_number++;
		  yytext[yyleng-7] = 0; /* Discard \nENDMAP */
		  yylval.map = (char *) alloc(strlen(yytext)+1);
		  strcpy(yylval.map, yytext);
		  return MAP_ID;
		}
^#.*\n		{ line_number++; }
MAZE		return MAZE_ID;
LEVEL		return LEVEL_ID;
GEOMETRY	return GEOMETRY_ID;
^MAP\n		{ BEGIN MAPC; line_number++; }
OBJECT		return OBJECT_ID;
MONSTER		return MONSTER_ID;
TRAP		return TRAP_ID;
DOOR		return DOOR_ID;
DRAWBRIDGE	return DRAWBRIDGE_ID;
MAZEWALK	return MAZEWALK_ID;
REGION		return REGION_ID;
RANDOM_OBJECTS	return RANDOM_OBJECTS_ID;
RANDOM_MONSTERS	return RANDOM_MONSTERS_ID;
RANDOM_PLACES	return RANDOM_PLACES_ID;
ALTAR		return ALTAR_ID;
LADDER		return LADDER_ID;
NON_DIGGABLE	return NON_DIGGABLE_ID;
ROOM		return ROOM_ID;
open		{ yylval.i=D_ISOPEN; return DOOR_STATE; }
closed		{ yylval.i=D_CLOSED; return DOOR_STATE; }
locked		{ yylval.i=D_LOCKED; return DOOR_STATE; }
nodoor		{ yylval.i=D_NODOOR; return DOOR_STATE; }
broken		{ yylval.i=D_BROKEN; return DOOR_STATE; }
north		{ yylval.i=W_NORTH; return DIRECTION; }
east		{ yylval.i=W_EAST; return DIRECTION; }
south		{ yylval.i=W_SOUTH; return DIRECTION; }
west		{ yylval.i=W_WEST; return DIRECTION; }
random		{ yylval.i = -1; return RANDOM_TYPE; }
object		return O_REGISTER;
monster		return M_REGISTER;
place		return P_REGISTER;
align		return A_REGISTER;
left		{ yylval.i=1; return LEFT_OR_RIGHT; }
right		{ yylval.i=3; return LEFT_OR_RIGHT; }
center		{ yylval.i=2; return CENTER; }
top		{ yylval.i=1; return TOP_OR_BOT; }
bottom		{ yylval.i=3; return TOP_OR_BOT; }
lit		{ yylval.i=1; return LIGHT_STATE; }
unlit		{ yylval.i=0; return LIGHT_STATE; }
law		{ yylval.i=A_LAW; return ALIGNMENT; }
neutral		{ yylval.i=A_NEUTRAL; return ALIGNMENT; }
chaos		{ yylval.i=A_CHAOS; return ALIGNMENT; }
shrine		{ yylval.i=1; return ALTAR_TYPE; }
altar		{ yylval.i=0; return ALTAR_TYPE; }
up		{ yylval.i=1; return UP_OR_DOWN; }
down		{ yylval.i=0; return UP_OR_DOWN; }
[0-9]+		{ yylval.i=atoi(yytext); return INTEGER; }
\"[^"]*\"	{ yytext[yyleng-1] = 0; /* Discard the trailing \" */
		  yylval.map = (char *) alloc(strlen(yytext+1)+1);
		  strcpy(yylval.map, yytext+1); /* Discard the first \" */
		  return STRING; }
\n		{ line_number++; }
[ \t]+		;
'.'		{ yylval.i = yytext[1]; return CHAR; }
.		{ return yytext[0]; }
%%
#ifdef	AMIGA
long *alloc(n)
	unsigned n;
{
	return ((long *)malloc (n));
}
#endif
