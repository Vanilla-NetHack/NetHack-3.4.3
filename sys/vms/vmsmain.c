/*	SCCS Id: @(#)vmsmain.c	3.1	93/05/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* main.c - VMS NetHack */

#include "hack.h"

#include <signal.h>

static void NDECL(whoami);
static void FDECL(process_options, (int, char **));
static void NDECL(byebye);
#ifndef SAVE_ON_FATAL_ERROR
# ifndef __DECC
extern void FDECL(VAXC$ESTABLISH, (int (*)(genericptr_t,genericptr_t)));
# endif
static int FDECL(vms_handler, (genericptr_t,genericptr_t));
#include <ssdef.h>	/* system service status codes */
#endif

int
main(argc,argv)
int argc;
char *argv[];
{
	register int fd;
#ifdef CHDIR
	register char *dir;
#endif

#ifdef SECURE	/* this should be the very first code executed */
	privoff();
	fflush((FILE *)0);	/* force stdio to init itself */
	privon();
#endif

	atexit(byebye);
	hname = argv[0];
	gethdate(hname);		/* find executable's creation date */
	hname = basename(hname);	/* name used in 'usage' type messages */
	hackpid = getpid();
	(void) umask(0);

	choose_windows(DEFAULT_WINDOW_SYS);

#ifdef CHDIR			/* otherwise no chdir() */
	/*
	 * See if we must change directory to the playground.
	 * (Perhaps hack is installed with privs and playground is
	 *  inaccessible for the player.)
	 * The logical name HACKDIR is overridden by a
	 *  -d command line option (must be the first option given)
	 */
	dir = getenv("NETHACKDIR");
	if (!dir) dir = getenv("HACKDIR");
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
#endif /* CHDIR */

	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	    if (!strncmp(argv[1], "-s", 2)) {
#ifdef CHDIR
		chdirx(dir, FALSE);
#endif
		prscore(argc, argv);
		exit(0);
	    }
	}

#ifdef CHDIR
	/* move to the playground directory; 'termcap' might be found there */
	chdirx(dir, TRUE);
#endif

#ifdef SECURE
	/* disable installed privs while loading nethack.cnf and termcap,
	   and also while initializing terminal [$assign("TT:")]. */
	privoff();
#endif
	initoptions();
	init_nhwindows();
	whoami();
#ifdef SECURE
	privon();
#endif

	/*
	 * It seems you really want to play.
	 */
	setrandom();
	u.uhp = 1;	/* prevent RIP on early quits */
#ifndef SAVE_ON_FATAL_ERROR
	/* used to clear hangup stuff while still giving standard traceback */
	VAXC$ESTABLISH(vms_handler);
#endif
	(void) signal(SIGHUP, (SIG_RET_TYPE) hangup);

	process_options(argc, argv);	/* command line options */

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
			Sprintf(lock, "_%u%s", (unsigned)getuid(), plname);
		getlock();
#ifdef WIZARD
	} else {
		Sprintf(lock, "_%u%s", (unsigned)getuid(), plname);
		getlock();
	}
#endif /* WIZARD */

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

	/*
	 *  Initialize the vision system.  This must be before mklev() on a
	 *  new game or before a level restore on a saved game.
	 */
	vision_init();

	display_gamewindows();

	set_savefile_name();
	if((fd = open_savefile()) >= 0 &&
	   /* if not up-to-date, quietly unlink file via false condition */
	   (uptodate(fd) || delete_savefile())) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
		(void) chmod(SAVEF,0);	/* disallow parallel restores */
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef NEWS
		if(flags.news) {
		    display_file(NEWS, FALSE);
		    flags.news = FALSE; /* in case dorecover() fails */
		}
#endif
		pline("Restoring save file...");
		mark_synch();	/* flush output */
		if(!dorecover(fd))
			goto not_recovered;
#ifdef WIZARD
		if(!wizard && remember_wiz_mode) wizard = TRUE;
#endif
		pline("Hello %s, welcome back to NetHack!", plname);
		check_special_room(FALSE);
#ifdef EXPLORE_MODE
		if (discover)
			You("are in non-scoring discovery mode.");
#endif
#if defined(EXPLORE_MODE) || defined(WIZARD)
		if (discover || wizard) {
			if (yn("Do you want to keep the save file?") == 'n')
			    (void) delete_savefile();
			else
			    (void) chmod(SAVEF,FCMASK); /* back to readable */
		}
#endif
		flags.move = 0;
	} else {
not_recovered:
		player_selection();
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
	}

	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		You("are lucky!  Full moon tonight.");
		change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful!  New moon tonight.");
	}
	if ((flags.friday13 = friday_13th()) != 0) {
		pline("Watch out!  Bad things can happen on Friday the 13th.");
		change_luck(-1);
	}

	initrack();

	moveloop();
	return(0);
}

static void
process_options(argc, argv)
int argc;
char *argv[];
{
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
			if(!strcmpi(getenv("USER"), WIZARD_NAME)) {
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
			flags.news = FALSE;
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
				raw_print("Player name expected after -u");
			break;
		case 'I':
		case 'i':
			if (!strncmpi(argv[0]+1, "IBM", 3))
				switch_graphics(IBM_GRAPHICS);
			break;
	    /*  case 'D': */
		case 'd':
			if (!strncmpi(argv[0]+1, "DEC", 3))
				switch_graphics(DEC_GRAPHICS);
			break;
		default:
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);

			/* raw_printf("Unknown option: %s", *argv); */
		}
	}

	if(argc > 1)
		locknum = atoi(argv[1]);
#ifdef MAX_NR_OF_PLAYERS
	if(!locknum || locknum > MAX_NR_OF_PLAYERS)
		locknum = MAX_NR_OF_PLAYERS;
#endif
}

#ifdef CHDIR
void
chdirx(dir, wr)
const char *dir;
boolean wr;
{
# ifndef HACKDIR
	static const char *defdir = ".";
# else
	static const char *defdir = HACKDIR;

	if(dir == NULL)
		dir = defdir;
	else if (wr && !same_dir(HACKDIR, dir))
		/* If we're playing anywhere other than HACKDIR, turn off any
		   privs we may have been installed with. */
		privoff();
# endif

	if(dir && chdir(dir) < 0) {
		perror(dir);
		error("Cannot chdir to %s.", dir);
	}

	/* warn the player if we can't write the record file */
	if (wr) check_recordfile(dir);

	defdir = dir;
}
#endif /* CHDIR */

static void
whoami()
{
	/*
	 * Who am i? Algorithm: 1. Use name as specified in NETHACKOPTIONS
	 *			2. Use lowercase of $USER  (if 1. fails)
	 * The resulting name is overridden by command line options.
	 * If everything fails, or if the resulting name is some generic
	 * account like "games" then eventually we'll ask him.
	 * Note that we trust the user here; it is possible to play under
	 * somebody else's name.
	 */
	register char *s;

	if (!*plname && (s = getenv("USER")))
		(void) lcase(strncpy(plname, s, sizeof(plname)-1));
}

static void
byebye()
{
    /*	Different versions of both VAX C and GNU C use different return types
	for signal functions.  Return type 'int' along with the explicit casts
	below satisfy the most combinations of compiler vs <signal.h>.
     */
    int (*hup)();
#ifdef SHELL
    extern unsigned long dosh_pid, mail_pid;
    extern unsigned long FDECL(SYS$DELPRC,(unsigned long *,const genericptr_t));

    /* clean up any subprocess we've spawned that may still be hanging around */
    if (dosh_pid) (void) SYS$DELPRC(&dosh_pid, 0), dosh_pid = 0;
    if (mail_pid) (void) SYS$DELPRC(&mail_pid, 0), mail_pid = 0;
#endif

    /* SIGHUP doesn't seem to do anything on VMS, so we fudge it here... */
    hup = (int(*)()) signal(SIGHUP, SIG_IGN);
    if (hup != (int(*)()) SIG_DFL && hup != (int(*)()) SIG_IGN)
	(void) (*hup)();

#ifdef CHDIR
    (void) chdir(getenv("PATH"));
#endif
}

#ifndef SAVE_ON_FATAL_ERROR
/* Condition handler to prevent byebye's hangup simulation
   from saving the game after a fatal error has occurred.  */
static int			/* should be `unsigned long', but the -*/
vms_handler(sigargs, mechargs)	/*+ prototype in <signal.h> is screwed */
genericptr_t sigargs, mechargs;	/* [0] is argc, [1..argc] are the real args */
{
    extern boolean hu;		/* src/save.c */
    unsigned long condition = ((unsigned long *)sigargs)[1];

    if (condition == SS$_ACCVIO		/* access violation */
     || (condition >= SS$_ASTFLT && condition <= SS$_TBIT)
     || (condition >= SS$_ARTRES && condition <= SS$_INHCHME)) {
	hu = TRUE;	/* pretend that hangup has already been attempted */
# if defined(WIZARD) && !defined(BETA)
	if (wizard)
# endif /*WIZARD && !BETA*/
# if defined(WIZARD) ||  defined(BETA)
	    abort();	/* enter the debugger */
# endif /*WIZARD || BETA*/
    }
    return SS$_RESIGNAL;
}
#endif

/*vmsmain.c*/
