/*	SCCS Id: @(#)lev_main.c	3.0	89/07/02
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

#include <stdio.h>

#define MAX_ERRORS	25

extern int line_number;
char *fname = "(stdin)";
int fatal_error = 0;

main(argc, argv) 
int argc;
char **argv;
{
	FILE *fin;
	int i;

	if (argc == 1)		/* Read standard input */
	    yyparse();
	else 			/* Otherwise every argument is a filename */
	    for(i=1; i<argc; i++) {
#ifdef VMS
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

yyerror(s)
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

yywarning(s)
char *s;
{
	fprintf(stderr,"%s : line %d : WARNING : %s\n",fname,line_number,s);
}

yywrap()
{
       return 1;
}
