$! yacc := bison/def			! .CLD version of bison
$! yacc := $bison$dir:bison -y -d	! non-CLD version of bison
$ yacc := $shell$exe:yacc -d		! yacc from DEC/Shell
$! lex := $flex$dir:flex		! flex
$ lex := $shell$exe:lex			! lex from DEC/Shell
$ cc = "CC/NOLIST/OPT=NOINLINE/DEB/INCL=[-.INCLUDE]/DEFI=(""WIZARD=""""GENTZEL"""""")"
$ link := link/nomap
$
$ yacc lev_comp.y
$ rename y_tab.c lev_comp.c
$ rename y_tab.h [-.include]lev_comp.h
$ lex lev_comp.l
$ rename lex_yy.c lev_lex.c
$ cc lev_comp.c
$ cc lev_lex.c
$ cc lev_main.c
$ link lev_comp.obj,lev_lex.obj,lev_main.obj,alloc.obj,monst.obj,objects.obj,panic.obj,vmsmisc.obj,sys$input:/opt
sys$share:vaxcrtl/share
$
$ old_dir = f$environment("DEFAULT")
$ set default [-.auxil]
$ mcr sys$disk:[-.src]lev_comp castle.des
$ mcr sys$disk:[-.src]lev_comp endgame.des
$ mcr sys$disk:[-.src]lev_comp tower.des
$ set default 'old_dir
