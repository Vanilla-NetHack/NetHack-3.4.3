/*    SCCS Id: @(#)wbprotos.h   3.1    93/01/08
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993.  */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef __STDC__
# define P(s) s
#else
# define P(s) ()
#endif


/* wb.c */
int main P(( int argc , struct WBStartup *argv ));
void CopyRight P(( void ));
void InitWB P(( int argc , struct WBStartup *wbs ));
void ReadConfig P(( void ));
void CloseDownWB P(( void ));
void SetupWB P(( void ));
void MapGadgets P(( int reason , int update ));
void ClearWindow P(( struct Window *win ));
void CalcLocs P(( int update ));
void text_requester P(( struct NewWindow *newwin , struct IntuiText *tlist ));
void getline P(( FILE *fp , long *offarr , int which , char *buf , int size ));
void help_requester P(( char *file ));
void do_closewindow P(( void ));
void do_menu P(( int mcode ));
void menu_discard P(( void ));
void menu_copyopt P(( void ));
void menu_rename P(( void ));
void run_game P(( GPTR gptr ));
void CleanUpLists P(( void ));
void CloseLibraries P(( void ));
void cleanup P(( int code ));
void SafeCloseWindow P(( struct Window *window ));
GPTR AllocGITEM P(( void ));
void FreeGITEM P(( GPTR gptr ));
struct DiskObject *AllocDObj P(( char *str ));
void FreeDObj P(( struct DiskObject *doptr ));
void LoadIcons P(( void ));
void menu_scores P(( void ));
void menu_recover P(( void ));
void menu_config P(( void ));
void menu_editdef P(( int newgame ));
void do_gadgetup P(( struct IntuiMessage *imsg ));
void do_buttons P(( struct IntuiMessage *imsg ));
void do_gadgetdown P(( struct IntuiMessage *imsg ));
int ask_request P(( char *str ));
void menu_setopt P(( void ));
void menu_info P(( void ));
int IsEditEntry P(( char *, GPTR ));
void menu_comment P(( void ));
void text_request P(( char *str , char *file ));
void errmsg P(( int flash , char *str, ... ));
void error P(( char *str ));
void SetGadgetUP P(( struct Gadget *gad ));
void SetGadgetDOWN P(( struct Gadget *gad ));
void UpdatePropGad P(( struct Window *win , struct Gadget *gad , long vis ,
    long total , long top ));
void *xmalloc P(( unsigned nbytes ));
int DeleteGame P(( GPTR gptr ));
GPTR FindGame P(( char *name ));
void setoneopt P(( int idx , char *str ));
char *dirname P(( char *str ));
int StrRequest P(( char *prompt , char *buff , char *val ));
int LoadDefaults P(( char *player ));
void SaveDefaults P(( char *player ));
void CopyDefs2Gad P(( void ));
void CopyGad2Defs P(( void ));
void UpdateTypes P(( struct Window *cwin ));
void CheckOnly P(( struct Menu *menuptr , int menu , int itemno ));
int Ask P(( char *quest ));
char *GameName P(( GPTR gptr , char *file ));
GPTR GetWBIcon P(( BPTR lock , char *dir , struct FileInfoBlock *finfo ));
void SetBorder P(( struct Gadget *gd, int val ));
GPTR NeedGame P(( void ));
void ChgGameItems P(( struct Menu *menup , int enable ));
void ChgNewGameItems P(( struct Menu *menup , int enable ));
int EditOptions P(( OPTR optr ));
struct Gadget *FindGadget P(( struct Window *window ,
    struct NewWindow *newwindow , int id ));
void ZapOptions P(( OPTR optr ));
void CopyOptions P(( OPTR optr , GPTR gptr ));
void CopyOptionStr P(( OPTR optr , char *str ));
void SetOptions P(( OPTR optr , GPTR gptr ));
void PutOptions P(( OPTR optr ));
char *ToolsEntry P(( GPTR gptr , char *name ));
void ReallocTools P(( GPTR gptr, int ));
void FreeTools P(( GPTR gptr ));
void SetToolLine P(( GPTR gptr , char *name , char *value ));
void WriteDObj P(( GPTR gptr , long lock ));
void UpdateGameIcon P(( GPTR gptr ));
char *Strdup P(( char *str ));
void Game2Defs P(( GPTR gptr ));
int CheckAndCopy P(( char *, char * ));
void ClearDelGames P(( void ));
int FindChecked P(( register struct Menu *menuptr, register int menu ));
void RPText P(( struct RastPort *rp, register char *s ));
#ifdef  INTUI_NEW_LOOK
struct Window *MyOpenWindow P(( struct ExtNewWindow *nw ));
#else
struct Window *MyOpenWindow P(( struct NewWindow *nw ));
#endif
void SetUpMenus( register struct Menu *mp, register struct Screen *scrn );
void UpdateCnfFile( void );
char *basename( char * );
void flushIDCMP( struct MsgPort *);

#undef P
