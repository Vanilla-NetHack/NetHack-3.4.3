/*	SCCS Id: @(#)end.c	3.0	88/05/03
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"
#ifndef NO_SIGNAL
#include <signal.h>
#endif

#include "eshk.h"

void end_box_display();

int
done1()
{
#ifndef NO_SIGNAL
	(void) signal(SIGINT,SIG_IGN);
#endif
	if(flags.ignintr) {
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
		clrlin();
		curs_on_u();
		(void) fflush(stdout);
		if(multi > 0) nomul(0);
		return 0;
	}
	return done2();
} 

int
done2()
{
	pline("Really quit? ");
	if(yn() == 'n') {
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
		clrlin();
		curs_on_u();
		(void) fflush(stdout);
		if(multi > 0) nomul(0);
		return 0;
	}
#if defined(WIZARD) && defined(UNIX)
	if(wizard) {
	    pline("Dump core? ");
	    if(yn() == 'y') {
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
		settty(NULL);
#ifdef SYSV
		(void)
#endif
		    abort();
	    }
	}
#endif
#ifndef LINT
	done("quit");
#endif
	return 0;
}

static
int
done_intr(){
	done_stopprint++;
#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#ifdef UNIX
	(void) signal(SIGQUIT, SIG_IGN);
#endif
#endif /* TOS /* */
	return 0;
}

#ifdef UNIX
static
int
done_hangup(){
	done_hup++;
	(void)signal(SIGHUP, SIG_IGN);
	(void)done_intr();
	return 0;
}
#endif

void
done_in_by(mtmp)
register struct monst *mtmp;
{
	char buf[BUFSZ];

	You("die...");
	buf[0] = '\0';
	if (mtmp->minvis)
		Sprintf(eos(buf), "invisible ");
	if (Hallucination)
		Sprintf(eos(buf), "hallucinogen-distorted ");

	if(mtmp->data->mlet == S_GHOST) {
		register char *gn = (char *) mtmp->mextra;
		if (!Hallucination && !mtmp->minvis && *gn)
			Sprintf(eos(buf), "the ");
		Sprintf(eos(buf), (*gn ? "ghost of %s" : "ghost%s"), gn);
	} else if(mtmp->isshk) {
		Sprintf(eos(buf), "%s %s, the shopkeeper",
			(ESHK(mtmp)->ismale ? "Mr." : "Ms."), shkname(mtmp));
	} else if (mtmp->iswiz)
		Sprintf(eos(buf), "the %s", mons[PM_WIZARD_OF_YENDOR].mname);
	else Sprintf(eos(buf), "%s", mtmp->data->mname);
	if (mtmp->mnamelth) Sprintf(eos(buf), " called %s", NAME(mtmp));
	killer = buf;
	if (mtmp->data->mlet == S_WRAITH)
		u.ugrave_arise = PM_WRAITH;
	else if (mtmp->data->mlet == S_MUMMY)
		u.ugrave_arise = (pl_character[0]=='E') ?
						PM_ELF_MUMMY : PM_HUMAN_MUMMY;
	else if (mtmp->data->mlet == S_VAMPIRE)
		u.ugrave_arise = PM_VAMPIRE;
	if (u.ugrave_arise > -1 && (mons[u.ugrave_arise].geno & G_GENOD))
		u.ugrave_arise = -1;
	if (mtmp->data->mlet == S_COCKATRICE)
		done("stoned");
	else
		done("died");
	return;
}

/*VARARGS1*/
boolean panicking;
extern boolean hu;	/* from save.c */

void
panic(str,a1,a2,a3,a4,a5,a6)
char *str;
{
	if(panicking++)
#ifdef SYSV
	    (void)
#endif
		abort();    /* avoid loops - this should never happen*/
				    /* was exit(1) */
	home(); cls();
	(void) puts(" Suddenly, the dungeon collapses.");
#ifdef WIZARD
# ifndef MSDOS
	if(!wizard) {
	    pline("Report error to %s and it may be possible to rebuild.",WIZARD);
	    more();
	}
	Sprintf (SAVEF, "%s.e", SAVEF);
	hu = FALSE;
	(void) dosave0();
# endif
#endif
	(void) fputs(" ERROR:  ", stdout);
	Printf(str,a1,a2,a3,a4,a5,a6);
	more();				/* contains a fflush() */
#ifdef WIZARD
# ifdef UNIX
	if (wizard)	
#  ifdef SYSV
		(void)
#  endif
		    abort();	/* generate core dump */
# endif
#endif
	done("panicked");
}

/* called with arg "died", "drowned", "escaped", "quit", "choked", "panicked",
   "burned", "starved", "stoned", or "tricked" */
/* Be careful not to call panic from here! */
void
done(st1)
register char *st1;
{
	struct permonst *upmon;
	char buf[BUFSZ], buf1[BUFSZ], buf2[BUFSZ], buf3[BUFSZ];
	/* buf: used if killer gets changed
	 * buf1: used if st1 gets changed
	 * buf2: same as player name, except it is capitalized
	 * buf3: used to copy killer in case it comes from something like
		xname(), which would otherwise get overwritten when we call
		xname() when listing possessions
	 */
	char	c;
	boolean taken;

	Strcpy(buf3, killer);
	killer = buf3;
#ifdef WIZARD
	if (wizard && *st1=='t') {
		You("are a very tricky wizard, it seems.");
		return;
	}
#endif
	if(Lifesaved && index("bcds", *st1)){
		u.uswldtim = 0;
		if(u.uhpmax < 0) u.uhpmax = 10;	/* arbitrary */
		u.uhp = u.uhpmax;
		adjattrib(A_CON, -1, TRUE);
		pline("But wait...");
		makeknown(AMULET_OF_LIFE_SAVING);
		Your("medallion %s!",
		      !Blind ? "begins to glow" : "feels warm");
		You("feel much better!");
		pline("The medallion crumbles to dust!");
		useup(uamul);
		Lifesaved = 0;
		if (u.uhunger < 500) u.uhunger = 500;
		nomovemsg = "You survived that attempt on your life.";
		curs_on_u();
		flags.move = 0;
		if(multi > 0) multi = 0; else multi = -1;
		flags.botl = 1;
		u.ugrave_arise = -1;
		if (!strncmp(killer, "genocide", 8)) {
			pline("Unfortunately you are still genocided...");
			done("died");
		}
		return;
	}
#if defined(WIZARD) || defined(EXPLORE_MODE)
	if((wizard || discover) && index("bcds", *st1)){
		pline("Die? ");
		if(yn() == 'y') goto die;
		u.uswldtim = 0;
		if(u.uhpmax < 0) u.uhpmax = 100;	/* arbitrary */
		u.uhp = u.uhpmax;
		if (u.uhunger < 500) u.uhunger = 500;
		pline("Ok, so you don't die.");
		nomovemsg = "You survived that attempt on your life.";
		curs_on_u();
		flags.move = 0;
		if(multi > 0) multi = 0; else multi = -1;
		flags.botl = 1;
		u.ugrave_arise = -1;
		return;
	}
#endif /* WIZARD || EXPLORE_MODE */
die:
#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done_intr);
#ifdef UNIX
	(void) signal(SIGQUIT, (SIG_RET_TYPE) done_intr);
	(void) signal(SIGHUP, (SIG_RET_TYPE) done_hangup);
#endif
#endif /* NO_SIGNAL /* */
	upmon = player_mon();
	if(u.ugrave_arise > -1) /* create no corpse */ ;
	else if(*st1 == 's' && st1[2] == 'o') 
		(mk_named_object(STATUE, upmon, u.ux, u.uy, plname,
					strlen(plname)))->spe = 0;
	else
		(void) mk_named_object(CORPSE, upmon, u.ux, u.uy, plname,
							strlen(plname));
	if(*st1 == 'q' && u.uhp < 1){
		st1 = "died";
		killer = "quit while already on Charon's boat";
	}
	if(*st1 == 's' && st1[2] == 'a') killer = "starvation"; else
	if(*st1 == 'd' && st1[1] == 'r') killer = "drowning"; else
	if(*st1 == 'p') killer = "panic"; else
	if(*st1 == 't') killer = "trickery"; else
	if(!index("bcds", *st1)) killer = st1;
	taken = paybill();
	paygd();
	clearlocks();
	if(flags.toplin == 1) more();

	if(invent) {
	    if(taken)
		pline("Do you want to see what you had when you %s? ",
			(*st1=='q') ? "quit" : "died");
	    else
		pline("Do you want your possessions identified? ");
	    /* New dump format by maartenj@cs.vu.nl */
	    if ((c = yn_function(ynqchars,'y')) == 'y') {
		struct obj *obj;

		for(obj = invent; obj && !done_stopprint; obj = obj->nobj) {
		    makeknown(obj->otyp);
		    obj->known = obj->bknown = obj->dknown = 1;
		}
		doinv(NULL);
		end_box_display();
	    }
	    if (c == 'q')  done_stopprint++;
	    if (taken) {
		/* paybill has already given the inventory locations in the shop
		 * and put it on the main object list
		 */
		struct obj *obj;

		for(obj = invent; obj; obj = obj->nobj) {
		    obj->owornmask = 0;
		    if(rn2(5)) curse(obj);
		}
	        invent = (struct obj *) 0;
	    }
	}

	if(index("bcds", *st1)){
#ifdef WIZARD
	    if(wizard) {
		pline("Save bones? ");
		if(yn() == 'y') savebones();
	    }  else
#endif
		savebones();
	    if(!flags.notombstone) outrip();
	}
	if(*st1 == 'c') killer = st1;		/* after outrip() */
	if(*st1 == 's' && st1[2] == 'o') {
		Sprintf(buf, "turned to stone by %s",killer);
		/* No a or an; topten.c will do that. */
		killer = buf;
		st1 = "turned to stone";
	}
	Strcpy(buf1, st1);
	if(u.uhave_amulet) Strcat(killer," (with the Amulet)");
	settty(NULL);	/* does a clear_screen() */
	Strcpy(buf2, plname);
	if('a' <= buf2[0] && buf2[0] <= 'z') buf2[0] += 'A'-'a';
	if(!done_stopprint)
	    Printf("Goodbye %s the %s...\n\n", buf2,
#ifdef ENDGAME
		   *st1 != 'a' ? pl_character : "Demigod");
#else
		   pl_character);
#endif
	{ long int tmp;
	  tmp = u.ugold - u.ugold0;
	  if(tmp < 0)
		tmp = 0;
	  if(*st1 == 'd' || *st1 == 'b')
		tmp -= tmp/10;
	  u.urexp += tmp;
	  u.urexp += 50 * maxdlevel;
	  if(maxdlevel > 20)
		u.urexp += 1000*((maxdlevel > 30) ? 10 : maxdlevel - 20);
#ifdef ENDGAME
	  if(*st1 == 'a') u.urexp *= 2;
#endif
	}
	if(*st1 == 'e') {
		register struct monst *mtmp;
		register struct obj *otmp;
		long i;
		register unsigned int worthlessct = 0;

		killer = st1;
		keepdogs();
		mtmp = mydogs;
		if(mtmp) {
			if(!done_stopprint) Printf("You");
			while(mtmp) {
				if(!done_stopprint)
					Printf(" and %s", mon_nam(mtmp));
				if(mtmp->mtame)
					u.urexp += mtmp->mhp;
				mtmp = mtmp->nmon;
			}
			if(!done_stopprint)
		    Printf("\nescaped from the dungeon with %ld points,\n",
			u.urexp);
		} else
		if(!done_stopprint)
		  Printf("You escaped from the dungeon with %ld points,\n",
		    u.urexp);
		get_all_from_box(); /* don't forget things in boxes and bags */
		for(otmp = invent; otmp; otmp = otmp->nobj) {
			if(otmp->olet == GEM_SYM && otmp->otyp < LUCKSTONE) {
				makeknown(otmp->otyp);
				i = (long) otmp->quan *
					objects[otmp->otyp].g_val;
				if(i == 0) {
					worthlessct += otmp->quan;
					continue;
				}
				u.urexp += i;
				Printf("        %s (worth %ld Zorkmids),\n",
				    doname(otmp), i);
			} else if(otmp->olet == AMULET_SYM) {
				otmp->known = 1;
				i = (otmp->spe < 0) ? 2 : 
					otmp->otyp == AMULET_OF_YENDOR ?
							5000 : 500;
				u.urexp += i;
				Printf("        %s (worth %ld Zorkmids),\n",
				    doname(otmp), i);
			}
		}
		if(worthlessct)
		  Printf("        %u worthless piece%s of colored glass,\n",
			worthlessct, plur((long)worthlessct));
		if(u.uhave_amulet) killer = "escaped (with Amulet)";
		else killer = "escaped";
	} else
		if(!done_stopprint) {
		    Printf("You %s ", 
			!strcmp(st1, "tricked") ? "were tricked" : st1);
#ifdef ENDGAME
		    if (*st1 != 'a') {
			if(dlevel == ENDLEVEL)
			     Printf("in the endgame ");
		    	else Printf("on dungeon level %d ", dlevel);
		    }
#else
		    Printf("on dungeon level %d ", dlevel);
#endif
		    Printf("with %ld points,\n", u.urexp);
		}
	if(!done_stopprint)
	  Printf("and %ld piece%s of gold, after %ld move%s.\n",
	    u.ugold, plur(u.ugold), moves, plur(moves));
	if(!done_stopprint)
  Printf("You were level %u with a maximum of %d hit points when you %s.\n",
	    u.ulevel, u.uhpmax, buf1);
	if(*st1 == 'e' && !done_stopprint){
		getret();	/* all those pieces of coloured glass ... */
		cls();
	}
#if defined(WIZARD) || defined(EXPLORE_MODE)
	if(wizard || discover)
		Printf("\nSince you were in %s mode, the score list \
will not be checked.\n", wizard ? "wizard" : "discover");
	else
#endif
		topten();
/* "So when I die, the first thing I will see in Heaven is a score list?" */
	if(done_stopprint) Printf("\n\n");
#ifdef APOLLO
	getret();
#endif
	exit(0);
}

void
clearlocks(){
#if defined(DGK) && !defined(OLD_TOS)
	eraseall(levels, alllevels);
	if (ramdisk)
		eraseall(permbones, alllevels);
#else
#if defined(UNIX) || (defined(MSDOS) && !defined(OLD_TOS))
	register int x;
#ifdef UNIX
	(void) signal(SIGHUP,SIG_IGN);
#endif
	for(x = maxdlevel; x >= 0; x--) {
		glo(x);
		(void) unlink(lock);	/* not all levels need be present */
	}
#endif
#endif
}

#ifdef NOSAVEONHANGUP
int
hangup()
{
	(void) signal(SIGINT, SIG_IGN);
	clearlocks();
	exit(1);
}
#endif

void
end_box_display()
{
	register struct obj *box, *obj;
	char buf[BUFSZ];

	for(box=invent; box; box=box->nobj) {
	    if (Is_container(box) && box->otyp != BAG_OF_TRICKS) {
		int cnt=0;

		for(obj=fcobj; obj; obj=obj->nobj) {
		    if (obj->cobj == box) {
			if (!cnt) {
			    Sprintf(buf, "Contents of the %s:",xname(box));
			    cornline(0, buf);
			}
			makeknown(obj->otyp);
			obj->known = obj->bknown = obj->dknown = 1;
			cornline(1,doname(obj));
			cnt++;
		    }
		}
		if (!cnt) pline("The %s is empty.", xname(box));
		else cornline(2,"");
	    }
	}
}
