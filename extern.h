/*	SCCS Id: @(#)extern.h	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include "config.h"

/*
 *	I have been told, that in Sys V R3.1, this has to be commented out.
 */
#ifndef MSDOS
extern char *sprintf();
#endif

extern long *alloc();

extern xchar xdnstair, ydnstair, xupstair, yupstair; /* stairs up and down. */

extern xchar dlevel;

#ifdef SPELLS
#include "spell.h"
extern	struct spell spl_book[];	/* sized in decl.c */
#endif

extern int occtime;
extern char *occtxt;		/* defined when occupation != NULL */

#ifdef REDO
extern int in_doagain;
#endif

extern char *HI, *HE;		/* set up in termcap.c */
#ifdef MSDOSCOLOR
extern char *HI_MON, *HI_OBJ;	/* set up in termcap.c */
#endif

#include "obj.h"
extern struct obj *invent, *uwep, *uarm, *uarm2, *uarmh, *uarms, *uarmg, 
#ifdef SHIRT
	*uarmu, /* under-wear, so to speak */
#endif
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
