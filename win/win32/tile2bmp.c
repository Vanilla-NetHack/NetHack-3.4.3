/*	SCCS Id: @(#)tile2bmp.c	3.2	95/09/06	*/
/*   Copyright (c) NetHack PC Development Team 1995                 */
/*   NetHack may be freely redistributed.  See license for details. */

/*
 * Edit History:
 *
 *	Initial Creation			M.Allison	94/01/11
 *
 */

/* #pragma warning(4103:disable) */

#include "hack.h"
#include "tile.h"
#include "win32api.h"
/* #include "pctiles.h" */

/* #define COLORS_IN_USE MAXCOLORMAPSIZE       /* 256 colors */
#define COLORS_IN_USE 16                       /* 16 colors */


extern char *FDECL(tilename, (int, int));

#if COLORS_IN_USE==16
#define MAX_X 320		/* 2 per byte, 4 bits per pixel */
#else
#define MAX_X 640
#endif	
#define MAX_Y 480

#pragma pack(1)
struct tagBMP{
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    RGBQUAD          bmaColors[COLORS_IN_USE];
#if COLORS_IN_USE==16
    uchar            packtile[MAX_Y][MAX_X];
#else
    uchar            packtile[TILE_Y][TILE_X];
#endif
} bmp;
#pragma pack()

#define BMPFILESIZE (sizeof(struct tagBMP))

FILE *tibfile2;

pixel tilepixels[TILE_Y][TILE_X];

static void FDECL(build_bmfh,(BITMAPFILEHEADER *));
static void FDECL(build_bmih,(BITMAPINFOHEADER *));
static void FDECL(build_bmptile,(pixel (*)[TILE_X]));

char *tilefiles[] = {	"../win/share/monsters.txt",
			"../win/share/objects.txt",
			"../win/share/other.txt"};

int num_colors = 0;
int tilecount;
int max_tiles_in_row = 40;
int tiles_in_row;
int filenum;
int initflag;
int yoffset,xoffset;
char bmpname[128];
FILE *fp;

int
main(argc, argv)
int argc;
char *argv[];
{
	int i;

	if (argc != 2) {
		Fprintf(stderr, "usage: tile2bmp outfile.bmp\n");
		exit(EXIT_FAILURE);
	} else
		strcpy(bmpname, argv[1]);

#ifdef OBSOLETE
	bmpfile2 = fopen(NETHACK_PACKED_TILEFILE, WRBMODE);
	if (bmpfile2 == (FILE *)0) {
		Fprintf(stderr, "Unable to open output file %s\n",
				NETHACK_PACKED_TILEFILE);
		exit(EXIT_FAILURE);
	}
#endif

	tilecount = 0;
	xoffset = yoffset = 0;
	initflag = 0;
	filenum = 0;
	fp = fopen(bmpname,"wb");
	if (!fp) {
	    printf("Error creating tile file %s, aborting.\n",bmpname);
	    exit(1);
	}
	while (filenum < 3) {
		if (!fopen_text_file(tilefiles[filenum], RDTMODE)) {
			Fprintf(stderr,
				"usage: tile2bmp (from the util directory)\n");
			exit(EXIT_FAILURE);
		}
		num_colors = colorsinmap;
		if (num_colors > 62) {
			Fprintf(stderr, "too many colors (%d)\n", num_colors);
			exit(EXIT_FAILURE);
		}
		if (!initflag) {
		    build_bmfh(&bmp.bmfh);
		    build_bmih(&bmp.bmih);
		    memset(&bmp.packtile,0,
				(MAX_X * MAX_Y * sizeof(uchar)));
		    for (i = 0; i < num_colors; i++) {
			    bmp.bmaColors[i].rgbRed = ColorMap[CM_RED][i];
			    bmp.bmaColors[i].rgbGreen = ColorMap[CM_GREEN][i];
			    bmp.bmaColors[i].rgbBlue = ColorMap[CM_BLUE][i];
			    bmp.bmaColors[i].rgbReserved = 0;
		    }
		    initflag = 1;
		}
				
		while (read_text_tile(tilepixels)) {
			build_bmptile(tilepixels);
			tilecount++;
			xoffset += (TILE_X / 2);
			if (xoffset >= MAX_X) {
				yoffset += TILE_Y;
				xoffset = 0;
			}
		}
		(void) fclose_text_file();
		++filenum;
	}
	fwrite(&bmp, sizeof(bmp), 1, fp);
	fclose(fp);
	Fprintf(stderr, "Total of %d tiles written to %s.\n",
		tilecount, bmpname);

	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}


static void
build_bmfh(pbmfh)
BITMAPFILEHEADER *pbmfh;
{
	pbmfh->bfType = (UINT)0x4D42;
	pbmfh->bfSize = (DWORD)BMPFILESIZE;
	pbmfh->bfReserved1 = (UINT)0;
	pbmfh->bfReserved2 = (UINT)0;
	pbmfh->bfOffBits = sizeof(bmp.bmfh) + sizeof(bmp.bmih) +
			   (COLORS_IN_USE * sizeof(RGBQUAD));
}

static void
build_bmih(pbmih)
BITMAPINFOHEADER *pbmih;
{
	pbmih->biSize = (DWORD) sizeof(bmp.bmih);
#if COLORS_IN_USE==16
	pbmih->biWidth = (LONG) MAX_X * 2;
#else
	pbmih->biWidth = (LONG) MAX_X;
#endif
	pbmih->biHeight = (LONG) MAX_Y;
	pbmih->biPlanes = (WORD) 1;
#if COLORS_IN_USE==16
	pbmih->biBitCount = (WORD) 4;
#else
	pbmih->biBitCount = (WORD) 8;
#endif
	pbmih->biCompression = (DWORD) BI_RGB;
	pbmih->biSizeImage = (DWORD)0;
	pbmih->biXPelsPerMeter = (LONG)0;
	pbmih->biYPelsPerMeter = (LONG)0;
	pbmih->biClrUsed = (DWORD)0;
	pbmih->biClrImportant = (DWORD)0;
}

static void
build_bmptile(pixels)
pixel (*pixels)[TILE_X];
{
	int cur_x, cur_y, cur_color;
	int x,y;

	for (cur_y = 0; cur_y < TILE_Y; cur_y++) {
		for (cur_x = 0; cur_x < TILE_X; cur_x++) {
		    for (cur_color = 0; cur_color < num_colors; cur_color++) {
				if (ColorMap[CM_RED][cur_color] ==
						pixels[cur_y][cur_x].r &&
				    ColorMap[CM_GREEN][cur_color] ==
						pixels[cur_y][cur_x].g &&
				    ColorMap[CM_BLUE][cur_color] ==
						pixels[cur_y][cur_x].b)
					break;
		    }
		    if (cur_color >= num_colors)
				Fprintf(stderr, "color not in colormap!\n");
		    y = (MAX_Y - 1) - (cur_y + yoffset);
#if COLORS_IN_USE==16
		    x = (cur_x / 2) + xoffset;
		    bmp.packtile[y][x] =
			   cur_x%2 ? (uchar)(bmp.packtile[y][x] | cur_color) :
			             (uchar)(cur_color<<4);
#else
		    bmp.packtile[y][x] =cur_color;
#endif
		}
	}
}
