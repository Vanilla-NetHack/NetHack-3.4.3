/*	SCCS Id: @(#)permonst.h	3.0	88/04/05
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PERMONST_H
#define PERMONST_H

/*	This structure covers all attack forms.
 *	aatyp is the gross attack type (eg. claw, bite, breath, ...)
 *	adtyp is the damage type (eg. physical, fire, cold, spell, ...)
 *	damn is the number of hit dice of damage from the attack.
 *	damd is the number of sides on each die.
 *
 *	Some attacks can do no points of damage.  Additionally, some can
 *	have special effects *and* do damage as well.  If damn and damd
 *	are set, they may have a special meaning.  For example, if set
 *	for a blinding attack, they determine the amount of time blinded.
 */
struct attack {

	uchar           aatyp;
	uchar           adtyp, damn, damd;
};

/*	Max # of attacks for any given monster.
 */

#define	NATTK	5

#include "monattk.h"
#include "monflag.h"

struct permonst {

#if defined(SMALLDATA) && !defined(MAKEDEFS_C)
	char		mname[24], mlet;		/* full name and sym */
#else
	char		*mname, mlet;		/* full name and sym */
#endif
	schar		mlevel,			/* base monster level */
			mmove,			/* move speed */
			ac,			/* (base) armor class */
			mr,			/* (base) magic resistance */
			maligntyp;		/* basic monster alignment */
	unsigned	geno;			/* creation/geno mask value */
	struct	attack	mattk[NATTK];		/* attacks matrix */
	unsigned	cwt,			/* weight of corpse */
			cnutrit;		/* its nutritional value */
	short		pxlth;			/* length of extension */
	uchar		msound; 		/* noise it makes (6 bits) */
	uchar		msize;			/* physical size (3 bits) */
	long		mflags1,		/* boolean bitflags */
			mflags2;		/* more boolean bitflags */
# ifdef TEXTCOLOR
	uchar		mcolor;			/* color to use */
# endif
};

extern struct permonst
#if defined(SMALLDATA) && !defined(MAKEDEFS_C)
			*mons;
#else
			mons[];		/* the master list of monster types */
#endif
extern struct permonst playermon, *uasmon;	/* you in the same terms */

#if defined(SMALLDATA) && defined(MAKEDEFS_C)

typedef struct pmpart {
	char		mlet;			/* full name and sym */
	schar		mlevel,			/* base monster level */
			mmove,			/* move speed */
			ac,			/* (base) armor class */
			mr,			/* (base) magic resistance */
			maligntyp;		/* basic monster alignment */
	unsigned	geno;			/* creation/geno mask value */
	struct	attack	mattk[NATTK];		/* attacks matrix */
	unsigned	cwt,			/* weight of corpse */
			cnutrit;		/* its nutritional value */
	short		pxlth;			/* length of extension */
	uchar		msound;			/* noise it makes */
	long		mflags1,		/* boolean bitflags */
			mflags2;		/* more boolean bitflags */
# ifdef TEXTCOLOR
	uchar		mcolor;			/* color to use */
# endif
} pmpart;

typedef struct pmstr {
	char		mname[24];		/* full name and sym */
	pmpart		pmp;
} pmstr;

#endif

#endif /* PERMONST_H /**/
