#!/bin/sh
#	SCCS Id: @(#)nethack.sh	1.3	87/07/14
HACKDIR=/usr/games/lib/nethackdir
HACK=$HACKDIR/nethack
MAXNROFPLAYERS=4

cd $HACKDIR
case $1 in
	-s*)
		exec $HACK $@
		;;
	*)
		exec $HACK $@ $MAXNROFPLAYERS
		;;
esac
