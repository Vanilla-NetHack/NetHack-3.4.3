/* 	SCCS Id: @(#)multi.c 3.1	93/01/08
/*	Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * NB - internal structure under development.  End users should NOT
 *      get too creative!
 */
#include <exec/types.h>
#include <exec/memory.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <dos.h>
#include <string.h>
#include <assert.h>
#include "multi.h"

static int start_next_file(multifh *);	/* should return enum */
BPTR
MultiOpen(char *dirfile, ULONG mode, union multiopts *mo){
	multifh *retval;

	assert(mode==MODE_OLDFILE);	/* no chioce this version */
	retval=(multifh *)AllocMem(sizeof(multifh),MEMF_CLEAR);
	if(retval){
		retval->mfh_dirfh=Open(dirfile,MODE_OLDFILE);
		if(retval->mfh_dirfh){
			retval->mfh_mo= *mo;
			if(start_next_file(retval)==1){
				return((BPTR)retval);		/* success */
			}
		}
	}

	if(retval)FreeMem(retval,sizeof(multifh));
	return 0;
}

ULONG
MultiRead(BPTR xmfp, ULONG *where, ULONG len){
	multifh *mfp=(multifh *)xmfp;
	ULONG sofar=0;
	ULONG this;

	if(len<=0)return len;
	if(mfp->mfh_fh==0)return 0;	/* pending EOF (possibly premature) */

	while(sofar<len){
		this=Read(mfp->mfh_fh,where,len-sofar);
		if(this==-1) return -1;
		if(this==0){
			Close(mfp->mfh_fh);
			mfp->mfh_fh=0;
			if(start_next_file(mfp)<=0){
				return sofar;
			}
		}
		sofar += this; where += this;
	}
	return sofar;
}

void
MultiClose(BPTR xmfp){
	multifh *mfp=(multifh *)xmfp;
	if(mfp->mfh_dirfh)Close(mfp->mfh_dirfh);
	if(mfp->mfh_fh)Close(mfp->mfh_fh);
	FreeMem(mfp,sizeof(multifh));
}

/* return 0==no more data, -1 error.  else more data available unless file
 * is empty
 */
static
start_next_file(multifh *mfp){
	ULONG t;
	char line[100];		/* should be based on PATHLEN */
	char *eol;

	while(1){
		t=Read(mfp->mfh_dirfh,line,99);
		if(t==0)return(0);
		if(t==-1)return(-1);

		line[t]='\0';
		eol=strchr(line,'\n');
		if(eol){
			*eol='\0';
			Seek(mfp->mfh_dirfh,-(t-(eol-line))+1,OFFSET_CURRENT);
		}
		switch(line[0]){
		case '\0':
		case '#':
			break;			/* comment, blank lines */
		default:
			if(line[0]==mfp->mfh_mo.r.mor_tag){
						/* allow blanks after tag */
				char *file= &line[1];
				while(*file && isspace(*file))file++;
				mfp->mfh_fh=Open(file,MODE_OLDFILE);
				if(!mfp->mfh_fh){
					return -1;	/* error */
				}
				return 1;
			}
		}
	}
}
