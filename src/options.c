/*	SCCS Id: @(#)options.c	3.0	89/11/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "termcap.h"

static boolean NEARDATA set_order;

static void FDECL(nmcpy, (char *, const char *, int));
void FDECL(escapes,(const char *, char *));

#ifdef AMIGA_WBENCH
extern int FromWBench;
#endif

void
initoptions()
{
	register char *opts;

	flags.time = flags.nonews = flags.notombstone = flags.end_own =
	flags.standout = flags.nonull = flags.ignintr = FALSE;
	flags.no_rest_on_space = flags.invlet_constant = TRUE;
	flags.end_top = 3;
	flags.end_around = 2;
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
	flags.help = TRUE;
	flags.IBMgraphics = FALSE;
	flags.DECgraphics = FALSE;
#ifdef TEXTCOLOR
	flags.use_color = TRUE;
#endif
#ifdef AMIFLUSH
	flags.amiflush = FALSE;	/* default to original behaviour */
#endif
#ifdef MSDOS
#ifdef DGK
	flags.IBMBIOS =
#ifdef TOS
	TRUE;			/* BIOS might as well always be on for TOS */
#endif
	flags.rawio = FALSE;
#endif
	read_config_file();
#endif /* MSDOS */
#ifdef MACOS
	read_config_file();
	flags.standout = TRUE;	
#endif
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
void
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

void
assign_graphics(graph_ints,glth)
register unsigned int *graph_ints;
register int glth;
{
	register int i;

	if (glth > MAXPCHARS) glth = MAXPCHARS;		/* sanity check */
	for (i = 0; i < glth; i++)
		showsyms[i] = graph_ints[i];
	for (i = glth; i < MAXPCHARS; i++)
		showsyms[i] = defsyms[i];
}

/*
 * Use the nice IBM Extended ASCII line-drawing characters (codepage 437).
 *
 * OS/2 defaults to a multilingual character set (codepage 850, corresponding
 * to the ISO 8859 character set.  We should probably do a VioSetCp() call to
 * set the codepage to 437.
 */
void
assign_ibm_graphics()
{
#ifdef ASCIIGRAPH
	flags.IBMgraphics = TRUE;	/* not set from command line */

	showsyms[S_vwall] = 0xb3;	/* meta-3, vertical rule */
	showsyms[S_hwall] = 0xc4;	/* meta-D, horizontal rule */
	showsyms[S_tlcorn] = 0xda;	/* meta-Z, top left corner */
	showsyms[S_trcorn] = 0xbf;	/* meta-?, top right corner */
	showsyms[S_blcorn] = 0xc0;	/* meta-@, bottom left */
	showsyms[S_brcorn] = 0xd9;	/* meta-Y, bottom right */
	showsyms[S_crwall] = 0xc5;	/* meta-E, cross */
	showsyms[S_tuwall] = 0xc1;	/* meta-A, T up */
	showsyms[S_tdwall] = 0xc2;	/* meta-B, T down */
	showsyms[S_tlwall] = 0xb4;	/* meta-4, T left */
	showsyms[S_trwall] = 0xc3;	/* meta-C, T right */
	showsyms[S_vbeam] = 0xb3;	/* meta-3, vertical rule */
	showsyms[S_hbeam] = 0xc4;	/* meta-D, horizontal rule */
	showsyms[S_ndoor] = 0xfa;
	showsyms[S_vodoor] = 0xfe;	/* meta-~, small centered square */
	showsyms[S_hodoor] = 0xfe;	/* meta-~, small centered square */
	showsyms[S_room] = 0xfa;	/* meta-z, centered dot */
	showsyms[S_pool] = 0xf7;	/* meta-w, approx. equals */
#endif  /* ASCIIGRAPH */
}

/* Use VT100 graphics for terminals that have them */
void
assign_dec_graphics()
{
#ifdef TERMLIB
	flags.DECgraphics = TRUE;	/* not set from command line */

	showsyms[S_vwall] = 0xf8;	/* vertical rule */
	showsyms[S_hwall] = 0xf1;	/* horizontal rule */
	showsyms[S_tlcorn] = 0xec;	/* top left corner */
	showsyms[S_trcorn] = 0xeb;	/* top right corner */
	showsyms[S_blcorn] = 0xed;	/* bottom left */
	showsyms[S_brcorn] = 0xea;	/* bottom right */
	showsyms[S_crwall] = 0xee;	/* cross */
	showsyms[S_tuwall] = 0xf6;	/* T up */
	showsyms[S_tdwall] = 0xf7;	/* T down */
	showsyms[S_tlwall] = 0xf5;	/* T left */
	showsyms[S_trwall] = 0xf4;	/* T right */
	showsyms[S_vbeam] = 0xf8;	/* vertical rule */
	showsyms[S_hbeam] = 0xf1;	/* horizontal rule */
	showsyms[S_ndoor] = 0xfe;
	showsyms[S_vodoor] = 0xe1;	/* small centered square */
	showsyms[S_hodoor] = 0xe1;	/* small centered square */
	showsyms[S_room] = 0xfe;	/* centered dot */
	showsyms[S_pool] = 0xe0;	/* diamond */
#endif  /* TERMLIB */
}

void
parseoptions(opts, from_env)
register char *opts;
boolean from_env;
{
#ifndef MACOS
	register char *op;
	unsigned num;
	boolean negated;

	if(op = index(opts, ',')) {
		*op++ = 0;
		parseoptions(op, from_env);
	}

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

	if (!strncmp(opts, "num", 3)) {
		flags.num_pad = !negated;
		return;
	}

	if (!strncmp(opts, "hel", 3)) {
		flags.help = !negated;
		return;
	}

	if (!strncmp(opts, "IBMg", 4)) {
		if(from_env) {
		    flags.IBMgraphics = !negated;
		    if(flags.IBMgraphics) assign_ibm_graphics();
		} else {
#ifdef MSDOS
		  pline("\"IBMgraphics\" settable only from %s.", configfile);
#else
		  pline("IBMgraphics can be set only from NETHACKOPTIONS.");
#endif
		}
		return;
	}

	if (!strncmp(opts, "DEC", 3)) {
		if(from_env) {
		    flags.DECgraphics = !negated;
		    if(flags.DECgraphics) assign_dec_graphics();
		} else {
#ifdef MSDOS
		  pline("\"DECgraphics\" settable only from %s.", configfile);
#else
		  pline("DECgraphics can be set only from NETHACKOPTIONS.");
#endif
		}
		return;
	}

#ifdef TEXTCOLOR
	if (!strncmp(opts, "col", 3)) {
		flags.use_color = !negated;
		return;
	}
#endif
#ifdef AMIFLUSH
	if (!strncmp(opts, "flus", 4)) {
		flags.amiflush = !negated;
		return;
	}
#endif
#ifdef DGK
	if (!strncmp(opts, "IBM_", 4)) {
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
# ifdef AMIGA_WBENCH
		 if(FromWBench){
		  pline("\"name\" settable only from %s or in icon.",
			configfile);
		 } else
# endif
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
		unsigned int translate[MAXPCHARS+1];
		int i, lth;

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

		lth = strlen(opts);
		if(lth > MAXPCHARS) lth = MAXPCHARS;
		/* match the form obtained from PC configuration files */
		for(i = 0; i < lth; i++)
			translate[i] = opts[i];
		assign_graphics(translate,lth);
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
			} else if(*op == '!') {
				negated = !negated;
				op++;
			}
			while(*op == ' ') op++;

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
			while(letter(*++op) || *op == ' ') ;
			if(*op == '/') op++;
		}
		return;
	}
	if (!strncmp(opts, "dog", 3)) {
		if(!from_env) {
#ifdef MSDOS
# ifdef AMIGA_WBENCH
		if(FromWBench){
		 pline("\"dogname\" settable only from %s or in icon.",
			configfile);
		} else
# endif
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
# ifdef AMIGA_WBENCH
		if(FromWBench){
		 pline("\"catname\" settable only from %s or in icon.",
			configfile);
		} else
# endif
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
# ifdef AMIGA_WBENCH
	if(ami_wbench_badopt(opts))
# endif
	Printf("Bad syntax in OPTIONS in %s: %s.", configfile, opts);
#else
	Printf("Bad syntax in NETHACKOPTIONS: %s.", opts);
	(void) puts("Use for example:");
	(void) puts(
"NETHACKOPTIONS=\"!rest_on_space,notombstone,endgame:own/5 topscorers/4 around me\""
	);
#endif
	getret();
#endif /* MACOS */
}

int
doset()
{
#ifdef MACOS
#define	OPTIONS			"Nethack prefs"
#define OK_BUTTON 1
#define SAVE_BUTTON 2
#define CANCEL_BUTTON 3
#define MIN_CHECKBOX 4
#define EXPLORE_BOX 4
#define FEM_BOX 5
#define NEWS_BOX 6
#define MIN_OK_CHECKBOX 7
#define FIXINV_BOX 7
#define TOMB_BOX 8
#define TIME_BOX 9
#define VERBOSE_BOX 10
#define SILENT_BOX 11
#define AUTOZOOM_BOX 12
#define INVERSE_BOX 13
#define SORT_BOX 14
#define COLOR_BOX 15
#define PICKUP_BOX 16
#define CONFIRM_BOX 17
#define SAFE_BOX 18
#define REST_SPACE_BOX 19
#define MAX_CHECKBOX 19
#define PLAYER_NAME 20
#define CAT_NAME 21
#define DOG_NAME 22
#define FRUIT_NAME 23
#define PACKORDER 24
#define END_TOP 26
#define END_AROUND 27
#define FRUIT_TEXT 35
#define PACK_TEXT 34
#define ENABLE_INFO_BOX 38
#define ALT_CURS_BOX 41
#define ITEMTEXT(item,text) {GetDItem(optionDlg,item,&type,&ItemHndl,&box); \
					         (void)CtoPstr(text); \
					         SetIText(ItemHndl,text);\
					         (void)PtoCstr(text);}
#define HIDEITEM(item) {GetDItem(optionDlg,item,&type,&ItemHndl,&box); \
				        HideControl(ItemHndl);\
				        SetDItem(optionDlg,item,type+128,ItemHndl,&box);}
#define HIDETEXT(item) {GetDItem(optionDlg,item,&type,&ItemHndl,&box);\
						SetDItem(optionDlg,item,128+statText,ItemHndl,&box);\
						SetIText(ItemHndl,"\0");}
#define SHOWITEM(item) {GetDItem(optionDlg,item,&type,&ItemHndl,&box);\
						SetDItem(optionDlg,item,type-128,ItemHndl,&box);\
						ShowControl(ItemHndl);}
#define GETTEXT(item,maxsize) {GetDItem(optionDlg,item,&type,&ItemHndl,&box);\
					GetIText (ItemHndl, &tmp_name);\
					tmp_name[tmp_name[0]+1] = 0;\
					if (tmp_name[0] > maxsize)\
						tmp_name[0] = maxsize;}
	static boolean NEARDATA *flag_ptrs[20] = {0, 0, 0, 0, &flags.explore,
			&flags.female, &flags.nonews,&flags.invlet_constant,
			&flags.notombstone, &flags.time, &flags.verbose,
			&flags.silent, 0, &flags.standout, &flags.sortpack,
#ifdef TEXTCOLOR
			&flags.use_color,
#else
			0,
#endif
			&flags.pickup, &flags.confirm,
			&flags.safe_dog, &flags.no_rest_on_space};
	extern short macflags;
	extern short altCurs;
	short dlgItem, type;
	Rect box;
	extern WindowPtr	HackWindow;
	Handle ItemHndl;
	unsigned num;
	char *op;
	char tmp_name[256];
	DialogPtr optionDlg;
	DialogTHndl	th, centreDlgBox();
	boolean done = FALSE;
    short savemacflags = macflags;
	register char	*sp, *tmp;
	char a_k_a[PL_NSIZ];
	boolean fairsex, debugger, explorer;

/* Option handling:
	Startup: read options from Prefs (making changes!)
	Save exit: write current options to prefs
	Cancel exit: revert to options as defined in Prefs
	Use exit: allow changes. erased at next dialog
*/
	strncpy(a_k_a, plname, strlen(plname));
	a_k_a[(int)strlen(plname)] = '\0';
	fairsex = flags.female;
	debugger = flags.debug;
	explorer = flags.explore;
	read_config_file();
	macflags = savemacflags;
	SetCursor(&ARROW_CURSOR);
	
	th = centreDlgBox(130, FALSE);

	optionDlg = GetNewDialog(130, (Ptr)NULL, (WindowPtr)-1);
/* set initial values of text items */
	ITEMTEXT(PLAYER_NAME,plname);
	if(*dogname) ITEMTEXT(DOG_NAME,dogname);
	if(*catname) ITEMTEXT(CAT_NAME,catname);
#ifdef TUTTI_FRUTTI
	if(*pl_fruit) ITEMTEXT(FRUIT_NAME,pl_fruit);
#else
	HIDETEXT(FRUIT_NAME);
	HIDETEXT(FRUIT_TEXT);
#endif
	ITEMTEXT(PACKORDER,inv_order);
/* set initial values of record items */
	Sprintf(tmp_name,"%u",flags.end_top);
	ITEMTEXT(END_TOP,tmp_name);
	Sprintf(tmp_name,"%u",flags.end_around);
	ITEMTEXT(END_AROUND,tmp_name);
/* set initial values of checkboxes */
	for(dlgItem = MIN_CHECKBOX; dlgItem <= MAX_CHECKBOX; dlgItem++) {
		GetDItem(optionDlg, dlgItem, &type, &ItemHndl, &box);
		switch (dlgItem){
			case NEWS_BOX:
#ifndef NEWS
				HIDEITEM(NEWS_BOX);
				break;
#endif
			case AUTOZOOM_BOX:
				SetCtlValue(ItemHndl,macflags & fZoomOnContextSwitch);
				break;
#ifndef TEXTCOLOR
			case COLOR_BOX:
				HIDEITEM(COLOR_BOX);
				break;
#endif
			default:
				SetCtlValue(ItemHndl,*(flag_ptrs[dlgItem]));
		}
	}
	GetDItem(optionDlg, ENABLE_INFO_BOX, &type, &ItemHndl, &box);
	SetCtlValue(ItemHndl, (int)flags.help);
	GetDItem(optionDlg, ALT_CURS_BOX, &type, &ItemHndl, &box);
 	SetCtlValue(ItemHndl, (short)altCurs);

	SelIText(optionDlg, PLAYER_NAME, 0, 32767);
	ShowWindow(optionDlg);
	GetDItem(optionDlg, OK_BUTTON, &type, &ItemHndl, &box);
	SetPort (optionDlg);
	PenSize(3, 3);
	InsetRect (&box, -4, -4);
	FrameRoundRect (&box, 16, 16);
	
	while(!done) {
		ModalDialog((ProcPtr)0, &dlgItem);
		GetDItem(optionDlg, dlgItem, &type, &ItemHndl, &box);
		if ((dlgItem >= MIN_CHECKBOX && dlgItem <= MAX_CHECKBOX)
			|| dlgItem == ENABLE_INFO_BOX || dlgItem == ALT_CURS_BOX) {
			SetCtlValue(ItemHndl, ! GetCtlValue (ItemHndl));
		}
		else switch(dlgItem){
			case SAVE_BUTTON:
				for(dlgItem = MIN_CHECKBOX; dlgItem <= MAX_CHECKBOX; dlgItem++) {
					GetDItem(optionDlg, dlgItem, &type, &ItemHndl, &box);
					if (dlgItem == AUTOZOOM_BOX) {
						if ((boolean)GetCtlValue(ItemHndl)) {
							macflags |= fZoomOnContextSwitch;
						} else {
							macflags &= ~fZoomOnContextSwitch;
						}
					} else {
						*(flag_ptrs[dlgItem]) = GetCtlValue(ItemHndl);
					}
				}
				GetDItem(optionDlg, ENABLE_INFO_BOX, &type, &ItemHndl, &box);
				flags.help = GetCtlValue(ItemHndl);
				GetDItem(optionDlg, ALT_CURS_BOX, &type, &ItemHndl, &box);
				altCurs = (short)GetCtlValue(ItemHndl);
				GETTEXT(PLAYER_NAME,PL_NSIZ-1);
				strncpy(plname, tmp_name, tmp_name[0]+1);
				(void)PtoCstr (plname);
			
				GETTEXT(DOG_NAME,62);
				strncpy(dogname, tmp_name, tmp_name[0]+1);
				(void)PtoCstr (dogname);
			
				GETTEXT(CAT_NAME,62);
				strncpy(catname, tmp_name, tmp_name[0]+1);
				(void)PtoCstr (catname);

#ifdef TUTTI_FRUTTI
				GETTEXT(FRUIT_NAME,PL_FSIZ-1);
				strncpy(pl_fruit, tmp_name, tmp_name[0]+1);
				(void)PtoCstr (pl_fruit);
#endif

				GETTEXT(PACKORDER,19);
				op = tmp_name+1;
				/* Missing characters in new order are filled in at the end 
				 * from inv_order.
				 */
				for (sp = op; *sp; sp++)
					if ((!index(inv_order, *sp))||(index(sp+1, *sp))){
						for(tmp = sp; *tmp;tmp++)
							tmp[0]=tmp[1];
						sp--;
					}			/* bad or duplicate char in order - remove it*/
				tmp = (char *) alloc((unsigned)(strlen(inv_order)+1));
				Strcpy(tmp, op);
				for (sp = inv_order, num = strlen(tmp); *sp; sp++)
					if (!index(tmp, *sp)) {
						tmp[num++] = *sp;
						tmp[num] = 0;
					}
				Strcpy(inv_order, tmp);
				free((genericptr_t)tmp);

				GETTEXT(END_TOP,5);
				op = tmp_name+1;
				while(*op) {
					num = 1;
					if(digit(*op)) {
						num = atoi(op);
						while(digit(*op)) op++;
					} else op++;
				}
				flags.end_top=num;
				GETTEXT(END_AROUND,5);
				op = tmp_name+1;
				while(*op) {
					num = 1;
					if(digit(*op)) {
						num = atoi(op);
						while(digit(*op)) op++;
					} else op++;
				}
				flags.end_around = num;
				
				write_opts();
				done = TRUE;
				break;
			case CANCEL_BUTTON:
				done = TRUE;
				break;
			case OK_BUTTON:
				for (dlgItem = MIN_OK_CHECKBOX; dlgItem <= MAX_CHECKBOX; dlgItem++) {
					GetDItem(optionDlg, dlgItem, &type, &ItemHndl, &box);
					if (dlgItem == AUTOZOOM_BOX) {
						if ((boolean)GetCtlValue(ItemHndl)) {
							macflags |= fZoomOnContextSwitch;
						} else {
							macflags &= ~fZoomOnContextSwitch;
						}
					} else {
						*(flag_ptrs[dlgItem]) = GetCtlValue(ItemHndl);
					}
				}
				GetDItem(optionDlg, ENABLE_INFO_BOX, &type, &ItemHndl, &box);
				flags.help = GetCtlValue(ItemHndl);
				GetDItem(optionDlg, ALT_CURS_BOX, &type, &ItemHndl, &box);
				altCurs = (short)GetCtlValue(ItemHndl);
				GETTEXT(END_TOP,5);
				op = tmp_name+1;
				while(*op) {
					num = 1;
					if(digit(*op)) {
						num = atoi(op);
						while(digit(*op)) op++;
					} else op++;
				}
				flags.end_top=num;
				GETTEXT(END_AROUND,5);
				op = tmp_name+1;
				while(*op) {
					num = 1;
					if(digit(*op)) {
						num = atoi(op);
						while(digit(*op)) op++;
					} else op++;
				}
				flags.end_around = num;
#ifdef TUTTI_FRUTTI
				GETTEXT(FRUIT_NAME,PL_FSIZ-1);
				(void)PtoCstr (tmp_name);
				(void)fruitadd(tmp_name);
				nmcpy(pl_fruit,tmp_name,PL_FSIZ-1);
#endif
				GETTEXT(PACKORDER,19);
				op = tmp_name+1;
				/* Missing characters in new order are filled in at the end 
				 * from inv_order.
				 */
				for (sp = op; *sp; sp++)
					if ((!index(inv_order, *sp))||(index(sp+1, *sp))){
						for (tmp = sp; *tmp;tmp++)
							tmp[0]=tmp[1];
						sp--;
					}			/* bad or duplicate char in order - remove it*/
				tmp = (char *) alloc((unsigned)(strlen(inv_order)+1));
				Strcpy(tmp, op);
				for (sp = inv_order, num = strlen(tmp); *sp; sp++)
					if (!index(tmp, *sp)) {
						tmp[num++] = *sp;
						tmp[num] = 0;
					}
				Strcpy(inv_order, tmp);
				free((genericptr_t)tmp);
				done = TRUE;
				break;
			default:;
		}
	} 
	flags.explore = explorer;
	flags.debug = debugger;
	flags.female = fairsex;
	strncpy(plname, a_k_a, strlen(a_k_a));
	plname[(int)strlen(a_k_a)] = '\0';
	HideWindow(optionDlg);
	DisposDialog (optionDlg);
	SetPort (HackWindow);
	return 0; 
#else
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
#ifdef TEXTCOLOR
	    if (flags.use_color) Strcat(buf, "color,");
#endif
#ifdef AMIFLUSH
	    if (flags.amiflush) Strcat(buf, "flush,");
#endif
	    if (!flags.help) Strcat(buf, "nohelp,");
	    if (flags.IBMgraphics) Strcat(buf,"IBMgraphics,");
	    if (flags.DECgraphics) Strcat(buf,"DECgraphics,");
	    if (flags.confirm) Strcat(buf,"confirm,");
	    if (flags.safe_dog) Strcat(buf,"safe_pet,");
	    if (flags.pickup) Strcat(buf,"pickup,");
	    if (flags.silent) Strcat(buf,"silent,");
	    if (flags.time) Strcat(buf,"time,");
	    if (flags.verbose) Strcat(buf,"verbose,");
#ifdef TUTTI_FRUTTI
	    Sprintf(eos(buf), "fruit:%s,", pl_fruit);
#endif
	    if(flags.end_top != 3 || flags.end_around != 2 || flags.end_own){
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
#endif /* MACOS */
}

int
dotogglepickup() {
	flags.pickup = !flags.pickup;
	pline("Pickup: %s.", flags.pickup ? "ON" : "OFF");
	return 0;
}

#define Page_line(x)	if(page_line(x)) goto quit
#define Next_opt(x)	if (next_opt(x)) goto quit

void
option_help() {
	char	buf[BUFSZ];

	set_pager(0);
	Sprintf(buf, "                 NetHack Options Help:");
	if(page_line("") || page_line(buf) || page_line(""))	 goto quit;

#ifdef MSDOS
# ifdef AMIGA_WBENCH
	if(FromWBench){
	 Sprintf(buf,"Set options as OPTIONS= in %s or in icon;",configfile);
	} else
# endif
	Sprintf(buf, "To set options use OPTIONS=<options> in %s;", configfile);
	Page_line(buf);
#else
	Page_line("To set options use `NETHACKOPTIONS=\"<options>\"' in your environment;");
#endif

	Page_line("or press \"O\" while playing, and type your <options> at the prompt.");
	Page_line("In either case, <options> is a list of options separated by commas.");
	Page_line("");

	Page_line("Boolean options (which can be negated by prefixing them with '!' or \"no\"):");
	Next_opt("DECgraphics, ");
#ifdef MSDOS
	Next_opt("IBM_BIOS, ");
#endif
	Next_opt("IBMgraphics, ");
#ifdef AMIFLUSH
	Next_opt("flush, ");
#endif
#ifdef TEXTCOLOR
	Next_opt("color, ");
#endif
	Next_opt("confirm, ");
	Next_opt("(fe)male, "); Next_opt("fixinv, ");
#ifdef UNIX
	Next_opt("ignintr, ");
#endif
	Next_opt("help, ");
#ifdef NEWS
	Next_opt("news, ");
#endif
#ifdef UNIX
	Next_opt("null, ");
#endif
	Next_opt("number_pad, ");
	Next_opt("pickup, ");
#ifdef MSDOS
	Next_opt("rawio, ");
#endif
	Next_opt("rest_on_space, "); Next_opt("safe_pet, ");
	Next_opt("silent, "); Next_opt("sortpack, ");
#ifdef UNIX
	Next_opt("standout, ");
#endif
	Next_opt("time, "); Next_opt("tombstone, ");
	Next_opt("and verbose.");
	Next_opt("");

	Page_line("Compound options:");
	Page_line("`name'      - your character's name (e.g., name:Merlin-W),");
	Page_line("`dogname'   - the name of your (first) dog (e.g., dogname:Fang),");
	Page_line("`catname'   - the name of your (first) cat (e.g., catname:Tabby),");

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
	(void) next_opt("\033");
	set_pager(2);
	return;
}

/*
 * prints the next boolean option, on the same line if possible, on a new
 * line if not. End with next_opt(""). Note that next_opt("\033") may be
 * used to abort.
 */
int
next_opt(str)
const char *str;
{
	static char buf[121];
	static int i = 0;
	int r = 0;

	if (*str == '\033') {
		i = 0; buf[0] = 0; return 0;
	}
	i += strlen(str);
	if (i > min(CO - 2, 120) || !*str) {
		r = page_line(buf);
		buf[0] = 0;
		i = strlen(str);
	}
	if (*str)
		Strcat(buf, str);
	else
		(void) page_line(str);	/* always returns 0 on "" */
	return r;
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
#ifdef __GNULINT__
	struct fruit *lastf = 0;
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
