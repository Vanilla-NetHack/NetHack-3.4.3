/*	SCCS Id: @(#)spell.h	1.3	87/07/14
/* def.spells.h - version 1.0.0		M. Stephenson  07-07-86 */

#ifndef SPELL_H
#define SPELL_H

struct	spell  {

	int	sp_id;			/* spell id */
	int	sp_lev;			/* power level */
	int	sp_flags;		/* spfx flags */
};

#endif
