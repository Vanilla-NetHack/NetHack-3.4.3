/*    SCCS Id: @(#)wbstruct.h   3.1    93/01/08
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1992, 1993.  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * The NetHack WorkBench's typedef/struct definitions.
 */
typedef unsigned char flag;

typedef struct GAMEITEM
{
    struct DiskObject *dobj;	/* Icon structure pointer */
    char
	*gname,			/* Process name running this game */
	*name,			/* name of icon without .info */
	*dname,			/* Directory where save file is */
	*fname;			/* File name on disk with .sav */
    struct GAMEITEM *nextwgad;  /* Next in list current visible */
    struct GAMEITEM *next;      /* Next in complete list */
    struct Process *prc;        /* Process running this game */
    struct MsgPort *port;       /* Port to send message to */
    struct MsgPort *prcport;    /* Port for process termination */
    long secs, mics;		/* Double click times */
    long oact, oflag;		/* orig Flags and activation for diskobj */
    struct WBStartup *wbs;	/* WorkBench startup message to send */
    struct WBArg *wba;		/* WorkBench args */
    char **otools;		/* Original dobj->do_ToolTypes pointer */
    BPTR lock;			/* Lock on game file */
    int toolcnt;		/* Number of pointers allocated in dobj */
    BPTR seglist;		/* Seglist of loaded game */
    flag talloc;		/* ToolTypes have been reallocated */
    flag active;		/* Is this game active */
} GAMEITEM, *GPTR;

typedef struct OPTIONS
{
    char optval;		/* Options current boolean value */
    char defval;		/* The default boolean value nethack assumes */
    char *name;			/* Name of the option. */
    char *optstr;		/* Options current string value or "" */
    int id;			/* GadgetID of gadget manipulating this
				 * options value.
				 */
} OPTIONS, *OPTR;
