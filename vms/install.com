$ ! vms/install.com -- set up nethack 'playground'
$ !
$ ! Use vmsbuild.com to create nethack.exe, makedefs, and lev_comp *first*.
$ !
$ ! Edit this file to define gamedir & gameuic, or else invoke it with two
$ ! command line parameters, as in:
$ !	@[.vms]install "disk$users:[games.nethack]" "[40,1]"
$ !
$	gamedir = "USR$ROOT0:[GENTZEL.NHDIR]"	!location of playground
$	gameuic = "GENTZEL"			!owner of playground
$	if p1.nes."" then  gamedir := 'p1'
$	if p2.nes."" then  gameuic := 'p2'
$
$	play_files = "PERM.,RECORD.,LOGFILE."
$	help_files = "HELP.,HH.,CMDHELP.,HISTORY.,OPTHELP.,ORACLES.,LICENSE."
$	data_files = "DATA.,RUMORS."
$	spec_files = "ENDGAME.,CASTLE.,TOWER%."
$	spec_input = "endgame.des castle.des tower.des"
$ makedefs := $sys$disk:[-.src]makedefs
$ lev_comp := $sys$disk:[-.src]lev_comp
$
$	! start from a known location -- [.vms]
$	set default 'f$parse(f$environment("PROCEDURE"),,,"DIRECTORY")'
$! generate miscellaneous data files
$	set default [-.auxil]	!move to auxiliary directory
$ makedefs -d	!data
$ makedefs -r	!rumors
$! lev_comp 'spec_input' !special levels
$! (flex'd version doesn't handle multiple inputs, so do them one at a time)
$ lev_comp endgame.des
$ lev_comp castle.des
$ lev_comp tower.des
$	set default [-]		!move up
$
$! set up the directories
$ create/directory/owner='gameuic'/prot=(s:rwe,o:rwe,g:rwe,w:rwe)/log -
	'f$string(gamedir - "]" + ".SAVE]")'
$! create empty writeable files
$ create/owner='gameuic'/prot=(s:rwed,o:rwed,g:rwed,w:rwed) -
	'gamedir''play_files'
$! copy over the remaining game files, then make them readonly
$ copy/prot=(s:rwed,o:rwed,g:re,w:re) -
	[.auxil]'help_files','data_files','spec_files',[.src]nethack.exe -
	'gamedir'*.*
$ set file/owner='gameuic'/prot=(s:re,o:re) -
	'gamedir''help_files','data_files','spec_files',nethack.exe
$! done
$ define/nolog hackdir 'gamedir'
$ write sys$output -
    f$fao("!/ Nethack installation complete. !/ Playground is !AS !/",gamedir)
$ exit
