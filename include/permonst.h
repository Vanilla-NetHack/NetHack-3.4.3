/*	SCCS Id: @(#)permonst.h	2.3	87/12/16
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

struct permonst {
	char *mname,mlet;
	schar mlevel,mmove,ac,mr,damn,damd;
	unsigned pxlth;
};

extern struct permonst mons[];
#define PM_GNOME	&mons[1]
#define PM_HOBGOBLIN	&mons[2]
#ifndef KOPS
#define PM_KOBOLD	&mons[4]
#endif
#define PM_ACID_BLOB	&mons[7]
#ifdef ROCKMOLE
#define PM_ORC		&mons[10]
#define	PM_ZOMBIE	&mons[12]
#else
#define PM_ORC		&mons[11]
#define	PM_ZOMBIE	&mons[13]
#endif
#define	PM_PIERCER	&mons[17]
#define PM_CENTAUR	&mons[22]
#define	PM_KILLER_BEE	&mons[26]
#ifdef SPIDERS
#define PM_SPIDER	&mons[31]
#endif
#define	PM_WRAITH	&mons[33]
#define	PM_MIMIC	&mons[37]
#define PM_TROLL	&mons[38]
#define	PM_VAMPIRE	&mons[43]
#define PM_XORN		&mons[44]
#define	PM_CHAMELEON	&mons[47]
#define PM_DRAGON	&mons[48]
#define PM_ETTIN	&mons[49]
/* The ones below changed to include giants. */
#define	PM_DEMON	&mons[55]

#define	PM_MINOTAUR	&mons[56]	/* last in mons array */
#define	PM_SHK		&mons[57]	/* very last */

#define	PM_GHOST	&pm_ghost
#define	PM_EEL		&pm_eel
#define	PM_WIZARD	&pm_wizard
#ifdef RPH
#define PM_MEDUSA	&pm_medusa
#endif
#ifdef SAC
#define PM_SOLDIER	&pm_soldier
#endif
#define	CMNUM		56		/* number of common monsters */
#ifdef STOOGES
#define PM_LARRY	&pm_larry
#define PM_CURLY	&pm_curly
#define PM_MOE		&pm_moe
#endif
#define PM_DJINNI	&pm_djinni
#define PM_GREMLIN	&pm_gremlin
