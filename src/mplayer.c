/*	SCCS Id: @(#)mplayer.c	3.2	96/02/27	*/
/*	Copyright (c) Izchak Miller, 1992.			  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static const char *NDECL(dev_name);
static void FDECL(get_mplname, (struct monst *, char *));
static void FDECL(mk_mplayer_armor, (struct monst *, int, int));

/* These are the names of those who
 * contributed to the development of NetHack 3.2.
 *
 * Keep in alphabetical order within teams.
 * Same first name is entered once within each team.
 */
static const char *developers[] = {
	/* devteam */
	"Dave", "Dean", "Eric", "Izchak", "Janet", "Jessie",
	"Ken", "Kevin", "Michael", "Mike", "Pat", "Paul", "Steve", "Timo",
	/* PC team */
	"Bill", "Eric", "Ken", "Kevin", "Michael", "Mike", "Paul",
	"Stephen", "Steve", "Timo", "Yamamoto", "Yitzhak",
	/* Amiga team */
	"Andy", "Gregg", "Keni", "Mike", "Olaf", "Richard",
	/* Mac team */
	"Andy", "Chris", "Dean", "Jon", "Jonathan", "Wang",
	/* Atari team */
	"Eric", "Warwick",
	/* NT team */
	"Michael",
	/* OS/2 team */
	"Timo",
	/* VMS team */
	"Joshua", "Pat",
	""};


/* return a randomly chosen developer name */
static const char *
dev_name()
{
	register int i, m = 0, n = SIZE(developers);
	register struct monst *mtmp;
	register boolean match;

	do {
	    match = FALSE;
	    i = rn2(n);
	    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if(!is_mplayer(mtmp->data)) continue;
		if(!strncmp(developers[i], NAME(mtmp),
			               strlen(developers[i]))) {
		    match = TRUE;
		    break;
	        }
	    }
	    m++;
	} while (match && m < 100); /* m for insurance */

	if (match) return (const char *)0;
	return(developers[i]);
}

static void
get_mplname(mtmp, nam)
register struct monst *mtmp;
char *nam;
{
	boolean fmlkind = is_female(mtmp->data);
	const char *devnam;

	devnam = dev_name();
	if (!devnam)
	    Strcpy(nam, fmlkind ? "Eve" : "Adam");
	else if (fmlkind && !!strcmp(devnam, "Janet"))
	    Strcpy(nam, rn2(2) ? "Maud" : "Eve");
	else Strcpy(nam, devnam);

	if (fmlkind || !strcmp(nam, "Janet"))
	    mtmp->female = 1;
	else
	    mtmp->female = 0;
	Strcat(nam, " the ");
	Strcat(nam, rank_of(rn1(11, 20),
			    highc(mtmp->data->mname[0]),
			    (boolean)mtmp->female));
}

static void
mk_mplayer_armor(mon, range1, range2)
struct monst *mon;
int range1, range2;
{
	struct obj *obj;

	obj = mksobj(rnd_class(range1, range2), FALSE, FALSE);
	if (!rn2(3)) obj->oerodeproof = 1;
	if (!rn2(3)) curse(obj);
	if (!rn2(3)) bless(obj);
	/* Most players who get to the endgame who have cursed equipment
	 * have it because the wizard or other monsters cursed it, so its
	 * chances of having plusses is the same as usual....
	 */
	obj->spe = rn2(10) ? (rn2(3) ? rn2(5) : rn1(4,4)) : -rnd(3);
	mpickobj(mon, obj);
}

struct monst *
mk_mplayer(ptr, x, y, special)
register struct permonst *ptr;
xchar x, y;
register boolean special;
{
	register struct monst *mtmp;
	char nam[PL_NSIZ];

	if(!is_mplayer(ptr))
		return((struct monst *)0);

	if(MON_AT(x, y))
		rloc(m_at(x, y)); /* insurance */

	if(!In_endgame(&u.uz)) special = FALSE;

	if ((mtmp = makemon(ptr, x, y)) != 0) {
	    int weapon, quan;
	    struct obj *otmp;

	    mtmp->m_lev = (special ? rn1(8,12) : rnd(16));
	    mtmp->mhp = mtmp->mhpmax = d((int)mtmp->m_lev,10) +
					(special ? (30 + rnd(30)) : 30);
	    if(special) {
	        get_mplname(mtmp, nam);
	        mtmp = christen_monst(mtmp, nam);
		/* that's why they are "stuck" in the endgame :-) */
		(void)mongets(mtmp, FAKE_AMULET_OF_YENDOR);
	    }
	    mtmp->mpeaceful = 0;
	    set_malign(mtmp); /* peaceful may have changed again */

	    switch(monsndx(ptr)) {
		case PM_ARCHEOLOGIST:
		    weapon = BULLWHIP;
		    break;
		case PM_BARBARIAN:
		    weapon = rn2(2) ? TWO_HANDED_SWORD : BATTLE_AXE;
		    break;
		case PM_CAVEMAN:
		case PM_CAVEWOMAN:
		    weapon = CLUB;
		    break;
		case PM_ELF:
		    weapon = ELVEN_SHORT_SWORD;
		    break;
		case PM_HEALER:
		    weapon = SCALPEL;
		    break;
		case PM_KNIGHT:
		    weapon = LONG_SWORD;
		    break;
		case PM_PRIEST:
		case PM_PRIESTESS:
		    weapon = MACE;
		    break;
		case PM_ROGUE:
		    weapon = SHORT_SWORD;
		    break;
		case PM_SAMURAI:
		    weapon = KATANA;
		    break;
#ifdef TOURIST
		case PM_TOURIST:
		    weapon = 0;
		    break;
#endif
		case PM_VALKYRIE:
		    weapon = LONG_SWORD;
		    break;
		case PM_WIZARD:
		    weapon = ATHAME;
		    break;
		default: impossible("bad mplayer monster");
		    weapon = 0;
		    break;
	    }
	    if (rn2(2) && weapon)
		otmp = mksobj(weapon, TRUE, FALSE);
	    else
		otmp = mksobj(rn2(2) ? LONG_SWORD :
			      rnd_class(SPEAR, BULLWHIP), TRUE, FALSE);
	    otmp->spe = (special ? rn1(5,4) : rn2(4));
	    if (!rn2(3)) otmp->oerodeproof = 1;
	    else if (!rn2(2)) otmp->greased = 1;
	    if (special && rn2(2))
		otmp = mk_artifact(otmp, A_NONE);
	    mpickobj(mtmp, otmp);

	    if(special) {
	        if (!rn2(10))
		    (void) mongets(mtmp, rn2(3) ? LUCKSTONE : LOADSTONE);
	        if (rn2(8))
	        mk_mplayer_armor(mtmp, ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY);
	        if (!rn2(3))
		    mk_mplayer_armor(mtmp, GRAY_DRAGON_SCALE_MAIL,
			YELLOW_DRAGON_SCALE_MAIL);
	        else if (rn2(15))
		    mk_mplayer_armor(mtmp, PLATE_MAIL, CHAIN_MAIL);
	        if (rn2(8))
		    mk_mplayer_armor(mtmp, ELVEN_SHIELD,
				               SHIELD_OF_REFLECTION);
	        if (rn2(8))
		    mk_mplayer_armor(mtmp, LEATHER_GLOVES,
				               GAUNTLETS_OF_DEXTERITY);
	        if (rn2(8))
		    mk_mplayer_armor(mtmp, LOW_BOOTS, LEVITATION_BOOTS);
	        m_dowear(mtmp, TRUE);

	        quan = rn2(3) ? rn2(3) : rn2(16);
	        while(quan--)
		    (void)mongets(mtmp, rnd_class(DILITHIUM_CRYSTAL, JADE));
	        /* To get the gold "right" would mean a player can double his */
	        /* gold supply by killing one mplayer.  Not good. */
	        mtmp->mgold = rn2(1000);
	        quan = rn2(10);
	        while(quan--)
		    mpickobj(mtmp, mkobj(RANDOM_CLASS, FALSE));
	    }
	    quan = rnd(3);
	    while(quan--)
		(void)mongets(mtmp, rnd_offensive_item(mtmp));
	    quan = rnd(3);
	    while(quan--)
		(void)mongets(mtmp, rnd_defensive_item(mtmp));
	    quan = rnd(3);
	    while(quan--)
		(void)mongets(mtmp, rnd_misc_item(mtmp));
	}

	return(mtmp);
}

/* create the indicated number (num) of monster-players,
 * randomly chosen, and in randomly chosen (free) locations
 * on the level.  If "special", the size of num should not
 * be bigger than the number of _non-repeated_ names in the
 * developers array, otherwise a bunch of Adams and Eves will
 * fill up the overflow.
 */
void
create_mplayers(num, special)
register int num;
boolean special;
{
	register int pm, x, y;

	while(num) {
		int tryct = 0;

		/* roll for character class */
		pm = PM_ARCHEOLOGIST + rn2(PM_WIZARD - PM_ARCHEOLOGIST + 1);

		/* roll for an available location */
		do {
		    x = rn1(COLNO-4, 2);
		    y = rnd(ROWNO-2);
		} while(!goodpos(x, y, (struct monst *)0, &mons[pm]) ||
			 tryct++ >= 50);

		/* if pos not found in 50 tries, don't bother to continue */
		if(tryct > 50) return;

		(void) mk_mplayer(&mons[pm], (xchar)x, (xchar)y, special);
		num--;
	}
}

void
mplayer_talk(mtmp)
register struct monst *mtmp;
{
	static const char *same_class_msg[3] = {
		"I can't win, and neither will you!",
		"You don't deserve to win!",
		"Mine should be the honor, not yours!",
	},		  *other_class_msg[3] = {
		"The low-life wants to talk, eh?",
		"Fight, scum!",
		"Here is what I have to say!",
	};

	if(mtmp->mpeaceful) return; /* will drop to humanoid talk */

	pline("Talk? -- %s", u.role == highc(*mtmp->data->mname) ?
		same_class_msg[rn2(3)] : other_class_msg[rn2(3)]);
}

/*mplayer.c*/
