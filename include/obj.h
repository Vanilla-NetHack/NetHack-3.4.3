/*	SCCS Id: @(#)obj.h	3.1	92/10/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJ_H
#define OBJ_H

/* #define obj obj_nh	/* uncomment for SCO UNIX, which has a conflicting
			 * typedef for "obj" in <sys/types.h> */

struct obj {
	struct obj *nobj;
	struct obj *nexthere;	/* for location lists */
	struct obj *cobj;	/* contents list for containers */
/*	unsigned o_cwt;		/* container weight capacity */
	unsigned o_id;
	xchar ox,oy;
	short otyp;		/* object class number */
	unsigned owt;
	long quan;		/* number of items */

	schar spe;		/* quality of weapon, armor or ring (+ or -)
				   number of charges for wand ( >= -1 )
				   marks your eggs, spinach tins
				   indicates statues have spellbooks inside
				   tells which fruit a fruit is
				   marks diluted potions
				   special for uball and amulet %% BAH */
	char	oclass;		/* object class */
	char	invlet;		/* designation in inventory */
	char	oartifact;	/* artifact array index */

	Bitfield(cursed,1);
	Bitfield(blessed,1);
	Bitfield(unpaid,1);	/* on some bill */
	Bitfield(no_charge,1);	/* if shk shouldn't charge for this */
	Bitfield(known,1);	/* exact nature known */
	Bitfield(dknown,1);	/* color or text known */
	Bitfield(bknown,1);	/* blessing or curse known */
	Bitfield(rknown,1);	/* rustproof or not known */

	Bitfield(oeroded,2);	/* rusted/corroded/burnt/rotted weapon/armor */
#define MAX_ERODE 3
#define orotten oeroded		/* rotten food */
	Bitfield(oerodeproof,1); /* erodeproof weapon/armor */
	Bitfield(olocked,1);	/* object is locked */
#define recharged olocked	/* recharged once */
	Bitfield(obroken,1);	/* lock has been broken */
	Bitfield(otrapped,1);	/* container is trapped */
#define opoisoned otrapped	/* object (weapon) is coated with poison */
	Bitfield(oldcorpse,1);	/* for troll corpses too old to revive */
#ifndef NO_SIGNAL
# define in_use oldcorpse	/* for magic items before useup items */
#endif
	Bitfield(lamplit,1);    /* a light-source -- can be lit */

	Bitfield(oinvis,1);	/* not yet implemented */
	Bitfield(greased,1);	/* covered with grease */
	Bitfield(onamelth,6);

	int	corpsenm;	/* type of corpse is mons[corpsenm] */
#define leashmon  corpsenm	/* gets m_id of attached pet */
#define spestudied corpsenm	/* how many times a spellbook has been studied */
	unsigned oeaten;	/* nutrition left in food, if partly eaten */
	long age;		/* creation date */
	long owornmask;

/* note that TIMEOUT in you.h is defined as 07777L; no bits for items that
 * confer properties may overlap that mask, or timeout.c will happily
 * rearrange the bits behind the back of the property code
 * shirts, balls, and chains are currently safe
 * FROMOUTSIDE and FROMEXPER in you.h are defined as 020000000L and 0400000000L
 * respectively.  Declarations here should not overlap with those bits either.
 */
#define W_BALL	02000L
#define W_CHAIN	04000L
#define W_RINGL	010000L		/* make W_RINGL = RING_LEFT (see uprop) */
#define W_RINGR	020000L
#define W_RING	(W_RINGL | W_RINGR)
#define W_ARM	040000L
#define W_ARMC	0100000L
#define W_ARMH	0200000L
#define W_ARMS	0400000L
#define W_ARMG	01000000L
#define W_ARMF	02000000L
#define W_AMUL	04000000L
#define W_TOOL	010000000L	/* wearing another tool (see uprop) */
#define W_WEP	020000000L
#define W_ART	040000000L	/* _carrying_ an artifact, not really worn */
#define W_ARTI	0100000000L	/* an invoked artifact, not really worn */
#ifdef TOURIST
#define W_ARMU	01000L
#define W_ARMOR	(W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
#else
#define W_ARMOR	(W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF)
#endif
	long oextra[1];		/* used for name of ordinary objects - length
				   is flexible; amount for tmp gold objects */
};

#define newobj(xl)	(struct obj *)alloc((unsigned)(xl) + sizeof(struct obj))
#define dealloc_obj(obj) free((genericptr_t) (obj))
#define ONAME(otmp)	((char *)(otmp)->oextra)

#define Is_container(otmp) (otmp->otyp >= LARGE_BOX && \
			    otmp->otyp <= BAG_OF_TRICKS)
#define Is_box(otmp)	(otmp->otyp == LARGE_BOX || otmp->otyp == CHEST)
#define Is_mbag(otmp)	(otmp->otyp == BAG_OF_HOLDING || \
			 otmp->otyp == BAG_OF_TRICKS)

#define is_sword(otmp)	(otmp->otyp >= SHORT_SWORD && \
			 otmp->otyp <= RUNESWORD)
#define is_blade(otmp)	(otmp->otyp >= DAGGER && \
			 otmp->otyp <= BILL_GUISARME)
#define bimanual(otmp)	((otmp->oclass == WEAPON_CLASS || \
			  otmp->otyp == UNICORN_HORN) && \
			 objects[otmp->otyp].oc_bimanual)

#define Is_candle(otmp)	(otmp->otyp == TALLOW_CANDLE || \
			 otmp->otyp == WAX_CANDLE)

#endif /* OBJ_H */
