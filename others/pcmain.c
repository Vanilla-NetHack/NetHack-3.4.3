/*	SCCS Id: @(#)pcmain.c	3.0	88/11/23
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* main.c - PC, ST, and Amiga NetHack */

#include "hack.h"
#ifndef NO_SIGNAL
#include <signal.h>
#endif

char orgdir[PATHLEN];

extern struct permonst mons[NUMMONS];
extern char plname[PL_NSIZ], pl_character[PL_CSIZ];

int (*afternmv)(), (*occupation)();
static void moveloop();	/* a helper function for MSC optimizer */
static void newgame();

#if defined(DGK) && !defined(OLD_TOS)
struct finfo	zfinfo = ZFINFO;
int i;
#endif /* DGK && !OLD_TOS */

char SAVEF[FILENAME];
char *hname = "NetHack";	/* used for syntax messages */
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */
int hackpid;		/* not used anymore, but kept in for save files */

extern char *nomovemsg;
extern long wailmsg;
#ifdef __TURBOC__	/* tell Turbo C to make a bigger stack */
extern unsigned _stklen = 0x2000;	/* 8K */
extern unsigned char _osmajor;
#endif

#if defined(TOS) && defined(__GNUC__)
long _stksize = 16*1024;
#endif

#ifdef OLD_TOS
#define OMASK	0x8000
#else
#define OMASK	0
#endif

int
main(argc,argv)
int argc;
char *argv[];
{
	extern int x_maze_max, y_maze_max;
	register int fd;
	register char *dir;

#ifdef AMIGA
	/*
	 *  Make sure screen IO is initialized before anything happens.
	 */
	gettty();
	startup();
#else /* AMIGA */
	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	int (*funcp)();

#if defined(TOS) && defined(__GNUC__)
	extern int _unixmode;
	_unixmode = 0;
#endif

# ifdef __TURBOC__
	if (_osmajor >= 3) hname = argv[0];	/* DOS 3.0+ */
# endif
	if (getcwd(orgdir, sizeof orgdir) == NULL) {
		xputs("NetHack: current directory path too long\n");
		return 1;
	}
	funcp = exit;	/* Kludge to get around LINT_ARGS of signal.
			 * This will produce a compiler warning, but that's OK.
			 */
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) funcp);	/* restore original directory */
# endif
#endif /* AMIGA */

	/* Set the default values of the presentation characters */
	(void) memcpy((genericptr_t) &showsyms,
		(genericptr_t) &defsyms, sizeof(struct symbols));
	if ((dir = getenv("HACKDIR")) != NULL) {
		Strcpy(hackdir, dir);
#ifdef CHDIR
		chdirx (dir, 1);
#endif
	}
#if defined(DGK) && !defined(OLD_TOS)
	/* zero "fileinfo" array to prevent crashes on level change */
	for (i = 0 ; i <= MAXLEVEL; i++) {
		fileinfo[i] = zfinfo;
	}
#endif /* DGK && !OLD_TOS */
	initoptions();
	if (!hackdir[0])
		Strcpy(hackdir, orgdir);

	if(argc > 1) {
#ifdef TOS
	    if(!strncmp(argv[1], "-D", 2)) {
#else
	    if(!strncmp(argv[1], "-d", 2)) {
#endif
		argc--;
		argv++;
		dir = argv[0]+2;
		if(*dir == '=' || *dir == ':') dir++;
		if(!*dir && argc > 1) {
			argc--;
			argv++;
			dir = argv[0];
		}
		if(!*dir) {
		    /* can't use error() until termcap read in */
		    xputs("Flag -d must be followed by a directory name.\n");
		    return 1;
		}
		Strcpy(hackdir, dir);
	    }

	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
#ifdef TOS
	    else if (!strncmp(argv[1], "-S", 2)) {
#else
	    else if (!strncmp(argv[1], "-s", 2)) {
#endif
#ifdef CHDIR
		chdirx(hackdir,0);
#endif
		prscore(argc, argv);
		exit(0);
	    }
	}

#ifndef AMIGA
	/*
	 * It seems he really wants to play.
	 * Remember tty modes, to be restored on exit.
	 */
	gettty();
	setbuf(stdout,obuf);
	startup();
#endif
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
# ifndef TOS
		case 'D':
# endif
		case 'X':
# ifdef WIZARD
			/* Must have "name" set correctly by NETHACK.CNF,
			 * NETHACKOPTIONS, or -U
			 * before this flag to enter wizard mode. */
			if(!strcmp(plname, WIZARD))
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
		case 'N':
# ifndef TOS
		case 'n':
# endif
			flags.nonews = TRUE;
			break;
#endif
		case 'U':
#ifndef TOS
		case 'u':
#endif
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
		/* Person does not want to use a ram disk
		 */
# ifdef TOS
		case 'R':
# else
		case 'r':
# endif
			ramdisk = FALSE;
			break;
#endif
		case 'C':   /* character role is next character */
			/* allow -CT for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+2,
				sizeof(pl_character)-1);
			break;
		default:
#ifndef TOS
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);
#else
			Printf("Unknown option: %s\n", *argv);
#endif
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

#ifdef AMIGA
	(void) strncat(SAVEF, plname, 31-4);
#else
	(void) strncat(SAVEF, plname, 8);
#endif
	Strcat(SAVEF, ".sav");
	cls();
	if (
#ifdef DGK
	    saveDiskPrompt(1) &&
#endif /* DGK */
	    ((fd = open(SAVEF, OMASK)) >= 0) &&
	    (uptodate(fd) || !unlink(SAVEF))) {
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
		pline("Restoring old save file...");
		(void) fflush(stdout);
		if(!dorecover(fd))
			goto not_recovered;
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
	/* Help for Microsoft optimizer.  Otherwise main is too large -dgk*/
	moveloop();
	return 0;
}

static void
moveloop()
{
	char ch;
	int abort;

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
#ifdef POLYSELF
			if(u.mtimedone)
			    if(u.mh < 1) rehumanize();
			else
#endif
			    if(u.uhp < 1) {
				You("die...");
				done(DIED);
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
			if(Polymorph && !rn2(100))
				polyself();
			if(u.ulycn >= 0 && !rn2(80 - (20 * night())))
				you_were();
#endif
			if(Searching && multi >= 0) (void) dosearch0(1);
			gethungry();
			hatch_eggs();
			invault();
			amulet();
#ifdef HARD
			if (!rn2(40+(int)(ACURR(A_DEX)*3))) u_wipe_engr(rnd(3));
			if (u.udemigod) {

				u.udg_cnt--;
				if(u.udg_cnt <= 0) {

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
		if(flags.time) flags.botl = 1;

		if(flags.botl || flags.botlx) bot();

		flags.move = 1;

		if(multi >= 0 && occupation) {
			abort = 0;
			if (kbhit()) {
				if ((ch = Getchar()) == ABORT)
					abort++;
#ifdef REDO
				else
					pushch(ch);
#endif /* REDO */
			}
			if(abort || monster_nearby())
				stop_occupation();
			else if ((*occupation)() == 0)
				occupation = 0;
			if (!(++occtime % 7))
				(void) fflush(stdout);
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
			}
			continue;
		}
		if(c != '-')
		if(c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
		if(ct < sizeof(plname)-1) {
#ifdef MSDOS
			msmsg("%c", c);
#endif
			plname[ct++] = c;
	}
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

	/* warn the player if he cannot write the record file */
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
newgame() {
#ifdef DGK
	gameDiskPrompt();
#endif

	fobj = fcobj = invent = 0;
	fmon = fallen_down = 0;
	ftrap = 0;
	fgold = 0;
	flags.ident = 1;

	init_objects();
	u_init();

#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif

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
