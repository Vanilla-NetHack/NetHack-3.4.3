/*	SCCS Id: @(#)topten.c	3.0	91/01/20
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define MONATTK_H	/* comment line for pre-compiled headers */
#define MONFLAG_H	/* comment line for pre-compiled headers */
/* block some unused #defines to avoid overloading some cpp's */
#include "hack.h"

#if defined(UNIX) || defined(VMS)
#include <errno.h>	/* George Barbanis */
extern int errno;
# if defined(NO_FILE_LINKS) && !defined(O_WRONLY)
#include <fcntl.h>	/* Ralf Brown */
# endif
#endif	/* UNIX || VMS */
#include <ctype.h>

#ifdef MACOS
extern short macflags;
extern WindowPtr	HackWindow;
#endif

#ifdef NO_SCAN_BRACK
static void FDECL(nsb_mung_line,(char*));
static void FDECL(nsb_unmung_line,(char*));
#endif

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

static char *FDECL(itoa, (int));
static const char *FDECL(ordin, (int));
static void NDECL(outheader);
static int FDECL(outentry, (int,struct toptenentry *,int));
static void FDECL(readentry, (FILE *,struct toptenentry *));
static void FDECL(writeentry, (FILE *,struct toptenentry *));
static int FDECL(classmon, (CHAR_P,BOOLEAN_P));
static boolean FDECL(onlyspace, (const char *));

/* must fit with end.c */
static const char NEARDATA *killed_by_prefix[] = {
	"killed by ", "choked on ", "poisoned by ", "", "drowned in ",
	"", "crushed to death by ", "petrified by ", "",
	"", "",
	"", "", "" };

static void
readentry(rfile,tt)
FILE *rfile;
struct toptenentry *tt;
{
# ifdef NO_SCAN_BRACK
	if(fscanf(rfile,"%6s %d %d %d %d %d %ld%*c%c%c %s %s",
#  define TTFIELDS 12
# else
	if(fscanf(rfile, "%6s %d %d %d %d %d %ld %c%c %[^,],%[^\n]",
#  define TTFIELDS 11
# endif
		tt->date, &tt->uid,
		&tt->level,
		&tt->maxlvl, &tt->hp, &tt->maxhp, &tt->points,
		&tt->plchar, &tt->sex,
#ifdef LATTICE	/* return value is broken also, sigh */
		tt->name, tt->death) < 1)
#else
		tt->name, tt->death) != TTFIELDS)
#endif
#undef TTFIELDS
			tt->points = 0;
#ifdef NO_SCAN_BRACK
	if(tt->points > 0) {
		nsb_unmung_line(tt->name);
		nsb_unmung_line(tt->death);
	}
#endif
}

static void
writeentry(rfile,tt)
FILE *rfile;
struct toptenentry *tt;
{
#ifdef NO_SCAN_BRACK
	nsb_mung_line(tt->name);
	nsb_mung_line(tt->death);
#endif
# ifdef NO_SCAN_BRACK
	(void) fprintf(rfile,"%6s %d %d %d %d %d %ld %c%c %s %s\n",
# else
	(void) fprintf(rfile,"%6s %d %d %d %d %d %ld %c%c %s,%s\n",
# endif
		tt->date, tt->uid,
		tt->level,
		tt->maxlvl, tt->hp, tt->maxhp, tt->points,
		tt->plchar, tt->sex,
		onlyspace(tt->name) ? "_" : tt->name, tt->death);
#ifdef NO_SCAN_BRACK
	nsb_unmung_line(tt->name);
	nsb_unmung_line(tt->death);
#endif
}

static boolean
onlyspace(s)
const char *s;
{
	for (;*s;s++) if (!isspace(*s)) return(FALSE);
	return(TRUE);
}

void
topten(how)
int how;
{
	int uid = getuid();
	int rank, rank0 = -1, rank1 = 0;
	int occ_cnt = PERSMAX;
	register struct toptenentry *t0, *tprev;
	register struct toptenentry *t1;
#ifdef UNIX
	char *reclock = "record_lock";
# ifdef NO_FILE_LINKS
	int lockfd ;
# endif
#endif /* UNIX */
#ifdef VMS
	const char *reclock = "record.lock;1";
	char recfile[] = RECORD;
#else
	const char *recfile = RECORD;
#endif
#if defined(UNIX) || defined(VMS)
	int sleepct = 100;
#endif
	FILE *rfile;
	register int flg = 0;
#ifdef LOGFILE
	char *lgfile = LOGFILE;
	FILE *lfile;
# ifdef UNIX
	char *loglock = "logfile_lock";
# endif /* UNIX */
# ifdef VMS
	const char *loglock = "logfile.lock;1";
# endif /* VMS */
# if defined(UNIX) || defined(VMS)
	int sleeplgct = 30;
# endif /* UNIX or VMS */
#endif /* LOGFILE */

#if defined(MSDOS) || defined(MACOS)
#define HUP
#else
#define	HUP	if(!done_hup)
#endif
#ifdef MACOS
	macflags &= ~fDoUpdate;
	uid = TickCount();
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
	(t0->death)[0] = 0;
	switch (killer_format) {
		default: impossible("bad killer format?");
		case KILLED_BY_AN:
			Strcat(t0->death, killed_by_prefix[how]),
			(void) strncat(t0->death, an(killer), DTHSZ);
			break;
		case KILLED_BY:
			Strcat(t0->death, killed_by_prefix[how]),
			(void) strncat(t0->death, killer, DTHSZ);
			break;
		case NO_KILLER_PREFIX:
			(void) strncat(t0->death, killer, DTHSZ);
			break;
	}
	Strcpy(t0->date, getdate());

#ifdef LOGFILE		/* used for debugging (who dies of what, where) */
# if defined(UNIX) || defined(VMS)
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
#  if defined(SYSV) || defined(ULTRIX) || defined(VMS)
		(void)
#  endif
		    sleep(1);
	}
# endif /* UNIX or VMS */
	if(!(lfile = fopen(lgfile,"a"))){
		HUP (void) puts("Cannot open log file!");
		goto lgend;
	}
	writeentry(lfile, t0);
	(void) fclose(lfile);
# if defined(UNIX) || defined(VMS)
	(void) unlink(loglock);
# endif /* UNIX or VMS */
 lgend: ;
# ifdef NO_FILE_LINKS
	(void) close(lockfd) ;
	free((genericptr_t) loglock) ;
# endif
#endif /* LOGFILE */

#if defined(WIZARD) || defined(EXPLORE_MODE)
	if (wizard || discover) {
 Printf("\nSince you were in %s mode, the score list will not be checked.\n",
	wizard ? "wizard" : "discover");
		return;
	}
#endif

#if defined(UNIX) || defined(VMS)
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
# if defined(SYSV) || defined(ULTRIX) || defined(VMS)
		(void)
# endif
		    sleep(1);
	}
#endif /* UNIX or VMS */
#ifdef MACOS
	{
		term_info	*t;
		
		t = (term_info *)GetWRefCon(HackWindow);
		SetVol((StringPtr)0L, t->recordVRefNum);
		if (!(rfile = fopen(recfile,"r")))
			rfile = openFile(recfile,"r");
	}
	if (!rfile) {
#else
	if(!(rfile = fopen(recfile,"r"))){
#endif
		HUP (void) puts("Cannot open record file!");
		goto unlock;
	}
	HUP (void) putchar('\n');

	/* assure minimum number of points */
	if(t0->points < POINTSMIN) t0->points = 0;

	t1 = tt_head = newttentry();
	tprev = 0;
	/* rank0: -1 undefined, 0 not_on_list, n n_th on list */
	for(rank = 1; ; ) {
	    readentry(rfile, t1);
	    if (t1->points < POINTSMIN) t1->points = 0;
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
#ifdef VMS
	    {	char *semi_colon = rindex(recfile, ';');
		if (semi_colon) *semi_colon = '\0';
	    }
#endif
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
	    if(flg) writeentry(rfile, t1);
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
#ifdef MACOS
	{
		Str255	name;
		FInfo	fndrInfo;
		term_info	*t;
		short	oldVol, error;
		
		t = (term_info *)GetWRefCon(HackWindow);
		GetVol(name, &oldVol);
		SetVol(0L, t->recordVRefNum);
		Strcpy((char *)name,recfile);
		CtoPstr((char *)name);
		error = GetFInfo(name, (short)0, &fndrInfo);
		fndrInfo.fdCreator = CREATOR;
		if (error == noErr)
			SetFInfo(name, (short)0, &fndrInfo);
		SetVol(0L, oldVol);
	}
#endif
#ifdef VMS
	if (flg) {
		delete(RECORD);
		rename(recfile, RECORD);
	}
# undef unlink
#endif
unlock:	;
#if defined(UNIX) || defined(VMS)
	if (unlink(reclock) < 0)
		Printf("Can't unlink %s\n",reclock) ;
# ifdef NO_FILE_LINKS
	(void) close(lockfd) ;
	free((genericptr_t) reclock) ;
# endif
#endif
}

static void
outheader() {
	char linebuf[BUFSZ];
	register char *bp;

	Strcpy(linebuf, " No  Points     Name");
	bp = eos(linebuf);
	while(bp < linebuf + COLNO - 9) *bp++ = ' ';
	Strcpy(bp, "Hp [max]");
	(void) puts(linebuf);
#ifdef MACOS
	putchar('\n');
#endif
}

/* so>0: standout line; so=0: ordinary line; so<0: no output, return lth */
static int
outentry(rank, t1, so)
register struct toptenentry *t1;
register int rank, so;
{
	register boolean second_line = TRUE;
	char linebuf[BUFSZ], linebuf2[BUFSZ];

	linebuf[0] = linebuf2[0] = 0;
	if(rank) Sprintf(eos(linebuf), "%3d", rank);
	else Strcat(linebuf, "   ");

	Sprintf(eos(linebuf), " %10ld  %.10s", t1->points, t1->name);
	Sprintf(eos(linebuf), "-%c ", t1->plchar);
	if(!strncmp("escaped", t1->death, 7)) {
	  second_line = FALSE;
	  if(!strcmp(" (with the Amulet)", t1->death+7))
	    Strcat(linebuf, "escaped the dungeon with the Amulet");
	  else
	    Sprintf(eos(linebuf), "escaped the dungeon [max level %d]",
	      t1->maxlvl);
#ifdef ENDGAME
	} else if(!strncmp("ascended", t1->death, 8)) {
	   Strcat(linebuf, "ascended to demigod");
	   if (t1->sex == 'F') Strcat(linebuf, "dess");
	   Strcat(linebuf, "-hood");
	   second_line = FALSE;
#endif
	} else {
	  if(!strncmp(t1->death,"quit",4)) {
		Strcat(linebuf, "quit");
		second_line = FALSE;
	  } else if(!strncmp(t1->death,"starv",5)) {
		Strcat(linebuf, "starved to death");
		second_line = FALSE;
	  } else if(!strncmp(t1->death,"choked",6)) {
		Sprintf(eos(linebuf), "choked on h%s food",
			(t1->sex == 'F') ? "er" : "is");
	  } else if(!strncmp(t1->death,"poisoned",8)) {
		Strcat(linebuf, "was poisoned");
	  } else if(!strncmp(t1->death,"crushed",7)) {
		Strcat(linebuf, "was crushed to death");
	  } else if(!strncmp(t1->death, "petrified by ",13)) {
		Strcat(linebuf, "turned to stone");
	  } else Strcat(linebuf, "died");
#ifdef ENDLEVEL
	  if (t1->level == ENDLEVEL)
		Strcat(linebuf, " in the endgame");
	  else
#endif
	    Sprintf(eos(linebuf), " on dungeon level %d", t1->level);
	  if(t1->maxlvl != t1->level)
	    Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
	/* kludge for "quit while already on Charon's boat" */
	  if(!strncmp(t1->death, "quit ", 5))
	    Strcat(linebuf, t1->death + 4);
	}
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
#ifdef MACOS
	    while(bp <= linebuf + hppos) *bp++ = ' ';
#else
	    while(bp < linebuf + hppos) *bp++ = ' ';
#endif
	    Strcpy(bp, hpbuf);
	    if(t1->maxhp < 10)
		 Sprintf(eos(bp), "   [%d]", t1->maxhp);
	    else if(t1->maxhp < 100)
		 Sprintf(eos(bp), "  [%d]", t1->maxhp);
	    else Sprintf(eos(bp), " [%d]", t1->maxhp);
	  }
	}

/*	Line 2 now contains the killer name */

	/* Quit, starved, ascended, and escaped contain no second line */
	if (second_line) {
		Strcpy(linebuf2, t1->death);
		if (islower(*linebuf2)) *linebuf2 = toupper(*linebuf2);
		Strcat(linebuf2, ".");
	}

	if(so == 0) {
	  (void) puts(linebuf);
	  if (second_line)
		(void) Printf("                %s\n", linebuf2);
	} else if(so > 0) {
	  register char *bp = eos(linebuf);
	  if(so >= COLNO) so = COLNO-1;
	  while(bp < linebuf + so) *bp++ = ' ';
	  *bp = 0;
	  standoutbeg();
	  (void) puts(linebuf);
	  if(second_line)
		(void) Printf("                %s", linebuf2);
	  standoutend();
	  if(second_line)
		(void) putchar('\n');
	}
	return(strlen(linebuf)+strlen(linebuf2));
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

static const char *
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
	const char **players;
	int playerct;
	int rank;
	register struct toptenentry *t1, *t2;
	const char *recfile = RECORD;
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
	const char *player0;
#endif
#ifdef MACOS
	if(!(rfile = fopen(recfile,"r")))
		rfile = openFile(recfile,"r");
	if (!rfile) {
#else
	if(!(rfile = fopen(recfile,"r"))){
#endif
		(void) puts("Cannot open record file!");
		return;
	}

	if(argc > 1 && !strncmp(argv[1], "-s", 2)){
		if(!argv[1][2]){
			argc--;
			argv++;
		} else if(!argv[1][3] && index(pl_classes, argv[1][2])) {
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
		players = (const char **)++argv;
	}
	if(outflg) (void) putchar('\n');

	t1 = tt_head = newttentry();
	for(rank = 1; ; rank++) {
	    readentry(rfile, t1);
	    if(t1->points == 0) break;
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
#ifdef PERS_IS_UID
			outwithit:
#endif
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
	    if(!index(totchars, pl_classes[i])) {
		flags.beginner = 1;
		if(!pl_character[0]) pl_character[0] = pl_classes[i];
		break;
	}
#endif /* nonsense /**/
#ifdef MACOS
	more();
#endif
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
	const char *recfile = RECORD;
	FILE *rfile;

	if (!otmp) return((struct obj *) 0);

#ifdef MACOS
	if(!(rfile = fopen(recfile,"r")))
		rfile = openFile(recfile, "r");
	if (!rfile) {
#else
	if(!(rfile = fopen(recfile,"r"))){
#endif
		panic("Cannot open record file!");
	}

	tt = newttentry();
	rank = rnd(10);
pickentry:
	for(i = rank; i; i--) {
	    readentry(rfile, tt);
	    if(tt->points == 0) break;
	}

	if(tt->points == 0) {
		if(rank > 1) {
			rank = 1;
			goto pickentry;
		}
		free((genericptr_t) tt);
		otmp = (struct obj *) 0;
	} else {
		otmp->corpsenm = classmon(tt->plchar, (tt->sex == 'F'));
		otmp->owt = weight(otmp);
		/* Note: oname() is safe since otmp is first in chains */
		otmp = oname(otmp, tt->name, 0);
		fobj = otmp;
		level.objects[otmp->ox][otmp->oy] = otmp;
		free((genericptr_t) tt);
	}

	(void) fclose(rfile);
	return otmp;
}

#ifdef NO_SCAN_BRACK
/* Lattice scanf isn't up to reading the scorefile.  What */
/* follows deals with that; I admit it's ugly. (KL) */
/* Now generally available (KL) */
static void nsb_mung_line(p)
	char *p;
	{
	while(p=strchr(p,' '))*p='|';
}
static void nsb_unmung_line(p)
	char *p;
	{
	while(p=strchr(p,'|'))*p=' ';
}
#endif
