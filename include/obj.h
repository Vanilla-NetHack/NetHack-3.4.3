/*	SCCS Id: @(#)obj.h	3.0	89/11/10
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJ_H
#define OBJ_H

struct obj {
	struct obj *nobj;
	struct obj *nexthere;		/* for location lists */
	struct obj *cobj;		/* id of container object is in */
/*	unsigned o_cwt;			/* container weight capacity */
	unsigned o_id;
	xchar ox,oy;
	xchar odx,ody;
	unsigned otyp;
	unsigned owt;
	unsigned quan;		/* use oextra for tmp gold objects */

	schar spe;		/* quality of weapon, armor or ring (+ or -)
				   number of charges for wand ( >= -1 )
				   marks your eggs, spinach tins, key shapes
				   indicates statues have spellbooks inside
				   tells which fruit a fruit is
				   marks diluted potions
				   special for uball and amulet %% BAH */
#define N_LOX	10	/* # of key/lock shapes */
	char	olet;
	char	invlet;
	int	corpsenm;	/* type of corpse is mons[corpsenm] */
#define leashmon  corpsenm	/* gets m_id of attached pet */
	Bitfield(oinvis,1);	/* not yet implemented */
	Bitfield(olocked,1);	/* object is locked */
#define recharged olocked	/* recharged once */
#define orotten olocked 	/* rotten food */
	Bitfield(otrapped,1);	/* container is trapped */
#define opoisoned otrapped	/* object (weapon) is coated with poison */
	Bitfield(odispl,1);
	Bitfield(known,1);	/* exact nature known */
	Bitfield(dknown,1);	/* color or text known */
	Bitfield(bknown,1);	/* blessing or curse known */
	Bitfield(cursed,1);
	Bitfield(blessed,1);
	Bitfield(unpaid,1);	/* on some bill */
	Bitfield(rustfree,1);
#ifndef NO_SIGNAL
# define in_use rustfree	/* for magic items before useup items */
#endif
	Bitfield(no_charge,1);	/* if shk shouldn't charge for this */
	Bitfield(onamelth,6);
	unsigned oeaten;	/* nutrition left in food, if partly eaten */
	long age;		/* creation date */
	long owornmask;

/* note that TIMEOUT in you.h is defined as 07777L; no bits for items that
 * confer properties may overlap that mask, or timeout.c will happily 
 * rearrange the bits behind the back of the property code
 * shirts, balls, and chains are currently safe
 */
#define	W_BALL	02000L
#define	W_CHAIN	04000L
#define	W_RINGL	010000L	/* make W_RINGL = RING_LEFT (see uprop) */
#define	W_RINGR	020000L
#define	W_RING	(W_RINGL | W_RINGR)
#define	W_ARM	040000L
#define	W_ARMC	0100000L
#define	W_ARMH	0200000L
#define	W_ARMS	0400000L
#define	W_ARMG	01000000L
#define	W_ARMF	02000000L
#define	W_AMUL	04000000L
#define	W_TOOL	010000000L	/* wearing another tool (see uprop) */
#define	W_WEP	020000000L
#ifdef SHIRT
#define	W_ARMU	01000L
#define	W_ARMOR	(W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
#else
#define	W_ARMOR	(W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF)
#endif
	long oextra[1];		/* used for name of ordinary objects - length
				   is flexible; amount for tmp gold objects */
};

#define newobj(xl)	(struct obj *) alloc((unsigned)(xl) + sizeof(struct obj))
#define	ONAME(otmp)	((char *) otmp->oextra)
#define	OGOLD(otmp)	(otmp->oextra[0])

# ifndef STUPID_CPP	/* otherwise these macros are functions */

#define Is_container(otmp)	(otmp->otyp >= ICE_BOX && otmp->otyp <= BAG_OF_TRICKS)
#define Is_box(otmp)	(otmp->otyp == LARGE_BOX || otmp->otyp == CHEST)
#define Is_mbag(otmp)	(otmp->otyp == BAG_OF_HOLDING || otmp->otyp == BAG_OF_TRICKS)

#define is_sword(otmp)	(otmp->otyp >= SHORT_SWORD && otmp->otyp <= KATANA)
#define bimanual(otmp)	(otmp->olet == WEAPON_SYM && objects[otmp->otyp].oc_bimanual)

#define is_flammable(otmp)	(objects[otmp->otyp].oc_material == WOOD || objects[otmp->otyp].oc_material == 0)
#define is_rustprone(otmp)	(objects[otmp->otyp].oc_material == METAL)
#define is_corrodeable(otmp)	(objects[otmp->otyp].oc_material == COPPER)
# endif /* STUPID_CPP */

#endif /* OBJ_H /**/
