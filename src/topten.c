/*	SCCS Id: @(#)topten.c	3.0	88/11/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"

#include <errno.h>	/* George Barbanis */
#ifdef NO_FILE_LINKS
#include <fcntl.h>	/* Ralf Brown */
#endif

static char *itoa P((int)), *ordin P((int));
static void outheader();
static int outentry P((int,struct toptenentry *,int));

#define newttentry() (struct toptenentry *) alloc(sizeof(struct toptenentry))
#define	NAMSZ	10
#define	DTHSZ	60
#define	PERSMAX	 3		/* entries per name/uid per char. allowed */
#define	POINTSMIN	1	/* must be > 0 */
#define	ENTRYMAX	100	/* must be >= 10 */
#ifndef MSDOS
#define	PERS_IS_UID		/* delete for PERSMAX per name; now per uid */
#endif
struct toptenentry {
	struct toptenentry *tt_next;
	long int points;
	int level,maxlvl,hp,maxhp;
	int uid;
	char plchar;
	char sex;
	char name[NAMSZ+1];
	char death[DTHSZ+1];
	char date[7];		/* yymmdd */
} *tt_head;

void
topten(){
	int uid = getuid();
	int rank, rank0 = -1, rank1 = 0;
	int occ_cnt = PERSMAX;
	register struct toptenentry *t0, *t1, *tprev;
	char *recfile = RECORD;
#ifdef UNIX
	char *reclock = "record_lock";
# ifdef NO_FILE_LINKS
	int lockfd ;
# endif
	int sleepct = 100;
#endif /* UNIX */
	FILE *rfile;
	register int flg = 0;
#ifdef LOGFILE
	char *lgfile = LOGFILE;
	FILE *lfile;
# ifdef UNIX
	char *loglock = "logfile_lock";
	int sleeplgct = 30;
# endif /* UNIX */
#endif /* LOGFILE */

#ifdef MSDOS
#define HUP
#else
#define	HUP	if(!done_hup)
#endif

	/* create a new 'topten' entry */
	t0 = newttentry();
	t0->level = dlevel;
	t0->maxlvl = maxdlevel;
	t0->hp = u.uhp;
	t0->maxhp = u.uhpmax;
	t0->points = u.urexp;
	t0->plchar = pl_character[0];
	t0->sex = (flags.female ? 'F' : 'M');
	t0->uid = uid;
	(void) strncpy(t0->name, plname, NAMSZ);
	(t0->name)[NAMSZ] = 0;
	(void) strncpy(t0->death, killer, DTHSZ);
	(t0->death)[DTHSZ] = 0;
	Strcpy(t0->date, getdate());

#ifdef LOGFILE		/* used for debugging (who dies of what, where) */
# ifdef UNIX
#  ifdef NO_FILE_LINKS
	loglock = (char *)alloc(sizeof(LOCKDIR)+1+strlen(lgfile)+6);
	Strcpy(loglock,LOCKDIR) ;
	Strcat(loglock,"/") ;
	Strcat(loglock,lgfile) ;
	Strcat(loglock,"_lock") ;
	while ((lockfd = open(loglock,O_RDWR|O_CREAT|O_EXCL,0666)) == -1) {
#  else
	while(link(lgfile, loglock) == -1) {
#  endif /* NO_FILE_LINKS */
		extern int errno;

		if (errno == ENOENT) /* If no such file, do not keep log */
			goto lgend;  /* George Barbanis */
		HUP perror(loglock);
		if(!sleeplgct--) {
			HUP (void) puts("I give up.  Sorry.");
			HUP (void) puts("Perhaps there is an old logfile_lock around?");
			goto lgend;
		}
		HUP Printf("Waiting for access to log file. (%d)\n",
 			sleeplgct);
		HUP (void) fflush(stdout);
#  if defined(SYSV) || defined(ULTRIX)
		(void)
#  endif
		    sleep(1);
	}
# endif /* UNIX */
	if(!(lfile = fopen(lgfile,"a"))){
		HUP (void) puts("Cannot open log file!");
		goto lgend;
	}
	(void) fprintf(lfile,"%6s %d %d %d %d %d %ld %c%c %s,%s\n",
	    t0->date, t0->uid,
	    t0->level, t0->maxlvl,
	    t0->hp, t0->maxhp, t0->points,
	    t0->plchar, t0->sex, t0->name, t0->death);
	(void) fclose(lfile);
# ifdef UNIX
	(void) unlink(loglock);
# endif /* UNIX */
      lgend:;
# ifdef NO_FILE_LINKS
	(void) close(lockfd) ;
# endif
# if defined(WIZARD) || defined(EXPLORE_MODE)
	if (wizard || discover) {
 Printf("\nSince you were in %s mode, the score list will not be checked.\n",
	wizard ? "wizard" : "discover");
		return;
	}
# endif
#endif /* LOGFILE */

#ifdef UNIX
# ifdef NO_FILE_LINKS
	reclock = (char *)alloc(sizeof(LOCKDIR)+1+strlen(recfile)+7);
	Strcpy(reclock,LOCKDIR) ;
	Strcat(reclock,"/") ;
	Strcat(reclock,recfile) ;
	Strcat(reclock,"_lock") ;
	while ((lockfd = open(reclock,O_RDWR|O_CREAT|O_EXCL,0666)) == -1) {
# else
	while(link(recfile, reclock) == -1) {
# endif /* NO_FILE_LINKS */
		HUP perror(reclock);
		if(!sleepct--) {
			HUP (void) puts("I give up.  Sorry.");
			HUP (void) puts("Perhaps there is an old record_lock around?");
			return;
		}
		HUP Printf("Waiting for access to record file. (%d)\n",
			sleepct);
		HUP (void) fflush(stdout);
# if defined(SYSV) || defined(ULTRIX)
		(void)
# endif
		    sleep(1);
	}
#endif /* UNIX */
	if(!(rfile = fopen(recfile,"r"))){
		HUP (void) puts("Cannot open record file!");
		goto unlock;
	}
#ifdef NO_FILE_LINKS
	(void) close(lockfd) ;
#endif
	HUP (void) putchar('\n');

	/* assure minimum number of points */
	if(t0->points < POINTSMIN) t0->points = 0;

	t1 = tt_head = newttentry();
	tprev = 0;
	/* rank0: -1 undefined, 0 not_on_list, n n_th on list */
	for(rank = 1; ; ) {
#ifdef OLD_TOS
	    char k1[2],k2[2];
	    if(fscanf(rfile, "%6s %d %d %d %d %d %ld %1s%1s %s %s]",
#else
	    if(fscanf(rfile, "%6s %d %d %d %d %d %ld %c%c %[^,],%[^\n]",
#endif
		t1->date, &t1->uid,
		&t1->level, &t1->maxlvl,
		&t1->hp, &t1->maxhp, &t1->points,
#ifdef OLD_TOS
		k1, k2,
#else
		&t1->plchar, &t1->sex,
#endif
		t1->name, t1->death) != 11 || t1->points < POINTSMIN)
			t1->points = 0;

#ifdef OLD_TOS
	    t1->plchar=k1[0];
	    t1->sex=k2[0];
#endif
	    if(rank0 < 0 && t1->points < t0->points) {
		rank0 = rank++;
		if(tprev == 0)
			tt_head = t0;
		else
			tprev->tt_next = t0;
		t0->tt_next = t1;
		occ_cnt--;
		flg++;		/* ask for a rewrite */
	    } else tprev = t1;

	    if(t1->points == 0) break;
	    if(
#ifdef PERS_IS_UID
		t1->uid == t0->uid &&
#else
		strncmp(t1->name, t0->name, NAMSZ) == 0 &&
#endif
		t1->plchar == t0->plchar && --occ_cnt <= 0) {
		    if(rank0 < 0) {
			rank0 = 0;
			rank1 = rank;
	HUP Printf("You didn't beat your previous score of %ld points.\n\n",
				t1->points);
		    }
		    if(occ_cnt < 0) {
			flg++;
			continue;
		    }
		}
	    if(rank <= ENTRYMAX) {
		t1 = t1->tt_next = newttentry();
		rank++;
	    }
	    if(rank > ENTRYMAX) {
		t1->points = 0;
		break;
	    }
	}
	if(flg) {	/* rewrite record file */
		(void) fclose(rfile);
		if(!(rfile = fopen(recfile,"w"))){
			HUP (void) puts("Cannot write record file\n");
			goto unlock;
		}

		if(!done_stopprint) if(rank0 > 0){
		    if(rank0 <= 10)
			(void) puts("You made the top ten list!\n");
		    else
		Printf("You reached the %d%s place on the top %d list.\n\n",
			rank0, ordin(rank0), ENTRYMAX);
		}
	}
	if(rank0 == 0) rank0 = rank1;
	if(rank0 <= 0) rank0 = rank;
	if(!done_stopprint) outheader();
	t1 = tt_head;
	for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
	  if(flg) (void) fprintf(rfile,"%6s %d %d %d %d %d %ld %c%c %s,%s\n",
	    t1->date, t1->uid,
	    t1->level, t1->maxlvl,
	    t1->hp, t1->maxhp, t1->points,
	    t1->plchar, t1->sex, t1->name, t1->death);
	  if(done_stopprint) continue;
	  if(rank > flags.end_top &&
	    (rank < rank0-flags.end_around || rank > rank0+flags.end_around)
	    && (!flags.end_own ||
#ifdef PERS_IS_UID
				  t1->uid != t0->uid
#else
				  strncmp(t1->name, t0->name, NAMSZ)
#endif
		)) continue;
	  if(rank == rank0-flags.end_around &&
	     rank0 > flags.end_top+flags.end_around+1 &&
	     !flags.end_own)
		(void) putchar('\n');
	  if(rank != rank0)
		(void) outentry(rank, t1, 0);
	  else if(!rank1)
		(void) outentry(rank, t1, 1);
	  else {
		int t0lth = outentry(0, t0, -1);
		int t1lth = outentry(rank, t1, t0lth);
		if(t1lth > t0lth) t0lth = t1lth;
		(void) outentry(0, t0, t0lth);
	  }
	}
	if(rank0 >= rank) if(!done_stopprint)
		(void) outentry(0, t0, 1);
	(void) fclose(rfile);
unlock:	;
#ifdef UNIX
# ifdef NO_FILE_LINKS
	(void) close(lockfd) ;
# endif
	if (unlink(reclock) < 0)
		Printf("Can't unlink %s\n",reclock) ;
#endif
}

static void
outheader() {
	char linebuf[BUFSZ];
	register char *bp;

	Strcpy(linebuf, " No  Points   Name");
	bp = eos(linebuf);
	while(bp < linebuf + COLNO - 9) *bp++ = ' ';
	Strcpy(bp, "Hp [max]");
	(void) puts(linebuf);
}

/* so>0: standout line; so=0: ordinary line; so<0: no output, return lth */
static int
outentry(rank, t1, so)
register struct toptenentry *t1;
register int rank, so;
{
	register boolean quit = FALSE, iskilled = FALSE, starv = FALSE,
		isstoned = FALSE;
	char linebuf[BUFSZ];
	linebuf[0] = 0;
	if(rank) Sprintf(eos(linebuf), " %2d", rank);
		else Strcat(linebuf, "   ");
	Sprintf(eos(linebuf), " %7ld  %.10s", t1->points, t1->name);
	Sprintf(eos(linebuf), "-%c ", t1->plchar);
	if(!strncmp("escaped", t1->death, 7)) {
	  if(!strcmp(" (with the Amulet)", t1->death+7))
	    Strcat(linebuf, "escaped the dungeon with the Amulet");
	  else
	    Sprintf(eos(linebuf), "escaped the dungeon [max level %d]",
	      t1->maxlvl);
#ifdef ENDGAME
	} else if(!strncmp("ascended", t1->death, 8)) {
	   Strcat(linebuf, "ascended to demigod-hood");
#endif
	} else {
	  if(!strncmp(t1->death,"quit",4)) {
		quit = TRUE;
		Strcat(linebuf, "quit");
	  } else if(!strcmp(t1->death,"choked")) {
		Sprintf(eos(linebuf), "choked on %s food",
			(t1->sex == 'F') ? "her" : "his");
	  } else if(!strncmp(t1->death,"starv",5)) {
		Strcat(linebuf, "starved to death");
		starv = TRUE;
	  } else if(!strcmp(t1->death,"poisoned")) {
		Strcat(linebuf, "was posioned");
	  } else if(!strcmp(t1->death,"crushed")) {
		Strcat(linebuf, "was crushed to death");
	  } else if(!strncmp(t1->death, "turned to stone by ",19)) {
		Strcat(linebuf, "was petrified");
		isstoned = TRUE;
	  } else {
		Strcat(linebuf, "was killed");
		iskilled = TRUE;
	  }
#ifdef ENDLEVEL
	  if (t1->level == ENDLEVEL)
		Strcat(linebuf, " in the endgame");
	  else
#endif
	    Sprintf(eos(linebuf), " on%s level %d",
	      (iskilled || isstoned || starv) ? "" : " dungeon", t1->level);
	  if(t1->maxlvl != t1->level)
	    Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
	  if(quit && t1->death[4]) Strcat(linebuf, t1->death + 4);
	}
	if(iskilled) Sprintf(eos(linebuf), " by %s%s",
	  (!strncmp(t1->death, "trick", 5) || !strncmp(t1->death, "the ", 4)
	   || !strncmp(t1->death, "Mr. ", 4) || !strncmp(t1->death, "Ms. ", 4)
	   || !strncmp(eos(t1->death)-5, "ation", 5)
	   ) ? "" :
	  index(vowels,*t1->death) ? "an " : "a ",
	  t1->death);
	if (isstoned) Sprintf(eos(linebuf), " by %s%s", index(vowels,
		*(t1->death + 19)) ? "an " : "a ", t1->death + 19);
	Strcat(linebuf, ".");
	if(t1->maxhp) {
	  register char *bp = eos(linebuf);
	  char hpbuf[10];
	  int hppos;
	  int lngr = strlen(linebuf);
	  Strcpy(hpbuf, (t1->hp > 0) ? itoa(t1->hp) : "-");
	  hppos = COLNO - 7 - strlen(hpbuf);
	  if (lngr >= hppos) hppos = (2*COLNO) - 7 - strlen(hpbuf);
	  if(bp <= linebuf + hppos) {
	    /* pad any necessary blanks to the hit point entry */
	    while(bp < linebuf + hppos) *bp++ = ' ';
	    Strcpy(bp, hpbuf);
	    if(t1->maxhp < 10)
		 Sprintf(eos(bp), "   [%d]", t1->maxhp);
	    else if(t1->maxhp < 100)
		 Sprintf(eos(bp), "  [%d]", t1->maxhp);
	    else Sprintf(eos(bp), " [%d]", t1->maxhp);
	  }
	}
	if(so == 0) (void) puts(linebuf);
	else if(so > 0) {
	  register char *bp = eos(linebuf);
	  if(so >= COLNO) so = COLNO-1;
	  while(bp < linebuf + so) *bp++ = ' ';
	  *bp = 0;
	  standoutbeg();
	  (void) fputs(linebuf,stdout);
	  standoutend();
	  (void) putchar('\n');
	}
	return(strlen(linebuf));
}

static char *
itoa(a) int a; {
#ifdef LINT	/* static char buf[12]; */
char buf[12];
#else
static char buf[12];
#endif
	Sprintf(buf,"%d",a);
	return(buf);
}

static char *
ordin(n)
int n; {
	register int dd = n%10;

#if ENTRYMAX > 110
	return((dd==0 || dd>3 || (n/10)%10==1) ? "th" :
#else
	return((dd==0 || dd>3 || n/10==1) ? "th" :
#endif
	       (dd==1) ? "st" : (dd==2) ? "nd" : "rd");
}

char *
eos(s)
register char *s;
{
	while(*s) s++;
	return(s);
}

/*
 * Called with args from main if argc >= 0. In this case, list scores as
 * requested. Otherwise, find scores for the current player (and list them
 * if argc == -1).
 */
void
prscore(argc,argv)
int argc;
char **argv;
{
	char **players;
	int playerct;
	int rank;
	register struct toptenentry *t1, *t2;
	char *recfile = RECORD;
	FILE *rfile;
	register int flg = 0, i;
#ifdef nonsense
	long total_score = 0L;
	char totchars[10];
	int totcharct = 0;
#endif
	int outflg = (argc >= -1);
#ifdef PERS_IS_UID
	int uid = -1;
#else
	char *player0;
#endif

	if(!(rfile = fopen(recfile,"r"))){
		(void) puts("Cannot open record file!");
		return;
	}

	if(argc > 1 && !strncmp(argv[1], "-s", 2)){
		if(!argv[1][2]){
			argc--;
			argv++;
		} else if(!argv[1][3] && index("ABCEHKPRSTVW", argv[1][2])) {
			argv[1]++;
			argv[1][0] = '-';
		} else	argv[1] += 2;
	}
	if(argc <= 1){
#ifdef PERS_IS_UID
		uid = getuid();
		playerct = 0;
#else
		player0 = plname;
		if(!*player0)
			player0 = "hackplayer";
		playerct = 1;
		players = &player0;
#endif
	} else {
		playerct = --argc;
		players = ++argv;
	}
	if(outflg) (void) putchar('\n');

	t1 = tt_head = newttentry();
	for(rank = 1; ; rank++) {
#ifdef OLD_TOS
	  char k1[2], k2[2];
	  if(fscanf(rfile, "%6s %d %d %d %d %d %ld %1s%1s %s %s]",
#else
	  if(fscanf(rfile, "%6s %d %d %d %d %d %ld %c%c %[^,],%[^\n]",
#endif
		t1->date, &t1->uid,
		&t1->level, &t1->maxlvl,
		&t1->hp, &t1->maxhp, &t1->points,
#ifdef OLD_TOS
		k1, k2,
#else
		&t1->plchar, &t1->sex,
#endif
		t1->name, t1->death) != 11)
			t1->points = 0;
	  if(t1->points == 0) break;
#ifdef OLD_TOS
	  t1->plchar=k1[0];
	  t1->sex=k2[0];
#endif
#ifdef PERS_IS_UID
	  if(!playerct && t1->uid == uid)
		flg++;
	  else
#endif
	  for(i = 0; i < playerct; i++){
		if(strcmp(players[i], "all") == 0 ||
		   strncmp(t1->name, players[i], NAMSZ) == 0 ||
		  (players[i][0] == '-' &&
		   players[i][1] == t1->plchar &&
		   players[i][2] == 0) ||
		  (digit(players[i][0]) && rank <= atoi(players[i])))
			flg++;
	  }
	  t1 = t1->tt_next = newttentry();
	}
	(void) fclose(rfile);
	if(!flg) {
	    if(outflg) {
		Printf("Cannot find any entries for ");
		if(playerct < 1) Printf("you.\n");
		else {
		  if(playerct > 1) Printf("any of ");
		  for(i=0; i<playerct; i++)
			Printf("%s%s", players[i], (i<playerct-1)?", ":".\n");
		  Printf("Call is: %s -s [-role] [maxrank] [playernames]\n", hname);
		}
	    }
	    return;
	}

	if(outflg) outheader();
	t1 = tt_head;
	for(rank = 1; t1->points != 0; rank++, t1 = t2) {
		t2 = t1->tt_next;
#ifdef PERS_IS_UID
		if(!playerct && t1->uid == uid)
			goto outwithit;
		else
#endif
		for(i = 0; i < playerct; i++){
			if(strcmp(players[i], "all") == 0 ||
			   strncmp(t1->name, players[i], NAMSZ) == 0 ||
			  (players[i][0] == '-' &&
			   players[i][1] == t1->plchar &&
			   players[i][2] == 0) ||
			  (digit(players[i][0]) && rank <= atoi(players[i]))){
			outwithit:
				if(outflg)
				    (void) outentry(rank, t1, 0);
#ifdef nonsense
				total_score += t1->points;
				if(totcharct < sizeof(totchars)-1)
				    totchars[totcharct++] = t1->plchar;
#endif
				break;
			}
		}
		free((genericptr_t) t1);
	}
#ifdef nonsense
	totchars[totcharct] = 0;

	/* We would like to determine whether he is experienced. However,
	   the information collected here only tells about the scores/roles
	   that got into the topten (top 100?). We should maintain a
	   .hacklog or something in his home directory. */
	flags.beginner = (total_score < 6000);
	for(i=0; i<6; i++)
	    if(!index(totchars, "ABCEHKPRSTVW"[i])) {
		flags.beginner = 1;
		if(!pl_character[0]) pl_character[0] = "ABCEHKPRSTVW"[i];
		break;
	}
#endif /* nonsense /**/
}

static int
classmon(plch, fem)
char plch;
boolean fem;
{
	switch (plch) {
		case 'A': return PM_ARCHEOLOGIST;
		case 'B': return PM_BARBARIAN;
		case 'C': return (fem ? PM_CAVEWOMAN : PM_CAVEMAN);
		case 'E': return PM_ELF;
		case 'H': return PM_HEALER;
		case 'F':	/* accept old Fighter class */
		case 'K': return PM_KNIGHT;
		case 'P': return (fem ? PM_PRIESTESS : PM_PRIEST);
		case 'R': return PM_ROGUE;
		case 'N':	/* accept old Ninja class */
		case 'S': return PM_SAMURAI;
		case 'T': return PM_TOURIST;
		case 'V': return PM_VALKYRIE;
		case 'W': return PM_WIZARD;
		default: impossible("What weird class is this? (%c)", plch);
			return PM_HUMAN_ZOMBIE;
	}
}

/*
 * Get a random player name and class from the high score list,
 * and attach them to an object (for statues or morgue corpses).
 */
struct obj *
tt_oname(otmp)
struct obj *otmp;
{
	int rank;
	register int i;
	register struct toptenentry *tt;
	char *recfile = RECORD;
	FILE *rfile;

	if (!otmp) return((struct obj *) 0);

	if(!(rfile = fopen(recfile,"r")))
		panic("Cannot open record file!");

	tt = newttentry();
	rank = rnd(10);
pickentry:
	for(i = rank; i; i--) {
#ifdef OLD_TOS
	  char k1[2], k2[2];
	  if(fscanf(rfile, "%6s %d %d %d %d %d %ld %1s%1s %s %s]",
#else
	  if(fscanf(rfile, "%6s %d %d %d %d %d %ld %c%c %[^,],%[^\n]",
#endif
		tt->date, &tt->uid,
		&tt->level, &tt->maxlvl,
		&tt->hp, &tt->maxhp, &tt->points,
#ifdef OLD_TOS
		k1, k2,
#else
		&tt->plchar, &tt->sex,
#endif
		tt->name, tt->death) != 11)
			tt->points = 0;
	  if(tt->points == 0) break;
#ifdef OLD_TOS
	  tt->plchar=k1[0];
	  tt->sex=k2[0];
#endif
	}
	(void) fclose(rfile);

	if(tt->points == 0) {
		if(rank > 1) {
			rank = 1;
			goto pickentry;
		}
		free((genericptr_t) tt);
		return((struct obj *) 0);
	} else {
		otmp->corpsenm = classmon(tt->plchar, (tt->sex == 'F'));
		otmp->owt = mons[otmp->corpsenm].cwt;
		otmp = oname(otmp, tt->name, 0);
		free((genericptr_t) tt);
		return otmp;
	}
}
