/*	SCCS Id: @(#)dlb.c	3.2	96/02/14	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "config.h"
#include "dlb.h"

#ifdef __DJGPP__
#include <string.h>
#endif

#ifdef DLB
/*
 * Data librarian.  Present a STDIO-like interface to NetHack while
 * multiplexing on one or more "data" libraries.  If a file is not found
 * in a given library, look for it outside the libraries.
 *
 * When initialized, we open all library files and read in their tables
 * of contents.  The library files stay open all the time.  When
 * a open is requested, the libraries' directories are searched.  If
 * successful, we return a descriptor that contains the library, file
 * size, and current file mark.  This descriptor is used for all
 * successive calls.
 *
 * The ability to open more than one library is supported but used
 * only in the Amiga port (the second library holds the sound files).
 * For Unix, the idea would be to split the NetHack library
 * into text and binary parts, where the text version could be shared.
 */


#define MAX_LIBS 4
static library dlb_libs[MAX_LIBS];
static boolean dlb_initialized = FALSE;

static boolean FDECL(readlibdir,(library *lp));
static boolean FDECL(find_file,(const char *name, library **lib, long *startp,
								long *sizep));

/* not static because shared with dlb_main.c */
boolean FDECL(open_library,(const char *lib_name, library *lp));
void FDECL(close_library,(library *lp));

/* without extern.h via hack.h, these haven't been declared for us */
extern char *FDECL(eos, (char *));
extern FILE *FDECL(fopen_datafile, (const char *,const char *));

/*
 * Read the directory out of the library.  Return 1 if successful,
 * 0 if it failed.
 *
 * NOTE: An improvement of the file structure should be the file
 * size as part of the directory entry or perhaps in place of the
 * offset -- the offset can be calculated by a running tally of
 * the sizes.
 *
 * Library file structure:
 *
 * HEADER:
 * %3ld	library FORMAT revision (currently rev 1)
 * %1c	space
 * %8ld	# of files in archive (includes 1 for directory)
 * %1c	space
 * %8ld	size of allocation for string space for directory names
 * %1c	space
 * %8ld	library offset - sanity check - lseek target for start of first file
 * %1c	space
 * %8ld	size - sanity check - byte size of complete archive file
 *
 * followed by one DIRECTORY entry for each file in the archive, including
 *  the directory itself:
 * %1c	handling information (compression, etc.)  Always ' ' in rev 1.
 * %s	file name
 * %1c	space
 * %8ld	offset in archive file of start of this file
 * %c	newline
 *
 * followed by the contents of the files
 */
#define DLB_MIN_VERS  1	/* min library version readable by this code */
#define DLB_MAX_VERS  1	/* max library version readable by this code */

/*
 * Read the directory from the library file.   This will allocate and
 * fill in our globals.  The file pointer is reset back to position
 * zero.  If any part fails, leave nothing that needs to be deallocated.
 *
 * Return TRUE on success, FALSE on failure.
 */
static boolean
readlibdir(lp)
    library *lp;	/* library pointer to fill in */
{
    int i;
    char *sp;
    long liboffset, totalsize;

    if (fscanf(lp->fdata, "%ld %ld %ld %ld %ld\n",
	    &lp->rev,&lp->nentries,&lp->strsize,&liboffset,&totalsize) != 5)
	return FALSE;
    if (lp->rev > DLB_MAX_VERS || lp->rev < DLB_MIN_VERS) return FALSE;

    lp->dir = (libdir *) alloc(lp->nentries * sizeof(libdir));
    lp->sspace = (char *) alloc(lp->strsize);

    /* read in each directory entry */
    for (i = 0, sp = lp->sspace; i < lp->nentries; i++) {
	lp->dir[i].fname = sp;
	if (fscanf(lp->fdata, "%c%s %ld\n",
			&lp->dir[i].handling, sp, &lp->dir[i].foffset) != 3) {
	    free((genericptr_t) lp->dir);
	    free((genericptr_t) lp->sspace);
	    lp->dir = (libdir *) 0;
	    lp->sspace = (char *) 0;
	    return FALSE;
	}
	sp = eos(sp) + 1;
    }

    /* calculate file sizes using offset information */
    for (i = 0; i < lp->nentries; i++) {
	if (i == lp->nentries - 1)
	    lp->dir[i].fsize = totalsize - lp->dir[i].foffset;
	else
	    lp->dir[i].fsize = lp->dir[i+1].foffset - lp->dir[i].foffset;
    }

    (void) fseek(lp->fdata, 0L, SEEK_SET);	/* reset back to zero */
    lp->fmark = 0;

    return TRUE;
}

/*
 * Look for the file in our directory structure.  Return 1 if successful,
 * 0 if not found.  Fill in the size and starting position.
 */
static boolean
find_file(name, lib, startp, sizep)
    const char *name;
    library **lib;
    long *startp, *sizep;
{
    int i, j;
    library *lp;

    for (i = 0; i < MAX_LIBS && dlb_libs[i].fdata; i++) {
	lp = &dlb_libs[i];
	for (j = 0; j < lp->nentries; j++) {
	    if (FILENAME_CMP(name, lp->dir[j].fname) == 0) {
		*lib = lp;
		*startp = lp->dir[j].foffset;
		*sizep = lp->dir[j].fsize;
		return TRUE;
	    }
	}
    }
    *lib = (library *) 0;
    *startp = *sizep = 0;
    return FALSE;
}

/*
 * Open the library of the given name and fill in the given library
 * structure.  Return TRUE if successful, FALSE otherwise.
 */
boolean
open_library(lib_name, lp)
    const char *lib_name;
    library *lp;
{
    boolean status = FALSE;

    lp->fdata = fopen_datafile(lib_name, RDBMODE);
    if (lp->fdata) {
	if (readlibdir(lp)) {
	    status = TRUE;
	} else {
	    (void) fclose(lp->fdata);
	    lp->fdata = (FILE *) 0;
	}
    }
    return status;
}

void
close_library(lp)
    library *lp;
{
    (void) fclose(lp->fdata);
    free((genericptr_t) lp->dir);
    free((genericptr_t) lp->sspace);

    (void) memset((char *)lp, 0, sizeof(library));
}

/*
 * Open the library file once using stdio.  Keep it open, but
 * keep track of the file position.
 */
boolean
dlb_init()
{
    if (dlb_initialized) return TRUE;

    /* zero out array */
    (void) memset((char *)&dlb_libs[0], 0, sizeof(dlb_libs));

    /* To open more than one library, add open library calls here. */
    if (!open_library(DLBFILE, &dlb_libs[0])) return dlb_initialized;
#ifdef DLBFILE2
    if (!open_library(DLBFILE2, &dlb_libs[1])) return dlb_initialized;
#endif

    dlb_initialized = TRUE;

    return dlb_initialized;
}

void
dlb_cleanup()
{
    int i;

    if (dlb_initialized) {
	/* close the data file(s) */
	for (i = 0; i < MAX_LIBS && dlb_libs[i].fdata; i++)
	    close_library(&dlb_libs[i]);

	dlb_initialized = FALSE;
    }
}

dlb *
dlb_fopen(name, mode)
    const char *name, *mode;
{
    long start, size;
    library *lp;
    FILE *fp;
    dlb *dp = (dlb *) 0;

    if (!dlb_initialized) return (dlb *) 0;

    /* look up file in directory */
    if (find_file(name, &lp, &start, &size)) {
	dp = (dlb *) alloc(sizeof(dlb));
	dp->fp = (FILE *) 0;
	dp->lib = lp;
	dp->start = start;
	dp->size = size;
	dp->mark = 0;
    } else if ((fp = fopen_datafile(name, mode)) != 0) {
	/* use an external file */
	dp = (dlb *) alloc(sizeof(dlb));
	dp->fp = fp;
	dp->lib = (library *) 0;
	dp->start = 0;
	dp->size = 0;
	dp->mark = 0;
    }
    return dp;
}

int
dlb_fclose(dp)
    dlb *dp;
{
    if (dlb_initialized) {
	if (dp->fp) (void) fclose(dp->fp);
	free((genericptr_t) dp);
	}
    return 0;
}

int
dlb_fread(buf, size, quan, dp)
    char *buf;
    int size, quan;
    dlb *dp;
{
    long pos, nread, nbytes;

    if (!dlb_initialized || size <= 0 || quan <= 0) return 0;
    if (dp->fp) return
#ifdef __SASC_60
	 (int)
#endif
	 fread(buf, size, quan, dp->fp);

    /* make sure we don't read into the next file */
    if ((dp->size - dp->mark) < (size * quan))
	quan = (dp->size - dp->mark) / size;
    if (quan == 0) return 0;

    pos = dp->start + dp->mark;
    if (dp->lib->fmark != pos) {
	fseek(dp->lib->fdata, pos, SEEK_SET);	/* check for error??? */
	dp->lib->fmark = pos;
    }

    nread = fread(buf, size, quan, dp->lib->fdata);
    nbytes = nread * size;
    dp->mark += nbytes;
    dp->lib->fmark += nbytes;

    return nread;
}

int
dlb_fseek(dp, pos, whence)
    dlb *dp;
    long pos;
    int whence;
{
    long curpos;

    if (!dlb_initialized) return EOF;
    if (dp->fp) return fseek(dp->fp, pos, whence);

    switch (whence) {
	case SEEK_CUR:	   curpos = dp->mark + pos;	break;
	case SEEK_END:	   curpos = dp->size - pos;	break;
	default: /* set */ curpos = pos;		break;
    }
    if (curpos < 0) curpos = 0;
    if (curpos > dp->size) curpos = dp->size;

    dp->mark = curpos;
    return 0;
}

char *
dlb_fgets(buf, len, dp)
    char *buf;
    int len;
    dlb *dp;
{
    int i;
    char *bp, c = 0;

    if (!dlb_initialized) return (char *) 0;
    if (dp->fp) return fgets(buf, len, dp->fp);

    if (len <= 0) return buf;	/* sanity check */

    /* return NULL on EOF */
    if (dp->mark >= dp->size) return (char *) 0;

    len--;	/* save room for null */
    for (i = 0, bp = buf;
		i < len && dp->mark < dp->size && c != '\n'; i++, bp++) {
	if (dlb_fread(bp, 1, 1, dp) <= 0) break;	/* EOF or error */
	c = *bp;
    }
    *bp = '\0';

    return buf;
}

int
dlb_fgetc(dp)
    dlb *dp;
{
    char c;

    if (!dlb_initialized) return EOF;
    if (dp->fp) return fgetc(dp->fp);

    if (dlb_fread(&c, 1, 1, dp) != 1) return EOF;
    return (int) c;
}

long
dlb_ftell(dp)
    dlb *dp;
{
    if (!dlb_initialized) return 0;
    if (dp->fp) return ftell(dp->fp);
    return dp->mark;
}

#endif /* DLB */

/*dlb.c*/
