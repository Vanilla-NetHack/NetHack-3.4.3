#	NetHack Makefile.
#	SCCS Id: @(#)Makefile.doc	3.1	90/05/28

GUIDEBOOK = Guidebook		# regular ASCII file
#GUIDEBOOK = Guidebook.ps	# PostScript file
#GUIDEBOOK = Guidebook.dvi	# TeX device-independent file

# the basic guidebook
Guidebook:	Guidebook.mn
	tbl tmac.n Guidebook.mn | nroff | col > Guidebook

# Fancier output for those with ditroff, psdit and a PostScript printer.
Guidebook.ps:	Guidebook.mn
	tbl tmac.n Guidebook.mn | ditroff | psdit > Guidebook.ps

# Guidebook.tex is the same as Guidebook.mn but formatted with LaTeX.
# - The invocation command for LaTeX may vary in different installations.
# - To print Guidebook.dvi you need to use a suitable dvi-driver.
Guidebook.dvi:	Guidebook.tex
	latex Guidebook.tex


GAME	= nethack
MANDIR	= /usr/man/man6
MANEXT	= 6

# manual installation for most BSD-style systems
GAMEMANCREATE = cp nethack.6
LEVMANCREATE = cp lev_comp.6
DGNMANCREATE = cp dgn_comp.6
RCVRMANCREATE = cp recover.6
# manual installation for most SYSV-style systems
# GAMEMANCREATE = nroff -man nethack.6 >
# LEVMANCREATE = nroff -man lev_comp.6 >
# DGNMANCREATE = nroff -man dgn_comp.6 >
# RCVRMANCREATE = nroff -man recover.6 >

manpages:
	-$(GAMEMANCREATE) $(MANDIR)/$(GAME).$(MANEXT)
	-$(LEVMANCREATE) $(MANDIR)/lev_comp.$(MANEXT)
	-$(DGNMANCREATE) $(MANDIR)/dgn_comp.$(MANEXT)
	-$(RCVRMANCREATE) $(MANDIR)/recover.$(MANEXT)


spotless:
	-rm -f Guidebook Guidebook.ps Guidebook.dvi Guidebook.aux Guidebook.log
