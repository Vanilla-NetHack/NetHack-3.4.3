/*	SCCS Id: @(#)msdos.c	 3.1	 93/02/16		  */
/* Copyright (c) NetHack PC Development Team 1990, 1991, 1992	  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  MSDOS system functions.
 *  Many thanks to Don Kneller who originated the DOS port and
 *  contributed much to the cause.
 */

#define NEED_VARARGS
#include "hack.h"
#ifdef MICRO
#include "termcap.h"
#endif

#ifdef MSDOS

#include <dos.h>
#include <ctype.h>

/*
 * MS-DOS functions
 */
#define DIRECT_INPUT    0x07    /* Unfiltered Character Input Without Echo */
#define FATINFO     0x1B    /* Get Default Drive Data */
/* MS-DOS 2.0+: */
#define GETDTA      0x2F    /* Get DTA Address */
#define FREESPACE   0x36    /* Get Drive Allocation Info */
#define GETSWITCHAR 0x3700  /* Get Switch Character */
#define FINDFIRST   0x4E    /* Find First File */
#define FINDNEXT    0x4F    /* Find Next File */
#define SETFILETIME 0x5701  /* Set File Date & Time */
/*
 * BIOS interrupts
 */
#define KEYBRD_BIOS 0x16
#define VIDEO_BIOS  0x10
/*
 * Keyboard BIOS functions
 */
#define READCHAR    0x00    /* Read Character from Keyboard */
#define GETKEYFLAGS 0x02    /* Get Keyboard Flags */
/*
 * Video BIOS functions
 */
#define SETCURPOS   0x02    /* Set Cursor Position */
#define GETMODE     0x0f    /* Get Video Mode */
#define FONTINFO    0x1130  /* Get Font Info */


#ifdef OVL0

/* direct bios calls are used only when flags.BIOS is set */

static char NDECL(DOSgetch);
static char NDECL(BIOSgetch);
static char * NDECL(getdta);
static unsigned int FDECL(dos_ioctl, (int,int,unsigned));

int
tgetch()
{
	char ch;

	/* BIOSgetch can use the numeric key pad on IBM compatibles. */
	if (flags.BIOS)
		ch = BIOSgetch();
	else
		ch = DOSgetch();
	return ((ch == '\r') ? '\n' : ch);
}



/*
 *  Keyboard translation tables.
 */
#define KEYPADLO	0x47
#define KEYPADHI	0x53

#define PADKEYS 	(KEYPADHI - KEYPADLO + 1)
#define iskeypad(x)	(KEYPADLO <= (x) && (x) <= KEYPADHI)

/*
 * Keypad keys are translated to the normal values below.
 * When flags.BIOS is active, shifted keypad keys are translated to the
 *    shift values below.
 */
static const struct pad {
	char normal, shift, cntrl;
} keypad[PADKEYS] = {
			{'y', 'Y', C('y')},		/* 7 */
			{'k', 'K', C('k')},		/* 8 */
			{'u', 'U', C('u')},		/* 9 */
			{'m', C('p'), C('p')},		/* - */
			{'h', 'H', C('h')},		/* 4 */
			{'g', 'g', 'g'},		/* 5 */
			{'l', 'L', C('l')},		/* 6 */
			{'p', 'P', C('p')},		/* + */
			{'b', 'B', C('b')},		/* 1 */
			{'j', 'J', C('j')},		/* 2 */
			{'n', 'N', C('n')},		/* 3 */
			{'i', 'I', C('i')},		/* Ins */
			{'.', ':', ':'}			/* Del */
}, numpad[PADKEYS] = {
			{'7', M('7'), '7'},		/* 7 */
			{'8', M('8'), '8'},		/* 8 */
			{'9', M('9'), '9'},		/* 9 */
			{'m', C('p'), C('p')},		/* - */
			{'4', M('4'), '4'},		/* 4 */
			{'g', 'G', 'g'},		/* 5 */
			{'6', M('6'), '6'},		/* 6 */
			{'p', 'P', C('p')},		/* + */
			{'1', M('1'), '1'},		/* 1 */
			{'2', M('2'), '2'},		/* 2 */
			{'3', M('3'), '3'},		/* 3 */
			{'i', 'I', C('i')},		/* Ins */
			{'.', ':', ':'}			/* Del */
};

/*
 * Unlike Ctrl-letter, the Alt-letter keystrokes have no specific ASCII
 * meaning unless assigned one by a keyboard conversion table, so the
 * keyboard BIOS normally does not return a character code when Alt-letter
 * is pressed.	So, to interpret unassigned Alt-letters, we must use a
 * scan code table to translate the scan code into a letter, then set the
 * "meta" bit for it.  -3.
 */
#define SCANLO		0x10

static const char scanmap[] = { 	/* ... */
	'q','w','e','r','t','y','u','i','o','p','[',']', '\n',
	0, 'a','s','d','f','g','h','j','k','l',';','\'', '`',
	0, '\\', 'z','x','c','v','b','n','m',',','.','?'	/* ... */
};

#define inmap(x)	(SCANLO <= (x) && (x) < SCANLO + SIZE(scanmap))

/*
 * BIOSgetch gets keys directly with a BIOS call.
 */
#define SHIFT		(0x1 | 0x2)
#define CTRL		0x4
#define ALT		0x8

static char
BIOSgetch()
{
	unsigned char scan, shift, ch;
	const struct pad *kpad;

	union REGS regs;

	/* Get scan code.
	 */
	regs.h.ah = READCHAR;
	int86(KEYBRD_BIOS, &regs, &regs);
	ch = regs.h.al;
	scan = regs.h.ah;
	/* Get shift status.
	 */
	regs.h.ah = GETKEYFLAGS;
	int86(KEYBRD_BIOS, &regs, &regs);
	shift = regs.h.al;

	/* Translate keypad keys */
	if (iskeypad(scan)) {
		kpad = flags.num_pad ? numpad : keypad;
		if (shift & SHIFT)
			ch = kpad[scan - KEYPADLO].shift;
		else if (shift & CTRL)
			ch = kpad[scan - KEYPADLO].cntrl;
		else
			ch = kpad[scan - KEYPADLO].normal;
	}
	/* Translate unassigned Alt-letters */
	if ((shift & ALT) && !ch) {
		if (inmap(scan))
			ch = scanmap[scan - SCANLO];
		return (isprint(ch) ? M(ch) : ch);
	}
	return ch;
}

static char
DOSgetch()
{
	union REGS regs;
	char ch;
	struct pad (*kpad)[PADKEYS];

	regs.h.ah = DIRECT_INPUT;
	intdos(&regs, &regs);
	ch = regs.h.al;

	/*
	 * The extended codes for Alt-shifted letters, and unshifted keypad
	 * and function keys, correspond to the scan codes.  So we can still
	 * translate the unshifted cursor keys and Alt-letters.  -3.
	 */
	if (ch == 0) {		/* an extended key */
		regs.h.ah = DIRECT_INPUT;
		intdos(&regs, &regs);	/* get the extended key code */
		ch = regs.h.al;

		if (iskeypad(ch)) {	/* unshifted keypad keys */
			kpad = (void *)(flags.num_pad ? numpad : keypad);
			ch = (*kpad)[ch - KEYPADLO].normal;
		} else if (inmap(ch)) { /* Alt-letters */
			ch = scanmap[ch - SCANLO];
			if (isprint(ch)) ch = M(ch);
		} else ch = 0;		/* munch it */
	}
	return (ch);
}

char
switchar()
{
	union REGS regs;

	regs.x.ax = GETSWITCHAR;
	intdos(&regs, &regs);
	return regs.h.dl;
}

long
freediskspace(path)
char *path;
{
	union REGS regs;

	regs.h.ah = FREESPACE;
	if (path[0] && path[1] == ':')
		regs.h.dl = (toupper(path[0]) - 'A') + 1;
	else
		regs.h.dl = 0;
	intdos(&regs, &regs);
	if (regs.x.ax == 0xFFFF)
		return -1L;		/* bad drive number */
	else
		return ((long) regs.x.bx * regs.x.cx * regs.x.ax);
}

#ifndef __GO32__
/*
 * Functions to get filenames using wildcards
 */
int
findfirst(path)
char *path;
{
	union REGS regs;
	struct SREGS sregs;

	regs.h.ah = FINDFIRST;
	regs.x.cx = 0;		/* attribute: normal files */
	regs.x.dx = FP_OFF(path);
	sregs.ds = FP_SEG(path);
	intdosx(&regs, &regs, &sregs);
	return !regs.x.cflag;
}

int
findnext() {
	union REGS regs;

	regs.h.ah = FINDNEXT;
	intdos(&regs, &regs);
	return !regs.x.cflag;
}

char *
foundfile_buffer()
{
	return (getdta() + 30);
}


/* Get disk transfer area */
static char *
getdta()
{
	union REGS regs;
	struct SREGS sregs;
	char *ret;

	regs.h.ah = GETDTA;
	intdosx(&regs, &regs, &sregs);
#   ifdef MK_FP
	ret = MK_FP(sregs.es, regs.x.bx);
#   else
	FP_OFF(ret) = regs.x.bx;
	FP_SEG(ret) = sregs.es;
#   endif
	return ret;
}

long
filesize(file)
char *file;
{
	char *dta;

	if (findfirst(file)) {
		dta = getdta();
		return  (* (long *) (dta + 26));
	} else
		return -1L;
}

#endif /* __GO32__ */

/*
 * Chdrive() changes the default drive.
 */
void
chdrive(str)
char *str;
{
#  define SELECTDISK	0x0E
	char *ptr;
	union REGS inregs;
	char drive;

	if ((ptr = index(str, ':')) != NULL) {
		drive = toupper(*(ptr - 1));
		inregs.h.ah = SELECTDISK;
		inregs.h.dl = drive - 'A';
		intdos(&inregs, &inregs);
	}
	return;
}


/* Use the IOCTL DOS function call to change stdin and stdout to raw
 * mode.  For stdin, this prevents MSDOS from trapping ^P, thus
 * freeing us of ^P toggling 'echo to printer'.
 * Thanks to Mark Zbikowski (markz@microsoft.UUCP).
 */

#define DEVICE		0x80
#define RAW		0x20
#define IOCTL		0x44
#define STDIN		fileno(stdin)
#define STDOUT		fileno(stdout)
#define GETBITS		0
#define SETBITS		1

static unsigned	int old_stdin, old_stdout;

void
disable_ctrlP()
{

	if (!flags.rawio) return;

    old_stdin = dos_ioctl(STDIN, GETBITS, 0);
    old_stdout = dos_ioctl(STDOUT, GETBITS, 0);
	if (old_stdin & DEVICE)
        dos_ioctl(STDIN, SETBITS, old_stdin | RAW);
	if (old_stdout & DEVICE)
        dos_ioctl(STDOUT, SETBITS, old_stdout | RAW);
	return;
}

void
enable_ctrlP()
{
	if (!flags.rawio) return;
	if (old_stdin)
        (void) dos_ioctl(STDIN, SETBITS, old_stdin);
	if (old_stdout)
        (void) dos_ioctl(STDOUT, SETBITS, old_stdout);
	return;
}

static unsigned int
dos_ioctl(handle, mode, setvalue)
int handle, mode;
unsigned setvalue;
{
	union REGS regs;

	regs.h.ah = IOCTL;
	regs.h.al = mode;
	regs.x.bx = handle;
	regs.h.dl = setvalue;
	regs.h.dh = 0;			/* Zero out dh */
	intdos(&regs, &regs);
	return (regs.x.dx);
}

#endif /* OVL0 */
#ifdef OVLB

void
get_scr_size()
{
	union REGS regs;

	if (!flags.BIOS) {		/* assume standard screen size */
		CO = 80;
		LI = 24;
		return;
	}

	regs.x.ax = FONTINFO;
	regs.x.bx = 0;			/* current ROM BIOS font */
	regs.h.dl = 24;			/* default row count */
					/* in case no EGA/MCGA/VGA */
	int86(VIDEO_BIOS, &regs, &regs); /* Get Font Information */

	/* MDA/CGA/PCjr ignore INT 10h, Function 11h, but since we
	 * cleverly loaded up DL with the default, everything's fine.
	 *
	 * Otherwise, DL now contains rows - 1.  Also, CX contains the
	 * points (bytes per character) and ES:BP points to the font
	 * table.  -3.
	 */

	regs.h.ah = GETMODE;
	int86(VIDEO_BIOS, &regs, &regs); /* Get Video Mode */

	/* This goes back all the way to the original PC.  Completely
	 * safe.  AH contains # of columns, AL contains display mode,
	 * and BH contains the active display page.
	 */

	LI = regs.h.dl + 1;
	CO = regs.h.ah;
}

#endif /* OVLB */
#ifdef OVL0

void
gotoxy(x,y)
int x,y;
{
	union REGS regs;

	x--; y--;			/* (0,0) is upper right corner */

	regs.h.ah = SETCURPOS;
	regs.h.bh = 0;			/* display page */
	regs.h.dh = y;			/* row */
	regs.h.dl = x;			/* column */
	int86(VIDEO_BIOS, &regs, &regs); /* Set Cursor Position */

	/* This, too, goes back all the way to the original PC.  If
	 * we ever get so fancy as to swap display pages (i doubt it),
	 * then we'll need to set BH appropriately.  This function
	 * returns nothing.  -3.
	 */
}

#endif /* OVL0 */

#endif /* MSDOS */
