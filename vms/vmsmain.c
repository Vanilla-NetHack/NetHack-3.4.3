/*	SCCS Id: @(#)vmsmain.c	3.0	89/01/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* main.c - VMS NetHack */

#include "hack.h"

#include <signal.h>

char SAVEF[PL_NSIZ + 20];			/* [.save]<uic>player;1 */

const char *hname = 0;		/* name of the game (argv[0] of call) */
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */
int hackpid = 0;				/* current pid */
int locknum = 0;				/* max num of players */

static void whoami();
static void byebye();

int
main(argc,argv)
int argc;
char *argv[];
{
	extern int x_maze_max, y_maze_max;
	register int fd;
#ifdef CHDIR
	register char *dir;
#endif

	atexit(byebye);
	hname = argv[0];
	hackpid = getpid();
	(void) umask(0);

	/*
	 *  Remember tty modes, to be restored on exit.
	 *
	 *  gettty() must be called before startup()
	 *    due to ordering of LI/CO settings
	 *  startup() must be called before initoptions()
	 *    due to ordering of graphics settings
	 */
	gettty();
	setbuf(stdout,obuf);
	startup();
	initoptions();
	whoami();

#ifdef CHDIR			/* otherwise no chdir() */
	/*
	 * See if we must change directory to the playground.
	 * (Perhaps hack is installed with privs and playground is
	 *  inaccessible for the player.)
	 * The logical name HACKDIR is overridden by a
	 *  -d command line option (must be the first option given)
	 */
	dir = getenv("HACKDIR");
#endif
	if(argc > 1) {
#ifdef CHDIR
	    if (!strncmp(argv[1], "-d", 2) && argv[1][2] != 'e') {
		/* avoid matching "-dec" for DECgraphics; since the man page
		 * says -d directory, hope nobody's using -desomething_else
		 */
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
	    } else
#endif /* CHDIR /**/

	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	    if (!strncmp(argv[1], "-s", 2)) {
#ifdef CHDIR
		chdirx(dir,0);
#endif
		prscore(argc, argv);
		if(isatty(1) > 0) getret();
		settty(NULL);
		exit(0);
	    }
	}

	/*
	 * It seems you really want to play.
	 */
	setrandom();
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
# ifndef EXPLORE_MODE
		case 'X':
		case 'x':
# endif
		case 'D':
# ifdef WIZARD
			if(!strcmp(getenv("USER"), WIZARD_NAME)) {
				wizard = TRUE;
				break;
			}
			/* otherwise fall thru to discover */
# endif
# ifdef EXPLORE_MODE
		case 'X':
		case 'x':
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
		case 'i':
			if(!strcmp(argv[0]+1, "ibm")) assign_ibm_graphics();
			break;
		case 'd':
			if(!strcmp(argv[0]+1, "dec")) assign_dec_graphics();
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
#ifdef WIZARD
	if (wizard)
		Strcpy(plname, "wizard");
	else
#endif
	if(!*plname || !strncmp(plname, "games", 4))
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

	Sprintf(SAVEF, "[.save]%d%s", getuid(), plname);
	regularize(SAVEF+7);	/* avoid bogus chars in name */
	if((fd = open(SAVEF,O_RDONLY)) >= 0 &&
	   /* if not up-to-date, quietly unlink file via false condition */
	   (uptodate(fd) || unlink(SAVEF) >= 0)) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
		(void) chmod(SAVEF,0);	/* disallow parallel restores */
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
		pline("Restoring save file...");
		(void) fflush(stdout);
		if(!dorecover(fd))
			goto not_recovered;
#ifdef WIZARD
		if(!wizard && remember_wiz_mode) wizard = TRUE;
#endif
		pline("Hello %s, welcome to NetHack!", plname);
		/* get shopkeeper set properly if restore is in shop */
		(void) inshop();
#ifdef EXPLORE_MODE
		if (discover)
			You("are in non-scoring discovery mode.");
#endif
#if defined(EXPLORE_MODE) || defined(WIZARD)
		if (discover || wizard) {
			pline("Do you want to keep the save file? ");
			if(yn() == 'n')
			    (void) unlink(SAVEF);
			else
			    (void) chmod(SAVEF,FCMASK); /* back to readable */
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

	moveloop();
	return(0);
}

void
glo(foo)
register int foo;
{
	/* construct the string  xlock.n  */
	register char *tf;

	tf = lock;
	while(*tf && *tf != '.') tf++;
	Sprintf(tf, ".%d;1", foo);
}

/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
void
askname() {
	register int c, ct;

	Printf("\nWho are you? ");
	(void) fflush(stdout);
	ct = 0;
	while((c = getchar()) != '\n') {
		if(c == EOF) error("End of input\n");
		if(c != '-')
		if(c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
		if(ct < sizeof(plname)-1)
			plname[ct++] = c;
	}
	plname[ct] = 0;
	if(ct == 0) askname();
}

#ifdef CHDIR
void
chdirx(dir, wr)
char *dir;
boolean wr;
{
# ifndef HACKDIR
	static char *defdir = ".";
# else
	static char *defdir = HACKDIR;

	if(dir == NULL)
		dir = defdir;
	else if (wr)
		/* If we're playing anywhere other than HACKDIR, turn off any
		   privs we may have been installed with. */
		privoff();
# endif

	if(dir && chdir(dir) < 0) {
		perror(dir);
		error("Cannot chdir to %s.", dir);
	}

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access systemcall is worthless */
	if(wr) {
	    register int fd;

	    if(dir == NULL)
		dir = "";
	    if((fd = open(RECORD, O_RDWR)) < 0) {
		if((fd = open(RECORD, O_CREAT|O_RDWR, FCMASK)) < 0) {
		    Printf("Warning: cannot write %s/%s", dir, RECORD);
		    getret();
		} else
		    (void) close(fd);
	    } else
		(void) close(fd);
	}
	defdir = dir;
}
#endif /* CHDIR /**/

static void
whoami() {
	/*
	 * Who am i? Algorithm: 1. Use name as specified in NETHACKOPTIONS
	 *			2. Use $USER	(if 1. fails)
	 * The resulting name is overridden by command line options.
	 * If everything fails, or if the resulting name is some generic
	 * account like "games" then eventually we'll ask him.
	 * Note that we trust the user here; it is possible to play under
	 * somebody else's name.
	 */
	register char *s;

	if(!*plname && (s = getenv("USER")))
		(void) strncpy(plname, s, sizeof(plname)-1);
}

static void
byebye()
{
    int (*hup)();
#ifdef SHELL
    extern unsigned int dosh_pid;

    if (dosh_pid)
	SYS$DELPRC(&dosh_pid, 0);
#endif SHELL

    /* SIGHUP doesn't seem to do anything on VMS, so we fudge it here... */
    hup = signal(SIGHUP, SIG_IGN);
    if (hup != SIG_DFL && hup != SIG_IGN)
	(*hup)();

#ifdef CHDIR
    (void) chdir(getenv("PATH"));
#endif
}
