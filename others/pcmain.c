/*	SCCS Id: @(#)pcmain.c	3.0	90/01/19
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* main.c - PC, ST, and Amiga NetHack */
#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif
#include <ctype.h>
#ifdef DGK
#ifndef AMIGA
#include <sys\stat.h>
#endif
#endif
#ifdef MACOS
extern WindowPtr	HackWindow;
extern short *switches;
extern short macflags;
pascal boolean FDECL(startDlogFProc, (DialogPtr, EventRecord *, short *));
#define msmsg mprintf
#endif

#if !defined(MACOS) && !defined(LATTICE)
char orgdir[PATHLEN];
#endif
char SAVEF[FILENAME];
#ifdef MSDOS
char SAVEP[FILENAME];
#endif

const char *hname = "NetHack";	/* used for syntax messages */
#if !defined(AMIGA) && !defined(MACOS)
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */
#endif
int hackpid;		/* not used anymore, but kept in for save files */

#if defined(DGK)
struct finfo	zfinfo = ZFINFO;
int i;
#endif /* DGK */

#ifdef __TURBOC__	/* tell Turbo C to make a bigger stack */
extern unsigned _stklen = 0x2000;	/* 8K */
extern unsigned char _osmajor;
#endif

#ifdef TOS
extern long compiletime;
boolean run_from_desktop = TRUE;	/* should we pause before exiting?? */
# ifdef __GNUC__
long _stksize = 16*1024;
# endif
#endif

#ifdef MACOS
#  ifdef AZTEC
#define OMASK	O_RDONLY
#  else
#define OMASK	(O_RDONLY | O_BINARY )
#  endif
# else
#define OMASK	O_RDONLY
#endif

#ifdef MACOS
Boolean justscores;
#endif

#ifdef AMIGA_WBENCH
extern int FromWBench;
#endif

const char *classes = "ABCEHKPRSTVW";

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
#ifdef TOS
	long clock;
# ifdef __GNUC__
	extern int _unixmode;
	_unixmode = 0;
# endif
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
			if ((theFile.fType == SAVE_TYPE)||(theFile.fType == EXPLORE_TYPE))
				break;
			message++;
		}
		if ((theFile.fType == SAVE_TYPE)||(theFile.fType == EXPLORE_TYPE)) {
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
	funcp = (int (*)())exit; /* Kludge to get around LINT_ARGS of signal. */
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
#if defined(AMIGA) && defined(CHDIR)
	/*
	 * If we're dealing with workbench, change the directory.  Otherwise
	 * we could get "Insert disk in drive 0" messages. (Must be done
	 * before initoptions())....
	 */
	if(argc == 0)
		chdirx(HACKDIR, 1);
#endif

# if defined(DGK)
	/* zero "fileinfo" array to prevent crashes on level change */
	for (i = 0 ; i <= MAXLEVEL; i++) {
		fileinfo[i] = zfinfo;
	}
# endif /* DGK */

	initoptions();
#ifdef AMIGA_WBENCH
	ami_wbench_init(argc,argv);
#endif
# if defined(TOS) && defined(TEXTCOLOR)
	if (flags.IBMBIOS && flags.use_color)
		set_colors();
# endif
	if (!hackdir[0])
#if !defined(LATTICE) && !defined(AMIGA)
		Strcpy(hackdir, orgdir);
#else
		Strcpy(hackdir, HACKDIR);
#endif
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
#ifdef TOS
	if ((unsigned long)time(&clock) < (unsigned long)compiletime)
		error("Your clock is incorrectly set!");
#endif
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = FAR;	/* prevent nscr() */

	/*
	 * Find the creation date of this game,
	 * so as to avoid restoring outdated savefiles.
	 */
	/* gethdate(hname); */

	/*
	 * We cannot do chdir earlier, otherwise gethdate will fail.
	 */
#ifdef CHDIR
	chdirx(hackdir,1);
#endif

#ifndef MACOS
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
#ifdef DGK
		/* Player doesn't want to use a RAM disk
		 */
		case 'r':
			ramdisk = FALSE;
			break;
#endif
		default:
			if (index(classes, toupper(argv[0][1]))) {
				/* allow -T for Tourist, etc. */
				(void) strncpy(pl_character, argv[0]+1,
					       sizeof(pl_character)-1);
				break;
			} else Printf("\nUnknown switch: %s\n", argv[0]);
		case '?':
Printf("\nUsage: %s [-d dir] -s [-[%s]] [maxrank] [name]...", hname, classes);
Printf("\n       or");
Printf("\n       %s [-d dir] [-u name] [-[%s]]", hname, classes);
#if defined(WIZARD) || defined(EXPLORE_MODE)
			Printf(" [-[DX]]");
#endif
#ifdef NEWS
			Printf(" [-n]");
#endif
#ifdef DGK
			Printf(" [-r]");
#endif
			putchar('\n');
			return 0;
		}
	}
#ifdef AMIGA_WBENCH
	ami_wbench_args();
#endif
#ifdef DGK
	set_lock_and_bones();
	copybones(FROMPERM);
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
#ifndef DGK
	Strcpy(lock,plname);
	Strcat(lock,".99");
#endif
#endif /* MACOS */
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
	if (!numFiles) {
		askname();
		if(justscores){
			prscore(1,&classes);
			exit(0);
		}
#endif
#if defined(AMIGA) || defined(MACOS)
# ifdef AMIGA_WBENCH
	if(!FromWBench)
# endif
	(void) strncat(SAVEF, plname, 31-4);
#else
	{
		int ix = strlen(SAVEF);
		(void)strncat(SAVEF, plname, 8);
		regularize(SAVEF+ix);
	}
#endif
#ifndef MACOS
# ifdef AMIGA_WBENCH
	if(!FromWBench)
# endif
	Strcat(SAVEF, ".sav");
#else
	}
	Strcpy(lock,plname);
	Strcat(lock,".99");
#endif
	cls();
	if (
#ifdef DGK
# ifdef AMIGA_WBENCH
	    (FromWBench?1:saveDiskPrompt(1)) &&
# else
	    saveDiskPrompt(1) &&
# endif
#endif /* DGK */
#ifdef AMIGA_WBENCH
	    ((fd=ami_wbench_getsave(OMASK)) >=0) &&
#else
	    ((fd = open(SAVEF, OMASK)) >= 0) &&
#endif
	   /* if not up-to-date, quietly unlink file via false condition */
	   (uptodate(fd) || unlink(SAVEF) == 666)) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
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
			if(yn() == 'n'){
				(void) unlink(SAVEF);
#ifdef AMIGA_WBENCH
				ami_wbench_unlink(SAVEF);
#endif
			}
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
		Rect	screen;
		
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
		SetPort(HackWindow);
		screen = HackWindow->portRect;
		ValidRect(&screen);
		
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
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
void
askname() {
#ifndef MACOS
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
			}
			continue;
		}
		if(ct < sizeof(plname)-1) {
#if defined(MSDOS)
			msmsg("%c", c);
#endif
			plname[ct++] = c;
		}
	}
	plname[ct] = 0;
	if(ct == 0) askname();
}
#else /* MACOS */
/* Macintosh startup Dialog written by Andy Swanson 10/20/89
		modified for to include a few more options 12/17/89 */
	DialogPtr asknameDlog;
	DialogTHndl	th, centreDlgBox();
	int kind;
	Rect box;
	Handle knob;
	Boolean Done;
	int chtype = 0,Hit,i;
	Str255 ptemp;
	char *p;
#define OK 1
#define NAME_TEXT 3
#define RADIO_MIN 5
#define RADIO_MAX 17
#define CAVEPERSON 7
#define CLERGY 11
#define VALKYRIE 15
#define ANY 17
#define WIZ 18
#define EXP 19
#define FEM 20
#define NEWS_BOX 21
#define SCORES 22
#define setCheckBox(a,b,c) {GetDItem(a,b,&kind,&knob,&box);SetCtlValue(knob,c?1:0);}
#define changeRadio(a,b,c) {setCheckBox(a,b,FALSE); setCheckBox(a,c,TRUE);}
#define Disable(b) {GetDItem(asknameDlog,b,&kind,&knob,&box);HiliteControl(knob,255);}
#define Enable(b) {GetDItem(asknameDlog,b,&kind,&knob,&box);HiliteControl(knob,0);}
#define Hide(b)  {GetDItem(asknameDlog,b,&kind,&knob,&box);HideControl(knob);\
			SetDItem(asknameDlog,b,kind+128,knob,&box);}
	justscores = FALSE;
	if(p=strrchr((char *)plname, '-')){
		*p = 0;
		if(('a'<= p[1]) && ('z'>= p[1]))p[1] += 'A' - 'a';
		pl_character[0] = p[1];
		pl_character[1] = 0;
		if(chtype = (int)index(classes,p[1]))
			chtype -= (int)(classes)-1;
		if(p[1] == 'V')
			flags.female = TRUE;
	}
	if(chtype != 0) chtype += 4;
	else chtype = 17;
#ifdef THINKC4
	if(!*plname && (p = getlogin()))
		(void) strncpy((char *)&plname,p,sizeof(plname)-1);
#endif
	th = centreDlgBox(131, FALSE);
	
	asknameDlog = GetNewDialog(131,0,-1);
	
	ReleaseResource((Handle)th);
	if(*plname){
		GetDItem(asknameDlog,NAME_TEXT,&kind,&knob,&box);
		strncpy((char*)ptemp,(char*)&plname,255);
		CtoPstr((char*)ptemp);
		SetIText(knob,ptemp);
	}
	GetDItem(asknameDlog,chtype,&kind,&knob,&box);
	SetCtlValue(knob,1);
	if(flags.female){
		setCheckBox(asknameDlog,FEM,TRUE);
		changeDgenders(asknameDlog,TRUE);
	}
#ifdef NEWS
	setCheckBox(asknameDlog,NEWS_BOX,!flags.nonews);
#else
	Hide(NEWS_BOX);
#endif
#ifdef WIZARD
	wizard = FALSE;
# ifdef KR1ED
	if (strcmp(plname,WIZARD_NAME)) {
# else
	if (strcmp(plname,WIZARD)) {
# endif
#else
	{
#endif
		Hide(WIZ);
	}
#ifdef EXPLORE_MODE
	setCheckBox(asknameDlog,EXP,discover);
#else
		Hide(EXP);
#endif
	SelIText(asknameDlog, NAME_TEXT, 0, 32767);
	ShowWindow(asknameDlog);
	Done = FALSE;
	while (!Done){
		ModalDialog(startDlogFProc, &Hit);
		if(Hit == OK){
			Done = TRUE;
			GetDItem(asknameDlog,NAME_TEXT,&kind,&knob,&box);
			GetIText(knob,&ptemp);
			PtoCstr((char*)ptemp);
			(void) strncpy((char*)&plname,(char*)ptemp,sizeof(plname)-1);
			pl_character[0]=classes[chtype-5];
			pl_character[1]=0;
			HideWindow(asknameDlog);
		} else if((Hit >= RADIO_MIN) && (Hit <= RADIO_MAX)){
			extern int lastDlgBut;
			
			changeRadio(asknameDlog,chtype,Hit);
			lastDlgBut = chtype = Hit;
			if ((chtype == VALKYRIE) && !flags.female) {
				flags.female = TRUE;
				setCheckBox(asknameDlog,FEM,flags.female);
				changeDgenders(asknameDlog,TRUE);
			}
		} else if(Hit == WIZ) {
			wizard = !wizard;
			setCheckBox(asknameDlog,WIZ,wizard);
		} else if(Hit == EXP) {
			discover = !discover;
			setCheckBox(asknameDlog,EXP,discover);
		} else if(Hit == FEM) {
			flags.female = !flags.female;
			setCheckBox(asknameDlog,FEM,flags.female);
			if(chtype == VALKYRIE) {
				chtype = ANY;
				changeRadio(asknameDlog,VALKYRIE,ANY);
			}
			changeDgenders(asknameDlog,flags.female);
		} else if(Hit == NEWS_BOX) {
			flags.nonews = !flags.nonews;
			setCheckBox(asknameDlog,NEWS_BOX,!flags.nonews);
		} else if(Hit == SCORES) {
			justscores = !justscores;
			setCheckBox(asknameDlog,SCORES,justscores);
			if(justscores) for (i=RADIO_MIN;i<SCORES;i++) {
				Disable(i);
				}
			else for (i=RADIO_MIN;i<SCORES;i++)
				Enable(i);
		}
	}
	DisposDialog(asknameDlog);
}


#define RADIO_STRING "ABCEHKPRSTVWL"
int	lastDlgBut = ANY;

/* The filterProc for handling character selection from keyboard
   by h+@nada.kth.se                                             */
pascal boolean
startDlogFProc(theDialog, theEvent, itemHit)
DialogPtr theDialog;
EventRecord * theEvent;
short * itemHit;
{
	int x, c;

	if(theEvent->what == keyDown) {
		c = theEvent->message & 0xFF;
#ifdef BETA /* We don't want this is no shipped version */
		if(c == '#') Debugger();
#endif
		if(c == 10 || c == 13 || c == 3) { /* Accept */
			*itemHit = OK;
			return TRUE;
		}
		if(c == '\t' || c == ' ') { /* Select */
			lastDlgBut++;
			if(lastDlgBut > RADIO_MAX) lastDlgBut = RADIO_MIN;
			*itemHit = lastDlgBut;
			return TRUE;
		}
		if(theEvent->modifiers & cmdKey) {
			if(c >= 'a' && c <= 'z') c &= 0x5F; /* Uppercase */
			switch(c) {
			case 'F' :
				*itemHit = FEM;
				return TRUE;
			case 'X' :
				*itemHit = EXP;
				return TRUE;
			case 'N' :
				*itemHit = NEWS_BOX;
				return TRUE;
			case 'J' :
				*itemHit = SCORES;
				return TRUE;
			default :
				for(x=0; RADIO_STRING[x]; x++) {
					if(c == RADIO_STRING[x]) {
						*itemHit = x + RADIO_MIN;
						return TRUE;
					}
				}
			}
			theEvent->what = nullEvent;
		}
	}

	return FALSE;
}


changeDgenders(Dlog,fem)
DialogPtr Dlog;
Boolean fem;
{	int kind;
	Rect box;
	Handle knob;
	Str255 ptemp;
	if(fem){
		GetDItem(Dlog,CAVEPERSON,&kind,&knob,&box);
		strncpy((char*)ptemp,"Cave-Woman",255);
		CtoPstr((char*)ptemp);
		SetCTitle(knob,ptemp);
		GetDItem(Dlog,CLERGY,&kind,&knob,&box);
		strncpy((char*)ptemp,"Priestess",255);
		CtoPstr((char*)ptemp);
		SetCTitle(knob,ptemp);
	} else {
		GetDItem(Dlog,CAVEPERSON,&kind,&knob,&box);
		strncpy((char*)ptemp,"Cave-Man",255);
		CtoPstr((char*)ptemp);
		SetCTitle(knob,ptemp);
		GetDItem(Dlog,CLERGY,&kind,&knob,&box);
		strncpy((char*)ptemp,"Priest",255);
		CtoPstr((char*)ptemp);
		SetCTitle(knob,ptemp);
	}
}
#endif /* MACOS */


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
	/* unfortunately the access systemcall is worthless */
	if(wr) {
	    register int fd;

	    if(dir == NULL)
		dir = thisdir;
#ifdef OS2_CODEVIEW  /* explicit path on opening for OS/2 */
	    {
	    char tmp[PATHLEN];

	    Strcpy(tmp, dir);
	    append_slash(tmp);
	    Strcat(tmp, RECORD);
	    if((fd = open(tmp, O_RDWR)) < 0) {
#else
	    if((fd = open(RECORD, O_RDWR)) < 0) {
#endif
#ifdef DGK
# ifndef OS2_CODEVIEW
		char tmp[PATHLEN];

		Strcpy(tmp, dir);
		append_slash(tmp);
# endif
		/* try to create empty record */

# ifdef OS2_CODEVIEW
		if((fd = open(tmp, O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) < 0) {
		    msmsg("Warning: cannot write %s\n", tmp);
# else
# ifdef AZTEC_C
		/* Aztec doesn't use the third argument */
		if((fd = open(RECORD, O_CREAT|O_RDWR)) < 0) {
		    msmsg("Warning: cannot write %s%s\n", tmp, RECORD);
# else
  		if((fd = open(RECORD, O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) < 0) {
		    msmsg("Warning: cannot write %s%s\n", tmp, RECORD);
# endif
# endif
		    getreturn("to continue");
		} else
		    (void) close(fd);
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
