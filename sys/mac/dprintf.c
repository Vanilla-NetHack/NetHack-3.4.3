/*	SCCS Id: @(#)dprintf.c	3.1	93/05/14		  */
/* Copyright (c) Jon W{tte, 1993.				  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include <Types.h>
#include <stdarg.h>
#include <stdio.h>
#include <strings.h>
#include <GestaltEqu.h>


Boolean
HasDebugger ( void ) {
long osAttr ;
	if ( Gestalt ( gestaltOSAttr , & osAttr ) ||
		! ( osAttr & ( 1 << gestaltSysDebuggerSupport ) ) ) {
		return 0 ;
	}
	return 1 ;
}


Boolean
KeyDown ( unsigned short code ) {
unsigned char keys [ 16 ] ;

	GetKeys ( ( void * ) keys ) ;
	return ( ( keys [ code >> 3 ] >> ( code & 7 ) ) & 1 ) != 0 ;
}


void
dprintf ( char * format , ... ) {
static char buffer [ 100 ] ;
va_list list ;
static Boolean checkedTrap = 0 ;
static Boolean trapAvailable = 0 ;

	if ( ! checkedTrap ) {
		checkedTrap = 1 ;
		trapAvailable = HasDebugger ( ) ;
	}
	list = va_start ( list , format ) ;
	vsprintf ( & buffer [ 1 ] , format , list ) ;
	va_end ( list )  ;
	buffer [ 0 ] = strlen ( & buffer [ 1 ] ) ;
	if ( trapAvailable ) {
		if ( KeyDown ( 0x39 ) ) {									/* Caps Lock */
			DebugStr ( buffer ) ;
		} else if ( KeyDown ( 0x3B ) && flags . window_inited &&	/* Control */
			( WIN_MESSAGE != -1 ) && theWindows [ WIN_MESSAGE ] . theWindow ) {
			pline ( "%s" , & buffer [ 1 ] ) ;
		}
	}
}
