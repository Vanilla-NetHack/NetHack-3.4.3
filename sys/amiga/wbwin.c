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
	0x0999,
	0x0002,
	0x0FFF,
	0x006B
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
	48,0,
	48,11,
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
	1,0,JAM2,
	8,2,
	NULL,
	"Null",
	NULL
};

static struct Gadget Options_Gadget47 = {
	NULL,
	536,41,
	47,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border5,
	NULL,
	&Options_IText15,
	NULL,
	NULL,
	GADONULL,
	NULL
};

static UBYTE Options_Options_PetTypeSIBuff[70];
static struct StringInfo Options_Options_PetTypeSInfo = {
	Options_Options_PetTypeSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors6[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border6 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors6,
	NULL
};

static struct Gadget Options_PetType = {
	&Options_Gadget47,
	435,134,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border6,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_PetTypeSInfo,
	GADOPETTYPE,
	NULL
};

static UBYTE Options_Options_ScoreSIBuff[70];
static struct StringInfo Options_Options_ScoreSInfo = {
	Options_Options_ScoreSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors7[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border7 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors7,
	NULL
};

static struct Gadget Options_Score = {
	&Options_PetType,
	435,121,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border7,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_ScoreSInfo,
	GADOSCORE,
	NULL
};

static UBYTE Options_Options_PaletteSIBuff[70];
static struct StringInfo Options_Options_PaletteSInfo = {
	Options_Options_PaletteSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors8[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border8 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors8,
	NULL
};

static struct Gadget Options_Palette = {
	&Options_Score,
	435,108,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border8,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_PaletteSInfo,
	GADOPALETTE,
	NULL
};

static UBYTE Options_Options_MsgHistorySIBuff[15];
static struct StringInfo Options_Options_MsgHistorySInfo = {
	Options_Options_MsgHistorySIBuff,
	UNDOBUFFER,
	0,
	15,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors9[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border9 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors9,
	NULL
};

static struct Gadget Options_MsgHistory = {
	&Options_Palette,
	435,95,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER+LONGINT,
	STRGADGET,
	(APTR)&Options_Border9,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_MsgHistorySInfo,
	GADOMSGHISTORY,
	NULL
};

static UBYTE Options_Options_WindowTypeSIBuff[70];
static struct StringInfo Options_Options_WindowTypeSInfo = {
	Options_Options_WindowTypeSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors10[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border10 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors10,
	NULL
};

static struct Gadget Options_WindowType = {
	&Options_MsgHistory,
	435,82,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border10,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_WindowTypeSInfo,
	GADOWINDOWTYPE,
	NULL
};

static UBYTE Options_Options_PickupTypesSIBuff[70];
static struct StringInfo Options_Options_PickupTypesSInfo = {
	Options_Options_PickupTypesSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors11[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border11 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors11,
	NULL
};

static struct Gadget Options_PickupTypes = {
	&Options_WindowType,
	435,69,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border11,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_PickupTypesSInfo,
	GADOPICKUPTYPES,
	NULL
};

static UBYTE Options_Options_NameSIBuff[70];
static struct StringInfo Options_Options_NameSInfo = {
	Options_Options_NameSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors12[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border12 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors12,
	NULL
};

static struct Gadget Options_Name = {
	&Options_PickupTypes,
	97,134,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border12,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_NameSInfo,
	GADONAME,
	NULL
};

static SHORT Options_BorderVectors13[] = {
	0,0,
	91,0,
	91,11,
	0,11,
	0,0
};
static struct Border Options_Border13 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors13,
	NULL
};

static struct IntuiText Options_IText16 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Checkpoint",
	NULL
};

static struct Gadget Options_Gadget39 = {
	&Options_Name,
	309,27,
	90,10,
	SELECTED,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border13,
	NULL,
	&Options_IText16,
	NULL,
	NULL,
	GADOCHKPOINT,
	NULL
};

static SHORT Options_BorderVectors14[] = {
	0,0,
	91,0,
	91,11,
	0,11,
	0,0
};
static struct Border Options_Border14 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors14,
	NULL
};

static struct IntuiText Options_IText17 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Show Score",
	NULL
};

static struct Gadget Options_Gadget38 = {
	&Options_Gadget39,
	493,55,
	90,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border14,
	NULL,
	&Options_IText17,
	NULL,
	NULL,
	GADOSHOWSCORE,
	NULL
};

static SHORT Options_BorderVectors15[] = {
	0,0,
	128,0,
	128,11,
	0,11,
	0,0
};
static struct Border Options_Border15 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors15,
	NULL
};

static struct IntuiText Options_IText18 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Show Experience",
	NULL
};

static struct Gadget Options_Gadget37 = {
	&Options_Gadget38,
	357,55,
	127,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border15,
	NULL,
	&Options_IText18,
	NULL,
	NULL,
	GADOSHOWEXP,
	NULL
};

static SHORT Options_BorderVectors16[] = {
	0,0,
	105,0,
	105,11,
	0,11,
	0,0
};
static struct Border Options_Border16 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors16,
	NULL
};

static struct IntuiText Options_IText19 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Lit Corridor",
	NULL
};

static struct Gadget Options_Gadget36 = {
	&Options_Gadget37,
	9,13,
	104,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border16,
	NULL,
	&Options_IText19,
	NULL,
	NULL,
	GADOLITCORRIDOR,
	NULL
};

static SHORT Options_BorderVectors17[] = {
	0,0,
	60,0,
	60,11,
	0,11,
	0,0
};
static struct Border Options_Border17 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors17,
	NULL
};

static struct IntuiText Options_IText20 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Legacy",
	NULL
};

static struct Gadget Options_Gadget35 = {
	&Options_Gadget36,
	288,55,
	59,10,
	SELECTED,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border17,
	NULL,
	&Options_IText20,
	NULL,
	NULL,
	GADOLEGACY,
	NULL
};

static SHORT Options_BorderVectors18[] = {
	0,0,
	115,0,
	115,11,
	0,11,
	0,0
};
static struct Border Options_Border18 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors18,
	NULL
};

static struct IntuiText Options_IText21 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Highlight Pet",
	NULL
};

static struct Gadget Options_Gadget34 = {
	&Options_Gadget35,
	9,55,
	114,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border18,
	NULL,
	&Options_IText21,
	NULL,
	NULL,
	GADOHILITEPET,
	NULL
};

static SHORT Options_BorderVectors19[] = {
	0,0,
	56,0,
	56,11,
	0,11,
	0,0
};
static struct Border Options_Border19 = {
	-1,-1,
	1,0,JAM1,
	5,
	Options_BorderVectors19,
	NULL
};

static struct IntuiText Options_IText22 = {
	1,0,JAM2,
	11,2,
	NULL,
	"OKAY",
	NULL
};

static struct Gadget Options_Gadget33 = {
	&Options_Gadget34,
	13,146,
	55,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Options_Border19,
	NULL,
	&Options_IText22,
	NULL,
	NULL,
	GADOOKAY,
	NULL
};

static SHORT Options_BorderVectors20[] = {
	0,0,
	56,0,
	56,11,
	0,11,
	0,0
};
static struct Border Options_Border20 = {
	-1,-1,
	1,0,JAM1,
	5,
	Options_BorderVectors20,
	NULL
};

static struct IntuiText Options_IText23 = {
	1,0,JAM2,
	3,2,
	NULL,
	"CANCEL",
	NULL
};

static struct Gadget Options_Gadget32 = {
	&Options_Gadget33,
	528,147,
	55,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Options_Border20,
	NULL,
	&Options_IText23,
	NULL,
	NULL,
	GADOCANCEL,
	NULL
};

static SHORT Options_BorderVectors21[] = {
	0,0,
	111,0,
	111,11,
	0,11,
	0,0
};
static struct Border Options_Border21 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors21,
	NULL
};

static struct IntuiText Options_IText24 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Ask Save Disk",
	NULL
};

static struct Gadget Options_Gadget31 = {
	&Options_Gadget32,
	473,27,
	110,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border21,
	NULL,
	&Options_IText24,
	NULL,
	NULL,
	GADOASKSAVE,
	NULL
};

static UBYTE Options_Options_ObjectsSIBuff[70];
static struct StringInfo Options_Options_ObjectsSInfo = {
	Options_Options_ObjectsSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors22[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border22 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors22,
	NULL
};

static struct Gadget Options_Objects = {
	&Options_Gadget31,
	97,121,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border22,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_ObjectsSInfo,
	GADOOBJECTS,
	NULL
};

static SHORT Options_BorderVectors23[] = {
	0,0,
	55,0,
	55,11,
	0,11,
	0,0
};
static struct Border Options_Border23 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors23,
	NULL
};

static struct IntuiText Options_IText25 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Female",
	NULL
};

static struct Gadget Options_Gadget29 = {
	&Options_Objects,
	473,13,
	54,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border23,
	NULL,
	&Options_IText25,
	NULL,
	NULL,
	GADOFEMALE,
	NULL
};

static UBYTE Options_Options_FruitSIBuff[70];
static struct StringInfo Options_Options_FruitSInfo = {
	Options_Options_FruitSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors24[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border24 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors24,
	NULL
};

static struct Gadget Options_Fruit = {
	&Options_Gadget29,
	97,108,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border24,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_FruitSInfo,
	GADOFRUIT,
	NULL
};

static UBYTE Options_Options_DogNameSIBuff[70];
static struct StringInfo Options_Options_DogNameSInfo = {
	Options_Options_DogNameSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors25[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border25 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors25,
	NULL
};

static struct Gadget Options_DogName = {
	&Options_Fruit,
	97,95,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border25,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_DogNameSInfo,
	GADODOGNAME,
	NULL
};

static UBYTE Options_Options_CatNameSIBuff[70];
static struct StringInfo Options_Options_CatNameSInfo = {
	Options_Options_CatNameSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors26[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border26 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors26,
	NULL
};

static struct Gadget Options_CatName = {
	&Options_DogName,
	97,82,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border26,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_CatNameSInfo,
	GADOCATNAME,
	NULL
};

static UBYTE Options_Options_PackOrderSIBuff[70];
static struct StringInfo Options_Options_PackOrderSInfo = {
	Options_Options_PackOrderSIBuff,
	UNDOBUFFER,
	0,
	70,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT Options_BorderVectors27[] = {
	0,0,
	149,0,
	149,10,
	0,10,
	0,0
};
static struct Border Options_Border27 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors27,
	NULL
};

static struct Gadget Options_PackOrder = {
	&Options_CatName,
	97,69,
	148,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border27,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_PackOrderSInfo,
	GADOPACKORDER,
	NULL
};

static SHORT Options_BorderVectors28[] = {
	0,0,
	61,0,
	61,11,
	0,11,
	0,0
};
static struct Border Options_Border28 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors28,
	NULL
};

static struct IntuiText Options_IText26 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Verbose",
	NULL
};

static struct Gadget Options_Gadget24 = {
	&Options_PackOrder,
	406,27,
	60,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border28,
	NULL,
	&Options_IText26,
	NULL,
	NULL,
	GADOVERBOSE,
	NULL
};

static SHORT Options_BorderVectors29[] = {
	0,0,
	86,0,
	86,11,
	0,11,
	0,0
};
static struct Border Options_Border29 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors29,
	NULL
};

static struct IntuiText Options_IText27 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Tomb Stone",
	NULL
};

static struct Gadget Options_Gadget23 = {
	&Options_Gadget24,
	380,13,
	85,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border29,
	NULL,
	&Options_IText27,
	NULL,
	NULL,
	GADOTOMBSTONE,
	NULL
};

static SHORT Options_BorderVectors30[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Options_Border30 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors30,
	NULL
};

static struct IntuiText Options_IText28 = {
	1,0,JAM2,
	11,2,
	NULL,
	"Time",
	NULL
};

static struct Gadget Options_Gadget22 = {
	&Options_Gadget23,
	9,27,
	57,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border30,
	NULL,
	&Options_IText28,
	NULL,
	NULL,
	GADOTIME,
	NULL
};

static SHORT Options_BorderVectors31[] = {
	0,0,
	78,0,
	78,11,
	0,11,
	0,0
};
static struct Border Options_Border31 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors31,
	NULL
};

static struct IntuiText Options_IText29 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Stand Out",
	NULL
};

static struct Gadget Options_Gadget21 = {
	&Options_Gadget22,
	452,41,
	77,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border31,
	NULL,
	&Options_IText29,
	NULL,
	NULL,
	GADOSTANDOUT,
	NULL
};

static SHORT Options_BorderVectors32[] = {
	0,0,
	48,0,
	48,11,
	0,11,
	0,0
};
static struct Border Options_Border32 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors32,
	NULL
};

static struct IntuiText Options_IText30 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Sound",
	NULL
};

static struct Gadget Options_Gadget20 = {
	&Options_Gadget21,
	536,13,
	47,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border32,
	NULL,
	&Options_IText30,
	NULL,
	NULL,
	GADOSOUND,
	NULL
};

static SHORT Options_BorderVectors33[] = {
	0,0,
	79,0,
	79,11,
	0,11,
	0,0
};
static struct Border Options_Border33 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors33,
	NULL
};

static struct IntuiText Options_IText31 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Sort Pack",
	NULL
};

static struct Gadget Options_Gadget19 = {
	&Options_Gadget20,
	366,41,
	78,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border33,
	NULL,
	&Options_IText31,
	NULL,
	NULL,
	GADOSORTPACK,
	NULL
};

static SHORT Options_BorderVectors34[] = {
	0,0,
	70,0,
	70,11,
	0,11,
	0,0
};
static struct Border Options_Border34 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors34,
	NULL
};

static struct IntuiText Options_IText32 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Safe Pet",
	NULL
};

static struct Gadget Options_Gadget18 = {
	&Options_Gadget19,
	288,41,
	69,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border34,
	NULL,
	&Options_IText32,
	NULL,
	NULL,
	GADOSAFEPET,
	NULL
};

static SHORT Options_BorderVectors35[] = {
	0,0,
	55,0,
	55,11,
	0,11,
	0,0
};
static struct Border Options_Border35 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors35,
	NULL
};

static struct IntuiText Options_IText33 = {
	1,0,JAM2,
	4,2,
	NULL,
	"Silent",
	NULL
};

static struct Gadget Options_Gadget17 = {
	&Options_Gadget18,
	319,13,
	54,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border35,
	NULL,
	&Options_IText33,
	NULL,
	NULL,
	GADOSILENT,
	NULL
};

static SHORT Options_BorderVectors36[] = {
	0,0,
	112,0,
	112,11,
	0,11,
	0,0
};
static struct Border Options_Border36 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors36,
	NULL
};

static struct IntuiText Options_IText34 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Rest On Space",
	NULL
};

static struct Gadget Options_Gadget16 = {
	&Options_Gadget17,
	201,13,
	111,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border36,
	NULL,
	&Options_IText34,
	NULL,
	NULL,
	GADORESTONSPACE,
	NULL
};

static SHORT Options_BorderVectors37[] = {
	0,0,
	109,0,
	109,11,
	0,11,
	0,0
};
static struct Border Options_Border37 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors37,
	NULL
};

static struct IntuiText Options_IText35 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Auto Pick Up",
	NULL
};

static struct Gadget Options_Gadget15 = {
	&Options_Gadget16,
	9,41,
	108,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border37,
	NULL,
	&Options_IText35,
	NULL,
	NULL,
	GADOPICKUP,
	NULL
};

static SHORT Options_BorderVectors38[] = {
	0,0,
	86,0,
	86,11,
	0,11,
	0,0
};
static struct Border Options_Border38 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors38,
	NULL
};

static struct IntuiText Options_IText36 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Number Pad",
	NULL
};

static struct Gadget Options_Gadget14 = {
	&Options_Gadget15,
	195,41,
	85,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border38,
	NULL,
	&Options_IText36,
	NULL,
	NULL,
	GADONUMBERPAD,
	NULL
};

static SHORT Options_BorderVectors39[] = {
	0,0,
	43,0,
	43,11,
	0,11,
	0,0
};
static struct Border Options_Border39 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors39,
	NULL
};

static struct IntuiText Options_IText37 = {
	1,0,JAM2,
	6,2,
	NULL,
	"News",
	NULL
};

static struct Gadget Options_Gadget13 = {
	&Options_Gadget14,
	73,27,
	42,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border39,
	NULL,
	&Options_IText37,
	NULL,
	NULL,
	GADONEWS,
	NULL
};

static SHORT Options_BorderVectors40[] = {
	0,0,
	62,0,
	62,11,
	0,11,
	0,0
};
static struct Border Options_Border40 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors40,
	NULL
};

static struct IntuiText Options_IText38 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Ignintr",
	NULL
};

static struct Gadget Options_Gadget12 = {
	&Options_Gadget13,
	125,41,
	61,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border40,
	NULL,
	&Options_IText38,
	NULL,
	NULL,
	GADOIGNINTR,
	NULL
};

static SHORT Options_BorderVectors41[] = {
	0,0,
	43,0,
	43,11,
	0,11,
	0,0
};
static struct Border Options_Border41 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors41,
	NULL
};

static struct IntuiText Options_IText39 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Help",
	NULL
};

static struct Gadget Options_Gadget11 = {
	&Options_Gadget12,
	125,27,
	42,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border41,
	NULL,
	&Options_IText39,
	NULL,
	NULL,
	GADOHELP,
	NULL
};

static SHORT Options_BorderVectors42[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Options_Border42 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors42,
	NULL
};

static struct IntuiText Options_IText40 = {
	1,0,JAM2,
	9,2,
	NULL,
	"Flush",
	NULL
};

static struct Gadget Options_Gadget10 = {
	&Options_Gadget11,
	175,27,
	57,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border42,
	NULL,
	&Options_IText40,
	NULL,
	NULL,
	GADOFLUSH,
	NULL
};

static SHORT Options_BorderVectors43[] = {
	0,0,
	62,0,
	62,11,
	0,11,
	0,0
};
static struct Border Options_Border43 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors43,
	NULL
};

static struct IntuiText Options_IText41 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Fixinv",
	NULL
};

static struct Gadget Options_Gadget9 = {
	&Options_Gadget10,
	239,27,
	61,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border43,
	NULL,
	&Options_IText41,
	NULL,
	NULL,
	GADOFIXINV,
	NULL
};

static SHORT Options_BorderVectors44[] = {
	0,0,
	69,0,
	69,11,
	0,11,
	0,0
};
static struct Border Options_Border44 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors44,
	NULL
};

static struct IntuiText Options_IText42 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Disclose",
	NULL
};

static struct Gadget Options_Gadget8 = {
	&Options_Gadget9,
	123,13,
	68,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border44,
	NULL,
	&Options_IText42,
	NULL,
	NULL,
	GADODISCLOSE,
	NULL
};

static SHORT Options_BorderVectors45[] = {
	0,0,
	78,0,
	78,11,
	0,11,
	0,0
};
static struct Border Options_Border45 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors45,
	NULL
};

static struct IntuiText Options_IText43 = {
	1,0,JAM2,
	9,2,
	NULL,
	"Confirm",
	NULL
};

static struct Gadget Options_Gadget7 = {
	&Options_Gadget8,
	202,55,
	77,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border45,
	NULL,
	&Options_IText43,
	NULL,
	NULL,
	GADOCONFIRM,
	NULL
};

static SHORT Options_BorderVectors46[] = {
	0,0,
	63,0,
	63,11,
	0,11,
	0,0
};
static struct Border Options_Border46 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors46,
	NULL
};

static struct IntuiText Options_IText44 = {
	1,0,JAM2,
	11,2,
	NULL,
	"Color",
	NULL
};

static struct Gadget Options_Gadget6 = {
	&Options_Gadget7,
	132,55,
	62,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border46,
	NULL,
	&Options_IText44,
	NULL,
	NULL,
	GADOCOLOR,
	NULL
};

#define Options_GadgetList3 Options_Gadget6

static struct IntuiText Options_IText56 = {
	3,0,JAM2,
	360,136,
	NULL,
	"Pet Type:",
	NULL
};

static struct IntuiText Options_IText55 = {
	3,0,JAM2,
	328,71,
	NULL,
	"Pickup Types:",
	&Options_IText56
};

static struct IntuiText Options_IText54 = {
	3,0,JAM2,
	336,97,
	NULL,
	"Msg History:",
	&Options_IText55
};

static struct IntuiText Options_IText53 = {
	3,0,JAM2,
	336,84,
	NULL,
	"Window Type:",
	&Options_IText54
};

static struct IntuiText Options_IText52 = {
	3,0,JAM2,
	368,110,
	NULL,
	"Palette:",
	&Options_IText53
};

static struct IntuiText Options_IText51 = {
	3,0,JAM2,
	384,123,
	NULL,
	"Score:",
	&Options_IText52
};

static struct IntuiText Options_IText50 = {
	3,0,JAM2,
	55,135,
	NULL,
	"Name:",
	&Options_IText51
};

static struct IntuiText Options_IText49 = {
	3,0,JAM2,
	31,122,
	NULL,
	"Objects:",
	&Options_IText50
};

static struct IntuiText Options_IText48 = {
	3,0,JAM2,
	47,109,
	NULL,
	"Fruit:",
	&Options_IText49
};

static struct IntuiText Options_IText47 = {
	3,0,JAM2,
	23,96,
	NULL,
	"Dog Name:",
	&Options_IText48
};

static struct IntuiText Options_IText46 = {
	3,0,JAM2,
	23,83,
	NULL,
	"Cat Name:",
	&Options_IText47
};

static struct IntuiText Options_IText45 = {
	3,0,JAM2,
	7,70,
	NULL,
	"Pack Order:",
	&Options_IText46
};

#define Options_IntuiTextList3 Options_IText45

static struct NewWindow Options_NewWindowStructure3 = {
	29,39,
	593,161,
	0,1,
	GADGETUP+CLOSEWINDOW+RAWKEY+DISKINSERTED+DISKREMOVED+ACTIVEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Options_Gadget6,
	NULL,
	"Edit Options",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Conf_BorderVectors47[] = {
	0,0,
	52,0,
	52,11,
	0,11,
	0,0
};
static struct Border Conf_Border47 = {
	-1,-1,
	2,0,JAM1,
	5,
	Conf_BorderVectors47,
	NULL
};

static struct IntuiText Conf_IText57 = {
	1,0,JAM2,
	10,2,
	NULL,
	"Save",
	NULL
};

static struct Gadget Conf_Gadget55 = {
	NULL,
	73,85,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Conf_Border47,
	NULL,
	&Conf_IText57,
	NULL,
	NULL,
	GADCONFSAVE,
	NULL
};

static SHORT Conf_BorderVectors48[] = {
	0,0,
	52,0,
	52,11,
	0,11,
	0,0
};
static struct Border Conf_Border48 = {
	-1,-1,
	2,0,JAM1,
	5,
	Conf_BorderVectors48,
	NULL
};

static struct IntuiText Conf_IText58 = {
	1,0,JAM2,
	10,2,
	NULL,
	"Load",
	NULL
};

static struct Gadget Conf_Gadget54 = {
	&Conf_Gadget55,
	9,85,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Conf_Border48,
	NULL,
	&Conf_IText58,
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

static SHORT Conf_BorderVectors49[] = {
	0,0,
	242,0,
	242,10,
	0,10,
	0,0
};
static struct Border Conf_Border49 = {
	-1,-1,
	1,0,JAM1,
	5,
	Conf_BorderVectors49,
	NULL
};

static struct Gadget Conf_ConfigName = {
	&Conf_Gadget54,
	151,100,
	241,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Conf_Border49,
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

static SHORT Conf_BorderVectors50[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border50 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors50,
	NULL
};

static struct Gadget Conf_StrSave = {
	&Conf_ConfigName,
	81,70,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border50,
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

static SHORT Conf_BorderVectors51[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border51 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors51,
	NULL
};

static struct Gadget Conf_StrLevels = {
	&Conf_StrSave,
	81,56,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border51,
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

static SHORT Conf_BorderVectors52[] = {
	0,0,
	311,0,
	311,10,
	0,10,
	0,0
};
static struct Border Conf_Border52 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors52,
	NULL
};

static struct Gadget Conf_StrPath = {
	&Conf_StrLevels,
	81,14,
	310,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border52,
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

static SHORT Conf_BorderVectors53[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border53 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors53,
	NULL
};

static struct Gadget Conf_StrPens = {
	&Conf_StrPath,
	81,42,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border53,
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

static SHORT Conf_BorderVectors54[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border54 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors54,
	NULL
};

static struct Gadget Conf_StrHackdir = {
	&Conf_StrPens,
	81,28,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border54,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrHackdirSInfo,
	GADSTRHACKDIR,
	NULL
};

#define Conf_GadgetList4 Conf_StrHackdir

static struct IntuiText Conf_IText64 = {
	1,0,JAM2,
	10,101,
	NULL,
	"Config File Name:",
	NULL
};

static struct IntuiText Conf_IText63 = {
	3,0,JAM2,
	7,72,
	NULL,
	"Save Dir:",
	&Conf_IText64
};

static struct IntuiText Conf_IText62 = {
	3,0,JAM2,
	23,58,
	NULL,
	"Levels:",
	&Conf_IText63
};

static struct IntuiText Conf_IText61 = {
	3,0,JAM2,
	39,44,
	NULL,
	"Pens:",
	&Conf_IText62
};

static struct IntuiText Conf_IText60 = {
	3,0,JAM2,
	15,30,
	NULL,
	"Hackdir:",
	&Conf_IText61
};

static struct IntuiText Conf_IText59 = {
	3,0,JAM2,
	39,16,
	NULL,
	"Path:",
	&Conf_IText60
};

#define Conf_IntuiTextList4 Conf_IText59

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

static SHORT Str_BorderVectors55[] = {
	0,0,
	57,0,
	57,11,
	0,11,
	0,0
};
static struct Border Str_Border55 = {
	-1,-1,
	3,0,JAM1,
	5,
	Str_BorderVectors55,
	NULL
};

static struct IntuiText Str_IText65 = {
	3,0,JAM2,
	4,2,
	NULL,
	"Cancel",
	NULL
};

static struct Gadget Str_Gadget57 = {
	NULL,
	9,15,
	56,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Str_Border55,
	NULL,
	&Str_IText65,
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

static SHORT Str_BorderVectors56[] = {
	0,0,
	439,0,
	439,11,
	0,11,
	0,0
};
static struct Border Str_Border56 = {
	-1,-1,
	3,0,JAM1,
	5,
	Str_BorderVectors56,
	NULL
};

static struct Gadget Str_String = {
	&Str_Gadget57,
	77,15,
	438,10,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Str_Border56,
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

static SHORT Info_BorderVectors57[] = {
	0,0,
	72,0,
	72,10,
	0,10,
	0,0
};
static struct Border Info_Border57 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors57,
	NULL
};

static struct IntuiText Info_IText66 = {
	3,0,JAM2,
	24,1,
	NULL,
	"Use",
	NULL
};

static struct Gadget Info_Gadget69 = {
	NULL,
	247,78,
	71,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border57,
	NULL,
	&Info_IText66,
	NULL,
	NULL,
	GADUSEINFO,
	NULL
};

static SHORT Info_BorderVectors58[] = {
	0,0,
	72,0,
	72,10,
	0,10,
	0,0
};
static struct Border Info_Border58 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors58,
	NULL
};

static struct IntuiText Info_IText67 = {
	3,0,JAM2,
	20,1,
	NULL,
	"Quit",
	NULL
};

static struct Gadget Info_Gadget68 = {
	&Info_Gadget69,
	474,78,
	71,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border58,
	NULL,
	&Info_IText67,
	NULL,
	NULL,
	GADQUITINFO,
	NULL
};

static SHORT Info_BorderVectors59[] = {
	0,0,
	72,0,
	72,10,
	0,10,
	0,0
};
static struct Border Info_Border59 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors59,
	NULL
};

static struct IntuiText Info_IText68 = {
	3,0,JAM2,
	21,1,
	NULL,
	"Save",
	NULL
};

static struct Gadget Info_Gadget67 = {
	&Info_Gadget68,
	11,78,
	71,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border59,
	NULL,
	&Info_IText68,
	NULL,
	NULL,
	GADSAVEINFO,
	NULL
};

static SHORT Info_BorderVectors60[] = {
	0,0,
	60,0,
	60,11,
	0,11,
	0,0
};
static struct Border Info_Border60 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors60,
	NULL
};

static struct IntuiText Info_IText69 = {
	3,0,JAM2,
	18,1,
	NULL,
	"Del",
	NULL
};

static struct Gadget Info_Gadget66 = {
	&Info_Gadget67,
	78,46,
	59,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border60,
	NULL,
	&Info_IText69,
	NULL,
	NULL,
	GADDELTOOL,
	NULL
};

static SHORT Info_BorderVectors61[] = {
	0,0,
	59,0,
	59,11,
	0,11,
	0,0
};
static struct Border Info_Border61 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors61,
	NULL
};

static struct IntuiText Info_IText70 = {
	3,0,JAM2,
	18,1,
	NULL,
	"Add",
	NULL
};

static struct Gadget Info_Gadget65 = {
	&Info_Gadget66,
	12,46,
	58,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border61,
	NULL,
	&Info_IText70,
	NULL,
	NULL,
	GADADDTOOL,
	NULL
};

static struct IntuiText Info_IText71 = {
	3,0,JAM2,
	6,1,
	NULL,
	"Edit Game Options",
	NULL
};

static struct Gadget Info_EditOpts = {
	&Info_Gadget65,
	397,47,
	148,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Info_IText71,
	NULL,
	NULL,
	GADEDITOPTS,
	NULL
};

static SHORT Info_BorderVectors62[] = {
	0,0,
	14,0,
	14,7,
	0,7,
	0,0
};
static struct Border Info_Border62 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors62,
	NULL
};

static struct Gadget Info_ToolDown = {
	&Info_EditOpts,
	97,68,
	13,6,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border62,
	NULL,
	NULL,
	NULL,
	NULL,
	GADTOOLDOWN,
	NULL
};

static SHORT Info_BorderVectors63[] = {
	0,0,
	14,0,
	14,7,
	0,7,
	0,0
};
static struct Border Info_Border63 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors63,
	NULL
};

static struct Gadget Info_ToolUp = {
	&Info_ToolDown,
	97,61,
	13,6,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border63,
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

static SHORT Info_BorderVectors64[] = {
	0,0,
	430,0,
	430,10,
	0,10,
	0,0
};
static struct Border Info_Border64 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors64,
	NULL
};

static struct IntuiText Info_IText72 = {
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
	(APTR)&Info_Border64,
	NULL,
	&Info_IText72,
	NULL,
	(APTR)&Info_Info_ToolTypesSInfo,
	GADTOOLTYPES,
	NULL
};

static SHORT Info_BorderVectors65[] = {
	0,0,
	144,0,
	144,12,
	0,12,
	0,0
};
static struct Border Info_Border65 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors65,
	NULL
};

static struct IntuiText Info_IText73 = {
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
	(APTR)&Info_Border65,
	NULL,
	&Info_IText73,
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

static SHORT Info_BorderVectors66[] = {
	0,0,
	197,0,
	197,12,
	0,12,
	0,0
};
static struct Border Info_Border66 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors66,
	NULL
};

static struct IntuiText Info_IText74 = {
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
	(APTR)&Info_Border66,
	NULL,
	&Info_IText74,
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

static SHORT Info_BorderVectors67[] = {
	0,0,
	466,0,
	466,11,
	0,11,
	0,0
};
static struct Border Info_Border67 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors67,
	NULL
};

static struct IntuiText Info_IText75 = {
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
	(APTR)&Info_Border67,
	NULL,
	&Info_IText75,
	NULL,
	(APTR)&Info_Info_CommentSInfo,
	-1,
	NULL
};

#define Info_GadgetList6 Info_Comment

static struct IntuiText Info_IText76 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Wizard",
	NULL
};

static struct MenuItem Info_MenuItem23 = {
	NULL,
	0,96,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	4095,
	(APTR)&Info_IText76,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText77 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Valkyrie",
	NULL
};

static struct MenuItem Info_MenuItem22 = {
	&Info_MenuItem23,
	0,88,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	6143,
	(APTR)&Info_IText77,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText78 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Tourist",
	NULL
};

static struct MenuItem Info_MenuItem21 = {
	&Info_MenuItem22,
	0,80,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	7167,
	(APTR)&Info_IText78,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText79 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Samurai",
	NULL
};

static struct MenuItem Info_MenuItem20 = {
	&Info_MenuItem21,
	0,72,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	7679,
	(APTR)&Info_IText79,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText80 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Rogue",
	NULL
};

static struct MenuItem Info_MenuItem19 = {
	&Info_MenuItem20,
	0,64,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	7935,
	(APTR)&Info_IText80,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText81 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Priest",
	NULL
};

static struct MenuItem Info_MenuItem18 = {
	&Info_MenuItem19,
	0,56,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8063,
	(APTR)&Info_IText81,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText82 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Knight",
	NULL
};

static struct MenuItem Info_MenuItem17 = {
	&Info_MenuItem18,
	0,48,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8127,
	(APTR)&Info_IText82,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText83 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Healer",
	NULL
};

static struct MenuItem Info_MenuItem16 = {
	&Info_MenuItem17,
	0,40,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8159,
	(APTR)&Info_IText83,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText84 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Elf",
	NULL
};

static struct MenuItem Info_MenuItem15 = {
	&Info_MenuItem16,
	0,32,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8175,
	(APTR)&Info_IText84,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText85 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Caveman",
	NULL
};

static struct MenuItem Info_MenuItem14 = {
	&Info_MenuItem15,
	0,24,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8183,
	(APTR)&Info_IText85,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText86 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Barbarian",
	NULL
};

static struct MenuItem Info_MenuItem13 = {
	&Info_MenuItem14,
	0,16,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8187,
	(APTR)&Info_IText86,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText87 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Archeologist",
	NULL
};

static struct MenuItem Info_MenuItem12 = {
	&Info_MenuItem13,
	0,8,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP,
	8189,
	(APTR)&Info_IText87,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText Info_IText88 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Random",
	NULL
};

static struct MenuItem Info_MenuItem11 = {
	&Info_MenuItem12,
	0,0,
	103,8,
	CHECKIT+ITEMTEXT+ITEMENABLED+HIGHCOMP+CHECKED,
	8190,
	(APTR)&Info_IText88,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct Menu Info_Menu2 = {
	NULL,
	0,0,
	70,0,
	MENUENABLED,
	"Character",
	&Info_MenuItem11
};

#define Info_MenuList6 Info_Menu2

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

static struct IntuiText Help1_IText89 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Help1_Gadget70 = {
	NULL,
	12,34,
	47,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Help1_IText89,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define Help1_GadgetList7 Help1_Gadget70

static struct IntuiText Help1_IText91 = {
	3,0,JAM2,
	10,22,
	NULL,
	"to start a new game or to resume a saved game.",
	NULL
};

static struct IntuiText Help1_IText90 = {
	3,0,JAM2,
	9,13,
	NULL,
	"Click on NewGame Gadget or a Saved Game twice",
	&Help1_IText91
};

#define Help1_IntuiTextList7 Help1_IText90

static struct NewWindow Help1_NewWindowStructure7 = {
	134,60,
	385,51,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help1_Gadget70,
	NULL,
	"Help for Game Selection",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText Help2_IText92 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Help2_Gadget71 = {
	NULL,
	17,42,
	47,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Help2_IText92,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define Help2_GadgetList8 Help2_Gadget71

static struct IntuiText Help2_IText95 = {
	3,0,JAM2,
	15,31,
	NULL,
	"resume the saved game.",
	NULL
};

static struct IntuiText Help2_IText94 = {
	3,0,JAM2,
	15,22,
	NULL,
	"selected game, or double click on a game to",
	&Help2_IText95
};

static struct IntuiText Help2_IText93 = {
	3,0,JAM2,
	15,13,
	NULL,
	"Use Menu button to select operation on the",
	&Help2_IText94
};

#define Help2_IntuiTextList8 Help2_IText93

static struct NewWindow Help2_NewWindowStructure8 = {
	139,60,
	372,58,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help2_Gadget71,
	NULL,
	"Help for Game Manipulation",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText About_IText96 = {
	1,0,JAM2,
	40,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget About_Gadget72 = {
	NULL,
	163,68,
	109,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&About_IText96,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define About_GadgetList9 About_Gadget72

static struct IntuiText About_IText105 = {
	2,0,JAM2,
	10,56,
	NULL,
	"1992 see NetHack license for details and limitations!",
	NULL
};

static struct IntuiText About_IText104 = {
	2,0,JAM2,
	20,47,
	NULL,
	"HackWB is copyright Gregg Wonderly and Ken Lorber,",
	&About_IText105
};

static struct IntuiText About_IText103 = {
	3,0,JAM2,
	8,31,
	NULL,
	"finished by Gregg...",
	&About_IText104
};

static struct IntuiText About_IText102 = {
	3,0,JAM2,
	135,22,
	NULL,
	"The programming was started by Ken and",
	&About_IText103
};

static struct IntuiText About_IText101 = {
	3,0,JAM2,
	120,22,
	NULL,
	".",
	&About_IText102
};

static struct IntuiText About_IText100 = {
	2,0,JAM2,
	8,22,
	NULL,
	"Gregg Wonderly",
	&About_IText101
};

static struct IntuiText About_IText99 = {
	3,0,JAM2,
	396,13,
	NULL,
	"and",
	&About_IText100
};

static struct IntuiText About_IText98 = {
	2,0,JAM2,
	310,13,
	NULL,
	"Ken Lorber",
	&About_IText99
};

static struct IntuiText About_IText97 = {
	3,0,JAM2,
	8,13,
	NULL,
	"The NetHack WorkBench was designed by",
	&About_IText98
};

#define About_IntuiTextList9 About_IText97

static struct NewWindow About_NewWindowStructure9 = {
	89,60,
	447,83,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&About_Gadget72,
	NULL,
	"About the NetHack WorkBench",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Help3_BorderVectors68[] = {
	0,0,
	489,0
};
static struct Border Help3_Border68 = {
	2,169,
	1,0,JAM1,
	2,
	Help3_BorderVectors68,
	NULL
};

static struct Gadget Help3_Gadget75 = {
	NULL,
	0,0,
	1,1,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Help3_Border68,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static struct IntuiText Help3_IText106 = {
	1,0,JAM2,
	8,2,
	NULL,
	"BKWD",
	NULL
};

static struct Gadget Help3_Gadget74 = {
	&Help3_Gadget75,
	434,173,
	47,10,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&Help3_IText106,
	NULL,
	NULL,
	GADHELPBKWD,
	NULL
};

static struct IntuiText Help3_IText107 = {
	1,0,JAM2,
	8,2,
	NULL,
	"FRWD",
	NULL
};

static struct Gadget Help3_Gadget73 = {
	&Help3_Gadget74,
	12,173,
	47,10,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&Help3_IText107,
	NULL,
	NULL,
	GADHELPFRWD,
	NULL
};

#define Help3_GadgetList10 Help3_Gadget73

static struct NewWindow Help3_NewWindowStructure10 = {
	75,9,
	494,189,
	0,1,
	GADGETDOWN+GADGETUP+CLOSEWINDOW+VANILLAKEY+INTUITICKS,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help3_Gadget73,
	NULL,
	"Help for Nethack WorkBench V3.1",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Rst_BorderVectors69[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Rst_Border69 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors69,
	NULL
};

static struct IntuiText Rst_IText108 = {
	1,0,JAM1,
	5,1,
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
	(APTR)&Rst_Border69,
	NULL,
	&Rst_IText108,
	NULL,
	NULL,
	GADRESTCAN,
	NULL
};

static SHORT Rst_BorderVectors70[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Rst_Border70 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors70,
	NULL
};

static struct IntuiText Rst_IText109 = {
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
	(APTR)&Rst_Border70,
	NULL,
	&Rst_IText109,
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

static SHORT Rst_BorderVectors71[] = {
	0,0,
	187,0,
	187,10,
	0,10,
	0,0
};
static struct Border Rst_Border71 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors71,
	NULL
};

static struct Gadget Rst_RestOld = {
	&Rst_RestOkay,
	101,28,
	186,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Rst_Border71,
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

static SHORT Rst_BorderVectors72[] = {
	0,0,
	187,0,
	187,10,
	0,10,
	0,0
};
static struct Border Rst_Border72 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors72,
	NULL
};

static struct Gadget Rst_RestDir = {
	&Rst_RestOld,
	101,15,
	186,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Rst_Border72,
	NULL,
	NULL,
	NULL,
	(APTR)&Rst_Rst_RestDirSInfo,
	GADRESTDIR,
	NULL
};

#define Rst_GadgetList11 Rst_RestDir

static struct IntuiText Rst_IText111 = {
	1,0,JAM1,
	18,29,
	NULL,
	"Old File:",
	NULL
};

static struct IntuiText Rst_IText110 = {
	1,0,JAM1,
	14,15,
	NULL,
	"Directory:",
	&Rst_IText111
};

#define Rst_IntuiTextList11 Rst_IText110

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
