#!/bin/sh
#	SCCS Id: @(#)nethack.sh	3.0	89/08/11

HACKDIR=/usr/games/lib/nethackdir
HACK=$HACKDIR/nethack
MAXNROFPLAYERS=4

# see if we can find the full path name of PAGER, so help files work properly
# ideas from brian@radio.astro.utoronto.ca
if test $PAGER
then
	if test ! -x $PAGER
	then
		IFS=:
		for i in $PATH
		do
			if test -x $i/$PAGER
			then
				PAGER=$i/$PAGER
				export PAGER
				break
			fi
		done
		IFS=' 	'
	fi
	if test ! -x $PAGER
	then
		echo Cannot find $PAGER -- unsetting PAGER.
		unset PAGER
	fi
fi


cd $HACKDIR
case $1 in
	-s*)
		exec $HACK $@
		;;
	*)
		exec $HACK $@ $MAXNROFPLAYERS
		;;
esac
