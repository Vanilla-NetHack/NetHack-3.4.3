/*	SCCS Id: @(#)macerrs.c	3.1	93/01/24		  */
/* Copyright (c) Michael Hamel, 1991 */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef applec	/* This needs to be resident always */
#pragma segment Main
#endif

#include "hack.h"

#include <OSUtils.h>
#include <files.h>
#include <Types.h>
#ifdef MAC_MPW32
#include <String.h>
#include <Strings.h>
#else
#include <pascal.h>
#endif
#include <Dialogs.h>
#include <Packages.h>
#include <ToolUtils.h>
#include <Resources.h>

#define stackDepth  4
#define errAlertID 129
#define stdIOErrID 1999

void FDECL(comment,(char *, long));
void FDECL(showerror,(char *, const char *));
Boolean FDECL(itworked,(short));
void FDECL(mustwork,(short));
static void VDECL(vprogerror,(const char *line, va_list the_args));
void FDECL(attemptingto,( char * activity ));
void FDECL(pushattemptingto,( char * activity ));
void NDECL(popattempt);

static Str255 gActivities[stackDepth] = {"","","",""};
static short gTopactivity = 1;

void  comment( char *s, long n )
{
  Str255 paserr;
  short itemHit;
  
  sprintf((char *)paserr, "%s - %d",s,n);
  ParamText(c2pstr((char *)paserr),(StringPtr)"",(StringPtr)"",(StringPtr)"");
  itemHit = Alert(128, (ModalFilterProcPtr)nil);
}

void showerror( char * errdesc, const char * errcomment )
{
   	short		itemHit;
	Str255		paserr,
				pascomment;
				
	SetCursor(&qd.arrow);
	if (errcomment == nil)  pascomment[0] = '\0';
	  else strcpy((char *)pascomment,(char *)errcomment);
	strcpy((char *)paserr,(char *)errdesc);
	ParamText(c2pstr((char *)paserr),c2pstr((char *)pascomment),gActivities[gTopactivity],(StringPtr)"");
	itemHit = Alert(errAlertID, (ModalFilterProcPtr)nil);
}


Boolean itworked( short errcode )
/* Return TRUE if it worked, do an error message and return false if it didn't. Error
   strings for native C errors are in STR#1999, Mac errs in STR 2000-errcode, e.g
   2108 for not enough memory */

{
  if (errcode != 0) {
    short		 itemHit;
	Str255 		 errdesc;
	StringHandle strh;
	
	errdesc[0] = '\0';
	if (errcode > 0) GetIndString(errdesc,stdIOErrID,errcode);  /* STDIO file rres, etc */
	else {
	   strh = GetString(2000-errcode);
	   if (strh != (StringHandle) nil) {
	      memcpy(errdesc,*strh,256);
		  ReleaseResource((Handle)strh);
	   }
	}
	if (errdesc[0] == '\0') {  /* No description found, just give the number */
	   sprintf((char *)errdesc,"a %d error occurred",errcode);
	   (void)c2pstr((char *)errdesc);
	}
	SetCursor(&qd.arrow);
	ParamText(errdesc,(StringPtr)"",gActivities[gTopactivity],(StringPtr)"");
	itemHit = Alert(errAlertID, (ModalFilterProcPtr)nil);

  }
  return(errcode==0);
}

void mustwork( short errcode )
/* For cases where we can't recover from the error by any means */
{
  if (itworked(errcode)) ;
  	 else ExitToShell();
}


#if defined(USE_STDARG) || defined(USE_VARARGS)
# ifdef USE_STDARG
static void vprogerror(const char *line, va_list the_args);
# else
static void vprogerror();
# endif

/* Macro substitute for error() */
void progerror VA_DECL(const char *, line)
	VA_START(line);
	VA_INIT(line, char *);
	vprogerror(line, VA_ARGS);
	VA_END();
}

# ifdef USE_STDARG
static void
vprogerror(const char *line, va_list the_args) {
# else
static void
vprogerror(line, the_args) const char *line; va_list the_args; {
# endif

#else  /* USE_STDARG | USE_VARARG */

void
progerror VA_DECL(const char *, line)
#endif
/* Do NOT use VA_START and VA_END in here... see above */

	if(!index(line, '%'))
	    showerror("of an internal error",line);
	else {
	    char pbuf[BUFSZ];
	    Vsprintf(pbuf,line,VA_ARGS);
	    showerror("of an internal error",pbuf);
	}
}

void attemptingto( char * activity )
/* Say what we are trying to do for subsequent error-handling: will appear as x in an
   alert in the form "Could not x because y" */
{
   strcpy((char *)gActivities[gTopactivity],activity);
   activity = (char *)c2pstr((char *)gActivities[gTopactivity]);
}

void pushattemptingto( char * activity )
/* Push a new description onto stack so we can pop later to previous state */
{
  if (gTopactivity < stackDepth) {
    gTopactivity++;
    attemptingto(activity);
  }
  else progerror("activity stack overflow");
}

void popattempt( void )
/* Pop to previous state */
{
  if (gTopactivity > 1) --gTopactivity;
				   else progerror("activity stack underflow");
}
