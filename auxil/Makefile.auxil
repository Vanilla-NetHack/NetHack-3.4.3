#	Hack Makefile.
#	SCCS Id: @(#)Makefile.auxdir	3.0	89/02/16

VARAUX = data rumors

all:	Guidebook $(VARAUX) spec_levs

Guidebook:	Guidebook.mn
	nroff -mn Guidebook.mn > Guidebook

data:	data.base ../include/config.h
	( cd ../src ; ./makedefs -d )

rumors:	rumors.tru rumors.fal
	( cd ../src ; ./makedefs -r )

spec_levs:
	../src/lev_comp castle.des
	../src/lev_comp endgame.des
	../src/lev_comp tower.des
