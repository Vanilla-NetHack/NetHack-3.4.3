/*	SCCS Id: @(#)topten.c	3.2	96/05/25	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

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
static long final_fpos;
#endif

#define done_stopprint program_state.stopprint

#define newttentry() (struct toptenentry *) alloc(sizeof(struct toptenentry))
#define dealloc_ttentry(ttent) free((genericptr_t) (ttent))
#define NAMSZ	10
#define DTHSZ	60
#define PERSMAX	 3		/* entries per name/uid per char. allowed */
#define POINTSMIN	1	/* must be > 0 */
#define ENTRYMAX	100	/* must be >= 10 */

#if !defined(MICRO) && !defined(MAC)
#define PERS_IS_UID		/* delete for PERSMAX per name; now per uid */
#endif
struct toptenentry {
	struct toptenentry *tt_next;
#ifdef UPDATE_RECORD_IN_PLACE
	long fpos;
#endif
	long points;
	int deathdnum, deathlev;
	int maxlvl, hp, maxhp, deaths;
	int ver_major, ver_minor, patchlevel;
	long deathdate, birthdate;
	int uid;
	char plchar;
	char sex;
	char name[NAMSZ+1];
	char death[DTHSZ+1];
} *tt_head;

static void FDECL(topten_print, (const char *));
static void FDECL(topten_print_bold, (const char *));
static xchar FDECL(observable_depth, (d_level *));
static void NDECL(outheader);
static void FDECL(outentry, (int,struct toptenentry *,BOOLEAN_P));
static void FDECL(readentry, (FILE *,struct toptenentry *));
static void FDECL(writeentry, (FILE *,struct toptenentry *));
static void FDECL(free_ttlist, (struct toptenentry *));
static int FDECL(classmon, (CHAR_P,BOOLEAN_P));
static int FDECL(score_wanted,
		(BOOLEAN_P, int,struct toptenentry *,int,const char **,int));
#ifdef NO_SCAN_BRACK
static void FDECL(nsb_mung_line,(char*));
static void FDECL(nsb_unmung_line,(char*));
#endif

/* must fit with end.c; used in rip.c */
NEARDATA const char *killed_by_prefix[] = {
	"killed by ", "choked on ", "poisoned by ", "", "drowned in ",
	"", "dissolved in ", "crushed to death by ", "petrified by ", "",
	"", "",
	"", "", ""
};

static winid toptenwin = WIN_ERR;

static void
topten_print(x)
const char *x;
{
	if (toptenwin == WIN_ERR)
	    raw_print(x);
	else
	    putstr(toptenwin, ATR_NONE, x);
}

static void
topten_print_bold(x)
const char *x;
{
	if (toptenwin == WIN_ERR)
	    raw_print_bold(x);
	else
	    putstr(toptenwin, ATR_BOLD, x);
}

static xchar
observable_depth(lev)
d_level *lev;
{
#if 0	/* if we ever randomize the order of the elemental planes, we
	   must use a constant external representation in the record file */
	if (In_endgame(lev)) {
	    if (Is_astralevel(lev))	 return -5;
	    else if (Is_waterlevel(lev)) return -4;
	    else if (Is_firelevel(lev))	 return -3;
	    else if (Is_airlevel(lev))	 return -2;
	    else if (Is_earthlevel(lev)) return -1;
	    else			 return 0;	/* ? */
	} else
#endif
	    return depth(lev);
}

static void
readentry(rfile,tt)
FILE *rfile;
struct toptenentry *tt;
{
#ifdef NO_SCAN_BRACK
	static char *fmt = "%d %d %d %ld %d %d %d %d %d %d %ld %ld %d%*c%c%c %s %s%*c";
#else
	static char *fmt = "%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d %c%c %[^,],%[^\n]%*c";
#endif

#ifdef UPDATE_RECORD_IN_PLACE
	/* note: fscanf() below must read the record's terminating newline */
	final_fpos = tt->fpos = ftell(rfile);
#endif
#define TTFIELDS 17
	if(fscanf(rfile, fmt,
			&tt->ver_major, &tt->ver_minor, &tt->patchlevel,
			&tt->points, &tt->deathdnum, &tt->deathlev,
			&tt->maxlvl, &tt->hp, &tt->maxhp, &tt->deaths,
			&tt->deathdate, &tt->birthdate,
			&tt->uid, &tt->plchar, &tt->sex,
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

	/* check old score entries for Y2K problem and fix whenever found */
	if (tt->points > 0) {
		if (tt->birthdate < 19000000L) tt->birthdate += 19000000L;
		if (tt->deathdate < 19000000L) tt->deathdate += 19000000L;
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
	(void) fprintf(rfile,"%d %d %d %ld %d %d %d %d %d %d %ld %ld %d %c%c %s %s\n",
#else
	(void) fprintf(rfile,"%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d %c%c %s,%s\n",
#endif
		tt->ver_major, tt->ver_minor, tt->patchlevel,
		tt->points, tt->deathdnum, tt->deathlev,
		tt->maxlvl, tt->hp, tt->maxhp, tt->deaths,
		tt->deathdate, tt->birthdate,
		tt->uid, tt->plchar, tt->sex,
		onlyspace(tt->name) ? "_" : tt->name, tt->death);
#ifdef NO_SCAN_BRACK
	nsb_unmung_line(tt->name);
	nsb_unmung_line(tt->death);
#endif
}

static void
free_ttlist(tt)
struct toptenentry *tt;
{
	struct toptenentry *ttnext;

	while (tt->points > 0) {
		ttnext = tt->tt_next;
		dealloc_ttentry(tt);
		tt = ttnext;
	}
	dealloc_ttentry(tt);
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
	boolean t0_used;
#ifdef LOGFILE
	FILE *lfile;
#endif /* LOGFILE */

/* Under DICE 3.0, this crashes the system consistently, apparently due to
 * corruption of *rfile somewhere.  Until I figure this out, just cut out
 * topten support entirely - at least then the game exits cleanly.  --AC
 */
#ifdef _DCC
	return;
#endif

	if (flags.toptenwin) {
	    toptenwin = create_nhwindow(NHW_TEXT);
	}

#if defined(UNIX) || defined(VMS)
#define HUP	if (!program_state.done_hup)
#else
#define HUP
#endif

#ifdef TOS
	restore_colors();	/* make sure the screen is black on white */
#endif
	/* create a new 'topten' entry */
	t0_used = FALSE;
	t0 = newttentry();
	/* deepest_lev_reached() is in terms of depth(), and reporting the
	 * deepest level reached in the dungeon death occurred in doesn't
	 * seem right, so we have to report the death level in depth() terms
	 * as well (which also seems reasonable since that's all the player
	 * sees on the screen anyway)
	 */
	t0->ver_major = VERSION_MAJOR;
	t0->ver_minor = VERSION_MINOR;
	t0->patchlevel = PATCHLEVEL;
	t0->points = u.urexp;
	t0->deathdnum = u.uz.dnum;
	t0->deathlev = observable_depth(&u.uz);
	t0->maxlvl = deepest_lev_reached(TRUE);
	t0->hp = u.uhp;
	t0->maxhp = u.uhpmax;
	t0->deaths = u.umortality;
	t0->uid = uid;
	t0->plchar = pl_character[0];
	t0->sex = (flags.female ? 'F' : 'M');
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
	t0->birthdate = yyyymmdd(u.ubirthday);
	t0->deathdate = yyyymmdd((time_t)0L);
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

	if (wizard || discover) {
	    if (how != PANICKED) HUP {
		char pbuf[BUFSZ];
		topten_print("");
		Sprintf(pbuf,
	      "Since you were in %s mode, the score list will not be checked.",
		    wizard ? "wizard" : "discover");
		topten_print(pbuf);
	    }
	    goto showwin;
	}

	if (!lock_file(RECORD, 60))
		goto destroywin;

#ifdef UPDATE_RECORD_IN_PLACE
	rfile = fopen_datafile(RECORD, "r+");
#else
	rfile = fopen_datafile(RECORD, "r");
#endif

	if (!rfile) {
		HUP raw_print("Cannot open record file!");
		unlock_file(RECORD);
		goto destroywin;
	}

	HUP topten_print("");

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
		t0_used = TRUE;
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
			    char pbuf[BUFSZ];
			    Sprintf(pbuf,
			  "You didn't beat your previous score of %ld points.",
				    t1->points);
			    topten_print(pbuf);
			    topten_print("");
			}
		    }
		    if(occ_cnt < 0) {
			flg++;
			continue;
		    }
		}
	    if(rank <= ENTRYMAX) {
		t1->tt_next = newttentry();
		t1 = t1->tt_next;
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
			free_ttlist(tt_head);
			goto destroywin;
		}
#endif	/* UPDATE_RECORD_IN_PLACE */
		if(!done_stopprint) if(rank0 > 0){
		    if(rank0 <= 10)
			topten_print("You made the top ten list!");
		    else {
			char pbuf[BUFSZ];
			Sprintf(pbuf,
			  "You reached the %d%s place on the top %d list.",
				rank0, ordin(rank0), ENTRYMAX);
			topten_print(pbuf);
		    }
		    topten_print("");
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
	    if (done_stopprint) continue;
	    if (rank > flags.end_top &&
		    (rank < rank0 - flags.end_around ||
		     rank > rank0 + flags.end_around) &&
		    (!flags.end_own ||
#ifdef PERS_IS_UID
					t1->uid != t0->uid
#else
					strncmp(t1->name, t0->name, NAMSZ)
#endif
		)) continue;
	    if (rank == rank0 - flags.end_around &&
		    rank0 > flags.end_top + flags.end_around + 1 &&
		    !flags.end_own)
		topten_print("");
	    if(rank != rank0)
		outentry(rank, t1, FALSE);
	    else if(!rank1)
		outentry(rank, t1, TRUE);
	    else {
		outentry(rank, t1, TRUE);
		outentry(0, t0, TRUE);
	    }
	}
	if(rank0 >= rank) if(!done_stopprint)
		outentry(0, t0, TRUE);
#ifdef UPDATE_RECORD_IN_PLACE
	if (flg) {
# ifdef TRUNCATE_FILE
	    /* if a reasonable way to truncate a file exists, use it */
	    truncate_file(rfile);
# else
	    /* use sentinel record rather than relying on truncation */
	    t1->points = 0L;	/* terminates file when read back in */
	    t1->ver_major = t1->ver_minor = t1->patchlevel = 0;
	    t1->uid = t1->deathdnum = t1->deathlev = 0;
	    t1->maxlvl = t1->hp = t1->maxhp = t1->deaths = 0;
	    t1->plchar = t1->sex = '-';
	    t1->birthdate = t1->deathdate = yyyymmdd((time_t)0L);
	    Strcpy(t1->name, "@");
	    Strcpy(t1->death, "<eod>\n");
	    writeentry(rfile, t1);
	    (void) fflush(rfile);
# endif	/* TRUNCATE_FILE */
	}
#endif	/* UPDATE_RECORD_IN_PLACE */
	(void) fclose(rfile);
	unlock_file(RECORD);
	free_ttlist(tt_head);

  showwin:
	if (flags.toptenwin && !done_stopprint) display_nhwindow(toptenwin, 1);
  destroywin:
	if (!t0_used) dealloc_ttentry(t0);
	if (flags.toptenwin) {
	    destroy_nhwindow(toptenwin);
	    toptenwin=WIN_ERR;
	}
}

static void
outheader()
{
	char linebuf[BUFSZ];
	register char *bp;

	Strcpy(linebuf, " No  Points     Name");
	bp = eos(linebuf);
	while(bp < linebuf + COLNO - 9) *bp++ = ' ';
	Strcpy(bp, "Hp [max]");
	topten_print(linebuf);
}

/* so>0: standout line; so=0: ordinary line */
static void
outentry(rank, t1, so)
struct toptenentry *t1;
int rank;
boolean so;
{
	boolean second_line = TRUE;
	char linebuf[BUFSZ];
	char *bp, hpbuf[24], linebuf3[BUFSZ];
	int hppos, lngr;


	linebuf[0] = '\0';
	if (rank) Sprintf(eos(linebuf), "%3d", rank);
	else Strcat(linebuf, "   ");

	Sprintf(eos(linebuf), " %10ld  %.10s", t1->points, t1->name);
	Sprintf(eos(linebuf), "-%c ", t1->plchar);
	if (!strncmp("escaped", t1->death, 7)) {
	    Sprintf(eos(linebuf), "escaped the dungeon %s[max level %d]",
		    !strncmp(" (", t1->death + 7, 2) ? t1->death + 7 + 2 : "",
		    t1->maxlvl);
	    /* fixup for closing paren in "escaped... with...Amulet)[max..." */
	    if ((bp = index(linebuf, ')')) != 0)
		*bp = (t1->deathdnum == astral_level.dnum) ? '\0' : ' ';
	    second_line = FALSE;
	} else if (!strncmp("ascended", t1->death, 8)) {
	    Sprintf(eos(linebuf), "ascended to demigod%s-hood",
		    (t1->sex == 'F') ? "dess" : "");
	    second_line = FALSE;
	} else {
	    if (!strncmp(t1->death, "quit", 4)) {
		Strcat(linebuf, "quit");
		second_line = FALSE;
	    } else if (!strncmp(t1->death, "starv", 5)) {
		Strcat(linebuf, "starved to death");
		second_line = FALSE;
	    } else if (!strncmp(t1->death, "choked", 6)) {
		Sprintf(eos(linebuf), "choked on h%s food",
			(t1->sex == 'F') ? "er" : "is");
	    } else if (!strncmp(t1->death, "poisoned", 8)) {
		Strcat(linebuf, "was poisoned");
	    } else if (!strncmp(t1->death, "crushed", 7)) {
		Strcat(linebuf, "was crushed to death");
	    } else if (!strncmp(t1->death, "petrified by ", 13)) {
		Strcat(linebuf, "turned to stone");
	    } else Strcat(linebuf, "died");

	    if (t1->deathdnum == astral_level.dnum) {
		const char *arg, *fmt = " on the Plane of %s";

		switch (t1->deathlev) {
		case -5:
			fmt = " on the %s Plane";
			arg = "Astral";	break;
		case -4:
			arg = "Water";	break;
		case -3:
			arg = "Fire";	break;
		case -2:
			arg = "Air";	break;
		case -1:
			arg = "Earth";	break;
		default:
			arg = "Void";	break;
		}
		Sprintf(eos(linebuf), fmt, arg);
	    } else {
		Sprintf(eos(linebuf), " in %s on level %d",
			dungeons[t1->deathdnum].dname, t1->deathlev);
		if (t1->deathlev != t1->maxlvl)
		    Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
	    }

	    /* kludge for "quit while already on Charon's boat" */
	    if (!strncmp(t1->death, "quit ", 5))
		Strcat(linebuf, t1->death + 4);
	}
	Strcat(linebuf, ".");

	/* Quit, starved, ascended, and escaped contain no second line */
	if (second_line)
	    Sprintf(eos(linebuf), "  %c%s.", highc(*(t1->death)), t1->death+1);

	lngr = (int)strlen(linebuf);
	if (t1->hp <= 0) hpbuf[0] = '-', hpbuf[1] = '\0';
	else Sprintf(hpbuf, "%d", t1->hp);
	/* beginning of hp column after padding (not actually padded yet) */
	hppos = COLNO - (sizeof("  Hp [max]")-1); /* sizeof(str) includes \0 */
	while (lngr >= hppos) {
	    for(bp = eos(linebuf);
		    !(*bp == ' ' && (bp-linebuf < hppos));
		    bp--)
		;
	    /* special case: if about to wrap in the middle of maximum
	       dungeon depth reached, wrap in front of it instead */
	    if (bp > linebuf + 5 && !strncmp(bp - 5, " [max", 5)) bp -= 5;
	    Strcpy(linebuf3, bp+1);
	    *bp = 0;
	    if (so) {
		while (bp < linebuf + (COLNO-1)) *bp++ = ' ';
		*bp = 0;
		topten_print_bold(linebuf);
	    } else
		topten_print(linebuf);
	    Sprintf(linebuf, "%15s %s", "", linebuf3);
	    lngr = strlen(linebuf);
	}
	/* beginning of hp column not including padding */
	hppos = COLNO - 7 - (int)strlen(hpbuf);
	bp = eos(linebuf);

	if (bp <= linebuf + hppos) {
	    /* pad any necessary blanks to the hit point entry */
	    while (bp < linebuf + hppos) *bp++ = ' ';
	    Strcpy(bp, hpbuf);
	    Sprintf(eos(bp), " %s[%d]",
		    (t1->maxhp < 10) ? "  " : (t1->maxhp < 100) ? " " : "",
		    t1->maxhp);
	}

	if (so) {
	    bp = eos(linebuf);
	    if (so >= COLNO) so = COLNO-1;
	    while (bp < linebuf + so) *bp++ = ' ';
	    *bp = 0;
	    topten_print_bold(linebuf);
	} else
	    topten_print(linebuf);
}

static int
score_wanted(current_ver, rank, t1, playerct, players, uid)
boolean current_ver;
int rank;
struct toptenentry *t1;
int playerct;
const char **players;
int uid;
{
	int i;

	if (current_ver && (t1->ver_major != VERSION_MAJOR ||
			    t1->ver_minor != VERSION_MINOR ||
			    t1->patchlevel != PATCHLEVEL))
		return 0;

#ifdef PERS_IS_UID
	if (!playerct && t1->uid == uid)
		return 1;
#endif

	for (i = 0; i < playerct; i++) {
		if (strcmp(players[i], "all") == 0 ||
		    strncmp(t1->name, players[i], NAMSZ) == 0 ||
		    (players[i][0] == '-' &&
		     players[i][1] == t1->plchar &&
		     players[i][2] == 0) ||
		    (digit(players[i][0]) && rank <= atoi(players[i])))
		return 1;
	}
	return 0;
}

/*
 * print selected parts of score list.
 * argc >= 2, with argv[0] untrustworthy (directory names, et al.),
 * and argv[1] starting with "-s".
 */
void
prscore(argc,argv)
int argc;
char **argv;
{
	const char **players;
	int playerct, rank;
	boolean current_ver = TRUE, init_done = FALSE;
	register struct toptenentry *t1;
	FILE *rfile;
	boolean match_found = FALSE;
	register int i;
	char pbuf[BUFSZ];
	int uid = -1;
#ifndef PERS_IS_UID
	const char *player0;
#endif

	if (argc < 2 || strncmp(argv[1], "-s", 2)) {
		raw_printf("prscore: bad arguments (%d)", argc);
		return;
	}

	rfile = fopen_datafile(RECORD, "r");
	if (!rfile) {
		raw_print("Cannot open record file!");
		return;
	}

#ifdef	AMIGA
	{
	    extern winid amii_rawprwin;
	    init_nhwindows(&argc, argv);
	    amii_rawprwin = create_nhwindow(NHW_TEXT);
	}
#endif

	/* If the score list isn't after a game, we never went through
	 * initialization. */
	if (wiz1_level.dlevel == 0) {
		dlb_init();
		init_dungeons();
		init_done = TRUE;
	}

	if (!argv[1][2]){	/* plain "-s" */
		argc--;
		argv++;
	} else if (!argv[1][3] && index(pl_classes, argv[1][2])) {
		/* may get this case instead of next accidentally,
		 * but neither is listed in the documentation, so
		 * anything useful that happens is a bonus anyway */
		argv[1]++;
		argv[1][0] = '-';
	} else	argv[1] += 2;

	if (argc > 1 && !strcmp(argv[1], "-v")) {
		current_ver = FALSE;
		argc--;
		argv++;
	}

	if (argc <= 1) {
#ifdef PERS_IS_UID
		uid = getuid();
		playerct = 0;
		players = (const char **)0;
#else
		player0 = plname;
		if (!*player0)
# ifdef AMIGA
			player0 = "all";	/* single user system */
# else
			player0 = "hackplayer";
# endif
		playerct = 1;
		players = &player0;
#endif
	} else {
		playerct = --argc;
		players = (const char **)++argv;
	}
	raw_print("");

	t1 = tt_head = newttentry();
	for (rank = 1; ; rank++) {
	    readentry(rfile, t1);
	    if (t1->points == 0) break;
	    if (!match_found &&
		    score_wanted(current_ver, rank, t1, playerct, players, uid))
		match_found = TRUE;
	    t1->tt_next = newttentry();
	    t1 = t1->tt_next;
	}

	(void) fclose(rfile);
	if (init_done) {
	    free_dungeons();
	    dlb_cleanup();
	}

	if (match_found) {
	    outheader();
	    t1 = tt_head;
	    for (rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
		if (score_wanted(current_ver, rank, t1, playerct, players, uid))
		    (void) outentry(rank, t1, 0);
	    }
	} else {
	    Sprintf(pbuf, "Cannot find any %sentries for ",
				current_ver ? "current " : "");
	    if (playerct < 1) Strcat(pbuf, "you.");
	    else {
		if (playerct > 1) Strcat(pbuf, "any of ");
		for (i = 0; i < playerct; i++) {
		    Strcat(pbuf, players[i]);
		    if (i < playerct-1) Strcat(pbuf, ":");
		}
	    }
	    raw_print(pbuf);
	    raw_printf("Call is: %s -s [-v] [-role] [maxrank] [playernames]",
			 hname);
	}
	free_ttlist(tt_head);
#ifdef	AMIGA
	display_nhwindow(amii_rawprwin, 1);
	destroy_nhwindow(amii_rawprwin);
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
	struct toptenentry tt_buf;

	if (!otmp) return((struct obj *) 0);

	rfile = fopen_datafile(RECORD, "r");
	if (!rfile) {
		impossible("Cannot open record file!");
		return (struct obj *)0;
	}

	tt = &tt_buf;
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
		otmp = (struct obj *) 0;
	} else {
		/* reset timer in case corpse started out as lizard or troll */
		if (otmp->otyp == CORPSE) obj_stop_timers(otmp);
		otmp->corpsenm = classmon(tt->plchar, (tt->sex == 'F'));
		otmp->owt = weight(otmp);
		otmp = oname(otmp, tt->name);
		if (otmp->otyp == CORPSE) start_corpse_timeout(otmp);
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
	while ((p = index(p, ' ')) != 0) *p = '|';
}

static void
nsb_unmung_line(p)
	char *p;
{
	while ((p = index(p, '|')) != 0) *p = ' ';
}
#endif /* NO_SCAN_BRACK */

/*topten.c*/
