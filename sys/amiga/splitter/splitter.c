/*    SCCS Id: @(#)splitter.c		3.1   93/01/08
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

#define SOUT	/* split output files */
#define SPLITSIZE (800 * 1024)		/* somewhat < one floppy */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <proto/exec.h>
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>

#include "split.h"
#include "amiout.h"
#include "arg.h"

int main(int,char **);

char *code_proto="%n.c%C";
char *data_proto="%n.d%D";
char *dir_proto="%n.dir";

char trace[MAXTRACEVAR];		/* debugging info */
char *basename;
int datacount;	/* for ssubst - should be replaced */
int codecount;
int data_file_count=0;	/* actual maxima */
int code_file_count=0;
struct hheader hheader;
struct shunk (*hlist)[];
char buf[80];
int wsf_count;

main(argc,argv)
    int argc;
    char **argv;
{
    int cur_arg;

    arg_init("C:D:d:t:T",argc,argv);
    while((cur_arg=arg_next())!=ARG_DONE){
	switch(cur_arg){
	case 'C':	/* code prototype */
		code_proto=strdup(argarg);break;
	case 'D':	/* data prototype */
		data_proto=strdup(argarg);break;
	case 'd':	/* directions prototype */
		dir_proto=strdup(argarg);break;
	case 't':	/* trace (debug) */
		{
		int dtype=0,dlevel=0;	/* rude defaults */
		sscanf(argarg,"%d=%d",&dtype,&dlevel);
		if(dtype<0 || dtype>=MAXTRACEVAR){
		    fprintf(stderr,"-t: bad trace num ignored\n");
		}else{
		    trace[dtype]=dlevel?dlevel:1;
		}
		break;
		}
	case 'T':	/* trace everything */
		{
		int dtype;
		for(dtype=0;dtype<MAXTRACEVAR;dtype++)trace[dtype]=255;
		}
		break;
	default:
		fprintf(stderr,"Unrecognized option.\n");
		/* FALLTHROUGH */
	case ARG_ERROR:
		panic("Error processing arguments.");
	case ARG_FREE:
		basename=strdup(argarg);
		read_load_file(basename);break;
	}
    }
    renumber();
    out_start(code_proto);
    write_header();
    write_code_file();
    out_stop();
    out_start(data_proto);
    write_data_file();
    out_stop();
    write_dir_file();
    exit(0);
}

char *
ssubst(buf,pat)
    char *buf;
    const char *pat;
{
    char *buf1=buf;

    while(*pat){
	if(*pat!='%'){
	    *buf++=*pat++;
	} else {
	    pat++;
	    switch(*pat++){
	    case '%': *buf++='%';break;
	    case 'n': strcpy(buf,basename);buf=eos(buf);break;
	    case 'D': sprintf(buf,"%02d",datacount);buf=eos(buf);break;
	    case 'C': sprintf(buf,"%02d",codecount);buf=eos(buf);break;
	    default:  panic("pattern substitution error");
	    }
	}
    }
    *buf='\0';
    return buf1;
}

void
panic(s)
    char *s;
{
    fprintf(stderr,"\npanic: %s\n",s);
    exit(1);
}

char *
eos(s)
    char *s;
{
    while(*s)s++;
    return s;
}

/* input routines */

	/* macro for reading the next long.  If e==EOF_OK, caller MUST check
	 * for EOF condition via hreadval or assure it can't occur */
static int hreadval=0;		/* macro internal temporary */
#define EOF_OK	1
#define EOF_BAD	0
#define READLONG(e)	\
	((4!=(hreadval=read(f->fd,&(READLONGx),4)))		\
	?((0==hreadval && (e)					\
		?0						\
		:rderror()))					\
	:READLONGx)
static long READLONGx;
#define READSHORT(e)	\
	((2!=(hreadval=read(f->fd,&(READSHORTx),2)))		\
	?((0==hreadval && (e)					\
		?0						\
		:rderror()))					\
	:READSHORTx)
static short READSHORTx;

#define LONGLEN(x)	(strlen(x)+3 >>2)	/* # longs for a string */

void
read_load_file(name)
    char *name;
{
    int t;
    int hc;
    file *f=NewFile(name);

		/* read HUNK_HEADER */
    t=READLONG(EOF_BAD);if(t!=HUNK_HEADER)panic("no HUNK_HEADER");
    t=READLONG(EOF_BAD);if(t)while(t--)READLONG(EOF_BAD); /* eat any name */
    hheader.hcount=READLONG(EOF_BAD);
    hheader.first=READLONG(EOF_BAD);
    hheader.last=READLONG(EOF_BAD);
    if(hheader.hcount !=(hheader.last-hheader.first+1))panic("can't count");
    hheader.sizes=calloc(hheader.hcount,sizeof(int*));
    for(t=0;t<hheader.hcount;t++)
	(*hheader.sizes)[t]=READLONG(EOF_BAD);

    hlist=calloc(hheader.hcount,sizeof(struct shunk));
    for(hc=0;hc<hheader.hcount;hc++){
	struct shunk *th = &(*hlist)[hc];
			/* read each hunk */
	th->h=ReadHunk(f);
    }
    close(f->fd);
}

/* write routines */
#define S_CODE	0
#define S_DATA	1

void
write_header(){
    int x;
    int target=0;

    owrite_long(HUNK_HEADER);
    owrite_long(0);
    owrite_long(hheader.hcount);
    owrite_long(hheader.first);
    owrite_long(hheader.last);

    for(x=0;x<hheader.hcount;x++){
	hunk *hp = (*hlist)[x].h;
	if(hp->hunknum==target){
	    owrite_long((*hheader.sizes)[x]);
	    target++;
	}
    }
    for(x=0;x<hheader.hcount;x++){
	hunk *hp = (*hlist)[x].h;
	if(hp->hunknum==target){
	    owrite_long((*hheader.sizes)[x]);
	    target++;
	}
    }
    if(target!=hheader.hcount)panic("lost hunks?");
}

void
write_code_file(){
    code_file_count=write_split_file(S_CODE)-1;
}

void
write_data_file(){
    data_file_count=write_split_file(S_DATA)-1;
}

void
write_dir_file(){
    int x;
    FILE *fp=fopen(ssubst(buf,dir_proto),"w");

    fprintf(fp,"# split binary direction file\n");
    fprintf(fp,"# Each line consists of:\n");
    fprintf(fp,"#   A single C or D for the type of the file (Code or Data)\n");
    fprintf(fp,"#   The full path of the file.\n");

    for(x=0;x<=code_file_count;x++){
	codecount=x;
	fprintf(fp,"C%s\n",ssubst(buf,code_proto));
    }
    for(x=0;x<=data_file_count;x++){
	datacount=x;
	fprintf(fp,"D%s\n",ssubst(buf,data_proto));
    }
    fclose(fp);
}

/* BUGFIX: 9/23/92: see HT() above */
#define HT(x)	((x) & ~MEM_OBJ_EXTEND)

int
write_split_file(fl)
    int fl;
{
    int hc;
    for(hc=0;hc<hheader.hcount;hc++){
	hunk *hp = (*hlist)[hc].h;
	if(fl==S_CODE && HT(hp->rb->id)==HUNK_CODE){
	    wsf_hunk(hp);
	} else if(fl==S_DATA && HT(hp->rb->id)==HUNK_DATA){
	    wsf_hunk(hp);
	} else if(fl==S_DATA && HT(hp->rb->id)==HUNK_BSS){
	    wsf_hunk(hp);
	}
    }
    return wsf_count;
}

/* BUGFIX: 9/23/92: see HT() below */
void
wsf_hunk(hp)
    hunk *hp;
{
    listlist *el;

    switch(HT(hp->rb->id)){
    case HUNK_CODE:
    case HUNK_DATA:
	owrite(hp->rb->b,(2+hp->rb->b[1])*sizeof(long));
	break;
    case HUNK_BSS:
	owrite(hp->rb->b,2*sizeof(long));
	break;
    default:panic("wsf_hunk: bad type");
    }
    foreach(el,&(hp->reloc),(listlist*)){
	write_lreloc(hp,el);
    }
    owrite_long(HUNK_END);
}

void
write_lreloc(hp,ll)
    hunk *hp;listlist *ll;
    {
    block *bp;

#ifdef EMIT_32s
    int x;
    ULONG *p;

		/* can we write the entire block with a HUNK_RELOC32s? */
    foreach(bp,&(ll->list),(block*)){
	if((((*hlist)[bp->b[1]]).h->hunknum)>0xffff)goto no_32s; /* no */
	for(p= &(bp->b[2]), x=bp->b[0];x;x--,p++){
	    	if(*p>0xffff)goto no_32s;	/* no, offset too big */
	}
    }
		/* yes */
    owrite_long(HUNK_RELOC32s);
    foreach(bp,&(ll->list),(block*)){
	owrite_long(bp->b[0]);
	owrite_short(((*hlist)[bp->b[1]]).h->hunknum);
	for(p= &(bp->b[2]), x=bp->b[0];x;x--,p++){
	    owrite_short(*p);
	}
			/* force long alignment.  Not documented, but makes
			 * reading dumps easier */
	if((bp->b[0] & 1) == 0){	/* note hunknum also short */
	    owrite_short(0);
	}
    }
    owrite_long(0);
    return;
no_32s:
#endif
    owrite_long(HUNK_RELOC32);
    foreach(bp,&(ll->list),(block*)){
	owrite_long(bp->b[0]);
	owrite_long(((*hlist)[bp->b[1]]).h->hunknum);
	owrite(&(bp->b[2]),bp->b[0]*sizeof(long));
    }
    owrite_long(0);
}

void
renumber()
{
    int n;
    n=renumber2(S_CODE,0);
    renumber2(S_DATA,n);
}

/* BUGFIX 9/23/92: hp->rb->id must be wrapped with a bit stripper to ignore
 * memory type bits still in that longword.
 */

renumber2(fl,n)
    int fl;
    int n;
{
    int hc;
    for(hc=0;hc<hheader.hcount;hc++){
	hunk *hp = (*hlist)[hc].h;
	if(fl==S_CODE && HT(hp->rb->id)==HUNK_CODE){
	    hp->hunknum=n++;
	} else if(fl==S_DATA && HT(hp->rb->id)==HUNK_DATA){
	    hp->hunknum=n++;
	} else if(fl==S_DATA && HT(hp->rb->id)==HUNK_BSS){
	    hp->hunknum=n++;
	}
    }
    return n;
}

/* output package */
#ifndef SOUT
/* NB - this version does NOT cope with multiple output files per type */
int ofile;

void
out_start(prot)
    char *prot;
{
    datacount=codecount=0;
    file=open(ssubst(buf,prot),O_WRONLY|O_CREAT|O_TRUNC);
    if(ofile<0)panic("can't open output file");
}

void
out_stop(){
    close(ofile);
}

void
owrite_long(literal)
    long literal;
{
    long x=literal;
    owrite(&x,sizeof(x));
}

void
owrite_short(literal)
    short literal;
{
    short x=literal;
    owrite(&x,sizeof(x));
}

void
owrite(where,len)
    void *where;
    long len;
{
    write(ofile,where,len);
}
#else /* SOUT */
int ofile=0;
int osize;
char *oprot;
void
out_start(prot)
    char *prot;
{
    datacount=codecount=wsf_count=0;
    oprot=prot;
    new_file();
}

void
out_stop(){
    close(ofile);
    ofile=0;
}

void
owrite_long(literal)
    long literal;
{
    long x=literal;
    if((osize+sizeof(x))>SPLITSIZE)new_file();
    owrite(&x,sizeof(x));
    osize += sizeof(x);
}

void
owrite_short(literal)
    short literal;
{
    short x=literal;
    if((osize+sizeof(x))>SPLITSIZE)new_file();
    owrite(&x,sizeof(x));
    osize += sizeof(x);
}

void
owrite(where,len)
    void *where;
    long len;
{
    if((osize+len)>SPLITSIZE)new_file();
    write(ofile,where,len);
    osize += len;
}

void
new_file(){
    if(ofile)close(ofile);
    ofile=open(ssubst(buf,oprot),O_WRONLY|O_CREAT|O_TRUNC);
    if(ofile<0)panic("can't open output file");
    wsf_count++,datacount++,codecount++;
    osize=0;
}
#endif /* SOUT */

struct Node *Head(l)
    struct List *l;
{
    if(!l)panic("Head(NULL)\n");
    return l->lh_Head->ln_Succ?l->lh_Head:0;
}
struct Node *Tail(l)
    struct List *l;
{
    if(!l)panic("Tail(NULL)\n");
    return (l->lh_TailPred==(NODE_P)l)?0:l->lh_TailPred;
}
struct Node *Next(n)
    struct Node *n;
{
    if(!n)printf("Warning: Next(NULL)\n");
    return n?(n->ln_Succ->ln_Succ?n->ln_Succ:0):0;
}
struct Node *Prev(n)
    struct Node *n;
{
    if(!n)printf("Warning: Prev(NULL)\n");
    return n?(n->ln_Pred->ln_Pred?n->ln_Pred:0):0;
}

struct List *_fortemp;	/* scratch for foreach macro */

void
dump_after_read(struct List *root){
    file *f;
    foreach(f,root,(file *)){
        punit *p;
        printf("FILE '%s'\n",f->name);
        foreach(p,&(f->punits),(punit *)){
	    hunk *h;
	    print_text_block("\tPUNIT %.*s\n",p->unit_header);
	    if(p->libsize){
		printf("\tlibsize=%08x\n",p->libsize);
	    } else {
		/* */
	    }
	    foreach(h,&(p->hunks),(hunk *)){
		print_text_block("\t\tHUNK %.*s",h->name);
		printf(" @%08x\n",h);
		print_bin_block(h->rb);
		printf("\t\t\tCode Reloc\n");
		printf("\t\t\tData Reloc\n");
		if(h->merge)printf("\t\t\tmerge(%08x)\n",h->merge);
		if(h->hunkstart)printf("\t\t\thunkstart\n");
		if(h->hunkchain)printf("\t\t\thunkchain\n");
		if(h->hunkgone)printf("\t\t\thunkgone\n");
		printf("\t\t\toverlay(%08x) hunknum(%08x) offset(%08x)\n",
		  h->overlay,h->hunknum,h->hunkoffset);
	    }
        }
    }
}

void
print_text_block(char *fmt,block *b){
    if(!b){
	printf(fmt,10,"(no block)");
    } else {
	if(b->sw){
	    printf(fmt,13,"(swapped out)");
	} else {
	    if(!(b->b[1]) || !*(char*)&(b->b[2])){
		printf(fmt,6,"(null)");
	    } else {
		printf(fmt,b->b[1]*4,&(b->b[2]));
	    }
	}
    }
}

void
print_bin_block(block *b){
    if(b->sw){
	printf("\t\t\t(swapped out)\n");
    } else {
	printf("\t\t\tid1=%08x id2=%08x len=%08x\n", b->id,b->b[0],b->b[1]);
    }
}

/*
 * read routines
 */

/*
 * ReadSimpleBlock
 * If the given id is recognized as a simple block (id, length, data),
 * allocate and fill in a block structure.  Include the id in the block.
 */
block *ReadSimpleBlock(f,id)
    file *f;
    long id;
{
    long len;
    long hid;
    block *b;

    hid=id & 0x0fffffff;
    if(	hid !=HUNK_UNIT && hid != HUNK_NAME && hid != HUNK_CODE &&
    	hid != HUNK_DATA && hid != HUNK_BSS && hid != HUNK_DEBUG
      ){
	printf("%08x\n",id);
	panic("ReadSImpleBlock");
    }

    len=READLONG(EOF_BAD);
    b=NewBlock();
    b->id=id;
    b->sw=0;
    b->b=NewData((hid==HUNK_BSS)?2:len+2);
    b->b[0]=id;
    b->b[1]=len;
    if(hid != HUNK_BSS)read(f->fd,&(b->b[2]),len*4);
    return(b);
}

/*
 * TossSimpleBlock
 * Skip past something we don't need.
 */
int TossSimpleBlock(f)
    file *f;
{
    long len=READLONG(EOF_BAD);

    if(len)if( lseek(f->fd,len*4,1) == -1)panic("Toss failed\n");
    return(len);
}

/*
 * ReadHunk
 * Read an entire hunk, building lists of each block type in the given hunk
 * structure.  If we are listing, do the listing as we read so we can see
 * where things die if we hit a type code we don't recognize.
 */
hunk *ReadHunk(f)
    file *f;
{
    long id;
    hunk *h=NewHunk();

    while(1){
	id=READLONG(EOF_OK);
	switch(id & 0x0fffffff){	/* ignore memory type bits */
	case 0: return 0;		/* EOF - not good test */
	case HUNK_RELOC32:
	    LIST{printf("Reloc32:\n");}
	    ReadReloc(f,id,&h->reloc);break;
	case HUNK_CODE:
	    h->rb=ReadSimpleBlock(f,id);
	    LIST{printf("Code size %d\n",block_size(h->rb)*4);};
	    break;
	case HUNK_DATA:
	    h->rb=ReadSimpleBlock(f,id);
	    LIST{printf("Data size %d\n",block_size(h->rb)*4);};
	    break;
	case HUNK_BSS:
	    h->rb=ReadSimpleBlock(f,id);
	    LIST{printf("Bss size %d\n",block_size(h->rb)*4);};
	    break;
	case HUNK_SYMBOL:
	    while(TossSimpleBlock(f))READLONG(EOF_BAD);
	    LIST{printf("Symbols skipped\n");};
	    break;
	case HUNK_DEBUG:
	    (void)TossSimpleBlock(f);
	    LIST{printf("Debug hunk skipped\n");};
	    break;
	case HUNK_END:	LIST{printf("End of hunk\n");};return h;
	case HUNK_BREAK:LIST{printf("End of overlay\n");};break;
	default:
			printf("Lost id=0x%x\n",id);exit(2);
	}
    }
    return 0;
}

/*
 * ReadReloc
 * Read a relocation block and build a linked list of the sections.
 * If we are listing, do that now.
 */
void ReadReloc(f,id,ls)
    file *f;
    long id;
    struct List *ls;
{
    long len;
    block *cur;
    listlist *blist=NewListList();

    AddTail(ls,blist);
    blist->id=id;
    len=READLONG(EOF_BAD);
    while(len){
	cur=NewBlock();
	cur->b=NewData(len+2);
	read(f->fd,&(cur->b[1]),len*4+4);
	cur->b[0]=len;
	LIST{printf("\thunk #%d - %d items\n",cur->b[1],len);}
	AddTail(&blist->list,cur);
	len=READLONG(EOF_BAD);
    }
}

int rderror(){
    panic("read error\n");
    return 0;	/* just to make it quiet - NOTREACHED */
}

long block_size(blk)
    block *blk;
{
    return(blk->b[1]);
}

/* Allocation routines - if this was C++ then this code would be buried in the
 * constructors.  Doing it this way means we can re-write the allocation later
 * to allocate things we'll need lots of in larger blocks to avoid the time and
 * space penalties of malloc. */
file *NewFile(fname)
    char *fname;
    {
    file *ret=calloc(sizeof(file),1);

    NewList(&ret->punits);
    ret->name=strdup(fname);
    ret->fd= open(fname,O_RDONLY);
    return(ret);
}

punit *NewPunit(){
	punit *ret=calloc(sizeof(punit),1);
	NewList(&ret->hunks);
	return(ret);
}

hunk *NewHunk(){
    hunk *ret=calloc(sizeof(hunk),1);

    NewList(&ret->reloc);
    NewList(&ret->dreloc);
    NewList(&ret->extsym);
    ret->overlay=UNASSIGNED_HUNK;
    return(ret);
}

block *NewBlock(){
    return calloc(sizeof(block),1);
}

listlist *NewListList(){
    listlist *ret=calloc(sizeof(listlist),1);

    NewList(&ret->list);
    return(ret);
}

long *NewData(longs)
    long longs;	
    {
    return(malloc(longs*4));
}
