/* Copyright (C) 1986,1987 by Manx Software Systems, Inc. */

#ifndef FCNTL_H
#define FCNTL_H

#define O_RDONLY	0
#define O_WRONLY	1
#define O_RDWR		2
#define O_CREAT 	0x0100
#define O_TRUNC 	0x0200
#define O_EXCL		0x0400
#define O_APPEND	0x0800

#define O_CONRAW	0x4000
#define O_STDIO 	0x8000

extern struct _dev {
	long	fd;
	short	mode;
} *_devtab;

extern short _numdev;

#define O_BINARY	0   /* For MSDOS */
#define setmode(f,m)

#endif

