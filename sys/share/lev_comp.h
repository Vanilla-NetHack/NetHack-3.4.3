#define CHAR 257
#define INTEGER 258
#define BOOLEAN 259
#define MESSAGE_ID 260
#define MAZE_ID 261
#define LEVEL_ID 262
#define LEV_INIT_ID 263
#define GEOMETRY_ID 264
#define NOMAP_ID 265
#define OBJECT_ID 266
#define MONSTER_ID 267
#define TRAP_ID 268
#define DOOR_ID 269
#define DRAWBRIDGE_ID 270
#define MAZEWALK_ID 271
#define WALLIFY_ID 272
#define REGION_ID 273
#define FILLING 274
#define RANDOM_OBJECTS_ID 275
#define RANDOM_MONSTERS_ID 276
#define RANDOM_PLACES_ID 277
#define ALTAR_ID 278
#define LADDER_ID 279
#define STAIR_ID 280
#define NON_DIGGABLE_ID 281
#define NON_PASSWALL_ID 282
#define ROOM_ID 283
#define PORTAL_ID 284
#define TELEPRT_ID 285
#define BRANCH_ID 286
#define LEV 287
#define CHANCE_ID 288
#define CORRIDOR_ID 289
#define GOLD_ID 290
#define ENGRAVING_ID 291
#define FOUNTAIN_ID 292
#define POOL_ID 293
#define SINK_ID 294
#define NONE 295
#define RAND_CORRIDOR_ID 296
#define DOOR_STATE 297
#define LIGHT_STATE 298
#define CURSE_TYPE 299
#define ENGRAVING_TYPE 300
#define DIRECTION 301
#define RANDOM_TYPE 302
#define O_REGISTER 303
#define M_REGISTER 304
#define P_REGISTER 305
#define A_REGISTER 306
#define ALIGNMENT 307
#define LEFT_OR_RIGHT 308
#define CENTER 309
#define TOP_OR_BOT 310
#define ALTAR_TYPE 311
#define UP_OR_DOWN 312
#define SUBROOM_ID 313
#define NAME_ID 314
#define FLAGS_ID 315
#define FLAG_TYPE 316
#define MON_ATTITUDE 317
#define MON_ALERTNESS 318
#define MON_APPEARANCE 319
#define STRING 320
#define MAP_ID 321
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
