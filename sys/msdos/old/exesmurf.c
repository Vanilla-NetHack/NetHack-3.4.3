/*	SCCS Id: @(#)exesmurf.c	 3.1	 91/01/29			  */
/* Copyright (c) Pierre Martineau and Stephen Spackman 1991, 1992, 1993.  */
/* NetHack may be freely redistributed.  See license for details.	  */

/******************************************************************************
*                                                                             *
*                         EXE header list and modify                          *
*                                                                             *
*                        by Pierre Martineau, 91/01/29                        *
*                                                                             *
*                                Version 1.2                                  *
*                                                                             *
>*****************************************************************************<
* Modified (stephen@estragon.uchicago.edu):                                   *
* 1990oct23 sps Overlay splitter-outer first cut                              *
*        31     Error handling; some #defines                                 *
*     nov01     /l                                                            *
*   91jan29     Changed default overlay file names to conform to ovlmgr 30a0  *
******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/** parameters ***************************************************************/
#define MAXFILENAME 128   /* Probably overkill - theoretical limit is 80     */
#define NPARTS	    36	  /* Maximum # of overlay files (excluding root .EXE)*/
#define COPYBUFSIZE 32768 /* Fair sized buffer for file copy                 */
#define BAKEXT      ".BAK"/* Extension for .exe backups                      */
#define OVLEXT      ".OVL"/* Default extension for overlay files             */
/* #define MANYZEROES */  /* Old style default: foo00001.ovl, not foo0.ovl   */
/*****************************************************************************/

#define BOOLEAN int
#define TRUE    1
#define FALSE   0

int sstrccnt(register char const *s, register char c)
  { int n = 0;

    while (*s) if (*s++ == c) n++;
    return n;
  }

FILE *wrkfile, *outfile;
long min, max, stk;
BOOLEAN listflg = FALSE;
BOOLEAN verbose = FALSE;
BOOLEAN minflg = FALSE;
BOOLEAN maxflg = FALSE;
BOOLEAN stkflg = FALSE;

int column = 0;

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
char fname[MAXFILENAME], oname[MAXFILENAME], zname[MAXFILENAME];
char *jname = NULL;
char *args;
int i;
long offset, oldstk;
unsigned nparts = 0, part = 0, partstart[NPARTS + 2];

    printf("EXE list and modify V1.1s, by Pierre Martineau, 90/05/20.\n");
    printf("This program may be freely distributed.\n");

	if ((argc < 2) || (argc > NPARTS + 2)) {
        usage();
        return;
    }

/*  Process any remaining arguments  */

    if (argc == 2) {
        listflg = TRUE;
        verbose = TRUE; /* ??? */
    }
    else {
        i = 2;
        while (argc-- > 2) {
            args = argv[i];
	    if ('0' <= args[0] && args[0] <= '9') { /* File split request */
			if (nparts >= NPARTS) {
			printf("\nToo many .OVL files requested (max. %d)\n", NPARTS);
		    usage();
		    return;
		}
		else if (!atoi(args)) {
		    printf("\nCan't relocate the root overlay (#0)\n");
		    usage();
		    return;
		}
		else if (nparts && partstart[nparts - 1] >= atoi(args)) {
		    printf("\nOverlay starts must be in ascending order\n");
		    usage();
		    return;
		}
		partstart[nparts++] = atoi(args);
	    } else {
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
		else if (strnicmp(args, "v", 1) == 0) {
		    listflg = TRUE;
                    verbose = TRUE;
                }
                else if (strnicmp(args, "l", 1) == 0)
                    listflg = TRUE;
                else if (strnicmp(args, "p", 1) == 0) {
                    args++;
                    jname = args;
                }
		else {
		    printf("\nInvalid paramater %s!\n", argv[i]);
		    usage();
		    return;
		}
            }
            i++;
        }
    }

/*  Extract filename from first argumemt  */

    strcpy(fname, argv[1]);
    dot = strrchr(fname, '.');
    slash = strrchr(fname, '\\');
    if ((dot == NULL) || (slash > dot))
        strcat(fname, ".exe");

    if (nparts) {
	strcpy(oname,fname);
	*strrchr(fname, '.') = '\0';
	strcat(fname,BAKEXT);
	if (!stricmp(oname,fname)) {
	    printf(
                "\nI refuse to split a file with extension "BAKEXT": %s\n",
                oname
            );
	    return;
	}
        if (!jname || nparts > 1 && !sstrccnt(jname, '?')) {
            char ext[5];
            char *t;

            if (!jname) {
                strcpy(zname, oname);
                *strrchr(zname, '.') = '\0';
                strcpy(ext, OVLEXT);
            } else {
                if (strrchr(jname, '.') &&
                     (!strrchr(jname, '\\') ||
                         strrchr(jname, '.') > strrchr(jname, '\\')
                     )
                ) {
                    strncpy(ext, strrchr(jname, '.'), sizeof(ext));
                    ext[sizeof(ext) - 1] = '\0';
                    strncpy(zname, jname, strrchr(jname, '.') - jname);
                    zname[strrchr(jname, '.') - jname] = '\0';
                } else {
                    strcpy(zname, jname);
                    strcpy(ext, OVLEXT);
                }
            }
            t = strrchr(zname, '\\') ? strrchr(zname, '\\') + 1:
                strrchr(zname, ':') ? strrchr(zname, ':') + 1:
                zname;
            if (strlen(t) >= 8)
                t[7] = '\0';
#if defined(MANYZEROES)
	    while (strlen(t) < 8)
#endif
	      strcat(t, "?");
            strcat(zname, ext);
            jname = zname;
        }
	if (rename(oname,fname)) { /* This assumes oldname, newname.
				      There's some confusion. OK for TC2.0 */
	    printf("\nCouldn't rename (original) %s to %s\n", oname, fname);
	    return;
	}
	if ((outfile = fopen(oname, "wb")) == NULL) {
            printf("\nCouldn't create file %s\n",oname);
            return;
        }
    }

    if ((wrkfile = fopen(fname, "r+b")) == NULL) {
        printf("\nCouldn't open file %s\n", fname);
        return;
    }

    fread(&exehdr_area, sizeof (struct exehdr), 1, wrkfile);
    if (exehdr_area.signature != 0x5a4d) {
        printf("\nNot an EXE file!\n");
        return;
    }

    while(!feof(wrkfile)) {
        if (nparts) {
	    if (exehdr_area.ovlnum == partstart[part]) {
	         fclose(outfile);
                 {
                     int p = part + 1;
                     strcpy(oname, jname);
                     while (sstrccnt(oname, '?') > 1) {
                         *strrchr(oname, '?') = '0' + p % 10;
                         p /= 10;
                     }
                     *strchr(oname, '?') = (p > 9 ? 'a' - 10 : '0') + p;
                 }
                 part++;
		 if ((outfile = fopen(oname, "wb")) == NULL) {
                     printf("\nCan't open file %s\n", oname);
                     return;
                 }
	    }
            fwrite(&exehdr_area, sizeof (struct exehdr), 1, outfile);
            if (ferror(outfile)) {
                printf("\nWrite error while moving overlay header in %s\n", oname);
                return;
            }
	}
        if (listflg)
            show_hdr();
        else if (nparts)
            printf("[overlay %d]\r", exehdr_area.ovlnum); /* Keep talking... */
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
            fseek(nparts ? outfile : wrkfile, ftell(wrkfile) - sizeof (struct exehdr), SEEK_SET);
            fwrite(&exehdr_area, sizeof (struct exehdr), 1, nparts ? outfile : wrkfile);
            if (ferror(nparts ? outfile : wrkfile)) {
                printf("Write error while trying to update header!\n");
                fclose(nparts ? outfile : wrkfile);
                return;
            }
        }
        offset = exehdr_area.pages;
        offset *= 512L;
        offset -= sizeof(struct exehdr);
        if (nparts) { /* Copy the stuff across */
	    static char buffer[COPYBUFSIZE];
	    while (offset > sizeof(buffer)) {
	         fread(buffer, sizeof(buffer), 1, wrkfile);
                 if (ferror(wrkfile)) {
                     printf("\nRead error in overlay body\n");
                     return;
                 }
		 fwrite(buffer, sizeof(buffer), 1, outfile);
                 if (ferror(outfile)) {
                     printf("\nWrite error moving overlay body, file %s\n", oname);
                     return;
                 }
		 offset -= sizeof(buffer);
	    }
	    fread(buffer, (unsigned)offset, 1, wrkfile);
            if (ferror(wrkfile)) {
                printf("\nRead error in overlay body\n");
                return;
            }
            fwrite(buffer, (unsigned)offset, 1, outfile);
            if (ferror(outfile)) {
                printf("\nWrite error moving overlay body, file %s\n", oname);
                return;
            }
        } else fseek(wrkfile, offset, SEEK_CUR);
        fread(&exehdr_area, sizeof (struct exehdr), 1, wrkfile);
        if (ferror(wrkfile)) {
            printf("Read error while trying to get a header!\n");
            fclose(wrkfile);
            return;
        }
    }
    if (nparts) {
        fclose(outfile);
        if (!listflg) printf("                    \r");
    }
    fclose(wrkfile);
    if (listflg && !verbose && column % 4) printf("\n");
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

    if (verbose) {
        printf("\nOverlay: %d\n", exehdr_area.ovlnum);
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
    } else {
        if (!exehdr_area.ovlnum) {
            printf("\nOverlay: %d\n", exehdr_area.ovlnum);
            printf("Minalloc (in paras)\t-%6x\t\t%6u\n", exehdr_area.minalloc, exehdr_area.minalloc);
            printf("Maxalloc (in paras)\t-%6x\t\t%6u\n", exehdr_area.maxalloc, exehdr_area.maxalloc);
            printf("Stored size (in bytes)\t-%6lx\t\t%6lu\n", exehdr_area.pages * 512L, exehdr_area.pages * 512L);
            printf("Load size (in bytes)\t-%6lx\t\t%6lu\n", lsize, lsize);
            printf("Initial CS:IP, SS:SP\t-  %04x:%04x\t  %04x:%04x\n", exehdr_area.cs, exehdr_area.ip, exehdr_area.ss, exehdr_area.sp);
	} else {
	    static bis = 0;
	    if (!bis++)
                printf("\nOvl StrdSz LoadSz | Ovl StrdSz LoadSz | Ovl StrdSz LoadSz | Ovl StrdSz LoadSz\n");
            printf("%3d:%6lu %6lu%s", exehdr_area.ovlnum, exehdr_area.pages * 512L, lsize, ++column % 4 ? " | " : "\n");
        }
    }
}

usage()
{
    printf("\nUsage: exesmurf exe_file [/l] [/v] [/min#####] [/max#####] [/stk#####]\n");
    printf("                [n1 n2...nn] [/p????????.???]\n");
    printf("       where: min   = minalloc\n");
    printf("              max   = maxalloc\n");
    printf("              stk   = stack size\n");
    printf("              ##### = decimal number of paragraphs\n");
    printf("              ni    = overlay starting each new .OVL file, n1 < n2 <...< nn\n");
    printf("              p     = DOS filename, maybe with ?s, for overlay files.\n");
}
