/*	SCCS Id: @(#)trampoli.c 	3.0	89/11/15	  */
/* Copyright (c) 1989, by Norm Meluch and Stephen Spackman	  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/****************************************************************************/
/*									    */
/*	This file contains a series of declarations of "one liner"	    */
/*	functions.  These functions are to avoid calls to functions via     */
/*	pointers.  This is necessary when the function called is in an      */
/*	overlay segment.						    */
/*	The original function (eg foo) has been defined to be foo_ and	    */
/*	now the declaration of foo is placed in this file calling foo	    */
/*	directly.  This module is _never_ placed in an overlay so	    */
/*	calls via pointers to these functions will not cause difficulties.  */
/*									    */
/****************************************************************************/

#ifdef OVERLAY

/* ### apply.c ### */
#undef dig
#undef doapply
#undef dojump
#undef dorub

int dig()     { return dig_();     }
int doapply() { return doapply_(); }
int dojump()  { return dojump_();  }
int dorub()   { return dorub_();   }


/* ### cmd.c ### */
#undef doextcmd
#undef doextlist

#ifdef POLYSELF
#undef domonability
#endif /* POLYSELF */

#undef timed_occupation

#if defined(WIZARD) || defined(EXPLORE_MODE)
#undef wiz_attributes
#endif

#ifdef WIZARD
#undef wiz_detect
#undef wiz_genesis
#undef wiz_identify
#undef wiz_level_tele
#undef wiz_map
#undef wiz_where
#undef wiz_wish
#endif

int doextcmd()         { return doextcmd_();         }
int doextlist()        { return doextlist_();        }

#ifdef POLYSELF
int domonability()     { return domonability_();     }
#endif /* POLYSELF */

int timed_occupation() { return timed_occupation_(); }

#if defined(WIZARD) || defined(EXPLORE_MODE)
int wiz_attributes()   { return wiz_attributes_();   }
#endif

#ifdef WIZARD
int wiz_detect()       { return wiz_detect_();       }
int wiz_genesis()      { return wiz_genesis_();      }
int wiz_identify()     { return wiz_identify_();     }
int wiz_level_tele()   { return wiz_level_tele_();   }
int wiz_map()          { return wiz_map_();          }
int wiz_where()        { return wiz_where_();        }
int wiz_wish()         { return wiz_wish_();         }
#endif


/* ### do.c ### */
#undef doddrop
#undef dodown
#undef dodrop
#undef donull
#undef doup
#undef dowipe
#undef drop
#undef wipeoff

int doddrop()             { return doddrop_(); }
int dodown()              { return dodown_();  }
int dodrop()              { return dodrop_();  }
int donull()              { return donull_();  }
int doup()                { return doup_();    }
int dowipe()              { return dowipe_();  }
int drop(obj)
register struct obj *obj; { return drop_(obj); }
int wipeoff()             { return wipeoff_(); }


/* ### do_name.c ### */
#undef ddocall
#undef do_mname

int ddocall()  { return ddocall_();  }
int do_mname() { return do_mname_(); } 


/* ### do_wear.c ### */

/* ### do_wear.c ### */
#undef Armor_off
#undef Boots_off
#undef Gloves_off
#undef Helmet_off
#undef Armor_on
#undef Boots_on
#undef Gloves_on
#undef Helmet_on
#undef doddoremarm
#undef doputon
#undef doremring
#undef dotakeoff
#undef dowear
#undef select_off
#undef take_off

int Armor_off()   { return Armor_off_();   }
int Boots_off()   { return Boots_off_();   }
int Gloves_off()  { return Gloves_off_();  }
int Helmet_off()  { return Helmet_off_();  }
/* int Armor_on()    { return Armor_on_();    } */
int Boots_on()    { return Boots_on_();    }
int Gloves_on()   { return Gloves_on_();   }
int Helmet_on()   { return Helmet_on_();   }
int doddoremarm() { return doddoremarm_(); }
int doputon()     { return doputon_();     }
int doremring()   { return doremring_();   }
int dotakeoff()   { return dotakeoff_();   }
int dowear()      { return dowear_();      }
int select_off(otmp) struct obj *otmp; { return select_off_(otmp); }
int take_off()    { return take_off_();    }


/* ### dokick.c ### */
#undef dokick

int dokick() { return dokick_(); }


/* ### dothrow.c ### */
#undef dothrow

int dothrow() { return dothrow_(); }


/* ### eat.c ### */
#undef Hear_again
#undef Meatdone
#undef doeat
#undef eatfood
#undef opentin
#undef unfaint

int Hear_again()  { return Hear_again_(); }
int Meatdone()    { return Meatdone_();   }
int doeat()       { return doeat_();      }
int eatfood()     { return eatfood_();    }
int opentin()     { return opentin_();    }
int unfaint()     { return unfaint_();    }


/* ### end.c ### */
#undef done2

int done2() { return done2_(); }


/* ### engrave.c ### */
#undef doengrave

int doengrave() { return doengrave_(); }


/* ### hack.c ### */
#undef dopickup
#undef identify

int dopickup() { return dopickup_(); }
int identify(otmp) struct obj *otmp; { return identify_(otmp); }


/* ### invent.c ### */
#undef ckunpaid
#undef ddoinv
#undef dolook
#undef dopramulet
#undef doprarm
#undef doprgold
#undef doprring
#undef doprtool
#undef doprwep
#undef dotypeinv

int ckunpaid(obj) struct obj *obj; { return ckunpaid_(obj); }
int ddoinv()     { return ddoinv_();     }
int dolook()     { return dolook_();     }
int dopramulet() { return dopramulet_(); }
int doprarm()    { return doprarm_();    }
int doprgold()   { return doprgold_();   }
int doprring()   { return doprring_();   }
int doprtool()   { return doprtool_();   }
int doprwep()    { return doprwep_();    }
int dotypeinv()  { return dotypeinv_();  }


/* ### ioctl.c ### */
/*
#ifdef UNIX
#ifdef SUSPEND
#undef dosuspend

int dosuspend() { return dosuspend_(); }
#endif
#endif
*/


/* ### lock.c ### */
#undef doclose
#undef doforce
#undef doopen
#undef forcelock
#undef picklock

int doclose()   { return doclose_();   }
int doforce()   { return doforce_();   }
int doopen()    { return doopen_();    }
int forcelock() { return forcelock_(); }
int picklock()  { return picklock_();  }


/* ### o_init.c ### */
#undef dodiscovered

int dodiscovered() { return dodiscovered_(); }


/* ### objnam.c ### */
#undef doname
#undef xname

char *doname(obj) struct obj *obj; { return doname_(obj); }
char *xname(obj)  struct obj *obj; { return xname_(obj); }


/* ### options.c ### */
#undef doset
#undef dotogglepickup

int doset()          { return doset_();         }
int dotogglepickup() { return dotogglepickup_(); }


/* ### pager.c ### */
#undef dohelp
#undef dohistory
#undef dowhatdoes
#undef dowhatis
#ifdef UNIX
#ifdef SHELL
#undef dosh

int dosh()       { return dosh_();       }
#endif
#endif

int dohelp()     { return dohelp_();     }
int dohistory()  { return dohistory_();  }
int dowhatdoes() { return dowhatdoes_(); }
int dowhatis()   { return dowhatis_();   }


/* ### pickup.c ### */
#undef ck_bag
#undef ck_container
#undef doloot
#undef in_container
#undef out_container

int ck_bag()  { return ck_bag_();  }
int ck_container(obj)  struct obj *obj; { return ck_container_(obj); }
int doloot() { return doloot_(); }
int in_container(obj)  struct obj *obj; { return in_container_(obj); }
int out_container(obj) struct obj *obj; { return out_container_(obj); }


/* ### potion.c ### */
#undef dodrink
#undef dodip

int dodrink() { return dodrink_(); }
int dodip()   { return dodip_();   }


/* ### pray.c ### */
#undef doturn
#ifdef THEOLOGY
#undef dopray
#undef dosacrifice

int dopray()      { return dopray_();      }
int dosacrifice() { return dosacrifice_(); }
#endif /* THEOLOGY */

int doturn()      { return doturn_();      }


/* ### pri.c ### */
#undef doredraw

int doredraw()    { return doredraw_(); }


/* ### read.c ### */
#undef doread

int doread() { return doread_(); }


/* ### save.c ### */
#undef dosave

int dosave() { return dosave_(); }


/* ### search.c ### */
#undef doidtrap
#undef dosearch

int doidtrap() { return doidtrap_(); }
int dosearch() { return dosearch_(); }


/* ### shk.c ### */
#undef dopay

int dopay() { return dopay_(); }


/* ### sit.c ### */
#undef dosit

int dosit() { return dosit_(); }


/* ### sounds.c ### */
#undef dotalk

int dotalk() { return dotalk_(); }


/* ### spell.c ### */
#ifdef SPELLS
#undef learn
#undef docast
#undef dovspell

int learn() { return learn_(); }
int docast()   { return docast_();   }
int dovspell() { return dovspell_(); }
#endif


/* ### steal.c ### */
#undef stealarm

int stealarm() { return stealarm_(); }


/* ### topl.c ### */
#undef doredotopl

int doredotopl() { return doredotopl_(); }


/* ### trap.c ### */
#undef dotele
#undef dountrap
#undef float_down

int dotele()     { return dotele_();     }
int dountrap()   { return dountrap_();   }
int float_down() { return float_down_(); }


/* ### version.c ### */
#undef doversion

int doversion() { return doversion_(); }


/* ### wield.c ### */
#undef dowield

int dowield() { return dowield_(); }


/* ### zap.c ### */
#undef bhitm
#undef bhito
#undef dozap

int bhitm(mtmp, otmp) struct monst *mtmp; struct obj *otmp;
	{ return bhitm_(mtmp, otmp); }
int bhito(obj, otmp) struct obj *obj, *otmp; { return bhito_(obj,  otmp); }
int dozap() { return dozap_(); }
#endif /* OVERLAY */
