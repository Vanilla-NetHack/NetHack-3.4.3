/*	SCCS Id: @(#)pickup.c	3.0	88/07/12
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	Contains code for picking objects up, and container use.
 */

#include	"hack.h"
STATIC_PTR int FDECL(in_container,(struct obj *));
STATIC_PTR int FDECL(ck_container,(struct obj *));
STATIC_PTR int FDECL(ck_bag,(struct obj *));
STATIC_PTR int FDECL(out_container,(struct obj *));
void FDECL(explode_bag,(struct obj *));

#define DELTA_CWT(cont) ((cont)->cursed?(obj->owt*2):(obj->owt/((cont)->blessed?4:2)) + 1)

#ifdef OVL0

static const char nearloadmsg[] = "have a little trouble lifting";

void
pickup(all)
int all;
{
	register struct gold *gold = g_at(u.ux, u.uy);
	register struct obj *obj, *obj2;
	register int wt;
	char buf[BUFSZ];
	register char *ip;
	register char sym;
	register int oletct = 0, iletct = 0;
	boolean all_of_a_type = FALSE, selective = FALSE;
	char olets[20], ilets[20];
	struct obj dummygold;

	dummygold.ox = u.ux;
	dummygold.oy = u.uy;
	dummygold.olet = GOLD_SYM;
	dummygold.nobj = fobj;
	dummygold.nexthere = level.objects[u.ux][u.uy];
	dummygold.cobj = 0;

	if(Levitation) {
		if ((multi && !flags.run) || (all && !flags.pickup))
			read_engr_at(u.ux,u.uy);
		return;
	}
	/* multi && !flags.run means they are in the middle of some other
	 * action, or possibly paralyzed, sleeping, etc.... and they just
	 * teleported onto the object.  They shouldn't pick it up.
	 */
	if ((multi && !flags.run) || (all && !flags.pickup)) {
		int ct = 0;

		for (obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
			if(!obj->cobj && obj != uchain)
				ct++;

		/* Stop on a zorkmid */
		if (gold) ct++;

		/* If there are objects here, take a look.
		 */
		if (ct) {
			if (flags.run)
				nomul(0);
			nscr();
			if (ct < 5)
				(void) dolook();
			else {
				read_engr_at(u.ux,u.uy);
				pline("There are several objects here.");
			}
		} else read_engr_at(u.ux,u.uy);
		return;
	}

	/* check for more than one object */
	if(!all) {
		register int ct = 0;

		if (gold) ct++;
		for(obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
			if(!obj->cobj) ct++;
		if(ct < 2)
			all++;
		else
			pline("There are several objects here.");
	}

	/* added by GAN 10/24/86 to allow selective picking up */
	if(!all)  {
		register struct obj *otmp = level.objects[u.ux][u.uy];

		ilets[iletct] = 0;
		if(gold) {
			ilets[iletct++] = GOLD_SYM;
			ilets[iletct] = 0;
		}
		while(otmp) {
			if(!index(ilets, otmp->olet) && !otmp->cobj) {
				ilets[iletct++] = otmp->olet;
				ilets[iletct] = 0;
			}
			otmp = otmp->nexthere;
		}
		if(iletct == 1)
			Strcpy(buf,ilets);
		else  {
			ilets[iletct++] = ' ';
			ilets[iletct++] = 'a';
			ilets[iletct++] = 'A';
			ilets[iletct] = 0;

			pline("What kinds of thing do you want to pick up? [%s] ", ilets);
			getlin(buf);
			if(buf[0] == '\033') {
				clrlin();
				return;
			}
			else if(!buf[0]) selective = TRUE;
		}
		ip = buf;
		olets[0] = 0;
		while(sym = *ip++){
			/* new A function (selective all) added by
			 * GAN 01/09/87
			 */
			if(sym == ' ') continue;
			if(sym == 'A') selective = TRUE;
			else if(sym == 'a') all_of_a_type = TRUE;
			else if(index(ilets, sym)){
				if(!index(olets, sym)){
					olets[oletct++] = sym;
					olets[oletct] = 0;
				}
			}
			else pline("There are no %c's here.", sym);
		}
	}
	if(all_of_a_type && !olets[0]) all = TRUE;

	for(obj = (gold ? &dummygold : level.objects[u.ux][u.uy]); obj;
			obj = obj2) {
	    obj2 = obj->nexthere;   /* perhaps obj will be picked up */
	    if(!obj->cobj) {
		if(flags.run) nomul(0);

		if(!all)  {
			char c;

			if(!selective && !index(olets,obj->olet)) continue;

			if (!all_of_a_type) {
				if (obj == &dummygold)
					pline("Pick up %ld gold piece%s? ",
					    gold->amount, plur(gold->amount));
				else pline("Pick up %s? ", doname(obj));
				if((c = ynaq()) == 'q') return;
				if(c == 'n') continue;
				if(c == 'a') {
					all_of_a_type = TRUE;
					if (selective) {
						selective = FALSE;
						olets[0] = obj->olet;
						olets[1] = 0;
						/* oletct = 1; */
					}
				}
			}
		}

		if(obj == &dummygold) {
		    int iw = inv_weight();
		    long gold_capacity;

#ifndef lint /* long/int conversion */
		    iw -= (int)((u.ugold + 500)/1000);
#endif
		    gold_capacity = ((-iw) * 1000L) - 500 + 999 - u.ugold;
		    if (gold_capacity <= 0L) {
	 pline("There %s %ld gold piece%s here, but you cannot carry any more.",
				(gold->amount == 1) ? "is" : "are",
				gold->amount, plur(gold->amount));
			continue;
		    }
		    if (gold_capacity >= gold->amount) {
			u.ugold += gold->amount;
			if (inv_weight() > -5)
				You(nearloadmsg);
			pline("%ld gold piece%s.",
				gold->amount, plur(gold->amount));
			freegold(gold);
			if(Invisible) newsym(u.ux,u.uy);
		    } else {
		You("can only carry %s of the %ld gold pieces lying here.",
			    gold_capacity == 1L ? "one" : "some", gold->amount);
			You(nearloadmsg);
			pline("%ld gold piece%s.",
				 gold_capacity, plur(gold_capacity));
			u.ugold += gold_capacity;
			gold->amount -= gold_capacity;
		    }
		    flags.botl = 1;
		    if(flags.run) nomul(0);
		    continue;
		}

		if((obj->otyp == CORPSE && obj->corpsenm == PM_COCKATRICE) &&
		   !uarmg
#ifdef POLYSELF
			&& !resists_ston(uasmon)
#endif
						) {
		    pline("Touching the dead cockatrice is a fatal mistake.");
		    You("turn to stone.");
		    You("die...");
		    killer_format = KILLED_BY_AN;
		    killer = "cockatrice corpse";
		    done(STONING);
		}

		if(obj->otyp == SCR_SCARE_MONSTER){
		  if(obj->blessed) obj->blessed = 0;
		  else if(!obj->spe && !obj->cursed) obj->spe = 1;
		  else {
		    pline("The scroll%s turn%s to dust as you pick %s up.",
				plur((long)obj->quan), (obj->quan==1) ? "s":"",
				(obj->quan==1) ? "it" : "them");
			if(!(objects[SCR_SCARE_MONSTER].oc_name_known) &&
			   !(objects[SCR_SCARE_MONSTER].oc_uname))
				docall(obj);
		    useupf(obj);
		    continue;
		  }
		}

		/* do not pick up uchain */
		if(obj == uchain)
			continue;

		wt = inv_weight() + (int)obj->owt;
		if (obj->otyp == LOADSTONE)
			goto lift_some; /* pick it up even if too heavy */
#ifdef POLYSELF
		if (obj->otyp == BOULDER && throws_rocks(uasmon)) {
			wt = inv_weight();
			goto lift_some;
		}
#endif
		if(wt > 0) {
			if(obj->quan > 1) {
				/* see how many we can lift */
				unsigned savequan = obj->quan;
				int iw = inv_weight();
				unsigned qq;
				for(qq = 1; qq < savequan; qq++){
					obj->quan = qq;
					if(iw + weight(obj) > 0)
						break;
				}
				obj->quan = savequan;
				qq--;
				/* we can carry qq of them */
				if(qq) {
				    register struct obj *obj3;

				You("can only carry %s of the %s lying here.",
					    (qq == 1) ? "one" : "some",
					    doname(obj));
				    obj3 = splitobj(obj, (int)qq);
				    if(obj3->otyp == SCR_SCARE_MONSTER)
					    if(obj3->spe) obj->spe = 0;
				    goto lift_some;
				}
			}
			pline("There %s %s here, but %s.",
				(obj->quan == 1) ? "is" : "are",
				doname(obj),
				!invent ? 
				(obj->quan == 1 ?
				    "it is too heavy for you to lift"
				  : "they are too heavy for you to lift")
					: "you cannot carry any more");
				if(obj->otyp == SCR_SCARE_MONSTER)
					if(obj->spe) obj->spe = 0;
			break;
		}
	lift_some:
		if(inv_cnt() >= 52) {
			Your("knapsack cannot accommodate any more items.");
			if(obj->otyp == SCR_SCARE_MONSTER)
				if(obj->spe) obj->spe = 0;
			break;
		}
		{ unsigned pickquan = obj->quan;
		  unsigned mergquan;

		  obj = pick_obj(obj);
		  if(wt > -5) You(nearloadmsg);
		  if(!Blind) obj->dknown = 1;
		  mergquan = obj->quan;
		  obj->quan = pickquan; /* to fool prinv() */
		  if(uwep && uwep == obj) mrg_to_wielded = TRUE;
		  prinv(obj);
		  mrg_to_wielded = FALSE;
		  obj->quan = mergquan;
		}
	    }
	}
}

struct obj *
pick_obj(otmp)
register struct obj *otmp;
{
	if (otmp != uball)	     /* don't charge for this - kd, 1/17/90 */
		addtobill(otmp, TRUE);       /* sets obj->unpaid if necessary */
	freeobj(otmp);
	if(Invisible) newsym(u.ux,u.uy);
	return(addinv(otmp));    /* might merge it with other objects */
}

#endif /* OVL1 */
#ifdef OVLB

int
doloot() {	/* loot a container on the floor. */

	register struct obj *cobj, *nobj;
	register int c;

	if (Levitation) {
		You("cannot reach the floor.");
		return(0);
	}
	for(cobj = level.objects[u.ux][u.uy]; cobj; cobj = nobj) {
	        nobj = cobj->nexthere;

		if(Is_container(cobj)) {

		    pline("There is %s here, loot it? ", doname(cobj));
		    c = ynq();
		    if(c == 'q') return 0;
		    if(c == 'n') continue;

		    if(cobj->olocked) {

			pline("Hmmm, it seems to be locked.");
			continue;
		    }
		    if(cobj->otyp == BAG_OF_TRICKS) {

			You("carefully open the bag...");
			pline("It develops a huge set of teeth and bites you!");
			losehp(rnd(10), "carnivorous bag", KILLED_BY_AN);
			makeknown(BAG_OF_TRICKS);
			continue;
		    }

		    You("carefully open the %s...", xname(cobj));
		    if(cobj->otrapped && chest_trap(cobj, FINGER)) /* don't use obj if obj dies */
		      continue;
		    if(multi < 0) return 0; /* a paralysis trap */

		    use_container(cobj, 0);
		}
	}
	return 0;
}

static
struct obj NEARDATA *current_container;	/* a local variable of use_container, to be
				used by its local procedures in/ck_container */
#define Icebox (current_container->otyp == ICE_BOX)
int baggone;	/* used in askchain so bag isn't used after explosion */

#endif /* OVLB */
#ifdef OVL1

void
inc_cwt(cobj, obj)
register struct obj *cobj, *obj;
{
	if (cobj->otyp == BAG_OF_HOLDING)
		cobj->owt += DELTA_CWT(cobj);
	else	cobj->owt += obj->owt;
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_PTR int
in_container(obj)
register struct obj *obj;
{
	char buf[BUFSZ];

	if(obj == uball || obj == uchain) {
		You("must be kidding.");
		return(0);
	}
	if(obj == current_container) {
		pline("That would be an interesting topological exercise.");
		return(0);
	}
	if(obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) {
		You("cannot %s something you are wearing.",
			Icebox ? "refrigerate" : "stash");
		return(0);
	}
	if((obj->otyp == LOADSTONE) && obj->cursed) {
		obj->bknown = 1;
		pline("The stone%s won't leave your person.",
			plur((long)obj->quan));
		return(0);
	}
	/* Prohibit Amulets in containers; if you allow it, monsters can't
	 * steal them.  It also becomes a pain to check to see if someone
	 * has the Amulet.
	 */
	if(obj->otyp == AMULET_OF_YENDOR && !obj->spe) {
	    pline("The Amulet of Yendor cannot be confined in such trappings.");
	    return(0);
	}
	/* no nested containers - they do not save/restore properly. */
	/* magic bag -> magic bag will self destruct later on. */
	if(Is_container(obj) && Is_container(current_container) &&
	    (!Is_mbag(obj) || !Is_mbag(current_container))) {
		pline("The %s won't go in.", xname(obj));
		return(1);	/* be careful! */
	}
	if(obj == uwep) {
		if(welded(obj)) {
			weldmsg(obj, FALSE);
			return(0);
		}
		setuwep((struct obj *) 0);
		if (uwep) return(0); /* unwielded, died, rewielded */
	}
#ifdef WALKIES
	if(obj->otyp == LEASH && obj->leashmon != 0) {
		pline("The %s is attached to your pet.", xname(obj));
		return(0);
	}
#endif
	inc_cwt(current_container, obj);
	freeinv(obj);

	obj->cobj = current_container;
	obj->nobj = fcobj;
	fcobj = obj;
	Strcpy(buf, xname(obj->cobj));
	You("put %s into the %s.", doname(obj), buf);

	if(Icebox) obj->age = monstermoves - obj->age; /* actual age */

	else if(Is_mbag(obj->cobj) &&
		(Is_mbag(obj) ||
		 (obj->otyp == WAN_CANCELLATION && (obj->spe > 0)) )) {
		explode_bag(obj);
		You("are blasted by a magical explosion!");
		losehp(d(6,6),"magical explosion", KILLED_BY_AN);
		baggone = 1;
	}
	return(1);
}

STATIC_PTR int
ck_container(obj)
register struct obj *obj;
{
	return(obj->cobj == current_container);
}

/* ck_bag() needs a formal argument to make the overlay/prototype mechanism
 * work right */
/*ARGSUSED*/
STATIC_PTR int
ck_bag(obj)
struct obj *obj;
{
	return(!baggone);
}

STATIC_PTR int
out_container(obj)
register struct obj *obj;
{
	register struct obj *otmp, *ootmp;
	register boolean near_capacity = (inv_weight() > -5);

	if(inv_cnt() >= 52) {
		pline("You have no room to hold anything else.");
		return(0);
	}
	if(obj->otyp != LOADSTONE && inv_weight() + (int)obj->owt -
	   (carried(current_container) ?
		(current_container->otyp == BAG_OF_HOLDING ?
		    (int)DELTA_CWT(current_container) : (int)obj->owt) : 0) > 0) {
		char buf[BUFSZ];

		Strcpy(buf, doname(obj));
		pline("There %s %s in the %s, but %s.",
			obj->quan==1 ? "is" : "are",
			buf, xname(current_container),
			invent ? "you cannot carry any more"
			: "it is too heavy for you to carry");
		/* "too heavy for you to lift" is not right if you're carrying
		   the container... */
		return(0);
	}
	if(obj == fcobj) fcobj = fcobj->nobj;
	else {
		for(otmp = fcobj; otmp->nobj != obj; otmp = otmp->nobj)
			if(!otmp->nobj) panic("out_container");
		otmp->nobj = obj->nobj;
	}
	dec_cwt(current_container, obj);
	obj->cobj = (struct obj *) 0;

	if (Icebox) obj->age = monstermoves - obj->age;
	/* simulated point of time */

	ootmp = addinv(obj);
	if (near_capacity) You("have a little trouble removing");
	prinv(ootmp);
	return 0;
}

/* for getobj: 0: allow cnt; #: allow all types; %: expect food */
static const char NEARDATA frozen_food[] = { '0', '#', FOOD_SYM, 0 };

void
use_container(obj, held)
register struct obj *obj;
register int held;
{
	register int cnt = 0;
	register struct obj *otmp;
	register struct obj *backobj;

	current_container = obj;	/* for use by in/out_container */
	if(current_container->olocked) {
		pline("The %s seems to be locked.", xname(current_container));
		return;
	}
	for(otmp = fcobj, backobj = (struct obj *) 0; otmp;
	    backobj = otmp, otmp = otmp->nobj)
		if(otmp->cobj == obj)
		    if(Is_mbag(obj) && obj->cursed && !rn2(13)) {
			if (otmp->known)
				pline("The %s to have vanished!",
							aobjnam(otmp,"seem"));
			else You("%s %s disappear.",
				Blind ? "notice" : "see",
				doname(otmp));
			if(!backobj) {
			    fcobj = otmp->nobj;
			    dec_cwt(current_container, otmp);
			    obfree(otmp, (struct obj *) 0);
			    otmp = fcobj;
			} else {
			    backobj->nobj = otmp->nobj;
			    dec_cwt(current_container, otmp);
			    obfree(otmp, (struct obj *) 0);
			    otmp = backobj->nobj;
			}
			if (!otmp) break;
			if(otmp->cobj == obj) cnt++;
		    } else cnt++;
	if(!cnt)
	    pline("%s %s is empty.", (held) ? "Your" : "The", xname(obj));
	else if (inv_cnt() < 52) {
	    pline("Do you want to take something out of the %s? ",
		  xname(obj));
	    if(yn() != 'n')
		if(askchain(fcobj, FALSE, NULL, 0, out_container, ck_container, 0, "nodot"))
		    return;
	}
	if(!invent) return;
	pline("Do you wish to put something in? ");
	if(yn() != 'y') return;
	if (Icebox && current_container->dknown) {
		otmp = getobj(frozen_food, "put in");
		if(!otmp || !in_container(otmp))
			flags.move = multi = 0;
	} else {
		baggone = 0; /* might be set by in_container */
		if(askchain(invent, TRUE, NULL, 0, in_container, ck_bag, 0, "nodot"))
		  return;
	}
	return;
}

void
delete_contents(obj)
register struct obj *obj;
{
	register struct obj *otmp, *notmp;

	while (fcobj && fcobj->cobj == obj) {
		otmp = fcobj;
		fcobj = fcobj->nobj;
		obfree(otmp,(struct obj *)0);
	}
	if (fcobj) {
		otmp = fcobj;
		while(otmp->nobj)
			if (otmp->nobj->cobj == obj) {
				notmp = otmp->nobj;
				otmp->nobj = notmp->nobj;
				obfree(notmp,(struct obj *)0);
			} else
				otmp = otmp->nobj;
	}
}

void
explode_bag(obj)
struct obj *obj;
{
	struct obj *otmp, *cobj;
	boolean found = FALSE;

	cobj = obj->cobj;
	delete_contents(cobj);

	for (otmp = invent; otmp; otmp = otmp->nobj)
		if (otmp == cobj) {
			found = TRUE;
			Your("%s blows apart!", xname(otmp));
			useup(otmp);
			break;
		}

	if (!found) {
	    /* maybe the bag was on the floor */
	    for (otmp=level.objects[u.ux][u.uy]; otmp; otmp=otmp->nexthere)
		if (otmp == cobj) {
			found = TRUE;
			pline("The %s blows apart!", xname(otmp));
			useupf(otmp);
			break;
		}
	}

	if (!found) panic("explode_bag: bag not found.");
}

void
dec_cwt(cobj, obj)
register struct obj *cobj, *obj;
{
	if (cobj->otyp == BAG_OF_HOLDING)
		cobj->owt -= DELTA_CWT(cobj);
	else	cobj->owt -= obj->owt;

	if(cobj->owt < objects[cobj->otyp].oc_weight)
		cobj->owt = objects[cobj->otyp].oc_weight;
}

#endif /* OVLB */
