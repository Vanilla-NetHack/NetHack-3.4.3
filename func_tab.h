/*	SCCS Id: @(#)func_tab.h	1.4	87/08/08
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* func_tab.h - version 1.0.2 */

struct func_tab {
	char f_char;
	int (*f_funct)();
	char *f_text;
};

extern struct func_tab cmdlist[];

struct ext_func_tab {
	char *ef_txt, *ef_desc;
	int (*ef_funct)();
};

extern struct ext_func_tab extcmdlist[];
