/*	SCCS Id: @(#)rip.c	3.0	88/04/27
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <ctype.h>

static void FDECL(center,(int,char *));

static const char *rip_txt[] = {
"                       ----------",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |       1001       |",
"                 *|     *  *  *      | *",
"        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______\n",
0
};

char **rip;

static void
center(line, text)
int line;
char *text;
{
	register char *ip,*op;
	ip = text;
	op = &rip[line][28 - ((strlen(text)+1)>>1)];
	while(*ip) *op++ = *ip++;
}

void
outrip(){
	register char **dp;
	register char *dpx;
	char buf[BUFSZ];
	register int x, y;
	int killed_by_line = 0;

	rip = dp = (char **) alloc(sizeof(rip_txt));
	if (!dp) return;
	for (x = 0; rip_txt[x]; x++) {
		dp[x] = (char *) alloc((unsigned int)(strlen(rip_txt[x]) + 1));
		if (!dp[x]) return;
		Strcpy(dp[x], rip_txt[x]);
	}
	dp[x] = (char *)0;

	cls();
	Sprintf(buf, "%s", plname);
	buf[16] = 0;
	center(6, buf);
	Sprintf(buf, "%ld Au", u.ugold);
	center(7, buf);
	if (killer_format != NO_KILLER_PREFIX) {
		killed_by_line = 1;
		Strcpy(buf, "killed by");
		if (killer_format == KILLED_BY_AN)
			Strcat(buf, index(vowels, *killer) ? " an" : " a");
		center(8, buf);
	}
	Strcpy(buf, killer);
	if(strlen(buf) > 16) {
	    register int i,i0,i1;
		i0 = i1 = 0;
		for(i = 0; i <= 16; i++)
			if(buf[i] == ' ') i0 = i, i1 = i+1;
		if(!i0) i0 = i1 = 16;
		buf[i1 + 16] = 0;
		center(9 + killed_by_line, buf+i1);
		buf[i0] = 0;
	}
	center(8 + killed_by_line, buf);
	Sprintf(buf, "%4d", getyear());
	center(11, buf);
	for(y=8; *dp; y++,dp++){
		x = 0;
		dpx = *dp;
		while(dpx[x]) {
			while(dpx[x] == ' ') x++;
			curs(x,y);
			while(dpx[x] && dpx[x] != ' '){
				if(done_stopprint)
					return;
				curx++;
				(void) putchar(dpx[x++]);
			}
		}
	}
	getret();
}

