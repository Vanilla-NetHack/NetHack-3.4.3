/*	SCCS Id: @(#)mondata.c	3.0	89/11/19
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"
#include "epri.h"

/*	These routines provide basic data for any type of monster. */

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

boolean
resists_ston(ptr)	/* returns TRUE if monster is petrify resistant */
	register struct permonst *ptr;
{
	return (ptr->mflags1 & M1_STON_RES || dmgtype(ptr, AD_STON) ||
		dmgtype(ptr, AD_ACID));
}

boolean
resists_drli(ptr)	/* returns TRUE if monster is drain-life resistant */

	register struct permonst *ptr;
{
	return(is_undead(ptr) || is_demon(ptr) || is_were(ptr));
}

boolean
ranged_attk(ptr)	/* returns TRUE if monster can attack at range */
	register struct permonst *ptr;
{
	return (attacktype(ptr, AT_BREA) || attacktype(ptr, AT_WEAP) ||
		attacktype(ptr, AT_SPIT) || attacktype(ptr, AT_GAZE) ||
		attacktype(ptr, AT_MAGC));
}

boolean
can_track(ptr)		/* returns TRUE if monster can track well */
	register struct permonst *ptr;
{
#ifdef NAMED_ITEMS
	if(uwep && !strcmp(ONAME(uwep), "Excalibur")) return TRUE;
#endif
	return(haseyes(ptr));
}

#ifdef POLYSELF
boolean
breakarm(ptr)	/* creature will break out of armor */
	register struct permonst *ptr;
{
	return(bigmonst(ptr) || (ptr->msize > MZ_SMALL && !humanoid(ptr))
#ifdef INFERNO
	       || ptr == &mons[PM_MARILITH]
#endif
	       );
	/* Marilith is about the only case of a monster which is otherwise
	 * humanoid but cannot wear armor (too many arms).  Centaurs would
	 * be another except that they are already accounted for by
	 * bigmonst.
	 */
}

boolean
sliparm(ptr)	/* creature will slide out of armor */
	register struct permonst *ptr;
{
	return(ptr->msize < MZ_LARGE &&
	       (ptr->msize <= MZ_SMALL || ptr == &mons[PM_GHOST]));
}
#endif

boolean
sticks(ptr)	/* creature sticks other creatures it hits */
	register struct permonst *ptr;
{
	return(dmgtype(ptr,AD_STCK) || dmgtype(ptr,AD_WRAP) ||
		attacktype(ptr,AT_HUGS));
}

/* not one hundred percent correct: now a snake may hide under an
 *				    invisible object.
 */
boolean
canseemon(mtmp)
	register struct monst *mtmp;
{
	return((!mtmp->minvis || See_invisible)
		&& (!mtmp->mhide ||
		    (!OBJ_AT(mtmp->mx, mtmp->my) &&
		     levl[mtmp->mx][mtmp->my].gmask == 0))
		&& cansee(mtmp->mx, mtmp->my));
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

int
monsndx(ptr)		/* return an index into the mons array */
	struct	permonst	*ptr;
{
	register int	i;

	if(ptr == &playermon) return(-1);

	i = (int)(ptr - &mons[0]);

	if(i < 0 || i >= NUMMONS) {    
	    panic("monsndx - could not index monster (%x)", ptr);
	    return FALSE;		/* will not get here */
	}

	return(i);
}

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
	int len=0;

	Strcpy(buf, str);
	str = buf;
	if (!strncmp(str, "a ", 2)) str += 2;
	else if (!strncmp(str, "an ", 3)) str += 3;

	/* Some irregular plurals */
#ifdef INFERNO
	if (!strncmp(str, "incubi", 6)) return PM_INCUBUS;
	if (!strncmp(str, "succubi", 7)) return PM_SUCCUBUS;
#endif
	if (!strncmp(str, "violet fungi", 12)) return PM_VIOLET_FUNGUS;
	if (!strncmp(str, "homunculi", 9)) return PM_HOMUNCULUS;
	if (!strncmp(str, "baluchitheria", 13)) return PM_BALUCHITHERIUM;
	if (!strncmp(str, "lurkers above", 13)) return PM_LURKER_ABOVE;
	if (!strncmp(str, "cavemen", 7)) return PM_CAVEMAN;
	if (!strncmp(str, "cavewomen", 9)) return PM_CAVEWOMAN;
	if (!strncmp(str, "zruties", 7)) return PM_ZRUTY;
	if (!strncmp(str, "djinn", 5)) return PM_DJINNI;
		/* be careful with "ies"; "priest", "zombies" */
	for(s=str; *s; s++) {
		if (!strncmp(s, "vortices", 8)) {
			Strcpy(s+4, "ex");
			break;
		}
		if (!strncmp(s, "jellies", 7) || !strncmp(s, "mummies", 7)) {
			Strcpy(s+4, "y");
			break;
		}
		if (!strncmp(s, "ves", 3)) {
		/* luckily no monster names end in fe or ve with ves plurals */
			Strcpy(s, "f");
			break;
		}
	}
	
	for(i = 0; mons[i].mlet; i++) {
		if(!strncmp(mons[i].mname, str, strlen(mons[i].mname))) {
			if (strlen(mons[i].mname) > len) {
				mntmp = i;
				len = strlen(mons[i].mname);
			}
		}
	}
	return mntmp;
}

#ifdef POLYSELF
boolean
webmaker(ptr)   /* creature can spin a web */
	register struct permonst *ptr;
{
	return (ptr->mlet == S_SPIDER && ptr != &mons[PM_SCORPION]);
}
#endif

boolean
is_female(mtmp)
	register struct monst *mtmp;
{
	if (mtmp->isshk) return !ESHK(mtmp)->ismale;
#if defined(ALTARS) && defined(THEOLOGY)
	if (mtmp->ispriest) return !EPRI(mtmp)->ismale;
#endif
	return !!(mtmp->data->mflags1 & M1_FEM);
}

/* Gender function.  Differs from is_female() in that 1) It allows the monster
 * type of a polymorphed shopkeeper to override ESHK(mtmp)->ismale, and 2)
 * it returns 3 values (0=male, 1=female, 2=none) instead of 2.
 */
int
gender(mtmp)
	register struct monst *mtmp;
{
	if (!humanoid(mtmp->data)) return 2;
	if (mtmp->data->mflags1 & M1_FEM) return 1;
	if (mtmp->data == &mons[PM_CAVEMAN]
		|| mtmp->data == &mons[PM_PRIEST]
#ifdef INFERNO
		|| mtmp->data == &mons[PM_INCUBUS]
#endif
						) return 0;
#if defined(ALTARS) && defined(THEOLOGY)
	if (mtmp->ispriest) return !EPRI(mtmp)->ismale;
#endif
	if (mtmp->isshk) return !ESHK(mtmp)->ismale;
	return 0;
}

boolean
levl_follower(mtmp)
register struct monst *mtmp;
{
	return (mtmp->mtame || (mtmp->data->mflags1 & M1_STALK) || is_fshk(mtmp)
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
		case 'T': return &mons[PM_TOURIST];
		case 'V': return &mons[PM_VALKYRIE];
		case 'W': return &mons[PM_WIZARD];
		default: impossible("what are you?");
			return &mons[PM_HUMAN];
	}
}

const int grownups[][2] = { {PM_LITTLE_DOG, PM_DOG}, {PM_DOG, PM_LARGE_DOG},
	{PM_HELL_HOUND_PUP, PM_HELL_HOUND}, {PM_KITTEN, PM_HOUSECAT},
	{PM_HOUSECAT, PM_LARGE_CAT}, {PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON},
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
#ifdef WORM
	{PM_BABY_LONG_WORM, PM_LONG_WORM},
#endif
#ifdef ARMY
	{PM_SOLDIER, PM_SERGEANT},
	{PM_SERGEANT, PM_LIEUTENANT},
	{PM_LIEUTENANT, PM_CAPTAIN},
#endif
	{-1,-1}
};

int little_to_big(montype)
int montype;
{
	register int i;
	
	for(i=0; grownups[i][0] >= 0; i++)
		if(montype == grownups[i][0]) return grownups[i][1];
	return montype;
}

int big_to_little(montype)
int montype;
{
	register int i;
	
	for(i=0; grownups[i][0] >= 0; i++)
		if(montype == grownups[i][1]) return grownups[i][0];
	return montype;
}


#ifdef STUPID_CPP	/* otherwise these functions are macros in mondata.h */

int
bigmonst(ptr) struct permonst *ptr; {
	return(ptr->msize >= MZ_LARGE);
}

int
verysmall(ptr) struct permonst *ptr; {
	return(ptr->msize < MZ_SMALL);
}

int
is_flyer(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_FLY) != 0L);
}

int
is_floater(ptr) struct permonst *ptr; {
	return(ptr->mlet == S_EYE);
}

int
is_swimmer(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_SWIM) != 0L);
}

int
passes_walls(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_WALLWALK) != 0L);
}

int
noncorporeal(ptr) struct permonst *ptr; {
	return(ptr->mlet == S_GHOST);
}

int
is_animal(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_ANIMAL) != 0L);
}

int
humanoid(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_HUMANOID) != 0L);
}

int
is_undead(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_UNDEAD) != 0L);
}

int
is_were(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_WERE) != 0L);
}

int haseyes(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_NOEYES) == 0L);
}

int
nohands(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_NOHANDS) != 0L);
}

int
lays_eggs(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_EGGS) != 0L);
}

int
poisonous(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_POIS) != 0L);
}

int
resists_poison(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & (M1_POIS | M1_POIS_RES)) != 0L);
}

int
resists_fire(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_FIRE_RES) != 0L);
}

int
resists_cold(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_COLD_RES) != 0L);
}

int
resists_acid(ptr) struct permonst *ptr; {
	return(dmgtype(ptr, AD_ACID));
}

int
resists_elec(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_ELEC_RES) != 0L);
}

int
resists_sleep(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & (M1_SLEE_RES | M1_UNDEAD)) != 0L);
}

int
resists_disint(ptr) struct permonst *ptr; {
	return(ptr == &mons[PM_BLACK_DRAGON] ||
		ptr == &mons[PM_BABY_BLACK_DRAGON]);
}

int
regenerates(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_REGEN) != 0L);
}

int
perceives(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_SEE_INVIS) != 0L);
}

int
can_teleport(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_TPORT) != 0L);
}

int
control_teleport(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_TPORT_CONTROL) != 0L);
}

int
is_armed(ptr) struct permonst *ptr; {
	return(attacktype(ptr, AT_WEAP));
}

int
likes_gold(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_GREEDY) != 0L);
}

int
likes_gems(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_JEWELS) != 0L);
}

int
likes_objs(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_COLLECT) != 0L || is_armed(ptr));
}

int
likes_magic(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_MAGIC) != 0L);
}

int
hides_under(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_CONCEAL) != 0L);
}

int
is_hider(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_HIDE) != 0L);
}

# ifdef POLYSELF
int
polyok(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_NOPOLY) == 0L);
}
# endif /* POLYSELF */

int
tunnels(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_TUNNEL) != 0L);
}

int
needspick(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_NEEDPICK) != 0L);
}

int
is_elf(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_ELF) != 0L);
}

int
is_dwarf(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_DWARF) != 0L);
}

int
is_giant(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_GIANT) != 0L);
}

# ifdef GOLEMS
int
is_golem(ptr) struct permonst *ptr; {
	return(ptr->mlet == S_GOLEM);
}
# endif /* GOLEMS */

int
is_orc(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_ORC) != 0L);
}

int
is_human(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_HUMAN) != 0L);
}

int
is_demon(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_DEMON) != 0L);
}

int
is_mercenary(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_MERC) != 0L);
}

int
throws_rocks(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_ROCKTHROW) != 0L);
}

int
is_wanderer(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_WANDER) != 0L);
}

int
is_lord(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_LORD) != 0L);
}

int
is_prince(ptr) struct permonst *ptr; {
	return((ptr->mflags1 & M1_PRINCE) != 0L);
}

# ifdef INFERNO
int
is_ndemon(ptr) struct permonst *ptr; {
	return(is_demon(ptr) &&
		(ptr->mflags1 & (M1_LORD | M1_PRINCE)) == 0L);
}
# else
int
is_ndemon(ptr) struct permonst *ptr; {
	return(ptr == &mons[PM_DEMON]);
}
# endif

int
is_dlord(ptr) struct permonst *ptr; {
	return(is_demon(ptr) && is_lord(ptr));
}

int
is_dprince(ptr) struct permonst *ptr; {
	return(is_demon(ptr) && is_prince(ptr));
}

int
type_is_pname(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_PNAME) != 0L);
}

int
always_hostile(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_HOSTILE) != 0L);
}

int
always_peaceful(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_PEACEFUL) != 0L);
}

int
strongmonst(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_STRONG) != 0L);
}

int
extra_nasty(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_NASTY) != 0L);
}

# ifdef POLYSELF
int
can_breathe(ptr) struct permonst *ptr; {
	return(attacktype(ptr, AT_BREA));
}

int
cantwield(ptr) struct permonst *ptr; {
	return(nohands(ptr) || verysmall(ptr));
}

int
cantweararm(ptr) struct permonst *ptr; {
	return(breakarm(ptr) || sliparm(ptr));
}
# endif /* POLYSELF */

int
nolimbs(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_NOLIMBS) != 0L);
}

int
carnivorous(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_CARNIVORE) != 0L);
}

int
herbivorous(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_HERBIVORE) != 0L);
}

int
thick_skinned(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_THICK_HIDE) != 0L);
}

int
amorphous(ptr) struct permonst *ptr; {
	return((ptr->mflags2 & M2_AMORPHOUS) != 0L);
}

#endif /* STUPID_CPP */
