/*	SCCS Id: @(#)version.c	3.0	89/06/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#include	"date.h"
#ifndef BETA
#if defined(MSDOS) && !defined(AMIGA)
# include	"patchlev.h"
#else
# include	"patchlevel.h"
#endif
#endif

int
doversion(){

#ifdef BETA
	pline("%s NetHack Beta Version %s - last build %s.",
#else
	pline("%s NetHack Version %s Patchlevel %d - last build %s.",
#endif
#if defined(MSDOS)
# if defined(TOS)
		"ST",
# else
#  if defined(AMIGA)
		"Amiga",
#  else
#   if defined(OS2)
		"OS/2",
#   else
		"PC",
#   endif
#  endif
# endif
#endif
#ifdef MACOS
		"Macintosh",
#endif
#ifdef UNIX
		"Unix",
#endif
#ifdef VMS
		"VMS",
#endif
		VERSION,
#ifndef BETA
		PATCHLEVEL,
#endif
		datestring);
	return 0;
}

#define Next_opt(x) if (next_opt(x)) goto quit;
#define Page_line(x) if (page_line(x)) goto quit;

int
doextversion()
{
	set_pager(0);
	(void)page_line("");
	Page_line("Options compiled into this version of NetHack");
	Page_line("(in no particular order):");
	Page_line("");
#ifdef POLYSELF
	Next_opt("self-polymorphing, ");
#endif
#ifdef THEOLOGY
	Next_opt("theology, ");
#endif
#ifdef SOUNDS
	Next_opt("sounds, ");
#endif
#ifdef KICK
	Next_opt("kicking, ");
#endif
#ifdef THRONES
	Next_opt("thrones, ");
#endif
#ifdef FOUNTAINS
	Next_opt("fountains, ");
#endif
#ifdef SINKS
	Next_opt("sinks, ");
#endif
#ifdef ALTARS
	Next_opt("altars, ");
#endif
#ifdef WALLIFIED_MAZE
	Next_opt("fancy mazes, ");
#endif
#ifdef REINCARNATION
	Next_opt("Rogue level, ");
#endif
#ifdef STRONGHOLD
	Next_opt("special levels, ");
#endif
#ifdef ORACLE
	Next_opt("oracle, ");
#endif
#ifdef MEDUSA
	Next_opt("Medusa, ");
#endif
#ifdef KOPS
	Next_opt("Kops, ");
#endif
#ifdef ARMY
	Next_opt("armies, ");
#endif
#ifdef WORM
	Next_opt("long worms, ");
#endif
#ifdef GOLEMS
	Next_opt("golems, ");
#endif
#ifdef INFERNO
	Next_opt("inferno, ");
#endif
#ifdef SEDUCE
	Next_opt("seduction, ");
#endif
#ifdef TOLKIEN
	Next_opt("Tolkien extras, ");
#endif
#ifdef PROBING
	Next_opt("wand of probing, ");
#endif
#ifdef WALKIES
	Next_opt("leashes, ");
#endif
#ifdef SHIRT
	Next_opt("Hawaiian shirt, ");
#endif
#ifdef MUSIC
	Next_opt("music, ");
#endif
#ifdef TUTTI_FRUTTI
	Next_opt("fruits, ");
#endif
#ifdef SPELLS
	Next_opt("spells, ");
#endif
#ifdef NAMED_ITEMS
	Next_opt("named items, ");
#endif
#ifdef ELBERETH
	Next_opt("Elbereth, ");
#endif
#ifdef EXPLORE_MODE
	Next_opt("explore mode, ");
#endif
#ifdef HARD
	Next_opt("HARD, ");
#endif
#ifdef REDO
	Next_opt("redo-commands, ");
#endif
#ifdef CLIPPING
	Next_opt("screen clipping, ");
#endif
#ifdef TEXTCOLOR
	Next_opt("color, ");
#endif
#ifdef DGK
	Next_opt("Don Kneller's enhancements, ");
#endif
#ifdef OVERLAY
	Next_opt("overlays, ");
#endif
#ifdef SHELL
	Next_opt("shell command, ");
#endif
#ifdef MAIL
	Next_opt("mail, ");
#endif
#ifdef NEWS
	Next_opt("news file, ");
#endif
#ifdef COM_COMPL
	Next_opt("command line completion, ");
#endif
#ifdef AMIGA_WBENCH
	Next_opt("Amiga WorkBench support, ");
#endif
#ifdef WIZARD
	Next_opt("wizard mode, ");
#endif
#ifdef LOGFILE
	Next_opt("logfile, ");
#endif
#ifdef TERMLIB
	Next_opt("termcap file, ");
#endif
#ifdef TERMINFO
	Next_opt("terminfo, ");
#endif
#ifdef ANSI_DEFAULT
	Next_opt("ANSI default terminal, ");
#endif
#ifdef COMPRESS
	Next_opt("compress bones/level/save files, ");
#endif
	Next_opt("basic NetHack features");
	Next_opt("");
	set_pager(1);
	return 0;
quit:
	(void) next_opt("\033");
	set_pager(2);
	return 0;
}

#ifdef MSDOS
int
comp_times(filetime)
long filetime;
{
	if(filetime < compiletime) return (1);
	else return (0);
}
#endif
