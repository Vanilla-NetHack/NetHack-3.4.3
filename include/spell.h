/*	SCCS Id: @(#)spell.h	3.1	90/22/02	*/
/* Copyright 1986, M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SPELL_H
#define SPELL_H

struct spell {
	int	sp_id;			/* spell id */
	int	sp_lev;			/* power level */
	int	sp_uses;		/* uses left to spell */
};

#endif /* SPELL_H */
