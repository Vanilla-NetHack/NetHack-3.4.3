/*	SCCS Id: @(#)wield.c	3.0	87/05/10
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

/* Note: setuwep() with a null obj, and uwepgone(), are NOT the same!  Sometimes
 * unwielding a weapon can kill you, and lifesaving will then put it back into
 * your hand.  If lifesaving is permitted to do this, use
 * setwuep((struct obj *)0); otherwise use uwepgone().
 * (The second function should probably be #ifdef NAMED_ITEMS since only then
 * may unwielding a weapon kill you, but...)
 */
void
setuwep(obj)
register struct obj *obj;
{
	setworn(obj, W_WEP);
	if (!obj) unweapon = TRUE;	/* for "bare hands" message */
}

void
uwepgone()
{
	if (uwep) {
		setnotworn(uwep);
		unweapon = TRUE;
	}
}

static const char NEARDATA wield_objs[] = { '#', '-', WEAPON_SYM, TOOL_SYM, 0 };

int
dowield()
{
	register struct obj *wep;
	register int res = 0;

	multi = 0;
#ifdef POLYSELF
	if (cantwield(uasmon)) {
		pline("Don't be ridiculous!");
		return(0);
	}
#endif
	if(!(wep = getobj(wield_objs, "wield"))) /* nothing */;
	else if(uwep == wep)
		You("are already wielding that!");
	else if(welded(uwep))
		weldmsg(uwep, TRUE);
	else if(wep == &zeroobj) {
	    if(uwep == 0)
		You("are already empty %s.", body_part(HANDED));
	    else  {
	  	You("are empty %s.", body_part(HANDED));
	  	setuwep((struct obj *) 0);
	  	res++;
	    }
	} else if(!uarmg &&
#ifdef POLYSELF
		!resists_ston(uasmon) &&
#endif
		(wep->otyp == CORPSE && wep->corpsenm == PM_COCKATRICE)) {
	    /* Prevent wielding cockatrice when not wearing gloves --KAA */
	    You("wield the cockatrice corpse in your bare %s.",
			makeplural(body_part(HAND)));
	    You("turn to stone...");
	    killer_format = KILLED_BY;
	    killer="touching a cockatrice corpse";
	    done(STONING);
	} else if(uarms && bimanual(wep))
	    You("cannot wield a two-handed %s and hold a shield.",
		 is_sword(wep) ? "sword" : "weapon");
	else if(wep->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL))
		You("cannot wield that!");
	else {
		res++;
		if(wep->cursed && (wep->olet == WEAPON_SYM ||
			wep->otyp == HEAVY_IRON_BALL || wep->otyp == PICK_AXE ||
			wep->otyp == UNICORN_HORN ||
			wep->otyp == TIN_OPENER)) {
		    pline("The %s %s to your %s!",
			aobjnam(wep, "weld"),
			(wep->quan == 1) ? "itself" : "themselves", /* a3 */
			body_part(HAND));
		    wep->bknown = 1;
		} else {
			/* The message must be printed before setuwep (since
			 * you might die and be revived from changing weapons),
			 * and the message must be before the death message and
			 * Lifesaved rewielding.  Yet we want the message to say
			 * "weapon in hand", thus this kludge.
			 */
			long dummy = wep->owornmask;
			wep->owornmask |= W_WEP;
			prinv(wep);
			wep->owornmask = dummy;
		}
		setuwep(wep);
	}
	/* Note: Explicitly wielding a pick-axe will not give a "bashing"
	 * message.  Wielding one via 'a'pplying it will.
	 */
	if(res && uwep)
		unweapon = ((uwep->otyp >= BOW || uwep->otyp <= BOOMERANG) &&
			uwep->otyp != PICK_AXE && uwep->otyp != UNICORN_HORN);
	return(res);
}

void
corrode_weapon(){
	if(!uwep || uwep->olet != WEAPON_SYM) return;	/* %% */
	if(uwep->rustfree || objects[uwep->otyp].oc_material != METAL)
		Your("%s not affected.", aobjnam(uwep, "are"));
	else if (uwep->spe > -6) {
		Your("%s!", aobjnam(uwep, "corrode"));
		uwep->spe--;
	} else	Your("%s quite rusted now.", aobjnam(uwep, Blind ? "feel" : "look"));
}

int
chwepon(otmp, amount)
register struct obj *otmp;
register int amount;
{
	register const char *color = Hallucination ? hcolor() :
				     (amount < 0) ? black : blue;
	register const char *xtime;

	if(!uwep || (uwep->olet != WEAPON_SYM && uwep->otyp != PICK_AXE
			&& uwep->otyp != UNICORN_HORN)) {
		char buf[36];

		Sprintf(buf, "Your %s %s.", makeplural(body_part(HAND)),
			(amount > 0) ? "twitch" : "itch");
		strange_feeling(otmp, buf);
		return(0);
	}

#ifdef WORM
	if(uwep->otyp == WORM_TOOTH && amount > 0) {
		uwep->otyp = CRYSKNIFE;
		Your("weapon seems sharper now.");
		uwep->cursed = 0;
		return(1);
	}

	if(uwep->otyp == CRYSKNIFE && amount < 0) {
		uwep->otyp = WORM_TOOTH;
		Your("weapon seems duller now.");
		return(1);
	}
#endif
#ifdef NAMED_ITEMS
	if(is_artifact(uwep) && restr_name(uwep, ONAME(uwep)) &&
		amount<0) {
	    if (!Blind)
		Your("%s faintly %s.", aobjnam(uwep, "glow"), color);
	    return(1);
	}
#endif
	/* there is a (soft) upper and lower limit to uwep->spe */
	if(((uwep->spe > 5 && amount > 0) || (uwep->spe < -5 && amount < 0))
								&& rn2(3)) {
	    if (!Blind)
	    Your("%s violently %s for a while and then evaporate%s.",
		aobjnam(uwep,"glow"), color, uwep->quan == 1 ? "s" : "");
	    else
		Your("%s.", aobjnam(uwep, "evaporate"));

	    while(uwep)		/* let all of them disappear */
				/* note: uwep->quan = 1 is nogood if unpaid */
		useup(uwep);
	    return(1);
	}
	if (!Blind) {
	    xtime = (amount*amount == 1) ? "moment" : "while";
	    Your("%s %s for a %s.",
		  aobjnam(uwep, "glow"), color, xtime);
	}
	uwep->spe += amount;
	if(amount > 0) uwep->cursed = 0;
	return(1);
}

int
welded(obj)
register struct obj *obj;
{
	if (obj && obj == uwep && obj->cursed &&
		  (obj->olet == WEAPON_SYM || obj->otyp == HEAVY_IRON_BALL ||
		   obj->otyp == TIN_OPENER || obj->otyp == PICK_AXE ||
		   obj->otyp == UNICORN_HORN))
	{
		obj->bknown = 1;
		return 1;
	}
	return 0;
}

/* The reason for "specific" is historical; some parts of the code used
 * the object name and others just used "weapon"/"sword".  This function
 * replaced all of those.  Which one we use is really arbitrary.
 */
void
weldmsg(obj, specific)
register struct obj *obj;
boolean specific;
{
	char buf[BUFSZ];

	if (specific) {
		long savewornmask = obj->owornmask;
		obj->owornmask &= ~W_WEP;
		Strcpy(buf, Doname2(obj));
		obj->owornmask = savewornmask;
	} else
		Sprintf(buf, "Your %s%s",
			is_sword(obj) ? "sword" : "weapon",
			plur((long)obj->quan));
	Strcat(buf, (obj->quan==1) ? " is" : " are");
#ifdef POLYSELF
	Sprintf(eos(buf), " welded to your %s!",
		bimanual(obj) ? makeplural(body_part(HAND)) : body_part(HAND));
#else
	Sprintf(eos(buf), " welded to your hand%s!",
		bimanual(obj) ? "s" : "");
#endif
	pline(buf);
}
