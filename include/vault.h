/*	SCCS Id: @(#)vault.h	3.0	88/04/25
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef VAULT_H
#define VAULT_H

#define	FCSIZ	(ROWNO+COLNO)
struct fakecorridor {
	xchar fx,fy,ftyp;
};

struct egd {
	int fcbeg, fcend;	/* fcend: first unused pos */
	xchar gdx, gdy;		/* goal of guard's walk */
	xchar ogx, ogy;		/* guard's last position */
	xchar gdlevel;		/* level guard was created on */
	xchar warncnt;		/* number of warnings to follow */
	int vroom;		/* room number of the vault */
	unsigned gddone:1;
	struct fakecorridor fakecorr[FCSIZ];
};

#define	EGD(mon)	((struct egd *)(&(mon->mextra[0])))

#endif /* VAULT_H /* */
