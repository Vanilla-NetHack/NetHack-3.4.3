/*	SCCS Id: @(#)pcmain.c	3.1	93/02/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - MSDOS, OS/2, ST, Amiga, and NT NetHack */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#include <ctype.h>

#if !defined(AMIGA) && !defined(GNUDOS)
#include <sys\stat.h>
#else
# ifdef GNUDOS
#include <sys/stat.h>
# endif
#endif

#if !defined(LATTICE)
char orgdir[PATHLEN];
#endif

#ifdef MFLOPPY
struct finfo	zfinfo = ZFINFO;
int i;
#endif /* MFLOPPY */

#ifdef TOS
boolean run_from_desktop = TRUE;	/* should we pause before exiting?? */
# ifdef __GNUC__
long _stksize = 16*1024;
# endif
#endif

#ifdef AMIGA
extern int bigscreen;
#endif
#ifdef AMII_GRAPHICS
extern unsigned short amii_initmap[8];
#endif

static void FDECL(process_options,(int argc,char **argv));

int FDECL(main, (int,char **));

const char *classes = "ABCEHKPRSTVW";

int
main(argc,argv)
int argc;
char *argv[];
{
	register int fd;
	register char *dir;
#ifdef TOS
	long clock_time;
	if (*argv[0]) {			/* only a CLI can give us argv[0] */
		hname = argv[0];
		run_from_desktop = FALSE;
	} else
#endif
		hname = "NetHack";	/* used for syntax messages */

	choose_windows(DEFAULT_WINDOW_SYS);

#if !defined(AMIGA) && !defined(GNUDOS)
	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	if (getcwd(orgdir, sizeof orgdir) == NULL)
		error("NetHack: current directory path too long");
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) exit);	/* restore original directory */
# endif
#endif /* !AMIGA && !GNUDOS */

	dir = getenv("NETHACKDIR");
	if (dir == NULL)
		dir = getenv("HACKDIR");
	if (dir != NULL) {
		Strcpy(hackdir, dir);
#ifdef CHDIR
		chdirx (dir, 1);
#endif
	}
#if defined(AMIGA) && defined(CHDIR)
	/*
	 * If we're dealing with workbench, change the directory.  Otherwise
	 * we could get "Insert disk in drive 0" messages. (Must be done
	 * before initoptions())....
	 */
	if(argc == 0)
		chdirx(HACKDIR, 1);
#endif

#ifdef MFLOPPY
	/* zero "fileinfo" array to prevent crashes on level change */
	for (i = 0 ; i <= MAXLEVEL; i++) {
		fileinfo[i] = zfinfo;
	}
#endif /* MFLOPPY */
#ifdef AMIGA
	ami_argc=argc;
	ami_argv=argv;
#endif
#ifdef AMII_GRAPHICS
	memcpy(flags.amii_curmap,amii_initmap,sizeof(flags.amii_curmap));
#endif
	initoptions();

#if defined(TOS) && defined(TEXTCOLOR)
	if (flags.BIOS && flags.use_color)
		set_colors();
#endif
	if (!hackdir[0])
#if !defined(LATTICE) && !defined(AMIGA)
		Strcpy(hackdir, orgdir);
#else
		Strcpy(hackdir, HACKDIR);
#endif
	if(argc > 1) {
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
		Strcpy(hackdir, dir);
	    } else

	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	    if (!strncmp(argv[1], "-s", 2)) {
#ifdef CHDIR
		chdirx(hackdir,0);
#endif
		prscore(argc, argv);
		exit(0);
	    }
	}

	/*
	 * It seems you really want to play.
	 */
	setrandom();

#ifdef TOS
	if (comp_times((long)time(&clock_time)))
		error("Your clock is incorrectly set!");
#endif
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = 0;	/* prevent flush_screen() */

	/* chdir shouldn't be called before this point to keep the
	 * code parallel to other ports which call gethdate just
	 * before here.
	 */
#ifdef CHDIR
	chdirx(hackdir,1);
#endif

	process_options(argc, argv);

#ifdef AMIGA
	ami_wbench_args();
#endif
	init_nhwindows();
#ifdef MFLOPPY
	set_lock_and_bones();
# ifndef AMIGA
	copybones(FROMPERM);
# endif
#endif
#ifdef WIZARD
	if (wizard)
		Strcpy(plname, "wizard");
	else
#endif
	if (!*plname)
		askname();
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifndef MFLOPPY
	Strcpy(lock,plname);
	Strcat(lock,".99");
	regularize(lock);	/* is this necessary? */
#endif

	/* set up level 0 file to keep game state in */
	/* this will have to be expanded to something like the
	 * UNIX/VMS getlock() to allow networked PCs in any case
	 */
	fd = create_levelfile(0);
	if (fd < 0) {
		raw_print("Cannot create lock file");
	} else {
		hackpid = 1;
		write(fd, (genericptr_t) &hackpid, sizeof(hackpid));
		close(fd);
	}
#ifdef MFLOPPY
	    fileinfo[0].where = ACTIVE;
#endif

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

	if (
#ifdef MFLOPPY
# ifdef AMIGA
	    (FromWBench || saveDiskPrompt(1)) &&
# else
	    saveDiskPrompt(1) &&
# endif
#endif /* MFLOPPY */
	    ((fd = open_savefile()) >= 0) &&
	   /* if not up-to-date, quietly unlink file via false condition */
	   (uptodate(fd) || ((void)close(fd), delete_savefile()))) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
		if(flags.news){
		    display_file(NEWS, FALSE);
		    flags.news = FALSE;
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
			if(yn("Do you want to keep the save file?") == 'n'){
				(void) delete_savefile();
			}
#ifdef AMIGA
			else
				preserve_icon();
#endif
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
		read_engr_at(u.ux,u.uy);
	}
	
	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		You("are lucky!  Full moon tonight.");
		change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful!  New moon tonight.");
	}
	if(flags.friday13 = friday_13th()) {
		pline("Watch out!  Bad things can happen on Friday the 13th.");
		change_luck(-1);
	}

	initrack();
#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#endif
#ifdef OS2
	gettty(); /* somehow ctrl-P gets turned back on during startup ... */
#endif

	moveloop();
	return 0;
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
# endif
		case 'D':
# ifdef WIZARD
			/* Must have "name" set correctly by NETHACK.CNF,
			 * NETHACKOPTIONS, or -u
			 * before this flag to enter wizard mode. */
#  ifdef KR1ED
			if(!strcmp(plname, WIZARD_NAME)) {
#  else
			if(!strcmp(plname, WIZARD)) {
#  endif
				wizard = TRUE;
				break;
			}
			/* otherwise fall thru to discover */
# endif
# ifdef EXPLORE_MODE
		case 'X':
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
#ifndef AMIGA
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
#endif
#ifdef MFLOPPY
# ifndef AMIGA
		/* Player doesn't want to use a RAM disk
		 */
		case 'r':
			ramdisk = FALSE;
			break;
# endif
#endif
#ifdef AMIGA
			/* interlaced and non-interlaced screens */
		case 'L':
			bigscreen = 1;
			break;
		case 'l':
			bigscreen = -1;
			break;
#endif
		default:
			if (index(classes, toupper(argv[0][1]))) {
				/* allow -T for Tourist, etc. */
				(void) strncpy(pl_character, argv[0]+1,
					       sizeof(pl_character)-1);
				break;
			} else raw_printf("\nUnknown switch: %s", argv[0]);
		case '?':
		    (void) printf(
			"\nUsage: %s [-d dir] -s [-[%s]] [maxrank] [name]...",
			hname, classes);
		    (void) printf("\n       or");
		    (void) printf("\n       %s [-d dir] [-u name] [-[%s]]",
			hname, classes);
#if defined(WIZARD) || defined(EXPLORE_MODE)
		    (void) printf(" [-[DX]]");
#endif
#ifdef NEWS
		    (void) printf(" [-n]");
#endif
#ifndef AMIGA
		    (void) printf(" [-I] [-i] [-d]");
#endif
#ifdef MFLOPPY
# ifndef AMIGA
		    (void) printf(" [-r]");
# endif
#endif
#ifdef AMIGA
		    (void) printf(" [-[lL]]");
#endif
		    putchar('\n');
		    exit(0);
		}
	}
}

#ifdef CHDIR
void
chdirx(dir, wr)
char *dir;
boolean wr;
{
#ifdef AMIGA
	static char thisdir[] = "";
#else
	static char thisdir[] = ".";
#endif
	if(dir && chdir(dir) < 0) {
		error("Cannot chdir to %s.", dir);
	}

	/* Change the default drive as well.
	 */
#ifndef AMIGA
	chdrive(dir);
#endif

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access system-call is worthless */
	if (wr) check_recordfile(dir ? dir : thisdir);
}
#endif /* CHDIR */

/*pcmain.c*/
