/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "hack.h"

extern char *getlogin();
extern char plname[PL_NSIZ], pl_character[PL_CSIZ];
extern char *getenv();

int (*afternmv)();

int done1();
int hangup();

char safelock[] = "safelock";
xchar locknum;				/* max num of players */
char *catmore = "/bin/cat";		/* or e.g. /usr/ucb/more */
char SAVEF[PL_NSIZ + 5] = "save/";
char perm[] = "perm";
char *hname;		/* name of the game (argv[0] of call) */
char obuf[BUFSIZ];	/* BUFSIZ is defined in stdio.h */

extern char *nomovemsg;
extern long wailmsg;

main(argc,argv)
int argc;
char *argv[];
{
	int fd;
#ifdef NEWS
	int nonews = 0;
#endif NEWS
	char *dir;

	hname = argv[0];

	/*
	 * See if we must change directory to the playground.
	 * (Perhaps hack runs suid and playground is inaccessible
	 *  for the player.)
	 * The environment variable HACKDIR is overridden by a
	 *  -d command line option.
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

	/*
	 * Now we know the directory containing 'record' and
	 * may do a prscore().
	 */
	if(argc > 1 && !strncmp(argv[1], "-s", 2)) {
		if(dir) chdirx(dir);
		prscore(argc, argv);
		exit(0);
	}

	/*
	 * It seems he really wants to play. Find the creation date of
	 * this game so as to avoid restoring outdated savefiles.
	 */
	gethdate(hname);

	/*
	 * We cannot do chdir earlier, otherwise gethdate will fail.
	 */
	if(dir) chdirx(dir);

	/*
	 * Who am i? Perhaps we should use $USER instead?
	 */
	(void) strncpy(plname, getlogin(), sizeof(plname)-1);

	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
#ifdef WIZARD
		case 'w':
			if(!strcmp(getlogin(), WIZARD))
				wizard = TRUE;
			else printf("Sorry.\n");
			break;
#endif WIZARD
#ifdef NEWS
		case 'n':
			nonews++;
			break;
#endif NEWS
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
			printf("Unknown option: %s\n", *argv);
		}
	}

	if(argc > 1)
		locknum = atoi(argv[1]);
	if(argc > 2)
		catmore = argv[2];
#ifdef WIZARD
	if(wizard) (void) strcpy(plname, "wizard"); else
#endif WIZARD
	if(!*plname || !strncmp(plname, "player", 4)) askname();
	plnamesuffix();		/* strip suffix from name */

	setbuf(stdout,obuf);
 	(void) srand(getpid());
	startup();
	cls();
	(void) signal(SIGHUP, hangup);
#ifdef WIZARD
	if(!wizard) {
#endif WIZARD
		(void) signal(SIGQUIT,SIG_IGN);
		(void) signal(SIGINT,SIG_IGN);
		if(locknum)
			lockcheck();
		else
			(void) strcpy(lock,plname);
#ifdef WIZARD
	} else {
		register char *sfoo;
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
				extern char genocided[], fut_geno[];
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
#endif WIZARD
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = FAR;	/* prevent nscr() */
	(void) strcat(SAVEF,plname);
	if((fd = open(SAVEF,0)) >= 0 &&
	   (uptodate(fd) || unlink(SAVEF) == 666)) {
		(void) signal(SIGINT,done1);
		puts("Restoring old save file...");
		(void) fflush(stdout);
		dorecover(fd);
		flags.move = 0;
	} else {
#ifdef NEWS
		if(!nonews)
			if((fd = open(NEWS,0)) >= 0)
				outnews(fd);
#endif NEWS
		flags.ident = 1;
		init_objects();
		u_init();
		(void) signal(SIGINT,done1);
		glo(1);
		mklev();
		u.ux = xupstair;
		u.uy = yupstair;
		(void) inshop();
		setsee();
		flags.botlx = 1;
		makedog();
		seemons();
		docrt();
		pickup();
		read_engr_at(u.ux,u.uy);	/* superfluous ? */
		flags.move = 1;
		flags.cbreak = ON;
		flags.echo = OFF;
	}
	setftty();
#ifdef TRACK
	initrack();
#endif TRACK
	for(;;) {
		if(flags.move) {
#ifdef TRACK
			settrack();
#endif TRACK
			if(moves%2 == 0 ||
			  (!(Fast & ~INTRINSIC) && (!Fast || rn2(3)))) {
				extern struct monst *makemon();
				movemon();
				if(!rn2(70))
				    (void) makemon((struct permonst *)0, 0, 0);
			}
			if(Glib) glibr();
			timeout();
			++moves;
			if(u.uhp < 1) {
				pline("You die...");
				done("died");
			}
			if(u.uhp*10 < u.uhpmax && moves-wailmsg > 50){
			    wailmsg = moves;
			    if(u.uhp == 1)
			    pline("You hear the wailing of the Banshee...");
			    else
			    pline("You hear the howling of the CwnAnnwn...");
			}
			if(u.uhp < u.uhpmax) {
				if(u.ulevel > 9) {
					if(Regeneration || !(moves%3)) {
					    flags.botl = 1;
					    u.uhp += rnd((int) u.ulevel-9);
					    if(u.uhp > u.uhpmax)
						u.uhp = u.uhpmax;
					}
				} else if(Regeneration ||
					(!(moves%(22-u.ulevel*2)))) {
					flags.botl = 1;
					u.uhp++;
				}
			}
			if(Teleportation && !rn2(85)) tele();
			if(Searching && multi >= 0) (void) dosearch();
			gethungry();
			invault();
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
		flags.move = 1;
		find_ac();
#ifndef QUEST
		if(!flags.mv || Blind)
#endif QUEST
		{
			seeobjs();
			seemons();
			nscr();
		}
		if(flags.botl || flags.botlx) bot();
		if(multi > 0) {
#ifdef QUEST
			if(flags.run >= 4) finddir();
#endif QUEST
			lookaround();
			if(!multi) {	/* lookaround may clear multi */
				flags.move = 0;
				continue;
			}
			if(flags.mv) {
				if(multi<COLNO && !--multi)
					flags.mv = flags.run = 0;
				domove();
			} else {
				--multi;
				rhack(save_cm);
			}
		} else if(multi == 0)
			rhack((char *) 0);
	}
}

lockcheck()
{
	extern int errno;
	register int i, fd;

	/* we ignore QUIT and INT at this point */
	if (link(perm,safelock) == -1)
		error("Cannot link safelock. (Try again or rm safelock.)");

	for(i = 0; i < locknum; i++) {
		lock[0]= 'a' + i;
		if((fd = open(lock,0)) == -1) {
			if(errno == ENOENT) goto gotlock;    /* no such file */
			(void) unlink(safelock);
			error("Cannot open %s", lock);
		}
		(void) close(fd);
	}
	(void) unlink(safelock);
	error("Too many hacks running now.");
gotlock:
	fd = creat(lock,FMASK);
	if(fd == -1) {
		error("cannot creat lock file.");
	} else {
		int pid;

		pid = getpid();
		if(write(fd, (char *) &pid, 2) != 2){
			error("cannot write lock");
		}
		if(close(fd) == -1){
			error("cannot close lock");
		}
	}
	if(unlink(safelock) == -1){
		error("Cannot unlink safelock");
	}
}

/*VARARGS1*/
error(s,a1,a2,a3,a4) char *s,*a1,*a2,*a3,*a4; {
	printf("Error: ");
	printf(s,a1,a2,a3,a4);
	(void) putchar('\n');
	exit(1);
}

glo(foo)
register foo;
{
	/* construct the string  xlock.n  */
	register char *tf;

	tf = lock;
	while(*tf && *tf!='.') tf++;
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
	ct = 0;
	while((c = getchar()) != '\n'){
		if(c == EOF) error("End of input\n");
		if(c != '-')
		if(c < 'A' || (c > 'Z' && c < 'a') || c > 'z') c = '_';
		if(ct < sizeof(plname)-1) plname[ct++] = c;
	}
	plname[ct] = 0;
	if(ct == 0) askname();
#ifdef QUEST
	else printf("Hello %s, welcome to quest!\n", plname);
#else
	else printf("Hello %s, welcome to hack!\n", plname);
#endif QUEST
}

impossible(){
	pline("Program in disorder - perhaps you'd better Quit");
}

#ifdef NEWS
int stopnews;

stopnws(){
	(void) signal(SIGINT, SIG_IGN);
	stopnews++;
}

outnews(fd) int fd; {
int (*prevsig)();
char ch;
	prevsig = signal(SIGINT, stopnws);
	while(!stopnews && read(fd,&ch,1) == 1)
		(void) putchar(ch);
	(void) putchar('\n');
	(void) fflush(stdout);
	(void) close(fd);
	(void) signal(SIGINT, prevsig);
	/* See whether we will ask TSKCFW: he might have told us already */
	if(!stopnews && pl_character[0])
		getret();
}
#endif NEWS

chdirx(dir) char *dir; {
	if(chdir(dir) < 0) {
		perror(dir);
		error("Cannot chdir to %s.", dir);
	}
}
