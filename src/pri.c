/*	SCCS Id: @(#)pri.c	3.0	89/11/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define MONATTK_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
#include "hack.h"
#include <ctype.h>  /* for isalpha() */
#if defined(ALTARS) && defined(THEOLOGY)
#include "epri.h"
#endif
#include "termcap.h"

OSTATIC void FDECL(hilite, (int,int,UCHAR_P, UCHAR_P));
OSTATIC void FDECL(cornbot, (int));
#ifdef TEXTCOLOR
OSTATIC uchar FDECL(mimic_color, (struct monst *));
#endif

#ifndef ASCIIGRAPH
# define g_putch  (void) putchar
#endif

#ifndef g_putch
#ifdef OVL0
static boolean GFlag = FALSE; /* graphic flag */
#endif /* OVL0 */
#endif

/* 100 suffices for bot(); must be larger than COLNO */
#define MAXCO 100
VSTATIC char oldbot1[MAXCO], newbot1[MAXCO];
VSTATIC char oldbot2[MAXCO], newbot2[MAXCO];
#ifdef OVL2
static const char *dispst = "*0#@#0#*0#@#0#*0#@#0#*0#@#0#*0#@#0#*";
#endif /* OVL2 */
#ifndef OVLB
OSTATIC int mrank_sz;
#else /* OVLB */
XSTATIC int mrank_sz = 0;  /* loaded by max_rank_sz (called in u_init) */
#endif /* OVLB */

#ifdef CLIPPING
#define curs(x, y) (void) win_curs((x), (y)-2)
#endif

#ifdef OVL0

char *
eos(s)
register char *s;
{
	while(*s) s++;
	return(s);
}

#endif /* OVL0 */
#ifdef OVLB

void
swallowed(first)
register int first;
{
	if(first) cls();
	else {
		curs(u.ustuck->mdx-1, u.ustuck->mdy+1);
#ifdef MACOS
		puts("   ");
#else
		(void) fputs("   ", stdout);
#endif
		curx = u.ustuck->mdx+2;
		curs(u.ustuck->mdx-1, u.ustuck->mdy+2);
#ifdef MACOS
		puts("   ");
#else
		(void) fputs("   ", stdout);
#endif
		curx = u.ustuck->mdx+2;
		curs(u.ustuck->mdx-1, u.ustuck->mdy+3);
#ifdef MACOS
		puts("   ");
#else
		(void) fputs("   ", stdout);
#endif
		curx = u.ustuck->mdx+2;
	}
	curs(u.ux-1, u.uy+1);
#ifdef MACOS
	puts("/-\\");
#else
	(void) fputs("/-\\", stdout);
#endif
	curx = u.ux+2;
	curs(u.ux-1, u.uy+2);
	(void) putchar('|');
	hilite(u.ux, u.uy, u.usym, AT_MON);
	(void) putchar('|');
	curx = u.ux+2;
	curs(u.ux-1, u.uy+3);
#ifdef MACOS
	puts("\\-/");
#else
	(void) fputs("\\-/", stdout);
#endif
	curx = u.ux+2;
	u.udispl = 1;
	u.udisx = u.ux;
	u.udisy = u.uy;
}
#ifdef CLIPPING
#undef curs
#endif

void
setclipped()
{
#ifndef CLIPPING
	error("NetHack needs a screen of size at least %d by %d.\n",
		ROWNO+3, COLNO);
#else
	clipping = TRUE;
	clipx = clipy = 0;
	clipxmax = CO;
	clipymax = LI - 3;
#endif
}

#ifdef CLIPPING
void
cliparound(x, y)
int x, y;
{
	int oldx = clipx, oldy = clipy;

	if (!clipping) return;
	if (x < clipx + 5) {
		clipx = max(0, x - 20);
		clipxmax = clipx + CO;
	}
	else if (x > clipxmax - 5) {
		clipxmax = min(COLNO, clipxmax + 20);
		clipx = clipxmax - CO;
	}
	if (y < clipy + 2) {
		clipy = max(0, y - 10);
		clipymax = clipy + (LI - 3);
	}
	else if (y > clipymax - 2) {
		clipymax = min(ROWNO, clipymax + 10);
		clipy = clipxmax - (LI - 3);
	}
	if (clipx != oldx || clipy != oldy) {
		if (u.udispl) {
			u.udispl = 0;
			levl[u.udisx][u.udisy].scrsym = news0(u.udisx, u.udisy);
		}
		(void) doredraw();
	}
}
#endif /* CLIPPING */

#endif /* OVLB */
#ifdef OVL0

/*
 *  Allow for a different implementation than this...
 */

#ifndef g_putch

static void
g_putch(ch)
uchar ch;
{
	if (IBMgraphics)    /* IBM-compatible displays don't need other stuff */
		(void) putchar(ch);
	else if (ch & 0x80) {
		if (!GFlag) {
			graph_on();
			GFlag = TRUE;
		}
		(void) putchar(ch ^ 0x80); /* Strip 8th bit */
	} else {
		if (GFlag) {
			graph_off();
			GFlag = FALSE;
		}
		(void) putchar(ch);
	}
}

#endif

boolean
showmon(mon)
register struct monst *mon;
{
	register boolean show = (Blind && Telepat) || canseemon(mon);

	if (!show && (HTelepat & WORN_HELMET))
		show = (dist(mon->mx, mon->my) <= (BOLT_LIM * BOLT_LIM));
	return(show);
}

void
at(x,y,ch,typ)
register xchar x,y;
uchar ch,typ;
{
#ifndef LINT
	/* if xchar is unsigned, lint will complain about  if(x < 0)  */
	if(x < 0 || x > COLNO-1 || y < 0 || y > ROWNO-1) {
		impossible("At gets 0%o at %d %d.", ch, x, y);
		return;
	}
#endif
	if(!ch) {
		impossible("At gets null at %d %d.", x, y);
		return;
	}

	if (typ == AT_APP
#if !defined(MSDOS) && !defined(MACOS)
	    && flags.standout
#endif
	   )
		/* don't hilite if this isn't a monster or object.
		 *
		 * not complete; a scroll dropped by some monster
		 * on an unseen doorway which is later magic mapped
		 * will still hilite the doorway symbol.  -3.
		 */
		if (!vism_at(x,y) &&
		    (!OBJ_AT(x, y) && !levl[x][y].gmask || is_pool(x,y)))
		    typ = AT_MAP;

#ifdef CLIPPING
	if (win_curs(x, y)) {
#else
	curs(x,y+2);
#endif
	hilite(x, y, ch, typ);
	curx++;
#ifdef CLIPPING
	}
#endif
}

#endif /* OVL0 */
#ifdef OVLB

void
prme(){
	if(!Invisible
#ifdef POLYSELF
			&& !u.uundetected
#endif
					) {
		levl[u.ux][u.uy].seen = 0; /* force atl */
		atl(u.ux,u.uy,(char)u.usym);
	}
}

#endif /* OVLB */
#ifdef OVL2

void
shieldeff(x, y)		/* produce a magical shield effect at x,y */
	register xchar x, y;
{
	register const char *ch;
	register struct monst *mtmp = 0;

	if((x != u.ux) || (y != u.uy)) {
	    if(!(mtmp = m_at(x, y))) {

		impossible("shield effect at %d,%d", x, y);
		return;
	    }
	    if(!showmon(mtmp)) return;
	}

	for(ch = dispst; *ch; ch++)  {
		at(x, y, (uchar) *ch, AT_ZAP);
		(void) fflush(stdout);
		delay_output();
		delay_output();
	}

	nomul(0);
	if(!mtmp) {
		if(Invisible) {
			prl(x, y);
			at(x, y, levl[x][y].scrsym, AT_APP);
		} else prme();
	} else {
		mtmp->mdispl = 0;	/* make sure it gets redrawn */
		prl(x, y);
		if(mtmp->minvis)
			at(x, y, levl[x][y].scrsym, AT_APP);
		else	at(x, y, (uchar) mtmp->data->mlet, AT_MON);
	}

	return;
}

#endif /* OVL2 */
#ifdef OVLB

int
doredraw()
{
	docrt();
	return 0;
}

#endif /* OVLB */
#ifdef OVL0

void
docrt()
{
	register int x,y;
	register struct rm *room;
	register struct monst *mtmp;
#ifdef MACOS
	term_info	*t;
	extern WindowPtr HackWindow;
#endif

	if(u.uswallow) {
		swallowed(1);
		return;
	}
	cls();

/* Some ridiculous code to get display of @ and monsters (almost) right */
	if(!Invisible
#ifdef POLYSELF
			&& !u.uundetected
#endif
					) {
		u.udisx = u.ux;
		u.udisy = u.uy;
		levl[u.udisx][u.udisy].scrsym = u.usym;
		levl[u.udisx][u.udisy].seen = 1;
		u.udispl = 1;
	} else	u.udispl = 0;

	seemons();	/* reset old positions */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		mtmp->mdispl = 0;
	seemons();	/* force new positions to be shown */

#if ((defined(DGK) && !defined(TEXTCOLOR)) || defined(MACOS)) && !defined(CLIPPING)
# ifdef MACOS
	t = (term_info *)GetWRefCon(HackWindow);
	if (!t->inColor)
# endif
	/* Otherwise, line buffer the output to do the redraw in
	 * about 2/3 of the time.
	 */
		for(y = 0; y < ROWNO; y++) {
			char buf[COLNO+1];
			int start, end;
# if defined(LSC) || defined(AZTEC) || defined(AZTEC_C)
			setmem(buf, COLNO, ' ');
# else
			memset(buf, ' ', COLNO);
# endif
			for(x = 0, start = -1, end = -1; x < COLNO; x++)
				if((room = &levl[x][y])->new) {
					room->new = 0;
					buf[x] = room->scrsym;
					if (start < 0)
						start = x;
					end = x;
				} else if(room->seen) {
					buf[x] = room->scrsym;
					if (start < 0)
						start = x;
					end = x;
				}
			if (end >= 0) {
				buf[end + 1] = '\0';
				curs(start, y + 2);
# ifdef MACOS
				puts(buf + start);
# else
				(void) fputs(buf + start, stdout);
# endif
				curx = end + 1;
			}
		}
# ifdef MACOS
	else {
		for(y = 0; y < ROWNO; y++)
		for(x = 0; x < COLNO; x++)
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym,AT_APP);
			} else if(room->seen)
				at(x,y,room->scrsym,AT_APP);
	}
# endif
#else
	for(y = 0; y < ROWNO; y++)
		for(x = 0; x < COLNO; x++)
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym,AT_APP);
			} else if(room->seen)
				at(x,y,room->scrsym,AT_APP);
#endif /* DGK && !TEXTCOLOR && !CLIPPING */
#ifndef g_putch
	if (GFlag) {
		graph_off();
		GFlag = FALSE;
	}
#endif
	scrlx = COLNO;
	scrly = ROWNO;
	scrhx = scrhy = 0;
	cornbot(0);
	bot();
}

#endif /* OVL0 */
#ifdef OVLB

XSTATIC void
cornbot(lth)
register int lth;
{
	oldbot1[lth] = 0;
	oldbot2[lth] = 0;
	flags.botl = 1;
}

#endif /* OVLB */
#ifdef OVL0

void
docorner(xmin, ymax)
register int xmin, ymax;
{
	register int x, y;
	register struct rm *room;
	register struct monst *mtmp;

	if(u.uswallow) {	/* Can be done more efficiently */
		swallowed(1);
		return;
	}

#ifdef CLIPPING
	xmin += clipx; ymax += clipy;
#endif
	seemons();	/* reset old positions */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->mx >= xmin && mtmp->my < ymax)
		mtmp->mdispl = 0;
	seemons();	/* force new positions to be shown */

#ifdef CLIPPING
	for(y = clipy; y < ymax; y++) {
		if(clipping && y > clipymax && CD) break;
		curs(xmin - clipx, (y - clipy)+2);
#else
	for(y = 0; y < ymax; y++) {
		if(y > ROWNO+1 && CD) break;
		curs(xmin,y+2);
#endif
		cl_end();
		if(y < ROWNO) {
		    for(x = xmin; x < COLNO; x++) {
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym,AT_APP);
			} else
				if(room->seen)
					at(x,y,room->scrsym,AT_APP);
		    }
		}
	}
#ifndef g_putch
	if (GFlag) {
		graph_off();
		GFlag = FALSE;
	}
#endif
	/* Note:          y values: 0 to ymax-1
	 * screen positions from y: 2 to ymax+1
	 *            whole screen: 1 to ROWNO+3
	 *                top line: 1
	 *         dungeon display: 2 to ROWNO+1
	 *       first bottom line: ROWNO+2
	 *      second bottom line: ROWNO+3
	 *         lines on screen: ROWNO+3
	 */
	if(ymax > ROWNO) {
		cornbot(xmin-1);
		if(ymax > ROWNO+2 && CD) {	/* clear portion of long */
			curs(1,ROWNO+4);	/* screen below status lines */
			cl_eos();
		}
	}
}

#endif /* OVL0 */
#ifdef OVL1

void
seeglds()
{
	register struct gold *gold, *gold2;

	for(gold = fgold; gold; gold = gold2) {
	    gold2 = gold->ngold;
	    if(Hallucination && cansee(gold->gx,gold->gy))
		if(!(gold->gx == u.ux && gold->gy == u.uy) || Invisible)
		    atl(gold->gx,gold->gy,rndobjsym());
	}
}

/* Trolls now regenerate thanks to KAA */

void
seeobjs()
{
	register struct obj *obj, *obj2;

	for(obj = fobj; obj; obj = obj2) {
	    obj2 = obj->nobj;

	    if(Hallucination && cansee(obj->ox,obj->oy))
		if(!(obj->ox == u.ux && obj->oy == u.uy) || Invisible)
		    atl(obj->ox,obj->oy,rndobjsym());

	    if(obj->olet == FOOD_SYM && obj->otyp == CORPSE) {

		if(mons[obj->corpsenm].mlet == S_TROLL &&
		    obj->age + 20 < monstermoves) {
			boolean visible = cansee(obj->ox,obj->oy);
			struct monst *mtmp = revive(obj, FALSE);

			if (mtmp && visible)
				pline("%s rises from the dead!",
					(mtmp->mhp==mtmp->mhpmax) ? Monnam(mtmp)
					: Amonnam(mtmp, "bite-covered"));
		} else if (obj->corpsenm != PM_LIZARD &&
						obj->age + 250 < monstermoves)
			delobj(obj);
	    }
	}

	for(obj = invent; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(obj->otyp == CORPSE) {
		if(mons[obj->corpsenm].mlet == S_TROLL
			    && obj->age + 20 < monstermoves) {
		    boolean wielded = (obj==uwep);
		    struct monst *mtmp = revive(obj, TRUE);

		    if (mtmp && wielded)
			pline("The %s%s %s writhes out of your grasp!",
				(mtmp->mhp < mtmp->mhpmax) ? "bite-covered ":"",
				mtmp->data->mname, xname(obj));
		    else if (mtmp)
			You("feel squirming in your backpack!");
		} else if (obj->corpsenm != PM_LIZARD &&
						obj->age + 250 < monstermoves)
		    useup(obj);
	    }
	}
}

#endif /* OVL1 */
#ifdef OVL0

void
seemons()
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if(mtmp->data->mlet == S_EEL)
		mtmp->minvis = (u.ustuck != mtmp && is_pool(mtmp->mx,mtmp->my));
	    pmon(mtmp);
#ifdef WORM
	    if(mtmp->wormno) wormsee(mtmp->wormno);
#endif
	}
}

void
pmon(mon)
register struct monst *mon;
{
	register int show = showmon(mon);

	if(mon->mdispl)
	    if(mon->mdx != mon->mx || mon->mdy != mon->my || !show)
		unpmon(mon);

/* If you're hallucinating, the monster must be redrawn even if it has
 * already been printed.
 */
	if(show && (!mon->mdispl || Hallucination)) {
	    if (Hallucination)
	    atl(mon->mx,mon->my,
		(char) ((!mon->mimic || Protection_from_shape_changers) ?
		rndmonsym() : (mon->m_ap_type == M_AP_FURNITURE) ?
		showsyms[mon->mappearance] : rndobjsym()));
	    else

		atl(mon->mx,mon->my,
		    (!mon->m_ap_type ||
		     Protection_from_shape_changers) ?
		     mon->data->mlet : (char) mimic_appearance(mon));
		mon->mdispl = 1;
		mon->mdx = mon->mx;
		mon->mdy = mon->my;
	}
#ifndef g_putch
	if (GFlag) {
		graph_off();
		GFlag = FALSE;
	}
#endif
}

#endif /* OVL0 */
#ifdef OVL1

void
unpmon(mon)
register struct monst *mon;
{
	if(mon->mdispl) {
		newsym(mon->mdx, mon->mdy);
		mon->mdispl = 0;
	}
}

#endif /* OVL1 */
#ifdef OVL0

void
nscr() {
	register int x, y;
	register struct rm *room;

	if(u.uswallow || u.ux == FAR || flags.nscrinh) return;
	pru();
	for(y = scrly; y <= scrhy; y++)
		for(x = scrlx; x <= scrhx; x++)
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym,AT_APP);
			}
#ifndef g_putch
	if (GFlag) {
		graph_off();
		GFlag = FALSE;
	}
#endif
	scrhx = scrhy = 0;
	scrlx = COLNO;
	scrly = ROWNO;
}

#endif /* OVL0 */
#ifdef OVL1

/* Make sure that there are 18 entries in the rank arrays. */
/* 0 and even entries are male ranks, odd entries are female. */

static const char *mage_ranks[] = {
	"Evoker",
	"Evoker",
	"Conjurer",
	"Conjurer",
	"Thaumaturge",
	"Thaumaturge",
	"Magician",
	"Magician",
	"Enchanter",
	"Enchanter",
	"Sorcerer",
	"Sorceress",
	"Necromancer",
	"Necromancer",
	"Wizard",
	"Wizard",
	"Mage",
	"Mage"
};

static const char *priest_ranks[] = {
	"Aspirant",
	"Aspirant",
	"Acolyte",
	"Acolyte",
	"Adept",
	"Adept",
	"Priest",
	"Priestess",
	"Curate",
	"Curate",
	"Canon",
	"Canoness",
	"Lama",
	"Lama",
	"Patriarch",
	"Matriarch",
	"High Priest",
	"High Priestess"
};

static const char *thief_ranks[] = {
	"Footpad",
	"Footpad",
	"Cutpurse",
	"Cutpurse",
	"Rogue",
	"Rogue",
	"Pilferer",
	"Pilferer",
	"Robber",
	"Robber",
	"Burglar",
	"Burglar",
	"Filcher",
	"Filcher",
	"Magsman",
	"Magswoman",
	"Thief",
	"Thief"
};

static const char *fighter_ranks[] = {
	"Stripling",
	"Stripling",
	"Skirmisher",
	"Skirmisher",
	"Fighter",
	"Fighter",
	"Man-at-arms",
	"Woman-at-arms",
	"Warrior",
	"Warrior",
	"Swashbuckler",
	"Swashbuckler",
	"Hero",
	"Heroine",
	"Champion",
	"Champion",
	"Lord",
	"Lady"
};

static const char *tourist_ranks[] = {
	"Rambler",
	"Rambler",
	"Sightseer",
	"Sightseer",
	"Excursionist",
	"Excursionist",
	"Peregrinator",
	"Peregrinator",
	"Traveler",
	"Traveler",
	"Journeyer",
	"Journeyer",
	"Voyager",
	"Voyager",
	"Explorer",
	"Explorer",
	"Adventurer",
	"Adventurer"
};

static const char *nomad_ranks[] = {
	"Troglodyte",
	"Troglodyte",
	"Aborigine",
	"Aborigine",
	"Wanderer",
	"Wanderer",
	"Vagrant",
	"Vagrant",
	"Wayfarer",
	"Wayfarer",
	"Roamer",
	"Roamer",
	"Nomad",
	"Nomad",
	"Rover",
	"Rover",
	"Pioneer",
	"Pioneer"
};

static const char *knight_ranks[] = {
	"Gallant",
	"Gallant",
	"Esquire",
	"Esquire",
	"Bachelor",
	"Bachelor",
	"Sergeant",
	"Sergeant",
	"Knight",
	"Knight",
	"Banneret",
	"Banneret",
	"Chevalier",
	"Chevalier",
	"Seignieur",
	"Seignieur",
	"Paladin",
	"Paladin"
};

static const char *archeo_ranks[] = {
	"Digger",
	"Digger",
	"Field Worker",
	"Field Worker",
	"Investigator",
	"Investigator",
	"Exhumer",
	"Exhumer",
	"Excavator",
	"Excavator",
	"Spelunker",
	"Spelunker",
	"Speleologist",
	"Speleologist",
	"Collector",
	"Collector",
	"Curator",
	"Curator"
};

static const char *healer_ranks[] = {
	"Pre-Med",
	"Pre-Med",
	"Med Student",
	"Med Student",
	"Medic",
	"Medic",
	"Intern",
	"Intern",
	"Doctor",
	"Doctor",
	"Physician",
	"Physician",
	"Specialist",
	"Specialist",
	"Surgeon",
	"Surgeon",
	"Chief Surgeon",
	"Chief Surgeon"
};

static const char *barbarian_ranks[] = {
	"Plunderer",
	"Plunderess",
	"Pillager",
	"Pillager",
	"Bandit",
	"Bandit",
	"Brigand",
	"Brigand",
	"Raider",
	"Raider",
	"Reaver",
	"Reaver",
	"Slayer",
	"Slayer",
	"Chieftain",
	"Chieftainess",
	"Conqueror",
	"Conqueress"
};

static const char *ninja_ranks[] = {
	"Chigo",
	"Chigo",
	"Bushi",
	"Bushi",
	"Genin",
	"Genin",
	"Genin",
	"Genin",
	"Chunin",
	"Chunin",
	"Chunin",
	"Chunin",
	"Jonin",
	"Jonin",
	"Jonin",
	"Jonin",
	"Jonin",
	"Jonin",
};

static const char *elf_ranks[] = {
	"Edhel",
	"Elleth",
	"Edhel",
	"Elleth", 	/* elf-maid */
	"Ohtar", 	/* warrior */
	"Ohtie",
	"Kano", 	/* commander (Q.) ['a] */
	"Kanie", 	/* educated guess, until further research- SAC */
	"Arandur", 	/* king's servant, minister (Q.) - educated guess */
	"Aranduriel", 	/* educated guess */
	"Hir", 		/* lord (S.) */
	"Hiril", 	/* lady (S.) ['ir] */
	"Aredhel", 	/* noble elf (S.) */
	"Arwen", 	/* noble maiden (S.) */
	"Ernil", 	/* prince (S.) */
	"Elentariel", 	/* elf-maiden (Q.) */
	"Elentar", 	/* Star-king (Q.) */
	"Elentari", 	/* Star-queen (Q.) */ /* Elbereth (S.) */
};

#endif /* OVL1 */

OSTATIC const char **NDECL(rank_array);

#ifdef OVL1

XSTATIC const char **
rank_array() {
	register const char **ranks;

	switch(pl_character[0]) {
		case 'A':  ranks = archeo_ranks; break;
		case 'B':  ranks = barbarian_ranks; break;
		case 'C':  ranks = nomad_ranks; break;
		case 'E':  ranks = elf_ranks; break;
		case 'H':  ranks = healer_ranks; break;
		case 'K':  ranks = knight_ranks; break;
		case 'P':  ranks = priest_ranks; break;
		case 'R':  ranks = thief_ranks; break;
		case 'S':  ranks = ninja_ranks; break;
		case 'T':  ranks = tourist_ranks; break;
		case 'V':  ranks = fighter_ranks; break;
		case 'W':  ranks = mage_ranks; break;
		default:   ranks = 0; break;
	}
	return(ranks);
}

#endif /* OVL1 */

OSTATIC const char *rank();

#ifdef OVL1

XSTATIC const char *
rank() {
	register int place;
	register const char **ranks = rank_array();

	if(u.ulevel < 3) place = 0;
	else if(u.ulevel <  6) place =  2;
	else if(u.ulevel < 10) place =  4;
	else if(u.ulevel < 14) place =  6;
	else if(u.ulevel < 18) place =  8;
	else if(u.ulevel < 22) place = 10;
	else if(u.ulevel < 26) place = 12;
	else if(u.ulevel < 30) place = 14;
	else place = 16;
	if(flags.female) place++;

	if (!!ranks) return(ranks[place]);
	return(pl_character);
}

#endif /* OVL1 */
#ifdef OVLB

void
max_rank_sz() {
	register int i, maxr = 0;
	register const char **ranks = rank_array();

	if (!!ranks) {
		for(i = flags.female; i < 18; i += 2)
			if(strlen(ranks[i]) > maxr) maxr = strlen(ranks[i]);
		mrank_sz = maxr;
	}
	else mrank_sz = strlen(pl_character);
}

#endif /* OVLB */
#ifdef OVL0

static void
fillbot(row,oldbot,newbot)
int row;
char *oldbot, *newbot;
{
	register char *ob = oldbot, *nb = newbot;
	register int i;
	int fillcol;

	fillcol = min(CO, MAXCO-1);

	/* compress in case line too long */
	if(strlen(newbot) >= fillcol) {
		register char *bp0 = newbot, *bp1 = newbot;

		do {
#ifdef CLIPPING
			if(*bp0 != ' ' || bp0[1] != ' ')
#else
			if(*bp0 != ' ' || bp0[1] != ' ' || bp0[2] != ' ')
#endif
				*bp1++ = *bp0;
		} while(*bp0++);
	}
	newbot[fillcol] = '\0';

	for(i = 1; i < fillcol; i++) {
		if(!*nb) {
			if(*ob || flags.botlx) {
				/* last char printed may be in middle of line */
				curs((int)strlen(newbot)+1,row);
				cl_end();
			}
			break;
		}
		if(*ob != *nb) {
			curs(i,row);
			(void) putchar(*nb);
			curx++;
		}
		if(*ob) ob++;
		nb++;
	}
	Strcpy(oldbot, newbot);
}

static void
bot1()
{
	register int i,j;

#ifdef CLIPPING
	if (CO > 59) {
#endif
	Strcpy(newbot1, plname);
	if('a' <= newbot1[0] && newbot1[0] <= 'z') newbot1[0] += 'A'-'a';
	newbot1[10] = 0;
	Sprintf(eos(newbot1)," the ");
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
		Sprintf(eos(newbot1), mbot);
	} else
		Sprintf(eos(newbot1), rank());
#else
	Sprintf(eos(newbot1), rank());
#endif
	Sprintf(eos(newbot1),"  ");
	i = mrank_sz + 15;
	j = strlen(newbot1);
	if((i - j) > 0)
	      do { Sprintf(eos(newbot1)," "); /* pad with spaces */
		   i--;
	      } while((i - j) > 0);
#ifdef CLIPPING
	}
	else
		*newbot1 = 0;
#endif
	if(ACURR(A_STR)>18) {
		if(ACURR(A_STR)>118)
		    Sprintf(eos(newbot1),"St:%2d ",ACURR(A_STR)-100);
		else if(ACURR(A_STR)<118)
		    Sprintf(eos(newbot1), "St:18/%02d ",ACURR(A_STR)-18);
		else
		    Sprintf(eos(newbot1),"St:18/** ");
	} else
		Sprintf(eos(newbot1), "St:%-1d ",ACURR(A_STR));
	Sprintf(eos(newbot1),
		"Dx:%-1d Co:%-1d In:%-1d Wi:%-1d Ch:%-1d",
		ACURR(A_DEX), ACURR(A_CON), ACURR(A_INT), ACURR(A_WIS), ACURR(A_CHA));
	Sprintf(eos(newbot1), (u.ualigntyp == U_CHAOTIC) ? "  Chaotic" :
			(u.ualigntyp == U_NEUTRAL) ? "  Neutral" : "  Lawful");
#ifdef SCORE_ON_BOTL
	Sprintf(eos(newbot1)," S:%lu"
	    ,(u.ugold - u.ugold0 > 0 ? u.ugold - u.ugold0 : 0)
	    + u.urexp + (50 * maxdlevel)
	    + (maxdlevel > 20? 1000*((maxdlevel > 30) ? 10 : maxdlevel - 20) :0));
#endif
#ifdef CLIPPING
	fillbot(min(LI-1, ROWNO+2), oldbot1, newbot1);
#else
	fillbot(ROWNO+2, oldbot1, newbot1);
#endif
}

static void
bot2()
{
#ifdef ENDGAME
	if(dlevel == ENDLEVEL)
		Sprintf(newbot2, "EndLevel ");
	else
#endif
#ifdef SPELLS
		Sprintf(newbot2, "Dlvl:%-2d ", dlevel);
#else
		Sprintf(newbot2, "Level:%-1d ", dlevel);
#endif
	Sprintf(eos(newbot2),
#ifdef SPELLS
		"G:%-2ld HP:%d(%d) Pw:%d(%d) AC:%-2d",
		u.ugold,
# ifdef POLYSELF
		u.mtimedone ? u.mh : u.uhp, u.mtimedone ? u.mhmax : u.uhpmax,
		u.uen, u.uenmax, u.uac);
# else
		u.uhp, u.uhpmax, u.uen, u.uenmax, u.uac);
# endif
#else
		"Gold:%-1lu HP:%d(%d) AC:%-1d",
		u.ugold,
# ifdef POLYSELF
		u.mtimedone ? u.mh : u.uhp, u.mtimedone ? u.mhmax : u.uhpmax,
		u.uac);
# else
		u.uhp, u.uhpmax, u.uac);
# endif
#endif
#ifdef POLYSELF
	if (u.mtimedone)
		Sprintf(eos(newbot2), " HD:%d", mons[u.umonnum].mlevel);
	else
#endif
#ifdef EXP_ON_BOTL
	Sprintf(eos(newbot2), " Xp:%u/%-1ld", u.ulevel,u.uexp);
#else
	Sprintf(eos(newbot2), " Exp:%u", u.ulevel);
#endif
	if(flags.time)
	    Sprintf(eos(newbot2), " T:%ld", moves);
	if(strcmp(hu_stat[u.uhs], "        ")) {
		Sprintf(eos(newbot2), " ");
		Strcat(newbot2, hu_stat[u.uhs]);
	}
	if(Confusion)	   Sprintf(eos(newbot2), " Conf");
	if(Sick)	   Sprintf(eos(newbot2), " Sick");
	if(Blind)	   Sprintf(eos(newbot2), " Blind");
	if(Stunned)	   Sprintf(eos(newbot2), " Stun");
	if(Hallucination)  Sprintf(eos(newbot2), " Hallu");
#ifdef CLIPPING
	fillbot(min(LI, ROWNO+3), oldbot2, newbot2);
#else
	fillbot(ROWNO+3, oldbot2, newbot2);
#endif
}

void
bot() {
register char *ob1 = oldbot1, *ob2 = oldbot2;
	if(flags.botlx) *ob1 = *ob2 = 0;
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}

#endif /* OVL0 */
#ifdef OVLB

void
mstatusline(mtmp)
register struct monst *mtmp;
{
#if defined(ALTARS) && defined(THEOLOGY)
	int align = mtmp->ispriest
		? ((EPRI(mtmp)->shralign & ~A_SHRINE)-1) :
		mtmp->data->maligntyp;
#else
	int align = mtmp->data->maligntyp;
#endif
	pline("Status of %s (%s): ", mon_nam(mtmp),
		(align <= -1) ? "chaotic" :
		align ? "lawful" : "neutral");
	pline("Level %d  Gold %lu  HP %d(%d)",
	    mtmp->m_lev, mtmp->mgold, mtmp->mhp, mtmp->mhpmax);
	pline("AC %d%s%s", mtmp->data->ac,
	    mtmp->mcan ? ", cancelled" : "" ,mtmp->mtame ? ", tame" : "");
}

void
ustatusline()
{
	pline("Status of %s (%s%s):", plname,
		(u.ualign > 3) ? "stridently " :
		(u.ualign == 3) ? "" :
		(u.ualign >= 1) ? "haltingly " :
		(u.ualign == 0) ? "nominally " :
				"insufficiently ",
		(u.ualigntyp == U_CHAOTIC) ? "chaotic" :
		u.ualigntyp ? "lawful" : "neutral");
	pline("Level %d  Gold %lu  HP %d(%d)  AC %d",
# ifdef POLYSELF
		u.mtimedone ? mons[u.umonnum].mlevel : u.ulevel,
		u.ugold, u.mtimedone ? u.mh : u.uhp,
		u.mtimedone ? u.mhmax : u.uhpmax, u.uac);
# else
		u.ulevel, u.ugold, u.uhp, u.uhpmax, u.uac);
# endif
}

void
cls()
{
	extern xchar tlx, tly;

	if(flags.toplin == 1)
		more();
	flags.toplin = 0;

	clear_screen();

	tlx = tly = 1;

	flags.botlx = 1;
}

#endif /* OVLB */
#ifdef OVL2

char
rndmonsym()
{
	return(mons[rn2(NUMMONS - 1)].mlet);
}

/*
 * we don't use objsyms here because (someday) objsyms may be
 * user programmable
 */

static const char rndobs[] = {
	WEAPON_SYM, ARMOR_SYM, POTION_SYM, SCROLL_SYM, WAND_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	RING_SYM, AMULET_SYM, FOOD_SYM, TOOL_SYM, GEM_SYM, GOLD_SYM, ROCK_SYM };

char
rndobjsym()
{
	return rndobs[rn2(SIZE(rndobs))];
}

static const char *hcolors[] = {
			"ultraviolet", "infrared", "hot pink", "psychedelic",
			"bluish-orange", "reddish-green", "dark white",
			"light black", "loud", "salty", "sweet", "sour",
			"bitter", "luminescent", "striped", "polka-dotted",
			"square", "round", "triangular", "brilliant",
			"navy blue", "cerise", "chartreuse", "mauve",
			"lime green", "copper", "sea green", "spiral",
			"swirly", "blotchy", "fluorescent green",
			"burnt orange", "indigo", "amber", "tan",
			"sky blue-pink", "lemon yellow", "off-white",
			"paisley", "plaid", "argyle", "incandescent"};

const char *
hcolor()
{
	return hcolors[rn2(SIZE(hcolors))];
}

#endif /* OVL2 */
#ifdef OVL0

/*ARGSUSED*/
XSTATIC void
hilite(x, y, let, typ)
int x, y;
uchar let, typ;
{
#ifdef TEXTCOLOR
	boolean colorit;
#endif
	if (let == ' '
#if !defined(MSDOS) && !defined(MACOS)
	    || !flags.standout
#endif
	    ) {
		/* don't hilite spaces; it's pointless colorwise,
		   and also hilites secret corridors and dark areas. -3. */
		g_putch(let);
		return;
	}

	if (!typ) {
		if (let == GOLD_SYM)
			typ = AT_GLD;
		else if (index(obj_symbols, (char) let) != NULL
			 || let == S_MIMIC_DEF)
			/* is an object */
			typ = AT_OBJ;
		else if (vism_at(x, y))
			/* is a monster */
			typ = AT_MON;
	}
#ifdef TEXTCOLOR
# ifdef REINCARNATION
	colorit = flags.use_color && dlevel != rogue_level;
# else
	colorit = flags.use_color;
# endif
	if (colorit) {
	    struct monst *mtmp;

	    switch (typ) {
		case AT_MON:
		    switch (let) {
			case S_MIMIC_DEF:
			    typ = HI_OBJ;
			    break;
		        default:
			    if (u.ux == x && u.uy == y && u.usym == let)
				typ = uasmon->mcolor;
			    else if (mtmp = m_at(x, y))
			        typ = mtmp->m_ap_type ?
					mimic_color(mtmp) :
					mtmp->data->mcolor;
			    else
				typ = 0;
		    }
		    break;
		case AT_OBJ:
		    { struct obj *otmp;

		    if (let == GOLD_SYM)
			typ = HI_GOLD;
		    else if ((otmp = level.objects[x][y]) && 
			   let == objects[otmp->otyp].oc_olet) {
			if (otmp->otyp == CORPSE ||
				otmp->otyp == DRAGON_SCALE_MAIL)
			    typ = mons[otmp->corpsenm].mcolor;
			else
			    typ = objects[level.objects[x][y]->otyp].oc_color;
		     } else
			typ = mimic_color(m_at(x, y));
		    }
		    break;
		case AT_MAP:
		    if ( ((let == POOL_SYM && IS_POOL(levl[x][y].typ))
#ifdef FOUNTAINS
		    || (let == FOUNTAIN_SYM && IS_FOUNTAIN(levl[x][y].typ))
#endif
		     ) && hilites[BLUE] != HI)

			typ = BLUE;
#ifdef THRONES
		    else if (let == THRONE_SYM && IS_THRONE(levl[x][y].typ)
				&& hilites[HI_GOLD] != HI)
			typ = HI_GOLD;
#endif
		    else if (levl[x][y].typ == ROOM && levl[x][y].icedpool
				&& hilites[CYAN] != HI)
			typ = CYAN;
		    else
			typ = 0;
		    break;
		case AT_ZAP:
		    typ = HI_ZAP;
		    break;
		}
	}
	if (typ && colorit)
		xputs(hilites[Hallucination ? rn2(MAXCOLORS) : typ]);
	else
#endif
#ifdef REINCARNATION
	if (typ == AT_MON && dlevel != rogue_level) revbeg();
#else
	if (typ == AT_MON) revbeg();
#endif
	g_putch(let);

#ifdef TEXTCOLOR
	if (typ && colorit) xputs(HE); else
#endif
#ifdef REINCARNATION
	if (typ == AT_MON && dlevel != rogue_level) m_end();
#else
	if (typ == AT_MON) m_end();
#endif
}

#endif /* OVL0 */
#ifdef OVL2

/*
 * find the appropriate symbol for printing a mimic
 */

uchar
mimic_appearance(mon)
struct monst *mon;
{
	switch(mon->m_ap_type) {
	case M_AP_NOTHING:
		return mon->data->mlet;
	case M_AP_FURNITURE:
		return showsyms[mon->mappearance];
	case M_AP_OBJECT:
		return objects[mon->mappearance].oc_olet;
	case M_AP_MONSTER:
		return mons[mon->mappearance].mlet;
	case M_AP_GOLD:
		return GOLD_SYM;
	default:
		impossible("Monster mimicking %d", mon->m_ap_type);
		return 0;
	}
/*NOTREACHED*/
}

#ifdef TEXTCOLOR
/* pick an appropriate color for a mimic imitating an object */

XSTATIC uchar
mimic_color(mtmp)
struct monst *mtmp;
{
	if (!mtmp)
		return 0;
	switch (mtmp->m_ap_type) {
	case M_AP_NOTHING:
		return mtmp->data->mcolor;
	case M_AP_FURNITURE:
# ifdef FOUNTAINS
		if (mtmp->mappearance == S_fountain && hilites[BLUE] != HI)
			return BLUE;
# endif
		return 0;
	case M_AP_OBJECT:
		return objects[mtmp->mappearance].oc_color;
	case M_AP_MONSTER:
		return mons[mtmp->mappearance].mcolor;
	case M_AP_GOLD:
		return HI_GOLD;
	default:
		return 0;
	}
}
#endif

#endif /* OVL2 */
