/*	SCCS Id: @(#)questpgr.c	3.1	93/01/20	*/
/*	Copyright 1991, M. Stephenson		  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef MULDGN
/*  quest-specific pager routines. */

#include "qtext.h"

#define QTEXT_FILE	"quest.dat"
#if defined(MICRO) && !defined(AMIGA)
# define RDMODE "rb"
#else
# define RDMODE "r"
#endif

#ifndef SEEK_SET
# define SEEK_SET 0
#endif

/* #define DEBUG	/* uncomment for debugging */

static void FDECL(Fread, (genericptr_t,int,int,FILE	*));
static struct qtmsg * FDECL(construct_qtlist, (long));
static unsigned NDECL(class_index);
static const char * NDECL(intermed);
static const char * NDECL(neminame);
static const char * NDECL(guardname);
static const char * NDECL(homebase);
static struct qtmsg * FDECL(msg_in, (struct qtmsg *,int));
static void FDECL(convert_arg, (CHAR_P));
static void NDECL(convert_line);
static void FDECL(deliver_by_pline, (struct qtmsg *));
static void FDECL(deliver_by_window, (struct qtmsg *));

static char	in_line[80], cvt_buf[64], out_line[128];
static struct	qtlists	qt_list;
static	FILE	*msg_file;

#ifdef DEBUG
static void NDECL(dump_qtlist);

static void
dump_qtlist()	/* dump the character msg list to check appearance */
{
	struct	qtmsg	*msg;
	long	size;

	for (msg = qt_list.chclass; msg->msgnum > 0; msg++) {
		pline("msgnum %d: delivery %c",
			msg->msgnum, msg->delivery);
		more();
		(void) fseek(msg_file, msg->offset, SEEK_SET);
		deliver_by_window(msg);
	}
}
#endif /* DEBUG */

static void
Fread(ptr, size, nitems, stream)
genericptr_t	ptr;
int	size, nitems;
FILE	*stream;
{
	int cnt;

	if ((cnt = fread(ptr, size, nitems, stream)) != nitems) {

	    panic("PREMATURE EOF ON QUEST TEXT FILE!\nExpected %d bytes - got %d\n",
		    (size * nitems), (size * cnt));
	}
}

static struct qtmsg *
construct_qtlist(hdr_offset)
long	hdr_offset;
{
	struct qtmsg *msg_list;
	int	n_msgs;

	(void) fseek(msg_file, hdr_offset, SEEK_SET);
	Fread(&n_msgs, sizeof(int), 1, msg_file);
	msg_list = (struct qtmsg *) alloc((n_msgs+1)*sizeof(struct qtmsg));

	/*
	 * Load up the list.
	 */
	Fread((genericptr_t)msg_list, n_msgs*sizeof(struct qtmsg), 1, msg_file);

	msg_list[n_msgs].msgnum = -1;
	return(msg_list);
}

void
load_qtlist()
{

	int	n_classes, i;
	char	qt_classes[N_HDR];
	long	qt_offsets[N_HDR];

	msg_file = fopen_datafile(QTEXT_FILE, RDMODE);
	if (!msg_file)
	    panic("\rCANNOT OPEN QUEST TEXT FILE %s.", QTEXT_FILE);

	/*
	 * Read in the number of classes, then the ID's & offsets for
	 * each header.
	 */

	Fread(&n_classes, sizeof(int), 1, msg_file);
	Fread(qt_classes, sizeof(char), n_classes, msg_file);
	Fread(qt_offsets, sizeof(long), n_classes, msg_file);

	/*
	 * Now construct the message lists for quick reference later
	 * on when we are actually paging the messages out.
	 */

	qt_list.common = qt_list.chclass = (struct qtmsg *)0;

	for (i = 0; i < n_classes; i++) {
	    if (qt_classes[i] == COMMON_ID)
		qt_list.common = construct_qtlist(qt_offsets[i]);
	    else if (qt_classes[i] == pl_character[0])
		qt_list.chclass = construct_qtlist(qt_offsets[i]);
	}

	if (!qt_list.common || !qt_list.chclass)
	    impossible("load_qtlist: cannot load quest text.");
#ifdef DEBUG
	dump_qtlist();
#endif
	return;	/* no ***DON'T*** close the msg_file */
}

static struct qt_matrix {

	const char *intermed;	/* intermediate goal text */
	const char *homebase;	/* leader's "location" */

	short	ldrnum,		/* mons[] indicies */
		neminum,
		guardnum;

	short	mtyp1, mtyp2;	/* monster types for enemies 0 == random */
	char	msym1, msym2;	/* monster classes for enemies */

	short	artinum;	/* index of quest artifact */

} qt_matrix[] = {

/* A */ { "the tomb of the Toltec Kings",
	  "the College of Archeology",
	  PM_LORD_CARNARVON, PM_MINION_OF_HUHETOL, PM_STUDENT,
	  0, PM_HUMAN_MUMMY, S_SNAKE, S_MUMMY,
	  ART_ORB_OF_DETECTION },

/* B */ { "the Duali Oasis",
	  "the Camp of the Duali Tribe",
	  PM_PELIAS, PM_THOTH_AMON, PM_CHIEFTAIN,
	  PM_OGRE, PM_TROLL, S_OGRE, S_TROLL,
	  ART_HEART_OF_AHRIMAN },

/* C */ { "the Dragon's Lair",
	  "the Caves of the Ancestors",
	  PM_SHAMAN_KARNOV, PM_CHROMATIC_DRAGON, PM_NEANDERTHAL,
	  PM_BUGBEAR, PM_HILL_GIANT, S_HUMANOID, S_GIANT,
	  ART_SCEPTRE_OF_MIGHT },

/* E */ { "the Goblins' Cave",
	  "the great Circle of Earendil",
	  PM_EARENDIL, PM_GOBLIN_KING, PM_HIGH_ELF,
	  PM_URUK_HAI, PM_OGRE, S_ORC, S_OGRE,
	  ART_PALANTIR_OF_WESTERNESSE },

/* E */ { "the Goblins' Cave",
	  "the great Circle of Elwing",
	  PM_ELWING, PM_GOBLIN_KING, PM_HIGH_ELF,
	  PM_URUK_HAI, PM_OGRE, S_ORC, S_OGRE,
	  ART_PALANTIR_OF_WESTERNESSE },

/* H */ { "the Temple of Coeus",
	  "the Temple of Epidaurus",
	  PM_HIPPOCRATES, PM_CYCLOPS, PM_ATTENDANT,
	  PM_GIANT_RAT, PM_SNAKE, S_RODENT, S_YETI,
	  ART_STAFF_OF_AESCULAPIUS },

/* K */ { "the Isle of Glass",
	  "Camelot Castle",
	  PM_KING_ARTHUR, PM_IXOTH, PM_PAGE,
	  PM_QUASIT, PM_OCHRE_JELLY, S_IMP, S_JELLY,
	  ART_MAGIC_MIRROR_OF_MERLIN },

/* P */ { "the Temple of Nalzok",
	  "the Great Temple",
	  PM_ARCH_PRIEST, PM_NALZOK, PM_ACOLYTE,
	  PM_HUMAN_ZOMBIE, PM_WRAITH, S_ZOMBIE, S_WRAITH,
	  ART_MITRE_OF_HOLINESS },

/* R */ { "the Asassins' Guild Hall",
	  "the Thieves' Guild Hall",
	  PM_MASTER_OF_THIEVES, PM_MASTER_ASSASSIN, PM_THUG,
	  PM_LEPRECHAUN, PM_GUARDIAN_NAGA, S_NYMPH, S_NAGA,
	  ART_MASTER_KEY_OF_THIEVERY },

/* S */ { "the Shogun's Castle",
	  "the castle of the Taro Clan",
	  PM_LORD_SATO, PM_ASHIKAGA_TAKAUJI, PM_NINJA,
	  PM_WOLF, PM_STALKER, S_DOG, S_STALKER,
	  ART_TSURUGI_OF_MURAMASA },

#ifdef TOURIST
/* T */ { "the Thieves' Guild Hall",
	  "the Traveller's Aid Office",
	  PM_TWOFLOWER, PM_MASTER_OF_THIEVES, PM_GUIDE,
	  PM_GIANT_SPIDER, PM_FOREST_CENTAUR, S_SPIDER, S_CENTAUR,
	  ART_YENDORIAN_EXPRESS_CARD },
#endif

/* V */ { "the cave of Surtur",
	  "the Shrine of Destiny",
	  PM_NORN, PM_LORD_SURTUR, PM_WARRIOR,
	  PM_FIRE_ANT, PM_FIRE_GIANT, S_ANT, S_GIANT,
	  ART_ORB_OF_FATE },

/* W */ { "the Tower of Darkness",
	  "the Tower of the Balance",
	  PM_WIZARD_OF_BALANCE, PM_DARK_ONE, PM_APPRENTICE,
	  PM_VAMPIRE_BAT, PM_XORN, S_BAT, S_WRAITH,
	  ART_EYE_OF_THE_AETHIOPICA },

/* - */ { "", "", 0, 0, 0, 0, 0, 0, 0, 0 }
};

static unsigned
class_index()
{
	switch (pl_character[0]) {

	    case 'A':	return(0);
	    case 'B':	return(1);
	    case 'C':	return(2);
	    case 'E':	return(3+flags.female);
	    case 'H':	return(5);
	    case 'K':	return(6);
	    case 'P':	return(7);
	    case 'R':	return(8);
	    case 'S':	return(9);
#ifdef TOURIST
	    case 'T':	return(10);
	    case 'V':	return(11);
	    case 'W':	return(12);
	    default:	return(13);
#else
	    case 'V':	return(10);
	    case 'W':	return(11);
	    default:	return(12);
#endif
	}
}

const char *
ldrname()	/* return your class leader's name */
{
	int i = qt_matrix[class_index()].ldrnum;
/*	return(mons[qt_matrix[class_index()].ldrnum].mname); */
	return(mons[i].mname);
}

static const char *
intermed()	/* return your intermediate target string */
{
	return(qt_matrix[class_index()].intermed);
}

boolean
is_quest_artifact(otmp)
struct obj *otmp;
{
	return(otmp->oartifact == qt_matrix[class_index()].artinum);
}

static const char *
neminame()	/* return your class nemesis' name */
{
	return(mons[qt_matrix[class_index()].neminum].mname);
}

static const char *
guardname()	/* return your class leader's guard monster name */
{
	return(mons[qt_matrix[class_index()].guardnum].mname);
}

static const char *
homebase()	/* return your class leader's location */
{
	return(qt_matrix[class_index()].homebase);
}

boolean
leaderless()	/* return true iff leader is dead */
{
	int i = qt_matrix[class_index()].ldrnum;
	return (u.nr_killed[i] > 0);
}

static struct qtmsg *
msg_in(qtm_list, msgnum)
struct qtmsg *qtm_list;
int	msgnum;
{
	struct qtmsg *qt_msg;

	for (qt_msg = qtm_list; qt_msg->msgnum > 0; qt_msg++)
	    if (qt_msg->msgnum == msgnum) return(qt_msg);

	return((struct qtmsg *)0);
}

static void
convert_arg(c)
char c;
{
	register const char *str;

	switch (c) {

	    case 'p':	str = plname;
			break;
	    case 'c':	str = pl_character;
			break;
	    case 'r':	str = rank_of(u.ulevel, pl_character[0], flags.female);
			break;
	    case 'R':	str = rank_of(MIN_QUEST_LEVEL, pl_character[0],
							  flags.female);
			break;
	    case 's':	str = (flags.female) ? "sister" : "brother";
			break;
	    case 'S':	str = (flags.female) ? "daughter" : "son";
			break;
	    case 'l':	str = ldrname();
			break;
	    case 'i':	str = intermed();
			break;
	    case 'o':	str = the(artiname(qt_matrix[class_index()].artinum));
			break;
	    case 'n':	str = neminame();
			break;
	    case 'g':	str = guardname();
			break;
	    case 'H':	str = homebase();
			break;
	    case 'a':	str = align_str(u.ualignbase[0]);
			break;
	    case 'A':	str = align_str(u.ualign.type);
			break;
	    case 'd':	str = u_gname();
			break;
	    case 'D':	str = align_gname(A_LAWFUL);
			break;
	    case 'C':	str = "chaotic";
			break;
	    case 'N':	str = "neutral";
			break;
	    case 'L':	str = "lawful";
			break;
	    case 'x':	str = Blind ? "sense" : "see";
			break;
	    case '%':	str = "%";
			break;
	     default:	str = "";
			break;
	}
	Strcpy(cvt_buf, str);
}

static void
convert_line()
{
	char *c, *cc;

	cc = out_line;
	for (c = xcrypt(in_line); *c; c++) {

	    *cc = 0;
	    switch(*c) {

		case '\r':
		case '\n':
			*(++cc) = 0;
			return;

		case '%':
			if (*(c+1)) {
			    convert_arg(*(++c));
			    switch (*(c+1)) {

				case 'A': Strcat(cc, An(cvt_buf));
				    cc += strlen(cc);
				    c++;
				    continue; /* for */
				case 'a': Strcat(cc, an(cvt_buf));
				    cc += strlen(cc);
				    c++;
				    continue; /* for */

				case 'C': cvt_buf[0] = highc(cvt_buf[0]);
				    c++;
				    break;

				case 'P': cvt_buf[0] = highc(cvt_buf[0]);
				case 'p': Strcpy(cvt_buf, makeplural(cvt_buf));
					    c++; break;

				default: break;
			    }
			    Strcat(cc, cvt_buf);
			    cc += strlen(cvt_buf);
			    break;
			}	/* else fall through */

		default:
			*cc++ = *c;
			break;
	    }
	}
	*cc = 0;
	return;
}

static void
deliver_by_pline(qt_msg)
struct qtmsg *qt_msg;
{
	long	size;

	for (size = 0; size < qt_msg->size; size += (long)strlen(in_line)) {
	    (void) fgets(in_line, 80, msg_file);
	    convert_line();
	    pline(out_line);
	}

}

static void
deliver_by_window(qt_msg)
struct qtmsg *qt_msg;
{
	long	size;
	winid datawin = create_nhwindow(NHW_TEXT);

	for (size = 0; size < qt_msg->size; size += (long)strlen(in_line)) {
	    (void) fgets(in_line, 80, msg_file);
	    convert_line();
	    putstr(datawin, 0, out_line);
	}
	display_nhwindow(datawin, TRUE);
	destroy_nhwindow(datawin);
}

void
com_pager(msgnum)
int	msgnum;
{
	struct qtmsg *qt_msg;

	if (!(qt_msg = msg_in(qt_list.common, msgnum))) {
		impossible("com_pager: message %d not found.", msgnum);
		return;
	}

	(void) fseek(msg_file, qt_msg->offset, SEEK_SET);
	if (qt_msg->delivery == 'p') deliver_by_pline(qt_msg);
	else		     deliver_by_window(qt_msg);
	return;
}

void
qt_pager(msgnum)
int	msgnum;
{
	struct qtmsg *qt_msg;

	if (!(qt_msg = msg_in(qt_list.chclass, msgnum))) {
		impossible("qt_pager: message %d not found.", msgnum);
		return;
	}

	(void) fseek(msg_file, qt_msg->offset, SEEK_SET);
	if (qt_msg->delivery == 'p' && strcmp(windowprocs.name, "X11"))
		deliver_by_pline(qt_msg);
	else	deliver_by_window(qt_msg);
	return;
}

struct permonst *
qt_montype()
{
	int class = class_index();

	if (rn2(5)) {
		if (qt_matrix[class].mtyp1 && rn2(5) &&
			 !(mons[qt_matrix[class].mtyp1].geno & G_GENOD))
				return(&mons[qt_matrix[class].mtyp1]);
		return(mkclass(qt_matrix[class].msym1,0));
	}
	if (qt_matrix[class].mtyp2 && rn2(5) &&
		 !(mons[qt_matrix[class].mtyp1].geno & G_GENOD))
			return(&mons[qt_matrix[class].mtyp2]);
	return(mkclass(qt_matrix[class].msym2,0));
}
#endif /* MULDGN */

/*questpgr.c*/
