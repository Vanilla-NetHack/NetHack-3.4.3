/*       SCCS Id: @(#)lev_lex.h   3.0      90/05/24
/* "vms/lev_lex.h" copied into "src/stdio.h" for use by lev_lex.c only!
 * This is an awful kludge to allow src/lev_lex.c to be compiled as is.
 * It works because the actual setup of yyin & yyout is performed in
 * src/lev_main.c, where stdin & stdout are still correctly defined.
 */
#ifdef VAXC
# module lev_lex "3.0.10"
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
