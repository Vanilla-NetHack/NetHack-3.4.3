/*	SCCS Id: @(#)engrave.c	3.1	92/02/25	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include <ctype.h>

STATIC_VAR NEARDATA struct engr *head_engr;

STATIC_DCL void FDECL(del_engr, (struct engr *));

#ifdef OVLB
/* random engravings */
const char *random_mesg[] = {
	"Elbereth", "ad ae?ar um",
	"?la? ?as he??",
	/* take-offs and other famous engravings */
	"Owlbreath", "?ala??iel",
	"?ilroy wa? h?re",
	"A.S. ->", "<- A.S.", /* Journey to the Center of the Earth */
	"Y?u won t get i? up ?he ste?s", /* Adventure */
	"Lasc?ate o?ni sp?ranz? o vo? c?'en?rate", /* Inferno */
	"Well Come", /* Prisoner */
	"W? ap?l???ze for t?e inc?nve??e?ce", /* So Long... */
	"S?e you n?xt Wed?esd?y", /* Thriller */
	"Fo? a ?ood time c?ll 8?7-53?9",
};

const char *
random_engraving()
{
	char *rumor, *s;

/* a random engraving may come from the "rumors" file, or from the
   list above */
	rumor = getrumor(0);
	if (rn2(4) && *rumor) {
		for (s = rumor; *s; s++)
			if (!rn2(7) && *s != ' ') *s = '?';
		if (s[-1] == '.') s[-1] = 0;
		return (const char *)rumor;
	}
	else
		return random_mesg[rn2(SIZE(random_mesg))];
}
#endif /* OVLB */
#ifdef OVL0

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

#ifdef ELBERETH
int
sengr_at(s,x,y)
	register const char *s;
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

#endif /* OVL0 */
#ifdef OVL2

void
u_wipe_engr(cnt)
register int cnt;
{
	if(!u.uswallow && !Levitation)
		wipe_engr_at(u.ux, u.uy, cnt);
}

#endif /* OVL2 */
#ifdef OVL1

void
wipe_engr_at(x,y,cnt) register xchar x,y,cnt; {
register struct engr *ep = engr_at(x,y);
register int lth,pos;
char ch;
	if(ep){
	    if(ep->engr_type != BURN) {
		if(ep->engr_type != DUST && ep->engr_type != BLOOD) {
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

#endif /* OVL1 */
#ifdef OVL2

void
read_engr_at(x,y)
register int x,y;
{
	register struct engr *ep = engr_at(x,y);
	register int	sensed = 0;

	if(ep && ep->engr_txt[0]) {
	    switch(ep->engr_type) {
	    case DUST:
		if(!Blind) {
			sensed = 1;
			pline("Something is written here in the dust.");
		}
		break;
	    case ENGRAVE:
		if(!Blind || !Levitation) {
			sensed = 1;
			pline("Something is engraved here on the floor.");
		}
		break;
	    case BURN:
		if(!Blind || !Levitation) {
			sensed = 1;
			pline("Some text has been burned into the floor here.");
		}
		break;
	    case MARK:
		if(!Blind) {
			sensed = 1;
			pline("There's some graffiti on the floor here.");
		}
		break;
	    case BLOOD:
		/* "It's a message!  Scrawled in blood!"
		 * "What's it say?"
		 * "It says... `See you next Wednesday.'" -- Thriller
		 */
		if(!Blind) {
			sensed = 1;
			You("see a message scrawled in blood here.");
		}
		break;
	    default:
		impossible("Something is written in a very strange way.");
		sensed = 1;
	    }
	    if (sensed) {
		You("%s: \"%s\".",
		      (Blind) ? "feel the words" : "read",  ep->engr_txt);
		if(flags.run > 1) nomul(0);
	    }
	}
}

#endif /* OVL2 */
#ifdef OVLB

void
make_engr_at(x,y,s,e_time,e_type)
register int x,y;
register const char *s;
register long e_time;
register xchar e_type;
{
	register struct engr *ep;

	if(ep = engr_at(x,y))
	    del_engr(ep);
	ep = newengr(strlen(s) + 1);
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = x;
	ep->engr_y = y;
	ep->engr_txt = (char *)(ep + 1);
	Strcpy(ep->engr_txt, s);
	if(strcmp(s, "Elbereth")) exercise(A_WIS, TRUE);
	ep->engr_time = e_time;
	ep->engr_type = e_type > 0 ? e_type : rnd(N_ENGRAVE);
	ep->engr_lth = strlen(s) + 1;
}

/*
 *	freehand - returns true if player has a free hand
 */
int
freehand()
{
	return(!uwep || !welded(uwep) ||
	   (!bimanual(uwep) && (!uarms || !uarms->cursed)));
/*	if ((uwep && bimanual(uwep)) ||
	    (uwep && uarms))
		return(0);
	else
		return(1);*/
}

static NEARDATA const char styluses[] =
	{ ALL_CLASSES, ALLOW_NONE, TOOL_CLASS, WEAPON_CLASS, WAND_CLASS,
	  GEM_CLASS, RING_CLASS, 0 };

/* Mohs' Hardness Scale:
 *  1 - Talc		 6 - Orthoclase
 *  2 - Gypsum		 7 - Quartz
 *  3 - Calcite		 8 - Topaz
 *  4 - Fluorite	 9 - Corundum
 *  5 - Apatite		10 - Diamond
 *
 * Since granite is a igneous rock hardness ~ 7, anything >= 8 should
 * probably be able to scratch the rock.
 * Devaluation of less hard gems is not easily possible because obj struct
 * does not contain individual oc_cost currently. 7/91
 *
 * dilithium  - ??			* jade	    -  5-6	(nephrite)
 * diamond    - 10			* turquoise -  5-6
 * ruby	      -  9	(corundum)	* opal	    -  5-6
 * sapphire   -  9	(corundum)	* iron	    -  4-5
 * topaz      -  8			* fluorite  -  4
 * emerald    -  7.5-8	(beryl)		* brass     -  3-4
 * aquamarine -  7.5-8	(beryl)		* gold	    -  2.5-3
 * garnet     -  7.25	(var. 6.5-8)	* silver    -  2.5-3
 * agate      -  7	(quartz)	* copper    -  2.5-3
 * amethyst   -  7	(quartz)	* amber     -  2-2.5
 * jasper     -  7	(quartz)	*	
 * onyx	      -  7 	(quartz)	* steel     -  5-8.5	(usu. weapon)
 * moonstone  -  6	(orthoclase)	*
 */

static NEARDATA const short hard_gems[] =
	{ DIAMOND, RUBY, SAPPHIRE, TOPAZ, EMERALD, AQUAMARINE, GARNET, 0 };

static NEARDATA const char *hard_ring_names[] =
	{"diamond", "ruby", "sapphire", "emerald", "topaz", ""};

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
doengrave()
{
	boolean dengr = FALSE;	/* TRUE if we wipe out the current engraving */
	boolean doblind = FALSE;/* TRUE if engraving blinds the player */
	boolean doknown = FALSE;/* TRUE if we identify the stylus */
	boolean eow = FALSE;	/* TRUE if we are overwriting oep */
	boolean jello = FALSE;	/* TRUE if we are engraving in slime */
	boolean ptext = TRUE;	/* TRUE if we must prompt for engrave text */
	boolean teleengr =FALSE;/* TRUE if we move the old engraving */
	boolean zapwand = FALSE;/* TRUE if we remove a wand charge */
	xchar type = DUST;	/* Type of engraving made */
	char buf[BUFSZ];	/* Buffer for final/poly engraving text */
	char ebuf[BUFSZ];	/* Buffer for initial engraving text */
	char qbuf[QBUFSZ];	/* Buffer for query text */
	const char *everb;	/* Present tense of engraving type */
	const char *eloc;	/* Where the engraving is (ie dust/floor/...) */
	const char *post_engr_text; /* Text displayed after engraving prompt */
	register char *sp;	/* Place holder for space count of engr text */
	register int len;	/* # of nonspace chars of new engraving text */
	register int maxelen;	/* Max allowable length of new engraving text */
	register int spct;	/* # of spaces in new engraving text */
	register struct engr *oep = engr_at(u.ux,u.uy);
				/* The current engraving */
	register struct obj *otmp; /* Object selected with which to engrave */


	multi = 0;		/* moves consumed */
	nomovemsg = (char *)0;	/* occupation end message */

	buf[0] = (char)0;
	ebuf[0] = (char)0;
	post_engr_text = (char *)0;
	maxelen = BUFSZ - 1;

	/* Can the adventurer engrave at all? */

	if(u.uswallow) {
		if (is_animal(u.ustuck->data)) {
			pline("What would you write?  \"Jonah was here\"?");
			return(0);
		} else if (is_whirly(u.ustuck->data)) {
			You("can't reach the ground.");
			return(0);
		} else 
			jello = TRUE;
    	} else if (is_lava(u.ux, u.uy)) {
		You("can't write on the lava!");
		return(0);
	} else if (is_pool(u.ux,u.uy) || IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		You("can't write on the water!");
		return(0);
	}
	if(Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)/* in bubble */) {
		You("can't write in thin air!");
		return(0);
	}
#ifdef POLYSELF
	if (cantwield(uasmon)) {
		You("can't even hold anything!");
		return(0);
	}
#endif
	if (check_capacity(NULL)) return (0);

	/* One may write with finger, or weapon, or wand, or..., or...
	 * Edited by GAN 10/20/86 so as not to change weapon wielded.
	 */

	otmp = getobj(styluses, "write with");
	if(!otmp) return(0);		/* otmp == zeroobj if fingers */

	/* There's no reason you should be able to write with a wand
	 * while both your hands are tied up.
	 */
	if (!freehand() && otmp != uwep && !otmp->owornmask) {
		You("have no free %s to write with!", body_part(HAND));
		return(0);
	}

	if (jello) {
		You("tickle %s with your %s.", mon_nam(u.ustuck), 
		    (otmp == &zeroobj) ? makeplural(body_part(FINGER)) :
			xname(otmp));
		Your("message dissolves...");
		return(0);
	}
	if(Levitation && otmp->oclass != WAND_CLASS){		/* riv05!a3 */
		You("can't reach the floor!");
		return(0);
	}

	/* SPFX for items */

	switch (otmp->oclass) {
	    default:
	    case AMULET_CLASS:
	    case CHAIN_CLASS:
	    case POTION_CLASS:
	    case GOLD_CLASS:
		break;

	    case RING_CLASS:
		/* "diamond" rings and others should work */
		{
		    register int i, j;

		    for (i=0, j=strlen(hard_ring_names[i]); j; i++)
			if ( !strncmp(hard_ring_names[i],
			     OBJ_DESCR(objects[otmp->otyp]),
			     j=strlen(hard_ring_names[i])) ) {
			    type = ENGRAVE;
			    break;
			}
		}
		break;

	    case GEM_CLASS:
		/* diamonds & other gems should work */
		{
		    register int i;

		    for (i=0; hard_gems[i]; i++)
			if (otmp->otyp == hard_gems[i]) {
			    type = ENGRAVE;
			    break;
			}
		}
		break;

	    /* Objects too large to engrave with */
	    case BALL_CLASS:
	    case ROCK_CLASS:
	    case ARMOR_CLASS:
		You("can't engrave with such a large object!");
		ptext = FALSE;
		break;

	    /* Objects too silly to engrave with */
	    case FOOD_CLASS:
	    case SCROLL_CLASS:
	    case SPBOOK_CLASS:
		Your("%s would get too dirty.", xname(otmp));
		ptext = FALSE;
		break;

	    case RANDOM_CLASS:	/* This should mean fingers */
		break;

	    /* The charge is removed from the wand before prompting for
	     * the engraving text, because all kinds of setup decisions
	     * and pre-engraving messages are based upon knowing what type
	     * of engraving the wand is going to do.  Also, the player
	     * will have potentially seen "You wrest .." message, and
	     * therefore will know they are using a charge.
	     */
	    case WAND_CLASS:
		if (zappable(otmp)) {
		    zapwand = TRUE;
		    if (Levitation) ptext = FALSE;

		    switch (otmp->otyp) {
		    /* DUST wands */
		    default:
			break;

			/* NODIR wands */
		    case WAN_LIGHT:
		    case WAN_SECRET_DOOR_DETECTION:
		    case WAN_CREATE_MONSTER:
		    case WAN_WISHING:
	  		zapnodir(otmp);
			break;

			/* IMMEDIATE wands */
	    		/* If wand is "IMMEDIATE", remember to effect the
			 * previous engraving even if turning to dust.,
			 */
		    case WAN_STRIKING:
			post_engr_text =
			"The wand unsuccessfully fights your attempt to write!";
			break;
		    case WAN_SLOW_MONSTER:
			if (!Blind)
			   post_engr_text = "The bugs on the ground slow down!";
			break;
		    case WAN_SPEED_MONSTER:
			if (!Blind)
			   post_engr_text = "The bugs on the ground speed up!";
			break;
		    case WAN_POLYMORPH:
			if(oep)  {
			    if (!Blind) {
				type = (xchar)0;	/* random */
				Strcpy(buf,random_engraving());
			    }
			    dengr = TRUE;
			}
			break;
		    case WAN_NOTHING:
		    case WAN_UNDEAD_TURNING:
		    case WAN_OPENING:
		    case WAN_LOCKING:
		    case WAN_PROBING:
			break;

			/* RAY wands */
		    case WAN_MAGIC_MISSILE:
			ptext = TRUE;
			if (!Blind)
			    post_engr_text =
				"The ground is riddled by bullet holes!";
			break;

		    /* can't tell sleep from death - Eric Backus */
		    case WAN_SLEEP:
		    case WAN_DEATH:
			if (!Blind)
			    post_engr_text =
				"The bugs on the ground stop moving!";
			break;

		    case WAN_COLD:
			if (!Blind)
			    post_engr_text =
				"A few ice cubes drop from the wand.";
			if(!oep || (oep->engr_type != BURN))
			    break;
		    case WAN_CANCELLATION:
		    case WAN_MAKE_INVISIBLE:
			if(oep) {
			    if (!Blind)
				pline("The engraving on the floor vanishes!");
			    dengr = TRUE;
			}
			break;
		    case WAN_TELEPORTATION:
			if (oep) {
			    if (!Blind)
				pline("The engraving on the floor vanishes!");
			    teleengr = TRUE;
			}
			break;

		    /* type = ENGRAVE wands */
		    case WAN_DIGGING:
			ptext = TRUE;
			type  = ENGRAVE;
			if(!objects[otmp->otyp].oc_name_known) {
	    		    if (flags.verbose)
				pline("This %s is a wand of digging!",
				xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind)
			    post_engr_text = "Gravel flies up from the floor.";
			else
			    post_engr_text = "You hear drilling!";
			break;

		    /* type = BURN wands */
		    case WAN_FIRE:
			ptext = TRUE;
			type  = BURN;
			if(!objects[otmp->otyp].oc_name_known) {
	    		if (flags.verbose)
			    pline("This %s is a wand of fire!", xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind)
			    post_engr_text = "Flames fly from the wand.";
			else
			    post_engr_text = "You feel the wand heat up.";
			break;
		    case WAN_LIGHTNING:
			ptext = TRUE;
			type  = BURN;
			if(!objects[otmp->otyp].oc_name_known) {
	    		    if (flags.verbose)
				pline("This %s is a wand of lightning!",
					xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind) {
			    post_engr_text = "Lightning arcs from the wand.";
			    doblind = TRUE;
			} else
			    post_engr_text = "You hear crackling!";
			break;

		    /* type = MARK wands */
		    /* type = BLOOD wands */
		    }
		} else /* end if zappable */
		    if (Levitation) {
			You("can't reach the floor!");
			return(0);
		    }
		break;

	    case WEAPON_CLASS:
		if(is_blade(otmp))
		    if ((int)otmp->spe > -3)
			type = ENGRAVE;
		    else
			Your("%s too dull for engraving.", aobjnam(otmp,"are"));
		break;

	    case TOOL_CLASS:
		if(otmp == ublindf) {
		    pline(
		"That is a bit difficult to engrave with, don't you think?");
		    return(0);
		}
		switch (otmp->otyp)  {
		    case MAGIC_MARKER:
			if (otmp->spe <= 0)
			    Your("marker has dried out.");
			else
			    type = MARK;
			break;
		    case TOWEL:
 			/* Can't really engrave with a towel */
			ptext = FALSE;
			if (oep)
			    if ((oep->engr_type == DUST ) ||
				(oep->engr_type == BLOOD) ||
				(oep->engr_type == MARK )) {
				if (!Blind)
				    You("wipe out the message here.");
				else
				    Your("%s gets dusty.", xname(otmp));
				dengr = TRUE;
			    } else
				Your("%s can't wipe out this engraving.",
				     xname(otmp));
			else
			    Your("%s gets dusty.", xname(otmp));
			break;
		    default:
			break;
		}
		break;

	    case VENOM_CLASS:
#ifdef WIZARD
		if (wizard) {
		    pline("Writing a poison pen letter??");
		    break;
		}
#endif
	    case ILLOBJ_CLASS:
		impossible("You're engraving with an illegal object!");
		break;
	}

	/* End of implement setup */

	/* Identify stylus */
	if (doknown) {
	    makeknown(otmp->otyp);
	    more_experienced(0,10);
	}

	if (teleengr) {
	    register int tx,ty;

	    do  {
 		tx = rn1(COLNO-3,2);
		ty = rn2(ROWNO);
	    } while(!goodpos(tx,ty, (struct monst *)0, (struct permonst *)0));

	    oep->engr_x = tx;
	    oep->engr_y = ty;

	    oep = (struct engr *)0;
	}

	if (dengr) {
	    del_engr (oep);
	    oep = (struct engr *)0;
	}

	/* Something has changed the engraving here */
	if (*buf) {
	    make_engr_at(u.ux, u.uy, buf, moves, type);
	    pline("The engraving now reads: \"%s\".", buf);
	    ptext = FALSE;
	}

	if (zapwand && (otmp->spe < 0)) {
	    pline("%s %sturns to dust.",
		  The(xname(otmp)), Blind ? "" : "glows violently, then ");
You("are not going to get anywhere trying to write in the dust with your dust.");
	    useup(otmp);
	    ptext = FALSE;
	}

	if (!ptext) {		/* Early exit for some implements. */
	    if (Levitation && (otmp->oclass == WAND_CLASS))
		You("can't reach the floor!");
	    return(1);
	}

	/* Special effects should have deleted the current engraving (if
	 * possible) by now.
	 */

	if (oep) {
	    register char c = 'n';

	    /* Give player the choice to add to engraving. */

	    if ( (type == oep->engr_type) && (!Blind ||
		 (oep->engr_type == BURN) || (oep->engr_type == ENGRAVE)) ) {
	    	c = yn_function("Do you want to add to the current engraving?",
				ynqchars, 'y');
		if (c == 'q') {
		    pline("Never mind.");
		    return(0);
		}
	    }

	    if (c == 'n' || Blind)

		if( (oep->engr_type == DUST) || (oep->engr_type == BLOOD) ||
		    (oep->engr_type == MARK) ) {
		    if (!Blind) {
			You("wipe out the message that was %s here.",
			    ((oep->engr_type == DUST)  ? "written in the dust" :
			    ((oep->engr_type == BLOOD) ? "scrawled in blood"   :
							 "written")));
			del_engr(oep);
			oep = (struct engr *)0;
		    } else
		   /* Don't delete engr until after we *know* we're engraving */
			eow = TRUE;
		} else
		    if ( (type == DUST) || (type == MARK) || (type == BLOOD) ) {
			You(
		       "cannot wipe out the message that is %s the floor here.",
		            (oep->engr_type == BURN) ? "burned into" :
			    "engraved in");
			return(1);
		    } else
			if ( (type != oep->engr_type) || (c == 'n') ) {
			    if (!Blind || !Levitation)
				You("will overwrite the current message.");
			    eow = TRUE;
			}
	}

	switch(type){
	    default:
		everb = (oep && !eow ? "add to the weird writing on" :
				       "write strangely on");
		eloc  = "the floor";
		break;
	    case DUST:
		everb = (oep && !eow ? "add to the writing in" :
				       "write in");
		eloc = "the dust";
		break;
	    case ENGRAVE:
		everb = (oep && !eow ? "add to the engraving in" :
				       "engrave in");
		eloc = "the floor";
		break;
	    case BURN:
		everb = (oep && !eow ? "add to the text burned into" :
				       "burn into");
		eloc = "the floor";
		break;
	    case MARK:
		everb = (oep && !eow ? "add to the graffiti on" :
				       "scribble on");
		eloc = "the floor";
		break;
	    case BLOOD:
		everb = (oep && !eow ? "add to the scrawl on" :
				       "scrawl on");
		eloc = "the floor";
		break;
	}

	/* Tell adventurer what is going on */
	if (otmp != &zeroobj)
	    You("%s %s with %s.", everb, eloc, doname(otmp));
	else
	    You("%s %s with your %s.", everb, eloc,
		makeplural(body_part(FINGER)));

	/* Prompt for engraving! */
	Sprintf(qbuf,"What do you want to %s %s here?", everb, eloc);
	getlin(qbuf, ebuf);

	/* Mix up engraving if surface or state of mind is unsound.  */
	/* Original kludge by stewr 870708.  modified by njm 910722. */
	for (sp = ebuf; *sp; sp++)
	    if ( ((type == DUST || type == BLOOD) && !rn2(25)) ||
		 (Blind   && !rn2(9)) || (Confusion     && !rn2(12)) ||
		 (Stunned && !rn2(4)) || (Hallucination && !rn2(1)) )
		 *sp = '!' + rn2(93); /* ASCII-code only */

	/* Count the actual # of chars engraved not including spaces */
	len = strlen(ebuf);

	for (sp = ebuf, spct = 0; *sp; sp++) if (isspace(*sp)) spct++;

	if ( (len == spct) || index(ebuf, '\033') ) {
	    if (zapwand) {
		if (!Blind)
		    pline("%s glows, then fades.", The(xname(otmp)));
	    	return(1);
	    } else {
		pline("Never mind.");
		return(0);
	    }
	}

	len -= spct;

	/* Previous engraving is overwritten */
	if (eow) {
	    del_engr(oep);
	    oep = (struct engr *)0;
	}

	/* Figure out how long it took to engrave, and if player has
	 * engraved too much.
	 */
	switch(type){
	    default:
		multi = -(len/10);
		if (multi) nomovemsg = "You finish your weird engraving.";
		break;
	    case DUST:
		multi = -(len/10);
		if (multi) nomovemsg = "You finish writing in the dust.";
		break;
	    case ENGRAVE:
		multi = -(len/10);
		if ((otmp->oclass == WEAPON_CLASS) &&
		    ((otmp->otyp != ATHAME) || otmp->cursed)) {
		    multi = -len;
		    maxelen = ((otmp->spe + 3) * 2) + 1;
			/* -2 = 3, -1 = 5, 0 = 7, +1 = 9, +2 = 11
			 * Note: this does not allow a +0 anything (except
			 *	 an athame) to engrave "Elbereth" all at once.
			 *	 However, you could now engrave "Elb", then
			 *	 "ere", then "th".
			 */
		    Your("%s dull.", aobjnam(otmp, "get"));
		    if (len > maxelen) {
		    	multi = -maxelen;
			otmp->spe = -3;
		    } else
			if (len > 1) otmp->spe -= len >> 1;
			else otmp->spe -= 1; /* Prevent infinite engraving */
		} else
		    if ( (otmp->oclass == RING_CLASS) ||
			 (otmp->oclass == GEM_CLASS) )
			multi = -len;
		if (multi) nomovemsg = "You finish engraving.";
		break;
	    case BURN:
		multi = -(len/10);
		if (multi)
		    nomovemsg =
			"You finish burning your message into the floor.";
		break;
	    case MARK:
		multi = -(len/10);
		if ((otmp->oclass == TOOL_CLASS) &&
		    (otmp->otyp == MAGIC_MARKER)) {
		    maxelen = (otmp->spe) * 2; /* one charge / 2 letters */
		    if (len > maxelen) {
			Your("marker dries out.");
			otmp->spe = 0;
			multi = -(maxelen/10);
		    } else
			if (len > 1) otmp->spe -= len >> 1;
			else otmp->spe -= 1; /* Prevent infinite grafitti */
		}
		if (multi) nomovemsg = "You finish defacing the dungeon.";
		break;
	    case BLOOD:
		multi = -(len/10);
		if (multi) nomovemsg = "You finish scrawling.";
		break;
	}

	/* Chop engraving down to size if necessary */
	if (len > maxelen) {
	    for (sp = ebuf; (maxelen && *sp); sp++)
		if (!isspace(*sp)) maxelen--;
	    if (!maxelen && *sp) {
		*sp = (char)0;
		if (multi) nomovemsg = "You cannot write any more.";
		You("only are able to write \"%s\"", ebuf);
	    }
	}

	/* Add to existing engraving */
	if (oep) Strcpy(buf, oep->engr_txt);	

	(void) strncat(buf, ebuf, (BUFSZ - (int)strlen(buf) - 1));

	make_engr_at(u.ux, u.uy, buf, (moves - multi), type);

	if (post_engr_text) pline(post_engr_text);

	if (doblind) {
	    You("are blinded by the flash!");
	    make_blinded((long)rnd(50),FALSE);
	}

	return(1);
}

void
save_engravings(fd, mode)
int fd, mode;
{
	register struct engr *ep = head_engr;
	register struct engr *ep2;
#ifdef GCC_WARN
	static long nulls[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
	while(ep) {
	    ep2 = ep->nxt_engr;
	    if(ep->engr_lth && ep->engr_txt[0]){
		bwrite(fd, (genericptr_t)&(ep->engr_lth), sizeof(ep->engr_lth));
		bwrite(fd, (genericptr_t)ep, sizeof(struct engr) + ep->engr_lth);
	    }
	    if (mode & FREE_SAVE)
		dealloc_engr(ep);
	    ep = ep2;
	}

#ifdef GCC_WARN
	bwrite(fd, (genericptr_t)nulls, sizeof(unsigned));
#else
	bwrite(fd, (genericptr_t)nul, sizeof(unsigned));
#endif

	if (mode & FREE_SAVE)
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
		ep = newengr(lth);
		mread(fd, (genericptr_t) ep, sizeof(struct engr) + lth);
		ep->nxt_engr = head_engr;
		head_engr = ep;
		ep->engr_txt = (char *) (ep + 1);	/* Andreas Bormann */
		/* mark as finished for bones levels -- no problem for
		 * normal levels as the player must have finished engraving
		 * to be able to move again */
		ep->engr_time = moves;
	}
}

STATIC_OVL void
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
	dealloc_engr(ep);
}

#endif /* OVLB */

/*engrave.c*/
