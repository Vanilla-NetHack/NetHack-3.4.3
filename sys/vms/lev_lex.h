/*	SCCS Id: @(#)lev_lex.h	3.2	93/02/07	*/
/* "vms/lev_lex.h" copied into "util/stdio.h" for use by lev_lex.c only!
 * This is an awful kludge to allow util/lev_lex.c made by SunOS's `lex'
 * to be compiled as is.  (It isn't needed with `flex' or VMS POSIX `lex'.)
 * It works because the actual setup of yyin & yyout is performed in
 * src/lev_main.c, where stdin & stdout are still correctly defined.
 */
/* note for 3.1 and 3.2: also used with util/dgn_lex.c */

#include <stdio.h>
#ifdef stdin
# undef stdin
#endif
#define stdin  0
#ifdef stdout
# undef stdout
#endif
#define stdout 0
