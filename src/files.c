/*	SCCS Id: @(#)files.c	3.1	93/02/20	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include <ctype.h>

#if !defined(MAC) && !defined(O_WRONLY) && !defined(AZTEC_C)
#include <fcntl.h>
#endif
#if defined(UNIX) || defined(VMS)
#include <errno.h>
# ifndef SKIP_ERRNO
extern int errno;
# endif
#include <signal.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(TOS) || defined(WIN32)
# ifndef GNUDOS
#include <sys\stat.h>
# else
#include <sys/stat.h>
# endif
#endif
#ifndef O_BINARY	/* used for micros, no-op for others */
# define O_BINARY 0
#endif

#ifdef MFLOPPY
char bones[FILENAME];		/* pathname of bones files */
char lock[FILENAME];		/* pathname of level files */
#else
static char bones[] = "bonesnn.xxxx";
# ifdef VMS
char lock[PL_NSIZ+17] = "1lock"; /* long enough for _uid+name+.99;1 */
# else
char lock[PL_NSIZ+14] = "1lock"; /* long enough for uid+name+.99 */
# endif
#endif

#ifdef MAC
#include <files.h>
MacDirs theDirs ;
#endif

#ifdef UNIX
#define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */
#else
# ifdef VMS
#define SAVESIZE	(PL_NSIZ + 22)	/* [.save]<uid>player.e;1 */
# else
#define SAVESIZE	FILENAME	/* from macconf.h or pcconf.h */
# endif
#endif

char SAVEF[SAVESIZE];	/* holds relative path of save file from playground */
#ifdef MICRO
char SAVEP[SAVESIZE];	/* holds path of directory for save file */
#endif
#ifdef AMIGA
extern char PATH[];	/* see sys/amiga/amidos.c */
#endif

extern int n_dgns;		/* from dungeon.c */

static char * FDECL(set_bonesfile_name, (char *,d_level*));

/* fopen a file, with OS-dependent bells and whistles */
FILE *
fopen_datafile(filename, mode)
const char *filename, *mode;
{
	FILE *fp;
#ifdef AMIGA
	fp = fopenp(filename, mode);
#else
# ifdef VMS	/* essential to have punctuation, to avoid logical names */
	char tmp[BUFSIZ];

	if (!index(filename, '.') && !index(filename, ';'))
		filename = strcat(strcpy(tmp, filename), ";0");
	fp = fopen(filename, mode, "mbc=16");
# else
	fp = fopen(filename, mode);
# endif
#endif
	return fp;
}


/* ----------  BEGIN LEVEL FILE HANDLING ----------- */

#ifdef MFLOPPY
/* Set names for bones[] and lock[] */
void
set_lock_and_bones()
{
	if (!ramdisk) {
		Strcpy(levels, permbones);
		Strcpy(bones, permbones);
	}
	append_slash(permbones);
	append_slash(levels);
	append_slash(bones);
	Strcat(bones, "bonesnn.*");
	Strcpy(lock, levels);
	Strcat(lock, alllevels);
	return;
}
#endif /* MFLOPPY */


/* Construct a file name for a level-type file, which is of the form
 * something.level (with any old level stripped off).
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'level'
 * but be careful if you use it for other things -dgk
 */
void
set_levelfile_name(file, lev)
char *file;
int lev;
{
	char *tf;

	tf = rindex(file, '.');
	if (!tf) tf = eos(file);
	Sprintf(tf, ".%d", lev);
#ifdef VMS
	Strcat(tf, ";1");
#endif
	return;
}

int
create_levelfile(lev)
int lev;
{
	int fd;

	set_levelfile_name(lock, lev);

#if defined(MICRO)
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
	fd = open(lock, O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
#else
# ifdef MAC
	fd = maccreat(lock, LEVL_TYPE);
# else
	fd = creat(lock, FCMASK);
# endif
#endif /* MICRO */

	return fd;
}


int
open_levelfile(lev)
int lev;
{
	int fd;

	set_levelfile_name(lock, lev);
#ifdef MFLOPPY
	/* If not currently accessible, swap it in. */
	if (fileinfo[lev].where != ACTIVE)
		swapin_file(lev);
#endif
#ifdef MAC
	fd = macopen(lock, O_RDONLY | O_BINARY, LEVL_TYPE);
#else
	fd = open(lock, O_RDONLY | O_BINARY, 0);
#endif
	return fd;
}


void
delete_levelfile(lev)
int lev;
{
	set_levelfile_name(lock, lev);
	(void) unlink(lock);
}


void
clearlocks()
{
#ifdef MFLOPPY
	eraseall(levels, alllevels);
# ifndef AMIGA
	if (ramdisk)
		eraseall(permbones, alllevels);
# endif
#else
	register int x;

# if defined(UNIX) || defined(VMS)
	(void) signal(SIGHUP, SIG_IGN);
# endif
	/* can't access maxledgerno() before dungeons are created -dlc */
	for (x = (n_dgns ? maxledgerno() : 0); x >= 0; x--)
		delete_levelfile(x);	/* not all levels need be present */
#endif
}

/* ----------  END LEVEL FILE HANDLING ----------- */


/* ----------  BEGIN BONES FILE HANDLING ----------- */

static char *
set_bonesfile_name(file, lev)
char *file;
d_level *lev;
{
	char *dptr = rindex(file, '.');
	s_level *sptr;

	if (!dptr) dptr = eos(file);
	*(dptr-2) = dungeons[lev->dnum].boneid;
#ifdef MULDGN
	*(dptr-1) = In_quest(lev) ? pl_character[0] : '0';
#else
	*(dptr-1) = '0';
#endif
	if ((sptr = Is_special(lev)) != 0)
	    Sprintf(dptr, ".%c", sptr->boneid);
	else
	    Sprintf(dptr, ".%d", lev->dlevel);
#ifdef VMS
	Strcat(dptr, ";1");
#endif
	return(dptr-2);
}

int
create_bonesfile(lev, bonesid)
d_level *lev;
char **bonesid;
{
	int fd;

	*bonesid = set_bonesfile_name(bones, lev);

#ifdef MICRO
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
	fd = open(bones, O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
#else
# ifdef MAC
	fd = maccreat(bones, BONE_TYPE);
# else
	fd = creat(bones, FCMASK);
# endif
# if defined(VMS) && !defined(SECURE)
	/*
	   Re-protect bones file with world:read+write+execute+delete access.
	   umask() doesn't seem very reliable; also, vaxcrtl won't let us set
	   delete access without write access, which is what's really wanted.
	   Can't simply create it with the desired protection because creat
	   ANDs the mask with the user's default protection, which usually
	   denies some or all access to world.
	 */
	(void) chmod(bones, FCMASK | 007);  /* allow other users full access */
# endif /* VMS && !SECURE */
#endif /* MICRO */

	return fd;
}


int
open_bonesfile(lev, bonesid)
d_level *lev;
char **bonesid;
{
	int fd;

	*bonesid = set_bonesfile_name(bones, lev);
	uncompress(bones);	/* no effect if nonexistent */
#ifdef MAC
	fd = macopen(bones, O_RDONLY | O_BINARY, BONE_TYPE);
#else
	fd = open(bones, O_RDONLY | O_BINARY, 0);
#endif
	return fd;
}


int
delete_bonesfile(lev)
d_level *lev;
{
	(void) set_bonesfile_name(bones, lev);
	return !(unlink(bones) < 0);
}


/* assume we're compressing the recently read or created bonesfile, so the
 * file name is already set properly */
void
compress_bonesfile()
{
	compress(bones);
}

/* ----------  END BONES FILE HANDLING ----------- */


/* ----------  BEGIN SAVE FILE HANDLING ----------- */

/* set savefile name in OS-dependent manner from pre-existing plname,
 * avoiding troublesome characters */
void
set_savefile_name()
{
#ifdef VMS
	Sprintf(SAVEF, "[.save]%d%s", getuid(), plname);
	regularize(SAVEF+7);
	Strcat(SAVEF, ";1");
#else
# ifdef MICRO
	Strcpy(SAVEF, SAVEP);
	{
		int i = strlen(SAVEP);
#  ifdef AMIGA
		/* plname has to share space with SAVEP and ".sav" */
		(void)strncat(SAVEF, plname, FILENAME - i - 4);
#  else
		(void)strncat(SAVEF, plname, 8);
#  endif
		regularize(SAVEF+i);
	}
	Strcat(SAVEF, ".sav");
# else
	Sprintf(SAVEF, "save/%d%s", (int)getuid(), plname);
	regularize(SAVEF+5);	/* avoid . or / in name */
# endif	/* MICRO */
#endif	/* VMS */
}

#ifdef INSURANCE
void
save_savefile_name(fd)
int fd;
{
	(void) write(fd, (genericptr_t) SAVEF, sizeof(SAVEF));
}
#endif


#if defined(WIZARD) && !defined(MICRO)
/* change pre-existing savefile name to indicate an error savefile */
void
set_error_savefile()
{
# ifdef VMS
      {
	char *semi_colon = rindex(SAVEF, ';');
	if (semi_colon) *semi_colon = '\0';
      }
	Strcat(SAVEF, ".e;1");
# else
#  ifdef MAC
	Strcat(SAVEF, "-e");
#  else
	Strcat(SAVEF, ".e");
#  endif
# endif
}
#endif


/* create save file, overwriting one if it already exists */
int
create_savefile()
{
	int fd;
#ifdef AMIGA
	fd = ami_wbench_getsave(O_WRONLY | O_CREAT | O_TRUNC);
#else
# ifdef MICRO
	fd = open(SAVEF, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
# else
#  ifdef MAC
	fd = maccreat(SAVEF, SAVE_TYPE);
#  else
	fd = creat(SAVEF, FCMASK);
#  endif
#  if defined(VMS) && !defined(SECURE)
	/*
	   Make sure the save file is owned by the current process.  That's
	   the default for non-privileged users, but for priv'd users the
	   file will be owned by the directory's owner instead of the user.
	 */
#   ifdef getuid	/*(see vmsunix.c)*/
#    undef getuid
#   endif
	(void) chown(SAVEF, getuid(), getgid());
#  endif /* VMS && !SECURE */
# endif	/* MICRO */
#endif	/* AMIGA */

	return fd;
}


/* open savefile for reading */
int
open_savefile()
{
	int fd;

#ifdef AMIGA
	fd = ami_wbench_getsave(O_RDONLY);
#else
# ifdef MAC
	fd = macopen(SAVEF, O_RDONLY | O_BINARY, SAVE_TYPE);
# else
	fd = open(SAVEF, O_RDONLY | O_BINARY, 0);
# endif
#endif /* AMIGA */
	return fd;
}


/* delete savefile */
int
delete_savefile()
{
#ifdef AMIGA
	ami_wbench_unlink(SAVEF);
#endif
	(void) unlink(SAVEF);
	return 0;	/* for xxxmain.c test */
}


/* ----------  END SAVE FILE HANDLING ----------- */


/* ----------  BEGIN FILE COMPRESSION HANDLING ----------- */

#ifdef COMPRESS
/* 
 * using system() is simpler, but opens up security holes and causes
 * problems on at least Interactive UNIX 3.0.1 (SVR3.2), where any
 * setuid is renounced by /bin/sh, so the files cannot be accessed.
 *
 * cf. child() in unixunix.c.
 */
void
docompress_file(filename, uncomp)
char *filename;
boolean uncomp;
{
	char *args[10];
# ifdef COMPRESS_OPTIONS
	char opts[80];
# endif
	int i = 0;
	int f;

	args[0] = COMPRESS;
	if (uncomp) args[++i] = "-d";	/* uncompress */
# ifdef COMPRESS_OPTIONS
	{
	    /* we can't guarantee there's only one additional option, sigh */
	    char *opt;
	    boolean inword = FALSE;

	    Strcpy(opts, COMPRESS_OPTIONS);
	    opt = opts;
	    while (*opt) {
		if ((*opt == ' ') || (*opt == '\t')) {
		    if (inword) {
			*opt = '\0';
			inword = FALSE;
		    }
		} else if (!inword) {
		    args[++i] = opt;
		    inword = TRUE;
		}
		opt++;
	    }
	}
# endif
	args[++i] = filename;
	args[++i] = NULL;

	f = fork();
	if (f == 0) {	/* child */
		(void) execv(args[0], args);
		perror(NULL);
		pline("Exec to %scompress %s failed.",
			uncomp ? "un" : "", filename);
		exit(1);
	} else if (f == -1) {
		perror(NULL);
		pline("Fork to %scompress %s failed.",
			uncomp ? "un" : "", filename);
		return;
	}
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) wait((int *)0);
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
# ifdef WIZARD
	if (wizard) (void) signal(SIGQUIT, SIG_DFL);
# endif
}
#endif

/* compress file */
void
compress(filename)
const char *filename;
#ifdef applec
# pragma unused(filename)
#endif
{
#ifdef COMPRESS
	docompress_file(filename, FALSE);
#endif
}


/* uncompress file if it exists */
void
uncompress(filename)
const char *filename;
#ifdef applec
# pragma unused(filename)
#endif
{
#ifdef COMPRESS
	char cfn[80];
	int fd;

	Strcpy(cfn, filename);
# ifdef COMPRESS_EXTENSION
	Strcat(cfn, COMPRESS_EXTENSION);
# endif
	if ((fd = open(cfn, O_RDONLY)) >= 0) {
		(void) close(fd);
		docompress_file(cfn, TRUE);
	}
#endif
}

/* ----------  END FILE COMPRESSION HANDLING ----------- */


/* ----------  BEGIN FILE LOCKING HANDLING ----------- */

#ifdef NO_FILE_LINKS	/* implies UNIX */
static int lockfd;	/* for lock_file() to pass to unlock_file() */
#endif

#if defined(UNIX) || defined(VMS)
#define HUP	if(!done_hup)

static char *
make_lockname(filename)
const char *filename;
{
	static char lockname[BUFSZ];

# ifdef NO_FILE_LINKS
	Strcpy(lockname, LOCKDIR);
	Strcat(lockname, "/");
	Strcat(lockname, filename);
# else
	Strcpy(lockname, filename);
# endif
# ifdef VMS
      {
	char *semi_colon = rindex(lockname, ';');
	if (semi_colon) *semi_colon = '\0';
      }
	Strcat(lockname, ".lock;1");
# else
	Strcat(lockname, "_lock");
# endif
	return lockname;
}
#endif  /* UNIX || VMS */


/* lock a file */
boolean
lock_file(filename, retryct)
const char *filename;
int retryct;
#ifdef applec
# pragma unused(filename, retryct)
#endif
{
#if defined(UNIX) || defined(VMS)
	char *lockname;

	lockname = make_lockname(filename);

# ifdef NO_FILE_LINKS
	while ((lockfd = open(lockname, O_RDWR|O_CREAT|O_EXCL, 0666)) == -1) {
# else
	while (link(filename, lockname) == -1) {
# endif
		register int errnosv = errno;

		switch (errnosv) {	/* George Barbanis */
		    case ENOENT:
			HUP raw_printf("Can't find file %s to lock!", filename);
			return FALSE;
		    case EACCES:
			HUP raw_printf("No write permission to lock %s!",
				       filename);
			return FALSE;
# ifdef VMS			/* c__translate(vmsfiles.c) */
		    case EPERM:
			/* could be misleading, but usually right */
			HUP raw_printf(
				  "Can't lock %s due to directory protection.",
				       filename);
			return FALSE;
# endif
		    case EEXIST:
			break;	/* retry checks below */
		    default:
			HUP perror(lockname);
			HUP raw_printf(
				     "Cannot lock %s for unknown reason (%d).",
				       filename, errnosv);
			return FALSE;
		}

		if (!retryct--) {
			HUP (void) raw_print("I give up.  Sorry.");
			HUP raw_printf("Perhaps there is an old %s around?",
				       lockname);
			return FALSE;
		}

		HUP raw_printf("Waiting for access to %s.  (%d retries left).",
			       filename, retryct);
# if defined(SYSV) || defined(ULTRIX) || defined(VMS)
		(void)
# endif
			sleep(1);
	}
#endif  /* UNIX || VMS */
	return TRUE;
}


#ifdef VMS	/* for unlock_file, use the unlink() routine in vmsunix.c */
# ifdef unlink
#  undef unlink
# endif
# define unlink(foo) vms_unlink(foo)
#endif

/* unlock file, which must be currently locked by lock_file */
void
unlock_file(filename)
const char *filename;
#if defined(applec)
# pragma unused(filename)
#endif
{
#if defined(UNIX) || defined(VMS)
	char *lockname;

	lockname = make_lockname(filename);

	if (unlink(lockname) < 0)
		HUP raw_printf("Can't unlink %s.", lockname);
# ifdef NO_FILE_LINKS
	(void) close(lockfd);
# endif

#endif  /* UNIX || VMS */
}

/* ----------  END FILE LOCKING HANDLING ----------- */


/* ----------  BEGIN CONFIG FILE HANDLING ----------- */

const char *configfile =
#ifdef UNIX
			".nethackrc";
#else
# ifdef MAC
			"NetHack defaults";
# else
			"NetHack.cnf";
# endif
#endif

static FILE *FDECL(fopen_config_file, (const char *));
static int FDECL(get_uchars, (FILE *, char *, char *, uchar *, int, const char *));
int FDECL(parse_config_line, (FILE *, char *, char *, char *));

#ifndef MFLOPPY
#define fopenp fopen
#endif

static FILE *
fopen_config_file(filename)
const char *filename;
{
	FILE *fp;
#if defined(UNIX) || defined(VMS)
	char	tmp_config[BUFSZ];
#endif

	/* "filename" is an environment variable, so it should hang around */
	if (filename) {
#ifdef UNIX
		if (access(filename, 4) == -1) {
			/* 4 is R_OK on newer systems */
			/* nasty sneaky attempt to read file through
			 * NetHack's setuid permissions -- this is the only
			 * place a file name may be wholly under the player's
			 * control
			 */
			raw_printf("Access to %s denied (%d).",
					filename, errno);
			wait_synch();
			/* fall through to standard names */
		} else
#endif
		if ((fp = fopenp(filename, "r")) != (FILE *)0) {
			configfile = filename;
			return(fp);
		}
	}

#if defined(MICRO) || defined(MAC)
	if ((fp = fopenp(configfile, "r")) != (FILE *)0)
		return(fp);
#else
# ifdef VMS
	if ((fp = fopenp("nethackini", "r")) != (FILE *)0) {
		configfile = "nethackini";
		return(fp);
	}
	if ((fp = fopenp("sys$login:nethack.ini", "r")) != (FILE *)0) {
		configfile = "nethack.ini";
		return(fp);
	}
	Sprintf(tmp_config, "%s%s", getenv("HOME"), "NetHack.cnf");
	if ((fp = fopenp(tmp_config, "r")) != (FILE *)0)
		return(fp);
# else	/* should be only UNIX left */
	Sprintf(tmp_config, "%s/%s", getenv("HOME"), ".nethackrc");
	if ((fp = fopenp(tmp_config, "r")) != (FILE *)0)
		return(fp);
# endif
#endif
	return (FILE *)0;

}


/*
 * Retrieve a list of integers from a file into a uchar array.
 *
 * NOTE:  This routine is unable to read a value of 0.
 */
static int
get_uchars(fp, buf, bufp, list, size, name)
    FILE *fp;		/* input file pointer */
    char *buf;		/* read buffer, must be of size BUFSZ */
    char *bufp;		/* current pointer */
    uchar *list;	/* return list */
    int  size;		/* return list size */
    const char *name;		/* name of option for error message */
{
    unsigned int num = 0;
    int count = 0;

    while (1) {
	switch(*bufp) {
	    case ' ':  case '\0':
	    case '\t': case '\n':
		if (num) {
		    list[count++] =  num;
		    num = 0;
		}
		if (count == size || !*bufp) return count;
		bufp++;
		break;

	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	    case '8': case '9':
		num = num*10 + (*bufp-'0');
		bufp++;
		break;

	    case '\\':
	    	if (fp == (FILE *)0)
		    goto gi_error;
		do  {
		    if (!fgets(buf, BUFSZ, fp)) goto gi_error;
		} while (buf[0] == '#');
		bufp = buf;
		break;

	    default:
gi_error:
		raw_printf("Syntax error in %s", name);
		wait_synch();
		return count;
	}
    }
    /*NOTREACHED*/
}

/*ARGSUSED*/
int
parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)
FILE		*fp;
char		*buf;
char		*tmp_ramdisk;
char		*tmp_levels;
#if defined(applec)
# pragma unused(tmp_ramdisk,tmp_levels)
#endif
{
	char		*bufp, *altp;

	if (*buf == '#')
		return 1;

	/* remove trailing whitespace */
	bufp = eos(buf);
	while (--bufp > buf && isspace(*bufp))
		continue;

	if (bufp <= buf)
		return 1;		/* skip all-blank lines */
	else
		*(bufp + 1) = '\0';	/* terminate line */

	/* find the '=' or ':' */
	bufp = index(buf, '=');
	altp = index(buf, ':');
	if (!bufp || (altp && altp < bufp)) bufp = altp;
	if (!bufp) return 0;

	/* skip  whitespace between '=' and value */
	do { ++bufp; } while (isspace(*bufp));

	/* Go through possible variables */
	if (!strncmpi(buf, "OPTIONS", 4)) {
		parseoptions(bufp, TRUE, TRUE);
		if (plname[0])		/* If a name was given */
			plnamesuffix();	/* set the character class */
#ifdef MICRO
	} else if (!strncmpi(buf, "HACKDIR", 4)) {
		(void) strncpy(hackdir, bufp, PATHLEN);
# ifdef MFLOPPY
	} else if (!strncmpi(buf, "RAMDISK", 3)) {
				/* The following ifdef is NOT in the wrong
				 * place.  For now, we accept and silently
				 * ignore RAMDISK */
#  ifndef AMIGA
		(void) strncpy(tmp_ramdisk, bufp, PATHLEN);
#  endif
# endif
	} else if (!strncmpi(buf, "LEVELS", 4)) {
		(void) strncpy(tmp_levels, bufp, PATHLEN);

	} else if (!strncmpi(buf, "SAVE", 4)) {
# ifdef MFLOPPY
		extern	int saveprompt;
#endif
		char *ptr;
		if (ptr = index(bufp, ';')) {
			*ptr = '\0';
# ifdef MFLOPPY
			if (*(ptr+1) == 'n' || *(ptr+1) == 'N') {
				saveprompt = FALSE;
			}
# endif
		}
#ifdef	MFLOPPY
		else
		    saveprompt = flags.asksavedisk;
#endif

		(void) strncpy(SAVEP, bufp, PATHLEN);
		append_slash(SAVEP);
#endif /* MICRO */
	} else if(!strncmpi(buf, "CHARACTER", 4)) {
	    (void) strncpy(pl_character, bufp, PL_CSIZ);
	} else if(!strncmpi(buf, "DOGNAME", 3)) {
	    (void) strncpy(dogname, bufp, 62);
	} else if(!strncmpi(buf, "CATNAME", 3)) {
	    (void) strncpy(catname, bufp, 62);
	} else if(!strncmpi(buf, "NAME", 4)) {
	    (void) strncpy(plname, bufp, PL_NSIZ);
	    plnamesuffix();
	} else if (!strncmpi(buf, "GRAPHICS", 4)) {
	    uchar   translate[MAXPCHARS];
	    int   len;

	    len = get_uchars(fp, buf, bufp, translate,
					MAXPCHARS, "GRAPHICS");
	    assign_graphics(translate, len);
	} else if (!strncmpi(buf, "OBJECTS", 3)) {
	    /* oc_syms[0] is the RANDOM object, unused */
	    (void) get_uchars(fp, buf, bufp, &(oc_syms[1]),
					MAXOCLASSES-1, "OBJECTS");
	} else if (!strncmpi(buf, "MONSTERS", 3)) {
	    /* monsyms[0] is unused */
	    (void) get_uchars(fp, buf, bufp, &(monsyms[1]),
					MAXMCLASSES-1, "MONSTERS");
#ifdef AMIGA
	} else if (!strncmpi(buf, "FONT", 4)) {
		char *t;
		int size;
		extern void amii_set_text_font( char *, int );

		if( t = strchr( buf+5, ':' ) )
		{
		    *t = 0;
		    amii_set_text_font( buf+5, atoi( t + 1 ) );
		    *t = ':';
		}
	} else if (!strncmpi(buf, "PATH", 4)) {
		(void) strncpy(PATH, bufp, PATHLEN);
	} else if (!strncmpi(buf, "PENS", 3)) {
# ifdef AMII_GRAPHICS
		int i;
		char *t;
		extern void amii_setpens( void );

		for (i = 0, t = strtok(bufp, ",/"); t != NULL;
				    t = strtok(NULL, ",/"), ++i)
		{
			sscanf(t, "%hx", &flags.amii_curmap[i]);
		}
		amii_setpens();
# endif
#endif
	} else
		return 0;
	return 1;
}

void
read_config_file(filename)
const char *filename;
{
#define tmp_levels	(char *)0
#define tmp_ramdisk	(char *)0

#ifdef MICRO
#undef tmp_levels
	char	tmp_levels[PATHLEN];
# ifdef MFLOPPY
#  ifndef AMIGA
#undef tmp_ramdisk
	char	tmp_ramdisk[PATHLEN];
#  endif
# endif
#endif
	char	buf[BUFSZ];
	FILE	*fp;

#ifdef MAC
	long nul = 0L ;
	Str255 volName ;
	/*
	 * We should try to get this data from a rsrc, in the profile file
	 * the user double-clicked...  This data should be saved with the
	 * save file in the resource fork, AND be saveable in "stationery"
	 */
	GetVol ( volName , & theDirs . dataRefNum ) ;
	GetWDInfo ( theDirs . dataRefNum , & theDirs . dataRefNum , & theDirs .
		dataDirID , & nul ) ;
	if ( volName [ 0 ] > 31 ) volName [ 0 ] = 31 ;
	for ( nul = 1 ; nul <= volName [ 0 ] ; nul ++ ) {
		if ( volName [ nul ] == ':' ) {
			volName [ nul ] = 0 ;
			volName [ 0 ] = nul - 1 ;
			break ;
		}
	}
	BlockMove ( volName , theDirs . dataName , 32L ) ;
#endif /* MAC */

	if (!(fp = fopen_config_file(filename))) return;

#ifdef MICRO
# ifdef MFLOPPY
#  ifndef AMIGA
	tmp_ramdisk[0] = 0;
#  endif
# endif
	tmp_levels[0] = 0;
#endif

	while (fgets(buf, BUFSZ, fp)) {
		if (!parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)) {
			raw_printf("Bad option line:  \"%s\"", buf);
			wait_synch();
		}
	}
	(void) fclose(fp);

#ifdef MICRO
# ifdef MFLOPPY
	Strcpy(permbones, tmp_levels);
#  ifndef AMIGA
	if (tmp_ramdisk[0]) {
		Strcpy(levels, tmp_ramdisk);
		if (strcmp(permbones, levels))		/* if not identical */
			ramdisk = TRUE;
	} else
#  endif /* AMIGA */
		Strcpy(levels, tmp_levels);

	Strcpy(bones, levels);
# endif /* MFLOPPY */
#endif /* MICRO */
	return;
}

/* ----------  END CONFIG FILE HANDLING ----------- */

/* ----------  BEGIN SCOREBOARD CREATION ----------- */

/* verify that we can write to the scoreboard file; if not, try to create one */
void
check_recordfile(dir)
const char *dir;
#if defined(applec)
# pragma unused(dir)
#endif
{
#if defined(UNIX) || defined(VMS)
	int fd = open(RECORD, O_RDWR, 0);

	if (fd >= 0) {
# ifdef VMS	/* must be stream-lf to use UPDATE_RECORD_IN_PLACE */
		if (!file_is_stmlf(fd)) {
		    raw_printf(	/* note: assume VMS dir has trailing punct */
		  "Warning: scoreboard file %s%s is not in stream_lf format",
				(dir ? dir : "[]"), RECORD);
		    wait_synch();
		}
# endif
	    (void) close(fd);	/* RECORD is accessible */
	} else if ((fd = open(RECORD, O_CREAT|O_RDWR, FCMASK)) >= 0) {
	    (void) close(fd);	/* RECORD newly created */
# if defined(VMS) && !defined(SECURE)
	    /* Re-protect RECORD with world:read+write+execute+delete access. */
	    (void) chmod(RECORD, FCMASK | 007); /* allow everyone full access */
# endif /* VMS && !SECURE */
	} else {
	    raw_printf("Warning: cannot write scoreboard file %s/%s",
			(dir ? dir : "."), RECORD);
	    wait_synch();
	}
#endif  /* !UNIX && !VMS */

#ifdef MICRO
	int fd;
	char tmp[PATHLEN];

# ifdef OS2_CODEVIEW   /* explicit path on opening for OS/2 */
	Strcpy(tmp, dir);
	append_slash(tmp);
	Strcat(tmp, RECORD);
# else
	Strcpy(tmp, RECORD);
# endif

	if ((fd = open(tmp, O_RDWR)) < 0) {
	    /* try to create empty record */
# ifdef AZTEC_C
	    /* Aztec doesn't use the third argument */
	    if ((fd = open(tmp, O_CREAT|O_RDWR)) < 0) {
# else
	    if ((fd = open(tmp, O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) < 0) {
# endif
        raw_printf("Warning: cannot write record %s", tmp);
		wait_synch();
	    } else
		(void) close(fd);
	} else		/* open succeeded */
	    (void) close(fd);
#else /* MICRO */

# ifdef MAC
	int fd = macopen ( RECORD , O_RDWR | O_CREAT , TEXT_TYPE ) ;

	if ( fd < 0 ) {
		raw_printf ( "Warning: cannot write %s" , RECORD ) ;
	} else {
		close ( fd ) ;
	}
# endif /* MAC */

#endif /* MICRO */
}

/* ----------  END SCOREBOARD CREATION ----------- */


/*files.c*/
