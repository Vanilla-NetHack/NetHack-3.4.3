/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.tty.c - version 1.0.2 */

#include	"hack.h"
#include	<stdio.h>
#include	<sgtty.h>
#include	<ctype.h>	/* for isprint() */

struct sgttyb inittyb, curttyb;
extern short ospeed;
char erase_char, kill_char;
static boolean settty_needed = FALSE;

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
gettty(){
	(void) gtty(0, &inittyb);
	(void) gtty(0, &curttyb);
	ospeed = inittyb.sg_ospeed;
	erase_char = inittyb.sg_erase;
	kill_char = inittyb.sg_kill;
	getioctls();

	/* do not expand tabs - they might be needed inside a cm sequence */
	if(curttyb.sg_flags & XTABS) {
		curttyb.sg_flags &= ~XTABS;
		setctty();
	}
	settty_needed = TRUE;
}

/* fatal error */
/*VARARGS1*/
error(s,x,y) char *s; {
	if(settty_needed)
		settty((char *) 0);
	printf(s,x,y);
	putchar('\n');
	exit(1);
}

/* reset terminal to original state */
settty(s) char *s; {
	clear_screen();
	end_screen();
	if(s) printf(s);
	(void) fflush(stdout);
	if(stty(0, &inittyb) == -1)
		puts("Cannot change tty");
	flags.echo = (inittyb.sg_flags & ECHO) ? ON : OFF;
	flags.cbreak = (inittyb.sg_flags & CBREAK) ? ON : OFF;
	setioctls();
}

setctty(){
	if(stty(0, &curttyb) == -1) puts("Cannot change tty");
}


setftty(){
	flags.cbreak = ON;
	flags.echo = OFF;
	setxtty();
}

setxtty(){
register int ef = (flags.echo == ON) ? ECHO : 0;
register int cf = (flags.cbreak == ON) ? CBREAK : 0;
register int change = 0;
	/* Should use (ECHO|CRMOD) here instead of ECHO */
	if((curttyb.sg_flags & ECHO) != ef){
		curttyb.sg_flags &= ~ECHO;
		curttyb.sg_flags |= ef;
		change++;
	}
	if((curttyb.sg_flags & CBREAK) != cf){
		curttyb.sg_flags &= ~CBREAK;
		curttyb.sg_flags |= cf;
		change++;
	}
	if(change){
		setctty();
	}
	start_screen();
}

/*
 * Read a line closed with '\n' into the array char bufp[BUFSZ].
 * (The '\n' is not stored. The string is closed with a '\0'.)
 * Reading can be interrupted by an escape ('\033') - now the
 * resulting string is "\033".
 */
getlin(bufp)
register char *bufp;
{
	register char *obufp = bufp;
	register int c;

	flags.toplin = 2;		/* nonempty, no --More-- required */
	for(;;) {
		(void) fflush(stdout);
		if((c = getchar()) == EOF) {
			*bufp = 0;
			return;
		}
		if(c == '\033') {
			*obufp = c;
			obufp[1] = 0;
			return;
		}
		if(c == erase_char || c == '\b') {
			if(bufp != obufp) {
				bufp--;
				putstr("\b \b"); /* putsym converts \b */
			} else	bell();
		} else if(c == '\n') {
			*bufp = 0;
			return;
		} else if(c == kill_char || c == '\177') { /* Robert Viduya */
			while(bufp != obufp) {
				bufp--;
				putstr("\b \b");
			}
		} else if(isprint(c)) {
			*bufp = c;
			bufp[1] = 0;
			putstr(bufp);
			if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
				bufp++;
		} else
			bell();
	}
}

getret() {
	xgetret("");
}

cgetret(s)
register char *s;
{
	xgetret(s);
}

xgetret(s)
register char *s;
{
	putsym('\n');
	if(flags.standout)
		standoutbeg();
	putstr("Hit ");
	putstr(flags.cbreak ? "space" : "return");
	putstr(" to continue: ");
	if(flags.standout)
		standoutend();
	xwaitforspace(s);
}

char morc;	/* tell the outside world what char he used */

xwaitforspace(s)
register char *s;	/* chars allowed besides space or return */
{
register int c;

	(void) fflush(stdout);
	morc = 0;

	while((c = getchar()) != '\n') {
	    if(c == EOF)
		end_of_input();
	    if(flags.cbreak) {
		if(c == ' ') break;
		if(s && index(s,c)) {
			morc = c;
			break;
		}
		bell();		/* useless if !cbreak */
	    }
	}
}

char *
parse()
{
	static char inline[COLNO];
	register foo;

	flags.move = 1;
	if(!Invis) curs_on_u(); else home();
	(void) fflush(stdout);
	while((foo = getchar()) >= '0' && foo <= '9')
		multi += 10*multi+foo-'0';
	if(multi) {
		multi--;
		save_cm = inline;
	}
	inline[0] = foo;
	inline[1] = 0;
	if(foo == EOF)
		end_of_input();
	if(foo == 'f' || foo == 'F'){
		inline[1] = getchar();
#ifdef QUEST
		if(inline[1] == foo) inline[2] = getchar(); else
#endif QUEST
		inline[2] = 0;
	}
	if(foo == 'm' || foo == 'M'){
		inline[1] = getchar();
		inline[2] = 0;
	}
	clrlin();
	return(inline);
}

char
readchar() {
	register int sym;
	(void) fflush(stdout);
	if((sym = getchar()) == EOF)
		end_of_input();
	if(flags.toplin == 1) flags.toplin = 2;
	return((char) sym);
}

end_of_input()
{
	settty("End of input?\n");
	clearlocks();
	exit(0);
}

