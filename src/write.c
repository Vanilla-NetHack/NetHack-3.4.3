/*	SCCS Id: @(#)write.c	3.1	91/01/04
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static int FDECL(cost,(struct obj *));

/*
 * returns basecost of a scroll or a spellbook
 */
static int
cost(otmp)
register struct obj *otmp;
{

	if (otmp->oclass == SPBOOK_CLASS)
		return(10 * objects[otmp->otyp].oc_level);

	switch(otmp->otyp)  {
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

static NEARDATA const char write_on[] = { SCROLL_CLASS, SPBOOK_CLASS, 0 };

int
dowrite(pen)
register struct obj *pen;
{
	register struct obj *paper;
	char namebuf[BUFSZ], scrbuf[BUFSZ];
	register struct obj *new_obj;
	int basecost, actualcost;
	int curseval;
	char qbuf[QBUFSZ];
	
	if(!pen)
		return(0);
	/* already tested before only call of dowrite() (from doapply())
	if(pen->otyp != MAGIC_MARKER)  {
		You("can't write with that!");
		return(0);
	}
	*/

	/* get paper to write on */
	paper = getobj(write_on,"write on");
	if(!paper)
		return(0);
	if(Blind && !paper->dknown) {
		You("don't know if that %s is blank or not!",
		      paper->oclass == SPBOOK_CLASS ? "spellbook" :
		      "scroll");
		return(1);
	}
	paper->dknown = 1;
	if(paper->otyp != SCR_BLANK_PAPER && paper->otyp != SPE_BLANK_PAPER) {
		pline("That %s is not blank!",
		    paper->oclass == SPBOOK_CLASS ? "spellbook" :
		    "scroll");
		exercise(A_WIS, FALSE);
		return(1);
	}

	/* what to write */
	Sprintf(qbuf, "What type of %s do you want to write? ",
	      paper->oclass == SPBOOK_CLASS ? "spellbook" :
	      "scroll");
	getlin(qbuf, namebuf);
	if(namebuf[0] == '\033' || !namebuf[0])
		return(1);
	scrbuf[0] = '\0';
	if (paper->oclass == SPBOOK_CLASS) {
		if(strncmp(namebuf,"spellbook of ",13) != 0)
			Strcpy(scrbuf,"spellbook of ");
	}
	else if(strncmp(namebuf,"scroll of ",10) != 0)
		Strcpy(scrbuf,"scroll of ");
	Strcat(scrbuf,namebuf);
	new_obj = readobjnam(scrbuf);

	new_obj->bknown = (paper->bknown && pen->bknown);

	if((new_obj->oclass != SCROLL_CLASS ||
	              new_obj->otyp == SCR_BLANK_PAPER)
	    && (new_obj->oclass != SPBOOK_CLASS || 
                      new_obj->otyp == SPE_BLANK_PAPER)) {
		You("can't write that!");
		pline("It's obscene!");
		obfree(new_obj, (struct obj *) 0); /* pb@ethz.uucp */
		return(1);
	}

	/* see if there's enough ink */
	basecost = cost(new_obj);
	if(pen->spe < basecost/2)  {
		Your("marker is too dry to write that!");
		obfree(new_obj, (struct obj *) 0);
		return(1);
	}

	/* we're really going to write now, so calculate cost
	 */
	actualcost = rn1(basecost/2,basecost/2);
	curseval = bcsign(pen) + bcsign(paper);
	exercise(A_WIS, TRUE);
	/* dry out marker */
	if(pen->spe < actualcost)  {
		Your("marker dries out!");
		/* scrolls disappear, spellbooks don't */
		if (paper->oclass == SPBOOK_CLASS)
			pline("The spellbook is left unfinished.");
		else {
			pline("The scroll is now useless and disappears!");
			useup(paper);
		}
		pen->spe = 0;
		obfree(new_obj, (struct obj *) 0);
		return(1);
	}
	pen->spe -= actualcost;

	/* can't write if we don't know it - unless we're lucky */
	if(!(objects[new_obj->otyp].oc_name_known) && 
	   !(objects[new_obj->otyp].oc_uname) && 
	   (rnl(pl_character[0] == 'W' ? 3 : 15))) {
		You("don't know how to write that!");
		/* scrolls disappear, spellbooks don't */
		if (paper->oclass == SPBOOK_CLASS)
			You("write in your best handwriting:  \"My Diary\".");
		else {
			You("write \"%s was here!\" and the scroll disappears.",plname);
			useup(paper);
		}
		obfree(new_obj, (struct obj *) 0);
		return(1);
	}

	/* useup old scroll / spellbook */
	useup(paper);

	/* now you know it! */
	makeknown(new_obj->otyp);

	/* success */
	new_obj->blessed = (curseval > 0);
	new_obj->cursed = (curseval < 0);
#ifdef MAIL
	if (new_obj->otyp == SCR_MAIL) new_obj->spe = 1;
#endif
	new_obj = hold_another_object(new_obj, "Oops!  %s out of your grasp!",
					       The(aobjnam(new_obj, "slip")),
					       (const char *)0);
	if (new_obj) new_obj->known = 1;
	return(1);
}

/*write.c*/
