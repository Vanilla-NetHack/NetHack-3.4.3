/*	SCCS Id: @(#)mac.c	3.0	88/08/05
/*      Copyright (c) Johnny Lee  1989	*/ 
/* NetHack may be freely redistributed.  See license for details. */

/*	Source file for character I/O and miscellaneous */
/*	user interface routines for the macintosh */

#include	"hack.h"

/* Global variables */
WindowPtr	HackWindow;	/* points to Hack's window */
extern char	*keys[8];
extern short macflags;

int
tgetch()

{
	char	ch;
	EventRecord	theEvent;
	Rect cursorRect;
	long	message,cursorTime,start;
	MenuHandle	theMenu;
	short	keyCode;
	term_info	*t;
	boolean	noControlKey;
	GrafPtr	oldPort,oldPort1;
	static char nextCommand;
	short aboutBox();
	char mButtonDown();
#define noEscapeKey	noControlKey
#define	clearKey	0x47
#define ESCAPEkey	0x1B
	
	t = (term_info *)GetWRefCon(HackWindow);
	cursorRect.left = t->tcur_x * t->charWidth + Screen_Border;
	cursorRect.right = cursorRect.left + t->charWidth - 1;
	cursorRect.top = t->height * t->tcur_y + Screen_Border;
	cursorRect.bottom = cursorRect.top + t->height;
	cursorTime = GetCaretTime()/2;
	keyCode = 0;
	noControlKey = (t->system.keyBoardType <= envMacPlusKbd) ? TRUE : FALSE;
	
	/* permit use of cursor keys and numeric keypad */
	/* does the same translation as in msdos.c but includes cursor keys */
	ch = '\0';
	/* handle extended command from menu */
	if (nextCommand && (macflags & (fExtCmdSeq1 | fExtCmdSeq2 | fExtCmdSeq3))) {
	    if (macflags & fExtCmdSeq1) {
		ch = '#';
		macflags = macflags ^ (fExtCmdSeq1 | fExtCmdSeq2);
	    } else if (macflags & fExtCmdSeq2) {
		ch = nextCommand;
		macflags = macflags ^ (fExtCmdSeq2 | fExtCmdSeq3);
		if (!(macflags & fExtCmdSeq3))
		    nextCommand = '\0';
	    } else if (macflags & fExtCmdSeq3) {
		ch = '\r';
		macflags &= ~fExtCmdSeq3;
	    }
	}
	GetPort(&oldPort);
	SetPort(HackWindow);
	while (!ch) {
	    GetPort(&oldPort1);
	    SetPort((GrafPtr)HackWindow);
	    /* do cursor blinking */
	    message = TickCount() + cursorTime;
	    while (!WaitNextEvent(everyEvent, &theEvent, 10L, 0L)) {
		/* wait until something occurs */
		if (!Invisible) {
		    if (TickCount() > message) {
			InvertRect(&cursorRect);
			message = TickCount() + cursorTime;
			keyCode = !keyCode;
		    }
		}
	    }
	    if (keyCode)
		InvertRect(&cursorRect);
	    SetPort(oldPort1);
	    if (theEvent.what == keyDown || theEvent.what == autoKey) {
		/* use raw key codes */
		keyCode = (LoWord(theEvent.message) & keyCodeMask)>>8;
		if (keyCode == 50 && noEscapeKey)
		    ch = (char)ESCAPEkey;	/* ESC */
		else if (keyCode == clearKey) {
		    macflags = macflags ^ fToggleNumPad;
		    SetWTitle(HackWindow,
			    (macflags & fToggleNumPad)	? "\015NetHack [MOV]"
							: "\015NetHack [NUM]");
		} else if ((macflags & fToggleNumPad) &&
				(keyCode>64 && keyCode<127))
		    ch = t->keyMap[keyCode-65];
		/* make the command key = control key on old Mac keyboards */
		if ((theEvent.modifiers & cmdKey) && noControlKey) {
		    ch = (char)(theEvent.message & 0x1F);
		}
		if ((theEvent.modifiers & shiftKey) && (ch)) {
		    ch = (ch == '.') ? ':' : (char)toupper(ch);
		}
		if (theEvent.modifiers & optionKey) {
		    for(start = 43; start < 56; start++) {
			if (t->keyMap[start] == (char)(theEvent.message & 0xFFL)) {
			    ch = t->keyMap[start - 13];
			    break;
			}
		    }
		}
		/* not a cursor key or from the numeric keypad */
		if (!ch && keyCode != clearKey) {
		    ch = (char)(theEvent.message & 0xFF);
		}
	    } else {
		/* what other events to handle */
		switch (theEvent.what) {		
		    case nullEvent:
			SetCursor(&ARROW_CURSOR);
			break;
		    case app4Evt:
#define	kSuspendResumeMessage 1  /* high byte of suspend/resume event message */
#define	SuspResIsResume(evtMessage)	((evtMessage) & 0x00000001)

			switch (theEvent.message >> 24) {
			    case kSuspendResumeMessage:
				if (!SuspResIsResume(theEvent.message)) {
				    /* ---------- SUSPEND EVENT ------------ */
				    if (macflags & fZoomOnContextSwitch)	
					SizeWindow(HackWindow,60,60,TRUE);
				} else {
				    /* ---------- RESUME EVENT ------------- */
				    if (macflags & fZoomOnContextSwitch)	
					SizeWindow(HackWindow,
					 (t->maxCol * t->charWidth) + 2*Screen_Border,
					 (t->maxRow * t->height) + 2*Screen_Border,
					 TRUE);
				}
				break;
			}
			break;
		
		    case updateEvt:
			if (HackWindow == (WindowPtr)theEvent.message) {
#ifdef TEXTCOLOR
			    if (t->system.hasColorQD) {
				Rect	r;
				GDHandle	gd;
				
				r = (**(*(WindowPeek)HackWindow).contRgn).rgnBBox;
				LocalToGlobal(&r.top);
				LocalToGlobal(&r.bottom);
				gd = GetMaxDevice(&r);
				HLock((Handle)gd);
				t->inColor = (**(**gd).gdPMap).pixelSize > 1;
				HUnlock((Handle)gd);
			    }
#endif
			    GetPort(&oldPort);
			    SetPort((GrafPtr)HackWindow);
			    BeginUpdate(HackWindow);
			    t = (term_info *)GetWRefCon(HackWindow);
			    if (t->inColor && macflags & fDoUpdate) {
				keyCode = flags.toplin;
				flags.toplin = 0;
				docrt();
				flags.toplin = keyCode;
			    } else {
				for (keyCode = 0;keyCode < t->maxRow; keyCode++) {
				    MoveTo(Screen_Border,
					    t->ascent + (keyCode * t->height)
						    + Screen_Border);
				    DrawText(&t->screen[keyCode][0], 0, t->maxCol);
				}
			    }
			    EndUpdate(HackWindow);
			    SetPort(oldPort);
			} 
			break;
				
		    case activateEvt:
			if (HackWindow == (WindowPtr)theEvent.message) {
			    if (theMenu = GetMHandle(editMenu)) {
				if (theEvent.modifiers & activeFlag) {
				    if (macflags & fDoUpdate) {
					SetMenuBar(t->fullMBarHandle);
					for (keyCode = fileMenu;
					      keyCode <= extendMenu;keyCode++) {
					    if (keyCode != editMenu)
						EnableItem(GetMHandle(keyCode), 0);
					}
					EnableItem(GetMHandle(appleMenu), 1);
				    }
				    DisableItem(theMenu, 0);
				} else {
				    EnableItem(theMenu, 0);
				    if (macflags & fDoUpdate) {
					SetMenuBar(t->shortMBarHandle);
					for (keyCode = fileMenu;
					      keyCode <= extendMenu;keyCode++) {
					    if (keyCode != editMenu)
						DisableItem(GetMHandle(keyCode), 0);
					}
					DisableItem(GetMHandle(appleMenu), 1);
				    }
				}
				DrawMenuBar();
			    }
			}
			break;
				
		    case mouseDown:
			ch = mButtonDown(theEvent, t, &nextCommand);
			break;
		}
	    }		
	}
	SetPort(oldPort);
	ObscureCursor();
	return ((ch == '\r') ? '\n' : ch);
}

char
mButtonDown(theEvent, t, nextCommand)
EventRecord	theEvent;
term_info	*t;
char	*nextCommand;
{
	Rect	boundsRect;
	WindowPtr	theWindow;
	long	message;
	char	deskacc[256];
	MenuHandle	theMenu;
	char	ch;
	short	menuBar;
	GrafPtr	oldPort;

	ch = '\0';
	if (macflags & fDoNonKeyEvt) 
	switch (FindWindow(theEvent.where,&theWindow)) {
	    case inMenuBar:

		SetCursor(&ARROW_CURSOR);
		message = MenuSelect(theEvent.where);

		if (!HiWord(message))
		    break;

		switch (HiWord(message)) {
		    case editMenu:
			(void)SystemEdit((short)message - 1);
			break;
		    case appleMenu:
			if (LoWord(message) > 2) {
			    GetItem(GetMHandle(HiWord(message)),LoWord(message),
					deskacc);
			    SetMenuBar(t->shortMBarHandle);
			    DrawMenuBar();
			    (void)OpenDeskAcc(deskacc);
			    if (theMenu = GetMHandle(editMenu))
				    EnableItem(theMenu, 0);
			} else
			    if (aboutBox(1))
				    ch = '?';
			break;
		    case fileMenu:
		    case inventMenu:
		    case prepMenu:
		    case actionMenu:
		    case moveMenu:
			*nextCommand =
			    keys[HiWord(message)-appleMenu][LoWord(message)-1];
			ch = (!(*nextCommand)) ? '\0' : (char)ESCAPEkey;
			macflags |= (fExtCmdSeq2 | fExtCmdSeq3);
#ifdef TEXTCOLOR
#define	MAC_BLACK	0
#define	MAC_WHITE	7
			if(HiWord(message) == fileMenu && LoWord(message) == 3) {
			    theMenu = GetMHandle(HiWord(message));
			    macflags = macflags ^ fInvertedScreen;
			    CheckItem(theMenu, 3,
					(boolean)(macflags & fInvertedScreen));

			    /* switch black & white */
			    message = t->color[BLACK];
			    t->color[MAC_BLACK] = t->color[MAC_WHITE];
			    t->color[MAC_WHITE] = message;

			    /* switch light blue & dark blue */
			    message = t->color[BLUE];
			    t->color[BLUE] = t->color[CYAN];
			    t->color[CYAN] = message;

			    ForeColor(t->color[MAC_BLACK]);
			    BackColor(t->color[MAC_WHITE]);
			    /* refresh screen without prompting 'More' */
			    message = flags.toplin;
			    flags.toplin = 0;
			    docrt();
			    flags.toplin = message;
			}
#endif
			break;
		    case extendMenu:
			ch = (char)ESCAPEkey;
			*nextCommand =
			 keys[HiWord(message) - appleMenu][LoWord(message) - 1];
			macflags |= fExtCmdSeq1;
			break;
		}
		HiliteMenu(0);
		break;
		
	    case inSysWindow:
		SystemClick(&theEvent, theWindow);
		break;

	    case inDrag:
		if (!(theEvent.modifiers & cmdKey)) {
			if (theWindow != FrontWindow()) {
				SelectWindow(theWindow);
				break;
			}
		}
		menuBar = (ROM85 == -1) ? 20 : GetMBarHeight();

		SetRect(&boundsRect, Screen_Border, menuBar + Screen_Border,
			SCREEN_BITS.bounds.right - Screen_Border,
			SCREEN_BITS.bounds.bottom - Screen_Border);
		SetCursor(&ARROW_CURSOR);
		DragWindow(theWindow, theEvent.where, &boundsRect);
		break;
	
	    case inContent:
		SelectWindow(theWindow);
		break;
	}
	return ch;
}

void
gethdate(name) char *name;
{
/*end gethdate*/}

int
uptodate(fd)
{
	return(1);
}

#ifndef THINKC4
char *
getenv(s)
char *s;
{
	return((char *)NULL);
}

int
memcmp(x,y,n)
char *x,*y;
int	n;

{
	int i;
	
	i = 0;
	while (i++< n && (*x++) == (*y++)) {
		/*x++; y++; i++*/
		;
	}
	if (i != n)
		return ((*x > *y) ? -1 : 1);
	else
		return (0);
}
#else
int
kbhit()
{	
	EventRecord	theEvent;
	
	return (EventAvail(keyDownMask, &theEvent));
}
#endif

#ifdef AZTEC

sleep(x)
int	x;
{
	long t;
	
	Delay((long)x, &t);
}
#endif
	

int
mcurs(col,row)
short	col,row;

{
	term_info	*t;
	
	t = (term_info *)GetWRefCon(HackWindow);
	t->tcur_y = row;
	t->tcur_x = col;
	return 1;
}

void
mput(s)
char	*s;
{
	unsigned short	sLen,temp;
	GrafPtr		prevPort;
	register term_info	*t;
	Point		cur;
	register short		x,y;
	Rect		eraseRect;
	register char		*stmp,*c,*c1;
	char	savech;
	
	t = (term_info *)GetWRefCon(HackWindow);
	sLen = strlen(s);
	
	x = t->tcur_x;
	y = t->tcur_y;
	if (y >= t->maxRow)
		panic("mput - incorrect cursor position\n");
	cur.h = (x * t->charWidth) + Screen_Border;
	cur.v = t->ascent + (y * t->height) + Screen_Border;
	GetPort(&prevPort);
	SetPort((GrafPtr)HackWindow);
	TextFont(t->fontNum);
	TextSize(t->fontSize);
	TextMode(srcCopy);
	macflags &= ~0x1000;
	/* a termcap-type escape string */
	if (!strncmp(s, "\033[", 2)) {
	    switch(*(s + 2)) {
		case 'c':	/* color kluge */
		    if (t->inColor) {
			temp = (short)(*(s + 3) - 'a');
			if (temp >= BLACK && temp < MAXCOLORS &&
			    (temp % (MAXCOLORS / 2) != GRAY))	/* set colour */
				ForeColor(t->color[temp % (MAXCOLORS / 2)]);
			if (temp == GRAY)
				macflags |= 0x1000;
			 /* yellow fgnd hard to see on white bgnd */
			 /* so change to green background */
			if (temp == YELLOW || temp == BROWN)
				BackColor(t->color[GREEN]);
			if (temp == BLUE)
				BackColor(t->color[CYAN]);
			if (temp == CYAN)
				BackColor(t->color[BLUE]);
		    }
		    break;
		case '0':	/* normal video begin */
		    if (*(s + 3) == 'm') {
			    ForeColor(t->color[0]);
			    BackColor(t->color[7]);
		    }
		    break;
		case '1':	/* inverse video begin */
		    if (*(s + 3) == 'm') {
			    ForeColor(t->color[7]);
			    BackColor(t->color[0]);
		    }
		    break;
		case 'A':	/* cursor up */
		    if (y > 0) {
			    t->tcur_y--;
			    cur.v -= t->height;
		    }
		    break;
		case 'B':	/* cursor down */
		    if (y < t->maxRow) {
			    t->tcur_y++;
			    cur.v += t->height;
		    }
		    break;
		case 'C':	/* cursor right */
		    if (x < t->maxCol) {
			    t->tcur_x++;
			    cur.h += t->charWidth;
		    }
		    break;
		case 'D':	/* cursor left */
		    if (x > 0) {
			    t->tcur_x--;
			    cur.h -= t->charWidth;
		    }
		    break;
		case 'H':	/* home cursor */
		    t->tcur_x = t->tcur_y = 0;
		    cur.h = Screen_Border;
		    cur.v = t->ascent + Screen_Border;
		    break;
		case 'K':	/* clear to end of line */
		    eraseRect.top = cur.v - (t->ascent);
		    eraseRect.left = cur.h;
		    eraseRect.bottom = eraseRect.top + t->height;
		    eraseRect.right = (t->maxCol*t->charWidth) + Screen_Border;
		    EraseRect(&eraseRect);
		    for (temp = x, c = &(t->screen[y][x]); temp < t->maxCol; temp++)
			    *c++ = ' ';
		    break;
		case '2':
		    if (*(s+3) == 'J') {	/* clear screen */
			x = y = t->tcur_x = t->tcur_y = 0;
			eraseRect.top = eraseRect.left = Screen_Border;
			eraseRect.bottom = t->maxRow*t->height + Screen_Border;
			eraseRect.right = t->charWidth*t->maxCol + Screen_Border;
			EraseRect(&eraseRect);
			for (y = 0, c = t->screen[0]; y < t->maxCol * t->maxRow; y++) {
					*c++ = ' ';
			}
			cur.h = Screen_Border;
			cur.v = t->ascent + Screen_Border;
		    }
		    break;
	    }
	    MoveTo(cur.h, cur.v);
	} else {
	    short	charleft = sLen;
	    
	    MoveTo(cur.h, cur.v);
	    stmp = s;
	    
	    if (sLen) {
		while (stmp < (s + sLen)) {
		    temp = (x + charleft - 1 < t->maxCol) ? sLen : t->maxCol - x;
		    savech = '\0';
		    c1 = stmp + temp;	/* point to the char after the end */
		    c = index(stmp, '\n');
		    if (c && c < c1) {
			    c1 = c;
			    savech = '\n';
			    temp = (short)(c - stmp);
			    /* don't want to include '\n' in print */
		    }
		    c = index(stmp, '\r');
		    if (c && c < c1) {
			    c1 = c;
			    savech = '\r';
			    temp = (short)(c - stmp);
			    /* don't want to include '\r' in print */
		    }
		    DrawText((Ptr)stmp, 0, temp);
		    BlockMove((Ptr)stmp, (Ptr)&(t->screen[y][x]), (long)temp);
		    stmp += temp + 1;
		    charleft -= temp + 1;
		    if (!savech) {
			    t->tcur_x += temp;
		    }
		    if (t->tcur_x == t->maxCol || savech) {
			t->tcur_x = 0;
			if (savech != '\r')
				y = (++t->tcur_y);
				/* should probably check for overflow */
		    }
		    x = t->tcur_x;
		    cur.h = Screen_Border;
		    cur.v = y * t->height + t->ascent + Screen_Border;
		    MoveTo(cur.h,cur.v);
		}
	    }
	}
	if (t->tcur_x >= t->maxCol) {
	    t->tcur_x = t->tcur_x % t->maxCol;
	    t->tcur_y++;
	}
	SetPort(prevPort);
}

int
mputc(c)
char	c;
{
	GrafPtr		prevPort;
	register term_info	*t;
	Point		cur;
	register short		x,y;
	Rect		eraseRect;
	char		savech;
	PenState	pnState;
	
	t = (term_info *)GetWRefCon(HackWindow);
	
	x = t->tcur_x;
	y = t->tcur_y;
	cur.h = (x * t->charWidth) + Screen_Border;
	cur.v = t->ascent + (y * t->height) + Screen_Border;
	GetPort(&prevPort);
	SetPort((GrafPtr)HackWindow);
	TextFont(t->fontNum);
	TextSize(t->fontSize);
	TextMode(srcCopy);
	
	MoveTo(cur.h, cur.v);
	savech = '\0';
	if (c == '\b') {
		if (x > 0) {
			c = ' ';
			x = (--t->tcur_x);
			cur.h = (x * t->charWidth) + Screen_Border;
			Move(-t->charWidth,0);
			savech = '\b';
		} else if (y > 0) {
				c = ' ';
				x = t->tcur_x = (t->maxCol - 1);
				y = (--t->tcur_y);
				cur.h = (x * t->charWidth) + Screen_Border;
				cur.v -= t->height;
				MoveTo(cur.h, cur.v);
				savech = '\b';
		}
	}
	if (c == '\007') {
		SysBeep(1);
	} else if ((c != '\n') && (c != '\r')) {
		t->screen[y][x] = c;
		if (macflags & 0x1000) {
			eraseRect.top = cur.v -t->ascent;
			eraseRect.bottom = eraseRect.top + t->height;
			eraseRect.left = cur.h;
			eraseRect.right = cur.h + t->charWidth;
			GetPenState(&pnState);
			FillRect(&eraseRect, dkGray);
			SetPenState(&pnState);
			macflags &= ~0x1000;
			TextMode(srcOr);
		}
		DrawText(&c, 0, 1);
		if (!savech)
			t->tcur_x++;
	} else {
		t->tcur_x = 0;
		if (t->tcur_y < t->maxRow) {
			if (c == '\n')
				t->tcur_y++;
		}
		cur.h = (t->tcur_x * t->charWidth) + Screen_Border;
		cur.v = t->ascent + (t->tcur_y * t->height) + Screen_Border;
		MoveTo(cur.h,cur.v);
	}
	SetPort(prevPort);	
	return 1;
}

int
mputs(s)
char	*s;
{
	mput(s);
	return 1;
}
		
		
		
int
mprintf(fstr)
char	*fstr;
{
#define	Bufsz	14		
	char		numAsStr[Bufsz];
	short		numsz;
	char		*ps;
	unsigned long	num;

	boolean		convchar;
	boolean		islong;
	char		c;
	char		*s;
	char		prBuffer[128];
	register char		*pb;
	
	prBuffer[0] = '\0';
	pb = &prBuffer[0];
	ps = (char *)&fstr;	/* convert to pointer to params */
	ps += sizeof(char *);	/* skip over format string ptr */
	while (*fstr)	{
	    s = index(fstr, '%');
	    if (s) {
		num = (short)(s - fstr);
		strncpy(pb, fstr, (short)num);
		pb += num;
		fstr = s;
	    } else {
		Strcpy(pb, fstr);
		fstr += strlen(fstr)-1;
	    }
	    switch (*fstr) {
		case '%':
		    fstr++;
		    convchar = FALSE;
		    islong = FALSE;
		    do {
			switch (*fstr) {
			    case 'l':	/* long */
				islong = TRUE;
				fstr++;
				break;
			    case 'u':	/* unsigned decimal */
			    case 'd':	/* signed decimal */
				num = (islong) ? *(unsigned long *)ps
					       : *(unsigned short *)ps;
				numsz = (islong) ? sizeof(long) : sizeof(short);
				ps += numsz;
				s = (islong) ? fstr - 2 : fstr - 1;
				c = *(fstr + 1);
				*(fstr + 1) = '\0';
				if (islong)
				    sprintf(numAsStr, s, num);
				else
				    sprintf(numAsStr, s, (short)num);
				*(fstr + 1) = c;
				Strcpy(pb, numAsStr);
				pb = (char *)(pb + strlen(numAsStr));
				convchar = TRUE;
				break;
			    case 's':
				s = *(char **)ps;
				Strcpy(pb, s);
				pb = (char *)(pb + strlen(s));
				ps += sizeof(char *);
				convchar = TRUE;
				break;
			    case 'c':
				c = *(unsigned short *)ps;
				numsz = sizeof(short);
				(*pb++) = (char)c;
				(*pb) = '\0';
				convchar = TRUE;
				ps += numsz;
				break;
			    default:
				convchar = TRUE;
			}
		    } while (!convchar);
		    break;
		default:
		    break;
	    }
	    fstr++;
	}
	if (prBuffer[0])
	    mput(&prBuffer[0]);
		
	return 1;
}

short
aboutBox(prompt)
short	prompt;
{
#define	OK_BUTTON	1
#define	MORE_INFO_BUTTON	2

	DialogPtr	theDialog;
	DialogRecord	space;
	Rect	rect;
	Handle	theControl;
	short	type,itemHit;
	GrafPtr	oldPort;
	EventRecord	theEvent;
	term_info	*t;
	DialogTHndl	th = (DialogTHndl) GetResource('DLOG', 129);
	short	dv, dh;

	/* if about box on startup, centre about box on screen */
	if (!prompt) {
		rect = SCREEN_BITS.bounds;
		dv = ((**th).boundsRect.bottom - 40 - (**th).boundsRect.top)/2;
		dh = ((**th).boundsRect.right - (**th).boundsRect.left)/2;

		(**th).boundsRect.bottom =
				(rect.bottom + rect.top + MBarHeight)/2 + dv;
		(**th).boundsRect.top	 =
				(rect.bottom + rect.top + MBarHeight)/2 - dv;
		(**th).boundsRect.right	 = (rect.right + rect.left)/2 + dh;
		(**th).boundsRect.left	 = (rect.right + rect.left)/2 - dh;
	}

	GetPort(&oldPort);
	theDialog = GetNewDialog(129, &space,(WindowPtr)-1);
	if (!prompt) {
		HideDItem(theDialog, OK_BUTTON);
		HideDItem(theDialog, MORE_INFO_BUTTON);
	} else
		MoveWindow((WindowPtr)theDialog, LEFT_OFFSET, TOP_OFFSET, TRUE);

	ShowWindow((WindowPtr)theDialog);
	SetPort(theDialog);
	TextFont(1);	/* 9 pt. Geneva */
	TextSize(9);
	DrawDialog(theDialog);
	itemHit = 0;
	if (prompt) {
		/* BOLD the OK button */
		GetDItem(theDialog, OK_BUTTON, &type, &theControl, &rect);
		PenSize(3,3);	
		InsetRect(&rect,-4,-4);
		FrameRoundRect(&rect,16,16);
		PenSize(1,1);
		while ((itemHit != OK_BUTTON) && (itemHit != MORE_INFO_BUTTON)) {
			ModalDialog(NULL, &itemHit);
		}
	} else {
		while (!itemHit) {
			if (GetNextEvent(everyEvent,&theEvent))
				if (theEvent.what == mouseDown)
					itemHit = OK_BUTTON;
		}
	}
	CloseDialog(theDialog);
	ReleaseResource((Handle)th);
	SetPort(oldPort);
	return (itemHit == MORE_INFO_BUTTON);
}
