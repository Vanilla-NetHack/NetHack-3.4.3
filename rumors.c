/*	SCCS Id: @(#)rumors.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.rumors.c - version 1.0.3 */

#include	<stdio.h>
#include	"hack.h"		/* for RUMORFILE and BSD (index) */
#ifdef DGK
/* Rumors has been entirely rewritten to speed up the access.  This is
 * essential when working from floppies.  Using fseek() the way that's done
 * here means rumors following longer rumors are output more often than those
 * following shorter rumors.  Also, you may see the same rumor more than once
 * in a particular game (although the odds are highly against it), but
 * this also happens with real fortune cookies.  Besides, a person can
 * just read the rumor file if they desire.  -dgk
 */
long rumors_size;
extern char *index();
extern long ftell();

outrumor()
{
	char	line[COLNO];
	char	*endp;
	char	roomer[FILENAME];
	FILE	*rumors;

	if (rumors_size < 0)	/* We couldn't open RUMORFILE */
		return;
	if(rumors = fopen(RUMORFILE, "r")) {
		if (!rumors_size) {	/* if this is the first outrumor() */
			fseek(rumors, 0L, 2);
			rumors_size = ftell(rumors);
		}
		fseek(rumors, rand() % rumors_size, 0);
		fgets(line, COLNO, rumors);
		if (!fgets(line, COLNO, rumors)) {	/* at EOF ? */
			fseek(rumors, 0L, 0);		/* seek back to start */
			fgets(line, COLNO, rumors);
		}
		if(endp = index(line, '\n')) *endp = 0;
		pline("This cookie has a scrap of paper inside! It reads: ");
		pline(line);
		fclose(rumors);
	} else {
		pline("Can't open rumors file!");
		rumors_size = -1;	/* don't try to open it again */
	}
}

#else

#define	CHARSZ	8			/* number of bits in a char */
extern long *alloc();
extern char *index();
int n_rumors = 0;
int n_used_rumors = -1;
char *usedbits;

init_rumors(rumf) register FILE *rumf; {
register int i;
	n_used_rumors = 0;
	while(skipline(rumf)) n_rumors++;
	rewind(rumf);
	i = n_rumors/CHARSZ;
	usedbits = (char *) alloc((unsigned)(i+1));
	for( ; i>=0; i--) usedbits[i] = 0;
}

skipline(rumf) register FILE *rumf; {
char line[COLNO];
	while(1) {
		if(!fgets(line, sizeof(line), rumf)) return(0);
		if(index(line, '\n')) return(1);
	}
}

outline(rumf) register FILE *rumf; {
char line[COLNO];
register char *ep;
	if(!fgets(line, sizeof(line), rumf)) return;
	if((ep = index(line, '\n')) != 0) *ep = 0;
	pline("This cookie has a scrap of paper inside! It reads: ");
	pline(line);
}

outrumor(){
register int rn,i;
register FILE *rumf;
	if(n_rumors <= n_used_rumors ||
	  (rumf = fopen(RUMORFILE, "r")) == (FILE *) 0) return;
	if(n_used_rumors < 0) init_rumors(rumf);
	if(!n_rumors) goto none;
	rn = rn2(n_rumors - n_used_rumors);
	i = 0;
	while(rn || used(i)) {
		(void) skipline(rumf);
		if(!used(i)) rn--;
		i++;
	}
	usedbits[i/CHARSZ] |= (1 << (i % CHARSZ));
	n_used_rumors++;
	outline(rumf);
none:
	(void) fclose(rumf);
}

used(i) register int i; {
	return(usedbits[i/CHARSZ] & (1 << (i % CHARSZ)));
}

#endif /* DGK /**/
