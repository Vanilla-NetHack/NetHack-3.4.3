/*    SCCS Id: @(#)wbdata.c     3.1    93/01/08
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993.  */
/* NetHack may be freely redistributed.  See license for details. */

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *IconBase;
struct DiskfontBase *DiskfontBase;
struct Screen *scrn;
struct Window *oldwin;
char *classes = "ABCEHKPRSTVW";
struct TmpRas tmpras;

DEFAULTS  defgame =
{
    PL_RANDOM,
    NULL, NULL,
};

OPTIONS curopts[] =
{
    { 0, 1, "checkpoint", NULL, GADOCHKPOINT, },
    { 0, 1, "color", NULL, GADOCOLOR, },
    { 0, 1, "confirm", NULL, GADOCONFIRM, },
    { 0, 1, "disclose", NULL, GADODISCLOSE, },
    { 0, 0, "female", NULL, GADOFEMALE, },
    { 0, 1, "fixinv", NULL, GADOFIXINV, },
    { 0, 0, "flush", NULL, GADOFLUSH, },
    { 0, 1, "help", NULL, GADOHELP, },
    { 0, 1, "hilite_pet", NULL, GADOHILITEPET, },
    { 0, 0, "ignintr", NULL, GADOIGNINTR, },
    { 0, 1, "legacy", NULL, GADOLEGACY, },
    { 0, 0, "lit_corridor", NULL, GADOLITCORRIDOR, },
    { 0, 1, "news", NULL, GADONEWS, },
    { 0, 0, "numberpad", NULL, GADONUMBERPAD, },
    { 0, 1, "null", NULL, GADONULL, },
    { 0, 1, "pickup", NULL, GADOPICKUP, },
    { 0, 0, "rest_on_space", NULL, GADORESTONSPACE, },
    { 0, 1, "safepet", NULL, GADOSAFEPET, },
    { 0, 0, "showexp", NULL, GADOSHOWEXP, },
    { 0, 0, "showscore", NULL, GADOSHOWSCORE, },
    { 0, 1, "silent", NULL, GADOSILENT, },
    { 0, 1, "sortpack", NULL, GADOSORTPACK, },
    { 0, 1, "sound", NULL, GADOSOUND, },
    { 0, 0, "standout", NULL, GADOSTANDOUT, },
    { 0, 0, "time", NULL, GADOTIME, },
    { 0, 1, "tombstone", NULL, GADOTOMBSTONE, },
    { 0, 1, "verbose", NULL, GADOVERBOSE, },
    { 0, 0, "asksave", NULL, GADOASKSAVE, },
    { 0, 0, "packorder", "", GADOPACKORDER, },
    { 0, 0, "dogname", "", GADODOGNAME, },
    { 0, 0, "catname", "", GADOCATNAME, },
    { 0, 0, "fruit", "", GADOFRUIT, },
    { 0, 0, "objects", "", GADOOBJECTS, },
    { 0, 0, NULL, NULL, -1 },
};

char *players[ ] =
{
    "Random",
    "Archeologist",
    "Barbarian",
    "CaveMan",
    "Elf",
    "Healer",
    "Knight",
    "Priest",
    "Rogue",
    "Samurai",
    "Tourist",
    "Valkyrie",
    "Wizard",
    NULL,
};

char *options[ NUMIDX + 1 ] =
{
    "Nethack:",				/* PATH_IDX */
    "",					/* OPTIONS_IDX */
    "Nethack:",				/* HACKDIR_IDX */
    "Nethack:levels",			/* LEVELS_IDX */
    "Nethack:save",			/* SAVE_IDX */
    "AAA,FFF,620,B08,181,C06,23E,D00",	/* PENS_IDX */

    NULL,				/* Terminating option */
};

USHORT __chip up_renderdata[] = {
/* Plane 0 */
   0xfff0,
   0x8700,
   0x8f80,
   0xbfe0,
   0x8000,

/* Plane 1 */
   0x0008,
   0x0708,
   0x0f88,
   0x3fe8,
   0x7ff8,

};

struct Image up_renderimage = {
   0, 0,
   13, 5, 2,
   up_renderdata,
   3,0,
   NULL,
};

USHORT __chip up_selectdata[] = {
/* Plane 0 */
   0x0008, 0x78f8, 0x7078, 0x4018, 0xfff8,
/* Plane 1 */
   0xfff0, 0xf8f0, 0xf070, 0xc010, 0x0000,
};

struct Image up_selectimage = {
   0, 0,
   13, 5, 2,
   up_selectdata,
   3,0,
   NULL,
};

USHORT __chip down_renderdata[] = {
/* Plane 0 */
   0xfff0,
   0xbfe0,
   0x8f80,
   0x8700,
   0x8000,

/* Plane 1 */
   0x0008,
   0x3fe8,
   0x0f88,
   0x0708,
   0x7ff8,

};

struct Image down_renderimage = {
   0, 0,
   13, 5, 2,
   down_renderdata,
   3,0,
   NULL,
};

USHORT __chip down_selectdata[] = {
/* Plane 0 */
   0x0008, 0x4018, 0x7078, 0x78f8, 0x7ff8,
/* Plane 1 */
   0xfff0, 0xc010, 0xf070, 0xf8f0, 0x8000,
};

struct Image down_selectimage = {
   0, 0,
   13, 5, 2,
   down_selectdata,
   3,0,
   NULL,
};

USHORT __chip leftimg[] =
{
    0x0380, 0x0000,
    0x0f80, 0x0000,
    0x3fff, 0xff00,
    0xffff, 0xff00,
    0x3fff, 0xff00,
    0x0f80, 0x0000,
    0x0380, 0x0000,
};

USHORT __chip rightimg[] =
{
    0x0001, 0xc000,
    0x0001, 0xf000,
    0xffff, 0xfc00,
    0xffff, 0xff00,
    0xffff, 0xfc00,
    0x0001, 0xf000,
    0x0001, 0xc000,
};

struct Image leftarrow =
{
    0, 0,
    24, 7, 1,
    leftimg,
    2, 0,
    NULL,
};

struct Image rightarrow =
{
    0, 0,
    24, 7, 1,
    rightimg,
    2, 0,
    NULL,
};

struct Image dnleftarrow =
{
    0, 0,
    24, 7, 1,
    leftimg,
    1, 0,
    NULL,
};

struct Image dnrightarrow =
{
    0, 0,
    24, 7, 1,
    rightimg,
    1, 0,
    NULL,
};

struct MsgPort
    *dosport;

PLANEPTR
    tmprasp;

GPTR
    lastgaddown,	/* Last game gadget user selected */
    globgptr,
    windowgads,		/* What is currently attached to the window */
    gamehead,		/* Pointer to active games */
    gameavail;		/* Pointer to available allocations */

int
    wbopen = 0,		/* Is workbench open? */
    shutdown = 0,	/* Close down workbench during game option */
    errup = 0,		/* Error message on line */
    cols = 0,		/* Total columns */
    vcols = 0,		/* Columns visible */
    scol = -1,		/* Starting column */
    height,		/* Height of window */
    width,		/* Width of window */
    curcol = 0,		/* Offset to first display column */
    active_count;	/* Number of games active */

char
    defgname[100] = "WBDefaults";

flag
    quit=0;		/* time to cleanup */

struct Window *win;	/* the window we create */
