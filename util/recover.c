/*	SCCS Id: @(#)recover.c	3.1	93/05/15	*/
/*	Copyright (c) Janet Walz, 1992.				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Utility for reconstructing NetHack save file from a set of individual
 *  level files.  Requires that the `checkpoint' option be enabled at the
 *  time NetHack creates those level files.
 */
#include "config.h"
#if !defined(O_WRONLY) && !defined(LSC) && !defined(AZTEC_C)
#include <fcntl.h>
#endif

#ifndef VMS
# ifdef exit
#  undef exit
# endif
#ifdef MICRO
extern void FDECL(exit, (int));
#endif
#else	/* VMS */
extern int FDECL(vms_creat, (const char *,unsigned));
extern int FDECL(vms_open, (const char *,int,unsigned));
#endif	/* VMS */

int FDECL(restore_savefile, (char *));
void FDECL(set_levelfile_name, (int));
int FDECL(open_levelfile, (int));
int NDECL(create_savefile);
void FDECL(copy_bytes, (int,int));

#ifdef UNIX
#define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */
#else
# ifdef VMS
#define SAVESIZE	(PL_NSIZ + 22)	/* [.save]<uid>player.e;1 */
# else
#define SAVESIZE	FILENAME	/* from macconf.h or pcconf.h */
# endif
#endif

char savename[SAVESIZE]; /* holds relative path of save file from playground */


int
main(argc, argv)
int argc;
char *argv[];
{
	int argno;
	const char *dir = (char *)0;
#ifdef AMIGA
	char *startdir = (char *)0;
#endif

	if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-"))) {
		(void) fprintf(stderr,
				"Usage: %s [-d directory] base1 base2 ...\n",
				argv[0]);
		exit(1);
	}

	argno = 1;
	if (!strncmp(argv[argno], "-d", 2)) {
		dir = argv[argno]+2;
		if (*dir == '=' || *dir == ':') dir++;
		if (!*dir && argc > argno) {
			argno++;
			dir = argv[argno];
		}
		if (!*dir) {
		    (void) fprintf(stderr,
			"%s: flag -d must be followed by a directory name.\n",
			argv[0]);
		    exit(1);
		}
		argno++;
	}

	if (!dir) dir = getenv("NETHACKDIR");
	if (!dir) dir = getenv("HACKDIR");
#if defined(SECURE) && !defined(VMS)
	if (dir
# ifdef HACKDIR
		&& strcmp(dir, HACKDIR)
# endif
		) {
		(void) setgid(getgid());
		(void) setuid(getuid());
	}
#endif	/* SECURE && !VMS */

#ifdef HACKDIR
	if (!dir) dir = HACKDIR;
#endif

#ifdef AMIGA
	startdir = getcwd(0,255);
#endif
	if (dir && chdir((char *) dir) < 0) {
		(void) fprintf(stderr, "%s: cannot chdir to %s.\n",
				argv[0], dir);
		exit(1);
	}

	while (argc > argno) {
		(void) restore_savefile(argv[argno]);
		argno++;
	}
#ifdef AMIGA
	if (startdir) (void)chdir(startdir);
#endif
#ifndef VMS
	return 0;
#else
	return 1;       /* vms success */
#endif /*VMS*/
}

static char lock[256];

void
set_levelfile_name(lev)
int lev;
{
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) {
		tf = lock;
		while (*tf) tf++;
	}
#ifdef VMS
	(void) sprintf(tf, ".%d;1", lev);
#else
	(void) sprintf(tf, ".%d", lev);
#endif
}

int
open_levelfile(lev)
int lev;
{
	int fd;

	set_levelfile_name(lev);
#ifdef MICRO
	fd = open(lock, O_RDONLY | O_BINARY);
#else
	fd = open(lock, O_RDONLY, 0);
#endif
	return fd;
}

int
create_savefile()
{
	int fd;

#ifdef MICRO
	fd = open(savename, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
	fd = creat(savename, FCMASK);
#endif
	return fd;
}

void
copy_bytes(ifd, ofd)
int ifd, ofd;
{
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) {
			(void) fprintf(stderr, "file copy failed!\n");
			exit(1);
		}
	} while (nfrom == BUFSIZ);
}

int
restore_savefile(basename)
char *basename;
{
	int gfd, lfd, sfd;
	int lev, savelev, hpid;
	xchar levc;

	/* level 0 file contains:
	 *	pid of creating process (ignored here)
	 *	level number for current level of save file
	 *	name of save file nethack would have created
	 *	and game state
	 */
	(void) strcpy(lock, basename);
	gfd = open_levelfile(0);
	if (gfd < 0) {
	    (void) fprintf(stderr, "Cannot open level 0 for %s.\n", basename);
	    return(-1);
	}
	(void) read(gfd, (genericptr_t) &hpid, sizeof(hpid));
	if (read(gfd, (genericptr_t) &savelev, sizeof(savelev))
							!= sizeof(savelev)) {
	    (void) fprintf(stderr,
	    "Checkpointing was not in effect for %s -- recovery impossible.\n",
		basename);
	    (void) close(gfd);
	    return(-1);
	}
	(void) read(gfd, (genericptr_t) savename, sizeof(savename));

	/* save file should contain:
	 *	current level (including pets)
	 *	(non-level-based) game state
	 *	other levels
	 */
	sfd = create_savefile();
	if (sfd < 0) {
	    (void) fprintf(stderr, "Cannot create savefile %s.\n", savename);
	    (void) close(gfd);
	    return(-1);
	}

	lfd = open_levelfile(savelev);
	if (lfd < 0) {
	    (void) fprintf(stderr, "Cannot open level of save for %s.\n",
				basename);
	    (void) close(gfd);
	    (void) close(sfd);
	    return(-1);
	}

	copy_bytes(lfd, sfd);
	(void) close(lfd);
	(void) unlink(lock);

	copy_bytes(gfd, sfd);
	(void) close(gfd);
	set_levelfile_name(0);
	(void) unlink(lock);

	for (lev = 1; lev < 256; lev++) {
		/* level numbers are kept in xchars in save.c, so the
		 * maximum level number (for the endlevel) must be < 256
		 */
		if (lev != savelev) {
			lfd = open_levelfile(lev);
			if (lfd >= 0) {
				/* any or all of these may not exist */
				levc = (xchar) lev;
				write(sfd, (genericptr_t) &levc, sizeof(levc));
				copy_bytes(lfd, sfd);
				(void) close(lfd);
				(void) unlink(lock);
			}
		}
	}

	(void) close(sfd);

#ifdef AMIGA
			/* we need to create an icon for the saved game
			 * or HackWB won't notice the file.
			 */
	{
	char iconfile[FILENAME];
	int in, out;

	sprintf(iconfile,"%s.info",savename);
	in=open("NetHack:default.icon",O_RDONLY);
	out=open(iconfile,O_WRONLY | O_TRUNC | O_CREAT);
	if(in > -1 && out > -1){
		copy_bytes(in,out);
	}
	if(in > -1)close(in);
	if(out > -1)close(out);
	}
#endif
	return(0);
}

/*recover.c*/
