/*	SCCS Id: @(#)engrave.c	1.3	87/07/14
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* engrave.c - version 1.0.2 */

#include	"hack.h"

extern char *nomovemsg;
extern char nul[];
extern struct obj zeroobj;
#ifdef KAA
extern char *xname();
#endif
struct engr {
	struct engr *nxt_engr;
	char *engr_txt;
	xchar engr_x, engr_y;
	unsigned engr_lth;	/* for save & restore; not length of text */
	long engr_time;	/* moment engraving was (will be) finished */
	xchar engr_type;
#define	DUST	1
#define	ENGRAVE	2
#define	BURN	3
#ifdef MARKER
#define MARK	4
#define POLY	5	/* temporary type - for polymorphing engraving */
#else
#define POLY	4	/* temporary type - for polymorphing engraving */
#endif
} *head_engr;

/* random engravings */
#ifdef KAA
char *random_engr[] =
#else
char random_engr[][30] =
#endif
			 {"Elbereth", "ad ae?ar um",
#ifdef NEWCLASS
			 "?la? ?as he??",
#endif
			/* more added by Eric Backus */
			"?ilroy wa? h?re", "?ala??iel", "Aba?don H?pe...",
			"Fo? a ?ood time c?ll 6?6-4311"};
#ifdef NEWCLASS
#define RAND_ENGRS	7
#else
#define RAND_ENGRS	6
#endif

struct engr *
engr_at(x,y) register xchar x,y; {
register struct engr *ep = head_engr;
	while(ep) {
		if(x == ep->engr_x && y == ep->engr_y)
			return(ep);
		ep = ep->nxt_engr;
	}
	return((struct engr *) 0);
}

sengr_at(s,x,y) register char *s; register xchar x,y; {
register struct engr *ep = engr_at(x,y);
register char *t;
register int n;
	if(ep && ep->engr_time <= moves) {
		t = ep->engr_txt;
/*
		if(!strcmp(s,t)) return(1);
*/
		n = strlen(s);
		while(*t) {
			if(!strncmp(s,t,n)) return(1);
			t++;
		}
	}
	return(0);
}

u_wipe_engr(cnt)
register int cnt;
{
	if(!u.uswallow && !Levitation)
		wipe_engr_at(u.ux, u.uy, cnt);
}

wipe_engr_at(x,y,cnt) register xchar x,y,cnt; {
register struct engr *ep = engr_at(x,y);
register int lth,pos;
char ch;
	if(ep){
		if(ep->engr_type != DUST) {
			cnt = rn2(1 + 50/(cnt+1)) ? 0 : 1;
		}
		lth = strlen(ep->engr_txt);
		if(lth && cnt > 0 ) {
			while(cnt--) {
				pos = rn2(lth);
				if((ch = ep->engr_txt[pos]) == ' ')
					continue;
				ep->engr_txt[pos] = (ch != '?') ? '?' : ' ';
			}
		}
		while(lth && ep->engr_txt[lth-1] == ' ')
			ep->engr_txt[--lth] = 0;
		while(ep->engr_txt[0] == ' ')
			ep->engr_txt++;
		if(!ep->engr_txt[0]) del_engr(ep);
	}
}

read_engr_at(x,y) register int x,y; {
register struct engr *ep = engr_at(x,y);
	if(ep && ep->engr_txt[0]) {
	    switch(ep->engr_type) {
	    case DUST:
		pline("Something is written here in the dust.");
		break;
	    case ENGRAVE:
		pline("Something is engraved here on the floor.");
		break;
	    case BURN:
		pline("Some text has been burned here in the floor.");
#ifdef MARKER
	    case MARK:
		pline("There's some graffiti here on the floor.");
		break;
#endif
		break;
	    default:
		impossible("Something is written in a very strange way.");
	    }
	    pline("You read: \"%s\".", ep->engr_txt);
	}
}

make_engr_at(x,y,s)
register int x,y;
register char *s;
{
	register struct engr *ep;

	if(ep = engr_at(x,y))
	    del_engr(ep);
	ep = (struct engr *)
	    alloc((unsigned)(sizeof(struct engr) + strlen(s) + 1));
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = x;
	ep->engr_y = y;
	ep->engr_txt = (char *)(ep + 1);
	(void) strcpy(ep->engr_txt, s);
	ep->engr_time = 0;
	ep->engr_type = DUST;
	ep->engr_lth = strlen(s) + 1;
}
/*
 *	freehand - returns true if player has a free hand
 */
int
freehand(){

	return(!uwep ||
	   !uwep->cursed ||
	   (uwep->otyp != TWO_HANDED_SWORD && (!uarms || !uarms->cursed)));
/*	if ((uwep && uwep->otyp == TWO_HANDED_SWORD) ||
	    (uwep && uarms))
		return(0);
	else
		return(1);*/
}



doengrave(){
register int len;
register char *sp;
register struct engr *ep, *oep = engr_at(u.ux,u.uy);
char buf[BUFSZ];
xchar type;
int spct;		/* number of leading spaces */
register struct obj *otmp;
	multi = 0;

	if(u.uswallow) {
		pline("You're joking. Hahaha!");	/* riv05!a3 */
		return(0);
	}

	/* one may write with finger, weapon or wand */
	/* edited by GAN 10/20/86 so as not to change
	 * weapon wielded.
	 */
	otmp = getobj("#-()/", "write with");
	if(!otmp) return(0);

#ifdef FREEHAND /* There's no reason you should be able to write with a wand
		 * while both your hands are tied up.  Also, it's necessary to
		 * prevent engraving with "worn" objects other than weapons.
		 */
	if (!freehand() && otmp != uwep) {
#else
	/* added by GAN 10/20/86 to require you to need a hand to
	   write with.
	 */
	if(!(otmp->owornmask || otmp->olet == WAND_SYM) && !freehand())  {
#endif
		pline("You have no free hand to write with!");
		return(0);
	}
#ifdef KAA
	if (cantwield(u.usym)) {
		pline("You can't even hold anything!");
		return(0);
	}
	if(otmp != &zeroobj && index("][0`",otmp->olet)) {
		pline("You can't engrave with such a large object!");
		return(1);
	}
#endif

	if(Levitation && otmp->olet != WAND_SYM){		/* riv05!a3 */
		pline("You can't reach the floor!");
		return(0);
	}

	if(otmp == &zeroobj) {
		pline("You write in the dust with your fingers.");
		type = DUST;
	} else if(otmp->olet == WAND_SYM && zappable(otmp)) {
		/* changed so any wand gets zapped out, but fire
		 * wands become known.
		 */
		if((objects[otmp->otyp].bits & NODIR))  {
			zapnodir(otmp);
			type = DUST;
		}  else  {
			switch(otmp->otyp)  {
			case WAN_FIRE:
				if(!objects[otmp->otyp].oc_name_known) {
					pline("The %s is a wand of fire!",
					   xname(otmp));
					objects[otmp->otyp].oc_name_known = 1;
					more_experienced(0,10);
				}
				type = BURN;
				break;
			case WAN_DIGGING:
				if(!objects[otmp->otyp].oc_name_known) {
					pline("The %s is a wand of digging!",
					   xname(otmp));
					objects[otmp->otyp].oc_name_known = 1;
					more_experienced(0,10);
				}
				type = ENGRAVE;
				break;
			case WAN_POLYMORPH:
				if(oep)  {
					del_engr(oep);
					oep = 0;
					type = POLY;
				}  else
					type = DUST;
				break;
			case WAN_COLD:
				type = DUST;
				if(!oep || (oep->engr_type != BURN))
					break;
			case WAN_CANCELLATION:
			case WAN_MAKE_INVISIBLE:
				if(!oep) {		/* Eric Backus */
					type = DUST;
					break;
				}
				del_engr(oep);
				pline("The engraving on the floor vanishes!");
				return(1);
				break;
			case WAN_TELEPORTATION:
				if(!oep)
					type = DUST;
				else  {
					register tx,ty;

					do  {
						tx = rn1(COLNO-3,2);
						ty = rn2(ROWNO);
					}  while(!goodpos(tx,ty));
					oep->engr_x = tx;
					oep->engr_y = ty;
					pline("The engraving on the floor vanishes!");
					return(1);
				}
				break;
			default:
				type = DUST;
			}
		}
		if(type == DUST)
			pline("You write in the dust with %s.",
			   doname(otmp));
	
	} else {
		if(otmp->otyp == DAGGER || otmp->otyp == TWO_HANDED_SWORD ||
		otmp->otyp == CRYSKNIFE || otmp->otyp == KATANA ||
		otmp->otyp == SCIMITAR || otmp->otyp == BROAD_SWORD ||
		otmp->otyp == SHORT_SWORD ||
		otmp->otyp == LONG_SWORD || otmp->otyp == AXE) {
			type = ENGRAVE;
			if((int)otmp->spe <= -3) {
				pline("Your %s too dull for engraving.",
					aobjnam(otmp, "are"));
				type = DUST;
				/* following messaged added 10/20/86 - GAN */
				pline("You write in the dust with %s.",
				   doname(otmp));
			}  else
				pline("You engrave with %s.", doname(otmp));
#ifdef MARKER
		} else if(otmp->otyp == MAGIC_MARKER)  {
			if(otmp->spe <= 0)  {
				pline("Your marker is dried out.");
				pline("You write in the dust with the marker.");
				type = DUST;
			}  else  {
				pline("You write with %s.", doname(otmp));
				type = MARK;
			}
#endif
		}  else  {
			pline("You write in the dust with %s.",
			   doname(otmp));
			type = DUST;
		}
	}
	
	if(type != POLY && oep && oep->engr_type == DUST){
		  pline("You wipe out the message that was written here.");
		  del_engr(oep);
		  oep = 0;
	}
	if(type == DUST && oep){
	pline("You cannot wipe out the message that is %s in the rock.",
		    (oep->engr_type == BURN) ? "burned" : (oep->engr_type == ENGRAVE)? "engraved" : "scribbled");
		  return(1);
	}
	if(type == POLY)  {
#ifdef MARKER
		type = rnd(4);
#else
		type = rnd(3);
#endif
		strcpy(buf,random_engr[rn2(RAND_ENGRS)]);
		switch(type){
		case DUST:
			pline("\"%s\" is now written on the ground.",buf);
			break;
		case ENGRAVE:
			pline("\"%s\" is now engraved in the rock.",buf);
			break;
		case BURN:
			pline("\"%s\" is now burned in the rock.",buf);
			break;
#ifdef MARKER
		case MARK:
			pline("\"%s\" is now scribbled on the rock.",buf);
			break;
#endif
		default:
			impossible("\"%s\" is now written in a very strange way.",
			   buf);
		}
	}  else  {
		pline("What do you want to %s on the floor here? ",
		  (type == ENGRAVE) ? "engrave" : (type == BURN) ? "burn" : "write");
		getlin(buf);
		clrlin();
	}
	spct = 0;
	sp = buf;
	while(*sp == ' ') spct++, sp++;
	len = strlen(sp);
	if(!len || *buf == '\033') {
		/* changed by GAN 11/01/86 to not recharge wand */
		if(type == BURN)
			pline("A few sparks fly from the wand of fire.");
		else
			if(otmp->otyp == WAN_DIGGING)
				pline("Gravel flies up from the floor.");
		return(1);
	}
	
	switch(type) {
	case DUST:
	case BURN:
		if(len > 15) {
			multi = -(len/10);
			nomovemsg = "You finished writing.";
		}
		break;
	case ENGRAVE:
#ifdef MARKER
	case MARK:
		{	int len2;
		
			if(type == ENGRAVE)
				len2 = (otmp->spe + 3) * 2 + 1;
			else
				len2 = (otmp->spe) * 2;
			nomovemsg = "You finished writing.";
			if(type != MARK)
#else
		{	int len2 = (otmp->spe + 3) * 2 + 1;
#endif
			nomovemsg = "You finished engraving.";
			if(otmp->olet != WAND_SYM)  {
				if(otmp->olet == WEAPON_SYM)
					pline("Your %s dull.",
					       aobjnam(otmp, "get"));
				if(len2 < len) {
					len = len2;
					sp[len] = 0;
					if(type == ENGRAVE)  {
						otmp->spe = -3;
					}  else  {
						pline("Your marker dries out!");
						otmp->spe = 0;
					}
					/* next line added by GAN 10/20/86 */
					pline("You only write \"%s\".", sp);
					nomovemsg = "You cannot write more.";
				} else
					otmp->spe -= len/2;
#ifdef MARKER
				if(type == MARK)
					multi = -(len/10);
				else
#endif
					multi = -len;
			}  else
				multi = -(len/10);
		}
		break;
	}
	if(oep) len += strlen(oep->engr_txt) + spct;
	ep = (struct engr *) alloc((unsigned)(sizeof(struct engr) + len + 1));
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = u.ux;
	ep->engr_y = u.uy;
	sp = (char *)(ep + 1);	/* (char *)ep + sizeof(struct engr) */
	ep->engr_txt = sp;
	if(oep) {
		(void) strcpy(sp, oep->engr_txt);
		(void) strcat(sp, buf);
		del_engr(oep);
	} else
		(void) strcpy(sp, buf);
	ep->engr_lth = len+1;
	ep->engr_type = type;
	ep->engr_time = moves-multi;

	/* kludge to protect pline against excessively long texts */
	if(len > BUFSZ-20) sp[BUFSZ-20] = 0;
	
	/* cute messages for odd wands */
	switch(otmp->otyp)  {
	case WAN_SLOW_MONSTER:
		pline("The bugs on the ground slow down!");
		break;
	case WAN_SPEED_MONSTER:
		pline("The bugs on the ground speed up!");
		break;
	case WAN_MAGIC_MISSILE:
		pline("The ground is riddled by bullet holes!");
		break;
	case WAN_SLEEP:
	case WAN_DEATH:	/* can't tell sleep from death - Eric Backus */
		pline("The bugs on the ground stop moving!");
		break;
	case WAN_COLD:
		pline("A few ice cubes drop from your %s.",xname(otmp));
		break;
	case WAN_STRIKING:
		pline("The %s unsuccessfully fights your attempt to write!",xname(otmp));
	}

	return(1);
}

save_engravings(fd) int fd; {
register struct engr *ep = head_engr;
	while(ep) {
		if(!ep->engr_lth || !ep->engr_txt[0]){
			ep = ep->nxt_engr;
			continue;
		}
		bwrite(fd, (char *) & (ep->engr_lth), sizeof(ep->engr_lth));
		bwrite(fd, (char *) ep, sizeof(struct engr) + ep->engr_lth);
		ep = ep->nxt_engr;
	}
	bwrite(fd, (char *) nul, sizeof(unsigned));
#ifdef DGK
	if (!count_only)
#endif
		head_engr = 0;
}

rest_engravings(fd) int fd; {
register struct engr *ep;
unsigned lth;
	head_engr = 0;
	while(1) {
		mread(fd, (char *) &lth, sizeof(unsigned));
		if(lth == 0) return;
		ep = (struct engr *) alloc(sizeof(struct engr) + lth);
		mread(fd, (char *) ep, sizeof(struct engr) + lth);
		ep->nxt_engr = head_engr;
		ep->engr_txt = (char *) (ep + 1);	/* Andreas Bormann */
		head_engr = ep;
	}
}

del_engr(ep) register struct engr *ep; {
register struct engr *ept;
	if(ep == head_engr)
		head_engr = ep->nxt_engr;
	else {
		for(ept = head_engr; ept; ept = ept->nxt_engr) {
			if(ept->nxt_engr == ep) {
				ept->nxt_engr = ep->nxt_engr;
				goto fnd;
			}
		}
		impossible("Error in del_engr?");
		return;
	fnd:	;
	}
	free((char *) ep);
}
