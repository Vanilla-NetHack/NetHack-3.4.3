/*    SCCS Id: @(#)amidos.c msdos.c - Amiga version   3.0    89/05/01
/* NetHack may be freely redistributed.  See license for details. */
/* An assortment of imitations of cheap plastic MSDOS functions.
 */

#include <libraries/dos.h>

#undef TRUE
#undef FALSE
#undef COUNT
#undef NULL

#define NEED_VARARGS
#include "hack.h"

extern char Initialized;

struct FileLock *Lock(), *CurrentDir(); /* Cheating - BCPL pointers */
struct FileHandle *Open();              /* Cheating - BCPL pointer */
long Read(), Write(), IoErr(), AvailMem();
void *malloc();
char *rindex(), *index();

int Enable_Abort = 0;	/* for stdio package */

/* Initial path, so we can find NetHack.cnf */

char PATH[PATHLEN] = "Ram:;df0:;NetHack:";

void
flushout()
{
    (void) fflush(stdout);
}

#ifndef getuid
getuid()
{
    return 1;
}
#endif

/*
 *  Actually make up a process id.
 *  Makes sure one can mess less with saved levels...
 */

int getpid()
{
    static short pid;

    while (pid == 0) {
	struct DateStamp dateStamp;
	pid = rnd(30000);
	pid += DateStamp(&dateStamp);    /* More or less random */
	pid ^= (short) (dateStamp.ds_Days >> 16) ^
	       (short) (dateStamp.ds_Days)       ^
	       (short) (dateStamp.ds_Minute)     +
	       (short) (dateStamp.ds_Tick);
	pid %= 30000;
    }

    return pid;
}

#ifndef getlogin
char *
getlogin()
{
    return ((char *) NULL);
}
#endif

int
abs(x)
int x;
{
    return x < 0? -x: x;
}

int
tgetch() {
    char ch;

    ch = WindowGetchar();
    return ((ch == '\r') ? '\n' : ch);
}

#ifdef DGK
# include <ctype.h>
/* # include <fcntl.h> */

# define Sprintf (void) sprintf

# ifdef SHELL
int
dosh()
{
    pline("No mysterious force prevented you from using multitasking.");
    return 0;
}
# endif /* SHELL */

#define ID_DOS1_DISK	'DOS\1'
#define EXTENSION	72

/*
 *  This routine uses an approximation of the free bytes on a disk.
 *  How large a file you can actually write depends on the number of
 *  extension blocks you need for it.
 *  In each extenstion block there are maximum 72 pointers to blocks,
 *  so every 73 disk blocks have only 72 available for data.
 *  The (necessary) file header is also good for 72 data block pointers.
 */
long
freediskspace(path)
char *path;
{
    register long freeBytes = 0;
    register struct InfoData *infoData; /* Remember... longword aligned */
    char fileName[32];

    /*
     *	Find a valid path on the device of which we want the free space.
     *	If there is a colon in the name, it is an absolute path
     *	and all up to the colon is everything we need.
     *	Remember slashes in a volume name are allowed!
     *	If there is no colon, it is relative to the current directory,
     *	so must be on the current device, so "" is enough...
     */
    {
	register char *colon;

	strncpy(fileName, path, sizeof(fileName)-1);
	fileName[31] = 0;
	if (colon = index(fileName, ':'))
	    colon[1] = '\0';
	else
	    fileName[0] = '\0';
    }

    if (infoData = malloc(sizeof(*infoData))) {
	struct FileLock *fileLock;  /* Cheating */
	if (fileLock = Lock(fileName, SHARED_LOCK)) {
	    if (Info(fileLock, infoData)) {
		/* We got a kind of DOS volume, since we can Lock it. */
		/* Calculate number of blocks available for new file */
		/* Kludge for the ever-full VOID: (oops RAM:) device */
		if (infoData->id_UnitNumber == -1 &&
		    infoData->id_NumBlocks == infoData->id_NumBlocksUsed) {
		    freeBytes = AvailMem(0L) - 64 * 1024L;
		    /* Just a stupid guess at the */
		    /* Ram-Handler overhead per block: */
		    freeBytes -= freeBytes/16;
		} else {
		    /* Normal kind of DOS file system device/volume */
		    freeBytes = infoData->id_NumBlocks -
				infoData->id_NumBlocksUsed;
		    freeBytes -= (freeBytes + EXTENSION) / (EXTENSION + 1);
		    freeBytes *= infoData->id_BytesPerBlock;
		}
		if (freeBytes < 0)
		    freeBytes = 0;
	    }
	    UnLock(fileLock);
	}
	free(infoData);
    }
    return freeBytes;
}


long
filesize(file)
char *file;
{
    register struct FileLock *fileLock;
    register struct FileInfoBlock *fileInfoBlock;
    register long size = 0;

    if (fileInfoBlock = malloc(sizeof(*fileInfoBlock))) {
	if (fileLock = Lock(file, SHARED_LOCK)) {
	    if (Examine(fileLock, fileInfoBlock)) {
		size = fileInfoBlock->fib_Size;
	    }
	    UnLock(fileLock);
	}
	free(fileInfoBlock);
    }
    return size;
}

/*
 *  On the Amiga, looking if a specific file exists is much faster
 *  than sequentially reading a directory.
 */

void
eraseall(path, files)
char *path, *files;
{
    char buf[FILENAME];
    short i;
    struct FileLock *fileLock, *dirLock;

    if (dirLock = Lock(path)) {
	dirLock = CurrentDir(dirLock);

	strcpy(buf, files);
	for (i = 0; i <= MAXLEVEL; i++) {
	    name_file(buf, i);
	    if (fileLock = Lock(buf, SHARED_LOCK)) {
		UnLock(fileLock);
		DeleteFile(buf);
	    } else if (IoErr() == ERROR_DEVICE_NOT_MOUNTED)
		break;
	    }

	UnLock(CurrentDir(dirLock));
    }
}

/* This size makes that most files can be copied with two Read()/Write()s */

#define COPYSIZE    4096

char *CopyFile(from, to)
char *from, *to;
{
    register struct FileHandle *fromFile, *toFile;
    register char *buffer;
    register long size;
    char *error = NULL;

    if (buffer = malloc(COPYSIZE)) {
	if (fromFile = Open(from, MODE_OLDFILE)) {
	    if (toFile = Open(to, MODE_NEWFILE)) {
		while (size = Read(fromFile, buffer, (long)COPYSIZE)) {
		    if (size != Write(toFile, buffer, size)) {
			error = "Write error";
			break;
		    }
		}
		Close(toFile);
	    } else /* Can't open destination file */
		error = "Cannot open destination";
	    Close(fromFile);
	} else /* Cannot open source file. Should not happen. */
	    error = "Huh?? Cannot open source??";
	free(buffer);
	return error;
    } else /* Cannot obtain buffer for copying */
	return "No Memory !";
}

void
copybones(mode)
int mode;
{
    struct FileLock *fileLock;
    char from[FILENAME], to[FILENAME];
    char *frompath, *topath, *status;
    short i;
    extern int saveprompt;

    if (!ramdisk)
	return;

    frompath = (mode != TOPERM) ? permbones : levels;
    topath = (mode == TOPERM) ? permbones : levels;

    /* Remove any bones files in `to' directory. */
    eraseall(topath, allbones);

    /* Copy `from' to `to' */
    strcpy(from, frompath);
    strcat(from, allbones);
    strcpy(to, topath);
    strcat(to, allbones);

    for (i = 1; i < MAXLEVEL; i++) {
	name_file(from, i);
	name_file(to, i);
	if (fileLock = Lock(from, SHARED_LOCK)) {
	    UnLock(fileLock);
	    if (status = CopyFile(from, to))
		goto failed;
	} else if (IoErr() == ERROR_DEVICE_NOT_MOUNTED) {
	    status = "disk not present";
	    goto failed;
	}
    }

    /*
     * The last file got there.  Remove the ramdisk bones files.
     */
    if (mode == TOPERM)
	eraseall(frompath, allbones);
    return;

    /* Last file didn't get there. */

failed:
    msmsg("Cannot copy `%s' to `%s'\n(%s)\n", from, to, status);

    if (mode == TOPERM) {
	msmsg("Bones will be left in `%s'\n",
	    *frompath ? frompath : hackdir);
	return;
    } else {
	/* Remove all bones files on the RAMdisk */
	eraseall(levels, allbones);
	playwoRAMdisk();
    }
}

void
playwoRAMdisk()
{
    msmsg("Do you wish to play without a RAMdisk (y/n) ? ");

    /* Set ramdisk false *before* exit'ing (because msexit calls
     * copybones)
     */
    ramdisk = FALSE;
    if (Getchar() != 'y') {
	settty("Be seeing you ...\n");
	exit(0);
    }
    set_lock_and_bones();
    return;
}

int
saveDiskPrompt(start)
{
    extern int saveprompt;
    char buf[BUFSIZ], *bp;
    struct FileLock *fileLock;

    if (saveprompt) {
	/* Don't prompt if you can find the save file */
	if (fileLock = Lock(SAVEF, SHARED_LOCK)) {
	    UnLock(fileLock);
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
record_exists()
{
    FILE *file;

    if (file = fopenp(RECORD, "r")) {
	fclose(file);
	return TRUE;
    }
    return FALSE;
}

/* Prompt for game disk, then check for record file.
 */
void
gameDiskPrompt()
{
    extern int saveprompt;

    if (record_exists())
	return;

    if (saveprompt) {
	(void) putchar('\n');
	getreturn("when the GAME disk has been put in");
    }

    if (!record_exists()) {
	msmsg("\n\nWARNING: can't find record file `%s'!\n", RECORD);

	msmsg("If the GAME disk is not in, put it in now.\n");
	getreturn("to continue");
    }
}

/* Read configuration */
void
read_config_file()
{
    char    tmp_ramdisk[PATHLEN], tmp_levels[PATHLEN];
    char    buf[BUFSZ], *bufp;
    FILE    *fp, *fopenp();
    extern  char plname[];
    extern  int saveprompt;

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

	/* remove trailing whitespace */

	bufp = index(buf, '\n');
	while (bufp > buf && isspace(*bufp))
	    bufp--;
	if (bufp == buf)
	    continue;	     /* skip all-blank lines */
	else
	    *(bufp + 1) = 0;    /* 0 terminate line */

	/* find the '=' */
	if (!(bufp = index(buf, '='))) {
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
	    parseoptions(bufp, (boolean)TRUE);
	    if (plname[0])        /* If a name was given */
		plnamesuffix();    /* set the character class */

	} else if (!strncmp(buf, "SAVE", 4)) {
	    char *ptr;
	    if (ptr = index(bufp, ';')) {
		*ptr = '\0';
		if (*(ptr+1) == 'n' || *(ptr+1) == 'N')
		    saveprompt = FALSE;
	    }
	    (void) strncpy(SAVEF, bufp, PATHLEN);
	    append_slash(SAVEF);
	} else if (!strncmp(buf, "GRAPHICS", 4)) {
	    unsigned int translate[MAXPCHARS+1]; /* for safety */
	    int  lth;

	    if ((lth = sscanf(bufp,
	     "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
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
	    } /* Yuck! Worked only with low-byte first!!! */
	    assign_graphics(translate, lth);
	} else if (!strncmp(buf, "PATH", 4)) {
	    strncpy(PATH, bufp, PATHLEN);

	} else {
	    msmsg("Bad option line: '%s'\n", buf);
	    getreturn("to continue");
	}
    }
    fclose(fp);

    strcpy(permbones, tmp_levels);
    if (tmp_ramdisk[0]) {
	strcpy(levels, tmp_ramdisk);
	if (strcmpi(permbones, levels))        /* if not identical */
	    ramdisk = TRUE;
    } else
	strcpy(levels, tmp_levels);
    strcpy(bones, levels);
}

/* Set names for bones[] and lock[] */

void
set_lock_and_bones()
{
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

/*
 * Add a slash to any name not ending in / or :.  There must
 * be room for the /.
 */
void
append_slash(name)
char *name;
{
    char *ptr;

    if (!*name)
	return;
    ptr = name + (strlen(name) - 1);
    if (*ptr != '/' && *ptr != ':') {
	*++ptr = '/';
	*++ptr = '\0';
    }
}


void
getreturn(str)
char *str;
{
    int ch;

    msmsg("Hit <RETURN> %s.", str);
    while ((ch = Getchar()) != '\n')
	;
}

void
msmsg VA_DECL(char *, fmt)
    VA_START(fmt);
    VA_INIT(fmt, char *);
    vprintf(fmt, VA_ARGS);
    (void) fflush(stdout);
    VA_END();
}

/* Follow the PATH, trying to fopen the file.
 */
#define PATHSEP    ';'
#undef fopen

FILE *
fopenp(name, mode)
register char *name, *mode;
{
    char buf[BUFSIZ], *bp, *pp, lastch;
    FILE *fp;
    register struct FileLock *theLock;

    /* Try the default directory first.  Then look along PATH.
     */
    strcpy(buf, name);
    if (theLock = Lock(buf, SHARED_LOCK)) {
	UnLock(theLock);
	if (fp = fopen(buf, mode))
	    return fp;
    }
    pp = PATH;
    while (pp && *pp) {
	bp = buf;
	while (*pp && *pp != PATHSEP)
	    lastch = *bp++ = *pp++;
	if (lastch != ':' && lastch != '/' && bp != buf)
	    *bp++ = '/';
	strcpy(bp, name);
	if (theLock = Lock(buf, SHARED_LOCK)) {
	    UnLock(theLock);
	    if (fp = fopen(buf, mode))
		return fp;
	}
	if (*pp)
	    pp++;
    }
    return NULL;
}
#endif /* DGK */

#ifdef CHDIR

/*
 *  A not general-purpose directory changing routine.
 *  Assumes you want to return to the original directory eventually,
 *  by chdir()ing to orgdir.
 *  Assumes -1 is not a valid lock, since 0 is valid.
 */

#define NO_LOCK     ((struct FileLock *) -1)

char orgdir[1];
static struct FileLock *OrgDirLock = NO_LOCK;

chdir(dir)
char *dir;
{
    if (dir == orgdir) {
	/* We want to go back to where we came from. */
	if (OrgDirLock != NO_LOCK) {
	    UnLock(CurrentDir(OrgDirLock));
	    OrgDirLock = NO_LOCK;
	}
    } else {
	/*
	 * Go to some new place. If still at the original
	 * directory, save the FileLock.
	 */
	struct FileLock *newDir;

	if (newDir = Lock(dir, SHARED_LOCK)) {
	    if (OrgDirLock == NO_LOCK) {
		OrgDirLock = CurrentDir(newDir);
	    } else {
		UnLock(CurrentDir(newDir));
	    }
	} else {
	    return -1;	/* Failed */
	}
    }
    /* CurrentDir always succeeds if you have a lock */
    return 0;
}

#endif

/* Chdir back to original directory
 */
#undef exit
void
msexit(code)
{
#ifdef CHDIR
    extern char orgdir[];
#endif

#ifdef DGK
    (void) fflush(stdout);
    if (ramdisk)
	copybones(TOPERM);
#endif
#ifdef CHDIR
    chdir(orgdir);      /* chdir, not chdirx */
#endif

    CleanUp();
    exit(code);
}

/*
 *  Strcmp while ignoring case. Not general-purpose, so static.
 */

static int strcmpi(a, b)
register char *a, *b;
{
    while (tolower(*a) == tolower(*b)) {
	if (!*a)        /* *a == *b, so at end of both strings */
	    return 0;	/* equal. */
	a++;
	b++;
    }
    return 1;
}

/*
 *  memcmp - used to compare two struct symbols, in lev.c
 */

#ifndef memcmp
memcmp(a, b, size)
register unsigned char *a, *b;
register int size;
{
    while (size--) {
	if (*a++ != *b++)
	    return 1;	/* not equal */
    }

    return 0;		/* equal */
}
#endif

#ifndef memcpy
char *
memcpy(dest, source, size)
register char *dest;
char *source;
int size;
{
    movmem(source, dest, size);
    return dest;
}
#endif
