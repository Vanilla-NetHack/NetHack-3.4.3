/*    SCCS Id: @(#)amiunix.c    3.0    89/05/02
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file collects some Unix dependencies; pager.c contains some more */

/*
 * The time is used for:
 *    - seed for rand()
 *    - year on tombstone and yymmdd in record file
 *    - phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *    - night and midnight (the undead are dangerous at midnight)
 *    - determination of what files are "very old"
 */

/* block some unused #defines to avoid overloading some cpp's */
#define MONDATA_H
#include "hack.h"   /* mainly for index() which depends on BSD */

#define     NOSTAT

#ifndef NOSTAT
#include    <stat.h>
static struct stat buf, hbuf;
#endif

extern time_t time();

static struct tm *NDECL(getlt);

void
setrandom()
{
	(void) Srand((unsigned int) time ((time_t *) 0));
}

static struct tm *
getlt()
{
	time_t date;
	struct tm *localtime();

	(void) time((long *)(&date));
	return(localtime(&date));
}

int
getyear()
{
	return(1900 + getlt()->tm_year);
}

char *
getdate()
{
#ifdef LINT	/* static char datestr[7]; */
	char datestr[7];
#else
	static char datestr[7];
#endif
	register struct tm *lt = getlt();

	Sprintf(datestr, "%2d%2d%2d",
		lt->tm_year, lt->tm_mon + 1, lt->tm_mday);
	if(datestr[2] == ' ') datestr[2] = '0';
	if(datestr[4] == ' ') datestr[4] = '0';
	return(datestr);
}

int
phase_of_the_moon()                     /* 0-7, with 0: new, 4: full */
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

int
night()
{
	register int hour = getlt()->tm_hour;

	return(hour < 6 || hour > 21);
}

int
midnight()
{
	return(getlt()->tm_hour == 0);
}

void
gethdate(name)
char *name;
{
#ifndef NOSTAT
/* old version - for people short of space */
/*
/* register char *np;
/*	if(stat(name, &hbuf))
/*		error("Cannot get status of %s.",
/*			(np = rindex(name, '/')) ? np+1 : name);
/*
/* version using PATH from: seismo!gregc@ucsf-cgl.ARPA (Greg Couch) */

/*
 * The problem with   #include	<sys/param.h> is that this include file
 * does not exist on all systems, and moreover, that it sometimes includes
 * <sys/types.h> again, so that the compiler sees these typedefs twice.
 */
#define 	MAXPATHLEN	80

extern char PATH[];	/* In amigaDOS.c */

register char *np, *path;
char filename[MAXPATHLEN+1];

    if (index(name, '/') != NULL || (path = PATH) == NULL)
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
#endif
}

int
uptodate(fd)
{
    return(1);
}

void
regularize(s)    /* normalize file name - we don't like :'s or /'s */
register char *s;
{
    register char *lp;

    while((lp = index(s, ':')) || (lp = index(s, '/')))
	*lp = '_';
}
