/*	SCCS Id: @(#)artifact.h	3.1	92/11/01	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ARTIFACT_H
#define ARTIFACT_H

#define SPFX_NONE   0x0000000	/* no special effects, just a bonus */
#define SPFX_NOGEN  0x0000001	/* item is special, bequeathed by gods */
#define SPFX_RESTR  0x0000002	/* item is restricted - can't be named */
#define SPFX_INTEL  0x0000004	/* item is self-willed - intelligent */
#define SPFX_SPEEK  0x0000008	/* item can speak */
#define SPFX_SEEK   0x0000010	/* item helps you search for things */
#define SPFX_WARN   0x0000020	/* item warns you of danger */
#define SPFX_ATTK   0x0000040	/* item has a special attack (attk) */
#define SPFX_DEFN   0x0000080	/* item has a special defence (defn) */
#define SPFX_DRLI   0x0000100	/* drains a level from monsters */
#define SPFX_SEARCH 0x0000200	/* helps searching */
#define SPFX_BEHEAD 0x0000400	/* beheads monsters */
#define SPFX_HALRES 0x0000800	/* blocks hallucinations */
#define SPFX_ESP    0x0001000	/* ESP (like amulet of ESP) */
#define SPFX_STLTH  0x0002000	/* Stealth */
#define SPFX_REGEN  0x0004000	/* Regeneration */
#define SPFX_EREGEN 0x0008000	/* Energy Regeneration */
#define SPFX_HSPDAM 0x0010000	/* 1/2 spell damage (on player) in combat */
#define SPFX_HPHDAM 0x0020000	/* 1/2 physical damage (on player) in combat */
#define SPFX_TCTRL  0x0040000	/* Teleportation Control */
#define SPFX_LUCK   0x0080000	/* Increase Luck (like Luckstone) */
#define SPFX_DMONS  0x0100000	/* attack bonus on one monster type */
#define SPFX_DCLAS  0x0200000	/* attack bonus on monsters w/ symbol mtype */
#define SPFX_DFLAG1 0x0400000	/* attack bonus on monsters w/ mflags1 flag */
#define SPFX_DFLAG2 0x0800000	/* attack bonus on monsters w/ mflags2 flag */
#define SPFX_DALIGN 0x1000000	/* attack bonus on non-aligned monsters  */

#define SPFX_DBONUS 0x1F00000	/* attack bonus mask */

struct artifact {
	unsigned    otyp;
	const char  *name;
	unsigned long spfx;	/* special effect from wielding/wearing */
	unsigned long cspfx;	/* special effect just from carrying obj */
	unsigned long mtype;	/* monster type, symbol, or flag */
	struct attack attk, defn, cary;
	uchar	    inv_prop;	/* property obtained by invoking artifact */
	aligntyp    alignment;	/* alignment of bequeathing gods */
	char	    class;	/* character class associated with */
};

/* invoked properties with special powers */
#define TAMING		(LAST_PROP+1)
#define HEALING		(LAST_PROP+2)
#define ENERGY_BOOST	(LAST_PROP+3)
#define UNTRAP		(LAST_PROP+4)
#define CHARGE_OBJ	(LAST_PROP+5)
#define LEV_TELE	(LAST_PROP+6)
#define CREATE_PORTAL	(LAST_PROP+7)

#endif /* ARTIFACT_H */
