/*	SCCS Id: @(#)unixmain.c	2.3	88/01/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* main.c - (Unix) version */

#include <stdio.h>
#include <signal.h>
#include "hack.h"

#ifdef QUEST
#define	gamename	"NetQuest"
#else
#define	gamename	"NetHack"
#endif

extern char *getlogin(), *getenv();
extern char plname[PL_NSIZ], pl_character[PL_CSIZ];

int (*afternmv)();
int (*occupation)();

int done1();
int hangup();

int hackpid;				/* current pid */
int locknum;				/* max num of players */
#ifdef DEF_PAGER
char *catmore;				/* default pager */
#endif
char SAVEF[PL_NSIZ + 11] = "save/";	/* save/99999player */
char *hname;		/* name of the game (argv[0] of call) */
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */

extern char *nomovemsg;
extern long wailmsg;

main(argc,argv)
int argc;
char *argv[];
{
	register int fd;
#ifdef CHDIR
	register char *dir;
#endif

	hname = argv[0];
	hackpid = getpid();

#ifdef CHDIR			/* otherwise no chdir() */
	/*
	 * See if we must change directory to the playground.
	 * (Perhaps hack runs suid and playground is inaccessible
	 *  for the player.)
	 * The environment variable HACKDIR is overridden by a
	 *  -d command line option (must be the first option given)
	 */

	dir = getenv("HACKDIR");
	if(argc > 1 && !strncmp(argv[1], "-d", 2)) {
		argc--;
		argv++;
		dir = argv[0]+2;
		if(*dir == '=' || *dir == ':') dir++;
		if(!*dir && argc > 1) {
			argc--;
			argv++;
			dir = argv[0];
		}
		if(!*dir)
		    error("Flag -d must be followed by a directory name.");
	}
#endif /* CHDIR /**/
#ifdef GRAPHICS
	/* Set the default values of the presentation characters */
	memcpy((char *) &showsyms, (char *) &defsyms, sizeof(struct symbols));
#endif
#ifdef HACKOPTIONS
	initoptions();
#endif
	whoami();
	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	if(argc > 1 && !strncmp(argv[1], "-s", 2)) {
#ifdef CHDIR
		chdirx(dir,0);
#endif
		prscore(argc, argv);
		exit(0);
	}

	/*
	 * It seems he really wants to play.
	 * Remember tty modes, to be restored on exit.
	 */
	gettty();
	setbuf(stdout,obuf);
	setrandom();
	startup();
	init_corpses();	/* initialize optional corpse names */
	cls();
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = FAR;	/* prevent nscr() */
	(void) signal(SIGHUP, hangup);

	/*
	 * Find the creation date of this game,
	 * so as to avoid restoring outdated savefiles.
	 */
	gethdate(hname);

	/*
	 * We cannot do chdir earlier, otherwise gethdate will fail.
	 */
#ifdef CHDIR
	chdirx(dir,1);
#endif

	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
#ifdef WIZARD
		case 'D':
			if(!strcmp(getlogin(), WIZARD))
				wizard = TRUE;
			else {
				settty("Sorry, you can't operate in debug mode.\n");
				clearlocks();
				exit(0);
			}
			break;
#endif
#ifdef NEWS
		case 'n':
			flags.nonews = TRUE;
			break;
#endif
		case 'u':
			if(argv[0][2])
			  (void) strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
			  argc--;
			  argv++;
			  (void) strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				printf("Player name expected after -u\n");
			break;
		default:
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);

			/* printf("Unknown option: %s\n", *argv); */
		}
	}

	if(argc > 1)
		locknum = atoi(argv[1]);
#ifdef MAX_NR_OF_PLAYERS
	if(!locknum || locknum > MAX_NR_OF_PLAYERS)
		locknum = MAX_NR_OF_PLAYERS;
#endif
#ifdef DEF_PAGER
	if(!(catmore = getenv("HACKPAGER")) && !(catmore = getenv("PAGER")))
		catmore = DEF_PAGER;
#endif
#ifdef MAIL
	getmailstatus();
#endif
#ifdef WIZARD
	if(wizard) (void) strcpy(plname, "wizard"); else
#endif
	if(!*plname || !strncmp(plname, "player", 4)
		    || !strncmp(plname, "games", 4))
		askname();
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if(!wizard) {
#endif
		/*
		 * check for multiple games under the same name
		 * (if !locknum) or check max nr of players (otherwise)
		 */
		(void) signal(SIGQUIT,SIG_IGN);
		(void) signal(SIGINT,SIG_IGN);
		if(!locknum)
			(void) strcpy(lock,plname);
		getlock();	/* sets lock if locknum != 0 */
#ifdef WIZARD
	} else {
		register char *sfoo;
		extern char genocided[], fut_geno[];
		(void) strcpy(lock,plname);
		if(sfoo = getenv("MAGIC"))
			while(*sfoo) {
				switch(*sfoo++) {
				case 'n': (void) srand(*sfoo++);
					break;
				}
			}
		if(sfoo = getenv("GENOCIDED")){
			if(*sfoo == '!'){
				extern struct permonst mons[CMNUM+2];
				register struct permonst *pm = mons;
				register char *gp = genocided;

				while(pm < mons+CMNUM+2){
					if(!index(sfoo, pm->mlet))
						*gp++ = pm->mlet;
					pm++;
				}
				*gp = 0;
			} else
				(void) strcpy(genocided, sfoo);
			(void) strcpy(fut_geno, genocided);
		}
	}
#endif /* WIZARD /**/
	setftty();
	(void) sprintf(SAVEF, "save/%d%s", getuid(), plname);
	regularize(SAVEF+5);		/* avoid . or / in name */
	if((fd = open(SAVEF,0)) >= 0 &&
	   (uptodate(fd) || unlink(SAVEF) == 666)) {
		(void) signal(SIGINT,done1);
		pline("Restoring old save file...");
		(void) fflush(stdout);
		if(!dorecover(fd))
			goto not_recovered;
		pline("Hello %s%s, welcome to %s!", 
			(Badged) ? "Officer " : "", plname, gamename);
		flags.move = 0;
	} else {
not_recovered:
		newgame();
		/* give welcome message before pickup messages */
		pline("Hello %s, welcome to %s!", plname, gamename);
#ifdef WIZARD
		if (wizard && dlevel == 1)
# ifdef STOOGES
pline ("The wiz is at %d, the medusa is at %d, and the stooges are at %d",
			u.wiz_level, u.medusa_level, u.stooge_level);
# else
	            pline ("The wiz is at %d, and the medusa at %d",
			   u.wiz_level, u.medusa_level);
# endif
#endif
		pickup(1);
		read_engr_at(u.ux,u.uy);
		flags.move = 1;
	}

	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		pline("You are lucky! Full moon tonight.");
		if(!u.uluck) change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful! New moon tonight.");
	}

	initrack();

	for(;;) {
		if(flags.move) {	/* actual time passed */

			settrack();

			if(moves%2 == 0 ||
			  (!(Fast & ~INTRINSIC) && (!Fast || rn2(3)))) {
				extern struct monst *makemon();
				movemon();
#ifdef HARD
				if(!rn2(u.udemigod?25:(dlevel>30)?50:70))
#else
				if(!rn2(70))
#endif
				    (void) makemon((struct permonst *)0, 0, 0);
			}
			if(Glib) glibr();
			timeout();
			++moves;
#ifdef PRAYERS
			if (u.ublesscnt)  u.ublesscnt--;
#endif
			if(flags.time) flags.botl = 1;
#ifdef KAA
			if(u.mtimedone)
			    if(u.mh < 1) rehumanize();
			else
#endif
			    if(u.uhp < 1) {
				pline("You die...");
				done("died");
			    }
			if(u.uhp*10 < u.uhpmax && moves-wailmsg > 50){
			    wailmsg = moves;
#ifdef KAA
			    if(index("WEV", pl_character[0])) {
				if (u.uhp == 1)
				pline("%s is about to die.", pl_character);
				else
				pline("%s, your life force is running out.",
					pl_character);
			    } else {
#endif
				if(u.uhp == 1)
				pline("You hear the wailing of the Banshee...");
				else
				pline("You hear the howling of the CwnAnnwn...");
#ifdef KAA
			    }
#endif
			}
#ifdef KAA
			if (u.mtimedone) {
			    if (u.mh < u.mhmax) {
				if (Regeneration || !(moves%20)) {
					flags.botl = 1;
					u.mh++;
				}
			    }
			}
#endif
			if(u.uhp < u.uhpmax) {
				if(u.ulevel > 9) {
					if(HRegeneration || !(moves%3)) {
					    flags.botl = 1;
					    u.uhp += rnd((int) u.ulevel-9);
					    if(u.uhp > u.uhpmax)
						u.uhp = u.uhpmax;
					}
				} else if(HRegeneration ||
					(!(moves%(22-u.ulevel*2)))) {
					flags.botl = 1;
					u.uhp++;
				}
			}
#ifdef SPELLS
			if ((u.uen<u.uenmax) && (!(moves%(21-u.ulevel/2)))) {
				u.uen += rn2(u.ulevel/4 + 1) + 1;
				if (u.uen > u.uenmax)  u.uen = u.uenmax;
				flags.botl = 1;
			}
#endif
			if(Teleportation && !rn2(85)) tele();
#if defined(KAA) && defined(BVH)
			if(Polymorph && !rn2(100)) polyself();
#endif
			if(Searching && multi >= 0) (void) dosearch();
			gethungry();
			invault();
			amulet();
#ifdef HARD
			if (!rn2(50+(u.ulevel*3))) u_wipe_engr(rnd(3));
			if (u.udemigod) {

				u.udg_cnt--;
				if(u.udg_cnt <= 0) {

					intervene();
					u.udg_cnt = rn1(200, 50);
				}
			}
#endif
		}
		if(multi < 0) {
			if(!++multi){
				pline(nomovemsg ? nomovemsg :
					"You can move again.");
				nomovemsg = 0;
				if(afternmv) (*afternmv)();
				afternmv = 0;
			}
		}

		find_ac();
#ifndef QUEST
		if(!flags.mv || Blind)
#endif
		{
			seeobjs();
			seemons();
			nscr();
		}
#ifdef DGK
		if(flags.time) flags.botl = 1;
#endif
		if(flags.botl || flags.botlx) bot();

		flags.move = 1;

		if(multi >= 0 && occupation) {

			if (monster_nearby())
				stop_occupation();
			else if ((*occupation)() == 0)
				occupation = 0;
			continue;
		}

		if(multi > 0) {
#ifdef QUEST
			if(flags.run >= 4) finddir();
#endif
			lookaround();
			if(!multi) {	/* lookaround may clear multi */
				flags.move = 0;
				continue;
			}
			if(flags.mv) {
				if(multi < COLNO && !--multi)
					flags.mv = flags.run = 0;
				domove();
			} else {
				--multi;
				rhack(save_cm);
			}
		} else if(multi == 0) {
#ifdef MAIL
			ckmailstatus();
#endif
			rhack((char *) 0);
		}
		if(multi && multi%7 == 0)
			(void) fflush(stdout);
	}
}

glo(foo)
register foo;
{
	/* construct the string  xlock.n  */
	register char *tf;

	tf = lock;
	while(*tf && *tf != '.') tf++;
	(void) sprintf(tf, ".%d", foo);
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (-w implies wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
askname(){
register int c,ct;
	printf("\nWho are you? ");
	(void) fflush(stdout);
	ct = 0;
	while((c = getchar()) != '\n'){
		if(c == EOF) error("End of input\n");
		/* some people get confused when their erase char is not ^H */
		if(c == '\010') {
			if(ct) ct--;
			continue;
		}
		if(c != '-')
		if(c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
		if(ct < sizeof(plname)-1) plname[ct++] = c;
	}
	plname[ct] = 0;
	if(ct == 0) askname();
}

/*VARARGS1*/
impossible(s,x1,x2)
register char *s;
{
	pline(s,x1,x2);
	pline("Program in disorder - perhaps you'd better Quit.");
}

#ifdef CHDIR
static
chdirx(dir, wr)
char *dir;
boolean wr;
{

# ifdef SECURE
	if(dir					/* User specified directory? */
#  ifdef HACKDIR
	       && strcmp(dir, HACKDIR)		/* and not the default? */
#  endif
		) {
		(void) setuid(getuid());		/* Ron Wessels */
		(void) setgid(getgid());
	}
# endif

# ifdef HACKDIR
	if(dir == NULL)
		dir = HACKDIR;
# endif

	if(dir && chdir(dir) < 0) {
		perror(dir);
		error("Cannot chdir to %s.", dir);
	}

	/* warn the player if he cannot write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access systemcall is worthless */
	if(wr) {
	    register fd;

	    if(dir == NULL)
		dir = ".";
	    if((fd = open(RECORD, 2)) < 0) {
		printf("Warning: cannot write %s/%s", dir, RECORD);
		getret();
	    } else
		(void) close(fd);
	}
}
#endif /* CHDIR /**/

stop_occupation()
{
	extern void pushch();

	if(occupation) {
		pline("You stop %s.", occtxt);
		occupation = 0;
#ifdef REDO
		multi = 0;
		pushch(0);		
#endif
	}
}

whoami() {
	/*
	 * Who am i? Algorithm: 1. Use name as specified in HACKOPTIONS
	 *			2. Use $USER or $LOGNAME	(if 1. fails)
	 *			3. Use getlogin()		(if 2. fails)
	 * The resulting name is overridden by command line options.
	 * If everything fails, or if the resulting name is some generic
	 * account like "games", "play", "player", "hack" then eventually
	 * we'll ask him.
	 * Note that we trust him here; it is possible to play under
	 * somebody else's name.
	 */
	register char *s;

#ifndef DGKMOD
	initoptions();
#endif
	if(!*plname && (s = getenv("USER")))
		(void) strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = getenv("LOGNAME")))
		(void) strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = getlogin()))
		(void) strncpy(plname, s, sizeof(plname)-1);
}

newgame() {
	extern struct monst *makedog();

	fobj = fcobj = invent = 0;
	fmon = fallen_down = 0;
	ftrap = 0;
	fgold = 0;
	flags.ident = 1;
	init_objects();
	u_init();

	(void) signal(SIGINT,done1);
	mklev();
	u.ux = xupstair;
	u.uy = yupstair;
	(void) inshop();
	setsee();
	flags.botlx = 1;
	{
		register struct monst *mtmp;

		/* Move the monster from under you or else
		 * makedog() will fail when it calls makemon().
		 * 			- ucsfcgl!kneller
		 */
		if (mtmp = m_at(u.ux, u.uy))  mnexto(mtmp);
	}
	(void) makedog();
	seemons();
#ifdef NEWS
	if(flags.nonews || !readnews())
		/* after reading news we did docrt() already */
#endif
		docrt();
	return(0);
}

#ifdef GENIX
jhndist(x1,y1,x2,y2)
{
	int x,y;
	x=x1-x2;
	y=y1-y2;
	return (x*x + y*y);
}
#endif
