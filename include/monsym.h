/*	SCCS Id: @(#)makemon.c	3.0	88/04/11
/*	Monster symbols and creation information rev 1.0 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONSYM_H
#define MONSYM_H

#define	S_ANT		'a'
#define	S_BLOB		'b'
#define	S_COCKATRICE	'c'
#define	S_DOG		'd'
#define	S_EYE		'e'
#define S_FELINE	'f'
#define	S_GREMLIN	'g'
#define	S_HUMANOID	'h'
#define	S_IMP		'i'
#define	S_JELLY		'j'
#define	S_KOBOLD	'k'
#define	S_LICH		'l'
#define	S_MIMIC		'm'
#define	S_NAGA		'n'
#define	S_ORC		'o'
#define	S_PIERCER	'p'
#define	S_QUADRUPED	'q'
#define	S_RODENT	'r'
#define	S_SPIDER	's'
#define	S_TRAPPER	't'
#define	S_UNICORN	'u'
#define	S_VORTEX	'v'
#define	S_WORM		'w'
#define	S_XAN		'x'
#define S_YLIGHT	'y'
#define	S_ZRUTY		'z'
#define	S_APE		'A'
#define	S_BAT		'B'
#define	S_CENTAUR	'C'
#define	S_DRAGON	'D'
#define	S_ELEMENTAL	'E'
#define	S_FUNGUS	'F'
#define	S_GNOME		'G'
#define	S_GIANT		'H'
#define	S_STALKER	'I'
#define	S_JABBERWOCK	'J'
#define	S_KOP		'K'
#define	S_LEPRECHAUN	'L'
#define	S_MUMMY		'M'
#define	S_NYMPH		'N'
#define	S_OGRE		'O'
#define	S_PUDDING	'P'
#define	S_QUANTMECH	'Q'
#define	S_RUSTMONST	'R'
#define	S_SNAKE		'S'
#define	S_TROLL		'T'
#define	S_UMBER		'U'
#define	S_VAMPIRE	'V'
#define	S_WRAITH	'W'
#define	S_XORN		'X'
#define	S_YETI		'Y'
#define	S_ZOMBIE	'Z'
#define	S_HUMAN		'@'
#define	S_GHOST		' '
#define S_GOLEM		'\''
#define	S_DEMON		'&'
#define	S_EEL		';'
#define	S_CHAMELEON	':'

#define	S_WORM_TAIL	'~'
#define	S_MIMIC_DEF	']'

#define	G_UNIQ		0x800		/* generated only once */
#define	G_HELL		0x400		/* generated only in "hell" */
#define	G_NOGEN		0x200		/* generated only specially */
#define	G_NOCORPSE	0x100		/* no corpse left ever */
#define	G_SGROUP	0x080		/* appear in small groups normally */
#define	G_LGROUP	0x040		/* appear in large groups normally */
#define	G_GENO		0x020		/* can be genocided */
#define G_GENOD		0x010		/* have been genocided */
#define G_FREQ		0x007		/* creation frequency mask */

#endif /* MONSYM_H /* */
