/*	SCCS Id: @(#)trampoli.c 	3.1	93/01/18	  */
/* Copyright (c) 1989 - 1993 by Norm Meluch and Stephen Spackman  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/****************************************************************************/
/*									    */
/*	This file contains a series of definitions of "one liner"	    */
/*	functions corresponding to *all* functions that NetHack calls	    */
/*	via pointers.							    */
/*	     IF ANY CALLS TO FUNCTIONS VIA POINTERS ARE ADDED		    */
/*	     TO THE CODE, AN ENTRY FOR THE FUNCTION CALLED MUST		    */
/*	     BE ADDED TO THIS FILE AND TO TRAMPOLI.H.			    */
/*	This mess is necessary for the Microsoft Compiler implementation    */
/*	of overlaid code (v5.1 - 6.0ax).				    */
/*									    */
/*	The original function (eg foo) has been #defined to be foo_	    */
/*	via double inclusion of trampoli.h in hack.h, and		    */
/*	now the definition of foo is placed in this file calling foo	    */
/*	directly.  This module is _never_ placed in an overlay so	    */
/*	calls via pointers to these functions will not cause difficulties.  */
/*									    */
/*	This leads to what could be called a "trampoline" effect, and	    */
/*	hence the silly name for these files. :-)			    */
/*									    */
/****************************************************************************/

#ifdef OVERLAY

/* ### apply.c ### */
#undef dig
#undef doapply
#undef dojump
#undef dorub

int dig()		{ return dig_(); }
int doapply()		{ return doapply_(); }
int dojump()		{ return dojump_(); }
int dorub()		{ return dorub_(); }


/* ### artifact.c ### */
#undef doinvoke

int doinvoke()		{ return doinvoke_(); }


/* ### cmd.c ### */
#undef doextcmd
#undef doextlist
#undef doprev_message

#ifdef POLYSELF
#undef domonability
#endif /* POLYSELF */

#ifdef EXPLORE_MODE
#undef enter_explore_mode

int enter_explore_mode()	{ return enter_explore_mode_(); }
#endif

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

int doextcmd()		{ return doextcmd_(); }
int doextlist()		{ return doextlist_(); }
int doprev_message()	{ return doprev_message_(); }

#ifdef POLYSELF
int domonability()	{ return domonability_(); }
#endif /* POLYSELF */

int timed_occupation()	{ return timed_occupation_(); }

#if defined(WIZARD) || defined(EXPLORE_MODE)
int wiz_attributes()	{ return wiz_attributes_(); }
#endif

#ifdef WIZARD
int wiz_detect()	{ return wiz_detect_(); }
int wiz_genesis()	{ return wiz_genesis_(); }
int wiz_identify()	{ return wiz_identify_(); }
int wiz_level_tele()	{ return wiz_level_tele_(); }
int wiz_map()		{ return wiz_map_(); }
int wiz_where()		{ return wiz_where_(); }
int wiz_wish()		{ return wiz_wish_(); }
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

int doddrop()			{ return doddrop_(); }
int dodown()			{ return dodown_(); }
int dodrop()			{ return dodrop_(); }
int donull()			{ return donull_(); }
int doup()			{ return doup_(); }
int dowipe()			{ return dowipe_(); }
int drop(obj)
  register struct obj *obj;	{ return drop_(obj); }
int wipeoff()			{ return wipeoff_(); }


/* ### do_name.c ### */
#undef ddocall
#undef do_mname

int ddocall()		{ return ddocall_(); }
int do_mname()		{ return do_mname_(); }


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

int Armor_off()		{ return Armor_off_(); }
int Boots_off()		{ return Boots_off_(); }
int Gloves_off()	{ return Gloves_off_(); }
int Helmet_off()	{ return Helmet_off_(); }
int Armor_on()		{ return Armor_on_(); }
int Boots_on()		{ return Boots_on_(); }
int Gloves_on()		{ return Gloves_on_(); }
int Helmet_on()		{ return Helmet_on_(); }
int doddoremarm()	{ return doddoremarm_(); }
int doputon()		{ return doputon_(); }
int doremring()		{ return doremring_(); }
int dotakeoff()		{ return dotakeoff_(); }
int dowear()		{ return dowear_(); }
int select_off(otmp)
  struct obj *otmp;	{ return select_off_(otmp); }
int take_off()		{ return take_off_(); }


/* ### dogmove.c ### */
#undef wantdoor

void wantdoor(x, y, dummy)
  int x, y; genericptr_t dummy; { wantdoor_(x, y, dummy); }


/* ### dokick.c ### */
#undef dokick

int dokick()		{ return dokick_(); }


/* ### dothrow.c ### */
#undef dothrow

int dothrow()		{ return dothrow_(); }


/* ### eat.c ### */
#undef Hear_again
#undef eatmdone
#undef doeat
#undef eatfood
#undef opentin
#undef unfaint

int Hear_again()	{ return Hear_again_(); }
int eatmdone()		{ return eatmdone_(); }
int doeat()		{ return doeat_(); }
int eatfood()		{ return eatfood_(); }
int opentin()		{ return opentin_(); }
int unfaint()		{ return unfaint_(); }


/* ### end.c ### */
#undef done1
#undef done2
#undef hangup
#undef done_intr

#if defined(UNIX) || defined(VMS)
#undef done_hangup
#endif /* UNIX || VMS */

int done1()		{ return done1_(); }
int done2()		{ return done2_(); }
int hangup()		{ return hangup_(); }
int done_intr()		{ return done_intr_(); }

#if defined(UNIX) || defined(VMS)
int done_hangup()	{ return done_hangup_(); }
#endif /* UNIX || VMS */


/* ### engrave.c ### */
#undef doengrave

int doengrave()		{ return doengrave_(); }


/* ### fountain.c ### */
#undef gush

void gush(x, y, poolcnt)
  int x, y; genericptr_t poolcnt; { gush_(x, y, poolcnt); }


/* ### hack.c ### */
#undef dopickup
#undef identify

int dopickup()		{ return dopickup_(); }
int identify(otmp)
  struct obj *otmp;	{ return identify_(otmp); }


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
#undef doorganize

int ckunpaid(obj)
  struct obj *obj;	{ return ckunpaid_(obj); }
int ddoinv()		{ return ddoinv_(); }
int dolook()		{ return dolook_(); }
int dopramulet()	{ return dopramulet_(); }
int doprarm()		{ return doprarm_(); }
int doprgold()		{ return doprgold_(); }
int doprring()		{ return doprring_(); }
int doprtool()		{ return doprtool_(); }
int doprwep()		{ return doprwep_(); }
int dotypeinv()		{ return dotypeinv_(); }
int doorganize()	{ return doorganize_(); }


/* ### ioctl.c ### */
#ifdef UNIX
# ifdef SUSPEND
#undef dosuspend

int dosuspend()		{ return dosuspend_(); }
# endif /* SUSPEND */
#endif /* UNIX */


/* ### lock.c ### */
#undef doclose
#undef doforce
#undef doopen
#undef forcelock
#undef picklock

int doclose()		{ return doclose_(); }
int doforce()		{ return doforce_(); }
int doopen()		{ return doopen_(); }
int forcelock()		{ return forcelock_(); }
int picklock()		{ return picklock_(); }


/* ### mklev.c ### */
#undef do_comp

int do_comp(vx, vy)
  genericptr_t vx, vy;	{ return comp_(vx, vy); }


/* ### mondata.c ### */
/* canseemon() is only called by a macro e_boolean.  If e_boolean ever does
   become a function, this may need to return. */

/* #undef canseemon */

/* boolean canseemon(x) struct monst *x; { return canseemon_(x); } */


/* ### muse.c ### */
#undef mbhitm

int mbhitm(mtmp, otmp)
  struct monst *mtmp; struct obj *otmp; { return mbhitm_(mtmp, otmp); }


/* ### o_init.c ### */
#undef dodiscovered

int dodiscovered()	{ return dodiscovered_(); }


/* ### objnam.c ### */
#undef doname
#undef xname

char *doname(obj)
  struct obj *obj;	{ return doname_(obj); }
char *xname(obj)
  struct obj *obj;	{ return xname_(obj); }


/* ### options.c ### */
#undef doset
#undef dotogglepickup

int doset()		{ return doset_(); }
int dotogglepickup()	{ return dotogglepickup_(); }


/* ### pager.c ### */
#undef dohelp
#undef dohistory
#undef dowhatdoes
#undef dowhatis
#undef doquickwhatis

int dohelp()		{ return dohelp_(); }
int dohistory()		{ return dohistory_(); }
int dowhatdoes()	{ return dowhatdoes_(); }
int dowhatis()		{ return dowhatis_(); }
int doquickwhatis()	{ return doquickwhatis_(); }


/* ### pcsys.c ### */
#ifdef SHELL
#undef dosh

int dosh()		{ return dosh_(); }
#endif /* SHELL */


/* ### pickup.c ### */
#undef ck_bag
#undef doloot
#undef in_container
#undef out_container

int ck_bag(obj)
  struct obj *obj;	{ return ck_bag_(obj);  }
int doloot()		{ return doloot_(); }
int in_container(obj)
  struct obj *obj;	{ return in_container_(obj); }
int out_container(obj)
  struct obj *obj;	{ return out_container_(obj); }


/* ### potion.c ### */
#undef dodrink
#undef dodip

int dodrink()		{ return dodrink_(); }
int dodip()		{ return dodip_(); }


/* ### pray.c ### */
#undef doturn
#undef dopray
#undef prayer_done
#undef dosacrifice

int doturn()		{ return doturn_(); }
int dopray()		{ return dopray_(); }
int prayer_done()	{ return prayer_done_(); }
int dosacrifice()	{ return dosacrifice_(); }


/* ### print.c ### */
#undef doredraw

int doredraw()		{ return doredraw_(); }


/* ### read.c ### */
#undef doread
#undef set_lit

int doread()		{ return doread_(); }
void set_lit(x, y, val)
  int x, y; genericptr_t val; { set_lit_(x, y, val); }


/* ### rip.c ### */
#undef genl_outrip

void genl_outrip(tmpwin, how)
  winid tmpwin; int how; { genl_outrip_(tmpwin, how); }


/* ### save.c ### */
#undef dosave

int dosave()		{ return dosave_(); }


/* ### search.c ### */
#undef findone
#undef openone
#undef doidtrap
#undef dosearch

void findone(zx, zy, num)
  int zx, zy; genericptr_t num; { findone_(zx, zy, num); }
void openone(zx, zy, num)
  int zx, zy; genericptr_t num; { openone_(zx, zy, num); }
int doidtrap()		{ return doidtrap_(); }
int dosearch()		{ return dosearch_(); }


/* ### shk.c ### */
#undef dopay

int dopay()		{ return dopay_(); }


/* ### sit.c ### */
#undef dosit

int dosit()		{ return dosit_(); }


/* ### sounds.c ### */
#undef dotalk

int dotalk()		{ return dotalk_(); }


/* ### spell.c ### */
#undef learn
#undef docast
#undef dovspell

int learn()		{ return learn_(); }
int docast()		{ return docast_(); }
int dovspell()		{ return dovspell_(); }


/* ### steal.c ### */
#undef stealarm

int stealarm()		{ return stealarm_(); }


/* ### trap.c ### */
#undef dotele
#undef dountrap
#undef float_down

int dotele()		{ return dotele_(); }
int dountrap()		{ return dountrap_(); }
int float_down()	{ return float_down_(); }


/* ### version.c ### */
#undef doversion
#undef doextversion

int doversion()		{ return doversion_(); }
int doextversion()	{ return doextversion_(); }


/* ### wield.c ### */
#undef dowield

int dowield()		{ return dowield_(); }


/* ### zap.c ### */
#undef bhitm
#undef bhito
#undef dozap

int bhitm(mtmp, otmp)
  struct monst *mtmp; struct obj *otmp; { return bhitm_(mtmp, otmp); }
int bhito(obj, otmp)
  struct obj *obj, *otmp; { return bhito_(obj,  otmp); }
int dozap()		{ return dozap_(); }


/*
 * Window Implementation Specific Functions.
 */

/* ### getline.c ### */
#undef tty_getlin
#ifdef COM_COMPL
#undef tty_get_ext_cmd

void tty_get_ext_cmd(bufp)
  char *bufp;			{ tty_get_ext_cmd_(bufp); }
#endif /* COM_COMPL */
void tty_getlin(query,bufp)
  const char *query; char *bufp;{ tty_getlin_(query,bufp); }


/* ### termcap.c ### */
#undef tty_nhbell
#undef tty_number_pad
#undef tty_delay_output
#undef tty_start_screen
#undef tty_end_screen

void tty_nhbell()		{ tty_nhbell_(); }
void tty_number_pad(state)
  int state;			{ tty_number_pad_(state); }
void tty_delay_output()		{ tty_delay_output_(); }
/* other defs that really should go away (they're tty specific) */
void tty_start_screen()		{ tty_start_screen_(); }
void tty_end_screen()		{ tty_end_screen_(); }


/* ### topl.c ### */
#undef tty_doprev_message
#undef tty_yn_function

int tty_doprev_message()	{ return tty_doprev_message_(); }
char tty_yn_function(query,resp,def)
  const char *query, *resp; char def;
				{ return tty_yn_function_(query,resp,def); }


/* ### wintty.c ### */
#undef tty_init_nhwindows
#undef tty_player_selection
#undef tty_askname
#undef tty_get_nh_event
#undef tty_exit_nhwindows
#undef tty_suspend_nhwindows
#undef tty_resume_nhwindows
#undef tty_create_nhwindow
#undef tty_clear_nhwindow
#undef tty_display_nhwindow
#undef tty_destroy_nhwindow
#undef tty_curs
#undef tty_putstr
#undef tty_display_file
#undef tty_start_menu
#undef tty_add_menu
#undef tty_end_menu
#undef tty_select_menu
#undef tty_update_inventory
#undef tty_mark_synch
#undef tty_wait_synch
#ifdef CLIPPING
#undef tty_cliparound
#endif
#undef tty_print_glyph
#undef tty_raw_print
#undef tty_raw_print_bold
#undef tty_nhgetch
#undef tty_nh_poskey

void tty_init_nhwindows()	{ tty_init_nhwindows_(); }
void tty_player_selection()	{ tty_player_selection_(); }
void tty_askname()		{ tty_askname_(); }
void tty_get_nh_event()		{ tty_get_nh_event_(); }
void tty_exit_nhwindows(str)
  const char *str;		{ tty_exit_nhwindows_(str); }
void tty_suspend_nhwindows(str)
  const char *str;		{ tty_suspend_nhwindows_(str); }
void tty_resume_nhwindows()	{ tty_resume_nhwindows_(); }
winid tty_create_nhwindow(type)
  int type;			{ return tty_create_nhwindow_(type); }
void tty_clear_nhwindow(window)
  winid window;			{ tty_clear_nhwindow_(window); }
void tty_display_nhwindow(window, blocking)
  winid window; boolean blocking;
				{ tty_display_nhwindow_(window,blocking); }
void tty_destroy_nhwindow(window)
  winid window;			{ tty_destroy_nhwindow_(window); }
void tty_curs(window,x,y)
  winid window; int x,y;	{ tty_curs_(window,x,y); }
void tty_putstr(window,attr,str)
  winid window; int attr; const char *str;
				{ tty_putstr_(window,attr,str); }
void tty_display_file(fname, complain)
  const char *fname; boolean complain;
				{ tty_display_file_(fname,complain); }
void tty_start_menu(window)
  winid window;			{ tty_start_menu_(window); }
void tty_add_menu(window,ch,attr,str)
  winid window; char ch; int attr; const char *str;
				{ tty_add_menu_(window,ch,attr,str); }
void tty_end_menu(window,ch,str,morestr)
  winid window; char ch; const char *str, *morestr;
				{ tty_end_menu_(window,ch,str,morestr); }
char tty_select_menu(window)
  winid window;			{ return tty_select_menu_(window); }
void tty_update_inventory()	{ tty_update_inventory_(); }
void tty_mark_synch()		{ tty_mark_synch_(); }
void tty_wait_synch()		{ tty_wait_synch_(); }
#ifdef CLIPPING
void tty_cliparound(x,y)
  int x,y;			{ tty_cliparound_(x,y); }
#endif
void tty_print_glyph(window,x,y,glyph)
  winid window; xchar x,y; int glyph;
				{ tty_print_glyph_(window,x,y,glyph); }
void tty_raw_print(str)
  const char *str;		{ tty_raw_print_(str); }
void tty_raw_print_bold(str)
  const char *str;		{ tty_raw_print_bold_(str); }
int tty_nhgetch()		{ return tty_nhgetch_(); }
int tty_nh_poskey(x,y,pos)
  int *x,*y,*pos;		{ return tty_nh_poskey_(x,y,pos); }

#endif /* OVERLAY */
