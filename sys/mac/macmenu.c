/*	SCCS Id: @(#)macmenu.c	3.1	           93/04/29       */
/*      Copyright (c) Macintosh NetHack Port Team, 1993.          */
/* NetHack may be freely redistributed.  See license for details. */

/****************************************\
 * Extended Macintosh menu support
 *
 * provides access to all keyboard commands from cmd.c
 * provides control key functionality for classic keyboards
 * provides key equivalent references and logical menu groups
 * supports various menu highlighting modes
 * does not (yet) provide balloon help support (maybe never will!)
\****************************************/

/****************************************\
 * Edit History:
 *
 * 930512	- More bug fixes and getting tty to work again, Jon W{tte
 * 930508	- Bug fixes in-flight, Jon W{tte
 * 04/29/93 - 1st Release Draft, David Hairston
 * 04/11/93 - 1st Draft, David Hairston
\****************************************/

/******** Application Defines ********/
#include "hack.h"
#include "patchlevel.h"

/******** Toolbox Defines ********/
/* #include <Controls.h> */
#include <Desk.h>
/* #include <Dialogs.h> */
/* #include <Memory.h> */
#include <Menus.h>
/* #include <Quickdraw.h> */
#include <Resources.h>
/* #include <SegLoad.h> */
#include <TextEdit.h>
#include <ToolUtils.h>
#include <Packages.h>
/* #include <Windows.h> */

/* Think/MPW incompatibility from Think.h/Script.h */
#if !defined(__THINK__) && !defined(__SCRIPT__)
#define GetMBarHeight()		(* (short *) 0x0BAA)
#endif

/* Think/MPW incompatibility from LoMem.h/SysEqu.h */
#if !defined(__LOMEM__) && !defined(__SYSEQU__)
enum { WindowList = 0x9D6 };
#endif

/* Think has separated out c2pstr and other pascal odditites */
#if defined(THINK_C)
#include <pascal.h>
#endif

/******** Local Defines ********/

/* 'MNU#' (menu list record) */
typedef union menuRefUnn
{
	short		mresID;		/* MENU resource ID (before GetMenu) */
	MenuHandle	mhnd;		/* MENU handle (after GetMenu) */
} menuRefUnn;

typedef struct menuListRec
{
	short		firstMenuID;
	short		numMenus;
	menuRefUnn	mref[];
} menuListRec, *menuListPtr, **menuListHandle;

/* indices and resource IDs of the menu list data */
enum
{
	listMenubar,
	listSubmenu,

	menuBarListID = 128,
	subMenuListID
};

/* the following mref[] indices are reserved */
enum
{
	/* menu bar */
	menuApple,
	menuFile,
	menuEdit,

	/* submenu */
	menuWizard = 0
};

/* the following menu items are reserved */
enum
{
	/* apple */
	menuAppleAboutBox = 1,
	____Apple__1,

	/* File */
	menuFileOpenMap = 1,
	menuFileRedraw,
	menuFilePrevMsg,
	menuFileCleanup,
	menuFileClose,
	____File___1,
	menuFilePlayMode,
	menuFileEnterExplore,
	____File___2,
	menuFileOptionEdit,
	____File___3,
	menuFileSave,
	____File___4,
	menuFileQuit,

	/* standard minimum Edit menu items */

	/* Wizard */
	menuWizardAttributes = 1
};

/* symbols here correspond to arrays in DialogAskName */
/* eventually this data will become a 'STR#' resource */
static unsigned char	uitmChar[3][16] = {{"ABCEHKPRSTVW"}, {"MF"}, {" XD"}};

	/* arrays here correspond to symbols in drawANUserItem */
	/* eventually this data will be moved to 'STR#' resources */
static unsigned char * nhRole [ askn_role_end ] = {
	{"\pArcheologist"}, {"\pBarbarian"},
	{"\pCaveman"}, {"\pElf"}, {"\pHealer"}, {"\pKnight"},
	{"\pPriest"}, {"\pRogue"}, {"\pSamurai"}, {"\pTourist"},
	{"\pValkyrie"}, {"\pWizard"}
};
static unsigned char * nhSex [ 2 ] = {
	{"\pMale"}, {"\pFemale"}
};
static unsigned char * nhMode [ 3 ] = {
	{"\pRegular"}, {"\pExplore"}, {"\pDebug"}
};


/*
 * menuListRec data (preloaded and locked) specifies the number of menus in
 * the menu bar, the number of hierarchal or submenus and the menu IDs of
 * all of those menus.  menus that go into in the menu bar are specified by
 * 'MNU#' 128 and submenus are specified by 'MNU#' 129.  the fields of the
 * menuListRec are:
 * firstMenuID - the menu ID (not resource ID) of the 1st menu.  subsequent
 *     menus in the list are _forced_ to have consecutively incremented IDs.
 * numMenus - the total count of menus in a given list (and the extent of
 *     valid menu IDs).
 * mref[] - initially the MENU resource ID is stored in the placeholder for
 *     the resource handle.  after loading (GetResource), the menu handle
 *     is stored and the menu ID, in memory, is set as noted above.
 *
 * NOTE: a ResEdit template editor is supplied to edit the 'MNU#' resources.
 *
 * NOTE: the resource IDs do not need to match the menu IDs in a menu list
 * record although they have been originally set that way.
 *
 * NOTE: the menu ID's of menus in the submenu list record may be reset, as
 * noted above.  it is the programmers responsibility to make sure that
 * submenu references/IDs are valid.
 *
 * WARNING: the existence of the submenu list record is assumed even if the
 * number of submenus is zero.  also, no error checking is done on the
 * extents of the menu IDs.  this must be correctly setup by the programmer.
 */

#define ID1_MBAR	pMenuList[listMenubar]->firstMenuID
#define ID1_SUBM	pMenuList[listSubmenu]->firstMenuID

#define NUM_MBAR	pMenuList[listMenubar]->numMenus
#define NUM_SUBM	pMenuList[listSubmenu]->numMenus

#define MHND_APPLE	pMenuList[listMenubar]->mref[menuApple].mhnd
#define MHND_FILE	pMenuList[listMenubar]->mref[menuFile].mhnd
#define MHND_EDIT	pMenuList[listMenubar]->mref[menuEdit].mhnd

#define MBARHND(x)	pMenuList[listMenubar]->mref[(x)].mhnd

#define MHND_WIZ	pMenuList[listSubmenu]->mref[menuWizard].mhnd


/* mutually exclusive (and prioritized) menu bar states */
enum
{
	mbarDim,
	mbarNoWindows,
	mbarDA,
	mbarNoMap,
	mbarRegular,
	mbarSpecial					/* explore or debug mode */
};

#define WKND_MAP		(WIN_BASE_KIND + NHW_MAP)


/* menu routine error numbers */
enum
{
	errGetMenuList,
	errGetMenu,
	errGetANDlogTemplate,
	errGetANDlogItems,
	errGetANDialog,
	errANNewMenu,
	err_Menu_total
};


/* menu 'STR#' comment char */
#define mstrEndChar		0xA5		/* '\245' or option-* or "bullet" */

/* max key queue (from macwin.c) */
#define QUEUE_LEN		keyQueueLen
extern const int keyQueueLen ;

/* 'ALRT' */
enum
{
	alrt_Menu_start = 5000,
	alrtMenuNote = alrt_Menu_start,
	alrtMenu_NY,
	alrt_Menu_limit
};

#define beepMenuAlertErr	1		/* # of SysBeep()'s before exitting */
enum
{
	bttnMenuAlertNo = 1,
	bttnMenuAlertYes
};

/* askname menus */
enum
{
	menuANRole,
	menuANSex,
	menuANMode,
	menuAN_total
};


/******** Globals ********/
static	unsigned char *menuErrStr[err_Menu_total] = 
	{
		"\pAbort: Bad \'MNU#\' resource!",		/* errGetMenuList */
		"\pAbort: Bad \'MENU\' resource!",		/* errGetMenu */
		"\pAbort: Bad \'DLOG\' resource!",		/* errGetANDlogTemplate */
		"\pAbort: Bad \'DITL\' resource!",		/* errGetANDlogItems */
		"\pAbort: Bad Dialog Allocation!",		/* errGetANDialog */
		"\pAbort: Bad Menu Allocation!",		/* errANNewMenu */
	};
static	menuListPtr	pMenuList[2];
static	short		theMenubar = mbarDA;	/* force initial update */
static	short		kAdjustWizardMenu = 1;


/******** Prototypes ********/
static	void alignAD(Rect *, short);
static	void mustGetMenuAlerts(void);
static	void menuError(short);

extern	void AddToKeyQueue ( int ch , Boolean force ) ;
pascal	void drawANUserItem(WindowPtr, short);
		void DialogAskName(asknameRec *);
		void InitMenuRes(void);
		void AdjustMenus(short);
		void DoMenuEvt(long);
extern	void WindowGoAway(EventRecord *, WindowPtr);

static	void aboutNetHack(void);
static	void openMap(void);
static	void closeFrontWindow(void);
static	void optionEditor(void);
static	void askSave(void);
static	void askQuit(void);


/******** Routines ********/
static void
alignAD(Rect *pRct, short vExempt)
{
	(*pRct).right -= (*pRct).left;		/* width */
	(*pRct).bottom -= (*pRct).top;		/* height */
	(*pRct).left = (qd.screenBits.bounds.right - (*pRct).right) / 2;
	(*pRct).top = (qd.screenBits.bounds.bottom - (*pRct).bottom - vExempt) / 2;
	(*pRct).top += vExempt;
	(*pRct).right += (*pRct).left;
	(*pRct).bottom += (*pRct).top;
}

static void
mustGetMenuAlerts()
{
	short		i, mbarHgt = GetMBarHeight();
	Rect		**hRct;

	for (i = alrt_Menu_start; i < alrt_Menu_limit; i++)
	{
		if (! (hRct = (Rect **) GetResource('ALRT', i)))	/* AlertTHndl */
		{
			for (i = 0; i < beepMenuAlertErr; i++)
				SysBeep(3);
			ExitToShell();
		}

		alignAD(*hRct, mbarHgt);
	}
}

static void
menuError(short menuErr)
{
	short	i;

	for (i = 0; i < beepMenuAlertErr; i++)
		SysBeep(3);

	ParamText(menuErrStr[menuErr], "\p", "\p", "\p");
	(void) Alert(alrtMenuNote, (ModalFilterProcPtr) 0L);

	ExitToShell();
}

pascal void
drawANUserItem(WindowPtr wPtr, short uItm)
{
	asknameRec	*pANR;
	short		iTyp;
	Handle		iHnd;
	Rect		iRct;
	short		whichUserItem;

	pANR = (asknameRec *) GetWRefCon(wPtr);
	GetDItem((DialogPtr) wPtr, uItm, &iTyp, &iHnd, &iRct);

	switch (uItm)
	{
	case uitmANOutlineDefault:
		PenSize(3, 3);
		FrameRoundRect(&iRct, 16, 16);
		break;

	case uitmANRole:
	case uitmANSex:
	case uitmANMode:
		whichUserItem = uItm - uitmANRole;

		PenNormal();
		EraseRect(&iRct);

		/* drop shadow */
		iRct.right--;
		iRct.bottom--;
		MoveTo(iRct.right, (iRct.top + 1));
		LineTo(iRct.right, iRct.bottom);
		LineTo((iRct.left + 1), iRct.bottom);

		/* frame */
		FrameRect(&iRct);

		/* menu character */
		MoveTo((iRct.left + 2), (iRct.top + 12));
		DrawChar(uitmChar[whichUserItem][(*pANR).anMenu[whichUserItem]]);

		/* popup symbol */
		MoveTo((iRct.left + 18), (iRct.top + 6));
		LineTo((iRct.left + 28), (iRct.top + 6));
		LineTo((iRct.left + 23), (iRct.top + 11));
		LineTo((iRct.left + 19), (iRct.top + 7));
		LineTo((iRct.left + 26), (iRct.top + 7));
		LineTo((iRct.left + 23), (iRct.top + 10));
		LineTo((iRct.left + 21), (iRct.top + 8));
		LineTo((iRct.left + 24), (iRct.top + 8));
		LineTo((iRct.left + 23), (iRct.top + 9));
		break;
	}
}

/* this routine manages the extended askname dialog */
void
DialogAskName(asknameRec *pANR)
{
	GrafPtr			pOldPort;
	DialogTHndl		dTHnd;
	Handle			dIHnd;
	DialogRecord	dRec;
	short			i, iHit, iTyp;
	Handle			iHnd;
	Rect			iRct, iRct2;
	MenuHandle		mhndAskName[menuAN_total];
	short			mbarHgt = GetMBarHeight();
	short			ndxItem, menuEntry;
	Point			pt;

	if (! (dTHnd = (DialogTHndl) GetResource('DLOG', dlogAskName)))
		menuError(errGetANDlogTemplate);
	alignAD((Rect *) *dTHnd, mbarHgt);

	if (! (dIHnd = GetResource('DITL', dlogAskName)))
		menuError(errGetANDlogItems);

	GetPort(&pOldPort);
	/* to do: check if theWorld.hasColorQD ... */
	(void) GetNewDialog(dlogAskName, (Ptr) &dRec, (WindowPtr) 0L);
	if (ResError() || MemError())
		menuError(errGetANDialog);

	SetWRefCon((WindowPtr) &dRec, (long) pANR);
	for (i = 0; i < menuAN_total; i++) {
		if (! (mhndAskName[i] = NewMenu((dlogAskName + i), "\pPopup")))
			menuError(errANNewMenu);
	}

	/* role menu */
	for (i = 0; i < askn_role_end; i++)
		AppendMenu(mhndAskName[menuANRole], * (Str255 *) nhRole[i]);

	/* sex menu */
	for (i = 0; i < 2; i++)
		AppendMenu(mhndAskName[menuANSex], * (Str255 *) nhSex[i]);

	/* mode menu */
	for (i = 0; i < 3; i++)
		AppendMenu(mhndAskName[menuANMode], * (Str255 *) nhMode[i]);

	/* insert the popup menus */
	for (i = 0; i < menuAN_total; i++)
		InsertMenu(mhndAskName[i], hierMenu);

	for (i = uitmANOutlineDefault; i <= uitmANMode; i++)
	{
		GetDItem((DialogPtr)&dRec, i, &iTyp, &iHnd, &iRct);
		SetDItem((DialogPtr)&dRec, i, iTyp, (Handle) drawANUserItem, &iRct);
	}
	{
	short kind ; Handle item ; Rect area ;
	Str32 pName ;
		pName [ 0 ] = 0 ;
		if ( plname && plname [ 0 ] ) {
			strcpy ( ( char * ) pName , plname ) ;
			c2pstr ( ( char * ) pName ) ;
		} else {
			Handle h ;
			h = GetResource ( 'STR ' , -16096 ) ;
			if ( ( (Handle) NULL != h ) && ( GetHandleSize ( h ) > 0 ) ) {
				DetachResource ( h ) ;
				HLock ( h ) ;
				if ( * * h > 31 ) {
					* * h = 31 ;
				}
				BlockMove ( * h , pName , * * h + 1 )  ;
				DisposeHandle ( h ) ;
			}
		}
		if ( pName [ 0 ] ) {
			GetDItem ( ( WindowPtr ) & dRec , etxtANWho , & kind , & item , & area ) ;
			SetIText ( item , pName ) ;
			if ( pName [ 0 ] > 2 && pName [ pName [ 0 ] - 1 ] == '-' ) {
				(*pANR).anMenu[anRole] = ( strchr ( (char *) uitmChar [ 0 ] , pName [ pName [ 0 ] ] ) -
					(char *) uitmChar [ 0 ] ) ;
			}
		}
	}
	SelIText ( ( WindowPtr ) & dRec , etxtANWho , 0 , 32767 ) ;

	SetPort((GrafPtr) &dRec);
	ShowWindow((WindowPtr) &dRec);
	SelectWindow((WindowPtr) &dRec);

	iHit = bttnANQuit + 1;
	while (iHit > bttnANQuit)
	{
		/* adjust the askname menus */
		{
			static short	checkFeatures = 1;
			static short	currRole = -1, currSex = -1;
			unsigned char	anCavePerson[2][16] = {{"\pCaveman"}, {"\pCavewoman"}};
			unsigned char	anCleric[2][16] = {{"\pPriest"}, {"\pPriestess"}};

			if (checkFeatures)
			{
				checkFeatures = 0;

#ifndef TOURIST
				DisableItem(mhndAskName[menuANRole], (asknTourist + 1));
#endif
#ifndef WIZARD
				DisableItem(mhndAskName[menuANMode], (asknDebug + 1));
#endif
#ifndef EXPLORE_MODE
				DisableItem(mhndAskName[menuANMode], (asknExplore + 1));
#endif
			}

			/* check role 1st, valkyrie forces female */
			if (currRole != (*pANR).anMenu[anRole])
			{
				currRole = (*pANR).anMenu[anRole];

				if ((*pANR).anMenu[anRole] == asknValkyrie)
				{
					(*pANR).anMenu[anSex] = asknFemale;

					GetDItem((DialogPtr) &dRec, uitmANSex, &iTyp, &iHnd, &iRct);
					InvalRect(&iRct);

					/* disable male sex option */
					DisableItem(mhndAskName[menuANSex], (asknMale + 1));
				}
				else
					/* enable male sex option */
					EnableItem(mhndAskName[menuANSex], (asknMale + 1));
			}

			if (currSex != (*pANR).anMenu[anSex])
			{
				currSex = (*pANR).anMenu[anSex];

				SetItem(mhndAskName[menuANRole], (asknCaveman + 1),
						* (Str255 *) anCavePerson[currSex]);
				SetItem(mhndAskName[menuANRole], (asknPriest + 1),
						* (Str255 *) anCleric[currSex]);
			}
		}

		ModalDialog((ModalFilterProcPtr) 0L, &iHit);

		switch (iHit)
		{
		case bttnANPlay:		/* done!  let's play */
			break;

		case bttnANQuit:
			(*pANR).anMenu[anMode] = asknQuit;
			break;

		case uitmANRole:
		case uitmANSex:
		case uitmANMode:
			ndxItem = iHit - uitmANRole;

			/* invert the popup title */
			GetDItem((DialogPtr) &dRec, (iHit + 3), &iTyp, &iHnd, &iRct2);
			InvertRect(&iRct2);

			/* get the menu item */
			GetDItem((DialogPtr) &dRec, iHit, &iTyp, &iHnd, &iRct);
			pt = * (Point *) &iRct;
			LocalToGlobal(&pt);
			if (menuEntry = PopUpMenuSelect(mhndAskName[ndxItem],
							pt.v, pt.h, ((*pANR).anMenu[ndxItem] + 1)))
				/* set last item selected */
				(*pANR).anMenu[ndxItem] = LoWord(menuEntry) - 1;

			InvertRect(&iRct2);
			InvalRect(&iRct);
			break;

		case etxtANWho:
			/* limit the data here to 25 chars */
			{
				short beepTEDelete = 1;

				while ((**dRec.textH).teLength > 25)
				{
					if (beepTEDelete++ <= 3)
						SysBeep(3);
					TEKey('\b', dRec.textH);
				}
			}

			/* special case filter (that doesn't plug all the holes!) */
			if (((**dRec.textH).teLength == 1) && (**((**dRec.textH).hText) < 32))
				TEKey('\b', dRec.textH);

			/* if no name then disable the play button */
			GetDItem((DialogPtr) &dRec, bttnANPlay, &iTyp, &iHnd, &iRct);
			if ((**dRec.textH).teLength > 0)
				iTyp &= ~itemDisable;
			else
				iTyp |= itemDisable;
			HiliteControl((ControlHandle) iHnd, ((iTyp & itemDisable) ? 255 : 0));
			SetDItem((DialogPtr) &dRec, bttnANPlay, iTyp, iHnd, &iRct);
			break;
		}
	}

	if ((*pANR).anMenu[anMode] != asknQuit) {
/*
 * This is a good example of how NOT to get text from a dialog
 *
 *		HLock((Handle) (**dRec.textH).hText);
 *		BlockMove(*((**dRec.textH).hText), &((*pANR).anWho[1]), (**dRec.textH).teLength);
 *		(*pANR).anWho[0] = (**dRec.textH).teLength;
 *		HUnlock((Handle) (**dRec.textH).hText);
 */
		short kind ; Rect outline ; Handle item ;
		Str32 pName ;

		GetDItem ( ( WindowPtr ) & dRec , etxtANWho , & kind , & item , & outline ) ;
 		GetIText ( item , ( * pANR ) . anWho ) ;
		BlockMove ( pANR -> anWho , pName , pANR -> anWho [ 0 ] + 1 ) ;
		if ( pName [ 0 ] > 2 && pName [ pName [ 0 ] - 1 ] == '-' ) {
			(*pANR).anMenu[anRole] = ( strchr ( (char *) uitmChar [ 0 ] , pName [ pName [ 0 ] ] ) -
				(char *) uitmChar [ 0 ] ) ;
		}
	}

	/* cleanup (in reverse order) and leave */
	for (i = 0; i < menuAN_total; i++)
	{
		DeleteMenu(dlogAskName + i);
		DisposeMenu(mhndAskName[i]);
	}
	CloseDialog((DialogPtr)&dRec);
	DisposHandle(dRec.items);
	ReleaseResource(dIHnd);
	ReleaseResource((Handle) dTHnd);
	SetPort(pOldPort);
}

void
InitMenuRes()
{
static Boolean was_inited = 0 ;
short			i, j;
menuListHandle	mlHnd;
MenuHandle		mHnd;

	if ( was_inited ) {
		return ;
	}
	was_inited = 1 ;

	mustGetMenuAlerts();

	for (i = listMenubar; i <= listSubmenu; i++)
	{
		if (! (mlHnd = (menuListHandle) GetResource('MNU#', (menuBarListID + i))))
			menuError(errGetMenuList);

		pMenuList[i] = *mlHnd;

		for (j = 0; j < (**mlHnd).numMenus; j++)
		{
			if (! (mHnd = (MenuHandle) GetMenu((**mlHnd).mref[j].mresID))) {
			Str31 d ;
				NumToString ( (**mlHnd).mref[j].mresID , d ) ;
//				DebugStr ( d ) ;
				menuError(errGetMenu);
			}

			(**mlHnd).mref[j].mhnd = mHnd;
			* ((short *) *mHnd) = j + (**mlHnd).firstMenuID;	/* consecutive IDs */

			/* expand apple menu */
			if ((i == listMenubar) && (j == menuApple)) {
				AddResMenu(mHnd, 'DRVR');
			}

			InsertMenu(mHnd, ((i == listSubmenu) ? hierMenu : 0));
		}
	}

	DrawMenuBar();
}

void
AdjustMenus(short dimMenubar)
{
short		newMenubar = mbarRegular;
WindowPeek	peekWindow = (WindowPeek) FrontWindow();
short		i;

/*
 *	if ( windowprocs != mac_procs ) {
 *		return ;
 *	}
 */
	/* determine the new menubar state */
	if (dimMenubar) {
		newMenubar = mbarDim;
	} else if (! peekWindow) {
		newMenubar = mbarNoWindows;
	} else if (peekWindow->windowKind < 0) {
		newMenubar = mbarDA;
	} else {
		while (peekWindow && (peekWindow->windowKind != WKND_MAP)) {
			peekWindow = peekWindow->nextWindow;
		}
		if ((! peekWindow) || (! peekWindow->visible)) {
			newMenubar = mbarNoMap;
		}
	}

	if (newMenubar != mbarRegular)
		;							/* we've already found its state */
#ifdef WIZARD
	else if (wizard)
	{
		newMenubar = mbarSpecial;

		if (kAdjustWizardMenu)
		{
			kAdjustWizardMenu = 0;

			SetItem(MHND_FILE, menuFilePlayMode, "\pDebug");
		}
	}
#endif

#ifdef EXPLORE_MODE
	else if (discover)
	{
		newMenubar = mbarSpecial;

		if (kAdjustWizardMenu)
		{
			kAdjustWizardMenu = 0;

			SetItem(MHND_FILE, menuFilePlayMode, "\pExplore");

			for (i = CountMItems(MHND_WIZ); i > menuWizardAttributes; i--)
				DelMenuItem(MHND_WIZ, i);
		}
	}
#endif

	/* adjust the menubar, if there's a state change */
	if (theMenubar != newMenubar)
	{
		switch(theMenubar = newMenubar)
		{
		case mbarDim:
			/* disable all menus (except the apple menu) */
			for (i = menuFile; i < NUM_MBAR; i++)
				DisableItem(MBARHND(i), 0);
			break;

		case mbarNoWindows:
		case mbarDA:
		case mbarNoMap:
			/* enable the file menu, but ... */
			EnableItem(MHND_FILE, 0);

			if (theMenubar == mbarDA)
				DisableItem(MHND_FILE, menuFileOpenMap);
			else
				EnableItem(MHND_FILE, menuFileOpenMap);

			/* ... disable the window commands! */
			for (i = menuFileRedraw; i <= menuFileEnterExplore; i++)
				DisableItem(MHND_FILE, i);

			if (theMenubar != mbarNoWindows)
				EnableItem(MHND_FILE, menuFileClose);

			/* ... and disable the rest of the menus */
			for (i = menuEdit; i < NUM_MBAR; i++)
				DisableItem(MBARHND(i), 0);

			if (theMenubar == mbarDA)
				EnableItem(MHND_EDIT, 0);

			break;

		case mbarRegular:
		case mbarSpecial:
			/* enable all menus ... */
			for (i = menuFile; i < NUM_MBAR; i++)
				EnableItem(MBARHND(i), 0);

			/* ... except the unused Edit menu */
			DisableItem(MHND_EDIT, 0);

			/* ... the map is already open! */
			DisableItem(MHND_FILE, menuFileOpenMap);

			/* ... enable the window commands */
			for (i = menuFileRedraw; i <= menuFileEnterExplore; i++)
				EnableItem(MHND_FILE, i);

			if (theMenubar == mbarRegular)
				DisableItem(MHND_FILE, menuFilePlayMode);
			else
				DisableItem(MHND_FILE, menuFileEnterExplore);

			break;
		}

		DrawMenuBar();
	}
}

void
DoMenuEvt(long menuEntry)
{
	short menuID = HiWord(menuEntry);
	short menuItem = LoWord(menuEntry);

	switch(menuID - ID1_MBAR)	/* all submenus are default case */
	{
	case menuApple:
		if (menuItem == menuAppleAboutBox)
			aboutNetHack();
		else
		{
			unsigned char daName[32];

			GetItem(MHND_APPLE, menuItem, * (Str255 *) daName);
			(void) OpenDeskAcc(daName);
		}
		break;

	/*
	 * Those direct calls are ugly: they should be installed into cmd.c .
	 * Those AddToKeyQueue() calls are also ugly: they should be put into
	 * the 'STR#' resource.
	 */
	case menuFile:
		switch(menuItem)
		{
		case menuFileOpenMap:
			openMap();
			break;

		case menuFileRedraw:
			AddToKeyQueue ( 'R' & 0x1f , 1 ) ;
			break;

		case menuFilePrevMsg:
			AddToKeyQueue ( 'P' & 0x1f , 1 ) ;
			break;

		case menuFileCleanup:
			(void) SanePositions();
			break;

		case menuFileClose:
			closeFrontWindow();
			break;

		case menuFileEnterExplore:
			AddToKeyQueue ( 'X' , 1 ) ;
			break;

		case menuFileOptionEdit:
			optionEditor();
			break;

		case menuFileSave:
			askSave();
			break;

		case menuFileQuit:
			askQuit();
			break;
		}
		break;

	case menuEdit:
		(void) SystemEdit(menuItem - 1);
		break;

	default:	/* get associated string and add to key queue */
		{
			Str255	mstr;
			short	i;

			GetIndString(mstr, menuID, menuItem);
			if (mstr[0] > QUEUE_LEN)
				mstr[0] = QUEUE_LEN;

			for (i = 1; ((i <= mstr[0]) && (mstr[i] != mstrEndChar)); i++)
				AddToKeyQueue(mstr[i], false);
		}
		break;
	}

	HiliteMenu(0);
}


static void
aboutNetHack() {
	if (theMenubar >= mbarRegular) {
		(void) doversion();				/* is this necessary? */
	} else {
	unsigned char aboutStr[32] = "\pNetHack 3.1.";

		if (PATCHLEVEL > 10) {
			aboutStr[++aboutStr[0]] = '0'+PATCHLEVEL/10;
		}

		aboutStr[++aboutStr[0]] = '0' + (PATCHLEVEL % 10);

		ParamText(aboutStr, "\p\rnethack-bugs@linc.cis.upenn.edu", "\p", "\p");
		(void) Alert(alrtMenuNote, (ModalFilterProcPtr) 0L);
		ResetAlrtStage();
	}
}

static void
openMap()
{
	WindowPeek	peekWindow = *(WindowPeek*) WindowList;

	while (peekWindow && (peekWindow->windowKind != WKND_MAP))
		peekWindow = peekWindow->nextWindow;

	if (! peekWindow)
		return;				/* impossible? */

	ShowWindow((WindowPtr) peekWindow);
	SelectWindow((WindowPtr) peekWindow);
}

static void
closeFrontWindow()
{
	WindowPeek	peekWindow = (WindowPeek) FrontWindow();

	if (! peekWindow)
		return;				/* impossible? */
	else if (peekWindow->windowKind < 0)
		CloseDeskAcc(peekWindow->windowKind);
	else if (peekWindow->windowKind == WKND_MAP)
		HideWindow((WindowPtr) peekWindow);
	else
		WindowGoAway((EventRecord *) 0L, (WindowPtr) peekWindow);
}

static void
optionEditor()
{
	ParamText("\pSorry, not yet implemented!  Use Options on the Help menu.", "\p", "\p", "\p");
	(void) Alert(alrtMenuNote, (ModalFilterProcPtr) 0L);
	ResetAlrtStage();
}

static void
askSave() {
Boolean doSave = 1 ;
Boolean doYes = 0 ;

	if (theMenubar < mbarRegular) {
	short	itemHit;

		ParamText("\pReally Save?", "\p", "\p", "\p");
		itemHit = Alert(alrtMenu_NY, (ModalFilterProcPtr) 0L);
		ResetAlrtStage();

		if (itemHit != bttnMenuAlertYes) {
			doSave = 0 ;
		} else {
			doYes = 1 ;
		}
	}
	if ( doSave ) {
		AddToKeyQueue ( 'S' , 1 ) ;
		if ( doYes ) {
			AddToKeyQueue ( 'y' , 1 ) ;
		}
	}
}

static void
askQuit() {
Boolean doQuit = 1 ;
Boolean doYes = 0 ;

	if (theMenubar < mbarRegular) {
	short	itemHit;

		ParamText("\pReally Quit?", "\p", "\p", "\p");
		itemHit = Alert(alrtMenu_NY, (ModalFilterProcPtr) 0L);
		ResetAlrtStage();

		if (itemHit != bttnMenuAlertYes) {
			doQuit = 0 ;
		} else {
			doYes = 1 ;
		}
	}
	if ( doQuit ) {
		AddToKeyQueue ( 'Q' , 1 ) ;
		if ( doYes ) {
			AddToKeyQueue ( 'y' , 1 ) ;
		}
	}
}
