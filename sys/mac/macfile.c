/*	SCCS Id: @(#)macfile.c	3.1	93/01/24		  */
/* Copyright (c) Jon W{tte, Hao-Yang Wang, Jonathan Handler 1992. */
/* NetHack may be freely redistributed.  See license for details. */
/*
 * macfile.c
 * MAC file I/O routines
 */

#include "hack.h"

#include <files.h>
#include <errors.h>
#include <resources.h>
#include <memory.h>
#include <ToolUtils.h>

/*
 * We should get the default dirID and volRefNum (from name) from prefs and
 * the situation at startup... For now, this will have to do.
 */


/* The HandleFiles are resources built into the application which are treated
   as read-only files: if we fail to open a file we look for a resource */
   
#define FIRST_HF 32000   /* file ID of first HandleFile */
#define MAX_HF 6		 /* Max # of open HandleFiles */

#define APP_NAME_RES_ID		(-16396)
#define PLAYER_NAME_RES_ID	1001

int FDECL(maccreat,(const char *, long));
int FDECL(macopen,(const char *, int, long));
int FDECL(macclose,(int));
int FDECL(macread,(int, void *, unsigned));
int FDECL(macwrite,(int, void *, unsigned));
long FDECL(macseek,(int, long, short));

char * FDECL(macgets,(int, char *, unsigned));

static short FDECL(IsHandleFile,(int));
static int FDECL(OpenHandleFile,(const unsigned char *, long));
static int FDECL(CloseHandleFile,(int));
static int FDECL(ReadHandleFile,(int, void *, unsigned));
static long FDECL(SetHandleFilePos,(int, short, long));
static void FDECL(C2P,(const char *, unsigned char *));

typedef struct handlefile {
	long		type ;  /* Resource type */
	short		id ;	/* Resource id */
	long		mark ;  /* Current position */
	long		size ;  /* total size */
	Handle		data ;  /* The resource, purgeable */
} HandleFile ;

HandleFile theHandleFiles [ MAX_HF ] ;

static short
IsHandleFile ( int fd )
{
	return (fd >= FIRST_HF) && 
		   (fd < FIRST_HF + MAX_HF) && 
		   (theHandleFiles[ fd - FIRST_HF ].data) ;
}


static int
OpenHandleFile ( const unsigned char * name , long fileType )
{
	int i ;
	OSErr err;
	Handle h ;
	Str255 s ;

	for ( i = 0 ; i < MAX_HF ; i ++ ) {
		if ( theHandleFiles[i].data == 0L ) break ;
	}
	
	if ( i >= MAX_HF ) {
		error("Ran out of HandleFiles");
		return -1 ;
	}

	h = GetNamedResource ( fileType , name ) ;
	err = ResError();
	if (err == resNotFound) return -1;  /* Don't complain, this might be normal */
	if ( ! itworked(err) ) return -1;
	
	theHandleFiles[i].data = h;
	theHandleFiles[i].size = GetHandleSize ( h ) ;
	GetResInfo ( h, & theHandleFiles[i].id, ( void* ) & theHandleFiles[i].type, s ) ;
	theHandleFiles[i].mark = 0L ;

	HPurge( h ) ;
	return(i + FIRST_HF);
}


static int
CloseHandleFile ( int fd )
{
	if ( ! IsHandleFile ( fd ) ) {
	   error("CloseHandleFile: isn't a handle");
	   return -1 ;
	}
	fd -= FIRST_HF ;
	ReleaseResource ( theHandleFiles[fd].data ) ;
	theHandleFiles[fd].data = 0L ;
	return(0);
}


static int
ReadHandleFile ( int fd , void * ptr , unsigned len )
{
	unsigned maxBytes ;
	Handle h ;

	if ( ! IsHandleFile ( fd ) ) return -1;
	
	fd -= FIRST_HF ;
	maxBytes = theHandleFiles[fd].size - theHandleFiles[fd].mark ;
	if ( len > maxBytes ) len = maxBytes ;
	
	h = theHandleFiles[fd].data ;
	LoadResource ( h ) ;
	if ( ! itworked(ResError()) ) return (-1);
	
	HLock(h);
	BlockMove ( *h + theHandleFiles[fd].mark , ptr , len );
	HUnlock(h);
	theHandleFiles[fd].mark += len ;

/*	comment("ReadHandleFile ",len); */
	
	return(len);
}


static long
SetHandleFilePos ( int fd , short whence , long pos )
{
	long curpos;
	
	if ( ! IsHandleFile ( fd ) ) return -1;
	
	fd -= FIRST_HF ;
	
	curpos = theHandleFiles [ fd ].mark;
	switch ( whence ) {
	case SEEK_CUR : 
		curpos += pos ;
		break ;
	case SEEK_END : 
		curpos = theHandleFiles[fd].size  - pos ;
		break ;
	default : /* set */
		curpos = pos ;
		break ;
	}

	if ( curpos < 0 ) curpos = 0 ;
	
	if ( curpos > theHandleFiles [ fd ].size ) curpos = theHandleFiles [ fd ].size ;
	
	theHandleFiles [ fd ].mark = curpos;
	
	return curpos ;
}


static void
C2P ( const char * c , unsigned char * p )
{
	long len = strlen ( c ) ;

	if ( len > 255 ) len = 255 ;

	p[0] = len & 0xff ;
	while (*c) *++p = *c++;
}

static void
P2C ( const unsigned char * p , char * c )
{
	int idx = p[0];
	c[idx] = '\0';
	while (idx > 0) {
		c[idx-1] = p[idx];
		--idx;
	}
}


static void
replace_resource(Handle new_res, ResType its_type, short its_id,
				 Str255 its_name) {
	Handle old_res;
	SetResLoad(false);
	old_res = Get1Resource(its_type, its_id);
	SetResLoad(true);
	if (old_res) {
		RmveResource(old_res);
		DisposHandle(old_res);
	}

	AddResource(new_res, its_type, its_id, its_name);
}


int
maccreat ( const char * name , long fileType )
{
	return macopen ( name , O_RDWR | O_CREAT | O_TRUNC , fileType ) ;
}


int
macopen ( const char * name , int flags , long fileType )
{
	short refNum ;
	short perm ;
	Str255 s ;

	C2P ( name , s ) ;
	if ( flags & O_CREAT ) {
		if ( HCreate ( theDirs.dataRefNum , theDirs.dataDirID , s ,
			MAC_CREATOR , fileType ) && ( flags & O_EXCL ) ) {
			return -1 ;
		}

		if (fileType == SAVE_TYPE) {
			short resRef;
			HCreateResFile(theDirs.dataRefNum, theDirs.dataDirID, s);
			resRef = HOpenResFile(theDirs.dataRefNum, theDirs.dataDirID, s,
								  fsRdWrPerm);
			if (resRef != -1) {
				Handle name;
				Str255 plnamep;

				C2P(plname, plnamep);
				name = (Handle)NewString(plnamep);
				if (name)
					replace_resource(name, 'STR ', PLAYER_NAME_RES_ID,
									"\pPlayer Name");

				/* The application name resource.  See IM VI, page 9-21. */
				name = (Handle)GetString(APP_NAME_RES_ID);
				if (name) {
					DetachResource(name);
					replace_resource(name, 'STR ', APP_NAME_RES_ID,
									 "\pApplication Name");
				}

				CloseResFile(resRef);
			}
		}

	}
	/*
	 * Here, we should check for file type, maybe a SFdialog if
	 * we fail with default, etc. etc. Besides, we should use HOpen
	 * and permissions.
	 */
	if ( ( flags & O_RDONLY ) == O_RDONLY ) {
		perm = fsRdPerm ;
	}
	if ( ( flags & O_WRONLY ) == O_WRONLY ) {
		perm = fsWrPerm ;
	}
	if ( ( flags & O_RDWR ) == O_RDWR ) {
		perm = fsRdWrPerm ;
	}
	if ( HOpen ( theDirs.dataRefNum , theDirs.dataDirID , s ,
		perm , & refNum ) ) {
		return OpenHandleFile ( s , fileType ) ;
	}
	if ( flags & O_TRUNC ) {
		if ( SetEOF ( refNum , 0L ) ) {
			FSClose ( refNum ) ;
			return -1 ;
		}
	}
	return refNum ;
}


int
macclose ( int fd )
{
	if ( IsHandleFile ( fd ) ) {
		CloseHandleFile ( fd ) ;
	} else {
		if ( FSClose ( fd ) ) {
			return -1 ;
		}
		FlushVol ( (StringPtr) NULL , theDirs . dataRefNum ) ;
	}
	return 0 ;
}


int
macread ( int fd , void * ptr , unsigned len )
{
	long amt = len;
	
	if ( IsHandleFile ( fd ) ) {

		return ReadHandleFile ( fd , ptr , amt ) ;
	} else {

		short err = FSRead ( fd , & amt , ptr ) ;
		if ( err == eofErr && len ) {

			return amt ;
		}
		if  ( itworked ( err ) ) {

			return ( amt ) ;

		} else {

			return -1 ;
		}
	}
}


char *
macgets ( int fd , char * ptr , unsigned len )
{
        int idx = 0 ;
        char c;

        while ( -- len > 0 ) {
                if ( macread ( fd , ptr + idx , 1 ) <= 0 )
                        return NULL ;
                c = ptr[ idx++ ];
                if ( c  == '\n' || c == '\r' )
                        break ;
        }
        ptr [ idx ] = '\0' ;
        return ptr ;
}


int
macwrite ( int fd , void * ptr , unsigned len )
{
	long amt = len ;

	if ( IsHandleFile ( fd ) ) return -1 ;
	
	if ( itworked( FSWrite ( fd , & amt , ptr ) ) ) return(amt) ;
		else return(-1) ;
}


long
macseek ( int fd , long where , short whence )
{
	short posMode ;
	long curPos ;

	if ( IsHandleFile ( fd ) ) {
		return SetHandleFilePos ( fd , whence , where ) ;
	}

	switch ( whence ) {
	default :
		posMode = fsFromStart ;
		break ;
	case SEEK_CUR :
		posMode = fsFromMark ;
		break ;
	case SEEK_END :
		posMode = fsFromLEOF ;
		break ;
	}

	if ( itworked( SetFPos ( fd , posMode, where ) )  &&
	     itworked( GetFPos ( fd , &curPos )) )
		    return(curPos);
	   
	return(-1);
}


static OSErr
copy_file(short src_vol, long src_dir, short dst_vol, long dst_dir,
		  Str255 fName,
		  pascal OSErr (*opener)(short vRefNum, long dirID,
								 ConstStr255Param fileName,
								 char permission, short *refNum)) {
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
force_hdelete(short vol, long dir, Str255 fName) {
	HRstFLock(vol, dir, fName);
	HDelete  (vol, dir, fName);
}

void
finder_file_request(void) {
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
}
