/*	SCCS Id: @(#)youprop.h	3.1	92/11/01	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOUPROP_H
#define YOUPROP_H

#ifndef PROP_H
#include "prop.h"
#endif
#ifndef PERMONST_H
#include "permonst.h"
#endif
#ifndef MONDATA_H
#include "mondata.h"
#endif
#ifndef PM_H
#include "pm.h"
#endif

#ifdef POLYSELF
# define UPROP(base,poly) (base || (poly))
#else
# define UPROP(base,poly) (base)
#endif

/* three pseudo-properties */
#define Blindfolded	(ublindf)
#define Punished	(uball)
#define Underwater	(u.uinwater)
/* Note that Underwater and u.uinwater are both used in code.
   The latter form is for later implementation of other in-water
   states, like swimming, wading, etc. */

#define HFire_resistance	u.uprops[FIRE_RES].p_flgs
#define Fire_resistance		UPROP(HFire_resistance,resists_fire(uasmon))

#define HCold_resistance	u.uprops[COLD_RES].p_flgs
#define Cold_resistance		UPROP(HCold_resistance,resists_cold(uasmon))

#define HSleep_resistance	u.uprops[SLEEP_RES].p_flgs
#define Sleep_resistance	UPROP(HSleep_resistance,resists_sleep(uasmon))

#define HDisint_resistance	u.uprops[DISINT_RES].p_flgs
#define Disint_resistance	UPROP(HDisint_resistance,resists_disint(uasmon))

#define HShock_resistance	u.uprops[SHOCK_RES].p_flgs
#define Shock_resistance	UPROP(HShock_resistance,resists_elec(uasmon))

#define HPoison_resistance	u.uprops[POISON_RES].p_flgs
#define Poison_resistance	UPROP(HPoison_resistance,resists_poison(uasmon))

#define Adornment		u.uprops[ADORNED].p_flgs

#define HRegeneration		u.uprops[REGENERATION].p_flgs
#define Regeneration		UPROP(HRegeneration,regenerates(uasmon))

#define Searching		u.uprops[SEARCHING].p_flgs

#define HSee_invisible		u.uprops[SEE_INVIS].p_flgs
#define See_invisible		UPROP(HSee_invisible,perceives(uasmon))

#define HInvis			u.uprops[INVIS].p_flgs
#define Invis			UPROP(HInvis,u.usym == S_STALKER)
#define Invisible		(Invis && !See_invisible)

#define HTeleportation		u.uprops[TELEPORT].p_flgs
#define Teleportation		UPROP(HTeleportation,can_teleport(uasmon))

#define HTeleport_control	u.uprops[TELEPORT_CONTROL].p_flgs
#define Teleport_control	UPROP(HTeleport_control,control_teleport(uasmon))

#ifdef POLYSELF
#define Polymorph		u.uprops[POLYMORPH].p_flgs
#define Polymorph_control	u.uprops[POLYMORPH_CONTROL].p_flgs
#endif

#define HLevitation		u.uprops[LEVITATION].p_flgs
#define Levitation		UPROP(HLevitation,is_floater(uasmon))

#define Stealth			u.uprops[STEALTH].p_flgs
#define Aggravate_monster	u.uprops[AGGRAVATE_MONSTER].p_flgs
#define Conflict		u.uprops[CONFLICT].p_flgs
#define Protection		u.uprops[PROTECTION].p_flgs
#define Protection_from_shape_changers \
				u.uprops[PROT_FROM_SHAPE_CHANGERS].p_flgs
#define Warning			u.uprops[WARNING].p_flgs

#define HTelepat		u.uprops[TELEPAT].p_flgs
#define Telepat			UPROP(HTelepat,telepathic(uasmon))

#define Fast			u.uprops[FAST].p_flgs

#define HStun			u.uprops[STUNNED].p_flgs
#define Stunned			UPROP(HStun,u.usym==S_BAT || u.usym==S_STALKER)

#define HConfusion		u.uprops[CONFUSION].p_flgs
#define Confusion		HConfusion

#define Sick			u.uprops[SICK].p_flgs
#define Blinded			u.uprops[BLINDED].p_flgs
#define Blind			UPROP(Blinded || Blindfolded,!haseyes(uasmon))
#define Sleeping		u.uprops[SLEEPING].p_flgs
#define Wounded_legs		u.uprops[WOUNDED_LEGS].p_flgs
#define Stoned			u.uprops[STONED].p_flgs
#define Strangled		u.uprops[STRANGLED].p_flgs
#define HHallucination		u.uprops[HALLUC].p_flgs
#define HHalluc_resistance	u.uprops[HALLUC_RES].p_flgs
#define Hallucination		(HHallucination && !HHalluc_resistance)
#define Fumbling		u.uprops[FUMBLING].p_flgs
#define Jumping			u.uprops[JUMPING].p_flgs
/* Wwalking is meaningless on water level */
#define Wwalking		(u.uprops[WWALKING].p_flgs && \
				 !Is_waterlevel(&u.uz))
#define Hunger			u.uprops[HUNGER].p_flgs
#define Glib			u.uprops[GLIB].p_flgs
#define Reflecting		u.uprops[REFLECTING].p_flgs
#define Lifesaved		u.uprops[LIFESAVED].p_flgs
#define Antimagic		u.uprops[ANTIMAGIC].p_flgs
#define Displaced		u.uprops[DISPLACED].p_flgs
#define Clairvoyant		u.uprops[CLAIRVOYANT].p_flgs
#define Vomiting		u.uprops[VOMITING].p_flgs
#define Energy_regeneration	u.uprops[ENERGY_REGENERATION].p_flgs
#define HMagical_breathing	u.uprops[MAGICAL_BREATHING].p_flgs
#define Magical_breathing	UPROP(HMagical_breathing,\
				      magic_breathing(uasmon))
#define Half_spell_damage	u.uprops[HALF_SPDAM].p_flgs
#define Half_physical_damage	u.uprops[HALF_PHDAM].p_flgs

#endif /* YOUPROP_H */
