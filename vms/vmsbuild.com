$ ! vms/vmsbuild.com -- compile and link NetHack 3.0 patchlevel 10	[pr]
$ !
$ ! usage:
$ !   $ set default [.src]	!or [-.src] if starting from [.vms]
$ !   $ @[-.vms]vmsbuild  [compiler-option]  [link-option]  [cc-switches]
$ ! options:
$ !	compiler-option :  either "VAXC" or "GNUC" or ""	!default VAXC
$ !	link-option	:  either "SHARE[able]" or "LIB[rary]"	!default SHARE
$ !	cc-switches	:  optional qualifiers for CC (such as "/noOpt/Debug")
$ ! notes:
$ !	If the symbol "CC" is defined, compiler-option is not used.
$ !	The link-option refers to VAXCRTL (C Run-Time Library) handling;
$ !	  to specify it while letting compiler-option default, use "" as
$ !	  the compiler-option.
$ !	To re-link without compiling, use "LINK" as special 'compiler-option';
$ !	  to re-link with GNUC library, 'CC' must begin with "G" (or "g").
$ !	Default wizard definition moved to include/vmsconf.h.
$
$	vaxc_ = "CC/NOLIST/OPTIMIZE=NOINLINE"	    !vaxc v3.x (2.x fixed below)
$	gnuc_ = "GCC"
$	gnulib = "gnu_cc:[000000]gcclib/Library"    !(not used w/ vaxc)
$ ! common CC options (/obj=file doesn't work for GCC 1.36, use rename instead)
$	c_c_  = "/INCLUDE=[-.INCLUDE]"	!/DEFINE=(""WIZARD=""""GENTZEL"""""")
$	if f$extract(1,3,f$getsyi("VERSION")).lts."4.6" then -
$		c_c_ = c_c_ + "/DEFINE=(""VERYOLD_VMS"")"
$ ! miscellaneous setup
$	ivqual = %x00038240	!DCL-W-IVQUAL (used to check for ancient vaxc)
$	abort := exit %x1000002A
$ ! validate first parameter
$	p1 := 'p1'
$	c_opt = f$locate("|"+p1, "|VAXC|GNUC|LINK|SPECIAL|") !5
$     if (c_opt/5)*5 .eq. c_opt then  goto p1_ok
$	copy sys$input: sys$error:	!p1 usage
%first arg is compiler option; it must be one of
       "VAXC" -- use VAX C to compile everything
   or  "GNUC" -- use GNU C to compile everything
   or  "LINK" -- skip compilation, just relink nethack.exe
   or  "SPEC[IAL]" -- just compile and link lev_comp.exe
   or    ""   -- default operation (VAXC unless 'CC' is defined)

Note: if a DCL symbol for CC is defined, "VAXC" and "GNUC" are no-ops.
      If the symbol value begins with "G" (or "g"), then the GNU C
      library will be included in all link operations.  Do not rebuild
      lev_comp with "SPECIAL" unless you have a CC symbol setup with
      the proper options.
$	abort
$p1_ok:
$ ! validate second parameter
$	p2 := 'p2'
$	l_opt = f$locate("|"+p2, "|SHAREABLE|LIBRARY__|") !10
$     if (l_opt/10)*10 .eq. l_opt then	goto p2_ok
$	copy sys$input: sys$error:	!p2 usage
%second arg is VAXCRTL handling; it must be one of
       "SHAREABLE" -- link with SYS$SHARE:VAXCRTL.EXE/SHAREABLE
   or   "LIBRARY"  -- link with SYS$LIBRARY:VAXCRTL.OLB/LIBRARY
   or      ""      -- default operation (use shareable image)

Note: for MicroVMS 4.x, "SHAREABLE" (which is the default) is required.
$	abort
$p2_ok:
$ ! compiler setup; if a symbol for "CC" is already defined it will be used
$     if f$type(cc).eqs."STRING" then  goto got_cc
$	cc = vaxc_			!assume "VAXC" requested or defaulted
$	if c_opt.eq.5 then  cc = gnuc_	!explicitly invoked w/ "GNUC" option
$	if c_opt.ne.0 then  goto got_cc !"GNUC" or "LINK", skip compiler check
$	! we want to prevent function inlining with vaxc v3.x (/opt=noinline)
$	!   but we can't use noInline with v2.x, so need to determine version
$	  set noOn
$	  msgenv = f$environment("MESSAGE")
$	  set message/noFacil/noSever/noIdent/noText
$	  cc/noObject _NLA0:/Include=[]     !strip 'noinline' if error
$	  sts = $status
$	if sts then  goto reset_msg	!3.0 or later will check out OK
$	! must be dealing with vaxc 2.x; ancient version (2.2 or earlier)
$	!   can't handle /include='dir', needs c$include instead
$	  cc = cc - "=NOINLINE" - ",NOINLINE" - "NOINLINE,"
$	  if sts.ne.IVQUAL then  goto reset_msg
$	    define/noLog c$include [-.INCLUDE]
$	    c_c_ = "/DEFINE=(""ANCIENT_VAXC"")"
$	    if f$extract(1,3,f$getsyi("VERSION")).lts."4.6" then -
$		c_c_ = c_c_ - ")" + ",""VERYOLD_VMS"")"
$reset_msg:
$	  set message 'msgenv'
$	  set On
$got_cc:
$	cc = cc + c_c_			!append common qualifiers
$	if p3.nes."" then  cc = cc + p3 !append optional user preferences
$	g := 'f$extract(0,1,cc)'
$	if g.nes."G" then  gnulib = ""
$	if g.eqs."G" then  gnulib = "," + gnulib
$ ! linker setup; if a symbol for "LINK" is defined, we'll use it
$	if f$type(link).nes."STRING" then  link = "LINK/NOMAP"
$	if p4.nes."" then  link = link + p4 !append optional user preferences
$	vaxcrtl = "sys$library:vaxcrtl.olb/Library"	!object library
$     if l_opt.ne.0 then  goto vaxcrtl_ok
$	vaxcrtl = "sys$disk:[]vaxcrtl.opt/Options"	!shareable image
$     if f$search("vaxcrtl.opt").nes."" then  goto vaxcrtl_ok !assume its right
$	create sys$disk:[]vaxcrtl.opt
sys$share:vaxcrtl/Shareable
sys$library:vaxcrtl/Library	!/Include=C$$TRANSLATE	!for link() substitute
$vaxcrtl_ok:
$ ! final setup
$	nethacklib = "nethack.olb"
$	milestone = "write sys$output f$fao("" !5%T "",0),"
$     if c_opt.eq.10 then  goto link	!"LINK" requested, skip compilation
$	rename	 := rename/New_Vers
$	touch	 := set file/Truncate
$	makedefs := $sys$disk:[]makedefs
$	show symbol cc
$!
$!  compile and link makedefs, then nethack, finally lev_comp.
$!
$ milestone "<compiling...>"
$ cc [-.vms]vmsmisc	!try simplest one first
$ cc alloc.c
$ if f$search("monst.c").eqs."" then  copy/Concat monst.c1+.c2 monst.c
$ cc monst.c
$ milestone " (monst)"
$ cc objects.c
$     if c_opt.eq.15 then  goto special !"SPECIAL" requested, skip main build
$ cc makedefs.c
$ link makedefs.obj,monst.obj,objects.obj,vmsmisc.obj,-
	'vaxcrtl''gnulib',sys$input:/Opt
identification="makedefs 3.0.10"
$ milestone "makedefs"
$! create some build-time files
$ makedefs -p	!pm.h
$ makedefs -o	!onames.h
$ makedefs -t	!trap.h
$ makedefs -v	!date.h
$! create new object library
$ libr/Obj 'nethacklib'/Create=(Block=2000,Hist=2) vmsmisc.obj,alloc.obj/Insert
$ if f$search(f$parse(".olb;-2",nethacklib)).nes."" then -
$	purge/Keep=2 'nethacklib'
$! compile most of the source files:
$ c1 = "decl,version,[-.vms]vmsmain,[-.vms]vmsunix,[-.vms]vmstty," -
      + "[-.others]random,[-.vms]vmstparam"
$ c2 = "allmain,apply,artifact,attrib,bones,cmd,dbridge,demon,do,do_name," -
      + "do_wear,dog,dogmove,dokick,dothrow,eat,end,engrave,exper,extralev"
$ c3 = "fountain,getline,hack,invent,lock,mail,makemon,mcastu,mhitm,mhitu," -
      + "mklev,mkmaze,mkobj,mkroom,mon,mondata,monmove,mthrowu,music,o_init"
$ c4 = "objnam,options,pager,pickup,polyself,potion,pray,pri,priest,prisym," -
      + "read,restore,rip,rnd,rumors,save,search,shk,shknam,sit,sounds,sp_lev"
$ c5 = "spell,steal,termcap,timeout,topl,topten,track,trap,u_init,uhitm," -
      + "vault,weapon,were,wield,wizard,worm,worn,write,zap"
$! process all 5 lists of files
$   i = 1
$list_loop:
$     list = c'i'	!get next list
$     j = 0
$file_loop:
$	file = f$element(j,",",list)	!get next file
$	if file.eqs."" .or. file.eqs."," then  goto list_done
$	cc 'file'.c
$	if f$extract(0,1,file).eqs."[" then -
$		file = f$edit(f$parse(file,,,"NAME"),"LOWERCASE")
$	libr/Obj 'nethacklib' 'file'.obj/Insert
$	delete 'file'.obj;*
$	milestone " (",file,")"
$	j = j + 1
$     goto file_loop
$list_done:
$     i = i + 1
$   if i.le.5 then  goto list_loop
$! one special case left:  gcc chokes on these commas, but has real bcopy
$	vmstermcap_options = "/Define=(""bcopy(s,d,n)=memcpy(d,s,n)"",""exit=vms_exit"")"
$	if c_opt.eq.5 then  vmstermcap_options = "/Define=(""exit=vms_exit"")"
$ cc 'vmstermcap_options' [-.vms]vmstermcap.c
$ libr/Obj 'nethacklib' vmstermcap.obj/Insert
$ delete vmstermcap.obj;*
$!
$link:
$ milestone "<linking...>"
$ link/Exe=nethack 'nethacklib'/Lib/Incl=(vmsmain,allmain,vmsunix,vmstty,decl),-
	sys$disk:[]monst.obj,objects.obj,-	!(data-only modules, like decl)
	sys$input:/Opt,'vaxcrtl''gnulib'
identification="NetHack 3.0.10"
$ milestone "NetHack"
$     if c_opt.eq.10 then  goto done	!"LINK" only
$special:
$!
$! build special level compiler
$!
$ cc lev_main.c
$ cc lev_comp.c
$ copy [-.vms]lev_lex.h stdio.*/Prot=(s:rwd,o:rwd)
$ cc lev_lex.c
$ rename stdio.h lev_lex.*
$ cc panic.c
$ link lev_comp.obj,lev_lex.obj,lev_main.obj,-
	monst.obj,objects.obj,alloc.obj,panic.obj,vmsmisc.obj,-
	'vaxcrtl''gnulib',sys$input:/Opt
identification="lev_comp 3.0.10"
$ milestone "lev_comp"
$!
$done:
$ exit
