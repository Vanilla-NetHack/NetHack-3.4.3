SHORT Rnd_BorderVectors1[] = {
	0,0,
	49,0,
	49,18,
	0,18,
	0,0
};
struct Border Rnd_Border1 = {
	-1,-1,	/* XY origin relative to container TopLeft */
	3,0,JAM1,	/* front pen, back pen and drawmode */
	5,	/* number of XY vectors */
	Rnd_BorderVectors1,	/* pointer to XY vectors */
	NULL	/* next border in list */
};

struct IntuiText Rnd_IText1 = {
	7,0,JAM1,	/* front and back text pens, drawmode and fill byte */
	8,5,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"OKAY",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct Gadget Rnd_Gadget1 = {
	NULL,	/* next gadget */
	99,65,	/* origin XY of hit box relative to window TopLeft */
	48,17,	/* hit box width and height */
	NULL,	/* gadget flags */
	RELVERIFY,	/* activation flags */
	BOOLGADGET,	/* gadget type flags */
	(APTR)&Rnd_Border1,	/* gadget border or image to be rendered */
	NULL,	/* alternate imagery for selection */
	&Rnd_IText1,	/* first IntuiText structure */
	NULL,	/* gadget mutual-exclude long word */
	NULL,	/* SpecialInfo structure */
	1,	/* user-definable data */
	NULL	/* pointer to user-definable data */
};

#define Rnd_GadgetList1 Rnd_Gadget1

struct IntuiText Rnd_IText6 = {
	6,0,JAM1,	/* front and back text pens, drawmode and fill byte */
	198,29,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"a",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct IntuiText Rnd_IText5 = {
	1,0,JAM1,	/* front and back text pens, drawmode and fill byte */
	67,47,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"Character Choice",	/* pointer to text */
	&Rnd_IText6	/* next IntuiText structure */
};

struct IntuiText Rnd_IText4 = {
	6,0,JAM1,	/* front and back text pens, drawmode and fill byte */
	25,29,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"exciting game playing as",	/* pointer to text */
	&Rnd_IText5	/* next IntuiText structure */
};

struct IntuiText Rnd_IText3 = {
	6,0,JAM1,	/* front and back text pens, drawmode and fill byte */
	15,18,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"I think that you will have an",	/* pointer to text */
	&Rnd_IText4	/* next IntuiText structure */
};

struct IntuiText Rnd_IText2 = {
	6,0,JAM1,	/* front and back text pens, drawmode and fill byte */
	7,6,	/* XY origin relative to container TopLeft */
	NULL,	/* font pointer or NULL for default */
	"You asked for a random Character.",	/* pointer to text */
	&Rnd_IText3	/* next IntuiText structure */
};

#define Rnd_IntuiTextList1 Rnd_IText2

struct NewWindow Rnd_NewWindowStructure1 = {
	174,58,	/* window XY origin relative to TopLeft of screen */
	249,90,	/* window width and height */
	0,1,	/* detail and block pens */
	GADGETUP+CLOSEWINDOW+INACTIVEWINDOW+INTUITICKS,	/* IDCMP flags */
	ACTIVATE+NOCAREREFRESH,	/* other window flags */
	&Rnd_Gadget1,	/* first gadget in gadget list */
	NULL,	/* custom CHECKMARK imagery */
	NULL,	/* window title */
	NULL,	/* custom screen pointer */
	NULL,	/* custom bitmap */
	5,5,	/* minimum width and height */
	-1,-1,	/* maximum width and height */
	CUSTOMSCREEN	/* destination screen type */
};


/* end of PowerWindows source generation */
