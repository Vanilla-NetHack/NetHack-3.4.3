/*	SCCS Id: @(#)do_name.c	2.1	87/11/09
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include <stdio.h>
#include "hack.h"
extern char plname[];
extern char *rndmonnam();

getpos(cc,force,goal)
coord	*cc;
int force; char *goal;
{
register cx,cy,i,c;
extern char sdir[];		/* defined in hack.c */
extern schar xdir[], ydir[];	/* idem */
extern char *visctrl();		/* see below */
	pline("(For instructions type a ?)");
	cx = u.ux;
	cy = u.uy;
	curs(cx,cy+2);
	while((c = readchar()) != '.'){
		for(i=0; i<8; i++) if(sdir[i] == c){
			if(1 <= cx + xdir[i] && cx + xdir[i] <= COLNO)
				cx += xdir[i];
			if(0 <= cy + ydir[i] && cy + ydir[i] <= ROWNO-1)
				cy += ydir[i];
			goto nxtc;
		}
		if(c == '?'){
			pline("Use [hjkl] to move the cursor to %s.", goal);
			pline("Type a . when you are at the right place.");
		} else {
			pline("Unknown direction: '%s' (%s).",
				visctrl(c),
				force ? "use hjkl or ." : "aborted");
			if(force) goto nxtc;
			cc->x = -1;
			cc->y = 0;
			return(0);
		}
	nxtc:	;
		curs(cx,cy+2);
	}
	cc->x = cx;
	cc->y = cy;
	return(0);
}

do_mname(){
char buf[BUFSZ];
coord cc;
register int cx,cy,lth,i;
register struct monst *mtmp, *mtmp2;
extern char *lmonnam();
	getpos(&cc, 0, "the monster you want to name");
	cx = cc.x;
	cy = cc.y;
	if(cx < 0) return(0);
#ifdef DGKMOD
	if (cx == u.ux && cy == u.uy) {
		pline("This ugly monster is called %s and cannot be renamed.",
		plname);
		return(1);
	}
	if (!cansee(cx, cy) || !(mtmp = m_at(cx, cy)) || mtmp->mimic) {
		pline("I see no monster there.");
		return(1);
	}
#else
	mtmp = m_at(cx,cy);
	if(!mtmp){
	    if(cx == u.ux && cy == u.uy)
		pline("This ugly monster is called %s and cannot be renamed.",
		    plname);
	    else
		pline("There is no monster there.");
	    return(1);
	}
	if(mtmp->mimic){
	    pline("I see no monster there.");
	    return(1);
	}
	if(!cansee(cx,cy)) {
	    pline("I cannot see a monster there.");
	    return(1);
	}
#endif
	pline("What do you want to call %s? ", lmonnam(mtmp));
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033')
		return(1);
	lth = strlen(buf)+1;
	if(lth > 63){
		buf[62] = 0;
		lth = 63;
	}
	mtmp2 = newmonst(mtmp->mxlth + lth);
	*mtmp2 = *mtmp;
	for(i=0; i<mtmp->mxlth; i++)
		((char *) mtmp2->mextra)[i] = ((char *) mtmp->mextra)[i];
	mtmp2->mnamelth = lth;
	(void) strcpy(NAME(mtmp2), buf);
	replmon(mtmp,mtmp2);
	return(1);
}

/*
 * This routine changes the address of  obj . Be careful not to call it
 * when there might be pointers around in unknown places. For now: only
 * when  obj  is in the inventory.
 */
do_oname(obj)
	register struct obj *obj;
{
	char buf[BUFSZ];

	pline("What do you want to name %s? ", doname(obj));
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033')	return;
#ifdef RPH
	if(!strcmp(buf, "Excalibur")) {
		pline("Somehow you can't seem to engrave that word.");
		return;
	}
#endif
	oname(obj, buf);
}

oname(obj, buf)
	register struct obj *obj;
	char	*buf;
{
register struct obj *otmp, *otmp2;
register int	lth;

	lth = strlen(buf)+1;
	if(lth > 63){
		buf[62] = 0;
		lth = 63;
	}
	otmp2 = newobj(lth);
	*otmp2 = *obj;
	otmp2->onamelth = lth;
	(void) strcpy(ONAME(otmp2), buf);

	setworn((struct obj *) 0, obj->owornmask);
	setworn(otmp2, otmp2->owornmask);

	/* do freeinv(obj); etc. by hand in order to preserve
	   the position of this object in the inventory */
	if(obj == invent) invent = otmp2;
	else for(otmp = invent; ; otmp = otmp->nobj){
		if(!otmp)
			panic("oname: cannot find obj.");
		if(otmp->nobj == obj){
			otmp->nobj = otmp2;
			break;
		}
	}
	/* obfree(obj, otmp2);	/* now unnecessary: no pointers on bill */
	free((char *) obj);	/* let us hope nobody else saved a pointer */
}

ddocall()
{
	register struct obj *obj;
	char	ch;

#ifdef REDO
	if (!in_doagain)
#endif
		pline("Do you want to name an individual object? [ny] ");
	switch(ch = readchar()) {
	case '\033':
		break;
	case 'y':
#ifdef REDO
		savech(ch);
#endif
		obj = getobj("#", "name");
		if(obj) do_oname(obj);
		break;
	default:
#ifdef REDO
		savech(ch);
#endif
#ifdef KAA
		obj = getobj("?!=/*", "call");
#else
		obj = getobj("?!=/", "call");
#endif
		if(obj) docall(obj);
	}
	return(0);
}

docall(obj)
register struct obj *obj;
{
	char buf[BUFSZ];
	struct obj otemp;
	register char **str1;
	extern char *xname();
	register char *str;

	otemp = *obj;
	otemp.quan = 1;
	otemp.onamelth = 0;
	str = xname(&otemp);
	pline("Call %s %s: ", index(vowels,*str) ? "an" : "a", str);
	getlin(buf);
	clrlin();
	if(!*buf || *buf == '\033')
		return;
	str = newstring(strlen(buf)+1);
	(void) strcpy(str,buf);
	str1 = &(objects[obj->otyp].oc_uname);
	if(*str1) free(*str1);
	*str1 = str;
}

char *ghostnames[] = {		/* these names should have length < PL_NSIZ */
	/* Capitalize the names for asthetics -dgk
	 */
	"Adri", "Andries", "Andreas", "Bert", "David", "Dirk", "Emile",
	"Frans", "Fred", "Greg", "Hether", "Jay", "John", "Jon", "Karnov",
	"Kay", "Kenny", "Kevin", "Maud", "Michiel", "Mike", "Peter", "Robert",
	"Ron", "Tom", "Wilmar", "Nick Danger", "Phoenix", "Miracleman",
	"Stephan"
};

char *
xmonnam(mtmp, vb) register struct monst *mtmp; int vb; {
static char buf[BUFSZ];		/* %% */
extern char *shkname();
	if(mtmp->mnamelth && !vb) {
		(void) strcpy(buf, NAME(mtmp));
		return(buf);
	}
	switch(mtmp->data->mlet) {
	case ' ':
		{ register char *gn = (char *) mtmp->mextra;
		  if(!*gn) {		/* might also look in scorefile */
		    gn = ghostnames[rn2(SIZE(ghostnames))];
		    if(!rn2(2)) (void)
		      strcpy((char *) mtmp->mextra, !rn2(5) ? plname : gn);
		  }
		  (void) sprintf(buf, "%s's ghost", gn);
		}
		break;
	case '@':
		if(mtmp->isshk) {
			(void) strcpy(buf, shkname(mtmp));
			break;
		}
		/* fall into next case */
	default:
		(void) sprintf(buf, "the %s%s",
			mtmp->minvis ? "invisible " : "",
			(Hallucination ? rndmonnam() : mtmp->data->mname));
	}
	if(vb && mtmp->mnamelth) {
		(void) strcat(buf, " called ");
		(void) strcat(buf, NAME(mtmp));
	}
	return(buf);
}

char *
lmonnam(mtmp) register struct monst *mtmp; {
	return(xmonnam(mtmp, 1));
}

char *
monnam(mtmp) register struct monst *mtmp; {
	return(xmonnam(mtmp, 0));
}

char *
Monnam(mtmp) register struct monst *mtmp; {
register char *bp = monnam(mtmp);
	if('a' <= *bp && *bp <= 'z') *bp += ('A' - 'a');
	return(bp);
}

char *
amonnam(mtmp,adj)
register struct monst *mtmp;
register char *adj;
{
	register char *bp = monnam(mtmp);
	static char buf[BUFSZ];		/* %% */

	if(!strncmp(bp, "the ", 4)) bp += 4;
	(void) sprintf(buf, "the %s %s", adj, bp);
	return(buf);
}

char *
Amonnam(mtmp, adj)
register struct monst *mtmp;
register char *adj;
{
	register char *bp = amonnam(mtmp,adj);

	*bp = 'T';
	return(bp);
}

char *
Xmonnam(mtmp) register struct monst *mtmp; {
register char *bp = Monnam(mtmp);
	if(!strncmp(bp, "The ", 4)) {
#ifdef KAA
		if(index("AEIOUaeio",*(bp+4))) {
			bp += 1; *(bp+1) = 'n';
		} else
#endif
			bp += 2;
		*bp = 'A';
	}
	return(bp);
}

char *
defmonnam(mtmp) register struct monst *mtmp; {
register char *bp = Xmonnam(mtmp);
	if (!strncmp(bp,"A ",2) || !strncmp(bp,"An ",3))
		*bp = 'a';
	return(bp);
}

char *
rndmonnam() {  /* Random name of monster type, if hallucinating */
int x;
	if ((x=rn2(CMNUM+2)) != CMNUM+1) return (&mons[x])->mname;
	return("giant eel");
}

char *
visctrl(c)
char c;
{
static char ccc[3];
	if(c < 040) {
		ccc[0] = '^';
		ccc[1] = c + 0100;
		ccc[2] = 0;
	} else {
		ccc[0] = c;
		ccc[1] = 0;
	}
	return(ccc);
}
