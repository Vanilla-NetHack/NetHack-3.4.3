/*	SCCS Id: @(#)version.c	3.2	96/05/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "date.h"
/*
 * All the references to the contents of patchlevel.h have been moved
 * into makedefs....
 */
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif

/* fill and return the given buffer with the nethack version string */
char *
getversionstring(buf)
char *buf;
{
	return strcpy(buf, VERSION_ID);
}

int
doversion()
{
	char buf[BUFSZ];

	pline(getversionstring(buf));
	return 0;
}

int
doextversion()
{
	display_file(OPTIONS_USED, TRUE);
	return 0;
}

#ifdef MICRO
boolean
comp_times(filetime)
long filetime;
{
	return((boolean)(filetime < BUILD_TIME));
}
#endif

boolean
check_version(version_info, filename, complain)
unsigned long *version_info;
const char *filename;
boolean complain;
{
	if (
#ifdef VERSION_COMPATIBILITY
	    version_info[0] < VERSION_COMPATIBILITY ||
	    version_info[0] > VERSION_NUMBER
#else
	    version_info[0] != VERSION_NUMBER
#endif
	  ) {
	    if (complain)
		pline("Version mismatch for file \"%s\".", filename);
	    return FALSE;
	} else if (version_info[1] != VERSION_FEATURES ||
		   version_info[2] != VERSION_SANITY1 ||
		   version_info[3] != VERSION_SANITY2) {
	    if (complain)
		pline("Configuration incompatability for file \"%s\".",
		      filename);
	    return FALSE;
	}
	return TRUE;
}

/* this used to be based on file date and somewhat OS-dependant,
   but now examines the initial part of the file's contents */
boolean
uptodate(fd, name)
int fd;
const char *name;
{
	unsigned long vers_info[4];
	boolean verbose = name ? TRUE : FALSE;

	(void) read(fd, (genericptr_t) vers_info, sizeof vers_info);
	minit();	/* ZEROCOMP */
	if (!check_version(vers_info, name, verbose)) {
		if (verbose) wait_synch();
		return FALSE;
	}
	return TRUE;
}

void
store_version(fd)
int fd;
{
	static unsigned long version_info[4] = {
			VERSION_NUMBER, VERSION_FEATURES,
			VERSION_SANITY1, VERSION_SANITY2
	};

	bufoff(fd);
	/* bwrite() before bufon() uses plain write() */
	bwrite(fd, (genericptr_t)version_info, (unsigned)(sizeof version_info));
	bufon(fd);
	return;
}

#ifdef AMIGA
const char amiga_version_string[] = AMIGA_VERSION_STRING;
#endif

/*version.c*/
