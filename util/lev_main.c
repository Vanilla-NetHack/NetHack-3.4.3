/*	SCCS Id: @(#)lev_main.c	3.0	89/07/02
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

#include "hack.h"

#ifdef MSDOS
# undef exit
# ifndef AMIGA
extern void FDECL(exit, (int));
# endif
#endif
#ifdef LATTICE
long *alloc(unsigned int);
# ifdef exit
#  undef exit
# endif
#include <stdlib.h>
#endif

#define MAX_ERRORS	25

extern int line_number;
char *fname = "(stdin)";
int fatal_error = 0;

/* Flex 2.3 bug work around */
int yy_more_len = 0;

int  FDECL (main, (int, char **));
int  NDECL (yyparse);
void FDECL (yyerror, (char *));
void FDECL (yywarning, (char *));
int  NDECL (yywrap);
void FDECL (init_yyin, (FILE *));
void FDECL (init_yyout, (FILE *));

#ifdef LSC
# define main _main
#endif
main(argc, argv)
int argc;
char **argv;
{
	FILE *fin;
	int i;

#if defined(MACOS) && defined(SMALLDATA)
# ifdef THINKC4
#include <console.h>
# endif
#define YYLMAX	2048
	extern char	*yysbuf, *yytext, *yysptr;
	Handle temp;
	Str255 name;
	long	j;
	extern struct permonst *mons;
	extern struct objclass *objects;
	/* 3 special level description files */
	char *descrip[] = {"lev_comp", ":auxil:castle.des",
			":auxil:endgame.des", ":auxil:tower.des"};

	/* sub in the Nethack resource filename */
	Strcpy((char *)name, "\021nethack.proj.rsrc");
	yysbuf = (char *)alloc(YYLMAX);
	yysptr = yysbuf;
	yytext = (char *)alloc(YYLMAX);

	(void)OpenResFile(name);
	temp = GetResource(HACK_DATA, MONST_DATA);
	if (temp) {
		DetachResource(temp);
		MoveHHi(temp);
		HLock(temp);
		i = GetHandleSize(temp);
		mons = (struct permonst *)(*temp);
	} else {
		panic("Can't get MONST resource data.");
	}
	
	temp = GetResource(HACK_DATA, OBJECT_DATA);
	if (temp) {
		DetachResource(temp);
		MoveHHi(temp);
		HLock(temp);
		i = GetHandleSize(temp);
		objects = (struct objclass *)(*temp);
		for (j = 0; j< NROFOBJECTS+1; j++) {
			objects[j].oc_name = sm_obj[j].oc_name;
			objects[j].oc_descr = sm_obj[j].oc_descr;
		}
	} else {
		panic("Can't get OBJECT resource data.");
	}
    argc = 4;    /* argv[0] is irrelevant, argv[i] = descrip[i] */
    argv = descrip;
#endif  /* !MACOS || !SMALLDATA */

	init_yyout(stdout);
	if (argc == 1) {		/* Read standard input */
	    init_yyin(stdin);
	    yyparse();
	} else {			/* Otherwise every argument is a filename */
	    for(i=1; i<argc; i++) {
		    fname = argv[i];
#ifdef MACOS
		    fprintf(stdout, "Working on %s\n", fname);
#endif
		    fin = freopen(fname, "r", stdin);
		    if (!fin) {
			fprintf(stderr,"Can't open \"%s\" for input.\n", fname);
			perror(fname);
		    } else {
			init_yyin(fin);
			yyparse();
		    }
		    line_number = 1;
		    fatal_error = 0;
	    }
	}
#ifndef VMS
	return 0;
#else
	return 1;       /* vms success */
#endif /*VMS*/
}

/* 
 * Each time the parser detects an error, it uses this function.
 * Here we take count of the errors. To continue farther than
 * MAX_ERRORS wouldn't be reasonable.
 */

void yyerror(s)
char *s;
{
	fprintf(stderr,"%s : line %d : %s\n",fname,line_number, s);
	if (++fatal_error > MAX_ERRORS) {
		fprintf(stderr,"Too many errors, good bye!\n");
		exit(1);
	}
}

/* 
 * Just display a warning (that is : a non fatal error)
 */

void yywarning(s)
char *s;
{
	fprintf(stderr,"%s : line %d : WARNING : %s\n",fname,line_number,s);
}

yywrap()
{
       return 1;
}
