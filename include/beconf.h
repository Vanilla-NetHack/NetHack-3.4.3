/*	SCCS Id: @(#)beconf.h	3.2	96/05/22	*/
/* Copyright (c) Dean Luick 1996.	*/
/* NetHack may be freely redistributed.  See license for details. */

/* Configuration for Be Inc.'s BeBox */

#ifndef BECONF_H
#define BECONF_H

/*
 * This header works for BeOS 1.1d7
 *
 * We must use UNWIDENED_PROTOTYPES because we mix C++ and C.
 */

#define index strchr
#define rindex strrchr
#define Rand rand	/* Be should have a better rand function! */
#define tgetch getchar
#define FCMASK 0666
#define PORT_ID	"BeBox"
#define TEXTCOLOR
#define POSIX_TYPES
#define SIG_RET_TYPE __signal_func_ptr

#include <time.h>	/* for time_t */
#include <unistd.h>	/* for lseek() */

/* could go in extern.h, under bemain.c (or something..) */
void regularize(char *);


/* instead of including system.h... */
#define Sprintf	(void) sprintf
#define Strcat	(void) strcat
#define Strcpy	(void) strcpy
#define Vprintf (void) vprintf
#define Vfprintf (void) vfprintf
#define Vsprintf (void) vsprintf
#include <string.h>
#include <stdlib.h>

/* 
 * The following is copied from /boot/develop/headers/gnu/termcap.h.
 * The name of this system header conflicts with a header in the NetHack
 * source.  Sigh...
 */
extern int tgetent (char *buffer, const char *termtype);
extern int tgetnum (const char *name);
extern int tgetflag (const char *name);
extern char *tgetstr (const char *name, char **area);
extern void tputs (const char *string, int nlines, int (*outfun) (int));
extern char *tparam (const char *ctlstring, char *buffer, int size, ...);
extern char *tgoto (const char *cstring, int hpos, int vpos);
extern char PC;
extern short ospeed;
extern char *UP;
extern char *BC;


#endif /* BECONF_H */
