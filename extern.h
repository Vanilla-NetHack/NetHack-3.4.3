/*	SCCS Id: @(#)extern.h	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* extern.h - version 1.0 */

#include "config.h"

#ifndef MSDOS
extern char *sprintf();
#endif

extern long *alloc();

extern xchar xdnstair, ydnstair, xupstair, yupstair; /* stairs up and down. */

extern xchar dlevel;

#ifdef SPELLS
#include "spell.h"
extern	struct spell spl_book[];
#endif

extern int occtime;
extern char *occtxt;		/* defined when occupation != NULL */

#ifdef REDO
extern int in_doagain;
#endif

extern char *HI, *HE;		/* set up in termcap.c */

#include "obj.h"
extern struct obj *invent, *uwep, *uarm, *uarm2, *uarmh, *uarms, *uarmg, 
	*uleft, *uright, *fcobj;
extern struct obj *uchain;	/* defined iff PUNISHED */
extern struct obj *uball;	/* defined if PUNISHED */
struct obj *o_at(), *getobj(), *sobj_at();

extern char *traps[];
extern char *monnam(), *Monnam(), *amonnam(), *Amonnam(),
	*doname(), *aobjnam();
extern char readchar();
extern char vowels[];

#include "you.h"

extern struct you u;

extern xchar curx,cury;	/* cursor location on screen */

extern xchar seehx,seelx,seehy,seely; /* where to see*/
extern char *save_cm,*killer;

extern xchar dlevel, maxdlevel; /* dungeon level */

extern long moves;

extern int multi;

extern char lock[];
