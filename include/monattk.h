/*	SCCS Id: @(#)monattk.h	3.0	89/06/15
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright 1988, M. Stephenson */

#ifndef MONATTK_H
#define MONATTK_H

/*	Add new attack types below - ordering affects experience (exper.c).
 *	Attacks > AT_BUTT are worth extra experience.
 */
#define	AT_NONE		0		/* passive monster (ex. acid blob) */
#define	AT_CLAW		1		/* claw (punch, hit, etc.) */
#define	AT_BITE		2		/* bite */
#define	AT_KICK		3		/* kick */
#define	AT_BUTT		4		/* head butt (ex. a unicorn) */
#define	AT_TUCH		5		/* touches */
#define	AT_STNG		6		/* sting */
#define AT_HUGS		7		/* crushing bearhug */
#define	AT_SPIT		10		/* spits substance - ranged */
#define	AT_ENGL		11		/* engulf (swallow or by a cloud) */
#define	AT_BREA		12		/* breath - ranged */
#define	AT_EXPL		13		/* explodes - proximity */
#define	AT_GAZE		14		/* gaze - ranged */

#define	AT_WEAP		254		/* uses weapon */
#define	AT_MAGC		255		/* uses magic spell(s) */

/*	Add new damage types below.
 *
 *	Note that 1-10 correspond to the types of attack used in buzz().
 *	Please don't disturb the order unless you rewrite the buzz() code.
 */
#define	AD_PHYS		0		/* ordinary physical */
#define	AD_MAGM		1		/* magic missiles */
#define	AD_FIRE		2		/* fire damage */
#define	AD_SLEE		3		/* sleep ray */
#define	AD_COLD		4		/* frost damage */
#define	AD_DISN		5		/* disintegration (death ray) */
#define	AD_ELEC		6		/* shock damage */
#define	AD_DRST		7		/* drains str (poison) */
#define	AD_ACID		8		/* acid damage */
#define	AD_SPC1		9		/* for extension of buzz() */
#define	AD_SPC2		10		/* for extension of buzz() */
#define	AD_BLND		11		/* blinds (glowing eye) */
#define	AD_STUN		12		/* stuns */
#define	AD_SLOW		13		/* slows */
#define	AD_PLYS		14		/* paralyses */
#define	AD_DRLI		15		/* drains life levels (Vampire) */
#define	AD_DREN		16		/* drains magic energy */
#define	AD_LEGS		17		/* damages legs (xan) */
#define	AD_STON		18		/* petrifies (Medusa, Cockatrice) */
#define	AD_STCK		19		/* sticks to you (Mimic) */
#define	AD_SGLD		20		/* steals gold (Leppie) */
#define	AD_SITM		21		/* steals item (Nymphs) */
#define AD_SEDU		22		/* seduces & steals multiple items */
#define	AD_TLPT		23		/* teleports you (Quantum Mech.) */
#define	AD_RUST		24		/* rusts armour (Rust Monster)*/
#define	AD_CONF		25		/* confuses (Umber Hulk) */
#define AD_DGST		26		/* digests opponent (trapper, etc.) */
#define AD_HEAL		27		/* heals opponent's wounds (nurse) */
#define AD_WRAP		28		/* special "stick" for eels */
#define	AD_WERE		29		/* confers lycanthropy */
#define AD_DRDX		30		/* drains dexterity (Quasit) */
#define AD_DRCO		31		/* drains constitution */
#define AD_DISE		32		/* confers diseases */
#define AD_DCAY		33		/* decays organics (Brown pudding) */
#define AD_SSEX		34		/* Succubus seduction (extended) */
					/* If no SEDUCE then same as AD_SEDU */

#define	AD_CLRC		240		/* random clerical spell */
#define	AD_SPEL		241		/* random magic spell */

#define AD_SAMU		252		/* hits, may steal Amulet (Wizard) */
#define AD_CURS		253		/* random curse (ex. gremlin) */
#define AD_CUSS		255		/* says nasty things about you */

#endif /* MONATTK_H /**/
