/*	SCCS Id: @(#)macinit.c	3.0	88/08/05
/* Copyright (c) Johnny Lee, 1989.		 */
/* NetHack may be freely redistributed.  See license for details. */

/*	Initialization routine for the Macintosh */

#include	"hack.h"

#ifdef MACOS

/* Global variables */
extern WindowPtr	HackWindow;	/* points to NetHack's window */
char	*keys[8];
short macflags;
typedef struct defaultData {
	long	defaultFlags;
	long	fontSize;
	Str255	fontName;
} defaultData;
#define	fDFZoomWindow	0x02L
#define	fDFUseDefaultFont	0x01L


int
initterm(row, col)
short	row, col;
{
	register short	i, j;
	short		tempFont, tempSize, fontNum, size;
	char	*l;
	EventRecord	theEvent;
	FontInfo	fInfo;
	Handle	temp;
	MenuHandle	theMenu;
	OSErr	error;
	Rect		boundsRect;
	Str255	appName, font;
	defaultData	*dD;
	term_info	*t;
	
	/* standard Mac initialization */
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	InitGraf(&MAINGRAFPORT);
	
	InitFonts();
	InitWindows();
	InitMenus();
	InitCursor();
	FlushEvents(everyEvent, 0);
	if (error = GetVol((StringPtr)&appName, &tempSize))
		SysBeep(1);
	
	/* Application-specific startup code */
	theMenu = NewMenu(appleMenu, "\001\024");	/*  apple menu  */
	AppendMenu(theMenu,"\030About NetHack 3.0g\311;(-");
	AddResMenu(theMenu, 'DRVR');
	InsertMenu(theMenu, 0);
	DisableItem(theMenu,0);

	t = (term_info *)malloc(sizeof(term_info));
	t->recordVRefNum = tempSize;
	
	for (i = fileMenu; i <= extendMenu; i++) {
		theMenu = GetMenu(i);
		if (theMenu) {
			InsertMenu(theMenu, 0);
			DisableItem(theMenu, 0);
		}
		if (i == editMenu) {
			t->shortMBarHandle = GetMenuBar();
		}
	}
	t->fullMBarHandle = GetMenuBar();
	
	DrawMenuBar();
	HiliteMenu(0);
	for (i = 0;i <= 7;i++) {
		temp = GetResource(HACK_DATA,(i + 100 + appleMenu));
		if (!temp) {
			SysBeep(1);
			panic("Can't get MENU_DATA resource");
		}
		MoveHHi(temp);
		HLock(temp);
		DetachResource(temp);
		keys[i] = *temp;
	}

	macflags = (fToggleNumPad | fDoNonKeyEvt);
	
	/* Set font to monaco, user-defined font or to Hackfont if available */
	size = 9;
	strcpy((char *)&font[0], "\006Monaco");
	
	temp = GetResource(HACK_DATA, DEFAULT_DATA);
	if (temp) {
		HLock(temp);
		dD = (defaultData *)(*temp);
		size = (short)dD->fontSize;
		strncpy((char *)&font[0], (char *)&dD->fontName[0],
					(short)dD->fontName[0] + 1);
		if (dD->defaultFlags & fDFZoomWindow)
			macflags |= fZoomOnContextSwitch;
		HUnlock(temp);
		ReleaseResource(temp);
	}
			
	tempFont = MAINGRAFPORT->txFont;
	tempSize = MAINGRAFPORT->txSize;
	GetFNum(font, &fontNum);
	TextFont(fontNum);
	TextSize(size);
	GetFontInfo(&fInfo);
	TextFont(tempFont);
	TextSize(tempSize);
	
	if (!(dD->defaultFlags & fDFUseDefaultFont)) {
		Strcpy((char *)&appName[0], "\010HackFont");
		GetFNum(appName,&tempFont);
		if (tempFont) {
			fontNum = tempFont;
			tempFont = MAINGRAFPORT->txFont;
			TextFont(fontNum);
			TextSize(size);
			GetFontInfo(&fInfo);
			TextFont(tempFont);
			TextSize(tempSize);
			macflags |= fUseCustomFont;
		}
	}
	
	i = fInfo.ascent + fInfo.descent + fInfo.leading;
	j = fInfo.widMax;
	if ((row * i + 2 * Screen_Border) >
		 (SCREEN_BITS.bounds.bottom - SCREEN_BITS.bounds.top)
		 ||
		 (col * j + 2 * Screen_Border) >
		 	(SCREEN_BITS.bounds.right - SCREEN_BITS.bounds.left)) {
		size = 9;
		Strcpy((char *)&font[0], "\006Monaco");
		tempFont = MAINGRAFPORT->txFont;
		tempSize = MAINGRAFPORT->txSize;
		GetFNum(font, &fontNum);
		TextFont(fontNum);
		TextSize(size);
		GetFontInfo(&fInfo);
		TextFont(tempFont);
		TextSize(tempSize);
		i = fInfo.ascent + fInfo.descent + fInfo.leading;
		j = fInfo.widMax;
		macflags &= ~fUseCustomFont;
	}		
		 
	t->ascent = fInfo.ascent;
	t->descent = fInfo.descent;
	t->height = i;
	t->charWidth = j;
	
	t->fontNum = fontNum;
	t->fontSize = size;
	t->maxRow = row;
	t->maxCol = col;
	t->tcur_x = 0;
	t->tcur_y = 0;
	t->auxFileVRefNum = 0;
	if (error = SysEnvirons(1, &(t->system))) {
		SysBeep(1);
	}

#define	KEY_MAP	103
	temp = GetResource(HACK_DATA, KEY_MAP);
	if (temp) {
		MoveHHi(temp);
		HLock(temp);
		DetachResource(temp);
		t->keyMap = (char *)(*temp);
	} else
		panic("Can't get keymap resource");

	SetRect(&boundsRect, LEFT_OFFSET, TOP_OFFSET + 10,
		(col * fInfo.widMax) + LEFT_OFFSET + 2 * Screen_Border,
		TOP_OFFSET + (row * t->height) + 2 * Screen_Border + 10);
	
	t->screen = (char **)malloc(row * sizeof(char *));
	l = malloc(row * col * sizeof(char));
	for (i = 0;i < row;i++) {
		t->screen[i] = (char *)(l + (i * col * sizeof(char)));
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < col; j++) {
			t->screen[i][j] = ' ';
		}
	}

	/* give time for Multifinder to bring NetHack window to front */
	for(tempFont = 0; tempFont<10; tempFont++) {
		(void)GetNextEvent(everyEvent,&theEvent);
	}

	HackWindow = NewWindow(0L, &boundsRect, "\015NetHack [MOV]",
			TRUE, noGrowDocProc, (WindowPtr)-1, FALSE, (long)t);

	t->inColor = 0;
#ifdef TEXTCOLOR
	t->color[0] = blackColor;
	t->color[1] = redColor;
	t->color[2] = greenColor;
	t->color[3] = yellowColor;
	t->color[4] = blueColor;
	t->color[5] = magentaColor;
	t->color[6] = cyanColor;
	t->color[7] = whiteColor;

	if (t->system.hasColorQD) {
		Rect	r;
		GDHandle	gd;
		
		r = (**(*(WindowPeek)HackWindow).contRgn).rgnBBox;
		LocalToGlobal(&r.top);
		LocalToGlobal(&r.bottom);
		gd = GetMaxDevice(&r);
		t->inColor = (**(**gd).gdPMap).pixelSize > 1;
	}
#endif
		
	temp = GetResource(HACK_DATA, MONST_DATA);
	if (temp) {
		DetachResource(temp);
		MoveHHi(temp);
		HLock(temp);
		i = GetHandleSize(temp);
		mons = (struct permonst *)(*temp);
	} else {
		panic("Can't get MONST resource data.");
	}
	
	temp = GetResource(HACK_DATA, OBJECT_DATA);
	if (temp) {
		DetachResource(temp);
		MoveHHi(temp);
		HLock(temp);
		i = GetHandleSize(temp);
		objects = (struct objclass *)(*temp);
		for (j = 0; j< NROFOBJECTS+1; j++) {
			objects[j].oc_name = sm_obj[j].oc_name;
			objects[j].oc_descr = sm_obj[j].oc_descr;
		}
	} else {
		panic("Can't get OBJECT resource data.");
	}
	
	(void)aboutBox(0);	
	return 0;
}

/* not really even needed. NH never gets to the end of main(), */
/* so this never gets called */
int
freeterm()
{
	return 0;
}

#ifdef SMALLDATA
/* SOME [:-( ] Mac compilers have a 32K global & static data limit */
/* these routines help the HANDICAPPED things */
void
init_decl()
{
	short	i;
	char	*l;
	extern char **Map;
	
	l = calloc(COLNO , sizeof(struct rm **));
	level.locations = (struct rm **)l;
	l = calloc(ROWNO * COLNO , sizeof(struct rm));
	for (i = 0; i < COLNO; i++) {
	    level.locations[i] = 
		(struct rm *)(l + (i * ROWNO * sizeof(struct rm)));
	}
	
	l = calloc(COLNO , sizeof(struct obj ***));
	level.objects = (struct obj ***)l;
	l = calloc(ROWNO * COLNO , sizeof(struct obj *));
	for (i = 0; i < COLNO; i++) {
	    level.objects[i] = 
		(struct obj **)(l + (i * ROWNO * sizeof(struct obj *)));
	}
	
	l = calloc(COLNO , sizeof(struct monst ***));
	level.monsters = (struct monst ***)l;
	l = calloc(ROWNO * COLNO , sizeof(struct monst *));
	for (i = 0; i < COLNO; i++) {
	    level.monsters[i] = 
		(struct monst **)(l + (i * ROWNO * sizeof(struct monst *)));
	}
	level.objlist = (struct obj *)0L;
	level.monlist = (struct monst *)0L;
	
l = calloc(COLNO, sizeof(char *));
Map = (char **)l;
l = calloc(ROWNO * COLNO, sizeof(char));
for (i = 0; i < COLNO; i++) {
    Map[i] = 
	(char *)(l + (i * ROWNO * sizeof(char)));
}

}

/* Since NetHack usually exits before reaching end of main()	*/
/* this routine could probably left out.	- J.L.		*/
void
free_decl()
{

	free((char *)level.locations[0]);
	free((char *)level.locations);
	free((char *)level.objects[0]);
	free((char *)level.objects);
	free((char *)level.monsters[0]);
	free((char *)level.monsters);
}
#endif /* SMALLDATA */
#endif /* MACOS */
