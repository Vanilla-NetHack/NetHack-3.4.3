/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <stdio.h>
#include <signal.h>
#include "hack.h"
#include "def.func_tab.h"

extern char *getenv(),*parse(),*getlogin(),*lowc(),*unctrl();
extern int float_down();
extern char *nomovemsg, *catmore;
extern struct obj *splitobj(), *addinv();
extern boolean hmon();

/*	Routines to do various user commands */

int done1();

dodrink() {
	register struct obj *otmp,*objs;
	register struct monst *mtmp;
	register int unkn = 0, nothing = 0;

	otmp = getobj("!", "drink");
	if(!otmp) return(0);
	switch(otmp->otyp){
	case POT_RESTORE_STRENGTH:
		unkn++;
		pline("Wow!  This makes you feel great!");
		if(u.ustr < u.ustrmax) {
			u.ustr = u.ustrmax;
			flags.botl = 1;
		}
		break;
	case POT_BOOZE:
		unkn++;
		pline("Ooph!  This tastes like liquid fire!");
		Confusion += d(3,8);
		/* the whiskey makes us feel better */
		if(u.uhp < u.uhpmax) losehp(-1, "bottle of whiskey");
		if(!rn2(4)) {
			pline("You pass out.");
			multi = -rnd(15);
			nomovemsg = "You awake with a headache.";
		}
		break;
	case POT_INVISIBILITY:
		if(Invis)
		  nothing++;
		else {
		  if(!Blind)
		    pline("Gee!  All of a sudden, you can't see yourself.");
		  else
		    pline("You feel rather airy."), unkn++;
		  newsym(u.ux,u.uy);
		}
		Invis += rn1(15,31);
		break;
	case POT_FRUIT_JUICE:
		pline("This tastes like fruit juice.");
		lesshungry(20);
		break;
	case POT_HEALING:
		pline("You begin to feel better.");
		flags.botl = 1;
		u.uhp += rnd(10);
		if(u.uhp > u.uhpmax)
			u.uhp = ++u.uhpmax;
		if(Blind) Blind = 1;	/* see on next move */
		if(Sick) Sick = 0;
		break;
	case POT_PARALYSIS:
		pline("Your feet are frozen to the floor!");
		nomul(-(rn1(10,25)));
		break;
	case POT_MONSTER_DETECTION:
		if(!fmon) {
			strange_feeling(otmp);
			return(1);
		} else {
			cls();
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				if(mtmp->mx > 0)
				at(mtmp->mx,mtmp->my,mtmp->data->mlet);
			prme();
			pline("You sense the presence of monsters.");
			more();
			docrt();
		}
		break;
	case POT_OBJECT_DETECTION:
		if(!fobj) {
			strange_feeling(otmp);
			return(1);
		} else {
		    for(objs = fobj; objs; objs = objs->nobj)
			if(objs->ox != u.ux || objs->oy != u.uy)
				goto outobjmap;
		    pline("You sense the presence of objects close nearby.");
		    break;
		outobjmap:
			cls();
			for(objs = fobj; objs; objs = objs->nobj)
				at(objs->ox,objs->oy,objs->olet);
			prme();
			pline("You sense the presence of objects.");
			more();
			docrt();
		}
		break;
	case POT_SICKNESS:
		pline("Yech! This stuff tastes like poison.");
		if(Poison_resistance)
    pline("(But in fact it was biologically contaminated orange juice.)");
		losestr(rn1(4,3));
		losehp(rnd(10), "poison potion");
		break;
	case POT_CONFUSION:
		if(!Confusion)
			pline("Huh, What?  Where am I?");
		else
			nothing++;
		Confusion += rn1(7,16);
		break;
	case POT_GAIN_STRENGTH:
		pline("Wow do you feel strong!");
		if(u.ustr == 118) break;
		if(u.ustr > 17) u.ustr += rnd(118-u.ustr);
		else u.ustr++;
		if(u.ustr > u.ustrmax) u.ustrmax = u.ustr;
		flags.botl = 1;
		break;
	case POT_SPEED:
		if(Wounded_legs) {
			if((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
				pline("Your legs feel somewhat better.");
			else
				pline("Your leg feels somewhat better.");
			Wounded_legs = 0;
			unkn++;
			break;
		}
		if(!(Fast & ~INTRINSIC))
			pline("You are suddenly moving much faster.");
		else
			pline("Your legs get new energy."), unkn++;
		Fast += rn1(10,100);
		break;
	case POT_BLINDNESS:
		if(!Blind)
			pline("A cloud of darkness falls upon you.");
		else
			nothing++;
		Blind += rn1(100,250);
		seeoff(0);
		break;
	case POT_GAIN_LEVEL: 
		pluslvl();
		break;
	case POT_EXTRA_HEALING:
		pline("You feel much better.");
		flags.botl = 1;
		u.uhp += d(2,20)+1;
		if(u.uhp > u.uhpmax)
			u.uhp = (u.uhpmax += 2);
		if(Blind) Blind = 1;
		if(Sick) Sick = 0;
		break;
	case POT_LEVITATION:
		if(!Levitation)
			float_up();
		else
			nothing++;
		Levitation += rnd(100);
		u.uprops[PROP(RIN_LEVITATION)].p_tofn = float_down;
		break;
	default:
		pline("What a funny potion! (%d)", otmp->otyp);
		impossible();
		return(0);
	}
	if(nothing) {
	    unkn++;
	    pline("You have a peculiar feeling for a moment, then it passes.");
	}
	if(otmp->dknown && !objects[otmp->otyp].oc_name_known) {
		if(!unkn) {
			objects[otmp->otyp].oc_name_known = 1;
			u.urexp += 10;
		} else if(!objects[otmp->otyp].oc_uname)
 docall(otmp);
	}
	useup(otmp);
	return(1);
}

pluslvl()
{
	register num;

	pline("You feel more experienced.");
	num = rnd(10);
	u.uhpmax += num;
	u.uhp += num;
	u.uexp = (10*pow(u.ulevel-1))+1;
	pline("Welcome to level %d.", ++u.ulevel);
	flags.botl = 1;
}

strange_feeling(obj)
register struct obj *obj;
{
	pline("You have a strange feeling for a moment, then it passes.");
	if(!objects[obj->otyp].oc_name_known && !objects[obj->otyp].oc_uname)
		docall(obj);
	useup(obj);
}

dodrop() {
	register struct obj *obj;

	obj = getobj("0$#", "drop");
	if(!obj) return(0);
	if(obj->olet == '$') {
		if(obj->quan == 0)
			pline("You didn't drop any gold pieces.");
		else {
			mkgold((int) obj->quan, u.ux, u.uy);
			pline("You dropped %u gold piece%s.",
				obj->quan, plur(obj->quan));
			if(Invis) newsym(u.ux, u.uy);
		}
		free((char *) obj);
		return(1);
	}
 return(drop(obj));
}

drop(obj) register struct obj *obj; {
	if(obj->owornmask & (W_ARMOR | W_RING)){
		pline("You cannot drop something you are wearing.");
		return(0);
	}
	if(obj == uwep) {
		if(uwep->cursed) {
			pline("Your weapon is welded to your hand!");
			return(0);
		}
 setuwep((struct obj *) 0);
	}
	pline("You dropped %s.", doname(obj));
	dropx(obj);
	return(1);
}

dropx(obj) register struct obj *obj; {
	if(obj->otyp == CRYSKNIFE)
		obj->otyp = WORM_TOOTH;
	freeinv(obj);
	obj->ox = u.ux;
	obj->oy = u.uy;
	obj->nobj = fobj;
	fobj = obj;
	if(Invis) newsym(u.ux,u.uy);
	subfrombill(obj);
	stackobj(obj);
}

/* drop several things */
doddrop() {
	return(ggetobj("drop", drop, 0));
}

rhack(cmd)
register char *cmd;
{
	register struct func_tab *tlist = list;
	boolean firsttime = FALSE;
	register res;

	if(!cmd) {
		firsttime = TRUE;
		flags.nopick = 0;
		cmd = parse();
	}
	if(!*cmd || *cmd == 0377)
		return;		/* probably we just had an interrupt */
	if(movecm(cmd)) {
	walk:
		if(multi) flags.mv = 1;
		domove();
		return;
	}
	if(movecm(lowc(cmd))) {
		flags.run = 1;
	rush:
		if(firsttime){
			if(!multi) multi = COLNO;
			u.last_str_turn = 0;
		}
		flags.mv = 1;
#ifdef QUEST
		if(flags.run >= 4) finddir();
		if(firsttime){
			u.ux0 = u.ux + u.dx;
			u.uy0 = u.uy + u.dy;
		}
#endif QUEST
		domove();
		return;
	}
	if((*cmd == 'f' && movecm(cmd+1)) ||
		movecm(unctrl(cmd))) {
		flags.run = 2;
		goto rush;
	}
	if(*cmd == 'F' && movecm(lowc(cmd+1))) {
		flags.run = 3;
		goto rush;
	}
	if(*cmd == 'm' && movecm(cmd+1)) {
		flags.run = 0;
		flags.nopick = 1;
		goto walk;
	}
	if(*cmd == 'M' && movecm(lowc(cmd+1))) {
		flags.run = 1;
		flags.nopick = 1;
		goto rush;
	}
#ifdef QUEST
	if(*cmd == cmd[1] && (*cmd == 'f' || *cmd == 'F')) {
		flags.run = 4;
		if(*cmd == 'F') flags.run += 2;
		if(cmd[2] == '-') flags.run += 1;
		goto rush;
	}
#endif QUEST
	while(tlist->f_char) {
		if(*cmd == tlist->f_char){
			res = (*(tlist->f_funct))(0);
			if(!res) {
				flags.move = 0;
				multi = 0;
			}
 return;
		}
 tlist++;
	}
	pline("Unknown command '%s'",cmd);
	multi = flags.move = 0;
}

doredraw()
{
	docrt();
	return(0);
}

dohelp()
{
	if(child(1)){
		execl(catmore,"more","help",(char *)0);
		exit(1);
	}
 return(0);
}

#ifdef SHELL
dosh(){
register char *str;
	if(child(0)) {
		(void) chdir(getenv("HOME"));
		if(str = getenv("SHELL")) execl(str,str,(char *) 0);
		if(strcmp("player", getlogin()))
			execl("/bin/sh","sh",(char *) 0);
		pline("sh: cannot execute.");
		exit(1);
	}
 return(0);
}
#endif SHELL

#ifdef BSD
#include	<sys/wait.h>
#else
#include	<wait.h>
#endif BSD

child(wt) {
register int f = fork();
	if(f == 0){		/* child */
		settty((char *) 0);
		(void) setuid(getuid());
		return(1);
	}
	if(f == -1) {	/* cannot fork */
		pline("Fork failed. Try again.");
		return(0);
	}
	/* fork succeeded; wait for child to exit */
	(void) signal(SIGINT,SIG_IGN);
	(void) signal(SIGQUIT,SIG_IGN);
	(void) wait((union wait *) 0);
	setctty();
	(void) signal(SIGINT,done1);
#ifdef WIZARD
	if(wizard) (void) signal(SIGQUIT,SIG_DFL);
#endif WIZARD
	if(wt) getret();
	docrt();
	return(0);
}

dodown()
{
	if(u.ux != xdnstair || u.uy != ydnstair) {
		pline("You can't go down here.");
		return(0);
	}
	if(u.ustuck) {
		pline("You are being held, and cannot go down.");
		return(1);
	}
	if(Levitation) {
		pline("You're floating high above the stairs.");
		return(0);
	}

	goto_level(dlevel+1, TRUE);
	return(1);
}

doup()
{
	if(u.ux != xupstair || u.uy != yupstair) {
		pline("You can't go up here.");
		return(0);
	}
	if(u.ustuck) {
		pline("You are being held, and cannot go up.");
		return(1);
	}
	if(inv_weight() + 5 > 0) {
		pline("Your load is too heavy to climb the stairs.");
		return(1);
	}

	goto_level(dlevel-1, TRUE);
	return(1);
}

goto_level(newlevel, at_stairs)
register int newlevel;
register boolean at_stairs;
{
	register fd;
	register boolean up = (newlevel < dlevel);

	if(newlevel <= 0) done("escaped");	/* in fact < 0 is impossible */
	if(newlevel == dlevel) return;		/* this cannot happen either */

	glo(dlevel);
	fd = creat(lock,FMASK);
	if(fd < 0) {
		/*
		 * This is not quite impossible: e.g., we may have
		 * exceeded our quota. If that is the case then we
		 * cannot leave this level, and cannot save either.
		 */
		pline("A mysterious force prevents you from going %d.",
			up ? "up" : "down");
		return;
	}

	if(Punished) unplacebc();
	keepdogs();
	seeoff(1);
	flags.nscrinh = 1;
	u.ux = FAR;				/* hack */
	(void) inshop();			/* probably was a trapdoor */

	savelev(fd);
	(void) close(fd);

	dlevel = newlevel;
	if(maxdlevel < dlevel)
		maxdlevel = dlevel;
	glo(dlevel);
	if((fd = open(lock,0)) < 0)
		mklev();
	else {
		(void) getlev(fd);
		(void) close(fd);
	}

	if(at_stairs) {
	    if(up) {
		u.ux = xdnstair;
		u.uy = ydnstair;
		if(!u.ux) {		/* entering a maze from below? */
		    u.ux = xupstair;	/* this will confuse the player! */
		    u.uy = yupstair;
		}
		if(Punished){
			pline("With great effort you climb the stairs");
			placebc(1);
		}
	    } else {
		u.ux = xupstair;
		u.uy = yupstair;
		if(inv_weight() + 5 > 0 || Punished){
			pline("You fall down the stairs.");
			losehp(rnd(3), "fall");
			if(Punished) {
			    if(uwep != uball && rn2(3)){
				pline("... and are hit by the iron ball");
				losehp(rnd(20), "iron ball");
			    }
			    placebc(1);
			}
 selftouch("Falling, you");
		}
  }
	} else {	/* trapdoor or level_tele */
	    do {
		u.ux = rnd(COLNO-1);
		u.uy = rn2(ROWNO);
	    } while(levl[u.ux][u.uy].typ != ROOM ||
			m_at(u.ux,u.uy));
	    if(Punished){
		if(uwep != uball && !up /* %% */ && rn2(5)){
			pline("The iron ball falls on your head.");
			losehp(rnd(25), "iron ball");
		}
		placebc(1);
	    }
	    selftouch("Falling, you");
	}
	(void) inshop();
#ifdef TRACK
	initrack();
#endif TRACK

	losedogs();
	flags.nscrinh = 0;
	setsee();
	docrt();
	pickup();
	read_engr_at(u.ux,u.uy);
}

donull() {
	return(1);	/* Do nothing, but let other things happen */
}

struct monst *bhit(), *boomhit();
dothrow()
{
	register struct obj *obj;
	register struct monst *mon;
	register tmp;

	obj = getobj("#)", "throw");	/* it is also possible to throw food */
					/* (or jewels, or iron balls ... ) */
	if(!obj || !getdir())
		return(0);
	if(obj->owornmask & (W_ARMOR | W_RING)){
		pline("You can't throw something you are wearing");
		return(0);
	}
	if(obj == uwep){
		if(obj->cursed){
			pline("Your weapon is welded to your hand");
			return(1);
		}
		if(obj->quan > 1)
			setuwep(splitobj(obj, 1));
		else
			setuwep((struct obj *) 0);
	}
	else if(obj->quan > 1)
		(void) splitobj(obj, 1);
	freeinv(obj);
	if(u.uswallow) {
		mon = u.ustuck;
		bhitpos.x = mon->mx;
		bhitpos.y = mon->my;
	} else if(obj->otyp == BOOMERANG) {
		mon = boomhit(u.dx,u.dy);
		/* boomhit delivers -1 if the thing was caught */
		if((int) mon == -1) {
			(void) addinv(obj);
			return(1);
		}
	} else
		mon = bhit(u.dx,u.dy,
			(!Punished || obj != uball) ? 8 :
				!u.ustuck ? 5 : 1,
			obj->olet);
	if(mon) {
		/* awake monster if sleeping */
		wakeup(mon);

		if(obj->olet == WEAPON_SYM) {
			tmp = -1+u.ulevel+mon->data->ac+abon();
			if(obj->otyp < ROCK) {
				if(!uwep ||
				    uwep->otyp != obj->otyp+(BOW-ARROW))
					tmp -= 4;
				else {
					tmp += uwep->spe;
				}
			} else
			if(obj->otyp == BOOMERANG) tmp += 4;
			tmp += obj->spe;
			if(u.uswallow || tmp >= rnd(20)) {
				if(hmon(mon,obj,1) == TRUE){
				  /* mon still alive */
#ifndef NOWORM
				  cutworm(mon,bhitpos.x,bhitpos.y,obj->otyp);
#endif NOWORM
				} else mon = 0;
				/* weapons thrown disappear sometimes */
				if(obj->otyp < BOOMERANG && rn2(3)) {
					/* check bill; free */
					obfree(obj, (struct obj *) 0);
					return(1);
				}
			} else miss(objects[obj->otyp].oc_name, mon);
		} else if(obj->otyp == HEAVY_IRON_BALL) {
			tmp = -1+u.ulevel+mon->data->ac+abon();
			if(!Punished || obj != uball) tmp += 2;
			if(u.utrap) tmp -= 2;
			if(u.uswallow || tmp >= rnd(20)) {
				if(hmon(mon,obj,1) == FALSE)
					mon = 0;	/* he died */
			} else miss("iron ball", mon);
		} else {
			if(cansee(bhitpos.x,bhitpos.y))
				pline("You miss %s.",monnam(mon));
			else pline("You miss it.");
			if(obj->olet == FOOD_SYM && mon->data->mlet == 'd')
				if(tamedog(mon,obj)) return(1);
			if(obj->olet == GEM_SYM && mon->data->mlet == 'u'){
			 if(obj->dknown && objects[obj->otyp].oc_name_known){
			  if(objects[obj->otyp].g_val > 0){
			    u.uluck += 5;
			    goto valuable;
			  } else {
			    pline("%s is not interested in your junk.",
				Monnam(mon));
			  }
			 } else { /* value unknown to @ */
			    u.uluck++;
			valuable:
			    pline("%s graciously accepts your gift.",
				Monnam(mon));
			    mpickobj(mon, obj);
			    rloc(mon);
			    return(1);
			 }
			}
		}
	}
	obj->ox = bhitpos.x;
	obj->oy = bhitpos.y;
	obj->nobj = fobj;
	fobj = obj;
	/* prevent him from throwing articles to the exit and escaping */
	/* subfrombill(obj); */
	stackobj(obj);
	if(Punished && obj == uball &&
		(bhitpos.x != u.ux || bhitpos.y != u.uy)){
		freeobj(uchain);
		unpobj(uchain);
		if(u.utrap){
			if(u.utraptype == TT_PIT)
				pline("The ball pulls you out of the pit!");
			else {
			    register int side =
				rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
			    pline("The ball pulls you out of the bear trap.");
			    pline("Your %s leg is severely damaged.",
				(side == LEFT_SIDE) ? "left" : "right");
			    Wounded_legs |= side + rnd(1000);
			    losehp(2, "thrown ball");
			}
			u.utrap = 0;
		}
		unsee();
		uchain->nobj = fobj;
		fobj = uchain;
		u.ux = uchain->ox = bhitpos.x - u.dx;
		u.uy = uchain->oy = bhitpos.y - u.dy;
		setsee();
		(void) inshop();
	}
	if(cansee(bhitpos.x, bhitpos.y)) prl(bhitpos.x,bhitpos.y);
	return(1);
}

getdir()
{
char buf[2];
	pline("What direction?");
	buf[0] = readchar();
	buf[1] = 0;
	return(movecm(buf));
}

/* split obj so that it gets size num */
/* remainder is put in the object structure delivered by this call */
struct obj *
splitobj(obj, num) register struct obj *obj; register int num; {
register struct obj *otmp;
	otmp = newobj(0);
	*otmp = *obj;		/* copies whole structure */
	otmp->o_id = flags.ident++;
	otmp->onamelth = 0;
	obj->quan = num;
	obj->owt = weight(obj);
	otmp->quan -= num;
	otmp->owt = weight(otmp);	/* -= obj->owt ? */
	obj->nobj = otmp;
	if(obj->unpaid) splitbill(obj,otmp);
	return(otmp);
}

char *
lowc(str)
register char *str;
{
	static char buf[2];

	if(*str >= 'A' && *str <= 'Z') *buf = *str+'a'-'A';
	else *buf = *str;
	buf[1] = 0;
	return(buf);
}

char *
unctrl(str)
register char *str;
{
	static char buf[2];
	if(*str >= ('A' & 037) && *str <= ('Z' & 037))
		*buf = *str + 0140;
	else *buf = *str;
	buf[1] = 0;
	return(buf);
}
