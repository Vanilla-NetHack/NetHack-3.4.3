
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
extern YYSTYPE yylval;
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
# define ROOM_ID 282
# define PORTAL_ID 283
# define TELEPRT_ID 284
# define BRANCH_ID 285
# define LEV 286
# define CHANCE_ID 287
# define CORRIDOR_ID 288
# define GOLD_ID 289
# define ENGRAVING_ID 290
# define FOUNTAIN_ID 291
# define POOL_ID 292
# define SINK_ID 293
# define NONE 294
# define RAND_CORRIDOR_ID 295
# define DOOR_STATE 296
# define LIGHT_STATE 297
# define CURSE_TYPE 298
# define ENGRAVING_TYPE 299
# define DIRECTION 300
# define RANDOM_TYPE 301
# define O_REGISTER 302
# define M_REGISTER 303
# define P_REGISTER 304
# define A_REGISTER 305
# define ALIGNMENT 306
# define LEFT_OR_RIGHT 307
# define CENTER 308
# define TOP_OR_BOT 309
# define ALTAR_TYPE 310
# define UP_OR_DOWN 311
# define SUBROOM_ID 312
# define NAME_ID 313
# define FLAGS_ID 314
# define FLAG_TYPE 315
# define MON_ATTITUDE 316
# define MON_ALERTNESS 317
# define MON_APPEARANCE 318
# define STRING 319
# define MAP_ID 320
