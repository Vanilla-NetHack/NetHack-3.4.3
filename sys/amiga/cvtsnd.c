/*	SCCS Id: @(#)cvtsnd.c	3.1	93/1/28	*/
/* 	Copyright (c) 1993 by Gregg Wonderly */
/* NetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <exec/types.h>

void process_snd( char *, char * );
char *basename( char * );

typedef struct AIFFHEAD
{
	short namelen;
	char name[62];
	short leneq64;
	char form[4];
	char stuff[10+16+32];
	char FORM[4];
	long flen;
	char AIFF[4];
	char SSND[4];
	long sndlen;
} AIFFHEAD;

typedef struct VHDR
{
	char name[4];
	long len;
	unsigned long oneshot, repeat, samples;
	UWORD freq;
	UBYTE n_octaves, compress;
	LONG volume;
} VHDR;

typedef struct IFFHEAD
{
	char FORM[4];
	long flen;
	char _8SVX[4];
	VHDR vhdr;
} IFFHEAD;

main( argc, argv )
	int argc;
	char **argv;
{
	if( argc != 3 )
	{
		fprintf( stderr, "usage: %s source-file dest-file\n", argv[ 0 ] );
		exit( 1 );
	}

	process_snd( argv[1], argv[2] );
}

void
process_snd( src, dest )
	char *src, *dest;
{
	int n;
	AIFFHEAD aih;
	IFFHEAD ih;
	long len;
	int fd, nfd;
	char buf[ 300 ];

	fd = open( src, 0 );
	if( fd == -1 )
	{
		perror( src );
		return;
	}

	read( fd, &aih, sizeof( aih ) );
	nfd = creat( dest, 0777 );
	memcpy( ih.FORM, "FORM", 4 );
	memcpy( ih._8SVX, "8SVX", 4 );
	ih.flen = aih.sndlen + sizeof( IFFHEAD ) + 8;
	memcpy( ih.vhdr.name, "VHDR", 4 );
	ih.vhdr.len = 20;
	ih.vhdr.oneshot = aih.sndlen;
	ih.vhdr.repeat = 0;
	ih.vhdr.samples = 0;
	ih.vhdr.freq = 22000;
	ih.vhdr.n_octaves = 1;
	ih.vhdr.compress = 0;
	ih.vhdr.volume = 0x10000;

	write( nfd, &ih, sizeof( ih ) );
	write( nfd, "NAME", 4 );
	len = aih.namelen;
	write( nfd, &len, 4 );
	write( nfd, aih.name, ( strlen( aih.name ) + 1 ) & ~1 );
	write( nfd, "BODY", 4 );
	write( nfd, &aih.sndlen, 4 );

	while( ( n = read( fd, buf, sizeof( buf ) ) ) > 0 )
		write( nfd, buf, n );
	close( fd );
	close( nfd );
}

char *basename( str )
	char *str;
{
	char *t;
	if( t = strrchr( str, '/' ) )
		return(t);
	if( t = strrchr( str, ':' ) )
		return(t);
	return( str );
}
