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
		if(WaitNextEvent(-1, &theEvent, 0L, 0L)) {
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
	bounds = savedPort->portRect;
	InvalRect(&bounds);

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

#define QUIT_BUT 0
#define TOP_BUT 1
#define MORE_BUT 2
#define MORE_TEXT	20
#define LITTLE_MORE_TEXT 20
#define TOP_TEXT	-2
#define QUIT_PAGER	-1
#define NULL_EVT	-3


static Rect rectSave;
static int window_inited = 0;

int more_disabled;


void
draw_box(char * whence, size_t length, Rect * where, int strip)
{
Point mark;
size_t bar = 0;
GrafPtr aPort;

	TextFont(4);
	TextSize(9);
	GetPort(&aPort);
	ClipRect(where);

	mark.h = where->left + 3;
	mark.v = where->top + 11;

	while(length-- && mark.v < where->bottom) {
		MoveTo(mark.h, mark.v);

		switch(whence[bar]) {

		case 0x9:
			mark.h += 45;
			mark.h -= mark.h % 48;
			mark.h += 3;
			break;

		case 0x7:
			if(!flags.silent) {
				if(!flags.silent) SysBeep(20);
			}
			break;

		case 0xC:
			EraseRect(where);
			mark.h = where->left + 3;
			mark.v = where->top + 11;
			break;

		case 0xA:
		case 0xD:
			if(strip) bar++;
			mark.h = where->left + 3;
			mark.v += 11;
			MoveTo(mark.h, mark.v);
			break;

		default:
			if(PtInRect(mark, where))
				DrawChar(whence[bar]);
			mark.h += 6;
			break;
		}
		bar++;
	}
	ClipRect(&(aPort->portRect));
}


void
draw_btns(Rect * buttons)
{
int i;

	PenNormal();

	TextFont(0);	/* System Font: usually Chicago */
	TextSize(12);	/* 12 pt */
	for(i=0; i<3; i++) {
		EraseRect(&(buttons[i]));
		FrameRoundRect(&(buttons[i]), BUT_CORNER, BUT_CORNER);
		MoveTo(buttons[i].left + 5, buttons[i].top + 13);
		DrawText("Quit TopMore", i * 4, 4);
	}
	if(more_disabled) {
		PenPat(patBic);
		PenPat(gray);
		PaintRoundRect(&(buttons[MORE_BUT]), BUT_CORNER, BUT_CORNER);
		PenNormal();
	}
}


static char lite[3] = { 0, 0, 0, };


int
how_scroll(Rect * btns, GrafPtr port)
{
EventRecord theEvent;
int hit;
long finalTicks;

	while(!WaitNextEvent(everyEvent, &theEvent, 0L, 0L));
	SetPort(port);
	ClipRect(&(port->portRect));

	switch(theEvent.what) {

	case keyDown:
		switch(theEvent.message & 0xFF) {

		case 0x20:
		case 'M':
		case 'm':
			if(!more_disabled) {
				InvertRoundRect(&(btns[MORE_BUT]), BUT_CORNER, BUT_CORNER);
				lite[MORE_BUT] = 1;
				return MORE_TEXT;
			} else if(!flags.silent) SysBeep(20);
			break;

		case 0xD:
		case 0xA:
			if(!more_disabled) {
				InvertRoundRect(&(btns[MORE_BUT]), BUT_CORNER, BUT_CORNER);
				lite[MORE_BUT] = 1;
				return LITTLE_MORE_TEXT;
			} else if(!flags.silent) SysBeep(20);
			break;

		case 'Q':
		case 'q':
		case 0x3:
		case 0x1B:
			InvertRoundRect(&(btns[QUIT_BUT]), BUT_CORNER, BUT_CORNER);
			return QUIT_PAGER;

		case '.':		
			if(theEvent.modifiers & cmdKey) {
				InvertRoundRect(&(btns[QUIT_BUT]), BUT_CORNER, BUT_CORNER);
				return QUIT_PAGER;
			}
			break;

		case 't':
		case 'T':
			InvertRoundRect(&(btns[TOP_BUT]), BUT_CORNER, BUT_CORNER);
			return TOP_TEXT;

		default:
			if(!flags.silent) SysBeep(20);
		}
		break;
	case mouseDown:
		{
		WindowPtr xW;
		short part = FindWindow(theEvent.where, &xW);
			if(StripAddress(xW) != StripAddress(port)) {
				if(part == inDrag && theEvent.modifiers & cmdKey) {
				RgnHandle grayReg = GetGrayRgn();
				char hState = HGetState(grayReg);
					HLock(grayReg);
					DragWindow(xW, theEvent.where, &((*grayReg)->rgnBBox));
					HSetState(grayReg, hState);
				} else if(!flags.silent) SysBeep(20);
			} else {
				switch(part) {
	
				case inDrag:
					{
					RgnHandle grayReg = GetGrayRgn();
					char hState = HGetState(grayReg);
						HLock(grayReg);
						DragWindow(xW, theEvent.where, &((*grayReg)->rgnBBox));
						HSetState(grayReg, hState);
						rectSave = xW->portRect;
						LocalToGlobal((Point *) &(rectSave.top));
						LocalToGlobal((Point *) &(rectSave.bottom));
					}
					break;
	
				case inContent:
					hit = TrackThem(btns, lite, more_disabled ? 2 : 3);
					switch(hit) {
	
					case QUIT_BUT:
						return QUIT_PAGER;
	
					case TOP_BUT:
						return TOP_TEXT;
	
					case MORE_BUT:
						return MORE_TEXT;
	
					default:
						if(!flags.silent) SysBeep(20);
					}
					break;
	
				case inGoAway:
					if(TrackGoAway (xW, theEvent.where)) return QUIT_PAGER;
					break;

				default:
					if(!flags.silent) SysBeep(20);
					break;
	
				}
			}
		}
		break;

	case updateEvt:
		BeginUpdate((WindowPtr) theEvent.message);
		if(StripAddress(theEvent.message) ==
			StripAddress(FrontWindow())) {
			EndUpdate((WindowPtr) theEvent.message);
			return 0;
		} else if(StripAddress(HackWindow) ==
			StripAddress(theEvent.message)) {
			SetPort(HackWindow);
			docrt();
			SetPort(port);
		}
		EndUpdate((WindowPtr) theEvent.message);
		break;

	default:
		break;
	}
	return NULL_EVT;
}


void
display_data(char * buffer, size_t length, int strip, WindowPtr window)
{
Rect buttons[3], text_area, bounds;
size_t b_index = 0;
short pgEvent, bar, i;

	more_disabled = 0;
	text_area = window->portRect;
	OffsetRect(&text_area, text_area.left, text_area.top);
	InsetRect(&text_area, 3, 3);
	bounds = text_area;
	text_area.bottom -= 30;

	PenNormal();

	buttons[QUIT_BUT].left = bounds.left;
	buttons[QUIT_BUT].top = bounds.bottom - 18;
	buttons[QUIT_BUT].right = bounds.left + 39;
	buttons[QUIT_BUT].bottom = bounds.bottom;

	i = bounds.right;

	buttons[MORE_BUT].left = i - 45;
	buttons[MORE_BUT].top = bounds.bottom - 18;
	buttons[MORE_BUT].right = i;
	buttons[MORE_BUT].bottom = bounds.bottom;

	buttons[TOP_BUT].left = i - 90;
	buttons[TOP_BUT].top = bounds.bottom - 18;
	buttons[TOP_BUT].right = i - 50;
	buttons[TOP_BUT].bottom = bounds.bottom;

	do {
		bar = 0;
		pgEvent = how_scroll(buttons, window);
		if(pgEvent > 0) bar = MORE_BUT;
		if(pgEvent == TOP_TEXT) {
			EraseRect(&text_area);
			more_disabled = 0;
			b_index = 0;
			bar = TOP_BUT;
			pgEvent = 0;
		}
		if(pgEvent == 0) {
			draw_btns(buttons);
			lite[TOP_BUT] = 0;
		}
		if(pgEvent == NULL_EVT) {
			pgEvent = 0;
		} else {
		int f = 0;
			if(pgEvent > 0) f = 1;
			while(pgEvent > 0 && b_index < length) {
				while(buffer[b_index] != '\r' && b_index < length) b_index++;
				if(buffer[b_index++] == '\r') pgEvent--;
				else break;
			}
			if(!pgEvent) {
				if(f) EraseRect(&text_area);
				draw_box(&(buffer[b_index]), length - b_index, &text_area, strip);
			} else if(pgEvent > 0) {
				PenMode(patOr);
				PenPat(gray);
				PaintRoundRect(&(buttons[MORE_BUT]), BUT_CORNER,
					BUT_CORNER);
				more_disabled = 1;
				PenNormal();
			}
			if(lite[bar])
				InvertRoundRect(&(buttons[bar]), BUT_CORNER, BUT_CORNER);
		}
		for(i=0; i<3; i++) lite[i] = 0;

	} while (pgEvent >= 0);
}


int
mac_more(FILE * fp, int strip)
{
size_t where, length, b_index;
char ** buffer, c;
WindowRecord pageWindow;
WindowPtr oldWindow;
GrafPtr oldPort;
extern WindowPtr HackWindow;

	GetPort(&oldPort);
	oldWindow = FrontWindow();
	InitCursor();

	SelectWindow(HackWindow);
	SetPort(HackWindow);
	EraseRect(&(HackWindow->portRect));
	docrt();
	ValidRect(&(HackWindow->portRect));
		
	where = ftell(fp);
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	fseek(fp, where, SEEK_SET);

	if(length - where < 1)
		return -1;
	if((buffer = (char **) NewHandle(length)) == NULL)
		return -1;
	if(MemError()) {
		DisposHandle(buffer);
		return -1;
	}

	MoveHHi(buffer);
	HLock(buffer);

	fread(*buffer, sizeof(char), length - where, fp);
	c = (*buffer)[length - where - 1];
	if(!(c == 0xD || c == 0xA || c == 0)) {
		(*buffer)[length - where - 1] = 0;
	}

	while(where < length) {
		c = (*buffer)[where];
		if(c == '\n' || c == 0xC || c == 0)
			c =(*buffer)[where] = '\r';
		where++;
	}

	if(!window_inited) {
		rectSave = thePort->portRect;
		rectSave.left = rectSave.right - 490;
		rectSave.top = rectSave.bottom - 270;
		OffsetRect(&rectSave, 10 - (rectSave.left >> 1), 40 - (rectSave.top >> 1));
		window_inited = 1;
	}

	(void) NewWindow(&pageWindow, &rectSave, "\011Hack-Info", TRUE, documentProc,
		0L, TRUE, 0L);
	SelectWindow(&pageWindow);
	SetPort(&pageWindow);
	ClipRect(&(pageWindow.port.portRect));

	display_data(*buffer, length, strip, (WindowPtr) &pageWindow);
	HUnlock(buffer);
	DisposHandle(buffer);

	Delay(FLASH_TIME, &where);
	CloseWindow(&pageWindow);
	SelectWindow(oldWindow);
	SetPort(oldWindow);
	if(StripAddress(oldWindow) == StripAddress(HackWindow)) {
		EraseRect(&(oldWindow->portRect));
		ValidRect(&(oldWindow->portRect));
		docrt();
	}
	SetPort(oldPort);
	return 0;
}

