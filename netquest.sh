#!/bin/sh
#	SCCS Id: @(#)netquest.sh	1.3	87/07/14
QUESTDIR=/usr/games/lib/questdir
QUEST=$HACKDIR/netquest
MAXNROFPLAYERS=4

cd $QUESTDIR
case $1 in
	-s*)
		exec $QUEST $@
		;;
	*)
		exec $QUEST $@ $MAXNROFPLAYERS
		;;
esac
