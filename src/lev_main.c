/*	SCCS Id: @(#)lev_main.c	3.0	89/07/02
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

/* #include "hack.h"	/* uncomment for the Mac */

#ifdef AMIGA
#include "hack.h"
#undef exit
#endif
#include <stdio.h>

#define MAX_ERRORS	25

extern int line_number;
char *fname = "(stdin)";
int fatal_error = 0;

#ifdef LATTICE
long *alloc(unsigned int);
#ifdef exit
#undef exit
#endif
#include <stdlib.h>
#endif

#ifdef	FDECL
int  FDECL (main, (int, char **));
int  NDECL (yyparse);
void FDECL (yyerror, (char *));
void FDECL (yywarning, (char *));
int  NDECL (yywrap);
#endif

#ifdef LSC
_main(argc, argv)
#else
main(argc, argv) 
#endif
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

	/* sub in the Nethack resource filename */
	strcpy((char *)name, "\010NH3.rsrc");
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
# ifdef THINKC4
	argc = ccommand(&argv);
# endif
#endif

	if (argc == 1)		/* Read standard input */
	    yyparse();
	else 			/* Otherwise every argument is a filename */
	    for(i=1; i<argc; i++) {
#if defined(VMS) || defined(AZTEC_C)
		    extern FILE *yyin;
		    yyin = fin = fopen(argv[i], "r");
#else
		    fin = freopen(argv[i], "r", stdin);
#endif
		    fname = argv[i];
		    if (!fin) 
			fprintf(stderr,"Can't open %s\n", argv[i]);
		    else
			yyparse();
		    line_number = 1;
		    fatal_error = 0;
	    }
	return 0;
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
