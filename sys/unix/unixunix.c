/*	SCCS Id: @(#)unixunix.c	3.3	94/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file collects some Unix dependencies */

#include "hack.h"	/* mainly for index() which depends on BSD */

#include <errno.h>
#include <sys/stat.h>
#if defined(NO_FILE_LINKS) || defined(SUNOS4) || defined(POSIX_TYPES)
#include <fcntl.h>
#endif
#include <signal.h>

#ifdef _M_UNIX
extern void NDECL(sco_mapon);
extern void NDECL(sco_mapoff);
#endif
#ifdef __linux__
extern void NDECL(linux_mapon);
extern void NDECL(linux_mapoff);
#endif

static struct stat buf, hbuf;

void
gethdate(name)
	const char *name;
{
/* old version - for people short of space
 *
 *	register char *np;
 *	if(stat(name, &hbuf))
 *		error("Cannot get status of %s.",
 *			(np = rindex(name, '/')) ? np+1 : name);
 */
/* version using PATH from: seismo!gregc@ucsf-cgl.ARPA (Greg Couch) */

/*
 * The problem with   #include	<sys/param.h>   is that this include file
 * does not exist on all systems, and moreover, that it sometimes includes
 * <sys/types.h> again, so that the compiler sees these typedefs twice.
 */
#define		MAXPATHLEN	1024

	register const char *np, *path;
	char filename[MAXPATHLEN+1];
	int pathlen;

	if (index(name, '/') != (char *)0 ||
					(path = getenv("PATH")) == (char *)0)
		path = "";

	for (;;) {
		if ((np = index(path, ':')) == (char *)0)
			np = path + strlen(path);	/* point to end str */
		pathlen = np - path;
		if (pathlen > MAXPATHLEN)
			pathlen = MAXPATHLEN;
		if (pathlen <= 1) {			/* %% */
			(void) strncpy(filename, name, MAXPATHLEN);
		} else {
			(void) strncpy(filename, path, pathlen);
			filename[pathlen] = '/';
			(void) strncpy(filename + pathlen + 1, name,
					(MAXPATHLEN - 1) - pathlen);
		}
		filename[MAXPATHLEN] = '\0';
		if (stat(filename, &hbuf) == 0)
			return;
		if (*np == '\0')
			break;
		path = np + 1;
	}
	if (strlen(name) > BUFSZ/2)
		name = name + strlen(name) - BUFSZ/2;
#if defined(BOS) && defined(NHSTDC)
/*
 *	This one is really **STUPID**.  I don't know why it's happening
 *	as similar constructs work elsewhere, but...
 */
	if((np = rindex(name, '/')))
	     error("Cannot get status of %s.", np+1);
	else error("Cannot get status of %s.", name);
#else
	error("Cannot get status of %s.",
		(np = rindex(name, '/')) ? np+1 : name);
#endif
}

#if 0
int
uptodate(fd)
int fd;
{
	if(fstat(fd, &buf)) {
		pline("Cannot get status of saved level? ");
		wait_synch();
		return(0);
	}
	if(buf.st_mtime < hbuf.st_mtime) {
		pline("Saved level is out of date. ");
		wait_synch();
		return(0);
	}
	return(1);
}
#endif

/* see whether we should throw away this xlock file */
static int
veryold(fd)
int fd;
{
	time_t date;

	if(fstat(fd, &buf)) return(0);			/* cannot get status */
#ifndef INSURANCE
	if(buf.st_size != sizeof(int)) return(0);	/* not an xlock file */
#endif
#ifdef BSD
	(void) time((long *)(&date));
#else
	(void) time(&date);
#endif
	if(date - buf.st_mtime < 3L*24L*60L*60L) {	/* recent */
#ifndef NETWORK
		extern int errno;
#endif
		int lockedpid;	/* should be the same size as hackpid */

		if(read(fd, (genericptr_t)&lockedpid, sizeof(lockedpid)) !=
			sizeof(lockedpid))
			/* strange ... */
			return(0);

		/* From: Rick Adams <seismo!rick> */
		/* This will work on 4.1cbsd, 4.2bsd and system 3? & 5. */
		/* It will do nothing on V7 or 4.1bsd. */
#ifndef NETWORK
		/* It will do a VERY BAD THING if the playground is shared
		   by more than one machine! -pem */
		if(!(kill(lockedpid, 0) == -1 && errno == ESRCH))
#endif
			return(0);
	}
	(void) close(fd);
	return(1);
}

static int
eraseoldlocks()
{
	register int i;

	/* cannot use maxledgerno() here, because we need to find a lock name
	 * before starting everything (including the dungeon initialization
	 * that sets astral_level, needed for maxledgerno()) up
	 */
	for(i = 1; i <= MAXDUNGEON*MAXLEVEL + 1; i++) {
		/* try to remove all */
		set_levelfile_name(lock, i);
		(void) unlink(fqname(lock, LEVELPREFIX, 0));
	}
	set_levelfile_name(lock, 0);
	if (unlink(fqname(lock, LEVELPREFIX, 0)))
		return(0);				/* cannot remove it */
	return(1);					/* success! */
}

void
getlock()
{
	extern int errno;
	register int i = 0, fd, c;
	const char *fq_lock;

#ifdef TTY_GRAPHICS
	/* idea from rpick%ucqais@uccba.uc.edu
	 * prevent automated rerolling of characters
	 * test input (fd0) so that tee'ing output to get a screen dump still
	 * works
	 * also incidentally prevents development of any hack-o-matic programs
	 */
	/* added check for window-system type -dlc */
	if (!strcmp(windowprocs.name, "tty"))
	    if (!isatty(0))
		error("You must play from a terminal.");
#endif

	/* we ignore QUIT and INT at this point */
	if (!lock_file(HLOCK, LOCKPREFIX, 10)) {
		wait_synch();
		error("%s", "");
	}

	regularize(lock);
	set_levelfile_name(lock, 0);

	if(locknum) {
		if(locknum > 25) locknum = 25;

		do {
			lock[0] = 'a' + i++;
			fq_lock = fqname(lock, LEVELPREFIX, 0);

			if((fd = open(fq_lock, 0)) == -1) {
			    if(errno == ENOENT) goto gotlock; /* no such file */
			    perror(fq_lock);
			    unlock_file(HLOCK);
			    error("Cannot open %s", fq_lock);
			}

			if(veryold(fd) /* closes fd if true */
							&& eraseoldlocks())
				goto gotlock;
			(void) close(fd);
		} while(i < locknum);

		unlock_file(HLOCK);
		error("Too many hacks running now.");
	} else {
		fq_lock = fqname(lock, LEVELPREFIX, 0);
		if((fd = open(fq_lock, 0)) == -1) {
			if(errno == ENOENT) goto gotlock;    /* no such file */
			perror(fq_lock);
			unlock_file(HLOCK);
			error("Cannot open %s", fq_lock);
		}

		if(veryold(fd) /* closes fd if true */ && eraseoldlocks())
			goto gotlock;
		(void) close(fd);

		if(iflags.window_inited) {
		    c = yn("There is already a game in progress under your name.  Destroy old game?");
		} else {
		    (void) printf("\nThere is already a game in progress under your name.");
		    (void) printf("  Destroy old game? [yn] ");
		    (void) fflush(stdout);
		    c = getchar();
		    (void) putchar(c);
		    (void) fflush(stdout);
		    while (getchar() != '\n') ; /* eat rest of line and newline */
		}
		if(c == 'y' || c == 'Y')
			if(eraseoldlocks())
				goto gotlock;
			else {
				unlock_file(HLOCK);
				error("Couldn't destroy old game.");
			}
		else {
			unlock_file(HLOCK);
			error("%s", "");
		}
	}

gotlock:
	fd = creat(fq_lock, FCMASK);
	unlock_file(HLOCK);
	if(fd == -1) {
		error("cannot creat lock file (%s).", fq_lock);
	} else {
		if(write(fd, (genericptr_t) &hackpid, sizeof(hackpid))
		    != sizeof(hackpid)){
			error("cannot write lock (%s)", fq_lock);
		}
		if(close(fd) == -1) {
			error("cannot close lock (%s)", fq_lock);
		}
	}
}

void
regularize(s)	/* normalize file name - we don't like .'s, /'s, spaces */
register char *s;
{
	register char *lp;
#if defined(SYSV) && !defined(AIX_31) && defined(COMPRESS)
	int i;
#endif

	while((lp=index(s, '.')) || (lp=index(s, '/')) || (lp=index(s,' ')))
		*lp = '_';
#if defined(SYSV) && !defined(AIX_31)
	/* avoid problems with 14 character file name limit */
# ifdef COMPRESS
	/* leave room for .e from error and .Z from compress appended to
	 * save files */
#  ifdef COMPRESS_EXTENSION
	i = 12 - strlen(COMPRESS_EXTENSION);
#  else
	i = 10;		/* should never happen... */
#  endif
	if(strlen(s) > i)
		s[i] = '\0';
# else
	if(strlen(s) > 11)
		/* leave room for .nn appended to level files */
		s[11] = '\0';
# endif
#endif
}

#ifdef SHELL
int
dosh()
{
	register char *str;
	if(child(0)) {
		if((str = getenv("SHELL")) != (char*)0)
			(void) execl(str, str, (char *)0);
		else
			(void) execl("/bin/sh", "sh", (char *)0);
		raw_print("sh: cannot execute.");
		exit(EXIT_FAILURE);
	}
	return 0;
}
#endif /* SHELL */

#if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
int
child(wt)
int wt;
{
	register int f;
	suspend_nhwindows((char *)0);	/* also calls end_screen() */
#ifdef _M_UNIX
	sco_mapon();
#endif
#ifdef __linux__
	linux_mapon();
#endif
	if((f = fork()) == 0){		/* child */
		(void) setgid(getgid());
		(void) setuid(getuid());
#ifdef CHDIR
		(void) chdir(getenv("HOME"));
#endif
		return(1);
	}
	if(f == -1) {	/* cannot fork */
		pline("Fork failed.  Try again.");
		return(0);
	}
	/* fork succeeded; wait for child to exit */
	(void) signal(SIGINT,SIG_IGN);
	(void) signal(SIGQUIT,SIG_IGN);
	(void) wait( (int *) 0);
#ifdef _M_UNIX
	sco_mapoff();
#endif
#ifdef __linux__
	linux_mapoff();
#endif
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef WIZARD
	if(wizard) (void) signal(SIGQUIT,SIG_DFL);
#endif
	if(wt) {
		raw_print("");
		wait_synch();
	}
	resume_nhwindows();
	return(0);
}
#endif
