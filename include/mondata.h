/*	SCCS Id: @(#)mondata.h	3.0	89/11/21
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright (c) 1989 Mike Threepoint */

#ifndef MONDATA_H
#define MONDATA_H

# ifndef STUPID_CPP	/* otherwise these macros are functions in mondata.c */

#define verysmall(ptr)		((ptr)->msize < MZ_SMALL)
#define bigmonst(ptr)		((ptr)->msize >= MZ_LARGE)

#define is_flyer(ptr)		(((ptr)->mflags1 & M1_FLY) != 0L)
#define is_floater(ptr) 	((ptr)->mlet == S_EYE)
#define is_swimmer(ptr) 	(((ptr)->mflags1 & M1_SWIM) != 0L)
#define passes_walls(ptr)	(((ptr)->mflags1 & M1_WALLWALK) != 0L)
#define amorphous(ptr)		(((ptr)->mflags1 & M1_AMORPHOUS) != 0L)
#define noncorporeal(ptr)	((ptr)->mlet == S_GHOST)
#define tunnels(ptr)		(((ptr)->mflags1 & M1_TUNNEL) != 0L)
#define needspick(ptr)		(((ptr)->mflags1 & M1_NEEDPICK) != 0L)
#define hides_under(ptr)	(((ptr)->mflags1 & M1_CONCEAL) != 0L)
#define is_hider(ptr)		(((ptr)->mflags1 & M1_HIDE) != 0L)
#define haseyes(ptr)		(((ptr)->mflags1 & M1_NOEYES) == 0L)
#define nohands(ptr)		(((ptr)->mflags1 & M1_NOHANDS) != 0L)
#define nolimbs(ptr)		(((ptr)->mflags1 & M1_NOLIMBS) == M1_NOLIMBS)
#  ifdef POLYSELF
#define polyok(ptr)		(((ptr)->mflags1 & M1_NOPOLY) == 0L)
#  endif
#define is_whirly(ptr)		((ptr)->mlet == S_VORTEX || (ptr) == &mons[PM_AIR_ELEMENTAL])
#define humanoid(ptr)		(((ptr)->mflags1 & M1_HUMANOID) != 0L)
#define is_animal(ptr)		(((ptr)->mflags1 & M1_ANIMAL) != 0L)
#define slithy(ptr)		(((ptr)->mflags1 & M1_SLITHY) != 0L)
#define thick_skinned(ptr)	(((ptr)->mflags1 & M1_THICK_HIDE) != 0L)
#define resists_fire(ptr)	(((ptr)->mflags1 & M1_FIRE_RES) != 0L)
#define resists_cold(ptr)	(((ptr)->mflags1 & M1_COLD_RES) != 0L)
#define resists_disint(ptr)	((ptr) == &mons[PM_BLACK_DRAGON] || (ptr) == &mons[PM_BABY_BLACK_DRAGON])
#define resists_elec(ptr)	(((ptr)->mflags1 & M1_ELEC_RES) != 0L)
#define resists_acid(ptr)	(((ptr)->mflags1 & M1_ACID) != 0L)
#define acidic(ptr)		(((ptr)->mflags1 & M1_ACID) != 0L)
#define resists_poison(ptr)	(((ptr)->mflags1 & (M1_POIS | M1_POIS_RES)) != 0L)
#define poisonous(ptr)		(((ptr)->mflags1 & M1_POIS) != 0L)
#define regenerates(ptr)	(((ptr)->mflags1 & M1_REGEN) != 0L)
#define perceives(ptr)		(((ptr)->mflags1 & M1_SEE_INVIS) != 0L)
#define can_teleport(ptr)	(((ptr)->mflags1 & M1_TPORT) != 0L)
#define control_teleport(ptr)	(((ptr)->mflags1 & M1_TPORT_CONTROL) != 0L)
#define is_armed(ptr)		attacktype(ptr, AT_WEAP)
#define likes_gold(ptr) 	(((ptr)->mflags1 & M1_GREEDY) != 0L)
#define likes_gems(ptr) 	(((ptr)->mflags1 & M1_JEWELS) != 0L)
#define likes_objs(ptr) 	(((ptr)->mflags1 & M1_COLLECT) != 0L || \
					is_armed(ptr))
#define likes_magic(ptr)	(((ptr)->mflags1 & M1_MAGIC) != 0L)
#define is_undead(ptr)		(((ptr)->mflags2 & M2_UNDEAD) != 0L)
#define resists_sleep(ptr)	(((ptr)->mflags1 & M1_SLEE_RES) != 0L || is_undead(ptr))
#define is_were(ptr)		(((ptr)->mflags2 & M2_WERE) != 0L)
#define is_elf(ptr)		(((ptr)->mflags2 & M2_ELF) != 0L)
#define is_dwarf(ptr)		(((ptr)->mflags2 & M2_DWARF) != 0L)
#define is_giant(ptr)		(((ptr)->mflags2 & M2_GIANT) != 0L)
#  ifdef GOLEMS
#define is_golem(ptr)		((ptr)->mlet == S_GOLEM)
#  endif
#define is_domestic(ptr)	(((ptr)->mflags2 & M2_DOMESTIC) != 0L)
#define is_orc(ptr)		(((ptr)->mflags2 & M2_ORC) != 0L)
#define is_human(ptr)		(((ptr)->mflags2 & M2_HUMAN) != 0L)
#define is_demon(ptr)		(((ptr)->mflags2 & M2_DEMON) != 0L)
#define is_mercenary(ptr)	(((ptr)->mflags2 & M2_MERC) != 0L)
#define is_wanderer(ptr)	(((ptr)->mflags2 & M2_WANDER) != 0L)
#define always_hostile(ptr)	(((ptr)->mflags2 & M2_HOSTILE) != 0L)
#define always_peaceful(ptr)	(((ptr)->mflags2 & M2_PEACEFUL) != 0L)
#define extra_nasty(ptr)	(((ptr)->mflags2 & M2_NASTY) != 0L)
#define strongmonst(ptr)	(((ptr)->mflags2 & M2_STRONG) != 0L)
#  ifdef POLYSELF
#define can_breathe(ptr)	attacktype(ptr, AT_BREA)
#define cantwield(ptr)		(nohands(ptr) || verysmall(ptr))
#define cantweararm(ptr)	(breakarm(ptr) || sliparm(ptr))
#  endif /* POLYSELF */
#define carnivorous(ptr)	(((ptr)->mflags2 & M2_CARNIVORE) != 0L)
#define herbivorous(ptr)	(((ptr)->mflags2 & M2_HERBIVORE) != 0L)
#define metallivorous(ptr)	(((ptr)->mflags2 & M2_METALLIVORE) != 0L)
#define lays_eggs(ptr)		(((ptr)->mflags2 & M2_EGGS) != 0L)
#define throws_rocks(ptr)	(((ptr)->mflags2 & M2_ROCKTHROW) != 0L)
#define type_is_pname(ptr)	(((ptr)->mflags2 & M2_PNAME) != 0L)
#define is_lord(ptr)		(((ptr)->mflags2 & M2_LORD) != 0L)
#define is_prince(ptr)		(((ptr)->mflags2 & M2_PRINCE) != 0L)
#  ifdef INFERNO
#define is_ndemon(ptr)		(is_demon(ptr) && \
			(((ptr)->mflags2 & (M2_LORD | M2_PRINCE)) == 0L))
#  else
#define is_ndemon(ptr)		(ptr == &mons[PM_DEMON])
#  endif
#define is_dlord(ptr)		(is_demon(ptr) && is_lord(ptr))
#define is_dprince(ptr)		(is_demon(ptr) && is_prince(ptr))

# endif /* STUPID_CPP */

#endif /* MONDATA_H */
