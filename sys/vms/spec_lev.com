$ ! sys/vms/spec_lev.com -- preprocess nethack's special level compiler code
$ !
$ ! This operation needs to be performed prior to executing vmsbuild.com.
$ ! Process the scanning and parsing code for NetHack's special level
$ ! and dungeon compilers.  *.l and *.y are converted into *'.c and *.h.
$ !
$
$ ! setup yacc/bison and lex/flex;
$ !	  (Uncomment the alternatives appropriate for your site;
$ !	   if yacc and lex are not defined, the pre-processed files
$ !	   distributed in sys/share will be copied and used.)
$     ! yacc := bison /Define			!native bison (w/ DCL CLD)
$     ! yacc := $bison$dir:bison -y -d		!'foreign' bison (w/o CLD)
$     ! yacc := posix /Run/Input=nl: posix$bin:yacc. """-d 
$     ! yacc := $shell$exe:yacc -d		!yacc from DECshell
$     ! lex  := $flex$dir:flex			!flex
$     ! lex  := posix /Run/Input=nl: posix$bin:lex. """
$     ! lex  := $shell$exe:lex
$ !	  (Nothing below this line should need to be changed.)
$ ! additional setup
$	rename	:= rename/New_Vers
$	mung	:= call mung	! not to be confused with teco :-)
$	delete	:= delete/noConfirm
$	copy	:= copy/noConcat
$	! start from a known location -- [.sys.vms], then move to [-.-.util]
$	cur_dir = f$environment("DEFAULT")
$	set default 'f$parse(f$environment("PROCEDURE"),,,"DIRECTORY")'
$	set default [-.-.util]	!move to utility directory
$
$mung: subroutine
$ ! kludge to strip bogus #module directives from POSIX-processed files
$ !   in lieu of $ rename 'p1' 'p2'
$	search/Exact/Match=NOR 'p1' "#module" /Output='p2'
$	delete 'p1';*
$ endsubroutine !mung
$
$ ! first some cleanup (to keep things safe for unknown yacc vs bison)
$  if f$search("y_tab.%").nes."" then  delete y_tab.%;* !old yacc
$  if f$search("ytab.%") .nes."" then  delete ytab.%;*	!VMS POSIX yacc
$
$ ! process lev_comp.y into lev_yacc.c and ../include/lev_comp.h
$ if f$type(yacc).eqs."STRING"
$ then
$  yacc lev_comp.y
$  if f$search("y_tab.%").nes."" then  rename y_tab.% lev_comp.*
$  if f$search("ytab.%") .nes."" then  rename ytab.% lev_comp.*
$ else		! use preprocessed files
$  copy [-.sys.share]lev_yacc.c,lev_comp.h []lev_comp.*
$ endif
$  mung   lev_comp.c lev_yacc.c
$  rename lev_comp.h [-.include]lev_comp.h
$
$ ! process lev_comp.l into lev_lex.c
$ if f$type(lex).eqs."STRING"
$ then
$  lex lev_comp.l
$ else		! use preprocessed file
$  copy [-.sys.share]lev_lex.c []lex_yy.*
$ endif
$  mung   lex_yy.c lev_lex.c
$
$ ! process dgn_comp.y into dgn_yacc.c and ../include/dgn_comp.h
$ if f$type(yacc).eqs."STRING"
$ then
$  yacc dgn_comp.y
$  if f$search("y_tab.%").nes."" then  rename y_tab.% dgn_comp.*
$  if f$search("ytab.%") .nes."" then  rename ytab.% dgn_comp.*
$ else
$  copy [-.sys.share]dgn_yacc.c,dgn_comp.h []dgn_comp.*
$ endif
$  mung   dgn_comp.c dgn_yacc.c
$  rename dgn_comp.h [-.include]dgn_comp.h
$
$ ! process dgn_comp.l into dgn_lex.c
$ if f$type(lex).eqs."STRING"
$ then
$  lex dgn_comp.l
$ else
$  copy [-.sys.share]dgn_lex.c []lex_yy.*
$ endif
$  mung   lex_yy.c dgn_lex.c
$
$ ! done
$  set default 'cur_dir'
$ exit
