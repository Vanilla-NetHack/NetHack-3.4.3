/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.whatis.c version 1.0.1 - whatis asks for one char only. */

#include	<stdio.h>
#include "hack.h"

dowhatis()
{
	FILE *fp;
	char bufr[BUFSZ];
	register char *ep, q;
	extern char readchar();

	if(!(fp = fopen("data","r")))
		pline("Cannot open data file!");
	else {
		pline("Specify what? ");
		q = readchar();
		while(fgets(bufr,BUFSZ,fp))
			if(*bufr == q) {
				ep = index(bufr, '\n');
				if(ep) *ep = 0;
				else impossible();
				pline(bufr);
				if(ep[-1] == ';') morewhat(fp);
				goto fnd;
			}
		pline("I've never heard of such things.");
	fnd:
		(void) fclose(fp);
	}
}

morewhat(fp) FILE *fp; {
char bufr[BUFSZ];
register char *ep;
	pline("More info? ");
	if(readchar() != 'y') return;
	cls();
	while(fgets(bufr,BUFSZ,fp) && *bufr == '\t'){
		ep = index(bufr, '\n');
		if(!ep) break;
		*ep = 0;
		puts(bufr+1);
	}
	more();
	docrt();
}
