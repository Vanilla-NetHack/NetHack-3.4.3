/*	SCCS Id: @(#)spell.h	2.1	87/10/18
/* M. Stephenson  07-07-86 */

#ifndef SPELL_H
#define SPELL_H

struct	spell  {

	int	sp_id;			/* spell id */
	int	sp_lev;			/* power level */
	int	sp_flags;		/* spfx flags */
#ifdef HARD
	int 	sp_uses;		/* uses left to spell */
#endif
};


#endif
