/*   SCCS Id: @(#)pctiles.c   3.2     95/07/31		     */
/*   Copyright (c) NetHack PC Development Team 1993, 1994           */
/*   NetHack may be freely redistributed.  See license for details. */
/*                                                                  */
/*
 * pctiles.c - PC Graphical Tile Support Routines
 *                                                  
 *Edit History:
 *     Initial Creation              M. Allison      93/10/30
 *
 */

#include "hack.h"

#ifdef USE_TILES

#ifdef __GO32__
#include <unistd.h>
#endif

# if defined(_MSC_VER)
#  if _MSC_VER >= 700
#pragma warning(disable:4018)	/* signed/unsigned mismatch */
#pragma warning(disable:4127)	/* conditional expression is constant */
#pragma warning(disable:4131)	/* old style declarator */
#pragma warning(disable:4309)	/* initializing */
#  endif
#include <conio.h>
# endif

#include "pcvideo.h"
#include "tile.h"
#include "pctiles.h"

STATIC_VAR FILE *tilefile;
STATIC_VAR FILE *tilefile_O;
extern short glyph2tile[];              /* in tile.c (made from tilemap.c) */

# ifdef OVLB

/*
 * Read the header/palette information at the start of the 
 * NetHack.tib file.
 *
 * There is 1024 bytes (1K) of header information
 * at the start of the file, including a palette.
 *
 */
int ReadTileFileHeader(tibhdr, filestyle)
struct tibhdr_struct *tibhdr;
boolean filestyle;
{
	FILE *x;
	x = filestyle ? tilefile_O : tilefile;
	if (fseek(x,0L,SEEK_SET)) {
		return 1;
	} else {
	  	fread(tibhdr, sizeof(struct tibhdr_struct), 1, x);
	}
	return 0;
} 

/*
 * Open the requested tile file.
 *
 * NetHack1.tib file is a series of
 * 'struct planar_tile_struct' structures, one for each
 * glyph tile.
 *
 * NetHack2.tib file is a series of
 * char arrays [TILE_Y][TILE_X] in dimensions, one for each
 * glyph tile.
 *
 * There is 1024 bytes (1K) of header information
 * at the start of each .tib file. The first glyph tile starts at
 * location 1024.
 *
 */
int
OpenTileFile(tilefilename, filestyle)
char *tilefilename;
boolean filestyle;
{
	FILE *x;

	if (filestyle) { 
		tilefile_O = fopen(tilefilename,"rb");
		if (tilefile_O == (FILE *)0) return 1;
	} else {
		tilefile = fopen(tilefilename,"rb");
		if (tilefile == (FILE *)0) return 1;
	}
	return 0;
}

void
CloseTileFile(filestyle)
boolean filestyle;
{
	fclose(filestyle ? tilefile_O : tilefile);   
}
# endif /* OVLB      */

# ifdef OVL0
/* This routine retrieves the requested NetHack glyph tile
 * from the planar style binary .tib file.
 * This is currently done 'on demand', so if the player
 * is running without a disk cache (ie. smartdrv) operating,
 * things can really be slowed down.  We don't have any
 * base memory under MSDOS, in which to store the pictures.
 *
 * Todo: Investigate the possibility of loading the glyph
 *       tiles into extended or expanded memory using
 *       the MSC virtual memory routines.
 *
 * Under an environment like djgpp, it should be possible to
 * read the entire set of glyph tiles into a large
 * array of 'struct planar_cell_struct' structures at
 * game initialization time, and recall them from the array
 * as needed.  That should speed things up (at the cost of
 * increasing the memory requirement - can't have everything).
 *
 */
#  ifdef PLANAR_FILE
int ReadPlanarTileFile(tilenum,gp)
int tilenum;
struct planar_cell_struct *gp;
{
	long fpos;
	
	fpos = ((long)(tilenum) * (long)sizeof(struct planar_cell_struct)) +
		(long)TIBHEADER_SIZE;
	if (fseek(tilefile,fpos,SEEK_SET)) {
		return 1;
	} else {
	  	fread(gp, sizeof(struct planar_cell_struct), 1, tilefile);
	}
	return 0;
}
int ReadPlanarTileFile_O(tilenum,gp)
int tilenum;
struct overview_planar_cell_struct *gp;
{
	long fpos;
	
	fpos = ((long)(tilenum) * 
		(long)sizeof(struct overview_planar_cell_struct)) +
		(long)TIBHEADER_SIZE;
	if (fseek(tilefile_O,fpos,SEEK_SET)) {
		return 1;
	} else {
	  	fread(gp, sizeof(struct overview_planar_cell_struct), 
			1, tilefile_O);
	}
	return 0;
}
#  endif

#  ifdef PACKED_FILE
int ReadPackedTileFile(tilenum,pta)
int tilenum;
char (*pta)[TILE_X];
{
	long fpos;
	
	fpos = ((long)(tilenum) * (long)(TILE_Y * TILE_X) +
		(long)TIBHEADER_SIZE;
	if (fseek(tilefile,fpos,SEEK_SET)) {
		return 1;
	} else {
	  	fread(pta, (TILE_Y * TILE_X), 1, tilefile);
	}
	return 0;
}
#  endif
# endif /* OVL0 */
#endif /* USE_TILES */

/* pctiles.c */
