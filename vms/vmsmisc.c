#include <ssdef.h>

void
vms_exit(status)
int status;
{
    exit(status ? SS$_ABORT : SS$_NORMAL);
}

void
vms_abort()
{
    (void) LIB$SIGNAL(SS$_DEBUG);
}
