/*	SCCS Id: @(#)unixunix.c	3.0	88/04/13
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

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#define MONFLAG_H
#include "hack.h"	/* mainly for index() which depends on BSD */

#include <errno.h>
#include <sys/stat.h>
#ifdef NO_FILE_LINKS
#include <fcntl.h>
#endif

void
setrandom()
{
#ifdef SYSV
	(void) Srand((long) time ((time_t *) 0));
#else
#ifdef ULTRIX
	Srand((int)time((time_t *)0));
#else
	(void) Srand((int) time ((long *) 0));
#endif /* ULTRIX */
#endif /* SYSV */

}

static struct tm *
getlt()
{
	time_t date;

#ifdef BSD
	(void) time((long *)(&date));
#else
	(void) time(&date);
#endif
#if defined(ULTRIX) || defined(BSD)
	return(localtime((long *)(&date)));
#else
	return(localtime(&date));
#endif /* ULTRIX */
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
phase_of_the_moon()			/* 0-7, with 0: new, 4: full */
{					/* moon period: 29.5306 days */
					/* year: 365.2422 days */
	register struct tm *lt = getlt();
	register int epact, diy, goldn;

	diy = lt->tm_yday;
	goldn = (lt->tm_year % 19) + 1;
	epact = (11 * goldn + 18) % 30;
	if ((epact == 25 && goldn > 11) || epact == 24)
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

static struct stat buf, hbuf;

void
gethdate(name) char *name; {
/* old version - for people short of space */
/*
/* register char *np;
/*	if(stat(name, &hbuf))
/*		error("Cannot get status of %s.",
/*			(np = rindex(name, '/')) ? np+1 : name);
/*
/* version using PATH from: seismo!gregc@ucsf-cgl.ARPA (Greg Couch) */


/*
 * The problem with   #include	<sys/param.h>   is that this include file
 * does not exist on all systems, and moreover, that it sometimes includes
 * <sys/types.h> again, so that the compiler sees these typedefs twice.
 */
#define		MAXPATHLEN	1024

register char *np, *path;
char filename[MAXPATHLEN+1];
	if (index(name, '/') != NULL || (path = getenv("PATH")) == NULL)
		path = "";

	for (;;) {
		if ((np = index(path, ':')) == NULL)
			np = path + strlen(path);	/* point to end str */
		if (np - path <= 1)			/* %% */
			Strcpy(filename, name);
		else {
			(void) strncpy(filename, path, np - path);
			filename[np - path] = '/';
			Strcpy(filename + (np - path) + 1, name);
		}
		if (stat(filename, &hbuf) == 0)
			return;
		if (*np == '\0')
			break;
		path = np + 1;
	}
	error("Cannot get status of %s.",
		(np = rindex(name, '/')) ? np+1 : name);
}

int
uptodate(fd)
int fd;
{
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

/* see whether we should throw away this xlock file */
static int
veryold(fd)
int fd;
{
	register int i;
	time_t date;

	if(fstat(fd, &buf)) return(0);			/* cannot get status */
	if(buf.st_size != sizeof(int)) return(0);	/* not an xlock file */
#ifdef BSD
	(void) time((long *)(&date));
#else
	(void) time(&date);
#endif
	if(date - buf.st_mtime < 3L*24L*60L*60L) {	/* recent */
		extern int errno;
		int lockedpid;	/* should be the same size as hackpid */

		if(read(fd, (char *)&lockedpid, sizeof(lockedpid)) !=
			sizeof(lockedpid))
			/* strange ... */
			return(0);

		/* From: Rick Adams <seismo!rick>
		/* This will work on 4.1cbsd, 4.2bsd and system 3? & 5.
		/* It will do nothing on V7 or 4.1bsd. */
#ifndef NETWORK
		/* It will do a VERY BAD THING if the playground is shared
		   by more than one machine! -pem */
  		if(!(kill(lockedpid, 0) == -1 && errno == ESRCH))
#endif
			return(0);
	}
	(void) close(fd);
	for(i = 1; i <= MAXLEVEL+1; i++) {		/* try to remove all */
		glo(i);
		(void) unlink(lock);
	}
	glo(0);
	if(unlink(lock)) return(0);			/* cannot remove it */
	return(1);					/* success! */
}

void
getlock()
{
	extern int errno;
	register int i = 0, fd;
#ifdef NO_FILE_LINKS
	int hlockfd ;
	int sleepct = 20 ;
#endif

#ifdef HARD
	/* idea from rpick%ucqais@uccba.uc.edu
	 * prevent automated rerolling of characters
	 * test input (fd0) so that tee'ing output to get a screen dump still
	 * works
	 * also incidentally prevents development of any hack-o-matic programs
	 */
	if (!isatty(0))
		error("You must play from a terminal.");
#endif

	(void) fflush(stdout);

	/* we ignore QUIT and INT at this point */
#ifdef NO_FILE_LINKS
	while ((hlockfd = open(LLOCK,O_RDONLY|O_CREAT|O_EXCL,0644)) == -1) {
	    if (--sleepct) {
		Printf( "Lock file in use.  %d retries left.\n",sleepct);
		(void) fflush(stdout);
# if defined(SYSV) || defined(ULTRIX)
		(void)
# endif
		    sleep(1);
	    } else {
		Printf("I give up!  Try again later.\n");
		getret();
		error("");
	    }
	}
	(void) close(hlockfd);

#else	/* NO_FILE_LINKS */
	if (link(HLOCK, LLOCK) == -1) {
		register int errnosv = errno;

		perror(HLOCK);
		Printf("Cannot link %s to %s\n", LLOCK, HLOCK);
		switch(errnosv) {
		case ENOENT:
		    Printf("Perhaps there is no (empty) file %s ?\n", HLOCK);
		    break;
		case EACCES:
		    Printf("It seems you don't have write permission here.\n");
		    break;
		case EEXIST:
		    Printf("(Try again or rm %s.)\n", LLOCK);
		    break;
		default:
		    Printf("I don't know what is wrong.");
		}
		getret();
		error("");
		/*NOTREACHED*/
	}
#endif /* NO_FILE_LINKS */

	regularize(lock);
	glo(0);
	if(locknum > 25) locknum = 25;

	do {
		if(locknum) lock[0] = 'a' + i++;

		if((fd = open(lock, 0)) == -1) {
			if(errno == ENOENT) goto gotlock;    /* no such file */
			perror(lock);
			(void) unlink(LLOCK);
			error("Cannot open %s", lock);
		}

		if(veryold(fd))	/* if true, this closes fd and unlinks lock */
			goto gotlock;
		(void) close(fd);
	} while(i < locknum);

	(void) unlink(LLOCK);
	error(locknum ? "Too many hacks running now."
		      : "There is a game in progress under your name.");
gotlock:
	fd = creat(lock, FCMASK);
	if(unlink(LLOCK) == -1)
		error("Cannot unlink %s.", LLOCK);
	if(fd == -1) {
		error("cannot creat lock file.");
	} else {
		if(write(fd, (char *) &hackpid, sizeof(hackpid))
		    != sizeof(hackpid)){
			error("cannot write lock");
		}
		if(close(fd) == -1) {
			error("cannot close lock");
		}
	}
}	

void
regularize(s)	/* normalize file name - we don't like .'s, /'s, spaces */
register char *s;
{
	register char *lp;

	while((lp=index(s, '.')) || (lp=index(s, '/')) || (lp=index(s,' ')))
		*lp = '_';
#ifdef SYSV
	/* avoid problems with 14 character file name limit */
# ifdef COMPRESS
	if(strlen(s) > 10)
		/* leave room for .e from error and .Z from compress
		 * appended to save files */
		s[10] = '\0';
# else
	if(strlen(s) > 11)
		/* leave room for .nn appended to level files */
		s[11] = '\0';
# endif
#endif
}
