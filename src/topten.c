/*	SCCS Id: @(#)topten.c	3.1	92/11/20	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef VMS
 /* We don't want to rewrite the whole file, because that entails	 */
 /* creating a new version which requires that the old one be deletable. */
# define UPDATE_RECORD_IN_PLACE
#endif

/*
 * Updating in place can leave junk at the end of the file in some
 * circumstances (if it shrinks and the O.S. doesn't have a straightforward
 * way to truncate it).  The trailing junk is harmless and the code
 * which reads the scores will ignore it.
 */
#ifdef UPDATE_RECORD_IN_PLACE
# ifndef SEEK_SET
#  define SEEK_SET 0
# endif
static long final_fpos;
#endif

#ifdef NO_SCAN_BRACK
static void FDECL(nsb_mung_line,(char*));
static void FDECL(nsb_unmung_line,(char*));
#endif

#define newttentry() (struct toptenentry *) alloc(sizeof(struct toptenentry))
#define dealloc_ttentry(ttent) free((genericptr_t) (ttent))
#define	NAMSZ	10
#define	DTHSZ	60
#define	PERSMAX	 3		/* entries per name/uid per char. allowed */
#define	POINTSMIN	1	/* must be > 0 */
#define	ENTRYMAX	100	/* must be >= 10 */

#if !defined(MICRO) && !defined(MAC)
#define	PERS_IS_UID		/* delete for PERSMAX per name; now per uid */
#endif
struct toptenentry {
	struct toptenentry *tt_next;
#ifdef UPDATE_RECORD_IN_PLACE
	long fpos;
#endif
	long points;
	int deathdnum, deathlev;
	int maxlvl,hp,maxhp;
	int uid;
	char plchar;
	char sex;
	char name[NAMSZ+1];
	char death[DTHSZ+1];
	char date[7];		/* yymmdd */
} *tt_head;

static void NDECL(outheader);
static int FDECL(outentry, (int,struct toptenentry *,int));
static void FDECL(readentry, (FILE *,struct toptenentry *));
static void FDECL(writeentry, (FILE *,struct toptenentry *));
static int FDECL(classmon, (CHAR_P,BOOLEAN_P));

/* must fit with end.c */
NEARDATA const char *killed_by_prefix[] = {
	"killed by ", "choked on ", "poisoned by ", "", "drowned in ",
	"", "crushed to death by ", "petrified by ", "",
	"", "",
	"", "", "" };

static void
readentry(rfile,tt)
FILE *rfile;
struct toptenentry *tt;
{
#ifdef UPDATE_RECORD_IN_PLACE
	/* note: fscanf() below must read the record's terminating newline */
	final_fpos = tt->fpos = ftell(rfile);
#endif
#define TTFIELDS 12
#ifdef NO_SCAN_BRACK
	if(fscanf(rfile,"%6s %d %d %d %d %d %d %ld%*c%c%c %s %s%*c",
#else
	if(fscanf(rfile, "%6s %d %d %d %d %d %d %ld %c%c %[^,],%[^\n]%*c",
#endif
			tt->date, &tt->uid,
			&tt->deathdnum, &tt->deathlev,
			&tt->maxlvl, &tt->hp, &tt->maxhp, &tt->points,
			&tt->plchar, &tt->sex,
			tt->name, tt->death) != TTFIELDS)
#undef TTFIELDS
		tt->points = 0;
	else {
#ifdef NO_SCAN_BRACK
		if(tt->points > 0) {
			nsb_unmung_line(tt->name);
			nsb_unmung_line(tt->death);
		}
#endif
	}
}

static void
writeentry(rfile,tt)
FILE *rfile;
struct toptenentry *tt;
{
#ifdef NO_SCAN_BRACK
	nsb_mung_line(tt->name);
	nsb_mung_line(tt->death);
	(void) fprintf(rfile,"%6s %d %d %d %d %d %d %ld %c%c %s %s\n",
#else
	(void) fprintf(rfile,"%6s %d %d %d %d %d %d %ld %c%c %s,%s\n",
#endif
		tt->date, tt->uid,
		tt->deathdnum, tt->deathlev,
		tt->maxlvl, tt->hp, tt->maxhp, tt->points,
		tt->plchar, tt->sex,
		onlyspace(tt->name) ? "_" : tt->name, tt->death);
#ifdef NO_SCAN_BRACK
	nsb_unmung_line(tt->name);
	nsb_unmung_line(tt->death);
#endif
}

void
topten(how)
int how;
{
	int uid = getuid();
	int rank, rank0 = -1, rank1 = 0;
	int occ_cnt = PERSMAX;
	register struct toptenentry *t0, *tprev;
	struct toptenentry *t1;
	FILE *rfile;
	register int flg = 0;
#ifdef LOGFILE
	FILE *lfile;
#endif /* LOGFILE */

#if defined(MICRO)
#define HUP
#else
#define	HUP	if(!done_hup)
#endif

#ifdef TOS
	restore_colors();	/* make sure the screen is black on white */
#endif
	/* create a new 'topten' entry */
	t0 = newttentry();
	/* deepest_lev_reached() is in terms of depth(), and reporting the
	 * deepest level reached in the dungeon death occurred in doesn't
	 * seem right, so we have to report the death level in depth() terms
	 * as well (which also seems reasonable since that's all the player
	 * sees on the screen anyway)
	 */
	t0->deathdnum = u.uz.dnum;
	t0->deathlev = depth(&u.uz);
	t0->maxlvl = deepest_lev_reached(TRUE);
	t0->hp = u.uhp;
	t0->maxhp = u.uhpmax;
	t0->points = u.urexp;
	t0->plchar = pl_character[0];
	t0->sex = (flags.female ? 'F' : 'M');
	t0->uid = uid;
	(void) strncpy(t0->name, plname, NAMSZ);
	t0->name[NAMSZ] = '\0';
	t0->death[0] = '\0';
	switch (killer_format) {
		default: impossible("bad killer format?");
		case KILLED_BY_AN:
			Strcat(t0->death, killed_by_prefix[how]);
			(void) strncat(t0->death, an(killer),
						DTHSZ-strlen(t0->death));
			break;
		case KILLED_BY:
			Strcat(t0->death, killed_by_prefix[how]);
			(void) strncat(t0->death, killer,
						DTHSZ-strlen(t0->death));
			break;
		case NO_KILLER_PREFIX:
			(void) strncat(t0->death, killer, DTHSZ);
			break;
	}
	Strcpy(t0->date, get_date());
	t0->tt_next = 0;
#ifdef UPDATE_RECORD_IN_PLACE
	t0->fpos = -1L;
#endif

#ifdef LOGFILE		/* used for debugging (who dies of what, where) */
	if (lock_file(LOGFILE, 10)) {
	    if(!(lfile = fopen_datafile(LOGFILE,"a"))) {
		HUP raw_print("Cannot open log file!");
	    } else {
		writeentry(lfile, t0);
		(void) fclose(lfile);
	    }
	    unlock_file(LOGFILE);
	}
#endif /* LOGFILE */

#if defined(WIZARD) || defined(EXPLORE_MODE)
	if (wizard || discover) {
	    HUP {
		raw_print("");
		raw_printf(
	      "Since you were in %s mode, the score list will not be checked.",
		    wizard ? "wizard" : "discover");
	    }
	    return;
	}
#endif

	if (!lock_file(RECORD, 60)) return;

#ifdef UPDATE_RECORD_IN_PLACE
	rfile = fopen_datafile(RECORD, "r+");
#else
	rfile = fopen_datafile(RECORD, "r");
#endif

	if (!rfile) {
		HUP raw_print("Cannot open record file!");
		unlock_file(RECORD);
		return;
	}

	HUP raw_print("");

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
#ifdef UPDATE_RECORD_IN_PLACE
		t0->fpos = t1->fpos;	/* insert here */
#endif
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
			HUP {
			    raw_printf(
			  "You didn't beat your previous score of %ld points.",
				    t1->points);
			    raw_print("");
			}
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
#ifdef UPDATE_RECORD_IN_PLACE
		(void) fseek(rfile, (t0->fpos >= 0 ?
				     t0->fpos : final_fpos), SEEK_SET);
#else
		(void) fclose(rfile);
		if(!(rfile = fopen_datafile(RECORD,"w"))){
			HUP raw_print("Cannot write record file");
			unlock_file(RECORD);
			return;
		}
#endif	/* UPDATE_RECORD_IN_PLACE */
		if(!done_stopprint) if(rank0 > 0){
		    if(rank0 <= 10)
			raw_print("You made the top ten list!");
		    else {
			raw_printf(
			  "You reached the %d%s place on the top %d list.",
				rank0, ordin(rank0), ENTRYMAX);
		    }
		    raw_print("");
		}
	}
	if(rank0 == 0) rank0 = rank1;
	if(rank0 <= 0) rank0 = rank;
	if(!done_stopprint) outheader();
	t1 = tt_head;
	for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
	    if(flg
#ifdef UPDATE_RECORD_IN_PLACE
		    && rank >= rank0
#endif
		) writeentry(rfile, t1);
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
	  	raw_print("");
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
#ifdef UPDATE_RECORD_IN_PLACE
	if (flg) {
# ifdef TRUNCATE_FILE
		/* if a reasonable way to truncate a file exists, use it */
		truncate_file(rfile);
# else
		/* use sentinel record rather than relying on truncation */
		t0->points = 0L;	/* terminates file when read back in */
		t0->uid = t0->deathdnum = t0->deathlev = 0;
		t0->maxlvl = t0->hp = t0->maxhp = 0;
		t0->plchar = t0->sex = '-';
		Strcpy(t0->name, "@");
		Strcpy(t0->death, "<eod>\n");
		writeentry(rfile, t0);
		(void) fflush(rfile);
# endif	/* TRUNCATE_FILE */
	}
#endif	/* UPDATE_RECORD_IN_PLACE */
	(void) fclose(rfile);
	unlock_file(RECORD);
}

static void
outheader() {
	char linebuf[BUFSZ];
	register char *bp;

	Strcpy(linebuf, " No  Points     Name");
	bp = eos(linebuf);
	while(bp < linebuf + COLNO - 9) *bp++ = ' ';
	Strcpy(bp, "Hp [max]");
	raw_print(linebuf);
}

/* so>0: standout line; so=0: ordinary line; so<0: no output, return lth */
static int
outentry(rank, t1, so)
register struct toptenentry *t1;
register int rank, so;
{
	register boolean second_line = TRUE;
	char linebuf[BUFSZ], linebuf2[BUFSZ], linebuf3[BUFSZ], pbuf[BUFSZ];

	linebuf[0] = linebuf2[0] = linebuf3[0] = 0;
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
	} else if(!strncmp("ascended", t1->death, 8)) {
	   Strcat(linebuf, "ascended to demigod");
	   if (t1->sex == 'F') Strcat(linebuf, "dess");
	   Strcat(linebuf, "-hood");
	   second_line = FALSE;
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

	  if (t1->deathdnum == astral_level.dnum)
		Strcpy(linebuf3, " in the endgame");
	  else
		Sprintf(linebuf3, " in %s on level %d",
		    dungeons[t1->deathdnum].dname, t1->deathlev);
	  if(t1->deathlev != t1->maxlvl)
		Sprintf(eos(linebuf3), " [max %d]", t1->maxlvl);
	  /* kludge for "quit while already on Charon's boat" */
	  if(!strncmp(t1->death, "quit ", 5))
		Strcat(linebuf3, t1->death + 4);
	}
	Strcat(linebuf3, ".");

	if(t1->maxhp) {
	  register char *bp;
	  char hpbuf[10];
	  int hppos;
	  int lngr = strlen(linebuf) + strlen(linebuf3);
	  if (t1->hp <= 0) hpbuf[0] = '-', hpbuf[1] = '\0';
	  else Sprintf(hpbuf, "%d", t1->hp);
	  hppos = COLNO - 7 - (int)strlen(hpbuf);
	  if (lngr >= hppos) {
	      if(so > 0) {
		  bp = eos(linebuf);
		  while(bp < linebuf + (COLNO-1)) *bp++ = ' ';
		  *bp = 0;
		  raw_print_bold(linebuf);
	      } else if(so == 0)
		  raw_print(linebuf);
	      Strcpy(linebuf, "               ");
	  }
	  Strcat(linebuf, linebuf3);
	  bp = eos(linebuf);

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

/*	Line 2 now contains the killer name */

	/* Quit, starved, ascended, and escaped contain no second line */
	if (second_line) {
		Strcpy(linebuf2, t1->death);
		*linebuf2 = highc(*linebuf2);
		Strcat(linebuf2, ".");
	}

	if(so == 0) {
	    raw_print(linebuf);
	    if (second_line)
		raw_printf("                %s", linebuf2);
	} else if(so > 0) {
	  register char *bp = eos(linebuf);
	  if(so >= COLNO) so = COLNO-1;
	  while(bp < linebuf + so) *bp++ = ' ';
	  *bp = 0;
	  raw_print_bold(linebuf);
	  if(second_line) {
	      Sprintf(pbuf, "                %s", linebuf2);
	      raw_print_bold(pbuf);
	  }
	}
	return((int)strlen(linebuf)+(int)strlen(linebuf2));
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
	FILE *rfile;
	register int flg = 0, i;
	char pbuf[BUFSZ];
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
	rfile = fopen_datafile(RECORD, "r");
	if (!rfile) {
		raw_print("Cannot open record file!");
		return;
	}

#ifdef	AMIGA
	{
	    extern winid amii_rawprwin;
	    init_nhwindows();
	    amii_rawprwin = create_nhwindow( NHW_TEXT );
	}
#endif

	/* If the score list isn't after a game, we never went through */
	/* init_dungeons() */
	if (wiz1_level.dlevel == 0) init_dungeons();

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
#  if defined(LINT) || defined(GCC_WARN)
		players = 0;
#  endif
#else
		player0 = plname;
		if(!*player0)
#ifdef AMIGA
			player0 = "all";	/* single user system */
#else
			player0 = "hackplayer";
#endif
		playerct = 1;
		players = &player0;
#endif
	} else {
		playerct = --argc;
		players = (const char **)++argv;
	}
	if(outflg) raw_print("");

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
		Strcpy(pbuf, "Cannot find any entries for ");
		if(playerct < 1) Strcat(pbuf, "you.");
		else {
		  if(playerct > 1) Strcat(pbuf, "any of ");
		  for(i=0; i<playerct; i++) {
		      Strcat(pbuf, players[i]);
		      if(i<playerct-1) Strcat(pbuf, ":");
		  }
		  raw_print(pbuf);
		  raw_printf("Call is: %s -s [-role] [maxrank] [playernames]",
			     hname);
		}
	    }
#ifdef	AMIGA
	    display_nhwindow( amii_rawprwin, 1 );
	    destroy_nhwindow( amii_rawprwin );
	    amii_rawprwin = WIN_ERR;
#endif
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
		dealloc_ttentry(t1);
	}
#ifdef nonsense
	totchars[totcharct] = 0;

	/* We would like to determine whether you're experienced.  However,
	   the information collected here only tells about the scores/roles
	   that got into the topten (top 100?).  We should maintain a
	   .hacklog or something in his home directory. */
	flags.beginner = (total_score < 6000);
	for(i=0; i<6; i++)
	    if(!index(totchars, pl_classes[i])) {
		flags.beginner = 1;
		if(!pl_character[0]) pl_character[0] = pl_classes[i];
		break;
	}
#endif /* nonsense /**/
#ifdef	AMIGA
	display_nhwindow( amii_rawprwin, 1 );
	destroy_nhwindow( amii_rawprwin );
	amii_rawprwin = WIN_ERR;
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
#ifdef TOURIST
		case 'T': return PM_TOURIST;
#else
		case 'T': return PM_HUMAN;
#endif
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
	FILE *rfile;

	if (!otmp) return((struct obj *) 0);

	rfile = fopen_datafile(RECORD, "r");
	if (!rfile) {
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
			rewind(rfile);
			goto pickentry;
		}
		dealloc_ttentry(tt);
		otmp = (struct obj *) 0;
	} else {
		otmp->corpsenm = classmon(tt->plchar, (tt->sex == 'F'));
		otmp->owt = weight(otmp);
		/* Note: oname() is safe since otmp is first in chains */
		otmp = oname(otmp, tt->name, 0);
		fobj = otmp;
		level.objects[otmp->ox][otmp->oy] = otmp;
		dealloc_ttentry(tt);
	}

	(void) fclose(rfile);
	return otmp;
}

#ifdef NO_SCAN_BRACK
/* Lattice scanf isn't up to reading the scorefile.  What */
/* follows deals with that; I admit it's ugly. (KL) */
/* Now generally available (KL) */
static void
nsb_mung_line(p)
	char *p;
	{
	while(p=index(p,' '))*p='|';
}

static void
nsb_unmung_line(p)
	char *p;
	{
	while(p=index(p,'|'))*p=' ';
}
#endif

/*topten.c*/
