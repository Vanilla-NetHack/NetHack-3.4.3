/*    SCCS Id: @(#)winkey.c    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "amiga:windefs.h"
#include "amiga:winext.h"
#include "amiga:winproto.h"

amii_nh_poskey(x, y, mod)
    int*x, *y, *mod;
{
    struct amii_WinDesc *cw;
    WETYPE type;
    struct RastPort *rp;
    struct Window *w;

    if( cw = amii_wins[WIN_MESSAGE] )
    {
	cw->wflags &= ~FLMAP_SKIP;
	if( scrollmsg )
	    cw->wflags |= FLMSG_FIRST;
	cw->disprows = 0;
    }

    if( WIN_MAP != WIN_ERR && (cw = amii_wins[ WIN_MAP ]) && ( w = cw->win ) )
    {
	cursor_on( WIN_MAP );
#ifdef	VIEWWINDOW
	cursor_on( WIN_VIEW );
#endif
    }
    else
	panic( "no MAP window opened for nh_poskey\n" );

    rp = w->RPort;

    while( 1 )
    {
	type = WindowGetevent( );
	if( type == WEMOUSE )
	{
	    *mod = CLICK_1;
	    if( lastevent.un.mouse.qual )
		*mod = 0;

	    /* X coordinates are 1 based, Y are zero based. */
	    *x = ( (lastevent.un.mouse.x - w->BorderLeft) / MAPFTWIDTH ) + 1;
	    *y = ( ( lastevent.un.mouse.y - w->BorderTop - MAPFTBASELN ) /
			MAPFTHEIGHT );
	    return( 0 );
	}
	else if( type == WEKEY )
	{
	    lastevent.type = WEUNK;
	    return( lastevent.un.key );
	}
    }
}

int
amii_nhgetch()
{
    int ch;
    struct amii_WinDesc *cw=amii_wins[WIN_MESSAGE];

    if( WIN_MAP != WIN_ERR && amii_wins[ WIN_MAP ] )
    {
	cursor_on( WIN_MAP );
#ifdef	VIEWWINDOW
	cursor_on( WIN_VIEW );
#endif
    }
    if(cw)
		cw->wflags &= ~FLMAP_SKIP;

    ch = WindowGetchar();
    return( ch );
}

void
amii_get_nh_event()
{
    /* nothing now - later I have no idea.  Is this just a Mac hook? */
}

void
amii_getret()
{
    register int c;

    raw_print( "" );
    raw_print( "Press Return..." );

    c = 0;

    while( c != '\n' && c != '\r' )
    {
	if( HackPort )
	    c = WindowGetchar();
	else
	    c = getchar();
    }
    return;
}

