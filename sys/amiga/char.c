SHORT Type_BorderVectors1[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border1 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors1,
	NULL
};

struct IntuiText Type_IText1 = {
	3,0,JAM2,
	38,1,
	NULL,
	(UBYTE *)"Wizard",
	NULL
};

struct Gadget Type_Gadget13 = {
	NULL,
	137,79,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border1,
	NULL,
	&Type_IText1,
	NULL,
	NULL,
	'W',
	NULL
};

SHORT Type_BorderVectors2[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border2 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors2,
	NULL
};

struct IntuiText Type_IText2 = {
	3,0,JAM2,
	29,1,
	NULL,
	(UBYTE *)"Valkyrie",
	NULL
};

struct Gadget Type_Gadget12 = {
	&Type_Gadget13,
	9,79,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border2,
	NULL,
	&Type_IText2,
	NULL,
	NULL,
	'V',
	NULL
};

SHORT Type_BorderVectors3[] = {
	0,0,
	251,0,
	251,11,
	0,11,
	0,0
};
struct Border Type_Border3 = {
	-1,-1,
	1,0,JAM1,
	5,
	Type_BorderVectors3,
	NULL
};

struct IntuiText Type_IText3 = {
	1,0,JAM2,
	14,1,
	NULL,
	(UBYTE *)"Pick a Random Character Type",
	NULL
};

struct Gadget Type_Gadget11 = {
	&Type_Gadget12,
	9,94,
	250,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border3,
	NULL,
	&Type_IText3,
	NULL,
	NULL,
	1,
	NULL
};

SHORT Type_BorderVectors4[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border4 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors4,
	NULL
};

struct IntuiText Type_IText4 = {
	3,0,JAM2,
	33,1,
	NULL,
	(UBYTE *)"Samurai",
	NULL
};

struct Gadget Type_Gadget10 = {
	&Type_Gadget11,
	9,66,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border4,
	NULL,
	&Type_IText4,
	NULL,
	NULL,
	'S',
	NULL
};

SHORT Type_BorderVectors5[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border5 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors5,
	NULL
};

struct IntuiText Type_IText5 = {
	3,0,JAM2,
	34,1,
	NULL,
	(UBYTE *)"Tourist",
	NULL
};

struct Gadget Type_Gadget9 = {
	&Type_Gadget10,
	137,66,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border5,
	NULL,
	&Type_IText5,
	NULL,
	NULL,
	'T',
	NULL
};

SHORT Type_BorderVectors6[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border6 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors6,
	NULL
};

struct IntuiText Type_IText6 = {
	3,0,JAM2,
	40,1,
	NULL,
	(UBYTE *)"Rogue",
	NULL
};

struct Gadget Type_Gadget8 = {
	&Type_Gadget9,
	137,53,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border6,
	NULL,
	&Type_IText6,
	NULL,
	NULL,
	'R',
	NULL
};

SHORT Type_BorderVectors7[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border7 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors7,
	NULL
};

struct IntuiText Type_IText7 = {
	3,0,JAM2,
	36,1,
	NULL,
	(UBYTE *)"Priest",
	NULL
};

struct Gadget Type_Gadget7 = {
	&Type_Gadget8,
	9,53,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border7,
	NULL,
	&Type_IText7,
	NULL,
	NULL,
	'P',
	NULL
};

SHORT Type_BorderVectors8[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border8 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors8,
	NULL
};

struct IntuiText Type_IText8 = {
	3,0,JAM2,
	35,1,
	NULL,
	(UBYTE *)"Healer",
	NULL
};

struct Gadget Type_Gadget6 = {
	&Type_Gadget7,
	9,40,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border8,
	NULL,
	&Type_IText8,
	NULL,
	NULL,
	'H',
	NULL
};

SHORT Type_BorderVectors9[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border9 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors9,
	NULL
};

struct IntuiText Type_IText9 = {
	3,0,JAM2,
	33,1,
	NULL,
	(UBYTE *)"Caveman",
	NULL
};

struct Gadget Type_Gadget5 = {
	&Type_Gadget6,
	9,27,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border9,
	NULL,
	&Type_IText9,
	NULL,
	NULL,
	'C',
	NULL
};

SHORT Type_BorderVectors10[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border10 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors10,
	NULL
};

struct IntuiText Type_IText10 = {
	3,0,JAM2,
	16,1,
	NULL,
	(UBYTE *)"Archeologist",
	NULL
};

struct Gadget Type_Gadget4 = {
	&Type_Gadget5,
	9,14,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border10,
	NULL,
	&Type_IText10,
	NULL,
	NULL,
	'A',
	NULL
};

SHORT Type_BorderVectors11[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border11 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors11,
	NULL
};

struct IntuiText Type_IText11 = {
	3,0,JAM2,
	36,1,
	NULL,
	(UBYTE *)"Knight",
	NULL
};

struct Gadget Type_Gadget3 = {
	&Type_Gadget4,
	137,40,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border11,
	NULL,
	&Type_IText11,
	NULL,
	NULL,
	'K',
	NULL
};

SHORT Type_BorderVectors12[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border12 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors12,
	NULL
};

struct IntuiText Type_IText12 = {
	3,0,JAM2,
	48,1,
	NULL,
	(UBYTE *)"Elf",
	NULL
};

struct Gadget Type_Gadget2 = {
	&Type_Gadget3,
	137,27,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border12,
	NULL,
	&Type_IText12,
	NULL,
	NULL,
	'E',
	NULL
};

SHORT Type_BorderVectors13[] = {
	0,0,
	123,0,
	123,11,
	0,11,
	0,0
};
struct Border Type_Border13 = {
	-1,-1,
	3,0,JAM1,
	5,
	Type_BorderVectors13,
	NULL
};

struct IntuiText Type_IText13 = {
	3,0,JAM2,
	27,1,
	NULL,
	(UBYTE *)"Barbarian",
	NULL
};

struct Gadget Type_Gadget1 = {
	&Type_Gadget2,
	137,14,
	122,10,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&Type_Border13,
	NULL,
	&Type_IText13,
	NULL,
	NULL,
	'B',
	NULL
};

#define Type_GadgetList1 Type_Gadget1

struct NewWindow Type_NewWindowStructure1 = {
	155,24,
	267,108,
	0,1,
	GADGETUP+CLOSEWINDOW+VANILLAKEY,
	WINDOWCLOSE+ACTIVATE+NOCAREREFRESH,
	&Type_Gadget1,
	NULL,
	(UBYTE *)"Pick a Character",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};


/* end of PowerWindows source generation */
