/*	SCCS Id: @(#)shknam.c	2.3	87/12/18
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

/* shknam.c -- initialize a shop */

#include "hack.h"
#include "mkroom.h"
#include "eshk.h"

extern struct monst *makemon();
extern struct obj *mkobj_at(), *mksobj_at();

static char *shkliquors[] = {
    /* Ukraine */
    "Njezjin", "Tsjernigof", "Gomel", "Ossipewsk", "Gorlowka",
    /* N. Russia */
    "Konosja", "Weliki Oestjoeg", "Syktywkar", "Sablja",
    "Narodnaja", "Kyzyl",
    /* Silezie */
    "Walbrzych", "Swidnica", "Klodzko", "Raciborz", "Gliwice",
    "Brzeg", "Krnov", "Hradec Kralove",
    /* Schweiz */
    "Leuk", "Brig", "Brienz", "Thun", "Sarnen", "Burglen", "Elm",
    "Flims", "Vals", "Schuls", "Zum Loch",
    ""
};

static char *shkbooks[] = {
    /* Eire */
    "Skibbereen", "Kanturk", "Rath Luirc", "Ennistymon", "Lahinch",
    "Kinnegad", "Lugnaquillia", "Enniscorthy", "Gweebarra",
    "Kittamagh", "Nenagh", "Sneem", "Ballingeary", "Kilgarvan",
    "Cahersiveen", "Glenbeigh", "Kilmihil", "Kiltamagh",
    "Droichead Atha", "Inniscrone", "Clonegal", "Lisnaskea",
    "Culdaff", "Dunfanaghy", "Inishbofin", "Kesh",
    ""
};

static char *shkarmors[] = {
    /* Turquie */
    "Demirci", "Kalecik", "Boyabai", "Yildizeli", "Gaziantep",
    "Siirt", "Akhalataki", "Tirebolu", "Aksaray", "Ermenak",
    "Iskenderun", "Kadirli", "Siverek", "Pervari", "Malasgirt",
    "Bayburt", "Ayancik", "Zonguldak", "Balya", "Tefenni",
    "Artvin", "Kars", "Makharadze", "Malazgirt", "Midyat",
    "Birecik", "Kirikkale", "Alaca", "Polatli", "Nallihan",
    ""
};

static char *shkwands[] = {
    /* Wales */
    "Yr Wyddgrug", "Trallwng", "Mallwyd", "Pontarfynach",
    "Rhaeader", "Llandrindod", "Llanfair-ym-muallt",
    "Y-Fenni", "Measteg", "Rhydaman", "Beddgelert",
    "Curig", "Llanrwst", "Llanerchymedd", "Caergybi",
    /* Scotland */
    "Nairn", "Turriff", "Inverurie", "Braemar", "Lochnagar",
    "Kerloch", "Beinn a Ghlo", "Drumnadrochit", "Morven",
    "Uist", "Storr", "Sgurr na Ciche", "Cannich", "Gairloch",
    "Kyleakin", "Dunvegan",
    ""
};

static char *shkrings[] = {
    /* Hollandse familienamen */
    "Feyfer", "Flugi", "Gheel", "Havic", "Haynin", "Hoboken",
    "Imbyze", "Juyn", "Kinsky", "Massis", "Matray", "Moy",
    "Olycan", "Sadelin", "Svaving", "Tapper", "Terwen", "Wirix",
    "Ypey",
    /* Skandinaviske navne */
    "Rastegaisa", "Varjag Njarga", "Kautekeino", "Abisko",
    "Enontekis", "Rovaniemi", "Avasaksa", "Haparanda",
    "Lulea", "Gellivare", "Oeloe", "Kajaani", "Fauske",
    ""
};

static char *shkfoods[] = {
    /* Indonesia */
    "Djasinga", "Tjibarusa", "Tjiwidej", "Pengalengan",
    "Bandjar", "Parbalingga", "Bojolali", "Sarangan",
    "Ngebel", "Djombang", "Ardjawinangun", "Berbek",
    "Papar", "Baliga", "Tjisolok", "Siboga", "Banjoewangi",
    "Trenggalek", "Karangkobar", "Njalindoeng", "Pasawahan",
    "Pameunpeuk", "Patjitan", "Kediri", "Pemboeang", "Tringanoe",
    "Makin", "Tipor", "Semai", "Berhala", "Tegal", "Samoe",
    ""
};

static char *shkweapons[] = {
    /* Perigord */
    "Voulgezac", "Rouffiac", "Lerignac", "Touverac", "Guizengeard",
    "Melac", "Neuvicq", "Vanzac", "Picq", "Urignac", "Corignac",
    "Fleac", "Lonzac", "Vergt", "Queyssac", "Liorac", "Echourgnac",
    "Cazelon", "Eypau", "Carignan", "Monbazillac", "Jonzac",
    "Pons", "Jumilhac", "Fenouilledes", "Laguiolet", "Saujon",
    "Eymoutiers", "Eygurande", "Eauze", "Labouheyre",
    ""
};

static char *shkgeneral[] = {
    /* Suriname */
    "Hebiwerie", "Possogroenoe", "Asidonhopo", "Manlobbi",
    "Adjama", "Pakka Pakka", "Kabalebo", "Wonotobo",
    "Akalapi", "Sipaliwini",
    /* Greenland */
    "Annootok", "Upernavik", "Angmagssalik",
    /* N. Canada */
    "Aklavik", "Inuvik", "Tuktoyaktuk",
    "Chicoutimi", "Ouiatchouane", "Chibougamau",
    "Matagami", "Kipawa", "Kinojevis",
    "Abitibi", "Maganasipi",
    /* Iceland */
    "Akureyri", "Kopasker", "Budereyri", "Akranes", "Bordeyri",
    "Holmavik",
    ""
};

/*
 * To add new shop types, all that is necessary is to edit the shtypes[] array.
 * See mkroom.h for the structure definition. Typically, you'll have to lower
 * some or all of the probability fields in old entries to free up some
 * percentage for the new type.
 *
 * The placement type field is not yet used but will be in the near future.
 *
 * The iprobs array in each entry defines the probabilities for various kinds
 * of artifacts to be present in the given shop type. You can associate with
 * each percentage either a generic artifact type (represented by one of the
 * *_SYM macros) or a specific artifact (represented by an onames.h define).
 * In the latter case, prepend it with a unary minus so the code can know
 * (by testing the sign) whether to use mkobj() or mksobj().
 */
struct shclass shtypes[] = {
	{"general store", RANDOM_SYM,
#ifdef SPELLS
	    47,
#else
	    50,
#endif
	    D_SHOP, {{100, RANDOM_SYM}, {0, 0}, {0, 0}}, shkgeneral},
	{"used armor dealership", ARMOR_SYM, 14,
	    D_SHOP, {{90, ARMOR_SYM}, {10, WEAPON_SYM}, {0, 0}}, shkarmors},
	{"second hand bookstore", SCROLL_SYM, 10, D_SHOP,
#ifdef SPELLS
	    {{90, SCROLL_SYM}, {10, SPBOOK_SYM}, {0, 0}},
#else
	    {{100, SCROLL_SYM}, {0, 0}, {0, 0}},
#endif
	    shkbooks},
	{"liquor emporium", POTION_SYM, 10, D_SHOP,
	    {{100, POTION_SYM}, {0, 0}, {0, 0}}, shkliquors},
	{"antique weapons outlet", WEAPON_SYM, 5, D_SHOP,
	    {{90, WEAPON_SYM}, {10, ARMOR_SYM}, {0, 0}}, shkweapons},
	{"delicatessen", FOOD_SYM, 5, D_SHOP,
	    {{95, FOOD_SYM}, {5, POTION_SYM}, {0, 0}}, shkfoods},
	{"jewelers", RING_SYM, 3, D_SHOP,
	    {{90, RING_SYM}, {10, GEM_SYM}, {0, 0}}, shkrings},
	{"quality apparel and accessories", WAND_SYM, 3, D_SHOP,
	    {{90, WAND_SYM}, {5, -PAIR_OF_GLOVES}, {5, -ELVEN_CLOAK}, {0, 0}},
	     shkwands},
#ifdef SPELLS
	{"rare books", SPBOOK_SYM, 3, D_SHOP,
	    {{90, SPBOOK_SYM}, {10, SCROLL_SYM}, {0, 0}}, shkbooks},
#endif
	{(char *)0, 0, 0, 0, {{0, 0}, {0, 0}, {0, 0}}, (char **)0}
};

static void
mkshobj_at(shp, sx, sy)
/* make an object of the appropriate type for a shop square */
struct shclass *shp;
int sx, sy;
{
    register int	i, j;
    register struct monst *mtmp;
    int		atype;

    /* select an appropriate artifact type at random */
    for(j = rnd(100), i = 0; j -= shp->iprobs[i].iprob; i++)
	if (j < 0)
	    break;

    /* generate the appropriate object */
    if ((atype = shp->iprobs[i].itype) >= 0)	/* if a class was given */
    {
	/* the artifact may actually be a mimic */
	if(rn2(100) < dlevel && !m_at(sx,sy) && (mtmp=makemon(PM_MIMIC,sx,sy)))
	{
	    mtmp->mimic = 1;
	    mtmp->mappearance =	(atype && rn2(10) < dlevel) ? atype : ']';
	    return;
	}

	/* it's not, go ahead and generate an article of the class */
	(void) mkobj_at(atype, sx, sy);
    }
    else	/* particular object was to be generated */
	(void) mksobj_at(-atype, sx, sy);
}

void
findname(nampt, nlp)
/* extract a shopkeeper name for the given shop type */
char *nampt;
char *nlp[];
{
    register int i;

    for(i = 0; i < dlevel; i++)
	if (strlen(nlp[i]) == 0)
	{
	    /* Not enough names, try general name */
	    if (nlp != shkgeneral)
		findname(nampt, shkgeneral);
	    else
		(void) strcpy(nampt, "Dirk");
	    return;
	}
    (void) strncpy(nampt, nlp[i], PL_NSIZ);
    nampt[PL_NSIZ-1] = 0;
}

static int
shkinit(shp, sroom)
/* create a new shopkeeper in the given room */
struct shclass	*shp;
struct mkroom	*sroom;
{
    register int sh, sx, sy;
    struct monst *shk;

    /* place the shopkeeper in the given room */
    sh = sroom->fdoor;
    sx = doors[sh].x;
    sy = doors[sh].y;

    /* check that the shopkeeper placement is sane */
    if(sx == sroom->lx-1) sx++; else
	if(sx == sroom->hx+1) sx--; else
	    if(sy == sroom->ly-1) sy++; else
		if(sy == sroom->hy+1) sy--; else {
#ifdef WIZARD
		    /* Said to happen sometimes, but I've never seen it. */
		    if(wizard) {
			register int j = sroom->doorct;
			extern int doorindex;

			pline("Where is shopdoor?");
			pline("Room at (%d,%d),(%d,%d).", sroom->lx, sroom->ly,
			      sroom->hx, sroom->hy);
			pline("doormax=%d doorct=%d fdoor=%d",
			doorindex, sroom->doorct, sh);
			while(j--) {
			    pline("door [%d,%d]", doors[sh].x, doors[sh].y);
			    sh++;
			}
			more();
		    }
#endif
		    return(-1);
		}

    /* now initialize the shopkeeper's monster structure */
#define	ESHK	((struct eshk *)(&(shk->mextra[0])))
    if(!(shk = makemon(PM_SHK,sx,sy)))
	return(-1);
    shk->isshk = shk->mpeaceful = 1;
    shk->msleep = 0;
    shk->mtrapseen = ~0;	/* we know all the traps already */
    ESHK->shoproom = sroom - rooms;
    ESHK->shoplevel = dlevel;
    ESHK->shd = doors[sh];
    ESHK->shk.x = sx;
    ESHK->shk.y = sy;
    ESHK->robbed = 0;
    ESHK->visitct = 0;
    ESHK->following = 0;
    shk->mgold = 1000 + 30*rnd(100);	/* initial capital */
    ESHK->billct = 0;
    findname(ESHK->shknam, shp->shknms);

    return(sh);
}

void
stock_room(shp, sroom)
/* stock a newly-created room with artifacts */
struct shclass	*shp;
register struct mkroom *sroom;
{
    /*
     * Someday soon we'll dispatch on the dist field of shclass to do
     * different placements in this routine. Currently it only supports
     * shop-style placement (all squares except a row nearest the first
     * door get artifacts).
     */
    register int sx, sy, sh;

    /* first, try to place a shopkeeper in the room */
    if ((sh = shkinit(shp, sroom)) < 0)
	return;

    for(sx = sroom->lx; sx <= sroom->hx; sx++)
	for(sy = sroom->ly; sy <= sroom->hy; sy++){
	    if((sx == sroom->lx && doors[sh].x == sx-1) ||
	       (sx == sroom->hx && doors[sh].x == sx+1) ||
	       (sy == sroom->ly && doors[sh].y == sy-1) ||
	       (sy == sroom->hy && doors[sh].y == sy+1)) continue;
	    mkshobj_at(shp, sx, sy);
	}

    /*
     * Special monster placements (if any) should go here: that way,
     * monsters will sit on top of artifacts and not the other way around.
     */
}

saleable(nshop, obj)			/* does "shop" stock this item type */
	register int	nshop;
	register struct	obj *obj;
{
	int i;

	if(shtypes[nshop].symb == RANDOM_SYM) return(1);
	else {
	    for(i = 0; shtypes[nshop].iprobs[i].iprob; i++)
		if(shtypes[nshop].iprobs[i].itype < 0) {
		   if(shtypes[nshop].iprobs[i].itype == - obj->otyp) return(1);
		}
	        else if(shtypes[nshop].iprobs[i].itype == obj->olet) return(1);
	}
	return(0);
}
