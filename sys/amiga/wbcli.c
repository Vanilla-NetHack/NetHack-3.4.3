/*    SCCS Id: @(#)wbcli.c  2.1   93/01/08			  */
/*    Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1991	  */
/*    Copyright (c) Gregg Wonderly, Naperville, IL, 1992, 1993    */
/* NetHack may be freely redistributed.  See license for details. */

/* Friendly Intuition interface for NetHack 3.1 on the Amiga */

/*
 * This file contains many routines common to the CLI and WB interfaces,
 * along with a few specific to each.  #defining CLI supports the CLI
 * interface; not #defining it supports (along with wb.c) the WB interface.
 */

#include "NH:include/date.h"		/* this gives us the version string */
#include "NH:include/patchlevel.h"	/* and the individual bits */

const char amiga_version_string[] = AMIGA_VERSION_STRING;

#ifdef AZTEC_C
/* Aztec doesn't recognize __chip syntax */
# define __chip
#endif

#include "NH:sys/amiga/wbdefs.h"       /* Miscellany information */
#ifdef  INTUI_NEW_LOOK
#define NewWindow   ExtNewWindow
#define NewScreen   ExtNewScreen
#endif
#include "NH:sys/amiga/wbstruct.h"
#include "NH:sys/amiga/wbprotos.h"

#ifdef CLI
#include "NH:sys/amiga/wbdata.c"       /* All structures and global data */

#undef NetHackCnf
char NetHackCnf[50]="NetHack:NetHack.cnf";
#endif  /* CLI */
void error( register const char *str );

#define C_GREY  0
#define C_BLACK 1
#define C_WHITE 2
#define C_BLUE  3

#if !defined(__SASC_60) && !defined(_DCC)
extern char *sys_errlist[];
#endif
extern int errno;
extern char scrntitle[ 90 ];

#define SPLIT           /* use splitter, if available */

void diskobj_filter(struct DiskObject *);
BPTR s_LoadSeg(char *);
void s_UnLoadSeg(void);
void append_slash(char *);

/*DCF - GetWBIcon() needs these to be available in both WB and CLI modes */
int amibbs=0;           /* BBS mode flag */
char *bbsuid=NULL;      /* Unique user identifier for bbs mode. */

#ifdef CLI
char *cnfsavedir="NetHack:save";    /* unless overridden in cnf file */
char argline[255];  /* no overflow - bigger than ADOS will pass */

void WaitEOG(GPTR);
char *eos(char *);
void condaddslash(char *);

/*DCF - Copies NewGame.info to new game. */
void CopyGameIcon(char *desticon);

# ifdef SPLIT
int running_split=0;        /* if 0, using normal LoadSeg/UnLoadSeg */
# endif
#else
extern char *options[NUMIDX+1];
extern GPTR gamehead,gameavail;
extern struct Window *win;
#endif  /* CLI */

#ifdef AZTEC_C
extern char *strdup(char *);

/*
 * Aztec has a strnicmp, but it doesn't work properly.
 *
 * Note: this came out of NH:src/hacklib.c
 */
static char
lowc(c)         /* force 'c' into lowercase */
    char c;
{
    return ('A' <= c && c <= 'Z') ? (c | 040) : c;
}

int
strnicmp(s1, s2, n)
    register const char *s1, *s2;
    register int n;
{
    register char t1, t2;

    while (n--) {
	if (!*s2) return (*s1 != 0);    /* s1 >= s2 */
	else if (!*s1) return -1;   /* s1  < s2 */
	t1 = lowc(*s1++);
	t2 = lowc(*s2++);
	if (t1 != t2) return (t1 > t2) ? 1 : -1;
    }
    return 0;               /* s1 == s2 */
}
#endif

#ifndef max
# define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
# define min(x,y) ((x) < (y) ? (x) : (y))
#endif

char *copyright_text[]={
	COPYRIGHT_BANNER_A,
	COPYRIGHT_BANNER_B,
	COPYRIGHT_BANNER_C,
	0
};

#ifdef CLI

main( argc, argv )
    int argc;
    char **argv;
{
    GPTR gptr;
    BPTR lc,lc2;
    struct FileInfoBlock finfo;
    char *name=0;
    char namebuf[50];
    struct WBStartup *wbs = (struct WBStartup *)argv;
    char newcmdline[80]="";
    char forcenewcmd=0;

    /*ZapOptions( );*/
    InitWB( argc, wbs );
    errmsg( NO_FLASH, "Welcome to NetHack Version %d.%d.%d!\n",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );
    CopyRight( );

    ReadConfig( );

    /* check for BBS mode */
    if (argc>1 && argv[1][0]==':'){
	amibbs=1;
	sprintf(newcmdline,":%08x %08x ;%s ",Input(),Output(),&argv[1][1]);

	/* DCF - Set BBS mode user identifier */
	bbsuid = &argv[1][1];
	argv++;argc--;
    }

		/* check/re-assemble initial command line */
    {
    int c;
				/* slow but easy - not a critical path */
    for(c=1;c<argc;c++){
	if(strlen(newcmdline)+strlen(argv[c])>=sizeof(newcmdline)){
	    forcenewcmd=1;
	    break;
	}
	strcpy(eos(newcmdline),argv[c]);
	if(!strncmp(argv[c],"-u",2)){
	    if(!strcmp(argv[c],"-u")){
		name= argv[c+1];
	    }else{
		name= &argv[c][2];
	    }
	    if(!name){
		errmsg(NO_FLASH, "No name found.\n");
		newcmdline[0]='\0';	/* don't leave -u as default */
		forcenewcmd=1;
	    }
	}
	if(c<argc)strcpy(eos(newcmdline)," ");
    }
    eos(newcmdline)[-1]='\0';
    strcpy(argline,newcmdline);
    }

    /* Wait till user quits */
    while( !quit )
    {
	char tbuf[80];
	char *p=tbuf;
	char *dirname=cnfsavedir;
/* play a game */

	if(forcenewcmd)
	    goto build_new_argline;	/* allow initial args to be wrong */
#undef TESTCMDLINE
#ifdef TESTCMDLINE
__builtin_printf("sending '%s'\n",argline);
#else
	/* DCF - If the user provided a name, try to get the icon for this
	 * game.  If there is no icon, try to find the NewGame.info
	 * icon and copy it to match the appropriate name.
	 */
	strcpy(namebuf,cnfsavedir);
	append_slash(namebuf);

	/* DAN - In BBS mode, prepend the bbsuid on the front of the player
	 * name.  This will make the icon match the format used by the
	 * Nethack executable when saving the game.
	 */

	if ((amibbs) && (bbsuid))
	{
	    strcat(namebuf,bbsuid);
	    strcat(namebuf,"_");
	}

	if(!name)
	    strcat(namebuf,"NewGame");
	else
	    strcat(namebuf,name);

	strcat(namebuf,".info");

	lc=Lock(namebuf,ACCESS_READ);

	if (!lc) /* && name) */
	{
	    /* If no icon found, this is probably a new game.  Build a new
	     * icon for the game based on the NewGame.info. */

	    CopyGameIcon(namebuf); /* copies the NewGame.info to namebuf */
	    lc = Lock(namebuf,ACCESS_READ);
	}

	if(!lc){
	    dirname="NetHack:";
	    strcpy(namebuf,dirname);
	    strcat(namebuf,"NewGame.info");
	    lc=Lock(namebuf,ACCESS_READ);
	    if(!lc){
		errmsg(NO_FLASH,"Can't find NewGame.info");
		cleanup(1);
	    }
	}
	if(!Examine(lc,&finfo)){
	    errmsg(NO_FLASH,"Can't find info file.\n");
	    cleanup(1);
	}
	lc2=ParentDir(lc);
	UnLock(lc);
	gptr=GetWBIcon(lc2,dirname,&finfo);
	if(!gptr)cleanup(1);
	UnLock(lc2);
	run_game(gptr);

/* wait for game to end */
	if(gptr->wbs)
	    WaitEOG(gptr);
	/* else load failed */

	FreeGITEM(gptr);
#endif /* TESTCMDLINE */
/* ask about another? */
build_new_argline:
	forcenewcmd=0;
	if(amibbs) {
	    quit = 1;		/* bbs mode aborts after one game */
	} else
	{
	char *x=argline;
	while(isspace(*x))x++;
	if(*x){			/* non-blank argline */
		printf("%s %s %s",
		  "Enter options for next game.  Default:\n\t", argline,
		  "\n(space return to clear) or Q to quit:\n");
	} else {
		printf("Enter options for next game or Q to quit:\n");
	}
	fgets(tbuf,sizeof(tbuf),stdin);
	tbuf[strlen(tbuf)-1]='\0';		/* kill \n */
	if(strlen(tbuf)==1 && (*p=='q' || *p=='Q')){
	    quit=1;
		} else
	    if(strlen(tbuf))strcpy(argline,tbuf);
	}
    }
    cleanup(0);
}

/* CLI */

void
WaitEOG(target)
    GPTR target;
{
    long mask, rmask;
    struct WBStartup *wbs;
    struct WBArg *wba;
    int i;
	/* Get a message */
    while(1){
	mask = ( 1L << dosport->mp_SigBit ) ;
	rmask = Wait( mask );


	if( rmask & ( 1L << dosport->mp_SigBit ) )
	{
	    /* Get process termination messages */

	    while( wbs = (struct WBStartup *) GetMsg( dosport ) )
	    {
		/* Find the game that has terminated */

		if(target->seglist == wbs->sm_Segment)
		{
#ifdef SPLIT
		    if(!running_split)
#endif
		    /* Unload the code */
		    UnLoadSeg( wbs->sm_Segment );
		    /* Free the startup message resources */

		    wba = (struct WBArg *)
			((long)wbs + sizeof( struct WBStartup ));
		    for( i = 0; i < wbs->sm_NumArgs; ++i )
		    {
			FreeMem( wba[i].wa_Name, strlen( wba[i].wa_Name ) + 1 );
			UnLock( wba[i].wa_Lock );
		    }
		    FreeMem( wbs, wbs->sm_Message.mn_Length );
		    wbs = NULL;

		    return;
		}
	    }

	}
    }
}

/* CLI */

void CopyRight()
{
    int line;
    for(line=0;copyright_text[line];line++){
	printf("%s\n",copyright_text[line]);
    }
}

/* CLI */

/*
 * Do the one time initialization things.
 */

void
InitWB( argc, wbs )
    int argc;
    register struct WBStartup *wbs;
{
    char **argv=(char **)wbs;

    /* Open Libraries */
    GfxBase= (struct GfxBase *) OldOpenLibrary("graphics.library");
    IconBase= OldOpenLibrary("icon.library");
    DiskfontBase= (struct DiskfontBase *)OldOpenLibrary("diskfont.library");
    IntuitionBase= (struct IntuitionBase *)OldOpenLibrary("intuition.library");

    if(!GfxBase || !IconBase || !DiskfontBase || !IntuitionBase)
    {
	error("library open failed");
	cleanup( 1 );
    }

    /* Get Port for replied WBStartup messages */

    if( ( dosport = CreatePort( NULL, 0 ) ) == NULL )
    {
	error("failed to create dosport" );
	cleanup( 1 );
    }

    /* If started from CLI */
    if( argc == 0 ){
	printf("Run this program from CLI only.\n");
	DisplayBeep(0);     /* could be more specific */
	Delay(400);
	cleanup(1);
    }
/* we should include hack.h but due to conflicting options to sc
 * we can't parse tradstdc.h and clib/graphics_protos.h - fake it
 */
#define NEWS
#define WIZARD
    if (argc>1 && argv[1][0]=='?'){
	    (void) printf(
"\nUsage:\n %s [:uname] [-d dir] -s [-[%s]] [maxrank] [name]...",
		argv[0], classes);
	    (void) printf("\n or");
	    (void) printf("\n %s [-d dir] [-u name] [-[%s]]",
		argv[0], classes);
	    (void) printf(" [-[DX]]");
#ifdef NEWS
	    (void) printf(" [-n]");
#endif
#ifndef AMIGA
	    (void) printf(" [-I] [-i] [-d]");
#endif
#ifdef MFLOPPY
# ifndef AMIGA
	    (void) printf(" [-r]");
# endif
#endif
#ifdef AMIGA
	    (void) printf(" [-[lL]]");
#endif
	    putchar('\n');
	    cleanup(1);
	}
}

/* CLI */

/*
 * Read a nethack.cnf like file and collect the configuration
 * information from it.
 */
void ReadConfig()
{
    register FILE *fp;
    register char *buf, *t;

    /* Use a dynamic buffer to limit stack use */

    if( ( buf = xmalloc( 1024 ) ) == NULL )
    {
	error( "Can't alloc space to read config file" );
	cleanup( 1 );
    }

    /* If the file is not there, can't load it */

    if( ( fp = fopen( NetHackCnf, "r" ) ) == NULL )
    {
	errmsg( FLASH, "Can't load config file %s", NetHackCnf );
	free( buf );
	return;
    }

    /* Read the lines... */

    while( fgets( buf, 1024, fp ) != NULL )
    {
	if( *buf == '#' )
	    continue;

	if( ( t = strchr( buf, '\n' ) ) != NULL )
	    *t = 0;

	if( strnicmp( buf, "SAVE=", 5 ) == 0 )
	{
	    cnfsavedir=strdup(buf+5);
	}
	else
	{
	    /* We don't care about the rest */
	}
    }
    fclose( fp );
    free( buf );
}

/* CLI */

void
run_game( gptr )
    register GPTR gptr;
{
    struct Task *ctask;
    register struct MsgPort *proc = NULL;
    int tidx;

    tidx = 0;

    gptr->gname = xmalloc( 20 + strlen( gptr->name ) );

    SetToolLine(gptr, "INTERNALCLI", argline);
    gptr->wbs = AllocMem( sizeof( struct WBStartup ) +
	( sizeof( struct WBArg ) * 2 ), MEMF_PUBLIC | MEMF_CLEAR );

    /* Check if we got everything */

    if( !gptr->gname || !gptr->wbs )
    {
	fprintf( stderr, "Can't allocate memory\n" );
	goto freemem;
    }

    /* Get the arguments structure space */

    gptr->wba = ( struct WBArg * ) ((long)gptr->wbs + 
	sizeof( struct WBStartup ) );
    /* Load the game into memory */
#ifdef SPLIT
	/* Which version do we run? */
    {
    char gi[80];
    BPTR tmplock;

    sprintf( gi, "%s.dir", GAMEIMAGE );
    tmplock=Lock( gi, ACCESS_READ );
    if( tmplock ){
	UnLock( tmplock );
	gptr->seglist = (BPTR)s_LoadSeg( gi );
	if(gptr->seglist)running_split=1;
    }else{
	gptr->seglist = (BPTR)LoadSeg( GAMEIMAGE );
    }
    }
#else
    gptr->seglist = (BPTR)LoadSeg( GAMEIMAGE );
#endif

    if( gptr->seglist == NULL)
    {
	errmsg( FLASH, "Can't load %s", GAMEIMAGE );
	goto freemem;
    }

    /* Set the game name for the status command */
    sprintf( gptr->gname, "NetHack %d.%d.%d %s",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, gptr->name );

    /* Create a process for the game to execute in */
    ctask = FindTask( NULL );
    proc = CreateProc( gptr->gname, ctask->tc_Node.ln_Pri,
		    gptr->seglist, GAMESTACK );

    /* Check if the create failed */

    if( proc == NULL )
    {
	fprintf(stderr, "Error creating process %d\n", IoErr() );
#ifdef SPLIT
	if(!running_split)
#endif
	    UnLoadSeg( gptr->seglist );
freemem:
	if( gptr->gname ) free( gptr->gname );
	gptr->gname = NULL;

	if( gptr->wbs ) FreeMem( gptr->wbs,
	    sizeof( struct WBStartup ) + sizeof( struct WBArg ) * 2 );
	gptr->wbs = NULL;
	return;
    }

    /* Get the Process structure pointer */
    gptr->prc = (struct Process *) (((long)proc) - sizeof( struct Task ));

    /* Set the current directory */
    gptr->prc->pr_CurrentDir=((struct Process *)FindTask(NULL))->pr_CurrentDir;

    /* Fill in the startup message */
    gptr->wbs->sm_Process = proc;
    gptr->wbs->sm_Segment = gptr->seglist;
    gptr->wbs->sm_NumArgs = 2;
    {
    static char title[90];	/* some slack */
    sprintf(title,"con:0/0/100/300/NetHack %d.%d.%d console",
      VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
    gptr->wbs->sm_ToolWindow = title;
    }
    gptr->wbs->sm_ArgList = gptr->wba;

    /* Fill in the args */

    gptr->wba[0].wa_Name = Strdup( GAMEIMAGE );
    gptr->wba[0].wa_Lock = Lock( dirname( GAMEIMAGE ), ACCESS_READ );

    gptr->wba[1].wa_Name = Strdup( gptr->name );
    gptr->wba[1].wa_Lock = Lock( "t:", ACCESS_READ );

    /* Write the updated tools types entries into t: so that the icon is not
     * mysteriously updated behind the users back.
     */

    WriteDObj( gptr, gptr->wba[1].wa_Lock );

    /* Set the message fields correctly */

    gptr->wbs->sm_Message.mn_Node.ln_Type = NT_MESSAGE;
    gptr->wbs->sm_Message.mn_Node.ln_Pri = 0;
    gptr->wbs->sm_Message.mn_ReplyPort = dosport;
    gptr->wbs->sm_Message.mn_Length =
	sizeof( struct WBStartup ) + ( sizeof( struct WBArg ) * 2 );

    /* Send the WB Startup message to let the game go... */

    PutMsg( proc, &gptr->wbs->sm_Message );
}

#else   /* CLI */

void RPText( rp, s )
    struct RastPort *rp;
    register char *s;
{
    Text( rp, s, strlen( s ) );
}

/* !CLI */

void
SetUpMenus( mp, scrn )
    register struct Menu *mp;
    register struct Screen *scrn;
{
    register struct MenuItem *mip;
    register int i, leftoff = 0, horiz=0, len;
    register int com, chk;

    for( ; mp; mp = mp->NextMenu )
    {
	i = 0;
	horiz = (scrn->RastPort.TxWidth*strlen(mp->MenuName))+16;
	/*mp->LeftEdge = leftoff;*/
	com = chk = 0;
	for( mip = mp->FirstItem; mip; mip = mip->NextItem )
	{
	    if( mip->Flags & COMMSEQ )
		com = scrn->RastPort.TxWidth * 6;
	    if( mip->Flags & CHECKIT )
		chk = scrn->RastPort.TxWidth * 3;
	}
	for( mip = mp->FirstItem; mip; mip = mip->NextItem )
	{
	    mip->Height = scrn->RastPort.TxHeight;
	    mip->TopEdge = scrn->RastPort.TxHeight * i++;

	    len = IntuiTextLength((struct IntuiText *)
		    (mip->ItemFill))+8+com+chk;
	    horiz = max( horiz,len );
	}

	for( mip = mp->FirstItem; mip; mip = mip->NextItem )
	{
	    mip->Width = horiz;
	}
	leftoff += horiz;
    }
}

/* !CLI */

/* Get a text line from the indicated file based on an array of per
 * line offsets.
 */

void getline( fp, offarr, which, buf, size )
    FILE *fp;
    long *offarr;
    int which, size;
    char *buf;
{
    fseek( fp, offarr[ which ], 0 );
    fgets( buf, size, fp );
}

/* !CLI */

void
do_closewindow()
{
    /*set close flag - cleanup must be done elsewhere*/
    cleanup( 0 );
}

/* !CLI */

void
menu_copyopt()
{
    GPTR gptr;
    char newname[ 100 ], oldname[ 100 ], cmd[ 300 ], dir[ 100 ];

    if( ( gptr = NeedGame() ) == NULL )
	return;

    if( StrRequest( "Enter new player name", newname, gptr->name ) == 0 )
	return;

    if( strcmp( newname, gptr->name ) == 0 )
    {
	errmsg( FLASH, "Copying aborted, new name same as old" );
	return;
    }

    strcpy( oldname, GameName( gptr, NULL ) );

    strcpy( dir, options[ SAVE_IDX ] );
    if( strchr( "/:", dir[strlen(dir)-1] ) == 0 && *dir )
	strcat( dir, "/" );
    if( gptr->dobj->do_Gadget.GadgetID == GADNEWGAME )
	sprintf( cmd, "c:copy \"%s\" \"%s%s.cfg.info\"", oldname, dir, newname );
    else
	sprintf( cmd, "c:copy \"%s\" \"%s%s.info\"", oldname, dir, newname );
    Execute( cmd, NULL, NULL );
    MapGadgets( R_DISK, 1 );
}

/* !CLI */

void
menu_rename()
{
    register GPTR gptr;
    char newname[ 100 ], oldname[ 100 ], cmd[ 200 ], name[100], *t;

    if( ( gptr = NeedGame() ) == NULL )
	return;

    strcpy( newname, gptr->name );
    if( t = strrchr( newname, '.' ) )
    {
	if( strcmp( t, ".sav" ) == 0 )
	    *t = 0;
    }

    if( StrRequest( "Enter New Name For Game", name, newname ) == 0)
	return;

    /* Name can only be this long to allow inclusion of appropriate suffix */
    name[ 30 - strlen( ".sav.info " ) ] = '\0';

    if( strcmp( name, newname ) == 0 )
    {
	errmsg( FLASH, "Rename aborted, name unchanged from %s", newname );
	return;
    }

    strcat( name, ".sav" );

    strcpy( oldname, GameName( gptr, NULL ) );

    strcpy( newname, GameName( gptr, name ) );
    strcat( newname, ".info" );

    /* Rename icon file */
    sprintf( cmd, "c:rename \"%s\" \"%s\"", oldname, newname );
    Execute( cmd, NULL, NULL );

    strcpy( oldname, GameName( gptr, gptr->name ) );

    strcpy( newname, GameName( gptr, name ) );

    /* Rename save file if it is actually there */
    if( access( oldname, 0 ) == 0 )
    {
	sprintf( cmd, "c:rename \"%s\" \"%s\"", oldname, newname );
	Execute( cmd, NULL, NULL );
    }

    MapGadgets( R_DISK, 1 );
}

#endif  /* CLI */

void CleanUpLists( )
{
    register GPTR gptr;

    while( gptr = gamehead )
    {
	gamehead = gamehead->next;
	FreeGITEM( gptr );
    }

    while( gptr = gameavail )
    {
	gameavail = gameavail->next;
	free( gptr );
    }
}

#ifndef CLI

void SafeCloseWindow( window )
    register struct Window *window;
{
    register struct Message *msg;

    if( !window )
	return;

    /* Remove any attached menu */

    if( window->MenuStrip )
    {
	ClearMenuStrip( window );
    }

    Forbid();
    while( window->UserPort != NULL &&
	    ( msg = GetMsg( window->UserPort) ) != NULL )
    {
	ReplyMsg( msg );
    }

    CloseWindow( window );
    Permit();
}

void RemoveGITEM( ggptr )
    register GPTR ggptr;
{
    register GPTR gptr, pgptr = NULL;

    for( gptr = gamehead; gptr; pgptr = gptr, gptr = gptr->next )
    {
	if( gptr == ggptr )
	{
	    if( pgptr )
		pgptr->next = gptr->next;
	    else
		gamehead = gptr->next;
	    FreeGITEM( gptr );
	    return;
	}
    }
}


#else   /* CLI */

void CloseLibraries( )
{
    if( IntuitionBase )     CloseLibrary( (void *) IntuitionBase );
    IntuitionBase = 0;
    if( DiskfontBase )      CloseLibrary( (void *) DiskfontBase );
    DiskfontBase = 0;
    if( IconBase )          CloseLibrary(  IconBase );
    IconBase = 0;
    if( GfxBase )           CloseLibrary( (void *) GfxBase );
    GfxBase = 0;
}

/* CLI */

void cleanup( code )
    int code;
{
    if( dosport ) DeletePort( dosport );
    dosport = NULL;

    CleanUpLists( );
    CloseLibraries( );

#ifdef SPLIT
    if(running_split){
	s_UnLoadSeg();
    }
#endif
    exit( code );
}

/* CLI */

GPTR AllocGITEM( )
{
    register GPTR gptr;

    if( gameavail )
    {
	gptr = gameavail;
	gameavail = gameavail->next;
    }
    else
    {
	gptr = xmalloc( sizeof( GAMEITEM ) );
    }

    if( gptr )
	memset( gptr, 0, sizeof( GAMEITEM ) );

    return( gptr );
}

/* CLI */

void FreeGITEM( gptr )
    register GPTR gptr;
{
    /* Free all of the pieces first */

    if( gptr->talloc )
	FreeTools( gptr );
    gptr->talloc = 0;

    if( gptr->dobj )
	FreeDObj( gptr->dobj );
    gptr->dobj = NULL;

    if( gptr->name )
	free( gptr->name );
    gptr->name = NULL;

    if( gptr->dname )
	free( gptr->dname );
    gptr->dname = NULL;

    if( gptr->fname )
	free( gptr->fname );
    gptr->fname = NULL;

    /* Connect it to free list */

    gptr->next = gameavail;
    gameavail = gptr;
}

/* CLI */

struct DiskObject *AllocDObj( str )
    register char *str;
{
    register struct DiskObject *doptr;
    register char *t, *t1;

    if( ( t = strrchr( str, '.' ) ) && stricmp( t, ".info" ) == 0 )
    {
	*t = 0;
    } else {
	t = NULL;
    }

    if( doptr = GetDiskObject( str ) )
    {
	struct IntuiText *ip;

	diskobj_filter(doptr);  /* delete all but INTERNALCLI */

	if( ip = xmalloc( sizeof( struct IntuiText ) ) )
	{
	    memset( ip, 0, sizeof( struct IntuiText ) );
	    ip->FrontPen = C_BLACK;
	    ip->DrawMode = JAM1;
	    ip->IText = strdup( str );
	    doptr->do_Gadget.GadgetText = ip;

		/* Trim any .sav off of the end. */

	    if( ( t1 = strrchr( ip->IText, '.' ) ) &&
		stricmp( t1, ".sav" ) == 0 )
	    {
		*t1 = 0;
		ip->LeftEdge = (2 * win->RPort->TxWidth);
	    }
        }
    }
    if( t ) *t = '.';

    return( doptr );
}

#endif  /* CLI */

void FreeDObj( doptr )
    register struct DiskObject *doptr;
{
    if( doptr->do_Gadget.GadgetText )
    {
	free( doptr->do_Gadget.GadgetText->IText );
	free( doptr->do_Gadget.GadgetText );
    }
    doptr->do_Gadget.GadgetText = NULL;
    FreeDiskObject( doptr );
}

#ifdef CLI
#ifdef AZTEC_C
void errmsg(int flash, char *str, ...)
#else
void errmsg( flash, str )
char *str;
int flash;
#endif
{
    va_list vp;

    va_start( vp, str );

/*  if( !win || !wbopen ) */
    {
	vprintf( str, vp );
	va_end( vp );
	printf("\n");
	return;
    }

}

/* CLI */

/*
 * Issue an error message to the users window because it can not be done
 * any other way.
 */

void error( str )
    register const char *str;
{
    char s[ 50 ];
    if( scrn ) ScreenToBack( scrn );
    Delay( 10 );
    fprintf( stderr, "%s\n", str );
    fprintf( stderr, "Hit Return: " );
    fflush( stderr );
    gets( s );
    if( scrn ) ScreenToFront( scrn );
}

#else   /* CLI */

void menu_scores()
{
    register char buf1[50];
    register char **oldtools;
    register GPTR gptr;
    int oldalloc;
    extern GPTR windowgads;

    if( StrRequest( "Scores for whom?", buf1, "all" ) != 0 )
    {
	for( gptr = windowgads; gptr; gptr = gptr->nextwgad )
	{
	    if( gptr->dobj->do_Gadget.GadgetID == GADNEWGAME )
		break;
	}

	if( !gptr )
	{
	    errmsg( FLASH, "Can't find NewGame icon" );
	    return;
	}

	/* Save current tools */
	oldtools = gptr->dobj->do_ToolTypes;

	/* Force a new tooltypes array to be allocated */
	if( oldalloc = gptr->talloc )
	{
	    gptr->dobj->do_ToolTypes = gptr->otools;
	    gptr->talloc = 0;
	}

	/* Add the scores entry */
	SetToolLine( gptr, "SCORES", *buf1 ? buf1 : "all" );

	/* Get the scores */
	run_game( gptr );

	/* Free the tools which contain "SCORES=" */
	FreeTools( gptr );

	/* Restore the old tools.  When this game exits, the tools
	 * will be written back out to disk to update things
	 */
	gptr->dobj->do_ToolTypes = oldtools;
	gptr->talloc = oldalloc;
	Delay( 100 );
	UpdateGameIcon( gptr );
    }
}

/* !CLI */

CheckAndCopy( gadstr, origstr )
    char *gadstr, *origstr;
{
    char *t;
    int i;

    if( t = strchr( gadstr, '=' ) )
    {
	i = t - gadstr;
	/* Check for original string and don't allow one line to be replaced with
	 * another.
	 */
	if( *origstr != 0 && strncmp( gadstr, origstr, i ) != 0 )
	{
	    strcpy( gadstr, origstr );
	    DisplayBeep( NULL );
	    return( 0 );
	}
    }
    else
    {
	/* If added an equals, there wasn't one previously, so signal an error */
	if( t = strchr( origstr, '=' ) )
	{
	    strcpy( gadstr, origstr );
	    DisplayBeep( NULL );
	    return( 0 );
	}
    }
    return( 1 );
}

/* !CLI */

int IsEditEntry( str, gptr )
    char *str;
    register GPTR gptr;
{
    if( strncmp( str, "CHARACTER=", 10 ) == 0 )
	return( 0 );
    return( 1 );
}

/* !CLI */

void menu_comment( )
{
    register GPTR gptr;
    struct FileInfoBlock *finfo;
    BPTR lock;
    char commentstr[ 100 ];

    if( ( gptr = NeedGame() ) == NULL )
	return;

    if( ( lock = Lock( GameName( gptr, NULL ), ACCESS_READ ) ) == NULL )
    {
	/* Can't get lock, reload and return */

	errmsg( FLASH, "Can't Lock game save file: %s",
		    GameName( gptr, NULL ) );
	MapGadgets( R_DISK, 1 );
	return;
    }

    finfo = (struct FileInfoBlock *) xmalloc(sizeof(struct FileInfoBlock));
    Examine( lock, finfo );
    UnLock( lock );
    strncpy( commentstr, finfo->fib_Comment, sizeof( finfo->fib_Comment ) );
    commentstr[ sizeof( finfo->fib_Comment ) ] = 0;
    free( finfo );

    /* Set the correct size */
    if( StrRequest( "Edit Comment as Desired",
		commentstr, commentstr ) == 0 )
    {
	return;
    }

    SetComment( GameName( gptr, NULL ), commentstr );
}

/* !CLI */

/*
 * Make the proportional gadget position match the values passed
 */

void UpdatePropGad( win, gad, vis, total, top )
    struct Window *win;
    struct Gadget *gad;
    register long vis, total, top;
{
    register long hidden;
    register int body, pot;

    hidden = max( total-vis, 0 );

    if( top > hidden )
	top = hidden;

    if( hidden > 0 )
	body = (ULONG) (vis * MAXBODY) / total;
    else
	body = MAXBODY;

    if( hidden > 0 )
	pot = (top * MAXPOT) / hidden;
    else
	pot = 0;

    NewModifyProp( gad, win, NULL,
		AUTOKNOB|FREEHORIZ, pot, 0, body, MAXBODY, 1 );
}

#endif  /* CLI */

/*
 * Allocate some memory
 */

void *xmalloc( nbytes )
    unsigned nbytes;
{
    return( malloc( nbytes ) );
}

#ifndef CLI
/*
 * Delete the game associated with the GAME structure passed
 */

int DeleteGame( gptr )
    register GPTR gptr;
{
    register int err;

    err = DeleteFile( GameName( gptr, gptr->name ) );
    err += DeleteFile( GameName( gptr, NULL ) );
    return( err );
}
#endif  /* CLI */

/*
 * Look through the list of games for one named 'name'
 */

GPTR FindGame( name )
    char *name;
{
    register GPTR gptr;

    for( gptr = gamehead; gptr; gptr = gptr->next )
    {
	if( stricmp( gptr->fname, name ) == 0 )
	    break;
    }

    return( gptr );
}
/*
 * Set the option string indicated by idx to 'str'
 */

void setoneopt( idx, str )
    int idx;
    char *str;
{
    /* This space accumulates, but is recovered at process exit */

    options[ idx ] = strdup( str );
}

/*
 * Get just the directory name of str
 */

char *dirname( str )
    char *str;
{
    static char buf[ 300 ];
    char *t;

    strncpy( buf, str, sizeof( buf ) );
    buf[ sizeof( buf ) - 1 ] = 0;

    if( (t = strrchr( buf, '/' ) ) == NULL ||
		(t = strrchr( buf, ':' ) ) == NULL )
    {
	return( "/" );
    }
    *t = 0;
    return( buf );
}

#ifndef CLI

/*
 * Make sure that only itemno is checked in 'menu' off of menuptr
 */

void CheckOnly( menuptr, menu, itemno )
    register struct Menu *menuptr;
    register int menu, itemno;
{
    register struct MenuItem *ip;

    while( menuptr && menu-- )
	menuptr = menuptr->NextMenu;

    if( menuptr )
    {
	for( ip = menuptr->FirstItem; ip && itemno; itemno--)
	{
	    ip->Flags &= ~CHECKED;
	    ip = ip->NextItem;
	}

	if( ip )
	{
	    ip->Flags |= CHECKED;
	    ip = ip->NextItem;
	}

	while( ip )
	{
	    ip->Flags &= ~CHECKED;
	    ip = ip->NextItem;
	}
    }
}

/* !CLI */

int FindChecked( menuptr, menu )
    register struct Menu *menuptr;
    register int menu;
{
    register int itemno;
    register struct MenuItem *ip;

    while( menuptr && menu-- )
	menuptr = menuptr->NextMenu;

    if( menuptr )
    {
	for( itemno = 0, ip = menuptr->FirstItem; ip; ip = ip->NextItem )
	{
	    if( ip->Flags & CHECKED )
		return( itemno );
	    ++itemno;
	}
    }
    return( 0 );
}

/* !CLI */

/*
 * Create a file name based in the GAMEs directory.  If file is NULL,
 * the file name is the icon file.  Otherwise it is 'file'.
 */

char *GameName( gptr, file )
    GPTR gptr;
    char *file;
{
    static char buf[200];

    if( file == NULL )
	file = gptr->fname;

    if( strchr( "/:", gptr->dname[ strlen( gptr->dname ) - 1 ] ) )
	sprintf( buf, "%s%s", gptr->dname, file );
    else
	sprintf( buf, "%s/%s", gptr->dname, file );
    return( buf );
}

#endif  /* CLI */

/*
 * Allocate a new GAME structure for the file passed and fill it in
 */

GPTR GetWBIcon( lock, dir, finfo )
    register BPTR lock;
    register char *dir;
    register struct FileInfoBlock *finfo;
{
    register BPTR odir;
    register char *t;
    register GPTR gptr;

    /* DCF */
    char *bbsptr=NULL;

    if( ( gptr = AllocGITEM( ) ) == NULL )
	goto noitems;

    if( ( gptr->dname = strdup( dir ) ) == NULL )
	goto outofmem;

    if( ( gptr->fname = strdup( finfo->fib_FileName ) ) == NULL )
	goto outofmem;

    /* Strip the .info off. */
    if( t = strrchr( finfo->fib_FileName, '.' ) )
    {
	if( stricmp( t, ".info" ) == 0 )
	    *t = 0;
	else
	    t = NULL;
    }

    gptr->name = xmalloc(strlen(finfo->fib_FileName)+1+9);

    /* DCF - This is wrong:
     * sprintf(gptr->name,"%s_%08x",finfo->fib_FileName,FindTask(0));
     *
     * We don't want to append the taskID, we want to append the
     * unique user identifier passed to the cmd line in BBS mode.
     * if one is not available, (i.e. we are not in BBS mode),
     * then only use the player name (or "NewGame" if not given)
     * with no additions.
     */

    if(amibbs)
    {
	/* BBS names are of the form <bbsuid>_<playerName> */
	/* e.g.: SYSOP_SuperHacker */

	bbsptr = strstr(finfo->fib_FileName,"_");
	++bbsptr;
	strcpy(gptr->name,bbsptr);
    }
    else
	strcpy(gptr->name,finfo->fib_FileName);

    /* If removed .info, put it back */

    if( t )
	*t = '.';

    /* Change to saved game directory */

    odir = CurrentDir( lock );

    /* Allocate a diskobj structure */

    if( ( gptr->dobj = AllocDObj( finfo->fib_FileName ) ) == NULL )
    {
	(void) CurrentDir( odir );
outofmem:
	FreeGITEM( gptr );
noitems:
	errmsg( FLASH, "Can't get Disk Object: %s", finfo->fib_FileName );
	return( NULL );
    }
    gptr->oflag = gptr->dobj->do_Gadget.Flags;
    gptr->oact = gptr->dobj->do_Gadget.Activation;
    gptr->dobj->do_Gadget.Activation |=
	    ( RELVERIFY | GADGIMMEDIATE | FOLLOWMOUSE );
    gptr->dobj->do_Gadget.Flags &= ~(GADGHIGHBITS);
    gptr->dobj->do_Gadget.Flags |= GADGHNONE;

    /* Make sure gptr->dobj->do_ToolTypes is not NULL */
    ReallocTools( gptr, 0 );

    (void) CurrentDir( odir );
    return( gptr );
}

#ifndef CLI

/*
 * Put a 3-D motif border around the gadget.  String gadgets or those
 * which do not have highlighting are rendered down.  Boolean gadgets
 * are rendered in the up position by default.
 */

void
SetBorder( gd, val )
    register struct Gadget *gd;
    int val;
{
    register struct Border *bp;
    register short *sp;
    register int i;
    int borders = 6;

    /* Allocate two border structures one for up image and one for down
     * image, plus vector arrays for the border lines.
     */

    if( val == 0 || val == 2 ||
	gd->GadgetType == STRGADGET || ( gd->GadgetType == BOOLGADGET &&
		( gd->Flags & GADGHIGHBITS ) == GADGHNONE ) )
    {
	borders = 12;
    }

    if( ( bp = xmalloc( ( ( sizeof( struct Border ) * 2 ) +
	    ( sizeof( short ) * borders ) ) * 2 ) ) == NULL )
    {
	return;
    }

    /* Remove any special rendering flags to avoid confusing intuition
     */

    gd->Flags &= ~(GADGHIGHBITS|GADGIMAGE);
		/*|(GRELWIDTH|GRELHEIGHT|GRELRIGHT|GRELBOTTOM);*/

    sp = (short *)(bp + 4);
    if( val == 0 || val == 2 ||
	gd->GadgetType == STRGADGET || ( gd->GadgetType == BOOLGADGET &&
		( gd->Flags & GADGHIGHBITS ) == GADGHNONE ) )
    {
	/* For a string gadget, we expand the border beyond the area where
	 * the text will be entered.
	 */

	sp[0] = -1;
	sp[1] = gd->Height - 1;
	sp[2] = -1;
	sp[3] = -1;
	sp[4] = gd->Width-1;
	sp[5] = -1;

	sp[6] = gd->Width + 1;
	sp[7] = -2;
	sp[8] = gd->Width + 1;
	sp[9] = gd->Height + 1;
	sp[10] = -2;
	sp[11] = gd->Height + 1;

	sp[12] = -2;
	sp[13] = gd->Height;
	sp[14] = -2;
	sp[15] = -2;
	sp[16] = gd->Width;
	sp[17] = -2;
	sp[18] = gd->Width;
	sp[19] = gd->Height;
	sp[20] = -2;
	sp[21] = gd->Height;

	for( i = 0; i < 3; ++i )
	{
	    bp[ i ].LeftEdge = bp[ i ].TopEdge = -1;
	    if( val == 2 )
		bp[ i ].FrontPen = ( i == 0 || i == 1 ) ? C_WHITE : C_BLACK;
	    else
		bp[ i ].FrontPen = ( i == 0 || i == 1 ) ? C_BLACK : C_WHITE;

	    /* Have to use JAM2 so that the old colors disappear. */
	    bp[ i ].BackPen = C_GREY;
	    bp[ i ].DrawMode = JAM2;
	    bp[ i ].Count = ( i == 0 || i == 1 ) ? 3 : 5;
	    bp[ i ].XY = &sp[ i*6 ];
	    bp[ i ].NextBorder = ( i == 2 ) ? NULL : &bp[ i + 1 ];
	}

	/* Set the up image */
	gd->GadgetRender = (APTR) bp;

	/* Same image for select image */
	gd->SelectRender = (APTR) bp;

	if( gd->Flags & GRELRIGHT )
	    gd->LeftEdge--;
	else
	    gd->LeftEdge++;
	if( gd->Flags & GRELBOTTOM )
	    gd->TopEdge--;
	else
	    gd->TopEdge++;
	gd->Flags |= GADGHCOMP;
    }
    else
    {
	/* Create the border vector values for up and left side, and
	 * also the lower and right side.
	 */

	sp[0] = 0;
	sp[1] = gd->Height;
	sp[2] = 0;
	sp[3] = 0;
	sp[4] = gd->Width;
	sp[5] = 0;

	sp[6] = gd->Width;
	sp[7] = 0;
	sp[8] = gd->Width;
	sp[9] = gd->Height;
	sp[10] = 0;
	sp[11] = gd->Height;

	/* We are creating 4 sets of borders, the two sides of the
	 * rectangle share the border vectors with the opposite image,
	 * but specify different colors.
	 */

	for( i = 0; i < 4; ++i )
	{
	    bp[ i ].TopEdge = bp[ i ].LeftEdge = 0;

	    /* A GADGHNONE is always down */

	    if( val != 3 && gd->GadgetType == BOOLGADGET &&
		( gd->Flags & GADGHIGHBITS ) != GADGHNONE )
	    {
		bp[ i ].FrontPen =
		    ( i == 1 || i == 2 ) ? C_BLACK : C_WHITE;
	    }
	    else
	    {
		bp[ i ].FrontPen =
		    ( i == 1 || i == 3 ) ? C_WHITE : C_BLACK;
	    }

	    /* Have to use JAM2 so that the old colors disappear. */
	    bp[ i ].BackPen = C_GREY;
	    bp[ i ].DrawMode = JAM2;
	    bp[ i ].Count = 3;
	    bp[ i ].XY = &sp[ 6 * ((i &1) != 0) ];
	    bp[ i ].NextBorder =
		( i == 1 || i == 3 ) ? NULL : &bp[ i + 1 ];
	}

	/* bp[0] and bp[1] two pieces for the up image */
	gd->GadgetRender = (APTR) bp;

	/* bp[2] and bp[3] two pieces for the down image */
	gd->SelectRender = (APTR) (bp + 2);
	gd->Flags |= GADGHIMAGE;
    }
}

/* !CLI */

struct Gadget *FindGadget( window, newwindow, id )
    struct Window *window;
    struct NewWindow *newwindow;
    int id;
{
    struct Gadget *gd = NULL;

    if( window )
	gd = window->FirstGadget;
    else if( newwindow )
	gd = newwindow->FirstGadget;

    while( gd )
    {
	if( gd->GadgetID == id )
	    break;
	gd = gd->NextGadget;
    }

    return( gd );
}

#endif  /* CLI */

#ifndef CLI

/*
 * Copy Options from GAMES OPTIONS= tooltypes element to the gadgets
 */
void CopyOptions( gptr )
    GPTR gptr;
{
    char **sp;

    for( sp = gptr->dobj->do_ToolTypes; sp && *sp; ++sp )
    {
    	if( strnicmp( *sp, "OPTIONS=", 8 ) == 0 )
	    ParseOptionStr( (*sp) + 8 );
    }
}

/* !CLI */

void
UpdateGameIcon( gptr )
    register GPTR gptr;
{
    register long lock;

    if( lock = Lock( gptr->dname, ACCESS_READ ) )
    {
	/* Write out the DiskObject */

	WriteDObj( gptr, lock );
	UnLock( lock );
    }
    else
    {
	errmsg( FLASH, "Can't change directory to %s", gptr->dname );
    }
}

#endif  /* CLI */

char *ToolsEntry( gptr, name )
    GPTR gptr;
    char *name;
{
    char *str;

    if( ! ( str = FindToolType( (char **) gptr->dobj->do_ToolTypes, name ) ) )
	str = "";

    return( str );
}

/* Reallocate the toolstype information into dynamic memory so that some
 * parts of it can be easily changed, but we can still do "FreeDiskObject"
 * later to clean up whatever "GetDiskObject" allocated.
 */
void ReallocTools( gptr, add )
    register GPTR gptr;
    register int add;
{
    register int i, cnt;
    register char **sp, **tp;

    /* Already allocated */
    if( gptr->talloc && add == 0 )
    	return;

    for( cnt = 0, tp = gptr->dobj->do_ToolTypes; tp && *tp ; ++tp )
	++cnt;

    if( !tp || cnt == 0 )
    {
	if( gptr->talloc )
	    free( gptr->dobj->do_ToolTypes );
	/* If no tooltypes array, fudge something to start with */
	if( sp = xmalloc( 2 * sizeof( char * ) ) )
	{
	    sp[0] = strdup("HACKDIR=NetHack:");
	    sp[1] = NULL;
	}
    }
    else if( sp = xmalloc( (cnt+1+add) * sizeof( char * ) ) )
    {
	for( i = 0, tp = gptr->dobj->do_ToolTypes;
		    tp && *tp && i < cnt; ++tp )
	{
	    sp[i++] = strdup( *tp );
	}

	if( gptr->talloc && gptr->dobj->do_ToolTypes )
	    free( gptr->dobj->do_ToolTypes );
	while( i < cnt+add+1 )
	    sp[ i++ ] = NULL;
    }
    if( ! gptr->talloc )
	gptr->otools = gptr->dobj->do_ToolTypes;
    gptr->dobj->do_ToolTypes = sp;
    gptr->toolcnt = cnt + 1;
    gptr->talloc = 1;
}

void FreeTools( gptr )
    register GPTR gptr;
{
    register int i;
    register char **sp;

    if( !gptr->talloc )
	return;

    for( i = 0, sp = gptr->dobj->do_ToolTypes; sp[i]; ++i )
    {
	free( sp[ i ] );
	sp[ i ] = NULL;
    }
    free( sp );
    gptr->dobj->do_ToolTypes = gptr->otools;
    gptr->talloc = 0;
}

void DelToolLines( gptr, name )
    GPTR gptr;
    char *name;
{
    char **sp;
    int i, j, len;

    sp = gptr->dobj->do_ToolTypes;
    len = strlen( name );

    /* Find any previous definitions and delete them */
    for( i = 0; sp[i] && i < gptr->toolcnt - 1; )
    {
	if( strnicmp( name, sp[i], len ) == 0 && sp[i][len] == '=' )
	{
	    for( j = i; j < gptr->toolcnt && (sp[ j ] = sp[ j + 1 ]); ++j )
	    	continue;
	}
	else
	{
	    ++i;
	}
    }
}

/* Add a tooltypes line that might be a duplicate of the existing ones. */
void AddToolLine( gptr, name, value )
    GPTR gptr;
    char *name, *value;
{
    char **sp;
    int i;

    /* Realloc ToolTypes to be in memory we know how to manage */
    ReallocTools( gptr, 1 );

    sp = gptr->dobj->do_ToolTypes;
    for( i = 0; sp[ i ] && i < gptr->toolcnt - 1; ++i )
    	continue;

    /* Allocate the space needed */
    if( value == NULL )
	sp[ i ] = xmalloc( strlen( name ) + 1 );
    else
	sp[ i ] = xmalloc( strlen (value) + strlen( name ) + 2 );

    /* Set the string */
    if( sp[ i ] != NULL )
	sprintf( sp[ i ], value ? "%s=%s" : "%s", name, value );
    else
	errmsg( FLASH, "Could not allocate string for value" );
}

void SetToolLine( gptr, name, value )
    GPTR gptr;
    char *name, *value;
{
    char **sp, **osp;
    int i, len;

    /* Realloc ToolTypes to be in memory we know how to manage */
    ReallocTools( gptr, 0 );

    sp = gptr->dobj->do_ToolTypes;
    len = strlen( name );

    /* Find any previous definition */
    for( i = 0; sp[i] && i < gptr->toolcnt - 1; ++i )
    {
	if( strnicmp( name, sp[i], len ) == 0 && sp[i][len] == '=' )
	    break;
    }

    /* Free up the space, or allocate new space if not there */
    if( sp[ i ] )
	free( sp[ i ] );
    else
    {
	/* Check for need to realloc */

	if( i >= gptr->toolcnt - 1 )
	{
	    int j=i;
	    osp = sp;
	    sp = xmalloc( ( i + 2 ) * sizeof( char * ) );
	    gptr->toolcnt = i + 2;
	    sp[ i + 1 ] = NULL;
	    while( j >= 0 )
	    {
		sp[ j ] = osp[ j ];
		--j;
	    }
	    free( osp );
	    /* i = gptr->toolcnt - 1; */
	    gptr->dobj->do_ToolTypes = sp;
	}
	else
	{
	    sp[ i + 1 ] = NULL;
	}
    }

    /* Allocate the space needed */
    if( value == NULL )
	sp[i] = xmalloc( strlen( name ) + 1 );
    else
	sp[ i ] = xmalloc( strlen (value) + strlen( name ) + 2 );

    /* Set the string */
    if( sp[ i ] != NULL )
	sprintf( sp[i], value ? "%s=%s" : "%s", name, value );
    else
	errmsg( FLASH, "Could not allocate string for value" );
}

void WriteDObj( gptr, lock )
    register GPTR gptr;
    long lock;
{
    register long odir;
    long flag, act;

    /* Don't write gadget out as selected */

    flag = gptr->dobj->do_Gadget.Flags;
    act = gptr->dobj->do_Gadget.Activation;
    gptr->dobj->do_Gadget.Flags = gptr->oflag;
    gptr->dobj->do_Gadget.Activation = gptr->oact;
    odir = CurrentDir( lock );

    if( PutDiskObject( gptr->name, gptr->dobj ) == 0 )
	errmsg( FLASH, "Could not write disk object values" );

    gptr->dobj->do_Gadget.Flags = flag;
    gptr->dobj->do_Gadget.Activation = act;

    if( odir )
	(void) CurrentDir( odir );
}

char *Strdup( str )
    char *str;
{
    char *t;

    if( t = AllocMem( strlen( str ) + 1, MEMF_PUBLIC ) )
	strcpy( t, str );
    return( t );
}

#ifdef CLI

char *
eos(s)
    char *s;
{
    while(*s)s++;
    return s;
}


/*
 * Add a slash to any name not ending in / or :.  There must
 * be room for the /.
 * NB: Duplicated from amidos.c
 */
void
append_slash(name)
char *name;
{
    char *ptr;

    if (!*name)return;

    ptr = eos(name) - 1;
    if (*ptr != '/' && *ptr != ':') {
	*++ptr = '/';
	*++ptr = '\0';
    }
}

#if 0
/* for debug only */
#define BP __builtin_printf
dumptools(sp,i)
    char **sp;
    int i;
{
    int x;
    BP("Dumptools: cnt=%d\n",i);
    for(x=0;sp[x];x++)
	BP("%d: '%s'\n",x,sp[x]);
}
#endif

#else   /* CLI */

void ClearDelGames()
{
    register GPTR gptr, pgptr = NULL;

    for( gptr = gamehead; gptr; )
    {
	/* Skip New Game */

	if( gptr->fname == NULL )
	{
	    gptr = gptr->next;
	    continue;
	}

	/* If gone, then remove structures */

	if( access( GameName( gptr, NULL ), 0 ) == -1 )
	{
	    if( pgptr )
		pgptr->next = gptr->next;
	    else
		gamehead = gptr->next;

	    FreeGITEM( gptr );
	    gptr = pgptr ? pgptr : gamehead;
	}
	else
	{
	    pgptr = gptr;
	    gptr = gptr->next;
	}
    }
}

/* !CLI */

struct TagItem tags[] =
{
    {WA_ScreenTitle, (ULONG) scrntitle},
    {TAG_DONE, 0l },
};

struct Window *
MyOpenWindow( nw )
#ifdef  INTUI_NEW_LOOK
    struct ExtNewWindow *nw;
#else
    struct NewWindow *nw;
#endif
{
#ifdef  INTUI_NEW_LOOK
    /*nw->Extension = tags;
    nw->Flags = WFLG_NW_EXTENDED;*/
#endif
#undef  NewWindow
    return( OpenWindow( (struct NewWindow *) nw ) );
}

#endif  /* CLI */

void
diskobj_filter( dobj )
    struct DiskObject *dobj;
{
    char **ta=dobj->do_ToolTypes;
    int x;

    /* if nothing there, just return. */
    if( !ta )
    	return;

#ifdef CLI
	/* kill everything except INTERNALCLI */

    for(x=0;ta[x];x++){
	if(!strncmp(ta[x],"INTERNALCLI=",12)){
	    ta[0]=ta[x];
	    ta[1]=0;
	    return;
	}
    }
    ta[0]=0;
#else
	/* kill INTERNALCLI */
    for(x=0;ta[x];x++){
        int offset=0;
	while(ta[x+offset] && !strncmp(ta[x+offset],"INTERNALCLI=",12)){
	    offset++;
        }
        ta[x]=ta[x+offset];
    }
#endif
}

/* DCF - This copies the NewGame.info file to the specified filename.
 *       Used to make an icon for a new game.
 */

#ifdef CLI
void CopyGameIcon(char *desticon)
{
    BPTR in,out;
    char *filen=NULL;
    struct FileInfoBlock *fib=NULL;
    UBYTE *buf=NULL;

    in = Open("Nethack:NewGame.info", MODE_OLDFILE);
    if (!in)
    {
	filen = xmalloc(strlen(cnfsavedir)+15);
	if (filen)
	{
	    strcat(filen,"/NewGame.info");
	    /* Try the save dir. */
	    in = Open(filen,MODE_OLDFILE);
	    free(filen);
	    if (!in)
	    {
		return; /* failed.  Oh well. */
	    }
	}
	else
	    return; /* No memory */
    }

    out = Open(desticon,MODE_NEWFILE);
    if (!out)
    {
	/* Should print error: can't copy icon */
	Close(in);
	in = NULL;
	return;
    }
    else
    {
	/* Copy the file. */
	fib = (struct FileInfoBlock *) AllocDosObject(DOS_FIB,TAG_DONE);
	ExamineFH(in,fib);
	buf = xmalloc (fib->fib_Size);
	FRead(in,buf,fib->fib_Size,1);
	Close(in);
	FWrite(out,buf,fib->fib_Size,1);
	free(buf);
	Close(out);
	FreeDosObject(DOS_FIB,fib);
    }
}

#endif /* CLI */


