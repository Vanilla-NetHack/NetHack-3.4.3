/*	SCCS Id: @(#)read.c	2.3	88/01/21
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include "hack.h"

extern struct monst *makemon();
extern struct permonst pm_eel;
extern struct obj *mkobj_at();
char *hcolor();
boolean	known; 
int identify();

doread() {
	register struct obj *scroll;
	register boolean confused = (Confusion != 0);

	known = FALSE;
	scroll = getobj("#-?", "read");	/*  "#-" added by GAN 10/22/86 */
	if(!scroll) return(0);
	
	/* below added to allow reading of fortune cookies */
	if(scroll->otyp == FORTUNE_COOKIE) {
		if(Blind) {
			pline("This cookie has a scrap of paper inside!");
			pline("What a pity, that you cannot read it!");
		} else
			outrumor();
		useup(scroll);
		return(1);
	}  else
		if(scroll->olet != SCROLL_SYM) {
			pline("That is a silly thing to read.");
			return(0);
		}

	if(!scroll->dknown && Blind) {
	    pline("Being blind, you cannot read the formula on the scroll.");
	    return(0);
	}
	if(Blind)
	  pline("As you pronounce the formula on it, the scroll disappears.");
	else
	  pline("As you read the scroll, it disappears.");
	if(confused) {
	  if (Hallucination)
	      pline("Being so trippy, you screw up ... ");
	  else
	      pline("Being confused, you mispronounce the magic words ... ");
	}
	if(!seffects(scroll))  {
		if(!objects[scroll->otyp].oc_name_known) {
		    if(known && !confused) {
			objects[scroll->otyp].oc_name_known = 1;
			more_experienced(0,10);
		    } else if(!objects[scroll->otyp].oc_uname)
			docall(scroll);
		}
#ifdef MARKER
		if(!(scroll->otyp == SCR_BLANK_PAPER) || confused)
#endif
			useup(scroll);
	}
	return(1);
}

seffects(sobj)
	register struct obj	*sobj;
{
	extern struct obj *some_armor();
	register boolean confused = (Confusion != 0);

	switch(sobj->otyp) {
#ifdef MAIL
	case SCR_MAIL:
		readmail(/* scroll */);
		break;
#endif
	case SCR_ENCHANT_ARMOR:
	    {
		register struct obj *otmp = some_armor();
		if(!otmp) {
			strange_feeling(sobj,"Your skin glows then fades.");
			return(1);
		}
		if(confused) {
			pline("Your %s is covered by a shimmering %s %s!",
				objects[otmp->otyp].oc_name, Hallucination ? hcolor() :
				"gold", (otmp->otyp == SHIELD ? "layer" : "shield"));
			otmp->rustfree = 1;
			break;
		}
#ifdef KAA
		if(otmp->spe > (otmp->otyp == ELFIN_CHAIN_MAIL ? 5 : 3)
				&& rn2(otmp->spe)) {
#else
		if(otmp->spe > 3 && rn2(otmp->spe)) {
#endif
		pline("Your %s glows violently %s for a while, then evaporates.",
			objects[otmp->otyp].oc_name,
			Hallucination ? hcolor() : "green");
			useup(otmp);
			break;
		}
		pline("Your %s glows %s for a moment.",
			objects[otmp->otyp].oc_name,
			Hallucination ? hcolor() : "green");
		otmp->cursed = 0;
		otmp->spe++;
		break;
	    }
	case SCR_DESTROY_ARMOR:
		if(confused) {
			register struct obj *otmp = some_armor();
			if(!otmp) {
				strange_feeling(sobj,"Your bones itch.");
				return(1);
			}
			pline("Your %s glows %s for a moment.",
				objects[otmp->otyp].oc_name,
				Hallucination ? hcolor() : "purple");
			otmp->rustfree = 0;
			break;
		}
		if(!destroy_arm()) {
			strange_feeling(sobj,"Your skin itches.");
			return(1);
		}
		break;
	case SCR_CONFUSE_MONSTER:
#ifdef SPELLS
	case SPE_CONFUSE_MONSTER:
#endif
		if(u.usym != '@') {
			pline("You feel confused.");
			HConfusion += rnd(100);
		} else  if(confused) {
			pline("Your hands begin to glow %s.",
			Hallucination ? hcolor() : "purple");
			HConfusion += rnd(100);
		} else {
			pline("Your hands begin to glow %s.",
			Hallucination ? hcolor() : "blue");
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
			if(confused)
			    mtmp->mflee = mtmp->mfroz = mtmp->msleep = 0;
			else
			    if (! resist(mtmp, sobj->olet, 0, NOTELL))
				mtmp->mflee = 1;
			ct++;
		    }
		if(!ct)
		    pline("You hear %s in the distance.",
			  (confused) ? "sad wailing" : "maniacal laughter");
#ifdef KAA
# ifdef SPELLS
		    else if(sobj->otyp == SCR_SCARE_MONSTER)
# endif
			pline ("You hear %s close by.",
			       (confused) ? "sad wailing" : "maniacal laughter");
#endif
		break;
	    }
	case SCR_BLANK_PAPER:
		if(confused)
		    pline("You see strange patterns on this scroll.");
		else  {
		    pline("This scroll seems to be blank.");
#ifdef MARKER
		    pline("No, wait...");
		    known = TRUE;
#endif
		}
		break;
	case SCR_REMOVE_CURSE:
#ifdef SPELLS
	case SPE_REMOVE_CURSE:
#endif
	    {	register struct obj *obj;
		if(confused)
		    if (Hallucination)
			pline("You feel the power of the Force against you!");
		    else
			pline("You feel like you need some help.");
		else
		    if (Hallucination)
			pline("You feel in touch with the Universal Oneness.");
		    else
			pline("You feel like someone is helping you.");
		for(obj = invent; obj ; obj = obj->nobj)
			if(obj->owornmask)
				obj->cursed = confused;
		if(Punished && !confused) {
			Punished = 0;
			freeobj(uchain);
			unpobj(uchain);
			free((char *) uchain);
			uball->spe = 0;
			uball->owornmask &= ~W_BALL;
			uchain = uball = (struct obj *) 0;
		}
		break;
	    }
	case SCR_CREATE_MONSTER:
#ifdef SPELLS
	case SPE_CREATE_MONSTER:
#endif
	    {	register int cnt = 1;

		if(!rn2(73)) cnt += rnd(4);
		if(confused) cnt += 12;
		while(cnt--)
#ifdef WIZARD
			if(wizard)  {
				char buf[BUFSZ], cmlet;
				struct permonst *crmonst;
				
				do {
					pline("What monster to create? ");
					getlin(buf);
				} while(strlen(buf) != 1);
				cmlet = buf[0];
				for(crmonst = mons; crmonst->mlet != cmlet &&
					crmonst != PM_EEL; crmonst++) ;
				(void) makemon(crmonst, u.ux, u.uy);
			} else
#endif /* WIZARD /**/
				(void) makemon(confused ? PM_ACID_BLOB :
					(struct permonst *) 0, u.ux, u.uy);
		break;
	    }
	case SCR_ENCHANT_WEAPON:
		if(uwep && uwep->olet == WEAPON_SYM && confused) {
		/* olet check added 10/25/86 GAN */
			pline("Your %s covered by a shimmering %s shield!",
				aobjnam(uwep, "are"),
				Hallucination ? hcolor() : "gold");
			uwep->rustfree = 1;
		} else
			if(!chwepon(sobj, 1))		/* tests for !uwep */
				return(1);
		break;
	case SCR_DAMAGE_WEAPON:
		if(uwep && uwep->olet == WEAPON_SYM && confused) {
		/* olet check added 10/25/86 GAN */
			pline("Your %s %s for a moment.",
				aobjnam(uwep,"glow"),
				Hallucination ? hcolor() : "purple");
			uwep->rustfree = 0;
		} else
			if(!chwepon(sobj, -1))	/* tests for !uwep */
				return(1);
		break;
	case SCR_TAMING:
#ifdef SPELLS
	case SPE_CHARM_MONSTER:
#endif
	    {	register int i,j;
		register int bd = confused ? 5 : 1;
		register struct monst *mtmp;

		for(i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++)
		if(mtmp = m_at(u.ux+i, u.uy+j))
		    if(!resist(mtmp, sobj->olet, 0, NOTELL))
			(void) tamedog(mtmp, (struct obj *) 0);
		break;
	    }
	case SCR_GENOCIDE:
		pline("You have found a scroll of genocide!");
#ifdef SPELLS
	case SPE_GENOCIDE:
#endif
		known = TRUE;
		do_genocide();
		break;
	case SCR_LIGHT:
		if(!Blind) known = TRUE;
		litroom(!confused);
		break;
	case SCR_TELEPORTATION:
		if(confused)
			level_tele();
		else {
#ifdef QUEST
			register int oux = u.ux, ouy = u.uy;
			tele();
			if(dist(oux, ouy) > 100) known = TRUE;
#else
			register int uroom = inroom(u.ux, u.uy);
			tele();
			if(uroom != inroom(u.ux, u.uy)) known = TRUE;
#endif
			if(Teleport_control)
				known = TRUE;
		}
		break;
	case SCR_GOLD_DETECTION:
	    /* Unfortunately this code has become slightly less elegant,
	       now that gold and traps no longer are of the same type. */
	    if(confused) {
		register struct trap *ttmp;

		if(!ftrap) {
			strange_feeling(sobj, "Your toes stop itching.");
			return(1);
		} else {
			for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
				if(ttmp->tx != u.ux || ttmp->ty != u.uy)
					goto outtrapmap;
			/* only under me - no separate display required */
			pline("Your toes itch!");
			break;
		outtrapmap:
			cls();
			for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
				at(ttmp->tx, ttmp->ty, Hallucination ? rndobjsym() : GOLD_SYM);
			prme();
			pline("You feel very greedy!");
		}
	    } else {
		register struct gold *gtmp;

		if(!fgold) {
			strange_feeling(sobj, "You feel materially poor.");
			return(1);
		} else {
			known = TRUE;
			for(gtmp = fgold; gtmp; gtmp = gtmp->ngold)
				if(gtmp->gx != u.ux || gtmp->gy != u.uy)
					goto outgoldmap;
			/* only under me - no separate display required */
			pline("You notice some gold between your feet.");
			break;
		outgoldmap:
			cls();
			for(gtmp = fgold; gtmp; gtmp = gtmp->ngold)
				at(gtmp->gx, gtmp->gy, Hallucination ? rndobjsym() : GOLD_SYM);
			prme();
			pline("You feel very greedy, and sense gold!");
		}
	    }
		/* common sequel */
		more();
		docrt();
		break;
	case SCR_FOOD_DETECTION:
#ifdef SPELLS
	case SPE_DETECT_FOOD:
#endif
	    {	register ct = 0, ctu = 0;
		register struct obj *obj;
		register char foodsym = confused ? POTION_SYM : FOOD_SYM;

		for(obj = fobj; obj; obj = obj->nobj)
			if(obj->olet == foodsym) {
				if(obj->ox == u.ux && obj->oy == u.uy) ctu++;
				else ct++;
			}
		if(!ct && !ctu) {
			strange_feeling(sobj,"Your nose twitches.");
			return(1);
		} else if(!ct) {
			known = TRUE;
			pline("You smell %s close nearby.",
				confused ? "something" : "food");
			
		} else {
			known = TRUE;
			cls();
			for(obj = fobj; obj; obj = obj->nobj)
			    if(obj->olet == foodsym)
				at(obj->ox, obj->oy, Hallucination ? rndobjsym() :
				 FOOD_SYM);
			prme();
			pline("Your nose tingles and you smell %s!",
				confused ? "something" : "food");
			more();
			docrt();
		}
		break;
	    }
	case SCR_IDENTIFY:
		/* known = TRUE; */
		if(confused)
			pline("You identify this as an identify scroll.");
		else
			pline("This is an identify scroll.");
		useup(sobj);
		objects[SCR_IDENTIFY].oc_name_known = 1;
#ifdef SPELLS
	case SPE_IDENTIFY:
#endif
		if(!confused)
		    while(!ggetobj("identify", identify, rn2(5) ? 1 : rn2(5)) && invent);
		return(1);
	case SCR_MAGIC_MAPPING:
		known = TRUE;
		pline("On this scroll %s a map!", confused ? "was" : "is");
#ifdef SPELLS
	case SPE_MAGIC_MAPPING:
#endif
		do_mapping();
		break;
	case SCR_AMNESIA:
	    {	register int zx, zy;

		known = TRUE;
		for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
		    if(!confused || rn2(7))
			if(!cansee(zx,zy))
			    levl[zx][zy].seen = 0;
		docrt();
		pline("Who was that Maude person anyway?");
#ifdef SPELLS
		losespells();
#endif
		break;
	    }
	case SCR_FIRE:
	    {	register int num;
		register struct monst *mtmp;

/* 
 * Note: This case was modified 11/4/86 by DKC to eliminate the problem with
 * reading a scroll of fire while confused or resistant to fire.  Formerly,
 * the code failed to initialize the variable "num" in these cases, resulting
 * in monsters being hit for a possibly large (and possibly negative) damage.
 * The actions taken now are: 
 * 				If the player is fire resistant, monsters
 * take the normal damage (1-6 except for Y's and F's), and the player is
 * unaffected.
 */
		known = TRUE;
		if(confused) {
		    if(Fire_resistance)
			pline("Oh look, what a pretty fire in your hands.");
		    else {
			pline("The scroll catches fire and you burn your hands.");
			losehp(1, "scroll of fire");
		    }
		    break;
		}
		pline("The scroll erupts in a tower of flame!");
		num = rnd(6);
		if(Fire_resistance)
			pline("You are uninjured.");
		else {
			u.uhpmax -= num;
			losehp(num, "scroll of fire");
		}
		num = (2*num + 1)/3;
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if(dist(mtmp->mx,mtmp->my) < 3) {
			mtmp->mhp -= num;		/* No saving throw! */
			if(index("FY", mtmp->data->mlet))
			    mtmp->mhp -= 3*num;	/* this might well kill 'F's */
			if(mtmp->mhp < 1) {
			    killed(mtmp);
			    break;		/* primitive */
			}
		    }
		}
		break;
	    }
	case SCR_PUNISHMENT:
		known = TRUE;
		if(confused) {
			pline("You feel guilty.");
			break;
		}
		pline("You are being punished for your misbehavior!");
		if(Punished){
			pline("Your iron ball gets heavier.");
			uball->owt += 15;
			break;
		}
		Punished = INTRINSIC;
		setworn(mkobj_at(CHAIN_SYM, u.ux, u.uy), W_CHAIN);
		setworn(mkobj_at(BALL_SYM, u.ux, u.uy), W_BALL);
		uball->spe = 1;		/* special ball (see save) */
		break;
	default:
		impossible("What weird effect is this? (%u)", sobj->otyp);
	}
	return(0);
}

identify(otmp)		/* also called by newmail() */
register struct obj *otmp;
{
	objects[otmp->otyp].oc_name_known = 1;
#ifdef KAA
	otmp->known = 1;
	if (otmp->olet != WEAPON_SYM) otmp->dknown = 1;
/* Now, the dknown field is special for weapons, indicating blessing. */
#else
	otmp->known = otmp->dknown = 1;
#endif
	prinv(otmp);
	return(1);
}

litroom(on)
register boolean on;
{
	register num,zx,zy;

	/* first produce the text (provided he is not blind) */
	if(Blind) goto do_it;
	if(!on) {
		if(u.uswallow || !xdnstair || levl[u.ux][u.uy].typ == CORR ||
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
		if(!xdnstair){
			pline("Nothing Happens.");
			return;
		}
#ifdef QUEST
		pline("The cave lights up around you, then fades.");
		return;
#else
		if(levl[u.ux][u.uy].typ == CORR) {
		    pline("The corridor lights up around you, then fades.");
		    return;
		} else if(levl[u.ux][u.uy].lit) {
		    pline("The light here seems better now.");
		    return;
		} else
		    pline("The room is lit.");
#endif
	}

do_it:
#ifdef QUEST
	return;
#else
	if(levl[u.ux][u.uy].lit == on)
		return;
	if(levl[u.ux][u.uy].typ == DOOR) {
		if(IS_ROOM(levl[u.ux][u.uy+1].typ)) zy = u.uy+1;
		else if(IS_ROOM(levl[u.ux][u.uy-1].typ)) zy = u.uy-1;
		else zy = u.uy;
		if(IS_ROOM(levl[u.ux+1][u.uy].typ)) zx = u.ux+1;
		else if(IS_ROOM(levl[u.ux-1][u.uy].typ)) zx = u.ux-1;
		else zx = u.ux;
	} else {
		zx = u.ux;
		zy = u.uy;
	}
	for(seelx = u.ux; (num = levl[seelx-1][zy].typ) != CORR && num != 0;
		seelx--);
	for(seehx = u.ux; (num = levl[seehx+1][zy].typ) != CORR && num != 0;
		seehx++);
	for(seely = u.uy; (num = levl[zx][seely-1].typ) != CORR && num != 0;
		seely--);
	for(seehy = u.uy; (num = levl[zx][seehy+1].typ) != CORR && num != 0;
		seehy++);
	for(zy = seely; zy <= seehy; zy++)
		for(zx = seelx; zx <= seehx; zx++) {
			levl[zx][zy].lit = on;
			if(!Blind && dist(zx,zy) > 2)
				if(on) prl(zx,zy); else nosee(zx,zy);
		}
	if(!on) seehx = 0;
#endif
}

/* Test whether we may genocide all monsters with symbol  ch  */
monstersym(ch)				/* arnold@ucsfcgl */
register char ch;
{
	register struct permonst *mp;

	/*
	 * can't genocide certain monsters
	 */
#ifdef SAC
	if (index("123 &:", ch)) return FALSE;
#else
	if (index("12 &:", ch))  return FALSE;
#endif
	if (ch == pm_eel.mlet)	return TRUE;
	for (mp = mons; mp < &mons[CMNUM+2]; mp++)
		if (mp->mlet == ch) return TRUE;

	return FALSE;
}

do_genocide() {
	extern char genocided[], fut_geno[];
	char buf[BUFSZ];
	register struct monst *mtmp, *mtmp2;

	if(Confusion != 0)  *buf = u.usym;
	else do {
	    pline("What monster do you want to genocide (Type the letter)? ");
	    getlin(buf);
	}

	while(strlen(buf) != 1 || !monstersym(*buf));

	if(!index(fut_geno, *buf))  charcat(fut_geno, *buf);
	if(!index(genocided, *buf)) charcat(genocided, *buf);
	else {
		pline("Such monsters do not exist in this world.");
		return;
	}
	for(mtmp = fmon; mtmp; mtmp = mtmp2){
		mtmp2 = mtmp->nmon;
		if(mtmp->data->mlet == *buf)
			mondead(mtmp);
	}
	pline("Wiped out all %c's.", Hallucination ? '@' : *buf);
	/* Scare the hallucinating player */
	if(*buf == '@') {
		u.uhp = -1;
		killer = "scroll of genocide";
	/* A polymorphed character will die as soon as he is rehumanized. */
		if(u.usym != '@')	pline("You feel dead inside.");
		else			done("died");
	}
#ifdef KAA
	else if (*buf==u.usym) rehumanize();
#endif
}

do_mapping()
{
	register struct rm *lev;
	register int num, zx, zy;

	for(zy = 0; zy < ROWNO; zy++)
	    for(zx = 0; zx < COLNO; zx++) {

		if((Confusion != 0) && rn2(7)) continue;
		lev = &(levl[zx][zy]);
		if((num = lev->typ) == 0)	continue;

		if(num == SCORR) {
			lev->typ = CORR;
			lev->scrsym = CORR_SYM;
		} else	if(num == SDOOR) {
			lev->typ = DOOR;
			lev->scrsym = DOOR_SYM;
		/* do sth in doors ? */
		} else if(lev->seen) continue;
#ifndef QUEST
		if(num != ROOM)
#endif
		{
			lev->seen = lev->new = 1;
			if(lev->scrsym == STONE_SYM || !lev->scrsym)
				newsym(zx,zy);
			else	on_scr(zx,zy);
		}
	    }
}

destroy_arm() {

	if(uarm) {
		pline("Your armor turns to dust and falls to the floor!");
		useup(uarm);
#ifdef SHIRT
	} else if(uarmu) {
		pline("Your shirt crumbles into tiny threads and falls apart!");
		useup(uarmu);
#endif
	} else if(uarmh) {
		pline("Your helmet turns to dust and is blown away!");
		useup(uarmh);
	} else if(uarmg) {
		pline("Your gloves vanish!");
		useup(uarmg);
		selftouch("You");
	} else if(uarms) {
		pline("Your shield crumbles away!");
		useup(uarms);
	} else  return(0);		/* could not destroy anything */

	return(1);
}
