/*	SCCS Id: @(#)o_init.c	2.3	88/01/24
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

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
register int i, j, first, last, sum, end;
register char let, *tmp;

	/* bug fix to prevent "initialization error" abort on Intel Xenix.
	 * reported by mikew@semike
	 */
        for(i = 0; i != sizeof(obj_symbols); i++)
                bases[i] = 0;

	/* init base; if probs given check that they add up to 100, 
	   otherwise compute probs; shuffle descriptions */
	end = SIZE(objects);
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
			if (let == GEM_SYM) {
			    while(--j > first)
				/* NOTE:  longest color name must be default */
				if(!strcmp(objects[j].oc_name,"turquoise")) {
				    if(rn2(2)) /* change from green? */
					strcpy(objects[j].oc_descr,"blue");
				} else if (!strcmp(objects[j].oc_name,"aquamarine")) {
				    if(rn2(2)) /* change from green? */
					strcpy(objects[j].oc_descr,"blue");
				} else if (!strcmp(objects[j].oc_name,"fluorite")) {
				    switch (rn2(4)) { /* change from violet? */
					case 0:  break;
					case 1:
					    strcpy(objects[j].oc_descr,"blue");
					    break;
					case 2:
					    strcpy(objects[j].oc_descr,"white");
					    break;
					case 3:
					    strcpy(objects[j].oc_descr,"green");
					    break;
					}
				}
			} else
			    while(--j > first) {
				i = first + rn2(j+1-first);
				tmp = objects[j].oc_descr;
				objects[j].oc_descr = objects[i].oc_descr;
				objects[i].oc_descr = tmp;
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
		objects[j].oc_prob = (18+j-first)/(LAST_GEM-first);
}

oinit()			/* level dependent initialization */
{
	setgemprobs();
}

extern long *alloc();

savenames(fd) register fd; {
register int i;
unsigned len;
struct objclass *now = &objects[0];
	bwrite(fd, (char *) &now, sizeof now);
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
struct objclass *then;
long differ;
	mread(fd, (char *) &then, sizeof then);
	mread(fd, (char *) bases, sizeof bases);
	mread(fd, (char *) objects, sizeof objects);
#ifndef MSDOS
	differ = (char *)&objects[0] - (char *)then;
#else
	differ = (long)&objects[0] - (long)then;
#endif
	for(i=0; i < SIZE(objects); i++) {
		if (objects[i].oc_name) {
#ifndef MSDOS
			objects[i].oc_name += differ;
#else
			objects[i].oc_name =
			    (char *)((long)(objects[i].oc_name) + differ);
#endif
		}
		if (objects[i].oc_descr) {
#ifndef MSDOS
			objects[i].oc_descr += differ;
#else
			objects[i].oc_descr =
			    (char *)((long)(objects[i].oc_descr) + differ);
#endif
		}
		if (objects[i].oc_uname) {
			mread(fd, (char *) &len, sizeof len);
			objects[i].oc_uname = (char *) alloc(len);
			mread(fd, objects[i].oc_uname, len);
		}
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

#ifdef SPIDERS
	strcpy(objects[DEAD_GIANT_SPIDER].oc_name, "dead giant spider");
#endif

#ifdef KOPS
	strcpy(objects[DEAD_KOP].oc_name, "dead Kop");
# endif

#ifdef ROCKMOLE
	strcpy(objects[DEAD_ROCKMOLE].oc_name, "dead rockmole");
#endif

#ifndef KAA
	strcpy(objects[DEAD_QUASIT].oc_name, "dead quasit");
	strcpy(objects[DEAD_VIOLET_FUNGI].oc_name, "dead violet fungi");
#endif
	return(0);
}
