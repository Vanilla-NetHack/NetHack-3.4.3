/*	SCCS Id: @(#)objnam.c	3.0	88/11/30
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#include <ctype.h>	/* for isalpha() */

#define	PREFIX	30
#define SCHAR_MAX 127

/*	We want the player to be able to learn what key goes in what lock.  */
const char *keystr[N_LOX] = { "round", "square", "triangular", "oval",
			    "octagonal", "hexagonal", "cylindrical",
			    "irregular", "conical", "wedge-shaped" },
	   *lockstr[N_LOX] = { "round", "square", "triangular", "oval",
			    "octagonal", "hexagonal", "wide",
			    "notched", "large round", "large square" };

static int rnd_class P((int,int));

static int
named_key(s) register char *s; {
	char tc[BUFSZ];
	register int i;

	for(i=0; i<10; i++) {
		Strcpy(tc, keystr[i]);
		Strcat(tc, " key");
		if(!strcmp(s,tc)) return(i+1);
	}
	return(0);
}

static int
named_box(s)
register char *s;
{
	char tc[BUFSZ];
	register int i;

	for(i=0; i<10; i++) {
		Strcpy(tc, lockstr[i]);
		Strcat(tc, " keyhole)");
		if(!strcmp(s,tc)) return(i+1);
	}
	return(0);
}

static char *
strprepend(s,pref) register char *s, *pref; {
register int i = strlen(pref);
	if(i > PREFIX) {
		pline("WARNING: prefix too short.");
		return(s);
	}
	s -= i;
	(void) strncpy(s, pref, i);	/* do not copy trailing 0 */
	return(s);
}

static char *
sitoa(a) int a; {
#ifdef LINT	/* static char buf[13]; */
	char buf[13];
#else
	static char buf[13];
#endif
	Sprintf(buf, (a < 0) ? "%d" : "+%d", a);
	return(buf);
}

char *
typename(otyp)
register int otyp;
{
#ifdef LINT	/* static char buf[BUFSZ]; */
char buf[BUFSZ];
#else
static char buf[BUFSZ];
#endif
register struct objclass *ocl = &objects[otyp];
register char *an = ocl->oc_name;
register char *dn = ocl->oc_descr;
register char *un = ocl->oc_uname;
register int nn = ocl->oc_name_known;
	switch(ocl->oc_olet) {
	case POTION_SYM:
		Strcpy(buf, "potion");
		break;
	case SCROLL_SYM:
		Strcpy(buf, "scroll");
		break;
	case WAND_SYM:
		Strcpy(buf, "wand");
		break;
#ifdef SPELLS
	case SPBOOK_SYM:
		Strcpy(buf, "spellbook");
		break;
#endif
	case RING_SYM:
		Strcpy(buf, "ring");
		break;
	case AMULET_SYM:
		if(nn)
			Strcpy(buf,an);
		else
			Strcpy(buf,"amulet");
		if(un)
			Sprintf(eos(buf)," called %s",un);
		if(dn)
			Sprintf(eos(buf)," (%s)",dn);
		return(buf);
	default:
		if(nn) {
			Strcpy(buf, an);
			if(otyp >= TURQUOISE && otyp <= JADE)
				Strcat(buf, " stone");
			if(un)
				Sprintf(eos(buf), " called %s", un);
			if(dn)
				Sprintf(eos(buf), " (%s)", dn);
		} else {
			Strcpy(buf, dn ? dn : an);
			if(ocl->oc_olet == GEM_SYM) {
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
		Sprintf(eos(buf), " of %s", an);
	if(un)
		Sprintf(eos(buf), " called %s", un);
	if(dn)
		Sprintf(eos(buf), " (%s)", dn);
	return(buf);
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
char *(*func) P((struct obj *));
{
	char *str;

	long save_Blinded = Blinded;
	Blinded = 1;
	str = (*func)(obj);
	Blinded = save_Blinded;
	return str;
}

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
register int nn = objects[obj->otyp].oc_name_known;
register char *an = objects[obj->otyp].oc_name;
register char *dn = objects[obj->otyp].oc_descr;
register char *un = objects[obj->otyp].oc_uname;

	buf[0] = 0;
	if(!Blind) obj->dknown=1;
	switch(obj->olet) {
	    case AMULET_SYM:
		if(obj->otyp == AMULET_OF_YENDOR) {
		    Strcpy(buf, (obj->spe < 0 && obj->known) ?
			   "cheap plastic imitation of the " : "");
		    Strcat(buf, an);
		} else if (!obj->dknown)
			Strcpy(buf, "amulet");
		else if (nn)
			Strcpy(buf, an);
		else if (un)
			Sprintf(buf,"amulet called %s", un);
		else
			Sprintf(buf,"%s amulet", dn);
		break;
	    case WEAPON_SYM:
		if(obj->otyp <= SHURIKEN && obj->opoisoned)
			Strcpy(buf, "poisoned ");
	    case VENOM_SYM:
	    case TOOL_SYM:
		if(nn)	Strcat(buf, an);
		else	Strcat(buf, dn);
		if(obj->otyp == FIGURINE) {
			Sprintf(eos(buf), " of a%s %s",
				index(vowels, *mons[obj->corpsenm].mname)
								? "n" : "",
				mons[obj->corpsenm].mname);
			break;
		}
		break;
	    case ARMOR_SYM:
		if(obj->otyp==DRAGON_SCALE_MAIL) {
			Sprintf(buf, "%s scale mail",
						mons[obj->corpsenm].mname);
			break;
		}

		if(is_boots(obj) || is_gloves(obj)) Strcpy(buf,"pair of ");

		if(nn)	Strcat(buf, an);
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
	    case FOOD_SYM:
#ifdef TUTTI_FRUTTI
		if (obj->otyp == SLIME_MOLD) {
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
		Strcpy(buf, an);
		if(obj->otyp == TIN && obj->known) {
		    if(obj->spe > 0)
			Strcat(buf, " of spinach");
		    else if (mons[obj->corpsenm].mlet == S_FUNGUS)
			Sprintf(eos(buf), " of %s", mons[obj->corpsenm].mname);
		    else if(obj->corpsenm >= 0)
			Sprintf(eos(buf), " of %s meat", mons[obj->corpsenm].mname);
		    else Strcpy(buf, "empty tin");
		}
		break;
	    case CHAIN_SYM:
		Strcpy(buf, an);
		break;
	    case ROCK_SYM:
		if(obj->otyp == STATUE)
		    Sprintf(buf, "%s of a%s %s", an,
			    (index(vowels, *(mons[obj->corpsenm].mname))) ? "n" : "",
			    mons[obj->corpsenm].mname);
		else Strcpy(buf, an);
		break;
	    case BALL_SYM:
		Sprintf(buf, "%sheavy iron ball",
		  (obj->owt > objects[obj->otyp].oc_weight) ? "very " : "");
		break;
	    case POTION_SYM:
		if(nn || un || !obj->dknown) {
			Strcpy(buf, "potion");
			if(!obj->dknown) break;
			if(nn) {
			    Strcat(buf, " of ");
			    if(obj->otyp == POT_WATER &&
			       objects[POT_WATER].oc_name_known &&
			       (obj->bknown || pl_character[0] == 'P') &&
			       (obj->blessed || obj->cursed)) {
				Strcat(buf, obj->blessed ? "holy " : "unholy ");
			    }
			    Strcat(buf, an);
			} else {
				Strcat(buf, " called ");
				Strcat(buf, un);
			}
		} else {
			Strcpy(buf, dn);
			Strcat(buf, " potion");
		}
		break;
	case SCROLL_SYM:
		Strcpy(buf, "scroll");
		if(!obj->dknown) break;
		if(nn) {
			Strcat(buf, " of ");
			Strcat(buf, an);
		} else if(un) {
			Strcat(buf, " called ");
			Strcat(buf, un);
		} else {
			Strcat(buf, " labeled ");
			Strcat(buf, dn);
		}
		break;
	case WAND_SYM:
		if(!obj->dknown)
			Sprintf(buf, "wand");
		else if(nn)
			Sprintf(buf, "wand of %s", an);
		else if(un)
			Sprintf(buf, "wand called %s", un);
		else
			Sprintf(buf, "%s wand", dn);
		break;
#ifdef SPELLS
	case SPBOOK_SYM:
		if(!obj->dknown)
			Sprintf(buf, "spellbook");
		else if(nn)
			Sprintf(buf, "spellbook of %s", an);
		else if(un)
			Sprintf(buf, "spellbook called %s", un);
		else
			Sprintf(buf, "%s spellbook", dn);
		break;
#endif
	case RING_SYM:
		if(!obj->dknown)
			Sprintf(buf, "ring");
		else if(nn)
			Sprintf(buf, "ring of %s", an);
		else if(un)
			Sprintf(buf, "ring called %s", un);
		else
			Sprintf(buf, "%s ring", dn);
		break;
	case GEM_SYM:
		if(!obj->dknown) {
			if (obj->otyp == ROCK || obj->otyp == LOADSTONE
					|| obj->otyp == LUCKSTONE)
				Strcpy(buf, "stone");
			else
				Strcpy(buf, "gem");
			break;
		}
		if(!nn) {
			char *rock=(obj->otyp==LOADSTONE||obj->otyp==LUCKSTONE)
				? "stone" : "gem";
			if(un)	Sprintf(buf,"%s called %s", rock, un);
			else	Sprintf(buf, "%s %s", dn, rock);
			break;
		}
		Strcpy(buf, an);
		if(obj->otyp >= TURQUOISE && obj->otyp <= JADE)
			Strcat(buf, " stone");
		break;
	default:
		Sprintf(buf,"glorkum %c (0%o) %u %d",
			obj->olet,obj->olet,obj->otyp,obj->spe);
	}
	if(obj->quan != 1) Strcpy(buf, makeplural(buf));

	if(obj->onamelth) {
		Strcat(buf, " named ");
		Strcat(buf, ONAME(obj));
	}
	return(buf);
}

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

	if(obj->quan != 1)
		Sprintf(prefix, "%u ", obj->quan);
	else
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
	    else if (((obj->olet != ARMOR_SYM
			&& obj->olet != WAND_SYM
			&& obj->olet != WEAPON_SYM
			&& ((obj->olet != TOOL_SYM &&
			     obj->olet != RING_SYM) ||
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
			&& obj->otyp != AMULET_OF_YENDOR &&
			pl_character[0] != 'P')
		Strcat(prefix, "uncursed ");
	}
	switch(obj->olet) {
	case AMULET_SYM:
		if(obj->otyp == AMULET_OF_YENDOR)
		    if(strncmp(bp, "cheap ", 6)) {
			Strcpy(tmpbuf, "the ");
			Strcat(tmpbuf, prefix+2); /* skip the "a " */
			Strcpy(prefix, tmpbuf);
		    }
		if(obj->owornmask & W_AMUL)
			Strcat(bp, " (being worn)");
		break;
	case WEAPON_SYM:
		if(ispoisoned)
			Strcat(prefix, "poisoned ");
plus:
		if(obj->known) {
			Strcat(prefix, sitoa(obj->spe));
			Strcat(prefix, " ");
		}
		break;
	case ARMOR_SYM:
		if(obj->owornmask & W_ARMOR)
			Strcat(bp, " (being worn)");
		goto plus;
	case TOOL_SYM:			/* temp. hack by GAN 11/18/86 */
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
		if(obj->otyp == KEY ||
		   (obj->otyp == SKELETON_KEY &&
		    !objects[obj->otyp].oc_name_known)) {
			Strcat(prefix, keystr[obj->spe]);
			Strcat(prefix, " ");
			break;
		}
		if(obj->otyp == LARGE_BOX || obj->otyp == CHEST) {
			Sprintf(eos(bp), " (%s keyhole)", lockstr[obj->spe]);
			break;
		}
		if(obj->otyp == PICK_AXE) goto plus;
		if(!objects[obj->otyp].oc_charged) break;
		/* if special tool, fall through to show charges */
	case WAND_SYM:
		if(obj->known)
			Sprintf(eos(bp), " (%d)", obj->spe);
		break;
	case RING_SYM:
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
	case FOOD_SYM:
		if(OEATEN(obj))
		    Strcat(prefix, "partly eaten ");
		if(obj->otyp == CORPSE) {
		    Strcat(prefix, mons[obj->corpsenm].mname);
		    Strcat(prefix, " ");
		} else if(obj->otyp == EGG && obj->known) {
		    if(obj->corpsenm >= 0) {
			Strcat(prefix, mons[obj->corpsenm].mname);
			Strcat(prefix, " ");
#ifdef POLYSELF
			if (obj->spe)
			    Strcat(bp, " (laid by you)");
#endif
		    }
		}
		break;
	case BALL_SYM:
		if(obj->owornmask & W_BALL)
			Strcat(bp, " (chained to you)");
			break;
	}

	if((obj->owornmask & W_WEP) && !mrg_to_wielded) {
		if (obj->quan != 1)
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
			&& (*(prefix+2) || strncmp(bp, "uranium", 7))) {
		Strcpy(tmpbuf, prefix);
		Strcpy(prefix, "an ");
		Strcpy(prefix+3, tmpbuf+2);
	}
	bp = strprepend(bp, prefix);
	return(bp);
}

/* used only in mthrowu.c (thitu) */
void
setan(str,buf)
register char *str,*buf;
{
	if(index(vowels,*str))
		Sprintf(buf, "an %s", str);
	else
		Sprintf(buf, "a %s", str);
}

char *
aobjnam(otmp,verb) register struct obj *otmp; register char *verb; {
register char *bp = xname(otmp);
char prefix[PREFIX];
	if(otmp->quan != 1) {
		Sprintf(prefix, "%u ", otmp->quan);
		bp = strprepend(bp, prefix);
	}

	if(verb) {
		/* verb is given in plural (without trailing s) */
		Strcat(bp, " ");
		if(otmp->quan != 1)
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

const char *wrp[] = { "wand", "ring", "potion", "scroll", "gem", "amulet",
#ifdef SPELLS
		"spellbook",
#endif
		/* for non-specific wishes */
		"weapon", "armor", "tool", "food", "comestible",
	      };
const char wrpsym[] = {WAND_SYM, RING_SYM, POTION_SYM, SCROLL_SYM, GEM_SYM, AMULET_SYM,
#ifdef SPELLS
		 SPBOOK_SYM,
#endif
		 WEAPON_SYM, ARMOR_SYM, TOOL_SYM, FOOD_SYM, FOOD_SYM
		};

void
lcase(str)
register char *str;
{
	register char *p;
	for (p = str; *p; p++)
		if('A' <= *p && *p <= 'Z') *p += 'a'-'A';
}

/* Plural routine; chiefly used for user-defined fruits.  We have to try to
 * account for everything reasonable the player has; something unreasonable
 * can still break the code.  However, it's still a lot more accurate than
 * "just add an s at the end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi.")
 * and body parts.
 */
char *
makeplural(oldstr)
char *oldstr;
{
	register char *spot;
	static char str[BUFSZ];
	static char *excess;
	int len;

	while (*oldstr==' ') oldstr++;
	if (!oldstr || !*oldstr) {
		impossible("plural of null?");
		return("s");
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
				|| !strncmp(spot, " with", 5)
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
	if (len==1 || !isalpha(*spot)) {
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
		else if (!strncmp(spot-4, "staf", 4))
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
	     (!strcmp(spot-4, "hypha") || !strcmp(spot-4, "larva")))) {
#else
	if (len >= 5 && (!strcmp(spot-4, "hypha"))) {
#endif
		Strcpy(spot, "ae");
		goto bottom;
	}

	/* fungus/fungi, homunculus/homunculi */
	if (!strcmp(spot-1, "us")) {
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
		*(spot--) = (char)0;
		goto bottom;
	}

	/* same singular and plural */
	/* note: also swine, trout, grouse */
	if ((len >= 7 && !strcmp(spot-6, "samurai")) ||
	    (len >= 5 &&
#ifdef TUTTI_FRUTTI
	     (!strcmp(spot-4, "manes") || !strcmp(spot-4, "sheep"))) ||
	    (len >= 4 &&
	     (!strcmp(spot-3, "fish") || !strcmp(spot-3, "tuna") ||
	      !strcmp(spot-3, "deer"))))
#else
	     !strcmp(spot-4, "manes")))
#endif
		goto bottom;

#ifdef TUTTI_FRUTTI
	/* mouse/mice,louse/lice (not a monster, but possible in a food name) */
	if (len >= 5 && !strcmp(spot-3, "ouse") && index("MmLl", *(spot-4))) {
		Strcpy(spot-3, "ice");
		goto bottom;
	}

	/* matzoh/matzot, possible food name */
	if (len >= 6 && !strcmp(spot-5, "matzoh")) {
		*(spot) = 't';
		goto bottom;
	}

	/* child/children (for the wise guys who give their food funny names) */
	if (len >= 5 && !strcmp(spot-4, "child")) {
		Strcpy(spot, "dren");
		goto bottom;
	}

	/* sis/ses (oasis, nemesis) */
	if (len >= 3 && !strcmp(spot-2, "sis")) {
		*(spot-1) = 'e';
		goto bottom;
	}

	/* note: -eau/-eaux (gateau, bordeau...) */
	/* note: ox/oxen, VAX/VAXen, goose/geese */
#endif

	/* Ends in z, x, s, ch, sh; add an "es" */
	if (index("zxsv", *spot) || (*spot=='h' && index("cs", *(spot-1)))
#ifdef TUTTI_FRUTTI
	/* Kludge to get "tomatoes" and "potatoes" right */
				|| (len >= 4 && !strcmp(spot-2, "ato"))
#endif
									) {
		Strcpy(spot+1, "es");
		goto bottom;
	}

	/* Ends in y preceded by consonant (note: also "qu"); change to "ies" */
	if (*spot == 'y' &&
	    (!index(vowels, *(spot-1)))) {
		Strcpy(spot, "ies");
		goto bottom;
	}

	/* Default: append an 's' */
	Strcpy(spot+1, "s");

bottom:	if (excess) Strcpy(str+strlen(str), excess);
	return str;
}

static const char *armor_classes[] = {
	/* "shield called reflection" gives a specific type of shield.
	 * "shield" gives a random type of shield--but not of all armor.
	 */
	"gloves", "boots", "cloak", "shield", "helmet"
};
#define ARMOR_CLASSES 5

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
	int cnt, spe, spesgn, typ, heavy, blessed, uncursed, halfeaten;
	int iscursed, ispoisoned, mntmp, contents;
	int iskey, isnamedbox;
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
	char *un, *dn, *an;
	char *name=0;
#ifdef WIZARD
	int fake=0;
#endif

	cnt = spe = spesgn = typ = heavy = blessed = uncursed = iscursed =
		ispoisoned = halfeaten = iskey = isnamedbox = 0;
	mntmp = -1;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
	contents = UNDEFINED;
	let = 0;
	an = dn = un = 0;
	
	for(;;) {
		if (!bp) goto any;
		if(!strncmp(bp, "an ", 3)) {
			cnt = 1;
			bp += 3;
		} else if(!strncmp(bp, "a ", 2)) {
			cnt = 1;
			bp += 2;
		} else if(!strncmp(bp, "cheap plastic imitation of ", 27)) {
#ifdef WIZARD
			fake = 1;
#endif
			bp += 27;
		} else if(!strncmp(bp, "the ", 4)){
	/*		the = 1; */
			bp += 4;
		} else if(!cnt && digit(*bp)){
			cnt = atoi(bp);
			while(digit(*bp)) bp++;
			while(*bp == ' ') bp++;
		} else if(!strncmp(bp, "partly eaten ", 13)) {
			halfeaten = 1;
			bp += 13;
		} else if(!strncmp(bp,"blessed ",8)) {
			blessed=1;
			bp += 8;
		} else if(!strncmp(bp,"holy ",5)) {
			blessed=1;
			bp += 5;
		} else if(!strncmp(bp,"cursed ",7) || !strncmp(bp,"unholy ",7)){
			iscursed=1;
			bp += 7;
		} else if(!strncmp(bp, "uncursed ",9)) {
			uncursed=1;
			bp += 9;
		} else break;
	}
	if(!cnt) cnt = 1;		/* %% what with "gems" etc. ? */
#ifdef TUTTI_FRUTTI
	Strcpy(fruitbuf, bp);
#endif
	if(!strncmp(bp, "empty ", 6)) {
		contents = EMPTY;
		bp += 6;
	} else if(!strncmp(bp, "poisoned ",9)) {
		ispoisoned=1;
		bp += 9;
#ifdef WIZARD
	} else if(wizard && !strncmp(bp, "trapped ",8)) {
		ispoisoned=1;
		bp += 8;
#endif
	}
	if(*bp == '+' || *bp == '-'){
		spesgn = (*bp++ == '+') ? 1 : -1;
		spe = atoi(bp);
		while(digit(*bp)) bp++;
		while(*bp == ' ') bp++;
	} else {
		p = rindex(bp, '(');
		if(p) {
			if(p > bp && p[-1] == ' ') p[-1] = 0;
			else *p = 0;
			p++;
			if (!(isnamedbox = named_box(p))) {
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
	if (spe > SCHAR_MAX)
		spe = SCHAR_MAX;

	/* now we have the actual name, as delivered by xname, say
		green potions called whisky
		scrolls labeled "QWERTY"
		egg
		fortune cookies
		very heavy iron ball named hoei
		wand of wishing
		elven cloak
	*/
	for(p = bp; *p; p++) if(!strncmp(p, " named ", 7)) {
		*p = 0;
		name = p+7;
	}
	for(p = bp; *p; p++) if(!strncmp(p, " called ", 8)) {
		*p = 0;
		un = p+8;
		/* "helmet called telepathy" is not "helmet" (a specific type)
		 * "shield called reflection" is not "shield" (a general type)
		 */
		for(i=0; i<ARMOR_CLASSES; i++)
		    if(!strncmp(bp,armor_classes[i], strlen(armor_classes[i]))){
			let = ARMOR_SYM;
			goto srch;
		    }
	}
	for(p = bp; *p; p++) if(!strncmp(p, " labeled ", 9)) {
		*p = 0;
		dn = p+9;
	}
	for(p = bp; *p; p++) if(!strncmp(p, " labelled ", 10)) {
		*p = 0;
		dn = p+10;
	}
	for(p = bp; *p; p++) if(!strncmp(p, " of spinach", 11)) {
		*p = 0;
		contents = SPINACH;
	}

	/* Skip over "pair of ", then jump to the singular so we don't
	   try to convert "gloves" or "boots". */
	if(cnt == 1 && !strncmp(bp, "pair of ",8)) {
		bp += 8;
		cnt = 2;
		goto sing;
		/* cnt is ignored for armor and other non-stackable objects;
		   DTRT for stackable objects */
	} else if(cnt > 1 && !strncmp(bp, "pairs of ",9)) {
		bp += 9;
		cnt *= 2;
	}

	/* Find corpse type using "of" (figurine of an orc, tin of orc meat) */
	for(p = bp; *p; p++)
		if (!strncmp(p, " of ", 4) && (mntmp = name_to_mon(p+4)) >= 0) {
			*p = 0;
			break;
	}
	/* Find corpse type w/o "of" (red dragon scale mail, yeti corpse) */
	if (strncmp(bp, "samurai sword", 13)) /* not the "samurai" monster! */
	if (strncmp(bp, "wizard lock", 11)) /* not the "wizard" monster! */
	if (strncmp(bp, "orcish", 6)) /* not the "orc" monster! */
	if (mntmp < 0) if ((mntmp = name_to_mon(bp)) >= 0) {
		bp += strlen(mons[mntmp].mname);
		if (*bp==' ') bp++;
	}

	/* first change to singular if necessary */
	if(cnt != 1) {
		/* find "cloves of garlic", "worthless pieces of blue glass" */
		for(p = bp; *p; p++) 
		    if(!strncmp(p, "s of ", 5)){
			/* but don't singularize "gauntlets" */
			if(strncmp(p-8, "gauntlet", 8))
				while(*p = p[1]) p++;
			goto sing;
		    }
		/* remove -s or -es (boxes) or -ies (rubies) */
		p = eos(bp);
		if(p[-1] == 's') {
			if(p[-2] == 'e') {
				if(p[-3] == 'i') {

					if(!strcmp(p-7, "cookies") ||
					   !strcmp(p-4, "pies"))
						goto mins;
					Strcpy(p-3, "y");
					goto sing;
				}

				/* note: cloves / knives from clove / knife */
				if(!strcmp(p-6, "knives")) {
					Strcpy(p-3, "fe");
					goto sing;
				}

				if(!strcmp(p-6, "staves")) {
					Strcpy(p-3, "ff");
					goto sing;
				}

				/* note: nurses, axes but boxes */
				if(!strcmp(p-5, "boxes")) {
					p[-2] = 0;
					goto sing;
				}
			}
			/* but don't singularize boots or gloves */
			else if(!strcmp(p-5, "boots") ||
				!strcmp(p-6, "gloves"))
					goto sing;
		mins:
			p[-1] = 0;
		} else {
			if(!strcmp(p-5, "teeth")) {
				Strcpy(p-5, "tooth");
				goto sing;
			}
			/* here we cannot find the plural suffix */
		}
	}
sing:
	/* Maybe we need a special strcmp() which ignores capitalization and
	 * dashes/spaces/underscores, so the below 3 special cases would be
	 * unnecessary.
	 */
	/* Alternate spellings (two-handed sword vs. two handed sword) */
	if(!strcmp(bp, "two handed sword")) {
		typ = TWO_HANDED_SWORD;
		goto typfnd;
	}
	/* pick-axe vs. pick axe */
	if(!strcmp(bp, "pick axe")) {
		typ = PICK_AXE;
		goto typfnd;
	}
	if(!strcmp(bp, "luck stone")){
		typ = LUCKSTONE;
		goto typfnd;
	}
	if(!strcmp(bp, "load stone")){
		typ = LOADSTONE;
		goto typfnd;
	}
	/* Alternate capitalizations (Amulet of Yendor, amulet of esp) */
	if(!strcmp(bp, "amulet of Yendor")) {
		typ = AMULET_OF_YENDOR;
		goto typfnd;
	}
	if(!strcmp(bp, "amulet of ESP")) {
		typ = AMULET_OF_ESP;
		goto typfnd;
	}
	if(!strcmp(bp, "ring mail") ||	/* Note: ring mail is not a ring ! */
	   !strcmp(bp, "leather armor") || /* Prevent falling to 'armor'. */
	   !strcmp(bp, "studded leather armor")) {
		let = ARMOR_SYM;
		an = bp;
		goto srch;
	}
	if(!strcmp(bp, "food ration")){
		let = FOOD_SYM;
		an = bp;
		goto srch;
	}
	if((iskey = named_key(bp)) > 0) {
		typ = KEY;
		goto typfnd;
	}
	p = eos(bp);
	if(!strcmp(p-10, "holy water")) {
		typ = POT_WATER;
		if (*(p-12) == 'u') iscursed = 1; /* unholy water */
		else blessed = 1;
		goto typfnd;
	}
#ifdef SHIRT
	if (!strcmp(p-5, "shirt")) {
		typ = HAWAIIAN_SHIRT;
		goto typfnd;
	}
#endif
	if (strlen(bp) == 1 && index(obj_symbols, *bp) && *bp != ILLOBJ_SYM) {
		let = *bp;
		goto any;
	}
	if(strncmp(bp, "enchant ", 8) &&
	   strncmp(bp, "destroy ", 8) &&
	   strncmp(bp, "food detection", 14))
	/* allow wishes for "enchant weapon" and "food detection" */
	for(i = 0; i < sizeof(wrpsym); i++) {
		register int j = strlen(wrp[i]);
		if(!strncmp(bp, wrp[i], j)){
			let = wrpsym[i];
			if(let != AMULET_SYM) {
			    bp += j;
			    if(!strncmp(bp, " of ", 4)) an = bp+4;
			    /* else if(*bp) ?? */
			} else
			    an = bp;
			goto srch;
		}
		if(!strcmp(p-j, wrp[i])){
			let = wrpsym[i];
			p -= j;
			*p = 0;
			if(p[-1] == ' ') p[-1] = 0;
			dn = bp;
			goto srch;
		}
	}
	if(!strcmp(p-6, " stone")){
		p[-6] = 0;
		let = GEM_SYM;
		dn = an = bp;
		goto srch;
	}
	if(!strcmp(p-10, "gold piece") || !strcmp(p-7, "zorkmid") ||
		   !strcmp(bp, "Zorkmid") ||
		   !strcmp(bp, "gold") || !strcmp(bp, "money") || *bp == GOLD_SYM) {
			if (cnt > 5000
#ifdef WIZARD
					&& !wizard
#endif
						) cnt=5000;
		if (cnt < 1) cnt=1;
		pline("%d gold piece%s.", cnt, plur((long)cnt));
		u.ugold += cnt;
		flags.botl=1;
		return (&zeroobj);
	}
#ifdef WIZARD
	/* Let wizards wish for traps --KAA */
	if (wizard) {
		int trap;
		char *tname;

		for (trap = NO_TRAP+1; trap < TRAPNUM; trap++) {
			tname = index(traps[trap], ' ');
			if (tname) {
				if (!strncmp(tname+1, bp, strlen(tname+1))) {
					(void) maketrap(u.ux, u.uy, trap);
					pline("A%s.", traps[trap]);
					if (Invisible) newsym(u.ux,u.uy);
					return(&zeroobj);
				}
			}
		}
	}
#endif
	if(!strcmp(bp, "very heavy iron ball")) {
		heavy = 1;
		typ = HEAVY_IRON_BALL;
		goto typfnd;
	}
	if(!strcmp(bp, "bag")) {
		typ = rnd_class(SACK, BAG_OF_TRICKS);
		goto typfnd;
	}
	if(!strcmp(bp, armor_classes[0])){ /* pair of gloves */
		typ = rnd_class(LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY);
		goto typfnd;
	}
	if(!strcmp(bp, armor_classes[1])){ /* pair of boots */
		typ = rnd_class(LOW_BOOTS, LEVITATION_BOOTS);
		goto typfnd;
	}
	if(!strcmp(bp, armor_classes[2])){ /* cloak */
		typ = rnd_class(MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT);
		goto typfnd;
	}
	if(!strcmp(bp, armor_classes[3])){ /* shield */
		typ = rnd_class(SMALL_SHIELD, SHIELD_OF_REFLECTION);
		goto typfnd;
	}
	/* helmet is not generic */

	an = bp;
	if (!dn) dn = an; /* ex. "black cap" */
srch:
	i = 1;
	if(let) i = bases[letindex(let)];
	while(i <= NROFOBJECTS && (!let || objects[i].oc_olet == let)){
		register char *zn;

		if(an && (zn = objects[i].oc_name) && !strcmp(an, zn)) {
			typ = i;
			goto typfnd;
		}
		if(dn && (zn = objects[i].oc_descr) && !strcmp(dn, zn)) {
			typ = i;
			goto typfnd;
		}
		if(un && (zn = objects[i].oc_uname) && !strcmp(un, zn)) {
			typ = i;
			goto typfnd;
		}
		i++;
	}
#ifdef TUTTI_FRUTTI
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
	if(!let) return((struct obj *)0);
any:
	if(!let) let = wrpsym[rn2(sizeof(wrpsym))];
typfnd:
	if(typ) {
		let = objects[typ].oc_olet;
		otmp = mksobj(typ,FALSE);
	} else {
		otmp = mkobj(let,FALSE);
		typ = otmp->otyp;
	}

	/* venom isn't really an object and can't be wished for; but allow
	 * wizards to wish for it since it's faster than polymorphing and
	 * spitting.
	 */
	if(otmp->olet==VENOM_SYM) {
#ifdef WIZARD
		if (!wizard) {
#endif
			free((genericptr_t) otmp);
			return((struct obj *)0);
#ifdef WIZARD
		} else otmp->spe = 1;
#endif
	}
	if(iskey) otmp->spe = (iskey-1);
	if(isnamedbox && (otmp->otyp==LARGE_BOX || otmp->otyp==CHEST))
		otmp->spe = (isnamedbox-1);

	if(cnt > 0 && objects[typ].oc_merge && 
#ifdef SPELLS
	        let != SPBOOK_SYM &&
#endif
		(cnt < rnd(6) ||
#ifdef WIZARD
		wizard ||
#endif
		 (cnt <= 20 &&
		  (let == WEAPON_SYM && typ <= SHURIKEN) || (typ == ROCK))))
			otmp->quan = cnt;

	if (spesgn == 0) spe = otmp->spe;
#ifdef WIZARD
	else if (wizard) /* no alteration to spe */ ;
#endif
	else if (let == ARMOR_SYM || let == WEAPON_SYM || typ == PICK_AXE ||
			(let==RING_SYM && objects[typ].oc_charged)) {
		if(spe > rnd(5) && spe > otmp->spe) spe = 0;
		if(spe > 2 && u.uluck < 0) spesgn = -1;
	} else {
		if (let == WAND_SYM) {
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
		case SKELETON_KEY: case KEY: case CHEST: case LARGE_BOX:
		case HEAVY_IRON_BALL: case IRON_CHAIN: case STATUE:
			/* otmp->spe already done in mksobj() */
				break;
#ifdef MAIL
		case SCR_MAIL: otmp->spe = 1; break;
#endif
		case AMULET_OF_YENDOR:
#ifdef WIZARD
			if (fake || !wizard)
#endif
				otmp->spe = -1;
#ifdef WIZARD
			else otmp->spe = 0;
#endif
			break;
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

	/* set otmp->corpsenm */
	if (mntmp > -1) switch(typ) {
		case TIN:
			otmp->spe = 0; /* No spinach */
		case CORPSE:
			if (!(mons[mntmp].geno & G_NOCORPSE))
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
		case DRAGON_SCALE_MAIL: /* Not actually possible unless they
				   typed "red dragon dragon scale mail" */
		case SCALE_MAIL:
			if (mntmp >= PM_GRAY_DRAGON &&
			    mntmp <= PM_YELLOW_DRAGON)
				otmp->corpsenm = mntmp;
			if (otmp->corpsenm >= 0)
				otmp->otyp = DRAGON_SCALE_MAIL;
			break;
	}

	/* set blessed/cursed */
	if (iscursed) {
		curse(otmp);
	} else if (uncursed) {
		otmp->blessed = 0;
		otmp->cursed = (u.uluck < 0);
	} else if (blessed) {
		otmp->blessed = (u.uluck >= 0);
		otmp->cursed = (u.uluck < 0);
	} else if (spesgn < 0) {
		curse(otmp);
	}

	/* prevent wishing abuse */
	if (otmp->otyp == WAN_WISHING || otmp->otyp == MAGIC_LAMP)
		otmp->recharged = 1;

	/* set poisoned */
	if (ispoisoned) {
	    if (let == WEAPON_SYM && typ <= SHURIKEN)
		otmp->opoisoned = (u.uluck >= 0);
#ifdef WIZARD
	    else if (Is_box(otmp))
		otmp->otrapped = 1;
	    else if (let == FOOD_SYM)
		/* try to taint by making it as old as possible */
	    	otmp->age = 1L;
#endif
	}

	if (name) otmp = oname(otmp, name, 0);
	otmp->owt = weight(otmp);
	if (heavy) otmp->owt += 15;
	if (halfeaten && otmp->olet == FOOD_SYM) OEATEN(otmp) = 1;
	return(otmp);
}

static int
rnd_class(first,last)
int first,last;
{
	int i, x, sum=0;
	for(i=first; i<=last; i++)
		sum += objects[i].oc_prob;
	x = rnd(sum);
	for(i=first; i<=last; i++)
		if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
			return i;
	return 0;
}
