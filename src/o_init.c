/*	SCCS Id: @(#)o_init.c	3.0	88/07/06
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"		/* for typedefs */

/* note that NROFOBJECTS is the number of legal objects, which does not count
 * the strange object and null object that take up positions 0 and NROFOBJECTS+1
 * in the objects array
 */
#define TOTAL_OBJS	(NROFOBJECTS+2)

const char obj_symbols[] = {
	ILLOBJ_SYM, AMULET_SYM, FOOD_SYM, WEAPON_SYM, TOOL_SYM,
	BALL_SYM, CHAIN_SYM, ROCK_SYM, ARMOR_SYM,
	POTION_SYM, SCROLL_SYM, WAND_SYM,
#ifdef SPELLS
	SPBOOK_SYM,
#endif
	RING_SYM, GEM_SYM, 0 };

int bases[sizeof(obj_symbols)] = DUMMY;
static int disco[TOTAL_OBJS] = DUMMY;

int
letindex(let) register char let; {
register int i = 0;
register char ch;
	while((ch = obj_symbols[i++]) != 0)
		if(ch == let) return(i);
	return(0);
}

static void
setgemprobs()
{
	register int j,first;
#ifdef STRONGHOLD
	int level = (dlevel > MAXLEVEL) ? MAXLEVEL : dlevel;
#endif

	first = bases[letindex(GEM_SYM)];

#ifdef STRONGHOLD
	for(j = 0; j < 9-level/3; j++)
#else
	for(j = 0; j < 9-dlevel/3; j++)
#endif
		objects[first+j].oc_prob = 0;
	first += j;
	if(first >= LAST_GEM || first > NROFOBJECTS ||
	    objects[first].oc_olet != GEM_SYM ||
	    objects[first].oc_name == NULL)
		Printf("Not enough gems? - first=%d j=%d LAST_GEM=%d\n",
			first, j, LAST_GEM);
	for(j = first; j < LAST_GEM; j++)
		objects[j].oc_prob = (180+j-first)/(LAST_GEM-first);
}

/* shuffle descriptions on objects o_low to o_high */
static void
shuffle(o_low, o_high, domaterial)

	register int o_low, o_high;
	register boolean domaterial;
{
	register int i, j;
	char *desc;
	int tmp;

	for(j=o_low; j <= o_high; j++) {
		i = o_low + rn2(j+1-o_low);
		desc = objects[j].oc_descr;
		objects[j].oc_descr = objects[i].oc_descr;
		objects[i].oc_descr = desc;
		/* shuffle discovery list */
		tmp = disco[j];
		disco[j] = disco[i];
		disco[i] = tmp;
		/* shuffle material */
		if(domaterial) {
			tmp = objects[j].oc_material;
			objects[j].oc_material = objects[i].oc_material;
			objects[i].oc_material = tmp;
		}
	}
}

void
init_objects(){
register int i, j, first, last, sum, end;
register char let;

	/* bug fix to prevent "initialization error" abort on Intel Xenix.
	 * reported by mikew@semike
	 */
	for(i = 0; i != sizeof(obj_symbols); i++)
		bases[i] = 0;
	for(i = 0; i != TOTAL_OBJS; i++)
		disco[i] = i;

	/* init base; if probs given check that they add up to 1000,
	   otherwise compute probs; shuffle descriptions */
	end = TOTAL_OBJS;
	first = 0;
	while( first < end ) {
		let = objects[first].oc_olet;
		last = first+1;
		while(last < end && objects[last].oc_olet == let
				 && objects[last].oc_name != NULL) last++;
		i = letindex(let);
		if((!i && let != ILLOBJ_SYM && let != '.') || bases[i] != 0)
			error("initialization error for %c", let);
		bases[i] = first;

		if(let == GEM_SYM) setgemprobs();
	check:
		sum = 0;
		for(j = first; j < last; j++) sum += objects[j].oc_prob;
		if(sum == 0) {
			for(j = first; j < last; j++)
			    objects[j].oc_prob = (1000+j-first)/(last-first);
			goto check;
		}
		if(sum != 1000)
			error("init-prob error for %c (%d%%)", let, sum);

		if(objects[first].oc_descr != NULL &&
		   let != TOOL_SYM && let != WEAPON_SYM && let != ARMOR_SYM) {

			/* shuffle, also some additional descriptions */
			while(last < end && objects[last].oc_olet == let)
				last++;
			j = last;
			if (let == GEM_SYM) {
			    while(--j > first)
				/* NOTE:  longest color name must be default */
				if(!strcmp(objects[j].oc_name,"turquoise")) {
				    if(rn2(2)) /* change from green? */
					Strcpy(objects[j].oc_descr, blue);
				} else if (!strcmp(objects[j].oc_name,"aquamarine")) {
				    if(rn2(2)) /* change from green? */
					Strcpy(objects[j].oc_descr, blue);
				} else if (!strcmp(objects[j].oc_name,"fluorite")) {
				    switch (rn2(4)) { /* change from violet? */
					case 0:  break;
					case 1:
					    Strcpy(objects[j].oc_descr, blue);
					    break;
					case 2:
					    Strcpy(objects[j].oc_descr, white);
					    break;
					case 3:
					    Strcpy(objects[j].oc_descr, green);
					    break;
					}
				}
			} else {
			    if (let == AMULET_SYM || let == POTION_SYM)
				j--;  /* THE amulet doesn't have description */
			    /* and water is always "clear" - 3. */
			    shuffle(first, --j, TRUE);
			}
		}
		first = last;
	}

	/* shuffle the helmets */
	shuffle(HELMET, HELM_OF_TELEPATHY, FALSE);

	/* shuffle the gloves */
	shuffle(LEATHER_GLOVES, GAUNTLETS_OF_DEXTERITY, FALSE);

	/* shuffle the cloaks */
	shuffle(CLOAK_OF_PROTECTION, CLOAK_OF_DISPLACEMENT, FALSE);

	/* shuffle the boots */
	shuffle(SPEED_BOOTS, LEVITATION_BOOTS, FALSE);
}

void
oinit()			/* level dependent initialization */
{
	setgemprobs();
}

void
savenames(fd)
register int fd;
{
	register int i;
	unsigned int len;
	struct objclass *now = &objects[0];
	bwrite(fd, (genericptr_t)&now, sizeof now);
	bwrite(fd, (genericptr_t)bases, sizeof bases);
	bwrite(fd, (genericptr_t)disco, sizeof disco);
	bwrite(fd, (genericptr_t)objects, sizeof(struct objclass) * TOTAL_OBJS);
	/* as long as we use only one version of Hack we
	   need not save oc_name and oc_descr, but we must save
	   oc_uname for all objects */
	for(i=0; i < TOTAL_OBJS; i++) {
		if(objects[i].oc_uname) {
			len = strlen(objects[i].oc_uname)+1;
			bwrite(fd, (genericptr_t)&len, sizeof len);
			bwrite(fd, (genericptr_t)objects[i].oc_uname, len);
		}
	}
}

void
restnames(fd)
register int fd;
{
	register int i;
	unsigned int len;
	struct objclass *then;
	long differ;
	mread(fd, (genericptr_t) &then, sizeof then);
	mread(fd, (genericptr_t) bases, sizeof bases);
	mread(fd, (genericptr_t) disco, sizeof disco);
	mread(fd, (genericptr_t) objects, sizeof(struct objclass) * TOTAL_OBJS);
#if !defined(MSDOS) && !defined(M_XENIX)
	differ = (genericptr_t)&objects[0] - (genericptr_t)then;
#else
	differ = (long)&objects[0] - (long)then;
#endif
	for(i=0; i < TOTAL_OBJS; i++) {
		if (objects[i].oc_name) {
#if !defined(MSDOS) && !defined(M_XENIX)
			objects[i].oc_name += differ;
#else
			objects[i].oc_name =
			    (char *)((long)(objects[i].oc_name) + differ);
#endif
		}
		if (objects[i].oc_descr) {
#if !defined(MSDOS) && !defined(M_XENIX)
			objects[i].oc_descr += differ;
#else
			objects[i].oc_descr =
			    (char *)((long)(objects[i].oc_descr) + differ);
#endif
		}
		if (objects[i].oc_uname) {
			mread(fd, (genericptr_t) &len, sizeof len);
			objects[i].oc_uname = (char *) alloc(len);
			mread(fd, (genericptr_t)objects[i].oc_uname, len);
		}
	}
}

static boolean
interesting_to_discover(i)
register int i;
{
    return objects[i].oc_uname != NULL ||
		(objects[i].oc_name_known && objects[i].oc_descr != NULL);
}

int
dodiscovered()				/* free after Robert Viduya */
{
    register int i, dis;
    int	ct = 0;
    char class = -1;

    cornline(0, "Discoveries");

    for (i = 0; i <= NROFOBJECTS; i++) {
	if (interesting_to_discover(dis = disco[i])) {
	    ct++;
	    if (objects[dis].oc_olet != class) {
		class = objects[dis].oc_olet;
		cornline(1, let_to_name(class));
	    }
	    cornline(1, typename(dis));
	}
    }
    if (ct == 0) {
	You("haven't discovered anything yet...");
	cornline(3, NULL);
    } else
	cornline(2, NULL);

    return 0;
}
