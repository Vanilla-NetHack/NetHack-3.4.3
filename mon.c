/*	SCCS Id: @(#)mon.c	2.1	87/10/17
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include "hack.h"
#include "mfndpos.h"
extern struct monst *mkmon_at();
extern struct trap *maketrap();
extern struct obj *mkobj_at(), *mksobj_at();
extern char *hcolor();
#ifdef KAA
extern boolean	stoned;
extern char mlarge[];
#endif
#ifdef RPH
extern struct obj *mk_named_obj_at();
#endif

int warnlevel;		/* used by movemon and dochugw */
long lastwarntime;
int lastwarnlev;
char	*warnings[] = {	"white", "pink", "red", "ruby", "purple", "black"  };

movemon()
{
	register struct monst *mtmp;
	register int fr;

	warnlevel = 0;

	while(1) {
		/* find a monster that we haven't treated yet */
		/* note that mtmp or mtmp->nmon might get killed
		   while mtmp moves, so we cannot just walk down the
		   chain (even new monsters might get created!) */
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(mtmp->mlstmv < moves) goto next_mon;
		/* treated all monsters */
		break;

	next_mon:
		mtmp->mlstmv = moves;

		/* most monsters drown in pools */
		{ boolean inpool, iseel;

		  inpool = (levl[mtmp->mx][mtmp->my].typ == POOL);
		  iseel = (mtmp->data->mlet == ';');
		  if(inpool && !iseel) {
			if(cansee(mtmp->mx,mtmp->my))
			    pline("%s drowns.", Monnam(mtmp));
			mondead(mtmp);
			continue;
		  }
		/* but eels have a difficult time outside */
		  if(iseel && !inpool) {
			if(mtmp->mhp > 1) mtmp->mhp--;
			mtmp->mflee = 1;
			mtmp->mfleetim += 2;
		  }
		}
		if(mtmp->mblinded && !--mtmp->mblinded)
			mtmp->mcansee = 1;
		if(mtmp->mfleetim && !--mtmp->mfleetim)
			mtmp->mflee = 0;
#ifdef HARD
		/* unwatched mimics and piercers may hide again  [MRS] */
		if(restrap(mtmp))	continue;
#endif
		if(mtmp->mimic) continue;
		if(mtmp->mspeed != MSLOW || !(moves%2)){
			/* continue if the monster died fighting */
			fr = -1;
			if(Conflict && cansee(mtmp->mx,mtmp->my)
				&& (fr = fightm(mtmp)) == 2)
				continue;
			if(fr<0 && dochugw(mtmp))
				continue;
		}
		if(mtmp->mspeed == MFAST && dochugw(mtmp))
			continue;
	}

	warnlevel -= u.ulevel;
	if(warnlevel >= SIZE(warnings))
		warnlevel = SIZE(warnings)-1;
	if(warnlevel >= 0)
	if(warnlevel > lastwarnlev || moves > lastwarntime + 5){
	    register char *rr;
	    switch((int) (Warning & (LEFT_RING | RIGHT_RING))){
	    case LEFT_RING:
		rr = "Your left ring glows";
		break;
	    case RIGHT_RING:
		rr = "Your right ring glows";
		break;
	    case LEFT_RING | RIGHT_RING:
		rr = "Both your rings glow";
		break;
	    default:
		rr = "Your fingertips glow";
		break;
	    }
	    pline("%s %s!", rr, Hallucination ? hcolor() : warnings[warnlevel]);
	    lastwarntime = moves;
	    lastwarnlev = warnlevel;
	}

	dmonsfree();	/* remove all dead monsters */
}

justswld(mtmp,name)
register struct monst *mtmp;
char *name;
{

	mtmp->mx = u.ux;
	mtmp->my = u.uy;
	u.ustuck = mtmp;
	pmon(mtmp);
	kludge("%s swallows you!",name);
	more();
	seeoff(1);
	u.uswallow = 1;
	u.uswldtim = 0;
	swallowed();
}

youswld(mtmp,dam,die,name)
register struct monst *mtmp;
register dam,die;
char *name;
{
	if(mtmp != u.ustuck) return;
	kludge("%s digests you!",name);
	u.uhp -= dam;
	if(u.uswldtim++ >= die){	/* a3 */
		pline("It totally digests you!");
		u.uhp = -1;
	}
	if(u.uhp < 1) done_in_by(mtmp);
	/* flags.botlx = 1;		/* should we show status line ? */
}

#ifdef ROCKMOLE
meatgold(mtmp) register struct monst *mtmp; {
register struct gold *gold;
register int pile;
register struct obj *otmp;
#ifdef KJSMODS
	if(dlevel < 4) return;
#endif
	/* Eats gold if it is there */
	while(gold = g_at(mtmp->mx, mtmp->my)){
		freegold(gold);
		/* Left behind a pile? */
		pile = rnd(25);
		if(pile < 3) mksobj_at(ROCK, mtmp->mx, mtmp->my);
		newsym(mtmp->mx, mtmp->my);
	}
	/* Eats armor if it is there */
	otmp = o_at(mtmp->mx,mtmp->my);
	if((otmp) && (otmp->otyp >= PLATE_MAIL) && (otmp->otyp <= RING_MAIL)){
		freeobj(otmp);
		/* Left behind a pile? */
		pile = rnd(25);
		if(pile < 3)  mksobj_at(ROCK, mtmp->mx, mtmp->my);
		newsym(mtmp->mx, mtmp->my);
	}
}
#endif /* ROCKMOLE /**/

mpickgold(mtmp) register struct monst *mtmp; {
register struct gold *gold;
	while(gold = g_at(mtmp->mx, mtmp->my)){
		mtmp->mgold += gold->amount;
		freegold(gold);
		if(levl[mtmp->mx][mtmp->my].scrsym == GOLD_SYM)
			newsym(mtmp->mx, mtmp->my);
	}
}

/* Now includes giants which pick up enormous rocks.  KAA */
mpickgems(mtmp) register struct monst *mtmp; {
register struct obj *otmp;
	for(otmp = fobj; otmp; otmp = otmp->nobj)
	  if(otmp->olet ==
#ifdef KAA
			   (mtmp->data->mlet=='9' ? ROCK_SYM : GEM_SYM))
#else
			   GEM_SYM)
#endif
	    if(otmp->ox == mtmp->mx && otmp->oy == mtmp->my)
	      if(mtmp->data->mlet != 'u' || objects[otmp->otyp].g_val != 0){
		freeobj(otmp);
		mpickobj(mtmp, otmp);
#ifndef KAA
		if(levl[mtmp->mx][mtmp->my].scrsym == GEM_SYM)
#endif
			newsym(mtmp->mx, mtmp->my);	/* %% */
		return;	/* pick only one object */
	      }
}

/* return number of acceptable neighbour positions */
mfndpos(mon,poss,info,flag)
register struct monst *mon;
coord poss[9];
long info[9], flag;
{
	register int x,y,nx,ny,cnt = 0,ntyp;
	register struct monst *mtmp;
	int nowtyp;
	boolean pool;

	x = mon->mx;
	y = mon->my;
	nowtyp = levl[x][y].typ;

	pool = (mon->data->mlet == ';');
nexttry:	/* eels prefer the water, but if there is no water nearby,
		   they will crawl over land */
	if(mon->mconf) {
		flag |= ALLOW_ALL;
		flag &= ~NOTONL;
	}
	for(nx = x-1; nx <= x+1; nx++) for(ny = y-1; ny <= y+1; ny++)
	if(nx != x || ny != y) if(isok(nx,ny))
#ifdef ROCKMOLE
	if(!IS_ROCK(ntyp = levl[nx][ny].typ) || (flag & ALLOW_WALL))
#else
	if(!IS_ROCK(ntyp = levl[nx][ny].typ))
#endif
	if(!(nx != x && ny != y && (nowtyp == DOOR || ntyp == DOOR)))
	if((ntyp == POOL) == pool) {
		info[cnt] = 0;
		if(nx == u.ux && ny == u.uy){
			if(!(flag & ALLOW_U)) continue;
			info[cnt] = ALLOW_U;
		} else if(mtmp = m_at(nx,ny)){
			if(!(flag & ALLOW_M)) continue;
			info[cnt] = ALLOW_M;
			if(mtmp->mtame){
				if(!(flag & ALLOW_TM)) continue;
				info[cnt] |= ALLOW_TM;
			}
		}
		if(sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
			if(flag & NOGARLIC) continue;
			info[cnt] |= NOGARLIC;
		}
		if(sobj_at(SCR_SCARE_MONSTER, nx, ny) ||
		   (!mon->mpeaceful && sengr_at("Elbereth", nx, ny))) {
			if(!(flag & ALLOW_SSM)) continue;
			info[cnt] |= ALLOW_SSM;
		}
		if(sobj_at(ENORMOUS_ROCK, nx, ny)) {
			if(!(flag & ALLOW_ROCK)) continue;
			info[cnt] |= ALLOW_ROCK;
		}
		if(!Invis && online(nx,ny)){
			if(flag & NOTONL) continue;
			info[cnt] |= NOTONL;
		}
		/* we cannot avoid traps of an unknown kind */
		{ register struct trap *ttmp = t_at(nx, ny);
		  register long tt;
			if(ttmp) {
				tt = 1L << ttmp->ttyp;
				/* below if added by GAN 02/06/87 to avoid
				 * traps out of range
				 */
				if(!(tt & ALLOW_TRAPS))  {
					impossible("A monster looked at a very strange trap");
					continue;
				}
				if(mon->mtrapseen & tt){
					if(!(flag & tt)) continue;
					info[cnt] |= tt;
				}
			}
		}
		poss[cnt].x = nx;
		poss[cnt].y = ny;
		cnt++;
	}
	if(!cnt && pool && nowtyp != POOL) {
		pool = FALSE;
		goto nexttry;
	}
	return(cnt);
}

dist(x,y) int x,y; {
	return((x-u.ux)*(x-u.ux) + (y-u.uy)*(y-u.uy));
}

poisoned(string, pname)
register char *string, *pname;
{
	register i, plural;

	plural = (string[strlen(string) - 1] == 's')? 1 : 0;
	if(Blind) {
		if (plural)	pline("They were poisoned.");
		else		pline("It was poisoned.");
	} else	{
		if (plural)	pline("The %s were poisoned!", string);
		else		pline("The %s was poisoned!", string);
	}

	if(Poison_resistance) {
		pline("The poison doesn't seem to affect you.");
		return;
	}
	i = rn2(10);
	if(i == 0) {
		u.uhp = -1;
		pline("I am afraid the poison was deadly ...");
	} else if(i <= 5) {
		losestr(rn1(3,3));
	} else {
		losehp(rn1(10,6), pname);
	}
	if(u.uhp < 1) {
		killer = pname;
		done("died");
	}
}

mondead(mtmp)
register struct monst *mtmp;
{
	relobj(mtmp,1);
	unpmon(mtmp);
	relmon(mtmp);
	unstuck(mtmp);
#ifdef KOPS
       if(mtmp->data->mlet == 'K' &&
          !strcmp(mtmp->data->mname,"Keystone Kop")) {
	   /* When a Kop dies, he probably comes back. */
	   switch(rnd(3)) {

		case 1:	     /* returns near the stairs */
			mkmon_at('K',xdnstair,ydnstair);
			break;
		case 2:	     /* randomly */
			mkmon_at('K',0,0);
			break;
		default:
			break;
	   }
	  }
#endif
	if(mtmp->isshk) shkdead(mtmp);
	if(mtmp->isgd) gddead();
#ifndef NOWORM
	if(mtmp->wormno) wormdead(mtmp);
#endif
#ifdef HARD
	if(mtmp->data->mlet == '1') wizdead(mtmp);
#endif
	monfree(mtmp);
}

/* called when monster is moved to larger structure */
replmon(mtmp,mtmp2)
register struct monst *mtmp, *mtmp2;
{
	relmon(mtmp);
	monfree(mtmp);
	mtmp2->nmon = fmon;
	fmon = mtmp2;
	if(u.ustuck == mtmp) u.ustuck = mtmp2;
	if(mtmp2->isshk) replshk(mtmp,mtmp2);
	if(mtmp2->isgd) replgd(mtmp,mtmp2);
}

relmon(mon)
register struct monst *mon;
{
	register struct monst *mtmp;

	if (fmon == 0)  panic ("relmon: no fmon available.");

	if(mon == fmon) fmon = fmon->nmon;
	else {
		for(mtmp = fmon; mtmp->nmon != mon; mtmp = mtmp->nmon) ;
		mtmp->nmon = mon->nmon;
	}
}

/* we do not free monsters immediately, in order to have their name
   available shortly after their demise */
struct monst *fdmon;	/* chain of dead monsters, need not to be saved */

monfree(mtmp) register struct monst *mtmp; {
	mtmp->nmon = fdmon;
	fdmon = mtmp;
}

dmonsfree(){
register struct monst *mtmp;
	while(mtmp = fdmon){
		fdmon = mtmp->nmon;
		free((char *) mtmp);
	}
}

unstuck(mtmp)
register struct monst *mtmp;
{
	if(u.ustuck == mtmp) {
		if(u.uswallow){
			u.ux = mtmp->mx;
			u.uy = mtmp->my;
			u.uswallow = 0;
			setsee();
			docrt();
		}
		u.ustuck = 0;
	}
}

killed(mtmp)
register struct monst *mtmp;
{
	xkilled(mtmp, 1);
}

xkilled(mtmp, dest)
register struct monst *mtmp;
int	dest;
/* Dest=1, normal; dest=0, don't print message; dest=2, don't drop corpse
   either; dest=3, message but no corpse */
{
#ifdef LINT
#define	NEW_SCORING
#endif
	register int tmp,tmp2,nk,x,y;
	register struct permonst *mdat = mtmp->data;
	extern long newuexp();
#ifdef RPH
	int old_nlth;
	char old_name[BUFSZ];
#endif

	if(mtmp->cham) mdat = PM_CHAMELEON;
	if (dest & 1) {
	    if(Blind) pline("You destroy it!");
	    else {
		pline("You destroy %s!",
			mtmp->mtame ? amonnam(mtmp, "poor") : monnam(mtmp));
	    }
	}
	if(u.umconf) {
		if(!Blind)
		{
			pline("Your hands stop glowing %s.",
			Hallucination ? hcolor() : "blue");
		}
		u.umconf = 0;
	}

	/* count killed monsters */
#define	MAXMONNO	100
	nk = 1;		      /* in case we cannot find it in mons */
	tmp = mdat - mons;    /* index in mons array (if not 'd', '@', ...) */
	if(tmp >= 0 && tmp < CMNUM+2) {
	    extern char fut_geno[];
	    u.nr_killed[tmp]++;
	    if((nk = u.nr_killed[tmp]) > MAXMONNO &&
#ifdef HARD
# ifdef KOPS
		!index("KkO&", mdat->mlet) &&
# else
		!index("kO&", mdat->mlet) &&
# endif
#endif
		!index(fut_geno, mdat->mlet))
		    charcat(fut_geno,  mdat->mlet);
	}

	/* punish bad behaviour */
	if(mdat->mlet == '@') {
		HTelepat = 0;
		u.uluck -= 2;
	}
	if(mtmp->mpeaceful || mtmp->mtame) u.uluck--;
	if(mdat->mlet == 'u') u.uluck -= 5;
	if((int)u.uluck < LUCKMIN) u.uluck = LUCKMIN;

	/* give experience points */
	tmp = 1 + mdat->mlevel * mdat->mlevel;
	if(mdat->ac < 3) tmp += 2*(7 - mdat->ac);
	if(index(
#ifdef RPH
# ifdef KAA
		 "AcsSDXaeRTVWU&In:P89",
# else
		 "AcsSDXaeRTVWU&In:P8",
# endif
#else
# ifdef KAA
		 "AcsSDXaeRTVWU&In:P9",
# else
		 "AcsSDXaeRTVWU&In:P",
# endif
#endif
					 mdat->mlet)) tmp += 2*mdat->mlevel;

	if(index("DeV&P",mdat->mlet)) tmp += (7*mdat->mlevel);
	if(mdat->mlevel > 6) tmp += 50;
	if(mdat->mlet == ';') tmp += 1000;

#ifdef NEW_SCORING
	/* ------- recent addition: make nr of points decrease
		   when this is not the first of this kind */
	{ int ul = u.ulevel;
	  int ml = mdat->mlevel;

	if(ul < 14)    /* points are given based on present and future level */
	    for(tmp2 = 0; !tmp2 || ul + tmp2 <= ml; tmp2++)
		if(u.uexp + 1 + (tmp + ((tmp2 <= 0) ? 0 : 4<<(tmp2-1)))/nk
		    >= 10*pow((unsigned)(ul-1)))
			if(++ul == 14) break;

	tmp2 = ml - ul -1;
	tmp = (tmp + ((tmp2 < 0) ? 0 : 4<<tmp2))/nk;
	if(!tmp) tmp = 1;
	}
	/* note: ul is not necessarily the future value of u.ulevel */
	/* ------- end of recent valuation change ------- */
#endif /* NEW_SCORING /**/

	more_experienced(tmp,0);
	flags.botl = 1;
	while(u.ulevel < 14 && u.uexp >= newuexp()){
#ifdef RPH
		/* make experience gaining simiar to d&d, whereby you */
		/* can at most go up by one level at a time, extra expr */
		/* possibly helping you along. Afterall, how much real */
		/* experience does one get shooting a wand of death at */
		/* a dragon created w/ a poymorph?? */
		u.ulevel++;
		if (u.uexp >= newuexp())
		    u.uexp = newuexp() - 1;
		pline("Welcome to experience level %u.", u.ulevel);
#else
		pline("Welcome to experience level %u.", ++u.ulevel);
#endif
		tmp = rnd(10);
		if(tmp < 3) tmp = rnd(10);
		u.uhpmax += tmp;
		u.uhp += tmp;
#ifdef SPELLS
		tmp = rnd(u.ulevel/2+1) + 1;	/* M. Stephenson */
		u.uenmax += tmp;
		u.uen += tmp;
#endif
		flags.botl = 1;
	}

	/* dispose of monster and make cadaver */
	x = mtmp->mx;	y = mtmp->my;
#ifdef RPH
	old_nlth = mtmp->mnamelth;
	if (old_nlth > 0)  (void) strcpy (old_name, NAME(mtmp));
#endif	    
	mondead(mtmp);
	tmp = mdat->mlet;
	if(tmp == 'm') { /* he killed a minotaur, give him a wand of digging */
			/* note: the dead minotaur will be on top of it! */
		mksobj_at(WAN_DIGGING, x, y);
		/* if(cansee(x,y)) atl(x,y,fobj->olet); */
		stackobj(fobj);
	} else
#ifndef NOWORM
	if(tmp == 'w') {
		mksobj_at(WORM_TOOTH, x, y);
		stackobj(fobj);
	} else
#endif
#ifdef KJSMODS
	if(tmp == 'N') { 
		mksobj_at(POT_OBJECT_DETECTION, x, y);
		stackobj(fobj);
	} else
#endif
#ifdef KAA
	if(tmp == '&') (void) mkobj_at(0, x, y);
	else
	if(stoned == FALSE && (!letter(tmp) || (!index("9&1", tmp) && !rn2(3)))) tmp = 0;
	    if(dest & 2) {
		newsym(x,y);
		return;
	    }
#else
	if(!letter(tmp) || (!index("mw", tmp) && !rn2(3))) tmp = 0;
#endif
	tmp2 = rn2(5);
#ifdef KJSMODS
	/* if a kobold or a giant rat does not become treasure, do
	 *  not make a corpse. */
# ifdef KOPS
	if(mdat->mlet == 'K'
	   && !strcmp(mdat->mname,"kobold") && tmp) tmp2 = 0;
# endif
# ifdef ROCKMOLE
	if((mdat->mlet == 'r' && dlevel < 4) && tmp) tmp2 = 0;
# endif
#endif
	if(!ACCESSIBLE(levl[x][y].typ)) {
	    /* might be mimic in wall or dead eel*/
 	    newsym(x,y);
	} else if(x != u.ux || y != u.uy) {
		/* might be here after swallowed */
#ifdef KAA
		if(stoned) {
			register int typetmp;
			if(index(mlarge, tmp))	typetmp = ENORMOUS_ROCK;
			else			typetmp = ROCK;
			mksobj_at(typetmp, x, y);
			if(cansee(x,y))
			    atl(x, y, Hallucination ? rndobjsym() :
				      objects[typetmp].oc_olet);
		} else
#endif
		if(index("NTVm&w",mdat->mlet) || tmp2) {
#ifndef RPH
			register struct obj *obj2 = mkobj_at(tmp,x,y);
#else
			register struct obj *obj2;
			if (letter(tmp))
			    obj2 = mk_named_obj_at(tmp, x, y,
						   old_name, old_nlth);
# ifdef KOPS
			else if (mdat->mlet == 'K')
			    obj2 = mksobj_at((rn2(4) ? CLUB : WHISTLE), x, y);
# endif
			else
			    obj2 = mkobj_at(tmp,x,y);
#endif   /* RPH /**/
			if(cansee(x,y))
			    atl(x, y, Hallucination ? rndobjsym() : obj2->olet);
			stackobj(obj2);
		}
	}
}

kludge(str,arg)
register char *str,*arg;
{
	if(Blind) {
		if(*str == '%') pline(str,"It");
		else pline(str,"it");
	} else pline(str,arg);
}

rescham()	/* force all chameleons to become normal */
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->cham) {
			mtmp->cham = 0;
			(void) newcham(mtmp, PM_CHAMELEON);
		}
}

#ifdef DGKMOD
/* Let the chameleons change again -dgk */
restartcham()
{
	register struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if (mtmp->data->mlet == ':') 
			mtmp->cham = 1;
}
#endif

newcham(mtmp,mdat)	/* make a chameleon look like a new monster */
			/* returns 1 if the monster actually changed */
register struct monst *mtmp;
register struct permonst *mdat;
{
	register mhp, hpn, hpd;

	if(mdat == mtmp->data) return(0);	/* still the same monster */
#ifndef NOWORM
	if(mtmp->wormno) wormdead(mtmp);	/* throw tail away */
#endif
	hpn = mtmp->mhp;
	hpd = (mtmp->data->mlevel)*8;	if(!hpd) hpd = 4;
	mhp = (mdat->mlevel)*8;		if(!mhp) mhp = 4;

	/* new hp: same fraction of max as before */
	mtmp->mhp = (hpn*mhp)/hpd;
	if (mhp > hpd && mtmp->mhp < hpn) mtmp->mhp = 127;
/* Not totally foolproof.  A 2HD monster with 80 HP that changes into a 6HD
   monster that really should have 240 and actually should have 127, the
   maximum possible, will wind up having 113.  */
	if (!mtmp->mhp) mtmp->mhp = 1;
/* Unlikely but not impossible; a 1HD creature with 1HP that changes into a
   0HD creature will require this statement */
	mtmp->data = mdat;
/* and the same for maximum hit points */
	hpn = mtmp->mhpmax;
	mtmp->mhpmax = (hpn*mhp)/hpd;
	if (mhp > hpd && mtmp->mhpmax < hpn) mtmp->mhp = 127;
	if (!mtmp->mhp) mtmp->mhp = 1;

	mtmp->minvis = (mdat->mlet == 'I') ? 1 : 0;
	/* only snakes and scorpions can hide under things -dgk */
	/* also generated by GAN */
	mtmp->mhide = (mdat->mlet == 'S' || mdat->mlet == 's') ? 1 : 0;
	if (!mtmp->mhide) mtmp->mundetected = 0;
#ifndef NOWORM
	if(mdat->mlet == 'w' && getwn(mtmp)) initworm(mtmp);
			/* perhaps we should clear mtmp->mtame here? */
#endif
	unpmon(mtmp);	/* necessary for 'I' and to force pmon */
	pmon(mtmp);
	return(1);
}

mnexto(mtmp)	/* Make monster mtmp next to you (if possible) */
struct monst *mtmp;
{
	coord mm;
	enexto(&mm, u.ux, u.uy);
	mtmp->mx = mm.x;
	mtmp->my = mm.y;
	pmon(mtmp);
}

ishuman(mtmp) register struct monst *mtmp; {
	return(mtmp->data->mlet == '@');
}

setmangry(mtmp) register struct monst *mtmp; {
	if(!mtmp->mpeaceful) return;
	if(mtmp->mtame) return;
	mtmp->mpeaceful = 0;
	if(ishuman(mtmp)) pline("%s gets angry!", Monnam(mtmp));
}

/* not one hundred procent correct: now a snake may hide under an
   invisible object */
canseemon(mtmp)
register struct monst *mtmp;
{
	return((!mtmp->minvis || See_invisible)
		&& (!mtmp->mhide || !o_at(mtmp->mx,mtmp->my))
		&& cansee(mtmp->mx, mtmp->my));
}

disturb(mtmp)		/* awaken monsters while in the same room.
			 * return a 1 if they have been woken.
			 */
register struct monst *mtmp;
{
	/* wake up, or get out of here. */
	/* ettins are hard to surprise */
	/* Nymphs and Leprechauns do not easily wake up */
	if(cansee(mtmp->mx,mtmp->my) &&
		(!Stealth || (mtmp->data->mlet == 'e' && rn2(10))) &&
		(!index("NL",mtmp->data->mlet) || !rn2(50)) &&
#ifdef RPH
		(Aggravate_monster || index("8d1", mtmp->data->mlet)
#else
		(Aggravate_monster || index("d1", mtmp->data->mlet)
#endif
			|| (!rn2(7) && !mtmp->mimic))) {
		mtmp->msleep = 0;
		return(1);
	}
	if(Hallucination) pmon(mtmp);
	return(0);
}

#ifdef HARD
restrap(mtmp)		/* unwatched mimics and piercers may hide again,
			 * if so, a 1 is returned.
			 */
register struct monst *mtmp;
{
	if(mtmp->data->mlet == 'M' && !mtmp->mimic && !mtmp->cham
	   && !mtmp->mcan && !cansee(mtmp->mx, mtmp->my)
	   && !rn2(3)) {
		mtmp->mimic = 1;
		mtmp->mappearance = (levl[mtmp->mx][mtmp->my].typ == DOOR) ? DOOR_SYM : GOLD_SYM;
		return(1);
	   }

	if(mtmp->data->mlet == 'p' && !mtmp->cham
	   && !mtmp->mcan && !cansee(mtmp->mx, mtmp->my)
	   && !rn2(3))  {

		if(levl[mtmp->mx][mtmp->my].typ == ROOM)  {

			maketrap(mtmp->mx, mtmp->my, PIERC);
			mondead(mtmp);
			return(1);
		}
	   }
	return(0);
}
#endif
