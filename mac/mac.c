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
extern long lowMemLimit;
short cursorPos=0;
short repDelay;
long lastMD;
long segments = SEG_DO;
extern short altCurs; /* should be a macflag */

int
tgetch()

{
	char	ch;
	EventRecord	theEvent;
	Rect cursorRect,box,windowRect;
	long	message,cursorTime,start;
	MenuHandle	theMenu;
	register short	keyCode;
	short	temp;
	term_info	*t;
	GrafPtr	oldPort,oldPort1;
	static char nextCommand;
	short aboutBox();
	char mButtonDown();
	Point	mouseLoc;
	WindowPtr	theWindow;
	void	doUpdate();
#define	clearKey	0x47
#define ESCAPEkey	0x1B
	static int see, saw = 0;
	int eye, cursOff = 100;
	boolean cursRectInv;	/* should be a macflag */
	short	fInFront = TRUE;

	t = (term_info *)GetWRefCon(HackWindow);
	mouseLoc.h = (macflags & fMoveWRTMouse) ? t->tcur_x : (u.ux-1);
	mouseLoc.v = (macflags & fMoveWRTMouse) ? t->tcur_y : (u.uy+1);
	cursorRect.left = t->tcur_x * t->charWidth + Screen_Border;
	cursorRect.right = cursorRect.left + t->charWidth - 1;
	cursorRect.top = t->height * t->tcur_y + Screen_Border;
	cursorRect.bottom = cursorRect.top + t->height - 1;
	cursorTime = GetCaretTime();
	box.left = mouseLoc.h * t->charWidth + Screen_Border - 1;
	box.right = box.left + t->charWidth + 2;
	box.top = mouseLoc.v * t->height + Screen_Border - 1;
	box.bottom = box.top + t->height + 2;
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
	if (!(macflags & fDoNonKeyEvt)) {
		cursorPos = -1;
		if (! flags.wantspace) SetCursor(&ARROW_CURSOR);
	}
	/* initialize cursor blinking */
	message = TickCount();
	cursRectInv = FALSE;
	while (!ch) {
		(void)WaitNextEvent(everyEvent, &theEvent, (fInFront) ? 0L : 50L, 0L);
		if (theEvent.what == keyDown || theEvent.what == autoKey) {
			ch = 0;
			ObscureCursor();
			/* use raw key codes */
			keyCode = (LoWord(theEvent.message) & keyCodeMask)>>8;
 			if (keyCode == clearKey) {
				macflags = macflags ^ fToggleNumPad;
				SetWTitle(HackWindow,
					(macflags & fToggleNumPad)	? "\016NetHack [MOVE]"
												: "\015NetHack [NUM]");
				ch = 0;
				continue;
			}
			if ((macflags & fToggleNumPad) && (keyCode>0x40 &&keyCode < 0x5D) 
				|| (keyCode > 0x7A && keyCode<0x7F)) {
				ch = t->keyMap[keyCode-65];
				if ((theEvent.modifiers & shiftKey) && (ch)) {
					ch = (ch == '.') ? ':' : (char)toupper(ch);
				}
				if (ch)
					break;
			}
			if (t->system.keyBoardType < 4 && keyCode == 50) {
				ch = (char)ESCAPEkey;	/* ESC */
				break;
			}			/* make the command key = control key on all non-adb keyboards
						 Now, LEAVE it this way ! Otherwise you'll render non-us
keyboards useless, since here is where "<" and ">" are located */
			if ((theEvent.message & 0xFF) == 0x60) {
				ch = (char)ESCAPEkey;
				break;
			} /* This is the way to fix backquote->esc for all macs */
			if (theEvent.modifiers & cmdKey) {
				ch = (char)(theEvent.message & 0x1F);
				if(ch == ('Q' & 0x1F)) ch = 'Q'; /* For Quit */
				break;
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
			if (!ch) {
				ch = (char)(theEvent.message & 0xFF);
			}
		} else {
		/* what other events to handle */
			switch (theEvent.what) {		
			case nullEvent:
				if (!fInFront)
					break;
				GetPort(&oldPort1);
				SetPort((GrafPtr)HackWindow);
				/* wait until something occurs */
				if ((TickCount() > message) && (FrontWindow() == HackWindow)) {
					message = TickCount() + cursorTime;
					InvertRect(&cursorRect);
					cursRectInv = !cursRectInv;
				}
				if (FrontWindow() == HackWindow && (macflags & fDoNonKeyEvt)) {
					if ((FindWindow(theEvent.where,&theWindow) == inContent)
						&& (macflags & fDoUpdate) && (HackWindow == theWindow)) {
						
						GetMouse(&mouseLoc);
						if (PtInRect(mouseLoc,&box)) {
							temp = 8;
						} else {
							PtToAngle(&box,mouseLoc,&temp);
							temp = ((temp + 22) / 45) % 8;	/* 0=N, 1=NE ... 7=NW */
						}
						/* since we're not doing anything let's look at the cursor */
						if (altCurs) {	/* this list is prioritized! */
							if (Sick) {
								cursOff = 130;	/* offset for CURS resource set */
								see = SICK;
							} else if (Blind) {
								cursOff = 120;
								see = BLINDED;
							} else if (Hallucination) {
								cursOff = 110;
								see = HALLUC;
							} else if (Confusion || Stunned) {
								see = CONFUSION;
							} else {
								cursOff = 100;
								see = 0;
							}
						} else {
							see = (Confusion || Stunned) ? CONFUSION : 0;
							cursOff = 100;
						}
						if ((cursorPos != temp) || (see != saw)) {
							if (see != saw) {
								switch(see) {
								case CONFUSION:
									break;
								default:	/* hmm, a bit much for !altCurs */
									for (eye=0; eye<=8; eye++) {
										EmptyHandle(t->cursor[eye]);
										t->cursor[eye] = GetCursor(eye+cursOff);
										HNoPurge(t->cursor[eye]);
									}
									break;
								}
								saw = see;
							}
							cursorPos = temp;
							eye = (Confusion || Stunned) ? rn2(9) : temp;
							SetCursor(*t->cursor[eye]);
						}
					} else if (cursorPos>=0) {
						cursorPos = -1;
						SetCursor(&ARROW_CURSOR);
					}
				}
				if (StillDown() && cursorPos>=0 && cursorPos < 8
					&& TickCount() > lastMD+repDelay) {
					ch = mButtonDown(theEvent, t, &nextCommand);
					if (repDelay) {
#ifdef THINK_C
						repDelay = KeyRepThresh;
#else
						repDelay /= 3;
#endif
					}
					lastMD = TickCount();
					/*return ch;*/
				}			
				SetPort(oldPort1);
				break;
			case app4Evt:
#define	kSuspendResumeMessage	1		/* high byte of suspend/resume event message */
#define	kMouseMovedMessage		0xFA	/* high byte of mouse-moved event message */
#define	SuspResIsResume(evtMessage)		((evtMessage) & 0x00000001)

				switch (theEvent.message >> 24) {
					case kSuspendResumeMessage:
						if (!SuspResIsResume(theEvent.message)) {
						/* ---------- SUSPEND EVENT ------------ */
							fInFront = FALSE;
							if (macflags & fZoomOnContextSwitch
								&& !EmptyRect(&(**(HackWindow)->visRgn).rgnBBox))
							{
								InvalRect(&HackWindow->portRect);
								SizeWindow(HackWindow,60,60,FALSE);
							}
						} else {
						/* ---------- RESUME EVENT ------------- */
							fInFront = TRUE;
							if (macflags & fZoomOnContextSwitch) {
								SizeWindow(HackWindow,
								 (t->maxCol * t->charWidth) + 2 * Screen_Border,
								 (t->maxRow * t->height) + 2 * Screen_Border,
								 TRUE);
								SetPort(HackWindow);
								InvalRect(&HackWindow->portRect);
								if ((int) (theMenu = GetMHandle(editMenu))
									&& FrontWindow() == HackWindow) {
									SetMenuBar(t->fullMBarHandle);
									for (temp = fileMenu;temp <= extendMenu;temp++) {
									if (temp != editMenu)
										EnableItem(GetMHandle(temp), 0);
									}
									EnableItem(GetMHandle(appleMenu), 1);
									DisableItem(theMenu, 0);
									DrawMenuBar();
								}
							}
						}
						break;
				}
				break;
		
			case updateEvt:
				if (HackWindow == (WindowPtr)theEvent.message) {
					doUpdate(t);
					if (cursRectInv) InvertRect(&cursorRect);	/* restore cursor */
				}
				break;
				
			case activateEvt:
				if (HackWindow == (WindowPtr)theEvent.message) {
					if (theMenu = GetMHandle(editMenu)) {
						if (theEvent.modifiers & activeFlag) {
							if (macflags & fDoUpdate) {
								SetMenuBar(t->fullMBarHandle);
								for (temp = fileMenu;temp <= extendMenu;temp++) {
									if (temp != editMenu)
										EnableItem(GetMHandle(temp), 0);
								}
								EnableItem(GetMHandle(appleMenu), 1);
							}
							DisableItem(theMenu, 0);
						} else {
							EnableItem(theMenu, 0);
							if (macflags & fDoUpdate) {
								SetMenuBar(t->shortMBarHandle);
								for (temp = fileMenu;temp <= extendMenu;temp++) {
									if (temp != editMenu)
										DisableItem(GetMHandle(temp), 0);
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
#ifdef THINK_C
				repDelay = KeyThresh;
#else
				repDelay = 42;
#endif
				lastMD = theEvent.when;
				break;
			}
		}		
	}
	if (cursRectInv) InvertRect(&cursorRect);

	SetPort(oldPort);
	return ((ch == '\r') ? '\n' : ch);
}

void
doUpdate(t)
term_info	*t;
{
	register short	temp;
	GrafPtr	oldPort;
	register	short i,j;
	register char	attr, *p;
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
    ForeColor(t->color[0]);
    BackColor(t->color[7]);
	for (i = 0; i<t->maxRow; i++) {
		p = t->scrAttr[i];
		for (j = temp = 0, attr = *p; j<t->maxCol; j +=1, p++) {
			if (*p != attr) {
				MoveTo(Screen_Border + temp*t->charWidth,
					t->ascent + (i * t->height) +
					Screen_Border);
				if (attr & 0x80) {
				    ForeColor(t->color[7]);
				    BackColor(t->color[0]);
				} else if (t->inColor) {
					ForeColor(t->color[attr]);
					 /* yellow fgnd hard to see on white bgnd */
					 /* so change to green background */
					switch(attr) {
					case BROWN:
						BackColor(t->color[GREEN]);
						break;
					case BLUE:
						BackColor(t->color[CYAN]);
						break;
					case CYAN:
						BackColor(t->color[BLUE]);
						break;
					default:
					    BackColor(t->color[7]);
					   	break;
					}
				} else {
				    ForeColor(t->color[0]);
				    BackColor(t->color[7]);
				}
				DrawText(&(t->screen[i][temp]), 0, j - temp);
				temp = j;
				attr = *p;
			} /* if */
		} /* for j */
		if (temp != t->maxCol) {
			MoveTo(Screen_Border + temp*t->charWidth,
				t->ascent + (i * t->height) +
				Screen_Border);
			if (attr & 0x80) {
			    ForeColor(t->color[7]);
			    BackColor(t->color[0]);
			} else if (t->inColor) {
				ForeColor(t->color[attr]);
				 /* yellow fgnd hard to see on white bgnd */
				 /* so change to green background */
				switch(attr) {
				case BROWN:
					BackColor(t->color[GREEN]);
					break;
				case BLUE:
					BackColor(t->color[CYAN]);
					break;
				case CYAN:
					BackColor(t->color[BLUE]);
					break;
				default:
				    BackColor(t->color[7]);
				   	break;
				}
			} else {
			    ForeColor(t->color[0]);
			    BackColor(t->color[7]);
			}
			DrawText(&(t->screen[i][temp]), 0, j - temp);
		} /* if */
	} /* for i */
	EndUpdate(HackWindow);
	SetPort(oldPort);
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
	if (macflags & fDoNonKeyEvt) {
		switch (FindWindow(theEvent.where,&theWindow)) {
	    case inMenuBar:
		if (!(macflags & fDoMenus))
			break;
		SetCursor(&ARROW_CURSOR);
		message = MenuSelect(theEvent.where);

		if (!HiWord(message))
		    break;

		switch (HiWord(message)) {
		    case editMenu:
			(void)SystemEdit((short)message - 1);
			break;
		    case appleMenu:
			if (LoWord(message) > 1) {
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

		{
		RgnHandle fooRgn = GetGrayRgn();
			boundsRect = (*fooRgn)->rgnBBox;
		}
		SetCursor(&ARROW_CURSOR);
		DragWindow(theWindow, theEvent.where, &boundsRect);
		break;
	
	case inContent:
		if (theWindow != FrontWindow()) {
			SelectWindow(theWindow);
		} else if (theWindow == HackWindow) {
			Point	mouseLoc;
			Rect	box;
			short	temp;

			if(flags.wantspace) {
				ch = 0x20;
			} else {
				box.left = (u.ux-1) * t->charWidth + Screen_Border + (t->charWidth/2);
				box.right = box.left + 1;
				box.top = (u.uy+1) * t->height + Screen_Border + t->height/2;
				box.bottom = box.top + 1;
				GetMouse(&mouseLoc);
				PtToAngle(&box,mouseLoc,&temp);
				temp = ((temp + 22) / 45) % 8;	/* ~same as above */
				switch(cursorPos) {
					case 0:
						ch = 'k';
						break;
					case 1:
						ch = 'u';
						break;
					case 2:
						ch = 'l';
						break;
					case 3:
						ch = 'n';
						break;
					case 4:
						ch = 'j';
						break;
					case 5:
						ch = 'b';
						break;
					case 6:
						ch = 'h';
						break;
					case 7:
						ch ='y';
						break;
					case 8:
						ch = '.';
						break;
				}
				if ((theEvent.modifiers & shiftKey) && (ch)) {
					ch = (ch == '.') ? ':' : (char)toupper(ch);
				}
			}
		}			
		break;
		}
	} else {
		switch (FindWindow(theEvent.where,&theWindow)) {
		case inContent:
			if(flags.wantspace) ch = 0x20;
			break;
		default:
			break;
		}
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
	
	SystemTask();
	return (EventAvail(keyDownMask | mDownMask, &theEvent));
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

static void
checkScroll(t)
term_info *t;
{
	if (t->tcur_y >= t->maxRow-1) {
		short	temp;
		char	*s, *s1;

		BlockMove((Ptr)t->screen[1], (Ptr)t->screen[0],
			(Size)((t->maxRow - 1) * t->maxCol));
		BlockMove((Ptr)t->scrAttr[1], (Ptr)t->scrAttr[0],
			(Size)((t->maxRow - 1) * t->maxCol));
		for (temp = 0, s = t->screen[t->maxRow - 1], s1 = t->scrAttr[t->maxRow - 1];
				temp < t->maxCol; temp++, s++, s1++) {
			*s = ' ';
			*s1 = 0;
		}
		{
			Pattern p, o;
			Rect	window;
			
			if (macflags & fInvertedScreen) {
				BlockMove((Ptr)&((GrafPtr)HackWindow)->bkPat, (Ptr)&o,
						sizeof(Pattern));
				GetIndPattern(&p, sysPatListID,1);
				BackPat(p);
			}
			window = HackWindow->portRect;
			InsetRect(&window, 4,4);
			window.top += t->height;
			ScrollRect(&window, 0, -t->height,
					((WindowPeek)HackWindow)->updateRgn);
			ValidRect(&window);
			if (macflags & fInvertedScreen) {
				BackPat(o);
			}
		}
		t->tcur_y = t->maxRow - 1;
	}
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
	/* a termcap-type escape string */
	if (!strncmp(s, "\033[", 2)) {
	    switch(*(s + 2)) {
		case 'c':	/* color kluge */
			temp = (short)(*(s + 3) - 'a');
			t->curAttr = temp;
			if (temp >= BLACK && temp < MAXCOLORS &&
			    (temp % (MAXCOLORS / 2) != GRAY)) /* set colour */
			    t->curAttr = temp % (MAXCOLORS / 2);
			if ((temp & 0x07) == GRAY)
				t->curAttr = 0;
		    if (t->inColor) {
			if (temp >= BLACK && temp < MAXCOLORS &&
			    (temp % (MAXCOLORS / 2) != GRAY)) /* set colour */
				ForeColor(t->color[temp % (MAXCOLORS / 2)]);
			if (temp == GRAY)
		                ForeColor(t->color[0]);
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
			    t->curHilite = FALSE;
			    t->curAttr = 0;
		    }
		    break;
		case '1':	/* inverse video begin */
		    if (*(s + 3) == 'm') {
			    ForeColor(t->color[7]);
			    BackColor(t->color[0]);
			    t->curHilite = TRUE;
			    t->curAttr = 0;
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
		    for (temp = x, c = &(t->screen[y][x]), c1 = &(t->scrAttr[y][x]);
		    	temp < t->maxCol; temp++) {
			    *c++ = ' ';
			    *c1++ = 0;
			}
		    break;
		case '2':
		    if (*(s+3) == 'J') {	/* clear screen */
			x = y = t->tcur_x = t->tcur_y = 0;
			eraseRect.top = eraseRect.left = Screen_Border;
			eraseRect.bottom = t->maxRow*t->height + Screen_Border;
			eraseRect.right = t->charWidth*t->maxCol + Screen_Border;
			EraseRect(&eraseRect);
			for (y = 0, c = t->screen[0], c1 = t->scrAttr[0];
				y < t->maxCol * t->maxRow; y++) {
					*c++ = ' ';
					*c1++ = 0;
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
		    temp = (x + charleft - 1 < t->maxCol - 1) ? charleft : t->maxCol - x;
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
		    memset(&(t->scrAttr[y][x]), ((t->curHilite) ? 128 : 0) + t->curAttr,
		    	temp);
		    stmp += temp + 1;
		    charleft -= temp + 1;
		    if (!savech) {
			    t->tcur_x += temp;
		    }

		    if (t->tcur_x >= t->maxCol-1 || savech) {
			    if (savech != '\r') {
				    if (t->tcur_y >= t->maxRow-1) {
					    checkScroll(t);
				    } else {
					    y = (++t->tcur_y);
				    }
			    }
	
			    x = t->tcur_x = 0;
			    cur.h = Screen_Border;
			    cur.v = y * t->height + t->ascent + Screen_Border;
			    MoveTo(cur.h,cur.v);
			}
	    }
	}
	}
	if (t->tcur_x >= t->maxCol-1) {
	    t->tcur_x = t->tcur_x % t->maxCol;
	    t->tcur_y++;
	    checkScroll(t);
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
	} else if ((c == '\n') || (c == '\r')) {
		t->tcur_x = 0;
		if (t->tcur_y >= t->maxRow && c == '\r') {
			t->tcur_y = t->maxRow - 1;
		} else if (c == '\n') {
			if (t->tcur_y >= t->maxRow-1) {
				checkScroll(t);
			} else {
				t->tcur_y++;
			}
		}
	} else {
		t->screen[y][x] = c;
		t->scrAttr[y][x] = ((t->curHilite) ? 128 : 0) + t->curAttr;
		DrawText(&c, 0, 1);
		if (!savech) {
			t->tcur_x++;
			if (t->tcur_x >= t->maxCol)
			{
				t->tcur_x = 0;
				t->tcur_y++;
				checkScroll(t);
			}
		}
	}
	cur.h = (t->tcur_x * t->charWidth) + Screen_Border;
	cur.v = t->ascent + (t->tcur_y * t->height) + Screen_Border;
	MoveTo(cur.h,cur.v);

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
				    Sprintf(numAsStr, s, num);
				else
				    Sprintf(numAsStr, s, (short)num);
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

DialogTHndl
centreDlgBox(resNum, clip)
short	resNum;
Boolean	clip;
{
	DialogTHndl	th = (DialogTHndl) GetResource('DLOG', resNum);
	Rect	rect;
	short	dv, dh;

	/* centre dialog box on screen */
	if (th) {
		rect = SCREEN_BITS.bounds;
		HLock((Handle)th);
		dv = ((**th).boundsRect.bottom - (**th).boundsRect.top)/2;
		dv -= (clip) ? 20 : 0;
		dh = ((**th).boundsRect.right - (**th).boundsRect.left)/2;
	
		(**th).boundsRect.bottom =
				(rect.bottom + rect.top + MBarHeight)/2 + dv;
		(**th).boundsRect.top	 =
				(rect.bottom + rect.top + MBarHeight)/2 - dv;
		(**th).boundsRect.right	 = (rect.right + rect.left)/2 + dh;
		(**th).boundsRect.left	 = (rect.right + rect.left)/2 - dh;
		HUnlock((Handle)th);
	} else
		panic("Couldn't load dialog resource");	

	return th;
}

short
aboutBox(prompt)
short	prompt;
{
#define	OK_BUTTON	1
#define	MORE_INFO_BUTTON	2

	DialogPtr	theDialog;
	Rect	rect;
	Handle	theControl;
	short	type,itemHit;
	GrafPtr	oldPort;
	EventRecord	theEvent;
	term_info	*t;
	DialogTHndl	th;

	/* if about box on startup, centre about box on screen */
	if (!prompt) {
		th = centreDlgBox(129, TRUE);
	}

	GetPort(&oldPort);
	theDialog = GetNewDialog(129, (Ptr)NULL,(WindowPtr)-1);
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
		long	tickNum;

		tickNum = TickCount() + 180;	/* 3 second wait max */
		while (!itemHit && TickCount()<tickNum) {
		/*	SystemTask();*/
			if (GetNextEvent(everyEvent,&theEvent))
				if (theEvent.what == mouseDown ||
  			            theEvent.what == keyDown ||
  				    theEvent.what == autoKey)
					itemHit = OK_BUTTON;
		}
	}
	DisposDialog(theDialog);
	ReleaseResource((Handle)th);
	SetPort(oldPort);
	return (itemHit == MORE_INFO_BUTTON);
}

void
freeSegs(segs)
long	*segs;
{
	register long s;
	
	s = (segs == (long *)0) ? 0L : (*segs);
	
	UnloadSeg(dothrow);
	UnloadSeg(dowear);
	UnloadSeg(ddocall);
	UnloadSeg(castmu);
	UnloadSeg(doforce);
	if (!(s & SEG_EAT))
		UnloadSeg(doeat);
	if (!(s & SEG_ZAP))
		UnloadSeg(dozap);
	UnloadSeg(initterm);
	if (!(s & SEG_APPLY))
		UnloadSeg(doapply);
	UnloadSeg(dokick);
	UnloadSeg(outrumor);
	if (!(s & SEG_DO))
		UnloadSeg(steal);
	UnloadSeg(done1);
	if (!(s & SEG_POTION))
		UnloadSeg(dodrink);
	UnloadSeg(doturn);
#ifdef REINCARNATION
	UnloadSeg(makeroguerooms);
#endif
#ifdef STRONGHOLD
	UnloadSeg(load_special);
#endif
	UnloadSeg(mklev);
#ifdef MUSIC
	if (!(s & SEG_SPELL))
		UnloadSeg(do_play_instrument);
#endif
#ifdef SPELLS
	if (!(s & SEG_SPELL))
		UnloadSeg(docast);
#endif
	UnloadSeg(savebones);
	UnloadSeg(expels);
	UnloadSeg(attack);
	UnloadSeg(doname);
	if (FreeMem() < lowMemLimit)
	{
		long	space;
		
		space = 0x7FFFFFFFL;
		MaxMem(&space);
	}

}

