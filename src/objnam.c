/*	SCCS Id: @(#)objnam.c	3.1	92/12/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

/* "an uncursed partly eaten guardian naga hatchling corpse" */
#define	PREFIX	50
#define SCHAR_LIM 127

STATIC_DCL char *FDECL(strprepend,(char *,const char *));

struct Jitem {
	int item;
	const char *name;
};

#ifndef OVLB

STATIC_DCL struct Jitem Japanese_items[];

#else /* OVLB */

STATIC_OVL struct Jitem Japanese_items[] = {
	{ SHORT_SWORD, "wakizashi" },
	{ BROADSWORD, "ninja-to" },
	{ GLAIVE, "naginata" },
	{ LOCK_PICK, "osaku" },
	{0, "" }
};

#endif /* OVLB */

STATIC_DCL const char *FDECL(Japanese_item_name,(int i));

#ifdef OVL1

STATIC_OVL char *
strprepend(s,pref)
register char *s;
register const char *pref; {
register int i = strlen(pref);
	if(i > PREFIX) {
		pline("WARNING: prefix too short.");
		return(s);
	}
	s -= i;
	(void) strncpy(s, pref, i);	/* do not copy trailing 0 */
	return(s);
}

#endif /* OVL1 */
#ifdef OVLB

char *
typename(otyp)
register int otyp;
{
#ifdef LINT	/* static char buf[BUFSZ]; */
char buf[BUFSZ];
#else
static char NEARDATA buf[BUFSZ];
#endif
register struct objclass *ocl = &objects[otyp];
register const char *actualn = OBJ_NAME(*ocl);
register const char *dn = OBJ_DESCR(*ocl);
register const char *un = ocl->oc_uname;
register int nn = ocl->oc_name_known;

	if (pl_character[0] == 'S' && Japanese_item_name(otyp))
		actualn = Japanese_item_name(otyp);
	switch(ocl->oc_class) {
	case GOLD_CLASS:
		Strcpy(buf, "coin");
		break;
	case POTION_CLASS:
		Strcpy(buf, "potion");
		break;
	case SCROLL_CLASS:
		Strcpy(buf, "scroll");
		break;
	case WAND_CLASS:
		Strcpy(buf, "wand");
		break;
	case SPBOOK_CLASS:
		Strcpy(buf, "spellbook");
		break;
	case RING_CLASS:
		Strcpy(buf, "ring");
		break;
	case AMULET_CLASS:
		if(nn)
			Strcpy(buf,actualn);
		else
			Strcpy(buf,"amulet");
		if(un)
			Sprintf(eos(buf)," called %s",un);
		if(dn)
			Sprintf(eos(buf)," (%s)",dn);
		return(buf);
	default:
		if(nn) {
			Strcpy(buf, actualn);
			if(otyp >= TURQUOISE && otyp <= JADE)
				Strcat(buf, " stone");
			if(un)
				Sprintf(eos(buf), " called %s", un);
			if(dn)
				Sprintf(eos(buf), " (%s)", dn);
		} else {
			Strcpy(buf, dn ? dn : actualn);
			if(ocl->oc_class == GEM_CLASS) {
				if (otyp == LOADSTONE || otyp == LUCKSTONE)
					Strcat(buf, " stone");
				else
					Strcat(buf, " gem");
			}
			if(un)
				Sprintf(eos(buf), " called %s", un);
		}
		return(buf);
	}
	/* here for ring/scroll/potion/wand */
	if(nn)
		Sprintf(eos(buf), " of %s", actualn);
	if(un)
		Sprintf(eos(buf), " called %s", un);
	if(dn)
		Sprintf(eos(buf), " (%s)", dn);
	return(buf);
}

boolean
obj_is_pname(obj)
register struct obj *obj;
{
    return (obj->dknown && obj->known && obj->onamelth && obj->oartifact &&
	    !objects[obj->otyp].oc_unique);
}

/* Give the name of an object seen at a distance.  Unlike xname/doname,
 * we don't want to set dknown if it's not set already.  The kludge used is
 * to temporarily set Blind so that xname() skips the dknown setting.  This
 * assumes that we don't want to do this too often; if this function becomes
 * frequently used, it'd probably be better to pass a parameter to xname()
 * or doname() instead.
 */
char *
distant_name(obj, func)
register struct obj *obj;
char *FDECL((*func), (OBJ_P));
{
	char *str;

	long save_Blinded = Blinded;
	Blinded = 1;
	str = (*func)(obj);
	Blinded = save_Blinded;
	return str;
}

#endif /* OVLB */
#ifdef OVL1

char *
xname(obj)
register struct obj *obj;
{
#ifdef LINT	/* lint may handle static decl poorly -- static char bufr[]; */
char bufr[BUFSZ];
#else
static char bufr[BUFSZ];
#endif
register char *buf = &(bufr[PREFIX]);	/* leave room for "17 -3 " */
register int typ = obj->otyp;
register int nn = objects[typ].oc_name_known;
register const char *actualn = OBJ_NAME(objects[typ]);
register const char *dn = OBJ_DESCR(objects[typ]);
register const char *un = objects[typ].oc_uname;

	if (pl_character[0] == 'S' && Japanese_item_name(typ))
		actualn = Japanese_item_name(typ);

	buf[0] = '\0';
	if (!Blind) obj->dknown=1;
	if (obj_is_pname(obj))
	    goto nameit;
	switch (obj->oclass) {
	    case AMULET_CLASS:
		if (!obj->dknown)
			Strcpy(buf, "amulet");
		else if (typ == FAKE_AMULET_OF_YENDOR)
			/* each must be identified individually */
			Strcpy(buf, obj->known ? actualn : dn);
		else if (nn) /* should be true for the Amulet of Yendor */
			Strcpy(buf, actualn);
		else if (un)
			Sprintf(buf,"amulet called %s", un);
		else
			Sprintf(buf,"%s amulet", dn);
		break;
	    case WEAPON_CLASS:
		if (typ <= SHURIKEN && obj->opoisoned)
			Strcpy(buf, "poisoned ");
	    case VENOM_CLASS:
	    case TOOL_CLASS:
		if(un) {
			/* un must come first here.  If it does not, they could
			 * tell objects apart by seeing which ones refuse to
			 * accept names.
			 */
			Sprintf(buf, "%s called %s",
				nn ? actualn : dn, un);
		} else if(nn)
			Strcat(buf, actualn);
		else
			Strcat(buf, dn);
		/* If we use an() here we'd have to remember never to use */
		/* it whenever calling doname() or xname(). */
		if (typ == FIGURINE)
		    Sprintf(eos(buf), " of a%s %s",
			index(vowels,*(mons[obj->corpsenm].mname)) ? "n" : "",
			mons[obj->corpsenm].mname);
		break;
	    case ARMOR_CLASS:
		/* depends on order of the dragon scales objects */
		if (typ >= GRAY_DRAGON_SCALES && typ <= YELLOW_DRAGON_SCALES) {
			Sprintf(buf, "set of %s", OBJ_NAME(objects[typ]));
			break;
		}
		if(is_boots(obj) || is_gloves(obj)) Strcpy(buf,"pair of ");

		if(nn)	Strcat(buf, actualn);
		else if(un) {
			if(is_boots(obj))
				Strcat(buf,"boots");
			else if(is_gloves(obj))
				Strcat(buf,"gloves");
			else if(is_cloak(obj))
				Strcpy(buf,"cloak");
			else if(is_helmet(obj))
				Strcpy(buf,"helmet");
			else if(is_shield(obj))
				Strcpy(buf,"shield");
			else
				Strcpy(buf,"armor");
			Strcat(buf, " called ");
			Strcat(buf, un);
		} else	Strcat(buf, dn);
		break;
	    case FOOD_CLASS:
#ifdef TUTTI_FRUTTI
		if (typ == SLIME_MOLD) {
			register struct fruit *f;

			for(f=ffruit; f; f = f->nextf) {
				if(f->fid == obj->spe) {
					Strcpy(buf, f->fname);
					break;
				}
			}
			if (!f) impossible("Bad fruit #%d?", obj->spe);
			break;
		}
#endif
		Strcpy(buf, actualn);
		if (typ == TIN && obj->known) {
		    if(obj->spe > 0)
			Strcat(buf, " of spinach");
		    else if (obj->corpsenm < 0)
		        Strcpy(buf, "empty tin");
		    else if (is_meaty(&mons[obj->corpsenm]))
			Sprintf(eos(buf), " of %s meat", mons[obj->corpsenm].mname);
		    else
			Sprintf(eos(buf), " of %s", mons[obj->corpsenm].mname);
		}
		break;
	    case GOLD_CLASS:
	    case CHAIN_CLASS:
		Strcpy(buf, actualn);
		break;
	    case ROCK_CLASS:
		if (typ == STATUE)
		    Sprintf(buf, "%s of %s%s", actualn,
			type_is_pname(&mons[obj->corpsenm]) ? "" :
			    (index(vowels,*(mons[obj->corpsenm].mname)) ?
								"an " : "a "),
			mons[obj->corpsenm].mname);
		else Strcpy(buf, actualn);
		break;
	    case BALL_CLASS:
		Sprintf(buf, "%sheavy iron ball",
			(obj->owt > objects[typ].oc_weight) ? "very " : "");
		break;
	    case POTION_CLASS:
		if(nn || un || !obj->dknown) {
			Strcpy(buf, "potion");
			if(!obj->dknown) break;
			if(nn) {
			    Strcat(buf, " of ");
			    if (typ == POT_WATER &&
				objects[POT_WATER].oc_name_known &&
				(obj->bknown || pl_character[0] == 'P') &&
				(obj->blessed || obj->cursed)) {
				Strcat(buf, obj->blessed ? "holy " : "unholy ");
			    }
			    Strcat(buf, actualn);
			} else {
				Strcat(buf, " called ");
				Strcat(buf, un);
			}
		} else {
			Strcpy(buf, dn);
			Strcat(buf, " potion");
		}
		break;
	case SCROLL_CLASS:
		Strcpy(buf, "scroll");
		if(!obj->dknown) break;
		if(nn) {
			Strcat(buf, " of ");
			Strcat(buf, actualn);
		} else if(un) {
			Strcat(buf, " called ");
			Strcat(buf, un);
		} else if (objects[typ].oc_magic) {
			Strcat(buf, " labeled ");
			Strcat(buf, dn);
		} else {
			Strcpy(buf, dn);
			Strcat(buf, " scroll");
		}
		break;
	case WAND_CLASS:
		if(!obj->dknown)
			Strcpy(buf, "wand");
		else if(nn)
			Sprintf(buf, "wand of %s", actualn);
		else if(un)
			Sprintf(buf, "wand called %s", un);
		else
			Sprintf(buf, "%s wand", dn);
		break;
	case SPBOOK_CLASS:
		if (!obj->dknown) {
			Strcpy(buf, "spellbook");
		} else if (nn) {
			if (typ != SPE_BOOK_OF_THE_DEAD)
			    Strcpy(buf, "spellbook of ");
			Strcat(buf, actualn);
		} else if (un) {
			Sprintf(buf, "spellbook called %s", un);
		} else
			Sprintf(buf, "%s spellbook", dn);
		break;
	case RING_CLASS:
		if(!obj->dknown)
			Strcpy(buf, "ring");
		else if(nn)
			Sprintf(buf, "ring of %s", actualn);
		else if(un)
			Sprintf(buf, "ring called %s", un);
		else
			Sprintf(buf, "%s ring", dn);
		break;
	case GEM_CLASS:
		if(!obj->dknown) {
			if (typ == ROCK || typ == LOADSTONE || typ == LUCKSTONE)
				Strcpy(buf, "stone");
			else
				Strcpy(buf, "gem");
			break;
		}
		if(!nn) {
			const char *rock =
			  (typ==LOADSTONE || typ==LUCKSTONE) ? "stone" : "gem";
			if(un)	Sprintf(buf,"%s called %s", rock, un);
			else	Sprintf(buf, "%s %s", dn, rock);
			break;
		}
		Strcpy(buf, actualn);
		if(typ >= TURQUOISE && typ <= JADE)
			Strcat(buf, " stone");
		break;
	default:
		Sprintf(buf,"glorkum %d %d %d", obj->oclass, typ, obj->spe);
	}
	if(obj->quan != 1L) Strcpy(buf, makeplural(buf));

	if(obj->onamelth &&
	   (!obj->oartifact || !objects[obj->otyp].oc_unique)) {
		Strcat(buf, " named ");
	    nameit:
		Strcat(buf, ONAME(obj));
	}
	return(buf);
}

#endif /* OVL1 */
#ifdef OVL0

char *
doname(obj)
register struct obj *obj;
{
	boolean ispoisoned = FALSE;
	char prefix[PREFIX];
	char tmpbuf[PREFIX+1];
	/* when we have to add something at the start of prefix instead of the
	 * end (Strcat is used on the end)
	 */
	register char *bp = xname(obj);
	/* When using xname, we want "poisoned arrow", and when using
	 * doname, we want "poisoned +0 arrow".  This kludge is about the only
	 * way to do it, at least until someone overhauls xname() and doname(),
	 * combining both into one function taking a parameter.
	 */
	if (!strncmp(bp, "poisoned ", 9)) {
		bp += 9;
		ispoisoned = TRUE;
	}

	if(obj->quan != 1L)
		Sprintf(prefix, "%ld ", obj->quan);
	else if(obj_is_pname(obj)) {
		if (!strncmpi(bp, "the ", 4))
		    bp += 4;
		Strcpy(prefix, "the ");
	} else
		Strcpy(prefix, "a ");
	if((obj->bknown || pl_character[0] == 'P') &&
	    (obj->otyp != POT_WATER || !objects[POT_WATER].oc_name_known
		|| (!obj->cursed && !obj->blessed))) {
	    /* allow 'blessed clear potion' if we don't know it's holy water;
	     * always allow "uncursed potion of water"
	     */
	    if(obj->cursed)
		Strcat(prefix, "cursed ");
	    else if(obj->blessed)
		Strcat(prefix, "blessed ");
	    else if (((obj->oclass != ARMOR_CLASS
			&& obj->oclass != WAND_CLASS
			&& obj->oclass != WEAPON_CLASS
			&& ((obj->oclass != TOOL_CLASS &&
			     obj->oclass != RING_CLASS) ||
			     !objects[obj->otyp].oc_charged))
			    || !obj->known)
		/* For items with charges or +/-, knowing the +/- means that
		 * the item has been totally identified, and therefore there
		 * is no doubt as to the object being uncursed if it's
		 * not described as "blessed" or "cursed".
		 *
		 * If the +/- isn't known, "uncursed" must be printed to
		 * avoid ambiguity between an item whose curse status is
		 * unknown, and an item known to be uncursed.
		 */
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
			&& obj->otyp != FAKE_AMULET_OF_YENDOR
			&& obj->otyp != AMULET_OF_YENDOR
			&& pl_character[0] != 'P')
		Strcat(prefix, "uncursed ");
	}
	if(obj->greased) Strcat(prefix, "greased ");
	switch(obj->oclass) {
	case AMULET_CLASS:
		if(obj->otyp == FAKE_AMULET_OF_YENDOR ||
		   obj->otyp == AMULET_OF_YENDOR)
		    if(strncmp(bp, "cheap ", 6)) {
			Strcpy(tmpbuf, "the ");
			Strcat(tmpbuf, prefix+2); /* skip the "a " */
			Strcpy(prefix, tmpbuf);
		    }
		if(obj->owornmask & W_AMUL)
			Strcat(bp, " (being worn)");
		break;
	case WEAPON_CLASS:
		if(ispoisoned)
			Strcat(prefix, "poisoned ");
plus:
		if (obj->oeroded) {
			switch (obj->oeroded) {
				case 2:	Strcat(prefix, "very "); break;
				case 3:	Strcat(prefix, "thoroughly "); break;
			}			
			Strcat(prefix,
			       is_rustprone(obj) ? "rusty " :
			       is_corrodeable(obj) ? "corroded " :
			       is_flammable(obj) ? "burnt " : "");
		} else if (obj->rknown && obj->oerodeproof)
			Strcat(prefix,
			       is_rustprone(obj) ? "rustproof " :
			       is_corrodeable(obj) ? "corrodeproof " :	/* "stainless"? */
			       is_flammable(obj) ? "fireproof " : "");
		if(obj->known) {
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case ARMOR_CLASS:
		if(obj->owornmask & W_ARMOR)
			Strcat(bp,
#ifdef POLYSELF
				(obj == uskin) ? " (embedded in your skin)" :
#endif
				" (being worn)");
		goto plus;
	case TOOL_CLASS:		/* temp. hack by GAN 11/18/86 */
		if(obj->owornmask & W_TOOL) { /* blindfold */
			Strcat(bp, " (being worn)");
			break;
		}
#ifdef WALKIES
		if(obj->otyp == LEASH && obj->leashmon != 0) {
			Strcat(bp, " (in use)");
			break;
		}
#endif
		if(obj->otyp == PICK_AXE || obj->otyp == UNICORN_HORN)
			goto plus;
		if (Is_candle(obj) &&
		    obj->age < 20L * (long)objects[obj->otyp].oc_cost)
			Sprintf(eos(prefix), "partly used ");
		if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
			obj->otyp == BRASS_LANTERN ||
		    Is_candle(obj) || obj->otyp == CANDELABRUM_OF_INVOCATION) {
			if(obj->lamplit)
				Sprintf(eos(bp), " (lit)");
			break;
		}
		if(!objects[obj->otyp].oc_charged) break;
		/* if special tool, fall through to show charges */
	case WAND_CLASS:
		if(obj->known)
			Sprintf(eos(bp), " (%d)", obj->spe);
		break;
	case RING_CLASS:
		if(obj->owornmask & W_RINGR) Strcat(bp, " (on right ");
		if(obj->owornmask & W_RINGL) Strcat(bp, " (on left ");
		if(obj->owornmask & W_RING) {
		    Strcat(bp, body_part(HAND));
		    Strcat(bp, ")");
		}
		if(obj->known && objects[obj->otyp].oc_charged) {
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case FOOD_CLASS:
		if (obj->oeaten)
		    Strcat(prefix, "partly eaten ");
		if (obj->otyp == CORPSE) {
		    if (type_is_pname(&mons[obj->corpsenm])) {
			Sprintf(prefix, "%s ",
				s_suffix(mons[obj->corpsenm].mname));
			if (obj->oeaten) Strcat(prefix, "partly eaten ");
		    } else {
			Strcat(prefix, mons[obj->corpsenm].mname);
			Strcat(prefix, " ");
		    }
		} else if (obj->otyp == EGG && obj->known) {
		    if (obj->corpsenm >= 0) {
			Strcat(prefix, mons[obj->corpsenm].mname);
			Strcat(prefix, " ");
#ifdef POLYSELF
			if (obj->spe)
			    Strcat(bp, " (laid by you)");
#endif
		    }
		}
		break;
	case BALL_CLASS:
		if(obj->owornmask & W_BALL)
			Strcat(bp, " (chained to you)");
			break;
	}

	if((obj->owornmask & W_WEP) && !mrg_to_wielded) {
		if (obj->quan != 1L)
			Strcat(bp, " (wielded)");
		else {
			Strcat(bp, " (weapon in ");
			Strcat(bp, body_part(HAND));
			Strcat(bp, ")");
		}
	}
	if(obj->unpaid)
		Strcat(bp, " (unpaid)");
	if (!strncmp(prefix, "a ", 2) &&
			index(vowels, *(prefix+2) ? *(prefix+2) : *bp)
			&& (*(prefix+2) || (strncmp(bp, "uranium", 7)
				&& strncmp(bp, "unicorn", 7)))) {
		Strcpy(tmpbuf, prefix);
		Strcpy(prefix, "an ");
		Strcpy(prefix+3, tmpbuf+2);
	}
	bp = strprepend(bp, prefix);
	return(bp);
}

#endif /* OVL0 */
#ifdef OVLB

/*
 * Used if only one of a collection of objects is named (e.g. in eat.c).
 */

char *
singular(otmp, func)
register struct obj *otmp;
char *FDECL((*func), (OBJ_P));
{
	long savequan;
	char *nam;

	/* Note: using xname for corpses will not give the monster type */
	if (otmp->otyp == CORPSE && func == xname) {
		static char NEARDATA buf[31];

		Sprintf(buf, "%s corpse", mons[otmp->corpsenm].mname);
		return buf;
	}
	savequan = otmp->quan;
	otmp->quan = 1L;
	nam = (*func)(otmp);
	otmp->quan = savequan;
	return nam;
}

char *
an(str)
register const char *str;
{
	static char NEARDATA buf[BUFSZ];

	buf[0] = '\0';

	if (strncmpi(str, "the ", 4) &&
	    strcmp(str, "molten lava") &&
	    strcmp(str, "ice"))
	    	if (index(vowels, *str) &&
		    strncmp(str, "useful", 6) &&
		    strncmp(str, "unicorn", 7) &&
		    strncmp(str, "uranium", 7))
		    	Strcpy(buf, "an ");
	    	else
		    Strcpy(buf, "a ");

	Strcat(buf, str);
	return buf;
}

char *
An(str)
const char *str;
{
	register char *tmp = an(str);
	*tmp = highc(*tmp);
	return tmp;
}

/*
 * Prepend "the" if necessary; assumes str is a subject derived from xname.
 * Use type_is_pname() for monster names, not the().  the() is idempotent.
 */
char *
the(str)
const char *str;
{
	static char NEARDATA buf[BUFSZ];

	if (!strncmpi(str, "the ", 4)) {
	    buf[0] = lowc(*str);
	    Strcpy(&buf[1], str+1);
	    return buf;
	} else if (*str < 'A' || *str > 'Z') {
	    /* not a proper name, needs an article */
	    Strcpy(buf, "the ");
	} else {
	    /* Probably a proper name, might not need an article */
	    register char *tmp;

	    buf[0] = 0;

	    /* some objects have capitalized adjectives in their names */
	    if(((tmp = rindex(str, ' ')) || (tmp = rindex(str, '-'))) &&
	       (tmp[1] < 'A' || tmp[1] > 'Z'))
		Strcpy(buf, "the ");
	    else if (tmp && (tmp = index(str, ' ')) != NULL) {
		/* it needs an article if the name contains "of" */
		while(tmp && strncmp(++tmp, "of ", 3))
		    tmp = index(tmp, ' ');
		if (tmp) /* found an "of" */
		    Strcpy(buf, "the ");
	    }
	}
	Strcat(buf, str);

	return buf;
}

char *
The(str)
const char *str;
{
    register char *tmp = the(str);
    *tmp = highc(*tmp);
    return tmp;
}

char *
aobjnam(otmp,verb)
register struct obj *otmp;
register const char *verb;
{
	register char *bp = xname(otmp);
	char prefix[PREFIX];

	if(otmp->quan != 1L) {
		Sprintf(prefix, "%ld ", otmp->quan);
		bp = strprepend(bp, prefix);
	}

	if(verb) {
		/* verb is given in plural (without trailing s) */
		Strcat(bp, " ");
		if(otmp->quan != 1L)
			Strcat(bp, verb);
		else if(!strcmp(verb, "are"))
			Strcat(bp, "is");
		else {
			Strcat(bp, verb);
			Strcat(bp, "s");
		}
	}
	return(bp);
}

char *
Doname2(obj)
register struct obj *obj;
{
	register char *s = doname(obj);

	if('a' <= *s && *s <= 'z') *s -= ('a' - 'A');
	return(s);
}

static const char *wrp[] = {
	"wand", "ring", "potion", "scroll", "gem", "amulet",
	"spellbook", "spell book",
	/* for non-specific wishes */
	"weapon", "armor", "tool", "food", "comestible",
};
static const char wrpsym[] = {
	WAND_CLASS, RING_CLASS, POTION_CLASS, SCROLL_CLASS, GEM_CLASS, 
        AMULET_CLASS, SPBOOK_CLASS, SPBOOK_CLASS,
	WEAPON_CLASS, ARMOR_CLASS, TOOL_CLASS, FOOD_CLASS, FOOD_CLASS
};

#endif /* OVLB */
#ifdef OVL0

/* Plural routine; chiefly used for user-defined fruits.  We have to try to
 * account for everything reasonable the player has; something unreasonable
 * can still break the code.  However, it's still a lot more accurate than
 * "just add an s at the end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi.")
 * and body parts.
 *
 * Also misused by muse.c to convert 1st person present verbs to 2nd person.
 */
char *
makeplural(oldstr)
const char *oldstr;
{
	/* Note: cannot use strcmpi here -- it'd give MATZot, CAVEMeN,... */
	register char *spot;
	static char NEARDATA str[BUFSZ];
	const char *excess;
	int len;

	while (*oldstr==' ') oldstr++;
	if (!oldstr || !*oldstr) {
		impossible("plural of null?");
		Strcpy(str, "s");
		return str;
	}
	Strcpy(str, oldstr);

	/* Search for common compounds, ex. lump of royal jelly */
	for(excess=(char *)0, spot=str; *spot; spot++) {
		if (!strncmp(spot, " of ", 4)
				|| !strncmp(spot, " labeled ", 9)
				|| !strncmp(spot, " called ", 8)
				|| !strncmp(spot, " named ", 7)
				|| !strcmp(spot, " above") /* lurkers above */
				|| !strncmp(spot, " versus ", 8)
#ifdef TUTTI_FRUTTI
				|| !strncmp(spot, " from ", 6)
				|| !strncmp(spot, " in ", 4)
				|| !strncmp(spot, " on ", 4)
				|| !strncmp(spot, " a la ", 6)
				|| !strncmp(spot, " with", 5)	/* " with "? */
				|| !strncmp(spot, " de ", 4)
				|| !strncmp(spot, " d'", 3)
				|| !strncmp(spot, " du ", 4)
#endif
				) {
			excess = oldstr + (int) (spot - str);
			*spot = 0;
			break;
		}
	}
	spot--;
	while (*spot==' ') spot--; /* Strip blanks from end */
	*(spot+1) = 0;
	/* Now spot is the last character of the string */

	len = strlen(str);
#ifdef TUTTI_FRUTTI
	/* Single letters */
	if (len==1 || !letter(*spot)) {
		Strcpy(spot+1, "'s");
		goto bottom;
	}
#endif

	/* man/men ("Wiped out all cavemen.") */
	if (len >= 3 && !strcmp(spot-2, "man") &&
			(len<6 || strcmp(spot-5, "shaman")) &&
			(len<5 || strcmp(spot-4, "human"))) {
		*(spot-1) = 'e';
		goto bottom;
	}

	/* tooth/teeth */
	if (len >= 5 && !strcmp(spot-4, "tooth")) {
		Strcpy(spot-3, "eeth");
		goto bottom;
	}

	/* knife/knives, etc... */
	if (!strcmp(spot-1, "fe"))
		*(spot-1) = 'v';
	else if (*spot == 'f')
		if (index("lr", *(spot-1)) || index(vowels, *(spot-1)))
			*spot = 'v';
		else if (len >= 5 && !strncmp(spot-4, "staf", 4))
			Strcpy(spot-1, "ve");

	/* foot/feet (body part) */
	if (len >= 4 && !strcmp(spot-3, "foot")) {
		Strcpy(spot-2, "eet");
		goto bottom;
	}

	/* ium/ia (mycelia, baluchitheria) */
	if (len >= 3 && !strcmp(spot-2, "ium")) {
		*(spot--) = (char)0;
		*spot = 'a';
		goto bottom;
	}

	/* algae, larvae, hyphae (another fungus part) */
#ifdef TUTTI_FRUTTI
	if ((len >= 4 && !strcmp(spot-3, "alga")) ||
	    (len >= 5 &&
	     (!strcmp(spot-4, "hypha") || !strcmp(spot-4, "larva"))))
#else
	if (len >= 5 && (!strcmp(spot-4, "hypha")))
#endif
	{
		Strcpy(spot, "ae");
		goto bottom;
	}

	/* fungus/fungi, homunculus/homunculi, but wumpuses */
	if (!strcmp(spot-1, "us") && (len < 6 || strcmp(spot-5, "wumpus"))) {
		*(spot--) = (char)0;
		*spot = 'i';
		goto bottom;
	}

	/* vortex/vortices */
	if (len >= 6 && !strcmp(spot-3, "rtex")) {
		Strcpy(spot-1, "ices");
		goto bottom;
	}

	/* djinni/djinn (note: also efreeti/efreet) */
	if (len >= 6 && !strcmp(spot-5, "djinni")) {
		*spot = (char)0;
		goto bottom;
	}

	/* mumak/mumakil */
	if (len >= 5 && !strcmp(spot-4, "mumak")) {
		Strcpy(spot, "il");
		goto bottom;
	}

	/* same singular and plural */
	/* note: also swine, trout, grouse */
	if ((len >= 7 && !strcmp(spot-6, "samurai")) ||
#ifdef TUTTI_FRUTTI
	    (len >= 5 &&
	     (!strcmp(spot-4, "manes") || !strcmp(spot-4, "sheep"))) ||
	    (len >= 4 &&
	     (!strcmp(spot-3, "fish") || !strcmp(spot-3, "tuna") ||
	      !strcmp(spot-3, "deer")))
#else
	    (len >= 5 && !strcmp(spot-4, "manes"))
#endif
	    ) goto bottom;

	/* sis/ses (nemesis) */
	if (len >= 3 && !strcmp(spot-2, "sis")) {
		*(spot-1) = 'e';
		goto bottom;
	}

#ifdef TUTTI_FRUTTI
	/* mouse/mice,louse/lice (not a monster, but possible in food names) */
	if (len >= 5 && !strcmp(spot-3, "ouse") && index("MmLl", *(spot-4))) {
		Strcpy(spot-3, "ice");
		goto bottom;
	}

	/* matzoh/matzot, possible food name */
	if (len >= 6 && (!strcmp(spot-5, "matzoh")
					|| !strcmp(spot-5, "matzah"))) {
		Strcpy(spot-1, "ot");
		goto bottom;
	}
	if (len >= 5 && (!strcmp(spot-4, "matzo")
					|| !strcmp(spot-5, "matza"))) {
		Strcpy(spot, "ot");
		goto bottom;
	}

	/* child/children (for wise guys who give their food funny names) */
	if (len >= 5 && !strcmp(spot-4, "child")) {
		Strcpy(spot, "dren");
		goto bottom;
	}

	/* note: -eau/-eaux (gateau, bordeau...) */
	/* note: ox/oxen, VAX/VAXen, goose/geese */
#endif

	/* Ends in z, x, s, ch, sh; add an "es" */
	if (index("zxsv", *spot)
			|| (len >= 2 && *spot=='h' && index("cs", *(spot-1)))
#ifdef TUTTI_FRUTTI
	/* Kludge to get "tomatoes" and "potatoes" right */
			|| (len >= 4 && !strcmp(spot-2, "ato"))
#endif
									) {
		Strcpy(spot+1, "es");
		goto bottom;
	}

	/* Ends in y preceded by consonant (note: also "qu") change to "ies" */
	if (*spot == 'y' &&
	    (!index(vowels, *(spot-1)))) {
		Strcpy(spot, "ies");
		goto bottom;
	}

	/* Japanese words: plurals are the same as singlar */
	if (len == 2 && !strcmp(str, "ya"))
	    goto bottom;

	/* Default: append an 's' */
	Strcpy(spot+1, "s");

bottom:	if (excess) Strcpy(eos(str), excess);
	return str;
}

#endif /* OVL0 */

struct o_range {
	const char *name, osym;
	int  f_o_range, l_o_range;
};

#ifndef OVLB

STATIC_DCL const struct o_range o_ranges[];

#else /* OVLB */

/* wishable subranges of objects */
STATIC_OVL const struct o_range NEARDATA o_ranges[] = {
	{ "bag",	TOOL_CLASS,   SACK,	      BAG_OF_TRICKS },
	{ "candle",	TOOL_CLASS,   TALLOW_CANDLE,  WAX_CANDLE },
	{ "horn",	TOOL_CLASS,   TOOLED_HORN,    HORN_OF_PLENTY },
	{ "gloves",	ARMOR_CLASS,  LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY },
	{ "gauntlets",	ARMOR_CLASS,  LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY },
	{ "boots",	ARMOR_CLASS,  LOW_BOOTS,      LEVITATION_BOOTS },
	{ "shoes",	ARMOR_CLASS,  LOW_BOOTS,      IRON_SHOES },
	{ "cloak",	ARMOR_CLASS,  MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT },
	{ "shield",	ARMOR_CLASS,  SMALL_SHIELD,   SHIELD_OF_REFLECTION },
	{ "helm",	ARMOR_CLASS,  ELVEN_LEATHER_HELM, HELM_OF_TELEPATHY },
	{ "dragon scales",
			ARMOR_CLASS,  GRAY_DRAGON_SCALES, YELLOW_DRAGON_SCALES },
	{ "dragon scale mail",
			ARMOR_CLASS,  GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL },
	{ "sword",	WEAPON_CLASS, SHORT_SWORD,    KATANA },
#ifdef WIZARD
	{ "venom",	VENOM_CLASS,  BLINDING_VENOM, ACID_VENOM },
#endif
};

#define BSTRCMP(base,ptr,string) ((ptr) < base || strcmp((ptr),string))
#define BSTRCMPI(base,ptr,string) ((ptr) < base || strcmpi((ptr),string))
#define BSTRNCMP(base,ptr,string,num) ((ptr)<base || strncmp((ptr),string,num))

/*
 * Singularize a string the user typed in; this helps reduce the complexity
 * of readobjnam, and is also used in pager.c to singularize the string
 * for which help is sought.
 */

char *
makesingular(oldstr)
const char *oldstr;
{
	register char *p, *bp;
	static char NEARDATA str[BUFSZ];

	if (!oldstr || !*oldstr) {
		impossible("singular of null?");
		str[0] = 0; return str;
	}
	Strcpy(str, oldstr);
	bp = str;

	while (*bp == ' ') bp++;
	/* find "cloves of garlic", "worthless pieces of blue glass" */
	if ((p = strstri(bp, "s of ")) != 0) {
	    /* but don't singularize "gauntlets" */
	    if (BSTRNCMP(bp, p-8, "gauntlet", 8))
		while ((*p = *(p+1)) != 0) p++;
	    return bp;
	}

	/* remove -s or -es (boxes) or -ies (rubies) */
	p = eos(bp);
	if (p >= bp+1 && p[-1] == 's') {
		if (p >= bp+2 && p[-2] == 'e') {
			if (p >= bp+3 && p[-3] == 'i') {
				if(!BSTRCMP(bp, p-7, "cookies") ||
				   !BSTRCMP(bp, p-4, "pies"))
					goto mins;
				Strcpy(p-3, "y");
				return bp;
			}

			/* note: cloves / knives from clove / knife */
			if(!BSTRCMP(bp, p-6, "knives")) {
				Strcpy(p-3, "fe");
				return bp;
			}

			if(!BSTRCMP(bp, p-6, "staves")) {
				Strcpy(p-3, "ff");
				return bp;
			}

			/* note: nurses, axes but boxes */
			if(!BSTRCMP(bp, p-5, "boxes")) {
				p[-2] = 0;
				return bp;
			}
			if (!BSTRCMP(bp, p-6, "gloves") ||
			    !BSTRCMP(bp, p-5, "shoes") ||
			    !BSTRCMP(bp, p-6, "scales"))
				return bp;
		} else if (!BSTRCMP(bp, p-5, "boots") ||
			   !BSTRCMP(bp, p-6, "tricks") ||
			   !BSTRCMP(bp, p-9, "paralysis") ||
			   !BSTRCMP(bp, p-5, "glass") ||
			   !BSTRCMP(bp, p-4, "ness") ||
			   !BSTRCMP(bp, p-14, "shape changers") ||
			   !BSTRCMP(bp, p-15, "detect monsters") ||
			   !BSTRCMPI(bp, p-11, "Aesculapius"))	/* staff */
				return bp;
	mins:
		p[-1] = 0;
	} else {
		if(!BSTRCMP(bp, p-5, "teeth")) {
			Strcpy(p-5, "tooth");
			return bp;
		}
		/* here we cannot find the plural suffix */
	}
	return bp;
}

/* alternate spellings: extra space, space instead of hyphen, etc */
struct alt_spellings {
	const char *sp;
	int ob;
} spellings[] = {
	{ "two handed sword", TWO_HANDED_SWORD },
	{ "battle axe", BATTLE_AXE },
	{ "lockpick", LOCK_PICK },
	{ "pick axe", PICK_AXE },
	{ "luck stone", LUCKSTONE },
	{ "load stone", LOADSTONE },
	{ "broad sword", BROADSWORD },
	{ "elven broad sword", ELVEN_BROADSWORD },
	{ "longsword", LONG_SWORD },
	{ "shortsword", SHORT_SWORD },
	{ "elven shortsword", ELVEN_SHORT_SWORD },
	{ "dwarvish shortsword", DWARVISH_SHORT_SWORD },
	{ "orcish shortsword", ORCISH_SHORT_SWORD },
	{ "warhammer", WAR_HAMMER },
	{ "grey dragon scale mail", GRAY_DRAGON_SCALE_MAIL },
	{ "grey dragon scales", GRAY_DRAGON_SCALES },
	{ "iron ball", HEAVY_IRON_BALL },
	{ "stone", ROCK },
	{ (const char *)0, 0 },
};

/* Return something wished for.  If not an object, return &zeroobj; if an error
 * (no matching object), return (struct obj *)0.  Giving readobjnam() a null
 * pointer skips the error return and creates a random object instead.
 */
struct obj *
readobjnam(bp)
register char *bp;
{
	register char *p;
	register int i;
	register struct obj *otmp;
	int cnt, spe, spesgn, typ, very;
	int blessed, uncursed, iscursed, ispoisoned;
	int eroded, erodeproof;
	int halfeaten, mntmp, contents;
	int islit, unlabeled;
#ifdef TUTTI_FRUTTI
	struct fruit *f;
	int ftype = current_fruit;
	char fruitbuf[BUFSZ];
	/* We want to check for fruits last so that, for example, someone
	 * who names their fruit "katana" and wishes for a katana gets a real
	 * one.  But, we have to keep around the old buf since in the meantime
	 * we have deleted "empty", "+6", etc...
	 */
#endif
	char let;
	char *un, *dn, *actualn;
	const char *name=0;

	cnt = spe = spesgn = typ = very = blessed = uncursed =
		iscursed = ispoisoned = eroded = erodeproof = halfeaten =
		islit = unlabeled = 0;
	mntmp = -1;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
	contents = UNDEFINED;
	let = 0;
	actualn = dn = un = 0;

	/* first, remove extra whitespace they may have typed */
	if (bp) {
		char c, *p2;
		boolean was_space = TRUE;

		for (p = p2 = bp; (c = *p) != '\0'; p++) {
		     /* if (c == '\t') c = ' '; */
			if (c != ' ' || !was_space)  *p2++ = c;
			was_space = (c == ' ');
		}
		if (was_space && p2 > bp)  p2--;
		*p2 = '\0';
	}

	for(;;) {
		register int l;

		if (!bp || !*bp) goto any;
		if (!strncmpi(bp, "an ", l=3) ||
		    !strncmpi(bp, "a ", l=2)) {
			cnt = 1;
		} else if (!strncmpi(bp, "the ", l=4)) {
			;	/* just increment `bp' by `l' below */
		} else if (!cnt && digit(*bp)) {
			cnt = atoi(bp);
			while(digit(*bp)) bp++;
			while(*bp == ' ') bp++;
			l = 0;
		} else if (!strncmpi(bp, "blessed ", l=8) ||
			   !strncmpi(bp, "holy ", l=5)) {
			blessed = 1;
		} else if (!strncmpi(bp, "cursed ", l=7) ||
			   !strncmpi(bp, "unholy ", l=7)) {
			iscursed = 1;
		} else if (!strncmpi(bp, "uncursed ", l=9)) {
			uncursed = 1;
		} else if (!strncmp(bp, "rustproof ", l=10) ||
			   !strncmp(bp, "erodeproof ", l=11) ||
			   !strncmp(bp, "corrodeproof ", l=13) ||
			   !strncmp(bp, "fireproof ", l=10)) {
			erodeproof = 1;
		} else if (!strncmpi(bp,"lit ", l=4) ||
			   !strncmpi(bp,"burning ", l=8)) {
			islit = 1;
		} else if (!strncmpi(bp,"unlit ", l=6) ||
			   !strncmpi(bp,"extinguished ", l=13)) {
			islit = 0;
		/* "unlabeled" and "blank" are synonymous */
		} else if (!strncmpi(bp,"unlabeled ", l=10) ||
			   !strncmpi(bp,"unlabelled ", l=11) ||
			   !strncmpi(bp,"blank ", l=6)) {
			unlabeled = 1;
		} else if (!strncmpi(bp, "very ", l=5)) {
			very = 1;
		} else if (!strncmpi(bp, "thoroughly ", l=11)) {
			very = 2;
		} else if (!strncmp(bp, "rusty ", l=6) ||
			   !strncmp(bp, "rusted ", l=7) ||
			   !strncmp(bp, "eroded ", l=7) ||
			   !strncmp(bp, "corroded ", l=9) ||
			   !strncmp(bp, "burnt ", l=6) ||
			   !strncmp(bp, "burned ", l=7) ||
			   !strncmp(bp, "rotted ", l=7)) {
			eroded = 1 + very; very = 0;
		} else if (!strncmpi(bp, "partly eaten ", l=13)) {
			halfeaten = 1;
		} else break;
		bp += l;
	}
	if(!cnt) cnt = 1;		/* %% what with "gems" etc. ? */
#ifdef TUTTI_FRUTTI
	Strcpy(fruitbuf, bp);
#endif
	if(!strncmpi(bp, "empty ", 6)) {
		contents = EMPTY;
		bp += 6;
	} else if(!strncmpi(bp, "poisoned ",9)) {
		ispoisoned=1;
		bp += 9;
#ifdef WIZARD
	} else if(wizard && !strncmpi(bp, "trapped ",8)) {
		ispoisoned=1;
		bp += 8;
#endif
	}
	if (strlen(bp) > 1) {
	    if (*bp == '+' || *bp == '-') {
		spesgn = (*bp++ == '+') ? 1 : -1;
		spe = atoi(bp);
		while(digit(*bp)) bp++;
		while(*bp == ' ') bp++;
	    } else if ((p = rindex(bp, '(')) != 0) {
		if (p > bp && p[-1] == ' ') p[-1] = 0;
		else *p = 0;
		p++;
		if (!strcmpi(p, "lit)"))
		    islit = 1;
		else {
		    spe = atoi(p);
		    while(digit(*p)) p++;
		    if (*p != ')') spe = 0;
		    else {
			spesgn = 1;
			p++;
			if (*p) Strcat(bp, p);
		    }
		}
	    }
	}
/*
   otmp->spe is type schar; so we don't want spe to be any bigger or smaller.
   also, spe should always be positive  -- some cheaters may try to confuse
   atoi()
*/
	if (spe < 0) {
		spesgn = -1;	/* cheaters get what they deserve */
		spe = abs(spe);
	}
	if (spe > SCHAR_LIM)
		spe = SCHAR_LIM;

	/* now we have the actual name, as delivered by xname, say
		green potions called whisky
		scrolls labeled "QWERTY"
		egg
		fortune cookies
		very heavy iron ball named hoei
		wand of wishing
		elven cloak
	*/
	if ((p = strstri(bp, " named ")) != 0) {
		*p = 0;
		name = p+7;
	}
	if ((p = strstri(bp, " called ")) != 0) {
		*p = 0;
		un = p+8;
		/* "helmet called telepathy" is not "helmet" (a specific type)
		 * "shield called reflection" is not "shield" (a general type)
		 */
		for(i = 0; i < SIZE(o_ranges); i++)
		    if(!strcmpi(bp, o_ranges[i].name)) {
			let = o_ranges[i].osym;
			goto srch;
		    }
	}
	if ((p = strstri(bp, " labeled ")) != 0) {
		*p = 0;
		dn = p+9;
	} else if ((p = strstri(bp, " labelled ")) != 0) {
		*p = 0;
		dn = p+10;
	}
	if ((p = strstri(bp, " of spinach")) != 0) {
		*p = 0;
		contents = SPINACH;
	}

	/* Skip over "pair of ", then jump to the singular since we don't
	   need to convert "gloves" or "boots". */
	if(cnt == 1 && !strncmpi(bp, "pair of ",8)) {
		bp += 8;
		cnt = 2;
		goto sing;
		/* cnt is ignored for armor and other non-stackable objects;
		   DTRT for stackable objects */
	} else if(cnt > 1 && !strncmpi(bp, "pairs of ",9)) {
		bp += 9;
		cnt *= 2;
	} else if (!strncmpi(bp, "set of ",7)) {
		bp += 7;
	} else if (!strncmpi(bp, "sets of ",8)) {
		bp += 8;
	}

	/*
	 * Find corpse type using "of" (figurine of an orc, tin of orc meat)
	 * Don't check if it's a wand or spellbook.
	 * (avoid "wand/finger of death" confusion).
	 */
	if (!strstri(bp, "wand ")
	 && !strstri(bp, "spellbook ")
	 && !strstri(bp, "finger ")) {
	    if ((p = strstri(bp, " of ")) != 0
		&& (mntmp = name_to_mon(p+4)) >= 0)
		*p = 0;
	}
	/* Find corpse type w/o "of" (red dragon scale mail, yeti corpse) */
	if (strncmp(bp, "samurai sword", 13)) /* not the "samurai" monster! */
	if (strncmp(bp, "wizard lock", 11)) /* not the "wizard" monster! */
	if (strncmp(bp, "ninja-to", 8)) /* not the "ninja" rank */
	if (mntmp < 0 && strlen(bp) > 2 && (mntmp = name_to_mon(bp)) >= 0) {
		int mntmptoo, mntmplen;	/* double check for rank title */
		char *obp = bp;
		mntmptoo = title_to_mon(bp, (int *)0, &mntmplen);
		bp += mntmp != mntmptoo ? strlen(mons[mntmp].mname) : mntmplen;
		if (*bp == ' ') bp++;
		else if (!strncmpi(bp, "s ", 2)) bp += 2;
		else if (!strncmpi(bp, "es ", 3)) bp += 3;
		else if (!*bp && !actualn && !dn && !un && !let) {
		    /* no referent; they don't really mean a monster type */
		    bp = obp;
		    mntmp = -1;
		}
	}

	/* first change to singular if necessary */
	if (*bp) {
		char *sng = makesingular(bp);
		if (strcmp(bp, sng)) {
			if (cnt == 1) cnt = 2;
			Strcpy(bp, sng);
		}
	}

sing:
	/* Alternate spellings (two-handed sword vs. two handed sword) */
	{struct alt_spellings *as = spellings;
		while(as->sp) {
			if (!strcmpi(bp, as->sp)) {
				typ = as->ob;
				goto typfnd;
			}
			as++;
		}
	}

	/* dragon scales - assumes order of dragons */
	if(!strcmpi(bp, "scales") &&
			mntmp >= PM_GRAY_DRAGON && mntmp <= PM_YELLOW_DRAGON) {
		typ = GRAY_DRAGON_SCALES + mntmp - PM_GRAY_DRAGON;
		mntmp = -1;	/* no monster */
		goto typfnd;
	}

	if(!strcmpi(bp, "ring mail") ||	/* Note: ring mail is not a ring ! */
	   !strcmpi(bp, "leather armor") || /* Prevent falling to 'armor'. */
	   !strcmpi(bp, "studded leather armor")) {
		let = ARMOR_CLASS;
		actualn = bp;
		goto srch;
	}
	if(!strcmpi(bp, "food ration")){
		let = FOOD_CLASS;
		actualn = bp;
		goto srch;
	}
	p = eos(bp);
	if(!BSTRCMPI(bp, p-10, "holy water")) {
		typ = POT_WATER;
		if ((p-bp) >= 12 && *(p-12) == 'u')
			iscursed = 1; /* unholy water */
		else blessed = 1;
		goto typfnd;
	}
	if(unlabeled && !BSTRCMPI(bp, p-6, "scroll")) {
		typ = SCR_BLANK_PAPER;
		goto typfnd;
	}
	if(unlabeled && !BSTRCMPI(bp, p-9, "spellbook")) {
		typ = SPE_BLANK_PAPER;
		goto typfnd;
	}
#ifdef TOURIST
	if (!BSTRCMPI(bp, p-5, "shirt")) {
		typ = HAWAIIAN_SHIRT;
		goto typfnd;
	}
#endif
	/*
	 * NOTE: Gold pieces are handled as objects nowadays, and therefore
	 * this section should probably be reconsidered as well as the entire
	 * gold/money concept.  Maybe we want to add other monetary units as
	 * well in the future. (TH)
	 */
	if(!BSTRCMPI(bp, p-10, "gold piece") || !BSTRCMPI(bp, p-7, "zorkmid") ||
	   !strcmpi(bp, "gold") || !strcmpi(bp, "money") || 
	   !strcmpi(bp, "coin") || *bp == GOLD_SYM) {
			if (cnt > 5000
#ifdef WIZARD
					&& !wizard
#endif
						) cnt=5000;
		if (cnt < 1) cnt=1;
		pline("%d gold piece%s.", cnt, plur(cnt));
		u.ugold += cnt;
		flags.botl=1;
		return (&zeroobj);
	}
	if (strlen(bp) == 1 &&
	   (i = def_char_to_objclass(*bp)) < MAXOCLASSES && i > ILLOBJ_CLASS) {
		let = i;
		goto any;
	}
	if(strncmpi(bp, "enchant ", 8) &&
	   strncmpi(bp, "destroy ", 8) &&
	   strncmpi(bp, "food detection", 14))
	/* allow wishes for "enchant weapon" and "food detection" */
	for(i = 0; i < sizeof(wrpsym); i++) {
		register int j = strlen(wrp[i]);
		if(!strncmpi(bp, wrp[i], j)){
			let = wrpsym[i];
			if(let != AMULET_CLASS) {
			    bp += j;
			    if(!strncmpi(bp, " of ", 4)) actualn = bp+4;
			    /* else if(*bp) ?? */
			} else
			    actualn = bp;
			goto srch;
		}
		if(!BSTRCMPI(bp, p-j, wrp[i])){
			let = wrpsym[i];
			p -= j;
			*p = 0;
			if(p > bp && p[-1] == ' ') p[-1] = 0;
			actualn = dn = bp;
			goto srch;
		}
	}
	if (!BSTRCMPI(bp, p-6, " stone")) {
		p[-6] = 0;
		let = GEM_CLASS;
		dn = actualn = bp;
		goto srch;
	} else if (!BSTRCMPI(bp, p-6, " glass") || !strcmpi(bp, "glass")) {
		register char *g = bp;
		if (strstri(g, "broken")) return (struct obj *)0;
		if (!strncmpi(g, "worthless ", 10)) g += 10;
		if (!strncmpi(g, "piece of ", 9)) g += 9;
		if (!strncmpi(g, "colored ", 8)) g += 8;
		if (!strcmpi(g, "glass")) {	/* choose random color */
			/* white, blue, red, yellowish brown, green, violet */
			typ = LAST_GEM + rnd(6);
			if (objects[typ].oc_class == GEM_CLASS) goto typfnd;
			else typ = 0;	/* somebody changed objects[]? punt */
		} else if (g > bp) {	/* try to construct canonical form */
			char tbuf[BUFSZ];
			Strcpy(tbuf, "worthless piece of ");
			Strcat(tbuf, g);  /* assume it starts with the color */
			Strcpy(bp, tbuf);
		}
	}
#ifdef WIZARD
	/* Let wizards wish for traps --KAA */
	if (wizard) {
		int trap;
		char *tname;

		for (trap = NO_TRAP+1; trap < TRAPNUM; trap++) {
			tname = index(traps[trap], ' ');
			if (tname) {
			    if (!strncmpi(tname+1, bp, strlen(tname+1))) {
				/* avoid stupid mistakes */
				if(trap == TRAPDOOR && !Can_fall_thru(&u.uz))
				    trap = ROCKTRAP;

				(void) maketrap(u.ux, u.uy, trap);
				pline("A%s.", traps[trap]);
				return(&zeroobj);
			    }
			}
		}
		/* or some other dungeon features -dlc */
		p = eos(bp);
		if(!BSTRCMP(bp, p-8, "fountain")) {
			levl[u.ux][u.uy].typ = FOUNTAIN;
			if(!strncmpi(bp, "magic ", 6))
				levl[u.ux][u.uy].blessedftn = 1;
			pline("A %sfountain.",
			      levl[u.ux][u.uy].blessedftn ? "magic " : "");
			newsym(u.ux, u.uy);
			return(&zeroobj);
		}
		if(!BSTRCMP(bp, p-5, "altar")) {
		    aligntyp al;

		    levl[u.ux][u.uy].typ = ALTAR;
		    if(!strncmpi(bp, "chaotic ", 8))
			al = A_CHAOTIC;
		    else if(!strncmpi(bp, "neutral ", 8))
			al = A_NEUTRAL;
		    else if(!strncmpi(bp, "lawful ", 7))
			al = A_LAWFUL;
		    else if(!strncmpi(bp, "unaligned ", 10))
			al = A_NONE;
		    else /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
			al = (!rn2(6)) ? A_NONE : rn2((int)A_LAWFUL+2) - 1;
		    levl[u.ux][u.uy].altarmask = Align2amask( al );
		    pline("%s altar.", An(align_str(al)));
		    newsym(u.ux, u.uy);
		    return(&zeroobj);
		}
	}
#endif
	for (i = 0; i < SIZE(o_ranges); i++)
	    if(!strcmpi(bp, o_ranges[i].name)) {
		typ = rnd_class(o_ranges[i].f_o_range, o_ranges[i].l_o_range);
		goto typfnd;
	    }

	actualn = bp;
	if (!dn) dn = actualn; /* ex. "skull cap" */
srch:
	/* check real names of gems first */
	if(!let && actualn) {
	    for(i = bases[letindex(GEM_CLASS)]; i <= LAST_GEM; i++) {
		register const char *zn;

		if((zn = OBJ_NAME(objects[i])) && !strcmpi(actualn, zn)) {
		    typ = i;
		    goto typfnd;
		}
	    }
	}
	i = 1;
	if(let) i = bases[letindex(let)];
	while(i <= NROFOBJECTS && (!let || objects[i].oc_class == let)){
		register const char *zn;

		if(actualn && (zn = OBJ_NAME(objects[i])) && !strcmpi(actualn, zn)) {
			typ = i;
			goto typfnd;
		}
		if(dn && (zn = OBJ_DESCR(objects[i])) && !strcmpi(dn, zn)) {
			/* don't match extra descriptions (w/o real name) */
			if (!OBJ_NAME(objects[i])) return (struct obj *)0;
			typ = i;
			goto typfnd;
		}
		if(un && (zn = objects[i].oc_uname) && !strcmpi(un, zn)) {
			typ = i;
			goto typfnd;
		}
		i++;
	}
	if (actualn) {
		struct Jitem *j = Japanese_items;
		while(j->item) {
			if (actualn && !strcmpi(actualn, j->name)) {
				typ = j->item;
				goto typfnd;
			}
			j++;
		}
	}
#ifdef TUTTI_FRUTTI
	/* Note: not strncmpi.  2 fruits, one capital, one not, is possible. */
	for(f=ffruit; f; f = f->nextf) {
		char *f1 = f->fname, *f2 = makeplural(f->fname);

		if(!strncmp(fruitbuf, f1, strlen(f1)) ||
					!strncmp(fruitbuf, f2, strlen(f2))) {
			typ = SLIME_MOLD;
			ftype = f->fid;
			goto typfnd;
		}
	}
#endif
	if (!strcmpi(bp, "spinach")) {
		contents = SPINACH;
		typ = TIN;
		goto typfnd;
	}

	if(!let && actualn) {
	    short objtyp;

	    /* Perhaps it's an artifact specified by name, not type */
	    name = artifact_name(actualn, &objtyp);
	    if(name) {
		typ = objtyp;
		goto typfnd;
	    }
	}
	if(!let) return((struct obj *)0);
any:
	if(!let) let = wrpsym[rn2((int)sizeof(wrpsym))];
typfnd:
	if (typ) let = objects[typ].oc_class;

	/* check for some objects that are not allowed */
	if (typ && objects[typ].oc_unique
#ifdef WIZARD
	    && !wizard 
	    /* should check flags.made_amulet, but it's not set anywhere */
#endif
	   )
	    switch (typ) {
		case AMULET_OF_YENDOR:
		    typ = FAKE_AMULET_OF_YENDOR;
		    break;
		case CANDELABRUM_OF_INVOCATION:
		    typ = rnd_class(TALLOW_CANDLE, WAX_CANDLE);
		    break;
		case BELL_OF_OPENING:
		    typ = BELL;
		    break;
		case SPE_BOOK_OF_THE_DEAD:
		    typ = SPE_BLANK_PAPER;
		    break;
	    }

	/* venom isn't really an object and can't be wished for; but allow
	 * wizards to wish for it since it's faster than polymorphing and
	 * spitting.
	 */
	if(let == VENOM_CLASS)
#ifdef WIZARD
		if (!wizard)
#endif
			return((struct obj *)0);

	if(typ) {
		otmp = mksobj(typ, TRUE, FALSE);
	} else {
		otmp = mkobj(let, FALSE);
		if (otmp) typ = otmp->otyp;
	}

	if(typ == OIL_LAMP || typ == MAGIC_LAMP || typ == BRASS_LANTERN)
		otmp->lamplit = islit;

	if(cnt > 0 && objects[typ].oc_merge && let != SPBOOK_CLASS && 
		(cnt < rnd(6) ||
#ifdef WIZARD
		wizard ||
#endif
		 (cnt <= 20 &&
		  ((let == WEAPON_CLASS && typ <= SHURIKEN) || (typ == ROCK)))))
			otmp->quan = (long) cnt;

#ifdef WIZARD
	if (let == VENOM_CLASS) otmp->spe = 1;
#endif

	if (spesgn == 0) spe = otmp->spe;
#ifdef WIZARD
	else if (wizard) /* no alteration to spe */ ;
#endif
	else if (let == ARMOR_CLASS || let == WEAPON_CLASS || typ == PICK_AXE ||
			typ == UNICORN_HORN ||
			(let==RING_CLASS && objects[typ].oc_charged)) {
		if(spe > rnd(5) && spe > otmp->spe) spe = 0;
		if(spe > 2 && Luck < 0) spesgn = -1;
	} else {
		if (let == WAND_CLASS) {
			if (spe > 1 && spesgn == -1) spe = 1;
		} else {
			if (spe > 0 && spesgn == -1) spe = 0;
		}
		if (spe > otmp->spe) spe = otmp->spe;
	}

	if (spesgn == -1) spe = -spe;

	/* set otmp->spe.  This may, or may not, use spe... */
	switch (typ) {
		case TIN: if (contents==EMPTY) {
				otmp->corpsenm = -1;
				otmp->spe = 0;
			} else if (contents==SPINACH) {
				otmp->corpsenm = -1;
				otmp->spe = 1;
			}
			break;
#ifdef TUTTI_FRUTTI
		case SLIME_MOLD: otmp->spe = ftype;
			/* Fall through */
#endif
		case SKELETON_KEY: case CHEST: case LARGE_BOX:
		case HEAVY_IRON_BALL: case IRON_CHAIN: case STATUE:
			/* otmp->cobj already done in mksobj() */
				break;
#ifdef MAIL
		case SCR_MAIL: otmp->spe = 1; break;
#endif
		case WAN_WISHING:
#ifdef WIZARD
			if (!wizard) {
#endif
				otmp->spe = (rn2(10) ? -1 : 0);
				break;
#ifdef WIZARD
			}
			/* fall through (twice), if wizard */
#endif
		case MAGIC_LAMP:
#ifdef WIZARD
			if (!wizard) {
#endif
				otmp->spe = 0;
				break;
#ifdef WIZARD
			}
			/* fall through, if wizard */
#endif
		default: otmp->spe = spe;
	}

	/* set otmp->corpsenm or dragon scale [mail] */
	if (mntmp > -1) switch(typ) {
		case TIN:
			otmp->spe = 0; /* No spinach */
		case CORPSE:
			if (!(mons[mntmp].geno & (G_UNIQ | G_NOCORPSE)))
				otmp->corpsenm = mntmp;
			break;
		case FIGURINE:
			if (!(mons[mntmp].geno & G_UNIQ)
			    && !is_human(&mons[mntmp]))
				otmp->corpsenm = mntmp;
			break;
		case EGG: if (lays_eggs(&mons[mntmp]) || mntmp==PM_KILLER_BEE)
				otmp->corpsenm = mntmp;
			break;
		case STATUE: otmp->corpsenm = mntmp;
			break;
		case SCALE_MAIL:
			/* Dragon mail - depends on the order of objects */
			/*		 & dragons.			 */
	    		if (mntmp >= PM_GRAY_DRAGON &&
						mntmp <= PM_YELLOW_DRAGON)
			    otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
						    mntmp - PM_GRAY_DRAGON;
			break;
	}

	/* set blessed/cursed -- setting the fields directly is safe
	 * since weight() is called below and addinv() will take care
	 * of luck */
	if (iscursed) {
		curse(otmp);
	} else if (uncursed) {
		otmp->blessed = 0;
		otmp->cursed = (Luck < 0
#ifdef WIZARD
					 && !wizard
#endif
							);
	} else if (blessed) {
		otmp->blessed = (Luck >= 0
#ifdef WIZARD
					 || wizard
#endif
							);
		otmp->cursed = (Luck < 0
#ifdef WIZARD
					 && !wizard
#endif
							);
	} else if (spesgn < 0) {
		curse(otmp);
	}

	/* set eroded */
	if (eroded)
		otmp->oeroded = eroded;

	/* set erodeproof */
	else if (erodeproof)
		otmp->oerodeproof = (Luck >= 0
#ifdef WIZARD
					 || wizard
#endif
				    );

	/* prevent wishing abuse */
	if (
#ifdef WIZARD
		!wizard &&
#endif
			otmp->otyp == WAN_WISHING)
		otmp->recharged = 1;

	/* set poisoned */
	if (ispoisoned) {
	    if (let == WEAPON_CLASS && typ <= SHURIKEN)
		otmp->opoisoned = (Luck >= 0);
	    else if (Is_box(otmp))
		otmp->otrapped = 1;
	    else if (let == FOOD_CLASS)
		/* try to taint by making it as old as possible */
	    	otmp->age = 1L;
	}

	if (name) {
		otmp = oname(otmp, name, 0);
		if (otmp->oartifact) otmp->quan = 1L;
	}
	otmp->owt = weight(otmp);
	if (very && otmp->otyp == HEAVY_IRON_BALL) otmp->owt += 160;
	if (halfeaten && otmp->oclass == FOOD_CLASS) {
		if (otmp->otyp == CORPSE)
			otmp->oeaten = mons[otmp->corpsenm].cnutrit;
		else otmp->oeaten = objects[otmp->otyp].oc_nutrition;
		otmp->owt /= 2;
		otmp->oeaten /= 2;
		if (!otmp->owt) otmp->owt = 1;
		if (!otmp->oeaten) otmp->oeaten = 1;
	}
	return(otmp);
}

int
rnd_class(first,last)
int first,last;
{
	int i, x, sum=0;
	for(i=first; i<=last; i++)
		sum += objects[i].oc_prob;
	if (!sum) /* all zero */
		return first + rn2(last-first+1);
	x = rnd(sum);
	for(i=first; i<=last; i++)
		if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
			return i;
	return 0;
}

STATIC_OVL const char *
Japanese_item_name(i)
int i;
{
	struct Jitem *j = Japanese_items;

	while(j->item) {
		if (i == j->item)
			return j->name;
		j++;
	}
	return (const char *)0;
}
#endif /* OVLB */

/*objnam.c*/
