/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. */

#include	"date.h"

doversion(){
	pline("%s 1.0 preliminary version - last edit %s.",
#ifdef QUEST
		"Quest"
#else
		"Hack"
#endif QUEST
	, datestring);
	return(0);
}
