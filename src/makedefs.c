/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* makedefs.c - NetHack version 3.0 */

#define MAKEDEFS_C 1	/* needs to be defined to 1 for Mac */

#define EXTERN_H
#include	"config.h"
#include	"permonst.h"
#include	"objclass.h"
#ifdef NULL
#undef NULL
#endif /* NULL */
#define NULL	((genericptr_t)0)

#ifndef LINT
static	const char	SCCS_Id[] = "@(#)makedefs.c\t3.0\t89/11/15";
#endif

#ifdef MSDOS
#ifndef TOS
# define freopen _freopen
#endif
# undef	exit
extern void FDECL(exit, (int));
# define RDMODE	"r"
# define WRMODE	"w"
#else
# define RDMODE  "r+"
# define WRMODE  "w+"
#endif
#if defined(SYSV) || defined(GENIX) || defined(UNIXDEBUG)
void rename();
#endif
#ifdef AMIGA
# undef freopen
# undef printf
# undef puts
# undef fflush
# define fflush FFLUSH
# undef fputs
# undef fprintf
#endif

/* construct definitions of object constants */

#ifdef AMIGA
# define MONST_FILE	 "include:pm.h"
# define ONAME_FILE	 "include:onames.h"
# define TRAP_FILE	 "include:trap.h"
# define DATE_FILE	 "include:date.h"
# define DATA_FILE	 "auxil:data"
# define RUMOR_FILE	 "auxil:rumors"
#else
# ifndef MACOS
/* construct definitions of object constants */
# define MONST_FILE	 "../include/pm.h"
# define ONAME_FILE	 "../include/onames.h"
# define TRAP_FILE	 "../include/trap.h"
# define DATE_FILE	 "../include/date.h"
# define DATA_FILE	 "../auxil/data"
# define RUMOR_FILE	 "../auxil/rumors"
# else
/*****
 * MAC OS uses ':' to separate dir's and filenames.
 * The following (partial) pathnames assume the makedefs program
 * runs in the same directory as the include and auxil directories
 *****/
# define MONST_FILE	":include:pm.h"
# define ONAME_FILE	":include:onames.h"
# define TRAP_FILE	":include:trap.h"
# define DATE_FILE	":include:date.h"
# define DATA_FILE	":auxil:data"
# define RUMOR_FILE	":auxil:rumors"
#  ifdef AZTEC
#define	perror(x)	Printf(x)
#include "Controls.h"
#  else
#include "ControlMgr.h"
#  endif
# endif
#endif


char	in_line[256];
extern char *FDECL(gets, (char *));
void do_objs(), do_traps(), do_data(), do_date(), do_permonst(), do_rumors();
#ifdef SMALLDATA
void do_monst(), save_resource();
#endif
char *FDECL(limit, (char *,BOOLEAN_P));
FILE *FDECL(_freopen, (char *,char *,FILE *));

int
main(argc, argv)
int	argc;
char	*argv[];
{
	char	*option;
#ifdef MACOS
	DialogPtr	dialog;
	char	params[3], *options;
	short	itemHit, lastItem, type;
	Rect	box;
	ControlHandle	theControl;
	GrafPtr	oldPort;

#define	OK_BUTTON	1
#define	CANCEL_BUTTON	2
#define	FIRST_RADIO_BUTTON	3
#define	ON	1
#define	OFF	0

	/* standard Mac initialization */
	InitGraf(&MAINGRAFPORT);
	
	InitFonts();
	InitWindows();
	InitMenus();
	InitCursor();
	FlushEvents(everyEvent,0);
	InitDialogs(NULL);
	
	params[0] = '-';
	options = "DVPRTOM";
	dialog = GetNewDialog(200, 0L, (WindowPtr) -1);
	GetPort(&oldPort);
	SetPort(dialog);
	GetDItem(dialog, OK_BUTTON, &type, &theControl, &box);
	LocalToGlobal(&box.top);
	LocalToGlobal(&box.bottom);
	SetPort(oldPort);
	PenSize(3, 3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenSize(1, 1);
	itemHit = 0;
	do {
		lastItem = itemHit;
		ModalDialog(NULL, &itemHit);
		if (itemHit != lastItem && itemHit > CANCEL_BUTTON) {
			if (lastItem) {
				GetDItem(dialog, lastItem, &type,
						&theControl, &box);
				SetCtlValue(theControl, OFF);
			}
			params[1] = options[itemHit - FIRST_RADIO_BUTTON];
			GetDItem(dialog, itemHit, &type, &theControl, &box);
			SetCtlValue(theControl, ON);
		}
	} while (itemHit >= FIRST_RADIO_BUTTON);
	DisposDialog(dialog);
	argc = 1;
	if (itemHit == OK_BUTTON && lastItem >= FIRST_RADIO_BUTTON) {
		argc = 2;
		option = params;
	
#else
	if(argc == 2) {
	    option = argv[1];
#endif
	    switch (option[1]) {

		case 'o':
		case 'O':	do_objs();
				break;
		case 't':
		case 'T':	do_traps();
				break;

		case 'd':
		case 'D':	do_data();
				break;

		case 'v':
		case 'V':	do_date();
				break;

		case 'p':
		case 'P':	do_permonst();
				break;

		case 'r':
		case 'R':	do_rumors();
				break;
#if defined(SMALLDATA) && defined(MACOS)
		case 'm':
		case 'M':	do_monst();
				break;
		
#endif	/* SMALLDATA && MACOS */

		default:
				(void) fprintf(stderr,
					"Unknown option '%c'.\n", option[1]);
				(void) fflush(stderr);
				exit(1);
	    }
	    exit(0);
	} else	(void) fprintf(stderr, "Bad arg count (%d).\n", argc-1);
	(void) fflush(stderr);
	exit(1);
/*NOTREACHED*/
#ifdef MSDOS
	return 0;
#endif
}

void
do_traps() {
	int	ntrap;
	char	tempfile[30];

#ifdef MACOS
	Sprintf(tempfile, ":include:makedefs.%d", getpid());
#else
	Sprintf(tempfile, "makedefs.%d", getpid());
#endif
	if(freopen(tempfile, WRMODE, stdout) == (FILE *)0) {
		perror(tempfile);
		exit(1);
	}

	if(freopen(TRAP_FILE, RDMODE, stdin) == (FILE *)0) {
		perror(TRAP_FILE);
		exit(1);
	}

	while(gets(in_line) != NULL) {
	    (void) puts(in_line);
	    if(!strncmp(in_line, "/* DO NOT REMOVE THIS LINE */", 29)) break;
	}
	ntrap = 10;
	Printf("\n");
	Printf("#define\tMGTRP\t\t%d\n", ntrap++);
	Printf("#define\tSQBRD\t\t%d\n", ntrap++);
	Printf("#define\tWEB\t\t%d\n", ntrap++);
	Printf("#define\tSPIKED_PIT\t%d\n", ntrap++);
	Printf("#define\tLEVEL_TELEP\t%d\n", ntrap++);
#ifdef SPELLS
	Printf("#define\tANTI_MAGIC\t%d\n", ntrap++);
#endif
	Printf("#define\tRUST_TRAP\t%d\n", ntrap++);
#ifdef POLYSELF
	Printf("#define\tPOLY_TRAP\t%d\n", ntrap++);
#endif
	Printf("#define\tLANDMINE\t%d\n", ntrap++);
	Printf("\n#define\tTRAPNUM\t%d\n", ntrap);
	Printf("\n#endif /* TRAP_H /**/\n");
	(void) fclose(stdin);
	(void) fclose(stdout);
#if defined(MSDOS) || defined(MACOS)
	remove(TRAP_FILE);
#endif
	rename(tempfile, TRAP_FILE);
	return;
}


void
do_rumors(){
	char	infile[30];
	FILE	*FDECL(freopen, (char *,char *,FILE *));
	long	true_rumor_size;

	if(freopen(RUMOR_FILE, WRMODE, stdout) == (FILE *)0) {
		perror(RUMOR_FILE);
		exit(1);
	}

	Sprintf(infile, "%s.tru", RUMOR_FILE);
	if(freopen(infile, RDMODE, stdin) == (FILE *)0) {
		perror(infile);
		exit(1);
	}

	/* get size of true rumors file */
	(void) fseek(stdin, 0L, 2);
	true_rumor_size = ftell(stdin);
	(void) fwrite((genericptr_t)&true_rumor_size,sizeof(long),1,stdout);
	(void) fseek(stdin, 0L, 0);

	/* copy true rumors */
	while(gets(in_line) != NULL)	 (void) puts(in_line);

	Sprintf(infile, "%s.fal", RUMOR_FILE);
	if(freopen(infile, RDMODE, stdin) == (FILE *)0) {
		perror(infile);
		exit(1);
	}

	/* copy false rumors */
	while(gets(in_line) != NULL)	 (void) puts(in_line);

	(void) fclose(stdin);
	(void) fclose(stdout);
	return;
}

#ifdef SYSV
extern long time();
#endif

void
do_date(){
	long	clock;
	char	cbuf[30], *c;

	if(freopen(DATE_FILE, WRMODE, stdout) == (FILE *)0) {
		perror(DATE_FILE);
		exit(1);
	}
	Printf("/*\tSCCS Id: @(#)date.h\t3.0\t88/11/20 */\n\n");

#ifdef KR1ED
	(void) time(&clock);
	Strcpy(cbuf, ctime(&clock));
#else
	(void) time((time_t *)&clock);
	Strcpy(cbuf, ctime((time_t *)&clock));
#endif
	for(c = cbuf; *c != '\n'; c++);	*c = 0; /* strip off the '\n' */
	Printf("const char datestring[] = \"%s\";\n", cbuf);

	(void) fclose(stdout);
	return;
}

void
do_data(){
	char	tempfile[30];

	Sprintf(tempfile, "%s.base", DATA_FILE);
	if(freopen(tempfile, RDMODE, stdin) == (FILE *)0) {
		perror(tempfile);
		exit(1);
	}

	if(freopen(DATA_FILE, WRMODE, stdout) == (FILE *)0) {
		perror(DATA_FILE);
		exit(1);
	}

	while(gets(in_line) != NULL) {
#ifndef GOLEMS
	    if(!strcmp(in_line, "'\ta golem;"))
		while(gets(in_line) != NULL && in_line[0] == '\t')
		    ; /* do nothing */
#endif
#ifndef	SPELLS
	    if(!strcmp(in_line, "+\ta spell book"))
		; /* do nothing */
	    else
#endif
#ifndef KOPS
	    if(!strcmp(in_line, "K\ta Keystone Kop"))
		; /* do nothing */
	    else
#endif
#ifndef WORM
	    if(!strcmp(in_line, "~\tthe tail of a long worm"))
		; /* do nothing */
	    else
#endif
		(void) puts(in_line);
	}
	(void) fclose(stdin);
	(void) fclose(stdout);
	return;
}

void
do_permonst() {

	int	i;
	char	*c;

	if(freopen(MONST_FILE, WRMODE, stdout) == (FILE *)0) {
		perror(MONST_FILE);
		exit(1);
	}
	Printf("/*\tSCCS Id: @(#)pm.h\t3.0\t88/11/20 */\n\n");
	Printf("#ifndef PM_H\n#define PM_H\n");

	for(i = 0; mons[i].mlet; i++) {
		Printf("\n#define\tPM_");
		for(c = mons[i].mname; *c; c++) {
		    if((*c >= 'a') && (*c <= 'z')) *c -= (char)('a' - 'A');
		    else if(*c == ' ' || *c == '-')	*c = '_';
		}
		Printf("%s\t%d", mons[i].mname, i);
	}
	Printf("\n\n#define\tNUMMONS\t%d\n", i);
	Printf("\n#endif /* PM_H /**/\n");
	(void) fclose(stdout);
	return;
}

static	char	temp[32];

char *
limit(name,pref)	/* limit a name to 30 characters length */
char	*name;
boolean	pref;
{
	(void) strncpy(temp, name, pref ? 26 : 30);
	temp[pref ? 26 : 30] = 0;
	return temp;
}

void
do_objs() {

	register int i = 0, sum = 0;
	register char *c;
#ifdef SPELLS
	register int nspell = 0;
#endif
	register boolean prefix = 0;
	register char let = '\0';
	boolean	sumerr = FALSE;

	if(freopen(ONAME_FILE, WRMODE, stdout) == (FILE *)0) {
		perror(ONAME_FILE);
		exit(1);
	}
	Printf("/*\tSCCS Id: @(#)onames.h\t3.0\t89/01/10 */\n\n");
	Printf("#ifndef ONAMES_H\n#define ONAMES_H\n\n");

	for(i = 0; !i || objects[i].oc_olet != ILLOBJ_SYM; i++) {
		if (!(c = objects[i].oc_name)) continue;

		/* make sure probabilities add up to 1000 */
		if(objects[i].oc_olet != let) {
			if (sum && sum != 1000) {
			    (void) fprintf(stderr,
					"prob error for %c (%d%%)", let, sum);
			    (void) fflush(stderr);
			    sumerr = TRUE;
			}
			let = objects[i].oc_olet;
			sum = 0;
		}

		for(; *c; c++) {
		    if((*c >= 'a') && (*c <= 'z')) *c -= (char)('a' - 'A');
		    else if(*c == ' ' || *c == '-')	*c = '_';
		}

		switch (let) {
		    case WAND_SYM:
			Printf("#define\tWAN_"); prefix = 1; break;
		    case RING_SYM:
			Printf("#define\tRIN_"); prefix = 1; break;
		    case POTION_SYM:
			Printf("#define\tPOT_"); prefix = 1; break;
#ifdef SPELLS
		    case SPBOOK_SYM:
			Printf("#define\tSPE_"); prefix = 1; nspell++; break;
#endif
		    case SCROLL_SYM:
			Printf("#define\tSCR_"); prefix = 1; break;
		    case GEM_SYM:
			/* avoid trouble with stupid C preprocessors */
			if(objects[i].oc_material == GLASS) {
			    Printf("/* #define\t%s\t%d */\n",
							objects[i].oc_name, i);
			    continue;
			}
		    default:
			Printf("#define\t");
		}
		Printf("%s\t%d\n", limit(objects[i].oc_name, prefix), i);
		prefix = 0;

		sum += objects[i].oc_prob;
	}
	Printf("#define\tLAST_GEM\t(JADE+1)\n");
#ifdef SPELLS
	Printf("#define\tMAXSPELL\t%d\n", nspell+1);
#endif
	Printf("#define\tNROFOBJECTS\t%d\n", i-1);
	Printf("\n#endif /* ONAMES_H /**/\n");
	(void) fclose(stdout);
	if (sumerr) exit(1);
	return;
}

#if defined(SYSV) || defined(GENIX) || defined(UNIXDEBUG)
void
rename(oldname, newname)
char	*oldname, *newname;
{
	if (strcmp(oldname, newname)) {
		(void) unlink(newname);
		(void) link(oldname, newname);
		(void) unlink(oldname);
	}
	return;
}
#endif

#ifdef MSDOS
# if !defined(AMIGA) && !defined(TOS)
/* Get around bug in freopen when opening for writing	*/
/* Supplied by Nathan Glasser (nathan@mit-eddie)	*/
#undef freopen
FILE *
_freopen(fname, fmode, fp)
char *fname, *fmode;
FILE *fp;
{
    if (!strncmp(fmode,"w",1))
    {
	FILE *tmpfp;

	if ((tmpfp = fopen(fname,fmode)) == (FILE *)0)
	    return (FILE *)0;
	if (dup2(fileno(tmpfp),fileno(fp)) < 0)
	    return (FILE *)0;
	(void) fclose(tmpfp);
	return fp;
    }
    else
	return freopen(fname,fmode,fp);
}
# endif /* !AMIGA && !TOS */

# if defined(__TURBOC__) || defined(AMIGA)
int
getpid()
{
	return 1;
}
# endif
#endif /* MSDOS */


#if defined(SMALLDATA) && defined(MACOS)
void
do_monst()
{
	Handle	monstData, objData;
	short i,j;
	pmstr	*pmMonst;
	SFReply	reply;
	short	refNum,error;
	Str255	name;
	short	findNamedFile();
	OSErr	write_resource();
	
	for(i = 0; mons[i].mlet; i++) {
		;
	}
	i++;
	
	/*
	 * convert to struct where character arrays instead of pointers to
	 * strings are used
	 */
	pmMonst = (pmstr *)NewPtr(i*sizeof(struct pmstr));
	for (j = 0; j < i; j++) {
		Strcpy(pmMonst[j].mname, mons[j].mname);
		BlockMove(&(mons[j].mlet), &(pmMonst[j].pmp.mlet),
				(long)sizeof(struct pmpart));
	}
	
	PtrToHand((Ptr)pmMonst, &monstData, (long)(i * sizeof(struct pmstr)));
	
	/* store the object data, in Nethack the char * will be copied in */
	for(i = 0; !i || objects[i].oc_olet != ILLOBJ_SYM; i++) {
		;
	}
	PtrToHand((Ptr)objects, &objData, ((i+1)*sizeof(struct objclass)));

	strcpy((char *)&name[0], "\014Nethack.rsrc");
	if (findNamedFile(&name[1], 0, &reply)) {
	    strncpy((char *)&name[0],(char *)&reply.fName[0], reply.fName[0]+1);
	    if ((refNum = OpenResFile(name)) != -1) {
		if (ResError() == noErr) {
		    strcpy((char *)&name[0], "\012MONST_DATA");
		    if (error = write_resource(monstData,
						MONST_DATA, name, refNum)) {
		    	SysBeep(1);
		    	Printf("Couldn't add monster data resource.\n");
		    }
		    strcpy((char *)&name[0], "\013OBJECT_DATA");
		    if (error = write_resource(objData,
						OBJECT_DATA, name, refNum)) {
		    	SysBeep(1);
		    	Printf("Couldn't add object data resource.\n");
		    }
		    CloseResFile(refNum);
		    if (ResError() != noErr) {
			SysBeep(1);
			Printf("Couldn't close resource file.");
		    }
		}
	    }
	}
	
	DisposHandle(monstData);
	DisposHandle(objData);
}

OSErr
write_resource(data, resID, resName, refNum)
Handle	data;
short	resID;
Str255	resName;
short	refNum;
{
	ResType	theType;
	short	error;
	Handle	theRes;

    theType = HACK_DATA;
    error = CurResFile();
    if (theRes = GetResource(theType, resID)) {
		RmveResource(theRes);
		error = ResError();
		if (error == noErr) {
			DisposHandle(theRes);
			UpdateResFile(refNum);
			error = ResError();
		}
		if (error != noErr) {
    		return error;
    	}
	} else if (ResError() != resNotFound && ResError() != noErr) {
			return (ResError());
	}
	AddResource(data, theType, resID, resName);
	error = ResError();
	if (error == noErr) {
		WriteResource(data);
		error = ResError();
	}
    return error;
}
# if defined(AZTEC) || defined(THINKC4)
int
getpid()
{
	return 1;
}
# endif
#endif	/* SMALLDATA && MACOS */
