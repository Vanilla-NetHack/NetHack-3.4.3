/*	SCCS Id: @(#)macwin.h	3.2	96/01/15	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MACWIN_H
# define MACWIN_H

/* more headers */
#ifdef THINK_C
#include <pascal.h>	/* for CtoPStr and PtoCStr */
#endif

/* resources */
#define PLAYER_NAME_RES_ID	1001

/* fake some things if we don't have universal headers.. */
#ifndef NewUserItemProc
typedef pascal void (*UserItemProcPtr)(WindowPtr theWindow, short itemNo);
typedef UserItemProcPtr UserItemUPP;
#define NewUserItemProc(p)	(UserItemUPP)(p)

typedef pascal void (*ControlActionProcPtr)(ControlHandle theControl, short partCode);
typedef ControlActionProcPtr ControlActionUPP;
#define NewControlActionProc(p)	(ControlActionUPP)(p)

typedef ModalFilterProcPtr ModalFilterUPP;
#define DisposeRoutineDescriptor(p)
#endif

/* misc */
#ifdef __MWERKS__
# define ResumeProcPtr long		/* for call to InitDialogs */
#endif


/*
 * Mac windows
 */
#define NUM_MACWINDOWS 15
#define TEXT_BLOCK 512L
#define WIN_BASE_RES 128
#define WIN_BASE_KIND 128
#define NUM_MENU_ITEMS 52			/* a-zA-z */
#define CHAR_ENTER ((char)3)
#define CHAR_BS ((char)8)
#define CHAR_LF ((char)10)
#define CHAR_CR ((char)13)
#define CHAR_ESC ((char)27)
#define CHAR_BLANK ((char)32)
#define CHAR_DELETE ((char)127)

/* Window constants */
#define kMapWindow 0
#define kStatusWindow 1
#define kMessageWindow 2
#define kTextWindow 3
#define kMenuWindow 4
#define kLastWindowKind kMenuWindow

/*
 * This determines the minimum logical line length in text windows
 * That is; even if physical width is less, this is where line breaks
 * go at the minimum. 350 is about right for score lines with a
 * geneva 10 pt font.
 */
#define MIN_RIGHT 350


typedef struct NhWindow {
	WindowPtr		theWindow ;
	short			kind ;
	void			( * keyFunc ) ( EventRecord * , WindowPtr ) ;
	void			( * clickFunc ) ( EventRecord * , WindowPtr ) ;
	void			( * updateFunc ) ( EventRecord * , WindowPtr ) ;
	void			( * cursorFunc ) ( EventRecord * , WindowPtr , RgnHandle ) ;
	Handle			windowText ;
	long			windowTextLen ;
	Point			cursor ;		/* In CHARS / LINES */
	short			leading ;
	short			charHeight ;
	short			charWidth ;
	short			fontNum ;
	short			fontSize ;
	short			last_more_lin ;	/* Used by message window */
	short			save_lin ;		/* Used by message window */
	short			lin ;			/* Used by menus */
	short			wid ;			/* Used by menus */
	char			itemChars [ NUM_MENU_ITEMS ] ;
	Boolean			itemSelected [ NUM_MENU_ITEMS ] ;
	anything		itemIdentifiers [ NUM_MENU_ITEMS ] ;
	short			how;			/* menu mode */
	char			menuChar;		/* next menu accelerator to use */
	char			clear ;
	short			scrollPos ;
	ControlHandle	scrollBar ;
} NhWindow ;

extern NhWindow *GetNhWin(WindowPtr mac_win);


#define NUM_STAT_ROWS 2
#define NUM_ROWS 22
#define NUM_COLS 81 /* We shouldn't use column 0 */

typedef struct MapData {
	char		map [ NUM_ROWS ] [ NUM_COLS ] ;
} MapData ;

typedef struct StatusData {
	char		map [ NUM_STAT_ROWS ] [ NUM_COLS ] ;
} StatusData ;

extern NhWindow * theWindows ;

extern struct window_procs mac_procs ;

extern short text_wind_font;
#define set_text_wind_font(fnt) (text_wind_font = fnt)
#define mono_font()	set_text_wind_font(monaco)
#define normal_font()	set_text_wind_font(geneva)


#define NHW_BASE 0
extern winid BASE_WINDOW , WIN_MAP , WIN_MESSAGE , WIN_INVEN , WIN_STATUS ;

/* askname dialog defines (shared between macmain.c and macmenu.c) */
enum
{
	dlog_start = 6000,
	dlogAskName = dlog_start,
	dlog_limit
};

/* askname dialog item list */
enum
{
	bttnANPlay = 1,
	bttnANQuit,
	uitmANOutlineDefault,
	uitmANRole,
	uitmANSex,
	uitmANMode,
	stxtANRole,
	stxtANSex,
	stxtANMode,
	stxtANWho,
	etxtANWho
};

typedef struct asknameRec
{
	short		anMenu[3];
	unsigned char	anWho[32];	/* player name Pascal string */
} asknameRec, *asknamePtr;

/* askname menus */
enum
{
	anRole,
	anSex,
	anMode
};

enum
{
	/* role */
	askn_role_start,	/* 0 */
	asknArcheologist = askn_role_start,
	asknBarbarian,
	asknCaveman,		/* Cavewoman */
	asknElf,
	asknHealer,
	asknKnight,
	asknPriest,		/* Priestess */
	asknRogue,
	asknSamurai,
	asknTourist,
	asknValkyrie,		/* female only */
	asknWizard,
	askn_role_end,

	/* sex */
	asknMale = 0,
	asknFemale,

	/* mode */
	asknRegular = 0,
	asknExplore,
	asknDebug,
	asknQuit		/* special token */
};


/*
 * External declarations for the window routines.
 */

#define E extern

/* ### dprintf.c ### */

extern void dprintf ( char * , ... ) ;

/* ### maccurs.c ### */

extern Boolean RetrievePosition ( short , short * , short * ) ;
extern Boolean RetrieveSize ( short , short , short , short * , short * ) ;
extern void SaveWindowPos ( WindowPtr ) ;
extern void SaveWindowSize ( WindowPtr ) ;
extern Boolean FDECL(RetrieveWinPos, (WindowPtr,short *,short *));

/* ### macerrs.c ### */

extern void comment(char *,long);
extern void showerror(char *,const char *);
extern Boolean itworked( short );
extern void mustwork( short );
extern void attemptingto( char *  );
extern void pushattemptingto( char *  );
extern void popattempt( void );

/* ### macfile.c ### */

#if 0
/* these declarations are in extern.h because they are used by the main code. */
extern int maccreat(const char *,long);
extern int macopen(const char *,int,long);
extern int macclose(int);
extern int macread(int,void *,unsigned);
extern int macwrite(int,void *,unsigned);
extern long macseek(int,long,short);
#endif /* 0 */
/* extern char *macgets(int fd, char *ptr, unsigned len); unused */
extern void FDECL(C2P,(const char *c, unsigned char *p));
extern void FDECL(P2C,(const unsigned char *p, char *c));

/* ### macmenu.c ### */

extern void	DialogAskName(asknameRec *);
extern void DoMenuEvt ( long ) ;
extern void InitMenuRes(void);
extern void AdjustMenus(short);

/* ### macmain.c ### */

#ifdef MAC68K
/* extern void UnloadAllSegments( void );	declared in extern.h */
extern void InitSegMgmt( void * );		/* called from macwin.c */
#endif
extern void NDECL ( finder_file_request ) ;

/* ### mactty.c ### */

extern pascal short tty_environment_changed(WindowPtr);

/* ### macwin.c ### */

extern void AddToKeyQueue(int, Boolean);
extern void SetFrameItem ( DialogPtr , short , short ) ;
extern void FlashButton ( DialogPtr , short ) ;
void trans_num_keys ( EventRecord * ) ;
extern void NDECL ( InitMac ) ;
int FDECL ( try_key_queue , ( char * ) ) ;
void FDECL ( enter_topl_mode , ( char * ) ) ;
void FDECL ( leave_topl_mode , ( char * ) ) ;
void FDECL ( topl_set_resp , ( char * , char ) ) ;
Boolean FDECL ( topl_key , ( unsigned char ) ) ;
Boolean FDECL ( topl_ext_key , ( unsigned char ) ) ;
void NDECL(InitRes);
extern void UpdateMenus ( void ) ;
extern void DoMenu ( long ) ;
extern int GetFromKeyQueue ( void ) ;
E void NDECL(port_help);
extern void WindowGoAway(EventRecord *, WindowPtr);
extern void DimMenuBar ( void ) ;
extern void UndimMenuBar ( void ) ;
E void FDECL(HandleEvent, (EventRecord *));	/* used in mmodal.c */

extern short frame_corner ;
extern winid inSelect ;
extern Boolean small_screen ;

/* ### mmodal.c ### */

E DialogPtr FDECL(mv_get_new_dialog, (short));
E void		FDECL(mv_close_dialog, (DialogPtr));
E void		FDECL(mv_modal_dialog, (ModalFilterProcPtr, short *));

/* ### mstring.c ### */

#ifdef applec
extern char *PtoCstr(unsigned char *);
extern unsigned char *CtoPstr(char *);
#endif



E void FDECL(mac_init_nhwindows, (int *, char **));
E void NDECL(mac_player_selection);
E void NDECL(mac_askname);
E void NDECL(mac_get_nh_event) ;
E void FDECL(mac_exit_nhwindows, (const char *));
E void FDECL(mac_suspend_nhwindows, (const char *));
E void NDECL(mac_resume_nhwindows);
E winid FDECL(mac_create_nhwindow, (int));
E void FDECL(mac_clear_nhwindow, (winid));
E void FDECL(mac_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(mac_destroy_nhwindow, (winid));
E void FDECL(mac_curs, (winid,int,int));
E void FDECL(mac_putstr, (winid, int, const char *));
E void FDECL(mac_display_file, (const char *, BOOLEAN_P));
E void FDECL(mac_start_menu, (winid));
E void FDECL(mac_add_menu, (winid,int,const anything *,
		CHAR_P,int,const char *, BOOLEAN_P));
E void FDECL(mac_end_menu, (winid, const char *));
E int FDECL(mac_select_menu, (winid, int, menu_item **));
E void NDECL(mac_update_inventory);
E void NDECL(mac_mark_synch);
E void NDECL(mac_wait_synch);
#ifdef CLIPPING
E void FDECL(mac_cliparound, (int, int));
#endif
E void FDECL(mac_print_glyph, (winid,XCHAR_P,XCHAR_P,int));
E void FDECL(mac_raw_print, (const char *));
E void FDECL(mac_raw_print_bold, (const char *));
E int NDECL(mac_nhgetch);
E int FDECL(mac_nh_poskey, (int *, int *, int *));
E void NDECL(mac_nhbell);
E int NDECL(mac_doprev_message);
E char FDECL(mac_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(mac_getlin, (const char *,char *));
E int NDECL(mac_get_ext_cmd);
E void FDECL(mac_number_pad, (int));
E void NDECL(mac_delay_output);

#undef E

#endif /* ! MACWIN_H */
