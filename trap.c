/*	SCCS Id: @(#)trap.c	2.1	87/10/18
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	<stdio.h>
#include	"hack.h"

extern struct monst *makemon();
#ifdef KAA
extern char *Xmonnam();
extern char *nomovemsg;
#endif

char vowels[] = "aeiou";

char *traps[] = {
	"",
	" bear trap",
	"n arrow trap",
	" dart trap",
	" trapdoor",
	" teleportation trap",
	" pit",
	" sleeping gas trap",
	" piercer",
	" mimic"
#ifdef NEWTRAPS
	," magic trap"
	," squeaky board"
#endif
#ifdef SPIDERS
	," web"
#endif
#ifdef NEWCLASS
	," spiked pit"
	," level teleporter"
#endif
#ifdef SPELLS
	," anti-magic field" 
#endif
#ifdef KAA
	," rust trap"
# ifdef RPH
	,"polymorph trap"
# endif
#endif
};

struct trap *
maketrap(x,y,typ)
register x,y,typ;
{
	register struct trap *ttmp;

	ttmp = newtrap();
	ttmp->ttyp = typ;
	ttmp->tseen = 0;
	ttmp->once = 0;
	ttmp->tx = x;
	ttmp->ty = y;
	ttmp->ntrap = ftrap;
	ftrap = ttmp;
	return(ttmp);
}

dotrap(trap) register struct trap *trap; {
	register int ttype = trap->ttyp;
	register struct monst *mtmp;

	nomul(0);
	if(trap->tseen && !rn2(5) && !(ttype == PIT
#ifdef NEWCLASS
	   || ttype == SPIKED_PIT
#endif
#ifdef SPELLS
	   || ttype == ANTI_MAGIC
#endif
		))
		pline("You escape a%s.", traps[ttype]);
	else {
		trap->tseen = 1;
		switch(ttype) {
		case SLP_GAS_TRAP:
			pline("A cloud of gas puts you to sleep!");
			nomul(-rnd(25));
			break;
		case BEAR_TRAP:
			if(Levitation) {
				pline("You float over a bear trap.");
				break;
			}
			u.utrap = 4 + rn2(4);
			u.utraptype = TT_BEARTRAP;
			pline("A bear trap closes on your foot!");
			if(u.usym=='o') pline("You howl in anger!");
			break;
		case PIERC:
			deltrap(trap);
			if(mtmp=makemon(PM_PIERCER,u.ux,u.uy)) {
			  pline("%s suddenly drops from the ceiling!", Xmonnam(mtmp));
			  if(uarmh)
				pline("Its blow glances off your helmet.");
			  else
				(void) thitu(3,d(4,6),"falling piercer");
			}
			break;
		case ARROW_TRAP:
			pline("An arrow shoots out at you!");
			if(!thitu(8,rnd(6),"arrow")){
				mksobj_at(ARROW, u.ux, u.uy);
				fobj->quan = 1;
			}
			break;
		case TRAPDOOR:
			if(!xdnstair) {
pline("A trap door in the ceiling opens and a rock falls on your head!");
if(uarmh) pline("Fortunately, you are wearing a helmet!");
			    losehp(uarmh ? 2 : d(2,10),"falling rock");
			    mksobj_at(ROCK, u.ux, u.uy);
			    fobj->quan = 1;
			    stackobj(fobj);
			    if(Invisible) newsym(u.ux, u.uy);
			} else {
			    register int newlevel = dlevel + 1;
				while(!rn2(4) && newlevel < 29)
					newlevel++;
				pline("A trap door opens up under you!");
				if(Levitation || u.ustuck) {
				pline("For some reason you don't fall in.");
					break;
				}
				fflush(stdout);
				goto_level(newlevel, FALSE);
			}
			break;
		case DART_TRAP:
			pline("A little dart shoots out at you!");
			if(thitu(7,rnd(3),"little dart")) {
			    if(!rn2(6))
				poisoned("dart","poison dart");
			} else {
				mksobj_at(DART, u.ux, u.uy);
				fobj->quan = 1;
			}
			break;
		case TELEP_TRAP:
			if(trap->once) {
				deltrap(trap);
				newsym(u.ux,u.uy);
				vtele();
			} else {
				newsym(u.ux,u.uy);
				tele();
			}
			break;
#ifdef KAA
		case RUST_TRAP:
			switch (rn2(5)) {
			case 0:
				pline("A gush of water hits you on the head!");
				if (uarmh) {
				    if (uarmh->rustfree)
					pline("Your helmet is not affected!");
				    else if (uarmh->spe > -6) {
					pline("Your helmet rusts!");
					uarmh->spe--;
				    } else
					pline("Your helmet looks quite rusted now.");
				}
				break;
			case 1:
				pline("A gush of water hits your left arm!");
				if (uarms) {
					pline("Your shield is not affected!");
					break;
				}
				if (uwep && uwep->otyp == TWO_HANDED_SWORD) goto two_hand;
				/* Two goto statements in a row--aaarrrgggh! */
		glovecheck: if(uarmg) pline("Your gloves are not affected!");
				break;
			case 2:
				pline("A gush of water hits your right arm!");
		two_hand: corrode_weapon();
				goto glovecheck;
			default:
				pline("A gush of water hits you!");
				if (uarm) {
				    if (uarm->rustfree ||
					uarm->otyp >= STUDDED_LEATHER_ARMOR) 
					    pline("Your %s not affected!",
						  aobjnam(uarm,"are"));
				    else if(uarm->spe > -6) {
					    pline("Your %s!",
						  aobjnam(uarm,"corrode"));
					    uarm->spe--;
				    } else
					    pline("Your %s quite rusted now.",
						  aobjnam(uarm, "look"));
				}
			}
			break;
#endif
		case PIT:
			if (Levitation || index("EyBfk'&",u.usym)) {
				pline("A pit opens up under you!");
				pline("You don't fall in!");
				break;
			}
			pline("You fall into a pit!");
			u.utrap = rn1(6,2);
			u.utraptype = TT_PIT;
			losehp(rnd(6),"fall into a pit");
			selftouch("Falling, you");
			break;
#ifdef NEWCLASS
		case SPIKED_PIT:
			if (Levitation || index("EyBfk'&",u.usym)) {
				pline("A pit opens up under you!");
				pline("You don't fall in!");
				pline("There are spikes in that pit!!!");
				break;
			}
			pline("You fall into a pit!");
			pline("You land on a set of sharp iron spikes!");
			u.utrap = rn1(6,2);
			u.utraptype = TT_PIT;
			losehp(rnd(10),"fall onto iron spikes");
			if(!rn2(6)) poisoned("spikes","poison spikes");
			selftouch("Falling, you");
			break;
		case LEVEL_TELEP:
			if (!Blind)	pline("You are momentarily blinded by a flash of light");
			else		pline("You are momentarily disoriented.");
			deltrap(trap);
			newsym(u.ux,u.uy);
			level_tele();
			break;
#endif
#ifdef SPELLS
		case ANTI_MAGIC:
			pline("You feel your magical energy drain away!");
			u.uen -= (rnd(u.ulevel) + 1);
			if(u.uen < 0)  {
				u.uenmax += u.uen;
				if(u.uenmax < 0) u.uenmax = 0;
				u.uen = 0;
			}
			flags.botl = 1;
			break;
#endif
#if defined(RPH) && defined(KAA)
		case POLY_TRAP:
			    pline("You feel a change coming over you.");
			    polyself();
			    deltrap(trap);
			    break;
#endif
#ifdef NEWTRAPS
		case MGTRP:
			/* A magic trap. */
			domagictrap();
			break;
		case SQBRD: {
#include      "edog.h"
			register struct monst *mtmp = fmon;
			/* Stepped on a squeaky board. */
			pline("A board underfoot gives off a loud squeak!");
			/* Wake up nearby monsters. */
		       while(mtmp) {
			 if(dist(mtmp->mx,mtmp->my) < u.ulevel*20) {
			       if(mtmp->msleep)
				       mtmp->msleep = 0;
			       if(mtmp->mtame)
				       EDOG(mtmp)->whistletime = moves;
			 }
			 mtmp = mtmp->nmon;
		       }
		}
			break;
#endif
#ifdef SPIDERS
	       case WEB:

		       /* Our luckless adventurer has stepped into a web. */

		       pline("You've stumbled into a spider web!");
		       u.utraptype = TT_WEB;

		       /* Time stuck in the web depends on your strength. */

		       if (u.ustr == 3) u.utrap = rn1(6,6);
		       else if (u.ustr < 6) u.utrap = rn1(6,4);
		       else if (u.ustr < 9) u.utrap = rn1(4,4);
		       else if (u.ustr < 12) u.utrap = rn1(4,2);
		       else if (u.ustr < 15) u.utrap = rn1(2,2);
		       else if (u.ustr < 18) u.utrap = rnd(2);
		       else if (u.ustr < 69) u.utrap = 1;
		       else {
			       u.utrap = 0;
			       pline("You tear through the web!");
			       deltrap(trap);
			    }
		       break;
#endif
		default:
			impossible("You hit a trap of type %u", trap->ttyp);
		}
	}
}

mintrap(mtmp) register struct monst *mtmp; {
	register struct trap *trap = t_at(mtmp->mx, mtmp->my);
	register int wasintrap = mtmp->mtrapped;

	if(!trap) {
		mtmp->mtrapped = 0;	/* perhaps teleported? */
	} else if(wasintrap) {
		if(!rn2(40)) mtmp->mtrapped = 0;
	} else {
	    register int tt = trap->ttyp;
#ifdef DGK
	/* A bug fix for dumb messages by ab@unido.
	 */
	    int in_sight = cansee(mtmp->mx,mtmp->my)
			   && (!mtmp->minvis || See_invisible);
#else
	    int in_sight = cansee(mtmp->mx,mtmp->my);
#endif
	    extern char mlarge[];

	    if(mtmp->mtrapseen & (1 << tt)) {
		/* he has been in such a trap - perhaps he escapes */
		if(rn2(4)) return(0);
	    }
	    mtmp->mtrapseen |= (1 << tt);
	    switch (tt) {
		case BEAR_TRAP:
			if(index(mlarge, mtmp->data->mlet)) {
				if(in_sight)
				  pline("%s is caught in a bear trap!",
					Monnam(mtmp));
				else
				  if(mtmp->data->mlet == 'o')
			    pline("You hear the roaring of an angry bear!");
				mtmp->mtrapped = 1;
			}
			break;
#ifdef KAA
# ifdef RPH
  		case POLY_TRAP:
		    if(!resist(mtmp, '/', 0, NOTELL))
			newcham(mtmp,&mons[rn2(CMNUM)]);
		    break;
# endif
		case RUST_TRAP:
			if(in_sight)
				pline("A gush of water hits %s!",monnam(mtmp));
			break;
#endif
		case PIT:
#ifdef NEWCLASS
		case SPIKED_PIT:
#endif
			/* there should be a mtmp/data -> floating */
			if(!index("EywBIfk'& ", mtmp->data->mlet)) { /* ab */
				mtmp->mtrapped = 1;
				if(in_sight)
				  pline("%s falls into a pit!", Monnam(mtmp));
			}
			break;
		case SLP_GAS_TRAP:
			if(!mtmp->msleep && !mtmp->mfroz) {
				mtmp->msleep = 1;
				if(in_sight)
				  pline("%s suddenly falls asleep!",
					Monnam(mtmp));
			}
			break;
		case TELEP_TRAP:
#ifdef NEWCLASS
		case LEVEL_TELEP:
#endif
			rloc(mtmp);
			if(in_sight && !cansee(mtmp->mx,mtmp->my))
				pline("%s suddenly disappears!",
					Monnam(mtmp));
			break;
		case ARROW_TRAP:
			if(in_sight)
				pline("%s is hit by an arrow!",	Monnam(mtmp));
			mtmp->mhp -= 3;
			break;
		case DART_TRAP:
			if(in_sight)
				pline("%s is hit by a dart!", Monnam(mtmp));
			mtmp->mhp -= 2;
			/* not mondied here !! */
			break;
		case TRAPDOOR:
			if(!xdnstair) {
				mtmp->mhp -= 10;
				if(in_sight)
pline("A trap door in the ceiling opens and a rock hits %s!", monnam(mtmp));
				break;
			}
			if(!index("EywBIfk", mtmp->data->mlet)){
				fall_down(mtmp);
				if(in_sight)
		pline("Suddenly, %s disappears out of sight.", monnam(mtmp));
				return(2);	/* no longer on this level */
			}
			break;
		case PIERC:
			break;
#ifdef NEWTRAPS
		case MGTRP:
			/* A magic trap.  Monsters immune. */
			break;
		case SQBRD: {
			register struct monst *ztmp = fmon;
			
			if(index("EyBIfk", mtmp->data->mlet)) break;
			/* Stepped on a squeaky board. */
			if (in_sight)
			   pline("%s steps on a squeaky board.", Monnam(mtmp));
			else
			   pline("You hear a distant squeak.");
			/* Wake up nearby monsters. */
		       while(ztmp) {
			 if(dist2(mtmp->mx,mtmp->my,ztmp->mx,ztmp->my) < 40)
			       if(ztmp->msleep) ztmp->msleep = 0;
			 ztmp = ztmp->nmon;
		       }
			break;
		}
#endif
#ifdef SPIDERS
	       case WEB:
		       /* Monster in a web. */
			/* in_sight check and confused bear by Eric Backus */
		       if(mtmp->data->mlet != 's') {
			 if(in_sight)
				pline("%s is caught in a web!", Monnam(mtmp));
			  else
			    if(mtmp->data->mlet == 'o')
			      pline("You hear the roaring of a confused bear!");
			 mtmp->mtrapped = 1;
		       }
		      break;
#endif
#ifdef SPELLS
		case ANTI_MAGIC:	break;
#endif
		default:
			impossible("Some monster encountered a strange trap of type %d.",tt);
	    }
	}
	return(mtmp->mtrapped);
}

selftouch(arg) char *arg; {
	if(uwep && uwep->otyp == DEAD_COCKATRICE){
		pline("%s touch the dead cockatrice.", arg);
		pline("You turn to stone.");
		pline("You die...");
		killer = objects[uwep->otyp].oc_name;
		done("died");
	}
}

float_up(){
	if(u.utrap) {
		if(u.utraptype == TT_PIT) {
			u.utrap = 0;
			pline("You float up, out of the pit!");
		} else {
			pline("You float up, only your leg is still stuck.");
		}
	} else
		if (Hallucination)
			pline("Oh wow!  You're floating in the air!");
		else
			pline("You start to float in the air!");
}

float_down(){
	register struct trap *trap;
	
	/* check for falling into pool - added by GAN 10/20/86 */
	if(IS_POOL(levl[u.ux][u.uy].typ) && !Levitation)
		drown();

	pline("You float gently to the ground.");
	if(trap = t_at(u.ux,u.uy))
		switch(trap->ttyp) {
		case PIERC:
			break;
		case TRAPDOOR:
			if(!xdnstair || u.ustuck) break;
			/* fall into next case */
		default:
			dotrap(trap);
	}
	pickup(1);
}

#include "mkroom.h"

vtele() {
	register struct mkroom *croom;

	for(croom = &rooms[0]; croom->hx >= 0; croom++)
	    if(croom->rtype == VAULT) {
		register x,y;

		x = rn2(2) ? croom->lx : croom->hx;
		y = rn2(2) ? croom->ly : croom->hy;
		if(teleok(x,y)) {
		    teleds(x,y);
		    return;
		}
	    }
	tele();
}

#ifdef BVH
int has_amulet() {
    register struct  obj *otmp;

    for(otmp = invent; otmp; otmp = otmp->nobj)
	if(otmp->olet == AMULET_SYM && otmp->spe >= 0)
	    return(1);
    return(0);
}
#endif

tele() {
	coord cc;
	register int nux,nuy;

#ifdef BVH
	if(has_amulet() && rn2(3)) {
	    pline("You feel disoriented for a moment.");
	    return;
	}
#endif
	if(Teleport_control) {
#ifdef KAA
	    if (multi < 0 && (!nomovemsg ||
			      !strncmp(nomovemsg,"You awake", 9) ||
			      !strncmp(nomovemsg,"You regain con", 15) ||
			      !strncmp(nomovemsg,"You are consci", 15)))
 
		pline("Being unconscious, you cannot control your teleport.");
	    else {
#endif
	
		    pline("To what position do you want to be teleported?");
		    getpos(&cc, 1, "the desired position"); /* 1: force valid */
		    /* possible extensions: introduce a small error if
		       magic power is low; allow transfer to solid rock */
		    if(teleok(cc.x, cc.y)){
			teleds(cc.x, cc.y);
			return;
		    }
		    pline("Sorry ...");
#ifdef KAA
		}
#endif
	}
	do {
		nux = rnd(COLNO-1);
		nuy = rn2(ROWNO);
	} while(!teleok(nux, nuy));
	teleds(nux, nuy);
}

teleds(nux, nuy)
register int nux,nuy;
{
	if(Punished) unplacebc();
	unsee();
	u.utrap = 0;
	u.ustuck = 0;
	u.ux = nux;
	u.uy = nuy;
	setsee();
	if(Punished) placebc(1);
	if(u.uswallow){
		u.uswldtim = u.uswallow = 0;
		docrt();
	}
	nomul(0);
	if(IS_POOL(levl[nux][nuy].typ) && !Levitation)
		drown();
	(void) inshop();
	pickup(1);
	read_engr_at(u.ux,u.uy);
}

teleok(x,y) register int x,y; {	/* might throw him into a POOL
				 * removed by GAN 10/20/86
				 */
#ifdef STUPID
	boolean	tmp1, tmp2, tmp3;
	tmp1 = isok(x,y) && !IS_ROCK(levl[x][y].typ) && !m_at(x,y);
	tmp2 = !sobj_at(ENORMOUS_ROCK,x,y) && !t_at(x,y);
	tmp3 = !(IS_POOL(levl[x][y].typ) && !Levitation);
	return(tmp1 && tmp2 && tmp3);
#else
	return( isok(x,y) && !IS_ROCK(levl[x][y].typ) && !m_at(x,y) &&
		!sobj_at(ENORMOUS_ROCK,x,y) && !t_at(x,y) &&
		!(IS_POOL(levl[x][y].typ) && !Levitation)
	);
#endif
	/* Note: gold is permitted (because of vaults) */
}

dotele() {
	extern char pl_character[];

	if((!index("LNt",u.usym)) &&
#ifdef WIZARD
	   !wizard &&
#endif
		      (!Teleportation || u.ulevel < 6 ||
			(pl_character[0] != 'W' && u.ulevel < 10))) {
		pline("You are not able to teleport at will.");
		return(0);
	}
	if(u.uhunger <= 100 || u.ustr < 6) {
		pline("You miss the strength for a teleport spell.");
#ifdef WIZARD
		if(!wizard)
#endif
		return(1);
	}
	tele();
	morehungry(100);
	return(1);
}

placebc(attach) int attach; {
	if(!uchain || !uball){
		impossible("Where are your chain and ball??");
		return;
	}
	uball->ox = uchain->ox = u.ux;
	uball->oy = uchain->oy = u.uy;
	if(attach){
		uchain->nobj = fobj;
		fobj = uchain;
		if(!carried(uball)){
			uball->nobj = fobj;
			fobj = uball;
		}
	}
}

unplacebc(){
	if(!carried(uball)){
		freeobj(uball);
		unpobj(uball);
	}
	freeobj(uchain);
	unpobj(uchain);
}

level_tele() {
register int newlevel;

#ifdef BVH
	if(has_amulet() && rn2(5)) {
	    pline("You feel very disoriented for a moment.");
	    return;
	}
#endif
	if(Teleport_control) {
	    char buf[BUFSZ];

	    do {
	      pline("To what level do you want to teleport? [type a number] ");
	      getlin(buf);
	    } while(!digit(buf[0]) && (buf[0] != '-' || !digit(buf[1])));
	    newlevel = atoi(buf);
	} else {
#ifdef DGKMOD
	    newlevel = rn2(5) | !Fire_resistance ? rnz(dlevel + 3) : 30;
#else
	    newlevel = rnz(dlevel + 3);			/* 5 - 24 */
#endif
	    if(dlevel == newlevel)
		if(!xdnstair) newlevel--; else newlevel++;
	}
	if(newlevel >= 30) {
	    if(newlevel > MAXLEVEL) newlevel = MAXLEVEL;
	    pline("You arrive at the center of the earth ...");
	    pline("Unfortunately it is here that hell is located.");
#ifdef DGK
	    fflush(stdout);
#endif
	    if(Fire_resistance) {
		pline("But the fire doesn't seem to harm you.");
	    } else {
		pline("You burn to a crisp.");
		pline("You die...");
		dlevel = maxdlevel = newlevel;
		killer = "visit to hell";
		done("burned");
	    }
	}
	if(newlevel < 0) {
		if(newlevel <= -10) {
			pline("You arrive in heaven.");
			pline("\"You are here a bit early, but we'll let you in.\"");
			killer = "visit to heaven";
			done("died");
		} else	if (newlevel == -9) {
			pline("You feel deliriously happy. ");
			pline("(In fact, you're on Cloud 9!) ");
			more();
		} else	newlevel = 0;
	    pline("You are now high above the clouds ...");
	    if(Levitation) {
		pline("You float gently down to earth.");
		done("escaped");
	    }
	    pline("Unfortunately, you don't know how to fly.");
	    pline("You plummet a few thousand feet to your death.");
	    dlevel = 0;
	    killer = "long fall";
	    done("died");
	}

	goto_level(newlevel, FALSE); /* calls done("escaped") if newlevel==0 */
}

#ifdef NEWTRAPS

domagictrap()
{
	register int fate = rnd(20);

	/* What happened to the poor sucker? */

	if (fate < 10) {

	  /* Most of the time, it creates some monsters. */
	  register int cnt = rnd(4);

	  /* below checks for blindness added by GAN 10/30/86 */
	  if (!Blind)  {
		pline("You are momentarily blinded by a flash of light!");
		Blinded += rn1(5,10);
		seeoff(0);
	  }  else
		pline("You hear a deafening roar!");
	  while(cnt--)
	   (void) makemon((struct permonst *) 0, u.ux, u.uy);
	}
	else
	  switch (fate) {

	     case 10:
	     case 11:
		      /* sometimes nothing happens */
			break;
	     case 12:
		      /* a flash of fire */
		      {
			register int num;
			
			/* changed to be in conformance with
			 * SCR_FIRE by GAN 11/02/86
			 */
			
			pline("A tower of flame bursts from the floor!");
			if(Fire_resistance)
				pline("You are uninjured.");
			else {
				num = rnd(6);
				u.uhpmax -= num;
				losehp(num,"a burst of flame");
				break;
			}
		      }

	     /* odd feelings */
	     case 13:   pline("A shiver runs up and down your spine!");
			break;
	     case 14:   pline("You hear distant howling.");
			break;
	     case 15:   pline("You suddenly yearn for your distant homeland.");
			break;
	     case 16:   pline("Your pack shakes violently!");
			break;

	     /* very occasionally something nice happens. */

	     case 19:
		    /* tame nearby monsters */
		   {   register int i,j;
		       register boolean confused = (Confusion != 0);
		       register int bd = confused ? 5 : 1;
		       register struct monst *mtmp;

		       /* below pline added by GAN 10/30/86 */
		       pline("You feel charismatic.");
		       for(i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++)
		       if(mtmp = m_at(u.ux+i, u.uy+j))
			       (void) tamedog(mtmp, (struct obj *) 0);
		       break;
		   }

	     case 20:
		    /* uncurse stuff */
		   {  register struct obj *obj;
		      register boolean confused = (Confusion != 0);

			/* below plines added by GAN 10/30/86 */
			if (confused)
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
	     default: break;
	  }
}
#endif /* NEWTRAPS /**/

drown()
{
	pline("You fall into a pool!");
	pline("You can't swim!");
	if(
#ifdef WIZARD
	wizard ||
#endif
	rn2(3) < u.uluck+2) {
		/* most scrolls become unreadable */
		register struct obj *obj;

		for(obj = invent; obj; obj = obj->nobj)
			if(obj->olet == SCROLL_SYM && rn2(12) > u.uluck)
				obj->otyp = SCR_BLANK_PAPER;
		/* we should perhaps merge these scrolls ? */

		pline("You attempt a teleport spell.");	/* utcsri!carroll */
		(void) dotele();
		if(!IS_POOL(levl[u.ux][u.uy].typ)) return;
	}
	pline("You drown.");
	pline("You die...");
	killer = "pool of water";
	done("drowned");
}
