/*	SCCS Id: @(#)pri.c	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include <stdio.h>
#include "hack.h"
#ifdef GENIX
#define	void	int	/* jhn - mod to prevent compiler from bombing */
#endif
#ifdef MSDOSCOLOR
extern int hilite();
#endif

xchar scrlx, scrhx, scrly, scrhy;	/* corners of new area on screen */

extern char *hu_stat[];	/* in eat.c */
extern char *CD;
extern struct monst *makemon();

swallowed()
{
	char *ulook = "|@|";
	ulook[1] = u.usym;

	cls();
	curs(u.ux-1, u.uy+1);
	fputs("/-\\", stdout);
	curx = u.ux+2;
	curs(u.ux-1, u.uy+2);
	fputs(ulook, stdout);
	curx = u.ux+2;
	curs(u.ux-1, u.uy+3);
	fputs("\\-/", stdout);
	curx = u.ux+2;
	u.udispl = 1;
	u.udisx = u.ux;
	u.udisy = u.uy;
}

setclipped(){
	error("Hack needs a screen of size at least %d by %d.\n",
		ROWNO+2, COLNO);
}

#ifdef DGK
static int multipleAts;		/* TRUE if we have many at()'s to do */
static int DECgraphics;		/* The graphics mode toggle */

#define DECgraphicsON() ((void) putchar('\16'), DECgraphics = TRUE)
#define DECgraphicsOFF() ((void) putchar('\17'), DECgraphics = FALSE)
#endif

at(x,y,ch)
register xchar x,y;
char ch;
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
	y += 2;
	curs(x,y);
#ifdef DGK
	if (flags.DECRainbow) {
		/* If there are going to be many at()s in a row without
		 * intervention, only change the graphics mode when the
		 * character changes between graphic and regular.
		 */
		if (multipleAts) {
			if (ch & 0x80) {
				if (!DECgraphics)
					DECgraphicsON();
				(void) putchar(ch ^ 0x80); /* Strip 8th bit */
			} else {
				if (DECgraphics)
					DECgraphicsOFF();
				(void) putchar(ch);
			}
		/* Otherwise, we don't know how many at()s will be happening
		 * before printing of normal strings, so change to graphics
		 * mode when necessary, then change right back.
		 */
		} else {
			if (ch & 0x80) {
				DECgraphicsON();
				(void) putchar(ch ^ 0x80); /* Strip 8th bit */
				DECgraphicsOFF();
			} else
				(void) putchar(ch);
		}
	} else
#endif
#ifdef MSDOSCOLOR
		hilite(ch);
#else
		(void) putchar(ch);
#endif
	curx++;
}

prme(){
	if(!Invisible) at(u.ux,u.uy,u.usym);
}

doredraw()
{
	docrt();
	return(0);
}

docrt()
{
	register x,y;
	register struct rm *room;
	register struct monst *mtmp;

	if(u.uswallow) {
		swallowed();
		return;
	}
	cls();

/* Some ridiculous code to get display of @ and monsters (almost) right */
	if(!Invisible) {
		levl[(u.udisx = u.ux)][(u.udisy = u.uy)].scrsym = u.usym;
		levl[u.udisx][u.udisy].seen = 1;
		u.udispl = 1;
	} else	u.udispl = 0;

	seemons();	/* reset old positions */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		mtmp->mdispl = 0;
	seemons();	/* force new positions to be shown */
/* This nonsense should disappear soon --------------------------------- */

#if defined(DGK) && !defined(MSDOSCOLOR)
	/* I don't know DEC Rainbows, but if HILITE_COLOR is applicable,
	 * the !defined(HILITE_COLOR) will have to be compensated for.
	 * -kjs */
	/* For DEC Rainbows, we must translate each character to strip
	 * out the 8th bit if necessary.
	 */
	if (flags.DECRainbow) {
		multipleAts = TRUE;
		for(y = 0; y < ROWNO; y++)
			for(x = 0; x < COLNO; x++)
				if((room = &levl[x][y])->new) {
					room->new = 0;
					at(x,y,room->scrsym);
				} else if(room->seen)
					at(x,y,room->scrsym);
		multipleAts = FALSE;
		if (DECgraphics)
			DECgraphicsOFF();
	} else {
	/* Otherwise, line buffer the output to do the redraw in
	 * about 2/3 of the time.
	 */
		for(y = 0; y < ROWNO; y++) {
			char buf[COLNO+1];
			int start, end;

			memset(buf, ' ', COLNO);
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
				fputs(buf + start, stdout);
				curx = end + 1;
			}
		}
	}
#else
	for(y = 0; y < ROWNO; y++)
		for(x = 0; x < COLNO; x++)
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym);
			} else if(room->seen)
				at(x,y,room->scrsym);
#endif
	scrlx = COLNO;
	scrly = ROWNO;
	scrhx = scrhy = 0;
	flags.botlx = 1;
	bot();
}

docorner(xmin,ymax) register xmin,ymax; {
	register x,y;
	register struct rm *room;
	register struct monst *mtmp;

	if(u.uswallow) {	/* Can be done more efficiently */
		swallowed();
		return;
	}

	seemons();	/* reset old positions */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if(mtmp->mx >= xmin && mtmp->my < ymax)
		mtmp->mdispl = 0;
	seemons();	/* force new positions to be shown */

#ifdef DGK
	if (flags.DECRainbow)
		multipleAts = TRUE;
#endif
	for(y = 0; y < ymax; y++) {
		if(y > ROWNO && CD) break;
		curs(xmin,y+2);
		cl_end();
		if(y < ROWNO) {
		    for(x = xmin; x < COLNO; x++) {
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym);
			} else
				if(room->seen)
					at(x,y,room->scrsym);
		    }
		}
	}
#ifdef DGK
	if (flags.DECRainbow) {
		multipleAts = FALSE;
		if (DECgraphics)
			DECgraphicsOFF();
	}
#endif
	if(ymax > ROWNO) {
		cornbot(xmin-1);
		if(ymax > ROWNO+1 && CD) {
			curs(1,ROWNO+3);
			cl_eos();
		}
	}
}

/* Trolls now regenerate thanks to KAA */

seeobjs(){
register struct obj *obj, *obj2;
	for(obj = fobj; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(obj->olet == FOOD_SYM && obj->otyp >= CORPSE) {

		if (obj->otyp == DEAD_TROLL && obj->age + 20 < moves) {
			delobj(obj);
			if (cansee(obj->ox, obj->oy)) 
				pline("The troll rises from the dead!");
			(void) makemon(PM_TROLL,obj->ox, obj->oy);
		} else if (obj->age + 250 < moves) delobj(obj);
	    }
	}

	for(obj = invent; obj; obj = obj2) {
	    obj2 = obj->nobj;
	    if(obj->olet == FOOD_SYM && obj->otyp >= CORPSE) {

		if (obj->otyp == DEAD_TROLL && obj->age + 20 < moves) {
		    if (obj == uwep)
			pline("The dead troll writhes out of your grasp!");
		    else
			pline("You feel squirming in your backpack!");
		    (void)makemon(PM_TROLL,u.ux,u.uy);
		    useup(obj);
		} else if (obj->age + 250 < moves) useup(obj);
	    }
	}
}

seemons(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		if(mtmp->data->mlet == ';')
			mtmp->minvis = (u.ustuck != mtmp &&
					levl[mtmp->mx][mtmp->my].typ == POOL);
		pmon(mtmp);
#ifndef NOWORM
		if(mtmp->wormno) wormsee(mtmp->wormno);
#endif
	}
}

pmon(mon) register struct monst *mon; {
register int show = (Blind && Telepat) || canseemon(mon);
	if(mon->mdispl){
		if(mon->mdx != mon->mx || mon->mdy != mon->my || !show)
			unpmon(mon);
	}

/* If you're hallucinating, the monster must be redrawn even if it has
   already been printed.  Problem: the monster must also be redrawn right
   after hallucination is over, so it looks normal again.  Therefore 
   code similar to pmon is in timeout.c. */
	if(show && (!mon->mdispl || Hallucination)) {
		if (Hallucination) 
		atl(mon->mx,mon->my,
			(!mon->mimic || Protection_from_shape_changers) ?
				rndmonsym() :
				(mon->mappearance == DOOR_SYM) ? DOOR_SYM
				: rndobjsym());
		else

		atl(mon->mx,mon->my,
		 (!mon->mappearance
		  || u.uprops[PROP(RIN_PROTECTION_FROM_SHAPE_CHAN)].p_flgs
		 ) ? mon->data->mlet : mon->mappearance);
		mon->mdispl = 1;
		mon->mdx = mon->mx;
		mon->mdy = mon->my;
	}
}

unpmon(mon) register struct monst *mon; {
	if(mon->mdispl){
		newsym(mon->mdx, mon->mdy);
		mon->mdispl = 0;
	}
}

nscr()
{
	register x,y;
	register struct rm *room;

	if(u.uswallow || u.ux == FAR || flags.nscrinh) return;
	pru();
	for(y = scrly; y <= scrhy; y++)
		for(x = scrlx; x <= scrhx; x++)
			if((room = &levl[x][y])->new) {
				room->new = 0;
				at(x,y,room->scrsym);
			}
	scrhx = scrhy = 0;
	scrlx = COLNO;
	scrly = ROWNO;
}

/* 100 suffices for bot(); no relation with COLNO */
char oldbot[100], newbot[100];
cornbot(lth)
register int lth;
{
	if(lth < sizeof(oldbot)) {
		oldbot[lth] = 0;
		flags.botl = 1;
	}
}

bot()
{
register char *ob = oldbot, *nb = newbot;
register int i;
extern char *eos();
	if(flags.botlx) *ob = 0;
	flags.botl = flags.botlx = 0;
	(void) sprintf(newbot,
#ifdef GOLD_ON_BOTL
# ifdef SPELLS
		"Lev %-2d Gp %-5lu Hp %3d(%d) Ep %3d(%d) Ac %-2d  ",
		dlevel, u.ugold,
#  ifdef KAA
		u.mtimedone ? u.mh : u.uhp, u.mtimedone ? u.mhmax : u.uhpmax,
		u.uen, u.uenmax, u.uac);
#  else
		u.uhp, u.uhpmax, u.uen, u.uenmax, u.uac);
#  endif
# else
		"Level %-2d  Gold %-5lu  Hp %3d(%d)  Ac %-2d  ",
		dlevel, u.ugold,
#  ifdef KAA
		u.mtimedone ? u.mh : u.uhp, u.mtimedone ? u.mhmax : u.uhpmax,
		u.uac);
#  else
		u.uhp, u.uhpmax, u.uac);
#  endif
# endif
#else
# ifdef SPELLS
		"Level %-2d Hp %3d(%d) Energy %3d(%d) Ac %-2d ",
		dlevel,
#  ifdef KAA
		u.mtimedone ? u.mh : u.uhp, u.mtimedone ? u.mhmax, u.uhpmax,
		u.uen, u.uenmax, u.uac);
#  else
		u.uhp, u.uhpmax, u.uen, u.uenmax, u.uac);
#  endif
# else
		"Level %-2d   Hp %3d(%d)   Ac %-2d   ",
		dlevel,
#  ifdef KAA
		u.mtimedone ? u.mh : u.uhp, u.mtimedone ? u.mhmax, u.uhpmax,
		u.uac);
#  else
		u.uhp, u.uhpmax, u.uac);
#  endif
# endif
#endif
#ifdef KAA
	if (u.mtimedone)
		(void) sprintf(eos(newbot), "HD %d", mons[u.umonnum].mlevel);
	else
#endif
	    if(u.ustr>18) {
		if(u.ustr>117)
		    (void) strcat(newbot,"Str 18/**");
		else
		    (void) sprintf(eos(newbot), "Str 18/%02d",u.ustr-18);
	    } else
		(void) sprintf(eos(newbot), "Str %-2d   ",u.ustr);
#ifdef EXP_ON_BOTL
	(void) sprintf(eos(newbot), "  Exp %2d/%-5lu ", u.ulevel,u.uexp);
#else
	(void) sprintf(eos(newbot), "   Exp %2u  ", u.ulevel);
#endif
	(void) strcat(newbot, hu_stat[u.uhs]);
	if(flags.time)
	    (void) sprintf(eos(newbot), "  %ld", moves);
#ifdef SCORE_ON_BOTL
	(void) sprintf(eos(newbot)," S:%lu "
	    ,(u.ugold - u.ugold0 > 0 ? u.ugold - u.ugold0 : 0)
	    + u.urexp + (50 * maxdlevel)
	    + (maxdlevel > 20? 1000*((maxdlevel > 30) ? 10 : maxdlevel - 20) :0));
#endif
	if(strlen(newbot) >= COLNO) {
		register char *bp0, *bp1;
		bp0 = bp1 = newbot;
		do {
			if(*bp0 != ' ' || bp0[1] != ' ' || bp0[2] != ' ')
				*bp1++ = *bp0;
		} while(*bp0++);
	}
	for(i = 1; i<COLNO; i++) {
		if(*ob != *nb){
			curs(i,ROWNO+2);
			(void) putchar(*nb ? *nb : ' ');
			curx++;
		}
		if(*ob) ob++;
		if(*nb) nb++;
	}
	(void) strcpy(oldbot, newbot);
}

#if defined(WAN_PROBING) || defined(KAA)
mstatusline(mtmp) register struct monst *mtmp; {
	pline("Status of %s: ", monnam(mtmp));
	pline("Level %-2d  Gold %-5lu  Hp %3d(%d)",
	    mtmp->data->mlevel, mtmp->mgold, mtmp->mhp, mtmp->mhpmax);
	pline("Ac %-2d  Dam %d %s %s",
	    mtmp->data->ac, (mtmp->data->damn + 1) * (mtmp->data->damd + 1),
	    mtmp->mcan ? ", cancelled" : "" ,mtmp->mtame ? " (tame)" : "");
}

extern char plname[];
ustatusline() {
	pline("Status of %s%s ", (Badged) ? "Officer " : "", plname);
	pline("Level %d, gold %lu, hit points %d(%d), AC %d.",
# ifdef KAA
		u.ulevel, u.ugold, u.mtimedone ? u.mh : u.uhp,
		u.mtimedone ? u.mhmax : u.uhpmax, u.uac);
# else
		u.ulevel, u.ugold, u.uhp, u.uhpmax, u.uac);
# endif
}
#endif

cls(){
	if(flags.toplin == 1)
		more();
	flags.toplin = 0;

	clear_screen();

	flags.botlx = 1;
}

rndmonsym() {
	register int x;
	if((x=rn2(58)) < 26)
		return('a'+x);
	else if (x<52)
		return('A'+x-26);
	else switch(x) {
		case 52: return(';');
		case 53: return('&');
		case 54: return(':');
		case 55: return('\'');
		case 56: return(',');
		case 57: return('9');
		default: impossible("Bad random monster %d",x); return('{');
	}
}

rndobjsym() {
	char *rndsym=")[!?%/=*($`";
	return *(rndsym+rn2(11));
}

char *hcolors[] = { "ultraviolet","infrared","hot pink", "psychedelic",
"bluish-orange","reddish-green","dark white","light black","loud",
"salty","sweet","sour","bitter","luminescent","striped","polka-dotted",
"square","round","triangular","brilliant","navy blue","cerise",
"chartreuse","copper","sea green","spiral","swirly","blotchy",
"fluorescent green","burnt orange","indigo","amber","tan",
"sky blue-pink","lemon yellow" };

char *
hcolor() {
	return hcolors[rn2(35)];
}

#ifdef MSDOSCOLOR
/* what if a level character is the same as an object/monster? */

extern char obj_symbols[];

hilite(let)
char let;
{
	char *isobjct = index(obj_symbols, let);
	int ismnst();

	if (!HI || !HE) {
		(void) putchar(let);
		return;
	}
	if (isobjct != NULL || let == GOLD_SYM) {
	/* is an object */
		printf("%s%c%s", HI_OBJ, let, HE);
	} else if (ismnst(let)) {
	/* is a monster */
		printf("%s%c%s", HI_MON, let, HE);
	} else {	
	/* default */
		(void) putchar(let);
	}
}

int
ismnst(let)
char let;
{
	register int ct;
	register struct permonst *ptr;

	for (ct = 0 ; ct < CMNUM + 2 ; ct++) {
		ptr = &mons[ct];
		if(ptr->mlet == let) return(1);
	}
	if (let == '1') return(1);
	else if (let == '2') return(1);
	else if (let == ';') return(1);
	else return(0);
}
#endif
