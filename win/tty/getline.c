/*	SCCS Id: @(#)getline.c	3.1	92/01/05	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "wintty.h"
#include "func_tab.h"

#ifdef OVL1

char morc = 0;	/* tell the outside world what char you chose */

#endif /* OVL1 */

extern char erase_char, kill_char;	/* from appropriate tty.c file */

#ifdef OVL1

/*
 * Read a line closed with '\n' into the array char bufp[BUFSZ].
 * (The '\n' is not stored. The string is closed with a '\0'.)
 * Reading can be interrupted by an escape ('\033') - now the
 * resulting string is "\033".
 */
void
tty_getlin(query, bufp)
const char *query;
register char *bufp;
{
	register char *obufp = bufp;
	register int c;
	struct WinDesc *cw = wins[WIN_MESSAGE];
	boolean doprev = 0;

	if(ttyDisplay->toplin == 1 && !(cw->flags & WIN_STOP)) more();
	cw->flags &= ~WIN_STOP;
	ttyDisplay->toplin = 3; /* special prompt state */
	ttyDisplay->inread++;
	pline("%s ", query);
	for(;;) {
		(void) fflush(stdout);
		if((c = Getchar()) == EOF) {
			*bufp = 0;
			break;
		}
		if(c == '\033') {
			*obufp = c;
			obufp[1] = 0;
			break;
		}
		if(c == '\020') { /* ctrl-P */
		    if(!doprev)
			(void) tty_doprev_message(); /* need two initially */
		    (void) tty_doprev_message();
		    doprev = 1;
		    continue;
		} else if(doprev) {
		    tty_clear_nhwindow(WIN_MESSAGE);
		    cw->maxcol = cw->maxrow;
		    doprev = 0;
		    addtopl(query);
		    addtopl(" ");
		    *bufp = 0;
		    addtopl(obufp);
		}
		if(c == erase_char || c == '\b') {
			if(bufp != obufp) {
				bufp--;
				putsyms("\b \b");/* putsym converts \b */
			} else	tty_nhbell();
#if defined(apollo)
		} else if(c == '\n' || c == '\r') {
#else
		} else if(c == '\n') {
#endif
			*bufp = 0;
			break;
		} else if(' ' <= c && c < '\177' && 
			    (bufp-obufp < BUFSZ-1 || bufp-obufp < COLNO)) {
				/* avoid isprint() - some people don't have it
				   ' ' is not always a printing char */
			*bufp = c;
			bufp[1] = 0;
			putsyms(bufp);
			bufp++;
		} else if(c == kill_char || c == '\177') { /* Robert Viduya */
				/* this test last - @ might be the kill_char */
			while(bufp != obufp) {
				bufp--;
				putsyms("\b \b");
			}
		} else
			tty_nhbell();
	}
	ttyDisplay->toplin = 2;		/* nonempty, no --More-- required */
	ttyDisplay->inread--;
}

void
xwaitforspace(s)
register const char *s;	/* chars allowed besides space or return */
{
    register int c;

    morc = 0;

    while((c = tty_nhgetch()) != '\n') {
	if(flags.cbreak) {
	    if(c == ' ') break;
	    if(s && index(s,c)) {
		morc = c;
		break;
	    }
	    tty_nhbell();
	}
    }

}

#endif /* OVL1 */
#ifdef OVL2

#ifdef COM_COMPL
/* Read in an extended command - doing command line completion for
 * when enough characters have been entered to make a unique command.
 * This is just a modified getlin().   -jsb
 */
void
tty_get_ext_cmd(bufp)
register char *bufp;
{
	register char *obufp = bufp;
	register int c;
	int com_index, oindex;

	pline("# ");
	ttyDisplay->toplin = 2;		/* nonempty, no --More-- required */

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
				putsyms("\b \b"); /* putsym converts \b */
			} else	tty_nhbell();
#if defined(apollo)
		} else if(c == '\n' || c == '\r') {
#else
		} else if(c == '\n') {
#endif
			*bufp = 0;
			return;
		} else if(' ' <= c && c < '\177') {
				/* avoid isprint() - some people don't have it
				   ' ' is not always a printing char */
			*bufp = c;
			bufp[1] = 0;
			oindex = 0;
			com_index = -1;

			while(extcmdlist[oindex].ef_txt != NULL){
				if(!strncmpi(obufp, extcmdlist[oindex].ef_txt,
				    strlen(obufp)))
					if(com_index == -1) /* No matches yet*/
					    com_index = oindex;
					else /* More than 1 match */
					    com_index = -2;
				oindex++;
			}
			if(com_index >= 0){
				Strcpy(obufp, extcmdlist[com_index].ef_txt);
				/* finish printing our string */
				putsyms(bufp);
				bufp = obufp; /* reset it */
				if((int)strlen(obufp) < BUFSZ-1 &&
						(int)strlen(obufp) < COLNO)
					/* set bufp at the end of our string */
					bufp += strlen(obufp);
			} else {
				putsyms(bufp);
				if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
					bufp++;
			}
		} else if(c == kill_char || c == '\177') { /* Robert Viduya */
				/* this test last - @ might be the kill_char */
			while(bufp != obufp) {
				bufp--;
				putsyms("\b \b");
			}
		} else
			tty_nhbell();
	}

}
#endif /* COM_COMPL */

#endif /* OVL2 */

/*getline.c*/
