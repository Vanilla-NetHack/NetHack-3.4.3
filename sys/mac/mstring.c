/*	SCCS Id: @(#)mstring.c	3.1	93/01/24		  */
/* Copyright (c) Jon W{tte */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef applec

extern int strlen ( char * ) ;
char * PtoCstr ( unsigned char * ) ;
unsigned char * PtoCstr ( char * ) ;

char *
PtoCstr ( unsigned char * p )
{
	int len = * p ;
	char * ret = ( char * ) p ;

	while ( len -- ) {

		* p = p [ 1 ] ;
		p ++ ;
	}
	* p = 0 ;

	return ret ;
}


unsigned char *
CtoPstr ( char * p )
{
	int len = strlen ( p ) ;
	unsigned char * ret = ( unsigned char * ) p ;

	p += len ;
	while ( p > ret ) {

		* p = p [ -1 ] ;
		p -- ;
	}
	* ret = len ;

	return ret ;
}

#endif
