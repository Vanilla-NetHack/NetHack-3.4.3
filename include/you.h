/*	SCCS Id: @(#)you.h	3.1	93/04/24	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOU_H
#define YOU_H

#ifndef ALIGN_H
#include "align.h"
#endif
#ifndef ATTRIB_H
#include "attrib.h"
#endif
#ifndef MONST_H
#include "monst.h"
#endif
#ifndef YOUPROP_H
#include "youprop.h"
#endif

struct prop {

#define TIMEOUT		007777	/* mask */

#define LEFT_RING	W_RINGL	/* 010000L */
#define RIGHT_RING	W_RINGR	/* 020000L */
#define LEFT_SIDE	LEFT_RING
#define RIGHT_SIDE	RIGHT_RING
#define BOTH_SIDES	(LEFT_SIDE | RIGHT_SIDE)

#define WORN_ARMOR	W_ARM	/* 040000L */
#define WORN_CLOAK	W_ARMC	/* 0100000L */
#define WORN_HELMET	W_ARMH	/* 0200000L */
#define WORN_SHIELD	W_ARMS	/* 0400000L */
#define WORN_GLOVES	W_ARMG	/* 01000000L */
#define WORN_BOOTS	W_ARMF	/* 02000000L */
#define WORN_AMUL	W_AMUL	/* 04000000L */
#define WORN_BLINDF	W_TOOL	/* 010000000L */
#ifdef TOURIST
#define WORN_SHIRT	W_ARMU	/* 01000L */
#endif

/*
 * FROMEXPER is for a property gained by virtue of your experience level,
 * which will be lost if you lose that level; FROMOUTSIDE is one that is
 * gained in some other way (e.g., a throne, a prayer, or a corpse).
 * INTRINSIC is either FROMEXPER or FROMOUTSIDE
 */
#define FROMOUTSIDE	0200000000L
#define FROMEXPER	0400000000L
#define INTRINSIC	(FROMOUTSIDE|FROMEXPER)

	long p_flgs;
	int NDECL((*p_tofn));	/* called after timeout */
};

struct u_have {
	Bitfield(amulet,1);	/* carrying Amulet	*/
	Bitfield(bell,1);	/* carrying Bell	*/
	Bitfield(book,1);	/* carrying Book	*/
	Bitfield(menorah,1);	/* carrying Candelabrum */
#ifdef MULDGN
	Bitfield(questart,1);	/* carrying the Quest Artifact */
	Bitfield(unused,3);
#else
	Bitfield(unused,4);
#endif
};

struct u_event {
	Bitfield(minor_oracle,1);	/* received at least 1 cheap oracle */
	Bitfield(major_oracle,1);	/*  "  expensive oracle */
#ifdef MULDGN
	Bitfield(qcalled,1);		/* called by Quest leader to do task */
	Bitfield(qexpelled,1);		/* expelled from the Quest dungeon */
	Bitfield(qcompleted,1);		/* successfully completed Quest task */
#endif
	Bitfield(uheard_tune,2);	/* 1=know about, 2=heard passtune */
	Bitfield(uopened_dbridge,1);	/* opened the drawbridge */
	Bitfield(invoked,1);		/* invoked Gate to the Sanctum level */

	Bitfield(gehennom_entered,1);	/* entered Gehennom via Valley */
#ifdef ELBERETH
	Bitfield(uhand_of_elbereth,2);	/* became Hand of Elbereth */
#endif
	Bitfield(udemigod,1);		/* killed the wiz */
	Bitfield(ascended,1);		/* has offered the Amulet */
};


struct you {
	xchar ux, uy;
	schar dx, dy, dz;	/* direction of move (or zap or ... ) */
	schar di;		/* direction of FF */
	xchar ux0, uy0;		/* initial position FF */
	d_level uz, uz0;	/* your level on this and the previous turn */
	d_level utolev;		/* level monster teleported you to, or uz */
	char utotype;		/* bitmask of goto_level() flags for utolev */
	char usym;		/* usually '@' */
	boolean umoved;		/* changed map location (post-move) */
	int last_str_turn;	/* 0: none, 1: half turn, 2: full turn */
				/* +: turn right, -: turn left */
	unsigned ulevel;	/* 1 - MAXULEV */
	unsigned utrap;		/* trap timeout */
	unsigned utraptype;	/* defined if utrap nonzero */
#define TT_BEARTRAP	0
#define TT_PIT		1
#define TT_WEB		2
#define TT_LAVA		3
#define TT_INFLOOR	4
	char	urooms[5];	/* rooms (roomno + 3) occupied now */
	char	urooms0[5];	/* ditto, for previous position */
	char	uentered[5];	/* rooms (roomno + 3) entered this turn */
	char	ushops[5];	/* shop rooms (roomno + 3) occupied now */
	char	ushops0[5];	/* ditto, for previous position */
	char	ushops_entered[5]; /* ditto, shops entered this turn */
	char	ushops_left[5];	/* ditto, shops exited this turn */

	int	 uhunger;	/* refd only in eat.c and shk.c */
	unsigned uhs;		/* hunger state - see eat.c */

	struct prop uprops[LAST_PROP+1];

	unsigned umconf;
	const char *usick_cause;
/* For messages referring to hands, eyes, feet, etc... when polymorphed */
#define ARM 0
#define EYE 1
#define FACE 2
#define FINGER 3
#define FINGERTIP 4
#define FOOT 5
#define HAND 6
#define HANDED 7
#define HEAD 8
#define LEG 9
#define LIGHT_HEADED 10
#define NECK 11
#define SPINE 12
#define TOE 13

	/* These ranges can never be more than MAX_RANGE (vision.h). */
	int nv_range;			/* current night vision range */
	int xray_range;			/* current xray vision range */

	/*
	 * These variables are valid globally only when punished and blind.
	 */
#define BC_BALL  0x01	/* bit mask for ball  in 'bc_felt' below */
#define BC_CHAIN 0x02	/* bit mask for chain in 'bc_felt' below */
	int bglyph;	/* glyph under the ball */
	int cglyph;	/* glyph under the chain */
	int bc_order;	/* ball & chain order [see bc_order() in ball.c] */
	int bc_felt;	/* mask for ball/chain being felt */


	/*
	 * Player type monster (e.g. PM_VALKYRIE).  This is set in u_init
	 * and never changed afterward.
	 */
	int umonster;
#ifdef POLYSELF
	int umonnum;				/* monster number or -1 */
	int mh, mhmax, mtimedone;		/* for polymorph-self */
	struct attribs	macurr,			/* for monster attribs */
			mamax;			/* for monster attribs */
	int ulycn;				/* lycanthrope type */
#endif
	unsigned ucreamed;
	unsigned uswldtim;		/* time you have been swallowed */

	Bitfield (uswallow,1);		/* true if swallowed */
	Bitfield(uinwater,1);		/* if you're currently in water (only
					   underwater possible currently) */
#ifdef POLYSELF
	Bitfield(uundetected,1);	/* if you're a hiding monster/piercer */
	Bitfield(mfemale,1);		/* saved human value of flags.female */
#endif
	Bitfield(uinvulnerable,1);	/* you're invulnerable (praying) */
	Bitfield(usleep,1);		/* you're sleeping */

	unsigned udg_cnt;		/* how long you have been demigod */
	struct u_event	uevent;		/* certain events have happened */
	struct u_have	uhave;		/* you're carrying special objects */
	struct attribs	acurr,		/* your current attributes (eg. str)*/
			aexe,		/* for gain/loss via "exercise" */
			abon,		/* your bonus attributes (eg. str) */
			amax,		/* your max attributes (eg. str) */
			atemp,		/* used for temporary loss/gain */
			atime;		/* used for loss/gain countdown */
	align	ualign;			/* character alignment */
#define CONVERT		2
	aligntyp ualignbase[CONVERT];	/* for ualign conversion record */
	schar uluck, moreluck;		/* luck and luck bonus */
#define LUCKADD		3	/* added value when carrying luck stone */
#define Luck	(u.uluck + u.moreluck)
#define LUCKMAX		10	/* on moonlit nights 11 */
#define LUCKMIN		(-10)
	schar	udaminc;
	schar	uac;
	int	uhp,uhpmax;
	int	uen, uenmax;		/* magical energy - M. Stephenson */
	int ugangr;			/* if the gods are angry at you */
	int ublessed, ublesscnt;	/* blessing/duration from #pray */
	long	ugold, ugold0;
	long	uexp, urexp;
	long	ucleansed;	/* to record moves when player was cleansed */
	int uinvault;
	struct monst *ustuck;
	int ugrave_arise; /* you die and become something aside from a ghost */
	int nr_killed[NUMMONS];		/* used for experience bookkeeping */
};

#endif	/* YOU_H */
