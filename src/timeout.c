/*	SCCS Id: @(#)timeout.c	3.0	87/07/06
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

/* He is being petrified - dialogue by inmet!tower */
static const char *stoned_texts[] = {
	"You are slowing down.",		/* 5 */
	"Your limbs are stiffening.",		/* 4 */
	"Your limbs have turned to stone.",	/* 3 */
	"You have turned to stone.",		/* 2 */
	"You are a statue."			/* 1 */
};

static void
stoned_dialogue() {
	register long i = (Stoned & TIMEOUT);

	if(i > 0 && i <= SIZE(stoned_texts))
		pline(stoned_texts[SIZE(stoned_texts) - i]);
	if(i == 5)
		Fast &= ~(TIMEOUT|INTRINSIC);
	if(i == 3)
		nomul(-3);
}

static const char *choke_texts[] = {
	"You find it hard to breathe.",
	"You're gasping for air.",
	"You can no longer breathe.",
	"You're turning %s.",
	"You suffocate."
};

static void
choke_dialogue()
{
	register long i = (Strangled & TIMEOUT);

	if(i > 0 && i <= SIZE(choke_texts))
		pline(choke_texts[SIZE(choke_texts) - i], Hallucination ?
			hcolor() : blue);
}

void
timeout()
{
	register struct prop *upp;
	int sleeptime;

	if(Stoned) stoned_dialogue();
	if(Strangled) choke_dialogue();
#ifdef POLYSELF
	if(u.mtimedone) if(!--u.mtimedone) rehumanize();
#endif
	if(u.ucreamed) u.ucreamed--;
	if(u.uluck && moves % (u.uhave_amulet
#ifdef THEOLOGY
		|| u.ugangr
#endif
		? 300 : 600) == 0) {
	/* Cursed luckstones stop bad luck from timing out; blessed luckstones
	 * stop good luck from timing out; normal luckstones stop both;
	 * neither is stopped if you don't have a luckstone.
	 */
	    register int time_luck = stone_luck(FALSE);
	    boolean nostone = !carrying(LUCKSTONE);

	    if(u.uluck > 0 && (nostone || time_luck < 0))
		u.uluck--;
	    else if(u.uluck < 0 && (nostone || time_luck > 0))
		u.uluck++;
	}

	for(upp = u.uprops; upp < u.uprops+SIZE(u.uprops); upp++)
	    if((upp->p_flgs & TIMEOUT) && !(--upp->p_flgs & TIMEOUT)) {
		if(upp->p_tofn) (*upp->p_tofn)();
		else switch(upp - u.uprops){
		case STONED:
			if (!killer) killer = "cockatrice";
			done(STONING);
			break;
		case SICK:
			You("die from food poisoning.");
			killer = u.usick_cause;
			done(POISONING);
			break;
		case FAST:
			You("feel yourself slowing down.");
			break;
		case CONFUSION:
			HConfusion = 1; /* So make_confused works properly */
			make_confused(0L, TRUE);
			break;
		case STUN:
			HStun = 1;
			make_stunned(0L, TRUE);
			break;
		case BLINDED:
			Blinded = 1;
			make_blinded(0L, TRUE);
			break;
		case INVIS:
			on_scr(u.ux,u.uy);
			if (!Invis && !See_invisible)
				You("are no longer invisible.");
			break;
		case WOUNDED_LEGS:
			heal_legs();
			break;
		case HALLUC:
			Hallucination = 1;
			make_hallucinated(0L, TRUE);
			break;
		case SLEEPING:
			if (unconscious() || Sleep_resistance)
				Sleeping += rnd(100);
			else {
				You("fall asleep.");
				sleeptime = rnd(20);
				nomul(-sleeptime);
				nomovemsg = "You wake up.";
				Sleeping = sleeptime + rnd(100);
			}
			break;
		case STRANGLED:
			killer = "strangulation";
			done(DIED);
			break;
		case FUMBLING:
			/* call this only when a move took place.  */
			/* otherwise handle fumbling msgs locally. */
			if (!Levitation && u.umoved) {
			    if (OBJ_AT(u.ux, u.uy))
				You("trip over something.");
			    else
				switch (rn2(4)) {
				    case 1:
					if (ACCESSIBLE(levl[u.ux][u.uy].typ)) { /* not POOL or STONE */
					    if (Hallucination) pline("A rock bites your foot.");
					    else You("trip over a rock.");
					    break;
					}
				    case 2:
					if (Hallucination) You("slip on a banana peel.");
					else You("slip and nearly fall.");
					break;
				    case 3:
					You("flounder.");
					break;
				    default:
					You("stumble.");
				}
			    nomul(-2);
			    nomovemsg = "";
			}
			Fumbling = rnd(20);
			break;
		}
	}
}

static const char slithy[] = { S_SNAKE, S_NAGA, S_WORM, 0 };

static void
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
			  (index(slithy, mtmp->data->mlet)) ?
			  "slither" : "drop");
		else
		    pline("A%s %s just %s out of your pack!",
			  (index(vowels, *(mtmp->data->mname))) ? "n" : "",
			  mtmp->data->mname,
			  (index(slithy, mtmp->data->mlet)) ?
			  "slithered" : "dropped");

#ifdef POLYSELF
		if (yours) {
		    pline("Its cries sound like \"%s.\"",
			flags.female ? "mommy" : "daddy");
		    (void) tamedog(mtmp, (struct obj *)0);
		    return;
		}
#endif
		if(mtmp->data->mlet == S_DRAGON) {
		    verbalize("Gleep!");		/* Mything eggs :-) */
		    (void) tamedog(mtmp, (struct obj *)0);
		}
	    }
	}
}

void
hatch_eggs()	    /* hatch any eggs that have been too long in pack */
{
	register struct obj *otmp,/* *ctmp, /* use of ctmp commented out below*/
		*otmp2;

	for(otmp = invent; otmp; otmp = otmp2) {

	    otmp2 = otmp->nobj;	    /* otmp may hatch */
	    if(otmp->otyp == EGG && otmp->corpsenm >= 0) hatch_it(otmp);
	}

/*	Not yet - there's a slight problem with "useup" on contained objs.
	for(otmp = fcobj; otmp; otmp = otmp2) {

	    otmp2 = otmp->nobj;
	    for(ctmp = invent; ctmp; ctmp = ctmp->nobj)
		if(otmp->cobj == ctmp)
		    if(ctmp->otyp != ICE_BOX)
			if(otmp->otyp == EGG && otmp->corpsenm >= 0)
			    hatch_it(otmp);
	}
*/
}
