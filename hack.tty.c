/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
#include	<stdio.h>
#include	<sgtty.h>

struct sgttyb inittyb, curttyb;
extern short ospeed;


gettty(){
	(void) gtty(0, &inittyb);
	(void) gtty(0, &curttyb);
	ospeed = inittyb.sg_ospeed;
/*
	if(ospeed <= B300) flags.oneline = 1;
*/
	getioctls();
	xtabs();
}

/* reset terminal to original state */
settty(s) char *s; {
	clear_screen();
	if(s) printf(s);
	(void) fflush(stdout);
	if(stty(0, &inittyb) == -1) puts("Cannot change tty");
	flags.echo = (inittyb.sg_flags & ECHO) ? ON : OFF;
	flags.cbreak = (inittyb.sg_flags & CBREAK) ? ON : OFF;
	setioctls();
}

setctty(){
	if(stty(0, &curttyb) == -1) puts("Cannot change tty");
}

setftty(){
register int ef = (flags.echo == ON) ? ECHO : 0;
register int cf = (flags.cbreak == ON) ? CBREAK : 0;
register int change = 0;
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
}

echo(n)
register n;
{

	/* (void) gtty(0,&curttyb); */
	if(n == ON)
		curttyb.sg_flags |= ECHO;
	else
		curttyb.sg_flags &= ~ECHO;
	setctty();
}

/* always want to expand tabs, or to send a clear line char before
   printing something on topline */
xtabs()
{

	/* (void) gtty(0, &curttyb); */
	curttyb.sg_flags |= XTABS;
	setctty();
}

#ifdef LONG_CMD
cbreak(n)
register n;
{

	/* (void) gtty(0,&curttyb); */
	if(n == ON)
		curttyb.sg_flags |= CBREAK;
	else
		curttyb.sg_flags &= ~CBREAK;
	setctty();
}
#endif LONG_CMD

getlin(bufp)
register char *bufp;
{
	register char *obufp = bufp;
	register int c;

	flags.topl = 2;		/* nonempty, no --More-- required */
	for(;;) {
		(void) fflush(stdout);
		if((c = getchar()) == EOF) {
			*bufp = 0;
			return;
		}
		if(c == '\b') {
			if(bufp != obufp) {
				bufp--;
				putstr("\b \b"); /* putsym converts \b */
			} else	bell();
		} else if(c == '\n') {
			*bufp = 0;
			return;
		} else {
			*bufp = c;
			bufp[1] = 0;
			putstr(bufp);
			if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
				bufp++;
		}
	}
}

getret() {
	xgetret(TRUE);
}

cgetret() {
	xgetret(FALSE);
}

xgetret(spaceflag)
boolean spaceflag;	/* TRUE if space (return) required */
{
	printf("\nHit %s to continue: ",
		flags.cbreak ? "space" : "return");
	xwaitforspace(spaceflag);
}

char morc;	/* tell the outside world what char he used */

xwaitforspace(spaceflag)
boolean spaceflag;
{
register int c;

	(void) fflush(stdout);
	morc = 0;

	while((c = getchar()) != '\n') {
	    if(c == EOF) {
		settty("End of input?\n");
		exit(0);
	    }
	    if(flags.cbreak) {
		if(c == ' ') break;
		if(!spaceflag && letter(c)) {
			morc = c;
			break;
		}
  }
	}
}

char *
parse()
{
	static char inline[COLNO];
	register foo;

	flags.move = 1;
	if(!Invis) curs(u.ux,u.uy+2); else home();
	(void) fflush(stdout);
	while((foo = getchar()) >= '0' && foo <= '9')
		multi += 10*multi+foo-'0';
	if(multi) {
		multi--;
		save_cm = inline;
	}
	inline[0] = foo;
	inline[1] = 0;
	if(foo == EOF) {
		settty("End of input?\n");
		exit(0);
	}
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
	if((sym = getchar()) == EOF) {
		settty("End of input?\n");
		exit(0);
	}
	if(flags.topl == 1) flags.topl = 2;
	return((char) sym);
}
