/******************************************************
 *                                                    *
 *        makedefs.r                                  *
 *                                                    *
 *        Resource file for makedefs application      *
 *                                                    *
 *        Copyright 1989 by Johnny Lee                *
 *        NetHack may be freely redistributed.        *
 *	  See license for details. 		      *
 *                                                    *
 ******************************************************/

#include	"Types.r"

resource 'DLOG' (200) {
	{40, 40, 200, 320},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	200,
	"Which option?"
};

resource 'DLOG' (128, "FindFile") {
	{40, 80, 100, 300},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	128,
	"FindFile"
};

resource 'DITL' (200) {
	{ /* array DITLarray: 9 elements */
		/* [1] */
		{116, 51, 136, 111},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{116, 158, 136, 218},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{15, 15, 35, 75},
		RadioButton {
			enabled,
			"data"
		},
		/* [4] */
		{15, 95, 35, 155},
		RadioButton {
			enabled,
			"date"
		},
		/* [5] */
		{40, 95, 60, 180},
		RadioButton {
			enabled,
			"permonst"
		},
		/* [6] */
		{15, 180, 35, 250},
		RadioButton {
			enabled,
			"rumors"
		},
		/* [7] */
		{40, 15, 60, 75},
		RadioButton {
			enabled,
			"traps"
		},
		/* [8] */
		{65, 15, 85, 125},
		RadioButton {
			enabled,
			"object names"
		},
		/* [9] */
		{65, 145, 85, 270},
		RadioButton {
			enabled,
			"monst resource"
		}
	}
};

resource 'DITL' (128) {
	{ /* array DITLarray: 1 elements */
		/* [1] */
		{4, 8, 80, 212},
		StaticText {
			disabled,
			"Please locate^0 file^1 ^2\nor press Cance"
			"l to abort."
		}
	}
};
