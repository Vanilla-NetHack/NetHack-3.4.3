/******************************************************************************
*									      *
*			  EXE header list and modify			      *
*									      *
*			 by Pierre Martineau, 90/05/20			      *
*									      *
*				 Version 1.1				      *
*									      *
*			  Placed in the public domain			      *
*									      *
******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define BOOLEAN int
#define TRUE	1
#define FALSE	0

FILE *wrkfile;
long min, max, stk;
BOOLEAN listflg = FALSE;
BOOLEAN minflg = FALSE;
BOOLEAN maxflg = FALSE;
BOOLEAN stkflg = FALSE;

struct exehdr {
unsigned signature;
unsigned mod512;
unsigned pages;
unsigned relocitems;
unsigned headerparas;
unsigned minalloc;
unsigned maxalloc;
unsigned ss;
unsigned sp;
unsigned checksum;
unsigned ip;
unsigned cs;
unsigned relocptr;
unsigned ovlnum;
} exehdr_area;

main(argc, argv)
int argc;
char *argv[];
{
char *dot, *slash;
char fname[128];
char *args;
int i;
long offset, oldstk;

    printf("EXE list and modify V1.1, by Pierre Martineau, 90/05/20.\n");
    printf("This program is public domain and may be freely distributed.\n");

    if ((argc < 2) || (argc > 6)) {
	usage();
	return;
    }

/*  Extract filename from first argumemt  */

    strcpy(fname, argv[1]);
    dot = strrchr(fname, '.');
    slash = strrchr(fname, '\\');
    if ((dot == NULL) || (slash > dot))
	strcat(fname, ".exe");

    if ((wrkfile = fopen(fname, "r+b")) == NULL) {
	printf("\nCouldn't open file %s\n", fname);
	return;
    }

/*  Process any remaining arguments  */

    if (argc == 2)
	listflg = TRUE;
    else {
	i = 2;
	while (argc-- > 2) {
	    args = argv[i];
	    if ((args[0] != '-') && (args[0] != '/')) {
		printf("\nInvalid switch in paramater %s!\n", argv[i]);
		usage();
		return;
	    }
	    args++;
	    if (strnicmp(args, "min", 3) == 0) {
		args += 3;
		min = atol(args);
		minflg = TRUE;
	    }
	    else if (strnicmp(args, "max", 3) == 0) {
		args += 3;
		max = atol(args);
		maxflg = TRUE;
	    }
	    else if (strnicmp(args, "stk", 3) == 0) {
		args += 3;
		stk = atol(args);
		stkflg = TRUE;
	    }
	    else if (strnicmp(args, "v", 1) == 0)
		listflg = TRUE;
	    else {
		printf("\nInvalid paramater %s!\n", argv[i]);
		usage();
		return;
	    }
	    i++;
	}
    }

    fread(&exehdr_area, sizeof (struct exehdr), 1, wrkfile);
    if (exehdr_area.signature != 0x5a4d) {
	printf("\nNot an EXE file!\n");
	return;
    }
    while(!feof(wrkfile)) {
	if (listflg)
	    show_hdr();
	if ((minflg || maxflg || stkflg) && (exehdr_area.ovlnum == 0) && (exehdr_area.signature == 0x5a4d)) {
	    if (minflg)
		exehdr_area.minalloc = min;
	    if (maxflg)
		exehdr_area.maxalloc = max;
	    if (stkflg) {
		oldstk = exehdr_area.sp;
		exehdr_area.sp = stk;
		if (!minflg) {
		    exehdr_area.minalloc += ((stk - oldstk) / 16);
		    printf("\nAdjusting size of minalloc!\n");
		}
	    }
	    fseek(wrkfile, ftell(wrkfile) - sizeof (struct exehdr), SEEK_SET);
	    fwrite(&exehdr_area, sizeof (struct exehdr), 1, wrkfile);
	    if (ferror(wrkfile)) {
		printf("Write error while trying to update header!\n");
		fclose(wrkfile);
		return;
	    }
	}
	offset = exehdr_area.pages;
	offset *= 512L;
	offset -= sizeof(struct exehdr);
	fseek(wrkfile, offset, SEEK_CUR);
	fread(&exehdr_area, sizeof (struct exehdr), 1, wrkfile);
	if (ferror(wrkfile)) {
	    printf("Read error while trying to get a header!\n");
	    fclose(wrkfile);
	    return;
	}
    }
    fclose(wrkfile);
}

show_hdr()
{
long lsize;

    lsize = exehdr_area.pages;
    if (exehdr_area.mod512 != 0)
	lsize--;
    lsize *= 512L;
    lsize += exehdr_area.minalloc * 16;
    lsize += exehdr_area.mod512;
    lsize -= exehdr_area.headerparas * 16;

    printf("\nOverlay: %d\n", exehdr_area.ovlnum);
    printf("--------\n");
    printf("Size (512 byte pages)\t-%6x\t\t%6u\n", exehdr_area.pages, exehdr_area.pages);
    printf("Remainder (last page)\t-%6x\t\t%6u\n", exehdr_area.mod512, exehdr_area.mod512);
    printf("Header size (in paras)\t-%6x\t\t%6u\n", exehdr_area.headerparas, exehdr_area.headerparas);
    printf("Minalloc (in paras)\t-%6x\t\t%6u\n", exehdr_area.minalloc, exehdr_area.minalloc);
    printf("Maxalloc (in paras)\t-%6x\t\t%6u\n", exehdr_area.maxalloc, exehdr_area.maxalloc);
    printf("Load size (in bytes)\t-%6lx\t\t%6lu\n", lsize, lsize);
    printf("Relocation items\t-%6x\t\t%6u\n", exehdr_area.relocitems, exehdr_area.relocitems);
    printf("Relocation table offset\t-%6x\t\t%6u\n", exehdr_area.relocptr, exehdr_area.relocptr);
    printf("Checksum\t\t-%6x\t\t%6u\n", exehdr_area.checksum, exehdr_area.checksum);
    printf("Initial CS:IP\t\t-  %04x:%04x\n", exehdr_area.cs, exehdr_area.ip);
    printf("Initial SS:SP\t\t-  %04x:%04x\n", exehdr_area.ss, exehdr_area.sp);
}

usage()
{
    printf("\nUsage: exesmurf exe_file [/v] [/min#####] [/max#####] [/stk#####]\n");
    printf("       where: min   = minalloc\n");
    printf("              max   = maxalloc\n");
    printf("              stk   = stack size\n");
    printf("              ##### = decimal number of paragraphs.\n");
}
