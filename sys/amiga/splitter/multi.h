/* 	SCCS Id: @(#)multi.h 3.1	93/01/08
/*	Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * external definitions for multi-file file handling package.
 * NB - internal structure under development.  End users should NOT
 *      get too creative!
 */

union multiopts {
	struct mo_read {
		char mor_tag;	/* tag character for this open */
	}r;
	struct mo_write {
		char dummy;	/* (no write in this version) */
	}w;
};

struct multifh {
	unsigned long mfh_fh;	/* AmigaDOS file handle of current segment */
	unsigned long mfh_dirfh;/* AmigaDOS file handle of direction file */
	union multiopts mfh_mo;	/* copy from MultiOpen */
	struct mfh_flags {
		int version:8;	/* not used yet */
		int flags:24;	/* not used yet */
	};
};

typedef union multiopts multiopts;
typedef struct multifh multifh;

extern BPTR MultiOpen(char *, ULONG, multiopts *);
extern ULONG MultiRead(BPTR, void *, ULONG);
extern void MultiClose(BPTR);
