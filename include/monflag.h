/*	SCCS Id: @(#)monflag.h	3.1	93/02/14	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONFLAG_H
#define MONFLAG_H

#define MS_SILENT	0	/* makes no sound */
#define MS_BARK		1	/* if full moon, may howl */
#define MS_MEW		2	/* mews or hisses */
#define MS_ROAR		3	/* roars */
#define MS_GROWL	4	/* growls */
#define MS_SQEEK	5	/* squeaks, as a rodent */
#define MS_SQAWK	6	/* squawks, as a bird */
#define MS_HISS		7	/* hisses */
#define MS_BUZZ		8	/* buzzes (killer bee) */
#define MS_GRUNT	9	/* grunts (or speaks own language) */
#define MS_NEIGH	10	/* neighs, as an equine */
#define MS_WAIL		11	/* wails, as a tortured soul */
#define MS_GURGLE	12	/* gurgles, as liquid or through saliva */
#define MS_BURBLE	13	/* burbles (jabberwock) */
#define MS_ANIMAL	13	/* up to here are animal noises */
#define MS_SHRIEK	15	/* wakes up others */
#define MS_BONES	16	/* rattles bones (skeleton) */
#define MS_LAUGH	17	/* grins, smiles, giggles, and laughs */
#define MS_MUMBLE	18	/* says something or other */
#define MS_IMITATE	19	/* imitates others (leocrotta) */
#define MS_ORC		MS_GRUNT	/* intelligent brutes */
#define MS_HUMANOID	20	/* generic traveling companion */
#ifdef KOPS
#define MS_ARREST	21	/* "Stop in the name of the law!" (Kops) */
#endif
#define MS_SOLDIER	22	/* army and watchmen expressions */
#define MS_GUARD	23	/* "Please drop that gold and follow me." */
#define MS_DJINNI	24	/* "Thank you for freeing me!" */
#define MS_NURSE	25	/* "Take off your shirt, please." */
#define MS_SEDUCE	26	/* "Hello, sailor." (Nymphs) */
#define MS_VAMPIRE	27	/* vampiric seduction, Vlad's exclamations */
#define MS_BRIBE	28	/* asks for money, or berates you */
#define MS_CUSS		29	/* berates (demons) or intimidates (Wiz) */
#define MS_RIDER	30	/* astral level special monsters */
#ifdef MULDGN
#define MS_LEADER	31	/* your class leader */
#define MS_NEMESIS	32	/* your nemesis */
#define MS_GUARDIAN	33	/* your leader's guards */
#endif
#define MS_SELL		34	/* demand payment, complain about shoplifters */
#define MS_ORACLE	35	/* do a consultation */
#define MS_PRIEST	36	/* ask for contribution; do cleansing */


#define MR_FIRE         0x01    /* resists fire */
#define MR_COLD         0x02    /* resists cold */
#define MR_SLEEP        0x04    /* resists sleep */
#define MR_DISINT       0x08    /* resists disintegration */
#define MR_ELEC         0x10    /* resists electricity */
#define MR_POISON       0x20    /* resists poison */
#define MR_ACID         0x40    /* resists acid */
#define MR_STONE        0x80    /* resists petrification */
/* other resistances: magic, sickness */
/* other conveyances: teleport, teleport control, telepathy */


#define M1_FLY		0x00000001L	/* can fly or float */
#define M1_SWIM		0x00000002L	/* can traverse water */
#define M1_AMORPHOUS	0x00000004L	/* can flow under doors */
#define M1_WALLWALK	0x00000008L	/* can phase thru rock */
#define M1_CLING	0x00000010L	/* can cling to ceiling */
#define M1_TUNNEL	0x00000020L	/* can tunnel thru rock */
#define M1_NEEDPICK	0x00000040L	/* needs pick to tunnel */
#define M1_CONCEAL	0x00000080L	/* hides under objects */
#define M1_HIDE		0x00000100L	/* mimics, blends in with ceiling */
#define M1_AMPHIBIOUS	0x00000200L	/* can survive underwater */
#define M1_BREATHLESS	0x00000400L	/* doesn't need to breathe */
#define M1_NOEYES	0x00001000L	/* no eyes to gaze into or blind */
#define M1_NOHANDS	0x00002000L	/* no hands to handle things */
#define M1_NOLIMBS	0x00006000L	/* no arms/legs to kick/wear on */
#define M1_NOHEAD	0x00008000L	/* no head to behead */
#define M1_MINDLESS	0x00010000L	/* has no mind--golem, zombie, mold */
#define M1_HUMANOID	0x00020000L	/* has humanoid head/arms/torso */
#define M1_ANIMAL	0x00040000L	/* has animal body */
#define M1_SLITHY	0x00080000L	/* has serpent body */
#define M1_UNSOLID	0x00100000L	/* has no solid or liquid body */
#define M1_THICK_HIDE	0x00200000L	/* has thick hide or scales */
#define M1_OVIPAROUS	0x00400000L	/* can lay eggs */
#define M1_REGEN	0x00800000L	/* regenerates hit points */
#define M1_SEE_INVIS	0x01000000L	/* can see invisible creatures */
#define M1_TPORT	0x02000000L	/* can teleport */
#define M1_TPORT_CNTRL	0x04000000L	/* controls where it teleports to */
#define M1_ACID		0x08000000L	/* acidic to eat */
#define M1_POIS		0x10000000L	/* poisonous to eat */
#define M1_CARNIVORE	0x20000000L	/* eats corpses */
#define M1_HERBIVORE	0x40000000L	/* eats fruits */
#define M1_OMNIVORE	0x60000000L	/* eats both */
#ifdef NHSTDC
#define M1_METALLIVORE	0x80000000UL	/* eats metal */
#else
#define M1_METALLIVORE	0x80000000L	/* eats metal */
#endif

#define M2_NOPOLY	0x00000001L	/* players mayn't poly into one */
#define M2_UNDEAD	0x00000002L	/* is walking dead */
#define M2_WERE		0x00000004L	/* is a lycanthrope */
#define M2_ELF		0x00000008L	/* is an elf */
#define M2_DWARF	0x00000010L	/* is a dwarf */
#define M2_GIANT	0x00000020L	/* is a giant */
#define M2_ORC		0x00000040L	/* is an orc */
#define M2_HUMAN	0x00000080L	/* is a human */
#define M2_DEMON	0x00000100L	/* is a demon */
#define M2_MERC		0x00000200L	/* is a guard or soldier */
#define M2_LORD		0x00000400L	/* is a lord to its kind */
#define M2_PRINCE	0x00000800L	/* is an overlord to its kind */
#define M2_MINION	0x00001000L	/* is a minion of a deity */
#define M2_MALE		0x00010000L	/* always male */
#define M2_FEMALE	0x00020000L	/* always female */
#define M2_NEUTER	0x00040000L	/* neither male nor female */
#define M2_PNAME	0x00080000L	/* monster name is a proper name */
#define M2_HOSTILE	0x00100000L	/* always starts hostile */
#define M2_PEACEFUL	0x00200000L	/* always starts peaceful */
#define M2_DOMESTIC	0x00400000L	/* can be tamed by feeding */
#define M2_WANDER	0x00800000L	/* wanders randomly */
#define M2_STALK	0x01000000L	/* follows you to other levels */
#define M2_NASTY	0x02000000L	/* extra-nasty monster (more xp) */
#define M2_STRONG	0x04000000L	/* strong (or big) monster */
#define M2_ROCKTHROW	0x08000000L	/* throws boulders */
#define M2_GREEDY	0x10000000L	/* likes gold */
#define M2_JEWELS	0x20000000L	/* likes gems */
#define M2_COLLECT	0x40000000L	/* picks up weapons and food */
#ifdef NHSTDC
#define M2_MAGIC	0x80000000UL	/* picks up magic items */
#else
#define M2_MAGIC	0x80000000L	/* picks up magic items */
#endif

#define M3_WANTSAMUL	0x01		/* would like to steal the amulet */
#define M3_WANTSBELL	0x02		/* wants the bell */
#define M3_WANTSBOOK	0x04		/* wants the book */
#define M3_WANTSCAND	0x08		/* wants the candelabrum */
#ifdef MULDGN
#define M3_WANTSARTI	0x10		/* wants the quest artifact */
#define M3_WANTSALL	0x1f		/* wants any major artifact */
#else
#define M3_WANTSALL	0x0f		/* wants any major artifact */
#endif
#define M3_WAITFORU	0x40		/* waits to see you or get attacked */
#define M3_CLOSE	0x80		/* lets you close unless attacked */

#define M3_COVETOUS	0x1f		/* wants something */
#define M3_WAITMASK	0xc0		/* waiting... */

#define MZ_TINY		0		/* < 2' */
#define MZ_SMALL	1		/* 2-4' */
#define MZ_MEDIUM	2		/* 4-7' */
#define MZ_HUMAN	MZ_MEDIUM	/* human-sized */
#define MZ_LARGE	3		/* 7-12' */
#define MZ_HUGE		4		/* 12-25' */
#define MZ_GIGANTIC	7		/* off the scale */

#endif /* MONFLAG_H */
