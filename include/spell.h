/*	SCCS Id: @(#)spell.h	3.2	95/06/01	*/
/* Copyright 1986, M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SPELL_H
#define SPELL_H

struct spell {
	short	sp_id;			/* spell id (== object.otyp) */
	xchar	sp_lev;			/* power level */
	xchar	sp_uses;		/* uses left to spell */
};

/* levels of memory destruction with a scroll of amnesia */
#define ALL_MAP		0x1
#define ALL_SPELLS	0x2

#endif /* SPELL_H */
