/*	SCCS Id: @(#)mondata.c	3.1	92/11/24	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"
#include "epri.h"

/*	These routines provide basic data for any type of monster. */

#ifdef OVL0

boolean
attacktype(ptr, atyp)
	register struct	permonst	*ptr;
	register int atyp;
{
	int	i;

	for(i = 0; i < NATTK; i++)
	    if(ptr->mattk[i].aatyp == atyp) return(TRUE);

	return(FALSE);
}

#endif /* OVL0 */
#ifdef OVLB

boolean
poly_when_stoned(ptr)
    struct permonst *ptr;
{
    return (is_golem(ptr) && ptr != &mons[PM_STONE_GOLEM] &&
	    !(mons[PM_STONE_GOLEM].geno & G_GENOD));	/* allow G_EXTINCT */
}

boolean
resists_drli(ptr)	/* returns TRUE if monster is drain-life resistant */

	register struct permonst *ptr;
{
	return(is_undead(ptr) || is_demon(ptr) || is_were(ptr));
}

#endif /* OVLB */
#ifdef OVL0

boolean
ranged_attk(ptr)	/* returns TRUE if monster can attack at range */
	register struct permonst *ptr;
{
	register int	i, j;
	register int atk_mask = (1<<AT_BREA) | (1<<AT_SPIT) | (1<<AT_GAZE);

	/* was: (attacktype(ptr, AT_BREA) || attacktype(ptr, AT_WEAP) ||
		attacktype(ptr, AT_SPIT) || attacktype(ptr, AT_GAZE) ||
		attacktype(ptr, AT_MAGC));
	   but that's too slow -dlc
	 */
	for(i = 0; i < NATTK; i++) {
	    if((j=ptr->mattk[i].aatyp) >= AT_WEAP || (atk_mask & (1<<j)))
		return TRUE;
	}

	return(FALSE);
}

boolean
hates_silver(ptr)
register struct permonst *ptr;
/* returns TRUE if monster is especially affected by silver weapons */
{
	return (is_were(ptr) || ptr->mlet==S_VAMPIRE || is_demon(ptr) ||
		ptr == &mons[PM_SHADE] ||
		(ptr->mlet==S_IMP && ptr != &mons[PM_TENGU]));
}

#endif /* OVL0 */
#ifdef OVL1

boolean
can_track(ptr)		/* returns TRUE if monster can track well */
	register struct permonst *ptr;
{
	if (uwep && uwep->oartifact == ART_EXCALIBUR)
		return TRUE;
	else
		return(haseyes(ptr));
}

#endif /* OVL1 */
#ifdef OVLB

#if defined(POLYSELF) || defined(MUSE)
boolean
sliparm(ptr)	/* creature will slide out of armor */
	register struct permonst *ptr;
{
	return is_whirly(ptr) || ptr->msize <= MZ_SMALL ||
		ptr == &mons[PM_GHOST];
}

boolean
breakarm(ptr)	/* creature will break out of armor */
	register struct permonst *ptr;
{
	return((bigmonst(ptr) || (ptr->msize > MZ_SMALL && !humanoid(ptr))
	                || ptr == &mons[PM_MARILITH]) && !sliparm(ptr));
	/* Marilith is about the only case of a monster which is otherwise
	 * humanoid but cannot wear armor (too many arms).  Centaurs would
	 * be another except that they are already accounted for by
	 * bigmonst.
	 */
}
#endif
#endif /* OVLB */
#ifdef OVL1

boolean
sticks(ptr)	/* creature sticks other creatures it hits */
	register struct permonst *ptr;
{
	return(dmgtype(ptr,AD_STCK) || dmgtype(ptr,AD_WRAP) ||
		attacktype(ptr,AT_HUGS));
}

boolean
dmgtype(ptr, dtyp)
	register struct	permonst	*ptr;
	register int dtyp;
{
	int	i;

	for(i = 0; i < NATTK; i++)
	    if(ptr->mattk[i].adtyp == dtyp) return TRUE;

	return FALSE;
}

/* returns the maximum damage a defender can do to the attacker via
 * a passive defense */
int
max_passive_dmg(mdef, magr)
    register struct monst *mdef, *magr;
{
    int	i, dmg = 0;
    uchar adtyp;

    for(i = 0; i < NATTK; i++)
	if(mdef->data->mattk[i].aatyp == AT_NONE) {
	    adtyp = mdef->data->mattk[i].adtyp;
	    if((adtyp == AD_ACID && !resists_acid(magr->data)) ||
		    (adtyp == AD_COLD && !resists_cold(magr->data)) ||
		    (adtyp == AD_FIRE && !resists_fire(magr->data)) ||
		    (adtyp == AD_ELEC && !resists_elec(magr->data))) {
		dmg = mdef->data->mattk[i].damn;
		if(!dmg) dmg = mdef->data->mlevel+1;
		dmg *= mdef->data->mattk[i].damd;
	    } else dmg = 0;

	    return dmg;
	}
    return 0;
}

#endif /* OVL1 */
#ifdef OVL0

int
monsndx(ptr)		/* return an index into the mons array */
	struct	permonst	*ptr;
{
	register int	i;

	if(ptr == &playermon) return(-1);

	i = (int)(ptr - &mons[0]);
	if(i < 0 || i >= NUMMONS) {    
	    panic("monsndx - could not index monster (%lx)", (long)ptr);
	    return FALSE;		/* will not get here */
	}

	return(i);
}

#endif /* OVL0 */
#ifdef OVL1


int
name_to_mon(str)
char *str;
{
	/* Be careful.  We must check the entire string in case it was
	 * something such as "ettin zombie corpse".  The calling routine
	 * doesn't know about the "corpse" until the monster name has
	 * already been taken off the front, so we have to be able to
	 * read the name with extraneous stuff such as "corpse" stuck on
	 * the end.
	 * This causes a problem for names which prefix other names such
	 * as "ettin" on "ettin zombie".  In this case we want the _longest_
	 * name which exists.
	 * This also permits plurals created by adding suffixes such as 's'
	 * or 'es'.  Other plurals must still be handled explicitly.
	 */
	register int i;
	register int mntmp = -1;
	register char *s;
	char buf[BUFSZ];
	int len, slen;

	Strcpy(buf, str);
	str = buf;
	if (!strncmp(str, "a ", 2)) str += 2;
	else if (!strncmp(str, "an ", 3)) str += 3;

	/* Some irregular plurals */
	if (!strncmpi(str, "incubi", 6)) return PM_INCUBUS;
	if (!strncmpi(str, "succubi", 7)) return PM_SUCCUBUS;
	if (!strncmpi(str, "violet fungi", 12)) return PM_VIOLET_FUNGUS;
	if (!strncmpi(str, "homunculi", 9)) return PM_HOMUNCULUS;
	if (!strncmpi(str, "baluchitheria", 13)) return PM_BALUCHITHERIUM;
	if (!strncmpi(str, "lurkers above", 13)) return PM_LURKER_ABOVE;
	if (!strncmpi(str, "cavemen", 7)) return PM_CAVEMAN;
	if (!strncmpi(str, "cavewomen", 9)) return PM_CAVEWOMAN;
	if (!strncmpi(str, "zruties", 7)) return PM_ZRUTY;
	if (!strncmpi(str, "djinn", 5)) return PM_DJINNI;
	if (!strncmpi(str, "mumakil", 7)) return PM_MUMAK;
	if ((s = strstri(str, "vortices")) != 0)
	    Strcpy(s+4, "ex");
	/* be careful with "ies"; "priest", "zombies" */
	else if ((s = strstri(str, "jellies")) != 0 ||
		 (s = strstri(str, "mummies")) != 0)
	    Strcpy(s+4, "y");
	/* luckily no monster names end in fe or ve with ves plurals */
	else if ((s = strstri(str, "ves")) != 0)
	    Strcpy(s, "f");

	slen = strlen(str);
	for (len = 0, i = 0; i < NUMMONS; i++) {
	    register int m_i_len = strlen(mons[i].mname);
	    if (m_i_len > len && !strncmpi(mons[i].mname, str, m_i_len)) {
		if (m_i_len == slen) return i;	/* exact match */
		else if (slen > m_i_len &&
			(str[m_i_len] == ' ' ||
			 !strcmpi(&str[m_i_len], "s") ||
			 !strncmpi(&str[m_i_len], "s ", 2) ||
			 !strcmpi(&str[m_i_len], "es") ||
			 !strncmpi(&str[m_i_len], "es ", 3))) {
		    mntmp = i;
		    len = m_i_len;
		}
	    }
	}
	if (mntmp == -1) mntmp = title_to_mon(str, (int *)0, (int *)0);
	return mntmp;
}

#endif /* OVL1 */
#ifdef OVLB

#ifdef POLYSELF
boolean
webmaker(ptr)   /* creature can spin a web */
	register struct permonst *ptr;
{
	return (ptr->mlet == S_SPIDER && ptr != &mons[PM_SCORPION]);
}
#endif

#endif /* OVLB */
#ifdef OVL2

/* returns 3 values (0=male, 1=female, 2=none) */
int
gender(mtmp)
register struct monst *mtmp;
{
	if (is_neuter(mtmp->data)) return 2;
	return mtmp->female;
}

/* like gender(), but lower animals and such are still "it" */
int
pronoun_gender(mtmp)
register struct monst *mtmp;
{
	if (Blind || !humanoid(mtmp->data)) return 2;
	return mtmp->female;
}

#endif /* OVL2 */
#ifdef OVLB

boolean
levl_follower(mtmp)
register struct monst *mtmp;
{
	return (mtmp->mtame || (mtmp->data->mflags2 & M2_STALK) || is_fshk(mtmp)
		|| (mtmp->iswiz && !mon_has_amulet(mtmp)));
}

struct permonst *
player_mon()
{
	switch (pl_character[0]) {
		case 'A': return &mons[PM_ARCHEOLOGIST];
		case 'B': return &mons[PM_BARBARIAN];
		case 'C': if (flags.female) return &mons[PM_CAVEWOMAN];
			else return &mons[PM_CAVEMAN];
		case 'E': return &mons[PM_ELF];
		case 'H': return &mons[PM_HEALER];
		case 'K': return &mons[PM_KNIGHT];
		case 'P': if (flags.female) return &mons[PM_PRIESTESS];
			else return &mons[PM_PRIEST];
		case 'R': return &mons[PM_ROGUE];
		case 'S': return &mons[PM_SAMURAI];
#ifdef TOURIST
		case 'T': return &mons[PM_TOURIST];
#endif
		case 'V': return &mons[PM_VALKYRIE];
		case 'W': return &mons[PM_WIZARD];
		default: impossible("what are you?");
			return &mons[PM_HUMAN];
	}
}

const int grownups[][2] = { {PM_LITTLE_DOG, PM_DOG}, {PM_DOG, PM_LARGE_DOG},
	{PM_HELL_HOUND_PUP, PM_HELL_HOUND}, {PM_KITTEN, PM_HOUSECAT},
	{PM_HOUSECAT, PM_LARGE_CAT}, {PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON},
	{PM_KOBOLD, PM_LARGE_KOBOLD}, {PM_LARGE_KOBOLD, PM_KOBOLD_LORD},
	{PM_GNOME, PM_GNOME_LORD}, {PM_GNOME_LORD, PM_GNOME_KING},
	{PM_DWARF, PM_DWARF_LORD}, {PM_DWARF_LORD, PM_DWARF_KING},
	{PM_SMALL_MIMIC, PM_LARGE_MIMIC}, {PM_LARGE_MIMIC, PM_GIANT_MIMIC},
	{PM_BAT, PM_GIANT_BAT},
	{PM_LICH, PM_DEMILICH}, {PM_DEMILICH, PM_MASTER_LICH},
	{PM_OGRE, PM_OGRE_LORD}, {PM_OGRE_LORD, PM_OGRE_KING},
	{PM_VAMPIRE, PM_VAMPIRE_LORD},
	{PM_BABY_RED_DRAGON, PM_RED_DRAGON},
	{PM_BABY_WHITE_DRAGON, PM_WHITE_DRAGON},
	{PM_BABY_BLUE_DRAGON, PM_BLUE_DRAGON},
	{PM_BABY_GREEN_DRAGON, PM_GREEN_DRAGON},
	{PM_BABY_ORANGE_DRAGON, PM_ORANGE_DRAGON},
	{PM_BABY_BLACK_DRAGON, PM_BLACK_DRAGON},
	{PM_BABY_YELLOW_DRAGON, PM_YELLOW_DRAGON},
	{PM_RED_NAGA_HATCHLING, PM_RED_NAGA},
	{PM_BLACK_NAGA_HATCHLING, PM_BLACK_NAGA},
	{PM_GOLDEN_NAGA_HATCHLING, PM_GOLDEN_NAGA},
	{PM_GUARDIAN_NAGA_HATCHLING, PM_GUARDIAN_NAGA},
	{PM_BABY_PURPLE_WORM, PM_PURPLE_WORM},
	{PM_BABY_LONG_WORM, PM_LONG_WORM},
#ifdef ARMY
	{PM_SOLDIER, PM_SERGEANT},
	{PM_SERGEANT, PM_LIEUTENANT},
	{PM_LIEUTENANT, PM_CAPTAIN},
#endif
	{PM_WATCHMAN, PM_WATCH_CAPTAIN},
	{PM_BABY_CROCODILE, PM_CROCODILE},
	{-1,-1}
};

int
little_to_big(montype)
int montype;
{
#ifndef AIXPS2_BUG
	register int i;
	
	for(i=0; grownups[i][0] >= 0; i++)
		if(montype == grownups[i][0]) return grownups[i][1];
	return montype;
#else
/* AIX PS/2 C-compiler 1.1.1 optimizer does not like the above for loop,
 * and causes segmentation faults at runtime.  (The problem does not
 * occur if -O is not used.)
 * lehtonen@cs.Helsinki.FI (Tapio Lehtonen) 28031990
 */
	int i;
	int monvalue;

	monvalue = montype;
	for(i=0; grownups[i][0] >= 0; i++)
		if(montype == grownups[i][0]) monvalue = grownups[i][1];
	
	return monvalue;
#endif
}

int
big_to_little(montype)
int montype;
{
	register int i;
	
	for(i=0; grownups[i][0] >= 0; i++)
		if(montype == grownups[i][1]) return grownups[i][0];
	return montype;
}


const char *
locomotion(ptr, def)
const struct permonst *ptr;
const char *def;
{
	return (
		is_floater(ptr) ? (const char *)"float" :
		is_flyer(ptr)   ? (const char *)"fly" :
		slithy(ptr)     ? (const char *)"slither" :
		amorphous(ptr)  ? (const char *)"ooze" :
		nolimbs(ptr)    ? (const char *)"crawl" :
		def
	       );

}

#endif /* OVLB */

/*mondata.c*/
