/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* hack.fight.c version 1.0.1 - corrected symbol of lurker above */

#include	"hack.h"
extern struct permonst li_dog, dog, la_dog;
extern char *exclam(), *xname();

static boolean far_noise;
static long noisetime;

/* hitmm returns 0 (miss), 1 (hit), or 2 (kill) */
hitmm(magr,mdef) register struct monst *magr,*mdef; {
register struct permonst *pa = magr->data, *pd = mdef->data;
int hit;
schar tmp;
boolean vis;
	if(index("Eauy", pa->mlet)) return(0);
	if(magr->mfroz) return(0);		/* riv05!a3 */
	tmp = pd->ac + pa->mlevel;
	if(mdef->mconf || mdef->mfroz || mdef->msleep){
		tmp += 4;
		if(mdef->msleep) mdef->msleep = 0;
	}
	hit = (tmp > rnd(20));
	if(hit) mdef->msleep = 0;
	vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my));
	if(vis){
		char buf[BUFSZ];
		if(mdef->mimic) seemimic(mdef);
		if(magr->mimic) seemimic(magr);
		(void) sprintf(buf,"%s %s", Monnam(magr),
			hit ? "hits" : "misses");
		pline("%s %s.", buf, monnam(mdef));
	} else {
		boolean far = (dist(magr->mx, magr->my) > 15);
		if(far != far_noise || moves-noisetime > 10) {
			far_noise = far;
			noisetime = moves;
			pline("You hear some noises%s.",
				far ? " in the distance" : "");
		}
	}
	if(hit){
		if(magr->data->mlet == 'c' && !magr->cham) {
			magr->orig_hp += 3;
			if(vis) pline("%s is turned to stone!", Monnam(mdef));
			else if(mdef->mtame)
     pline("You have a peculiarly sad feeling for a moment, then it passes.");
			monstone(mdef);
			hit = 2;
		} else
		if((mdef->mhp -= d(pa->damn,pa->damd)) < 1) {
			magr->orig_hp += 1 + rn2(pd->mlevel+1);
			if(magr->mtame && magr->orig_hp > 8*pa->mlevel){
				if(pa == &li_dog) magr->data = pa = &dog;
				else if(pa == &dog) magr->data = pa = &la_dog;
			}
			if(vis) pline("%s is killed!", Monnam(mdef));
			else if(mdef->mtame)
		pline("You have a sad feeling for a moment, then it passes.");
			mondied(mdef);
			hit = 2;
		}
	}
 return(hit);
}

/* drop (perhaps) a cadaver and remove monster */
mondied(mdef) register struct monst *mdef; {
register struct permonst *pd = mdef->data;
		if(letter(pd->mlet) && rn2(3)){
			mksobj_at(pd->mlet,CORPSE,mdef->mx,mdef->my);
			if(cansee(mdef->mx,mdef->my)){
				unpmon(mdef);
				atl(mdef->mx,mdef->my,fobj->olet);
			}
 stackobj(fobj);
		}
 mondead(mdef);
}

/* drop a rock and remove monster */
monstone(mdef) register struct monst *mdef; {
	extern char mlarge[];
	if(index(mlarge, mdef->data->mlet))
		mksobj_at(ROCK_SYM, ENORMOUS_ROCK, mdef->mx, mdef->my);
	else
		mksobj_at(WEAPON_SYM, ROCK, mdef->mx, mdef->my);
	if(cansee(mdef->mx, mdef->my)){
		unpmon(mdef);
		atl(mdef->mx,mdef->my,fobj->olet);
	}
 mondead(mdef);
}
		

fightm(mtmp) register struct monst *mtmp; {
register struct monst *mon;
	for(mon = fmon; mon; mon = mon->nmon) if(mon != mtmp) {
		if(DIST(mon->mx,mon->my,mtmp->mx,mtmp->my) < 3)
		    if(rn2(4))
			return(hitmm(mtmp,mon));
	}
 return(-1);
}

hitu(mtmp,dam)
register struct monst *mtmp;
register dam;
{
	register tmp;

	if(u.uswallow) return(0);

	if(mtmp->mhide && mtmp->mundetected) {
		mtmp->mundetected = 0;
		if(!Blind) {
			register struct obj *obj;
			extern char * Xmonnam();
			if(obj = o_at(mtmp->mx,mtmp->my))
				pline("%s was hidden under %s!",
					Xmonnam(mtmp), doname(obj));
		}
	}

	tmp = u.uac;
	/* give people with Ac = -10 at least some vulnerability */
	if(tmp < 0) {
		dam += tmp;		/* decrease damage */
		if(dam <= 0) dam = 1;
		tmp = -rn2(-tmp);
	}
	tmp += mtmp->data->mlevel;
	if(multi < 0) tmp += 4;
	if(Invis || !mtmp->mcansee) tmp -= 2;
	if(mtmp->mtrapped) tmp -= 2;
	if(tmp <= rnd(20)) {
		if(Blind) pline("It misses.");
		else pline("%s misses.",Monnam(mtmp));
		return(0);
	}
	if(Blind) pline("It hits!");
	else pline("%s hits!",Monnam(mtmp));
	losehp_m(dam, mtmp);
	return(1);
}

/* u is hit by sth, but not a monster */
thitu(tlev,dam,name)
register tlev,dam;
register char *name;
{
char buf[BUFSZ];
	setan(name,buf);
	if(u.uac + tlev <= rnd(20)) {
		if(Blind) pline("It misses.");
		else pline("You are almost hit by %s!", buf);
		return(0);
	} else {
		if(Blind) pline("You are hit!");
		else pline("You are hit by %s!", buf);
		losehp(dam,name);
		return(1);
	}
}

char mlarge[] = "bCDdegIlmnoPSsTUwY\',&";

boolean
hmon(mon,obj,thrown)	/* return TRUE if mon still alive */
register struct monst *mon;
register struct obj *obj;
register thrown;
{
	register tmp;

	if(!obj){
		tmp = rnd(2);	/* attack with bare hands */
		if(mon->data->mlet == 'c' && !uarmg){
			pline("You hit the cockatrice with your bare hands");
			pline("You turn to stone ...");
			done_in_by(mon);
		}
	} else if(obj->olet == WEAPON_SYM) {
	    if(obj == uwep && (obj->otyp > SPEAR || obj->otyp < BOOMERANG))
		tmp = rnd(2);
	    else {
		if(index(mlarge, mon->data->mlet)) {
			tmp = rnd(objects[obj->otyp].wldam);
			if(obj->otyp == TWO_HANDED_SWORD) tmp += d(2,6);
			else if(obj->otyp == FLAIL) tmp += rnd(4);
		} else {
 tmp = rnd(objects[obj->otyp].wsdam);
		}
		tmp += obj->spe;
		if(!thrown && obj == uwep && obj->otyp == BOOMERANG
		 && !rn2(3)){
		  pline("As you hit %s, the boomerang breaks into splinters.",
				monnam(mon));
			freeinv(obj);
			setworn((struct obj *) 0, obj->owornmask);
			obfree(obj, (struct obj *) 0);
			tmp++;
		}
	    }
	    if(mon->data->mlet == 'O' && !strcmp(ONAME(obj), "Orcrist"))
		tmp += rnd(10);
	} else	switch(obj->otyp) {
		case HEAVY_IRON_BALL:
			tmp = rnd(25); break;
		case EXPENSIVE_CAMERA:
	pline("You succeed in destroying your camera. Congratulations!");
			freeinv(obj);
			if(obj->owornmask)
				setworn((struct obj *) 0, obj->owornmask);
			obfree(obj, (struct obj *) 0);
			return(TRUE);
		case DEAD_COCKATRICE:
			pline("You hit %s with the cockatrice corpse",
				monnam(mon));
			pline("%s is turned to stone!", Monnam(mon));
			killed(mon);
			return(FALSE);
		case CLOVE_OF_GARLIC:
			if(index(" VWZ", mon->data->mlet))
				mon->mflee = 1;
			tmp = 1;
			break;
		default:
			/* non-weapons can damage because of their weight */
			/* (but not too much) */
			tmp = obj->owt/10;
			if(tmp < 1) tmp = 1;
			else tmp = rnd(tmp);
			if(tmp > 6) tmp = 6;
		}

	/****** NOTE: perhaps obj is undefined!! (if !thrown && BOOMERANG) */

	tmp += u.udaminc + dbon();
	if(u.uswallow)
		if(mon->data->mlet == 'P') {
			if((tmp -= u.uswldtim) <= 0) {
				pline("Your arms are no longer able to hit.");
				return(TRUE);
			}
		}
	if(tmp < 1) tmp = 1;
	mon->mhp -= tmp;
	if(mon->mhp < 1) {
		killed(mon);
		return(FALSE);
	}

	if(thrown) {	/* this assumes that we cannot throw plural things */
		hit( xname(obj)		/* or: objects[obj->otyp].oc_name */,
			mon, exclam(tmp) );
		return(TRUE);
	}
	if(Blind) pline("You hit it.");
	else pline("You hit %s%s", monnam(mon), exclam(tmp));

	if(u.umconf) {
		if(!Blind) {
			pline("Your hands stop glowing blue.");
			if(!mon->mfroz && !mon->msleep)
				pline("%s appears confused.",Monnam(mon));
		}
		mon->mconf = 1;
		u.umconf = 0;
	}
	return(TRUE);	/* mon still alive */
}
