/*	SCCS Id: @(#)monflag.h	3.0	89/06/23
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright (c) 1989 Mike Threepoint */

#ifndef MONFLAG_H
#define MONFLAG_H

#define MS_SILENT	0	/* makes no sound */
#define MS_SQEEK	1	/* squeaks, as a rodent */
#define MS_SQAWK	2	/* squawks, as a bird */
#define MS_HISS 	3	/* hisses */
#define MS_BUZZ 	4	/* buzzes (killer bee) */
#define MS_GRUNT	5	/* grunts (or speaks own language) */
#define MS_GROWL	6	/* growls */
#define MS_BARK 	7	/* if full moon, may howl */
#define MS_MEW		8	/* mews or hisses */
#define MS_ROAR 	9	/* roars */
#define MS_NEIGH	10	/* neighs, as an equine */
#define MS_WAIL 	11	/* wails, as a tortured soul */
#define MS_GURGLE	12	/* gurgles, as liquid or through saliva */
#define MS_SHRIEK	15	/* wakes up others */
#define MS_IMITATE	18	/* imitates others (leocrotta) */
#define MS_MUMBLE	19	/* says something or other */
#define MS_SEDUCE	20	/* "Hello, sailor." (Nymphs) */
#ifdef KOPS
#define MS_ARREST	21	/* "Stop in the name of the law!" (Kops) */
#endif
#define MS_LAUGH	22	/* grins, smiles, giggles, and laughs */
#define MS_JEER 	23	/* berates you */
#ifdef HARD
#define MS_BRIBE	24	/* asks for money, or berates you */
#endif
#define MS_CUSS 	25	/* really berates you (the Wiz) */
#ifdef ORACLE
#define MS_ORACLE	26	/* do a consultation */
#endif
#ifdef ALTARS
#define MS_PRIEST	27	/* ask for contribution; do cleansing */
#endif
#define MS_GUARD	28	/* "Please drop that gold and follow me." */
#define MS_NURSE	29	/* "Take off your shirt, please." */
#define MS_SELL 	30	/* demand payment, complain about shoplifters */
#define MS_DJINNI	31	/* "Thank you for freeing me!" */
#ifdef ARMY
#define MS_SOLDIER	32	/* army expressions */
#endif
#define MS_VAMPIRE	33	/* vampiric seduction, Vlad's exclamations */
#define MS_HUMANOID	34	/* generic traveling companion */
#define MS_ORC		MS_GRUNT	/* other intelligent brutes */

#define M1_BIG		0x00000001L
#define M1_VSMALL	0x00000002L
#define M1_FLY		0x00000004L
#define M1_SWIM		0x00000008L
#define M1_WALLWALK	0x00000010L
#define M1_ANIMAL	0x00000020L
#define M1_HUMANOID	0x00000040L
#define M1_UNDEAD	0x00000080L
#define M1_WERE		0x00000100L
#define M1_NOEYES	0x00000200L
#define M1_NOHANDS	0x00000400L
#define M1_NOPOLY	0x00000800L
#define M1_EGGS		0x00001000L
#define M1_POIS		0x00002000L
#define M1_POIS_RES	0x00004000L
#define M1_FIRE_RES	0x00008000L
#define M1_COLD_RES	0x00010000L
#define M1_ELEC_RES	0x00020000L
#define M1_SLEE_RES	0x00040000L
#define M1_STON_RES	0x00080000L
#define M1_REGEN	0x00100000L
#define M1_SEE_INVIS	0x00200000L
#define M1_TPORT	0x00400000L
#define M1_TPORT_CONTROL 0x00800000L
#define M1_GREEDY	0x01000000L
#define M1_JEWELS	0x02000000L
#define M1_MAGIC	0x04000000L
#define M1_COLLECT	0x08000000L
#define M1_LORD		0x10000000L
#define M1_PRINCE	0x20000000L
#define M1_STALK	0x40000000L
#define M1_FEM		0x80000000L

#define M2_TUNNEL	0x00000001L	/* can tunnel through rock */
#define M2_NEEDPICK	0x00000002L	/* needs pick to tunnel */
#define M2_CONCEAL	0x00000004L	/* hides under objects */
#define M2_HIDE 	0x00000008L	/* blends in with ceiling, &c. */
#define M2_ELF		0x00000010L	/* is an elf */
#define M2_DWARF	0x00000020L	/* is a dwarf */
#define M2_GIANT	0x00000040L	/* is a giant */
#define M2_ORC		0x00000080L	/* is an orc */
#define M2_HUMAN	0x00000100L	/* is a human */
#define M2_DEMON	0x00000200L	/* is a demon */
#define M2_MERC 	0x00000400L	/* is a guard or soldier */
#define M2_ROCKTHROW	0x00000800L	/* throws boulders */
#define M2_WANDER	0x00001000L	/* wanders randomly */
#define M2_PNAME	0x00002000L	/* monster name is a proper name */
#define M2_HOSTILE	0x00004000L	/* always starts hostile */
#define M2_PEACEFUL	0x00010000L	/* always starts peaceful */
#define M2_STRONG	0x00020000L	/* strong (or big) monster */
#define M2_NASTY	0x00040000L	/* extra-nasty monster (more xp) */
#define M2_NOLIMBS	0x00080000L	/* no rings or kicking */
#define M2_CARNIVORE	0x00100000L	/* eats corpses */
#define M2_HERBIVORE	0x00200000L	/* eats fruits */
#define M2_OMNIVORE	0x00300000L	/* eats both */
#define M2_THICK_HIDE	0x01000000L	/* has thick hide or scales */
#define M2_AMORPHOUS	0x02000000L	/* fluid; can slide under doors */

#endif /* MONFLAG_H */
