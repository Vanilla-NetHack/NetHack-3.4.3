/*	SCCS Id: @(#)pcunix.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* unix.c - version 1.0.3 */

/* This file collects some Unix dependencies; pager.c contains some more */

/*
 * The time is used for:
 *	- seed for rand()
 *	- year on tombstone and yymmdd in record file
 *	- phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *	- night and midnight (the undead are dangerous at midnight)
 *	- determination of what files are "very old"
 */

#include "hack.h"	/* mainly for index() which depends on BSD */

#include	<sys/types.h>		/* for time_t */
#include	<time.h>

extern time_t time();

setrandom()
{
	(void) srand((int) time ((time_t *) 0));
}

struct tm *
getlt()
{
	time_t date;
	struct tm *localtime();

	(void) time(&date);
	return(localtime(&date));
}

getyear()
{
	return(1900 + getlt()->tm_year);
}

char *
getdate()
{
	static char datestr[7];
	register struct tm *lt = getlt();

	(void) sprintf(datestr, "%2d%2d%2d",
		lt->tm_year, lt->tm_mon + 1, lt->tm_mday);
	if(datestr[2] == ' ') datestr[2] = '0';
	if(datestr[4] == ' ') datestr[4] = '0';
	return(datestr);
}

phase_of_the_moon()			/* 0-7, with 0: new, 4: full */
{					/* moon period: 29.5306 days */
					/* year: 365.2422 days */
	register struct tm *lt = getlt();
	register int epact, diy, golden;

	diy = lt->tm_yday;
	golden = (lt->tm_year % 19) + 1;
	epact = (11 * golden + 18) % 30;
	if ((epact == 25 && golden > 11) || epact == 24)
		epact++;

	return( (((((diy + epact) * 6) + 11) % 177) / 22) & 7 );
}

night()
{
	register int hour = getlt()->tm_hour;

	return(hour < 6 || hour > 21);
}

midnight()
{
	return(getlt()->tm_hour == 0);
}

regularize(s)	/* normalize file name - we don't like ..'s or /'s */
register char *s;
{
	register char *lp;

	while((lp = index(s, '.')) || (lp = index(s, '/')))
		*lp = '_';
}
