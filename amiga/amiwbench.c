/*    SCCS Id: @(#)amiwbench.c - Amiga Workbench interface  3.0   */
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1990	  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#undef TRUE
#undef FALSE
#undef COUNT
#undef NULL

#ifdef LATTICE
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#endif

#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <exec/memory.h>
#include <ctype.h>

#ifdef LATTICE
#include <string.h>
#undef strlen			/* grrr */
#endif

#define ALLOC_SIZE		((long)sizeof(struct FileInfoBlock))

#ifdef AZTEC_C
/*
 * Change when manx becomes ANSI complient
 */
BPTR FDECL(CurrentDir,(BPTR));
BPTR FDECL(ParentDir, (BPTR));
BPTR FDECL(Lock, (char *, long));
void *FDECL(AllocMem, (long, long));
void FDECL(FreeMem, (void *, long));
unsigned short FDECL(Examine, (BPTR, struct FileInfoBlock *));
struct Library *FDECL(OpenLibrary,(char *, long));
struct DiskObject *FDECL(GetDiskObject, (char *));

extern struct Library *IconBase;
#endif

#ifdef AMIGA_WBENCH
static void FDECL(ami_wb_findme,(char *,char *,struct WBArg *));
static int FDECL(buildPath,(LONG,struct FileInfoBlock *,char *));
static void FDECL(insert,(char *,char *));

BOOL FromWBench=0;		/* if FALSE, this file is a big NOP */
static BOOL FromTool=0;		/* or from Project (ergo nothing to restore) */
static char argline[80];	/* fake command line from ToolTypes */
static BOOL TTparse=0;		/* parsing tooltypes? */
static BOOL KillIcon=FALSE;	/* delayed expunge of user's icon */
static char iconname[PATHLEN+5];
static char origicon[PATHLEN+5];
static char savefname[PL_NSIZ];		/* name from name of save file */

extern const char *classes;	/* liberated from pcmain */
extern char *PATH;

/* Called after NetHack.cnf (and maybe NETHACKOPTIONS) are read.
 * If this is a request to show the score file, do it here and quit.
 */
void ami_wbench_init(argc,argv)
int argc;
char *argv[];
{
	struct WBStartup *wbs=(struct WBStartup *)argv;
	struct WBArg *wa;
	int	ia;			/* arg of active icon */
	int	x,doscore=0;
	char 	*p,*lp;
	BPTR	olddir;			/* starting directory */
	struct DiskObject *dobj;
	char	*scorearg;
	char	tmp_ramdisk[PATHLEN];
	char	tmp_levels[PATHLEN];

	FromWBench=(argc==0);
	if(!FromWBench)return;			/* nothing if from CLI */

	/*
	 * "NULL" out arrays
	 */
	tmp_ramdisk[0] = '\0';
	tmp_levels[0]  = '\0';

	IconBase=OpenLibrary("icon.library",33L);
	if(!IconBase)error("icon.library missing!");

	wa=wbs->sm_ArgList;
	if(wbs->sm_NumArgs>2)error("You can only play one game at a time!");
	ia=wbs->sm_NumArgs-1;
	strcpy(savefname,wa[ia].wa_Name);
	if(!strncmp(index(savefname,'.'),".sav",4)){
		*index(savefname,'.')='\0';
	} else {
		savefname[0]='\0';	/* don't override if not save file */
	}

	olddir=CurrentDir(wa[ia].wa_Lock);   /* where the icon is */

	dobj=GetDiskObject(wa[ia].wa_Name);
	(void)CurrentDir(olddir);		/* and back */
	if(!dobj){
		error("Sorry, I can't find your icon!");
	}

	FromTool=(dobj->do_Type==WBTOOL)?1:
			(dobj->do_Type==WBPROJECT)?0:
			(error("Sorry, I don't recognize this icon type!"),1);

	ami_wb_findme(SAVEF,SAVEP,&wa[ia]);
	strcpy(origicon,SAVEF);
	strcat(origicon,".info");

	argline[0]='\0';
	if(dobj->do_ToolTypes)for(x=0;p=dobj->do_ToolTypes[x];x++){
		lp=index(p,'=');
		if(!lp++){
			if((strncmp(p,"SCORES",6)==0) ||
			   (strncmp(p,"SCORE",5)==0)){
				doscore=1;
				scorearg=malloc(strlen(p)+1);
				strcpy(scorearg,p);
			} else {
				TTparse=TRUE;
				parseoptions(p,(boolean)TRUE);
				TTparse=FALSE;
			}
		} else {
			while(*lp && isspace(*lp))lp++;
				/* vars and lengths below match amidos.c,
				 * but there is no SAVE - you put the
				 * icon where you want it, and GRAPHICS
				 * just doesn't belong		*/
			if(!strncmp(p,"OPTIONS",4)){
				TTparse=TRUE;
				parseoptions(lp,(boolean)TRUE);
				TTparse=FALSE;
			} else
			if(lp[0]=='#'){
				/* for perversity's sake, a comment */
			} else
			if(!strncmp(p,"HACKDIR",4)){
				strncpy(hackdir,lp,PATHLEN);
			} else
			if(!strncmp(p,"RAMDISK",3)){
				strncpy(tmp_ramdisk,lp,PATHLEN);
			} else
			if(!strncmp(p,"LEVELS",4)){
				strncpy(tmp_levels,lp,PATHLEN);
			} else
			if(!strncmp(p,"PATH",4)){
				strncpy(PATH,lp,PATHLEN);
			} else
				/* new things */
			if((strncmp(p,"CMDLINE",7)==0)||
			   (strncmp(p,"COMMANDLINE",11)==0)){
				strncpy(argline,lp,79);
			} else
			{
				msmsg("Bad ToolTypes line: '%s'\n",p);
				getreturn("to continue");
			}
		}
	}
		/* cleanup - from amidos.c, except we only change things
		 * that are explicitly changed, since we already
		 * did this once to get the defaults (in amidos.c)	*/
	if(plname[0])plnamesuffix();	/* from amidos.c */
	if(tmp_levels[0])strcpy(permbones,tmp_levels);
	if(tmp_ramdisk[0]){
		strcpy(levels,tmp_ramdisk);
		strcpy(bones,levels);
		if(strcmp(permbones,levels))
			ramdisk=TRUE;
	} else {
		if(tmp_levels[0]){
			strcpy(levels,tmp_levels);
			strcpy(bones,levels);
		}
	}

	FreeDiskObject(dobj);	/* we'll get it again later if we need it */

	if(doscore){
		long ac;
		char *p;
		char **av=calloc(1,50*sizeof(char *));
#ifdef CHDIR
		chdirx(hackdir,0);
#endif
		av[0]="NetHack";			/* why not? */
		for(ac=1,p=scorearg;*p;ac++){
			av[ac]=p;
			while(*p && !isspace(*p))p++;
			if(!*p)break;
			*p++='\0';
			while(*p && isspace(*p))p++;
			if(ac==1)sprintf(av[ac],"-s");	/* overwrite SCORES */
		}
		prscore(ac+1,av);
		exit(0);		/* overloaded */
	}

			/* if the user started us from the tool icon,
			 * we can't save the game in the same place
			 * we started from, so pick up the plname
			 * and hope for the best.
			 */
	if(FromTool){
		strcat(SAVEF,plname);
		strcat(SAVEP,plname);
	}
}

/* Simulate the command line (-s is already done, although this is
 * not exactly the way it should be). Note that we only handle flags
 * that are not otherwise available in NetHack.cnf		*/
void ami_wbench_args(){
	char *p=argline;
	if(!FromWBench)return;
	if(!argline)return;

	while(*p){
		switch(*p++){
		case '-':	break;
#ifdef NEWS
		case 'n':	flags.nonews = TRUE;
#endif
#if defined(WIZARD) || defined(EXPLORE_MODE)
# ifndef EXPLORE_MODE
		case 'X':
# endif
		case 'D':
# ifdef WIZARD
#  ifdef KR1ED
			if(!strcmp(plname,WIZARD_NAME)){
#  else
			if(!strcmp(plname,WIZARD)){
#  endif
				wizard=TRUE;break;
			}
			/* else fall through */
# endif
# ifdef EXPLORE_MODE
		case 'X':	discover=TRUE;
# endif
				break;
#endif
#ifdef DGK
		case 'r':	/* no ram disk */
			ramdisk=FALSE;
			break;
#endif
		default:
			p--;
			if(index(classes,toupper(*p))){
				char *t=pl_character;
				int cnt=sizeof(pl_character)-1;
				while(cnt && *p && !isspace(*p))*t++=*p++,cnt--;
				*t=0;
			} else {
				Printf("Unknown switch: %s\n",p);
				return;
			}
		}
	}
}


/* IF (from workbench) && (currently parsing ToolTypes)
 * THEN print error message and return 0
 * ELSE return 1
 */
ami_wbench_badopt(oopsline)
char *oopsline;
{
	if(!FromWBench)return 1;
	if(!TTparse)return 1;
	Printf("Bad Syntax in OPTIONS in ToolTypes: %s.",oopsline);
	return 0;
}

/* Construct (if necessary) and fill in icon for given save file */
void ami_wbench_iconwrite(base)
char *base;
{
	BPTR lock;
	char tmp[PATHLEN+5];

	if(!FromWBench)return;

	strcpy(tmp,base);
	strcat(tmp,".info");
	if(FromTool){				/* user clicked on main icon */
		(void)CopyFile(DEFAULT_ICON,tmp);
	} else {				/* from project */
		lock=Lock(tmp,ACCESS_READ);
		if(lock==0){	/* maybe our name changed - try to get
				 * original icon */
		    if(!Rename(origicon,tmp)){
				/* nope, build a new icon */
			lock=Lock(DEFAULT_ICON,ACCESS_READ);
			if(lock==0)return;		/* no icon today */
			UnLock(lock);
			(void)CopyFile(DEFAULT_ICON,tmp);
		    }
		} else UnLock(lock);
	}
	KillIcon=FALSE;

/*	dobj=GetDiskObject(base);
	anything we need to change?  I don't think so.
	PutDiskObject(base,dobj);
	FreeDiskObject(dobj);
*/
}

/* How much disk space will we need for the icon? */
int ami_wbench_iconsize(base)
char *base;
{
	struct FileInfoBlock *fib;
	BPTR lock;
	int	rv;
	char tmp[PATHLEN+5];

	if(!FromWBench)return(0);
	strcpy(tmp,base);
	strcat(tmp,".info");
	lock=Lock(tmp,ACCESS_READ);
	if(lock==0){	/* check the default */
		lock=Lock(DEFAULT_ICON,ACCESS_READ);
		if(lock==0)return(0);
	}
	fib = (struct FileInfoBlock *)AllocMem(ALLOC_SIZE, MEMF_CLEAR);
	if(!Examine(lock,fib)){
		UnLock(lock);
		FreeMem(fib, ALLOC_SIZE);
		return(0);			/* if no icon, there
						 * never will be one */
	}
	rv=fib->fib_Size+strlen(plname);	/* guessing */
	UnLock(lock);
	FreeMem(fib, ALLOC_SIZE);
	return(rv);
}

/* Delete the icon associated with the given file (NOT the file itself! */
/* (Don't worry if the icon doesn't exist */
void ami_wbench_unlink(base)
char *base;
{
	if(!FromWBench)return;

	strcpy(iconname,base);
	strcat(iconname,".info");
	KillIcon=TRUE;			/* don't do it now - this way the user
					 * gets back whatever picture we had
					 * when we started if the game is
					 * saved again			 */
/*	unlink(tmp); */
}

/* Check for a saved game.
IF not a saved game -> -1
IF can't open SAVEF -> -1
ELSE -> fd for reading SAVEF */
int ami_wbench_getsave(mode)
int mode;
{
	BPTR lock;
	struct FileInfoBlock *fib;

	if(!FromWBench)return(open(SAVEF,mode));
			/* if the file will be created anyway, skip the
			 * checks and just do it			*/
	if(mode & O_CREAT)return(open(SAVEF,mode));
	if(FromTool)return(-1);		/* otherwise, by definition, there
					 * isn't a save file (even if a
					 * file of the right name exists) */
	if(savefname[0])
		strncpy(plname,savefname,PL_NSIZ-1); /* restore poly'd name */
	lock=Lock(SAVEF,ACCESS_READ);
	fib = (struct FileInfoBlock *)AllocMem(ALLOC_SIZE, MEMF_CLEAR);
	if(lock && Examine(lock,fib)){
		if(fib->fib_Size>100){	/* random number << save file size */
			UnLock(lock);
                        FreeMem(fib,ALLOC_SIZE);
			return(open(SAVEF,mode));
		} else {
				/* this is a dummy file we need because
				 * workbench won't duplicate an icon with no
				 * "real" data attached - try to get rid of it.
				 */
			UnLock(lock);
			unlink(SAVEF);
			FreeMem(fib,ALLOC_SIZE);
			return(-1);
		}
	}
	FreeMem(fib,ALLOC_SIZE);
	return(-1);		/* give up */
}

#ifdef notdef
/* cleanup */
void ami_wbench_cleanup(){
	if(!FromWBench)return;
	if(KillIcon){
		unlink(iconname);
	} else {
		if(!FromTool){	/* game started and ended in one session */
			char buf[PATHLEN+5];
			strcpy(buf,SAVEF);
			strcat(buf,".info");
			unlink(buf);
		}
	}
}
#endif

/* get printable version of where we came from */
static void ami_wb_findme(bufp,dirp,wa)
	char *bufp,*dirp;
	struct WBArg *wa;
	{
	BPTR dir;
	struct FileInfoBlock *fib;
	char *p;
	int len;

	dir=wa->wa_Lock;

	fib = (struct FileInfoBlock *)AllocMem(ALLOC_SIZE,MEMF_CLEAR);
	buildPath(dir,fib,dirp);
	strcat(dirp,"/");
	strcpy(bufp,dirp);
	if(FromTool){
		/* do nothing - filename will be added later */
	} else {
		strcat(bufp,wa->wa_Name);
	};
	/* I know this looks redundent, but its not since we may add
	 * a slash after returning from buildPath
	 */
	p=index(bufp,':');
	if(!p){
		p=index(bufp,'/');
		if(p)*p=':';
	}
	p=index(dirp,':');
	if(!p){
		p=index(dirp,'/');
		if(p)*p=':';
	}
	/* We found the icon - but we need the main file. */
	len=strlen(bufp);
	if(len<5)return;			/* who knows? */
	if(strcmp(".info",&bufp[len-5]))return; /* who knows? */
	bufp[len-5]='\0';
        FreeMem(fib, ALLOC_SIZE);
}

/* Carolyn Scheppner - CATS, AmigaMail II-34 */
static int
buildPath(inlock,fib,buf)
LONG inlock;
struct FileInfoBlock *fib;       	/* ASSUMED LONGWORD BOUNDARY!! */
char *buf;
	{
	int i;
	LONG lock,oldlock;
	BOOL MyOldLock = FALSE;

	buf[0]='\0';
	lock=inlock;

	while(lock){
		if(Examine(lock,fib)){
			if(fib->fib_FileName[0]>' '){
				if(buf[0])insert(buf,"/");
				insert(buf,fib->fib_FileName);
			}
		}
		oldlock=lock;
		lock=ParentDir(lock);
		if(MyOldLock) UnLock(oldlock);
		else MyOldLock=TRUE;
	}
	if(fib->fib_FileName[0]>' '){
		for(i=0;i<(strlen(buf));i++){
			if(buf[i]=='/'){
				buf[i]=':';
				break;
			}
		}
	}
	else insert(buf,"RAM:");
	return((int)strlen(buf));
}
static void
insert(buf,s)
char *buf,*s;
{
	char tmp[256];
	strcpy(tmp,buf);
	strcpy(buf,s);
	strcpy(&buf[strlen(s)],tmp);
}

#if 0 /* CopyFile should be OK */
static void copyicon(from,to)
char *from,*to;
{
	int df,dt;
	char buf[512];
	int len=512;

	df=open(from,O_RDONLY);
	dt=open(to,O_WRONLY,0);
	while(len=512){
		len=read(df,buf,len);
		write(dt,buf,len);
	}
	close(df);
	close(dt);
}
#endif
#endif /* AMIGA_WBENCH */
