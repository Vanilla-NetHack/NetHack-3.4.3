/*    SCCS Id: @(#)split.h		3.1   93/01/08
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993	  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * split.h
 * Common definitions for binary file splitting and loading of split files.
 */

#define SVER	1	/* Basic file splitting capability */
/*#define SVER	2	/* */
/*#define SVER	3	/* */

/* Nothing below this line should need to be modified. */

#ifndef SVER
 __SPLIT_H__SVER_MUST_BE_DEFINED
#endif

#if SVER >= 2
	/* enable options */
#endif

/* internal structures, etc */

#include <exec/types.h>
#include <exec/lists.h>
#include <proto/exec.h>
/*#include "ldextern.h"*/

		/* one for each file we need something from */
struct file_ {
	struct MinNode	node;		/* linkage */
	struct List	punits;		/* program units we need */
	int		fd;		/* fd (-1 if not open) */
	char *		name;		/* open(2)'able name */
};
typedef struct file_ file;

		/* one for each program unit we need to load */
struct punit_ {
	struct MinNode	node;		/* linkage */
	int		libsize;	/* 0 if normal, libsize if library */
	struct block_ *	unit_header;
	struct List	hunks;		/* hunks in this program unit */
};
typedef struct punit_ punit;

		/* one for each hunk we need to load */
struct hunk_ {
	struct MinNode	node;		/* linkage */
	struct List	reloc;		/* reloc8,16,32 */
	struct List	dreloc;		/* drelec8,16,32 */
	struct block_ *	rb;		/* ONE relocatable block */
	struct block_ *	name;		/* max ONE name */
	struct List	extsym;		/* external symbol entries */
	struct punit_ * punit;		/* back pointer */
	struct hunk_  * merge;		/* 0 if lone or last section, else next
					 * part in this (merged by name) hunk */
	struct hunk_  * jmptab;		/* alv's live here, if any do. If so,
					 * it's at the end of the chain */
	int	hunkstart:1;	/* lone hunk or start of a chain */
	int	hunkchain:1;	/* allocated to a chain */
	int	hunkgone:1;	/* hunk has been written */
	long	overlay;	/* 0 if root node */
	long 	hunknum;	/* in output file */
	long	hunkoffset;	/* 0 unless not at start of chain */
};
typedef struct hunk_ hunk;
#define UNASSIGNED_HUNK	0x7ffffff0

struct block_ {
	struct MinNode	node;	/* linkage */
	struct swap_ *sw;	/* if !0, where to reload from disk */
	int id;			/* if used */
	long *b;		/* if !0, block of raw data (else swapped) */
				/* (this should be replaced with a union) */
};
typedef struct block_ block;

		/* This is used to keep memory usage down.  We don't read in
		 * the actual data until we are writing the output file. */
struct swap_ {
	file *f;
	long pos;
	long len;			/* in longs */
};
typedef struct swap_ swap;

		/* When we need a list of lists. */
struct listlist_ {
	struct MinNode	node;		/* linkage */
	int 		id;
	struct List	list;
};
typedef struct listlist_ listlist;

typedef char flag;			/* TRUE or FALSE only */

/* tracing system */
#define MAXTRACEVAR	7
extern char trace[MAXTRACEVAR];
#define LIST		if(trace[0])		/* -t0=1 */
#define HASHSTAT	if(trace[1])		/* -t1=1 */
#define HASHTBL		if(trace[1]>1)		/* -t1=2 */
#define NAME		if(trace[2])		/* -t2=1 */
#define OUT		if(trace[3])		/* -t3=1 */
#define PROC		if(trace[4])		/* -t4=1 */
#define LIB		if(trace[5])		/* -t5=1 */
#define VLIB		if(trace[5]>1)		/* -t5=2 */
#define OVER		if(trace[6])		/* -t6=1 */

/* name_ (symbol table) system */
#define HSIZE	128		/* MUST be power of two */
#define HMASK	(HSIZE-1)

struct nentry_ {		/* a name entry */
	struct MinNode	next;	/* next ref or def in bucket */
	struct hunk_ *defh;	/* hunk where defined, else 0 */
	long defo;		/* offset value of definition */
	char *name;		/* name */
	short len;		/* len of name */
	unsigned refflag:1;	/* just for input_check */
	unsigned linkvar:1;	/* linker variable */
	unsigned inroot:1;	/* forced into root node */
};
typedef struct nentry_ nentry;

/* hunk numbers in the overlay file start at this value: */
#define OVRHUNK_BASE 0x40000000
#define OVRHUNK_MASK 0x0fffffff

/* Lists */
#define LIST_P struct List *
#define NODE_P struct Node *

struct Node *Head(struct List *);
struct Node *Tail(struct List *);
struct Node *Next(struct Node *);
struct Node *Prev(struct Node *);

extern flag read_any_bss;
extern flag overlaying;

extern struct List *_fortemp;
#define foreach(n,lp,t)	_fortemp=(struct List *)(lp);if(_fortemp)	\
			for(n= t Head(_fortemp);n;			\
			  n= t Next((struct Node *)(n)))

/* privates for splitter */

/* structs */
struct hheader {
	int hcount;		/* table size */
	int first;		/* first hunk # */
	int last;		/* last hunk # */
	int (*sizes)[];		/* size of each hunk */
};
struct shunk {
	struct hunk_ *h;		/* linker hunk info */
};

/* externs */
extern char *ssubst(char *,const char *);
extern void panic(char *);
extern char *eos(char *);
extern void read_load_file(char *);
extern void write_code_file(void);
extern void write_data_file(void);
extern void write_dir_file(void);
extern int write_split_file(int);
extern void write_lreloc(struct hunk_ *,struct listlist_ *);
extern void wsf_hunk(struct hunk_ *);
extern void renumber(void);
extern int renumber2(int,int);
extern void write_header(void);
extern void owrite(void*,long);
extern void owrite_long(long);
extern void out_start(char *);
extern void out_stop(void);
extern void new_file(void);

void print_text_block(char *,struct block_ *);
void print_bin_block(struct block_ *);
struct file_ *NewFile(char *);
struct punit_ *NewPunit(void);
struct hunk_ *NewHunk(void);
struct listlist_ *NewListList(void);
struct block_ *NewBlock(void);
long *NewData(long);
int rderror(void);	/* should be void, but needs return val for ?: */
struct block_ *ReadSimpleBlock(struct file_ *,int);
int TossSimpleBlock(struct file_ *);
struct hunk_ *ReadHunk(struct file_ *);
void ReadReloc(struct file_ *,long,struct List *);
long block_size(struct block_ *);
