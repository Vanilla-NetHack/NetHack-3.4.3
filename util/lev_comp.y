%{ 
/*	SCCS Id: @(#)lev_comp.c	3.0	90/01/03
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* block some unused #defines to avoid overloading some cpp's */
#define MONDATA_H	/* comment line for pre-compiled headers */
#define MONFLAG_H	/* comment line for pre-compiled headers */

#include "hack.h"
#include "sp_lev.h"
#ifndef O_WRONLY
# include <fcntl.h>
#endif
#ifndef O_CREAT	/* some older BSD systems do not define O_CREAT in <fcntl.h> */
# include <sys/file.h>
#endif

void FDECL(yyerror, (char *));
void FDECL(yywarning, (char *));
int NDECL(yylex);
int NDECL(yyparse);

int FDECL(get_room_type, (char *));
int FDECL(get_trap_type, (char *));
int FDECL(get_monster_id, (char *, CHAR_P));
int FDECL(get_object_id, (char *, CHAR_P));
boolean FDECL(check_monster_char, (CHAR_P));
boolean FDECL(check_object_char, (CHAR_P));
void FDECL(scan_map, (char *));
void NDECL(store_part);
void FDECL(write_maze, (int, specialmaze *));

#ifdef AMIGA
char *fgets();
# undef     fopen
# undef     printf
# undef     Printf
# define    Printf  printf
#ifndef	LATTICE
# define    memset(addr,val,len)    setmem(addr,len,val)
#endif
#endif

#ifdef MSDOS
# undef exit
extern void FDECL(exit, (int));
#endif

#ifdef MACOS
# undef printf
# undef Printf
# define Printf printf
#endif

#undef NULL

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
	"sleep gas",	SLP_GAS_TRAP,
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
static digpos *tmpdig[256];
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

%}

%union
{
	int	i;
	char*	map;
}


%token	<i> CHAR INTEGER
%token	<i> MAZE_ID LEVEL_ID GEOMETRY_ID
%token	<i> OBJECT_ID MONSTER_ID TRAP_ID DOOR_ID DRAWBRIDGE_ID MAZEWALK_ID
%token	<i> REGION_ID RANDOM_OBJECTS_ID RANDOM_MONSTERS_ID RANDOM_PLACES_ID
%token	<i> ALTAR_ID LADDER_ID NON_DIGGABLE_ID ROOM_ID
%token	<i> DOOR_STATE LIGHT_STATE
%token	<i> DIRECTION RANDOM_TYPE O_REGISTER M_REGISTER P_REGISTER A_REGISTER
%token	<i> ALIGNMENT LEFT_OR_RIGHT CENTER TOP_OR_BOT ALTAR_TYPE UP_OR_DOWN
%token	<i> ',' ':' '(' ')' '[' ']'
%token	<map> STRING MAP_ID
%type	<i> h_justif v_justif trap_name room_type door_state light_state
%type	<i> alignment altar_type a_register
%type	<map> string maze_def m_name o_name
%start	file

%%
file		: /* notthing */
		| levels ;

levels		: level
		| level levels ;

level		: maze_level ;

maze_level	: maze_def regions
		  {
			  int fout, i;

			  if (fatal_error > 0)
				  fprintf(stderr,"%s : %d errors detected. No output created!\n", fname, fatal_error);
			  else {
#ifdef MACOS
				  OSErr	result;
				  
				  result = Create(CtoPstr($1), 0, CREATOR, AUXIL_TYPE);
				  (void)PtoCstr($1);
#endif		
				  fout = open($1, O_WRONLY | O_CREAT
#if defined(MSDOS) || defined(MACOS)
					      | O_BINARY
#endif /* MSDOS || MACOS */
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
		  }

maze_def	: MAZE_ID ':' string
		  {
			  $$ = $3;
		  }

regions 	: aregion
		| aregion regions ;

aregion 	: map_definition reg_init map_details
		  {
			store_part();
		  }

map_definition	: map_geometry MAP_ID
		  {
			tmppart[npart] = (mazepart *) alloc(sizeof(mazepart));
			tmppart[npart]->halign = $<i>1 % 10;
			tmppart[npart]->valign = $<i>1 / 10;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			scan_map($2);
		  }

map_geometry	: GEOMETRY_ID ':' h_justif ',' v_justif
		  {
			  $<i>$ = $<i>3 + ( $<i>5 * 10 );
		  }

h_justif	: LEFT_OR_RIGHT
		| CENTER ;

v_justif	: TOP_OR_BOT
		| CENTER ;

reg_init	: /* nothing */
		| reg_init init_reg ;

init_reg	: RANDOM_OBJECTS_ID ':' object_list
		  {
			  if (tmppart[npart]->nrobjects)
			      yyerror("Object registers already initialized!");
			  else {
				  tmppart[npart]->robjects = (char *) alloc(n_olist);
				  memcpy(tmppart[npart]->robjects, olist, n_olist);
				  tmppart[npart]->nrobjects = n_olist;
			  }
		  }
		| RANDOM_PLACES_ID ':' place_list
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
		  }
		| RANDOM_MONSTERS_ID ':' monster_list
		  {
			  if (tmppart[npart]->nrmonst)
			      yyerror("Monster registers already initialized!");
			  else {
				  tmppart[npart]->rmonst = (char *) alloc(n_mlist);
				  memcpy(tmppart[npart]->rmonst, mlist, n_mlist);
				  tmppart[npart]->nrmonst = n_mlist;
			  }
		  }

object_list	: object
		  {
			  if (n_olist < MAX_REGISTERS)
			      olist[n_olist++] = $<i>1;
			  else
			      yyerror("Object list too long!");
		  }
		| object ',' object_list
		  {
			  if (n_olist < MAX_REGISTERS)
			      olist[n_olist++] = $<i>1;
			  else
			      yyerror("Object list too long!");
		  }

monster_list	: monster
		  {
			  if (n_mlist < MAX_REGISTERS)
			      mlist[n_mlist++] = $<i>1;
			  else
			      yyerror("Monster list too long!");
		  }
		| monster ',' monster_list
		  {
			  if (n_mlist < MAX_REGISTERS)
			      mlist[n_mlist++] = $<i>1;
			  else
			      yyerror("Monster list too long!");
		  }

place_list	: place
		  {
			  if (n_plist < MAX_REGISTERS)
			      plist[n_plist++] = current_coord;
			  else
			      yyerror("Location list too long!");
		  }
		| place
		  {
			  if (n_plist < MAX_REGISTERS)
			      plist[n_plist++] = current_coord;
			  else
			      yyerror("Location list too long!");
		  } ',' place_list

map_details	: /* nothing */
		| map_details map_detail ;

map_detail	: monster_detail
		| object_detail
		| door_detail
		| trap_detail
		| drawbridge_detail
		| region_detail
		| altar_detail
		| mazewalk_detail
		| ladder_detail
		| diggable_detail ;

monster_detail	: MONSTER_ID ':' monster_c ',' m_name ',' coordinate
		  {
			  int token;

			  tmpmonst[nmons] = (monster *) alloc(sizeof(monster));
			  tmpmonst[nmons]->x = current_coord.x;
			  tmpmonst[nmons]->y = current_coord.y;
			  tmpmonst[nmons]->class = $<i>3;
			  if (!$5)
			      tmpmonst[nmons]->id = -1;
			  else {
				  token = get_monster_id($5, (char) $<i>3);
				  if (token == ERR) {
				      yywarning("Illegal monster name!  Making random monster.");
				      tmpmonst[nmons]->id = -1;
				  } else
				      tmpmonst[nmons]->id = token;
			  }
			  nmons++;
		  }

object_detail	: OBJECT_ID ':' object_c ',' o_name ',' coordinate
		  {
			  int token;

			  tmpobj[nobj] = (object *) alloc(sizeof(object));
			  tmpobj[nobj]->x = current_coord.x;
			  tmpobj[nobj]->y = current_coord.y;
			  tmpobj[nobj]->class = $<i>3;
			  if (!$5)
			      tmpobj[nobj]->id = -1;
			  else {
				  token = get_object_id($5, (char) $<i>3);
				  if (token == ERR) {
				      yywarning("Illegal object name!  Making random object.");
				      tmpobj[nobj]->id = -1;
				  } else
				      tmpobj[nobj]->id = token;
			  }
			  nobj++;
		  }

door_detail	: DOOR_ID ':' door_state ',' coordinate
		  {
			tmpdoor[ndoor] = (door *) alloc(sizeof(door));
			tmpdoor[ndoor]->x = current_coord.x;
			tmpdoor[ndoor]->y = current_coord.y;
			tmpdoor[ndoor]->mask = $<i>3;
			ndoor++;
		  }

trap_detail	: TRAP_ID ':' trap_name ',' coordinate
		  {
			tmptrap[ntrap] = (trap *) alloc(sizeof(trap));
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = $<i>3;
			ntrap++;
		  }

drawbridge_detail: DRAWBRIDGE_ID ':' coordinate ',' DIRECTION ',' door_state
		   {
			tmpdb[ndb] = (drawbridge *) alloc(sizeof(drawbridge));
			tmpdb[ndb]->x = current_coord.x;
			tmpdb[ndb]->y = current_coord.y;
			tmpdb[ndb]->dir = db_dirs[$5];
			if ( $<i>7 == D_ISOPEN )
			  tmpdb[ndb]->open = 1;
			else if ( $<i>7 == D_CLOSED )
			  tmpdb[ndb]->open = 0;
			else
			  yyerror("A drawbridge can only be open or closed!");
			ndb++;
		   }

mazewalk_detail : MAZEWALK_ID ':' coordinate ',' DIRECTION
		  {
			tmpwalk[nwalk] = (walk *) alloc(sizeof(walk));
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = $5;
			nwalk++;
		  }

ladder_detail	: LADDER_ID ':' coordinate ',' UP_OR_DOWN
		  {
			tmplad[nlad] = (lad *) alloc(sizeof(lad));
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = $<i>5;
			nlad++;
		  }

diggable_detail : NON_DIGGABLE_ID ':' region
		  {
			tmpdig[ndig] = (digpos *) alloc(sizeof(digpos));
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
		  }

region_detail	: REGION_ID ':' region ',' light_state ',' room_type
		  {
			tmpreg[nreg] = (region *) alloc(sizeof(region));
			tmpreg[nreg]->x1 = current_region.x1;
			tmpreg[nreg]->y1 = current_region.y1;
			tmpreg[nreg]->x2 = current_region.x2;
			tmpreg[nreg]->y2 = current_region.y2;
			tmpreg[nreg]->rlit = $<i>5;
			tmpreg[nreg]->rtype = $<i>7;
			nreg++;
		  }

altar_detail	: ALTAR_ID ':' coordinate ',' alignment ',' altar_type
		  {
#ifndef ALTARS
			yywarning("Altars are not allowed in this version!  Ignoring...");
#else
			tmpaltar[naltar] = (altar *) alloc(sizeof(altar));
			tmpaltar[naltar]->x = current_coord.x;
			tmpaltar[naltar]->y = current_coord.y;
			tmpaltar[naltar]->align = $<i>5;
			tmpaltar[naltar]->shrine = $<i>7;
			naltar++;
#endif /* ALTARS */
		  }

monster_c	: monster
		| RANDOM_TYPE
		  {
			  $<i>$ = - MAX_REGISTERS - 1;
		  }
		| m_register ;

object_c	: object
		| RANDOM_TYPE
		  {
			  $<i>$ = - MAX_REGISTERS - 1;
		  }
		| o_register;

m_name		: string
		| RANDOM_TYPE
		  {
			  $$ = (char *) 0;
		  }

o_name		: string
		| RANDOM_TYPE
		  {
			  $$ = (char *) 0;
		  }

trap_name	: string
		  {
			int token = get_trap_type($1);
			if (token == ERR)
				yyerror("unknown trap type!");
			$<i>$ = token;
		  }
		| RANDOM_TYPE

room_type	: string
		  {
			int token = get_room_type($1);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				$<i>$ = OROOM;
			} else
				$<i>$ = token;
		  }
		| RANDOM_TYPE

coordinate	: coord
		| p_register
		| RANDOM_TYPE
		  {
			  current_coord.x = current_coord.y = -MAX_REGISTERS-1;
		  }

door_state	: DOOR_STATE
		| RANDOM_TYPE

light_state	: LIGHT_STATE
		| RANDOM_TYPE

alignment	: ALIGNMENT
		| a_register
		| RANDOM_TYPE
		  {
			  $<i>$ = - MAX_REGISTERS - 1;
		  }

altar_type	: ALTAR_TYPE
		| RANDOM_TYPE

p_register	: P_REGISTER '[' INTEGER ']'
		  {
			if ( $3 >= MAX_REGISTERS ) {
				yyerror("Register Index overflow!");
			} else {
				current_coord.x = current_coord.y = - $3 - 1;
			}
		  }

o_register	: O_REGISTER '[' INTEGER ']'
		  {
			if ( $3 >= MAX_REGISTERS ) {
				yyerror("Register Index overflow!");
			} else {
				$<i>$ = - $3 - 1;
			}
		  }

m_register	: M_REGISTER '[' INTEGER ']'
		  {
			if ( $3 >= MAX_REGISTERS ) {
				yyerror("Register Index overflow!");
			} else {
				$<i>$ = - $3 - 1;
			}
		  }

a_register	: A_REGISTER '[' INTEGER ']'
		  {
			if ( $3 >= 3 ) {
				yyerror("Register Index overflow!");
			} else {
				$<i>$ = - $3 - 1;
			}
		  }

place		: coord

monster 	: CHAR
		  {
			if (check_monster_char((char) $1))
				$<i>$ = $1 ;
			else {
				yyerror("unknown monster class!");
				$<i>$ = ERR;
			}
		  }

object		: CHAR
		  {
			char c;

			c = $1;
#ifndef SPELLS
			if ( c == '+') {
				c = '?';
				yywarning("Spellbooks are not allowed in this version! (converted into scroll)");
			}
#endif
			if (check_object_char(c))
				$<i>$ = c;
			else {
				yyerror("Unknown char class!");
				$<i>$ = ERR;
			}
		  }
string		: STRING

coord		: '(' INTEGER ',' INTEGER ')'
		  {
			if ($2 < 0 || $2 > max_x_map ||
			    $4 < 0 || $4 > max_y_map)
			    yyerror("Coordinates out of map range!");
			current_coord.x = $2;
			current_coord.y = $4;
		  }

region		: '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'
		  {
			if ($2 < 0 || $2 > max_x_map ||
			    $4 < 0 || $4 > max_y_map ||
			    $6 < 0 || $6 > max_x_map ||
			    $8 < 0 || $8 > max_y_map)
			    yyerror("Region out of map range!");
			current_region.x1 = $2;
			current_region.y1 = $4;
			current_region.x2 = $6;
			current_region.y2 = $8;
		  }


%%

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
		return ((int) room_types[i].type);
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
		return((int)trap_types[i].type);
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
		tmppart[npart]->digs = (digpos **) alloc(sizeof(digpos*) * ndig);
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
		    (void) write(fd,(genericptr_t) pt->digs[j], sizeof(digpos));
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
