/*	SCCS Id: @(#)pickup.c	3.1	93/01/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	Contains code for picking objects up, and container use.
 */

#include	"hack.h"

static void FDECL(unsplitobj, (struct obj *,struct obj *,long));
static void FDECL(simple_look, (struct obj *,BOOLEAN_P));
static boolean FDECL(query_classes, (char *,boolean *,boolean *,
			     const char *,struct obj *,BOOLEAN_P,BOOLEAN_P));
static boolean FDECL(pickup_object, (struct obj *,struct obj *));
static boolean FDECL(mbag_explodes, (struct obj *,int));
STATIC_PTR int FDECL(in_container,(struct obj *));
STATIC_PTR int FDECL(ck_bag,(struct obj *));
STATIC_PTR int FDECL(out_container,(struct obj *));

/*
 *  How much the weight of the given container will change when the given
 *  object is removed from it.  This calculation must match the one used
 *  by weight() in mkobj.c.
 */
#define DELTA_CWT(cont,obj)		\
    ((cont)->cursed ? (obj)->owt * 2 :	\
		      1 + ((obj)->owt / ((cont)->blessed ? 4 : 2)))

static const char moderateloadmsg[] = "You have a little trouble lifting";
static const char nearloadmsg[] = "You have much trouble lifting";

static void
unsplitobj(obj_block, obj_chip, resplit)
register struct obj *obj_block, *obj_chip;
long resplit;	/* non-zero => shift the quantities */
{
	if (obj_block->nobj != obj_chip) {
		impossible("can't unsplit objects");
	} else if (resplit) { /* 1st object should be reduced to 'resplit' */
		obj_chip->quan += (obj_block->quan - resplit);
		obj_block->quan = resplit;
		obj_block->owt = weight(obj_block);
		obj_chip->owt = weight(obj_chip);
	} else {  /* 2nd obj should be merged back into 1st, then destroyed */
		obj_block->nobj = obj_chip->nobj;
		obj_block->nexthere = obj_chip->nexthere;
		obj_block->quan += obj_chip->quan;
		obj_block->owt = weight(obj_block);
		/* no need to worry about 'unsplitbill'; unsplit only occurs
		   when unable to pick something up, hence we're not dealing
		   with billable objects here (I hope!)
		 */
		dealloc_obj(obj_chip);
	}
}

/* much simpler version of the look-here code; used by query_classes() */
static void
simple_look(otmp, here)
struct obj *otmp;	/* list of objects */
boolean here;		/* flag for type of obj list linkage */
{
	/* Neither of the first two cases is expected to happen, since
	 * we're only called after multiple classes of objects have been
	 * detected, hence multiple objects must be present.
	 */
	if (!otmp) {
	    impossible("simple_look(NULL)");
	} else if (!(here ? otmp->nexthere : otmp->nobj)) {
	    pline("%s", doname(otmp));
	} else {
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    putstr(tmpwin, 0, "");
	    do {
		putstr(tmpwin, 0, doname(otmp));
		otmp = here ? otmp->nexthere : otmp->nobj;
	    } while (otmp);
	    display_nhwindow(tmpwin, TRUE);
	    destroy_nhwindow(tmpwin);
	}
}

int
collect_obj_classes(ilets, otmp, here, incl_gold)
char ilets[];
register struct obj *otmp;
boolean here, incl_gold;
{
	register int iletct = 0;
	register char c, last_c = '\0';

	if (incl_gold)
		ilets[iletct++] = def_oc_syms[GOLD_CLASS];
	ilets[iletct] = '\0'; /* terminate ilets so that index() will work */
	while (otmp) {
		c = def_oc_syms[(int)otmp->oclass];
		if (c != last_c && !index(ilets, (last_c = c)))
			ilets[iletct++] = c,  ilets[iletct] = '\0';
		otmp = here ? otmp->nexthere : otmp->nobj;
	}

	return iletct;
}

static boolean
query_classes(olets, one_at_a_time, everything, action, objs, here, incl_gold)
char olets[];
boolean *one_at_a_time, *everything;
const char *action;
struct obj *objs;
boolean here, incl_gold;
{
	char ilets[20], inbuf[BUFSZ];
	int iletct, oletct;
	char qbuf[QBUFSZ];

	olets[oletct = 0] = '\0';
	*one_at_a_time = *everything = FALSE;
	iletct = collect_obj_classes(ilets, objs, here, incl_gold);
	if (iletct == 0) {
		return FALSE;
	} else if (iletct == 1) {
		olets[0] = def_char_to_objclass(ilets[0]);
		olets[1] = '\0';
	} else  {	/* more than one choice available */
		const char *where = 0;
		register char sym, oc_of_sym, *p;
		/* additional choices */
		ilets[iletct++] = ' ';
		ilets[iletct++] = 'a';
		ilets[iletct++] = 'A';
		ilets[iletct++] = (objs == invent ? 'i' : ':');
		ilets[iletct] = '\0';
ask_again:
		olets[oletct = 0] = '\0';
		*one_at_a_time = *everything = FALSE;
		Sprintf(qbuf,"What kinds of thing do you want to %s? [%s]",
			action, ilets);
		getlin(qbuf,inbuf);
		if (*inbuf == '\033') {
			clear_nhwindow(WIN_MESSAGE);
			return FALSE;
		}
		for (p = inbuf; (sym = *p++); ) {
		    /* new A function (selective all) added by GAN 01/09/87 */
		    if (sym == ' ') continue;
		    else if (sym == 'A') *one_at_a_time = TRUE;
		    else if (sym == 'a') *everything = TRUE;
		    else if (sym == ':') {
			simple_look(objs, here);  /* dumb if objs==invent */
			goto ask_again;
		    } else if (sym == 'i') {
			(void) display_inventory(NULL, FALSE);
			goto ask_again;
		    } else {
			oc_of_sym = def_char_to_objclass(sym);
			if (index(ilets,sym)) {
			    olets[oletct++] = oc_of_sym;
			    olets[oletct] = '\0';
			} else {
			    if (!where)
				where = !strcmp(action,"pick up")  ? "here" :
					!strcmp(action,"take out") ?
							    "inside" : "";
			    if (*where)
				pline("There are no %c's %s.", sym, where);
			    else
				You("have no %c's.", sym);
			}
		    }
		}
		if (!oletct && !*everything) *one_at_a_time = TRUE;
	}
	return TRUE;
}

void
pickup(all)
int all;	/* all >= 0 => yes/no; < 0 => -count */
{
	register struct obj *obj;
	struct obj *obj2, *objx;
	boolean all_of_a_type = FALSE, selective = FALSE;
	char olets[20];
	long count;

	count = (all < 0) ? (-1L * all) : 0L;
	if (count) all = 0;

	if(Levitation && !Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz)) {
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

		for (obj = level.objects[u.ux][u.uy]; obj;
						 obj = obj->nexthere)
			if(obj != uchain)
				ct++;

		/* If there are objects here, take a look.
		 */
		if (ct) {
			if (flags.run)
				nomul(0);
			flush_screen(1);
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

		for(obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
			ct++;
		if(ct < 2)
			all++;
		else {
			pline("There are several objects here.");
			count = 0;
		}
	}

#ifdef POLYSELF
	if (nolimbs(uasmon)) {
		You("cannot pick things up without limbs.");
		return;
	}
#endif

	/* added by GAN 10/24/86 to allow selective picking up */
	if (!all) {
		if (!query_classes(olets, &selective, &all_of_a_type,
			  "pick up", level.objects[u.ux][u.uy], TRUE, FALSE))
			return;
	}
	if(all_of_a_type && !olets[0]) all = TRUE;

	for(obj = level.objects[u.ux][u.uy]; obj; obj = obj2) {
		obj2 = obj->nexthere;	/* perhaps obj will be picked up */
		objx = 0;
		if(flags.run) nomul(0);

		if(!all)  {
		    if(!selective && !index(olets,obj->oclass)) continue;

		    if (!all_of_a_type) {
			char qbuf[QBUFSZ];
			Sprintf(qbuf,"Pick up %s?", doname(obj));
			switch ((obj->quan < 2L) ? ynaq(qbuf) : ynNaq(qbuf)) {
			case 'q': return;
			case 'n': continue;
			case 'a':
			    all_of_a_type = TRUE;
			    if (selective) {
				selective = FALSE;
				olets[0] = obj->oclass;
				olets[1] = '\0';
			    }
			    break;
			case '#':	/* count was entered */
			    if (!yn_number) continue; /* 0 count => No */
			    else count = yn_number;
			    /* fall thru :-} */
			default:	/* 'y' */
			    break;
			}
		    }
		}

		if (count) {
		    /* Pickup a specific number of items; split the object
		       unless count corresponds to full quantity.  1 special
		       case:  cursed loadstones will remain as merged unit.
		     */
		    if (count < obj->quan &&
			    (!obj->cursed || obj->otyp != LOADSTONE)) {
			objx = splitobj(obj, count);
			if (!objx || objx->nexthere != obj2)
			    impossible("bad object split in pickup");
		    }
		    count = 0;	/* reset */
		}
		if (pickup_object(obj, objx)) break;
	}

	/*
	 *  Re-map what is at the hero's location after the pickup so the
	 *  map is correct.
	 */
	newsym(u.ux,u.uy);
}

/*
 * Pick up an object from the ground or out of a container and add it to
 * the inventory.  Returns true if pickup() should break out of its loop.
 */
static boolean
pickup_object(obj, objx)
struct obj *obj, *objx;
{
	int wt, nearload;
	long pickquan;

	if (obj == uchain) {    /* do not pick up attached chain */
	    return FALSE;
	} else if (obj->oartifact && !touch_artifact(obj,&youmonst)) {
	    return FALSE;
	} else if (obj->otyp == GOLD_PIECE) {
	    /*
	     *  Special consideration for gold pieces...
	     */
	    long iw = (long)max_capacity() - ((u.ugold + 50L) / 100L);
	    long gold_capacity = ((-iw) * 100L) - 50L + 99L - u.ugold;

	    if (gold_capacity <= 0L) {
		if (objx) unsplitobj(obj, objx, 0L);
       pline("There %s %ld gold piece%s here, but you cannot carry any more.",
			(obj->quan == 1L) ? "is" : "are",
			obj->quan, plur(obj->quan));
		return FALSE;
	    } else if (gold_capacity < obj->quan) {
		if (objx) unsplitobj(obj, objx, 0L);
		You("can only carry %s of the %ld gold pieces lying here.",
		    gold_capacity == 1L ? "one" : "some", obj->quan);
		pline("%s %ld gold piece%s.",
		    nearloadmsg, gold_capacity, plur(gold_capacity));
		u.ugold += gold_capacity;
		obj->quan -= gold_capacity;
		costly_gold(obj->ox, obj->oy, gold_capacity);
	    } else {
		u.ugold += obj->quan;
		if ((nearload = near_capacity()) != 0)
		    pline("%s %ld gold piece%s.",
			  nearload < MOD_ENCUMBER ?
			  moderateloadmsg : nearloadmsg,
			  obj->quan, plur(obj->quan));
		else
		    prinv(NULL, obj, 0L);
		costly_gold(obj->ox, obj->oy, obj->quan);
		delobj(obj);
	    }
	    flags.botl = 1;
	    if (flags.run) nomul(0);
	    return FALSE;
	} else if (obj->otyp == CORPSE) {

	    if (obj->corpsenm == PM_COCKATRICE && !uarmg
#ifdef POLYSELF
		&& !resists_ston(uasmon)
#endif
	    ) {
#ifdef POLYSELF
		if (poly_when_stoned(uasmon) && polymon(PM_STONE_GOLEM))
		    display_nhwindow(WIN_MESSAGE, FALSE);
		else
#endif
		{
		  pline("Touching the cockatrice corpse is a fatal mistake.");
		    You("turn to stone.");
		    You("die...");
		    killer_format = KILLED_BY_AN;
		    killer = "cockatrice corpse";
		    done(STONING);
		}
	    } else if (is_rider(&mons[obj->corpsenm])) {
		pline("At your touch, the corpse suddenly moves...");
		revive_corpse(obj, 1, FALSE);
		exercise(A_WIS, FALSE);
		return FALSE;
	    }

	} else  if (obj->otyp == SCR_SCARE_MONSTER) {
	    if (obj->blessed) obj->blessed = 0;
	    else if (!obj->spe && !obj->cursed) obj->spe = 1;
	    else {
		pline("The scroll%s turn%s to dust as you pick %s up.",
			plur(obj->quan), (obj->quan == 1L) ? "s" : "",
			(obj->quan == 1L) ? "it" : "them");
		if (!(objects[SCR_SCARE_MONSTER].oc_name_known) &&
				    !(objects[SCR_SCARE_MONSTER].oc_uname))
		    docall(obj);
		useupf(obj);
		return FALSE;
	    }
	}

	wt = max_capacity() + (int)obj->owt;
	if (obj->otyp == LOADSTONE)
	    goto lift_some;     /* pick it up even if too heavy */
#ifdef POLYSELF
	if (obj->otyp == BOULDER && throws_rocks(uasmon)) {
	    goto lift_some;
	}
#endif
	if (wt > 0) {
	    if (obj->quan > 1L) {
		/* see how many we can lift */
		long qq, savequan = obj->quan;
		int iw = max_capacity();
		/*  This is correct only because containers */
		/*  don't merge.	-dean		    */
		for (qq = 1; qq < savequan; qq++) {
			obj->quan = qq;
			if (iw + weight(obj) > 0)
				break;
		}
		obj->quan = savequan;
		qq--;
		/* we can carry qq of them */
		if (qq) {
		    if (objx) {		/* temporarily unsplit */
			savequan = obj->quan;
			obj->quan += objx->quan;
		    }
		    You("can only carry %s of the %s lying here.",
			(qq == 1L) ? "one" : "some", doname(obj));
		    if (objx) {		/* re-do the prior split */
			obj->quan = savequan;
			unsplitobj(obj, objx, qq);
		    } else {		/* split into two groups */
			objx = splitobj(obj, qq);
			if (objx->otyp == SCR_SCARE_MONSTER) objx->spe = 0;
		    }
		    goto lift_some;
		}
	    }
	    if (objx) unsplitobj(obj, objx, 0L);
	    pline("There %s %s here, but %s.",
		    (obj->quan == 1L) ? "is" : "are", doname(obj),
		    !invent ? (obj->quan == 1L ?
				"it is too heavy for you to lift" :
				"they are too heavy for you to lift") :
			"you cannot carry any more");
	    if (obj->otyp == SCR_SCARE_MONSTER) obj->spe = 0;
	    return TRUE;
	}

lift_some:
	if (inv_cnt() >= 52) {
	    if (objx) unsplitobj(obj, objx, 0L);
	    Your("knapsack cannot accommodate any more items.");
	    if (obj->otyp == SCR_SCARE_MONSTER) obj->spe = 0;
	    return TRUE;
	}

	pickquan = obj->quan;	/* save number picked up */
	obj = pick_obj(obj);

	if (!Blind) obj->dknown = 1;
	if (uwep && uwep == obj) mrg_to_wielded = TRUE;
	nearload = near_capacity();
	prinv(nearload > SLT_ENCUMBER ? nearloadmsg :
	      nearload > UNENCUMBERED ? moderateloadmsg : NULL,
	      obj, pickquan);
	mrg_to_wielded = FALSE;
	return FALSE;
}

/* Gold never reaches this routine. */
struct obj *
pick_obj(otmp)
register struct obj *otmp;
{
	freeobj(otmp);
	if (*u.ushops && costly_spot(u.ux, u.uy) &&
	    otmp != uball)     /* don't charge for this - kd, 1/17/90 */
	   /* sets obj->unpaid if necessary */
	    addtobill(otmp, TRUE, FALSE, FALSE);
	if(Invisible) newsym(u.ux,u.uy);
	return(addinv(otmp));    /* might merge it with other objects */
}

/*
 * prints a message if encumbrance changed since the last check and
 * returns the new encumbrance value (from near_capacity()).
 */
int
encumber_msg()
{
    static int oldcap = UNENCUMBERED;
    int newcap = near_capacity();

    if(oldcap < newcap) {
	switch(newcap) {
	case 1: Your("movements are slowed slightly because of your load.");
		break;
	case 2: You("rebalance your load.  Movement is difficult.");
		break;
	case 3: You("stagger under your heavy load.  Movement is very hard.");
		break;
	default: You("can barely move a handspan with this load!");
		break;
	}
	flags.botl = 1;
    } else if(oldcap > newcap) {
	switch(newcap) {
	case 0: Your("movements are now unencumbered.");
		break;
	case 1: Your("movements are only slowed slightly by your load.");
		break;
	case 2: You("rebalance your load.  Movement is still difficult.");
		break;
	case 3: You("stagger under your load.  Movement is still very hard.");
		break;
	}
	flags.botl = 1;
    }

    oldcap = newcap;
    return (newcap);
}

int
doloot()	/* loot a container on the floor. */
{
	register struct obj *cobj, *nobj;
	register int c;
	int timepassed = 0;

	if (Levitation && !Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz)) {
		You("cannot reach the floor.");
		return(0);
	}
	if(is_pool(u.ux, u.uy)) {
		You("cannot loot things that are deep in the water.");
		return(0);
	}

#ifdef POLYSELF
	if(nolimbs(uasmon)) {
		You("cannot loot things without limbs.");
		return(0);
	}
#endif

	for(cobj = level.objects[u.ux][u.uy]; cobj; cobj = nobj) {
		nobj = cobj->nexthere;

		if(Is_container(cobj)) {
		    char qbuf[QBUFSZ];

		    Sprintf(qbuf, "There is %s here, loot it?", doname(cobj));
		    c = ynq(qbuf);
		    if(c == 'q') return (timepassed);
		    if(c == 'n') continue;

		    if(cobj->olocked) {
			pline("Hmmm, it seems to be locked.");
			continue;
		    }
		    if(cobj->otyp == BAG_OF_TRICKS) {
			You("carefully open the bag...");
			pline("It develops a huge set of teeth and bites you!");
			c = rnd(10);
			if(Half_physical_damage) c = (c+1) / 2;
			losehp(c, "carnivorous bag", KILLED_BY_AN);
			makeknown(BAG_OF_TRICKS);
			timepassed = 1;
			continue;
		    }

		    You("carefully open %s...", the(xname(cobj)));
		    if (cobj->otrapped && chest_trap(cobj, FINGER, FALSE)) {
			timepassed = 1;
			continue;	/* explosion destroyed cobj */
		    }
		    if(multi < 0) return (1); /* a paralysis trap */

		    timepassed |= use_container(cobj, 0);
		}
	}
	return (timepassed);
}

/*
 * Decide whether an object being placed into a magic bag will cause
 * it to explode.  If the object is a bag itself, check recursively.
 */
static boolean
mbag_explodes(obj, depthin)
    struct obj *obj;
    int depthin;
{
    /* odds: 1/1, 2/2, 3/4, 4/8, 5/16, 6/32, 7/64, 8/128, 9/128, 10/128,... */
    if ((Is_mbag(obj) || (obj->otyp == WAN_CANCELLATION && obj->spe > 0)) &&
	(rn2(1 << (depthin > 7 ? 7 : depthin)) <= depthin))
	return TRUE;
    else if (Is_container(obj)) {
	struct obj *otmp;

	for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
	    if (mbag_explodes(otmp, depthin+1)) return TRUE;
    }
    return FALSE;
}

/* A variable set in use_container(), to be used by the callback routines */
/* chk_bg(), in_container(), and out_container() from askchain().	  */
static struct obj NEARDATA *current_container;
#define Icebox (current_container->otyp == ICE_BOX)

STATIC_PTR int
in_container(obj)
register struct obj *obj;
{
	register struct obj *gold;
	boolean is_gold = (obj->otyp == GOLD_PIECE);
	boolean floor_container = !carried(current_container);
	char buf[BUFSZ];

	if (!current_container) {
		impossible("<in> no current_container?");
		return 0;
	} else if (obj == uball || obj == uchain) {
		You("must be kidding.");
		return 0;
	} else if (obj == current_container) {
		pline("That would be an interesting topological exercise.");
		return 0;
	} else if (obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) {
		Norep("You cannot %s something you are wearing.",
			Icebox ? "refrigerate" : "stash");
		return 0;
	} else if ((obj->otyp == LOADSTONE) && obj->cursed) {
		obj->bknown = 1;
	      pline("The stone%s won't leave your person.", plur(obj->quan));
		return 0;
	} else if (obj->otyp == AMULET_OF_YENDOR ||
		   obj->otyp == CANDELABRUM_OF_INVOCATION ||
		   obj->otyp == BELL_OF_OPENING ||
		   obj->otyp == SPE_BOOK_OF_THE_DEAD) {
	/* Prohibit Amulets in containers; if you allow it, monsters can't
	 * steal them.  It also becomes a pain to check to see if someone
	 * has the Amulet.  Ditto for the Candelabrum, the Bell and the Book.
	 */
	    pline("%s cannot be confined in such trappings.", The(xname(obj)));
	    return 0;
	}
#ifdef WALKIES
	else if (obj->otyp == LEASH && obj->leashmon != 0) {
		pline("%s is attached to your pet.", The(xname(obj)));
		return 0;
	}
#endif
	else if (obj == uwep) {
		if (welded(obj)) {
			weldmsg(obj, FALSE);
			return 0;
		}
		setuwep((struct obj *) 0);
		if (uwep) return 0;	/* unwielded, died, rewielded */
	}

	/* boxes can't fit into any container */
	if (obj->otyp == ICE_BOX || Is_box(obj)) {
		/*
		 *  xname() uses a static result array.  Save obj's name
		 *  before current_container's name is computed.  Don't
		 *  use the result of strcpy() within You() --- the order
		 *  of evaluation of the parameters is undefined.
		 */
		Strcpy(buf, the(xname(obj)));
		You("cannot fit %s into %s.", buf,
		    the(xname(current_container)));
		return 0;
	}

	freeinv(obj);

	if (is_gold) {	/* look for other gold within the container */
		for (gold = current_container->cobj; gold; gold = gold->nobj)
			if (gold->otyp == GOLD_PIECE) break;
	} else
		gold = 0;

	if (gold) {
		gold->quan += obj->quan;
	} else {
		obj->nobj = current_container->cobj;
		current_container->cobj = obj;
	}

	current_container->owt = weight(current_container);

	Strcpy(buf, the(xname(current_container)));
	You("put %s into %s.", doname(obj), buf);

	if (floor_container && costly_spot(u.ux, u.uy)) {
		sellobj_state(TRUE);
		sellobj(obj, u.ux, u.uy);
		sellobj_state(FALSE);
	}
	(void) snuff_candle(obj); /* must follow the "put" msg */
	if (Icebox && obj->otyp != OIL_LAMP && obj->otyp != BRASS_LANTERN
			&& !Is_candle(obj))
		obj->age = monstermoves - obj->age; /* actual age */

	else if (Is_mbag(current_container) && mbag_explodes(obj, 0)) {
		You("are blasted by a magical explosion!");

		/* the !floor_container case is taken care of */
		if(*u.ushops && costly_spot(u.ux, u.uy) && floor_container) {
		    register struct monst *shkp;

		    if ((shkp = shop_keeper(*u.ushops)) != 0)
			(void)stolen_value(current_container, u.ux, u.uy,
					   (boolean)shkp->mpeaceful, FALSE);
		}
		delete_contents(current_container);
		if (!floor_container)
			useup(current_container);
		else if (obj_here(current_container, u.ux, u.uy))
			useupf(current_container);
		else
			panic("in_container:  bag not found.");

		losehp(d(6,6),"magical explosion", KILLED_BY_AN);
		current_container = 0;	/* baggone = TRUE; */
	}

	if (is_gold) {
		if (gold) dealloc_obj(obj);
		bot();	/* update character's gold piece count immediately */
	}

	return(current_container ? 1 : -1);
}

STATIC_PTR int
ck_bag(obj)
struct obj *obj;
{
	return current_container && obj != current_container;
}

STATIC_PTR int
out_container(obj)
register struct obj *obj;
{
	register struct obj *otmp, *ootmp;
	boolean is_gold = (obj->otyp == GOLD_PIECE);
	int loadlev;
	long quan;

	if (!current_container) {
		impossible("<out> no current_container?");
		return -1;
	} else if (is_gold) {
		obj->owt = weight(obj);
	} else if (inv_cnt() >= 52) {
		You("have no room to hold anything else.");
		return -1;	/* skips gold too; oh well */
	}

	if(obj->oartifact && !touch_artifact(obj,&youmonst)) return 0;

	if(obj->otyp != LOADSTONE && max_capacity() + (int)obj->owt -
	   (carried(current_container) ?
	    (current_container->otyp == BAG_OF_HOLDING ?
	     (int)DELTA_CWT(current_container,obj) : (int)obj->owt) : 0) > 0) {
		char buf[BUFSZ];

		Strcpy(buf, doname(obj));
		pline("There %s %s in %s, but %s.",
			obj->quan==1 ? "is" : "are",
			buf, the(xname(current_container)),
			invent ? "you cannot carry any more"
			: "it is too heavy for you to carry");
		/* "too heavy for you to lift" is not right if you're carrying
		   the container... */
		return(0);
	}
	/* Remove the object from the list. */
	if (obj == current_container->cobj)
		current_container->cobj = obj->nobj;
	else {
		for(otmp = current_container->cobj; otmp->nobj != obj;
							    otmp = otmp->nobj)
			if(!otmp->nobj) panic("out_container");
		otmp->nobj = obj->nobj;
	}

	current_container->owt = weight(current_container);

	if (Icebox && obj->otyp != OIL_LAMP && obj->otyp != BRASS_LANTERN
			&& !Is_candle(obj))
		obj->age = monstermoves - obj->age;
	/* simulated point of time */

	if(!obj->unpaid && !carried(current_container) &&
	     costly_spot(current_container->ox, current_container->oy)) {

		addtobill(obj, FALSE, FALSE, FALSE);
	}

	quan = obj->quan;
	ootmp = addinv(obj);
	loadlev = near_capacity();
	prinv(loadlev ?
	      (loadlev < MOD_ENCUMBER ?
	       "You have a little trouble removing" :
	       "You have much trouble removing") : NULL,
	      ootmp, quan);

	if (is_gold) {
		dealloc_obj(obj);
		bot();	/* update character's gold piece count immediately */
	}
	return 1;
}

/* for getobj: allow counts, allow all types, expect food */
static const char NEARDATA frozen_food[] =
	{ ALLOW_COUNT, ALL_CLASSES, FOOD_CLASS, 0 };

int
use_container(obj, held)
register struct obj *obj;
register int held;
{
	register int cnt = 0;
	register struct obj *curr, *prev, *otmp;
	boolean one_by_one, allflag;
	char select[MAXOCLASSES+1];
	char qbuf[QBUFSZ];
	int used = 0, lcnt = 0;
	long loss = 0L;
	register struct monst *shkp;

	current_container = obj;	/* for use by in/out_container */
	if (current_container->olocked) {
		pline("%s seems to be locked.", The(xname(current_container)));
		if (held) You("must put it down to unlock.");
		return 0;
	}
	/* Count the number of contained objects. Sometimes toss objects if */
	/* a cursed magic bag.						    */
	for(curr = obj->cobj, prev = (struct obj *) 0; curr;
					    prev = curr, curr = otmp) {
	    otmp = curr->nobj;
	    if (Is_mbag(obj) && obj->cursed && !rn2(13)) {
		if (curr->known)
		    pline("%s to have vanished!", The(aobjnam(curr,"seem")));
		else
		    You("%s %s disappear.", Blind ? "notice" : "see",
							doname(curr));
		if (prev)
		    prev->nobj = otmp;
		else
		    obj->cobj = otmp;

		if(*u.ushops && (shkp = shop_keeper(*u.ushops))) {
		    if(held) {
			if(curr->unpaid)
			    loss += stolen_value(curr, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE);
			lcnt++;
		    } else if(costly_spot(u.ux, u.uy)) {
			loss += stolen_value(curr, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE);
			lcnt++;
		    }
		}
		/* obfree() will free all contained objects */
		obfree(curr, (struct obj *) 0);
	    } else
		cnt++;
	}

	if (cnt && loss)
	    You("owe %ld zorkmids for lost item%s.",
		loss, lcnt > 1 ? "s" : "");

	current_container->owt = weight(current_container);

	if(!cnt)
	    pline("%s %s is empty.", (held) ? "Your" : "The", xname(obj));
	else {
	    Sprintf(qbuf, "Do you want to take something out of %s?",
			the(xname(obj)));
ask_again:
	    switch (yn_function(qbuf, ":ynq", 'n')) {
	    case ':':
		container_contents(current_container, FALSE, FALSE);
		goto ask_again;
	    case 'y':
		if (query_classes(select, &one_by_one, &allflag, "take out",
				   current_container->cobj, FALSE, FALSE)) {
		    if (askchain((struct obj **)&current_container->cobj,
				 (one_by_one ? (char *)0 : select), allflag,
				 out_container, (int (*)())0, 0, "nodot"))
			used = 1;
		}
		/*FALLTHRU*/
	    case 'n':
		break;
	    case 'q':
	    default:
		return 0;
	    }
	}

	if (!invent && (u.ugold == 0 || Icebox)) return used;
	if (yn_function("Do you wish to put something in?", ynqchars, 'n')
	    != 'y') return used;
	if (Icebox && current_container->dknown) {
		otmp = getobj(frozen_food, "put in");
		if(!otmp || !in_container(otmp))
			flags.move = multi = 0;
	} else {
		if (query_classes(select, &one_by_one, &allflag, "put in",
					   invent, FALSE, (u.ugold != 0L))) {
		    struct obj *u_gold = (struct obj *)0;
		    if (u.ugold && (one_by_one || (allflag && !*select)
				    || index(select, GOLD_CLASS))) {
			/* make gold object & insert at head of inventory */
			u_gold = mkgoldobj(u.ugold);	/*(removes gold too)*/
			u.ugold = u_gold->quan;		/* put the gold back */
			u_gold->nobj = invent;
			invent = u_gold;
		    }
		    used = (askchain((struct obj **)&invent,
				(one_by_one ? (char *)0 : select), allflag,
				in_container, ck_bag, 0, "nodot") > 0);
		    if (u_gold && invent && invent->otyp == GOLD_PIECE) {
			/* didn't stash [all of] it */
			u_gold = invent;
			invent = u_gold->nobj;
			dealloc_obj(u_gold);
		    }
		}
	}

	return used;
}

/*pickup.c*/
