/*	SCCS Id: @(#)pcmain.c	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* main.c - (PC) version */

#include <stdio.h>
#include <signal.h>
#include "hack.h"

#ifdef QUEST
#define	gamename	"PC NetQuest"
#else
#define	gamename	"PC NetHack"
#endif

char orgdir[PATHLEN], *getcwd();

extern struct permonst mons[CMNUM+2];
extern char genocided[], fut_geno[];
extern char *getlogin(), *getenv();
extern char plname[PL_NSIZ], pl_character[PL_CSIZ];

int (*afternmv)(), done1(), (*occupation)();

char SAVEF[FILENAME];
char *hname = gamename;
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */
int hackpid;		/* not used anymore, but kept in for save files */

extern char *nomovemsg;
extern long wailmsg;

main(argc,argv)
int argc;
char *argv[];
{
	register int fd;
	register char *dir;
	extern struct monst *makedog();
#ifdef MSDOS
	static void moveloop();	/* a helper function for MSC optimizer */

	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	int (*funcp)();

	if (getcwd(orgdir, sizeof orgdir) == NULL) {
		xputs("NetHack: current directory path too long\n");
		_exit(1);
	}
	funcp = exit;	/* Kludge to get around LINT_ARGS of signal.
			 * This will produce a compiler warning, but that's OK.
			 */
	signal(SIGINT, funcp);	/* restore original directory */
#endif

#ifdef GRAPHICS
	/* Set the default values of the presentation characters */
	memcpy((char *) &showsyms, (char *) &defsyms, sizeof(struct symbols));
#endif
#ifdef DGK
	if ((dir = getenv("HACKDIR")) != (char *) NULL) {
		(void) strcpy(hackdir, dir);
		chdirx (dir, 1);
	}
	zero_finfo();
	initoptions();
	if (!hackdir[0])
		(void) strcpy(hackdir, orgdir);
	dir = hackdir;
#else
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
#endif /* DGK */

	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	if(argc > 1 && !strncmp(argv[1], "-s", 2)) {
		chdirx(dir,0);
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

	/*
	 * We cannot do chdir earlier, otherwise gethdate will fail.
	 */
	chdirx(dir,1);

	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
#ifdef WIZARD
		case 'D':
# ifdef MSDOS
			wizard = TRUE;
# else
			if(!strcmp(getlogin(), WIZARD))
				wizard = TRUE;
			else {
				settty("Sorry, you can't operate in debug mode.\n");
				clearlocks();
				exit(0);
			}
# endif
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
#ifdef DGK
		/* Person does not want to use a ram disk
		 */
		case 'R':
			ramdisk = FALSE;
			break;
#endif
		default:
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);

			/* printf("Unknown option: %s\n", *argv); */
		}
	}

#ifdef DGK
	set_lock_and_bones();
	copybones(FROMPERM);
#endif
#ifdef WIZARD
	if (wizard)
		(void) strcpy(plname, "wizard");
	else
#endif
	if (!*plname)
		askname();
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if(wizard) {
		register char *sfoo;
# ifndef DGK
		/* lock is set in read_config_file */
		(void) strcpy(lock,plname);
# endif
		if(sfoo = getenv("MAGIC"))
			while(*sfoo) {
				switch(*sfoo++) {
				case 'n': (void) srand(*sfoo++);
					break;
				}
			}
		if(sfoo = getenv("GENOCIDED")){
			if(*sfoo == '!'){
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
#endif /* WIZARD */
	start_screen();
#ifdef DGK
	strncat(SAVEF, plname, 8);
	strcat(SAVEF, ".sav");
	cls();
	if (saveDiskPrompt(1) && ((fd = open(SAVEF, 0)) >= 0) &&
	   (uptodate(fd) || !unlink(SAVEF))) {
#else 
	(void) sprintf(SAVEF, "save/%d%s", getuid(), plname);
	regularize(SAVEF+5);		/* avoid . or / in name */
	if((fd = open(SAVEF,0)) >= 0 &&
	   (uptodate(fd) || unlink(SAVEF) == 666)) {
#endif /* DGK */
		(void) signal(SIGINT,done1);
		pline("Restoring old save file...");
		(void) fflush(stdout);
		if(!dorecover(fd))
			goto not_recovered;
		pline("Hello %s%s, welcome to %s!", 
			(Badged) ? "Officer " : "", plname, hname);
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
		flags.move = 0;
	} else {
not_recovered:
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

		(void) signal(SIGINT,done1);
		mklev();
		u.ux = xupstair;
		u.uy = yupstair;
		(void) inshop();
		setsee();
		flags.botlx = 1;
		/* Fix bug with dog not being made because a monster
		 * was on the level 1 staircase
		 */
		{
			struct monst *mtmp;

			if (mtmp = m_at(u.ux, u.uy))
				mnexto(mtmp);
		}
		makedog();
		{ register struct monst *mtmp;
		  if(mtmp = m_at(u.ux, u.uy)) mnexto(mtmp);	/* riv05!a3 */
		}
		seemons();
		docrt();

		/* give welcome message before pickup messages */
		pline("Hello %s, welcome to %s!", plname, hname);

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
	(void) signal(SIGINT, SIG_IGN);
#ifdef MSDOS
	/* Help for Microsoft optimizer.  Otherwise main is too large -dgk*/
	moveloop();
}

static void
moveloop()
{
	char ch;
	int abort;
#endif /* MSDOS */
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
#ifndef DGK
			if(flags.time) flags.botl = 1;
#endif
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
#ifdef DGK
			abort = 0;
			if (kbhit()) {
				if ((ch = getchar()) == ABORT)
					abort++;
# ifdef REDO
				else
					pushch(ch);
# endif
			}
			if (abort || monster_nearby())
				stop_occupation();
			else if ((*occupation)() == 0)
				occupation = 0;
			if (!(++occtime % 7))
				(void) fflush(stdout);
#else
			if (monster_nearby())
				stop_occupation();
			else if ((*occupation)() == 0)
				occupation = 0;
#endif
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
			rhack((char *) 0);
		}
		if(multi && multi%7 == 0)
			(void) fflush(stdout);
	}
}

#ifndef DGK
/* This function is unnecessary and incompatible with the #define
 * of glo(x) in config.h -dgk
 */
glo(foo)
register foo;
{
	/* construct the string  xlock.n  */
	register char *tf;

	tf = lock;
	while(*tf && *tf != '.') tf++;
	(void) sprintf(tf, ".%d", foo);
}
#endif

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
#ifdef MSDOS
		msmsg("%c", c);
#endif
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
chdirx(dir, wr)
char *dir;
boolean wr;
{

	if(dir && chdir(dir) < 0) {
		error("Cannot chdir to %s.", dir);
	}

#ifdef DGK
	/* Change the default drive as well.
	 */
	chdrive(dir);
#endif

	/* warn the player if he cannot write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access systemcall is worthless */
	if(wr) {
	    register fd;

	    if(dir == NULL)
		dir = ".";
	    if((fd = open(RECORD, 2)) < 0) {
#ifdef DGK
		char tmp[PATHLEN];

		strcpy(tmp, dir);
		append_slash(tmp);
		msmsg("Warning: cannot write %s%s\n", tmp, RECORD);
		getreturn("to continue");
#else
		printf("Warning: cannot write %s/%s", dir, RECORD);
		getret();
#endif
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

#ifdef DGK
struct finfo	zfinfo = ZFINFO;

zero_finfo() {	/* zero "fileinfo" array to prevent crashes on level change */
	int i;

	for (i = 0 ; i <= MAXLEVEL; i++)
		fileinfo[i] = zfinfo;
}
#endif
