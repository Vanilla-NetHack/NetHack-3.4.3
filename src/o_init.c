/*	SCCS Id: @(#)o_init.c	3.1	92/12/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"		/* for typedefs */

static void NDECL(setgemprobs);
static void FDECL(shuffle,(int,int,BOOLEAN_P));
static boolean FDECL(interesting_to_discover,(int));

/* note that NROFOBJECTS is the number of legal objects, which does not count
 * the strange object and null object that take up positions 0 and NROFOBJECTS+1
 * in the objects array
 */
#define TOTAL_OBJS	(NROFOBJECTS+2)

const char obj_symbols[] = {
	ILLOBJ_CLASS, AMULET_CLASS, GOLD_CLASS, FOOD_CLASS, WEAPON_CLASS,
	TOOL_CLASS, BALL_CLASS, CHAIN_CLASS, ROCK_CLASS, ARMOR_CLASS,
	POTION_CLASS, SCROLL_CLASS, WAND_CLASS,
	SPBOOK_CLASS, RING_CLASS, GEM_CLASS, 0 };

static NEARDATA short disco[TOTAL_OBJS] = DUMMY;

int
letindex(acls) register char acls; {
register int i = 0;
register char ch;
	while ((ch = obj_symbols[i++]) != 0)
		if (ch == acls) return(i);
	return(0);
}

static void
setgemprobs()
{
	register int j, first;
	int lev = (ledger_no(&u.uz) > maxledgerno())
				? maxledgerno() : ledger_no(&u.uz);

	first = bases[letindex(GEM_CLASS)];

	for(j = 0; j < 9-lev/3; j++)
		objects[first+j].oc_prob = 0;
	first += j;
	if (first > LAST_GEM || objects[first].oc_class != GEM_CLASS ||
	    OBJ_NAME(objects[first]) == NULL) {
		raw_printf("Not enough gems? - first=%d j=%d LAST_GEM=%d",
			first, j, LAST_GEM);
		wait_synch();
	    }
	for (j = first; j <= LAST_GEM; j++)
		objects[j].oc_prob = (184+j-first)/(LAST_GEM+1-first);
}

/* shuffle descriptions on objects o_low to o_high */
static void
shuffle(o_low, o_high, domaterial)
	register int o_low, o_high;
	register boolean domaterial;
{
	register int i, j;
#ifdef TEXTCOLOR
	int color;
#endif /* TEXTCOLOR */
	register short sw;

	for (j=o_low; j <= o_high; j++) {
		i = o_low + rn2(j+1-o_low);
		sw = objects[j].oc_descr_idx;
		objects[j].oc_descr_idx = objects[i].oc_descr_idx;
		objects[i].oc_descr_idx = sw;
#ifdef TEXTCOLOR
		color = objects[j].oc_color;
		objects[j].oc_color = objects[i].oc_color;
		objects[i].oc_color = color;
#endif /* TEXTCOLOR */
		/* shuffle material */
		if (domaterial) {
			sw = objects[j].oc_material;
			objects[j].oc_material = objects[i].oc_material;
			objects[i].oc_material = sw;
		}
	}
}

void
init_objects(){
register int i, j, first, last, sum, end;
register char acls;
#ifdef TEXTCOLOR
# define COPY_OBJ_DESCR(o_dst,o_src) \
			o_dst.oc_descr_idx = o_src.oc_descr_idx,\
			o_dst.oc_color = o_src.oc_color
#else
# define COPY_OBJ_DESCR(o_dst,o_src) o_dst.oc_descr_idx = o_src.oc_descr_idx
#endif

	/* bug fix to prevent "initialization error" abort on Intel Xenix.
	 * reported by mikew@semike
	 */
	for (i = 0; i < sizeof(obj_symbols); i++)
		bases[i] = 0;
	/* initialize object descriptions */
	for (i = 0; i < TOTAL_OBJS; i++)
		objects[i].oc_name_idx = objects[i].oc_descr_idx = i;
	init_artifacts();
	/* init base; if probs given check that they add up to 1000,
	   otherwise compute probs; shuffle descriptions */
	end = TOTAL_OBJS;
	first = 0;
	while( first < end ) {
		acls = objects[first].oc_class;
		last = first+1;
		while (last < end && objects[last].oc_class == acls) last++;
		i = letindex(acls);
		if ((!i && acls != ILLOBJ_CLASS && acls != VENOM_CLASS) ||
								bases[i] != 0)
			error("initialization error for object class %d", acls);
		bases[i] = first;

		if (acls == GEM_CLASS) setgemprobs();
	check:
		sum = 0;
		for(j = first; j < last; j++) sum += objects[j].oc_prob;
		if(sum == 0) {
			for(j = first; j < last; j++)
			    objects[j].oc_prob = (1000+j-first)/(last-first);
			goto check;
		}
		if(sum != 1000)
			error("init-prob error for %d (%d%%)", acls, sum);

		if (OBJ_DESCR(objects[first]) != NULL &&
		   acls != TOOL_CLASS && acls != WEAPON_CLASS && acls != ARMOR_CLASS) {

		    /* shuffle, also some additional descriptions */
		    while (last < end && objects[last].oc_class == acls)
			last++;
		    j = last;
		    if (acls == GEM_CLASS) {
			if (rn2(2)) { /* change turquoise from green to blue? */
			    COPY_OBJ_DESCR(objects[TURQUOISE],objects[SAPPHIRE]);
			}
			if (rn2(2)) { /* change aquamarine from green to blue? */
			    COPY_OBJ_DESCR(objects[AQUAMARINE],objects[SAPPHIRE]);
			}
			switch (rn2(4)) { /* change fluorite from violet? */
			    case 0:  break;
			    case 1:	/* blue */
				COPY_OBJ_DESCR(objects[FLUORITE],objects[SAPPHIRE]);
				break;
			    case 2:	/* white */
				COPY_OBJ_DESCR(objects[FLUORITE],objects[DIAMOND]);
				break;
			    case 3:	/* green */
				COPY_OBJ_DESCR(objects[FLUORITE],objects[EMERALD]);
				break;
			}
		    } else {
			if (acls == POTION_CLASS)
			    j--;  /* only water has a fixed description */
			else if (acls == AMULET_CLASS ||
				 acls == SCROLL_CLASS ||
				 acls == SPBOOK_CLASS)
			    do { j--; }
			    while (!objects[j].oc_magic || objects[j].oc_unique);
			/* non-magical amulets, scrolls, and spellbooks
			 * (ex. imitation Amulets, blank, scrolls of mail)
			 * and one-of-a-kind magical artifacts at the end of
			 * their class in objects[] have fixed descriptions.
			 */
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

	/* shuffle the boots [if they change, update find_skates() below] */
	shuffle(SPEED_BOOTS, LEVITATION_BOOTS, FALSE);
}

/* find the object index for snow boots; used [once] by slippery ice code */
int
find_skates()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= LEVITATION_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "snow boots"))
	    return i;

    impossible("snow boots not found?");
    return -1;	/* not 0, or caller would try again each move */
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

	bwrite(fd, (genericptr_t)bases, MAXOCLASSES * sizeof *bases);
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

	mread(fd, (genericptr_t) bases, MAXOCLASSES * sizeof *bases);
	mread(fd, (genericptr_t) disco, sizeof disco);
	mread(fd, (genericptr_t) objects, sizeof(struct objclass) * TOTAL_OBJS);
	for(i=0; i < TOTAL_OBJS; i++) {
		if (objects[i].oc_uname) {
			mread(fd, (genericptr_t) &len, sizeof len);
			objects[i].oc_uname = (char *) alloc(len);
			mread(fd, (genericptr_t)objects[i].oc_uname, len);
		}
	}
}

void
discover_object(oindx, mark_as_known)
register int oindx;
boolean mark_as_known;
{
    if (!objects[oindx].oc_name_known) {
	register int dindx, acls = objects[oindx].oc_class;

	/* Loop thru disco[] 'til we find the target (which may have been
	   uname'd) or the next open slot; one or the other will be found
	   before we reach the next class...
	 */
	for (dindx = bases[letindex(acls)]; disco[dindx] != 0; dindx++)
	    if (disco[dindx] == oindx) break;
	disco[dindx] = oindx;

	if (mark_as_known) {
	    objects[oindx].oc_name_known = 1;
	    exercise(A_WIS, TRUE);
	}
    }
}

/* if a class name has been cleared, we may need to purge it from disco[] */
void
undiscover_object(oindx)
register int oindx;
{
    if (!objects[oindx].oc_name_known) {
	register int dindx, acls = objects[oindx].oc_class;
	register boolean found = FALSE;

	/* find the object; shift those behind it forward one slot */
	for (dindx = bases[letindex(acls)];
	      dindx <= NROFOBJECTS && disco[dindx] != 0
		&& objects[dindx].oc_class == acls; dindx++)
	    if (found)
		disco[dindx-1] = disco[dindx];
	    else if (disco[dindx] == oindx)
		found = TRUE;

	/* clear last slot */
	if (found) disco[dindx-1] = 0;
	else impossible("named object not in disco");
    }
}

static boolean
interesting_to_discover(i)
register int i;
{
    return objects[i].oc_uname != NULL ||
		(objects[i].oc_name_known && OBJ_DESCR(objects[i]) != NULL);
}

int
dodiscovered()				/* free after Robert Viduya */
{
    register int i, dis;
    int	ct = 0;
    char class = -1;
    winid tmpwin;

    tmpwin = create_nhwindow(NHW_MENU);
    putstr(tmpwin, 0, "Discoveries");
    putstr(tmpwin, 0, "");

    for (i = 0; i <= NROFOBJECTS; i++) {
	if ((dis = disco[i]) && interesting_to_discover(dis)) {
	    ct++;
	    if (objects[dis].oc_class != class) {
		class = objects[dis].oc_class;
		putstr(tmpwin, ATR_INVERSE, let_to_name(class, FALSE));
	    }
	    putstr(tmpwin, 0, typename(dis));
	}
    }
    if (ct == 0) {
	You("haven't discovered anything yet...");
    } else
	display_nhwindow(tmpwin, TRUE);
    destroy_nhwindow(tmpwin);

    return 0;
}

/*o_init.c*/
