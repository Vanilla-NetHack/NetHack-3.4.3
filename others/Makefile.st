#	SCCS Id: @(#)Makefile.st	3.0	90/01/14
#	ST NetHack 3.0 Makefile for GCC 1.34 or higher
#
#    NOTE: There's a bug in the GCC 1.35 (and maybe 1.34) that
#    requires the omission of the -O flag on a couple of files;
#    see the comments further down.
#
#    Also: There's one really awful kludge here: I had to break
#    monst.o up into two pieces to get it to compile on my machine
#    (a Mega 2). You'll need "sed" to get this to work.
#    If you have 2.5 megs or more, you can probably delete the
#    strange dependency lines for monst.o and just compile
#    it directly.
#
CC	= d:\gnu\bin\gcc.ttp
LD	= d:\gnu\bin\gcc-ld.ttp
YACC	= d:\gnu\bin\bison.ttp -y
LEX	= d:\gnu\bin\flex.ttp
#
# for 32 bit integers (slow, but type mismatches are no problem)
# MODEL	=
# TERMCAP = -lcurses
#
# for 16 bit integers (faster, but more finicky)
MODEL	= -mshort
TERMCAP = -lcurses16

# Directories (makedefs hardcodes these, don't change them)
INCL	= ..\include
AUX	= ..\auxil
SRC	= ..\src

#
# Use the following line for maximum warnings
WARN=-D__GNULINT__ -W -Wimplicit -Wreturn-type -Wunused -Wpointer-arith \
 -Wcast-qual -Wwrite-strings
#
# WARN =
#
CFLAGS = $(MODEL) -O -fomit-frame-pointer $(WARN) -I..\include
LFLAGS	= $(MODEL) -s
TARG	= tos
PC	= pc

# Optional high-quality BSD random number generation routines (see tosconf.h).
# Set to nothing if not used.
RANDOM = random.o

# The game name
GAME= nethack

# The game directory
GAMEDIR = \games\$(GAME)

# The game filename
GAMEFILE = $(GAMEDIR)\$(GAME).ttp

# object files for makedefs
MAKEOBJS = makedefs.o alloc.o monst.o objects.o panic.o

# object files for special levels compiler
SPLEVOBJS = lev_comp.o lev_lex.o \
	lev_main.o alloc.o monst.o objects.o panic.o

# nothing below this line should have to be changed
#
# other things that have to be reconfigured are in config.h,
# {unixconf.h, pcconf.h, tosconf.h}, and possibly system.h

VOBJ = allmain.o alloc.o apply.o artifact.o attrib.o bones.o cmd.o \
	dbridge.o decl.o \
	demon.o do.o do_name.o do_wear.o dog.o dogmove.o dokick.o dothrow.o \
 	eat.o end.o engrave.o exper.o extralev.o fountain.o getline.o hack.o \
	invent.o lock.o main.o makemon.o mcastu.o mhitm.o \
	mhitu.o mklev.o mkmaze.o mkobj.o mkroom.o mon.o mondata.o monmove.o \
	monst.o  mthrowu.o msdos.o music.o o_init.o objects.o \
	objnam.o options.o pager.o \
	pickup.o polyself.o potion.o pray.o pri.o priest.o prisym.o read.o \
	restore.o rip.o rnd.o rumors.o save.o search.o shk.o shknam.o sit.o \
	sounds.o sp_lev.o spell.o steal.o termcap.o timeout.o topl.o topten.o \
	track.o trap.o tty.o u_init.o uhitm.o unix.o vault.o \
	weapon.o were.o wield.o wizard.o worm.o worn.o write.o zap.o $(RANDOM)

HOBJ =	$(VOBJ) version.o

PCCONF_H   = $(INCL)\$(TARG)conf.h $(INCL)\msdos.h $(INCL)\system.h \
	     $(INCL)\extern.h 
GLOBAL_H   = $(INCL)\global.h $(INCL)\coord.h $(PCCONF_H)
CONFIG_H   = $(INCL)\config.h $(INCL)\tradstdc.h $(GLOBAL_H)
TRAP_H	   = $(INCL)\trap.h
PERMONST_H = $(INCL)\permonst.h $(INCL)\monflag.h
YOU_H	   = $(INCL)\you.h $(INCL)\attrib.h $(PERMONST_H) $(INCL)\mondata.h \
	     $(INCL)\monst.h $(INCL)\youprop.h
DECL_H	   = $(INCL)\decl.h $(INCL)\spell.h $(INCL)\obj.h $(YOU_H) \
	     $(INCL)\onames.h $(INCL)\pm.h
HACK_H	   = $(CONFIG_H) $(DECL_H) $(INCL)\monsym.h $(INCL)\mkroom.h \
	     $(INCL)\objclass.h $(INCL)\gold.h $(INCL)\trap.h $(INCL)\flag.h \
	     $(INCL)\rm.h

# The main target
$(GAME).ttp: $(HOBJ)
	$(CC) -o $(GAME).ttp $(LFLAGS) $(HOBJ) $(TERMCAP)

$(GAME): $(GAME).ttp

all:	$(GAME) lev_comp.ttp auxil
	@echo Done.

makedefs.ttp:	$(MAKEOBJS)
	$(CC) $(LFLAGS) -o makedefs.ttp $(MAKEOBJS)

makedefs.o:  $(INCL)\config.h $(INCL)\permonst.h $(INCL)\objclass.h

lev_comp.ttp:  $(SPLEVOBJS)
	$(CC) $(LFLAGS) -o lev_comp.ttp $(SPLEVOBJS)

lev_comp.o:  $(HACK_H) $(INCL)\sp_lev.h
lev_lex.o:  $(INCL)\lev_comp.h $(HACK_H) $(INCL)\sp_lev.h
lev_main.o:  $(HACK_H) $(INCL)\sp_lev.h

# If you have yacc or lex programs, and make any changes,
# you'll need rules like these. The names of the output
# files (e.g. lex,yy.c) vary widely among different ST implementations
# of YACC and LEX.
#
# Also: for some reason the supplied lev_lex.c causes the GCC to abort
# on my machine (maybe not enough memory?) when compiled with -mshort.
# With MODEL=-mshort, I had to use the lev_lex.c from others.
#
lev_comp.c:  lev_comp.y
	$(YACC) -d lev_comp.y
	cp y,tab.c lev_comp.c
	cp y,tab.h ..\include\lev_comp.h
	rm y,tab.c y,tab.h

lev_lex.c:  lev_comp.l
	$(LEX) lev_comp.l
	cp lex,yy.c lev_lex.c
	rm lex,yy.c
#
#	The following include files depend on makedefs to be created.
#
#	date.h should be remade any time any of the source or include code
#	is modified.
#
$(INCL)\date.h:	 $(VOBJ) makedefs.ttp
	.\makedefs -v

$(INCL)\trap.h:	 makedefs.ttp
	.\makedefs -t

$(INCL)\onames.h:  makedefs.ttp
	.\makedefs -o

$(INCL)\pm.h:  makedefs.ttp
	.\makedefs -p

data:	$(AUX)\data.base makedefs.ttp
	.\makedefs -d

rumors: $(AUX)\rumors.tru $(AUX)\rumors.fal makedefs.ttp
	.\makedefs -r

#
#	The following programs vary depending on what OS you are using.
#
main.o:   $(HACK_H) $(PC)main.c
	$(CC) -c $(CFLAGS) $(PC)main.c -o main.o

tty.o:   $(HACK_H) $(INCL)\func_tab.h $(PC)tty.c
	$(CC) -c $(CFLAGS) $(PC)tty.c -o tty.o

unix.o:   $(HACK_H) $(PC)unix.c
	$(CC) -c $(CFLAGS) $(PC)unix.c -o unix.o

#
# GCC 1.35 has trouble with the construct
#	for (x = ...) for(y = ...) levl[x][y].foo = stuff;
# this happens in (at least) bones.c and shknam.c. So if you're using
# 1.35, uncomment the following lines. The problem is fixed in 1.36.
#
#bones.o: $(HACK_H)
#	$(CC) $(MODEL) $(WARN) -I$(INCL) -c bones.c
#shknam.o: $(HACK_H) $(INCL)\eshk.h
#	$(CC) $(MODEL) $(WARN) -I$(INCL) -c shknam.c
#
# Secondary targets
#

AUXMISC = cmdhelp help hh history license opthelp oracles

auxil:	data rumors spec_levs
	cd $(AUX)
	cp data rumors $(GAMEDIR)
	rm data rumors
	cp $(AUXMISC) $(GAMEDIR)

spec_levs: $(AUX)\castle.des $(AUX)\endgame.des $(AUX)\tower.des
	cd $(AUX)
	..\src\lev_comp castle.des
	..\src\lev_comp endgame.des
	..\src\lev_comp tower.des
	cp castle endgame $(GAMEDIR)
	rm castle endgame
	cp tower1 tower2 tower3 $(GAMEDIR)
	rm tower1 tower2 tower3

clean:
	rm *.o

spotless: clean
	cd $(INCL)
	rm date.h
	rm onames.h
	rm pm.h
	touch date.h onames.h pm.h
	cd $(SRC)
	rm makedefs.ttp
	rm lev_comp.ttp

#
# Other dependencies
#

# GO AHEAD, DELETE THIS LINE

allmain.o: $(HACK_H)
alloc.o:  $(CONFIG_H)
apply.o:  $(HACK_H) $(INCL)\edog.h
artifact.o:  $(HACK_H) $(INCL)\artifact.h
attrib.o:  $(HACK_H)
bones.o:  $(HACK_H)
cmd.o:  $(HACK_H) $(INCL)\func_tab.h
dbridge.o: $(HACK_H)
decl.o:  $(HACK_H)
demon.o:  $(HACK_H)
do.o:  $(HACK_H)
do_name.o:  $(HACK_H)
do_wear.o:  $(HACK_H)
dog.o:  $(HACK_H) $(INCL)\edog.h
dogmove.o:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\edog.h
dokick.o:  $(HACK_H)
dothrow.o:  $(HACK_H)
eat.o:  $(HACK_H)
end.o:  $(HACK_H) $(INCL)\eshk.h
engrave.o:  $(HACK_H)
exper.o:  $(HACK_H)
extralev.o:  $(HACK_H)
fountain.o:  $(HACK_H)
getline.o:  $(HACK_H) $(INCL)\func_tab.h
hack.o:  $(HACK_H)
invent.o:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
ioctl.o:  $(HACK_H)
lev_comp.o:  $(HACK_H) $(INCL)\sp_lev.h
lock.o:  $(HACK_H)
makemon.o:  $(HACK_H)
mail.o:  $(HACK_H)
mcastu.o:  $(HACK_H)
mhitm.o:  $(HACK_H) $(INCL)\artifact.h
mhitu.o:  $(HACK_H) $(INCL)\artifact.h $(INCL)\edog.h
mklev.o:  $(HACK_H)
mkmaze.o:  $(HACK_H)
mkobj.o:  $(HACK_H)
mkroom.o:  $(HACK_H)
mon.o:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\artifact.h
mondata.o:  $(HACK_H) $(INCL)\eshk.h $(INCL)\epri.h
monmove.o:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\artifact.h

# kludge for "monst.c: Virtual memory exhausted" errors
# we build monst.s in two pieces, then glue them together with
# "sed". if you don't have memory problems, uncomment out
# all but the line starting "monst.o:". if you don't have sed, get it from
# a comp.binaries.atari.st site, or get gnu sed (the port to
# the atari GCC is easy).

monst.o:  $(CONFIG_H) $(PERMONST_H) $(INCL)\eshk.h $(INCL)\vault.h $(INCL)\epri.h
	$(CC) $(CFLAGS) -S -o monst1.s -DSPLITMON_1 monst.c
	$(CC) $(CFLAGS) -S -o monst2.s -DSPLITMON_2 monst.c
	sed -e s/LC/LD/ -e s/gcc_compiled.:// monst2.s >>monst1.s
	$(CC) $(CFLAGS) -c monst1.s -o monst.o
	rm monst1.s monst2.s

msdos.o:  $(HACK_H)
mthrowu.o:  $(HACK_H)
music.o:  $(HACK_H)
o_init.o:  $(HACK_H) $(INCL)\onames.h
objects.o:  $(CONFIG_H) $(INCL)\obj.h $(INCL)\objclass.h $(INCL)\prop.h
objnam.o:  $(HACK_H)
options.o:  $(HACK_H)
pager.o:  $(HACK_H)
panic.o:  $(CONFIG_H)
pickup.o:  $(HACK_H)
polyself.o:  $(HACK_H)
potion.o:  $(HACK_H)
pray.o:  $(HACK_H)
pri.o:  $(HACK_H) $(INCL)\termcap.h
priest.o:  $(HACK_H) $(INCL)\mfndpos.h $(INCL)\eshk.h $(INCL)\epri.h
prisym.o:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
random.o:
read.o:  $(HACK_H)
restore.o:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
rip.o:  $(HACK_H)
rnd.o:  $(HACK_H)
rumors.o:  $(HACK_H)
save.o:  $(HACK_H) $(INCL)\lev.h $(INCL)\wseg.h
search.o:  $(HACK_H) $(INCL)\artifact.h
shk.o:  $(HACK_H) $(INCL)\eshk.h
shknam.o:  $(HACK_H) $(INCL)\eshk.h
sit.o:  $(HACK_H)
sounds.o:  $(HACK_H) $(INCL)\edog.h $(INCL)\eshk.h
sp_lev.o:  $(HACK_H) $(INCL)\sp_lev.h
spell.o:  $(HACK_H)
steal.o:  $(HACK_H)
termcap.o:  $(HACK_H) $(INCL)\termcap.h
timeout.o:  $(HACK_H)
topl.o:  $(HACK_H)
topten.o:  $(HACK_H)
track.o:  $(HACK_H)
trap.o:  $(HACK_H) $(INCL)\edog.h
u_init.o:  $(HACK_H)
uhitm.o:  $(HACK_H) $(INCL)\artifact.h
vault.o:  $(HACK_H) $(INCL)\vault.h
version.o:  $(HACK_H) $(INCL)\date.h
weapon.o:  $(HACK_H)
were.o:  $(HACK_H)
wield.o:  $(HACK_H)
wizard.o:  $(HACK_H)
worm.o:  $(HACK_H) $(INCL)\wseg.h
worn.o:  $(HACK_H)
write.o:  $(HACK_H)
zap.o:  $(HACK_H)
