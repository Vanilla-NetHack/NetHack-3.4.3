/*	SCCS Id: @(#)topl.c	3.0	89/01/09
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS
#include "hack.h"

char toplines[BUFSIZ];
xchar tlx, tly;			/* set by pline; used by addtopl */
static boolean no_repeat = FALSE;

struct topl {
	struct topl *next_topl;
	char *topl_text;
} *old_toplines, *last_redone_topl;
#define	OTLMAX	20		/* max nr of old toplines remembered */

static void
redotoplin() {
	home();
	if(index(toplines, '\n')) cl_end();
	if((*toplines & 0x80) && AS) {
		/* kludge for the / command, the only time we ever want a */
		/* graphics character on the top line */
		putstr(AS);
		xputc(*toplines);
		putstr(AE);
		putstr(toplines+1);
	} else putstr(toplines);
	cl_end();
	tlx = curx;
	tly = cury;
	flags.toplin = 1;
	if(tly > 1)
		more();
}

int
doredotopl(){
	if(last_redone_topl)
		last_redone_topl = last_redone_topl->next_topl;
	if(!last_redone_topl)
		last_redone_topl = old_toplines;
	if(last_redone_topl){
		Strcpy(toplines, last_redone_topl->topl_text);
	}
	redotoplin();
	return 0;
}

void
remember_topl() {
	register struct topl *tl;
	register int cnt = OTLMAX;
	if(last_redone_topl &&
	   !strcmp(toplines, last_redone_topl->topl_text)) return;
	if(old_toplines &&
	   !strcmp(toplines, old_toplines->topl_text)) return;
	last_redone_topl = 0;
	tl = (struct topl *)
		alloc((unsigned)(strlen(toplines) + sizeof(struct topl) + 1));
	tl->next_topl = old_toplines;
	tl->topl_text = (char *)(tl + 1);
	Strcpy(tl->topl_text, toplines);
	old_toplines = tl;
	while(cnt && tl){
		cnt--;
		tl = tl->next_topl;
	}
	if(tl && tl->next_topl){
		free((genericptr_t) tl->next_topl);
		tl->next_topl = 0;
	}
}

void
addtopl(s) char *s; {
	curs(tlx,tly);
	if(tlx + strlen(s) > CO) putsym('\n');
	putstr(s);
	tlx = curx;
	tly = cury;
	flags.toplin = 1;
}

static void
xmore(s)
char *s;	/* allowed chars besides space/return */
{
	if(flags.toplin) {
		curs(tlx, tly);
		if(tlx + 8 > CO) putsym('\n'), tly++;
	}

	if(flags.standout)
		standoutbeg();
	putstr("--More--");
	if(flags.standout)
		standoutend();

	xwaitforspace(s);
	if(flags.toplin && tly > 1) {
		home();
		cl_end();
		docorner(1, tly-1);
		tlx = tly = 1;
	}
	flags.toplin = 0;
}

void
more(){
	xmore("");
}

void
cmore(s)
register char *s;
{
	xmore(s);
}

void
clrlin(){
	if(flags.toplin) {
		home();
		cl_end();
		if(tly > 1) {
			docorner(1, tly-1);
			tlx = tly = 1;
		}
		remember_topl();
	}
	flags.toplin = 0;
}

/*VARARGS1*/
/* Note the modified mstatusline has 9 arguments KAA */

/* Also note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
void
pline VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, char *);
	vpline(line, VA_ARGS);
	VA_END();
}

# ifdef USE_STDARG
void
vpline(const char *line, va_list the_args) {
# else
void
vpline(line, the_args) const char *line; va_list the_args; {
# endif

#else  /* USE_STDARG | USE_VARARG */

void
pline VA_DECL(const char *, line)
#endif

	char pbuf[BUFSZ];
	register char *bp = pbuf, *tl;
	register int n,n0;
/* Do NOT use VA_START and VA_END in here... see above */

	if(!line || !*line) return;
	if(!index(line, '%')) Strcpy(pbuf,line); else
	Vsprintf(pbuf,line,VA_ARGS);
	if(no_repeat && flags.toplin == 1 && !strcmp(pbuf, toplines)) return;
	nscr();		/* %% */

	/* If there is room on the line, print message on same line */
	/* But messages like "You die..." deserve their own line */
	n0 = strlen(bp);
	if(flags.toplin == 1 && tly == 1 &&
	    n0 + strlen(toplines) + 3 < CO-8 &&  /* leave room for --More-- */
	    strncmp(bp, "You die", 7)) {
		Strcat(toplines, "  ");
		Strcat(toplines, bp);
		tlx += 2;
		addtopl(bp);
		return;
	}
	if(flags.toplin == 1 && !strcmp(pbuf, toplines) &&
	    (n0 + strlen(toplines) + 3 >= CO-8)) {
		more();
		home();
		putstr("");
		cl_end();
		goto again;
	}
	if(flags.toplin == 1) more();
	else if(tly > 1) {	/* for when flags.toplin == 2 && tly > 1 */
		docorner(1, tly-1);	/* reset tly = 1 if redraw screen */
		tlx = tly = 1;	/* from home--cls() and docorner(1,n) */
	}
again:
	remember_topl();
	toplines[0] = 0;
	while(n0){
		if(n0 >= CO){
			/* look for appropriate cut point */
			n0 = 0;
			for(n = 0; n < CO; n++) if(bp[n] == ' ')
				n0 = n;
			if(!n0) for(n = 0; n < CO-1; n++)
				if(!letter(bp[n])) n0 = n;
			if(!n0) n0 = CO-2;
		}
		(void) strncpy((tl = eos(toplines)), bp, n0);
		tl[n0] = 0;
		bp += n0;

		/* remove trailing spaces, but leave one */
		while(n0 > 1 && tl[n0-1] == ' ' && tl[n0-2] == ' ')
			tl[--n0] = 0;

		n0 = strlen(bp);
		if(n0 && tl[0]) Strcat(tl, "\n");
	}
	redotoplin();
}

/*VARARGS1*/
void
Norep VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, const char *);
	no_repeat = TRUE;
	vpline(line, VA_ARGS);
	no_repeat = FALSE;
	VA_END();
	return;
}

/*VARARGS1*/
void
You VA_DECL(const char *, line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	tmp = (char *)alloc((unsigned int)(strlen(line) + 5));
	Strcpy(tmp, "You ");
	Strcat(tmp, line);
	vpline(tmp, VA_ARGS);
	free(tmp);
	VA_END();
	return;
}

/*VARARGS1*/
void
Your VA_DECL(const char *,line)
	char *tmp;
	VA_START(line);
	VA_INIT(line, const char *);
	tmp = (char *)alloc((unsigned int)(strlen(line) + 6));
	Strcpy(tmp, "Your ");
	Strcat(tmp, line);
	vpline(tmp, VA_ARGS);
	free(tmp);
	VA_END();
	return;
}

/*ARGSUSED*/
/*VARARGS2*/
void
kludge  VA_DECL2(char *, str, char *, arg)
#ifdef VA_NEXT
	char *other1, *other2, *other3;
#endif
	VA_START(arg);
	VA_INIT(str, char *);
	VA_INIT(arg, char *);
#ifdef VA_NEXT
	VA_NEXT(other1, char *);
	VA_NEXT(other2, char *);
	VA_NEXT(other3, char *);
# define OTHER_ARGS other1,other2,other3
#else
# define OTHER_ARGS arg1,arg2,arg3
#endif
	if(Blind || !flags.verbose) {
		if(*str == '%') pline(str,"It",OTHER_ARGS);
		else pline(str,"it",OTHER_ARGS);
	} else pline(str,arg,OTHER_ARGS);
	VA_END();
}

void
putsym(c)
char c;
{
	switch(c) {
	case '\b':
		backsp();
		curx--;
		if(curx == 1 && cury > 1)
			curs(CO, cury-1);
		return;
	case '\n':
		curx = 1;
		cury++;
		if(cury > tly) tly = cury;
		break;
	default:
		if(curx == CO)
			putsym('\n');	/* 1 <= curx <= CO; avoid CO */
		curx++;
	}
	(void) putchar(c);
}

void
putstr(s)
register const char *s;
{
	while(*s) putsym(*s++);
}

char
yn_function(resp, def)
const char *resp;
char def;
/*
 *   Generic yes/no function
 */
{
	register char q;
	char rtmp[8];

	Sprintf(rtmp, "[%s] ", resp);
	addtopl(rtmp);

	do {
		q = lowc(readchar());

		if (index(quitchars, q)) q = def;
		else if (!index(resp, q)) {
			bell();
			q = (char)0;
		}
	} while(!q);

	Sprintf(rtmp, "%c", q);
	addtopl(rtmp);
	flags.toplin = 2;

	return q;
}

/*VARARGS1*/
void
impossible VA_DECL(char *, s)
	VA_START(s);
	VA_INIT(s, char *);
	vpline(s,VA_ARGS);
	pline("Program in disorder - perhaps you'd better Quit.");
	VA_END();
}
