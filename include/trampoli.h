/*	SCCS Id: @(#)trampoli.h 	3.0	89/11/15	  */
/* Copyright (c) 1989, by Norm Meluch and Stephen Spackman	  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRAMPOLI_H
#define TRAMPOLI_H

#ifdef OVERLAY

/* ### apply.c ### */
#define dig()     dig_()
#define doapply() doapply_()
#define dojump()  dojump_()
#define dorub()   dorub_()


/* ### cmd.c ### */
#define doextcmd()         doextcmd_()
#define doextlist()        doextlist_()
#ifdef POLYSELF
#define domonability()     domonability_()
#endif /* POLYSELF */
#define timed_occupation() timed_occupation_()
#if defined(WIZARD) || defined(EXPLORE_MODE)
#define wiz_attributes()   wiz_attributes_()
#endif
#ifdef WIZARD
#define wiz_detect()       wiz_detect_()
#define wiz_genesis()      wiz_genesis_()
#define wiz_identify()     wiz_identify_()
#define wiz_level_tele()   wiz_level_tele_()
#define wiz_map()          wiz_map_()
#define wiz_where()        wiz_where_()
#define wiz_wish()         wiz_wish_()
#endif


/* ### do.c ### */
#define doddrop()  doddrop_()
#define dodown()   dodown_()
#define dodrop()   dodrop_()
#define donull()   donull_()
#define doup()     doup_()
#define dowipe()   dowipe_()
#define drop(x)    drop_(x)
#define wipeoff()  wipeoff_()


/* ### do_name.c ### */
#define ddocall()  ddocall_()
#define do_mname() do_mname_()


/* ### do_wear.c ### */
#define Armor_off()   Armor_off_()
#define Boots_off()   Boots_off_()
#define Gloves_off()  Gloves_off_()
#define Helmet_off()  Helmet_off_()
#define Armor_on()    Armor_on_()
#define Boots_on()    Boots_on_()
#define Gloves_on()   Gloves_on_()
#define Helmet_on()   Helmet_on_()
#define doddoremarm() doddoremarm_()
#define doputon()     doputon_()
#define doremring()   doremring_()
#define dotakeoff()   dotakeoff_()
#define dowear()      dowear_()
#define select_off(x) select_off_(x)
#define take_off()    take_off_()


/* ### dokick.c ### */
#define dokick() dokick_()


/* ### dothrow.c ### */
#define dothrow() dothrow_()


/* ### eat.c ### */
#define Hear_again() Hear_again_()
#define Meatdone()   Meatdone_()
#define doeat()      doeat_()
#define eatfood()    eatfood_()
#define opentin()    opentin_()
#define unfaint()    unfaint_()


/* ### end.c ### */
#define done2() done2_()


/* ### engrave.c ### */
#define doengrave() doengrave_()


/* ### hack.c ### */
#define dopickup() dopickup_()
#define identify(x) identify_(x)


/* ### invent.c ### */
#define ckunpaid(x)  ckunpaid_(x)
#define ddoinv()     ddoinv_()
#define dolook()     dolook_()
#define dopramulet() dopramulet_()
#define doprarm()    doprarm_()
#define doprgold()   doprgold_()
#define doprring()   doprring_()
#define doprtool()   doprtool_()
#define doprwep()    doprwep_()
#define dotypeinv()  dotypeinv_()


/* ### ioctl.c ### */
/*
#ifdef UNIX
#ifdef SUSPEND
#define dosuspend() dosuspend_()
#endif
#endif
*/


/* ### lock.c ### */
#define doclose()   doclose_()
#define doforce()   doforce_()
#define doopen()    doopen_()
#define forcelock() forcelock_()
#define picklock()  picklock_()


/* ### o_init.c ### */
#define dodiscovered() dodiscovered_()


/* ### objnam.c ### */
#define doname(x)   doname_(x)
#define xname(x)    xname_(x)

/* ### options.c ### */
#define doset()          doset_()
#define dotogglepickup() dotogglepickup_()


/* ### pager.c ### */
#define dohelp()     dohelp_()
#define dohistory()  dohistory_()
#ifdef UNIX
#ifdef SHELL
#define dosh()       dosh_()
#endif
#endif
#define dowhatdoes() dowhatdoes_()
#define dowhatis()   dowhatis_()


/* ### pickup.c ### */
#define ck_bag()         ck_bag_()
#define ck_container(x)  ck_container_(x)
#define doloot()         doloot_()
#define in_container(x)  in_container_(x)
#define out_container(x) out_container_(x)


/* ### potion.c ### */
#define dodrink() dodrink_()
#define dodip()   dodip_()


/* ### pray.c ### */
#ifdef THEOLOGY
#define dopray()      dopray_()
#define dosacrifice() dosacrifice_()
#endif /* THEOLOGY */
#define doturn()      doturn_()


/* ### pri.c ### */
#define doredraw() doredraw_()


/* ### read.c ### */
#define doread() doread_()


/* ### save.c ### */
#define dosave() dosave_()


/* ### search.c ### */
#define doidtrap() doidtrap_()
#define dosearch() dosearch_()


/* ### shk.c ### */
#define dopay() dopay_()


/* ### sit.c ### */
#define dosit() dosit_()


/* ### sounds.c ### */
#define dotalk() dotalk_()


/* ### spell.c ### */
#ifdef SPELLS
#define learn()	learn_()
#define docast()   docast_()
#define dovspell() dovspell_()
#endif


/* ### steal.c ### */
#define stealarm() stealarm_()


/* ### topl.c ### */
#define doredotopl() doredotopl_()

/* ### trap.c ### */
#define dotele()     dotele_()
#define dountrap()   dountrap_()
#define float_down() float_down_()

/* ### version.c ### */
#define doversion() doversion_()

/* ### wield.c ### */
#define dowield() dowield_()

/* ### zap.c ### */
#define bhitm(x, y) bhitm_(x, y)
#define bhito(x, y) bhito_(x, y)
#define dozap()     dozap_()

#endif /* OVERLAY */

#endif /* TRAMPOLI_H */
