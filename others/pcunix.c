/*	SCCS Id: @(#)pcunix.c	1.4	87/08/08
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

#include <stdio.h>	/* mainly for NULL */
#include "hack.h"	/* mainly for index() which depends on BSD */

#ifndef __TURBOC__  /* Turbo C has time_t in time.h */
#include	<sys/types.h>		/* for time_t */
#endif
#include	<time.h>
#include        <sys/stat.h>

extern time_t time();
static struct stat buf, hbuf;

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

gethdate(name) char *name; {
/* old version - for people short of space */
/*
/* register char *np;
/*      if(stat(name, &hbuf))
/*              error("Cannot get status of %s.",
/*                      (np = rindex(name, '/')) ? np+1 : name);
/*
/* version using PATH from: seismo!gregc@ucsf-cgl.ARPA (Greg Couch) */

/*
 * The problem with   #include  <sys/param.h> is that this include file
 * does not exist on all systems, and moreover, that it sometimes includes
 * <sys/types.h> again, so that the compiler sees these typedefs twice.
 */
#define         MAXPATHLEN      1024

register char *np, *path;
char filename[MAXPATHLEN+1];

    if (index(name, '/') != NULL || (path = getenv("PATH")) == NULL)
        path = "";

    for (;;) {
        if ((np = index(path, ':')) == NULL)
            np = path + strlen(path);       /* point to end str */
        if (np - path <= 1)                     /* %% */
            (void) strcpy(filename, name);
        else {
            (void) strncpy(filename, path, np - path);
            filename[np - path] = '/';
            (void) strcpy(filename + (np - path) + 1, name);
        }
        if (stat(filename, &hbuf) == 0)
            return;
        if (*np == '\0')
        path = "";
        path = np + 1;
    }
    error("Cannot get status of %s.", (np = rindex(name, '/')) ? np+1 : name);
}

uptodate(fd) {
    if(fstat(fd, &buf)) {
        pline("Cannot get status of saved level? ");
        return(0);
    }
    if(buf.st_mtime < hbuf.st_mtime) {
        pline("Saved level is out of date. ");
        return(0);
    }
    return(1);
}

regularize(s)	/* normalize file name - we don't like ..'s or /'s */
register char *s;
{
	register char *lp;

	while((lp = index(s, '.')) || (lp = index(s, '/')))
		*lp = '_';
}
