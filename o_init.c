/*	SCCS Id: @(#)o_init.c	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* o_init.c - version 1.0.3 */

#include	"config.h"		/* for typedefs */
#include	"objects.h"
#include	"onames.h"		/* for LAST_GEM */
extern char *index();

int
letindex(let) register char let; {
register int i = 0;
register char ch;
	while((ch = obj_symbols[i++]) != 0)
		if(ch == let) return(i);
	return(0);
}

init_objects(){
register int i, j, first, last, sum, end, tmp_i;
register char let, *tmp;

	/* bug fix to prevent "initialization error" abort on Intel Xenix.
	 * reported by mikew@semike
	 */
        for(i = 0; i != sizeof(obj_symbols); i++)
                bases[i] = 0;

	/* init base; if probs given check that they add up to 100, 
	   otherwise compute probs; shuffle descriptions */
	end = SIZE(objects);
#ifdef MSDOS
	/* Assign indices to all oc_descr_i first */
	for (i = 0; i < end; i++)
		objects[i].oc_descr_i = i;
#endif
	first = 0;
	while( first < end ) {
		let = objects[first].oc_olet;
		last = first+1;
		while(last < end && objects[last].oc_olet == let
				 && objects[last].oc_name != NULL) last++;
		i = letindex(let);
		if((!i && let != ILLOBJ_SYM) || bases[i] != 0)
			error("initialization error");
		bases[i] = first;

		if(let == GEM_SYM) setgemprobs();
	check:
		sum = 0;
		for(j = first; j < last; j++) sum += objects[j].oc_prob;
		if(sum == 0) {
			for(j = first; j < last; j++)
			    objects[j].oc_prob = (100+j-first)/(last-first);
			goto check;
		}
		if(sum != 100)
			error("init-prob error for %c (%d%%)", let, sum);

		if(objects[first].oc_descr != NULL && let != TOOL_SYM){
			/* shuffle, also some additional descriptions */
			while(last < end && objects[last].oc_olet == let)
				last++;
			j = last;
			while(--j > first) {
				i = first + rn2(j+1-first);
				tmp = objects[j].oc_descr;
				objects[j].oc_descr = objects[i].oc_descr;
				objects[i].oc_descr = tmp;
#ifdef MSDOS
	/* keep track of where the description came from */
				tmp_i = objects[j].oc_descr_i;
				objects[j].oc_descr_i = objects[i].oc_descr_i;
				objects[i].oc_descr_i = tmp_i;
#endif
			}
		}
		first = last;
	}
}

probtype(let) register char let; {
register int i = bases[letindex(let)];
register int prob = rn2(100);
	while((prob -= objects[i].oc_prob) >= 0) i++;
	if(objects[i].oc_olet != let || !objects[i].oc_name)
		panic("probtype(%c) error, i=%d", let, i);
	return(i);
}

setgemprobs()
{
	register int j,first;
	extern xchar dlevel;

	first = bases[letindex(GEM_SYM)];

	for(j = 0; j < 9-dlevel/3; j++)
		objects[first+j].oc_prob = 0;
	first += j;
	if(first >= LAST_GEM || first >= SIZE(objects) ||
	    objects[first].oc_olet != GEM_SYM ||
	    objects[first].oc_name == NULL)
		printf("Not enough gems? - first=%d j=%d LAST_GEM=%d\n",
			first, j, LAST_GEM);
	for(j = first; j < LAST_GEM; j++)
		objects[j].oc_prob = (20+j-first)/(LAST_GEM-first);
}

oinit()			/* level dependent initialization */
{
	setgemprobs();
}

extern long *alloc();

savenames(fd) register fd; {
register int i;
unsigned len;
	bwrite(fd, (char *) bases, sizeof bases);
	bwrite(fd, (char *) objects, sizeof objects);
	/* as long as we use only one version of Hack/Quest we
	   need not save oc_name and oc_descr, but we must save
	   oc_uname for all objects */
	for(i=0; i < SIZE(objects); i++) {
		if(objects[i].oc_uname) {
			len = strlen(objects[i].oc_uname)+1;
			bwrite(fd, (char *) &len, sizeof len);
			bwrite(fd, objects[i].oc_uname, len);
		}
	}
}

restnames(fd) register fd; {
register int i;
unsigned len;
#ifdef MSDOS
	char *oc_descr[NROFOBJECTS + 1], *oc_name;

	mread(fd, (char *) bases, sizeof bases);

	/* Read in objects 1 at a time, correcting oc_name pointer and
	 * saving pointer to current description.
	 */
	for (i = 0; i < SIZE(objects); i++) {
		oc_name = objects[i].oc_name;
		oc_descr[i] = objects[i].oc_descr;
		mread(fd, (char *) &objects[i], sizeof (struct objclass));
		objects[i].oc_name = oc_name;
	}

	/* Convert from saved indices into pointers */
	for (i = 0; i < SIZE(objects); i++)
		objects[i].oc_descr = oc_descr[objects[i].oc_descr_i];
#else
	mread(fd, (char *) bases, sizeof bases);
	mread(fd, (char *) objects, sizeof objects);
#endif
	for(i=0; i < SIZE(objects); i++) if(objects[i].oc_uname) {
		mread(fd, (char *) &len, sizeof len);
		objects[i].oc_uname = (char *) alloc(len);
		mread(fd, objects[i].oc_uname, len);
	}
}

dodiscovered()				/* free after Robert Viduya */
{
    extern char *typename();
    register int i, end;
    int	ct = 0;
#ifdef DGKMOD
    char class = -1;
    extern char *let_to_name();
#endif

    cornline(0, "Discoveries");

    end = SIZE(objects);
    for (i = 0; i < end; i++) {
	if (interesting_to_discover (i)) {
	    ct++;
#ifdef DGKMOD
	    if (objects[i].oc_olet != class) {
		class = objects[i].oc_olet;
		cornline(1, let_to_name(class));
	    }
#endif
	    cornline(1, typename(i));
	}
    }
    if (ct == 0) {
	pline ("You haven't discovered anything yet...");
	cornline(3, (char *) 0);
    } else
	cornline(2, (char *) 0);

    return(0);
}

interesting_to_discover(i)
register int i;
{
    return(
	objects[i].oc_uname != NULL ||
	 (objects[i].oc_name_known && objects[i].oc_descr != NULL)
    );
}

init_corpses() {

#ifdef KOPS
	strcpy(objects[DEAD_KOP].oc_name, "dead Kop");
#endif
#ifdef SPIDERS
	strcpy(objects[DEAD_GIANT_SPIDER].oc_name, "dead giant spider");
#endif
#ifdef ROCKMOLE
	strcpy(objects[DEAD_ROCKMOLE].oc_name, "dead rockmole");
#endif
#ifndef KAA
	strcpy(objects[DEAD_QUASIT].oc_name, "dead quasit");
	strcpy(objects[DEAD_VIOLET_FUNGI].oc_name, "dead violet fungi");
#endif
	return(0);
}
