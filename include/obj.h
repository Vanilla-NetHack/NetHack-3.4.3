/*	SCCS Id: @(#)obj.h	3.2	95/11/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJ_H
#define OBJ_H

/* #define obj obj_nh	/* uncomment for SCO UNIX, which has a conflicting
			 * typedef for "obj" in <sys/types.h> */

union vptrs {
	    struct obj *v_nexthere;	/* floor location lists */
	    struct obj *v_ocontainer;	/* point back to container */
	    struct monst *v_ocarry;	/* point back to carrying monst */
};

struct obj {
	struct obj *nobj;
	union vptrs v;
#define nexthere	v.v_nexthere
#define ocontainer	v.v_ocontainer
#define ocarry		v.v_ocarry

	struct obj *cobj;       /* contents list for containers */
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
				   royal coffers for a court ( == 2)
				   tells which fruit a fruit is
				   special for uball and amulet %% BAH */
	char	oclass;		/* object class */
	char	invlet;		/* designation in inventory */
	char	oartifact;	/* artifact array index */

	xchar where;		/* where the object thinks it is */
#define OBJ_FREE	0		/* object not attached to anything */
#define OBJ_FLOOR	1		/* object on floor */
#define OBJ_CONTAINED	2		/* object in a container */
#define OBJ_INVENT	3		/* object in the hero's inventory */
#define OBJ_MINVENT	4		/* object in a monster inventory */
#define OBJ_MIGRATING   5		/* object sent off to another level */
#define OBJ_BURIED	6		/* object buried */
#define OBJ_ONBILL	7		/* object on shk bill */
#define NOBJ_STATES	8
	xchar timed;		/* # of fuses (timers) attached to this obj */

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
#define odiluted oeroded	/* diluted potions */
	Bitfield(oerodeproof,1); /* erodeproof weapon/armor */
	Bitfield(olocked,1);	/* object is locked */
#define recharged olocked	/* recharged once */
	Bitfield(obroken,1);	/* lock has been broken */
	Bitfield(otrapped,1);	/* container is trapped */
#define opoisoned otrapped	/* object (weapon) is coated with poison */
#ifndef NO_SIGNAL
	Bitfield(in_use,1);	/* for magic items before useup items */
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
#define ONAME(otmp)	((char *)(otmp)->oextra)

#define carried(o)	((o)->where == OBJ_INVENT)
#define Has_contents(o) (/* (Is_container(o) || (o)->otyp == STATUE) && */ \
			 (o)->cobj != (struct obj *)0)
#define Is_container(o) ((o)->otyp >= LARGE_BOX && (o)->otyp <= BAG_OF_TRICKS)
#define Is_box(otmp)	(otmp->otyp == LARGE_BOX || otmp->otyp == CHEST)
#define Is_mbag(otmp)	(otmp->otyp == BAG_OF_HOLDING || \
			 otmp->otyp == BAG_OF_TRICKS)

#define is_shield(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SHIELD)
#define is_helmet(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_HELM)
#define is_boots(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_BOOTS)
#define is_gloves(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_GLOVES)
#define is_cloak(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_CLOAK)
#define is_shirt(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SHIRT)
#define is_suit(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SUIT)

#define is_sword(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_wepcat == WEP_SWORD)
#define is_blade(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 (objects[otmp->otyp].oc_wepcat == WEP_BLADE || \
			  objects[otmp->otyp].oc_wepcat == WEP_SWORD))
#define is_weptool(o)	((o)->oclass == TOOL_CLASS && \
			 objects[(o)->otyp].oc_weptool)
#define bimanual(otmp)	((otmp->oclass == WEAPON_CLASS || \
			  otmp->oclass == TOOL_CLASS) && \
			 objects[otmp->otyp].oc_bimanual)

#define Is_candle(otmp)	(otmp->otyp == TALLOW_CANDLE || \
			 otmp->otyp == WAX_CANDLE)

/* Flags for get_obj_location(). */
#define CONTAINED_TOO	0x1
#define BURIED_TOO	0x2

/* maximum amount of oil in a potion of oil */
#define MAX_OIL_IN_FLASK 400

/* egg stuff */
#define MAX_EGG_HATCH_TIME 200	/* longest an egg can remain unhatched */
#define stale_egg(egg)	((monstermoves - (egg)->age) > (2*MAX_EGG_HATCH_TIME))

#define ofood(o) ((o)->otyp == CORPSE || (o)->otyp == EGG || (o)->otyp == TIN)
#define polyfodder(obj)	(ofood(obj) && (obj)->corpsenm == PM_CHAMELEON)
#define mlevelgain(obj)	(ofood(obj) && (obj)->corpsenm == PM_WRAITH)
#define mhealup(obj)	(ofood(obj) && (obj)->corpsenm == PM_NURSE)

#endif /* OBJ_H */
