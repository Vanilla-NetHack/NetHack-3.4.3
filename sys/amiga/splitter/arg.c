/*    SCCS Id: @(#)arg.c		3.1   93/01/08
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * arg.c - semi-generic argument parser
 */
#include <exec/types.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "arg.h"

static char *a_desc;
static int a_argc;
static char **a_argv;
static int a_apos;
static char more=0;
static char *argp;

static char *arg_fillbuf(void);
/*static char *splitline(struct fentry *); L505 bug? see below */
static char *splitline2(char *);

char *argarg;

#ifndef ARGLEN
#define ARGLEN 134		/* longest line we can deal with */
#endif

static struct List flist;	/* stack of files to read from */
struct fentry {			/* structures on that stack */
	struct Node node;
	FILE *fp;
	char *ptr;
	char buf[ARGLEN];
};

static char *splitline(struct fentry *); /* L505 bug? can't be above def  */

#define ListHead(x) ((x).lh_Head)
#define ListEmpty(x) (!(x).lh_Head->ln_Succ)

/*
 * arg_init(description string, argc, argv)
 * Called by user to initialize system - must be called before calling
 * other entry points.  desc is getopt(3) style description string; argc and
 * argv are the arguments to main().
 */
void arg_init(char *desc,int argc,char *argv[]){
	a_desc=desc;
	a_argc=argc;
	a_argv=argv;
	a_apos=0;
	NewList(&flist);
}

/*
 * arg_next(void)
 * Called by user to return each argument.  See arg.h for exceptional return
 * values - normally returns the argument flag found; if a flag takes an
 * argument, the argument is returned in argarg.
 * An argument beginning with @ is taken to be a file name from which to read
 * further arguments - such files are held on a stack and read as found, then
 * the previous file (or the command line) is continued.
 * In an argument file, the following are recognized:
 * #  as the first character of a line only, causes the line to be ignored
 * @  recursively read arguments from another file
 * \n embedded newline
 * \r embedded carriage return
 * \f embedded formfeed
 * \b embedded backspace
 * \  literal next character (except above)
 * "  start/end double quoted string
 * '  start/end single quoted string
 */
int arg_next(){
	char *cp;
	char key;

	if(!more){				/* anything still buffered? */
		if(!(argp=arg_fillbuf())){	/* nothing more */
			argarg=0;		/* be neat */
			return(ARG_DONE);
		}
	}
	if(more ||(*argp=='-' && argp++)){
		for(cp=a_desc;*cp;cp++){
			if(*cp== *argp){
				key=*argp++;
				if(*++cp==':'){
					if(*argp){
						argarg=argp;
					}else{
						argarg=arg_fillbuf();
						if(!argarg)return(ARG_ERROR);
					}
					more=0;
				}else{
					argarg=0; /* doesn't take an arg */
					more= *argp;
				}
				return((int) key);
			}
		}
		return(ARG_ERROR);	/* no such option */
	}else{
		argarg=argp;
		more=0;
		return(ARG_FREE);
	}
}

static char *arg_fillbuf(){
	char *p,*nlp;;

	if(ListEmpty(flist)){
		if(++a_apos>a_argc)return(0);
		p=a_argv[a_apos];
	}else{
		struct fentry *f=(struct fentry *)ListHead(flist);
		if(!f->ptr){
			do{
				if(!fgets(f->buf,ARGLEN,f->fp)){
					if(ferror(f->fp)){
						fprintf(stderr,
							"I/O error on @file\n");
						return(0);
					}
					fclose(f->fp);
					RemHead(&flist);
					free(f);
					return(arg_fillbuf());
				}
			}while(f->buf[0]=='#');	/* comment */
			if(nlp=strchr(f->buf,'\n'))*nlp='\0';
		}
		p=splitline(f);
		if(p== (char *)-1)return(0);		/* error */
		if(!p)return(arg_fillbuf());	/* skip blank line */
	}
	if(p && *p=='@'){
		struct fentry *f=calloc(sizeof(struct fentry),1);
		f->fp=fopen(++p,"r");
		if(!(f->fp)){
			fprintf(stderr,"Can't open @file '%s'\n",p);
			free(f);
			return(0);
		}
		AddHead(&flist,(struct Node *)f);
		return(arg_fillbuf());
	}
	return(p);
}

static char *splitline(struct fentry *f){
	char *out=(f->ptr?f->ptr:f->buf);
	char *ret;
	while(*out && isspace(*out))out++;
	if(!*out)return(0);	/* blank line or spaces at end */
	ret=out;
	while(*out && !isspace(*out)){
		switch(*out){
		case '\\':
		case '\"':
		case '\'':
			out=splitline2(out);
			if(!out)return((char *)-1);	/* error */
			break;
		default:
			out++;
			break;
		}
	}
	if(!*out){
		f->ptr=0;	/* this was last arg on current line */
	}else{
		*out='\0';
		f->ptr= ++out;
		if(!(*f->ptr))f->ptr=0;
	}
	return ret;
}

static char *splitline2(char *p){
	char *out=p;
	char c;
	char dq=0,sq=0;
	while(*p){
		switch(c= *p++){
		case '\\':
			switch(c= *p++){
			case 'n':	*out++='\n';break;
			case 'r':	*out++='\r';break;
			case 'f':	*out++='\f';break;
			case 'b':	*out++='\b';break;
			case 0:		p--;break;
			default:	*out++=c;break;
			}
			break;
		case '\"':	if(sq){
					*out++=c;
				}else{
					dq=1-dq;
				}
				break;
		case '\'':	if(dq){
					*out++=c;
				}else{
					sq=1-sq;
				}
				break;
		case ' ':	if(!sq && !dq){*out=0;return(p-1);}
				*out++=' ';
				break;
		default:	*out++=c;break;
		}
	}
	if(sq ||dq){
		fprintf(stderr,"Warning - quote error in @file\n");
		return((char *)-1);
	}
	*out=0;
	return(p);
}

#ifdef DEBUG_ARG
main(int argc,char *argv[]){
	int x=0;
	arg_init(getenv("test_arg"),argc,argv);
	do{
		x=arg_next();
		printf("r=%d (%d)'%s'\n",x,argarg,argarg);
	}while(x >= 0);
}
#endif /* DEBUG_ARG */
