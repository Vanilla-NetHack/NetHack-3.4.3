/*	SCCS Id: @(#)rnd.c	1.3	87/07/14
/* rnd.c - version 1.0.2 */

#define RND(x)	((rand()>>3) % x)

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
