/* 	SCCS Id: @(#)loader.c 3.1	93/01/08
/*	Copyright (c) Kenneth Lorber, Bethesda, Maryland 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Amiga split binary runtime system
 */

/*#define LDEBUG	 	/* turn on debugging I/O */
#define SDEBUG		/* static primary array allocation */
/*#define NOCLEAN		/* turn off ovl$memchain code */
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
#include <setjmp.h>

jmp_buf jbuf;

#include "amiout.h"

#include "multi.h"

#define F_LOAD 0
#define F_RELOAD 1

#define HT(x)	((x) & ~MEM_OBJ_EXTEND)

void *ovl$AllocMem(unsigned int);
void spanic(char *);			/* think about this!!!! */
void exit(int);

#ifdef SDEBUG
unsigned long *ovl$hunktable[500];	/* 229 as of 2/3/93 */
#else
unsigned long *(*ovl$hunktable)[];
int ovl$hunktablesize;
#endif

#ifndef NOCLEAN
BPTR ovl$memchain=0;
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

	if( setjmp( jbuf ) != 0 )
		return( NULL );
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
	ovl$hunktable= (long*(*)[])ovl$AllocMem(hc*4);
	ovl$hunktablesize=hc*4;
#endif
#ifdef LDEBUG
	fprintf(stderr,"table at %08x\n",ovl$hunktable);
#endif
	Read(fh,&x,4);	/* F==0 */
	Read(fh,&x,4);	/* L==size-1 */
	for(c=0;c<hc;c++){
		Read(fh,&x,4);
#ifdef SDEBUG
		xx=ovl$hunktable[c]=ovl$AllocMem(x*4);
#else
		xx=(*ovl$hunktable)[c]=ovl$AllocMem(x*4);
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
		yy=ovl$hunktable[c];
# else
		yy=(*ovl$hunktable)[c];
# endif
		fprintf(stderr,"loading hunk %d@%08x len=%08x\n",c,yy,yy[-2]);
#endif
#ifdef SDEBUG
		xp=load_hunk(fh,dummy,ovl$hunktable[c]);
#else
		xp=load_hunk(fh,dummy,(*ovl$hunktable)[c]);
#endif
	}
	database=c-1;	/* first hunk for use for data on each load */
	Close(fh);
#ifdef LDEBUG
# ifdef SDEBUG
	fprintf(stderr,"retval=%08x\n",ovl$hunktable[0]);
# else
	fprintf(stderr,"retval=%08x\n",(*ovl$hunktable)[0]);
# endif
#endif
#ifdef SDEBUG
	r= (unsigned long) ovl$hunktable[0];		/* BPTR to seglist */
#else
	r= (unsigned long) (*ovl$hunktable)[0];	/* BPTR to seglist */
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
		x=load_hunk(fh,fl,ovl$hunktable[c]);
#else
		x=load_hunk(fh,fl,(*ovl$hunktable)[c]);
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
	static int lbufsize=680;	/* max xref in one hunk 347 2/3/93 */
	static unsigned long *lbuf=0;	/* load buffer */
	unsigned long *lbp;
	unsigned short *lbps;

	if(!lbuf)lbuf=malloc(lbufsize*4);
	if(!lbuf)spanic("Can't allocate lbuf");
#ifdef LDEBUG
# ifndef MULTI
	{
	int pos=Seek(ovlfile,0,0);
	fprintf(stderr,"load_hunk (fpos=%08x) @%08x len=%08x(%08x)\n",pos,
		lbase,lbase[-2],lbase[-2]/4);
	}
# endif
#endif
	if(0==Read(ovlfile,data,sizeof(data))){
#ifdef LDEBUG
		fprintf(stderr,"getchar EOF\n");
		getchar();
#endif
		return(0);	/* EOF */
	}
#ifdef LDEBUG
	fprintf(stderr,"read type=%08x len=%08x\n",data[0],data[1]<<2);
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
#if 0			/* don't ship enabled - Gigamem returns 0 */
		if(!TypeOfMem(p))spanic("clearing bogus memory");
#endif
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
	xx=Read(ovlfile,&reloc_type,sizeof(reloc_type));
	if(xx!=sizeof(reloc_type))spanic("lost reloc_type");
	while(reloc_type!=HUNK_END){
		unsigned long reloc_count;
		unsigned long reloc_count2;
		unsigned long *base;
		unsigned long reloc_offset;
		unsigned long reloc_shift;
		int hnum;
		if(reloc_type==HUNK_END)continue;	/* and quit */
		if(reloc_type!=HUNK_RELOC32 && reloc_type!=HUNK_RELOC32s){
			fprintf(stderr,"bad data %08x\n",reloc_type);
			spanic("ovlfile reloc cookie botch");
		}
		reloc_shift=(reloc_type==HUNK_RELOC32)?2:1;
		xx=Read(ovlfile,&reloc_count,sizeof(reloc_count));
		if(xx!=sizeof(reloc_count))spanic("lost reloc_count");

		reloc_count2=reloc_count;
		while(reloc_count){     /* fix indent */
			if((reloc_count<<reloc_shift) >= (lbufsize*4)){
				free(lbuf);
				lbufsize=10+reloc_count;
				lbuf=malloc(lbufsize*4);
				if(!lbuf)spanic("Can't realloc lbuf");
			}
			xx=Read(ovlfile,lbuf,((1+reloc_count)<<reloc_shift));
			if(xx!=((1+reloc_count)<<reloc_shift))
				spanic("can't fill lbuf");
			lbp= &lbuf[1];		/* 0 is reloc_hunk */
			lbps= ((unsigned short *)lbuf)+1;
			hnum=(reloc_shift==2)? lbp[-1]: lbps[-1];
#ifdef SDEBUG
			base=ovl$hunktable[hnum];
#else
			base=(*ovl$hunktable)[hnum];
#endif
#ifdef LDEBUG
			fprintf(stderr,"reloc #%d: hunk #%d@%08x\n",
			  reloc_count,hnum,base);
#endif
			while(reloc_count--){
				if(reloc_shift==2){
					reloc_offset= *lbp++;
				} else {
					reloc_offset= *lbps++;
				}
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
			if( reloc_shift == 1 && (reloc_count2 & 1) == 0){ /* longword align */
				short x;
				Read(ovlfile,&x,sizeof(x));
			}
			xx=Read(ovlfile,&reloc_count,sizeof(reloc_count));
			if(xx!=sizeof(reloc_count))spanic("lost reloc_count2");
			reloc_count2=reloc_count;
		}
		xx=Read(ovlfile,&reloc_type,sizeof(reloc_type));
		if(xx!=sizeof(reloc_count))spanic("lost reloc_type2");
	}
/* BUG -
 * lbuf never freed
 */
	return(where);			/* return execute start point */
}

/*
	-2	len (bytes)
	-1	next block
	 0	data
 */
void *
ovl$AllocMem(len)
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
	adr[1]=(unsigned long)ovl$memchain;	/* list for freeing at end */
	ovl$memchain=((long)adr>>2)+1;	/* BPTR to next ptr */
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
	fprintf(stderr,"starting Free loop: ovlmemchain=%x\n",ovl$memchain);
# endif
	for(p=ovl$memchain;p;p=p1){
		p1=*(BPTR *)BADDR(p);
# ifdef LDEBUG
		fprintf(stderr,"Free(%x,%x)\n",BADDR(p-1),
		  (*(long *)BADDR(p-1))+8);
# endif
		FreeMem(BADDR(p-1),(*(long *)BADDR(p-1))+8);
	}
#endif
#ifndef SDEBUG
	FreeMem(ovl$hunktable,ovl$hunktablesize);
#endif
	return;
}

/* this needs to be improved and integrated with wb.c */
void
spanic(s)
	char *s;
{
	fprintf(stderr,"s_LoadSeg failed: %s\n",s);
	s_UnLoadSeg();
	longjmp( jbuf, -1 );
}
#endif /* SPLIT */
