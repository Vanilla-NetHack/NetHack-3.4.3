/*	SCCS Id: @(#)hack.h	3.2	95/07/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef HACK_H
#define HACK_H

#ifndef CONFIG_H
#include "config.h"
#endif

/*	For debugging beta code.	*/
#ifdef BETA
#define Dpline	pline
#endif

#define TELL		1
#define NOTELL		0
#define ON		1
#define OFF		0
#define BOLT_LIM    8	/* from this distance ranged attacks will be made */
#define MAX_CARR_CAP	1000	/* so that boulders can be heavier */
#define DUMMY { 0 }

/* symbolic names for capacity levels */
#define UNENCUMBERED	0
#define SLT_ENCUMBER	1
#define MOD_ENCUMBER	2
#define HVY_ENCUMBER	3
#define EXT_ENCUMBER	4
#define OVERLOADED	5

/*
 * This is the way the game ends.  If these are rearranged, the arrays
 * in end.c and topten.c will need to be changed.
 */
#define DIED		 0
#define CHOKING		 1
#define POISONING	 2
#define STARVING	 3
#define DROWNING	 4
#define BURNING		 5
#define DISSOLVED	 6
#define CRUSHING	 7
#define STONING		 8
#define GENOCIDED	 9
#define PANICKED	10
#define TRICKED		11
#define QUIT		12
#define ESCAPED		13
#define ASCENDED	14

#ifndef DUNGEON_H	/* includes align.h */
#include "dungeon.h"
#endif

#ifndef MONSYM_H
#include "monsym.h"
#endif
#ifndef MKROOM_H
#include "mkroom.h"
#endif
#ifndef OBJCLASS_H
#include "objclass.h"
#endif
#ifndef WINTYPE_H
#include "wintype.h"
#endif
#ifndef DECL_H
#include "decl.h"
#endif
#ifndef TIMEOUT_H
#include "timeout.h"
#endif

NEARDATA extern coord bhitpos;	/* place where thrown weapon falls to the ground */

/* types of calls to bhit() */
#define ZAPPED_WAND	0
#define THROWN_WEAPON	1
#define KICKED_WEAPON	2
#define FLASHED_LIGHT	3
#define INVIS_BEAM	4

#ifndef TRAP_H
#include "trap.h"
#endif
#ifndef FLAG_H
#include "flag.h"
#endif

#ifndef RM_H
#include "rm.h"
#endif

#ifndef VISION_H
#include "vision.h"
#endif

#ifndef DISPLAY_H
#include "display.h"
#endif

#ifndef ENGRAVE_H
#include "engrave.h"
#endif

#ifndef RECT_H
#include "rect.h"
#endif

#ifdef USE_TRAMPOLI /* This doesn't belong here, but we have little choice */
#undef NDECL
#define NDECL(f) f()
#endif

#ifndef EXTERN_H
#include "extern.h"
#endif

#ifndef WINPROCS_H
#include "winprocs.h"
#endif

#ifdef USE_TRAMPOLI
#include "wintty.h"
#undef WINTTY_H

#ifndef TRAMPOLI_H
#include "trampoli.h"
#endif

#undef EXTERN_H
#include "extern.h"
#endif /* USE_TRAMPOLI */

#define NO_SPELL	0

/* flags to control query_objlist() */
#define BY_NEXTHERE	  0x1	/* follow objlist by nexthere field */
#define AUTOSELECT_SINGLE 0x2	/* if only 1 object, don't ask */
#define USE_INVLET	  0x4	/* use object's invlet */
#define INVORDER_SORT	  0x8	/* sort objects by packorder */
#define SIGNAL_NOMENU	  0x10	/* return -1 rather than 0 if none allowed */

/* Flags to control query_category() */
/* BY_NEXTHERE used by query_category() too, so skip 0x01 */
#define UNPAID_TYPES 0x02
#define GOLD_TYPES   0x04
#define WORN_TYPES   0x08
#define ALL_TYPES    0x10
#define BILLED_TYPES 0x20
#define CHOOSE_ALL   0x40
#define ALL_TYPES_SELECTED -2

/*** some utility macros ***/
#define yn(query) yn_function(query,ynchars, 'n')
#define ynq(query) yn_function(query,ynqchars, 'q')
#define ynaq(query) yn_function(query,ynaqchars, 'y')
#define nyaq(query) yn_function(query,ynaqchars, 'n')
#define nyNaq(query) yn_function(query,ynNaqchars, 'n')
#define ynNaq(query) yn_function(query,ynNaqchars, 'y')

/* Macros for scatter */
#define VIS_EFFECTS  0x1	/* Display visual effects             */
#define MAY_HITMON   0x2	/* Objects may hit monsters           */
#define MAY_HITYOU   0x4	/* Objects may hit you                */
#define MAY_HIT      0x6	/* Objects may hit you and monsters   */
#define MAY_DESTROY  0x8	/* Objects may be destroyed at random */
#define MAY_FRACTURE 0x10	/* Boulders & statues may fracture    */

/* Macros for launching objects */
#define ROLL	1
#define FLING	2

/* Flags to control menus */
#define MENUTYPELEN sizeof("traditional ")
#define MENU_TRADITIONAL 0
#define MENU_COMBINATION 1
#define MENU_PARTIAL	 2
#define MENU_FULL	 3

#define MENU_SELECTED	TRUE
#define MENU_UNSELECTED	FALSE

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif
#define plur(x)	(((x) == 1) ? "" : "s")

#define ARM_BONUS(obj)	(objects[(obj)->otyp].a_ac + (obj)->spe \
			 - min((int)(obj)->oeroded,objects[(obj)->otyp].a_ac))

#define makeknown(x)	discover_object((x),TRUE)
#define distu(xx,yy)	dist2((int)(xx),(int)(yy),(int)u.ux,(int)u.uy)
#define onlineu(xx,yy)	online2((int)(xx),(int)(yy),(int)u.ux,(int)u.uy)

#define rn1(x,y)	(rn2(x)+(y))

/* negative armor class is randomly weakened to prevent invulnerability */
#define AC_VALUE(AC)	((AC) >= 0 ? (AC) : -rnd(-(AC)))

#if defined(MICRO) && !defined(__DJGPP__)
#define getuid() 1
#define getlogin() ((char *)0)
#endif /* MICRO */

#if defined(OVERLAY)&&(defined(OVL0)||defined(OVL1)||defined(OVL2)||defined(OVL3)||defined(OVLB))
# define USE_OVLx
# define STATIC_DCL extern
# define STATIC_OVL
# ifdef OVLB
#  define STATIC_VAR
# else
#  define STATIC_VAR extern
# endif

#else	/* !OVERLAY || (!OVL0 && !OVL1 && !OVL2 && !OVL3 && !OVLB) */
# define STATIC_DCL static
# define STATIC_OVL static
# define STATIC_VAR static

/* If not compiling an overlay, compile everything. */
# define OVL0	/* Highest priority */
# define OVL1
# define OVL2
# define OVL3	/* Lowest specified priority */
# define OVLB	/* The base overlay segment */
#endif	/* OVERLAY && (OVL0 || OVL1 || OVL2 || OVL3 || OVLB) */

/* Macro for a few items that are only static if we're not overlaid.... */
#if defined(USE_TRAMPOLI) || defined(USE_OVLx)
# define STATIC_PTR
#else
# define STATIC_PTR static
#endif

#endif /* HACK_H */
