/*	SCCS Id: @(#)extern.h	3.0	89/11/22
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

E int NDECL((*occupation));
E int NDECL((*afternmv));
E void NDECL(moveloop);
E void NDECL(stop_occupation);
E void NDECL(newgame);

/* ### alloc.c ### */

#ifndef __TURBOC__
E long *FDECL(alloc, (unsigned int));
#endif

#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### amiwbench.c ### */

#ifdef AMIGA_WBENCH
E void FDECL(ami_wbench_init,(int,char *[]));	
E void NDECL(ami_wbench_args);
E int FDECL(ami_wbench_getsave,(int));
E void FDECL(ami_wbench_unlink,(char *));
E int FDECL(ami_wbench_iconsize,(char *));
E void FDECL(ami_wbench_iconwrite,(char *));
E int FDECL(ami_wbench_badopt,(char *));
E void NDECL(ami_wbench_cleanup);
#endif /* AMIGA_WBENCH */

/* ### apply.c ### */

#ifdef OVERLAY
E int NDECL(dig);
#endif
E int NDECL(doapply);
E int NDECL(holetime);
E void NDECL(dighole);
E int NDECL(dorub);
E int NDECL(dojump);
#ifdef WALKIES
E int NDECL(number_leashed);
E void FDECL(o_unleash, (struct obj *));
E void FDECL(m_unleash, (struct monst *));
E void NDECL(unleash_all);
E boolean NDECL(next_to_u);
E struct obj *FDECL(get_mleash, (struct monst *));
E void FDECL(check_leash, (XCHAR_P,XCHAR_P));
#endif
E boolean FDECL(um_dist, (XCHAR_P,XCHAR_P,XCHAR_P));
E int FDECL(use_unicorn_horn, (struct obj *));

/* ### artifact.c ### */

#ifdef NAMED_ITEMS
E void NDECL(init_exists);
E void FDECL(mkartifact, (struct obj **));
E boolean FDECL(is_artifact, (struct obj *));
E boolean FDECL(exist_artifact, (struct obj *,const char *));
E void FDECL(artifact_exists, (struct obj *,const char *,BOOLEAN_P));
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
E void NDECL(restore_attrib);
E void FDECL(init_attr, (int));
E void NDECL(redist_attr);
E void FDECL(adjabil, (int,int));
E int NDECL(newhp);
E schar FDECL(acurr, (int));
E void FDECL(adjalign, (int));

/* ### bones.c ### */

E void NDECL(savebones);
E int NDECL(getbones);
E void FDECL(name_file, (char *,int));

/* ### cmd.c ### */

#ifdef OVERLAY
E int NDECL(doextcmd);
#ifdef POLYSELF
E int NDECL(domonability);
#endif /* POLYSELF */
E int NDECL(timed_occupation);
#if defined(WIZARD) || defined(EXPLORE_MODE)
E int NDECL(wiz_attributes);
#endif /* WIZARD */
# ifdef EXPLORE_MODE
E int NDECL(enter_explore_mode);
# endif	/* EXPLORE_MODE */
#ifdef WIZARD
E int NDECL(wiz_detect);
E int NDECL(wiz_genesis);
E int NDECL(wiz_identify);
E int NDECL(wiz_level_tele);
E int NDECL(wiz_map);
E int NDECL(wiz_where);
E int NDECL(wiz_wish);
#endif /* WIZARD */
#endif /* OVERLAY */
E void NDECL(reset_occupations);
E void FDECL(set_occupation, (int NDECL((*)),const char *,int));
#ifdef REDO
E char NDECL(pgetchar);
E void FDECL(pushch, (CHAR_P));
E void FDECL(savech, (CHAR_P));
#endif
E void FDECL(rhack, (char *));
E char FDECL(lowc, (CHAR_P));
E void NDECL(enlightenment);
E int FDECL(xytod, (SCHAR_P,SCHAR_P));
E void FDECL(dtoxy, (coord *,int));
E int FDECL(movecmd, (CHAR_P));
E int FDECL(getdir, (BOOLEAN_P));
E void NDECL(confdir);
E int FDECL(isok, (int,int));
E int NDECL(doextlist);

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
E int NDECL(dprince);
E int NDECL(dlord);
E int NDECL(ndemon);

/* ### do.c ### */

#ifdef OVERLAY
E int FDECL(drop, (struct obj *));
E int NDECL(wipeoff);
#endif
E int NDECL(dodrop);
E boolean FDECL(flooreffects, (struct obj *,int,int));
E void FDECL(doaltarobj, (struct obj *));
E boolean FDECL(canletgo, (struct obj *,const char *));
E void FDECL(dropx, (struct obj *));
E void FDECL(dropy, (struct obj *));
E int NDECL(doddrop);
E int NDECL(dodown);
E int NDECL(doup);
E void FDECL(goto_level, (int,BOOLEAN_P,BOOLEAN_P));
E int NDECL(donull);
E int NDECL(dowipe);
E struct obj *FDECL(splitobj, (struct obj *,int));
E void FDECL(set_wounded_legs, (long,int));
E void NDECL(heal_legs);

/* ### do_name.c ### */

E void FDECL(getpos, (coord *,int,const char *));
E struct monst *FDECL(christen_monst, (struct monst *,const char *));
E int NDECL(do_mname);
E struct obj *FDECL(oname, (struct obj *,const char *,int));
E int NDECL(ddocall);
E void FDECL(docall, (struct obj *));
E char *FDECL(x_monnam, (struct monst *,int));
E char *FDECL(lmonnam, (struct monst *));
E char *FDECL(mon_nam, (struct monst *));
E char *FDECL(Monnam, (struct monst *));
E char *FDECL(a_monnam, (struct monst *,const char *));
E char *FDECL(a2_monnam, (struct monst *,const char *));
E char *FDECL(Amonnam, (struct monst *,const char *));
E char *FDECL(Xmonnam, (struct monst *));
E char *FDECL(defmonnam, (struct monst *));
E const char *NDECL(rndmonnam);
E char *FDECL(self_pronoun, (const char *,const char *));
#ifdef REINCARNATION
E const char *NDECL(roguename);
#endif

/* ### do_wear.c ### */

#ifdef OVERLAY
E int NDECL(Armor_on);
E int NDECL(Boots_on);
E int NDECL(Gloves_on);
E int NDECL(Helmet_on);
E int FDECL(select_off, (struct obj *otmp));
E int NDECL(take_off);
#endif
E void FDECL(off_msg, (struct obj *));
E boolean FDECL(is_helmet, (struct obj *));
E boolean FDECL(is_gloves, (struct obj *));
E boolean FDECL(is_boots, (struct obj *));
E boolean FDECL(is_cloak, (struct obj *));
E boolean FDECL(is_shield, (struct obj *));
E void NDECL(set_wear);
E boolean FDECL(donning, (struct obj *));
E void NDECL(cancel_don);
E int NDECL(Armor_off);
E int NDECL(Armor_gone);
E int NDECL(Helmet_off);
E int NDECL(Gloves_off);
E int NDECL(Boots_off);
E int NDECL(Cloak_off);
E int NDECL(Shield_off);
E void NDECL(Amulet_off);
E void FDECL(Ring_on, (struct obj *));
E void FDECL(Ring_off, (struct obj *));
E void FDECL(Ring_gone, (struct obj *));
E void FDECL(Blindf_on, (struct obj *));
E void FDECL(Blindf_off, (struct obj *));
E int NDECL(dotakeoff);
E int NDECL(doremring);
E int FDECL(cursed, (struct obj *));
E int FDECL(armoroff, (struct obj *));
E int NDECL(dowear);
E int NDECL(doputon);
E void NDECL(find_ac);
E void NDECL(glibr);
E struct obj *NDECL(some_armor);
E void NDECL(corrode_armor);
E void NDECL(reset_remarm);
E int NDECL(doddoremarm);
E int FDECL(destroy_arm, (struct obj *));
E void FDECL(adj_abon, (struct obj *,SCHAR_P));

/* ### dog.c ### */

E void FDECL(initedog, (struct monst *));
E void FDECL(make_familiar, (struct obj *));
E struct monst *NDECL(makedog);
E void NDECL(losedogs);
E void NDECL(keepdogs);
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
E int NDECL(dokick);

/* ### dothrow.c ### */

E int NDECL(dothrow);
E int FDECL(throwit, (struct obj *));
E int FDECL(thitmonst, (struct monst *,struct obj *));
E int FDECL(breaks, (struct obj *,BOOLEAN_P));

/* ### eat.c ### */

#ifdef OVERLAY
E int NDECL(Meatdone);
E int NDECL(eatfood);
E int NDECL(opentin);
E int NDECL(unfaint);
#endif
E boolean FDECL(is_edible, (struct obj *));
E void NDECL(init_uhunger);
E int NDECL(Hear_again);
E void NDECL(reset_eat);
E int NDECL(doeat);
E void NDECL(gethungry);
E void FDECL(morehungry, (int));
E void FDECL(lesshungry, (int));
E boolean NDECL(is_fainted);
E void NDECL(reset_faint);
E void NDECL(sync_hunger);
E void FDECL(newuhs, (BOOLEAN_P));
E struct obj *FDECL(floorfood, (const char *,BOOLEAN_P));
E void NDECL(vomit);
E int FDECL(eaten_stat, (int, struct obj *));
E void FDECL(food_disappears, (struct obj *));

/* ### end.c ### */

E int NDECL(done1);
E int NDECL(done2);
E void FDECL(done_in_by, (struct monst *));
E void VDECL(panic, (const char *,...));
E void FDECL(done, (int));
E void NDECL(clearlocks);
#ifdef NOSAVEONHANGUP
E void NDECL(hangup);
#endif

/* ### engrave.c ### */

#ifdef ELBERETH
E int FDECL(sengr_at, (const char *,XCHAR_P,XCHAR_P));
#endif
E void FDECL(u_wipe_engr, (int));
E void FDECL(wipe_engr_at, (XCHAR_P,XCHAR_P,XCHAR_P));
E void FDECL(read_engr_at, (int,int));
E void FDECL(make_engr_at, (int,int,const char *));
E int NDECL(freehand);
E int NDECL(doengrave);
E void FDECL(save_engravings, (int));
E void FDECL(rest_engravings, (int));

/* ### exper.c ### */

E long FDECL(newuexp, (unsigned));
E int FDECL(experience, (struct monst *,int));
E void FDECL(more_experienced, (int,int));
E void NDECL(losexp);
E void NDECL(newexplevel);
E void NDECL(pluslvl);
E long NDECL(rndexp);

/* ### extralev.c ### */

#ifdef REINCARNATION
E void NDECL(makeroguerooms);
E void FDECL(corr, (int,int));
E void NDECL(makerogueghost);
#endif

/* ### fountain.c ### */

#ifdef FOUNTAINS
E void NDECL(dryup);
E void NDECL(drinkfountain);
E void FDECL(dipfountain, (struct obj *));
#endif /* FOUNTAINS */
#ifdef SINKS
E void NDECL(drinksink);
#endif

/* ### getline.c ### */

E void FDECL(getlin, (char *));
E void NDECL(getret);
E void FDECL(cgetret, (const char *));
E void FDECL(xwaitforspace, (const char *));
E char *NDECL(parse);
E char NDECL(readchar);
#ifdef COM_COMPL
E void FDECL(get_ext_cmd, (char *));
#endif /* COM_COMPL */

/* ### hack.c ### */

E void NDECL(unsee);
E void FDECL(seeoff, (int));
E void FDECL(movobj, (struct obj *,XCHAR_P,XCHAR_P));
E boolean FDECL(may_dig, (XCHAR_P,XCHAR_P));
E void NDECL(domove);
E void NDECL(spoteffects);
E int NDECL(dopickup);
E void NDECL(lookaround);
E int NDECL(monster_nearby);
E int FDECL(cansee, (XCHAR_P,XCHAR_P));
E int FDECL(sgn, (int));
E void FDECL(getcorners, (xchar *,xchar *,xchar *,xchar *,xchar *,xchar *,xchar *,xchar *));
E void NDECL(setsee);
E void FDECL(nomul, (int));
E void FDECL(losehp, (int,const char *,BOOLEAN_P));
E int NDECL(weight_cap);
E int NDECL(inv_weight);
E int NDECL(inv_cnt);
E int FDECL(identify, (struct obj *));
#ifdef STUPID_CPP	/* otherwise these functions are macros in hack.h */
E char NDECL(yn);
E char NDECL(ynq);
E char NDECL(ynaq);
E char NDECL(nyaq);
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
E boolean NDECL(have_lizard);
E struct obj *FDECL(o_on, (unsigned int,struct obj *));
E boolean FDECL(obj_here, (struct obj *,int,int));
E struct gold *FDECL(g_at, (int,int));
E struct obj *FDECL(getobj, (const char *,const char *));
E int FDECL(ggetobj, (const char *,int FDECL((*),(struct obj *)),int));
E int FDECL(askchain, (struct obj *,int,const char *,int,int FDECL((*),(struct obj *)),int FDECL((*),(struct obj *)),int,const char *));
E void FDECL(prinv, (struct obj *));
E int NDECL(ddoinv);
E void FDECL(doinv, (const char *));
E int NDECL(dotypeinv);
E int NDECL(dolook);
E void FDECL(stackobj, (struct obj *));
E int NDECL(doprgold);
E int NDECL(doprwep);
E int NDECL(doprarm);
E int NDECL(doprring);
E int NDECL(dopramulet);
E int NDECL(doprtool);
E int FDECL(digit, (CHAR_P));
E void FDECL(useupf, (struct obj *));
E char *FDECL(let_to_name, (CHAR_P));
E void NDECL(reassign);

/* ### ioctl.c ### */

#ifdef UNIX
E void NDECL(getioctls);
E void NDECL(setioctls);
# ifdef SUSPEND
E int NDECL(dosuspend);
# endif /* SUSPEND */
#endif /* UNIX */

/* ### lock.c ### */

#ifdef OVERLAY
E int NDECL(forcelock);
E int NDECL(picklock);
#endif
E void NDECL(reset_pick);
E int FDECL(pick_lock, (struct obj *));
E int NDECL(doforce);
E int FDECL(boxlock, (struct obj *,struct obj *));
E int FDECL(doorlock, (struct obj *,int,int));
E int NDECL(doopen);
E int NDECL(doclose);

/* ### mac.c ### */
/* All of these Mac-specific declarations LOOK like they should take the
   NDECL() form, but don't be fooled: it's just that (at the time of
   writing) the Mac C compilers are all pre-ANSI and the prototype
   information isn't recorded here anyway. sps 89dec14
*/
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
E DialogTHndl centreDlgBox();
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
E int read_config_file();
E int write_opts();
#endif
#ifdef SMALLDATA
E void init_decl();
E void free_decl();
#endif  /* SMALLDATA */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
E void NDECL(getmailstatus);
# endif
E void NDECL(ckmailstatus);
E void NDECL(readmail);
#endif /* MAIL */

/* ### makemon.c ### */

E struct monst *FDECL(makemon, (struct permonst *,int,int));
E void FDECL(enexto, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
E int FDECL(goodpos, (int,int, struct permonst *));
E void FDECL(rloc, (struct monst *));
E void FDECL(vloc, (struct monst *));
E void NDECL(init_monstr);
E struct permonst *NDECL(rndmonst);
E struct permonst *FDECL(mkclass, (CHAR_P));
E int FDECL(adj_lev, (struct permonst *));
E struct permonst *FDECL(grow_up, (struct monst *));
E int FDECL(mongets, (struct monst *,int));
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
E struct monst *NDECL(cloneu);
#endif
E void FDECL(expels, (struct monst *,struct permonst *,BOOLEAN_P));
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
E void NDECL(mklev);
E int FDECL(okdoor, (XCHAR_P,XCHAR_P));
E void FDECL(dodoor, (int,int,struct mkroom *));
E void FDECL(mktrap, (int,int,struct mkroom *));
E void FDECL(mkfount, (int,struct mkroom *));

/* ### mkmaze.c ### */

#if defined(WALLIFIED_MAZE) || defined(STRONGHOLD)
E void FDECL(wallification, (int,int,int,int,BOOLEAN_P));
#endif
E void FDECL(walkfrom, (int,int));
E void NDECL(makemaz);
E void FDECL(move, (int *,int *,int));
E void FDECL(mazexy, (coord *));
E void NDECL(bound_digging);

/* ### mkobj.c ### */

E struct obj *FDECL(mkobj_at, (CHAR_P,int,int));
E struct obj *FDECL(mksobj_at, (int,int,int));
E struct obj *FDECL(mkobj, (CHAR_P,BOOLEAN_P));
E int NDECL(rndmonnum);
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
E boolean FDECL(is_flammable, (struct obj *));
#ifdef STUPID_CPP
E boolean FDECL(is_rustprone, (struct obj *));
E boolean FDECL(is_corrodeable, (struct obj *));
E boolean FDECL(OBJ_AT, (int, int));
#endif
E void FDECL(place_object, (struct obj *,int,int));
E void FDECL(move_object, (struct obj *,int,int));
E void FDECL(remove_object, (struct obj *));
E int FDECL(bcsign, (struct obj *));

/* ### mkroom.c ### */

E void FDECL(mkroom, (int));
E void FDECL(shrine_pos, (int *, int*, struct mkroom *));
E boolean FDECL(nexttodoor, (int,int));
E boolean FDECL(has_dnstairs, (struct mkroom *));
E boolean FDECL(has_upstairs, (struct mkroom *));
E int FDECL(dist2, (int,int,int,int));
E struct permonst *NDECL(courtmon);

/* ### mon.c ### */

E void NDECL(movemon);
E void FDECL(meatgold, (struct monst *));
E void FDECL(meatobj, (struct monst *));
E void FDECL(mpickgold, (struct monst *));
E void FDECL(mpickgems, (struct monst *));
E int FDECL(curr_mon_load, (struct monst *));
E int FDECL(max_mon_load, (struct monst *));
E boolean FDECL(can_carry, (struct monst *,struct obj *));
E void FDECL(mpickstuff, (struct monst *,const char *));
E int FDECL(mfndpos, (struct monst *,coord *,long *,long));
E int FDECL(dist, (int,int));
E boolean FDECL(monnear, (struct monst *,int,int));
E void FDECL(poisontell, (int));
E void FDECL(poisoned, (const char *,int,const char *,int));
E void FDECL(mondead, (struct monst *));
E void FDECL(replmon, (struct monst *,struct monst *));
E void FDECL(relmon, (struct monst *));
E void FDECL(monfree, (struct monst *));
E void FDECL(unstuck, (struct monst *));
E void FDECL(killed, (struct monst *));
E void FDECL(xkilled, (struct monst *,int));
E void NDECL(rescham);
E void NDECL(restartcham);
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
E struct permonst *NDECL(player_mon);
E int FDECL(little_to_big, (int));
E int FDECL(big_to_little, (int));     
E const char *FDECL(locomotion, (const struct permonst *,const char *));

/* ### monmove.c ### */

E boolean FDECL(mb_trapped, (struct monst *));
E int FDECL(dochugw, (struct monst *));
E boolean FDECL(onscary, (int,int,struct monst *));
E int FDECL(dochug, (struct monst *));
E int FDECL(m_move, (struct monst *,int));
E boolean FDECL(closed_door, (int, int));
E boolean FDECL(accessible, (int, int));
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
E void NDECL(flushout);
E int NDECL(tgetch);
E int NDECL(dosh);
# ifdef DGK
E long FDECL(freediskspace, (char *));
E long FDECL(filesize, (char *));
E void FDECL(eraseall, (const char *,const char *));
E void FDECL(copybones, (int));
E void NDECL(playwoRAMdisk);
E int FDECL(saveDiskPrompt, (int));
E void NDECL(gameDiskPrompt);
# endif
E void NDECL(read_config_file);
E void NDECL(set_lock_and_bones);
E void FDECL(append_slash, (char *));
E void FDECL(getreturn, (const char *));
E void VDECL(msmsg, (const char *,...));
E void FDECL(chdrive, (char *));
# ifndef TOS
E void NDECL(disable_ctrlP);
E void NDECL(enable_ctrlP);
# endif
# ifdef DGK
E FILE *FDECL(fopenp, (const char *,const char *));
# endif
E void FDECL(msexit, (int));
# ifdef DGK
E void NDECL(get_scr_size);
#  ifndef TOS
E void FDECL(gotoxy, (int,int));
#  endif
# endif
#endif /* MSDOS */
#ifdef TOS
E int FDECL(_copyfile, (char *, char *));
E int NDECL(kbhit);
E void NDECL(restore_colors);
E void NDECL(set_colors);
#endif /* TOS */

/* ### mthrowu.c ### */

E int FDECL(thitu, (int,int,struct obj *,const char *));
E int FDECL(thrwmu, (struct monst *));
E int FDECL(spitmu, (struct monst *,struct attack *));
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
E void NDECL(init_objects);
E void NDECL(oinit);
E void FDECL(savenames, (int));
E void FDECL(restnames, (int));
E int NDECL(dodiscovered);

/* ### objnam.c ### */

E char *FDECL(typename, (int));
E char *FDECL(distant_name, (struct obj *, char *(*)(struct obj *)));
E char *FDECL(xname, (struct obj *));
E char *FDECL(doname, (struct obj *));
E char *FDECL(singular, (struct obj *, char *(*)(struct obj *)));
E char *FDECL(an, (const char *));
E char *FDECL(An, (const char *));
E char *FDECL(aobjnam, (struct obj *,const char *));
E char *FDECL(Doname2, (struct obj *));
E void FDECL(lcase, (char *));
E char *FDECL(makeplural, (const char *));
E char *FDECL(makesingular, (const char *));
E struct obj *FDECL(readobjnam, (char *));

/* ### options.c ### */

E void NDECL(initoptions);
E void FDECL(assign_graphics, (unsigned int *, int));
E void FDECL(parseoptions, (char *,BOOLEAN_P));
E int NDECL(doset);
E int NDECL(dotogglepickup);
E void NDECL(option_help);
E int FDECL(next_opt, (const char *));
#ifdef TUTTI_FRUTTI
E int FDECL(fruitadd, (char *));
#endif

/* ### pager.c ### */

E int NDECL(dowhatis);
E int NDECL(dowhatdoes);
E void NDECL(set_whole_screen);
#ifdef NEWS
E int NDECL(readnews);
#endif /* NEWS */
E void FDECL(set_pager, (int));
E int FDECL(page_line, (const char *));
E void FDECL(cornline, (int,const char *));
E int NDECL(dohelp);
E int NDECL(dohistory);
E int FDECL(page_file, (const char *,BOOLEAN_P));
#ifdef UNIX
# ifdef SHELL
E int NDECL(dosh);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
E int FDECL(child, (int));
# endif
#endif /* UNIX */

/* ### pcmain.c ### */

#if defined(MSDOS) || defined(MACOS)
E void NDECL(askname);
# ifdef CHDIR
E void FDECL(chdirx, (char *,BOOLEAN_P));
# endif /* CHDIR */
#endif /* MSDOS || MACOS */

/* ### pctty.c ### */

#if defined(MSDOS) || defined(MACOS)
E void NDECL(gettty);
E void FDECL(settty, (const char *));
E void VDECL(error, (const char *,...));
#endif /* MSDOS || MACOS  */

/* ### pcunix.c ### */

#if defined(MSDOS) || defined(MACOS)
# ifndef OLD_TOS
E void NDECL(setrandom);
E int NDECL(getyear);
E char *NDECL(getdate);
E int NDECL(phase_of_the_moon);
E int NDECL(night);
E int NDECL(midnight);
E void FDECL(gethdate, (char *));
E int FDECL(uptodate, (int));
# endif /* TOS */
E void FDECL(regularize, (char *));
#endif /* MSDOS */

/* ### pickup.c ### */

#ifdef OVERLAY
E int FDECL(ck_bag, (struct obj *));
E int FDECL(ck_container, (struct obj *));
E int FDECL(in_container, (struct obj *));
E int FDECL(out_container, (struct obj *));
#endif
E void FDECL(pickup, (int));
E struct obj *FDECL(pick_obj, (struct obj *));
E int NDECL(doloot);
E void NDECL(get_all_from_box);
E void FDECL(use_container, (struct obj *, int));
E void FDECL(inc_cwt, (struct obj *, struct obj *));
E void FDECL(delete_contents, (struct obj *));
E void FDECL(dec_cwt, (struct obj *, struct obj *));

/* ### polyself.c ### */

E void NDECL(newman);
#ifdef POLYSELF
E void NDECL(polyself);
E int FDECL(polymon, (int));
E void NDECL(rehumanize);
E int NDECL(dobreathe);
E int NDECL(dospit);
E int NDECL(doremove);
E int NDECL(dospinweb);
E int NDECL(dosummon);
E int NDECL(doconfuse);
E int NDECL(dohide);
#endif
E const char *FDECL(body_part, (int));
E int NDECL(poly_gender);
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
E int NDECL(dodrink);
E int FDECL(dopotion, (struct obj *));
E int FDECL(peffects, (struct obj *));
E void FDECL(healup, (int,int,BOOLEAN_P,BOOLEAN_P));
E void FDECL(strange_feeling, (struct obj *,const char *));
E void FDECL(potionhit, (struct monst *,struct obj *));
E void FDECL(potionbreathe, (struct obj *));
E boolean FDECL(get_wet, (struct obj *));
E int NDECL(dodip);
E void FDECL(djinni_from_bottle, (struct obj *));
E int FDECL(monster_detect, (struct obj *));
E int FDECL(object_detect, (struct obj *));
E int FDECL(trap_detect, (struct obj *));

/* ### pray.c ### */

# ifdef THEOLOGY
E int NDECL(dosacrifice);
E int NDECL(dopray);
E const char *NDECL(u_gname);
#endif /* THEOLOGY */
E int NDECL(doturn);
#ifdef ALTARS
E const char *NDECL(a_gname);
E const char *FDECL(a_gname_at, (XCHAR_P,XCHAR_P));
# ifdef THEOLOGY
E void FDECL(altar_wrath, (int,int));
# endif
#endif

/* ### pri.c ### */

E char *FDECL(eos, (char *));
E void FDECL(swallowed, (int));
E void NDECL(setclipped);
#ifdef CLIPPING
E void FDECL(cliparound, (int, int));
#endif
E boolean FDECL(showmon, (struct monst *));
E void FDECL(at, (XCHAR_P,XCHAR_P,UCHAR_P,UCHAR_P));
E void NDECL(prme);
E void FDECL(shieldeff, (XCHAR_P,XCHAR_P));
E int NDECL(doredraw);
E void NDECL(docrt);
E void FDECL(docorner, (int,int));
E void NDECL(seeglds);
E void NDECL(seeobjs);
E void NDECL(seemons);
E void FDECL(pmon, (struct monst *));
E void FDECL(unpmon, (struct monst *));
E void NDECL(nscr);
E void NDECL(bot);
E void FDECL(mstatusline, (struct monst *));
E void NDECL(ustatusline);
E void NDECL(cls);
E void NDECL(max_rank_sz);
E char NDECL(rndmonsym);
E char NDECL(rndobjsym);
E const char *NDECL(hcolor);
E uchar FDECL(mimic_appearance, (struct monst *));

/* ### priest.c ### */

E int FDECL(move_special, (struct monst *,SCHAR_P,SCHAR_P,BOOLEAN_P,BOOLEAN_P,
			XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
#if defined(ALTARS) && defined(THEOLOGY)
E struct mkroom *FDECL(in_temple, (int,int));
E int FDECL(pri_move, (struct monst *));
E void FDECL(priestini, (int,int,int,int));
E char *FDECL(priestname, (struct monst *));
E boolean FDECL(p_coaligned, (struct monst *));
E void NDECL(intemple);
E void FDECL(priest_talk, (struct monst *));
E boolean FDECL(u_in_sanctuary, (struct mkroom *));
E void NDECL(ghod_hitsu);
E void NDECL(angry_priest);
#endif

/* ### prisym.c ### */

E void FDECL(atl, (int,int,CHAR_P));
E void FDECL(on_scr, (int,int));
E void FDECL(tmp_at, (int,int));
E void FDECL(Tmp_at2, (int,int));
E void NDECL(curs_on_u);
E void NDECL(pru);
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

E int NDECL(doread);
E int FDECL(seffects, (struct obj *));
E void FDECL(litroom, (BOOLEAN_P));
E void FDECL(do_genocide, (int));
E void NDECL(do_mapping);
E void NDECL(do_vicinity_map);
E int FDECL(gold_detect, (struct obj *));
E int FDECL(food_detect, (struct obj *));
E void FDECL(punish, (struct obj *));
E void NDECL(unpunish);
E boolean FDECL(cant_create, (int *));
#if defined(WIZARD) || defined(EXPLORE_MODE)
E boolean NDECL(create_particular);
#endif

/* ### restore.c ### */

E int FDECL(dorecover, (int));
E void FDECL(getlev, (int,int,XCHAR_P,BOOLEAN_P));
#ifdef ZEROCOMP
E void NDECL(minit);
E int FDECL(mread, (int,genericptr_t,unsigned int));
#else
E void FDECL(mread, (int,genericptr_t,unsigned int));
#endif

/* ### rip.c ### */

E void NDECL(outrip);

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

E int NDECL(dosave);
#ifndef NOSAVEONHANGUP
E int NDECL(hangup);
#endif /* NOSAVEONHANGUP */
E int NDECL(dosave0);
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

E int NDECL(findit);
E int NDECL(dosearch);
E int FDECL(dosearch0, (int));
E int NDECL(doidtrap);
E void FDECL(wakeup, (struct monst *));
E void FDECL(seemimic, (struct monst *));

/* ### shk.c ### */

E char *FDECL(shkname, (struct monst *));
E void FDECL(shkdead, (struct monst *));
E void FDECL(replshk, (struct monst *,struct monst *));
E int NDECL(inshop);
E int FDECL(inhishop, (struct monst *));
E boolean FDECL(tended_shop, (struct mkroom *));
E void FDECL(obfree, (struct obj *,struct obj *));
E int NDECL(dopay);
E void FDECL(home_shk, (struct monst *));
E void FDECL(make_happy_shk, (struct monst *));
E boolean NDECL(paybill);
E void FDECL(pay_for_door, (int,int,const char *));
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
E boolean FDECL(block_door, (int,int));
E boolean FDECL(block_entry, (int,int));

/* ### shknam.c ### */

E void FDECL(stock_room, (const struct shclass *,struct mkroom *));
E int FDECL(saleable, (int,struct obj *));
E int FDECL(get_shop_item, (int));

/* ### sit.c ### */

#if defined(THRONES) || defined(SPELLS)
E void NDECL(take_gold);
#endif
E int NDECL(dosit);
E void NDECL(rndcurse);
E void NDECL(attrcurse);

/* ### sounds.c ### */

E void FDECL(verbalize, (const char *));
#ifdef SOUNDS
E void NDECL(dosounds);
E void FDECL(growl, (struct monst *));
E void FDECL(yelp, (struct monst *));
E void FDECL(whimper, (struct monst *));
#endif
E struct monst *FDECL(qname, (struct monst *));
E int NDECL(dotalk);

/* ### sp_lev.c ### */

#ifdef STRONGHOLD
E boolean FDECL(load_special, (const char *));
#endif /* STRONGHOLD /**/

/* ### spell.c ### */
#ifdef SPELLS
#ifdef OVERLAY
E int NDECL(learn);
#endif
E int FDECL(study_book, (struct obj *));
E int NDECL(docast);
E int FDECL(spelleffects, (int, BOOLEAN_P));
E void NDECL(losespells);
E int NDECL(dovspell);
#endif /* SPELLS */

/* ### steal.c ### */

#ifdef OVERLAY
E int NDECL(stealarm);
#endif
E long NDECL(somegold);
E void FDECL(stealgold, (struct monst *));
E int FDECL(steal, (struct monst *));
E void FDECL(mpickobj, (struct monst *,struct obj *));
E void FDECL(stealamulet, (struct monst *));
E void FDECL(relobj, (struct monst *,int));

/* ### termcap.c ### */

E void NDECL(startup);
E void NDECL(start_screen);
E void NDECL(end_screen);
#ifdef CLIPPING
E boolean FDECL(win_curs, (int,int));
#endif
E void FDECL(curs, (int,int));
E void FDECL(cmov, (int,int));
E void FDECL(xputc, (CHAR_P));
E void FDECL(xputs, (const char *));
E void NDECL(cl_end);
E void NDECL(clear_screen);
E void NDECL(home);
E void NDECL(standoutbeg);
E void NDECL(standoutend);
E void NDECL(revbeg);
#if 0
E void NDECL(boldbeg);
E void NDECL(blinkbeg);
E void NDECL(dimbeg);
#endif
E void NDECL(m_end);
E void NDECL(backsp);
E void NDECL(bell);
E void NDECL(graph_on);
E void NDECL(graph_off);
E void NDECL(delay_output);
E void NDECL(cl_eos);

/* ### timeout.c ### */

E void NDECL(timeout);
E void NDECL(hatch_eggs);

/* ### topl.c ### */

E int NDECL(doredotopl);
E void NDECL(remember_topl);
E void FDECL(addtopl, (const char *));
E void NDECL(more);
E void FDECL(cmore, (const char *));
E void NDECL(clrlin);
#ifdef NEED_VARARGS
# if defined(USE_STDARG) || defined(USE_VARARGS)
E void FDECL(vpline, (const char *, va_list));
# endif
#endif
E void VDECL(pline, (const char *,...));
E void VDECL(Norep, (const char *,...));
E void VDECL(You, (const char *,...));
E void VDECL(Your, (const char *,...));
E void VDECL(kludge, (const char *,const char *,...));
E void FDECL(putsym, (CHAR_P));
E void FDECL(putstr, (const char *));
E char FDECL(yn_function, (const char *,CHAR_P));
E void VDECL(impossible, (const char *,...));

/* ### topten.c ### */

E void FDECL(topten, (int));
E void FDECL(prscore, (int,char **));
E struct obj *FDECL(tt_oname, (struct obj *));

/* ### track.c ### */

E void NDECL(initrack);
E void NDECL(settrack);
E coord *FDECL(gettrack, (int,int));

/* ### trap.c ### */

E boolean FDECL(rust_dmg, (struct obj *,const char *,int,BOOLEAN_P));
E struct trap *FDECL(maketrap, (int,int,int));
E int FDECL(teleok, (int,int));
E void FDECL(dotrap, (struct trap *));
E int FDECL(mintrap, (struct monst *));
E void FDECL(selftouch, (const char *));
E void NDECL(float_up);
E int NDECL(float_down);
E void NDECL(tele);
E void FDECL(teleds, (int,int));
E int NDECL(dotele);
E void FDECL(placebc, (int));
E void NDECL(unplacebc);
E void NDECL(level_tele);
E void NDECL(drown);
#ifdef SPELLS
E void FDECL(drain_en, (int));
#endif
E int NDECL(dountrap);
E void FDECL(chest_trap, (struct obj *,int));
E void NDECL(wake_nearby);
E void FDECL(deltrap, (struct trap *));
E struct trap *FDECL(t_at, (int,int));
E void FDECL(b_trapped, (const char *));
E boolean NDECL(unconscious);

/* ### u_init.c ### */

E void NDECL(u_init);
E void NDECL(plnamesuffix);

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
E void NDECL(askname);
#endif /* UNIX */

/* ### unixtty.c ### */

#ifdef UNIX
E void NDECL(gettty);
E void FDECL(settty, (const char *));
E void NDECL(setftty);
E void NDECL(intron);
E void NDECL(introff);
E void VDECL(error, (const char *,...));
#endif /* UNIX */

/* ### unixunix.c ### */

#ifdef UNIX
E void NDECL(setrandom);
E int NDECL(getyear);
E char *NDECL(getdate);
E int NDECL(phase_of_the_moon);
E int NDECL(night);
E int NDECL(midnight);
E void FDECL(gethdate, (char *));
E int FDECL(uptodate, (int));
E void NDECL(getlock);
E void FDECL(regularize, (char *));
#endif /* UNIX */

/* ### vault.c ### */

E void NDECL(invault);
E int FDECL(gd_move, (struct monst *));
E void NDECL(paygd);
E boolean NDECL(gd_sound);

/* ### version.c ### */

E int NDECL(doversion);
E int NDECL(doextversion);
#ifdef MSDOS
E int FDECL(comp_times,(long));
#endif

/* ### vmsmain.c ### */

#ifdef VMS
# ifdef CHDIR
E void FDECL(chdirx, (char *,BOOLEAN_P));
# endif /* CHDIR */
E void FDECL(glo, (int));
E void NDECL(askname);
#endif /* VMS */

/* ### vmsmisc.c ### */

#ifdef VMS
E void NDECL(vms_abort);
E void FDECL(vms_exit, (int));
#endif /* VMS */

/* ### vmstty.c ### */

#ifdef VMS
E void NDECL(gettty);
E void FDECL(settty, (char *));
E void NDECL(setftty);
E void NDECL(intron);
E void NDECL(introff);
E void VDECL(error, (char *,...));
#endif /* VMS */

/* ### vmsunix.c ### */

#ifdef VMS
E void NDECL(setrandom);
E int NDECL(getyear);
E char *NDECL(getdate);
E int NDECL(phase_of_the_moon);
E int NDECL(night);
E int NDECL(midnight);
E void FDECL(gethdate, (char *));
E int FDECL(uptodate, (int));
E void NDECL(getlock);
E void FDECL(regularize, (char *));
E int FDECL(vms_creat, (char *,unsigned int));
E int NDECL(vms_getuid);
E void NDECL(privoff);
E void NDECL(privon);
# ifdef SHELL
E int NDECL(dosh);
# endif
#endif /* VMS */

/* ### weapon.c ### */

E int FDECL(hitval, (struct obj *,struct permonst *));
E int FDECL(dmgval, (struct obj *,struct permonst *));
E void NDECL(set_uasmon);
E struct obj *FDECL(select_rwep, (struct monst *));
E struct obj *FDECL(select_hwep, (struct monst *));
E int NDECL(abon);
E int NDECL(dbon);

/* ### were.c ### */

E void FDECL(were_change, (struct monst *));
E void FDECL(new_were, (struct monst *));
E boolean FDECL(were_summon, (struct permonst *,BOOLEAN_P));
#ifdef POLYSELF
E void NDECL(you_were);
#endif /* POLYSELF */

/* ### wield.c ### */

E void FDECL(setuwep, (struct obj *));
E void NDECL(uwepgone);
E int NDECL(dowield);
E void NDECL(corrode_weapon);
E int FDECL(chwepon, (struct obj *,int));
E int FDECL(welded, (struct obj *));
E void FDECL(weldmsg, (struct obj *,BOOLEAN_P));

/* ### wizard.c ### */

E void NDECL(amulet);
E int FDECL(mon_has_amulet, (struct monst *));
E int FDECL(wiz_get_amulet, (struct monst *));
E void NDECL(aggravate);
E void NDECL(clonewiz);
#ifdef HARD
E void NDECL(nasty);
E void NDECL(resurrect);
E void NDECL(intervene);
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
E int NDECL(dozap);
E int FDECL(zapyourself, (struct obj *));
E void FDECL(weffects, (struct obj *));
E const char *FDECL(exclam, (int));
E void FDECL(hit, (const char *,struct monst *,const char *));
E void FDECL(miss, (const char *,struct monst *));
E struct monst *FDECL(bhit, (int,int,int,CHAR_P,int FDECL((*), (struct monst *, struct obj *)),
  int FDECL((*), (struct obj *, struct obj *)),struct obj *));
E struct monst *FDECL(boomhit, (int,int));
E void FDECL(buzz, (int,int,XCHAR_P,XCHAR_P,int,int));
E void FDECL(rlocgold, (struct gold *));
E void FDECL(rloco, (struct obj *));
E void FDECL(fracture_rock, (struct obj *));
E boolean FDECL(break_statue, (struct obj *));
E void FDECL(destroy_item, (int,int));
E int FDECL(destroy_mitem, (struct monst *,int,int));
E int FDECL(resist, (struct monst *,CHAR_P,int,int));
E void NDECL(makewish);

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#undef E

#endif /* EXTERN_H /**/
