/*	SCCS Id: @(#)pager.c	3.0	88/10/25 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file contains the command routine dowhatis() and a pager. */
/* Also readmail() and doshell(), and generally the things that
   contact the outside world. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include	 "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif
#if defined(BSD) || defined(ULTRIX)
#include <sys/wait.h>
#endif

static char hc = 0;

static void page_more();
static boolean clear_help P((CHAR_P));
static boolean valid_help P((CHAR_P));

int
dowhatis()
{
	FILE *fp;
	char bufr[BUFSZ+6];
	register char *buf = &bufr[6], *ep, q;
	register struct monst *mtmp;
#ifdef OS2_CODEVIEW
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,DATAFILE);
	if(!(fp = fopen(tmp,"r")))
#else
	if(!(fp = fopen(DATAFILE, "r")))
#endif
		pline("Cannot open data file!");
	else {
		coord	cc;
		uchar	r;
		boolean bycurs = FALSE;

		pline ("Specify unknown object by cursor? ");
		q = ynq();
		if (q == 'q') {
			(void) fclose(fp);
			return 0;
		} else if (q == 'n') {
			cc.x = cc.y = -1;
			pline("Specify what? ");
			r = readchar();
		} else {
		    bycurs = TRUE;
		    cc.x = u.ux;
		    cc.y = u.uy;
	selobj:
		    if(flags.verbose)
			pline("Please move the cursor to an unknown object.");
		    getpos(&cc, FALSE, "an unknown object");
		    if (cc.x < 0) {
			    (void) fclose(fp); /* sweet@scubed */
			    return 0;
		    }
		    r = levl[cc.x][cc.y].scrsym;
		}

#define conv_sym(x)	if(r == showsyms.x) q = defsyms.x
		conv_sym(stone);
		else conv_sym(vwall);
		else conv_sym(hwall);
		else conv_sym(tlcorn);
		else conv_sym(trcorn);
		else conv_sym(blcorn);
		else conv_sym(brcorn);
		else conv_sym(crwall);
		else conv_sym(tuwall);
		else conv_sym(tdwall);
		else conv_sym(tlwall);
		else conv_sym(trwall);
		else conv_sym(door);
		else conv_sym(room);
		else conv_sym(corr);
		else conv_sym(upstair);
		else conv_sym(dnstair);
		else conv_sym(trap);
		else conv_sym(web); 
		else conv_sym(pool);
#ifdef FOUNTAINS
		else conv_sym(fountain);
#endif
#ifdef SINKS
		else conv_sym(sink);
#endif
#ifdef THRONES
		else conv_sym(throne);
#endif
#ifdef ALTARS
		else conv_sym(altar);
#endif
#ifdef STRONGHOLD
		else conv_sym(upladder);
		else conv_sym(dnladder);
		else conv_sym(dbvwall);
		else conv_sym(dbhwall);
#endif
		else q = r;
#undef conv_sym

		if (index(quitchars, q)) {
			(void) fclose(fp); /* sweet@scubed */
			return 0;
		}
		if(q == '%') {
			pline("%%       a piece of food");
			if(bycurs) {
				buf = &bufr[6];
				more();
				rewind(fp);
				goto selobj;
			}
			(void) fclose(fp);
			return 0;
		}

		if(q != '\t')
		while(fgets(buf,BUFSZ,fp))
		    if(*buf == q) {
			ep = index(buf, '\n');
			if(ep) *ep = 0;
			/* else: bad data file */
			/* Expand tab 'by hand' */
			if(buf[1] == '\t'){
				buf = bufr;
				buf[0] = r;
				(void) strncpy(buf+1, "       ", 7);
			}
			pline(buf);
			if(cc.x != -1) {
#ifdef ALTARS
			    if (r == showsyms.altar && q == defsyms.altar &&
				(IS_ALTAR(levl[cc.x][cc.y].typ) ||
				 (levl[cc.x][cc.y].mmask &&
				    m_at(cc.x,cc.y)->mimic))
			       ) {
				    int type = levl[cc.x][cc.y].altarmask &
						~A_SHRINE;
				    pline((type == A_CHAOS) ? "(chaotic)" :
					  (type == A_NEUTRAL) ? "(neutral)" :
					  "(lawful)");
			    } else
			    if (q == CHAIN_SYM && OBJ_AT(cc.x, cc.y))
				    pline("(chain)");
			    else
#endif
			    if (r == showsyms.door && q == defsyms.door &&
				(IS_DOOR(levl[cc.x][cc.y].typ) ||
                                 (levl[cc.x][cc.y].mmask &&
				    m_at(cc.x,cc.y)->mimic))) {
				/* Note: this will say mimics in walls are
				 *	 closed doors, which we want.
				 */
				switch(levl[cc.x][cc.y].doormask & ~D_TRAPPED) {
				    case D_NODOOR: pline("(doorway)"); break;
				    case D_BROKEN: pline("(broken door)"); break;
				    case D_ISOPEN: pline("(open door)"); break;
				    default:	   pline("(closed door)"); break;
						   /* locked or not */
				}
			    }
#ifdef SPELLS
			    else
			    if (q == SPBOOK_SYM && OBJ_AT(cc.x, cc.y))
				    pline("(spellbook)");
#endif
#ifdef STRONGHOLD
			    else
			    if (((r == showsyms.dbvwall && q == defsyms.dbvwall) ||
				 (r == showsyms.dbvwall && q == defsyms.dbvwall)) &&
				is_db_wall(cc.x,cc.y))
				    pline("(raised drawbridge)");
#endif
#ifdef SINKS
			    else
			    if (r == showsyms.sink && q == defsyms.sink &&
				IS_SINK(levl[cc.x][cc.y].typ))
				    pline("(sink)");
#endif
			}
			if (!Invisible 
#ifdef POLYSELF
				&& !u.uundetected
#endif
					&& u.ux==cc.x && u.uy==cc.y) {
			    pline("(%s named %s)",
#ifdef POLYSELF
				u.mtimedone ? mons[u.umonnum].mname :
#endif
				pl_character, plname);
			} else if(levl[cc.x][cc.y].mmask) {
			    mtmp = m_at(cc.x,cc.y);
			    if (q == mtmp->data->mlet)
				pline("(%s%s)",
				    mtmp->mtame ? "tame " :
				      mtmp->mpeaceful ? "peaceful " : "",
				    strncmp(lmonnam(mtmp), "the ", 4)
				      ? lmonnam(mtmp) : lmonnam(mtmp)+4);
			}
			if(ep[-1] == ';') {
				pline("More info? ");
				if(yn() == 'y') {
					page_more(fp,1); /* does fclose() */
					return 0;
				}
			}
			if(bycurs) {
				buf = &bufr[6];
				more();
				rewind(fp);
				goto selobj;
			}
			(void) fclose(fp); 	/* kopper@psuvax1 */
			return 0;
		    }
		pline("I've never heard of such things.");
		(void) fclose(fp);
	}
	return 0;
}

int
dowhatdoes()
{
	FILE *fp;
	char bufr[BUFSZ+6];
	register char *buf = &bufr[6], *ep, q, ctrl, meta;
#ifdef OS2_CODEVIEW
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,CMDHELPFILE);
	if(!(fp = fopen(tmp,"r"))) {
#else
	if(!(fp = fopen(CMDHELPFILE, "r"))) {
#endif
		pline("Cannot open data file!");
		return 0;
	}

	pline("What command? ");
#ifdef UNIX
	introff();
#endif
	q = readchar();
#ifdef UNIX
	intron();
#endif
	ctrl = ((q <= '\033') ? (q - 1 + 'A') : 0);
	meta = ((0x80 & q) ? (0x7f & q) : 0);
	while(fgets(buf,BUFSZ,fp))
	    if ((ctrl && *buf=='^' && *(buf+1)==ctrl) ||
		(meta && *buf=='M' && *(buf+1)=='-' && *(buf+2)==meta) ||
		*buf==q) {
		ep = index(buf, '\n');
		if(ep) *ep = 0;
		if (ctrl && buf[2] == '\t'){
			buf = bufr + 1;
			(void) strncpy(buf, "^?      ", 8);
			buf[1] = ctrl;
		} else if (meta && buf[3] == '\t'){
			buf = bufr + 2;
			(void) strncpy(buf, "M-?     ", 8);
			buf[2] = meta;
		} else if(buf[1] == '\t'){
			buf = bufr;
			buf[0] = q;
			(void) strncpy(buf+1, "       ", 7);
		}
		pline(buf);
		(void) fclose(fp);
		return 0;
	    }
	pline("I've never heard of such commands.");
	(void) fclose(fp);
	return 0;
}

/* make the paging of a file interruptible */
static int got_intrup;

#if !defined(MSDOS) && !defined(TOS)
static int
intruph(){
	(void) signal(SIGINT, (SIG_RET_TYPE) intruph);
	got_intrup++;
	return 0;
}
#endif

/* simple pager, also used from dohelp() */
static void
page_more(fp,strip)
FILE *fp;
int strip;	/* nr of chars to be stripped from each line (0 or 1) */
{
	register char *bufr;
#if !defined(MSDOS) && !defined(MINIMAL_TERM)
	register char *ep;
#endif
#if !defined(MSDOS) && !defined(TOS)
	int (*prevsig)() = (int (*)())signal(SIGINT, (SIG_RET_TYPE) intruph);
#endif
#if defined(MSDOS) || defined(MINIMAL_TERM)
	/* There seems to be a bug in ANSI.SYS  The first tab character
	 * after a clear screen sequence is not expanded correctly.  Thus
	 * expand the tabs by hand -dgk
	 */
	int tabstop = 8, spaces;
	char buf[BUFSIZ], *bufp, *bufrp;

	set_pager(0);
	bufr = (char *) alloc((unsigned) COLNO);
	while (fgets(buf, BUFSIZ, fp) && (!strip || *buf == '\t')){
		bufp = buf;
		bufrp = bufr;
		while (*bufp && *bufp != '\n') {
			if (*bufp == '\t') {
				spaces = tabstop - (bufrp - bufr) % tabstop;
				while (spaces--)
					*bufrp++ = ' ';
				bufp++;
			} else
				*bufrp++ = *bufp++;
		}
		*bufrp = '\0';
#else /* MSDOS /**/
	set_pager(0);
	bufr = (char *) alloc((unsigned) COLNO);
	bufr[COLNO-1] = 0;
	while(fgets(bufr,COLNO-1,fp) && (!strip || *bufr == '\t')){
		ep = index(bufr, '\n');
		if(ep)
			*ep = 0;
#endif /* MSDOS /**/
		if(got_intrup || page_line(bufr+strip)) {
			set_pager(2);
			goto ret;
		}
	}
	set_pager(1);
ret:
	free((genericptr_t) bufr);
	(void) fclose(fp);
#if !defined(MSDOS) && !defined(TOS)
	(void) signal(SIGINT, (SIG_RET_TYPE) prevsig);
	got_intrup = 0;
#endif
}

static boolean whole_screen = TRUE;
#define	PAGMIN	12	/* minimum # of lines for page below level map */

void
set_whole_screen() {	/* called in termcap as soon as LI is known */
	whole_screen = (LI-ROWNO-2 <= PAGMIN || !CD);
}

#ifdef NEWS
int
readnews() {
	register int ret;

	whole_screen = TRUE;	/* force a docrt(), our first */
	ret = page_file(NEWS, TRUE);
	set_whole_screen();
	return(ret);		/* report whether we did docrt() */
}
#endif

void
set_pager(mode)
register int mode;	/* 0: open  1: wait+close  2: close */
{
#ifdef LINT	/* lint may handle static decl poorly -- static boolean so; */
	boolean so;
#else
	static boolean so;
#endif
	if(mode == 0) {
		if(!whole_screen) {
			/* clear topline */
			clrlin();
			/* use part of screen below level map */
			curs(1, ROWNO+4);
		} else {
			cls();
		}
		so = flags.standout;
		flags.standout = 1;
	} else {
		if(mode == 1) {
			curs(1, LI);
			more();
		}
		flags.standout = so;
		if(whole_screen)
			docrt();
		else {
			curs(1, ROWNO+4);
			cl_eos();
		}
	}
}

int
page_line(s)		/* returns 1 if we should quit */
register char *s;
{
	if(cury == LI-1) {
		if(!*s)
			return(0);	/* suppress blank lines at top */
		(void) putchar('\n');
		cury++;
		cmore("q\033");
		if(morc) {
			morc = 0;
			return(1);
		}
		if(whole_screen)
			cls();
		else {
			curs(1, ROWNO+4);
			cl_eos();
		}
	}
#ifdef TERMINFO
	xputs(s); xputc('\n');
#else
	(void) puts(s);
#endif
	cury++;
	return(0);
}

/*
 * Flexible pager: feed it with a number of lines and it will decide
 * whether these should be fed to the pager above, or displayed in a
 * corner.
 * Call:
 *	cornline(0, title or 0)	: initialize
 *	cornline(1, text)	: add text to the chain of texts
 *	cornline(2, morcs)	: output everything and cleanup
 *	cornline(3, 0)		: cleanup
 *	cornline(-1,"")		: special, for help menu mode only
 */

void
cornline(mode, text)
int mode;
char *text;
{
	static struct line {
		struct line *next_line;
		char *line_text;
	} *texthead, *texttail;
	static int maxlen;
	static int linect;
	register struct line *tl;
	register boolean hmenu = FALSE;

	if(mode == -1) { /* help menu display only */
		mode = 2;
		hmenu = TRUE;
	}
	if(mode == 0) {
		texthead = 0;
		maxlen = 0;
		linect = 0;
		if(text) {
			cornline(1, text);	/* title */
			cornline(1, "");	/* blank line */
		}
		return;
	}

	if(mode == 1) {
	    register int len;

	    if(!text) return;	/* superfluous, just to be sure */
	    linect++;
	    len = strlen(text) + 1; /* allow for an extra leading space */
	    if(len > maxlen)
		maxlen = len;
	    tl = (struct line *)
		alloc((unsigned)(len + sizeof(struct line) + 1));
	    tl->next_line = 0;
	    tl->line_text = (char *)(tl + 1);
	    tl->line_text[0] = ' ';
	    tl->line_text[1] = '\0';
	    Strcat(tl->line_text, text);
	    if(!texthead)
		texthead = tl;
	    else
		texttail->next_line = tl;
	    texttail = tl;
	    return;
	}

	/* --- now we really do it --- */
	if(mode == 2 && linect == 1)			    /* topline only */
		pline(texthead->line_text);
	else
	if(mode == 2) {
	    register int curline, lth;

	    if(flags.toplin == 1) more();	/* ab@unido */
	    remember_topl();

	    lth = CO - maxlen - 2;		   /* Use full screen width */
	    if (linect < LI && lth >= 10) {		     /* in a corner */
		home ();
		cl_end ();
		flags.toplin = 0;
		curline = 1;
		for (tl = texthead; tl; tl = tl->next_line) {
#if defined(MSDOS) && !defined(AMIGA)
		    cmov (lth, curline);
#else
		    curs (lth, curline);
#endif
		    if(curline > 1)
			cl_end ();
		    xputs(tl->line_text);
		    curx = curx + strlen(tl->line_text);
		    curline++;
		}
		if(hmenu) {	/* help menu display */
			do 
				hc = lowc(readchar());
			while (!valid_help(hc));
		}
#if defined(MSDOS) && !defined(AMIGA)
		cmov (lth, curline);
#else
		curs (lth, curline);
#endif
		cl_end ();
		if (!hmenu) cmore (text);
		if (!hmenu || clear_help(hc)) {
		    home ();
		    cl_end ();
		    docorner (lth, curline-1);
		}
	    } else {					/* feed to pager */
		set_pager(0);
		for (tl = texthead; tl; tl = tl->next_line) {
		    if (page_line (tl->line_text)) {
			set_pager(2);
			while(tl = texthead) {
			    texthead = tl->next_line;
			    free((genericptr_t) tl);
			}
			return;
		    }
		}
		if(text) {
			cgetret(text);
			set_pager(2);
		} else
			set_pager(1);
	    }
	}

	while(tl = texthead) {
		texthead = tl->next_line;
		free((genericptr_t) tl);
	}
}

#ifdef WIZARD
static
void
wiz_help()
{
	cornline(0, "Wizard-Mode Quick Reference:");
	cornline(1, "^E  ==  detect secret doors and traps.");
	cornline(1, "^F  ==  do magic mapping.");
	cornline(1, "^G  ==  create monster.");
	cornline(1, "^I  ==  identify items in pack.");
	cornline(1, "^O  ==  tell locations of special levels.");
	cornline(1, "^T  ==  do intra-level teleport.");
	cornline(1, "^V  ==  do trans-level teleport.");
	cornline(1, "^W  ==  make wish.");
	cornline(1, "^X  ==  show intrinsic attributes.");
	cornline(1, "");
	cornline(2, "");
}
#endif

static void
help_menu() {
	cornline(0, "Information available:");
	cornline(1, "a.  Long description of the game and commands.");
	cornline(1, "b.  List of game commands.");
	cornline(1, "c.  Concise history of NetHack.");
	cornline(1, "d.  Info on a character in the game display.");
	cornline(1, "e.  Info on what a given key does.");
	cornline(1, "f.  List of game options.");
	cornline(1, "g.  Longer explanation of game options.");
	cornline(1, "h.  List of extended commands.");
	cornline(1, "i.  The NetHack license.");
#ifdef WIZARD
	if (wizard)
		cornline(1, "j.  List of wizard-mode commands.");
#endif
	cornline(1, "");
#ifdef WIZARD
	if (wizard)
		cornline(1, "Select one of a,b,c,d,e,f,g,h,i,j or ESC: ");
	else
#endif
		cornline(1, "Select one of a,b,c,d,e,f,g,h,i or ESC: ");
	cornline(-1,"");
}

static boolean
clear_help(c)
char c;
{
	/* those valid_help characters which do not correspond to help routines
	 * that redraw the whole screen on their own.  if we always clear the
	 * help menu, we end up restoring the part of the maze underneath the
	 * help menu when the last page of a long help file is displayed with
	 * an external pager.
	 */
	return(index(quitchars,c) || c == 'd' || c == 'e'
#ifdef WIZARD
		|| c == 'j'
#endif
		);
}

static boolean
valid_help(c)
char c;
{
#ifdef WIZARD
	return ((c >= 'a' && c <= (wizard ? 'j' : 'i')) || index(quitchars,c));
#else
	return ((c >= 'a' && c <= 'i') || index(quitchars,c));
#endif
}

int
dohelp()
{
	help_menu();
	if (!index(quitchars, hc)) {
		switch(hc) {
			case 'a':  (void) page_file(HELP, FALSE);  break;
			case 'b':  (void) page_file(SHELP, FALSE);  break;
			case 'c':  (void) dohistory();  break;
			case 'd':  (void) dowhatis();  break;
			case 'e':  (void) dowhatdoes();  break;
			case 'f':  option_help();  break;
			case 'g':  (void) page_file(OPTIONFILE, FALSE);  break;
			case 'h':  (void) doextlist();  break;
			case 'i':  (void) page_file(LICENSE, FALSE);  break;
#ifdef WIZARD
			case 'j':  wiz_help();  break;
#endif
		}
	}
	return 0;
}

int
dohistory()
{
	(void) page_file(HISTORY, FALSE);
	return 0;
}

int
page_file(fnam, silent)	/* return: 0 - cannot open fnam; 1 - otherwise */
register char *fnam;
boolean silent;
{
#ifdef DEF_PAGER			/* this implies that UNIX is defined */
      {
	/* use external pager; this may give security problems */

	register int fd = open(fnam, 0);

	if(fd < 0) {
		if(!silent) pline("Cannot open %s.", fnam);
		return(0);
	}
	if(child(1)){
		/* Now that child() does a setuid(getuid()) and a chdir(),
		   we may not be able to open file fnam anymore, so make
		   it stdin. */
		(void) close(0);
		if(dup(fd)) {
			if(!silent) Printf("Cannot open %s as stdin.\n", fnam);
		} else {
			(void) execl(catmore, "page", NULL);
			if(!silent) Printf("Cannot exec %s.\n", catmore);
		}
		exit(1);
	}
	(void) close(fd);
      }
#else
      {
	FILE *f;			/* free after Robert Viduya */
#ifdef OS2_CODEVIEW
	char tmp[PATHLEN];

	Strcpy(tmp,hackdir);
	append_slash(tmp);
	Strcat(tmp,fnam);
	if ((f = fopen (tmp, "r")) == (FILE *) 0) {
#else
	if ((f = fopen (fnam, "r")) == (FILE *) 0) {
#endif
		if(!silent) {
			home(); perror (fnam); flags.toplin = 1;
			pline ("Cannot open %s.", fnam);
		}
		return(0);
	}
	page_more(f, 0);
      }
#endif /* DEF_PAGER /**/

	return(1);
}

#ifdef UNIX
#ifdef SHELL
int
dosh(){
register char *str;
	if(child(0)) {
		if(str = getenv("SHELL"))
			(void) execl(str, str, NULL);
		else
			(void) execl("/bin/sh", "sh", NULL);
		pline("sh: cannot execute.");
		exit(1);
	}
	return 0;
}
#endif /* SHELL /**/

int
child(wt)
int wt;
{
register int f = fork();
	if(f == 0){		/* child */
		settty(NULL);		/* also calls end_screen() */
		(void) setgid(getgid());
		(void) setuid(getuid());
#ifdef CHDIR
		(void) chdir(getenv("HOME"));
#endif
		return(1);
	}
	if(f == -1) {	/* cannot fork */
		pline("Fork failed.  Try again.");
		return(0);
	}
	/* fork succeeded; wait for child to exit */
	(void) signal(SIGINT,SIG_IGN);
	(void) signal(SIGQUIT,SIG_IGN);
	(void) wait(
#if defined(BSD) || defined(ULTRIX)
		(union wait *)
#else
		(int *)
#endif
		0);
	gettty();
	setftty();
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef WIZARD
	if(wizard) (void) signal(SIGQUIT,SIG_DFL);
#endif
	if(wt) {
		boolean so;

		cmov(1, LI);	/* get prompt in reasonable place */
		so = flags.standout;
		flags.standout = 1;
		more();
		flags.standout = so;
	}
	docrt();
	return(0);
}
#endif /* UNIX /**/
