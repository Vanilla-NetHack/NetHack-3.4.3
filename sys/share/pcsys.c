/*	SCCS Id: @(#)pcsys.c	3.2	93/05/24
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  System related functions for MSDOS, OS/2, TOS, and Windows NT
 */

#define NEED_VARARGS
#include "hack.h"
#include "wintty.h"

#include <ctype.h>
#include <fcntl.h>
#ifndef MSDOS			/* already done */
#include <process.h>
#endif
#ifdef __GO32__
#define P_WAIT		0
#define P_NOWAIT	1
#endif
#ifdef TOS
#include <osbind.h>
#endif

#ifdef MOVERLAY
extern void __far __cdecl _movepause( void );
extern void __far __cdecl _moveresume( void );
extern unsigned short __far __cdecl _movefpause;
extern unsigned short __far __cdecl _movefpaused;
#define     __MOVE_PAUSE_DISK	  2   /* Represents the executable file */
#define     __MOVE_PAUSE_CACHE	  4   /* Represents the cache memory */
#endif /* MOVERLAY */

static boolean NDECL(record_exists);
#ifndef TOS
static boolean NDECL(comspec_exists);
#endif

#ifdef WIN32CON
extern int ProgmanLaunched;    /* from nttty.c */
#endif

#ifdef MICRO

void
flushout()
{
	(void) fflush(stdout);
	return;
}

static const char *COMSPEC =
# ifdef TOS
"SHELL";
# else
"COMSPEC";
# endif

#define getcomspec() getenv(COMSPEC)

# ifdef SHELL
int
dosh()
{
	extern char orgdir[];
	char *comspec;
	int spawnstat;

	if (comspec = getcomspec()) {
#  ifndef TOS	/* TOS has a variety of shells */
		suspend_nhwindows("To return to NetHack, enter \"exit\" at the system prompt.\n");
#  else
		suspend_nhwindows((char *)0);
#  endif /* TOS */
		chdirx(orgdir, 0);
#  ifdef __GO32__
		if (system(comspec) < 0) {  /* wsu@eecs.umich.edu */
#  else
#   ifdef MOVERLAY
       /* Free the cache memory used by overlays, close .exe */
	_movefpause |= __MOVE_PAUSE_DISK;
	_movefpause |= __MOVE_PAUSE_CACHE;
	_movepause();
#   endif
		spawnstat = spawnl(P_WAIT, comspec, comspec, (char *)0);
#   ifdef MOVERLAY
		 _moveresume();
#   endif

		if ( spawnstat < 0) {
#  endif
			raw_printf("Can't spawn \"%s\"!", comspec);
			getreturn("to continue");
		}
#  ifdef TOS
/* Some shells (e.g. Gulam) turn the cursor off when they exit */
		if (flags.BIOS)
			(void)Cursconf(1, -1);
#  endif
		get_scr_size(); /* maybe the screen mode changed (TH) */
		resume_nhwindows();
		chdirx(hackdir, 0);
	} else
		pline("Can't find %s.",COMSPEC);
	return 0;
}
# endif /* SHELL */

# ifdef MFLOPPY

void
eraseall(path, files)
const char *path, *files;
{
	char buf[PATHLEN];
	char *foundfile;

	foundfile = foundfile_buffer();
	Sprintf(buf, "%s%s", path, files);
	if (findfirst(buf))
	    do {
	       Sprintf(buf, "%s%s", path, foundfile);
		(void) unlink(buf);
	    } while (findnext());
	return;
}

/*
 * Rewritten for version 3.3 to be faster
 */
void
copybones(mode)
int mode;
{
	char from[PATHLEN], to[PATHLEN], last[13];
	char *frompath, *topath;
	char *foundfile;
#  ifndef TOS
	int status;
	char copy[8], *comspec;
#  endif

	if (!ramdisk)
		return;

	/* Find the name of the last file to be transferred
	 */
	frompath = (mode != TOPERM) ? permbones : levels;
	foundfile = foundfile_buffer();
	last[0] = '\0';
	Sprintf(from, "%s%s", frompath, allbones);
	topath = (mode == TOPERM) ? permbones : levels;
#  ifdef TOS
	eraseall(topath, allbones);
#  endif
	if (findfirst(from))
		do {
#  ifdef TOS
			Sprintf(from, "%s%s", frompath, foundfile);
			Sprintf(to, "%s%s", topath, foundfile);
			if (_copyfile(from, to))
				goto error_copying;
#  endif
			Strcpy(last, foundfile);
		} while (findnext());
#  ifdef TOS
	else
		return;
#  else
	if (last[0]) {
		Sprintf(copy, "%cC copy",switchar());

		/* Remove any bones files in `to' directory.
		 */
		eraseall(topath, allbones);

		/* Copy `from' to `to' */
		Sprintf(to, "%s%s", topath, allbones);
		comspec = getcomspec();
		status =spawnl(P_WAIT, comspec, comspec, copy, from,
			to, "> nul", (char *)0);
	} else
		return;
#  endif /* TOS */

	/* See if the last file got there.  If so, remove the ramdisk bones
	 * files.
	 */
	Sprintf(to, "%s%s", topath, last);
	if (findfirst(to)) {
		if (mode == TOPERM)
			eraseall(frompath, allbones);
		return;
	}

#  ifdef TOS
error_copying:
#  endif
	/* Last file didn't get there.
	 */
	Sprintf(to, "%s%s", topath, allbones);
	msmsg("Can't copy \"%s\" to \"%s\" -- ", from, to);
#  ifndef TOS
	if (status < 0)
	    msmsg("can't spawn \"%s\"!", comspec);
	else
#  endif
	    msmsg((freediskspace(topath) < filesize(from)) ?
	    "insufficient disk space." : "bad path(s)?");
	if (mode == TOPERM) {
		msmsg("Bones will be left in \"%s\"\n",
			*levels ? levels : hackdir);
	} else {
		/* Remove all bones files on the RAMdisk */
		eraseall(levels, allbones);
		playwoRAMdisk();
	}
	return;
}

void
playwoRAMdisk()
{
	int c;

	msmsg("Do you wish to play without a RAMdisk? [yn] (n)");

	/* Set ramdisk false *before* exit-ing (because msexit calls
	 * copybones)
	 */
	ramdisk = FALSE;
	c = tgetch(); if (c == 'Y') c = 'y';
	if (c != 'y') {
		settty("Be seeing you...\n");
		exit(EXIT_SUCCESS);
	}
	set_lock_and_bones();
	return;
}

int
saveDiskPrompt(start)
int start;
{
	char buf[BUFSIZ], *bp;
	char qbuf[QBUFSZ];

	int fd;

	if (flags.asksavedisk) {
		/* Don't prompt if you can find the save file */
		if ((fd = open_savefile()) >= 0) {
			(void) close(fd);
			return 1;
		}
		clear_nhwindow(WIN_MESSAGE);
		pline("If save file is on a save disk, insert that disk now.");
		mark_synch();
		Sprintf(qbuf,"File name (default \"%s\"%s) ?", SAVEF,
			start ? "" : ", <Esc> cancels save");
		getlin(qbuf, buf);
		clear_nhwindow(WIN_MESSAGE);
		if (!start && *buf == '\033')
			return 0;

		/* Strip any whitespace. Also, if nothing was entered except
		 * whitespace, do not change the value of SAVEF.
		 */
		for (bp = buf; *bp; bp++)
			if (!isspace(*bp)) {
				strncpy(SAVEF, bp, PATHLEN);
				break;
			}
	}
	return 1;
}

# endif /* MFLOPPY */

/* Return 1 if the record file was found */
static boolean
record_exists()
{
	FILE *fp;

	fp = fopen_datafile(RECORD, "r");
	if (fp) {
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

# ifdef TOS
#define comspec_exists() 1
# else
/* Return 1 if the comspec was found */
static boolean
comspec_exists()
{
	int fd;
	char *comspec;

	if (comspec = getcomspec())
		if ((fd = open(comspec, O_RDONLY)) >= 0) {
			(void) close(fd);
			return TRUE;
		}
	return FALSE;
}
# endif

# ifdef MFLOPPY
/* Prompt for game disk, then check for record file.
 */
void
gameDiskPrompt()
{
	if (flags.asksavedisk) {
		if (record_exists() && comspec_exists())
			return;
		(void) putchar('\n');
		getreturn("when the game disk has been inserted");
	}
	if (comspec_exists() && record_exists())
		return;

	if (!comspec_exists())
		msmsg("\n\nWARNING: can't find command processor \"%s\"!\n", getcomspec());
	if (!record_exists())
		msmsg("\n\nWARNING: can't find record file \"%s\"!\n", RECORD);
	msmsg("If the game disk is not in, insert it now.\n");
	getreturn("to continue");
	return;
}
# endif /* MFLOPPY */
#endif /* MICRO */

/*
 * Add a backslash to any name not ending in /, \ or :	 There must
 * be room for the \
 */
void
append_slash(name)
char *name;
{
	char *ptr;

	if (!*name)
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '\\' && *ptr != '/' && *ptr != ':') {
		*++ptr = '\\';
		*++ptr = '\0';
	}
	return;
}

void
getreturn(str)
const char *str;
{
#ifdef TOS
	msmsg("Hit <Return> %s.", str);
#else
	msmsg("Hit <Enter> %s.", str);
#endif
	while (Getchar() != '\n') ;
	return;
}

#ifndef WIN32
void
msmsg VA_DECL(const char *, fmt)
	VA_START(fmt);
	VA_INIT(fmt, const char *);
# if defined(MSDOS)
	if (flags.grmode) {
		char buf[BUFSZ];

		Vsprintf(buf,fmt, VA_ARGS);
		xputs(buf);
	} else
# endif
	{
	  Vprintf(fmt, VA_ARGS);
	  flushout();
	}
	VA_END();
	return;
}
#else
void
msmsg VA_DECL(const char *, fmt)
	VA_START(fmt);
	VA_INIT(fmt, const char *);
	{
	    char buf[BUFSZ];

	    Vsprintf(buf,fmt, VA_ARGS);
	    xputs(buf);
	}
}
#endif


/*
 * Follow the PATH, trying to fopen the file.
 */
#ifdef TOS
# ifdef __MINT__
#define PATHSEP ':'
# else
#define PATHSEP ','
# endif
#else
#define PATHSEP ';'
#endif

FILE *
fopenp(name, mode)
const char *name, *mode;
{
	char buf[BUFSIZ], *bp, *pp, lastch = 0;
	FILE *fp;

	/* Try the default directory first.  Then look along PATH.
	 */
	Strcpy(buf, name);
	if (fp = fopen(buf, mode))
		return fp;
	else {
		pp = getenv("PATH");
		while (pp && *pp) {
			bp = buf;
			while (*pp && *pp != PATHSEP)
				lastch = *bp++ = *pp++;
			if (lastch != '\\' && lastch != '/')
				*bp++ = '\\';
			Strcpy(bp, name);
			if (fp = fopen(buf, mode))
				return fp;
			if (*pp)
				pp++;
		}
	}
#ifdef OS2_CODEVIEW /* one more try for hackdir */
	Strcpy(buf,hackdir);
	append_slash(buf);
	Strcat(buf,name);
	if(fp = fopen(buf,mode))
		return fp;
#endif
	return (FILE *)0;
}

/* Chdir back to original directory
 */
#ifdef TOS
extern boolean run_from_desktop;	/* set in pcmain.c */
#endif

#undef exit
#if !defined(MSDOS) && !defined(WIN32)
extern void FDECL(exit, (int));
#endif

void
msexit(code)
int code;
{
#ifdef CHDIR
	extern char orgdir[];
#endif

	flushout();
#ifndef TOS
# ifndef WIN32
	enable_ctrlP(); 	/* in case this wasn't done */
# endif
#endif
#ifdef MFLOPPY
	if (ramdisk) copybones(TOPERM);
#endif
#ifdef CHDIR
	chdir(orgdir);		/* chdir, not chdirx */
	chdrive(orgdir);
#endif
#ifdef TOS
	if (run_from_desktop)
	    getreturn("to continue"); /* so the user can read the score list */
# ifdef TEXTCOLOR
	if (colors_changed)
		restore_colors();
# endif
#endif
#ifdef WIN32CON
	/* Only if we started from Progman, not command prompt,
	 * we need to get one last return, so the score board does
	 * not vanish instantly after being created.
	 * ProgmanLaunched is defined and set in nttty.c.
	 */

	if (ProgmanLaunched) getreturn("to end");
#endif
	exit(code);
	return;
}
