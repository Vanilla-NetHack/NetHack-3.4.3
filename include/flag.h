/*	SCCS Id: @(#)flag.h	3.0	89/02/02
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef FLAG_H
#define FLAG_H

struct flag {
	unsigned ident;		/* social security number for each monster */
	boolean  debug;		/* in debugging mode */
#define	wizard	flags.debug
	boolean  explore;	/* in exploration mode */
#define	discover	flags.explore
	unsigned toplin;	/* a top line (message) has been printed */
				/* 0: top line empty; 2: no --More-- reqd. */
	boolean  cbreak;	/* in cbreak mode, rogue format */
	boolean  standout;	/* use standout for --More-- */
	boolean  nonull;	/* avoid sending nulls to the terminal */
	boolean  ignintr;	/* ignore interrupts */
	boolean  time;		/* display elapsed 'time' */
	boolean  nonews;	/* suppress news printing */
	boolean  notombstone;
	unsigned end_top, end_around;	/* describe desired score list */
	boolean  end_own;		/* idem (list all own scores) */
	boolean  no_rest_on_space;	/* spaces are ignored */
	boolean  beginner;
	boolean  female;
	boolean  invlet_constant;	/* let objects keep their
					   inventory symbol */
	boolean  move;
	boolean  mv;
	unsigned run;		/* 0: h (etc), 1: H (etc), 2: fh (etc) */
				/* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
	boolean  nopick;	/* do not pickup objects (as when running) */
	boolean  echo;		/* 1 to echo characters */
	boolean  botl;		/* partially redo status line */
	boolean  botlx;		/* print an entirely new bottom line */
	boolean  nscrinh;	/* inhibit nscr() in pline(); */
	boolean  made_amulet;
	unsigned no_of_wizards;	/* 0, 1 or 2 (wizard and his shadow) */
				/* reset from 2 to 1, but never to 0 */
	unsigned moonphase;
#define	NEW_MOON	0
#define	FULL_MOON	4

	boolean  sortpack;	/* sorted inventory */
	boolean  confirm;	/* confirm before hitting tame monsters */
	boolean  safe_dog;	/* give complete protection to the dog */
	boolean  soundok;	/* ok to tell about sounds heard */
	boolean  verbose;	/* max battle info */
	boolean  silent;	/* whether the bell rings or not */
	boolean  pickup;	/* whether you pickup or move and look */
	boolean  num_pad;	/* use numbers for movement commands */
#ifdef TEXTCOLOR
	boolean  use_color;	/* use color graphics */
#endif
#ifdef DGK
	boolean  IBMBIOS;	/* whether we can use a BIOS call for
				 * redrawing the screen and character input */
#ifdef DECRAINBOW
	boolean  DECRainbow;	/* Used for DEC Rainbow graphics. */
#endif
	boolean  rawio;		/* Whether can use rawio (IOCTL call) */
#endif
};

extern struct flag flags;

#endif /* FLAG_H /**/
