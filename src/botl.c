/*	SCCS Id: @(#)botl.c	3.1	93/01/17	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVL0
extern const char *hu_stat[];	/* defined in eat.c */

const char *enc_stat[] = {
	"",
	"Burdened",
	"Stressed",
	"Strained",
	"Overtaxed",
	"Overloaded"
};

static void NDECL(bot1);
static void NDECL(bot2);
#endif /* OVL0 */

/* 100 suffices for bot(); must be larger than COLNO */
#if COLNO <= 80
#define MAXCO 100
#else
#define MAXCO (COLNO+20)
#endif

#ifndef OVLB
STATIC_DCL int mrank_sz;
#else /* OVLB */
STATIC_OVL int NEARDATA mrank_sz = 0; /* loaded by max_rank_sz (from u_init) */
#endif /* OVLB */

struct rank_title {
    char const * const	m;	/* male title */
    char const * const	f;	/* female title, or 0 if same as male */
};
struct class_ranks {
    char		plclass, fill_;
    short		mplayer_class;
    struct rank_title	titles[9];
};

STATIC_DCL const struct rank_title *FDECL(rank_array, (CHAR_P));
STATIC_DCL const char *NDECL(rank);

#ifdef OVL1

/* 9 pairs of ranks for each class */

static const
struct class_ranks all_classes[] = {
  {					'A',0,	PM_ARCHEOLOGIST, {
	{"Digger",	0},
	{"Field Worker",0},
	{"Investigator",0},
	{"Exhumer",	0},
	{"Excavator",	0},
	{"Spelunker",	0},
	{"Speleologist",0},
	{"Collector",	0},
	{"Curator",	0}
  } },
  {					'B',0,	PM_BARBARIAN, {
	{"Plunderer",	"Plunderess"},
	{"Pillager",	0},
	{"Bandit",	0},
	{"Brigand",	0},
	{"Raider",	0},
	{"Reaver",	0},
	{"Slayer",	0},
	{"Chieftain",	"Chieftainess"},
	{"Conqueror",	"Conqueress"}
  } },
  {					'C',0,	PM_CAVEMAN, {
	{"Troglodyte",	0},
	{"Aborigine",	0},
	{"Wanderer",	0},
	{"Vagrant",	0},
	{"Wayfarer",	0},
	{"Roamer",	0},
	{"Nomad",	0},
	{"Rover",	0},
	{"Pioneer",	0}
  } },
  {					'E',0,	PM_ELF, {
	{"Edhel",	"Elleth"},
	{"Edhel",	"Elleth"},	/* elf-maid */
	{"Ohtar",	"Ohtie"},	/* warrior */
	{"Kano",			/* commander (Q.) ['a] */
			"Kanie"}, /* educated guess, until further research- SAC */
	{"Arandur",		  /* king's servant, minister (Q.) - guess */
			"Aranduriel"},	/* educated guess */
	{"Hir",		"Hiril"},	/* lord, lady (S.) ['ir] */
	{"Aredhel",	"Arwen"},	/* noble elf, maiden (S.) */
	{"Ernil",	"Elentariel"},	/* prince (S.), elf-maiden (Q.) */
	{"Elentar",	"Elentari"}	/* Star-king, -queen (Q.) */
  } },
  {					'H',0,	PM_HEALER, {
	{"Barber",	"Midwife"},
	{"Leech",	0},
	{"Embalmer",	0},
	{"Dresser",	0},
	{"Bone Setter",	0},
	{"Herbalist",	0},
	{"Apothecary",	0},
	{"Physician",	0},
	{"Chirurgeon",	0}
  } },
  {					'K',0,	PM_KNIGHT, {
	{"Gallant",	0},
	{"Esquire",	0},
	{"Bachelor",	0},
	{"Sergeant",	0},
	{"Knight",	0},
	{"Banneret",	0},
	{"Chevalier",	0},
	{"Seignieur",	0},
	{"Paladin",	0}
  } },
  {					'P',0,	PM_PRIEST, {
	{"Aspirant",	0},
	{"Acolyte",	0},
	{"Adept",	0},
	{"Priest",	"Priestess"},
	{"Curate",	0},
	{"Canon",	"Canoness"},
	{"Lama",	0},
	{"Patriarch",	"Matriarch"},
	{"High Priest", "High Priestess"}
  } },
  {					'R',0,	PM_ROGUE, {
	{"Footpad",	0},
	{"Cutpurse",	0},
	{"Rogue",	0},
	{"Pilferer",	0},
	{"Robber",	0},
	{"Burglar",	0},
	{"Filcher",	0},
	{"Magsman",	"Magswoman"},
	{"Thief",	0}
  } },
  {					'S',0,	PM_SAMURAI, {
	{"Hatamoto",	0},  /* Banner Knight */
	{"Ronin",	0},  /* no allegiance */
	{"Ninja",	0},  /* secret society */
	{"Joshu",	0},  /* heads a castle */
	{"Ryoshu",	0},  /* has a territory */
	{"Kokushu",	0},  /* heads a province */
	{"Daimyo",	0},  /* a samurai lord */
	{"Kuge",	0},  /* Noble of the Court */
	{"Shogun",	0}   /* supreme commander, warlord */
  } },
#ifdef TOURIST
  {					'T',0,	PM_TOURIST, {
	{"Rambler",	0},
	{"Sightseer",	0},
	{"Excursionist",0},
	{"Peregrinator","Peregrinatrix"},
	{"Traveler",	0},
	{"Journeyer",	0},
	{"Voyager",	0},
	{"Explorer",	0},
	{"Adventurer",	0}
  } },
#endif
  {					'V',0,	PM_VALKYRIE, {
	{"Stripling",	0},
	{"Skirmisher",	0},
	{"Fighter",	0},
	{"Man-at-arms", "Woman-at-arms"},
	{"Warrior",	0},
	{"Swashbuckler",0},
	{"Hero",	"Heroine"},
	{"Champion",	0},
	{"Lord",	"Lady"}
  } },
  {					'W',0,	PM_WIZARD, {
	{"Evoker",	0},
	{"Conjurer",	0},
	{"Thaumaturge", 0},
	{"Magician",	0},
	{"Enchanter",	"Enchantress"},
	{"Sorcerer",	"Sorceress"},
	{"Necromancer", 0},
	{"Wizard",	0},
	{"Mage",	0}
  } },
};

STATIC_OVL const struct rank_title *
rank_array(pc)
char pc;
{
	register int i;

	for (i = 0; i < SIZE(all_classes); i++)
	    if (all_classes[i].plclass == pc) return all_classes[i].titles;
	return 0;
}

/* convert experience level (1..30) to rank index (0..8) */
int xlev_to_rank(xlev)
int xlev;
{
	return (xlev <= 2) ? 0 : (xlev <= 30) ? ((xlev + 2) / 4) : 8;
}

#if 0	/* not currently needed */
/* convert rank index (0..8) to experience level (1..30) */
int rank_to_xlev(rank)
int rank;
{
	return (rank <= 0) ? 1 : (rank <= 8) ? ((rank * 4) - 2) : 30;
}
#endif

const char *
rank_of(lev, pc, female)
register unsigned lev;
char pc;
boolean female;
{
	register int idx = xlev_to_rank((int)lev);
	const struct rank_title *ranks = rank_array(pc);

	if (ranks)
	    return( female && ranks[idx].f ? ranks[idx].f : ranks[idx].m );
	return(pl_character);
}

STATIC_OVL const char *
rank()
{
	return(rank_of(u.ulevel, pl_character[0], flags.female));
}

int
title_to_mon(str, rank_indx, title_length)
const char *str;
int *rank_indx, *title_length;
{
	register int i, j;
	register const struct rank_title *ttl;

	for (i = 0; i < SIZE(all_classes); i++)
	    for (j = 0; j < 9; j++) {
		ttl = &all_classes[i].titles[j];
		if (!strncmpi(ttl->m, str, strlen(ttl->m))) {
		    if (rank_indx) *rank_indx = j;
		    if (title_length) *title_length = strlen(ttl->m);
		    return all_classes[i].mplayer_class;
		} else if (ttl->f && !strncmpi(ttl->f, str, strlen(ttl->f))) {
		    if (rank_indx) *rank_indx = j;
		    if (title_length) *title_length = strlen(ttl->f);
		    return all_classes[i].plclass == 'C' ? PM_CAVEWOMAN :
			   all_classes[i].plclass == 'P' ? PM_PRIESTESS :
			   all_classes[i].mplayer_class;
		}
	    }
	return -1;	/* not found */
}

#endif /* OVL1 */
#ifdef OVLB

void
max_rank_sz()
{
	register int i, r, maxr = 0;
	const struct rank_title *ranks = rank_array(pl_character[0]);

	if (ranks) {
	    for (i = 0; i < 9; i++) {
		if ((r = strlen(ranks[i].m)) > maxr) maxr = r;
		if (ranks[i].f)
		    if ((r = strlen(ranks[i].f)) > maxr) maxr = r;
	    }
	    mrank_sz = maxr;
	}
	else mrank_sz = strlen(pl_character);
}

#endif /* OVLB */
#ifdef OVL0

static void
bot1()
{
	char newbot1[MAXCO];
	register char *nb;
	register int i,j;

	Strcpy(newbot1, plname);
	if('a' <= newbot1[0] && newbot1[0] <= 'z') newbot1[0] += 'A'-'a';
	newbot1[10] = 0;
	Sprintf(nb = eos(newbot1)," the ");
#ifdef POLYSELF
	if (u.mtimedone) {
		char mbot[BUFSZ];
		int k = 0;

		Strcpy(mbot, mons[u.umonnum].mname);
		while(mbot[k] != 0) {
		    if ((k == 0 || (k > 0 && mbot[k-1] == ' ')) &&
					'a' <= mbot[k] && mbot[k] <= 'z')
			mbot[k] += 'A' - 'a';
		    k++;
		}
		Sprintf(nb = eos(nb), mbot);
	} else
		Sprintf(nb = eos(nb), rank());
#else
	Sprintf(nb = eos(nb), rank());
#endif
	Sprintf(nb = eos(nb),"  ");
	i = mrank_sz + 15;
	j = (nb + 2) - newbot1; /* aka strlen(newbot1) but less computation */
	if((i - j) > 0)
		Sprintf(nb = eos(nb),"%*s", i-j, " ");	/* pad with spaces */
	if (ACURR(A_STR) > 18) {
		if (ACURR(A_STR) > 118)
		    Sprintf(nb = eos(nb),"St:%2d ",ACURR(A_STR)-100);
		else if (ACURR(A_STR) < 118)
		    Sprintf(nb = eos(nb), "St:18/%02d ",ACURR(A_STR)-18);
		else
		    Sprintf(nb = eos(nb),"St:18/** ");
	} else
		Sprintf(nb = eos(nb), "St:%-1d ",ACURR(A_STR));
	Sprintf(nb = eos(nb),
		"Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
		ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
	Sprintf(nb = eos(nb), (u.ualign.type == A_CHAOTIC) ? "  Chaotic" :
			(u.ualign.type == A_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
	if (flags.showscore) {
	    int deepest = deepest_lev_reached(FALSE);
	    long ugold = u.ugold + hidden_gold();

	    if ((ugold -= u.ugold0) < 0L) ugold = 0L;
	    Sprintf(nb = eos(nb), " S:%ld",
		    ugold + u.urexp + (long)(50 * (deepest - 1))
			  + (long)(deepest > 30 ? 10000 :
				   deepest > 20 ? 1000*(deepest - 20) : 0));
	}
#endif
	curs(WIN_STATUS, 1, 0);
	putstr(WIN_STATUS, 0, newbot1);
}

static void
bot2()
{
	char  newbot2[MAXCO];
	register char *nb;
	int hp, hpmax;
	int cap = near_capacity();

#ifdef POLYSELF
	hp = u.mtimedone ? u.mh : u.uhp;
	hpmax = u.mtimedone ? u.mhmax : u.uhpmax;
#else
	hp = u.uhp;
	hpmax = u.uhpmax;
#endif
	if(hp < 0) hp = 0;
/* TODO:	Add in dungeon name */
#ifdef MULDGN
	if(Is_knox(&u.uz)) Sprintf(newbot2, "%s ", dungeons[u.uz.dnum].dname);
	else
	if(In_quest(&u.uz)) Sprintf(newbot2, "Home %d ", dunlev(&u.uz));
	else
#endif
	if(In_endgame(&u.uz))
		Sprintf(newbot2,
			Is_astralevel(&u.uz) ? "Astral Plane " : "End Game ");
	else
		Sprintf(newbot2, "Dlvl:%-2d ", depth(&u.uz));
	Sprintf(nb = eos(newbot2),
		"%c:%-2ld HP:%d(%d) Pw:%d(%d) AC:%-2d", oc_syms[GOLD_CLASS],
		u.ugold, hp, hpmax, u.uen, u.uenmax, u.uac);
#ifdef POLYSELF
	if (u.mtimedone)
		Sprintf(nb = eos(nb), " HD:%d", mons[u.umonnum].mlevel);
	else
#endif
#ifdef EXP_ON_BOTL
	if(flags.showexp)
		Sprintf(nb = eos(nb), " Xp:%u/%-1ld", u.ulevel,u.uexp);
	else
#endif
	Sprintf(nb = eos(nb), " Exp:%u", u.ulevel);
	if(flags.time)
	    Sprintf(nb = eos(nb), " T:%ld", moves);
	if(strcmp(hu_stat[u.uhs], "        ")) {
		Sprintf(nb = eos(nb), " ");
		Strcat(newbot2, hu_stat[u.uhs]);
	}
	if(Confusion)	   Sprintf(nb = eos(nb), " Conf");
	if(Sick)	   Sprintf(nb = eos(nb), " Sick");
	if(Blind)	   Sprintf(nb = eos(nb), " Blind");
	if(Stunned)	   Sprintf(nb = eos(nb), " Stun");
	if(Hallucination)  Sprintf(nb = eos(nb), " Hallu");
	if(cap > UNENCUMBERED)
		Sprintf(nb = eos(nb), " %s", enc_stat[cap]);
	curs(WIN_STATUS, 1, 1);
	putstr(WIN_STATUS, 0, newbot2);
}

void
bot()
{
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

#endif /* OVL0 */

/*botl.c*/
