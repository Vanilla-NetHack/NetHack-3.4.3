/*	SCCS Id: @(#)msdos.c	3.0	89/12/26
/* NetHack may be freely redistributed.  See license for details. */
/* An assortment of MSDOS functions.
 */

#define NEED_VARARGS
#include "hack.h"

#ifdef MSDOS

# ifdef TOS
#  include <osbind.h>
#  ifndef WORD
#    define WORD short		/* 16 bits -- redefine if necessary */
#  endif
# else
#  ifdef __TURBOC__	/* avoid incompatible redeclaration */
#   undef getdate
#  endif
#  include <dos.h>
# endif
# ifdef OS2
#  include "def_os2.h"   /* OS2 definitions (Timo Hakulinen) */
# endif

#include <ctype.h>
#include <fcntl.h>
#include <process.h>

static char NDECL(DOSgetch);
# ifdef DGK
static char NDECL(BIOSgetch);
# endif
# ifdef TOS
static void NDECL(init_aline);
char *_a_line;			/* for Line A variables */
# else
static unsigned int FDECL(ioctl, (int,int,unsigned));
static boolean NDECL(comspec_exists);
# endif

static int FDECL(findfirst, (char *));
static int NDECL(findnext);
static boolean NDECL(record_exists);
# if !defined(TOS) && !defined(__TURBOC__) && !defined(OS2)
static char NDECL(switchar);
# endif
# ifndef OS2
static char * NDECL(getdta);
# endif

void
flushout()
{
	(void) fflush(stdout);
	return;
}

int
tgetch() {
	char ch;

# ifdef DGK
	/* BIOSgetch can use the numeric key pad on IBM compatibles. */
	if (flags.IBMBIOS)
		ch = BIOSgetch();
	else
# endif
		ch = DOSgetch();
	return ((ch == '\r') ? '\n' : ch);
}

# if !defined(OS2) && !defined(TOS)
/*
 * MS-DOS functions
 */
#define DIRECT_INPUT	0x07	/* Unfiltered Character Input Without Echo */
#define FATINFO 	0x1B	/* Get Default Drive Data */
/* MS-DOS 2.0+: */
#define GETDTA		0x2F	/* Get DTA Address */
#define FREESPACE	0x36	/* Get Drive Allocation Info */
#define GETSWITCHAR	0x3700	/* Get Switch Character */
#define FINDFIRST	0x4E	/* Find First File */
#define FINDNEXT	0x4F	/* Find Next File */
#define SETFILETIME	0x5701	/* Set File Date & Time */
/*
 * BIOS interrupts
 */
#define KEYBRD_BIOS	0x16
#define VIDEO_BIOS	0x10
/*
 * Keyboard BIOS functions
 */
#define READCHAR	0x00	/* Read Character from Keyboard */
#define GETKEYFLAGS	0x02	/* Get Keyboard Flags */
/*
 * Video BIOS functions
 */
#define SETCURPOS	0x02	/* Set Cursor Position */
#define GETMODE 	0x0f	/* Get Video Mode */
#define FONTINFO	0x1130	/* Get Font Info */
# endif


/*
 *  Keyboard translation tables.
 */
#  ifdef TOS
#define KEYPADLO	0x61
#define KEYPADHI	0x71
#  else
#define KEYPADLO	0x47
#define KEYPADHI	0x53
#  endif

#define PADKEYS 	(KEYPADHI - KEYPADLO + 1)
#define iskeypad(x)	(KEYPADLO <= (x) && (x) <= KEYPADHI)

/*
 * Keypad keys are translated to the normal values below.
 * When IBM_BIOS is active, shifted keypad keys are translated to the
 *    shift values below.
 */
static const struct pad {
	char normal, shift, cntrl;
} keypad[PADKEYS] = {
#  ifdef TOS
			{C('['), 'Q', C('[')},		/* UNDO */
			{'?', '/', '?'},		/* HELP */
			{'(', 'a', '('},		/* ( */
			{')', 'w', ')'},		/* ) */
			{'/', '/', '/'},		/* / */
			{C('p'), '$', C('p')},		/* * */
#  endif
			{'y', 'Y', C('y')},		/* 7 */
			{'k', 'K', C('k')},		/* 8 */
			{'u', 'U', C('u')},		/* 9 */
#  ifndef TOS
			{'m', C('p'), C('p')},		/* - */
#  endif
			{'h', 'H', C('h')},		/* 4 */
#  ifdef TOS
			{'.', '.', '.'},
#  else
			{'g', 'g', 'g'},		/* 5 */
#  endif
			{'l', 'L', C('l')},		/* 6 */
#  ifndef TOS
			{'p', 'P', C('p')},		/* + */
#  endif
			{'b', 'B', C('b')},		/* 1 */
			{'j', 'J', C('j')},		/* 2 */
			{'n', 'N', C('n')},		/* 3 */
			{'i', 'I', C('i')},		/* Ins */
			{'.', ':', ':'}			/* Del */
}, numpad[PADKEYS] = {
#  ifdef TOS
			{C('['), 'Q', C('[')}	,	/* UNDO */
			{'?', '/', '?'},		/* HELP */
			{'(', 'a', '('},		/* ( */
			{')', 'w', ')'},		/* ) */
			{'/', '/', '/'},		/* / */
			{C('p'), '$', C('p')},		/* * */
#  endif
			{'7', M('7'), '7'},		/* 7 */
			{'8', M('8'), '8'},		/* 8 */
			{'9', M('9'), '9'},		/* 9 */
#  ifndef TOS
			{'m', C('p'), C('p')},		/* - */
#  endif
			{'4', M('4'), '4'},		/* 4 */
#  ifdef TOS
			{'.', '.', '.'},		/* 5 */
#  else
			{'g', 'G', 'g'},		/* 5 */
#  endif
			{'6', M('6'), '6'},		/* 6 */
#  ifndef TOS
			{'p', 'P', C('p')},		/* + */
#  endif
			{'1', M('1'), '1'},		/* 1 */
			{'2', M('2'), '2'},		/* 2 */
			{'3', M('3'), '3'},		/* 3 */
			{'i', 'I', C('i')},		/* Ins */
			{'.', ':', ':'}			/* Del */
};

/*
 * Unlike Ctrl-letter, the Alt-letter keystrokes have no specific ASCII
 * meaning unless assigned one by a keyboard conversion table, so the
 * keyboard BIOS normally does not return a character code when Alt-letter
 * is pressed.	So, to interpret unassigned Alt-letters, we must use a
 * scan code table to translate the scan code into a letter, then set the
 * "meta" bit for it.  -3.
 */
#define SCANLO		0x10
#define SCANHI		0x32
#define SCANKEYS	(SCANHI - SCANLO + 1)
#define inmap(x)	(SCANLO <= (x) && (x) <= SCANHI)

static const char scanmap[SCANKEYS] = { 	/* ... */
	'q','w','e','r','t','y','u','i','o','p','[',']', '\n',
	0, 'a','s','d','f','g','h','j','k','l',';','\'', '`',
	0, '\\', 'z','x','c','v','b','N','m' 	/* ... */
};

# ifdef DGK
/*
 * BIOSgetch gets keys directly with a BIOS call.
 */
#define SHIFT		(0x1 | 0x2)
#define CTRL		0x4
#define ALT		0x8

static char
BIOSgetch() {
	unsigned char scan, shift, ch;
	const struct pad *kpad;

#  ifdef OS2
	KBDKEYINFO CharData;
	USHORT IOWait = 0;
	HKBD KbdHandle = 0;

	KbdCharIn(&CharData,IOWait,KbdHandle);
	ch = CharData.chChar;
	scan = CharData.chScan;
	shift = CharData.fsState;
#  else /* OS2 */
#   ifdef TOS
	long  x;
#   else
	union REGS regs;
#   endif

	/* Get scan code.
	 */
#   ifdef TOS
	x = Crawcin();
	ch = x & 0x0ff;
	scan = (x & 0x00ff0000L) >> 16;
#   else
	regs.h.ah = READCHAR;
	int86(KEYBRD_BIOS, &regs, &regs);
	ch = regs.h.al;
	scan = regs.h.ah;
#   endif
	/* Get shift status.
	 */
#   ifdef TOS
	shift = Kbshift(-1);
#   else
	regs.h.ah = GETKEYFLAGS;
	int86(KEYBRD_BIOS, &regs, &regs);
	shift = regs.h.al;
#   endif
#  endif /* OS2 */

	/* Translate keypad keys */
	if (iskeypad(scan)) {
		kpad = flags.num_pad ? numpad : keypad;
		if (shift & SHIFT)
			ch = kpad[scan - KEYPADLO].shift;
		else if (shift & CTRL)
			ch = kpad[scan - KEYPADLO].cntrl;
		else
			ch = kpad[scan - KEYPADLO].normal;
	}
	/* Translate unassigned Alt-letters */
	if ((shift & ALT) && !ch) {
		if (inmap(scan))
			ch = scanmap[scan - SCANLO];
		return (isprint(ch) ? M(ch) : ch);
	}
	return ch;
}

static char
DOSgetch() {
# ifdef TOS
	return (Crawcin() & 0x007f);
# else
#  ifdef OS2
	KBDKEYINFO CharData;
	USHORT IOWait = 0;
	HKBD KbdHandle = 0;

	KbdCharIn(&CharData,IOWait,KbdHandle);
	if (CharData.chChar == 0) {	/* an extended code -- not yet supported */
		KbdCharIn(&CharData,IOWait,KbdHandle);	   /* eat the next character */
		CharData.chChar = 0;		/* and return a 0 */
	}
	return (CharData.chChar);
#  else
	union REGS regs;
	char ch;
	struct pad (*kpad)[PADKEYS];

	regs.h.ah = DIRECT_INPUT;
	intdos(&regs, &regs);
	ch = regs.h.al;

	/*
	 * The extended codes for Alt-shifted letters, and unshifted keypad
	 * and function keys, correspond to the scan codes.  So we can still
	 * translate the unshifted cursor keys and Alt-letters.  -3.
	 */
	if (ch == 0) {		/* an extended key */
		regs.h.ah = DIRECT_INPUT;
		intdos(&regs, &regs);	/* get the extended key code */
		ch = regs.h.al;

		if (iskeypad(ch)) {	/* unshifted keypad keys */
			kpad = (void *)(flags.num_pad ? numpad : keypad);
			ch = (*kpad)[ch - KEYPADLO].normal;
		} else if (inmap(ch)) { /* Alt-letters */
			ch = scanmap[ch - SCANLO];
			if (isprint(ch)) ch = M(ch);
		} else ch = 0;		/* munch it */
	}
	return (ch);
#  endif /* OS2 */
# endif /* TOS */
}


#  ifndef TOS

#   ifdef __TURBOC__
#define switchar()	(char)getswitchar()
#   else
#    ifndef OS2
static char
switchar()
{
	union REGS regs;

	regs.x.ax = GETSWITCHAR;
	intdos(&regs, &regs);
	return regs.h.dl;
}
#    endif /* OS2 */
#   endif /* __TURBOC__ */
# endif  /* TOS */

static const char *COMSPEC = 
# ifdef TOS
"SHELL";
# else
"COMSPEC";
# endif

#define getcomspec() getenv(COMSPEC)

# ifdef SHELL
int
dosh() {
	extern char orgdir[];
	char *comspec;

	if (comspec = getcomspec()) {
#  if defined(DGK) && !defined(TOS)	/* TOS has a variety of shells */
		settty("To return to NetHack, enter \"exit\" at the DOS prompt.\n");
#  else
		settty((char *)0);
#  endif /* DGK */
		chdirx(orgdir, 0);
		if (spawnl(P_WAIT, comspec, comspec, NULL) < 0) {
			Printf("\nCan't spawn \"%s\"!\n", comspec);
			flags.toplin = 0;
			more();
		}
#ifdef TOS
/* Some shells (e.g. Gulam) turn the cursor off when they exit */
		if (flags.IBMBIOS) {
			(void)Cursconf(1, -1);
			get_scr_size(); /* maybe they changed the screen */
		}
#else
		gettty(); /* ctrl-P might get turned back on (TH) */
		get_scr_size(); /* maybe the screen mode changed (TH) */
#endif
		chdirx(hackdir, 0);
		start_screen();
		docrt();
	} else
#  ifdef OS2
		pline("Can't execute CMD.EXE");
#  else
#   ifdef TOS
		pline("Can't find SHELL.");
#   else
		pline("Can't find COMSPEC.");
#   endif
#  endif /* OS2 */
	return 0;
}
# endif /* SHELL */

#ifndef TOS

long
freediskspace(path)
char *path;
{
#   ifdef OS2
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
#   else /* OS2 */
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
#   endif /* OS2 */
}

#   ifdef OS2
FILEFINDBUF ResultBuf;
HDIR DirHandle;
#   endif

/* Functions to get filenames using wildcards
 */
static int
findfirst(path)
char *path;
{
#   ifdef OS2
	USHORT res, SearchCount = 1;

	DirHandle = 1;
	res = DosFindFirst((PSZ)path,&DirHandle,0,&ResultBuf,sizeof(FILEFINDBUF),&SearchCount,0L);
	return(!res);
#   else
	union REGS regs;
	struct SREGS sregs;

	regs.h.ah = FINDFIRST;
	regs.x.cx = 0;		/* attribute: normal files */
	regs.x.dx = FP_OFF(path);
	sregs.ds = FP_SEG(path);
	intdosx(&regs, &regs, &sregs);
	return !regs.x.cflag;
#   endif /* OS2 */
}

static int
findnext() {
#   ifdef OS2
	USHORT res, SearchCount = 1;

	res = DosFindNext(DirHandle,&ResultBuf,sizeof(FILEFINDBUF),&SearchCount);
	return(!res);
#   else
	union REGS regs;

	regs.h.ah = FINDNEXT;
	intdos(&regs, &regs);
	return !regs.x.cflag;
#   endif /* OS2 */
}

#   ifndef OS2
/* Get disk transfer area, Turbo C already has getdta */
static char *
getdta() {
	union REGS regs;
	struct SREGS sregs;
	char *ret;

	regs.h.ah = GETDTA;
	intdosx(&regs, &regs, &sregs);
#    ifdef MK_FP
	ret = MK_FP(sregs.es, regs.x.bx);
#    else
	FP_OFF(ret) = regs.x.bx;
	FP_SEG(ret) = sregs.es;
#    endif
	return ret;
}
#   endif /* OS2 */

#  else /* TOS */

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

#  endif /* TOS */

long
filesize(file)
char *file;
{
#  ifndef OS2
	char *dta;
#  endif

	if (findfirst(file)) {
#  ifdef OS2
		return  (* (long *) (ResultBuf.cbFileAlloc));
#  else
		dta = getdta();
		return  (* (long *) (dta + 26));
#  endif
	} else
		return -1L;
}

void
eraseall(path, files)
const char *path, *files;
{
#  ifndef OS2
	char *dta;
#  endif
	char buf[PATHLEN];

#  ifndef OS2
	dta = getdta();
#  endif
	Sprintf(buf, "%s%s", path, files);
	if (findfirst(buf))
		do {
			Sprintf(buf, "%s%s", path,
#  ifdef OS2
				ResultBuf.achName
#  else
				dta + 30
#  endif
				);
			(void) unlink(buf);
		} while (findnext());
	return;
}

/* Rewritten for version 3.3 to be faster
 */
void
copybones(mode)
int mode;
{
	char from[PATHLEN], to[PATHLEN], last[13];
	char *frompath, *topath;
#  ifndef OS2
	char *dta;
#  endif
#  ifndef TOS
	int status;
	char copy[8], *comspec;
	extern saveprompt;
#  endif

	if (!ramdisk)
		return;

	/* Find the name of the last file to be transferred
	 */
	frompath = (mode != TOPERM) ? permbones : levels;
#  ifndef OS2
	dta = getdta();
#  endif
	last[0] = '\0';
	Sprintf(from, "%s%s", frompath, allbones);
	topath = (mode == TOPERM) ? permbones : levels;
#  ifdef TOS
	eraseall(topath, allbones);
#  endif
	if (findfirst(from))
		do {
#  ifdef TOS
			Sprintf(from, "%s%s", frompath, dta+30); 
			Sprintf(to, "%s%s", topath, dta+30);
			if (_copyfile(from, to))
				goto error_copying;
#  endif
			Strcpy(last,
#  ifdef OS2
				ResultBuf.achName
#  else
				dta + 30
#  endif
				);
		} while (findnext());
#  ifdef TOS
	else
		return;
#  else
	if (last[0]) {
		Sprintf(copy, "%cC copy",
#   ifdef OS2
			'/'
#   else
			switchar()
#   endif
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

#if 0 /* defined(MSDOS) && !defined(TOS) && !defined(OS2) */
boolean
removeable_drive(drive)
char drive;
/* check whether current drive is a fixed disk,
   so we don't ask the player to insert one */
{
	union REGS regs;
	char *fat_id;

	regs.h.ah = FATINFO;
	intdos(&regs, &regs);
	/* also returns size info, as
	   AL (sectors/cluster) * CX (bytes/sector) * DX (clusters/disk) */
#   ifdef MK_FP
	fat_id = MK_FP(sregs.ds, regs.x.bx);
#   else
	FP_OFF(fat_id) = regs.x.bx;
	FP_SEG(fat_id) = sregs.ds;
#   endif
	return (*fat_id != 0xF8);
}
#endif /* 0 */

void
playwoRAMdisk() {
	msmsg("Do you wish to play without a RAMdisk? ");

	/* Set ramdisk false *before* exit-ing (because msexit calls
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
saveDiskPrompt(start)
int start;
{
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
		msmsg("If save file is on a save disk, insert that disk now.\n");
		cl_end();
		msmsg("File name (default \"%s\"%s) ? ", SAVEF,
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
#  ifdef OS2_CODEVIEW
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,RECORD);
	if ((fd = open(tmp, 0)) >= 0) {
#  else
	if ((fd = open(RECORD, 0)) >= 0) {
#  endif
		(void) close(fd);
		return TRUE;
	}
	return FALSE;
}

#  ifdef TOS
#define comspec_exists() 1
#  else
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
#  endif

/* Prompt for game disk, then check for record file.
 */
void
gameDiskPrompt() {
	extern int saveprompt;

	if (saveprompt) {
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

# endif /* DGK */

/* Read configuration */
void
read_config_file() {
# ifdef DGK
	char	tmp_ramdisk[PATHLEN];
	extern	int saveprompt;
# else
#define fopenp fopen
# endif
	char	tmp_levels[PATHLEN];
	char	buf[BUFSZ], *bufp;
	FILE	*fp;
	extern	char plname[];

# ifdef DGK
	tmp_ramdisk[0] = 0;
# endif
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
			msmsg("Bad option line: \"%s\"\n", buf);
			getreturn("to continue");
			continue;
		}
		
		/* skip  whitespace between '=' and value */
		while (isspace(*++bufp))
			;

		/* Go through possible variables */
		if (!strncmp(buf, "HACKDIR", 4)) {
			strncpy(hackdir, bufp, PATHLEN);
		
# ifdef DGK
		} else if (!strncmp(buf, "RAMDISK", 3)) {
			strncpy(tmp_ramdisk, bufp, PATHLEN);
# endif

		} else if (!strncmp(buf, "LEVELS", 4)) {
			strncpy(tmp_levels, bufp, PATHLEN);

		} else if (!strncmp(buf, "OPTIONS", 4)) {
			parseoptions(bufp, TRUE);
			if (plname[0])		/* If a name was given */
				plnamesuffix();	/* set the character class */

		} else if (!strncmp(buf, "SAVE", 4)) {
# ifdef DGK
			char *ptr;
			if (ptr = index(bufp, ';')) {
				*ptr = '\0';
				if (*(ptr+1) == 'n' || *(ptr+1) == 'N')
					saveprompt = FALSE;
			}
# endif /* DGK */
			(void) strncpy(SAVEF, bufp, PATHLEN);
			(void) strncpy(SAVEP, bufp, PATHLEN);
			append_slash(SAVEF);
			append_slash(SAVEP);
		} else if (!strncmp(buf, "GRAPHICS", 4)) {
			unsigned int translate[MAXPCHARS+1];
			int lth;
#ifdef OVERLAY
			/* THIS is what I call a stupid hack, but MSC cannot survive
			   the overlays without it (TH) */
			lth = sscanf(bufp,
	"%d%d%d%d%d%d%d%d%d%d%d%d",
				&translate[0], &translate[1], &translate[2],
				&translate[3], &translate[4], &translate[5],
				&translate[6], &translate[7], &translate[8],
				&translate[9], &translate[10], &translate[11]);
			lth += sscanf(bufp,
	"%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%d%d%d%d%d%d%d%d%d%d%d%d",
				&translate[12], &translate[13], &translate[14],
				&translate[15], &translate[16], &translate[17],
				&translate[18], &translate[19], &translate[20],
				&translate[21], &translate[22], &translate[23]);
			lth += sscanf(bufp,
	"%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%*d%d%d%d%d%d%d%d%d%d%d%d",
				&translate[24], &translate[25], &translate[26],
				&translate[27], &translate[28], &translate[29],
				&translate[30], &translate[31], &translate[32],
				&translate[33], &translate[34]);
#else
		     lth = sscanf(bufp,
	"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
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
				&translate[30], &translate[31], &translate[32],
				&translate[33], &translate[34]);
#endif /* OVERLAY */
			if (lth <= 0) {
				msmsg ("Syntax error in GRAPHICS\n");
				getreturn("to continue");
			}
			assign_graphics(translate, lth);
		} else {
			msmsg("Bad option line: \"%s\"\n", buf);
			getreturn("to continue");
		}
	}
	(void) fclose(fp);

# ifdef DGK
	Strcpy(permbones, tmp_levels);
	if (tmp_ramdisk[0]) {
		Strcpy(levels, tmp_ramdisk);
		if (strcmp(permbones, levels))		/* if not identical */
			ramdisk = TRUE;
	} else
# endif /* DGK */
		Strcpy(levels, tmp_levels);
	Strcpy(bones, levels);
	return;
}

# ifdef DGK
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
# endif /* DGK */

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
const char *str;
{
# ifdef TOS
	msmsg("Hit <Return> %s.", str);
# else
	msmsg("Hit <Enter> %s.", str);
# endif
	while (Getchar() != '\n') ;
	return;
}

void
msmsg VA_DECL(const char *, fmt)
	VA_START(fmt);
	VA_INIT(fmt, const char *);
	Vprintf(fmt, VA_ARGS);
	flushout();
	VA_END();
	return;
}

/* Chdrive() changes the default drive.
 */
# ifndef __TURBOC__
#  ifndef OS2
#define SELECTDISK	0x0E
#  endif
void
chdrive(str)
char *str;
{
	char *ptr;
#  ifndef TOS
#   ifndef OS2
	union REGS inregs;
#   endif
#  endif
	char drive;

	if ((ptr = index(str, ':')) != NULL) {
		drive = toupper(*(ptr - 1));
#  ifdef TOS
		(void)Dsetdrv(drive - 'A');
#  else
#   ifdef OS2
		DosSelectDisk((USHORT)(drive - 'A' + 1));
#   else
		inregs.h.ah = SELECTDISK;
		inregs.h.dl = drive - 'A';
		intdos(&inregs, &inregs);
#   endif
#  endif
	}
	return;
}
# else
extern int setdisk(int);

void
chdrive(str)
char *str;
{
	if (str[1] == ':')
		(void)setdisk((int)(toupper(str[0]) - 'A'));
	return;
}
# endif

# ifndef TOS
/* Use the IOCTL DOS function call to change stdin and stdout to raw
 * mode.  For stdin, this prevents MSDOS from trapping ^P, thus
 * freeing us of ^P toggling 'echo to printer'.
 * Thanks to Mark Zbikowski (markz@microsoft.UUCP).
 */

#  ifndef OS2
#define DEVICE		0x80
#define RAW		0x20
#define IOCTL		0x44
#define STDIN		fileno(stdin)
#define STDOUT		fileno(stdout)
#define GETBITS		0
#define SETBITS		1
#  endif

static unsigned	int old_stdin, old_stdout;

void
disable_ctrlP() {
#  ifdef OS2
	KBDINFO KbdInfo;
	HKBD KbdHandle = 0;
#  endif

#  ifdef DGK
	if (!flags.rawio) return;
#  endif
#  ifdef OS2
	KbdInfo.cb = sizeof(KbdInfo);
	KbdGetStatus(&KbdInfo,KbdHandle);
	KbdInfo.fsMask &= 0xFFF7; /* ASCII off */
	KbdInfo.fsMask |= 0x0004; /* BINARY on */
	KbdSetStatus(&KbdInfo,KbdHandle);
#  else
	old_stdin = ioctl(STDIN, GETBITS, 0);
	old_stdout = ioctl(STDOUT, GETBITS, 0);
	if (old_stdin & DEVICE)
		ioctl(STDIN, SETBITS, old_stdin | RAW);
	if (old_stdout & DEVICE)
		ioctl(STDOUT, SETBITS, old_stdout | RAW);
#  endif /* OS2 */
	return;
}

void
enable_ctrlP() {
#  ifdef OS2
	KBDINFO KbdInfo;
	HKBD KbdHandle = 0;
#  endif

#  ifdef DGK
	if (!flags.rawio) return;
#  endif
#  ifdef OS2
	KbdInfo.cb = sizeof(KbdInfo);
	KbdGetStatus(&KbdInfo,KbdHandle);
	KbdInfo.fsMask &= 0xFFFB; /* BINARY off */
	KbdInfo.fsMask |= 0x0008; /* ASCII on */
	KbdSetStatus(&KbdInfo,KbdHandle);
#  else
	if (old_stdin)
		(void) ioctl(STDIN, SETBITS, old_stdin);
	if (old_stdout)
		(void) ioctl(STDOUT, SETBITS, old_stdout);
#  endif
	return;
}

#  ifndef OS2
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
#  endif /* OS2 */
# endif /* TOS */

# ifdef DGK
/* Follow the PATH, trying to fopen the file.
 */
#  ifdef TOS
#define PATHSEP	','
#  else
#define PATHSEP	';'
#  endif

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
#  ifdef OS2_CODEVIEW /* one more try for hackdir */
	Strcpy(buf,hackdir);
	append_slash(buf);
	Strcat(buf,name);
	if(fp = fopen(buf,mode))
		return fp;
#  endif
	return (FILE *)0;
}
# endif /* DGK */

/* Chdir back to original directory
 */
#undef exit
# ifdef TOS
extern boolean run_from_desktop;	/* set in pcmain.c */
# endif

void exit(int);
void
msexit(code)
int code;
{
# ifdef CHDIR
	extern char orgdir[];
# endif

	flushout();
# ifndef TOS
	enable_ctrlP();		/* in case this wasn't done */
# endif
# ifdef DGK
	if (ramdisk) copybones(TOPERM);
# endif
# ifdef CHDIR
	chdir(orgdir);		/* chdir, not chdirx */
	chdrive(orgdir);
# endif
# ifdef TOS
	if (run_from_desktop)
	    getreturn("to continue"); /* so the user can read the score list */
#  ifdef TEXTCOLOR
	if (flags.IBMBIOS && flags.use_color)
		restore_colors();
#  endif
# endif
	exit(code);
	return;
}

#  ifdef DGK		/* for flags.IBMBIOS */
void
get_scr_size()
{
#   ifdef OS2
	VIOMODEINFO ModeInfo;
	HVIO VideoHandle = 0;

	ModeInfo.cb = sizeof(ModeInfo);

	(void) VioGetMode(&ModeInfo,VideoHandle);

	CO = ModeInfo.col;
	LI = ModeInfo.row;
#   else
#    ifndef TOS
	union REGS regs;

	if (!flags.IBMBIOS) {		/* assume standard screen size */
		CO = 80;
		LI = 24;
		return;
	}

	regs.x.ax = FONTINFO;
	regs.x.bx = 0;			/* current ROM BIOS font */
	regs.h.dl = 24;			/* default row count */
					/* in case no EGA/MCGA/VGA */
	int86(VIDEO_BIOS, &regs, &regs); /* Get Font Information */

	/* MDA/CGA/PCjr ignore INT 10h, Function 11h, but since we
	 * cleverly loaded up DL with the default, everything's fine.
	 *
	 * Otherwise, DL now contains rows - 1.  Also, CX contains the
	 * points (bytes per character) and ES:BP points to the font
	 * table.  -3.
	 */

	regs.h.ah = GETMODE;
	int86(VIDEO_BIOS, &regs, &regs); /* Get Video Mode */

	/* This goes back all the way to the original PC.  Completely
	 * safe.  AH contains # of columns, AL contains display mode,
	 * and BH contains the active display page.
	 */

	LI = regs.h.dl + 1;
	CO = regs.h.ah;
#    else  /* TOS */
	init_aline();
	LI = (*((WORD  *)(_a_line + -42L))) + 1;
	CO = (*((WORD  *)(_a_line + -44L))) + 1;
#    endif /* TOS */
#   endif /* OS2 */
}

#   ifndef TOS
void
gotoxy(x,y)
int x,y;
{
#    ifdef OS2
	HVIO VideoHandle = 0;

	x--; y--;			/* (0,0) is upper right corner */

	(void) VioSetCurPos(x, y, VideoHandle);
#    else
	union REGS regs;

	x--; y--;			/* (0,0) is upper right corner */

	regs.h.ah = SETCURPOS;
	regs.h.bh = 0;			/* display page */
	regs.h.dh = y;			/* row */
	regs.h.dl = x;			/* column */
	int86(VIDEO_BIOS, &regs, &regs); /* Set Cursor Position */

	/* This, too, goes back all the way to the original PC.  If
	 * we ever get so fancy as to swap display pages (i doubt it),
	 * then we'll need to set BH appropriately.  This function
	 * returns nothing.  -3.
	 */
#    endif /* OS2 */
}
#   endif /* TOS */
#  endif /* DGK */

#endif /* MSDOS */


#ifdef TOS
# define BIGBUF  8192

int
_copyfile(from, to)
char *from, *to;
{
	int fromfd, tofd, r;
	char *buf;
	extern genericptr_t FDECL(malloc, (size_t));

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

static void
init_aline()
{
# ifdef __GNUC__
/* line A calls nuke registers d0-d2,a0-a2; not all compilers regard these
   as scratch registers, though, so we save them
 */
	asm(" moveml d0-d2/a0-a2, sp@-");
	asm(" .word 0xa000; movel d0, __a_line");
	asm(" moveml sp@+, d0-d2/a0-a2");
# else
	asm(" movem.l d0-d2/a0-a2, -(sp)");
	asm(" .dc.w 0xa000");	/* tweak as necessary for your compiler */
	asm(" move.l d0, __a_line");
	asm(" movem.l (sp)+, d0-d2/a0-a2");
# endif
}

# ifdef TEXTCOLOR
static unsigned orig_color[4] = {-1, -1, -1, -1};
static unsigned new_color[4] = { 0x0, 0x730, 0x047, 0x555 };
static int numcolors = 2;

void set_colors()
{
	int i;
	char *s;
	static char newhe[] = "\033q\033b\017\033c0";

	if (!flags.IBMBIOS)
		return;
	init_aline();
	numcolors = 1 << (((unsigned char *) _a_line)[1]);
	if (numcolors == 2) {			/* mono */
		flags.use_color = FALSE;
		return;
	}
	else if (numcolors == 4) {
		for (i = 0; i < 4; i++)
			orig_color[i] = Setcolor(i, new_color[i]);
	}
	else {
		orig_color[0] = Setcolor(0, new_color[0]);
		orig_color[1] = Setcolor(15, 0x777);
		hilites[0] = "";
		for (i = 1; i < 16; i++) {
			s = (char *) alloc(sizeof("\033b0"));
			sprintf(s, "\033b%c", '0'+i);
			hilites[i] = s;
		}
		HE = newhe;
	}
}

void restore_colors()
{
	int i;

	if (numcolors == 2)
		return;
	else if (numcolors == 4)
		for (i = 0; i < 4; i++)
			(void) Setcolor(i, orig_color[i]);
	else {
		(void) Setcolor(0, orig_color[0]);
		(void) Setcolor(15, orig_color[1]);
	}
}
# endif /* TEXTCOLOR */
#endif /* TOS */
