/*	SCCS Id: @(#)fountain.c	2.3	88/01/21
/* fountain.c  v 1.4.4 */

/*
 * Revision 1.4.4  88/02/11  08:31:00  M. Stephenson
 * Implemented "coins" fixes by woodbury@bme.unc.edu
 * Fixed minor bugs.
 *
 * Revision 1.4.3  87/11/25  19:16:00  M. Stephenson
 * Implemented levitation bug fixes.
 *
 * Revision 1.4.3  87/11/25  19:16:00  M. Stephenson
 * Implemented levitation bug fixes.
 *
 * Revision 1.4.2  87/10/19  11:48:00  M. Stephenson
 * Implementation of KJS bug fixes.
 *
 * Revision 1.4.1  87/05/20  11:53:00  M. Stephenson
 * Implementation of KAA bug fixes.
 *
 * Revision 1.4    87/05/04  17:39:00  M. Stephenson
 * Integration of independent modifications
 *
 * Revision 1.3    87/03/02            Eric Backus
 * Rearranged, and dipfountain added
 * 
 * Revision 1.2    87/03/01  13:59:59  gil
 * patches
 * 
 * Revision 1.1    87/02/11  15:14:10  gil
 * Initial revision
 *
 */

/* Code for drinking from fountains.   */
/* Scott R. Turner, srt@ucla, 10/27/86 */

#include "hack.h"

extern struct monst *mkmon_at();
extern struct obj *mkobj_at();
extern char genocided[];

#ifdef FOUNTAINS
#define somex() ((int)(rand()%(croom->hx-croom->lx+1))+croom->lx)
#define somey() ((int)(rand()%(croom->hy-croom->ly+1))+croom->ly)

dowatersnakes() /* Fountain of snakes! */ {
	register int num = rnd(6);
	if (!index(genocided, 'S')) {

		pline("Good Lord!  An endless stream of snakes pours forth!");
		while(num-- > 0) (void) mkmon_at('S',u.ux,u.uy);
	} else
		pline("The fountain bubbles furiously for a moment, then calms.");
}

dowaterdemon() /* Water demon */ {
register struct monst *mtmp;

	if((mtmp = mkmon_at('&',u.ux,u.uy))) {
	    pline("You have unleashed a water demon!");

	/* Give those on low levels a (slightly) better chance of survival */
	    if ( rnd(100) > (80 + dlevel)) {
		pline("Grateful for his release, he grants you a wish!");
		makewish();
		mondied(mtmp);
	    }
	}
}

dowaternymph() /* Water Nymph */ {
	register struct monst *mtmp;
	if((mtmp = mkmon_at('N',u.ux,u.uy))) {

		pline("You have attracted a water nymph!");
		mtmp->msleep = 0;
	} else
		pline("A large bubble rises to the surface and pops.");
}

#include	"mkroom.h"

dogushforth() /* Gushing forth in this room */ {
register int num = rnd(10);
register xchar mx,my;
register int tryct = 0;
register int uroom = inroom(u.ux, u.uy);
register struct mkroom *croom = &rooms[uroom];
register int madepool = 0;

	if(croom->hx < 0 || has_upstairs(croom) ||
	   has_dnstairs(croom))  {
		pline("Your thirst is quenched.");
		return;
	}
	while(num--) {
	    do {
		if(++tryct > 200)  {
		    if(madepool)
			pline("Water gushes forth from the overflowing fountain!");
		    else
			pline("Your thirst is quenched.");
		    return;
		}
		mx = somex();
		my = somey();
	    } while(nexttodoor(mx,my) || !((mx+my)%2) ||
		    (mx == u.ux && my == u.uy) ||
		    (IS_POOL(levl[mx][my].typ)));
		       
	    /* Put a pool at mx, my */
		     
	    levl[mx][my].typ = POOL;
	    atl(mx,my,POOL_SYM);
	    madepool = 1;
	}

	pline("Water gushes forth from the overflowing fountain!");
}

dofindgem() /* Find a gem in the sparkling waters. */ {

	if (!Blind) pline("You spot a gem in the sparkling waters!");
	mkobj_at('*',u.ux,u.uy);
}

dryup(){
	if (!rn2(3) && (levl[u.ux][u.uy].typ == FOUNTAIN)) {
		pline("The fountain dries up!");
		levl[u.ux][u.uy].typ = ROOM;
		if(Invis) newsym(u.ux, u.uy);
	}
}

drinkfountain() {

	/* What happens when you drink from a fountain? */
	register int fate = rnd(30);

	if(Levitation) 	pline("You are floating high above the fountain.");
	else if (fate < 10) {
		pline("The cool draught refreshes you.");
		lesshungry(rnd(10));
	} else {
	    switch (fate) {

		case 20: /* Foul water */

			pline("The water is foul!  You gag and vomit.");
			morehungry(rnd(20)+10);
			if(Sick)  {
				Sick = 0;
				pline("What a relief!");
			}
			break;

		case 21: /* Poisonous */

			pline("The water is contaminated!");
			if (Poison_resistance) {
			   pline("Perhaps it is run off from the nearby orange farm.");
			   losehp(rnd(4),"unrefrigerated orange juice");
			   break;
			}
			losestr(rn1(4,3));
			losehp(rnd(10),"contaminated water");
			break;
	
		case 22: /* Fountain of snakes! */
			dowatersnakes();
			break;

		case 23: /* Water demon */
			dowaterdemon();
			break;

		case 24: /* Curse an item... */ {
			register struct obj *obj;

			pline("This water's no good!");
			morehungry(rnd(20)+10);
			for(obj = invent; obj ; obj = obj->nobj)
				if (!rn2(5))	obj->cursed++;
			break;
			}
			 
		case 25: /* See invisible */

			pline("You see an image of someone stalking you.");
			pline("But it disappears.");
			HSee_invisible |= INTRINSIC;
			break;

		case 26: /* See Monsters */ {
			register struct monst *mtmp;

			  if(!fmon) pline("You feel oddly disturbed.");
			  else {
			    cls();
			    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				if(mtmp->mx > 0)
				    at(mtmp->mx,mtmp->my,mtmp->data->mlet);
			    prme();
			    pline("You sense the presence of monsters.");
			    more();
			    docrt();
			  }
			}
			break;

		case 27: /* Find a gem in the sparkling waters. */
			dofindgem();
			break;

		case 28: /* Water Nymph */
			dowaternymph();
			break;

		case 29: /* Scare */ {
			register struct monst *mtmp;

			pline("This water gives you bad breath!");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) 
				mtmp->mflee = 1;
			}
			break;

		case 30: /* Gushing forth in this room */
			dogushforth();
			break;
		default:
			break;
	    }
	}
	dryup();
}

dipfountain(obj)
register struct obj *obj;
{
	register int fate = rnd(30);

	if(Levitation) 	pline("You are floating high above the fountain.");
	else if(fate<10)
		if(!obj->rustfree &&
			/* Only swords affected here */
			(obj->otyp == LONG_SWORD ||
			obj->otyp == KATANA ||
			obj->otyp == BROAD_SWORD ||
			obj->otyp == SHORT_SWORD ||
			obj->otyp == TWO_HANDED_SWORD)) {
			if(obj->spe > -6) {
				pline("Your weapon rusts somewhat.");
				obj->spe--;
			} else pline("Your weapon looks quite rusted.");
		} else pline("Well, it looks wet now.");
	else if(fate<14)
		if(obj->otyp == LONG_SWORD
#ifndef RPH
		   && !strcmp(ONAME(obj), "Excalibur")
#endif
		) {
			/* The lady of the lake acts! - Eric Backus */
			/* Be *REAL* nice to him */
	pline("A murky hand from the depths reaches up to bless the sword.");
	pline("As the hand retreats, the fountain disappears!");
#ifndef RPH
			if(obj->spe < 5) obj->spe = 5;
#else
			/* otherwise +rnd(10) / +5 "Super"sword */
			oname(obj, "Excalibur");
#endif
#ifdef KAA
			obj->dknown = 1;	/* blessed */
#endif
			obj->cursed = 0;
			obj->rustfree = 1;
			levl[u.ux][u.uy].typ = ROOM;
			if(Invis) newsym(u.ux, u.uy);
			return(0);
		} else pline ("Well, it looks wet now.");
	else {
	    switch (fate) {
		case 16: /* Curse the item */
			pline("Well, it looks wet now.");
			obj->cursed = 1;
			break;
		case 17:
		case 18:
		case 19:
		case 20: /* Uncurse the item */
			if(obj->cursed) {
			    pline("The water glows for a moment.");
			    obj->cursed = 0;
			} else {
			    pline("A feeling of loss comes over you.");
			}
			break;
		case 21: /* Water Demon */
			dowaterdemon();
			break;
		case 22: /* Water Nymph */
			dowaternymph();
			break;
		case 23: /* An Endless Stream Of Snakes */
			dowatersnakes();
			break;
		case 24: /* Find a gem */
			dofindgem();
			break;
		case 25: /* Water gushes forth */
			dogushforth();
			break;
		case 26: /* Strange feeling */
			pline("A strange tingling runs up your arm.");
			break;
		case 27: /* Strange feeling */
			pline("You feel a sudden chill.");
			break;
		case 28: /* Strange feeling */
		pline("An urge to take a bath overwhelms you.");
			if (u.ugold > 10) {
			     	u.ugold -= somegold()/10;
			  pline("You lost some of your gold in the fountain!");
	 		}
			break;
		case 29: /* You see coins */

		/* We make fountains have more coins the closer you are to the
		 * surface.  After all, there will have been more people going
		 * by.  Just like a shopping mall!  Chris Woodbury  */

			mkgold((long)(rnd((MAXLEVEL-dlevel)*2)+5),u.ux,u.uy);
		pline("Far below you, you see coins glistening in the water.");
			break;
		default:
			break;
	    }
	}
	dryup();
	return(0);
}
#endif
