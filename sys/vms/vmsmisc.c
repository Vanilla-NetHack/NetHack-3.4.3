/*	SCCS Id: @(#)vmsmisc.c	3.1	93/01/07	*/
/* NetHack may be freely redistributed.  See license for details. */

#include <ssdef.h>
#include <stsdef.h>

extern void exit( /*_ int _*/ );
extern void LIB$SIGNAL( /*_ unsigned long,... _*/ );

void
vms_exit(status)
int status;
{
    exit(status ? (SS$_ABORT | STS$M_INHIB_MSG) : SS$_NORMAL);
}

void
vms_abort()
{
    LIB$SIGNAL(SS$_DEBUG);
}

#ifndef __GNUC__
# ifndef bcopy
/* needed by gnutermcap.c, possibly by bison generated foo_yacc.c */
void
bcopy(src, dst, cnt)
const char *src;
char *dst;
unsigned cnt;
{
    while (cnt-- > 0) *dst++ = *src++;
}
# endif
#endif	/* !__GNUC__ */

#ifdef VERYOLD_VMS
#include "oldcrtl.c"        /* "[-.vms]oldcrtl.c" */
#endif

/*vmsmisc.c*/
