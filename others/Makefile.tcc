#	SCCS Id: @(#)Makefile.tcc	3.0	89/07/07
#	PC NetHack makefile for Turbo C 2.0
#	Perpetrator: Mike Threepoint, 890707

# Unfortunately, Turbo C's large model has a limit of 64K total global data
MODEL=h
# Wizardly defines
WIZARD	= -DDEBUG

# Directories (makedefs hardcodes these, don't change them)
INCL	= ..\include
AUX	= ..\auxil
SRC	= ..\src

# Signed chars, optimize jumps, ANSI compatibility, no register optimizaton,
# no stack frame.
# Note: There is a bug in Turbo C 2.0's -Z.  If you have weird problems,
#	use -Z-.
CFLAGS	= -c -no -m$(MODEL) -I$(INCL) -K- -O -A -Z -k- -w-pia -w-pro $(WIZARD)
CC	= tcc

TARG	= pc

# Optional PC NetHack features (see pcconf.h).  Set to nothing if not used.
#
#	Fish's TERMLIB termcap library.
#TERMLIB = $(LIB)\termlib.lib
TERMLIB =
#
# 	High-quality BSD random number generation routines.
RANDOM = o\random.obj

LFLAGS  = /noi
TLFLAGS = /x/c
# No need to link in the floating point library
LIBS	= $(LIB)\c$(MODEL)

#
# There is a bug in TLINK and huge model:
#
# TLINK 1.0 treated huge like large, with 64K data limit.
# TLINK 1.1 fixed that, but chokes over huge data segments anyway.
# TLINK 2.0 links and is smaller than LINK /EXEPACK, but for some
# reason with too many objects it produces a file that freaks out
# and hangs the system.
#
# Also note:
#
# Using /EXEPACK with LINK will greatly reduce the size of the
# executable (about 50K), it will also greatly increase the memory
# required to load it (about 20K).
TLINK	= tlink
LINK	= link

# The game name
GAME= nethack

# The game directory
GAMEDIR = \games\$(GAME)

# The game filename
GAMEFILE = $(GAMEDIR)\$(GAME).exe

# object files for makedefs
MAKEOBJS = o\makedefs.obj o\monst.obj o\objects.obj

# object files for special levels compiler
SPLEVOBJS = o\lev_comp.obj o\lev_lex.obj o\lev_main.obj o\monst.obj o\objects.obj

# alloc.c and panic.c are unnecessary for PC NetHack's makedefs.exe
# and lev_comp.exe.

# nothing below this line should have to be changed
#
# other things that have to be reconfigured are in config.h,
# {unixconf.h, pcconf.h, tosconf.h}, and possibly system.h

VOBJS = o\allmain.obj o\main.obj o\tty.obj o\unix.obj o\hack.obj o\termcap.obj \
	o\getline.obj o\pri.obj o\prisym.obj o\topl.obj o\cmd.obj o\msdos.obj \
	o\decl.obj o\monst.obj o\objects.obj \
	o\timeout.obj $(RANDOM) o\rnd.obj \
	o\monmove.obj o\dogmove.obj o\mondata.obj o\exper.obj o\mon.obj \
	o\mhitu.obj o\uhitm.obj o\mkobj.obj o\makemon.obj o\invent.obj \
	o\pager.obj o\restore.obj
VOBJM = o\apply.obj o\artifact.obj o\attrib.obj o\dbridge.obj o\demon.obj \
	o\do.obj o\do_name.obj o\do_wear.obj o\dog.obj o\dokick.obj \
	o\dothrow.obj o\eat.obj o\lock.obj o\mcastu.obj o\mhitm.obj \
	o\mthrowu.obj o\objnam.obj o\options.obj o\pickup.obj o\polyself.obj \
	o\potion.obj o\pray.obj o\priest.obj o\read.obj o\search.obj \
	o\shk.obj o\sit.obj o\sounds.obj o\steal.obj o\track.obj o\trap.obj \
	o\vault.obj o\weapon.obj o\were.obj o\wield.obj o\wizard.obj \
	o\worm.obj o\worn.obj o\write.obj o\zap.obj
VOBJ1 = o\engrave.obj o\fountain.obj o\spell.obj o\rumors.obj o\music.obj
VOBJ2 = o\save.obj o\mklev.obj o\mkmaze.obj o\extralev.obj \
	o\sp_lev.obj o\mkroom.obj o\bones.obj o\shknam.obj
VOBJL = o\topten.obj o\end.obj o\o_init.obj o\u_init.obj o\rip.obj

VOBJ  = $(VOBJS) $(VOBJM) $(VOBJ1) $(VOBJ2) $(VOBJL)
HOBJ  = $(VOBJS) $(VOBJM) $(VOBJ1) $(VOBJ2) o\version.obj $(VOBJL)

#
# Weird order, isn't it?  It puts the most often used utility routines
# and the main loop at the start of the file, and the routines that are
# only called at the beginning and end of a game come last.  This
# should improve speed and make overlays work more efficiently.
#
# alloc.c, ioctl.c, and mail.c are unnecessary for PC NetHack.

PCCONF_H   = $(INCL)\$(TARG)conf.h $(INCL)\msdos.h
GLOBAL_H   = $(INCL)\global.h $(INCL)\coord.h $(PCCONF_H)
CONFIG_H   = $(INCL)\config.h $(INCL)\tradstdc.h $(GLOBAL_H)
TRAP_H	   = $(INCL)\trap.h
#PCCONF_H  = $(INCL)\system.h $(INCL)\extern.h
PERMONST_H = $(INCL)\permonst.h $(INCL)\monflag.h
YOU_H	   = $(INCL)\you.h $(INCL)\attrib.h $(PERMONST_H) $(INCL)\mondata.h \
	     $(INCL)\monst.h $(INCL)\youprop.h
#DECL_H	   = $(INCL)\decl.h
DECL_H	   = $(INCL)\spell.h $(INCL)\obj.h $(YOU_H) $(INCL)\onames.h \
	     $(INCL)\pm.h
HACK_H	   = $(CONFIG_H) $(DECL_H) $(INCL)\monsym.h $(INCL)\mkroom.h \
	     $(INCL)\objclass.h $(INCL)\gold.h $(INCL)\trap.h $(INCL)\flag.h \
	     $(INCL)\rm.h
# extern.h, decl.h, and system.h contain only external declarations.
#
# If anything in them changes, all other files involving the changed routines
# should be changed to reflect them.  Including them in their respective
# dependency lists will make sure everything is correct, but causes frequent
# near-total recompiles.  By leaving them out, we allow quicker testing of
# changes, but we presume the wiz knows to be circumspect.

# The main target
$(GAMEFILE): o $(HOBJ) Makefile
	if exist $@ del $@
	$(LINK) $(C0) $(HOBJ),$@ /seg:1024,,$(LIBS) $(TERMLIB) $(LFLAGS)
	echo 
$(GAME): $(GAMEFILE)

.c.obj:
	$(CC) $(CFLAGS) $<

all:	o $(GAME) auxil
	@echo Done.

o:
	mkdir o

makedefs.exe:  $(MAKEOBJS)
	@$(TLINK) $(TLFLAGS) $(C0) $(MAKEOBJS),$@,,$(LIBS);

o\makedefs.obj:  $(INCL)\config.h $(INCL)\permonst.h $(INCL)\objclass.h

lev_comp.exe:  $(SPLEVOBJS)
	@$(TLINK) $(TLFLAGS) $(C0) $(SPLEVOBJS),$@,,$(LIBS);

o\lev_comp.obj:  $(HACK_H) $(INCL)\sp_lev.h
o\lev_lex.obj:  $(INCL)\lev_comp.h $(HACK_H) $(INCL)\sp_lev.h
o\lev_main.obj:  $(HACK_H) $(INCL)\sp_lev.h

# If you have yacc or lex programs, and make any changes,
# add some .y.c and .l.c rules to your Make.ini.

lev_comp.c:  lev_comp.y
lev_lex.c:  lev_comp.l

#
#	The following include files depend on makedefs to be created.
#
#	date.h should be remade any time any of the source or include code
#	is modified.
#
$(INCL)\date.h: 	$(VOBJ) makedefs.exe
	.\makedefs -v

$(INCL)\trap.h: 	makedefs.exe
	.\makedefs -t

$(INCL)\onames.h:	makedefs.exe
	.\makedefs -o

$(INCL)\pm.h:		makedefs.exe
	.\makedefs -p

data:	$(AUX)\data.base makedefs.exe
	.\makedefs -d

rumors: $(AUX)\rumors.tru $(AUX)\rumors.fal makedefs.exe
	.\makedefs -r

#
#	The following programs vary depending on what OS you are using.
#
o\main.obj:	$(HACK_H) $(TARG)main.c
	$(CC) $(CFLAGS) -o$@ $(TARG)main.c

o\tty.obj:	$(HACK_H) $(INCL)\func_tab.h $(TARG)tty.c
	$(CC) $(CFLAGS) -o$@ $(TARG)tty.c

o\unix.obj:	$(HACK_H) $(TARG)unix.c
	$(CC) $(CFLAGS) -o$@ $(TARG)unix.c

#
# Secondary targets
#

auxil:	spec_levs
	cd $(AUX)
	xcopy *. $(GAMEDIR)

spec_levs: $(AUX)\castle.des $(AUX)\endgame.des $(AUX)\tower.des lev_comp.exe
	lev_comp $(AUX)\castle.des
	lev_comp $(AUX)\endgame.des
	lev_comp $(AUX)\tower.des
	cd $(AUX)
	xcopy castle $(GAMEDIR)
	del castle
	xcopy endgame $(GAMEDIR)
	del endgame
	xcopy tower? $(GAMEDIR)
	del tower?

clean:
	del o\*.obj
	rmdir o

spotless: clean
	cd $(INCL)
	del date.h
	del onames.h
	del pm.h
	touch date.h onames.h pm.h
	cd $(AUX)
	del data
	del rumors
	cd $(SRC)
	del makedefs.exe
	if exist lev_comp.exe del lev_comp.exe

#
# Other dependencies
#

# GO AHEAD, DELETE THIS LINE

o\allmain.obj:  $(HACK_H)
o\alloc.obj:  $(CONFIG_H)
o\apply.obj:  $(HACK_H) $(INCL)\edog.h
o\artifact.obj:  $(HACK_H) $(INCL)\artifact.h
o\attrib.obj:  $(HACK_H)
o\bones.obj:  $(HACK_H)
o\cmd.obj:  $(HACK_H) $(INCL)\func_tab.h
o\dbridge.obj: $(HACK_H)
o\decl.obj:  $(HACK_H)
o\demon.obj:  $(HACK_H)
o\do.obj:  $(HACK_H)
o\do_name.obj:  $(HACK_H)
o\do_wear.obj:  $(HACK_H)
o\dog.obj:  $(HACK_H) $(INCL)\edog.h
o\dogmove.obj:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\edog.h
o\dokick.obj:  $(HACK_H)
o\dothrow.obj:  $(HACK_H)
o\eat.obj:  $(HACK_H)
o\end.obj:  $(HACK_H) $(INCL)\eshk.h
o\engrave.obj:  $(HACK_H)
o\exper.obj:  $(HACK_H)
o\extralev.obj:  $(HACK_H)
o\fountain.obj:  $(HACK_H)
o\getline.obj:  $(HACK_H) $(INCL)\func_tab.h
o\hack.obj:  $(HACK_H)
o\invent.obj:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
o\ioctl.obj:  $(HACK_H)
o\lev_comp.obj:  $(HACK_H) $(INCL)\sp_lev.h
o\lock.obj:  $(HACK_H)
o\makemon.obj:  $(HACK_H)
o\mail.obj:  $(HACK_H)
o\mcastu.obj:  $(HACK_H)
o\mhitm.obj:  $(HACK_H) $(INCL)\artifact.h
o\mhitu.obj:  $(HACK_H) $(INCL)\artifact.h $(INCL)\edog.h
o\mklev.obj:  $(HACK_H)
o\mkmaze.obj:  $(HACK_H)
o\mkobj.obj:  $(HACK_H)
o\mkroom.obj:  $(HACK_H)
o\mon.obj:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\artifact.h
o\mondata.obj:  $(HACK_H) $(INCL)\eshk.h $(INCL)\epri.h
o\monmove.obj:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\artifact.h
o\monst.obj:  $(CONFIG_H) $(PERMONST_H) $(INCL)\eshk.h $(INCL)\vault.h $(INCL)\epri.h
o\msdos.obj:  $(HACK_H) msdos.c
	$(CC) $(CFLAGS) -A- $*.c
# set ANSI only off; many MS-DOS specific things.
o\mthrowu.obj:  $(HACK_H)
o\music.obj:  $(HACK_H)
o\o_init.obj:  $(HACK_H) $(INCL)\onames.h
o\objects.obj:	$(CONFIG_H) $(INCL)\obj.h $(INCL)\objclass.h $(INCL)\prop.h
o\objnam.obj:  $(HACK_H)
o\options.obj:  $(HACK_H)
o\pager.obj:  $(HACK_H)
o\panic.obj:  $(CONFIG_H)
o\pickup.obj:  $(HACK_H)
o\polyself.obj:  $(HACK_H)
o\potion.obj:  $(HACK_H)
o\pray.obj:  $(HACK_H)
o\pri.obj:  $(HACK_H)
o\priest.obj:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\eshk.h $(INCL)\epri.h
o\prisym.obj:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
o\random.obj:
o\read.obj:  $(HACK_H)
o\restore.obj:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
o\rip.obj:  $(HACK_H) rip.c
	$(CC) $(CFLAGS) -d- $*.c
# must not merge strings, or the tombstone lines will overlap.
o\rnd.obj:  $(HACK_H)
o\rumors.obj:  $(HACK_H)
o\save.obj:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
o\search.obj:  $(HACK_H) $(INCL)\artifact.h
o\shk.obj:  $(HACK_H) $(INCL)\eshk.h
o\shknam.obj:  $(HACK_H) $(INCL)\eshk.h
o\sit.obj:  $(HACK_H)
o\sounds.obj:  $(HACK_H) $(INCL)\edog.h $(INCL)\eshk.h
o\sp_lev.obj:  $(HACK_H) $(INCL)\sp_lev.h
o\spell.obj:  $(HACK_H)
o\steal.obj:  $(HACK_H)
o\termcap.obj:  $(HACK_H)
o\timeout.obj:  $(HACK_H)
o\topl.obj:  $(HACK_H)
o\topten.obj:  $(HACK_H)
o\track.obj:  $(HACK_H)
o\trap.obj:  $(HACK_H) $(INCL)\edog.h $(INCL)\trapname.h
o\u_init.obj:  $(HACK_H)
o\uhitm.obj:  $(HACK_H) $(INCL)\artifact.h
o\vault.obj:  $(HACK_H) $(INCL)\vault.h
o\version.obj:  $(HACK_H) $(INCL)\date.h
o\weapon.obj:  $(HACK_H)
o\were.obj:  $(HACK_H)
o\wield.obj:  $(HACK_H)
o\wizard.obj:  $(HACK_H)
o\worm.obj:  $(HACK_H) $(INCL)\wseg.h
o\worn.obj:  $(HACK_H)
o\write.obj:  $(HACK_H)
o\zap.obj:  $(HACK_H)
