#	NetHack Makefile.
#	SCCS Id: @(#)Makefile.auxil	3.0	90/05/28

VARAUX = data rumors
GUIDEBOOK = Guidebook		# regular ASCII file
#GUIDEBOOK = Guidebook.ps	# PostScript file
#GUIDEBOOK = Guidebook.dvi	# TeX device-independent file

all:	$(GUIDEBOOK) $(VARAUX) spec_levs

# The tmac.n macros come with the B news software, which should be available
# at your friendly local archive site if you don't already have it.
# An updated version also comes with the Elm mail distribution.
# They are used here because they are more widely available (if not more
# widely installed) than various proprietary macro packages that came with
# your machine but not your neighbor's.
#
# If you can't install tmac.n in the appropriate system directory, you
# can create the processed Guidebook in stages:
#	tbl Guidebook.mn > Guidebook.tmp
#	nroff tmac.n Guidebook.tmp | col > Guidebook
#	rm Guidebook.tmp
# or simply by saying:
#	tbl tmac.n Guidebook.mn | nroff | col > Guidebook
Guidebook:	Guidebook.mn
	tbl Guidebook.mn | nroff -mn | col > Guidebook

# Fancier output for those with ditroff, psdit and a PostScript printer.
Guidebook.ps:	Guidebook.mn
	tbl Guidebook.mn | ditroff -mn | psdit > Guidebook.ps

# Guidebook.tex is the same as Guidebook.mn but formatted with LaTeX.
# - The invocation command for LaTeX may vary in different installations.
# - To print Guidebook.dvi you need to use a suitable dvi-driver.
Guidebook.dvi:	Guidebook.tex
	latex Guidebook.tex

data:	data.base ../src/makedefs
	( cd ../src ; ./makedefs -d )

rumors:	rumors.tru rumors.fal ../src/makedefs
	( cd ../src ; ./makedefs -r )

spec_levs: castle.des endgame.des tower.des ../src/lev_comp
	../src/lev_comp castle.des
	../src/lev_comp endgame.des
	../src/lev_comp tower.des
	touch spec_levs

spotless:
	-rm -f Guidebook Guidebook.ps Guidebook.dvi Guidebook.aux Guidebook.log data rumors castle endgame tower1 tower2 tower3
