$ ! vms/spec_lev.com -- preprocess nethack's special level compiler code
$ !
$ ! This operation is optional.  If you are able to perform it and choose
$ ! to do so, you should do it prior to executing vmsbuild.com.
$ !
$
$ ! setup yacc/bison and lex/flex;
$ !	  (uncomment the alternatives appropriate for your site)
$     ! yacc := bison/def			!native bison (w/ DCL CLD)
$     ! yacc := $bison$dir:bison -y -d		!'foreign' bison (w/o CLD)
$	yacc := $shell$exe:yacc -d		!yacc from DECshell
$     ! lex  := $flex$dir:flex			!flex
$	lex  := $shell$exe:lex			!lex from DECshell
$ ! additional setup
$	rename := rename/new_vers
$	! start from a known location -- [.vms], then move to [-.src]
$	cur_dir = f$environment("DEFAULT")
$	set default 'f$parse(f$environment("PROCEDURE"),,,"DIRECTORY")'
$	set default [-.src]	!move to source directory
$
$ ! process lev_comp.y into lev_comp.c and ../include/lev_comp.h
$  yacc lev_comp.y
$  rename y_tab.c lev_comp.c
$  rename y_tab.h [-.include]lev_comp.h
$
$ ! process lev_comp.l into lev_lex.c
$  lex lev_comp.l
$  rename lex_yy.c lev_lex.c
$
$ ! done
$  set default 'cur_dir'
$ exit
