/*	SCCS Id: @(#)vmsfiles.c	3.1	93/01/20	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  VMS-specific file manipulation routines to implement some missing
 *  routines or substitute for ones where we want behavior modification.
 */
#include "config.h"

#include <rms.h>
#if 0
#include <psldef.h>
#else
#define PSL$C_EXEC 1	/* executive mode, for priv'd logical name handling */
#endif
#ifndef C$$TRANSLATE	/* don't rely on VAXCRTL's internal routine */
#include <errno.h>
#define C$$TRANSLATE(status) (errno = EVMSERR,  vaxc$errno = (status))
#else
int FDECL(c__translate, (int));
#endif
extern unsigned long SYS$PARSE(), SYS$SEARCH(), SYS$ENTER(), SYS$REMOVE();

#define vms_success(sts) ((sts)&1)		/* odd, */
#define vms_failure(sts) (!vms_success(sts))	/* even */

/* vms_link() -- create an additional directory for an existing file */
int vms_link(file, new)
const char *file, *new;
{
    struct FAB fab;
    struct NAM nam;
    unsigned short fid[3];
    char esa[NAM$C_MAXRSS];

    fab = cc$rms_fab;	/* set block ID and length, zero the rest */
    fab.fab$l_fop = FAB$M_OFP;
    fab.fab$l_fna = (char *) file;
    fab.fab$b_fns = strlen(file);
    fab.fab$l_nam = &nam;
    nam = cc$rms_nam;
    nam.nam$l_esa = esa;
    nam.nam$b_ess = sizeof esa;

    if (vms_success(SYS$PARSE(&fab)) && vms_success(SYS$SEARCH(&fab))) {
	fid[0] = nam.nam$w_fid[0];
	fid[1] = nam.nam$w_fid[1];
	fid[2] = nam.nam$w_fid[2];
	fab.fab$l_fna = (char *) new;
	fab.fab$b_fns = strlen(new);

	if (vms_success(SYS$PARSE(&fab))) {
	    nam.nam$w_fid[0] = fid[0];
	    nam.nam$w_fid[1] = fid[1];
	    nam.nam$w_fid[2] = fid[2];
	    nam.nam$l_esa = nam.nam$l_name;
	    nam.nam$b_esl = nam.nam$b_name + nam.nam$b_type + nam.nam$b_ver;

	    (void) SYS$ENTER(&fab);
	}
    }

    if (vms_failure(fab.fab$l_sts)) {
	C$$TRANSLATE(fab.fab$l_sts);
	return -1;
    }
    return 0;	/* success */
}

/*
   vms_unlink() -- remove a directory entry for a file; should only be used
   for files which have had extra directory entries added, not for deletion
   (because the file won't be deleted, just made inaccessible!).
 */
int vms_unlink(file)
const char *file;
{
    struct FAB fab;
    struct NAM nam;
    char esa[NAM$C_MAXRSS];

    fab = cc$rms_fab;	/* set block ID and length, zero the rest */
    fab.fab$l_fop = FAB$M_DLT;
    fab.fab$l_fna = (char *) file;
    fab.fab$b_fns = strlen(file);
    fab.fab$l_nam = &nam;
    nam = cc$rms_nam;
    nam.nam$l_esa = esa;
    nam.nam$b_ess = sizeof esa;

    if (vms_failure(SYS$PARSE(&fab)) || vms_failure(SYS$REMOVE(&fab))) {
	C$$TRANSLATE(fab.fab$l_sts);
	return -1;
    }
    return 0;
}

/*
   Substitute creat() routine -- if trying to create a specific version,
   explicitly remove an existing file of the same name.  Since it's only
   used when we expect exclusive access, add a couple RMS options for
   optimization.  (Don't allow sharing--eliminates coordination overhead,
   and use 32 block buffer for faster throughput; ~30% speedup measured.)
 */
#undef creat
int vms_creat(file, mode)
const char *file;
unsigned int mode;
{
    if (index(file, ';'))
	(void) unlink(file);	/* assumes remove or delete, not vms_unlink */
    return creat(file, mode, "shr=nil", "mbc=32");
}

/*
   Similar substitute for open() -- can't disallow sharing, because we're
   relying on deleting a file that we've got open, so must share it with
   ourself!
 */
#undef open
int vms_open(file, flags, mode)
const char *file;
int flags;
unsigned int mode;
{
    return open(file, flags, mode, "mbc=32");
}

/*
   Determine whether two strings contain the same directory name.
   Used for deciding whether installed privileges should be disabled
   when HACKDIR is defined in the environment (or specified via -d on
   the command line).  This version doesn't handle Unix-style file specs.
 */
boolean
same_dir(d1, d2)
const char *d1, *d2;
{
    if (!d1 || !*d1 || !d2 || !*d2)
	return FALSE;
    else if (!strcmp(d1, d2))	/* strcmpi() would be better, but that leads */
	return TRUE;		/* to linking problems for the utilities */
    else {
	struct FAB f1, f2;
	struct NAM n1, n2;

	f1 = f2 = cc$rms_fab;	/* initialize file access block */
	n1 = n2 = cc$rms_nam;	/* initialize name block */
	f1.fab$b_acmodes = PSL$C_EXEC << FAB$V_LNM_MODE;
	f1.fab$b_fns = strlen( f1.fab$l_fna = (char *)d1 );
	f2.fab$b_fns = strlen( f2.fab$l_fna = (char *)d2 );
	f1.fab$l_nam = (genericptr_t)&n1;	/* link nam to fab */
	f2.fab$l_nam = (genericptr_t)&n2;
	n1.nam$b_nop = n2.nam$b_nop = NAM$M_NOCONCEAL; /* want true device name */

	return (vms_success(SYS$PARSE(&f1)) && vms_success(SYS$PARSE(&f2))
	     && n1.nam$t_dvi[0] == n2.nam$t_dvi[0]
	     && !strncmp(&n1.nam$t_dvi[1], &n2.nam$t_dvi[1], n1.nam$t_dvi[0])
	     && !memcmp((genericptr_t)n1.nam$w_did,
			(genericptr_t)n2.nam$w_did,
			sizeof n1.nam$w_did));	/*{ short nam$w_did[3]; }*/
    }
}


/*
 * c__translate -- substitute for VAXCRTL routine C$$TRANSLATE.
 *
 *	Try to convert a VMS status code into its Unix equivalent,
 *	then set `errno' to that value; use EVMSERR if there's no
 *	appropriate translation; set `vaxc$errno' to the original
 *	status code regardless.
 *
 *	These translations match only a subset of VAXCRTL's lookup
 *	table, but work even if the severity has been adjusted or
 *	the inhibit-message bit has been set.
 */
#include <errno.h>
#include <ssdef.h>
#include <rmsdef.h>
/* #include <libdef.h> */
/* #include <mthdef.h> */

#define VALUE(U)	trans = U; break
#define CASE1(V)	case (V >> 3)
#define CASE2(V,W)	CASE1(V): CASE1(W)

int c__translate(code)
    int code;
{
    register int trans;

    switch ((code & 0x0FFFFFF8) >> 3) {	/* strip upper 4 and bottom 3 bits */
	CASE2(RMS$_PRV,SS$_NOPRIV):
				VALUE(EPERM);	/* not owner */
	CASE2(RMS$_DNF,RMS$_DIR):
	CASE2(RMS$_FNF,RMS$_FND):
	CASE1(SS$_NOSUCHFILE):
				VALUE(ENOENT);	/* no such file or directory */
	CASE2(RMS$_IFI,RMS$_ISI):
				VALUE(EIO);	/* i/o error */
	CASE1(RMS$_DEV):
	CASE2(SS$_NOSUCHDEV,SS$_DEVNOTMOUNT):
				VALUE(ENXIO);	/* no such device or address codes */
	CASE1(RMS$_DME):
     /* CASE1(LIB$INSVIRMEM): */
	CASE2(SS$_VASFULL,SS$_INSFWSL):
				VALUE(ENOMEM);	/* not enough core */
	CASE1(SS$_ACCVIO):
				VALUE(EFAULT);	/* bad address */
	CASE2(RMS$_DNR,SS$_DEVASSIGN):
	CASE2(SS$_DEVALLOC,SS$_DEVALRALLOC):
	CASE2(SS$_DEVMOUNT,SS$_DEVACTIVE):
				VALUE(EBUSY);	/* mount device busy codes to name a few */
	CASE2(RMS$_FEX,SS$_FILALRACC):
				VALUE(EEXIST);	/* file exists */
	CASE2(RMS$_IDR,SS$_BADIRECTORY):
				VALUE(ENOTDIR);	/* not a directory */
	CASE1(SS$_NOIOCHAN):
				VALUE(EMFILE);	/* too many open files */
	CASE1(RMS$_FUL):
	CASE2(SS$_DEVICEFULL,SS$_EXDISKQUOTA):
				VALUE(ENOSPC);	/* no space left on disk codes */
	CASE2(RMS$_WLK,SS$_WRITLCK):
				VALUE(EROFS);	/* read-only file system */
	default:
				VALUE(EVMSERR);
    };

    errno = trans;
    vaxc$errno = code;
    return code;	/* (not very useful) */
}

#undef VALUE
#undef CASE1
#undef CASE2

/*vmsfiles.c*/
