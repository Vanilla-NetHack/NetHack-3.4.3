/*	SCCS Id: @(#)epri.h	3.0	88/04/25
/* 	Copyright (c) 	Izchak Miller, 1989. 		*/
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EPRI_H
#define EPRI_H

#if defined(ALTARS) && defined(THEOLOGY)
struct epri {
	schar shroom;		/* index in rooms */
	schar shralign;		/* alignment of priest's shrine */
	coord shrpos;		/* position of shrine */
	int shrlevel;		/* level of shrine */
	boolean ismale;
	char deitynam[PL_NSIZ];
};

#define	EPRI(mon)	((struct epri *)(&(mon->mextra[0])))
#endif

#endif /* EPRI_H /**/
