/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include "config.h"
#ifdef OPTIONS
#include "hack.h"

doset()
{
	register flg = 1;
	char buf[BUFSZ];
	register char *str;

	pline("What option do you want to set? [(!)eo] ");
	getlin(buf);
	str = buf;
	while(*str == ' ') str++;
	if(*str == '!') {
		flg = 0;
		str++;
	}
	switch(*str) {
	case 'e':
		flags.echo = flg;
		if(flg) echo(ON);
		else echo(OFF);
		break;
	case 'o':
		flags.oneline = flg;
		break;
	default:
		pline("Unknown option '%s'",str);
	}
 return(0);
}
#endif OPTIONS
