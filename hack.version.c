/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */

#include	"date.h"

doversion(){
	pline("%s 1.0.1 - last edit %s.",
#ifdef QUEST
		"Quest"
#else
		"Hack"
#endif QUEST
	, datestring);
	return(0);
}
