/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.end.c version 1.0.1 - added "escaped with amulet" */

#include "hack.h"
#include <stdio.h>
#include <signal.h>
#define	Sprintf	(void) sprintf
extern char plname[], pl_character[];
extern char *itoa(), *ordin(), *eos(), *getlogin();

xchar maxdlevel = 1;

done1()
{
	(void) signal(SIGINT,SIG_IGN);
	pline("Really quit?");
	if(readchar() != 'y') {
		(void) signal(SIGINT,done1);
		clrlin();
		(void) fflush(stdout);
		if(multi > 0) nomul(0);
		return(0);
	}
	done("quit");
	/* NOTREACHED */
}

int done_stopprint;

done_intr(){
	done_stopprint++;
	(void) signal(SIGINT,SIG_IGN);
}

done_in_by(mtmp) register struct monst *mtmp; {
static char buf[BUFSZ];
	pline("You die ...");
	if(mtmp->data->mlet == ' '){
		Sprintf(buf, "the ghost of %s", (char *) mtmp->mextra);
		killer = buf;
	} else if(mtmp->mnamelth) {
		Sprintf(buf, "%s called %s",
			mtmp->data->mname, NAME(mtmp));
		killer = buf;
	} else if(mtmp->minvis) {
		Sprintf(buf, "invisible %s", mtmp->data->mname);
		killer = buf;
	} else killer = mtmp->data->mname;
 done("died");
}

/* called with arg "died", "escaped", "quit", "choked", "panic"
   or "starved" */
/* Be careful not to call panic from here! */
done(st1)
register char *st1;
{

#ifdef WIZARD
	if(wizard && *st1 == 'd'){
		u.ustr = u.ustrmax += 2;
		u.uhp = u.uhpmax += 10;
		if(uarm) uarm->spe++;
		if(uwep) uwep->spe++; /* NB: uwep need not be a weapon! */
		u.uswldtim = 0;
		pline("For some reason you are still alive.");
		flags.move = 0;
		if(multi > 0) multi = 0; else multi = -1;
		flags.botl = 1;
		return;
	}
#endif WIZARD
	(void) signal(SIGINT, done_intr);
	if(*st1 == 'q' && u.uhp < 1){
		st1 = "died";
		killer = "quit while already on Charon's boat";
	}
	if(*st1 == 's') killer = "starvation";
	paybill();
	clearlocks();
	if(index("cds", *st1)){
		savebones();
		if(!flags.notombstone)
			outrip();
		else
			more();
	}
	settty((char *) 0);	/* does a cls() */
	if(!done_stopprint)
		printf("Goodbye %s %s...\n\n", pl_character, plname);
	{ long int tmp;
	tmp = u.ugold - u.ugold0;
	if(tmp < 0) tmp = 0;
	if(*st1 == 'd') tmp -= tmp/10;
	else killer = st1;
	u.urexp += tmp;
	}
	if(*st1 == 'e') {
		extern struct monst *mydogs;
		register struct monst *mtmp = mydogs;
		register struct obj *otmp;
		register int i;
		register unsigned worthlessct = 0;

		killer = st1;
		u.urexp += 50 * maxdlevel;
		if(mtmp) {
			if(!done_stopprint) printf("You");
			while(mtmp) {
				if(!done_stopprint)
					printf(" and %s", monnam(mtmp));
				u.urexp += mtmp->mhp;
				mtmp = mtmp->nmon;
			}
			if(!done_stopprint)
		    printf("\nescaped from the dungeon with %lu points,\n",
			u.urexp);
		} else
		if(!done_stopprint)
		  printf("You escaped from the dungeon with %lu points,\n",
		    u.urexp);
		for(otmp = invent; otmp; otmp = otmp->nobj) {
			if(otmp->olet == GEM_SYM){
				objects[otmp->otyp].oc_name_known = 1;
				i = otmp->quan*objects[otmp->otyp].g_val;
				if(i == 0) {
					worthlessct += otmp->quan;
					continue;
				}
				u.urexp += i;
				if(!done_stopprint)
				  printf("\t%s (worth %d Zorkmids),\n",
				    doname(otmp), i);
			} else if(otmp->olet == AMULET_SYM) {
				otmp->known = 1;
				i = (otmp->spe < 0) ? 2 : 5000;
				u.urexp += i;
				if(!done_stopprint)
				  printf("\t%s (worth %d Zorkmids),\n",
				    doname(otmp), i);
				if(otmp->spe >= 0) {
					u.urexp *= 2;
					killer = "escaped (with amulet)";
				}
			}
		}
		if(worthlessct) if(!done_stopprint)
		  printf("\t%d worthless piece%s of coloured glass,\n",
		  worthlessct, plur(worthlessct));
	} else
		if(!done_stopprint)
		  printf("You %s on dungeon level %d with %lu points,\n",
		    st1,dlevel,u.urexp);
	if(!done_stopprint)
	  printf("and %lu piece%s of gold, after %lu move%s.\n",
	    u.ugold, (u.ugold == 1) ? "" : "s",
	    moves, (moves == 1) ? "" : "s");
	if(!done_stopprint)
  printf("You were level %d with a maximum of %d hit points when you %s.\n",
	    u.ulevel, u.uhpmax, st1);
	if(*st1 == 'e'){
		getret();	/* all those pieces of coloured glass ... */
		cls();
	}
#ifdef WIZARD
	if(!wizard)
#endif WIZARD
		topten();
	if(done_stopprint) printf("\n\n");
	exit(0);
}

#define newttentry() (struct toptenentry *) alloc(sizeof(struct toptenentry))
#define	NAMSZ	8
#define	DTHSZ	40
#define	PERSMAX	1
#define	POINTSMIN	1	/* must be > 0 */
#define	ENTRYMAX	100	/* must be >= 10 */
struct toptenentry {
	struct toptenentry *tt_next;
	long int points;
	int level,maxlvl,hp,maxhp;
	char plchar;
	char str[NAMSZ+1];
	char death[DTHSZ+1];
} *tt_head;

topten(){
	int rank, rank0 = -1, rank1 = 0;
	int occ_cnt = PERSMAX;
	register struct toptenentry *t0, *t1, *tprev;
	char *recfile = "record";
	FILE *rfile;
	register flg = 0;

	if(!(rfile = fopen(recfile,"r"))){
		puts("Cannot open record file!");
		return;
	}
	(void) putchar('\n');

	/* create a new 'topten' entry */
	t0 = newttentry();
	t0->level = dlevel;
	t0->maxlvl = maxdlevel;
	t0->hp = u.uhp;
	t0->maxhp = u.uhpmax;
	t0->points = u.urexp;
	t0->plchar = pl_character[0];
	(void) strncpy(t0->str, plname, NAMSZ);
	(t0->str)[NAMSZ] = 0;
	(void) strncpy(t0->death, killer, DTHSZ);
	(t0->death)[DTHSZ] = 0;

	/* assure minimum number of points */
	if(t0->points < POINTSMIN)
		t0->points = 0;

	t1 = tt_head = newttentry();
	tprev = 0;
	/* rank0: -1 undefined, 0 not_on_list, n n_th on list */
	for(rank = 1; ; ) {
	  if(fscanf(rfile, "%d %d %d %d %ld %c %[^,],%[^\n]",
		&t1->level, &t1->maxlvl,
		&t1->hp, &t1->maxhp, &t1->points,
		&t1->plchar, t1->str, t1->death) != 8
	  || t1->points < POINTSMIN)
			t1->points = 0;
	  if(rank0 < 0 && t1->points < t0->points) {
		rank0 = rank++;
		if(tprev == 0)
			tt_head = t0;
		else
			tprev->tt_next = t0;
		t0->tt_next = t1;
		occ_cnt--;
		flg++;		/* ask for a rewrite */
	  } else
		tprev = t1;
	  if(t1->points == 0) break;
	  if(strncmp(t1->str, t0->str, NAMSZ) == 0 &&
	     t1->plchar == t0->plchar && --occ_cnt <= 0){
		if(rank0 < 0){
			rank0 = 0;
			rank1 = rank;
	printf("You didn't beat your previous score of %ld points.\n\n",
				t1->points);
		}
		if(occ_cnt < 0){
			flg++;
			continue;
		}
	  }
	  if(rank <= ENTRYMAX){
	  	t1 = t1->tt_next = newttentry();
	  	rank++;
	  }
	  if(rank > ENTRYMAX){
		t1->points = 0;
		break;
	  }
	}
	if(flg) {	/* rewrite record file */
		(void) fclose(rfile);
		if(!(rfile=fopen(recfile,"w"))){
			puts("Cannot write record file\n");
			return;
		}

		if(!done_stopprint) if(rank0 > 0){
		    if(rank0 <= 10)
			puts("You made the top ten list!\n");
		    else
		printf("You reached the %d%s place on the top %d list.\n\n",
			rank0, ordin(rank0), ENTRYMAX);
		}
	}
	if(rank0 == 0) rank0 = rank1;
	if(rank0 <= 0) rank0 = rank;
	if(!done_stopprint) outheader();
	t1 = tt_head;
	for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
	  if(flg) fprintf(rfile,"%d %d %d %d %ld %c %s,%s\n",
	    t1->level, t1->maxlvl,
	    t1->hp, t1->maxhp, t1->points,
	    t1->plchar,t1->str,t1->death);
	  if(done_stopprint) continue;
	  if(rank > flags.end_top &&
	    (rank < rank0-flags.end_around || rank > rank0+flags.end_around)
	    && (!flags.end_own || strncmp(t1->str, t0->str, NAMSZ)))
	  	continue;
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
	if(rank0 >= rank)
		(void) outentry(0, t0, 1);
	(void) fclose(rfile);
}

outheader() {
char linebuf[BUFSZ];
register char *bp;
	(void) strcpy(linebuf, "Number Points  Name");
	bp = eos(linebuf);
	while(bp < linebuf + COLNO - 9) *bp++ = ' ';
	(void) strcpy(bp, "Hp [max]");
	puts(linebuf);
}

/* so>0: standout line; so=0: ordinary line; so<0: no output, return lth */
int
outentry(rank,t1,so) register struct toptenentry *t1; {
boolean quit = FALSE, killed = FALSE, starv = FALSE;
char linebuf[BUFSZ];
	linebuf[0] = 0;
	if(rank) Sprintf(eos(linebuf), "%3d", rank);
		else Sprintf(eos(linebuf), "   ");
	Sprintf(eos(linebuf), " %6ld %8s", t1->points, t1->str);
	if(t1->plchar == 'X') Sprintf(eos(linebuf), " ");
	else Sprintf(eos(linebuf), "-%c ", t1->plchar);
	if(!strncmp("escaped", t1->death, 7)) {
	  if(!strcmp(" (with amulet)", t1->death+7))
	    Sprintf(eos(linebuf), "escaped the dungeon with amulet");
	  else
	    Sprintf(eos(linebuf), "escaped the dungeon [max level %d]",
	      t1->maxlvl);
	} else {
	  if(!strncmp(t1->death,"quit",4))
	    Sprintf(eos(linebuf), "quit"), quit = TRUE;
	  else if(!strcmp(t1->death,"choked"))
	    Sprintf(eos(linebuf), "choked in his food");
	  else if(!strncmp(t1->death,"starv",5))
	    Sprintf(eos(linebuf), "starved to death"), starv = TRUE;
	  else Sprintf(eos(linebuf), "was killed"), killed = TRUE;
	  Sprintf(eos(linebuf), " on%s level %d",
	    (killed || starv) ? "" : " dungeon", t1->level);
	  if(t1->maxlvl != t1->level)
	    Sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
	  if(quit && t1->death[4]) Sprintf(eos(linebuf), t1->death + 4);
	}
	if(killed) Sprintf(eos(linebuf), " by %s%s",
	  !strncmp(t1->death, "the ", 4) ? "" :
	  index(vowels,*t1->death) ? "an " : "a ",
	  t1->death);
	Sprintf(eos(linebuf), ".");
	if(t1->maxhp) {
	  register char *bp = eos(linebuf);
	  char hpbuf[10];
	  int hppos;
	  Sprintf(hpbuf, (t1->hp > 0) ? itoa(t1->hp) : "-");
	  hppos = COLNO - 7 - strlen(hpbuf);
	  if(bp <= linebuf + hppos) {
	    while(bp < linebuf + hppos) *bp++ = ' ';
	    (void) strcpy(bp, hpbuf);
	    Sprintf(eos(bp), " [%d]", t1->maxhp);
	  }
	}
	if(so == 0) puts(linebuf);
	else if(so > 0) {
	  register char *bp = eos(linebuf);
	  if(so >= COLNO) so = COLNO-1;
	  while(bp < linebuf + so) *bp++ = ' ';
	  *bp = 0;
	  standoutbeg();
	  fputs(linebuf,stdout);
	  standoutend();
	  (void) putchar('\n');
	}
 return(strlen(linebuf));
}

char *
itoa(a) int a; {
static char buf[12];
	Sprintf(buf,"%d",a);
	return(buf);
}

char *
ordin(n) int n; {
register int d = n%10;
	return((d==0 || d>3 || n/10==1) ? "th" : (d==1) ? "st" :
		(d==2) ? "nd" : "rd");
}

clearlocks(){
register x;
	(void) signal(SIGHUP,SIG_IGN);
	for(x = 1; x <= maxdlevel; x++) {
		glo(x);
		(void) unlink(lock);	/* not all levels need be present */
	}
	(*index(lock,'.')) = 0;
	(void) unlink(lock);
}

#ifdef NOSAVEONHANGUP
hangup(){
	(void) signal(SIGINT,SIG_IGN);
	clearlocks();
	exit(1);
}
#endif NOSAVEONHANGUP

char *
eos(s) register char *s; {
	while(*s) s++;
	return(s);
}

/* it is the callers responsibility to check that there is room for c */
charcat(s,c) register char *s, c; {
	while(*s) s++;
	*s++ = c;
	*s = 0;
}

prscore(argc,argv) int argc; char **argv; {
	extern char *hname;
	char *player0;
	char **players;
	int playerct;
	int rank;
	register struct toptenentry *t1;
	char *recfile = "record";
	FILE *rfile;
	register flg = 0;
	register int i;

	if(!(rfile = fopen(recfile,"r"))){
		puts("Cannot open record file!");
		return;
	}

	if(argc > 1 && !strncmp(argv[1], "-s", 2)){
		if(!argv[1][2]){
			argc--;
			argv++;
		} else if(!argv[1][3] && index("CFKSTWX", argv[1][2])) {
			argv[1]++;
			argv[1][0] = '-';
		} else	argv[1] += 2;
	}
	if(argc <= 1){
		player0 = getlogin();
		if(!player0) player0 = "player";
		playerct = 1;
		players = &player0;
	} else {
		playerct = --argc;
		players = ++argv;
	}
	putchar('\n');

	t1 = tt_head = newttentry();
	for(rank = 1; ; rank++) {
	  if(fscanf(rfile, "%d %d %d %d %ld %c %[^,],%[^\n]",
		&t1->level, &t1->maxlvl,
		&t1->hp, &t1->maxhp, &t1->points,
		&t1->plchar, t1->str, t1->death) != 8)
			t1->points = 0;
	  if(t1->points == 0) break;
	  for(i = 0; i < playerct; i++){
		if(strcmp(players[i], "all") == 0 ||
		   strncmp(t1->str, players[i], NAMSZ) == 0 ||
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
		printf("Cannot find any entries for ");
		if(playerct > 1) printf("any of ");
		for(i=0; i<playerct; i++)
			printf("%s%s", players[i], (i<playerct-1)?", ":".\n");
		printf("Call is: %s -s [playernames]\n", hname);
		return;
	}

	outheader();
	t1 = tt_head;
	for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
		for(i = 0; i < playerct; i++){
			if(strcmp(players[i], "all") == 0 ||
			   strncmp(t1->str, players[i], NAMSZ) == 0 ||
			  (players[i][0] == '-' &&
			   players[i][1] == t1->plchar &&
			   players[i][2] == 0) ||
			  (digit(players[i][0]) && rank <= atoi(players[i])))
				goto out;
		}
		continue;
	out:
	  (void) outentry(rank, t1, 0);
	}
}
