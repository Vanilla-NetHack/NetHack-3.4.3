/*	SCCS Id: @(#)pcmain.c	3.0	88/11/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* main.c - PC, ST, and Amiga NetHack */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif
#ifdef MACOS
extern WindowPtr	HackWindow;
extern short *switches;
extern short macflags;
#define msmsg mprintf
#endif

#ifndef MACOS
char orgdir[PATHLEN];
#endif
char SAVEF[FILENAME];

char *hname = "NetHack";	/* used for syntax messages */
#ifndef AMIGA
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */
#endif
int hackpid;		/* not used anymore, but kept in for save files */

#if defined(DGK) && !defined(OLD_TOS)
struct finfo	zfinfo = ZFINFO;
int i;
#endif /* DGK && !OLD_TOS */

#ifdef __TURBOC__	/* tell Turbo C to make a bigger stack */
extern unsigned _stklen = 0x2000;	/* 8K */
extern unsigned char _osmajor;
#endif

#ifdef TOS
boolean run_from_desktop = TRUE;	/* should we pause before exiting?? */
# ifdef __GNUC__
long _stksize = 16*1024;
# endif
#endif

#ifdef OLD_TOS
#define OMASK	0x8000
#else
# ifdef MACOS
#  ifdef AZTEC
#define OMASK	O_RDONLY
#  else
#define OMASK	(O_RDONLY | O_BINARY )
#  endif
# else
#define OMASK	0
# endif
#endif

int
main(argc,argv)
int argc;
char *argv[];
{
	extern int x_maze_max, y_maze_max;
	register int fd;
	register char *dir;
#ifndef AMIGA
	int (*funcp)();
#endif
#if defined(TOS) && defined(__GNUC__)
	extern int _unixmode;
	_unixmode = 0;
#endif
#ifdef __TURBOC__
	if (_osmajor >= 3) hname = argv[0];	/* DOS 3.0+ */
#endif
#ifdef TOS
	if (*argv[0]) {			/* only a CLI can give us argv[0] */
		hname = argv[0];
		run_from_desktop = FALSE;
	}
#endif
#ifdef MACOS
	AppFile	theFile;
	short	message,numFiles;
	SFReply	reply;

	initterm(24,80);
	ObscureCursor();
# ifdef SMALLDATA
	init_decl();
# endif
	/* user might have started up with a save file, so check */
	CountAppFiles(&message,&numFiles);
	if (!message && numFiles) {
		message = 1;
		
		while(message <= numFiles) {
			GetAppFiles(message,&theFile);
			ClrAppFiles(message);
			if (theFile.fType == SAVE_TYPE)
				break;
		}
		if (theFile.fType == SAVE_TYPE) {
			(void)strncpy(SAVEF, (char *)&theFile.fName[1],
						(int)theFile.fName[0]);
			(void)strncpy(plname, (char *)&theFile.fName[1],
						(int)theFile.fName[0]);
			SetVol(0,theFile.vRefNum);
			SAVEF[(int)theFile.fName[0]] = '\0';
			numFiles = 1;
		} else
			numFiles = 0;
	} 
	switches = (short *)malloc((NROFOBJECTS+2) * sizeof(long));
	for (fd = 0; fd < (NROFOBJECTS + 2); fd++)
		switches[fd] = fd;
#endif


	/*
	 *  Initialize screen I/O before anything is displayed.
	 *
	 *  startup() must be called before initoptions()
	 *    due to ordering of graphics settings
	 *  and before error(), due to use of termcap strings.
	 */
	gettty();
#if !defined(AMIGA) && !defined(MACOS)
	setbuf(stdout,obuf);
#endif
	startup();
#if !defined(AMIGA) && !defined(MACOS)
	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	if (getcwd(orgdir, sizeof orgdir) == NULL)
		error("NetHack: current directory path too long");
	funcp = exit;	/* Kludge to get around LINT_ARGS of signal.
			 * This will produce a compiler warning, but that's OK.
			 */
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) funcp);	/* restore original directory */
# endif
#endif /* AMIGA || MACOS */

#ifndef MACOS
	if ((dir = getenv("HACKDIR")) != NULL) {
		Strcpy(hackdir, dir);
# ifdef CHDIR
		chdirx (dir, 1);
# endif
	}
# if defined(DGK) && !defined(OLD_TOS)
	/* zero "fileinfo" array to prevent crashes on level change */
	for (i = 0 ; i <= MAXLEVEL; i++) {
		fileinfo[i] = zfinfo;
	}
# endif /* DGK && !OLD_TOS */

	initoptions();
# ifdef TOS
	if (flags.IBMBIOS && flags.use_color)
		set_colors();
# endif
	if (!hackdir[0])
		Strcpy(hackdir, orgdir);

	if(argc > 1) {
	    if (!strncmp(argv[1], "-d", 2)) {
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
# ifdef CHDIR
		chdirx(hackdir,0);
# endif
		prscore(argc, argv);
		exit(0);
	    }
	}
#else
	initoptions();
#endif	/* MACOS /* */	

	/*
	 * It seems you really want to play.
	 */
	setrandom();
	cls();
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = FAR;	/* prevent nscr() */

	/*
	 * Find the creation date of this game,
	 * so as to avoid restoring outdated savefiles.
	 */
	/* gethdate(hname); */

#ifndef OLD_TOS
	/*
	 * We cannot do chdir earlier, otherwise gethdate will fail.
	 */
# ifdef CHDIR
	chdirx(hackdir,1);
# endif
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
# endif
		case 'D':
# ifdef WIZARD
			/* Must have "name" set correctly by NETHACK.CNF,
			 * NETHACKOPTIONS, or -U
			 * before this flag to enter wizard mode. */
			if(!strcmp(plname, WIZARD)) {
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
				Printf("Player name expected after -U\n");
			break;
#ifdef DGK
		/* Player doesn't want to use a RAM disk
		 */
		case 'r':
			ramdisk = FALSE;
			break;
#endif
		default:
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);
		}
	}

#ifdef DGK
	set_lock_and_bones();
	copybones(FROMPERM);
#endif
#ifdef WIZARD
	if (wizard)
		Strcpy(plname, "wizard");
	else
#endif
#if defined(KR1ED) && defined(WIZARD) && defined(MACOS)
	if (!strcmp(plname,WIZARD))
		Strcpy(plname, "wizard");
	else
#endif
	if (!*plname)
		askname();
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifndef DGK
	Strcpy(lock,plname);
	Strcat(lock,".99");
#endif
	start_screen();

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
#ifdef MACOS
	if (!numFiles && findNamedFile(plname,1,&reply)) {
		if (reply.good) {
		    strncpy(SAVEF,(char *)&reply.fName[1],(int)reply.fName[0]);
		    SAVEF[(int)reply.fName[0]] = '\0';
		}
	} else if (!numFiles)
#endif
#if defined(AMIGA) || defined(MACOS)
	(void) strncat(SAVEF, plname, 31-4);
#else
	(void) strncat(SAVEF, plname, 8);
#endif
#ifndef MACOS
	Strcat(SAVEF, ".sav");
#endif
	cls();
	if (
#ifdef DGK
	    saveDiskPrompt(1) &&
#endif /* DGK */
	    ((fd = open(SAVEF, OMASK)) >= 0) &&
	    (uptodate(fd) || !unlink(SAVEF))) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
		pline("Restoring old save file...");
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
	
#ifdef MACOS
	{
		short	i;
		MenuHandle	theMenu;
		
		theMenu = GetMHandle(appleMenu);
		EnableItem(theMenu, 0);
		EnableItem(theMenu, 1);
		theMenu = GetMHandle(fileMenu);
		EnableItem(theMenu,0);
		for (i = inventMenu;i <= extendMenu; i++) {
			theMenu = GetMHandle(i);
			EnableItem(theMenu, 0);
		}
		DrawMenuBar();
		macflags |= fDoUpdate;
	}
#endif
			
	flags.moonphase = phase_of_the_moon();
	if(flags.moonphase == FULL_MOON) {
		You("are lucky!  Full moon tonight.");
		if(!u.uluck) change_luck(1);
	} else if(flags.moonphase == NEW_MOON) {
		pline("Be careful!  New moon tonight.");
	}

	initrack();
#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#endif
#ifdef OS2
	gettty(); /* somehow ctrl-P gets turned back on during startup ... */
#endif

	moveloop();
#ifdef MACOS 
	/* Help for Mac compilers */
	free_decl();
#endif
	return 0;
}

/*
 * plname is filled either by an option (-U Player  or  -UPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
void
askname() {
	register int c, ct;

	Printf("\nWho are you? ");
	(void) fflush(stdout);
	ct = 0;
	while((c = Getchar()) != '\n') {
		if(c == EOF) error("End of input\n");
		/* some people get confused when their erase char is not ^H */
		if(c == '\b') {
			if(ct) {
				ct--;
#ifdef MSDOS
				msmsg("\b \b");
#endif
#ifdef MACOS
				putc('\b');
#endif
			}
			continue;
		}
		if(c != '-')
		if(c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
		if(ct < sizeof(plname)-1) {
#if defined(MSDOS) || defined(MACOS)
			msmsg("%c", c);
#endif
			plname[ct++] = c;
		}
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
	/* unfortunately the access systemcall is worthless */
	if(wr) {
	    register int fd;

	    if(dir == NULL)
#ifdef AMIGA
		dir = "";
#else
		dir = ".";
#endif
#ifdef OS2_CODEVIEW  /* explicit path on opening for OS/2 */
		{
		char tmp[PATHLEN];

		Strcpy(tmp, dir);
		append_slash(tmp);
		Strcat(tmp, RECORD);
		if((fd = open(tmp, 2)) < 0) {
#else
		if((fd = open(RECORD, 2)) < 0) {
#endif
#ifdef DGK
#ifndef OS2_CODEVIEW
		char tmp[PATHLEN];

		Strcpy(tmp, dir);
		append_slash(tmp);
#endif
		msmsg("Warning: cannot write %s%s\n", tmp, RECORD);
		getreturn("to continue");
#else
		Printf("Warning: cannot write %s/%s", dir, RECORD);
		getret();
#endif
	    } else
		(void) close(fd);
#ifdef OS2_CODEVIEW
		}
#endif
	}
}
#endif /* CHDIR /**/
