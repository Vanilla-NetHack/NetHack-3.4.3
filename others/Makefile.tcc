#	SCCS Id: @(#)Makefile.tcc	3.0	89/11/20
#	PC NetHack 3.0 Makefile for Turbo C 2.0
#	Perpetrator: Mike Threepoint, 890707

###
### Locals
###
# the name of the game
GAME	= nethack

# the place of the game
GAMEDIR = \games\$(GAME)

# the filename of the game
GAMEFILE = $(GAMEDIR)\$(GAME).exe


###
### Directories
###
# makedefs.c hardcodes the include and auxil directories, don't change them.
OBJ	= o
INCL	= ..\include
AUX	= ..\auxil
SRC	= ..\src
OTHERS	= ..\others

# where the Turbo C libraries are kept
LIB     = \turbo\c\lib

# directory NDMAKE uses for temporary files
MAKE_TMP = $(TMP)


###
### Compiler
###
CC	= tcc

# must use Huge model; Large is limited to 64K total global data.
MODEL   = h

# signed chars, jump optimize, strict ANSI, register optimize, no stack frame
CFLAGS	= -c -no -m$(MODEL) -I$(INCL) -K- -O -A -Z -k- -w-pia -w-pro $(WIZARD)
## Note: Turbo C 2.0's -Z is bugged.  If you have weird problems, try -Z-.

# wizardly defines
WIZARD  =

# linkers
TLINK	= tlink
LINK	= link
## There is a bug in TLINK and huge model:
##
## TLINK 1.0 treated huge like large, with 64K data limit.
## TLINK 1.1 fixed that, but chokes over huge data segments anyway.
## TLINK 2.0 links and is smaller than LINK /EXEPACK, but for some
## reason with too many objects it produces a file that freaks out
## and hangs the system.
##
## Also note:
##
## Using /EXEPACK with LINK will greatly reduce the size of the
## executable (about 50K), it will also greatly increase the memory
## required to load it (about 20K).

LIBS	= $(LIB)\c$(MODEL)
# no need to link in the floating point library
C0	= $(LIB)\c0$(MODEL).obj

LFLAGS	= /noi /seg:1024
TLFLAGS = /x/c

# assembler
ASM	= tasm
AFLAGS	= /MX

# yacc/lex
YACC	= bison
LEX	= flex


###
### Rules
###
# search order
.SUFFIXES: .exe .obj .c .asm .y .l

# .l -> .c (for flex)
.l.c:
	$(LEX) $<
	del $@
	ren lex.yyc $@
# .y -> .c (for bison)
.y.c:
	$(YACC) $<
	del $@
	ren y.tbc $@
	del $*.h
	ren y.tbh $*.h
# .c -> .obj
.c.obj:
	$(CC) $(CFLAGS) -c $<
# .asm -> .obj
.asm.obj:
	$(ASM) $(AFLAGS) $<;
# .obj -> .exe (for tlink)
.obj.exe:
	$(TLINK) $(TLFLAGS) $(C0) $<, $@,, $(LIBS);

# NDMAKE automatic response file generation
.RESPONSE_LINK: tlink
.RESPONSE_LIB:  tlib


###
### Optional features (see pcconf.h)
###
# uncomment the definitions used

# overlays
#OVERLAY = $(OBJ)\trampoli.obj ovlmgr.obj
#OVERLAY_H = $(INCL)\trampoli.h
#LINK_LIST = $(OVERLAYS)
OVERLAY =
OVERLAY_H =
LINK_LIST = $(HOBJ)

# Fish's TERMLIB termcap library (see the rule below)
#TERMLIB = $(LIB)\termlib.lib
TERMLIB =

# high-quality BSD random number generation routines
#RANDOM = $(OBJ)\random.obj
RANDOM =


###
### Dependencies
###
# nothing below this line should have to be changed
# other things that must be reconfigured are in config.h and $(TARG)conf.h

# target prefix
TARG	= pc

# object files for makedefs.exe
MAKEOBJS = $(OBJ)\makedefs.obj $(OBJ)\monst.obj $(OBJ)\objects.obj

# object files for lev_comp.exe
SPLEVOBJS = $(OBJ)\lev_comp.obj   $(OBJ)\lev_lex.obj  $(OBJ)\lev_main.obj \
	    $(OBJ)\monst.obj	  $(OBJ)\objects.obj

# object files for termlib.lib
TERMOBJS = $(OBJ)\tgetent.obj  $(OBJ)\tgetflag.obj  $(OBJ)\tgetnum.obj \
	   $(OBJ)\tgetstr.obj  $(OBJ)\tgoto.obj     $(OBJ)\tputs.obj \
	   $(OBJ)\isdigit.obj  $(OBJ)\fgetlr.obj
TERMLIST = -+ $(OBJ)\tgetent.obj -+ $(OBJ)\tgetflag.obj -+ $(OBJ)\tgetnum.obj \
	   -+ $(OBJ)\tgetstr.obj -+ $(OBJ)\tgoto.obj	-+ $(OBJ)\tputs.obj \
	   -+ $(OBJ)\isdigit.obj -+ $(OBJ)\fgetlr.obj

# alloc.c is completely unnecessary for any PC NetHack executable.
# panic.c is unnecessary for makedefs.exe and lev_comp.exe.
# ioctl.c is unnecessary for nethack.exe.

ROOT =	$(OBJ)\main.obj    $(OBJ)\allmain.obj $(OBJ)\msdos.obj \
	$(OBJ)\termcap.obj $(OBJ)\cmd.obj     $(OBJ)\hack.obj \
	$(OVERLAY)

# the overlays -- the Microsoft Overlay Linker is limited to 63

OVL01 = $(OBJ)\decl.obj
OVL02 = $(OBJ)\topl.obj
OVL03 = $(OBJ)\pri.obj $(OBJ)\prisym.obj
OVL04 = $(OBJ)\rnd.obj $(RANDOM)
OVL05 = $(OBJ)\timeout.obj
OVL06 = $(OBJ)\mon.obj $(OBJ)\exper.obj
OVL07 = $(OBJ)\attrib.obj
OVL08 = $(OBJ)\monst.obj $(OBJ)\mondata.obj
OVL09 = $(OBJ)\monmove.obj $(OBJ)\track.obj
OVL10 = $(OBJ)\dog.obj $(OBJ)\dogmove.obj
OVL11 = $(OBJ)\makemon.obj
OVL12 = $(OBJ)\do_name.obj $(OBJ)\getline.obj
OVL13 = $(OBJ)\weapon.obj
OVL14 = $(OBJ)\wield.obj
OVL15 = $(OBJ)\invent.obj
OVL16 = $(OBJ)\objects.obj
OVL17 = $(OBJ)\mkobj.obj $(OBJ)\o_init.obj
OVL18 = $(OBJ)\objnam.obj
OVL19 = $(OBJ)\worn.obj
OVL20 = $(OBJ)\do_wear.obj
OVL21 = $(OBJ)\trap.obj
OVL22 = $(OBJ)\dothrow.obj
OVL23 = $(OBJ)\dokick.obj
OVL24 = $(OBJ)\uhitm.obj
OVL25 = $(OBJ)\mhitu.obj
OVL26 = $(OBJ)\mcastu.obj
OVL27 = $(OBJ)\mhitm.obj
OVL28 = $(OBJ)\mthrowu.obj
OVL29 = $(OBJ)\steal.obj
OVL30 = $(OBJ)\priest.obj
OVL31 = $(OBJ)\vault.obj
OVL32 = $(OBJ)\shk.obj $(OBJ)\shknam.obj
OVL33 = $(OBJ)\wizard.obj
OVL34 = $(OBJ)\worm.obj
OVL35 = $(OBJ)\were.obj
OVL36 = $(OBJ)\demon.obj
OVL37 = $(OBJ)\artifact.obj
OVL38 = $(OBJ)\music.obj $(OBJ)\dbridge.obj
OVL39 = $(OBJ)\sit.obj $(OBJ)\fountain.obj
OVL40 = $(OBJ)\sounds.obj
OVL41 = $(OBJ)\spell.obj
OVL42 = $(OBJ)\read.obj
OVL43 = $(OBJ)\potion.obj
OVL44 = $(OBJ)\zap.obj
OVL45 = $(OBJ)\eat.obj $(OBJ)\rumors.obj
OVL46 = $(OBJ)\do.obj
OVL47 = $(OBJ)\search.obj
OVL48 = $(OBJ)\lock.obj
OVL49 = $(OBJ)\apply.obj
OVL50 = $(OBJ)\engrave.obj $(OBJ)\write.obj
OVL51 = $(OBJ)\pray.obj
OVL52 = $(OBJ)\options.obj
OVL53 = $(OBJ)\pickup.obj
OVL54 = $(OBJ)\polyself.obj
OVL55 = $(OBJ)\u_init.obj
OVL56 = $(OBJ)\extralev.obj
OVL57 = $(OBJ)\mklev.obj $(OBJ)\mkroom.obj
OVL58 = $(OBJ)\mkmaze.obj $(OBJ)\sp_lev.obj
OVL59 = $(OBJ)\restore.obj $(OBJ)\save.obj $(OBJ)\bones.obj
OVL60 = $(OBJ)\rip.obj $(OBJ)\topten.obj $(OBJ)\end.obj
OVL61 = $(OBJ)\unix.obj $(OBJ)\tty.obj $(OBJ)\mail.obj
OVL62 = $(OBJ)\pager.obj
OVL63 = $(OBJ)\version.obj

# date.h dependencies
VOBJ = $(ROOT)	$(OVL01) $(OVL02) $(OVL03) $(OVL04) $(OVL05) $(OVL06) $(OVL07) \
       $(OVL08) $(OVL09) $(OVL10) $(OVL11) $(OVL12) $(OVL13) $(OVL14) $(OVL15) \
       $(OVL16) $(OVL17) $(OVL18) $(OVL19) $(OVL20) $(OVL21) $(OVL22) $(OVL23) \
       $(OVL24) $(OVL25) $(OVL26) $(OVL27) $(OVL28) $(OVL29) $(OVL30) $(OVL31) \
       $(OVL32) $(OVL33) $(OVL34) $(OVL35) $(OVL36) $(OVL37) $(OVL38) $(OVL39) \
       $(OVL40) $(OVL41) $(OVL42) $(OVL43) $(OVL44) $(OVL45) $(OVL46) $(OVL47) \
       $(OVL48) $(OVL49) $(OVL50) $(OVL51) $(OVL52) $(OVL53) $(OVL54) $(OVL55) \
       $(OVL56) $(OVL57) $(OVL58) $(OVL59) $(OVL60) $(OVL61) $(OVL62)

# nethack.exe dependencies, non-overlay link list
HOBJ =	$(VOBJ) $(OVL63)

# overlay link list
OVERLAYS = $(ROOT)    ($(OVL01)) ($(OVL02)) ($(OVL03)) ($(OVL04)) ($(OVL05)) \
	   ($(OVL06)) ($(OVL07)) ($(OVL08)) ($(OVL09)) ($(OVL10)) ($(OVL11)) \
	   ($(OVL12)) ($(OVL13)) ($(OVL14)) ($(OVL15)) ($(OVL16)) ($(OVL17)) \
	   ($(OVL18)) ($(OVL19)) ($(OVL20)) ($(OVL21)) ($(OVL22)) ($(OVL23)) \
	   ($(OVL24)) ($(OVL25)) ($(OVL26)) ($(OVL27)) ($(OVL28)) ($(OVL29)) \
	   ($(OVL30)) ($(OVL31)) ($(OVL32)) ($(OVL33)) ($(OVL34)) ($(OVL35)) \
	   ($(OVL36)) ($(OVL37)) ($(OVL38)) ($(OVL39)) ($(OVL40)) ($(OVL41)) \
	   ($(OVL42)) ($(OVL43)) ($(OVL44)) ($(OVL45)) ($(OVL46)) ($(OVL47)) \
	   ($(OVL48)) ($(OVL49)) ($(OVL50)) ($(OVL51)) ($(OVL52)) ($(OVL53)) \
	   ($(OVL54)) ($(OVL55)) ($(OVL56)) ($(OVL57)) ($(OVL58)) ($(OVL59)) \
	   ($(OVL60)) ($(OVL61)) ($(OVL62)) ($(OVL63))

# header dependencies

PCCONF_H   = $(INCL)\$(TARG)conf.h  $(INCL)\msdos.h	$(INCL)\system.h
GLOBAL_H   = $(INCL)\global.h	    $(INCL)\coord.h	$(PCCONF_H)
CONFIG_H   = $(INCL)\config.h	    $(INCL)\tradstdc.h	$(GLOBAL_H)
TRAP_H	   = $(INCL)\trap.h
PERMONST_H = $(INCL)\permonst.h     $(INCL)\monflag.h
YOU_H	   = $(INCL)\you.h	    $(INCL)\attrib.h	$(PERMONST_H) \
	     $(INCL)\mondata.h	    $(INCL)\monst.h	$(INCL)\youprop.h
DECL_H	   = $(INCL)\spell.h	    $(INCL)\obj.h	$(YOU_H) \
	     $(INCL)\onames.h	    $(INCL)\pm.h
HACK_H	   = $(CONFIG_H)	    $(DECL_H)		$(INCL)\monsym.h \
	     $(INCL)\mkroom.h	    $(INCL)\objclass.h	$(INCL)\gold.h \
	     $(INCL)\trap.h	    $(INCL)\flag.h	$(INCL)\rm.h \
	     $(OVERLAY_H)

## extern.h, and decl.h contain only external declarations.
##
## If anything in them changes, all other files involving the changed routines
## should be changed to reflect them.  Including them in their respective
## dependency lists will make sure everything is correct, but causes frequent
## near-total recompiles.  By leaving them out, we allow quicker testing of
## changes, but we presume the wiz knows to be circumspect.


###
### Main targets
###

$(GAME): $(GAMEFILE) $(GAMEDIR)\data $(GAMEDIR)\rumors

$(GAMEFILE): $(GAMEDIR) $(OBJ) $(HOBJ) $(TERMLIB) Makefile
	@echo Linking...
	if exist $@ del $@
        $(LINK) $(C0) $(LINK_LIST),$@,,$(LIBS) $(TERMLIB) $(LFLAGS);
	@echo NetHack is up to date.

all:	$(GAME) install
	@echo Done.

$(OBJ):
	mkdir $(OBJ)

$(GAMEDIR):
	mkdir $(GAMEDIR)
	mkdir $(GAMEDIR)\bones


###
### makedefs.exe
###

makedefs.exe:  $(MAKEOBJS)
	@$(TLINK) $(TLFLAGS) $(C0) $(MAKEOBJS),$@,,$(LIBS);

$(OBJ)\makedefs.obj:  $(INCL)\config.h $(INCL)\permonst.h $(INCL)\objclass.h


###
### makedefs-generated files
###

# date.h should be remade any time any of the source is modified
$(INCL)\date.h: 	makedefs.exe $(VOBJ)
	makedefs -v

$(INCL)\trap.h: 	makedefs.exe
	makedefs -t

$(INCL)\onames.h:	makedefs.exe
	makedefs -o

$(INCL)\pm.h:		makedefs.exe
	makedefs -p

$(GAMEDIR)\data:	makedefs.exe $(AUX)\data.base
	makedefs -d
	xcopy $(AUX)\data $(GAMEDIR)
	del $(AUX)\data

$(GAMEDIR)\rumors:	makedefs.exe $(AUX)\rumors.tru $(AUX)\rumors.fal
	makedefs -r
	xcopy $(AUX)\rumors $(GAMEDIR)
	del $(AUX)\rumors


###
### lev_comp.exe
###

lev_comp.exe:  $(SPLEVOBJS)
	$(TLINK) $(TLFLAGS) $(C0) $(SPLEVOBJS),$@,,$(LIBS);

## Note: UNIX yacc may generate a line reading "#", which Turbo C 2.0, despite
##	 the manual's claims that it should be ignored, treats as an error.
##	 You may have to remove such a line to compile lev_comp.c.
$(OBJ)\lev_comp.obj:  $(HACK_H) $(INCL)\sp_lev.h
	$(CC) $(CFLAGS) -A- $*.c
$(OBJ)\lev_lex.obj:  $(INCL)\lev_comp.h $(HACK_H) $(INCL)\sp_lev.h
$(OBJ)\lev_main.obj:  $(HACK_H) $(INCL)\sp_lev.h

# If you have yacc or lex programs, and make any changes,
# add some .y.c and .l.c rules to your Make.ini.
#
#lev_comp.c:  lev_comp.y
#lev_lex.c:  lev_comp.l


###
### termlib.lib
###

#$(TERMLIB): $(TERMOBJS)
#	tlib $(TERMLIB) /C $(TERMLIST);


###
### Secondary targets
###

install:  $(GAMEDIR)\NetHack.cnf $(GAMEDIR)\record $(GAMEDIR)\termcap spec_levs
	xcopy $(AUX)\*. $(GAMEDIR)
	@echo Auxiliary files installed.

$(GAMEDIR)\NetHack.cnf:
	xcopy $(OTHERS)\NetHack.cnf $(GAMEDIR)
$(GAMEDIR)\record:
	touch $(GAMEDIR)\record
$(GAMEDIR)\termcap:
	xcopy $(OTHERS)\termcap $(GAMEDIR)

spec_levs: $(AUX)\castle.des $(AUX)\endgame.des $(AUX)\tower.des lev_comp.exe
	lev_comp $(AUX)\castle.des
	lev_comp $(AUX)\endgame.des
	lev_comp $(AUX)\tower.des
	chdir $(AUX)
	xcopy castle $(GAMEDIR)
	del castle
	xcopy endgame $(GAMEDIR)
	del endgame
	xcopy tower? $(GAMEDIR)
	del tower?
	chdir $(SRC)
	@echo Special levels compiled.

clean:
	del $(OBJ)\*.obj
	rmdir $(OBJ)

spotless: clean
	del $(INCL)\date.h
	del $(INCL)\onames.h
	del $(INCL)\pm.h
	del makedefs.exe
	if exist lev_comp.exe del lev_comp.exe


###
### Other dependencies
###

# OS-dependent filenames
$(OBJ)\main.obj:     $(HACK_H) $(TARG)main.c
	$(CC) $(CFLAGS) -o$@ $(TARG)main.c

$(OBJ)\tty.obj:      $(HACK_H) $(INCL)\func_tab.h $(TARG)tty.c
	$(CC) $(CFLAGS) -o$@ $(TARG)tty.c

$(OBJ)\unix.obj:     $(HACK_H) $(TARG)unix.c
	$(CC) $(CFLAGS) -o$@ $(TARG)unix.c

# GO AHEAD, DELETE THIS LINE

$(OBJ)\allmain.obj:	$(HACK_H)
$(OBJ)\alloc.obj:	$(CONFIG_H)
$(OBJ)\apply.obj:	$(HACK_H)   $(INCL)\edog.h
$(OBJ)\artifact.obj:	$(HACK_H)   $(INCL)\artifact.h
$(OBJ)\attrib.obj:	$(HACK_H)
$(OBJ)\bones.obj:	$(HACK_H)
$(OBJ)\cmd.obj: 	$(HACK_H)   $(INCL)\func_tab.h
$(OBJ)\dbridge.obj:	$(HACK_H)
$(OBJ)\decl.obj:	$(HACK_H)
$(OBJ)\demon.obj:	$(HACK_H)
$(OBJ)\do.obj:		$(HACK_H)
$(OBJ)\do_name.obj:	$(HACK_H)
$(OBJ)\do_wear.obj:	$(HACK_H)
$(OBJ)\dog.obj: 	$(HACK_H)   $(INCL)\edog.h
$(OBJ)\dogmove.obj:	$(HACK_H)   $(INCL)\mfndpos.h	 $(INCL)\edog.h
$(OBJ)\dokick.obj:	$(HACK_H)
$(OBJ)\dothrow.obj:	$(HACK_H)
$(OBJ)\eat.obj: 	$(HACK_H)
$(OBJ)\end.obj: 	$(HACK_H)   $(INCL)\eshk.h
$(OBJ)\engrave.obj:	$(HACK_H)
$(OBJ)\exper.obj:	$(HACK_H)
$(OBJ)\extralev.obj:	$(HACK_H)
$(OBJ)\fountain.obj:	$(HACK_H)
$(OBJ)\getline.obj:	$(HACK_H)   $(INCL)\func_tab.h
$(OBJ)\hack.obj:	$(HACK_H)
$(OBJ)\invent.obj:	$(HACK_H)   $(INCL)\lev.h	 $(INCL)\wseg.h
$(OBJ)\ioctl.obj:	$(HACK_H)
$(OBJ)\lev_comp.obj:	$(HACK_H)   $(INCL)\sp_lev.h
$(OBJ)\lev_lex.obj:	$(HACK_H)   $(INCL)\sp_lev.h	 $(INCL)\lev_comp.h
$(OBJ)\lev_main.obj:	$(HACK_H)   $(INCL)\sp_lev.h
$(OBJ)\lock.obj:	$(HACK_H)
$(OBJ)\makemon.obj:	$(HACK_H)
$(OBJ)\mail.obj:	$(HACK_H)
$(OBJ)\mcastu.obj:	$(HACK_H)
$(OBJ)\mhitm.obj:	$(HACK_H)   $(INCL)\artifact.h
$(OBJ)\mhitu.obj:	$(HACK_H)   $(INCL)\artifact.h	 $(INCL)\edog.h
$(OBJ)\mklev.obj:	$(HACK_H)
$(OBJ)\mkmaze.obj:	$(HACK_H)
$(OBJ)\mkobj.obj:	$(HACK_H)
$(OBJ)\mkroom.obj:	$(HACK_H)
$(OBJ)\mon.obj: 	$(HACK_H)   $(INCL)\mfndpos.h	 $(INCL)\wseg.h
$(OBJ)\mondata.obj:	$(HACK_H)   $(INCL)\eshk.h	 $(INCL)\epri.h
$(OBJ)\monmove.obj:	$(HACK_H)   $(INCL)\mfndpos.h	 $(INCL)\artifact.h
$(OBJ)\monst.obj:	$(CONFIG_H) $(PERMONST_H)	 $(INCL)\eshk.h \
				    $(INCL)\vault.h	 $(INCL)\epri.h
$(OBJ)\msdos.obj:	$(HACK_H) msdos.c
	$(CC) $(CFLAGS) -A- $*.c
# set ANSI only off -- many MS-DOS specific things.
$(OBJ)\mthrowu.obj:	$(HACK_H)
$(OBJ)\music.obj:	$(HACK_H)
$(OBJ)\o_init.obj:	$(HACK_H)   $(INCL)\onames.h
$(OBJ)\objects.obj:	$(CONFIG_H) $(INCL)\obj.h	 $(INCL)\objclass.h \
				    $(INCL)\prop.h
$(OBJ)\objnam.obj:	$(HACK_H)
$(OBJ)\options.obj:	$(HACK_H)
$(OBJ)\pager.obj:	$(HACK_H)
$(OBJ)\panic.obj:	$(CONFIG_H)
$(OBJ)\pickup.obj:	$(HACK_H)
$(OBJ)\polyself.obj:	$(HACK_H)
$(OBJ)\potion.obj:	$(HACK_H)
$(OBJ)\pray.obj:	$(HACK_H)
$(OBJ)\pri.obj: 	$(HACK_H)   $(INCL)\epri.h	 $(INCL)\termcap.h
$(OBJ)\priest.obj:	$(HACK_H)   $(INCL)\mfndpos.h	 $(INCL)\eshk.h \
				    $(INCL)\epri.h
$(OBJ)\prisym.obj:	$(HACK_H)   $(INCL)\lev.h	 $(INCL)\wseg.h
$(OBJ)\random.obj:
$(OBJ)\read.obj:	$(HACK_H)
$(OBJ)\restore.obj:	$(HACK_H)   $(INCL)\lev.h	 $(INCL)\wseg.h
$(OBJ)\rip.obj:
	$(CC) $(CFLAGS) -d- $*.c
# must not merge strings, or the tombstone lines will overlap
$(OBJ)\rnd.obj: 	$(HACK_H)
$(OBJ)\rumors.obj:	$(HACK_H)
$(OBJ)\save.obj:	$(HACK_H)   $(INCL)\lev.h	 $(INCL)\wseg.h
$(OBJ)\search.obj:	$(HACK_H)   $(INCL)\artifact.h
$(OBJ)\shk.obj: 	$(HACK_H)   $(INCL)\eshk.h
$(OBJ)\shknam.obj:	$(HACK_H)   $(INCL)\eshk.h
$(OBJ)\sit.obj: 	$(HACK_H)
$(OBJ)\sounds.obj:	$(HACK_H)   $(INCL)\edog.h	 $(INCL)\eshk.h
$(OBJ)\sp_lev.obj:	$(HACK_H)   $(INCL)\sp_lev.h
$(OBJ)\spell.obj:	$(HACK_H)
$(OBJ)\steal.obj:	$(HACK_H)
$(OBJ)\termcap.obj:	$(HACK_H)   $(INCL)\termcap.h
$(OBJ)\timeout.obj:	$(HACK_H)
$(OBJ)\topl.obj:	$(HACK_H)
$(OBJ)\topten.obj:	$(HACK_H)
$(OBJ)\track.obj:	$(HACK_H)
$(OBJ)\trampoli.obj:	$(HACK_H)
$(OBJ)\trap.obj:	$(HACK_H)   $(INCL)\edog.h
$(OBJ)\u_init.obj:	$(HACK_H)
$(OBJ)\uhitm.obj:	$(HACK_H)   $(INCL)\artifact.h
$(OBJ)\vault.obj:	$(HACK_H)   $(INCL)\vault.h
$(OBJ)\version.obj:	$(HACK_H)   $(INCL)\date.h	 $(INCL)\patchlev.h
$(OBJ)\weapon.obj:	$(HACK_H)
$(OBJ)\were.obj:	$(HACK_H)
$(OBJ)\wield.obj:	$(HACK_H)
$(OBJ)\wizard.obj:	$(HACK_H)
$(OBJ)\worm.obj:	$(HACK_H)   $(INCL)\wseg.h
$(OBJ)\worn.obj:	$(HACK_H)
$(OBJ)\write.obj:	$(HACK_H)
$(OBJ)\zap.obj: 	$(HACK_H)
