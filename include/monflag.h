/*	SCCS Id: @(#)monflag.h	3.0	89/11/21
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
#define MS_BURBLE	13	/* burbles (jabberwock) */
#define MS_SHRIEK	15	/* wakes up others */
#define MS_LAUGH	17	/* grins, smiles, giggles, and laughs */
#define MS_MUMBLE	18	/* says something or other */
#define MS_IMITATE	19	/* imitates others (leocrotta) */
#define MS_SEDUCE	20	/* "Hello, sailor." (Nymphs) */
#define MS_VAMPIRE	21	/* vampiric seduction, Vlad's exclamations */
#define MS_ORC		MS_GRUNT	/* intelligent brutes */
#ifdef INFERNO
#define MS_BRIBE	25	/* asks for money, or berates you */
#endif
#define MS_CUSS 	26	/* berates (demons) or intimidates (Wiz) */
#define MS_NURSE	27	/* "Take off your shirt, please." */
#define MS_DJINNI	28	/* "Thank you for freeing me!" */
#define MS_HUMANOID	29	/* generic traveling companion */
#define MS_GUARD	30	/* "Please drop that gold and follow me." */
#define MS_SELL 	31	/* demand payment, complain about shoplifters */
#ifdef ORACLE
#define MS_ORACLE	32	/* do a consultation */
#endif
#ifdef ALTARS
#define MS_PRIEST	33	/* ask for contribution; do cleansing */
#endif
#ifdef KOPS
#define MS_ARREST	34	/* "Stop in the name of the law!" (Kops) */
#endif
#ifdef ARMY
#define MS_SOLDIER	35	/* army expressions */
#endif

#define M1_FLY		0x00000001L	/* can fly or float */
#define M1_SWIM 	0x00000002L	/* can traverse water */
#define M1_AMORPHOUS	0x00000004L	/* can flow under doors */
#define M1_WALLWALK	0x00000008L	/* can phase thru rock */
#define M1_TUNNEL	0x00000010L	/* can tunnel thru rock */
#define M1_NEEDPICK	0x00000020L	/* needs pick to tunnel */
#define M1_CONCEAL	0x00000040L	/* hides under objects */
#define M1_HIDE 	0x00000080L	/* mimics, blends in with ceiling */
#define M1_NOEYES	0x00000100L	/* no eyes to gaze into or blind */
#define M1_NOHANDS	0x00000200L	/* no hands to handle things */
#define M1_NOLIMBS	0x00000600L	/* no arms/legs to kick/wear on */
#define M1_NOPOLY	0x00000800L	/* players mayn't poly into one */
#define M1_HUMANOID	0x00001000L	/* has humanoid body */
#define M1_ANIMAL	0x00002000L	/* has animal body */
#define M1_SLITHY	0x00004000L	/* has serpent body */
#define M1_THICK_HIDE	0x00008000L	/* has thick hide or scales */
#define M1_FIRE_RES	0x00010000L	/* resists fire */
#define M1_SLEE_RES	0x00020000L	/* resists sleep */
#define M1_COLD_RES	0x00040000L	/* resists cold */
#define M1_ELEC_RES	0x00080000L	/* resists electricity */
#define M1_STON_RES	0x00100000L	/* resists stoning */
#define M1_ACID 	0x00200000L	/* acidic to eat */
#define M1_POIS_RES	0x00400000L	/* resists poison */
#define M1_POIS 	0x00800000L	/* poisonous to eat */
#define M1_REGEN	0x01000000L	/* regenerates hit points */
#define M1_SEE_INVIS	0x02000000L	/* can see invisible creatures */
#define M1_TPORT	0x04000000L	/* can teleport */
#define M1_TPORT_CONTROL 0x08000000L	/* controls where it teleports to */
#define M1_GREEDY	0x10000000L	/* likes gold */
#define M1_JEWELS	0x20000000L	/* likes gems */
#define M1_COLLECT	0x40000000L	/* picks up weapons and food */
#define M1_MAGIC	0x80000000L	/* picks up magic items */

#define M2_UNDEAD	0x00000001L	/* walking dead */
#define M2_WERE 	0x00000002L	/* lycanthrope */
#define M2_ELF		0x00000010L	/* is an elf */
#define M2_DWARF	0x00000020L	/* is a dwarf */
#define M2_GIANT	0x00000040L	/* is a giant */
#define M2_ORC		0x00000080L	/* is an orc */
#define M2_HUMAN	0x00000100L	/* is a human */
#define M2_DEMON	0x00000200L	/* is a demon */
#define M2_MERC 	0x00000400L	/* is a guard or soldier */
#define M2_FEM		0x00000800L	/* characteristically female */
#define M2_WANDER	0x00001000L	/* wanders randomly */
#define M2_STALK	0x00002000L	/* follows you to other levels */
#define M2_DOMESTIC	0x00004000L	/* can be tamed by feeding */
#define M2_HOSTILE	0x00010000L	/* always starts hostile */
#define M2_PEACEFUL	0x00020000L	/* always starts peaceful */
#define M2_NASTY	0x00040000L	/* extra-nasty monster (more xp) */
#define M2_STRONG	0x00080000L	/* strong (or big) monster */
#define M2_CARNIVORE	0x00100000L	/* eats corpses */
#define M2_HERBIVORE	0x00200000L	/* eats fruits */
#define M2_OMNIVORE	0x00300000L	/* eats both */
#define M2_METALLIVORE	0x00400000L	/* eats metal */
#define M2_EGGS 	0x01000000L	/* lays eggs */
#define M2_ROCKTHROW	0x04000000L	/* throws boulders */
#define M2_PNAME	0x20000000L	/* monster name is a proper name */
#define M2_LORD 	0x40000000L	/* a lord to its kind */
#define M2_PRINCE	0x80000000L	/* an overlord to its kind */

#define MZ_TINY		0		/* < 2' */
#define MZ_SMALL 	1		/* 2-4' */
#define MZ_MEDIUM	2		/* 4-7' */
#define MZ_HUMAN	MZ_MEDIUM	/* human-sized */
#define MZ_LARGE 	3		/* 7-12' */
#define MZ_HUGE		4		/* 12-25' */
#define MZ_GIGANTIC	7		/* off the scale */

#endif /* MONFLAG_H */
