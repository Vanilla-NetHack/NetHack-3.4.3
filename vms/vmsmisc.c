/*	SCCS Id: @(#)vmsmisc.c	3.0	90/22/02
/* NetHack may be freely redistributed.  See license for details. */

#include <ssdef.h>
#include <stsdef.h>

void
vms_exit(status)
int status;
{
    exit(status ? (SS$_ABORT | STS$M_INHIB_MSG) : SS$_NORMAL);
}

void
vms_abort()
{
    (void) LIB$SIGNAL(SS$_DEBUG);
}

#ifdef VERYOLD_VMS
#include "oldcrtl.c"        /* "[-.vms]oldcrtl.c" */
#endif
