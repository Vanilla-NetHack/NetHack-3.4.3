/*	SCCS Id: @(#)macfile.c	3.0	88/08/05
/*      Copyright (c) Johnny Lee, 1989.		 */
/* NetHack may be freely redistributed.  See license for details. */

/*	Common routines to locate files using mac dialog boxes */

#include "config.h"
#ifdef MACOS

#define	LARGE_SFGETDLG	-4000
short
findNamedFile(filename,type,reply)
char	*filename;
short	type;	/* if 0 - don't care if name matches selected file */
SFReply	*reply;
{
	DialogPtr	dialog;
	short	numTypes;
	SFTypeList	types;	
	Str255	prompt, name;
	Point	where;
	short	ok;
	DialogRecord	storage;
	
	name[0] = (char)strlen(filename);
	Strcpy((char *)&name[1], filename);
	SetResLoad(TRUE);
	
	if (type == 1)
		ParamText("\005 save","\004 for",name,"");
	else
		ParamText("","",name,"");
	
	where.h = 80;
	where.v = 111;
	ok = FALSE;
	
	switch (type) {
	  case 0:	/* don't care what file gets loaded */
	  	numTypes = -1;
	  	break;
	  case 1:	/* limit what types of files can be selected */
	  	numTypes = 2;
		types[0] = SAVE_TYPE;
		types[1] = EXPLORE_TYPE;
		break;
	  case 2:	/* unlimited types of files but names have to match*/
	  	numTypes = -1;
	  	break;
	}
	reply->good = TRUE;
	do {
		SFPGetFile(where,prompt,0L,numTypes,types,0L,reply,LARGE_SFGETDLG,0L);
		if (reply->good) {
			if ((type == 2 && 
				!strncmp((char *)&name[1],
				    (char *)&(reply->fName[1]), (short)name[0]))
				 || (type<2)) {
				SetVol(0L,reply->vRefNum);
				ok = TRUE;
			}
		}
	} while (!ok && reply->good);

	return ok;
}

#ifdef CUSTOM_IO
extern WindowPtr HackWindow;
extern short macflags;
/*	this function also gets called by topten() in topten.c to
 *	locate the record file, but it doesn't matter at this point
 *	since the game is over by now. If nethack ever restarts,
 *	this will have to change.
 */

FILE *
openFile(fileName, rdmode)
char	*fileName, *rdmode;
{
	term_info *t;
	FILE	*fp;
		
	t = (term_info *)GetWRefCon(HackWindow);

	SetVol(0L, t->recordVRefNum);
	fp = fopen(fileName, rdmode);
	if (!fp) {		
		SFReply	reply;

		if (t->auxFileVRefNum) {
			SetVol(0L,t->auxFileVRefNum);
		}
		
		reply.good = false;
		fp = fopen(fileName, rdmode);
		if (!fp && findNamedFile(fileName,2,&reply)) {
			if (reply.good) {
				t->auxFileVRefNum = reply.vRefNum;
			}
		}
		
		if (!fp)
			fp = fopen(fileName, rdmode);
		else if (!t->auxFileVRefNum && reply.good) {
			(void)GetVol((StringPtr)&reply.fName,&t->auxFileVRefNum);
		}

		SetPort(HackWindow);
		if ((macflags & fDoUpdate) && !reply.good)
			docrt();

	}

	return fp;
}
#endif
#endif
