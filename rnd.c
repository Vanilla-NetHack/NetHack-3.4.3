#define RND(x)	((rand()>>3) % x)

rn1(x,y)
register int x,y;
{
	return(RND(x)+y);
}

rn2(x)
register int x;
{
	return(RND(x));
}

rnd(x)
register int x;
{
	return(RND(x)+1);
}

d(n,x)
register int n,x;
{
	register int tmp = n;

	while(n--) tmp += RND(x);
	return(tmp);
}
