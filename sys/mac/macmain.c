/*	SCCS Id: @(#)macmain.c	3.1	94/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - Mac NetHack */

#include "hack.h"
#include "macwin.h"

#include <OSUtils.h>
#include <files.h>
#include <Types.h>
#ifdef MAC_MPW32
#include <String.h>
#include <Strings.h>
#endif
#include <Dialogs.h>
#include <Packages.h>
#include <ToolUtils.h>
#include <Resources.h>
#ifdef applec
#include <SysEqu.h>
#endif
#include <Errors.h>

#ifndef O_RDONLY
#include <fcntl.h>
#endif


int NDECL(main);

#ifdef MAC68K
static void FDECL(IsResident,(void *));
#endif
static void NDECL(process_options);
static void NDECL(whoami);


int
main ( void )
{
	register int fd;
	int argc = 1;

	windowprocs = mac_procs ;
	InitMac ( ) ;

	hname = "Mac Hack" ;
	hackpid = getpid();

	initoptions();
	init_nhwindows(&argc, (char **)&hname);
	whoami();

	/*
	 * It seems you really want to play.
	 */
	setrandom();
	u.uhp = 1;	/* prevent RIP on early quits */

	process_options ( ) ;	/* emulate command line options */
	finder_file_request ( ) ;

#ifdef WIZARD
	if (wizard)
		Strcpy(plname, "wizard");
	else
#endif
	if(!*plname || !strncmp(plname, "player", 4)
		    || !strncmp(plname, "games", 4))
		askname();
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */

	Sprintf ( lock , "%d%s" , getuid ( ) , plname ) ;
	getlock ( ) ;

	/*
	 * Initialisation of the boundaries of the mazes
	 * Both boundaries have to be even.
	 */

	x_maze_max = COLNO-1;
	if (x_maze_max % 2)
		x_maze_max--;
	y_maze_max = ROWNO-1;
	if (y_maze_max % 2)
		y_maze_max--;

	/*
	 *  Initialize the vision system.  This must be before mklev() on a
	 *  new game or before a level restore on a saved game.
	 */
	vision_init();

	display_gamewindows();

	if ((fd = restore_saved_game()) >= 0) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifdef NEWS
		if(flags.news) {
			display_file(NEWS, FALSE);
			flags.news = FALSE;	/* in case dorecover() fails */
		}
#endif
		pline("Restoring save file...");
		mark_synch();	/* flush output */
		if(!dorecover(fd))
			goto not_recovered;
#ifdef WIZARD
		if(!wizard && remember_wiz_mode) wizard = TRUE;
#endif
		pline("Hello %s, welcome back to NetHack!", plname);
		check_special_room(FALSE);
		if (discover)
			You("are in non-scoring discovery mode.");

		if (discover || wizard) {
			if(yn("Do you want to keep the save file?") == 'n')
			    (void) delete_savefile();
			else {
			    compress(SAVEF);
			}
		}

		flags.move = 0;
	} else {
not_recovered:
		player_selection();
		newgame();
		/* give welcome message before pickup messages */
		pline("Hello %s, welcome to NetHack!", plname);
		if (discover)
			You("are in non-scoring discovery mode.");

		flags.move = 0;
		set_wear();
		pickup(1);
	}

	UndimMenuBar ( ) ; /* Yes, this is the place for it (!) */
	
	attemptingto("proceed");
#if defined(MAC68K) && defined(MAC_MPW32) && !defined(MODEL_FAR)
	UnloadAllSegments();						/* Do this before naming residents */
	IsResident( (Ptr) um_dist );				/* Sample resident segments */
	IsResident( (Ptr) flush_screen );
	IsResident( (Ptr) rhack );
	IsResident( (Ptr) remove_cadavers );
	IsResident( (Ptr) dog_move );
	IsResident( (Ptr) gethungry );
	IsResident( (Ptr) engr_at );
	IsResident( (Ptr) domove );
	IsResident( (Ptr) carried );
	IsResident( (Ptr) movemon );
	IsResident( (Ptr) attacktype ) ;
	IsResident( (Ptr) mac_get_nh_event ) ;
	IsResident( (Ptr) dosounds ) ;
	IsResident( (Ptr) t_at ) ;
	IsResident( (Ptr) nh_timeout ) ;
#endif
	moveloop();
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}


/*
 * This filter handles the movable-modal dialog
 *
 */
static pascal Boolean
DragFilter ( DialogPtr dp , EventRecord * event , short * item )
{
	WindowPtr wp ;
	short code ;
	Rect r ;

/*
 *	Handle shortcut keys
 *	enter, return -> OK
 *	clear, escape, period -> Cancel
 *	all others are handled default
 *
 */

	if ( event -> what == keyDown ) {

		char c = event -> message & 0xff ;
		unsigned char b = ( event -> message >> 8 ) & 0xff ;

		switch ( c ) {

		case 3 :	/* 3 == Enter */
		case 10 :	/* Newline */
		case 13 :	/* Return */
			* item = 1 ;
			return 1 ;

		case '.' :	/* Cmd-period - we allow only period */
		case 27 :	/* Escape */
			* item = 2 ;
			return 1 ;
		}

		switch ( b ) {

		case 0x4c :	/* Enter */
		case 0x24 :	/* Return */
			* item = 1 ;
			return 1 ;

		case 0x35 :	/* Escape */
		case 0x47 :	/* Clear */
			* item = 2 ;
			return 1 ;
		}

		return 0 ;
	}

/*
 *	OK, don't handle others
 *
 */

	if ( event -> what != mouseDown ) {

		return 0 ;
	}
	code = FindWindow ( event -> where , & wp ) ;
	if ( wp != dp || code != inDrag ) {

		return 0 ;
	}
	r = ( * GetGrayRgn ( ) ) -> rgnBBox ;
	InsetRect ( & r , 3 , 3 ) ;

	DragWindow ( wp , event -> where , & r ) ;
	SaveWindowPos ( wp ) ;

	event -> what = nullEvent ;
	return 1 ;
}


/*
 * plname is filled either by an option (-u Player  or  -uPlayer) or
 * explicitly (by being the wizard) or by askname.
 * It may still contain a suffix denoting pl_character.
 */
void
mac_askname(void) /* Code taken from getlin */
{
	asknameRec	anr;
	/* eventually use roles[] a/o pl_classes[] */
	static char		asknRoles [ ] = "ABCEHKPRSTVW" ;

	/* initialize the askname record */
	qd.randSeed = TickCount() ;
	anr.anMenu[anRole] = (Random ( ) & 0x7fff ) % askn_role_end;

#ifndef TOURIST
	if (anr.anMenu[anRole] == asknTourist) {
		anr.anMenu[anRole] = asknValkyrie;
	}
#endif

	if (anr.anMenu[anRole] == asknValkyrie) {
		anr.anMenu[anSex] = asknFemale;
	} else {
		anr.anMenu[anSex] = ( (Random() & 2) ? asknMale : asknFemale);
	}

	anr.anMenu[anMode] = asknRegular;

	InitCursor();
	DialogAskName(&anr);

	if (anr.anMenu[anMode] == asknQuit) {
		ExitToShell();
	}

	if (anr.anMenu[anMode] == asknExplore ) {
		discover = 1 ;
	} else {
		discover = 0 ;
	}

#ifdef WIZARD
	if ( anr.anMenu [ anMode ] == asknDebug ) {
		wizard = 1 ;
	} else {
		wizard = 0 ;
	}
	if (wizard) {
		strcpy(plname, WIZARD);
	} else
#endif
	{
		BlockMove(&(anr.anWho[1]), plname, anr.anWho[0]);
		plname [ anr . anWho [ 0 ] ] = 0 ;
	}

	flags.female = anr.anMenu[anSex];
	pl_character[0] = asknRoles[anr.anMenu[anRole]];
}


static void
process_options(void)
{
	int argc = 0 ;
	char * foo [ ] = { "Mac Hack" , (char *)0 } ;
	char * * argv = foo ;
	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
		case 'D':
# ifdef WIZARD
			wizard = TRUE ;
			break ;
# endif
		case 'X':
			discover = TRUE;
			break;
#ifdef NEWS
		case 'n':
			flags.news = FALSE;
			break;
#endif
		case 'u':
			if(argv[0][2])
			  (void) strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
			  argc--;
			  argv++;
			  (void) strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				raw_print("Player name expected after -u");
			break;
		case 'I':
		case 'i':
			if (!strncmpi(argv[0]+1, "IBM", 3))
				switch_graphics(IBM_GRAPHICS);
			break;
	    /*  case 'D': */
		case 'd':
			if (!strncmpi(argv[0]+1, "DEC", 3))
				switch_graphics(DEC_GRAPHICS);
			break;
		default:
			/* allow -T for Tourist, etc. */
			(void) strncpy(pl_character, argv[0]+1,
				sizeof(pl_character)-1);

			/* raw_print("Unknown option: %s", *argv); */
		}
	}
}


static void
whoami ( void )
{
	/*TODO*/
	donull ( ) ;
}


static OSErr
copy_file(short src_vol, long src_dir, short dst_vol, long dst_dir,
		  Str255 fName,
		  pascal OSErr (*opener)(short vRefNum, long dirID,
								 ConstStr255Param fileName,
								 signed char permission, short *refNum)) {
	short src_ref, dst_ref;
	OSErr err = (*opener)(src_vol, src_dir, fName, fsRdPerm, &src_ref);
	if (err == noErr) {
		err = (*opener)(dst_vol, dst_dir, fName, fsWrPerm, &dst_ref);
		if (err == noErr) {

			long file_len;
			err = GetEOF(src_ref, &file_len);
			if (err == noErr) {
				Handle buf;
				long count = MaxBlock();
				if (count > file_len)
					count = file_len;

				buf = NewHandle(count);
				err = MemError();
				if (err == noErr) {

					while (count > 0) {
						OSErr rd_err = FSRead(src_ref, &count, *buf);
						err = FSWrite(dst_ref, &count, *buf);
						if (err == noErr)
							err = rd_err;
						file_len -= count;
					}
					if (file_len == 0)
						err = noErr;

					DisposHandle(buf);

				}
			}
			FSClose(dst_ref);
		}
		FSClose(src_ref);
	}

	return err;
}

static void
force_hdelete(short vol, long dir, Str255 fName)
{
	HRstFLock(vol, dir, fName);
	HDelete  (vol, dir, fName);
}

void
finder_file_request(void)
{
#ifdef MAC68K
	short finder_msg, file_count;
	CountAppFiles(&finder_msg, &file_count);
	if (finder_msg == appOpen && file_count == 1) {
		OSErr	err;
		AppFile src;
		short	src_vol;
		long	src_dir, nul = 0;
		GetAppFiles(1, &src);
		err = GetWDInfo(src.vRefNum, &src_vol, &src_dir, &nul);
		if (err == noErr && src.fType == SAVE_TYPE) {

			if ( src_vol != theDirs.dataRefNum ||
				 src_dir != theDirs.dataDirID &&
				 CatMove(src_vol, src_dir, src.fName,
						 theDirs.dataDirID, "\p:") != noErr) {

				HCreate(theDirs.dataRefNum, theDirs.dataDirID, src.fName,
						MAC_CREATOR, SAVE_TYPE);
				err = copy_file(src_vol, src_dir, theDirs.dataRefNum,
								theDirs.dataDirID, src.fName, &HOpen); /* HOpenDF is only there under 7.0 */
				if (err == noErr)
					err = copy_file(src_vol, src_dir, theDirs.dataRefNum,
									theDirs.dataDirID, src.fName, &HOpenRF);
				if (err == noErr)
					force_hdelete(src_vol, src_dir, src.fName);
				else
					HDelete(theDirs.dataRefNum, theDirs.dataDirID, src.fName);
			}

			if (err == noErr) {
				short ref = HOpenResFile(theDirs.dataRefNum, theDirs.dataDirID,
										 src.fName, fsRdPerm);
				if (ref != -1) {
					Handle name = Get1Resource('STR ', PLAYER_NAME_RES_ID);
					if (name) {

						Str255 save_f_p;
						P2C(*(StringHandle)name, plname);
						set_savefile_name();
						C2P(SAVEF, save_f_p);
						force_hdelete(theDirs.dataRefNum, theDirs.dataDirID,
									  save_f_p);
						if (HRename(theDirs.dataRefNum, theDirs.dataDirID,
									src.fName, save_f_p) == noErr)
							ClrAppFiles(1);

					}
					CloseResFile(ref);
				}
			}
		}
	}
#endif /* MAC68K */
}



/*------------------- UnloadAllSegments and support stuff --------------------------*/
/* Derived from MacApp source */

/*
 * Don't unload segments unless you are exactly controlling the file
 * placement in each of the segments you plan to unload.
 *
 * The 68K MetroWerks compile is done with automatic segment placement, so
 * don't mess with this stuff...
 */
#if defined(MAC68K) && !defined(__MWERKS__)	/* segments only make sense on 68K macs */

typedef Handle **HandleListHandle;
typedef Boolean **BoolListHandle;
typedef short *ShortPtr, **ShortHandle;

#if defined(MAC_MPW32) || defined(THINK_C)
pascal long GetA5(void) = { 0x2E8D };					/* MOVE.L A5,(A7) */
pascal short GetCurJTOffset(void) = { 0x3EB8, 0x934 };	/* MOVE.W CurJTOffset,(SP) */
#endif

static void NDECL(ListGUnloads);
static short FDECL(GetSegNumber, (ShortPtr));
static void FDECL(NotResident, (void *));

short 			 pMaxSegNum = 0,		/* Highest segment number */
	  			 gCodeRefNum;			/* rsrc refnum of the application */
HandleListHandle gCodeSegs;				/* List of code seg handles */
BoolListHandle   gIsResidentSeg;		/* Resident flags */

#define kLoaded   0x4EF9				/* if loaded then a JMP instruction */
#define	kUnLoaded 0x3F3C				/* if unloaded then a LoadSeg trap */
										/* Note: probably incorrect for -model far! */

/* #define TRACKSEGS /* Utility to print a trace of segment load frequencies. */

#ifdef TRACKSEGS

long	  gUnloads[120];
char	  gSegNames[120][16];

void ListGUnloads(void)
{
  int i;
  FILE *f;
  
  f = fopen("unloads","w");
  fprintf(f,"%d calls to UnloadAllSegments\n\n",gUnloads[0]);
  for (i=1; i<=pMaxSegNum; i++) {
	 fprintf(f,"Unloaded %10s, segment %2d, %6d times\n",gSegNames[i],i,gUnloads[i]);
  }
  fclose(f);
}

#endif

short GetSegNumber(ShortPtr aProc)
/* Derives seg number from a procedure ptr */

{
	if (*aProc == kLoaded) 				/* loaded segment */
		return(*--aProc);
	else if (*aProc == kUnLoaded)  		/* unloaded segment */
		return(*++aProc);
	else {
		progerror("GetSegNumber was not passed an jump table address");
		return(1);
	}
}

void InitSegMgmt(void * mainSeg)
/* Initialise a list of handles to all the CODE segments and mark the mainseg as resident */
{
	short 	i,
			lastRsrc,
			rsrcID,
			oldResFile;
	Handle  seg;
	ResType rsrcType;
	Str255  rsrcName;
	 
	gCodeRefNum = HomeResFile(GetResource('CODE', 1));	
	oldResFile = CurResFile();
	UseResFile(gCodeRefNum);
	
	/* Discover the highest CODE rsrc ID: be ready for noncontiguous IDs */
	lastRsrc = Count1Resources('CODE');	
	SetResLoad(false);
	for (i=1; i<=lastRsrc; i++) 
		if (seg = Get1IndResource('CODE', i)) {
			GetResInfo(seg, &rsrcID, &rsrcType, rsrcName);
			if (rsrcID > pMaxSegNum) pMaxSegNum = rsrcID;
		}
		
	/* Make handles of appropriate size to keep flags/segment handles */
	SetResLoad(true);  /* In case we fail */
	gCodeSegs = (HandleListHandle) NewHandle((pMaxSegNum+1) * sizeof(Handle));	
	mustwork(MemError());
	gIsResidentSeg = (BoolListHandle) NewHandle((pMaxSegNum+1) * sizeof(Boolean));
	mustwork(MemError());
	SetResLoad(false);	

	#ifdef TRACKSEGS
	atexit(&ListGUnloads);
	gUnloads[0]=0;
	#endif
	for (i=1; i<=pMaxSegNum; i++) {
	   (*gIsResidentSeg)[i] = false;
	   (*gCodeSegs)[i] = Get1Resource('CODE',i);   /* Will be NIL if it doesn't exist */
	   #ifdef TRACKSEGS
	   {  /* Go get the segment name and save it */
	      short id;
		  ResType rType;
		  Str255 name;
		  char *cptr;
		  
		  GetResInfo((*gCodeSegs)[i],&id,&rType,&name);
		  if (name[0]>15) name[0]=15;
		  cptr = p2cstr(&name);
		  cptr = strcpy(&gSegNames[i], &name);
		  gUnloads[i] = 0;
	   }
	   #endif
	}
	SetResLoad(true);	
	(*gIsResidentSeg)[GetSegNumber((ShortPtr)mainSeg)] = true;	
	UseResFile(oldResFile);
}


void UnloadAllSegments(void)
{
  short	 i,
		 oldResFile;
  Handle seg;
  long	 jumpTablePtr;

  jumpTablePtr = GetA5() + GetCurJTOffset();
  oldResFile = CurResFile();
  UseResFile(gCodeRefNum);
#ifdef TRACKSEGS
  gUnloads[0]++;
#endif
  for (i=1; i<=pMaxSegNum; i++)
	  if (!(*gIsResidentSeg)[i]) {
		  seg = (*gCodeSegs)[i];
		  if ((seg != (Handle) nil) && (*seg != (Ptr) nil))  /* Check it exists and hasn't been purged */
			  if (HGetState(seg) & 0x80)  {   /* Is it locked? => loaded */
#ifdef TRACKSEGS
				 gUnloads[i]++;
#endif
				 UnloadSeg( (void *) (jumpTablePtr + **(ShortHandle)seg + 2) );
			  }
	  }

  UseResFile(oldResFile);
}

static void IsResident( void * routineaddr )
/* We want to move this high up in the heap as it won't be shifted again, so... */
{
	int    segnum;
	Handle theseg;
	
	segnum = GetSegNumber((ShortPtr)routineaddr);
	theseg = (*gCodeSegs)[segnum];
	UnloadSeg( routineaddr );
	if (*theseg != nil) {
	   MoveHHi( theseg );  /* If it has been purged we can't do this */
	   HLock( theseg );
	}
	(*gIsResidentSeg)[segnum] = true;	
}

static void NotResident( void * routineaddr )
{
	(*gIsResidentSeg)[GetSegNumber((ShortPtr)routineaddr)] = false;	
}

#endif /* MAC68K */

/*macmain.c*/
