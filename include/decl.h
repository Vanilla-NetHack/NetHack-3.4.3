/*	SCCS Id: @(#)decl.h	3.1	92/12/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

E int NDECL((*occupation));
E int NDECL((*afternmv));

E const char *hname;
E int hackpid;
#if defined(UNIX) || defined(VMS)
E int locknum;
#endif
#ifdef DEF_PAGER
E char *catmore;
#endif	/* DEF_PAGER */

E char SAVEF[];
#ifdef MICRO
E char SAVEP[];
#endif

E int NEARDATA bases[];

E int NEARDATA multi;
E int NEARDATA warnlevel;
E int NEARDATA nroom;
E int NEARDATA nsubroom;
E int NEARDATA occtime;

E int x_maze_max, y_maze_max;
E int otg_temp;

#ifdef REDO
E int NEARDATA in_doagain;
#endif

E struct dgn_topology {	/* special dungeon levels for speed */
    d_level	d_oracle_level;
    d_level	d_bigroom_level;	/* unused */
#ifdef REINCARNATION
    d_level	d_rogue_level;
#endif
    d_level	d_medusa_level;
    d_level	d_stronghold_level;
    d_level	d_valley_level;
    d_level	d_wiz1_level;
    d_level	d_wiz2_level;
    d_level	d_wiz3_level;
    d_level     d_juiblex_level;
    d_level     d_orcus_level;
    d_level	d_baalzebub_level;	/* unused */
    d_level	d_asmodeus_level;	/* unused */
    d_level	d_portal_level;		/* only in goto_level() [do.c] */
    d_level	d_sanctum_level;
    d_level	d_earth_level;
    d_level	d_water_level;
    d_level	d_fire_level;
    d_level	d_air_level;
    d_level	d_astral_level;
    xchar	d_tower_dnum;
#ifdef MULDGN
    xchar	d_mines_dnum, d_quest_dnum;
    d_level	d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level	d_knox_level;
#endif
} dungeon_topology;
/* macros for accesing the dungeon levels by their old names */
#define oracle_level		(dungeon_topology.d_oracle_level)
#define bigroom_level		(dungeon_topology.d_bigroom_level)
#ifdef REINCARNATION
#define rogue_level		(dungeon_topology.d_rogue_level)
#endif
#define medusa_level		(dungeon_topology.d_medusa_level)
#define stronghold_level	(dungeon_topology.d_stronghold_level)
#define valley_level		(dungeon_topology.d_valley_level)
#define wiz1_level		(dungeon_topology.d_wiz1_level)
#define wiz2_level		(dungeon_topology.d_wiz2_level)
#define wiz3_level		(dungeon_topology.d_wiz3_level)
#define juiblex_level		(dungeon_topology.d_juiblex_level)
#define orcus_level		(dungeon_topology.d_orcus_level)
#define baalzebub_level		(dungeon_topology.d_baalzebub_level)
#define asmodeus_level		(dungeon_topology.d_asmodeus_level)
#define portal_level		(dungeon_topology.d_portal_level)
#define sanctum_level		(dungeon_topology.d_sanctum_level)
#define earth_level		(dungeon_topology.d_earth_level)
#define water_level		(dungeon_topology.d_water_level)
#define fire_level		(dungeon_topology.d_fire_level)
#define air_level		(dungeon_topology.d_air_level)
#define astral_level		(dungeon_topology.d_astral_level)
#define tower_dnum		(dungeon_topology.d_tower_dnum)
#ifdef MULDGN
#define mines_dnum		(dungeon_topology.d_mines_dnum)
#define quest_dnum		(dungeon_topology.d_quest_dnum)
#define qstart_level		(dungeon_topology.d_qstart_level)
#define qlocate_level		(dungeon_topology.d_qlocate_level)
#define nemesis_level		(dungeon_topology.d_nemesis_level)
#define knox_level		(dungeon_topology.d_knox_level)
#endif

E stairway NEARDATA dnstair, NEARDATA upstair; /* stairs up and down. */
#define xdnstair	(dnstair.sx)
#define ydnstair	(dnstair.sy)
#define xupstair	(upstair.sx)
#define yupstair	(upstair.sy)

E stairway NEARDATA dnladder, NEARDATA upladder; /* ladders up and down. */
#define xdnladder	(dnladder.sx)
#define ydnladder	(dnladder.sy)
#define xupladder	(upladder.sx)
#define yupladder	(upladder.sy)

E stairway NEARDATA sstairs;

E dest_area NEARDATA updest, NEARDATA dndest; /* level-change dest. areas */

E coord NEARDATA inv_pos;
E dungeon NEARDATA dungeons[];
E s_level NEARDATA *sp_levchn;
#define dunlev_reached(x)	(dungeons[(x)->dnum].dunlev_ureached)

#ifdef MULDGN
#include "quest.h"
E struct q_score 	quest_status;
#endif

E int NEARDATA done_stopprint;
E int NEARDATA done_hup;

E char NEARDATA pl_character[PL_CSIZ];
#ifdef TUTTI_FRUTTI
E char NEARDATA pl_fruit[PL_FSIZ];
E int NEARDATA current_fruit;
E struct fruit NEARDATA *ffruit;
#endif

E char NEARDATA tune[6];

E const char NEARDATA quitchars[];
E const char NEARDATA vowels[];
E const char NEARDATA ynchars[];
E const char NEARDATA ynqchars[];
E const char NEARDATA ynaqchars[];
E const char NEARDATA ynNaqchars[];
E long NEARDATA yn_number;
E int NEARDATA smeq[];
E int NEARDATA doorindex;
E char NEARDATA *save_cm;
#define KILLED_BY_AN	 0
#define KILLED_BY	 1
#define NO_KILLER_PREFIX 2
E int NEARDATA killer_format;
E const char NEARDATA *killer;
E const char *configfile;
E char NEARDATA plname[PL_NSIZ];
E char NEARDATA dogname[];
E char NEARDATA catname[];
E char preferred_pet;
E const char NEARDATA *occtxt;		/* defined when occupation != NULL */
E const char NEARDATA *nomovemsg;
E const char NEARDATA nul[];
E const char *traps[];
E char lock[];

E const char NEARDATA sdir[], NEARDATA ndir[];
E const schar NEARDATA xdir[], NEARDATA ydir[], zdir[];

E schar NEARDATA tbx, NEARDATA tby;		/* set in mthrowu.c */
E int NEARDATA dig_effort;	/* apply.c, hack.c */
E d_level NEARDATA dig_level;
E coord NEARDATA dig_pos;
E boolean NEARDATA dig_down;

E long NEARDATA moves, NEARDATA monstermoves;
E long NEARDATA wailmsg;

E boolean NEARDATA in_mklev;
E boolean NEARDATA stoned;
E boolean NEARDATA unweapon;
E boolean NEARDATA mrg_to_wielded;

#ifdef KOPS
E boolean NEARDATA allow_kops;
#endif

E const int shield_static[];

#ifndef SPELLS_H
#include "spell.h"
#endif
E struct spell NEARDATA spl_book[];	/* sized in decl.c */

#ifdef TEXTCOLOR
# ifndef COLOR_H
#include "color.h"
# endif
E const int zapcolors[];
#endif

E const char def_oc_syms[MAXOCLASSES];	/* default class symbols */
E uchar oc_syms[MAXOCLASSES];		/* current class symbols */
E const char def_monsyms[MAXMCLASSES];	/* default class symbols */
E uchar monsyms[MAXMCLASSES];		/* current class symbols */

#ifndef OBJ_H
#include "obj.h"
#endif

E struct obj NEARDATA *invent, NEARDATA *uarm, NEARDATA *uarmc,
	NEARDATA *uarmh, NEARDATA *uarms, NEARDATA *uarmg, NEARDATA *uarmf,
#ifdef TOURIST
	NEARDATA *uarmu, /* under-wear, so to speak */
#endif
#ifdef POLYSELF
	NEARDATA *uskin,
#endif
	NEARDATA *uamul, NEARDATA *uleft, NEARDATA *uright, NEARDATA *ublindf,
	NEARDATA *uwep;

E struct obj NEARDATA *uchain;	/* defined only when punished */
E struct obj NEARDATA *uball;
E struct obj NEARDATA *migrating_objs;
E struct obj NEARDATA *billobjs;
E struct obj NEARDATA zeroobj;		/* init'd and defined in decl.c */

#ifndef YOU_H
#include "you.h"
#endif

E struct you NEARDATA u;

#ifndef ONAMES_H
#include "onames.h"
#endif
#ifndef PM_H
#include "pm.h"
#endif

E struct permonst NEARDATA playermon, NEARDATA *uasmon;
					/* also decl'd extern in permonst.h */
					/* init'd in monst.c */

E struct monst NEARDATA youmonst;	/* init'd and defined in decl.c */
E struct monst NEARDATA *mydogs, NEARDATA *migrating_mons;

E struct c_color_names {
    char const	*const c_black, *const c_amber, *const c_golden,
		*const c_light_blue,*const c_red, *const c_green,
		*const c_silver, *const c_blue, *const c_purple,
		*const c_white;
} NEARDATA c_color_names;
#define Black		c_color_names.c_black
#define amber		c_color_names.c_amber
#define golden		c_color_names.c_golden
#define light_blue	c_color_names.c_light_blue
#define red		c_color_names.c_red
#define green		c_color_names.c_green
#define silver		c_color_names.c_silver
#define blue		c_color_names.c_blue
#define purple		c_color_names.c_purple
#define White		c_color_names.c_white

E struct c_common_strings {
    char const	*const c_nothing_happens, *const c_thats_enough_tries,
		*const c_silly_thing_to, *const c_shudder_for_moment;
} c_common_strings;
#define nothing_happens    c_common_strings.c_nothing_happens
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to	   c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment

/* Vision */
E boolean NEARDATA vision_full_recalc;	/* TRUE if need vision recalc */
E char NEARDATA **viz_array;	/* could see/in sight row pointers */

/* Window system stuff */
E winid NEARDATA WIN_MESSAGE, NEARDATA WIN_STATUS;
E winid NEARDATA WIN_MAP, NEARDATA WIN_INVEN;
E char toplines[];
#ifndef TERMCAP_H
E struct tc_gbl_data {	/* also declared in termcap.h */
    char *tc_AS, *tc_AE;	/* graphics start and end (tty font swapping) */
    int   tc_LI,  tc_CO;	/* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

/* xxxexplain[] is in drawing.c */
E const char *monexplain[], *objexplain[], *oclass_names[];

E const char NEARDATA *pl_classes;

#undef E

#endif /* DECL_H */
