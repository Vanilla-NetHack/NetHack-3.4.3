/*	SCCS Id: @(#)end.c	3.0	88/05/03
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define MONATTK_H	/* comment line for pre-compiled headers */
#define NEED_VARARGS	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */

#include "hack.h"
#ifndef NO_SIGNAL
#include <signal.h>
#endif

#include "eshk.h"

void NDECL(end_box_display);
STATIC_PTR int NDECL(done_intr);
static void FDECL(disclose,(int,BOOLEAN_P));

static const char NEARDATA *deaths[] = {		/* the array of death */
	"died", "choked", "poisoned", "starvation", "drowning",
	"burning", "crushed", "turned to stone", "genocided",
	"panic", "trickery",
	"quit", "escaped", "ascended" };

static const char NEARDATA *ends[] = {		/* "when you..." */
	"died", "choked", "were poisoned", "starved", "drowned",
	"burned", "were crushed", "turned to stone", "were genocided",
	"panicked", "were tricked",
	"quit", "escaped", "ascended" };

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
#ifdef MACOS
	if(!flags.silent) SysBeep(1);
	if(UseMacAlert(128) != 1) {
#else
	pline("Really quit? ");
	if(yn() == 'n') {
#endif
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
		clrlin();
		curs_on_u();
		(void) fflush(stdout);
		if(multi > 0) nomul(0);
		multi = 0;
		return 0;
	}
#if defined(WIZARD) && (defined(UNIX) || defined(VMS) || defined(LATTICE))
	if(wizard) {
# ifdef VMS
	    pline("Enter debugger? ");
# else
#  ifdef LATTICE
	    pline("Create SnapShot? ");
#  else
	    pline("Dump core? ");
#  endif
# endif
/* KL - do I need to change the next 3 lines? */
	    if(yn() == 'y') {
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
		settty(NULL);
#ifdef AMIGA
		Abort(0);
#else
# ifdef SYSV
		(void)
# endif
		    abort();
#endif
	    }
	}
#endif
#ifndef LINT
	done(QUIT);
#endif
	return 0;
}

STATIC_PTR
int
done_intr(){
	done_stopprint++;
#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
# if defined(UNIX) || defined(VMS)
	(void) signal(SIGQUIT, SIG_IGN);
# endif
#endif /* NO_SIGNAL /* */
	return 0;
}

#if defined(UNIX) || defined(VMS)
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
	if (mtmp->iswiz) {
		Strcat(buf, "the ");
		killer_format = KILLED_BY;
	}
	if (mtmp->minvis)
		Strcat(buf, "invisible ");
	if (Hallucination)
		Strcat(buf, "hallucinogen-distorted ");

	if(mtmp->data->mlet == S_GHOST) {
		register char *gn = (char *) mtmp->mextra;
		if (!Hallucination && !mtmp->minvis && *gn) {
			Strcat(buf, "the ");
			killer_format = KILLED_BY;
		}
		Sprintf(eos(buf), (*gn ? "ghost of %s" : "ghost%s"), gn);
	} else if(mtmp->isshk) {
		Sprintf(eos(buf), "%s %s, the shopkeeper",
			(ESHK(mtmp)->ismale ? "Mr." : "Ms."), shkname(mtmp));
		killer_format = KILLED_BY;
	} else Strcat(buf, mtmp->data->mname);
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
		done(STONING);
	else
		done(DIED);
	return;
}

/*VARARGS1*/
boolean panicking;
extern boolean hu;	/* from save.c */

void
panic VA_DECL(const char *, str)
	VA_START(str);
	VA_INIT(str, char *);

	if(panicking++)
#ifdef AMIGA
	    Abort(0);
#else
# ifdef SYSV
	    (void)
# endif
		abort();    /* avoid loops - this should never happen*/
				    /* was exit(1) */
#endif
	home(); cls();
	(void) puts(" Suddenly, the dungeon collapses.");
#if defined(WIZARD) && !defined(MSDOS)
	if(!wizard) {
	    pline("Report error to %s and it may be possible to rebuild.",WIZARD);
	    more();
	}
#ifdef VMS
	{
		char *sem = rindex(SAVEF, ';');

		if (sem)
			*sem = '\0';
	}
	Strcat(SAVEF, ".e;1");
#else
	Strcat(SAVEF, ".e");
#endif
	hu = FALSE;
	(void) dosave0();
#endif
#ifdef MACOS
	puts(" ERROR:  ");
#else
	(void) fputs(" ERROR:  ", stdout);
#endif
#ifdef LATTICE
	{
	char pbuf[100];
	vsprintf(pbuf,str,VA_ARGS);
	(void)puts(pbuf);
	}
#else
	Vprintf(str,VA_ARGS);
#endif
	more();				/* contains a fflush() */
#if defined(WIZARD) && (defined(UNIX) || defined(VMS) || defined(LATTICE))
	if (wizard)
# ifdef AMIGA
		Abort(0);
# else
#  ifdef SYSV
		(void)
#  endif
		    abort();	/* generate core dump */
# endif
#endif
	VA_END();
	done(PANICKED);
}

static void
disclose(how,taken)
int how;
boolean taken;
{
#ifdef MACOS
	int see_c;
	char mac_buf[80];
#endif
	char	c;

	if(invent) {
#ifndef MACOS
	    if(taken)
		pline("Do you want to see what you had when you %s? ",
			(how == QUIT) ? "quit" : "died");
	    else
		pline("Do you want your possessions identified? ");
	    if ((c = yn_function(ynqchars,'y')) == 'y') {
#else
		{
			extern short macflags;
		
			/* stop user from using menus, etc. */
			macflags &= ~(fDoNonKeyEvt | fDoUpdate);
		}
	    if(taken)
		Sprintf(mac_buf, "Do you want to see what you had when you %s? ",
			(how == QUIT) ? "quit" : "died");
	    else
		Sprintf(mac_buf, "Do you want your possessions identified? ");
		if(!flags.silent) SysBeep(1);
	    if ((c = "qqynq"[UseMacAlertText(129,mac_buf)+1]) == 'y') {
#endif
	    /* New dump format by maartenj@cs.vu.nl */
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
		/* paybill has already given the inventory locations 
		 * in the shop and put it on the main object list
		 */
		struct obj *obj;

		for(obj = invent; obj; obj = obj->nobj) {
		    obj->owornmask = 0;
		    if(rn2(5)) curse(obj);
		}
	        invent = (struct obj *) 0;
	    }
	}

	if (!done_stopprint) {
#ifdef MACOS
		c = "qqynq"[UseMacAlertText(129, "Do you want to see your instrinsics ?")+1];
#else
	    pline("Do you want to see your intrinsics? ");
	    c = yn_function(ynqchars, 'y');
#endif
	    if (c == 'y') enlightenment();
	    if (c == 'q') done_stopprint++;
	}

}

/* Be careful not to call panic from here! */
void
done(how)
int how;
{
	struct permonst *upmon;
	boolean taken;
	char kilbuf[BUFSZ], buf2[BUFSZ];
	/* kilbuf: used to copy killer in case it comes from something like
	 *	xname(), which would otherwise get overwritten when we call
	 *	xname() when listing possessions
	 * buf2: same as player name, except it is capitalized
	 */
#ifdef ENDGAME
	if (how == ASCENDED)
		killer_format = NO_KILLER_PREFIX;
#endif
	/* Avoid killed by "a" burning or "a" starvation */
	if (!killer && (how == STARVING || how == BURNING))
		killer_format = KILLED_BY;
	Strcpy(kilbuf, (!killer || how >= PANICKED ? deaths[how] : killer));
	killer = kilbuf;
#ifdef WIZARD
	if (wizard && how == TRICKED) {
		You("are a very tricky wizard, it seems.");
		return;
	}
#endif
	if(Lifesaved && how <= GENOCIDED) {
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
		if (u.uhunger < 500) u.uhunger = 500;
		nomovemsg = "You survived that attempt on your life.";
		curs_on_u();
		flags.move = 0;
		if(multi > 0) multi = 0; else multi = -1;
		flags.botl = 1;
		u.ugrave_arise = -1;
		if (how == GENOCIDED)
			pline("Unfortunately you are still genocided...");
		else {
			killer = 0;
			return;
		}
	}
#if defined(WIZARD) || defined(EXPLORE_MODE)
	if((wizard || discover) && how <= GENOCIDED) {
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
		killer = 0;
		return;
	}
#endif /* WIZARD || EXPLORE_MODE */
die:
#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done_intr);
# if defined(UNIX) || defined(VMS)
	(void) signal(SIGQUIT, (SIG_RET_TYPE) done_intr);
	(void) signal(SIGHUP, (SIG_RET_TYPE) done_hangup);
# endif
#endif /* NO_SIGNAL /* */
	upmon = player_mon();
	if(u.ugrave_arise > -1) /* create no corpse */ ;
	else if(how == STONED)
		(mk_named_object(STATUE, upmon, u.ux, u.uy, plname,
					strlen(plname)))->spe = 0;
/*
 * If you're burned to a crisp, why leave a corpse?
 */
	else if (how != BURNING)
		(void) mk_named_object(CORPSE, upmon, u.ux, u.uy, plname,
							strlen(plname));

	if (how == QUIT) {
		killer_format = NO_KILLER_PREFIX;
		if (u.uhp < 1) {
			how = DIED;
/* note that killer is pointing at kilbuf */
			Strcpy(kilbuf, "quit while already on Charon's boat");
		}
	}
	if (how == ESCAPED || how == PANICKED)
		killer_format = NO_KILLER_PREFIX;

	/* paybill() must be called unconditionally, or strange things will
	 * happen to bones levels */
	taken = paybill();
	paygd();
	clearlocks();
	if(flags.toplin == 1) more();

	disclose(how,taken);

	if(how < GENOCIDED) {
#ifdef WIZARD
	    if(wizard) {
#ifdef MACOS
		if(!flags.silent) SysBeep(20);
		if(UseMacAlertText(128, "Save bones ?") == 1) savebones();
#else
		pline("Save bones? ");
		if(yn() == 'y') savebones();
#endif
	    }  else
#endif
		if (how != PANICKED && how !=TRICKED)
			savebones();
	    if(!flags.notombstone) outrip();
	}

/* changing kilbuf really changes killer. we do it this way because
   killer is declared a (const char *)
*/
	if(u.uhave_amulet) Strcat(kilbuf, " (with the Amulet)");
	settty(NULL);	/* does a clear_screen() */
	Strcpy(buf2, plname);
	if('a' <= buf2[0] && buf2[0] <= 'z') buf2[0] += 'A'-'a';
	if(!done_stopprint)
	    Printf("Goodbye %s the %s...\n\n", buf2,
#ifdef ENDGAME
		   how != ASCENDED ? (const char *)pl_character :
		   flags.female ? (const char *)"Demigoddess" : 
			(const char *)"Demigod");
#else
		   pl_character);
#endif
	{ long int tmp;
	  tmp = u.ugold - u.ugold0;
	  if(tmp < 0)
		tmp = 0;
	  if(how < PANICKED)
		tmp -= tmp/10;
	  u.urexp += tmp;
	  u.urexp += 50 * maxdlevel;
	  if(maxdlevel > 20)
		u.urexp += 1000*((maxdlevel > 30) ? 10 : maxdlevel - 20);
#ifdef ENDGAME
	  if(how == ASCENDED) u.urexp *= 2;
#endif
	}
	if(how == ESCAPED
#ifdef ENDGAME
			|| how == ASCENDED
#endif
					) {
		register struct monst *mtmp;
		register struct obj *otmp, *otmp2, *prevobj;
		struct obj *jewels = (struct obj *)0;
		long i;
		register unsigned int worthlessct = 0;
#if defined(LINT) || defined(__GNULINT__)
		prevobj = (struct obj *)0;
#endif

		/* put items that count into jewels chain
		 * rewriting the fcobj and invent chains here is safe,
		 * as they'll never be used again
		 */
		for(otmp = fcobj; otmp; otmp = otmp2) {
			otmp2 = otmp->nobj;
			if(carried(otmp->cobj)
					&& ((otmp->olet == GEM_SYM &&
					     otmp->otyp < LUCKSTONE)
					    || otmp->olet == AMULET_SYM)) {
				if(otmp == fcobj)
					fcobj = otmp->nobj;
				else
					prevobj->nobj = otmp->nobj;
				otmp->nobj = jewels;
				jewels = otmp;
			} else
				prevobj = otmp;
		}
		for(otmp = invent; otmp; otmp = otmp2) {
			otmp2 = otmp->nobj;
			if((otmp->olet == GEM_SYM && otmp->otyp < LUCKSTONE)
					    || otmp->olet == AMULET_SYM) {
				if(otmp == invent)
					invent = otmp->nobj;
				else
					prevobj->nobj = otmp->nobj;
				otmp->nobj = jewels;
				jewels = otmp;
			} else
				prevobj = otmp;
		}

		/* add points for jewels */
		for(otmp = jewels; otmp; otmp = otmp->nobj) {
			if(otmp->olet == GEM_SYM)
				u.urexp += (long) otmp->quan *
					    objects[otmp->otyp].g_val;
			else 	/* amulet */
				u.urexp += (otmp->spe < 0) ? 2 :
					otmp->otyp == AMULET_OF_YENDOR ?
							5000 : 500;
		}

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
#ifdef ENDGAME
		    Printf("\n%s with %ld points,\n",
			how==ASCENDED ? "went to your reward"
				: "escaped from the dungeon",
#else
		    Printf("\nescaped from the dungeon with %ld points,\n",
#endif
			u.urexp);
		} else
		if(!done_stopprint)
#ifdef ENDGAME
		  Printf("You %s with %ld points,\n",
			how==ASCENDED ? "went to your reward"
				: "escaped from the dungeon",
#else
		  Printf("You escaped from the dungeon with %ld points,\n",
#endif
		    u.urexp);

		/* print jewels chain here */
		for(otmp = jewels; otmp; otmp = otmp->nobj) {
			makeknown(otmp->otyp);
			if(otmp->olet == GEM_SYM && otmp->otyp < LUCKSTONE) {
				i = (long) otmp->quan *
					objects[otmp->otyp].g_val;
				if(i == 0) {
					worthlessct += otmp->quan;
					continue;
				}
				Printf("        %s (worth %ld zorkmids),\n",
				    doname(otmp), i);
			} else {		/* amulet */
				otmp->known = 1;
				i = (otmp->spe < 0) ? 2 :
					otmp->otyp == AMULET_OF_YENDOR ?
							5000 : 500;
				Printf("        %s (worth %ld zorkmids),\n",
				    doname(otmp), i);
			}
		}
		if(worthlessct)
		  Printf("        %u worthless piece%s of colored glass,\n",
			worthlessct, plur((long)worthlessct));
	} else
		if(!done_stopprint) {
		    Printf("You %s ", ends[how]);
#ifdef ENDGAME
		    if (how != ASCENDED) {
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
	    u.ulevel, u.uhpmax, ends[how]);
#if (defined(WIZARD) || defined(EXPLORE_MODE))
# ifndef LOGFILE
	if(wizard || discover)
		Printf("\nSince you were in %s mode, the score list \
will not be checked.\n", wizard ? "wizard" : "discover");
	else
# endif
#endif
	{
		if (!done_stopprint) {
			getret();
			cls();
		}
/* "So when I die, the first thing I will see in Heaven is a score list?" */
		topten(how);
	}
	if(done_stopprint) Printf("\n\n");
#if defined(APOLLO) || defined(MACOS)
	getret();
#endif
	exit(0);
}

void
clearlocks(){
#if defined(DGK)
	eraseall(levels, alllevels);
	if (ramdisk)
		eraseall(permbones, alllevels);
#else
# if defined(UNIX) || defined(MSDOS) || defined(VMS) || defined(MACOS)
	register int x;
#  if defined(UNIX) || defined(VMS)
	(void) signal(SIGHUP,SIG_IGN);
#  endif
	for(x = maxdlevel; x >= 0; x--) {
		glo(x);
		(void) unlink(lock);	/* not all levels need be present */
	}
# endif
#endif
}

#ifdef NOSAVEONHANGUP
int
hangup()
{
	(void) signal(SIGINT, SIG_IGN);
	clearlocks();
# ifndef VMS
	exit(1);
# endif
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
		if (!cnt) {
		    pline("The %s is empty.", xname(box));
		    more();
		} else cornline(2,"");
	    }
	}
}
