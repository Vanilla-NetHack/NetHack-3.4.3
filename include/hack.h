/*	SCCS Id: @(#)hack.h	3.0	88/07/19
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef HACK_H
#define HACK_H

#ifndef CONFIG_H
#include "config.h"
#endif

#define	TELL		1
#define NOTELL		0
#define ON 		1
#define OFF 		0
#define	BOLT_LIM    8	/* from this distance ranged attacks will be made */
#ifdef HARD
#define	MAX_CARR_CAP	120	/* so that boulders can be heavier */
#else
#define	MAX_CARR_CAP	500
#endif
#define	FAR	(COLNO+2)	/* position outside screen */
#ifdef NULL
#undef NULL
#endif /* NULL */
#define NULL  ((genericptr_t)0)
#define DUMMY { 0 }

/* this is the way the game ends */
/* if these are rearranged, the arrays in end.c will need to be changed */
#define DIED		0
#define CHOKING 	1
#define POISONING	2
#define STARVING	3
#define DROWNING	4
#define BURNING 	5
#define CRUSHING	6
#define STONING 	7
#define GENOCIDED	8
#define PANICKED	9
#define TRICKED 	10
#define QUIT		11
#define ESCAPED 	12
#ifdef ENDGAME
#define ASCENDED	13
#endif

#ifndef DECL_H
#include "decl.h"
#endif

#ifndef MONSYM_H
#include 	"monsym.h"
#endif
#ifndef MKROOM_H
#include	"mkroom.h"
#endif
#ifndef OBJCLASS_H
#include	"objclass.h"
#endif

extern coord bhitpos;	/* place where thrown weapon falls to the ground */

#ifndef GOLD_H
#include	"gold.h"
#endif
#ifndef TRAP_H
#include	"trap.h"
#endif
#ifndef FLAG_H
#include	"flag.h"
#endif

#ifndef RM_H
#include	"rm.h"
#endif

#ifdef OVERLAY	/* This doesn't belong here, but we have little choice */
#undef NDECL
#define NDECL(f) f()
#endif

#ifndef EXTERN_H
#include	"extern.h"
#endif

#ifdef OVERLAY
#ifndef TRAMPOLI_H
#include	"trampoli.h"
#endif

#undef EXTERN_H
#include	"extern.h"
#endif /* OVERLAY */


#ifdef STRONGHOLD
# define Inhell		(dlevel > stronghold_level && dlevel <= MAXLEVEL)
#else
# define Inhell 	(dlevel >= HELLLEVEL)
#endif

#ifdef SPELLS
#define	NO_SPELL	0
#endif

/*** some utility macros ***/
# ifndef STUPID_CPP	/* otherwise these macros are functions in hack.c */
#define yn() yn_function(ynchars, 'n')
#define ynq() yn_function(ynqchars, 'q')
#define ynaq() yn_function(ynaqchars, 'y')
#define nyaq() yn_function(nyaqchars, 'n')

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(x,y) ((x) < (y) ? (x) : (y))
#define	plur(x)	(((x) == 1) ? "" : "s")

#define	makeknown(x)	objects[x].oc_name_known = 1
# endif /* STUPID_CPP */

#if defined(MSDOS) || defined(MACOS)
#define getuid() 1
#define getlogin() ((char *) NULL)
#endif /* MSDOS */

/* Macro for a few items that are only static if we're not overlaid.... */
#if defined(OVERLAY)
# define STATIC_PTR
#else
# define STATIC_PTR static
#endif

#if defined(OVERLAY) && (defined(OVL0) || defined(OVL1) || defined(OVL2) || defined(OVL3) || defined(OVLB))
# define STATIC_DCL extern
# define STATIC_OVL
# ifdef OVLB
#  define STATIC_VAR
# else
#  define STATIC_VAR extern
# endif
#else
# define STATIC_DCL static
# define STATIC_OVL static
# define STATIC_VAR static
#endif

/* Unless explicit control is being taken of what is linked where, */
/* always compile everything */
#if !defined(OVERLAY) || (!defined(OVL0) && !defined(OVL1) && !defined(OVL2) && !defined(OVL3) && !defined(OVLB))
# define OVL0  /* Highest priority */
# define OVL1
# define OVL2
# define OVL3  /* Lowest specified priority */
# define OVLB  /* The base overlay segment */
#endif

#endif /* HACK_H /**/
