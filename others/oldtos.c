/*	SCCS Id: @(#)oldtos.c	3.0	88/11/09
 * An assortment of functions for the Atari with Lattice C compiler.
 * Adapted from a similar set for MSDOS (written by Don Kneller) by
 * R. Black (Louisville, CO).
 */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern int errno;

void
flushout()
{
	(void) fflush(stdout);
}

getuid()
{
	return 1;
}

char *
getlogin()
{
	return NULL;
}

tgetch()
{
	char ch;
	ch = BIOSgetch();
	return ((ch == '\r') ? '\n' : ch);
}

#ifdef exit
#undef exit
#endif

#undef creat
#define LEVELS	"level.00"
#define BONES	"bones.00"

extern char hackdir[], levels[], SAVEF[], bones[], permbones[];
extern int ramdisk;

#ifdef SHELL
#ifndef UNIXDEBUG
#include <process.h>
#else	    /* null stuff for debugging purposes */
#define	    P_WAIT	0
#endif
dosh()
{
	char *comspec, *getenv();
	extern char orgdir[];

	if (comspec = getenv("COMSPEC")) {
		end_screen();
		clear_screen();
		(void) puts("To return to HACK, type \"exit\" at the TOS prompt.");
		(void) fflush(stdout);
		chdirx(orgdir, 0);
		if (spawnl(P_WAIT, comspec, NULL)) {
			Printf("\nCan't execute COMSPEC \"%s\"!\n", comspec);
			flags.toplin = 0;
			more();
		}
		chdirx(hackdir, 0);
		start_screen();
		docrt();
	}
	return(0);
}
#endif SHELL

/* Map the keypad to equivalent character output.  If scroll lock is turned
 * on, run without stopping at interesting branches (like YUHJKLBN keys), if
 * it is off, run with stopping at branches (like ^Y^U^H^J^K^L^B^N keys).
 */
static char
BIOSgetch()
{
	int scan,c,result;
	char ch;
	result=Crawcin();
	scan=(result>>16)&0xff;
	c=result&0xff;
	switch (scan) {
		case 0x4a : {/*-*/
			ch=(c==0x2d) ? 'm' : '\20';
			break;
		}
		case 0x4e : {/*+*/
			ch=(c==0x2b) ? 'p' : 'P';
			break;
		}
		case 0x6d : {/*1*/
			if (flags.num_pad) ch='1';
			else ch=(c==0x31) ? 'b' : '\2';
			break;
		}
		case 0x6e : {/*2*/
			if (flags.num_pad) ch='2';
			else ch=(c==0x32) ? 'j' : '\12';
			break;
		}
		case 0x6f : {/*3*/
			if (flags.num_pad) ch='3';
			else ch=(c==0x33) ? 'n' : '\16';
			break;
		}
		case 0x6a : {/*4*/
			if (flags.num_pad) ch='4';
			else ch=(c==0x34) ? 'h' : '\10';
			break;
		}
		case 0x6b : {/*5*/
			ch='.';
			break;
		}
		case 0x6c : {/*6*/
			if (flags.num_pad) ch='6';
			else ch=(c==0x36) ? 'l' : '\14';
			break;
		}
		case 0x67 : {/*7*/
			if (flags.num_pad) ch='7';
			else ch=(c==0x37) ? 'y' : '\31';
			break;
		}
		case 0x68 : {/*8*/
			if (flags.num_pad) ch='8';
			else ch=(c==0x38) ? 'k' : '\13';
			break;
		}
		case 0x69 : {/*9*/
			if (flags.num_pad) ch='9';
			else ch=(c==0x39) ? 'u' : '\25';
			break;
		}
		default : ch=c;
	}
	return (ch);
}

/* After changing the value of flags.invlet_constant, make the current
 * inventory letters the fixed ones. -dgk
 */
void
fixinv()
{
	struct obj *otmp;
	extern int lastinvnr;
	char ilet = 'a';

	for(otmp = invent; otmp; otmp = otmp->nobj) {
		otmp->invlet = ilet;
		if (++ilet > 'z') ilet = 'A';
	}
	lastinvnr = 51;
}


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

long
filesize(file)
char *file;
{
	long old_dta;
	struct {
		char pad1[21];
		char attr;
		unsigned short time;
		unsigned short date;
		long size;
		char name[14];
	} fcb;
	fcb.size = (-1L);
	old_dta=Fgetdta();
	Fsetdta(&fcb);
	Fsfirst(file,0);
	Fsetdta(old_dta);
	return fcb.size;
}

long
all_files_size(path)
char *path;
{
	register int level;
	long size, tmp;
	char buf[PATHLEN];

	Strcpy(buf, path);
	for (level = 1, size = 0; level <= MAXLEVEL; level++) {
		name_file(buf, level);
		tmp = filesize(buf);
		if (tmp > 0)
			size += tmp;
	}
	return (size);
}

void
copybones(mode)
int mode;
{
	register int fd, level;
	char from[PATHLEN], to[PATHLEN];
	long sizes[MAXLEVEL + 1];
	extern boolean level_exists[];
	if (!ramdisk)
		return;
	Strcpy(to, (mode == TOPERM) ? permbones : bones);
	Strcpy(from, (mode == TOPERM) ? bones : permbones);

	for (level = 1; level <= MAXLEVEL; level++) {
		name_file(from, level);
		sizes[level] = filesize(from);	/* -1 if file doesn't exist */
		name_file(to, level);
		(void) unlink(to);	/* remove old bones files in 'to' */
	}
	for (level = 1; level <= MAXLEVEL; level++) {
		if (sizes[level] == -1L)
			continue;
		name_file(from, level);
		name_file(to, level);
		if (sizes[level] > freediskspace(to)) {
			cprintf(
				"Not enough room to copy file '%s' to '%s'.\n",
				from, to);
			goto cleanup;
		}
		/* We use savelev and getlev to move the bones files around,
		 * but savelev sets level_exists[] TRUE for this level, so
		 * we have to set it back FALSE again.
		 */
#ifdef TOS
		if ((fd = open(from, 0x8000)) < 0) {
#else TOS
		if ((fd = open(from, 0)) < 0) {
#endif TOS
			cprintf( "Warning: can't open '%s'.\n", from);
			continue;
		} else {
			sizes[level] = 0;	/* 'from' bones exists */
			getlev(fd, 0, level, FALSE);
			(void) close(fd);
			if ((fd = creat(to, FCMASK)) < 0) {
				cprintf(
					"Warning: can't create '%s'.\n", to);
				continue;
			} else {
				savelev(fd, level);
				(void) close(fd);
				level_exists[level] = FALSE;	/* see above */
			}
		}
	}
	/* If we are copying bones files back to permanent storage, unlink
	 * the bones files in the LEVELS directory.
	 */
	if (mode == TOPERM)
		for (level = 1; level <= MAXLEVEL; level++) {
			if (sizes[level] == -1)
				continue;
			name_file(from, level);
			(void) unlink(from);
		}
	return;

cleanup:
	/* Ran out of disk space!  Unlink the "to" files and issue an
	 * appropriate message.
	 */
	for (level = 1; level <= MAXLEVEL; level++)
		if (sizes[level] == 0) {
			name_file(to, level);
			(void) unlink(to);
		}
	cprintf( "There is not enough room in ");
	if (mode == TOPERM) {
		cprintf( "permanent storage for bones files!\n");
		cprintf("Bones will be left in LEVELS directory '%s'!\n",
			levels[0] ? levels : ".");
			return;
	} else {
		cprintf("LEVELS directory '%s' to copy bones files!\n",
			levels[0] ? levels : ".");
		getreturn("to quit");
		settty("Be seeing you...\n");
		exit(0);
	}
}

saveDiskPrompt(start)
int start;
{
	extern saveprompt;
	char buf[BUFSIZ];
	int i;

	if (saveprompt) {
		remember_topl();
		home();
		cl_end();
		Printf("If save file is on a SAVE disk, put that disk in now.\n");
		cl_end();
		Printf("Name of save file (default: '%s'%s) ? ", SAVEF,
			start ? "" : ", <Esc> aborts save");
		(void) fflush(stdout);
		getlin(buf);
		if (!start && buf[0] == '\033') {
			home();
			cl_end();
			curs(1, 2);
			cl_end();
			return 0;
		}
		for (i = 0; buf[i]; i++)
			if (!isspace(buf[i])) {
				strncpy(SAVEF, buf, PATHLEN);
				break;
			}
	}
	return 1;
}

/* Prompt for the game disk, then check if you can access the record file.
 * If not, warn the player.
 */
void
gameDiskPrompt()
{
	FILE	*fp;
	extern saveprompt;

	if (saveprompt) {
		putch('\n');
		getreturn("when the GAME disk is ready");
	}
	if ((fp = fopen(RECORD, "r")))	/* check for GAME disk */
		(void) fclose(fp);
	else {
		cprintf("\n\nWARNING: I can't find record file '%s'!\n",
			 RECORD);
		cprintf("If the GAME disk is not in, put it in now.\n");
		getreturn("to continue");
	}
}

#define CONFIGFILE	"hack103.cnf"
void
read_config_file()
{
	char	config[FILENAME], tmp_ramdisk[PATHLEN], tmp_levels[PATHLEN];
	char	buf[BUFSZ], *bufp;
	FILE	*fp;
	extern	char plname[];
	extern	int saveprompt;

	tmp_ramdisk[0] = 0;
	tmp_levels[0] = 0;
	Strcpy(config, hackdir);
	append_slash(config);
	Strcat(config, CONFIGFILE);
	if (!(fp = fopen(config, "r"))) {
		cprintf("Warning: no configuration file '%s'!\n",
			config);
		getreturn("to continue");
		return;
	}
	while (fgets(buf, BUFSZ, fp)) {
		if (*buf == '#')		/* comment first character */
			continue;

		/* remove any trailing whitespace
		 */
		bufp = index(buf, '\n');
		while (bufp > buf && isspace(*bufp))
			bufp--;
		if (bufp == buf)
			continue;		/* skip all-blank lines */
		else
			*(bufp + 1) = 0;	/* 0 terminate line */

		/* find the '=' separating option name from option value
		 */
		if (!(bufp = strchr(buf, '='))) {
			cprintf("Bad option line: '%s'\n", buf);
			getreturn("to continue");
			continue;
		}

		/* move past whitespace between '=' and option value
		 */
		while (isspace(*++bufp))
			;

		/* Now go through the possible configurations
		 */
		if (!strncmp(buf, "RAMDISK", 3)) {
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

		} else if (!strncmp(buf, "GRAPHICS", 4)) {
			unsigned int translate[MAXPCHARS+1];
			int lth;

		     if ((lth = sscanf(bufp,
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
					cprintf("Syntax error in GRAPHICS\n");
					getreturn("to continue");
			}
			assign_graphics(translate, lth);
		} else {
			cprintf("Bad option line: '%s'\n", buf);
			getreturn("to continue");
		}
	}
	(void) fclose(fp);

	Strcpy(permbones, tmp_levels);
	if (tmp_ramdisk[0]) {
		Strcpy(levels, tmp_ramdisk);
		if (strcmp(permbones, levels))		/* if not identical */
			ramdisk = TRUE;
	} else
		Strcpy(levels, tmp_levels);
	Strcpy(bones, levels);
}

void
set_lock_and_bones()
{
	if (!ramdisk) {
		Strcpy(levels, permbones);
		Strcpy(bones, permbones);
	}
	append_slash(bones);
	Strcat(bones, BONES);
	append_slash(permbones);
	Strcat(permbones, BONES);
	Strcpy(lock, levels);
	append_slash(lock);
	Strcat(lock, LEVELS);
}

void
append_slash(name)
char *name;
{
	char *ptr;

	if (!name[0])
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '\\' && *ptr != '/' && *ptr != ':') {
		*(ptr + 1) = '\\';
		*(ptr + 2) = 0;
	}
}


check_then_creat(file, pmode)
char *file;
int pmode;
{
	long freespace = freediskspace(file);
	extern boolean restoring;
	if (freespace < 0)
		return (-1);
	if (!restoring && freespace < 8000L) {
		pline("\7NetHack is almost out of disk space for making levels!");
		pline("You should save the game now.  Save this game?");
		(void) fflush(stdout);
		if (yn() == 'y')
			dosave();
		return(-1);	/* In case he decides not to save don't let him
				 * change levels
				 */
	}
	return( creat(file, pmode));
}

void
getreturn(str)
char *str;
{
	int ch;

	cprintf("Hit <RETURN> %s.", str);
	while ((ch = Getchar()) != '\n')
		putch(ch);
}

void
chdrive(str)
char *str;
{
	int drive;
	char *ptr;
	if ((ptr = index(str, ':')) != NULL) {
		drive = toupper(*(ptr - 1))-'A';
		Dsetdrv(drive);
	}
}


/* Do a chdir back to the original directory
 */
void msexit(code)
int code;
{
#ifdef CHDIR
	extern char orgdir[];
#endif CHDIR

#ifdef DGK
	(void) fflush(stdout);
	copybones(TOPERM);
#endif DGK
#ifdef CHDIR
	Dsetpath(orgdir);	/* do the chdir, but not with chdirx */
#endif CHDIR
	exit(code);
}

/*qsort: sort an array.  Really a shell sort, but it will do*/
void qsort(base,nel,size,compar)
char *base;
int nel, size;
int (*compar) ();
{
	int gap, i, j;
	register int cnt;
	for (gap=nel>>1; gap>0; gap>>=1)
		for (i=gap; i<nel; i++)
			for (j=i-gap; j>=0; j-=gap) {
				register char *s1, *s2;
				s1=base+(j*size);
				s2=base+((j+gap)*size);
				if ((*compar)(s1,s2)<=0) break;
				for (cnt=size;--cnt>=0;) {
					register char ch;
					ch    = *s1;
					*s1++ = *s2;
					*s2++ = ch;
				}
			}
/*end qsort*/}

static 	char *msgs[]={" ",
		"not file owner",
		"no such file or directory",
		"no such process",
		"interrupted system call",
		"I/O error",
		"no such device",
		"argument list too long",
		"exec format error",
		"bad file number",
		"no child process",
		"no more processes allowed",
		"not enough memory space",
		"permission denied",
		"bad memory address",
		"bulk device required",
		"resource is busy",
		"file exists",
		"cross device link",
		"no such device",
		"not a directory",
		"is a directory",
		"invalid argument",
		"no more files allowed",
		"too many open files",
		"not a terminal",
		"text file busy",
		"file too large",
		"no space left on device",
		"illegal seek",
		"read only file system",
		"too many links",
		"broken pipe",
		"math argument error",
		"math result too large"};

/*perror: print a string, then the error*/
void perror(s)
char *s;
{
	int select;
	if ((errno<0) || (errno>34)) select=0;
	else select=errno;
	cprintf("%s: %s\n",s,msgs[select]);
/*end perror*/}

setrandom()
{
	Srand(Tgettime());
/*end setrandom*/}

getyear()
{
	return (1980+((Tgetdate()>>9)&0x7f));
/*end getyear*/}

char *getdate()
{
	static char datestr[7];
	int day, mo, yr, date;
	date=Tgetdate();
	day=date&0x1f;
	mo=(date>>5)&0xf;
	yr=getyear()-1900;
	Sprintf(datestr,"%2d%2d%2d",yr,mo,day);
	if (datestr[2]==' ') datestr[2]='0';
	if (datestr[4]==' ') datestr[4]='0';
	return (datestr);
/*getdate*/}
static int cum[]={0,0,31,59,90,120,151,181,212,243,273,304,334};

phase_of_the_moon()
{
	int day,mo,epact,golden,date;
	date=Tgetdate();
	day=date&0x1f;
	mo=(date>>5)&0xf;
	day+=cum[mo];
	if (((getyear() % 4)==0) && (mo>2)) day++;
	golden=(getyear()%19)+1;
	epact=(11*golden+18) % 30;
	if ((epact==25 && golden>11) || (epact==24)) epact++;
	return( (((((day+epact)*6)+11)%177)/22)&7);
/*end phase_of_the_moon*/}

night()
{
	int hour;
	hour=(Tgettime()>>11)&0x1f;
	return ((hour<6) || (hour>21));
/*end night*/}

midnight()
{
	int hour;
	hour=(Tgettime()>>11)&0x1f;
	return (hour==0);
/*end midnight*/}

gethdate(name) char *name;
{
/*end gethdate*/}

uptodate(fd)
{
	return(1);
/*end uptodate*/}

getlock()
{
/*end getlock*/}

getenv(s)
char *s;
{
	return(0);
/*end getenv*/}

/*getcwd: return the current directory*/
char *getcwd(path,len)
char *path;
int len;
{
	if (Dgetpath(path,0)<0) return (NULL);
	else return(path);
/*end getcwd*/}

/*chdir: change directories*/
chdir(path)
char *path;
{
	if (*path) return (Dsetpath(path));
	else return 0;
/*end chdir*/}

/*Lattice's strncpy always appends a null*/
int strncpy(to,from,len)
char *to,*from;
int len;
{	int result;
	result=len;
	while (*from && len) {*to++ = *from++; --len;}
	if (len) {*to='\0'; --len;}
	return result-len;
/*end strncpy*/}

#ifdef DEBUG
printer(s)
char *s;
{
	while (*s) Cprnout(*s++);
/*end printer*/}
#endif DEBUG
