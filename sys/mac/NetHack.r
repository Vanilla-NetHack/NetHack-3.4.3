#ifndef THINK_Rez
INCLUDE "NetHack.rsrc";			/* MENU, ALRT, WIND, ICN#, etc */
INCLUDE "Sounds.rsrc";			/* Instruments as snd resources */
#endif

#include "Types.r"
#include "SysTypes.r"
#include "BalloonTypes.r"	    /* Mac resource type definitions */

#ifdef THINK_Rez
read 'TEXT' (1000,"Help") 	 "help";
read 'TEXT' (1001,"WizHelp") "wizhelp";
read 'TEXT' (1002,"OptHelp") "opthelp";
read 'TEXT' (1004,"MacHelp") "MacHelp";
read 'TEXT' (1005,"HH") 	 "hh";
read 'TEXT' (1006,"History") "history";
read 'TEXT' (1007,"License") "license";
read 'TEXT' (1008,"News")	 "News";
read 'TEXT' (1009,"Options") "options";
#else	/* MPW rez */
read 'TEXT' (1000,"Help") 	 $$Shell("Dat") "Help";
read 'TEXT' (1001,"WizHelp") $$Shell("Dat") "WizHelp";
read 'TEXT' (1002,"OptHelp") $$Shell("Dat") "OptHelp";
/*
read 'TEXT' (1003,"CmdHelp") $$Shell("Dat") "CmdHelp";
*/
read 'TEXT' (1004,"MacHelp") $$Shell("MacDir") "MacHelp";

read 'TEXT' (1005,"HH") 	 $$Shell("Dat") "HH";
read 'TEXT' (1006,"History") $$Shell("Dat") "History";
read 'TEXT' (1007,"License") $$Shell("Dat") "License";

read 'TEXT' (1008,"News")	 $$Shell("MacDir") "News";
read 'TEXT' (1009,"Options") $$Shell("ObjDir") "Options";

/*
read 'TEXT' (1010,"Rumors")  $$Shell("ObjDir") "Rumors";
read 'TEXT' (1011,"Data")    $$Shell("ObjDir") "Data";
*/

/* Think C generates a SIZE resource into NetHack.rsrc, MPW needs this */
resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreAppDiedEvents,
	is32BitCompatible,
	notHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	1500 * 1024,		/* recommended */
	1000 * 1024		/* absolute minimum, to be determined */
};
#endif

/* System 7 help balloon information */
resource 'hfdr' (-5696, purgeable) { 
     HelpMgrVersion, hmDefaultOptions, 0, 0, /* header information */ 
	    { HMSTRResItem { /* use 'STR ' resource 2000 */ 2000 } 
	} 
};

resource 'STR ' (2000, purgeable) { /* Help message for app icon */
   "NetHack 3.1\nThis is the famous Dungeons and Dragons*-like game ported to the Macintosh." 
};

resource 'STR '(-16396, purgeable) {	/* Will be copied to the saved file. */
	"NetHack"							/* See Inside Mac VI, page 9-21.	 */
};


/* Mac error decodes : 2000 - err# for common errors. Add your favorites below */

resource 'STR ' (2034) { "the disk is full"  };
resource 'STR ' (2036) { "there was an I/O error"  };
resource 'STR ' (2043) { "a file is missing"  };
resource 'STR ' (2044) { "the disk is write-protected"  };
resource 'STR ' (2047) { "the file is busy"  };
resource 'STR ' (2049) { "the file is already open"  };
resource 'STR ' (2108) { "there is not enough memory"  };
resource 'STR ' (2192) { "a resource is missing"  };
