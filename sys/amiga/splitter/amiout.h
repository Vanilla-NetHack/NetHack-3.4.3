/*    SCCS Id: @(#)amiout.h		3.1   91/15/09
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1990	  */
/* NetHack may be freely redistributed.  See license for details. */
/* This version: $Revision: 1.2 $ */
/* Last edit: $Date: 92/02/09 12:38:04 $ */

/*
 * This is essentially a.out.h for the Amiga object and load file format,
 * as extended by Lattice (now SAS, then adopted by CBM, and extended again).
 */
/* object file structure */
	
	/* these blocks are of the form: ID,N,N data longs */
#define HUNK_UNIT	999
#define HUNK_NAME	1000
#define HUNK_DEBUG	1009
	/* these blocks are of the form: ID,N*,N data longs. */
	/* If (N* && 0xc0000000 == 0xc0000000) then an additional long appears*/
#define HUNK_CODE	1001
#define HUNK_DATA	1002
#define HUNK_BSS	1003
# define MEM_OBJ_ANY	 0
# define MEM_OBJ_FAST	(1<<31)
# define MEM_OBJ_CHIP	(1<<30)
# define MEM_OBJ_EXTEND 0xc0000000
	/* these blocks are of the form: ID, (N!=0,long,N data longs)*, 0 */
#define HUNK_RELOC32	1004
#define HUNK_RELOC16	1005
#define HUNK_RELOC8	1006
#define HUNK_RELOC32s	0x3fc		/* ADOS 2.0 */
#define HUNK_DRELOC32	0x3f7		/* Lattice & ADOS 2.0 */
#define HUNK_DRELOC16	0x3f8		/* Lattice & ADOS 2.0 */
#define HUNK_DRELOC8	0x3f9		/* Lattice & ADOS 2.0 */
	/* these blocks are of the form: ID,(symbol data unit)*,0 */
#define HUNK_EXT	1007
#define		EXT_SYMB	0	/* SDU format 0 */
#define		EXT_DEF		1	/* SDU format 1 */
#define		EXT_ABS		2	/* SDU format 1 */
#define		EXT_RES		3	/* SDU format 1 */
#define		EXT_REF32	129	/* SDU format 2 */
#define		EXT_COMMON	130	/* SDU format 3 */
#define		EXT_REF16	131	/* SDU format 2 */
#define		EXT_REF8	132	/* SDU format 2 */
#define		EXT_DREF32	133	/* ADOS 2.0 */
#define		EXT_DREF16	0x86	/* SDU format 2 Lattice - data ref */
#define		EXT_DREF8	135	/* ADOS 2.0 */
#define	HUNK_SYMBOL	1008
	/* these blocks are of the form: ID */
#define HUNK_END	1010

/* load file structure */
	/* these blocks are of the form:
	 * ID,(N!=0,X,X longs)*,0,SZ,first,last,last-first+1 sizes
	 */
#define HUNK_HEADER	1011
	/* these blocks are of the form:
	 * ID,SZ,M+2,M+1 longs of 0,overlay data table [(O+1)*8+M+1 longs]
	 * where M=tree depth (root=0), O=# overlays-1 [zero base]
	 */
#define HUNK_OVERLAY	1013
	/* these blocks are of the form: ID */
#define HUNK_BREAK	1014

/* link library structure */
		/* NB - this is a Lattice extension.  It is perfectly
		 * legal to concatenate .o files and call it a library.
		 * Now adopted by ADOS 2.0
		 */
	/* these blocks are of the form:
	 * ID, size in longs of the code/data/etc following
	 */
#define LIB_HUNK	1018
	/* these blocks are of the form:
	 * ID, size, 16-bit size of string table, string table, reloc info
	 */
#define LIB_INDEX	1019
	/* the ADOS 2.0 book defines them this way -
	 * it's wrong, right?
	 */
/* #define HUNK_LIB 1019 */
/* #define HUNK_INDEX 1020 */
