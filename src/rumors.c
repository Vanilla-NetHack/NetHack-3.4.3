/*	SCCS Id: @(#)rumors.c	3.0	89/02/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* hack.rumors.c - version 1.0.3 */

#include	"hack.h"		/* for RUMORFILE and BSD (index) */

/* Rumors has been entirely rewritten to speed up the access.  This is
 * essential when working from floppies.  Using fseek() the way that's done
 * here means rumors following longer rumors are output more often than those
 * following shorter rumors.  Also, you may see the same rumor more than once
 * in a particular game (although the odds are highly against it), but
 * this also happens with real fortune cookies.  Besides, a person can
 * just read the rumor file if they desire.  -dgk
 */

/* The rumors file consists of a long giving the number of bytes of useful/true
 * rumors, followed by the true rumors (one per line), followed by the useless/
 * false/misleading/cute rumors (one per line).
 */

/* The oracle file consists of a number of multiple-line records, separated
 * (but not terminated) by "-----" lines.
 */

long first_rumor = sizeof(long);
long true_rumor_size, false_rumor_size, end_rumor_file;
#ifdef ORACLE
long oracle_size;
#endif

static void
init_rumors()
{
	register FILE *fp;

#ifdef OS2_CODEVIEW
	{
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,RUMORFILE);
	if(fp = fopen(tmp, "r")) {
#else
	if(fp = fopen(RUMORFILE, "r")) {
#endif
	    (void) fread((genericptr_t)&true_rumor_size,sizeof(long),1,fp);
	    (void) fseek(fp, 0L, 2);
	    end_rumor_file = ftell(fp);
	    false_rumor_size = (end_rumor_file-sizeof(long)) - true_rumor_size;
	    (void) fclose(fp);
	} else {
		pline("Can't open rumors file!");
		end_rumor_file = -1;	/* don't try to open it again */
	}
#ifdef OS2_CODEVIEW
	}
#endif
#ifdef ORACLE
#ifdef OS2_CODEVIEW
	{
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,ORACLEFILE);
	if(fp = fopen(tmp, "r")) {
#else
	if(fp = fopen(ORACLEFILE, "r")) {
#endif
	    (void) fseek(fp, 0L, 2);
	    oracle_size = ftell(fp);
	    (void) fclose(fp);
	} else {
		pline("Can't open oracles file!");
		oracle_size = -1;	/* don't try to open it again */
	}
#ifdef OS2_CODEVIEW
	}
#endif
#endif
}


void
outrumor(truth,cookie)
int truth; /* 1=true, -1=false, 0=either */
boolean cookie;
{
	static const char fortune_msg[] =
		"This cookie has a scrap of paper inside.";
	char	line[COLNO];
	char	*endp;
	FILE	*rumors;
	long tidbit, beginning;

	if (cookie && Blind) {
		pline(fortune_msg);
		pline("What a pity that you cannot read it!");
		return;
	}
	if (end_rumor_file < 0) /* We couldn't open RUMORFILE */
		return;
#ifdef OS2_CODEVIEW
	{
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,RUMORFILE);
	if(rumors = fopen(tmp, "r")) {
#else
# ifdef MACOS
	if(rumors = fopen(RUMORFILE, "r"))
		rumors = openFile(RUMORFILE);
	if (rumors) {
# else
	if(rumors = fopen(RUMORFILE, "r")) {
# endif
#endif
		if (!end_rumor_file) {	/* if this is the first outrumor() */
			init_rumors();
		}
		if (!truth) truth = (rn2(100) >= 50 ? 1 : -1);
		/* otherwise, 50% chance of being true */
		switch(truth) {
		    case 1: beginning = first_rumor;
			tidbit = Rand() % true_rumor_size;
			break;
		    case -1: beginning = first_rumor + true_rumor_size;
			tidbit = true_rumor_size + Rand() % false_rumor_size;
			break;
		}
		(void) fseek(rumors, first_rumor + tidbit, 0);
		(void) fgets(line, COLNO, rumors);
		if (!fgets(line, COLNO, rumors) || (truth == 1 &&
		    (ftell(rumors) > true_rumor_size + sizeof(long)))) {
			/* reached end of rumors -- go back to beginning */
			(void) fseek(rumors, beginning, 0);
			(void) fgets(line, COLNO, rumors);
		}
		if (endp = index(line, '\n')) *endp = 0;
		if (cookie) {
			pline(fortune_msg);
			pline("It reads:");
		} else pline("Tidbit of information #%ld: ",tidbit);
		pline(line);
		(void) fclose(rumors);
	} else {
		pline("Can't open rumors file!");
		end_rumor_file = -1;	/* don't try to open it again */
	}
#ifdef OS2_CODEVIEW
	}
#endif
}

#ifdef ORACLE
static void
outoracle()
{
	char	line[COLNO];
	char	*endp;
	FILE	*oracles;

	if (oracle_size < 0)	/* We couldn't open ORACLEFILE */
		return;
#ifdef OS2_CODEVIEW
	{
    char tmp[PATHLEN];

    Strcpy(tmp,hackdir);
    append_slash(tmp);
    Strcat(tmp,ORACLEFILE);
	if(oracles = fopen(tmp, "r")) {
#else
# ifdef MACOS
	if(oracles = fopen(ORACLEFILE, "r"))
		oracles = openFile(ORACLEFILE);
	if (oracles) {
# else
	if(oracles = fopen(ORACLEFILE, "r")) {
# endif
#endif
		if (!oracle_size) {	/* if this is the first outrumor() */
			init_rumors();
		}
		(void) fseek(oracles, Rand() % oracle_size, 0);
		(void) fgets(line, COLNO, oracles);
		while (1)
		    if (!fgets(line, COLNO, oracles)) {
			/* reached end of oracle info -- go back to beginning */
			(void) fseek(oracles, 0L, 0);
			break;
		    } else if (!strncmp(line,"-----",5)) {
			/* found end of an oracle proclamation */
			break;
		    }
		pline("The Oracle meditates for a moment and then intones: ");
		cornline(0,NULL);
		while (fgets(line, COLNO, oracles) && strncmp(line,"-----",5)) {
			if (endp = index(line, '\n')) *endp = 0;
			cornline(1,line);
		}
		cornline(2,"");
		(void) fclose(oracles);
	} else {
		pline("Can't open oracles file!");
		oracle_size = -1;	/* don't try to open it again */
	}
#ifdef OS2_CODEVIEW
	}
#endif
}

int
doconsult(oracl)
register struct monst *oracl;
{
	register char ans;

	multi = 0;
	(void) inshop();

	if(!oracl) {
		pline("There is no one here to consult.");
		return(0);
	}
	if(!oracl->mpeaceful) {
		pline("The Oracle is in no mood for consultations.");
		return(0);
	} else {
		if(!u.ugold) {
			You("have no money.");
			return(0);
		}
		pline("\"Wilt thou settle for a minor consultation?\"  (50 zorkmids) ");
		ans = ynq();
		if(ans == 'y') {
			if(u.ugold < 50) {
			    You("don't even have enough money for that!");
			    return(0);
			}
			u.ugold -= 50;
			oracl->mgold += 50;
			flags.botl = 1;
			outrumor(1, FALSE);
			return(1);
		} else if(ans == 'q') return(0);
		else {
			pline("\"Then dost thou desire a major one?\"  (1000 zorkmids) ");
			if (yn() != 'y') return(0);
		}
		if(u.ugold < 1000) {
		pline("The Oracle scornfully takes all your money and says:");
cornline(0,NULL);
cornline(1,"\"...it is rather disconcerting to be confronted with the");
cornline(1,"following theorem from [Baker, Gill, and Solovay, 1975].");
cornline(1,"");
cornline(1,"Theorem 7.18  There exist recursive languages A and B such that");
cornline(1,"  (1)  P(A) == NP(A), and");
cornline(1,"  (2)  P(B) != NP(B)");
cornline(1,"");
cornline(1,"This provides impressive evidence that the techniques that are");
cornline(1,"currently available will not suffice for proving that P != NP or");
cornline(1,"that P == NP.\"  [Garey and Johnson, p. 185.]");
cornline(2,"");
		    oracl->mgold += u.ugold;
		    u.ugold = 0;
		    flags.botl = 1;
		    return(1);
		}
		u.ugold -= 1000;
		oracl->mgold += 1000;
		flags.botl = 1;
		outoracle();
		return(1);
	}
}

#endif
