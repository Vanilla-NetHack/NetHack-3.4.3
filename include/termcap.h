/*	SCCS Id: @(#)termcap.h	3.1	92/10/21	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

/* common #defines for print.c and termcap.c */

#ifndef TERMCAP_H
#define TERMCAP_H

#ifndef MICRO
#  define TERMLIB	/* include termcap code */
#endif

/* might display need graphics code? */
#if !defined(AMIGA) && !defined(TOS)
# if defined(TERMLIB) || defined(OS2) || defined(MSDOS)
#  define ASCIIGRAPH
# endif
#endif

#ifndef DECL_H
extern struct tc_gbl_data {   /* also declared in decl.h; defined in decl.c */
    char *tc_AS, *tc_AE;	/* graphics start and end (tty font swapping) */
    int   tc_LI,  tc_CO;	/* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

extern struct tc_lcl_data {   /* defined and set up in termcap.c */
    char *tc_CM, *tc_ND, *tc_CD;
    char *tc_HI, *tc_HE, *tc_US, *tc_UE;
    boolean tc_ul_hack;
} tc_lcl_data;
#define CM tc_lcl_data.tc_CM
#define ND tc_lcl_data.tc_ND
#define CD tc_lcl_data.tc_CD
#define HI tc_lcl_data.tc_HI
#define HE tc_lcl_data.tc_HE
#define US tc_lcl_data.tc_US
#define UE tc_lcl_data.tc_UE
#define ul_hack tc_lcl_data.tc_ul_hack

extern short ospeed;		/* set up in termcap.c */

#ifdef TEXTCOLOR
# ifdef TOS
extern const char *hilites[MAXCOLORS];
# else
extern NEARDATA char *hilites[MAXCOLORS];
# endif
#endif

#endif /* TERMCAP_H */
