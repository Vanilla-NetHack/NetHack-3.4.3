/*	SCCS Id: @(#)pri.c	3.0	89/06/16
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"
#include <ctype.h>  /* for isalpha() */
#if defined(ALTARS) && defined(THEOLOGY)
#include "epri.h"
#endif

static void hilite P((UCHAR_P, UCHAR_P));
static void cornbot P((int));
static boolean ismnst P((CHAR_P));
#if !defined(DECRAINBOW) && !defined(UNIX)
#  define g_putch  (void) putchar
#endif

#ifndef g_putch
static boolean GFlag = FALSE; /* graphic flag */
#endif

/* 100 suffices for bot(); must be larger than COLNO */
#define MAXCO 100
static char oldbot1[MAXCO], newbot1[MAXCO];
static char oldbot2[MAXCO], newbot2[MAXCO];
static const char *dispst = "*0#@#0#*0#@#0#*0#@#0#*0#@#0#*0#@#0#*";
static int mrank_sz = 0;  /* loaded by max_rank_sz (called in u_init) */

void
swallowed(first)
register int first;
{
	if(first) cls();
	else {
		curs(u.ustuck->mdx-1, u.ustuck->mdy+1);
		(void) fputs("   ", stdout);
		curx = u.ustuck->mdx+2;
		curs(u.ustuck->mdx-1, u.ustuck->mdy+2);
		(void) fputs("   ", stdout);
		curx = u.ustuck->mdx+2;
		curs(u.ustuck->mdx-1, u.ustuck->mdy+3);
		(void) fputs("   ", stdout);
		curx = u.ustuck->mdx+2;
	}
	curs(u.ux-1, u.uy+1);
	(void) fputs("/-\\", stdout);
	curx = u.ux+2;
	curs(u.ux-1, u.uy+2);
	(void) putchar('|');
	hilite(u.usym, AT_MON);
	(void) putchar('|');
	curx = u.ux+2;
	curs(u.ux-1, u.uy+3);
	(void) fputs("\\-/", stdout);
	curx = u.ux+2;
	u.udispl = 1;
	u.udisx = u.ux;
	u.udisy = u.uy;
}

void
setclipped()
{
	error("NetHack needs a screen of size at least %d by %d.\n",
		ROWNO+3, COLNO);
}

/*
 *  Allow for a different implementation than this...
 */

#ifndef g_putch

static void
g_putch(ch)
uchar ch;
{
	if (ch & 0x80) {
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

static boolean
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
#ifndef MSDOS
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

	y += 2;
	curs(x,y);

	hilite(ch,typ);
	curx++;
}

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

void
shieldeff(x, y)		/* produce a magical shield effect at x,y */
	register xchar x, y;
{
	register char *ch;
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

int
doredraw()
{
	docrt();
	return 0;
}

void
docrt()
{
	register int x,y;
	register struct rm *room;
	register struct monst *mtmp;

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

#if defined(DGK) && !defined(TEXTCOLOR)
	/* Otherwise, line buffer the output to do the redraw in
	 * about 2/3 of the time.
	 */
		for(y = 0; y < ROWNO; y++) {
			char buf[COLNO+1];
			int start, end;
#ifdef OLD_TOS
			setmem(buf, COLNO, ' ');
#else
			memset(buf, ' ', COLNO);
#endif /* OLD_TOS */
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
				(void) fputs(buf + start, stdout);
				curx = end + 1;
			}
		}
#else /* DGK && !TEXTCOLOR */
	for(y = 0; y < ROWNO; y++)
		for(x = 0; x < COLNO; x++)
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym,AT_APP);
			} else if(room->seen)
				at(x,y,room->scrsym,AT_APP);
#endif /* DGK && !TEXTCOLOR */
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

static void
cornbot(lth)
register int lth;
{
	oldbot1[lth] = 0;
	oldbot2[lth] = 0;
	flags.botl = 1;
}

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

	seemons();	/* reset old positions */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->mx >= xmin && mtmp->my < ymax)
		mtmp->mdispl = 0;
	seemons();	/* force new positions to be shown */

	for(y = 0; y < ymax; y++) {
		if(y > ROWNO+1 && CD) break;
		curs(xmin,y+2);
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
		    obj->age + 20 < moves) {
			boolean visible = cansee(obj->ox,obj->oy);
			struct monst *mtmp = revive(obj, FALSE);

			if (mtmp && visible)
				pline("%s rises from the dead!", Monnam(mtmp));
		} else if (obj->age + 250 < moves) delobj(obj);
	    }
	}

	for(obj = invent; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(obj->otyp == CORPSE) {
		if(mons[obj->corpsenm].mlet == S_TROLL
			    && obj->age + 20 < moves) {
		    boolean wielded = (obj==uwep);
		    struct monst *mtmp = revive(obj, TRUE);

		    if (mtmp && wielded)
			pline("The %s %s writhes out of your grasp!",
				mtmp->data->mname, xname(obj));
		    else if (mtmp)
			You("feel squirming in your backpack!");
		} else if (obj->age + 250 < moves) useup(obj);
	    }
	}
}

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
		rndmonsym() : (mon->mappearance == DOOR_SYM) ?
		DOOR_SYM : rndobjsym()));
	    else

		atl(mon->mx,mon->my,
		    (!mon->mappearance ||
		     Protection_from_shape_changers) ?
		     mon->data->mlet : mon->mappearance);
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

void
unpmon(mon)
register struct monst *mon;
{
	if(mon->mdispl) {
		newsym(mon->mdx, mon->mdy);
		mon->mdispl = 0;
	}
}

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

static const char **
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

static char *
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
			if(*bp0 != ' ' || bp0[1] != ' ' || bp0[2] != ' ')
				*bp1++ = *bp0;
		} while(*bp0++);
	}
	newbot[fillcol] = '\0';

	for(i = 1; i < fillcol; i++) {
		if(!*nb) {
			if(*ob || flags.botlx) {
				/* last char printed may be in middle of line */
				curs(strlen(newbot)+1,row);
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
	fillbot(ROWNO+2, oldbot1, newbot1);
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
	if(Blinded)	   Sprintf(eos(newbot2), " Blind");
	if(Stunned)	   Sprintf(eos(newbot2), " Stun");
	if(Hallucination)  Sprintf(eos(newbot2), " Hallu");
	fillbot(ROWNO+3, oldbot2, newbot2);
}

void
bot() {
register char *ob1 = oldbot1, *ob2 = oldbot2;
	if(flags.botlx) *ob1 = *ob2 = 0;
	bot1();
	bot2();
	flags.botl = flags.botlx = 0;
}


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

char
rndmonsym()
{
	return(mons[rn2(NUMMONS - 1)].mlet);
}

static const char objsyms[] = {
	WEAPON_SYM, ARMOR_SYM, POTION_SYM, SCROLL_SYM, WAND_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	RING_SYM, AMULET_SYM, FOOD_SYM, TOOL_SYM, GEM_SYM, GOLD_SYM, ROCK_SYM };

char
rndobjsym()
{
	return objsyms[rn2(SIZE(objsyms))];
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

/*  Bug: if a level character is the same as an object/monster, it may be
 *  hilited, because we use a kludge to figure out if a character is an
 *  object/monster symbol.  It's smarter than it was in 2.3, but you
 *  can still fool it (ex. if an object is in a doorway you have not seen,
 *  and you look at a map, the '+' will be taken as a spellbook symbol).
 *
 *  The problem is that whenever a portion of the map needs to be redrawn
 *  (by ^R, after an inventory dropover, after regurgitation...), the
 *  levl[][].scrsym field is used to redraw the map.  A great duplication
 *  of code would be needed to trace back every scrsym to find out what color
 *  it should be.
 *
 *  What is really needed is a levl[][].color field; the color be figured
 *  out at the same time as the screen symbol, and be restored with
 *  redraws.  Unfortunately, as this requires much time and testing,
 *  it will have to wait for NetHack 3.1.  -3.
 */

static void
hilite(let,typ)
uchar let, typ;
{

	if (let == ' '
#ifndef MSDOS
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
		else if (ismnst((char) let))
			/* is a monster */
			typ = AT_MON;
	}
#ifdef TEXTCOLOR
	switch (typ) {
	    case AT_MON:
		switch (let) {
		    case S_MIMIC_DEF:
			typ = HI_OBJ;
			break;
		    case S_YLIGHT:	/* make 'em "glow" */
			typ = YELLOW;
			break;
		    default:
			typ = HI_MON;
		}
		break;
	    case AT_OBJ:
		switch (let) {
		    case GOLD_SYM:
			typ = HI_GOLD;
			break;
		    case WEAPON_SYM:
		    case ARMOR_SYM:
		    case RING_SYM:
		    case AMULET_SYM:
			typ = HI_METAL;
			break;
		    case FOOD_SYM:
		    case POTION_SYM:
			typ = HI_ORGANIC;
			break;
		    default:
			typ = HI_OBJ;
		}
		break;
	    case AT_MAP:
#ifdef FOUNTAINS
		typ = ((let == POOL_SYM || let == FOUNTAIN_SYM)
#else
		typ = (let == POOL_SYM
#endif
			&& HI_COLOR[BLUE] != HI ? BLUE : 0);
		break;
	    case AT_ZAP:
		typ = HI_ZAP;
		break;
	}
	if (typ)
		xputs(HI_COLOR[typ]);
#else
	if (typ == AT_MON) revbeg();
#endif

	g_putch(let);

#ifdef TEXTCOLOR
	if (typ) xputs(HE);
#else
	if (typ == AT_MON) m_end();
#endif
}

static boolean
ismnst(let)
char let;
{
	register int ct;
	register struct permonst *ptr;

	if (let & 0x80) return 0;
	if (isalpha(let)) return 1; /* for speed */

	for (ct = 0 ; ct < NUMMONS; ct++) {
		ptr = &mons[ct];
		if(ptr->mlet == let) return 1;
	}
#ifdef WORM
	if (let == S_WORM_TAIL) return 1;
#endif
	return 0;
}
