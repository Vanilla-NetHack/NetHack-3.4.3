
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

static struct MenuItem MenuItem6 = {
	NULL,
	0,40,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText2,
	NULL,
	'R',
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

static struct MenuItem MenuItem5 = {
	&MenuItem6,
	0,32,
	152,8,
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
	"Copy Options",
	NULL
};

static struct MenuItem MenuItem4 = {
	&MenuItem5,
	0,24,
	152,8,
	ITEMTEXT+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText4,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText IText5 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Set Options",
	NULL
};

static struct MenuItem MenuItem3 = {
	&MenuItem4,
	0,16,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText5,
	NULL,
	'O',
	NULL,
	MENUNULL
};

static struct IntuiText IText6 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Change Comment",
	NULL
};

static struct MenuItem MenuItem2 = {
	&MenuItem3,
	0,8,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText6,
	NULL,
	'C',
	NULL,
	MENUNULL
};

static struct IntuiText IText7 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Info",
	NULL
};

static struct MenuItem MenuItem1 = {
	&MenuItem2,
	0,0,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText7,
	NULL,
	'I',
	NULL,
	MENUNULL
};

static struct Menu Menu2 = {
	NULL,
	70,0,
	39,0,
	MENUENABLED,
	"Game",
	&MenuItem1
};

static struct IntuiText IText8 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Quit",
	NULL
};

static struct MenuItem MenuItem13 = {
	NULL,
	0,48,
	184,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText8,
	NULL,
	'Q',
	NULL,
	MENUNULL
};

static struct IntuiText IText9 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Edit Configuration",
	NULL
};

static struct MenuItem MenuItem12 = {
	&MenuItem13,
	0,40,
	184,8,
	ITEMTEXT+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText9,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText IText10 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Edit Default Game",
	NULL
};

static struct MenuItem MenuItem11 = {
	&MenuItem12,
	0,32,
	184,8,
	ITEMTEXT+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText10,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText IText11 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Recover",
	NULL
};

static struct MenuItem MenuItem10 = {
	&MenuItem11,
	0,24,
	184,8,
	ITEMTEXT+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText11,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText IText12 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Top Scores",
	NULL
};

static struct MenuItem MenuItem9 = {
	&MenuItem10,
	0,16,
	184,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText12,
	NULL,
	'S',
	NULL,
	MENUNULL
};

static struct IntuiText IText13 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"About",
	NULL
};

static struct MenuItem MenuItem8 = {
	&MenuItem9,
	0,8,
	184,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText13,
	NULL,
	'A',
	NULL,
	MENUNULL
};

static struct IntuiText IText14 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"Help",
	NULL
};

static struct MenuItem MenuItem7 = {
	&MenuItem8,
	0,0,
	184,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&IText14,
	NULL,
	'H',
	NULL,
	MENUNULL
};

static struct Menu Menu1 = {
	&Menu2,
	0,0,
	63,0,
	MENUENABLED,
	"Project",
	&MenuItem7
};

#define MenuList1 Menu1

static struct NewWindow NewWindowStructure1 = {
	34,23,
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

static struct IntuiText Quest_IText15 = {
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
	&Quest_IText15,
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

static struct IntuiText Quest_IText16 = {
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
	&Quest_IText16,
	NULL,
	NULL,
	GADQUESTYES,
	NULL
};

#define Quest_GadgetList2 Quest_Yes

static struct IntuiText Quest_IText17 = {
	1,0,JAM2,
	59,21,
	NULL,
	"Sure you want to QUIT?",
	NULL
};

#define Quest_IntuiTextList2 Quest_IText17

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
	91,0,
	91,11,
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

static struct IntuiText Options_IText18 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Checkpoint",
	NULL
};

static struct Gadget Options_Gadget40 = {
	NULL,
	253,27,
	90,10,
	SELECTED,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border5,
	NULL,
	&Options_IText18,
	NULL,
	NULL,
	GADOCHKPOINT,
	NULL
};

static SHORT Options_BorderVectors6[] = {
	0,0,
	91,0,
	91,11,
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

static struct IntuiText Options_IText19 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Show Score",
	NULL
};

static struct Gadget Options_Gadget39 = {
	&Options_Gadget40,
	439,55,
	90,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border6,
	NULL,
	&Options_IText19,
	NULL,
	NULL,
	GADOSHOWSCORE,
	NULL
};

static SHORT Options_BorderVectors7[] = {
	0,0,
	128,0,
	128,11,
	0,11,
	0,0
};
static struct Border Options_Border7 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors7,
	NULL
};

static struct IntuiText Options_IText20 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Show Experience",
	NULL
};

static struct Gadget Options_Gadget38 = {
	&Options_Gadget39,
	306,55,
	127,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border7,
	NULL,
	&Options_IText20,
	NULL,
	NULL,
	GADOSHOWEXP,
	NULL
};

static SHORT Options_BorderVectors8[] = {
	0,0,
	105,0,
	105,11,
	0,11,
	0,0
};
static struct Border Options_Border8 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors8,
	NULL
};

static struct IntuiText Options_IText21 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Lit Corridor",
	NULL
};

static struct Gadget Options_Gadget37 = {
	&Options_Gadget38,
	130,55,
	104,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border8,
	NULL,
	&Options_IText21,
	NULL,
	NULL,
	GADOLITCORRIDOR,
	NULL
};

static SHORT Options_BorderVectors9[] = {
	0,0,
	60,0,
	60,11,
	0,11,
	0,0
};
static struct Border Options_Border9 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors9,
	NULL
};

static struct IntuiText Options_IText22 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Legacy",
	NULL
};

static struct Gadget Options_Gadget36 = {
	&Options_Gadget37,
	240,55,
	59,10,
	SELECTED,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border9,
	NULL,
	&Options_IText22,
	NULL,
	NULL,
	GADOLEGACY,
	NULL
};

static SHORT Options_BorderVectors10[] = {
	0,0,
	115,0,
	115,11,
	0,11,
	0,0
};
static struct Border Options_Border10 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors10,
	NULL
};

static struct IntuiText Options_IText23 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Highlight Pet",
	NULL
};

static struct Gadget Options_Gadget35 = {
	&Options_Gadget36,
	9,55,
	114,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border10,
	NULL,
	&Options_IText23,
	NULL,
	NULL,
	GADOHILITEPET,
	NULL
};

static SHORT Options_BorderVectors11[] = {
	0,0,
	56,0,
	56,11,
	0,11,
	0,0
};
static struct Border Options_Border11 = {
	-1,-1,
	1,0,JAM1,
	5,
	Options_BorderVectors11,
	NULL
};

static struct IntuiText Options_IText24 = {
	1,0,JAM2,
	11,2,
	NULL,
	"OKAY",
	NULL
};

static struct Gadget Options_Gadget34 = {
	&Options_Gadget35,
	10,120,
	55,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Options_Border11,
	NULL,
	&Options_IText24,
	NULL,
	NULL,
	GADOOKAY,
	NULL
};

static SHORT Options_BorderVectors12[] = {
	0,0,
	56,0,
	56,11,
	0,11,
	0,0
};
static struct Border Options_Border12 = {
	-1,-1,
	1,0,JAM1,
	5,
	Options_BorderVectors12,
	NULL
};

static struct IntuiText Options_IText25 = {
	1,0,JAM2,
	3,2,
	NULL,
	"CANCEL",
	NULL
};

static struct Gadget Options_Gadget33 = {
	&Options_Gadget34,
	474,120,
	55,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Options_Border12,
	NULL,
	&Options_IText25,
	NULL,
	NULL,
	GADOCANCEL,
	NULL
};

static SHORT Options_BorderVectors13[] = {
	0,0,
	72,0,
	72,11,
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

static struct IntuiText Options_IText26 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Ask Save",
	NULL
};

static struct Gadget Options_Gadget32 = {
	&Options_Gadget33,
	458,27,
	71,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border13,
	NULL,
	&Options_IText26,
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

static SHORT Options_BorderVectors14[] = {
	0,0,
	215,0,
	215,10,
	0,10,
	0,0
};
static struct Border Options_Border14 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors14,
	NULL
};

static struct Gadget Options_Objects = {
	&Options_Gadget32,
	211,121,
	214,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border14,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_ObjectsSInfo,
	GADOOBJECTS,
	NULL
};

static SHORT Options_BorderVectors15[] = {
	0,0,
	55,0,
	55,11,
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

static struct IntuiText Options_IText27 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Female",
	NULL
};

static struct Gadget Options_Gadget30 = {
	&Options_Objects,
	475,13,
	54,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border15,
	NULL,
	&Options_IText27,
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

static SHORT Options_BorderVectors16[] = {
	0,0,
	215,0,
	215,10,
	0,10,
	0,0
};
static struct Border Options_Border16 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors16,
	NULL
};

static struct Gadget Options_Fruit = {
	&Options_Gadget30,
	211,108,
	214,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border16,
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

static SHORT Options_BorderVectors17[] = {
	0,0,
	215,0,
	215,10,
	0,10,
	0,0
};
static struct Border Options_Border17 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors17,
	NULL
};

static struct Gadget Options_DogName = {
	&Options_Fruit,
	211,95,
	214,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border17,
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

static SHORT Options_BorderVectors18[] = {
	0,0,
	215,0,
	215,10,
	0,10,
	0,0
};
static struct Border Options_Border18 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors18,
	NULL
};

static struct Gadget Options_CatName = {
	&Options_DogName,
	211,82,
	214,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border18,
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

static SHORT Options_BorderVectors19[] = {
	0,0,
	215,0,
	215,10,
	0,10,
	0,0
};
static struct Border Options_Border19 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors19,
	NULL
};

static struct Gadget Options_PackOrder = {
	&Options_CatName,
	211,69,
	214,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Options_Border19,
	NULL,
	NULL,
	NULL,
	(APTR)&Options_Options_PackOrderSInfo,
	GADOPACKORDER,
	NULL
};

static SHORT Options_BorderVectors20[] = {
	0,0,
	61,0,
	61,11,
	0,11,
	0,0
};
static struct Border Options_Border20 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors20,
	NULL
};

static struct IntuiText Options_IText28 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Verbose",
	NULL
};

static struct Gadget Options_Gadget25 = {
	&Options_PackOrder,
	391,27,
	60,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border20,
	NULL,
	&Options_IText28,
	NULL,
	NULL,
	GADOVERBOSE,
	NULL
};

static SHORT Options_BorderVectors21[] = {
	0,0,
	86,0,
	86,11,
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

static struct IntuiText Options_IText29 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Tomb Stone",
	NULL
};

static struct Gadget Options_Gadget24 = {
	&Options_Gadget25,
	383,13,
	85,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border21,
	NULL,
	&Options_IText29,
	NULL,
	NULL,
	GADOTOMBSTONE,
	NULL
};

static SHORT Options_BorderVectors22[] = {
	0,0,
	39,0,
	39,11,
	0,11,
	0,0
};
static struct Border Options_Border22 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors22,
	NULL
};

static struct IntuiText Options_IText30 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Time",
	NULL
};

static struct Gadget Options_Gadget23 = {
	&Options_Gadget24,
	9,27,
	38,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border22,
	NULL,
	&Options_IText30,
	NULL,
	NULL,
	GADOTIME,
	NULL
};

static SHORT Options_BorderVectors23[] = {
	0,0,
	78,0,
	78,11,
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

static struct IntuiText Options_IText31 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Stand Out",
	NULL
};

static struct Gadget Options_Gadget22 = {
	&Options_Gadget23,
	398,41,
	77,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border23,
	NULL,
	&Options_IText31,
	NULL,
	NULL,
	GADOSTANDOUT,
	NULL
};

static SHORT Options_BorderVectors24[] = {
	0,0,
	48,0,
	48,11,
	0,11,
	0,0
};
static struct Border Options_Border24 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors24,
	NULL
};

static struct IntuiText Options_IText32 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Sound",
	NULL
};

static struct Gadget Options_Gadget21 = {
	&Options_Gadget22,
	482,41,
	47,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border24,
	NULL,
	&Options_IText32,
	NULL,
	NULL,
	GADOSOUND,
	NULL
};

static SHORT Options_BorderVectors25[] = {
	0,0,
	79,0,
	79,11,
	0,11,
	0,0
};
static struct Border Options_Border25 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors25,
	NULL
};

static struct IntuiText Options_IText33 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Sort Pack",
	NULL
};

static struct Gadget Options_Gadget20 = {
	&Options_Gadget21,
	314,41,
	78,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border25,
	NULL,
	&Options_IText33,
	NULL,
	NULL,
	GADOSORTPACK,
	NULL
};

static SHORT Options_BorderVectors26[] = {
	0,0,
	70,0,
	70,11,
	0,11,
	0,0
};
static struct Border Options_Border26 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors26,
	NULL
};

static struct IntuiText Options_IText34 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Safe Pet",
	NULL
};

static struct Gadget Options_Gadget19 = {
	&Options_Gadget20,
	239,41,
	69,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border26,
	NULL,
	&Options_IText34,
	NULL,
	NULL,
	GADOSAFEPET,
	NULL
};

static SHORT Options_BorderVectors27[] = {
	0,0,
	55,0,
	55,11,
	0,11,
	0,0
};
static struct Border Options_Border27 = {
	-1,-1,
	3,0,JAM1,
	5,
	Options_BorderVectors27,
	NULL
};

static struct IntuiText Options_IText35 = {
	1,0,JAM2,
	4,2,
	NULL,
	"Silent",
	NULL
};

static struct Gadget Options_Gadget18 = {
	&Options_Gadget19,
	323,13,
	54,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border27,
	NULL,
	&Options_IText35,
	NULL,
	NULL,
	GADOSILENT,
	NULL
};

static SHORT Options_BorderVectors28[] = {
	0,0,
	112,0,
	112,11,
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

static struct IntuiText Options_IText36 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Rest On Space",
	NULL
};

static struct Gadget Options_Gadget17 = {
	&Options_Gadget18,
	206,13,
	111,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border28,
	NULL,
	&Options_IText36,
	NULL,
	NULL,
	GADORESTONSPACE,
	NULL
};

static SHORT Options_BorderVectors29[] = {
	0,0,
	66,0,
	66,11,
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

static struct IntuiText Options_IText37 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Pick Up",
	NULL
};

static struct Gadget Options_Gadget16 = {
	&Options_Gadget17,
	9,41,
	65,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border29,
	NULL,
	&Options_IText37,
	NULL,
	NULL,
	GADOPICKUP,
	NULL
};

static SHORT Options_BorderVectors30[] = {
	0,0,
	37,0,
	37,11,
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

static struct IntuiText Options_IText38 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Null",
	NULL
};

static struct Gadget Options_Gadget15 = {
	&Options_Gadget16,
	349,27,
	36,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border30,
	NULL,
	&Options_IText38,
	NULL,
	NULL,
	GADONULL,
	NULL
};

static SHORT Options_BorderVectors31[] = {
	0,0,
	86,0,
	86,11,
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

static struct IntuiText Options_IText39 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Number Pad",
	NULL
};

static struct Gadget Options_Gadget14 = {
	&Options_Gadget15,
	148,41,
	85,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border31,
	NULL,
	&Options_IText39,
	NULL,
	NULL,
	GADONUMBERPAD,
	NULL
};

static SHORT Options_BorderVectors32[] = {
	0,0,
	38,0,
	38,11,
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

static struct IntuiText Options_IText40 = {
	1,0,JAM2,
	3,2,
	NULL,
	"News",
	NULL
};

static struct Gadget Options_Gadget13 = {
	&Options_Gadget14,
	53,27,
	37,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border32,
	NULL,
	&Options_IText40,
	NULL,
	NULL,
	GADONEWS,
	NULL
};

static SHORT Options_BorderVectors33[] = {
	0,0,
	62,0,
	62,11,
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

static struct IntuiText Options_IText41 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Ignintr",
	NULL
};

static struct Gadget Options_Gadget12 = {
	&Options_Gadget13,
	80,41,
	61,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border33,
	NULL,
	&Options_IText41,
	NULL,
	NULL,
	GADOIGNINTR,
	NULL
};

static SHORT Options_BorderVectors34[] = {
	0,0,
	38,0,
	38,11,
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

static struct IntuiText Options_IText42 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Help",
	NULL
};

static struct Gadget Options_Gadget11 = {
	&Options_Gadget12,
	96,27,
	37,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border34,
	NULL,
	&Options_IText42,
	NULL,
	NULL,
	GADOHELP,
	NULL
};

static SHORT Options_BorderVectors35[] = {
	0,0,
	50,0,
	50,11,
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

static struct IntuiText Options_IText43 = {
	1,0,JAM2,
	4,2,
	NULL,
	"Flush",
	NULL
};

static struct Gadget Options_Gadget10 = {
	&Options_Gadget11,
	140,27,
	49,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border35,
	NULL,
	&Options_IText43,
	NULL,
	NULL,
	GADOFLUSH,
	NULL
};

static SHORT Options_BorderVectors36[] = {
	0,0,
	54,0,
	54,11,
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

static struct IntuiText Options_IText44 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Fixinv",
	NULL
};

static struct Gadget Options_Gadget9 = {
	&Options_Gadget10,
	195,27,
	53,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border36,
	NULL,
	&Options_IText44,
	NULL,
	NULL,
	GADOFIXINV,
	NULL
};

static SHORT Options_BorderVectors37[] = {
	0,0,
	69,0,
	69,11,
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

static struct IntuiText Options_IText45 = {
	1,0,JAM2,
	2,2,
	NULL,
	"Disclose",
	NULL
};

static struct Gadget Options_Gadget8 = {
	&Options_Gadget9,
	133,13,
	68,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border37,
	NULL,
	&Options_IText45,
	NULL,
	NULL,
	GADODISCLOSE,
	NULL
};

static SHORT Options_BorderVectors38[] = {
	0,0,
	63,0,
	63,11,
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

static struct IntuiText Options_IText46 = {
	1,0,JAM2,
	3,2,
	NULL,
	"Confirm",
	NULL
};

static struct Gadget Options_Gadget7 = {
	&Options_Gadget8,
	65,13,
	62,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border38,
	NULL,
	&Options_IText46,
	NULL,
	NULL,
	GADOCONFIRM,
	NULL
};

static SHORT Options_BorderVectors39[] = {
	0,0,
	51,0,
	51,11,
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

static struct IntuiText Options_IText47 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Color",
	NULL
};

static struct Gadget Options_Gadget6 = {
	&Options_Gadget7,
	9,13,
	50,10,
	NULL,
	RELVERIFY+TOGGLESELECT,
	BOOLGADGET,
	(APTR)&Options_Border39,
	NULL,
	&Options_IText47,
	NULL,
	NULL,
	GADOCOLOR,
	NULL
};

#define Options_GadgetList3 Options_Gadget6

static struct IntuiText Options_IText52 = {
	3,0,JAM2,
	142,122,
	NULL,
	"Objects:",
	NULL
};

static struct IntuiText Options_IText51 = {
	3,0,JAM2,
	158,109,
	NULL,
	"Fruit:",
	&Options_IText52
};

static struct IntuiText Options_IText50 = {
	3,0,JAM2,
	134,96,
	NULL,
	"Dog Name:",
	&Options_IText51
};

static struct IntuiText Options_IText49 = {
	3,0,JAM2,
	134,83,
	NULL,
	"Cat Name:",
	&Options_IText50
};

static struct IntuiText Options_IText48 = {
	3,0,JAM2,
	118,70,
	NULL,
	"Pack Order:",
	&Options_IText49
};

#define Options_IntuiTextList3 Options_IText48

static struct NewWindow Options_NewWindowStructure3 = {
	52,58,
	538,135,
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

static SHORT Conf_BorderVectors40[] = {
	0,0,
	52,0,
	52,11,
	0,11,
	0,0
};
static struct Border Conf_Border40 = {
	-1,-1,
	2,0,JAM1,
	5,
	Conf_BorderVectors40,
	NULL
};

static struct IntuiText Conf_IText53 = {
	1,0,JAM2,
	10,2,
	NULL,
	"Save",
	NULL
};

static struct Gadget Conf_Gadget48 = {
	NULL,
	73,85,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Conf_Border40,
	NULL,
	&Conf_IText53,
	NULL,
	NULL,
	GADCONFSAVE,
	NULL
};

static SHORT Conf_BorderVectors41[] = {
	0,0,
	52,0,
	52,11,
	0,11,
	0,0
};
static struct Border Conf_Border41 = {
	-1,-1,
	2,0,JAM1,
	5,
	Conf_BorderVectors41,
	NULL
};

static struct IntuiText Conf_IText54 = {
	1,0,JAM2,
	10,2,
	NULL,
	"Load",
	NULL
};

static struct Gadget Conf_Gadget47 = {
	&Conf_Gadget48,
	9,85,
	51,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Conf_Border41,
	NULL,
	&Conf_IText54,
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

static SHORT Conf_BorderVectors42[] = {
	0,0,
	242,0,
	242,10,
	0,10,
	0,0
};
static struct Border Conf_Border42 = {
	-1,-1,
	1,0,JAM1,
	5,
	Conf_BorderVectors42,
	NULL
};

static struct Gadget Conf_ConfigName = {
	&Conf_Gadget47,
	151,100,
	241,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Conf_Border42,
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

static SHORT Conf_BorderVectors43[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border43 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors43,
	NULL
};

static struct Gadget Conf_StrSave = {
	&Conf_ConfigName,
	81,70,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border43,
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

static SHORT Conf_BorderVectors44[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border44 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors44,
	NULL
};

static struct Gadget Conf_StrLevels = {
	&Conf_StrSave,
	81,56,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border44,
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

static SHORT Conf_BorderVectors45[] = {
	0,0,
	311,0,
	311,10,
	0,10,
	0,0
};
static struct Border Conf_Border45 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors45,
	NULL
};

static struct Gadget Conf_StrPath = {
	&Conf_StrLevels,
	81,14,
	310,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border45,
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

static SHORT Conf_BorderVectors46[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border46 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors46,
	NULL
};

static struct Gadget Conf_StrPens = {
	&Conf_StrPath,
	81,42,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border46,
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

static SHORT Conf_BorderVectors47[] = {
	0,0,
	312,0,
	312,10,
	0,10,
	0,0
};
static struct Border Conf_Border47 = {
	-1,-1,
	3,0,JAM1,
	5,
	Conf_BorderVectors47,
	NULL
};

static struct Gadget Conf_StrHackdir = {
	&Conf_StrPens,
	81,28,
	311,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Conf_Border47,
	NULL,
	NULL,
	NULL,
	(APTR)&Conf_Conf_StrHackdirSInfo,
	GADSTRHACKDIR,
	NULL
};

#define Conf_GadgetList4 Conf_StrHackdir

static struct IntuiText Conf_IText60 = {
	1,0,JAM2,
	10,101,
	NULL,
	"Config File Name:",
	NULL
};

static struct IntuiText Conf_IText59 = {
	3,0,JAM2,
	7,72,
	NULL,
	"Save Dir:",
	&Conf_IText60
};

static struct IntuiText Conf_IText58 = {
	3,0,JAM2,
	23,58,
	NULL,
	"Levels:",
	&Conf_IText59
};

static struct IntuiText Conf_IText57 = {
	3,0,JAM2,
	39,44,
	NULL,
	"Pens:",
	&Conf_IText58
};

static struct IntuiText Conf_IText56 = {
	3,0,JAM2,
	15,30,
	NULL,
	"Hackdir:",
	&Conf_IText57
};

static struct IntuiText Conf_IText55 = {
	3,0,JAM2,
	39,16,
	NULL,
	"Path:",
	&Conf_IText56
};

#define Conf_IntuiTextList4 Conf_IText55

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

static SHORT Str_BorderVectors48[] = {
	0,0,
	57,0,
	57,11,
	0,11,
	0,0
};
static struct Border Str_Border48 = {
	-1,-1,
	3,0,JAM1,
	5,
	Str_BorderVectors48,
	NULL
};

static struct IntuiText Str_IText61 = {
	3,0,JAM2,
	4,2,
	NULL,
	"Cancel",
	NULL
};

static struct Gadget Str_Gadget50 = {
	NULL,
	9,15,
	56,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Str_Border48,
	NULL,
	&Str_IText61,
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

static SHORT Str_BorderVectors49[] = {
	0,0,
	439,0,
	439,11,
	0,11,
	0,0
};
static struct Border Str_Border49 = {
	-1,-1,
	3,0,JAM1,
	5,
	Str_BorderVectors49,
	NULL
};

static struct Gadget Str_String = {
	&Str_Gadget50,
	77,15,
	438,10,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Str_Border49,
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

static SHORT Info_BorderVectors50[] = {
	0,0,
	82,0,
	82,11,
	0,11,
	0,0
};
static struct Border Info_Border50 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors50,
	NULL
};

static struct IntuiText Info_IText62 = {
	3,0,JAM2,
	18,1,
	NULL,
	"Delete",
	NULL
};

static struct Gadget Info_Gadget59 = {
	NULL,
	208,61,
	81,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border50,
	NULL,
	&Info_IText62,
	NULL,
	NULL,
	GADDELTOOL,
	NULL
};

static SHORT Info_BorderVectors51[] = {
	0,0,
	82,0,
	82,11,
	0,11,
	0,0
};
static struct Border Info_Border51 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors51,
	NULL
};

static struct IntuiText Info_IText63 = {
	3,0,JAM2,
	29,1,
	NULL,
	"Add",
	NULL
};

static struct Gadget Info_Gadget58 = {
	&Info_Gadget59,
	115,61,
	81,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border51,
	NULL,
	&Info_IText63,
	NULL,
	NULL,
	GADADDTOOL,
	NULL
};

static struct IntuiText Info_IText64 = {
	1,0,JAM2,
	16,9,
	NULL,
	"Edit Game Definition",
	NULL
};

static struct Gadget Info_EditDef = {
	&Info_Gadget58,
	353,15,
	192,26,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Info_IText64,
	NULL,
	NULL,
	GADEDDEF,
	NULL
};

static SHORT Info_BorderVectors52[] = {
	0,0,
	14,0,
	14,7,
	0,7,
	0,0
};
static struct Border Info_Border52 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors52,
	NULL
};

static struct Gadget Info_ToolDown = {
	&Info_EditDef,
	97,81,
	13,6,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border52,
	NULL,
	NULL,
	NULL,
	NULL,
	GADTOOLDOWN,
	NULL
};

static SHORT Info_BorderVectors53[] = {
	0,0,
	14,0,
	14,7,
	0,7,
	0,0
};
static struct Border Info_Border53 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors53,
	NULL
};

static struct Gadget Info_ToolUp = {
	&Info_ToolDown,
	97,74,
	13,6,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Info_Border53,
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

static SHORT Info_BorderVectors54[] = {
	0,0,
	430,0,
	430,10,
	0,10,
	0,0
};
static struct Border Info_Border54 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors54,
	NULL
};

static struct Gadget Info_ToolTypes = {
	&Info_ToolUp,
	115,76,
	429,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&Info_Border54,
	NULL,
	NULL,
	NULL,
	(APTR)&Info_Info_ToolTypesSInfo,
	GADTOOLTYPES,
	NULL
};

static SHORT Info_BorderVectors55[] = {
	0,0,
	197,0,
	197,12,
	0,12,
	0,0
};
static struct Border Info_Border55 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors55,
	NULL
};

static struct Gadget Info_Class = {
	&Info_ToolTypes,
	148,30,
	196,11,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Info_Border55,
	NULL,
	NULL,
	NULL,
	NULL,
	-1,
	NULL
};

static SHORT Info_BorderVectors56[] = {
	0,0,
	197,0,
	197,12,
	0,12,
	0,0
};
static struct Border Info_Border56 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors56,
	NULL
};

static struct Gadget Info_Player = {
	&Info_Class,
	148,15,
	196,11,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Info_Border56,
	NULL,
	NULL,
	NULL,
	NULL,
	-1,
	NULL
};

static SHORT Info_BorderVectors57[] = {
	0,0,
	463,0,
	463,11,
	0,11,
	0,0
};
static struct Border Info_Border57 = {
	-1,-1,
	3,0,JAM1,
	5,
	Info_BorderVectors57,
	NULL
};

static struct Gadget Info_Comment = {
	&Info_Player,
	83,46,
	462,10,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Info_Border57,
	NULL,
	NULL,
	NULL,
	NULL,
	-1,
	NULL
};

#define Info_GadgetList6 Info_Comment

static struct IntuiText Info_IText68 = {
	3,0,JAM2,
	13,76,
	NULL,
	"ToolTypes:",
	NULL
};

static struct IntuiText Info_IText67 = {
	3,0,JAM2,
	13,34,
	NULL,
	"Character Class:",
	&Info_IText68
};

static struct IntuiText Info_IText66 = {
	3,0,JAM2,
	45,19,
	NULL,
	"Player Name:",
	&Info_IText67
};

static struct IntuiText Info_IText65 = {
	3,0,JAM2,
	13,49,
	NULL,
	"Comment:",
	&Info_IText66
};

#define Info_IntuiTextList6 Info_IText65

static struct NewWindow Info_NewWindowStructure6 = {
	40,60,
	555,92,
	0,1,
	GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+VANILLAKEY,
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

static struct IntuiText Help1_IText69 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Help1_Gadget60 = {
	NULL,
	12,34,
	47,12,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Help1_IText69,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define Help1_GadgetList7 Help1_Gadget60

static struct IntuiText Help1_IText71 = {
	3,0,JAM2,
	10,22,
	NULL,
	"to start a new game or to resume a saved game.",
	NULL
};

static struct IntuiText Help1_IText70 = {
	3,0,JAM2,
	9,13,
	NULL,
	"Click on NewGame Gadget or a Saved Game twice",
	&Help1_IText71
};

#define Help1_IntuiTextList7 Help1_IText70

static struct NewWindow Help1_NewWindowStructure7 = {
	134,60,
	385,51,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help1_Gadget60,
	NULL,
	"Help for Game Selection",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText Help2_IText72 = {
	1,0,JAM2,
	7,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget Help2_Gadget61 = {
	NULL,
	172,63,
	47,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&Help2_IText72,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define Help2_GadgetList8 Help2_Gadget61

static struct IntuiText Help2_IText76 = {
	3,0,JAM2,
	14,46,
	NULL,
	"Use the Help menu item to view the help file",
	NULL
};

static struct IntuiText Help2_IText75 = {
	3,0,JAM2,
	15,31,
	NULL,
	"resume the saved game.",
	&Help2_IText76
};

static struct IntuiText Help2_IText74 = {
	3,0,JAM2,
	15,22,
	NULL,
	"selected game, or double click on a game to",
	&Help2_IText75
};

static struct IntuiText Help2_IText73 = {
	3,0,JAM2,
	15,13,
	NULL,
	"Use Menu button to select operation on the",
	&Help2_IText74
};

#define Help2_IntuiTextList8 Help2_IText73

static struct NewWindow Help2_NewWindowStructure8 = {
	136,60,
	380,82,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help2_Gadget61,
	NULL,
	"Help for Game Manipulation",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static struct IntuiText About_IText77 = {
	1,0,JAM2,
	40,2,
	NULL,
	"Okay",
	NULL
};

static struct Gadget About_Gadget62 = {
	NULL,
	163,68,
	109,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	NULL,
	NULL,
	&About_IText77,
	NULL,
	NULL,
	GADHELPOKAY,
	NULL
};

#define About_GadgetList9 About_Gadget62

static struct IntuiText About_IText86 = {
	2,0,JAM2,
	10,56,
	NULL,
	"1992 see NetHack license for details and limitations!",
	NULL
};

static struct IntuiText About_IText85 = {
	2,0,JAM2,
	20,47,
	NULL,
	"HackWB is copyright Gregg Wonderly and Ken Lorber,",
	&About_IText86
};

static struct IntuiText About_IText84 = {
	3,0,JAM2,
	8,31,
	NULL,
	"finished by Gregg...",
	&About_IText85
};

static struct IntuiText About_IText83 = {
	3,0,JAM2,
	135,22,
	NULL,
	"The programming was started by Ken and",
	&About_IText84
};

static struct IntuiText About_IText82 = {
	3,0,JAM2,
	120,22,
	NULL,
	".",
	&About_IText83
};

static struct IntuiText About_IText81 = {
	2,0,JAM2,
	8,22,
	NULL,
	"Gregg Wonderly",
	&About_IText82
};

static struct IntuiText About_IText80 = {
	3,0,JAM2,
	396,13,
	NULL,
	"and",
	&About_IText81
};

static struct IntuiText About_IText79 = {
	2,0,JAM2,
	310,13,
	NULL,
	"Ken Lorber",
	&About_IText80
};

static struct IntuiText About_IText78 = {
	3,0,JAM2,
	8,13,
	NULL,
	"The NetHack WorkBench was designed by",
	&About_IText79
};

#define About_IntuiTextList9 About_IText78

static struct NewWindow About_NewWindowStructure9 = {
	89,60,
	447,83,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&About_Gadget62,
	NULL,
	"About the NetHack WorkBench",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Help3_BorderVectors58[] = {
	0,0,
	489,0
};
static struct Border Help3_Border58 = {
	2,169,
	1,0,JAM1,
	2,
	Help3_BorderVectors58,
	NULL
};

static struct Gadget Help3_Gadget65 = {
	NULL,
	0,0,
	1,1,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Help3_Border58,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static struct IntuiText Help3_IText87 = {
	1,0,JAM2,
	8,2,
	NULL,
	"BKWD",
	NULL
};

static struct Gadget Help3_Gadget64 = {
	&Help3_Gadget65,
	434,173,
	47,10,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&Help3_IText87,
	NULL,
	NULL,
	GADHELPBKWD,
	NULL
};

static struct IntuiText Help3_IText88 = {
	1,0,JAM2,
	8,2,
	NULL,
	"FRWD",
	NULL
};

static struct Gadget Help3_Gadget63 = {
	&Help3_Gadget64,
	12,173,
	47,10,
	NULL,
	RELVERIFY+GADGIMMEDIATE,
	BOOLGADGET,
	NULL,
	NULL,
	&Help3_IText88,
	NULL,
	NULL,
	GADHELPFRWD,
	NULL
};

#define Help3_GadgetList10 Help3_Gadget63

static struct NewWindow Help3_NewWindowStructure10 = {
	76,11,
	494,189,
	0,1,
	MOUSEBUTTONS+GADGETDOWN+GADGETUP+CLOSEWINDOW+ACTIVEWINDOW+INACTIVEWINDOW+VANILLAKEY+INTUITICKS,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Help3_Gadget63,
	NULL,
	"Help for Nethack WorkBench V3.1",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static UBYTE Defs_Defs_DefaultNameSIBuff[50] =
	"CutNHack";
static struct StringInfo Defs_Defs_DefaultNameSInfo = {
	Defs_Defs_DefaultNameSIBuff,
	UNDOBUFFER,
	0,
	50,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static struct Gadget Defs_DefaultName = {
	NULL,
	132,27,
	179,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&Defs_Defs_DefaultNameSInfo,
	GADOUTFILE,
	NULL
};

static SHORT Defs_BorderVectors59[] = {
	0,0,
	109,0,
	109,11,
	0,11,
	0,0
};
static struct Border Defs_Border59 = {
	-1,-1,
	1,0,JAM1,
	5,
	Defs_BorderVectors59,
	NULL
};

static struct IntuiText Defs_IText89 = {
	1,0,JAM2,
	6,2,
	NULL,
	"Edit Options",
	NULL
};

static struct Gadget Defs_Gadget70 = {
	&Defs_DefaultName,
	111,55,
	108,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Defs_Border59,
	NULL,
	&Defs_IText89,
	NULL,
	NULL,
	GADEDOPTIONS,
	NULL
};

static SHORT Defs_BorderVectors60[] = {
	0,0,
	44,0,
	44,11,
	0,11,
	0,0
};
static struct Border Defs_Border60 = {
	-1,-1,
	1,0,JAM1,
	5,
	Defs_BorderVectors60,
	NULL
};

static struct IntuiText Defs_IText90 = {
	1,0,JAM2,
	5,2,
	NULL,
	"Save",
	NULL
};

static struct Gadget Defs_Gadget69 = {
	&Defs_Gadget70,
	268,55,
	43,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Defs_Border60,
	NULL,
	&Defs_IText90,
	NULL,
	NULL,
	GADDEFSAVE,
	NULL
};

static SHORT Defs_BorderVectors61[] = {
	0,0,
	50,0,
	50,11,
	0,11,
	0,0
};
static struct Border Defs_Border61 = {
	-1,-1,
	1,0,JAM1,
	5,
	Defs_BorderVectors61,
	NULL
};

static struct IntuiText Defs_IText91 = {
	1,0,JAM2,
	8,2,
	NULL,
	"Load",
	NULL
};

static struct Gadget Defs_Gadget68 = {
	&Defs_Gadget69,
	12,55,
	49,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Defs_Border61,
	NULL,
	&Defs_IText91,
	NULL,
	NULL,
	GADDEFLOAD,
	NULL
};

static SHORT Defs_BorderVectors62[] = {
	0,0,
	180,0,
	180,11,
	0,11,
	0,0
};
static struct Border Defs_Border62 = {
	-1,-1,
	3,0,JAM1,
	5,
	Defs_BorderVectors62,
	NULL
};

static struct IntuiText Defs_IText92 = {
	1,0,JAM2,
	1,2,
	NULL,
	"Archeologist",
	NULL
};

static struct Gadget Defs_PlayerType = {
	&Defs_Gadget68,
	132,40,
	179,10,
	GADGHBOX+GADGHIMAGE,
	NULL,
	BOOLGADGET,
	(APTR)&Defs_Border62,
	NULL,
	&Defs_IText92,
	NULL,
	NULL,
	-1,
	NULL
};

static UBYTE Defs_Defs_PlayerNameSIBuff[50] =
	"CutNHack";
static struct StringInfo Defs_Defs_PlayerNameSInfo = {
	Defs_Defs_PlayerNameSIBuff,
	UNDOBUFFER,
	0,
	50,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static struct Gadget Defs_PlayerName = {
	&Defs_PlayerType,
	132,14,
	179,9,
	NULL,
	RELVERIFY,
	STRGADGET,
	NULL,
	NULL,
	NULL,
	NULL,
	(APTR)&Defs_Defs_PlayerNameSInfo,
	GADCHARNAME,
	NULL
};

#define Defs_GadgetList11 Defs_PlayerName

static struct IntuiText Defs_IText93 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Wizard",
	NULL
};

static struct MenuItem Defs_MenuItem26 = {
	NULL,
	0,96,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	4095,
	(APTR)&Defs_IText93,
	NULL,
	'W',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText94 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Valkyrie",
	NULL
};

static struct MenuItem Defs_MenuItem25 = {
	&Defs_MenuItem26,
	0,88,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	6143,
	(APTR)&Defs_IText94,
	NULL,
	'V',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText95 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Tourist",
	NULL
};

static struct MenuItem Defs_MenuItem24 = {
	&Defs_MenuItem25,
	0,80,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	7167,
	(APTR)&Defs_IText95,
	NULL,
	'T',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText96 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Samurai",
	NULL
};

static struct MenuItem Defs_MenuItem23 = {
	&Defs_MenuItem24,
	0,72,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	7679,
	(APTR)&Defs_IText96,
	NULL,
	'S',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText97 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Rogue",
	NULL
};

static struct MenuItem Defs_MenuItem22 = {
	&Defs_MenuItem23,
	0,64,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	7935,
	(APTR)&Defs_IText97,
	NULL,
	'R',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText98 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Priest",
	NULL
};

static struct MenuItem Defs_MenuItem21 = {
	&Defs_MenuItem22,
	0,56,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8063,
	(APTR)&Defs_IText98,
	NULL,
	'P',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText99 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Knight",
	NULL
};

static struct MenuItem Defs_MenuItem20 = {
	&Defs_MenuItem21,
	0,48,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8127,
	(APTR)&Defs_IText99,
	NULL,
	'K',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText100 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Healer",
	NULL
};

static struct MenuItem Defs_MenuItem19 = {
	&Defs_MenuItem20,
	0,40,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8159,
	(APTR)&Defs_IText100,
	NULL,
	'H',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText101 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Elf",
	NULL
};

static struct MenuItem Defs_MenuItem18 = {
	&Defs_MenuItem19,
	0,32,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8175,
	(APTR)&Defs_IText101,
	NULL,
	'E',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText102 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Cave Man",
	NULL
};

static struct MenuItem Defs_MenuItem17 = {
	&Defs_MenuItem18,
	0,24,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8183,
	(APTR)&Defs_IText102,
	NULL,
	'C',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText103 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Barbarian",
	NULL
};

static struct MenuItem Defs_MenuItem16 = {
	&Defs_MenuItem17,
	0,16,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8187,
	(APTR)&Defs_IText103,
	NULL,
	'B',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText104 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Archeologist",
	NULL
};

static struct MenuItem Defs_MenuItem15 = {
	&Defs_MenuItem16,
	0,8,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	8189,
	(APTR)&Defs_IText104,
	NULL,
	'A',
	NULL,
	MENUNULL
};

static struct IntuiText Defs_IText105 = {
	3,1,COMPLEMENT,
	19,0,
	NULL,
	"Random",
	NULL
};

static struct MenuItem Defs_MenuItem14 = {
	&Defs_MenuItem15,
	0,0,
	155,8,
	CHECKIT+ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP+CHECKED,
	8190,
	(APTR)&Defs_IText105,
	NULL,
	'?',
	NULL,
	MENUNULL
};

static struct Menu Defs_Menu2 = {
	NULL,
	0,0,
	79,0,
	MENUENABLED,
	"Character",
	&Defs_MenuItem14
};

#define Defs_MenuList11 Defs_Menu2

static struct IntuiText Defs_IText108 = {
	1,0,JAM2,
	31,28,
	NULL,
	"OutPut File:",
	NULL
};

static struct IntuiText Defs_IText107 = {
	1,0,JAM2,
	47,42,
	NULL,
	"Character:",
	&Defs_IText108
};

static struct IntuiText Defs_IText106 = {
	1,0,JAM2,
	15,15,
	NULL,
	"Player's Name:",
	&Defs_IText107
};

#define Defs_IntuiTextList11 Defs_IText106

static struct NewWindow Defs_NewWindowStructure11 = {
	164,60,
	320,70,
	0,1,
	GADGETUP+MENUPICK+CLOSEWINDOW+DISKINSERTED+ACTIVEWINDOW+VANILLAKEY,
	WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Defs_PlayerName,
	NULL,
	"Edit Default Game Definition",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static SHORT Rst_BorderVectors63[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Rst_Border63 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors63,
	NULL
};

static struct IntuiText Rst_IText109 = {
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
	(APTR)&Rst_Border63,
	NULL,
	&Rst_IText109,
	NULL,
	NULL,
	GADRESTCAN,
	NULL
};

static SHORT Rst_BorderVectors64[] = {
	0,0,
	58,0,
	58,11,
	0,11,
	0,0
};
static struct Border Rst_Border64 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors64,
	NULL
};

static struct IntuiText Rst_IText110 = {
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
	(APTR)&Rst_Border64,
	NULL,
	&Rst_IText110,
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

static SHORT Rst_BorderVectors65[] = {
	0,0,
	187,0,
	187,10,
	0,10,
	0,0
};
static struct Border Rst_Border65 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors65,
	NULL
};

static struct Gadget Rst_RestOld = {
	&Rst_RestOkay,
	101,28,
	186,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Rst_Border65,
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

static SHORT Rst_BorderVectors66[] = {
	0,0,
	187,0,
	187,10,
	0,10,
	0,0
};
static struct Border Rst_Border66 = {
	-1,-1,
	3,0,JAM1,
	5,
	Rst_BorderVectors66,
	NULL
};

static struct Gadget Rst_RestDir = {
	&Rst_RestOld,
	101,15,
	186,9,
	NULL,
	RELVERIFY+STRINGCENTER,
	STRGADGET,
	(APTR)&Rst_Border66,
	NULL,
	NULL,
	NULL,
	(APTR)&Rst_Rst_RestDirSInfo,
	GADRESTDIR,
	NULL
};

#define Rst_GadgetList12 Rst_RestDir

static struct IntuiText Rst_IText112 = {
	1,0,JAM1,
	18,29,
	NULL,
	"Old File:",
	NULL
};

static struct IntuiText Rst_IText111 = {
	1,0,JAM1,
	14,15,
	NULL,
	"Directory:",
	&Rst_IText112
};

#define Rst_IntuiTextList12 Rst_IText111

static struct NewWindow Rst_NewWindowStructure12 = {
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
