/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"hack.h"
#include	"def.trap.h"

extern struct monst *makemon();

char vowels[] = "aeiou";

char *traps[] = {
	" bear trap",
	"n arrow trap",
	" dart trap",
	" trapdoor",
	" teleportation trap",
	" pit",
	" sleeping gas trap",
	" piercer",
	" mimic"
};

dotrap(trap) register struct gen *trap; {
	nomul(0);
	if(trap->gflag&SEEN && !rn2(5))
		pline("You escape a%s.",traps[trap->gflag&037]);
	else {
		trap->gflag |= SEEN;
		switch(trap->gflag & ~SEEN) {
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
			break;
		case PIERC:
			deltrap(trap);
			if(makemon(PM_PIERC,u.ux,u.uy)) {
			  pline("A piercer suddenly drops from the ceiling!");
			  if(uarmh)
				pline("Its blow glances off your helmet.");
			  else
				(void) thitu(3,d(4,6),"falling piercer");
			}
			break;
		case ARROW_TRAP:
			pline("An arrow shoots out at you!");
			if(!thitu(8,rnd(6),"arrow")){
				mksobj_at(WEAPON_SYM, ARROW, u.ux, u.uy);
				fobj->quan = 1;
			}
			break;
		case TRAPDOOR:
			if(!xdnstair) {
pline("A trap door in the ceiling opens and a rock falls on your head!");
if(uarmh) pline("Fortunately, you are wearing a helmet!");
			losehp(uarmh ? 2 : d(2,10),"falling rock");
			} else {
			    register int newlevel = dlevel + 1;
				while(!rn2(4) && newlevel < 29)
					newlevel++;
				pline("A trap door opens up under you!");
				if(Levitation || u.ustuck) {
 				pline("For some reason you don't fall in.");
					break;
				}

				goto_level(newlevel, FALSE);
			}
			break;
		case DART_TRAP:
			pline("A little dart shoots out at you!");
			if(thitu(7,rnd(3),"little dart")) {
			    if(!rn2(6))
				poisoned("dart","poison dart");
			} else {
				mksobj_at(WEAPON_SYM, DART, u.ux, u.uy);
				fobj->quan = 1;
			}
			break;
		case TELEP_TRAP:
			newsym(u.ux,u.uy);
			tele();
			break;
		case PIT:
			if(Levitation) {
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
		default:
			pline("You hit a trap of type %d",trap->gflag);
			impossible();
		}
	}
}

mintrap(mtmp) register struct monst *mtmp; {
	register struct gen *gen = g_at(mtmp->mx, mtmp->my, ftrap);
	register int wasintrap = mtmp->mtrapped;

	if(!gen) {
		mtmp->mtrapped = 0;	/* perhaps teleported? */
	} else if(wasintrap) {
 if(!rn2(40)) mtmp->mtrapped = 0;
	} else {
	    register int tt = (gen->gflag & ~SEEN);
	    int in_sight = cansee(mtmp->mx,mtmp->my);
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
		case PIT:
			if(!index("Eyw", mtmp->data->mlet)) {
				mtmp->mtrapped = 1;
				if(in_sight)
				  pline("%s falls in a pit!", Monnam(mtmp));
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
			rloc(mtmp);
			if(in_sight && !cansee(mtmp->mx,mtmp->my))
				pline("%s suddenly disappears!",
					Monnam(mtmp));
			break;
		case ARROW_TRAP:
			if(in_sight) {
				pline("%s is hit by an arrow!",
					Monnam(mtmp));
			}
			mtmp->mhp -= 3;
			break;
		case DART_TRAP:
			if(in_sight) {
				pline("%s is hit by a dart!",
					Monnam(mtmp));
			}
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
			if(mtmp->data->mlet != 'w'){
				fall_down(mtmp);
				if(in_sight)
		pline("Suddenly, %s disappears out of sight.", monnam(mtmp));
				return(2);	/* no longer on this level */
			}
			break;
		case PIERC:
			break;
		default:
			pline("Some monster encountered an impossible trap.");
			impossible();
	    }
	}
 return(mtmp->mtrapped);
}

selftouch(arg) char *arg; {
	if(uwep && uwep->otyp == DEAD_COCKATRICE){
		pline("%s touch the dead cockatrice.", arg);
		pline("You turn to stone.");
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
 pline("You start to float in the air!");
}

float_down(){
	register struct gen *trap;
	pline("You float gently to the ground.");
	if(trap = g_at(u.ux,u.uy,ftrap))
		switch(trap->gflag & 037) {
		case PIERC:
			break;
		case TRAPDOOR:
			if(!xdnstair || u.ustuck) break;
			/* fall into next case */
		default:
			dotrap(trap);
	}
 pickup();
}

tele()
{
extern coord getpos();
coord cc;
register int nux,nuy;
	if(Teleport_control) {
		pline("To what position do you want to be teleported?");
		cc = getpos(1, "the desired position"); /* 1: force valid */
		/* possible extensions: introduce a small error if
		   magic power is low; allow transfer to solid rock */
		if(teleok(cc.x, cc.y)){
			nux = cc.x;
			nuy = cc.y;
			goto gotpos;
		}
 pline("Sorry ...");
	}
	do {
		nux = rnd(COLNO-1);
		nuy = rn2(ROWNO);
	} while(!teleok(nux, nuy));
gotpos:
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
	(void) inshop();
	pickup();
	if(!Blind) read_engr_at(u.ux,u.uy);
}

teleok(x,y) register int x,y; {
	return( isok(x,y) && levl[x][y].typ > DOOR && !m_at(x,y) &&
		!sobj_at(ENORMOUS_ROCK,x,y) && !g_at(x,y,ftrap)
	);
	/* Note: gold is permitted (because of vaults) */
}

placebc(attach) int attach; {
	if(!uchain || !uball){
		pline("Where are your chain and ball??");
		impossible();
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
register int newlevel = 5 + rn2(20);	/* 5 - 24 */
	if(dlevel == newlevel)
		if(!xdnstair) newlevel--; else newlevel++;
	goto_level(newlevel, FALSE);
}
