/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
#include	<stdio.h>
extern struct obj *splitobj();
extern char morc;
#ifndef NOWORM
#include	"def.wseg.h"

extern struct wseg *wsegs[32];
#endif NOWORM

struct obj *
addinv(obj) register struct obj *obj; {
	register struct obj *otmp;
	for(otmp = invent; otmp; otmp = otmp->nobj) {
		if(merged(otmp, obj, 0)) return(otmp);
		if(!otmp->nobj) {
			otmp->nobj = obj;
			obj->nobj = 0;
			return(obj);
		}
	}
	invent = obj;
	obj->nobj = 0;
	return(obj);
}

useup(obj)
register struct obj *obj;
{
	if(obj->quan > 1){
		obj->quan--;
		obj->owt = weight(obj);
	} else {
		setnotworn(obj);
		freeinv(obj);
		obfree(obj, (struct obj *) 0);
	}
}

freeinv(obj) register struct obj *obj; {
	register struct obj *otmp;
	if(obj == invent) invent = invent->nobj;
	else {
		for(otmp = invent; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp->nobj) panic("freeinv");
		otmp->nobj = obj->nobj;
	}
}

/* destroy object in fobj chain (if unpaid, it remains on the bill) */
delobj(obj) register struct obj *obj; {
	freeobj(obj);
	unpobj(obj);
	obfree(obj, (struct obj *) 0);
}

/* unlink obj from chain starting with fobj */
freeobj(obj) register struct obj *obj; {
	register struct obj *otmp;

	if(obj == fobj) fobj = fobj->nobj;
	else {
		for(otmp = fobj; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp) panic("error in freeobj");
		otmp->nobj = obj->nobj;
	}
}

/* Note: freegold throws away its argument! */
freegold(gold) register struct gen *gold; {
	register struct gen *gtmp;

	if(gold == fgold) fgold = gold->ngen;
	else {
		for(gtmp = fgold; gtmp->ngen != gold; gtmp = gtmp->ngen)
			if(!gtmp) panic("error in freegold");
		gtmp->ngen = gold->ngen;
	}
 free((char *) gold);
}

deltrap(trap)
register struct gen *trap;
{
	register struct gen *gtmp;

	if(trap==ftrap) ftrap=ftrap->ngen;
	else {
		for(gtmp=ftrap;gtmp->ngen!=trap;gtmp=gtmp->ngen) ;
		gtmp->ngen=trap->ngen;
	}
 free((char *) trap);
}

struct wseg *m_atseg;

struct monst *
m_at(x,y)
register x,y;
{
	register struct monst *mtmp;
#ifndef NOWORM
	register struct wseg *wtmp;
#endif NOWORM

	m_atseg = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		if(mtmp->mx == x && mtmp->my == y)
			return(mtmp);
#ifndef NOWORM
		if(mtmp->wormno){
		    for(wtmp = wsegs[mtmp->wormno]; wtmp; wtmp = wtmp->nseg)
		    if(wtmp->wx == x && wtmp->wy == y){
			m_atseg = wtmp;
			return(mtmp);
		    }
		}
 #endif NOWORM
	}
 return(0);
}

struct obj *
o_at(x,y)
register x,y;
{
	register struct obj *otmp;

	for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(otmp->ox == x && otmp->oy == y) return(otmp);
	return(0);
}

struct obj *
sobj_at(n,x,y)
register n,x,y;
{
	register struct obj *otmp;

	for(otmp = fobj; otmp; otmp = otmp->nobj)
		if(otmp->ox == x && otmp->oy == y && otmp->otyp == n)
			return(otmp);
	return(0);
}

carried(obj) register struct obj *obj; {
register struct obj *otmp;
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp == obj) return(1);
	return(0);
}

struct obj *
o_on(id, objchn) unsigned int id; register struct obj *objchn; {
	while(objchn) {
		if(objchn->o_id == id) return(objchn);
		objchn = objchn->nobj;
	}
 return((struct obj *) 0);
}

struct gen *
g_at(x,y,ptr)
register x,y;
register struct gen *ptr;
{
	while(ptr) {
		if(ptr->gx == x && ptr->gy == y) return(ptr);
		ptr = ptr->ngen;
	}
 return(0);
}

/* getobj returns:
	struct obj *xxx:	object to do something with.
	0				error return: no object.
	-1			explicitly no object (as in w-).
*/
struct obj *
getobj(let,word)
register char *let,*word;
{
	register struct obj *otmp;
	register char ilet,ilet1,ilet2;
	char buf[BUFSZ];
	char lets[BUFSZ];
	register int foo = 0, foo2, cnt;
	register char *bp = buf;
	xchar allowcnt = 0;	/* 0, 1 or 2 */
	boolean allowgold = FALSE;
	boolean allowall = FALSE;
	boolean allownone = FALSE;
	xchar foox = 0;

	if(*let == '0') let++, allowcnt = 1;
	if(*let == '$') let++, allowgold = TRUE;
	if(*let == '#') let++, allowall = TRUE;
	if(*let == '-') let++, allownone = TRUE;
	if(allownone) *bp++ = '-';
	if(allowgold) *bp++ = '$';
	if(bp[-1] == '-') *bp++ = ' ';

	ilet = 'a';
	for(otmp = invent; otmp; otmp = otmp->nobj){
		if(!*let || index(let, otmp->olet)) {
			bp[foo++] = ilet;
			/* ugly check: remove inappropriate things */
			if((!strcmp(word, "take off") &&
			    !(otmp->owornmask & (W_ARMOR - W_ARM2)))
			|| (!strcmp(word, "wear") &&
    (otmp->owornmask & (W_ARMOR | W_RING)))
			|| (!strcmp(word, "wield") &&
			    (otmp->owornmask & W_WEP))) {
				foo--;
				foox++;
			}
		}
 if(ilet == 'z') ilet = 'A'; else ilet++;
	}
	bp[foo] = 0;
	if(foo == 0 && bp > buf && bp[-1] == ' ') *--bp = 0;
	(void) strcpy(lets, bp);	/* necessary since we destroy buf */
	if(foo > 5) {			/* compactify string */
		foo = foo2 = 1;
		ilet2 = bp[0];
		ilet1 = bp[1];
		while(ilet = bp[++foo2] = bp[++foo]){
			if(ilet == ilet1+1){
				if(ilet1 == ilet2+1)
					bp[foo2 - 1] = ilet1 = '-';
				else if(ilet2 == '-') {
					bp[--foo2] = ++ilet1;
					continue;
				}
			}
			ilet2 = ilet1;
			ilet1 = ilet;
		}
	}
	if(!foo && !allowall && !allowgold && !allownone) {
		pline("You don't have anything %sto %s.",
			foox ? "else " : "", word);
		return(0);
	}
	for(;;) {
		if(!buf[0])
			pline("What do you want to %s [*]? ", word);
		else
			pline("What do you want to %s [%s or ?*]? ",
				word, buf);

		cnt = 0;
		ilet = readchar();
		while(digit(ilet) && allowcnt) {
			cnt = 10*cnt + (ilet - '0');
			allowcnt = 2;	/* signal presence of cnt */
			ilet = readchar();
		}
		if(digit(ilet)) {
			pline("No count allowed with this command.");
			continue;
		}
		if(ilet == '\033' || ilet == ' ' || ilet == '\n')
			return((struct obj *)0);
		if(ilet == '-') {
			return((struct obj *)(allownone ? -1 : 0));
		}
		if(ilet == '$') {
			if(!allowgold){
				pline("You cannot %s gold.", word);
				continue;
			}
			otmp = newobj(0);
			/* should set o_id etc. but otmp will be freed soon */
			otmp->olet = '$';
			if(allowcnt == 2 && cnt < u.ugold)
				u.ugold -= cnt;
			else {
				cnt = u.ugold;
				u.ugold = 0;
			}
			flags.botl = 1;
			otmp->quan = cnt;
			return(otmp);
		}
		if(ilet == '?') {
			doinv(lets);
			if(!(ilet = morc)) continue;
			/* he typed a letter (not a space) to more() */
		} else if(ilet == '*') {
			doinv("");
			if(!(ilet = morc)) continue;
			/* ... */
		}
		if(ilet >= 'A' && ilet <= 'Z') ilet += 'z'-'A'+1;
		ilet -= 'a';
		for(otmp = invent; otmp && ilet; ilet--, otmp = otmp->nobj) ;
		if(!otmp) {
			pline("You don't have that object.");
			continue;
		}
		if(cnt < 0 || otmp->quan < cnt) {
			pline("You don't have that many! [You have %d]"
			, otmp->quan);
			continue;
		}
 break;
	}
	if(!allowall && let && !index(let,otmp->olet)) {
		pline("That is a silly thing to %s.",word);
		return(0);
	}
	if(allowcnt == 2) {	/* cnt given */
		if(cnt == 0) return(0);
		if(cnt != otmp->quan) {
			register struct obj *obj;
			obj = splitobj(otmp, cnt);
			if(otmp == uwep) setuwep(obj);
		}
	}
 return(otmp);
}

ckunpaid(otmp) register struct obj *otmp; {
	return( otmp->unpaid );
}

/* interactive version of getobj */
/* used for Drop and Identify */
ggetobj(word, fn, max)
char *word;
int (*fn)(),  max;
{
char buf[BUFSZ];
register char *ip;
register char sym;
register int oletct = 0, iletct = 0;
register boolean allflag = FALSE;
char olets[20], ilets[20];
int (*ckfn)() = (int (*)()) 0;
	if(!invent){
		pline("You have nothing to %s.", word);
		return(0);
	} else {
		register struct obj *otmp = invent;
		register int uflg = 0;

		ilets[0] = 0;
		while(otmp) {
			if(!index(ilets, otmp->olet)){
				ilets[iletct++] = otmp->olet;
				ilets[iletct] = 0;
			}
			if(otmp->unpaid) uflg = 1;
			otmp = otmp->nobj;
		}
		ilets[iletct++] = ' ';
		if(uflg) ilets[iletct++] = 'u';
		ilets[iletct++] = 'a';
		ilets[iletct] = 0;
	}
	pline("What kinds of thing do you want to %s? [%s] ",
		word, ilets);
	getlin(buf);
	ip = buf;
	olets[0] = 0;
	while(sym = *ip++){
		if(sym == ' ') continue;
		if(sym == 'a') allflag = TRUE; else
		if(sym == 'u') ckfn = ckunpaid; else
		if(index("!%?[()=*/\"0", sym)){
			if(!index(olets, sym)){
				olets[oletct++] = sym;
				olets[oletct] = 0;
			}
		}
 else pline("You don't have any %c's.", sym);
	}
 return askchain(invent, olets, allflag, fn, ckfn, max);
}

/* Walk through the chain starting at objchn and ask for all objects
   with olet in olets (if nonNULL) and satisfying ckfn (if nonNULL)
   whether the action in question (i.e., fn) has to be performed.
   If allflag then no questions are asked. Max gives the max nr of
   objects treated.
 */
askchain(objchn, olets, allflag, fn, ckfn, max)
struct obj *objchn;
register char *olets;
int allflag;
int (*fn)(), (*ckfn)();
int max;
{
register struct obj *otmp, *otmp2;
register char sym, ilet;
register int cnt = 0;
	ilet = 'a'-1;
	for(otmp = objchn; otmp; otmp = otmp2){
		if(ilet == 'z') ilet = 'A'; else ilet++;
		otmp2 = otmp->nobj;
		if(olets && *olets && !index(olets, otmp->olet)) continue;
		if(ckfn && !(*ckfn)(otmp)) continue;
		if(!allflag) {
			prname(otmp, ilet, 1);
			addtopl(" (ynaq)? ");
			sym = readchar();
		}
		else	sym = 'y';

		switch(sym){
		case 'a':
			allflag = 1;
		case 'y':
			cnt += (*fn)(otmp);
			if(--max == 0) goto ret;
		case 'n':
		default:
			break;
		case 'q':
			goto ret;
		}
	}
	pline(cnt ? "That was all." : "No applicable objects.");
ret:
	if(!flags.echo) echo(OFF);
	return(cnt);
}

obj_to_let(obj)
register struct obj *obj;
{
	register struct obj *otmp;
	register char ilet = 'a';

	for(otmp = invent; otmp && otmp != obj; otmp = otmp->nobj)
		if(++ilet > 'z') ilet = 'A';
	return(otmp ? ilet : 0);
}

prinv(obj)
register struct obj *obj;
{
	prname(obj, obj_to_let(obj), 1);
}

prname(obj,let,onelin)
register struct obj *obj;
register char let;
{
	char li[BUFSZ];

	(void) sprintf(li, " %c - %s.", let, doname(obj));
	switch(onelin) {
	case 1:
		pline(li+1);
		break;
	case 0:
		puts(li+1);
		break;
	case -1:
		cl_end();
		fputs(li, stdout);
		curx += strlen(li);
	}
}

ddoinv()
{
	doinv((char *) 0);
	return(0);
}

/* called with "": all objects in inventory; with 0: also usedup ones */
/* otherwise: all objects with (serial) letter in lets */
doinv(lets) register char *lets; {
	register struct obj *otmp;
	register char ilet = 'a';
	int ct = 0;
	int maxlth = 0;
	int lth;

	if(!invent){
		pline("Not carrying anything");
		if(lets) return;
	}
	if(!flags.oneline) {
	    if(!lets || !*lets)
		for(otmp = invent; otmp; otmp = otmp->nobj) ct++;
	    else
		ct = strlen(lets);
	    if(ct > 1 && ct < ROWNO && (lets || !inshop())){
		for(otmp = invent; otmp; otmp = otmp->nobj) {
		    if(!lets || !*lets || index(lets, ilet)) {
			lth = strlen(doname(otmp));
			if(lth > maxlth) maxlth = lth;
		    }
		    if(++ilet > 'z') ilet = 'A';
		}
		ilet = 'a';
		lth = COLNO - maxlth - 7;
		if(lth < 10) goto clrscr;
		home();
		cl_end();
		flags.topl = 0;
		ct = 0;
		for(otmp = invent; otmp; otmp = otmp->nobj) {
		    if(!lets || !*lets || index(lets, ilet)) {
			curs(lth, ++ct);
			prname(otmp, ilet, -1);
		    }
		    if(++ilet > 'z') ilet = 'A';
		}
		curs(lth, ct+1);
		cl_end();
		cmore();	/* sets morc */
		/* test whether morc is a reasonable answer */
		if(lets && *lets && !index(lets, morc)) morc = 0;

		home();
		cl_end();
		docorner(lth, ct);
		return;
	    }
	}
    clrscr:
	if(ct > 1) cls();
	for(otmp = invent; otmp; otmp = otmp->nobj){
		if(!lets || !*lets || index(lets, ilet))
			prname(otmp, ilet, (ct > 1) ? 0 : 1);
		if(++ilet > 'z') ilet = 'A';
	}
	/* tell doinvbill whether we cleared the screen */
	if(!lets) doinvbill((ct > 1));
	if(ct > 1){
		cgetret();
		docrt();
	} else
 morc = 0;	/* %% */
}

stackobj(obj) register struct obj *obj; {
register struct obj *otmp = fobj;
	for(otmp = fobj; otmp; otmp = otmp->nobj) if(otmp != obj)
	if(otmp->ox == obj->ox && otmp->oy == obj->oy &&
		merged(obj,otmp,1))
			return;
}

/* merge obj with otmp and delete obj if types agree */
merged(otmp,obj,lose) register struct obj *otmp, *obj; {
	if(otmp->otyp == obj->otyp &&
	  obj->unpaid == otmp->unpaid && obj->spe == otmp->spe &&
	  obj->known == otmp->known && obj->dknown == otmp->dknown &&
	  obj->cursed == otmp->cursed &&
	  ((obj->olet == WEAPON_SYM && obj->otyp < BOOMERANG)
	  || index("%?!*",otmp->olet))){
		otmp->quan += obj->quan;
		otmp->owt += obj->owt;
		if(lose) freeobj(obj);
		obfree(obj,otmp);	/* free(obj), bill->otmp */
		return(1);
	} else	return(0);
}

doprwep(){
	if(!uwep) pline("You are empty handed.");
	else prinv(uwep);
	return(0);
}

doprarm(){
	if(!uarm && !uarmg && !uarms && !uarmh)
		pline("You are not wearing any armor.");
	else {
		char lets[6];
		register int ct = 0;

		if(uarm) lets[ct++] = obj_to_let(uarm);
		if(uarm2) lets[ct++] = obj_to_let(uarm2);
		if(uarmh) lets[ct++] = obj_to_let(uarmh);
		if(uarms) lets[ct++] = obj_to_let(uarms);
		if(uarmg) lets[ct++] = obj_to_let(uarmg);
		lets[ct] = 0;
		doinv(lets);
	}
 return(0);
}

doprring(){
	if(!uleft && !uright)
		pline("You are not wearing any rings.");
	else {
		char lets[3];
		register int ct = 0;

		if(uleft) lets[ct++] = obj_to_let(uleft);
		if(uright) lets[ct++] = obj_to_let(uright);
		lets[ct] = 0;
		doinv(lets);
	}
 return(0);
}

digit(c) char c; {
	return(c >= '0' && c <= '9');
}
