#	Hack Makefile.
#	SCCS Id: @(#)Makefile.top	3.0	89/03/23

# newer makes predefine $(MAKE) to 'make' and do smarter processing of
# recursive make calls if $(MAKE) is used
# these makes allow $(MAKE) to be overridden by the environment if someone
# wants to (or has to) use something other than the standard make, so we do
# not want to unconditionally set $(MAKE) here
#
# unfortunately, some older makes do not predefine $(MAKE); if you have one of
# these, uncomment the following line
# (you will know that you have one if you get complaints about unable to
# execute things like 'data' and 'rumors')
# MAKE = make

# make NetHack
GAME     = nethack
GAMEUID  = games
GAMEGRP  = bin

# GAMEDIR also appears in config.h as "HACKDIR".
#
# note that 'make install' believes in creating a nice tidy GAMEDIR for
# installation, free of debris from previous NetHack versions --
# therefore there should not be anything in GAMEDIR that you want to keep
# (if there is, you'll have to do the installation by hand or modify the
# instructions)
GAMEDIR  = /usr/games/lib/$(GAME)dir
SHELLDIR = /usr/games
MANDIR   = /usr/man/man6
MANEXT   = 6

# manual installation for most BSD-style systems
GAMEMANCREATE = cp auxil/nethack.6
LEVMANCREATE = cp auxil/lev_comp.6
# manual installation for most SYSV-style systems
# GAMEMANCREATE = nroff -man auxil/nethack.6 >
# LEVMANCREATE = nroff -man auxil/lev_comp.6 >

AUXHELP = help hh cmdhelp history opthelp oracles
VARAUX = data rumors
SPEC_LEVS = endgame castle tower1 tower2 tower3
AUX = $(AUXHELP) $(VARAUX) $(SPEC_LEVS) license

$(GAME):
	( cd src ; $(MAKE) )

all:	$(GAME) Guidebook data rumors spec_levs
	@echo "Done."

Guidebook:
	( cd auxil ; $(MAKE) Guidebook )

data:
	( cd auxil ; $(MAKE) data )

rumors:
	( cd auxil ; $(MAKE) rumors )

spec_levs:
	( cd src ; $(MAKE) lev_comp )
	( cd auxil ; $(MAKE) spec_levs )
	
update: $(GAME) data rumors spec_levs
# copy over new versions of the game files
	( cd auxil ; cp $(AUX) $(GAMEDIR) )
#	(don't yank the old version out from under people who're playing it)
	-mv $(GAMEDIR)/$(GAME) $(GAMEDIR)/$(GAME).old
	cp src/$(GAME) $(GAMEDIR)
	-rm -f $(SHELLDIR)/$(GAME)
	sed -e 's;/usr/games/lib/nethackdir;$(GAMEDIR);' \
		-e 's;HACKDIR/nethack;HACKDIR/$(GAME);' \
		< auxil/nethack.sh \
		> $(SHELLDIR)/$(GAME)
# correct the permissions on the new versions
	-( cd $(GAMEDIR) ; chown $(GAMEUID) $(AUX) $(GAME) ; \
			chgrp $(GAMEGRP) $(AUX) $(GAME) ; \
			chmod 644 $(AUX) )
	-chown $(GAMEUID) $(SHELLDIR)/$(GAME)
	chgrp $(GAMEGRP) $(SHELLDIR)/$(GAME)
	chmod 0755 $(SHELLDIR)/$(GAME)
	chmod 04755 $(GAMEDIR)/$(GAME)
# touch time-sensitive files
	-touch -c $(GAMEDIR)/bones* $(GAMEDIR)/?lock* $(GAMEDIR)/wizard*
	-touch -c $(GAMEDIR)/save/*
	touch $(GAMEDIR)/perm $(GAMEDIR)/record
# and copy the man pages
	-$(GAMEMANCREATE) $(MANDIR)/$(GAME).$(MANEXT)
	-$(LEVMANCREATE) $(MANDIR)/lev_comp.$(MANEXT)

install: $(GAME) data rumors spec_levs
# set up the directories
	-mkdir $(SHELLDIR)
	-rm -rf $(GAMEDIR)
	-mkdir $(GAMEDIR) $(GAMEDIR)/save
# create some files
	touch $(GAMEDIR)/perm $(GAMEDIR)/record $(GAMEDIR)/logfile
# copy over the game files
	( cd auxil ; cp $(AUX) $(GAMEDIR) )
	cp src/$(GAME) $(GAMEDIR)
	-rm -f $(SHELLDIR)/$(GAME)
	sed -e 's;/usr/games/lib/nethackdir;$(GAMEDIR);' \
		-e 's;HACKDIR/nethack;HACKDIR/$(GAME);' \
		< auxil/nethack.sh \
		> $(SHELLDIR)/$(GAME)
# set up the permissions
	-chown $(GAMEUID) $(SHELLDIR)/$(GAME) $(GAMEDIR) $(GAMEDIR)/*
	chgrp $(GAMEGRP) $(SHELLDIR)/$(GAME) $(GAMEDIR) $(GAMEDIR)/*
	chmod 0755 $(SHELLDIR)/$(GAME)
	chmod 644 $(GAMEDIR)/*
	chmod 755 $(GAMEDIR) $(GAMEDIR)/save
	chmod 04755 $(GAMEDIR)/$(GAME)
# and copy the man pages
	-$(GAMEMANCREATE) $(MANDIR)/$(GAME).$(MANEXT)
	-$(LEVMANCREATE) $(MANDIR)/lev_comp.$(MANEXT)

# 'make clean' removes all the .o files, but leaves around all the executables
# and compiled data files
clean:
	( cd src ; $(MAKE) clean )

# 'make spotless' returns the source tree to near-distribution condition.
# it removes .o files, executables, and compiled data files
spotless:
	( cd src ; $(MAKE) spotless )
	( cd auxil ; $(MAKE) spotless )
