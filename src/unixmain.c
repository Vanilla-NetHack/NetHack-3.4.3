/*	SCCS Id: @(#)unixmain.c	3.0	89/01/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* main.c - (Unix) version */

#include <signal.h>
#include <pwd.h>

#include "hack.h"

int hackpid = 0;				/* current pid */
int locknum = 0;				/* max num of players */
#ifdef DEF_PAGER
char *catmore = 0;				/* default pager */
#endif
char SAVEF[PL_NSIZ + 11] = "save/";	/* save/99999player */
char *hname = 0;		/* name of the game (argv[0] of call) */
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */

int (*occupation)() = DUMMY;
int (*afternmv)() = DUMMY;
#ifdef CHDIR
static void chdirx();
#endif /* CHDIR */
static void whoami(), newgame();

main(argc,argv)
int argc;
char *argv[];
{
	struct passwd *pw;
	extern struct passwd *getpwuid();
	extern int x_maze_max, y_maze_max;
	register int fd;
#ifdef CHDIR
	register char *dir;
#endif
#ifdef COMPRESS
	char	cmd[80], old[80];
#endif
	hname = argv[0];
	hackpid = getpid();
	(void) umask(0);

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
	/* Set the default values of the presentation characters */
	(void) memcpy((char *) &showsyms, 
		(char *) &defsyms, sizeof(struct symbols));
	initoptions();
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
	cls();
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = FAR;	/* prevent nscr() */
	(void) signal(SIGHUP, (SIG_RET_TYPE) hangup);

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
#if defined(WIZARD) || defined(EXPLORE_MODE)
		case 'D':
		case 'X':
			pw = getpwuid(getuid());
# ifdef WIZARD
			if(!strcmp(pw->pw_name, WIZARD))
				wizard = TRUE;
# endif
# if defined(WIZARD) && defined(EXPLORE_MODE)
			else
# endif
# ifdef EXPLORE_MODE
				discover = TRUE;
# endif
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
				Printf("Player name expected after -u\n");
			break;
		default:
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);

			/* Printf("Unknown option: %s\n", *argv); */
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
	if(wizard) Strcpy(plname, "wizard"); else
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
			Sprintf(lock, "%d%s", getuid(), plname);
		getlock();	/* sets lock if locknum != 0 */
#ifdef WIZARD
	} else
		Sprintf(lock, "%d%s", getuid(), plname);
#endif /* WIZARD /**/
	setftty();

	/* 
	 * Initialisation of the boundaries of the mazes
	 * Both boundaries have to be even.
	 */
	 
	x_maze_max = COLNO-1;
	if (x_maze_max % 2) 
		x_maze_max--;
	y_maze_max = ROWNO-1;
	if (y_maze_max % 2) 
		y_maze_max--;

	/* initialize static monster strength array */
	init_monstr();

	Sprintf(SAVEF, "save/%d%s", getuid(), plname);
	regularize(SAVEF+5);		/* avoid . or / in name */
#ifdef COMPRESS
	Strcpy(old,SAVEF);
	Strcat(SAVEF,".Z");
	if((fd = open(SAVEF,0)) >= 0) {
 	    (void) close(fd);
	    Strcpy(cmd, COMPRESS);
	    Strcat(cmd, " -d ");	/* uncompress */
# ifdef COMPRESS_OPTIONS
	    Strcat(cmd, COMPRESS_OPTIONS);
	    Strcat(cmd, " ");
# endif
	    Strcat(cmd,SAVEF);
	    (void) system(cmd);
	}
	Strcpy(SAVEF,old);
#endif
	if((fd = open(SAVEF,0)) >= 0 &&
	   (uptodate(fd) || unlink(SAVEF) == 666)) {
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
		pline("Restoring old save file...");
		(void) fflush(stdout);
		if(!dorecover(fd))
			goto not_recovered;
		pline("Hello %s, welcome to NetHack!", plname);
		/* get shopkeeper set properly if restore is in shop */
		(void) inshop();
#ifdef EXPLORE_MODE
		if (discover) {
			You("are in non-scoring discovery mode.");
			pline("Do you want to keep the save file? ");
			if(yn() == 'n')
				(void) unlink(SAVEF);
		}
#endif
		flags.move = 0;
	} else {
not_recovered:
		newgame();
		/* give welcome message before pickup messages */
		pline("Hello %s, welcome to NetHack!", plname);
#ifdef EXPLORE_MODE
		if (discover)
			You("are in non-scoring discovery mode.");
#endif
		flags.move = 0;
		set_wear();
		pickup(1);
		read_engr_at(u.ux,u.uy);
	}

	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		You("are lucky!  Full moon tonight.");
		if(!u.uluck) change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful!  New moon tonight.");
	}

	initrack();

	for(;;) {
		if(flags.move) {	/* actual time passed */

#ifdef SOUNDS
			dosounds();
#endif
			settrack();

			if(moves%2 == 0 ||
			  (!(Fast & ~INTRINSIC) && (!Fast || rn2(3)))) {
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
#ifdef THEOLOGY
			if (u.ublesscnt)  u.ublesscnt--;
#endif
			if(flags.time) flags.botl = 1;
#ifdef POLYSELF
			if(u.mtimedone)
			    if(u.mh < 1) rehumanize();
			else
#endif
			    if(u.uhp < 1) {
				You("die...");
				done("died");
			    }
#ifdef POLYSELF
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
				    int heal;

				    if(HRegeneration || !(moves%3)) {
					flags.botl = 1;
					if (ACURR(A_CON) <= 12) heal = 1;
					else heal = rnd((int) ACURR(A_CON)-12);
					if (heal > u.ulevel-9) heal = u.ulevel-9;
					u.uhp += heal;
					if(u.uhp > u.uhpmax)
					    u.uhp = u.uhpmax;
				    }
				} else if(HRegeneration ||
				      (!(moves%((MAXULEV+12)/(u.ulevel+2)+1)))) {
					flags.botl = 1;
					u.uhp++;
				}
			}
#ifdef SPELLS
			if ((u.uen<u.uenmax) && (!(moves%(19-ACURR(A_INT)/2)))) {
				u.uen += rn2((int)ACURR(A_WIS)/5 + 1) + 1;
				if (u.uen > u.uenmax)  u.uen = u.uenmax;
				flags.botl = 1;
			}
#endif
			if(Teleportation && !rn2(85)) tele();
#ifdef POLYSELF
			if(Polymorph && !rn2(100)) polyself();
			if(u.ulycn >= 0 && !rn2(80 - (20 * night())))
				you_were();
#endif
			if(Searching && multi >= 0) (void) dosearch0(1);
			hatch_eggs();
			gethungry();
			invault();
			amulet();
#ifdef HARD
			if (!rn2(40+(int)(ACURR(A_DEX)*3))) u_wipe_engr(rnd(3));
			if (u.udemigod) {

				if(u.udg_cnt) u.udg_cnt--;
				if(!u.udg_cnt) {

					intervene();
					u.udg_cnt = rn1(200, 50);
				}
			}
#endif
			restore_attrib();
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
		if(!flags.mv || Blind)
		{
			seeobjs();
			seemons();
			seeglds();
			nscr();
		}
		if(flags.botl || flags.botlx) bot();

		flags.move = 1;

		if(multi >= 0 && occupation) {

			if(monster_nearby())
				stop_occupation();
			else if ((*occupation)() == 0)
				occupation = 0;
			continue;
		}

		if((u.uhave_amulet || Clairvoyant) && 
#ifdef ENDGAME
			dlevel != ENDLEVEL &&
#endif
			!(moves%15) && !rn2(2)) do_vicinity_map();

		u.umoved = FALSE;
		if(multi > 0) {
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
			rhack(NULL);
		}
		if(multi && multi%7 == 0)
			(void) fflush(stdout);
	}
}

void
glo(foo)
register int foo;
{
	/* construct the string  xlock.n  */
	register char *tf;

	tf = lock;
	while(*tf && *tf != '.') tf++;
	Sprintf(tf, ".%d", foo);
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
void
askname(){
register int c,ct;
	Printf("\nWho are you? ");
	(void) fflush(stdout);
	ct = 0;
	while((c = Getchar()) != '\n'){
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
void
impossible(s,x1,x2)
register char *s, *x1, *x2;
{
	pline(s,x1,x2);
	pline("Program in disorder - perhaps you'd better Quit.");
}

#ifdef CHDIR
static void
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
		(void) setgid(getgid());
		(void) setuid(getuid());		/* Ron Wessels */
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
	    register int fd;

	    if(dir == NULL)
		dir = ".";
	    if((fd = open(RECORD, 2)) < 0) {
		Printf("Warning: cannot write %s/%s", dir, RECORD);
		getret();
	    } else
		(void) close(fd);
	}
}
#endif /* CHDIR /**/

void
stop_occupation()
{
	if(occupation) {
		You("stop %s.", occtxt);
		occupation = 0;
#ifdef REDO
		multi = 0;
		pushch(0);		
#endif
	}
}

static void
whoami() {
	/*
	 * Who am i? Algorithm: 1. Use name as specified in NETHACKOPTIONS
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

	if(!*plname && (s = getenv("USER")))
		(void) strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = getenv("LOGNAME")))
		(void) strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = getlogin()))
		(void) strncpy(plname, s, sizeof(plname)-1);
}

static void
newgame() {
	fobj = fcobj = invent = 0;
	fmon = fallen_down = 0;
	ftrap = 0;
	fgold = 0;
	flags.ident = 1;

	init_objects();
	u_init();

	(void) signal(SIGINT, (SIG_RET_TYPE) done1);

	mklev();
	u.ux = xupstair;
	u.uy = yupstair;
	(void) inshop();

	setsee();
	flags.botlx = 1;

	/* Move the monster from under you or else
	 * makedog() will fail when it calls makemon().
	 * 			- ucsfcgl!kneller
	 */
	if(levl[u.ux][u.uy].mmask) mnexto(m_at(u.ux, u.uy));

	(void) makedog();
	seemons();
#ifdef NEWS
	if(flags.nonews || !readnews())
		/* after reading news we did docrt() already */
#endif
		docrt();

	return;
}
