/*    SCCS Id: @(#)amiwbench.c      3.1   93/01/08
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1990, 1992, 1993 */
/* NetHack may be freely redistributed.  See license for details. */

/*  Amiga Workbench interface  */

#include "hack.h"

/* Have to #undef CLOSE, because it's used in display.h and intuition.h */
#undef CLOSE

#ifdef __SASC
# undef COUNT
# include <proto/exec.h>
# include <proto/dos.h>
# include <proto/icon.h>
#endif

#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <exec/memory.h>
#include <ctype.h>

#ifdef __SASC
# include <string.h>
# undef strlen          /* grrr */
#endif

#define ALLOC_SIZE      ((long)sizeof(struct FileInfoBlock))

#ifdef AZTEC_C
# include <functions.h>
extern struct Library *IconBase;
#endif

extern void NDECL( preserve_icon );
extern void NDECL( clear_icon );
#ifndef	SHAREDLIB
extern void FDECL( amii_set_text_font, ( char *, int ) );
#endif
extern int FDECL(parse_config_line, (FILE *, char *, char *, char *));
extern char *FDECL( ami_default_icon, ( char * ) );

int ami_argc;           /* global argc */
char **ami_argv;        /* global argv */
boolean FromWBench=0;       /* how did we get started? */
boolean FromCLI=0;      /* via frontend and INTERNALCLI */
static BOOL FromTool=0;     /* or from Project (ergo nothing to restore) */
static char argline[80];    /* fake command line from ToolTypes */
static BOOL TTparse=0;      /* parsing tooltypes? */
static BOOL KillIcon=FALSE; /* delayed expunge of user's icon */
static char iconname[PATHLEN+5];
static char origicon[PATHLEN+5];
static char savefname[PL_NSIZ];     /* name from name of save file */
int amibbs=0;			/* BBS mode */

#ifdef AMII_GRAPHICS
extern int bigscreen;
#endif
extern const char *classes; /* liberated from pcmain */
extern char PATH[];

static void score(char *);

/* Called after NetHack.cnf (and maybe NETHACKOPTIONS) are read.
 * If this is a request to show the score file, do it here and quit.
 */
void ami_wbench_init()
{
    struct WBStartup *wbs=(struct WBStartup *)ami_argv;
    struct WBArg *wa;
    int ia;         /* arg of active icon */
    int x,doscore=0;
    char    *p,*lp;
    BPTR    olddir;         /* starting directory */
    struct DiskObject *dobj;
    char    *scorearg, *t;
    char    tmp_levels[PATHLEN];

    FromWBench=(ami_argc==0);
    if(!FromWBench)return;          /* nothing if from CLI */

    /*
     * "NULL" out arrays
     */
    tmp_levels[0]  = '\0';

    IconBase=OpenLibrary("icon.library",33L);
    if(!IconBase)error("icon.library missing!");

    wa=wbs->sm_ArgList;
    if(wbs->sm_NumArgs>2)error("You can only play one game at a time!");
    ia=wbs->sm_NumArgs-1;

    if(strcmp("NewGame",wa[ia].wa_Name)){
	strcpy(savefname,wa[ia].wa_Name);
	strcpy(plname,wa[ia].wa_Name);
    }

    if( ( t = strrchr( plname, '.' ) ) && strcmp( t, ".sav" ) == 0 )
	*t = 0;

    olddir=CurrentDir(wa[ia].wa_Lock);   /* where the icon is */

    dobj=GetDiskObject(wa[ia].wa_Name);
    (void)CurrentDir(olddir);       /* and back */
    if(!dobj){
	error("Sorry, I can't find your icon!");
    }

    FromTool=(dobj->do_Type==WBTOOL)?1:
	(dobj->do_Type==WBPROJECT)?0:
	(error("Sorry, I don't recognize this icon type!"),1);

    if(index(savefname,'.') && !strncmp(index(savefname,'.'),".sav",4)){
	*index(savefname,'.')='\0';
    } else {
	savefname[0]='\0';  /* don't override if not save file */
	FromTool = 1;
    }

    argline[0]='\0';
#ifdef AMII_GRAPHICS
    if( p = FindToolType( dobj->do_ToolTypes, "SCREEN" ) )
    {
	extern int bigscreen;
	if( MatchToolValue( p, "NOLACE" ) )
	    bigscreen = -1;
	else if( MatchToolValue( p, "LACE" ) )
	    bigscreen = 1;
    }
#endif
    if(dobj->do_ToolTypes)for(x=0;p=dobj->do_ToolTypes[x];x++){
	lp=index(p,'=');
	if( !lp || strncmp(p, "SCORE", 5 ) == 0 ){
	    if((strncmp(p,"SCORES",6)==0) || (strncmp(p,"SCORE",5)==0)){
		if( !lp )
		    lp = "";
		else
		    ++lp;
		doscore=1;
		scorearg=(char *)alloc(strlen(lp)+1);
		strcpy(scorearg,lp);
	    } else {
		TTparse=TRUE;
		parseoptions(p,(boolean)TRUE,(boolean)FALSE);
		TTparse=FALSE;
	    }
	} else {
	    lp++;
	    TTparse=TRUE;
		/* new things */
	    if((strncmp(p,"CMDLINE",7)==0)||
	      (strncmp(p,"COMMANDLINE",11)==0)||
	      (strncmp(p,"INTERNALCLI",11)==0)){
	    	strncpy(argline,lp,79);
		if(*p=='I'){
		    FromTool=0; /* ugly hack bugfix */
		    FromCLI=1;  /* frontend ICLI only */
		    if(*argline==':'){
			char *x=lp;
			amibbs=1;
#ifdef for_later
			copy to next blank in argline to amibasename
			delete from argline
#else
			while(*x && !isspace(*x))x++;
			while(*x && isspace(*x))x++;
			strncpy(argline,x,79);
#endif
		    }
		}
	    }
	    else if( strncmp( p, "FONT", 4 ) == 0 )
	    {
		if( p = strdup( lp ) )
		{
		    lp = strchr( p, ':' );
		    *lp++ = 0;
		    amii_set_text_font( p, atoi( lp ) );
		    free( p );
		}
	    }
	    else if( strncmp( p, "SCREEN",6 ) )
	    {
		if (!parse_config_line((FILE *)0, p, 0, tmp_levels)){
		    raw_printf("Bad ToolTypes line: '%s'\n",p);
		    getreturn("to continue");
		}
	    } 
	    TTparse=FALSE;
	}
    }

    /* cleanup - from files.c, except we only change things
     * that are explicitly changed, since we already
     * did this once to get the defaults (in amidos.c)
     */

    if(plname[0]){
	plnamesuffix(); /* from files.c */
	set_savefile_name();
    }
    if(tmp_levels[0])strcpy(permbones,tmp_levels);
    if(tmp_levels[0]){
	strcpy(levels,tmp_levels);
	strcpy(bones,levels);
    }
    FreeDiskObject(dobj);   /* we'll get it again later if we need it */

    if(doscore)score(scorearg);

	    /* if the user started us from the tool icon,
	     * we can't save the game in the same place
	     * we started from, so pick up the plname
	     * and hope for the best.
	     */
    if(FromTool){
	set_savefile_name();
    }
}

/* Simulate the command line. Note that we do not handle the
 * entire range of standard NetHack flags.
 */
void ami_wbench_args(){
    char *p=argline;

    if(!FromWBench) return;

    while(*p){
	switch(*p++){
	case ' ':
	case '-':   break;
#ifdef NEWS
	case 'n':   flags.news = FALSE;
#endif
#if defined(WIZARD) || defined(EXPLORE_MODE)
# ifndef EXPLORE_MODE
	case 'X':
# endif
	case 'D':
# ifdef WIZARD
#  ifdef KR1ED
	    if(!strcmp(plname,WIZARD_NAME))
#  else
	    if(!strcmp(plname,WIZARD))
#  endif
	    {
		wizard=TRUE;break;
	    }
	    /* else fall through */
# endif
# ifdef EXPLORE_MODE
	case 'X':   discover=TRUE;
# endif
	    break;
#endif
#ifdef AMII_GRAPHICS
	case 'L':   /* interlaced screen */
	    bigscreen = 1;
	    break;
	case 'l':   /* No interlaced screen */
	    bigscreen = -1;
	    break;
#endif
	case 'u':
	    {
	    char *c,*dest;
	    while(*p && isascii(*p) && isspace(*p))p++;
	    c=p;
	    dest=plname;
	    for(;*p && isascii(*p) && !isspace(*p);){
		if(dest-plname>=(sizeof(plname)-1))break;
		*dest++=*p++;
	    }
	    *dest='\0';
	    if(c==dest)
		raw_print("Player name expected after -u");
	    }
	    strcpy(savefname,plname);
	    set_savefile_name();
	    break;
	case 's':
	    score(p);
	    /* NOTREACHED */
	default:
	    p--;
	    if(index(classes,toupper(*p))){
		char *t=pl_character;
		int cnt=sizeof(pl_character)-1;
		while(cnt && *p && !isspace(*p))*t++=*p++,cnt--;
		*t=0;
		break;
	    }
	    raw_printf("Unknown switch: %s\n",p);
	    /* FALL THROUGH */
	case '?':
	    {
		char buf[77];

		raw_printf("Usage: %s -s [-[%s]] [maxrank] [name]...",
		  hname, classes);
		raw_print("       or");
		sprintf(buf,"       %s [-u name] [-[%s]]", hname, classes);
#if defined(WIZARD) || defined(EXPLORE_MODE)
		strcat(buf," [-[DX]]");
#endif
#ifdef NEWS
		strcat(buf," [-n]");
#endif
#ifdef MFLOPPY
# ifndef AMIGA
		strcat(" [-r]");
# endif
#endif
		raw_print(buf);
		exit(0);
	    }
	}
    }
}


/* IF (from workbench) && (currently parsing ToolTypes)
 * THEN print error message and return 0
 * ELSE return 1
 */
ami_wbench_badopt(oopsline)
const char *oopsline;
{
    if(!FromWBench)return 1;
    if(!TTparse)return 1;

    raw_printf("Bad Syntax in OPTIONS in ToolTypes: %s.",oopsline);
    return 0;
}

/* Construct (if necessary) and fill in icon for given save file */
void ami_wbench_iconwrite(base)
char *base;
{
    BPTR lock;
    char tmp[PATHLEN+5];
    char *n;

    if(!FromWBench)return;
    if(FromCLI)return;

    strcpy(tmp,base);
    strcat(tmp,".info");

    /* Get the name of the icon */
    n = ami_default_icon( DEFAULT_ICON );
    if(FromTool){               /* user clicked on main icon */
	(void)CopyFile( n, tmp );
    } else {                /* from project */
	lock=Lock(tmp,ACCESS_READ);
	if(lock==0){    /* maybe our name changed - try to get
			 * original icon */
	    if(!Rename(origicon,tmp)){
		/* nope, build a new icon */
	    lock=Lock( n, ACCESS_READ);
	    if(lock==0)return;      /* no icon today */
	    UnLock(lock);
	    (void)CopyFile( n,tmp);
	    }
	} else UnLock(lock);
    }
    KillIcon=FALSE;
}

/* How much disk space will we need for the icon? */
int ami_wbench_iconsize(base)
char *base;
{
    struct FileInfoBlock *fib;
    BPTR lock;
    int rv;
    char tmp[PATHLEN+5];

    if(!FromWBench)return(0);
    if(FromCLI)return(0);

    strcpy(tmp,base);
    strcat(tmp,".info");
    lock=Lock(tmp,ACCESS_READ);
    if(lock==0){    /* check the default */
	lock=Lock( ami_default_icon( DEFAULT_ICON ),ACCESS_READ);
	if(lock==0)return(0);
    }
    fib = (struct FileInfoBlock *)AllocMem(ALLOC_SIZE, MEMF_CLEAR);
    if(!Examine(lock,fib)){
	UnLock(lock);
	FreeMem(fib, ALLOC_SIZE);
	return(0);          /* if no icon, there never will be one */
    }
    rv=fib->fib_Size+strlen(plname);    /* guessing */
    UnLock(lock);
    FreeMem(fib, ALLOC_SIZE);
    return(rv);
}

char *
ami_default_icon( defname )
    char *defname;
{
    long lock;
    static char name[ 300 ];

    strcpy( name, "NetHack:x.icon" );
    name[ 8 ] = pl_character[ 0 ];

    if( access( name, 0 ) == 0 )
    	return( name );

    return( defname );
}

/* Delete the icon associated with the given file (NOT the file itself! */
/* (Don't worry if the icon doesn't exist */
void ami_wbench_unlink(base)
char *base;
{
    if(!FromWBench)return;
    if(FromCLI)return;

    strcpy(iconname,base);
    strcat(iconname,".info");
    KillIcon=TRUE;          /* don't do it now - this way the user
			     * gets back whatever picture we had
			     * when we started if the game is
			     * saved again           */
}

static int preserved=0;		/* wizard mode && saved save file */

void
preserve_icon(){
    preserved=1;
}

void
clear_icon(){
    if(!FromWBench)return;
    if(FromCLI)return;
    if(preserved)return;
    if(!KillIcon)return;

    DeleteFile(iconname);
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
	     * checks and just do it            */
    if(mode & O_CREAT)return(open(SAVEF,mode));
    if(FromTool)return(-1);	/* otherwise, by definition, there
				 * isn't a save file (even if a
				 * file of the right name exists) */
    if(savefname[0])
	strncpy(plname,savefname,PL_NSIZ-1); /* restore poly'd name */
    lock=Lock(SAVEF,ACCESS_READ);
    fib = (struct FileInfoBlock *)AllocMem(ALLOC_SIZE, MEMF_CLEAR);
    if(lock && Examine(lock,fib)){
	if(fib->fib_Size>100){  /* random number << save file size */
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
    return(-1);     /* give up */
}

static void
score(scorearg)
	char *scorearg;
        {
	long ac;
	char *p;
	char **av=calloc(1,50*sizeof(char *));

#ifdef CHDIR
	chdirx(hackdir,0);
#endif
	av[0]="NetHack";            /* why not? */
	av[1]="-s";             /* why not? */
	av[2]=0;
	for(ac=2,p=scorearg;*p;ac++){
	    av[ac]=p;av[ac+1]=0;
	    while(*p && !isspace(*p))p++;
	    if(!*p)break;
	    *p++='\0';
	    while(*p && isspace(*p))p++;
	    /* *p='\0';	/* extra? */
	}
	prscore(ac+1,av);
	free( av );
	exit(0);        /* #defined to msexit() */
}
