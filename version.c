/*	SCCS Id: @(#)version.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* version.c - Usenet version 1.0 */

#include	"hack.h"
#include	"date.h"

doversion(){

#ifdef BETA
	pline("%s Net%s Beta Version %s - last edit %s.",
#else
	pline("%s Net%s Version %s - last edit %s.",
#endif
#ifdef UNIX
		"Unix"
#endif
#ifdef MSDOS
		"PC"
#endif
#ifdef QUEST
		, "Quest"
#else
		, "Hack"
#endif
		, VERSION, datestring);
	return(0);
}

#define pg_line(x)	if(page_line(x)) goto quit;

doMSCversion()
{
	char	buf[BUFSZ];

	set_pager(0);
	sprintf(buf, "Behold mortal, the origins of %s Net%s...",
#ifdef UNIX
		"Unix"
#endif
#ifdef MSDOS
		"PC"
#endif
#ifdef QUEST
		, "Quest");
#else
		, "Hack");
#endif
	pg_line("");
	pg_line(buf); pg_line(""); pg_line("");

	pg_line("The original HACK was written by Jay Fenlason with help from");
	pg_line("Kenny Woodland, Mike Thome and Jon Payne.");

	pg_line("");
	pg_line("Andries Brouwer did a major re-write and published (at least)");
	pg_line("two versions (1.0.2 and 1.0.3) to the Usenet.");

	pg_line("");
	pg_line("PC HACK 3.51K was an MSDOS(tm) version of HACK 1.03.");
	pg_line("The PC implementation was done in Microsoft(tm) C by Don Kneller");
	pg_line("and modified by Ken Arromdee.");

	pg_line("");
	pg_line("PC and UNIX HACK were merged by Mike Stephenson and Ken Arromdee");
	pg_line("incorporating many modifications and features made by the above,");
	pg_line("as well as the following honoured hackers:");

	pg_line("");
	pg_line("        Scott R. Turner");
	pg_line("        Gil Neiger");
	pg_line("        Eric Backus");
	set_pager(1);
	return(0);
quit:
	set_pager(2);
	return(0);
}
