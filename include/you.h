/*	SCCS Id: @(#)you.h	3.2	95/08/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOU_H
#define YOU_H

#ifndef ALIGN_H
#include "align.h"
#endif
#ifndef ATTRIB_H
#include "attrib.h"
#endif
#ifndef MONST_H
#include "monst.h"
#endif
#ifndef YOUPROP_H
#include "youprop.h"
#endif

/*
 *	[this stuff should be moved into a separate "weapon.h" file]
 */
#ifdef WEAPON_SKILLS

/* Weapon Skills - Stephen White */
#define P_DAGGER		0
#define P_KNIFE			1
#define P_AXE			2
#define P_PICK_AXE		3
#define P_SHORT_SWORD		4
#define P_BROAD_SWORD		5
#define P_LONG_SWORD		6
#define P_TWO_HANDED_SWORD	7
#define P_SCIMITAR		8
#define P_SABER			9
#define P_CLUB			10
#define P_MACE			11
#define P_MORNING_STAR		12
#define P_FLAIL			13
#define P_HAMMER		14
#define P_QUARTERSTAFF		15 
#define P_POLEARMS		16
#define P_SPEAR			17
#define P_JAVELIN		18
#define P_TRIDENT		19
#define P_LANCE			20
#define P_BOW			21
#define P_SLING			22
#define P_CROSSBOW		23
#define P_DART			24
#define P_SHURIKEN		25
#define P_BOOMERANG		26
#define P_WHIP			27
#define P_UNICORN_HORN		28	/* last weapon */
#define P_TWO_WEAPON_COMBAT	29
#define P_BARE_HANDED_COMBAT	30
#define P_MARTIAL_ARTS		31	/* currently unused */
#define P_NUM_SKILLS		32	/* should always be the last entry */

#define P_NO_TYPE		P_NUM_SKILLS
#define P_LAST_WEAPON		P_UNICORN_HORN

/*
 * These are the standard weapon skill levels.  It is important that
 * the lowest "valid" skill be be 1.  The code calculates the
 * previous amount to practice by calling  practice_needed_to_advance()
 * with the current skill-1.  To work out for the UNSKILLED case,
 * a value of 0 needed.
 */
#define P_ISRESTRICTED		0
#define P_UNSKILLED		1
#define P_BASIC			2
#define P_SKILLED		3
#define P_EXPERT		4

#define practice_needed_to_advance(level) ((level)*(level)*20)

/* The hero's skill in various weapons. */
struct skills {
	xchar skill;
	xchar max_skill;
	unsigned short advance;
};

#define P_SKILL(type)		(u.weapon_skills[type].skill)
#define P_MAX_SKILL(type)	(u.weapon_skills[type].max_skill)
#define P_ADVANCE(type)		(u.weapon_skills[type].advance)
#define P_RESTRICTED(type)	(u.weapon_skills[type].skill == P_ISRESTRICTED)

#define PN_POLEARMS		(NUM_OBJECTS+0)
#define PN_TWO_WEAPON_COMBAT	(NUM_OBJECTS+1)
#define PN_BARE_HANDED_COMBAT	(NUM_OBJECTS+2)
#define PN_MARTIAL_ARTS		(NUM_OBJECTS+3)
#define PN_SABER		(NUM_OBJECTS+4)

#define P_SKILL_LIMIT 60	/* max number of skill advancements */

/* initial skill matrix structure; used in u_init.c and weapon.c */
struct def_skill {
	xchar skill;
	xchar skmax;
};

#endif /* WEAPON_SKILLS */


struct prop {

#define TIMEOUT		007777	/* mask */

#define LEFT_RING	W_RINGL	/* 010000L */
#define RIGHT_RING	W_RINGR	/* 020000L */
#define LEFT_SIDE	LEFT_RING
#define RIGHT_SIDE	RIGHT_RING
#define BOTH_SIDES	(LEFT_SIDE | RIGHT_SIDE)

#define WORN_ARMOR	W_ARM	/* 040000L */
#define WORN_CLOAK	W_ARMC	/* 0100000L */
#define WORN_HELMET	W_ARMH	/* 0200000L */
#define WORN_SHIELD	W_ARMS	/* 0400000L */
#define WORN_GLOVES	W_ARMG	/* 01000000L */
#define WORN_BOOTS	W_ARMF	/* 02000000L */
#define WORN_AMUL	W_AMUL	/* 04000000L */
#define WORN_BLINDF	W_TOOL	/* 010000000L */
#ifdef TOURIST
#define WORN_SHIRT	W_ARMU	/* 01000L */
#endif

/*
 * FROMEXPER is for a property gained by virtue of your experience level,
 * which will be lost if you lose that level; FROMOUTSIDE is one that is
 * gained in some other way (e.g., a throne, a prayer, or a corpse).
 * INTRINSIC is either FROMEXPER or FROMOUTSIDE
 */
#define FROMOUTSIDE	0200000000L
#define FROMEXPER	0400000000L
#define INTRINSIC	(FROMOUTSIDE|FROMEXPER)

/*
 * Sometimes an intrinsic may be overridden or controllable.
 */
#define I_BLOCKED	01000000000L
#define I_SPECIAL	02000000000L

	long p_flgs;
};

struct u_have {
	Bitfield(amulet,1);	/* carrying Amulet	*/
	Bitfield(bell,1);	/* carrying Bell	*/
	Bitfield(book,1);	/* carrying Book	*/
	Bitfield(menorah,1);	/* carrying Candelabrum */
	Bitfield(questart,1);	/* carrying the Quest Artifact */
	Bitfield(unused,3);
};

struct u_event {
	Bitfield(minor_oracle,1);	/* received at least 1 cheap oracle */
	Bitfield(major_oracle,1);	/*  "  expensive oracle */
	Bitfield(qcalled,1);		/* called by Quest leader to do task */
	Bitfield(qexpelled,1);		/* expelled from the Quest dungeon */
	Bitfield(qcompleted,1);		/* successfully completed Quest task */
	Bitfield(uheard_tune,2);	/* 1=know about, 2=heard passtune */
	Bitfield(uopened_dbridge,1);	/* opened the drawbridge */

	Bitfield(invoked,1);		/* invoked Gate to the Sanctum level */
	Bitfield(gehennom_entered,1);	/* entered Gehennom via Valley */
#ifdef ELBERETH
	Bitfield(uhand_of_elbereth,2);	/* became Hand of Elbereth */
#endif
	Bitfield(udemigod,1);		/* killed the wiz */
	Bitfield(ascended,1);		/* has offered the Amulet */
};


struct you {
	xchar ux, uy;
	schar dx, dy, dz;	/* direction of move (or zap or ... ) */
	schar di;		/* direction of FF */
	xchar ux0, uy0;		/* initial position FF */
	d_level uz, uz0;	/* your level on this and the previous turn */
	d_level utolev;		/* level monster teleported you to, or uz */
	uchar utotype;		/* bitmask of goto_level() flags for utolev */
	char role;		/* 'A'==archeologist, 'K'==knight, &c */
	char usym;		/* usually '@' */
	boolean umoved;		/* changed map location (post-move) */
	int last_str_turn;	/* 0: none, 1: half turn, 2: full turn */
				/* +: turn right, -: turn left */
	int ulevel;		/* 1 - MAXULEV */
	unsigned utrap;		/* trap timeout */
	unsigned utraptype;	/* defined if utrap nonzero */
#define TT_BEARTRAP	0
#define TT_PIT		1
#define TT_WEB		2
#define TT_LAVA		3
#define TT_INFLOOR	4
	char	urooms[5];	/* rooms (roomno + 3) occupied now */
	char	urooms0[5];	/* ditto, for previous position */
	char	uentered[5];	/* rooms (roomno + 3) entered this turn */
	char	ushops[5];	/* shop rooms (roomno + 3) occupied now */
	char	ushops0[5];	/* ditto, for previous position */
	char	ushops_entered[5]; /* ditto, shops entered this turn */
	char	ushops_left[5];	/* ditto, shops exited this turn */

	int	 uhunger;	/* refd only in eat.c and shk.c */
	unsigned uhs;		/* hunger state - see eat.c */

	struct prop uprops[LAST_PROP+1];

	unsigned umconf;
	char usick_cause[PL_PSIZ+20]; /* sizeof "unicorn horn named "+1 */
	Bitfield(usick_type,2);
#define SICK_VOMITABLE 0x01
#define SICK_NONVOMITABLE 0x02
#define SICK_ALL 0x03

/* For messages referring to hands, eyes, feet, etc... when polymorphed */
#define ARM 0
#define EYE 1
#define FACE 2
#define FINGER 3
#define FINGERTIP 4
#define FOOT 5
#define HAND 6
#define HANDED 7
#define HEAD 8
#define LEG 9
#define LIGHT_HEADED 10
#define NECK 11
#define SPINE 12
#define TOE 13
#define HAIR 14

	/* These ranges can never be more than MAX_RANGE (vision.h). */
	int nv_range;			/* current night vision range */
	int xray_range;			/* current xray vision range */

	/*
	 * These variables are valid globally only when punished and blind.
	 */
#define BC_BALL  0x01	/* bit mask for ball  in 'bc_felt' below */
#define BC_CHAIN 0x02	/* bit mask for chain in 'bc_felt' below */
	int bglyph;	/* glyph under the ball */
	int cglyph;	/* glyph under the chain */
	int bc_order;	/* ball & chain order [see bc_order() in ball.c] */
	int bc_felt;	/* mask for ball/chain being felt */


	/*
	 * Player type monster (e.g. PM_VALKYRIE).  This is set in u_init
	 * and never changed afterward.
	 */
	int umonster;
	int umonnum;				/* monster number or -1 */
	int mh, mhmax, mtimedone;		/* for polymorph-self */
	struct attribs	macurr,			/* for monster attribs */
			mamax;			/* for monster attribs */
	int ulycn;				/* lycanthrope type */

	unsigned ucreamed;
	unsigned uswldtim;		/* time you have been swallowed */

	Bitfield (uswallow,1);		/* true if swallowed */
	Bitfield(uinwater,1);		/* if you're currently in water (only
					   underwater possible currently) */
	Bitfield(uundetected,1);	/* if you're a hiding monster/piercer */
	Bitfield(mfemale,1);		/* saved human value of flags.female */
	Bitfield(uinvulnerable,1);	/* you're invulnerable (praying) */
	Bitfield(uburied,1);		/* you're buried */
	/* 2 free bits! */

	unsigned udg_cnt;		/* how long you have been demigod */
	struct u_event	uevent;		/* certain events have happened */
	struct u_have	uhave;		/* you're carrying special objects */
	struct attribs	acurr,		/* your current attributes (eg. str)*/
			aexe,		/* for gain/loss via "exercise" */
			abon,		/* your bonus attributes (eg. str) */
			amax,		/* your max attributes (eg. str) */
			atemp,		/* used for temporary loss/gain */
			atime;		/* used for loss/gain countdown */
	align	ualign;			/* character alignment */
#define CONVERT		2
	aligntyp ualignbase[CONVERT];	/* for ualign conversion record */
	schar uluck, moreluck;		/* luck and luck bonus */
#define LUCKADD		3	/* added value when carrying luck stone */
#define Luck	(u.uluck + u.moreluck)
#define LUCKMAX		10	/* on moonlit nights 11 */
#define LUCKMIN		(-10)
	schar	udaminc;
	schar	uac;
	int	uhp,uhpmax;
	int	uen, uenmax;		/* magical energy - M. Stephenson */
	int ugangr;			/* if the gods are angry at you */
	int ublessed, ublesscnt;	/* blessing/duration from #pray */
	long	ugold, ugold0;
	long	uexp, urexp;
	long	ucleansed;	/* to record moves when player was cleansed */
	long	usleep;		/* sleeping; monstermove you last started */
	int uinvault;
	struct monst *ustuck;
	int	umortality;		/* how many times you died */
	int ugrave_arise; /* you die and become something aside from a ghost */
	time_t	ubirthday;		/* real world time when game began */

#ifdef WEAPON_SKILLS
	int	weapon_slots;		/* unused skill slots */
	int	skills_advanced;		/* # of advances made so far */
	xchar	skill_record[P_SKILL_LIMIT];	/* skill advancements */
	struct skills weapon_skills[P_NUM_SKILLS];
#endif /* WEAPON_SKILLS */

};	/* end of `struct you' */

#define Role_is(X) (u.role == X)
#define human_role() (!Role_is('E'))
#define Upolyd (u.mtimedone != 0)

#endif	/* YOU_H */
