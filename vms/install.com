$ gamedir = "USR$ROOT0:[GENTZEL.NHDIR]"
$ gameuic = "GENTZEL"
$
$ set default [.src]
$ mcr sys$disk:[]makedefs -d
$ mcr sys$disk:[]makedefs -r
$ set default [-]
$
$! set up the directories
$ create/directory/owner='gameuic'/prot=(s:rwed,o:rwed,g:rwed,w:rwed) -
	'f$string(gamedir - "]" + ".SAVE]")'
$! create some files
$ create/prot=(s:rwed,o:rwed,g:rwed,w:rwed) 'gamedir'perm.,record.,logfile.
$! copy over the game files
$ copy/prot=(s:re,o:re,g:re,w:re) [.auxil]help.,hh,cmdhelp,history,opthelp, -
	oracles,data,rumors,endgame,castle,tower1,tower2,tower3,license -
	'gamedir'
$ copy/prot=(s:re,o:re,g:re,w:re) [.src]nethack.exe 'gamedir'
$! set up the permissions
$ set file/owner='gameuic' 'gamedir'*.*;*
