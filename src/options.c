/*	SCCS Id: @(#)options.c	3.1	93/06/27	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef OPTION_LISTS_ONLY	/* want option lists for external program */
#include "config.h"
#include "objclass.h"
#include "flag.h"
NEARDATA struct flag flags;	/* provide linkage */
#define static
#else
#include "hack.h"
#include "termcap.h"
#include <ctype.h>
#endif

/*
 *  NOTE:  If you add (or delete) an option, please update the short
 *  options help (option_help()), the long options help (dat/opthelp),
 *  and the current options setting display function (doset()),
 *  and also the Guidebooks.
 */

static struct Bool_Opt
{
	const char *name;
	boolean	*addr, initvalue;
} boolopt[] = {
#ifdef MFLOPPY
	{"asksavedisk", &flags.asksavedisk, FALSE},
#else
	{"asksavedisk", (boolean *)0, FALSE},
#endif
	{"autopickup", &flags.pickup, TRUE},
#if defined(MICRO) && !defined(AMIGA)
	{"BIOS", &flags.BIOS, FALSE},
#else
	{"BIOS", (boolean *)0, FALSE},
#endif
#ifdef INSURANCE
	{"checkpoint", &flags.ins_chkpt, TRUE},
#else
	{"checkpoint", (boolean *)0, FALSE},
#endif
#ifdef TEXTCOLOR
# ifdef MICRO
	{"color", &flags.use_color, TRUE},
# else	/* systems that support multiple terminals, many monochrome */
	{"color", &flags.use_color, FALSE},
# endif
#else
	{"color", (boolean *)0, FALSE},
#endif
	{"confirm",&flags.confirm, TRUE},
#ifdef TERMLIB
	{"DECgraphics", &flags.DECgraphics, FALSE},
#else
	{"DECgraphics", (boolean *)0, FALSE},
#endif
	{"female", &flags.female, FALSE},
	{"fixinv", &flags.invlet_constant, TRUE},
#ifdef AMIFLUSH
	{"flush", &flags.amiflush, FALSE},
#else
	{"flush", (boolean *)0, FALSE},
#endif
	{"help", &flags.help, TRUE},
#ifdef TEXTCOLOR
	{"hilite_pet", &flags.hilite_pet, FALSE},
#else
	{"hilite_pet", (boolean *)0, FALSE},
#endif
#ifdef ASCIIGRAPH
	{"IBMgraphics", &flags.IBMgraphics, FALSE},
#else
	{"IBMgraphics", (boolean *)0, FALSE},
#endif
	{"ignintr", &flags.ignintr, FALSE},
#ifdef MAC_GRAPHICS_ENV
	{"large_font", &flags.large_font, FALSE},
#else
	{"large_font", (boolean *)0, FALSE},
#endif
	{"legacy",&flags.legacy, TRUE},
	{"lit_corridor", &flags.lit_corridor, FALSE},
#ifdef MAC_GRAPHICS_ENV
	{"Macgraphics", &flags.MACgraphics, TRUE},
#else
	{"Macgraphics", (boolean *)0, FALSE},
#endif
#ifdef NEWS
	{"news", &flags.news, TRUE},
#else
	{"news", (boolean *)0, FALSE},
#endif
	{"null", &flags.null, TRUE},
	{"number_pad", &flags.num_pad, FALSE},
#ifdef MAC
	{"page_wait", &flags.page_wait, TRUE},
#else
	{"page_wait", (boolean *)0, FALSE},
#endif
#ifdef MAC
	{"popup_dialog", &flags.popup_dialog, FALSE},
#else
	{"popup_dialog", (boolean *)0, FALSE},
#endif
#if defined(MICRO) && !defined(AMIGA)
	{"rawio", &flags.rawio, FALSE},
#else
	{"rawio", (boolean *)0, FALSE},
#endif
	{"rest_on_space", &flags.rest_on_space, FALSE},
	{"safe_pet", &flags.safe_dog, TRUE},
#ifdef EXP_ON_BOTL
	{"showexp", &flags.showexp, FALSE},
#else
	{"showexp", (boolean *)0, FALSE},
#endif
#ifdef SCORE_ON_BOTL
	{"showscore", &flags.showscore, FALSE},
#else
	{"showscore", (boolean *)0, FALSE},
#endif
	{"silent", &flags.silent, TRUE},
	{"sortpack", &flags.sortpack, TRUE},
	{"sound", &flags.soundok, TRUE},
	{"standout", &flags.standout, FALSE},
	{"time", &flags.time, FALSE},
	{"tombstone",&flags.tombstone, TRUE},
	{"verbose", &flags.verbose, TRUE},
	{NULL, (boolean *)0, FALSE}
};

/* compound options, for option_help() and external programs like Amiga
 * frontend */
static struct Comp_Opt
{
	const char *name, *descr;
} compopt[] = {
	{ "catname",  "the name of your (first) cat (e.g., catname:Tabby)," },
	{ "disclose", "the kinds of information to disclose at end of game," },
	{ "dogname",  "the name of your (first) dog (e.g., dogname:Fang)," },
#ifdef TUTTI_FRUTTI
	{ "fruit",    "the name of a fruit you enjoy eating," },
#endif
	{ "graphics", "the symbols to use in drawing the dungeon map," },
	{ "monsters", "the symbols to use for monsters," },
	{ "msghistory", "number of top line messages to save," },
	{ "name",     "your character's name (e.g., name:Merlin-W)," },
	{ "objects",  "the symbols to use for objects," },
	{ "packorder", "the inventory order of the items in your pack," },
#ifdef CHANGE_COLOR
	{ "palette",  "palette (00c/880/-fff is blue/yellow/reverse white)," },
# if defined(MAC)
	{ "hicolor",  "same as palette, only order is reversed," },
# endif
#endif
	{ "pettype",  "your preferred initial pet type," },
	{ "pickup_types", "types of objects to pick up automatically," },
	{ "scores",   "the parts of the score list you wish to see," },
#ifdef VIDEOSHADES
	{ "videocolors", "color mappings for internal screen routines," },
	{ "videoshades", "gray shades to map to black/gray/white," },
#endif
	{ "windowtype", "windowing system to use." },
	{ NULL, NULL }
};

#ifndef OPTION_LISTS_ONLY	/* use rest of file */

static boolean need_redraw; /* for doset() */

#if defined(TOS) && defined(TEXTCOLOR)
extern boolean colors_changed;	/* in tos.c */
#endif

#ifdef VIDEOSHADES
extern char *shade[3];		  /* in sys/msdos/video.c */
extern char ttycolors[MAXCOLORS]; /* in sys/msdos/video.c */
#endif

extern const char *roles[];	/* from u_init.c */

static char def_inv_order[MAXOCLASSES] = {
	AMULET_CLASS, WEAPON_CLASS, ARMOR_CLASS, FOOD_CLASS, SCROLL_CLASS,
	SPBOOK_CLASS, POTION_CLASS, RING_CLASS, WAND_CLASS, TOOL_CLASS, 
	GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, 0,
};

static boolean initial, from_file;

static void FDECL(nmcpy, (char *, const char *, int));
static void FDECL(escapes, (const char *, char *));
static void FDECL(rejectoption, (const char *));
static void FDECL(badoption, (const char *));
static char *FDECL(string_for_opt, (char *,BOOLEAN_P));
static char *FDECL(string_for_env_opt, (const char *, char *,BOOLEAN_P));
static int FDECL(change_inv_order, (char *));
static void FDECL(oc_to_str, (char *, char *));

void
initoptions()
{
	register char *opts;
	int i;

	for (i = 0; boolopt[i].name; i++) {
		if (boolopt[i].addr)
			*(boolopt[i].addr) = boolopt[i].initvalue;
	}
	flags.end_own = FALSE;
	flags.end_top = 3;
	flags.end_around = 2;
	flags.msg_history = 20;

	/* Set the default monster and object class symbols.  Don't use */
	/* memcpy() --- sizeof char != sizeof uchar on some machines.	*/
	for (i = 0; i < MAXOCLASSES; i++)
		oc_syms[i] = (uchar) def_oc_syms[i];
	for (i = 0; i < MAXMCLASSES; i++)
		monsyms[i] = (uchar) def_monsyms[i];

     /* assert( sizeof flags.inv_order == sizeof def_inv_order ); */
	(void)memcpy((genericptr_t)flags.inv_order,
		     (genericptr_t)def_inv_order, sizeof flags.inv_order);
	flags.pickup_types[0] = '\0';

	switch_graphics(ASCII_GRAPHICS);	/* set default characters */
#if defined(UNIX) && defined(TTY_GRAPHICS)
	/*
	 * Set defaults for some options depending on what we can
	 * detect about the environment's capabilities.
	 * This has to be done after the global initialization above
	 * and before reading user-specific initialization via
	 * config file/environment variable below.
	 */
	/* this detects the IBM-compatible console on most 386 boxes */
	if (!strncmp(getenv("TERM"), "AT", 2)) {
		switch_graphics(IBM_GRAPHICS);
# ifdef TEXTCOLOR
		flags.use_color = TRUE;
# endif
	}
#endif /* UNIX && TTY_GRAPHICS */
#if defined(UNIX) || defined(VMS)
# ifdef TTY_GRAPHICS
	/* detect whether a "vt" terminal can handle alternate charsets */
	if (!strncmpi(getenv("TERM"), "vt", 2) && (AS && AE) &&
	    index(AS, '\016') && index(AE, '\017')) {
		switch_graphics(DEC_GRAPHICS);
	}
# endif
#endif /* UNIX || VMS */

#ifdef MAC_GRAPHICS_ENV
	switch_graphics(MAC_GRAPHICS);
#endif /* MAC_GRAPHICS_ENV */

#ifdef TUTTI_FRUTTI
	/* since this is done before init_objects(), do partial init here */
	objects[SLIME_MOLD].oc_name_idx = SLIME_MOLD;
	nmcpy(pl_fruit, OBJ_NAME(objects[SLIME_MOLD]), PL_FSIZ);
#endif
	opts = getenv("NETHACKOPTIONS");
	if (!opts) opts = getenv("HACKOPTIONS");
	if (opts) {
		if (*opts == '/' || *opts == '\\' || *opts == '@') {
			if (*opts == '@') opts++;	/* @filename */
			/* looks like a filename */
			read_config_file(opts);
		} else {
			read_config_file(NULL);
			parseoptions(opts, TRUE, FALSE);
		}
	} else {
		read_config_file(NULL);
	}
#ifdef AMIGA
	ami_wbench_init();	/* must be here or can't set fruit */
#endif
#ifdef TUTTI_FRUTTI
	(void)fruitadd(pl_fruit);
	/* Remove "slime mold" from list of object names; this will	*/
	/* prevent it from being wished unless it's actually present	*/
	/* as a named (or default) fruit.  Wishing for "fruit" will	*/
	/* result in the player's preferred fruit [better than "\033"].	*/
	obj_descr[SLIME_MOLD].oc_name = "fruit";
#endif
	if (flags.female)  {	/* should have been set in NETHACKOPTIONS */
		roles[2] = "Cavewoman";
		roles[6] = "Priestess";
	}
}

static void
nmcpy(dest, src, maxlen)
	char	*dest;
	const char *src;
	int	maxlen;
{
	int	count;

	for(count = 1; count < maxlen; count++) {
		if(*src == ',' || *src == '\0') break; /*exit on \0 terminator*/
		*dest++ = *src++;
	}
	*dest = 0;
}

/*
 * escapes: escape expansion for showsyms. C-style escapes understood include
 * \n, \b, \t, \r, \xnnn (hex), \onnn (octal), \nnn (decimal). The ^-prefix
 * for control characters is also understood, and \[mM] followed by any of the
 * previous forms or by a character has the effect of 'meta'-ing the value (so
 * that the alternate character set will be enabled).
 */
static void
escapes(cp, tp)
const char	*cp;
char *tp;
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
	    const char *dp, *hex = "00112233445566778899aAbBcCdDeEfF";
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

static void
rejectoption(optname)
const char *optname;
{
#ifdef MICRO
# ifdef AMIGA
	if(FromWBench){
		pline("\"%s\" settable only from %s or in icon.",
			optname, configfile);
	} else
# endif
		pline("\"%s\" settable only from %s.", optname, configfile);
#else
	pline("%s can be set only from NETHACKOPTIONS or %s.", optname,
			configfile);
#endif
}

static void
badoption(opts)
const char *opts;
{
	if (!initial) {
	    if (!strncmp(opts, "h", 1) || !strncmp(opts, "?", 1))
		option_help();
	    else
		pline("Bad syntax: %s.  Enter \"?g\" for help.", opts);
	    return;
	}
# ifdef AMIGA
	if(ami_wbench_badopt(opts)) {
# endif
	if(from_file)
	    raw_printf("Bad syntax in OPTIONS in %s: %s.", configfile, opts);
	else
	    raw_printf("Bad syntax in NETHACKOPTIONS: %s.", opts);
# ifdef AMIGA
	}
# endif
	wait_synch();
}

static char *
string_for_opt(opts, val_optional)
char *opts;
boolean val_optional;
{
	register char *colon;

	colon = index(opts, ':');
	if(!colon) {
		if (!val_optional) badoption(opts);
		return NULL;
	}
	return ++colon;
}

static char *
string_for_env_opt(optname, opts, val_optional)
const char *optname;
char *opts;
boolean val_optional;
{
	if(!initial) {
		rejectoption(optname);
		return NULL;
	}
	return string_for_opt(opts, val_optional);
}

/*
 * Change the inventory order, using the given string as the new order.
 * Missing characters in the new order are filled in at the end from
 * the current inv_order.
 *
 * This routine returns 1 unless there is a duplicate or bad char in
 * the string.
 */
static int
change_inv_order(op)
char *op;
{
    int oc_sym, num;
    char *sp, buf[BUFSZ];

    for (sp = op; *sp; sp++) {
	oc_sym = def_char_to_objclass(*sp);

	/* Remove bad or duplicate entries. */
	if (oc_sym == MAXOCLASSES ||
		(!index(flags.inv_order, oc_sym)) || (index(sp+1, *sp)))

	    return 0;

	*sp = (char) oc_sym;
    } 
    Strcpy(buf, op);
    for (sp = flags.inv_order, num = strlen(buf); *sp; sp++)
	if (!index(buf, *sp)) {
	    buf[num++] = *sp;
	    buf[num] = '\0';	/* explicitly terminate for next index() */
	}

    buf[num] = 0;
    Strcpy(flags.inv_order, buf);
    return 1;
}

void
parseoptions(opts, tinitial, tfrom_file)
register char *opts;
boolean tinitial, tfrom_file;
{
	register char *op;
	unsigned num;
	boolean negated;
	int i;

	initial = tinitial;
	from_file = tfrom_file;
	if ((op = index(opts, ',')) != 0) {
		*op++ = 0;
		parseoptions(op, initial, from_file);
	}

	/* strip leading and trailing white space */
	while (isspace(*opts)) opts++;
	op = eos(opts);
	while (--op >= opts && isspace(*op)) *op = '\0';

	if (!*opts) return;
	negated = FALSE;
	while ((*opts == '!') || !strncmpi(opts, "no", 2)) {
		if (*opts == '!') opts++; else opts += 2;
		negated = !negated;
	}
	
	/* special boolean options */

	if (!strncmpi(opts, "female", 3)) {
		if(!initial && flags.female == negated)
			pline("That is not anatomically possible.");
		else
			flags.female = !negated;
		return;
	}

	if (!strncmpi(opts, "male", 4)) {
		if(!initial && flags.female != negated)
			pline("That is not anatomically possible.");
		else
			flags.female = negated;
		return;
	}

#if defined(MICRO) && !defined(AMIGA)
	/* included for compatibility with old NetHack.cnf files */
	if (!strncmp(opts, "IBM_", 4)) {
		flags.BIOS = !negated;
		return;
	}
#endif /* MICRO */

	/* compound options */

	if (!strncmpi(opts, "pettype", 3)) {
		if ((op = string_for_env_opt("pettype", opts, FALSE)) != 0)
		    switch (*op) {
			case 'd':	/* dog */
			case 'D':
			    preferred_pet = 'd';
			    break;
			case 'c':	/* cat */
			case 'C':
			case 'f':	/* feline */
			case 'F':
			    preferred_pet = 'c';
			    break;
			default:
			    pline("Unrecognized pet type '%s'", op);
			    break;
		    }
		return;
	}

	if (!strncmpi(opts, "catname", 3)) {
		if ((op = string_for_env_opt("catname", opts, FALSE)) != 0)
			nmcpy(catname, op, 62);
		return;
	}

	if (!strncmpi(opts, "dogname", 3)) {
		if ((op = string_for_env_opt("dogname", opts, FALSE)) != 0)
			nmcpy(dogname, op, 62);
		return;
	}

	if (!strncmpi(opts, "msghistory", 3)) {
		if ((op = string_for_env_opt("msghistory", opts, FALSE)) != 0) {
			flags.msg_history = atoi(op);
		}
		return;
	}

#ifdef CHANGE_COLOR
	if (!strncmpi(opts, "palette", 3)
# ifdef MAC
					|| !strncmpi(opts, "hicolor", 3)
# endif
									) {
	    int color_number, color_incr;
# ifdef MAC
	    if (!strncmpi(opts, "hicolor", 3)) {
		color_number = MAXCOLORS + 4;	/* HARDCODED inverse number */
		color_incr = -1;
	    } else {
# endif
		color_number = 0;
		color_incr = 1;
# ifdef MAC
	    }
# endif
	    if ((op = string_for_opt(opts, FALSE)) != NULL) {
		char *pt = op;
		int cnt, tmp, reverse;
		long rgb;

		while (*pt && color_number >= 0) {
		    cnt = 3;
		    rgb = 0L;
		    if (*pt == '-') {
			reverse = 1;
			pt++;
		    } else {
			reverse = 0;
		    }
		    while (cnt-- > 0) {
			if (*pt && *pt != '/') {
# ifdef AMIGA
			    rgb <<= 4;
# else
			    rgb <<= 8;
# endif
			    tmp = *(pt++);
			    if (isalpha(tmp)) {
				tmp = (tmp + 9) & 0xf;	/* Assumes ASCII... */
			    } else {
				tmp &= 0xf;	/* Digits in ASCII too... */
			    }
# ifndef AMIGA
			    /* Add an extra so we fill f -> ff and 0 -> 00 */
			    rgb += tmp << 4;
# endif
			    rgb += tmp;
			}
		    }
		    if (*pt == '/') {
			pt++;
		    }
		    change_color(color_number, rgb, reverse);
		    color_number += color_incr;
		}
	    }
	    if (!initial) {
		need_redraw = TRUE;
	    }
	    return;
	}
#endif

#ifdef TUTTI_FRUTTI
	if (!strncmpi(opts, "fruit", 2)) {
		if (!(op = string_for_opt(opts, FALSE))) return;
		if (!initial) {
		    struct fruit *f;

		    num = 0;
		    for(f=ffruit; f; f=f->nextf) {
			if (!strcmp(op, f->fname)) goto goodfruit;
			num++;
		    }
		    if (num >= 100) {
			pline("Doing that so many times isn't very fruitful.");
			return;
		    }
		}
goodfruit:
		nmcpy(pl_fruit, op, PL_FSIZ);
		if (!*pl_fruit)
		    nmcpy(pl_fruit, OBJ_NAME(objects[SLIME_MOLD]), PL_FSIZ);
		if (!initial)
		    (void)fruitadd(pl_fruit);
		/* If initial, then initoptions is allowed to do it instead
		 * of here (initoptions always has to do it even if there's
		 * no fruit option at all.  Also, we don't want people
		 * setting multiple fruits in their options.)
		 */
		return;
	}
#endif
	/* graphics:string */
	if (!strncmpi(opts, "graphics", 2)) {
		uchar translate[MAXPCHARS+1];
		int length;

		if (!(opts = string_for_env_opt("graphics", opts, FALSE)))
			return;
		escapes(opts, opts);

		length = strlen(opts);
		if (length > MAXPCHARS) length = MAXPCHARS;
		/* match the form obtained from PC configuration files */
		for (i = 0; i < length; i++)
			translate[i] = (uchar) opts[i];
		assign_graphics(translate, length);
		return;
	}

	/* objects:string */
	if (!strncmpi(opts, "objects", 7)) {
		int length;

		if (!(opts = string_for_env_opt("objects", opts, FALSE)))
			return;
		escapes(opts, opts);

		/*
		 * Override the default object class symbols.  The first
		 * object in the object class is the "random object".  I
		 * don't want to use 0 as an object class, so the "random
		 * object" is basically a place holder.
		 *
		 * The object class symbols have already been initialized in
		 * initoptions().
		 */
		length = strlen(opts);
		if (length >= MAXOCLASSES)
		    length = MAXOCLASSES-1;	/* don't count RANDOM_OBJECT */

		for (i = 0; i < length; i++)
		    oc_syms[i+1] = (uchar) opts[i];
		return;
	}

	/* monsters:string */
	if (!strncmpi(opts, "monsters", 8)) {
		int length;

		if (!(opts = string_for_env_opt("monsters", opts, FALSE)))
			return;
		escapes(opts, opts);

		/* Override default mon class symbols set in initoptions(). */
		length = strlen(opts);
		if (length >= MAXMCLASSES)
		    length = MAXMCLASSES-1;	/* mon class 0 unused */

		for (i = 0; i < length; i++)
		    monsyms[i+1] = (uchar) opts[i];
		return;
	}

	/* name:string */
	if (!strncmpi(opts, "name", 4)) {
		if ((op = string_for_env_opt("name", opts, FALSE)) != 0)
			nmcpy(plname, op, (int)sizeof(plname)-1);
		return;
	}

	/* the order to list the pack */
	if (!strncmpi(opts, "packorder", 4)) {
		if (!(op = string_for_opt(opts, FALSE))) return;

		if (!change_inv_order(op))
			badoption(opts);
		return;
	}

	/* types of objects to pick up automatically */
	if (!strncmpi(opts, "pickup_types", 4)) {
		int oc_sym;
		boolean badopt = FALSE, compat = (strlen(opts) <= 6);

		flags.pickup_types[0] = '\0';	/* all */
		if (!(op = string_for_opt(opts, compat))) {
		    /* for backwards compatibility, "pickup" without a value
		       is a synonym for boolean autopickup, and pickup_types
		       gets reset to "all"				    */
		    flags.pickup = !negated;
		    return;
		}

		while (*op == ' ') op++;
		if (*op != 'a' && *op != 'A') {
		    num = 0;
		    while (*op) {
			oc_sym = def_char_to_objclass(*op);
			/* make sure all are valid obj symbols occuring once */
			if (oc_sym != MAXOCLASSES &&
			    !index(flags.pickup_types, oc_sym)) {
			    flags.pickup_types[num] = (char)oc_sym;
			    flags.pickup_types[++num] = '\0';
			} else
			    badopt = TRUE;
			op++;
		    }
		    if (badopt) badoption(opts);
		}
		return;
	}

	/* things to disclose at end of game */
	if (!strncmpi(opts, "disclose", 4)) {
		flags.end_disclose[0] = '\0';	/* all */
		if (!(op = string_for_opt(opts, TRUE))) {
			/* for backwards compatibility, "disclose" without a
			 * value means all (was inventory and attributes,
			 * the only things available then), but negated
			 * it means "none"
			 * (note "none" contains none of "iavkg")
			 */
			if (negated) Strcpy(flags.end_disclose, "none");
			return;
		}
		num = 0;
		while (*op && num < sizeof flags.end_disclose - 1) {
			register char c;
			c = lowc(*op);
			if (c == 'k') c = 'v';	/* killed -> vanquished */
			if (!index(flags.end_disclose, c)) {
				flags.end_disclose[num++] = c;
				flags.end_disclose[num] = '\0';	/* for index */
			}
			op++;
		}
		return;
	}

	/* scores:5t[op] 5a[round] o[wn] */
	if (!strncmpi(opts, "scores", 6)) {
		if (!(op = string_for_opt(opts, FALSE))) return;

		while (*op) {
			num = 1;
			if(digit(*op)) {
				num = atoi(op);
				while(digit(*op)) op++;
			} else if(*op == '!') {
				negated = !negated;
				op++;
			}
			while(*op == ' ') op++;

			switch(*op) {
				case 't':
				case 'T':
					flags.end_top = num;
					break;
				case 'a':
				case 'A':
					flags.end_around = num;
					break;
				case 'o':
				case 'O':
					flags.end_own = !negated;
					break;
				default:
					badoption(opts);
					return;
			}
			while(letter(*++op) || *op == ' ') ;
			if(*op == '/') op++;
		}
		return;
	}

#ifdef VIDEOSHADES
	/* videocolors:string */
	if (!strncmpi(opts, "videocolors", 6)) {
		int length;

		if (!(opts = string_for_env_opt("videocolors", opts, FALSE))) {
			return;
		}
		badoption(opts);
		return;
	}
	/* videoshades:string */
	if (!strncmpi(opts, "videoshades", 6)) {
		int length;

		if (!(opts = string_for_env_opt("videoshades", opts, FALSE))) {
			return;
		}
		badoption(opts);
		return;
	}
#endif /* VIDEOSHADES */

	if (!strncmpi(opts, "windowtype", 3)) {
	    if ((op = string_for_env_opt("windowtype", opts, FALSE)) != 0) {
		char buf[16];
		nmcpy(buf, op, 15);
		choose_windows(buf);
	    }
	    return;
	}

	/* OK, if we still haven't recognized the option, check the boolean
	 * options list
	 */
	for (i = 0; boolopt[i].name; i++) {
		if (strlen(opts) >= 3 &&
		    !strncmpi(boolopt[i].name, opts, strlen(opts))) {
			/* options that don't exist */
			if (!boolopt[i].addr) {
			    if (!initial && !negated)
				pline("The \"%s\" option is not available.",
					boolopt[i].name);
			    return;
			}
			/* options that must come from config file */
			if (!initial &&
			    ((boolopt[i].addr) == &flags.legacy
#if defined(MICRO) && !defined(AMIGA)
			  || (boolopt[i].addr) == &flags.rawio
#endif
			     )) {
			    rejectoption(boolopt[i].name);
			    return;
			}

			*(boolopt[i].addr) = !negated;

#if defined(TERMLIB) || defined(ASCIIGRAPH) || defined(MAC_GRAPHICS_ENV)
			if (FALSE
# ifdef TERMLIB
				 || (boolopt[i].addr) == &flags.DECgraphics
# endif
# ifdef ASCIIGRAPH
				 || (boolopt[i].addr) == &flags.IBMgraphics
# endif
# ifdef MAC_GRAPHICS_ENV
				 || (boolopt[i].addr) == &flags.MACgraphics
# endif
				) {
# ifdef REINCARNATION
			    if (!initial && Is_rogue_level(&u.uz))
				assign_rogue_graphics(FALSE);
# endif
			    need_redraw = TRUE;
# ifdef TERMLIB
			    if ((boolopt[i].addr) == &flags.DECgraphics)
				switch_graphics(flags.DECgraphics ?
						DEC_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef ASCIIGRAPH
			    if ((boolopt[i].addr) == &flags.IBMgraphics)
				switch_graphics(flags.IBMgraphics ?
						IBM_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef MAC_GRAPHICS_ENV
			    if ((boolopt[i].addr) == &flags.MACgraphics)
				switch_graphics(flags.MACgraphics ?
						MAC_GRAPHICS : ASCII_GRAPHICS);
# endif
# ifdef REINCARNATION
			    if (!initial && Is_rogue_level(&u.uz))
				assign_rogue_graphics(TRUE);
# endif
			}
#endif /* TERMLIB || ASCIIGRAPH || MAC_GRAPHICS_ENV */

			/* only do processing below if setting with doset() */
			if (initial) return;

			if ((boolopt[i].addr) == &flags.time
#ifdef EXP_ON_BOTL
			 || (boolopt[i].addr) == &flags.showexp
#endif
#ifdef SCORE_ON_BOTL
			 || (boolopt[i].addr) == &flags.showscore
#endif
			    )
			    flags.botl = TRUE;

			else if ((boolopt[i].addr) == &flags.invlet_constant) {
			    if (flags.invlet_constant) reassign();
			}

			else if ((boolopt[i].addr) == &flags.num_pad)
			    number_pad(flags.num_pad ? 1 : 0);

			else if ((boolopt[i].addr) == &flags.lit_corridor) {
			    /*
			     * All corridor squares seen via night vision or
			     * candles & lamps change.  Update them by calling
			     * newsym() on them.  Don't do this if we are
			     * initializing the options --- the vision system
			     * isn't set up yet.
			     */
			    vision_recalc(2);		/* shut down vision */
			    vision_full_recalc = 1;	/* delayed recalc */
			}

#ifdef TEXTCOLOR
			else if ((boolopt[i].addr) == &flags.use_color
			      || (boolopt[i].addr) == &flags.hilite_pet) {
			    need_redraw = TRUE;
# ifdef TOS
			    if ((boolopt[i].addr) == &flags.use_color
				&& flags.BIOS) {
				if (colors_changed)
				    restore_colors();
				else
				    set_colors();
			    }
# endif
			}
#endif

			return;
		}
	}

	/* out of valid options */
	badoption(opts);
}

/*
 * Convert the given string of object classes to a string of default object
 * symbols.
 */
static void
oc_to_str(src,dest)
    char *src, *dest;
{
    int i;

    while ((i = (int) *src++) != 0) {
	if (i < 0 || i >= MAXOCLASSES)
	    impossible("oc_to_str:  illegal object class %d", i);
	else
	    *dest++ = def_oc_syms[i];
    }
    *dest = '\0';
}

#if defined(MICRO) || defined(MAC)
# define OPTIONS_HEADING "OPTIONS"
#else
# define OPTIONS_HEADING "NETHACKOPTIONS"
#endif

int
doset()
{
	char buf[BUFSZ], ocl[MAXOCLASSES+1], on_off;
	const char *opt_name;
	int i;
	winid tmpwin;

	switch (yn_function("Show the current settings [c], or set options [s]?",
			    "csq", 'q')) {
	default:
	case 'q':
	    clear_nhwindow(WIN_MESSAGE);
	    return 0;
	case 'c':
	    tmpwin = create_nhwindow(NHW_MENU);
	    putstr(tmpwin, 0, OPTIONS_HEADING);
	    putstr(tmpwin, 0, "");
	    /* print the booleans */
	    for (i = 0; boolopt[i].name; i++) {
		if (!boolopt[i].addr) continue;
		opt_name = boolopt[i].name;
		if (*(boolopt[i].addr)) {
		    on_off = ' ';               /* on */
		} else {
		    if (!strcmp(opt_name, "female"))
			opt_name = "male",  on_off = ' ';
		    else
			on_off = '!';           /* off */
		}
		Sprintf(buf, "%c%s", on_off, opt_name);
		putstr(tmpwin, 0, buf);
	    }
	    /* print the compounds */
	    Sprintf(buf, " catname: %s",
			(catname[0] != 0) ? catname : "(null)");
	    putstr(tmpwin, 0, buf);
	    Sprintf(buf, " disclose: %s",
			(flags.end_disclose[0]) ? flags.end_disclose : "all");
	    putstr(tmpwin, 0, buf);
	    Sprintf(buf, " dogname: %s",
			(dogname[0] != 0) ? dogname : "(null)");
	    putstr(tmpwin, 0, buf);
#ifdef TUTTI_FRUTTI
	    Sprintf(buf, " fruit: %s", pl_fruit);
	    putstr(tmpwin, 0, buf);
#endif
	    Sprintf(buf, " msghistory: %u", flags.msg_history);
	    putstr(tmpwin, 0, buf);
	    Sprintf(buf, " name: %s", plname);
	    putstr(tmpwin, 0, buf);
	    oc_to_str(flags.inv_order, ocl);
	    Sprintf(buf, " packorder: %s", ocl);
	    putstr(tmpwin, 0, buf);
#ifdef CHANGE_COLOR
	    Sprintf(buf, " palette: %s", get_color_string());
	    putstr(tmpwin, 0, buf);
#endif
	    Sprintf(buf, " pettype: %s", preferred_pet == 'c' ? "cat" :
				    preferred_pet == 'd' ? "dog" : "random");
	    putstr(tmpwin, 0, buf);
	    oc_to_str(flags.pickup_types, ocl);
	    Sprintf(buf, " pickup_types: %s", (ocl[0]) ? ocl : "all");
	    putstr(tmpwin, 0, buf);
	    Sprintf(buf, " scores: %u top/%u around%s",
		        flags.end_top, flags.end_around,
		        (flags.end_own ? "/own" : ""));
	    putstr(tmpwin, 0, buf);
#ifdef VIDEOSHADES
            Sprintf(buf, " videoshades: %s %s %s",
				shade[0],shade[1],shade[2]);
            putstr(tmpwin, 0, buf);
            Sprintf(buf, " videocolors: %d %d %d %d %d %d %d %d %d %d %d %d",
		 ttycolors[RED],ttycolors[GREEN],ttycolors[BROWN],
                 ttycolors[BLUE],ttycolors[MAGENTA],ttycolors[CYAN],
                 ttycolors[ORANGE_COLORED],ttycolors[BRIGHT_GREEN],
		 ttycolors[YELLOW],ttycolors[BRIGHT_BLUE],
                 ttycolors[BRIGHT_MAGENTA],ttycolors[BRIGHT_CYAN]);
            putstr(tmpwin, 0, buf);
#endif /* VIDEOSHADES */
	    Sprintf(buf, " windowtype: %s", windowprocs.name);
	    putstr(tmpwin, 0, buf);
	    display_nhwindow(tmpwin, TRUE);
	    destroy_nhwindow(tmpwin);
	    break;
	case 's':
	    clear_nhwindow(WIN_MESSAGE);
	    getlin("What options do you want to set?", buf);
	    if(buf[0] == '\033') return 0;
	    need_redraw = FALSE;
	    parseoptions(buf, FALSE, FALSE);
	    if(need_redraw)
		(void) doredraw();
	    break;
	}

	return 0;
}

int
dotogglepickup()
{
	char buf[BUFSZ], ocl[MAXOCLASSES+1];

	flags.pickup = !flags.pickup;
	if (flags.pickup) {
	    oc_to_str(flags.pickup_types, ocl);
	    Sprintf(buf, "ON, for %s objects", ocl[0] ? ocl : "all");
	} else {
	    Strcpy(buf, "OFF");
	}
	pline("Autopickup: %s.", buf);
	return 0;
}

/* data for option_help() */
static const char *opt_intro[] = {
	"",
	"                 NetHack Options Help:",
	"",
#define CONFIG_SLOT 3	/* fill in next value at run-time */
	NULL,
#if !defined(MICRO) && !defined(MAC)
	"or use `NETHACKOPTIONS=\"<options>\"' in your environment;",
# ifdef VMS
	"-- for example, $ DEFINE NETHACKOPTIONS \"noautopickup,fruit:kumquat\"",
# endif
#endif
	"or press \"O\" while playing, and type your <options> at the prompt.",
	"In all cases, <options> is a list of options separated by commas.",
	"",
 "Boolean options (which can be negated by prefixing them with '!' or \"no\"):",
	NULL
};

static const char *opt_epilog[] = {
	"",
 "Some of the options can be set only before the game is started.  You will",
	"be so informed, if you attempt to set them while in the game.",
	NULL
};

void
option_help()
{
    char buf[BUFSZ], buf2[BUFSZ];
    register int i;
    winid datawin;

    datawin = create_nhwindow(NHW_TEXT);
#ifdef AMIGA
    if(FromWBench){
	Sprintf(buf,"Set options as OPTIONS= in %s or in icon;",configfile);
    } else
#endif
	Sprintf(buf, "Set options as OPTIONS=<options> in %s;", configfile);
    opt_intro[CONFIG_SLOT] = (const char *) buf;
    for (i = 0; opt_intro[i]; i++)
	putstr(datawin, 0, opt_intro[i]);

    /* Boolean options */
    for (i = 0; boolopt[i].name; i++) {
	if (boolopt[i].addr)
	    next_opt(datawin, boolopt[i].name);
    }
    next_opt(datawin, "");

    /* Compound options */
    putstr(datawin, 0, "Compound options:");
    for (i = 0; compopt[i].name; i++) {
	Sprintf(buf2, "`%s'", compopt[i].name);
	Sprintf(buf, "%-14s - %s", buf2, compopt[i].descr);
	putstr(datawin, 0, buf);
    }

    for (i = 0; opt_epilog[i]; i++)
	putstr(datawin, 0, opt_epilog[i]);

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);
    return;
}

/*
 * prints the next boolean option, on the same line if possible, on a new
 * line if not. End with next_opt("").
 */
void
next_opt(datawin, str)
winid datawin;
const char *str;
{
	static char buf[121];
	int i;
	char *s;

	if (!*str) {
		for (s = buf; *s; s++);	/* find end of string */
		if (s > &buf[1] && s[-2] == ',')
			s[-2] = 0;	/* strip last ", " */
		i = 121;
	}
	else	
		i = strlen(buf) + strlen(str) + 2;

	if (i > COLNO - 2) { /* rule of thumb */
		putstr(datawin, 0, buf);
		buf[0] = 0;
	}
	if (*str) {
		Strcat(buf, str);
		Strcat(buf, ", ");
	}
	else
		putstr(datawin, 0, str);
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
#ifdef GCC_WARN
	struct fruit *lastf = (struct fruit *)0;
#else
	struct fruit *lastf;
#endif
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

		for(i = bases[j=letindex(FOOD_CLASS)]; i < bases[j+1]; i++) {
			if (!strcmp(OBJ_NAME(objects[i]), pl_fruit)) {
				found = TRUE;
				break;
			}
		}
		if (found ||
		    (!strncmp(str, "tin of ", 7) && name_to_mon(str+7) > -1) ||
		    !strcmp(str, "empty tin") ||
		    !strcmp(str, "tin of spinach") ||
		    ((!strncmp(eos(str)-6," corpse",7) ||
						!strncmp(eos(str)-3, " egg",4))
			&& name_to_mon(str) > -1))
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

#endif	/* OPTION_LISTS_ONLY */

/*options.c*/
