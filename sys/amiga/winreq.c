/*    SCCS Id: @(#)winreq.c    3.1    93/04/02 */
/* Copyright (c) Gregg Wonderly, Naperville, Illinois,  1991,1992,1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "amiga:windefs.h"
#include "amiga:winext.h"
#include "amiga:winproto.h"

#define GADBLUEPEN      2
#define GADREDPEN       3
#define GADGREENPEN     4
#define GADCOLOKAY      5
#define GADCOLCANCEL    6
#define GADCOLSAVE      7

UBYTE UNDOBUFFER[300];
SHORT BorderVectors1[] = { 0,0, 57,0, 57,11, 0,11, 0,0 };
struct Border Border1 = { -1,-1, 3,0,JAM1, 5, BorderVectors1, NULL };
struct IntuiText IText1 = { 3,0,JAM2, 4,1, NULL, (UBYTE *)"Cancel", NULL };
struct Gadget Gadget2 = {
    NULL, 9,15, 56,10, NULL, RELVERIFY, BOOLGADGET, (APTR)&Border1,
    NULL, &IText1, NULL, NULL, 1, NULL
};
UBYTE StrStringSIBuff[300];
struct StringInfo StrStringSInfo = {
    StrStringSIBuff, UNDOBUFFER, 0, 300, 0, 0,0,0,0,0, 0, 0, NULL
};
SHORT BorderVectors2[] = { 0,0, 439,0, 439,11, 0,11, 0,0 };
struct Border Border2 = { -1,-1, 3,0,JAM1, 5, BorderVectors2, NULL };
struct Gadget String = {
    &Gadget2, 77,15, 438,10, NULL, RELVERIFY+STRINGCENTER, STRGADGET,
    (APTR)&Border2, NULL, NULL, NULL, (APTR)&StrStringSInfo, 2, NULL
};

#define StrString \
   ((char *)(((struct StringInfo *)(String.SpecialInfo))->Buffer))

struct NewWindow StrWindow = {
    57,74, 526,31, 0,1, GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
    WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
    &String, NULL, NULL, NULL, NULL, 5,5, 0xffff,0xffff, CUSTOMSCREEN
};

#include "colorwin.c"

void
EditColor()
{
    extern const char *configfile;
    int i, done = 0, okay = 0;
    long code, qual, class;
    register struct Gadget *gd, *dgad;
    register struct Window *nw;
    register struct IntuiMessage *imsg;
    register struct PropInfo *pip;
    register struct Screen *scrn;
    long aidx;
    int msx, msy;
    int curcol = 0, drag = 0;
    int bxorx, bxory, bxxlen, bxylen;
    UWORD colors[ 1L << DEPTH ];
    static int once = 0;

    bxylen = Col_NewWindowStructure1.Height -
			    ( Col_BluePen.TopEdge + Col_BluePen.Height + 6 );
    bxxlen = Col_BluePen.Width;
    bxorx = Col_BluePen.LeftEdge;
    bxory = Col_BluePen.TopEdge + Col_BluePen.Height + 2;
    scrn = HackScreen;

    if( !once )
    {
	SetBorder( &Col_Okay );
	SetBorder( &Col_Cancel );
	SetBorder( &Col_Save );
	once = 1;
    }

    for( i = 0; i < (1L << DEPTH); ++i )
    {
	colors[ i ] = GetRGB4( scrn->ViewPort.ColorMap, i );
    }

    Col_NewWindowStructure1.Screen = scrn;
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	((struct PropInfo *)Col_BluePen.SpecialInfo)->Flags |= PROPNEWLOOK;
	((struct PropInfo *)Col_RedPen.SpecialInfo)->Flags |=  PROPNEWLOOK;
	((struct PropInfo *)Col_GreenPen.SpecialInfo)->Flags |= PROPNEWLOOK;
    }
#endif
    nw = OpenWindow( (void *)&Col_NewWindowStructure1 );
    PrintIText( nw->RPort, &Col_IntuiTextList1, 0, 0 );

    DrawCol( nw, curcol, colors );
    while( !done )
    {
	WaitPort( nw->UserPort );

	while( imsg = (struct IntuiMessage * )GetMsg( nw->UserPort ) )
	{
	    gd = (struct Gadget *)imsg->IAddress;
	    code = imsg->Code;
	    class = imsg->Class;
	    qual = imsg->Qualifier;
	    msx = imsg->MouseX;
	    msy = imsg->MouseY;

	    ReplyMsg( (struct Message *)imsg );

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == 'v' && qual == AMIGALEFT )
			okay = done = 1;
		    else if( code == 'b' && qual == AMIGALEFT )
			okay = 0, done = 1;
		    else if( code == 'o' || code == 'O' )
			okay = done = 1;
		    else if( code == 'c' || code == 'C' )
			okay = 0, done = 1;
		    break;

		case CLOSEWINDOW:
		    done = 1;
		    break;

		case GADGETUP:
		    drag = 0;
		    if( gd->GadgetID == GADREDPEN ||
					    gd->GadgetID == GADBLUEPEN ||
						gd->GadgetID == GADGREENPEN )
		    {
			pip = (struct PropInfo *)gd->SpecialInfo;
			aidx = pip->HorizPot / (MAXPOT/15);
			if( gd->GadgetID == GADREDPEN )
			{
			    colors[ curcol ] =
				( colors[ curcol ] & ~0xf00 ) | (aidx << 8);
			    LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
			}
			else if( gd->GadgetID == GADBLUEPEN )
			{
			    colors[ curcol ] =
					( colors[ curcol ] & ~0xf ) | aidx;
			    LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
			}
			else if( gd->GadgetID == GADGREENPEN )
			{
			    colors[ curcol ] =
				( colors[ curcol ] & ~0x0f0 ) | (aidx << 4);
			    LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
			}
			DispCol( nw, curcol, colors );
		    }
		    else if( gd->GadgetID == GADCOLOKAY )
		    {
			done = 1;
			okay = 1;
		    }
		    else if( gd->GadgetID == GADCOLSAVE )
		    {
		    	FILE *fp, *nfp;
		    	char buf[ 300 ], nname[ 300 ], oname[ 300 ];
			int once = 0;

    			fp = fopen( configfile, "r" );
			if( !fp )
			{
			    pline( "can't find NetHack.cnf" );
                    	    break;
			}

			strcpy( oname, dirname( configfile ) );
			if( oname[ strlen(oname)-1 ] != ':' )
			{
			    sprintf( nname, "%s/New_NetHack.cnf", oname );
			    strcat( oname, "/" );
			    strcat( oname, "Old_NetHack.cnf" );
			}
			else
			{
			    sprintf( nname, "%sNew_NetHack.cnf", oname );
			    strcat( oname, "Old_NetHack.cnf" );
			}

    			nfp = fopen( nname, "w" );
    			if( !nfp )
    			{
			    pline( "can't write to New_NetHack.cnf" );
                    	    fclose( fp );
                    	    break;
                    	}
			while( fgets( buf, sizeof( buf ), fp ) )
			{
			    if( strncmp( buf, "PENS=", 5 ) == 0 )
			    {
				once = 1;
			    	fputs( "PENS=", nfp );
			    	for( i = 0; i < (1l << DEPTH); ++i )
			    	{
			    	    fprintf( nfp, "%03x", colors[i] );
			    	    if(( i + 1 ) < (1l << DEPTH))
			    	        putc( '/', nfp );
			    	}
		    	        putc( '\n', nfp );
			    }
			    else
			    {
			    	fputs( buf, nfp );
			    }
			}

			/* If none in the file yet, now write it */
			if( !once )
			{
    		    	    fputs( "PENS=", nfp );
    		    	    for( i = 0; i < (1l << DEPTH); ++i )
    		    	    {
    		    	        fprintf( nfp, "%03x", colors[i] );
    		    	        if(( i + 1 ) < (1l << DEPTH))
    		    	            putc( ',', nfp );
    		    	    }
    	    	            putc( '\n', nfp );
			}
			fclose( fp );
			fclose( nfp );
			unlink( oname );
			if( filecopy( configfile, oname ) == 0 )
			    filecopy( nname, configfile );
			done = 1;
			okay = 1;
		    }
		    else if( gd->GadgetID == GADCOLCANCEL )
		    {
			done = 1;
			okay = 0;
		    }
		    break;

		case GADGETDOWN:
		    drag = 1;
		    dgad = gd;
		    break;

		case MOUSEMOVE:
		    if( !drag )
			break;
		    pip = (struct PropInfo *)dgad->SpecialInfo;
		    aidx = pip->HorizPot / (MAXPOT/15);
		    if( dgad->GadgetID == GADREDPEN )
		    {
			colors[ curcol ] =
				( colors[ curcol ] & ~0xf00 ) | (aidx << 8);
			LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
		    }
		    else if( dgad->GadgetID == GADBLUEPEN )
		    {
			colors[ curcol ] = ( colors[ curcol ] & ~0xf ) | aidx;
			LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
		    }
		    else if( dgad->GadgetID == GADGREENPEN )
		    {
			colors[ curcol ] =
				( colors[ curcol ] & ~0x0f0 ) | (aidx << 4);
			LoadRGB4( &scrn->ViewPort, colors, 1l << DEPTH );
		    }
		    DispCol( nw, curcol, colors );
		    break;

		case MOUSEBUTTONS:
		    if( code == SELECTDOWN )
		    {
			if( msy > bxory && msy < bxory + bxylen - 1 &&
				msx > bxorx && msx < bxorx + bxxlen - 1 )
			{
			    curcol = ( msx - bxorx )/(bxxlen / (1l << DEPTH));
			    DrawCol( nw, curcol, colors );
			}
		    }
		    break;
	    }
	}
    }

    if( okay )
    {
	for( i = 0; i < ( 1L << DEPTH ); ++i )
	    flags.amii_curmap[ i ] = colors[ i ];
    }

    LoadRGB4( &scrn->ViewPort, flags.amii_curmap, 1L << DEPTH );
    CloseWindow( nw );
}

char *dirname( str )
    char *str;
{
    char *t, c;
    static char dir[ 300 ];

    t = strrchr( str, '/' );
    if( !t )
	t = strrchr( str, ':' );
    if( !t )
	t = str;
    else
    {
    	c = *t;
    	*t = 0;
    	strcpy( dir, str );
    	*t = c;
    }
    return( dir );
}

char *basename( str )
    char *str;
{
    char *t;

    t = strrchr( str, '/' );
    if( !t )
	t = strrchr( str, ':' );
    if( !t )
	t = str;
    else
	++t;
    return( t );
}

filecopy( from, to )
    char *from, *to;
{
    char *buf;
    int i = 0;

    buf = malloc( strlen(to) + strlen(from) + 20 );
    if( buf )
    {
    	sprintf( buf, "c:copy \"%s\" \"%s\" clone", from, to );

    	/* Check SysBase instead?  Shouldn't matter  */
#ifdef	INTUI_NEW_LOOK
	if( IntuitionBase->LibNode.lib_Version >= 37 )
	    i = System( buf, NULL );
	else
#endif
	    Execute( buf, NULL, NULL );
    	free( buf );
    }
    else
    {
    	return( -1 );
    }
    return( i );
}

/* The colornames, and the default values for the pens */
static struct
{
    char *name, *defval;
} colnames[] =
{
    "Black","(000)",
    "White","(fff)",
    "Brown","(830)",
    "Cyan","(7ac)",
    "Green","(181)",
    "Magenta","(c06)",
    "Blue","(23e)",
    "Red","(c00)",
};

void
DrawCol( w, idx, colors )
    struct Window *w;
    int idx;
    UWORD *colors;
{
    int bxorx, bxory, bxxlen, bxylen;
    int i, incx, incy, r, g, b, yh = 4;
    long mflags;

    if( w->WScreen->Height > 300 )
	yh *= 2;
    bxylen = Col_NewWindowStructure1.Height - (Col_Okay.Height + txheight + 8) -
		    ( Col_BluePen.TopEdge + Col_BluePen.Height + 10 ) + yh;
    bxxlen = Col_BluePen.Width - 2;
    bxorx = Col_BluePen.LeftEdge + 1;
    bxory = Col_BluePen.TopEdge + Col_BluePen.Height + 2;

    incx = bxxlen / (1L << DEPTH);
    incy = bxylen - 2;

    SetAPen( w->RPort, C_WHITE );
    SetBPen( w->RPort, C_BLACK );
    SetDrMd( w->RPort, JAM2 );
    RectFill( w->RPort, bxorx, bxory, bxorx + bxxlen - 1, bxory + bxylen );

    SetAPen( w->RPort, C_BLACK );
    RectFill( w->RPort, bxorx+2, bxory+1,
				    bxorx + bxxlen - 4, bxory + bxylen - 1);

    for( i = 0; i < (1L << DEPTH); ++i )
    {
	if( i == idx )
	{
	    SetAPen( w->RPort, scrnpens[ SHINEPEN ] );
	    Move( w->RPort, bxorx + 3 + (i*incx)+0, bxory+bxylen - 1);
	    Draw( w->RPort, bxorx + 3 + (i*incx)+0, bxory + 1 );
	    Draw( w->RPort, bxorx + ((i+1)*incx)-1, bxory + 1 );

	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + 3 + (i*incx)+1, bxory + 2 );
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2 );

	    SetAPen( w->RPort, scrnpens[ SHINEPEN ] );
	    Move( w->RPort, bxorx + 3 + (i*incx)+0, bxory+bxylen - 1);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-1, bxory+bxylen - 1);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-1, bxory + 1 );

	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2);
	}
	else
	{
	    SetAPen( w->RPort, scrnpens[ SHINEPEN ] );
	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + 3 + (i*incx)+1, bxory + 2 );
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2 );
	    Move( w->RPort, bxorx + 3 + (i*incx)+1, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory+bxylen - 2);
	    Draw( w->RPort, bxorx + ((i+1)*incx)-2, bxory + 2);
	}

	SetAPen( w->RPort, i );
	RectFill( w->RPort, bxorx + 3 + (i*incx)+3, bxory + 4,
				    bxorx + ((i+1)*incx)-4, bxory+bxylen - 4);
    }

    DispCol( w, idx, colors );

    r = (colors[ idx ] & 0xf00) >> 8;
    g = (colors[ idx ] & 0x0f0) >> 4;
    b = colors[ idx ] & 0x00f;

    mflags = AUTOKNOB|FREEHORIZ;
#ifdef  INTUI_NEW_LOOK
    if( IntuitionBase->LibNode.lib_Version >= 37 )
    {
	mflags |= PROPNEWLOOK;
    }
#endif
    NewModifyProp( &Col_RedPen, w, NULL, mflags, (r * MAXPOT ) / 15, 0,
							    MAXPOT/15, 0, 1 );
    NewModifyProp( &Col_GreenPen, w, NULL, mflags, (g * MAXPOT ) / 15, 0,
							    MAXPOT/15, 0, 1 );
    NewModifyProp( &Col_BluePen, w, NULL, mflags, (b * MAXPOT ) / 15, 0,
							    MAXPOT/15, 0, 1 );
}

void
DispCol( w, idx, colors )
    struct Window *w;
    int idx;
    UWORD *colors;
{
    char buf[ 50 ];

    Move( w->RPort, Col_Save.LeftEdge,
	Col_Save.TopEdge - 4 );
    sprintf( buf, "%s=%03x default=%s%s", colnames[idx].name, colors[idx],
	colnames[idx].defval,
	"        "+strlen(colnames[idx].name)+1 );
    SetAPen( w->RPort, C_WHITE );
    SetBPen( w->RPort, 0 );
    SetDrMd( w->RPort, JAM2 );
    Text( w->RPort, buf, strlen( buf ) );
}

void
amii_setpens()
{
    /* If the pens are set in NetHack.cnf, we can get called before
     * HackScreen has been opened...
     */
    if( HackScreen != NULL )
    {
	LoadRGB4( &HackScreen->ViewPort, flags.amii_curmap, 1L << DEPTH );
    }
}

/* Generate a requester for a string value. */

void amii_getlin(prompt,bufp)
    const char *prompt;
    char *bufp;
{
    getlind(prompt,bufp,0);
}

/* and with default */
void getlind(prompt,bufp, dflt)
    const char *prompt;
    char *bufp;
    const char *dflt;
{
#ifndef TOPL_GETLINE
    register struct Window *cwin;
    register struct IntuiMessage *imsg;
    register long class, code, qual;
    register int aredone = 0;
    register struct Gadget *gd;
    static int once;

    *StrString = 0;
    if( dflt )
	strcpy( StrString, dflt );
    StrWindow.Title = (UBYTE *)prompt;
    StrWindow.Screen = HackScreen;

    if( !once )
    {
	if( bigscreen )
	    StrWindow.TopEdge = (HackScreen->Height/2) - (StrWindow.Height/2);
	SetBorder( &String );
	SetBorder( &Gadget2 );
	once = 1;
    }

    if( ( cwin = OpenWindow( (void *)&StrWindow ) ) == NULL )
    {
	return;
    }

    WindowToFront( cwin );
    while( !aredone )
    {
	WaitPort( cwin->UserPort );
	while( ( imsg = (void *) GetMsg( cwin->UserPort ) ) != NULL )
	{
	    class = imsg->Class;
	    code = imsg->Code;
	    qual = imsg->Qualifier;
	    gd = (struct Gadget *) imsg->IAddress;

	    switch( class )
	    {
		case VANILLAKEY:
		    if( code == '\033' && (qual &
			    (IEQUALIFIER_LALT|IEQUALIFIER_RALT|
			    IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND) ) == 0 )
		    {
			if( bufp )
			{
			    bufp[0]='\033';
			    bufp[1]=0;
			}
			aredone = 1;
		    }
		    else
		    {
			ActivateGadget( &String, cwin, NULL );
		    }
		    break;

		case ACTIVEWINDOW:
		    ActivateGadget( &String, cwin, NULL );
		    break;

		case GADGETUP:
		    switch( gd->GadgetID )
		    {
			case 2:
			    aredone = 1;
			    if( bufp )
				strcpy( bufp, StrString );
			    break;

			case 1:
			    if( bufp )
			    {
				bufp[0]='\033';
				bufp[1]=0;
			    }
			    aredone = 1;
			    break;
		    }
		    break;

		case CLOSEWINDOW:
		    if( bufp )
		    {
			bufp[0]='\033';
			bufp[1]=0;
		    }
		    aredone = 1;
		    break;
	    }
	    ReplyMsg( (struct Message *) imsg );
	}
    }

    CloseWindow( cwin );
#else
    struct amii_WinDesc *cw;
    struct Window *w;
    int colx, ocolx, c;
    char *obufp;

    amii_clear_nhwindow( WIN_MESSAGE );
    amii_putstr( WIN_MESSAGE, 0, prompt );
    cw = amii_wins[ WIN_MESSAGE ];
    w = cw->win;
    ocolx = colx = strlen( prompt ) + 1;

    obufp = bufp;
    cursor_on(WIN_MESSAGE);
    while((c = WindowGetchar()) != EOF)
    {
	cursor_off(WIN_MESSAGE);
	amii_curs( WIN_MESSAGE, colx, 0 );
	if(c == '\033')
	{
	    *obufp = c;
	    obufp[1] = 0;
	    return;
	}
	else if(c == '\b')
	{
	    if(bufp != obufp)
	    {
		bufp--;
		amii_curs( WIN_MESSAGE, --colx, 0);
		Text( w->RPort, "\177 ", 2 );
		amii_curs( WIN_MESSAGE, colx, 0);
	    }
	    else
		DisplayBeep( NULL );
	}
	else if( c == '\n' || c == '\r' )
	{
	    *bufp = 0;
	    TOPL_NOSPACE;
	    amii_putstr( WIN_MESSAGE, -1, obufp );
	    TOPL_SPACE;
	    return;
	}
	else if(' ' <= c && c < '\177')
	{
		/* avoid isprint() - some people don't have it
		   ' ' is not always a printing char */
	    *bufp = c;
	    bufp[1] = 0;

	    Text( w->RPort, bufp, 1 );
	    Text( w->RPort, "\177", 1 );
	    if(bufp-obufp < BUFSZ-1 && bufp-obufp < COLNO)
	    {
		colx++;
		bufp++;
	    }
	}
	else if(c == ('X'-64) || c == '\177')
	{
	    amii_curs( WIN_MESSAGE, ocolx, 0 );
	    Text( w->RPort,
		"                                                            ",
		colx - ocolx );
	    amii_curs( WIN_MESSAGE, colx = ocolx, 0 );
	} else
	    DisplayBeep( NULL );
	cursor_on(WIN_MESSAGE);
    }
    cursor_off(WIN_MESSAGE);
    *bufp = 0;
#endif
}

void amii_change_color( pen, val, rev )
    int pen, rev;
    long val;
{
    if( rev )
	flags.amii_curmap[ pen ] = ~val;
    else
	flags.amii_curmap[ pen ] = val;

    if( HackScreen )
	LoadRGB4( &HackScreen->ViewPort, flags.amii_curmap, 1L << DEPTH );
}

char *
amii_get_color_string( )
{
    int i;
    char s[ 10 ];
    static char buf[ 100 ];

    *buf = 0;
    for( i = 0; i < DEPTH; ++i )
    {
    	sprintf( s, "%s%03lx", i ? "/" : "", (long)flags.amii_curmap[ i ] );
    	strcat( buf, s );
    }

    return( buf );
}
