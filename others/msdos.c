/*	SCCS Id: @(#)msdos.c	3.0	88/11/20
/* NetHack may be freely redistributed.  See license for details. */
/* An assortment of MSDOS functions.
 */

#include "hack.h"
#ifdef MSDOS
# ifdef TOS
#  include <osbind.h>
# else
#  ifdef __TURBOC__	/* avoid incompatible redeclaration */
#   define getdate quux
#  endif
#  include <dos.h>
#  ifdef __TURBOC__
#   undef getdate
#  endif
# endif
#ifdef OS2
# include "def_os2.h"    /* OS2 definitions (Timo Hakulinen) */
#endif
static char DOSgetch();
#ifdef DGK
static char BIOSgetch();
#endif
static unsigned int ioctl();

void
flushout()
{
	(void) fflush(stdout);
	return;
}

int
tgetch() {
	char ch;

#ifdef DGK
	/* BIOSgetch can use the numeric key pad on IBM compatibles. */
	if (flags.IBMBIOS)
		ch = BIOSgetch();
	else
#endif
		ch = DOSgetch();
	return ((ch == '\r') ? '\n' : ch);
}

#ifndef OS2
#define DIRECT_INPUT	0x7
#endif
static char
DOSgetch() {
#ifdef OS2
	KBDKEYINFO CharData;
	USHORT IOWait = 0;
	HKBD KbdHandle = 0;

	KbdCharIn(&CharData,IOWait,KbdHandle);
	if (CharData.chChar == 0) {	/* an extended code -- not yet supported */
		KbdCharIn(&CharData,IOWait,KbdHandle);	   /* eat the next character */
		CharData.chChar = 0;		/* and return a 0 */
	}
	return (CharData.chChar);
#else
#ifdef TOS
	return (Crawcin() & 0x007f);
#else
    union REGS regs;

	regs.h.ah = DIRECT_INPUT;
	intdos(&regs, &regs);
	if (!regs.h.al) {	/* an extended code -- not yet supported */
		regs.h.ah = DIRECT_INPUT;
		intdos(&regs, &regs);	/* eat the next character */
		regs.h.al = 0;		/* and return a 0 */
	}
	return (regs.h.al);
#endif /* TOS */
#endif /* OS2 */
}

#include <ctype.h>
#include <fcntl.h>
#include <process.h>

static char *COMSPEC = 
#ifdef TOS
"SHELL";
#else
"COMSPEC";
#endif

#define getcomspec() getenv(COMSPEC)

#ifdef SHELL
int
dosh() {
	extern char orgdir[];
	char *comspec;

	if (comspec = getcomspec()) {
#ifdef DGK
		settty("To return to NetHack, type \"exit\" at the DOS prompt.\n");
#else
		settty((char *)0);
#endif /* DGK */
		chdirx(orgdir, 0);
		if (spawnl(P_WAIT, comspec, comspec, NULL) < 0) {
			Printf("\nCan't spawn %s !\n", comspec);
			flags.toplin = 0;
			more();
		}
		gettty(); /* ctrl-P might get turned back on (TH) */
		chdirx(hackdir, 0);
		start_screen();
		docrt();
	} else
#ifdef OS2
		pline("Cannot exec CMD.EXE");
#else
#ifdef TOS
		pline("Cannot find SHELL");
#else
		pline("Cannot exec COMMAND.COM");
#endif
#endif /* OS2 */
	return 0;
}
#endif /* SHELL */

#ifdef DGK
/* Normal characters are output when the shift key is not pushed.
 * Shift characters are output when either shift key is pushed.
 */
#ifdef TOS
#define KEYPADHI	113
#define KEYPADLOW	103
#else
#define KEYPADHI	83
#define KEYPADLOW	71
#endif

#define PADKEYS		(KEYPADHI - KEYPADLOW + 1)
#define iskeypad(x)	(KEYPADLOW <= (x) && (x) <= KEYPADHI)
static const struct pad {
	char normal, shift;
	} keypad[PADKEYS] = {
			{'y', 'Y'},		/* 7 */
			{'k', 'K'},		/* 8 */
			{'u', 'U'},		/* 9 */
#ifndef TOS
			{'m', CTRL('P')},	/* - */
#endif
			{'h', 'H'},		/* 4 */
#ifdef TOS
			{'.', '.'},
#else
			{'g', 'g'},		/* 5 */
#endif
			{'l', 'L'},		/* 6 */
#ifndef TOS
			{'p', 'P'},		/* + */
#endif
			{'b', 'B'},		/* 1 */
			{'j', 'J'},		/* 2 */
			{'n', 'N'},		/* 3 */
			{'i', 'I'},		/* Ins */
			{'.', ':'}		/* Del */
	}, numpad[PADKEYS] = {
			{'7', '7'},		/* 7 */
			{'8', '8'},		/* 8 */
			{'9', '9'},		/* 9 */
#ifndef TOS
			{'m', CTRL('P')},	/* - */
#endif
			{'4', '4'},		/* 4 */
#ifdef TOS
			{'.', '.'},		/* 5 */
#else
			{'g', 'G'},		/* 5 */
#endif
			{'6', '6'},		/* 6 */
#ifndef TOS
			{'p', 'P'},		/* + */
#endif
			{'1', '1'},		/* 1 */
			{'2', '2'},		/* 2 */
			{'3', '3'},		/* 3 */
			{'i', 'I'},		/* Ins */
			{'.', ':'}		/* Del */
};

/* BIOSgetch gets keys directly with a BIOS call.
 */
#define SHIFT		(0x1 | 0x2)
/* #define CTRL		0x4 */
/* #define ALT		0x8 */
#ifndef OS2
#define KEYBRD_BIOS	0x16
#endif

static char
BIOSgetch() {
	unsigned char scan, shift, ch;
	struct pad (*kpad)[PADKEYS];

#ifdef OS2
	KBDKEYINFO CharData;
	USHORT IOWait = 0;
	HKBD KbdHandle = 0;

	KbdCharIn(&CharData,IOWait,KbdHandle);
	ch = CharData.chChar;
	scan = CharData.chScan;
	shift = CharData.fsState;
#else /* OS2 */
#ifdef TOS
	long  x;
#else
	union REGS regs;
#endif

	/* Get scan code.
	 */
#ifdef TOS
	x = Crawcin();
	ch = x & 0x0ff;
	scan = (x & 0x00ff0000L) >> 16;
#else
	regs.h.ah = 0;
	int86(KEYBRD_BIOS, &regs, &regs);
	ch = regs.h.al;
	scan = regs.h.ah;
#endif
	/* Get shift status.
	 */
#ifdef TOS
	shift = Kbshift(-1);
#else
	regs.h.ah = 2;
	int86(KEYBRD_BIOS, &regs, &regs);
	shift = regs.h.al;
#endif
#endif /* OS2 */

	/* If scan code is for the keypad, translate it.
	 */
	kpad = flags.num_pad ? numpad : keypad;
	if (iskeypad(scan)) {
		if (shift & SHIFT) {
			/* if number_pad is on, this makes little difference */
			ch = (*kpad)[scan - KEYPADLOW].shift;
		} else
			ch = (*kpad)[scan - KEYPADLOW].normal;
	}
	return ch;
}

#ifndef TOS

#ifndef OS2
#define FINDFIRST	0x4E00
#define FINDNEXT	0x4F00
#define GETDTA		0x2F00
#define SETFILETIME	0x5701
#define GETSWITCHAR	0x3700
#define FREESPACE	0x36
#endif

#ifdef __TURBOC__
#define switchar()	(char)getswitchar()
#else
#ifndef OS2
static char
switchar()
{
    union REGS regs;

	regs.x.ax = GETSWITCHAR;
	intdos(&regs, &regs);
	return regs.h.dl;
}
#endif /* OS2 */
#endif

long
freediskspace(path)
char *path;
{
#ifdef OS2
	struct {
		ULONG  idFileSystem;
		ULONG  cSectorUnit;
		ULONG  cUnit;
		ULONG  cUnitAvail;
		USHORT cbSector;
	} FSInfoBuf;
	USHORT DriveNumber, FSInfoLevel = 1, res;

	if (path[0] && path[1] == ':')
		DriveNumber = (toupper(path[0]) - 'A') + 1;
	else
		DriveNumber = 0;
	res = DosQFSInfo(DriveNumber,FSInfoLevel,(PBYTE)&FSInfoBuf,sizeof(FSInfoBuf));
	if (res)
		return -1L;		/* error */
	else
		return ((long) FSInfoBuf.cSectorUnit * FSInfoBuf.cUnitAvail *
			       FSInfoBuf.cbSector);
#else
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
#endif /* OS2 */
}

#ifdef OS2
FILEFINDBUF ResultBuf;
HDIR DirHandle;
#endif

/* Functions to get filenames using wildcards
 */
static int
findfirst(path)
char *path;
{
#ifdef OS2
	USHORT res, SearchCount = 1;

	DirHandle = 1;
	res = DosFindFirst((PSZ)path,&DirHandle,0,&ResultBuf,sizeof(FILEFINDBUF),&SearchCount,0L);
	return(!res);
#else
	union REGS regs;
	struct SREGS sregs;

	regs.x.ax = FINDFIRST;
	regs.x.cx = 0;		/* normal files */
	regs.x.dx = FP_OFF(path);
	sregs.ds = FP_SEG(path);
	intdosx(&regs, &regs, &sregs);
	return !regs.x.cflag;
#endif /* OS2 */
}

static int
findnext() {
#ifdef OS2
	USHORT res, SearchCount = 1;

	res = DosFindNext(DirHandle,&ResultBuf,sizeof(FILEFINDBUF),&SearchCount);
	return(!res);
#else
	union REGS regs;

	regs.x.ax = FINDNEXT;
	intdos(&regs, &regs);
	return !regs.x.cflag;
#endif /* OS2 */
}

#ifndef OS2
/* Get disk transfer area, Turbo C already has getdta */
static char *
getdta() {
	union REGS regs;
	struct SREGS sregs;
	char *ret;

	regs.x.ax = GETDTA;
	intdosx(&regs, &regs, &sregs);
#ifdef MK_FP
	ret = MK_FP(sregs.es, regs.x.bx);
#else
	FP_OFF(ret) = regs.x.bx;
	FP_SEG(ret) = sregs.es;
#endif
	return ret;
}
#endif  /* OS2 */

#else /* TOS */

long
freediskspace(path)
char *path;
{
	int drive = 0;
	struct {
		long freal; /*free allocation units*/
		long total; /*total number of allocation units*/
		long bps;   /*bytes per sector*/
		long pspal; /*physical sectors per allocation unit*/
	} freespace;
	if (path[0] && path[1] == ':')
		drive = (toupper(path[0]) - 'A') + 1;
	if (Dfree(&freespace,drive)<0) return -1;
	return freespace.freal*freespace.bps*freespace.pspal;
}

static int
findfirst(path)
char *path;
{
	return (Fsfirst(path, 0) == 0);
}

static int findnext() {
	return (Fsnext() == 0);
}

static char *
getdta() {
	return (char *) Fgetdta();
}

#endif /* TOS */

long
filesize(file)
char *file;
{
#ifndef OS2
	char *dta;
#endif

	if (findfirst(file)) {
#ifdef OS2
		return  (* (long *) (ResultBuf.cbFileAlloc));
#else
		dta = getdta();
		return  (* (long *) (dta + 26));
#endif
	} else
		return -1L;
}

void
eraseall(path, files)
char *path, *files;
{
	char
#ifndef OS2
		*dta,
#endif
	buf[PATHLEN];

#ifndef OS2
	dta = getdta();
#endif
	Sprintf(buf, "%s%s", path, files);
	if (findfirst(buf))
		do {
			Sprintf(buf, "%s%s", path,
#ifdef OS2
				ResultBuf.achName
#else
				dta + 30
#endif
				);
			(void) unlink(buf);
		} while (findnext());
	return;
}

/* Rewritten for version 3.3 to be faster
 */
void
copybones(mode) {
	char from[PATHLEN], to[PATHLEN], last[13], copy[8];
	char *frompath, *topath,
#ifndef OS2
	*dta,
#endif
	*comspec;
	int status;
	long fs;
	extern saveprompt;

#ifdef TOS
	extern int _copyfile();
#endif
	if (!ramdisk)
		return;

	/* Find the name of the last file to be transferred
	 */
	frompath = (mode != TOPERM) ? permbones : levels;
#ifndef OS2
	dta = getdta();
#endif
	last[0] = '\0';
	Sprintf(from, "%s%s", frompath, allbones);
	topath = (mode == TOPERM) ? permbones : levels;
#ifdef TOS
	eraseall(topath, allbones);
#endif
	if (findfirst(from))
		do {
#ifdef TOS
			Sprintf(from, "%s%s", frompath, dta+30); 
			Sprintf(to, "%s%s", topath, dta+30);
			if (_copyfile(from, to))
				goto error_copying;
#endif
			Strcpy(last,
#ifdef OS2
				ResultBuf.achName
#else
				dta + 30
#endif
				);
		} while (findnext());
#ifdef TOS
	else
		return;
#else
	if (last[0]) {
		Sprintf(copy, "%cC copy",
#ifdef OS2
			'/'
#else
			switchar()
#endif
			);

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
#endif /* TOS */

	/* See if the last file got there.  If so, remove the ramdisk bones
	 * files.
	 */
	Sprintf(to, "%s%s", topath, last);
	if (findfirst(to)) {
		if (mode == TOPERM)
			eraseall(frompath, allbones);
		return;
	}

error_copying:
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
	} else {
		/* Remove all bones files on the RAMdisk */
		eraseall(levels, allbones);
		playwoRAMdisk();
	}
	return;
}

void
playwoRAMdisk() {
	msmsg("Do you wish to play without a RAMdisk? ");

	/* Set ramdisk false *before* exit'ing (because msexit calls
	 * copybones)
	 */
	ramdisk = FALSE;
	if (yn() != 'y') {
		settty("Be seeing you...\n");
		exit(0);
	}
	set_lock_and_bones();
	return;
}

int
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
static boolean
record_exists() {
	int fd;
#ifdef OS2_CODEVIEW
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,RECORD);
	if ((fd = open(tmp, 0)) >= 0) {
#else
	if ((fd = open(RECORD, 0)) >= 0) {
#endif
		(void) close(fd);
		return TRUE;
	}
	return FALSE;
}

#ifdef TOS
#define comspec_exists() 1
#else
/* Return 1 if the comspec was found */
static boolean
comspec_exists() {
	int fd;
	char *comspec;

	if (comspec = getcomspec())
		if ((fd = open(comspec, 0)) >= 0) {
			(void) close(fd);
			return TRUE;
		}
	return FALSE;
}
#endif

/* Prompt for game disk, then check for record file.
 */
void
gameDiskPrompt() {
	extern int saveprompt;

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
	return;
}
#endif /* DGK */

/* Read configuration */
void
read_config_file() {
#ifdef DGK
	char	tmp_ramdisk[PATHLEN];
	extern	int saveprompt;
	FILE	*fopenp();
#else
#define fopenp fopen
#endif
	char	tmp_levels[PATHLEN];
	char	buf[BUFSZ], *bufp;
	FILE	*fp;
	extern	char plname[];

#ifdef DGK
	tmp_ramdisk[0] = 0;
#endif
	tmp_levels[0] = 0;
	if ((fp = fopenp(configfile, "r")) == (FILE *)0) {
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
		
#ifdef DGK
		} else if (!strncmp(buf, "RAMDISK", 3)) {
			strncpy(tmp_ramdisk, bufp, PATHLEN);
#endif

		} else if (!strncmp(buf, "LEVELS", 4)) {
			strncpy(tmp_levels, bufp, PATHLEN);

		} else if (!strncmp(buf, "OPTIONS", 4)) {
			parseoptions(bufp, TRUE);
			if (plname[0])		/* If a name was given */
				plnamesuffix();	/* set the character class */

		} else if (!strncmp(buf, "SAVE", 4)) {
#ifdef DGK
			char *ptr;
			if (ptr = index(bufp, ';')) {
				*ptr = '\0';
				if (*(ptr+1) == 'n' || *(ptr+1) == 'N')
					saveprompt = FALSE;
			}
#endif /* DGK */
			(void) strncpy(SAVEF, bufp, PATHLEN);
			append_slash(SAVEF);
		} else if (!strncmp(buf, "GRAPHICS", 4)) {
			unsigned int translate[MAXPCHARS];
			int i;

		     if ((i = sscanf(bufp,
		 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
				&translate[0], &translate[1], &translate[2],
				&translate[3], &translate[4], &translate[5],
				&translate[6], &translate[7], &translate[8],
				&translate[9], &translate[10], &translate[11],
				&translate[12], &translate[13], &translate[14],
				&translate[15], &translate[16], &translate[17],
				&translate[18], &translate[19], &translate[20],
				&translate[21], &translate[22], &translate[23],
				&translate[24], &translate[25], &translate[26],
				&translate[27], &translate[28], &translate[29],
				&translate[30], &translate[31])) < 0) {
					msmsg ("Syntax error in GRAPHICS\n");
					getreturn("to continue");
			}
#define SETPCHAR(f, n)	showsyms.f = (i > n) ? translate[n] : defsyms.f
			SETPCHAR(stone, 0);
			SETPCHAR(vwall, 1);
			SETPCHAR(hwall, 2);
			SETPCHAR(tlcorn, 3);
			SETPCHAR(trcorn, 4);
			SETPCHAR(blcorn, 5);
			SETPCHAR(brcorn, 6);
			SETPCHAR(crwall, 7);
			SETPCHAR(tuwall, 8);
			SETPCHAR(tdwall, 9);
			SETPCHAR(tlwall, 10);
			SETPCHAR(trwall, 11);
			SETPCHAR(vbeam, 12);
			SETPCHAR(hbeam, 13);
			SETPCHAR(lslant, 14);
			SETPCHAR(rslant, 15);
			SETPCHAR(door, 16);
			SETPCHAR(room, 17);
			SETPCHAR(corr, 18);
			SETPCHAR(upstair, 19);
			SETPCHAR(dnstair, 20);
			SETPCHAR(trap, 21);
			SETPCHAR(web, 22);
			SETPCHAR(pool, 23);
#ifdef FOUNTAINS
			SETPCHAR(fountain, 24);
#endif
#ifdef SINKS
			SETPCHAR(sink, 25);
#endif
#ifdef THRONES
			SETPCHAR(throne, 26);
#endif
#ifdef ALTARS
			SETPCHAR(altar, 27);
#endif
#ifdef STRONGHOLD
			SETPCHAR(upladder, 28);
			SETPCHAR(dnladder, 29);
			SETPCHAR(dbvwall, 30);
			SETPCHAR(dbhwall, 31);
#endif
#undef SETPCHAR
		} else {
			msmsg("Bad option line: '%s'\n", buf);
			getreturn("to continue");
		}
	}
	(void) fclose(fp);

#ifdef DGK
	Strcpy(permbones, tmp_levels);
	if (tmp_ramdisk[0]) {
		Strcpy(levels, tmp_ramdisk);
		if (strcmp(permbones, levels))		/* if not identical */
			ramdisk = TRUE;
	} else
#endif /* DGK */
		Strcpy(levels, tmp_levels);
	Strcpy(bones, levels);
	return;
}

#ifdef DGK
/* Set names for bones[] and lock[]
 */
void
set_lock_and_bones() {
	if (!ramdisk) {
		Strcpy(levels, permbones);
		Strcpy(bones, permbones);
	}
	append_slash(permbones);
	append_slash(levels);
	append_slash(bones);
	Strcat(bones, allbones);
	Strcpy(lock, levels);
	Strcat(lock, alllevels);
	return;
}
#endif /* DGK */

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
	return;
}

void
getreturn(str)
char *str;
{
	msmsg("Hit <RETURN> %s.", str);
	while (Getchar() != '\n') ;
	return;
}

void
msmsg(fmt, a1, a2, a3)
char *fmt;
long a1, a2, a3;
{
	Printf(fmt, a1, a2, a3);
	flushout();
	return;
}

/* Chdrive() changes the default drive.
 */
#ifndef __TURBOC__
#ifndef OS2
#define SELECTDISK	0x0E
#endif
void
chdrive(str)
char *str;
{
	char *ptr;
#ifndef TOS
#ifndef OS2
	union REGS inregs;
#endif
#endif
	char drive;

	if ((ptr = index(str, ':')) != NULL) {
		drive = toupper(*(ptr - 1));
#ifdef TOS
		Dsetdrv(drive - 'A');
#else
#ifdef OS2
		DosSelectDisk((USHORT)(drive - 'A' + 1));
#else
		inregs.h.ah = SELECTDISK;
		inregs.h.dl = drive - 'A';
		intdos(&inregs, &inregs);
#endif
#endif
	}
	return;
}
#else
void
chdrive(str)
char *str;
{
	if (str[1] == ':')
		(void)setdisk((int)(toupper(str[0]) - 'A'));
	return;
}
#endif

#ifndef TOS
/* Use the IOCTL DOS function call to change stdin and stdout to raw
 * mode.  For stdin, this prevents MSDOS from trapping ^P, thus
 * freeing us of ^P toggling 'echo to printer'.
 * Thanks to Mark Zbikowski (markz@microsoft.UUCP).
 */

#ifndef OS2
#define DEVICE		0x80
#define RAW		0x20
#define IOCTL		0x44
#define STDIN		fileno(stdin)
#define STDOUT		fileno(stdout)
#define GETBITS		0
#define SETBITS		1
#endif

static unsigned	int old_stdin, old_stdout;

void
disable_ctrlP() {
#ifdef OS2
	KBDINFO KbdInfo;
	HKBD KbdHandle = 0;
#endif

#ifdef DGK
	if (!flags.rawio) return;
#endif
#ifdef OS2
	KbdInfo.cb = 10;
	KbdGetStatus(&KbdInfo,KbdHandle);
	KbdInfo.fsMask &= 0xFFF7; /* ASCII off */
	KbdInfo.fsMask |= 0x0004; /* BINARY on */
	KbdSetStatus(&KbdInfo,KbdHandle);
#else
	old_stdin = ioctl(STDIN, GETBITS, 0);
	old_stdout = ioctl(STDOUT, GETBITS, 0);
	if (old_stdin & DEVICE)
		ioctl(STDIN, SETBITS, old_stdin | RAW);
	if (old_stdout & DEVICE)
		ioctl(STDOUT, SETBITS, old_stdout | RAW);
#endif /* OS2 */
	return;
}

void
enable_ctrlP() {
#ifdef OS2
	KBDINFO KbdInfo;
	HKBD KbdHandle = 0;
#endif

#ifdef DGK
	if (!flags.rawio) return;
#endif
#ifdef OS2
	KbdInfo.cb = 10;
	KbdGetStatus(&KbdInfo,KbdHandle);
	KbdInfo.fsMask &= 0xFFFB; /* BINARY off */
	KbdInfo.fsMask |= 0x0008; /* ASCII on */
	KbdSetStatus(&KbdInfo,KbdHandle);
#else
	if (old_stdin)
		(void) ioctl(STDIN, SETBITS, old_stdin);
	if (old_stdout)
		(void) ioctl(STDOUT, SETBITS, old_stdout);
#endif
	return;
}

#ifndef OS2
static unsigned int
ioctl(handle, mode, setvalue)
int handle, mode;
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
#endif /* OS2 */
#endif /* TOS */

#ifdef DGK
/* Follow the PATH, trying to fopen the file.
 */
#ifdef TOS
#define PATHSEP ','
#else
#define PATHSEP	';'
#endif

FILE *
fopenp(name, mode)
char *name, *mode;
{
	char buf[BUFSIZ], *bp, *pp, lastch;
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
#endif /* DGK */

/* Chdir back to original directory
 */
#undef exit
void exit(int);
void
msexit(code)
{
#ifdef CHDIR
	extern char orgdir[];
#endif

	flushout();
#ifndef TOS
	enable_ctrlP();		/* in case this wasn't done */
#endif
#ifdef DGK
	if (ramdisk) copybones(TOPERM);
#endif
#ifdef CHDIR
	chdir(orgdir);		/* chdir, not chdirx */
	chdrive(orgdir);
#endif
#ifdef TOS
	getreturn("to continue"); /* so the user can read the score list */
#endif
	exit(code);
	return;
}

# ifdef DGK		/* for flags.IBMBIOS */
void
get_scr_size()
{
#  ifdef OS2
	VIOMODEINFO ModeInfo;
	HVIO VideoHandle = 0;

	ModeInfo.cb = sizeof(ModeInfo);

	VioGetMode(&ModeInfo,VideoHandle);

	CO = ModeInfo.col;
	LI = ModeInfo.row;
#  else
	union REGS regs;

	if (!flags.IBMBIOS) {		/* assume standard screen size */
		CO = 80;
		LI = 24;
		return;
	}

	regs.x.ax = 0x1130;		/* Func AH = 11h, Subfunc AL = 30h */
	regs.x.bx = 0;			/* current ROM BIOS font */
	regs.h.dl = 24;			/* default row count */
					/* in case no EGA/MCGA/VGA */
	int86(0x10, &regs, &regs);	/* Get Font Information */

	/* MDA/CGA/PCjr ignore INT 10h, Function 11h, but since we
	 * cleverly loaded up DL with the default, everything's fine.
	 *
	 * Otherwise, DL now contains rows - 1.  Also, CX contains the
	 * points (bytes per character) and ES:BP points to the font
	 * table.  -3.
	 */

	regs.h.ah = 0x0f;
	int86(0x10, &regs, &regs);	/* Get Video Mode */

	/* This goes back all the way to the original PC.  Completely
	 * safe.  AH contains # of columns, AL contains display mode,
	 * and BH contains the active display page.
	 */

	LI = regs.h.dl + 1;
	CO = regs.h.ah;
#  endif
}
# endif
#endif /* MSDOS */

#ifdef TOS
#define BIGBUF  8192

int
_copyfile(from, to)
{
	int fromfd, tofd, r;
	char *buf;

	if ((fromfd = open(from, O_RDONLY|O_BINARY, 0)) < 0)
		return -1;
	if ((tofd = open(to, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, FCMASK)) < 0)
		return -1;
	if (!(buf = (char *)malloc((size_t)BIGBUF)))
		return -1;
	while ( (r = read(fromfd, buf, BIGBUF)) > 0)
		write(tofd, buf, r);
	close(fromfd);
	close(tofd);
	free(buf);
	return 0;	/* successful */
}

int kbhit()
{
	return Cconis();
}
#endif /* TOS */
