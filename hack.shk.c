/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#ifdef QUEST
int shlevel = 0;
struct monst *shopkeeper = 0;
struct obj *billobjs = 0;
obfree(obj,merge) register struct obj *obj, *merge; {
	free((char *) obj);
}
inshop(){ return(0); }
addtobill(){}
subfrombill(){}
splitbill(){}
dopay(){}
paybill(){}
doinvbill(){}
shkdead(){}
shk_move(){ return(0); }
setshk(){}
char *shkname(){ return(""); }

#else
#include	"hack.mfndpos.h"
#include	"def.eshk.h"

#define	ESHK	((struct eshk *)(&(shopkeeper->mextra[0])))
#define	NOTANGRY	shopkeeper->mpeaceful
#define	ANGRY	!NOTANGRY

extern char plname[];
extern struct obj *o_on();
struct monst *shopkeeper = 0;
struct bill_x *bill;
int shlevel = 0;	/* level of this shopkeeper */
struct obj *billobjs;	/* objects on bill with bp->useup */
/* #define	billobjs	shopkeeper->minvent
   doesnt work so well, since we do not want these objects to be dropped
   when the shopkeeper is killed.
   (See also the save and restore routines.)
 */

/* invariants: obj->unpaid iff onbill(obj) [unless bp->useup]
		obj->quan <= bp->bquan
 */

long int total;

char shtypes[] = "=/)%?!["; /* 8 shoptypes: 7 specialized, 1 mixed */
char *shopnam[] = {
	"engagement ring", "walking cane", "antique weapon",
	"delicatessen", "second hand book", "liquor",
	"used armor", "assorted antiques"
};

char *
shkname() {
	return(ESHK->shknam);
}

shkdead(){
	rooms[ESHK->shoproom].rtype = 0;
	setpaid();
	shopkeeper = 0;
	bill = (struct bill_x *) -1000;	/* dump core when referenced */
}

setpaid(){	/* caller has checked that shopkeeper exists */
register struct obj *obj;
	for(obj = invent; obj; obj = obj->nobj)
		obj->unpaid = 0;
	for(obj = fobj; obj; obj = obj->nobj)
		obj->unpaid = 0;
	while(obj = billobjs){
		billobjs = obj->nobj;
		free((char *) obj);
	}
 ESHK->billct = 0;
}

addupbill(){	/* delivers result in total */
		/* caller has checked that shopkeeper exists */
register ct = ESHK->billct;
register struct bill_x *bp = bill;
	total = 0;
	while(ct--){
		total += bp->price * bp->bquan;
		bp++;
	}
}

inshop(){
register tmp = inroom(u.ux,u.uy);
	if(tmp < 0 || rooms[tmp].rtype < 8) {
		u.uinshop = 0;
		if(shopkeeper && ESHK->billct){
			pline("Somehow you escaped the shop without paying!");
			addupbill();
			pline("You stole for a total worth of %lu zorkmids.",
				total);
			ESHK->robbed += total;
			setpaid();
		}
		if(tmp >= 0 && rooms[tmp].rtype == 7){
			register struct monst *mtmp;
			pline("Welcome to David's treasure zoo!");
			rooms[tmp].rtype = 0;
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
				if(!rn2(4)) mtmp->msleep = 0;
		}
	} else {
		if(shlevel != dlevel) setshk();
		if(!shopkeeper) u.uinshop = 0;
		else if(!u.uinshop){
			if(!ESHK->visitct ||
				strncmp(ESHK->customer, plname, PL_NSIZ)){
				/* He seems to be new here */
				ESHK->visitct = 0;
				(void) strncpy(ESHK->customer,plname,PL_NSIZ);
				NOTANGRY = 1;
			}
			pline("Hello %s! Welcome%s to %s's %s shop!",
				plname,
				ESHK->visitct++ ? " again" : "",
				shkname(),
				shopnam[rooms[ESHK->shoproom].rtype - 8] );
			u.uinshop = 1;
		}
	}
 return(u.uinshop);
}

setshk(){
register struct monst *mtmp;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(mtmp->isshk){
		shopkeeper = mtmp;
		bill = &(ESHK->bill[0]);
		shlevel = dlevel;
		if(ANGRY && strncpy(ESHK->customer,plname,PL_NSIZ))
			NOTANGRY = 1;
		billobjs = 0;
		return;
	}
	shopkeeper = 0;
	bill = (struct bill_x *) -1000;	/* dump core when referenced */
}

struct bill_x *
onbill(obj) register struct obj *obj; {
register struct bill_x *bp;
	if(!shopkeeper) return(0);
	for(bp = bill; bp < &bill[ESHK->billct]; bp++)
		if(bp->bo_id == obj->o_id) {
			if(!obj->unpaid) pline("onbill: paid obj on bill?");
			return(bp);
		}
	if(obj->unpaid) pline("onbill: unpaid obj not on bill?");
	return(0);
}

/* called with two args on merge */
obfree(obj,merge) register struct obj *obj, *merge; {
register struct bill_x *bp = onbill(obj);
register struct bill_x *bpm;
	if(bp) {
		if(!merge){
			bp->useup = 1;
			obj->unpaid = 0;	/* only for doinvbill */
			obj->nobj = billobjs;
			billobjs = obj;
			return;
		}
		bpm = onbill(merge);
		if(!bpm){
			/* this used to be a rename */
			impossible();
			return;
		} else {
			/* this was a merger */
			bpm->bquan += bp->bquan;
			ESHK->billct--;
			*bp = bill[ESHK->billct];
		}
	}
 free((char *) obj);
}

pay(tmp) long tmp; {
	u.ugold -= tmp;
	shopkeeper->mgold += tmp;
	flags.botl = 1;
}

dopay(){
long ltmp;
register struct bill_x *bp;
int shknear = (shlevel == dlevel && shopkeeper &&
	dist(shopkeeper->mx,shopkeeper->my) < 3);
int pass, tmp;

	multi = 0;
	if(!inshop() && !shknear) {
		pline("You are not in a shop.");
		return(0);
	}
	if(!shknear &&
	    inroom(shopkeeper->mx,shopkeeper->my) != ESHK->shoproom){
		pline("There is nobody here to receive your payment.");
		return(0);
	}
	if(!ESHK->billct){
		pline("You do not owe %s anything.", monnam(shopkeeper));
		if(!u.ugold){
			pline("Moreover, you have no money.");
			return(1);
		}
		if(ESHK->robbed){
			pline("But since the shop has been robbed recently,");
			pline("you %srepay %s's expenses.",
				(u.ugold < ESHK->robbed) ? "partially " : "",
				monnam(shopkeeper));
			pay((u.ugold<ESHK->robbed) ? u.ugold : ESHK->robbed);
			ESHK->robbed = 0;
			return(1);
		}
		if(ANGRY){
			pline("But in order to appease %s,",
				amonnam(shopkeeper, "angry"));
			if(u.ugold >= 1000){
				ltmp = 1000;
				pline(" you give him 1000 gold pieces.");
			} else {
				ltmp = u.ugold;
				pline(" you give him all your money.");
			}
			pay(ltmp);
			if(rn2(3)){
				pline("%s calms down.", Monnam(shopkeeper));
				NOTANGRY = 1;
			} else	pline("%s is as angry as ever.",
 	Monnam(shopkeeper));
		}
 return(1);
	}
	for(pass = 0; pass <= 1; pass++) {
		tmp = 0;
		while(tmp < ESHK->billct) {
			bp = &bill[tmp];
			if(!pass && !bp->useup) {
				tmp++;
				continue;
			}
			if(!dopayobj(bp)) return(1);
			bill[tmp] = bill[--ESHK->billct];
		}
	}
	pline("Thank you for shopping in %s's %s store!",
		shkname(),
		shopnam[rooms[ESHK->shoproom].rtype - 8]);
	NOTANGRY = 1;
	return(1);
}

/* return 1 if paid successfully */
/*        0 if not enough money */
/*       -1 if object could not be found (but was paid) */
dopayobj(bp) register struct bill_x *bp; {
register struct obj *obj;
long ltmp;

	/* find the object on one of the lists */
	if(bp->useup)
		obj = o_on(bp->bo_id, billobjs);
	else if(!(obj = o_on(bp->bo_id, invent)) &&
		!(obj = o_on(bp->bo_id, fobj)) &&
		!(obj = o_on(bp->bo_id, fcobj))) {
		    register struct monst *mtmp;
		    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if(obj = o_on(bp->bo_id, mtmp->minvent))
			    break;
		    for(mtmp = fallen_down; mtmp; mtmp = mtmp->nmon)
			if(obj = o_on(bp->bo_id, mtmp->minvent))
			    break;
		}
	if(!obj) {
		pline("Shopkeeper administration out of order.");
		impossible();
		setpaid();	/* be nice to the player */
		return(0);
	}

	if(!obj->unpaid && !bp->useup){
		pline("Paid object on bill??");
		impossible();
		return(1);
	}
	obj->unpaid = 0;
	ltmp = bp->price * bp->bquan;
	if(ANGRY) ltmp += ltmp/3;
	if(u.ugold < ltmp){
		pline("You don't have gold enough to pay %s.",
			doname(obj));
		obj->unpaid = 1;
		return(0);
	}
	pay(ltmp);
	pline("You bought %s for %ld gold piece%s.",
		doname(obj), ltmp, (ltmp == 1) ? "" : "s");
	if(bp->useup) {
		register struct obj *otmp = billobjs;
		if(obj == billobjs)
			billobjs = obj->nobj;
		else {
			while(otmp && otmp->nobj != obj) otmp = otmp->nobj;
			if(otmp) otmp->nobj = obj->nobj;
			else pline("Error in shopkeeper administration");
		}
 free((char *) obj);
	}
 return(1);
}

/* routine called after dying (or quitting) with nonempty bill */
paybill(){
	if(shopkeeper && ESHK->billct){
		addupbill();
		if(total > u.ugold){
			shopkeeper->mgold += u.ugold;
			u.ugold = 0;
		pline("%s comes and takes all your possessions.",
			Monnam(shopkeeper));
		} else {
			u.ugold -= total;
			shopkeeper->mgold += total;
	pline("%s comes and takes the %ld zorkmids you owed him.",
		Monnam(shopkeeper), total);
		}
 setpaid();	/* in case we create bones */
	}
}

/* called in hack.c when we pickup an object */
addtobill(obj) register struct obj *obj; {
register struct bill_x *bp;
	if(!inshop() || (u.ux == ESHK->shk.x && u.uy == ESHK->shk.y) ||
		(u.ux == ESHK->shd.x && u.uy == ESHK->shd.y) ||
		onbill(obj) /* perhaps we threw it away earlier */
	) return;
	if(ESHK->billct == BILLSZ){
		pline("You got that for free!");
		return;
	}
	bp = &bill[ESHK->billct];
	bp->bo_id = obj->o_id;
	bp->bquan = obj->quan;
	bp->useup = 0;
	bp->price = getprice(obj);
	ESHK->billct++;
	obj->unpaid = 1;
}

splitbill(obj,otmp) register struct obj *obj, *otmp; {
	/* otmp has been split off from obj */
register struct bill_x *bp;
register int tmp;
	bp = onbill(obj);
	if(!bp) { impossible(); return; }
	if(bp->bquan < otmp->quan) {
		pline("Negative quantity on bill??");
		impossible();
	}
	if(bp->bquan == otmp->quan) {
		pline("Zero quantity on bill??");
		impossible();
	}
	bp->bquan -= otmp->quan;

	/* addtobill(otmp); */
	if(ESHK->billct == BILLSZ) otmp->unpaid = 0;
	else {
		tmp = bp->price;
		bp = &bill[ESHK->billct];
		bp->bo_id = otmp->o_id;
		bp->bquan = otmp->quan;
		bp->useup = 0;
		bp->price = tmp;
		ESHK->billct++;
	}
}

subfrombill(obj) register struct obj *obj; {
long ltmp;
register int tmp;
register struct obj *otmp;
register struct bill_x *bp;
	if(!inshop() || (u.ux == ESHK->shk.x && u.uy == ESHK->shk.y) ||
		(u.ux == ESHK->shd.x && u.uy == ESHK->shd.y))
		return;
	if((bp = onbill(obj)) != 0){
		obj->unpaid = 0;
		if(bp->bquan > obj->quan){
			otmp = newobj(0);
			*otmp = *obj;
			bp->bo_id = otmp->o_id = flags.ident++;
			otmp->quan = (bp->bquan -= obj->quan);
			otmp->owt = 0;	/* superfluous */
			otmp->onamelth = 0;
			bp->useup = 1;
			otmp->nobj = billobjs;
			billobjs = otmp;
			return;
		}
		ESHK->billct--;
		*bp = bill[ESHK->billct];
		return;
	}
	if(obj->unpaid){
		pline("%s didn't notice.", Monnam(shopkeeper));
		obj->unpaid = 0;
		return;		/* %% */
	}
	/* he dropped something of his own - probably wants to sell it */
	if(shopkeeper->msleep || shopkeeper->mfroz ||
		inroom(shopkeeper->mx,shopkeeper->my) != ESHK->shoproom)
		return;
	if(ESHK->billct == BILLSZ ||
	  ((tmp = shtypes[rooms[ESHK->shoproom].rtype-8]) && tmp != obj->olet)
	  || index("_0", obj->olet)) {
		pline("%s seems not interested.", Monnam(shopkeeper));
		return;
	}
	ltmp = getprice(obj) * obj->quan;
	if(ANGRY) {
		ltmp /= 3;
		NOTANGRY = 1;
	} else	ltmp /= 2;
	if(ESHK->robbed){
		if((ESHK->robbed -= ltmp) < 0) ESHK->robbed = 0;
pline("Thank you for your contribution to restock this recently plundered shop.");
		return;
	}
	if(ltmp > shopkeeper->mgold) ltmp = shopkeeper->mgold;
	pay(-ltmp);
	if(!ltmp)
	pline("%s gladly accepts %s but cannot pay you at present.",
		Monnam(shopkeeper), doname(obj));
	else
	pline("You sold %s and got %ld gold piece%s.", doname(obj), ltmp,
		(ltmp == 1) ? "" : "s");
}

doinvbill(cl) int cl; {
register unsigned tmp,cnt = 0;
register struct obj *obj;
char buf[BUFSZ];
	if(!shopkeeper) return;
	for(tmp = 0; tmp < ESHK->billct; tmp++) if(bill[tmp].useup) cnt++;
	if(!cnt) return;
	if(!cl && !flags.oneline) cls();
	if(!flags.oneline) puts("\n\nUnpaid articles already used up:\n");
	for(tmp = 0; tmp < ESHK->billct; tmp++) if(bill[tmp].useup){
		for(obj = billobjs; obj; obj = obj->nobj)
			if(obj->o_id == bill[tmp].bo_id) break;
		if(!obj) {
			pline("Bad shopkeeper administration.");
			impossible();
			return;
		}
		(void) sprintf(buf, "* -  %s", doname(obj));
		for(cnt=0; buf[cnt]; cnt++);
		while(cnt < 50) buf[cnt++] = ' ';
		(void) sprintf(&buf[cnt], " %5d zorkmids",
				bill[tmp].price * bill[tmp].bquan);
		if(flags.oneline)
			pline(buf);
		else
			puts(buf);
	}
	if(!cl && !flags.oneline) {
		getret();
		docrt();
	}
}

getprice(obj) register struct obj *obj; {
register int tmp,ac;
	switch(obj->olet){
		case AMULET_SYM:
			tmp = 10*rnd(500);
			break;
		case TOOL_SYM:
			tmp = 10*rnd(150);
			break;
		case RING_SYM:
			tmp = 10*rnd(100);
			break;
		case WAND_SYM:
			tmp = 10*rnd(100);
			break;
		case SCROLL_SYM:
			tmp = 10*rnd(50);
			break;
		case POTION_SYM:
			tmp = 10*rnd(50);
			break;
		case FOOD_SYM:
			tmp = 10*rnd(5 + (2000/realhunger()));
			break;
		case GEM_SYM:
			tmp = 10*rnd(20);
			break;
		case ARMOR_SYM:
			ac = 10 - obj->spe;
			tmp = 100 + (10-ac)*(10-ac)*rnd(20-ac);
			break;
		case WEAPON_SYM:
			if(obj->otyp < BOOMERANG)
				tmp = 5*rnd(10);
			else if(obj->otyp == LONG_SWORD ||
				obj->otyp == TWO_HANDED_SWORD)
				tmp = 10*rnd(150);
			else	tmp = 10*rnd(75);
			break;
		case CHAIN_SYM:
			pline("Strange ..., carrying a chain?");
		case BALL_SYM:
			tmp = 10;
			break;
		default:
			tmp = 10000;
	}
 return(tmp);
}

realhunger(){	/* not completely foolproof */
register tmp = u.uhunger;
register struct obj *otmp = invent;
	while(otmp){
		if(otmp->olet == FOOD_SYM && !otmp->unpaid)
			tmp += objects[otmp->otyp].nutrition;
		otmp = otmp->nobj;
	}
 return((tmp <= 0) ? 1 : tmp);
}

shk_move(){
register struct monst *mtmp;
register struct permonst *mdat = shopkeeper->data;
register xchar gx,gy,omx,omy,nx,ny,nix,niy;
register schar appr,i;
schar shkr,tmp,chi,chcnt,cnt;
boolean uondoor, avoid;
coord poss[9];
int info[9];
	omx = shopkeeper->mx;
	omy = shopkeeper->my;
	shkr = inroom(omx,omy);
	if(ANGRY && dist(omx,omy) < 3){
		(void) hitu(shopkeeper, d(mdat->damn, mdat->damd)+1);
		return(0);
	}
	appr = 1;
	gx = ESHK->shk.x;
	gy = ESHK->shk.y;
	if(ANGRY){
		int saveBlind = Blind;
		Blind = 0;
		if(shopkeeper->mcansee && !Invis && cansee(omx,omy)) {
			gx = u.ux;
			gy = u.uy;
		}
		Blind = saveBlind;
		avoid = FALSE;
	} else {
#define	GDIST(x,y)	((x-gx)*(x-gx)+(y-gy)*(y-gy))
		if(Invis)
		  avoid = FALSE;
		else {
		  uondoor = (u.ux == ESHK->shd.x && u.uy == ESHK->shd.y);
		  avoid = ((u.uinshop && dist(gx,gy) > 8) || uondoor);
		  if(((!ESHK->robbed && !ESHK->billct) || avoid)
		  	&& GDIST(omx,omy) < 3){
		  	if(!online(omx,omy)) return(0);
		  	if(omx == gx && omy == gy)
		  		appr = gx = gy = 0;
		  }
		}
	}
	if(omx == gx && omy == gy) return(0);
	if(shopkeeper->mconf) appr = 0;
	nix = omx;
	niy = omy;
	cnt = mfndpos(shopkeeper,poss,info,
		(avoid ? NOTONL : 0) | ALLOW_SSM);
	if(cnt == 0 && avoid && uondoor)
		cnt = mfndpos(shopkeeper,poss,info,ALLOW_SSM);
	chi = -1;
	chcnt = 0;
	for(i=0; i<cnt; i++){
		nx = poss[i].x;
		ny = poss[i].y;
	   	if((tmp = levl[nx][ny].typ) == ROOM ||
		(shkr != ESHK->shoproom && (tmp==CORR || tmp==DOOR)))
#ifdef STUPID
		/* cater for stupid compilers */
		{ int zz;
		if((!appr && !rn2(++chcnt)) ||
		   (appr && (zz = GDIST(nix,niy)) && zz > GDIST(nx,ny))){
#else
		if((!appr && !rn2(++chcnt)) ||
		   (appr && GDIST(nx,ny) < GDIST(nix,niy))){
#endif STUPID
			nix = nx;
			niy = ny;
			chi = i;
#ifdef STUPID
		   }
#endif STUPID
		}
	}
	if(nix != omx || niy != omy){
		if(info[chi] & ALLOW_M){
			mtmp = m_at(nix,niy);
			if(hitmm(shopkeeper,mtmp) == 1 && rn2(3) &&
			   hitmm(mtmp,shopkeeper) == 2) return(2);
			return(0);
		} else if(info[chi] & ALLOW_U){
			(void) hitu(shopkeeper, d(mdat->damn, mdat->damd)+1);
			return(0);
		}
		shopkeeper->mx = nix;
		shopkeeper->my = niy;
		pmon(shopkeeper);
		return(1);
	}
 return(0);
}
#endif QUEST

char *
plur(n) unsigned n; {
	return((n==1) ? "" : "s");
}

online(x,y) {
	return(x==u.ux || y==u.uy ||
		(x-u.ux)*(x-u.ux) == (y-u.uy)*(y-u.uy));
}
