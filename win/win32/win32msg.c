/*	SCCS Id: @(#)win32msg.c	3.2	95/09/06	*/
/* Copyright (c) NetHack MS Windows Porting Team 1993, 1994       */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * The routines in this file build on the initial MS Windows NetHack 
 * groundwork done by Bill Dyer.
 *
 * This file contains all of the Win32 Window Procedures.
 */

#include "hack.h"

#ifdef WIN32_GRAPHICS

# ifndef NO_SIGNAL
#include <signal.h>
# endif
#include <ctype.h>
#include <sys\stat.h>
#include "win32api.h"
#include "nhwin32.h"
	
#ifdef DEBUG_FULL
static char debugbuf[256];
#endif

LONG WINAPI BaseWndProc(HWND hWnd,UINT messg,UINT wParam,LONG lParam)
{
	HDC hdc;
	static int xClientView,yClientView;
	static HWND hInst;
	static FARPROC fpfnAboutDiaProc;
	static RECT rcWindow;
	RECT rect;
	unsigned char ch;

	switch (messg)
	{
	   case WM_CREATE:
	     hdc = GetDC(hWnd);
	     SelectObject(hdc,hDefFnt);
	     GetTextMetrics(hdc, &tm);
	     ReleaseDC(hWnd,hdc);

	     DefCharWidth = tm.tmAveCharWidth;
	     DefCharHeight = tm.tmHeight;
	     BaseUnits = DefCharHeight;

	     GetClientRect(hWnd, &rect);
	     rect.top += GetSystemMetrics(SM_CYMENU) + 
			 (GetSystemMetrics(SM_CYFRAME) * 2) +
			 GetSystemMetrics(SM_CYSIZE);
	     rect.left += GetSystemMetrics(SM_CXFRAME) * 2;
	     rect.right = rect.left + (COLNO * DefCharWidth);
   	     rect.bottom = rect.top + ((5 + ROWNO + 2) * BaseUnits); 
	     (void) AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW, TRUE);
	     BaseWidth = rect.right - rect.left + 1;
	     BaseHeight = rect.bottom - rect.top + 1;
	     MoveWindow(hWnd, rect.left, rect.top, BaseWidth, BaseHeight, TRUE);
	     GetClientRect(hWnd, &rcClient);

	     MessageHeight  = BaseUnits * 5;
	     MessageWidth   = BaseWidth - GetSystemMetrics(SM_CXVSCROLL);
	     MessageX = rcClient.left;
	     MessageY = rcClient.top;
	     MapHeight = BaseUnits * ROWNO;
	     MapWidth  = BaseWidth;
	     MapX = rcClient.left;
	     MapY = rcClient.top + MessageHeight;
	     StatusHeight = BaseUnits * 2;
	     StatusWidth  = BaseWidth;
	     StatusX = rcClient.left;
	     StatusY = rcClient.top + MessageHeight + MapHeight;
	     break;
	  case WM_SIZE:
	     GetClientRect(hWnd, &rcClient);
	     EnumChildWindows(hWnd, EnumChildProc,(LPARAM)&rcClient);
	     return 0;
	     break;

	  case WM_CHAR:
	    switch(wParam) {
		default:
			ch = (unsigned char) wParam;
			if (pchPut >= pchBuf + (RINGBUFSIZE - 1)) {
				pchPut = pchBuf;		/* wrap it */
			}
			*pchPut++ = ch;
			++pchCount;
	    }
#ifdef DEBUG_FULL
	    sprintf(debugbuf,"Message = %X, wParam = %d, pchCount = %d",
				messg, wParam, pchCount);
	    DEBUG_MSG(debugbuf);
#endif
	    break;
	  case WM_DESTROY:
	    PostQuitMessage(0);
	    exit(EXIT_SUCCESS);
	    break;

	  default:
	    return(DefWindowProc(hWnd,messg,wParam,lParam));
	}
	return(0L);
}

LONG WINAPI TextWndProc(HWND hWnd,UINT messg,UINT wParam,LONG lParam)
{
	int idChild;
	HDC hdc;
#if 0
	char buf[BUFSZ];
#endif
	PAINTSTRUCT ps;

	static int xClientView,yClientView;
	static HWND hInst;
	static FARPROC fpfnAboutDiaProc;
	int row,col;
	int srow, scol;
	int colcount, rowcount;
	int offset;
	uchar ch;
	uchar *pch;
	int color;
	int *pcolor;
	/* RECT rect; */
	char tch[2]="X";

	idChild = GetWindowLong(hWnd, GWL_ID);
	switch (messg)
	{

          case WM_CREATE:
	    wins[idChild]->dwCharX = DefCharWidth;
	    wins[idChild]->dwCharY = DefCharHeight;
	    return 0;
#if 0
	  case WM_SIZE:
	    break;
#endif
	  case WM_PAINT:
	    HideCaret(hWnd);
	    hdc=BeginPaint(hWnd,&ps);
	    SelectObject(hdc,wins[idChild]->hFnt);

	    scol = ps.rcPaint.left / DefCharWidth;
	    srow = ps.rcPaint.top  / DefCharHeight;
	    rowcount = ((ps.rcPaint.bottom / DefCharHeight) - srow) + 1;
	    colcount = ((ps.rcPaint.right / DefCharWidth) - scol) + 1;
	    switch(wins[idChild]->type) {
	 	case NHW_MESSAGE:
		    col = ps.rcPaint.left / wins[NHW_MESSAGE]->dwCharX;
		    offset = (srow * wins[NHW_MESSAGE]->maxcols) + col;
		    SelectObject(hdc,wins[NHW_MESSAGE]->hFnt);
		    SetBkColor(hdc,wins[NHW_MESSAGE]->BackGroundColor);
		    for (row = srow; row <= srow + rowcount; ++row) {
		    	offset = (row * wins[NHW_MESSAGE]->maxcols + col);
		    	pcolor = wins[NHW_MESSAGE]->color + offset;
		    	pch = wins[NHW_MESSAGE]->data + offset;
		    	color = *pcolor;
		    	SetTextColor(hdc,color);
		    	TextOut(hdc,
				 ps.rcPaint.left,
				 (row * wins[NHW_MESSAGE]->dwCharY),
				 pch, colcount);
		    }
		    break;
		case NHW_TEXT:
		    SelectObject(hdc,wins[idChild]->hFnt);
		    SetBkColor(hdc,wins[idChild]->BackGroundColor);
		    colcount = wins[idChild]->widest;
		    for (row = srow; row <= srow + rowcount; ++row) {
			for (col = scol; col <= scol + colcount; ++col) { 
		    		offset = (row * wins[idChild]->maxcols) + col;
		    		pch = wins[idChild]->data + offset;
		    		pcolor = wins[idChild]->color + offset;
		    		ch = *pch;
		    		color = *pcolor;
		    		SetTextColor(hdc,color);
		    		TextOut(hdc,
	       			(col * wins[idChild]->dwCharX),
	       			(row * wins[idChild]->dwCharY),
				 &ch, 1);
			}
		     }
		     break;
		default:
		    SelectObject(hdc,wins[idChild]->hFnt);
		    SetBkColor(hdc,wins[idChild]->BackGroundColor);
		    colcount = wins[idChild]->maxcols;
		    for (row = srow; row <= srow + rowcount; ++row) {
			for (col = scol; col <= scol + colcount; ++col) { 
		    		offset = (row * wins[idChild]->maxcols) + col;
		    		pch = wins[idChild]->data + offset;
		    		pcolor = wins[idChild]->color + offset;
		    		ch = *pch;
		    		color = *pcolor;
		    		SetTextColor(hdc,color);
		    		TextOut(hdc,
	       			(col * wins[idChild]->dwCharX),
	       			(row * wins[idChild]->dwCharY),
				 &ch, 1);
			}
		     }
	    }
	    EndPaint(hWnd,&ps);
	    ShowCaret(hWnd);
	    break;

	  case WM_SETFOCUS:
	    if (idChild == WIN_MAP) {
	    	CreateCaret(hWnd,(HBITMAP)1,0,0);
		SetCaretPos(wins[idChild]->nCaretPosX,
			    wins[idChild]->nCaretPosY * wins[idChild]->dwCharY);
		ShowCaret(hWnd);
	    } else {
		SetFocus(wins[WIN_MAP]->hWnd);
	    }
	    break;

	  case WM_KILLFOCUS:
	    if (idChild == WIN_MAP) {
	    	HideCaret(hWnd);
		DestroyCaret();
	    }
	    break;

	  case WM_CHAR:
	    switch(wParam) {
		default:
			ch = (unsigned char) wParam;
			if (pchPut >= pchBuf + (RINGBUFSIZE - 1)) {
				pchPut = pchBuf;		/* wrap it */
			}
			*pchPut++ = ch;
			++pchCount;
	    }
#ifdef DEBUG_FULL
	    sprintf(debugbuf,"Message = %X, wParam = %d, pchCount = %d",
				messg, wParam, pchCount);
	    DEBUG_MSG(debugbuf);
#endif
	    break;

	  case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

	  case WM_VSCROLL:
	    switch(wParam) {
		case SB_LINEUP:
			break;
	    }
	    break;		
	  case WM_HSCROLL:
	  default:
	    return(DefWindowProc(hWnd,messg,wParam,lParam));
	}
	return(0L);
}

LONG WINAPI PopupWndProc(HWND hWnd,UINT messg,UINT wParam,LONG lParam)
{
	int idPopup;
	HDC hdc;
	PAINTSTRUCT ps;
	static int xClientView,yClientView;
	static HWND hInst;
	static FARPROC fpfnAboutDiaProc;

	idPopup = GetWindowLong(hWnd, GWL_ID);
	switch (messg)
	{

          case WM_CREATE:
	    hdc =GetDC(hWnd);
	    GetTextMetrics(hdc, &tm);
	    ReleaseDC(hWnd,hdc);
	    wins[idPopup]->dwCharX = tm.tmAveCharWidth;
	    wins[idPopup]->dwCharY = tm.tmHeight;
	    return 0;

	  case WM_PAINT:
	    hdc=BeginPaint(hWnd,&ps);

	    ValidateRect(hWnd,0);
	    EndPaint(hWnd,&ps);
	    break;

	  case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

	  case WM_VSCROLL:
	  case WM_HSCROLL:
	  default:
	    return(DefWindowProc(hWnd,messg,wParam,lParam));
	}
	return(0L);
}

LONG WINAPI ListboxWndProc(HWND hWnd,UINT messg,UINT wParam,LONG lParam)
{
	int idListbox;
	HDC hdc;
	static int xClientView,yClientView;
	static HWND hInst;

	idListbox = GetWindowLong(hWnd, GWL_ID);
	switch (messg)
	{

          case WM_CREATE:
	    hdc =GetDC(hWnd);
	    GetTextMetrics(hdc, &tm);
	    ReleaseDC(hWnd,hdc);
	    return 0;

	  case WM_SETFOCUS:
#if 0
	    CreateCaret(hWnd,(HBITMAP)1,0,0);
	    SetCaretPos(wins[idListbox]->nCaretPosX,
		wins[idListbox]->nCaretPosY * wins[idListbox]->dwCharY);
	    ShowCaret(hWnd);
	    break;
#endif
#if 0
	  case WM_KILLFOCUS:
	    HideCaret(hWnd);
	    DestroyCaret();
	    break;
#endif

	  case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

	  case WM_COMMAND:
	    switch (HIWORD(wParam)) {
	     case LBN_DBLCLK:
		DEBUG_MSG("Got LBN_DBLCLK");
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	     case LBN_KILLFOCUS:
		DEBUG_MSG("Got LBN_KILLFOCUS");
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	     case LBN_SELCANCEL:
		DEBUG_MSG("Got LBN_SELCANCEL");
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	     case LBN_SELCHANGE:
		DEBUG_MSG("Got LBN_SELCHANGE");
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	     case LBN_SETFOCUS:
		DEBUG_MSG("Got LBN_SETFOCUS");
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	     case WM_VKEYTOITEM:
		DEBUG_MSG("Got LBN_VKEYTOITEM");
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
		break;
	     default:
	        return(DefWindowProc(hWnd,messg,wParam,lParam));
	    }
	    break;
	  case WM_VSCROLL:
	  case WM_HSCROLL:
	  default:
	    return(DefWindowProc(hWnd,messg,wParam,lParam));
	}
	return(0L);
}

LRESULT CALLBACK MenuDialogProc(HWND hdlg,UINT messg,UINT wParam,LONG lParam)
{
	switch(messg) {
		case WM_INITDIALOG:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hdlg,0);
					break;
				default:
					return FALSE;
			}
			break;
		default:
			return FALSE;
		}
	return TRUE;
}

BOOL WINAPI AskNameProc(HWND hDlg, UINT messg, UINT wParam, LONG lParam)
{
	char buf[BUFSZ];
	HWND hEditBox;
	WPARAM maxsize;

	switch(messg) {
	    case WM_INITDIALOG:
		SendMessage(hDlg, DM_SETDEFID, (WPARAM)IDD_NAME, (LPARAM)0);

		maxsize = (WPARAM)((sizeof plname) - 1);
		hEditBox = GetDlgItem(hDlg, IDD_NAME);
		SendMessage(hEditBox, EM_LIMITTEXT, maxsize, (LPARAM)0);
		return TRUE;
	    case WM_COMMAND:
		switch(LOWORD(wParam)) {
		    case IDOK:
			/* Get number of characters */
			input_text_size = (int)SendDlgItemMessage(
					hDlg,
					IDD_NAME,
					EM_LINELENGTH,
					(WPARAM)0,
					(LPARAM)0);
			Sprintf(buf, "input_text_size = %d", input_text_size);
			/* Get the characters */
			if (input_text_size != 0) {
				*((LPWORD)input_text) = input_text_size;
				SendDlgItemMessage(hDlg,
					IDD_NAME,
					EM_GETLINE,
					(WPARAM)0,
					(LPARAM)(LPCSTR)input_text);
			}
			EndDialog(hDlg, TRUE);
			return TRUE;
		    case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		return 0;
	}
	return FALSE;
}
static int nCurrentChar;

BOOL WINAPI PlayerSelectProc(HWND hDlg, UINT messg, UINT wParam, LONG lParam)
{
	int i;

	switch (messg) {
		case WM_INITDIALOG:
			CheckRadioButton(hDlg,IDD_ARCH,IDD_RAND,IDD_RAND);
			nCurrentChar = IDD_RAND;
			return TRUE;
		case WM_COMMAND:
			switch (wParam) {
			   case IDOK:
				if (nCurrentChar == IDD_RAND) {
					i = rn2((int)strlen(pl_classes));
					pl_character[0] = pl_classes[i];
				} else	{
					pl_character[0] = 
					   pl_classes[nCurrentChar-IDD_ARCH];
				}
				EndDialog(hDlg,TRUE);
				return TRUE;
				break;
			   case IDCANCEL:
				pl_character[0] = 0;
				EndDialog(hDlg,FALSE);
				return TRUE;
				break;
			   case IDD_ARCH:
			   case IDD_BARB:
			   case IDD_CAVEMAN:
			   case IDD_ELF:
			   case IDD_HEAL:
			   case IDD_KNIGHT:
			   case IDD_PRIEST:
			   case IDD_ROGUE:
			   case IDD_SAM:
			   case IDD_TOUR:
			   case IDD_VAL:
			   case IDD_WIZ:
			   case IDD_RAND:
				nCurrentChar = wParam;
				CheckRadioButton(hDlg,IDD_ARCH,
						 IDD_RAND,wParam);
				return TRUE;
				break;
		}
	}
	return FALSE;
}

BOOL WINAPI CopyrightProc(HWND hDlg, UINT messg, UINT wParam, LONG lParam)
{
	switch (messg)
	{
		case WM_COMMAND:
			switch (wParam)
			{
			   case IDOK:
				EndDialog(hDlg,TRUE);
				return TRUE;
				break;
			}
			break;

		default:
			return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK EnumChildProc(hwndChild, lParam)
HWND hwndChild;
LPARAM lParam;
{
	LPRECT rcParent;
	int idChild;

	idChild = GetWindowLong(hwndChild, GWL_ID);
	rcParent = (LPRECT) lParam;

	switch(idChild) {	
	   case NHW_MESSAGE:
			MoveWindow(hwndChild,
				   MessageX, MessageY,
				   MessageWidth, MessageHeight, TRUE);
			wins[NHW_MESSAGE]->WindowWidth = MessageWidth;
			wins[NHW_MESSAGE]->WindowHeight = MessageHeight;
			wins[NHW_MESSAGE]->nWindowX = MessageX;
			wins[NHW_MESSAGE]->nWindowY = MessageY;
			break;

	   case NHW_MAP:
			MoveWindow(hwndChild,
				   MapX, MapY,
				   MapWidth, MapHeight, TRUE);
			wins[NHW_MAP]->WindowWidth = MapWidth;
			wins[NHW_MAP]->WindowHeight = MapHeight;
			wins[NHW_MAP]->nWindowX = MapX;
			wins[NHW_MAP]->nWindowY = MapY;
			break;

	   case NHW_STATUS:
			MoveWindow(hwndChild,
				   0, StatusY,
				   StatusWidth, StatusHeight, TRUE);
			wins[NHW_STATUS]->WindowWidth = StatusWidth;
			wins[NHW_STATUS]->WindowHeight = StatusHeight;
			wins[NHW_STATUS]->nWindowX = 0;
			wins[NHW_STATUS]->nWindowY = StatusY;
			break;
	}
	ShowWindow(hwndChild, SW_SHOW);
	return TRUE;
}

BOOL WINAPI AboutDiaProc(HWND hdlg,UINT messg,UINT wParam,LONG lParam)
{
	switch(messg) {
		case WM_INITDIALOG:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hdlg,0);
					break;
				default:
					return FALSE;
			}
			break;
		default:
			return FALSE;
		}
	return TRUE;
}


#endif /* WIN32_GRAPHICS */

/* win32msg.c */
