/*	SCCS Id: @(#)objclass.h	3.0	89/01/10
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJCLASS_H
#define OBJCLASS_H

/* definition of a class of objects */

struct objclass {
	char *oc_name;		/* actual name */
	char *oc_descr;		/* description when name unknown */
	char *oc_uname;		/* called by user */
	Bitfield(oc_name_known,1);
	Bitfield(oc_merge,1);	/* merge otherwise equal objects */
	Bitfield(oc_bool,1);
#define oc_bimanual	oc_bool	/* for weapons */
#define oc_bulky	oc_bool	/* for armor */
#define oc_charged	oc_bool	/* for rings & tools: allow +n or (n) */
	Bitfield(oc_material,4);
#define GLASS		1
#define WOOD		2
#define COPPER		3 /* Cu */
#define METAL		4 /* Fe */
#define SILVER		5 /* Ag */
#define GOLD		6 /* Au */
#define PLATINUM	7 /* Pt */
#define MITHRIL		8
#define MINERAL		15
	uchar oc_oprop; 	/* property (invis, &c.) conveyed */
	char oc_olet;
	int oc_prob;		/* probability for mkobj() */
	schar oc_delay;		/* delay when using such an object */
	uchar oc_weight;
	int oc_cost;		/* base cost in shops */
	schar oc_oc1, oc_oc2;
	int oc_oi;
#define	nutrition	oc_oi	/* for foods */
#define w_propellor	oc_oi	/* for weapons */
#define WP_BOW		1
#define WP_SLING	2
#define WP_CROSSBOW	3
#define	a_ac		oc_oc1	/* for armors - only used in ARM_BONUS */
#define ARM_BONUS(obj)	((10 - objects[obj->otyp].a_ac) + obj->spe)
#define	a_can		oc_oc2	/* for armors */
#define bits		oc_oc1	/* for wands */
				/* wands */
#define		NODIR		1
#define		IMMEDIATE	2
#define		RAY		4
  /* Check the AD&D rules!  The FIRST is small monster damage. */
#define	wsdam		oc_oc1	/* for weapons, PICK_AXE, rocks, and gems */
#define	wldam		oc_oc2	/* for weapons, PICK_AXE, rocks, and gems */

#define	g_val		oc_cost	/* for gems: value on exit */

#ifdef SPELLS
#define spl_lev		oc_oi	/* for books: spell level */
#endif
};

extern struct objclass objects[];

/* definitions of all object-symbols */

#define	RANDOM_SYM	'\0'	/* used for generating random objects */
#define	ILLOBJ_SYM	'\\'
#define	AMULET_SYM	'"'
#define	FOOD_SYM	'%'
#define	WEAPON_SYM	')'
#define	TOOL_SYM	'('
#define	BALL_SYM	'0'
#define	CHAIN_SYM	'_'
#define	ROCK_SYM	'`'
#define	ARMOR_SYM	'['
#define	POTION_SYM	'!'
#define	SCROLL_SYM	'?'
#define	WAND_SYM	'/'
#define	RING_SYM	'='
#define	GEM_SYM		'*'
#define	GOLD_SYM	'$'
#define	VENOM_SYM	'.'
#ifdef SPELLS
#define	SPBOOK_SYM	'+'	/* actually SPELL-book */
#endif
/* Other places with explicit knowledge of object symbols:
 * pager.c:	if(q == '%') pline("%%	a piece of food");
 */

#ifdef TUTTI_FRUTTI
struct fruit {
	char fname[PL_FSIZ];
	int fid;
	struct fruit *nextf;
};
#define newfruit() (struct fruit *)alloc(sizeof(struct fruit))
#endif
#endif /* OBJCLASS_H */
