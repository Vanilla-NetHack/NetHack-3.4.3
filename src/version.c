/*	SCCS Id: @(#)version.c	3.0	89/06/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#include	"date.h"

int
doversion(){

#ifdef BETA
	pline("%s NetHack Beta Version %s - last build %s.",
#else
	pline("%s NetHack Version %s - last build %s.",
#endif
#if defined(MSDOS)
# if defined(TOS)
		"ST",
# else
#  if defined(AMIGA)
		"Amiga",
#  else
		"PC",
#  endif
# endif
#endif
#ifdef UNIX
		"Unix",
#endif
		VERSION, datestring);
	return 0;
}
