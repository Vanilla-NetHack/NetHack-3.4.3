/*	SCCS Id: @(#)write.c	3.0	89/01/10
 */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/*
 * returns basecost of a scroll
 */
static int
cost(scroll)
register struct obj *scroll;
{
	switch(scroll->otyp)  {
# ifdef MAIL
	case SCR_MAIL:
		return(2);
/*		break; */
# endif
	case SCR_LIGHT:
	case SCR_GOLD_DETECTION:
	case SCR_FOOD_DETECTION:
	case SCR_MAGIC_MAPPING:
	case SCR_AMNESIA:
	case SCR_FIRE:
		return(8);
/*		break; */
	case SCR_DESTROY_ARMOR:
	case SCR_CREATE_MONSTER:
	case SCR_PUNISHMENT:
		return(10);
/*		break; */
	case SCR_CONFUSE_MONSTER:
		return(12);
/*		break; */
	case SCR_IDENTIFY:
		return(14);
/*		break; */
	case SCR_ENCHANT_ARMOR:
	case SCR_REMOVE_CURSE:
	case SCR_ENCHANT_WEAPON:
	case SCR_CHARGING:
		return(16);
/*		break; */
	case SCR_SCARE_MONSTER:
	case SCR_TAMING:
	case SCR_TELEPORTATION:
		return(20);
/*		break; */
	case SCR_GENOCIDE:
		return(30);
/*		break; */
	case SCR_BLANK_PAPER:
	default:
		impossible("You can't write such a weird scroll!");
	}
	return(1000);
}

static const char write_on[] = { SCROLL_SYM, 0 };

void
dowrite(pen)
register struct obj *pen;
{
	register struct obj *paper;
	char namebuf[BUFSZ], scrbuf[BUFSZ];
	register struct obj *newscroll;
	int basecost, actualcost;
	int newquan;
	int curseval;
	
	if(!pen)
		return;
	if(pen->otyp != MAGIC_MARKER)  {
		You("can't write with that!");
		return;
	}
	
	/* get paper to write on */
	paper = getobj(write_on,"write on");
	if(!paper)
		return;
	if(!(objects[paper->otyp].oc_name_known))  {
		pline("In your haste, you rip the scroll to pieces.");
		useup(paper);
		return;
	}
	if(paper->otyp != SCR_BLANK_PAPER)  {
		You("fool, that scroll's not blank!");
		return;
	}
	
	/* what to write */
	pline("What type of scroll do you want to write? ");
	getlin(namebuf);
	if(namebuf[0] == '\033' || !namebuf[0])
		return;
	scrbuf[0] = '\0';
	if(strncmp(namebuf,"scroll of ",10) != 0)
		Strcpy(scrbuf,"scroll of ");
	Strcat(scrbuf,namebuf);
	newscroll = readobjnam(scrbuf);

	newscroll->bknown = (paper->bknown && pen->bknown);
	/* do this now, before we useup() the paper */

	if(newscroll->olet != SCROLL_SYM ||
	   newscroll->otyp == SCR_BLANK_PAPER)  {
		You("can't write that!");
		pline("It's obscene!");
		return;
	}
	
	/* see if there's enough ink */
	basecost = cost(newscroll);
	if(pen->spe < basecost/2)  {
		Your("marker is too dry to write that!");
		obfree(newscroll, (struct obj *) 0);
		return;
	}
	
	/* we're really going to write now, so calculate
	 * cost and useup old scroll
	 */
	actualcost = rn1(basecost/2,basecost/2);
	curseval = bcsign(pen) + bcsign(paper);
	useup(paper);
	
	/* dry out marker */
	if(pen->spe < actualcost)  {
		Your("marker dries out!");
		pline("The scroll is now useless and disappears!");
		pen->spe = 0;
		obfree(newscroll, (struct obj *) 0);
		return;
	}
	pen->spe -= actualcost;
	
	/* can't write if we don't know it - unless we're lucky */
	if(!(objects[newscroll->otyp].oc_name_known) && 
	   !(objects[newscroll->otyp].oc_uname) && 
	   (rnl(pl_character[0] == 'W' ? 3 : 15))) {
		You("don't know how to write that!");
		You("write \"%s was here!\" and the scroll disappears.",
			plname);
		obfree(newscroll, (struct obj *) 0);
		return;
	}
	
	/* and now you know it! */
	makeknown(newscroll->otyp);
	
	/* success - don't forget to fool prinv() */
	newscroll = addinv(newscroll);
	newquan = newscroll->quan;
	newscroll->quan = 1;
	newscroll->blessed = (curseval > 0);
	newscroll->cursed = (curseval < 0);
	prinv(newscroll);
	newscroll->quan = newquan;
#ifdef MAIL
	if (newscroll->otyp == SCR_MAIL) newscroll->spe = 1;
#endif
	newscroll->known = 1;
}
