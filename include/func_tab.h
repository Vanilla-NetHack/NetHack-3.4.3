/*	SCCS Id: @(#)func_tab.h	3.0	88/10/15
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* func_tab.h - version 1.0.2 */

#ifndef FUNC_TAB_H
#define FUNC_TAB_H

struct func_tab {
	char f_char;
	int NDECL((*f_funct));
	const char *f_text;
};

extern const struct func_tab cmdlist[];

struct ext_func_tab {
	const char *ef_txt, *ef_desc;
	int NDECL((*ef_funct));
};

extern const struct ext_func_tab extcmdlist[];

#endif /* FUNC_TAB_H /**/
