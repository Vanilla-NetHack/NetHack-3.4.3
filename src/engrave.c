/*	SCCS Id: @(#)engrave.c	3.0	89/06/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

static void del_engr P((struct engr *));

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
#define MARK	4
#define POLY	5	/* temporary type - for polymorphing engraving */
} *head_engr;

/* random engravings */
const char *random_engr[] =
			 {"Elbereth", "ad ae?ar um",
			 "?la? ?as he??",
			 /* more added by Eric Backus */
			 "?ilroy wa? h?re", "?ala??iel",
			 "Fo? a ?ood time c?ll 6?6-4311",
			 /* some other famous engravings -3. */
			 "Lasc?ate o?ni sp?ranz? o vo? c?'en?rate",
			 "Y?u won?t get i? up ?he ste?s",
			 "A.S. ->"};

static struct engr *
engr_at(x,y) register xchar x,y; {
register struct engr *ep = head_engr;
	while(ep) {
		if(x == ep->engr_x && y == ep->engr_y)
			return(ep);
		ep = ep->nxt_engr;
	}
	return((struct engr *) 0);
}

#ifdef ELBERETH
int
sengr_at(s,x,y)
	register char *s;
	register xchar x,y;
{
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
#endif

void
u_wipe_engr(cnt)
register int cnt;
{
	if(!u.uswallow && !Levitation)
		wipe_engr_at(u.ux, u.uy, cnt);
}

void
wipe_engr_at(x,y,cnt) register xchar x,y,cnt; {
register struct engr *ep = engr_at(x,y);
register int lth,pos;
char ch;
	if(ep){
	    if(ep->engr_type != BURN) {
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
}

void
read_engr_at(x,y) register int x,y; {
register struct engr *ep = engr_at(x,y);
register int	canfeel;
	if(ep && ep->engr_txt[0]) {
	    switch(ep->engr_type) {
	    case DUST:
		if(!Blind) pline("Something is written here in the dust.");
		canfeel = 0;
		break;
	    case ENGRAVE:
		pline("Something is engraved here on the floor.");
		canfeel = 1;
		break;
	    case BURN:
		pline("Some text has been burned here in the floor.");
		canfeel = 1;
		break;
	    case MARK:
		if(!Blind) pline("There's some graffiti here on the floor.");
		canfeel = 0;
		break;
	    default:
		impossible("Something is written in a very strange way.");
		canfeel = 1;
	    }
	    if (canfeel || !Blind)
		You("%s: \"%s\".",
		      (Blind) ? "feel the words" : "read",  ep->engr_txt);
	}
}

void
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
	Strcpy(ep->engr_txt, s);
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
	   (!bimanual(uwep) && (!uarms || !uarms->cursed)));
/*	if ((uwep && bimanual(uwep)) ||
	    (uwep && uarms))
		return(0);
	else
		return(1);*/
}

static const char styluses[] = { '#', '-', TOOL_SYM, WEAPON_SYM, WAND_SYM, 0 };
static const char too_large[] = { ARMOR_SYM, BALL_SYM, ROCK_SYM, 0 };
static const char paper[] = { SCROLL_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	0 };

int
doengrave(){
register int len, tmp;
register char *sp, *sptmp;
register struct engr *ep, *oep = engr_at(u.ux,u.uy);
char buf[BUFSZ];
xchar type, polytype = 0;
int spct;		/* number of leading spaces */
register struct obj *otmp;
	multi = 0;

	if(u.uswallow) {
		pline("What would you write?  \"Jonah was here\"?");
		return(0);
	}

	/* one may write with finger, weapon or wand */
	/* edited by GAN 10/20/86 so as not to change
	 * weapon wielded.
	 */
	otmp = getobj(styluses, "write with");
	if(!otmp) return(0);

	/* There's no reason you should be able to write with a wand
	 * while both your hands are tied up.
	 */
	if (!freehand() && otmp != uwep && !otmp->owornmask) {
		You("have no free %s to write with!", body_part(HAND));
		return(0);
	}
#ifdef POLYSELF
	if (cantwield(uasmon)) {
		You("can't even hold anything!");
		return(0);
	}
#endif
	if(otmp == ublindf) {
		pline("That is a bit difficult to engrave with, don't you think?");
		return(1);
	}
	if(otmp != &zeroobj && index(too_large,otmp->olet)) {
		You("can't engrave with such a large object!");
		return(1);
	}

	if(otmp != &zeroobj && index(paper,otmp->olet)) {
		Your("%s would get dirty.",xname(otmp));
		return(1);
	}

	if(Levitation && otmp->olet != WAND_SYM){		/* riv05!a3 */
		You("can't reach the floor!");
		return(0);
	}

	if(otmp == &zeroobj) {
		You("write in the dust with your %s.",
			makeplural(body_part(FINGER)));
		type = DUST;
	} else if(otmp->olet == WAND_SYM && zappable(otmp)) {
		/* changed so any wand gets zapped out */
		if((objects[otmp->otyp].bits & NODIR))  {
			zapnodir(otmp);
			type = DUST;
		}  else  {
			switch(otmp->otyp)  {
			case WAN_LIGHTNING:
				if(!objects[otmp->otyp].oc_name_known) {
				    if(flags.verbose)
					pline("The %s is a wand of lightning!",
						xname(otmp));
				    makeknown(otmp->otyp);
				    more_experienced(0,10);
				}
				type = BURN;
				break;
			case WAN_FIRE:
				if(!objects[otmp->otyp].oc_name_known) {
				    if(flags.verbose)
					pline("The %s is a wand of fire!",
					   xname(otmp));
				    makeknown(otmp->otyp);
				    more_experienced(0,10);
				}
				type = BURN;
				break;
			case WAN_DIGGING:
				if(!objects[otmp->otyp].oc_name_known) {
				    if(flags.verbose)
					pline("The %s is a wand of digging!",
					   xname(otmp));
				    makeknown(otmp->otyp);
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
				/* break; */
			case WAN_TELEPORTATION:
				if(!oep)
					type = DUST;
				else  {
					register int tx,ty;

					do  {
						tx = rn1(COLNO-3,2);
						ty = rn2(ROWNO);
					}  while(!goodpos(tx,ty,(struct permonst *)0));
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
			You("write in the dust with %s.",
			   doname(otmp));

	} else {
		if((otmp->otyp >= DAGGER && otmp->otyp <= AXE) ||
#ifdef WORM
		   otmp->otyp == CRYSKNIFE ||
#endif
		   is_sword(otmp)) {
			type = ENGRAVE;

			if((int)otmp->spe <= -3) {
				Your("%s too dull for engraving.",
					aobjnam(otmp, "are"));
				type = DUST;
				/* following messaged added 10/20/86 - GAN */
				You("write in the dust with %s.",
				   doname(otmp));
			}  else
				You("engrave with %s.", doname(otmp));
		} else if(otmp->otyp == MAGIC_MARKER)  {
			if(otmp->spe <= 0)  {
				Your("marker is dried out.");
				You("write in the dust with the marker.");
				type = DUST;
			}  else  {
				You("write with %s.", doname(otmp));
				type = MARK;
			}
		}  else  {
			You("write in the dust with %s.",
			   doname(otmp));
			type = DUST;
		}
	}

	if(type != POLY && oep && oep->engr_type == DUST){
		  You("wipe out the message that was written here.");
		  del_engr(oep);
		  oep = 0;
	}
	if(type == DUST && oep) {
	    You("cannot wipe out the message that is %s in the rock.",
		  (oep->engr_type == BURN) ? "burned" :
		  (oep->engr_type == ENGRAVE) ? "engraved" : "scribbled");
		  return(1);
	}
	if(type == POLY)  {
		polytype = rnd(4);
		Strcpy(buf,random_engr[rn2(SIZE(random_engr))]);
		switch(polytype){
		case DUST:
			pline("\"%s\" is now written on the ground.",buf);
			break;
		case ENGRAVE:
			pline("\"%s\" is now engraved in the rock.",buf);
			break;
		case BURN:
			pline("\"%s\" is now burned in the rock.",buf);
			break;
		case MARK:
			pline("\"%s\" is now scribbled on the rock.",buf);
			break;
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
		return(1);
	}
	if(otmp->otyp == WAN_FIRE) {
		if (!Blind) pline("Flames fly from the wand.");
		else You("feel the wand heat up.");
	} else if(otmp->otyp == WAN_LIGHTNING) {
		if (!Blind) {
			pline("Lightning arcs from the wand.");
			You("are blinded by the flash!");
			make_blinded((long)rnd(50),FALSE);
		} else You("hear crackling!");
	} else if(otmp->otyp == WAN_DIGGING) {
		if (!Blind) pline("Gravel flies up from the floor.");
		else You("hear drilling!");
  	}
		/* kludge by stewr 870708 */
	for (sptmp = sp, tmp=0; !(tmp == len); sptmp++,tmp++) {
		if (((type == DUST) && !rn2(25))
		     || (Blind && !rn2(12))
		     || (Confusion && !rn2(3))) {
			 *sptmp = '!' + rn2(93); /* ASCII-code only */
		       }
	      }

	switch(type) {
	case DUST:
	case BURN:
		if(len > 15) {
			multi = -(len/10);
			nomovemsg = "You finish writing.";
		}
		break;
	case ENGRAVE:
	case MARK:
		{	int len2;

			if(type == ENGRAVE)
				len2 = (otmp->spe + 3) * 2 + 1;
			else
				len2 = (otmp->spe) * 2;
			nomovemsg = "You finish writing.";
			if(type != MARK)
			nomovemsg = "You finish engraving.";
			if(otmp->olet != WAND_SYM)  {
				if(otmp->olet == WEAPON_SYM)
					Your("%s dull.",
					       aobjnam(otmp, "get"));
				if(len2 < len) {
					len = len2;
					sp[len] = 0;
					if(type == ENGRAVE)  {
						otmp->spe = -3;
					}  else  {
						Your("marker dries out!");
						otmp->spe = 0;
					}
					/* next line added by GAN 10/20/86 */
					You("only write \"%s\".", sp);
					nomovemsg = "You cannot write more.";
				} else
					otmp->spe -= len >> 1;
				if(type == MARK)
					multi = -(len/10);
				else
					multi = -len;
			}  else
				multi = -(len/10);
			if (multi == 0)
				nomovemsg = (char *)0;
		}
		break;
	case POLY:
		type = polytype;
		multi = 0;
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
		Strcpy(sp, oep->engr_txt);
		Strcat(sp, buf);
		del_engr(oep);
	} else
		Strcpy(sp, buf);
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

void
save_engravings(fd) int fd; {
register struct engr *ep = head_engr;
register struct engr *ep2;
	while(ep) {
	    ep2 = ep->nxt_engr;
	    if(ep->engr_lth && ep->engr_txt[0]){
		bwrite(fd, (genericptr_t)&(ep->engr_lth), sizeof(ep->engr_lth));
		bwrite(fd, (genericptr_t)ep, sizeof(struct engr) + ep->engr_lth);
	    }
#if defined(DGK) && !defined(OLD_TOS)
	    if (!count_only)
#endif
		free((genericptr_t) ep);
	    ep = ep2;
	}
	bwrite(fd, (genericptr_t)nul, sizeof(unsigned));
#if defined(DGK) && !defined(OLD_TOS)
	if (!count_only)
#endif
		head_engr = 0;
}

void
rest_engravings(fd) int fd; {
register struct engr *ep;
unsigned lth;
	head_engr = 0;
	while(1) {
		mread(fd, (genericptr_t) &lth, sizeof(unsigned));
		if(lth == 0) return;
		ep = (struct engr *) alloc(sizeof(struct engr) + lth);
		mread(fd, (genericptr_t) ep, sizeof(struct engr) + lth);
		ep->nxt_engr = head_engr;
		ep->engr_txt = (char *) (ep + 1);	/* Andreas Bormann */
		head_engr = ep;
	}
}

static void
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
	free((genericptr_t) ep);
}
