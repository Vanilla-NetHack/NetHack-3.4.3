/*	SCCS Id: @(#)pcunix.c	3.0	89/12/29
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

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

#ifndef MACOS
#include	<sys/types.h>
#include	<sys/stat.h>
#endif

static struct tm * NDECL(getlt);

#ifdef OVLB

#ifndef MACOS
static struct stat buf;
# ifdef WANT_GETHDATE
static struct stat hbuf;
# endif
#endif

void
setrandom()
{
	(void) Srand((int) time ((time_t *) 0));
}

static struct tm *
getlt()
{
	time_t date;

	(void) time(&date);
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
	static char datestr[7];
	register struct tm *lt = getlt();

	Sprintf(datestr, "%2d%2d%2d",
		lt->tm_year, lt->tm_mon + 1, lt->tm_mday);
	if(datestr[2] == ' ') datestr[2] = '0';
	if(datestr[4] == ' ') datestr[4] = '0';
	return(datestr);
}

int
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

#ifndef MACOS
void
gethdate(name)
char *name;
{
# ifdef WANT_GETHDATE
/* old version - for people short of space */
/*
/* register char *np;
/*      if(stat(name, &hbuf))
/*	      error("Cannot get status of %s.",
/*		      (np = rindex(name, '/')) ? np+1 : name);
/*
/* version using PATH from: seismo!gregc@ucsf-cgl.ARPA (Greg Couch) */

/*
 * The problem with   #include  <sys/param.h> is that this include file
 * does not exist on all systems, and moreover, that it sometimes includes
 * <sys/types.h> again, so that the compiler sees these typedefs twice.
 */
#define	 MAXPATHLEN      1024

    register char *np, *path;
    char filename[MAXPATHLEN+1], *getenv();

    if (index(name, '/') != NULL || (path = getenv("PATH")) == NULL)
	path = "";

    for (;;) {
	if ((np = index(path, ':')) == NULL)
	    np = path + strlen(path);       /* point to end str */
	if (np - path <= 1)		     /* %% */
	    Strcpy(filename, name);
	else {
	    (void) strncpy(filename, path, np - path);
	    filename[np - path] = '/';
	    Strcpy(filename + (np - path) + 1, name);
	}
	if (stat(filename, &hbuf) == 0)
	    return;
	if (*np == '\0')
	path = "";
	path = np + 1;
    }
    error("Cannot get status of %s.", (np = rindex(name, '/')) ? np+1 : name);
# endif /* WANT_GETHDATE */
}

int
uptodate(fd)
int fd;
{
# ifdef WANT_GETHDATE
    if(fstat(fd, &buf)) {
	pline("Cannot get status of saved level? ");
	return(0);
    }
    if(buf.st_mtime < hbuf.st_mtime) {
	pline("Saved level is out of date. ");
	return(0);
    }
# else
#  if defined(MSDOS) && !defined(NO_FSTAT)
    if(fstat(fd, &buf)) {
	if(moves > 1) pline("Cannot get status of saved level? ");
	else pline("Cannot get status of saved game");
	return(0);
    } 
    if(comp_times(buf.st_mtime)) { 
	if(moves > 1) pline("Saved level is out of date");
	else pline("Saved game is out of date. ");
	return(0);
    }
#  endif  /* MSDOS /* */
# endif /* WANT_GETHDATE */
    return(1);
}
#endif	/* MACOS /* */

void
regularize(s)
/*
 * normalize file name - we don't like .'s, /'s, :'s [Mac], or spaces,
 * and in msdos / OS/2 we really get picky
 */
register char *s;
{
	register char *lp;

#ifdef MSDOS
	for (lp = s; *lp; lp++)
		if (*lp <= ' ' || *lp == '"' || (*lp >= '*' && *lp <= ',') ||
		    *lp == '.' || *lp == '/' || (*lp >= ':' && *lp <= '?') ||
# ifdef OS2
		    *lp == '&' || *lp == '(' || *lp == ')' ||
# endif
		    *lp == '|' || *lp >= 127 || (*lp >= '[' && *lp <= ']'))
                        *lp = '_';
#else
	while((lp=index(s, '.')) || (lp=index(s, '/')) || (lp=index(s,' '))
# ifdef MACOS
	   || (lp=index(s, ':'))
# endif
		) *lp = '_';
#endif
}

#endif /* OVLB */
