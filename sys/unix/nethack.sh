#!/bin/sh
#	SCCS Id: @(#)nethack.sh	3.4	1990/02/26

HACKDIR=/usr/games/lib/nethackdir
export HACKDIR
HACK=$HACKDIR/nethack
MAXNROFPLAYERS=20

# JP
# set LC_ALL, NETHACKOPTIONS etc..
#
if [ "X$LC_ALL" = "X" -o "X$LC_ALL" = "XC" ] ; then
	LC_ALL=ja_JP.eucJP
	export LC_ALL
fi
if [ "X$LANG" = "X" -o "X$LANG" = "XC" ] ; then
	LANG=ja_JP.eucJP
	export LANG
fi
if [ "X$NETHACKOPTIONS" = "X" ] ; then
	NEXTHACKOPTIONS=
	export NETHACKOPTIONS
fi
if [ "X$USERFILESEARCHPATH" = "X" ] ; then
	USERFILESEARCHPATH=$HACKDIR/%L/%N%C%S:$HACKDIR/%N%C%S:$HACKDIR/%N%S
	export USERFILESEARCHPATH
fi

#if [ "X$DISPLAY" ] ; then
#	xset fp+ $HACKDIR
#fi

# Since Nethack.ad is installed in HACKDIR, add it to XUSERFILESEARCHPATH
case "x$XUSERFILESEARCHPATH" in
x)	XUSERFILESEARCHPATH="$HACKDIR/%N.ad"
	;;
*)	XUSERFILESEARCHPATH="$XUSERFILESEARCHPATH:$HACKDIR/%N.ad"
	;;
esac
export XUSERFILESEARCHPATH

# see if we can find the full path name of PAGER, so help files work properly
# assume that if someone sets up a special variable (HACKPAGER) for NetHack,
# it will already be in a form acceptable to NetHack
# ideas from brian@radio.astro.utoronto.ca
if test \( "xxx$PAGER" != xxx \) -a \( "xxx$HACKPAGER" = xxx \)
then

	HACKPAGER=$PAGER

#	use only the first word of the pager variable
#	this prevents problems when looking for file names with trailing
#	options, but also makes the options unavailable for later use from
#	NetHack
	for i in $HACKPAGER
	do
		HACKPAGER=$i
		break
	done

	if test ! -f $HACKPAGER
	then
		IFS=:
		for i in $PATH
		do
			if test -f $i/$HACKPAGER
			then
				HACKPAGER=$i/$HACKPAGER
				export HACKPAGER
				break
			fi
		done
		IFS=' 	'
	fi
	if test ! -f $HACKPAGER
	then
		echo Cannot find $PAGER -- unsetting PAGER.
		unset HACKPAGER
		unset PAGER
	fi
fi


cd $HACKDIR
case $1 in
	-s*)
		exec $HACK "$@"
		;;
	*)
		exec $HACK "$@" $MAXNROFPLAYERS
		;;
esac
