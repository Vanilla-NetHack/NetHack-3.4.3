#
#	SCCS Id: @(#)Makefile.tcc	1.4	87/08/08
# 	Makefile for NetHack (PC) version 1.0 written using
#	Turbo C v1.0
# 
# Unfortunately, large model is limited to a total of 64K global data
# Huge memory model, remove stack probes, optimize for space:
WIZARD=
V = 14
CFLAGS = -m$(MODEL) -DLINT_ARGS -DVER=$(V) $(WIZARD) -N- -Z -K- -O -w-pro -w-nod

# The game name
GAME = hack.exe

# The game directory
GAMEDIR = .

# The directory containing the libraries
LIBDIR = c:\c\lib

# All object modules
OBJS = decl.obj apply.obj bones.obj cmd.obj do.obj dothrow.obj\
	do_name.obj do_wear.obj dog.obj dogmove.obj eat.obj end.obj \
	engrave.obj fight.obj fountain.obj hack.obj invent.obj \
	lev.obj main.obj makemon.obj mhitu.obj mklev.obj \
	mkmaze.obj mkobj.obj mkshop.obj mon.obj monmove.obj\
	monst.obj o_init.obj objnam.obj options.obj \
	pager.obj polyself.obj potion.obj pray.obj pri.obj prisym.obj\
	read.obj rip.obj rumors.obj save.obj \
	search.obj shk.obj shknam.obj sit.obj spell.obj steal.obj \
	termcap.obj timeout.obj topl.obj topten.obj track.obj trap.obj \
	tty.obj unix.obj u_init.obj vault.obj wield.obj \
	wizard.obj worm.obj worn.obj write.obj zap.obj \
	version.obj rnd.obj alloc.obj msdos.obj

# The main target
#
$(GAME) : $(OBJS)
	link /x:400 $(LIBDIR)\c0$(MODEL).obj @objs.lnk,$(GAME),,$(LIBDIR)\c$(MODEL).lib

#	variable auxilary files.
#
VARAUX = data rumors

install : $(GAME) $(VARAUX)
	- exepack $(GAME) $(GAMEDIR)\$(GAME)
	- exemod $(GAMEDIR)\$(GAME) /max 1

clean :
	erase $(GAME)

spotless: clean
	erase *.obj
	erase main.c
	erase tty.c
	erase unix.c

srcs :
	copy makefile \tmp
	copy *.c \tmp
	copy *.h \tmp
	copy \local\make\make.doc \tmp
	copy \local\make\make.ini \tmp
	copy \bin\make.exe \tmp
	cd \tmp
	time
	touch *.*
	arc m hack$Vs * *.*
	cd $(CWD)


#	Other dependencies
#
RUMORFILES= rumors.bas rumors.kaa rumors.mrx

makedefs.exe:	makedefs.c alloc.obj config.h
	cl -AL makedefs.c alloc.obj


rumors :  config.h $(RUMORFILES) makedefs.exe
	makedefs.exe -r

data :  config.h data.bas makedefs.exe
	makedefs.exe -d

onames.h :  config.h objects.h makedefs.exe
	makedefs.exe -o

#	Below is a kluge.  date.h should actually depend on any source
#	module being changed. (but hack.h is close enough for most).
#
date.h :  hack.h makedefs.exe
	makedefs.exe -D

trap.h :  config.h makedefs.exe
	makedefs.exe -t

main.obj : main.c

main.c :  pcmain.c hack.h
	copy pcmain.c main.c
	touch main.c

tty.obj : tty.c

tty.c :  pctty.c hack.h msdos.h
	copy pctty.c tty.c
	touch tty.c

unix.obj : unix.c

unix.c :  pcunix.c hack.h mkroom.h
	copy pcunix.c unix.c
	touch unix.c

decl.obj :  hack.h mkroom.h
apply.obj :  hack.h edog.h mkroom.h
bones.obj :  hack.h
hack.obj :  hack.h
cmd.obj :  hack.h func_tab.h msdos.h
do.obj :  hack.h
do_name.obj :  hack.h
do_wear.obj :  hack.h
dog.obj :  hack.h edog.h mkroom.h
dogmove.obj :  hack.h mfndpos.h
dothrow.obj :  hack.h
eat.obj :  hack.h
end.obj :  hack.h
engrave.obj :  hack.h
fight.obj :  hack.h
fountain.obj :  hack.h
invent.obj :  hack.h wseg.h
ioctl.obj :  config.h
lev.obj :  hack.h mkroom.h wseg.h
makemon.obj :  hack.h
mhitu.obj :  hack.h
mklev.obj :  hack.h mkroom.h
mkmaze.obj :  hack.h mkroom.h
mkobj.obj :  hack.h
mkshop.obj :  hack.h mkroom.h eshk.h
mon.obj :  hack.h mfndpos.h
monmove.obj :  hack.h mfndpos.h
monst.obj :  hack.h eshk.h
o_init.obj :  config.h objects.h onames.h
objnam.obj :  hack.h
options.obj :  config.h hack.h
pager.obj :  hack.h
polyself.obj :  hack.h
potion.obj :  hack.h
pray.obj :  hack.h
pri.obj :  hack.h
prisym.obj :  hack.h wseg.h
read.obj :  hack.h
rip.obj :  hack.h
	tcc -c $(CFLAGS) -d- rip.c
rumors.obj :  config.h
save.obj :  hack.h
search.obj :  hack.h
shk.obj :  hack.h mfndpos.h mkroom.h eshk.h
shknam.obj :  hack.h
sit.obj :  hack.h
spell.obj:  hack.h
steal.obj :  hack.h
termcap.obj :  config.h flag.h
timeout.obj :  hack.h
topl.obj :  hack.h
topten.obj :  hack.h
track.obj :  hack.h
trap.obj :  hack.h mkroom.h
u_init.obj :  hack.h
vault.obj :  hack.h mkroom.h
version.obj : hack.h date.h
wield.obj :  hack.h
wizard.obj :  hack.h
worm.obj :  hack.h wseg.h
worn.obj :  hack.h
write.obj :  hack.h
zap.obj :  hack.h
msdos.obj : msdos.h
extern.h: config.h
	touch extern.h
hack.h :  config.h objclass.h monst.h gold.h trap.h obj.h flag.h rm.h permonst.h onames.h spell.h extern.h you.h
	touch hack.h
objects.h :  config.h objclass.h
	touch objects.h
