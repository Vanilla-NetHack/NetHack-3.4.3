/*	SCCS Id: @(#)dgn_main.c 3.1	92/03/10	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet	*/
/*	Copyright (c) 1990 by M. Stephenson		*/
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

#include "config.h"

#ifdef MICRO
# undef exit
# ifndef AMIGA
extern void FDECL(exit, (int));
# endif
#endif

#if defined(MICRO) && !defined(AMIGA)
# define WRMODE "w+b"
#else
# define WRMODE "w+"
#endif

#ifdef MAC
# ifdef applec
#  define MPWTOOL
#  include <CursorCtl.h>
# endif
#endif

#ifndef MPWTOOL
# define SpinCursor(x)
#endif

#define MAX_ERRORS	25

extern int line_number;
char *fname = "(stdin)";
int fatal_error = 0;

int  FDECL (main, (int, char **));
int  NDECL (yyparse);
void FDECL (yyerror, (char *));
void FDECL (yywarning, (char *));
int  NDECL (yywrap);
void FDECL (init_yyin, (FILE *));
void FDECL (init_yyout, (FILE *));

#ifdef AZTEC_36
FILE *FDECL (freopen, (char *, char *, FILE *));
#endif

int
main(argc, argv)
int argc;
char **argv;
{
	char	infile[64], outfile[64];
	FILE	*fin, *fout;
	int	i, len;
#ifdef MAC_THINKC5
	static char *mac_argv[] = {	"dgn_comp",	/* dummy argv[0] */
				":dat:dungeon.pdf"
				};

	argc = SIZE(mac_argv);
	argv = mac_argv;
#endif

	if (argc == 1) {	/* Read standard input */
	    init_yyin(stdin);
	    init_yyout(stdout);
	    yyparse();
	} else			/* Otherwise every argument is a filename */
	    for(i=1; i<argc; i++) {
		    fname = strcpy(infile, argv[i]);
		    len = strlen(fname) - 4;	/* length excluding suffix */
		    if (len < 0 || strncmp(".pdf", fname + len, 4)) {
			fprintf(stderr,
				"Error - file name \"%s\" in wrong format.\n",
				fname);
			continue;
		    }
#ifdef VMS	/* make sure to avoid possible interaction with logical name */
		    len++;	/* retain "." as trailing punctuation */
#endif
		    (void) strncpy(outfile, infile, len);
		    outfile[len] = '\0';

		    fin = freopen(infile, "r", stdin);
		    if (!fin) {
			fprintf(stderr,"Can't open %s for input\n", infile);
			perror(infile);
			continue;
		    }
		    fout = freopen(outfile, WRMODE, stdout);
		    if (!fout) {
			fprintf(stderr,"Can't open %s for output\n", outfile);
			perror(outfile);
			continue;
		    }
		    init_yyin(fin);
		    init_yyout(fout);
		    yyparse();
		    line_number = 1;
		    fatal_error = 0;
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

int yywrap()
{
	SpinCursor(3); /*	Don't know if this is a good place to put it ?
						Is it called for our grammar ? Often enough ?
						Too often ? -- h+ */
       return 1;
}

/*dgn_main.c*/
