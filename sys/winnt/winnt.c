/*	SCCS Id: @(#)winnt.c	 3.1	 93/01/31		  */
/* Copyright (c) NetHack PC Development Team 1990, 1991, 1992	  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Windows NT system functions.
 *
 *  Initial Creation: Michael Allison - January 31/93
 *
 */

#define NEED_VARARGS
#include "hack.h"

#ifdef WIN32

#include <dos.h>
#include <direct.h>
#include <ctype.h>
#include <windows.h>

/*
 * The following WIN32 API routines are used in this file.
 *
 * GetDiskFreeSpace
 * GetVolumeInformation
 * FindFirstFile
 * FindNextFile
 * FindClose
 *
 */


/* globals required within here */
HANDLE ffhandle = NULL;
WIN32_FIND_DATA ffd;


char
switchar()
{
 /* Could not locate a WIN32 API call for this- MJA */
	return '-';
}

long
freediskspace(path)
char *path;
{
	char tmppath[4];
	DWORD SectorsPerCluster = 0;
	DWORD BytesPerSector = 0;
	DWORD FreeClusters = 0;
	DWORD TotalClusters = 0;

	tmppath[0] = *path;
	tmppath[1] = ':';
	tmppath[2] = '\\';
	tmppath[3] = '\0';
	GetDiskFreeSpace(tmppath, &SectorsPerCluster,
			&BytesPerSector,
			&FreeClusters,
			&TotalClusters);
	return (long)(SectorsPerCluster * BytesPerSector *
			FreeClusters);
}

/*
 * Functions to get filenames using wildcards
 */
int
findfirst(path)
char *path;
{
	if (ffhandle){
		 FindClose(ffhandle);
		 ffhandle = NULL;
	}
	ffhandle = FindFirstFile(path,&ffd);
	return 
	  (ffhandle == INVALID_HANDLE_VALUE) ? 0 : 1;
}

int
findnext() 
{
	return FindNextFile(ffhandle,&ffd) ? 1 : 0;
}

char *
foundfile_buffer()
{
	return &ffd.cFileName[0];
}

long
filesize(file)
char *file;
{
	if (findfirst(file)) {
		return ((long *)&ffd.nFileSizeLow);
	} else
		return -1L;
}

/*
 * Chdrive() changes the default drive.
 */
void
chdrive(str)
char *str;
{
	char *ptr;
	char drive;
	if ((ptr = index(str, ':')) != NULL) 
	{
		drive = toupper(*(ptr - 1));
		_chdrive((drive - 'A') + 1);
	}
}

/* NT supports long file names, but does THIS particular volume? */
void
nt_regularize(s)
char *s;
{
	DWORD maxflen;
	int status=0;
	
	status = GetVolumeInformation(NULL,NULL,NULL
			,NULL,&maxflen,NULL,NULL,NULL);
	if (status)
	{
	   if (strlen(s) > maxflen-4) s[maxflen-4] = '\0';
	}
}
#endif /* WIN32 */
