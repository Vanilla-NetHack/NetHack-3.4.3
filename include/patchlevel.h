/*
 *  Patch 1, July 31, 1989
 *  add support for Atari TOS (courtesy Eric Smith) and Andrew File System
 *	(courtesy Ralf Brown)
 *  include the uuencoded version of termcap.arc for the MSDOS versions that
 *	was included with 2.2 and 2.3
 *  make a number of simple changes to accommodate various compilers
 *  fix a handful of bugs, and do some code cleaning elsewhere
 *  add more instructions for new environments and things commonly done wrong
 */

/*
 *  Patch 2, August 16, 1989
 *  add support for OS/2 (courtesy Timo Hakulinen)
 *  add a better makefile for MicroSoft C (courtesy Paul Gyugyi)
 *  more accomodation of compilers and preprocessors
 *  add better screen-size sensing
 *  expand color use for PCs and introduce it for SVR3 UNIX machines
 *  extend '/' to multiple identifications
 *  allow meta key to be used to invoke extended commands
 *  fix various minor bugs, and do further code cleaning
 */

/*
 *  Patch 3, September 6, 1989
 *  add war hammers and revise object prices
 *  extend prototypes to ANSI compilers in addition to the previous MSDOS ones
 *  move object-on-floor references into functions in preparation for planned
 *	data structures to allow faster access and better colors
 *  fix some more bugs, and extend the portability of things added in earlier
 *	patches
 */

/*
 *  Patch 4, September 27, 1989
 *  add support for VMS (courtesy David Gentzel)
 *  move monster-on-floor references into functions and implement the new
 *	lookup structure for both objects and monsters
 *  extend the definitions of objects and monsters to provide "living color"
 *	in the dungeon, instead of a single monster color
 *  ifdef varargs usage to satisfy ANSI compilers
 *  standardize on the color 'gray'
 *  assorted bug fixes
 */

/*
 *  Patch 5, October 15, 1989
 *  add support for Macintosh OS (courtesy Johnny Lee)
 *  fix annoying dependency loop via new color.h file
 *  allow interruption while eating -- general handling of partially eaten food
 *  smarter treatment of iron balls (courtesy Kevin Darcy)
 *  a handful of other bug fixes
 */

/*
 *  Patch 6, November 19, 1989
 *  add overlay support for MS-DOS (courtesy Pierre Martineau, Stephen
 *	Spackman, and Norm Meluch)
 *  refine Macintosh port
 *  different door states show as different symbols (courtesy Ari Huttunen)
 *  smarter drawbridges (courtesy Kevin Darcy)
 *  add CLIPPING and split INFERNO off HARD
 *  further refine eating code wrt picking up and resumption
 *  make first few levels easier, by adding :x monsters and increasing initial
 *	attribute points and hitting probability
 *  teach '/' about configurable symbols
 */

/*
 *  Patch 7, February 19, 1990
 *  refine overlay support to handle portions of .c files through OVLx
 *	(courtesy above plus Kevin Smolkowski)
 *  update and extend Amiga port and documentation (courtesy Richard Addison,
 *	Jochen Erwied, Mark Gooderum, Ken Lorber, Greg Olson, Mike Passaretti,
 *	and Gregg Wonderly)
 *  refine and extend Macintosh port and documentation (courtesy Johnny Lee,
 *	Kevin Sitze, Michael Sokolov, Andy Swanson, Jon Watte, and Tom West)
 *  refine VMS documentation
 *  continuing ANSIfication, this time of const usage
 *  teach '/' about differences within monster classes
 *  smarter eating code (yet again), death messages, and treatment of
 *	non-animal monsters, monster unconsciousness, and naming
 *  extended version command to give compilation options
 *  and the usual bug fixes and hole plugs
 */

/*
 *  Patch 8, June 3, 1990
 *  further debug and refine Macintosh port
 *  refine the overlay manager, rearrange the OVLx breakdown for better
 *	efficiency, rename the overlay macros, and split off the overlay
 *	instructions to Install.ovl
 *  introduce NEARDATA for better Amiga efficiency
 *  support for more VMS versions (courtesy Joshua Delahunty and Pat Rankin)
 *  more const fixes
 *  better support for common graphics (DEC VT and IBM)
 *  and a number of simple fixes and consistency extensions
 */

#define PATCHLEVEL	8
