/*	SCCS Id: @(#)pctty.c	2.3	87/12/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* tty.c - (PC) version */

/* With thanks to the people who sent code for SYSV - hpscdi!jon,
 * arnold@ucsf-cgl, wcs@bo95b, cbcephus!pds and others.
 */

#include <stdio.h>
#include "hack.h"
#include "func_tab.h"

extern void savech();

static char erase_char, kill_char;

/*
 * Get initial state of terminal, set ospeed (for termcap routines)
 * and switch off tab expansion if necessary.
 * Called by startup() in termcap.c and after returning from ! or ^Z
 */
gettty(){
	erase_char = '\b';
	kill_char = 21;		/* cntl-U */
	flags.cbreak = TRUE;
#ifdef DGK
	disable_ctrlP();	/* turn off ^P processing */
#endif
}

/* reset terminal to original state */
settty(s) char *s; {
	end_screen();
	if(s) printf(s);
	(void) fflush(stdout);
#ifdef DGK
	enable_ctrlP();		/* turn on ^P processing */
#endif
}


/* fatal error */
/*VARARGS1*/
error(s,x,y) char *s; {
	end_screen();
	putchar('\n');
	printf(s,x,y);
	putchar('\n');
	exit(1);
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
		} else if(' ' <= c && c < '\177') {
				/* avoid isprint() - some people don't have it
				   ' ' is not always a printing char */
			*bufp = c;
			bufp[1] = 0;
			putstr(bufp);
			if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
				bufp++;
		} else if(c == kill_char || c == '\177') { /* Robert Viduya */
				/* this test last - @ might be the kill_char */
			while(bufp != obufp) {
				bufp--;
				putstr("\b \b");
			}
		} else
			bell();
	}
}

getret() {
	cgetret("");
}

cgetret(s)
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

	morc = 0;
	while((c = readchar()) != '\n') {
	    if(flags.cbreak) {
		if(c == ' ') break;
		if(s && index(s,c)) {
			morc = c;
			break;
		}
		bell();
	    }
	}
}

static int last_multi;

char *
parse()
{
	static char inline[COLNO];
	register foo;

	flags.move = 1;
	if(!Invisible) curs_on_u(); else home();
	multi = 0;
#ifdef DGK
	while((foo = readchar()) >= '0' && foo <= '9') {
		multi = 10*multi+foo-'0';
		if (multi < 0 || multi > LARGEST_INT)
			multi = LARGEST_INT;
		if (multi > 9) {
			remember_topl();
			home();
			cl_end();
			printf("Count: %d", multi);
		}
		last_multi = multi;
	}
# ifdef REDO
	if (foo == DOAGAIN || in_doagain)
		multi = last_multi;
	else {
		savech(0);	/* reset input queue */
		savech(foo);
	}
# endif

#else /* DGK */

	while((foo = readchar()) >= '0' && foo <= '9')
		multi = 10*multi+foo-'0';

#endif /* DGK */

	if(multi) {
		multi--;
		save_cm = inline;
	}
	inline[0] = foo;
	inline[1] = 0;
	if(foo == 'g' || foo == 'G'){
		inline[1] = getchar();
#ifdef REDO
		savech(inline[1]);
#endif
		inline[2] = 0;
	}
	if(foo == 'm' || foo == 'M'){
		inline[1] = getchar();
#ifdef REDO
		savech(inline[1]);
#endif
		inline[2] = 0;
	}
	clrlin();
	return(inline);
}

char
readchar() {
	register int sym;

	(void) fflush(stdout);
	sym = getchar();
	if(flags.toplin == 1)
		flags.toplin = 2;
	return((char) sym);
}
#ifdef COM_COMPL
/* Read in an extended command - doing command line completion for
 * when enough character have been entered to make a unique command.
 * This is just a modified getlin().   -jsb
 */
get_ext_cmd(bufp)
register char *bufp;
{
	register char *obufp = bufp;
	register int c;
	int com_index, index;

	flags.toplin = 2;		/* nonempty, no --More-- required */

	for(;;) {
		(void) fflush(stdout);
		if((c = readchar()) == EOF) {
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
		} else if(' ' <= c && c < '\177') {
				/* avoid isprint() - some people don't have it
				   ' ' is not always a printing char */
			*bufp = c;
			bufp[1] = 0;
			index = 0;
			com_index = -1;

			while(extcmdlist[index].ef_txt != (char *) 0){
				if(!strncmp(obufp, extcmdlist[index].ef_txt,
				strlen(obufp)))
					if(com_index == -1) /* No matches yet*/
					    com_index = index;
					else /* More than 1 match */
					    com_index = -2;
				index++;
			}
			if(com_index >= 0){
				strcpy(obufp,
				extcmdlist[com_index].ef_txt);
				/* finish print our string */
				putstr(bufp);
				bufp = obufp; /* reset it */
				if(strlen(obufp) < BUFSIZ-1 &&
				 strlen(obufp) < COLNO)
					/* set bufp at the end of our
					 * string
					 */
					bufp += strlen(obufp);
			} else {
				putstr(bufp);
				if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
					bufp++;
			}
		} else if(c == kill_char || c == '\177') { /* Robert Viduya */
				/* this test last - @ might be the kill_char */
			while(bufp != obufp) {
				bufp--;
				putstr("\b \b");
			}
		} else
			bell();
	}

}
#endif /* COM_COMPL /* */
