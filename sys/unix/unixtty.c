/*	SCCS Id: @(#)unixtty.c	3.1	90/22/02
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* tty.c - (Unix) version */

/* With thanks to the people who sent code for SYSV - hpscdi!jon,
 * arnold@ucsf-cgl, wcs@bo95b, cbcephus!pds and others.
 */

#define NEED_VARARGS
#include "hack.h"

/*
 * The distinctions here are not BSD - rest but rather USG - rest, as
 * BSD still has the old sgttyb structure, but SYSV has termio. Thus:
 */
#if (defined(BSD) || defined(ULTRIX)) && !defined(POSIX_TYPES)
#define	V7
#else
#define USG
#endif


#ifdef USG

# ifdef POSIX_TYPES
#include	<termios.h>
#include	<unistd.h>
#define termstruct	termios
# else
#include	<termio.h>
#  if defined(TCSETS) && !defined(AIX_31)
#define termstruct	termios
#  else
#define termstruct	termio
#  endif
# endif /* POSIX_TYPES */
#define kill_sym	c_cc[VKILL]
#define erase_sym	c_cc[VERASE]
#define intr_sym	c_cc[VINTR]
# ifdef TAB3	/* not a POSIX flag, but some have it anyway */
#define EXTABS		TAB3
# else
#define EXTABS		0
# endif
#define tabflgs		c_oflag
#define echoflgs	c_lflag
#define cbrkflgs	c_lflag
#define CBRKMASK	ICANON
#define CBRKON		! /* reverse condition */
# ifdef POSIX_TYPES
#define OSPEED(x)	(speednum(cfgetospeed(&x)))
# else
#  ifndef CBAUD
# define CBAUD		_CBAUD /* for POSIX nitpickers (like RS/6000 cc) */
#  endif
#define OSPEED(x)	((x).c_cflag & CBAUD)
# endif
#define IS_7BIT(x)	((x).c_cflag & CS7)
#define inputflags	c_iflag
#define STRIPHI		ISTRIP
# ifdef POSIX_TYPES
#  define GTTY(x)	(tcgetattr(0, x))
#  define STTY(x)	(tcsetattr(0, TCSADRAIN, x))
# else
#  if defined(TCSETS) && !defined(AIX_31)
#   define GTTY(x)	(ioctl(0, TCGETS, x))
#   define STTY(x)	(ioctl(0, TCSETSW, x))
#  else
#   define GTTY(x)	(ioctl(0, TCGETA, x))
#   define STTY(x)	(ioctl(0, TCSETAW, x))
#  endif
# endif /* POSIX_TYPES */
#define GTTY2(x)	1
#define STTY2(x)	1
# ifdef POSIX_TYPES
#  ifdef BSD
#   define nonesuch	_POSIX_VDISABLE
#  else
#   define nonesuch	(fpathconf(0, _PC_VDISABLE))
#  endif
# else
#  define nonesuch	0
# endif
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
#define IS_7BIT(x)	(FALSE)
#define STRIPHI		0		/* should actually be done on BSD */
#define OSPEED(x)	(x).sg_ospeed
#if defined(bsdi) || defined(__386BSD)
# define GTTY(x)	(ioctl(0, TIOCGETP, (char *)x))
# define STTY(x)	(ioctl(0, TIOCSETP, (char *)x))
#else
# define GTTY(x)	(gtty(0, x))
# define STTY(x)	(stty(0, x))
#endif
#define GTTY2(x)	(ioctl(0, TIOCGETC, (char *)x))
#define STTY2(x)	(ioctl(0, TIOCSETC, (char *)x))
#define nonesuch	-1
struct tchars inittyb2, curttyb2;

#endif

#if defined(TTY_GRAPHICS) && ((!defined(SYSV) && !defined(HPUX)) || defined(UNIXPC) || defined(SVR4))
# ifndef LINT
extern			/* it is defined in libtermlib (libtermcap) */
# endif
	short ospeed;	/* terminal baudrate; set by gettty */
#else
short	ospeed = 0;	/* gets around "not defined" error message */
#endif

#if defined(POSIX_TYPES) && defined(BSD)
unsigned
#endif
	char erase_char, intr_char, kill_char;
static boolean settty_needed = FALSE;
struct termstruct inittyb, curttyb;

#ifdef POSIX_TYPES
static int
speednum(speed)
speed_t speed;
{
	switch (speed) {
		case B0:	return 0;
		case B50:	return 1;
		case B75:	return 2;
		case B110:	return 3;
		case B134:	return 4;
		case B150:	return 5;
		case B200:	return 6;
		case B300:	return 7;
		case B600:	return 8;
		case B1200:	return 9;
		case B1800:	return 10;
		case B2400:	return 11;
		case B4800:	return 12;
		case B9600:	return 13;
		case B19200:	return 14;
		case B38400:	return 15;
	}

	return 0;
}
#endif

static void
setctty()
{
	if(STTY(&curttyb) < 0 || STTY2(&curttyb2) < 0)
		perror("NetHack (setctty)");
}

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
void
gettty()
{
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
const char *s;
{
	end_screen();
	if(s) raw_print(s);
	if(STTY(&inittyb) < 0 || STTY2(&inittyb2) < 0)
		perror("NetHack (settty)");
	flags.echo = (inittyb.echoflgs & ECHO) ? ON : OFF;
	flags.cbreak = (CBRKON(inittyb.cbrkflgs & CBRKMASK)) ? ON : OFF;
	curttyb.inputflags |= STRIPHI;
	setioctls();
}

void
setftty()
{
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
# ifdef POSIX_JOB_CONTROL
		/* turn off system suspend character
		 * due to differences in structure layout, this has to be
		 * here instead of in ioctl.c:getioctls() with the BSD
		 * equivalent
		 */
#  ifdef VSUSP	/* real POSIX */
		curttyb.c_cc[VSUSP] = nonesuch;
#  else		/* other later SYSV */
		curttyb.c_cc[VSWTCH] = nonesuch;
#  endif
# endif
# ifdef VDSUSP /* SunOS Posix extensions */
		curttyb.c_cc[VDSUSP] = nonesuch;
# endif
# ifdef VREPRINT
		curttyb.c_cc[VREPRINT] = nonesuch;
# endif
# ifdef VDISCARD
		curttyb.c_cc[VDISCARD] = nonesuch;
# endif
# ifdef VWERASE
		curttyb.c_cc[VWERASE] = nonesuch;
# endif
# ifdef VLNEXT
		curttyb.c_cc[VLNEXT] = nonesuch;
# endif
#endif
		change++;
	}
	if(!IS_7BIT(inittyb)) curttyb.inputflags &=~ STRIPHI;
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
intron()		/* enable kbd interupts if enabled when game started */
{
#ifdef TTY_GRAPHICS
	/* Ugly hack to keep from changing tty modes for non-tty games -dlc */
	if (!strcmp(windowprocs.name, "tty") &&
	    intr_char != nonesuch && curttyb2.intr_sym != '\003') {
	    curttyb2.intr_sym = '\003';
	    setctty();
	}
#endif
}

void
introff()		/* disable kbd interrupts if required*/
{
#ifdef TTY_GRAPHICS
	/* Ugly hack to keep from changing tty modes for non-tty games -dlc */
	if (!strcmp(windowprocs.name, "tty") &&
	   curttyb2.intr_sym != nonesuch) {
	    curttyb2.intr_sym = nonesuch;
	    setctty();
	}
#endif
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
