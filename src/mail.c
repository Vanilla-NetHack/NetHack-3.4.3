/*	SCCS Id: @(#)mail.c	3.0	89/07/07
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"	/* mainly for index() which depends on BSD */

#ifdef MAIL

# ifdef UNIX
#include <sys/stat.h>
# endif

/*
 * Notify user when new mail has arrived. [Idea from Merlyn Leroy, but
 * I don't know the details of his implementation.]
 * { Later note: he disliked my calling a general mailreader and felt that
 *   hack should do the paging itself. But when I get mail, I want to put it
 *   in some folder, reply, etc. - it would be unreasonable to put all these
 *   functions in hack. }
 * The motion of the mail daemon is less restrained than usual:
 * diagonal moves from a DOOR are possible. He might also use SDOOR's. Also,
 * the mail daemon is visible in a ROOM, even when you are Blind.
 * Its path should be longer when you are Telepat-hic and Blind.
 *
 * Possible extensions:
 *	- Open the file MAIL and do fstat instead of stat for efficiency.
 *	  (But sh uses stat, so this cannot be too bad.)
 *	- Examine the mail and produce a scroll of mail named "From somebody".
 *	- Invoke MAILREADER in such a way that only this single letter is read.
 *	- Do something to the text when the scroll is enchanted or cancelled.
 */

/*
 * { Note by Olaf Seibert: On the Amiga, we usually don't get mail.
 *   So we go through most of the effects at 'random' moments. }
 */

/*
 * I found many bugs in this code, some dating back to Hack.
 * Here are some minor problems i didn't fix:  -3.
 *
 *	- The code sometimes pops up the mail daemon next to you on
 *	  the corridor side of doorways when there are open spaces in
 *	  the room.
 *	- It may also do this with adjoining castle rooms.
 */

# ifndef UNIX
int mustgetmail = -1;
# endif

# ifdef UNIX
static struct stat omstat,nmstat;
static char *mailbox;
static long laststattime;

void
getmailstatus() {
	if(!(mailbox = getenv("MAIL")))
		return;
	if(stat(mailbox, &omstat)){
#  ifdef PERMANENT_MAILBOX
		pline("Cannot get status of MAIL=%s .", mailbox);
		mailbox = 0;
#  else
		omstat.st_mtime = 0;
#  endif
	}
}
# endif /* UNIX */

/* make md run through the cave */
static void
mdrush(md,away)
register struct monst *md;
boolean away;
{
	register int uroom = inroom(u.ux, u.uy);
	if(uroom >= 0 && inroom(md->mx,md->my) == uroom) {
		register int tmp = rooms[uroom].fdoor;
		register int cnt = rooms[uroom].doorct;
		register int fx = u.ux, fy = u.uy;
		while(cnt--) {
			if(dist(fx,fy) < dist(doors[tmp].x, doors[tmp].y)){
				fx = doors[tmp].x;
				fy = doors[tmp].y;
			}
			tmp++;
		}
		if (has_dnstairs(&rooms[uroom]))
			if(dist(fx,fy) < dist(xdnstair, ydnstair)){
				fx = xdnstair;
				fy = ydnstair;
			}
		if (has_upstairs(&rooms[uroom]))
			if(dist(fx,fy) < dist(xupstair, yupstair)){
				fx = xupstair;
				fy = yupstair;
			}
		tmp_at(-1, md->data->mlet);	/* open call */
		tmp_at(-3, (int)AT_MON);
		if(away) {	/* interchange origin and destination */
			unpmon(md);
			levl[md->mx][md->my].mmask = 0;
			levl[fx][fy].mmask = 1;
			tmp = fx; fx = md->mx; md->mx = tmp;
			tmp = fy; fy = md->my; md->my = tmp;
		}
		while(fx != md->mx || fy != md->my) {
			register int dx,dy,nfx = fx,nfy = fy,d1,d2;

			tmp_at(fx,fy);
			d1 = dist2(fx,fy,md->mx,md->my);
			for(dx = -1; dx <= 1; dx++) for(dy = -1; dy <= 1; dy++)
			    if((dx || dy) && 
			       !IS_STWALL(levl[fx+dx][fy+dy].typ)) {
				d2 = dist2(fx+dx,fy+dy,md->mx,md->my);
				if(d2 < d1) {
				    d1 = d2;
				    nfx = fx+dx;
				    nfy = fy+dy;
				}
			    }
			if(nfx != fx || nfy != fy) {
			    fx = nfx;
			    fy = nfy;
			} else {
			    if(!away) {
				levl[md->mx][md->my].mmask = 0;
				levl[fx][fy].mmask = 1;
				md->mx = fx;
				md->my = fy;
			    }
			    break;
			} 
		}
		tmp_at(-1,-1);			/* close call */
	}
	if(!away)
		pmon(md);
}

static void
newmail() {
	/* deliver a scroll of mail */
	register boolean invload =
	((inv_weight() + (int)objects[SCR_MAIL].oc_weight) > 0 ||
	 inv_cnt() >= 52 || Fumbling);
	register struct monst *md = 
	makemon(&mons[PM_MAIL_DAEMON], u.ux, u.uy);

	if(!md)	return;

	mdrush(md,0);

	pline("\"Hello, %s!  I have some mail for you.\"", plname);

	if(dist(md->mx,md->my) > 2)
		verbalize("Catch!");
	more();
	if(invload) {
		struct obj *obj = mksobj_at(SCR_MAIL,u.ux,u.uy);
		obj->known = obj->dknown = TRUE;
		makeknown(SCR_MAIL);
		stackobj(fobj);		
		verbalize("Oops!");
		more();
	} else {
		/* set known and do prinv() */
		(void) identify(addinv(mksobj(SCR_MAIL,FALSE)));
	}

	/* disappear again */
	mdrush(md,1);
	mongone(md);

	/* force the graphics character set off */
	nscr();
}

# ifndef UNIX
void
ckmailstatus() {
	if (mustgetmail < 0)
	    return;
	if (--mustgetmail <= 0) {
		newmail();
		mustgetmail = -1;
	}
}

void
readmail()
{
	pline("It says:  \"Please disregard previous letter.\"");
}

# else /* UNIX */

void
ckmailstatus() {
	if(!mailbox
#  ifdef MAILCKFREQ
		    || moves < laststattime + MAILCKFREQ
#  endif
							)
		return;

	laststattime = moves;
	if(stat(mailbox, &nmstat)){
#  ifdef PERMANENT_MAILBOX
		pline("Cannot get status of MAIL=%s anymore.", mailbox);
		mailbox = 0;
#  else
		nmstat.st_mtime = 0;
#  endif
	} else if(nmstat.st_mtime > omstat.st_mtime) {
		if(nmstat.st_size)
			newmail();
		getmailstatus();	/* might be too late ... */
	}
}

void
readmail() {
#  ifdef DEF_MAILREADER			/* This implies that UNIX is defined */
	register char *mr = 0;
	more();
	if(!(mr = getenv("MAILREADER")))
		mr = DEF_MAILREADER;
	if(child(1)){
		(void) execl(mr, mr, NULL);
		exit(1);
	}
#  else
	(void) page_file(mailbox, FALSE);
#  endif
	/* get new stat; not entirely correct: there is a small time
	   window where we do not see new mail */
	getmailstatus();
}
# endif /* UNIX */

#endif /* MAIL */
