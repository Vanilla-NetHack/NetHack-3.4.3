/*    SCCS Id: @(#)wbgads.c     1.0   93/04/18			  */
/*    Copyright (c) Gregg Wonderly, Naperville IL, 1993	  	  */
/* NetHack may be freely redistributed.  See license for details. */

/* wbgads.c */
static void GadSpaceOut(struct OPTGAD *gads , int row , int maxx);
static void CompSpaceOut(struct OPTGAD *gads , int row , int maxx);

extern NEARDATA struct flag flags;

#define INITX	7
#define GADBORD	3
#define	YSPACE	2
#define XSPACE	3

#define MAXGADSTRLEN	2000

static char undobuffer[ MAXGADSTRLEN + 1 ];

static struct TextAttr textfont;
static int compgadid;

char **compvals;

#ifdef	TESTING
void SetBorder( register struct Gadget *gd, int val );

main( argc, argv )
    int argc;
    char **argv;
{
    int done = 0;
    struct IntuiMessage *imsg;
    struct Window *w;
    struct Screen *screen;
    int cury = -1, gadid = 1;
    struct OPTGAD *gp, *boolgads, *compgads;

    screen = LockPubScreen( NULL );
    UnlockPubScreen( NULL, screen );

    w = OpenWindowTags( NULL,
	WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_NEWSIZE|IDCMP_SIZEVERIFY,
	WA_Flags, WFLG_DRAGBAR|WFLG_SIZEGADGET|WFLG_DEPTHGADGET|
		  WFLG_CLOSEGADGET,
	WA_SmartRefresh, 1,
	WA_NoCareRefresh, 1,
	WA_MinWidth, 100,
	WA_MinHeight, 50,
	WA_PubScreen, screen,
	WA_Width, 640,
	WA_Height, 400,
	WA_Top, 0,
	WA_Left, 0,
	TAG_DONE );

    if( w )
    {
	textfont.ta_Name = w->RPort->Font->tf_Message.mn_Node.ln_Name;
	textfont.ta_YSize = w->RPort->Font->tf_YSize;
	textfont.ta_Style = w->RPort->Font->tf_Style;
	textfont.ta_Flags = w->RPort->Font->tf_Flags;
	boolgads = LayoutBoolOpts( w->Width - w->BorderLeft - w->BorderRight - 5,
			w->Height - w->BorderTop - w->BorderBottom - 6,
			w->RPort, &cury, &gadid );
	if( boolgads )
	{
	    for( gp = boolgads; gp; gp = gp->next )
		AddGList( w, &gp->gad, 0, 1, NULL );
	    RefreshGList( w->FirstGadget, w, NULL, -1 );
	}
	compgads = LayoutCompOpts( w->Width - w->BorderLeft - w->BorderRight - 5,
			w->Height - w->BorderTop - w->BorderBottom - 6, w->RPort,
			&cury, &gadid );
	if( compgads )
	{
	    for( gp = compgads; gp; gp = gp->next )
		AddGList( w, &gp->gad, 0, 1, NULL );
	    RefreshGList( w->FirstGadget, w, NULL, -1 );
	}
    }

    while( !done && w )
    {
	WaitPort( w->UserPort );
	while( imsg = (struct IntuiMessage *) GetMsg( w->UserPort ) )
	{
	    if( imsg->Class == CLOSEWINDOW )
		    done = 1;
	    else if( imsg->Class == SIZEVERIFY )
	    {
		for( gp = boolgads; gp; gp = gp->next )
		    RemoveGList( w, &gp->gad, 1 );
		FreeBoolOpts( boolgads );
		for( gp = compgads; gp; gp = gp->next )
		    RemoveGList( w, &gp->gad, 1 );
		FreeCompOpts( compgads );
	    }
	    else if( imsg->Class == NEWSIZE )
	    {
	    	cury = -1;
		gadid = 1;
		SetAPen( w->RPort, 0 );
		SetBPen( w->RPort, 0 );
		SetDrMd( w->RPort, JAM2 );
		RectFill( w->RPort, w->BorderLeft, w->BorderTop,
				    w->Width - w->BorderRight - 1,
				    w->Height - w->BorderBottom - 1 );
		SetAPen( w->RPort, 1 );
		SetBPen( w->RPort, 0 );
		boolgads = LayoutBoolOpts( w->Width - w->BorderLeft - w->BorderRight - 5,
		    w->Height - w->BorderTop - w->BorderBottom - 6, w->RPort,
			&cury, &gadid );
		for( gp = boolgads; gp; gp = gp->next )
		    AddGList( w, &gp->gad, 0, 1, NULL );
		compgads = LayoutCompOpts( w->Width - w->BorderLeft - w->BorderRight - 5,
		    w->Height - w->BorderTop - w->BorderBottom - 6, w->RPort,
			&cury, &gadid );
		for( gp = compgads; gp; gp = gp->next )
		    AddGList( w, &gp->gad, 0, 1, NULL );
		RefreshGList( w->FirstGadget, w, NULL, -1 );
	    }

	    ReplyMsg( (struct Message *)imsg );
	}
    }
    if( w ) CloseWindow( w );
    if( boolgads ) FreeBoolOpts( boolgads );
    if( compgads ) FreeCompOpts( compgads );

    return( 0 );
}

#endif

int AllocCompVals( void )
{
    int i;
    if( compvals == 0 )
    {
	for( i = 0; compopt[ i ].name; ++i )
	    continue;
	compvals = malloc( (i+1) * sizeof( char * ) );
	if( compvals == 0 )
	    return 0;
	for( i = 0; compopt[ i ].name; ++i )
	{
	    if( ( compvals[ i ] = malloc( compopt[ i ].size + 1 ) ) == NULL )
		return 0;
	    *compvals[ i ] = 0;
	}

	/* Create a null pointer terminator */
	compvals[ i ] = 0;
    }
    return 1;
}

struct OPTGAD *
LayoutCompOpts( maxx, maxy, rp, cury, gadid )
    int maxx, maxy;
    struct RastPort *rp;
    int *cury;
    int *gadid;

{
    int len;
    struct OPTGAD *gp;
    char *bp;
    struct Gadget *gd;
    struct StringInfo *sp;
    struct OPTGAD *compgads;
    struct IntuiText *ip;
    int i, curx;
    int incy;

    compgadid = *gadid;
    incy = rp->TxHeight + GADBORD + YSPACE;

    compgads = 0;
    curx = INITX;

    /* The value of 8 used here is not related to GADBORD!
     * It is an estimate of screen->WBorTop + 1, with some
     * extra white space for a border between the top
     * of the window and the gadgets.
     */

    textfont.ta_Name = rp->Font->tf_Message.mn_Node.ln_Name;
    textfont.ta_YSize = rp->Font->tf_YSize;
    textfont.ta_Style = rp->Font->tf_Style;
    textfont.ta_Flags = rp->Font->tf_Flags;

    if( *cury == -1 )
	*cury = rp->TxHeight + 4;

    if( AllocCompVals() == 0 )
    	return( NULL );
    
    for( i = 0; compopt[ i ].name; ++i )
    {
	len = ( ( maxx - 20 - GADBORD ) / 2 ) + GADBORD;

	if( curx + len > maxx )
	{
	    CompSpaceOut( compgads, *cury, maxx );
	    curx = INITX;
	    *cury += incy;
	}

	gp = malloc( sizeof( *gp ) );
	ip = malloc( sizeof( *ip ) );
	sp = malloc( sizeof( *sp ) );
	bp = malloc( compopt[i].size + 1 );
	if( !gp || !ip || !sp || !bp )
	{
	    if( gp ) free( gp );
	    if( ip ) free( ip );
	    if( sp ) free( sp );
	    if( bp ) free( bp );
	    FreeCompOpts( compgads );
	    return( NULL );
	}
	memset( gp, 0, sizeof( *gp ) );
	memset( ip, 0, sizeof( *ip ) );
	memset( sp, 0, sizeof( *sp ) );
	gd = &gp->gad;
	gd->LeftEdge = curx + TextLength( rp, (char *)compopt[ i ].name,
			    strlen( (char *)compopt[ i ].name ) ) + 10;
	curx += len + GADBORD;
	gd->Width = len - (TextLength( rp, (char *)compopt[ i ].name,
			    strlen( (char *)compopt[ i ].name ) ) + 10 );
	gd->TopEdge = *cury;
	gd->Height = incy - (YSPACE * 2);
	gd->Flags = GADGHCOMP;
	gd->Activation = RELVERIFY;
	gd->GadgetType = STRGADGET;
	gd->GadgetRender = gd->SelectRender = 0;
	gd->GadgetText = ip;
	gd->MutualExclude = 0;
	gd->SpecialInfo = (APTR)sp;

	sp->Buffer = bp;
	memcpy( bp, compvals[ i ], compopt[i].size );
	bp[ compopt[i].size ] = 0;
	sp->UndoBuffer = undobuffer;
	sp->MaxChars = compopt[i].size;
	gp->val = bp;

	gd->GadgetID = (*gadid)++;
	gd->UserData = 0;
	gp->next = compgads;
	compgads = gp;

	ip->FrontPen = 1;
	ip->BackPen = 0;
	ip->DrawMode = JAM1;
	ip->LeftEdge = -( TextLength( rp, (char *)compopt[ i ].name,
			    strlen( (char *)compopt[ i ].name ) ) + 10);
	ip->TopEdge = ((gd->Height - rp->TxHeight)/2);
	ip->ITextFont = &textfont;	/* rp->Font will be used */
	ip->IText = (char *)compopt[ i ].name;
	ip->NextText = 0;
	SetBorder( gd, 0 );
    }

    /* Perhaps leave last row ragged? */
    if( curx != INITX )
	CompSpaceOut( compgads, *cury, maxx );

    *cury += incy;
    return( compgads );
}

struct OPTGAD *
LayoutBoolOpts( maxx, maxy, rp, cury, gadid )
    int maxx, maxy;
    struct RastPort *rp;
    int *cury;
    int *gadid;
{
    int len;
    struct OPTGAD *gp;
    struct Gadget *gd;
    struct OPTGAD *boolgads;
    struct IntuiText *ip;
    int i, curx;
    int incy;

    incy = rp->TxHeight + GADBORD + YSPACE;

    boolgads = 0;
    curx = INITX;

    /* The value of 5 used here is not related to GADBORD!
     * It is an estimate of screen->WBorTop + 1, with some
     * extra white space for a border are between the top
     * of the window and the gadgets.
     */

    if( *cury == -1 )
	*cury = rp->TxHeight + 5;

    textfont.ta_Name = rp->Font->tf_Message.mn_Node.ln_Name;
    textfont.ta_YSize = rp->Font->tf_YSize;
    textfont.ta_Style = rp->Font->tf_Style;
    textfont.ta_Flags = rp->Font->tf_Flags;

    for( i = 0; boolopt[ i ].name; ++i )
    {
    	/* Null pointers indicate options which we do not have available to us */
    	if( boolopt[ i ].addr == NULL )
    	{
	    (*gadid)++;
	    continue;
    	}
	len = TextLength( rp, (char *)boolopt[ i ].name,
			    strlen( (char *)boolopt[ i ].name ) );

	if( curx + len > maxx )
	{
	    GadSpaceOut( boolgads, *cury, maxx );
	    curx = INITX;
	    *cury += incy;
	}

	gp = malloc( sizeof( *gp ) );
	ip = malloc( sizeof( *ip ) );
	if( !gp || !ip )
	{
	    if( gp ) free( gp );
	    if( ip ) free( ip );
	    FreeBoolOpts( boolgads );
	    return( NULL );
	}
	memset( gp, 0, sizeof( *gp ) );
	memset( ip, 0, sizeof( *ip ) );
	gd = &gp->gad;
	gd->LeftEdge = curx;
	curx += len + GADBORD + XSPACE;
	gd->Width = len + GADBORD;
	gd->TopEdge = *cury;
	gd->Height = incy - YSPACE;
	gd->Flags = GFLG_GADGHCOMP;
	if( *boolopt[ i ].addr == TRUE )
	    gd->Flags |= GFLG_SELECTED;
	gd->Activation = GACT_IMMEDIATE|GACT_TOGGLESELECT|GACT_RELVERIFY;
	gd->GadgetType = GTYP_BOOLGADGET;
	gd->GadgetRender = gd->SelectRender = 0;
	gd->GadgetText = ip;
	gd->MutualExclude = 0;
	gd->SpecialInfo = 0;
	gd->GadgetID = (*gadid)++;
	gd->UserData = 0;
	gp->next = boolgads;
	boolgads = gp;

	ip->FrontPen = 1;
	ip->BackPen = 0;
	ip->DrawMode = JAM1;
	ip->LeftEdge = (gd->Width - len)/2;
	ip->TopEdge = ((gd->Height - rp->TxHeight)/2) + 1;
	ip->ITextFont = &textfont;	/* rp->Font will be used */
	ip->IText = (char *)boolopt[ i ].name;
	ip->NextText = 0;
	SetBorder( gd, 1 );
    }

    /* Perhaps leave last row ragged? */
    if( curx != INITX )
	GadSpaceOut( boolgads, *cury, maxx );

    *cury += incy + GADBORD;
    return( boolgads );
}

static void
CompSpaceOut( gads, row, maxx )
    struct OPTGAD *gads;
    int row, maxx;
{
    struct Gadget *gd;
    struct OPTGAD *gp;
    int cnt, tlen;

    tlen = cnt = 0;

    for( gp = gads; gp; gp = gp->next )
    {
	if( gp->gad.TopEdge == row+1 )
	    ++cnt;
    }

    if( cnt > 1)
    {
	for( gp = gads; gp; gp = gp->next )
	{
	    gd = &gp->gad;
	    if( gd->TopEdge == row+1 && cnt != 0 )
	    {
		if( gd->LeftEdge > maxx/2 )
		    gd->LeftEdge += maxx - (gd->LeftEdge + gd->Width) + 2;
	    }
	}
    }
}

static void
GadSpaceOut( gads, row, maxx )
    struct OPTGAD *gads;
    int row, maxx;
{
    struct Gadget *gd;
    struct OPTGAD *gp;
    int cnt, tlen, mod, inc;

    tlen = cnt = 0;

    for( gp = gads; gp; gp = gp->next )
    {
	gd = &gp->gad;
	if( gd->TopEdge == row )
	{
	    ++cnt;
	    tlen += gd->Width + XSPACE;
	}
    }

    if( tlen < maxx && cnt > 1)
    {
    	inc = ( maxx - tlen ) / (cnt-1);
    	mod = ( maxx - tlen ) % (cnt-1);

	for( gp = gads; gp; gp = gp->next )
	{
	    gd = &gp->gad;
	    if( gd->TopEdge == row && cnt != 0 )
	    {
		gd->LeftEdge += (inc * --cnt);
		if( mod )
		    gd->LeftEdge += --mod;
	    }
	}
    }
}

void
FreeCompOpts( compgads )
    register struct OPTGAD *compgads;
{
    register struct OPTGAD *gp;
    register struct IntuiText *ip;
    struct StringInfo *sp;
    struct Gadget *gd;

    while( gp = compgads )
    {
	compgads = compgads->next;
	gd = &gp->gad;

	if( ip = gd->GadgetText )
	{
	    if( ip->IText ) free( ip->IText );
	    free( ip );
	}
	if( sp = (struct StringInfo *)gd->SpecialInfo )
	{
	    if( sp->Buffer )
	    {
		memcpy( compvals[ gd->GadgetID - compgadid ], sp->Buffer, sp->MaxChars );
		free( sp->Buffer );
	    }
	    free( sp );
	}
	free( gp );
    }
}

void
FreeBoolOpts( boolgads )
    register struct OPTGAD *boolgads;
{
    register struct OPTGAD *gp;
    register struct IntuiText *ip;

    while( gp = boolgads )
    {
	boolgads = boolgads->next;

	if( ip = gp->gad.GadgetText )
	{
	    if( ip->IText ) free( ip->IText );
	    free( ip );
	}
	free( gp );
    }
}

#ifdef	TESTING
/*
 * Put a 3-D motif border around the gadget.  String gadgets or those
 * which do not have highlighting are rendered down.  Boolean gadgets
 * are rendered in the up position by default.
 */

void SetBorder( gd, val )
    register struct Gadget *gd;
    int val;
{
    register struct Border *bp;
    register short *sp;
    register int i;
    int borders = 6;

    /* Allocate two border structures one for up image and one for down
     * image, plus vector arrays for the border lines.
     */

    if( gd->GadgetType == STRGADGET )
	borders = 12;

    if( ( bp = malloc( ( ( sizeof( struct Border ) * 2 ) +
	    ( sizeof( short ) * borders ) ) * 2 ) ) == NULL )
    {
	return;
    }

    /* Remove any special rendering flags to avoid confusing intuition
     */

    gd->Flags &= ~(GADGHIGHBITS|GADGIMAGE|GRELWIDTH|
		    GRELHEIGHT|GRELRIGHT|GRELBOTTOM);

    sp = (short *)(bp + 4);
    if( val == 0 || val == 2 ||
	gd->GadgetType == STRGADGET || ( gd->GadgetType == BOOLGADGET &&
		( gd->Flags & GADGHIGHBITS ) == GADGHNONE ) )
    {
	/* For a string gadget, we expand the border beyond the area where
	 * the text will be entered.
	 */

	sp[0] = -1;
	sp[1] = gd->Height - 1;
	sp[2] = -1;
	sp[3] = -1;
	sp[4] = gd->Width + 1;
	sp[5] = -1;

	sp[6] = gd->Width + 3;
	sp[7] = -2;
	sp[8] = gd->Width + 3;
	sp[9] = gd->Height + 1;
	sp[10] = -2;
	sp[11] = gd->Height + 1;

	sp[12] = -2;
	sp[13] = gd->Height;
	sp[14] = -2;
	sp[15] = -2;
	sp[16] = gd->Width + 2;
	sp[17] = -2;
	sp[18] = gd->Width + 2;
	sp[19] = gd->Height;
	sp[20] = -2;
	sp[21] = gd->Height;

	for( i = 0; i < 3; ++i )
	{
	    bp[ i ].LeftEdge = bp[ i ].TopEdge = -1;
	    if( val == 2 )
		bp[ i ].FrontPen = ( i == 0 || i == 1 ) ? 2 : 1;
	    else
		bp[ i ].FrontPen = ( i == 0 || i == 1 ) ? 1 : 2;

	    /* Have to use JAM2 so that the old colors disappear. */
	    bp[ i ].BackPen = 0;
	    bp[ i ].DrawMode = JAM2;
	    bp[ i ].Count = ( i == 0 || i == 1 ) ? 3 : 5;
	    bp[ i ].XY = &sp[ i*6 ];
	    bp[ i ].NextBorder = ( i == 2 ) ? NULL : &bp[ i + 1 ];
	}

	/* Set the up image */
	gd->GadgetRender = (APTR) bp;

	/* Same image for select image */
	gd->SelectRender = (APTR) bp;

	gd->LeftEdge++;
	gd->TopEdge++;
	gd->Flags |= GADGHCOMP;
    }
    else
    {
	/* Create the border vector values for up and left side, and
	 * also the lower and right side.
	 */

	sp[0] = 0;
	sp[1] = gd->Height-1;
	sp[2] = 0;
	sp[3] = 0;
	sp[4] = gd->Width-1;
	sp[5] = 0;

	sp[6] = gd->Width-1;
	sp[7] = 0;
	sp[8] = gd->Width-1;
	sp[9] = gd->Height-1;
	sp[10] = 0;
	sp[11] = gd->Height-1;

	/* We are creating 4 sets of borders, the two sides of the
	 * rectangle share the border vectors with the opposite image,
	 * but specify different colors.
	 */

	for( i = 0; i < 4; ++i )
	{
	    bp[ i ].TopEdge = bp[ i ].LeftEdge = 0;

	    /* A GADGHNONE is always down */

	    if( val != 3 && gd->GadgetType == BOOLGADGET &&
		( gd->Flags & GADGHIGHBITS ) != GADGHNONE )
	    {
		bp[ i ].FrontPen =
		    ( i == 1 || i == 2 ) ? 2 : 1;
	    }
	    else
	    {
		bp[ i ].FrontPen =
		    ( i == 1 || i == 3 ) ? 1 : 2;
	    }

	    /* Have to use JAM2 so that the old colors disappear. */
	    bp[ i ].BackPen = 0;
	    bp[ i ].DrawMode = JAM2;
	    bp[ i ].Count = 3;
	    bp[ i ].XY = &sp[ 6 * ((i &1) != 0) ];
	    bp[ i ].NextBorder =
		( i == 1 || i == 3 ) ? NULL : &bp[ i + 1 ];
	}

	/* bp[0] and bp[1] two pieces for the up image */
	gd->GadgetRender = (APTR) bp;

	/* bp[2] and bp[3] two pieces for the down image */
	gd->SelectRender = (APTR) (bp + 2);
	gd->Flags |= GADGHIMAGE;
    }
}
#endif
