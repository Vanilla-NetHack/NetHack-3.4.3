/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#include <stdio.h>
extern char *eos();
#define	TOPLSZ	(COLNO-8)	/* leave room for --More-- */
char toplines[BUFSZ];
xchar tlx, tly;			/* set by pline; used by addtopl */

struct topl {
	struct topl *next_topl;
	char *topl_text;
} *old_toplines, *last_redone_topl;
#define	OTLMAX	20		/* max nr of old toplines remembered */

doredotopl(){
	if(last_redone_topl)
		last_redone_topl = last_redone_topl->next_topl;
	if(!last_redone_topl)
		last_redone_topl = old_toplines;
	if(last_redone_topl){
		(void) strcpy(toplines, last_redone_topl->topl_text);
	}
	redotoplin();
	return(0);
}

redotoplin() {
	home();
	if(index(toplines, '\n')) cl_end();
	putstr(toplines);
	cl_end();
	tlx = curx;
	tly = cury;
	flags.topl = 1;
	if(tly > 1)
		more();
}

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
	(void) strcpy(tl->topl_text, toplines);
	old_toplines = tl;
	while(cnt && tl){
		cnt--;
		tl = tl->next_topl;
	}
	if(tl && tl->next_topl){
		free((char *) tl->next_topl);
		tl->next_topl = 0;
	}
}

addtopl(s) char *s; {
	curs(tlx,tly);
	if(tlx + strlen(s) > COLNO) putsym('\n');
	putstr(s);
	tlx = curx;
	tly = cury;
	flags.topl = 1;
}

xmore(spaceflag)
boolean spaceflag;	/* TRUE if space required */
{
	if(flags.topl) {
		curs(tlx, tly);
		if(tlx + 8 > COLNO) putsym('\n'), tly++;
	}
	putstr("--More--");
	xwaitforspace(spaceflag);
	if(flags.topl && tly > 1) {
		home();
		cl_end();
		docorner(1, tly-1);
	}
	flags.topl = 0;
}

more(){
	xmore(TRUE);
}

cmore(){
	xmore(FALSE);
}

clrlin(){
	if(flags.topl) {
		home();
		cl_end();
		if(tly > 1) docorner(1, tly-1);
		remember_topl();
	}
	flags.topl = 0;
}

/*VARARGS1*/
pline(line,arg1,arg2,arg3,arg4,arg5,arg6)
register char *line,*arg1,*arg2,*arg3,*arg4,*arg5,*arg6;
{
	char pbuf[BUFSZ];
	register char *bp = pbuf, *tl;
	register int n,n0;

	if(!line || !*line) return;
	if(!index(line, '%')) (void) strcpy(pbuf,line); else
	(void) sprintf(pbuf,line,arg1,arg2,arg3,arg4,arg5,arg6);
	if(flags.topl == 1 && !strcmp(pbuf, toplines)) return;
	nscr();		/* %% */

	/* If there is room on the line, print message on same line */
	/* But messages like "You die..." deserve their own line */
	n0 = strlen(bp);
	if(flags.topl == 1 && tly == 1 &&
	    n0 + strlen(toplines) + 3 < TOPLSZ &&
	    strncmp(bp, "You ", 4)) {
		(void) strcat(toplines, "  ");
		(void) strcat(toplines, bp);
		tlx += 2;
		addtopl(bp);
		return;
	}
	if(flags.topl == 1) more();
	remember_topl();
	toplines[0] = 0;
	while(n0){
		if(n0 >= COLNO){
			/* look for appropriate cut point */
			n0 = 0;
			for(n = 0; n < COLNO; n++) if(bp[n] == ' ')
				n0 = n;
			if(!n0) for(n = 0; n < COLNO-1; n++)
				if(!letter(bp[n])) n0 = n;
			if(!n0) n0 = COLNO-2;
		}
		(void) strncpy((tl = eos(toplines)), bp, n0);
		tl[n0] = 0;
		bp += n0;

		/* remove trailing spaces, but leave one */
		while(n0 > 1 && tl[n0-1] == ' ' && tl[n0-2] == ' ')
			tl[--n0] = 0;

		n0 = strlen(bp);
		if(n0 && tl[0]) (void) strcat(tl, "\n");
	}
	redotoplin();
}

putsym(c) char c; {
	switch(c) {
	case '\b':
		backsp();
		return;
	case '\n':
		curx = 1;
		cury++;
		if(cury > tly) tly = cury;
		break;
	default:
		curx++;
		if(curx == COLNO) putsym('\n');
	}
	(void) putchar(c);
}

putstr(s) register char *s; {
	while(*s) putsym(*s++);
}
