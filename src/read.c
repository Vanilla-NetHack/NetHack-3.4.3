/*	SCCS Id: @(#)read.c	3.0	88/04/13
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

boolean	known;

static const char readable[] = { '#', SCROLL_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	0 };

static void explode P((struct obj *));
static void do_class_genocide();

int
doread() {
	register struct obj *scroll;
	register boolean confused = (Confusion != 0);

	known = FALSE;
	scroll = getobj(readable, "read");	/* "#-" added by GAN 10/22/86 */
	if(!scroll) return(0);

	/* below added to allow reading of fortune cookies */
	if(scroll->otyp == FORTUNE_COOKIE) {
	    if(flags.verbose)
		You("break up the cookie and throw away the pieces.");
	    outrumor(bcsign(scroll), TRUE);
	    useup(scroll);
	    return(1);
	} else
		if(scroll->olet != SCROLL_SYM
#ifdef SPELLS
		   && scroll->olet != SPBOOK_SYM
#endif
		  ) {
			pline("That is a silly thing to read.");
			return(0);
		}

	if(Blind)
#ifdef SPELLS
	    if (scroll->olet == SPBOOK_SYM) {
		pline("Being blind, you cannot read the mystic runes.");
		return(0);
	    } else
#endif
	    if (!scroll->dknown) {
		pline("Being blind, you cannot read the formula on the scroll.");
		return(0);
	    }
#ifdef SPELLS
	if(scroll->olet == SPBOOK_SYM) {
	    if(confused) {
		You("cannot grasp the meaning of this tome.");
		useup(scroll);
		return(0);
	    } else
		return(study_book(scroll));
	}
#endif
	if(scroll->otyp != SCR_BLANK_PAPER) {
	  if(Blind)
	    pline("As you pronounce the formula on it, the scroll disappears.");
	  else
	    pline("As you read the scroll, it disappears.");
	  if(confused) {
	    if (Hallucination)
		pline("Being so trippy, you screw up...");
	    else
		pline("Being confused, you mispronounce the magic words...");
	  }
	}
	if(!seffects(scroll))  {
		if(!objects[scroll->otyp].oc_name_known) {
		    if(known && !confused) {
			makeknown(scroll->otyp);
			more_experienced(0,10);
		    } else if(!objects[scroll->otyp].oc_uname)
			docall(scroll);
		}
		if(!(scroll->otyp == SCR_BLANK_PAPER) || confused)
			useup(scroll);
	}
	return(1);
}

static void
stripspe(obj)
register struct obj *obj;
{
	if (obj->blessed) pline(nothing_happens);
	else {
		if (obj->spe > 0) {
		    obj->spe = 0;
		    Your("%s vibrates briefly.",xname(obj));
		} else pline(nothing_happens);
	}
}

static void
p_glow1(otmp)
register struct obj	*otmp;
{
	Your("%s %s briefly.", xname(otmp),
		Blind ? "vibrates" : "glows");
}

static void
p_glow2(otmp,color)
register struct obj	*otmp;
register char *color;
{
	Your("%s %s%s for a moment.",
		xname(otmp),
		Blind ? "vibrates" : "glows ",
		Blind ? "" : Hallucination ? hcolor() : color);
}

int
seffects(sobj)
register struct obj	*sobj;
{
	register int cval = 0;
	register boolean confused = (Confusion != 0);

	switch(sobj->otyp) {
#ifdef MAIL
	case SCR_MAIL:
		known = TRUE;
		if (sobj->spe)
		    pline("This seems to be junk mail addressed to the finder of the Eye of Larn.");
		/* note to the puzzled: the game Larn actually sends you junk
		 * mail if you win!
		 */
		else readmail(/* scroll */);
		break;
#endif
	case SCR_ENCHANT_ARMOR:
	    {
		register struct obj *otmp = some_armor();
		register schar s = 0;
		if(!otmp) {
			strange_feeling(sobj,
					!Blind ? "Your skin glows then fades." :
					"Your skin feels warm for a moment.");
			return(1);
		}
		if(confused) {
			if(Blind)
			    Your("%s feels warm for a moment.",
				xname(otmp));
			else
			    Your("%s is covered by a %s %s %s!",
				xname(otmp),
				sobj->cursed ? "mottled" : "shimmering",
				Hallucination ? hcolor() :
				  sobj->cursed ? black : "gold",
				sobj->cursed ? "glow" :
				  (is_shield(otmp) ? "layer" : "shield"));
			if(!(otmp->rustfree))
				otmp->rustfree = !(sobj->cursed);
			break;
		}
#ifdef TOLKIEN
		if((otmp->spe > ((otmp->otyp == ELVEN_MITHRIL_COAT) ? 5 : 3))
#else
		if((otmp->spe > 3)
#endif
				&& rn2(otmp->spe) && !sobj->cursed) {
		Your("%s violently %s%s for a while, then evaporates.",
			    xname(otmp),
			    Blind ? "vibrates" : "glows ",
			    Blind ? "" : Hallucination ? hcolor() : silver);
			if(is_cloak(otmp)) (void) Cloak_off();
			if(is_boots(otmp)) (void) Boots_off();
			if(is_helmet(otmp)) (void) Helmet_off();
			if(is_gloves(otmp)) (void) Gloves_off();
			if(is_shield(otmp)) (void) Shield_off();
			if(otmp == uarm) (void) Armor_gone();
			useup(otmp);
			break;
		}
		s = sobj->blessed ? rnd(3) : sobj->cursed ? -1 : 1;
		Your("%s %s%s for a %s.",
			xname(otmp),
			Blind ? "vibrates" : "glows ",
			Blind ? "" : Hallucination ? hcolor() :
			  sobj->cursed ? black : silver,
			  (s*s>1) ? "while" : "moment");
		otmp->cursed = sobj->cursed;
		if (!otmp->blessed || sobj->cursed)
			otmp->blessed = sobj->blessed;
		otmp->spe += s;
		adj_abon(otmp, s);
		break;
	    }
	case SCR_DESTROY_ARMOR:
	    {   register struct obj *otmp = some_armor();

		if(confused) {
			if(!otmp) {
				strange_feeling(sobj,"Your bones itch.");
				return(1);
			}
			p_glow2(otmp,purple);
			otmp->rustfree = sobj->cursed;
			break;
		}
		if(!sobj->cursed || (sobj->cursed && (!otmp || !otmp->cursed))) {
		    if(!destroy_arm(otmp)) {
			strange_feeling(sobj,"Your skin itches.");
			return(1);
		    }
		} else {	/* armor and scroll both cursed */
		    Your("%s vibrates", xname(otmp));
		    otmp->spe--;
		    make_stunned(HStun + rn1(10, 10), TRUE);
		}
	    }
	    break;
	case SCR_CONFUSE_MONSTER:
#ifdef SPELLS
	case SPE_CONFUSE_MONSTER:
#endif
		if(u.usym != S_HUMAN || sobj->cursed) {
			if(!HConfusion) You("feel confused.");
			make_confused(HConfusion + rnd(100),FALSE);
		} else  if(confused) {
		    if(!sobj->blessed) {
			Your("%s begin to %s%s.",
			    makeplural(body_part(HAND)),
			    Blind ? "tingle" : "glow ",
			    Blind ? "" : Hallucination ? hcolor() : purple);
			make_confused(HConfusion + rnd(100),FALSE);
		    } else {
			pline("A %s%s surrounds your %s.",
			    Blind ? "" : Hallucination ? hcolor() : red,
			    Blind ? "faint buzz" : " glow",
			    body_part(HEAD));
			make_confused(0L,TRUE);
		    }
		} else {
			Your("%s%s %s.",
			makeplural(body_part(HAND)),
			Blind ? "" : " begin to glow",
			Blind ? "tingle" : Hallucination ? hcolor() : red);
			u.umconf = 1;
		}
		break;
	case SCR_SCARE_MONSTER:
#ifdef SPELLS
	case SPE_CAUSE_FEAR:
#endif
	    {	register int ct = 0;
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		    if(cansee(mtmp->mx,mtmp->my)) {
			if(confused || sobj->cursed)
			    mtmp->mflee = mtmp->mfroz = mtmp->msleep = 0;
			else
			    if (! resist(mtmp, sobj->olet, 0, NOTELL))
				mtmp->mflee = 1;
			if(!mtmp->mtame) ct++;	/* pets don't laugh at you */
		    }
		if(!ct)
		      You("hear %s in the distance.",
			       (confused || sobj->cursed) ? "sad wailing" :
							"maniacal laughter");
		else
#ifdef SPELLS
		     if(sobj->otyp == SCR_SCARE_MONSTER)
#endif
			    You("hear %s close by.",
				  (confused || sobj->cursed) ? "sad wailing" :
						 "maniacal laughter");
		break;
	    }
	case SCR_BLANK_PAPER:
		if(confused)
		    You("try to read the strange patterns on this scroll, but it disappears.");
		else  {
		    pline("This scroll seems to be blank.");
		    known = TRUE;
		}
		break;
	case SCR_REMOVE_CURSE:
#ifdef SPELLS
	case SPE_REMOVE_CURSE:
#endif
	    {	register struct obj *obj;
		if(confused)
		    if (Hallucination)
			You("feel the power of the Force against you!");
		    else
			You("feel like you need some help.");
		else
		    if (Hallucination)
			You("feel in touch with the Universal Oneness.");
		    else
			You("feel like someone is helping you.");

		if(sobj->cursed) pline("The scroll disintegrates.");
		else {
		    for(obj = invent; obj ; obj = obj->nobj)
			if(sobj->blessed || obj->owornmask ||
			   (obj->otyp == LOADSTONE)) {
			    if(confused) blessorcurse(obj, 2);
			    else obj->cursed = 0;
			}
		}
		if(Punished && !confused) unpunish();
		break;
	    }
	case SCR_CREATE_MONSTER:
#if defined(WIZARD) || defined(EXPLORE_MODE)
	    if (wizard || discover)
		known = TRUE;
#endif /* WIZARD || EXPLORE_MODE */
#ifdef SPELLS
	case SPE_CREATE_MONSTER:
#endif
	    {	register int cnt = 1;

		if(!rn2(73)) cnt += rnd(4);
		if(confused || sobj->cursed) cnt += 12;
		while(cnt--) {
#if defined(WIZARD) || defined(EXPLORE_MODE)
		    if((!wizard && !discover) || !create_particular())
#endif /* WIZARD || EXPLORE_MODE */
		    (void) makemon (confused ? &mons[PM_ACID_BLOB] :
					(struct permonst *) 0, u.ux, u.uy);
		}
		break;
	    }
/*	    break;	/*NOTREACHED*/
	case SCR_ENCHANT_WEAPON:
		if(uwep && (uwep->olet == WEAPON_SYM || uwep->otyp == PICK_AXE)
							&& confused) {
		/* olet check added 10/25/86 GAN */
			if(Blind)
			    Your("weapon feels warm for a moment.");
			else
			    Your("%s covered by a %s %s %s!",
				aobjnam(uwep, "are"),
				sobj->cursed ? "mottled" : "shimmering",
				Hallucination ? hcolor() :
				  sobj->cursed ? purple : "gold",
				sobj->cursed ? "glow" : "shield");
			uwep->rustfree = !(sobj->cursed);
		} else return !chwepon(sobj, bcsign(sobj)*2+1);
		break;
	case SCR_TAMING:
#ifdef SPELLS
	case SPE_CHARM_MONSTER:
#endif
	    {	register int i,j;
		register int bd = confused ? 5 : 1;
		register struct monst *mtmp;

		for(i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++)
		if(levl[u.ux+i][u.uy+j].mmask && (mtmp = m_at(u.ux+i, u.uy+j))) {
		    if(sobj->cursed) {
			if(!mtmp->mtame) mtmp->mpeaceful = 0;
		    } else {
			if (mtmp->isshk) {
			    if (!mtmp->mpeaceful) {
				kludge("%s calms down.", Monnam(mtmp));
				mtmp->mpeaceful = 1;
			    }
			} else if(!resist(mtmp, sobj->olet, 0, NOTELL))
			    (void) tamedog(mtmp, (struct obj *) 0);
		    }
		}
		break;
	    }
	case SCR_GENOCIDE:
		You("have found a scroll of genocide!");
#ifdef SPELLS
	case SPE_GENOCIDE:
#endif
		known = TRUE;
		if (sobj->blessed) do_class_genocide();
		else do_genocide(!sobj->cursed | (2 * !!Confusion));
		break;
	case SCR_LIGHT:
		if(!Blind) known = TRUE;
		litroom(!confused && !sobj->cursed);
		break;
	case SCR_TELEPORTATION:
		if(confused || sobj->cursed) level_tele();
		else {
			register int uroom = inroom(u.ux, u.uy);

			if (sobj->blessed && !Teleport_control) {
				known = TRUE;
				pline("Do you wish to teleport? ");
				if (yn()=='n') break;
			}
			tele();
			if(uroom != inroom(u.ux, u.uy)) known = TRUE;
			if(Teleport_control) known = TRUE;
		}
		break;
	case SCR_GOLD_DETECTION:
		if (confused || sobj->cursed) return(trap_detect(sobj));
		else return(gold_detect(sobj));
	case SCR_FOOD_DETECTION:
#ifdef SPELLS
	case SPE_DETECT_FOOD:
#endif
		if (food_detect(sobj))
			return(1);	/* nothing detected */
		break;
	case SCR_IDENTIFY:
		/* known = TRUE; */
		if(confused)
			You("identify this as an identify scroll.");
		else
			pline("This is an identify scroll.");
		if (sobj->blessed || (!sobj->cursed && !rn2(5)))
			cval = rn2(5);
			/* Note: if rn2(5)==0, identify all items */
		else	cval = 1;
		useup(sobj);
		makeknown(SCR_IDENTIFY);
#ifdef SPELLS
	case SPE_IDENTIFY:
#endif
		if(!confused)
		    while(invent && !ggetobj("identify", identify, cval));
		return(1);
	case SCR_CHARGING:
	    {	register struct obj *obj;
		register int n;
		if (confused) {
		    You("feel charged up!");
		    break;
		}
		known = TRUE;
		pline("This is a charging scroll.");
		obj = getobj("0#", "charge");
		if (!obj) break;
		if (obj->olet != WAND_SYM) {
		    switch(obj->otyp) {
		    case MAGIC_MARKER:
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    n = obj->spe;
			    if (n < 50) obj->spe = 50;
			    if (n >= 50 && n < 75) obj->spe = 75;
			    if (n >= 75) obj->spe += 10;
			    p_glow2(obj,blue);
			} else {
			    if (obj->spe < 50) obj->spe = 50;
			    else obj->spe++;
			    p_glow2(obj,white);
			}
			break;
		    case LAMP:
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    n = rn2(11);
			    if (obj->spe < n) obj->spe = n;
			    else obj->spe += rnd(3);
			    p_glow2(obj,blue);
			} else {
			    obj->spe++;
			    p_glow1(obj);
			}
			break;
		    case MAGIC_LAMP:
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    if (obj->spe == 1 || obj->recharged)
				pline(nothing_happens);
			    else {
				obj->spe = 1;
				obj->recharged = 1;
				p_glow1(obj);
			    }
			} else {
			    if (obj->spe == 1 || obj->recharged)
				pline(nothing_happens);
			    else {
				n = rn2(2);
				if (!n) {
				    obj->spe = 1;
				    obj->recharged = 1;
				    p_glow1(obj);
				} else pline(nothing_happens);
			    }
			}
			break;
		    case CRYSTAL_BALL:
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    obj->spe = 6;
			    p_glow2(obj,blue);
			} else {
			    if (obj->spe < 5) {
				obj->spe++;
				p_glow1(obj);
			    } else pline(nothing_happens);
			}
			break;
		    case BAG_OF_TRICKS:
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    if (obj->spe <= 10)
				obj->spe += (5 + rnd(10));
			    else obj->spe += (5 + rnd(5));
			    p_glow2(obj,blue);
			} else {
			    obj->spe += rnd(5);
			    p_glow1(obj);
			}
			break;
		    default:
			pline("The scroll %s%s, and disintegrates.",
				Blind ? "vibrates violently" : "glows ",
				Blind ? "" : Hallucination ? hcolor() : "dark red");
		    } /* switch */
		    break;
		}
		else {
		    if (obj->otyp == WAN_WISHING) {
			if (obj->recharged) { /* recharged once already? */
			    explode(obj);
			    break;
			}
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    if (obj->spe != 3) {
				obj->spe = 3;
				p_glow2(obj,blue);
			    } else {
				explode(obj);
				break;
			    }
			} else {
			    if (obj->spe < 3) {
				obj->spe++;
				p_glow2(obj,blue);
			    } else pline(nothing_happens);
			}
			obj->recharged = 1; /* another recharging disallowed */
		    }
		    else {
			if (sobj->cursed) stripspe(obj);
			else if (sobj->blessed) {
			    if (objects[obj->otyp].bits & NODIR) {
				n = rn1(5,11);
				if (obj->spe < n) obj->spe = n;
				else obj->spe++;
			    }
			    else {
				n = rn1(5,4);
				if (obj->spe < n) obj->spe = n;
				else obj->spe++;
			    }
			    p_glow2(obj,blue);
			} else {
			    obj->spe++;
			    p_glow1(obj);
			}
			break;
		    }
		}
	    }
		break;
	case SCR_MAGIC_MAPPING:
		known = TRUE;
		pline("On this scroll %s a map.", confused ? "was" : "is");
#ifdef SPELLS
	case SPE_MAGIC_MAPPING:
#endif
		cval = (sobj->cursed && !confused);
		if(cval) HConfusion = 1;	/* to screw up map */
		do_mapping();
		if(cval) {
		    HConfusion = 0;		/* restore */
		    pline("Unfortunately, it is of a very poor quality.");
		}
		break;
	case SCR_AMNESIA:
	    {	register int zx, zy;

		known = TRUE;
		for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
		    if(!confused || sobj->cursed || rn2(7))
			if(!cansee(zx,zy))
			    levl[zx][zy].seen = levl[zx][zy].new =
				levl[zx][zy].scrsym = 0;
		docrt();
		if (Hallucination) /* Ommmmmm! */
			Your("mind releases itself from mundane concerns.");
		else if (!strncmp(plname, "Maud", 4))
			pline("As your mind turns inward on itself, you forget everything else.");
		else if (rn2(2))
			pline("Who was that Maud person anyway?");
		else
			pline("Thinking of Maud you forget everything else.");
#ifdef SPELLS
		if(!sobj->blessed) losespells();
#endif
		break;
	    }
	case SCR_FIRE:
	    {	register int num;
		register struct monst *mtmp;
/*
 * Note: Modifications have been made as of 3.0 to allow for some damage
 *	 under all potential cases.
 */
		cval = bcsign(sobj);
		useup(sobj);
		makeknown(SCR_FIRE);
		if(confused) {
		    if(Fire_resistance) {
  			shieldeff(u.ux, u.uy);
			if(!Blind)
			    pline("Oh, look, what a pretty fire in your %s.",
				makeplural(body_part(HAND)));
			else You("feal a pleasant warmth in your %s.",
				makeplural(body_part(HAND)));
		    } else {
			pline("The scroll catches fire and you burn your %s.",
				makeplural(body_part(HAND)));
			losehp(1, "scroll of fire");
		    }
		    return(1);
		}
		pline("The scroll erupts in a tower of flame!");
		num = rnd(6) - 3 * cval;
		if(num <= 0 || Fire_resistance) {
			shieldeff(u.ux, u.uy);
			You("are uninjured.");
		} else {
			u.uhpmax -= num;
			losehp(num, "scroll of fire");
		}
		destroy_item(SCROLL_SYM, AD_FIRE);
#ifdef SPELLS
		destroy_item(SPBOOK_SYM, AD_FIRE);
#endif
		destroy_item(POTION_SYM, AD_FIRE);

		num = (2*(rn1(3, 3) + 2 * cval) + 1)/3;
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if(dist(mtmp->mx,mtmp->my) < 3) {
			if (resists_fire(mtmp->data)) continue;
			if (u.uswallow) {
			    if (mtmp != u.ustuck) continue;
			    pline("%s gets heartburn.", Monnam(u.ustuck));
			    num *= 2;
			}
			mtmp->mhp -= num;		/* No saving throw! */
			if(resists_cold(mtmp->data))
			    mtmp->mhp -= 3*num;
			if(mtmp->mhp < 1) {
			    killed(mtmp);
			    break;		/* primitive */
			}
		    }
		}
		return(1);
	    }
	case SCR_PUNISHMENT:
		known = TRUE;
		if(confused || sobj->blessed) {
			You("feel guilty.");
			break;
		}
		punish(sobj);
		break;
	default:
		impossible("What weird effect is this? (%u)", sobj->otyp);
	}
	return(0);
}

static void
explode(obj)
register struct obj *obj;
{
    Your("%s vibrates violently, and explodes!",xname(obj));
    bell();
    losehp(rn2(2*(u.uhpmax+1)/3),"exploding wand");
    useup(obj);
}

void
litroom(on)
register boolean on;
{
	register int zx,zy;

	/* first produce the text (provided he is not blind) */
	if(Blind) goto do_it;
	if(!on) {
		if(u.uswallow || is_maze_lev || levl[u.ux][u.uy].typ == CORR ||
		    !levl[u.ux][u.uy].lit) {
			pline("It seems even darker in here than before.");
			return;
		} else
			pline("It suddenly becomes dark in here.");
	} else {
		if(u.uswallow){
			pline("%s's stomach is lit.", Monnam(u.ustuck));
			return;
		}
		if(is_maze_lev){
			pline(nothing_happens);
			return;
		}
		if(levl[u.ux][u.uy].typ == CORR) {
		    pline("The corridor lights up around you, then fades.");
		    return;
		} else if(levl[u.ux][u.uy].lit) {
		    pline("The light here seems better now.");
		    return;
		} else
		    pline("The room is lit.");
	}

do_it:
	if(levl[u.ux][u.uy].lit == on)
		return;
	if (inroom(u.ux,u.uy) < 0)
		return;
	getcorners(&seelx,&seehx,&seely,&seehy,&seelx2,&seehx2,&seely2,&seehy2);

	for(zy = seely; zy <= seehy; zy++)
		for(zx = seelx; zx <= seehx; zx++) {
			levl[zx][zy].lit = on;
			if(!Blind && dist(zx,zy) > 2)
				if(on) prl(zx,zy); else nosee(zx,zy);
		}
	for(zy = seely2; zy <= seehy2; zy++)
		for(zx = seelx2; zx <= seehx2; zx++) {
			levl[zx][zy].lit = on;
			if(!Blind && dist(zx,zy) > 2)
				if(on) prl(zx,zy); else nosee(zx,zy);
		}
	if(!on) seehx = 0;
}

static void
do_class_genocide()
{
	register int i, j, immunecnt, gonecnt, goodcnt;
	char buf[BUFSZ];

	for(j=0; ; j++) {
		if (j >= 5) {
			pline(thats_enough_tries);
			return;
		}
    pline("What class of monsters do you wish to genocide? [type a letter] ");
		do {
			getlin(buf);
		} while (buf[0]=='\033' || strlen(buf) != 1);
		immunecnt = gonecnt = goodcnt = 0;
		for(i=0; mons[i].mlet; i++) {
			if(mons[i].mlet == buf[0]) {
				if (!(mons[i].geno & G_GENO)) immunecnt++;
				else if(mons[i].geno & G_GENOD) gonecnt++;
				else goodcnt++;
			}
		}
		if (!goodcnt && buf[0] != S_HUMAN) {
			if (gonecnt)
	pline("All such monsters are already nonexistent.");
			else if (immunecnt)
	You("aren't permitted to genocide such monsters.");
			else
	pline("That symbol does not represent any monster.");
			continue;
		}
		for(i=0; mons[i].mlet; i++) {
		    if(mons[i].mlet == buf[0]) {
			register struct monst *mtmp, *mtmp2;
			char *n = makeplural(mons[i].mname);

			if (&mons[i]==player_mon() || ((mons[i].geno & G_GENO)
				&& !(mons[i].geno & G_GENOD))) {
			/* This check must be first since player monsters might
			 * have G_GENOD or !G_GENO.
			 */
			    pline("Wiped out all %s.", n);
			    if (&mons[i] == player_mon()) {
				u.uhp = -1;
				killer = "scroll of genocide";
#ifdef POLYSELF
				if (u.umonnum >= 0)
				    You("feel dead inside.");
				else
#endif
				    done(GENOCIDED);
			    }
			    /* for simplicity (and fairness) let's avoid
			     * alignment changes here...
			     */
#ifdef POLYSELF
			    if (i==u.umonnum) rehumanize();
#endif
			    mons[i].geno |= G_GENOD;
			    for(mtmp = fmon; mtmp; mtmp = mtmp2) {
				mtmp2 = mtmp->nmon;
				if(mtmp->data == &mons[i])
				    mondead(mtmp);
			    }
			} else if (mons[i].geno & G_GENOD)
			    pline("All %s are already nonexistent.", n);
			else
			    You("aren't permitted to genocide %s%s.",
				i == PM_WIZARD_OF_YENDOR ? "the " : "",
				type_is_pname(&mons[i]) ? mons[i].mname : n);
			}
		}
		return;
	}
}
		
#define REALLY 1
#define PLAYER 2
void
do_genocide(how)
int how;
/* 0 = no genocide; create monsters (cursed scroll) */
/* 1 = normal genocide */
/* 3 = forced genocide of player */
{
	char buf[BUFSZ];
	register int	i, j, killplayer = 0;
	register struct permonst *ptr;
	register struct monst *mtmp, *mtmp2;

	if (how & PLAYER) {
		ptr = player_mon();
		Strcpy(buf, ptr->mname);
		killplayer++;
	} else {
	    for(j = 0; ; j++) {
		if(j >= 5) {
		    pline(thats_enough_tries);
		    return;
		}
		pline("What monster do you want to genocide? [type the name] ");
		getlin(buf);

		if(strlen(buf) && (!strncmp(buf, pl_character, PL_CSIZ))) {
	/* Note: pl_character starts with capitals and player_mon does not */
		    ptr = player_mon();
		    killplayer++;
		    goto deadmeat;
		} else {
		    i = name_to_mon(buf);
		    if(i == -1 || (mons[i].geno & G_GENOD)) {
			pline("Such creatures do not exist in this world.");
			continue;
		    }
		    ptr = &mons[i];
		    if (ptr == player_mon()) {
			killplayer++;
			goto deadmeat;
		    }
		    if (is_human(ptr)) adjalign(-sgn(u.ualigntyp));
		    if (is_demon(ptr)) adjalign(sgn(u.ualigntyp));

		    if(!(ptr->geno & G_GENO))  {
			if(flags.soundok) {
			    if(flags.verbose)
			pline("A thunderous voice booms though the caverns:");
			    pline("\"No, mortal!  That will not be done.\"");
			}
			continue;
		    }
		    break;
		}
	    }
	}
deadmeat:
	if (Hallucination) {
#ifdef POLYSELF
	    if (u.umonnum != -1)
		Strcpy(buf,uasmon->mname);
	    else
#endif
	    {
		Strcpy(buf, pl_character);
		buf[0] += 'a' - 'A';
	    }
	} else Strcpy(buf,ptr->mname); /* make sure we have standard singular */
	if (how & REALLY) {
	    pline("Wiped out all %s.", makeplural(buf));
	    if(killplayer) {
		u.uhp = -1;
		killer = "genocide spell";
#ifdef POLYSELF
	/* A polymorphed character will die as soon as he is rehumanized. */
		if(u.umonnum >= 0)	You("feel dead inside.");
		else
#endif
			done(GENOCIDED);
		return;
	    }
#ifdef POLYSELF
	    else if ((how & REALLY) && ptr == &mons[u.umonnum]) rehumanize();
#endif
	    ptr->geno |= G_GENOD;
	    for(mtmp = fmon; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if(mtmp->data == ptr)
		    mondead(mtmp);
	    }
	} else {
	    pline("Sent in some %s.", makeplural(buf));
	    j = rnd(3) + 3;
	    for(i=1; i<=j; i++)
		(void) makemon(ptr, u.ux, u.uy);
	}
}

static void
show_map_spot(x, y)
register int x, y;
{
	register struct rm *lev;
	register int num;

	if((Confusion != 0) && rn2(7)) return;
	lev = &(levl[x][y]);
	if((num = lev->typ) == 0) return;

	if(num == SCORR) {
		lev->typ = CORR;
		lev->scrsym = CORR_SYM;
	/*
	 * magic mapping shouldn't find secret doors,
	 * especially on the stronghold level
	 */
	} else if(lev->seen) return;
	if(num != ROOM)
	{
		lev->seen = lev->new = 1;
		if(lev->scrsym == STONE_SYM || !lev->scrsym)
			newsym(x, y);
		else	on_scr(x, y);
	}
}

void
do_mapping() {
	register int zx, zy;

	for(zy = 0; zy < ROWNO; zy++)
	    for(zx = 0; zx < COLNO; zx++)
		show_map_spot(zx, zy);
}

void
do_vicinity_map() {
	register int zx, zy;
	
	for(zy = (u.uy-5 < 0 ? 0 : u.uy-5); 
			zy < (u.uy+6 > ROWNO ? ROWNO : u.uy+6); zy++)
	    for(zx = (u.ux-9 < 0 ? 0 : u.ux-9); 
			zx < (u.ux+10 > COLNO ? COLNO : u.ux+10); zx++)
		show_map_spot(zx, zy);
}

int
gold_detect(sobj)
register struct obj	*sobj;
{
	register struct gold *gtmp;

	if(!fgold) {
		if(sobj)
		    strange_feeling(sobj, "You feel materially poor.");
		return(1);
	} else {
		known = TRUE;
		for(gtmp = fgold; gtmp; gtmp = gtmp->ngold)
			if(gtmp->gx != u.ux || gtmp->gy != u.uy)
				goto outgoldmap;
		/* only under me - no separate display required */
		You("notice some gold between your %s.",
			makeplural(body_part(FOOT)));
		return(0);
	outgoldmap:
		cls();
		for(gtmp = fgold; gtmp; gtmp = gtmp->ngold)
		    at( gtmp->gx, gtmp->gy,
			(uchar)(Hallucination ? rndobjsym() : GOLD_SYM),
			AT_OBJ);
		prme();
		You("feel very greedy, and sense gold!");
		more();
		docrt();
	}
	return(0);
}

/* food_detection is pulled out so that it 	*/
/* can also be used in the crystal ball routine	*/
/* returns 1 if nothing was detected		*/
/* returns 0 if something was detected		*/
int
food_detect(sobj)
register struct obj	*sobj;
{
	register boolean confused = (Confusion || (sobj && sobj->cursed));
	register int ct = 0, ctu = 0;
	register struct obj *obj;
	register char foodsym = confused ? POTION_SYM : FOOD_SYM;

	for(obj = fobj; obj; obj = obj->nobj)
		if(obj->olet == foodsym) {
			if(obj->ox == u.ux && obj->oy == u.uy) ctu++;
			else ct++;
		}
	if(!ct && !ctu) {
		if (sobj) strange_feeling(sobj,"Your nose twitches.");
		return(1);
	} else if(!ct) {
		known = TRUE;
		You("%s %s close nearby.", sobj ? "smell" : "sense",
			confused ? "something" : "food");
	} else {
		known = TRUE;
		cls();
		for(obj = fobj; obj; obj = obj->nobj)
		    if(obj->olet == foodsym)
			at(obj->ox, obj->oy,
			   (uchar)(Hallucination ? rndobjsym() : FOOD_SYM),
			   AT_OBJ);
		prme();
		if (sobj) Your("nose tingles and you smell %s.",
				confused ? "something" : "food");
		else You("sense %s.", confused ? "something" : "food");
		more();
		docrt();
	}
	return(0);
}

void
punish(sobj)
register struct obj	*sobj;
{
	You("are being punished for your misbehavior!");
	if(Punished){
		Your("iron ball gets heavier.");
		uball->owt += 15 * (1 + sobj->cursed);
		return;
	}
	setworn(mkobj_at(CHAIN_SYM, u.ux, u.uy), W_CHAIN);
	setworn(mkobj_at(BALL_SYM, u.ux, u.uy), W_BALL);
	uball->spe = 1;		/* special ball (see save) */
}

void
unpunish()
{	    /* remove the ball and chain */
	freeobj(uchain);
	unpobj(uchain);
	free((genericptr_t) uchain);
	setworn((struct obj *)0, W_CHAIN);
	uball->spe = 0;
	setworn((struct obj *)0, W_BALL);
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to revive
 * one, the disoriented creature becomes a zombie
 */
boolean
cant_create(mtype)
int *mtype;
{

	if (*mtype==PM_GUARD || *mtype==PM_SHOPKEEPER
#if defined(ALTARS) && defined(THEOLOGY)
	     || *mtype==PM_TEMPLE_PRIEST || *mtype==PM_TEMPLE_PRIESTESS
#endif
								) {
		*mtype = PM_HUMAN_ZOMBIE;
		return TRUE;
	} else
		return FALSE;
}

#if defined(WIZARD) || defined(EXPLORE_MODE)
boolean
create_particular()
{
	char buf[BUFSZ];
	int which;

	do {
	    pline("Create what kind of monster? [type the name] ");
	    getlin(buf);
	} while(strlen(buf) < 1);
	which = name_to_mon(buf);
	if (which != -1) {
	    if (!(mons[which].geno & G_GENOD) && cant_create(&which) &&
								!Blind) {
		if (mons[which].geno & G_GENOD)
pline("An image of the creature forms, wavers momentarily, then fades.");
		else
pline("The disoriented creature's eyes slowly glaze over.");
	    }
	    (void) makemon(&mons[which], u.ux, u.uy);
	    return TRUE;
	}
	return FALSE;
}
#endif /* WIZARD || EXPLORE_MODE */
