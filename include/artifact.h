/*	SCCS Id: @(#)artifact.h	3.0	88/07/27
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef NAMED_ITEMS
#ifndef ARTIFACT_H
#define ARTIFACT_H

#define SPFX_NONE   0x00	/* no special effects, just a bonus */
#define SPFX_NOGEN  0x01	/* item is special, bequeathed by gods */
#define SPFX_RESTR  0x02	/* item is restricted - can't be named */
#define	SPFX_INTEL  0x04	/* item is self-willed - intelligent */
#define SPFX_SPEEK  0x08	/* item can speak */
#define SPFX_SEEK   0x10	/* item helps you search for things */
#define SPFX_WARN   0x20	/* item warns you of danger */
#define SPFX_ATTK   0x40	/* item has a special attack (attk) */
#define SPFX_DEFN   0x80	/* item has a special defence (defn) */
#define SPFX_DMONS  0x100	/* attack bonus on one type of monster */
#define	SPFX_DCLAS  0x200	/* attack bonus on one class of monster */
#define SPFX_DRLI   0x400	/* drains a level from monsters */
#define SPFX_SEARCH 0x800	/* helps searching */

struct artifact {
	unsigned    otyp;
	char	    *name;
	unsigned    spfx;
	unsigned    mtype;
	struct attack attk, defn;
	unsigned    align;
};

#endif /* ARTIFACT_H /* */
#endif /* NAMED_ITEMS /* */
