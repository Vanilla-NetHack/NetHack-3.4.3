/*	SCCS Id: @(#)track.c	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* track.c - version 1.0.2 */

#include "hack.h"

#define	UTSZ	50

coord utrack[UTSZ];
int utcnt = 0;
int utpnt = 0;

void
initrack(){
	utcnt = utpnt = 0;
}

/* add to track */
void
settrack(){
	if(utcnt < UTSZ) utcnt++;
	if(utpnt == UTSZ) utpnt = 0;
	utrack[utpnt].x = u.ux;
	utrack[utpnt].y = u.uy;
	utpnt++;
}

coord *
gettrack(x, y)
register int x, y;
{
	register int i, cnt, ndist;
	coord tc;
	cnt = utcnt;
	for(i = utpnt-1; cnt--; i--){
		if(i == -1) i = UTSZ-1;
		tc = utrack[i];
		ndist = dist2(x,y,tc.x,tc.y);
		if(ndist < 3)
			return(ndist ? (coord *)&(utrack[i]) : 0);
	}
	return (coord *)0;
}
