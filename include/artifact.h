/*	SCCS Id: @(#)artifact.h	3.0	88/07/27
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef NAMED_ITEMS
#ifndef ARTIFACT_H
#define ARTIFACT_H

#define SPFX_NONE   0x0000	/* no special effects, just a bonus */
#define SPFX_NOGEN  0x0001	/* item is special, bequeathed by gods */
#define SPFX_RESTR  0x0002	/* item is restricted - can't be named */
#define	SPFX_INTEL  0x0004	/* item is self-willed - intelligent */
#define SPFX_SPEEK  0x0008	/* item can speak */
#define SPFX_SEEK   0x0010	/* item helps you search for things */
#define SPFX_WARN   0x0020	/* item warns you of danger */
#define SPFX_ATTK   0x0040	/* item has a special attack (attk) */
#define SPFX_DEFN   0x0080	/* item has a special defence (defn) */
#define SPFX_DRLI   0x0100	/* drains a level from monsters */
#define SPFX_SEARCH 0x0200	/* helps searching */
#define SPFX_DMONS  0x1000	/* attack bonus on one monster type */
#define	SPFX_DCLAS  0x2000	/* attack bonus on monsters w/ symbol mtype */
#define	SPFX_DFLAG1 0x4000	/* attack bonus on monsters w/ mflags1 flag */
#define	SPFX_DFLAG2 0x8000	/* attack bonus on monsters w/ mflags2 flag */

#define SPFX_DBONUS 0xF000	/* attack bonus mask */

struct artifact {
	unsigned    otyp;
	const char  *name;
	unsigned    spfx;
	unsigned long mtype;	/* monster type, symbol, or flag */
	struct attack attk, defn;
	uchar	    align;	/* alignment of bequeathing gods */
	char	    class;	/* character class associated with */
};

#endif /* ARTIFACT_H /* */
#endif /* NAMED_ITEMS /* */
