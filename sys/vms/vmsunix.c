/*	SCCS Id: @(#)vmsunix.c	3.0	88/04/13
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

#include "hack.h"

#include <rms.h>
#include <jpidef.h>
#include <ssdef.h>
#include <errno.h>
#include <signal.h>
#undef off_t
#ifndef VAXC
#include <sys/stat.h>
#else
#include <stat.h>
#endif
#include <ctype.h>
#ifdef no_c$$translate
#include <errno.h>
#define C$$TRANSLATE(status) (errno = EVMSERR,  vaxc$errno = (status))
#else   /* must link with vaxcrtl object library (/lib or /incl=c$$translate) */
extern FDECL(C$$TRANSLATE, (unsigned long));
#endif
extern unsigned long SYS$PARSE(), SYS$SEARCH(), SYS$ENTER(), SYS$REMOVE();
extern unsigned long SYS$SETPRV();
extern unsigned long LIB$GETJPI(), LIB$SPAWN(), LIB$ATTACH();

int FDECL(link, (const char *, const char *));

void
setrandom()
{
	(void) Srand((long) time ((time_t *) 0));
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
gethdate(name) const char *name; {
	register char *np;

	if(stat(name, &hbuf))
		error("Cannot get status of %s.",
			(np = rindex(name, ']')) ? np+1 : name);
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

static int
veryold(fd)
int fd;
{
	register int i;
	time_t date;

	if(fstat(fd, &buf)) return(0);			/* cannot get status */
	if(buf.st_size != sizeof(int)) return(0);	/* not an xlock file */
	(void) time(&date);
	if(date - buf.st_mtime < 3L*24L*60L*60L) {	/* recent */
		int lockedpid;	/* should be the same size as hackpid */
		int status, dummy, code = JPI$_PID;

		if(read(fd, (char *)&lockedpid, sizeof(lockedpid)) !=
			sizeof(lockedpid))
			/* strange ... */
			return(0);
  		if(!(!((status = LIB$GETJPI(&code, &lockedpid, 0, &dummy)) & 1)
		     && status == SS$_NONEXPR))
			return(0);
	}
	(void) close(fd);
	for(i = 1; i <= MAXLEVEL+1; i++) {		/* try to remove all */
		glo(i);
		(void) delete(lock);
	}
	glo(0);
	if(delete(lock)) return(0);			/* cannot remove it */
	return(1);					/* success! */
}

void
getlock()
{
	register int i = 0, fd;

#ifdef HARD
	/* idea from rpick%ucqais@uccba.uc.edu
	 * prevent automated rerolling of characters
	 * test input (fd0) so that tee'ing output to get a screen dump still
	 * works
	 * also incidentally prevents development of any hack-o-matic programs
	 */
	if (isatty(0) <= 0)
		error("You must play from a terminal.");
#endif

	(void) fflush(stdout);

	/* we ignore QUIT and INT at this point */
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
regularize(s)	/* normalize file name */
register char *s;
{
	register char *lp;

	for (lp = s; *lp; lp++)         /* note: '-' becomes '_' */
	    if (!(isalpha(*lp) || isdigit(*lp) || *lp == '$'))
			*lp = '_';
}

int link(file, new)
const char *file, *new;
{
    unsigned long status;
    struct FAB fab;
    struct NAM nam;
    unsigned short fid[3];
    char esa[NAM$C_MAXRSS];

    fab = cc$rms_fab;
    fab.fab$l_fop = FAB$M_OFP;
    fab.fab$l_fna = file;
    fab.fab$b_fns = strlen(file);
    fab.fab$l_nam = &nam;

    nam = cc$rms_nam;
    nam.nam$l_esa = esa;
    nam.nam$b_ess = NAM$C_MAXRSS;

    if (!((status = SYS$PARSE(&fab)) & 1)
	|| !((status = SYS$SEARCH(&fab)) & 1))
    {
	C$$TRANSLATE(status);
	return -1;
    }

    fid[0] = nam.nam$w_fid[0];
    fid[1] = nam.nam$w_fid[1];
    fid[2] = nam.nam$w_fid[2];

    fab.fab$l_fna = new;
    fab.fab$b_fns = strlen(new);

    if (!((status = SYS$PARSE(&fab)) & 1))
    {
	C$$TRANSLATE(status);
	return -1;
    }

    nam.nam$w_fid[0] = fid[0];
    nam.nam$w_fid[1] = fid[1];
    nam.nam$w_fid[2] = fid[2];

    nam.nam$l_esa = nam.nam$l_name;
    nam.nam$b_esl = nam.nam$b_name + nam.nam$b_type + nam.nam$b_ver;

    if (!((status = SYS$ENTER(&fab)) & 1))
    {
	C$$TRANSLATE(status);
	return -1;
    }

    return 0;
}

#undef unlink
int unlink(file)
const char *file;
{
    int status;
    struct FAB fab = cc$rms_fab;
    struct NAM nam = cc$rms_nam;
    char esa[NAM$C_MAXRSS];

    fab.fab$l_fop = FAB$M_DLT;
    fab.fab$l_fna = (char *) file;
    fab.fab$b_fns = strlen(file);
    fab.fab$l_nam = &nam;
    nam.nam$l_esa = esa;
    nam.nam$b_ess = NAM$C_MAXRSS;

    if (!((status = SYS$PARSE(&fab)) & 1)
	|| !((status = SYS$REMOVE(&fab)) & 1))
    {
	C$$TRANSLATE(status);
	return -1;
    }

    return 0;
}

#undef creat
int vms_creat(file, mode)
char *file;
unsigned int mode;
{
    if (index(file, ';'))
	(void) delete(file);
    return creat(file, mode);
}

#undef getuid
int
vms_getuid()
{
    return (getgid() << 16) | getuid();
}

/*------*/
#ifndef LNM$_STRING
#include <lnmdef.h>	/* logical name definitions */
#endif
#define ENVSIZ LNM$C_NAMLENGTH  /*255*/

#define ENV_USR 0	/* user-mode */
#define ENV_SUP 1	/* supervisor-mode */
#define ENV_JOB 2	/* job-wide entry */

/* vms_define() - assign a value to a logical name */
int
vms_define(name, value, flag)
const char *name;
const char *value;
int flag;
{
    struct dsc { int len; const char *adr; };	/* string descriptor */
    struct itm3 { short buflen, itmcode; const char *bufadr; short *retlen; };
    static struct itm3 itm_lst[] = { {0,LNM$_STRING,0,0}, {0,0} };
    struct dsc nam_dsc, val_dsc, tbl_dsc;
    unsigned long result, SYS$CRELNM(), LIB$SET_LOGICAL();

    /* set up string descriptors */
    nam_dsc.len = strlen( nam_dsc.adr = name );
    val_dsc.len = strlen( val_dsc.adr = value );
    tbl_dsc.len = strlen( tbl_dsc.adr = "LNM$PROCESS" );

    switch (flag) {
	case ENV_JOB:	/* job logical name */
		tbl_dsc.len = strlen( tbl_dsc.adr = "LNM$JOB" );
	    /*FALLTHRU*/
	case ENV_SUP:	/* supervisor-mode process logical name */
		result = LIB$SET_LOGICAL(&nam_dsc, &val_dsc, &tbl_dsc);
	    break;
	case ENV_USR:	/* user-mode process logical name */
		itm_lst[0].buflen = val_dsc.len;
		itm_lst[0].bufadr = val_dsc.adr;
		result = SYS$CRELNM(0, &tbl_dsc, &nam_dsc, 0, itm_lst);
	    break;
	default:	/*[ bad input ]*/
		result = 0;
	    break;
    }
    result &= 1;	/* odd => success (== 1), even => failure (== 0) */
    return !result;	/* 0 == success, 1 == failure */
}

/* vms_putenv() - create or modify an environment value */
int
vms_putenv(string)
const char *string;
{
    char name[ENVSIZ+1], value[ENVSIZ+1], *p;   /* [255+1] */

    p = strchr(string, '=');
    if (p > string && p < string + sizeof name && strlen(p+1) < sizeof value) {
	(void)strncpy(name, string, p - string),  name[p - string] = '\0';
	(void)strcpy(value, p+1);
	return vms_define(name, value, ENV_USR);
    } else
	return 1;	/* failure */
}

/*
   Figure out whether the termcap code will find a termcap file; if not,
   try to help it out.  This avoids modifying the GNU termcap sources and
   can simplify configuration for sites which don't already use termcap.
 */
#define GNU_DEFAULT_TERMCAP "emacs_library:[etc]termcap.dat"
#define NETHACK_DEF_TERMCAP "hackdir:termcap"

const char *
verify_termcap()	/* called from startup(src/termcap.c) */
{
    struct stat dummy;
    char *tc = getenv("TERMCAP");
    if (tc) return (const char *)0;	/* no fixups needed */
    if (!tc && !stat(GNU_DEFAULT_TERMCAP, &dummy)) tc = GNU_DEFAULT_TERMCAP;
    if (!tc && !stat(NETHACK_DEF_TERMCAP, &dummy)) tc = NETHACK_DEF_TERMCAP;
    if (!tc && !stat("termcap", &dummy))  tc = "termcap";  /* current dir */
    if (!tc && !stat("$TERMCAP", &dummy)) tc = "$TERMCAP"; /* alt environ */
    if (tc) {
	/* putenv(strcat(strcpy(buffer,"TERMCAP="),tc)); */
	vms_define("TERMCAP", tc, ENV_USR);
    } else {
	/* Perhaps later we'll construct a termcap entry string and return that
	   when no file is found; for now, just return NULL unconditionally. */
    }
    return (const char *)0;
}
/*------*/

#if defined(CHDIR) || defined(SHELL)
static unsigned long oprv[2];

void
privoff()
{
    unsigned long prv[2] = { -1, -1 }, code = JPI$_PROCPRIV;

    (void) SYS$SETPRV(0, prv, 0, oprv);
    (void) LIB$GETJPI(&code, 0, 0, prv);
    (void) SYS$SETPRV(1, prv, 0, 0);
}

void
privon()
{
    (void) SYS$SETPRV(1, oprv, 0, 0);
}
#endif  /*CHDIR || SHELL*/

#ifdef SHELL
unsigned long dosh_pid = 0;

int
dosh()
{
	int status;

	settty((char *) NULL);	/* also calls end_screen() */
	(void) signal(SIGQUIT,SIG_IGN);
	(void) signal(SIGINT,SIG_DFL);
	if (!dosh_pid || !((status = LIB$ATTACH(&dosh_pid)) & 1))
	{
#ifdef CHDIR
		(void) chdir(getenv("PATH"));
#endif
		privoff();
		dosh_pid = 0;
		status = LIB$SPAWN(0, 0, 0, 0, 0, &dosh_pid);
		privon();
#ifdef CHDIR
		chdirx((char *) 0, 0);
#endif
	}
	gettty();
	setftty();
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef WIZARD
	if(wizard) (void) signal(SIGQUIT,SIG_DFL);
#endif
	docrt();
	if (!(status & 1))
	    pline("Spawn failed.  (%%x%08X) ", status);
	return 0;
}
#endif
