/*	SCCS Id: @(#)decl.h	3.2	95/08/13	*/
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

E NEARDATA int bases[MAXOCLASSES];

E NEARDATA int multi;
E NEARDATA int warnlevel;
E NEARDATA int nroom;
E NEARDATA int nsubroom;
E NEARDATA int occtime;

E int x_maze_max, y_maze_max;
E int otg_temp;

#ifdef REDO
E NEARDATA int in_doagain;
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
    xchar	d_mines_dnum, d_quest_dnum;
    d_level	d_qstart_level, d_qlocate_level, d_nemesis_level;
    d_level	d_knox_level;
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
#define mines_dnum		(dungeon_topology.d_mines_dnum)
#define quest_dnum		(dungeon_topology.d_quest_dnum)
#define qstart_level		(dungeon_topology.d_qstart_level)
#define qlocate_level		(dungeon_topology.d_qlocate_level)
#define nemesis_level		(dungeon_topology.d_nemesis_level)
#define knox_level		(dungeon_topology.d_knox_level)

E NEARDATA stairway dnstair, upstair; /* stairs up and down. */
#define xdnstair	(dnstair.sx)
#define ydnstair	(dnstair.sy)
#define xupstair	(upstair.sx)
#define yupstair	(upstair.sy)

E NEARDATA stairway dnladder, upladder; /* ladders up and down. */
#define xdnladder	(dnladder.sx)
#define ydnladder	(dnladder.sy)
#define xupladder	(upladder.sx)
#define yupladder	(upladder.sy)

E NEARDATA stairway sstairs;

E NEARDATA dest_area updest, dndest; /* level-change dest. areas */

E NEARDATA coord inv_pos;
E NEARDATA dungeon dungeons[];
E NEARDATA s_level *sp_levchn;
#define dunlev_reached(x)	(dungeons[(x)->dnum].dunlev_ureached)

#include "quest.h"
E struct q_score		quest_status;

E NEARDATA char pl_character[PL_CSIZ];

E NEARDATA char pl_fruit[PL_FSIZ];
E NEARDATA int current_fruit;
E NEARDATA struct fruit *ffruit;

E NEARDATA char tune[6];

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
E struct linfo level_info[MAXLINFO];

E NEARDATA struct sinfo {
	int stopprint;		/* game over, inhibit further disclosure */
#if defined(UNIX) || defined(VMS)
	int done_hup;		/* SIGHUP or moral equivalent received
				 * -- no more screen output */
#endif
	int something_worth_saving;	/* in case of panic */
	int panicking;		/* `panic' is in progress */
#ifdef VMS
	int exiting;		/* an exit handler is executing */
#endif
} program_state;

E const char quitchars[];
E const char vowels[];
E const char ynchars[];
E const char ynqchars[];
E const char ynaqchars[];
E const char ynNaqchars[];
E NEARDATA long yn_number;
E NEARDATA int smeq[];
E NEARDATA int doorindex;
E NEARDATA char *save_cm;
#define KILLED_BY_AN	 0
#define KILLED_BY	 1
#define NO_KILLER_PREFIX 2
E NEARDATA int killer_format;
E const char *killer;
E const char *configfile;
E NEARDATA char plname[PL_NSIZ];
E NEARDATA char dogname[];
E NEARDATA char catname[];
E char preferred_pet;
E const char *occtxt;		/* defined when occupation != NULL */
E const char *nomovemsg;
E const char nul[];
E char lock[];

E const char sdir[], ndir[];
E const schar xdir[], ydir[], zdir[];

E NEARDATA schar tbx, tby;		/* set in mthrowu.c */

E NEARDATA struct dig_info {	/* apply.c, hack.c */
	int	effort;
	d_level	level;
	coord	pos;
	boolean	down, chew;
} digging;

E NEARDATA long moves, monstermoves;
E NEARDATA long wailmsg;

E NEARDATA boolean in_mklev;
E NEARDATA boolean stoned;
E NEARDATA boolean unweapon;
E NEARDATA boolean mrg_to_wielded;
E NEARDATA struct obj *current_wand;

E const int shield_static[];

#ifndef SPELLS_H
#include "spell.h"
#endif
E NEARDATA struct spell spl_book[];	/* sized in decl.c */

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

E NEARDATA struct obj *invent, *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
#ifdef TOURIST
	*uarmu, /* under-wear, so to speak */
#endif
	*uskin, *uamul, *uleft, *uright, *ublindf, *uwep;

E NEARDATA struct obj *uchain;	/* defined only when punished */
E NEARDATA struct obj *uball;
E NEARDATA struct obj *migrating_objs;
E NEARDATA struct obj *billobjs;
E NEARDATA struct obj zeroobj;		/* init'd and defined in decl.c */

E const char *he[3];
E const char *him[3];
E const char *his[3];

#ifndef YOU_H
#include "you.h"
#endif

E NEARDATA struct you u;

#ifndef ONAMES_H
#include "onames.h"
#endif
#ifndef PM_H
#include "pm.h"
#endif

E NEARDATA struct permonst playermon, *uasmon;
					/* also decl'd extern in permonst.h */
					/* init'd in monst.c */

E NEARDATA struct monst youmonst;	/* init'd and defined in decl.c */
E NEARDATA struct monst *mydogs, *migrating_mons;

E NEARDATA struct mvitals {
	uchar	died;
	uchar	mvflags;
} mvitals[NUMMONS];

E NEARDATA struct c_color_names {
    const char	*const c_black, *const c_amber, *const c_golden,
		*const c_light_blue,*const c_red, *const c_green,
		*const c_silver, *const c_blue, *const c_purple,
		*const c_white;
} c_color_names;
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
    const char	*const c_nothing_happens, *const c_thats_enough_tries,
		*const c_silly_thing_to, *const c_shudder_for_moment,
		*const c_something, *const c_Something, 
		*const c_You_can_move_again;
} c_common_strings;
#define nothing_happens    c_common_strings.c_nothing_happens
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to	   c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment
#define something          c_common_strings.c_something
#define Something          c_common_strings.c_Something
#define You_can_move_again c_common_strings.c_You_can_move_again

/* Vision */
E NEARDATA boolean vision_full_recalc;	/* TRUE if need vision recalc */
E NEARDATA char **viz_array;	/* could see/in sight row pointers */

/* Window system stuff */
E NEARDATA winid WIN_MESSAGE, WIN_STATUS;
E NEARDATA winid WIN_MAP, WIN_INVEN;
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

E const char *pl_classes;

#undef E

#endif /* DECL_H */
