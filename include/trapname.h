/*	SCCS Id: @(#)trapname.h	3.0	88/11/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRAPNAME_H	/* must be included after "hack.h" */
#define TRAPNAME_H

char *traps[] = {
	"",
	" monster trap",
	" statue trap",
	" bear trap",
	"n arrow trap",
	" dart trap",
	" trapdoor",
	" teleportation trap",
	" pit",
	" sleeping gas trap"
	," magic trap"
	," squeaky board"
	," web"
	," spiked pit"
	," level teleporter"
#ifdef SPELLS
	,"n anti-magic field" 
#endif
	," rust trap"
#ifdef POLYSELF
	," polymorph trap"
#endif
	," land mine"
};

#endif	/* TRAPNAME_H /* */
