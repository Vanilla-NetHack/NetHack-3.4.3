/*	SCCS Id: @(#)you.h	3.0	88/04/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOU_H
#define YOU_H

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

#define	TIMEOUT		007777	/* mask */

#define	LEFT_RING	W_RINGL	/* 010000L */
#define	RIGHT_RING	W_RINGR	/* 020000L */
#define	LEFT_SIDE	LEFT_RING
#define	RIGHT_SIDE	RIGHT_RING
#define	BOTH_SIDES	(LEFT_SIDE | RIGHT_SIDE)

#define WORN_ARMOR	W_ARM	/* 040000L */
#define WORN_CLOAK	W_ARMC	/* 0100000L */
#define WORN_HELMET	W_ARMH	/* 0200000L */
#define WORN_SHIELD	W_ARMS	/* 0400000L */
#define WORN_GLOVES	W_ARMG	/* 01000000L */
#define WORN_BOOTS	W_ARMF	/* 02000000L */
#define	WORN_AMUL	W_AMUL	/* 04000000L */
#define	WORN_BLINDF	W_TOOL	/* 010000000L */
#ifdef SHIRT
#define	WORN_SHIRT	W_ARMU	/* 01000L */
#endif
#define	INTRINSIC	040000000L


	long p_flgs;
	int (*p_tofn)();	/* called after timeout */
};

struct you {
	xchar ux, uy;
	schar dx, dy, dz;	/* direction of move (or zap or ... ) */
	schar di;		/* direction of FF */
	xchar ux0, uy0;		/* initial position FF */
	xchar udisx, udisy;	/* last display pos */
	uchar usym;		/* usually '@' */
	int last_str_turn;	/* 0: none, 1: half turn, 2: full turn */
				/* +: turn right, -: turn left */
	boolean umoved;		/* changed map location (post-move) */
	unsigned udispl;	/* @ on display */
	unsigned ulevel;	/* 1 - MAXULEV */
	unsigned utrap;		/* trap timeout */
	unsigned utraptype;	/* defined if utrap nonzero */
#define	TT_BEARTRAP	0
#define	TT_PIT		1
#define	TT_WEB		2
	unsigned uinshop;	/* used only in shk.c - (roomno+1) of shop */
	int	 uhunger;	/* refd only in eat.c and shk.c */
	unsigned uhs;		/* hunger state - see eat.c */

	struct prop uprops[LAST_PROP+1];

	unsigned umconf;
	char *usick_cause;
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
#define TOE 12
#ifdef POLYSELF
	int mh, mhmax, mtimedone, umonnum;	/* for polymorph-self */
	struct attribs	macurr,			/* for monster attribs */
			mamax;			/* for monster attribs */
	int ulycn;				/* lycanthrope type */
#endif
	unsigned ucreamed;
	unsigned uswallow;		/* set if swallowed by a monster */
	unsigned uswldtim;		/* time you have been swallowed */
#ifdef POLYSELF
	Bitfield(uundetected,1);	/* if you're a hiding monster/piercer */
#endif
#if defined(THEOLOGY) && defined(ELBERETH)
	Bitfield(uhand_of_elbereth,1);	/* if you become Hand of Elbereth */
#endif
#ifdef MEDUSA
	Bitfield(ukilled_medusa,1);	/* if you kill the medusa */
#endif
	Bitfield(uhave_amulet,1);	/* you're carrying the Amulet */
#ifdef HARD
	Bitfield(udemigod,1);		/* once you kill the wiz */
	unsigned udg_cnt;		/* how long you have been demigod */
#endif
	struct	attribs	acurr,		/* your current attributes (eg. str) */
			abon,		/* your bonus attributes (eg. str) */
			amax,		/* your max attributes (eg. str) */
			atemp,		/* used for temporary loss/gain */
			atime;		/* used for loss/gain countdown */
#define U_CHAOTIC      -1		/* the value range of ualigntyp */
#define U_NEUTRAL	0
#define U_LAWFUL	1
	int 	ualign;			/* running alignment score */
	schar 	ualigntyp;		/* basic character alignment */
#ifdef THEOLOGY
#define CONVERT 	2
	schar   ualignbase[CONVERT];	/* for ualigntyp conversion record */
#endif
	schar uluck, moreluck;		/* luck and luck bonus */
#define	LUCKADD		3	/* added value when carrying luck stone */
#define Luck	(u.uluck + u.moreluck)
#define	LUCKMAX		10	/* on moonlit nights 11 */
#define	LUCKMIN		(-10)
	schar	udaminc;
	schar	uac;
	int	uhp,uhpmax;
#ifdef SPELLS
	int	uen, uenmax;		/* magical energy - M. Stephenson */
#endif
#ifdef THEOLOGY
	int ugangr;			/* if the gods are angry at you */
	int ublessed, ublesscnt;	/* blessing/duration from #pray */
#endif
	long	ugold, ugold0;
	long	uexp, urexp;
#ifdef ALTARS
	long 	ucleansed;	/* to record moves when player was cleansed */
#endif
	int uinvault;
	struct monst *ustuck;
	int ugrave_arise; /* you die and become something aside from a ghost */
	int nr_killed[NUMMONS]; 	/* used for experience bookkeeping */
};

#endif /* YOU_H /**/
