/*	SCCS Id: @(#)msdos.c	2.3	87/12/16
/* An assortment of MSDOS functions.
 */

#include <stdio.h>
#include "hack.h"

#ifdef MSDOS
# include <dos.h>

void
flushout()
{
	(void) fflush(stdout);
}

getuid() {
	return 1;
}

char *
getlogin() {
	return ((char *) NULL);
}
# ifdef REDO
tgetch() {
	char ch, popch();
	static char DOSgetch(), BIOSgetch();

	if (!(ch = popch())) {
#  ifdef DGK
		/* BIOSgetch can use the numeric key pad on IBM compatibles. */
		if (flags.IBMBIOS)
			ch = BIOSgetch();
		else
#  endif
			ch = DOSgetch();
	}
	return ((ch == '\r') ? '\n' : ch);
}
# else /* REDO /**/
tgetch() {
	char ch;
	static char DOSgetch(), BIOSgetch();

#  ifdef DGK
	/* BIOSgetch can use the numeric key pad on IBM compatibles. */
	if (flags.IBMBIOS)
		ch = BIOSgetch();
	else
#  endif
		ch = DOSgetch();
	return ((ch == '\r') ? '\n' : ch);
}
# endif /* REDO /**/

# define DIRECT_INPUT	0x7
static char
DOSgetch() {
	union REGS regs;

	regs.h.ah = DIRECT_INPUT;
	intdos(&regs, &regs);
	if (!regs.h.al) {	/* an extended code -- not yet supported */
		regs.h.ah = DIRECT_INPUT;
		intdos(&regs, &regs);	/* eat the next character */
		regs.h.al = 0;		/* and return a 0 */
	}
	return (regs.h.al);
}


# ifdef DGK
#  include <ctype.h>
#  include <fcntl.h>

#  define Sprintf (void) sprintf
#  define WARN 1
#  define NOWARN 0

static char *
getcomspec(warn) {
	return getenv("COMSPEC");
}

#  ifdef SHELL
#   include <process.h>
dosh() {
	extern char orgdir[];
	char *comspec;

	if (comspec = getcomspec()) {
		settty("To return to NetHack, type \"exit\" at the DOS prompt.\n");
		chdirx(orgdir, 0);
		if (spawnl(P_WAIT, comspec, comspec, NULL) < 0) {
			printf("\nCan't spawn %s !\n", comspec);
			flags.toplin = 0;
			more();
		}
		chdirx(hackdir, 0);
		start_screen();
		docrt();
	} else
		pline("No COMSPEC !?  Can't exec COMMAND.COM");
	return(0);
}
#  endif /* SHELL */

/* Normal characters are output when the shift key is not pushed.
 * Shift characters are output when either shift key is pushed.
 */
#  define KEYPADHI	83
#  define KEYPADLOW	71
#  define iskeypad(x)	(KEYPADLOW <= (x) && (x) <= KEYPADHI)
static struct {
	char normal, shift;
	} keypad[KEYPADHI - KEYPADLOW + 1] = {
			{'y', 'Y'},		/* 7 */
			{'k', 'K'},		/* 8 */
			{'u', 'U'},		/* 9 */
			{'m', CTRL('P')},	/* - */
			{'h', 'H'},		/* 4 */
			{'g', 'g'},		/* 5 */
			{'l', 'L'},		/* 6 */
			{'p', 'P'},		/* + */
			{'b', 'B'},		/* 1 */
			{'j', 'J'},		/* 2 */
			{'n', 'N'},		/* 3 */
			{'i', 'I'},		/* Ins */
			{'.', ':'}		/* Del */
};

/* BIOSgetch gets keys directly with a BIOS call.
 */
#  define SHIFT		(0x1 | 0x2)
#  define KEYBRD_BIOS	0x16

static char
BIOSgetch() {
	unsigned char scan, shift, ch;
	union REGS regs;

	/* Get scan code.
	 */
	regs.h.ah = 0;
	int86(KEYBRD_BIOS, &regs, &regs);
	ch = regs.h.al;
	scan = regs.h.ah;

	/* Get shift status.
	 */
	regs.h.ah = 2;
	int86(KEYBRD_BIOS, &regs, &regs);
	shift = regs.h.al;

	/* If scan code is for the keypad, translate it.
	 */
	if (iskeypad(scan)) {
		if (shift & SHIFT)
			ch = keypad[scan - KEYPADLOW].shift;
		else
			ch = keypad[scan - KEYPADLOW].normal;
	}
	return ch;
}

/* construct the string  file.level */
void
name_file(file, level)
char *file;
int level;
{
	char *tf;
	
	if (tf = rindex(file, '.'))
		Sprintf(tf+1, "%d", level);
}


#  define FINDFIRST	0x4E00
#  define FINDNEXT	0x4F00
#  define GETDTA	0x2F00
#  define SETFILETIME	0x5701
#  define GETSWITCHAR	0x3700
#  define FREESPACE	0x36

static char
switchar()
{
	union REGS regs;

	regs.x.ax = GETSWITCHAR;
	intdos(&regs, &regs);
	return regs.h.dl;
}

long
freediskspace(path)
char *path;
{
	union REGS regs;

	regs.h.ah = FREESPACE;
	if (path[0] && path[1] == ':')
		regs.h.dl = (toupper(path[0]) - 'A') + 1;
	else
		regs.h.dl = 0;
	intdos(&regs, &regs);
	if (regs.x.ax == 0xFFFF)
		return -1L;		/* bad drive number */
	else
		return ((long) regs.x.bx * regs.x.cx * regs.x.ax);
}

/* Functions to get filenames using wildcards
 */
static
findfirst(path)
char *path;
{
	union REGS regs;
	struct SREGS sregs;

	regs.x.ax = FINDFIRST;
	regs.x.cx = 0;		/* normal files */
	regs.x.dx = FP_OFF(path);
	sregs.ds = FP_SEG(path);
	intdosx(&regs, &regs, &sregs);
	return !regs.x.cflag;
}

static
findnext() {
	union REGS regs;

	regs.x.ax = FINDNEXT;
	intdos(&regs, &regs);
	return !regs.x.cflag;
}

#ifndef __TURBOC__
/* Get disk transfer area, Turbo C already has getdta */
static char *
getdta() {
	union REGS regs;
	struct SREGS sregs;
	char *ret;

	regs.x.ax = GETDTA;
	intdosx(&regs, &regs, &sregs);
	FP_OFF(ret) = regs.x.bx;
	FP_SEG(ret) = sregs.es;
	return ret;
}
#endif

long
filesize(file)
char *file;
{
	char *dta;

	if (findfirst(file)) {
		dta = getdta();
		return  (* (long *) (dta + 26));
	} else
		return -1L;
}

void
eraseall(path, files)
char *path, *files;
{
	char	*dta, buf[PATHLEN];

	dta = getdta();
	Sprintf(buf, "%s%s", path, files);
	if (findfirst(buf))
		do {
			Sprintf(buf, "%s%s", path, dta + 30);
			(void) unlink(buf);
		} while (findnext());
}

/* Rewritten for version 3.3 to be faster
 */
void
copybones(mode) {
	char from[PATHLEN], to[PATHLEN], last[13], copy[8];
	char *frompath, *topath, *dta, *comspec;
	int status;
	long fs;
	extern saveprompt;

	if (!ramdisk)
		return;

	/* Find the name of the last file to be transferred
	 */
	frompath = (mode != TOPERM) ? permbones : levels;
	dta = getdta();
	last[0] = '\0';
	Sprintf(from, "%s%s", frompath, allbones);
	if (findfirst(from))
		do {
			strcpy(last, dta + 30);
		} while (findnext());

	topath = (mode == TOPERM) ? permbones : levels;
	if (last[0]) {
		Sprintf(copy, "%cC copy", switchar());

		/* Remove any bones files in `to' directory.
		 */
		eraseall(topath, allbones);

		/* Copy `from' to `to' */
		Sprintf(to, "%s%s", topath, allbones);
		comspec = getcomspec();
		status =spawnl(P_WAIT, comspec, comspec, copy, from,
			to, "> nul", NULL);
	} else
		return;

	/* See if the last file got there.  If so, remove the ramdisk bones
	 * files.
	 */
	Sprintf(to, "%s%s", topath, last);
	if (findfirst(to)) {
		if (mode == TOPERM)
			eraseall(frompath, allbones);
		return;
	}

	/* Last file didn't get there.
	 */
	Sprintf(to, "%s%s", topath, allbones);
	msmsg("Cannot copy `%s' to `%s' -- %s\n", from, to,
		(status < 0) ? "can't spawn COMSPEC !" :
		(freediskspace(topath) < filesize(from)) ?
			"insufficient disk space." : "bad path(s)?");
	if (mode == TOPERM) {
		msmsg("Bones will be left in `%s'\n",
			*levels ? levels : hackdir);
		return;
	} else {
		/* Remove all bones files on the RAMdisk */
		eraseall(levels, allbones);
		playwoRAMdisk();
	}
}

playwoRAMdisk() {
	msmsg("Do you wish to play without a RAMdisk (y/n) ? ");

	/* Set ramdisk false *before* exit'ing (because msexit calls
	 * copybones)
	 */
	ramdisk = FALSE;
	if (getchar() != 'y') {
		settty("Be seeing you ...\n");
		exit(0);
	}
	set_lock_and_bones();
	return;
}

saveDiskPrompt(start) {
	extern saveprompt;
	char buf[BUFSIZ], *bp;
	int fd;

	if (saveprompt) {
		/* Don't prompt if you can find the save file */
		if ((fd = open(SAVEF, 0)) >= 0) {
			(void) close(fd);
			return 1;
		}
		remember_topl();
		home();
		cl_end();
		msmsg("If save file is on a SAVE disk, put that disk in now.\n");
		cl_end();
		msmsg("File name (default `%s'%s) ? ", SAVEF,
			start ? "" : ", <Esc> cancels save");
		getlin(buf);
		home();
		cl_end();
		curs(1, 2);
		cl_end();
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

/* Return 1 if the record file was found */
static
record_exists() {
	int fd;

	if ((fd = open(RECORD, 0)) >= 0) {
		close(fd);
		return TRUE;
	}
	return FALSE;
}

/* Return 1 if the comspec was found */
static
comspec_exists() {
	int fd;
	char *comspec;

	if (comspec = getcomspec())
		if ((fd = open(comspec, 0)) >= 0) {
			close(fd);
			return TRUE;
		}
	return FALSE;
}

/* Prompt for game disk, then check for record file.
 */
void
gameDiskPrompt() {
	extern saveprompt;

	if (saveprompt) {
		if (record_exists() && comspec_exists())
			return;
		(void) putchar('\n');
		getreturn("when the GAME disk has been put in");
	}
	if (comspec_exists() && record_exists())
		return;

	if (!comspec_exists())
		msmsg("\n\nWARNING: can't find comspec `%s'!\n", getcomspec());
	if (!record_exists())
		msmsg("\n\nWARNING: can't find record file `%s'!\n", RECORD);
	msmsg("If the GAME disk is not in, put it in now.\n");
	getreturn("to continue");
}

/* Read configuration */
void
read_config_file() {
	char	tmp_ramdisk[PATHLEN], tmp_levels[PATHLEN];
	char	buf[BUFSZ], *bufp;
	FILE	*fp, *fopenp();
	extern	char plname[];
	extern	int saveprompt;

	tmp_ramdisk[0] = 0;
	tmp_levels[0] = 0;
	if ((fp = fopenp(configfile, "r")) == NULL) {
		msmsg("Warning: no configuration file!\n");
		getreturn("to continue");
		return;
	}
	while (fgets(buf, BUFSZ, fp)) {
		if (*buf == '#')
			continue;

		/* remove trailing whitespace
		 */
		bufp = index(buf, '\n');
		while (bufp > buf && isspace(*bufp))
			bufp--;
		if (bufp == buf)
			continue;		/* skip all-blank lines */
		else
			*(bufp + 1) = 0;	/* 0 terminate line */

		/* find the '=' */
		if (!(bufp = strchr(buf, '='))) {
			msmsg("Bad option line: '%s'\n", buf);
			getreturn("to continue");
			continue;
		}
		
		/* skip  whitespace between '=' and value */
		while (isspace(*++bufp))
			;

		/* Go through possible variables */
		if (!strncmp(buf, "HACKDIR", 4)) {
			strncpy(hackdir, bufp, PATHLEN);
		
		} else if (!strncmp(buf, "RAMDISK", 3)) {
			strncpy(tmp_ramdisk, bufp, PATHLEN);

		} else if (!strncmp(buf, "LEVELS", 4)) {
			strncpy(tmp_levels, bufp, PATHLEN);

		} else if (!strncmp(buf, "OPTIONS", 4)) {
			parseoptions(bufp, TRUE);
			if (plname[0])		/* If a name was given */
				plnamesuffix();	/* set the character class */

		} else if (!strncmp(buf, "SAVE", 4)) {
			char *ptr;
			if (ptr = index(bufp, ';')) {
				*ptr = '\0';
				if (*(ptr+1) == 'n' || *(ptr+1) == 'N')
					saveprompt = FALSE;
			}
			(void) strncpy(SAVEF, bufp, PATHLEN);
			append_slash(SAVEF);
#ifdef GRAPHICS
		} else if (!strncmp(buf, "GRAPHICS", 4)) {
			char translate[17];
			short i;

		     if ((i = sscanf(bufp, "%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u%u",
				&translate[0], &translate[1], &translate[2],
				&translate[3], &translate[4], &translate[5],
				&translate[6], &translate[7], &translate[8],
				&translate[9], &translate[10], &translate[11],
				&translate[12], &translate[13], &translate[14],
				&translate[15], &translate[16])) < 0) {
					msmsg ("Syntax error in GRAPHICS\n");
					getreturn("to continue");
			}
			translate[i] = '\0';
#endif /* GRAPHICS /**/
/*
 * You could have problems here if you configure FOUNTAINS, SPIDERS or NEWCLASS
 * in or out and forget to change the tail entries in your graphics string.
 */
#define SETPCHAR(f, n)	showsyms.f = (strlen(translate) > n) ? translate[n] : defsyms.f
			SETPCHAR(stone, 0);
			SETPCHAR(vwall, 1);
			SETPCHAR(hwall, 2);
			SETPCHAR(tlcorn, 3);
			SETPCHAR(trcorn, 4);
			SETPCHAR(blcorn, 5);
			SETPCHAR(brcorn, 6);
			SETPCHAR(door, 7);
			SETPCHAR(room, 8);
			SETPCHAR(corr, 9);
			SETPCHAR(upstair, 10);
			SETPCHAR(dnstair, 11);
			SETPCHAR(trap, 12);
#ifdef FOUNTAINS
			SETPCHAR(pool, 13);
			SETPCHAR(fountain, 14);
#endif
#ifdef NEWCLASS
			SETPCHAR(throne, 15);
#endif
#ifdef SPIDERS
			SETPCHAR(web, 16);
#endif
#undef SETPCHAR
		} else {
			msmsg("Bad option line: '%s'\n", buf);
			getreturn("to continue");
		}
	}
	fclose(fp);

	strcpy(permbones, tmp_levels);
	if (tmp_ramdisk[0]) {
		strcpy(levels, tmp_ramdisk);
		if (strcmpi(permbones, levels))		/* if not identical */
			ramdisk = TRUE;
	} else
		strcpy(levels, tmp_levels);
	strcpy(bones, levels);
}

/* Set names for bones[] and lock[]
 */
void
set_lock_and_bones() {
	if (!ramdisk) {
		strcpy(levels, permbones);
		strcpy(bones, permbones);
	}
	append_slash(permbones);
	append_slash(levels);
	append_slash(bones);
	strcat(bones, allbones);
	strcpy(lock, levels);
	strcat(lock, alllevels);
}

/* Add a backslash to any name not ending in /, \ or :   There must
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
}


void
getreturn(str)
char *str;
{
	int ch;

	msmsg("Hit <RETURN> %s.", str);
	while ((ch = getchar()) != '\n')
		;
}

void
msmsg(fmt, a1, a2, a3)
char *fmt;
long a1, a2, a3;
{
	printf(fmt, a1, a2, a3);
	flushout();
}

/* Chdrive() changes the default drive.
 */
#define SELECTDISK	0x0E
void
chdrive(str)
char *str;
{
	char *ptr;
	union REGS inregs;
	char drive;

	if ((ptr = index(str, ':')) != NULL) {
		drive = toupper(*(ptr - 1));
		inregs.h.ah = SELECTDISK;
		inregs.h.dl = drive - 'A';
		intdos(&inregs, &inregs);
	}
}

/* Use the IOCTL DOS function call to change stdin and stdout to raw
 * mode.  For stdin, this prevents MSDOS from trapping ^P, thus
 * freeing us of ^P toggling 'echo to printer'.
 * Thanks to Mark Zbikowski (markz@microsoft.UUCP).
 */

#  define DEVICE	0x80
#  define RAW		0x20
#  define IOCTL		0x44
#  define STDIN		fileno(stdin)
#  define STDOUT	fileno(stdout)
#  define GETBITS	0
#  define SETBITS	1

static unsigned	old_stdin, old_stdout, ioctl();

disable_ctrlP() {
	if (!flags.rawio)
		return;
	old_stdin = ioctl(STDIN, GETBITS, 0);
	old_stdout = ioctl(STDOUT, GETBITS, 0);
	if (old_stdin & DEVICE)
		ioctl(STDIN, SETBITS, old_stdin | RAW);
	if (old_stdout & DEVICE)
		ioctl(STDOUT, SETBITS, old_stdout | RAW);
}

enable_ctrlP() {
	if (!flags.rawio)
		return;
	if (old_stdin)
		(void) ioctl(STDIN, SETBITS, old_stdin);
	if (old_stdout)
		(void) ioctl(STDOUT, SETBITS, old_stdout);
}

static unsigned
ioctl(handle, mode, setvalue)
unsigned setvalue;
{
	union REGS regs;

	regs.h.ah = IOCTL;
	regs.h.al = mode;
	regs.x.bx = handle;
	regs.h.dl = setvalue;
	regs.h.dh = 0;			/* Zero out dh */
	intdos(&regs, &regs);
	return (regs.x.dx);
}

/* Follow the PATH, trying to fopen the file.
 */
#define PATHSEP	';'

FILE *
fopenp(name, mode)
char *name, *mode;
{
	char buf[BUFSIZ], *bp, *pp, *getenv(), lastch;
	FILE *fp;

	/* Try the default directory first.  Then look along PATH.
	 */
	strcpy(buf, name);
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
			strcpy(bp, name);
			if (fp = fopen(buf, mode))
				return fp;
			if (*pp)
				pp++;
		}
	}
	return NULL;
}
# endif /* DGK */

/* Chdir back to original directory
 */
# undef exit
void
msexit(code)
{
# ifdef CHDIR
	extern char orgdir[];
# endif

# ifdef DGK
	flushout();
	enable_ctrlP();		/* in case this wasn't done */
	if (ramdisk)
		copybones(TOPERM);
# endif
# ifdef CHDIR
	chdir(orgdir);		/* chdir, not chdirx */
#  ifdef DGK
	chdrive(orgdir);
#  endif
# endif
	exit(code);
}
#endif /* MSDOS */
