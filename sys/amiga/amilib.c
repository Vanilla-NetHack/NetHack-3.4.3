/*	SCCS Id: @(#)amilib.c	3.1	93/04/26	*/
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "wintype.h"
#include "winami.h"
#include "func_tab.h"

#ifdef AMIGA_INTUITION

#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dosextens.h>
#include <ctype.h>
#undef  strcmpi
#include <string.h>
#include <errno.h>

#ifdef  IDCMP_CLOSEWINDOW
# define	INTUI_NEW_LOOK
#endif

#ifdef AZTEC_C
#include <functions.h>
#else
#include <dos.h>
#include <proto/exec.h>
#endif

#include "Amiga:lib/amilib.h"

#include "Amiga:winami.p"
#include "Amiga:amiwind.p"

WinamiBASE *WinamiBase = 0;

extern char *roles[];
extern char orgdir[];

struct Library *ConsoleDevice = 0;
int bigscreen = 0;
char Initialized = 0;
struct amii_DisplayDesc *amiIDisplay = 0;
struct Screen *HackScreen = 0;
winid WIN_BASE = WIN_ERR;
winid amii_rawprwin = WIN_ERR;

/*void genl_botl_flush( void );*/
void amii_outrip( winid, int );
void setup_librefs( WinamiBASE * );

/* The current color map */
unsigned short amii_initmap[] = {
#define C_BLACK		0
#define C_WHITE		1
#define C_BROWN		2
#define C_CYAN		3
#define C_GREEN		4
#define C_MAGENTA	5
#define C_BLUE		6
#define C_RED		7

    0x0000, /* color #0 */
    0x0FFF, /* color #1 */
    0x0830, /* color #2 */
    0x07ac, /* color #3 */
    0x0181, /* color #4 */
    0x0C06, /* color #5 */
    0x023E, /* color #6 */
    0x0c00  /* color #7 */
};



/* Interface definition, for use by windows.c and winprocs.h to provide
 * the simple intuition interface for the amiga...
 */
struct window_procs amii_procs =
{
    "amii",
    amii_init_nhwindows,
    amii_player_selection,
    amii_askname,
    amii_get_nh_event,
    amii_exit_nhwindows,
    amii_suspend_nhwindows,
    amii_resume_nhwindows,
    amii_create_nhwindow,
    amii_clear_nhwindow,
    amii_display_nhwindow,
    amii_destroy_nhwindow,
    amii_curs,
    amii_putstr,
    amii_display_file,
    amii_start_menu,
    amii_add_menu,
    amii_end_menu,
    amii_select_menu,
    amii_update_inventory,
    amii_mark_synch,
    amii_wait_synch,
#ifdef CLIPPING
    amii_cliparound,
#endif
    amii_print_glyph,
    amii_raw_print,
    amii_raw_print_bold,
    amii_nhgetch,
    amii_nh_poskey,
    amii_bell,
    amii_doprev_message,
    amii_yn_function,
    amii_getlin,
#ifdef COM_COMPL
    amii_get_ext_cmd,
#endif /* COM_COMPL */
    amii_number_pad,
    amii_delay_output,
    /* other defs that really should go away (they're tty specific) */
#ifdef CHANGE_COLOR
    amii_delay_output,
    amii_delay_output,
#endif
    amii_delay_output,
    amii_delay_output,

    amii_outrip,
};


/* Interface definition, for use by windows.c and winprocs.h to provide
 * the view window interface to nethack...
 */
struct window_procs amiv_procs =
{
    "amiv",
    amii_init_nhwindows,
    amii_player_selection,
    amii_askname,
    amii_get_nh_event,
    amii_exit_nhwindows,
    amii_suspend_nhwindows,
    amii_resume_nhwindows,
    amii_create_nhwindow,
    amii_clear_nhwindow,
    amii_display_nhwindow,
    amii_destroy_nhwindow,
    amii_curs,
    amii_putstr,
    amii_display_file,
    amii_start_menu,
    amii_add_menu,
    amii_end_menu,
    amii_select_menu,
    amii_update_inventory,
    amii_mark_synch,
    amii_wait_synch,
#ifdef CLIPPING
    amii_cliparound,
#endif
    amii_print_glyph,
    amii_raw_print,
    amii_raw_print_bold,
    amii_nhgetch,
    amii_nh_poskey,
    amii_bell,
    amii_doprev_message,
    amii_yn_function,
    amii_getlin,
#ifdef COM_COMPL
    amii_get_ext_cmd,
#endif /* COM_COMPL */
    amii_number_pad,
    amii_delay_output,
    /* other defs that really should go away (they're tty specific) */
#ifdef CHANGE_COLOR
    amii_delay_output,
    amii_delay_output,
#endif
    amii_delay_output,
    amii_delay_output,

    amii_outrip,
};

void
amii_loadlib( void )
{
    /* Close the library if opened it already (switching display types) */
    if( WinamiBase )
	CloseLibrary( (struct Library *)WinamiBase );

    if( ( WinamiBase = ( WinamiBASE *)OpenLibrary( "winami.library", 0 ) ) == NULL )
    {
	panic( "can't find winami.library" );
    }
    setup_librefs( WinamiBase );
}

void
amiv_loadlib( void )
{
    /* Close the library if opened it already (switching display types) */
    if( WinamiBase )
	CloseLibrary( (struct Library *)WinamiBase );

    if( ( WinamiBase = ( WinamiBASE *)OpenLibrary( "winamiv.library", 0 ) ) == NULL )
    {
	panic( "can't find winami.library" );
    }
    setup_librefs( WinamiBase );
}

void
CleanUp()
{
    if( WinamiBase )
    {
	CloseLibrary( (struct Library *)WinamiBase );
	WinamiBase = NULL;
    }
}

/* The library has references to the following code and data items in the main
 * game, so fill in the access pointers for it to user...uggghhh...
 */
void
setup_librefs( base )
	WinamiBASE *base;
{
    base->G_pline = pline;
    base->G_display_inventory = display_inventory;
    base->G_terminate = terminate;
    base->G_rnd = rnd;
    base->G_panic = panic;
    base->G_clearlocks = clearlocks;
    base->G_on_level = on_level;
    base->G_exit = exit;
    base->G_lowc = lowc;
    base->G_alloc = alloc;
    base->G_Abort = Abort;
    base->G_error = error;
    base->G_fopenp = fopenp;
/*    base->G_genl_botl_flush = genl_botl_flush; */

    base->G_yn_number = &yn_number;
    base->G_zapcolors = zapcolors;
    base->G_plname = plname;
    base->G_objects = objects;
    base->G_monsyms = monsyms;
    base->G_extcmdlist = extcmdlist;
    base->G_flags = &flags;
    base->G_oc_syms = oc_syms;
    base->G_showsyms = showsyms;
    base->G_quitchars = quitchars;
    base->G_pl_character = pl_character;
    base->G_WIN_MESSAGE = &WIN_MESSAGE;
    base->G_WIN_MAP = &WIN_MAP;
    base->G_tc_gbl_data = &tc_gbl_data;
    base->G_defsyms = defsyms;
    base->G_WIN_STATUS = &WIN_STATUS;
    base->G_u = &u;
    base->G_roles = roles;
    base->G_dungeon_topology = &dungeon_topology;
    base->G_toplines = toplines;
    base->G_WIN_INVEN = &WIN_INVEN;
    base->G_windowprocs = &windowprocs;
    base->G_orgdir = orgdir;
    base->G_mons = mons;
    base->G_amiIDisplay = &amiIDisplay;
    base->G_HackScreen = &HackScreen;
    base->G_pl_classes = pl_classes;
    base->G_bigscreen = &bigscreen;
    base->G_WINBASE = &WIN_BASE;
    base->G_amii_rawprwin = &amii_rawprwin;
    base->G_amii_initmap = amii_initmap;
    base->G_Initialized = &Initialized;
    base->G_ConsoleDevice = &ConsoleDevice;
    base->G_amii_procs = &amii_procs;
}
#endif

void Abort(rc)
long rc;
{
#ifdef CHDIR
    chdir(orgdir);
#endif
    if (Initialized && ConsoleDevice) {
	printf("\n\nAbort with alert code %08lx...\n", rc);
	amii_getret();
    } else
	Alert(rc);
#ifdef __SASC
    {
/*  __emit(0x4afc);     /* illegal instruction */
    __emit(0x40fc);     /* divide by */
    __emit(0x0000);     /*  #0  */
	/* NOTE: don't move CleanUp() above here - */
	/* it is too likely to kill the system     */
	/* before it can get the SnapShot out, if  */
	/* there is something really wrong.    */
    }
#endif
    CleanUp();
#undef exit
#ifdef AZTEC_C
    _abort();
#endif
    exit((int) rc);
}

/* fatal error */
/*VARARGS1*/
void error VA_DECL(const char *, s)
    VA_START(s);
    VA_INIT(s, char *);

    putchar('\n');
    vprintf(s, VA_ARGS);
    putchar('\n');

    VA_END();
    Abort(0L);
}

