/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include <sys/types.h>
#include <sys/stat.h>
extern char *index();
struct stat buf,hbuf;

gethdate(name) char *name; {
register char *np;
	if(stat(name, &hbuf))
		error("Cannot get status of %s.",
			(np = index(name, '/')) ? np+1 : name);
}

uptodate(fd) {
	if(fstat(fd, &buf)) {
		pline("Cannot get status?");
		return(0);
	}
	if(buf.st_ctime < hbuf.st_ctime) {
		pline("Saved level is out of date.");
		return(0);
	}
 return(1);
}
