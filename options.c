/*	SCCS Id: @(#)options.c	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* options.c - version 1.0.3 */

#include "config.h"
#include "hack.h"
extern char *eos();
#ifdef SORTING
static boolean set_order;
#endif

initoptions()
{
	register char *opts;
	extern char *getenv();

	flags.time = flags.nonews = flags.notombstone = flags.end_own =
	flags.standout = flags.nonull = FALSE;
	flags.no_rest_on_space = TRUE;
	flags.invlet_constant = TRUE;
	flags.end_top = 5;
	flags.end_around = 4;
	flags.female = FALSE;			/* players are usually male */
#ifdef SORTING
	flags.sortpack = TRUE;
#endif
#ifdef SAFE_ATTACK
	flags.confirm = TRUE;
#endif
#ifdef DGKMOD
	flags.silent = 	flags.pickup = TRUE;
#endif
#ifdef DGK
	flags.IBMBIOS = flags.DECRainbow = flags.rawio = FALSE;
	read_config_file();
#endif
#ifdef HACKOPTIONS
	if(opts = getenv("HACKOPTIONS"))
		parseoptions(opts,TRUE);
#endif
}

parseoptions(opts, from_env)
register char *opts;
boolean from_env;
{
	register char *op,*op2;
	unsigned num;
	boolean negated;

	if(op = index(opts, ',')) {
		*op++ = 0;
		parseoptions(op, from_env);
	}
	if(op = index(opts, ' ')) {
		op2 = op;
		while(*op++)
			if(*op != ' ') *op2++ = *op;
	}
	if(!*opts) return;
	negated = FALSE;
	while((*opts == '!') || !strncmp(opts, "no", 2)) {
		if(*opts == '!') opts++; else opts += 2;
		negated = !negated;
	}
	
#ifndef DGK
	if(!strncmp(opts,"standout",4)) {
		flags.standout = !negated;
		return;
	}

	if(!strncmp(opts,"null",4)) {
		flags.nonull = negated;
		return;
	}

	if(!strncmp(opts,"tombstone",4)) {
		flags.notombstone = negated;
		return;
	}

	if(!strncmp(opts,"news",4)) {
		flags.nonews = negated;
		return;
	}
#endif

#ifdef SAFE_ATTACK
	if (!strncmp(opts, "conf", 4)) {
		flags.confirm = !negated;
		return;
	}

#endif
#ifdef DGKMOD
	if (!strncmp(opts, "sile", 4)) {
		flags.silent = !negated;
		return;
	}

	if (!strncmp(opts, "pick", 4)) {
		flags.pickup = !negated;
		return;
	}
#endif
#ifdef DGK
	if (!strncmp(opts, "IBMB", 4)) {
		flags.IBMBIOS = !negated;
		return;
	}

	if (!strncmp(opts, "rawi", 4)) {
		if (from_env)
			flags.rawio = !negated;
		else
			pline("'rawio' only settable from %s.", configfile);
		return;
	}

	if (!strncmp(opts, "DECR", 4)) {
		flags.DECRainbow = !negated;
		return;
	}
#endif

#ifdef SORTING
	if (!strncmp(opts, "sort", 4)) {
		flags.sortpack = !negated;
		return;
	}

	/*
	 * the order to list the pack
	 */
	if (!strncmp(opts,"packorder",4)) {
		register char	*sp, *tmp;
		extern char	inv_order[];
		int tmpend;

		op = index(opts,':');
		if(!op) goto bad;
		op++;			/* skip : */

		/* Missing characters in new order are filled in at the end 
		 * from inv_order.
		 */
		for (sp = op; *sp; sp++)
			if (!index(inv_order, *sp))
				goto bad;		/* bad char in order */
			else if (index(sp + 1, *sp))
				goto bad;		/* dup char in order */
		tmp = (char *) alloc(strlen(inv_order) + 1);
		(void) strcpy(tmp, op);
		for (sp = inv_order, tmpend = strlen(tmp); *sp; sp++)
			if (!index(tmp, *sp)) {
				tmp[tmpend++] = *sp;
				tmp[tmpend] = 0;
			}
		(void) strcpy(inv_order, tmp);
		free(tmp);
		set_order = TRUE;
		return;
	}
#endif

	if(!strncmp(opts,"time",4)) {
		flags.time = !negated;
		flags.botl = 1;
		return;
	}

	if(!strncmp(opts,"restonspace",4)) {
		flags.no_rest_on_space = negated;
		return;
	}

#ifndef DGK
	if(!strncmp(opts,"fixinv",4)) {
		if(from_env)
			flags.invlet_constant = !negated;
		else
			pline("The fixinvlet option must be in HACKOPTIONS.");
		return;
	}
#endif

	if(!strncmp(opts,"male",4)) {
#ifdef KAA
		if(!from_env && flags.female != negated)
			pline("That is not anatomically possible.");
		else
#endif
			flags.female = negated;
		return;
	}
	if(!strncmp(opts,"female",6)) {
#ifdef KAA
		if(!from_env && flags.female == negated)
			pline("That is not anatomically possible.");
		else
#endif
			flags.female = !negated;
		return;
	}

	/* name:string */
	if(!strncmp(opts,"name",4)) {
		extern char plname[PL_NSIZ];
		if(!from_env) {
#ifdef DGK
		  pline("'name' only settable from %s.", configfile);
#else
		  pline("The playername can be set only from HACKOPTIONS.");
#endif
		  return;
		}
		op = index(opts,':');
		if(!op) goto bad;
		nmcpy(plname, op+1, sizeof(plname)-1);
		return;
	}

#ifdef GRAPHICS
	/* graphics:string */
	if(!strncmp(opts,"graphics",4)) {
		char buf[MAXPCHARS];
		if(!from_env) {
#ifdef DGK
		  pline("'graphics' only settable from %s.", configfile);
#else
		  pline("The graphics string can be set only from HACKOPTIONS.");
#endif
		  return;
		}
		op = index(opts,':');
		if(!op)
		    goto bad;
		else
		    opts++;
/*
 * You could have problems here if you configure FOUNTAINS, SPIDERS or NEWCLASS
 * in or out and forget to change the tail entries in your graphics string.
 */
#define SETPCHAR(f, n)	showsyms.f = (strlen(opts) > n) ? opts[n] : defsyms.f
		SETPCHAR(stone, 0);
		SETPCHAR(vwall, 1);
		SETPCHAR(hwall, 2);
		SETPCHAR(tlcorn, 3);
		SETPCHAR(trcorn, 4);
		SETPCHAR(blcorn, 5);
		SETPCHAR(brcorn, 6);
		SETPCHAR(door, 7);
		SETPCHAR(room, 8);
		SETPCHAR(corr, 9);
		SETPCHAR(upstair, 10);
		SETPCHAR(dnstair, 11);
		SETPCHAR(trap, 12);
#ifdef FOUNTAINS
		SETPCHAR(pool, 13);
		SETPCHAR(fountain, 14);
#endif
#ifdef NEWCLASS
		SETPCHAR(throne, 15);
#endif
#ifdef SPIDERS
		SETPCHAR(web, 16);
#endif
#undef SETPCHAR
		return;
	}
#endif /* GRAPHICS */

	/* endgame:5t[op] 5a[round] o[wn] */
	if(!strncmp(opts,"endgame",3)) {
		op = index(opts,':');
		if(!op) goto bad;
		op++;
		while(*op) {
			num = 1;
			if(digit(*op)) {
				num = atoi(op);
				while(digit(*op)) op++;
			} else
			if(*op == '!') {
				negated = !negated;
				op++;
			}
			switch(*op) {
			case 't':
				flags.end_top = num;
				break;
			case 'a':
				flags.end_around = num;
				break;
			case 'o':
				flags.end_own = !negated;
				break;
			default:
				goto bad;
			}
			while(letter(*++op)) ;
			if(*op == '/') op++;
		}
		return;
	}
#ifdef	DOGNAME
	if(!strncmp(opts, "dogname", 3)) {
		extern char dogname[];
		op = index(opts, ':');
		if (!op) goto bad;
		nmcpy(dogname, ++op, 62);
		return;
	}
#endif	/* DOGNAME */
bad:
	if(!from_env) {
		if(!strncmp(opts, "help", 4)) {
			pline("%s%s%s",
#ifdef DGK

"To set options use OPTIONS=<options> in ", configfile,
" or give the command \"O\" followed by the line <options> while playing.  ",
"Here <options> is a list of options separated by commas." );
			pline("%s%s",
"Boolean options are confirm, pickup, rawio, silent, sortpack, time, IBMBIOS,",
" and DECRainbow.  These can be negated by prefixing them with '!' or \"no\"." );
			pline("%s%s%s%s",
"The compound options are name, as in OPTIONS=name:Merlin-W,",
#ifdef	DOGNAME
" dogname, which gives the name of your (first) dog (e.g. dogname:Rover)",
#endif	/* DOGNAME */
#ifdef SORTING
" packorder, which lists the order that items should appear in your pack",
#ifdef SPELLS
" (the default is:  packorder:\")[%?+/=!(*0  ), and endgame." );
#else
" (the default is:  packorder:\")[%?/=!(*0  ), and endgame." );
#endif /* SPELLS /**/
#else
#ifdef GRAPHICS
"engame, and graphics.", "", "");
#else
"and engame.", "", "");
#endif
#endif /* SORTING /**/ 	
			pline("%s%s%s",
"Endgame is followed by a description of which parts of the scorelist ",
"you wish to see.  You might for example say: ",
"\"endgame:own scores/5 top scores/4 around my score\"." );

#else

"To set options use `HACKOPTIONS=\"<options>\"' in your environment, or ",
"give the command 'O' followed by the line `<options>' while playing. ",
"Here <options> is a list of <option>s separated by commas." );
			pline("%s%s%s",
"Simple (boolean) options are rest_on_space, news, time, ",
"null, tombstone, (fe)male. ",
"These can be negated by prefixing them with '!' or \"no\"." );
			pline("%s%s%s%s",
"The compound options are name, as in OPTIONS=name:Merlin-W,",
#ifdef	DOGNAME
" dogname, which gives the name of your (first) dog (e.g. dogname:Rover)",
#endif	/* DOGNAME */
#ifdef SORTING
" packorder, which lists the order that items should appear in your pack",
#ifdef SPELLS
" (the default is:  packorder:\")[%?+/=!(*0  ), and endgame." );
#else
" (the default is:  packorder:\")[%?/=!(*0  ), and endgame." );
#endif /* SPELLS /**/
#else
"and engame.", "", "");
#endif /* SORTING /**/ 	
			pline("%s%s%s",
"Endgame is followed by a description of what parts of the scorelist",
"you want to see. You might for example say: ",
"`endgame:own scores/5 top scores/4 around my score'." );

#endif /* DGK /**/
			return;
		}
		pline("Bad option: %s.", opts);
		pline("Type `o help<cr>' for help.");
		return;
	}
#ifdef DGK
	printf("Bad syntax in OPTIONS in %s.", configfile);
#else
	puts("Bad syntax in HACKOPTIONS.");
	puts("Use for example:");
	puts(
"HACKOPTIONS=\"!restonspace,notombstone,endgame:own/5 topscorers/4 around me\""
	);
#endif
	getret();
}

doset()
{
	char buf[BUFSZ];
#ifdef SORTING
	extern char inv_order[];
#endif

	pline("What options do you want to set? ");
	getlin(buf);
	if(!buf[0] || buf[0] == '\033') {
#ifdef DGK
	    (void) strcpy(buf,"OPTIONS=");
#else
	    (void) strcpy(buf,"HACKOPTIONS=");
	    (void) strcat(buf, flags.female ? "female," : "male,");
	    if(flags.standout) (void) strcat(buf,"standout,");
	    if(flags.nonull) (void) strcat(buf,"nonull,");
	    if(flags.nonews) (void) strcat(buf,"nonews,");
	    if(flags.notombstone) (void) strcat(buf,"notombstone,");
	    if(flags.no_rest_on_space)	(void) strcat(buf,"!rest_on_space,");
#endif
#ifdef SORTING
	    if (flags.sortpack) (void) strcat(buf,"sortpack,");
	    if (set_order){
		(void) strcat(buf, "packorder: ");
		(void) strcat(buf, inv_order);
		(void) strcat(buf, ",");
	    }
#endif
#ifdef SAFE_ATTACK
	    if (flags.confirm) (void) strcat(buf,"confirm,");
#endif
#ifdef DGKMOD
	    if (flags.pickup) (void) strcat(buf,"pickup,");
	    if (flags.silent) (void) strcat(buf,"silent,");
#endif
#ifdef DGK
	    if (flags.rawio) (void) strcat(buf,"rawio,");
	    if (flags.IBMBIOS) (void) strcat(buf,"IBMBIOS,");
	    if (flags.DECRainbow) (void) strcat(buf,"DECRainbow,");
#endif
	    if(flags.time) (void) strcat(buf,"time,");
	    if(flags.end_top != 5 || flags.end_around != 4 || flags.end_own){
		(void) sprintf(eos(buf), "endgame: %u topscores/%u around me",
			flags.end_top, flags.end_around);
		if(flags.end_own) (void) strcat(buf, "/own scores");
	    } else {
		register char *eop = eos(buf);
		if(*--eop == ',') *eop = 0;
	    }
	    pline(buf);
	} else
	    parseoptions(buf, FALSE);

	return(0);
}

#ifdef DGKMOD
dotogglepickup() {
	flags.pickup = !flags.pickup;
	pline("Pickup: %s.", flags.pickup ? "ON" : "OFF");
	return (0);
}
#endif

nmcpy(dest, source, maxlen)
	char	*dest, *source;
	int	maxlen;
{
	char	*cs, *cd;
	int	count;

	cd = dest;
	cs = source;
	for(count = 1; count < maxlen; count++) {
		if(*cs == ',') break;
		*cd++ = *cs++;
	}
	*cd = 0;
}
