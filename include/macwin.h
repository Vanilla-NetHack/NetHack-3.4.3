/*	SCCS Id: @(#)macwin.h	3.3	96/01/15	*/
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
#define NewControlActionProc(p) (ControlActionUPP)(p)

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

typedef struct {
	anything id;
	char accelerator;
	char groupAcc;
	short line;
} MacMHMenuItem;

typedef struct NhWindow {
	WindowPtr		its_window ;
/*	short			kind ;*/

	short			font_number ;
	short			font_size ;
	short			char_width ;
	short			row_height ;
	short			ascent_height ;
	
	short			x_size;
	short			y_size;
	short			x_curs;
	short			y_curs;
	
	short		last_more_lin ; /* Used by message window */
	short		save_lin ;		/* Used by message window */

	short			miSize;		/* size of menu items arrays */
	short			miLen;		/* number of menu items in array */
	MacMHMenuItem	**menuInfo;		/* Used by menus (array handle) */
	char		menuChar;		/* next menu accelerator to use */
	short			**menuSelected;	/* list of selected elements from list */
	short			miSelLen;	/* number of items selected */
	short			how;		/* menu mode */

	char			drawn ;
	Handle			windowText ;
	long			windowTextLen ;
	short		scrollPos ;
	ControlHandle	scrollBar ;
} NhWindow ;

extern NhWindow *GetNhWin(WindowPtr mac_win);


#define NUM_STAT_ROWS 2
#define NUM_ROWS 22
#define NUM_COLS 80 /* We shouldn't use column 0 */
#define QUEUE_LEN 24

extern NhWindow * theWindows ;

extern struct window_procs mac_procs ;

#define NHW_BASE 0
extern winid BASE_WINDOW , WIN_MAP , WIN_MESSAGE , WIN_INVEN , WIN_STATUS ;


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

/* extern char *macgets(int fd, char *ptr, unsigned len); unused */
extern void FDECL(C2P,(const char *c, unsigned char *p));
extern void FDECL(P2C,(const unsigned char *p, char *c));

/* ### macmenu.c ### */

extern void DoMenuEvt ( long ) ;
extern void InitMenuRes(void);
extern void AdjustMenus(short);

/* ### macmain.c ### */

extern void FDECL ( process_openfile, (short src_vol, long src_dir, Str255 fName, OSType ftype));

/* ### macwin.c ### */

extern void AddToKeyQueue(int, Boolean);
extern int GetFromKeyQueue ( void ) ;
void trans_num_keys ( EventRecord * ) ;
extern void NDECL ( InitMac ) ;
int FDECL ( try_key_queue , ( char * ) ) ;
void FDECL ( enter_topl_mode , ( char * ) ) ;
void FDECL ( leave_topl_mode , ( char * ) ) ;
void FDECL ( topl_set_resp , ( char * , char ) ) ;
Boolean FDECL ( topl_key , ( unsigned char ) ) ;
Boolean FDECL ( topl_ext_key , ( unsigned char ) ) ;
extern void WindowGoAway(EventRecord *, WindowPtr);
E void FDECL(HandleEvent, (EventRecord *));	/* used in mmodal.c */
extern void NDECL(port_help);

#define DimMenuBar() AdjustMenus(1)
#define UndimMenuBar() AdjustMenus(0)

extern Boolean small_screen ;

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
		CHAR_P,CHAR_P,int,const char *, BOOLEAN_P));
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
