static struct NewScreen NewScreenStructure = {
	0,0,
	640,200,
	2,
	0,2,
	HIRES,
	CUSTOMSCREEN,
	NULL,
	"NetHack WorkBench V3.1",
	NULL,
	NULL
};

#define NEWSCREENSTRUCTURE NewScreenStructure

static USHORT Palette[] = {
	0x0AAA,
	0x0002,
	0x0FFF,
	0x016A
#define PaletteColorCount 4
};

#define PALETTE Palette

static UBYTE UNDOBUFFER[300];

static SHORT BorderVectors1[] = {
	0,0,
	566,0
};
static struct Border Border1 = {
	-1,11,
	2,0,JAM1,
	2,
	BorderVectors1,
	NULL
};

static struct IntuiText IText1 = {
	3,0,JAM2,
	6,1,
	NULL,
	"  ",
	NULL
};

static struct Gadget Message = {
	NULL,
	5,12,
	-26,10,
	GADGHBOX+GADGHIMAGE+GRELWIDTH,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Border1,
	NULL,
	&IText1,
	NULL,
	NULL,
	-1,
	NULL
};

static struct PropInfo ScrollSInfo = {
	AUTOKNOB+FREEHORIZ,
	-1,-1,
	-1,-1,
};

static struct Image Image1 = {
	0,0,
	531,2,
	0,
	NULL,
	0x0000,0x0000,
	NULL
};

static struct Gadget Scroll = {
	&Message,
	5,-7,
	-26,6,
	GRELBOTTOM+GRELWIDTH,
	RELVERIFY+GADGIMMEDIATE+FOLLOWMOUSE+BOTTOMBORDER,
	PROPGADGET,
	(APTR)&Image1,
	NULL,
	NULL,
	NULL,
	(APTR)&ScrollSInfo,
	GADSCROLL,
	NULL
};

#define GadgetList1 Scroll

static struct IntuiText IText2 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Rename",
	NULL
};

static struct MenuItem MenuItem4 = {
	NULL,
	0,24,
	103,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText2,
	NULL,
	'N',
	NULL,
	MENUNULL
};

static struct IntuiText IText3 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Discard",
	NULL
};

static struct MenuItem MenuItem3 = {
	&MenuItem4,
	0,16,
	103,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText3,
	NULL,
	'D',
	NULL,
	MENUNULL
};

static struct IntuiText IText4 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Copy Info",
	NULL
};

static struct MenuItem MenuItem2 = {
	&MenuItem3,
	0,8,
	103,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText4,
	NULL,
	'C',
	NULL,
	MENUNULL
};

static struct IntuiText IText5 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Info",
	NULL
};

static struct MenuItem MenuItem1 = {
	&MenuItem2,
	0,0,
	103,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText5,
	NULL,
	'I',
	NULL,
	MENUNULL
};

static struct Menu Menu2 = {
	NULL,
	63,0,
	35,0,
	MENUENABLED,
	"Game",
	&MenuItem1
};

static struct IntuiText IText6 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Quit",
	NULL
};

static struct MenuItem MenuItem10 = {
	NULL,
	0,40,
	166,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText6,
	NULL,
	'Q',
	NULL,
	MENUNULL
};

static struct IntuiText IText7 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Edit Configuration",
	NULL
};

static struct MenuItem MenuItem9 = {
	&MenuItem10,
	0,32,
	166,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText7,
	NULL,
	'E',
	NULL,
	MENUNULL
};

static struct IntuiText IText8 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Recover",
	NULL
};

static struct MenuItem MenuItem8 = {
	&MenuItem9,
	0,24,
	166,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText8,
	NULL,
	'R',
	NULL,
	MENUNULL
};

static struct IntuiText IText9 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Top Scores",
	NULL
};

static struct MenuItem MenuItem7 = {
	&MenuItem8,
	0,16,
	166,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText9,
	NULL,
	'S',
	NULL,
	MENUNULL
};

static struct IntuiText IText10 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"About",
	NULL
};

static struct MenuItem MenuItem6 = {
	&MenuItem7,
	0,8,
	166,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText10,
	NULL,
	'A',
	NULL,
	MENUNULL
};

static struct IntuiText IText11 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Help",
	NULL
};

static struct MenuItem MenuItem5 = {
	&MenuItem6,
	0,0,
	166,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText11,
	NULL,
	'H',
	NULL,
	MENUNULL
};

static struct Menu Menu1 = {
	&Menu2,
	0,0,
	56,0,
	MENUENABLED,
	"Project",
	&MenuItem5
};

#define MenuList1 Menu1

static struct NewWindow NewWindowStructure1 = {
	40,15,
	565,148,
	0,2,
	NEWSIZE+MOUSEBUTTONS+MOUSEMOVE+GADGETDOWN+GADGETUP+MENUPICK+CLOSEWINDOW+RAWKEY+DISKINSERTED,
	WINDOWSIZING+WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+SIZEBRIGHT+SIZEBBOTTOM+ACTIVATE+NOCAREREFRESH,
	&Scroll,
	NULL,
	"Select Saved Game or New Game",
	NULL,
	NULL,
	170,80,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Quest_BorderVectors2[] = {
	0,0,
	275,0,
	275,22,
	0,22,
	0,0
};
static struct Border Quest_Border2 = {
	-1,-1,
	3,0,JAM1,
	5,
	Quest_BorderVectors2,
	NULL
};

static struct Gadget Quest_Borders2 = {
	NULL,
	12,16,
	274,21,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Quest_Border2,
	NULL,
	NULL,
	NULL,
	NULL,
	-1,
	NULL
};

static SHORT Quest_BorderVectors3[] = {
	0,0,
	88,0,
	88,12,
	0,12,
	0,0
};
static struct Border Quest_Border3 = {
	-1,-1,
	2,0,JAM1,
	5,
	Quest_BorderVectors3,
	NULL
};

static struct IntuiText Quest_IText12 = {
	3,0,JAM2,
	35,2,
	NULL,
	"No",
	NULL
};

static struct Gadget Quest_No = {
	&Quest_Borders2,
	199,43,
	87,11,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Quest_Border3,
	NULL,
	&Quest_IText12,
	NULL,
	NULL,
	GADQUESTNO,
	NULL
};

static SHORT Quest_BorderVectors4[] = {
	0,0,
	88,0,
	88,12,
	0,12,
	0,0
};
static struct Border Quest_Border4 = {
	-1,-1,
	2,0,JAM1,
	5,
	Quest_BorderVectors4,
	NULL
};

static struct IntuiText Quest_IText13 = {
	3,0,JAM2,
	32,2,
	NULL,
	"Yes",
	NULL
};

static struct Gadget Quest_Yes = {
	&Quest_No,
	12,43,
	87,11,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Quest_Border4,
	NULL,
	&Quest_IText13,
	NULL,
	NULL,
	GADQUESTYES,
	NULL
};

#define Quest_GadgetList2 Quest_Yes

static struct IntuiText Quest_IText14 = {
	1,0,JAM2,
	59,21,
	NULL,
	"Sure you want to QUIT?",
	NULL
};

#define Quest_IntuiTextList2 Quest_IText14

static struct NewWindow Quest_NewWindowStructure2 = {
	174,60,
	298,60,
	0,3,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Quest_Yes,
	NULL,
	"NetHack WorkBench Request",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Options_BorderVectors5[] = {
	0,0,
	72,0,
	72,11,
	0,11,
	0,0
};
static struct Border Options_Border5 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors5,
	NULL
};

static struct IntuiText Options_IText15 = {
	3,0,JAM2,
	15,1,
	NULL,
	"CANCEL",
	NULL
};

static struct Gadget Options_Gadget7 = {
	NULL,
	-96,-15,
	71,10,
	GRELBOTTOM+GRELRIGHT,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Options_Border5,
	NULL,
	&Options_IText15,
	NULL,
	NULL,
	GADOPTCANCEL,
	NULL
};

static SHORT Options_BorderVectors6[] = {
	0,0,
	72,0,
	72,11,
	0,11,
	0,0
};
static struct Border Options_Border6 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors6,
	NULL
};

static struct IntuiText Options_IText16 = {
	3,0,JAM2,
	21,1,
	NULL,
	"OKAY",
	NULL
};

static struct Gadget Options_Gadget6 = {
	&Options_Gadget7,
	13,-15,
	71,10,
	GRELBOTTOM,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Options_Border6,
	NULL,
	&Options_IText16,
	NULL,
	NULL,
	GADOPTOKAY,
	NULL
};

#define Options_GadgetList3 Options_Gadget6

static struct IntuiText Options_IText17 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Cancel",
	NULL
};

static struct MenuItem Options_MenuItem12 = {
	NULL,
	0,8,
	82,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&Options_IText17,
	NULL,
	'C',
	NULL,
	MENUNULL
};

static struct IntuiText Options_IText18 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Save",
	NULL
};

static struct MenuItem Options_MenuItem11 = {
	&Options_MenuItem12,
	0,0,
	82,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&Options_IText18,
	NULL,
	'S',
	NULL,
	MENUNULL
};

static struct Menu Options_Menu2 = {
	NULL,
	0,0,
	56,0,
	MENUENABLED,
	"Project",
	&Options_MenuItem11
};

#define Options_MenuList3 Options_Menu2

static struct NewWindow Options_NewWindowStructure3 = {
	0,21,
	640,156,
	0,1,
	SIZEVERIFY+NEWSIZE+MOUSEBUTTONS+GADGETDOWN+GADGETUP+MENUPICK+CLOSEWINDOW+RAWKEY+DISKINSERTED+DISKREMOVED+ACTIVEWINDOW+VANILLAKEY,
	WINDOWSIZING+WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Options_Gadget6,
	NULL,
	"Edit Options",
	NULL,
	NULL,
	200,50,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Conf_BorderVectors7[] = {
	0,0,
	52,0,
	52,11,
	0,11,
	0,0
};
static struct Border Conf_Border7 = {
	-1,-1,
	2,0,JAM1,
	5,
	Conf_BorderVectors7,
	NULL
};

static struct IntuiText Conf_IText19 = {
	1,0,JAM2,
	10,2,
	NULL,
	"Save",
	NULL
};

static struct Gadget Conf_Gadget15 = {
	NULL,
	73,85,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Conf_Border7,
	NULL,
	&Conf_IText19,
	NULL,
	NULL,
	GADCONFSAVE,
	NULL
};

static SHORT Conf_BorderVectors8[] = {
	0,0,
	52,0,
	52,11,
	0,11,
	0,0
};
static struct Border Conf_Border8 = {
	-1,-1,
	2,0,JAM1,
	5,
	Conf_BorderVectors8,
	NULL
};

static struct IntuiText Conf_IText20 = {
	1,0,JAM2,
	10,2,
	NULL,
	"Load",
	NULL
};

static struct Gadget Conf_Gadget14 = {
	&Conf_Gadget15,
	9,85,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Conf_Border8,
	NULL,
	&Conf_IText20,
	NULL,
	NULL,
	GADCONFLOAD,
	NULL
};

static UBYTE Conf_Conf_ConfigNameSIBuff[50] =
	"Nethack:NetHack.cnf";
static struct StringInfo Conf_Conf_ConfigNameSInfo = {
	Conf_Conf_ConfigNameSIBuff,
	UNDOBUFFER,
	0,
	50,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Conf_BorderVectors9[] = {
	0,0,
	242,0,
	242,10,
	0,10,
	0,0
};
static struct Border Conf_Border9 = {
	-1,-1,
	1,0,JAM1,
	5,
	Conf_BorderVectors9,
	NULL
};

static struct Gadget Conf_ConfigName = {
	&Conf_Gadget14,
	151,100,
	241,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Conf_Border9,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_ConfigNameSInfo,
	GADCONFNAME,
	NULL
};

static UBYTE Conf_Conf_StrSaveSIBuff[200];
static struct StringInfo Conf_Conf_StrSaveSInfo = {
	Conf_Conf_StrSaveSIBuff,
	UNDOBUFFER,
	0,
	200,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Conf_BorderVectors10[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border10 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors10,
	NULL
};

static struct Gadget Conf_StrSave = {
	&Conf_ConfigName,
	81,70,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border10,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrSaveSInfo,
	GADSTRSAVE,
	NULL
};

static UBYTE Conf_Conf_StrLevelsSIBuff[200];
static struct StringInfo Conf_Conf_StrLevelsSInfo = {
	Conf_Conf_StrLevelsSIBuff,
	UNDOBUFFER,
	0,
	200,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Conf_BorderVectors11[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border11 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors11,
	NULL
};

static struct Gadget Conf_StrLevels = {
	&Conf_StrSave,
	81,56,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border11,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrLevelsSInfo,
	GADSTRLEVELS,
	NULL
};

static UBYTE Conf_Conf_StrPathSIBuff[200];
static struct StringInfo Conf_Conf_StrPathSInfo = {
	Conf_Conf_StrPathSIBuff,
	UNDOBUFFER,
	0,
	200,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Conf_BorderVectors12[] = {
	0,0,
	311,0,
	311,10,
	0,10,
	0,0
};
static struct Border Conf_Border12 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors12,
	NULL
};

static struct Gadget Conf_StrPath = {
	&Conf_StrLevels,
	81,14,
	310,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border12,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrPathSInfo,
	GADSTRPATH,
	NULL
};

static UBYTE Conf_Conf_StrPensSIBuff[200];
static struct StringInfo Conf_Conf_StrPensSInfo = {
	Conf_Conf_StrPensSIBuff,
	UNDOBUFFER,
	0,
	200,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Conf_BorderVectors13[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border13 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors13,
	NULL
};

static struct Gadget Conf_StrPens = {
	&Conf_StrPath,
	81,42,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border13,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrPensSInfo,
	GADSTRPENS,
	NULL
};

static UBYTE Conf_Conf_StrHackdirSIBuff[200];
static struct StringInfo Conf_Conf_StrHackdirSInfo = {
	Conf_Conf_StrHackdirSIBuff,
	UNDOBUFFER,
	0,
	200,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Conf_BorderVectors14[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border14 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors14,
	NULL
};

static struct Gadget Conf_StrHackdir = {
	&Conf_StrPens,
	81,28,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border14,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrHackdirSInfo,
	GADSTRHACKDIR,
	NULL
};

#define Conf_GadgetList4 Conf_StrHackdir

static struct IntuiText Conf_IText26 = {
	1,0,JAM2,
	10,101,
	NULL,
	"Config File Name:",
	NULL
};

static struct IntuiText Conf_IText25 = {
	3,0,JAM2,
	7,72,
	NULL,
	"Save Dir:",
	&Conf_IText26
};

static struct IntuiText Conf_IText24 = {
	3,0,JAM2,
	23,58,
	NULL,
	"Levels:",
	&Conf_IText25
};

static struct IntuiText Conf_IText23 = {
	3,0,JAM2,
	39,44,
	NULL,
	"Pens:",
	&Conf_IText24
};

static struct IntuiText Conf_IText22 = {
	3,0,JAM2,
	15,30,
	NULL,
	"Hackdir:",
	&Conf_IText23
};

static struct IntuiText Conf_IText21 = {
	3,0,JAM2,
	39,16,
	NULL,
	"Path:",
	&Conf_IText22
};

#define Conf_IntuiTextList4 Conf_IText21

static struct NewWindow Conf_NewWindowStructure4 = {
	126,60,
	402,114,
	0,1,
	GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Conf_StrHackdir,
	NULL,
	"Edit Game Configuration",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Str_BorderVectors15[] = {
	0,0,
	57,0,
	57,11,
	0,11,
	0,0
};
static struct Border Str_Border15 = {
	-1,-1,
	3,0,JAM1,
	5,
	Str_BorderVectors15,
	NULL
};

static struct IntuiText Str_IText27 = {
	3,0,JAM2,
	4,2,
	NULL,
	"Cancel",
	NULL
};

static struct Gadget Str_Gadget17 = {
	NULL,
	9,15,
	56,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Str_Border15,
	NULL,
	&Str_IText27,
	NULL,
	NULL,
	GADSTRCANCEL,
	NULL
};

static UBYTE Str_Str_StringSIBuff[100];
static struct StringInfo Str_Str_StringSInfo = {
	Str_Str_StringSIBuff,
	UNDOBUFFER,
	0,
	100,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Str_BorderVectors16[] = {
	0,0,
	439,0,
	439,11,
	0,11,
	0,0
};
static struct Border Str_Border16 = {
	-1,-1,
	3,0,JAM1,
	5,
	Str_BorderVectors16,
	NULL
};

static struct Gadget Str_String = {
	&Str_Gadget17,
	77,15,
	438,10,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Str_Border16,
	NULL,
	NULL,
	NULL,
	(APTR)&Str_Str_StringSInfo,
	-1,
	NULL
};

#define Str_GadgetList5 Str_String

static struct NewWindow Str_NewWindowStructure5 = {
	55,60,
	526,31,
	0,1,
	GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Str_String,
	NULL,
	"String Requester",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Info_BorderVectors17[] = {
	0,0,
	72,0,
	72,10,
	0,10,
	0,0
};
static struct Border Info_Border17 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors17,
	NULL
};

static struct IntuiText Info_IText28 = {
	3,0,JAM2,
	24,1,
	NULL,
	"Use",
	NULL
};

static struct Gadget Info_Gadget29 = {
	NULL,
	247,78,
	71,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border17,
	NULL,
	&Info_IText28,
	NULL,
	NULL,
	GADUSEINFO,
	NULL
};

static SHORT Info_BorderVectors18[] = {
	0,0,
	72,0,
	72,10,
	0,10,
	0,0
};
static struct Border Info_Border18 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors18,
	NULL
};

static struct IntuiText Info_IText29 = {
	3,0,JAM2,
	20,1,
	NULL,
	"Quit",
	NULL
};

static struct Gadget Info_Gadget28 = {
	&Info_Gadget29,
	474,78,
	71,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border18,
	NULL,
	&Info_IText29,
	NULL,
	NULL,
	GADQUITINFO,
	NULL
};

static SHORT Info_BorderVectors19[] = {
	0,0,
	72,0,
	72,10,
	0,10,
	0,0
};
static struct Border Info_Border19 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors19,
	NULL
};

static struct IntuiText Info_IText30 = {
	3,0,JAM2,
	21,1,
	NULL,
	"Save",
	NULL
};

static struct Gadget Info_Gadget27 = {
	&Info_Gadget28,
	11,78,
	71,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border19,
	NULL,
	&Info_IText30,
	NULL,
	NULL,
	GADSAVEINFO,
	NULL
};

static SHORT Info_BorderVectors20[] = {
	0,0,
	60,0,
	60,11,
	0,11,
	0,0
};
static struct Border Info_Border20 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors20,
	NULL
};

static struct IntuiText Info_IText31 = {
	3,0,JAM2,
	18,1,
	NULL,
	"Del",
	NULL
};

static struct Gadget Info_Gadget26 = {
	&Info_Gadget27,
	78,46,
	59,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border20,
	NULL,
	&Info_IText31,
	NULL,
	NULL,
	GADDELTOOL,
	NULL
};

static SHORT Info_BorderVectors21[] = {
	0,0,
	59,0,
	59,11,
	0,11,
	0,0
};
static struct Border Info_Border21 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors21,
	NULL
};

static struct IntuiText Info_IText32 = {
	3,0,JAM2,
	18,1,
	NULL,
	"Add",
	NULL
};

static struct Gadget Info_Gadget25 = {
	&Info_Gadget26,
	12,46,
	58,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border21,
	NULL,
	&Info_IText32,
	NULL,
	NULL,
	GADADDTOOL,
	NULL
};

static struct IntuiText Info_IText33 = {
	3,0,JAM2,
	6,1,
	NULL,
	"Edit Game Options",
	NULL
};

static struct Gadget Info_EditOpts = {
	&Info_Gadget25,
	397,47,
	148,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Info_IText33,
	NULL,
	NULL,
	GADEDITOPTS,
	NULL
};

static SHORT Info_BorderVectors22[] = {
	0,0,
	14,0,
	14,7,
	0,7,
	0,0
};
static struct Border Info_Border22 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors22,
	NULL
};

static struct Gadget Info_ToolDown = {
	&Info_EditOpts,
	97,68,
	13,6,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border22,
	NULL,
	NULL,
	NULL,
	NULL,
	GADTOOLDOWN,
	NULL
};

static SHORT Info_BorderVectors23[] = {
	0,0,
	14,0,
	14,7,
	0,7,
	0,0
};
static struct Border Info_Border23 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors23,
	NULL
};

static struct Gadget Info_ToolUp = {
	&Info_ToolDown,
	97,61,
	13,6,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border23,
	NULL,
	NULL,
	NULL,
	NULL,
	GADTOOLUP,
	NULL
};

static UBYTE Info_Info_ToolTypesSIBuff[200];
static struct StringInfo Info_Info_ToolTypesSInfo = {
	Info_Info_ToolTypesSIBuff,
	UNDOBUFFER,
	0,
	200,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Info_BorderVectors24[] = {
	0,0,
	430,0,
	430,10,
	0,10,
	0,0
};
static struct Border Info_Border24 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors24,
	NULL
};

static struct IntuiText Info_IText34 = {
	3,0,JAM2,
	-110,1,
	NULL,
	"Tool Types:",
	NULL
};

static struct Gadget Info_ToolTypes = {
	&Info_ToolUp,
	116,63,
	429,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Info_Border24,
	NULL,
	&Info_IText34,
	NULL,
	(APTR)&Info_Info_ToolTypesSInfo,
	GADTOOLTYPES,
	NULL
};

static SHORT Info_BorderVectors25[] = {
	0,0,
	144,0,
	144,12,
	0,12,
	0,0
};
static struct Border Info_Border25 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors25,
	NULL
};

static struct IntuiText Info_IText35 = {
	3,0,JAM2,
	-85,2,
	NULL,
	"Character:",
	NULL
};

static struct Gadget Info_Class = {
	&Info_ToolTypes,
	402,15,
	143,11,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Info_Border25,
	NULL,
	&Info_IText35,
	NULL,
	NULL,
	-1,
	NULL
};

static UBYTE Info_Info_PlayerSIBuff[100];
static struct StringInfo Info_Info_PlayerSInfo = {
	Info_Info_PlayerSIBuff,
	NULL,
	0,
	100,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Info_BorderVectors26[] = {
	0,0,
	197,0,
	197,12,
	0,12,
	0,0
};
static struct Border Info_Border26 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors26,
	NULL
};

static struct IntuiText Info_IText36 = {
	3,0,JAM2,
	-101,2,
	NULL,
	"Player Name:",
	NULL
};

static struct Gadget Info_Player = {
	&Info_Class,
	110,15,
	196,11,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Info_Border26,
	NULL,
	&Info_IText36,
	NULL,
	(APTR)&Info_Info_PlayerSInfo,
	GADPLNAME,
	NULL
};

static UBYTE Info_Info_CommentSIBuff[100];
static struct StringInfo Info_Info_CommentSInfo = {
	Info_Info_CommentSIBuff,
	UNDOBUFFER,
	0,
	100,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Info_BorderVectors27[] = {
	0,0,
	466,0,
	466,11,
	0,11,
	0,0
};
static struct Border Info_Border27 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors27,
	NULL
};

static struct IntuiText Info_IText37 = {
	3,0,JAM2,
	-70,1,
	NULL,
	"Comment:",
	NULL
};

static struct Gadget Info_Comment = {
	&Info_Player,
	80,31,
	465,10,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Info_Border27,
	NULL,
	&Info_IText37,
	NULL,
	(APTR)&Info_Info_CommentSInfo,
	-1,
	NULL
};

#define Info_GadgetList6 Info_Comment

static struct IntuiText Info_IText38 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Wizard",
	NULL
};

static struct MenuItem Info_MenuItem25 = {
	NULL,
	0,96,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	4095,
	(APTR)&Info_IText38,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText39 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Valkyrie",
	NULL
};

static struct MenuItem Info_MenuItem24 = {
	&Info_MenuItem25,
	0,88,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	6143,
	(APTR)&Info_IText39,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText40 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Tourist",
	NULL
};

static struct MenuItem Info_MenuItem23 = {
	&Info_MenuItem24,
	0,80,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	7167,
	(APTR)&Info_IText40,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText41 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Samurai",
	NULL
};

static struct MenuItem Info_MenuItem22 = {
	&Info_MenuItem23,
	0,72,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	7679,
	(APTR)&Info_IText41,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText42 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Rogue",
	NULL
};

static struct MenuItem Info_MenuItem21 = {
	&Info_MenuItem22,
	0,64,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	7935,
	(APTR)&Info_IText42,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText43 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Priest",
	NULL
};

static struct MenuItem Info_MenuItem20 = {
	&Info_MenuItem21,
	0,56,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8063,
	(APTR)&Info_IText43,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText44 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Knight",
	NULL
};

static struct MenuItem Info_MenuItem19 = {
	&Info_MenuItem20,
	0,48,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8127,
	(APTR)&Info_IText44,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText45 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Healer",
	NULL
};

static struct MenuItem Info_MenuItem18 = {
	&Info_MenuItem19,
	0,40,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8159,
	(APTR)&Info_IText45,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText46 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Elf",
	NULL
};

static struct MenuItem Info_MenuItem17 = {
	&Info_MenuItem18,
	0,32,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8175,
	(APTR)&Info_IText46,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText47 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Caveman",
	NULL
};

static struct MenuItem Info_MenuItem16 = {
	&Info_MenuItem17,
	0,24,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8183,
	(APTR)&Info_IText47,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText48 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Barbarian",
	NULL
};

static struct MenuItem Info_MenuItem15 = {
	&Info_MenuItem16,
	0,16,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8187,
	(APTR)&Info_IText48,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText49 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Archeologist",
	NULL
};

static struct MenuItem Info_MenuItem14 = {
	&Info_MenuItem15,
	0,8,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8189,
	(APTR)&Info_IText49,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText50 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Random",
	NULL
};

static struct MenuItem Info_MenuItem13 = {
	&Info_MenuItem14,
	0,0,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP+CHECKED,
	8190,
	(APTR)&Info_IText50,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct Menu Info_Menu3 = {
	NULL,
	0,0,
	70,0,
	MENUENABLED,
	"Character",
	&Info_MenuItem13
};

#define Info_MenuList6 Info_Menu3

static struct NewWindow Info_NewWindowStructure6 = {
	41,51,
	556,93,
	0,1,
	GADGETUP+MENUPICK+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Info_Comment,
	NULL,
	"Game Information",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText Help1_IText51 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Help1_Gadget30 = {
	NULL,
	12,34,
	47,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Help1_IText51,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define Help1_GadgetList7 Help1_Gadget30

static struct IntuiText Help1_IText53 = {
	3,0,JAM2,
	10,22,
	NULL,
	"to start a new game or to resume a saved game.",
	NULL
};

static struct IntuiText Help1_IText52 = {
	3,0,JAM2,
	9,13,
	NULL,
	"Click on NewGame Gadget or a Saved Game twice",
	&Help1_IText53
};

#define Help1_IntuiTextList7 Help1_IText52

static struct NewWindow Help1_NewWindowStructure7 = {
	134,60,
	385,51,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help1_Gadget30,
	NULL,
	"Help for Game Selection",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText Help2_IText54 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Help2_Gadget31 = {
	NULL,
	17,42,
	47,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Help2_IText54,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define Help2_GadgetList8 Help2_Gadget31

static struct IntuiText Help2_IText57 = {
	3,0,JAM2,
	15,31,
	NULL,
	"resume the saved game.",
	NULL
};

static struct IntuiText Help2_IText56 = {
	3,0,JAM2,
	15,22,
	NULL,
	"selected game, or double click on a game to",
	&Help2_IText57
};

static struct IntuiText Help2_IText55 = {
	3,0,JAM2,
	15,13,
	NULL,
	"Use Menu button to select operation on the",
	&Help2_IText56
};

#define Help2_IntuiTextList8 Help2_IText55

static struct NewWindow Help2_NewWindowStructure8 = {
	139,60,
	372,58,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help2_Gadget31,
	NULL,
	"Help for Game Manipulation",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText About_IText58 = {
	1,0,JAM2,
	40,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget About_Gadget32 = {
	NULL,
	163,68,
	109,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&About_IText58,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define About_GadgetList9 About_Gadget32

static struct IntuiText About_IText67 = {
	2,0,JAM2,
	10,56,
	NULL,
	"1992 see NetHack license for details and limitations!",
	NULL
};

static struct IntuiText About_IText66 = {
	2,0,JAM2,
	20,47,
	NULL,
	"HackWB is copyright Gregg Wonderly and Ken Lorber,",
	&About_IText67
};

static struct IntuiText About_IText65 = {
	3,0,JAM2,
	8,31,
	NULL,
	"finished by Gregg...",
	&About_IText66
};

static struct IntuiText About_IText64 = {
	3,0,JAM2,
	135,22,
	NULL,
	"The programming was started by Ken and",
	&About_IText65
};

static struct IntuiText About_IText63 = {
	3,0,JAM2,
	120,22,
	NULL,
	".",
	&About_IText64
};

static struct IntuiText About_IText62 = {
	2,0,JAM2,
	8,22,
	NULL,
	"Gregg Wonderly",
	&About_IText63
};

static struct IntuiText About_IText61 = {
	3,0,JAM2,
	396,13,
	NULL,
	"and",
	&About_IText62
};

static struct IntuiText About_IText60 = {
	2,0,JAM2,
	310,13,
	NULL,
	"Ken Lorber",
	&About_IText61
};

static struct IntuiText About_IText59 = {
	3,0,JAM2,
	8,13,
	NULL,
	"The NetHack WorkBench was designed by",
	&About_IText60
};

#define About_IntuiTextList9 About_IText59

static struct NewWindow About_NewWindowStructure9 = {
	89,60,
	447,83,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&About_Gadget32,
	NULL,
	"About the NetHack WorkBench",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Help3_BorderVectors28[] = {
	0,0,
	489,0
};
static struct Border Help3_Border28 = {
	2,169,
	1,0,JAM1,
	2,
	Help3_BorderVectors28,
	NULL
};

static struct Gadget Help3_Gadget35 = {
	NULL,
	0,0,
	1,1,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Help3_Border28,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static struct IntuiText Help3_IText68 = {
	1,0,JAM2,
	8,2,
	NULL,
	"BKWD",
	NULL
};

static struct Gadget Help3_Gadget34 = {
	&Help3_Gadget35,
	434,173,
	47,10,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&Help3_IText68,
	NULL,
	NULL,
	GADHELPBKWD,
	NULL
};

static struct IntuiText Help3_IText69 = {
	1,0,JAM2,
	8,2,
	NULL,
	"FRWD",
	NULL
};

static struct Gadget Help3_Gadget33 = {
	&Help3_Gadget34,
	12,173,
	47,10,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&Help3_IText69,
	NULL,
	NULL,
	GADHELPFRWD,
	NULL
};

#define Help3_GadgetList10 Help3_Gadget33

static struct NewWindow Help3_NewWindowStructure10 = {
	75,9,
	494,189,
	0,1,
	GADGETDOWN+GADGETUP+CLOSEWINDOW+VANILLAKEY+INTUITICKS,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help3_Gadget33,
	NULL,
	"Help for Nethack WorkBench V3.1",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Rst_BorderVectors29[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Rst_Border29 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors29,
	NULL
};

static struct IntuiText Rst_IText70 = {
	1,0,JAM1,
	8,1,
	NULL,
	"Cancel",
	NULL
};

static struct Gadget Rst_RestCancel = {
	NULL,
	230,42,
	57,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Rst_Border29,
	NULL,
	&Rst_IText70,
	NULL,
	NULL,
	GADRESTCAN,
	NULL
};

static SHORT Rst_BorderVectors30[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Rst_Border30 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors30,
	NULL
};

static struct IntuiText Rst_IText71 = {
	1,0,JAM1,
	12,1,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Rst_RestOkay = {
	&Rst_RestCancel,
	10,42,
	57,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Rst_Border30,
	NULL,
	&Rst_IText71,
	NULL,
	NULL,
	GADRESTOKAY,
	NULL
};

static UBYTE Rst_Rst_RestOldSIBuff[300];
static struct StringInfo Rst_Rst_RestOldSInfo = {
	Rst_Rst_RestOldSIBuff,
	UNDOBUFFER,
	0,
	300,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Rst_BorderVectors31[] = {
	0,0,
	187,0,
	187,10,
	0,10,
	0,0
};
static struct Border Rst_Border31 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors31,
	NULL
};

static struct Gadget Rst_RestOld = {
	&Rst_RestOkay,
	101,28,
	186,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Rst_Border31,
	NULL,
	NULL,
	NULL,
	(APTR)&Rst_Rst_RestOldSInfo,
	GADRESTOLD,
	NULL
};

static UBYTE Rst_Rst_RestDirSIBuff[300];
static struct StringInfo Rst_Rst_RestDirSInfo = {
	Rst_Rst_RestDirSIBuff,
	UNDOBUFFER,
	0,
	300,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Rst_BorderVectors32[] = {
	0,0,
	187,0,
	187,10,
	0,10,
	0,0
};
static struct Border Rst_Border32 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors32,
	NULL
};

static struct Gadget Rst_RestDir = {
	&Rst_RestOld,
	101,15,
	186,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Rst_Border32,
	NULL,
	NULL,
	NULL,
	(APTR)&Rst_Rst_RestDirSInfo,
	GADRESTDIR,
	NULL
};

#define Rst_GadgetList11 Rst_RestDir

static struct IntuiText Rst_IText73 = {
	1,0,JAM1,
	18,29,
	NULL,
	"Old File:",
	NULL
};

static struct IntuiText Rst_IText72 = {
	1,0,JAM1,
	14,15,
	NULL,
	"Directory:",
	&Rst_IText73
};

#define Rst_IntuiTextList11 Rst_IText72

static struct NewWindow Rst_NewWindowStructure11 = {
	177,60,
	295,57,
	0,2,
	GADGETDOWN+GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Rst_RestDir,
	NULL,
	"Recover Parameters",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};


/* end of PowerWindows source generation */
