/*    SCCS Id: @(#)arg.h		3.1   93/01/08
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * arg.h
 * external interface for argument parsing
 */

int arg_next(void);
void arg_init(char *,int, char **);

extern char *argarg;

#define ARG_ERROR	(-2)	/* no such arg, bad file, etc. */
#define ARG_DONE	(-1)	/* ok, but nothing left */
#define ARG_FREE	0	/* a name not associated with an arg */
