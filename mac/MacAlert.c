/*	SCCS Id: @(#)MacAlert.c		3.0	90/01/06
/*      Copyright (c) Jon Watte  1989		*/ 
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"	/* */

/* UseMacAlert is like UseMacAlertText, but without the text parameter.
   This is so you can simply say "UseMacAlert(CAUTION)" or similar, if
   you use predefined alert numbers.
*/


extern WindowPtr HackWindow;


int
UseMacAlert(MAno)
int MAno;
{
	return UseMacAlertText(MAno, 0L);
}


/* UseMacAlertText fetches a record in a resource of type MAlt, and uses
   this as a template calling MacAlert. This is so you can have ready-
   made resources, like ALRT templates, for your MacAlerts, which makes
   it easier to customize the program, and to translate it. It also
   makes for clearer code...
   
   If the txt argument is NULL, the text from the template is used,
   otherwise the sypplied text is used.
*/
int
UseMacAlertText(MAno, txt)
int MAno;
char * txt;
{
	MAlrtHandle foo;
	MAlrtPtr bar;
	int item;

	SetResLoad(1);
	foo = (MAlrtHandle) GetResource(MAtype, MAno);
	if(foo) {
		MoveHHi(foo);
		HLock(foo);
		bar = *foo;
		item = MacAlert(bar->width, bar->height, bar->PICTno,
			txt != 0L ? txt : bar->text, bar->but1, bar->but2, bar->but3,
			bar->but4, bar->def, bar->esc);
	} else {
		if(!flags.silent) SysBeep(20);
		item = -1;
	}

	ReleaseResource(foo);
	DisposHandle(foo);
	return item;
}


/* This is the MacAlert function. It creates a new window, frames it
   (like a modal dialog) and tries to position the various buttons,
   text and picture in a reasonable way. The arguments are:
   
   width : width of the window. If less than a constant, it defaults
           to that constant.
   height: see width.
   PICTno: Resource ID of a PICT to display. If 0, no PICT is displayed.
   text  : the C string containing the relevant text to be displayed.
   but1 -: These are the button texts. Empty strings (they point at 0)
   - but4: makes that button go away. If all are empty, the default OK
           button text is used.
   def   : This is which button is default, starting button # 1.
   esc   : This is the button that corresponds to the ESC key.

   The function returns the button hit, or maybe -1 on error.
*/
int
MacAlert(width, height, PICTno, text, but1, but2, but3, but4, def, esc)
int width, height, PICTno;
char * text,* but1, * but2, * but3, * but4;
int def, esc;
{
	PicHandle thePICT;
	char border[8];
	EventRecord theEvent;
	GrafPtr savedPort;
	WindowPtr theWindow, tmpWind;
	Rect bounds, textBox, brect[4];
	char hilite[4], * but[4], s1[2], s2[2];
	int nobutts, x, pool, spacing, heig, bwid[4],
		item, c, ret;
	long finalTicks;

	memset(hilite, sizeof(hilite), 0);
	memset(bwid, sizeof(bwid), 0);
	SetCursor(&arrow);
	def--;
	esc--;
	GetPort(&savedPort);

	if(PICTno)
		thePICT = (PicHandle) GetResource('PICT', PICTno);
	else
		thePICT = 0;
	if(thePICT) DetachResource(thePICT);
	bounds = (MAINGRAFPORT)->portRect;

	but[0] = but1;
	but[1] = but2;
	but[2] = but3;
	but[3] = but4;

	if(width > bounds.right) width = bounds.right;
	if(width < MIN_WIDTH) width = MIN_WIDTH;
	if(height > bounds.bottom) height = bounds.bottom;
	if(height < MIN_HEIGHT) height = MIN_HEIGHT;

	InsetRect(&bounds, (int) ((bounds.right - width) >> 1), (int)
		((bounds.bottom - height) >> 1));
	theWindow = NewWindow(0L, &bounds, "", 1, plainDBox, 0L, 0, 0L);
	ShowWindow(theWindow);
	SelectWindow(theWindow);
	SetPort(theWindow);
	OffsetRect(&bounds, - bounds.left, - bounds.top);

	nobutts = 0;
	if(but1[0]) nobutts=1;
	if(but2[0]) nobutts=2;
	if(but3[0]) nobutts=3;
	if(but4[0]) nobutts=4;

	if(!nobutts) {
		but[0] = "OK";
		nobutts++;
	}

	spacing = 0;
	for(x=0 ; x < nobutts; x++) {
		bwid[x] = TextWidth(but[x], 0, strlen(but[x])) + 2 * BUT_MARGIN;
		spacing += bwid[x] + BUT_SPACING;
	}

	pool = bounds.right - 2 * BUT_MARGIN;

	heig = 1;
	while(spacing / heig > pool - (nobutts - 1) * BUT_SPACING) {
		heig++;
	}

	for(x=0; x<nobutts; x++) {
		bwid[x] = (int) ((float) bwid[x] / spacing * pool);
	}

	pool = BUT_MARGIN + BUT_SPACING;
	heig = bounds.bottom - BUT_MARGIN - BUT_SPACING - BUT_HEIGHT * heig;
	for(x=0; x<nobutts; x++) {
		SetRect(&(brect[x]), pool, heig, pool + bwid[x], bounds.bottom -
			BUT_SPACING - BUT_MARGIN);
		pool += bwid[x] + BUT_SPACING;
		hilite[x] = 0;
	}

	InsetRect(&bounds, 2, 2);
	textBox = bounds;
	if(thePICT) {
		textBox.left += (*thePICT)->picFrame.right;
	}
	textBox.left += 8;
	textBox.right -= 8;
	textBox.top += 8;
	textBox.bottom = heig - BUT_MARGIN;
	if(textBox.bottom < textBox.top + 15) {
		textBox.bottom = bounds.bottom - 8;
	}

	goto mainLoop;

drawWindow:
	SetPort(theWindow);
	EraseRect(&(theWindow->portRect));
	PenNormal();
	PenSize(BORDER_WIDTH, BORDER_WIDTH);
	GetIndPattern((void *) border, 0, BORDER_PAT);
	PenPat(border);
	FrameRect(&bounds);

	PenNormal();

	if(thePICT) {
		HLock(thePICT);
		DrawPicture(thePICT, &((*thePICT)->picFrame));
		HUnlock(thePICT);
	}

	PenNormal();
	TextFont(0);
	TextSize(12);
	TextBox(text, strlen(text), &textBox, teJustLeft);

	for(x=0; x<nobutts; x++) {
		EraseRoundRect(&(brect[x]), BUT_CORNER, BUT_CORNER);
		FrameRoundRect(&(brect[x]), BUT_CORNER, BUT_CORNER);
		InsetRect(&(brect[x]), BUT_MARGIN, 3);
		TextBox(but[x], strlen(but[x]), &(brect[x]), teJustCenter);
		InsetRect(&(brect[x]), - BUT_MARGIN, - 3);
		if(hilite[x]) {
			InvertRoundRect(&(brect[x]), BUT_CORNER, BUT_CORNER);
		}
		if(x == def) {
			PenSize(FRAME_WIDTH, FRAME_WIDTH);
			InsetRect(&(brect[x]), - FRAME_WIDTH - FRAME_OFFSET,
				- FRAME_WIDTH - FRAME_OFFSET);
			FrameRoundRect(&(brect[x]), FRAME_CORNER, FRAME_CORNER);
			InsetRect(&(brect[x]), FRAME_WIDTH + FRAME_OFFSET,
				FRAME_WIDTH + FRAME_OFFSET);
			PenNormal();
		}
	}

mainLoop:
	while(1) {
		if(WaitNextEvent(-1, &theEvent, 10L, 0L)) {
			switch(theEvent.what) {

			case updateEvt :
				BeginUpdate((WindowPtr) theEvent.message);
				EndUpdate((WindowPtr) theEvent.message);
				goto drawWindow;

			case mouseDown :
				if (FindWindow(theEvent.where, &tmpWind) < inContent ||
					tmpWind != theWindow) {
					if(!flags.silent) SysBeep(20);
					item = -1;
				} else {
					item = TrackThem(brect, hilite, nobutts);
				}
				if(item >= 0) {
					Delay(FLASH_TIME, &finalTicks);
					InvertRoundRect(&(brect[item]), BUT_CORNER, BUT_CORNER);
					ret = item + 1;
					goto getout;
				}
				break;

			case keyDown :
				c = theEvent.message & 0xFF;
				if((c == 13) || (c == 3)) {
					if(def >= 0) {
						InvertRoundRect(&(brect[def]), BUT_CORNER, BUT_CORNER);
						Delay(FLASH_TIME, &finalTicks);
						InvertRoundRect(&(brect[def]), BUT_CORNER, BUT_CORNER);
						ret = def + 1;
						goto getout;
					}
				}
				if(c == 27) {
					if(esc >= 0) {
						InvertRoundRect(&(brect[esc]), BUT_CORNER, BUT_CORNER);
						Delay(FLASH_TIME, &finalTicks);
						InvertRoundRect(&(brect[esc]), BUT_CORNER, BUT_CORNER);
						ret = esc + 1;
						goto getout;
					}
				}
				for(x=0; x<nobutts; x++) {
					s1[0] = 1;
					s1[1] = c;
					s2[0] = 1;
					s2[1] = but[x][0];
					UprString(s1, 1);
					UprString(s2, 1);
					if(s1[1] == s2[1]) {
						InvertRoundRect(&(brect[x]), BUT_CORNER, BUT_CORNER);
						Delay(FLASH_TIME, &finalTicks);
						InvertRoundRect(&(brect[x]), BUT_CORNER, BUT_CORNER);
						ret = x + 1;
						goto getout;
					}
				}

				PenSize(FRAME_WIDTH, FRAME_WIDTH);
				InsetRect(&(brect[def]), - FRAME_WIDTH - FRAME_OFFSET,
					- FRAME_WIDTH - FRAME_OFFSET);
				PenMode(patXor);
				FrameRoundRect(&(brect[def]), FRAME_CORNER, FRAME_CORNER);
				InsetRect(&(brect[def]), FRAME_WIDTH + FRAME_OFFSET,
					FRAME_WIDTH + FRAME_OFFSET);
				PenNormal();

				def++;
				def %= nobutts;

				PenSize(FRAME_WIDTH, FRAME_WIDTH);
				InsetRect(&(brect[def]), - FRAME_WIDTH - FRAME_OFFSET,
					- FRAME_WIDTH - FRAME_OFFSET);
				FrameRoundRect(&(brect[def]), FRAME_CORNER, FRAME_CORNER);
				InsetRect(&(brect[def]), FRAME_WIDTH + FRAME_OFFSET,
					FRAME_WIDTH + FRAME_OFFSET);
				PenNormal();

				break;

			default:
				;
			}
		}
	}

getout:
	DisposeWindow(theWindow);
	if(thePICT) DisposHandle(thePICT);
	SetPort(savedPort);

	return ret;
}


/* TrackThem is a help function to MacAlert, it tracks buttons,
   returns the button number (0 - (no_butts-1)) or -1 if no button
   was hit. */
int
TrackThem(b_rect, hi_lite, no_butts)
Rect * b_rect;
char * hi_lite;
int no_butts;
{
Point p;
int x, i;

	while(Button()) {
		SystemTask();
		GetMouse(&p);
		for(x=0; x<no_butts; x++) {
			if(PtInRect(p, &(b_rect[x]))) {
				if(!hi_lite[x]) {
					hi_lite[x] = 1;
					InvertRoundRect(&(b_rect[x]), BUT_CORNER, BUT_CORNER);
				}
			} else {
				if(hi_lite[x]) {
					hi_lite[x] = 0;
					InvertRoundRect(&(b_rect[x]), BUT_CORNER, BUT_CORNER);
				}
			}
		}
	}

	i = -1;
	for(x=0; x<no_butts; x++) {
		if(hi_lite[x]) i=x;
	}

	return i;
}


/* #defines for the mini-pager */

#define SLACK 400 /* How much extra is taken per chunk ? */
#define TAB_SIZE 8 /* How large tabs ? */

#define X_POS 8 /* Window init pos */
#define Y_POS 64
#define X_SIZE 80 /* Window size */
#define Y_SIZE 24
#define X_BORDER 20 /* Extra space besides text */
#define Y_BORDER 4
#define TOP_MARGIN 2 /* Offset of text from border */
#define LEFT_MARGIN 2
#define MAX_LINE_LEN 128

#include <ControlMgr.h>

typedef struct select {
	long start;
	long end;
	long startline;
	long endline;
} SELECTION;

extern WindowPtr HackWindow;

static WindowPtr PagerWindow;
static width, height;
static Handle theData;
static long length, textPos;
static Rect text_area;
static Rect windowSize;
static int inited;
static ControlHandle theScrollBar;
static int scrollMax = 0, scrollPt = 0;
static long ** lineStarts;
static long noRs;
static MenuHandle editMenuH;
static SELECTION theSelect, oldSelect;

int DoMenu(long selection);
void DoScroll(long amount);


/* The mini-pager */


long
LineLength(long from, long lines)
{
long ndx;
	if(lines + from > noRs) ndx = noRs;
	else ndx = lines + from;
	return (*lineStarts)[ndx] - (*lineStarts)[from];
}


long
WhatOffset(Point p, long * line)
{
int x, y;
long l, ndx;

	x = (p.h - LEFT_MARGIN) / width;
	if(x < 0) x = 0;
	y = (p.v - TOP_MARGIN) / height;
	if(y < 0) {
		y = 0;
		DoScroll(-1);
	} else if(y >= Y_SIZE) {
		y = Y_SIZE;
		DoScroll(1);
	}
	l = y + scrollPt > noRs-1 ? noRs-1 : y + scrollPt;
	ndx = x + (*lineStarts)[l];
	if(l == noRs) ndx = length;
	else if(ndx > (*lineStarts)[l+1]) ndx = (*lineStarts)[l+1];

	* line = l;
	return ndx;
}


void
InvertRange(SELECTION * s, Rect * cr)
{
long sc = s->start;
long ec = s->end;
long sl = s->startline;
long el = s->endline;
int sx, ex;
Rect r;

#define HilitePtr (void *) 0x938

	if(sc > ec) {
	long t = sc;
		sc = ec;
		ec = t;
		t = sl;
		sl = el;
		el = t;
	}

	sx = sc - (*lineStarts)[sl];
	ex = ec - (*lineStarts)[el];
	sl -= scrollPt;
	el -= scrollPt;
	r = text_area;
	ClipRect(cr ? cr : &r);
	if(el == sl) {
		SetRect(&r, sx * width, sl * height, ex * width, (el + 1) * height);
		OffsetRect(&r, LEFT_MARGIN, TOP_MARGIN);
		BitClr(HilitePtr, pHiliteBit);
		InvertRect(&r);
	} else {
		SetRect(&r, sx * width, sl * height, X_SIZE * width, (sl + 1) * height);
		OffsetRect(&r, LEFT_MARGIN, TOP_MARGIN);
		BitClr(HilitePtr, pHiliteBit);
		InvertRect(&r);
		SetRect(&r, 0, el * height, ex * width, (el + 1) * height);
		OffsetRect(&r, LEFT_MARGIN, TOP_MARGIN);
		BitClr(HilitePtr, pHiliteBit);
		InvertRect(&r);
		SetRect(&r, 0, (sl + 1) * height, X_SIZE * width, el * height);
		OffsetRect(&r, LEFT_MARGIN, TOP_MARGIN);
		BitClr(HilitePtr, pHiliteBit);
		InvertRect(&r);
	}
	SetRect(&r, 0, 0, 20000, 20000);
	ClipRect(&r);
}


void
DoDraw(EventRecord * theEvent, term_info * t)
{
	BeginUpdate((WindowPtr) theEvent->message);
	if(StripAddress(theEvent->message) == StripAddress(HackWindow)) {
		SetPort(HackWindow);
		docrt();
	} else if(StripAddress(theEvent->message) == StripAddress(PagerWindow)) {
		MoveHHi(theData);
		HLock(theData);
		MoveHHi(lineStarts);
		DrawControls(PagerWindow);
		TextBox(&((*theData)[(*lineStarts)[textPos]]), LineLength(textPos,
			Y_SIZE), &text_area, teJustLeft);
		HUnlock(theData);
		InvertRange(&theSelect, 0L);
	}
	EndUpdate((WindowPtr) theEvent->message);
}


void
DoScroll(long amount)
{
int ab = amount > 0 ? amount : - amount;

	if(textPos == 0 && amount < 0) return;
	if(textPos == scrollMax && amount > 0) return;

	textPos += amount;
	if(textPos < 0) {
		amount -= textPos;
		textPos = 0;
	}
	if(textPos > scrollMax) {
		amount -= textPos - scrollMax;
		textPos = scrollMax;
	}
	scrollPt = textPos;

	if(ab > Y_SIZE - 1) {
		HLock(theData);
		TextBox(&((*theData)[(*lineStarts)[textPos]]), LineLength(textPos,
			ab), &text_area, teJustLeft);
		HUnlock(theData);
		InvertRange(&theSelect, (Rect *) 0L);
	} else {
	Rect r;
	RgnHandle rgn = NewRgn();
		r = text_area;
		ScrollRect(&text_area, 0, - amount * height, rgn);
		DisposHandle(rgn);
		if(amount < 0) {
			r.bottom -= (Y_SIZE - ab) * height;
			HLock(theData);
			TextBox(&((*theData)[(*lineStarts)[textPos]]), LineLength
				(textPos, ab), &r, teJustLeft);
			HUnlock(theData);
			InvertRange(&theSelect, &r);
		} else {
			r.top += (Y_SIZE - ab) * height;
			HLock(theData);
			TextBox(&((*theData)[(*lineStarts)[textPos + Y_SIZE - ab]]),
				LineLength(textPos + Y_SIZE - ab, ab), &r,
				teJustLeft);
			HUnlock(theData);
			InvertRange(&theSelect, &r);
		}
	}
	SetCtlValue(theScrollBar, scrollPt);
	DrawControls(PagerWindow);
}


pascal void
LineUp(ControlHandle stl, int part)
{
	DoScroll(-1);
}

pascal void
LineDown(ControlHandle stl, int part)
{
	DoScroll(1);
}

pascal void
PageUp(ControlHandle stl, int part)
{
	DoScroll(- Y_SIZE + 1);
}

pascal void
PageDown(ControlHandle stl, int part)
{
	DoScroll(Y_SIZE - 1);
}


int
DoKey(EventRecord * theEvent, term_info * t)
{
char c = theEvent->message & 0xFF;
char k = (theEvent->message & 0xFF00) >> 8;

	switch(k) {

	case 0x7A:
		DoMenu((long)(editMenu << 16) | 1);
		return 1;

	case 0x78:
		DoMenu((long)(editMenu << 16) | 3);
		return 1;

	case 0x63:
		DoMenu((long)(editMenu << 16) | 4);
		return 1;

	case 0x76:
		DoMenu((long)(editMenu << 16) | 5);
		return 1;

	case 0x7E:
	case 0x7B:
		if(theEvent->modifiers & (optionKey | shiftKey | cmdKey))
			DoScroll(- Y_SIZE + 1);
		else DoScroll(-1);
		return 1;

	case 0x7C:
	case 0x7D:
		if(theEvent->modifiers & (optionKey | shiftKey | cmdKey))
			DoScroll(Y_SIZE - 1);
		else DoScroll(1);
		return 1;

	case 0x73:
		DoScroll(-scrollMax);
		return 1;

	case 0x77:
		DoScroll(scrollMax);
		return 1;

	case 0x74:
		DoScroll(1 - Y_SIZE);
		return 1;
		
	case 0x79:
		DoScroll(Y_SIZE - 1);
		return 1;
		
	case 0x7F:
		return 0;

	default:
		break;
	}

	switch(c) {

	case 'c':
	case 'C':
		if(theEvent->modifiers & cmdKey) {
			HiliteMenu(editMenu);
			DoMenu((long)(editMenu << 16) | 4);
			HiliteMenu(0);
		}
		return 1;

	case '.':
		if(!(theEvent->modifiers & cmdKey)) break;
	case 'q':
	case 'Q':
	case 0x1B:
		return 0;

	case ' ':
	case 0x9:
		if(theEvent->modifiers & (optionKey | shiftKey | cmdKey))
			DoScroll(- Y_SIZE + 1);
		else DoScroll(Y_SIZE - 1);
		break;

	case 0x3:
	case '\r':
	case '\n':
		if(theEvent->modifiers & (optionKey | shiftKey | cmdKey))
			DoScroll(-1);
		else DoScroll(1);
		break;

	case '<':
		DoScroll(- scrollMax);
		break;

	case '>':
		DoScroll(scrollMax);
		break;

	default:
		break;
	}

	return 1;
}


void
DoClick(EventRecord * theEvent, term_info * t)
{
ControlHandle control;
Point pt = theEvent->where;
int part;

	GlobalToLocal(&pt);
	part = FindControl(pt, PagerWindow, &control);
	if(part) {
		switch(part) {

		case inThumb:
			if(TrackControl(control, pt, 0L) == inThumb) {
				scrollPt = GetCtlValue(control);
				DoScroll(scrollPt - textPos);
			}
			break;

		case inUpButton:
			TrackControl(control, pt, LineUp);
			break;

		case inDownButton:
			TrackControl(control, pt, LineDown);
			break;

		case inPageUp:
			TrackControl(control, pt, PageUp);
			break;

		case inPageDown:
			TrackControl(control, pt, PageDown);
			break;

		default:
			break;
		}
	} else {
		if(theEvent->modifiers & shiftKey) {
			theSelect.end = WhatOffset(pt, &(theSelect.endline));
			oldSelect.start = theSelect.end;
			oldSelect.startline = theSelect.endline;
			InvertRange(&oldSelect, (Rect *) 0L);
			oldSelect = theSelect;
		} else {
			InvertRange(&theSelect, (Rect *) 0L);
			theSelect.start = WhatOffset(pt, &(theSelect.startline));
			theSelect.end = theSelect.start;
			theSelect.endline = theSelect.startline;
			InvertRange(&theSelect, (Rect *) 0L);
			oldSelect = theSelect;
		}
		while(StillDown()) {
			GetMouse(&pt);
			theSelect.end = WhatOffset(pt, &(theSelect.endline));
			oldSelect.start = theSelect.end;
			oldSelect.startline = theSelect.endline;
			InvertRange(&oldSelect, (Rect *) 0L);
			oldSelect = theSelect;
		}
		if(theSelect.start > theSelect.end) {
		long t = theSelect.start;
			theSelect.start = theSelect.end;
			theSelect.end = t;
			t = theSelect.startline;
			theSelect.startline = theSelect.endline;
			theSelect.endline = t;
			oldSelect = theSelect;
		}
	}
}


int
DoMenu(long selection)
{
int menu = HiWord(selection);
int item = LoWord(selection);

	switch(menu) {

	case fileMenu:
		if(item == 9) return 0;
		if(item == 4) {
		Rect foom = PagerWindow->portRect;
			OffsetRect(&foom, -foom.left, -foom.top);
			InvalRect(&foom);
			return 1;
		}
		break;

	case editMenu:
		switch(item) {

		case 1:
			SysBeep(20);
			break;

		case 3:
			SysBeep(20);
			break;

		case 4:
			if(theSelect.start == theSelect.end) {
				SysBeep(20);
			} else {
				ZeroScrap();
				HLock(theData);
				PutScrap(theSelect.end - theSelect.start, 'TEXT', &((*theData)
					[theSelect.start]));
				HUnlock(theData);
			}
			break;

		case 5:
			SysBeep(20);
			break;

		default:
			break;

		}
		break;

	default:
		break;

	}

	return 1;
}


int
DoMDown(EventRecord * theEvent, term_info * t)
{
WindowPtr whatWindow;
int where;

	where = FindWindow(theEvent->where, &whatWindow);
	switch(where) {

	case inMenuBar:
		if(theSelect.start != theSelect.end) {
			EnableItem(editMenuH, 4);
		} else {
			DisableItem(editMenuH, 4);
		}
		where = DoMenu(MenuSelect(theEvent->where));
		HiliteMenu(0);
		return where;

	case inGrow:
	case inContent:
		if(StripAddress(whatWindow) == StripAddress(PagerWindow)) {
			DoClick(theEvent, t);
		} else {
			SysBeep(20);
		}
		break;

	case inDrag:
		if(StripAddress(whatWindow) == StripAddress(PagerWindow)) {
		RgnHandle theRgn = GetGrayRgn();
		Point p;
			DragWindow(PagerWindow, theEvent->where, &((*theRgn)->rgnBBox));
			windowSize = PagerWindow->portRect;
			p.h = windowSize.left;
			p.v = windowSize.top;
			LocalToGlobal(&p);
			OffsetRect(&windowSize, p.h, p.v);
		} else {
			SysBeep(20);
		}
		break;

	case inGoAway:
		if(TrackGoAway(PagerWindow, theEvent->where)) return 0;
		break;

	default:
		break;

	}
	return 1;
}


int
CheckEvent(EventRecord * theEvent, term_info * t)
{
	switch(theEvent->what) {

	case autoKey:
	case keyDown:
		return DoKey(theEvent, t);
		break;

	case updateEvt:
		DoDraw(theEvent, t);
		break;

	case mouseDown:
		return DoMDown(theEvent, t);
		break;

	case activateEvt:
		if(theEvent->modifiers & 1) {
			ShowControl(theScrollBar);
		} else {
			HideControl(theScrollBar);
		}
		break;

	case app4Evt:
		if(theEvent->message >> 24 == 1) {
			if(theEvent->message & 1) {
				HiliteControl(theScrollBar, scrollMax ? 0 : 254);
			} else {
				HiliteControl(theScrollBar, 255);
			}
		}

	default:
		break;
	}

	return 1;
}


void
MagicDisplay(term_info * t)
{
EventRecord theEvent;

	SetCursor(&ARROW_CURSOR);
	do {
		WaitNextEvent(everyEvent, &theEvent, 42L, 0L);
		SetPort(PagerWindow);
	} while(CheckEvent(&theEvent, t));
}


int
CountChars(char * s, long n, int c)
{
int r = 0;

	while(n-- > 0) if(*(s++) == c) r++;

	return r;
}


int
TabSize(char * s)
{
int r = 0, q;

	for(q = 0; s[q]; q++)
		r += (s[q] == '\t') ? 8 - (r & 7) : 1;

	return r;
}


int
ExpandTabs(char * s, char * d)
{
int r, q, c = 0, t = 0;

	for(q = 0; s[q]; q++)
		switch(s[q]) {

		case '\t':
			for(r = 0; r < 8 - (t & 7); r++, c++)
				*(d++) = 0x20;
			t += 8 - (t & 7);
			break;

		case '\n':
			*(d++) = '\r';
			c++;
			t = 0;
			break;

		default:
			*(d++) = s[q];
			c++;
			t++;
			break;
		}

	return c;
}


int
MoofFile(FILE * fp, int strip)
{
long			fpos;
long			delta_slack;
long			slack_left;
char			buf[MAX_LINE_LEN];

	if(!fp) {
		panic("No file for pager");
	}
	fpos = ftell(fp);
	length = 0;
	theData = NewHandle(SLACK);
	slack_left = SLACK;

	do {
		if(!fgets(buf, MAX_LINE_LEN, fp)) break;
		if(!strip || isspace(*buf)) {
			delta_slack = TabSize(buf);
			slack_left -= delta_slack;
			if(slack_left < 0) {
				slack_left += SLACK;
				SetHandleSize(theData, length + SLACK + delta_slack);
				if(MemError())
					panic("Out of memory");
			}
			MoveHHi(theData);
			HLock(theData);
			length += ExpandTabs(buf+strip, &((*theData)[length]));
			HUnlock(theData);
		}
	} while(!feof(fp) && (isspace(*buf) || !strip));

	fclose(fp);
	return 0;
}


void
setLineStarts(long ** starts, char * data, long length)
{
long x = length;
long p = 0;

	**starts = 0;
	while(length--)
		if(*(data++) == '\r')
			(*starts)[++p] = x - length;
}


void
MoreDisabling(MenuHandle theMenu, int i)
{
int x;

	switch(i) {

	default :
		DisableItem(theMenu, 0);
		break;

	case fileMenu:
		for(x = 1; x < 9; x++)
			DisableItem(theMenu, x);
		SetItem(theMenu, 9, "\PClose");
		EnableItem(theMenu, 9);
		EnableItem(theMenu, 0);
		EnableItem(theMenu, 4);
		break;

	case editMenu:
		for(x = 1; x < 7; x++)
			DisableItem(theMenu, x);
		EnableItem(theMenu, 0);
		editMenuH = theMenu;
		break;

	}
}


void
MoreEnabling(MenuHandle theMenu, int i)
{
int x;

	switch(i) {

	default :
		EnableItem(theMenu, 0);
		break;

	case fileMenu:
		for(x = 0; x < 9; x++) if(x != 6 && x != 8)
			EnableItem(theMenu, x);
		SetItem(theMenu, 9, "\PQuit");
		break;

	case editMenu:
		for(x = 1; x < 7; x++)
			EnableItem(theMenu, x);
		DisableItem(theMenu, 0);
		break;

	}
}


void
MoreMenus(void)
{
MenuHandle theMenu;
int i;

	for(i=appleMenu; i <=extendMenu; i++) {
		if(theMenu = GetMHandle(i)) {
			MoreDisabling(theMenu, i);
		}
	}
	DrawMenuBar();
}


void
LessMenus(void)
{
MenuHandle theMenu;
int i;

	for(i=appleMenu; i <=extendMenu; i++) {
		if(theMenu = GetMHandle(i)) {
			MoreEnabling(theMenu, i);
		}
	}	
	DrawMenuBar();
}


int
mac_more(FILE * fp, int strip)
{
WindowRecord	MoreWindow;
term_info		* t;
GrafPtr			savedPort;

	if(MoofFile(fp, strip)) return -1; /* God knows what happened */

	theSelect.start = theSelect.end = theSelect.startline =
		theSelect.endline = 0;
	oldSelect = theSelect;
	MoveHHi(theData);
	HLock(theData);
	noRs = CountChars(*theData, length, '\r');
	lineStarts = (long **) NewHandle((noRs + 1) * sizeof(long));
	if(!lineStarts || MemError()) panic("Out of memory");
	setLineStarts(lineStarts, *theData, length);
	HUnlock(theData);
	scrollMax = noRs - Y_SIZE;
	if(scrollMax < 0) scrollMax = 0;

	GetPort(&savedPort);
	t = (term_info *) GetWRefCon(HackWindow);

	if(t->inColor ? !GetNewCWindow(301, &MoreWindow, 0L) :
		!GetNewWindow(301, &MoreWindow, 0)) {
		DisposHandle(theData);
		panic("No WIND resource for pager");
	} else if(ResError()) {
		DisposHandle(theData);
		panic("No WIND resource for pager");
	} /* And now set the sizes & things */
	PagerWindow = (GrafPtr) &MoreWindow;

	width = t->charWidth;
	height = t->height;
	if(inited != t->height) {
		SetRect(&windowSize, X_POS, Y_POS, width * X_SIZE + X_BORDER + X_POS,
			height * Y_SIZE + Y_BORDER + Y_POS);
		inited = t->height;
	}	
	SizeWindow(PagerWindow, windowSize.right - windowSize.left,
		windowSize.bottom - windowSize.top, 0);
	MoveWindow(PagerWindow, windowSize.left, windowSize.top, 0);
	ShowWindow(PagerWindow);
	SelectWindow(PagerWindow);
	SetPort(PagerWindow);

	text_area = windowSize;
	OffsetRect(&text_area, - text_area.left, - text_area.top);
	text_area.left = text_area.right - 14;
	InsetRect(&text_area, - 1, - 1);
	theScrollBar = NewControl(PagerWindow, &text_area,
		"\PMore Text", 1, 0, 0, scrollMax, scrollBarProc, 0);
	HiliteControl(theScrollBar, scrollMax ? 0 : 254);

	SetRect(&text_area, LEFT_MARGIN, TOP_MARGIN, width * X_SIZE + LEFT_MARGIN,
		height * Y_SIZE + TOP_MARGIN);
	TextFont(t->fontNum);
	TextSize(t->fontSize);
	TextMode(srcCopy);
	textPos = 0;

	MoreMenus();
	MagicDisplay(t);
	LessMenus();

	if(theScrollBar) KillControls((WindowPtr) &MoreWindow);
	CloseWindow((WindowPtr) &MoreWindow);
	DisposHandle(theData);
	SetPort(HackWindow);
	docrt();
	SetPort(savedPort);
	return 0;
}
