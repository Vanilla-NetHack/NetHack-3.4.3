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
E long *alloc P((unsigned int));
#endif

#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### apply.c ### */

E int doapply();
E int holetime();
E void dighole();
E int dorub();
E int dojump();
#ifdef WALKIES
E int number_leashed();
E void o_unleash P((struct obj *));
E void m_unleash P((struct monst *));
E void unleash_all();
E boolean next_to_u();
E struct obj *get_mleash P((struct monst *));
E void check_leash P((XCHAR_P,XCHAR_P));
#endif
E boolean um_dist P((XCHAR_P,XCHAR_P,XCHAR_P));

/* ### artifact.c ### */

#ifdef NAMED_ITEMS
E void mkartifact P((struct obj **));
E boolean is_artifact P((struct obj *));
E boolean spec_ability P((struct obj *,unsigned));
E int restr_name P((struct obj *,char *));
# if defined(THEOLOGY) && defined(ALTARS)
E struct obj *mk_aligned_artifact P((unsigned));
# endif
E int defends P((int,struct obj *));
E int spec_abon P((struct obj *,struct permonst *));
E int spec_dbon P((struct obj *,struct permonst *,int));
#endif

/* ### attrib.c ### */

E void adjattrib P((int,int,BOOLEAN_P));
E void change_luck P((SCHAR_P));
E int stone_luck P((BOOLEAN_P));
E void gainstr P((struct obj *,int));
E void losestr P((int));
E void restore_attrib();
E void init_attr P((int));
E void redist_attr();
E void adjabil P((int));
E int newhp();
E schar acurr();
E void adjalign P((int));

/* ### bones.c ### */

E void savebones();
E int getbones();
E void name_file P((char *,int));

/* ### cmd.c ### */

E void reset_occupations ();
E void set_occupation P((int(*)(),char *,int));
#ifdef REDO
E char pgetchar();
E void pushch P((CHAR_P));
E void savech P((CHAR_P));
#endif
E void rhack P((char *));
E char lowc P((CHAR_P));
E void enlightenment();
E int xytod P((SCHAR_P,SCHAR_P));
E void dtoxy P((coord *,int));
E int movecmd P((CHAR_P));
E int getdir P((BOOLEAN_P));
E void confdir();
E int isok P((int,int));
E int doextlist();

/* ### dbridge.c ### */

E boolean is_pool P((int,int));
#ifdef STRONGHOLD
E void initsym P((int,int));
E int is_drawbridge_wall P((int, int));
E boolean is_db_wall P((int, int));
E boolean find_drawbridge P((int *, int*));
E boolean create_drawbridge P((int, int, int, BOOLEAN_P));
E void open_drawbridge P((int, int));
E void close_drawbridge P((int, int));
E void destroy_drawbridge P((int, int));
#endif	/* STRONGHOLD /**/

/* ### decl.c ### */

/* ### demon.c ### */

E void dsummon P((struct permonst *));
E int demon_talk P((struct monst *));
E long bribe P((struct monst *));
E int dprince ();
E int dlord ();
E int ndemon ();

/* ### do.c ### */

E int dodrop();
E boolean flooreffects P((struct obj *,int,int));
E void doaltarobj P((struct obj *));
E boolean canletgo P((struct obj *,char *));
E void dropx P((struct obj *));
E void dropy P((struct obj *));
E int doddrop();
E int dodown();
E int doup();
E void goto_level P((int,BOOLEAN_P,BOOLEAN_P));
E int donull();
E int dowipe();
E struct obj *splitobj P((struct obj *,int));
E void set_wounded_legs P((long,int));
E void heal_legs();

/* ### do_name.c ### */

E void getpos P((coord *,int,char *));
E int do_mname();
E struct obj *oname P((struct obj *,char *,int));
E int ddocall();
E void docall P((struct obj *));
E char *x_monnam P((struct monst *,int));
E char *lmonnam P((struct monst *));
E char *mon_nam P((struct monst *));
E char *Monnam P((struct monst *));
E char *a_monnam P((struct monst *,char *));
E char *a2_monnam P((struct monst *,char *));
E char *Amonnam P((struct monst *,char *));
E char *Xmonnam P((struct monst *));
E char *defmonnam P((struct monst *));
E char *rndmonnam();
#ifdef REINCARNATION
E char *roguename();
#endif

/* ### do_wear.c ### */

E void off_msg P((struct obj *));
E boolean is_helmet P((struct obj *));
E boolean is_gloves P((struct obj *));
E boolean is_boots P((struct obj *));
E boolean is_cloak P((struct obj *));
E boolean is_shield P((struct obj *));
E void set_wear();
E int Armor_off();
E int Armor_gone();
E int Helmet_off();
E int Gloves_off();
E int Boots_off();
E int Cloak_off();
E int Shield_off();
E void Amulet_off();
E void Ring_on P((struct obj *));
E void Ring_off P((struct obj *));
E void Ring_gone P((struct obj *));
E void Blindf_on P((struct obj *));
E void Blindf_off P((struct obj *));
E int dotakeoff();
E int doremring();
E int cursed P((struct obj *));
E int armoroff P((struct obj *));
E int dowear();
E int doputon();
E void find_ac();
E void glibr();
E struct obj *some_armor();
E void corrode_armor();
E void reset_remarm();
E int doddoremarm();
E int destroy_arm P((struct obj *));
E void adj_abon P((struct obj *,SCHAR_P));

/* ### dog.c ### */

E void initedog P((struct monst *));
E void make_familiar P((struct obj *));
E struct monst *makedog();
E void losedogs();
E void keepdogs();
E void fall_down P((struct monst *,int));
E int dogfood P((struct monst *,struct obj *));
E int inroom P((XCHAR_P,XCHAR_P));
E int tamedog P((struct monst *,struct obj *));

/* ### dogmove.c ### */

E int dog_move P((struct monst *,int));

/* ### dokick.c ### */

E boolean ghitm P((struct monst *,long));
E boolean bad_kick_throw_pos P((XCHAR_P,XCHAR_P));
E struct monst *ghit P((int,int,int));
E int dokick();

/* ### dothrow.c ### */

E int dothrow();
E int throwit P((struct obj *));
E int thitmonst P((struct monst *,struct obj *));
E int breaks P((struct obj *,BOOLEAN_P));

/* ### eat.c ### */

E void init_uhunger();
E int Hear_again();
E void reset_eat();
E int doeat();
E void gethungry();
E void morehungry P((int));
E void lesshungry P((int));
E void newuhs P((BOOLEAN_P));
E struct obj *floorfood P((char *,BOOLEAN_P));
E void vomit();

/* ### end.c ### */

E int done1();
E int done2();
E void done_in_by P((struct monst *));
E void panic V((char *,...));
E void done P((int));
E void clearlocks();
#ifdef NOSAVEONHANGUP
E void hangup();
#endif

/* ### engrave.c ### */

#ifdef ELBERETH
E int sengr_at P((char *,XCHAR_P,XCHAR_P));
#endif
E void u_wipe_engr P((int));
E void wipe_engr_at P((XCHAR_P,XCHAR_P,XCHAR_P));
E void read_engr_at P((int,int));
E void make_engr_at P((int,int,char *));
E int freehand();
E int doengrave();
E void save_engravings P((int));
E void rest_engravings P((int));

/* ### exper.c ### */

E long newuexp P((unsigned));
E int experience P((struct monst *,int));
E void more_experienced P((int,int));
E void losexp();
E void newexplevel();
E void pluslvl();
E long rndexp();

/* ### extralev.c ### */

#ifdef REINCARNATION
E void makeroguerooms();
E void corr P((int,int));
E void makerogueghost();
#endif

/* ### fountain.c ### */

#ifdef FOUNTAINS
E void dryup();
E void drinkfountain();
E void dipfountain P((struct obj *));
#endif /* FOUNTAINS */
#ifdef SINKS
E void drinksink();
#endif

/* ### getline.c ### */

E void getlin P((char *));
E void getret();
E void cgetret P((char *));
E void xwaitforspace P((char *));
E char *parse();
E char readchar();
#ifdef COM_COMPL
E void get_ext_cmd P((char *));
#endif /* COM_COMPL */

/* ### hack.c ### */

E void unsee();
E void seeoff P((int));
E void movobj P((struct obj *,XCHAR_P,XCHAR_P));
E boolean may_dig P((XCHAR_P,XCHAR_P));
E void domove();
E void spoteffects();
E int dopickup();
E void lookaround();
E int monster_nearby();
E int cansee P((XCHAR_P,XCHAR_P));
E int sgn P((int));
E void getcorners
	P((xchar *,xchar *,xchar *,xchar *,xchar *,xchar *,xchar *,xchar *));
E void setsee();
E void nomul P((int));
E void losehp P((int,char *));
E int weight_cap();
E int inv_weight();
E int inv_cnt();
E int identify P((struct obj *));
#ifdef STUPID_CPP	/* otherwise these functions are macros in hack.h */
E char yn();
E char ynq();
E char ynaq();
E char nyaq();
E char *plur P((long));
E void makeknown P((unsigned));
#endif

/* ### invent.c ### */

E struct obj *addinv P((struct obj *));
E void useup P((struct obj *));
E void freeinv P((struct obj *));
E void delobj P((struct obj *));
E void freeobj P((struct obj *));
E void freegold P((struct gold *));
E struct obj *sobj_at P((int,int,int));
E int carried P((struct obj *));
E struct obj *carrying P((int));
E struct obj *o_on P((unsigned int,struct obj *));
E struct gold *g_at P((int,int));
E struct obj *getobj P((char *,char *));
E int ggetobj P((char *,int(*)(),int));
E int askchain P((struct obj *,int,char *,int,int(*)(),int(*)(),int,char *));
E void prinv P((struct obj *));
E int ddoinv();
E void doinv P((char *));
E int dotypeinv();
E int dolook();
E void stackobj P((struct obj *));
E int doprgold();
E int doprwep();
E int doprarm();
E int doprring();
E int dopramulet();
E int doprtool();
E int digit P((CHAR_P));
E void useupf P((struct obj *));
E char *let_to_name P((CHAR_P));
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

E void reset_pick();
E int pick_lock P((struct obj *));
E int doforce();
E int boxlock P((struct obj *,struct obj *));
E int doorlock P((struct obj *,int,int));
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

E struct monst *makemon P((struct permonst *,int,int));
E void enexto P((coord *,XCHAR_P,XCHAR_P,struct permonst *));
E int goodpos P((int,int, struct permonst *));
E void rloc P((struct monst *));
E void vloc P((struct monst *));
E void init_monstr();
E struct permonst *rndmonst();
E struct permonst *mkclass P((CHAR_P));
E int adj_lev P((struct permonst *));
E struct permonst *grow_up P((struct monst *));
E int mongets P((struct monst *,int));
#ifdef REINCARNATION
E struct permonst *roguemon();
#endif
#ifdef GOLEMS
E int golemhp P((int));
#endif /* GOLEMS */
E boolean peace_minded P((struct permonst *));
E void set_malign P((struct monst *));
E void set_mimic_sym P((struct monst *));

/* ### mcastu.c ### */

E int castmu P((struct monst *,struct attack *));
E int buzzmu P((struct monst *,struct attack *));

/* ### mhitm.c ### */

E int fightm P((struct monst *));
E int mattackm P((struct monst *,struct monst *));
E int noattacks P((struct permonst *));

/* ### mhitu.c ### */

#ifdef POLYSELF
E struct monst *cloneu();
#endif
E void regurgitates P((struct monst *));
E int mattacku P((struct monst *));
E void mdamageu P((struct monst *,int));
E int could_seduce P((struct monst *,struct monst *,struct attack *));
#ifdef SEDUCE
E int doseduce P((struct monst *));
#endif

/* ### mklev.c ### */

E int somex P((struct mkroom *));
E int somey P((struct mkroom *));
#ifdef ORACLE
E boolean place_oracle P((struct mkroom *,int *,int *,int *));
#endif
E void mklev();
E int okdoor P((XCHAR_P,XCHAR_P));
E void dodoor P((int,int,struct mkroom *));
E void mktrap P((int,int,struct mkroom *));
E void mkfount P((int,struct mkroom *));

/* ### mkmaze.c ### */

#if defined(WALLIFIED_MAZE) || defined(STRONGHOLD)
E void wallification P((int,int,int,int,BOOLEAN_P));
#endif
E void walkfrom P((int,int));
E void makemaz();
E void move P((int *,int *,int));
E void mazexy P((coord *));
E void bound_digging();

/* ### mkobj.c ### */

E struct obj *mkobj_at P((CHAR_P,int,int));
E struct obj *mksobj_at P((int,int,int));
E struct obj *mkobj P((CHAR_P,BOOLEAN_P));
E int rndmonnum();
E struct obj *mksobj P((int,BOOLEAN_P));
E int letter P((int));
E int weight P((struct obj *));
E void mkgold P((long,int,int));
E struct obj *mkcorpstat P((int,struct permonst *,int,int));
E struct obj *mk_tt_object P((int,int,int));
E struct obj *mk_named_object P((int,struct permonst *,int,int,char *,int));
E void bless P((struct obj *));
E void curse P((struct obj *));
E void blessorcurse P((struct obj *,int));
#ifdef STUPID_CPP
E boolean is_flammable P((struct obj *));
E boolean is_rustprone P((struct obj *));
E boolean is_corrodeable P((struct obj *));
E boolean OBJ_AT P((int, int));
#endif
E void place_object P((struct obj *,int,int));
E void move_object P((struct obj *,int,int));
E void remove_object P((struct obj *));

/* ### mkroom.c ### */

E void mkroom P((int));
E void shrine_pos P((int *, int*, struct mkroom *));
E boolean nexttodoor P((int,int));
E boolean has_dnstairs P((struct mkroom *));
E boolean has_upstairs P((struct mkroom *));
E int dist2 P((int,int,int,int));
E struct permonst *courtmon();
E int bcsign P((struct obj *));

/* ### mon.c ### */

E void movemon();
E void meatgold P((struct monst *));
E void meatobj P((struct monst *));
E void mpickgold P((struct monst *));
E void mpickgems P((struct monst *));
E int curr_mon_load P((struct monst *));
E int max_mon_load P((struct monst *));
E boolean can_carry P((struct monst *,struct obj *));
E void mpickstuff P((struct monst *,char *));
E int mfndpos P((struct monst *,coord *,long *,long));
E int dist P((int,int));
E void poisontell P((int));
E void poisoned P((char *,int,char *));
E void mondead P((struct monst *));
E void replmon P((struct monst *,struct monst *));
E void relmon P((struct monst *));
E void monfree P((struct monst *));
E void unstuck P((struct monst *));
E void killed P((struct monst *));
E void xkilled P((struct monst *,int));
E void rescham();
E void restartcham();
E int newcham P((struct monst *,struct permonst *));
E void mnexto P((struct monst *));
E void mnearto P((struct monst *, XCHAR_P, XCHAR_P, BOOLEAN_P));
E void setmangry P((struct monst *));
E int disturb P((struct monst *));
E void mondied P((struct monst *));
E void mongone P((struct monst *));
E void monstone P((struct monst *));
#ifdef GOLEMS
E void golemeffects P((struct monst *, int, int));
#endif /* GOLEMS */

/* ### mondata.c ### */

E boolean attacktype P((struct permonst *,int));
E boolean resists_ston P((struct permonst *));
E boolean resists_drli P((struct permonst *));
E boolean ranged_attk P((struct permonst *));
E boolean can_track P((struct permonst *));
#ifdef POLYSELF
E boolean breakarm P((struct permonst *));
E boolean sliparm P((struct permonst *));
#endif
E boolean sticks P((struct permonst *));
E boolean canseemon P((struct monst *));
E boolean dmgtype P((struct permonst *,int));
E int monsndx P((struct permonst *));
E int name_to_mon P((char *));
#ifdef POLYSELF
E boolean webmaker P((struct permonst *));
#endif
E boolean is_female P((struct monst *));
E int gender P((struct monst *));
E boolean levl_follower P((struct monst *));
E struct permonst *player_mon();
E int little_to_big P((int));
E int big_to_little P((int));     

/* ### monmove.c ### */

E boolean mb_trapped P((struct monst *));
E int dochugw P((struct monst *));
E boolean onscary P((int,int,struct monst *));
E int dochug P((struct monst *));
E int m_move P((struct monst *,int));
E void set_apparxy P((struct monst *));
E boolean mdig_tunnel P((struct monst *));
#ifdef STUPID_CPP
E boolean MON_AT P((int, int));
E void place_monster P((struct monst *, int, int));
E void place_worm_seg P((struct monst *, int, int));
E void remove_monster P((int, int));
E struct monst *m_at P((int, int));
#endif

/* ### monst.c ### */

/* ### msdos.c ### */

#ifdef MSDOS
E void flushout();
E int tgetch();
E int dosh();
# ifdef DGK
E long freediskspace P((char *));
E long filesize P((char *));
E void eraseall P((char *,char *));
E void copybones P((int));
E void playwoRAMdisk();
E int saveDiskPrompt P((int));
E void gameDiskPrompt();
# endif
E void read_config_file();
E void set_lock_and_bones();
E void append_slash P((char *));
E void getreturn P((char *));
E void msmsg V((char *,...));
E void chdrive P((char *));
# ifndef TOS
E void disable_ctrlP();
E void enable_ctrlP();
# endif
# ifdef DGK
E FILE *fopenp P((char *,char *));
# endif
E void msexit P((int));
# ifdef DGK
E void get_scr_size();
# endif
#endif /* MSDOS */
#ifdef TOS
E int _copyfile P((char *, char *));
E int kbhit();
E void restore_colors();
E void set_colors();
#endif /* TOS */

/* ### mthrowu.c ### */

E int thitu P((int,int,struct obj *,char *));
E int thrwmu P((struct monst *));
E int spitmu P((struct monst *));
E int breamu P((struct monst *,struct attack *));
E boolean linedup P((XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E boolean lined_up P((struct monst *));
E struct obj *m_carrying P((struct monst *,int));
E void m_useup P((struct monst *,struct obj *));

/* ### music.c ### */

#ifdef MUSIC
E int do_play_instrument P((struct obj *));
#endif /* MUSIC /**/

/* ### o_init.c ### */

E int letindex P((CHAR_P));
E void init_objects();
E void oinit();
E void savenames P((int));
E void restnames P((int));
E int dodiscovered();

/* ### objnam.c ### */

E char *typename P((int));
E char *distant_name P((struct obj *, char *(*)(struct obj *)));
E char *xname P((struct obj *));
E char *doname P((struct obj *));
E char *singular P((struct obj *));
E void setan P((char *,char *));
E char *aobjnam P((struct obj *,char *));
E char *Doname2 P((struct obj *));
E void lcase P((char *));
E char *makeplural P((char *));
E struct obj *readobjnam P((char *));

/* ### options.c ### */

E void initoptions();
E void assign_graphics P((unsigned int *, int));
E void parseoptions P((char *,BOOLEAN_P));
E int doset();
E int dotogglepickup();
E void option_help();
#ifdef TUTTI_FRUTTI
E int fruitadd P((char *));
#endif

/* ### pager.c ### */

E int dowhatis();
E int dowhatdoes();
E void set_whole_screen();
#ifdef NEWS
E int readnews();
#endif /* NEWS */
E void set_pager P((int));
E int page_line P((char *));
E void cornline P((int,char *));
E int dohelp();
E int dohistory();
E int page_file P((char *,BOOLEAN_P));
#ifdef UNIX
# ifdef SHELL
E int dosh();
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
E int child P((int));
# endif
#endif /* UNIX */

/* ### pcmain.c ### */

#if defined(MSDOS) || defined(MACOS)
E void askname();
# ifdef CHDIR
E void chdirx P((char *,BOOLEAN_P));
# endif /* CHDIR */
#endif /* MSDOS || MACOS */

/* ### pctty.c ### */

#if defined(MSDOS) || defined(MACOS)
E void gettty();
E void settty P((char *));
E void error V((char *,...));
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
E void gethdate P((char *));
E int uptodate P((int));
# endif /* TOS */
E void regularize P((char *));
#endif /* MSDOS */

/* ### pickup.c ### */

E void pickup P((int));
E struct obj *pick_obj P((struct obj *));
E int doloot();
E void get_all_from_box();
E void use_container P((struct obj *, int));
E void inc_cwt P((struct obj *, struct obj *));
E void delete_contents P((struct obj *));
E void dec_cwt P((struct obj *, struct obj *));

/* ### polyself.c ### */

E void newman();
#ifdef POLYSELF
E void polyself();
E int polymon P((int));
E void rehumanize();
E int dobreathe();
E int dospit();
E int doremove();
E int dospinweb();
E int dosummon();
E int doconfuse();
E int dohide();
#endif
E char *body_part P((int));
E int poly_gender();
#ifdef POLYSELF
# ifdef GOLEMS
E void ugolemeffects P((int, int));
# endif /* GOLEMS */
#endif

/* ### potion.c ### */

E void make_confused P((long,BOOLEAN_P));
E void make_stunned P((long,BOOLEAN_P));
E void make_blinded P((long,BOOLEAN_P));
E void make_sick P((long,BOOLEAN_P));
E void make_vomiting P((long,BOOLEAN_P));
E void make_hallucinated P((long,BOOLEAN_P));
E int dodrink();
E int dopotion P((struct obj *));
E int peffects P((struct obj *));
E void healup P((int,int,BOOLEAN_P,BOOLEAN_P));
E void strange_feeling P((struct obj *,char *));
E void potionhit P((struct monst *,struct obj *));
E void potionbreathe P((struct obj *));
E int dodip();
E void djinni_from_bottle P((struct obj *));
E int monster_detect P((struct obj *));
E int object_detect P((struct obj *));
E int trap_detect P((struct obj *));

/* ### pray.c ### */

# ifdef THEOLOGY
E int dosacrifice();
E int dopray();
E char *u_gname();
#endif
E int doturn();
#ifdef ALTARS
E char *a_gname();
E char *a_gname_at P((XCHAR_P,XCHAR_P));
# ifdef THEOLOGY
E void altar_wrath P((int,int));
# endif
#endif

/* ### pri.c ### */

E void swallowed();
E void setclipped();
E void at P((XCHAR_P,XCHAR_P,UCHAR_P,UCHAR_P));
E void prme();
E void shieldeff P((XCHAR_P,XCHAR_P));
E int doredraw();
E void docrt();
E void docorner P((int,int));
E void seeglds();
E void seeobjs();
E void seemons();
E void pmon P((struct monst *));
E void unpmon P((struct monst *));
E void nscr();
E void bot();
E void mstatusline P((struct monst *));
E void ustatusline();
E void cls();
E void max_rank_sz();
E char rndmonsym();
E char rndobjsym();
E const char *hcolor();

/* ### priest.c ### */

E int move_special P((struct monst *,SCHAR_P,SCHAR_P,BOOLEAN_P,BOOLEAN_P,
			XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
#if defined(ALTARS) && defined(THEOLOGY)
E struct mkroom *in_temple P((int,int));
E int pri_move P((struct monst *));
E void priestini P((int,int,int,int));
E char *priestname P((struct monst *));
E boolean p_coaligned P((struct monst *));
E void intemple();
E void priest_talk P((struct monst *));
E boolean u_in_sanctuary P((struct mkroom *));
E void ghod_hitsu();
E void angry_priest();
#endif

/* ### prisym.c ### */

E void atl P((int,int,CHAR_P));
E void on_scr P((int,int));
E void tmp_at P((int,int));
E void Tmp_at2 P((int,int));
E void curs_on_u();
E void pru();
E void prl P((int,int));
E uchar news0 P((XCHAR_P,XCHAR_P));
E void newsym P((int,int));
E void mnewsym P((int,int));
E void nosee P((int,int));
E void prl1 P((int,int));
E void nose1 P((int,int));
E int vism_at P((int,int));
#ifdef NEWSCR
E void pobj P((struct obj *));
#endif /* NEWSCR */
E void unpobj P((struct obj *));

/* ### read.c ### */

E int doread();
E int seffects P((struct obj *));
E void litroom P((BOOLEAN_P));
E void do_genocide P((int));
E void do_mapping();
E void do_vicinity_map();
E int gold_detect P((struct obj *));
E int food_detect P((struct obj *));
E void punish P((struct obj *));
E void unpunish();
E boolean cant_create P((int *));
#if defined(WIZARD) || defined(EXPLORE_MODE)
E boolean create_particular();
#endif

/* ### restore.c ### */

E int dorecover P((int));
E void getlev P((int,int,XCHAR_P,BOOLEAN_P));
#ifdef ZEROCOMP
E void minit();
E int mread P((int,genericptr_t,unsigned int));
#else
E void mread P((int,genericptr_t,unsigned int));
#endif

/* ### rip.c ### */

E void outrip();

/* ### rnd.c ### */

E int rn1 P((int,int));
E int rn2 P((int));
E int rnl P((int));
E int rnd P((int));
E int d P((int,int));
E int rne P((int));
#ifdef THEOLOGY
E int rnz P((int));
#endif

/* ### rumors.c ### */

E void outrumor P((int,BOOLEAN_P));
#ifdef ORACLE
E int doconsult P((struct monst *));
#endif

/* ### save.c ### */

E int dosave();
#ifndef NOSAVEONHANGUP
E int hangup();
#endif /* NOSAVEONHANGUP */
E int dosave0();
#if defined(DGK) && !defined(OLD_TOS)
E boolean savelev P((int,XCHAR_P,int));
E boolean swapin_file P((int));
#else /* DGK && !OLD_TOS */
E void savelev P((int, XCHAR_P));
#endif /* DGK && !OLD_TOS */
#ifdef ZEROCOMP
E void bflush P((int));
#endif
E void bwrite P((int,genericptr_t,unsigned int));
#ifdef TUTTI_FRUTTI
E void savefruitchn P((int));
#endif

/* ### search.c ### */

E int findit();
E int dosearch();
E int dosearch0 P((int));
E int doidtrap();
E void wakeup P((struct monst *));
E void seemimic P((struct monst *));

/* ### shk.c ### */

E char *shkname P((struct monst *));
E void shkdead P((struct monst *));
E void replshk P((struct monst *,struct monst *));
E int inshop();
E int inhishop P((struct monst *));
E boolean tended_shop P((int));
E void obfree P((struct obj *,struct obj *));
E int dopay();
E void home_shk P((struct monst *));
E void make_happy_shk P((struct monst *));
E boolean paybill();
E void pay_for_door P((int,int,char *));
E void addtobill P((struct obj *,BOOLEAN_P));
E void splitbill P((struct obj *,struct obj *));
E void sellobj P((struct obj *));
E int doinvbill P((int));
E int shkcatch P((struct obj *));
E int shk_move P((struct monst *));
E int online P((XCHAR_P,XCHAR_P));
E boolean is_fshk P((struct monst *));
E void shopdig P((int));
E boolean in_shop P((int,int));
E boolean costly_spot P((int,int));
E void check_unpaid P((struct obj *));

/* ### shknam.c ### */

E void stock_room P((struct shclass *,struct mkroom *));
E int saleable P((int,struct obj *));
E int get_shop_item P((int));

/* ### sit.c ### */

#if defined(THRONES) || defined(SPELLS)
E void take_gold();
#endif
E int dosit();
E void rndcurse();
E void attrcurse();

/* ### sp_lev.c ### */

#ifdef STRONGHOLD
E boolean load_special P((char *));
#endif /* STRONGHOLD /**/

/* ### sounds.c ### */

E void verbalize P((char *));
#ifdef SOUNDS
E void dosounds();
E void growl P((struct monst *));
E void yelp P((struct monst *));
E void whimper P((struct monst *));
#endif
E int dotalk();

/* ### spell.c ### */

#ifdef SPELLS
E int study_book P((struct obj *));
E int docast();
E int spelleffects P((int, BOOLEAN_P));
E void losespells();
E int dovspell();
#endif /* SPELLS */

/* ### steal.c ### */

E long somegold();
E void stealgold P((struct monst *));
E int steal P((struct monst *));
E void mpickobj P((struct monst *,struct obj *));
E void stealamulet P((struct monst *));
E void relobj P((struct monst *,int));

/* ### termcap.c ### */

E void startup();
E void start_screen();
E void end_screen();
E void curs P((int,int));
E void cmov P((int,int));
E void xputc P((CHAR_P));
E void xputs P((char *));
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
E void addtopl P((char *));
E void more();
E void cmore P((char *));
E void clrlin();
#ifdef NEED_VARARGS
# if defined(USE_STDARG) || defined(USE_VARARGS)
E void vpline P((const char *, va_list));
# endif
#endif
E void pline V((const char *,...));
E void Norep V((const char *,...));
E void You V((const char *,...));
E void Your V((const char *,...));
E void kludge V((char *,char *,...));
E void putsym P((CHAR_P));
E void putstr P((char *));
E char yn_function P((char *,CHAR_P));
E void impossible V((char *,...));

/* ### topten.c ### */

E void topten();
E char *eos P((char *));
E void prscore P((int,char **));
E struct obj *tt_oname P((struct obj *));

/* ### track.c ### */

E void initrack();
E void settrack();
E coord *gettrack P((int,int));

/* ### trap.c ### */

E boolean rust_dmg P((struct obj *,char *,int,BOOLEAN_P));
E struct trap *maketrap P((int,int,int));
E int teleok P((int,int));
E void dotrap P((struct trap *));
E int mintrap P((struct monst *));
E void selftouch P((char *));
E void float_up();
E int float_down();
E void tele();
E void teleds P((int,int));
E int dotele();
E void placebc P((int));
E void unplacebc();
E void level_tele();
E void drown();
#ifdef SPELLS
E void drain_en P((int));
#endif
E int dountrap();
E void chest_trap P((struct obj *,int));
E void wake_nearby();
E void deltrap P((struct trap *));
E struct trap *t_at P((int,int));
E void b_trapped();
E boolean unconscious();

/* ### u_init.c ### */

E void u_init();
E void plnamesuffix();

/* ### uhitm.c ### */

E struct monst *clone_mon P((struct monst *));
E boolean special_case P((struct monst *));
E schar find_roll_to_hit P((struct monst *));
E boolean attack P((struct monst *));
E boolean hmon P((struct monst *,struct obj *,int));
E int damageum P((struct monst *, struct attack *));
E void missum P((struct monst *, struct attack *));
E int passive P((struct monst *,BOOLEAN_P,int,BOOLEAN_P));
E void stumble_onto_mimic P((struct monst *));

/* ### unixmain.c ### */

#ifdef UNIX
E void glo P((int));
E void askname();
#endif /* UNIX */

/* ### unixtty.c ### */

#ifdef UNIX
E void gettty();
E void settty P((char *));
E void setftty();
E void intron();
E void introff();
E void error V((char *,...));
#endif /* UNIX */

/* ### unixunix.c ### */

#ifdef UNIX
E void setrandom();
E int getyear();
E char *getdate();
E int phase_of_the_moon();
E int night();
E int midnight();
E void gethdate P((char *));
E int uptodate P((int));
E void getlock();
E void regularize P((char *));
#endif /* UNIX */

/* ### vault.c ### */

E void setgd();
E void invault();
E int gd_move();
E void gddead();
E void replgd P((struct monst *,struct monst *));
E void paygd();

/* ### version.c ### */

E int doversion();

/* ### vmsmain.c ### */

#ifdef VMS
# ifdef CHDIR
E void chdirx P((char *,char));
# endif /* CHDIR */
E void glo P((int));
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
E void settty P((char *));
E void setftty();
E void intron();
E void introff();
E void error V((char *,...));
#endif /* VMS */

/* ### vmsunix.c ### */

#ifdef VMS
E void setrandom();
E int getyear();
E char *getdate();
E int phase_of_the_moon();
E int night();
E int midnight();
E void gethdate P((char *));
E int uptodate P((int));
E void getlock();
E void regularize P((char *));
E int vms_creat P((char *,unsigned int));
E int vms_getuid();
E void privoff();
E void privon();
# ifdef SHELL
E int dosh();
# endif
#endif /* VMS */

/* ### weapon.c ### */

E int hitval P((struct obj *,struct permonst *));
E int dmgval P((struct obj *,struct permonst *));
E void set_uasmon();
E struct obj *select_rwep P((struct monst *));
E struct obj *select_hwep P((struct monst *));
E int abon();
E int dbon();

/* ### were.c ### */

E void were_change P((struct monst *));
E void new_were P((struct monst *));
E boolean were_summon P((struct permonst *,BOOLEAN_P));
#ifdef POLYSELF
E void you_were();
#endif /* POLYSELF */

/* ### wield.c ### */

E void setuwep P((struct obj *));
E void uwepgone();
E int dowield();
E void corrode_weapon();
E int chwepon P((struct obj *,int));
E int welded P((struct obj *));
E void weldmsg P((struct obj *,BOOLEAN_P));

/* ### wizard.c ### */

E void amulet();
E int mon_has_amulet P((struct monst *));
E int wiz_get_amulet P((struct monst *));
E void aggravate();
E void clonewiz();
#ifdef HARD
E void nasty();
E void resurrect();
E void intervene();
E void wizdead P((struct monst *));
#endif /* HARD */
E void cuss P((struct monst *));

/* ### worm.c ### */

#ifdef WORM
E int getwn P((struct monst *));
E void initworm P((struct monst *));
E void worm_move P((struct monst *));
E void worm_nomove P((struct monst *));
E void wormdead P((struct monst *));
E void wormhit P((struct monst *));
E void wormsee P((unsigned int));
E void cutworm P((struct monst *,XCHAR_P,XCHAR_P,unsigned));
#endif /* WORM */

/* ### worn.c ### */

E void setworn P((struct obj *,long));
E void setnotworn P((struct obj *));

/* ### write.c ### */

E void dowrite P((struct obj *));

/* ### zap.c ### */

E struct monst *revive P((struct obj *,BOOLEAN_P));
E int zappable P((struct obj *));
E void zapnodir P((struct obj *));
E int dozap();
E int zapyourself P((struct obj *));
E void weffects P((struct obj *));
E char *exclam P((int));
E void hit P((char *,struct monst *,char *));
E void miss P((char *,struct monst *));
E struct monst *bhit P((int,int,int,CHAR_P,int(*)(),int(*)(),struct obj *));
E struct monst *boomhit P((int,int));
E void buzz P((int,int,XCHAR_P,XCHAR_P,int,int));
E void rloco P((struct obj *));
E void fracture_rock P((struct obj *));
E boolean break_statue P((struct obj *));
E void destroy_item P((int,int));
E int destroy_mitem P((struct monst *,int,int));
E int resist P((struct monst *,CHAR_P,int,int));
E void makewish();

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#undef E

#endif /* EXTERN_H /**/
