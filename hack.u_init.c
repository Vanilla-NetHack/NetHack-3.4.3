/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "hack.h"
#include <stdio.h>
#include <signal.h>
#define	Strcat	(void) strcat
#define	UNDEF_TYP	0
#define	UNDEF_SPE	(-1)
extern struct obj *addinv();
extern char plname[];

char pl_character[PL_CSIZ];

struct trobj {
	uchar trotyp;
	schar trspe;
	char trolet;
	Bitfield(trquan,6);
	Bitfield(trknown,1);
};

#ifdef WIZARD
struct trobj Extra_objs[] = {
	{ 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif WIZARD

struct trobj Cave_man[] = {
	{ MACE, 1, WEAPON_SYM, 1, 1 },
	{ BOW, 1, WEAPON_SYM, 1, 1 },
	{ ARROW, 0, WEAPON_SYM, 25, 1 },	/* quan is variable */
	{ LEATHER_ARMOR, 2, ARMOR_SYM, 1, 1 },
	{ 0, 0, 0, 0, 0}
};

struct trobj Fighter[] = {
	{ TWO_HANDED_SWORD, 0, WEAPON_SYM, 1, 1 },
	{ RING_MAIL, 3, ARMOR_SYM, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

struct trobj Knight[] = {
	{ LONG_SWORD, 0, WEAPON_SYM, 1, 1 },
	{ SPEAR, 2, WEAPON_SYM, 1, 1 },
	{ RING_MAIL, 4, ARMOR_SYM, 1, 1 },
	{ HELMET, 1, ARMOR_SYM, 1, 1 },
	{ SHIELD, 1, ARMOR_SYM, 1, 1 },
	{ PAIR_OF_GLOVES, 1, ARMOR_SYM, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

struct trobj Speleologist[] = {
	{ STUDDED_LEATHER_ARMOR, 3, ARMOR_SYM, 1, 1 },
	{ UNDEF_TYP, 0, POTION_SYM, 2, 0 },
	{ FOOD_RATION, 0, FOOD_SYM, 3, 1 },
	{ ICE_BOX, 0, TOOL_SYM, 1, 0 },
	{ 0, 0, 0, 0, 0}
};

struct trobj Tourist[] = {
	{ UNDEF_TYP, 0, FOOD_SYM, 10, 1 },
	{ POT_EXTRA_HEALING, 0, POTION_SYM, 2, 0 },
	{ EXPENSIVE_CAMERA, 0, TOOL_SYM, 1, 1 },
	{ DART, 2, WEAPON_SYM, 25, 1 },	/* quan is variable */
	{ 0, 0, 0, 0, 0 }
};

struct trobj Wizard[] = {
	{ ELVEN_CLOAK, 1, ARMOR_SYM, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, WAND_SYM, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, RING_SYM, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_SYM, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_SYM, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};

#ifdef NEWS
int u_in_infl;

u_in_intrup(){
	u_in_infl++;
	(void) signal(SIGINT, u_in_intrup);
}
#endif NEWS

u_init(){
register int c,pc,i;
#ifdef NEWS
	/* It is not unlikely that we get an interrupt here
	   intended to kill the news; unfortunately this would
	   also kill (part of) the following question */
int (*prevsig)() = signal(SIGINT, u_in_intrup);
#endif NEWS
register char *cp;
char buf[256];
	if(pc = pl_character[0]) goto got_suffix;
	buf[0] = 0;
	Strcat(buf, "\nTell me what kind of character you are:\n");
	Strcat(buf, "Are you a Tourist, a Speleologist, a Fighter,\n");
	Strcat(buf, "\ta Knight, a Cave-man or a Wizard? [TSFKCW] ");
intrup:
	for(cp = buf; *cp; cp++){
#ifdef NEWS
		if(u_in_infl){
			u_in_infl = 0;
			goto intrup;
		}
#endif NEWS
		(void) putchar(*cp);
	}
loop:
	(void) fflush(stdout);
	pc = 0;
	while((c = getchar()) != '\n') {
		if(c == EOF) {
#ifdef NEWS
			if(u_in_infl) goto intrup;	/* %% */
#endif NEWS
			settty("\nEnd of input?\n");
			exit(0);
		}
 if(!pc) pc = c;
	}
	if(!pc || !index("TSFKCWtsfkcw", pc)){
		printf("Answer with T,S,F,K,C or W. What are you? ");
		goto loop;
	}
got_suffix:
	if('a' <= pc && pc <= 'z') pc += 'A'-'a';

#ifdef NEWS
	(void) signal(SIGINT,prevsig);
#endif NEWS

	u.usym = '@';
	u.ulevel = 1;
	init_uhunger();
	u.uhpmax = u.uhp = 12;
	u.ustrmax = u.ustr = !rn2(20) ? 14 + rn2(7) : 16;
#ifdef QUEST
	u.uhorizon = 6;
#endif QUEST
	switch(pc) {
	case 'C':
		setpl_char("Cave-man");
		Cave_man[2].trquan = 12 + rnd(9)*rnd(9);
		u.uhp = u.uhpmax = 16;
		u.ustr = u.ustrmax = 18;
		ini_inv(Cave_man);
		break;
	case 'T':
		setpl_char("Tourist");
		Tourist[3].trquan = 20 + rnd(20);
		u.ugold = u.ugold0 = rnd(1000);
		u.uhp = u.uhpmax = 10;
		u.ustr = u.ustrmax = 8;
		ini_inv(Tourist);
		break;
	case 'W':
		setpl_char("Wizard");
		for(i=1; i<=4; i++) if(!rn2(5))
			Wizard[i].trquan += rn2(3) - 1;
		u.uhp = u.uhpmax = 15;
		u.ustr = u.ustrmax = 16;
		ini_inv(Wizard);
		break;
	case 'S':
		setpl_char("Speleologist");
		Fast = INTRINSIC;
		Stealth = INTRINSIC;
		u.uhp = u.uhpmax = 12;
		u.ustr = u.ustrmax = 10;
		ini_inv(Speleologist);
		break;
	case 'K':
		setpl_char("Knight");
		u.uhp = u.uhpmax = 12;
		u.ustr = u.ustrmax = 10;
		ini_inv(Knight);
		break;
	case 'F':
		setpl_char("Fighter");
		u.uhp = u.uhpmax = 14;
		u.ustr = u.ustrmax = 17;
		ini_inv(Fighter);
	}
	find_ac();
	/* make sure he can carry all he has - especially for T's */
	while(inv_weight() > 0 && u.ustr < 118)
		u.ustr++, u.ustrmax++;
#ifdef WIZARD
	if(wizard) wiz_inv();
#endif WIZARD
}

ini_inv(trop) register struct trobj *trop; {
register struct obj *obj;
extern struct obj *mkobj();
	while(trop->trolet) {
		obj = mkobj(trop->trolet);
		obj->known = trop->trknown;
		obj->cursed = 0;
		if(obj->olet == WEAPON_SYM){
			obj->quan = trop->trquan;
			trop->trquan = 1;
		}
		if(trop->trspe != UNDEF_SPE)
			obj->spe = trop->trspe;
		if(trop->trotyp != UNDEF_TYP)
			obj->otyp = trop->trotyp;
		obj->owt = weight(obj);	/* defined after setting otyp+quan */
		obj = addinv(obj);
		if(obj->olet == ARMOR_SYM){
			switch(obj->otyp){
			case SHIELD:
				if(!uarms) setworn(obj, W_ARMS);
				break;
			case HELMET:
				if(!uarmh) setworn(obj, W_ARMH);
				break;
			case PAIR_OF_GLOVES:
				if(!uarmg) setworn(obj, W_ARMG);
				break;
			case ELVEN_CLOAK:
				if(!uarm2)
					setworn(obj, W_ARM);
				break;
			default:
				if(!uarm) setworn(obj, W_ARM);
			}
		}
		if(obj->olet == WEAPON_SYM)
			if(!uwep) setuwep(obj);
		if(--trop->trquan) continue;	/* make a similar object */
		trop++;
	}
}

#ifdef WIZARD
wiz_inv(){
register struct trobj *trop = &Extra_objs[0];
extern char *getenv();
register char *ep = getenv("INVENT");
register int type;
	while(ep && *ep) {
		type = atoi(ep);
		ep = index(ep, ',');
		if(ep) while(*ep == ',' || *ep == ' ') ep++;
		if(type <= 0 || type > NROFOBJECTS) continue;
		trop->trotyp = type;
		trop->trolet = objects[type].oc_olet;
		trop->trspe = 4;
		trop->trknown = 1;
		trop->trquan = 1;
		ini_inv(trop);
	}
	/* give him a wand of wishing by default */
	trop->trotyp = WAN_WISHING;
	trop->trolet = WAND_SYM;
	trop->trspe = 20;
	trop->trknown = 1;
	trop->trquan = 1;
	ini_inv(trop);
}
#endif WIZARD

setpl_char(plc) char *plc; {
	(void) strncpy(pl_character, plc, PL_CSIZ-1);
	pl_character[PL_CSIZ-1] = 0;
}

plnamesuffix() {
register char *p;
	if(p = rindex(plname, '-')) {
		*p = 0;
		if(!plname[0]) {
			askname();
			plnamesuffix();
		}
		if(index("TSFKCWtsfkcw", p[1])) {
			pl_character[0] = p[1];
			pl_character[1] = 0;
		}
	}
}
