$ ! vms/install.com -- set up nethack 'playground'
$ !
$ ! Use vmsbuild.com to create nethack.exe, makedefs, and lev_comp *first*.
$ !
$ ! Edit this file to define gamedir & gameuic, or else invoke it with two
$ ! command line parameters, as in:
$ !	@[.sys.vms]install "disk$users:[games.nethack]" "games"
$ ! or	@[.sys.vms]install "[-.play]" "[40,1]"
$ !
$	! default location is old playground, default owner is installer
$	gamedir = f$trnlnm("HACKDIR")	!location of playground
$	gameuic = f$user()		!owner of playground
$	! --- nothing below this line should need to be changed ---
$	if p1.nes."" then  gamedir := 'p1'
$	if p2.nes."" then  gameuic := 'p2'
$
$	! note: all filespecs contain some punctuation,
$	!	to avoid inadvertent logical name interaction
$	play_files = "PERM.,RECORD.,LOGFILE."
$	help_files = "HELP.,HH.,CMDHELP.,WIZHELP.,OPTHELP.,HISTORY.,LICENSE."
$	data_files = "DATA.,RUMORS.,ORACLES.,OPTIONS.,QUEST.DAT"
$	guidebook  = "[.doc]Guidebook.txt"
$	invoc_proc = "[.sys.vms]nethack.com"
$	trmcp_file = "[.sys.share]termcap"
$	spec_files = "AIR.LEV,ASMODEUS.LEV,ASTRAL.LEV,BAALZ.LEV,BIGROOM.LEV," -
		   + "CASTLE.LEV,EARTH.LEV,FAKEWIZ%.LEV,FIRE.LEV," -
		   + "JUIBLEX.LEV,KNOX.LEV,MEDUSA-%.LEV,MINEFILL.LEV," -
		   + "MINETOWN.LEV,MINE_END.LEV,ORACLE.LEV,ORCUS.LEV," -
		   + "SANCTUM.LEV,TOWER%.LEV,VALLEY.LEV,WATER.LEV,WIZARD%.LEV"
$	spec_input = "bigroom.des castle.des endgame.des " -
		   + "gehennom.des knox.des medusa.des mines.des " -
		   + "oracle.des tower.des yendor.des"
$	qstl_files = "%-GOAL.LEV,%-FILL%.LEV,%-LOCATE.LEV,%-START.LEV"
$	qstl_input = "Arch.des Barb.des Caveman.des Elf.des " -
		   + "Healer.des Knight.des Priest.des Rogue.des " -
		   + "Samurai.des Tourist.des Wizard.des Valkyrie.des"
$	dngn_files = "DUNGEON."
$	dngn_input = "dungeon.pdf"
$ makedefs := $sys$disk:[-.util]makedefs
$ lev_comp := $sys$disk:[-.util]lev_comp
$ dgn_comp := $sys$disk:[-.util]dgn_comp
$ milestone = "write sys$output f$fao("" !5%T "",0),"
$ if p3.nes."" then  milestone = "!"
$!
$! make sure we've got a playground location
$ gamedir := 'gamedir'
$ if gamedir.eqs."" then  gamedir = "[.play]"	!last ditch default
$ gamedir = f$parse(gamedir,,,,"SYNTAX_ONLY") - ".;"
$ if gamedir.eqs."" then  write sys$error "% must specify playground directory"
$ if gamedir.eqs."" then  exit %x1000002C	!ss$_abort
$
$!
$!	['p3' is used in Makefile.top]
$ if p3.nes."" then  goto make_'p3'
$
$	milestone "<installation...>"
$!
$make_data:
$	! start from a known location -- [.sys.vms]
$	set default 'f$parse(f$environment("PROCEDURE"),,,"DIRECTORY")'
$! generate miscellaneous data files
$	set default [-.-.dat]	!move to data directory
$	milestone "(data)"
$ makedefs -d	!data.base -> data
$	milestone "(rumors)"
$ makedefs -r	!rumors.tru + rumors.fal -> rumors
$	milestone "(oracles)"
$ makedefs -h	!oracles.txt -> oracles
$	milestone "(dungeon preprocess)"
$ makedefs -e	!dungeon.def -> dungeon.pdf
$	milestone "(quest text)"
$ makedefs -q	!quest.txt -> quest.dat
$	milestone "(special levels)"
$ lev_comp 'spec_input' !special levels
$	milestone "(quest levels)"
$ lev_comp 'qstl_input' !quest levels
$	milestone "(dungeon compile)"
$ dgn_comp 'dngn_input' !dungeon database
$	set default [-]		!move up
$ if p3.nes."" then  exit
$
$!
$! set up the playground and save directories
$	milestone "(directories)"
$make_directories:
$	srctree = f$environment("DEFAULT")
$	set default 'gamedir'
$ if f$parse("[-]").eqs."" then  create/dir/log [-] !default owner & protection
$ if f$parse("[]" ).eqs."" then - !needs to be world writable
   create/directory/owner='gameuic'/prot=(s:rwe,o:rwe,g:rwe,w:rwe)/log []
$ if f$search("SAVE.DIR;1").eqs."" then -
   create/directory/owner='gameuic'/prot=(s:rwe,o:rwe,g:rwe,w:rwe)/log -
	[.SAVE]/version_limit=2
$	set default 'srctree'
$ if p3.nes."" then  exit
$!
$! create empty writeable files -- logfile, scoreboard, multi-user access lock
$	milestone "(writeable files)"
$make_writeable_files:
!-!$ create/owner='gameuic'/prot=(s:rwed,o:rwed,g:rwed,w:rwed) -
!-!	'gamedir''play_files'
$	i = 0
$ploop: f = f$element(i,",",play_files)
$	if f.eqs."," then  goto pdone
$	i = i + 1
$	f = gamedir + f
$	if f$search(f).nes.""	!file already exists
$	then	if f$file_attrib(f,"RFM").eqs."STMLF" then  goto ploop
$		rename/new_vers 'f' *.old	!needs to be stream_lf
$	endif
$	create/fdl=sys$input:/owner='gameuic' 'f'/log
file
 organization sequential
 protection (system:rwd,owner:rwd,group:rw,world:rw)
record
 format stream_lf
$	goto ploop
$pdone:
$ if p3.nes."" then  exit
$!
$! copy over the remaining game files, then make them readonly
$	milestone "(readonly files)"
$make_readonly_files:
$ copy/prot=(s:rwed,o:rwed,g:re,w:re) -
	[.dat]'help_files','data_files','spec_files','qstl_files','dngn_files' -
	'gamedir'*.*
$ set file/owner='gameuic'/prot=(s:re,o:re) -
	'gamedir''help_files','data_files','spec_files','qstl_files','dngn_files'
$ if p3.nes."" then  exit
$!
$	milestone "(nethack.exe)"
$make_executable:
$ copy/prot=(s:rwed,o:rwed,g:re,w:re) [.src]nethack.exe 'gamedir'*.*
$ set file/owner='gameuic'/prot=(s:re,o:re) 'gamedir'nethack.exe
$ if p3.nes."" then  exit
$!
$! provide invocation procedure (if available)
$make_procedure:
$ if f$search(invoc_proc).eqs."" then  goto skip_dcl
$ if f$search("''gamedir'nethack.com").nes."" then -
    if f$cvtime(f$file_attr("''gamedir'nethack.com","RDT")) -
      .ges. f$cvtime(f$file_attr(invoc_proc,"RDT")) then  goto skip_dcl
$	milestone "(nethack.com)"
$  copy/prot=(s:rwed,o:rwed,g:re,w:re) 'invoc_proc' 'gamedir'nethack.com
$  set file/owner='gameuic'/prot=(s:re,o:re) 'gamedir'nethack.com
$skip_dcl:
$ if p3.nes."" then  exit
$!
$! provide plain-text Guidebook doc file (if available)
$make_documentation:
$ if f$search(guidebook).eqs."" then  goto skip_doc
$	milestone "(Guidebook)"
$  copy/prot=(s:rwed,o:rwed,g:re,w:re) 'guidebook' 'gamedir'Guidebook.doc
$  set file/owner='gameuic'/prot=(s:re,o:re) 'gamedir'Guidebook.doc
$skip_doc:
$ if p3.nes."" then  exit
$!
$! provide last-resort termcap file (if available)
$make_termcap:
$ if f$search(trmcp_file).eqs."" then  goto skip_termcap
$ if f$search("''gamedir'termcap").nes."" then  goto skip_termcap
$	milestone "(termcap)"
$  copy/prot=(s:rwed,o:rwed,g:re,w:re) 'trmcp_file' 'gamedir'termcap
$  set file/owner='gameuic'/prot=(s:re,o:re) 'gamedir'termcap
$skip_termcap:
$ if p3.nes."" then  exit
$!
$! done
$	milestone "<done>"
$ define/nolog hackdir 'gamedir'
$ write sys$output -
    f$fao("!/ Nethack installation complete. !/ Playground is !AS !/",gamedir)
$ exit
