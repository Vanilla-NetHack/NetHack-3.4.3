/*	SCCS Id: @(#)def_os2.h	3.0	89/08/13

/*  OS/2 defines based on MSC 5.1 OS/2 include files.
    Only a small portion of all OS/2 defines are needed,
    so the actual include files are not used.

    Timo Hakulinen
 */

#define APIENTRY pascal far

#define CHAR	char		/* ch  */
#define SHORT	int		/* s   */
#define LONG	long		/* l   */
#define INT	int		/* i   */

typedef unsigned char UCHAR;	/* uch */
typedef unsigned int  USHORT;	/* us  */
typedef unsigned long ULONG;	/* ul  */
typedef unsigned int  UINT;	/* ui  */

typedef unsigned char BYTE;	/* b   */
typedef BYTE   far *PBYTE;

typedef unsigned short	SHANDLE;
typedef SHANDLE 	HKBD;
typedef SHANDLE 	HVIO;
typedef SHANDLE 	HDIR;	/* hdir */
typedef HDIR far *PHDIR;

typedef USHORT far *PUSHORT;
typedef char far *PSZ;

typedef struct {
	UCHAR  chChar;
	UCHAR  chScan;
	UCHAR  fbStatus;
	UCHAR  bNlsShift;
	USHORT fsState;
	ULONG  time;
} KBDKEYINFO;
typedef KBDKEYINFO far *PKBDKEYINFO;

/* File time and date types */

typedef struct _FTIME { 	/* ftime */
    unsigned twosecs : 5;
    unsigned minutes : 6;
    unsigned hours   : 5;
} FTIME;
typedef FTIME far *PFTIME;

typedef struct _FDATE { 	/* fdate */
    unsigned day     : 5;
    unsigned month   : 4;
    unsigned year    : 7;
} FDATE;
typedef FDATE far *PFDATE;

typedef struct _FILEFINDBUF {	/* findbuf */
	FDATE  fdateCreation;
	FTIME  ftimeCreation;
	FDATE  fdateLastAccess;
	FTIME  ftimeLastAccess;
	FDATE  fdateLastWrite;
	FTIME  ftimeLastWrite;
	ULONG  cbFile;
	ULONG  cbFileAlloc;
	USHORT attrFile;
	UCHAR  cchName;
	CHAR   achName[13];
} FILEFINDBUF;
typedef FILEFINDBUF far *PFILEFINDBUF;

/* KBDINFO structure, for KbdSet/GetStatus */
typedef struct _KBDINFO {	/* kbst */
	USHORT cb;
	USHORT fsMask;
	USHORT chTurnAround;
	USHORT fsInterim;
	USHORT fsState;
} KBDINFO;
typedef KBDINFO far *PKBDINFO;

/* VIOMODEINFO structure, for VioGetMode */
typedef struct _VIOMODEINFO {
	USHORT cb;
	UCHAR  fbType;
	UCHAR  color;
	USHORT col;
	USHORT row;
	USHORT hres;
	USHORT vres;
	UCHAR  fmt_ID;
	UCHAR  attrib;
} VIOMODEINFO;
typedef VIOMODEINFO far *PVIOMODEINFO;

/* OS2 API functions */

USHORT APIENTRY KbdGetStatus(PKBDINFO, HKBD);
USHORT APIENTRY KbdSetStatus(PKBDINFO, HKBD);
USHORT APIENTRY KbdCharIn(PKBDKEYINFO, USHORT, HKBD );
USHORT APIENTRY DosQFSInfo(USHORT, USHORT, PBYTE, USHORT);
USHORT APIENTRY DosFindFirst(PSZ, PHDIR, USHORT, PFILEFINDBUF, USHORT, PUSHORT, ULONG);
USHORT APIENTRY DosFindNext(HDIR, PFILEFINDBUF, USHORT, PUSHORT);
USHORT APIENTRY DosSelectDisk(USHORT);
USHORT APIENTRY VioGetMode(PVIOMODEINFO, HVIO);
USHORT APIENTRY VioSetCurPos(USHORT, USHORT, HVIO);
