/*	SCCS Id: @(#)options.c	3.0	88/11/09
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
static boolean set_order;

static void nmcpy();

void
initoptions()
{
	register char *opts;

	flags.time = flags.nonews = flags.notombstone = flags.end_own =
	flags.standout = flags.nonull = flags.ignintr = FALSE;
	flags.no_rest_on_space = flags.invlet_constant = TRUE;
	flags.end_top = 5;
	flags.end_around = 4;
	flags.female = FALSE;			/* players are usually male */
	flags.sortpack = TRUE;
	flags.soundok = TRUE;
	flags.verbose = TRUE;
	flags.confirm = TRUE;
	flags.safe_dog = TRUE;
	flags.silent = 	flags.pickup = TRUE;
#ifdef TUTTI_FRUTTI
	nmcpy(pl_fruit, objects[SLIME_MOLD].oc_name, PL_FSIZ);
#endif
	flags.num_pad = FALSE;
#ifdef MSDOS
#ifdef DECRAINBOW
	flags.DECRainbow = FALSE;
#endif
#ifdef DGK
	flags.IBMBIOS =
#ifdef TOS
	TRUE;			/* BIOS might as well always be on for TOS */
#endif
	flags.rawio = FALSE;
#endif
	read_config_file();
#endif /* MSDOS */
	if(opts = getenv("NETHACKOPTIONS"))
		parseoptions(opts,TRUE);
#ifdef TUTTI_FRUTTI
	(void)fruitadd(pl_fruit);
	objects[SLIME_MOLD].oc_name = "\033";
	/* Put something untypable in there */
	/* We cannot just use NULL because that marks the end of objects */
#endif
}

static void
nmcpy(dest, source, maxlen)
	char	*dest, *source;
	int	maxlen;
{
	char	*cs, *cd;
	int	count;

	cd = dest;
	cs = source;
	for(count = 1; count < maxlen; count++) {
		if(*cs == ',' || *cs == '\0') break; /*exit on \0 terminator*/
		*cd++ = *cs++;
	}
	*cd = 0;
}

/*
 * escapes: escape expansion for showsyms. C-style escapes understood include
 * \n, \b, \t, \r, \xnnn (hex), \onnn (octal), \nnn (decimal). The ^-prefix
 * for control characters is also understood, and \[mM] followed by any of the
 * previous forms or by a character has the effect of 'meta'-ing the value (so
 * that the alternate character set will be enabled).
 */
void
escapes(cp, tp)
char	*cp, *tp;
{
    while (*cp)
    {
	int	cval = 0, meta = 0;

	if (*cp == '\\' && index("mM", cp[1])) {
		meta = 1;
		cp += 2;
	}
	if (*cp == '\\' && index("0123456789xXoO", cp[1]))
	{
	    char *dp, *hex = "00112233445566778899aAbBcCdDeEfF";
	    int dcount = 0;

	    cp++;
	    if (*cp == 'x' || *cp == 'X')
		for (++cp; (dp = index(hex, *cp)) && (dcount++ < 2); cp++)
		    cval = (cval * 16) + (dp - hex) / 2;
	    else if (*cp == 'o' || *cp == 'O')
		for (++cp; (index("01234567",*cp)) && (dcount++ < 3); cp++)
		    cval = (cval * 8) + (*cp - '0');
	    else
		for (; (index("0123456789",*cp)) && (dcount++ < 3); cp++)
		    cval = (cval * 10) + (*cp - '0');
	}
	else if (*cp == '\\')		/* C-style character escapes */
	{
	    switch (*++cp)
	    {
	    case '\\': cval = '\\'; break;
	    case 'n': cval = '\n'; break;
	    case 't': cval = '\t'; break;
	    case 'b': cval = '\b'; break;
	    case 'r': cval = '\r'; break;
	    default: cval = *cp;
	    }
	    cp++;
	}
	else if (*cp == '^')		/* expand control-character syntax */
	{
	    cval = (*++cp & 0x1f);
	    cp++;
	}
	else
	    cval = *cp++;
	if (meta)
	    cval |= 0x80;
	*tp++ = cval;
    }
    *tp = '\0';
}

void
parseoptions(opts, from_env)
register char *opts;
boolean from_env;
{
	register char *op;
/*
	register char *op2;
*/
	unsigned num;
	boolean negated;

	if(op = index(opts, ',')) {
		*op++ = 0;
		parseoptions(op, from_env);
	}
/*
	if(op = index(opts, ' ')) {
		op2 = op;
		while(*op++)
			if(*op != ' ') *op2++ = *op;
	}
*/
	if(!*opts) return;
	negated = FALSE;
	while((*opts == '!') || !strncmp(opts, "no", 2)) {
		if(*opts == '!') opts++; else opts += 2;
		negated = !negated;
	}
	
#ifndef MSDOS
	if (!strncmp(opts, "stan", 4)) {
		flags.standout = !negated;
		return;
	}

	if (!strncmp(opts, "null", 4)) {
		flags.nonull = negated;
		return;
	}
#endif

	if (!strncmp(opts, "ign", 3)) {
		flags.ignintr = !negated;
		return;
	}

	if (!strncmp(opts, "tomb", 4)) {
		flags.notombstone = negated;
		return;
	}

#ifdef NEWS
	if (!strncmp(opts, "news", 4)) {
		flags.nonews = negated;
		return;
	}
#endif

	if (!strncmp(opts, "conf", 4)) {
		flags.confirm = !negated;
		return;
	}
	if (!strncmp(opts, "safe", 4)) {
		flags.safe_dog = !negated;
		return;
	}

	if (!strncmp(opts, "sil", 3)) {
		flags.silent = !negated;
		return;
	}

	if (!strncmp(opts, "verb", 4)) {
		flags.verbose = !negated;
		return;
	}

	if (!strncmp(opts, "pick", 4)) {
		flags.pickup = !negated;
		return;
	}

	if (!strncmp(opts, "numb", 4)) {
		flags.num_pad = !negated;
		return;
	}

#ifdef DGK
	if (!strncmp(opts, "IBM", 3)) {
		flags.IBMBIOS = !negated;
		return;
	}

	if (!strncmp(opts, "raw", 3)) {
		if (from_env)
			flags.rawio = !negated;
		else
			pline("\"rawio\" settable only from %s.", configfile);
		return;
	}

#ifdef DECRAINBOW
	if (!strncmp(opts, "DEC", 3)) {
		flags.DECRainbow = !negated;
		return;
	}
#endif /* DECRAINBOW */
#endif

	if (!strncmp(opts, "sort", 4)) {
		flags.sortpack = !negated;
		return;
	}

	/*
	 * the order to list the pack
	 */
	if (!strncmp(opts, "pack", 4)) {
		register char	*sp, *tmp;
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
		tmp = (char *) alloc((unsigned)(strlen(inv_order)+1));
		Strcpy(tmp, op);
		for (sp = inv_order, tmpend = strlen(tmp); *sp; sp++)
			if (!index(tmp, *sp)) {
				tmp[tmpend++] = *sp;
				tmp[tmpend] = 0;
			}
		Strcpy(inv_order, tmp);
		free((genericptr_t)tmp);
		set_order = TRUE;
		return;
	}

	if (!strncmp(opts, "time", 4)) {
		flags.time = !negated;
		flags.botl = 1;
		return;
	}

	if (!strncmp(opts, "rest", 4)) {
		flags.no_rest_on_space = negated;
		return;
	}

	if (!strncmp(opts, "fix", 3)) {
		flags.invlet_constant = !negated;
		if(!from_env && flags.invlet_constant) reassign ();
		return;
	}

	if (!strncmp(opts, "male", 4)) {
		if(!from_env && flags.female != negated)
			pline("That is not anatomically possible.");
		else
			flags.female = negated;
		return;
	}
	if (!strncmp(opts, "fem", 3)) {
		if(!from_env && flags.female == negated)
			pline("That is not anatomically possible.");
		else
			flags.female = !negated;
		return;
	}

	/* name:string */
	if (!strncmp(opts, "name", 4)) {
		if(!from_env) {
#ifdef MSDOS
		  pline("\"name\" settable only from %s.", configfile);
#else
		  pline("The playername can be set only from NETHACKOPTIONS.");
#endif
		  return;
		}
		op = index(opts,':');
		if(!op) goto bad;
		nmcpy(plname, op+1, sizeof(plname)-1);
		return;
	}

	/* graphics:string */
	if (!strncmp(opts, "gr", 2)) {
		if(!from_env) {
#ifdef MSDOS
		  pline("\"graphics\" settable only from %s.", configfile);
#else
		  pline("The graphics string can be set only from NETHACKOPTIONS.");
#endif
		  return;
		}
		op = index(opts,':');
		if(!op)
		    goto bad;
		else
		    opts = op + 1;
		escapes(opts, opts);
#define SETPCHAR(f, n)	showsyms.f = (strlen(opts) > n) ? opts[n] : defsyms.f
		SETPCHAR(stone, 0);
		SETPCHAR(vwall, 1);
		SETPCHAR(hwall, 2);
		SETPCHAR(tlcorn, 3);
		SETPCHAR(trcorn, 4);
		SETPCHAR(blcorn, 5);
		SETPCHAR(brcorn, 6);
		SETPCHAR(crwall, 7);
		SETPCHAR(tuwall, 8);
		SETPCHAR(tdwall, 9);
		SETPCHAR(tlwall, 10);
		SETPCHAR(trwall, 11);
		SETPCHAR(vbeam, 12);
		SETPCHAR(hbeam, 13);
		SETPCHAR(lslant, 14);
		SETPCHAR(rslant, 15);
		SETPCHAR(door, 16);
		SETPCHAR(room, 17);
		SETPCHAR(corr, 18);
		SETPCHAR(upstair, 19);
		SETPCHAR(dnstair, 20);
		SETPCHAR(trap, 21);
		SETPCHAR(web, 22);
		SETPCHAR(pool, 23);
#ifdef FOUNTAINS
		SETPCHAR(fountain, 24);
#endif
#ifdef SINKS
		SETPCHAR(sink, 25);
#endif
#ifdef THRONES
		SETPCHAR(throne, 26);
#endif
#ifdef ALTARS
		SETPCHAR(altar, 27);
#endif
#ifdef STRONGHOLD
		SETPCHAR(upladder, 28);
		SETPCHAR(dnladder, 29);
		SETPCHAR(dbvwall, 30);
		SETPCHAR(dbhwall, 31);
#endif
#undef SETPCHAR
		return;
	}

	/* endgame:5t[op] 5a[round] o[wn] */
	if (!strncmp(opts, "end", 3)) {
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
	if (!strncmp(opts, "dog", 3)) {
		if(!from_env) {
#ifdef MSDOS
		  pline("\"dogname\" settable only from %s.", configfile);
#else
		  Your("dog's name can be set only from NETHACKOPTIONS.");
#endif
		  return;
		}
		op = index(opts, ':');
		if (!op) goto bad;
		nmcpy(dogname, ++op, 62);
		return;
	}
	if (!strncmp(opts, "cat", 3)) {
		if(!from_env) {
#ifdef MSDOS
		  pline("\"catname\" settable only from %s.", configfile);
#else
		  Your("cat's name can be set only from NETHACKOPTIONS.");
#endif
		  return;
		}
		op = index(opts, ':');
		if (!op) goto bad;
		nmcpy(catname, ++op, 62);
		return;
	}
#ifdef TUTTI_FRUTTI
	if (!strncmp(opts, "fr", 2)) {
		op = index(opts, ':');
		if (!op++) goto bad;
		if (!from_env) {
		    struct fruit *f;
		    int numfruits = 0;

		    for(f=ffruit; f; f=f->nextf) {
			if (!strcmp(op, f->fname)) goto goodfruit;
			numfruits++;
		    }
		    if (numfruits >= 100) {
			pline("Doing that so many times isn't very fruitful.");
			return;
		    }
		}
goodfruit:
		nmcpy(pl_fruit, op, PL_FSIZ);
		if (!from_env)
		    (void)fruitadd(pl_fruit);
		/* If from_env, then initoptions is allowed to do it instead
		 * of here (initoptions always has to do it even if there's
		 * no fruit option at all.  Also, we don't want people
		 * setting multiple fruits in their options.)
		 */
		return;
	}
#endif
bad:
	if(!from_env) {
		if(!strncmp(opts, "h", 1) ||
		   !strncmp(opts, "?", 1)) {
			option_help();
			return;
		}
		pline("Unknown option: %s.  Enter \"O?\" for help.", opts);
		return;
	}
#ifdef MSDOS
	Printf("Bad syntax in OPTIONS in %s: %s.", configfile, opts);
#else
	Printf("Bad syntax in NETHACKOPTIONS: %s.", opts);
	(void) puts("Use for example:");
	(void) puts(
"NETHACKOPTIONS=\"!rest_on_space,notombstone,endgame:own/5 topscorers/4 around me\""
	);
#endif
	getret();
}

int
doset()
{
	char buf[BUFSZ];

	pline("What options do you want to set? ");
	getlin(buf);
	if(!buf[0] || buf[0] == '\033') {
#ifdef MSDOS
	    Strcpy(buf,"OPTIONS=");
#ifdef DGK
	    if (flags.rawio) Strcat(buf,"rawio,");
	    if (flags.IBMBIOS) Strcat(buf,"IBM_BIOS,");
#endif /* DGK */
#ifdef DECRAINBOW
	    if (flags.DECRainbow) Strcat(buf,"DEC_Rainbow,");
#endif /* DECRAINBOW */
#else /* MSDOS */
	    Strcpy(buf,"NETHACKOPTIONS=");
	    if(flags.standout) Strcat(buf,"standout,");
	    if(flags.nonull) Strcat(buf,"nonull,");
#endif /* MSDOS */
	    if(flags.ignintr) Strcat(buf,"ignintr,");
	    if(flags.num_pad) Strcat(buf,"number_pad,");
#ifdef NEWS
	    if(flags.nonews) Strcat(buf,"nonews,");
#endif
	    if(flags.notombstone) Strcat(buf,"notombstone,");
	    Strcat(buf, flags.female ? "female," : "male,");
	    if(flags.no_rest_on_space)	Strcat(buf,"!rest_on_space,");
	    if (flags.invlet_constant) Strcat(buf,"fixinv,");
	    if (flags.sortpack) Strcat(buf,"sortpack,");
	    if (set_order){
		Strcat(buf, "packorder: ");
		Strcat(buf, inv_order);
		Strcat(buf, ",");
	    }
	    if (flags.confirm) Strcat(buf,"confirm,");
	    if (flags.safe_dog) Strcat(buf,"safe_pet,");
	    if (flags.pickup) Strcat(buf,"pickup,");
	    if (flags.silent) Strcat(buf,"silent,");
	    if (flags.time) Strcat(buf,"time,");
	    if (flags.verbose) Strcat(buf,"verbose,");
#ifdef TUTTI_FRUTTI
	    Sprintf(eos(buf), "fruit:%s,", pl_fruit);
#endif
	    if(flags.end_top != 5 || flags.end_around != 4 || flags.end_own){
		Sprintf(eos(buf), "endgame: %u top scores/%u around me",
			flags.end_top, flags.end_around);
		if(flags.end_own) Strcat(buf, "/own scores");
	    } else {
		register char *eop = eos(buf);
		if(*--eop == ',') *eop = 0;
	    }
	    pline(buf);
	} else {
	    clrlin();
	    parseoptions(buf, FALSE);
	}

	return 0;
}

int
dotogglepickup() {
	flags.pickup = !flags.pickup;
	pline("Pickup: %s.", flags.pickup ? "ON" : "OFF");
	return 0;
}

#define Page_line(x)	if(page_line(x)) goto quit

void
option_help() {
	char	buf[BUFSZ];

	set_pager(0);
	Sprintf(buf, "                 NetHack Options Help:");
	if(page_line("") || page_line(buf) || page_line(""))	 goto quit;

#ifdef MSDOS
	Sprintf(buf, "To set options use OPTIONS=<options> in %s;", configfile);
	Page_line(buf);
#else
	Page_line("To set options use `NETHACKOPTIONS=\"<options>\"' in your environment;");
#endif

	Page_line("or press \"O\" while playing, and type your <options> at the prompt.");
	Page_line("In either case, <options> is a list of options separated by commas.");
	Page_line("");

	Page_line("Boolean options (which can be negated by prefixing them with '!' or \"no\"):");
	Page_line("confirm, (fe)male, fixinv, pickup, rest_on_space, safe_pet, silent, sortpack,");
#ifdef MSDOS
#ifdef NEWS
	Page_line("time, tombstone, verbose, news, number_pad, rawio, and IBM_BIOS");
#else
	Page_line("time, tombstone, verbose, number_pad, rawio, and IBM_BIOS");
#endif
#ifdef DECRAINBOW
	Page_line("and DEC_Rainbow.");
#endif /* DECRAINBOW */
#else /* MSDOS */
#ifdef NEWS
	Page_line("time, tombstone, verbose, news, null, ignintr, and standout.");
#else
	Page_line("time, tombstone, verbose, null, ignintr, and standout.");
#endif
#endif /* MSDOS */
	Page_line("");

	Page_line("Compound options:");
	Page_line("`name'      - your character's name (e.g., name:Merlin-W),");
	Page_line("`dogname'   - the name of your (first) dog (e.g., dogname:Fang),");

	Page_line("`packorder' - the inventory order of the items in your pack");
	Sprintf(buf, "              (currently, packorder:%s ),", inv_order);
	Page_line(buf);
#ifdef TUTTI_FRUTTI
	Page_line("`fruit'     - the name of a fruit you enjoy eating,");
#endif

	Page_line("`endgame'   - the parts of the score list you wish to see,");

	Page_line("`graphics'  - defines the symbols to use in drawing the dungeon map.");
	Page_line("");
	Page_line("Some of the options can be set only before the game is started.  You will");
	Page_line("be so informed, if you attempt to set them while in the game.");
	set_pager(1);
	return;
quit:
	set_pager(2);
	return;
}

#ifdef TUTTI_FRUTTI
/* Returns the fid of the fruit type; if that type already exists, it
 * returns the fid of that one; if it does not exist, it adds a new fruit
 * type to the chain and returns the new one.
 */
int
fruitadd(str)
char *str;
{
	register int i,j;
	register struct fruit *f;
	struct fruit *lastf;
	int highest_fruit_id = 0;
	char buf[PL_FSIZ];
	boolean user_specified = (str == pl_fruit);
	/* if not user-specified, then it's a fruit name for a fruit on
	 * a bones level...
	 */

	/* Note: every fruit has an id (spe for fruit objects) of at least
	 * 1; 0 is an error.
	 */
	if (user_specified) {
		/* disallow naming after other foods (since it'd be impossible
		 * to tell the difference)
		 */

		boolean found = FALSE;

		for(i = bases[j=letindex(FOOD_SYM)]; i < bases[j+1]; i++) {
			if (!strcmp(objects[i].oc_name, pl_fruit)) {
				found = TRUE;
				break;
			}
		}
		if (found ||
		    (!strncmp(buf, "tin of ", 7) && name_to_mon(buf+7) > -1) ||
		    !strcmp(buf, "empty tin") ||
		    !strcmp(buf, "tin of spinach") ||
		    ((!strncmp(eos(buf)-6," corpse",7) ||
						!strncmp(eos(buf)-3, " egg",4))
			&& name_to_mon(buf) > -1))
			{
				Strcpy(buf, pl_fruit);
				Strcpy(pl_fruit, "candied ");
				nmcpy(pl_fruit+8, buf, PL_FSIZ-8);
		}
	}
	for(f=ffruit; f; f = f->nextf) {
		lastf = f;
		if(f->fid > highest_fruit_id) highest_fruit_id = f->fid;
		if(!strncmp(str, f->fname, PL_FSIZ))
			goto nonew;
	}
	/* if adding another fruit would overflow spe, use a random
	   fruit instead... we've got a lot to choose from. */
	if (highest_fruit_id >= 127) return rnd(127);
	highest_fruit_id++;
	f = newfruit();
	if (ffruit) lastf->nextf = f;
	else ffruit = f;
	Strcpy(f->fname, str);
	f->fid = highest_fruit_id;
	f->nextf = 0;
nonew:
	if (user_specified) current_fruit = highest_fruit_id;
	return f->fid;
}
#endif
