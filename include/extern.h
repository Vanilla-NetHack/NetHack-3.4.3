/*	SCCS Id: @(#)extern.h	3.0	88/10/18 */
/* 	Copyright (c)   Steve Creps, 1988. 		 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

#if defined(MSDOS) && defined(MSC)
/* MSC include files do not contain "extern" keyword */
#define E
#else
#define E extern
#endif

/* ### allmain.c ### */

E int (*occupation)();
E int (*afternmv)();
E void moveloop();
E void stop_occupation();
E void newgame();

/* ### alloc.c ### */

#ifndef __TURBOC__
E long *FDECL(alloc, (unsigned int));
#endif

#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### apply.c ### */

#ifdef OVERLAY
E int dig();
#endif
E int doapply();
E int holetime();
E void dighole();
E int dorub();
E int dojump();
#ifdef WALKIES
E int number_leashed();
E void FDECL(o_unleash, (struct obj *));
E void FDECL(m_unleash, (struct monst *));
E void unleash_all();
E boolean next_to_u();
E struct obj *FDECL(get_mleash, (struct monst *));
E void FDECL(check_leash, (XCHAR_P,XCHAR_P));
#endif
E boolean FDECL(um_dist, (XCHAR_P,XCHAR_P,XCHAR_P));
E int FDECL(use_unicorn_horn, (struct obj *));

/* ### artifact.c ### */

#ifdef NAMED_ITEMS
E void FDECL(mkartifact, (struct obj **));
E boolean FDECL(is_artifact, (struct obj *));
E boolean FDECL(spec_ability, (struct obj *,unsigned));
E int FDECL(restr_name, (struct obj *,char *));
# if defined(THEOLOGY) && defined(ALTARS)
E struct obj *FDECL(mk_aligned_artifact, (unsigned));
# endif
E int FDECL(defends, (int,struct obj *));
E int FDECL(spec_abon, (struct obj *,struct permonst *));
E int FDECL(spec_dbon, (struct obj *,struct permonst *,int));
#endif

/* ### attrib.c ### */

E void FDECL(adjattrib, (int,int,BOOLEAN_P));
E void FDECL(change_luck, (SCHAR_P));
E int FDECL(stone_luck, (BOOLEAN_P));
E void FDECL(gainstr, (struct obj *,int));
E void FDECL(losestr, (int));
E void restore_attrib();
E void FDECL(init_attr, (int));
E void redist_attr();
E void FDECL(adjabil, (int));
E int newhp();
E schar acurr();
E void FDECL(adjalign, (int));

/* ### bones.c ### */

E void savebones();
E int getbones();
E void FDECL(name_file, (char *,int));

/* ### cmd.c ### */

#ifdef OVERLAY
E int doextcmd();
#ifdef POLYSELF
E int domonability();
#endif /* POLYSELF */
E int timed_occupation();
#if defined(WIZARD) || defined(EXPLORE_MODE)
E int wiz_attributes();
#endif /* WIZARD */
#ifdef WIZARD
E int wiz_detect();
E int wiz_genesis();
E int wiz_identify();
E int wiz_level_tele();
E int wiz_map();
E int wiz_where();
E int wiz_wish();
#endif /* WIZARD */
#endif /* OVERLAY */
E void reset_occupations();
E void FDECL(set_occupation, (int(*)(),char *,int));
#ifdef REDO
E char pgetchar();
E void FDECL(pushch, (CHAR_P));
E void FDECL(savech, (CHAR_P));
#endif
E void FDECL(rhack, (char *));
E char FDECL(lowc, (CHAR_P));
E void enlightenment();
E int FDECL(xytod, (SCHAR_P,SCHAR_P));
E void FDECL(dtoxy, (coord *,int));
E int FDECL(movecmd, (CHAR_P));
E int FDECL(getdir, (BOOLEAN_P));
E void confdir();
E int FDECL(isok, (int,int));
E int doextlist();

/* ### dbridge.c ### */

E boolean FDECL(is_pool, (int,int));
#ifdef STRONGHOLD
E void FDECL(initsym, (int,int));
E int FDECL(is_drawbridge_wall, (int, int));
E boolean FDECL(is_db_wall, (int, int));
E boolean FDECL(find_drawbridge, (int *, int*));
E boolean FDECL(create_drawbridge, (int, int, int, BOOLEAN_P));
E void FDECL(open_drawbridge, (int, int));
E void FDECL(close_drawbridge, (int, int));
E void FDECL(destroy_drawbridge, (int, int));
#endif	/* STRONGHOLD /**/

/* ### decl.c ### */

/* ### demon.c ### */

E void FDECL(dsummon, (struct permonst *));
E int FDECL(demon_talk, (struct monst *));
E long FDECL(bribe, (struct monst *));
E int dprince();
E int dlord();
E int ndemon();

/* ### do.c ### */

#ifdef OVERLAY
E int FDECL(drop, (struct obj *));
E int wipeoff();
#endif
E int dodrop();
E boolean FDECL(flooreffects, (struct obj *,int,int));
E void FDECL(doaltarobj, (struct obj *));
E boolean FDECL(canletgo, (struct obj *,char *));
E void FDECL(dropx, (struct obj *));
E void FDECL(dropy, (struct obj *));
E int doddrop();
E int dodown();
E int doup();
E void FDECL(goto_level, (int,BOOLEAN_P,BOOLEAN_P));
E int donull();
E int dowipe();
E struct obj *FDECL(splitobj, (struct obj *,int));
E void FDECL(set_wounded_legs, (long,int));
E void heal_legs();

/* ### do_name.c ### */

E void FDECL(getpos, (coord *,int,char *));
E int do_mname();
E struct obj *FDECL(oname, (struct obj *,char *,int));
E int ddocall();
E void FDECL(docall, (struct obj *));
E char *FDECL(x_monnam, (struct monst *,int));
E char *FDECL(lmonnam, (struct monst *));
E char *FDECL(mon_nam, (struct monst *));
E char *FDECL(Monnam, (struct monst *));
E char *FDECL(a_monnam, (struct monst *,char *));
E char *FDECL(a2_monnam, (struct monst *,char *));
E char *FDECL(Amonnam, (struct monst *,char *));
E char *FDECL(Xmonnam, (struct monst *));
E char *FDECL(defmonnam, (struct monst *));
E char *rndmonnam();
#ifdef REINCARNATION
E char *roguename();
#endif

/* ### do_wear.c ### */

#ifdef OVERLAY
E int Armor_on();
E int Boots_on();
E int Gloves_on();
E int Helmet_on();
E int FDECL(select_off, (struct obj *otmp));
E int take_off();
#endif
E void FDECL(off_msg, (struct obj *));
E boolean FDECL(is_helmet, (struct obj *));
E boolean FDECL(is_gloves, (struct obj *));
E boolean FDECL(is_boots, (struct obj *));
E boolean FDECL(is_cloak, (struct obj *));
E boolean FDECL(is_shield, (struct obj *));
E void set_wear();
E boolean FDECL(donning, (struct obj *));
E int Armor_off();
E int Armor_gone();
E int Helmet_off();
E int Gloves_off();
E int Boots_off();
E int Cloak_off();
E int Shield_off();
E void Amulet_off();
E void FDECL(Ring_on, (struct obj *));
E void FDECL(Ring_off, (struct obj *));
E void FDECL(Ring_gone, (struct obj *));
E void FDECL(Blindf_on, (struct obj *));
E void FDECL(Blindf_off, (struct obj *));
E int dotakeoff();
E int doremring();
E int FDECL(cursed, (struct obj *));
E int FDECL(armoroff, (struct obj *));
E int dowear();
E int doputon();
E void find_ac();
E void glibr();
E struct obj *some_armor();
E void corrode_armor();
E void reset_remarm();
E int doddoremarm();
E int FDECL(destroy_arm, (struct obj *));
E void FDECL(adj_abon, (struct obj *,SCHAR_P));

/* ### dog.c ### */

E void FDECL(initedog, (struct monst *));
E void FDECL(make_familiar, (struct obj *));
E struct monst *makedog();
E void losedogs();
E void keepdogs();
E void FDECL(fall_down, (struct monst *,int));
E int FDECL(dogfood, (struct monst *,struct obj *));
E int FDECL(inroom, (XCHAR_P,XCHAR_P));
E int FDECL(tamedog, (struct monst *,struct obj *));

/* ### dogmove.c ### */

E int FDECL(dog_move, (struct monst *,int));

/* ### dokick.c ### */

E boolean FDECL(ghitm, (struct monst *,long));
E boolean FDECL(bad_kick_throw_pos, (XCHAR_P,XCHAR_P));
E struct monst *FDECL(ghit, (int,int,int));
E int dokick();

/* ### dothrow.c ### */

E int dothrow();
E int FDECL(throwit, (struct obj *));
E int FDECL(thitmonst, (struct monst *,struct obj *));
E int FDECL(breaks, (struct obj *,BOOLEAN_P));

/* ### eat.c ### */

#ifdef OVERLAY
E int Meatdone();
E int eatfood();
E int opentin();
E int unfaint();
#endif
E void init_uhunger();
E int Hear_again();
E void reset_eat();
E int doeat();
E void gethungry();
E void FDECL(morehungry, (int));
E void FDECL(lesshungry, (int));
E void FDECL(newuhs, (BOOLEAN_P));
E struct obj *FDECL(floorfood, (char *,BOOLEAN_P));
E void vomit();
E int FDECL(eaten_stat, (int, struct obj *));

/* ### end.c ### */

E int done1();
E int done2();
E void FDECL(done_in_by, (struct monst *));
E void VDECL(panic, (char *,...));
E void FDECL(done, (int));
E void clearlocks();
#ifdef NOSAVEONHANGUP
E void hangup();
#endif

/* ### engrave.c ### */

#ifdef ELBERETH
E int FDECL(sengr_at, (char *,XCHAR_P,XCHAR_P));
#endif
E void FDECL(u_wipe_engr, (int));
E void FDECL(wipe_engr_at, (XCHAR_P,XCHAR_P,XCHAR_P));
E void FDECL(read_engr_at, (int,int));
E void FDECL(make_engr_at, (int,int,char *));
E int freehand();
E int doengrave();
E void FDECL(save_engravings, (int));
E void FDECL(rest_engravings, (int));

/* ### exper.c ### */

E long FDECL(newuexp, (unsigned));
E int FDECL(experience, (struct monst *,int));
E void FDECL(more_experienced, (int,int));
E void losexp();
E void newexplevel();
E void pluslvl();
E long rndexp();

/* ### extralev.c ### */

#ifdef REINCARNATION
E void makeroguerooms();
E void FDECL(corr, (int,int));
E void makerogueghost();
#endif

/* ### fountain.c ### */

#ifdef FOUNTAINS
E void dryup();
E void drinkfountain();
E void FDECL(dipfountain, (struct obj *));
#endif /* FOUNTAINS */
#ifdef SINKS
E void drinksink();
#endif

/* ### getline.c ### */

E void FDECL(getlin, (char *));
E void getret();
E void FDECL(cgetret, (char *));
E void FDECL(xwaitforspace, (char *));
E char *parse();
E char readchar();
#ifdef COM_COMPL
E void FDECL(get_ext_cmd, (char *));
#endif /* COM_COMPL */

/* ### hack.c ### */

E void unsee();
E void FDECL(seeoff, (int));
E void FDECL(movobj, (struct obj *,XCHAR_P,XCHAR_P));
E boolean FDECL(may_dig, (XCHAR_P,XCHAR_P));
E void domove();
E void spoteffects();
E int dopickup();
E void lookaround();
E int monster_nearby();
E int FDECL(cansee, (XCHAR_P,XCHAR_P));
E int FDECL(sgn, (int));
E void FDECL(getcorners, (xchar *,xchar *,xchar *,xchar *,xchar *,xchar *,xchar *,xchar *));
E void setsee();
E void FDECL(nomul, (int));
E void FDECL(losehp, (int,const char *));
E int weight_cap();
E int inv_weight();
E int inv_cnt();
E int FDECL(identify, (struct obj *));
#ifdef STUPID_CPP	/* otherwise these functions are macros in hack.h */
E char yn();
E char ynq();
E char ynaq();
E char nyaq();
E char *FDECL(plur, (long));
E void FDECL(makeknown, (unsigned));
#endif

/* ### invent.c ### */

#ifdef OVERLAY
E int FDECL(ckunpaid, (struct obj *));
#endif
E struct obj *FDECL(addinv, (struct obj *));
E void FDECL(useup, (struct obj *));
E void FDECL(freeinv, (struct obj *));
E void FDECL(delobj, (struct obj *));
E void FDECL(freeobj, (struct obj *));
E void FDECL(freegold, (struct gold *));
E struct obj *FDECL(sobj_at, (int,int,int));
E int FDECL(carried, (struct obj *));
E struct obj *FDECL(carrying, (int));
E boolean have_lizard();
E struct obj *FDECL(o_on, (unsigned int,struct obj *));
E boolean FDECL(obj_here, (struct obj *,int,int));
E struct gold *FDECL(g_at, (int,int));
E struct obj *FDECL(getobj, (const char *,const char *));
E int FDECL(ggetobj, (char *,int(*)(),int));
E int FDECL(askchain, (struct obj *,int,char *,int,int(*)(),int(*)(),int,char *));
E void FDECL(prinv, (struct obj *));
E int ddoinv();
E void FDECL(doinv, (char *));
E int dotypeinv();
E int dolook();
E void FDECL(stackobj, (struct obj *));
E int doprgold();
E int doprwep();
E int doprarm();
E int doprring();
E int dopramulet();
E int doprtool();
E int FDECL(digit, (CHAR_P));
E void FDECL(useupf, (struct obj *));
E char *FDECL(let_to_name, (CHAR_P));
E void reassign();

/* ### ioctl.c ### */

#ifdef UNIX
E void getioctls();
E void setioctls();
# ifdef SUSPEND
E int dosuspend();
# endif /* SUSPEND */
#endif /* UNIX */

/* ### lock.c ### */

#ifdef OVERLAY
E int forcelock();
E int picklock();
#endif
E void reset_pick();
E int FDECL(pick_lock, (struct obj *));
E int doforce();
E int FDECL(boxlock, (struct obj *,struct obj *));
E int FDECL(doorlock, (struct obj *,int,int));
E int doopen();
E int doclose();

/* ### mac.c ### */
#ifdef MACOS
E int tgetch();
E void gethdate();
E int uptodate();
# ifndef THINKC4
E char *getenv();
E int memcmp();
# else
E int kbhit();
# endif
E int mcurs();
E int mputc();
E int mputs();
E int mprintf();
E int about();
#endif  /* MACOS */

/* ### macfile.c ### */
#ifdef MACOS
E short findNamedFile();
E FILE *openFile();
#endif	/* MACOS */

/* ### macinit.c ### */
#ifdef MACOS
E int initterm();
E int freeterm();
#ifdef SMALLDATA
E void init_decl();
E void free_decl();
#endif  /* SMALLDATA */
#endif	/* MACOS */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
E void getmailstatus();
# endif
E void ckmailstatus();
E void readmail();
#endif /* MAIL */

/* ### makemon.c ### */

E struct monst *FDECL(makemon, (struct permonst *,int,int));
E void FDECL(enexto, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
E int FDECL(goodpos, (int,int, struct permonst *));
E void FDECL(rloc, (struct monst *));
E void FDECL(vloc, (struct monst *));
E void init_monstr();
E struct permonst *rndmonst();
E struct permonst *FDECL(mkclass, (CHAR_P));
E int FDECL(adj_lev, (struct permonst *));
E struct permonst *FDECL(grow_up, (struct monst *));
E int FDECL(mongets, (struct monst *,int));
#ifdef REINCARNATION
E struct permonst *roguemon();
#endif
#ifdef GOLEMS
E int FDECL(golemhp, (int));
#endif /* GOLEMS */
E boolean FDECL(peace_minded, (struct permonst *));
E void FDECL(set_malign, (struct monst *));
E void FDECL(set_mimic_sym, (struct monst *));

/* ### mcastu.c ### */

E int FDECL(castmu, (struct monst *,struct attack *));
E int FDECL(buzzmu, (struct monst *,struct attack *));

/* ### mhitm.c ### */

E int FDECL(fightm, (struct monst *));
E int FDECL(mattackm, (struct monst *,struct monst *));
E int FDECL(noattacks, (struct permonst *));

/* ### mhitu.c ### */

#ifdef POLYSELF
E struct monst *cloneu();
#endif
E void FDECL(regurgitates, (struct monst *));
E int FDECL(mattacku, (struct monst *));
E void FDECL(mdamageu, (struct monst *,int));
E int FDECL(could_seduce, (struct monst *,struct monst *,struct attack *));
#ifdef SEDUCE
E int FDECL(doseduce, (struct monst *));
#endif

/* ### mklev.c ### */

E int FDECL(somex, (struct mkroom *));
E int FDECL(somey, (struct mkroom *));
#ifdef ORACLE
E boolean FDECL(place_oracle, (struct mkroom *,int *,int *,int *));
#endif
E void mklev();
E int FDECL(okdoor, (XCHAR_P,XCHAR_P));
E void FDECL(dodoor, (int,int,struct mkroom *));
E void FDECL(mktrap, (int,int,struct mkroom *));
E void FDECL(mkfount, (int,struct mkroom *));

/* ### mkmaze.c ### */

#if defined(WALLIFIED_MAZE) || defined(STRONGHOLD)
E void FDECL(wallification, (int,int,int,int,BOOLEAN_P));
#endif
E void FDECL(walkfrom, (int,int));
E void makemaz();
E void FDECL(move, (int *,int *,int));
E void FDECL(mazexy, (coord *));
E void bound_digging();

/* ### mkobj.c ### */

E struct obj *FDECL(mkobj_at, (CHAR_P,int,int));
E struct obj *FDECL(mksobj_at, (int,int,int));
E struct obj *FDECL(mkobj, (CHAR_P,BOOLEAN_P));
E int rndmonnum();
E struct obj *FDECL(mksobj, (int,BOOLEAN_P));
E int FDECL(letter, (int));
E int FDECL(weight, (struct obj *));
E void FDECL(mkgold, (long,int,int));
E struct obj *FDECL(mkcorpstat, (int,struct permonst *,int,int));
E struct obj *FDECL(mk_tt_object, (int,int,int));
E struct obj *FDECL(mk_named_object, (int,struct permonst *,int,int,char *,int));
E void FDECL(bless, (struct obj *));
E void FDECL(curse, (struct obj *));
E void FDECL(blessorcurse, (struct obj *,int));
#ifdef STUPID_CPP
E boolean FDECL(is_flammable, (struct obj *));
E boolean FDECL(is_rustprone, (struct obj *));
E boolean FDECL(is_corrodeable, (struct obj *));
E boolean FDECL(OBJ_AT, (int, int));
#endif
E void FDECL(place_object, (struct obj *,int,int));
E void FDECL(move_object, (struct obj *,int,int));
E void FDECL(remove_object, (struct obj *));

/* ### mkroom.c ### */

E void FDECL(mkroom, (int));
E void FDECL(shrine_pos, (int *, int*, struct mkroom *));
E boolean FDECL(nexttodoor, (int,int));
E boolean FDECL(has_dnstairs, (struct mkroom *));
E boolean FDECL(has_upstairs, (struct mkroom *));
E int FDECL(dist2, (int,int,int,int));
E struct permonst *courtmon();
E int FDECL(bcsign, (struct obj *));

/* ### mon.c ### */

E void movemon();
E void FDECL(meatgold, (struct monst *));
E void FDECL(meatobj, (struct monst *));
E void FDECL(mpickgold, (struct monst *));
E void FDECL(mpickgems, (struct monst *));
E int FDECL(curr_mon_load, (struct monst *));
E int FDECL(max_mon_load, (struct monst *));
E boolean FDECL(can_carry, (struct monst *,struct obj *));
E void FDECL(mpickstuff, (struct monst *,char *));
E int FDECL(mfndpos, (struct monst *,coord *,long *,long));
E int FDECL(dist, (int,int));
E boolean FDECL(monnear, (struct monst *,int,int));
E void FDECL(poisontell, (int));
E void FDECL(poisoned, (char *,int,char *));
E void FDECL(mondead, (struct monst *));
E void FDECL(replmon, (struct monst *,struct monst *));
E void FDECL(relmon, (struct monst *));
E void FDECL(monfree, (struct monst *));
E void FDECL(unstuck, (struct monst *));
E void FDECL(killed, (struct monst *));
E void FDECL(xkilled, (struct monst *,int));
E void rescham();
E void restartcham();
E int FDECL(newcham, (struct monst *,struct permonst *));
E void FDECL(mnexto, (struct monst *));
E void FDECL(mnearto, (struct monst *, XCHAR_P, XCHAR_P, BOOLEAN_P));
E void FDECL(setmangry, (struct monst *));
E int FDECL(disturb, (struct monst *));
E void FDECL(mondied, (struct monst *));
E void FDECL(mongone, (struct monst *));
E void FDECL(monstone, (struct monst *));
#ifdef GOLEMS
E void FDECL(golemeffects, (struct monst *, int, int));
#endif /* GOLEMS */

/* ### mondata.c ### */

E boolean FDECL(attacktype, (struct permonst *,int));
E boolean FDECL(resists_ston, (struct permonst *));
E boolean FDECL(resists_drli, (struct permonst *));
E boolean FDECL(ranged_attk, (struct permonst *));
E boolean FDECL(can_track, (struct permonst *));
#ifdef POLYSELF
E boolean FDECL(breakarm, (struct permonst *));
E boolean FDECL(sliparm, (struct permonst *));
#endif
E boolean FDECL(sticks, (struct permonst *));
E boolean FDECL(canseemon, (struct monst *));
E boolean FDECL(dmgtype, (struct permonst *,int));
E int FDECL(monsndx, (struct permonst *));
E int FDECL(name_to_mon, (char *));
#ifdef POLYSELF
E boolean FDECL(webmaker, (struct permonst *));
#endif
E boolean FDECL(is_female, (struct monst *));
E int FDECL(gender, (struct monst *));
E boolean FDECL(levl_follower, (struct monst *));
E struct permonst *player_mon();
E int FDECL(little_to_big, (int));
E int FDECL(big_to_little, (int));     

/* ### monmove.c ### */

E boolean FDECL(mb_trapped, (struct monst *));
E int FDECL(dochugw, (struct monst *));
E boolean FDECL(onscary, (int,int,struct monst *));
E int FDECL(dochug, (struct monst *));
E int FDECL(m_move, (struct monst *,int));
E void FDECL(set_apparxy, (struct monst *));
E boolean FDECL(mdig_tunnel, (struct monst *));
#ifdef STUPID_CPP
E boolean FDECL(MON_AT, (int, int));
E void FDECL(place_monster, (struct monst *, int, int));
E void FDECL(place_worm_seg, (struct monst *, int, int));
E void FDECL(remove_monster, (int, int));
E struct monst *FDECL(m_at, (int, int));
#endif

/* ### monst.c ### */

/* ### msdos.c ### */

#ifdef MSDOS
E void flushout();
E int tgetch();
E int dosh();
# ifdef DGK
E long FDECL(freediskspace, (char *));
E long FDECL(filesize, (char *));
E void FDECL(eraseall, (const char *,const char *));
E void FDECL(copybones, (int));
E void playwoRAMdisk();
E int FDECL(saveDiskPrompt, (int));
E void gameDiskPrompt();
# endif
E void read_config_file();
E void set_lock_and_bones();
E void FDECL(append_slash, (char *));
E void FDECL(getreturn, (const char *));
E void VDECL(msmsg, (const char *,...));
E void FDECL(chdrive, (char *));
# ifndef TOS
E void disable_ctrlP();
E void enable_ctrlP();
# endif
# ifdef DGK
E FILE *FDECL(fopenp, (char *,char *));
# endif
E void FDECL(msexit, (int));
# ifdef DGK
E void get_scr_size();
# endif
#endif /* MSDOS */
#ifdef TOS
E int FDECL(_copyfile, (char *, char *));
E int kbhit();
E void restore_colors();
E void set_colors();
#endif /* TOS */

/* ### mthrowu.c ### */

E int FDECL(thitu, (int,int,struct obj *,char *));
E int FDECL(thrwmu, (struct monst *));
E int FDECL(spitmu, (struct monst *));
E int FDECL(breamu, (struct monst *,struct attack *));
E boolean FDECL(linedup, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E boolean FDECL(lined_up, (struct monst *));
E struct obj *FDECL(m_carrying, (struct monst *,int));
E void FDECL(m_useup, (struct monst *,struct obj *));

/* ### music.c ### */

#ifdef MUSIC
E int FDECL(do_play_instrument, (struct obj *));
#endif /* MUSIC /**/

/* ### o_init.c ### */

E int FDECL(letindex, (CHAR_P));
E void init_objects();
E void oinit();
E void FDECL(savenames, (int));
E void FDECL(restnames, (int));
E int dodiscovered();

/* ### objnam.c ### */

E char *FDECL(typename, (int));
E char *FDECL(distant_name, (struct obj *, char *(*)(struct obj *)));
E char *FDECL(xname, (struct obj *));
E char *FDECL(doname, (struct obj *));
E char *FDECL(singular, (struct obj *, char *(*)(struct obj *)));
E char *FDECL(an, (char *));
E char *FDECL(An, (char *));
E char *FDECL(aobjnam, (struct obj *,char *));
E char *FDECL(Doname2, (struct obj *));
E void FDECL(lcase, (char *));
E char *FDECL(makeplural, (char *));
E struct obj *FDECL(readobjnam, (char *));

/* ### options.c ### */

E void initoptions();
E void FDECL(assign_graphics, (unsigned int *, int));
E void FDECL(parseoptions, (char *,BOOLEAN_P));
E int doset();
E int dotogglepickup();
E void option_help();
#ifdef TUTTI_FRUTTI
E int FDECL(fruitadd, (char *));
#endif

/* ### pager.c ### */

E int dowhatis();
E int dowhatdoes();
E void set_whole_screen();
#ifdef NEWS
E int readnews();
#endif /* NEWS */
E void FDECL(set_pager, (int));
E int FDECL(page_line, (char *));
E void FDECL(cornline, (int,char *));
E int dohelp();
E int dohistory();
E int FDECL(page_file, (char *,BOOLEAN_P));
#ifdef UNIX
# ifdef SHELL
E int dosh();
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
E int FDECL(child, (int));
# endif
#endif /* UNIX */

/* ### pcmain.c ### */

#if defined(MSDOS) || defined(MACOS)
E void askname();
# ifdef CHDIR
E void FDECL(chdirx, (char *,BOOLEAN_P));
# endif /* CHDIR */
#endif /* MSDOS || MACOS */

/* ### pctty.c ### */

#if defined(MSDOS) || defined(MACOS)
E void gettty();
E void FDECL(settty, (char *));
E void VDECL(error, (char *,...));
#endif /* MSDOS || MACOS  */

/* ### pcunix.c ### */

#if defined(MSDOS) || defined(MACOS)
# ifndef OLD_TOS
E void setrandom();
E int getyear();
E char *getdate();
E int phase_of_the_moon();
E int night();
E int midnight();
E void FDECL(gethdate, (char *));
E int FDECL(uptodate, (int));
# endif /* TOS */
E void FDECL(regularize, (char *));
#endif /* MSDOS */

/* ### pickup.c ### */

#ifdef OVERLAY
E int ck_bag();
E int FDECL(ck_container, (struct obj *));
E int FDECL(in_container, (struct obj *));
E int FDECL(out_container, (struct obj *));
#endif
E void FDECL(pickup, (int));
E struct obj *FDECL(pick_obj, (struct obj *));
E int doloot();
E void get_all_from_box();
E void FDECL(use_container, (struct obj *, int));
E void FDECL(inc_cwt, (struct obj *, struct obj *));
E void FDECL(delete_contents, (struct obj *));
E void FDECL(dec_cwt, (struct obj *, struct obj *));

/* ### polyself.c ### */

E void newman();
#ifdef POLYSELF
E void polyself();
E int FDECL(polymon, (int));
E void rehumanize();
E int dobreathe();
E int dospit();
E int doremove();
E int dospinweb();
E int dosummon();
E int doconfuse();
E int dohide();
#endif
E char *FDECL(body_part, (int));
E int poly_gender();
#ifdef POLYSELF
# ifdef GOLEMS
E void FDECL(ugolemeffects, (int, int));
# endif /* GOLEMS */
#endif

/* ### potion.c ### */

E void FDECL(make_confused, (long,BOOLEAN_P));
E void FDECL(make_stunned, (long,BOOLEAN_P));
E void FDECL(make_blinded, (long,BOOLEAN_P));
E void FDECL(make_sick, (long,BOOLEAN_P));
E void FDECL(make_vomiting, (long,BOOLEAN_P));
E void FDECL(make_hallucinated, (long,BOOLEAN_P));
E int dodrink();
E int FDECL(dopotion, (struct obj *));
E int FDECL(peffects, (struct obj *));
E void FDECL(healup, (int,int,BOOLEAN_P,BOOLEAN_P));
E void FDECL(strange_feeling, (struct obj *,char *));
E void FDECL(potionhit, (struct monst *,struct obj *));
E void FDECL(potionbreathe, (struct obj *));
E boolean FDECL(get_wet, (struct obj *));
E int dodip();
E void FDECL(djinni_from_bottle, (struct obj *));
E int FDECL(monster_detect, (struct obj *));
E int FDECL(object_detect, (struct obj *));
E int FDECL(trap_detect, (struct obj *));

/* ### pray.c ### */

# ifdef THEOLOGY
E int dosacrifice();
E int dopray();
E char *u_gname();
#endif /* THEOLOGY */
E int doturn();
#ifdef ALTARS
E char *a_gname();
E char *FDECL(a_gname_at, (XCHAR_P,XCHAR_P));
# ifdef THEOLOGY
E void FDECL(altar_wrath, (int,int));
# endif
#endif

/* ### pri.c ### */

E void swallowed();
E void setclipped();
#ifdef CLIPPING
E void FDECL(cliparound, (int, int));
#endif
E boolean FDECL(showmon, (struct monst *));
E void FDECL(at, (XCHAR_P,XCHAR_P,UCHAR_P,UCHAR_P));
E void prme();
E void FDECL(shieldeff, (XCHAR_P,XCHAR_P));
E int doredraw();
E void docrt();
E void FDECL(docorner, (int,int));
E void seeglds();
E void seeobjs();
E void seemons();
E void FDECL(pmon, (struct monst *));
E void FDECL(unpmon, (struct monst *));
E void nscr();
E void bot();
E void FDECL(mstatusline, (struct monst *));
E void ustatusline();
E void cls();
E void max_rank_sz();
E char rndmonsym();
E char rndobjsym();
E const char *hcolor();

/* ### priest.c ### */

E int FDECL(move_special, (struct monst *,SCHAR_P,SCHAR_P,BOOLEAN_P,BOOLEAN_P,
			XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
#if defined(ALTARS) && defined(THEOLOGY)
E struct mkroom *FDECL(in_temple, (int,int));
E int FDECL(pri_move, (struct monst *));
E void FDECL(priestini, (int,int,int,int));
E char *FDECL(priestname, (struct monst *));
E boolean FDECL(p_coaligned, (struct monst *));
E void intemple();
E void FDECL(priest_talk, (struct monst *));
E boolean FDECL(u_in_sanctuary, (struct mkroom *));
E void ghod_hitsu();
E void angry_priest();
#endif

/* ### prisym.c ### */

E void FDECL(atl, (int,int,CHAR_P));
E void FDECL(on_scr, (int,int));
E void FDECL(tmp_at, (int,int));
E void FDECL(Tmp_at2, (int,int));
E void curs_on_u();
E void pru();
E void FDECL(prl, (int,int));
E uchar FDECL(news0, (XCHAR_P,XCHAR_P));
E void FDECL(newsym, (int,int));
E void FDECL(mnewsym, (int,int));
E void FDECL(nosee, (int,int));
E void FDECL(prl1, (int,int));
E void FDECL(nose1, (int,int));
E int FDECL(vism_at, (int,int));
#ifdef NEWSCR
E void FDECL(pobj, (struct obj *));
#endif /* NEWSCR */
E void FDECL(unpobj, (struct obj *));
#ifdef STUPID_CPP	/* otherwise these functions are macros in rm.h */
E boolean FDECL(IS_WALL, (unsigned));
E boolean FDECL(IS_STWALL, (unsigned));
E boolean FDECL(IS_ROCK, (unsigned));
E boolean FDECL(IS_DOOR, (unsigned));
E boolean FDECL(IS_FLOOR, (unsigned));
E boolean FDECL(ACCESSIBLE, (unsigned));
E boolean FDECL(IS_ROOM, (unsigned));
E boolean FDECL(ZAP_POS, (unsigned));
E boolean FDECL(SPACE_POS, (unsigned));
E boolean FDECL(IS_POOL, (unsigned));
E boolean FDECL(IS_THRONE, (unsigned));
E boolean FDECL(IS_FOUNTAIN, (unsigned));
E boolean FDECL(IS_SINK, (unsigned));
E boolean FDECL(IS_ALTAR, (unsigned));
E boolean FDECL(IS_DRAWBRIDGE, (unsigned));
E boolean FDECL(IS_FURNITURE, (unsigned));
#endif /* STUPID_CPP */

/* ### read.c ### */

E int doread();
E int FDECL(seffects, (struct obj *));
E void FDECL(litroom, (BOOLEAN_P));
E void FDECL(do_genocide, (int));
E void do_mapping();
E void do_vicinity_map();
E int FDECL(gold_detect, (struct obj *));
E int FDECL(food_detect, (struct obj *));
E void FDECL(punish, (struct obj *));
E void unpunish();
E boolean FDECL(cant_create, (int *));
#if defined(WIZARD) || defined(EXPLORE_MODE)
E boolean create_particular();
#endif

/* ### restore.c ### */

E int FDECL(dorecover, (int));
E void FDECL(getlev, (int,int,XCHAR_P,BOOLEAN_P));
#ifdef ZEROCOMP
E void minit();
E int FDECL(mread, (int,genericptr_t,unsigned int));
#else
E void FDECL(mread, (int,genericptr_t,unsigned int));
#endif

/* ### rip.c ### */

E void outrip();

/* ### rnd.c ### */

E int FDECL(rn1, (int,int));
E int FDECL(rn2, (int));
E int FDECL(rnl, (int));
E int FDECL(rnd, (int));
E int FDECL(d, (int,int));
E int FDECL(rne, (int));
#ifdef THEOLOGY
E int FDECL(rnz, (int));
#endif

/* ### rumors.c ### */

E void FDECL(outrumor, (int,BOOLEAN_P));
#ifdef ORACLE
E int FDECL(doconsult, (struct monst *));
#endif

/* ### save.c ### */

E int dosave();
#ifndef NOSAVEONHANGUP
E int hangup();
#endif /* NOSAVEONHANGUP */
E int dosave0();
#if defined(DGK) && !defined(OLD_TOS)
E boolean FDECL(savelev, (int,XCHAR_P,int));
E boolean FDECL(swapin_file, (int));
#else /* DGK && !OLD_TOS */
E void FDECL(savelev, (int, XCHAR_P));
#endif /* DGK && !OLD_TOS */
#ifdef ZEROCOMP
E void FDECL(bflush, (int));
#endif
E void FDECL(bwrite, (int,genericptr_t,unsigned int));
#ifdef TUTTI_FRUTTI
E void FDECL(savefruitchn, (int));
#endif

/* ### search.c ### */

E int findit();
E int dosearch();
E int FDECL(dosearch0, (int));
E int doidtrap();
E void FDECL(wakeup, (struct monst *));
E void FDECL(seemimic, (struct monst *));

/* ### shk.c ### */

E char *FDECL(shkname, (struct monst *));
E void FDECL(shkdead, (struct monst *));
E void FDECL(replshk, (struct monst *,struct monst *));
E int inshop();
E int FDECL(inhishop, (struct monst *));
E boolean FDECL(tended_shop, (struct mkroom *));
E void FDECL(obfree, (struct obj *,struct obj *));
E int dopay();
E void FDECL(home_shk, (struct monst *));
E void FDECL(make_happy_shk, (struct monst *));
E boolean paybill();
E void FDECL(pay_for_door, (int,int,char *));
E void FDECL(addtobill, (struct obj *,BOOLEAN_P));
E void FDECL(splitbill, (struct obj *,struct obj *));
E void FDECL(sellobj, (struct obj *));
E int FDECL(doinvbill, (int));
E int FDECL(shkcatch, (struct obj *));
E int FDECL(shk_move, (struct monst *));
E int FDECL(online, (XCHAR_P,XCHAR_P));
E boolean FDECL(is_fshk, (struct monst *));
E void FDECL(shopdig, (int));
E boolean FDECL(in_shop, (int,int));
E boolean FDECL(costly_spot, (int,int));
E void FDECL(check_unpaid, (struct obj *));

/* ### shknam.c ### */

E void FDECL(stock_room, (struct shclass *,struct mkroom *));
E int FDECL(saleable, (int,struct obj *));
E int FDECL(get_shop_item, (int));

/* ### sit.c ### */

#if defined(THRONES) || defined(SPELLS)
E void take_gold();
#endif
E int dosit();
E void rndcurse();
E void attrcurse();

/* ### sounds.c ### */

E void FDECL(verbalize, (char *));
#ifdef SOUNDS
E void dosounds();
E void FDECL(growl, (struct monst *));
E void FDECL(yelp, (struct monst *));
E void FDECL(whimper, (struct monst *));
#endif
E int dotalk();

/* ### sp_lev.c ### */

#ifdef STRONGHOLD
E boolean FDECL(load_special, (char *));
#endif /* STRONGHOLD /**/

/* ### spell.c ### */
#ifdef SPELLS
#ifdef OVERLAY
E int learn();
#endif
E int FDECL(study_book, (struct obj *));
E int docast();
E int FDECL(spelleffects, (int, BOOLEAN_P));
E void losespells();
E int dovspell();
#endif /* SPELLS */

/* ### steal.c ### */

#ifdef OVERLAY
E int stealarm();
#endif
E long somegold();
E void FDECL(stealgold, (struct monst *));
E int FDECL(steal, (struct monst *));
E void FDECL(mpickobj, (struct monst *,struct obj *));
E void FDECL(stealamulet, (struct monst *));
E void FDECL(relobj, (struct monst *,int));

/* ### termcap.c ### */

E void startup();
E void start_screen();
E void end_screen();
#ifdef CLIPPING
E boolean FDECL(win_curs, (int,int));
#endif
E void FDECL(curs, (int,int));
E void FDECL(cmov, (int,int));
E void FDECL(xputc, (CHAR_P));
E void FDECL(xputs, (char *));
E void cl_end();
E void clear_screen();
E void home();
E void standoutbeg();
E void standoutend();
E void revbeg();
#if 0
E void boldbeg();
E void blinkbeg();
E void dimbeg();
#endif
E void m_end();
E void backsp();
E void bell();
E void graph_on();
E void graph_off();
E void delay_output();
E void cl_eos();

/* ### timeout.c ### */

E void timeout();
E void hatch_eggs();

/* ### topl.c ### */

E int doredotopl();
E void remember_topl();
E void FDECL(addtopl, (char *));
E void more();
E void FDECL(cmore, (char *));
E void clrlin();
#ifdef NEED_VARARGS
# if defined(USE_STDARG) || defined(USE_VARARGS)
E void FDECL(vpline, (const char *, va_list));
# endif
#endif
E void VDECL(pline, (const char *,...));
E void VDECL(Norep, (const char *,...));
E void VDECL(You, (const char *,...));
E void VDECL(Your, (const char *,...));
E void VDECL(kludge, (char *,char *,...));
E void FDECL(putsym, (CHAR_P));
E void FDECL(putstr, (const char *));
E char FDECL(yn_function, (const char *,CHAR_P));
E void VDECL(impossible, (char *,...));

/* ### topten.c ### */

E void topten();
E char *FDECL(eos, (char *));
E void FDECL(prscore, (int,char **));
E struct obj *FDECL(tt_oname, (struct obj *));

/* ### track.c ### */

E void initrack();
E void settrack();
E coord *FDECL(gettrack, (int,int));

/* ### trap.c ### */

E boolean FDECL(rust_dmg, (struct obj *,char *,int,BOOLEAN_P));
E struct trap *FDECL(maketrap, (int,int,int));
E int FDECL(teleok, (int,int));
E void FDECL(dotrap, (struct trap *));
E int FDECL(mintrap, (struct monst *));
E void FDECL(selftouch, (char *));
E void float_up();
E int float_down();
E void tele();
E void FDECL(teleds, (int,int));
E int dotele();
E void FDECL(placebc, (int));
E void unplacebc();
E void level_tele();
E void drown();
#ifdef SPELLS
E void FDECL(drain_en, (int));
#endif
E int dountrap();
E void FDECL(chest_trap, (struct obj *,int));
E void wake_nearby();
E void FDECL(deltrap, (struct trap *));
E struct trap *FDECL(t_at, (int,int));
E void b_trapped();
E boolean unconscious();

/* ### u_init.c ### */

E void u_init();
E void plnamesuffix();

/* ### uhitm.c ### */

E struct monst *FDECL(clone_mon, (struct monst *));
E boolean FDECL(special_case, (struct monst *));
E schar FDECL(find_roll_to_hit, (struct monst *));
E boolean FDECL(attack, (struct monst *));
E boolean FDECL(hmon, (struct monst *,struct obj *,int));
E int FDECL(damageum, (struct monst *, struct attack *));
E void FDECL(missum, (struct monst *, struct attack *));
E int FDECL(passive, (struct monst *,BOOLEAN_P,int,BOOLEAN_P));
E void FDECL(stumble_onto_mimic, (struct monst *));

/* ### unixmain.c ### */

#ifdef UNIX
E void FDECL(glo, (int));
E void askname();
#endif /* UNIX */

/* ### unixtty.c ### */

#ifdef UNIX
E void gettty();
E void FDECL(settty, (char *));
E void setftty();
E void intron();
E void introff();
E void VDECL(error, (char *,...));
#endif /* UNIX */

/* ### unixunix.c ### */

#ifdef UNIX
E void setrandom();
E int getyear();
E char *getdate();
E int phase_of_the_moon();
E int night();
E int midnight();
E void FDECL(gethdate, (char *));
E int FDECL(uptodate, (int));
E void getlock();
E void FDECL(regularize, (char *));
#endif /* UNIX */

/* ### vault.c ### */

E void setgd();
E void invault();
E int gd_move();
E void gddead();
E void FDECL(replgd, (struct monst *,struct monst *));
E void paygd();

/* ### version.c ### */

E int doversion();

/* ### vmsmain.c ### */

#ifdef VMS
# ifdef CHDIR
E void FDECL(chdirx, (char *,char));
# endif /* CHDIR */
E void FDECL(glo, (int));
E void askname();
#endif /* VMS */

/* ### vmsmisc.c ### */

#ifdef VMS
E void vms_abort();
E void vms_exit();
#endif /* VMS */

/* ### vmstty.c ### */

#ifdef VMS
E void gettty();
E void FDECL(settty, (char *));
E void setftty();
E void intron();
E void introff();
E void VDECL(error, (char *,...));
#endif /* VMS */

/* ### vmsunix.c ### */

#ifdef VMS
E void setrandom();
E int getyear();
E char *getdate();
E int phase_of_the_moon();
E int night();
E int midnight();
E void FDECL(gethdate, (char *));
E int FDECL(uptodate, (int));
E void getlock();
E void FDECL(regularize, (char *));
E int FDECL(vms_creat, (char *,unsigned int));
E int vms_getuid();
E void privoff();
E void privon();
# ifdef SHELL
E int dosh();
# endif
#endif /* VMS */

/* ### weapon.c ### */

E int FDECL(hitval, (struct obj *,struct permonst *));
E int FDECL(dmgval, (struct obj *,struct permonst *));
E void set_uasmon();
E struct obj *FDECL(select_rwep, (struct monst *));
E struct obj *FDECL(select_hwep, (struct monst *));
E int abon();
E int dbon();

/* ### were.c ### */

E void FDECL(were_change, (struct monst *));
E void FDECL(new_were, (struct monst *));
E boolean FDECL(were_summon, (struct permonst *,BOOLEAN_P));
#ifdef POLYSELF
E void you_were();
#endif /* POLYSELF */

/* ### wield.c ### */

E void FDECL(setuwep, (struct obj *));
E void uwepgone();
E int dowield();
E void corrode_weapon();
E int FDECL(chwepon, (struct obj *,int));
E int FDECL(welded, (struct obj *));
E void FDECL(weldmsg, (struct obj *,BOOLEAN_P));

/* ### wizard.c ### */

E void amulet();
E int FDECL(mon_has_amulet, (struct monst *));
E int FDECL(wiz_get_amulet, (struct monst *));
E void aggravate();
E void clonewiz();
#ifdef HARD
E void nasty();
E void resurrect();
E void intervene();
E void FDECL(wizdead, (struct monst *));
#endif /* HARD */
E void FDECL(cuss, (struct monst *));

/* ### worm.c ### */

#ifdef WORM
E int FDECL(getwn, (struct monst *));
E void FDECL(initworm, (struct monst *));
E void FDECL(worm_move, (struct monst *));
E void FDECL(worm_nomove, (struct monst *));
E void FDECL(wormdead, (struct monst *));
E void FDECL(wormhit, (struct monst *));
E void FDECL(wormsee, (unsigned int));
E void FDECL(cutworm, (struct monst *,XCHAR_P,XCHAR_P,unsigned));
#endif /* WORM */

/* ### worn.c ### */

E void FDECL(setworn, (struct obj *,long));
E void FDECL(setnotworn, (struct obj *));

/* ### write.c ### */

E void FDECL(dowrite, (struct obj *));

/* ### zap.c ### */

#ifdef OVERLAY
E int FDECL(bhito, (struct obj *, struct obj *));
E int FDECL(bhitm, (struct monst *, struct obj *));
#endif
E struct monst *FDECL(revive, (struct obj *,BOOLEAN_P));
E int FDECL(zappable, (struct obj *));
E void FDECL(zapnodir, (struct obj *));
E int dozap();
E int FDECL(zapyourself, (struct obj *));
E void FDECL(weffects, (struct obj *));
E char *FDECL(exclam, (int));
E void FDECL(hit, (const char *,struct monst *,const char *));
E void FDECL(miss, (const char *,struct monst *));
E struct monst *FDECL(bhit, (int,int,int,CHAR_P,int(*)(),int(*)(),struct obj *));
E struct monst *FDECL(boomhit, (int,int));
E void FDECL(buzz, (int,int,XCHAR_P,XCHAR_P,int,int));
E void FDECL(rlocgold, (struct gold *));
E void FDECL(rloco, (struct obj *));
E void FDECL(fracture_rock, (struct obj *));
E boolean FDECL(break_statue, (struct obj *));
E void FDECL(destroy_item, (int,int));
E int FDECL(destroy_mitem, (struct monst *,int,int));
E int FDECL(resist, (struct monst *,CHAR_P,int,int));
E void makewish();

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#undef E

#endif /* EXTERN_H /**/
