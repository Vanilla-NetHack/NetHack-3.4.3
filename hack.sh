#!/bin/sh
HACK=/usr/games/HACK
HACKDIR=/usr/games/lib/hack/tmp
MAXNROFPLAYERS=6
MORE=/usr/ucb/more

cd $HACKDIR
case $1 in
	-s*)
		$HACK $@
		;;
	*)
#		/bin/cat news
		$HACK $HACKDIR $MAXNROFPLAYERS $MORE
		;;
esac
