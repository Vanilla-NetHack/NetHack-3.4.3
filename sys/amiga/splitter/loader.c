/* 	SCCS Id: @(#)loader.c 3.1	93/01/08
/*	Copyright (c) Kenneth Lorber, Bethesda, Maryland 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Amiga split binary runtime system
 */

/*#define LDEBUG	 	/* turn on debugging I/O */
#define SDEBUG		/* static primary array allocation */
/*#define NOCLEAN		/* turn off $ovl_memchain code */
/*#define NOSPLIT		/* debug: load an unsplit binary(run ONCE!)*/
#define MULTI			/* real file reading code */
/*#define PARANOID		/* check for refs off end that might be OK */
#define CACHE			/* deal with cache flushing */

unsigned long xx;
long *yy;

#include "split.h"

#ifdef SPLIT

#include <stdio.h>		/* for spanic - should change */
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos.h>			/* NOT libraries/dos.h! */
#include "amiout.h"

#include "multi.h"

#define F_LOAD 0
#define F_RELOAD 1

#define HT(x)	((x) & ~MEM_OBJ_EXTEND)

void *$ovl_AllocMem(unsigned int);
void spanic(char *);			/* think about this!!!! */
void exit(int);

#ifdef SDEBUG
unsigned long *$ovl_hunktable[500];	/* 223 as of 10/21/92 */
#else
unsigned long *(*$ovl_hunktable)[];
int $ovl_hunktablesize;
#endif

#ifndef NOCLEAN
BPTR $ovlmemchain=0;
#endif
BPTR ovlfile=0;
BPTR fh;
ULONG database;

BPTR s_LoadSeg(char *);
void s_UnLoadSeg(void);
BPTR load_code(int,char *);
void load_data(int,char *);
unsigned long *load_hunk(BPTR,int,ULONG *);

#ifdef MULTI
union multiopts mo;
	/* dump these after testing */
#define Read MultiRead
#define Close MultiClose
#endif

BPTR
s_LoadSeg(dir)
	char *dir;
	{
	static BPTR base;
	static char never=1;
	if(never){
#ifdef LDEBUG
		fprintf(stderr,"s_LoadSeg waiting\n");
		Delay(100);
		fprintf(stderr,"going\n");
#endif
		base=load_code(F_LOAD,dir);
#ifndef NOSPLIT
		load_data(F_LOAD,dir);
		never=0;
#endif
	}else{
		load_data(F_RELOAD,dir);
	}
#ifdef LDEBUG
	fprintf(stderr,"loadseg done! (waiting)\n");
	getchar();
#endif
#ifdef CACHE
	{
	struct Library *eb=OpenLibrary("exec.library",36);
	if(eb){
		CacheClearU();
		CloseLibrary(eb);
	} else {
		/* force a context switch and hope for the best */
		Delay(1);
	}
	}
#endif
	return(base);
}

BPTR
load_code(dummy,dir)
	int dummy;	/* always F_LOAD */
	char *dir;	/* direction file */
{
	ULONG x;
	ULONG *xp;
	ULONG c,hc;
	ULONG r;
#ifdef MULTI
	mo.r.mor_tag='C';
	fh=MultiOpen(dir,MODE_OLDFILE,&mo);
#else
	fh=Open("s_NetHack.c00",MODE_OLDFILE);
#endif
	if(fh==0){
		fprintf(stderr,"open failed %d\n",IoErr());
		spanic("missing code file");
	}
	Read(fh,&x,4);	/* HUNK_HEADER */
	Read(fh,&x,4);	/* 0 */
	Read(fh,&hc,4);	/* table size */
#ifdef LDEBUG
	fprintf(stderr,"hunk count=%d\n",hc);
#endif
#ifndef SDEBUG
	$ovl_hunktable= (long*(*)[])$ovl_AllocMem(hc*4);
	$ovl_hunktablesize=hc*4;
#endif
#ifdef LDEBUG
	fprintf(stderr,"table at %08x\n",$ovl_hunktable);
#endif
	Read(fh,&x,4);	/* F==0 */
	Read(fh,&x,4);	/* L==size-1 */
	for(c=0;c<hc;c++){
		Read(fh,&x,4);
#ifdef SDEBUG
		xx=$ovl_hunktable[c]=$ovl_AllocMem(x*4);
#else
		xx=(*$ovl_hunktable)[c]=$ovl_AllocMem(x*4);
#endif
#ifdef LDEBUG
		fprintf(stderr,"t[%d]=%08x, len=%08x\n",c,xx,((long*)xx)[-2]);
#endif
	}
#ifdef LDEBUG
	fprintf(stderr,"TABLE LOADED\n");Delay(50);
#endif
	for(c=0,xp=(unsigned long*)1;xp;c++){
#ifdef LDEBUG
# ifdef SDEBUG
		yy=$ovl_hunktable[c];
# else
		yy=(*$ovl_hunktable)[c];
# endif
		fprintf(stderr,"loading hunk %d@%08x len=%08x\n",c,yy,yy[-2]);
#endif
#ifdef SDEBUG
		xp=load_hunk(fh,dummy,$ovl_hunktable[c]);
#else
		xp=load_hunk(fh,dummy,(*$ovl_hunktable)[c]);
#endif
	}
	database=c-1;	/* first hunk for use for data on each load */
	Close(fh);
#ifdef LDEBUG
# ifdef SDEBUG
	fprintf(stderr,"retval=%08x\n",$ovl_hunktable[0]);
# else
	fprintf(stderr,"retval=%08x\n",(*$ovl_hunktable)[0]);
# endif
#endif
#ifdef SDEBUG
	r= (unsigned long) $ovl_hunktable[0];		/* BPTR to seglist */
#else
	r= (unsigned long) (*$ovl_hunktable)[0];	/* BPTR to seglist */
#endif
	return (BPTR)(r>>2)-1;
}

void
load_data(fl,dir)
	int fl;
	char *dir;
{
	int c;
	unsigned long *x;
#ifdef MULTI
	mo.r.mor_tag='D';
	fh=MultiOpen(dir,MODE_OLDFILE,&mo);
#else
	fh=Open("s_NetHack.d00",MODE_OLDFILE);
#endif
			/* doing it this way we don't need the hunk count */
	for(c=database,x=(unsigned long*)1;x;c++){
#ifdef SDEBUG
		x=load_hunk(fh,fl,$ovl_hunktable[c]);
#else
		x=load_hunk(fh,fl,(*$ovl_hunktable)[c]);
#endif
	}
#ifdef LDEBUG
	fprintf(stderr,"end of load_data (waiting)\n");
	getchar();
#endif
	Close(fh);
}

unsigned long *
load_hunk(ovlfile,fl,lbase)
	BPTR ovlfile;		/* AmigaDOS file handle */
	int fl;
	ULONG *lbase;
{
	unsigned long data[2];
	unsigned long *where;
	unsigned long reloc_type;

#ifdef LDEBUG
	{
	int pos=Seek(ovlfile,0,0);
	fprintf(stderr,"load_hunk (fpos=%08x) @%08x len=%08x(%08x)\n",pos,
	lbase,lbase[-2],lbase[-2]/4);
	}
#endif
	if(0==Read(ovlfile,data,sizeof(data))){
#ifdef LDEBUG
		fprintf(stderr,"getchar EOF\n");
		getchar();
#endif
		return(0);	/* EOF */
	}
#ifdef LDEBUG
	fprintf(stderr,"read type=%08x len=%08x(longs)\n",data[0],data[1]);
#endif
	if( HT(data[0])!=HUNK_CODE &&
	    HT(data[0])!=HUNK_DATA &&
	    HT(data[0])!=HUNK_BSS){
		fprintf(stderr,"bad data=%08x\n",data[0]);
		spanic("ovlfile cookie botch");
	}
	where=lbase;
#if 0
				/* clear memory if:
				 * 1. not the first time (MEMF_CLEAR'd already)
				 * 2. data or bss (just in case)
				 * This is just a sanity check since these are
				 * the only hunks we should be seeing on reload.
				 */
	if(fl==F_RELOAD && (HT(data[0])==HUNK_DATA || HT(data[0])==HUNK_BSS)
#endif
	{
		ULONG *p=where;		/* clear memory block */
		ULONG c=(where[-2]/4)-1;	/* (len includes ptr) */
		if(!TypeOfMem(p))spanic("clearing bogus memory");
		while(c--)*p++=0;	/* not memset - use longs for speed */
	}

	if(HT(data[0])==HUNK_DATA || HT(data[0])==HUNK_CODE){
		xx=Read(ovlfile,where,data[1]*4);	/* load the block */
		if(xx!=data[1]*4){
			fprintf(stderr,"Read(%08x,%08x)->%08x\n",where,
			  data[1]*4,xx);
			spanic("out of data");
		}
	} else {
#ifdef LDEBUG
		fprintf(stderr,"BSS - no load\n");
#endif
	}
			/* link/relocate as needed */
			/* NB this could be done faster if we keep a buffer of
			 * relocation information (instead of issuing 4 byte
			 * Reads)
			 */
	xx=Read(ovlfile,&reloc_type,sizeof(reloc_type));
	if(xx!=sizeof(reloc_type))spanic("lost reloc_type");
	while(reloc_type!=HUNK_END){
		unsigned long reloc_count;
		unsigned long reloc_hunk;
		unsigned long *base;
		unsigned long reloc_offset;
		if(reloc_type!=HUNK_RELOC32){
			if(reloc_type==HUNK_END)continue;	/* and quit */
			fprintf(stderr,"bad data %08x\n",reloc_type);
			spanic("ovlfile reloc cookie botch");
		}
		xx=Read(ovlfile,&reloc_count,sizeof(reloc_count));
		if(xx!=sizeof(reloc_count))spanic("lost reloc_count");
		while(reloc_count){
			xx=Read(ovlfile,&reloc_hunk,sizeof(reloc_hunk));
			if(xx!=sizeof(reloc_count))spanic("lost reloc_hunk");
#ifdef SDEBUG
			base=$ovl_hunktable[reloc_hunk];
#else
			base=(*$ovl_hunktable)[reloc_hunk];
#endif
#ifdef LDEBUG
			fprintf(stderr,"reloc #%d: hunk #%d@%08x\n",
			  reloc_count,reloc_hunk,base);
#endif
			while(reloc_count--){
				xx=Read(ovlfile,&reloc_offset,sizeof(long));
				if(xx!=sizeof(reloc_count))
					spanic("lost offset");
				if(reloc_offset<0 || reloc_offset>where[-2]){
					fprintf(stderr,"where[-2]==%08x\n",
					  where[-2]);
					spanic("offset out of hunk");
				}
				{
				ULONG *p=(ULONG*)(((ULONG)where)+reloc_offset);
#ifdef PARANOID
/* NB - nasty violation of signed/unsigned here */
				{
				if(*p > base[-2])
				  fprintf(stderr,
				  "WARNING: offset points outside block\n");
				}
#endif
#ifdef LDEBUG
				fprintf(stderr,
				  "reloc_offset=%08x where=%08x p=%08x\n",
				  reloc_offset,where,p);
				fprintf(stderr," current *p=%08x\n",*p);
#endif
				(*p)+=(unsigned long)base;
#ifdef LDEBUG
				fprintf(stderr," new *p=%08x\n",*p);
#endif
				}
			}
			xx=Read(ovlfile,&reloc_count,sizeof(reloc_count));
			if(xx!=sizeof(reloc_count))spanic("lost reloc_count2");
		}
		xx=Read(ovlfile,&reloc_type,sizeof(reloc_type));
		if(xx!=sizeof(reloc_count))spanic("lost reloc_type2");
	}
	return(where);			/* return execute start point */
}

/*
	-2	len (bytes)
	-1	next block
	 0	data
 */
void *
$ovl_AllocMem(len)
	unsigned int len;
	{
	unsigned long *adr;
	ULONG length=(len&0x0fffffff);
				/* Always clear the memory.  On reload of
				 * bss or data we have to do it manually */
	adr=AllocMem(length+8,(6&(len>>29))|MEMF_CLEAR);
	{
		int type=6&(len>>29);
		if(	type!=MEMF_CHIP &&
			type!=MEMF_FAST &&
			type!=MEMF_PUBLIC &&
			type != 0
		){
			printf("%08x %08x* ",len,type);
			spanic("bad memory type");
		}
	}
	if(!adr)spanic("allocation failure");
	adr[0]=length;
#ifndef NOCLEAN
	adr[1]=(unsigned long)$ovlmemchain;	/* list for freeing at end */
	$ovlmemchain=((long)adr>>2)+1;	/* BPTR to next ptr */
# ifdef LDEBUG
	fprintf(stderr,"Alloc: adr[0]=%08x adr[1]=%08x\n",adr[0],adr[1]);
# endif
#endif
	return adr+2;
}

void
s_UnLoadSeg()
{
#ifndef NOCLEAN
	BPTR p,p1;

# ifdef LDEBUG
	fprintf(stderr,"starting Free loop: ovlmemchain=%x\n",$ovlmemchain);
# endif
	for(p=$ovlmemchain;p;p=p1){
		p1=*(BPTR *)BADDR(p);
# ifdef LDEBUG
		fprintf(stderr,"Free(%x,%x)\n",BADDR(p-1),
		  (*(long *)BADDR(p-1))+8);
# endif
		FreeMem(BADDR(p-1),(*(long *)BADDR(p-1))+8);
	}
#endif
#ifndef SDEBUG
	FreeMem($ovl_hunktable,$ovl_hunktablesize);
#endif
	return;
}

/* this needs to be improved and integrated with wb.c */
void
spanic(s)
	char *s;
{
	fprintf(stderr,"s_LoadSeg failed: %s\n",s);
	getchar();
	exit(1);
}
#endif /* SPLIT */
