/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	<stdio.h>
#include "hack.h"

dowhatis(str)
register char *str;
{
	FILE *fp;
	char bufr[BUFSZ];
	register char *ep, q;

	pline("Specify what? ");
	getlin(bufr);
	str = bufr;
	while(*str == ' ') str++;
	q = *str;
	if(*(str+1)) pline("One character please.");
	else if(!(fp = fopen("data","r"))) pline("Cannot open data file!");
	else {
		while(fgets(bufr,BUFSZ,fp))
			if(*bufr == q) {
				ep = index(bufr, '\n');
				if(ep) *ep = 0;
				else impossible();
				pline(bufr);
				if(ep[-1] == ';') morewhat(fp);
				goto fnd;
			}
		pline("Unknown symbol.");
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
