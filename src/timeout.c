/*	SCCS Id: @(#)timeout.c	3.1	93/03/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

STATIC_DCL void NDECL(stoned_dialogue);
STATIC_DCL void NDECL(vomiting_dialogue);
STATIC_DCL void NDECL(choke_dialogue);
STATIC_DCL void FDECL(hatch_it, (struct obj *));

static void FDECL(age_candle, (struct obj *));

#ifdef OVLB

/* He is being petrified - dialogue by inmet!tower */
static NEARDATA const char *stoned_texts[] = {
	"You are slowing down.",		/* 5 */
	"Your limbs are stiffening.",		/* 4 */
	"Your limbs have turned to stone.",	/* 3 */
	"You have turned to stone.",		/* 2 */
	"You are a statue."			/* 1 */
};

STATIC_OVL void
stoned_dialogue() {
	register long i = (Stoned & TIMEOUT);

	if(i > 0 && i <= SIZE(stoned_texts))
		pline(stoned_texts[SIZE(stoned_texts) - i]);
	if(i == 5)
		Fast &= ~(TIMEOUT|INTRINSIC);
	if(i == 3)
		nomul(-3);
	exercise(A_DEX, FALSE);
}

/* He is getting sicker and sicker prior to vomiting */
static NEARDATA const char *vomiting_texts[] = {
	"You are feeling mildly nauseous.",	/* 14 */
	"You feel slightly confused.",		/* 11 */
	"You can't seem to think straight.",	/* 8 */
	"You feel incredibly sick.",		/* 5 */
	"You suddenly vomit!"			/* 2 */
};

STATIC_OVL void
vomiting_dialogue() {
	register long i = (Vomiting & TIMEOUT) / 3L;

	if ((((Vomiting & TIMEOUT) % 3L) == 2) && (i >= 0)
	    && (i < SIZE(vomiting_texts)))
		pline(vomiting_texts[SIZE(vomiting_texts) - i - 1]);

	switch ((int) i) {
	case 0:
		vomit();
		morehungry(20);
		break;
	case 2:
		make_stunned(HStun + d(2,4), FALSE);
		/* fall through */
	case 3:
		make_confused(HConfusion + d(2,4), FALSE);
		break;
	}
	exercise(A_CON, FALSE);
}

static NEARDATA const char *choke_texts[] = {
	"You find it hard to breathe.",
	"You're gasping for air.",
	"You can no longer breathe.",
	"You're turning %s.",
	"You suffocate."
};

STATIC_OVL void
choke_dialogue()
{
	register long i = (Strangled & TIMEOUT);

	if(i > 0 && i <= SIZE(choke_texts))
		pline(choke_texts[SIZE(choke_texts) - i], Hallucination ?
			hcolor() : blue);
	exercise(A_STR, FALSE);
}

#endif /* OVLB */
#ifdef OVL0

void
nh_timeout()
{
	register struct prop *upp;
	int sleeptime;

	if(u.uluck && moves % (u.uhave.amulet || u.ugangr ? 300 : 600) == 0) {
	/* Cursed luckstones stop bad luck from timing out; blessed luckstones
	 * stop good luck from timing out; normal luckstones stop both;
	 * neither is stopped if you don't have a luckstone.
	 * Luck is based at 0 usually, +1 if a full moon and -1 on Friday 13th
	 */
	    register int time_luck = stone_luck(FALSE);
	    boolean nostone = !carrying(LUCKSTONE) && !stone_luck(TRUE);
	    int baseluck = (flags.moonphase == FULL_MOON) ? 1 : 0;

	    baseluck -= (flags.friday13 ? 1 : 0);

	    if(u.uluck > baseluck && (nostone || time_luck < 0))
		u.uluck--;
	    else if(u.uluck < baseluck && (nostone || time_luck > 0))
		u.uluck++;
	}
	if(u.uinvulnerable) return; /* things past this point could kill you */
	if(Stoned) stoned_dialogue();
	if(Vomiting) vomiting_dialogue();
	if(Strangled) choke_dialogue();
#ifdef POLYSELF
	if(u.mtimedone) if(!--u.mtimedone) rehumanize();
#endif
	if(u.ucreamed) u.ucreamed--;

	for(upp = u.uprops; upp < u.uprops+SIZE(u.uprops); upp++)
	    if((upp->p_flgs & TIMEOUT) && !(--upp->p_flgs & TIMEOUT)) {
		if(upp->p_tofn) (*upp->p_tofn)();
		else switch(upp - u.uprops){
		case STONED:
			if (!killer) {
				killer_format = KILLED_BY_AN;
				killer = "cockatrice";
			} done(STONING);
			break;
		case VOMITING:
			make_vomiting(0L, TRUE);
			break;
		case SICK:
			You("die from your illness.");
			killer_format = KILLED_BY_AN;
			killer = u.usick_cause;
			done(POISONING);
			break;
		case FAST:
			if (Fast & ~INTRINSIC) /* boot speed */
				;
			else
				You("feel yourself slowing down%s.",
							Fast ? " a bit" : "");
			break;
		case CONFUSION:
			HConfusion = 1; /* So make_confused works properly */
			make_confused(0L, TRUE);
			stop_occupation();
			break;
		case STUNNED:
			HStun = 1;
			make_stunned(0L, TRUE);
			stop_occupation();
			break;
		case BLINDED:
			Blinded = 1;
			make_blinded(0L, TRUE);
			stop_occupation();
			break;
		case INVIS:
			newsym(u.ux,u.uy);
			if (!Invis && !See_invisible && !Blind) {
				You("are no longer invisible.");
				stop_occupation();
			}
			break;
		case SEE_INVIS:
			set_mimic_blocking(); /* do special mimic handling */
			see_monsters();		/* make invis mons appear */
			newsym(u.ux,u.uy);	/* make self appear */
			stop_occupation();
			break;
		case WOUNDED_LEGS:
			heal_legs();
			stop_occupation();
			break;
		case HALLUC:
			HHallucination = 1;
			make_hallucinated(0L, TRUE, 0L);
			stop_occupation();
			break;
		case SLEEPING:
			if (unconscious() || Sleep_resistance)
				Sleeping += rnd(100);
			else {
				You("fall asleep.");
				sleeptime = rnd(20);
				nomul(-sleeptime);
				u.usleep = 1;
				nomovemsg = "You wake up.";
				Sleeping = sleeptime + rnd(100);
			}
			break;
		case STRANGLED:
			killer_format = KILLED_BY;
			killer = "strangulation";
			done(DIED);
			break;
		case FUMBLING:
			/* call this only when a move took place.  */
			/* otherwise handle fumbling msgs locally. */
			if (!Levitation && u.umoved) {
			    if (OBJ_AT(u.ux, u.uy))
				You("trip over something.");
			    else if (rn2(3) && is_ice(u.ux, u.uy))
				You("%s on the ice.",
				    rn2(2) ? "slip" : "slide");
			    else
				switch (rn2(4)) {
				    case 1:
					if (ACCESSIBLE(levl[u.ux][u.uy].typ)) { /* not POOL or STONE */
					    if (Hallucination) pline("A rock bites your %s.", body_part(FOOT));
					    else You("trip over a rock.");
					    break;
					}
				    case 2:
					if (ACCESSIBLE(levl[u.ux][u.uy].typ)) { /* not POOL or STONE */
					    if (Hallucination) You("slip on a banana peel.");
					    else You("slip and nearly fall.");
					    break;
					}
				    case 3:
					You("flounder.");
					break;
				    default:
					You("stumble.");
				}
			    nomul(-2);
			    nomovemsg = "";
			    /* Fumbling can be noisy */
			    if ((inv_weight() > -500)) {
			    	You("make a lot of noise!");
			    	wake_nearby();
			    }
			}
			Fumbling += rnd(20);
			break;
		}
	}
}

#endif /* OVL0 */
#ifdef OVLB

STATIC_OVL void
hatch_it(otmp)		/* hatch the egg "otmp" if possible */
register struct obj *otmp;
{
	register struct monst *mtmp;
#ifdef POLYSELF
	int yours = otmp->spe;
#endif

	if(monstermoves-otmp->age > 200)  /* very old egg - it's dead */
	    otmp->corpsenm = -1;
#ifdef LINT	/* long conv. ok */
	else if(rnd(150) > 150) {
#else
	else if(rnd((int)(monstermoves-otmp->age)) > 150) {
#endif
	    mtmp = makemon(&mons[big_to_little(otmp->corpsenm)], u.ux, u.uy);
	    useup(otmp);
	    if(mtmp) {

		if(Blind)
		    You("feel something %s from your pack!",
			locomotion(mtmp->data, "drop"));
		else
		    You("see %s %s out of your pack!",
			a_monnam(mtmp),
			locomotion(mtmp->data, "drop"));

#ifdef POLYSELF
		if (yours) {
		    struct monst *mtmp2;

		    pline("Its cries sound like \"%s.\"",
			flags.female ? "mommy" : "daddy");
		    if ((mtmp2 = tamedog(mtmp, (struct obj *)0)) != 0)
			mtmp = mtmp2;
		    mtmp->mtame = 20;
		    while ((otmp = (mtmp->minvent)) != 0) {
			mtmp->minvent = otmp->nobj;
			dealloc_obj(otmp);
		    }
		    return;
		}
#endif
		if(mtmp->data->mlet == S_DRAGON) {
		    struct monst *mtmp2;

		    verbalize("Gleep!");		/* Mything eggs :-) */
		    if ((mtmp2 = tamedog(mtmp, (struct obj *)0)) != 0)
			mtmp = mtmp2;
		    while ((otmp = (mtmp->minvent)) != 0) {
			mtmp->minvent = otmp->nobj;
			dealloc_obj(otmp);
		    }
		}
	    }
	}
}

#endif /* OVLB */
#ifdef OVL1

void
hatch_eggs()	    /* hatch any eggs that have been too long in pack */
{
	register struct obj *otmp, *otmp2;

	for(otmp = invent; otmp; otmp = otmp2) {
	    otmp2 = otmp->nobj;	    /* otmp may hatch */
	    if(otmp->otyp == EGG && otmp->corpsenm >= 0) hatch_it(otmp);
	    /* else if (Has_contents(otmp)) ...				*/
	    /*								*/
	    /* Check for container here and hatch with the container.	*/
	    /* One of these days...					*/
	    /* Maybe call hatch_eggs() with invent as a parameter so	*/
	    /* that we can call it recursively.				*/
	}
}

/* Burn up lit lamps.  Only applies to non-magic lamps; magic lamps stay
 * lit as long as there's a genie inside.  We use obj->age to see how long
 * there is left for the lamp to burn, but this differs from the use of
 * age for corpses and eggs: for the latter items it's when the object was
 * created, but for lamps it's the number of moves remaining.
 */
void
burn_lamps()
{
	register struct obj *obj, *obj2;

	/* Note: magic lamps never go out as long as the genie's inside */
	for (obj=invent; obj; obj=obj2) {
	    obj2 = obj->nobj;
	    if ((obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN)
							&& obj->lamplit) {
		obj->age--;
		switch((int)obj->age) {
		    case 150:
		    case 100:
			if (obj->otyp == BRASS_LANTERN) goto advmsg;
			if (!Blind)
			    Your("%s flickers.", xname(obj));
			break;
		    case 50:
			if (obj->otyp == BRASS_LANTERN) goto advmsg;
			if (!Blind)
			    Your("%s flickers considerably.", xname(obj));
			break;
		    case 25:
	advmsg:		if (!Blind) {
			    if (obj->otyp == BRASS_LANTERN) {
				Your("lamp is getting dim.");
				if (Hallucination)
				    pline("Batteries have not been invented yet.");
			    } else
				Your("%s seems about to go out.", xname(obj));
			}
			break;
		    case 0: /* even if blind you'll know */
			if (obj->otyp == BRASS_LANTERN)
				Your("lamp has run out of power.");
			else Your("%s goes out.", xname(obj));
			obj->lamplit = 0;
			obj->spe = 0;
			check_lamps();
			break;
		    default: break;
		}
	    }
	    if ((obj->otyp == CANDELABRUM_OF_INVOCATION || Is_candle(obj)) &&
			obj->lamplit)
		age_candle(obj);	/* candles may vanish */
	}
}

static void
age_candle(otmp)
register struct obj *otmp;
{
	register boolean many, 
	                 menorah = otmp->otyp == CANDELABRUM_OF_INVOCATION;

	otmp->age--;

	if (otmp->age == 0L) {
	    otmp->lamplit = 0;
	    many = menorah ? otmp->spe > 1 : otmp->quan > 1L;
	    if (menorah) {
		pline("%s's flame%s.", 
			The(xname(otmp)), (many ? "s die" : " dies"));
		otmp->spe = 0;
	    } else {
		Your("%s %s consumed!  %s",
			xname(otmp), (many ? "are" : "is"),
			(Hallucination ?
			    (many ? "They shriek!" : "It shrieks!") :
			 Blind ? "" :
			    (many ? "Their flames die." : "Its flame dies.")));
		freeinv(otmp);
		obfree(otmp, (struct obj *)0);
	    }
	    check_lamps();
	} else if (Blind) {
	    return;
	} else if (otmp->age == 15L) {
	    many = menorah ? otmp->spe > 1 : otmp->quan > 1L;
	    Norep("The %scandle%s flame%s flicker%s low!",
			(menorah ? "candelabrum's " : ""),
			(many ? "s'" : "'s"),
			(many ? "s" : ""),
			(many ? "" : "s"));
	} else if (otmp->age == 75L) {
	    many = menorah ? otmp->spe > 1 : otmp->quan > 1L;
	    Norep("The %scandle%s getting short.",
			menorah ? "candelabrum's " : "",
			(many ? "s are" : " is"));
	}
}
void
do_storms()
{
    int nstrike;
    register int x, y;
    int dirx, diry;
    int count;

    /* no lightning if not the air level or too often, even then */
    if(!Is_airlevel(&u.uz) || rn2(8))
	return;

    /* the number of strikes is 8-log2(nstrike) */
    for(nstrike = rnd(64); nstrike <= 64; nstrike *= 2) {
	count = 0;
	do {
	    x = rnd(COLNO-1);
	    y = rn2(ROWNO);
	} while (++count < 100 && levl[x][y].typ != CLOUD);

	if(count < 100) {
	    dirx = rn2(3) - 1;
	    diry = rn2(3) - 1;
	    if(dirx != 0 || diry != 0)
		buzz(-15, /* "monster" LIGHTNING spell */
		     8, x, y, dirx, diry);
	}
    }

    if(levl[u.ux][u.uy].typ == CLOUD) {
	/* inside a cloud during a thunder storm is deafening */
	pline("Kaboom!!!  Boom!!  Boom!!");
	if(!u.uinvulnerable) {
	    stop_occupation();
	    nomul(-3);
	}
    } else
	You("hear a rumbling noise.");
}
#endif /* OVL1 */

/*timeout.c*/
