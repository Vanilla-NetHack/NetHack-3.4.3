/*	SCCS Id: @(#)polyself.c	2.3	88/01/21
/* Polymorph self routine.  Called in zap.c. Copyright 1987 by Ken Arromdee */

#include "hack.h"

#ifdef KAA

extern char genocided[];
extern char pl_character[PL_CSIZ];
extern char plname[PL_NSIZ];

extern long newuexp();

polyself()
{
	char buf[BUFSZ];
	int tmp, tmp2, mntmp;

#ifdef BVH
	if(!Polymorph_control) {
#endif
	    if (rn2(5)-3 > u.uluck) {
		pline("You shudder for a moment.");
		losehp(rn2(30),"system shock");
		return;
	    }
#ifdef BVH
	} else {
	    char buf[BUFSZ];
	    int i;

	    pline("Type the letter of the monster to become: ");
	    getlin(buf);
	    for(i=0; i<CMNUM; ++i)
		if(mons[i].mlet == *buf) {
		    mntmp = i;
		    goto gotone;
		}
	}
#endif
	mntmp = rn2(CMNUM);
gotone:
/* We want to disallow certain monsters, but also allow humans. */
	if (index("w:",mons[mntmp].mlet) || !rn2(5)) {
	    if (!rn2(10)) flags.female = !flags.female;
	    tmp = u.uhpmax;
	    tmp2 = u.ulevel;
	    u.usym = '@';
	    prme();
	    u.mtimedone = u.mh = u.mhmax = 0;
	    u.ulevel = u.ulevel-2+rn2(5);
	    if (u.ulevel > 127 || u.ulevel == 0) u.ulevel = 1;
	    if (u.ulevel > 14) u.ulevel = 14;
	    if (u.ulevel == 1) u.uexp = rnd(10);
	    else {  /* For the new experience level, random EXP. */
		u.ulevel--;     
		u.uexp = newuexp();
		u.uexp += rn2(u.uexp);
		u.ulevel++;
	    }
	    u.uhpmax = (u.uhpmax-10)*u.ulevel/tmp2 + 19 - rn2(19);
/* If it was u.uhpmax*u.ulevel/tmp+9-rn2(19), then a 1st level character
   with 16 hp who polymorphed into a 3rd level one would have an average
   of 48 hp.  */
	    u.uhp = u.uhp*u.uhpmax/tmp;
	    tmp = u.ustrmax;
	    u.ustrmax += (rn2(5)-2);
	    if (u.ustrmax > 118) u.ustrmax = 118;
	    if (u.ustrmax < 3) u.ustrmax = 3;
	    u.ustr = u.ustr * u.ustrmax / tmp;
	    if (u.ustr < 3) u.ustr = 3;  /* > 118 is impossible */
	    u.uhunger = 500 + rn2(500);
	    Sick = 0;
	    Stoned = 0;
	    if (u.uhp <= 0 || u.uhpmax <= 0) {
#ifdef BVH
		if(Polymorph_control) {
		    u.uhp = (u.uhp <= 0) ? 1 : u.uhp;
		    u.uhpmax = (u.uhpmax <= 0) ? 1  : u.uhpmax;
		} else {
#endif
		    killer="unsuccessful polymorph";
		    done("died");
#ifdef BVH
		}
#endif
	    }
	    pline("You feel like a new %sman!", flags.female ? "wo" : "");
newname:    more();
	    do {
		pline("What is your new name? ");
		getlin(buf);
	    } while (buf[0]=='\033' || buf[0]==0);
	    if (!strcmp(plname,buf)) {
		pline("That is the same as your old name!");
		goto newname;
	    }
	    (void)strncpy(plname, buf, sizeof(plname)-1);
	    flags.botl = 1;
	    find_ac();
	} else {
	    if (index(genocided,mons[mntmp].mlet)) {
		pline("You feel rather %sish.",mons[mntmp].mname);
		return;
	    }
	    if(u.usym == '@') {
		u.mstr = u.ustr;
		u.mstrmax = u.ustrmax;
	    }
	    u.umonnum = mntmp;
	    u.usym = mons[mntmp].mlet;
	    if(index("CDelmoPTUVXYz9", u.usym)) u.ustr = u.ustrmax = 118;
	    if (u.usym == 'D') u.mhmax = 80;
	    else if (!(mons[mntmp].mlevel)) u.mhmax = rnd(4);
	    else u.mhmax = d(mons[mntmp].mlevel,8);
	    u.mh = u.mhmax;
	    pline("You turn into a%s %s!", index("aeioOU",u.usym) ? "n" : "",
		  mons[mntmp].mname);
	    break_armor(u.usym);
	    drop_weapon(u.usym);
	    prme();
	    u.mtimedone = 500 + rn2(500);
	    flags.botl = 1;
	    if (u.usym == 'D')
		pline("Use the command #breathe to breathe fire.");
	    if (u.usym == 'N')
		pline("Use the command #remove if you have to remove an iron ball.");
	    find_ac();
	}
	if (Inhell && !Fire_resistance) {
	    pline("You burn to a crisp.");
	    killer = "unwise polymorph";
	    done("died");
	}
}

break_armor(turninto)
char turninto;
{
     struct obj *otmp;
     if (uarm) {
	if (index("CDMPRUXYdejlouz,'9", turninto)) {
	    pline("The transformation causes you to %s out of your armor!",
		   (uarm2 || uarm->otyp != ELVEN_CLOAK) ? "break" : "tear");
#ifdef SHIRT
	    if (uarmu) useup(uarmu);
#endif
	    if (uarm2) useup(uarm2);
	    useup(uarm);
	} else	if (index("abcfghikpqrstvxyABEFJQS", turninto)) {
		pline("Your armor falls around you!");
		if (otmp = uarm2) {
		    setworn((struct obj *)0,otmp->owornmask & W_ARM2);
		    dropx(otmp);
		}
		otmp = uarm;
		setworn((struct obj *)0, otmp->owornmask & W_ARM);
		dropx(otmp);
	}
     }
#ifdef SHIRT
     else if (uarmu) {
		pline("The transformation causes you to tear out of your shirt!");
		if (uarmu) useup(uarmu);
	}
#endif
     if (!index("enozCGHIKLNOTUVWXYZ&',", turninto)) {
	  if (otmp = uarmg) {
	       pline("You drop your gloves!");
	       setworn((struct obj *)0, otmp->owornmask & W_ARMG);
	       dropx(otmp);
	       drop_weapon('a'); /* the 'a' is dummy to ensure dropping */
	  }
	  if (otmp = uarms) {
	       pline("You can no longer hold your shield!");
	       setworn((struct obj *)0, otmp->owornmask & W_ARMS);
	       dropx(otmp);
	  }
	  if (otmp = uarmh) {
	       pline("Your helmet falls to the floor!");
	       setworn((struct obj *)0, otmp->owornmask & W_ARMH);
	       dropx(otmp);
	  }
     }
}

drop_weapon(turninto) 
char turninto;
{
     struct obj *otmp;
     if (otmp = uwep) {
	  if (cantwield(turninto)) {
	       pline("You find you must drop your weapon!");
	       setuwep((struct obj *)0);
	       dropx(otmp);
	  }
     }
}

cantwield(c)  /* creature type c cannot wield a weapon */
char c;
{
     return(!!index("abcdfgjklpqrsuvxyABEFJPRS',",c));
}

cantweararm(c)   /* creature type c cannot wear armor */
char c;
{
     return(!index("@nGHIKLNOTVWZ&',",c));
}

humanoid(c)   /* creature type c has hands */
char c;
{
	return(!!index("@ehintCGHIKLMNOQTVWZ&",c));
}

rehumanize()
{
	u.mh = u.mhmax = u.mtimedone = 0;
	u.ustr = u.mstr;
	u.ustrmax = u.mstrmax;
	u.usym = '@';
	prme();
	pline("You return to %sn form!",(pl_character[0]=='E')?"elve":"huma");

	if (u.uhp < 1)	done("died");
	if (!Fire_resistance && Inhell) {
	    pline("You burn to a crisp.");
	    killer = "dissipating polymorph spell";
	   done("died");
	}
	flags.botl = 1;
	find_ac();
}

dobreathe()
{
     if (u.usym == 'D') {
	  if(!getdir(1)) return(0);
	  if (rn2(4))
	       pline("You exhale a bit of smoke.");
	  else buzz(20, u.ux, u.uy, u.dx, u.dy);
     /* Changes must be made in zap.c to accommodate this. */
     } else pline("You do not have the ability to breathe fire!");
     return(1);
}

doremove()
{
     if (!Punished) {
	  pline("You do not have a ball attached to your leg!");
	  return(0);
     }
     if(u.usym != 'N')
	  pline("You are not capable of removing a locked chain!");
     else {
	  Punished = 0;
	  uchain->spe = 0;
	  uball->spe = 0;
	  uchain->owornmask &= ~W_CHAIN;
	  uball->owornmask &= ~W_BALL;
	  uchain = uball = (struct obj *)0;
     }
     return(1);
}
#endif
