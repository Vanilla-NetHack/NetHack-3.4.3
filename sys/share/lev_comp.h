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
#define COBJECT_ID 267
#define MONSTER_ID 268
#define TRAP_ID 269
#define DOOR_ID 270
#define DRAWBRIDGE_ID 271
#define MAZEWALK_ID 272
#define WALLIFY_ID 273
#define REGION_ID 274
#define FILLING 275
#define RANDOM_OBJECTS_ID 276
#define RANDOM_MONSTERS_ID 277
#define RANDOM_PLACES_ID 278
#define ALTAR_ID 279
#define LADDER_ID 280
#define STAIR_ID 281
#define NON_DIGGABLE_ID 282
#define NON_PASSWALL_ID 283
#define ROOM_ID 284
#define PORTAL_ID 285
#define TELEPRT_ID 286
#define BRANCH_ID 287
#define LEV 288
#define CHANCE_ID 289
#define CORRIDOR_ID 290
#define GOLD_ID 291
#define ENGRAVING_ID 292
#define FOUNTAIN_ID 293
#define POOL_ID 294
#define SINK_ID 295
#define NONE 296
#define RAND_CORRIDOR_ID 297
#define DOOR_STATE 298
#define LIGHT_STATE 299
#define CURSE_TYPE 300
#define ENGRAVING_TYPE 301
#define DIRECTION 302
#define RANDOM_TYPE 303
#define O_REGISTER 304
#define M_REGISTER 305
#define P_REGISTER 306
#define A_REGISTER 307
#define ALIGNMENT 308
#define LEFT_OR_RIGHT 309
#define CENTER 310
#define TOP_OR_BOT 311
#define ALTAR_TYPE 312
#define UP_OR_DOWN 313
#define SUBROOM_ID 314
#define NAME_ID 315
#define FLAGS_ID 316
#define FLAG_TYPE 317
#define MON_ATTITUDE 318
#define MON_ALERTNESS 319
#define MON_APPEARANCE 320
#define CONTAINED 321
#define STRING 322
#define MAP_ID 323
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
