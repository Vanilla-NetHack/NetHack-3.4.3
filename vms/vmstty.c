/*	SCCS Id: @(#)vmstty.c	3.0	88/05/03
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* tty.c - (VMS) version */

#define NEED_VARARGS
#include "hack.h"

#include	<descrip.h>
#include	<iodef.h>
#include	<smgdef.h>

#define LIB$M_CLI_CTRLT 0x100000
#define LIB$M_CLI_CTRLY 0x2000000

extern short ospeed;
char erase_char, intr_char, kill_char;
static boolean settty_needed = FALSE;
static unsigned int kb = 0;

int
vms_getchar()
{
    short key;

    if (kb)
    {
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
    }
    else
	key = getchar();
    return key;
}

static struct sensemode {
    short status;
    unsigned char xmit_baud;
    unsigned char rcv_baud;
    unsigned char crfill;
    unsigned char lffill;
    unsigned char parity;
    unsigned char unused;
    char class;
    char type;
    short scr_wid;
    unsigned long tt_char: 24, scr_len: 8;
    unsigned long tt2_char;
} sg;
unsigned int ctrl_mask;

static void
setctty(){
}

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty(){
    int status;
    int input_chan;
    $DESCRIPTOR (input_dsc, "TT");
    unsigned int zero = 0;

    if (!(SYS$ASSIGN (&input_dsc, &input_chan, 0, 0) & 1))
	perror("NetHack (gettty)");
    status = SYS$QIOW(0, input_chan, IO$_SENSEMODE, &sg, 0, 0, &sg.class, 12,
		      0, 0, 0, 0);
    SYS$DASSGN (input_chan);
    if (!(status & 1))
	perror("NetHack (gettty)");
    ospeed = sg.xmit_baud;
    erase_char = '\177';
    kill_char = '\025';
    intr_char = '\003';
    (void) LIB$ENABLE_CTRL(&zero, &ctrl_mask);
    /* Use the systems's values for lines and columns if it has any idea. */
    if (sg.scr_len)
	LI = sg.scr_len;
    if (sg.scr_wid)
	CO = sg.scr_wid;
    settty_needed = TRUE;
}

#ifdef MAIL
unsigned long pasteboard_id = 0;
#endif

/* reset terminal to original state */
void
settty(s)
char *s;
{
	clear_screen();
	end_screen();
	if(s) Printf(s);
	(void) fflush(stdout);
	SMG$DELETE_VIRTUAL_KEYBOARD(&kb);
#ifdef MAIL
	SMG$DELETE_PASTEBOARD(&pasteboard_id);
#endif
	if (ctrl_mask)
	    (void) LIB$ENABLE_CTRL(&ctrl_mask, 0);
	flags.echo = ON;
	flags.cbreak = OFF;
}

#ifdef MAIL
static void
broadcast_ast(dummy)
{
	extern int broadcasts;

	broadcasts++;
}
#endif

void
setftty(){
	unsigned int mask = LIB$M_CLI_CTRLT | LIB$M_CLI_CTRLY;

	flags.cbreak = ON;
	flags.echo = OFF;
	(void) LIB$DISABLE_CTRL(&mask, 0);
	SMG$CREATE_VIRTUAL_KEYBOARD(&kb);
#ifdef MAIL
	SMG$CREATE_PASTEBOARD(&pasteboard_id, 0, 0, 0, 0);
	SMG$SET_BROADCAST_TRAPPING(&pasteboard_id, broadcast_ast, 0);
#endif
	start_screen();
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
error VA_DECL(char *,s)
	VA_START(s);
	VA_INIT(char *,s);
	if(settty_needed)
		settty(NULL);
	Vprintf(s,VA_ARGS);
	(void) putchar('\n');
	VA_END();
	exit(1);
}
