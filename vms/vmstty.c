/*	SCCS Id: @(#)vmstty.c	3.0	88/05/03
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* tty.c - (VMS) version */

#define NEED_VARARGS
#include "hack.h"

#include	<descrip.h>
#include	<iodef.h>
#include	<smgdef.h>
#include	<ttdef.h>
#include <errno.h>

#define vms_ok(sts) ((sts)&1)
#define META(c)  ((c)|0x80)	/*(Same as DOS's M(c).)*/
#define CTRL(c)  ((c)&0x1F)
#define CMASK(c) (1<<CTRL(c))
#define LIB$M_CLI_CTRLT CMASK('T')	/* 0x00100000 */
#define LIB$M_CLI_CTRLY CMASK('Y')	/* 0x02000000 */

extern short ospeed;
char erase_char, intr_char, kill_char;
static boolean settty_needed = FALSE,  bombing = FALSE;
#ifndef MAIL
static	    /* else global ('extern' in mail.c) */
#endif
       unsigned long pasteboard_id = 0; /* for AST & broadcast-msg handling */
static unsigned long kb = 0;

int
vms_getchar()
{
    static volatile int recurse = 0;	/* SMG is not AST re-entrant! */
    short key;

    if (recurse++ == 0 && kb != 0) {
	SMG$READ_KEYSTROKE(&kb, &key);
	switch (key)
	{
	  case SMG$K_TRM_UP:
	    key = 'k';
	    break;
	  case SMG$K_TRM_DOWN:
	    key = 'j';
	    break;
	  case SMG$K_TRM_LEFT:
	    key = 'h';
	    break;
	  case SMG$K_TRM_RIGHT:
	    key = 'l';
	    break;
	  case '\r':
	    key = '\n';
	    break;
	  default:
	    if (key == '\007' || key == '\032' || key > 255)
		key = '\033';
	    break;
	}
    } else {
	/* abnormal input--either SMG didn't initialize properly or
	   vms_getchar() has been called recursively (via SIGINT handler).
	 */
	if (kb != 0)			/* must have been a recursive call */
	    SMG$CANCEL_INPUT(&kb);	/*  from an interrupt handler	   */
	key = getchar();
    }
    --recurse;
    return (int)key;
}

#define TT_SPECIAL_HANDLING (TT$M_MECHTAB|TT$M_MECHFORM)
#define Uword unsigned short
#define Ubyte unsigned char
struct _sm_iosb {		/* i/o status block for sense-mode qio */
	Uword	  status;
	Ubyte	  xmt_speed,  rcv_speed;
	Ubyte	  cr_fill,  lf_fill,  parity;
	unsigned   : 8;
};
struct _sm_bufr {		/* sense-mode characteristics buffer */
	Ubyte	  class,  type;		/* class==DC$_TERM, type==(various) */
	Uword	  buf_siz;		/* aka page width */
#define page_width buf_siz		/* number of columns */
	unsigned  tt_char  : 24;	/* primary characteristics */
	Ubyte	  page_length;		/* number of lines */
	unsigned  tt2_char : 32;	/* secondary characteristics */
};
static struct {
    struct _sm_iosb io;
    struct _sm_bufr sm;
} sg = {{0},{0}};
static unsigned short tt_chan = 0;
static unsigned long  tt_char_restore = 0, tt_char_active = 0;
static unsigned long  ctrl_mask = 0;

static void
setctty(){
    struct _sm_iosb iosb;
    long status = SYS$QIOW(0, tt_chan, IO$_SETMODE, &iosb, (void(*)())0, 0,
			   &sg.sm, sizeof sg.sm, 0, 0, 0, 0);
    if (vms_ok(status))  status = iosb.status;
    if (!vms_ok(status)) {
	errno = EVMSERR,  vaxc$errno = status;
	perror("NetHack (setctty: setmode)");
    }
}

static void
resettty(){			/* atexit() routine */
    if (settty_needed) {
	bombing = TRUE;     /* don't clear screen; preserve traceback info */
	settty((char *)NULL);
    }
    (void) SYS$DASSGN(tt_chan),  tt_chan = 0;
}

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty(){
    long status;
    $DESCRIPTOR(input_dsc, "TT");
    unsigned long zero = 0;

    if (tt_chan == 0) {		/* do this stuff once only */
	status = SYS$ASSIGN(&input_dsc, &tt_chan, 0, 0);
	if (!vms_ok(status)) {
	    errno = EVMSERR,  vaxc$errno = status;
	    perror("NetHack (gettty: $assign)");
	}
	atexit(resettty);   /* register an exit handler to reset things */
    }
    status = SYS$QIOW(0, tt_chan, IO$_SENSEMODE, &sg.io, (void(*)())0, 0,
		      &sg.sm, sizeof sg.sm, 0, 0, 0, 0);
    if (vms_ok(status))  status = sg.io.status;
    if (!vms_ok(status)) {
	errno = EVMSERR,  vaxc$errno = status;
	perror("NetHack (gettty: sensemode)");
    }
    ospeed = sg.io.xmt_speed;
    erase_char = '\177';	/* <rubout>, aka <delete> */
    kill_char = CTRL('U');
    intr_char = CTRL('C');
    (void) LIB$ENABLE_CTRL(&zero, &ctrl_mask);
    /* Use the systems's values for lines and columns if it has any idea. */
    if (sg.sm.page_length)
	LI = sg.sm.page_length;
    if (sg.sm.page_width)
	CO = sg.sm.page_width;
    /* Determine whether TTDRIVER is doing tab and/or form-feed expansion;
       if so, we want to suppress that but also restore it at final exit. */
    if ((sg.sm.tt_char & TT_SPECIAL_HANDLING) != TT_SPECIAL_HANDLING) {
	tt_char_restore = sg.sm.tt_char;
	tt_char_active	= sg.sm.tt_char |= TT_SPECIAL_HANDLING;
#if 0		/*[ defer until setftty() ]*/
	setctty();
#endif 0
    } else	/* no need to take any action */
	tt_char_restore = tt_char_active = 0;
}

/* reset terminal to original state */
void
settty(s)
char *s;
{
	if (!bombing) {
	    end_screen();
	    if(s) Printf(s);
	    (void) fflush(stdout);
	}
#ifdef MAIL	/* this is essential, or lib$spawn & lib$attach will fail */
	SMG$DISABLE_BROADCAST_TRAPPING(&pasteboard_id);
#endif
#if 0		/* let SMG's exit handler do the cleanup (as per doc) */
	SMG$DELETE_PASTEBOARD(&pasteboard_id);
	SMG$DELETE_VIRTUAL_KEYBOARD(&kb),  kb = 0;
#endif 0
	if (ctrl_mask)
	    (void) LIB$ENABLE_CTRL(&ctrl_mask, 0);
	flags.echo = ON;
	flags.cbreak = OFF;
	if (tt_char_restore != 0) {
	    sg.sm.tt_char = tt_char_restore;
	    setctty();
	}
	settty_needed = FALSE;
}

#ifdef MAIL
static void
broadcast_ast(dummy)
{
	extern volatile int broadcasts;

	broadcasts++;
}
#endif

void
setftty(){
	unsigned int mask = LIB$M_CLI_CTRLT | LIB$M_CLI_CTRLY;

	flags.cbreak = ON;
	flags.echo = OFF;
	(void) LIB$DISABLE_CTRL(&mask, 0);
	if (kb == 0) {		/* do this stuff once only */
	SMG$CREATE_VIRTUAL_KEYBOARD(&kb);
	SMG$CREATE_PASTEBOARD(&pasteboard_id, 0, 0, 0, 0);
	}
#ifdef MAIL
	/* note side effect: also intercepts hangup notification */
	SMG$SET_BROADCAST_TRAPPING(&pasteboard_id, broadcast_ast, 0);
#endif
	/* disable tab & form-feed expansion */
	if (tt_char_active != 0) {
	    sg.sm.tt_char = tt_char_active;
	    setctty();
	}
	start_screen();
	settty_needed = TRUE;
}


void
intron() {		/* enable kbd interupts if enabled when game started */
}

void
introff() {		/* disable kbd interrupts if required*/
}


/* fatal error */
/*VARARGS1*/
void
error VA_DECL(const char *,s)
	VA_START(s);
	VA_INIT(s, const char *);
	if(settty_needed)
		settty(NULL);
	Vprintf(s,VA_ARGS);
	(void) putchar('\n');
	VA_END();
	exit(1);
}
