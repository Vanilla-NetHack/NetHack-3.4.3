/*	SCCS Id: @(#)write.c	1.4	87/08/08
/* write.c - version 1.0.3 */

#include "hack.h"

extern char pl_character[];

#ifdef MARKER

/*
 * returns basecost of a scroll
 */
int
cost(scroll)
register struct obj *scroll;
{
	switch(scroll->otyp)  {
# ifdef MAIL
	case SCR_MAIL:
		return(0);
		break;
# endif
	case SCR_LIGHT:
	case SCR_GOLD_DETECTION:
	case SCR_FOOD_DETECTION:
	case SCR_MAGIC_MAPPING:
	case SCR_AMNESIA:
	case SCR_FIRE:
		return(8);
		break;
	case SCR_DESTROY_ARMOR:
	case SCR_DAMAGE_WEAPON:
	case SCR_CREATE_MONSTER:
	case SCR_PUNISHMENT:
		return(10);
		break;
	case SCR_CONFUSE_MONSTER:
		return(12);
		break;
	case SCR_IDENTIFY:
		return(14);
		break;
	case SCR_ENCHANT_ARMOR:
	case SCR_REMOVE_CURSE:
	case SCR_ENCHANT_WEAPON:
		return(16);
		break;
	case SCR_SCARE_MONSTER:
	case SCR_TAMING:
	case SCR_TELEPORTATION:
		return(20);
		break;
	case SCR_GENOCIDE:
		return(30);
		break;
	case SCR_BLANK_PAPER:
	default:
		impossible("You can't write such a weird scroll!");
		return(1000);
	}
}


dowrite(pen)
	register struct obj *pen;
{
	register struct obj *paper;
	char namebuf[BUFSZ], scrbuf[BUFSZ];
	register struct obj *newscroll;
	extern struct obj *readobjnam(), *addinv();
	int basecost, actualcost;
	int newquan;
	
	if(!pen)
		return(0);
	if(pen->otyp != MAGIC_MARKER)  {
		pline("You can't write with that!");
		return(0);
	}
	
	/* get paper to write on */
	paper = getobj("?","write on");
	if(!paper)
		return(0);
	if(!(objects[paper->otyp].oc_name_known))  {
		pline("In your haste, you rip the scroll to pieces.");
		useup(paper);
		return(1);
	}
# ifndef KAA
/* If this is included, the strategy would be to name all scrolls so that
 * you can test them for blankness with a magic marker.  This is tedious,
 * thus, let's make it easier. */
	if(!(objects[paper->otyp].oc_name_known))  {
		pline("In your haste, you rip the scroll to pieces.");
		useup(paper);
		return(0);
	}
# endif
	if(paper->otyp != SCR_BLANK_PAPER)  {
		pline("You fool, that scroll's not blank!");
		return(0);
	}
	
	/* what to write */
	pline("What do you want to write? ");
	getlin(namebuf);
	if(namebuf[0] == '\033' || !namebuf[0])
		return(0);
	strcpy(scrbuf,"scroll of ");
	strcat(scrbuf,namebuf);
	newscroll = readobjnam(scrbuf);
	if(newscroll->olet != SCROLL_SYM ||
	   newscroll->otyp == SCR_BLANK_PAPER)  {
		pline("You can't write that!");
		pline("It's obscene!");
		return(0);
	}
	
	/* see if there's enough ink */
	basecost = cost(newscroll);
	if(pen->spe < basecost/2)  {
		pline("You marker is too dried out to write that!");
		obfree(newscroll, (struct obj *) 0);
		return(0);
	}
	
	/* we're really going to write now, so calculate
	 * cost and useup old scroll
	 */
	actualcost = rn1(basecost/2,basecost/2);
	useup(paper);
	
	/* dry out marker */
	if(pen->spe < actualcost)  {
		pline("Your marker dries out!");
		pline("The scroll is now useless and disappears!");
		pen->spe = 0;
		obfree(newscroll, (struct obj *) 0);
		return(1);
	}
	pen->spe -= actualcost;
# ifdef KAA /* Since the KAA modification allows writing on unknown blank
		paper, identify blank paper. */
	objects[SCR_BLANK_PAPER].oc_name_known=1;
# endif
	
	/* can't write if we don't know it - unless we're lucky */
	if(!(objects[newscroll->otyp].oc_name_known) && 
# ifdef KAA
	   !(objects[newscroll->otyp].oc_uname) && 
# endif
	   ((pl_character[0] == 'W' && rn2(3)) ||
	    (pl_character[0] != 'W' && rn2(10))))  {
		pline("You don't know how to write that!");
		pline("You write \"Shah was here!\" and the scroll disappears.");
		obfree(newscroll, (struct obj *) 0);
		return(1);
	}
	
	/* and now you know it! */
	objects[newscroll->otyp].oc_name_known = 1;
	
	/* success - don't forget to fool prinv() */
	newscroll = addinv(newscroll);
	newquan = newscroll->quan;
	newscroll->quan = 1;
	prinv(newscroll);
	newscroll->quan = newquan;
	
	return(1);
}
# endif /* MARKER /**/
