# Hack or Quest Makefile.

# on some systems the termcap library is in -ltermcap
TERMLIB = -ltermlib


# make hack
GAME = hack
CAPGAME = HACK
GAMEDIR = ../tmp
CFLAGS = -O
HACKCSRC = hack.Decl.c\
	hack.apply.c hack.bones.c hack.c hack.cmdlist.c hack.do.c\
	hack.do_name.c hack.do_wear.c hack.dog.c hack.eat.c hack.end.c\
	hack.engrave.c hack.fight.c hack.invent.c hack.ioctl.c\
	hack.lev.c hack.main.c hack.makemon.c\
	hack.mkobj.c hack.mhitu.c\
	hack.mon.c hack.monst.c hack.o_init.c hack.objnam.c\
	hack.options.c\
	hack.pri.c hack.read.c hack.rip.c hack.rumors.c\
	hack.save.c\
	hack.search.c hack.shk.c hack.stat.c hack.steal.c\
	hack.termcap.c hack.timeout.c hack.topl.c\
	hack.track.c hack.trap.c hack.tty.c hack.u_init.c hack.vault.c\
	hack.whatis.c hack.wield.c hack.worm.c hack.worn.c hack.zap.c\
	hack.version.c rnd.c alloc.c

CSOURCES = $(HACKCSRC) mklev.c mklv.shk.c mklv.shknam.c mklv.makemaz.c\
	makedefs.c

HSOURCES = savelev.h\
	mklev.h hack.h hack.mfndpos.h config.h\
	def.edog.h def.eshk.h def.func_tab.h def.gen.h def.objclass.h\
	def.monst.h def.obj.h def.permonst.h def.trap.h def.wseg.h\
	def.objects.h

SOURCES = $(CSOURCES) $(HSOURCES)

AUX = data help hh rumors hack.6 hack.sh

HOBJ = hack.Decl.o hack.apply.o hack.bones.o hack.o hack.cmdlist.o hack.do.o\
	hack.do_name.o hack.do_wear.o hack.dog.o hack.eat.o hack.end.o\
	hack.engrave.o hack.fight.o hack.invent.o hack.ioctl.o\
	hack.lev.o hack.main.o hack.makemon.o\
	hack.mkobj.o hack.mhitu.o hack.mon.o\
	hack.monst.o hack.o_init.o hack.objnam.o hack.options.o hack.pri.o\
	hack.read.o hack.rip.o\
	hack.rumors.o hack.save.o\
	hack.search.o\
	hack.shk.o hack.stat.o hack.steal.o hack.termcap.o\
	hack.timeout.o hack.topl.o\
	hack.track.o hack.trap.o\
	hack.tty.o hack.u_init.o hack.vault.o hack.whatis.o hack.wield.o\
	hack.worm.o hack.worn.o hack.zap.o\
	hack.version.o

GOBJ = rnd.o alloc.o

all:	$(GAME) mklev
	@echo "Made Hack."

$(GAME):	$(HOBJ) $(GOBJ) Makefile
	@echo "Loading ..."
	@ld -X -o $(GAME) /lib/crt0.o $(GOBJ) $(HOBJ) $(TERMLIB) -lc

makedefs:	makedefs.c
	cc -o makedefs makedefs.c


hack.onames.h:	makedefs def.objects.h
	makedefs > hack.onames.h
mklv.mkobj.o:	mklev.h hack.onames.h hack.mkobj.c
	rm -f mklv.mkobj.c
	ln hack.mkobj.c mklv.mkobj.c
	cc -c $(CFLAGS) -DMKLEV mklv.mkobj.c
	rm -f mklv.mkobj.c
mklv.makemon.o:	mklev.h hack.makemon.c
	rm -f mklv.makemon.c
	ln hack.makemon.c mklv.makemon.c
	cc -c $(CFLAGS) -DMKLEV mklv.makemon.c
	rm -f mklv.makemon.c
mklv.o_init.o:	hack.o_init.c def.objects.h hack.onames.h
	rm -f mklv.o_init.c
	ln hack.o_init.c mklv.o_init.c
	cc -c $(CFLAGS) -DMKLEV mklv.o_init.c
	rm -f mklv.o_init.c
mklv.engrave.o: hack.engrave.c mklev.h
	rm -f mklv.engrave.c
	ln hack.engrave.c mklv.engrave.c
	cc -c $(CFLAGS) -DMKLEV mklv.engrave.c
	rm -f mklv.engrave.c
mklev.o:
	cc -c $(CFLAGS) -DMKLEV mklev.c
MKLOBJ = mklev.o hack.monst.o mklv.o_init.o mklv.mkobj.o mklv.shk.o\
	mklv.shknam.o mklv.makemon.o mklv.makemaz.o mklv.engrave.o $(GOBJ)
mklev:	$(MKLOBJ)
	cc -o mklev $(MKLOBJ)

lint:	lint_h lint_m
	@echo "Lint done."

lint_h:
# lint cannot have -p here because (i) capitals are meaningful:
# [Ww]izard, (ii) identifiers may coincide in the first six places:
# doweararm() versus dowearring().
# _flsbuf comes from <stdio.h>, a bug in the system libraries.
	lint -axbh -DLINT $(HACKCSRC) | sed '/_flsbuf/d' > olint_h
	cat olint_h | sed '/never used/d;/warning/d'

lint_m:
	lint -axbh -DLINT -DMKLEV mklev.c hack.makemon.c hack.monst.c\
		hack.o_init.c hack.mkobj.c mklv.shk.c mklv.makemaz.c alloc.c\
		rnd.c


print:
	print *.h Makefile  $(AUX)
	print mklev.c mklev.*.c rnd.c alloc.c hack.c
	print hack.Decl.c Hack.magic.C hack.[a-m]*.c
	print hack.[n-z]*.c
	print show.c makedefs.c

diff:
	@- for i in $(SOURCES) $(AUX) ; do \
		echo ; echo diff $D/$$i $$i ; \
		diff $D/$$i $$i ; done

distribution: Makefile
	@- for i in READ_ME $(SOURCES) $(AUX) Makefile date.h hack.onames.h\
		; do \
		cmp -s $$i $D/$$i || \
		( echo cp $$i $D ; cp $$i $D ) ; done
# the distribution directory also contains the empty files
# perm and record, and the informative files Porting and Bugs.


install:
	rm -f /usr/games/$(CAPGAME)
	cp $(GAME) /usr/games/$(CAPGAME)
#	chmod 0710 /usr/games/$(CAPGAME)
	chmod 04711 /usr/games/$(CAPGAME)
	rm -f $(GAMEDIR)/mklev
	cp mklev $(GAMEDIR)/mklev
#	chmod 0750 $(GAMEDIR)/mklev
	chmod 0751 $(GAMEDIR)/mklev
	rm -f $(GAMEDIR)/bones*

depend:
# For the moment we are lazy and disregard /usr/include files because
# the sources contain them conditionally. Perhaps we should use cpp.
#			-e 's,<\(.*\)>,"/usr/include/\1",' \
#
	for i in ${CSOURCES}; do \
		( /bin/grep '^#[ 	]*include' $$i | sed -n \
			-e '/<.*>/d' \
			-e 's/[^"]*"\([^"]*\)".*/\1/' \
			-e H -e '$$g' -e '$$s/\n/ /g' \
			-e '$$s/.*/'$$i': &/' -e '$$s/\.c:/.o:/p' \
			>> makedep); done
	for i in ${HSOURCES}; do \
		( /bin/grep '^#[ 	]*include' $$i | sed -n \
			-e '/<.*>/d' \
			-e 's/[^"]*"\([^"]*\)".*/\1/' \
			-e H -e '$$g' -e '$$s/\n/ /g' \
			-e '$$s/.*/'$$i': &\
				touch '$$i/p \
			>> makedep); done
	@echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	@echo '$$r makedep' >>eddep
	@echo 'w' >>eddep
	@cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	@echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	@echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	@echo '# see make depend above' >> Makefile
	- diff Makefile Makefile.bak
	@rm -f Makefile.bak

# DO NOT DELETE THIS LINE

hack.Decl.o:  hack.h
hack.apply.o:  hack.h def.edog.h
hack.bones.o:  hack.h
hack.o:  hack.h def.trap.h
hack.cmdlist.o:  config.h def.objclass.h def.func_tab.h
hack.dog.o:  hack.h hack.mfndpos.h def.edog.h
hack.eat.o:  hack.h
hack.engrave.o:  mklev.h hack.h
hack.fight.o:  hack.h
hack.invent.o:  hack.h def.wseg.h
hack.main.o:  hack.h
hack.makemon.o:  mklev.h hack.h
hack.mkobj.o:  mklev.h hack.h hack.onames.h
hack.mhitu.o:  hack.h
hack.mon.o:  hack.h hack.mfndpos.h
hack.monst.o:  mklev.h def.eshk.h
hack.o_init.o:  config.h def.objects.h hack.onames.h
hack.objnam.o:  hack.h
hack.options.o:  config.h hack.h
hack.pri.o:  hack.h def.wseg.h
hack.read.o:  hack.h
hack.rumors.o:  config.h
hack.search.o:  hack.h def.trap.h
hack.shk.o:  hack.h hack.mfndpos.h def.eshk.h
hack.steal.o:  hack.h
hack.termcap.o:  config.h
hack.timeout.o:  hack.h
hack.track.o:  hack.h
hack.trap.o:  hack.h def.trap.h
hack.vault.o:  hack.h
hack.whatis.o:  hack.h
hack.wield.o:  hack.h
hack.worm.o:  hack.h def.wseg.h
hack.worn.o:  hack.h
hack.zap.o:  hack.h
hack.version.o:  date.h
mklev.o:  mklev.h def.trap.h hack.onames.h savelev.h
mklv.shk.o:  mklev.h def.eshk.h
mklv.shknam.o:  mklev.h
mklv.makemaz.o:  mklev.h
mklev.h:  config.h def.objclass.h def.monst.h def.gen.h def.obj.h def.permonst.h
			touch mklev.h
hack.h:  mklev.h hack.onames.h
			touch hack.h
def.objects.h:  def.objclass.h
			touch def.objects.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
