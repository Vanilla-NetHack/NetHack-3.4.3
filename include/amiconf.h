/*	SCCS Id: @(#)amiconf.h  3.0     89/09/04
/* NetHack may be freely redistributed.  See license for details. */

#ifndef AMICONF_H
#define AMICONF_H

#define MSDOS           /* must be defined to allow some inclusions */
#define AMIGA           /* and for some other inclusions */

#define O_BINARY        0
#define remove(x)       unlink(x)

#define DGK             /* You'll probably want this; define it in PCCONF.H */
#define RANDOM

#ifndef MSDOS_H
#include "msdos.h"
#endif
#ifndef PCCONF_H
#include "pcconf.h"     /* remainder of stuff is almost same as the PC */
#endif

#define memcpy(dest, source, size)  movmem(source, dest, size)

/*
 *  Configurable Amiga options:
 */

#define TEXTCOLOR              /* Use colored monsters and objects */
#define HACKFONT                /* Use special hack.font */
#define SHELL                   /* Have a shell escape command (!) */
#define MAIL                    /* Get mail at unexpected occasions */
#undef  TERMLIB
#define fopen       fopenp      /* Open most text files according to PATH */

#endif /* AMICONF_H /* */
