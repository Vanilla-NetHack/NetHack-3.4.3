/*	SCCS Id: @(#)end.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* end.c - version 1.0.3 */

#include <stdio.h>
#include <signal.h>
#include "hack.h"
#define	Sprintf	(void) sprintf
extern char plname[], pl_character[];

xchar maxdlevel = 1;
int done_stopprint;
int done_hup;


done1()
{
	(void) signal(SIGINT,SIG_IGN);
#if defined(WIZARD) && defined(UNIX) 
	if(wizard) {
	    pline("Dump core?");
	    if(readchar() == 'y') {
		(void) signal(SIGINT,done1);
		abort();
	    }
	}
#endif
	pline("Really quit?");
	if(readchar() != 'y') {
		(void) signal(SIGINT,done1);
		clrlin();
		(void) fflush(stdout);
		if(multi > 0) nomul(0);
		return(0);
	}
	done("quit");
	/* NOTREACHED */
}

done_intr(){
	done_stopprint++;
	(void) signal(SIGINT, SIG_IGN);
#ifdef UNIX
	(void) signal(SIGQUIT, SIG_IGN);
#endif
}

#ifdef UNIX
done_hangup(){
	done_hup++;
	(void) signal(SIGHUP, SIG_IGN);
	done_intr();
}
#endif

done_in_by(mtmp) register struct monst *mtmp; {
static char buf[BUFSZ];
	pline("You die ...");
	if(mtmp->data->mlet == ' '){
		Sprintf(buf, "the ghost of %s", (char *) mtmp->mextra);
		killer = buf;
	} else if(mtmp->mnamelth) {
		Sprintf(buf, "%s called %s",
			mtmp->data->mname, NAME(mtmp));
		killer = buf;
	} else if(mtmp->minvis) {
		Sprintf(buf, "invisible %s", mtmp->data->mname);
		killer = buf;
	} else killer = mtmp->data->mname;
	done("died");
}

/*VARARGS1*/
boolean panicking;

panic(str,a1,a2,a3,a4,a5,a6)
char *str;
{
	if(panicking++) abort();    /* avoid loops - this should never happen*/
				    /* was exit(1) */
	home(); cls();
	puts(" Suddenly, the dungeon collapses.");
	fputs(" ERROR:  ", stdout);
	printf(str,a1,a2,a3,a4,a5,a6);
	more();				/* contains a fflush() */
#ifdef WIZARD
# ifdef UNIX
	if (wizard)	abort();	/* generate core dump */
# endif
#endif
	done("panicked");
}

/* called with arg "died", "drowned", "escaped", "quit", "choked", "panicked",
   "burned", "starved" or "tricked" */
/* Be careful not to call panic from here! */
done(st1)
register char *st1;
{
#ifdef DIAGS
	char	c;
#endif
#ifdef WIZARD
	extern char	*nomovemsg;

	if(wizard && index("bcds", *st1)){
		char buf[BUFSZ];
		pline("Die? ");
		getlin(buf);
		if(index("yY",buf[0])) goto die;
		u.uswldtim = 0;
		if(u.uhpmax < 0) u.uhpmax = 100;	/* arbitrary */
		u.uhp = u.uhpmax;
		pline("Ok, so you don't die.");
		nomovemsg = "You survived that attempt on your life.";
		flags.move = 0;
		if(multi > 0) multi = 0; else multi = -1;
		flags.botl = 1;
		return;
	}
#endif /* WIZARD /**/
die:
	(void) signal(SIGINT, done_intr);
#ifdef UNIX
	(void) signal(SIGQUIT, done_intr);
	(void) signal(SIGHUP, done_hangup);
#endif
	if(*st1 == 'q' && u.uhp < 1){
		st1 = "died";
		killer = "quit while already on Charon's boat";
	}
	if(*st1 == 's') killer = "starvation"; else
	if(*st1 == 'd' && st1[1] == 'r') killer = "drowning"; else
	if(*st1 == 'p') killer = "panic"; else
	if(*st1 == 't') killer = "trickery"; else
	if(!index("bcd", *st1)) killer = st1;
	paybill();
	clearlocks();
	if(flags.toplin == 1) more();
#ifdef DIAGS
	pline("Do you want to have your possessions identified? [Yynq] ");
	if ((c = readchar()) == 'y' || c == 'Y') {
	    struct obj *obj;
	    for(obj = invent; obj && !done_stopprint; obj = obj->nobj)
		identify(obj);
	    pline("That's all, folks!"), more();
	}
	if (c == 'q' || c == 'Y')  done_stopprint++;
#endif
	if(index("bcds", *st1)){
#ifdef WIZARD
	    if(wizard) {
		char buf[BUFSZ];
		pline("Save bones? ");
		getlin(buf);
		if(buf[0] == 'y') savebones();
	    }  else
#endif
		savebones();
		if(!flags.notombstone) outrip();
	}
	if(*st1 == 'c') killer = st1;		/* after outrip() */
	settty((char *) 0);	/* does a clear_screen() */
	if(!done_stopprint)
		printf("Goodbye %s %s...\n\n", pl_character, plname);
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
	}
	if(*st1 == 'e') {
		extern struct monst *mydogs;
		register struct monst *mtmp;
		register struct obj *otmp;
#ifdef DGKMOD
		long i;
#else
		register int i;
#endif
		register unsigned worthlessct = 0;
		boolean has_amulet = FALSE;

		killer = st1;
		keepdogs();
		mtmp = mydogs;
		if(mtmp) {
			if(!done_stopprint) printf("You");
			while(mtmp) {
				if(!done_stopprint)
					printf(" and %s", monnam(mtmp));
				if(mtmp->mtame)
					u.urexp += mtmp->mhp;
				mtmp = mtmp->nmon;
			}
			if(!done_stopprint)
		    printf("\nescaped from the dungeon with %ld points,\n",
			u.urexp);
		} else
		if(!done_stopprint)
		  printf("You escaped from the dungeon with %ld points,\n",
		    u.urexp);
		for(otmp = invent; otmp; otmp = otmp->nobj) {
			if(otmp->olet == GEM_SYM){
				objects[otmp->otyp].oc_name_known = 1;
#ifdef DGKMOD
				i = (long) otmp->quan *
					objects[otmp->otyp].g_val;
#else
				i = otmp->quan*objects[otmp->otyp].g_val;
#endif
				if(i == 0) {
					worthlessct += otmp->quan;
					continue;
				}
				u.urexp += i;
#ifndef DGKMOD
				if(!done_stopprint)
				  printf("\t%s (worth %d Zorkmids),\n",
#else
				printf("        %s (worth %ld Zorkmids),\n",
#endif
				    doname(otmp), i);
			} else if(otmp->olet == AMULET_SYM) {
				otmp->known = 1;
				i = (otmp->spe < 0) ? 2 : 5000;
				u.urexp += i;
#ifndef DGKMOD
				if(!done_stopprint)
				  printf("\t%s (worth %d Zorkmids),\n",
#else
				printf("        %s (worth %d Zorkmids),\n",
#endif
				    doname(otmp), i);
				if(otmp->spe >= 0) {
					has_amulet = TRUE;
					killer = "escaped (with amulet)";
				}
			}
		}
		if(worthlessct)
#ifndef DGKMOD
		  if(!done_stopprint)
		    printf("\t%u worthless piece%s of coloured glass,\n",
#else
		  printf("        %u worthless piece%s of coloured glass,\n",
#endif
			worthlessct, plur(worthlessct));
		if(has_amulet) u.urexp *= 2;
	} else
		if(!done_stopprint)
		  printf("You %s on dungeon level %d with %ld points,\n",
		    st1, dlevel, u.urexp);
	if(!done_stopprint)
	  printf("and %ld piece%s of gold, after %ld move%s.\n",
	    u.ugold, plur(u.ugold), moves, plur(moves));
	if(!done_stopprint)
  printf("You were level %u with a maximum of %d hit points when you %s.\n",
	    u.ulevel, u.uhpmax, st1);
	if(*st1 == 'e' && !done_stopprint){
		getret();	/* all those pieces of coloured glass ... */
		cls();
	}
#ifdef WIZARD
	if(!wizard)
#endif
		topten();
	if(done_stopprint) printf("\n\n");
#ifdef APOLLO
	getret();
#endif
	exit(0);
}
clearlocks(){
#ifdef DGK
	eraseall(levels, alllevels);
	if (ramdisk)
		eraseall(permbones, alllevels);
#else
# ifdef UNIX
register x;
	(void) signal(SIGHUP,SIG_IGN);
	for(x = maxdlevel; x >= 0; x--) {
		glo(x);
		(void) unlink(lock);	/* not all levels need be present */
	}
# endif
#endif
}

#ifdef NOSAVEONHANGUP
hangup()
{
	(void) signal(SIGINT, SIG_IGN);
	clearlocks();
	exit(1);
}
#endif

/* it is the callers responsibility to check that there is room for c */
charcat(s,c) register char *s, c; {
	while(*s) s++;
	*s++ = c;
	*s = 0;
}
