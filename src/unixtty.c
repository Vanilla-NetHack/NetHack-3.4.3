/*	SCCS Id: @(#)unixtty.c	3.0	88/05/03
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* tty.c - (Unix) version */

/* With thanks to the people who sent code for SYSV - hpscdi!jon,
 * arnold@ucsf-cgl, wcs@bo95b, cbcephus!pds and others.
 */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#define ONAMES_H
#define NEED_VARARGS
#include "hack.h"

/*
 * The distinctions here are not BSD - rest but rather USG - rest, as
 * BSD still has the old sgttyb structure, but SYSV has termio. Thus:
 */
#if defined(BSD) || defined(ULTRIX)
#define	V7
#else
#define USG
#endif


#ifdef USG

#include	<termio.h>
#define termstruct	termio
#define kill_sym	c_cc[VKILL]
#define erase_sym	c_cc[VERASE]
#define intr_sym	c_cc[VINTR]
#define EXTABS		TAB3
#define tabflgs		c_oflag
#define echoflgs	c_lflag
#define cbrkflgs	c_lflag
#define CBRKMASK	ICANON
#define CBRKON		! /* reverse condition */
#define OSPEED(x)	((x).c_cflag & CBAUD)
#define inputflags	c_iflag
#define STRIPHI		ISTRIP
#define GTTY(x)		(ioctl(0, TCGETA, x))
/* STTY now modified to run under Sys V R3.	- may have to be #ifdef'ed */
#define STTY(x)		(ioctl(0, TCSETAW, x))	/* TCSETAF? TCSETAW? */
#define GTTY2(x)	1
#define STTY2(x)	1
#define nonesuch	0
#define inittyb2	inittyb
#define curttyb2	curttyb

#else	/* V7 */

#include	<sgtty.h>
#define termstruct	sgttyb
#define	kill_sym	sg_kill
#define	erase_sym	sg_erase
#define	intr_sym	t_intrc
#define EXTABS		XTABS
#define tabflgs		sg_flags
#define echoflgs	sg_flags
#define cbrkflgs	sg_flags
#define CBRKMASK	CBREAK
#define CBRKON		/* empty */
#define inputflags	sg_flags	/* don't know how enabling meta bits */
#define STRIPHI		0		/* should actually be done on BSD */
#define OSPEED(x)	(x).sg_ospeed
#define GTTY(x)		(gtty(0, x))
#define STTY(x)		(stty(0, x))
#define GTTY2(x)	(ioctl(0, TIOCGETC, (char *)x))
#define STTY2(x)	(ioctl(0, TIOCSETC, (char *)x))
#define nonesuch	-1
struct tchars inittyb2, curttyb2;

#endif

extern short ospeed;
char erase_char, intr_char, kill_char;
static boolean settty_needed = FALSE;
struct termstruct inittyb, curttyb;

static void
setctty(){
	if(STTY(&curttyb) < 0 || STTY2(&curttyb2) < 0)
		perror("NetHack (setctty)");
}

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty(){
	if(GTTY(&inittyb) < 0 || GTTY2(&inittyb2) < 0)
		perror("NetHack (gettty)");
	curttyb = inittyb;
	curttyb2 = inittyb2;
	ospeed = OSPEED(inittyb);
	erase_char = inittyb.erase_sym;
	kill_char = inittyb.kill_sym;
	intr_char = inittyb2.intr_sym;
	getioctls();

	/* do not expand tabs - they might be needed inside a cm sequence */
	if(curttyb.tabflgs & EXTABS) {
		curttyb.tabflgs &= ~EXTABS;
		setctty();
	}
	settty_needed = TRUE;
}

/* reset terminal to original state */
void
settty(s)
char *s;
{
	clear_screen();
	end_screen();
	if(s) Printf(s);
	(void) fflush(stdout);
	if(STTY(&inittyb) < 0 || STTY2(&inittyb2) < 0)
		perror("NetHack (settty)");
	flags.echo = (inittyb.echoflgs & ECHO) ? ON : OFF;
	flags.cbreak = (CBRKON(inittyb.cbrkflgs & CBRKMASK)) ? ON : OFF;
	curttyb.inputflags |= STRIPHI;
	setioctls();
}

void
setftty(){
register int ef = 0;			/* desired value of flags & ECHO */
#ifdef LINT	/* cf = CBRKON(CBRKMASK); const expr to initialize is ok */
register int cf = 0;
#else
register int cf = CBRKON(CBRKMASK);	/* desired value of flags & CBREAK */
#endif
register int change = 0;
	flags.cbreak = ON;
	flags.echo = OFF;
	/* Should use (ECHO|CRMOD) here instead of ECHO */
	if((curttyb.echoflgs & ECHO) != ef){
		curttyb.echoflgs &= ~ECHO;
/*		curttyb.echoflgs |= ef;					*/
		change++;
	}
	if((curttyb.cbrkflgs & CBRKMASK) != cf){
		curttyb.cbrkflgs &= ~CBRKMASK;
		curttyb.cbrkflgs |= cf;
#ifdef USG
		/* be satisfied with one character; no timeout */
		curttyb.c_cc[VMIN] = 1;		/* was VEOF */
		curttyb.c_cc[VTIME] = 0;	/* was VEOL */
#endif
		change++;
	}
	curttyb.inputflags &=~ STRIPHI;
	/* If an interrupt character is used, it will be overriden and
	 * set to ^C.
	 */
	if(intr_char != nonesuch && curttyb2.intr_sym != '\003') {
	    curttyb2.intr_sym = '\003';
	    change++;
	}

	if(change) setctty();
	start_screen();
}

void
intron() {		/* enable kbd interupts if enabled when game started */

	if(intr_char != nonesuch && curttyb2.intr_sym != '\003') {
	    curttyb2.intr_sym = '\003';
	    setctty();
	}
}

void
introff() {		/* disable kbd interrupts if required*/

	if(curttyb2.intr_sym != nonesuch) {
	    curttyb2.intr_sym = nonesuch;
	    setctty();
	}
}


/* fatal error */
/*VARARGS1*/

void
error VA_DECL(char *,s)
	VA_START(s);
	VA_INIT(s, char *);
	if(settty_needed)
		settty(NULL);
	Vprintf(s,VA_ARGS);
	(void) putchar('\n');
	VA_END();
	exit(1);
}
