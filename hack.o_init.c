/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"config.h"		/* for typedefs */
#include	"def.objects.h"

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
	/* init base; if probs given check that they add up to 100, 
	   otherwise compute probs; shuffle descriptions */
	end = sizeof(objects)/sizeof(objects[0]);
	first = 0;
	while( first < end ) {
		let = objects[first].oc_olet;
		last = first+1;
		while(last < end && objects[last].oc_olet == let
				&& objects[last].oc_name != NULL)
			last++;
		i = letindex(let);
		if((!i && let != ILLOBJ_SYM) || bases[i] != 0)
			panic("initialization error");
		bases[i] = first;
	check:
#ifdef MKLEV
#include	"hack.onames.h"
		if(let == GEM_SYM) {
			extern xchar dlevel;
			for(j=0; j < 9-dlevel/3; j++)
				objects[first+j].oc_prob = 0;
			first += j;
			if(first >= last || first >= LAST_GEM)
				printf("Not enough gems? - first=%d last=%d j=%d LAST_GEM=%d\n", first, last, j, LAST_GEM);
			for(j = first; j < LAST_GEM; j++)
			    objects[j].oc_prob = (20+j-first)/(LAST_GEM-first);
		}
#endif MKLEV
		sum = 0;
		for(j = first; j < last; j++) sum += objects[j].oc_prob;
		if(sum == 0) {
			for(j = first; j < last; j++)
			    objects[j].oc_prob = (100+j-first)/(last-first);
			goto check;
		}
		if(sum != 100)
#ifdef MKLEV
			panic
#else
			error
#endif MKLEV
				("init-prob error for %c", let);
		/* shuffling is rather meaningless in mklev, 
		   but we must update  last  anyway */
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

#ifndef MKLEV
#define SIZE(x) (sizeof x)/(sizeof x[0])
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
	mread(fd, (char *) bases, sizeof bases);
	mread(fd, (char *) objects, sizeof objects);
	for(i=0; i < SIZE(objects); i++) if(objects[i].oc_uname) {
		mread(fd, (char *) &len, sizeof len);
		objects[i].oc_uname = (char *) alloc(len);
		mread(fd, objects[i].oc_uname, len);
	}
}
#endif MKLEV
