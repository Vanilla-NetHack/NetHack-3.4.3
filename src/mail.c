/*	SCCS Id: @(#)mail.c	3.0	89/07/07
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */
#define MONATTK_H
#include "hack.h"	/* mainly for index() which depends on BSD */

#ifdef MAIL
static void FDECL(mdrush,(struct monst *,int,int));
static void FDECL(mdappear,(struct monst *,BOOLEAN_P));
static void NDECL(newmail);

# ifdef UNIX
#  include <sys/stat.h>
#  include <pwd.h>
# endif
# ifdef VMS
#  include <descrip.h>
#  include <ssdef.h>
# endif

/*
 * Notify user when new mail has arrived. [Idea from Merlyn Leroy, but
 * I don't know the details of his implementation.]
 * { Later note: he disliked my calling a general mailreader and felt that
 *   hack should do the paging itself. But when I get mail, I want to put it
 *   in some folder, reply, etc. - it would be unreasonable to put all these
 *   functions in hack. }
 *
 * The mail daemon can move with less than usual restraint.  It can:
 *	- move diagonally from a door
 *	- use secret doors
 *	- run thru a monster
 *
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
 *	  the corridor side of doorways when there are open spaces
 *	  within the room.
 *	- It may also do this with adjoining castle rooms.
 */

#ifdef OVL0

# if !defined(UNIX) && !defined(VMS)
int mustgetmail = -1;
# endif

#endif /* OVL0 */
#ifdef OVLB

# ifdef UNIX
extern struct passwd *getpwuid();
static struct stat omstat,nmstat;
static char *mailbox = NULL;
static long laststattime;

# ifdef AMS				/* Just a placeholder for AMS */
#   define MAILPATH "/dev/null"
# else
#  ifdef BSD
#   define MAILPATH "/usr/spool/mail/"
#  endif
#  ifdef SYSV
#   define MAILPATH "/usr/mail/"
#  endif
# endif /* AMS */

void
getmailstatus() {
	if(!mailbox && !(mailbox = getenv("MAIL"))) {
#  ifdef MAILPATH
#   ifdef AMS
	        struct passwd ppasswd;

		bcopy(getpwuid(getuid()), &ppasswd, sizeof(struct passwd));
		if (ppasswd.pw_dir) {
		     mailbox = (char *) alloc((unsigned) strlen(ppasswd.pw_dir)+sizeof(AMS_MAILBOX));
		     Strcpy(mailbox, ppasswd.pw_dir);
		     Strcat(mailbox, AMS_MAILBOX);
		} else
		  return;
#   else
		mailbox = (char *) alloc(sizeof(MAILPATH)+8);
		Strcpy(mailbox, MAILPATH);
		Strcat(mailbox, getpwuid(getuid())->pw_name);
#  endif /* AMS */
#  else
		return;
#  endif
	}
	if(stat(mailbox, &omstat)){
#  ifdef PERMANENT_MAILBOX
		pline("Cannot get status of MAIL=\"%s\".", mailbox);
		mailbox = 0;
#  else
		omstat.st_mtime = 0;
#  endif
	}
}
# endif /* UNIX */

# ifdef VMS
extern unsigned long pasteboard_id;
volatile int broadcasts = 0;
#  define getmailstatus()
# endif /* VMS */


/* make mail daemon run through the dungeon */
static void
mdrush(md,fx,fy)
struct monst *md;
register int fx, fy;	/* origin, where the '&' is displayed */
{
	register int tx = md->mx, ty = md->my;
			/* real location, where the '&' is going */

	tmp_at(-1, md->data->mlet);	/* open call */
#ifdef TEXTCOLOR
	tmp_at(-3, (int)md->data->mcolor);
#endif

	while(fx != tx || fy != ty) {
		register int dx, dy,		/* direction counters */
			     nfx = fx, nfy = fy,/* next location */ 
			     d1, d2;		/* shortest dist, eval */

		/* display the '&' at (fx,fy) */
		tmp_at(fx,fy);

		/* find location next to (fx,fy) closest to (tx,ty) */
		d1 = dist2(fx,fy,tx,ty);
		for(dx = -1; dx <= 1; dx++) for(dy = -1; dy <= 1; dy++)
		    if((dx || dy) && isok(fx+dx,fy+dy) && 
		       !IS_STWALL(levl[fx+dx][fy+dy].typ)) {
			d2 = dist2(fx+dx,fy+dy,tx,ty);
			if(d2 < d1) {
			    d1 = d2;
			    nfx = fx+dx;
			    nfy = fy+dy;
			}
		    }

		/* set (fx,fy) to next location, unless it stopped */
		if(nfx != fx || nfy != fy) {
		    fx = nfx;
		    fy = nfy;
		} else break;
	}

	tmp_at(-1,-1);			/* close call */
}

static void
mdappear(md,away)
struct monst *md;
boolean away;
{
	static int fx, fy;			/* origin */
	int tx = md->mx, ty = md->my;		/* destination */
	register int uroom = inroom(u.ux, u.uy);/* room you're in */

	/* if mail daemon is in same room as you */
	if(uroom >= 0 && inroom(md->mx,md->my) == uroom && (!Blind || Telepat))
		if(away) {
			unpmon(md);
			remove_monster(tx, ty);

			/* fake "real" location to origin */
			md->mx = fx; md->my = fy;

			/* rush from destination */
			mdrush(md,tx,ty);
			return;
		} else {
			/* choose origin */
			register int cnt = rooms[uroom].doorct;
			register int tmp = rooms[uroom].fdoor;
			register int dd = 0;

			/* which door (or staircase) is farthest? */
			while (cnt--) {
				if(dd < dist(doors[tmp].x, doors[tmp].y)) {
					fx = doors[tmp].x;
					fy = doors[tmp].y;
					dd = dist(tx,ty);
				}
				tmp++;
			}
			if (has_dnstairs(&rooms[uroom]))
				if(dd < dist(xdnstair, ydnstair)) {
					fx = xdnstair;
					fy = ydnstair;
					dd = dist(xdnstair, ydnstair);
				}
			if (has_upstairs(&rooms[uroom]))
				if(dd < dist(xupstair, yupstair)) {
					fx = xupstair;
					fy = yupstair;
				}

			/* rush from origin */
			mdrush(md,fx,fy);
		}

	pmon(md);
}

static void
newmail() {
	struct obj *obj;
	/* deliver a scroll of mail */
	register boolean invload =
		((inv_weight() + (int)objects[SCR_MAIL].oc_weight) > 0 ||
			 Fumbling);
	register struct monst *md = makemon(&mons[PM_MAIL_DAEMON], u.ux, u.uy);

	if(!md)	return;

	mdappear(md,FALSE);

# ifdef VMS
	pline("\"Hello, %s!  I have a message for you.\"", plname);
# else
#  ifdef NO_MAILREADER
	pline("\"Hello, %s!  You have some mail in the outside world.\"", plname);
#  else
	pline("\"Hello, %s!  I have some mail for you.\"", plname);
# endif
# endif

# ifndef NO_MAILREADER
	if(dist(md->mx,md->my) > 2)
		verbalize("Catch!");
	more();
	obj = mksobj(SCR_MAIL, FALSE);
	obj->known = obj->dknown = TRUE;
	makeknown(SCR_MAIL);
	if (!invload) obj = addinv(obj);
	if(invload || inv_cnt() > 52) {
		if (invload) dropy(obj);
		else dropx(obj);
		stackobj(fobj);		
		verbalize("Oops!");
	} else {
		int savequan = obj->quan;
		obj->quan = 1;
		prinv(obj);
		obj->quan = savequan;
	}
# endif /* NO_MAILREADER */

	/* disappear again */
	mdappear(md,TRUE);
	mongone(md);

	/* force the graphics character set off */
	nscr();
# ifdef VMS
	broadcasts--;
# endif
}

#endif /* OVLB */

# if !defined(UNIX) && !defined(VMS)

#ifdef OVL0

void
ckmailstatus() {
	if (mustgetmail < 0)
	    return;
	if (--mustgetmail <= 0) {
		newmail();
		mustgetmail = -1;
	}
}

#endif /* OVL0 */
#ifdef OVLB

void
readmail()
{
	pline("It says:  \"Please disregard previous letter.\"");
}

#endif /* OVLB */

# endif /* !UNIX && !VMS */

# ifdef UNIX

#ifdef OVL0

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
		pline("Cannot get status of MAIL=\"%s\" anymore.", mailbox);
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

#endif /* OVL0 */
#ifdef OVLB

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
#   ifndef AMS  			/* AMS mailboxes are directories */
	(void) page_file(mailbox, FALSE);
#   endif /* AMS */
#  endif /* DEF_MAILREADER */

	/* get new stat; not entirely correct: there is a small time
	   window where we do not see new mail */
	getmailstatus();
}

#endif /* OVLB */

# endif /* UNIX */

# ifdef VMS

#ifdef OVL0

void
ckmailstatus()
{
	if (broadcasts)
		newmail();
}

#endif /* OVL0 */
#ifdef OVLB

void
readmail()
{
	char buf[16384];	/* $BRKTHRU limits messages to 16350 bytes */
	$DESCRIPTOR(message, buf);
	short length;

	if (SMG$GET_BROADCAST_MESSAGE(&pasteboard_id, &message, &length)
	    == SS$_NORMAL && length != 0) {
		buf[length] = '\0';
		pline("%s", buf);
	}
}
# endif /* VMS */

#endif /* OVLB */

#endif /* MAIL */
