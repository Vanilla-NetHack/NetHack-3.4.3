
# line 1 "lev_comp.y"
 
/*	SCCS Id: @(#)lev_comp.c	3.0	89/07/02
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* block some unused #defines to avoid overloading some cpp's */
#define MONDATA_H
#include "hack.h"
#include "sp_lev.h"
#include <fcntl.h>

#ifdef AMIGA
char *fgets();
# define    alloc   malloc
# undef     fopen
# undef     printf
# undef     Printf
# define    Printf  printf
# define    memset(addr,val,len)    setmem(addr,len,val)
#endif

#ifdef MSDOS
# undef exit
#endif

#define MAX_REGISTERS	10
#define ERR		(-1)

struct reg {
	int x1, y1;
	int x2, y2;
}		current_region;

struct coord {
	int x;
	int y;
}		current_coord;

struct {
	char *name;
	short type;
} trap_types[TRAPNUM-1] = {
	"monster",	MONST_TRAP,
	"statue",	STATUE_TRAP,
	"bear",		BEAR_TRAP,
	"arrow",	ARROW_TRAP,
	"dart",		DART_TRAP,
	"trapdoor",	TRAPDOOR,
	"teleport",	TELEP_TRAP,
	"pit",		PIT,
	"sleeping gas",	SLP_GAS_TRAP,
	"magic",	MGTRP,
	"board",	SQBRD,
	"web",		WEB,
	"spiked pit",	SPIKED_PIT,
	"level teleport",LEVEL_TELEP,
#ifdef SPELLS
	"anti magic",	ANTI_MAGIC,
#endif
	"rust",		RUST_TRAP
#ifdef POLYSELF
	, "polymorph",	POLY_TRAP
#endif
#ifdef ARMY
	, "land mine",	LANDMINE
#endif
  };

struct {
	char *name;
	int type;
} room_types[SHOPBASE-1] = {
	/* for historical reasons, room types are not contiguous numbers */
	/* (type 1 is skipped) */
	"ordinary",	OROOM,
#ifdef THRONES
	"throne",	COURT,
#endif
	"swamp",	SWAMP,
	"vault",	VAULT,
	"beehive",	BEEHIVE,
	"morgue",	MORGUE,
#ifdef ARMY
	"barracks",	BARRACKS,
#endif
	"zoo",		ZOO,
	"temple",	TEMPLE,
	"shop",		SHOPBASE,
};

short db_dirs[4] = {
	DB_NORTH,
	DB_EAST,
	DB_SOUTH,
	DB_WEST
};

#ifdef ALTARS
static altar *tmpaltar[256];
#endif /* ALTARS /**/
static lad *tmplad[256];
static dig *tmpdig[256];
static char *tmpmap[ROWNO];
static region *tmpreg[16];
static door *tmpdoor[256];
static trap *tmptrap[256];
static monster *tmpmonst[256];
static object *tmpobj[256];
static drawbridge *tmpdb[256];
static walk *tmpwalk[256];
static mazepart *tmppart[10];
static room *tmproom[MAXNROFROOMS];
static specialmaze maze;

static char olist[MAX_REGISTERS], mlist[MAX_REGISTERS];
static struct coord plist[MAX_REGISTERS];
static int n_olist = 0, n_mlist = 0, n_plist = 0;

unsigned int nreg = 0, ndoor = 0, ntrap = 0, nmons = 0, nobj = 0;
unsigned int ndb = 0, nwalk = 0, npart = 0, ndig = 0, nlad = 0;
#ifdef ALTARS
unsigned int naltar = 0;
#endif /* ALTARS /*/

unsigned int max_x_map, max_y_map;

extern int fatal_error;
extern char* fname;

boolean check_monster_char(), check_object_char();
void scan_map(), store_part(), write_maze();


# line 140 "lev_comp.y"
typedef union 
{
	int	i;
	char*	map;
} YYSTYPE;
# define CHAR 257
# define INTEGER 258
# define MAZE_ID 259
# define LEVEL_ID 260
# define GEOMETRY_ID 261
# define OBJECT_ID 262
# define MONSTER_ID 263
# define TRAP_ID 264
# define DOOR_ID 265
# define DRAWBRIDGE_ID 266
# define MAZEWALK_ID 267
# define REGION_ID 268
# define RANDOM_OBJECTS_ID 269
# define RANDOM_MONSTERS_ID 270
# define RANDOM_PLACES_ID 271
# define ALTAR_ID 272
# define LADDER_ID 273
# define NON_DIGGABLE_ID 274
# define ROOM_ID 275
# define DOOR_STATE 276
# define LIGHT_STATE 277
# define DIRECTION 278
# define RANDOM_TYPE 279
# define O_REGISTER 280
# define M_REGISTER 281
# define P_REGISTER 282
# define A_REGISTER 283
# define ALIGNMENT 284
# define LEFT_OR_RIGHT 285
# define CENTER 286
# define TOP_OR_BOT 287
# define ALTAR_TYPE 288
# define UP_OR_DOWN 289
# define STRING 290
# define MAP_ID 291
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 617 "lev_comp.y"


/* 
 * Find the type of a room in the table, knowing its name.
 */

int
get_room_type(s)
char *s;
{
	register int i;
	
	for(i=0; i < SHOPBASE -1; i++)
	    if (!strcmp(s, room_types[i].name))
		return room_types[i].type;
	return ERR;
}

/* 
 * Find the type of a trap in the table, knowing its name.
 */

int
get_trap_type(s)
char *s;
{
	register int i;
	
	for(i=0; i < TRAPNUM - 1; i++)
	    if(!strcmp(s,trap_types[i].name))
		return(trap_types[i].type);
	return ERR;
}

/* 
 * Find the index of a monster in the table, knowing its name.
 */

int
get_monster_id(s, c)
char *s;
char c;
{
	register int i;
	
	for(i=0; mons[i].mname[0]; i++)
	    if(!strncmp(s, mons[i].mname, strlen(mons[i].mname))
	       && c == mons[i].mlet)
		return i;
	return ERR;
}

/* 
 * Find the index of an object in the table, knowing its name.
 */

int
get_object_id(s, c)
char *s;
char c;
{
	register int i;
	
	for(i=0; i<=NROFOBJECTS;i++)
	    if(objects[i].oc_name &&
	       !strncmp(s, objects[i].oc_name, strlen(objects[i].oc_name))
	       && c == objects[i].oc_olet)
		return i;
	return ERR;
}

/* 
 * Is the character 'c' a valid monster class ?
 */

boolean
check_monster_char(c)
char c;
{
	register int i;
	
	for(i=0; mons[i].mname[0]; i++)
	    if( c ==  mons[i].mlet)
		return 1;
	return(0);
}

/* 
 * Is the character 'c' a valid object class ?
 */

boolean
check_object_char(c)
char c;
{
	register int i;
	
	for(i=0; i<=NROFOBJECTS;i++)
	    if( c == objects[i].oc_olet)
		return 1;
	return 0;
}

/* 
 * Yep! LEX gives us the map in a raw mode.
 * Just analyze it here.
 */

void scan_map(map)
char *map;
{
	register int i, len;
	register char *s1, *s2;
	int max_len = 0;
	int max_hig = 0;
	
	/* First : find the max width of the map */

	s1 = map;
	while (s1 && *s1) {
		s2 = index(s1, '\n');
		if (s2) {
			if (s2-s1 > max_len)
			    max_len = s2-s1;
			s1 = s2 + 1;
		} else {
			if (strlen(s1) > max_len)
			    max_len = strlen(s1);
			s1 = (char *) 0;
		}
	}

	/* Then parse it now */

	while (map && *map) {
		tmpmap[max_hig] = (char *) alloc(max_len);
		s1 = index(map, '\n');
		if (s1) {
			len = s1 - map;
			s1++;
		} else {
			len = strlen(map);
			s1 = map + len;
		}
		for(i=0; i<len; i++)
		    switch(map[i]) {
			  case '-' : tmpmap[max_hig][i] = HWALL; break;
			  case '|' : tmpmap[max_hig][i] = VWALL; break;
			  case '+' : tmpmap[max_hig][i] = DOOR; break;
			  case 'S' : tmpmap[max_hig][i] = SDOOR; break;
			  case '{' : 
#ifdef FOUNTAINS
			      tmpmap[max_hig][i] = FOUNTAIN;
#else
			      tmpmap[max_hig][i] = ROOM;
			      yywarning("Fountains are not allowed in this version!  Ignoring...");
#endif
			      break;
			  case '\\' : 
#ifdef THRONES
			      tmpmap[max_hig][i] = THRONE;
#else
			      tmpmap[max_hig][i] = ROOM;
			      yywarning("Thrones are not allowed in this version!  Ignoring...");
#endif
			      break;
			  case 'K' : 
#ifdef SINKS
			      tmpmap[max_hig][i] = SINK;
#else
			      tmpmap[max_hig][i] = ROOM;
			      yywarning("Sinks are not allowed in this version!  Ignoring...");
#endif
			      break;
			  case '}' : tmpmap[max_hig][i] = MOAT; break;
			  case '#' : tmpmap[max_hig][i] = CORR; break;
			  default  : tmpmap[max_hig][i] = ROOM; break;
		    }
		while(i < max_len)
		    tmpmap[max_hig][i++] = ROOM;
		map = s1;
		max_hig++;
	}

	/* Memorize boundaries */

	max_x_map = max_len - 1;
	max_y_map = max_hig - 1;

	/* Store the map into the mazepart structure */

	tmppart[npart]->xsize = max_len;
	tmppart[npart]->ysize = max_hig;
	tmppart[npart]->map = (char **) alloc(max_hig*sizeof(char *));
	for(i = 0; i< max_hig; i++)
	    tmppart[npart]->map[i] = tmpmap[i];
}

/* 
 * Here we want to store the maze part we just got.
 */

void
store_part()
{
	register int i;

	/* Ok, We got the whole part, now we store it. */
	
	/* The Regions */

	if(tmppart[npart]->nreg = nreg) {
		tmppart[npart]->regions = (region**)alloc(sizeof(region*) * nreg);
		for(i=0;i<nreg;i++)
		    tmppart[npart]->regions[i] = tmpreg[i];
	}
	nreg = 0;

	/* the doors */

	if(tmppart[npart]->ndoor = ndoor) {
		tmppart[npart]->doors = (door **)alloc(sizeof(door *) * ndoor);
		for(i=0;i<ndoor;i++)
		    tmppart[npart]->doors[i] = tmpdoor[i];
	}
	ndoor = 0;

	/* the traps */

	if(tmppart[npart]->ntraps = ntrap) {
		tmppart[npart]->traps = (trap **)alloc(sizeof(trap*) * ntrap);
		for(i=0;i<ntrap;i++)
		    tmppart[npart]->traps[i] = tmptrap[i];
	}
	ntrap = 0;

	/* the monsters */

	if(tmppart[npart]->nmonster = nmons) {
		tmppart[npart]->monsters = (monster**)alloc(sizeof(monster*)*nmons);
		for(i=0;i<nmons;i++)
		    tmppart[npart]->monsters[i] = tmpmonst[i];
	}
	nmons = 0;

	/* the objects */

	if(tmppart[npart]->nobjects = nobj) {
		tmppart[npart]->objects = (object**)alloc(sizeof(object*)*nobj);
		for(i=0;i<nobj;i++)
		    tmppart[npart]->objects[i] = tmpobj[i];
	}
	nobj = 0;

	/* the drawbridges */

	if(tmppart[npart]->ndrawbridge = ndb) {
		tmppart[npart]->drawbridges = (drawbridge**)alloc(sizeof(drawbridge*)*ndb);
		for(i=0;i<ndb;i++)
		    tmppart[npart]->drawbridges[i] = tmpdb[i];
	}
	ndb = 0;

	/* The walkmaze directives */

	if(tmppart[npart]->nwalk = nwalk) {
		tmppart[npart]->walks = (walk**)alloc(sizeof(walk*)*nwalk);
		for(i=0;i<nwalk;i++)
		    tmppart[npart]->walks[i] = tmpwalk[i];
	}
	nwalk = 0;

	/* The non_diggable directives */

	if(tmppart[npart]->ndig = ndig) {
		tmppart[npart]->digs = (dig **) alloc(sizeof(dig*) * ndig);
		for(i=0;i<ndig;i++)
		    tmppart[npart]->digs[i] = tmpdig[i];
	}
	ndig = 0;

	/* The ladders */

	if(tmppart[npart]->nlad = nlad) {
		tmppart[npart]->lads = (lad **) alloc(sizeof(lad*) * nlad);
		for(i=0;i<nlad;i++)
		    tmppart[npart]->lads[i] = tmplad[i];
	}
	nlad = 0;
#ifdef ALTARS
	/* The altars */

	if(tmppart[npart]->naltar = naltar) {
		tmppart[npart]->altars = (altar**)alloc(sizeof(altar*) * naltar);
		for(i=0;i<naltar;i++)
		    tmppart[npart]->altars[i] = tmpaltar[i];
	}
	naltar = 0;
#endif /* ALTARS /**/
	npart++;
	n_plist = n_mlist = n_olist = 0;
}

/* 
 * Here we write the structure of the maze in the specified file (fd).
 * Also, we have to free the memory allocated via alloc()
 */

void
write_maze(fd, maze)
int fd;
specialmaze *maze;
{
	char c;
	short i,j;
	mazepart *pt;

	c = 2;
	(void) write(fd, &c, 1);	/* Header for special mazes */
	(void) write(fd, &(maze->numpart), 1);	/* Number of parts */
	for(i=0;i<maze->numpart;i++) {
	    pt = maze->parts[i];

	    /* First, write the map */

	    (void) write(fd, &(pt->halign), 1);
	    (void) write(fd, &(pt->valign), 1);
	    (void) write(fd, &(pt->xsize), 1);
	    (void) write(fd, &(pt->ysize), 1);
	    for(j=0;j<pt->ysize;j++) {
		    (void) write(fd, pt->map[j], pt->xsize);
		    free(pt->map[j]);
	    }
	    free(pt->map);

	    /* The random registers */
	    (void) write(fd, &(pt->nrobjects), 1);
	    if(pt->nrobjects) {
		    (void) write(fd, pt->robjects, pt->nrobjects);
		    free(pt->robjects);
	    }
	    (void) write(fd, &(pt->nloc), 1);
	    if(pt->nloc) {
		(void) write(fd,pt->rloc_x, pt->nloc);
		(void) write(fd,pt->rloc_y, pt->nloc);
		free(pt->rloc_x);
		free(pt->rloc_y);
	    }
	    (void) write(fd,&(pt->nrmonst), 1);
	    if(pt->nrmonst) {
		    (void) write(fd, pt->rmonst, pt->nrmonst);
		    free(pt->rmonst);
	    }

	    /* subrooms */
	    (void) write(fd, &(pt->nreg), 1);
	    for(j=0;j<pt->nreg;j++) {
		    (void) write(fd,(genericptr_t) pt->regions[j], sizeof(region));
		    free(pt->regions[j]);
	    }
	    if(pt->nreg > 0)
		free(pt->regions);

	    /* the doors */
	    (void) write(fd,&(pt->ndoor),1);
	    for(j=0;j<pt->ndoor;j++) {
		    (void) write(fd,(genericptr_t) pt->doors[j], sizeof(door));
		    free(pt->doors[j]);
	    }
	    if (pt->ndoor > 0)
		free(pt->doors);

	    /* The traps */
	    (void) write(fd,&(pt->ntraps), 1);
	    for(j=0;j<pt->ntraps;j++) {
		    (void) write(fd,(genericptr_t) pt->traps[j], sizeof(trap));
		    free(pt->traps[j]);
	    }
	    if (pt->ntraps)
		free(pt->traps);

	    /* The monsters */
	    (void) write(fd, &(pt->nmonster), 1);
	    for(j=0;j<pt->nmonster;j++) {
		    (void) write(fd,(genericptr_t) pt->monsters[j], sizeof(monster));
		    free(pt->monsters[j]);
	    }
	    if (pt->nmonster > 0)
		free(pt->monsters);

	    /* The objects */
	    (void) write(fd, &(pt->nobjects), 1);
	    for(j=0;j<pt->nobjects;j++) {
		    (void) write(fd,(genericptr_t) pt->objects[j], sizeof(object));
		    free(pt->objects[j]);
	    }
	    if(pt->nobjects > 0)
		free(pt->objects);

	    /* The drawbridges */
	    (void) write(fd, &(pt->ndrawbridge),1);
	    for(j=0;j<pt->ndrawbridge;j++) {
		    (void) write(fd,(genericptr_t) pt->drawbridges[j], sizeof(drawbridge));
		    free(pt->drawbridges[j]);
	    }
	    if(pt->ndrawbridge > 0)
		free(pt->drawbridges);

	    /* The mazewalk directives */
	    (void) write(fd, &(pt->nwalk), 1);
	    for(j=0; j<pt->nwalk; j++) {
		    (void) write(fd,(genericptr_t) pt->walks[j], sizeof(walk));
		    free(pt->walks[j]);
	    }
	    if (pt->nwalk > 0)
		free(pt->walks);

	    /* The non_diggable directives */
	    (void) write(fd, &(pt->ndig), 1);
	    for(j=0;j<pt->ndig;j++) {
		    (void) write(fd,(genericptr_t) pt->digs[j], sizeof(dig));
		    free(pt->digs[j]);
	    }
	    if (pt->ndig > 0)
		free(pt->digs);

	    /* The ladders */
	    (void) write(fd, &(pt->nlad), 1);
	    for(j=0;j<pt->nlad;j++) {
		    (void) write(fd,(genericptr_t) pt->lads[j], sizeof(lad));
		    free(pt->lads[j]);
	    }
	    if (pt->nlad > 0)
		free(pt->lads);
#ifdef ALTARS
	    /* The altars */
	    (void) write(fd, &(pt->naltar), 1);
	    for(j=0;j<pt->naltar;j++) {
		    (void) write(fd,(genericptr_t) pt->altars[j], sizeof(altar));
		    free(pt->altars[j]);
	    }
	    if (pt->naltar > 0)
		free(pt->altars);
#endif /* ALTARS /**/
	    free(pt);
	}
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 86
# define YYLAST 251
short yyact[]={

 164, 130, 126,  91,  16,  19, 168, 146,  69,  72,
 145,  19,  19,  19,  19, 167,  75,  74,  26,  27,
 143, 137,  12, 138, 144, 141,  65,  87, 134,   6,
  88,  78, 173,  80,  40,  39,  42,  41,  43,  46,
  44, 170, 169, 155,  45,  47,  48, 147,  83,  85,
  22,  24,  23, 135, 131, 127, 116, 105,  72,  65,
  18,  92,  71,  86, 171, 153,  67,  93, 151, 149,
 157,  64, 114, 110, 108,  97,  62,  61,  60,  59,
  58,  57,  56,  55,  54,  53,  51,  50,  49,  17,
  13, 172, 165, 156, 154, 152, 150, 148, 139, 122,
 119, 118, 117, 115, 113, 112, 111, 109, 107, 106,
 104, 103,  52, 174, 158,  69,  77,  90,  68,  98,
   9,  99, 100, 101,   3, 142,  82,   7,  94,  14,
  84,  79, 166, 140, 162, 136,  89,  81, 102,  76,
  38,  37,  36,  35,  34,  33,  32,  31,  30,  29,
  28,  70,  66,  63,  21,  73,  25,  11,  20,  15,
  10,   8,   4,   2,   1, 128, 124,   5, 125, 123,
 129, 121,  68, 132, 133, 120,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 159,   0, 160,   0,   0, 163, 161,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  95,   0,   0,
  96 };
short yypact[]={

-230,-1000,-230,-1000,-1000,-239,  32,-1000,-239,-1000,
-1000,-287,  31,-285,-1000,-219,-1000,-267,-1000,-1000,
-228,-1000,  30,  29,  28,  68,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,  27,
  26,  25,  24,  23,  22,  21,  20,  19,  18,-198,
  75,-199,-270,-248,-231,-249,-276, -32,  79, -32,
 -32, -32,  79,  67,-1000,-1000,  66,-1000,-1000,-201,
  65,-1000,-1000,-1000,-1000,-1000,  64,-1000,-1000,-1000,
 -17,  63,-1000,-1000,-1000, -18,  62,-1000,-1000,  61,
-1000,-1000,  60,-1000,-1000,-1000, -19,  59,-202,  58,
  57,  56,-1000,-198,  75,  55,-199,-277,-203,-278,
-204, -32, -32,-250,-205,-256,  54,-259,-268,-282,
-1000,-1000,-211,-1000,  53,-1000,-1000, -24,  52,-1000,
-1000, -25,-1000,-1000,  51, -28,  50,-1000,-1000,-215,
  49,-1000,-1000,-1000, -21,-1000,-1000,  73, -32,-1000,
 -32,-1000,-249,-1000,-279,  48,-273,-216,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-217,-1000,-1000,-1000, -29,
  47,-1000,-226,  72,-1000 };
short yypgo[]={

   0,  60, 167, 166, 165, 164, 163, 124, 162, 161,
 120, 160, 159, 158, 157, 156, 155, 154, 153, 152,
 151,  71,  62,  66, 150, 149, 148, 147, 146, 145,
 144, 143, 142, 141, 140, 139,  61, 137,  63, 136,
  75, 135, 134, 133, 132, 131, 130,  67, 128, 125 };
short yyr1[]={

   0,   5,   5,   6,   6,   7,   8,   2,   9,   9,
  10,  11,  14,  15,  15,  16,  16,  12,  12,  17,
  17,  17,  18,  18,  20,  20,  19,  19,  13,  13,
  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
  25,  26,  27,  28,  29,  32,  33,  34,  30,  31,
  35,  35,  35,  37,  37,  37,   3,   3,   4,   4,
  39,  39,  42,  42,  36,  36,  36,  38,  38,  41,
  41,  43,  43,  43,  44,  44,  48,  46,  45,  49,
  23,  22,  21,   1,  47,  40 };
short yyr2[]={

   0,   0,   1,   1,   2,   1,   2,   3,   1,   2,
   3,   2,   5,   1,   1,   1,   1,   0,   2,   3,
   3,   3,   1,   3,   1,   3,   1,   3,   0,   2,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   7,   7,   5,   5,   7,   5,   5,   3,   7,   7,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   4,   4,   4,   4,
   1,   1,   1,   1,   5,   9 };
short yychk[]={

-1000,  -5,  -6,  -7,  -8,  -2, 259,  -7,  -9, -10,
 -11, -14, 261,  58, -10, -12, 291,  58,  -1, 290,
 -13, -17, 269, 271, 270, -15, 285, 286, -24, -25,
 -26, -27, -28, -29, -30, -31, -32, -33, -34, 263,
 262, 265, 264, 266, 268, 272, 267, 273, 274,  58,
  58,  58,  44,  58,  58,  58,  58,  58,  58,  58,
  58,  58,  58, -18, -21, 257, -19, -23, -47,  40,
 -20, -22, 257, -16, 287, 286, -35, -22, 279, -45,
 281, -37, -21, 279, -46, 280, -38, 276, 279, -39,
  -1, 279, -36, -47, -48, 279, 282, -40,  40, -36,
 -36, -36, -40,  44,  44, 258,  44,  44,  91,  44,
  91,  44,  44,  44,  91,  44, 258,  44,  44,  44,
 -21, -23,  44, -22,  -3,  -1, 279, 258,  -4,  -1,
 279, 258, -36, -36, 278, 258, -41, 277, 279,  44,
 -43, 284, -49, 279, 283, 278, 289, 258,  44,  93,
  44,  93,  44,  93,  44, 258,  44,  91,  41, -36,
 -36, -38, -42,  -1, 279,  44, -44, 288, 279, 258,
 258,  93,  44, 258,  41 };
short yydef[]={

   1,  -2,   2,   3,   5,   0,   0,   4,   6,   8,
  17,   0,   0,   0,   9,  28,  11,   0,   7,  83,
  10,  18,   0,   0,   0,   0,  13,  14,  29,  30,
  31,  32,  33,  34,  35,  36,  37,  38,  39,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,  19,  22,  82,  20,  26,  80,   0,
  21,  24,  81,  12,  15,  16,   0,  50,  51,  52,
   0,   0,  53,  54,  55,   0,   0,  67,  68,   0,
  60,  61,   0,  64,  65,  66,   0,   0,   0,   0,
   0,   0,  47,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  23,  27,   0,  25,   0,  56,  57,   0,   0,  58,
  59,   0,  42,  43,   0,   0,   0,  69,  70,   0,
   0,  71,  72,  73,   0,  45,  46,   0,   0,  78,
   0,  77,   0,  76,   0,   0,   0,   0,  84,  40,
  41,  44,  48,  62,  63,   0,  49,  74,  75,   0,
   0,  79,   0,   0,  85 };
#ifndef lint
static char yaccpar_sccsid[] = "@(#)yaccpar	4.1	(Berkeley)	2/11/83";
#endif not lint

#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 6:
# line 170 "lev_comp.y"
{
			  int fout, i;

			  if (fatal_error > 0)
				  fprintf(stderr,"%s : %d errors detected. No output created!\n", fname, fatal_error);
			  else {
				  fout = open(yypvt[-1].map, O_WRONLY | O_CREAT
#ifdef MSDOS
					      | O_BINARY
#endif /* MSDOS */
					      , 0644);
				  if (fout < 0) {
					  yyerror("Can't open output file!!");
					  exit(1);
				  }
				  maze.numpart = npart;
				  maze.parts = (mazepart**) alloc(sizeof(mazepart*)*npart);
				  for(i=0;i<npart;i++)
				      maze.parts[i] = tmppart[i];
				  write_maze(fout, &maze);
				  (void) close(fout);
				  npart = 0;
			  }
		  } break;
case 7:
# line 196 "lev_comp.y"
{
			  yyval.map = yypvt[-0].map;
		  } break;
case 10:
# line 204 "lev_comp.y"
{
			store_part();
		  } break;
case 11:
# line 209 "lev_comp.y"
{
			tmppart[npart] = (mazepart *) alloc(sizeof(mazepart));
			tmppart[npart]->halign = yypvt[-1].i % 10;
			tmppart[npart]->valign = yypvt[-1].i / 10;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			scan_map(yypvt[-0].map);
		  } break;
case 12:
# line 220 "lev_comp.y"
{
			  yyval.i = yypvt[-2].i + ( yypvt[-0].i * 10 );
		  } break;
case 19:
# line 234 "lev_comp.y"
{
			  if (tmppart[npart]->nrobjects)
			      yyerror("Object registers already initialized!");
			  else {
				  tmppart[npart]->robjects = (char *) alloc(n_olist);
				  memcpy(tmppart[npart]->robjects, olist, n_olist);
				  tmppart[npart]->nrobjects = n_olist;
			  }
		  } break;
case 20:
# line 244 "lev_comp.y"
{
			  if (tmppart[npart]->nloc)
			      yyerror("Location registers already initialized!");
			  else {
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
case 21:
# line 259 "lev_comp.y"
{
			  if (tmppart[npart]->nrmonst)
			      yyerror("Monster registers already initialized!");
			  else {
				  tmppart[npart]->rmonst = (char *) alloc(n_mlist);
				  memcpy(tmppart[npart]->rmonst, mlist, n_mlist);
				  tmppart[npart]->nrmonst = n_mlist;
			  }
		  } break;
case 22:
# line 270 "lev_comp.y"
{
			  if (n_olist < MAX_REGISTERS)
			      olist[n_olist++] = yypvt[-0].i;
			  else
			      yyerror("Object list too long!");
		  } break;
case 23:
# line 277 "lev_comp.y"
{
			  if (n_olist < MAX_REGISTERS)
			      olist[n_olist++] = yypvt[-0].i;
			  else
			      yyerror("Object list too long!");
		  } break;
case 24:
# line 285 "lev_comp.y"
{
			  if (n_mlist < MAX_REGISTERS)
			      mlist[n_mlist++] = yypvt[-0].i;
			  else
			      yyerror("Monster list too long!");
		  } break;
case 25:
# line 292 "lev_comp.y"
{
			  if (n_mlist < MAX_REGISTERS)
			      mlist[n_mlist++] = yypvt[-0].i;
			  else
			      yyerror("Monster list too long!");
		  } break;
case 26:
# line 300 "lev_comp.y"
{
			  if (n_plist < MAX_REGISTERS)
			      plist[n_plist++] = current_coord;
			  else
			      yyerror("Location list too long!");
		  } break;
case 27:
# line 307 "lev_comp.y"
{
			  if (n_plist < MAX_REGISTERS)
			      plist[n_plist++] = current_coord;
			  else
			      yyerror("Location list too long!");
		  } break;
case 40:
# line 329 "lev_comp.y"
{
			  int token;

			  tmpmonst[nmons] = (monster *) alloc(sizeof(monster));
			  tmpmonst[nmons]->x = current_coord.x;
			  tmpmonst[nmons]->y = current_coord.y;
			  tmpmonst[nmons]->class = yypvt[-4].i;
			  if (!yypvt[-2].map)
			      tmpmonst[nmons]->id = -1;
			  else {
				  token = get_monster_id(yypvt[-2].map, (char) yypvt[-4].i);  
				  if (token == ERR) {
				      yywarning("Illegal monster name!  Making random monster.");
				      tmpmonst[nmons]->id = -1;
				  } else
				      tmpmonst[nmons]->id = token;
			  }
			  nmons++;
		  } break;
case 41:
# line 350 "lev_comp.y"
{
			  int token;

			  tmpobj[nobj] = (object *) alloc(sizeof(object));
			  tmpobj[nobj]->x = current_coord.x;
			  tmpobj[nobj]->y = current_coord.y;
			  tmpobj[nobj]->class = yypvt[-4].i;
			  if (!yypvt[-2].map)
			      tmpobj[nobj]->id = -1;
			  else {
				  token = get_object_id(yypvt[-2].map, (char) yypvt[-4].i);
				  if (token == ERR) {
				      yywarning("Illegal object name!  Making random object.");
				      tmpobj[nobj]->id = -1;
				  } else
				      tmpobj[nobj]->id = token;
			  }
			  nobj++;
		  } break;
case 42:
# line 371 "lev_comp.y"
{
			tmpdoor[ndoor] = (door *) alloc(sizeof(door));
			tmpdoor[ndoor]->x = current_coord.x;
			tmpdoor[ndoor]->y = current_coord.y;
			tmpdoor[ndoor]->mask = yypvt[-2].i;
			ndoor++;
		  } break;
case 43:
# line 380 "lev_comp.y"
{
			tmptrap[ntrap] = (trap *) alloc(sizeof(trap));
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yypvt[-2].i;
			ntrap++;
		  } break;
case 44:
# line 389 "lev_comp.y"
{
			tmpdb[ndb] = (drawbridge *) alloc(sizeof(drawbridge));
			tmpdb[ndb]->x = current_coord.x;
			tmpdb[ndb]->y = current_coord.y;
			tmpdb[ndb]->dir = db_dirs[yypvt[-2].i];
			if ( yypvt[-0].i == D_ISOPEN )
			  tmpdb[ndb]->open = 1;
			else if ( yypvt[-0].i == D_CLOSED )
			  tmpdb[ndb]->open = 0;
			else
			  yyerror("A drawbridge can only be open or closed!");
			ndb++;
		   } break;
case 45:
# line 404 "lev_comp.y"
{
			tmpwalk[nwalk] = (walk *) alloc(sizeof(walk));
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = yypvt[-0].i;
			nwalk++;
		  } break;
case 46:
# line 413 "lev_comp.y"
{
			tmplad[nlad] = (lad *) alloc(sizeof(lad));
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = yypvt[-0].i;
			nlad++;
		  } break;
case 47:
# line 422 "lev_comp.y"
{
			tmpdig[ndig] = (dig *) alloc(sizeof(dig));
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
		  } break;
case 48:
# line 432 "lev_comp.y"
{
			tmpreg[nreg] = (region *) alloc(sizeof(region));
			tmpreg[nreg]->x1 = current_region.x1;
			tmpreg[nreg]->y1 = current_region.y1;
			tmpreg[nreg]->x2 = current_region.x2;
			tmpreg[nreg]->y2 = current_region.y2;
			tmpreg[nreg]->rlit = yypvt[-2].i;
			tmpreg[nreg]->rtype = yypvt[-0].i;
			nreg++;
		  } break;
case 49:
# line 444 "lev_comp.y"
{
#ifndef ALTARS
			yywarning("Altars are not allowed in this version!  Ignoring...");
#else
			tmpaltar[naltar] = (altar *) alloc(sizeof(altar));
			tmpaltar[naltar]->x = current_coord.x;
			tmpaltar[naltar]->y = current_coord.y;
			tmpaltar[naltar]->align = yypvt[-2].i;
			tmpaltar[naltar]->shrine = yypvt[-0].i;
			naltar++;
#endif /* ALTARS */
		  } break;
case 51:
# line 459 "lev_comp.y"
{
			  yyval.i = - MAX_REGISTERS - 1;
		  } break;
case 54:
# line 466 "lev_comp.y"
{
			  yyval.i = - MAX_REGISTERS - 1;
		  } break;
case 57:
# line 473 "lev_comp.y"
{
			  yyval.map = (char *) 0;
		  } break;
case 59:
# line 479 "lev_comp.y"
{
			  yyval.map = (char *) 0;
		  } break;
case 60:
# line 484 "lev_comp.y"
{
		  	int token = get_trap_type(yypvt[-0].map);
			if (token == ERR)
				yyerror("unknown trap type!");
			yyval.i = token;
		  } break;
case 62:
# line 493 "lev_comp.y"
{
			int token = get_room_type(yypvt[-0].map);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				yyval.i = OROOM;
			} else
				yyval.i = token;
		  } break;
case 66:
# line 506 "lev_comp.y"
{
			  current_coord.x = current_coord.y = -MAX_REGISTERS-1;
		  } break;
case 73:
# line 519 "lev_comp.y"
{
			  yyval.i = - MAX_REGISTERS - 1;
		  } break;
case 76:
# line 527 "lev_comp.y"
{
			if ( yypvt[-1].i >= MAX_REGISTERS ) {
				yyerror("Register Index overflow!");
			} else {
				current_coord.x = current_coord.y = - yypvt[-1].i - 1;
			}
		  } break;
case 77:
# line 536 "lev_comp.y"
{
			if ( yypvt[-1].i >= MAX_REGISTERS ) {
				yyerror("Register Index overflow!");
			} else {
				yyval.i = - yypvt[-1].i - 1;
			}
		  } break;
case 78:
# line 545 "lev_comp.y"
{
			if ( yypvt[-1].i >= MAX_REGISTERS ) {
				yyerror("Register Index overflow!");
			} else {
				yyval.i = - yypvt[-1].i - 1;
			}
		  } break;
case 79:
# line 554 "lev_comp.y"
{
			if ( yypvt[-1].i >= 3 ) {
				yyerror("Register Index overflow!");
			} else {
				yyval.i = - yypvt[-1].i - 1;
			}
		  } break;
case 81:
# line 565 "lev_comp.y"
{
			if (check_monster_char(yypvt[-0].i))
				yyval.i = yypvt[-0].i ;
			else {
				yyerror("unknown monster class!");
				yyval.i = ERR;
			}
		  } break;
case 82:
# line 575 "lev_comp.y"
{
			char c;

			c = yypvt[-0].i;
#ifndef SPELLS
			if ( c == '+') {
				c = '?';
				yywarning("Spellbooks are not allowed in this version! (converted into scroll)");
			}
#endif
			if (check_object_char(c))
				yyval.i = c;
			else {
				yyerror("Unknown char class!");
				yyval.i = ERR;
			}
		  } break;
case 84:
# line 595 "lev_comp.y"
{
		        if (yypvt[-3].i < 0 || yypvt[-3].i > max_x_map ||
			    yypvt[-1].i < 0 || yypvt[-1].i > max_y_map)
			    yyerror("Coordinates out of map range!");
			current_coord.x = yypvt[-3].i;
			current_coord.y = yypvt[-1].i;
		  } break;
case 85:
# line 604 "lev_comp.y"
{
		        if (yypvt[-7].i < 0 || yypvt[-7].i > max_x_map ||
			    yypvt[-5].i < 0 || yypvt[-5].i > max_y_map ||
			    yypvt[-3].i < 0 || yypvt[-3].i > max_x_map ||
			    yypvt[-1].i < 0 || yypvt[-1].i > max_y_map)
			    yyerror("Region out of map range!");
			current_region.x1 = yypvt[-7].i;
			current_region.y1 = yypvt[-5].i;
			current_region.x2 = yypvt[-3].i;
			current_region.y2 = yypvt[-1].i;
		  } break; 
		}
		goto yystack;  /* stack new state and value */

	}
