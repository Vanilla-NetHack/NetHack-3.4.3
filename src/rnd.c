/*	SCCS Id: @(#)rnd.c	3.0	87/07/06
 */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"

#if defined(LINT) && defined(UNIX)	/* rand() is long... */
extern int rand();
#define RND(x)	(rand() % x)
#else /* LINT */
/* rand() is either random() or lrand48() - see config.h. */
#ifdef UNIX
#define RND(x)	(int)(Rand() % (long)(x))
#else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x)	(int)((Rand()>>3) % (x))
#endif
#endif /* LINT */

int
rn1(x,y)	/* y <= rn1(x,y) < (y+x) */
register int x, y;
{
	return(RND(x)+y);
}

int
rn2(x)		/* 0 <= rn2(x) < x */
register int x;
{
#ifdef DEBUG
	if (x == 0) {
		impossible("rn2(0) attempted");
		return(0);
	}
#endif
	return(RND(x));
}

int
rnl(x)		/* 0 <= rnl(x) < x; somtimes subtracting Luck */
register int x;	/* good luck approaches 0, bad luck approaches (x-1) */
{
	register int i = RND(x);

	if (Luck && rn2(50 - Luck)) {
	    i -= (x <= 15 && Luck >= -5 ? Luck/3 : Luck);
	    if (i < 0) i = 0;
	    else if (i >= x) i = x-1;
	}

	return i;
}

int
rnd(x)		/* 1 <= rnd(x) <= x */
register int x;
{
#ifdef DEBUG
	if (x == 0) {
		impossible("rnd(0) attempted");
		return(1);
	}
#endif
	return(RND(x)+1);
}

int
d(n,x)		/* n <= d(n,x) <= (n*x) */
register int n, x;
{
	register int tmp = n;

	while(n--) tmp += RND(x);
	return(tmp);
}

int
rne(x)	  /* by stewr 870807 */
register int x;
{
	register int tmp = 1;
	while(!rn2(x)) tmp++;
	return(min(tmp,(u.ulevel < 15) ? 5 : (int)u.ulevel/3));
}

#ifdef THEOLOGY
int
rnz(i)
int i;
{
# ifdef LINT
	int x = i;
	int tmp = 1000;
# else
	register long x = i;
	register long tmp = 1000;
# endif
	tmp += rn2(1000);
	tmp *= rne(4);
	if (rn2(2)) { x *= tmp; x /= 1000; }
	else { x *= 1000; x /= tmp; }
	return((int)x);
}
#endif
