/*  SCCS Id: @(#)amirip.c   3.2 93/01/08
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland 1991, 1992, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include <exec/types.h>
#include <exec/io.h>
#include <exec/alerts.h>
#include <exec/devices.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <libraries/dosextens.h>
#include <ctype.h>
#include <string.h>
#include "winami.h"

#undef  NULL
#define NULL    0

#ifdef AZTEC_C
# include <functions.h>
#else
# include <proto/dos.h>
# include <proto/exec.h>
# include <proto/console.h>
# include <proto/diskfont.h>

/* terrible kludge */
/* this is why prototypes should have ONLY types in them! */
# undef red
# undef green
# undef blue
# undef index
# include <proto/graphics.h>

# include <proto/intuition.h>
#endif /* AZTEC_C */

static void grass ( int x , int y );
static void strand ( int x , int y , int dir );
static void tomb_text ( char *p );
void draw_moon(int phase);

extern char *killed_by_prefix[];
struct Window *ripwin;

#define STONE_LINE_LEN 18   /* # chars that fit on one line
			     * (note 1 ' ' border)
			     */
#define MOONSIZE    30

static struct stabstuff
{
    short x1,y0;
}
stab[]=
{
    {0,0}, {0,1}, {0,2}, {1,3}, {2,4},
    {3,5}, {5,6}, {7,7}, {9,8}, {11,9}
};

static struct
{
    int x, y;	/* center of moon */
}
moon[]=
{
/* 0-7, left to right, 4 is full and just right of center
0-3 hollow to left, 5-7 hollow to right (DONT GO OFF THE SCREEN EDGE!) */
    { 50, 70, },
    { 120, 60, },
    { 190, 50, },
    { 260, 40, },
    { 330, 30, },
    { 410, 40, },
    { 480, 50, },
    { 550, 60, },
};

unsigned short dirtpat[ 7 ][ 3 ][ 8 ]=
{
{
 {0xb8a0, 0xc124, 0xa60f, 0x7894, 0x1152, 0x0ec1, 0x14c0, 0xa921, },
 {0x4611, 0x365b, 0x5030, 0x0460, 0x44a0, 0xd106, 0x0131, 0x4282, },
 {0xb9ee, 0xc9a4, 0xafcf, 0xfb9f, 0xbb5f, 0x2ef9, 0xfece, 0xbd7d, },
},

{
 {0x1258, 0x1015, 0xd430, 0x0488, 0x1402, 0x1040, 0x22e3, 0x8ce8, },
 {0x00a4, 0x818a, 0x2a45, 0x6255, 0x49a8, 0xe69a, 0x9118, 0x1215, },
 {0xff5b, 0x7e75, 0xd5ba, 0x9daa, 0xb657, 0x1965, 0x6ee7, 0xedea, },
},

{
 {0x9958, 0x0164, 0x80c8, 0xa660, 0x0412, 0x0025, 0x22ab, 0x2512, },
 {0x64a4, 0xb292, 0x5525, 0x489d, 0x73c0, 0x7e8a, 0x0514, 0xd2ad, },
 {0x9b5b, 0x4d6d, 0xaada, 0xb762, 0x8c3f, 0x8175, 0xfaeb, 0x2d52, },
},

{
 {0x8f41, 0xca1e, 0x29c2, 0xa4c0, 0x5481, 0x94d8, 0x9702, 0x0914, },
 {0x608c, 0x05c0, 0x4425, 0x1936, 0x2a3e, 0x4203, 0x4064, 0x54c0, },
 {0x9f73, 0xfa3f, 0xbbda, 0xe6c9, 0xd5c1, 0xbdfc, 0xbf9b, 0xab3f, },
},

{
 {0x4000, 0xd52b, 0x1010, 0x5008, 0x40c1, 0x4057, 0x014a, 0x606c, },
 {0xa900, 0x2810, 0x0a85, 0x8fc6, 0x3406, 0xbfa0, 0xf020, 0x9d10, },
 {0x56ff, 0xd7ef, 0xf57a, 0x7039, 0xcbf9, 0x405f, 0x0fdf, 0x62ef, },
},

{
 {0x8368, 0x0480, 0x900e, 0xf41f, 0x2e24, 0xfa03, 0x0397, 0x895c, },
 {0x5814, 0x1022, 0x4ca0, 0x0300, 0x0042, 0x0078, 0xf048, 0x6683, },
 {0xa7eb, 0xefdd, 0xb35f, 0xfcff, 0xffbd, 0xff87, 0x0fb7, 0x997c, },
},

{
 {0x4228, 0x0050, 0xa016, 0x42a3, 0x341c, 0x46a2, 0x23d3, 0x4001, },
 {0xb515, 0x6383, 0x13c8, 0x8d5c, 0x0822, 0x1149, 0x4400, 0x8728, },
 {0x4aea, 0x9c7c, 0xec37, 0x72a3, 0xf7dd, 0xeeb6, 0xbbff, 0x78d7, },
},
};

static USHORT stonepat[] =
{
    0x8242,
    0x2421,
    0x1888,
    0x4112,
    0x2444,
    0x8218,
    0x4181,
    0x1824,
};

static USHORT moundpat[] =
{
    0x5235,
    0xd7c6,
    0x1298,
    0x34a7,
    0x2736,
    0x2c54,
    0xdc93,
    0xc551,
};

#define DEATH_LINE  10
#define YEAR_LINE   15

static int horizon;
static struct RastPort *rp;
static unsigned char tomb_line;

extern struct DisplayDesc *amiIDisplay;
extern struct Screen *HackScreen;
extern int havelace;

#undef  BLACK
#undef  WHITE
#undef  BROWN
#undef  GREEN

#define BLACK   0
#define WHITE   1
#define BROWN   2
#define GREY    3
#define GREEN   4
#define DKGRN   5

static unsigned short zeropalette[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
static unsigned short toppalette[] = {
    0x0000, 0x0EEF, 0x0730, 0x0778, 0x0082, 0x0573, 0x0000, 0x0000,
};

#define AREA_SIZE   200
static WORD areabuffer[ AREA_SIZE ];

static struct NewWindow newwin =
{
    0,0,640,200,1,0,
    MOUSEBUTTONS|VANILLAKEY|NOCAREREFRESH,
    BORDERLESS|ACTIVATE|SMART_REFRESH,
    NULL,NULL,(UBYTE*)NULL,NULL,NULL,-1,-1,-1,-1,CUSTOMSCREEN
};

int wh; /* was local in outrip, but needed for SCALE macro */

#define WIN_DEPTH ripwin->RPort->BitMap->Depth
/*TODO: should use real algorithm to get circles */
#define SCALE(x) ((wh<300)?(x):((x)*2))
#define MOONSX  MOONSIZE
#define MOONSY  ((SCALE(MOONSIZE)/2)-4)

void
draw_moon(int phase)
{
    int x;
    int off_c_x=MOONSX+10;	/* moon center x in srp */
    int off_c_y=MOONSY;		/* moon center y in srp */
    int blit_urhx=10;		/* upper right hand corner for blit */
    int blit_urhy=0;
    int blit_width=MOONSX*2;	/* sizes for blit */
    int blit_height=MOONSY*2+1;	/* +1 - hmm (rounding? 0 base?)*/

    /* set up scratch rastport */
    struct BitMap sbm;
    struct RastPort srp;
    struct AreaInfo ainfo;
    WORD abuf[100];
    struct TmpRas atr;
    PLANEPTR ap = NULL;

#define	XSZ		8
#define	YSZ		8

    /*TODO: sizes here are bigger than needed */
    InitBitMap(&sbm,WIN_DEPTH,MOONSIZE*XSZ,MOONSIZE*YSZ);
    for(x=0;x<WIN_DEPTH;x++){
	sbm.Planes[x]=(PLANEPTR)AllocRaster(MOONSIZE*XSZ,MOONSIZE*YSZ);
	if(!sbm.Planes[x])goto free;
    }
    InitRastPort(&srp);
    srp.BitMap=&sbm;
    memset( abuf, 0, sizeof( abuf ) );		/* Must be zeroed */
    InitArea(&ainfo,abuf,sizeof(abuf)/5);	/* 5 bytes per vertex */
    srp.AreaInfo= &ainfo;
    ap=AllocRaster(320,200);
    if(!ap)goto free;
#ifdef AZTEC_C
    /*
     * Aztec, in their infinite wisdom, require a char * as the
     * second argument.
     */
    InitTmpRas(&atr, (char *)ap, RASSIZE(320,200));
#else
    InitTmpRas(&atr,ap,RASSIZE(320,200));
#endif
    srp.TmpRas = &atr;
    SetAfPt(rp,(UWORD *)NULL,0);
    SetRast(&srp,BLACK);

    switch(phase){
    case 0: /* new moon - no image */
	break;
    case 1: /* waxing crescent */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	SetAPen(&srp,BLACK);
	AreaEllipse(&srp,off_c_x+10,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	break;

    case 2: /* 1st quarter */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	SetAPen(&srp,BLACK);
	RectFill(&srp,off_c_x,0,MOONSIZE*4-1,MOONSIZE*2-1);
	break;
    case 3: /* gibbous */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	SetAPen(&srp,BLACK);
	RectFill(&srp,off_c_x,0,MOONSIZE*4-1,MOONSIZE*2-1);
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX/2,MOONSY);
	AreaEnd(&srp);
	break;
    case 4: /* full */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	break;
    case 5: /* gibbous */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	SetAPen(&srp,BLACK);
	RectFill(&srp,0,0,off_c_x,MOONSIZE*2-1);
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX/2,MOONSY);
	AreaEnd(&srp);
	break;
    case 6: /* last quarter */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	SetAPen(&srp,BLACK);
	RectFill(&srp,0,0,off_c_x,MOONSIZE*4-1+10);
	break;
    case 7: /* waning crescent */
	SetAPen(&srp,WHITE);
	AreaEllipse(&srp,off_c_x,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	SetAPen(&srp,BLACK);
	AreaEllipse(&srp,off_c_x-10,off_c_y,MOONSX,MOONSY);
	AreaEnd(&srp);
	break;
    }
    /*move rastport into window (so that there are no stars "in" the moon)*/
	/* draw black circle in window rport */
    SetAPen(rp,BLACK);
    AreaEllipse( rp, moon[phase].x,moon[phase].y,MOONSX,MOONSY);
    AreaEnd(rp);

    if(phase != 0){
	/*move scratch to window: set all white, ignore black (in src)*/
	ClipBlit(&srp,blit_urhx,blit_urhy,
	  rp,moon[phase].x-(blit_width/2),moon[phase].y-(blit_height/2),
	  blit_width,blit_height,0xe0);
    }

free:
    for(x=0;x<WIN_DEPTH;x++)
    {
	if(sbm.Planes[x])
	    FreeRaster(sbm.Planes[x],MOONSIZE*XSZ,MOONSIZE*YSZ);
    }
    if(ap)FreeRaster(ap,320,200);
}
#undef WIN_DEPTH

void
outrip( how, tmpwin )
int how;
winid tmpwin;
{
    int done, rtxth;
    struct IntuiMessage *imsg;
    struct AreaInfo areaInfo;
    PLANEPTR planeptr;
    struct TmpRas tmpRas;
    int i;
    register char *dpx;
    char buf[ 200 ];
    register int x;
    int line, phase, c, offset, tw, ww;

    /* Use the users display size */
    newwin.Height = amiIDisplay->ypix - newwin.TopEdge;
    newwin.Width = amiIDisplay->xpix;
    newwin.Screen = HackScreen;

    ripwin = OpenWindow( &newwin );
    if( !ripwin ){
	return;
    }

    rp= ripwin->RPort;
    wh = ripwin->Height;
    ww = ripwin->Width;
    if( !( planeptr = AllocRaster( ww, wh ) ) )
	return;

#ifdef AZTEC_C
    InitTmpRas( &tmpRas, (char *) planeptr, RASSIZE( ww, wh ) );
#else
    InitTmpRas( &tmpRas, planeptr, RASSIZE( ww, wh ) );
#endif
    rp->TmpRas = &tmpRas;

    for( i = 0; i < AREA_SIZE; ++i )
	areabuffer[ i ] = 0;
    InitArea( &areaInfo, areabuffer, (AREA_SIZE*2)/5);
    rp->AreaInfo = &areaInfo;

    LoadRGB4( &HackScreen->ViewPort, zeropalette, 8L );

    horizon=ripwin->Height*2/3;

    /* sky */
    SetDrMd(rp,JAM1);
    SetAPen( rp, BLACK );
    RectFill(rp,0,0,ripwin->Width,horizon);

    /* ground */
    SetDrMd( rp, JAM2 );
    SetAfPt( rp, dirtpat[random()%7], -3 );
    SetAPen( rp, 255 );
    SetBPen( rp, 0 );
    RectFill( rp, 0, horizon+1, ripwin->Width, ripwin->Height );
    SetAfPt( rp, (UWORD *)NULL, 0 );

    /* stars */
    SetAPen(rp,WHITE);
    for(c=d(30,40);c;c--)
	WritePixel(rp,rn2(ripwin->Width-1),rn2(horizon));

    /* moon (NB destroys area fill pattern) */
    phase = phase_of_the_moon() % 8;
    draw_moon(phase);

    /* grass */
    SetAPen(rp,GREEN);
    for( x = 5; x < ripwin->Width-5; x+=5+rn2(8))
	grass(x, horizon+5 );
    for( x = 5; x < ripwin->Width-5; x+=5+rn2(10))
	grass(x, horizon+10 );
    for( x = 5; x < ripwin->Width-5; x+=5+rn2(15))
	grass(x, horizon+10 );
    for( x = 5; x < ripwin->Width-5; x+=5+rn2(20))
	grass(x, horizon+10 );

    /* fence - horizontal, then vertical, each with a moonlit side */
    SetAPen(rp,GREY);
    Move(rp,0,horizon-SCALE(20));
    Draw(rp,ripwin->Width,horizon-SCALE(20));
    Move(rp,0,horizon+30);
    Draw(rp,ripwin->Width,horizon+30);

    offset=(phase<4)?-1:1;
    for(x=30;x<ripwin->Width;x+=50)
    {
	Move(rp,x,horizon-SCALE(25));
	Draw(rp,x,horizon+35);
	Move(rp,x-offset,horizon-SCALE(25));
	Draw(rp,x-offset,horizon+35);
	Move(rp,x-(2*offset),horizon-SCALE(25));
	Draw(rp,x-(2*offset),horizon+35);
    }


    if(phase)SetAPen(rp,WHITE); /* no vertical white if no moon */
    Move(rp,0,horizon-SCALE(20)-1);
    Draw(rp,ripwin->Width,horizon-SCALE(20)-1);
    Move(rp,0,horizon+29);
    Draw(rp,ripwin->Width,horizon+29);

    if(phase!=0 && phase!=4){
	SetAPen(rp,WHITE);
    } else {
	SetAPen(rp,GREY);   /* no hor. white if no or full moon */
    }
    for(x=30;x<ripwin->Width;x+=50)
    {
	Move(rp,x+offset,horizon-SCALE(25));
	Draw(rp,x+offset,horizon+35);
	Move(rp,x+(2*offset),horizon-SCALE(25));
	Draw(rp,x+(2*offset),horizon+35);
    }

    /* Put together death description */
    switch (killer_format) {
    default:
	impossible("bad killer format?");
    case KILLED_BY_AN:
	Strcpy(buf, killed_by_prefix[how]);
	Strcat(buf, an(killer));
	break;
    case KILLED_BY:
	Strcpy(buf, killed_by_prefix[how]);
	Strcat(buf, killer);
	break;
    case NO_KILLER_PREFIX:
	Strcpy(buf, killer);
	break;
    }

    tw = TextLength(rp,buf,STONE_LINE_LEN) + 40;

    {
	char *p=buf;
	int x, tmp;
	for(x=STONE_LINE_LEN;x;x--)*p++='W';
	*p='\0';
	tmp = TextLength(rp,buf,STONE_LINE_LEN) + 40;
	tw = max( tw, tmp);
    }

    SetAPen( rp, phase ? BLACK : GREY );
    SetBPen( rp, phase ? WHITE : BLACK );
    SetDrMd( rp, JAM2 );
    SetAfPt( rp, stonepat, 3 );

    /* There are 5 lines of text on the stone. */
    rtxth = ripwin->RPort->TxHeight * 5;

    /* Do shadow ellipse on stone */
    if( phase < 4 )
	AreaEllipse(rp,ripwin->Width/2-3,horizon-rtxth,tw/2+3,tw/6);
    else if( phase > 4 )
	AreaEllipse(rp,ripwin->Width/2+3,horizon-rtxth,tw/2+3,tw/6);
    else
	AreaEllipse(rp,ripwin->Width/2,horizon-rtxth-2,tw/2+3,tw/6);
    AreaEnd( rp );

    /* Top ellipse on stone */
    SetAPen( rp, BLACK );
    SetBPen( rp, GREY );
    AreaEllipse(rp,ripwin->Width/2,horizon-rtxth,tw/2,tw/6);
    AreaEnd( rp );

    /* Body of stone */
    RectFill(rp,ripwin->Width/2-tw/2,horizon-rtxth,ripwin->Width/2+tw/2,
      horizon-rtxth+15+(rp->TxHeight+1)*8);

    SetAPen( rp, phase ? BROWN : GREY );
    SetBPen( rp, phase ? GREY : BROWN );
    SetAfPt( rp, stonepat, 3 );

#if 0
    AreaMove( rp, ripwin->Width/2-tw/2, horizon-rtxth+15+(rp->TxHeight+1)*8 );
    AreaDraw( rp, ripwin->Width/2-tw/4, horizon-rtxth+((ripwin->Height > 200)
      ? 55 : 35)+(rp->TxHeight+1)*8 );
    AreaDraw( rp, ripwin->Width/2+tw, horizon-rtxth+((ripwin->Height > 200)
      ? 55 : 35)+(rp->TxHeight+1)*8 );
    AreaDraw( rp, ripwin->Width/2+tw/2, horizon-rtxth+15+(rp->TxHeight+1)*8 );
    AreaEnd( rp );

    SetAfPt( rp, stonepat, 3 );

#endif
    /* Draw shadow on correct side of stone */
    SetAPen( rp, phase ? BLACK : GREY );
    SetBPen( rp, phase ? WHITE : BLACK );
    if( phase < 4 )
    {
	AreaMove( rp, ripwin->Width/2-tw/2-6, horizon-rtxth );
	AreaDraw( rp, ripwin->Width/2-tw/2-6,
	  horizon-rtxth+15+(rp->TxHeight+1)*8-4 );
	AreaDraw( rp, ripwin->Width/2-tw/2-1,
	  horizon-rtxth+15+(rp->TxHeight+1)*8 );
	AreaDraw( rp, ripwin->Width/2-tw/2-1, horizon-rtxth );
    }
    else if( phase > 4 )
    {
	AreaMove( rp, ripwin->Width/2+tw/2+6, horizon-rtxth );
	AreaDraw( rp, ripwin->Width/2+tw/2+6,
	  horizon-rtxth+15+(rp->TxHeight+1)*8-4 );
	AreaDraw( rp, ripwin->Width/2+tw/2+1,
	  horizon-rtxth+15+(rp->TxHeight+1)*8 );
	AreaDraw( rp, ripwin->Width/2+tw/2+1, horizon-rtxth );
    }
    AreaEnd( rp );

    SetAfPt( rp, (UWORD *)NULL, 0 );
    SetDrPt( rp, ~0 );

    tomb_line=0;
    SetBPen(rp,GREY);
    SetDrMd(rp,JAM1);
    tomb_text("REST");
    tomb_text("IN");
    tomb_text("PEACE");

    /* Put name on stone */
    Sprintf(buf, "%s", plname);
    buf[STONE_LINE_LEN] = 0;
    tomb_text(buf);

    /* Put $ on stone */
    Sprintf(buf, "%ld Au", u.ugold);
    buf[STONE_LINE_LEN] = 0; /* It could be a *lot* of gold :-) */
    tomb_text(buf);

    /* Put together death description */
    switch (killer_format) {
    default:
	impossible("bad killer format?");
    case KILLED_BY_AN:
	Strcpy(buf, killed_by_prefix[how]);
	Strcat(buf, an(killer));
	break;
    case KILLED_BY:
	Strcpy(buf, killed_by_prefix[how]);
	Strcat(buf, killer);
	break;
    case NO_KILLER_PREFIX:
	Strcpy(buf, killer);
	break;
    }

    /* Put death type on stone */
    for (line=DEATH_LINE, dpx = buf; line<YEAR_LINE; line++)
    {
	register int i,i0;
	char tmpchar;

	if ( (i0=strlen(dpx)) > STONE_LINE_LEN)
	{
	    for(i=STONE_LINE_LEN;((i0 > STONE_LINE_LEN) && i); i--)
	    {
		if(dpx[i] == ' ')
		    i0 = i;
	    }
	    if(!i)
		i0 = STONE_LINE_LEN;
	}

	tmpchar = dpx[i0];
	dpx[i0] = 0;
	tomb_text(dpx);

	if (tmpchar != ' ')
	{
	    dpx[i0] = tmpchar;
	    dpx= &dpx[i0];
	}
	else
	{
	    dpx= &dpx[i0+1];
	}
    }

    /* Put year on stone */
    Sprintf(buf, "%4d", getyear());
    tomb_text(buf);

    LoadRGB4( &HackScreen->ViewPort, toppalette, 8L );

    done = 0;
    while( !done )
    {
	WaitPort( ripwin->UserPort );
	while( imsg = (struct IntuiMessage *)GetMsg(ripwin->UserPort) )
	{
	    switch( imsg->Class )
	    {
		case MOUSEBUTTONS:
		case VANILLAKEY:
		    done = 1;
		    break;
	    }
	    ReplyMsg( (struct Message *)imsg );
	}
    }

    if( planeptr ) FreeRaster( planeptr, ww, wh );
    rp->TmpRas = NULL;
    Forbid();
    while( imsg = (struct IntuiMessage *)GetMsg( ripwin->UserPort ) )
	ReplyMsg( (struct Message *)imsg );
    CloseWindow( ripwin );
    ripwin = NULL;
    Permit();
    LoadRGB4( &HackScreen->ViewPort, flags.amii_curmap, 8L );
}

static void grass(x,y)
register int x,y;
{
    register int ct=rn2(5)+3;
    register int c;
    x-=2;
    for(c=ct;c;c--)
    {
	strand(x,y,(c>ct/2)?1:-1);
	x+=2;
    }
}

static void strand(x,y,dir)
   register int x,y;
   register int dir;    /* which way the wind blows :-) */
{
    register int i;
    register struct RastPort *nrp = rp;
    register struct stabstuff *st;
    for(i=rn2(10);i>=0;i--)
    {
	st = &stab[i];
	WritePixel(nrp,x+st->x1*dir,y-st->y0);
    }
}

static void tomb_text(p)
char *p;
{
    char buf[STONE_LINE_LEN*2];
    int l;
    /*int i;*/

    if( !*p )
	return;
    sprintf(buf," %s ",p);
    l=TextLength(rp,buf,strlen(buf));
    ++tomb_line;

    SetAPen(rp,WHITE);
    Move(rp,(ripwin->Width/2)-(l/2)-1, 
      ((tomb_line-6)*(rp->TxHeight+1))+horizon);
    Text(rp,buf,strlen(buf));

    SetAPen(rp,WHITE);
    Move(rp,(ripwin->Width/2)-(l/2)+1,
      ((tomb_line-6)*(rp->TxHeight+1))+horizon);
    Text(rp,buf,strlen(buf));

    SetAPen(rp,WHITE);
    Move(rp,(ripwin->Width/2)-(l/2),
      ((tomb_line-6)*(rp->TxHeight+1))+horizon - 1);
    Text(rp,buf,strlen(buf));

    SetAPen(rp,WHITE);
    Move(rp,(ripwin->Width/2)-(l/2),
      ((tomb_line-6)*(rp->TxHeight+1))+horizon + 1);
    Text(rp,buf,strlen(buf));

    SetAPen(rp,BLACK);
    Move(rp,(ripwin->Width/2)-(l/2),
      ((tomb_line-6)*(rp->TxHeight+1))+horizon);
    Text(rp,buf,strlen(buf));
}
