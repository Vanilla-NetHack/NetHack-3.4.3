/*	SCCS Id: @(#)pcmain.c	3.2	95/07/26	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - MSDOS, OS/2, ST, Amiga, and NT NetHack */

#include "hack.h"
#include "dlb.h"

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

#ifdef WIN32
#include "win32api.h"			/* for GetModuleFileName */
#endif

#ifdef __DJGPP__
#include <unistd.h>			/* for getcwd() prototype */
#endif

#ifdef OVL0
#define SHARED_DCL
#else
#define SHARED_DCL extern
#endif


SHARED_DCL char orgdir[PATHLEN];	/* also used in pcsys.c, amidos.c */

#ifdef TOS
boolean run_from_desktop = TRUE;	/* should we pause before exiting?? */
# ifdef __GNUC__
long _stksize = 16*1024;
# endif
#endif

#ifdef AMIGA
extern int bigscreen;
void NDECL( preserve_icon );
#endif

static void FDECL(process_options,(int argc,char **argv));
STATIC_DCL void NDECL(nhusage);

#ifdef EXEPATH
STATIC_DCL char *FDECL(exepath,(char *));
#endif

#ifdef OVL0
int FDECL(main, (int,char **));
#endif

extern void FDECL(pcmain, (int,char **));

#ifdef OVLB
const char *classes = "ABCEHKPRSTVW";
#else
extern const char *classes;
#endif

#ifdef __BORLANDC__
void NDECL( startup );
# ifdef OVLB
unsigned _stklen = STKSIZ;
# else
extern unsigned _stklen;
# endif
#endif

#ifdef OVL0
int
main(argc,argv)
int argc;
char *argv[];
{
     pcmain(argc,argv);
     moveloop();
     exit(EXIT_SUCCESS);
     /*NOTREACHED*/
     return 0;
}
#endif /*OVL0*/
#ifdef OVL1

void
pcmain(argc,argv)
int argc;
char *argv[];
{

	register int fd;
	register char *dir;

#ifdef __BORLANDC__
	startup();
#endif

#ifdef TOS
	long clock_time;
	if (*argv[0]) { 		/* only a CLI can give us argv[0] */
		hname = argv[0];
		run_from_desktop = FALSE;
	} else
#endif
		hname = "NetHack";      /* used for syntax messages */

#ifndef WIN32_GRAPHICS
	choose_windows(DEFAULT_WINDOW_SYS);
#endif

#if !defined(AMIGA) && !defined(GNUDOS)
	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	if (getcwd(orgdir, sizeof orgdir) == (char *)0)
		error("NetHack: current directory path too long");
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) exit);	/* restore original directory */
# endif
#endif /* !AMIGA && !GNUDOS */

	dir = getenv("NETHACKDIR");
	if (dir == (char *)0)
		dir = getenv("HACKDIR");
#ifdef EXEPATH
	if (dir == (char *)0)
		dir = exepath(argv[0]);
#endif
	if (dir != (char *)0) {
		Strcpy(hackdir, dir);
#ifdef CHDIR
		chdirx (dir, 1);
#endif
	}
#ifdef AMIGA
# ifdef CHDIR
	/*
	 * If we're dealing with workbench, change the directory.  Otherwise
	 * we could get "Insert disk in drive 0" messages. (Must be done
	 * before initoptions())....
	 */
	if(argc == 0)
		chdirx(HACKDIR, 1);
# endif
	ami_argset(&argc, argv);
	ami_wininit_data();
#endif

	initoptions();

#ifdef AMIGA
	ami_mkargline(&argc, &argv);
#endif

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
	    }
	    if (argc > 1) {

		/*
		 * Now we know the directory containing 'record' and
		 * may do a prscore().
		 */
		if (!strncmp(argv[1], "-s", 2)) {
#ifdef CHDIR
			chdirx(hackdir,0);
#endif
			prscore(argc, argv);
			exit(EXIT_SUCCESS);
		}
		/* Don't inialize the window system just to print usage */
		if (!strncmp(argv[1], "-?", 2) || !strncmp(argv[1], "/?", 2)) {
			nhusage();
			exit(EXIT_SUCCESS);
		}
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

	init_nhwindows(&argc,argv);
	process_options(argc, argv);

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
	plnamesuffix(); 	/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifndef MFLOPPY
	Strcpy(lock,plname);
	Strcat(lock,".99");
	regularize(lock);	/* is this necessary? */
#endif

	/* Set up level 0 file to keep the game state.	This will have to
	 * be expanded to something like the UNIX/VMS getlock() to allow
	 * networked PCs in any case.
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
	level_info[0].where = ACTIVE;
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

	dlb_init();

	display_gamewindows();

	if ((fd = restore_saved_game()) >= 0) {
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
		if (discover)
			You("are in non-scoring discovery mode.");

		if (discover || wizard) {
			if(yn("Do you want to keep the save file?") == 'n'){
				(void) delete_savefile();
			}
# ifdef AMIGA
			else
				preserve_icon();
# endif
		}

		flags.move = 0;
	} else {
not_recovered:
		player_selection();
		newgame();
		/* give welcome message before pickup messages */
		pline("Hello %s, welcome to NetHack!", plname);
		if (discover)
			You("are in non-scoring discovery mode.");

		flags.move = 0;
		set_wear();
		pickup(1);
		read_engr_at(u.ux,u.uy);
	}

#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#endif
#ifdef OS2
	gettty(); /* somehow ctrl-P gets turned back on during startup ... */
#endif

	return;
}

STATIC_OVL void
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
		case 'X':
			discover = TRUE;
			break;
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
	    /*	case 'D': */
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
			nhusage();
			exit(EXIT_SUCCESS);
		}
	}
}

static void 
nhusage()
{
	char buf1[BUFSZ];

	(void) Sprintf(buf1,
	       "\nUsage: %s [-d dir] -s [-[%s]] [maxrank] [name]...\n       or",
		hname, classes);
	if (!flags.window_inited)
		raw_printf(buf1);
	else
		(void)	printf(buf1);

	(void) Sprintf(buf1,"\n       %s [-d dir] [-u name] [-[%s]] [-[DX]]",
		hname, classes);
#ifdef NEWS
	Strcat(buf1," [-n]");
#endif
#ifndef AMIGA
	Strcat(buf1," [-I] [-i] [-d]");
#endif
#ifdef MFLOPPY
# ifndef AMIGA
	Strcat(buf1," [-r]");
# endif
#endif
#ifdef AMIGA
	Strcat(buf1," [-[lL]]");
#endif
	if (!flags.window_inited)
		raw_printf("%s\n",buf1);
	else
		(void) printf("%s\n",buf1);
}

#ifdef CHDIR
void
chdirx(dir, wr)
char *dir;
boolean wr;
{
# ifdef AMIGA
	static char thisdir[] = "";
# else
	static char thisdir[] = ".";
# endif
	if(dir && chdir(dir) < 0) {
		error("Cannot chdir to %s.", dir);
	}

# ifndef AMIGA
	/* Change the default drive as well.
	 */
	chdrive(dir);
# endif

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access system-call is worthless */
	if (wr) check_recordfile(dir ? dir : thisdir);
}
#endif /* CHDIR */
#endif /*OVL1*/
#ifdef OVLB

#ifdef PORT_HELP
# if defined(MSDOS) || defined(WIN32)
void
port_help()
{
    /* display port specific help file */
    display_file( PORT_HELP, 1 );
}
# endif /* MSDOS || WIN32 */
#endif /* PORT_HELP */

#ifdef EXEPATH

#ifdef __DJGPP__
#define PATH_SEPARATOR '/'
#else
#define PATH_SEPARATOR '\\'
#endif

char *exepath(str)
char *str;
{
	char *tmp, *tmp2;
	int bsize;

	if (!str) return (char *)0;
#ifndef WIN32
	bsize = strlen(str) + 1;
#else
	bsize = 256;
#endif
	tmp = (char *)alloc(bsize);
#ifndef WIN32
	strcpy (tmp, str);
#else
	*(tmp + GetModuleFileName((HANDLE)0, tmp, bsize)) = '\0';
#endif
	tmp2 = strrchr(tmp, PATH_SEPARATOR);
	if (tmp2) *tmp2 = '\0';
	return tmp;
}
#endif /* EXEPATH */
#endif /*OVLB*/
/*pcmain.c*/
