/*	SCCS Id: @(#)rnd.c	2.0	87/09/15
 */
#include	"config.h"
#ifdef UNIX
#define RND(x)	(random() % (x))
#else
/* Good luck: the bottom order bits are cyclic. */
#define RND(x)	((rand()>>3) % (x))
#endif

rn1(x,y)	/* y <= rn1(x,y) < (y+x) */ 
register x,y;
{
	return(RND(x)+y);
}

rn2(x)		/* 0 <= rn2(x) < x */
register x;
{
	return(RND(x));
}

rnd(x)		/* 1 <= rnd(x) <= x */
register x;
{
	return(RND(x)+1);
}

d(n,x)		/* n <= d(n,x) <= (n*x) */
register n,x;
{
	register tmp = n;

	while(n--) tmp += RND(x);
	return(tmp);
}

rne(x)          /* by stewr 870807 */
register x;
{
        register tmp = 1;
	while(!rn2(x)) tmp++;
	return(tmp);
}

rnz(x)
register x;
{
        register tmp = 1000;
	tmp += rn2(1000);
	tmp *= rne(4);
	if (rn2(2)) { x *= tmp; x /= 1000; }
	else { x *= 1000; x /= tmp; }
	return(x);
}
