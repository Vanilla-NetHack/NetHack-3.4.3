/******************************************************************************
*									      *
*			File Splitter and Re-assembler			      *
*									      *
*			by Pierre Martineau, 90/05/20			      *
*									      *
*				 Version 1.1				      *
*									      *
*			 Placed in the public domain			      *
*									      *
******************************************************************************/

#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

FILE *infile, *outfile;
char fname[16];
char chunk_name[16];
int extent = 0;
long hunk_size;
unsigned buflen = 0x8000;
char *buf = 0;

main(argc, argv)
int argc;
char *argv[];
{
struct stat stat_buf;
char *cptr;

    printf("File Splitter and Re-assembler V1.1, by Pierre Martineau, 90/05/20.\n");
    printf("This program is public domain and may be freely distributed.\n");
    if ((argc < 2) || (argc > 3)) {
	printf("\nUsage: splitf file_to_split [chunk_size]\n");
	printf("       If chunk_size isn't specified, the file will be split\n");
	printf("       into two files of (approximately) equal size.\n\n");
	printf("       splitf dest_file /r\n");
	printf("       /r will re-assemble the parts back into the whole\n");
	printf("       specified by dest_file.\n");
	return;
    }

/*  Extract filename from first argumemt  */

    if ((cptr = strrchr(argv[1], '\\')) == NULL)
	cptr = argv[1];
    else
	cptr++;
    strcpy(fname, cptr);
    if ((cptr = strchr(fname, '.')) != NULL)
	*++cptr = '\000';
    else
	strcat(fname, ".");

    if ((argc == 3) && ((strcmpi(argv[2], "-r") == 0) || (strcmpi(argv[2], "/r") == 0))) {
	getbuf();
	printf("\nRe-assembling %s ...\n\n", argv[1]);
	copy_hunks(argv[1]);
	fclose(outfile);
	freebuf();
	printf("\nDone.\n");
    }
    else {
	getbuf();
	if ((infile = fopen(argv[1], "rb")) == NULL) {
	    printf("\nCouldn't open input file!\n");
	    return;
	}
	if (stat(argv[1], &stat_buf) != 0) {
	    printf("\nBad file handle!\n");
	    return;
	}
	if (argc == 3)
	    hunk_size = atol(argv[2]);
	else
	    hunk_size = (stat_buf.st_size / 2) + 1;
	if (hunk_size < 1) {
	    printf("\nInvalid chunk size!\n");
	    return;
	}
	printf("\nSplitting %s ...\n\n", argv[1]);
	write_hunks();
	fclose(infile);
	freebuf();
	printf("\nDone.\n");
    }
}

write_hunks()
{
long size;
unsigned bufsize;
unsigned numread;

    for (;;) {
	if(!next_file()) {
	    printf("Too many files, please specify a chunk size that\n");
	    printf("will result in fewer than 1000 output files!\n");
	    return;
	}
	if ((outfile = fopen(chunk_name, "wb")) == NULL) {
	    printf("Unable to create output file %s\n", chunk_name);
	    return;
	}
	size = hunk_size;
	numread = 1;
	while(size > 0 && numread /* Work around TC idiot-syncracy */) {
	    bufsize = size < buflen ? size : buflen;
	    numread = fread(buf, sizeof(char), bufsize, infile);
	    if (ferror(infile)) {
		printf("Error while reading input file %s\n", chunk_name);
		fclose(outfile);
		return;
	    }
	    fwrite(buf, sizeof(char), numread, outfile);
	    if (ferror(outfile)) {
		printf("Error while writing output file!\n");
		fclose(outfile);
		return;
	    }
	    size -= numread;
	    if (numread != bufsize) {
		printf("    Writing %ld bytes to %s\n", hunk_size-size, chunk_name);
		fclose(outfile);
		return;
	    }
	}
	fclose(outfile);
	printf("    Writing %ld bytes to %s\n", hunk_size-size, chunk_name);
    }
}

copy_hunks(filename)
char *filename;
{
unsigned numread;

    if(!next_file())
	return;
    if ((infile = fopen(chunk_name, "rb")) == NULL) {
	printf("Nothing to do!\n");
	return;
    }
    if ((outfile = fopen(filename, "wb")) == NULL) {
	printf("Couldn't open output file!\n");
	return;
    }
    for (;;) {
	numread = 1;
	while(!feof(infile) && numread /* Avoid TC problem */) {
	    numread = fread(buf, sizeof(char), buflen, infile);
	    if (ferror(infile)) {
		printf("Error while reading input file %s\n", chunk_name);
		fclose(infile);
		return;
	    }
	    fwrite(buf, sizeof(char), numread, outfile);
	    if (ferror(outfile)) {
		printf("Error while writing output file!\n");
		fclose(infile);
		return;
	    }
	}
	printf("    Copying file %s to output file.\n", chunk_name);
	fclose(infile);
	if(!next_file())
	    return;
	if ((infile = fopen(chunk_name, "rb")) == NULL)
	    return;
	    
    }
}

next_file()
{
char num[4];

    if (extent > 999)
	return(0);
    strcpy(chunk_name, fname);
    itoa(extent,num, 10);
    if (strlen(num) == 1) {
	strcat(chunk_name, "00");
	strcat(chunk_name, num);
    }
    else if (strlen(num) == 2) {
	strcat(chunk_name, "0");
	strcat(chunk_name, num);
    }
    else
	strcat(chunk_name, num);
    ++extent;
    return(-1);
}

getbuf()
{
    while (buflen >= 256 && !(buf = malloc(buflen)))
	buflen >>= 1;
    if (!buf) {
	printf("\nCan't allocate an adequate copy buffer.\n");
	exit(2);
	}
}

freebuf()
{
    free(buf);
}

