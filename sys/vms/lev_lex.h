/*       SCCS Id: @(#)lev_lex.h 3.1     92/01/10        */
/* "vms/lev_lex.h" copied into "util/stdio.h" for use by lev_lex.c only!
 * This is an awful kludge to allow util/lev_lex.c to be compiled as is.
 * It works because the actual setup of yyin & yyout is performed in
 * src/lev_main.c, where stdin & stdout are still correctly defined.
 */
/* note for 3.1: also used with util/dgn_lex.c */

#ifdef __GNUC__
# ifndef CONST_OK
#  define const
# endif
#endif

#include <stdio.h>
#ifdef stdin
# undef stdin
#endif
#define stdin  0
#ifdef stdout
# undef stdout
#endif
#define stdout 0
