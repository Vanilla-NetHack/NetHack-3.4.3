/*	SCCS Id: @(#)pager.c	3.1	93/02/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file contains the command routines dowhatis() and dohelp() and */
/* a few other help related facilities */

#include	"hack.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

static boolean FDECL(is_swallow_sym, (UCHAR_P));
static int FDECL(append_str, (char *, const char *));
static void FDECL(lookat, (int, int, char *));
static void FDECL(checkfile, (char *, BOOLEAN_P));
static int FDECL(do_look, (BOOLEAN_P));
static char NDECL(help_menu);
#ifdef PORT_HELP
extern void NDECL(port_help);
#endif

/* Returns "true" for characters that could represent a monster's stomach. */
static boolean
is_swallow_sym(c)
    uchar c;
{
    int i;
    for (i = S_sw_tl; i <= S_sw_br; i++)
	if (showsyms[i] == c) return TRUE;
    return FALSE;
}

/*
 * Append new_str to the end of buf if new_str doesn't already exist as
 * a substring of buf.  Return 1 if the string was appended, 0 otherwise.
 * It is expected that buf is of size BUFSZ.
 */
static int
append_str(buf, new_str)
    char *buf;
    const char *new_str;
{
    int space_left;	/* space remaining in buf */

    if (strstri(buf, new_str)) return 0;

    space_left = BUFSZ - strlen(buf) - 1;
    (void) strncat(buf, " or ", space_left);
    (void) strncat(buf, new_str, space_left - 4);
    return 1;
}

/*
 * Return the name of the glyph found at (x,y).
 */
static void
lookat(x, y, buf)
    int x, y;
    char *buf;
{
    register struct monst *mtmp;
    struct trap *trap;
    register char *s, *t;
    int glyph;

    buf[0] = 0;
    glyph = glyph_at(x,y);
    if (u.ux == x && u.uy == y && canseeself()) {
	Sprintf(buf, "%s called %s",
#ifdef POLYSELF
		u.mtimedone ? mons[u.umonnum].mname :
#endif
		player_mon()->mname, plname);
    }
    else if (u.uswallow) {
	/* all locations when swallowed other than the hero are the monster */
	Sprintf(buf, "interior of %s",
				    Blind ? "a monster" : a_monnam(u.ustuck));
    }
    else if (glyph_is_monster(glyph)) {
	bhitpos.x = x;
	bhitpos.y = y;
	mtmp = m_at(x,y);
	if(mtmp != (struct monst *) 0) {
	    register boolean hp = (mtmp->data == &mons[PM_HIGH_PRIEST]);

	    Sprintf(buf, "%s%s%s",
		    (!hp && mtmp->mtame && !Hallucination) ? "tame " :
		    (!hp && mtmp->mpeaceful && !Hallucination) ?
		                                          "peaceful " : "",
		    (hp ? "high priest" : l_monnam(mtmp)),
		    u.ustuck == mtmp ?
#ifdef POLYSELF
			((u.mtimedone && sticks(uasmon)) ? ", being held" :
#endif
			 ", holding you"
#ifdef POLYSELF
			 )
#endif
			: "");
	}
    }
    else if (glyph_is_object(glyph)) {
	struct obj *otmp = vobj_at(x,y);

	if(otmp == (struct obj *) 0 || otmp->otyp != glyph_to_obj(glyph)) {
	    if(glyph_to_obj(glyph) != STRANGE_OBJECT) {
		otmp = mksobj(glyph_to_obj(glyph), FALSE, FALSE);
		if(otmp->oclass == GOLD_CLASS)
		    otmp->quan = 2L; /* to force pluralization */
		Strcpy(buf, distant_name(otmp, xname));
		dealloc_obj(otmp);
	    }
	} else
	    Strcpy(buf, distant_name(otmp, xname));
    }
    else if (glyph_is_trap(glyph)) {
	if (trap = t_at(x, y)) {
	    if (trap->ttyp == WEB)
		Strcpy(buf, "web");
	    else {
		Strcpy(buf, traps[ Hallucination ?
				     rn2(TRAPNUM-3)+3 : trap->ttyp]);
		/* strip leading garbage */
		for (s = buf; *s && *s != ' '; s++) ;
		if (*s) ++s;
		for (t = buf; *t++ = *s++; ) ;
	    }
	}
    }
    else if(!glyph_is_cmap(glyph))
	Strcpy(buf,"dark part of a room");
    else switch(glyph_to_cmap(glyph)) {
    case S_altar:
        if(!In_endgame(&u.uz))
	    Sprintf(buf, "%s altar",
		align_str(Amask2align(levl[x][y].altarmask & ~AM_SHRINE)));
	else Sprintf(buf, "aligned altar");
	break;
    case S_ndoor:
	if((levl[x][y].doormask & ~D_TRAPPED) == D_BROKEN)
	    Strcpy(buf,"broken door");
	else
	    Strcpy(buf,"doorway");
	break;
    default:
	Strcpy(buf,defsyms[glyph_to_cmap(glyph)].explanation);
	break;
    }
}

/*
 * Look in the "data" file for more info.  Called if the user typed in the
 * whole name (user_typed_name == TRUE), or we've found a possible match
 * with a character/glyph and flags.help is TRUE.
 *
 * NOTE: when (user_typed_name == FALSE), inp is considered read-only and 
 *	 must not be changed directly, e.g. via lcase(). Permitted are
 *	 functions, e.g. makesingular(), which operate on a copy of inp.
 */
static void
checkfile(inp, user_typed_name)
    char *inp;
    boolean user_typed_name;
{
    FILE *fp;
    char buf[BUFSZ];
    char *ep;
    long txt_offset;
    boolean found_in_file = FALSE;

    fp = fopen_datafile(DATAFILE, "r");
    if (!fp) {
	pline("Cannot open data file!");
	return;
    }

    if (!strncmp(inp, "interior of ", 12))
	inp += 12;
    if (!strncmp(inp, "a ", 2))
	inp += 2;
    else if (!strncmp(inp, "an ", 3))
	inp += 3;
    else if (!strncmp(inp, "the ", 4))
	inp += 4;
    if (!strncmp(inp, "tame ", 5))
	inp += 5;
    else if (!strncmp(inp, "peaceful ", 9))
	inp += 9;
    if (!strncmp(inp, "invisible ", 10))
	inp += 10;

    /* Make sure the name is non-empty. */
    if (*inp) {
	/* adjust the input to remove "named " and convert to lower case */
	char *alt = 0;	/* alternate description */
	if ((ep = strstri(inp, " named ")) != 0)
	    alt = ep + 7;
	else
	    ep = strstri(inp, " called ");
	if (ep) *ep = '\0';
	if (user_typed_name)
	    (void) lcase(inp);

	/*
	 * If the object is named, then the name is the alternate description;
	 * otherwise, the result of makesingular() applied to the name is. This
	 * isn't strictly optimal, but named objects of interest to the user
	 * will usually be found under their name, rather than under their
	 * object type, so looking for a singular form is pointless.
	 */

	if (!alt)
	    alt = makesingular(inp);
	else
	    if (user_typed_name)
	    	(void) lcase(alt);

	/* skip first record; read second */
	txt_offset = 0L;
	if (!fgets(buf, BUFSZ, fp) || !fgets(buf, BUFSZ, fp)) {
	    impossible("can't read 'data' file");
	    (void) fclose(fp);
	    return;
	} else if (sscanf(buf, "%8lx\n", &txt_offset) < 1 || txt_offset <= 0)
	    goto bad_data_file;

	/* look for the appropriate entry */
	while (fgets(buf,BUFSZ,fp)) {
	    if (*buf == '.') break;  /* we passed last entry without success */

	    if (!digit(*buf)) {
		if (!(ep = index(buf, '\n'))) goto bad_data_file;
		*ep = 0;
		if (pmatch(buf, inp) || (alt && pmatch(buf, alt))) {
		    found_in_file = TRUE;
		    break;
		}
	    }
	}
    }

    if(found_in_file) {
	long entry_offset;
	int  entry_count;
	int  i;

	/* skip over other possible matches for the info */
	do {
	    if (!fgets(buf, BUFSZ, fp)) goto bad_data_file;
	} while (!digit(*buf));
	if (sscanf(buf, "%ld,%d\n", &entry_offset, &entry_count) < 2) {
bad_data_file:	impossible("'data' file in wrong format");
		(void) fclose(fp);
		return;
	}

	if (user_typed_name || yn("More info?") == 'y') {
	    winid datawin;

	    if (fseek(fp, txt_offset + entry_offset, SEEK_SET) < 0) {
		pline("? Seek error on 'data' file!");
		(void) fclose(fp);
		return;
	    }
	    datawin = create_nhwindow(NHW_TEXT);
	    for (i = 0; i < entry_count; i++) {
		if (!fgets(buf, BUFSZ, fp)) goto bad_data_file;
		if ((ep = index(buf, '\n')) != 0) *ep = 0;
		if (index(buf+1, '\t') != 0) (void) tabexpand(buf+1);
		putstr(datawin, 0, buf+1);
	    }
	    display_nhwindow(datawin, FALSE);
	    destroy_nhwindow(datawin);
	}
    } else if (user_typed_name)
	pline("I don't have any information on those things.");

    (void) fclose(fp);
}

static int
do_look(quick)
    boolean quick;	/* use cursor && don't search for "more info" */
{
    char    out_str[BUFSZ], look_buf[BUFSZ];
    const char    *firstmatch = 0;
    int     i;
    int     sym;		/* typed symbol or converted glyph */
    int	    found;		/* count of matching syms found */
    coord   cc;			/* screen pos of unknown glyph */
    boolean save_verbose;	/* saved value of flags.verbose */
    boolean from_screen;	/* question from the screen */
    boolean need_to_look;	/* need to get explan. from glyph */
    static const char *mon_interior = "the interior of a monster";

#ifdef GCC_WARN
    sym = 0;
#endif

    if (quick) {
	from_screen = TRUE;	/* yes, we want to use the cursor */
    } else {
	i = ynq("Specify unknown object by cursor?");
	if (i == 'q') return 0;
	from_screen = (i == 'y');
    }

    if (from_screen) {
	cc.x = u.ux;
	cc.y = u.uy;
    } else {
	getlin("Specify what? (type the word)", out_str);
	if (out_str[0] == '\0' || out_str[0] == '\033')
	    return 0;

	if (out_str[1]) {	/* user typed in a complete string */
	    checkfile(out_str, TRUE);
	    return 0;
	}
	sym = out_str[0];
    }

    /* Save the verbose flag, we change it later. */
    save_verbose = flags.verbose;
    flags.verbose = flags.verbose && !quick;
    /*
     * The user typed one letter, or we're identifying from the screen.
     */
    do {
	/* Reset some variables. */
	need_to_look = FALSE;
	found = 0;
	out_str[0] = '\0';

	if (from_screen) {
	    int glyph;	/* glyph at selected position */

	    if (flags.verbose)
		pline("Please move the cursor to an unknown object.");
	    else
		pline("Pick an object.");

	    getpos(&cc, FALSE, "an unknown object");
	    if (cc.x < 0) {
		flags.verbose = save_verbose;
		return 0;	/* done */
	    }
	    flags.verbose = FALSE;	/* only print long question once */

	    /* Convert the glyph at the selected position to a symbol. */
	    glyph = glyph_at(cc.x,cc.y);
	    if (glyph_is_cmap(glyph)) {
		sym = showsyms[glyph_to_cmap(glyph)];
	    } else if (glyph_is_trap(glyph)) {
		sym = showsyms[(glyph_to_trap(glyph) == WEB) ? S_web : S_trap];
	    } else if (glyph_is_object(glyph)) {
		sym = oc_syms[objects[glyph_to_obj(glyph)].oc_class];
	    } else if (glyph_is_monster(glyph)) {
		sym = monsyms[mons[glyph_to_mon(glyph)].mlet];
	    } else if (glyph_is_swallow(glyph)) {
		sym = showsyms[glyph_to_swallow(glyph)+S_sw_tl];
	    } else {
		impossible("do_look:  bad glyph %d at (%d,%d)",
						glyph, (int)cc.x, (int)cc.y);
		sym = ' ';
	    }
	}

	/*
	 * Check all the possibilities, saving all explanations in a buffer.
	 * When all have been checked then the string is printed.
	 */

	/* Check for monsters */
	for (i = 0; i < MAXMCLASSES; i++) {
	    if (sym == (from_screen ? monsyms[i] : def_monsyms[i])) {
		need_to_look = TRUE;
		if (!found) {
		    Sprintf(out_str, "%c       %s", sym, an(monexplain[i]));
		    firstmatch = monexplain[i];
		    found++;
		} else {
		    found += append_str(out_str, an(monexplain[i]));
		}
	    }
	}

	/*
	 * Special case: if identifying from the screen, and we're swallowed,
	 * and looking at something other than our own symbol, then just say
	 * "the interior of a monster".
	 */
	if (u.uswallow && from_screen && is_swallow_sym((uchar) sym)) {
	    if (!found) {
		Sprintf(out_str, "%c       %s", sym, mon_interior);
		firstmatch = mon_interior;
	    } else {
		found += append_str(out_str, mon_interior);
	    }
	    need_to_look = TRUE;
	}

	/* Now check for objects */
	for (i = 1; i < MAXOCLASSES; i++) {
	    if (sym == (from_screen ? oc_syms[i] : def_oc_syms[i])) {
		need_to_look = TRUE;
		if (!found) {
		    Sprintf(out_str, "%c       %s", sym, an(objexplain[i]));
		    firstmatch = objexplain[i];
		    found++;
		} else {
		    found += append_str(out_str, an(objexplain[i]));
		}
	    }
	}

	/* Now check for graphics symbols */
	for (i = 0; i < MAXPCHARS; i++) {
	    if (sym == (from_screen ? showsyms[i] : defsyms[i].sym) &&
						(*defsyms[i].explanation)) {
		if (!found) {
		    Sprintf(out_str, "%c       %s",
					    sym, an(defsyms[i].explanation));
		    firstmatch = defsyms[i].explanation;
		    found++;
		} else if (!u.uswallow) {
		    found += append_str(out_str, an(defsyms[i].explanation));
		}

		if (i == S_altar || i == S_trap || i == S_web)
		    need_to_look = TRUE;
	    }
	}

	/*
	 * If we are looking at the screen, follow multiple posibilities or
	 * an ambigious explanation by something more detailed.
	 */
	if (from_screen) {
	    if (found > 1 || need_to_look) {
		lookat(cc.x, cc.y, look_buf);
		firstmatch = look_buf;
		if (*firstmatch) {
		    char temp_buf[BUFSZ];
		    Sprintf(temp_buf, " (%s)", firstmatch);
		    (void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
		    found = 1;	/* we have something to look up */
		}
	    }
	}

	/* Finally, print out our explanation. */
	if (found) {
	    pline(out_str);
	    /* check the data file for information about this thing */
	    if (found == 1 && !quick && flags.help) {
		char temp_buf[BUFSZ];
		Strcpy(temp_buf, firstmatch);
		checkfile(temp_buf, FALSE);
	    }
	} else {
	    pline("I've never heard of such things.");
	}

    } while (from_screen && !quick);

    flags.verbose = save_verbose;
    return 0;
}


int
dowhatis()
{
	return do_look(FALSE);
}

int
doquickwhatis()
{
	return do_look(TRUE);
}

int
doidtrap()
{
	register struct trap *trap;
	register int x,y;

	if(!getdir(NULL)) return 0;
	x = u.ux + u.dx;
	y = u.uy + u.dy;
	for(trap = ftrap; trap; trap = trap->ntrap)
		if(trap->tx == x && trap->ty == y && trap->tseen) {
		    if(u.dz) {
			if(u.dz < 0 && trap->ttyp == TRAPDOOR)
			    continue;
		        if(u.dz > 0 && trap->ttyp == ROCKTRAP)
			    continue;
		    }
		    pline("That is a%s.",
			  traps[ Hallucination ? rn1(TRAPNUM-3, 3) :
				trap->ttyp]);
		    return 0;
		}
	pline("I can't see a trap there.");
	return 0;
}

int
dowhatdoes()
{
	FILE *fp;
	char bufr[BUFSZ+6];
	register char *buf = &bufr[6], *ep, q, ctrl, meta;

	fp = fopen_datafile(CMDHELPFILE, "r");
	if (!fp) {
		pline("Cannot open data file!");
		return 0;
	}

#if defined(UNIX) || defined(VMS)
	introff();
#endif
	q = yn_function("What command?", NULL, '\0');
#if defined(UNIX) || defined(VMS)
	intron();
#endif
	ctrl = ((q <= '\033') ? (q - 1 + 'A') : 0);
	meta = ((0x80 & q) ? (0x7f & q) : 0);
	while(fgets(buf,BUFSZ,fp))
	    if ((ctrl && *buf=='^' && *(buf+1)==ctrl) ||
		(meta && *buf=='M' && *(buf+1)=='-' && *(buf+2)==meta) ||
		*buf==q) {
		ep = index(buf, '\n');
		if(ep) *ep = 0;
		if (ctrl && buf[2] == '\t'){
			buf = bufr + 1;
			(void) strncpy(buf, "^?      ", 8);
			buf[1] = ctrl;
		} else if (meta && buf[3] == '\t'){
			buf = bufr + 2;
			(void) strncpy(buf, "M-?     ", 8);
			buf[2] = meta;
		} else if(buf[1] == '\t'){
			buf = bufr;
			buf[0] = q;
			(void) strncpy(buf+1, "       ", 7);
		}
		pline("%s", buf);
		(void) fclose(fp);
		return 0;
	    }
	pline("I've never heard of such commands.");
	(void) fclose(fp);
	return 0;
}

/* data for help_menu() */
static const char *help_menu_items[] = {
	"Information available:",
	"",
	"a.  Long description of the game and commands.",
	"b.  List of game commands.",
	"c.  Concise history of NetHack.",
	"d.  Info on a character in the game display.",
	"e.  Info on what a given key does.",
	"f.  List of game options.",
	"g.  Longer explanation of game options.",
	"h.  List of extended commands.",
	"i.  The NetHack license.",
#ifdef PORT_HELP
	"j.  %s-specific help and commands.",
#endif
#ifdef WIZARD
# ifdef PORT_HELP
# define WIZHLP_SLOT 12	/* assumed to be next to last by code below */
	"k.  List of wizard-mode commands.",
# else
# define WIZHLP_SLOT 11	/* assumed to be next to last by code below */
	"j.  List of wizard-mode commands.",
# endif
#endif
	"",
	NULL
};

static char
help_menu()
{
	winid tmpwin = create_nhwindow(NHW_MENU);
#ifdef PORT_HELP
	char helpbuf[QBUFSZ];
#endif
	char hc;
	register int i;

	start_menu(tmpwin);
#ifdef WIZARD
	if (!wizard) help_menu_items[WIZHLP_SLOT] = "",
		     help_menu_items[WIZHLP_SLOT+1] = NULL;
#endif
	for (i = 0; help_menu_items[i]; i++)
#ifdef PORT_HELP
	    /* port-specific line has a %s in it for the PORT_ID */
	    if (index(help_menu_items[i], '%')) {
		Sprintf(helpbuf, help_menu_items[i], PORT_ID);
		add_menu(tmpwin, helpbuf[0], 0, helpbuf);
	    } else
#endif
	    {
		add_menu(tmpwin, i ? *help_menu_items[i] : 0, 0,
			 help_menu_items[i]);
	    }
	end_menu(tmpwin, '\033', "\033", "Select one item or ESC: ");
	hc = select_menu(tmpwin);
	destroy_nhwindow(tmpwin);
	return hc;
}

int
dohelp()
{
	char hc = help_menu();
	if (!index(quitchars, hc)) {
		switch(hc) {
			case 'a':  display_file(HELP, TRUE);  break;
			case 'b':  display_file(SHELP, TRUE);  break;
			case 'c':  (void) dohistory();  break;
			case 'd':  (void) dowhatis();  break;
			case 'e':  (void) dowhatdoes();  break;
			case 'f':  option_help();  break;
			case 'g':  display_file(OPTIONFILE, TRUE);  break;
			case 'h':  (void) doextlist();  break;
			case 'i':  display_file(LICENSE, TRUE);  break;
#ifdef PORT_HELP
			case 'j':  port_help();  break;
# ifdef WIZARD
			case 'k':  display_file(DEBUGHELP, TRUE);  break;
# endif
#else
# ifdef WIZARD
			case 'j':  display_file(DEBUGHELP, TRUE);  break;
# endif
#endif
		}
	}
	return 0;
}

int
dohistory()
{
	display_file(HISTORY, TRUE);
	return 0;
}

/*pager.c*/
